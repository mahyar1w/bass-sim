#pragma once

#include "app/realtime_exchange.h"
#include "dsp/processor.h"

#include <atomic>
#include <cstdint>

namespace bass {

class StreamBridge {
public:
  StreamBridge();
  void prepare(unsigned input_rate, unsigned output_rate);
  bool submit_parameters(const Parameters& parameters) noexcept;
  void capture(float sample) noexcept;
  float render() noexcept;
  bool poll_meter(MeterSnapshot& meter) noexcept { return exchange_.meters.pop(meter); }
  std::uint64_t underruns() const noexcept { return underruns_.load(); }
  std::uint64_t overruns() const noexcept { return overruns_.load(); }

private:
  RealtimeExchange exchange_;
  Processor processor_;
  double base_ratio_{1.0};
  double phase_{};
  float previous_{};
  float next_{};
  bool primed_{};
  unsigned meter_frames_{};
  float input_peak_{};
  float output_peak_{};
  std::atomic<std::uint64_t> underruns_{};
  std::atomic<std::uint64_t> overruns_{};
};

} // namespace bass
