#!/usr/bin/env python3
"""Verify shared outdoor graphics preserve native pixels and tile behavior.

Uses the repository's indexed PNG decoder; no ROM toolchain or Pillow required.
"""
from functools import lru_cache
from pathlib import Path
import json
import struct
import sys
from virelia.build_outdoor_tileset import REMAP, SOURCES, SYMBOL
from virelia.localize_type_icons import read_png

ROOT = Path(__file__).resolve().parents[1]


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
            'Arcanine pawprints must use BG3/BG2 ground layers below the player')
    require(len(image('secondary/virelia_outdoor')) == 256, 'Invalid shared graphics dimensions')
    checked = set()
    for name, source in SOURCES.items():
        layout = next(x for x in layouts if x['name'] == name + '_Layout')
        require(layout['primary_tileset'] == 'gTileset_General' and layout['secondary_tileset'] == SYMBOL,
                f'{name}: connected outdoors must share both tilesets')
        inverse = {target: old for old, target in REMAP[source].items()}
        ids = {v & 0x3FF for v in words(layout['blockdata_filepath']) if v & 0x3FF >= 512}
        if source == 'whisperwood':
            ids.update(range(0x318, 0x329))
        original = words(f'data/tilesets/secondary/{source}/metatiles.bin')
        original_attrs = words(f'data/tilesets/secondary/{source}/metatile_attributes.bin')
        for tile in ids:
            require(tile in inverse, f'{name}: unknown packed metatile {tile:#x}')
            old = inverse[tile]
            require(attrs[tile - 512] == original_attrs[old - 512],
                    f'{name}: changed behavior/layer attributes for {old:#x}')
            for layer in range(8):
                before = original[(old - 512) * 8 + layer]
                after = packed[(tile - 512) * 8 + layer]
                require(after & 0x3FF < 1008, 'Graphics overlap native door animation tiles')
                require(after >> 12 < 13, 'Graphics use a reserved UI palette')
                require(rendered_tile(source, before) == rendered_tile('virelia_outdoor', after),
                        f'{name}: changed pixels in {old:#x} -> {tile:#x}, layer tile {layer}')
            checked.add((source, old))
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
