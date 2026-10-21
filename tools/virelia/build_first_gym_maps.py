#!/usr/bin/env python3
"""Build v0.3 maps from existing Emerald assets, without regenerating old maps.

The Slateford edits replace only the two former closed service exteriors and
append the quarry entrance. Existing prologue layouts and event IDs remain.
Run from the repository root; no external image editor or assets are required.
"""
from pathlib import Path
import copy
import json
import struct

ROOT = Path(__file__).resolve().parents[2]
REGISTRY = ROOT / 'data/layouts/layouts.json'
layouts_doc = json.loads(REGISTRY.read_text(encoding='utf-8'))
layouts = layouts_doc['layouts']
CITY = 'MAP_VIRELIA_SLATEFORD_CITY'
PREFIX = 'Virelia_Slateford_'


def json_write(path, value):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')


def grid_for(layout_id):
    layout = next(x for x in layouts if x['id'] == layout_id)
    values = struct.unpack('<' + 'H' * (layout['width'] * layout['height']),
                           (ROOT / layout['blockdata_filepath']).read_bytes())
    return [list(values[y * layout['width']:(y + 1) * layout['width']]) for y in range(layout['height'])]


def write_grid(path, grid):
    path.parent.mkdir(parents=True, exist_ok=True)
    values = [v for row in grid for v in row]
    path.write_bytes(struct.pack('<' + 'H' * len(values), *values))


def rect(grid, x, y, width, height, value):
    for yy in range(y, y + height):
        for xx in range(x, x + width):
            grid[yy][xx] = value


def copy_rect(grid, source, sx, sy, width, height, x, y):
    for yy in range(height):
        for xx in range(width):
            grid[y + yy][x + xx] = source[sy + yy][sx + xx]


def add_layout(name, grid, primary, secondary, border):
    constant = 'LAYOUT_VIRELIA_SLATEFORD_' + {'QuarryEntrance': 'QUARRY_ENTRANCE', 'QuarryDepths': 'QUARRY_DEPTHS'}.get(name, name.upper())
    # PokemonCenter deliberately uses the standard separated spelling.
    if name == 'PokemonCenter':
        constant = 'LAYOUT_VIRELIA_SLATEFORD_POKEMON_CENTER'
    path = 'data/layouts/' + PREFIX + name
    value = dict(id=constant, name=PREFIX + name + '_Layout', width=len(grid[0]), height=len(grid),
                 primary_tileset='gTileset_' + primary, secondary_tileset='gTileset_' + secondary,
                 border_filepath=path + '/border.bin', blockdata_filepath=path + '/map.bin',
                 layout_version='emerald')
    if not any(x['id'] == constant for x in layouts):
        layouts.append(value)
    else:
        assert next(x for x in layouts if x['id'] == constant) == value
    write_grid(ROOT / value['blockdata_filepath'], grid)
    (ROOT / value['border_filepath']).write_bytes(struct.pack('<4H', *border))
    return constant


def actor(gfx, x, y, script, *, local_id=None, trainer=False, flag='0', facing='DOWN'):
    obj = dict(graphics_id='OBJ_EVENT_GFX_' + gfx, x=x, y=y, elevation=3,
               movement_type='MOVEMENT_TYPE_FACE_' + facing, movement_range_x=0, movement_range_y=0,
               trainer_type='TRAINER_TYPE_NORMAL' if trainer else 'TRAINER_TYPE_NONE',
               trainer_sight_or_berry_tree_id='1' if trainer else '0', script=script, flag=flag)
    if local_id is not None:
        obj = dict(local_id=local_id, **obj)
    return obj


def warp(x, y, destination, index):
    return dict(x=x, y=y, elevation=0, dest_map=destination, dest_warp_id=str(index))


def sign(x, y, script):
    return dict(type='sign', x=x, y=y, elevation=0,
                player_facing_dir='BG_EVENT_PLAYER_FACING_ANY', script=script)


def new_map(name, layout, objects, warps, signs=(), *, cave=False):
    suffix = {'PokemonCenter': 'POKEMON_CENTER', 'QuarryEntrance': 'QUARRY_ENTRANCE',
              'QuarryDepths': 'QUARRY_DEPTHS'}.get(name, name.upper())
    music = {'PokemonCenter': 'MUS_POKE_CENTER', 'Mart': 'MUS_POKE_MART', 'Gym': 'MUS_GYM'}.get(name, 'MUS_PETALBURG_WOODS')
    result = dict(id='MAP_VIRELIA_SLATEFORD_' + suffix, name=PREFIX + name, layout=layout,
                  music=music, region='REGION_HOENN', region_map_section='MAPSEC_VIRELIA_SLATEFORD_CITY',
                  requires_flash=False, weather='WEATHER_NONE',
                  map_type='MAP_TYPE_UNDERGROUND' if cave else 'MAP_TYPE_INDOOR',
                  allow_cycling=False, allow_escaping=cave, allow_running=True, show_map_name=False,
                  battle_scene='MAP_BATTLE_SCENE_GYM' if name == 'Gym' else 'MAP_BATTLE_SCENE_NORMAL',
                  connections=None, object_events=objects, warp_events=warps, coord_events=[], bg_events=list(signs))
    json_write(ROOT / 'data/maps' / (PREFIX + name) / 'map.json', result)


def cave_grid(width, height, rooms):
    grid = [[0x0611] * width for _ in range(height)]
    for room in rooms:
        rect(grid, *room, 0x3201)
    # Use the stock cave's upper cliff lips and side faces around carved rooms.
    original = [row[:] for row in grid]
    for y in range(1, height - 1):
        for x in range(1, width - 1):
            if original[y][x] != 0x0611:
                continue
            below = original[y + 1][x] == 0x3201
            right = original[y][x + 1] == 0x3201
            left = original[y][x - 1] == 0x3201
            above = original[y - 1][x] == 0x3201
            grid[y][x] = 0x0619 if below else 0x0612 if right else 0x0610 if left else 0x0609 if above else 0x0611
    return grid


def main():
    center = grid_for('LAYOUT_POKEMON_CENTER_1F')
    # Replace the upstairs approach and escalator with the stock floor and wall.
    # This chapter has no upstairs map, multiplayer gate or network interaction.
    for y in (5, 6, 7):
        center[y][0] = 0x3220
        center[y][1] = 0x3202
        center[y][2] = 0x3202
    lc = add_layout('PokemonCenter', center, 'Building', 'PokemonCenter', (0x0608,) * 4)
    cp = PREFIX + 'PokemonCenter_'
    new_map('PokemonCenter', lc,
            [actor('NURSE', 7, 2, cp + 'EventScript_Nurse', local_id='LOCALID_VIRELIA_SLATEFORD_NURSE'),
             actor('GENTLEMAN', 4, 4, cp + 'EventScript_PCAdvice'),
             actor('BOY_1', 10, 6, cp + 'EventScript_Advice')],
            [warp(7, 8, CITY, 0), warp(6, 8, CITY, 0)])

    mart = grid_for('LAYOUT_MART')
    lm = add_layout('Mart', mart, 'Building', 'Shop', (0x0610,) * 4)
    mp = PREFIX + 'Mart_'
    new_map('Mart', lm,
            [actor('MART_EMPLOYEE', 1, 3, mp + 'EventScript_Clerk', facing='RIGHT'),
             actor('WOMAN_5', 5, 5, mp + 'EventScript_Woman'),
             actor('BOY_1', 9, 4, mp + 'EventScript_Boy')],
            [warp(3, 7, CITY, 1), warp(4, 7, CITY, 1)])

    entrance = cave_grid(24, 20, [(2, 3, 8, 5), (3, 11, 8, 7), (7, 7, 4, 7),
                                 (10, 8, 9, 3), (16, 4, 6, 7), (17, 10, 4, 7), (10, 15, 10, 3)])
    for x, y in [(3, 5), (8, 6), (9, 16), (17, 8), (19, 14)]:
        entrance[y][x] = 0x0603
    entrance[17][3] = 0x3207
    entrance[4][20] = 0x3214
    entrance[15][5] = 0x3202
    le = add_layout('QuarryEntrance', entrance, 'General', 'RusturfTunnel', (0x0611,) * 4)
    ep = PREFIX + 'QuarryEntrance_'
    new_map('QuarryEntrance', le,
            [actor('HIKER', 8, 12, ep + 'EventScript_Bruno', trainer=True),
             actor('ITEM_BALL', 5, 4, ep + 'EventScript_Item', flag='FLAG_VIRELIA_ITEM_QUARRY_SUPER_POTION'),
             actor('HIKER', 5, 15, ep + 'EventScript_Guide')],
            [warp(3, 17, CITY, 3), warp(20, 4, 'MAP_VIRELIA_SLATEFORD_QUARRY_DEPTHS', 0)],
            [sign(17, 5, ep + 'EventScript_Equipment')], cave=True)
    # An abandoned loading crate near the ladder is readable from the room.
    entrance[5][17] = 0x3403
    write_grid(ROOT / 'data/layouts/Virelia_Slateford_QuarryEntrance/map.bin', entrance)

    depths = cave_grid(24, 22, [(2, 14, 8, 6), (5, 10, 14, 7), (16, 13, 6, 6),
                              (2, 7, 6, 5), (7, 8, 6, 4), (10, 5, 3, 9), (8, 2, 9, 4)])
    # A one-tile maintenance passage to the evidence chamber is initially blocked.
    rect(depths, 8, 6, 9, 1, 0x0619)
    depths[6][11] = 0x3603
    depths[7][10] = 0x0612
    depths[7][12] = 0x0610
    depths[19][3] = 0x3214
    for x, y in [(20, 14), (4, 9), (6, 17)]:
        depths[y][x] = 0x3403
    for x, y in [(6, 15), (8, 10), (18, 17), (15, 4)]:
        depths[y][x] = 0x0603
    ld = add_layout('QuarryDepths', depths, 'General', 'RusturfTunnel', (0x0611,) * 4)
    dp = PREFIX + 'QuarryDepths_'
    new_map('QuarryDepths', ld,
            [actor('WOMAN_2', 13, 12, dp + 'EventScript_Lenka', trainer=True),
             actor('MOVING_BOX', 12, 3, dp + 'EventScript_Evidence'),
             actor('MOVING_BOX', 17, 16, dp + 'EventScript_Crate')],
            [warp(3, 19, 'MAP_VIRELIA_SLATEFORD_QUARRY_ENTRANCE', 1)],
            [sign(20, 14, dp + 'EventScript_Pump'), sign(4, 9, dp + 'EventScript_Feed'),
             sign(6, 17, dp + 'EventScript_Instructions')], cave=True)

    gym = [[0x0623] * 13 for _ in range(20)]
    rect(gym, 1, 2, 11, 18, 0x3201)
    for x in range(1, 12):
        gym[0][x] = 0x0606
        gym[1][x] = 0x060e
    # Survey bays, visible strata and a single maintenance bridge to Mara.
    rect(gym, 1, 5, 4, 2, 0x0621)
    rect(gym, 7, 5, 5, 2, 0x0621)
    gym[5][5] = gym[6][5] = 0x0621
    gym[5][6] = 0x3201
    gym[6][6] = 0x0621
    for x, y, width, height in [(1, 5, 5, 2), (7, 5, 5, 2), (1, 8, 2, 3), (5, 9, 3, 2), (7, 13, 3, 2), (2, 14, 1, 2)]:
        rect(gym, x, y, width, height, 0x0621)
        for xx in range(x, x + width):
            gym[y + height - 1][xx] = 0x0628 if xx == x else 0x062a if xx == x + width - 1 else 0x0629
            gym[y][xx] = 0x0620 if xx == x else 0x0622 if xx == x + width - 1 else 0x0621
    gym[6][6] = 0x3629
    for y in range(2, 20):
        gym[y][12] = 0x0622
    for x, y in [(3, 11), (9, 8)]:
        gym[y][x] = 0x0626
    for x, y in [(2, 18), (10, 18)]:
        gym[y][x] = 0x0626
    gym[19][5], gym[19][6] = 0x3006, 0x3007
    # Leader dais and a clear return route through every training bay.
    rect(gym, 4, 2, 5, 2, 0x3203)
    lg = add_layout('Gym', gym, 'Building', 'RustboroGym', (0x0623,) * 4)
    gp = PREFIX + 'Gym_'
    new_map('Gym', lg,
            [actor('ROXANNE', 6, 2, gp + 'EventScript_Mara'),
             actor('HIKER', 4, 14, gp + 'EventScript_Ivan', trainer=True),
             actor('WOMAN_2', 9, 10, gp + 'EventScript_Dana', trainer=True),
             actor('MAN_2', 3, 17, gp + 'EventScript_Guide')],
            [warp(5, 19, CITY, 2), warp(6, 19, CITY, 2)],
            [sign(3, 11, gp + 'EventScript_OldLayer'), sign(9, 8, gp + 'EventScript_YoungLayer'),
             sign(2, 18, gp + 'EventScript_Statue'), sign(10, 18, gp + 'EventScript_Statue')])

    # Append five map IDs. Never reorder an old entry.
    group_path = ROOT / 'data/maps/map_groups.json'
    groups = json.loads(group_path.read_text(encoding='utf-8'))
    group = groups['gMapGroup_VireliaInteriors']
    for name in ['PokemonCenter', 'Mart', 'QuarryEntrance', 'QuarryDepths', 'Gym']:
        if PREFIX + name not in group:
            group.append(PREFIX + name)
    json_write(group_path, groups)
    json_write(REGISTRY, layouts_doc)

    # The source city survives intact; alter only the former empty service plots.
    city = grid_for('LAYOUT_VIRELIA_SLATEFORD_CITY')
    stock_city = grid_for('LAYOUT_RUSTBORO_CITY')
    rect(city, 3, 5, 7, 6, 0x3001)
    copy_rect(city, stock_city, 15, 35, 4, 4, 5, 7)  # Center door (6,10).
    rect(city, 3, 15, 7, 6, 0x3001)
    copy_rect(city, stock_city, 15, 42, 4, 4, 5, 17) # Mart door (6,20).
    city[11][6] = city[21][6] = 0x32bb
    city[15][25], city[16][25] = 0x049f, 0x00a7
    city[17][25] = 0x3121
    write_grid(ROOT / 'data/layouts/Virelia_SlatefordCity/map.bin', city)
    city_path = ROOT / 'data/maps/Virelia_SlatefordCity/map.json'
    entry = json.loads(city_path.read_text(encoding='utf-8'))
    entry['warp_events'] = [warp(6, 10, 'MAP_VIRELIA_SLATEFORD_POKEMON_CENTER', 0),
                            warp(6, 20, 'MAP_VIRELIA_SLATEFORD_MART', 0),
                            warp(22, 9, 'MAP_VIRELIA_SLATEFORD_GYM', 0),
                            warp(25, 16, 'MAP_VIRELIA_SLATEFORD_QUARRY_ENTRANCE', 0)]
    entry['bg_events'] = [event for event in entry['bg_events']
                          if event['script'] not in ('Virelia_SlatefordCity_EventScript_ClosedHouse',
                                                     'Virelia_SlatefordCity_EventScript_ClosedOffice')]
    if not any(obj['script'] == 'Virelia_SlatefordCity_EventScript_Manager' for obj in entry['object_events']):
        entry['object_events'].append(actor('EXPERT_M', 24, 18, 'Virelia_SlatefordCity_EventScript_Manager'))
    json_write(city_path, entry)
    print('Built five appended Slateford maps and updated only the city service plots.')


if __name__ == '__main__':
    main()

