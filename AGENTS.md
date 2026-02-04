# OpenQ4 Agent Guide

This file describes project goals, rules, and upstream credits for anyone working on OpenQ4.

**Goals**
- Deliver a drop-in engine replacement for Quake 4.
- Preserve behavior required by original Quake 4 game DLLs.
- Maintain full single-player and multiplayer parity.
- Modernize the engine using the RBDOOM3 base where it improves stability and compatibility.

**Rules**
- Keep compatibility with original Quake 4 binaries (DLLs) as the primary requirement.
- Prefer changes that match Quake 4 SDK expectations and shipped content behavior.
- Document significant changes in the documentation and keep `README.md` accurate.
- Keep credits accurate and add new attributions when incorporating upstream work.

**References (Local, Not Included In Repo)**
- Quake 4 SDK: `E:\_SOURCE\_CODE\Quake4-1.4.2-SDK`
- Quake4Doom upstream: `E:\_SOURCE\_CODE\Quake4Doom-master`
- Quake 4 BSE (Basic Set of Effects): `E:\_SOURCE\_CODE\Quake4BSE-master`
- Quake 4 engine decompiled (Hex-Rays): `E:\_SOURCE\_CODE\Quake4Decompiled-main`

**Upstream Credits**
- Quake4Doom by Justin Marshall.
- RBDOOM3 by Robert Backebans.
- Doom 3 BFG GPL source by id Software and Raven Software.
- Quake 4 original game and SDK by Raven Software and id Software.
