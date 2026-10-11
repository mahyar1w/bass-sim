#pragma once

#include <stdexcept>
#include <string>

#define CHECK(condition) do { if (!(condition)) throw std::runtime_error( \
  std::string(__FILE__) + ":" + std::to_string(__LINE__) + ": " + #condition); } while (false)

void test_engine_state();
void test_live_processing();
void test_device_catalog();
void test_route_manager();
void test_octave_shifter();
void test_processor();
void test_effects();
void test_control_transition();
void test_presets();
void test_config();
