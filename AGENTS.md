# OpenQ4 Agent Guide

This file describes project goals, rules, and upstream credits for anyone working on OpenQ4.

**Project Metadata**
- Name: OpenQ4
- Author: themuffinator
- Company: DarkMatter Productions
- Version: 0.0.1
- Website: `www.darkmatter-quake.com`
- Repository: `https://github.com/themuffinator/OpenQ4`

**Goals**
- Deliver a complete, open-source code replacement for Quake 4 (engine + game code).
- Preserve behavior required by the shipped Quake 4 assets (base PK4s) where practical.
- Maintain full single-player and multiplayer parity with in-tree game code.
- Modernize the engine and game code while keeping stock-asset compatibility as a guiding constraint.
- Package both SP/MP under one unified game directory (`openbase/`) with `game_sp` + `game_mp`.
- Establish a cross-platform foundation targeting modern systems (Windows, Linux, macOS; x64 first) through SDL3 and Meson.

**Rules**
- Do not target compatibility with the proprietary Quake 4 game DLLs; OpenQ4 ships its own game modules and keeps full freedom to evolve the project.
- Keep `openbase/` as the single unified game directory; do not split SP/MP into separate mod folders.
- Prefer changes that match Quake 4 SDK expectations and shipped content behavior.
- Document significant changes in the documentation and keep `README.md` accurate.
- Use `builddir/` as the standard Meson build output directory for local builds, VS Code tasks, and launch configurations.
- On Windows, do not invoke raw `meson ...` from an arbitrary shell; use `tools/build/meson_setup.ps1 ...` (or run `tools/build/openq4_devcmd.cmd` first) so `cl.exe`/MSVC tools are always available.
- Prefer platform abstractions through SDL3 and avoid introducing new platform-specific dependencies in shared engine code when an SDL3 path exists.
- Keep Meson as the primary build entry point and keep dependency management through Meson subprojects.
- Treat x64 as the baseline architecture for active support while staging additional modern architectures incrementally.
- Keep credits accurate and add new attributions when incorporating upstream work.
- Avoid adding engine-side content files (e.g., custom material scripts) unless absolutely required for compatibility; the goal is to run with the original game assets and only OpenQ4 binaries (engine + game modules, plus minimal external libs).
- Any existing custom `q4base/` content is treated as an expedient bootstrap, not a long-term solution. The goal is to remove this reliance by fixing engine compatibility issues rather than shipping replacement assets.
- For investigations, reference the log file written by `logFileName` (VS Code launch uses `logs/openq4.log`), located under `fs_savepath\<gameDir>\` (e.g. `%LOCALAPPDATA%\OpenQ4\openbase\logs\openq4.log`).

**Development Procedure (Correct Direction)**
1. Develop against the installed Quake 4 assets only (base PK4s), not repo `q4base/` content.
2. Keep `fs_devpath` empty or pointed to a non-content location unless explicitly testing a temporary overlay.
3. If something is missing or broken, fix the engine/game/loader/parser rather than shipping new material/decl/shader assets.
4. If engine-side shaders are needed, prefer internal defaults or generated resources that ship with the executable.
5. Re-run Procedure 1 after each fix to verify clean initialization without custom content.

**Procedure 1 (Debug Loop)**
1. Launch the game via the default launch task.
2. Close the game after 3 seconds.
3. Read `fs_savepath\<gameDir>\logs\openq4.log` (commonly `%LOCALAPPDATA%\OpenQ4\openbase\logs\openq4.log`).
4. Identify errors and warnings to resolve.
5. Resolve the errors and warnings.
6. Repeat until clean.

**Planned Review**
- Strong preference to review and reduce `q4base/` usage, file-by-file, until the engine runs cleanly without any repo content overrides.

**References (Local, Not Included In Repo)**
- Quake 4 SDK: `E:\_SOURCE\_CODE\Quake4-1.4.2-SDK`
- Upstream engine base (local folder name retained): `E:\_SOURCE\_CODE\Quake4Doom-master`
- Quake 4 BSE (Basic Set of Effects): `E:\_SOURCE\_CODE\Quake4BSE-master`
- Quake 4 engine decompiled (Hex-Rays): `E:\_SOURCE\_CODE\Quake4Decompiled-main`

**Upstream Credits**
- Justin Marshall.
- Robert Backebans.
- id Software.
- Raven Software.
