# Bass-sim

A Linux desktop guitar-to-bass prototype with independent PipeWire input and output selection, a monophonic octave-down effect, sound controls, meters, and local presets.

## Quick start

Install a C++23 compiler, CMake 3.22+, PipeWire, Rubber Band, GLFW, OpenGL, and nlohmann/json development files. On a Debian-family system the corresponding packages are typically `build-essential cmake libpipewire-0.3-dev librubberband-dev libglfw3-dev libgl1-mesa-dev nlohmann-json3-dev`. Dear ImGui v1.92.9b and its official GLFW/OpenGL3 backends are included in `third_party/imgui`.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j4
ctest --test-dir build --output-on-failure
./build/bass-sim
```

Select a capture source and channel, select a playback sink, then press **Start**. The app starts with audio stopped and only enables output after verifying the selected PipeWire links. Use **Mute** before changing external hardware connections. Save a sound in the Presets panel; endpoint selections are saved separately.

## Platform and validation

| Platform | Build | PipeWire route probe | Guitar listening | Physical latency |
|---|---|---|---|---|
| Linux desktop, PipeWire 1.6.9, GCC 16.2.1 | Passed | Passed with C-Media capture/playback at 48 kHz, 128 frames | Preferred engine render; live build pending test | Unverified physically; current offline streaming onset is about 85 ms |
| Other Linux configurations | Untested | Untested | Untested | Untested |
| Windows/macOS | No audio backend | Untested | Untested | Untested |

The current octave method shifts the full waveform down an octave and passes offline single-note checks. Chords, attacks, live sound, and 30-minute stability need further testing. The present engine's roughly 85 ms offline streaming onset delay already exceeds the 20 ms physical target; the displayed single-buffer duration is an estimate, not end-to-end latency.

For setup and controls see [user guide](docs/user-guide.md); for module boundaries see [architecture](docs/architecture.md); for dependency versions and licenses see [dependencies](docs/dependencies.md). The detailed [validation results](docs/validation/results.md) distinguish performed checks from open checks.

If no devices appear, check that PipeWire and its session manager are running and refresh the device list. If Start stays muted, inspect the Diagnostics panel for the selected node, negotiated channel, or link error; select available endpoints again and retry. Bluetooth playback may add considerable latency even when a small buffer is requested.
