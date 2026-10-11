#pragma once

#include "app/types.h"
#include "audio/device_catalog.h"
#include "audio/pipewire_streams.h"
#include "audio/route_manager.h"
#include "audio/stream_bridge.h"
#include "dsp/parameters.h"

#include <chrono>
#include <memory>
#include <string>
#include <vector>

namespace bass {

class EngineController {
public:
  EngineController();
  ~EngineController();
  EngineController(const EngineController&) = delete;
  EngineController& operator=(const EngineController&) = delete;

  std::vector<AudioEndpoint> endpoints() const;
  const AudioConfiguration& configuration() const noexcept { return config_; }
  const Parameters& parameters() const noexcept { return parameters_; }
  EngineState state() const noexcept { return state_; }
  const Diagnostic& diagnostic() const noexcept { return diagnostic_; }
  const MeterSnapshot& meter() const noexcept { return meter_; }
  bool meter_available() const noexcept { return meter_available_; }
  bool server_available() const noexcept { return static_cast<bool>(context_); }
  bool configure(const AudioConfiguration& config);
  bool set_parameters(const Parameters& parameters);
  bool start();
  void stop();
  void tick();
  void refresh();

private:
  void fail(std::string message, std::string action);
  std::unique_ptr<PipeWireContext> context_;
  std::unique_ptr<DeviceCatalog> catalog_;
  std::unique_ptr<RouteManager> routes_;
  StreamBridge bridge_;
  std::unique_ptr<PipeWireStreams> streams_;
  AudioConfiguration config_{};
  Parameters parameters_{};
  EngineState state_{EngineState::Stopped};
  Diagnostic diagnostic_{};
  MeterSnapshot meter_{};
  bool meter_available_{};
  bool pending_parameters_{};
  std::chrono::steady_clock::time_point connect_started_{};
};

} // namespace bass
