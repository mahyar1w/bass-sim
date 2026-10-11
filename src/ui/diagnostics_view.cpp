#include "ui/views.h"

#include <imgui.h>

namespace bass {

void draw_diagnostics_view(const EngineController& engine, const UiState& ui) {
  ImGui::Begin("Diagnostics");
  const auto& message = engine.diagnostic();
  ImGui::TextWrapped("%s", message.message.c_str());
  if (!message.action.empty()) ImGui::TextWrapped("Action: %s", message.action.c_str());
  if (!ui.notice.empty()) ImGui::TextWrapped("%s", ui.notice.c_str());
  const auto& config = engine.configuration();
  ImGui::TextUnformatted("Backend-reported input/output latency: unavailable");
  if (config.actual_rate && config.actual_quantum) {
    const double buffer_ms = 1000. * *config.actual_quantum / *config.actual_rate;
    ImGui::Text("Estimated single buffer duration: %.2f ms (current settings)", buffer_ms);
  } else ImGui::TextUnformatted("Estimated buffer duration: unavailable");
  ImGui::TextWrapped("Measured physical input-to-output latency: unavailable; no loopback measurement setup or measurement date");
  if (engine.meter_available()) {
    ImGui::Text("Observed underruns: %llu", static_cast<unsigned long long>(engine.meter().underruns));
    ImGui::Text("Observed overruns: %llu", static_cast<unsigned long long>(engine.meter().overruns));
  } else ImGui::TextUnformatted("Underrun/overrun observations: unavailable");
  ImGui::End();
}

} // namespace bass
