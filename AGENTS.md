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

**Rules**
- Keep compatibility with original Quake 4 binaries (DLLs) as the primary requirement.
- Prefer changes that match Quake 4 SDK expectations and shipped content behavior.
- Document significant changes in the documentation and keep `README.md` accurate.
- Keep credits accurate and add new attributions when incorporating upstream work.
- For investigations, reference the log file at `${env:USERPROFILE}\Saved Games\OpenQ4\q4base\logs\openq4.log` (static name, overwritten each launch) before making further changes.

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
