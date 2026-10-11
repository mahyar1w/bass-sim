#pragma once

#include "app/engine_controller.h"
#include "persistence/config_store.h"
#include "persistence/preset_store.h"

#include <array>
#include <filesystem>
#include <string>
#include <vector>

namespace bass {

struct UiState {
  AudioConfiguration draft;
  UserConfig saved_config;
  std::vector<Preset> presets;
  std::filesystem::path config_path;
  std::filesystem::path presets_path;
  std::array<char, 256> preset_name{};
  std::string notice;
  int preset_index{};
  bool confirm_replace{};
  bool restored{};
};

void draw_live_view(EngineController& engine);
void draw_audio_setup_view(EngineController& engine, UiState& ui);
void draw_sound_view(EngineController& engine);
void draw_meters_view(const EngineController& engine);
void draw_diagnostics_view(const EngineController& engine, const UiState& ui);
void draw_preset_view(EngineController& engine, UiState& ui);

} // namespace bass
