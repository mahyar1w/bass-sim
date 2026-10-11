#include "dsp/processor.h"

#include <algorithm>
#include <cmath>

namespace bass {

namespace {
constexpr float kCeiling = 0.89125094f; // -1 dBFS
float db_gain(float db) noexcept { return std::pow(10.f, db / 20.f); }
}

void Processor::prepare(unsigned sample_rate) {
  sample_rate_ = std::max(1u, sample_rate);
  octave_.prepare(sample_rate_);
  gate_.prepare(sample_rate_);
  compressor_.prepare(sample_rate_);
  lowpass_ = 0.f;
  input_gain_ = db_gain(target_.input_gain_db);
  output_gain_ = db_gain(target_.output_gain_db);
  octave_mix_ = target_.octave_enabled ? 1.f : 0.f;
  wet_mix_ = target_.wet_mix;
  bypass_ = target_.effect_bypass ? 1.f : 0.f;
  mute_gain_ = target_.mute ? 0.f : 1.f;
  smooth_ = 1.f - std::exp(-1.f / (static_cast<float>(sample_rate_) * 0.005f));
  set_parameters(target_);
}

void Processor::set_parameters(const Parameters& parameters) noexcept {
  if (!valid_parameters(parameters, sample_rate_)) return;
  target_ = parameters;
  input_target_ = db_gain(target_.input_gain_db);
  output_target_ = db_gain(target_.output_gain_db);
  gate_threshold_linear_ = db_gain(target_.gate_threshold_db);
  tone_alpha_ = 1.f - std::exp(-6.28318530718f * target_.tone_cutoff_hz / sample_rate_);
}

float Processor::process(float sample) noexcept {
  if (!std::isfinite(sample)) sample = 0.f;
  input_gain_ += (input_target_ - input_gain_) * smooth_;
  output_gain_ += (output_target_ - output_gain_) * smooth_;
  octave_mix_ += ((target_.octave_enabled ? 1.f : 0.f) - octave_mix_) * smooth_;
  wet_mix_ += (target_.wet_mix - wet_mix_) * smooth_;
  bypass_ += ((target_.effect_bypass ? 1.f : 0.f) - bypass_) * smooth_;
  mute_gain_ += ((target_.mute ? 0.f : 1.f) - mute_gain_) * smooth_;

  const float input = sample * input_gain_;
  const float dry = input;

  float wet = gate_.process(input, gate_threshold_linear_, target_.gate_enabled);
  const float pitched = octave_.process(wet);
  wet += (pitched - wet) * octave_mix_;
  lowpass_ += tone_alpha_ * (wet - lowpass_);
  wet = compressor_.process(lowpass_, target_.compressor_amount, target_.compressor_enabled);
  wet = saturation_.process(wet, target_.saturation_amount, target_.saturation_enabled);
  const float blend = dry * (1.f - wet_mix_) + wet * wet_mix_;
  const float output = (blend * (1.f - bypass_) + dry * bypass_) * output_gain_ * mute_gain_;
  return std::isfinite(output) ? std::clamp(output, -kCeiling, kCeiling) : 0.f;
}

} // namespace bass
