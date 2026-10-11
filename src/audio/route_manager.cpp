#include "audio/route_manager.h"

#include <cstdlib>
#include <cstring>
#include <stdexcept>

namespace bass {

namespace {
const pw_registry_events kEvents = [] {
  pw_registry_events events{};
  events.version = PW_VERSION_REGISTRY_EVENTS;
  events.global = RouteManager::on_global;
  events.global_remove = RouteManager::on_remove;
  return events;
}();
}

RouteManager::RouteManager(PipeWireContext& context) : context_(context) {
  PipeWireLock lock(context_);
  registry_ = pw_core_get_registry(context_.core(), PW_VERSION_REGISTRY, 0);
  if (!registry_) throw std::runtime_error("PipeWire link registry is unavailable");
  pw_registry_add_listener(registry_, &hook_, &kEvents, this);
}

RouteManager::~RouteManager() {
  PipeWireLock lock(context_);
  spa_hook_remove(&hook_);
  pw_proxy_destroy(reinterpret_cast<pw_proxy*>(registry_));
}

void RouteManager::on_global(void* data, std::uint32_t id, std::uint32_t,
                             const char* type, std::uint32_t, const spa_dict* props) {
  if (std::strcmp(type, PW_TYPE_INTERFACE_Link) != 0 || !props) return;
  const char* output = spa_dict_lookup(props, PW_KEY_LINK_OUTPUT_NODE);
  const char* input = spa_dict_lookup(props, PW_KEY_LINK_INPUT_NODE);
  if (!output || !input) return;
  auto* self = static_cast<RouteManager*>(data);
  std::lock_guard lock(self->mutex_);
  self->links_[id] = {static_cast<std::uint32_t>(std::strtoul(output, nullptr, 10)),
                      static_cast<std::uint32_t>(std::strtoul(input, nullptr, 10))};
}

void RouteManager::on_remove(void* data, std::uint32_t id) {
  auto* self = static_cast<RouteManager*>(data);
  std::lock_guard lock(self->mutex_);
  self->links_.erase(id);
}

bool RouteManager::verified(std::uint32_t source, std::uint32_t capture_stream,
                            std::uint32_t playback_stream, std::uint32_t sink) const {
  if (source == SPA_ID_INVALID || capture_stream == SPA_ID_INVALID ||
      playback_stream == SPA_ID_INVALID || sink == SPA_ID_INVALID) return false;
  std::lock_guard lock(mutex_);
  bool capture_link = false, playback_link = false;
  for (const auto& [id, pair] : links_) {
    (void)id;
    capture_link |= pair.first == source && pair.second == capture_stream;
    playback_link |= pair.first == playback_stream && pair.second == sink;
  }
  return capture_link && playback_link;
}

} // namespace bass
