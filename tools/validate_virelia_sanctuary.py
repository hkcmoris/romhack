#!/usr/bin/env python3
"""Validate sanctuary gates, walking paths, native assets and script registration.

Native tests exercise the compiled puzzle, map callbacks, bag handling and saves.
This graph check proves the approaches and shortcut cannot bypass their gates.
"""
from collections import deque
from pathlib import Path
import json
import struct
import sys

ROOT = Path(__file__).resolve().parents[1]
ROUTE = 'Virelia_Route01'
SHRINE = 'Virelia_WhisperwoodSanctuary'


def require(condition, message):
    if not condition:
        raise ValueError(message)


def load(path):
    return json.loads((ROOT / path).read_text(encoding='utf-8-sig'))


def validate():
    groups = load('data/maps/map_groups.json')
    require(groups['gMapGroup_VireliaOverworld'][3] == SHRINE, 'Sanctuary must be appended as outdoor map 75,3')
    require(SHRINE not in groups['gMapGroup_VireliaInteriors'], 'Outdoor sanctuary must not be in the interiors group')
    registry = load('data/layouts/layouts.json')['layouts']
    maps = {name: load(f'data/maps/{name}/map.json') for name in (ROUTE, SHRINE)}
    layouts = {name: next(x for x in registry if x['id'] == maps[name]['layout']) for name in maps}
    require((layouts[ROUTE]['width'], layouts[ROUTE]['height']) == (30, 42), 'Route dimensions changed')
    require((layouts[SHRINE]['width'], layouts[SHRINE]['height']) == (22, 26), 'Sanctuary dimensions changed')
    require(maps[SHRINE]['region_map_section'] == maps[ROUTE]['region_map_section'], 'Sanctuary must share Whisperwood section')
    require(dict(map='MAP_VIRELIA_WHISPERWOOD_SANCTUARY', offset=6, direction='left') in maps[ROUTE]['connections'], 'Missing westward route connection')
    require(maps[SHRINE]['connections'] == [dict(map='MAP_VIRELIA_ROUTE01', offset=-6, direction='right')], 'Incorrect sanctuary return connection')
    blocks = {}
    for name, layout in layouts.items():
        raw = (ROOT / layout['blockdata_filepath']).read_bytes()
        require(len(raw) == layout['width'] * layout['height'] * 2, f'{name}: invalid block count')
        blocks[name] = struct.unpack(f'<{len(raw)//2}H', raw)
    gates = {
        1: {(ROUTE, 1, 16), (ROUTE, 1, 17), (SHRINE, 20, 10), (SHRINE, 20, 11)},
        2: {(ROUTE, 1, 8), (ROUTE, 1, 9), (SHRINE, 16, 2), (SHRINE, 16, 3)},
    }
    occupied = {(name, obj['x'], obj['y']) for name in maps for obj in maps[name]['object_events']}

    def tile(node):
        name, x, y = node
        width, height = layouts[name]['width'], layouts[name]['height']
        return blocks[name][y * width + x] if 0 <= x < width and 0 <= y < height else None

    def reached(state, start):
        def walkable(node):
            value = tile(node)
            return value is not None and node not in occupied and (not value & 0xC00 or any(node in points and state >= gate for gate, points in gates.items()))
        require(walkable(start), f'Blocked start: {start}')
        seen, queue = {start}, deque([start])
        while queue:
            node = queue.popleft()
            name, x, y = node
            neighbors = [(name, x + dx, y + dy) for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1))]
            if name == ROUTE and x == 0 and 6 <= y < 32:
                neighbors.append((SHRINE, 21, y - 6))
            if name == SHRINE and x == 21:
                neighbors.append((ROUTE, 0, y + 6))
            for target in neighbors:
                if not walkable(target) or target in seen:
                    continue
                a, b = tile(node) >> 12, tile(target) >> 12
                if a == b or 0 in (a, b):
                    seen.add(target)
                    queue.append(target)
        return seen

    locked = reached(0, (ROUTE, 4, 16))
    require(not any(node[0] == SHRINE for node in locked), 'Sanctuary accessible before helping Hana')
    approach = reached(1, (ROUTE, 4, 16))
    require((SHRINE, 9, 10) in approach, 'Ancient tree cannot be reached after helping Hana')
    require((SHRINE, 19, 2) not in approach, 'Shortcut gate can be bypassed before solving stones')
    for event in maps[SHRINE]['bg_events']:
        if event['script'].endswith('ShortcutRoots'):
            continue
        require(any((SHRINE, event['x'] + dx, event['y'] + dy) in approach
                    for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1))), f'Unreachable interaction: {event}')
    opened = reached(2, (SHRINE, 9, 10))
    require({(SHRINE, 21, 2), (ROUTE, 0, 8), (ROUTE, 4, 8), (ROUTE, 4, 16)} <= opened, 'Solved shortcut or original exit cannot be used')
    require((SHRINE, 9, 10) in reached(2, (ROUTE, 4, 8)), 'Shortcut cannot be used in reverse')
    # Seam cells remain passable even while the inward gates are closed; native
    # connection buffers copy these cells before the neighbor's callback runs.
    for y in (8, 9, 16, 17):
        require(not tile((ROUTE, 0, y)) & 0xC00, 'Route seam is blocked in raw neighbor data')
        require(not tile((SHRINE, 21, y - 6)) & 0xC00, 'Sanctuary seam is blocked in raw neighbor data')
    for points in gates.values():
        # Neighbor buffers can read the authored layout before its load script
        # runs. Collision must seal each gate; the callback supplies its art.
        require(all(tile(point) & 0xC00 for point in points), 'Gate must be closed in the saved layout')
    path = ROOT / 'data/tilesets/secondary/whisperwood'
    raw = (path / 'metatiles.bin').read_bytes()
    attrs = (path / 'metatile_attributes.bin').read_bytes()
    require(len(raw) == 310 * 16 and len(attrs) == 310 * 2, 'Incorrect sanctuary metatile tables')
    prop_attrs = struct.unpack('<30H', attrs[-60:])
    require(prop_attrs == (0x1000, *([0] * 29)),
            'Pawprints must use covered ground layers; stones/tree keep normal layers and behavior')
    require(struct.unpack('>II', (path / 'tiles.png').read_bytes()[16:24]) == (128, 256), 'Invalid native tiles image size')
    require(struct.unpack('>II', (path / 'old_tree.png').read_bytes()[16:24]) == (80, 80), 'Old tree must be exactly 5x5 16px map tiles')
    for dy in range(5):
        for dx in range(5):
            expected = 0x31D + dy * 5 + dx
            value = tile((SHRINE, 7 + dx, 5 + dy))
            require(value & 0x3FF == expected and value & 0xC00, 'Custom old-tree footprint is incomplete')
            pieces = struct.unpack_from('<8H', raw, (expected - 512) * 16)
            require(all(value >> 12 == 6 for value in pieces[4:]), 'Old-tree tile must use its dedicated palette')
    require((path / 'palettes/06.pal').is_file() and (path / 'palettes/15.pal').is_file(), 'Missing old-tree day/night palettes')
    scripts = (ROOT / 'data/event_scripts.s').read_text(encoding='utf-8')
    for name in ('data/scripts/virelia_sanctuary.inc', f'data/maps/{SHRINE}/scripts.inc'):
        require(f'.include "{name}"' in scripts, f'Unregistered script: {name}')
    require('#include "constants/virelia_sanctuary.h"' in scripts, 'Missing event-state constants')
    for name in (ROUTE, SHRINE):
        map_scripts = (ROOT / f'data/maps/{name}/scripts.inc').read_text(encoding='utf-8')
        require('callnative VireliaSanctuary_UpdateConnectionGates' in map_scripts,
                f'{name}: connected camera buffers must restore the same quest gates')
    print('PASS: Hana gate, overgrown approach, all shrine interactions, sealed and open two-way shortcut')
    print('PASS: inward gates preserve native connection buffers; appended map and native forest assets registered')


if __name__ == '__main__':
    try:
        validate()
    except (ValueError, KeyError, OSError) as error:
        print(f'FAIL: {error}', file=sys.stderr)
        sys.exit(1)
