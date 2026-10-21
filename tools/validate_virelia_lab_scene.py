#!/usr/bin/env python3
"""Interpret the lab's actual event scripts and check every starter-scene route.

This deliberately small interpreter checks overworld choreography, not the battle
engine. Native gift/reward calls use their documented postconditions; their save
semantics are covered by test/virelia_prologue.c. Unsupported scene instructions
fail explicitly rather than silently making an unverified route pass.
"""
import argparse
from collections import deque
from dataclasses import dataclass
import json
from pathlib import Path
import re
import struct
import sys

ROOT = Path(__file__).resolve().parents[1]
MAP = "Virelia_MaplebrookResearchLaboratory"
PREFIX = MAP + "_EventScript_"
PLAYER = "LOCALID_PLAYER"
VIKTOR = "LOCALID_VIRELIA_LAB_VIKTOR"
CHOICES = ("GRASS", "WATER", "FIRE")
COUNTER = {"GRASS": "FIRE", "WATER": "GRASS", "FIRE": "WATER"}
DIRECTIONS = {"DIR_SOUTH": (0, 1), "DIR_NORTH": (0, -1),
              "DIR_WEST": (-1, 0), "DIR_EAST": (1, 0)}
ACTION_DIRECTIONS = {"down": "DIR_SOUTH", "up": "DIR_NORTH",
                     "left": "DIR_WEST", "right": "DIR_EAST"}
STARTER_FLAG = "FLAG_VIRELIA_STARTER_RECEIVED"
RIVAL_FLAG = "FLAG_VIRELIA_RIVAL_COMPLETE"


class SceneError(ValueError):
    pass


def require(condition, message):
    if not condition:
        raise SceneError(message)


def validate_battle_macro(source):
    """Ensure the optional actor survives expansion into the native command."""
    macro = re.search(r"^\s*\.macro\s+trainerbattle_earlyrival\s+([^\n]+)\n(.*?)^\s*\.endm", source, re.M | re.S)
    require(macro is not None, "missing trainerbattle_earlyrival macro")
    parameters = [parameter.strip() for parameter in macro[1].split(",")]
    require(len(parameters) == 5, "trainerbattle_earlyrival must accept a fifth battle actor argument")
    local_id = re.fullmatch(r"(\w+)\s*=\s*OBJ_ID_NONE", parameters[4])
    require(local_id is not None, "rival battle actor must remain optional with OBJ_ID_NONE default")
    expansion = re.search(r"^\s*trainerbattle\s+([^\n]+)", macro[2], re.M)
    require(expansion is not None, "rival macro does not invoke trainerbattle")
    arguments = [argument.strip() for argument in expansion[1].split(",")]
    require(arguments[0] == "\\" + local_id[1], "rival macro does not forward its battle actor to trainerbattle")


@dataclass(frozen=True)
class Instruction:
    command: str
    args: tuple
    location: str


class Program:
    def __init__(self, root, script_source=None):
        validate_battle_macro((root / "asm/macros/event.inc").read_text(encoding="utf-8"))
        self.labels = {}
        self.code = []
        paths = [f"data/maps/{MAP}/scripts.inc", "data/scripts/virelia_prologue.inc"]
        for index, path in enumerate(paths):
            source = script_source if index == 0 and script_source is not None else (root / path).read_text(encoding="utf-8")
            self.parse(source, path)

    def parse(self, source, path):
        active = True
        conditionals = []
        configuration = {"VIRELIA_PROLOGUE": True, "VIRELIA_DEVELOPMENT_MODE": False, "IS_FRLG": False}
        for number, raw in enumerate(source.splitlines(), 1):
            line = raw.split("@", 1)[0].strip()
            location = f"{path}:{number}"
            if line.startswith("#if "):
                terms = line[4:].strip().split("&&")
                values = []
                for term in terms:
                    token = term.strip()
                    key = token.lstrip("!")
                    require(key in configuration, f"{location}: unsupported preprocessor condition {token}")
                    values.append(not configuration[key] if token.startswith("!") else configuration[key])
                conditionals.append((active, all(values)))
                active = active and all(values)
            elif line == "#else":
                require(conditionals, f"{location}: unmatched #else")
                parent, condition = conditionals[-1]
                active = parent and not condition
            elif line == "#endif":
                require(conditionals, f"{location}: unmatched #endif")
                active = conditionals.pop()[0]
            elif active and line:
                label = re.fullmatch(r"(\w+)::?", line)
                if label:
                    require(label[1] not in self.labels, f"{location}: duplicate label {label[1]}")
                    self.labels[label[1]] = len(self.code)
                else:
                    parts = line.split(None, 1)
                    args = tuple(part.strip() for part in parts[1].split(",")) if len(parts) > 1 else ()
                    self.code.append(Instruction(parts[0], args, location))
        require(not conditionals, f"{path}: unterminated preprocessor condition")

    def address(self, label):
        require(label in self.labels, f"missing script/movement label {label}")
        return self.labels[label]

    def movement(self, label):
        actions = []
        for instruction in self.code[self.address(label):]:
            if instruction.command == "step_end":
                return actions
            require(not instruction.args and re.fullmatch(r"(?:walk|face)_(?:down|up|left|right)|delay_(?:1|2|4|8|16)", instruction.command),
                    f"{instruction.location}: unsupported movement action {instruction.command}")
            actions.append(instruction.command)
        raise SceneError(f"{label}: movement has no step_end")


class World:
    def __init__(self, root):
        self.map = json.loads((root / f"data/maps/{MAP}/map.json").read_text(encoding="utf-8"))
        layouts = json.loads((root / "data/layouts/layouts.json").read_text(encoding="utf-8"))["layouts"]
        layout = next(entry for entry in layouts if entry["id"] == self.map["layout"])
        self.width, self.height = layout["width"], layout["height"]
        data = (root / layout["blockdata_filepath"]).read_bytes()
        require(len(data) == self.width * self.height * 2, "lab layout dimensions do not match map.bin")
        self.tiles = struct.unpack(f"<{self.width * self.height}H", data)
        self.objects = {}
        self.aliases = {}
        for index, obj in enumerate(self.map["object_events"], 1):
            local_id = obj.get("local_id", str(index))
            self.objects[local_id] = obj
            self.aliases[str(index)] = local_id
            self.aliases[local_id] = local_id
        self.aliases[PLAYER] = PLAYER
        require(VIKTOR in self.objects, "lab has no Viktor object")
        self.balls = {choice: next(key for key, obj in self.objects.items()
                                  if obj["script"] == PREFIX + choice.title() + "Starter") for choice in CHOICES}
        flags = [self.objects[ball]["flag"] for ball in self.balls.values()]
        require(len(set(flags)) == 3 and all(flag.startswith("FLAG_") for flag in flags),
                "starter balls need distinct persistent visibility flags")
        self.constants = {"TRUE": 1, "FALSE": 0, "YES": 1, "NO": 0}
        header = (root / "include/constants/virelia_prologue.h").read_text(encoding="utf-8")
        for name, number in re.findall(r"^#define\s+(\w+)\s+(0x[0-9A-Fa-f]+|\d+)\s*$", header, re.M):
            self.constants[name] = int(number, 0)

    def floor(self, position):
        x, y = position
        return 0 <= x < self.width and 0 <= y < self.height and (self.tiles[y * self.width + x] >> 10) & 3 == 0

    def elevation(self, position):
        x, y = position
        return self.tiles[y * self.width + x] >> 12

    def neighbors(self, position):
        x, y = position
        return [(x + dx, y + dy) for dx, dy in DIRECTIONS.values()]

    def reachable(self):
        occupied = {(obj["x"], obj["y"]) for obj in self.objects.values()}
        starts = [(entry["x"], entry["y"]) for entry in self.map["warp_events"]]
        queue = deque(position for position in starts if self.floor(position) and position not in occupied)
        seen = set(queue)
        while queue:
            current = queue.popleft()
            for position in self.neighbors(current):
                if position not in seen and self.floor(position) and position not in occupied:
                    old, new = self.elevation(current), self.elevation(position)
                    if old == new or old == 0 or new == 0:
                        seen.add(position)
                        queue.append(position)
        return seen


def facing_between(source, target):
    delta = (target[0] - source[0], target[1] - source[1])
    return next((direction for direction, offset in DIRECTIONS.items() if offset == delta), None)


class Scene:
    def __init__(self, world, program, choice, start, selected, *, legacy=False,
                 hidden=False, completed=False, accepted=True, gift_success=True):
        self.world, self.program, self.choice = world, program, choice
        self.legacy, self.accepted, self.gift_success = legacy, accepted, gift_success
        self.positions = {key: (obj["x"], obj["y"]) for key, obj in world.objects.items()}
        self.positions[PLAYER] = start
        self.templates = dict(self.positions)
        self.visible = set(self.positions)
        self.facings = {key: ACTION_DIRECTIONS[obj["movement_type"].rsplit("_", 1)[1].lower()]
                        for key, obj in world.objects.items()}
        self.facings[PLAYER] = facing_between(start, self.positions[selected]) if selected else "DIR_SOUTH"
        self.selected = selected
        self.flags = {"FLAG_VIRELIA_MOM_MET", "FLAG_VIRELIA_PROFESSOR_MET"}
        self.variables = {"VAR_VIRELIA_STARTER": world.constants["VIRELIA_STARTER_" + choice],
                          "VAR_RESULT": 0}
        self.given = legacy
        if legacy:
            self.flags.add(STARTER_FLAG)
        if hidden:
            self.visible.remove(world.balls[choice])
        if completed:
            self.flags.add(RIVAL_FLAG)
            self.visible.discard(world.balls[choice])
            self.visible.discard(world.balls[COUNTER[choice]])
        self.pending = {}
        self.last_moved = None
        self.removals = []
        self.battles = []
        self.steps = 0
        self.dialogue_ready = False
        self.locked = False
        self.compare = None
        self.switch = None

    def value(self, token):
        if token.startswith("VAR_"):
            require(token in self.variables, f"read of uninitialized {token}")
            return self.variables[token]
        if token in self.world.constants:
            return self.world.constants[token]
        try:
            return int(token, 0)
        except ValueError:
            return token

    def actor(self, token):
        require(token in self.world.aliases, f"unknown scene actor {token}")
        return self.world.aliases[token]

    def settled(self):
        require(not self.pending, "scene continues before waitmovement finishes")

    def adjacent(self):
        player, rival = self.positions[PLAYER], self.positions[VIKTOR]
        require(facing_between(player, rival) is not None, f"Viktor {rival} is not adjacent to player {player}")
        require(self.facings[PLAYER] == facing_between(player, rival), "player does not face Viktor")
        require(self.facings[VIKTOR] == facing_between(rival, player), "Viktor does not face player")

    def move(self, actor):
        require(actor in self.pending, f"waitmovement has no pending movement for {actor}")
        for action in self.pending.pop(actor):
            if action.startswith("delay_"):
                continue
            kind, direction = action.split("_")
            self.facings[actor] = ACTION_DIRECTIONS[direction]
            if kind == "walk":
                source = self.positions[actor]
                dx, dy = DIRECTIONS[self.facings[actor]]
                destination = source[0] + dx, source[1] + dy
                require(self.world.floor(destination), f"{actor} walks into a wall at {destination}")
                old, new = self.world.elevation(source), self.world.elevation(destination)
                require(old == new or old == 0 or new == 0, f"{actor} crosses incompatible elevation at {destination}")
                blockers = [other for other in self.visible if other != actor and self.positions[other] == destination]
                require(not blockers, f"{actor} walks through {','.join(blockers)} at {destination}")
                self.positions[actor] = destination
                self.steps += 1

    def remove(self, actor):
        self.settled()
        require(actor in self.world.balls.values(), f"scene unexpectedly removes {actor}")
        require(self.given and STARTER_FLAG in self.flags, "starter ball disappears before successful gift")
        expected = [self.world.balls[self.choice], self.world.balls[COUNTER[self.choice]]]
        require(len(self.removals) < 2 and actor == expected[len(self.removals)],
                f"wrong pickup order: {self.removals + [actor]}, expected {expected}")
        if not self.removals and not self.legacy:
            require(self.steps == 0, "scene walks before the player takes the chosen ball")
            require(self.facings[PLAYER] == facing_between(self.positions[PLAYER], self.positions[actor]),
                    "player's chosen ball disappears without an adjacent, facing player")
        elif self.removals:
            require(self.facings[VIKTOR] == facing_between(self.positions[VIKTOR], self.positions[actor]),
                    "rival ball disappears before Viktor reaches and faces it")
        self.removals.append(actor)
        self.visible.discard(actor)
        self.flags.add(self.world.objects[actor]["flag"])

    def native(self, name):
        # Only native state transitions are stubbed; all event control flow and
        # movement remain the real sources. The C suite checks these contracts.
        if name == "VireliaPrologue_GiveStarter":
            self.variables["VAR_RESULT"] = int(self.gift_success)
            if self.gift_success:
                self.given = True
                self.flags.add(STARTER_FLAG)
                self.variables["VAR_VIRELIA_STARTER"] = self.value("VAR_0x8004")
        elif name == "VireliaPrologue_FinishRival":
            require(len(self.battles) == 1, "rival completion occurs without exactly one battle")
            self.flags.add(RIVAL_FLAG)
            self.variables["VAR_RESULT"] = 1
        elif name == "VireliaPrologue_GrantSupplies":
            require(RIVAL_FLAG in self.flags, "supplies granted before rival completion")
            self.variables["VAR_RESULT"] = 1
            self.flags.add("FLAG_VIRELIA_SUPPLIES_RECEIVED")
        else:
            raise SceneError(f"unsupported native scene call {name}")

    def run(self, entry, expect_battle):
        pc = self.program.address(entry)
        stack = []
        noop = {"closemessage", "playfanfare", "waitfanfare", "playse", "waitse", "delay", "hidefollower"}
        for _ in range(1000):
            require(pc < len(self.program.code), "script falls off end")
            instruction = self.program.code[pc]
            pc += 1
            command, args = instruction.command, instruction.args
            try:
                if command == "end":
                    self.settled()
                    require(not stack, "script ends inside a subroutine")
                    require(not self.locked, "script ends with the player locked")
                    require(len(self.battles) == int(expect_battle), f"expected {int(expect_battle)} battle, got {len(self.battles)}")
                    if expect_battle:
                        require(RIVAL_FLAG in self.flags, "script ends without recording rival completion")
                    if not expect_battle:
                        require(not self.removals and self.steps == 0, "cancelled/failed/completed scene changes the overworld")
                    return
                if command == "goto":
                    pc = self.program.address(args[0])
                elif command == "call":
                    stack.append(pc)
                    pc = self.program.address(args[0])
                elif command == "return":
                    require(stack, "return without call")
                    pc = stack.pop()
                elif command == "callnative":
                    self.native(args[0])
                elif command in ("setvar", "copyvar"):
                    self.variables[args[0]] = self.value(args[1])
                elif command in ("addvar", "subvar"):
                    self.variables[args[0]] = (self.value(args[0]) + (1 if command == "addvar" else -1) * self.value(args[1])) & 0xFFFF
                elif command == "checkflag":
                    self.compare = int(args[0] in self.flags)
                elif command == "setflag":
                    self.flags.add(args[0])
                elif command == "clearflag":
                    self.flags.discard(args[0])
                elif command == "compare":
                    self.compare = (self.value(args[0]) > self.value(args[1])) - (self.value(args[0]) < self.value(args[1]))
                elif command == "goto_if":
                    if self.compare == self.value(args[0]):
                        pc = self.program.address(args[1])
                elif command in ("goto_if_eq", "goto_if_ne"):
                    if len(args) == 3:
                        equal = self.value(args[0]) == self.value(args[1])
                    else:
                        require(len(args) == 1 and self.compare is not None, "invalid conditional branch")
                        equal = self.compare == 0
                    if equal == (command == "goto_if_eq"):
                        pc = self.program.address(args[-1])
                elif command == "switch":
                    self.switch = self.value(args[0])
                elif command == "case":
                    if self.switch == self.value(args[0]):
                        pc = self.program.address(args[1])
                elif command in ("lock", "lockall"):
                    self.locked = True
                elif command in ("release", "releaseall"):
                    self.settled()
                    require(not expect_battle or self.battles, "scene releases control before the rival battle")
                    self.locked = False
                elif command == "faceplayer":
                    if self.selected in self.visible:
                        self.facings[self.selected] = facing_between(self.positions[self.selected], self.positions[PLAYER])
                elif command == "msgbox":
                    self.settled()
                    if args[-1] == "MSGBOX_YESNO":
                        self.variables["VAR_RESULT"] = int(self.accepted)
                    if args[0] == MAP + "_Text_RivalIntro":
                        require(len(self.removals) == 2, "battle invitation precedes both pickups")
                        self.adjacent()
                        self.dialogue_ready = True
                elif command == "getplayerxy":
                    self.settled()
                    self.variables[args[0]], self.variables[args[1]] = self.positions[PLAYER]
                elif command == "getobjectfacingdirection":
                    self.settled()
                    self.variables[args[1]] = self.facings[self.actor(args[0])]
                elif command == "turnobject":
                    self.settled()
                    self.facings[self.actor(args[0])] = self.value(args[1])
                elif command == "applymovement":
                    require(self.locked, "scripted walking starts without locking the scene")
                    self.settled()  # Concurrent movement needs a frame-timing model.
                    actor = self.actor(args[0])
                    require(actor in self.visible, f"movement applied to absent {actor}")
                    self.pending[actor] = self.program.movement(args[1])
                    self.last_moved = actor
                elif command in ("waitmovement", "waitmovementall"):
                    actor = self.actor(args[0]) if args and args[0] not in ("0", "LOCALID_NONE") else self.last_moved
                    self.move(actor)
                elif command == "removeobject":
                    self.remove(self.actor(args[0]))
                elif command == "setobjectxyperm":
                    self.settled()
                    actor = self.actor(args[0])
                    self.templates[actor] = self.value(args[1]), self.value(args[2])
                    require(self.templates[actor] == self.positions[actor], f"{actor} permanent position disagrees with completed walking")
                elif command == "setobjectmovementtype":
                    self.settled()
                    actor = self.actor(args[0])
                    direction = ACTION_DIRECTIONS.get(args[1].removeprefix("MOVEMENT_TYPE_FACE_").lower())
                    require(direction == self.facings[actor], f"{actor} saved movement type disagrees with final facing")
                elif command == "trainerbattle_earlyrival":
                    self.settled()
                    require(self.locked and self.dialogue_ready, "battle starts without locked invitation dialogue")
                    require(not self.battles, "duplicate rival battle")
                    require(args[0] == "TRAINER_VIRELIA_VIKTOR_" + self.choice, f"wrong rival trainer for {self.choice}: {args[0]}")
                    require(len(args) == 5, "rival battle needs an explicit fifth actor argument")
                    battle_actor = self.actor(args[4])
                    require(battle_actor == VIKTOR and battle_actor in self.visible,
                            "rival battle actor must resolve to visible Viktor, never the removed starter ball")
                    expected = {self.world.balls[self.choice], self.world.balls[COUNTER[self.choice]]}
                    require(set(self.removals) == expected and not expected & self.visible, "chosen and counter balls must both disappear")
                    require((set(self.world.balls.values()) - expected) <= self.visible, "the third ball disappeared")
                    self.adjacent()
                    self.battles.append(args[0])
                elif command not in noop:
                    raise SceneError(f"unsupported scene command {command}")
            except (SceneError, KeyError, TypeError, IndexError) as error:
                raise SceneError(f"{instruction.location}: {error}") from error
        raise SceneError("script exceeds 1000 instructions (possible loop)")


def validate(root=ROOT, script_source=None):
    """Return a JSON-safe report; optionally validate an in-memory script variant."""
    report = {"passed": False, "fresh_routes": 0, "legacy_routes": 0,
              "guard_cases": 0, "walked_tiles": 0, "errors": []}
    try:
        world, program = World(Path(root)), Program(Path(root), script_source)
        reachable = world.reachable()
        rival = world.objects[VIKTOR]
        legacy_entries = [(event["script"], (event["x"], event["y"]), None)
                          for event in world.map["coord_events"] if event["script"] == PREFIX + "RivalGate"]
        legacy_entries += [(rival["script"], position, VIKTOR)
                           for position in world.neighbors((rival["x"], rival["y"])) if position in reachable]
        require(legacy_entries, "no reachable legacy scene entrances")

        def check(category, name, entry, choice, position, selected, battle, **options):
            report[category] += 1
            try:
                scene = Scene(world, program, choice, position, selected, **options)
                scene.run(entry, battle)
                report["walked_tiles"] += scene.steps
            except (SceneError, KeyError, StopIteration) as error:
                report["errors"].append(f"{name}: {error}")

        for choice in CHOICES:
            ball_id = world.balls[choice]
            ball = world.objects[ball_id]
            positions = [position for position in world.neighbors((ball["x"], ball["y"])) if position in reachable]
            require(positions, f"{choice} ball cannot be approached from the entrance")
            for position in positions:
                name = f"{choice} fresh from {position}"
                check("fresh_routes", name, ball["script"], choice, position, ball_id, True)
                check("guard_cases", name + " declined", ball["script"], choice, position, ball_id, False, accepted=False)
                check("guard_cases", name + " gift failed", ball["script"], choice, position, ball_id, False, gift_success=False)
            for entry, position, selected in legacy_entries:
                require(position in reachable, f"legacy entrance {position} is unreachable")
                for hidden in (False, True):
                    check("legacy_routes", f"{choice} legacy from {position}, chosen hidden={hidden}",
                          entry, choice, position, selected, True, legacy=True, hidden=hidden)
                check("guard_cases", f"{choice} completed from {position}", entry, choice, position, selected,
                      False, legacy=True, completed=True)
                check("guard_cases", f"{choice} before gift from {position}", entry, choice, position, selected, False)
    except (OSError, ValueError, KeyError, StopIteration) as error:
        report["errors"].append(str(error))
    report["passed"] = not report["errors"]
    return report


def mutation_checks(root=ROOT):
    """Prove the interpreter rejects representative scene regressions in memory."""
    source = (Path(root) / f"data/maps/{MAP}/scripts.inc").read_text(encoding="utf-8")
    movement = MAP + "_Movement_"
    mutations = {
        "wall": (movement + "Down3:\n\twalk_down", movement + "Down3:\n\twalk_left"),
        "npc": (movement + "ToFire:\n", movement + "ToFire:\n\twalk_right\n"),
        "ball": (movement + "ToWater:\n", movement + "ToWater:\n\twalk_up\n"),
        "first_pickup": (PREFIX + "RemoveGrassStarter:\n\tremoveobject LOCALID_VIRELIA_LAB_GRASS_STARTER",
                         PREFIX + "RemoveGrassStarter:\n\tremoveobject LOCALID_VIRELIA_LAB_FIRE_STARTER"),
        "wrong_counter": ("playse SE_BALL_OPEN\n\tremoveobject LOCALID_VIRELIA_LAB_FIRE_STARTER",
                          "playse SE_BALL_OPEN\n\tremoveobject LOCALID_VIRELIA_LAB_WATER_STARTER"),
        "missing_wait": ("waitmovement LOCALID_VIRELIA_LAB_VIKTOR", "delay 1"),
        "wrong_trainer": ("trainerbattle_earlyrival TRAINER_VIRELIA_VIKTOR_GRASS", "trainerbattle_earlyrival TRAINER_VIRELIA_VIKTOR_WATER"),
        "early_invitation": ("msgbox " + MAP + "_Text_RivalChoosing", "msgbox " + MAP + "_Text_RivalIntro"),
        "missing_battle_actor": (MAP + "_Text_RivalWon, LOCALID_VIRELIA_LAB_VIKTOR", MAP + "_Text_RivalWon"),
        "wrong_battle_actor": (MAP + "_Text_RivalWon, LOCALID_VIRELIA_LAB_VIKTOR",
                               MAP + "_Text_RivalWon, LOCALID_VIRELIA_LAB_GRASS_STARTER"),
        "duplicate_battle": ("callnative VireliaPrologue_FinishRival", "trainerbattle_earlyrival TRAINER_VIRELIA_VIKTOR_GRASS, RIVAL_BATTLE_HEAL_AFTER, unused, unused, LOCALID_VIRELIA_LAB_VIKTOR\n\tcallnative VireliaPrologue_FinishRival"),
    }
    results = {}
    for name, (before, after) in mutations.items():
        require(before in source, f"mutation check {name} no longer matches the script; update its probe")
        results[name] = not validate(root, source.replace(before, after, 1))["passed"]
        require(results[name], f"mutation check accepted regression: {name}")
    return results


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--json", action="store_true", help="Print the structured result")
    parser.add_argument("--self-test", action="store_true", help="Also reject representative in-memory script mutations")
    args = parser.parse_args()
    result = validate()
    if args.self_test and result["passed"]:
        try:
            result["mutation_checks"] = mutation_checks()
        except SceneError as error:
            result["passed"] = False
            result["errors"].append(str(error))
    if args.json:
        print(json.dumps(result, indent=2))
    else:
        print(f"{'PASS' if result['passed'] else 'FAIL'}: lab scene, {result['fresh_routes']} fresh routes, "
              f"{result['legacy_routes']} legacy routes, {result['guard_cases']} guard cases")
        for error in result["errors"]:
            print(error)
        if "mutation_checks" in result:
            print(f"PASS: {len(result['mutation_checks'])} invalid scene mutations rejected")
    return 0 if result["passed"] else 1


if __name__ == "__main__":
    sys.exit(main())
