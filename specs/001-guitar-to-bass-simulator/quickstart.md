# Quickstart and End-to-End Validation

This guide is the validation target for the implementation. The build/run commands below are runnable on the tested Linux setup. Record actual observations; do not report manual scenarios as passed before running them.

## Prerequisites

- Linux desktop with a running PipeWire service and compatible session manager, a mono guitar capture path, and an audible output.
- C++23 compiler, CMake, and the dependencies and tested versions that the implementation's README will specify.
- For latency measurement, a reproducible hardware loopback path or equivalent measurement equipment.

## Build and launch

From the repository root after implementation:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
./build/bass-sim
```

Expected: the application opens, the automated suite passes, and no audio route starts until the user selects valid endpoints and presses Start. If the generated executable path differs, the README and this guide must be updated together.

## Scenario 1: Live bass sound and route isolation

1. Record the current system default input/output and any other application routes using the desktop audio control or PipeWire graph inspector.
2. In Bass-sim, refresh endpoint lists, select the guitar input channel and a playback endpoint, and request a supported sample rate and buffer size.
3. Press Start. Confirm the effective capture/playback links match the chosen endpoints before unmuting. Play single notes, including open low E, short attacks, and sustained notes.
4. Observe input/output meters, clipping, engine state, requested versus accepted settings, and latency labels.
5. Inspect the desktop defaults and the other application's routes again.

Expected: a controlled octave-down bass-like sound, truthful actual settings, no change to system defaults or other routes, and no unexplained link to a different endpoint. Record the devices, accepted settings, CPU context, and listening observations.

## Scenario 2: Controls and safe transitions

1. While holding a note, change input gain, tone, wet/dry mix, output gain, gate, compressor, and saturation within their UI bounds.
2. Toggle each optional effect, full effect bypass, and mute. Load another preset; stop and restart processing.
3. Deliberately present a hot input at a safe monitoring level and inspect clipping indicators and final output protection.

Expected: no click or loud burst from control or state changes; bypass plays controlled dry guitar, mute is silent, and the output remains bounded. Record any audible artifact and its control/setting.

## Scenario 3: Recovery and saved data

1. Disconnect the selected input, then the selected output, while processing. Reconnect and use the displayed recovery action.
2. Request an unsupported device combination or audio setting, if the UI permits the attempt.
3. Save and reload a preset, then restart the application. Separately test missing, malformed, future-version, and out-of-range saved values.

Expected: affected processing mutes safely, state and error messages identify the cause, and the user can recover without an application restart once devices are available. Invalid saved data uses safe defaults or keeps the previous valid live state. See [persistence.md](contracts/persistence.md).

## Scenario 4: Automated signal and stability checks

The CTest suite must cover silence, impulses, sustained and changing chromatic single-note tones including low E, multiple supported rates and block sizes, control extremes, parameter smoothing, finite/bounded output, saved-data validation, and engine state transitions. For pitch accuracy, analyze after a declared settling interval and record the analysis method and tolerance; at least 95% of documented steady-note cases should have a dominant pitch within 5% of half input frequency. Run a 30-minute live session for each documented supported configuration and count observed dropouts, underruns, or overruns.

Expected: automated results and live observations are reported separately. A test's absence is marked unverified, not passed.

## Scenario 5: Loopback latency measurement

1. Document the input/output hardware, route, accepted rate and buffer, and any monitoring/loopback wiring. Avoid acoustic feedback.
2. Enable the octave effect, set the mix to 100% wet, and feed a repeatable transient to the capture path while preserving a reference copy. Record the processed output through the loopback path.
3. Estimate elapsed time between reference and processed output onset over multiple trials, including a low note and a transient. Report the median and trial count. Account for measurement-device latency or state that it is included.
4. Compare the measured median physical input-to-output latency with the 20 ms target. Report separately the backend-reported timing, calculated buffer/algorithm estimates, note settling time, and measured end-to-end value.

Expected: the display labels estimates and measurements correctly. A target miss is recorded with conditions and audible tradeoffs; it is not silently relabeled as a pass.

## Validation record

For every manual session, record date, tested platform, dependency versions, hardware, selected and actual settings, route evidence, observed sound behavior, dropouts, latency method/results, and limitations. The README should identify tested versus merely supported or intended environments.
