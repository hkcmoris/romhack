#!/usr/bin/env python3
"""Rebuild the four Virelia v0.1 block layouts from stock Emerald metatiles.

No original layout is modified. Run from any directory with Python 3.
These layouts may also be edited in Porymap; rerunning replaces those edits.
"""
from pathlib import Path
import json
import struct

ROOT = Path(__file__).resolve().parents[2]
LAYOUTS = json.loads((ROOT / 'data/layouts/layouts.json').read_text(encoding='utf-8'))['layouts']
GRASS = 0x3001
TALL_GRASS = 0x300D
FLOWER = 0x3004
TREE = ((0x05D4, 0x05D5), (0x05E4, 0x05E5))


def stock(name):
    layout = next(x for x in LAYOUTS if x['name'] == name + '_Layout')
    raw = (ROOT / layout['blockdata_filepath']).read_bytes()
    values = struct.unpack('<' + 'H' * (len(raw) // 2), raw)
    return [list(values[y * layout['width']:(y + 1) * layout['width']]) for y in range(layout['height'])]


def rect(grid, x, y, w, h, block):
    for yy in range(y, y + h):
        for xx in range(x, x + w):
            grid[yy][xx] = block


def copy(grid, source, sx, sy, w, h, x, y):
    for yy in range(h):
        for xx in range(w):
            grid[y + yy][x + xx] = source[sy + yy][sx + xx]


def woods(w, h):
    return [[TREE[y % 2][x % 2] for x in range(w)] for y in range(h)]


def brush_tree(grid, x, y):
    for yy, row in enumerate(((0x05CE, 0x05CF), (0x05D4, 0x05D5), (0x05E4, 0x05E5))):
        for xx, value in enumerate(row):
            grid[y + yy][x + xx] = value


def road(cells, x, y, w, h):
    cells.update((xx, yy) for yy in range(y, y + h) for xx in range(x, x + w))


def paint_roads(grid, cells):
    # Stock dirt-path borders. All road cells are traversable at elevation 3.
    for x, y in cells:
        top = (x, y - 1) not in cells
        bottom = (x, y + 1) not in cells
        left = (x - 1, y) not in cells
        right = (x + 1, y) not in cells
        tile = (0x118 if left else 0x11A if right else 0x119) if top else (0x128 if left else 0x12A if right else 0x129) if bottom else 0x120 if left else 0x122 if right else 0x121
        grid[y][x] = tile | 0x3000


def save(name, grid, indoor=False):
    path = ROOT / 'data/layouts' / ('Virelia_' + name)
    path.mkdir(parents=True, exist_ok=True)
    values = [v for row in grid for v in row]
    (path / 'map.bin').write_bytes(struct.pack('<' + 'H' * len(values), *values))
    # Solid border prevents walking out of disconnected map edges.
    border = (0x0648,) * 4 if indoor else (0x05D4, 0x05D5, 0x05DC, 0x05DD)
    (path / 'border.bin').write_bytes(struct.pack('<4H', *border))


def main():
    town = woods(30, 28)
    rect(town, 2, 2, 26, 24, GRASS)
    town_road = set()
    road(town_road, 14, 0, 2, 24)
    road(town_road, 7, 21, 19, 2)
    road(town_road, 7, 10, 2, 13)
    road(town_road, 24, 19, 2, 4)
    road(town_road, 14, 11, 10, 2)
    road(town_road, 22, 10, 2, 3)
    paint_roads(town, town_road)
    littleroot = stock('LittlerootTown')
    copy(town, littleroot, 2, 4, 5, 5, 4, 5)      # Player home: closed placeholder door (7,9).
    copy(town, littleroot, 13, 4, 5, 5, 21, 5)    # Neighbour's cottage: door (22,9).
    copy(town, littleroot, 3, 12, 7, 5, 20, 14)   # Laboratory: door (24,18).
    # Patches of meadow and gardens surround the main paths.
    for x, y in [(3, 11), (4, 12), (10, 7), (11, 8), (18, 4), (19, 5), (26, 10), (27, 11), (19, 22), (20, 23), (3, 22), (4, 23)]:
        town[y][x] = FLOWER
    for y in (16, 18, 20):
        for x in range(3, 6):
            town[y][x] = FLOWER
    for x, y in [(10, 15), (17, 15), (9, 23), (23, 23)]:
        brush_tree(town, x, y)
    for x, y in [(10, 9), (23, 19), (12, 4), (17, 22)]:
        town[y][x] = 0x3403
    save('MaplebrookTown', town)

    lab = stock('LittlerootTown_ProfessorBirchsLab')
    # Keep stock furnishings and two exit carpets; reserve open display space.
    rect(lab, 4, 5, 5, 4, 0x3202)
    save('MaplebrookResearchLaboratory', lab, indoor=True)

    trail = woods(30, 42)
    paths = set()
    for r in [(14, 32, 2, 10), (8, 30, 8, 2), (8, 20, 2, 12), (8, 20, 12, 2), (18, 8, 2, 14), (14, 8, 6, 2), (14, 0, 2, 10), (4, 10, 12, 2), (4, 10, 2, 12), (4, 20, 6, 2), (8, 30, 18, 2), (24, 26, 2, 6)]:
        road(paths, *r)
    # A two-tile grassy verge gives each forest path breathing room.
    for x, y in paths:
        for yy in range(max(2, y - 2), min(40, y + 3)):
            for xx in range(max(2, x - 2), min(28, x + 3)):
                trail[yy][xx] = GRASS
    paint_roads(trail, paths)
    # Connection edges stay exactly two tiles wide on both neighbours.
    for y in (0, 1, 40, 41):
        for x in (14, 15):
            trail[y][x] = 0x3120 if x == 14 else 0x3122
    rect(trail, 16, 14, 6, 4, TALL_GRASS)
    rect(trail, 6, 25, 6, 4, TALL_GRASS)
    rect(trail, 2, 14, 4, 4, TALL_GRASS)
    rect(trail, 22, 26, 4, 3, TALL_GRASS)
    # The side clearing contains a single persistent Potion pickup.
    trail[27][25] = GRASS
    for x, y in [(12, 37), (16, 35), (11, 22), (6, 9), (20, 11), (21, 28), (26, 32)]:
        trail[y][x] = FLOWER
    trail[38][16] = 0x3403
    trail[3][12] = 0x3403
    save('Route01', trail)

    city = woods(30, 30)
    rect(city, 2, 2, 26, 26, GRASS)
    # Compact paved mining-town square, with a dirt quarry to the east.
    rect(city, 11, 4, 6, 23, 0x32BB)
    rect(city, 4, 11, 22, 4, 0x32BB)
    rect(city, 4, 21, 17, 3, 0x32BB)
    rect(city, 14, 27, 2, 3, 0x3121)
    rustboro = stock('RustboroCity')
    copy(city, rustboro, 24, 15, 6, 5, 19, 5)  # First Gym exterior: door (22,9).
    copy(city, rustboro, 31, 15, 5, 5, 4, 6)   # Residential placeholder: door (6,10).
    copy(city, rustboro, 31, 15, 5, 5, 4, 16)  # Mining office placeholder: door (6,20).
    rect(city, 21, 10, 3, 1, 0x32BB)
    quarry = set()
    road(quarry, 20, 17, 7, 8)
    paint_roads(city, quarry)
    for x, y in [(3, 3), (8, 3), (24, 25), (2, 24), (7, 24)]:
        brush_tree(city, x, y)
    for x, y in [(18, 9), (17, 25), (20, 16), (9, 20)]:
        city[y][x] = 0x3403
    for x, y in [(10, 4), (10, 6), (3, 14), (4, 15), (26, 12), (25, 13)]:
        city[y][x] = FLOWER
    save('SlatefordCity', city)
    # These older base layouts use stock IDs. The first-Gym builder finishes
    # the service plots and then packs all connected outdoors into one bank.
    for name, secondary in [('MaplebrookTown', 'Petalburg'), ('Route01', 'Rustboro'),
                            ('SlatefordCity', 'Rustboro')]:
        next(x for x in LAYOUTS if x['name'] == 'Virelia_' + name + '_Layout')['secondary_tileset'] = 'gTileset_' + secondary
    registry_path = ROOT / 'data/layouts/layouts.json'
    registry = json.loads(registry_path.read_text(encoding='utf-8'))
    registry['layouts'] = LAYOUTS
    registry_path.write_text(json.dumps(registry, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
    print('Wrote four Virelia layouts; original Emerald layouts unchanged.')
    print('Next run build_first_gym_maps.py to restore services and repack the shared outdoor tileset.')


if __name__ == '__main__':
    main()
