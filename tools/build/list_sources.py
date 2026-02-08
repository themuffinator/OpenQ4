#!/usr/bin/env python3
"""List C++ sources for a source subtree, excluding explicit relative paths."""

from __future__ import annotations

import sys
from pathlib import Path


def normalize(path: str) -> str:
    return path.replace("\\", "/").strip()


def main(argv: list[str]) -> int:
    if len(argv) < 3:
        print(
            f"usage: {argv[0]} <source-root> <subdir> [exclude-relative-path ...]",
            file=sys.stderr,
        )
        return 2

    source_root = Path(argv[1]).resolve()
    subdir = normalize(argv[2]).strip("/")
    target_root = (source_root / subdir).resolve()

    if not target_root.is_dir():
        print(f"error: source subtree not found: {target_root}", file=sys.stderr)
        return 1

    excludes = {normalize(entry) for entry in argv[3:]}
    sources: list[str] = []

    for path in target_root.rglob("*.cpp"):
        rel = path.relative_to(source_root).as_posix()
        if rel in excludes:
            continue
        sources.append(rel)

    for rel in sorted(sources):
        print(rel)

    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))
