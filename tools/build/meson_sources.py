#!/usr/bin/env python3
"""Emit Meson source paths for the OpenQ4 executable target."""

from __future__ import annotations

import pathlib
import sys


Q4GAME_SOURCES = [
    "game/Actor.cpp",
    "game/AF.cpp",
    "game/AFEntity.cpp",
    "game/BrittleFracture.cpp",
    "game/Camera.cpp",
    "game/Effect.cpp",
    "game/Entity.cpp",
    "game/FreeView.cpp",
    "game/GameEdit.cpp",
    "game/Game_Debug.cpp",
    "game/Game_local.cpp",
    "game/Game_Log.cpp",
    "game/Game_network.cpp",
    "game/Game_render.cpp",
    "game/Healing_Station.cpp",
    "game/Icon.cpp",
    "game/IconManager.cpp",
    "game/IK.cpp",
    "game/Instance.cpp",
    "game/Item.cpp",
    "game/Light.cpp",
    "game/LipSync.cpp",
    "game/Misc.cpp",
    "game/Moveable.cpp",
    "game/Mover.cpp",
    "game/MultiplayerGame.cpp",
    "game/Playback.cpp",
    "game/Player.cpp",
    "game/PlayerView.cpp",
    "game/Player_Cheats.cpp",
    "game/Player_States.cpp",
    "game/Projectile.cpp",
    "game/Pvs.cpp",
    "game/SecurityCamera.cpp",
    "game/Sound.cpp",
    "game/spawner.cpp",
    "game/SplineMover.cpp",
    "game/Target.cpp",
    "game/TramGate.cpp",
    "game/Trigger.cpp",
    "game/Weapon.cpp",
    "game/WorldSpawn.cpp",
]

SOURCE_GLOBS = [
    "aas/*.cpp",
    "bse/*.cpp",
    "cm/*.cpp",
    "framework/*.cpp",
    "framework/async/*.cpp",
    "idlib/*.cpp",
    "idlib/bv/*.cpp",
    "idlib/containers/*.cpp",
    "idlib/geometry/*.cpp",
    "idlib/hashing/*.cpp",
    "idlib/math/*.cpp",
    "renderer/*.cpp",
    "renderer/Color/*.cpp",
    "renderer/DXT/*.cpp",
    "renderer/jpeg-6/*.c",
    "renderer/OpenGL/*.cpp",
    "tools/compilers/dmap/*.cpp",
    "sound/*.cpp",
    "sound/OpenAL/*.cpp",
    "sys/*.cpp",
    "sys/win32/*.cpp",
    "ui/*.cpp",
    "game/ai/*.cpp",
    "game/anim/*.cpp",
    "game/bots/*.cpp",
    "game/client/*.cpp",
    "game/gamesys/*.cpp",
    "game/mp/*.cpp",
    "game/mp/stats/*.cpp",
    "game/physics/*.cpp",
    "game/script/*.cpp",
    "game/vehicle/*.cpp",
    "game/weapon/*.cpp",
]


def add_source(
    source_set: set[str], ordered_sources: list[str], rel_path: pathlib.Path
) -> None:
    normalized = rel_path.as_posix()
    if normalized not in source_set:
        source_set.add(normalized)
        ordered_sources.append(normalized)


def main() -> int:
    repo_root = pathlib.Path(__file__).resolve().parents[2]
    src_root = repo_root / "src"

    if not src_root.is_dir():
        print(f"Missing source root: {src_root}", file=sys.stderr)
        return 1

    source_set: set[str] = set()
    ordered_sources: list[str] = []

    for rel in Q4GAME_SOURCES:
        path = src_root / rel
        if not path.is_file():
            print(f"Missing expected source file: {path}", file=sys.stderr)
            return 1
        add_source(source_set, ordered_sources, pathlib.Path("src") / rel)

    for pattern in SOURCE_GLOBS:
        matches = sorted(src_root.glob(pattern))
        for match in matches:
            if match.is_file():
                add_source(
                    source_set,
                    ordered_sources,
                    pathlib.Path("src") / match.relative_to(src_root),
                )

    if not ordered_sources:
        print("No source files discovered.", file=sys.stderr)
        return 1

    print("\n".join(ordered_sources))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
