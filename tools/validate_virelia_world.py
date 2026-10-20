#!/usr/bin/env python3
"""Validate Virelia assets and walking paths without a ROM toolchain.

This is a collision/elevation graph check, not a replacement for emulator QA.
Use --base <committed-starter-revision> to also enforce unchanged legacy assets.
"""

import argparse
from collections import deque
import json
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
)
TOWN, LAB, ROUTE, CITY, HOME = NAMES
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
        if name in GROUPS:
            require(groups[name][: len(old_groups[name])] == old_groups[name],
                    f"Existing Virelia map IDs changed: {name}")
        else:
            require(groups[name] == old_groups[name], f"Existing map IDs changed: {name}")
    old_layouts = json.loads(git("show", f"{base}:data/layouts/layouts.json"))["layouts"]
    old_sections = json.loads(
        git("show", f"{base}:src/data/region_map/region_map_sections.json")
    )["map_sections"]
    require(layouts[: len(old_layouts)] == old_layouts, "Existing layouts changed or moved")
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
    print(f"PASS: {len(legacy)} legacy map/layout files and all existing IDs preserved from {base}")


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
    require(groups[GROUPS[0]] == [TOWN, ROUTE, CITY], "Unexpected Virelia overworld IDs")
    require(groups[GROUPS[1]] == [LAB, HOME], "Unexpected Virelia interior IDs")
    require(groups["group_order"].index(GROUPS[1]) < 0x7F, "Virelia group exceeds signed warp storage")
    if base:
        verify_legacy(base, groups, layouts, sections)

    by_layout = {entry["id"]: entry for entry in layouts}
    by_section = {entry["id"]: entry for entry in sections}
    maps = {name: read_json(f"data/maps/{name}/map.json") for name in NAMES}
    by_id = {entry["id"]: name for name, entry in maps.items()}
    unique([entry["id"] for entry in maps.values()], "Virelia map IDs")
    unique([entry["layout"] for entry in maps.values()], "Virelia layouts")
    blocks, occupied = {}, {}
    for name, entry in maps.items():
        require(entry["region"] == "REGION_HOENN", f"{name}: Emerald compiler region mismatch")
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
        occupied[name] = {(obj["x"], obj["y"]) for obj in entry["object_events"]}
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

    def walkable(node):
        name, x, y = node
        tile = block(name, x, y)
        return tile is not None and not tile & 0xC00 and (x, y) not in occupied[name]

    def same_elevation(a, b):
        elevation_a, elevation_b = block(*a) >> 12, block(*b) >> 12
        return elevation_a == elevation_b or 0 in (elevation_a, elevation_b)

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
                    if walkable(other) and same_elevation(node, other):
                        edges[node].append(other)

    def warp_access(name, warp):
        node = name, warp["x"], warp["y"]
        # Animated Emerald doors have collision=1/elevation=0; the warp
        # triggers from the walkable tile immediately below the door.
        if not walkable(node):
            node = name, warp["x"], warp["y"] + 1
        require(walkable(node), f"{name}: inaccessible door/warp {warp}")
        return node

    warp_points = []
    for name, entry in maps.items():
        for warp in entry["warp_events"]:
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
            require(any(back["map"] == entry["id"] and back["direction"] == OPPOSITE[direction]
                        and int(back["offset"]) == -offset
                        for back in maps[target_name].get("connections") or []),
                    f"{name}: missing reciprocal connection")
            require(direction in ("up", "down"), f"{name}: v0.1 expects north/south connections")
            width, height = dimensions(name)
            target_height = dimensions(target_name)[1]
            source_y, target_y = (0, target_height - 1) if direction == "up" else (height - 1, 0)
            seam_count = 0
            for x in range(width):
                source_node = name, x, source_y
                target_node = target_name, x - offset, target_y
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

    start = (TOWN, 14, 21)
    require(walkable(start), "Development arrival is blocked")
    seen = reachable(start)
    require({node[0] for node in seen} == set(NAMES), "Cannot reach every Virelia map from Maplebrook")
    require(set(warp_points) <= seen, "A building entrance/exit cannot be reached")
    for name in NAMES:
        target = next(node for node in seen if node[0] == name)
        require(start in reachable(target), f"{name}: no return path to Maplebrook")
        for obj in maps[name]["object_events"]:
            adjacent = {(name, obj["x"] + dx, obj["y"] + dy)
                        for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1))}
            require(bool(adjacent & seen), f"{name}: object cannot be approached: {obj.get('local_id', obj['script'])}")

    general_attrs = (ROOT / "data/tilesets/primary/general/metatile_attributes.bin").read_bytes()
    attrs = struct.unpack(f"<{len(general_attrs) // 2}H", general_attrs)
    # MB_TALL_GRASS = 0x02 in Emerald; only primary General grass is used here.
    grass = [node for node in seen if node[0] == ROUTE and (block(*node) & 0x3FF) < len(attrs)
             and attrs[block(*node) & 0x3FF] & 0xFF == 0x02]
    require(grass, "No reachable native tall grass on Route 01")
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
