#include "audio/device_catalog.h"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <stdexcept>

namespace bass {

namespace {
const pw_registry_events kEvents = [] {
  pw_registry_events events{};
  events.version = PW_VERSION_REGISTRY_EVENTS;
  events.global = DeviceCatalog::on_global;
  events.global_remove = DeviceCatalog::on_remove;
  return events;
}();
}

DeviceCatalog::DeviceCatalog(PipeWireContext& context) : context_(context) {
  PipeWireLock lock(context_);
  registry_ = pw_core_get_registry(context_.core(), PW_VERSION_REGISTRY, 0);
  if (!registry_) throw std::runtime_error("PipeWire device registry is unavailable");
  pw_registry_add_listener(registry_, &hook_, &kEvents, this);
}

DeviceCatalog::~DeviceCatalog() {
  PipeWireLock lock(context_);
  spa_hook_remove(&hook_);
  pw_proxy_destroy(reinterpret_cast<pw_proxy*>(registry_));
}

std::vector<AudioEndpoint> DeviceCatalog::endpoints() const {
  std::lock_guard lock(mutex_);
  return endpoints_;
}

std::optional<AudioEndpoint> DeviceCatalog::find(std::uint32_t id) const {
  std::lock_guard lock(mutex_);
  const auto it = std::find_if(endpoints_.begin(), endpoints_.end(),
                               [id](const auto& e) { return e.id == id; });
  if (it == endpoints_.end()) return std::nullopt;
  return *it;
}

void DeviceCatalog::on_global(void* data, std::uint32_t id, std::uint32_t,
                              const char* type, std::uint32_t, const spa_dict* props) {
  if (std::strcmp(type, PW_TYPE_INTERFACE_Node) != 0 || !props) return;
  const char* klass = spa_dict_lookup(props, PW_KEY_MEDIA_CLASS);
  if (!klass) return;
  Direction direction;
  if (std::strcmp(klass, "Audio/Source") == 0) direction = Direction::Capture;
  else if (std::strcmp(klass, "Audio/Sink") == 0) direction = Direction::Playback;
  else return;
  const char* node_name = spa_dict_lookup(props, PW_KEY_NODE_NAME);
  const char* description = spa_dict_lookup(props, PW_KEY_NODE_DESCRIPTION);
  const char* serial = spa_dict_lookup(props, PW_KEY_OBJECT_SERIAL);
  const char* channels = spa_dict_lookup(props, PW_KEY_AUDIO_CHANNELS);
  AudioEndpoint endpoint;
  endpoint.id = id;
  endpoint.serial = serial ? std::strtoull(serial, nullptr, 10) : 0;
  endpoint.node_name = node_name ? node_name : std::to_string(id);
  endpoint.name = description && *description ? description : endpoint.node_name;
  endpoint.direction = direction;
  const unsigned count = channels ? std::min(32u, static_cast<unsigned>(std::strtoul(channels, nullptr, 10))) : 0;
  for (unsigned i = 0; i < count; ++i) endpoint.channels.push_back("Channel " + std::to_string(i + 1));
  auto* self = static_cast<DeviceCatalog*>(data);
  std::lock_guard lock(self->mutex_);
  auto it = std::find_if(self->endpoints_.begin(), self->endpoints_.end(),
                         [id](const auto& e) { return e.id == id; });
  if (it == self->endpoints_.end()) self->endpoints_.push_back(std::move(endpoint));
  else *it = std::move(endpoint);
}

void DeviceCatalog::on_remove(void* data, std::uint32_t id) {
  auto* self = static_cast<DeviceCatalog*>(data);
  std::lock_guard lock(self->mutex_);
  std::erase_if(self->endpoints_, [id](const auto& e) { return e.id == id; });
}

} // namespace bass
