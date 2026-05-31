# NARC Data Structures

These notes describe the game resources currently handled by
`tools/helpers/dump_game_data.py`. Offsets are relative to the start of each
member file inside the NARC. Integer fields are little-endian unless otherwise
noted.

## Personal Data

NARC: `a/0/1/6`

Each file is one Pokemon/form personal-data record. The original White 2 archive
contains 710 records; the expansion can add more in repo data.

| Offset | Field | Type | Notes |
| --- | --- | --- | --- |
| `0x00` | base HP | `u8` | Base stat |
| `0x01` | base Attack | `u8` | Base stat |
| `0x02` | base Defense | `u8` | Base stat |
| `0x03` | base Speed | `u8` | Base stat |
| `0x04` | base Special Attack | `u8` | Base stat |
| `0x05` | base Special Defense | `u8` | Base stat |
| `0x06` | primary type | `u8` | `TYPE_*` enum |
| `0x07` | secondary type | `u8` | `TYPE_*` enum |
| `0x08` | capture rate | `u8` | Species catch rate |
| `0x09` | evolution stage | `u8` | Family stage marker |
| `0x0A` | EV yield | `u16` | Packed EV yield bits |
| `0x0C` | wild item 50% | `u16` | `ITEM_*` enum |
| `0x0E` | wild item 5% | `u16` | `ITEM_*` enum |
| `0x10` | wild item 1% | `u16` | `ITEM_*` enum |
| `0x12` | gender probability | `u8` | `255` is genderless |
| `0x13` | egg hatch cycles | `u8` | Existing repo name: egg happiness |
| `0x14` | base happiness | `u8` | Initial friendship |
| `0x15` | experience group | `u8` | Growth-rate index |
| `0x16` | egg group 1 | `u8` | Egg group index |
| `0x17` | egg group 2 | `u8` | Egg group index |
| `0x18` | ability 1 | `u8` | `ABILITY_*` enum when known |
| `0x19` | ability 2 | `u8` | `ABILITY_*` enum when known |
| `0x1A` | hidden ability | `u8` | `ABILITY_*` enum when known |
| `0x1B` | escape rate | `u8` | Safari-style escape data |
| `0x1C` | forme data offset | `u16` | Form table offset/index |
| `0x1E` | forme sprite offset | `u16` | Form sprite offset/index |
| `0x20` | forme count | `u8` | Number of forms |
| `0x21` | color | `u8` | Dex color/body metadata |
| `0x22` | base EXP | `u16` | Experience yield |
| `0x24` | height cm | `u16` | Height in centimeters |
| `0x26` | weight cg | `u16` | Weight in centigrams |
| `0x28` | TM/HM flags 1 | `s32` | Compatibility bits |
| `0x2C` | TM/HM flags 2 | `s32` | Compatibility bits |
| `0x30` | TM/HM flags 3 | `s32` | Compatibility bits |
| `0x34` | TM/HM flags 4 | `s32` | Compatibility bits |
| `0x38` | type tutor flags | `s32` | Compatibility bits |
| `0x3C` | special tutor flags | `s32[4]` | Compatibility bits |

Entry size: `0x4C`.

## Child Species

NARC: `a/0/1/7`

Each file stores a single `u16` species ID. The value maps an evolved species
back to the child species used for eggs and related family logic.

## Level-Up Learnsets

NARC: `a/0/1/8`

Each file is a variable-length sequence of 4-byte entries:

| Offset | Field | Type | Notes |
| --- | --- | --- | --- |
| `+0x0` | move | `u16` | `MOVE_*` enum |
| `+0x2` | level | `u16` | Learn level |

The terminator is `move = 0xFFFF`, `level = 0xFFFF`.

## Evolutions

NARC: `a/0/1/9`

Each file is a fixed sequence of evolution slots. Current data uses 8 slots.

| Offset | Field | Type | Notes |
| --- | --- | --- | --- |
| `+0x0` | method | `u16` | `EVOLUTION_*` enum |
| `+0x2` | parameter | `u16` | Method-specific argument |
| `+0x4` | target species | `u16` | `SPECIES_*` enum |

Entry size: `0x06`.

## Moves

NARC: `a/0/2/1`

Each file is one move record.

| Offset | Field | Type | Notes |
| --- | --- | --- | --- |
| `0x00` | type | `u8` | `TYPE_*` |
| `0x01` | quality/effect category | `u8` | `EFFECT_*` |
| `0x02` | physical/special/status category | `u8` | `SPLIT_*` |
| `0x03` | power | `u8` | Base power |
| `0x04` | accuracy | `u8` | `101` means must-hit |
| `0x05` | base PP | `u8` | Base PP |
| `0x06` | priority | `s8` | Signed turn priority |
| `0x07` | hit count | `u8` | Low nibble min, high nibble max |
| `0x08` | inflict status | `u16` | `0xFFFF` means special-code status |
| `0x0A` | inflict chance | `u8` | Percent chance |
| `0x0B` | inflict duration | `u8` | Duration mode |
| `0x0C` | turn min | `u8` | Status/field duration |
| `0x0D` | turn max | `u8` | Status/field duration |
| `0x0E` | critical-hit stage | `u8` | Crit-stage modifier |
| `0x0F` | flinch rate | `u8` | Percent chance |
| `0x10` | animation ID | `u16` | Move animation resource |
| `0x12` | recoil | `s8` | Signed percent/handler value |
| `0x13` | heal | `s8` | Signed percent/handler value |
| `0x14` | target | `u8` | `TARGET_*` |
| `0x15` | stat change stats | `u8[3]` | `STAT_*` |
| `0x18` | stat change stages | `s8[3]` | Signed stage changes |
| `0x1B` | stat change chances | `s8[3]` | Chance per stat change |
| `0x1E` | padding | `u16` | Usually `0x5353` |
| `0x20` | flags | `u32` | `FLAG_*` bitfield |

Entry size: `0x24`.

## Items

NARC: `a/0/2/4`

Each file is one item record. The first `0x10` bytes describe bag/battle
metadata; the remaining bytes are battle-use stats and medicine parameters.

| Offset | Field | Type |
| --- | --- | --- |
| `0x00` | price | `u16` |
| `0x02` | held effect | `u8` |
| `0x03` | held argument | `u8` |
| `0x04` | natural gift effect | `u8` |
| `0x05` | fling effect | `u8` |
| `0x06` | fling power | `u8` |
| `0x07` | natural gift power | `u8` |
| `0x08` | packed flags | `u16` |
| `0x0A` | field effect | `u8` |
| `0x0B` | battle effect | `u8` |
| `0x0C` | has battle stats | `u8` |
| `0x0D` | item class | `u8` |
| `0x0E` | consumable | `u8` |
| `0x0F` | sort index | `u8` |
| `0x10` | cure inflict | `u8` |
| `0x11` | boost values | `u8[4]` |
| `0x15` | function flags | `u16` |
| `0x17` | EV gains | `u8[6]` |
| `0x1D` | heal amount | `u8` |
| `0x1E` | PP gain | `u8` |
| `0x1F` | friendship 1 | `u8` |
| `0x20` | friendship 2 | `u8` |
| `0x21` | friendship 3 | `u8` |
| `0x22` | field_1F | `u8` |
| `0x23` | field_20 | `u8` |

Entry size: `0x24`.

## Encounters

NARC: `a/1/2/7`

Each file is one encounter container, not four seasons in one file. The record
starts with eight rate bytes:

`grass_singles`, `grass_doubles`, `grass_special`, `surf_singles`,
`surf_special`, `fish_singles`, `fish_special`, `unknown`.

The rate header is followed by slot groups. Each slot is:

| Offset | Field | Type | Notes |
| --- | --- | --- | --- |
| `+0x0` | packed species/form | `u16` | species in low 11 bits, form in high 5 bits |
| `+0x2` | minimum level | `u8` | Inclusive |
| `+0x3` | maximum level | `u8` | Inclusive |

Slot-group order:

| Group | Slots | Chances |
| --- | --- | --- |
| grass singles | 12 | `20,20,10,10,10,10,5,5,4,4,1,1` |
| grass doubles | 12 | same as grass singles |
| grass special | 12 | same as grass singles |
| surf singles | 5 | `60,30,5,4,1` |
| surf special | 5 | same as surf singles |
| fish singles | 5 | same as surf singles |
| fish special | 5 | same as surf singles |

Entry size: `0xE8`.

## Trainers

NARCs: `a/0/9/1` for trainer metadata, `a/0/9/2` for trainer parties.

The metadata record determines how the matching party record should be decoded.

| Offset | Field | Type | Notes |
| --- | --- | --- | --- |
| `0x00` | party format | `u8` | bit 0 = explicit moves, bit 1 = held item |
| `0x01` | trainer class | `u8` | Class index |
| `0x02` | battle type | `u8` | Battle mode |
| `0x03` | party count | `u8` | Number of party members |
| `0x04` | items | `u16[4]` | Bag items |
| `0x0C` | AI flags | `u32` | AI behavior flags |
| `0x10` | can heal | `u8` | Present in 20-byte records |
| `0x11` | reward money | `u8` | Present in 20-byte records |
| `0x12` | reward item | `u16` | Present in 20-byte records |

Some unused original-game trainer records are 16 zero bytes. The dumper records
their raw size and emits an empty party.

Party member base fields:

| Field | Type | Notes |
| --- | --- | --- |
| difficulty value | `u8` | Difficulty/AI value |
| ability/gender | `u8` | ability in high nibble, gender in low nibble |
| level | `u16` | Pokemon level |
| species | `u16` | `SPECIES_*` |
| form | `u16` | Form index |

If party format bit 1 is set, a `u16` held item follows. If bit 0 is set, four
`u16` move IDs follow.

## Graphics NARCs

NARCs: `a/0/0/4`, `a/0/0/7`

These archives contain Nitro graphics resources. The dumper extracts each member
file and writes a manifest. LZ11 files are decompressed when possible.

Recognized magic:

| Magic | Extension | Meaning |
| --- | --- | --- |
| `RGCN` | `.ncgr` | character/tile graphics |
| `RLCN` | `.nclr` | palette |
| `RECN` | `.ncer` | cell data |
| `RNAN` | `.nanr` | animation data |
| `RCMN` | `.nmcr` | mapping resource |
| `RAMN` | `.nmar` | animation mapping resource |

The battle sprite collection format is related to NCGR/NCLR/NCER/NANR style
resources; see the external sprite collection notes in the ds-pokemon-hacking
documentation for a deeper description of that format family.

