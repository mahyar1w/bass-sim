# Persistence validation — 2026-10-08

CTest passed save/load of a named preset and user configuration, trim and invalid/duplicate/overlong-name checks, fallback for missing and invalid individual values, safe default on an overflowing JSON number, requested rate/buffer fallback, missing config, failed destination writes, and rejection of future or malformed files without overwriting them. Files are intended for `$XDG_CONFIG_HOME/bass-sim/` or `~/.config/bass-sim/`; tests use temporary files under `/tmp`.

UI save/restart/load and switching presets during live monitored output remain unverified. The app keeps Mute and full bypass as session controls when loading a sound. A real user config and preset were not deliberately written during validation; the five-second UI smoke run left processing stopped.
