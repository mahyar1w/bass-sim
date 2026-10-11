# Validation results — 2026-10-09

| Check | Result | Evidence / limit |
|---|---|---|
| Release configuration and build | Pass | `cmake -S . -B build -DCMAKE_BUILD_TYPE=Release`, `cmake --build build -j4` on Linux, GCC 16.2.1, CMake 4.4.3 |
| Hardware-free suite | Pass | `ctest --test-dir build --output-on-failure`: 1/1 CTest executable passed; internal DSP, app, audio, and persistence checks passed |
| Settled offline half pitch | Pass for sampled cases | 36/36 single-sine cases across 44.1, 48, 96, and 192 kHz within 5%; 16/16 harmonic-rich and 7/7 weak-fundamental cases at 48 kHz passed the shifted-third-harmonic check. [Frequencies and method](latency-feasibility.md). |
| Muted selected route | Pass in one configuration | C-Media capture/playback, 48 kHz, 128-frame observed quantum; effective links verified twice. [Route record](routing.md). |
| Desktop default and other app route isolation | Pass in observed muted probe | `wpctl status` and `pw-link -l` comparison preserved QCY-T13 default and Chrome route. [Route record](routing.md). |
| GUI launch | Pass smoke check | Process remained active until a five-second timeout. Resize and high-DPI appearance unreviewed. |
| Sound controls and persistence | Partial | Automated checks pass. The user preferred the level-matched Rubber Band R3 octave render to the in-house granular and oscillator versions. The live build now uses the same engine; live listening of this build is pending. [Controls](sound-controls.md), [presets](presets.md). |
| 30-minute live stability, CPU, dropout counts | Unverified | No monitored 30-minute session was run. The muted probe is not a stability or sound-quality test. |
| 20 ms physical full-wet loopback | Current algorithm fails feasibility | No loopback hardware available, so no physical median or trial count. The exact streaming processor showed about 85 ms offline onset difference on one recorded attack before hardware latency. [Feasibility](latency-feasibility.md). |
| Ten-user setup and label study | Unverified | No participants tested. [Usability](usability.md). |

The executable commands in the [quickstart](../../specs/001-guitar-to-bass-simulator/quickstart.md) match the build. Functional requirements FR-001–FR-018 have implementations or UI surfaces, but complete acceptance still depends on the manual and hardware checks above. SC-002 is supported for the sampled single-sine cases; the current engine cannot meet the 20 ms SC-003 target. SC-001 and SC-006 remain unverified. Other success criteria that depend on sustained live performance remain open. No physical latency estimate is relabeled as an end-to-end measurement.
