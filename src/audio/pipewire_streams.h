#pragma once

#include "app/types.h"
#include "audio/pipewire_context.h"
#include "audio/stream_bridge.h"

#include <atomic>
#include <cstdint>

namespace bass {

class PipeWireStreams {
public:
  PipeWireStreams(PipeWireContext& context, StreamBridge& bridge);
  ~PipeWireStreams();
  bool start(const AudioEndpoint& capture, const AudioEndpoint& playback, const AudioConfiguration& config);
  void stop();
  bool streaming() const noexcept;
  bool failed() const noexcept { return failed_.load(); }
  void set_route_verified(bool verified) noexcept { route_verified_.store(verified); }
  std::uint32_t capture_node() const noexcept;
  std::uint32_t playback_node() const noexcept;
  unsigned capture_rate() const noexcept { return capture_rate_.load(); }
  unsigned capture_channels() const noexcept { return capture_channels_.load(); }
  unsigned playback_rate() const noexcept { return playback_rate_.load(); }
  unsigned playback_channels() const noexcept { return playback_channels_.load(); }
  unsigned observed_quantum() const noexcept { return observed_quantum_.load(); }
  struct Side { PipeWireStreams* owner{}; bool capture{}; };
  static void on_process(void* data) noexcept;
  static void on_state(void* data, pw_stream_state old, pw_stream_state state, const char* error) noexcept;
  static void on_param(void* data, std::uint32_t id, const spa_pod* param) noexcept;

private:
  void process(bool capture) noexcept;
  PipeWireContext& context_;
  StreamBridge& bridge_;
  pw_stream* capture_{};
  pw_stream* playback_{};
  spa_hook capture_hook_{};
  spa_hook playback_hook_{};
  Side capture_side_{this, true};
  Side playback_side_{this, false};
  std::atomic<pw_stream_state> capture_state_{PW_STREAM_STATE_UNCONNECTED};
  std::atomic<pw_stream_state> playback_state_{PW_STREAM_STATE_UNCONNECTED};
  std::atomic<unsigned> capture_rate_{};
  std::atomic<unsigned> playback_rate_{};
  std::atomic<unsigned> capture_channels_{};
  std::atomic<unsigned> playback_channels_{};
  std::atomic<unsigned> observed_quantum_{};
  std::atomic<bool> failed_{};
  std::atomic<bool> route_verified_{};
  unsigned capture_channel_{};
};

} // namespace bass
