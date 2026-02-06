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
- Deliver a drop-in engine replacement for Quake 4.
- Preserve behavior required by original Quake 4 game DLLs.
- Maintain full single-player and multiplayer parity.
- Modernize the engine while preserving Quake 4 compatibility.
- Establish a cross-platform foundation targeting modern systems (Windows, Linux, macOS; x64 first) through SDL3 and Meson.

**Rules**
- Keep compatibility with original Quake 4 binaries (DLLs) as the primary requirement.
- Prefer changes that match Quake 4 SDK expectations and shipped content behavior.
- Document significant changes in the documentation and keep `README.md` accurate.
- Prefer platform abstractions through SDL3 and avoid introducing new platform-specific dependencies in shared engine code when an SDL3 path exists.
- Keep Meson as the primary build entry point and keep dependency management through Meson subprojects.
- Treat x64 as the baseline architecture for active support while staging additional modern architectures incrementally.
- Keep credits accurate and add new attributions when incorporating upstream work.
- Avoid adding engine-side content files (e.g., custom material scripts) unless absolutely required for compatibility; the drop-in goal is to run with the original game assets and only the OpenQ4 executable (plus minimal external libs).
- Any existing custom `q4base/` content is treated as an expedient bootstrap, not a long-term solution. The goal is to remove this reliance by fixing engine compatibility issues rather than shipping replacement assets.
- For investigations, reference the log file at `C:\Program Files (x86)\Steam\steamapps\common\Quake 4\q4base\logs\openq4.log` (static name, overwritten each launch) before making further changes.

**Development Procedure (Correct Direction)**
1. Develop against the installed Quake 4 assets only (base PK4s), not repo `q4base/` content.
2. Keep `fs_devpath` empty or pointed to a non-content location unless explicitly testing a temporary overlay.
3. If something is missing or broken, fix the engine/loader/parser rather than shipping new material/decl/shader assets.
4. If engine-side shaders are needed, prefer internal defaults or generated resources that ship with the executable.
5. Re-run Procedure 1 after each fix to verify clean initialization without custom content.

**Procedure 1 (Debug Loop)**
1. Launch the game via the default launch task.
2. Close the game after 3 seconds.
3. Read `C:\Program Files (x86)\Steam\steamapps\common\Quake 4\q4base\logs\openq4.log`.
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
