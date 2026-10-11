#include "app/engine_controller.h"

#include <chrono>
#include <iostream>
#include <thread>

int main() {
  bass::EngineController engine;
  if (!engine.server_available()) {
    std::cerr << engine.diagnostic().message << '\n';
    return 1;
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(400));
  bass::AudioConfiguration config;
  for (const auto& endpoint : engine.endpoints()) {
    std::cout << endpoint.id << ' ' << (endpoint.direction == bass::Direction::Capture ? "in " : "out ")
              << endpoint.name << '\n';
    if (endpoint.name.find("C-Media") != std::string::npos) {
      if (endpoint.direction == bass::Direction::Capture) config.capture_id = endpoint.id;
      else config.playback_id = endpoint.id;
    }
  }
  if (!config.capture_id || !config.playback_id) return 2;
  engine.configure(config);
  auto parameters = engine.parameters();
  parameters.mute = true;
  engine.set_parameters(parameters);
  if (!engine.start()) {
    std::cerr << engine.diagnostic().message << '\n';
    return 3;
  }
  for (int i = 0; i < 40; ++i) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    engine.tick();
    if (engine.state() == bass::EngineState::Running) break;
  }
  std::cout << "state=" << static_cast<int>(engine.state())
            << " route=" << engine.configuration().route_verified
            << " actual_rate=" << engine.configuration().actual_rate.value_or(0)
            << " actual_quantum=" << engine.configuration().actual_quantum.value_or(0)
            << " message=" << engine.diagnostic().message << '\n';
  const bool okay = engine.state() == bass::EngineState::Running && engine.configuration().route_verified;
  engine.stop();
  return okay ? 0 : 4;
}
