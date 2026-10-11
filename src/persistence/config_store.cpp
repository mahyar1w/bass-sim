#include "persistence/config_store.h"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <fstream>

namespace bass {

namespace {
using nlohmann::json;

void read_string(const json& object, const char* key, std::string& target,
                 std::vector<std::string>& warnings) {
  if (!object.contains(key)) { warnings.emplace_back(std::string(key) + " missing; default used"); return; }
  if (!object.at(key).is_string()) { warnings.emplace_back(std::string(key) + " invalid; default used"); return; }
  target = object.at(key).get<std::string>();
}

void read_choice(const json& object, const char* key, unsigned& target,
                 std::initializer_list<unsigned> allowed, std::vector<std::string>& warnings) {
  if (!object.contains(key)) { warnings.emplace_back(std::string(key) + " missing; default used"); return; }
  if (!object.at(key).is_number_unsigned() && !object.at(key).is_number_integer()) {
    warnings.emplace_back(std::string(key) + " invalid; default used"); return;
  }
  try {
    const unsigned value = object.at(key).get<unsigned>();
    if (std::find(allowed.begin(), allowed.end(), value) != allowed.end()) target = value;
    else warnings.emplace_back(std::string(key) + " unsupported; default used");
  } catch (...) { warnings.emplace_back(std::string(key) + " invalid; default used"); }
}
}

ConfigLoad load_config(const std::filesystem::path& path) {
  ConfigLoad result;
  if (!std::filesystem::exists(path)) return result;
  try {
    std::ifstream input(path);
    const auto root = json::parse(input);
    if (!root.is_object() || !root.contains("schema_version") ||
        !root.at("schema_version").is_number_integer() ||
        root.at("schema_version").get<int>() != 1) {
      result.compatible = false;
      result.warnings.emplace_back("Unsupported or malformed user configuration version");
      return result;
    }
    auto& c = result.value;
    read_string(root, "capture_endpoint_id", c.capture_endpoint_id, result.warnings);
    read_string(root, "playback_endpoint_id", c.playback_endpoint_id, result.warnings);
    read_string(root, "selected_preset", c.selected_preset, result.warnings);
    if (root.contains("capture_channel") && root.at("capture_channel").is_number_unsigned())
      c.capture_channel = std::min(31u, root.at("capture_channel").get<unsigned>());
    else result.warnings.emplace_back("capture_channel invalid; default used");
    read_choice(root, "playback_channels", c.playback_channels, {1, 2}, result.warnings);
    read_choice(root, "requested_sample_rate_hz", c.requested_sample_rate_hz,
                {44100, 48000, 96000, 192000}, result.warnings);
    read_choice(root, "requested_buffer_frames", c.requested_buffer_frames,
                {64, 128, 256, 512, 1024}, result.warnings);
  } catch (const std::exception& e) {
    result.compatible = false;
    result.warnings.emplace_back(std::string("User configuration could not be read: ") + e.what());
  }
  return result;
}

bool save_config(const std::filesystem::path& path, const UserConfig& c, std::string& error) {
  try {
    if (std::filesystem::exists(path) && !load_config(path).compatible) {
      error = "Existing configuration has an unsupported version or is malformed";
      return false;
    }
  } catch (const std::exception& e) {
    error = e.what();
    return false;
  }
  try {
    std::filesystem::create_directories(path.parent_path());
    const auto temporary = path.string() + ".tmp";
    {
      std::ofstream output(temporary, std::ios::trunc);
      if (!output) throw std::runtime_error("temporary file could not be opened");
      output << json({{"schema_version", 1},
                      {"capture_endpoint_id", c.capture_endpoint_id},
                      {"capture_channel", c.capture_channel},
                      {"playback_endpoint_id", c.playback_endpoint_id},
                      {"playback_channels", c.playback_channels},
                      {"requested_sample_rate_hz", c.requested_sample_rate_hz},
                      {"requested_buffer_frames", c.requested_buffer_frames},
                      {"selected_preset", c.selected_preset}}).dump(2) << '\n';
      output.flush();
      if (!output) throw std::runtime_error("temporary file write failed");
    }
    std::filesystem::rename(temporary, path);
    return true;
  } catch (const std::exception& e) { error = e.what(); return false; }
}

} // namespace bass
