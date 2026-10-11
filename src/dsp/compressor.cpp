#include "dsp/compressor.h"

#include <algorithm>
#include <cmath>

namespace bass {
void Compressor::prepare(unsigned sample_rate) noexcept {
  const float rate = static_cast<float>(std::max(1u, sample_rate));
  attack_ = 1.f - std::exp(-1.f / (rate * 0.01f));
  release_ = 1.f - std::exp(-1.f / (rate * 0.12f));
  envelope_ = 0.f;
}
float Compressor::process(float x, float amount, bool enabled) noexcept {
  envelope_ += (std::abs(x) - envelope_) * (std::abs(x) > envelope_ ? attack_ : release_);
  if (!enabled) return x;
  const float threshold = 0.35f;
  const float ratio = 1.f + 7.f * std::clamp(amount, 0.f, 1.f);
  const float gain = envelope_ > threshold ?
    (threshold + (envelope_ - threshold) / ratio) / envelope_ : 1.f;
  return x * gain;
}
}
