# Tasks: Real-Time Guitar-to-Bass Simulator

**Input**: Design documents in `specs/001-guitar-to-bass-simulator/`  
**Prerequisites**: [plan.md](plan.md), [spec.md](spec.md), [research.md](research.md), [data-model.md](data-model.md), [contracts/](contracts/), [quickstart.md](quickstart.md)  
**Tests**: Required by FR-017 and the source brief. Write each automated test before the behavior it verifies and confirm it fails for the missing behavior.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Independent work in different files after the stated phase prerequisite; tasks without it follow the listed order.
- **[USn]**: User story from `spec.md`.
- File paths are relative to the repository root. A task is complete only when its stated behavior is verified; documentation of an untested outcome is not a passing test.

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Make the empty application workspace buildable with explicit dependencies and a test target.

- [X] T001 Create `CMakeLists.txt` with C++23, explicit `bass-sim`, DSP, audio, UI, and persistence targets; link PipeWire/SPA, pinned Dear ImGui official GLFW/OpenGL3 backends, GLFW, OpenGL, and nlohmann/json without machine-specific paths.
- [X] T002 Add CTest registration and a small hardware-free test executable in `tests/CMakeLists.txt`, wired from `CMakeLists.txt`, so `ctest --test-dir build --output-on-failure` runs the suite.
- [X] T003 [P] Document each dependency's purpose, tested version, maintenance source, license, platform scope, and native PipeWire versus compatibility tradeoff in `docs/dependencies.md`.
- [X] T004 Create the Dear ImGui window/context lifecycle and scalable font/spacing setup in `src/ui/window.cpp`, with a minimal no-audio entry point in `src/main.cpp`; ensure the application can open and close without starting audio.

**Checkpoint**: Build and launch an empty window; CTest discovers its test target.

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Define shared state and bounded cross-thread communication before any story changes live audio.

- [X] T005 [P] Define `AudioEndpoint`, `AudioConfiguration`, `EngineState`, and `Diagnostic` in `src/app/types.h`; preserve endpoint constraints verbatim: ID "Required while listed; rediscover after restart", name "Nonempty fallback to ID", direction "Exactly one direction", channels "At least one usable channel for selection", capabilities "Never present an unknown value as guaranteed", and availability "Updated on enumeration events/refresh".
- [X] T006 [P] Define validated sound parameters and safe defaults in `src/dsp/parameters.h` using the exact v1 ranges in `specs/001-guitar-to-bass-simulator/contracts/persistence.md`; preserve "Bounded; conservative startup/default values", "Default ratio 0.5; unsupported ratios rejected", "0 to 1; align paths to avoid combing surprises", and "Bypass passes controlled dry signal; mute silences all"; reject nonfinite values and limit tone cutoff below Nyquist.
- [X] T007 [P] Add preallocated, bounded single-producer/single-consumer audio transfer and complete parameter/meter snapshots in `src/app/realtime_exchange.h`; snapshots obey "Values are bounded or marked unavailable" and "The audio thread sees complete validated snapshots, never partially updated controls"; callbacks must not allocate, block, perform file I/O, log, or throw.
- [ ] T008 Write state-transition and safe-mute tests for `Stopped`, `Preparing`, `Connecting`, `Running`, `Reconfiguring`, `Stopping`, and `ErrorMuted` in `tests/app/engine_state_test.cpp`; verify only a verified route may become audible.
- [X] T009 Implement the tested engine state machine and diagnostic publication in `src/app/engine_controller.cpp`: "Only `Running` with `route_verified = true` may produce audible output"; failures enter `ErrorMuted`, and reconfiguration mutes before replacing streams; keep resource creation and message formatting off callbacks.
- [X] T010 Initialize and clean up the PipeWire connection/event loop outside callbacks in `src/audio/pipewire_context.cpp`; report server-unavailable errors instead of falling back silently.

**Checkpoint**: Shared types, state, and real-time exchange build; state tests pass. User story work starts after this phase.

---

## Phase 3: User Story 1 - Play guitar with a bass-like sound (Priority: P1) 🎯 MVP

**Goal**: Select a mono guitar input and output, hear a controlled octave-down sound, and start/stop/mute safely.

**Independent Test**: With a working capture and playback endpoint, start processing, play single notes including low E, confirm audible octave-down output, then mute, stop, and restart without a loud burst. Run offline pitch and finite-output checks.

### Tests for User Story 1

- [X] T011 [P] [US1] Add silence, impulse, low-E-through-documented-range steady-note, changing-note, supported-rate/block-size, and finite-output tests in `tests/dsp/octave_shifter_test.cpp`; record settling interval and verify SC-002 (at least 95% within 5% of half input frequency).
- [X] T012 [P] [US1] Add core-chain tests for gain, tone, wet-only output, final protection, and bounded output under hot input in `tests/dsp/processor_test.cpp`.
- [ ] T013 [P] [US1] Add start, stop, mute, route-verification, and failure-safe tests using a fake audio source/sink in `tests/app/live_processing_test.cpp`.

### Implementation for User Story 1

- [ ] T014 [P] [US1] Measure note-onset settling of the current Rubber Band R3 waveform shifter in `src/dsp/octave_shifter.cpp` and its tests. One supplied recording showed about 85 ms offline streaming onset difference under stated RMS thresholds; repeat across low and high notes. The current algorithm cannot meet the 20 ms physical target.
- [X] T015 [P] [US1] Create separately targeted PipeWire capture/playback streams in `src/audio/pipewire_streams.cpp`; apply available no-fallback/no-move policy, observe negotiated format/quantum, verify effective links before sound, and never change global defaults.
- [X] T016 [US1] Bridge capture and playback using the preallocated queue in `src/audio/stream_bridge.cpp`; handle variable block sizes, underflow/overflow, mono input and mono/stereo output, and distinct stream clocks without uncontrolled output.
- [X] T017 [US1] Build the core chain in `src/dsp/processor.cpp`: input gain, octave down enabled by default, bass-oriented tone, fully wet default, conservative output gain, final output protection, and smooth mute/start/stop.
- [X] T018 [US1] Connect validated settings, stream lifecycle, route verification, and fail-muted behavior in `src/app/engine_controller.cpp`; no exception may escape a process callback.
- [X] T019 [US1] Add endpoint/channel selectors, Start/Stop/Mute, current state, actual-versus-requested settings, and actionable failure text in `src/ui/live_view.cpp`.
- [X] T020 [US1] Wire the window, live view, controller, and orderly shutdown in `src/main.cpp`; audio stays stopped until an explicit valid selection and Start.
- [ ] T021 [US1] Run the offline pitch tests and full-wet physical input-to-output loopback measurement per `specs/001-guitar-to-bass-simulator/quickstart.md`; record hardware, accepted settings, trial count, pitch results, and 20 ms median outcome in `docs/validation/latency-feasibility.md`.
- [ ] T022 [US1] Decide the final octave-down method in `docs/validation/latency-feasibility.md`: retain `src/dsp/octave_shifter.cpp` only if SC-002 and the full-wet 20 ms SC-003 gate pass; otherwise compare a lower-latency candidate, update `src/dsp/octave_shifter.cpp` and its tests, and repeat the measurements before claiming the MVP meets the spec.

**Checkpoint**: US1 is independently playable and pitch-tested. Its 20 ms result is measured and honestly reported; a failed gate remains open until the algorithm or setup satisfies it.

---

## Phase 4: User Story 2 - Select and recover audio routing (Priority: P2)

**Goal**: Refresh and select independent endpoints/channels, expose requested and accepted settings, and recover safely from device or route failure.

**Independent Test**: With a fake graph and a live PipeWire graph, refresh nodes, change both endpoints and channel, request supported/unsupported settings, disconnect a device, and verify safe mute, diagnostics, and unchanged desktop defaults/routes.

### Tests for User Story 2

- [ ] T023 [P] [US2] Test duplicate display names, missing IDs, capture/playback direction, channel membership, refresh, and removal in `tests/audio/device_catalog_test.cpp`; IDs are session-scoped and a name is never the identity.
- [ ] T024 [P] [US2] Test requested/actual setting mismatches, incompatible endpoint pairs, route moves, failed reconfiguration, and recovery states in `tests/audio/route_manager_test.cpp`.

### Implementation for User Story 2

- [X] T025 [P] [US2] Enumerate PipeWire nodes and channel labels, maintain stable session IDs and nonempty names, and update on refresh/add/remove in `src/audio/device_catalog.cpp`; unknown capabilities must remain unknown.
- [X] T026 [US2] Validate endpoint direction, format compatibility, effective links, and no-fallback behavior in `src/audio/route_manager.cpp`; enforce audio-configuration constraints verbatim: capture "ID exists and channel belongs to source", playback "ID exists and selected channels are supported", requests "Positive, within UI bounds; may be unavailable", actual settings "Populated only after successful negotiation", and verified route "Required before unmuting"; reject unsupported pairs with a recovery action.
- [X] T027 [US2] Implement muted stop/prepare/reconnect/verify/fade reconfiguration and lost-device recovery in `src/app/engine_controller.cpp`; never unmute an unverified or moved route.
- [X] T028 [US2] Add input/output/channel choices, refresh, requested/accepted rate and buffer, and `Reconfiguring`/error states in `src/ui/audio_setup_view.cpp`; unavailable actual values display `Unknown`.
- [X] T029 [US2] Publish reported, estimated, and measured latency as different categories in `src/ui/diagnostics_view.cpp`; reported input/output timing is "Optional; never relabeled measured end-to-end", latency includes value, units, category, measurement setup when applicable, and age, and "A buffer-duration estimate is never total latency"; show only observed underrun/overrun counts, otherwise unavailable.
- [ ] T030 [US2] Perform before/after route and system-default checks, disconnection, incompatible-pair, and clock-drift exercises from `specs/001-guitar-to-bass-simulator/quickstart.md`; record observed results and recoverability in `docs/validation/routing.md`.

**Checkpoint**: US2 selection/recovery works through the same app without moving other applications; fake-graph tests can validate its rules independently of hardware.

---

## Phase 5: User Story 3 - Shape and bypass the sound (Priority: P3)

**Goal**: Adjust bounded sound controls during playback, compare controlled dry input with the effect, and see levels/clipping without unsafe output.

**Independent Test**: On sustained notes, adjust each control and toggle each optional stage, full bypass, and mute; verify smoothed output, dry-only bypass, silence on mute, bounded peak, and readable meters.

### Tests for User Story 3

- [X] T031 [P] [US3] Test gate, compression, saturation, parameter bounds, silence, impulses, multiple rates/blocks, and finite output in `tests/dsp/effects_test.cpp`.
- [X] T032 [P] [US3] Test gain/mix/bypass/mute and octave-toggle smoothing, direct dry path, final peak at or below -1 dBFS, and no sudden preset-like parameter jump in `tests/dsp/control_transition_test.cpp`.

### Implementation for User Story 3

- [X] T033 [P] [US3] Implement independently bypassable noise gate with the validated -80 to -20 dB threshold range in `src/dsp/gate.cpp`.
- [X] T034 [P] [US3] Implement independently bypassable compressor with amount from 0 to 1 and bounded state in `src/dsp/compressor.cpp`.
- [X] T035 [P] [US3] Implement independently bypassable subtle saturation with amount from 0 to 1 and finite output in `src/dsp/saturation.cpp`.
- [X] T036 [US3] Integrate the optional stages, 0-to-1 wet mix, direct dry path for the zero-fixed-delay oscillator, full-effect bypass, smoothed control changes, and always-on final protection in `src/dsp/processor.cpp`; mute overrides both paths.
- [X] T037 [P] [US3] Add grouped controls, range labels, enable/bypass switches, and concise tooltips in `src/ui/sound_view.cpp` using the v1 limits in `specs/001-guitar-to-bass-simulator/contracts/persistence.md`.
- [X] T038 [US3] Publish input/output levels and latched clipping indicators without audio-thread UI work in `src/ui/meters_view.cpp`; use text/icons as well as color and a clear action.
- [ ] T039 [US3] Validate live sustained-note control, bypass, mute, clipping, and resized/high-DPI UI behavior from `specs/001-guitar-to-bass-simulator/quickstart.md`; record observations in `docs/validation/sound-controls.md`.

**Checkpoint**: US3 sound shaping can be verified with generated signals independently of presets; 20 ms feasibility remains a required release gate.

---

## Phase 6: User Story 4 - Resume a saved setup (Priority: P4)

**Goal**: Save and load named sound presets and restore applicable settings while surviving missing, malformed, unsupported, or unwritable files.

**Independent Test**: Save/reload a preset during playback, restart, and inject invalid files; confirm bounded values, smooth transitions, safe defaults, preserved valid files, and no route change from a preset.

### Tests for User Story 4

- [X] T040 [P] [US4] Test v1 preset parsing, missing/wrong-type/nonfinite/out-of-range fields, unknown keys, future schema rejection, and 80-character nonempty unique names in `tests/persistence/preset_test.cpp`.
- [X] T041 [P] [US4] Test missing/unavailable endpoint hints, requested rate/buffer fallback, malformed config, unwritable destination, and old-file preservation in `tests/persistence/config_test.cpp`.

### Implementation for User Story 4

- [X] T042 [P] [US4] Implement v1 named preset load/save and field-wise validation in `src/persistence/preset_store.cpp`: "All stored values must be finite"; missing invalid fields use defaults with named warnings; future schemas stay untouched; names are trimmed, nonempty, unique, and at most 80 Unicode characters; preset data excludes endpoint IDs, engine state, mute, and full bypass; save by same-directory temporary file replacement.
- [X] T043 [P] [US4] Implement v1 user configuration load/save in `src/persistence/config_store.cpp`: "Missing endpoint IDs do not prevent startup; they require fresh selection or a visible safe default"; endpoint IDs are revalidated hints, out-of-choice rate/buffer use defaults, and "write failure leaves the previous valid file intact and reports the problem".
- [X] T044 [US4] Add preset selection, explicit replace-on-name-collision, save/load errors, and no route or playback changes on preset load in `src/ui/preset_view.cpp`.
- [X] T045 [US4] Apply complete validated preset snapshots through the smoothed control path in `src/app/engine_controller.cpp`; never parse JSON or write files in a process callback and never persist mute or full bypass in a preset.
- [ ] T046 [US4] Verify save/restart/load, invalid-value recovery, and controlled output during live preset changes from `specs/001-guitar-to-bass-simulator/quickstart.md`; record file paths and outcomes in `docs/validation/presets.md`.

**Checkpoint**: US4 works with a fake engine and with live US1/US3 audio; presets remain separate from device routing.

---

## Phase 7: Polish & Cross-Cutting Concerns

**Purpose**: Complete truthful documentation and release evidence across all stories.

- [X] T047 [P] Document architecture, module ownership, DSP signal flow, pitch-shift limitations, callback/thread boundaries, route policy, and latency sources in `docs/architecture.md`.
- [X] T048 [P] Document setup, controls, preset/config schema and storage paths, external routing steps, loopback procedure, testing methods, known limitations, and platform-specific status in `docs/user-guide.md`.
- [X] T049 Add the supported/tested platform table, exact dependency setup, Linux build/run commands, three-minute quick start, troubleshooting, and links to detailed docs in `README.md`; do not claim unperformed hardware or platform validation.
- [ ] T050 Run the complete CTest suite plus 30-minute live sessions for each documented supported audio configuration; report rates/blocks, route evidence, CPU/dropouts, pitch results, and test failures separately in `docs/validation/results.md`.
- [ ] T051 Conduct the SC-001 and SC-006 ten-user usability checks for setup time and identification of route/settings/state/latency labels; record counts and unmet criteria in `docs/validation/usability.md`.
- [ ] T052 Reconcile `specs/001-guitar-to-bass-simulator/quickstart.md` with actual executable commands and results; verify links, contracts, 20 ms full-wet measurement, and all FR-001–FR-018/SC-001–SC-006 coverage in `docs/validation/results.md`.

---

## Dependencies & Execution Order

### Phase Dependencies

1. **Setup (T001–T004)** has no incoming dependency. T002 needs T001; T004 needs the linked dependencies from T001. T003 is independent documentation.
2. **Foundation (T005–T010)** needs the build/test skeleton. T005–T007 can be written in parallel; T008 precedes T009. Finish the phase before story integration.
3. **US1 (T011–T022)** depends on foundation. Write and observe failing tests first. T014 and T015 can proceed in parallel; T016–T020 integrate them. T021 measures the full-wet path; T022 closes the algorithm feasibility gate. **Do not treat US1 as spec-complete while SC-002 or the 20 ms SC-003 target is unverified or missed.**
4. **US2 (T023–T030)** uses the live engine from US1 for final integration, but its catalog/routing rules can be tested with a fake graph while US1 is underway. T025 may proceed alongside T014; T027 follows T018.
5. **US3 (T031–T039)** starts optional effects only after T022's algorithm decision. Its independent generated-signal tests do not need US2. T033–T035 can run in parallel; T036 integrates them.
6. **US4 (T040–T046)** can develop persistence/tests after foundation, using fake settings, but T045 live integration follows US3 parameter integration. T042 and T043 can run in parallel.
7. **Polish (T047–T052)** follows the relevant story results; T047 and T048 can be drafted in parallel and must be checked against actual behavior before T049/T052.

### User Story Completion Order

```text
Setup → Foundation → US1 core → 20 ms + pitch feasibility gate
                              ├→ US2 live route recovery
                              └→ US3 sound controls → US4 live preset integration
US2 + US3 + US4 → final validation and documentation
```

US2's catalog and US4's file rules have independent fake-driven tests. Their live integrations depend on the US1 application. US3's optional effects wait for the octave algorithm decision so quality/latency evidence is not obscured.

### Parallel Execution Examples

- **US1**: After foundation, T011–T013 are separate test files; T014 DSP prototype and T015 PipeWire streams are separate implementation files. Integration starts at T016.
- **US2**: T023 and T024 can test catalog and routing rules separately; T025 catalog enumeration can run while the US1 shifter is prototyped.
- **US3**: T031 and T032 cover different tests; T033–T035 implement separate optional stages before T036 integration.
- **US4**: T040 and T041 cover different file formats; T042 and T043 implement separate stores before T044–T045 integration.

## Implementation Strategy

### MVP First: User Story 1

Complete setup and foundation, then US1 tests and core processing. Measure steady-note pitch and fully wet physical input-to-output latency before committing to the shifter. Only call the MVP spec-complete after its 20 ms and pitch gates pass; otherwise revise the algorithm/setup and rerun those checks.

### Incremental Delivery

Add routing recovery (US2), sound controls (US3), and saved setups (US4) as independently verifiable increments. Run their automated tests before live integration, then perform the final documentation, hardware, 30-minute stability, and usability validation. Distinguish passed checks from planned or unverified checks in every report.
