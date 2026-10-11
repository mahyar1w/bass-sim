#include "ui/views.h"

#include <imgui.h>

#include <algorithm>
#include <string>

namespace bass {

namespace {
void select_endpoint(const char* label, Direction direction,
                     const std::vector<AudioEndpoint>& endpoints, std::uint32_t& id) {
  std::string selected = "Select device";
  for (const auto& e : endpoints) if (e.id == id) selected = e.name;
  if (ImGui::BeginCombo(label, selected.c_str())) {
    for (const auto& e : endpoints) {
      if (e.direction != direction) continue;
      const std::string item = e.name + "##" + std::to_string(e.id);
      if (ImGui::Selectable(item.c_str(), id == e.id)) id = e.id;
    }
    ImGui::EndCombo();
  }
}

void select_number(const char* label, unsigned& value, const unsigned* options,
                   std::size_t count) {
  std::string current = std::to_string(value);
  if (ImGui::BeginCombo(label, current.c_str())) {
    for (std::size_t i = 0; i < count; ++i) {
      const std::string text = std::to_string(options[i]);
      if (ImGui::Selectable(text.c_str(), value == options[i])) value = options[i];
    }
    ImGui::EndCombo();
  }
}
}

void draw_audio_setup_view(EngineController& engine, UiState& ui) {
  ImGui::Begin("Audio setup");
  if (ImGui::Button("Refresh devices")) engine.refresh();
  const auto endpoints = engine.endpoints();
  select_endpoint("Guitar input", Direction::Capture, endpoints, ui.draft.capture_id);
  select_endpoint("Playback output", Direction::Playback, endpoints, ui.draft.playback_id);
  int channel = static_cast<int>(ui.draft.capture_channel + 1);
  if (ImGui::InputInt("Input channel", &channel))
    ui.draft.capture_channel = static_cast<unsigned>(std::clamp(channel, 1, 32) - 1);
  const unsigned modes[] = {1, 2};
  select_number("Output channels", ui.draft.playback_channels, modes, 2);
  const unsigned rates[] = {44100, 48000, 96000, 192000};
  select_number("Requested sample rate (Hz)", ui.draft.requested_rate, rates, 4);
  const unsigned buffers[] = {64, 128, 256, 512, 1024};
  select_number("Requested buffer (frames)", ui.draft.requested_quantum, buffers, 5);
  if (ImGui::Button("Apply audio settings")) {
    engine.configure(ui.draft);
    for (const auto& e : endpoints) {
      if (e.id == ui.draft.capture_id) ui.saved_config.capture_endpoint_id = e.node_name;
      if (e.id == ui.draft.playback_id) ui.saved_config.playback_endpoint_id = e.node_name;
    }
    ui.saved_config.capture_channel = ui.draft.capture_channel;
    ui.saved_config.playback_channels = ui.draft.playback_channels;
    ui.saved_config.requested_sample_rate_hz = ui.draft.requested_rate;
    ui.saved_config.requested_buffer_frames = ui.draft.requested_quantum;
    std::string error;
    if (!save_config(ui.config_path, ui.saved_config, error)) ui.notice = "Settings save failed: " + error;
    else ui.notice = "Audio settings applied";
  }
  const auto& actual = engine.configuration();
  ImGui::Text("Accepted rate: %s", actual.actual_rate ? std::to_string(*actual.actual_rate).c_str() : "Unknown");
  ImGui::Text("Accepted buffer: %s", actual.actual_quantum ? std::to_string(*actual.actual_quantum).c_str() : "Unknown");
  ImGui::Text("Route verified: %s", actual.route_verified ? "Yes" : "No");
  ImGui::End();
}

} // namespace bass
