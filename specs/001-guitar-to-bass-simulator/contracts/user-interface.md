# User Interface Contract

This is the observable behavior of the desktop application. It complements [the data model](../data-model.md) and the [feature spec](../spec.md).

## Audio setup

- Show separate capture and playback selectors, with readable endpoint names and a refresh control. Show the selected mono capture channel and available mono/stereo playback mode.
- Show requested sample rate and buffer size beside the actual accepted values. If the audio environment does not expose a reliable actual value, label it `Unknown` rather than repeating the request as fact.
- Start is enabled only when selections are valid. An unsupported pair, failed link, or missing endpoint leaves output muted and displays a specific reason and action such as `Refresh devices` or `Select another output`.
- Changing an endpoint or audio setting while running follows a visible `Reconfiguring` state. Audio fades to silence before old streams are replaced and fades up only after the chosen route is verified.
- Selecting a route never changes the system default or another application's route. If the active session policy moves a stream away from the selected endpoint, report the mismatch and mute it.

## Processing and safety

- Show explicit `Start`, `Stop`, `Mute`, and `Effect bypass` controls. `Stop` releases the active route. `Mute` silences both processed and dry signals. `Effect bypass` passes the selected dry input at a controlled level, and remains subordinate to mute.
- Expose input gain, octave effect, tone, wet/dry balance, output gain, and optional gate, compressor, and saturation. Optional stages have independent enable controls. Group labels and concise tooltips explain unfamiliar controls.
- Controls use the persisted ranges in [persistence.md](persistence.md). Reject out-of-range input instead of wrapping or producing invalid audio. Smooth audible transitions for gain, mix, bypass, mute, presets, and device start/stop.
- Show input and output level meters and clipping indicators. A clipping indicator remains visible long enough to notice and can be cleared. Overload protection is the final output stage, with a documented ceiling no higher than -1 dBFS for normalized output samples.
- The default preset starts with octave down enabled, optional effects disabled, full wet mix, and conservative output gain.

## Status, latency, and diagnostics

- Show one of `Stopped`, `Preparing`, `Connecting`, `Running`, `Reconfiguring`, `Stopping`, or `Error (muted)`; never show `Running` while the intended route is unverified.
- Each failure names the affected input, output, setting, or saved file when available, and gives one recovery action. A recent diagnostic history is viewable without obscuring the main controls.
- Show latency values with explicit labels: `Reported by audio environment`, `Estimated from buffers and processing`, or `Measured by loopback`. Show `Unavailable` for unknown components. Do not label buffer duration as total latency.
- Show underrun/overrun counts only when observed or exposed by the audio environment; otherwise label diagnostics unavailable. Do not infer zero events from missing counters.
- Status, clipping, mute, and errors use text or icons as well as color. The layout remains usable after resizing and display-scale changes.

## Presets and persistence

- Provide preset selection, save, and load controls. Saving asks for a nonempty name and makes replacement of an existing name explicit.
- Loading a preset changes sound settings through smoothed transitions and never changes the selected audio route or starts playback by itself.
- Missing or damaged saved data produces a visible, nonfatal warning and safe defaults. A failed save reports the problem without discarding the previously valid file.

## Acceptance checks

1. Select different capture and playback nodes, start processing, and confirm displayed and actual links match while system defaults and other applications' links remain unchanged.
2. Change controls, load a preset, bypass, mute, and stop during a sustained note; confirm no loud burst and the documented dry/mute behavior.
3. Disconnect either endpoint and attempt an unsupported setting; confirm safe mute, accurate state, and actionable recovery.
4. Resize the window or change monitor scale; confirm readable labels, meters, controls, and diagnostic state.
