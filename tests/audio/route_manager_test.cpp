#include "test_support.h"

#include "app/types.h"

void test_route_manager() {
  bass::AudioConfiguration c;
  c.capture_id = 1;
  c.playback_id = 2;
  CHECK(!c.route_verified);
  CHECK(!c.actual_quantum);
  c.actual_rate = 44100;
  CHECK(c.actual_rate != c.requested_rate);
}
