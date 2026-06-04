# GDB Debugging Notes

These notes describe the runtime hooks used while verifying Route 6 battles in
melonDS. They are intentionally procedural so the same setup can be repeated.

## melonDS GDB

The local melonDS configuration has ARM9 GDB enabled, normally on port `3333`.
Start the current ROM:

```sh
melonDS build/White2Upgrade.nds
```

Then attach:

```gdb
gdb -nx
(gdb) target remote :3333
```

If melonDS breaks on startup, use `signal 0` to resume without re-delivering
the stop signal. This has been more reliable than plain `continue` after a
manual GDB interrupt.

## Input Injection

Use the game's HID state instead of host keyboard events or patched key-read
stubs. `GFL_HIDUpdateKeypad` ends at `0x0203dd70`; at that point `r4` is the
live keypad block. A breakpoint there can overwrite the freshly updated key
fields for a fixed number of frames.

The fields used during Route 6 verification were:

| Offset | Meaning |
| ---: | --- |
| `0x0c` | raw/current keys |
| `0x10` | raw/pressed keys |
| `0x14` | raw/typed keys |
| `0x18` | held keys |
| `0x1c` | pressed keys |
| `0x20` | typed keys |
| `0x24` | alternate held keys |
| `0x28` | alternate pressed keys |
| `0x2c` | alternate typed keys |

```gdb
set $count = 10
set $mask = 0x1
break *0x0203dd70
commands
silent
set {unsigned int}($r4+0x0c) = $mask
set {unsigned int}($r4+0x10) = $mask
set {unsigned int}($r4+0x14) = $mask
set {unsigned int}($r4+0x18) = $mask
set {unsigned int}($r4+0x1c) = $mask
set {unsigned int}($r4+0x20) = $mask
set {unsigned int}($r4+0x24) = $mask
set {unsigned int}($r4+0x28) = $mask
set {unsigned int}($r4+0x2c) = $mask
set $count = $count - 1
if $count <= 0
delete $bpnum
end
continue
end
signal 0
```

Useful DS key masks:

| Key | Mask |
| --- | ---: |
| A | `0x01` |
| B | `0x02` |
| Select | `0x04` |
| Start | `0x08` |
| Right | `0x10` |
| Left | `0x20` |
| Up | `0x40` |
| Down | `0x80` |

For Route 6 verification, the working path was:

- `Start` (`0x08`) from the title screen.
- `A` (`0x01`) on Continue.
- `A` (`0x01`) again on the save-load/C-Gear prompt.
- Longer directional holds in the field, for example 120 frames of Down or an
  alternating Left/Right loop over `0x0203dd70`.

The global HID manager pointer is at `0x021418c4`. Its first word points to the
same keypad block, but the end-of-update breakpoint is preferred because it
writes after the normal hardware scan has refreshed the fields.

## Text Round Trip Note

When rebuilding every file in `/a/0/0/2` and `/a/0/0/3`, use the original
GFString encryption algorithm: entry `i` starts with key `(i + 3) * 0x2983`,
truncated to 16 bits, then each 16-bit word is XORed and the key rotates left
by three bits. The title/save menu bank `/a/0/0/2/371` should round-trip
byte-identically to the original game. If extra terminators are dropped, the
menu can render incorrectly or stall on blank prompt screens even though the
expanded species and move banks decode.

## Party Move Injection

Use the SWAN headers for structure offsets:

- `PokeParty` is declared in `include/swan/pml/poke_party.h`.
- `PartyPkm` embeds `BoxPkm Base` at offset `0`.
- `BoxPkmBlock1.Moves[4]` is declared in `include/swan/pml/poke_data.h`.

Prefer calling the game helper instead of editing encrypted BoxPkm blocks by
hand:

- `PokeParty_SetMove` is at `0x0201d251` (`+1` Thumb address already reflected
  in `pmc/IRDO.yml`).
- `PML_PkmSetMove` is at `0x0201d259` (`+1` Thumb address already reflected in
  `pmc/IRDO.yml`).

When stopped in GDB and holding a valid `PartyPkm*`, call the helper with:

```gdb
call ((void (*)(void *, int, int))0x0201d259)(PKM_PTR, SLOT, MOVE_ID)
```

For text verification, useful move IDs are:

- `MOVE_GEOMANCY = 601`
- `MOVE_OBLIVION_WING = 613`
- `MOVE_HYPERSPACE_FURY = 621`
