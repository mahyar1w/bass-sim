#include "persistence/preset_store.h"

#include <nlohmann/json.hpp>

#include <cstdlib>
#include <cctype>
#include <cmath>
#include <fstream>
#include <set>
#include <type_traits>

namespace bass {

namespace {
using nlohmann::json;

template <typename T> void read_field(const json& source, const char* key, T& value,
                                      T low, T high, std::vector<std::string>& warnings) {
  if (!source.contains(key)) { warnings.emplace_back(std::string(key) + " missing; default used"); return; }
  try {
    const auto parsed = source.at(key).get<T>();
    if constexpr (std::is_floating_point_v<T>) {
      if (!std::isfinite(parsed)) throw std::runtime_error("nonfinite");
    }
    if (parsed < low || parsed > high) throw std::runtime_error("out of range");
    value = parsed;
  } catch (...) { warnings.emplace_back(std::string(key) + " invalid; default used"); }
}

void read_bool(const json& source, const char* key, bool& value,
               std::vector<std::string>& warnings) {
  if (!source.contains(key)) { warnings.emplace_back(std::string(key) + " missing; default used"); return; }
  if (!source.at(key).is_boolean()) { warnings.emplace_back(std::string(key) + " invalid; default used"); return; }
  value = source.at(key).get<bool>();
}

bool replace_json(const std::filesystem::path& path, const json& value, std::string& error) {
  try {
    std::filesystem::create_directories(path.parent_path());
    const auto temporary = path.string() + ".tmp";
    {
      std::ofstream output(temporary, std::ios::trunc);
      if (!output) throw std::runtime_error("temporary file could not be opened");
      output << value.dump(2) << '\n';
      output.flush();
      if (!output) throw std::runtime_error("temporary file write failed");
    }
    std::filesystem::rename(temporary, path);
    return true;
  } catch (const std::exception& e) { error = e.what(); return false; }
}

json sound_json(const Parameters& p) {
  return {{"input_gain_db", p.input_gain_db}, {"gate_enabled", p.gate_enabled},
          {"gate_threshold_db", p.gate_threshold_db}, {"octave_enabled", p.octave_enabled},
          {"tone_cutoff_hz", p.tone_cutoff_hz}, {"compressor_enabled", p.compressor_enabled},
          {"compressor_amount", p.compressor_amount}, {"saturation_enabled", p.saturation_enabled},
          {"saturation_amount", p.saturation_amount}, {"wet_mix", p.wet_mix},
          {"output_gain_db", p.output_gain_db}};
}
}

std::filesystem::path settings_directory() {
  if (const char* base = std::getenv("XDG_CONFIG_HOME"); base && *base)
    return std::filesystem::path(base) / "bass-sim";
  if (const char* home = std::getenv("HOME"); home && *home)
    return std::filesystem::path(home) / ".config" / "bass-sim";
  return std::filesystem::current_path() / ".bass-sim";
}

bool valid_preset_name(const std::string& name) {
  if (name.empty() || trim_preset_name(name) != name) return false;
  std::size_t characters = 0;
  for (std::size_t i = 0; i < name.size();) {
    const auto ch = static_cast<unsigned char>(name[i]);
    unsigned width = 0;
    if (ch < 0x80) width = 1;
    else if (ch >= 0xc2 && ch <= 0xdf) width = 2;
    else if (ch >= 0xe0 && ch <= 0xef) width = 3;
    else if (ch >= 0xf0 && ch <= 0xf4) width = 4;
    else return false;
    if (i + width > name.size()) return false;
    for (unsigned j = 1; j < width; ++j)
      if ((static_cast<unsigned char>(name[i + j]) & 0xc0) != 0x80) return false;
    if (width == 3) {
      const auto next = static_cast<unsigned char>(name[i + 1]);
      if ((ch == 0xe0 && next < 0xa0) || (ch == 0xed && next >= 0xa0)) return false;
    }
    if (width == 4) {
      const auto next = static_cast<unsigned char>(name[i + 1]);
      if ((ch == 0xf0 && next < 0x90) || (ch == 0xf4 && next >= 0x90)) return false;
    }
    i += width;
    ++characters;
  }
  return characters <= 80;
}

std::string trim_preset_name(const std::string& name) {
  std::size_t first = 0;
  std::size_t last = name.size();
  while (first < last && std::isspace(static_cast<unsigned char>(name[first]))) ++first;
  while (last > first && std::isspace(static_cast<unsigned char>(name[last - 1]))) --last;
  return name.substr(first, last - first);
}

PresetLoad load_presets(const std::filesystem::path& path) {
  PresetLoad result;
  if (!std::filesystem::exists(path)) {
    result.presets.push_back({"Default Bass", Parameters{}});
    return result;
  }
  try {
    std::ifstream input(path);
    const auto root = json::parse(input);
    if (!root.is_object() || !root.contains("schema_version") ||
        !root.at("schema_version").is_number_integer() ||
        root.at("schema_version").get<int>() != 1 ||
        !root.contains("presets") || !root.at("presets").is_array()) {
      result.compatible = false;
      result.warnings.emplace_back("Unsupported or invalid preset file version");
      return result;
    }
    std::set<std::string> names;
    for (const auto& item : root.at("presets")) {
      if (!item.is_object() || !item.contains("name") || !item.at("name").is_string() ||
          !item.contains("sound") || !item.at("sound").is_object()) {
        result.warnings.emplace_back("Invalid preset entry skipped");
        continue;
      }
      const std::string name = item.at("name").get<std::string>();
      if (!valid_preset_name(name) || !names.insert(name).second) {
        result.warnings.emplace_back("Invalid or duplicate preset name skipped");
        continue;
      }
      const auto& sound = item.at("sound");
      Parameters p;
      read_field(sound, "input_gain_db", p.input_gain_db, -24.f, 24.f, result.warnings);
      read_bool(sound, "gate_enabled", p.gate_enabled, result.warnings);
      read_field(sound, "gate_threshold_db", p.gate_threshold_db, -80.f, -20.f, result.warnings);
      read_bool(sound, "octave_enabled", p.octave_enabled, result.warnings);
      read_field(sound, "tone_cutoff_hz", p.tone_cutoff_hz, 100.f, 8000.f, result.warnings);
      read_bool(sound, "compressor_enabled", p.compressor_enabled, result.warnings);
      read_field(sound, "compressor_amount", p.compressor_amount, 0.f, 1.f, result.warnings);
      read_bool(sound, "saturation_enabled", p.saturation_enabled, result.warnings);
      read_field(sound, "saturation_amount", p.saturation_amount, 0.f, 1.f, result.warnings);
      read_field(sound, "wet_mix", p.wet_mix, 0.f, 1.f, result.warnings);
      read_field(sound, "output_gain_db", p.output_gain_db, -48.f, 6.f, result.warnings);
      result.presets.push_back({name, p});
    }
    if (result.presets.empty()) result.presets.push_back({"Default Bass", Parameters{}});
  } catch (const std::exception& e) {
    result.compatible = false;
    result.warnings.emplace_back(std::string("Preset file could not be read: ") + e.what());
    result.presets.push_back({"Default Bass", Parameters{}});
  }
  return result;
}

bool save_presets(const std::filesystem::path& path, const std::vector<Preset>& presets,
                  std::string& error) {
  try {
    if (std::filesystem::exists(path) && !load_presets(path).compatible) {
      error = "Existing preset file has an unsupported version or is malformed";
      return false;
    }
  } catch (const std::exception& e) {
    error = e.what();
    return false;
  }
  json items = json::array();
  std::set<std::string> names;
  for (const auto& preset : presets) {
    if (!valid_preset_name(preset.name) || !names.insert(preset.name).second ||
        !valid_parameters(preset.sound, 48000)) {
      error = "Preset name or sound value is invalid";
      return false;
    }
    items.push_back({{"name", preset.name}, {"sound", sound_json(preset.sound)}});
  }
  return replace_json(path, {{"schema_version", 1}, {"presets", items}}, error);
}

} // namespace bass
