#!/usr/bin/env python3
"""Build the lab's original indexed briefcase art and deduplicated GBA tilemap."""
import argparse
from pathlib import Path
import struct
import zlib

ROOT = Path(__file__).resolve().parents[2]
PALETTE = [
    (16, 24, 32), (24, 48, 40), (40, 72, 56), (64, 104, 80),
    (96, 136, 104), (144, 168, 136), (192, 208, 176), (232, 232, 208),
    (104, 80, 48), (160, 128, 72), (216, 184, 112), (48, 64, 72),
    (88, 112, 120), (152, 176, 176), (208, 224, 216), (248, 248, 232),
]


def png(pixels):
    width, height = len(pixels[0]), len(pixels)
    def chunk(kind, data):
        return struct.pack('>I', len(data)) + kind + data + struct.pack('>I', zlib.crc32(kind + data))
    raw = b''.join(b'\0' + bytes((row[x] << 4) | row[x + 1] for x in range(0, width, 2)) for row in pixels)
    return (b'\x89PNG\r\n\x1a\n'
            + chunk(b'IHDR', struct.pack('>IIBBBBB', width, height, 4, 3, 0, 0, 0))
            + chunk(b'PLTE', bytes(c for color in PALETTE for c in color))
            + chunk(b'IDAT', zlib.compress(raw, 9)) + chunk(b'IEND', b''))


def build():
    pixels = [[1] * 240 for _ in range(160)]
    def rect(x, y, w, h, color):
        for yy in range(y, y + h):
            pixels[yy][x:x + w] = [color] * w
    def ellipse(cx, cy, rx, ry, color):
        for y in range(cy - ry, cy + ry + 1):
            for x in range(cx - rx, cx + rx + 1):
                if (x - cx) ** 2 * ry ** 2 + (y - cy) ** 2 * rx ** 2 <= rx ** 2 * ry ** 2:
                    pixels[y][x] = color
    # Dark lab worktop with quiet horizontal grain; the top is reserved for text.
    rect(0, 32, 240, 88, 2)
    for y in (35, 67, 99, 119):
        rect(0, y, 240, 1, 3)
    rect(14, 39, 216, 80, 0)
    # Brushed metal case, raised lid, green lining and a brass centre plate.
    rect(10, 34, 220, 80, 11)
    rect(11, 35, 218, 77, 13)
    rect(13, 36, 214, 74, 14)
    rect(15, 38, 210, 69, 12)
    rect(17, 40, 206, 23, 1)
    rect(19, 42, 202, 18, 3)
    rect(21, 43, 198, 1, 4)
    rect(95, 44, 50, 14, 8)
    rect(96, 44, 48, 12, 10)
    rect(98, 46, 44, 8, 9)
    # A small engraved leaf, drawn as pixels rather than baked text.
    for i in range(9):
        rect(115 + i // 2, 53 - i, 2, 1, 7)
        if i in (2, 4, 6):
            rect(112 + i // 2, 53 - i, 4, 1, 10)
    rect(17, 65, 206, 42, 1)
    rect(18, 66, 204, 1, 4)
    for x in (52, 120, 188):
        ellipse(x, 87, 26, 19, 11)
        ellipse(x, 86, 25, 18, 5)
        ellipse(x, 86, 23, 16, 0)
        ellipse(x, 89, 19, 11, 2)
        rect(x - 10, 106, 20, 1, 13)
    for x in (45, 185):
        rect(x - 7, 61, 14, 5, 11)
        rect(x - 6, 61, 12, 2, 15)
    for x in (72, 168):
        rect(x - 5, 106, 10, 9, 8)
        rect(x - 4, 106, 8, 7, 10)
        rect(x - 2, 108, 4, 2, 15)
    rect(102, 111, 36, 7, 11)
    rect(105, 113, 30, 3, 0)
    for x, y in ((14, 38), (224, 38), (14, 109), (224, 109)):
        rect(x, y, 2, 2, 15)
    # BG3's screen block starts at 0x3000, so graphics must fit below it.
    tiles, lookup, tilemap = [], {}, [0] * 1024
    for ty in range(20):
        for tx in range(30):
            tile = tuple(pixels[ty * 8 + y][tx * 8 + x] for y in range(8) for x in range(8))
            if tile not in lookup:
                lookup[tile] = len(tiles)
                tiles.append(tile)
            tilemap[ty * 32 + tx] = lookup[tile]
    assert len(tiles) * 32 < 0x3000, 'Briefcase graphics overlap BG3 tilemap'
    atlas = [[0] * 128 for _ in range(((len(tiles) + 15) // 16) * 8)]
    for i, tile in enumerate(tiles):
        for y in range(8):
            atlas[i // 16 * 8 + y][i % 16 * 8:i % 16 * 8 + 8] = tile[y * 8:y * 8 + 8]
    return png(atlas), struct.pack('<1024H', *tilemap), pixels, len(tiles)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--check', action='store_true')
    parser.add_argument('--preview', type=Path)
    args = parser.parse_args()
    atlas, tilemap, pixels, count = build()
    outputs = {'graphics/starter_choose/virelia_case.png': atlas,
               'graphics/starter_choose/virelia_case.bin': tilemap}
    for relative, data in outputs.items():
        path = ROOT / relative
        if args.check:
            assert path.read_bytes() == data, f'{relative}: regenerate briefcase assets'
        else:
            path.write_bytes(data)
    if args.preview:
        args.preview.write_bytes(png(pixels))
    print(f'PASS: briefcase art, {count} unique tiles, 16 colors, separate BG/window memory')


if __name__ == '__main__':
    main()
