#!/usr/bin/env python3

from __future__ import annotations

from argparse import ArgumentParser
from pathlib import Path
import shutil


def sort_key(path: Path) -> int:
    stem = path.name.rsplit("_", 1)[-1]
    try:
        return int(stem)
    except ValueError:
        return 1_000_000


def main() -> int:
    parser = ArgumentParser(description="Stage generated binary files as CTRMap NARC-entry overlays.")
    parser.add_argument("stamp", type=Path)
    parser.add_argument("output", type=Path)
    parser.add_argument("inputs", nargs="+", type=Path)
    args = parser.parse_args()

    if args.output.is_dir():
        shutil.rmtree(args.output)
    elif args.output.exists():
        args.output.unlink()
    args.output.mkdir(parents=True, exist_ok=True)
    (args.output / ".arc").write_text(".arc\ncompress default auto\n")

    inputs = sorted(args.inputs, key=sort_key)
    for index, path in enumerate(inputs):
        (args.output / str(index)).write_bytes(path.read_bytes())

    args.stamp.parent.mkdir(parents=True, exist_ok=True)
    args.stamp.write_text(f"{args.output}\n{len(inputs)} files\n")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
