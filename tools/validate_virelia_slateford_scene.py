#!/usr/bin/env python3
"""Check the finale's actual movement paths against city terrain and actors."""
import argparse
import copy
import json
from pathlib import Path
import re
import struct

ROOT = Path(__file__).resolve().parents[1]
PREFIX = 'Virelia_SlatefordCity_'
POLICEMAN = 'LOCALID_VIRELIA_SLATEFORD_POLICEMAN'
GROWLITHE = 'LOCALID_VIRELIA_SLATEFORD_GROWLITHE'
POLICE_FLAG = 'FLAG_VIRELIA_HIDE_POLICEMAN'


def validate(entry=None, script=None, *, quiet=False):
    if entry is None:
        entry = json.loads((ROOT / 'data/maps/Virelia_SlatefordCity/map.json').read_text())
    layouts = json.loads((ROOT / 'data/layouts/layouts.json').read_text())['layouts']
    layout = next(item for item in layouts if item['id'] == entry['layout'])
    width, height = layout['width'], layout['height']
    raw = (ROOT / layout['blockdata_filepath']).read_bytes()
    cells = struct.unpack(f'<{len(raw) // 2}H', raw)
    if script is None:
        script = (ROOT / 'data/maps/Virelia_SlatefordCity/scripts.inc').read_text(encoding='utf-8')
    objects = {obj.get('local_id', obj['script']): obj for obj in entry['object_events']}
    assert GROWLITHE in objects, 'Officer has no Growlithe companion'
    assert objects[GROWLITHE]['graphics_id'] == 'OBJ_EVENT_GFX_SPECIES(GROWLITHE)', 'Wrong police companion species'
    assert objects[GROWLITHE]['flag'] == objects[POLICEMAN]['flag'] == POLICE_FLAG, 'Police pair must share visibility'
    assert objects[GROWLITHE]['movement_type'] == 'MOVEMENT_TYPE_FACE_RIGHT', 'Growlithe must remain beside the officer'
    actors = {obj.get('local_id', obj['script']): (obj['x'], obj['y']) for obj in entry['object_events']}
    rocket = actors.pop('LOCALID_VIRELIA_SLATEFORD_ROCKET')
    policeman = actors.pop(POLICEMAN)
    growlithe = actors.pop(GROWLITHE)
    assert growlithe == (policeman[0] - 1, policeman[1]), 'Saved Growlithe position must trail the officer'
    occupied = set(actors.values())
    labels = re.findall(r'(?m)^([A-Za-z_][A-Za-z0-9_]*):{1,2}\s*\n', script)
    movements = {}
    for label in labels:
        if '_Movement_' not in label:
            continue
        body = re.search(r'(?m)^' + label + r':{1,2}\s*\n(.*?)(?=^[A-Za-z_]\w*:|\Z)', script, re.S | re.M)[1]
        movements[label.removeprefix(PREFIX + 'Movement_')] = re.findall(r'^\s*(\w+)\s*$', body, re.M)

    arrival = script.split(PREFIX + 'EventScript_ResolveRocket::', 1)[1].split('\n\tend', 1)[0]
    arrival = '\n'.join(line.split('@', 1)[0] for line in arrival.splitlines())

    def index(command):
        match = re.search(r'(?m)^\s*' + re.escape(command) + r'\s*$', arrival)
        assert match, f'Missing arrival command: {command}'
        return match.start()

    def spawn_position(actor):
        spawn = re.search(r'setobjectxyperm ' + actor + r', (\d+), (\d+)', arrival)
        assert spawn, f'{actor} must be positioned offscreen before spawning'
        assert index(spawn[0]) < index('addobject ' + actor)
        return tuple(map(int, spawn.groups()))

    start = spawn_position(POLICEMAN)
    growlithe_start = spawn_position(GROWLITHE)
    applied = re.findall(r'applymovement (\w+), ' + PREFIX + r'Movement_(\w+)', arrival)
    officer_moves = [name for actor, name in applied if actor == POLICEMAN]
    dog_moves = [name for actor, name in applied if actor == GROWLITHE]
    assert len(officer_moves) == 2 and len(dog_moves) == 1, 'Expected officer lead-in followed by paired arrival'
    lead_in, officer_route = officer_moves
    dog_route = dog_moves[0]
    lead_apply = index(f'applymovement {POLICEMAN}, {PREFIX}Movement_{lead_in}')
    lead_wait = index('waitmovement ' + POLICEMAN)
    officer_apply = index(f'applymovement {POLICEMAN}, {PREFIX}Movement_{officer_route}')
    dog_apply = index(f'applymovement {GROWLITHE}, {PREFIX}Movement_{dog_route}')
    officer_wait = arrival.rfind('waitmovement ' + POLICEMAN)
    dog_wait = index('waitmovement ' + GROWLITHE)
    assert index('lockall') < index('hidefollower') < index('addobject ' + POLICEMAN), 'Player follower must be hidden before police arrive'
    assert index('clearflag ' + POLICE_FLAG) < index('addobject ' + POLICEMAN) < lead_apply < lead_wait
    assert lead_wait < index('addobject ' + GROWLITHE) < min(officer_apply, dog_apply)
    assert max(officer_apply, dog_apply) < min(officer_wait, dog_wait), 'Police companions must walk together'
    for actor in (POLICEMAN, GROWLITHE):
        assert max(officer_wait, dog_wait) < index('copyobjectxytoperm ' + actor) < index('msgbox ' + PREFIX + 'Text_Policeman, MSGBOX_DEFAULT'), 'Persist both stopped positions before dialogue'

    directions = {'up': (0, -1), 'down': (0, 1), 'left': (-1, 0), 'right': (1, 0)}

    def walk(start, names, player, police=False):
        x, y = start
        path = [start]
        for name in names:
            for action in movements[name]:
                if action in ('step_end', 'face_player', 'face_right', 'delay_16'):
                    continue
                match = re.fullmatch(r'walk_(?:(fast)_)?(up|down|left|right)', action)
                assert match, f'Unexpected scene movement: {action}'
                assert not police or not match[1], 'Police companions must use normal walking speed'
                dx, dy = directions[match[2]]
                x, y = x + dx, y + dy
                assert 0 <= x < width and 0 <= y < height, 'Scene leaves the map before its exit'
                assert not cells[y * width + x] & 0xc00, f'{name} walks through solid terrain at {(x, y)}'
                assert (x, y) not in occupied | {player}, f'{name} walks through a person at {(x, y)}'
                path.append((x, y))
        return path

    # Derive legal A-button approaches from the authored worker position.
    # Rocks and terrain can change which sides remain available after mapping.
    approaches = []
    for dx, dy, facing in ((-1, 0, 'DIR_EAST'), (1, 0, 'DIR_WEST'),
                            (0, -1, 'DIR_SOUTH'), (0, 1, 'DIR_NORTH')):
        player = rocket[0] + dx, rocket[1] + dy
        if not (0 <= player[0] < width and 0 <= player[1] < height):
            continue
        if cells[player[1] * width + player[0]] & 0xc00 or player in occupied:
            continue
        opening = 'RocketAroundPlayer' if facing == 'DIR_EAST' else 'RocketToRoad'
        approaches.append((player, facing, opening))
    assert approaches, 'Worker has no legal interaction approach'
    for player, facing, opening in approaches:
        end = walk(rocket, (opening, 'RocketExit'), player)[-1]
        assert end[1] - player[1] >= 7, f'{facing}: grunt removed while still on camera'
        assert start[1] - player[1] >= 7, f'{facing}: policeman spawns visibly'
        assert growlithe_start[1] - player[1] >= 7, f'{facing}: Growlithe spawns visibly'
        lead = walk(start, (lead_in,), player, police=True)
        assert lead == [start, (start[0], start[1] - 1)], 'Officer must vacate one tile before releasing Growlithe'
        assert growlithe_start == start, 'Growlithe must start on the tile the officer just vacated'
        officer_path = walk(lead[-1], (officer_route,), player, police=True)
        dog_path = walk(growlithe_start, (dog_route,), player, police=True)
        assert len(officer_path) == len(dog_path), 'Police companions must keep the same walking cadence'
        for step in range(1, len(officer_path)):
            assert dog_path[step] == officer_path[step - 1], f'{facing}: Growlithe leaves the officer trail on step {step}'
            assert dog_path[step] != officer_path[step], f'{facing}: police companions overlap'
        assert officer_path[-1] == policeman, 'Officer stop does not match the saved map position'
        assert dog_path[-1] == growlithe, 'Growlithe stop does not match the saved map position'
        assert sum(abs(a - b) for a, b in zip(policeman, player)) <= 4
    assert 'face_player' in movements[officer_route], 'Officer must face the player on arrival'
    assert 'goto_if_eq VAR_0x8008, DIR_EAST, ' + PREFIX + 'EventScript_RocketDetour' in script
    assert re.search(r'EventScript_RocketGone:\s*waitmovement LOCALID_VIRELIA_SLATEFORD_ROCKET\s*'
                     r'@[^\n]*\n\s*removeobject LOCALID_VIRELIA_SLATEFORD_ROCKET', script)
    interaction = objects[GROWLITHE]['script']
    cry = re.search(r'(?m)^' + interaction + r'::\s*\n(.*?)(?=^[A-Za-z_]\w*:|\Z)', script, re.S | re.M)
    assert cry and 'playmoncry SPECIES_GROWLITHE, CRY_MODE_NORMAL' in cry[1], 'Growlithe interaction must play its cry'
    assert 'waitmoncry' in cry[1] and 'release' in cry[1], 'Growlithe interaction must return control'
    if not quiet:
        print(f'PASS: all {len(approaches)} legal Rocket approaches, offscreen police arrival, Growlithe follows every step and corner, shared visibility and persistent stops')


def self_test():
    entry = json.loads((ROOT / 'data/maps/Virelia_SlatefordCity/map.json').read_text())
    script = (ROOT / 'data/maps/Virelia_SlatefordCity/scripts.inc').read_text(encoding='utf-8')
    mutations = []
    for field, value in (('graphics_id', 'OBJ_EVENT_GFX_SPECIES(EKANS)'), ('flag', '0'), ('x', 20)):
        changed = copy.deepcopy(entry)
        next(obj for obj in changed['object_events'] if obj.get('local_id') == GROWLITHE)[field] = value
        mutations.append((field, changed, script))
    for command in ('hidefollower', 'waitmovement ' + GROWLITHE, 'copyobjectxytoperm ' + GROWLITHE):
        assert '\t' + command in script
        mutations.append((command, entry, script.replace('\t' + command, '\t@ removed ' + command, 1)))
    for name in ('RocketAroundPlayer', 'RocketToRoad'):
        label = PREFIX + 'Movement_' + name + ':\n'
        before, body = script.split(label, 1)
        mutations.append((name, entry, before + label + body.replace('\twalk_fast_left\n', '', 1)))
    dog_route = re.search(r'applymovement ' + GROWLITHE + r', (\w+)', script)[1]
    label = dog_route + ':\n'
    assert label in script
    before, body = script.split(label, 1)
    for action in ('walk_fast_up', 'walk_right'):
        mutations.append((action, entry, before + label + body.replace('walk_up', action, 1)))
    for name, changed_entry, changed_script in mutations:
        try:
            validate(changed_entry, changed_script, quiet=True)
        except AssertionError:
            continue
        raise AssertionError(f'Validator accepted invalid mutation: {name}')
    print(f'PASS: {len(mutations)} invalid scene mutations rejected')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--self-test', action='store_true')
    args = parser.parse_args()
    validate()
    if args.self_test:
        self_test()
