#!/usr/bin/env python3
"""Append the abandoned gallery and its two powered quarry approaches.

Run after build_first_gym_maps.py, or alone to update the existing quarry.
Reapplies the dedicated quarry art pass after generating the quest geometry.
"""
import json
import struct
from pathlib import Path

from build_first_gym_maps import actor, cave_grid, rect, sign, warp

ROOT = Path(__file__).resolve().parents[2]
GALLERY = 'Virelia_Slateford_QuarryGallery'
PREFIX = GALLERY + '_'


def write_json(path, value):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, ensure_ascii=False, indent=2) + '\n', encoding='utf-8', newline='\n')


def patch_approach(name, x, y, gallery_warp, clue=False):
    path = ROOT / f'data/maps/{name}/map.json'
    entry = json.loads(path.read_text(encoding='utf-8'))
    entry['warp_events'] = [event for event in entry['warp_events']
                            if event['dest_map'] != 'MAP_VIRELIA_SLATEFORD_QUARRY_GALLERY']
    entry['warp_events'].append(warp(x, y, 'MAP_VIRELIA_SLATEFORD_QUARRY_GALLERY', gallery_warp))
    entry['bg_events'] = [event for event in entry['bg_events']
                          if event['script'] != name + '_EventScript_GalleryGate']
    entry['bg_events'].append(sign(x, y, name + '_EventScript_GalleryGate'))
    if clue and not any(obj['script'] == PREFIX + 'EventScript_Wrench' for obj in entry['object_events']):
        entry['object_events'].append(actor('ITEM_BALL', 7, 6, PREFIX + 'EventScript_Wrench',
                                            flag='FLAG_VIRELIA_TOOL_WRENCH'))
    write_json(path, entry)
    layout = next(item for item in json.loads((ROOT / 'data/layouts/layouts.json').read_text())['layouts']
                  if item['id'] == entry['layout'])
    path = ROOT / layout['blockdata_filepath']
    cells = bytearray(path.read_bytes())
    # Keep native ladder behavior in the layout; ON_LOAD covers it with a
    # parked cart until electricity returns, including after Continue.
    struct.pack_into('<H', cells, 2 * (y * layout['width'] + x), 0x3214)
    path.write_bytes(cells)


def main():
    patch_approach('Virelia_Slateford_QuarryEntrance', 4, 3, 0, clue=True)
    patch_approach('Virelia_Slateford_QuarryDepths', 3, 7, 1)
    grid = cave_grid(30, 24, [(2, 16, 8, 6), (5, 8, 5, 12), (5, 4, 17, 6),
                              (18, 6, 10, 6), (23, 9, 5, 13), (8, 17, 18, 5),
                              (18, 10, 4, 8)])
    # Leave a broad ring around an untouched rock pillar. The thief's hoard is
    # an alcove off this ring, so neither Sableye nor its tools block the loop.
    for x, y in [(3, 17), (8, 12), (10, 5), (16, 8), (20, 7), (26, 14), (12, 20), (20, 19)]:
        grid[y][x] = 0x0603
    for x, y in [(6, 4), (12, 4), (19, 6), (27, 10)]:
        grid[y][x] = 0x3403
    grid[20][3] = grid[20][26] = 0x3214
    folder = ROOT / 'data/layouts' / GALLERY
    folder.mkdir(parents=True, exist_ok=True)
    cells = [value for row in grid for value in row]
    (folder / 'map.bin').write_bytes(struct.pack(f'<{len(cells)}H', *cells))
    (folder / 'border.bin').write_bytes(struct.pack('<4H', *([0x0211] * 4)))
    registry = ROOT / 'data/layouts/layouts.json'
    layouts = json.loads(registry.read_text(encoding='utf-8'))
    layout = dict(id='LAYOUT_VIRELIA_SLATEFORD_QUARRY_GALLERY', name=GALLERY + '_Layout',
                  width=30, height=24, primary_tileset='gTileset_General',
                  secondary_tileset='gTileset_RusturfTunnel',
                  border_filepath=f'data/layouts/{GALLERY}/border.bin',
                  blockdata_filepath=f'data/layouts/{GALLERY}/map.bin', layout_version='emerald')
    if not any(item['id'] == layout['id'] for item in layouts['layouts']):
        layouts['layouts'].append(layout)
    else:
        existing = next(item for item in layouts['layouts'] if item['id'] == layout['id'])
        assert all(existing[key] == value for key, value in layout.items() if key != 'secondary_tileset')
    write_json(registry, layouts)
    sableye = actor('ITEM_BALL', 24, 9, PREFIX + 'EventScript_Sableye',
                    local_id='LOCALID_VIRELIA_GALLERY_SABLEYE',
                    flag='FLAG_VIRELIA_MISSING_TOOLS_RECOVERED')
    sableye['graphics_id'] = 'OBJ_EVENT_GFX_SPECIES(SABLEYE)'
    entry = dict(id='MAP_VIRELIA_SLATEFORD_QUARRY_GALLERY', name=GALLERY, layout=layout['id'],
                 music='MUS_CAVE_OF_ORIGIN', region='REGION_HOENN',
                 region_map_section='MAPSEC_VIRELIA_SLATEFORD_CITY', requires_flash=False,
                 weather='WEATHER_NONE', map_type='MAP_TYPE_UNDERGROUND', allow_cycling=False,
                 allow_escaping=True, allow_running=True, show_map_name=False,
                 battle_scene='MAP_BATTLE_SCENE_NORMAL', connections=None,
                 object_events=[sableye,
                     actor('ITEM_BALL', 7, 17, PREFIX + 'EventScript_Hammer', flag='FLAG_VIRELIA_TOOL_HAMMER'),
                     actor('ITEM_BALL', 17, 6, PREFIX + 'EventScript_Pick', flag='FLAG_VIRELIA_TOOL_PICK'),
                     actor('MOVING_BOX', 25, 8, PREFIX + 'EventScript_Hoard')],
                 warp_events=[warp(3, 20, 'MAP_VIRELIA_SLATEFORD_QUARRY_ENTRANCE', 2),
                              warp(26, 20, 'MAP_VIRELIA_SLATEFORD_QUARRY_DEPTHS', 1)],
                 coord_events=[], bg_events=[sign(4, 20, PREFIX + 'EventScript_Sign')])
    write_json(ROOT / f'data/maps/{GALLERY}/map.json', entry)
    group_path = ROOT / 'data/maps/map_groups.json'
    groups = json.loads(group_path.read_text(encoding='utf-8'))
    if GALLERY not in groups['gMapGroup_VireliaInteriors']:
        groups['gMapGroup_VireliaInteriors'].append(GALLERY)
    write_json(group_path, groups)
    print('Built Missing Tools gallery: QuarryEntrance <-> Gallery <-> QuarryDepths.')
    from build_quarry import main as build_quarry
    build_quarry()


if __name__ == '__main__':
    main()
