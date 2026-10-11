# Research: Bass-sim implementation choices

**Date**: 2026-10-08  
**Scope**: Resolve the technical choices and integration risks needed for [plan.md](plan.md). These are design decisions, not claims of completed hardware or sound-quality validation.

## Linux audio and routing

**Decision**: Use native PipeWire/SPA clients for the Linux release. Enumerate nodes, open distinct targeted capture and playback streams, and let the active PipeWire graph/session manager negotiate formats and links. Set target and no-fallback/no-move properties where the policy supports them; verify effective links before audible output. Never change the global default source, sink, or graph clock settings.

**Rationale**: Native streams expose explicit targets and route policy, which directly addresses independent endpoint selection and avoiding changes to other applications. PipeWire's [capture example](https://docs.pipewire.org/audio-capture_8c-example.html) shows `target.object` and format negotiation. [WirePlumber linking policy](https://pipewire.pages.freedesktop.org/wireplumber/policies/linking.html) documents no-fallback, no-reconnect, and no-move behavior. [PipeWire properties](https://pipewire.pages.freedesktop.org/pipewire/page_man_pipewire-props_7.html) and [stream timing](https://docs.pipewire.org/devel/structpw__time.html) distinguish requested settings from observed stream state. The actively published [PipeWire project](https://pipewire.pages.freedesktop.org/pipewire/) targets low-latency media graphs; its [license file](https://github.com/PipeWire/pipewire/blob/master/LICENSE) describes MIT licensing for the core, with separately licensed components requiring review if bundled.

**Alternatives considered**: [RtAudio](https://github.com/thestk/rtaudio) and [PortAudio](https://github.com/PortAudio/portaudio/blob/master/doc/src/api_overview.dox) provide cross-platform host API wrappers; their PulseAudio/JACK/ALSA paths could work with PipeWire compatibility services, but do not expose the same native WirePlumber target policy. PulseAudio compatibility is valid but would need explicit stream endpoint binding and moving-policy checks. Raw ALSA device access risks bypassing desktop routing and does not solve independent-device clocking. Native PipeWire increases Linux-specific code; future Windows/macOS support needs an isolated alternate backend.

**Integration risk and verification**: Capture and playback can run on distinct schedules. Use a bounded preallocated bridge and observe occupancy, under/overruns, actual format, quantum, and route. PipeWire's [resampling properties](https://docs.pipewire.org/devel/page_man_pipewire-props_7.html) describe graph adaptation, but do not prove every endpoint pair meets the latency goal. Reject unsupported pairs clearly. Check routes before/after startup, and disconnect each endpoint while running.

## Octave-down processing

**Decision update (implementation, 2026-10-09)**: The two-read-head delay failed stronger steady-note dominant-pitch checks. A waveform divider sounded fuzzy live, while a pure sine oscillator sounded synthetic. Additive overtones sounded harsh. The user correctly noted that a later filtered-pickup blend did not pitch shift the guitar waveform. In-house granular shifting sounded distorted. The user preferred the level-matched Rubber Band R3 waveform-shift render, so the live build now uses that engine with no dry or synthetic voice in the 100% wet path. All 36 single-sine checks pass, and synthetic harmonic cases verify that the third harmonic moves down with the waveform. Live listening is pending. One recorded attack showed about 85 ms offline streaming onset difference, so this engine cannot satisfy the 20 ms physical target; a different low-latency design is needed for SC-003.

**Rationale**: The original crossfaded-delay prototype gave incorrect dominant pitches on several test notes, and the revised in-house version sounded distorted to the user. [Dattorro's pitch-shifting research](https://aes.org/publications/elibrary-page/?id=10159) and a [UCSD crossfaded delay-line guitar implementation](https://msp.ucsd.edu/smeck/smeck-01/doc/) describe that approach. Open low E is about 82.4 Hz (12.1 ms period), so low-note behavior deserves dedicated tests. The chosen Rubber Band engine gives the preferred tone but exceeds the 20 ms physical target even before capture and playback.

**Alternatives considered**: A conventional long-window phase vocoder can improve some complex material but tends to increase delay and transient smearing; specialized [low-latency frequency-domain research](https://www.researchgate.net/publication/261078164_Low_latency_audio_pitch_shifting_in_the_frequency_domain) shows this is not a universal limitation. Monophonic period tracking can make a clean sub-octave but has note-onset, octave-error, chord, and timbre risks; the original [YIN paper](https://iro.umontreal.ca/~pift6080/H09/documents/papers/yin_pitch_tracker.pdf) is a reference for more advanced tracking. A simple divider changes the spectrum strongly; listening validation remains necessary.

**Validation risk**: Measure settled pitch separately from note-onset settling and physical input-to-output latency, with the effect enabled and 100% wet so a dry path cannot mask delay. Include low E, chromatic single notes, silence, impulses, sustained notes, attacks, note changes, and overlapping notes. Record artifacts and CPU/latency tradeoffs. If the prototype cannot meet both the quality and 20 ms target, compare lower-latency alternatives and revise the design before proceeding with optional effects; report any unmet target.

## Interface and graphics

**Decision**: Use Dear ImGui with its maintained GLFW platform and OpenGL3 renderer backends. Pin a tested release/commit and compile the core and official backend sources as explicit local build targets. Use GLFW for window/input/context creation; keep graphics setup separate from DSP and audio.

**Rationale**: The [official backend guide](https://github.com/ocornut/imgui/blob/master/docs/BACKENDS.md) and [GLFW/OpenGL3 example](https://github.com/ocornut/imgui/blob/master/examples/example_glfw_opengl3/main.cpp) document the exact pairing. [GLFW's build guide](https://www.glfw.org/docs/latest/compile.html) covers integration and its [platform overview](https://www.glfw.org/docs/latest/intro.html) covers desktop targets. Dear ImGui uses the [MIT license](https://github.com/ocornut/imgui/blob/master/LICENSE.txt); GLFW uses the [zlib/libpng license](https://www.glfw.org/license.html). Both projects publish maintained official source and documentation.

**Alternatives considered**: SDL plus ImGui is viable and may consolidate more platform services, but adds a wider dependency surface when native PipeWire already owns audio. A custom renderer or toolkit adds work without advancing the core audio goal.

**Integration risk and verification**: Handle monitor scale changes, readable fonts/spacing, and a resized window. The [ImGui font guide](https://github.com/ocornut/imgui/blob/master/docs/FONTS.md) describes DPI scaling. Graphics portability alone does not establish audio portability.

## Persistence, build, and verification

**Decision**: Use versioned JSON for a user config and named sound presets, with strict per-field validation and safe defaults. Use nlohmann/json off the audio thread. Build through CMake with focused targets and run hardware-free checks through CTest; manual hardware checks remain separately reported.

**Rationale**: A documented readable format makes presets inspectable and supportable. The [nlohmann/json project](https://github.com/nlohmann/json) documents a CMake target and MIT license; its [CMake integration guide](https://github.com/nlohmann/json/blob/develop/docs/mkdocs/docs/integration/cmake.md) describes package use. CMake and CTest are required by the source brief. Parsing, writing, and validation happen outside process callbacks.

**Alternatives considered**: A custom text format avoids a dependency but requires additional escaping, versioning, and error handling. A database is unnecessary for local settings and a small preset collection.

**Integration risk and verification**: Persist presets independently of endpoint selection; atomic file replacement should prevent partial writes. Reject nonfinite/out-of-range control values. Test missing, malformed, older-version, and unwritable files. Document dependency versions, licenses, installation, and tested compiler/platform combinations during implementation; do not infer support from a library's platform list.
