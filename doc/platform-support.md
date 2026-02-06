# OpenQ4 Platform And Architecture Roadmap

This document defines the long-term platform direction for OpenQ4 and how SDL3 + Meson are used to get there.

## Target End State

- First-class support on modern desktop operating systems:
  - Windows
  - Linux
  - macOS
- First-class support for modern 64-bit desktop architecture:
  - x64 (`x86_64`)
- Keep original Quake 4 gameplay/module behavior compatible while modernizing platform and build layers.

## Current Baseline (0.0.1 era)

- Primary actively validated build target: Windows x64.
- Build system: Meson + Ninja.
- Dependency model: Meson subprojects/wraps.
- Platform backend direction: SDL3-first (legacy Win32 backend is transitional only).
- Language baseline target: C++23 semantics (`vc++latest` on current MSVC Meson front-end).
- Toolchain baseline direction: MSVC 19.46+ (Visual Studio 2026 generation), with compatibility fallback permitted during migration.

## SDL3 Direction

- SDL3 is the default backend path and the portability layer for:
  - window lifecycle
  - input event handling
  - context/window interop glue
- New platform-facing work should prefer SDL3 abstractions first.
- Platform-specific code should be isolated under `src/sys/<platform>/` when SDL3 cannot cover a requirement directly.

## Meson Direction

- Meson is the canonical build system going forward.
- External dependencies should be consumed via Meson dependency resolution and subprojects/wraps.
- New build logic should be host-aware and architecture-aware, with x64 as the active compatibility baseline.
- Meson configuration defaults to `cpp_std=vc++latest` (C++23-targeting mode on MSVC).
- Meson currently adds `/Zc:strictStrings-` on MSVC to preserve compatibility with legacy string-literal usage while the codebase is modernized.
- `tools/build/meson_setup.ps1` prefers VS 2026+ (major 18) when present; strict minimum enforcement can be enabled with `-Denforce_msvc_2026=true`.

## Bring-Up Staging

1. Keep Windows x64 stable with SDL3 default backend.
2. Incrementally wire Linux build/source selection and validation into Meson.
3. Incrementally wire macOS build/source selection and validation into Meson.
4. Promote Linux and macOS to first-class once they pass consistent compile/link/runtime validation loops.

## Definition Of Done For First-Class Platform Support

- Clean configure + build in Meson.
- Engine initializes and reaches map/session startup with stock Quake 4 assets.
- Core input, rendering, audio, and networking paths work without platform-specific content hacks.
- Regressions are tracked in docs and fixed in engine/platform code, not with asset overrides.
