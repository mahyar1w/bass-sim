#pragma once

#include <cmath>

namespace bass {

struct Parameters {
  float input_gain_db{0.f};
  bool gate_enabled{false};
  float gate_threshold_db{-60.f};
  bool octave_enabled{true};
  float tone_cutoff_hz{2400.f};
  bool compressor_enabled{false};
  float compressor_amount{};
  bool saturation_enabled{false};
  float saturation_amount{};
  float wet_mix{1.f};
  float output_gain_db{-6.f};
  bool effect_bypass{false};
  bool mute{false};
};

inline bool valid_parameters(const Parameters& p, unsigned sample_rate) noexcept {
  auto between = [](float x, float lo, float hi) {
    return std::isfinite(x) && x >= lo && x <= hi;
  };
  return sample_rate > 0 && between(p.input_gain_db, -24.f, 24.f) &&
         between(p.gate_threshold_db, -80.f, -20.f) &&
         between(p.tone_cutoff_hz, 100.f, 8000.f) && p.tone_cutoff_hz < sample_rate * 0.5f &&
         between(p.compressor_amount, 0.f, 1.f) &&
         between(p.saturation_amount, 0.f, 1.f) &&
         between(p.wet_mix, 0.f, 1.f) && between(p.output_gain_db, -48.f, 6.f);
}

} // namespace bass
