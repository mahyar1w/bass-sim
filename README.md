# Bass-sim

Real-time guitar-to-bass simulator for Linux using PipeWire and Rubber Band pitch shifting.

Turns standard electric guitar input into a bass-like sound using an octave-down DSP pipeline with tone shaping, compression, saturation, and noise gating.

![License](https://img.shields.io/badge/license-Apache--2.0-blue.svg)
![Platform](https://img.shields.io/badge/platform-Linux-orange.svg)
![C++](https://img.shields.io/badge/C%2B%2B-23-purple.svg)

---

## Features

- **PipeWire native audio routing:** Independent capture source/channel selection and playback sink selection.
- **DSP pipeline:**
  - Real-time octave-down pitch shifter (Rubber Band Engine R3)
  - Tone shaping and filtering
  - Dynamics: noise gate and compressor
  - Saturation stage
  - Dry / Wet mix control
- **GUI:** Dear ImGui interface with real-time level meters, spectrum visualization, and diagnostics.
- **Preset management:** Save and load custom DSP presets; persistent audio routing configuration.

---

## Requirements

- **OS:** Linux (with running PipeWire daemon and session manager, e.g., WirePlumber)
- **Compiler:** C++23 capable (GCC 13+ or Clang 17+)
- **Build tool:** CMake 3.22+
- **System packages** (Debian / Ubuntu):
  ```sh
  sudo apt install build-essential cmake \
      libpipewire-0.3-dev \
      librubberband-dev \
      libglfw3-dev \
      libgl1-mesa-dev \
      nlohmann-json3-dev
  ```

*Dear ImGui is bundled under `third_party/imgui`.*

---

## Building and Running

```sh
# Configure
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build -j$(nproc)

# Run tests
ctest --test-dir build --output-on-failure

# Launch
./build/bass-sim
```

---

## Usage

1. **Audio setup:** Select your capture device and channel, select your playback sink.
2. **Start engine:** Click **Start**. The app begins processing input audio.
3. **Sound shaping:** Adjust pitch shift, drive, tone, compressor, and gate controls in the sound panel.
4. **Presets:** Save current settings under the Presets panel.
5. **Safety:** Click **Mute** before unplugging or switching hardware endpoints.

For more details on project structure and architecture, see [docs/architecture.md](docs/architecture.md) and [docs/user-guide.md](docs/user-guide.md).

---

## License

Licensed under the [Apache License, Version 2.0](LICENSE).
