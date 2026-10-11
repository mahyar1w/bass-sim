#pragma once

namespace bass {
class Gate {
public:
  void prepare(unsigned sample_rate) noexcept;
  float process(float x, float threshold_linear, bool enabled) noexcept;
private:
  float envelope_{};
  float gain_{1.f};
  float attack_{};
  float release_{};
};
}
