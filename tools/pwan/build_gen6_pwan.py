#!/usr/bin/env python3
import argparse
import json
import struct
import sys
import urllib.error
import urllib.request
from pathlib import Path

from compile_pwan import compile_pwan


ROOT = Path(__file__).resolve().parents[2]
SOURCE_ROOT = ROOT / "data" / "pwan" / "source_gifs"
OUTPUT_ROOT = ROOT / "data" / "pwan" / "pokeweb_pwan"
SOURCE_MANIFEST = OUTPUT_ROOT / "sources.json"

CONFIG_MAGIC = 0x434E5750
CONFIG_VERSION = 1
CONFIG_ENTRY_SIZE = 8
CONFIG_HEADER_SIZE = 16
MAX_TIMELINE_ENTRIES = 128

GEN6_SPECIES = [
    (650, "chespin"),
    (651, "quilladin"),
    (652, "chesnaught"),
    (653, "fennekin"),
    (654, "braixen"),
    (655, "delphox"),
    (656, "froakie"),
    (657, "frogadier"),
    (658, "greninja"),
    (659, "bunnelby"),
    (660, "diggersby"),
    (661, "fletchling"),
    (662, "fletchinder"),
    (663, "talonflame"),
    (664, "scatterbug"),
    (665, "spewpa"),
    (666, "vivillon"),
    (667, "litleo"),
    (668, "pyroar"),
    (669, "flabebe"),
    (670, "floette"),
    (671, "florges"),
    (672, "skiddo"),
    (673, "gogoat"),
    (674, "pancham"),
    (675, "pangoro"),
    (676, "furfrou"),
    (677, "espurr"),
    (678, "meowstic"),
    (679, "honedge"),
    (680, "doublade"),
    (681, "aegislash"),
    (682, "spritzee"),
    (683, "aromatisse"),
    (684, "swirlix"),
    (685, "slurpuff"),
    (686, "inkay"),
    (687, "malamar"),
    (688, "binacle"),
    (689, "barbaracle"),
    (690, "skrelp"),
    (691, "dragalge"),
    (692, "clauncher"),
    (693, "clawitzer"),
    (694, "helioptile"),
    (695, "heliolisk"),
    (696, "tyrunt"),
    (697, "tyrantrum"),
    (698, "amaura"),
    (699, "aurorus"),
    (700, "sylveon"),
    (701, "hawlucha"),
    (702, "dedenne"),
    (703, "carbink"),
    (704, "goomy"),
    (705, "sliggoo"),
    (706, "goodra"),
    (707, "klefki"),
    (708, "phantump"),
    (709, "trevenant"),
    (710, "pumpkaboo"),
    (711, "gourgeist"),
    (712, "bergmite"),
    (713, "avalugg"),
    (714, "noibat"),
    (715, "noivern"),
    (716, "xerneas"),
    (717, "yveltal"),
    (718, "zygarde"),
    (719, "diancie"),
    (720, "hoopa"),
    (721, "volcanion"),
]


def source_urls(side: str, slug: str) -> list[tuple[str, str]]:
    if side == "front":
        folders = ("gen5ani", "ani")
    else:
        folders = ("gen5ani-back", "ani-back")
    return [
        (folder, f"https://play.pokemonshowdown.com/sprites/{folder}/{slug}.gif")
        for folder in folders
    ]


def download(url: str, dst: Path, force: bool) -> None:
    if dst.exists() and not force:
        return
    dst.parent.mkdir(parents=True, exist_ok=True)
    request = urllib.request.Request(url, headers={"User-Agent": "White2Upgrade PWAN asset builder"})
    try:
        with urllib.request.urlopen(request, timeout=30) as response:
            data = response.read()
    except urllib.error.URLError as exc:
        raise RuntimeError(f"failed to download {url}: {exc}") from exc
    if not data.startswith(b"GIF"):
        raise RuntimeError(f"{url} did not return a GIF")
    dst.write_bytes(data)


def download_first(candidates: list[tuple[str, str]], dst: Path, force: bool) -> tuple[str, str]:
    if dst.exists() and not force:
        return "cached", ""

    errors = []
    for source_name, url in candidates:
        try:
            download(url, dst, force=True)
            return source_name, url
        except RuntimeError as exc:
            errors.append(str(exc))
    raise RuntimeError("no sprite source succeeded:\n" + "\n".join(errors))


def write_config(output_dir: Path) -> None:
    output_dir.mkdir(parents=True, exist_ok=True)
    entries = []
    for index, (species, _slug) in enumerate(GEN6_SPECIES):
        entries.append(struct.pack("<HHHH", species, 0x0003, index, index))

    header = struct.pack(
        "<IHHHHI",
        CONFIG_MAGIC,
        CONFIG_VERSION,
        len(entries),
        MAX_TIMELINE_ENTRIES,
        0,
        CONFIG_HEADER_SIZE,
    )
    if len(header) != CONFIG_HEADER_SIZE:
        raise RuntimeError(f"unexpected config header size {len(header)}")

    (output_dir / "config.bin").write_bytes(header + b"".join(entries))


def build_assets(force_download: bool) -> None:
    max_timeline = 0
    total_bytes = 0
    sources = []
    for index, (species, slug) in enumerate(GEN6_SPECIES):
        for side in ("front", "back"):
            gif = SOURCE_ROOT / side / f"{species}_{slug}.gif"
            pwan = OUTPUT_ROOT / f"{index:03}_{side}.pwan"
            source_name, url = download_first(source_urls(side, slug), gif, force_download)
            stats = compile_pwan(gif, pwan)
            max_timeline = max(max_timeline, stats["timeline"])
            total_bytes += stats["bytes"]
            sources.append({
                "species": species,
                "slug": slug,
                "side": side,
                "source": source_name,
                "url": url,
                "pwan": pwan.name,
            })
            print(
                f"{species:03} {slug:<12} {side:<5} {source_name:<12} "
                f"{stats['frames']:>3} unique, {stats['timeline']:>3} timeline, "
                f"{stats['bytes']:>6} bytes"
            )
    write_config(OUTPUT_ROOT)
    SOURCE_MANIFEST.write_text(json.dumps(sources, indent=2) + "\n")
    print(f"wrote {OUTPUT_ROOT / 'config.bin'} ({len(GEN6_SPECIES)} mappings)")
    print(f"wrote {SOURCE_MANIFEST}")
    print(f"max timeline {max_timeline}, total PWAN bytes {total_bytes}")


def main() -> int:
    parser = argparse.ArgumentParser(description="Build Gen 6 animated PWAN Pokemon sprites.")
    parser.add_argument("--force-download", action="store_true")
    args = parser.parse_args()
    try:
        build_assets(args.force_download)
    except Exception as exc:
        print(exc, file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
