#!/usr/bin/env python3
"""Check the gallery loop with real collisions, object occupancy and warps."""
from collections import deque
import json
from pathlib import Path
import struct

ROOT = Path(__file__).resolve().parents[1]
ENTRANCE = 'Virelia_Slateford_QuarryEntrance'
DEPTHS = 'Virelia_Slateford_QuarryDepths'
GALLERY = 'Virelia_Slateford_QuarryGallery'


def require(condition, message):
    if not condition:
        raise ValueError(message)


def load(path):
    return json.loads((ROOT / path).read_text(encoding='utf-8'))


def validate():
    maps = {name: load(f'data/maps/{name}/map.json') for name in (ENTRANCE, DEPTHS, GALLERY)}
    layouts = {item['id']: item for item in load('data/layouts/layouts.json')['layouts']}
    ids = {entry['id']: name for name, entry in maps.items()}
    cells = {}
    occupied = set()
    for name, entry in maps.items():
        layout = layouts[entry['layout']]
        raw = (ROOT / layout['blockdata_filepath']).read_bytes()
        require(len(raw) == layout['width'] * layout['height'] * 2, f'{name}: bad dimensions')
        cells[name] = struct.unpack(f'<{len(raw) // 2}H', raw)
        occupied.update((name, obj['x'], obj['y']) for obj in entry['object_events'])
    gates = {(ENTRANCE, 4, 3), (DEPTHS, 3, 7)}

    def block(node):
        name, x, y = node
        layout = layouts[maps[name]['layout']]
        return cells[name][y * layout['width'] + x] if 0 <= x < layout['width'] and 0 <= y < layout['height'] else None

    def reached(start, powered, original_ladder=True):
        def walkable(node):
            value = block(node)
            return value is not None and not value & 0xC00 and node not in occupied and (powered or node not in gates)

        require(walkable(start), f'Blocked arrival: {start}')
        seen, queue = {start}, deque([start])
        while queue:
            node = queue.popleft()
            name, x, y = node
            # A ladder warps as soon as it is stepped on. Do not allow the
            # graph to walk across one to reach an otherwise isolated room.
            warps = [warp for warp in maps[name]['warp_events'] if (warp['x'], warp['y']) == (x, y)]
            if warps:
                neighbors = []
                for warp in warps:
                    if warp['dest_map'] not in ids:
                        continue
                    other = ids[warp['dest_map']]
                    if not original_ladder and GALLERY not in (name, other):
                        continue
                    target = maps[other]['warp_events'][int(warp['dest_warp_id'])]
                    require(target['dest_map'] == maps[name]['id'], 'Nonreciprocal gallery warp')
                    # Arrival tile must let the player step off the ladder.
                    neighbors.extend((other, target['x'] + dx, target['y'] + dy)
                                     for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)))
            else:
                neighbors = [(name, x + dx, y + dy) for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1))]
            for target in neighbors:
                if target in seen or not walkable(target):
                    continue
                if target[0] == name:
                    a, b = block(node) >> 12, block(target) >> 12
                    if a != b and 0 not in (a, b):
                        continue
                seen.add(target)
                queue.append(target)
        return seen

    start = ENTRANCE, 4, 16
    locked = reached(start, False)
    require(not any(node[0] == GALLERY for node in locked), 'Gallery bypasses power restoration')
    opened = reached(start, True)
    # Remove the old inter-section ladder. The gallery alone must connect
    # both work areas, in both directions, with Sableye still occupying its tile.
    shortcut = reached(start, True, original_ladder=False)
    target = DEPTHS, 4, 10
    require(target in shortcut, 'Gallery is a dead end rather than an alternate route')
    require(start in reached(target, True, original_ladder=False), 'Gallery has no return route')
    for name in maps:
        for obj in maps[name]['object_events']:
            if name != GALLERY and not obj['script'].endswith('Wrench'):
                continue
            require(any((name, obj['x'] + dx, obj['y'] + dy) in opened
                        for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1))),
                    f'Cannot approach {obj["script"]}')
    gallery = (ROOT / f'data/maps/{GALLERY}/scripts.inc').read_text(encoding='utf-8')
    for helper in ('BeginMissingTools', 'CollectTool', 'RecoverTools', 'RewardTools'):
        require(f'callnative VireliaFirstGym_{helper}' in gallery, f'Missing compiled helper: {helper}')
    require('setwildbattle SPECIES_SABLEYE, 13' in gallery and 'dowildbattle' in gallery,
            'Sableye must be an ordinary catchable wild encounter')
    require('dynmultichoice' in gallery and all(f'Text_{name}' in gallery for name in ('Fight', 'Catch', 'Later')),
            'Encounter needs fight, catch and cancel choices')
    require(maps[GALLERY]['object_events'][0]['flag'] == 'FLAG_VIRELIA_MISSING_TOOLS_RECOVERED',
            'Resolved encounter must stay hidden')
    for name, x, y in ((ENTRANCE, 4, 3), (DEPTHS, 3, 7)):
        script = (ROOT / f'data/maps/{name}/scripts.inc').read_text(encoding='utf-8')
        require(f'setmetatile {x}, {y}, 0x203, 1' in script
                and f'setmetatile {x}, {y}, 0x214, 0' in script, 'Missing persistent ladder gate')
    print('PASS: powered two-way gallery loop, reachable clues/Sableye, native battle and persistent gates')


if __name__ == '__main__':
    validate()
