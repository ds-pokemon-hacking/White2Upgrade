# Data Dumpers

The helper dumpers read resources from an extracted, unmodified White 2 ROM and
write editable TOML-oriented source files. They use `ndspy.narc.NARC` for NARC
archives. Binary Nitro graphics are extracted as files with a TOML manifest
because those resources are not scalar tables.

## Usage

Run every supported dump:

```sh
python3 tools/helpers/dump_game_data.py all --romfs ../IRDO_Extracted --out dump
```

Run one resource:

```sh
python3 tools/helpers/dump_game_data.py personal --out dump
python3 tools/helpers/dump_game_data.py trainers --out dump
```

The legacy entry points under `tools/helpers/DumpUtil` remain as wrappers:

```sh
python3 tools/helpers/DumpUtil/DumpPersonal.py --out dump
python3 tools/helpers/DumpUtil/DumpMoves.py --out dump
```

## Supported Resources

| Resource | NARC | Output |
| --- | --- | --- |
| `species` | enum source, not a NARC | `species/species.toml` |
| `personal` | `a/0/1/6` | `personal/NNNN.toml` |
| `children` | `a/0/1/7` | `children/NNNN.toml` |
| `learnsets` | `a/0/1/8` | `learnsets/NNNN.toml` |
| `evolutions` | `a/0/1/9` | `evolutions/NNNN.toml` |
| `moves` | `a/0/2/1` | `moves/NNNN.toml` |
| `items` | `a/0/2/4` | `items/NNNN.toml` |
| `encounters` | `a/1/2/7` | `encounters/NNNN.toml` |
| `trainers` | `a/0/9/1`, `a/0/9/2` | `trainers/NNNN.toml` |
| `pokegra_battle` | `a/0/0/4` | raw Nitro files plus `_manifest.toml` |
| `pokegra_icons` | `a/0/0/7` | raw Nitro files plus `_manifest.toml` |
| `system_text` | `a/0/0/2` | raw files plus `_manifest.toml` |

Each resource directory includes a `_manifest.toml` with the source NARC path,
description, and file count.

## TOML Conventions

Numeric IDs are emitted as labels when the repository has a matching enum in
`tools/mkdata/enum`. Unknown values remain numeric so no source information is
lost.

Repeated records use TOML arrays of tables:

```toml
[learnset]
index = 1
species = "SPECIES_BULBASAUR"

[[learnset.entries]]
slot = 0
move = "MOVE_TACKLE"
level = 1
```

Graphics and other binary payloads are written as extracted files. If a file is
LZ11-compressed, the dumper attempts to decompress it first and records `lz11 =
true` in the manifest entry. The file extension is inferred from Nitro magic
where possible: `ncgr`, `nclr`, `ncer`, `nanr`, `nmcr`, and `nmar`.

