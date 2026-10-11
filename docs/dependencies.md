# Dependencies

Bass-sim targets Linux with C++23 and CMake. The versions below are the initial build environment; [validation results](validation/results.md) will distinguish builds from live hardware tests.

| Dependency | Purpose | Version used | License | Platform scope and tradeoff |
|---|---|---|---|---|
| [PipeWire](https://pipewire.pages.freedesktop.org/pipewire/) / SPA | Native capture, playback, device graph and timing | 1.6.9 | MIT core; check separately licensed components when packaging | Linux. Direct node targeting and WirePlumber routing policy; requires a Linux-specific backend. |
| [Rubber Band](https://breakfastquay.com/rubberband/) | Full-waveform octave shift | 4.0.0 | GPL-2.0-or-later or commercial license; see [upstream licensing](https://breakfastquay.com/rubberband/license.html) | System library; R3 real-time short-window mode sounded best in the supplied offline comparison but adds substantial latency. Review licensing before distribution. |
| [WirePlumber](https://pipewire.pages.freedesktop.org/wireplumber/) | Session routing policy | System installation | MIT | Linux service; policy support for no-fallback and no-move must be verified at runtime. |
| [Dear ImGui](https://github.com/ocornut/imgui/releases/tag/v1.92.9b) | Desktop controls and meters | v1.92.9b, source pinned in `third_party/imgui/` | MIT; see `third_party/imgui/LICENSE.txt` | Actively maintained desktop UI; not an audio backend. Official GLFW/OpenGL3 backends are included. |
| [GLFW](https://www.glfw.org/) | Window and input | 3.5.1 | zlib/libpng | Desktop windowing on Linux, Windows and macOS; graphics portability does not imply audio portability. |
| [OpenGL](https://www.khronos.org/opengl/) | ImGui rendering | CMake OpenGL development package 1.2; application requests a 3.3 core context | Platform driver/API terms | Requires a working graphics context; the package number is not a measured runtime GL version. |
| [nlohmann/json](https://github.com/nlohmann/json) | Versioned local settings and presets | 3.12.0 | MIT | Header-only; used only off the audio thread. Avoids custom escaping and parser logic. |
| CMake / CTest | Build and hardware-free tests | 4.4.3 | BSD 3-Clause | Build tooling; project needs CMake 3.22 or newer. |

These are maintained upstream projects with current official documentation. Pinning Dear ImGui source avoids hidden machine-specific source paths. Native PipeWire is distinct from using PipeWire through PulseAudio/JACK/ALSA compatibility: the native path exposes direct node targets and session policy but requires a separate audio backend for other platforms. RtAudio/PortAudio would offer wider host API abstraction at the cost of less direct control over the selected PipeWire route.

The Dear ImGui archive was downloaded from `https://codeload.github.com/ocornut/imgui/tar.gz/refs/tags/v1.92.9b` with SHA-256 `21d8a0a565e85dce943e375db00812c2f3f0ab21f3f0f7964e364a63422d7f99`. Only the required core and official GLFW/OpenGL3 backend sources plus license were vendored.
