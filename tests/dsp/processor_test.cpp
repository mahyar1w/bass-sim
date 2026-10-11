#include "test_support.h"

#include "dsp/processor.h"

#include <cmath>

void test_processor() {
  bass::Processor processor;
  processor.prepare(48000);
  for (int i = 0; i < 10000; ++i) {
    const auto result = processor.process(i % 2 ? 10.f : -10.f);
    CHECK(std::isfinite(result));
    CHECK(std::abs(result) <= 0.891251f);
  }
  auto response_energy = [](float input_gain, float tone, float wet_mix, bool bypass) {
    bass::Processor p;
    p.prepare(48000);
    bass::Parameters settings;
    settings.input_gain_db = input_gain;
    settings.tone_cutoff_hz = tone;
    settings.wet_mix = wet_mix;
    settings.effect_bypass = bypass;
    settings.octave_enabled = false;
    p.set_parameters(settings);
    double energy = 0.;
    for (int i = 0; i < 24000; ++i) {
      const float input = 0.03f * std::sin(6.2831853f * 3000.f * i / 48000.f);
      const float output = p.process(input);
      CHECK(std::isfinite(output));
      if (i > 12000) energy += output * output;
    }
    return energy;
  };
  const auto low_gain = response_energy(-12.f, 8000.f, 1.f, false);
  const auto high_gain = response_energy(12.f, 8000.f, 1.f, false);
  CHECK(high_gain > low_gain * 10.);
  const auto low_tone = response_energy(0.f, 100.f, 1.f, false);
  const auto high_tone = response_energy(0.f, 8000.f, 1.f, false);
  CHECK(high_tone > low_tone * 10.);
  CHECK(response_energy(0.f, 100.f, 0.f, false) > low_tone * 10.);
  CHECK(response_energy(0.f, 100.f, 1.f, true) > low_tone * 10.);
}
