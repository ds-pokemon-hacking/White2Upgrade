#!/usr/bin/env python3
from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from dump_game_data import main

if __name__ == "__main__":
    raise SystemExit(main(["personal", *sys.argv[1:]]))
