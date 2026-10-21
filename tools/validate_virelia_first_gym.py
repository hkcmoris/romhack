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
    "Virelia_Slateford_PokemonCenter", "Virelia_Slateford_Mart",
    "Virelia_Slateford_QuarryEntrance", "Virelia_Slateford_QuarryDepths",
    "Virelia_Slateford_Gym",
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


def validate():
    groups = load("data/maps/map_groups.json")
    require(groups["gMapGroup_VireliaOverworld"] == [
        "Virelia_MaplebrookTown", "Virelia_Route01", "Virelia_SlatefordCity"],
        "Existing overworld map IDs changed")
    require(groups["gMapGroup_VireliaInteriors"] == [
        "Virelia_MaplebrookResearchLaboratory", "Virelia_MaplebrookPlayersHouse", *NEW_NAMES],
        "Interior map IDs must be append-only")
    maps = {name: load(f"data/maps/{name}/map.json") for name in NEW_NAMES}
    layouts = {entry["id"]: entry for entry in load("data/layouts/layouts.json")["layouts"]}
    scripts = {name: read(f"data/maps/{name}/scripts.inc") for name in NEW_NAMES}
    city = read("data/maps/Virelia_SlatefordCity/scripts.inc")
    require("FLAG_VIRELIA_PROLOGUE_COMPLETE" in city, "Quarry entrance lacks prologue gate")
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
        (NEW_NAMES[3], (11, 6), (3, 19), (12, 3), ((20, 14), (4, 9)),
         "FLAG_VIRELIA_QUARRY_POWER_RESTORED", "VireliaFirstGym_ResetQuarryPuzzle"),
        (NEW_NAMES[4], (6, 6), (5, 18), (6, 2), ((3, 11), (9, 8)),
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
    require("EventScript_PkmnCenterNurse" in center, "Center does not use standard healing")
    require("setrespawn HEAL_LOCATION_VIRELIA_SLATEFORD" in center,
            "Center does not register its own whiteout destination")
    require("EventScript_PC" in center or any(obj.get("script") == "EventScript_PC"
            for obj in maps[NEW_NAMES[0]]["object_events"]), "Ordinary PC script missing")
    mart = scripts[NEW_NAMES[1]]
    require("pokemart" in mart, "Mart does not use the ordinary purchase interface")
    for item in ("ITEM_POKE_BALL", "ITEM_POTION", "ITEM_SUPER_POTION", "ITEM_ANTIDOTE", "ITEM_PARALYZE_HEAL"):
        require(item in mart, f"Missing early-game Mart stock: {item}")
    encounter_groups = load("src/data/wild_encounters.json")["wild_encounter_groups"]
    quarry_species = set()
    for name in NEW_NAMES[2:4]:
        entries = [entry for group in encounter_groups if group.get("for_maps")
                   for entry in group["encounters"] if entry["map"] == maps[name]["id"]]
        require(len(entries) == 1, f"{name}: expected one native encounter table")
        land = entries[0]["land_mons"]
        require(0 < land["encounter_rate"] <= 20, f"{name}: excessive/missing encounter pressure")
        require(len(land["mons"]) == 12, f"{name}: invalid encounter slots")
        require(all(8 <= mon["min_level"] <= mon["max_level"] <= 13 for mon in land["mons"]),
                f"{name}: encounter levels outside early quarry range")
        quarry_species.update(mon["species"] for mon in land["mons"])
    require({"SPECIES_GEODUDE", "SPECIES_SANDSHREW", "SPECIES_MAKUHITA"} <= quarry_species,
            "Quarry lacks Rock, Ground and useful Rock-counter team-building opportunities")
    opponents = read("include/constants/opponents.h")
    ids = [int(re.search(r"#define\s+" + name + r"\s+(\d+)", opponents).group(1)) for name in TRAINERS]
    require(ids == list(range(866, 871)), "Chapter trainer IDs overlap old trainers or partner bank")
    require(re.search(r"#define\s+MAX_TRAINERS_COUNT_EMERALD\s+864\b", opponents),
            "Changing the original trainer flag bank shifts every Hoenn system flag")
    require("MAX(TRAINERS_COUNT, TRAINER_PARTNER(PARTNER_COUNT))" in read("src/trainer_slide.c"),
            "Trainer slide table cannot safely index appended trainers")
    core = read("src/virelia_first_gym.c")
    require(not re.search(r"Flag(?:Set|Clear)\(FLAG_(?:BADGE\d|SYS_)", core),
            "Chapter writes original Hoenn badges/system flags")
    require("VAR_VIRELIA_PROLOGUE_STATE" not in core, "Chapter mutates completed prologue state")
    require("MAX_TRAINERS_COUNT" not in read("include/global.h"), "Unexpected save-layout trainer dependency")
    print("PASS: append-only maps/trainers, story entry gates, five ordinary battles, native service scripts")
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
