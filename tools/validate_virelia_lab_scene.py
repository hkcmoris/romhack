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
COMPANION = "LOCALID_VIRELIA_LAB_VIKTOR_POKEMON"
PROFESSOR = "LOCALID_VIRELIA_LAB_PROFESSOR"
CHOICES = ("GRASS", "WATER", "FIRE")
COUNTER = {"GRASS": "FIRE", "WATER": "GRASS", "FIRE": "WATER"}
STARTER_SPECIES = {"GRASS": "SPECIES_POOCHYENA_VIRELIA",
                   "WATER": "SPECIES_NATU_VIRELIA", "FIRE": "SPECIES_EKANS_VIRELIA"}
DIRECTIONS = {"DIR_SOUTH": (0, 1), "DIR_NORTH": (0, -1),
              "DIR_WEST": (-1, 0), "DIR_EAST": (1, 0)}
ACTION_DIRECTIONS = {"down": "DIR_SOUTH", "up": "DIR_NORTH",
                     "left": "DIR_WEST", "right": "DIR_EAST"}
STARTER_FLAG = "FLAG_VIRELIA_STARTER_RECEIVED"
RIVAL_FLAG = "FLAG_VIRELIA_RIVAL_COMPLETE"
COMPANION_FLAG = "FLAG_VIRELIA_HIDE_RIVAL_POKEMON"


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
        common = (root / "data/event_scripts.s").read_text(encoding="utf-8")
        naming = re.search(r"^Common_EventScript_NameReceivedPartyMon::\n.*?\n\s*return\n", common, re.M | re.S)
        require(naming is not None, "missing native party Pokemon naming script")
        self.parse(naming[0], "data/event_scripts.s")

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
        require(COMPANION in self.objects, "lab has no rival Pokemon object")
        require(self.objects[COMPANION]["flag"] == COMPANION_FLAG,
                "rival Pokemon needs its own persistent visibility flag")
        require(self.objects[COMPANION]["graphics_id"] == "OBJ_EVENT_GFX_VAR_0",
                "rival Pokemon must use the selected starter's dynamic graphics")
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
        occupied = {(obj["x"], obj["y"]) for key, obj in self.objects.items() if key != COMPANION}
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
    def __init__(self, world, program, choice, start, selected, *, received=False,
                 completed=False, accepted=True, nickname=True, gift_success=True,
                 met_mom=True, introduced=True):
        self.world, self.program, self.choice = world, program, choice
        self.received, self.accepted, self.gift_success = received, accepted, gift_success
        self.nickname = nickname
        self.positions = {key: (obj["x"], obj["y"]) for key, obj in world.objects.items()}
        self.positions[PLAYER] = start
        if introduced and not completed:
            self.positions[VIKTOR] = (5, 8)
        self.templates = dict(self.positions)
        self.visible = set(self.positions)
        self.visible.discard(COMPANION)
        self.facings = {key: ACTION_DIRECTIONS[obj["movement_type"].rsplit("_", 1)[1].lower()]
                        for key, obj in world.objects.items()}
        self.facings[PLAYER] = facing_between(start, self.positions[selected]) if selected else "DIR_SOUTH"
        self.selected = selected
        self.flags = {"FLAG_VIRELIA_PROFESSOR_MET"} if introduced else set()
        self.flags.add(COMPANION_FLAG)
        if met_mom:
            self.flags.add("FLAG_VIRELIA_MOM_MET")
        self.variables = {"VAR_VIRELIA_STARTER": world.constants["VIRELIA_STARTER_" + choice],
                          "VAR_RESULT": 0}
        self.given = received
        if received:
            self.flags.add(STARTER_FLAG)
        if completed:
            self.flags.add(RIVAL_FLAG)
            self.flags.add(world.objects[world.balls[choice]]["flag"])
            self.flags.add(world.objects[world.balls[COUNTER[choice]]]["flag"])
            self.visible.discard(world.balls[choice])
            self.visible.discard(world.balls[COUNTER[choice]])
        self.pending = {}
        self.last_moved = None
        self.removals = []
        self.battles = []
        self.steps = 0
        self.preview_species = None
        self.preview_count = 0
        self.expect_preview = selected in world.balls.values() and not received and met_mom
        self.starter_description_seen = False
        self.confirmation_seen = False
        self.dialogue_ready = False
        self.locked = False
        self.compare = None
        self.switch = None
        self.dialogue = []
        self.nickname_prompts = 0
        self.naming_screens = 0
        self.faded = False
        self.rival_previous = None
        self.rival_trail = deque()
        self.companion_species = None
        self.companion_steps = 0
        self.companion_spawns = 0

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
                if actor == VIKTOR:
                    self.rival_previous = source
                    if COMPANION in self.visible:
                        self.rival_trail.append(source)
                elif actor == COMPANION:
                    require(self.rival_trail and destination == self.rival_trail.popleft(),
                            "rival Pokemon does not follow Viktor's vacated tile")
                    self.companion_steps += 1
                self.positions[actor] = destination
                self.steps += 1

    def remove(self, actor):
        self.settled()
        require(actor in self.world.balls.values(), f"scene unexpectedly removes {actor}")
        require(self.given and STARTER_FLAG in self.flags, "starter ball disappears before successful gift")
        expected = [self.world.balls[self.choice], self.world.balls[COUNTER[self.choice]]]
        require(len(self.removals) < 2 and actor == expected[len(self.removals)],
                f"wrong pickup order: {self.removals + [actor]}, expected {expected}")
        if not self.removals:
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
        if name == "VireliaPrologue_SyncStarterBallFlags":
            pass  # Ball visibility/save semantics are checked by the C suite.
        elif name == "VireliaPrologue_SyncRivalPokemon":
            # Native C tests cover the selected graphics, visibility guards,
            # and persisted pickup flag used by this script contract.
            self.flags.add(COMPANION_FLAG)
            if STARTER_FLAG in self.flags and "FLAG_VIRELIA_HIDE_STORY_ACTORS" not in self.flags:
                self.companion_species = STARTER_SPECIES[COUNTER[self.choice]]
                if self.world.objects[self.world.balls[COUNTER[self.choice]]]["flag"] in self.flags:
                    self.flags.discard(COMPANION_FLAG)
        elif name == "UpdateFollowingPokemon":
            pass  # Party eligibility is covered by the C follower suite.
        elif name == "VireliaPrologue_GiveStarter":
            require(self.confirmation_seen and self.accepted is True, "starter gift was not confirmed")
            require(self.preview_species is None, "starter preview remains open when giving the Pokemon")
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

    def run(self, entry, expect_battle=False, *, phase="interaction"):
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
                    require(self.preview_species is None, "script ends with the starter preview open")
                    require(self.preview_count == int(self.expect_preview), "starter examination did not show exactly one preview")
                    require(self.confirmation_seen == self.expect_preview, "starter examination skipped its confirmation")
                    require(len(self.battles) == int(expect_battle), f"expected {int(expect_battle)} battle, got {len(self.battles)}")
                    if expect_battle:
                        require(self.companion_spawns == 1 and self.companion_steps > 0,
                                "rival Pokemon must appear once and walk behind Viktor")
                        require(COMPANION in self.visible and COMPANION_FLAG not in self.flags,
                                "rival Pokemon disappears after the battle")
                        require(self.templates[COMPANION] == self.positions[COMPANION],
                                "rival Pokemon's final position was not saved")
                        require(RIVAL_FLAG in self.flags, "script ends without recording rival completion")
                        require(self.nickname_prompts == 1, "professor must offer naming once after the rival battle")
                        require(self.naming_screens == int(self.nickname is True), "naming choice did not open/skip the naming screen correctly")
                        require("FLAG_VIRELIA_SUPPLIES_RECEIVED" in self.flags, "naming flow failed to continue to supplies")
                    if not expect_battle and phase == "interaction":
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
                elif command == "fadescreen":
                    require(args == ("FADE_TO_BLACK",), "unexpected naming fade")
                    self.faded = True
                elif command == "special":
                    require(args == ("ChangePokemonNickname",), "unexpected scene special")
                    require(self.locked and self.faded and RIVAL_FLAG in self.flags and self.battles,
                            "naming screen must open after the battle with the scene locked and faded")
                    require(self.nickname_prompts == 1 and self.nickname is True, "naming screen opened without consent")
                    require(self.value("VAR_0x8004") == 0, "naming screen must target the starter in party slot zero")
                    self.naming_screens += 1
                    self.faded = False  # The native callback returns to the map.
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
                    require(self.preview_species is None, "script releases control with the starter preview open")
                    if phase == "welcome":
                        require(self.dialogue == [MAP + "_Text_Introduction", MAP + "_Text_RivalWaiting"],
                                "welcome dialogue must introduce the professor before Viktor's tactics")
                        require("FLAG_VIRELIA_PROFESSOR_MET" in self.flags, "welcome has no completion flag")
                        require(self.positions[PLAYER] == (6, 8) and self.positions[VIKTOR] == (5, 8),
                                "welcome releases control outside the central aisle")
                        require(self.templates[VIKTOR] == self.positions[VIKTOR], "Viktor's greeting position was not saved")
                        self.adjacent()
                    self.locked = False
                elif command == "showmonpic":
                    require(self.locked and self.preview_species is None, "preview needs a locked scene and no existing picture")
                    require(STARTER_FLAG not in self.flags and "FLAG_VIRELIA_MOM_MET" in self.flags,
                            "starter preview opened before its availability guards")
                    require(self.value(args[0]) == STARTER_SPECIES[self.choice], "preview shows the wrong regional starter")
                    self.preview_species = self.value(args[0])
                    self.preview_count += 1
                elif command == "hidemonpic":
                    self.preview_species = None
                elif command == "faceplayer":
                    if self.selected in self.visible:
                        self.facings[self.selected] = facing_between(self.positions[self.selected], self.positions[PLAYER])
                elif command == "msgbox":
                    self.settled()
                    self.dialogue.append(args[0])
                    if phase == "welcome" and args[0] == MAP + "_Text_Introduction":
                        require(self.locked and self.positions[PLAYER] == (6, 8) and self.facings[PLAYER] == "DIR_NORTH",
                                "professor speaks before the player stops and faces her in the middle of the lab")
                        require(self.positions[VIKTOR] == (4, 8) and self.facings[VIKTOR] == "DIR_EAST",
                                "player must stop in Viktor's sight before his approach")
                    if phase == "welcome" and args[0] == MAP + "_Text_RivalWaiting":
                        self.adjacent()
                    if args[0] == MAP + "_Text_Nickname":
                        require(self.locked and len(self.battles) == 1 and RIVAL_FLAG in self.flags,
                                "nickname offer precedes rival completion")
                        require("FLAG_VIRELIA_SUPPLIES_RECEIVED" not in self.flags, "nickname offer must precede supplies")
                        self.nickname_prompts += 1
                    if args[0] == MAP + "_Text_" + self.choice.title() + "Starter":
                        require(self.preview_species == STARTER_SPECIES[self.choice], "starter description has no correct preview")
                        self.starter_description_seen = True
                    if args[0] == MAP + "_Text_ConfirmStarter":
                        require(self.starter_description_seen and self.preview_species == STARTER_SPECIES[self.choice],
                                "starter confirmation has no correct preview or description")
                        self.confirmation_seen = True
                    if args[-1] == "MSGBOX_YESNO":
                        # Native Task_HandleYesNoInput maps both No and B to 0.
                        answer = self.nickname if args[0] == MAP + "_Text_Nickname" else self.accepted
                        self.variables["VAR_RESULT"] = int(answer is True)
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
                    require(self.preview_species is None, "scripted walking starts with the starter preview open")
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
                elif command == "addobject":
                    self.settled()
                    actor = self.actor(args[0])
                    require(actor == COMPANION and actor not in self.visible, "unexpected object spawn")
                    require(COMPANION_FLAG not in self.flags and len(self.removals) == 2,
                            "rival Pokemon appears before Viktor picks up his Ball")
                    position = self.templates[actor]
                    require(position == self.rival_previous, "rival Pokemon must appear on Viktor's vacated tile")
                    require(self.world.floor(position) and all(self.positions[other] != position for other in self.visible),
                            "rival Pokemon spawns on blocked or occupied ground")
                    require(self.companion_species == STARTER_SPECIES[COUNTER[self.choice]],
                            "rival Pokemon has the wrong starter species")
                    self.positions[actor] = position
                    self.visible.add(actor)
                    self.companion_spawns += 1
                elif command == "setobjectxyperm":
                    self.settled()
                    actor = self.actor(args[0])
                    self.templates[actor] = self.value(args[1]), self.value(args[2])
                    if phase == "load":
                        self.positions[actor] = self.templates[actor]  # Before sprites are spawned.
                    elif actor in self.visible:
                        require(self.templates[actor] == self.positions[actor], f"{actor} permanent position disagrees with completed walking")
                elif command == "setobjectmovementtype":
                    self.settled()
                    actor = self.actor(args[0])
                    direction = ACTION_DIRECTIONS.get(args[1].removeprefix("MOVEMENT_TYPE_FACE_").lower())
                    if phase == "load" or actor not in self.visible:
                        self.facings[actor] = direction
                    else:
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
                    require(COMPANION in self.visible and not self.rival_trail,
                            "rival battle starts without Viktor's companion caught up")
                    require(facing_between(self.positions[COMPANION], self.positions[VIKTOR]) is not None,
                            "rival Pokemon must stand next to Viktor before battle")
                    self.adjacent()
                    self.battles.append(args[0])
                elif command not in noop:
                    raise SceneError(f"unsupported scene command {command}")
            except (SceneError, KeyError, TypeError, IndexError) as error:
                raise SceneError(f"{instruction.location}: {error}") from error
        raise SceneError("script exceeds 1000 instructions (possible loop)")


def validate(root=ROOT, script_source=None):
    """Return a JSON-safe report; optionally validate an in-memory script variant."""
    report = {"passed": False, "welcome_cases": 0, "fresh_routes": 0,
              "guard_cases": 0, "preview_cases": 0, "naming_screens": 0,
              "companion_reload_cases": 0, "walked_tiles": 0, "errors": []}
    try:
        world, program = World(Path(root)), Program(Path(root), script_source)
        reachable = world.reachable()
        # The table remains solid even after its Ball objects are removed.
        require([world.tiles[6 * world.width + x] for x in range(4, 9)]
                == [0x0693, 0x0694, 0x0694, 0x0694, 0x0695], "starter table surface/collision changed")
        require([world.tiles[7 * world.width + x] for x in range(4, 9)]
                == [0x329B, 0x329C, 0x329C, 0x329C, 0x329D], "starter table front changed")
        for choice, x in zip(CHOICES, (4, 6, 8)):
            ball = world.objects[world.balls[choice]]
            require((ball["x"], ball["y"]) == (x, 6), f"{choice} Ball is not on the table")

        for warp in world.map["warp_events"]:
            for introduced, met_mom in ((False, True), (True, True), (False, False)):
                scene = Scene(world, program, "GRASS", (warp["x"], warp["y"]), None,
                              introduced=introduced, met_mom=met_mom)
                # Loading fresh map templates must restore the current scene's
                # greeting position when returning before choosing a starter.
                scene.positions[VIKTOR] = (4, 8)
                scene.run(MAP + "_OnLoad", phase="load")
                scene.run(MAP + "_OnTransition", phase="transition")
                expected_welcome = not introduced and met_mom
                require(scene.variables["VAR_TEMP_0"] == int(expected_welcome), "welcome does not run exactly once after meeting Mom")
                if expected_welcome:
                    scene.run(PREFIX + "Welcome", phase="welcome")
                    require(not scene.removals and not scene.battles and not scene.given,
                            "welcome chooses a starter or starts a battle before returning control")
                    scene.run(MAP + "_OnTransition", phase="transition")
                    require(scene.variables["VAR_TEMP_0"] == 0, "welcome replays after completion")
                elif introduced:
                    require(scene.positions[VIKTOR] == (5, 8), "Viktor returns to the wrong pre-choice position on re-entry")
                require(COMPANION_FLAG in scene.flags, "rival Pokemon appears before a starter is selected")
                report["welcome_cases"] += 1
                report["walked_tiles"] += scene.steps

        def check(category, name, entry, choice, position, selected, battle, **options):
            report[category] += 1
            try:
                scene = Scene(world, program, choice, position, selected, **options)
                scene.run(entry, battle)
                report["walked_tiles"] += scene.steps
                report["preview_cases"] += scene.preview_count
                report["naming_screens"] += scene.naming_screens
            except (SceneError, KeyError, StopIteration) as error:
                report["errors"].append(f"{name}: {error}")

        for choice in CHOICES:
            for completed in (False, True):
                scene = Scene(world, program, choice, (6, 12), None, received=True, completed=completed)
                scene.flags.add(world.objects[world.balls[COUNTER[choice]]]["flag"])
                # Battle return occurs before FinishRival; re-entry happens
                # after it. Both must restore the selected companion.
                scene.run(MAP + "_OnLoad", phase="load")
                require(COMPANION_FLAG not in scene.flags, "rival Pokemon disappears on battle return or re-entry")
                require(scene.companion_species == STARTER_SPECIES[COUNTER[choice]], "reloaded rival Pokemon has the wrong species")
                expected_rival = (7, 8) if choice == "FIRE" else (5, 8)
                expected_companion = (4, 8) if choice == "WATER" else (6, 8)
                expected_facing = "DIR_WEST" if choice == "GRASS" else "DIR_EAST"
                require(scene.positions[VIKTOR] == expected_rival and scene.positions[COMPANION] == expected_companion,
                        "reloaded rival and Pokemon do not match their final scene positions")
                require(scene.facings[VIKTOR] == scene.facings[COMPANION] == expected_facing,
                        "reloaded rival Pokemon does not face along Viktor's trail")
                report["companion_reload_cases"] += 1
            ball_id = world.balls[choice]
            ball = world.objects[ball_id]
            positions = [position for position in world.neighbors((ball["x"], ball["y"])) if position in reachable]
            require(positions, f"{choice} ball cannot be approached from the entrance")
            for position in positions:
                name = f"{choice} fresh from {position}"
                for nickname in (True, False, "cancel"):
                    check("fresh_routes", name + f", naming={nickname}", ball["script"], choice, position, ball_id,
                          True, nickname=nickname)
                check("guard_cases", name + " declined", ball["script"], choice, position, ball_id, False, accepted=False)
                check("guard_cases", name + " B cancelled", ball["script"], choice, position, ball_id, False, accepted="cancel")
                check("guard_cases", name + " gift failed", ball["script"], choice, position, ball_id, False, gift_success=False)
                check("guard_cases", name + " before meeting Mom", ball["script"], choice, position, ball_id, False, met_mom=False)
                for prior_choice in CHOICES:
                    if prior_choice != choice:
                        check("guard_cases", name + " after choosing " + prior_choice, ball["script"], prior_choice,
                              position, ball_id, False, received=True)
    except (OSError, ValueError, KeyError, StopIteration) as error:
        report["errors"].append(str(error))
    report["passed"] = not report["errors"]
    return report


def mutation_checks(root=ROOT):
    """Prove the interpreter rejects representative scene regressions in memory."""
    source = (Path(root) / f"data/maps/{MAP}/scripts.inc").read_text(encoding="utf-8")
    movement = MAP + "_Movement_"
    mutations = {
        "missing_preview": ("showmonpic SPECIES_POOCHYENA_VIRELIA, 10, 3", "delay 1"),
        "wrong_preview_species": ("showmonpic SPECIES_NATU_VIRELIA, 10, 3", "showmonpic SPECIES_NATU, 10, 3"),
        "early_preview_close": ("showmonpic SPECIES_EKANS_VIRELIA, 10, 3", "showmonpic SPECIES_EKANS_VIRELIA, 10, 3\n\thidemonpic"),
        "missing_preview_close": ("\thidemonpic\n", "\tdelay 1\n"),
        "table": (movement + "Down1:\n\twalk_down", movement + "Down1:\n\twalk_up"),
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
        "early_speech": (movement + "EnterLab:\n\twalk_up", movement + "EnterLab:\n\tface_up"),
        "missing_greeting": ("\tapplymovement " + VIKTOR + ", " + movement + "GreetPlayer", "\tdelay 1"),
        "missing_welcome_flag": ("\tsetflag FLAG_VIRELIA_PROFESSOR_MET", "\tdelay 1"),
        "lost_waiting_position": ("\tsetobjectxyperm " + VIKTOR + ", 5, 8", "\tdelay 1"),
        "wrong_starter_named": ("\tsetvar VAR_0x8004, 0\n\tcall Common_EventScript_NameReceivedPartyMon",
                                "\tsetvar VAR_0x8004, 1\n\tcall Common_EventScript_NameReceivedPartyMon"),
        "missing_naming_screen": ("\tcall Common_EventScript_NameReceivedPartyMon", "\tdelay 1"),
        "missing_companion_spawn": ("\taddobject " + COMPANION, "\tdelay 1"),
        "companion_in_wall": ("\tsetobjectxyperm " + COMPANION + ", 8, 7",
                              "\tsetobjectxyperm " + COMPANION + ", 8, 6"),
        "missing_companion_step": ("\tcall " + PREFIX + "FollowRivalLeft\n", "\tdelay 1\n"),
        "missing_companion_wait": ("\twaitmovement " + COMPANION, "\tdelay 1"),
        "wrong_companion_reload": ("\tsetobjectxyperm " + COMPANION + ", 6, 8",
                                   "\tsetobjectxyperm " + COMPANION + ", 6, 9"),
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
              f"{result['welcome_cases']} welcome cases, {result['guard_cases']} guard cases, "
              f"{result['companion_reload_cases']} companion reload cases")
        for error in result["errors"]:
            print(error)
        if "mutation_checks" in result:
            print(f"PASS: {len(result['mutation_checks'])} invalid scene mutations rejected")
    return 0 if result["passed"] else 1


if __name__ == "__main__":
    sys.exit(main())
