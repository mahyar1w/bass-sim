# Implementation Plan: Real-Time Guitar-to-Bass Simulator

**Branch**: `N/A (no Git repository; setup identifier: 001-guitar-to-bass-simulator)` | **Date**: 2026-10-08 | **Spec**: [spec.md](spec.md)

**Input**: Feature specification from `specs/001-guitar-to-bass-simulator/spec.md`; original brief at `/home/mahyar/Desktop/basesim/basssim.md`.

## Summary

Build a Linux-first standalone desktop application that routes a selected mono guitar input to a selected output, processes single notes down one octave, and exposes safe sound controls, meters, presets, and diagnostics. Use native PipeWire streams for explicit routing, an independently testable DSP chain, and a Dear ImGui interface. The two-head crossfaded delay prototype failed offline pitch checks. A waveform divider sounded fuzzy in the user's guitar test; a pure sine replacement was cleaner but insufficiently bass-like. Two additive-harmonic versions also sounded harsh live. The current build retains the cleaner filtered tracker and sine voice while a dry guitar recording is gathered to guide the next timbre change. Full-wet physical latency remains unmeasured. Keep audio callbacks bounded and allocation-free.

## Technical Context

**Language/Version**: C++23; C where required by PipeWire/SPA interfaces

**Primary Dependencies**: PipeWire/SPA and the active WirePlumber session policy on Linux; Dear ImGui with official GLFW and OpenGL3 backends; GLFW; OpenGL; nlohmann/json for versioned settings and presets. Pin and document tested releases during implementation.

**Storage**: Local, versioned JSON files for user configuration and named sound presets; no database or network service

**Testing**: CMake/CTest with hardware-free DSP, serialization, and state tests; repeatable Linux PipeWire routing and loopback latency checks; documented listening sessions

**Target Platform**: Linux desktop with PipeWire and a compatible session manager; Windows/macOS are future backend work, not release claims

**Project Type**: Standalone desktop audio application

**Performance Goals**: Stable single-note octave-down processing; at least 95% of documented steady-note tests within 5% of half input frequency; measured median physical input-to-output latency at most 20 ms with the effect enabled and 100% wet on a documented supported setup; 30-minute live stability sessions

**Constraints**: Never change system audio defaults or other apps' routes; independent capture/playback choices; no allocation, blocking, file access, UI work, or logging in process callbacks; bounded output; requested and accepted audio settings visible; explicit recovery on route or device loss

**Scale/Scope**: One application, one mono instrument input, mono/stereo output, one active audio route, local presets, six outcome criteria; no plugin hosting, recording, MIDI, or cloud features

## Constitution Check

*GATE: Checked before Phase 0 research and again after Phase 1 design.*

`.specify/memory/constitution.md` contains only unfilled template placeholders. It defines no ratified principles or enforceable gates. There are therefore no constitutional violations to justify; source-brief constraints and the feature spec govern this plan. Recheck if the constitution is completed before implementation.

**Pre-research result**: PASS (no active constitutional rules).

**Post-design result**: PASS (no active constitutional rules; design remains within the feature spec and excludes unrelated features).

## Phase 0: Research Decisions

The decisions, alternatives, dependency status/licensing, primary documentation, and unresolved measurement risks are recorded in [research.md](research.md). No `NEEDS CLARIFICATION` items remain. Algorithm quality, cross-device clock behavior, and end-to-end latency are validation risks with explicit tests, not assumed capabilities.

**Feasibility gate status (2026-10-09)**: Offline single-sine SC-002 pitch cases pass with the Rubber Band R3 waveform shifter. The exact streaming processor showed about 85 ms of offline onset difference on the supplied recording, so the current algorithm cannot meet the 20 ms physical SC-003 target. A full-wet physical loopback measurement remains unavailable without loopback hardware. Do not treat a dry-path onset or buffer-duration estimate as proof of success.

## Phase 1: Design

- [data-model.md](data-model.md) defines endpoints, requested/actual settings, engine state, sound parameters, presets, and diagnostics.
- [contracts/user-interface.md](contracts/user-interface.md) defines visible controls, safe transitions, errors, and latency labels.
- [contracts/persistence.md](contracts/persistence.md) defines the versioned preset/config boundary and invalid-data behavior.
- [quickstart.md](quickstart.md) defines build and end-to-end validation once the application is implemented.

## Processing and Routing Design

1. Enumerate PipeWire capture/playback nodes outside real-time callbacks. Show stable readable names, channels, and available capabilities. Keep endpoint identity separate from display name.
2. Validate selection, then create targeted capture and playback streams without altering global defaults. Disable fallback/move behavior where supported by the active session policy. Verify the effective links before unmuting; fail closed if the chosen route cannot be maintained.
3. Bridge the two streams through a preallocated bounded mono queue. Handle format/clock differences using PipeWire graph conversion where available; monitor queue occupancy, underruns, and overruns. If stable independent-device operation is not available for a pair, reject it with a specific explanation.
4. Process input gain, optional gate, octave shifter, tone, optional compression/saturation, wet/dry mix, output gain, and final protection. Process dry and wet paths with deliberate delay alignment; smooth controls, bypass, mute, and stream starts/stops. Do not let exceptions cross process callbacks.
5. Reconfigure by muting, stopping affected streams, preparing all new resources outside callbacks, connecting and verifying the new route, and fading up only after negotiated settings and stable buffers are known. On failure, stay muted or restore a verified prior route.
6. Pass bounded parameter snapshots from UI to the processing engine. Return meter/diagnostic snapshots to the UI; store files and format messages off the audio thread.
7. Separate displayed latency into negotiated/reported transport information, calculated algorithm/buffer estimates, and measured hardware loopback end-to-end results. Never label an estimate as a measurement.

## Project Structure

### Documentation (this feature)

```text
specs/001-guitar-to-bass-simulator/
├── spec.md
├── plan.md
├── research.md
├── data-model.md
├── quickstart.md
├── contracts/
│   ├── user-interface.md
│   └── persistence.md
└── tasks.md                 # Created by $speckit-tasks
```

### Source Code (repository root)

```text
CMakeLists.txt
src/
├── main.cpp
├── audio/                   # PipeWire enumeration, streams, routing, timing
├── dsp/                     # Standalone processing stages and parameter smoothing
├── app/                     # Engine state, commands, snapshots, diagnostics
├── ui/                      # ImGui screens and presentation
└── persistence/             # Versioned settings and presets
tests/
├── dsp/
├── app/
└── persistence/
docs/                        # Setup, architecture, signal flow, tests, limits
README.md
```

**Structure Decision**: One executable and a small set of focused internal targets. Keep DSP independent of the UI and PipeWire so hardware-free tests can exercise it. Platform-specific audio code stays under `src/audio/` to permit a later alternative backend without changing DSP behavior.

## Validation Gates for Implementation

- Build from documented Linux instructions; run all hardware-free CTest checks.
- Verify explicit capture/playback links and unchanged desktop defaults/routes before and after opening streams.
- Test silence, impulses, low E through the documented guitar range, sustained notes, note transitions, control extremes, multiple rates/block sizes, and finite/bounded output.
- Test device removal, incompatible settings, malformed presets, mute/bypass/start/stop, and failed reconfiguration with safe output and actionable diagnostics.
- Run live guitar listening and 30-minute stability checks, recording observed dropouts and CPU conditions.
- Measure full-wet loopback physical input-to-output latency on named hardware/settings and compare with the 20 ms SC-003 target; distinguish actual measurements from estimates and unverified expectations.

## Complexity Tracking

No constitutional violations. Native PipeWire streams and a bounded inter-stream queue are necessary to satisfy independent endpoint selection and route isolation; their clock and latency behavior must be verified on actual devices.
