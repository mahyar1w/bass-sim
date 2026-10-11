#include "test_support.h"

#include "audio/stream_bridge.h"

#include <cmath>

void test_live_processing() {
  bass::StreamBridge bridge;
  bridge.prepare(48000, 48000);
  bass::Parameters p;
  p.mute = true;
  CHECK(bridge.submit_parameters(p));
  for (int i = 0; i < 4000; ++i) bridge.capture(std::sin(i * 0.03f) * 0.2f);
  float last = 1.f;
  for (int i = 0; i < 2000; ++i) {
    last = bridge.render();
    CHECK(std::abs(last) <= 0.891251f);
  }
  CHECK(std::abs(last) < 0.001f);
  bass::MeterSnapshot meter;
  CHECK(bridge.poll_meter(meter));
  bridge.prepare(48000, 48000); // Simulated stop/restart resets queued audio.
  for (int i = 0; i < 100; ++i) CHECK(bridge.render() == 0.f);
  CHECK(!bridge.poll_meter(meter));
}
