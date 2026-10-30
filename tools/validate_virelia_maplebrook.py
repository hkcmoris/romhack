#!/usr/bin/env python3
"""Audit Maplebrook's Cut pocket and Slateford's first-HM delivery scene."""
import argparse
from collections import deque
import copy
import json
from pathlib import Path
import re
import struct

ROOT = Path(__file__).resolve().parents[1]
TOWN = 'Virelia_MaplebrookTown'
CITY = 'Virelia_SlatefordCity'
CUT_TREE = 'LOCALID_VIRELIA_MAPLEBROOK_CUT_TREE'
AIDE = 'LOCALID_VIRELIA_SLATEFORD_CUT_AIDE'
ITEM_FLAG = 'FLAG_VIRELIA_MAPLEBROOK_WOODLAND_ITEM'
DELTAS = ((1, 0), (-1, 0), (0, 1), (0, -1))


def require(condition, message):
    if not condition:
        raise ValueError(message)


def load(path):
    return json.loads((ROOT / path).read_text(encoding='utf-8'))


def words(path):
    raw = (ROOT / path).read_bytes()
    return list(struct.unpack(f'<{len(raw) // 2}H', raw))


def block(source, label):
    found = re.search(r'^' + re.escape(label) + r'::?\s*\n(.*?)(?=^\w+:|\Z)', source, re.M | re.S)
    require(found is not None, f'Missing scene label: {label}')
    return '\n'.join(line.split('@', 1)[0] for line in found[1].splitlines())


def validate(town=None, cells=None, city=None, scripts=None, quiet=False):
    town = load(f'data/maps/{TOWN}/map.json') if town is None else town
    city = load(f'data/maps/{CITY}/map.json') if city is None else city
    scripts = (ROOT / f'data/maps/{CITY}/scripts.inc').read_text(encoding='utf-8') if scripts is None else scripts
    layouts = {l['id']: l for l in load('data/layouts/layouts.json')['layouts']}
    layout = layouts[town['layout']]
    width, height = layout['width'], layout['height']
    cells = words(layout['blockdata_filepath']) if cells is None else cells
    attrs = words('data/tilesets/primary/general/metatile_attributes.bin')
    attrs += words('data/tilesets/secondary/virelia_outdoor/metatile_attributes.bin')
    behaviors = re.findall(r'\b(MB_[A-Z0-9_]+)\s*,', (ROOT / 'include/constants/metatile_behaviors.h').read_text())
    water = {'MB_POND_WATER', 'MB_OCEAN_WATER', 'MB_DEEP_WATER', 'MB_INTERIOR_DEEP_WATER',
             'MB_SOOTOPOLIS_DEEP_WATER', 'MB_EASTWARD_CURRENT', 'MB_WESTWARD_CURRENT',
             'MB_NORTHWARD_CURRENT', 'MB_SOUTHWARD_CURRENT'}
    objects = {o.get('local_id', o['script']): o for o in town['object_events']}
    constants = (ROOT / 'include/constants/virelia_maplebrook.h').read_text()

    def coordinate(name):
        return int(re.search(r'^#define\s+VIRELIA_MAPLEBROOK_' + name + r'\s+(\d+)', constants, re.M)[1])

    cut_x, cut_y = coordinate('CUT_TREE_X'), coordinate('CUT_TREE_Y')
    item_x, item_y = coordinate('WOODLAND_ITEM_X'), coordinate('WOODLAND_ITEM_Y')
    require(CUT_TREE in objects, 'Missing native cuttable tree')
    tree = objects[CUT_TREE]
    require((tree['x'], tree['y']) == (cut_x, cut_y) and tree['elevation'] == 3,
            'Cut tree must block the one-tile woodland passage')
    require(tree['graphics_id'] == 'OBJ_EVENT_GFX_CUTTABLE_TREE'
            and tree['flag'] == 'FLAG_TEMP_11' and tree['movement_range_x'] == tree['movement_range_y'] == 0,
            'Cut tree must use the native stationary, map-local respawning object')
    require(tree['script'] == 'Virelia_MaplebrookTown_EventScript_CutTree', 'Tree interaction bypasses Cut checks')
    item = [e for e in town['bg_events'] if e.get('flag') == ITEM_FLAG]
    require(len(item) == 1, 'Expected one independent woodland reward')
    item = item[0]
    require(item['type'] == 'hidden_item' and item['item'] == 'ITEM_MIRACLE_SEED'
            and (item['x'], item['y']) == (item_x, item_y), 'Woodland reward must remain hidden at the dead end')

    def tile(x, y):
        return cells[y * width + x] if 0 <= x < width and 0 <= y < height else None

    def behavior(x, y):
        value = tile(x, y)
        return behaviors[attrs[value & 1023] & 255] if value is not None else None

    def reached(cut=False, surf=False):
        occupied = {(o['x'], o['y']) for name, o in objects.items() if name != CUT_TREE or not cut}

        def walkable(x, y):
            value = tile(x, y)
            return value is not None and not value & 0xC00 and (x, y) not in occupied \
                and behavior(x, y) != 'MB_WATERFALL' and (surf or behavior(x, y) not in water)

        start = (9, 47)
        require(walkable(*start), 'Southern arrival is blocked')
        seen, queue = {start}, deque([start])
        while queue:
            x, y = queue.popleft()
            for dx, dy in DELTAS:
                target = x + dx, y + dy
                if target in seen or not walkable(*target):
                    continue
                a, b = tile(x, y) >> 12, tile(*target) >> 12
                if a == b or {0, 15} & {a, b} or surf and (behavior(x, y) in water or behavior(*target) in water):
                    seen.add(target)
                    queue.append(target)
        return seen

    locked, cut_open, surf_open = reached(), reached(cut=True), reached(surf=True)
    approaches = {(item['x'] + dx, item['y'] + dy) for dx, dy in DELTAS}
    require(not locked & approaches, 'Woodland item can be reached without Cut or Surf')
    require(bool(cut_open & approaches), 'Cut does not open a two-way route to the hidden item')
    require(bool(surf_open & approaches), 'Future Surf landing cannot reach the woodland pocket')
    require((cut_x - 1, cut_y) in locked and (cut_x + 1, cut_y) not in locked,
            'Cut tree cannot be approached from town or has a bypass')
    for warp in town['warp_events']:
        require((warp['x'], warp['y'] + 1) in locked, 'Woodland blocks an existing house or lab')
    require({(14, 0), (15, 0)} <= locked, 'Woodland blocks the north trail')
    require(all(tile(x, y) & 0xC00 for x in range(12, 36) for y in (46, 47)),
            'Southern woodland boundary has an opening')
    require(all(tile(cut_x, y) & 0xC00 for y in (cut_y - 1, cut_y + 1)),
            'Woodland passage must stay one tile wide')
    native_doors = {(x, y) for y in range(height) for x in range(width)
                    if behavior(x, y) == 'MB_ANIMATED_DOOR'}
    require(native_doors == {(w['x'], w['y']) for w in town['warp_events']},
            'A Maplebrook door has no registered interior, or a warp has left its door')
    ema = [w for w in town['warp_events'] if w['dest_map'] == 'MAP_VIRELIA_MAPLEBROOK_EMAS_HOUSE']
    require(len(ema) == 1 and (ema[0]['x'], ema[0]['y']) == (30, 20), 'Ema house must use the last town doorway')
    stairs = {}
    for x, first, last in ((14, 3, 4), (30, 7, 8), (14, 13, 17), (13, 26, 29)):
        for y in range(first, last + 1):
            stairs[x, y], stairs[x + 1, y] = 0x30AF, 0x30CF
    bridges = {(x, y): 0x3000 | (0x363 if x == 13 else 0x364)
               for x in (13, 14) for y in range(20, 24)}
    bridges.update({(x, y): (0xF000 if x in (18, 19) else 0x3000) | (0x2B3 if y == 32 else 0x2B4)
                    for x in range(16, 22) for y in (32, 33)})
    require(all(tile(x, y) == value for (x, y), value in {**stairs, **bridges}.items()),
            'Existing staircase or bridge was changed')

    aide = next((o for o in city['object_events'] if o.get('local_id') == AIDE), None)
    require(aide is not None and aide['graphics_id'] == 'OBJ_EVENT_GFX_SCIENTIST_1'
            and aide['flag'] == 'FLAG_VIRELIA_HIDE_CUT_AIDE' and (aide['x'], aide['y']) == (23, 10),
            'Missing stationary professor aide for delivery or full-bag retry')
    require('callnative VireliaFirstGym_PrepareCutDelivery' in scripts
            and 'map_script_2 VAR_VIRELIA_CUT_DELIVERY, 1, Virelia_SlatefordCity_EventScript_CutDelivery' in scripts,
            'Gym-exit delivery is not registered with native map callbacks')
    scene = block(scripts, 'Virelia_SlatefordCity_EventScript_CutDelivery')
    require('setvar VAR_VIRELIA_CUT_DELIVERY, 0' in scene, 'Exit scene may replay every frame')
    require(scene.index('lockall') < scene.index('hidefollower') < scene.index('applymovement'),
            'Aide must lock controls and hide followers before walking')
    require(scene.index('waitmovement') < scene.index('copyobjectxytoperm') < scene.index('EventScript_GiveCut'),
            'Aide must reach and persist the retry position before dialogue')
    require('goto_if_unset FLAG_VIRELIA_HM_CUT_RECEIVED' in scene, 'Full Bag must leave the aide available')
    movement = block(scripts, 'Virelia_SlatefordCity_Movement_CutAideApproach')
    actions = re.findall(r'^\s*(walk_\w+)\s*$', movement, re.M)
    require(actions == ['walk_fast_left'] * 3, 'Aide should approach and stop beside the gym exit')
    city_layout = layouts[city['layout']]
    city_cells = words(city_layout['blockdata_filepath'])
    for x in range(23, 27):
        value = city_cells[10 * city_layout['width'] + x]
        require(not value & 0xC00 and value >> 12 == 3, 'Aide approach crosses blocked terrain or an elevation change')
        require(not any(o['x'] == x and o['y'] == 10 and o.get('local_id') != AIDE for o in city['object_events']),
                'Aide approach collides with another resident')
    departure = block(scripts, 'Virelia_SlatefordCity_Movement_CutAideLeave')
    actions = re.findall(r'^\s*(walk_\w+)\s*$', departure, re.M)
    require(actions == ['walk_fast_right'] * 3 + ['walk_fast_up'] * 8,
            'Aide must leave the camera before being removed')
    for y in range(2, 10):
        value = city_cells[y * city_layout['width'] + 26]
        require(not value & 0xC00 and value >> 12 == 3, 'Aide departure crosses blocked terrain')
        require(not any(o['x'] == 26 and o['y'] == y for o in city['object_events']),
                'Aide departure collides with another resident')
    town_script = (ROOT / f'data/maps/{TOWN}/scripts.inc').read_text(encoding='utf-8')
    cut_script = block(town_script, tree['script'])
    require('checkfieldmove FIELD_MOVE_CUT, TRUE' in cut_script and 'goto EventScript_CutTreeCommon' in cut_script,
            'Woodland tree must check badges and learned moves before native Cut removal')
    if not quiet:
        print('PASS: L-shaped woodland, Cut-only footpath, hidden Miracle Seed, future Surf landing and all 46 original stair/bridge cells')
        print('PASS: one-shot gym-exit aide approach, safe full-Bag retry position and native Cut interaction')


def self_test():
    town = load(f'data/maps/{TOWN}/map.json')
    cells = words('data/layouts/Virelia_MaplebrookTown/map.bin')
    script = (ROOT / f'data/maps/{CITY}/scripts.inc').read_text(encoding='utf-8')
    mutations = []
    changed = list(cells)
    for x in (26, 27):
        changed[42 * 36 + x] = 0x3001
    mutations.append(dict(cells=changed))
    changed = list(cells)
    changed[20 * 36 + 13] = 0x3001
    mutations.append(dict(cells=changed))
    for field, value in (('x', 21), ('flag', '0'), ('graphics_id', 'OBJ_EVENT_GFX_MAN_1')):
        changed = copy.deepcopy(town)
        next(o for o in changed['object_events'] if o.get('local_id') == CUT_TREE)[field] = value
        mutations.append(dict(town=changed))
    changed = copy.deepcopy(town)
    next(e for e in changed['bg_events'] if e.get('flag') == ITEM_FLAG)['item'] = 'ITEM_POTION'
    mutations.append(dict(town=changed))
    for command in ('setvar VAR_VIRELIA_CUT_DELIVERY, 0', 'hidefollower', 'copyobjectxytoperm ' + AIDE):
        mutations.append(dict(scripts=script.replace('\t' + command, '\t@ removed ' + command, 1)))
    for index, mutation in enumerate(mutations):
        try:
            validate(**mutation, quiet=True)
        except (ValueError, KeyError):
            continue
        raise ValueError(f'Invalid woodland/scene mutation accepted: {index}')
    print(f'PASS: {len(mutations)} broken gate, reward, bridge and scene mutations rejected')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--self-test', action='store_true')
    args = parser.parse_args()
    validate()
    if args.self_test:
        self_test()
