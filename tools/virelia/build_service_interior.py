#!/usr/bin/env python3
"""Build Slateford's shared Center and shop from unmodified Emerald donors.

Only this interior is regenerated. Native healing geometry and primary Building
behaviors stay in place; Shop overlays use the Center's floor and wall below.
"""
from pathlib import Path
import json
import struct

ROOT = Path(__file__).resolve().parents[2]
NAME = 'Virelia_Slateford_PokemonCenter'
LAYOUT_ID = 'LAYOUT_VIRELIA_SLATEFORD_POKEMON_CENTER'
SYMBOL = 'gTileset_VireliaServiceInterior'
TARGET = ROOT / 'data/tilesets/secondary/virelia_service_interior'
WIDTH, HEIGHT = 24, 9
# Destination ID, native Shop overlay, Center lower layer. The receipt remains
# visual only: it is an ordinary counter, not the stock questionnaire service.
SHOP_PARTS = (
    (0x300, 0x320, 0x202), (0x301, 0x240, 0x202),
    (0x302, 0x242, 0x202), (0x303, 0x22A, 0x213),
    (0x304, 0x232, 0x202), (0x305, 0x223, 0x20B),
    (0x306, 0x224, 0x20B), (0x307, 0x228, 0x213),
    (0x308, 0x229, 0x213), (0x309, 0x230, 0x21B),
    (0x30A, 0x231, 0x21B), (0x30B, 0x22B, 0x202),
    (0x30C, 0x22C, 0x202), (0x30D, 0x233, 0x202),
    (0x30E, 0x234, 0x202), (0x30F, 0x23B, 0x202),
    (0x310, 0x23C, 0x202), (0x311, 0x23A, 0x202),
    (0x312, 0x242, 0x202),
)


def words(path):
    raw = path.read_bytes()
    return struct.unpack(f'<{len(raw) // 2}H', raw)


def write_words(path, values):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(struct.pack(f'<{len(values)}H', *values))


def write_json(path, value):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, ensure_ascii=False, indent=2) + '\n', encoding='utf-8', newline='\n')


def source_grid(registry):
    native = next(x for x in registry['layouts'] if x['id'] == 'LAYOUT_POKEMON_CENTER_1F')
    values = words(ROOT / native['blockdata_filepath'])
    old = [list(values[y * 14:(y + 1) * 14]) for y in range(HEIGHT)]
    # This local Center has no upstairs link service.
    for y in range(5, 8):
        old[y][:3] = [0x3220, 0x3202, 0x3202]
    grid = [row[:13] + [0x3202] * 10 + [row[13]] for row in old]
    for x in range(13, 23):
        grid[0][x], grid[1][x], grid[2][x] = 0x060B, 0x0613, 0x321B
    grid[3][14:19] = [0x0712, 0x0700, 0x0701, 0x0701, 0x0702]
    grid[2][14] = 0x0711
    grid[1][18], grid[2][18] = 0x0703, 0x0704
    for y, first in ((0, 0x305), (1, 0x307), (2, 0x309),
                     (4, 0x30B), (5, 0x30D), (6, 0x30F)):
        for x in range(2):
            # Back cabinet bases leave the native half-tile approach open.
            grid[y][20 + x] = (0x3000 if y == 2 else 0x400) | (first + x)
    return grid, words(ROOT / native['border_filepath'])


def pack_tiles(grid, border):
    from PIL import Image

    sources = {name: ROOT / f'data/tilesets/secondary/{name}'
               for name in ('pokemon_center', 'shop')}
    images = {name: Image.open(path / 'tiles.png') for name, path in sources.items()}
    metas = {name: words(path / 'metatiles.bin') for name, path in sources.items()}
    attrs = {name: words(path / 'metatile_attributes.bin') for name, path in sources.items()}
    used = {v & 0x3FF for row in grid for v in row} | {v & 0x3FF for v in border}
    definitions = {}
    attributes = {}
    for index in sorted(used):
        if 512 <= index < 0x300:
            definitions[index] = [('pokemon_center', v) for v in
                                  metas['pokemon_center'][(index - 512) * 8:(index - 511) * 8]]
            attributes[index] = attrs['pokemon_center'][index - 512]
    for index, shop, base in SHOP_PARTS:
        definitions[index] = [('pokemon_center', v) for v in
                              metas['pokemon_center'][(base - 512) * 8:(base - 512) * 8 + 4]]
        overlay = metas['shop'][(shop - 512) * 8 + 4:(shop - 511) * 8]
        if index in (0x311, 0x312):
            overlay = [overlay[i] ^ 0x400 if overlay[i] else 0 for i in (1, 0, 3, 2)]
        definitions[index] += [('shop', v) for v in overlay]
        attributes[index] = 0x1080 if index == 0x300 else attrs['shop'][shop - 512]

    # All selected Shop overlay pixels use palette 7. Center leaves slot 10
    # unused, so the original palettes and their exact colors fit without loss.
    assert all(v >> 12 in (0, 7) for parts in definitions.values()
               for source, v in parts if source == 'shop')
    assert all(v >> 12 != 10 for parts in definitions.values()
               for source, v in parts if source == 'pokemon_center')
    bank, lookup = [], {}
    packed = [0] * ((0x313 - 512) * 8)
    packed_attrs = [0] * (0x313 - 512)
    for index, parts in sorted(definitions.items()):
        packed_attrs[index - 512] = attributes[index]
        for n, (source, value) in enumerate(parts):
            tile = value & 0x3FF
            if tile >= 512:
                tile -= 512
                data = images[source].crop((tile % 16 * 8, tile // 16 * 8,
                                            tile % 16 * 8 + 8, tile // 16 * 8 + 8)).tobytes()
                if any(data):
                    if data not in lookup:
                        lookup[data] = 512 + len(bank)
                        bank.append(data)
                    value = (value & ~0x3FF) | lookup[data]
                else:
                    value = 0
            if source == 'shop' and value >> 12 == 7:
                value = (value & 0xFFF) | (10 << 12)
            packed[(index - 512) * 8 + n] = value
    assert len(bank) <= 496, 'Interior graphics exceed the secondary tile bank'
    TARGET.mkdir(parents=True, exist_ok=True)
    sheet = Image.new('P', (128, 256))
    sheet.putpalette(images['pokemon_center'].getpalette())
    for i, data in enumerate(bank):
        sheet.paste(Image.frombytes('P', (8, 8), data), (i % 16 * 8, i // 16 * 8))
    sheet.save(TARGET / 'tiles.png', bits=4)
    write_words(TARGET / 'metatiles.bin', packed)
    write_words(TARGET / 'metatile_attributes.bin', packed_attrs)
    (TARGET / 'palettes').mkdir(exist_ok=True)
    for slot in range(16):
        source, source_slot = ('shop', 7) if slot == 10 else ('pokemon_center', slot)
        (TARGET / f'palettes/{slot:02}.pal').write_bytes(
            (sources[source] / f'palettes/{source_slot:02}.pal').read_bytes())
    return len(bank)


def actor(gfx, x, y, label, facing='DOWN', local_id=None):
    obj = dict(graphics_id='OBJ_EVENT_GFX_' + gfx, x=x, y=y, elevation=3,
               movement_type='MOVEMENT_TYPE_FACE_' + facing, movement_range_x=0, movement_range_y=0,
               trainer_type='TRAINER_TYPE_NONE', trainer_sight_or_berry_tree_id='0',
               script=NAME + '_EventScript_' + label, flag='0')
    return dict(local_id=local_id, **obj) if local_id else obj


def main():
    registry_path = ROOT / 'data/layouts/layouts.json'
    registry = json.loads(registry_path.read_text(encoding='utf-8'))
    grid, border = source_grid(registry)
    tile_count = pack_tiles(grid, border)
    layout = dict(id=LAYOUT_ID, name=NAME + '_Layout', width=WIDTH, height=HEIGHT,
                  primary_tileset='gTileset_Building', secondary_tileset=SYMBOL,
                  border_filepath=f'data/layouts/{NAME}/border.bin',
                  blockdata_filepath=f'data/layouts/{NAME}/map.bin', layout_version='emerald')
    old = next((x for x in registry['layouts'] if x['id'] == LAYOUT_ID), None)
    if old is None:
        registry['layouts'].append(layout)
    else:
        old.clear()
        old.update(layout)
    registry['layouts'] = [x for x in registry['layouts'] if x['id'] != 'LAYOUT_VIRELIA_SLATEFORD_MART']
    write_json(registry_path, registry)
    write_words(ROOT / layout['blockdata_filepath'], [v for row in grid for v in row])
    write_words(ROOT / layout['border_filepath'], border)
    objects = [actor('NURSE', 7, 2, 'Nurse', local_id='LOCALID_VIRELIA_SLATEFORD_NURSE'),
               actor('GENTLEMAN', 4, 4, 'PCAdvice'), actor('BOY_1', 10, 6, 'Advice'),
               actor('MART_EMPLOYEE', 16, 2, 'Clerk'), actor('WOMAN_5', 18, 6, 'ShopWoman'),
               actor('BOY_1', 22, 7, 'ShopBoy')]
    map_path = ROOT / 'data/maps' / NAME / 'map.json'
    metadata = json.loads(map_path.read_text(encoding='utf-8')) if map_path.exists() else dict(
        id='MAP_VIRELIA_SLATEFORD_POKEMON_CENTER', name=NAME, layout=LAYOUT_ID,
        music='MUS_POKE_CENTER', region_map_section='MAPSEC_VIRELIA_SLATEFORD_CITY',
        requires_flash=False, weather='WEATHER_NONE', map_type='MAP_TYPE_INDOOR',
        allow_cycling=False, allow_escaping=False, allow_running=True, show_map_name=False,
        battle_scene='MAP_BATTLE_SCENE_NORMAL', connections=None, coord_events=[], bg_events=[],
        region='REGION_HOENN')
    metadata['object_events'] = objects
    metadata['warp_events'] = [dict(x=x, y=8, elevation=0, dest_map='MAP_VIRELIA_SLATEFORD_CITY',
                                    dest_warp_id='0') for x in (7, 6)]
    write_json(map_path, metadata)
    print(f'Built shared Center/shop: {WIDTH}x{HEIGHT}, {tile_count} secondary graphics tiles')


if __name__ == '__main__':
    main()
