# Persistence Contract

The application stores local, versioned JSON. Parsing and writing occur outside real-time processing. This contract defines the first schema and validation behavior; file locations must follow documented user configuration directories on each supported platform.

## Preset v1

```json
{
  "schema_version": 1,
  "presets": [{
    "name": "Default Bass",
    "sound": {
    "input_gain_db": 0.0,
    "gate_enabled": false,
    "gate_threshold_db": -60.0,
    "octave_enabled": true,
    "tone_cutoff_hz": 2400.0,
    "compressor_enabled": false,
    "compressor_amount": 0.0,
    "saturation_enabled": false,
    "saturation_amount": 0.0,
    "wet_mix": 1.0,
    "output_gain_db": -6.0
    }
  }]
}
```

| Sound field | Allowed range | Default |
|---|---:|---:|
| `input_gain_db` | -24 to +24 dB | 0 dB |
| `gate_threshold_db` | -80 to -20 dB | -60 dB |
| `tone_cutoff_hz` | 100 to 8000 Hz, further limited below the current Nyquist frequency | 2400 Hz |
| `compressor_amount` | 0 to 1 | 0 |
| `saturation_amount` | 0 to 1 | 0 |
| `wet_mix` | 0 to 1 | 1 |
| `output_gain_db` | -48 to +6 dB | -6 dB |
| Each `*_enabled` field | Boolean | As shown above |

Octave-down ratio is fixed at 0.5 in v1 when enabled. Full effect bypass and safety mute are live session controls and are not stored in a preset. Final output protection remains active even when the effect is bypassed. Parameter limits are product choices for v1; verify the chosen DSP remains stable at those limits before accepting files.

## User configuration v1

```json
{
  "schema_version": 1,
  "capture_endpoint_id": "",
  "capture_channel": 0,
  "playback_endpoint_id": "",
  "playback_channels": 2,
  "requested_sample_rate_hz": 48000,
  "requested_buffer_frames": 128,
  "selected_preset": "Default Bass"
}
```

Endpoint IDs are selection hints, not authority to start audio. Re-enumerate and validate them on each launch. Empty or unavailable endpoint IDs leave processing stopped with a visible selection prompt. Requested rate and buffer are preferences; show negotiated values separately. The UI offers only rates and buffer choices it can request; persisted values outside those offered choices fall back to defaults.

## Loading and saving rules

- A file must be valid JSON with `schema_version: 1` and the expected root type. Unsupported future versions are left untouched, not overwritten automatically; use safe defaults and report the version mismatch.
- Missing individual v1 fields use documented defaults with a warning. Parseable wrong-type and out-of-range fields use defaults with a warning naming the field. JSON has no nonfinite number literals; a numeric overflow that the parser cannot represent makes the entire file malformed and uses safe defaults. Unknown fields are ignored to allow additive updates.
- A malformed entire file leaves the current valid state unchanged if loading during a session; at startup it uses safe defaults. Never apply a partially parsed preset to the audio thread.
- Preset names are trimmed of surrounding ASCII whitespace before save, nonempty after trimming, valid UTF-8, limited to 80 Unicode scalar values, and unique within the preset collection. A preset file never contains audio endpoint IDs, mute state, or engine state.
- Save through a temporary file in the same directory and replace the target only after a complete successful write. On write failure, keep the previous valid file and show an error.
- Document the actual file paths, backup/export method, and any future migration behavior in the application manual.
