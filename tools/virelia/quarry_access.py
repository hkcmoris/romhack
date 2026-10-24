#!/usr/bin/env python3
"""Repair quarry warp footprints without regenerating hand-edited terrain."""
import json
from pathlib import Path
import struct

ROOT = Path(__file__).resolve().parents[2]
LEVELS = {'MAP_VIRELIA_SLATEFORD_QUARRY_ENTRANCE': 0,
          'MAP_VIRELIA_SLATEFORD_QUARRY_DEPTHS': 1,
          'MAP_VIRELIA_SLATEFORD_QUARRY_GALLERY': 2}
DOWN_SHAFT = 0x227
UPPER_LADDER_TOP = 0x20e
UPPER_LADDER_BASE = 0x216
WALL_EXIT = 0x251


def apply_access(grid, entry):
    """Match art/collision to destination height, keeping existing warp IDs."""
    level = LEVELS[entry['id']]
    warp_points = {(warp['x'], warp['y']) for warp in entry['warp_events']}
    for warp in entry['warp_events']:
        x, y, destination = warp['x'], warp['y'], warp['dest_map']
        if destination == 'MAP_VIRELIA_SLATEFORD_CITY':
            # A deep-south cave mouth faces the arriving player north and the
            # native non-animated-door task walks them onto the clear floor.
            grid[y][x - 1:x + 2] = [0x0650, 0x3251, 0x0652]
            grid[y - 1][x] = 0x3201
            if (3, 17) not in warp_points and grid[17][3] & 1023 == 0x207:
                grid[17][3] = 0x3201
        elif destination in LEVELS:
            if LEVELS[destination] < level:
                grid[y][x] = 0x3000 | UPPER_LADDER_BASE
                grid[y - 1][x] = 0x3400 | UPPER_LADDER_TOP
            else:
                grid[y][x] = 0x3000 | DOWN_SHAFT
                cap = grid[y - 1][x]
                if cap & 1023 in (0x20e, 0x20f):
                    grid[y - 1][x] = 0x0619 if cap & 0xc00 else 0x3201


def main():
    layouts = json.loads((ROOT / 'data/layouts/layouts.json').read_text())['layouts']
    for name in ('QuarryEntrance', 'QuarryDepths', 'QuarryGallery'):
        entry = json.loads((ROOT / f'data/maps/Virelia_Slateford_{name}/map.json').read_text())
        layout = next(item for item in layouts if item['id'] == entry['layout'])
        path = ROOT / layout['blockdata_filepath']
        cells = list(struct.unpack(f'<{layout["width"] * layout["height"]}H', path.read_bytes()))
        grid = [cells[y * layout['width']:(y + 1) * layout['width']] for y in range(layout['height'])]
        apply_access(grid, entry)
        cells = [cell for row in grid for cell in row]
        path.write_bytes(struct.pack(f'<{len(cells)}H', *cells))
    path = ROOT / 'data/tilesets/secondary/virelia_quarry/metatile_attributes.bin'
    attrs = bytearray(path.read_bytes())
    struct.pack_into('<H', attrs, (WALL_EXIT - 512) * 2, 0x106e)
    path.write_bytes(attrs)
    print('Repaired wall entrance and directed quarry ladders; preserved hand-edited terrain.')


if __name__ == '__main__':
    main()
