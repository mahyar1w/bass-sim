# Routing validation — 2026-10-08

On the local Linux PipeWire 1.6.9 setup, a muted `live_route_probe` selected the C-Media source and sink independently. It reached Running (`state=3`) with `route=1`, accepted output rate 48,000 Hz, and observed quantum 128 frames. It ran twice without a probe error. The probe kept Mute enabled, so this verifies connection and effective links only.

`wpctl status` and `pw-link -l` were inspected before and after. The configured output remained the Bluetooth QCY-T13 sink, the configured input remained C-Media, and Google Chrome output links remained attached to QCY-T13. Bass-sim's probe did not change desktop defaults or the other application's routes in this observation.

Device unplug/replug, incompatible endpoint pairs, moving an active link, and prolonged independent-clock drift have not been exercised. Recovery and 30-minute route stability are unverified. To repeat the isolation check, capture `wpctl status` and `pw-link -l` before and after a muted start, confirm both Bass-sim links point to the selected nodes, and compare other application links and defaults.
