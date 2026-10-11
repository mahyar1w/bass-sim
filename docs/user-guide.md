# User guide

## Setup and routing

Build and launch using the commands in the [README](../README.md). Connect a guitar to an audio interface or input that PipeWire exposes as an `Audio/Source`. In Audio Setup, refresh the list, choose the capture node and its channel, choose a separate playback `Audio/Sink`, then choose a requested rate and quantum. The shown accepted rate and quantum are observed after negotiation and may differ. Press Start. The Live panel reports state, Mute, and Stop. The Diagnostics panel describes a route or negotiation failure and what to change. The app does not select a desktop default or reroute other applications.

Use an external mixer or PipeWire patchbay if the interface requires hardware or software monitoring configuration. Keep monitoring volume conservative when first testing an instrument. An unavailable endpoint or unsupported channel keeps the engine muted; refresh and select a new endpoint, then restart. Endpoint IDs can change between sessions.

## Sound and meters

The octave is enabled by default, with a fully wet mix. Input gain ranges from -24 to +24 dB, gate threshold from -80 to -20 dB, tone cutoff from 100 to 8000 Hz subject to Nyquist, compressor and saturation amounts from 0 to 1, wet mix from 0 to 1, and output gain from -48 to +6 dB. The gate, compressor, and saturation can be switched independently. Full effect bypass uses controlled dry guitar; Mute silences both paths. The output is clamped at -1 dBFS even in bypass. Meters show observed input and output peaks and latched clip labels; clear the latches with the meter button. A zero reading before audio starts is not presented as an observation.

The octave effect shifts the whole guitar waveform down one octave, including its harmonics; no dry guitar is mixed into the 100% wet path. It uses the system Rubber Band library. Pitch shifting may blur attacks, and long-session stability has not been checked.

## Presets and local files

Presets contain sound settings only. Enter a name and press Save current sound; an existing name requires explicit replacement. Loading a preset leaves routing, Mute, and full bypass unchanged. The app saves `config.json` and `presets.json` in `$XDG_CONFIG_HOME/bass-sim/` or, when unset, `~/.config/bass-sim/`. They are readable JSON with `schema_version: 1`; [the schema contract](../specs/001-guitar-to-bass-simulator/contracts/persistence.md) lists fields and limits. Copy these files to back them up or export them. Unsupported future versions are not overwritten by the app. Missing or invalid values use safe defaults with a warning.

## Diagnostics and verification

The displayed buffer duration is an estimate from the accepted quantum and rate. It is one component of latency; physical end-to-end latency remains unavailable until a loopback test. The current octave engine adds about 85 ms of onset delay in an offline streaming render of the supplied recording, before hardware latency, so it does not meet the 20 ms physical target. Underrun and overrun counts are observed callback values after processing starts. Use `ctest --test-dir build --output-on-failure` for hardware-free tests. For route inspection, compare `wpctl status` and `pw-link -l` before and after a muted start. The manual `./build/tests/live_route_probe` is intended for a C-Media source/sink setup and leaves output muted.

For a physical latency test, feed a repeatable transient to the selected input while recording a reference and the processed output via a wired loopback. Enable the octave, use 100% wet, record the hardware and accepted settings, and compute the median onset difference across several trials. Avoid acoustic feedback. See the [validation procedure](../specs/001-guitar-to-bass-simulator/quickstart.md). This test has not been performed because loopback hardware is unavailable.
