#include "test_support.h"

#include "app/types.h"

void test_device_catalog() {
  bass::AudioEndpoint a{1, 10, "Interface", "capture.one", bass::Direction::Capture,
                        {"Channel 1"}, bass::Availability::Present};
  bass::AudioEndpoint b{2, 11, "Interface", "sink.two", bass::Direction::Playback,
                        {"Channel 1", "Channel 2"}, bass::Availability::Present};
  CHECK(a.name == b.name);
  CHECK(a.id != b.id);
  CHECK(a.direction != b.direction);
  CHECK(b.channels.size() == 2);
}
