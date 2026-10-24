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
            require(not instruction.args and re.fullmatch(r"(?:walk(?:_fast)?|face)_(?:down|up|left|right)|delay_(?:1|2|4|8|16)", instruction.command),
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
        self.case = next(event for event in self.map["bg_events"] if event["script"] == PREFIX + "StarterCase")
        require((self.case["x"], self.case["y"]) == (6, 6)
                and self.case["player_facing_dir"] == "BG_EVENT_PLAYER_FACING_NORTH",
                "case must have one north-facing interaction point")
        require(not any("STARTER" in actor for actor in self.objects), "loose starter Ball objects remain")
        self.constants = {"TRUE": 1, "FALSE": 0, "YES": 1, "NO": 0, "MALE": 0, "FEMALE": 1}
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


RAT = "LOCALID_VIRELIA_LAB_RATTATA"
CAT = "LOCALID_VIRELIA_LAB_MEOWTH"
RESCUE_FLAG = "FLAG_VIRELIA_LAB_RESCUE_COMPLETE"


class Scene:
    def __init__(self, world, program, choice="GRASS", start=(6, 12), *,
                 nickname=True, rescue="WON", duel="WON", flee=False,
                 mom=True, introduced=False, gift_success=True, player_gender="FEMALE"):
        self.world, self.program, self.choice = world, program, choice
        self.nickname, self.rescue, self.duel, self.flee = nickname, rescue, duel, flee
        self.gift_success = gift_success
        self.player_gender = player_gender
        self.rival_name = "VIKTORIE" if player_gender == "MALE" else "VIKTOR"
        self.rival_graphics = None
        self.positions = {key: (obj["x"], obj["y"]) for key, obj in world.objects.items()}
        self.positions[PLAYER] = start
        self.templates = dict(self.positions)
        self.visible = set(self.positions)
        self.facings = {key: ACTION_DIRECTIONS[obj["movement_type"].rsplit("_", 1)[1].lower()]
                        for key, obj in world.objects.items()}
        self.facings[PLAYER] = "DIR_NORTH"
        self.flags = {"FLAG_VIRELIA_MOM_MET"} if mom else set()
        if introduced:
            self.flags.add("FLAG_VIRELIA_PROFESSOR_MET")
        self.variables = {"VAR_VIRELIA_STARTER": world.constants["VIRELIA_STARTER_NONE"], "VAR_RESULT": 0}
        self.pending = {}
        self.compare = self.switch = None
        self.locked = self.faded = self.given = self.partner = self.healed = False
        self.prepared = False
        self.dialogue = []
        self.removals = []
        self.battles = []
        self.case_count = self.gifts = self.steps = self.naming_screens = self.naming_prompts = 0
        self.last_outcome = None

    def value(self, token):
        if token.startswith("VAR_"):
            require(token in self.variables, f"uninitialized {token}")
            return self.variables[token]
        if token in self.world.constants:
            return self.world.constants[token]
        return int(token, 0) if re.fullmatch(r"(?:0x[\dA-Fa-f]+|\d+)", token) else token

    def actor(self, token):
        require(token in self.world.aliases, f"unknown actor {token}")
        return self.world.aliases[token]

    def settled(self):
        require(not self.pending, "scene continues before its movements finish")

    def move_all(self):
        # Both escapees use the same fast walking speed. Advance them together;
        # checking old and new occupied tiles also rejects swaps and shortcuts.
        while any(self.pending.values()):
            destinations = {}
            for actor, actions in self.pending.items():
                if not actions:
                    continue
                action = actions.pop(0)
                if action.startswith("delay_"):
                    continue
                direction = action.rsplit("_", 1)[1]
                self.facings[actor] = ACTION_DIRECTIONS[direction]
                if action.startswith("walk_"):
                    x, y = self.positions[actor]
                    dx, dy = DIRECTIONS[self.facings[actor]]
                    target = x + dx, y + dy
                    require(self.world.floor(target), f"{actor} walks into solid furniture at {target}")
                    old, new = self.world.elevation((x, y)), self.world.elevation(target)
                    require(old == new or old == 0 or new == 0, f"{actor} crosses incompatible elevation at {target}")
                    blockers = [other for other in self.visible if other != actor and self.positions[other] == target]
                    require(not blockers, f"{actor} walks through {blockers} at {target}")
                    require(target not in destinations.values(), f"actors collide at {target}")
                    destinations[actor] = target
                    self.steps += 1
            self.positions.update(destinations)
        self.pending.clear()

    def adjacent(self):
        player, rival = self.positions[PLAYER], self.positions[VIKTOR]
        require(facing_between(player, rival) is not None, "player and Viktor are not adjacent")
        require(self.facings[PLAYER] == facing_between(player, rival), "player is not facing Viktor")
        require(self.facings[VIKTOR] == facing_between(rival, player), "Viktor is not facing the player")

    def native(self, name):
        if name == "VireliaPrologue_SetRivalAppearance":
            self.rival_graphics = "MAY" if self.player_gender == "MALE" else "BRENDAN"
            return
        elif name == "VireliaPrologue_SyncStarterBallFlags":
            if self.given:
                self.flags.add("FLAG_VIRELIA_HIDE_" + self.choice + "_STARTER")
                taken = [choice for choice in CHOICES if "FLAG_VIRELIA_HIDE_" + choice + "_STARTER" in self.flags]
                for choice in taken:
                    if choice in self.removals:
                        continue
                    expected = [self.choice, COUNTER[self.choice]]
                    require(len(self.removals) < 2 and choice == expected[len(self.removals)], "wrong case slot pickup order")
                    actor = VIKTOR if self.removals else PLAYER
                    require(self.positions[actor] == (6, 7) and self.facings[actor] == "DIR_NORTH",
                            "starter taken away from the single case interaction point")
                    self.removals.append(choice)
            return
        if name == "VireliaPrologue_GiveStarter":
            require(self.case_count == 1 and not self.given, "gift bypasses the case or repeats")
            require(self.value("VAR_0x8004") == self.value("VIRELIA_STARTER_" + self.choice), "wrong starter granted")
            self.variables["VAR_RESULT"] = int(self.gift_success)
            if self.gift_success:
                self.given = True
                self.gifts += 1
                self.flags.add(STARTER_FLAG)
                self.variables["VAR_VIRELIA_STARTER"] = self.value("VAR_0x8004")
        elif name == "VireliaPrologue_PrepareRescue":
            require(self.given and len(self.removals) == 2, "rescue starts before both starter pickups")
            require(RAT in self.visible and CAT in self.visible, "rescue starts without both escapees")
            require(self.positions[RAT] == (6, 9) and self.positions[CAT] == (7, 9), "chase did not finish at the confrontation")
            self.prepared = self.partner = True
            self.variables["VAR_RESULT"] = 1
        elif name == "VireliaPrologue_FinishRescue":
            require(self.battles and self.battles[-1] == "rescue", "rescue completes before its battle")
            success = self.last_outcome in ("WON", "LOST", "DREW")
            self.variables["VAR_RESULT"] = int(success)
            if success:
                self.flags.add(RESCUE_FLAG)
                self.healed, self.partner = True, False
        elif name == "VireliaPrologue_FinishRival":
            require(self.battles[-1] == "duel" and self.last_outcome in ("WON", "LOST", "DREW"), "invalid rival completion")
            self.flags.add(RIVAL_FLAG)
            self.variables["VAR_RESULT"] = 1
        elif name == "VireliaPrologue_GrantSupplies":
            require(RIVAL_FLAG in self.flags and self.naming_prompts == 1, "supplies precede the duel/naming offer")
            self.flags.add("FLAG_VIRELIA_SUPPLIES_RECEIVED")
            self.variables["VAR_RESULT"] = 1
        else:
            raise SceneError(f"unsupported native call {name}")

    def special(self, name):
        require(self.locked, "screen or battle starts without locking controls")
        if name == "DrawWholeMapView":
            return
        if name == "ChooseVireliaStarter":
            require(self.faded and not self.given, "case opens without a fade or after the gift")
            require(self.positions[PLAYER] == (6, 7) and self.facings[PLAYER] == "DIR_NORTH", "player has not reached the case")
            require(MAP + "_Text_TakeCase" in self.dialogue, "case opens before the emergency request")
            self.case_count += 1
            self.variables["VAR_RESULT"] = self.value("VIRELIA_STARTER_" + self.choice)
            self.faded = False
        elif name == "VireliaPrologue_StartRescueBattle":
            require(self.prepared and self.partner, "rescue has no NPC partner setup")
            require("duel" not in self.battles and RESCUE_FLAG not in self.flags, "rescue repeats after completion/duel")
            require(self.facings[PLAYER] == self.facings[VIKTOR] == "DIR_SOUTH", "trainers do not face the escapees")
            self.last_outcome = "RAN" if self.flee and not self.battles else self.rescue
            self.battles.append("rescue")
            self.prepared = False
        elif name == "ChangePokemonNickname":
            require(self.faded and RIVAL_FLAG in self.flags and self.nickname is True, "naming screen opens without consent after the duel")
            require(self.value("VAR_0x8004") == 0, "naming screen targets another party slot")
            self.naming_screens += 1
            self.faded = False
        else:
            raise SceneError(f"unsupported special {name}")

    def remove(self, actor):
        self.settled()
        if actor in (RAT, CAT):
            require(RESCUE_FLAG in self.flags and not self.partner and self.healed, "escapee removed before rescue completion and healing")
        else:
            raise SceneError("only the two escapees should be removed; starters use case slots")
        self.visible.discard(actor)

    def run(self, entry, *, opening=False):
        pc, stack = self.program.address(entry), []
        noop = {"closemessage", "playfanfare", "waitfanfare", "playse", "waitse", "delay",
                "hidefollower", "playmoncry", "waitmoncry", "playbgm"}
        for _ in range(1500):
            instruction = self.program.code[pc]
            pc += 1
            command, args = instruction.command, instruction.args
            try:
                if command == "end":
                    self.settled()
                    require(not stack and not self.locked, "scene ends inside a subroutine or with controls locked")
                    if opening and self.gift_success:
                        require(self.battles == ["rescue"] * (1 + int(self.flee)) + ["duel"], "wrong battle sequence")
                        require(self.gifts == self.case_count == self.naming_prompts == 1, "gift/case/naming is missing or repeated")
                        require(self.naming_screens == int(self.nickname is True), "wrong naming choice handling")
                        require("FLAG_VIRELIA_SUPPLIES_RECEIVED" in self.flags, "opening did not continue to supplies")
                        require(RAT not in self.visible and CAT not in self.visible and not self.partner, "escapees/ally setup survive the opening")
                        require(len(self.removals) == 2, "unused third case slot was lost")
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
                elif command == "special":
                    self.special(args[0])
                elif command in ("setvar", "copyvar"):
                    self.variables[args[0]] = self.value(args[1])
                elif command == "checkflag":
                    self.compare = int(args[0] in self.flags)
                elif command == "checkplayergender":
                    self.variables["VAR_RESULT"] = self.value(self.player_gender)
                elif command == "setflag":
                    self.flags.add(args[0])
                elif command == "goto_if":
                    if self.compare == self.value(args[0]):
                        pc = self.program.address(args[1])
                elif command in ("goto_if_eq", "goto_if_ne"):
                    equal = self.value(args[0]) == self.value(args[1])
                    if equal == (command == "goto_if_eq"):
                        pc = self.program.address(args[2])
                elif command == "switch":
                    self.switch = self.value(args[0])
                elif command == "case":
                    if self.switch == self.value(args[0]):
                        pc = self.program.address(args[1])
                elif command in ("lock", "lockall"):
                    self.locked = True
                elif command in ("release", "releaseall"):
                    self.settled()
                    self.locked = False
                elif command == "fadescreen":
                    require(args == ("FADE_TO_BLACK",), "unexpected screen fade")
                    self.faded = True
                elif command == "getplayerxy":
                    self.variables[args[0]], self.variables[args[1]] = self.positions[PLAYER]
                elif command == "turnobject":
                    self.settled()
                    self.facings[self.actor(args[0])] = self.value(args[1])
                elif command == "applymovement":
                    require(self.locked, "walking starts without locking controls")
                    actor = self.actor(args[0])
                    require(actor not in self.pending and actor in self.visible, "movement repeats or targets absent actor")
                    self.pending[actor] = self.program.movement(args[1])
                elif command in ("waitmovement", "waitmovementall"):
                    require(self.pending, "movement wait has no movement")
                    self.move_all()
                elif command == "setobjectxyperm":
                    actor = self.actor(args[0])
                    self.templates[actor] = self.value(args[1]), self.value(args[2])
                    require(self.templates[actor] == self.positions[actor], "saved NPC position disagrees with walking")
                elif command == "setobjectmovementtype":
                    actor = self.actor(args[0])
                    direction = ACTION_DIRECTIONS[args[1].removeprefix("MOVEMENT_TYPE_FACE_").lower()]
                    require(direction == self.facings[actor], "saved NPC facing disagrees with walking")
                elif command == "removeobject":
                    self.remove(self.actor(args[0]))
                elif command == "msgbox":
                    self.settled()
                    self.dialogue.append(args[0])
                    if args[0] == MAP + "_Text_Introduction" and opening:
                        require(self.positions[PLAYER] == (6, 8) and self.facings[PLAYER] == "DIR_NORTH", "professor speaks before the player reaches her sight")
                    if args[0] == MAP + "_Text_Escape":
                        require(self.positions[VIKTOR] == (6, 11), "Viktor is not blocking the exit during the chase")
                    if args[0] == MAP + "_Text_RescueHealed":
                        require(RESCUE_FLAG in self.flags and self.healed and not self.partner, "healing dialogue precedes rescue cleanup")
                        require(not ({RAT, CAT} & self.visible), "escapees remain visible after containment")
                    if args[0] in (MAP + "_Text_RivalIntro", MAP + "_Text_RivalIntroFemale"):
                        expected = MAP + "_Text_RivalIntro" + ("Female" if self.player_gender == "MALE" else "")
                        require(args[0] == expected, "rival tactics dialogue uses the wrong gender")
                        require(MAP + "_Text_RescueHealed" in self.dialogue, "duel invitation precedes healing")
                        self.adjacent()
                    if args[0] == MAP + "_Text_Nickname":
                        require(RIVAL_FLAG in self.flags and self.battles[-1] == "duel", "naming is offered before the rival duel")
                        self.naming_prompts += 1
                    if args[-1] == "MSGBOX_YESNO":
                        self.variables["VAR_RESULT"] = int(self.nickname is True)
                elif command == "trainerbattle_earlyrival":
                    self.settled()
                    require(self.locked and RESCUE_FLAG in self.flags and self.healed and not self.partner, "duel starts with uncleared ally state or before rescue/healing")
                    require(args[0] == "TRAINER_VIRELIA_" + self.rival_name + "_" + self.choice,
                            "duel uses the wrong rival gender or opposing starter")
                    lost_text = "RivalLostFemale" if self.player_gender == "MALE" else "RivalLost"
                    require(args[2] == MAP + "_Text_" + lost_text, "duel uses the wrong gendered defeat line")
                    require(len(args) == 5 and self.actor(args[4]) == VIKTOR, "duel actor is not Viktor")
                    require("duel" not in self.battles, "duel repeats")
                    self.adjacent()
                    self.battles.append("duel")
                    self.last_outcome = self.duel
                elif command not in noop:
                    raise SceneError(f"unsupported scene command {command}")
            except (SceneError, KeyError, IndexError) as error:
                raise SceneError(f"{instruction.location}: {error}") from error
        raise SceneError("scene exceeds its instruction budget")


def validate(root=ROOT, script_source=None):
    report = {"passed": False, "opening_cases": 0, "guard_cases": 0, "walked_tiles": 0, "errors": []}
    try:
        world, program = World(Path(root)), Program(Path(root), script_source)
        require({RAT, CAT} <= world.objects.keys(), "lab escapees are missing")
        require(all(not world.floor((x, y)) for x in (6, 7) for y in (5, 6)), "compact case surface is walkable")
        require(all(world.floor((x, y)) for x in (4, 5, 8) for y in (6, 7)), "long table was not removed")
        from itertools import product
        require(world.objects[VIKTOR]["graphics_id"] == "OBJ_EVENT_GFX_VAR_0", "rival sprite is not dynamic")
        for warp, choice, nickname, rescue, duel, flee, gender in product(
                world.map["warp_events"], CHOICES, (True, False, "cancel"),
                ("WON", "LOST", "DREW"), ("WON", "LOST", "DREW"), (False, True), ("MALE", "FEMALE")):
            scene = Scene(world, program, choice, (warp["x"], warp["y"]),
                          nickname=nickname, rescue=rescue, duel=duel, flee=flee, player_gender=gender)
            report["opening_cases"] += 1
            try:
                scene.run(MAP + "_OnLoad")
                require(scene.rival_graphics == ("MAY" if gender == "MALE" else "BRENDAN"), "rival appearance is not initialized on map load")
                scene.run(MAP + "_OnTransition")
                require(scene.variables["VAR_TEMP_0"] == 1, "first welcome was not queued")
                scene.run(PREFIX + "Welcome", opening=True)
                scene.run(MAP + "_OnTransition")
                require(scene.variables["VAR_TEMP_0"] == 0, "completed opening replays on re-entry")
                report["walked_tiles"] += scene.steps
            except (SceneError, KeyError, IndexError) as error:
                report["errors"].append(f"{choice}, player={gender}, entrance={warp['x']}, nickname={nickname}, rescue={rescue}, duel={duel}, flee={flee}: {error}")
                break
        for mom, introduced in product((False, True), repeat=2):
            scene = Scene(world, program, mom=mom, introduced=introduced)
            scene.run(MAP + "_OnTransition")
            require(scene.variables["VAR_TEMP_0"] == int(mom and not introduced), "welcome prerequisite/repeat guard is wrong")
            report["guard_cases"] += 1
        for choice in CHOICES:
            scene = Scene(world, program, choice, gift_success=False)
            scene.run(PREFIX + "Welcome", opening=True)
            require(not scene.removals and not scene.battles and not scene.given, "failed gift starts battles or removes a Ball")
            report["guard_cases"] += 1
            scene = Scene(world, program, choice)
            scene.run(world.case["script"])
            require(not scene.case_count and not scene.given, "display bypasses the opening")
            report["guard_cases"] += 1
    except (OSError, ValueError, KeyError, IndexError, StopIteration) as error:
        report["errors"].append(str(error))
    report["passed"] = not report["errors"]
    return report


def mutation_checks(root=ROOT):
    source = (Path(root) / f"data/maps/{MAP}/scripts.inc").read_text(encoding="utf-8")
    movement = MAP + "_Movement_"
    mutations = {
        "early_speech": (movement + "EnterLab:\n\twalk_up", movement + "EnterLab:\n\tface_up"),
        "missing_chase_wait": ("\twaitmovementall", "\tdelay 1"),
        "chase_collision": (movement + "MeowthChase:\n\twalk_fast_down", movement + "MeowthChase:\n\twalk_fast_right"),
        "walk_on_table": (movement + "ToCase:\n\twalk_up", movement + "ToCase:\n\twalk_up\n\twalk_up"),
        "wrong_ball": ("setflag FLAG_VIRELIA_HIDE_FIRE_STARTER", "setflag FLAG_VIRELIA_HIDE_WATER_STARTER"),
        "missing_case": ("special ChooseVireliaStarter", "setvar VAR_RESULT, 0"),
        "missing_rescue": ("special VireliaPrologue_StartRescueBattle", "delay 1"),
        "missing_cleanup": ("callnative VireliaPrologue_FinishRescue", "setvar VAR_RESULT, TRUE"),
        "wrong_rival_gender": ("goto_if_eq VAR_RESULT, MALE, " + PREFIX + "RivalGrassFemale",
                               "goto_if_eq VAR_RESULT, FEMALE, " + PREFIX + "RivalGrassFemale"),
        "missing_rival_appearance": ("callnative VireliaPrologue_SetRivalAppearance", "delay 1"),
        "escapee_survives": ("removeobject LOCALID_VIRELIA_LAB_MEOWTH", "delay 1"),
        "wrong_duel": ("trainerbattle_earlyrival TRAINER_VIRELIA_VIKTOR_GRASS", "trainerbattle_earlyrival TRAINER_VIRELIA_VIKTOR_WATER"),
        "wrong_actor": (MAP + "_Text_RivalWon, " + VIKTOR, MAP + "_Text_RivalWon, LOCALID_VIRELIA_LAB_GRASS_STARTER"),
        "wrong_named_slot": ("setvar VAR_0x8004, 0\n\tcall Common_EventScript_NameReceivedPartyMon", "setvar VAR_0x8004, 1\n\tcall Common_EventScript_NameReceivedPartyMon"),
        "missing_naming": ("call Common_EventScript_NameReceivedPartyMon", "delay 1"),
    }
    results = {}
    for name, (before, after) in mutations.items():
        require(before in source, f"mutation {name} no longer matches")
        results[name] = not validate(root, source.replace(before, after, 1))["passed"]
        require(results[name], f"accepted broken scene: {name}")
    return results


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--json", action="store_true")
    parser.add_argument("--self-test", action="store_true")
    args = parser.parse_args()
    report = validate()
    if args.self_test and report["passed"]:
        try:
            report["mutation_checks"] = mutation_checks()
        except SceneError as error:
            report["passed"] = False
            report["errors"].append(str(error))
    if args.json:
        print(json.dumps(report, indent=2))
    else:
        print(f"{'PASS' if report['passed'] else 'FAIL'}: lab opening, {report['opening_cases']} opening cases, {report['guard_cases']} guard cases")
        for error in report["errors"]:
            print(error)
        if "mutation_checks" in report:
            print(f"PASS: {len(report['mutation_checks'])} broken scene mutations rejected")
    return 0 if report["passed"] else 1


if __name__ == "__main__":
    sys.exit(main())
