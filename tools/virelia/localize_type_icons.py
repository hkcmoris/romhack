#!/usr/bin/env python3
"""Render Czech type badges from the game's native narrow font, without dependencies.

The existing indexed palettes and badge borders are preserved. The script is
idempotent and uses supplemental Czech glyphs from src/data/fonts/czech.inc.
"""

from collections import Counter
from pathlib import Path
import re
import struct
import zlib


ROOT = Path(__file__).resolve().parents[2]
LABELS = {
    "none": "ŽÁDNÝ",
    "normal": "BĚŽNÝ",
    "fight": "BOJOVÝ",
    "flying": "LÉTAJ.",
    "poison": "JEDOVÝ",
    "ground": "ZEMNÍ",
    "rock": "KAMENNÝ",
    "bug": "HMYZÍ",
    "ghost": "DUCHOVÝ",
    "steel": "OCELOVÝ",
    "fire": "OHNIVÝ",
    "water": "VODNÍ",
    "grass": "TRAVNÍ",
    "electric": "ELEKTR.",
    "psychic": "PSYCH.",
    "ice": "LEDOVÝ",
    "dragon": "DRAČÍ",
    "dark": "TEMNÝ",
    "fairy": "VÍLÍ",
    "stellar": "HVĚZDNÝ",
    "contest_cool": "STYL",
    "contest_beauty": "KRÁSA",
    "contest_cute": "ROZT.",
    "contest_smart": "CHYT.",
    "contest_tough": "DRSN.",
}


def paeth(a, b, c):
    p = a + b - c
    distances = (abs(p - a), abs(p - b), abs(p - c))
    return (a, b, c)[distances.index(min(distances))]


def read_png(path):
    content = path.read_bytes()
    assert content[:8] == b"\x89PNG\r\n\x1a\n", path
    chunks, compressed = [], bytearray()
    offset = 8
    while offset < len(content):
        size = struct.unpack_from(">I", content, offset)[0]
        kind = content[offset + 4:offset + 8]
        data = content[offset + 8:offset + 8 + size]
        chunks.append((kind, data))
        if kind == b"IDAT":
            compressed.extend(data)
        offset += size + 12
    header = chunks[0][1]
    width, height, depth, color, comp, filt, interlace = struct.unpack(">IIBBBBB", header)
    assert color == 3 and depth in (1, 2, 4, 8) and interlace == 0, path
    raw = zlib.decompress(compressed)
    stride = (width * depth + 7) // 8
    previous = [0] * stride
    pixels = []
    for y in range(height):
        filter_type = raw[y * (stride + 1)]
        row = list(raw[y * (stride + 1) + 1:(y + 1) * (stride + 1)])
        for x in range(stride):
            left = row[x - 1] if x else 0
            above = previous[x]
            upper_left = previous[x - 1] if x else 0
            predictors = (0, left, above, (left + above) // 2, paeth(left, above, upper_left))
            row[x] = (row[x] + predictors[filter_type]) & 0xFF
        pixels.append([
            (row[(x * depth) // 8] >> (8 - depth - (x * depth) % 8)) & ((1 << depth) - 1)
            for x in range(width)
        ])
        previous = row
    return pixels, depth, chunks


def write_png(path, pixels, depth, chunks):
    def chunk(kind, data):
        return struct.pack(">I", len(data)) + kind + data + struct.pack(">I", zlib.crc32(kind + data))

    raw = bytearray()
    for row in pixels:
        raw.append(0)  # PNG filter NONE.
        for start in range(0, len(row), 8 // depth):
            byte = 0
            for i, value in enumerate(row[start:start + 8 // depth]):
                byte |= value << (8 - depth - i * depth)
            raw.append(byte)
    output = bytearray(b"\x89PNG\r\n\x1a\n")
    for kind, data in chunks:
        if kind not in (b"IDAT", b"IEND"):
            output.extend(chunk(kind, data))
    output.extend(chunk(b"IDAT", zlib.compress(raw, 9)))
    output.extend(chunk(b"IEND", b""))
    path.write_bytes(output)


def main():
    font_pixels, _, _ = read_png(ROOT / "graphics/fonts/latin_small_narrower.png")
    fonts = (ROOT / "src/fonts.c").read_text(encoding="utf-8")
    width_data = re.search(r"gFontSmallNarrowerLatinGlyphWidths\[\] = \{(.*?)\n\};", fonts, re.S)
    widths = [int(value) for value in re.findall(r"\d+", width_data[1])]
    czech = (ROOT / "src/data/fonts/czech.inc").read_text(encoding="utf-8")
    glyph_data = re.search(r"gFontSmallNarrowerCzechGlyphs\[\]\[32\] = \{(.*?)\n\};", czech, re.S)
    glyphs = [int(value, 16) for value in re.findall(r"0x[0-9A-F]{4}", glyph_data[1])]
    charmap = (ROOT / "charmap.txt").read_text(encoding="utf-8").split("@ Hiragana")[0]
    codes = {char: int(code, 16) for char, code in re.findall(r"^'(.)'\s*=\s*([0-9A-F]{2})", charmap, re.M)}

    def glyph_pixel(code, x, y):
        if 0x3B <= code <= 0x4E:
            word = glyphs[(code - 0x3B) * 32 + (y // 8) * 16 + (x // 8) * 8 + y % 8]
            return (word >> (14 - (x % 8) * 2)) & 3
        return font_pixels[(code // 16) * 16 + y][(code % 16) * 16 + x]

    for name, label in LABELS.items():
        path = ROOT / "graphics/types" / (name + ".png")
        pixels, depth, chunks = read_png(path)
        assert len(pixels) == 16 and len(pixels[0]) == 32, path
        total_width = sum(widths[codes[char]] for char in label)
        assert total_width <= 30, (label, total_width)
        # Index 15 is white lettering, 14 its shadow; each badge retains its
        # original color and indexed palette. Leave the rounded border intact.
        background = Counter(
            pixels[y][x] for y in range(2, 13) for x in range(1, 31)
            if pixels[y][x] not in (0, 14, 15)
        ).most_common(1)[0][0]
        for y in range(2, 13):
            for x in range(1, 31):
                pixels[y][x] = background
        start_x = (32 - total_width) // 2
        for char in label:
            code = codes[char]
            # Some supplemental acute marks start one row above stock small
            # capitals. Normalize the mark without shifting the text baseline.
            high_mark = any(glyph_pixel(code, x, 0) in (1, 2) for x in range(widths[code]))
            for y in range(12):
                for x in range(widths[code]):
                    value = glyph_pixel(code, x, y)
                    if value in (1, 2):
                        dest_y = y + 1 + (1 if high_mark and y < 3 else 0)
                        assert 1 <= start_x + x <= 30 and 2 <= dest_y <= 12, (label, x, y)
                        pixels[dest_y][start_x + x] = 15 if value == 1 else 14
            start_x += widths[code]
        write_png(path, pixels, depth, chunks)
    print(f"Rendered {len(LABELS)} Czech badges with native glyphs and unchanged palettes.")


if __name__ == "__main__":
    main()
