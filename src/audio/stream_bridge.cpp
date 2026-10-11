#include "audio/stream_bridge.h"

#include <algorithm>
#include <cmath>

namespace bass {

StreamBridge::StreamBridge() : exchange_(8192) {}

void StreamBridge::prepare(unsigned input_rate, unsigned output_rate) {
  exchange_.audio.clear();
  exchange_.controls.clear();
  exchange_.meters.clear();
  processor_.prepare(output_rate);
  base_ratio_ = static_cast<double>(std::max(1u, input_rate)) / std::max(1u, output_rate);
  phase_ = 0.;
  previous_ = next_ = 0.f;
  primed_ = false;
  meter_frames_ = 0;
  input_peak_ = output_peak_ = 0.f;
  underruns_ = 0;
  overruns_ = 0;
}

bool StreamBridge::submit_parameters(const Parameters& parameters) noexcept {
  return exchange_.controls.push(parameters);
}

void StreamBridge::capture(float sample) noexcept {
  if (!exchange_.audio.push(std::isfinite(sample) ? sample : 0.f)) ++overruns_;
}

float StreamBridge::render() noexcept {
  Parameters parameters;
  while (exchange_.controls.pop(parameters)) processor_.set_parameters(parameters);
  if (!primed_) {
    if (exchange_.audio.size() < 128) return 0.f;
    exchange_.audio.pop(previous_);
    exchange_.audio.pop(next_);
    primed_ = true;
  }
  const float input = previous_ + (next_ - previous_) * static_cast<float>(phase_);
  const float output = processor_.process(input);
  input_peak_ = std::max(input_peak_, std::abs(input));
  output_peak_ = std::max(output_peak_, std::abs(output));
  if (++meter_frames_ >= 128) {
    MeterSnapshot meter;
    meter.input_peak = input_peak_;
    meter.output_peak = output_peak_;
    meter.input_clipped = input_peak_ >= 1.f;
    meter.output_clipped = output_peak_ >= 0.89125f;
    meter.underruns = underruns_.load(std::memory_order_relaxed);
    meter.overruns = overruns_.load(std::memory_order_relaxed);
    exchange_.meters.push(meter);
    meter_frames_ = 0;
    input_peak_ = output_peak_ = 0.f;
  }
  const double error = static_cast<double>(exchange_.audio.size()) - 128.;
  const double adjustment = std::clamp(error * 0.000002, -0.005, 0.005);
  phase_ += base_ratio_ * (1. + adjustment);
  while (phase_ >= 1.) {
    phase_ -= 1.;
    previous_ = next_;
    if (!exchange_.audio.pop(next_)) {
      next_ = 0.f;
      primed_ = false;
      ++underruns_;
      break;
    }
  }
  return output;
}

} // namespace bass
