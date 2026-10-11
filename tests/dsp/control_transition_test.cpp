#include "test_support.h"

#include "dsp/processor.h"

#include <cmath>

void test_control_transition() {
  bass::Processor processor;
  processor.prepare(48000);
  bass::Parameters p;
  float previous = 0.f;
  for (int i = 0; i < 4800; ++i) {
    if (i == 2000) { p.mute = true; processor.set_parameters(p); }
    const float value = processor.process(0.2f);
    CHECK(std::abs(value) <= 0.891251f);
    CHECK(std::abs(value - previous) < 0.3f);
    previous = value;
  }
  CHECK(std::abs(previous) < 0.001f);

  bass::Processor controls;
  controls.prepare(48000);
  bass::Parameters active;
  active.octave_enabled = false;
  active.wet_mix = 0.f;
  active.output_gain_db = -12.f;
  controls.set_parameters(active);
  for (int i = 0; i < 10000; ++i) controls.process(0.15f);
  previous = controls.process(0.15f);
  active.input_gain_db = 18.f;
  active.wet_mix = 1.f;
  active.effect_bypass = true;
  active.output_gain_db = 6.f;
  controls.set_parameters(active);
  const float first = controls.process(0.15f);
  CHECK(std::abs(first - previous) < 0.02f);
  for (int i = 0; i < 4800; ++i) {
    const float value = controls.process(0.15f);
    CHECK(std::isfinite(value));
    CHECK(std::abs(value) <= 0.891251f);
  }

  bass::Processor dry_mix;
  bass::Processor bypass;
  dry_mix.prepare(48000);
  bypass.prepare(48000);
  bass::Parameters dry;
  dry.wet_mix = 0.f;
  dry_mix.set_parameters(dry);
  dry.wet_mix = 1.f;
  dry.effect_bypass = true;
  bypass.set_parameters(dry);
  for (int i = 0; i < 24000; ++i) {
    const float input = std::sin(6.2831853f * 110.f * i / 48000.f) * 0.1f;
    const float a = dry_mix.process(input);
    const float b = bypass.process(input);
    if (i > 12000) CHECK(std::abs(a - b) < 0.0001f);
  }

  bass::Processor octave_toggle;
  octave_toggle.prepare(48000);
  bass::Parameters toggled;
  octave_toggle.set_parameters(toggled);
  for (int i = 0; i < 4800; ++i)
    previous = octave_toggle.process(0.1f * std::sin(6.2831853f * 110.f * i / 48000.f));
  toggled.octave_enabled = false;
  octave_toggle.set_parameters(toggled);
  const float after_toggle = octave_toggle.process(0.1f * std::sin(6.2831853f * 110.f * 4800 / 48000.f));
  CHECK(std::abs(after_toggle - previous) < 0.02f);
}
