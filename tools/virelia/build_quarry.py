#!/usr/bin/env python3
"""Build Slateford's dedicated Emerald quarry bank and event props.

Uses stock Rusturf rock contours with a limestone palette, plus original native
16px industrial tiles. Pillow is only required for regeneration and previews.
Run after build_first_gym_maps.py/build_missing_tools.py to reapply the art pass.
"""
import json
from pathlib import Path
import random
import shutil
import struct

from PIL import Image, ImageDraw

from build_first_gym_maps import cave_grid
from quarry_contours import CLIFF_TILES, cliff_at, quadrants
from quarry_access import apply_access

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
            (56, 184, 224), (152, 232, 240), (232, 248, 240), (104, 40, 64),
            (176, 48, 80), (224, 80, 104), (248, 144, 152), (248, 208, 192)]


def write_words(path, values):
    path.write_bytes(struct.pack(f'<{len(values)}H', *values))


def write_json(path, value):
    path.write_text(json.dumps(value, ensure_ascii=False, indent=2) + '\n', encoding='utf-8', newline='\n')


def pal(path, colors):
    path.write_bytes(('JASC-PAL\r\n0100\r\n16\r\n' + ''.join(
        ' '.join(map(str, color)) + '\r\n' for color in colors)).encode('ascii'))


def floor(seed):
    image = Image.new('P', (16, 16), 3)
    draw = ImageDraw.Draw(image)
    rng = random.Random(seed)
    for _ in range(9):
        x, y = rng.randrange(15), rng.randrange(16)
        draw.line((x, y, x + 1, y), fill=rng.choice((2, 4, 4)))
    return image


def machine(kind, on=False):
    image = Image.new('P', (16, 16))
    d = ImageDraw.Draw(image)
    d.ellipse((1, 12, 15, 15), fill=1)
    d.rectangle((2, 3, 13, 13), fill=1)
    d.rectangle((3, 4, 12, 11), fill=3)
    d.line((3, 3, 12, 3), fill=5)
    d.line((3, 4, 3, 10), fill=4)
    if kind == 'pump':
        d.rectangle((5, 1, 10, 4), fill=2)
        d.line((5, 1, 10, 1), fill=4)
        d.rectangle((5, 5, 10, 8), fill=1)
        d.line((6, 6, 9, 6), fill=13 if on else 11)
        d.rectangle((0, 8, 2, 11), fill=4)
        d.rectangle((13, 8, 15, 11), fill=4)
    else:
        d.rectangle((4, 5, 11, 9), fill=2)
        d.line((6, 5, 6, 8, 9, 8, 9, 6), fill=10)
    d.point((5, 11), fill=13 if on else 14)
    d.point((10, 11), fill=15 if on else 2)
    return image


def props():
    images = {}
    for kind in ('pump', 'feed'):
        for on in (False, True):
            images[f'{kind}_{"on" if on else "off"}'] = machine(kind, on), 7, False
    for i, name in enumerate(('floor_a', 'floor_b', 'floor_c')):
        images[name] = floor(i + 31), 6, True
    for name, vertical in (('rail_h', False), ('rail_v', True)):
        im = Image.new('P', (16, 16))
        d = ImageDraw.Draw(im)
        for y in (1, 7, 13):
            d.rectangle((2, y, 13, y + 2), fill=6)
            d.line((3, y, 12, y), fill=8)
        for x in (4, 11):
            d.rectangle((x, 0, x + 1, 15), fill=2)
            d.line((x, 0, x, 15), fill=4)
            for y in (2, 8, 14):
                d.point((x, y), fill=5)
        images[name] = im if vertical else im.transpose(Image.Transpose.ROTATE_90), 7, False
    for name, flipx, flipy in (('rail_ne', False, False), ('rail_nw', True, False),
                               ('rail_se', False, True), ('rail_sw', True, True)):
        im = Image.new('P', (16, 16))
        d = ImageDraw.Draw(im)
        d.line((3, 2, 13, 12), fill=7, width=3)
        d.line((4, 0, 4, 4, 11, 11, 15, 11), fill=2, width=2)
        d.line((11, 0, 11, 2, 13, 4, 15, 4), fill=2, width=2)
        d.line((4, 0, 4, 4, 11, 11, 15, 11), fill=4)
        d.line((11, 0, 11, 2, 13, 4, 15, 4), fill=4)
        if flipx:
            im = im.transpose(Image.Transpose.FLIP_LEFT_RIGHT)
        if flipy:
            im = im.transpose(Image.Transpose.FLIP_TOP_BOTTOM)
        images[name] = im, 7, False
    for name in ('board', 'crate', 'timber', 'lamp', 'water', 'drain', 'cable_h', 'cable_v'):
        im = Image.new('P', (16, 16))
        d = ImageDraw.Draw(im)
        if name == 'board':
            d.rectangle((3, 9, 4, 15), fill=6)
            d.rectangle((11, 9, 12, 15), fill=6)
            d.rectangle((1, 1, 14, 10), fill=6)
            d.rectangle((2, 2, 13, 8), fill=9)
            for y in (3, 5, 7):
                d.line((4, y, 11, y), fill=7)
        elif name == 'crate':
            d.rectangle((1, 2, 14, 14), fill=6)
            d.rectangle((2, 3, 13, 12), fill=8)
            for y in (3, 7, 11):
                d.line((2, y, 13, y), fill=9)
            d.line((2, 12, 13, 3), fill=6, width=2)
            for x in (2, 12):
                d.line((x, 3, x, 12), fill=7)
        elif name == 'timber':
            d.rectangle((3, 0, 6, 15), fill=6)
            d.rectangle((4, 0, 5, 15), fill=8)
            d.rectangle((2, 3, 7, 5), fill=2)
            d.rectangle((2, 11, 7, 13), fill=2)
            d.point((3, 4), fill=5)
            d.point((3, 12), fill=5)
        elif name == 'lamp':
            d.rectangle((7, 0, 9, 15), fill=6)
            d.rectangle((4, 3, 12, 10), fill=1)
            d.rectangle((5, 4, 11, 9), fill=10)
            d.rectangle((6, 5, 10, 8), fill=15)
            d.line((8, 4, 8, 9), fill=8)
        elif name in ('water', 'drain'):
            d.rounded_rectangle((0, 3, 15, 14), radius=3, fill=11)
            d.line((3, 5, 7, 5), fill=12)
            d.line((9, 10, 13, 10), fill=13)
            if name == 'drain':
                d.rectangle((2, 4, 13, 12), fill=2)
                for x in range(4, 13, 3):
                    d.line((x, 5, x, 11), fill=4)
        else:
            d.line((0, 9, 15, 9), fill=1, width=3)
            d.line((0, 8, 15, 8), fill=10)
            for x in (3, 12):
                d.line((x, 7, x, 10), fill=3)
            if name == 'cable_v':
                im = im.transpose(Image.Transpose.ROTATE_90)
        images[name] = im, 7, False
    for color in ('blue', 'red'):
        for kind in ('seam', 'ore'):
            im = Image.new('P', (16, 16))
            d = ImageDraw.Draw(im)
            a, b, c, highlight = (6, 7, 8, 9) if color == 'blue' else (11, 12, 13, 14)
            gems = [(4, 6, 3, 7), (10, 3, 4, 9)] if kind == 'ore' else [(4, 5, 2, 4), (10, 10, 2, 3)]
            for x, y, w, h in gems:
                d.polygon([(x - 1, y + 1), (x + w // 2, y - 2), (x + w + 1, y + 1),
                           (x + w, y + h), (x, y + h + 1)], fill=1)
                d.polygon([(x, y + 1), (x + w // 2, y - 1), (x + w, y + 1),
                           (x + w - 1, y + h - 1), (x, y + h)], fill=b)
                d.line((x, y + 1, x, y + h - 1), fill=highlight)
                d.line((x + 1, y + 1, x + 1, y + h - 2), fill=c)
                d.point((x + 1, y), fill=10 if color == 'blue' else 15)
            images[f'{color}_{kind}'] = im, 8, False
    return images


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

    put(0x201, floor(30), 6, True, behavior=8)
    for name, (im, slot, ground) in props().items():
        base = 0x219 if name.endswith('seam') else 0x201
        put(TILES[name], im, slot, ground, base, 8 if name.startswith('floor') else 0)
    # Crystal seams keep both native cliff layers behind their overlay.
    for i, edge in enumerate((0x219, 0x210, 0x212, 0x209)):
        base = 0x26c + i * 3
        put(base, flattened(edge), 6, True)
        for j, color in enumerate(('blue', 'red'), 1):
            index = base + j
            TILES[f'{color}_seam_{edge:x}'] = index
            put(index, props()[f'{color}_seam'][0], 8, base=base)
    # Make the scripted blockade an actual loaded mine cart, on matching rail.
    im = Image.new('P', (16, 16))
    d = ImageDraw.Draw(im)
    d.rectangle((2, 7, 13, 12), fill=1)
    d.rectangle((3, 7, 12, 10), fill=3)
    d.line((2, 6, 13, 6), fill=5)
    d.polygon([(3, 5), (4, 2), (7, 3), (10, 1), (12, 5)], fill=4)
    d.line((4, 2, 6, 2), fill=5)
    d.rectangle((3, 12, 5, 14), fill=1)
    d.rectangle((10, 12, 12, 14), fill=1)
    put(0x203, im, 7, base=TILES['rail_v'])
    # A ladder in a timber-lined shaft, readable at native game scale.
    im = Image.new('P', (16, 16), 1)
    d = ImageDraw.Draw(im)
    d.rectangle((0, 0, 15, 2), fill=6)
    for x in (3, 11):
        d.line((x, 1, x, 15), fill=9, width=2)
    for y in (4, 8, 12):
        d.line((4, y, 11, y), fill=10)
        d.line((4, y + 1, 11, y + 1), fill=7)
    put(0x214, im, 7, True, behavior=0x61)
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


def build_objects():
    folder = ROOT / 'graphics/object_events/pics/quarry'
    folder.mkdir(parents=True, exist_ok=True)
    colors = [tuple(map(int, line.split())) for line in
              (ROOT / 'graphics/object_events/palettes/moving_box.pal').read_text().splitlines()[3:19]]
    for name in ('wrench', 'hammer', 'pick', 'ore_crate', 'tool_hoard'):
        im = Image.new('P', (16, 16))
        im.putpalette([c for color in colors for c in color] + [0] * (768 - 48))
        d = ImageDraw.Draw(im)
        d.ellipse((2, 12, 14, 15), fill=6)
        if name in ('ore_crate', 'tool_hoard'):
            d.rectangle((1, 5, 14, 13), fill=6)
            d.rectangle((2, 6, 13, 12), fill=13)
            for y in (6, 10):
                d.line((2, y, 13, y), fill=12)
            d.line((4, 6, 4, 12), fill=6)
            d.line((11, 6, 11, 12), fill=6)
            for x, y in ((3, 4), (7, 3), (10, 4)):
                d.polygon([(x, y + 1), (x + 1, y - 1), (x + 3, y), (x + 2, y + 3)], fill=10)
                d.point((x + 1, y), fill=4)
            if name == 'ore_crate':
                d.rectangle((6, 7, 9, 10), fill=9)
                d.line((7, 8, 8, 8), fill=8)
            else:
                d.line((4, 4, 12, 10), fill=7, width=2)
                d.rectangle((2, 2, 6, 4), fill=9)
        else:
            d.line((4, 12, 11, 5), fill=6, width=4)
            d.line((4, 11, 10, 5), fill=12 if name != 'wrench' else 10, width=2)
            d.point((4, 11), fill=9)
            if name == 'wrench':
                d.polygon([(8, 5), (8, 2), (10, 1), (10, 4), (12, 4), (13, 1),
                           (14, 3), (13, 6), (10, 7)], fill=6)
                d.line((9, 3, 9, 5, 11, 6, 13, 4), fill=9)
            elif name == 'hammer':
                d.polygon([(6, 3), (9, 0), (14, 4), (11, 7)], fill=6)
                d.polygon([(7, 3), (9, 1), (13, 4), (11, 6)], fill=10)
                d.line((8, 2, 12, 5), fill=9)
            else:
                d.line((5, 2, 9, 2, 13, 5, 14, 8), fill=6, width=3)
                d.line((5, 1, 9, 1, 13, 4, 14, 7), fill=9)
        im.save(folder / f'{name}.png', bits=4, transparency=0)


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


def main():
    count = build_tiles()
    build_objects()
    build_maps()
    print(f'Built limestone quarry, {count}/512 BG tiles, 5 native event props.')


if __name__ == '__main__':
    main()
