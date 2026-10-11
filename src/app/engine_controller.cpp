#include "app/engine_controller.h"

#include <algorithm>
#include <exception>
#include <utility>

namespace bass {

EngineController::EngineController() {
  try {
    context_ = std::make_unique<PipeWireContext>();
    catalog_ = std::make_unique<DeviceCatalog>(*context_);
    routes_ = std::make_unique<RouteManager>(*context_);
    streams_ = std::make_unique<PipeWireStreams>(*context_, bridge_);
    diagnostic_ = {Severity::Info, "Audio engine is stopped", "Select an input and output"};
  } catch (const std::exception& error) {
    streams_.reset();
    routes_.reset();
    catalog_.reset();
    context_.reset();
    fail(error.what(), "Start PipeWire and restart Bass-sim");
  }
}

EngineController::~EngineController() { stop(); }

std::vector<AudioEndpoint> EngineController::endpoints() const {
  return catalog_ ? catalog_->endpoints() : std::vector<AudioEndpoint>{};
}

void EngineController::fail(std::string message, std::string action) {
  if (streams_) streams_->set_route_verified(false);
  config_.route_verified = false;
  state_ = EngineState::ErrorMuted;
  diagnostic_ = {Severity::Error, std::move(message), std::move(action)};
}

bool EngineController::configure(const AudioConfiguration& config) {
  const bool was_running = state_ == EngineState::Running || state_ == EngineState::Connecting;
  if (was_running) {
    state_ = EngineState::Reconfiguring;
    stop();
  }
  config_ = config;
  config_.actual_rate.reset();
  config_.actual_quantum.reset();
  config_.actual_channels.reset();
  config_.route_verified = false;
  if (was_running) return start();
  return true;
}

bool EngineController::set_parameters(const Parameters& parameters) {
  if (!valid_parameters(parameters, config_.actual_rate.value_or(config_.requested_rate))) return false;
  parameters_ = parameters;
  pending_parameters_ = true;
  return true;
}

bool EngineController::start() {
  if (!catalog_ || !streams_) {
    fail("PipeWire server is unavailable", "Start PipeWire and restart Bass-sim");
    return false;
  }
  if (state_ == EngineState::Running || state_ == EngineState::Connecting) return true;
  const auto capture = catalog_->find(config_.capture_id);
  const auto playback = catalog_->find(config_.playback_id);
  if (!capture || capture->direction != Direction::Capture) {
    fail("Selected guitar input is unavailable", "Refresh devices and select an input");
    return false;
  }
  if (!playback || playback->direction != Direction::Playback) {
    fail("Selected output is unavailable", "Refresh devices and select an output");
    return false;
  }
  if (!capture->channels.empty() && config_.capture_channel >= capture->channels.size()) {
    fail("Input channel is unavailable", "Select an available input channel");
    return false;
  }
  if (config_.playback_channels < 1 || config_.playback_channels > 2 ||
      config_.requested_rate < 8000 || config_.requested_rate > 192000 ||
      config_.requested_quantum < 32 || config_.requested_quantum > 2048) {
    fail("Audio setting is outside the supported range", "Choose a supported rate, buffer and output mode");
    return false;
  }
  state_ = EngineState::Preparing;
  config_.route_verified = false;
  try {
    if (!streams_->start(*capture, *playback, config_)) {
      fail("Could not open selected audio endpoints", "Choose another device or audio setting");
      return false;
    }
  } catch (const std::exception& error) {
    fail(error.what(), "Choose another device or audio setting");
    return false;
  }
  state_ = EngineState::Connecting;
  connect_started_ = std::chrono::steady_clock::now();
  diagnostic_ = {Severity::Info, "Connecting selected input and output", ""};
  return true;
}

void EngineController::stop() {
  if (state_ == EngineState::Stopped) return;
  state_ = EngineState::Stopping;
  if (streams_) streams_->stop();
  config_.route_verified = false;
  state_ = EngineState::Stopped;
  diagnostic_ = {Severity::Info, "Audio engine is stopped", ""};
}

void EngineController::tick() {
  if (!streams_ || (state_ != EngineState::Connecting && state_ != EngineState::Running)) return;
  if (streams_->failed()) {
    fail("Audio stream failed", "Refresh devices and restart audio");
    return;
  }
  const auto capture = catalog_->find(config_.capture_id);
  const auto playback = catalog_->find(config_.playback_id);
  if (!capture || !playback) {
    fail("Selected audio device disconnected", "Reconnect it or select another device");
    return;
  }
  const bool linked = routes_->verified(config_.capture_id, streams_->capture_node(),
                                         streams_->playback_node(), config_.playback_id);
  if (state_ == EngineState::Connecting) {
    if (streams_->streaming() && linked) {
      const unsigned input_rate = streams_->capture_rate();
      const unsigned output_rate = streams_->playback_rate();
      if (!input_rate || !output_rate || !streams_->capture_channels()) return;
      if (config_.capture_channel >= streams_->capture_channels()) {
        fail("Selected input channel was not accepted", "Choose another input channel");
        return;
      }
      bridge_.prepare(input_rate, output_rate);
      bridge_.submit_parameters(parameters_);
      config_.actual_rate = output_rate;
      if (streams_->observed_quantum()) config_.actual_quantum = streams_->observed_quantum();
      config_.actual_channels = streams_->playback_channels();
      config_.route_verified = true;
      pending_parameters_ = false;
      streams_->set_route_verified(true);
      state_ = EngineState::Running;
      diagnostic_ = {Severity::Info, "Processing on selected devices", ""};
    } else if (std::chrono::steady_clock::now() - connect_started_ > std::chrono::seconds(5)) {
      fail("Selected audio route could not be verified", "Check PipeWire links or choose other devices");
    }
  } else if (!linked || !streams_->streaming()) {
    fail("Audio route moved or disconnected", "Refresh devices and restart audio");
  }
  if (state_ == EngineState::Running) {
    if (streams_->observed_quantum()) config_.actual_quantum = streams_->observed_quantum();
    if (pending_parameters_ && bridge_.submit_parameters(parameters_)) pending_parameters_ = false;
    MeterSnapshot value;
    while (bridge_.poll_meter(value)) { meter_ = value; meter_available_ = true; }
  }
}

void EngineController::refresh() {
  if (!catalog_) return;
  diagnostic_ = {Severity::Info, "Device list refreshed from PipeWire", ""};
}

} // namespace bass
