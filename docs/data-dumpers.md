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
| `pokegra_battle` | `a/0/0/4` | `pokegra/battle/NNNN.kind` plus `_manifest.toml` |
| `pokegra_icons` | `a/0/0/7` | `pokegra/icons/NNNN.kind` plus `_manifest.toml` |
| `pokegra_footprints` | `a/1/6/5` | `pokegra/footprints/165_NNNNNNNN.bin` plus `_manifest.toml` |
| `system_text` | `a/0/0/2` | message TOML plus `_manifest.toml` |
| `game_text` | `a/0/0/3` | game/map message TOML plus `_manifest.toml` |

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

System text and game/map text are decoded to TOML message files using numbered
entry tables:

```toml
[msg.section_0.entries.0]
text = [
  "First page\\n",
  "Second page\\c$",
]
```

`text_bits` is omitted for normal 16-bit text. Compressed strings use
`text_bits = 9`. Plaintext controls follow the SwissArmyKnife/BeaterLibrary
style: `\n` is `0xFFFE`, `\l` is `F000 BE00 0000`, `\c` is `F000 BE01 0000`,
`\xNNNN` emits a raw 16-bit code unit, and `$` terminates the string. Gen V
variables use CTRMap-style tags such as `[VAR MOVE(0)]`, `[VAR TRNAME(1)]`,
`[WAIT(60)]`, and `[SETXPOS(70)]`.

Message payloads use the original GFString encryption algorithm. The initial
16-bit key for entry `i` is `(i + 3) * 0x2983`, truncated to 16 bits. Each
stored 16-bit word is XORed with the current key, then the key is rotated left
by three bits. The TOML source does not store per-entry encryption seeds.

Graphics and other binary payloads are written as extracted files. If a file is
LZ11-compressed, the dumper attempts to decompress it first and records `lz11 =
true` in the manifest entry. The file extension is inferred from Nitro magic
where possible: `ncgr`, `nclr`, `ncer`, `nanr`, `nmcr`, and `nmar`.
