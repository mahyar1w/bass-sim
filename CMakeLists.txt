cmake_minimum_required(VERSION 3.22)
project(BassSim VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(PkgConfig REQUIRED)
pkg_check_modules(PIPEWIRE REQUIRED IMPORTED_TARGET libpipewire-0.3)
pkg_check_modules(RUBBERBAND REQUIRED IMPORTED_TARGET rubberband)
find_package(glfw3 3.3 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(nlohmann_json 3.11 REQUIRED)

add_library(imgui STATIC
  third_party/imgui/imgui.cpp
  third_party/imgui/imgui_draw.cpp
  third_party/imgui/imgui_tables.cpp
  third_party/imgui/imgui_widgets.cpp
  third_party/imgui/backends/imgui_impl_glfw.cpp
  third_party/imgui/backends/imgui_impl_opengl3.cpp)
target_include_directories(imgui PUBLIC third_party/imgui third_party/imgui/backends)
target_link_libraries(imgui PUBLIC glfw OpenGL::GL)

add_library(bass_dsp STATIC
  src/dsp/octave_shifter.cpp
  src/dsp/processor.cpp
  src/dsp/gate.cpp
  src/dsp/compressor.cpp
  src/dsp/saturation.cpp)
target_include_directories(bass_dsp PUBLIC src)
target_link_libraries(bass_dsp PUBLIC PkgConfig::RUBBERBAND)

add_library(bass_audio STATIC
  src/audio/pipewire_context.cpp
  src/audio/pipewire_streams.cpp
  src/audio/stream_bridge.cpp
  src/audio/device_catalog.cpp
  src/audio/route_manager.cpp)
target_include_directories(bass_audio PUBLIC src)
target_link_libraries(bass_audio PUBLIC PkgConfig::PIPEWIRE bass_dsp)

add_library(bass_persistence STATIC
  src/persistence/preset_store.cpp
  src/persistence/config_store.cpp)
target_include_directories(bass_persistence PUBLIC src)
target_link_libraries(bass_persistence PUBLIC nlohmann_json::nlohmann_json bass_dsp)

add_library(bass_app STATIC src/app/engine_controller.cpp)
target_include_directories(bass_app PUBLIC src)
target_link_libraries(bass_app PUBLIC bass_audio bass_persistence)

add_library(bass_ui STATIC
  src/ui/window.cpp
  src/ui/live_view.cpp
  src/ui/audio_setup_view.cpp
  src/ui/diagnostics_view.cpp
  src/ui/sound_view.cpp
  src/ui/meters_view.cpp
  src/ui/preset_view.cpp)
target_include_directories(bass_ui PUBLIC src)
target_link_libraries(bass_ui PUBLIC imgui bass_app)

add_executable(bass-sim src/main.cpp)
target_link_libraries(bass-sim PRIVATE bass_ui)

include(CTest)
if(BUILD_TESTING)
  add_subdirectory(tests)
endif()
