#!/usr/bin/env python3
"""Verify shared outdoor graphics preserve native pixels and tile behavior.

Uses the repository's indexed PNG decoder; no ROM toolchain or Pillow required.
"""
from functools import lru_cache
from pathlib import Path
import json
import struct
import sys
from virelia.build_outdoor_tileset import REMAP, SOURCES, SYMBOL, SERVICE_IDS
from virelia.localize_type_icons import read_png
from virelia.build_maplebrook_props import BENCH_IDS, BENCH_PALETTE_SLOT, bench_pixels

ROOT = Path(__file__).resolve().parents[1]

# Porymap-authored shared metatiles have no single secondary-bank donor. Keep
# their intended native General composition explicit so they receive the same
# pixel, flip, palette, and behavior checks as the repacked donor metatiles.
AUTHORED_METATILES = {
    0x200: (0x0000, (0x2004, 0x2404, 0x2014, 0x2414,
                     0x2000, 0x2000, 0x20CD, 0x2000)),
}
VERTICAL_BRIDGE_IDS = (0x363, 0x364)


def validate_bench(tile, packed, attrs):
    values = packed[(tile - 512) * 8:(tile - 511) * 8]
    grass = words('data/tilesets/primary/general/metatiles.bin')[8:12]
    require(values[:4] == grass and attrs[tile - 512] == 0x1000,
            'Bench must retain native grass underlay and draw above ground')
    image = bench_pixels()
    colors = palette('secondary/virelia_outdoor', BENCH_PALETTE_SLOT)
    half = BENCH_IDS.index(tile)
    for part, value in enumerate(values[4:]):
        expected = tuple(colors[index] if index else None
                         for y in range(8) for x in range(8)
                         for index in (image[part // 2 * 8 + y][half * 16 + part % 2 * 8 + x],))
        require(rendered_tile('virelia_outdoor', value) == expected, 'Bench timber pixels or palette changed')
    require(all(value & 1023 < 1008 and value >> 12 < 13 for value in values),
            'Bench overlaps reserved graphics/palettes')


def validate_vertical_bridge(tile, packed, attrs):
    source = [[None] * 16 for _ in range(32)]
    for half, original in enumerate((0x2B3, 0x2B4)):
        for part, value in enumerate(packed[(original - 512) * 8 + 4:(original - 511) * 8]):
            pixels = rendered_tile('virelia_outdoor', value)
            for y in range(8):
                for x in range(8):
                    source[half * 16 + part // 2 * 8 + y][part % 2 * 8 + x] = pixels[y * 8 + x]
    half = VERTICAL_BRIDGE_IDS.index(tile)
    values = packed[(tile - 512) * 8:(tile - 511) * 8]
    require(attrs[tile - 512] == 0x1000, 'Vertical bridge must draw below actors with normal walking behavior')
    require(values[:4] == packed[(0x2B3 - 512) * 8:(0x2B3 - 512) * 8 + 4],
            'Vertical bridge water underlay changed')
    for part, value in enumerate(values[4:]):
        actual = rendered_tile('virelia_outdoor', value)
        expected = tuple(source[31 - (half * 16 + part % 2 * 8 + x)][part // 2 * 8 + y]
                         for y in range(8) for x in range(8))
        require(actual == expected, f'Vertical bridge {tile:#x} does not match the rotated wooden deck')
    require(all(value & 1023 < 1008 and value >> 12 < 13 for value in values),
            'Vertical bridge exceeds native graphics or palette limits')


def require(condition, message):
    if not condition:
        raise ValueError(message)


def words(path):
    raw = (ROOT / path).read_bytes()
    return struct.unpack(f'<{len(raw) // 2}H', raw)


@lru_cache(None)
def image(source):
    return read_png(ROOT / f'data/tilesets/{source}/tiles.png')[0]


@lru_cache(None)
def palette(source, slot):
    return [tuple(int(c) // 8 for c in line.split()) for line in
            (ROOT / f'data/tilesets/{source}/palettes/{slot:02}.pal').read_text().splitlines()[3:19]]


@lru_cache(None)
def rendered_tile(source, value):
    tile, slot = value & 0x3FF, value >> 12
    pixels = image('primary/general' if tile < 512 else 'secondary/' + source)
    tile %= 512
    pal_source = 'primary/general' if slot < 6 else 'secondary/' + source
    if source == 'whisperwood' and slot >= 6 and slot != 6:
        pal_source = 'secondary/fortree'
    colors = palette(pal_source, slot)
    result = []
    for y in range(8):
        for x in range(8):
            dx, dy = (7 - x if value & 0x400 else x), (7 - y if value & 0x800 else y)
            row = tile // 16 * 8 + dy
            index = pixels[row][tile % 16 * 8 + dx] if row < len(pixels) else 0
            result.append(colors[index] if index else None)
    return tuple(result)


def validate():
    layouts = json.loads((ROOT / 'data/layouts/layouts.json').read_text())['layouts']
    packed = words('data/tilesets/secondary/virelia_outdoor/metatiles.bin')
    attrs = words('data/tilesets/secondary/virelia_outdoor/metatile_attributes.bin')
    require(len(packed) == len(attrs) * 8 <= 512 * 8, 'Invalid shared metatile tables')
    require(attrs[0x318 - 512] == 0x1000,
            'Guardian pawprints must use BG3/BG2 ground layers below the player')
    require(len(image('secondary/virelia_outdoor')) == 256, 'Invalid shared graphics dimensions')
    provenance = {}
    for art_source, remap in REMAP.items():
        for old, tile in remap.items():
            require(tile not in provenance, f'Ambiguous packed metatile {tile:#x}')
            provenance[tile] = (art_source, old)
    checked = set()
    for name, source in SOURCES.items():
        layout = next(x for x in layouts if x['name'] == name + '_Layout')
        require(layout['primary_tileset'] == 'gTileset_General' and layout['secondary_tileset'] == SYMBOL,
                f'{name}: connected outdoors must share both tilesets')
        ids = {v & 0x3FF for v in words(layout['blockdata_filepath']) if v & 0x3FF >= 512}
        if source == 'whisperwood':
            ids.update(range(0x318, 0x336))
        if name == 'Virelia_SlatefordCity':
            ids.update(SERVICE_IDS)
        ids.update(VERTICAL_BRIDGE_IDS)
        ids.update(BENCH_IDS)
        for tile in ids:
            require(tile - 512 < len(attrs), f'{name}: packed metatile {tile:#x} is out of bounds')
            if tile in BENCH_IDS:
                validate_bench(tile, packed, attrs)
                checked.add(('maplebrook_bench', tile))
                continue
            elif tile in VERTICAL_BRIDGE_IDS:
                validate_vertical_bridge(tile, packed, attrs)
                checked.add(('vertical_bridge', tile))
                continue
            elif tile in AUTHORED_METATILES:
                art_source, old = 'virelia_outdoor', tile
                original_attr, original = AUTHORED_METATILES[tile]
            else:
                # Every connected map can use the entire shared bank, regardless
                # of which source tileset the map used before packing.
                require(tile in provenance, f'{name}: unknown packed metatile {tile:#x}')
                art_source, old = provenance[tile]
                original = words(f'data/tilesets/secondary/{art_source}/metatiles.bin')[(old - 512) * 8:(old - 511) * 8]
                original_attr = words(f'data/tilesets/secondary/{art_source}/metatile_attributes.bin')[old - 512]
            require(attrs[tile - 512] == original_attr,
                    f'{name}: changed behavior/layer attributes for {old:#x}')
            for layer in range(8):
                before = original[layer]
                after = packed[(tile - 512) * 8 + layer]
                require(after & 0x3FF < 1008, 'Graphics overlap native door animation tiles')
                require(after >> 12 < 13, 'Graphics use a reserved UI palette')
                require(rendered_tile(art_source, before) == rendered_tile('virelia_outdoor', after),
                        f'{name}: changed pixels in {old:#x} -> {tile:#x}, layer tile {layer}')
            checked.add((art_source, old))
    for slot in (6, 8, 10):
        require(palette('secondary/petalburg', slot) == palette('secondary/virelia_outdoor', slot),
                'Maplebrook door animation palettes changed')
    print(f'PASS: {len(checked)} outdoor metatiles preserve every pixel, flip, behavior and layer')
    print('PASS: shared map banks, hardware tile/palette limits and native door palettes')


if __name__ == '__main__':
    try:
        validate()
    except (ValueError, KeyError, OSError, AssertionError) as error:
        print(f'FAIL: {error}', file=sys.stderr)
        sys.exit(1)
