# Octave and latency feasibility — 2026-10-09

The current octave effect uses Rubber Band 4.0.0 R3 in real-time, short-window mode with a 0.5 pitch scale. The user preferred a level-matched render from this engine over the in-house granular and oscillator versions. The live app now feeds it 128-frame blocks through a preallocated output queue. It shifts the whole waveform, without a dry-guitar or synthesized-bass blend. Live listening of this exact build is pending.

The automated check covers 36 settled single-sine notes: nine notes at each of 44.1, 48, 96, and 192 kHz. Analysis starts after a 250 ms settling interval. A Hann-window FFT finds the strongest bin between 25 and 500 Hz. Its window length grows with sample rate from 32,768 to 131,072 samples, giving about 1.35–1.46 Hz frequency-bin resolution. All 36 were within 5% of half the input frequency. Representative 48 kHz results:

| Input Hz | Dominant output Hz | Target Hz | Relative error |
|---:|---:|---:|---:|
| 82.4069 | 41.0156 | 41.2035 | 0.46% |
| 110 | 55.6641 | 55 | 1.21% |
| 146.832 | 73.2422 | 73.416 | 0.24% |
| 196 | 98.1445 | 98 | 0.15% |
| 246.942 | 123.047 | 123.471 | 0.34% |
| 329.628 | 164.062 | 164.814 | 0.46% |
| 415.305 | 208.008 | 207.653 | 0.17% |
| 659.255 | 329.590 | 329.628 | 0.01% |
| 987.767 | 493.652 | 493.884 | 0.05% |

Sixteen harmonic-rich phase cases and seven weak-fundamental cases check that the third input harmonic moves from 3 times the played frequency to 1.5 times it. In every case the shifted third-harmonic energy exceeds energy left at its unshifted frequency by at least 4:1. A waveform shift can leave a higher harmonic louder than the new fundamental, so the synthetic rich-input check no longer demands that the fundamental be the largest FFT peak. The tests also cover silence, an impulse, a note change, and varying offline chunks. These checks do not prove live sound quality.

The engine reports a 2,560-frame start delay at 48 kHz (53.3 ms) for the selected mode. In the exact streaming processor render of the supplied recording, a centered 10 ms RMS detector crossed 0.01 on the first input attack at 1.158 s and 0.005 on output at 1.243 s: about **85 ms of offline onset difference**. The full-waveform CLI render compensates its delay and cannot represent live latency. The streamed render matched its settled-note spectrum and level after accounting for the delay. The 85 ms measurement already exceeds the 20 ms physical target before capture and playback are included, so the current algorithm cannot meet SC-003. No physical loopback measurement has been performed because no loopback cable/interface is available; an end-to-end value remains unknown.
