#!/usr/bin/env python3

from __future__ import annotations

import argparse
import re
import struct
import tomllib
from pathlib import Path

from ndspy import narc


TERMINATOR = 0xFFFF
LINE_BREAK = 0xFFFE
COMPRESSED_TEXT_FLAG = 0xF100
DEFAULT_TEXT_BITS = 16
DEFAULT_PAYLOAD_START = None

CONTROL_SEQUENCES = {
    (0xF000, 0xBE00, 0x0000): r"\l",
    (0xF000, 0xBE01, 0x0000): r"\c",
}

REVERSE_CONTROL_SEQUENCES = {value[1]: key for key, value in CONTROL_SEQUENCES.items()}

VAR_IDENT = 0xF000
VARIABLE_CODES = {
    0xBE00: ("SCROLL", True),
    0xBE01: ("CLEAR", True),
    0xBE02: ("WAIT", True),
    0xBE09: ("SPEED", True),
    0xBDFF: ("BLANK", True),
    0xFF00: ("COLOR", True),
    0xBD00: ("COLOREX", True),
    0xBD01: ("COLORRESET", True),
    0xBD02: ("CENTER", True),
    0xBD03: ("RIGHT", True),
    0xBD04: ("SKIPPIXELS", True),
    0xBD05: ("SETXPOS", True),
    0x0100: ("TRNAME", False),
    0x0101: ("PKNAME", False),
    0x0102: ("PKNICK", False),
    0x0103: ("TYPE", False),
    0x0105: ("LOCATION", False),
    0x0106: ("ABILITY", False),
    0x0107: ("MOVE", False),
    0x0108: ("ITEM1", False),
    0x0109: ("ITEM2", False),
    0x010A: ("DRESSUPPROP", False),
    0x010B: ("BOX", False),
    0x010C: ("BATTLEPK", False),
    0x010D: ("STAT", False),
    0x010E: ("TRCLASS", False),
    0x010F: ("HOBBY", False),
    0x0110: ("PASSPOWER", False),
    0x0112: ("BAGPOCKET", False),
    0x0113: ("SURVEYRESULT", False),
    0x011C: ("GENERIC", False),
    0x0122: ("DRESSUPSHOWNAME", False),
    0x0123: ("DRESSUPSHOWFEELING", False),
    0x0124: ("COUNTRY", False),
    0x0125: ("PROVINCE", False),
    0x0131: ("DRESSUPBODYPART", False),
    0x0132: ("DECORNAME", False),
    0x0133: ("DRESSUPAUDIENCE", False),
    0x0135: ("MEDAL", False),
    0x0136: ("MEDALISTRANK", False),
    0x0137: ("JOINAVINPUT", False),
    0x013B: ("TOURNAMENT", False),
    0x013C: ("BATTLEMODE", False),
    0x013D: ("INSTTITLE", False),
    0x013E: ("WEATHER", False),
    0x013F: ("MOVIENAME", False),
    0x0140: ("FUNFESTMISSION", False),
    0x0142: ("JOINAVRANK", False),
    0x0143: ("ENTRALINKLVL", False),
    0x0200: ("NUM1", False),
    0x0201: ("NUM2", False),
    0x0202: ("NUM3", False),
    0x0203: ("NUM4", False),
    0x0204: ("NUM5", False),
    0x0205: ("NUM6", False),
    0x0206: ("NUM7", False),
    0x0207: ("NUM8", False),
    0x0208: ("NUM9", False),
}
VARIABLE_CODES_BY_NAME = {name: (code, is_imperative) for code, (name, is_imperative) in VARIABLE_CODES.items()}
VARIABLE_PATTERN = re.compile(r"\[(?:(VAR)\s+)?([A-Za-z0-9_]+|0x[0-9A-Fa-f]{1,4})(?:\(([^]]*)\))?\]")


def rol16(value: int, amount: int) -> int:
    return ((value << amount) | (value >> (16 - amount))) & 0xFFFF


def gfstring_initial_key(index: int) -> int:
    return ((index + 3) * 0x2983) & 0xFFFF


def gfstring_crypt_words(words: list[int], index: int) -> list[int]:
    key = gfstring_initial_key(index)
    result = []
    for word in words:
        result.append((word ^ key) & 0xFFFF)
        key = rol16(key, 3)
    return result


def decrypt_words(words: list[int], index: int) -> list[int]:
    return gfstring_crypt_words(words, index)


def encrypt_words(code_units: list[int], index: int) -> list[int]:
    words = list(code_units)
    if not words or words[-1] != TERMINATOR:
        words.append(TERMINATOR)
    return gfstring_crypt_words(words, index)


def unpack_text_units(words: list[int]) -> tuple[int, list[int]]:
    if words and words[0] == COMPRESSED_TEXT_FLAG:
        return 9, unpack_compressed_units(words[1:])
    return DEFAULT_TEXT_BITS, words


def unpack_compressed_units(words: list[int], text_bits: int = 9) -> list[int]:
    mask = (1 << text_bits) - 1
    units: list[int] = []
    bit_buffer = 0
    bit_count = 0
    for word in words:
        bit_buffer |= (word & 0xFFFF) << bit_count
        bit_count += DEFAULT_TEXT_BITS
        while bit_count >= text_bits:
            unit = bit_buffer & mask
            bit_buffer >>= text_bits
            bit_count -= text_bits
            if unit == mask:
                if not units or units[-1] != TERMINATOR:
                    units.append(TERMINATOR)
                return units
            units.append(unit)
    if not units or units[-1] != TERMINATOR:
        units.append(TERMINATOR)
    return units


def pack_compressed_units(code_units: list[int], text_bits: int = 9) -> list[int]:
    mask = (1 << text_bits) - 1
    units = [unit & mask for unit in code_units if unit != TERMINATOR]
    words = [COMPRESSED_TEXT_FLAG]
    bit_buffer = 0
    bit_count = 0
    for unit in units:
        bit_buffer |= unit << bit_count
        bit_count += text_bits
        while bit_count >= DEFAULT_TEXT_BITS:
            words.append(bit_buffer & 0xFFFF)
            bit_buffer >>= DEFAULT_TEXT_BITS
            bit_count -= DEFAULT_TEXT_BITS
    if bit_count > 0:
        words.append((bit_buffer | (mask << bit_count)) & 0xFFFF)
    words.append(TERMINATOR)
    return words


def escape_toml_string(value: str) -> str:
    out = ['"']
    for char in value:
        code = ord(char)
        if char == "\\":
            out.append("\\\\")
        elif char == '"':
            out.append('\\"')
        elif char == "\n":
            out.append("\\n")
        elif char == "\r":
            out.append("\\r")
        elif char == "\t":
            out.append("\\t")
        elif code < 0x20 or 0x7F <= code <= 0x9F:
            out.append(f"\\u{code:04X}")
        else:
            out.append(char)
    out.append('"')
    return "".join(out)


def text_units_to_plaintext(code_units: list[int]) -> list[str]:
    parts: list[str] = [""]
    index = 0
    while index < len(code_units):
        unit = code_units[index]
        if unit == TERMINATOR:
            parts[-1] += "$"
            index += 1
            continue
        if unit == LINE_BREAK:
            parts[-1] += r"\n"
            parts.append("")
            index += 1
            continue

        matched = False
        for sequence, escaped in CONTROL_SEQUENCES.items():
            if tuple(code_units[index : index + len(sequence)]) == sequence:
                parts[-1] += escaped
                index += len(sequence)
                matched = True
                break
        if matched:
            continue

        if unit == VAR_IDENT and index + 2 < len(code_units):
            cmd = code_units[index + 1]
            arg_count = code_units[index + 2]
            end = index + 3 + arg_count
            if end <= len(code_units):
                code_name, is_imperative = VARIABLE_CODES.get(cmd, (f"0x{cmd:04X}", False))
                prefix = "" if is_imperative else "VAR "
                args = ", ".join(str(arg) for arg in code_units[index + 3 : end])
                parts[-1] += f"[{prefix}{code_name}{f'({args})' if args else ''}]"
                index = end
                continue

        if unit == ord("$"):
            parts[-1] += r"\$"
        elif unit == ord("\\"):
            parts[-1] += r"\\"
        elif unit == ord('"'):
            parts[-1] += r"\""
        elif unit == ord("\t"):
            parts[-1] += r"\t"
        elif unit == ord("\r"):
            parts[-1] += r"\r"
        elif 0x20 <= unit <= 0xD7FF or 0xE000 <= unit <= 0xFFFD:
            parts[-1] += chr(unit)
        else:
            parts[-1] += f"\\x{unit:04X}"
        index += 1
    return parts


def parse_plaintext_units(text_parts: list[str] | str) -> list[int]:
    if isinstance(text_parts, str):
        text = text_parts
    else:
        text = "".join(str(part) for part in text_parts)

    units: list[int] = []
    index = 0
    while index < len(text):
        char = text[index]
        if char == "$":
            units.append(TERMINATOR)
            index += 1
            continue
        if char == "[":
            match = VARIABLE_PATTERN.match(text, index)
            if match:
                units.extend(parse_variable_units(match))
                index = match.end()
                continue
        if char == "\n":
            units.append(LINE_BREAK)
            index += 1
            continue
        if char != "\\":
            units.append(ord(char) & 0xFFFF)
            index += 1
            continue

        if index + 1 >= len(text):
            units.append(ord("\\"))
            index += 1
            continue
        escaped = text[index + 1]
        if escaped == "n":
            units.append(LINE_BREAK)
            index += 2
        elif escaped == "l":
            units.extend(REVERSE_CONTROL_SEQUENCES["l"])
            index += 2
        elif escaped == "c":
            units.extend(REVERSE_CONTROL_SEQUENCES["c"])
            index += 2
        elif escaped == "t":
            units.append(ord("\t"))
            index += 2
        elif escaped == "r":
            units.append(ord("\r"))
            index += 2
        elif escaped == "x" and index + 5 < len(text):
            units.append(int(text[index + 2 : index + 6], 16) & 0xFFFF)
            index += 6
        else:
            units.append(ord(escaped) & 0xFFFF)
            index += 2
    if not units or units[-1] != TERMINATOR:
        units.append(TERMINATOR)
    return units


def parse_variable_units(match: re.Match[str]) -> list[int]:
    name = match.group(2)
    name_key = name.upper()
    if name_key.startswith("0X"):
        cmd = int(name_key, 16) & 0xFFFF
    else:
        cmd = VARIABLE_CODES_BY_NAME[name_key][0]
    args_text = match.group(3)
    args = []
    if args_text:
        args = [int(part.strip(), 0) & 0xFFFF for part in args_text.split(",") if part.strip()]
    return [VAR_IDENT, cmd, len(args), *args]


def decode_message_file(data: bytes) -> dict:
    if len(data) < 0x14:
        raise ValueError("message file is too small")
    section_count, entry_count = struct.unpack_from("<HH", data, 0)
    total_size, reserved, section_offset, section_size = struct.unpack_from("<IIII", data, 4)
    if section_count != 1:
        raise ValueError(f"unsupported section count {section_count}")
    if section_offset + section_size > len(data):
        raise ValueError("section exceeds file size")

    entries = []
    table_offset = section_offset + 4
    for index in range(entry_count):
        relative_offset, length = struct.unpack_from("<II", data, table_offset + index * 8)
        offset = section_offset + relative_offset
        if offset + length * 2 > len(data):
            raise ValueError(f"entry {index} exceeds file size")
        encrypted_words = list(struct.unpack_from("<" + "H" * length, data, offset))
        encoded_units = decrypt_words(encrypted_words, index)
        text_bits, code_units = unpack_text_units(encoded_units)
        entries.append(
            {
                "index": index,
                "offset": relative_offset,
                "text_bits": text_bits,
                "code_units": code_units,
                "text": text_units_to_plaintext(code_units),
            }
        )

    text_end = max((section_offset + entry["offset"] + struct.unpack_from("<I", data, table_offset + entry["index"] * 8 + 4)[0] * 2) for entry in entries) if entries else section_offset

    return {
        "section_count": section_count,
        "entry_count": entry_count,
        "reserved": reserved,
        "total_size": total_size,
        "section_offset": section_offset,
        "section_size": section_size,
        "payload_start": entries[0]["offset"] if entries else 4,
        "trailing_data": data[text_end : section_offset + section_size].hex() if entries else "",
        "entries": entries,
    }


def entry_code_units(entry: dict) -> list[int]:
    if "code_units" in entry:
        return [int(unit) & 0xFFFF for unit in entry["code_units"]]
    return parse_plaintext_units(entry.get("text", []))


def normalize_entry(entry: dict, index: int) -> dict:
    normalized = dict(entry)
    normalized.setdefault("index", index)
    normalized.setdefault("text_bits", DEFAULT_TEXT_BITS)
    units = entry_code_units(normalized)
    if int(normalized.get("text_bits", DEFAULT_TEXT_BITS)) == DEFAULT_TEXT_BITS and units and units[0] == COMPRESSED_TEXT_FLAG:
        text_bits, unpacked_units = unpack_text_units(units)
        normalized["text_bits"] = text_bits
        normalized["code_units"] = unpacked_units
        normalized.pop("text", None)
    return normalized


def entry_encoded_units(entry: dict) -> list[int]:
    code_units = entry_code_units(entry)
    text_bits = int(entry.get("text_bits", DEFAULT_TEXT_BITS))
    if text_bits == DEFAULT_TEXT_BITS:
        return code_units
    return pack_compressed_units(code_units, text_bits)


def encode_message_file(doc: dict) -> bytes:
    entries = doc["entries"]
    entry_count = len(entries)
    section_offset = 0x10
    cursor = int(doc.get("payload_start", 4 + entry_count * 8))
    table = bytearray()
    payload = bytearray()

    for index, entry in enumerate(entries):
        words = encrypt_words(entry_encoded_units(entry), index)
        table += struct.pack("<II", cursor, len(words))
        payload += struct.pack("<" + "H" * len(words), *words)
        cursor += len(words) * 2

    table_size = 4 + len(table)
    if int(doc.get("payload_start", table_size)) < table_size:
        raise ValueError("payload_start overlaps the message table")
    gap = b"\x00" * (int(doc.get("payload_start", table_size)) - table_size)
    trailing_data = bytes.fromhex(str(doc.get("trailing_data", "")))
    section_size = table_size + len(gap) + len(payload) + len(trailing_data)
    out = bytearray()
    out += struct.pack("<HHII", 1, entry_count, section_size, int(doc.get("reserved", 0)))
    out += struct.pack("<II", section_offset, section_size)
    out += table
    out += gap
    out += payload
    out += trailing_data
    return bytes(out)


def write_toml(doc: dict, output: Path) -> None:
    output.parent.mkdir(parents=True, exist_ok=True)
    default_payload_start = 4 + len(doc["entries"]) * 8
    metadata = []
    if int(doc.get("section_count", 1)) != 1:
        metadata.append(f"section_count = {doc.get('section_count', 1)}")
    if int(doc.get("reserved", 0)) != 0:
        metadata.append(f"reserved = {doc.get('reserved', 0)}")
    if int(doc.get("payload_start", default_payload_start)) != default_payload_start:
        metadata.append(f"payload_start = {doc.get('payload_start', default_payload_start)}")
    if str(doc.get("trailing_data", "")):
        metadata.append(f"trailing_data = {escape_toml_string(doc.get('trailing_data', ''))}")
    lines = []
    if metadata:
        lines.append("[msg]")
        lines.extend(metadata)
        lines.append("")
    for index, entry in enumerate(doc["entries"]):
        lines.append(f"[msg.section_0.entries.{index}]")
        if int(entry.get("text_bits", DEFAULT_TEXT_BITS)) != DEFAULT_TEXT_BITS:
            lines.append(f"text_bits = {int(entry['text_bits'])}")
        lines.append("text = [")
        for part in text_units_to_plaintext(entry_code_units(entry)):
            lines.append(f"  {escape_toml_string(part)},")
        lines.append("]")
        lines.append("")
    output.write_text("\n".join(lines), encoding="utf-8")


def read_toml(path: Path) -> dict:
    raw = tomllib.loads(path.read_text(encoding="utf-8"))
    if "msg" in raw:
        msg = raw["msg"]
        section = msg.get("section_0", {})
        entries = []
        raw_entries = section.get("entries", [])
        if isinstance(raw_entries, dict):
            entry_items = sorted(raw_entries.items(), key=lambda item: int(item[0]))
            raw_entries = [entry for _, entry in entry_items]
        for index, entry in enumerate(raw_entries):
            entries.append(normalize_entry({
                "index": index,
                "text_bits": entry.get("text_bits", DEFAULT_TEXT_BITS),
                "text": entry.get("text", []),
            }, index))
        return {
            "section_count": msg.get("section_count", 1),
            "reserved": msg.get("reserved", 0),
            "payload_start": msg.get("payload_start", 4 + len(entries) * 8),
            "trailing_data": msg.get("trailing_data", ""),
            "entries": entries,
        }

    message_file = raw.get("message_file", {})
    entries = []
    for index, entry in enumerate(raw.get("entries", [])):
        converted = dict(entry)
        converted.setdefault("index", index)
        converted.setdefault("text_bits", DEFAULT_TEXT_BITS)
        entries.append(normalize_entry(converted, index))
    return {
        "section_count": message_file.get("section_count", 1),
        "reserved": message_file.get("reserved", 0),
        "payload_start": message_file.get("payload_start", 4 + len(entries) * 8),
        "trailing_data": message_file.get("trailing_data", ""),
        "entries": entries,
    }


def dump_file(input_path: Path, output_path: Path) -> None:
    write_toml(decode_message_file(input_path.read_bytes()), output_path)


def build_file(input_path: Path, output_path: Path) -> None:
    output_path.parent.mkdir(parents=True, exist_ok=True)
    output_path.write_bytes(encode_message_file(read_toml(input_path)))


def dump_narc(input_path: Path, output_dir: Path) -> None:
    output_dir.mkdir(parents=True, exist_ok=True)
    archive = narc.NARC.fromFile(input_path)
    for index, data in enumerate(archive.files):
        write_toml(decode_message_file(data), output_dir / f"{index}.toml")


def build_dir(input_dir: Path, output_dir: Path, stamp: Path | None = None) -> None:
    output_dir.mkdir(parents=True, exist_ok=True)
    for old_file in output_dir.iterdir():
        if old_file.is_file():
            old_file.unlink()
    (output_dir / ".arc").write_text(".arc\ncompress default auto\n")
    inputs = sorted(input_dir.glob("*.toml"), key=lambda path: int(path.stem))
    for index, path in enumerate(inputs):
        (output_dir / str(index)).write_bytes(encode_message_file(read_toml(path)))
    if stamp is not None:
        stamp.parent.mkdir(parents=True, exist_ok=True)
        stamp.write_text(f"{output_dir}\n{len(inputs)} files\n")


def main() -> int:
    parser = argparse.ArgumentParser(description="Decode and encode Pokemon Gen 5 message files as TOML.")
    subparsers = parser.add_subparsers(dest="command", required=True)

    dump_file_parser = subparsers.add_parser("dump-file")
    dump_file_parser.add_argument("input", type=Path)
    dump_file_parser.add_argument("output", type=Path)

    build_file_parser = subparsers.add_parser("build-file")
    build_file_parser.add_argument("input", type=Path)
    build_file_parser.add_argument("output", type=Path)

    dump_narc_parser = subparsers.add_parser("dump-narc")
    dump_narc_parser.add_argument("input", type=Path)
    dump_narc_parser.add_argument("output", type=Path)

    build_dir_parser = subparsers.add_parser("build-dir")
    build_dir_parser.add_argument("input", type=Path)
    build_dir_parser.add_argument("output", type=Path)
    build_dir_parser.add_argument("--stamp", type=Path)

    args = parser.parse_args()
    if args.command == "dump-file":
        dump_file(args.input, args.output)
    elif args.command == "build-file":
        build_file(args.input, args.output)
    elif args.command == "dump-narc":
        dump_narc(args.input, args.output)
    elif args.command == "build-dir":
        build_dir(args.input, args.output, args.stamp)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
