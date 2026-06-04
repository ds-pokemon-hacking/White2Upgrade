#!/usr/bin/env python3
import argparse
import json
import shutil
import struct
import sys
import tomllib
from pathlib import Path

TOOLS_ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(TOOLS_ROOT / "pwan"))

from compile_pwan import compile_pwan  # noqa: E402
from PIL import Image  # noqa: E402


CONFIG_MAGIC = 0x434E5750
CONFIG_VERSION = 1
CONFIG_ENTRY_SIZE = 8
CONFIG_HEADER_SIZE = 16
MAX_TIMELINE_ENTRIES = 128
NCGR_TILED_HEADER = bytes.fromhex(
    "5247434efffe01013020000010000100"
    "52414843202000000c000c0003000000"
    "00000000000000000012000018000000"
)
NCGR_BITMAP_HEADER = bytes.fromhex(
    "5247434efffe01014040000010000200"
    "52414843204000001000200003000000"
    "00000000010000000040000018000000"
)
NCGR_BITMAP_TAIL = bytes.fromhex("534f5043100000000000000020001000")
NCLR_HEADER = bytes.fromhex(
    "524c434efffe00014800000010000100"
    "54544c503800000004000a0000000000"
    "2000000010000000"
)
TOML_BINARY_KINDS = {"ncer", "nanr", "nmcr", "nmar"}
ORDER_FILE = "order.toml"
G2D_MAGIC_BY_KIND = {
    "ncer": "RECN",
    "nanr": "RNAN",
    "nmcr": "RCMN",
    "nmar": "RAMN",
}
PRIMARY_SECTION_BY_KIND = {
    "ncer": "KBEC",
    "nanr": "KNBA",
    "nmcr": "KBCM",
    "nmar": "KNBA",
}


def load_toml(path: Path) -> dict:
    with path.open("rb") as f:
        return tomllib.load(f)


def clean_dir(path: Path) -> None:
    if path.exists():
        shutil.rmtree(path)
    path.mkdir(parents=True, exist_ok=True)


def ensure_dir(path: Path) -> None:
    path.mkdir(parents=True, exist_ok=True)


def lz11_compress(data: bytes) -> bytes:
    # Literal-only LZ11 is larger than search-compressed LZ11, but it is still a
    # valid 0x11 stream and keeps ordinary rebuilds fast.
    out = bytearray(struct.pack("<I", (len(data) << 8) | 0x11))
    for index in range(0, len(data), 8):
        out.append(0)
        out.extend(data[index:index + 8])
    padding = (-len(out)) % 4
    if padding:
        out.extend(b"\xff" * padding)
    return bytes(out)


def ntr_to_rgb(value: int) -> tuple[int, int, int]:
    return (
        ((value & 0x1F) * 255) // 31,
        (((value >> 5) & 0x1F) * 255) // 31,
        (((value >> 10) & 0x1F) * 255) // 31,
    )


def rgb_to_ntr(red: int, green: int, blue: int) -> int:
    return (red // 8) | ((green // 8) << 5) | ((blue // 8) << 10)


def png_indices(path: Path) -> tuple[Image.Image, bytes]:
    image = Image.open(path)
    if image.mode != "P":
        raise RuntimeError(f"{path}: expected indexed PNG")
    return image, bytes(image.getdata())


def palette_from_nclr(data: bytes) -> list[int]:
    if len(data) < len(NCLR_HEADER) + 32 or data[:4] != b"RLCN":
        raise RuntimeError("not a supported pokegra NCLR")
    return [int.from_bytes(data[len(NCLR_HEADER) + i * 2:len(NCLR_HEADER) + i * 2 + 2], "little") for i in range(16)]


def read_jasc_palette(path: Path) -> list[tuple[int, int, int]]:
    lines = path.read_text().splitlines()
    if len(lines) < 3 or lines[0] != "JASC-PAL" or lines[1] != "0100":
        raise RuntimeError(f"{path}: expected JASC-PAL palette")
    color_count = int(lines[2])
    if color_count < 16:
        raise RuntimeError(f"{path}: expected at least 16 colors")
    colors = []
    for line in lines[3:3 + 16]:
        parts = line.split()
        if len(parts) != 3:
            raise RuntimeError(f"{path}: invalid palette row {line!r}")
        red, green, blue = (int(part) for part in parts)
        colors.append((red, green, blue))
    return colors


def palette_payload(path: Path, high_bits: list[int] | None = None) -> bytes:
    palette = read_jasc_palette(path)
    high_bit_set = set(high_bits or [])
    out = bytearray()
    for index, (red, green, blue) in enumerate(palette):
        value = rgb_to_ntr(red, green, blue)
        if index in high_bit_set:
            value |= 0x8000
        out.extend(value.to_bytes(2, "little"))
    return bytes(out)


def build_nclr_from_pal(path: Path, high_bits: list[int] | None = None) -> bytes:
    return NCLR_HEADER + palette_payload(path, high_bits)


def pack_oam_attribute(attribute: dict) -> bytes:
    attr0 = (
        (int(attribute["position_y"]) & 0xFF)
        | ((int(attribute.get("transformable", 0)) & 1) << 8)
        | ((int(attribute.get("double_sized", 0)) & 1) << 9)
        | ((int(attribute.get("object_mode", 0)) & 3) << 10)
        | ((int(attribute.get("mosaic", 0)) & 1) << 12)
        | ((int(attribute.get("palette_mode", 0)) & 1) << 13)
        | ((int(attribute.get("key_shape", 0)) & 3) << 14)
    )
    attr1 = (
        (int(attribute["position_x"]) & 0x1FF)
        | ((int(attribute.get("transform_parameter", 0)) & 7) << 9)
        | ((int(attribute.get("horizontal_flip", 0)) & 1) << 12)
        | ((int(attribute.get("vertical_flip", 0)) & 1) << 13)
        | ((int(attribute.get("key_size", 0)) & 3) << 14)
    )
    attr2 = (
        (int(attribute["tile_index"]) & 0x3FF)
        | ((int(attribute.get("priority", 0)) & 3) << 10)
        | ((int(attribute.get("palette_index", 0)) & 0xF) << 12)
    )
    return struct.pack("<HHH", attr0, attr1, attr2)


def frame_property_size(prop: dict) -> int:
    animation_type = int(prop.get("animation_type", 0))
    if animation_type == 0:
        return 2
    if animation_type == 1:
        return 16
    if animation_type == 2:
        return 8
    raise RuntimeError(f"unsupported animation_type {animation_type}")


def fixed_20_12(value: int | float) -> int:
    if isinstance(value, float):
        return round(value * 4096.0) & 0xFFFFFFFF
    return int(value) & 0xFFFFFFFF


def pack_kbec(section: dict) -> bytes:
    if isinstance(section.get("cells"), dict):
        section = dict(section)
        section["cells"] = [
            {"name": name, **cell}
            for name, cell in section["cells"].items()
        ]

    if section.get("cells") and isinstance(section["cells"][0].get("objects"), list):
        attributes = []
        cells = []
        for cell in section["cells"]:
            objects = cell.get("objects", [])
            derived = dict(cell)
            derived["number_objects"] = len(objects)
            derived["offset_object"] = len(attributes) * 6
            derived.pop("objects", None)
            cells.append(derived)
            attributes.extend(objects)

        section = dict(section)
        section["cells"] = cells
        section["attributes"] = attributes
        section["number_cells"] = len(cells)
        section["offset_data_cell"] = 0x18

    if isinstance(section.get("object_groups"), dict):
        section = dict(section)
        section["object_groups"] = [
            {"name": name, **group}
            for name, group in section["object_groups"].items()
        ]

    if section.get("object_groups"):
        object_groups = section.get("object_groups", [])
        group_offsets = {}
        attributes = []
        for group in object_groups:
            name = group.get("name")
            if not isinstance(name, str):
                raise RuntimeError("KBEC object group requires name")
            group_offsets[name] = len(attributes) * 6
            attributes.extend(group.get("attributes", []))

        cells = []
        for cell in section.get("cells", []):
            object_group = cell.get("objects")
            if object_group not in group_offsets:
                raise RuntimeError(f"KBEC cell references unknown object group {object_group!r}")
            group = next(group for group in object_groups if group.get("name") == object_group)
            derived = dict(cell)
            derived["number_objects"] = len(group.get("attributes", []))
            derived["offset_object"] = group_offsets[object_group]
            cells.append(derived)

        section = dict(section)
        section["cells"] = cells
        section["attributes"] = attributes
        section["number_cells"] = len(cells)
        section["offset_data_cell"] = 0x18

    cells = section.get("cells", [])
    attributes = section.get("attributes", [])
    use_bounds = int(section.get("use_bounds", 0))
    cell_size = 16 if use_bounds else 8
    body = bytearray(struct.pack(
        "<HHIIIII",
        int(section.get("number_cells", len(cells))),
        use_bounds,
        int(section.get("offset_data_cell", 0x18)),
        int(section.get("size_boundary", 0)),
        int(section.get("unknown0", 0)),
        int(section.get("unknown1", 0)),
        int(section.get("unknown2", 0)),
    ))
    if len(body) < int(section.get("offset_data_cell", 0x18)):
        body.extend(bytes(int(section.get("offset_data_cell", 0x18)) - len(body)))
    for cell in cells:
        body.extend(struct.pack(
            "<HBBI",
            int(cell["number_objects"]),
            int(cell.get("unknown0", 0)) & 0xFF,
            int(cell.get("unknown1", 0)) & 0xFF,
            int(cell["offset_object"]),
        ))
        if use_bounds:
            body.extend(struct.pack(
                "<hhhh",
                int(cell.get("bound_right", 0)),
                int(cell.get("bound_bottom", 0)),
                int(cell.get("bound_left", 0)),
                int(cell.get("bound_top", 0)),
            ))
    expected = int(section.get("offset_data_cell", 0x18)) + len(cells) * cell_size
    if len(body) < expected:
        body.extend(bytes(expected - len(body)))
    for attribute in attributes:
        body.extend(pack_oam_attribute(attribute))
    body.extend(bytes(int(section.get("padding_size", 0))))
    return bytes(body)


def pack_frame_property(prop: dict) -> bytes:
    animation_type = int(prop.get("animation_type", 0))
    cell_index = prop.get("cell_index", prop.get("cell", prop.get("multi_cell", 0)))
    if animation_type == 0:
        return struct.pack("<H", int(cell_index))
    if animation_type == 1:
        return struct.pack(
            "<HHIIhh",
            int(cell_index),
            int(prop.get("rotate", 0)),
            fixed_20_12(prop.get("scale_w", 0)),
            fixed_20_12(prop.get("scale_h", 0)),
            int(prop.get("translate_x", 0)),
            int(prop.get("translate_y", 0)),
        )
    if animation_type == 2:
        return struct.pack(
            "<HHhh",
            int(cell_index),
            int(prop.get("unknown0", 0)),
            int(prop.get("translate_x", 0)),
            int(prop.get("translate_y", 0)),
        )
    raise RuntimeError(f"unsupported animation_type {animation_type}")


def pack_knba(section: dict) -> bytes:
    if isinstance(section.get("sequences"), dict):
        section = dict(section)
        section["sequences"] = [
            {"name": name, **sequence}
            for name, sequence in section["sequences"].items()
        ]

    if section.get("sequences") and "frames" in section["sequences"][0]:
        property_offsets = {}
        property_data = bytearray()

        frames = []
        frame_offset = 0
        sequences = []
        for sequence in section.get("sequences", []):
            sequence_frames = sequence.get("frames", [])
            derived_sequence = dict(sequence)
            derived_sequence["number_frames"] = len(sequence_frames)
            derived_sequence["offset_frame"] = frame_offset
            derived_sequence.pop("frames", None)
            sequences.append(derived_sequence)
            frame_offset += len(sequence_frames) * 8
            for frame in sequence_frames:
                if "property" in frame:
                    raise RuntimeError("KNBA authoring frames must inline properties")
                property_offset = len(property_data)
                property_data.extend(pack_frame_property(frame))
                property_data.extend(bytes(int(value) & 0xFF for value in frame.get("padding_after", [])))
                frames.append({
                    "offset_properties": property_offset,
                    "duration_in_frames": frame.get("duration_in_frames", 0),
                    "unknown0": frame.get("frame_unknown", -16657),
                })
        property_data_size = len(property_data)

        section = dict(section)
        section["sequences"] = sequences
        section["frames"] = frames
        section["frame_properties"] = []
        section["property_padding"] = []
        section["number_sequences"] = len(sequences)
        section["number_frames"] = len(frames)
        section["offset_data_sequences"] = 0x18
        section["offset_data_frame"] = 0x18 + len(sequences) * 16
        section["offset_data_frame_properties"] = section["offset_data_frame"] + len(frames) * 8
        section["property_data_size"] = property_data_size
    else:
        property_data = None

    sequences = section.get("sequences", [])
    frames = section.get("frames", [])
    properties = section.get("frame_properties", [])
    property_padding = section.get("property_padding", [])
    property_data_size = int(section.get("property_data_size", 0))
    body = bytearray(struct.pack(
        "<HHIIIII",
        int(section.get("number_sequences", len(sequences))),
        int(section.get("number_frames", len(frames))),
        int(section.get("offset_data_sequences", 0x18)),
        int(section.get("offset_data_frame", 0)),
        int(section.get("offset_data_frame_properties", 0)),
        int(section.get("unknown0", 0)),
        int(section.get("unknown1", 0)),
    ))
    if len(body) < int(section.get("offset_data_sequences", 0x18)):
        body.extend(bytes(int(section.get("offset_data_sequences", 0x18)) - len(body)))
    for sequence in sequences:
        body.extend(struct.pack(
            "<IHHII",
            int(sequence["number_frames"]),
            int(sequence.get("animation_type", 0)),
            int(sequence.get("cell_type", 0)),
            int(sequence.get("loop_mode", 0)),
            int(sequence["offset_frame"]),
        ))
    if len(body) < int(section["offset_data_frame"]):
        body.extend(bytes(int(section["offset_data_frame"]) - len(body)))
    for frame in frames:
        body.extend(struct.pack(
            "<IHh",
            int(frame["offset_properties"]),
            int(frame["duration_in_frames"]),
            int(frame["unknown0"]),
        ))
    if len(body) < int(section["offset_data_frame_properties"]):
        body.extend(bytes(int(section["offset_data_frame_properties"]) - len(body)))
    if property_data is None:
        props = bytearray(property_data_size)
        for prop in properties:
            packed = pack_frame_property(prop)
            offset = int(prop["offset"])
            props[offset:offset + len(packed)] = packed
        for pad in property_padding:
            data = bytes(int(value) & 0xFF for value in pad.get("bytes", []))
            offset = int(pad["offset"])
            props[offset:offset + len(data)] = data
    else:
        props = property_data
    body.extend(props)
    return bytes(body)


def pack_kbcm(section: dict) -> bytes:
    if isinstance(section.get("multi_cells"), dict):
        section = dict(section)
        section["multi_cells"] = [
            {"name": name, **multi_cell}
            for name, multi_cell in section["multi_cells"].items()
        ]

    if section.get("multi_cells") and "properties" in section["multi_cells"][0]:
        properties = []
        offset = 0
        multi_cells = []
        for multi_cell in section.get("multi_cells", []):
            rows = multi_cell.get("properties", [])
            derived = dict(multi_cell)
            derived["number_displayed_cells"] = len(rows)
            derived["number_loaded_cells"] = int(multi_cell.get("loaded_cells", len(rows)))
            derived["offset_data"] = offset
            derived.pop("properties", None)
            derived.pop("loaded_cells", None)
            multi_cells.append(derived)
            offset += len(rows) * 8
            properties.extend(rows)

        section = dict(section)
        section["multi_cells"] = multi_cells
        section["properties"] = properties
        section["number_multi_cells"] = len(multi_cells)
        section["offset_data_multi_cell"] = 0x14
        section["offset_data_multi_cell_properties"] = 0x14 + len(multi_cells) * 8

    multi_cells = section.get("multi_cells", [])
    properties = section.get("properties", [])
    body = bytearray(struct.pack(
        "<HHIIII",
        int(section.get("number_multi_cells", len(multi_cells))),
        int(section.get("unknown0", 0)),
        int(section.get("offset_data_multi_cell", 0x14)),
        int(section.get("offset_data_multi_cell_properties", 0)),
        int(section.get("unknown1", 0)),
        int(section.get("unknown2", 0)),
    ))
    if len(body) < int(section.get("offset_data_multi_cell", 0x14)):
        body.extend(bytes(int(section.get("offset_data_multi_cell", 0x14)) - len(body)))
    for multi_cell in multi_cells:
        body.extend(struct.pack(
            "<HHI",
            int(multi_cell["number_displayed_cells"]),
            int(multi_cell["number_loaded_cells"]),
            int(multi_cell["offset_data"]),
        ))
    if len(body) < int(section["offset_data_multi_cell_properties"]):
        body.extend(bytes(int(section["offset_data_multi_cell_properties"]) - len(body)))
    for prop in properties:
        if "sequence" in prop:
            raise RuntimeError("KBCM sequence references must be resolved before packing")
        body.extend(struct.pack(
            "<HhhBB",
            int(prop.get("index_sequence", 0)),
            int(prop.get("translate_x", 0)),
            int(prop.get("translate_y", 0)),
            int(prop.get("frame_mode", 0)) & 0xFF,
            int(prop.get("unique_id", 0)) & 0xFF,
        ))
    return bytes(body)


def named_index_map(path: Path, key: str) -> dict[str, int]:
    data = load_toml(path)
    values = data.get(key, {})
    if isinstance(values, dict):
        return {name: index for index, name in enumerate(values.keys())}
    if isinstance(values, list):
        out = {}
        for index, value in enumerate(values):
            name = value.get("name")
            if isinstance(name, str):
                out[name] = index
        return out
    return {}


def resolve_named_animation_references(data: dict, name_map: dict[str, int]) -> dict:
    if not name_map or not isinstance(data.get("sequences"), dict):
        return data
    data = dict(data)
    sequences = {}
    for sequence_name, sequence in data["sequences"].items():
        sequence = dict(sequence)
        frames = []
        for frame in sequence.get("frames", []):
            frame = dict(frame)
            for field in ("cell", "multi_cell"):
                if field in frame:
                    name = frame.pop(field)
                    if name not in name_map:
                        raise RuntimeError(f"unknown {field} reference {name!r}")
                    frame["cell_index"] = name_map[name]
            frames.append(frame)
        sequence["frames"] = frames
        sequences[sequence_name] = sequence
    data["sequences"] = sequences
    return data


def resolve_named_multicell_references(data: dict, sequence_map: dict[str, int]) -> dict:
    if not sequence_map or not isinstance(data.get("multi_cells"), dict):
        return data
    data = dict(data)
    multi_cells = {}
    for multi_cell_name, multi_cell in data["multi_cells"].items():
        multi_cell = dict(multi_cell)
        properties = []
        for prop in multi_cell.get("properties", []):
            prop = dict(prop)
            if "sequence" in prop:
                name = prop.pop("sequence")
                if name not in sequence_map:
                    raise RuntimeError(f"unknown sequence reference {name!r}")
                prop["index_sequence"] = sequence_map[name]
            properties.append(prop)
        multi_cell["properties"] = properties
        multi_cells[multi_cell_name] = multi_cell
    data["multi_cells"] = multi_cells
    return data


def pack_lbal(section: dict) -> bytes:
    labels = section.get("labels")
    if not isinstance(labels, list):
        raise RuntimeError("LBAL section requires labels")
    offsets = []
    string_data = bytearray()
    for label in labels:
        if not isinstance(label, str):
            raise RuntimeError("LBAL labels must be strings")
        offsets.append(len(string_data))
        string_data.extend(label.encode("utf-8"))
        string_data.append(0)
    body = bytearray()
    for offset in offsets:
        body.extend(int(offset).to_bytes(4, "little"))
    body.extend(string_data)
    return bytes(body)


def pack_section(section: dict) -> bytes:
    magic = section.get("magic")
    if not isinstance(magic, str) or len(magic.encode("ascii")) != 4:
        raise RuntimeError(f"invalid section magic {magic!r}")

    if magic == "KBEC":
        body = pack_kbec(section)
    elif magic == "KNBA":
        body = pack_knba(section)
    elif magic == "KBCM":
        body = pack_kbcm(section)
    elif magic == "LBAL":
        body = pack_lbal(section)
    elif magic == "TXEU":
        body = struct.pack("<I", int(section.get("extended", 0)))
    else:
        raise RuntimeError(f"unsupported section magic {magic!r}")

    return magic.encode("ascii") + (len(body) + 8).to_bytes(4, "little") + bytes(body)


def authoring_sections(data: dict, expected_kind: str) -> list[dict]:
    primary_magic = PRIMARY_SECTION_BY_KIND[expected_kind]
    primary = {"magic": primary_magic}

    if expected_kind == "ncer":
        for key in ("use_bounds", "size_boundary", "unknown0", "unknown1", "unknown2", "padding_size"):
            if key in data:
                primary[key] = data[key]
        primary["object_groups"] = data.get("object_groups", [])
        primary["cells"] = data.get("cells", [])
    elif expected_kind in ("nanr", "nmar"):
        for key in ("unknown0", "unknown1"):
            if key in data:
                primary[key] = data[key]
        primary["sequences"] = data.get("sequences", [])
    elif expected_kind == "nmcr":
        for key in ("unknown0", "unknown1", "unknown2"):
            if key in data:
                primary[key] = data[key]
        primary["multi_cells"] = data.get("multi_cells", [])
    else:
        raise RuntimeError(f"unsupported G2D kind {expected_kind!r}")

    derived_labels = None
    if expected_kind == "ncer" and isinstance(data.get("cells"), dict):
        derived_labels = list(data["cells"].keys())
    elif expected_kind in ("nanr", "nmar") and isinstance(data.get("sequences"), dict):
        derived_labels = list(data["sequences"].keys())
    elif expected_kind == "nmcr" and isinstance(data.get("multi_cells"), dict):
        derived_labels = list(data["multi_cells"].keys())

    sections = [primary]
    if "labels" in data:
        sections.append({"magic": "LBAL", "labels": data["labels"]})
    elif derived_labels is not None:
        sections.append({"magic": "LBAL", "labels": derived_labels})
    if "extended" in data:
        sections.append({"magic": "TXEU", "extended": data["extended"]})
    return sections


def build_binary_from_toml(path: Path, expected_kind: str, reference_map: dict[str, int] | None = None) -> bytes:
    data = load_toml(path)
    if reference_map is not None:
        if expected_kind == "nmcr":
            data = resolve_named_multicell_references(data, reference_map)
        else:
            data = resolve_named_animation_references(data, reference_map)
    fmt = data.get("format")
    if fmt != expected_kind:
        raise RuntimeError(f"{path}: expected format {expected_kind!r}, got {fmt!r}")
    payload = data.get("data")
    if isinstance(payload, str):
        return bytes.fromhex(payload)

    sections = data.get("sections")
    if sections is None:
        sections = authoring_sections(data, expected_kind)
    if not isinstance(sections, list):
        raise RuntimeError(f"{path}: invalid sections")
    magic = data.get("magic", G2D_MAGIC_BY_KIND[expected_kind])
    if not isinstance(magic, str) or len(magic.encode("ascii")) != 4:
        raise RuntimeError(f"{path}: invalid magic")
    byte_order = int(data.get("byte_order", 0xFEFF))
    version = int(data.get("version", 0x0100))
    header_size = int(data.get("header_size", 0x10))
    if header_size != 0x10:
        raise RuntimeError(f"{path}: unsupported header_size {header_size}")

    section_payload = b"".join(pack_section(section) for section in sections)
    file_size = header_size + len(section_payload)
    return (
        magic.encode("ascii")
        + byte_order.to_bytes(2, "little")
        + version.to_bytes(2, "little")
        + file_size.to_bytes(4, "little")
        + header_size.to_bytes(2, "little")
        + len(sections).to_bytes(2, "little")
        + section_payload
    )


def pack_tiled_4bpp(indices: bytes, width: int, height: int) -> bytes:
    if width % 8 != 0 or height % 8 != 0:
        raise RuntimeError("NCGR PNG dimensions must be multiples of 8")
    tiles_wide = width // 8
    tiles_high = height // 8
    out = bytearray()
    for tile_y in range(tiles_high):
        for tile_x in range(tiles_wide):
            for y in range(8):
                row = (tile_y * 8 + y) * width + tile_x * 8
                for x in range(0, 8, 2):
                    left = indices[row + x] & 0xF
                    right = indices[row + x + 1] & 0xF
                    out.append((right << 4) | left)
    return bytes(out)


def pack_bitmap_4bpp(indices: bytes, width: int, height: int) -> bytes:
    if width % 8 != 0 or height % 8 != 0:
        raise RuntimeError("NCGR PNG dimensions must be multiples of 8")
    tiles_wide = width // 8
    tiles_high = height // 8
    out = bytearray(tiles_wide * tiles_high * 32)
    for tile_y in range(tiles_high):
        for tile_x in range(tiles_wide):
            base = 4 * tile_x + 32 * tile_y * tiles_wide
            for y in range(8):
                row = (tile_y * 8 + y) * width + tile_x * 8
                for x in range(0, 8, 2):
                    left = indices[row + x] & 0xF
                    right = indices[row + x + 1] & 0xF
                    out[base + (x // 2) + 4 * y * tiles_wide] = (right << 4) | left
    return bytes(out)


def build_ncgr_from_png(path: Path, sopc_height: int = 32) -> bytes:
    image, indices = png_indices(path)
    width, height = image.size
    if (width, height) == (96, 96):
        return NCGR_TILED_HEADER + pack_tiled_4bpp(indices, width, height)
    if (width, height) == (256, 128):
        if sopc_height not in (16, 32):
            raise RuntimeError(f"{path}: unsupported SOPC height {sopc_height}")
        tail = bytearray(NCGR_BITMAP_TAIL)
        tail[14] = sopc_height
        return NCGR_BITMAP_HEADER + pack_bitmap_4bpp(indices, width, height) + bytes(tail)
    raise RuntimeError(f"{path}: unsupported NCGR PNG dimensions {width}x{height}")


def build_nns_payload(entry: dict, manifest: Path, manifest_data: dict | None = None) -> bytes:
    for dependency in entry.get("depends_on", []):
        if not (manifest.parent / dependency).exists():
            raise RuntimeError(f"{manifest}: {entry['raw']} depends on missing {dependency}")
    raw = manifest.parent / entry["raw"]
    if not raw.exists():
        raise RuntimeError(f"{manifest}: raw file {raw.name!r} does not exist")
    kind = entry.get("kind")
    if kind == "ncgr" and raw.suffix == ".png":
        sopc_height = entry.get("sopc_height")
        if sopc_height is None and manifest_data is not None:
            sopc_height = manifest_data.get("sopc_height")
        return build_ncgr_from_png(raw, int(sopc_height or 32))
    if kind == "nclr" and raw.suffix == ".pal":
        return build_nclr_from_pal(raw, entry.get("high_bits"))
    if kind in TOML_BINARY_KINDS and raw.suffix == ".toml":
        reference_map = None
        dependencies = entry.get("depends_on", [])
        if kind == "nanr" and dependencies:
            reference_map = named_index_map(manifest.parent / dependencies[0], "cells")
        elif kind == "nmcr" and dependencies:
            reference_map = named_index_map(manifest.parent / dependencies[0], "sequences")
        elif kind == "nmar" and len(dependencies) >= 2:
            reference_map = named_index_map(manifest.parent / dependencies[1], "multi_cells")
        return build_binary_from_toml(raw, kind, reference_map)
    return raw.read_bytes()


def stage_nns_entry(entry: dict, manifest: Path, manifest_data: dict, output: Path) -> None:
    payload = build_nns_payload(entry, manifest, manifest_data)
    if entry.get("lz11", False):
        output.write_bytes(lz11_compress(payload))
    else:
        output.write_bytes(payload)


def graphics_order_entries(source_root: Path) -> list[dict]:
    order_path = source_root / ORDER_FILE
    order = load_toml(order_path)
    entries = order.get("entries")
    if not isinstance(entries, list):
        raise RuntimeError(f"{order_path}: missing entries array")
    for index, entry in enumerate(entries):
        folder = entry.get("folder")
        kind = entry.get("type")
        if not isinstance(folder, str):
            raise RuntimeError(f"{order_path}: entries[{index}].folder must be a string")
        if kind not in ("nns", "pwan"):
            raise RuntimeError(f"{order_path}: entries[{index}].type must be \"nns\" or \"pwan\"")
    return entries


def copy_nns_archive_entries(source_root: Path, battle_vfs: Path) -> int:
    if (source_root / ORDER_FILE).exists():
        count = 0
        for item in graphics_order_entries(source_root):
            manifest = source_root / item["folder"] / "nns.toml"
            manifest_data = load_toml(manifest)
            for entry in manifest_data.get("entries", []):
                stage_nns_entry(entry, manifest, manifest_data, battle_vfs / str(count))
                count += 1
        return count

    source = source_root
    count = 0
    for file in sorted(source.iterdir(), key=lambda p: int(p.name) if p.name.isdecimal() else -1):
        if not file.is_file() or not file.name.isdecimal():
            continue
        shutil.copy2(file, battle_vfs / file.name)
        count += 1
    return count


def build_pwan_assets(source_root: Path, pwan_vfs: Path) -> list[dict]:
    sources = []
    config_entries = []
    asset_index = 0
    ensure_dir(pwan_vfs)
    expected_outputs = {"config.bin", "sources.json"}

    for entry in graphics_order_entries(source_root):
        if entry["type"] != "pwan":
            continue
        folder_name = entry["folder"]
        pokemon_dir = source_root / folder_name
        manifest_data = load_toml(pokemon_dir / "nns.toml")
        species = int(manifest_data["species"])
        config_path = pokemon_dir / "config.toml"
        if not config_path.exists():
            raise RuntimeError(f"{config_path}: missing PWAN config")

        config = load_toml(config_path)
        fmt = config.get("format")
        if fmt != "pwan":
            raise RuntimeError(f"{config_path}: unsupported format {fmt!r}")

        side_outputs = {}
        for side in ("front", "back"):
            side_config = config.get(side)
            if not isinstance(side_config, dict) or "source" not in side_config:
                raise RuntimeError(f"{config_path}: missing [{side}].source")
            src = pokemon_dir / side_config["source"]
            dst = pwan_vfs / f"{asset_index:03}_{side}.pwan"
            expected_outputs.add(dst.name)
            scale = float(side_config.get("scale", 1.0))
            offset_x = int(side_config.get("offset_x", 0))
            offset_y = int(side_config.get("offset_y", 0))
            source_mtime = max(src.stat().st_mtime, config_path.stat().st_mtime)
            if not dst.exists() or dst.stat().st_mtime < source_mtime:
                stats = compile_pwan(src, dst, scale=scale, offset_x=offset_x, offset_y=offset_y)
            else:
                stats = {
                    "frames": None,
                    "timeline": None,
                    "ticks": None,
                    "bytes": dst.stat().st_size,
                }
            side_outputs[side] = dst.name
            sources.append({
                "species": species,
                "folder": folder_name,
                "side": side,
                "source": str(src.relative_to(source_root)),
                "pwan": dst.name,
                "frames": stats["frames"],
                "timeline": stats["timeline"],
                "ticks": stats["ticks"],
                "bytes": stats["bytes"],
                "scale": scale,
                "offset_x": offset_x,
                "offset_y": offset_y,
            })

        config_entries.append(struct.pack("<HHHH", species, 0x0003, asset_index, asset_index))
        asset_index += 1

    for file in pwan_vfs.iterdir():
        if file.is_file() and file.name not in expected_outputs:
            file.unlink()

    header = struct.pack(
        "<IHHHHI",
        CONFIG_MAGIC,
        CONFIG_VERSION,
        len(config_entries),
        MAX_TIMELINE_ENTRIES,
        0,
        CONFIG_HEADER_SIZE,
    )
    pwan_vfs.mkdir(parents=True, exist_ok=True)
    (pwan_vfs / "config.bin").write_bytes(header + b"".join(config_entries))
    (pwan_vfs / "sources.json").write_text(json.dumps(sources, indent=2) + "\n")
    return sources


def main() -> int:
    parser = argparse.ArgumentParser(description="Build pokegra battle graphics into VFS outputs.")
    parser.add_argument("--source", type=Path, required=True)
    parser.add_argument("--battle-vfs", type=Path, required=True)
    parser.add_argument("--pwan-vfs", type=Path, required=True)
    parser.add_argument("--arc-text", required=True)
    parser.add_argument("--stamp", type=Path, required=True)
    args = parser.parse_args()

    clean_dir(args.battle_vfs)
    ensure_dir(args.pwan_vfs)
    nns_count = copy_nns_archive_entries(args.source, args.battle_vfs)
    (args.battle_vfs / ".arc").write_text(args.arc_text)
    sources = build_pwan_assets(args.source, args.pwan_vfs)
    args.stamp.parent.mkdir(parents=True, exist_ok=True)
    args.stamp.write_text(
        f"nns_entries={nns_count}\n"
        f"pwan_assets={len(sources)}\n"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
