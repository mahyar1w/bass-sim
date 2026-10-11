# Bass-sim — Real-Time Guitar-to-Bass Simulator

## Project objective

Build **Bass-sim**, a standalone desktop application that accepts live electric guitar audio and processes it into a convincing bass-like sound using pitch shifting, filtering, and other appropriate DSP techniques.

Deliver a working, maintainable application with a polished interface, reliable audio routing, low-latency processing, and comprehensive documentation.


## Technology stack

- **Language:** C++23
- **Build system:** CMake
- **User interface:** Dear ImGui — https://github.com/ocornut/imgui
- **Primary platform:** Linux
- **Audio integration:** A maintained audio library or backend that works with PipeWire on Linux
- **Secondary platforms:** Windows and macOS where practical

Choose an appropriate windowing and rendering backend for ImGui. Keep platform-specific functionality isolated so that additional platforms can be supported without restructuring the DSP engine.

Before selecting dependencies, explain their purpose, maintenance status, licensing, platform support, and relevant tradeoffs. Distinguish native PipeWire integration from compatibility through another audio API. Do not assume that a library provides capabilities without verifying them.

## Functional requirements

### 1. Audio devices and routing

The application must:

- Enumerate available audio input and output devices or endpoints exposed by the selected backend.
- Display readable device names and available channel configurations.
- Allow users to select an input, output, and input channel.
- Support a mono guitar input and mono or stereo output.
- Allow users to configure sample rate and buffer size where supported.
- Clearly show requested settings and the settings actually accepted by the backend.
- Refresh the device list without restarting the application.
- Handle missing devices, unsupported configurations, and device disconnection gracefully.
- Show audio engine status and actionable error messages.
- Provide explicit controls to start and stop audio processing.

Document any routing that requires external PipeWire tools. If separate input and output devices require synchronization or resampling, implement that support or clearly explain the limitation.

### 2. Guitar-to-bass DSP

Create a modular processing chain with an octave-down transformation as the core effect.

Include:

- Input gain
- Optional noise gate
- Pitch shifting, defaulting to **one octave down**
- Tone shaping appropriate for a bass-like sound
- Optional compression and subtle saturation
- Wet/dry mix
- Output gain
- Output overload protection

Expose useful controls with sensible defaults and bounded parameter ranges.

Prioritize stable tracking, controlled low frequencies, natural note transitions, and low perceived latency. Avoid excessive muddiness, harsh artifacts, unstable output, and abrupt parameter changes.

Explain the selected pitch-shifting approach and its limitations, including:

- Algorithmic latency
- Pitch accuracy and tracking behavior
- Transient handling
- Behavior with sustained notes
- Behavior with chords and overlapping notes
- CPU usage and quality tradeoffs

Make single-note playing the primary quality target. If polyphonic processing is supported, explain its additional costs and limitations. Do not claim that octave shifting alone reproduces every tonal characteristic of a physical bass guitar.

Separate essential processing from optional effects so that each stage can be tested and bypassed independently.

### 3. Latency and real-time safety

Treat latency and audio stability as core design requirements.

- Make buffer size adjustable when the backend permits it.
- Provide a practical low-latency default and explain its tradeoff with dropout risk.
- Support changes to audio settings through a controlled engine reconfiguration process.
- Smooth parameter changes to prevent clicks and zipper noise.
- Preallocate buffers and initialize expensive resources before processing begins.
- Avoid memory allocation, blocking locks, file access, logging, and other unbounded operations in the real-time audio callback.
- Use a suitable thread-safe mechanism to transfer control changes from the UI to the audio engine.
- Keep UI rendering and nonessential analysis outside the audio callback.
- Report underruns or overruns where the backend exposes that information.

Distinguish buffer latency, algorithmic latency, backend-reported latency, and measured end-to-end latency. Label estimates as estimates. Do not present buffer duration as the application's total latency.

If a reliable hardware latency measurement is unavailable, document a reproducible loopback measurement procedure.

## User interface requirements

Create a sleek, vibrant, cohesive ImGui interface designed for everyday use.

Include:

- Audio device and channel selection
- Sample-rate and buffer-size settings
- Start/stop controls and engine status
- Clearly grouped DSP controls
- Input and output level meters
- Clipping indicators
- Effect bypass
- A safe way to mute output
- A latency display with clear measurement or estimation labels
- Preset selection and save/load controls
- An accessible error and diagnostic area

Use consistent spacing, typography, colors, and control sizing. Keep labels readable and avoid relying on color alone to communicate status.

Provide concise tooltips for controls whose effects may be unfamiliar. Ensure the layout remains usable when the window is resized and on high-DPI displays.

Define bypass behavior explicitly, including whether dry input remains audible and how transitions avoid clicks. Avoid unexpectedly loud output when starting the engine, changing devices, or loading a preset.

Persist user settings and presets using a documented format. Validate loaded values and recover gracefully from missing or malformed files.

## Architecture and code quality

Use a clear separation between:

- Audio backend and device management
- DSP engine and individual processing stages
- Parameter state and communication between threads
- UI rendering
- Configuration and preset persistence
- Diagnostics and testing

Follow modern C++23 practices:

- RAII and explicit resource ownership
- Const correctness
- Clear interfaces and descriptive names
- Minimal global mutable state
- Appropriate use of standard-library facilities
- Predictable error handling
- Consistent formatting

Keep exception handling and resource cleanup compatible with real-time constraints. Do not allow exceptions to escape the audio callback.

Comments should explain non-obvious DSP decisions, threading assumptions, mathematical reasoning, and platform limitations. Avoid comments that merely restate the code.

Use CMake targets with explicit dependencies. Document supported compiler versions and dependency installation. Avoid relying on undocumented local paths or machine-specific configuration.

Keep the implementation focused. Do not add plugin hosting, recording, MIDI, cloud services, or other unrelated features unless required for the core application.

## Testing and validation

Provide automated tests for behavior that can be verified without audio hardware, including:

- DSP processing of silence, sine waves, impulses, and sustained tones
- Parameter boundaries and smoothing
- Different supported sample rates and processing block sizes
- Finite output values and resistance to numerical instability
- Configuration and preset serialization
- Recovery from invalid configuration data

For an octave-down sine-wave test, verify that the dominant output frequency is approximately half the input frequency after the algorithm settles. Document tolerance and analysis conditions.

Provide a repeatable manual validation procedure for:

- Live electric guitar input
- Device enumeration and selection
- Audio routing under PipeWire
- Buffer-size changes
- Start, stop, mute, and bypass transitions
- Device disconnection and recovery
- CPU usage and dropout behavior
- Listening comparisons across representative playing techniques

Separate automated results, manual observations, and unverified expectations. Do not claim hardware, platform, latency, or sound-quality testing that has not been performed.

## Documentation

### README.md

Provide a clear README containing:

- What Bass-sim does
- Current implementation status
- Supported and tested platforms
- Dependencies
- Linux installation and build instructions
- How to run the application
- How to connect and route guitar audio
- A quick-start workflow
- Common troubleshooting steps
- Known limitations
- Links to detailed documentation

### docs/

Include substantial documentation covering:

- Architecture and module responsibilities
- Audio backend selection and PipeWire integration
- DSP signal flow and algorithm choices
- Pitch-shifting limitations and quality tradeoffs
- Latency sources and measurement procedures
- Real-time threading and parameter synchronization
- UI controls and presets
- Configuration format and storage locations
- Testing procedures and results
- Platform-specific setup
- Development and contribution guidance

Include diagrams where they clarify architecture or signal flow. Keep documentation consistent with the implemented behavior.

## Implementation approach

1. Review the existing project, if present, and preserve relevant conventions.
2. State material assumptions and identify technical risks.
3. Select the audio backend and DSP approach, explaining the tradeoffs.
4. Establish a buildable application with working audio input and output.
5. Implement and validate the core octave-down processing chain.
6. Add controls, metering, configuration, and presets.
7. Refine the UI and test error handling.
8. Complete documentation and report verification results.

Make reasonable implementation decisions when details are unspecified. Ask for clarification only when a decision materially changes the project's scope or prevents meaningful progress.

Deliver functioning code rather than a design-only response. Clearly identify any unfinished functionality or environment limitations.

## Deliverables

- Complete C++23 source code
- CMake build configuration
- Dependency setup instructions
- A working ImGui desktop interface
- A PipeWire-compatible Linux audio implementation
- Modular guitar-to-bass DSP
- Automated tests and manual validation instructions
- README.md
- Comprehensive docs/ documentation

## Acceptance criteria

The project is complete when:

- It builds using the documented Linux instructions.
- It lists available audio inputs and outputs and supports selection.
- It processes live guitar input into a bass-like, octave-down output.
- Its main processing controls work without audible discontinuities caused by parameter changes.
- Buffer settings are adjustable where supported, and actual settings are visible.
- The audio callback follows the stated real-time safety requirements.
- Invalid settings and device failures produce clear, recoverable behavior.
- Tests pass, and untested platform or hardware behavior is explicitly identified.
- Documentation accurately explains setup, operation, architecture, limitations, and validation.
