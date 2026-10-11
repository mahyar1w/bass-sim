#pragma once

#include "app/types.h"
#include "audio/pipewire_context.h"

#include <mutex>
#include <optional>
#include <vector>

namespace bass {

class DeviceCatalog {
public:
  explicit DeviceCatalog(PipeWireContext& context);
  ~DeviceCatalog();
  std::vector<AudioEndpoint> endpoints() const;
  std::optional<AudioEndpoint> find(std::uint32_t id) const;
  static void on_global(void* data, std::uint32_t id, std::uint32_t permissions,
                        const char* type, std::uint32_t version, const spa_dict* props);
  static void on_remove(void* data, std::uint32_t id);

private:
  PipeWireContext& context_;
  pw_registry* registry_{};
  spa_hook hook_{};
  mutable std::mutex mutex_;
  std::vector<AudioEndpoint> endpoints_;
};

} // namespace bass
