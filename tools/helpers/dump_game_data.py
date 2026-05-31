#!/usr/bin/env python3
from __future__ import annotations

import argparse
import sys
from pathlib import Path

from DumpUtil.lzss import DecompressionError, decompress_file
from dumpers.common import (
    DEFAULT_OUT,
    DEFAULT_ROMFS,
    NARCS,
    Reader,
    detect_magic,
    dump_manifest,
    flags,
    label,
    load_label_map,
    read_narc_files,
    write_toml,
)


TYPE = load_label_map("types")
SPECIES = load_label_map("species")
MOVES = load_label_map("moves")
ITEMS = load_label_map("items")
PSS = load_label_map("pss")
BTL_EFF = load_label_map("btl_eff")
BTL_INFLICT = load_label_map("btl_inflict")
BTL_STAT = load_label_map("btl_stat")
BTL_TARGET = load_label_map("btl_target")
MOVE_FLAGS = load_label_map("move_flags")
EVOLUTION_METHODS = load_label_map("evolution_methods")
BTL_GENDER = load_label_map("btl_gender")
BTL_ABILITY = load_label_map("btl_abil")


def dump_record_set(resource: str, romfs: Path, out: Path, parser) -> None:
    spec = NARCS[resource]
    files = read_narc_files(spec.source_path(romfs))
    target = out / resource
    dump_manifest(target / "_manifest.toml", spec, files)
    for index, data in enumerate(files):
        write_toml(target / f"{index:04}.toml", parser(index, data))


def parse_personal(index: int, data: bytes) -> dict:
    r = Reader(data, f"personal:{index}")
    return {
        "personal": {
            "index": index,
            "species": label(SPECIES, index),
            "base_stats": {
                "hp": r.u8(),
                "attack": r.u8(),
                "defense": r.u8(),
                "speed": r.u8(),
                "special_attack": r.u8(),
                "special_defense": r.u8(),
            },
            "typing": {"primary": label(TYPE, r.u8()), "secondary": label(TYPE, r.u8())},
            "capture_rate": r.u8(),
            "evolution_stage": r.u8(),
            "ev_yield": r.u16(),
            "wild_items": {"common_50": label(ITEMS, r.u16()), "rare_5": label(ITEMS, r.u16()), "rare_1": label(ITEMS, r.u16())},
            "gender_probability": r.u8(),
            "egg_happiness": r.u8(),
            "base_happiness": r.u8(),
            "experience_group": r.u8(),
            "egg_groups": [r.u8(), r.u8()],
            "abilities": {"primary": label(BTL_ABILITY, r.u8()), "secondary": label(BTL_ABILITY, r.u8()), "hidden": label(BTL_ABILITY, r.u8())},
            "escape_rate": r.u8(),
            "forms": {"data_offset": r.u16(), "sprite_offset": r.u16(), "count": r.u8()},
            "color": r.u8(),
            "base_experience": r.u16(),
            "height_cm": r.u16(),
            "weight_cg": r.u16(),
            "compatibility": {
                "tmhm": [r.s32(), r.s32(), r.s32(), r.s32()],
                "type_tutors": r.s32(),
                "special_tutors": [r.s32(), r.s32(), r.s32(), r.s32()],
            },
        }
    }


def parse_move(index: int, data: bytes) -> dict:
    r = Reader(data, f"move:{index}")
    move_type = r.u8()
    quality = r.u8()
    category = r.u8()
    power = r.u8()
    accuracy = r.u8()
    base_pp = r.u8()
    priority = r.s8()
    hit_packed = r.u8()
    inflict_status = r.u16()
    result = {
        "move": {
            "index": index,
            "name": label(MOVES, index),
            "type": label(TYPE, move_type),
            "quality": label(BTL_EFF, quality),
            "category": label(PSS, category),
            "power": power,
            "accuracy": "MUST_HIT" if accuracy == 101 else accuracy,
            "base_pp": base_pp,
            "priority": priority,
            "hits": {"minimum": hit_packed & 0xF, "maximum": hit_packed >> 4},
            "inflict": {
                "status": "STATUS_SPECIAL_CODE" if inflict_status == 0xFFFF else label(BTL_INFLICT, inflict_status),
                "chance": r.u8(),
                "duration": r.u8(),
                "turn_min": r.u8(),
                "turn_max": r.u8(),
            },
            "critical_hit_stage": r.u8(),
            "flinch_rate": r.u8(),
            "animation_id": r.u16(),
            "recoil": r.s8(),
            "heal": r.s8(),
            "target": label(BTL_TARGET, r.u8()),
            "stat_changes": [],
            "padding": None,
            "flags": [],
        }
    }
    stats = [label(BTL_STAT, r.u8()) for _ in range(3)]
    stages = [r.s8() for _ in range(3)]
    chances = [r.s8() for _ in range(3)]
    result["move"]["stat_changes"] = [
        {"stat": stats[i], "stages": stages[i], "chance": chances[i]} for i in range(3)
    ]
    result["move"]["padding"] = r.u16()
    result["move"]["flags"] = flags(MOVE_FLAGS, r.u32())
    return result


def parse_item(index: int, data: bytes) -> dict:
    r = Reader(data, f"item:{index}")
    return {
        "item": {
            "index": index,
            "name": label(ITEMS, index),
            "price": r.u16(),
            "held_effect": r.u8(),
            "held_argument": r.u8(),
            "natural_gift_effect": r.u8(),
            "fling_effect": r.u8(),
            "fling_power": r.u8(),
            "natural_gift_power": r.u8(),
            "packed": r.u16(),
            "effect_field": r.u8(),
            "effect_battle": r.u8(),
            "has_battle_stats": r.u8(),
            "item_class": r.u8(),
            "consumable": r.u8(),
            "sort_index": r.u8(),
            "battle_stats": {
                "cure_inflict": r.u8(),
                "boost": [r.u8(), r.u8(), r.u8(), r.u8()],
                "function_flags": r.u16(),
                "ev_hp": r.u8(),
                "ev_attack": r.u8(),
                "ev_defense": r.u8(),
                "ev_speed": r.u8(),
                "ev_special_attack": r.u8(),
                "ev_special_defense": r.u8(),
                "heal_amount": r.u8(),
                "pp_gain": r.u8(),
                "friendship_1": r.u8(),
                "friendship_2": r.u8(),
                "friendship_3": r.u8(),
                "field_1f": r.u8(),
                "field_20": r.u8(),
            },
        }
    }


def parse_evolution(index: int, data: bytes) -> dict:
    r = Reader(data, f"evolution:{index}")
    entries = []
    slot = 0
    while not r.done():
        method = r.u16()
        parameter = r.u16()
        target = r.u16()
        entries.append(
            {
                "slot": slot,
                "method": label(EVOLUTION_METHODS, method),
                "parameter": parameter,
                "target_species": label(SPECIES, target),
            }
        )
        slot += 1
    return {"evolutions": {"index": index, "species": label(SPECIES, index), "entries": entries}}


def parse_learnset(index: int, data: bytes) -> dict:
    r = Reader(data, f"learnset:{index}")
    entries = []
    slot = 0
    while not r.done():
        move = r.u16()
        level = r.u16()
        if move == 0xFFFF and level == 0xFFFF:
            break
        entries.append({"slot": slot, "move": label(MOVES, move), "level": level})
        slot += 1
    return {"learnset": {"index": index, "species": label(SPECIES, index), "entries": entries}}


def parse_child(index: int, data: bytes) -> dict:
    r = Reader(data, f"child:{index}")
    return {"child": {"index": index, "species": label(SPECIES, index), "child_species": label(SPECIES, r.u16())}}


ENCOUNTER_RATE_KEYS = ["grass_singles", "grass_doubles", "grass_special", "surf_singles", "surf_special", "fish_singles", "fish_special", "unknown"]
GRASS_SLOTS = [20, 20, 10, 10, 10, 10, 5, 5, 4, 4, 1, 1]
WATER_SLOTS = [60, 30, 5, 4, 1]


def parse_encounter(index: int, data: bytes) -> dict:
    r = Reader(data, f"encounter:{index}")
    rates = {key: r.u8() for key in ENCOUNTER_RATE_KEYS}
    slots = []
    for category, encounter_type, rates_table in [
        ("grass", "singles", GRASS_SLOTS),
        ("grass", "doubles", GRASS_SLOTS),
        ("grass", "special", GRASS_SLOTS),
        ("surf", "singles", WATER_SLOTS),
        ("surf", "special", WATER_SLOTS),
        ("fish", "singles", WATER_SLOTS),
        ("fish", "special", WATER_SLOTS),
    ]:
        for slot, chance in enumerate(rates_table):
            species_form = r.u16()
            slots.append(
                {
                    "category": category,
                    "type": encounter_type,
                    "slot": slot,
                    "chance": chance,
                    "species": label(SPECIES, species_form & 0x7FF),
                    "form": species_form >> 11,
                    "minimum_level": r.u8(),
                    "maximum_level": r.u8(),
                }
            )
    return {"encounter": {"index": index, "rates": rates, "slots": slots}}


def parse_trdata(index: int, data: bytes) -> dict:
    r = Reader(data, f"trdata:{index}")
    if len(data) == 16 and not any(data):
        return {
            "trainer": {
                "index": index,
                "party_format": 0,
                "trainer_class": 0,
                "battle_type": 0,
                "party_count": 0,
                "items": [],
                "ai": 0,
                "can_heal": 0,
                "reward_money": 0,
                "reward_item": 0,
                "raw_size": len(data),
            }
        }
    return {
        "trainer": {
            "index": index,
            "party_format": r.u8(),
            "trainer_class": r.u8(),
            "battle_type": r.u8(),
            "party_count": r.u8(),
            "items": [label(ITEMS, r.u16()) for _ in range(4)],
            "ai": r.u32(),
            "can_heal": r.u8() if r.remaining() else 0,
            "reward_money": r.u8() if r.remaining() else 0,
            "reward_item": label(ITEMS, r.u16()) if r.remaining() >= 2 else 0,
            "raw_size": len(data),
        }
    }


def parse_trpoke_file(index: int, data: bytes, trdata: dict) -> dict:
    fmt = trdata["trainer"]["party_format"]
    count = trdata["trainer"]["party_count"]
    has_moves = bool(fmt & 1)
    has_item = bool(fmt & 2)
    r = Reader(data, f"trpoke:{index}")
    party = []
    for slot in range(count):
        difficulty = r.u8()
        ability_gender = r.u8()
        member = {
            "slot": slot,
            "difficulty_value": difficulty,
            "ability": label(BTL_ABILITY, ability_gender >> 4),
            "gender": label(BTL_GENDER, ability_gender & 0xF),
            "level": r.u16(),
            "species": label(SPECIES, r.u16()),
            "form": r.u16(),
        }
        if has_item:
            member["held_item"] = label(ITEMS, r.u16())
        if has_moves:
            member["moves"] = [label(MOVES, r.u16()) for _ in range(4)]
        party.append(member)
    return {"trainer_party": {"index": index, "party": party}}


def dump_trainers(romfs: Path, out: Path) -> None:
    trdata_files = read_narc_files(NARCS["trdata"].source_path(romfs))
    trpoke_files = read_narc_files(NARCS["trpoke"].source_path(romfs))
    dump_manifest(out / "trainers" / "_trdata_manifest.toml", NARCS["trdata"], trdata_files)
    dump_manifest(out / "trainers" / "_trpoke_manifest.toml", NARCS["trpoke"], trpoke_files)
    for index, data in enumerate(trdata_files):
        parsed = parse_trdata(index, data)
        write_toml(out / "trainers" / f"{index:04}.toml", parsed | parse_trpoke_file(index, trpoke_files[index], parsed))


def try_decompress(data: bytes) -> tuple[bytes, bool]:
    if not data.startswith(b"\x11"):
        return data, False
    class Handle:
        def __init__(self, payload: bytes):
            self.payload = payload
            self.offset = 0
        def read(self, amount=-1):
            if amount < 0:
                amount = len(self.payload) - self.offset
            chunk = self.payload[self.offset:self.offset + amount]
            self.offset += amount
            return chunk
        def seek(self, offset):
            self.offset = offset
    try:
        decompressed = decompress_file(Handle(data))
    except DecompressionError:
        return data, False
    return (bytes(decompressed), True) if decompressed else (data, False)


def dump_raw_assets(resource: str, romfs: Path, out: Path) -> None:
    spec = NARCS[resource]
    files = read_narc_files(spec.source_path(romfs))
    target = out / resource
    target.mkdir(parents=True, exist_ok=True)
    manifest = {"narc": {"name": spec.name, "path": spec.path, "description": spec.description, "file_count": len(files)}, "files": {"entries": []}}
    for index, data in enumerate(files):
        payload, compressed = try_decompress(data)
        kind = detect_magic(payload)
        file_name = f"{index:04}.{kind}"
        (target / file_name).write_bytes(payload)
        manifest["files"]["entries"].append({"index": index, "file": file_name, "size": len(payload), "lz11": compressed, "kind": kind})
    write_toml(target / "_manifest.toml", manifest)


def dump_resource(resource: str, romfs: Path, out: Path) -> None:
    parsers = {
        "personal": parse_personal,
        "moves": parse_move,
        "items": parse_item,
        "evolutions": parse_evolution,
        "learnsets": parse_learnset,
        "children": parse_child,
        "encounters": parse_encounter,
    }
    if resource == "species":
        entries = [{"id": key, "label": value} for key, value in sorted(SPECIES.items())]
        write_toml(out / "species" / "species.toml", {"species": {"entries": entries}})
    elif resource == "trainers":
        dump_trainers(romfs, out)
    elif resource in parsers:
        dump_record_set(resource, romfs, out, parsers[resource])
    elif resource in {"pokegra_battle", "pokegra_icons", "system_text"}:
        dump_raw_assets(resource, romfs, out)
    else:
        raise SystemExit(f"Unknown dump resource: {resource}")


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Dump White 2 original-game NARCs to TOML-oriented source files.")
    parser.add_argument("resources", nargs="*", help="Resources to dump. Use 'all' for every supported resource.")
    parser.add_argument("--romfs", type=Path, default=DEFAULT_ROMFS, help="Extracted original ROM root. Default: ../IRDO_Extracted")
    parser.add_argument("--out", type=Path, default=DEFAULT_OUT, help="Output directory. Default: ./dump")
    args = parser.parse_args(argv)

    resources = args.resources or ["all"]
    if "all" in resources:
        resources = ["species", "personal", "children", "moves", "items", "evolutions", "learnsets", "encounters", "trainers", "pokegra_battle", "pokegra_icons", "system_text"]

    for resource in resources:
        dump_resource(resource, args.romfs.resolve(), args.out.resolve())
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
