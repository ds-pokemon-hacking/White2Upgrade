#!/usr/bin/env python3
import argparse
import hashlib
import json
import re
import sys
import tomllib
from pathlib import Path
from urllib.error import HTTPError, URLError
from urllib.request import Request, urlopen


USER_AGENT = "White2Upgrade sprite importer"
BASE_URL = "https://play.pokemonshowdown.com/sprites"
SOURCES = {
    "front": (
        ("animated", "gen5ani", "gif"),
        ("static", "gen5", "png"),
    ),
    "back": (
        ("animated", "gen5ani-back", "gif"),
        ("static", "gen5-back", "png"),
    ),
}
ORDER_FILE = "order.toml"


def sha256_bytes(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def load_toml(path: Path) -> dict:
    with path.open("rb") as f:
        return tomllib.load(f)


def pokemon_entries(source_root: Path) -> list[dict]:
    order_path = source_root / ORDER_FILE
    order = load_toml(order_path)
    entries = order.get("entries", [])
    if not isinstance(entries, list):
        raise RuntimeError(f"{order_path}: missing entries array")
    return [entry for entry in entries if entry.get("type") == "pwan"]


def fetch(url: str) -> bytes | None:
    request = Request(url, headers={"User-Agent": USER_AGENT})
    try:
        with urlopen(request, timeout=30) as response:
            if response.status != 200:
                return None
            return response.read()
    except HTTPError as e:
        if e.code == 404:
            return None
        raise
    except URLError as e:
        raise RuntimeError(f"failed to fetch {url}: {e}") from e


def update_config_source(config_path: Path, side: str, source: str) -> None:
    text = config_path.read_text()
    pattern = rf"(\[{re.escape(side)}\]\nsource\s*=\s*)\"[^\"]+\""
    new_text, count = re.subn(pattern, rf'\1"{source}"', text)
    if count != 1:
        raise RuntimeError(f"{config_path}: could not update [{side}].source")
    if new_text != text:
        config_path.write_text(new_text)


def remove_stale_side_files(pokemon_dir: Path, side: str, keep_name: str) -> None:
    for ext in ("gif", "png"):
        path = pokemon_dir / f"{side}.{ext}"
        if path.name != keep_name and path.exists():
            path.unlink()


def import_side(pokemon_dir: Path, slug: str, side: str) -> dict | None:
    for kind, remote_dir, ext in SOURCES[side]:
        url = f"{BASE_URL}/{remote_dir}/{slug}.{ext}"
        data = fetch(url)
        if data is None:
            continue

        dest_name = f"{side}.{ext}"
        dest = pokemon_dir / dest_name
        old = dest.read_bytes() if dest.exists() else None
        if old != data:
            dest.write_bytes(data)
        remove_stale_side_files(pokemon_dir, side, dest_name)
        return {
            "kind": kind,
            "source": url,
            "dest": dest_name,
            "sha256": sha256_bytes(data),
            "changed": old != data,
        }
    return None


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Import Smogon/Pokemon Showdown Gen 5-style battle sprites."
    )
    parser.add_argument(
        "--battle-root",
        type=Path,
        default=Path("data/graphics/pokegra"),
    )
    parser.add_argument(
        "--audit",
        type=Path,
        default=Path("data/graphics/pokegra/smogon_sprite_project_audit.json"),
    )
    args = parser.parse_args()

    battle_root = args.battle_root
    audit = {
        "source": "Smogon Sprite Project via Pokemon Showdown sprite CDN",
        "base_url": BASE_URL,
        "entries": [],
        "missing": [],
    }

    changed = 0
    unchanged = 0
    animated = 0
    static = 0
    for entry in pokemon_entries(battle_root):
        folder = str(entry["folder"])
        manifest = load_toml(battle_root / folder / "nns.toml")
        species = int(manifest["species"])
        slug = str(entry.get("slug", folder))
        pokemon_dir = battle_root / folder
        config_path = pokemon_dir / "config.toml"
        record = {"species": species, "folder": folder, "slug": slug, "files": {}}

        for side in ("front", "back"):
            result = import_side(pokemon_dir, slug, side)
            if result is None:
                audit["missing"].append({"species": species, "folder": folder, "slug": slug, "side": side})
                continue
            update_config_source(config_path, side, result["dest"])
            changed += int(result["changed"])
            unchanged += int(not result["changed"])
            animated += int(result["kind"] == "animated")
            static += int(result["kind"] == "static")
            record["files"][side] = result

        audit["entries"].append(record)

    args.audit.write_text(json.dumps(audit, indent=2) + "\n")
    print(
        f"changed {changed}, unchanged {unchanged}, animated {animated}, "
        f"static {static}, missing {len(audit['missing'])}, audit {args.audit}"
    )
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except Exception as e:
        print(f"error: {e}", file=sys.stderr)
        raise SystemExit(1)
