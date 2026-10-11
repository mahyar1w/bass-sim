#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace bass {

enum class Direction { Capture, Playback };
enum class Availability { Present, Disconnected };
enum class EngineState { Stopped, Preparing, Connecting, Running, Reconfiguring, Stopping, ErrorMuted };
enum class Severity { Info, Warning, Error };

struct AudioEndpoint {
  std::uint32_t id{};
  std::uint64_t serial{};
  std::string name;
  std::string node_name;
  Direction direction{};
  std::vector<std::string> channels;
  Availability availability{Availability::Present};
};

struct AudioConfiguration {
  std::uint32_t capture_id{};
  std::uint32_t playback_id{};
  unsigned capture_channel{};
  unsigned playback_channels{2};
  unsigned requested_rate{48000};
  unsigned requested_quantum{128};
  std::optional<unsigned> actual_rate;
  std::optional<unsigned> actual_quantum;
  std::optional<unsigned> actual_channels;
  bool route_verified{};
};

struct Diagnostic {
  Severity severity{Severity::Info};
  std::string message;
  std::string action;
};

struct MeterSnapshot {
  float input_peak{};
  float output_peak{};
  bool input_clipped{};
  bool output_clipped{};
  std::uint64_t underruns{};
  std::uint64_t overruns{};
};

} // namespace bass
