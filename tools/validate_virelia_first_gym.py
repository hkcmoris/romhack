#!/usr/bin/env python3
"""Check the first Gym's real map gates, encounter and script registrations.

This is source and collision-graph validation. Native regression tests and real
controller/emulator testing provide separate runtime evidence.
"""
from collections import deque
import json
from pathlib import Path
import re
import struct
import sys

ROOT = Path(__file__).resolve().parents[1]
NEW_NAMES = (
    "Virelia_Slateford_PokemonCenter",
    "Virelia_Slateford_QuarryEntrance", "Virelia_Slateford_QuarryDepths",
    "Virelia_Slateford_Gym", "Virelia_Slateford_QuarryGallery",
)
TRAINERS = (
    "TRAINER_VIRELIA_MINER_BRUNO", "TRAINER_VIRELIA_EXPLORER_LENKA",
    "TRAINER_VIRELIA_GYM_IVAN", "TRAINER_VIRELIA_GYM_DANA", "TRAINER_VIRELIA_MARA",
)


def require(condition, message):
    if not condition:
        raise ValueError(message)


def read(path):
    return (ROOT / path).read_text(encoding="utf-8-sig")


def load(path):
    return json.loads(read(path))


def reachable(layout, objects, start, gate, opened):
    width, height = layout["width"], layout["height"]
    raw = (ROOT / layout["blockdata_filepath"]).read_bytes()
    cells = struct.unpack(f"<{width * height}H", raw)
    occupied = {(obj["x"], obj["y"]) for obj in objects}

    def walkable(point):
        x, y = point
        return (0 <= x < width and 0 <= y < height and point not in occupied
                and (opened and point == gate or not cells[y * width + x] & 0xC00))

    require(walkable(start), f"Blocked puzzle arrival {start}")
    seen, queue = {start}, deque([start])
    while queue:
        x, y = queue.popleft()
        for point in ((x + 1, y), (x - 1, y), (x, y + 1), (x, y - 1)):
            if point not in seen and walkable(point):
                seen.add(point)
                queue.append(point)
    return seen


def adjacent(point):
    x, y = point
    return {(x + 1, y), (x - 1, y), (x, y + 1), (x, y - 1)}


def encounter_types(species):
    """Check team-building roles against species data, allowing roster edits."""
    definitions = {}
    for path in (ROOT / "src/data/pokemon/species_info").glob("*.h"):
        source = path.read_text(encoding="utf-8")
        entries = list(re.finditer(r"(?m)^\s*\[(SPECIES_\w+)\]\s*=", source))
        for index, entry in enumerate(entries):
            if entry[1] not in species:
                continue
            end = entries[index + 1].start() if index + 1 < len(entries) else len(source)
            types = re.search(r"\.types\s*=\s*MON_TYPES\(([^)]+)\)", source[entry.end():end])
            require(types is not None, f"Missing encounter types: {entry[1]}")
            definitions[entry[1]] = set(re.findall(r"TYPE_\w+", types[1]))
    require(species <= definitions.keys(),
            f"Unknown quarry species: {', '.join(sorted(species - definitions.keys()))}")
    return set().union(*(definitions[name] for name in species))


def validate_shared_services(entry, layout):
    city = load("data/maps/Virelia_SlatefordCity/map.json")
    warps = city["warp_events"]
    require([warp["dest_map"] for warp in warps[:3]] == [
        "MAP_VIRELIA_SLATEFORD_POKEMON_CENTER", "MAP_VIRELIA_SLATEFORD_GYM",
        "MAP_VIRELIA_SLATEFORD_QUARRY_ENTRANCE"],
        "Slateford must have one shared service entrance followed by Gym/quarry warps")
    # World validation checks the authored door's behavior and reachability.
    require(int(warps[0]["dest_warp_id"]) == 0,
            "Shared service entrance must enter the Center through warp 0")
    exits = entry["warp_events"]
    require([(warp["x"], warp["y"]) for warp in exits] == [(7, 8), (6, 8)],
            "Shared Center must retain both ordinary exit tiles")
    require(all(warp["dest_map"] == city["id"] and int(warp["dest_warp_id"]) == 0 for warp in exits),
            "Both service exits must return to the same city entrance")

    raw = (ROOT / layout["blockdata_filepath"]).read_bytes()
    cells = struct.unpack(f"<{len(raw) // 2}H", raw)
    headers, sources = read("src/data/tilesets/headers.h"), read("src/data/tilesets/metatiles.h")
    attributes = {}
    for offset, key in ((0, "primary_tileset"), (512, "secondary_tileset")):
        header = re.search(r"const struct Tileset " + layout[key] + r"\s*=\s*\{(.*?)\};", headers, re.S)
        require(header is not None, f"Unregistered service tileset: {layout[key]}")
        symbol = re.search(r"\.metatileAttributes\s*=\s*(\w+)", header[1]).group(1)
        source = re.search(r"const u16 " + symbol + r'\[\] = INCBIN_U16\("([^"]+)"\)', sources)
        require(source is not None, f"Missing service attributes: {symbol}")
        raw_attrs = (ROOT / source[1]).read_bytes()
        attributes[offset] = struct.unpack(f"<{len(raw_attrs) // 2}H", raw_attrs)
    behaviors = re.findall(r"\b(MB_[A-Z0-9_]+)\s*,", read("include/constants/metatile_behaviors.h"))
    stations = []
    for role in ("Nurse", "Clerk"):
        script = "Virelia_Slateford_PokemonCenter_EventScript_" + role
        objects = [obj for obj in entry["object_events"] if obj["script"] == script]
        require(len(objects) == 1, f"Shared Center needs exactly one {role.lower()}")
        obj = objects[0]
        x, y = obj["x"], obj["y"]
        if role == "Nurse":
            require((x, y) == (7, 2), "Nurse must retain the native heal/whiteout position")
        require(obj["movement_type"] == "MOVEMENT_TYPE_FACE_DOWN", f"{role}: wrong service-facing direction")
        cell = cells[(y + 1) * layout["width"] + x]
        tile = cell & 0x3FF
        offset = 512 if tile >= 512 else 0
        require(cell & 0xC00 and attributes[offset][tile - offset] & 255 == behaviors.index("MB_COUNTER"),
                f"{role}: interaction must cross an actual solid native counter")
        stations.append((x, y + 2))
    for warp in exits:
        seen = reachable(layout, entry["object_events"], (warp["x"], warp["y"]), None, False)
        require(set(stations) <= seen, "Both exits must reach nursing and shopping in one connected room")


def validate_nurse_path(center, start="Virelia_Slateford_PokemonCenter_EventScript_Nurse"):
    """Follow the actual nurse script and shared helpers, including fallthrough.

    The shared full-center entry reaches the Union Room name buffer. This local
    Center has no Union Room initialization, so using that entry can copy an
    unterminated name into gStringVar1 and corrupt later globals such as gFonts.
    """
    source = read("data/scripts/pkmn_center_nurse.inc") + "\n" + center
    labels = list(re.finditer(r"(?m)^([A-Za-z_][A-Za-z0-9_]*):{1,2}\s*$", source))
    blocks = {}
    for index, label in enumerate(labels):
        body = source[label.end():labels[index + 1].start() if index + 1 < len(labels) else len(source)]
        instructions = [line.strip() for line in body.splitlines()
                        if line.strip() and not line.lstrip().startswith(("@", ".", "#"))]
        targets = [args.split(",")[-1].strip() for args in
                   re.findall(r"(?m)^\s*(?:call|goto)(?:_[a-z_]+)*\s+([^\n]+)", body)]
        if instructions and index + 1 < len(labels):
            last = instructions[-1]
            if last not in ("end", "return") and not last.startswith("goto "):
                targets.append(labels[index + 1].group(1))
        blocks[label.group(1)] = body, targets
    require(start in blocks, "Local Center nurse event is missing")
    queue, visited = [start], set()
    while queue:
        label = queue.pop()
        if label in visited or label not in blocks:
            continue
        visited.add(label)
        body, targets = blocks[label]
        require("BufferUnionRoomPlayerName" not in body
                and "CheckTrainerHillAndUnionRoom" not in label,
                "Local nurse reaches an uninitialized Union Room name buffer")
        queue.extend(targets)
    reachable_source = "\n".join(blocks[label][0] for label in visited)
    require("dofieldeffect FLDEFF_POKECENTER_HEAL" in reachable_source
            and "special HealPlayerParty" in reachable_source,
            "Local nurse must use the ordinary healing animation and HealPlayerParty")
    require("EventScript_PkmnCenterNurse_ReturnPkmn" in visited,
            "Local nurse must return Pokemon through the ordinary message and bow helper")


def validate():
    groups = load("data/maps/map_groups.json")
    require(groups["gMapGroup_VireliaOverworld"][:3] == [
        "Virelia_MaplebrookTown", "Virelia_Route01", "Virelia_SlatefordCity"],
        "Existing overworld map IDs changed")
    # Later chapters and optional town interiors append after these fixed IDs.
    require(groups["gMapGroup_VireliaInteriors"][:2 + len(NEW_NAMES)] == [
        "Virelia_MaplebrookResearchLaboratory", "Virelia_MaplebrookPlayersHouse", *NEW_NAMES],
        "Unexpected merged-service interior map order")
    maps = {name: load(f"data/maps/{name}/map.json") for name in NEW_NAMES}
    for name, entry in maps.items():
        for obj in entry["object_events"]:
            if "local_id" in obj:
                require(re.fullmatch(r"[A-Za-z_][A-Za-z0-9_]*", obj["local_id"]) is not None,
                        f"{name}: local_id must be a C identifier, never a numeric string")
    layouts = {entry["id"]: entry for entry in load("data/layouts/layouts.json")["layouts"]}
    scripts = {name: read(f"data/maps/{name}/scripts.inc") for name in NEW_NAMES}
    city = read("data/maps/Virelia_SlatefordCity/scripts.inc")
    require("FLAG_VIRELIA_PROLOGUE_COMPLETE" in city, "Quarry entrance lacks prologue gate")
    city_map = load("data/maps/Virelia_SlatefordCity/map.json")
    quarry_warps = [warp for warp in city_map["warp_events"]
                    if warp["dest_map"] == "MAP_VIRELIA_SLATEFORD_QUARRY_ENTRANCE"]
    require(len(quarry_warps) == 1, "City must have exactly one quarry entrance")
    quarry_warp = quarry_warps[0]
    manager_id = "LOCALID_VIRELIA_SLATEFORD_MANAGER"
    managers = [obj for obj in city_map["object_events"] if obj.get("local_id") == manager_id]
    require(len(managers) == 1, "Quarry must have one manager with a stable local ID")
    manager = managers[0]
    require((manager["x"], manager["y"]) == (quarry_warp["x"], quarry_warp["y"] + 1),
            "Manager must block the approach immediately below the open quarry entrance")
    require(manager["movement_type"] == "MOVEMENT_TYPE_FACE_DOWN",
            "Manager must remain in place until quarry access is granted")
    for label, x in (("FirstGymOnLoad", manager["x"]), ("PositionManager", manager["x"] - 1)):
        block = re.search(r"^Virelia_SlatefordCity_" + label + r":\s*\n(.*?)(?=^[A-Za-z_]\w*:|\Z)", city, re.M | re.S)
        require(block and f"setobjectxyperm {manager_id}, {x}, {manager['y']}" in block[1],
                f"{label}: manager must restore the correct persistent position")
    require("call_if_set FLAG_VIRELIA_PROLOGUE_COMPLETE, Virelia_SlatefordCity_PositionManager" in city,
            "Manager must step aside only after the prologue grants access")
    opening = re.search(r"^Virelia_SlatefordCity_OpenQuarry:\s*\n(.*?)(?=^[A-Za-z_]\w*:|\Z)", city, re.M | re.S)
    require(opening and f"applymovement {manager_id}, Virelia_SlatefordCity_Movement_ManagerStepAside" in opening[1]
            and f"waitmovement {manager_id}" in opening[1] and f"copyobjectxytoperm {manager_id}" in opening[1],
            "Quarry access scene must animate and persist the manager's step aside")
    require(re.search(r"Movement_ManagerStepAside:\s+walk_left\s+face_down\s+step_end", city),
            "Manager must move exactly one tile left and face the player approach")
    require(not re.search(rf"setmetatile\s+{quarry_warp['x']},\s*{quarry_warp['y']},", city),
            "Quarry mouth must retain its authored open metatile")
    city_layout = layouts[city_map["layout"]]
    cells = struct.unpack(f"<{city_layout['width'] * city_layout['height']}H",
                          (ROOT / city_layout["blockdata_filepath"]).read_bytes())

    def city_tile(x, y):
        return cells[y * city_layout["width"] + x]

    for x, y in ((manager["x"], manager["y"]), (manager["x"] - 1, manager["y"]),
                 (quarry_warp["x"], quarry_warp["y"])):
        require(not city_tile(x, y) & 0xC00, "Quarry mouth and manager's step must remain passable terrain")
    for dx, dy in ((-1, 0), (1, 0), (0, -1)):
        require(city_tile(quarry_warp["x"] + dx, quarry_warp["y"] + dy) & 0xC00,
                "Quarry entrance must not bypass its manager from another side")
    require("FLAG_VIRELIA_QUARRY_COMPLETE" in city, "Gym entrance lacks quarry completion gate")
    require("VireliaFirstGym_BeginChapter" in city, "Completed saves cannot enter chapter on city load")
    scripts_all = "\n".join(scripts.values())
    for trainer in TRAINERS:
        require(len(re.findall(r"trainerbattle_single\s+" + trainer + r"\b", scripts_all)) == 1,
                f"Missing/duplicate ordinary single battle: {trainer}")
    wrappers = ("BeginChapter", "QuarrySwitch", "ReadEvidence", "CompleteQuarry", "GymSwitch",
                "AwardBadge", "AwardTM", "TakeQuarryItem")
    for suffix in wrappers:
        require(re.search(r"VireliaFirstGym_EventScript_" + suffix
                          + r"::?\s*callnative VireliaFirstGym_" + suffix + r"\s+return", scripts_all),
                f"Compiled event wrapper missing: {suffix}")
    for name, gate, arrival, target, panels, solved_flag, reset_fn in (
        ("Virelia_Slateford_QuarryDepths", (11, 6), (3, 19), (12, 3), ((20, 14), (4, 9)),
         "FLAG_VIRELIA_QUARRY_POWER_RESTORED", "VireliaFirstGym_ResetQuarryPuzzle"),
        ("Virelia_Slateford_Gym", (6, 6), (5, 18), (6, 2), ((3, 11), (9, 8)),
         "FLAG_VIRELIA_GYM_PUZZLE_COMPLETE", "VireliaFirstGym_ResetGymPuzzle"),
    ):
        layout = layouts[maps[name]["layout"]]
        locked = reachable(layout, maps[name]["object_events"], arrival, gate, False)
        unlocked = reachable(layout, maps[name]["object_events"], arrival, gate, True)
        require(not adjacent(target) & locked, f"{name}: puzzle can be bypassed")
        require(adjacent(target) & unlocked, f"{name}: completed puzzle does not reach objective")
        for panel in panels:
            require(adjacent(panel) & locked, f"{name}: switch cannot be reached before puzzle completion")
        require(solved_flag in scripts[name] and reset_fn in scripts[name],
                f"{name}: persistent opening or partial-input entry reset missing")
        require(re.search(rf"setmetatile\s+{gate[0]},\s*{gate[1]},\s*0x201,\s*0", scripts[name]),
                f"{name}: native gate does not open with passable floor")
    center = scripts[NEW_NAMES[0]]
    validate_shared_services(maps[NEW_NAMES[0]], layouts[maps[NEW_NAMES[0]]["layout"]])
    validate_nurse_path(center)
    require("setrespawn HEAL_LOCATION_VIRELIA_SLATEFORD" in center,
            "Center does not register its own whiteout destination")
    require("EventScript_PC" in center or any(obj.get("script") == "EventScript_PC"
            for obj in maps[NEW_NAMES[0]]["object_events"]), "Ordinary PC script missing")
    require("pokemart Virelia_Slateford_PokemonCenter_Pokemart" in center,
            "Combined Center does not use the ordinary purchase interface")
    for item in ("ITEM_POKE_BALL", "ITEM_POTION", "ITEM_SUPER_POTION", "ITEM_ANTIDOTE", "ITEM_PARALYZE_HEAL",
                 "ITEM_AWAKENING", "ITEM_REPEL", "ITEM_ESCAPE_ROPE"):
        require(item in center, f"Missing early-game Mart stock: {item}")
    encounter_groups = load("src/data/wild_encounters.json")["wild_encounter_groups"]
    quarry_species = set()
    for name in ("Virelia_Slateford_QuarryEntrance", "Virelia_Slateford_QuarryDepths"):
        entries = [entry for group in encounter_groups if group.get("for_maps")
                   for entry in group["encounters"] if entry["map"] == maps[name]["id"]]
        require(len(entries) == 1, f"{name}: expected one native encounter table")
        land = entries[0]["land_mons"]
        require(0 < land["encounter_rate"] <= 20, f"{name}: excessive/missing encounter pressure")
        require(len(land["mons"]) == 12, f"{name}: invalid encounter slots")
        require(all(8 <= mon["min_level"] <= mon["max_level"] <= 13 for mon in land["mons"]),
                f"{name}: encounter levels outside early quarry range")
        quarry_species.update(mon["species"] for mon in land["mons"])
    require({"TYPE_ROCK", "TYPE_GROUND", "TYPE_FIGHTING"} <= encounter_types(quarry_species),
            "Quarry lacks Rock, Ground and Fighting team-building opportunities")
    opponents = read("include/constants/opponents.h")
    ids = [int(re.search(r"#define\s+" + name + r"\s+(\d+)", opponents).group(1)) for name in TRAINERS]
    require(ids == list(range(866, 871)), "Chapter trainer IDs overlap old trainers or partner bank")
    require(re.search(r"#define\s+MAX_TRAINERS_COUNT_EMERALD\s+864\b", opponents),
            "Changing the original trainer flag bank shifts every Hoenn system flag")
    require("max(TRAINERS_COUNT, TRAINER_PARTNER(PARTNER_COUNT))" in read("src/trainer_slide.c"),
            "Trainer slide table cannot safely index appended trainers")
    party_source = read("src/data/trainers.party")
    trainer_constants = read("include/constants/trainers.h")
    for trainer in TRAINERS:
        team = re.search(r"=== " + trainer + r" ===\n(.*?)(?=\n=== |\Z)", party_source, re.S)
        require(team is not None, f"{trainer}: missing trainer party definition")
        portrait = re.search(r"^Pic: (.+)$", team[1], re.M)
        require(portrait is not None, f"{trainer}: missing stock portrait")
        portrait_id = "TRAINER_PIC_" + portrait[1].strip().upper().replace(" ", "_")
        require(re.search(r"\b" + portrait_id + r"\b", trainer_constants) is not None,
                f"{trainer}: unregistered stock portrait {portrait_id}")
    require('#include "constants/virelia_first_gym.h"' in read("data/map_events.s"),
            "Map object flags require chapter constants during assembly")
    core = read("src/virelia_first_gym.c")
    require(not re.search(r"Flag(?:Set|Clear)\(FLAG_(?:BADGE\d|SYS_)", core),
            "Chapter writes original Hoenn badges/system flags")
    require("VAR_VIRELIA_PROLOGUE_STATE" not in core, "Chapter mutates completed prologue state")
    require("MAX_TRAINERS_COUNT" not in read("include/global.h"), "Unexpected save-layout trainer dependency")
    print("PASS: current maps/trainers, story entry gates, five ordinary battles, native service scripts")
    print("PASS: one Center/Mart entrance, both exits, and connected native nurse/shop counters")
    print("PASS: locked/open collision graphs for quarry and Gym; both switches reachable and objectives gated")
    print(f"PASS: two encounter tables, {len(quarry_species)} quarry species, independent badge/Hoenn save flags")
    print("Native test suites and emulator playtest remain separate verification steps.")


def main():
    try:
        validate()
    except (OSError, ValueError, KeyError, AttributeError) as error:
        print(f"FAIL: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
