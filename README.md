# OpenQ4

OpenQ4 is an open-source, drop-in engine replacement for Quake 4 currently under development and based upon [Quake4Doom](https://github.com/idSoftware/Quake4Doom)..

**Goals**
- Provide a drop-in replacement engine for Quake 4 with compatibility for original game DLLs.
- Maintain feature parity for both single-player and multiplayer.
- Modernize rendering, audio, and platform support while preserving Quake 4 behavior.
- Keep documentation current as significant changes land.
- Reach full support for modern systems across Windows, Linux, and macOS with x64 as the active baseline architecture.

**Audio**
- WAV and Ogg Vorbis (`.ogg`) assets are supported (Ogg decoded via stb_vorbis).

**Assets**
- This repository is engine-only and does not ship game assets (audio, textures, or media).
- Users must provide their own genuine Quake 4 installation to run the game.
- OpenQ4 validates required official `q4base` PK4 checksums at startup by default (`fs_validateOfficialPaks 1`) and refuses to continue when required stock assets are missing or modified.
- Current baseline checksum table: `doc/official-pk4-checksums.md`.
- Startup now auto-discovers `fs_basepath` (`override -> CWD -> Steam -> GOG`) and uses `fs_homepath` as the writable root (with `fs_savepath` defaulting to it).

**Build (Meson + Ninja)**
- Install Meson and Ninja.
- Current actively validated target is Windows x64 with Microsoft `cl.exe`.
- Language baseline target is C++23 semantics (`cpp_std=vc++latest` on MSVC).
- Toolchain direction is MSVC 19.46+ (Visual Studio 2026 baseline), with compatibility fallback to older installed MSVC when strict enforcement is disabled.
- MSVC strict string-literal conformance is temporarily disabled (`/Zc:strictStrings-`) to keep legacy idTech4-era code building during migration.
- Linux and macOS bring-up are staged (see `doc/platform-support.md`), with SDL3 + Meson as the required path.
- Recommended workflow (inspired by WORR-2):
  - `powershell -ExecutionPolicy Bypass -File tools/build/meson_setup.ps1 setup --wipe builddir . --backend ninja --buildtype debug --wrap-mode=forcefallback`
  - `powershell -ExecutionPolicy Bypass -File tools/build/meson_setup.ps1 compile -C builddir`
- `tools/build/meson_setup.ps1` auto-loads Visual Studio build tools when needed (prefers VS 2026+/major 18 when available), and exports `WINDRES` via `tools/build/rc.cmd`.
- `tools/build/meson_setup.ps1 compile` now auto-regenerates `builddir` if it is missing or stale, which resolves "not a meson build directory" errors.
- Non-Windows wrapper: `tools/build/meson_setup.sh` (passes through to `meson`).
- For manual Windows shells, bootstrap MSVC first with `tools/build/openq4_devcmd.cmd`.
- If `cl.exe` is unavailable or `CreateProcess` fails, you are in a non-developer shell; use `tools/build/meson_setup.ps1 ...` (or run `tools/build/openq4_devcmd.cmd` once in that shell) and retry.
- Optional strict baseline gate:
  - `-Denforce_msvc_2026=true` to fail configure when MSVC is older than 19.46.
- Equivalent commands from an already-open VS Developer shell:
  - `meson setup --wipe builddir . --backend ninja --buildtype debug --wrap-mode=forcefallback`
  - `meson compile -C builddir`
- Output binaries:
  - `builddir/OpenQ4.exe`
  - `builddir/OpenQ4-ded.exe` (dedicated server build, compiled with `ID_DEDICATED`)
- Debug crash diagnostics:
  - `_DEBUG` builds install a Win32 unhandled-exception filter.
  - On crash, OpenQ4 writes a timestamped `.log` and `.dmp` under `builddir/crashes/`.

**External Dependencies (Meson Subprojects)**
- `subprojects/glew` (GLEW 2.3.1 source snapshot, vendored)
- `subprojects/stb_vorbis` (stb_vorbis 1.22, vendored)
- `subprojects/openal-soft-prebuilt` (OpenAL Soft 1.25.1 headers/defs; Win64 import lib provided locally)
- `subprojects/sdl3.wrap` (SDL3 3.4.0 fallback via WrapDB, plus local packagefile patch for Win32 WGL/OpenGL enablement)
- These subprojects are used by Meson to keep third-party dependency wiring self-contained.

**Platform And Architecture Direction**
- SDL3 + Meson are the forward-looking portability foundation.
- SDL3 backend now includes integrated gamepad/joystick input (hotplug, JOY/AUX bind events, and dual-stick analog movement/look routing).
- Controller tuning cvars: `in_joystick`, `in_joystickDeadZone`, `in_joystickTriggerThreshold`.
- SDL3 multi-monitor selection is configurable through `r_screen` (`-1` auto/current display, `0..N` explicit display index); use `listDisplays` to inspect available monitor indices.
- Display mode cvars: `r_fullscreen` (`0/1`), `r_borderless` (`1` = borderless window when not fullscreen), windowed sizing via `r_windowWidth`/`r_windowHeight`, and fullscreen custom sizing via `r_mode -1` with `r_customWidth`/`r_customHeight`.
- x64 (`x86_64`) is the active architecture baseline.
- Cross-platform goal is full support on Windows, Linux, and macOS.
- Engine-side screen UI now supports dynamic aspect handling across both wide and narrow/tall display ratios (with live relayout on resize/aspect change).
- Detailed roadmap and staging policy: `doc/platform-support.md`.
- Input parity audit checklist and current SDL3 gaps: `doc/input-key-matrix.md`.
- Release completion checklist for changelog drafting: `doc/release-completion.md`.

**Rules**
- Always strive for binary compatibility with original Quake 4 game modules (DLLs).
- Preserve behavior expected by the Quake 4 SDK and shipped game data.
- Document significant changes for future maintainers.
- Keep credits accurate and up to date.
- Keep pure server behavior anchored to the official Quake 4 base PK4 set.

**Credits**
- Justin Marshall (Quake4Doom, BSE reverse engineering efforts).
- Robert Backebans (RBDOOM3).
- id Software (idTech4/Quake 4).
- Raven Software (Quake 4).
- Sean Barrett (stb_vorbis).
- Nigel Stewart, Milan Ikits, Marcelo E. Magallon, and Lev Povalahev (GLEW).
- OpenAL Soft contributors.
