# OpenQ4 Release Completion List

Use this file as the source list for release changelog entries.

Process:
1. Add completed work under "Ready For Changelog".
2. When cutting a release, copy relevant entries into that release section in `CHANGELOG.md` (or release notes).
3. Move shipped items into a historical release section here (optional), and keep remaining work in "Carry Forward".

## Ready For Changelog

- [x] Material handling fixes completed; engine startup no longer depends on custom material script overrides in repo `q4base/`.
- [x] Menu rendering issues fixed.
- [x] SDL3 backend integrated as the default platform path (legacy Win32 backend remains transitional).
- [x] Meson + Ninja build system introduced as canonical build path.
- [x] External dependencies moved to Meson subprojects/wraps (`sdl3`, `glew`, `stb_vorbis`, `openal-soft-prebuilt`).
- [x] Ogg Vorbis (`.ogg`) playback support integrated (decoded via `stb_vorbis`).
- [x] C++23-targeting baseline enabled on MSVC (`cpp_std=vc++latest`).
- [x] MSVC 2026 toolchain direction documented and implemented as an optional enforceable baseline (`-Denforce_msvc_2026=true`).
- [x] Meson setup wrapper improved to auto-recover missing/stale build directories and avoid VS tool discovery null-crash.
- [x] Windows SDL3 key/input parity improvements: backspace fix, control-char synthesis, locale-aware RightAlt behavior.
- [x] Manual key matrix audit completed and documented for console, GUI edit fields, chat, binds, numpad, and modifiers.
- [x] GUI scaling behavior updated to preserve uniform/aspect-correct rendering on window resize.
- [x] Engine-side console/UI relayout now handles wide and narrow/tall aspect ratios, with live updates on screen size/aspect changes.
- [x] Platform/architecture roadmap documentation added for Windows/Linux/macOS direction with x64 baseline.
- [x] Legacy/redundant build-system artifacts reduced (CMake path retired from active source tree).

## Carry Forward

- [ ] Non-English key layout parity still needs completion in SDL3 key translation.
- [ ] Console toggle localization parity for non-English layouts still needs completion.
- [ ] UTF-8 text input should be decoded to characters before queuing `SE_CHAR` events.
- [ ] Linux and macOS bring-up needs full compile/link/runtime validation to reach first-class status.
- [ ] Replace temporary MSVC compatibility flag (`/Zc:strictStrings-`) with full strict-strings codebase compliance.
