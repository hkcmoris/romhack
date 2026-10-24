#!/usr/bin/env python3
"""Check the finale's actual movement paths against city terrain and actors."""
import json
from pathlib import Path
import re
import struct

ROOT = Path(__file__).resolve().parents[1]
PREFIX = 'Virelia_SlatefordCity_'


def validate():
    entry = json.loads((ROOT / 'data/maps/Virelia_SlatefordCity/map.json').read_text())
    layouts = json.loads((ROOT / 'data/layouts/layouts.json').read_text())['layouts']
    layout = next(item for item in layouts if item['id'] == entry['layout'])
    width, height = layout['width'], layout['height']
    raw = (ROOT / layout['blockdata_filepath']).read_bytes()
    cells = struct.unpack(f'<{len(raw) // 2}H', raw)
    script = (ROOT / 'data/maps/Virelia_SlatefordCity/scripts.inc').read_text(encoding='utf-8')
    actors = {obj.get('local_id', obj['script']): (obj['x'], obj['y']) for obj in entry['object_events']}
    rocket = actors.pop('LOCALID_VIRELIA_SLATEFORD_ROCKET')
    policeman = actors.pop('LOCALID_VIRELIA_SLATEFORD_POLICEMAN')
    occupied = set(actors.values())
    labels = re.findall(r'(?m)^([A-Za-z_][A-Za-z0-9_]*):{1,2}\s*\n', script)
    movements = {}
    for label in labels:
        if '_Movement_' not in label:
            continue
        body = re.search(r'(?m)^' + label + r':\s*\n(.*?)(?=^[A-Za-z_]\w*:|\Z)', script, re.S | re.M)[1]
        movements[label.removeprefix(PREFIX + 'Movement_')] = re.findall(r'^\s*(\w+)\s*$', body, re.M)

    spawn = re.search(r'setobjectxyperm LOCALID_VIRELIA_SLATEFORD_POLICEMAN, (\d+), (\d+)', script)
    start = tuple(map(int, spawn.groups()))
    directions = {'up': (0, -1), 'down': (0, 1), 'left': (-1, 0), 'right': (1, 0)}

    def walk(start, names, player, police=False):
        x, y = start
        for name in names:
            for action in movements[name]:
                if action in ('step_end', 'face_player', 'delay_16'):
                    continue
                match = re.fullmatch(r'walk_(?:(fast)_)?(up|down|left|right)', action)
                assert match, f'Unexpected scene movement: {action}'
                assert not police or not match[1], 'Policeman must use normal walking speed'
                dx, dy = directions[match[2]]
                x, y = x + dx, y + dy
                assert 0 <= x < width and 0 <= y < height, 'Scene leaves the map before its exit'
                assert not cells[y * width + x] & 0xc00, f'{name} walks through solid terrain at {(x, y)}'
                assert (x, y) not in occupied | {player}, f'{name} walks through a person at {(x, y)}'
        return x, y

    # These are all legal A-button approaches to the worker. The fourth side
    # is occupied by a quarry rock and cannot be used by ordinary gameplay.
    for player, facing, opening in (((21, 22), 'DIR_EAST', 'RocketAroundPlayer'),
                                    ((23, 22), 'DIR_WEST', 'RocketToRoad'),
                                    ((22, 21), 'DIR_SOUTH', 'RocketToRoad')):
        assert not cells[player[1] * width + player[0]] & 0xc00 and player not in occupied
        end = walk(rocket, (opening, 'RocketExit'), player)
        assert end[1] - player[1] >= 7, f'{facing}: grunt removed while still on camera'
        assert start[1] - player[1] >= 7, f'{facing}: policeman spawns visibly'
        assert walk(start, ('Policeman',), player, police=True) == policeman
        assert sum(abs(a - b) for a, b in zip(policeman, player)) <= 3
    assert 'face_player' in movements['Policeman'], 'Officer must face the player on arrival'
    assert 'goto_if_eq VAR_0x8008, DIR_EAST, ' + PREFIX + 'EventScript_RocketDetour' in script
    assert re.search(r'EventScript_RocketGone:\s*waitmovement LOCALID_VIRELIA_SLATEFORD_ROCKET\s*'
                     r'@[^\n]*\n\s*removeobject LOCALID_VIRELIA_SLATEFORD_ROCKET', script)
    arrival = script.split(PREFIX + 'EventScript_ResolveRocket::', 1)[1].split('\n\tend', 1)[0]
    assert arrival.index('setobjectxyperm') < arrival.index('clearflag') < arrival.index('addobject')
    assert 'copyobjectxytoperm LOCALID_VIRELIA_SLATEFORD_POLICEMAN' in arrival
    print('PASS: all three Rocket approaches, obstacle-free offscreen exit, normal-speed police entrance and persistent stop')


if __name__ == '__main__':
    validate()
