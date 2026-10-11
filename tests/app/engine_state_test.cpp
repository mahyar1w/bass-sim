#include "test_support.h"

#include "app/types.h"
#include "dsp/parameters.h"

#include <iostream>

void test_engine_state() {
  bass::AudioConfiguration config;
  CHECK(!config.route_verified);
  CHECK(!config.actual_rate);
  CHECK(config.requested_rate == 48000);
  CHECK(bass::valid_parameters(bass::Parameters{}, config.requested_rate));
}

int main() {
  try {
    test_engine_state();
    test_live_processing();
    test_device_catalog();
    test_route_manager();
    test_octave_shifter();
    test_processor();
    test_effects();
    test_control_transition();
    test_presets();
    test_config();
    std::cout << "All Bass-sim hardware-free checks passed\n";
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
