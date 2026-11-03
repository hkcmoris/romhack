---
name: porymap-mapping
description: Create or edit Virelia maps in this pokeemerald-expansion repo. Use for layout geometry, metatile placement, collision and elevation, map events, warps, connections and encounters. Use gba-tileset-pipeline when the change is to the shared tileset artwork or definitions.
---

# Porymap mapping

Paths are relative to the repository root. Open that folder with Porymap's File > Open Project. For source edits, follow the same file relationships as the editor.

1. **Locate the map and its owner.** Start at `data/maps/<map>/map.json`. Resolve its layout ID in `data/layouts/layouts.json` to get dimensions, primary and secondary tilesets, and the actual block and border paths. Search the layout ID for other maps sharing that layout. Check the working diff and search the map name in `tools/virelia/`, `tools/validate_virelia_*.py` and the relevant `docs/VIRELIA_*.md`. The checked-in blocks and events may contain later hand edits. The legacy `generate_layouts.py` rewrites early maps, and `build_quarry_outdoor.py` replaces four quarry layouts. For a generator-owned change, inspect its writes and reconcile the current assets with its inputs before rebuilding. An isolated regeneration and diff exposes unrelated edits that would otherwise be lost.

2. **Edit the inputs the game consumes.** Use the Map and Collisions tabs for geometry. Each block in this fork's `map.bin` is a little-endian 16-bit word, row-major. The low ten bits select a metatile, the next two encode collision, and the top four encode elevation. Preserve unrelated fields on targeted edits. Check the metatile behavior for doors, ladders and encounter grass. For encounters, check the corresponding map entry in `src/data/wild_encounters.json`. For a new map, append it to the appropriate group in `data/maps/map_groups.json`, register a new layout when independent geometry is needed, and select the intended region section. Follow a neighboring map's `scripts.inc` and its inclusion in `data/event_scripts.s`. The build generates `events.inc`, `header.inc`, `connections.inc` and map constants from JSON. See `map_data_rules.mk` and the [new-map manual](https://huderlem.github.io/porymap/manual/creating-new-maps.html).

3. **Trace the playable route.** Use zero-based metatile coordinates. From spawn, trace the player's approach and exit across both sides of each changed transition. An animated door can have collision on its own cell and activate from below. Check its behavior, destination warp ID, arrival tile and return path. Numeric warp IDs follow array order. Moving or inserting a warp can change references from other maps. For a seamless two-way connection, match opposite directions and inverse offsets, check both edge openings and elevations, then save both maps. At an NPC or trigger, check the interaction side, elevation, script label and progression state. For a puzzle gate, test whether the closed state blocks alternate paths and whether the open state permits the intended route. [Events](https://huderlem.github.io/porymap/manual/editing-map-events.html) · [Connections](https://huderlem.github.io/porymap/manual/editing-map-connections.html)

4. **Validate the changed area.** Run `python3 tools/validate_virelia_world.py` from the root for registry and existing walking-path checks. Use an available Python 3 interpreter on Windows. The world validator enumerates specific maps. For a new route, extend the relevant check if the requested behavior falls outside its coverage. Select additional commands from the map-validation step in `.github/workflows/build.yml`. Examples:

   | Area changed | Targeted check |
   | --- | --- |
   | Maplebrook | `python3 tools/validate_virelia_maplebrook.py --self-test` |
   | Outdoor quarry | `python3 tools/validate_virelia_quarry.py --self-test` |
   | Rainfall / Seabreak | `python3 tools/validate_virelia_rainfall.py` |
   | First Gym gates | `python3 tools/validate_virelia_first_gym.py` |

   Review the resulting source and binary diff. For a playtest, rebuild, re-enter the edited map or start a fresh save, and walk the transition in both directions. Report which route and progression state were actually played and which were checked only by validators.
