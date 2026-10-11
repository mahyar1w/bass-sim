add_executable(bass_tests
  app/engine_state_test.cpp
  app/live_processing_test.cpp
  audio/device_catalog_test.cpp
  audio/route_manager_test.cpp
  dsp/octave_shifter_test.cpp
  dsp/processor_test.cpp
  dsp/effects_test.cpp
  dsp/control_transition_test.cpp
  persistence/preset_test.cpp
  persistence/config_test.cpp)
target_include_directories(bass_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(bass_tests PRIVATE bass_app)
add_test(NAME bass_tests COMMAND bass_tests)

add_executable(live_route_probe audio/live_route_probe.cpp)
target_link_libraries(live_route_probe PRIVATE bass_app)
