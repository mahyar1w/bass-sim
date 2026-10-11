#include "ui/views.h"

#include <imgui.h>

#include <algorithm>

namespace bass {

void draw_meters_view(const EngineController& engine) {
  ImGui::Begin("Levels");
  const auto& m = engine.meter();
  static bool input_clipped = false;
  static bool output_clipped = false;
  input_clipped |= m.input_clipped;
  output_clipped |= m.output_clipped;
  ImGui::TextUnformatted("Input");
  ImGui::ProgressBar(std::clamp(m.input_peak, 0.f, 1.f));
  ImGui::TextUnformatted("Output");
  ImGui::ProgressBar(std::clamp(m.output_peak, 0.f, 1.f));
  if (input_clipped) ImGui::TextUnformatted("Input clipping detected: lower input gain");
  if (output_clipped) ImGui::TextUnformatted("Output reached protection ceiling");
  if ((input_clipped || output_clipped) && ImGui::Button("Clear clipping indicators"))
    input_clipped = output_clipped = false;
  ImGui::End();
}

} // namespace bass
