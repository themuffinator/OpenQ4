# OpenQ4

OpenQ4 is an open-source, drop-in engine replacement for Quake 4.

**Project Metadata**
- Name: OpenQ4
- Author: themuffinator
- Company: DarkMatter Productions
- Version: 0.0.1
- Website: `www.darkmatter-quake.com`
- Repository: `https://github.com/themuffinator/OpenQ4`

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
- Users must supply their own Quake 4 game data (e.g., `q4base` PK4s) to run the game.

**Build (Meson + Ninja)**
- Install Meson and Ninja.
- Current actively validated target is Windows x64 with Microsoft `cl.exe`.
- Language baseline target is C++23 semantics (`cpp_std=vc++latest` on MSVC).
- Toolchain direction is MSVC 19.46+ (Visual Studio 2026 baseline), with compatibility fallback to older installed MSVC when strict enforcement is disabled.
- MSVC strict string-literal conformance is temporarily disabled (`/Zc:strictStrings-`) to keep legacy idTech4-era code building during migration.
- Linux and macOS bring-up are staged (see `doc/platform-support.md`), with SDL3 + Meson as the required path.
- Recommended workflow (inspired by WORR-2):
  - `powershell -ExecutionPolicy Bypass -File tools/build/meson_setup.ps1 setup --wipe build_meson . --backend ninja --buildtype debug --wrap-mode=forcefallback`
  - `powershell -ExecutionPolicy Bypass -File tools/build/meson_setup.ps1 compile -C build_meson`
- `tools/build/meson_setup.ps1` auto-loads Visual Studio build tools when needed (prefers VS 2026+/major 18 when available), and exports `WINDRES` via `tools/build/rc.cmd`.
- `tools/build/meson_setup.ps1 compile` now auto-regenerates `build_meson` if it is missing or stale, which resolves "not a meson build directory" errors.
- Non-Windows wrapper: `tools/build/meson_setup.sh` (passes through to `meson`).
- Optional strict baseline gate:
  - `-Denforce_msvc_2026=true` to fail configure when MSVC is older than 19.46.
- Equivalent commands from an already-open VS Developer shell:
  - `meson setup --wipe build_meson . --backend ninja --buildtype debug --wrap-mode=forcefallback`
  - `meson compile -C build_meson`
- Output binary:
  - `build_meson/OpenQ4.exe`

**External Dependencies (Meson Subprojects)**
- `subprojects/glew`
- `subprojects/stb_vorbis`
- `subprojects/openal-soft-prebuilt`
- `subprojects/sdl3.wrap` (SDL3 fallback via WrapDB, plus local packagefile patch for Win32 WGL/OpenGL enablement)
- These subprojects are used by Meson to keep third-party dependency wiring self-contained.

**Platform And Architecture Direction**
- SDL3 + Meson are the forward-looking portability foundation.
- x64 (`x86_64`) is the active architecture baseline.
- Cross-platform goal is full support on Windows, Linux, and macOS.
- Detailed roadmap and staging policy: `doc/platform-support.md`.
- Input parity audit checklist and current SDL3 gaps: `doc/input-key-matrix.md`.
- Release completion checklist for changelog drafting: `doc/release-completion.md`.

**Rules**
- Always strive for binary compatibility with original Quake 4 game modules (DLLs).
- Preserve behavior expected by the Quake 4 SDK and shipped game data.
- Document significant changes for future maintainers.
- Keep credits accurate and up to date.

**References (Local, Not Included In Repo)**
- Quake 4 SDK: `E:\_SOURCE\_CODE\Quake4-1.4.2-SDK`
- Upstream engine base (local folder name retained): `E:\_SOURCE\_CODE\Quake4Doom-master`
- Quake 4 BSE (Basic Set of Effects): `E:\_SOURCE\_CODE\Quake4BSE-master`
- Quake 4 engine decompiled (Hex-Rays): `E:\_SOURCE\_CODE\Quake4Decompiled-main`

**Credits**
- Justin Marshall.
- Robert Backebans.
- id Software.
- Raven Software.
- Sean Barrett (stb_vorbis).
- Nigel Stewart, Milan Ikits, Marcelo E. Magallon, and Lev Povalahev (GLEW).
- OpenAL Soft contributors.
