from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
from struct import unpack_from
from typing import Any
import tomllib

from ndspy import narc


REPO_ROOT = Path(__file__).resolve().parents[3]
DEFAULT_ROMFS = REPO_ROOT.parent / "IRDO_Extracted"
DEFAULT_OUT = REPO_ROOT / "dump"


@dataclass(frozen=True)
class NarcSpec:
    name: str
    path: str
    description: str

    @property
    def parts(self) -> tuple[str, ...]:
        return tuple(self.path.split("/"))

    def source_path(self, romfs: Path) -> Path:
        return romfs / "data" / Path(*self.parts)


NARCS: dict[str, NarcSpec] = {
    "personal": NarcSpec("personal", "a/0/1/6", "Pokemon personal data"),
    "children": NarcSpec("children", "a/0/1/7", "Pokemon child-species lookup data"),
    "learnsets": NarcSpec("learnsets", "a/0/1/8", "Level-up learnsets"),
    "evolutions": NarcSpec("evolutions", "a/0/1/9", "Evolution tables"),
    "moves": NarcSpec("moves", "a/0/2/1", "Move data"),
    "items": NarcSpec("items", "a/0/2/4", "Item data"),
    "encounters": NarcSpec("encounters", "a/1/2/7", "Wild encounter tables"),
    "trdata": NarcSpec("trdata", "a/0/9/1", "Trainer metadata"),
    "trpoke": NarcSpec("trpoke", "a/0/9/2", "Trainer parties"),
    "pokegra_battle": NarcSpec("pokegra_battle", "a/0/0/4", "Battle sprite assets"),
    "pokegra_icons": NarcSpec("pokegra_icons", "a/0/0/7", "Pokemon icon assets"),
    "pokegra_footprints": NarcSpec("pokegra_footprints", "a/1/6/5", "Pokemon footprint assets"),
    "system_text": NarcSpec("system_text", "a/0/0/2", "System message files"),
    "game_text": NarcSpec("game_text", "a/0/0/3", "Map and event message files"),
}


class Reader:
    def __init__(self, data: bytes, source: str = "<bytes>"):
        self.data = data
        self.source = source
        self.offset = 0

    def _read(self, fmt: str) -> int:
        value = unpack_from("<" + fmt, self.data, self.offset)[0]
        self.offset += {"B": 1, "b": 1, "H": 2, "h": 2, "I": 4, "i": 4}[fmt]
        return value

    def u8(self) -> int:
        return self._read("B")

    def s8(self) -> int:
        return self._read("b")

    def u16(self) -> int:
        return self._read("H")

    def s16(self) -> int:
        return self._read("h")

    def u32(self) -> int:
        return self._read("I")

    def s32(self) -> int:
        return self._read("i")

    def done(self) -> bool:
        return self.offset >= len(self.data)

    def remaining(self) -> int:
        return len(self.data) - self.offset


def read_narc_files(path: Path) -> list[bytes]:
    if path.is_dir():
        return [
            file.read_bytes()
            for file in sorted(path.iterdir(), key=lambda item: int(item.name) if item.name.isdigit() else item.name)
            if file.is_file() and not file.name.startswith(".")
        ]
    if not path.exists():
        raise FileNotFoundError(path)
    return list(narc.NARC.fromFile(path).files)


def load_label_map(enum_name: str) -> dict[int, str]:
    path = REPO_ROOT / "tools" / "mkdata" / "enum" / f"{enum_name}.toml"
    labels: dict[int, str] = {}
    with path.open("rb") as enum_file:
        defines = tomllib.load(enum_file).get("DEFINE", {})
    for key, value in defines.items():
        labels[int(value)] = key
    return labels


def label(labels: dict[int, str], value: int) -> str | int:
    return labels.get(value, value)


def flags(labels: dict[int, str], value: int) -> list[str | int]:
    result: list[str | int] = []
    bit = 1
    while bit <= value:
        if value & bit:
            result.append(labels.get(bit, bit))
        bit <<= 1
    return result


def toml_scalar(value: Any) -> str:
    if isinstance(value, bool):
        return "true" if value else "false"
    if isinstance(value, int):
        return str(value)
    if isinstance(value, str):
        return '"' + value.replace("\\", "\\\\").replace('"', '\\"') + '"'
    if isinstance(value, list):
        return "[" + ", ".join(toml_scalar(item) for item in value) + "]"
    raise TypeError(f"Unsupported TOML scalar: {value!r}")


def toml_key(key: str) -> str:
    if key.isascii() and key.replace("_", "").replace("-", "").isalnum() and not key[0].isdigit():
        return key
    return '"' + key.replace("\\", "\\\\").replace('"', '\\"') + '"'


def toml_table(table: str) -> str:
    return ".".join(toml_key(part) for part in table.split("."))


def write_toml(path: Path, data: dict[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    lines: list[str] = []

    def is_array_of_tables(value: Any) -> bool:
        return isinstance(value, list) and bool(value) and isinstance(value[0], dict)

    def emit_assignments(values: dict[str, Any]) -> None:
        for key, value in values.items():
            if isinstance(value, dict) or is_array_of_tables(value):
                continue
            lines.append(f"{toml_key(key)} = {toml_scalar(value)}")

    def emit_children(table: str, values: dict[str, Any]) -> None:
        for key, value in values.items():
            if isinstance(value, dict):
                emit_table(f"{table}.{key}", value)
            elif is_array_of_tables(value):
                emit_array_of_tables(f"{table}.{key}", value)

    def emit_array_of_tables(table: str, values: list[dict[str, Any]]) -> None:
        for item in values:
            lines.append("")
            lines.append(f"[[{toml_table(table)}]]")
            emit_assignments(item)
            emit_children(table, item)

    def emit_table(table: str, values: dict[str, Any]) -> None:
        if lines:
            lines.append("")
        lines.append(f"[{toml_table(table)}]")
        emit_assignments(values)
        emit_children(table, values)

    for key, value in data.items():
        if isinstance(value, dict):
            emit_table(key, value)
        elif is_array_of_tables(value):
            emit_array_of_tables(key, value)
        else:
            lines.append(f"{toml_key(key)} = {toml_scalar(value)}")
    path.write_text("\n".join(lines) + "\n")


def dump_manifest(path: Path, spec: NarcSpec, files: list[bytes]) -> None:
    write_toml(
        path,
        {
            "narc": {
                "name": spec.name,
                "path": spec.path,
                "description": spec.description,
                "file_count": len(files),
            }
        },
    )


def detect_magic(data: bytes) -> str:
    if len(data) < 4:
        return "empty" if not data else "bin"
    magic = data[:4]
    return {
        b"RGCN": "ncgr",
        b"RLCN": "nclr",
        b"RECN": "ncer",
        b"RNAN": "nanr",
        b"RCMN": "nmcr",
        b"RAMN": "nmar",
    }.get(magic, "bin")
