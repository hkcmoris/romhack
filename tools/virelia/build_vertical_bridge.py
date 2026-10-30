#!/usr/bin/env python3
"""Add north-south wooden bridge metatiles to the shared outdoor bank.

Rotate the existing bridge's overlay at native pixel size, preserving its
palette and water underlay. No existing graphics, metatiles or maps are repacked.
"""
from pathlib import Path
import struct

from localize_type_icons import read_png, write_png

ROOT = Path(__file__).resolve().parents[2]
TARGET = ROOT / 'data/tilesets/secondary/virelia_outdoor'
BRIDGE_IDS = (0x363, 0x364)
SOURCE_IDS = (0x2B3, 0x2B4)


def words(path):
    raw = path.read_bytes()
    return list(struct.unpack(f'<{len(raw) // 2}H', raw))


def build():
    pixels, depth, chunks = read_png(TARGET / 'tiles.png')
    primary, _, _ = read_png(ROOT / 'data/tilesets/primary/general/tiles.png')
    metas = words(TARGET / 'metatiles.bin')
    attrs = words(TARGET / 'metatile_attributes.bin')

    def tile_pixels(value):
        index = value & 1023
        image = primary if index < 512 else pixels
        index %= 512
        return tuple(image[index // 16 * 8 + (7 - y if value & 0x800 else y)]
                          [index % 16 * 8 + (7 - x if value & 0x400 else x)]
                     for y in range(8) for x in range(8))

    # A 16x32 overlay becomes a 32x16 deck, with the railings on its sides.
    overlay = [[0] * 16 for _ in range(32)]
    slots = set()
    for half, source in enumerate(SOURCE_IDS):
        for part, value in enumerate(metas[(source - 512) * 8 + 4:(source - 511) * 8]):
            slots.add(value >> 12)
            tile = tile_pixels(value)
            for y in range(8):
                for x in range(8):
                    overlay[half * 16 + part // 2 * 8 + y][part % 2 * 8 + x] = tile[y * 8 + x]
    if len(slots) != 1:
        raise ValueError('Bridge overlay must use one existing palette')
    slot = slots.pop()
    rotated = [[overlay[31 - x][y] for x in range(32)] for y in range(16)]
    existing = {tile_pixels(index): index for index in range(512, 1008)}
    # Allocate only after referenced graphics, before native door animation RAM.
    next_tile = max(value & 1023 for value in metas) + 1

    def add_tile(data):
        nonlocal next_tile
        if data in existing:
            return existing[data]
        if next_tile >= 1008:
            raise ValueError('Vertical bridge overlaps native door animation graphics')
        index = next_tile
        next_tile += 1
        if any(tile_pixels(index)):
            raise ValueError(f'Unreferenced graphics at {index:#x} must not be overwritten')
        for y in range(8):
            for x in range(8):
                pixels[(index - 512) // 16 * 8 + y][(index - 512) % 16 * 8 + x] = data[y * 8 + x]
        existing[data] = index
        return index

    required = BRIDGE_IDS[-1] - 511
    metas.extend([0] * max(0, required * 8 - len(metas)))
    attrs.extend([0] * max(0, required - len(attrs)))
    underlay = metas[(SOURCE_IDS[0] - 512) * 8:(SOURCE_IDS[0] - 512) * 8 + 4]
    for half, destination in enumerate(BRIDGE_IDS):
        result = list(underlay)
        for part in range(4):
            data = tuple(rotated[part // 2 * 8 + y][half * 16 + part % 2 * 8 + x]
                         for y in range(8) for x in range(8))
            result.append((slot << 12) | add_tile(data))
        metas[(destination - 512) * 8:(destination - 511) * 8] = result
        # MB_NORMAL and BG3/BG2 layers: actors draw above the wooden deck.
        attrs[destination - 512] = 0x1000
    write_png(TARGET / 'tiles.png', pixels, depth, chunks)
    (TARGET / 'metatiles.bin').write_bytes(struct.pack(f'<{len(metas)}H', *metas))
    (TARGET / 'metatile_attributes.bin').write_bytes(struct.pack(f'<{len(attrs)}H', *attrs))
    print('PASS: north-south bridge tiles 0x363/0x364; existing graphics and palettes preserved')


if __name__ == '__main__':
    build()
