#include "ui/views.h"

#include <imgui.h>

namespace bass {

void draw_live_view(EngineController& engine) {
  ImGui::Begin("Live");
  const auto state = engine.state();
  const char* label = "Stopped";
  switch (state) {
    case EngineState::Preparing: label = "Preparing"; break;
    case EngineState::Connecting: label = "Connecting"; break;
    case EngineState::Running: label = "Running"; break;
    case EngineState::Reconfiguring: label = "Reconfiguring"; break;
    case EngineState::Stopping: label = "Stopping"; break;
    case EngineState::ErrorMuted: label = "Error (muted)"; break;
    default: break;
  }
  ImGui::Text("Engine: %s", label);
  if (ImGui::Button("Start")) engine.start();
  ImGui::SameLine();
  if (ImGui::Button("Stop")) engine.stop();
  auto settings = engine.parameters();
  if (ImGui::Checkbox("Mute output", &settings.mute)) engine.set_parameters(settings);
  ImGui::SameLine();
  if (ImGui::Checkbox("Effect bypass (dry guitar)", &settings.effect_bypass))
    engine.set_parameters(settings);
  if (state == EngineState::ErrorMuted)
    ImGui::TextWrapped("%s: %s", engine.diagnostic().message.c_str(), engine.diagnostic().action.c_str());
  ImGui::End();
}

} // namespace bass
