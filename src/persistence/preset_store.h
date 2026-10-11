#pragma once

#include "dsp/parameters.h"

#include <filesystem>
#include <string>
#include <vector>

namespace bass {

struct Preset {
  std::string name;
  Parameters sound;
};

struct PresetLoad {
  std::vector<Preset> presets;
  std::vector<std::string> warnings;
  bool compatible{true};
};

std::filesystem::path settings_directory();
PresetLoad load_presets(const std::filesystem::path& path);
bool save_presets(const std::filesystem::path& path, const std::vector<Preset>& presets,
                  std::string& error);
bool valid_preset_name(const std::string& name);
std::string trim_preset_name(const std::string& name);

} // namespace bass
