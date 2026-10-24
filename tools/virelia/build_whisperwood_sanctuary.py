#!/usr/bin/env python3
"""Build the Whisperwood sanctuary and patch only its two Route 01 entrances.

Uses stock forest trees, a custom 48x64 old tree, and the native Arcanine
footprint as an anonymous paw impression. Pillow is needed only when regenerating these checked-in assets.
Existing towns, route actors, items and map IDs are preserved.
"""
from pathlib import Path
import json
import struct
from PIL import Image, ImageDraw

ROOT = Path(__file__).resolve().parents[2]
NAME = 'Virelia_WhisperwoodSanctuary'
PREFIX = 'Virelia_WhisperwoodSanctuary_'


def write_json(path, value):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')


def write_grid(path, grid):
    path.parent.mkdir(parents=True, exist_ok=True)
    cells = [v for row in grid for v in row]
    path.write_bytes(struct.pack(f'<{len(cells)}H', *cells))


def rect(grid, x, y, w, h, value):
    for yy in range(y, y + h):
        for xx in range(x, x + w):
            grid[yy][xx] = value


def build_props():
    props = []
    paw = Image.new('P', (16, 16), 0)
    footprint = Image.open(ROOT / 'graphics/pokemon/arcanine/footprint.png').convert('RGB')
    for y in range(16):
        for x in range(16):
            if sum(footprint.getpixel((x, y))) < 384:
                paw.putpixel((x, y), 8)
    props.append(paw)
    symbols = (
        [(7, 7), (8, 6), (9, 7), (8, 8)],  # seed
        [(6, 6), (6, 8), (9, 7), (9, 9)],  # rain
        [(8, 6), (8, 7), (8, 8), (7, 9), (9, 9), (6, 10), (10, 10)],
    )
    for symbol in symbols:
        stone = Image.new('P', (16, 16), 0)
        draw = ImageDraw.Draw(stone)
        draw.polygon([(4, 3), (10, 2), (13, 5), (13, 12), (10, 14), (3, 13), (2, 9)], fill=8)
        draw.polygon([(5, 4), (10, 3), (12, 6), (11, 12), (4, 12), (3, 8)], fill=6)
        draw.line([(4, 4), (7, 3), (10, 4), (12, 6)], fill=2, width=2)
        draw.point([(3, 10), (4, 11), (5, 12), (11, 11)], fill=3)
        draw.point(symbol, fill=9)
        props.append(stone)
    return props


def build_tileset():
    source = ROOT / 'data/tilesets/secondary/fortree'
    target = ROOT / 'data/tilesets/secondary/whisperwood'
    original = Image.open(source / 'tiles.png')
    tree = Image.open(target / 'old_tree.png')
    if tree.mode != 'P' or tree.size != (48, 64) or tree.info.get('transparency') != 0:
        raise ValueError('Old tree must be a 48x64 indexed sprite with transparent index zero')
    bank, indices = [], {}

    def add_tile(tile):
        pixels = tile.tobytes()
        if pixels not in indices:
            indices[pixels] = len(bank)
            bank.append(tile.copy())
        return 512 + indices[pixels]

    # Deduplicate stock pixel patterns, keeping every Fortree metatile intact.
    # This frees room for the new 48x64 tree without exceeding 512 8x8 tiles.
    remap = {}
    for index in range(512):
        remap[index] = add_tile(original.crop(((index % 16) * 8, (index // 16) * 8,
                                             (index % 16) * 8 + 8, (index // 16) * 8 + 8)))
    original_meta = (source / 'metatiles.bin').read_bytes()
    values = struct.unpack(f'<{len(original_meta) // 2}H', original_meta)
    values = [(value & ~0x3FF) | remap[(value & 0x3FF) - 512]
              if value & 0x3FF >= 512 else value for value in values]
    raw = bytearray(struct.pack(f'<{len(values)}H', *values))
    attrs = bytearray((source / 'metatile_attributes.bin').read_bytes())
    assert 0x200 + len(raw) // 16 == 0x318
    primary = (ROOT / 'data/tilesets/primary/general/metatiles.bin').read_bytes()
    grass = struct.unpack_from('<8H', primary, 16)

    def metatile(image, x, y, palette):
        overlay = []
        for dy in (0, 8):
            for dx in (0, 8):
                overlay.append((palette << 12) | add_tile(image.crop((x + dx, y + dy, x + dx + 8, y + dy + 8))))
        return struct.pack('<8H', *grass[:4], *overlay)

    for prop in build_props():
        raw.extend(metatile(prop, 0, 0, 2))
        attrs.extend(struct.pack('<H', 0))
    # Keep the earlier root metatile ID valid for saved map views.
    root_offset = len(raw)
    raw.extend(struct.pack('<8H', *grass))
    attrs.extend(struct.pack('<H', 0))
    # Twelve contiguous metatiles: 0x31D..0x328, row major, three wide.
    for y in range(0, 64, 16):
        for x in range(0, 48, 16):
            raw.extend(metatile(tree, x, y, 6))
            attrs.extend(struct.pack('<H', 0))
    raw[root_offset:root_offset + 16] = metatile(tree, 16, 48, 6)
    assert len(bank) <= 512, 'Whisperwood secondary tile bank is full'
    tiles = Image.new('P', (128, 256), 0)
    tiles.putpalette(original.getpalette())
    for index, tile in enumerate(bank):
        tiles.paste(tile, ((index % 16) * 8, (index // 16) * 8))
    tiles.save(target / 'tiles.png')
    (target / 'metatiles.bin').write_bytes(raw)
    (target / 'metatile_attributes.bin').write_bytes(attrs)
    colors = tree.getpalette()[:48]
    palettes = target / 'palettes'
    palettes.mkdir(exist_ok=True)
    for slot in (6, 15):
        rgb = [tuple(colors[i:i + 3]) for i in range(0, 48, 3)]
        if slot == 15:
            rgb = [tuple((channel * 3 // 4 // 8) * 8 for channel in color) for color in rgb]
        text = 'JASC-PAL\n0100\n16\n' + '\n'.join(' '.join(map(str, color)) for color in rgb) + '\n'
        (palettes / f'{slot:02}.pal').write_text(text, encoding='ascii', newline='\r\n')
    print(f'Packed {len(bank)} unique 8x8 tiles; all stock Fortree artwork retained.')


def sign(x, y, suffix):
    return dict(type='sign', x=x, y=y, elevation=0,
                player_facing_dir='BG_EVENT_PLAYER_FACING_ANY', script=PREFIX + 'EventScript_' + suffix)


def paint_forest(grid):
    """Stamp whole 2x3 trees; fill narrow fringes with complete small shrubs.

    Paint after carving paths so no rectangle can cut a canopy in half.
    All six cells are reserved together, including the canopy's grass backing.
    """
    tree = ((0x05CE, 0x05CF), (0x05D4, 0x05D5), (0x05E4, 0x05E5))
    available = {(x, y) for y, row in enumerate(grid) for x, tile in enumerate(row) if tile == 0x040E}
    for y in range(len(grid) - 2):
        for x in range(len(grid[0]) - 1):
            footprint = {(x + dx, y + dy) for dy in range(3) for dx in range(2)}
            if not footprint <= available:
                continue
            for dy, row in enumerate(tree):
                for dx, tile in enumerate(row):
                    grid[y + dy][x + dx] = tile
            available.difference_update(footprint)


def main():
    build_tileset()
    grid = [[0x040E for x in range(22)] for y in range(26)]
    rect(grid, 4, 4, 10, 10, 0x3001)
    # Long overgrown approach loops south of the sanctuary.
    rect(grid, 16, 10, 4, 12, 0x3001)
    rect(grid, 4, 18, 16, 4, 0x3001)
    rect(grid, 4, 12, 4, 10, 0x3001)
    rect(grid, 19, 10, 3, 2, 0x300D)
    for x, y, w, h in [(17, 12, 2, 7), (7, 19, 10, 1), (6, 14, 2, 4)]:
        rect(grid, x, y, w, h, 0x300D)
    for x, y in [(16, 20), (8, 20), (5, 18), (7, 14), (13, 4), (4, 10), (12, 12)]:
        grid[y][x] = 0x3004
    # Carve both branches before painting complete forest trees.
    rect(grid, 13, 2, 9, 2, 0x3001)
    paint_forest(grid)
    # Standalone shrubs conceal the inward gates without half-tree fragments.
    rect(grid, 16, 2, 1, 2, 0x040E)
    rect(grid, 20, 10, 1, 2, 0x040E)
    # Dedicated custom tree: one continuous 48x64 sprite, 3x4 map tiles.
    for dy in range(4):
        for dx in range(3):
            grid[6 + dy][8 + dx] = 0x0400 | (0x31D + dy * 3 + dx)
    for x, y, tile in [(6, 8, 0x719), (12, 8, 0x71A), (9, 12, 0x71B),
                       (9, 10, 0x3318), (10, 11, 0x3318), (11, 13, 0x3318)]:
        grid[y][x] = tile
    layout_path = ROOT / 'data/layouts' / NAME
    write_grid(layout_path / 'map.bin', grid)
    (layout_path / 'border.bin').write_bytes(struct.pack('<4H', *([0x040E] * 4)))
    registry_path = ROOT / 'data/layouts/layouts.json'
    registry = json.loads(registry_path.read_text(encoding='utf-8'))
    entry = dict(id='LAYOUT_VIRELIA_WHISPERWOOD_SANCTUARY', name=NAME + '_Layout', width=22, height=26,
                 primary_tileset='gTileset_General', secondary_tileset='gTileset_Whisperwood',
                 border_filepath=f'data/layouts/{NAME}/border.bin', blockdata_filepath=f'data/layouts/{NAME}/map.bin',
                 layout_version='emerald')
    if not any(x['id'] == entry['id'] for x in registry['layouts']):
        registry['layouts'].append(entry)
    else:
        assert next(x for x in registry['layouts'] if x['id'] == entry['id']) == entry
    write_json(registry_path, registry)
    group_path = ROOT / 'data/maps/map_groups.json'
    groups = json.loads(group_path.read_text(encoding='utf-8'))
    groups['gMapGroup_VireliaInteriors'] = [name for name in groups['gMapGroup_VireliaInteriors'] if name != NAME]
    if NAME not in groups['gMapGroup_VireliaOverworld']:
        groups['gMapGroup_VireliaOverworld'].append(NAME)
    write_json(group_path, groups)
    sanctuary = dict(id='MAP_VIRELIA_WHISPERWOOD_SANCTUARY', name=NAME, layout=entry['id'],
        music='MUS_PETALBURG_WOODS', region='REGION_HOENN', region_map_section='MAPSEC_VIRELIA_WHISPERWOOD_TRAIL',
        requires_flash=False, weather='WEATHER_SUNNY', map_type='MAP_TYPE_ROUTE', allow_cycling=False,
        allow_escaping=False, allow_running=True, show_map_name=False, battle_scene='MAP_BATTLE_SCENE_NORMAL',
        connections=[dict(map='MAP_VIRELIA_ROUTE01', offset=-6, direction='right')],
        object_events=[], warp_events=[], coord_events=[],
        bg_events=[sign(6, 8, 'SeedStone'), sign(12, 8, 'RainStone'), sign(9, 12, 'RootStone'),
                   sign(9, 9, 'AncientTree'), sign(9, 10, 'Pawprints'), sign(10, 11, 'Pawprints'),
                   sign(11, 13, 'Pawprints'), sign(16, 2, 'ShortcutRoots'), sign(16, 3, 'ShortcutRoots')])
    write_json(ROOT / 'data/maps' / NAME / 'map.json', sanctuary)
    route_path = ROOT / 'data/maps/Virelia_Route01/map.json'
    route = json.loads(route_path.read_text(encoding='utf-8'))
    connection = dict(map=sanctuary['id'], offset=6, direction='left')
    if connection not in route['connections']:
        route['connections'].append(connection)
    for x, y, suffix in [(1, 16, 'OvergrownPath'), (1, 17, 'OvergrownPath'), (1, 8, 'SanctuaryShortcut'), (1, 9, 'SanctuaryShortcut')]:
        event = sign(x, y, suffix)
        event['script'] = 'Virelia_Route01_EventScript_' + suffix
        if event not in route['bg_events']:
            route['bg_events'].append(event)
    write_json(route_path, route)
    binary = ROOT / 'data/layouts/Virelia_Route01/map.bin'
    cells = list(struct.unpack('<1260H', binary.read_bytes()))
    # Seams are always passable; dynamic gates sit one tile inward. Connected
    # border buffers use the neighbor's raw layout before its load script runs.
    for y in (8, 9, 16, 17):
        cells[y * 30] = 0x300D if y >= 16 else 0x3001
        cells[y * 30 + 1] = 0x040E
    binary.write_bytes(struct.pack('<1260H', *cells))
    print('Built Whisperwood sanctuary; only the two western Route 01 entrances were patched.')


if __name__ == '__main__':
    main()
