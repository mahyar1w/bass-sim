#include "dsp/saturation.h"

#include <algorithm>
#include <cmath>

namespace bass {
float Saturation::process(float x, float amount, bool enabled) const noexcept {
  if (!enabled) return x;
  const float mix = std::clamp(amount, 0.f, 1.f);
  const float drive = 1.f + 5.f * mix;
  return x * (1.f - mix) + mix * std::tanh(x * drive) / std::tanh(drive);
}
}
