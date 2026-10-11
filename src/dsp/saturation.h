#pragma once

namespace bass {
class Saturation {
public:
  float process(float x, float amount, bool enabled) const noexcept;
};
}
