# Pokemon Graphics

This directory is species-centric. Pokemon graphics live under folders named for
the species:

```text
pokegra/
  bulbasaur/
    front.0.png
    front.ncer.toml
    back.0.png
    back.ncer.toml
    normal.pal
    shiny.pal
    nns.toml
  chespin/
    config.toml
    front.png
    back.png
```

`order.toml` defines the build order for battle graphics. Every folder listed
there is staged into `/a/0/0/4` in order. Entries marked `type = "pwan"` also
compile their source assets into `vfs/data/pokeweb_pwan`:

```toml
entries = [
  { folder = "bulbasaur", type = "nns" },
  { folder = "chespin", type = "pwan" },
]
```

Each PWAN species has `config.toml` selecting the source format:

```toml
format = "pwan"

[front]
source = "front.gif"

[back]
source = "back.gif"
```

PWAN battle sources use ordinary GIF/PNG files and are compiled into
`vfs/data/pokeweb_pwan` during the build.

Gen 6 battle sprites are imported from the Smogon Sprite Project assets hosted
by Pokemon Showdown with:

```sh
tools/graphics/import_smogon_battle_sprites.py
```

The importer prefers animated Gen 5-style GIFs from `gen5ani` and
`gen5ani-back`. If a side has no animated Gen 5-style asset, it falls back to
the indexed PNG from `gen5` or `gen5-back` rather than using model sprites. It
writes `smogon_sprite_project_audit.json` with the source URL, selected format,
and file hashes.

Additional Gen 6 animated sprite references may be sourced from Diegotoon20's
DeviantArt gallery, "6 gen Animated Sprites":

https://www.deviantart.com/diegotoon20/gallery/98400613/6-gen-animated-sprites

Any battle sprite assets imported from that gallery must retain attribution to
Diegotoon20 in this README and in the relevant audit/source notes.

NNS battle files are intermixed directly in each species folder:

```text
bulbasaur/
  nns.toml
  front.0.png
  front.ncer.toml
  normal.pal
```

The build stages these files back into `/a/0/0/4` by walking `order.toml` and
then each folder's `nns.toml`, preserving the original byte layout.
NCGR archive members are stored as indexed PNGs in the repository, and NCLR
archive members are stored as JASC `.pal` files, following the same broad shape
as pokeemerald species folders. Both are rebuilt into Nintendo binaries during
staging. Compressed archive members use raw sources in the repository; `lz11 =
true` tells the builder to LZ11 compress the rebuilt payload when staging
`/a/0/0/4`.

NCER, NANR, NMCR, and NMAR archive members are stored as structured TOML files
following the G2D section layouts documented by ds-pokemon-hacking:

```toml
format = "ncer"
magic = "RECN"
byte_order = 65279
version = 256
header_size = 16

[[sections]]
magic = "KBEC"
number_cells = 1
use_bounds = 0

[[sections.cells]]
number_objects = 4
offset_object = 0

[[sections.attributes]]
position_y = 208
position_x = 464
tile_index = 0
```

Supported section models include `KBEC` cells and OAM attributes, `KNBA`
animation sequences/frames/frame properties, `KBCM` multi-cells/properties,
`LBAL` labels, and `TXEU` extended metadata. Padding that exists inside KNBA
frame-property data is represented explicitly as `[[sections.property_padding]]`
so rebuilds remain byte-for-byte exact.

Most palette entries are just:

```toml
[[entries]]
role = "normal"
kind = "nclr"
raw = "normal.pal"
lz11 = false
```

If an original NCLR used palette bit 15, the manifest keeps that explicitly:

```toml
high_bits = [14, 15]
```

`nns.toml` is the single manifest for the raw NNS block. It does not track
archive IDs; it only describes the local files:

```toml
species = 1
symbol = "SPECIES_BULBASAUR"
block = 0
label = "base"

[[entries]]
role = "front.0"
kind = "ncgr"
raw = "front.0.png"
lz11 = true
```

`order.toml` is the central build-order file for `/a/0/0/4`. It lists folders
in archive-block order. The builder reads each folder's `nns.toml` and assigns
archive IDs by concatenating the listed entries:

```toml
entries = [
  { folder = "bulbasaur", type = "nns" },
  { folder = "ivysaur", type = "nns" },
  { folder = "chespin", type = "pwan" },
]
```

Alternate forms use form subfolders, for example `venusaur/form_1/`, following
the same file naming scheme. To add another NNS-backed Pokemon or form, create
the folder with its `nns.toml` and raw files, then add the folder path to
`order.toml` in dex/archive order. Use `type = "pwan"` when the folder also has
a PWAN `config.toml`; otherwise use `type = "nns"`.

Additional unknown fields are kept with `unknown*` names until their semantics
are confirmed.

`shared/` contains the small set of `/a/0/0/4` entries that do not currently
have a species/form owner in the available metadata.
