#pragma once

#include <rubberband/RubberBandStretcher.h>

#include <array>
#include <memory>

namespace bass {

class OctaveShifter {
public:
  void prepare(unsigned sample_rate);
  void reset();
  float process(float sample) noexcept;

private:
  static constexpr unsigned kBlock = 128;
  using Stretcher = RubberBand::RubberBandStretcher;
  std::unique_ptr<Stretcher> stretcher_;
  std::array<float, kBlock> input_{};
  std::array<float, kBlock> scratch_{};
  std::array<float, 16384> output_{};
  unsigned input_count_{};
  unsigned read_{};
  unsigned write_{};
  unsigned queued_{};

  void drain() noexcept;
};

} // namespace bass
