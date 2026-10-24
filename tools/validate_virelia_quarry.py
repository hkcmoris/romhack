#!/usr/bin/env python3
"""Check quarry art banks, interactions and encounter-free haul routes."""
import json
from pathlib import Path
import struct

from virelia.localize_type_icons import read_png
from validate_virelia_missing_tools import validate as validate_routes

ROOT = Path(__file__).resolve().parents[1]
BANK = ROOT / 'data/tilesets/secondary/virelia_quarry'


def words(path):
    raw = path.read_bytes()
    return struct.unpack(f'<{len(raw) // 2}H', raw)


def validate():
    pixels, depth, _ = read_png(BANK / 'tiles.png')
    assert depth == 4 and len(pixels) == 256 and len(pixels[0]) == 128
    meta, attrs = words(BANK / 'metatiles.bin'), words(BANK / 'metatile_attributes.bin')
    assert len(meta) == len(attrs) * 8 <= 512 * 8
    for value in meta:
        assert value >> 12 < 13, 'Art uses a reserved UI palette'
    assert attrs[0x214 - 512] & 255 == 0x61, 'Custom ladders lost native warp behavior'
    assert attrs[0x207 - 512] & 255 == 0x65, 'City exit lost native warp behavior'
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
            cell = cells[event['y'] * layout['width'] + event['x']]
            assert not cell & 0xc00, 'Blocked arrival tile'
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
