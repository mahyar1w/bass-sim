#include "ui/views.h"

#include <imgui.h>

namespace bass {

void draw_sound_view(EngineController& engine) {
  ImGui::Begin("Sound");
  auto p = engine.parameters();
  bool changed = false;
  changed |= ImGui::SliderFloat("Input gain (dB)", &p.input_gain_db, -24.f, 24.f);
  changed |= ImGui::Checkbox("Noise gate", &p.gate_enabled);
  changed |= ImGui::SliderFloat("Gate threshold (dB)", &p.gate_threshold_db, -80.f, -20.f);
  changed |= ImGui::Checkbox("Octave down", &p.octave_enabled);
  changed |= ImGui::SliderFloat("Tone cutoff (Hz)", &p.tone_cutoff_hz, 100.f, 8000.f, "%.0f", ImGuiSliderFlags_Logarithmic);
  changed |= ImGui::Checkbox("Compression", &p.compressor_enabled);
  changed |= ImGui::SliderFloat("Compression amount", &p.compressor_amount, 0.f, 1.f);
  changed |= ImGui::Checkbox("Saturation", &p.saturation_enabled);
  changed |= ImGui::SliderFloat("Saturation amount", &p.saturation_amount, 0.f, 1.f);
  changed |= ImGui::SliderFloat("Wet mix", &p.wet_mix, 0.f, 1.f);
  changed |= ImGui::SliderFloat("Output gain (dB)", &p.output_gain_db, -48.f, 6.f);
  if (changed) engine.set_parameters(p);
  ImGui::TextDisabled("Octave down targets single notes. Chords can produce artifacts.");
  ImGui::End();
}

} // namespace bass
