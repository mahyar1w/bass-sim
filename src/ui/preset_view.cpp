#include "ui/views.h"

#include <imgui.h>

#include <algorithm>
#include <cstring>

namespace bass {

void draw_preset_view(EngineController& engine, UiState& ui) {
  ImGui::Begin("Presets");
  const char* selected = ui.presets.empty() ? "No presets" :
                         ui.presets[std::clamp(ui.preset_index, 0,
                           static_cast<int>(ui.presets.size()) - 1)].name.c_str();
  if (ImGui::BeginCombo("Saved sounds", selected)) {
    for (int i = 0; i < static_cast<int>(ui.presets.size()); ++i) {
      if (ImGui::Selectable(ui.presets[i].name.c_str(), i == ui.preset_index)) ui.preset_index = i;
    }
    ImGui::EndCombo();
  }
  if (ImGui::Button("Load preset") && !ui.presets.empty()) {
    auto p = ui.presets[ui.preset_index].sound;
    p.mute = engine.parameters().mute;
    p.effect_bypass = engine.parameters().effect_bypass;
    if (engine.set_parameters(p)) {
      ui.saved_config.selected_preset = ui.presets[ui.preset_index].name;
      std::string error;
      if (!save_config(ui.config_path, ui.saved_config, error)) ui.notice = "Settings save failed: " + error;
      else ui.notice = "Preset loaded";
    } else ui.notice = "Preset values do not fit the current audio rate";
  }
  ImGui::InputText("Preset name", ui.preset_name.data(), ui.preset_name.size());
  if (ImGui::Button("Save current sound")) {
    const std::string name = trim_preset_name(ui.preset_name.data());
    if (!valid_preset_name(name)) ui.notice = "Name must be nonempty and at most 80 characters";
    else {
      const auto it = std::find_if(ui.presets.begin(), ui.presets.end(),
                                   [&](const Preset& preset) { return preset.name == name; });
      if (it != ui.presets.end()) ui.confirm_replace = true;
      else {
        auto p = engine.parameters();
        p.mute = false;
        p.effect_bypass = false;
        ui.presets.push_back({name, p});
        std::string error;
        if (!save_presets(ui.presets_path, ui.presets, error)) {
          ui.presets.pop_back();
          ui.notice = "Preset save failed: " + error;
        } else {
          ui.preset_index = static_cast<int>(ui.presets.size()) - 1;
          ui.notice = "Preset saved";
        }
      }
    }
  }
  if (ui.confirm_replace) {
    ImGui::TextUnformatted("A preset with this name exists. Replace it?");
    if (ImGui::Button("Replace preset")) {
      const std::string name = trim_preset_name(ui.preset_name.data());
      const auto it = std::find_if(ui.presets.begin(), ui.presets.end(),
                                   [&](const Preset& preset) { return preset.name == name; });
      if (it != ui.presets.end()) {
        auto old = it->sound;
        it->sound = engine.parameters();
        it->sound.mute = false;
        it->sound.effect_bypass = false;
        std::string error;
        if (!save_presets(ui.presets_path, ui.presets, error)) {
          it->sound = old;
          ui.notice = "Preset replacement failed: " + error;
        } else ui.notice = "Preset replaced";
      }
      ui.confirm_replace = false;
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel replacement")) ui.confirm_replace = false;
  }
  ImGui::End();
}

} // namespace bass
