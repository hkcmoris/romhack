#!/usr/bin/env python3
"""Pack the Aseprite bench into the shared outdoor bank without changing palettes."""
from pathlib import Path
import struct
try:
    from .localize_type_icons import read_png, write_png
except ImportError:
    from localize_type_icons import read_png, write_png

ROOT = Path(__file__).resolve().parents[2]
BANK = ROOT / 'data/tilesets/secondary/virelia_outdoor'
BENCH_IDS = (0x365, 0x366)
BENCH_PALETTE_SLOT = 10
BENCH_ART = ROOT / 'art/maplebrook/bench.png'


def bench_pixels():
    image, _, chunks = read_png(BENCH_ART)
    if len(image) != 16 or any(len(row) != 32 for row in image):
        raise ValueError('Bench export must be a 32x16 indexed PNG')
    source = dict(chunks)
    colors = list(zip(*[iter(source[b'PLTE'])] * 3))
    alpha = source.get(b'tRNS', b'')
    native = [tuple(map(int, line.split())) for line in
              (BANK / f'palettes/{BENCH_PALETTE_SLOT:02}.pal').read_text().splitlines()[3:19]]
    remap = {}
    for index in {pixel for row in image for pixel in row}:
        opacity = alpha[index] if index < len(alpha) else 255
        if opacity == 0:
            remap[index] = 0
        elif opacity == 255 and colors[index] in native[1:]:
            remap[index] = native.index(colors[index], 1)
        else:
            raise ValueError(f'Bench color {index} is outside the native opaque palette')
    return [[remap[pixel] for pixel in row] for row in image]


def build():
    pixels, depth, chunks = read_png(BANK / 'tiles.png')
    raw = (BANK / 'metatiles.bin').read_bytes()
    metas = list(struct.unpack(f'<{len(raw) // 2}H', raw))
    raw = (BANK / 'metatile_attributes.bin').read_bytes()
    attrs = list(struct.unpack(f'<{len(raw) // 2}H', raw))
    primary = (ROOT / 'data/tilesets/primary/general/metatiles.bin').read_bytes()
    grass = list(struct.unpack_from('<4H', primary, 16))

    def tile(index):
        n = index - 512
        return tuple(pixels[n // 16 * 8 + y][n % 16 * 8 + x] for y in range(8) for x in range(8))

    # Reclaim only graphics owned exclusively by the previous bench. This keeps
    # repeated art edits from consuming more tile RAM and preserves shared art.
    shared, old_bench = set(), set()
    for offset in range(0, len(metas), 8):
        target = old_bench if offset // 8 + 512 in BENCH_IDS else shared
        target.update(value & 1023 for value in metas[offset:offset + 8]
                      if 512 <= value & 1023 < 1008)

    def put_tile(index, data):
        n = index - 512
        for y in range(8):
            for x in range(8):
                pixels[n // 16 * 8 + y][n % 16 * 8 + x] = data[y * 8 + x]

    for index in old_bench - shared:
        put_tile(index, (0,) * 64)
    lookup = {tile(index): index for index in sorted(shared)}
    available = iter(index for index in range(512, 1008)
                     if index not in shared and not any(tile(index)))
    metas.extend([0] * max(0, (BENCH_IDS[-1] - 511) * 8 - len(metas)))
    attrs.extend([0] * max(0, BENCH_IDS[-1] - 511 - len(attrs)))
    image = bench_pixels()
    for half, metatile in enumerate(BENCH_IDS):
        values = list(grass)
        for part in range(4):
            data = tuple(image[part // 2 * 8 + y][half * 16 + part % 2 * 8 + x]
                         for y in range(8) for x in range(8))
            if data not in lookup:
                index = next(available, None)
                if index is None:
                    raise ValueError('No safe graphics space remains for the bench')
                lookup[data] = index
                put_tile(index, data)
            values.append(BENCH_PALETTE_SLOT << 12 | lookup[data])
        metas[(metatile - 512) * 8:(metatile - 511) * 8] = values
        attrs[metatile - 512] = 0x1000
    write_png(BANK / 'tiles.png', pixels, depth, chunks)
    (BANK / 'metatiles.bin').write_bytes(struct.pack(f'<{len(metas)}H', *metas))
    (BANK / 'metatile_attributes.bin').write_bytes(struct.pack(f'<{len(attrs)}H', *attrs))
    print('PASS: Maplebrook benches use existing timber colors and preserve existing art')


if __name__ == '__main__':
    build()
