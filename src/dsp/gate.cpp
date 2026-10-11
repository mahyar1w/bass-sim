#include "dsp/gate.h"

#include <algorithm>
#include <cmath>

namespace bass {
void Gate::prepare(unsigned sample_rate) noexcept {
  const float rate = static_cast<float>(std::max(1u, sample_rate));
  attack_ = 1.f - std::exp(-1.f / (rate * 0.005f));
  release_ = 1.f - std::exp(-1.f / (rate * 0.08f));
  envelope_ = 0.f;
  gain_ = 1.f;
}
float Gate::process(float x, float threshold_linear, bool enabled) noexcept {
  envelope_ += (std::abs(x) - envelope_) * (std::abs(x) > envelope_ ? attack_ : release_);
  const float target = !enabled || envelope_ >= threshold_linear ? 1.f : 0.f;
  gain_ += (target - gain_) * (target > gain_ ? attack_ : release_);
  return x * gain_;
}
}
