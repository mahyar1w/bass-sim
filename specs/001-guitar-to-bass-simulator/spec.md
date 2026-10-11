# Feature Specification: Real-Time Guitar-to-Bass Simulator

**Feature Branch**: `N/A (workspace is not a Git repository)`

**Created**: 2026-10-08

**Status**: Draft

**Input**: User description: `basssim.md` (identical copies at `/home/mahyar/Desktop/basesim/basssim.md` and `/home/mahyar/Downloads/basssim.md`)

## User Scenarios & Testing *(mandatory)*

### User Story 1 - Play guitar with a bass-like sound (Priority: P1)

A guitarist connects a live instrument, chooses where to hear it, starts processing, and plays single notes with a stable octave-down bass-like sound. They can stop or mute output safely.

**Why this priority**: Live conversion is the core reason to use the application.

**Independent Test**: Select a connected guitar input and output, start processing, play single notes, then mute and stop. Confirm audible octave-down output and safe transitions.

**Acceptance Scenarios**:

1. **Given** a working mono guitar input and available output, **When** the user selects both and starts processing, **Then** they hear the selected input with the bass effect active and see that processing is running.
2. **Given** processing is running, **When** the user mutes, stops, or restarts it, **Then** output changes smoothly without an unexpectedly loud burst.
3. **Given** a steady single note, **When** the octave-down effect settles, **Then** the dominant processed pitch is approximately one octave below the played pitch.

---

### User Story 2 - Select and recover audio routing (Priority: P2)

A user sees available inputs, outputs, and channel choices; selects independent endpoints where supported; and understands the actual audio settings and any routing failure.

**Why this priority**: Reliable device choice and clear recovery are needed across different setups.

**Independent Test**: Change input, output, and input channel; refresh the list; request supported and unsupported settings; and disconnect a selected device. Check the displayed route, accepted settings, and recovery guidance.

**Acceptance Scenarios**:

1. **Given** multiple endpoints, **When** the user selects an input, its channel, and an output, **Then** the displayed route matches the selection and other applications' output routes remain unchanged.
2. **Given** an unavailable or unsupported selection, **When** the user starts or changes processing, **Then** the application avoids unsafe output and shows the cause and a recovery action.
3. **Given** an endpoint is added or removed, **When** the user refreshes the list, **Then** choices update without an application restart.

---

### User Story 3 - Shape and bypass the sound (Priority: P3)

A guitarist adjusts gain, pitch effect, tone, optional dynamics, and wet/dry balance while playing, monitors levels, and compares the effect with the original input.

**Why this priority**: The result must be useful across guitars, playing styles, and listening systems.

**Independent Test**: Adjust every sound control during sustained notes, bypass individual optional effects and the full effect, and observe meters and clipping warnings.

**Acceptance Scenarios**:

1. **Given** processing is running, **When** a control changes within its allowed range, **Then** the change takes effect without a click, abrupt level jump, or invalid value.
2. **Given** the full effect is bypassed, **When** the user plays, **Then** the selected dry input remains audible at a controlled level; muting silences both paths.
3. **Given** input or output is near overload, **When** the user plays, **Then** meters and clipping indicators show the condition and output remains overload protected.

---

### User Story 4 - Resume a saved setup (Priority: P4)

A returning user loads saved sound settings or a preset and can recover from a missing or damaged settings file.

**Why this priority**: Saved settings make a repeatable live setup practical.

**Independent Test**: Save, reload, and select presets; restart the application; then provide missing, malformed, and out-of-range saved values.

**Acceptance Scenarios**:

1. **Given** a valid saved preset, **When** the user loads it during playback, **Then** the documented sound controls are restored without a sudden loud output.
2. **Given** saved data is missing, malformed, or out of range, **When** the application opens or loads it, **Then** it uses safe defaults or valid portions, identifies the problem, and remains usable.

### Edge Cases

- No capture or playback endpoints are available, or an endpoint disappears during playback.
- Input and output have incompatible formats or clocks; the user must see whether the combination is supported.
- A requested sample rate, channel, or buffer size is rejected or changed by the audio environment.
- The guitar is silent, very quiet, clipped, or playing overlapping notes; output remains finite and controlled.
- A preset contains invalid, missing, or extreme values, or a settings file cannot be read or written.
- A user changes devices, audio settings, bypass, or mute while processing is active.
- The window is resized or displayed at high pixel density; controls and diagnostics remain readable.

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: The application MUST provide live, standalone guitar-to-bass processing on Linux desktop systems with the project's supported audio environment.
- **FR-002**: The application MUST list available capture and playback endpoints with readable names, channel choices, and a refresh action.
- **FR-003**: Users MUST be able to select a capture endpoint, one mono input channel, and a playback endpoint independently, with mono or stereo playback where available.
- **FR-004**: Users MUST be able to request a sample rate and buffer size where configurable, and MUST see both requested and accepted settings or a clear unsupported-setting explanation.
- **FR-005**: Users MUST be able to start, stop, and mute processing explicitly and see engine state and actionable errors.
- **FR-006**: The application MUST preserve other applications' routing and system defaults when selecting its own audio endpoints.
- **FR-007**: The application MUST handle endpoint loss, missing devices, incompatible endpoint combinations, and failed reconfiguration by entering a safe state and explaining recovery.
- **FR-008**: The effect MUST provide input gain, an octave-down transformation enabled by default, bass-oriented tone shaping, wet/dry mix, output gain, and output overload protection.
- **FR-009**: The effect MUST provide independently adjustable or bypassable noise gating, compression, and subtle saturation.
- **FR-010**: Each sound control MUST have a documented range and safe default; changing controls during playback MUST avoid abrupt audible discontinuities.
- **FR-011**: Full effect bypass MUST pass the selected dry input at a controlled level; mute MUST silence both dry and processed paths; bypass and mute transitions MUST avoid loud bursts.
- **FR-012**: The application MUST show input and output levels, clipping indicators, and available dropout or overload diagnostics.
- **FR-013**: The application MUST show latency information with a clear distinction among reported values, calculated estimates, and measured end-to-end latency; it MUST provide a repeatable measurement procedure when direct measurement is unavailable.
- **FR-014**: Users MUST be able to save, select, and load sound presets and retain applicable user settings across launches. Invalid saved values MUST be rejected or replaced with safe defaults while keeping the application usable.
- **FR-015**: The interface MUST group audio setup, sound controls, meters, and diagnostics clearly; convey status without relying on color alone; explain unfamiliar controls; and remain usable when resized or displayed at high pixel density.
- **FR-016**: The application MUST document setup, routing, operation, sound controls, pitch-shift limits, latency, troubleshooting, supported and tested platforms, known limitations, and how to reproduce validation.
- **FR-017**: Verification MUST cover representative signals, parameter boundaries, supported audio settings, saved data recovery, live routing, device loss, and playback transitions, with automated results distinguished from manual observations and unverified expectations.
- **FR-018**: Processing MUST keep up with accepted live audio settings without avoidable dropouts caused by routine control changes; a failed or overloaded processing state MUST not emit uncontrolled output.

### Key Entities

- **Audio endpoint**: Available capture or playback destination with a readable identity, channel choices, and supported settings.
- **Audio configuration**: Requested and accepted endpoint, channel, sample-rate, and buffer choices, plus current processing state.
- **Sound settings**: Bounded controls for gain, octave effect, tone, optional effects, mix, bypass, and mute.
- **Preset**: A named, saved set of sound settings that can be loaded independently of audio device selection.
- **Diagnostic**: A user-visible status, meter, clipping event, latency value, or actionable failure message.

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: In a supported live setup, at least 9 of 10 first-time test users can select the guitar input and output, start processing, and hear an octave-down sound within 3 minutes using the quick start.
- **SC-002**: For steady single-note test tones across the documented guitar range, at least 95% of tested notes have a dominant processed pitch within 5% of half the input frequency after the documented settling interval.
- **SC-003**: In a documented low-latency setup, measured median physical input-to-output latency is at most 20 ms with the octave effect enabled and the mix fully wet; the report states the hardware, settings, method, and any unmet target.
- **SC-004**: Across a 30-minute live test at each documented supported audio setting, routine sound-control adjustments and preset changes cause no application crashes or uncontrolled output, and observed dropouts are counted and reported.
- **SC-005**: In all tested mute, stop, bypass, device-change, and invalid-preset scenarios, output stays within the documented safe level and the user can restore playback without restarting, except when a required device remains unavailable.
- **SC-006**: At least 9 of 10 test users can identify the active input, output, actual audio settings, processing state, and whether a latency value is measured or estimated without assistance.

## Assumptions

- The primary quality target is one guitar note at a time. Chords and overlapping notes may have documented artifacts; full polyphonic fidelity is outside the initial scope.
- Linux desktop use is the first release target. Other desktop platforms are future possibilities and must be described as untested unless validated.
- A compatible guitar capture path, playback endpoint, and monitoring setup are available. External routing steps may be necessary and must be documented.
- The default preset uses an octave-down effect with conservative output level. Optional effects can be bypassed individually.
- Presets store sound settings; audio endpoint selections are retained as user settings when endpoints remain available.
- Plugin hosting, recording, MIDI, cloud services, and emulation of every physical bass characteristic are outside this feature's scope.
- Documentation and test evidence are deliverables for the eventual application; this specification does not claim implementation or hardware validation has occurred.
