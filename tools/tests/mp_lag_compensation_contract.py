#!/usr/bin/env python3
"""Pins the agreements that make server-side lag compensation honest.

Nothing in either repository asserted anything about lag compensation before this file, which is
how the feature came to ship with a rewind that could not measure what it was for.

Three agreements are pinned here.

  * THE ESTIMATE MUST NOT COME FROM THE COMMAND'S AGE.  `time - shooter->usercmd.gameTime` looks
    like the end-to-end pipeline delay and is in fact a structural constant: idAsyncServer files a
    client's command at THAT client's own frame index and executes the slot for the frame it is
    running, and idGameLocal::RunFrame has already advanced `time` past it, so the difference is
    one GetMSec() at any ping.  A duplicated command is re-stamped with the server's own clock to
    the same effect.  The rewind must be derived from the client's reported prediction lead,
    capped by the round trip the server measured for itself.

  * THE TARGET MUST BE PUT BACK BEFORE ANYTHING ACTS ON THE HIT.  The trace resolves against the
    rewound world, but the damage, the blood decal, the impact effect, the PVS areas that effect
    is broadcast to and the ragdoll a corpse starts from all belong in the present.  The gauntlet
    and the lightning gun already restored before they damaged; the hit-scan path did not.

  * A DISCONTINUITY MUST INVALIDATE THE HISTORY.  Rewinding onto a position a player teleported
    away from, or onto the previous occupant of a recycled client slot, aims a shot at a place
    nobody could have been.

The multiplayer tree is the only one that matters here: src/game is game_sp and never runs
multiplayer.  Its copy of this code is dead, and is checked only for defaults that would read as a
disagreement between the forks.
"""

from __future__ import annotations

import os
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
GAME_LIBS_ROOT = Path(
    os.environ.get("OPENQ4_GAMELIBS_REPO", ROOT.parent / "openQ4-game")
).resolve()


def read(root: Path, relative: str) -> str:
    path = root / relative
    if not path.is_file():
        raise AssertionError(f"Required file not found: {path}")
    return path.read_text(encoding="utf-8", errors="replace")


def require(text: str, token: str, context: str) -> None:
    if token not in text:
        raise AssertionError(f"Missing {token!r} in {context}")


def reject(text: str, token: str, context: str) -> None:
    if token in text:
        raise AssertionError(f"Unexpected {token!r} in {context}")


def require_before(text: str, first: str, second: str, context: str) -> None:
    require(text, first, context)
    require(text, second, context)
    if text.index(first) >= text.index(second):
        raise AssertionError(f"Expected {first!r} before {second!r} in {context}")


def function_body(source: str, signature: str, context: str) -> str:
    start = source.find(signature)
    if start < 0:
        raise AssertionError(f"Missing {signature!r} in {context}")
    opening = source.find("{", start + len(signature))
    if opening < 0:
        raise AssertionError(f"Missing body for {signature!r} in {context}")
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start : index + 1]
    raise AssertionError(f"Unbalanced body for {signature!r} in {context}")


def validate_estimator() -> None:
    game_local = read(GAME_LIBS_ROOT, "src/mpgame/Game_local.cpp")
    estimator = function_body(
        game_local,
        "bool idGameLocal::ComputeMPLagCompensationRewind(",
        "MP lag compensation estimator",
    )
    label = "MP lag compensation estimator"

    # The command-age estimate cannot measure anything; see the module docstring.  Rejected as the
    # assignment rather than as the bare symbol, so the comment explaining why it is wrong is
    # allowed to name it.
    reject(estimator, "rewindEstimateMS = time - shooter->usercmd.gameTime", label)
    reject(estimator, "= pingMS / 2", label)

    require(estimator, "ServerGetClientPrediction", label)
    require(estimator, "ServerGetClientPing", label)
    # Both report a 99999 sentinel for a slot that is not fully connected, and the reported lead is
    # a client-authored short, so it needs clamping as well as the sentinel check.
    require(estimator, "99999", label)
    require(estimator, "idMath::ClampInt( 0, 1000, reported )", label)
    require(estimator, "net_mpLagCompSlackMS", label)
    require(estimator, "RemoteExtrapolationMS()", label)
    require(estimator, "net_mpLagCompMaxMS", label)

    # Lag compensation and client-side remote extrapolation correct the same error from opposite
    # ends.  Applying both at full strength inverts the shooter's lead rather than removing it, so
    # the coupling has to stay visible in the code.
    coupling = function_body(
        game_local,
        "int idGameLocal::RemoteExtrapolationMS(",
        "remote extrapolation coupling",
    )
    require(coupling, "return 0;", "remote extrapolation coupling")
    require(game_local, "net_mpPredictMode", "remote extrapolation coupling rationale")


def validate_hit_is_resolved_in_the_present() -> None:
    game_local = read(GAME_LIBS_ROOT, "src/mpgame/Game_local.cpp")
    label = "MP lag compensation hit resolution"

    resolve = function_body(
        game_local,
        "bool idGameLocal::ResolveMPLagCompensationHit(",
        label,
    )
    require(resolve, "restore.originalOrigin - restore.rewoundOrigin", label)
    require(resolve, "tr.c.point += shift;", label)
    require(resolve, "tr.endpos += shift;", label)
    require(resolve, "tr.c.dist = tr.c.normal * tr.c.point;", label)
    require(resolve, "restoreState[ i ] = restoreState[ --restoreCount ];", label)

    hitscan = function_body(game_local, "idEntity* idGameLocal::HitScan(", label)
    require(hitscan, "ResolveMPLagCompensationHit(", label)
    # The target must be back in the present before ANY of these run against the hit.
    for consumer in (
        "collisionArea  = pvs.GetPVSArea( tr.c.point )",
        "ent->Damage(",
        "AddDamageEffect(",
    ):
        require_before(hitscan, "ResolveMPLagCompensationHit(", consumer, label)


def validate_invalidation() -> None:
    label = "MP lag compensation history invalidation"
    player = read(GAME_LIBS_ROOT, "src/mpgame/Player.cpp")
    network = read(GAME_LIBS_ROOT, "src/mpgame/Game_network.cpp")

    # A respawn was the only invalidation site the feature shipped with.
    spawn = function_body(player, "void idPlayer::SpawnToPoint(", label)
    require(spawn, "InvalidateMPLagCompensationHistory", label)

    # A teleport does not route through SpawnToPoint.
    teleport = function_body(
        player,
        "void idPlayer::Teleport( const idVec3 &origin, const idAngles &angles, idEntity *destination )",
        label,
    )
    require(teleport, "InvalidateMPLagCompensationHistory", label)

    # A recycled slot must not be rewound onto its previous occupant's positions.
    disconnect = function_body(
        network, "void idGameLocal::ServerClientDisconnect(", label
    )
    require(disconnect, "InvalidateMPLagCompensationHistory", label)


def validate_usercmd_stamp_is_server_authored() -> None:
    label = "usercmd gameTime sanitisation"
    async_server = read(ROOT, "src/framework/async/AsyncServer.cpp")
    process = function_body(
        async_server,
        "void idAsyncServer::ProcessUnreliableClientMessage(",
        label,
    )
    # gameTime arrives client-authored and reaches the game unchecked.  An honest client sends
    # exactly this value, so overwriting it costs nothing and removes a client-controlled dial on
    # anything that measures a command against the server clock.
    require(
        process,
        "userCmds[index][clientNum].gameTime = i * common->GetUserCmdMSec();",
        label,
    )
    require_before(
        process,
        "userCmds[index][clientNum].gameFrame = i;",
        "userCmds[index][clientNum].gameTime = i * common->GetUserCmdMSec();",
        label,
    )


def validate_defaults() -> None:
    label = "MP lag compensation defaults"
    mp_cvar = read(GAME_LIBS_ROOT, "src/mpgame/gamesys/SysCvar.cpp")

    # Off until the rewind has been measured against a real server, and because it must not be on
    # at the same time as full remote extrapolation - the two corrections invert rather than
    # compose.  A deliberate flip changes this line and this assertion together.
    require(mp_cvar, 'net_mpLagCompensation(', label)
    line = next(
        stripped
        for stripped in mp_cvar.splitlines()
        if "net_mpLagCompensation(" in stripped
    )
    if '"0"' not in line:
        raise AssertionError(
            f"{label}: net_mpLagCompensation must ship at 0 until the rewind is measured; got: {line.strip()}"
        )

    require(mp_cvar, 'net_mpLagCompSlackMS(', label)
    require(mp_cvar, 'net_mpLagCompMaxMS(', label)


def main() -> None:
    validate_estimator()
    validate_hit_is_resolved_in_the_present()
    validate_invalidation()
    validate_usercmd_stamp_is_server_authored()
    validate_defaults()
    print("mp_lag_compensation_contract: ok")


if __name__ == "__main__":
    main()
