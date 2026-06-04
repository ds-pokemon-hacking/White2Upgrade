# PWAN Animation Workflow

This repository uses the HZLA GIF animation workflow for expanded Pokemon sprites.

## Runtime

The animation runtime is built into `w2u_main.dll` from `src/pwan_animation`.
It loads assets directly from the ROM filesystem through these root-level paths:

- `pokeweb_pwan/config.bin`
- `pokeweb_pwan/NNN_front.pwan`
- `pokeweb_pwan/NNN_back.pwan`

Species not present in `config.bin` fall back to normal game rendering.

## Asset Format

Each `.pwan` file is a 96x96 4bpp animation:

- Header magic: `PWAN`
- Palette: 16 BGR555 colors, with palette index 0 reserved for transparency
- Timeline: `{ u16 frameIndex, u16 ticks }`
- Frame data: `0x1200` bytes per unique frame

The 96x96 frame is stored as four OBJ-compatible tiled regions:

- `64x64` top-left
- `32x64` top-right
- `64x32` bottom-left
- `32x32` bottom-right

The runtime keeps 128 timeline entries per loaded asset, so the compiler resamples
longer source GIF timelines down to that limit.

## Gen 6 Asset Pack

Run:

```sh
python3 tools/pwan/build_gen6_pwan.py
```

The script downloads Pokemon Showdown Gen 5-style animated GIFs from
`https://play.pokemonshowdown.com/sprites/gen5ani/` into
`data/pwan/source_gifs`, compiles them with `tools/pwan/compile_pwan.py`, and
writes the runtime pack to `data/pwan/pokeweb_pwan`.

The Gen 5-style project does not currently expose every Gen 6 front/back under
`gen5ani` and `gen5ani-back`, so the generator falls back to `ani`/`ani-back`
only when the Gen 5-style URL is missing. The selected source for each compiled
asset is written to `data/pwan/pokeweb_pwan/sources.json`.

The current mapping covers species `650..721`. Asset index `0` corresponds to
Chespin, `1` to Quilladin, and so on through Volcanion. For each species, the
front and back entries share the same index:

```text
front asset id = index * 2
back asset id  = index * 2 + 1
```

`data/pwan/meson.build` stages the compiled files into `vfs/data/pokeweb_pwan`.
Because `White2Upgrade.cmproj` uses `UserDataPath: data`, those files become the
root-level `pokeweb_pwan/...` paths expected by the runtime.

## In-Game Verification

The current Route 6 save can exercise the battle path with wild Xerneas. Use the
ARM9 GDB stub and HID input method documented in `docs/debugging-gdb.md`.

Useful filtered breakpoint:

```gdb
break *0x02070ecc
commands
silent
if *(unsigned char*)$r1 == 0x70
  printf "PWAN_OPEN %s\n", $r1
end
continue
end
```

Then load the save and walk with the HID update breakpoint at `0x0203dd70`.
When Xerneas appears, the battle animation runtime should open:

```text
pokeweb_pwan/066_front.pwan
```

Asset index `66` maps to species `650 + 66 = 716`, which is Xerneas. A local
test on the rebuilt ROM recorded repeated opens of `066_front.pwan` during a
Route 6 encounter, confirming the battle runtime found the Gen 6 PWAN asset in
the ROM filesystem.
