#include "dsp/octave_shifter.h"

#include <algorithm>
#include <cmath>

namespace bass {

void OctaveShifter::prepare(unsigned sample_rate) {
  const auto options = Stretcher::OptionProcessRealTime |
                       Stretcher::OptionEngineFiner |
                       Stretcher::OptionWindowShort;
  stretcher_ = std::make_unique<Stretcher>(std::max(1u, sample_rate), 1, options, 1., 0.5);
  stretcher_->setMaxProcessSize(kBlock);
  reset();
}

void OctaveShifter::reset() {
  input_count_ = read_ = write_ = queued_ = 0;
  input_.fill(0.f);
  output_.fill(0.f);
  if (!stretcher_) return;
  stretcher_->reset();
}

float OctaveShifter::process(float sample) noexcept {
  if (!stretcher_) return 0.f;
  input_[input_count_++] = std::isfinite(sample) ? sample : 0.f;
  if (input_count_ == kBlock) {
    const float* input[] = {input_.data()};
    stretcher_->process(input, kBlock, false);
    input_count_ = 0;
    drain();
  }
  if (!queued_) return 0.f;
  const float value = output_[read_];
  read_ = (read_ + 1) % output_.size();
  --queued_;
  return std::isfinite(value) ? value : 0.f;
}

void OctaveShifter::drain() noexcept {
  float* output[] = {scratch_.data()};
  for (;;) {
    const int available = stretcher_->available();
    if (available <= 0) break;
    const auto count = std::min<unsigned>(static_cast<unsigned>(available), kBlock);
    const auto received = stretcher_->retrieve(output, count);
    if (!received) break;
    for (unsigned i = 0; i < received; ++i) {
      if (queued_ == output_.size()) {
        read_ = (read_ + 1) % output_.size();
        --queued_;
      }
      output_[write_] = scratch_[i];
      write_ = (write_ + 1) % output_.size();
      ++queued_;
    }
  }
}

} // namespace bass
