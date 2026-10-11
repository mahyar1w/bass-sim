#include "ui/views.h"
#include "ui/window.h"

#include <imgui.h>

#include <algorithm>
#include <exception>
#include <iostream>

int main() {
  try {
    bass::Window window;
    bass::EngineController engine;
    bass::UiState ui;
    ui.config_path = bass::settings_directory() / "config.json";
    ui.presets_path = bass::settings_directory() / "presets.json";
    auto config = bass::load_config(ui.config_path);
    auto presets = bass::load_presets(ui.presets_path);
    ui.saved_config = config.value;
    ui.presets = std::move(presets.presets);
    if (!config.warnings.empty()) ui.notice = config.warnings.front();
    if (!presets.warnings.empty()) ui.notice = presets.warnings.front();
    ui.draft.capture_channel = ui.saved_config.capture_channel;
    ui.draft.playback_channels = ui.saved_config.playback_channels;
    ui.draft.requested_rate = ui.saved_config.requested_sample_rate_hz;
    ui.draft.requested_quantum = ui.saved_config.requested_buffer_frames;
    for (int i = 0; i < static_cast<int>(ui.presets.size()); ++i)
      if (ui.presets[i].name == ui.saved_config.selected_preset) ui.preset_index = i;
    if (!ui.presets.empty() &&
        !engine.set_parameters(ui.presets[ui.preset_index].sound))
      ui.notice = "Saved sound does not fit the requested audio rate; defaults are active";
    while (window.open()) {
      if (!ui.restored) {
        const auto endpoints = engine.endpoints();
        if (!endpoints.empty()) {
          for (const auto& e : endpoints) {
            if (e.node_name == ui.saved_config.capture_endpoint_id && e.direction == bass::Direction::Capture)
              ui.draft.capture_id = e.id;
            if (e.node_name == ui.saved_config.playback_endpoint_id && e.direction == bass::Direction::Playback)
              ui.draft.playback_id = e.id;
          }
          engine.configure(ui.draft);
          ui.restored = true;
        }
      }
      engine.tick();
      window.begin_frame();
      const auto* viewport = ImGui::GetMainViewport();
      const float gap = 10.f;
      const float left_width = viewport->WorkSize.x * 0.39f;
      const float right_x = viewport->WorkPos.x + left_width + gap;
      const float right_width = viewport->WorkSize.x - left_width - gap;
      const float top = viewport->WorkPos.y;
      const float height = viewport->WorkSize.y;
      auto place = [&](float x, float y, float width, float panel_height) {
        ImGui::SetNextWindowPos(ImVec2(x, y), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(width, panel_height), ImGuiCond_Always);
      };
      place(viewport->WorkPos.x, top, left_width, height * 0.24f - gap);
      bass::draw_live_view(engine);
      place(viewport->WorkPos.x, top + height * 0.24f, left_width, height * 0.52f - gap);
      bass::draw_audio_setup_view(engine, ui);
      place(right_x, top, right_width, height * 0.54f - gap);
      bass::draw_sound_view(engine);
      place(right_x, top + height * 0.54f, right_width, height * 0.16f - gap);
      bass::draw_meters_view(engine);
      place(right_x, top + height * 0.70f, right_width, height * 0.30f);
      bass::draw_preset_view(engine, ui);
      place(viewport->WorkPos.x, top + height * 0.76f, left_width, height * 0.24f);
      bass::draw_diagnostics_view(engine, ui);
      window.end_frame();
    }
    engine.stop();
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
