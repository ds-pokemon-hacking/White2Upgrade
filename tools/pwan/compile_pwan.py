#!/usr/bin/env python3
import argparse
import hashlib
import struct
from pathlib import Path

from PIL import Image, ImageSequence


MAGIC = b"PWAN"
VERSION = 1
WIDTH = 96
HEIGHT = 96
FRAME_BYTES = 0x1200
PALETTE_COLORS = 16
MAX_TIMELINE_ENTRIES = 128

SEGMENTS = (
    (0, 0, 64, 64),
    (64, 0, 32, 64),
    (0, 64, 64, 32),
    (64, 64, 32, 32),
)


def bgr555(color):
    r, g, b = color[:3]
    return (r >> 3) | ((g >> 3) << 5) | ((b >> 3) << 10)


def iter_gif_frames(path):
    image = Image.open(path)
    canvas = Image.new("RGBA", image.size, (0, 0, 0, 0))
    last = None
    last_disposal = 0
    last_bbox = None

    for frame in ImageSequence.Iterator(image):
        if last_disposal == 2 and last_bbox:
            canvas.paste((0, 0, 0, 0), last_bbox)
        elif last_disposal == 3 and last is not None:
            canvas = last.copy()

        disposal = getattr(frame, "disposal_method", 0)
        if disposal == 3:
            last = canvas.copy()

        rgba = frame.convert("RGBA")
        bbox = frame.getbbox()
        canvas.alpha_composite(rgba)

        duration_ms = frame.info.get("duration", image.info.get("duration", 100))
        yield canvas.copy(), max(1, round(duration_ms * 60 / 1000))

        last_disposal = disposal
        last_bbox = bbox


def normalize_frame(frame, scale=1.0, offset_x=0, offset_y=0):
    frame = frame.convert("RGBA")
    if scale <= 0:
        raise ValueError(f"scale must be positive, got {scale}")
    if scale != 1.0:
        width = max(1, round(frame.width * scale))
        height = max(1, round(frame.height * scale))
        frame = frame.resize((width, height), Image.Resampling.LANCZOS)
    frame.thumbnail((WIDTH, HEIGHT), Image.Resampling.LANCZOS)
    out = Image.new("RGBA", (WIDTH, HEIGHT), (0, 0, 0, 0))
    x = (WIDTH - frame.width) // 2 + int(offset_x)
    y = HEIGHT - frame.height + int(offset_y)
    out.alpha_composite(frame, (x, y))
    return out


def make_palette(frames):
    sheet = Image.new("RGBA", (WIDTH, HEIGHT * len(frames)), (0, 0, 0, 0))
    for i, frame in enumerate(frames):
        sheet.alpha_composite(frame, (0, i * HEIGHT))

    opaque = Image.new("RGBA", sheet.size, (0, 0, 0, 0))
    opaque.alpha_composite(sheet)
    pal = opaque.convert("P", palette=Image.Palette.ADAPTIVE, colors=PALETTE_COLORS - 1)
    raw = pal.getpalette()[: (PALETTE_COLORS - 1) * 3]
    colors = [(0, 0, 0)]
    colors.extend(tuple(raw[i : i + 3]) for i in range(0, len(raw), 3))
    colors = colors[:PALETTE_COLORS]
    while len(colors) < PALETTE_COLORS:
        colors.append((0, 0, 0))
    return colors


def nearest_palette_index(pixel, palette):
    r, g, b, a = pixel
    if a < 128:
        return 0
    best = 1
    best_dist = 1 << 30
    for i, (pr, pg, pb) in enumerate(palette[1:], 1):
        dr = r - pr
        dg = g - pg
        db = b - pb
        dist = dr * dr + dg * dg + db * db
        if dist < best_dist:
            best_dist = dist
            best = i
    return best


def tile_region(frame, palette, x0, y0, width, height):
    pixels = frame.load()
    out = bytearray()
    for tile_y in range(0, height, 8):
        for tile_x in range(0, width, 8):
            for y in range(8):
                for x in range(0, 8, 2):
                    lo = nearest_palette_index(pixels[x0 + tile_x + x, y0 + tile_y + y], palette)
                    hi = nearest_palette_index(pixels[x0 + tile_x + x + 1, y0 + tile_y + y], palette)
                    out.append(lo | (hi << 4))
    return bytes(out)


def compile_frame(frame, palette):
    chunks = [tile_region(frame, palette, *segment) for segment in SEGMENTS]
    data = b"".join(chunks)
    if len(data) != FRAME_BYTES:
        raise ValueError(f"compiled frame is {len(data)} bytes, expected {FRAME_BYTES}")
    return data


def compile_pwan(src, dst, scale=1.0, offset_x=0, offset_y=0):
    normalized = []
    delays = []
    for frame, ticks in iter_gif_frames(src):
        normalized.append(normalize_frame(frame, scale, offset_x, offset_y))
        delays.append(ticks)

    if not normalized:
        raise ValueError("GIF has no frames")

    palette = make_palette(normalized)
    compiled = [compile_frame(frame, palette) for frame in normalized]
    unique = []
    frame_map = {}
    timeline = []

    for data, ticks in zip(compiled, delays):
        digest = hashlib.sha1(data).digest()
        if digest not in frame_map:
            frame_map[digest] = len(unique)
            unique.append(data)
        timeline.append((frame_map[digest], ticks))

    if len(timeline) > MAX_TIMELINE_ENTRIES:
        timeline = resample_timeline(timeline, MAX_TIMELINE_ENTRIES)

    palette_offset = 0x30
    timeline_offset = palette_offset + PALETTE_COLORS * 2
    frame_offset = timeline_offset + len(timeline) * 4
    total_ticks = sum(ticks for _, ticks in timeline)

    header = struct.pack(
        "<4sHHHHHHIIIIII",
        MAGIC,
        VERSION,
        WIDTH,
        HEIGHT,
        4,
        len(unique),
        len(timeline),
        total_ticks,
        FRAME_BYTES,
        PALETTE_COLORS,
        palette_offset,
        timeline_offset,
        frame_offset,
    )

    dst.parent.mkdir(parents=True, exist_ok=True)
    with dst.open("wb") as f:
        f.write(header)
        f.write(bytes(palette_offset - len(header)))
        f.write(struct.pack("<" + "H" * PALETTE_COLORS, *(bgr555(c) for c in palette)))
        for frame_index, ticks in timeline:
            f.write(struct.pack("<HH", frame_index, ticks))
        for data in unique:
            f.write(data)

    return {
        "frames": len(unique),
        "timeline": len(timeline),
        "ticks": total_ticks,
        "bytes": dst.stat().st_size,
    }


def resample_timeline(timeline, max_entries):
    if len(timeline) <= max_entries:
        return timeline

    total_ticks = sum(ticks for _, ticks in timeline)
    result = []
    for out_index in range(max_entries):
        start_tick = (total_ticks * out_index) // max_entries
        end_tick = (total_ticks * (out_index + 1)) // max_entries
        if end_tick <= start_tick:
            end_tick = start_tick + 1

        elapsed = 0
        selected_frame = timeline[-1][0]
        for frame_index, ticks in timeline:
            next_elapsed = elapsed + ticks
            if next_elapsed > start_tick:
                selected_frame = frame_index
                break
            elapsed = next_elapsed

        segment_ticks = max(1, end_tick - start_tick)
        if result and result[-1][0] == selected_frame:
            result[-1] = (selected_frame, min(0xffff, result[-1][1] + segment_ticks))
        else:
            result.append((selected_frame, min(0xffff, segment_ticks)))

    return result


def main():
    parser = argparse.ArgumentParser(description="Compile a GIF into a PWAN summary animation asset.")
    parser.add_argument("src", type=Path)
    parser.add_argument("dst", type=Path)
    parser.add_argument("--scale", type=float, default=1.0)
    parser.add_argument("--offset-x", type=int, default=0)
    parser.add_argument("--offset-y", type=int, default=0)
    args = parser.parse_args()
    stats = compile_pwan(args.src, args.dst, args.scale, args.offset_x, args.offset_y)
    print(
        f"wrote {args.dst} "
        f"({stats['frames']} unique frames, {stats['timeline']} timeline entries, "
        f"{stats['ticks']} ticks, {stats['bytes']} bytes)"
    )


if __name__ == "__main__":
    main()
