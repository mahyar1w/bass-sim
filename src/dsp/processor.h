#pragma once

#include "dsp/compressor.h"
#include "dsp/gate.h"
#include "dsp/octave_shifter.h"
#include "dsp/parameters.h"
#include "dsp/saturation.h"

namespace bass {

class Processor {
public:
  void prepare(unsigned sample_rate);
  void set_parameters(const Parameters& parameters) noexcept;
  float process(float sample) noexcept;
private:
  unsigned sample_rate_{48000};
  Parameters target_{};
  OctaveShifter octave_;
  Gate gate_;
  Compressor compressor_;
  Saturation saturation_;
  float input_gain_{1.f};
  float output_gain_{0.5011872f};
  float octave_mix_{1.f};
  float wet_mix_{1.f};
  float bypass_{};
  float mute_gain_{1.f};
  float lowpass_{};
  float smooth_{};
  float input_target_{1.f};
  float output_target_{0.5011872f};
  float gate_threshold_linear_{0.001f};
  float tone_alpha_{};
};

} // namespace bass
