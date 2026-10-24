#!/usr/bin/env python3
"""Check quarry art banks, interactions and encounter-free haul routes."""
import json
from pathlib import Path
import struct

from virelia.localize_type_icons import read_png
from validate_virelia_missing_tools import validate as validate_routes
from virelia.quarry_access import LEVELS, DOWN_SHAFT, UPPER_LADDER_TOP, UPPER_LADDER_BASE, WALL_EXIT
from virelia.quarry_contours import CLIFF_TILES

ROOT = Path(__file__).resolve().parents[1]
BANK = ROOT / 'data/tilesets/secondary/virelia_quarry'


def words(path):
    raw = path.read_bytes()
    return struct.unpack(f'<{len(raw) // 2}H', raw)


def validate_cliff_coverage(pixels, meta):
    primary = read_png(ROOT / 'data/tilesets/primary/general/tiles.png')[0]

    def pixel(value, x, y):
        tile = value & 1023
        bank = primary if tile < 512 else pixels
        tile %= 512
        sx = 7 - x if value & 1024 else x
        sy = 7 - y if value & 2048 else y
        return bank[tile // 16 * 8 + sy][tile % 16 * 8 + sx]

    # Palette index zero is transparent on the GBA even when its RGB entry
    # looks like a dark rock color in an offline preview. Overlays may retain
    # zero pixels; opaque cliffs must cover them with the underlying layer.
    cliffs = set(CLIFF_TILES.values()) | {0x262, 0x263} | set(range(0x26c, 0x278))
    for index in sorted(cliffs):
        values = meta[(index - 512) * 8:(index - 511) * 8]
        assert len(values) == 8, f'Missing cliff metatile {index:#x}'
        for q in range(4):
            for y in range(8):
                for x in range(8):
                    assert pixel(values[q], x, y) or pixel(values[q + 4], x, y), (
                        f'Transparent cliff hole in {index:#x} at '
                        f'({q % 2 * 8 + x}, {q // 2 * 8 + y})')


def validate():
    pixels, depth, _ = read_png(BANK / 'tiles.png')
    assert depth == 4 and len(pixels) == 256 and len(pixels[0]) == 128
    meta, attrs = words(BANK / 'metatiles.bin'), words(BANK / 'metatile_attributes.bin')
    assert len(meta) == len(attrs) * 8 <= 512 * 8
    for value in meta:
        assert value >> 12 < 13, 'Art uses a reserved UI palette'
    validate_cliff_coverage(pixels, meta)
    assert attrs[0x214 - 512] & 255 == 0x61, 'Custom ladders lost native warp behavior'
    assert attrs[WALL_EXIT - 512] & 255 == 0x6e, 'Wall entrance needs native north-facing walk-out behavior'
    for index in (0x201, 0x253, 0x254, 0x255):
        assert attrs[index - 512] & 255 == 8, 'Rough floor lost cave encounters'
    for index in range(0x256, 0x25c):
        assert attrs[index - 512] == 0x1000, 'Haul rails must be normal ground below actors'
    for off, on in ((0x25c, 0x25d), (0x25e, 0x25f)):
        assert meta[(off - 512) * 8:(off - 511) * 8] != meta[(on - 512) * 8:(on - 511) * 8]
    layouts = json.loads((ROOT / 'data/layouts/layouts.json').read_text())['layouts']
    flags = {'Wrench': 'WRENCH', 'Hammer': 'HAMMER', 'Pick': 'PICK'}
    for suffix in ('Entrance', 'Depths', 'Gallery'):
        name = 'Virelia_Slateford_Quarry' + suffix
        entry = json.loads((ROOT / f'data/maps/{name}/map.json').read_text())
        layout = next(item for item in layouts if item['id'] == entry['layout'])
        assert layout['secondary_tileset'] == 'gTileset_VireliaQuarry'
        cells = words(ROOT / layout['blockdata_filepath'])
        assert all(512 <= cell & 1023 < 512 + len(attrs) for cell in cells)
        if suffix == 'Gallery':
            # The central pillar must have four capped rock corners, rather
            # than straight faces ending abruptly against the ground.
            for x, y, tile in ((10, 10, 0x208), (17, 10, 0x20a),
                               (10, 16, 0x218), (17, 16, 0x21a)):
                cell = cells[y * layout['width'] + x]
                assert cell & 1023 == tile and cell & 0xc00, 'Broken central pillar corner'
        for event in entry['warp_events']:
            x, y = event['x'], event['y']
            cell = cells[y * layout['width'] + x]
            assert not cell & 0xc00, 'Blocked arrival tile'
            above = cells[(y - 1) * layout['width'] + x]
            destination = event['dest_map']
            if destination == 'MAP_VIRELIA_SLATEFORD_CITY':
                assert cell & 1023 == WALL_EXIT and not above & 0xc00
                for dx, tile in ((-1, 0x250), (1, 0x252)):
                    side = cells[y * layout['width'] + x + dx]
                    assert side & 1023 == tile and side & 0xc00, 'Mouth shoulders must remain solid'
            elif LEVELS[destination] < LEVELS[entry['id']]:
                assert cell & 1023 == UPPER_LADDER_BASE
                assert above & 1023 == UPPER_LADDER_TOP and above & 0xc00, 'Upper ladder cap must be solid'
            else:
                assert cell & 1023 == DOWN_SHAFT, 'Descending warp must use a floor shaft'
                assert above & 1023 not in (0x20e, 0x20f), 'Descending shaft has an orphan upper ladder cap'
        for obj in entry['object_events']:
            script = obj['script'].rsplit('_', 1)[-1]
            cell = cells[obj['y'] * layout['width'] + obj['x']]
            assert not cell & 0xc00, f'Object over solid terrain: {script}'
            if script in flags:
                tool = flags[script]
                assert obj['graphics_id'] == 'OBJ_EVENT_GFX_QUARRY_' + tool
                assert obj['flag'] == 'FLAG_VIRELIA_TOOL_' + tool
    palette = (ROOT / 'graphics/object_events/palettes/moving_box.pal').read_text().splitlines()[3:19]
    for path in (ROOT / 'graphics/object_events/pics/quarry').glob('*.png'):
        pixels, depth, chunks = read_png(path)
        assert len(pixels) == len(pixels[0]) == 16 and depth == 4
        colors = next(data for kind, data in chunks if kind == b'PLTE')
        assert colors[:48] == bytes(int(channel) for line in palette for channel in line.split())
    validate_routes()
    print('PASS: quarry VRAM/palettes, cliff corners, native ladders, collectible props and safe rails')


if __name__ == '__main__':
    validate()
