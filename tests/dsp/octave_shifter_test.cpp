#include "test_support.h"

#include "dsp/octave_shifter.h"

#include <algorithm>
#include <cmath>
#include <complex>
#include <iostream>
#include <vector>

namespace {
void fft(std::vector<std::complex<double>>& samples) {
  const auto n = samples.size();
  for (std::size_t i = 1, j = 0; i < n; ++i) {
    std::size_t bit = n >> 1;
    for (; j & bit; bit >>= 1) j ^= bit;
    j ^= bit;
    if (i < j) std::swap(samples[i], samples[j]);
  }
  for (std::size_t length = 2; length <= n; length <<= 1) {
    const double angle = -6.283185307179586 / static_cast<double>(length);
    const std::complex<double> step(std::cos(angle), std::sin(angle));
    for (std::size_t start = 0; start < n; start += length) {
      std::complex<double> rotation(1., 0.);
      for (std::size_t j = 0; j < length / 2; ++j) {
        const auto even = samples[start + j];
        const auto odd = samples[start + j + length / 2] * rotation;
        samples[start + j] = even + odd;
        samples[start + j + length / 2] = even - odd;
        rotation *= step;
      }
    }
  }
}

double dominant_frequency(double input_frequency, unsigned rate, double preceding = 0.,
                          bool harmonic_rich = false, double* purity = nullptr,
                          double harmonic_phase = 0.3, bool weak_fundamental = false,
                          double* shifted_third_ratio = nullptr) {
  bass::OctaveShifter shifter;
  shifter.prepare(rate);
  if (preceding > 0.)
    for (std::size_t i = 0; i < rate / 2; ++i)
      shifter.process(static_cast<float>(std::sin(6.283185307179586 * preceding * i / rate)));
  const std::size_t count = rate >= 192000 ? 131072 : rate >= 96000 ? 65536 : 32768;
  std::vector<std::complex<double>> spectrum(count);
  const std::size_t skip = rate / 4;
  for (std::size_t i = 0; i < skip + count; ++i) {
    const double angle = 6.283185307179586 * input_frequency * i / rate;
    const double input = weak_fundamental ?
      0.22 * std::sin(angle) + 0.38 * std::sin(2 * angle + harmonic_phase) +
      0.75 * std::sin(3 * angle - harmonic_phase * 0.67) +
      0.55 * std::sin(5 * angle + 0.2) : harmonic_rich ?
      0.45 * std::sin(angle) + 0.55 * std::sin(2 * angle + harmonic_phase) +
      0.25 * std::sin(3 * angle - harmonic_phase * 0.67) + 0.08 : std::sin(angle);
    const double output = shifter.process(static_cast<float>(input));
    if (i >= skip) {
      const std::size_t index = i - skip;
      const double window = 0.5 - 0.5 * std::cos(6.283185307179586 * index / (count - 1));
      spectrum[index] = output * window;
    }
  }
  fft(spectrum);
  std::size_t peak = 0;
  double level = 0.;
  for (std::size_t i = 1; i < count / 2; ++i) {
    const double frequency = static_cast<double>(i) * rate / count;
    if (frequency < 25. || frequency > 500.) continue;
    const double magnitude = std::norm(spectrum[i]);
    if (magnitude > level) { level = magnitude; peak = i; }
  }
  if (purity) {
    double total = 0., desired = 0.;
    const auto target_bin = static_cast<long>(std::lround(input_frequency * 0.5 * count / rate));
    for (std::size_t i = 1; i < count / 2; ++i) {
      const double frequency = static_cast<double>(i) * rate / count;
      if (frequency < 25. || frequency > 1000.) continue;
      const double magnitude = std::norm(spectrum[i]);
      total += magnitude;
      if (std::abs(static_cast<long>(i) - target_bin) <= 3) desired += magnitude;
    }
    *purity = total > 0. ? desired / total : 0.;
  }
  if (shifted_third_ratio) {
    auto energy_near = [&](double frequency) {
      const auto center = static_cast<long>(std::lround(frequency * count / rate));
      double energy = 0.;
      for (long bin = std::max(1L, center - 3); bin <= center + 3; ++bin)
        if (bin < static_cast<long>(count / 2)) energy += std::norm(spectrum[bin]);
      return energy;
    };
    *shifted_third_ratio = energy_near(input_frequency * 1.5) /
                           std::max(energy_near(input_frequency * 3.), 1e-12);
  }
  return static_cast<double>(peak) * rate / count;
}
}

void test_octave_shifter() {
  bass::OctaveShifter shifter;
  shifter.prepare(48000);
  for (int i = 0; i < 2000; ++i) CHECK(std::isfinite(shifter.process(0.f)));
  float energy = 0.f;
  for (int i = 0; i < 24000; ++i) {
    const float value = shifter.process(std::sin(6.2831853f * 82.4f * i / 48000.f));
    if (i > 12000) energy += value * value;
  }
  CHECK(energy > 100.f);
  float tail = 1.f;
  for (int i = 0; i < 9600; ++i) tail = shifter.process(0.f);
  CHECK(std::abs(tail) < 0.001f);
  shifter.reset();
  for (int i = 0; i < 2000; ++i) CHECK(std::isfinite(shifter.process(0.f)));
  CHECK(std::isfinite(shifter.process(1.f)));
  for (int i = 0; i < 2000; ++i) CHECK(std::isfinite(shifter.process(0.f)));
  const double notes[] = {82.4069, 110., 146.832, 196., 246.942, 329.628,
                          415.305, 659.255, 987.767};
  unsigned accurate = 0;
  for (const unsigned rate : {44100u, 48000u, 96000u, 192000u}) {
    for (const auto note : notes) {
      const double measured = dominant_frequency(note, rate);
      std::cerr << "pitch " << rate << " Hz: " << note << " -> " << measured << '\n';
      if (std::abs(measured - note * 0.5) <= note * 0.025) ++accurate;
    }
  }
  CHECK(accurate >= 35); // At least 95% of 36 documented steady-note cases.
  const auto changed = dominant_frequency(196., 48000, 110.);
  CHECK(std::abs(changed - 98.) < 4.9);
  for (double note : {82.4069, 110., 196., 329.628})
    for (double phase : {0., 1.57, 3.14, 4.71}) {
      double purity = 0.;
      double shifted_third = 0.;
      dominant_frequency(note, 48000, 0., true, &purity, phase, false, &shifted_third);
      CHECK(purity > 0.05);
      CHECK(shifted_third > 4.);
    }
  for (double note : {82.4069, 93.75, 110., 131., 186., 415.305, 659.255}) {
    double purity = 0.;
    double shifted_third = 0.;
    dominant_frequency(note, 48000, 0., false, &purity, 0.3, true, &shifted_third);
    CHECK(purity > 0.01);
    CHECK(shifted_third > 4.);
  }
  for (unsigned block : {64u, 128u, 512u, 1024u}) {
    bass::OctaveShifter streamed;
    streamed.prepare(48000);
    for (unsigned start = 0; start < 4096; start += block)
      for (unsigned i = start; i < std::min(4096u, start + block); ++i)
        CHECK(std::isfinite(streamed.process(
          std::sin(6.2831853f * 110.f * i / 48000.f))));
  }
}
