#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace bass {

struct UserConfig {
  std::string capture_endpoint_id;
  unsigned capture_channel{};
  std::string playback_endpoint_id;
  unsigned playback_channels{2};
  unsigned requested_sample_rate_hz{48000};
  unsigned requested_buffer_frames{128};
  std::string selected_preset{"Default Bass"};
};

struct ConfigLoad {
  UserConfig value;
  std::vector<std::string> warnings;
  bool compatible{true};
};

ConfigLoad load_config(const std::filesystem::path& path);
bool save_config(const std::filesystem::path& path, const UserConfig& config, std::string& error);

} // namespace bass
