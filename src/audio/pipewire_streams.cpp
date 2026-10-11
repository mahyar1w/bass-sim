#include "audio/pipewire_streams.h"

#include <spa/param/audio/format-utils.h>

#include <algorithm>
#include <array>
#include <string>

namespace bass {

namespace {
const pw_stream_events kEvents = [] {
  pw_stream_events events{};
  events.version = PW_VERSION_STREAM_EVENTS;
  events.state_changed = PipeWireStreams::on_state;
  events.param_changed = PipeWireStreams::on_param;
  events.process = PipeWireStreams::on_process;
  return events;
}();
}

PipeWireStreams::PipeWireStreams(PipeWireContext& context, StreamBridge& bridge)
    : context_(context), bridge_(bridge) {}

PipeWireStreams::~PipeWireStreams() { stop(); }

bool PipeWireStreams::start(const AudioEndpoint& capture, const AudioEndpoint& playback,
                            const AudioConfiguration& config) {
  stop();
  capture_channel_ = config.capture_channel;
  capture_rate_ = playback_rate_ = 0;
  capture_channels_ = playback_channels_ = 0;
  observed_quantum_ = 0;
  failed_ = false;
  route_verified_ = false;
  bridge_.prepare(config.requested_rate, config.requested_rate);
  const std::string latency = std::to_string(config.requested_quantum) + "/" +
                              std::to_string(config.requested_rate);
  PipeWireLock lock(context_);
  auto make = [&](const char* name, const std::string& target, bool input) {
    auto* props = pw_properties_new(PW_KEY_MEDIA_TYPE, "Audio",
                                    PW_KEY_MEDIA_CATEGORY, input ? "Capture" : "Playback",
                                    PW_KEY_MEDIA_ROLE, "Music",
                                    PW_KEY_TARGET_OBJECT, target.c_str(),
                                    PW_KEY_NODE_LATENCY, latency.c_str(),
                                    PW_KEY_NODE_DONT_RECONNECT, "true",
                                    "node.dont-fallback", "true",
                                    "node.dont-move", "true", nullptr);
    return pw_stream_new(context_.core(), name, props);
  };
  capture_ = make("bass-sim-capture", capture.node_name, true);
  playback_ = make("bass-sim-playback", playback.node_name, false);
  if (!capture_ || !playback_) {
    failed_ = true;
    if (capture_) pw_stream_destroy(capture_);
    if (playback_) pw_stream_destroy(playback_);
    capture_ = playback_ = nullptr;
    return false;
  }
  pw_stream_add_listener(capture_, &capture_hook_, &kEvents, &capture_side_);
  pw_stream_add_listener(playback_, &playback_hook_, &kEvents, &playback_side_);

  auto connect = [&](pw_stream* stream, pw_direction direction, unsigned channels) {
    std::array<std::uint8_t, 1024> bytes{};
    spa_pod_builder builder = SPA_POD_BUILDER_INIT(bytes.data(), bytes.size());
    spa_audio_info_raw format{};
    format.format = SPA_AUDIO_FORMAT_F32;
    format.rate = config.requested_rate;
    format.channels = channels;
    const spa_pod* pod = spa_format_audio_raw_build(&builder, SPA_PARAM_EnumFormat, &format);
    return pw_stream_connect(stream, direction, PW_ID_ANY,
                             static_cast<pw_stream_flags>(PW_STREAM_FLAG_AUTOCONNECT |
                               PW_STREAM_FLAG_MAP_BUFFERS | PW_STREAM_FLAG_RT_PROCESS), &pod, 1);
  };
  if (connect(capture_, PW_DIRECTION_INPUT, 0) < 0 ||
      connect(playback_, PW_DIRECTION_OUTPUT, config.playback_channels) < 0) {
    failed_ = true;
    pw_stream_destroy(capture_);
    pw_stream_destroy(playback_);
    capture_ = playback_ = nullptr;
    return false;
  }
  return true;
}

void PipeWireStreams::stop() {
  route_verified_ = false;
  PipeWireLock lock(context_);
  if (capture_) { pw_stream_destroy(capture_); capture_ = nullptr; }
  if (playback_) { pw_stream_destroy(playback_); playback_ = nullptr; }
  capture_state_ = playback_state_ = PW_STREAM_STATE_UNCONNECTED;
}

bool PipeWireStreams::streaming() const noexcept {
  return capture_state_.load() == PW_STREAM_STATE_STREAMING &&
         playback_state_.load() == PW_STREAM_STATE_STREAMING;
}

std::uint32_t PipeWireStreams::capture_node() const noexcept {
  return capture_ ? pw_stream_get_node_id(capture_) : SPA_ID_INVALID;
}

std::uint32_t PipeWireStreams::playback_node() const noexcept {
  return playback_ ? pw_stream_get_node_id(playback_) : SPA_ID_INVALID;
}

void PipeWireStreams::on_state(void* data, pw_stream_state, pw_stream_state state,
                               const char*) noexcept {
  const auto* side = static_cast<Side*>(data);
  (side->capture ? side->owner->capture_state_ : side->owner->playback_state_).store(state);
  if (state == PW_STREAM_STATE_ERROR) side->owner->failed_ = true;
}

void PipeWireStreams::on_param(void* data, std::uint32_t id, const spa_pod* param) noexcept {
  if (id != SPA_PARAM_Format || !param) return;
  spa_audio_info info{};
  if (spa_format_parse(param, &info.media_type, &info.media_subtype) < 0 ||
      info.media_type != SPA_MEDIA_TYPE_audio || info.media_subtype != SPA_MEDIA_SUBTYPE_raw ||
      spa_format_audio_raw_parse(param, &info.info.raw) < 0) return;
  const auto* side = static_cast<Side*>(data);
  if (info.info.raw.format != SPA_AUDIO_FORMAT_F32 || info.info.raw.channels == 0 ||
      info.info.raw.channels > 32 || info.info.raw.rate == 0) {
    side->owner->failed_ = true;
    return;
  }
  (side->capture ? side->owner->capture_rate_ : side->owner->playback_rate_).store(info.info.raw.rate);
  (side->capture ? side->owner->capture_channels_ : side->owner->playback_channels_).store(info.info.raw.channels);
}

void PipeWireStreams::on_process(void* data) noexcept {
  const auto* side = static_cast<Side*>(data);
  side->owner->process(side->capture);
}

void PipeWireStreams::process(bool capture) noexcept {
  auto* stream = capture ? capture_ : playback_;
  if (!stream) return;
  pw_buffer* buffer = pw_stream_dequeue_buffer(stream);
  if (!buffer) return;
  spa_buffer* raw = buffer->buffer;
  if (!raw || raw->n_datas == 0 || !raw->datas[0].data) {
    pw_stream_queue_buffer(stream, buffer);
    return;
  }
  auto& data = raw->datas[0];
  float* samples = static_cast<float*>(data.data);
  if (capture) {
    const unsigned channels = std::max(1u, capture_channels_.load(std::memory_order_relaxed));
    const unsigned channel = std::min(capture_channel_, channels - 1);
    const std::size_t offset = data.chunk ? data.chunk->offset : 0;
    const std::size_t bytes = data.chunk ? data.chunk->size : data.maxsize;
    const auto frames = std::min<std::size_t>(bytes, data.maxsize - std::min<std::size_t>(offset, data.maxsize)) /
                        (sizeof(float) * channels);
    samples += offset / sizeof(float);
    if (route_verified_.load(std::memory_order_relaxed))
      for (std::size_t i = 0; i < frames; ++i) bridge_.capture(samples[i * channels + channel]);
  } else {
    const unsigned channels = std::max(1u, playback_channels_.load(std::memory_order_relaxed));
    const auto capacity = data.maxsize / (sizeof(float) * channels);
    const auto frames = buffer->requested ?
      std::min<std::size_t>(capacity, buffer->requested) : capacity;
    observed_quantum_.store(static_cast<unsigned>(frames), std::memory_order_relaxed);
    const bool enabled = route_verified_.load(std::memory_order_relaxed);
    for (std::size_t i = 0; i < frames; ++i) {
      const float value = enabled ? bridge_.render() : 0.f;
      for (unsigned channel = 0; channel < channels; ++channel)
        samples[i * channels + channel] = value;
    }
    if (data.chunk) {
      data.chunk->offset = 0;
      data.chunk->stride = sizeof(float) * channels;
      data.chunk->size = frames * sizeof(float) * channels;
    }
    buffer->size = frames;
  }
  pw_stream_queue_buffer(stream, buffer);
}

} // namespace bass
