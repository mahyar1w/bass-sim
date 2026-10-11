#include "test_support.h"

#include "dsp/compressor.h"
#include "dsp/gate.h"
#include "dsp/saturation.h"
#include "dsp/parameters.h"

#include <cmath>
#include <limits>

void test_effects() {
  for (unsigned rate : {44100u, 48000u, 96000u, 192000u}) {
    bass::Gate gate;
    gate.prepare(rate);
    bass::Compressor compressor;
    compressor.prepare(rate);
    bass::Saturation saturation;
    for (int block : {64, 128, 512}) {
      for (int i = 0; i < block; ++i) {
        const float input = i == 0 ? 1.f : i % 2 ? 0.f : 0.7f;
        for (float amount : {0.f, 1.f}) {
          CHECK(std::isfinite(gate.process(input, 0.01f, true)));
          CHECK(std::isfinite(compressor.process(input, amount, true)));
          CHECK(std::isfinite(saturation.process(input, amount, true)));
        }
      }
    }
  }
  bass::Parameters p;
  p.input_gain_db = 25.f;
  CHECK(!bass::valid_parameters(p, 48000));
  p = {};
  p.wet_mix = std::numeric_limits<float>::quiet_NaN();
  CHECK(!bass::valid_parameters(p, 48000));
  p = {};
  p.tone_cutoff_hz = 8000.f;
  CHECK(!bass::valid_parameters(p, 16000));
  CHECK(bass::valid_parameters(p, 44100));
}
