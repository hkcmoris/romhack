#!/usr/bin/env python3
"""Validate Virelia assets and walking paths without a ROM toolchain.

This is a collision/elevation graph check, not a replacement for emulator QA.
Use --base <committed-starter-revision> to also enforce unchanged legacy assets.
"""

import argparse
from collections import deque
import json
import re
from pathlib import Path
import struct
import subprocess
import sys


ROOT = Path(__file__).resolve().parents[1]
NAMES = (
    "Virelia_MaplebrookTown",
    "Virelia_MaplebrookResearchLaboratory",
    "Virelia_Route01",
    "Virelia_SlatefordCity",
    "Virelia_MaplebrookPlayersHouse",
    "Virelia_Slateford_PokemonCenter",
    "Virelia_Slateford_QuarryEntrance",
    "Virelia_Slateford_QuarryDepths",
    "Virelia_Slateford_Gym",
    "Virelia_Slateford_QuarryGallery",
    "Virelia_MaplebrookCottage",
    "Virelia_WhisperwoodSanctuary",
    "Virelia_MaplebrookFishermansHouse",
    "Virelia_MaplebrookEmasHouse",
)
TOWN, LAB, ROUTE, CITY, HOME = NAMES[:5]
GROUPS = ("gMapGroup_VireliaOverworld", "gMapGroup_VireliaInteriors")
OPPOSITE = {"up": "down", "down": "up", "left": "right", "right": "left"}


def require(condition, message):
    if not condition:
        raise ValueError(message)


def read_json(path):
    return json.loads((ROOT / path).read_text(encoding="utf-8"))


def unique(values, description):
    require(len(values) == len(set(values)), f"Duplicate {description}")


def git(*args):
    return subprocess.check_output(
        ["git", *args], cwd=ROOT, stderr=subprocess.PIPE
    ).decode("utf-8")


def verify_legacy(base, groups, layouts, sections):
    old_groups = json.loads(git("show", f"{base}:data/maps/map_groups.json"))
    require(
        groups["group_order"][: len(old_groups["group_order"])] == old_groups["group_order"],
        "Existing map group indices changed",
    )
    for name in old_groups["group_order"]:
        if name not in GROUPS:
            require(groups[name] == old_groups[name], f"Existing map IDs changed: {name}")
    old_layouts = json.loads(git("show", f"{base}:data/layouts/layouts.json"))["layouts"]
    old_sections = json.loads(
        git("show", f"{base}:src/data/region_map/region_map_sections.json")
    )["map_sections"]
    old_legacy_layouts = [layout for layout in old_layouts if not layout["id"].startswith("LAYOUT_VIRELIA_")]
    current_legacy_layouts = [layout for layout in layouts if not layout["id"].startswith("LAYOUT_VIRELIA_")]
    require(current_legacy_layouts == old_legacy_layouts, "Existing non-Virelia layouts changed or moved")
    for old, current in zip(old_sections, sections):
        if old["id"].startswith("MAPSEC_VIRELIA_"):
            require({k: v for k, v in old.items() if k != "name"}
                    == {k: v for k, v in current.items() if k != "name"},
                    "Existing Virelia section IDs or coordinates changed")
        else:
            require(current == old, "Existing sections changed or moved")
    # Includes every original Hoenn and FRLG map, story script and binary layout.
    paths = git("ls-tree", "-r", "--name-only", base, "data/maps", "data/layouts").splitlines()
    legacy = [p for p in paths
              if p not in ("data/maps/map_groups.json", "data/layouts/layouts.json")
              and not p.startswith(("data/maps/Virelia_", "data/layouts/Virelia_"))]
    changed = set(git("diff", "--name-only", base, "--", "data/maps", "data/layouts").splitlines())
    require(not changed.intersection(legacy), f"Existing map assets changed: {sorted(changed.intersection(legacy))}")
    print(f"PASS: {len(legacy)} legacy map/layout files and non-Virelia IDs preserved from {base}")


def validate(base=None):
    groups = read_json("data/maps/map_groups.json")
    layouts = read_json("data/layouts/layouts.json")["layouts"]
    sections = read_json("src/data/region_map/region_map_sections.json")["map_sections"]
    unique(groups["group_order"], "map groups")
    unique([name for group in groups["group_order"] for name in groups[group]], "grouped maps")
    unique([layout["id"] for layout in layouts], "layout IDs")
    unique([section["id"] for section in sections], "map section IDs")
    require(len(sections) < 0xFD, "Map sections overlap reserved Pokemon met-location IDs")
    require(groups["group_order"][-2:] == list(GROUPS), "Virelia groups must be appended")
    require(groups[GROUPS[0]][:4] == [TOWN, ROUTE, CITY, "Virelia_WhisperwoodSanctuary"], "Unexpected Virelia overworld IDs")
    require(groups[GROUPS[1]][:8] == [LAB, HOME, *NAMES[5:11]], "Unexpected Virelia interior IDs")
    require(all(name in groups[GROUPS[1]] for name in ("Virelia_MaplebrookFishermansHouse", "Virelia_MaplebrookEmasHouse")),
            "Missing Maplebrook house interior")
    require(groups["group_order"].index(GROUPS[1]) < 0x7F, "Virelia group exceeds signed warp storage")
    if base:
        verify_legacy(base, groups, layouts, sections)

    by_layout = {entry["id"]: entry for entry in layouts}
    by_section = {entry["id"]: entry for entry in sections}
    maps = {name: read_json(f"data/maps/{name}/map.json") for name in NAMES}
    by_id = {entry["id"]: name for name, entry in maps.items()}
    unique([entry["id"] for entry in maps.values()], "Virelia map IDs")
    unique([entry["layout"] for entry in maps.values()], "Virelia layouts")
    # Reachability below checks the eventual completed state of dynamic gates.
    # The first-Gym validator separately proves locked-state puzzle gating.
    dynamic_open = {
        "Virelia_Slateford_QuarryDepths": {(11, 6)},
        "Virelia_Slateford_Gym": {(6, 6)},
        ROUTE: {(1, 8), (1, 9), (1, 16), (1, 17)},
        "Virelia_WhisperwoodSanctuary": {(16, 2), (16, 3), (20, 10), (20, 11)},
    }
    tileset_headers = (ROOT / "src/data/tilesets/headers.h").read_text()
    metatile_sources = (ROOT / "src/data/tilesets/metatiles.h").read_text()
    behavior_names = re.findall(r"\b(MB_[A-Z0-9_]+)\s*,", (ROOT / "include/constants/metatile_behaviors.h").read_text())
    counter_behavior = behavior_names.index("MB_COUNTER")
    tileset_attrs = {}

    def tile_behavior(name, x, y):
        tile = block(name, x, y)
        if tile is None:
            return None
        metatile = tile & 0x3FF
        layout = by_layout[maps[name]["layout"]]
        tileset = layout["primary_tileset" if metatile < 512 else "secondary_tileset"]
        if tileset not in tileset_attrs:
            header = re.search(r"const struct Tileset " + tileset + r"\s*=\s*\{(.*?)\};", tileset_headers, re.S)
            require(header is not None, f"Unknown tileset: {tileset}")
            symbol = re.search(r"\.metatileAttributes\s*=\s*(\w+)", header.group(1)).group(1)
            source = re.search(r"const u16 " + symbol + r"\[\] = INCBIN_U16\(\"([^\"]+)\"\)", metatile_sources)
            require(source is not None, f"Missing native attributes: {symbol}")
            raw_attrs = (ROOT / source.group(1)).read_bytes()
            tileset_attrs[tileset] = struct.unpack(f"<{len(raw_attrs) // 2}H", raw_attrs)
        attrs = tileset_attrs[tileset]
        offset = metatile if metatile < 512 else metatile - 512
        require(offset < len(attrs), f"{name}: invalid native metatile index {metatile}")
        return attrs[offset] & 0xFF

    def object_position(name, obj):
        # Reachability uses the completed chapter, when the manager has stepped
        # left and the open quarry mouth can be approached from the south.
        if name == CITY and obj.get("local_id") == "LOCALID_VIRELIA_SLATEFORD_MANAGER":
            return obj["x"] - 1, obj["y"]
        return obj["x"], obj["y"]

    blocks, occupied = {}, {}
    for name, entry in maps.items():
        # An omitted region defaults to each build's native region in mapjson.
        # Explicit Hoenn excludes these Emerald-only assets from FRLG builds.
        require(entry.get("region") == "REGION_HOENN",
                f"{name}: Emerald compiler region mismatch")
        layout = by_layout[entry["layout"]]
        require(layout["layout_version"] == "emerald", f"{name}: incorrect layout version")
        width, height = layout["width"], layout["height"]
        require(width > 0 and height > 0 and (width + 15) * (height + 14) <= 10240,
                f"{name}: invalid native map buffer dimensions")
        raw = (ROOT / layout["blockdata_filepath"]).read_bytes()
        require(len(raw) == width * height * 2, f"{name}: incorrect map.bin size")
        blocks[name] = struct.unpack(f"<{width * height}H", raw)
        require(len((ROOT / layout["border_filepath"]).read_bytes()) == 8, f"{name}: incorrect 2x2 border")
        section = by_section[entry["region_map_section"]]
        require(section.get("width", 0) > 0 and section.get("height", 0) > 0,
                f"{name}: region map dimensions would divide by zero")
        require(len(section["name"]) <= 19, f"{name}: region name exceeds native 20-byte buffer")
        require((ROOT / f"data/maps/{name}/scripts.inc").is_file(), f"{name}: missing scripts")
        occupied[name] = {object_position(name, obj) for obj in entry["object_events"]}
        require(len(occupied[name]) == len(entry["object_events"]), f"{name}: overlapping objects")
        for obj in entry["object_events"]:
            require(0 <= obj["x"] < width and 0 <= obj["y"] < height,
                    f"{name}: object outside map")
            require(obj["trainer_type"] in ("TRAINER_TYPE_NONE", "TRAINER_TYPE_NORMAL"),
                    f"{name}: unexpected trainer battle kind")

    def dimensions(name):
        layout = by_layout[maps[name]["layout"]]
        return layout["width"], layout["height"]

    def block(name, x, y):
        width, height = dimensions(name)
        return blocks[name][y * width + x] if 0 <= x < width and 0 <= y < height else None

    water_behaviors = {behavior_names.index(name) for name in (
        "MB_POND_WATER", "MB_OCEAN_WATER", "MB_DEEP_WATER", "MB_INTERIOR_DEEP_WATER",
        "MB_WATERFALL", "MB_SOOTOPOLIS_DEEP_WATER", "MB_EASTWARD_CURRENT", "MB_WESTWARD_CURRENT",
        "MB_NORTHWARD_CURRENT", "MB_SOUTHWARD_CURRENT",
    )}

    def walkable(node):
        name, x, y = node
        tile = block(name, x, y)
        return (tile is not None and (not tile & 0xC00 or (x, y) in dynamic_open.get(name, set()))
                and tile_behavior(*node) not in water_behaviors and (x, y) not in occupied[name])

    def same_elevation(a, b):
        elevation_a, elevation_b = block(*a) >> 12, block(*b) >> 12
        # Native ELEVATION_MULTI_LEVEL (15) keeps the actor's incoming elevation.
        return elevation_a == elevation_b or bool({0, 15} & {elevation_a, elevation_b})

    jump_directions = {behavior_names.index("MB_JUMP_" + name): delta for name, delta in (
        ("EAST", (1, 0)), ("WEST", (-1, 0)), ("SOUTH", (0, 1)), ("NORTH", (0, -1)),
    )}
    edges = {}
    for name in NAMES:
        width, height = dimensions(name)
        for y in range(height):
            for x in range(width):
                node = name, x, y
                if not walkable(node):
                    continue
                edges[node] = []
                for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                    other = name, x + dx, y + dy
                    if jump_directions.get(tile_behavior(*other)) == (dx, dy):
                        landing = name, x + 2 * dx, y + 2 * dy
                        if walkable(landing):
                            edges[node].append(landing)
                    elif walkable(other) and same_elevation(node, other):
                        edges[node].append(other)

    def warp_access(name, warp):
        node = name, warp["x"], warp["y"]
        # Animated Emerald doors have collision=1/elevation=0; the warp
        # triggers from the walkable tile immediately below the door.
        if not walkable(node):
            node = name, warp["x"], warp["y"] + 1
        require(walkable(node), f"{name}: inaccessible door/warp {warp}")
        return node

    warp_behaviors = {behavior_names.index(name) for name in (
        "MB_ANIMATED_DOOR", "MB_NON_ANIMATED_DOOR", "MB_DEEP_SOUTH_WARP", "MB_LADDER",
        "MB_EAST_ARROW_WARP", "MB_WEST_ARROW_WARP", "MB_NORTH_ARROW_WARP", "MB_SOUTH_ARROW_WARP",
    )}
    animated_door = behavior_names.index("MB_ANIMATED_DOOR")
    warp_points = []
    for name, entry in maps.items():
        for warp in entry["warp_events"]:
            behavior = tile_behavior(name, warp["x"], warp["y"])
            require(behavior in warp_behaviors,
                    f"{name}: warp tile has no native walking/door behavior: {warp}")
            require(behavior == animated_door or not block(name, warp["x"], warp["y"]) & 0xC00,
                    f"{name}: non-animated warp tile is blocked: {warp}")
            # Later chapters append a conventional exit; their own graph audit
            # checks its destination. Keep this audit focused on the old world.
            if warp["dest_map"] == "MAP_VIRELIA_RAINFALL_FOOTHILLS" and name == CITY:
                require((warp["x"], warp["y"]) == (12, 1), "Rainfall exit moved from its audited footprint")
                continue
            require(warp["dest_map"] in by_id, f"{name}: unexpected external warp")
            target_name = by_id[warp["dest_map"]]
            target_warps = maps[target_name]["warp_events"]
            target_id = int(warp["dest_warp_id"])
            require(0 <= target_id < len(target_warps), f"{name}: invalid target warp")
            arrival = target_warps[target_id]
            require(arrival["dest_map"] == entry["id"], f"{name}: warp does not return")
            source_node, target_node = warp_access(name, warp), warp_access(target_name, arrival)
            edges[source_node].append(target_node)
            warp_points.append(source_node)

        for connection in entry.get("connections") or []:
            direction, offset = connection["direction"], int(connection["offset"])
            require(connection["map"] in by_id, f"{name}: unexpected external connection")
            target_name = by_id[connection["map"]]
            source_layout = by_layout[entry["layout"]]
            target_layout = by_layout[maps[target_name]["layout"]]
            for key in ("primary_tileset", "secondary_tileset"):
                require(source_layout[key] == target_layout[key],
                        f"{name} -> {target_name}: connected maps must share {key} for native border rendering")
            require(any(back["map"] == entry["id"] and back["direction"] == OPPOSITE[direction]
                        and int(back["offset"]) == -offset
                        for back in maps[target_name].get("connections") or []),
                    f"{name}: missing reciprocal connection")
            width, height = dimensions(name)
            target_width, target_height = dimensions(target_name)
            seam_count = 0
            vertical = direction in ("up", "down")
            for coordinate in range(width if vertical else height):
                if vertical:
                    source_y, target_y = (0, target_height - 1) if direction == "up" else (height - 1, 0)
                    source_node = name, coordinate, source_y
                    target_node = target_name, coordinate - offset, target_y
                else:
                    source_x, target_x = (0, target_width - 1) if direction == "left" else (width - 1, 0)
                    source_node = name, source_x, coordinate
                    target_node = target_name, target_x, coordinate - offset
                if walkable(source_node) and walkable(target_node) and same_elevation(source_node, target_node):
                    edges[source_node].append(target_node)
                    seam_count += 1
            require(seam_count >= 2, f"{name}: connection has fewer than two passable tiles")

    def reachable(start):
        seen, queue = {start}, deque([start])
        while queue:
            for target in edges[queue.popleft()]:
                if target not in seen:
                    seen.add(target)
                    queue.append(target)
        return seen

    start = (TOWN, 9, 47)
    require(walkable(start), "Development arrival is blocked")
    seen = reachable(start)
    require({node[0] for node in seen} == set(NAMES), "Cannot reach every Virelia map from Maplebrook")
    require(set(warp_points) <= seen, "A building entrance/exit cannot be reached")
    center_doors = [warp for warp in maps[CITY]["warp_events"]
                    if warp["dest_map"] == maps["Virelia_Slateford_PokemonCenter"]["id"]]
    heals = [entry for entry in read_json("src/data/heal_locations.json")["heal_locations"]
             if entry["id"] == "HEAL_LOCATION_VIRELIA_SLATEFORD_POKEMON_CENTER"]
    require(len(center_doors) == 1 and len(heals) == 1, "Center needs one city entrance and heal location")
    door, heal = center_doors[0], heals[0]
    # Teleport and direct heal warps use the outdoor anchor, independently of
    # the nurse's indoor whiteout coordinates. Follow the authored entrance.
    require(heal["map"] == maps[CITY]["id"]
            and (heal["x"], heal["y"]) == (door["x"], door["y"] + 1)
            and (CITY, heal["x"], heal["y"]) in seen,
            "Center outdoor heal location must be reachable immediately below its city door")
    for name in NAMES:
        target = next(node for node in seen if node[0] == name)
        require(start in reachable(target), f"{name}: no return path to Maplebrook")
        for obj in maps[name]["object_events"]:
            x, y = object_position(name, obj)
            adjacent = {(name, x + dx, y + dy)
                        for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1))}
            # The engine extends A-button interaction by one tile across an
            # actual MB_COUNTER. A wall or arbitrary blocked tile never qualifies.
            for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                counter = name, x + dx, y + dy
                stand = name, x + 2 * dx, y + 2 * dy
                if tile_behavior(*counter) == counter_behavior and stand in seen:
                    adjacent.add(stand)
            require(bool(adjacent & seen), f"{name}: object cannot be approached: {obj.get('local_id', obj['script'])}")

    for name in (TOWN, "Virelia_MaplebrookCottage", "Virelia_MaplebrookFishermansHouse", "Virelia_MaplebrookEmasHouse"):
        for event in maps[name]["bg_events"]:
            if event.get("flag") == "FLAG_VIRELIA_MAPLEBROOK_WOODLAND_ITEM":
                # Its separate locked/open graph audit proves the Cut gate.
                continue
            adjacent = {(name, event["x"] + dx, event["y"] + dy)
                        for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1))}
            require(bool(adjacent & seen), f"{name}: sign or inspection cannot be approached: {event['script']}")

    for state in range(4):
        for x in (14, 15):
            gates = [event for event in maps[TOWN]["coord_events"]
                     if (event["x"], event["y"]) == (x, 1)
                     and event["var"] == "VAR_VIRELIA_PROLOGUE_STATE"
                     and int(event["var_value"]) == state
                     and event["script"] == "Virelia_MaplebrookTown_EventScript_NorthGate"]
            require(len(gates) == 1 and (TOWN, x, 2) in seen,
                    f"Maplebrook north gate missing or step-back blocked at x={x}, state={state}")

    general_attrs = (ROOT / "data/tilesets/primary/general/metatile_attributes.bin").read_bytes()
    attrs = struct.unpack(f"<{len(general_attrs) // 2}H", general_attrs)
    # MB_TALL_GRASS = 0x02 in Emerald; only primary General grass is used here.
    grass = [node for node in seen if node[0] == ROUTE and (block(*node) & 0x3FF) < len(attrs)
             and attrs[block(*node) & 0x3FF] & 0xFF == 0x02]
    require(grass, "No reachable native tall grass on Route 01")
    behavior_source = (ROOT / "src/metatile_behavior.c").read_text(encoding="utf-8")
    encounter_behaviors = {
        behavior_names.index(name) for name, flags in
        re.findall(r"\[(MB_[A-Z0-9_]+)\]\s*=([^,]+),", behavior_source)
        if "TILE_FLAG_HAS_ENCOUNTERS" in flags and "TILE_FLAG_SURFABLE" not in flags
    }
    for cave in ("Virelia_Slateford_QuarryEntrance", "Virelia_Slateford_QuarryDepths"):
        floors = [node for node in seen if node[0] == cave and tile_behavior(*node) in encounter_behaviors]
        require(floors, f"{cave}: no reachable native land-encounter floor")
        print(f"PASS: {cave}: {len(floors)} reachable encounter-enabled floor tiles")
    encounter_groups = read_json("src/data/wild_encounters.json")["wild_encounter_groups"]
    route_encounters = [entry for group in encounter_groups if group.get("for_maps")
                        for entry in group["encounters"] if entry["map"] == maps[ROUTE]["id"]]
    require(len(route_encounters) == 1, "Route 01 must have exactly one wild encounter table")
    land = route_encounters[0]["land_mons"]
    require(land["encounter_rate"] > 0 and len(land["mons"]) == 12, "Invalid land encounter table")
    require(all(1 <= mon["min_level"] <= mon["max_level"] <= 100 for mon in land["mons"]),
            "Invalid encounter levels")
    require(any(obj["flag"] == "FLAG_ITEM_VIRELIA_ROUTE01_POTION"
                for obj in maps[ROUTE]["object_events"]), "Missing optional route item")
    print(f"PASS: {len(NAMES)} maps, native layouts, reciprocal warps/connections, {len(seen)} reachable tiles")
    print(f"PASS: {len(grass)} reachable tall-grass tiles, 12 encounter slots, approachable NPCs/item")
    from validate_virelia_maplebrook import validate as validate_maplebrook
    validate_maplebrook()
    print("Run the native test/virelia_world.c suite and emulator checklist for runtime/save validation.")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--base", help="Committed starter revision for legacy preservation checks")
    args = parser.parse_args()
    try:
        validate(args.base)
    except (ValueError, KeyError, OSError, subprocess.CalledProcessError) as error:
        print(f"FAIL: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
