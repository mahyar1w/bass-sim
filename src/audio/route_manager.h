#pragma once

#include "audio/pipewire_context.h"

#include <cstdint>
#include <mutex>
#include <unordered_map>
#include <utility>

namespace bass {

class RouteManager {
public:
  explicit RouteManager(PipeWireContext& context);
  ~RouteManager();
  bool verified(std::uint32_t source, std::uint32_t capture_stream,
                std::uint32_t playback_stream, std::uint32_t sink) const;
  static void on_global(void* data, std::uint32_t id, std::uint32_t permissions,
                        const char* type, std::uint32_t version, const spa_dict* props);
  static void on_remove(void* data, std::uint32_t id);

private:
  PipeWireContext& context_;
  pw_registry* registry_{};
  spa_hook hook_{};
  mutable std::mutex mutex_;
  std::unordered_map<std::uint32_t, std::pair<std::uint32_t, std::uint32_t>> links_;
};

} // namespace bass
