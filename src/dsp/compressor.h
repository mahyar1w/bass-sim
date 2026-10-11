#pragma once

namespace bass {
class Compressor {
public:
  void prepare(unsigned sample_rate) noexcept;
  float process(float x, float amount, bool enabled) noexcept;
private:
  float envelope_{};
  float attack_{};
  float release_{};
};
}
