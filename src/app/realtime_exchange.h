#pragma once

#include "app/types.h"
#include "dsp/parameters.h"

#include <atomic>
#include <cstddef>
#include <vector>

namespace bass {

template <typename T> class SpscRing {
public:
  explicit SpscRing(std::size_t capacity) : data_(capacity + 1) {}
  SpscRing(const SpscRing&) = delete;
  SpscRing& operator=(const SpscRing&) = delete;

  bool push(const T& item) noexcept {
    const auto head = head_.load(std::memory_order_relaxed);
    const auto next = (head + 1) % data_.size();
    if (next == tail_.load(std::memory_order_acquire)) return false;
    data_[head] = item;
    head_.store(next, std::memory_order_release);
    return true;
  }

  bool pop(T& item) noexcept {
    const auto tail = tail_.load(std::memory_order_relaxed);
    if (tail == head_.load(std::memory_order_acquire)) return false;
    item = data_[tail];
    tail_.store((tail + 1) % data_.size(), std::memory_order_release);
    return true;
  }

  void clear() noexcept { T item{}; while (pop(item)) {} }
  std::size_t size() const noexcept {
    const auto head = head_.load(std::memory_order_acquire);
    const auto tail = tail_.load(std::memory_order_acquire);
    return head >= tail ? head - tail : data_.size() - tail + head;
  }

private:
  std::vector<T> data_;
  alignas(64) std::atomic<std::size_t> head_{};
  alignas(64) std::atomic<std::size_t> tail_{};
};

struct RealtimeExchange {
  explicit RealtimeExchange(std::size_t audio_frames) : audio(audio_frames), controls(32), meters(128) {}
  SpscRing<float> audio;
  SpscRing<Parameters> controls;
  SpscRing<MeterSnapshot> meters;
};

} // namespace bass
