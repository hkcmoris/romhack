#!/usr/bin/env python3
"""Build Slateford's dedicated Emerald quarry bank and event props.

Reuses Emerald rock, ship and Slateport parts alongside FRLG industrial art.
Pillow is only required for regeneration and previews.
Run after build_first_gym_maps.py/build_missing_tools.py to reapply the art pass.
"""
import json
from pathlib import Path
import shutil
import struct

from PIL import Image

from build_first_gym_maps import cave_grid
from quarry_contours import CLIFF_TILES, cliff_at, quadrants
from quarry_access import apply_access
from quarry_art import floor, props as quarry_props, cart, shaft, stamp
from quarry_objects import build_objects

ROOT = Path(__file__).resolve().parents[2]
TARGET = ROOT / 'data/tilesets/secondary/virelia_quarry'
STOCK = ROOT / 'data/tilesets/secondary/rusturf_tunnel'
NAMES = ('Entrance', 'Depths', 'Gallery')
ROOMS = (
    (24, 20, [(2, 3, 8, 5), (3, 11, 8, 7), (7, 7, 4, 7), (10, 8, 9, 3),
              (16, 4, 6, 7), (17, 10, 4, 7), (10, 15, 10, 3)]),
    (24, 22, [(2, 14, 8, 6), (5, 10, 14, 7), (16, 13, 6, 6), (2, 7, 6, 5),
              (7, 8, 6, 4), (10, 5, 3, 9), (8, 2, 9, 4)]),
    (30, 24, [(2, 16, 8, 6), (5, 8, 5, 12), (5, 4, 17, 6), (18, 6, 10, 6),
              (23, 9, 5, 13), (8, 17, 18, 5), (18, 10, 4, 8)]),
)
BEVELS = (
    ((2, 3), (9, 3), (2, 7), (16, 4), (21, 4), (21, 10), (3, 11), (20, 16)),
    ((2, 14), (9, 19), (21, 18), (21, 13), (2, 7), (7, 7), (8, 2), (16, 2), (16, 5)),
    ((5, 4), (21, 4), (27, 6), (27, 21), (8, 21), (2, 16), (2, 21)),
)
# IDs remain local to this bank; 201/203/214 retain scripted floor/cart/ladder.
TILES = {'floor_a': 0x253, 'floor_b': 0x254, 'floor_c': 0x255,
         'rail_h': 0x256, 'rail_v': 0x257, 'rail_ne': 0x258,
         'rail_nw': 0x259, 'rail_se': 0x25a, 'rail_sw': 0x25b,
         'pump_off': 0x25c, 'pump_on': 0x25d, 'feed_off': 0x25e,
         'feed_on': 0x25f, 'board': 0x260, 'crate': 0x261,
         'blue_seam': 0x262, 'red_seam': 0x263, 'blue_ore': 0x264,
         'red_ore': 0x265, 'timber': 0x266, 'lamp': 0x267,
         'water': 0x268, 'drain': 0x269, 'cable_h': 0x26a, 'cable_v': 0x26b}
LIMESTONE = [(24, 24, 32), (216, 200, 184), (192, 176, 160), (176, 160, 144),
             (160, 144, 128), (144, 128, 112), (128, 112, 104), (104, 88, 80),
             (216, 200, 184), (192, 176, 160), (176, 160, 152), (152, 136, 128),
             (128, 112, 104), (104, 88, 88), (80, 64, 64), (48, 40, 48)]
MACHINERY = [(0, 0, 0), (48, 48, 56), (72, 72, 80), (104, 112, 120),
             (152, 160, 160), (208, 216, 200), (80, 56, 48), (112, 80, 56),
             (152, 112, 72), (192, 152, 96), (232, 200, 120), (64, 104, 112),
             (88, 152, 152), (144, 208, 192), (192, 80, 72), (248, 224, 160)]
MINERALS = [(0, 0, 0), (48, 40, 48), (72, 56, 64), (112, 88, 88),
            (152, 128, 128), (192, 168, 160), (24, 72, 112), (32, 120, 176),
            (56, 168, 208), (120, 208, 224), (208, 232, 232), (104, 40, 64),
            (176, 48, 80), (208, 72, 96), (232, 128, 136), (248, 192, 176)]


def write_words(path, values):
    path.write_bytes(struct.pack(f'<{len(values)}H', *values))


def write_json(path, value):
    path.write_text(json.dumps(value, ensure_ascii=False, indent=2) + '\n', encoding='utf-8', newline='\n')


def pal(path, colors):
    path.write_bytes(('JASC-PAL\r\n0100\r\n16\r\n' + ''.join(
        ' '.join(map(str, color)) + '\r\n' for color in colors)).encode('ascii'))


def props():
    return quarry_props(MACHINERY, MINERALS)


def build_tiles():
    TARGET.mkdir(parents=True, exist_ok=True)
    (TARGET / 'palettes').mkdir(exist_ok=True)
    for path in (STOCK / 'palettes').glob('*.pal'):
        shutil.copyfile(path, TARGET / 'palettes' / path.name)
    for slot, colors in ((6, LIMESTONE), (7, MACHINERY), (8, MINERALS)):
        pal(TARGET / f'palettes/{slot:02}.pal', colors)
    source = Image.open(STOCK / 'tiles.png')
    tiles = Image.new('P', (128, 256))
    tiles.putpalette(source.getpalette())
    tiles.paste(source, (0, 0))
    meta_raw = (STOCK / 'metatiles.bin').read_bytes()
    meta = list(struct.unpack(f'<{len(meta_raw) // 2}H', meta_raw))
    # Rusturf's southwest inset has six pixels transparent in BOTH layers.
    # Give its flipped rock overlays the same opaque backing as its other
    # quarters before seams/compound contours copy them. Keep stock intact.
    for quadrant in (1, 2):
        meta[(0x21c - 512) * 8 + quadrant] = 0x621e
    attr_raw = (STOCK / 'metatile_attributes.bin').read_bytes()
    attrs = list(struct.unpack(f'<{len(attr_raw) // 2}H', attr_raw))
    next_tile = (source.height + 7) // 8 * 16 + 512

    def flattened(index):
        im = Image.new('P', (16, 16), 3)
        for n, value in enumerate(meta[(index - 512) * 8:(index - 511) * 8]):
            tile = value & 1023
            if tile < 512:
                continue
            tile -= 512
            for y in range(8):
                for x in range(8):
                    sx, sy = (7 - x if value & 1024 else x), (7 - y if value & 2048 else y)
                    p = source.getpixel((tile % 16 * 8 + sx, tile // 16 * 8 + sy))
                    if p:
                        im.putpixel((n % 2 * 8 + x, n % 4 // 2 * 8 + y), p)
        return im

    def put(index, im, slot, ground=False, base=0x201, behavior=0):
        nonlocal next_tile
        while len(attrs) <= index - 512:
            attrs.append(0)
            meta.extend([0] * 8)
        values = [0] * 8 if ground else meta[(base - 512) * 8:(base - 511) * 8].copy()
        if ground:
            values[4:] = [0x1000] * 4
        for y in range(2):
            for x in range(2):
                piece = im.crop((x * 8, y * 8, x * 8 + 8, y * 8 + 8))
                n = next_tile - 512
                assert n < 512, 'Quarry art exceeds the secondary BG bank'
                tiles.paste(piece, (n % 16 * 8, n // 16 * 8))
                values[(0 if ground else 4) + y * 2 + x] = next_tile | slot << 12
                next_tile += 1
        meta[(index - 512) * 8:(index - 511) * 8] = values
        # BG3/BG2 keeps rails/props under actors. Collision lives in map cells.
        attrs[index - 512] = 0x1000 | behavior

    art = props()
    put(0x201, floor(30), 6, True, behavior=8)
    for name, (im, slot, ground) in art.items():
        base = 0x219 if name.endswith('seam') else 0x201
        put(TILES[name], im, slot, ground, base, 8 if name.startswith('floor') else 0)
    # Crystal seams keep both native cliff layers behind their overlay.
    for i, edge in enumerate((0x219, 0x210, 0x212, 0x209)):
        base = 0x26c + i * 3
        put(base, flattened(edge), 6, True)
        for j, color in enumerate(('blue', 'red'), 1):
            index = base + j
            TILES[f'{color}_seam_{edge:x}'] = index
            put(index, art[f'{color}_seam'][0], 8, base=base)
    # A stock cargo-box silhouette rebuilt as a shaded steel ore hopper.
    loaded_cart = art['rail_v'][0].copy()
    stamp(loaded_cart, cart(MACHINERY))
    put(0x203, loaded_cart, 7)
    # Native cave ladder perspective and its recessed shaft shadow.
    put(0x214, shaft(MACHINERY), 7, True, behavior=0x61)
    # Reuse native 8px rock quarters for compound corners. No extra graphics
    # tiles or palettes are needed, including for opposite exposed ridge faces.
    for mask, index in CLIFF_TILES.items():
        if index < 0x280:
            continue
        while len(attrs) <= index - 512:
            attrs.append(0)
            meta.extend([0] * 8)
        for q, original in enumerate(quadrants(mask)):
            for layer in (0, 4):
                meta[(index - 512) * 8 + layer + q] = meta[(original - 512) * 8 + layer + q]
        attrs[index - 512] = 0x1000
    # The city exit uses the stock cave entrance behavior.
    attrs[0x251 - 512] = 0x106e  # MB_DEEP_SOUTH_WARP: north-facing cave arrival.
    tiles.save(TARGET / 'tiles.png', bits=4)
    write_words(TARGET / 'metatiles.bin', meta)
    write_words(TARGET / 'metatile_attributes.bin', attrs)
    return next_tile - 512


def build_maps():
    registry = ROOT / 'data/layouts/layouts.json'
    doc = json.loads(registry.read_text())
    for suffix, (width, height, rooms), bevels in zip(NAMES, ROOMS, BEVELS):
        name = 'Virelia_Slateford_Quarry' + suffix
        grid = cave_grid(width, height, rooms)
        entry_path = ROOT / f'data/maps/{name}/map.json'
        entry = json.loads(entry_path.read_text())
        protected = {(e['x'], e['y']) for key in ('object_events', 'warp_events', 'bg_events') for e in entry[key]}
        for x, y in bevels:
            if (x, y) not in protected:
                grid[y][x] = 0x0611
        if suffix == 'Depths':
            # Include the evidence barrier and its narrow throat in the same
            # contour pass, rather than stamping straight walls over corners.
            for x in range(8, 17):
                grid[6][x] = 0x0619
            grid[7][10], grid[7][12] = 0x0612, 0x0610
        # Eight neighbors distinguish caps, insets and narrow rock ridges.
        mask = [[not cell & 0xc00 for cell in row] for row in grid]
        for y in range(height):
            for x in range(width):
                if not mask[y][x]:
                    grid[y][x] = 0x400 | cliff_at(mask, x, y)
        # Calm, sparse floor variation. Keep all original walls/collisions.
        for y, row in enumerate(grid):
            for x, value in enumerate(row):
                if value == 0x3201:
                    row[x] = 0x3000 | (0x201 if (x * 7 + y * 11) % 4 == 0 else TILES['floor_a'] + (x + y * 3) % 3)

        def place(x, y, tile, solid=False):
            grid[y][x] = (0x3400 if solid else 0x3000) | TILES.get(tile, tile)

        def seams(color, offset):
            for y in range(1, height - 1):
                for x in range(1, width - 1):
                    edge = grid[y][x] & 1023
                    if edge in (0x219, 0x210, 0x212, 0x209) and (x * 3 + y * 5) % 11 == offset and (x, y) not in protected:
                        place(x, y, f'{color}_seam_{edge:x}', True)

        def rail(points):
            for i, (x, y) in enumerate(points):
                if (x, y) in protected:
                    continue
                neighbors = set()
                for j in (i - 1, i + 1):
                    if 0 <= j < len(points):
                        nx, ny = points[j]
                        neighbors.add('e' if nx > x else 'w' if nx < x else 's' if ny > y else 'n')
                tile = 'rail_' + ''.join(d for d in 'ns ew'.replace(' ', '') if d in neighbors)
                tile = tile if tile in TILES else 'rail_h' if neighbors <= {'e', 'w'} else 'rail_v'
                place(x, y, tile)

        if suffix == 'Entrance':
            rail([(x, 16) for x in range(4, 19)] + [(18, y) for y in range(15, 8, -1)] +
                 [(x, 9) for x in range(17, 8, -1)] + [(9, y) for y in range(8, 3, -1)])
            rocks = [(3, 5), (8, 6), (9, 15), (17, 8), (19, 14)]
            for x, y in rocks:
                if (x, y) not in protected:
                    place(x, y, 0x204, True)
            place(17, 5, 'crate', True)
            for x, y in ((6, 3), (20, 6)):
                place(x, y, 'lamp', True)
            seams('blue', 1)
            place(5, 3, 'timber', True)
        elif suffix == 'Depths':
            rail([(11, y) for y in range(16, 2, -1)])
            for x, y in ((6, 15), (8, 10), (18, 17), (15, 4)):
                place(x, y, 0x204, True)
            place(20, 14, 'pump_off', True)
            place(4, 9, 'feed_off', True)
            place(6, 17, 'board', True)
            place(11, 6, 0x203, True)
            place(20, 15, 'water', True)
            place(20, 16, 'drain', True)
            for x in range(5, 8):
                place(x, 9, 'cable_h')
            seams('red', 4)
            for x, y in ((9, 3), (16, 2)):
                place(x, y, 'lamp', True)
        else:
            rail([(x, 20) for x in range(4, 27)])
            for x, y in ((3, 17), (8, 12), (10, 5), (16, 8), (20, 7), (26, 14), (12, 19), (20, 19)):
                place(x, y, 0x204, True)
            for x, y in ((6, 4), (12, 4), (19, 6), (27, 10)):
                place(x, y, 'crate', True)
            seams('blue', 1)
            seams('red', 5)
            for x, y in ((11, 9), (16, 9), (21, 12)):
                place(x, y, 'blue_ore', True)
            place(4, 20, 'board', True)
            for x, y in ((6, 7), (26, 18)):
                place(x, y, 'lamp', True)
        apply_access(grid, entry)
        for obj in entry['object_events']:
            script = obj['script'].rsplit('_', 1)[-1]
            gfx = {'Wrench': 'WRENCH', 'Hammer': 'HAMMER', 'Pick': 'PICK',
                   'Evidence': 'ORE_CRATE', 'Crate': 'ORE_CRATE', 'Hoard': 'TOOL_HOARD'}.get(script)
            if gfx:
                obj['graphics_id'] = 'OBJ_EVENT_GFX_QUARRY_' + gfx
        write_json(entry_path, entry)
        layout = next(item for item in doc['layouts'] if item['id'] == entry['layout'])
        layout['secondary_tileset'] = 'gTileset_VireliaQuarry'
        write_words(ROOT / layout['blockdata_filepath'], [v for row in grid for v in row])
    write_json(registry, doc)


def main(tiles_only=False):
    count = build_tiles()
    if not tiles_only:
        build_objects()
        build_maps()
    print(f'Built limestone quarry art, {count}/512 BG tiles.')


if __name__ == '__main__':
    import argparse
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--tiles-only', action='store_true',
                        help='repack graphics without regenerating maps or event sprites')
    main(parser.parse_args().tiles_only)
