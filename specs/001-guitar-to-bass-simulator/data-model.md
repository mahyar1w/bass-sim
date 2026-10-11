# Data Model: Bass-sim

This model describes application state and persisted data; it does not prescribe C++ class layout. See [persistence contract](contracts/persistence.md) for the file boundary and [UI contract](contracts/user-interface.md) for visible behavior.

## AudioEndpoint

| Field | Meaning | Validation |
|---|---|---|
| `id` | Stable identity for the current server session, including object serial or equivalent | Required while listed; rediscover after restart |
| `name` | Readable display name | Nonempty fallback to ID |
| `direction` | Capture or playback | Exactly one direction |
| `channels` | Available channel labels/indices | At least one usable channel for selection |
| `capabilities` | Advertised formats/rates or unknown | Never present an unknown value as guaranteed |
| `availability` | Present, unavailable, or disconnected | Updated on enumeration events/refresh |

An endpoint may disappear at any time. Display names are not unique keys. Persisted IDs are hints and must be revalidated before opening a stream.

## AudioConfiguration

| Field | Meaning | Validation |
|---|---|---|
| `capture_endpoint_id`, `capture_channel` | Selected mono instrument source | ID exists and channel belongs to source |
| `playback_endpoint_id`, `playback_channels` | Selected mono/stereo destination | ID exists and selected channels are supported |
| `requested_rate`, `requested_quantum` | User preference | Positive, within UI bounds; may be unavailable |
| `actual_rate`, `actual_quantum`, `actual_channels` | Negotiated/observed stream settings | Populated only after successful negotiation |
| `route_verified` | Effective stream links match the selection | Required before unmuting |
| `input_latency`, `output_latency` | Reported timing with source/units | Optional; never relabeled measured end-to-end |

Requested settings are retained separately from actual settings. A mismatch is visible, not silently normalized in the UI. If capture and playback cannot run together safely, the configuration is rejected with a specific diagnostic.

## EngineState

`Stopped → Preparing → Connecting → Running → Stopping → Stopped`; changes made while running pass through `Reconfiguring` before returning to `Running`.

Failure from any active transition enters `ErrorMuted`; recovery may return to `Preparing` on explicit retry or to `Stopped`. Reconfiguration from `Running` passes through a muted transition before replacing streams. `mute` and `effect_bypass` are independent flags; mute silences both dry and wet paths. Only `Running` with `route_verified = true` may produce audible output.

## SoundSettings

| Field | Meaning | Rule |
|---|---|---|
| `input_gain`, `output_gain` | Input and output level | Bounded; conservative startup/default values |
| `gate_enabled`, `gate_threshold` | Optional noise gate | Bounded threshold and smoothed changes |
| `octave_enabled`, `pitch_ratio` | Core pitch effect | Default ratio 0.5; unsupported ratios rejected |
| `tone` | Bass-oriented tone shape controls | Bounded, stable at supported rates |
| `compressor_enabled`, `compressor_amount` | Optional dynamics | Bounded |
| `saturation_enabled`, `saturation_amount` | Optional soft coloration | Bounded |
| `wet_mix` | Processed versus dry blend | 0 to 1; align paths to avoid combing surprises |
| `effect_bypass`, `mute` | Immediate intent with smoothed audio transition | Bypass passes controlled dry signal; mute silences all |

Exact numeric control bounds and defaults will be published in the implemented UI and persistence contract before files are accepted. All stored values must be finite. The audio thread sees complete validated snapshots, never partially updated controls.

## Preset and UserConfiguration

- **Preset**: `schema_version`, unique user-visible `name`, and the persistable subset of validated `SoundSettings`. It excludes audio endpoint IDs, engine running state, mute, and full effect bypass; the octave ratio is fixed at 0.5 in v1.
- **UserConfiguration**: `schema_version`, last requested audio endpoint/channel/settings, selected preset identity, and applicable UI preferences. Missing endpoint IDs do not prevent startup; they require fresh selection or a visible safe default.
- **Persistence failure**: Read failure uses defaults and reports why; write failure leaves the previous valid file intact and reports the problem.

## Diagnostic and MeterSnapshot

- **Diagnostic**: severity, short message, recovery action where relevant, time, and source category (device, route, processing, persistence). The UI retains recent messages outside the audio callback.
- **MeterSnapshot**: input/output peak and level, clipping flags, underrun/overrun counts where exposed, and observation time. Values are bounded or marked unavailable.
- **LatencyDisplay**: value, units, category (`reported`, `estimated`, or `measured`), measurement setup when applicable, and age. A buffer-duration estimate is never total latency.

## Relationships and invariants

- One `AudioConfiguration` references one capture and one playback `AudioEndpoint` at a time.
- One `EngineState` and one active `SoundSettings` snapshot drive the current audio session.
- `Preset` loads replace sound settings through a smoothed transition; they do not silently change devices.
- Diagnostics and meter snapshots describe observed state and never drive control changes from the process callback.
- No invalid file value, route failure, or device disappearance can transition directly to audible output.
