#!/usr/bin/env python3
"""Audit Chapter 2 identifiers, real collision graph, services and source guards.

This source audit complements native FLASH/event tests and controller playtests;
it does not certify Android compatibility or measured chapter playtime.
"""
from collections import deque
import hashlib
import json
from pathlib import Path
import re
import struct
import sys
from validate_virelia_tilesets import rendered_tile
from virelia.localize_type_icons import read_png

ROOT = Path(__file__).resolve().parents[1]
BASELINE = '492267dadc0f9b54322a39038c479d717eba958a'
CITY = 'Virelia_SlatefordCity'
OUTDOORS = tuple('Virelia_' + name for name in (
    'RainfallFoothills', 'RainfallUpper', 'RainfallLower', 'RainfallLowlands', 'SeabreakTown', 'SeabreakApproaches'))
COAST_APPROACHES = OUTDOORS[-1]
INTERIORS = tuple('Virelia_Seabreak_' + name for name in (
    'PokemonCenter', 'Lighthouse', 'LanternRoom', 'KeepersHouse'))
NAMES = (CITY, *OUTDOORS, *INTERIORS)
DELTAS = ((1, 0), (-1, 0), (0, 1), (0, -1))
OPPOSITE = {'up': 'down', 'down': 'up', 'left': 'right', 'right': 'left'}
ROUTE_LINKS = ((CITY, OUTDOORS[0]), (OUTDOORS[0], OUTDOORS[1]), (OUTDOORS[0], OUTDOORS[2]),
               (OUTDOORS[2], OUTDOORS[3]), (OUTDOORS[1], COAST_APPROACHES), (OUTDOORS[3], COAST_APPROACHES),
               (COAST_APPROACHES, OUTDOORS[4]))
PROTECTED = {
    'include/constants/species.h': '40580ee3c7b6900859639df465dad05a5bdb7e40d32712ca173a8107a8880b48',
    'data/layouts/Virelia_WhisperwoodSanctuary/map.bin': 'ed93585f671788c530f6d51a74923de0444936d8e445a40ff7faa3b21bb88787',
    'data/layouts/Virelia_Route01/map.bin': 'd016edf56502bf6606063b18c4581af6cace5c253d8802bbca44c92a3e70d872',
    'data/layouts/Virelia_MaplebrookTown/map.bin': 'a9a782a6890e788137539586fb97e6178383b200631196c55930987d84695597',
    'data/layouts/Virelia_Slateford_Gym/map.bin': 'ac4dc01fd25aad0565fd245e5b181053a3b66193b39e24672d0a14f37a40395b',
}


def require(ok, message):
    if not ok:
        raise ValueError(message)


def text(path):
    return (ROOT / path).read_text(encoding='utf-8-sig')


def load(path):
    return json.loads(text(path))


def digest(value):
    return hashlib.sha256(json.dumps(value, sort_keys=True, separators=(',', ':')).encode()).hexdigest()


def words(path):
    raw = (ROOT / path).read_bytes()
    require(len(raw) % 2 == 0, f'Odd-sized native data: {path}')
    return struct.unpack(f'<{len(raw) // 2}H', raw)


def preprocessor_errors(source):
    """Check nesting after C line splicing and comment/string masking.

    This does not evaluate #if or replace a compiler. Physical start-line numbers
    remain available after backslash-newline continuations are joined.
    """
    logical, starts, pending, first = [], [], None, 1
    for number, line in enumerate(source.splitlines(keepends=True), 1):
        line = line.rstrip("\r\n")
        if pending is None:
            first, pending = number, ""
        if line.endswith("\\"):
            pending += line[:-1]
            continue
        logical.append(pending + line + "\n")
        starts.append(first)
        pending = None
    if pending is not None:
        logical.append(pending + "\n")
        starts.append(first)
    # Consume literals first so comment delimiters inside strings stay inert.
    tokens = re.compile(r""" "(?:\\.|[^"\\])*" | '(?:\\.|[^'\\])*' | /\*.*?\*/ | //[^\n]* """, re.S | re.X)
    masked = tokens.sub(lambda m: "".join("\n" if c == "\n" else " " for c in m[0]), "".join(logical))
    stack, errors = [], []
    for index, line in enumerate(masked.splitlines()):
        match = re.match(r"^\s*#\s*(if|ifdef|ifndef|elif|else|endif)\b", line)
        if not match:
            continue
        command, number = match[1], starts[index]
        if command in ("if", "ifdef", "ifndef"):
            stack.append([number, False])
        elif command == "endif":
            if stack:
                stack.pop()
            else:
                errors.append(f"line {number}: #endif without #if")
        elif not stack:
            errors.append(f"line {number}: #{command} without #if")
        elif stack[-1][1]:
            errors.append(f"line {number}: #{command} follows #else from conditional at line {stack[-1][0]}")
        elif command == "else":
            stack[-1][1] = True
    errors.extend(f"line {number}: unclosed conditional" for number, _ in stack)
    return errors


def validate_localized_conditionals():
    paths = ("src/data/items.h", "src/data/moves_info.h", "src/data/abilities.h",
             "src/data/pokemon/species_info/gen_1_families.h",
             "src/data/pokemon/species_info/gen_2_families.h",
             "src/data/pokemon/species_info/gen_3_families.h",
             "src/data/pokemon/species_info/virelia_families.h")
    errors = [f"{path}: {error}" for path in paths for error in preprocessor_errors(text(path))]
    require(not errors, "Localized C conditional nesting: " + "; ".join(errors))
    print(f"PASS: conditional nesting preserved across {len(paths)} localized C data headers")


def preserve_baseline(groups, layouts, sections):
    old_groups = dict(groups)
    old_groups['gMapGroup_VireliaOverworld'] = groups['gMapGroup_VireliaOverworld'][:4]
    old_groups['gMapGroup_VireliaInteriors'] = groups['gMapGroup_VireliaInteriors'][:8]
    require(digest(old_groups) == '55a0bc955f6c5e221ae1ceb629f00551aa7a8d150ec266a7aedbee6fc54e4956',
            'Existing map groups or map IDs moved from the chapter baseline')
    require(digest([x['id'] for x in layouts[:797]]) == '11cd0967f496dc86c7931293cfbe6e957f45c38200e81a8c7f9219f836fbd5ec',
            'Existing layout IDs moved')
    require(digest([x['id'] for x in sections[:212]]) == '86f6c9860f45bbebd03472094ce57749f2b963b169df8a11a0973bfb532c61ea',
            'Existing region section IDs moved')
    overworld = groups['gMapGroup_VireliaOverworld']
    require(overworld[4:9] == list(OUTDOORS[:5]) and overworld[9:11] == ['Virelia_Slateford_QuarryTerraces', COAST_APPROACHES],
            'Chapter 2 must preserve existing map IDs and append the coast approach')
    require(groups['gMapGroup_VireliaInteriors'][8:8 + len(INTERIORS)] == list(INTERIORS), 'Incorrect Chapter 2 interior map order')
    for path, expected in PROTECTED.items():
        data = (ROOT / path).read_bytes()
        if path.endswith('.h'):
            data = data.replace(b'\r\n', b'\n')
        require(hashlib.sha256(data).hexdigest() == expected, f'Protected baseline changed: {path}')
    make = text('Makefile')
    for flags in ('override CFLAGS', 'LDFLAGS'):
        require(re.search(r'^' + re.escape(flags) + r'\s*\+=.*-fno-isolate-erroneous-paths-dereference', make, re.M),
                f'My Boy UDF fix missing from {flags}')
    require(re.search(r'^MYBOY_DIAGNOSTIC_MODE\s*\?=\s*0\s*$', make, re.M), 'Diagnostic ROM mode enabled by default')
    require(re.search(r'^#define MYBOY_DIAGNOSTIC_MODE\s+0\s*$', text('include/config/general.h'), re.M),
            'Ordinary engine config enables My Boy diagnostics')
    print('PASS: baseline IDs, starter species and My Boy fix retained; current authored first-chapter geometry verified')


class World:
    def __init__(self, layouts, contract):
        self.maps = {name: load(f'data/maps/{name}/map.json') for name in NAMES}
        self.ids = {m['id']: name for name, m in self.maps.items()}
        registry = {entry['id']: entry for entry in layouts}
        self.layouts = {name: registry[m['layout']] for name, m in self.maps.items()}
        self.blocks = {name: words(l['blockdata_filepath']) for name, l in self.layouts.items()}
        self.scripts = {name: text(f'data/maps/{name}/scripts.inc') for name in NAMES}
        self.contract = contract
        self.behaviors = re.findall(r'\b(MB_[A-Z0-9_]+)\s*,', text('include/constants/metatile_behaviors.h'))
        self.attrs = {}
        self.metatiles = {}
        headers, sources = text('src/data/tilesets/headers.h'), text('src/data/tilesets/metatiles.h')
        for layout in self.layouts.values():
            for key in ('primary_tileset', 'secondary_tileset'):
                tileset = layout[key]
                if tileset in self.attrs:
                    continue
                header = re.search(r'const struct Tileset ' + tileset + r'\s*=\s*\{(.*?)\};', headers, re.S)
                require(header, f'Unregistered native tileset: {tileset}')
                symbol = re.search(r'\.metatileAttributes\s*=\s*(\w+)', header[1])[1]
                source = re.search(r'const u16 ' + symbol + r'\[\] = INCBIN_U16\("([^"]+)"\)', sources)
                require(source, f'Missing metatile attributes: {tileset}')
                self.attrs[tileset] = words(source[1])
                meta_symbol = re.search(r'\.metatiles\s*=\s*(\w+)', header[1])[1]
                meta_source = re.search(r'const u16 ' + meta_symbol + r'\[\] = INCBIN_U16\("([^"]+)"\)', sources)
                require(meta_source, f'Missing native metatiles: {tileset}')
                self.metatiles[tileset] = words(meta_source[1])

    def tile(self, node, flags=frozenset()):
        name, x, y = node
        layout = self.layouts[name]
        if not (0 <= x < layout['width'] and 0 <= y < layout['height']):
            return None
        block = self.blocks[name][y * layout['width'] + x]
        for gate in self.contract['dynamic_tiles']:
            if (name, x, y) == (gate['map'], gate['x'], gate['y']):
                key = 'open_block' if gate['flag'] in flags else 'closed_block'
                # Native setmetatile replaces the ID and collision but preserves the layout elevation.
                return (block & 0xF000) | (gate[key] & 0x0FFF)
        return block

    def behavior(self, node, flags=frozenset()):
        cell = self.tile(node, flags)
        if cell is None:
            return None
        layout = self.layouts[node[0]]
        index = cell & 1023
        key = 'primary_tileset' if index < 512 else 'secondary_tileset'
        attrs = self.attrs[layout[key]]
        require(index % 512 < len(attrs), f'{node}: metatile outside native bank')
        return self.behaviors[attrs[index % 512] & 255]

    def occupied(self, node, flags):
        return any((o['x'], o['y']) == node[1:] and o.get('flag', '0') not in flags
                   for o in self.maps[node[0]]['object_events'])

    def walkable(self, node, flags):
        cell = self.tile(node, flags)
        water = {'MB_POND_WATER', 'MB_OCEAN_WATER', 'MB_DEEP_WATER', 'MB_INTERIOR_DEEP_WATER',
                 'MB_WATERFALL', 'MB_SOOTOPOLIS_DEEP_WATER', 'MB_EASTWARD_CURRENT', 'MB_WESTWARD_CURRENT',
                 'MB_NORTHWARD_CURRENT', 'MB_SOUTHWARD_CURRENT'}
        return cell is not None and not cell & 0xC00 and not self.occupied(node, flags) and self.behavior(node, flags) not in water

    def warp_access(self, name, warp, flags):
        node = (name, warp['x'], warp['y'])
        if self.behavior(node, flags) == 'MB_ANIMATED_DOOR':
            node = (name, warp['x'], warp['y'] + 1)
        return node

    def connection_tiles(self, name, connection):
        """Pair the touching edge cells using Emerald's signed destination offset."""
        target = self.ids[connection['map']]
        src, dst = self.layouts[name], self.layouts[target]
        direction, offset = connection['direction'], int(connection['offset'])
        if direction in ('up', 'down'):
            y, ty = (0, dst['height'] - 1) if direction == 'up' else (src['height'] - 1, 0)
            for x in range(max(0, offset), min(src['width'], offset + dst['width'])):
                yield (name, x, y), (target, x - offset, ty)
        else:
            x, tx = (0, dst['width'] - 1) if direction == 'left' else (src['width'] - 1, 0)
            for y in range(max(0, offset), min(src['height'], offset + dst['height'])):
                yield (name, x, y), (target, tx, y - offset)

    def graph(self, flags=frozenset(), excluded=frozenset()):
        edges = {}
        jumps = {'MB_JUMP_EAST': (1, 0), 'MB_JUMP_WEST': (-1, 0), 'MB_JUMP_NORTH': (0, -1), 'MB_JUMP_SOUTH': (0, 1)}
        for name, layout in self.layouts.items():
            if name in excluded:
                continue
            for y in range(layout['height']):
                for x in range(layout['width']):
                    node = name, x, y
                    if not self.walkable(node, flags):
                        continue
                    targets = []
                    for dx, dy in DELTAS:
                        other = name, x + dx, y + dy
                        jumping = jumps.get(self.behavior(other, flags)) == (dx, dy)
                        if jumping:
                            other = name, x + 2 * dx, y + 2 * dy
                        if self.walkable(other, flags):
                            a, b = self.tile(node, flags) >> 12, self.tile(other, flags) >> 12
                            if jumping or a == b or 0 in (a, b):
                                targets.append(other)
                    edges[node] = targets
        for name, entry in self.maps.items():
            if name in excluded:
                continue
            for connection in entry.get('connections') or ():
                if self.ids.get(connection['map']) in excluded or connection['map'] not in self.ids:
                    continue
                for src, dst in self.connection_tiles(name, connection):
                    if src in edges and dst in edges:
                        a, b = self.tile(src, flags) >> 12, self.tile(dst, flags) >> 12
                        if a == b or 0 in (a, b):
                            edges[src].append(dst)
            for warp in entry['warp_events']:
                if warp['dest_map'] not in self.ids:
                    require(name == CITY, f'{name}: unexplained external warp')
                    continue
                target = self.ids[warp['dest_map']]
                if target in excluded:
                    continue
                arrival = self.maps[target]['warp_events'][int(warp['dest_warp_id'])]
                src, dst = self.warp_access(name, warp, flags), self.warp_access(target, arrival, flags)
                if src in edges and dst in edges:
                    edges[src].append(dst)
        return edges


def distances(edges, start):
    require(start in edges, f'Blocked graph start: {start}')
    found, queue = {start: 0}, deque([start])
    while queue:
        node = queue.popleft()
        for target in edges[node]:
            if target not in found:
                found[target] = found[node] + 1
                queue.append(target)
    return found


def validate_route_connections(world, graph, reached):
    required = {frozenset(link) for link in ROUTE_LINKS}
    outdoor_ids = {world.maps[n]['id'] for n in (CITY, *OUTDOORS)}
    observed = set()
    for name in (CITY, *OUTDOORS):
        entry = world.maps[name]
        neighbors = set()
        for connection in entry.get('connections') or ():
            if name == CITY and connection['map'] == 'MAP_VIRELIA_ROUTE01':
                continue
            target = world.ids.get(connection['map'])
            pair = frozenset((name, target))
            require(pair in required and target not in neighbors, f'{name}: unexpected connection to {target}')
            neighbors.add(target)
            observed.add(pair)
            direction, offset = connection['direction'], int(connection['offset'])
            require(direction in OPPOSITE and any(
                back['map'] == entry['id'] and back['direction'] == OPPOSITE[direction] and int(back['offset']) == -offset
                for back in world.maps[target].get('connections') or ()), f'{name}: missing reciprocal link to {target}')
            crossing = [(a, b) for a, b in world.connection_tiles(name, connection) if b in graph.get(a, ())]
            require(crossing and any(a in reached for a, b in crossing), f'{name}: no accessible walking seam to {target}')
            require(all(a in graph[b] for a, b in crossing), f'{name}: one-way edge opening to {target}')
            # Emerald picks the first matching connection, including the far endpoint.
            # Two neighbors can share a side (both Rainfall approaches west of Seabreak).
            adjacent = [c for c in entry['connections'] if c['direction'] == direction and c['map'] in world.ids]
            vertical = direction in ('up', 'down')
            for a, b in crossing:
                coord = a[1 if vertical else 2]
                chosen = next(c for c in adjacent if int(c['offset']) <= coord <= int(c['offset'])
                              + world.layouts[world.ids[c['map']]]['width' if vertical else 'height'])
                require(chosen == connection, f'{a}: another connection takes precedence over the trail to {target}')
                for point in (a, b):
                    behavior = world.behavior(point)
                    require('WARP' not in behavior and 'DOOR' not in behavior and behavior != 'MB_LADDER',
                            f'{point}: walking seam still has {behavior}')
        require(not any(w['dest_map'] in outdoor_ids for w in entry['warp_events']),
                f'{name}: outdoor traversal still uses a warp event')
        scripted_destinations = re.findall(r'^\s*warp\w*\s+(MAP_\w+)', world.scripts[name], re.M)
        require(not outdoor_ids.intersection(scripted_destinations), f'{name}: scripted warp bypasses an outdoor seam')
    require(observed == required, f'Missing route connections: {required - observed}')
    # Both branches form a loop on Porymap's canvas. Following either must
    # place Seabreak at the same world coordinates without overlapping a route.
    origins, queue = {CITY: (0, 0)}, deque([CITY])
    while queue:
        name = queue.popleft()
        x, y = origins[name]
        src = world.layouts[name]
        for link in world.maps[name]['connections']:
            target = world.ids.get(link['map'])
            if target is None:
                continue
            dst, off = world.layouts[target], int(link['offset'])
            dx, dy = {'up': (off, -dst['height']), 'down': (off, src['height']),
                      'left': (-dst['width'], off), 'right': (src['width'], off)}[link['direction']]
            origin = x + dx, y + dy
            if target in origins:
                require(origins[target] == origin, f'Conflicting Porymap position for {target} via {name}')
                continue
            for other, (ox, oy) in origins.items():
                size = world.layouts[other]
                require(origin[0] + dst['width'] <= ox or ox + size['width'] <= origin[0]
                        or origin[1] + dst['height'] <= oy or oy + size['height'] <= origin[1],
                        f'{target} overlaps {other} on the connection canvas')
            origins[target] = origin
            queue.append(target)
    print(f'PASS: all {len(required)} outdoor map boundaries have reciprocal, walkable connections')


def approaches(world, name, event, flags):
    x, y = event['x'], event['y']
    nodes = {(name, x + dx, y + dy) for dx, dy in DELTAS}
    for dx, dy in DELTAS:
        if world.behavior((name, x + dx, y + dy), flags) == 'MB_COUNTER':
            nodes.add((name, x + 2 * dx, y + 2 * dy))
    # Native facing interaction rejects objects on another nonzero elevation.
    # A geometric neighbor alone is not an accessible NPC after adding cliffs.
    elevation = event.get('elevation', 0)
    if elevation and name in OUTDOORS:
        nodes = {node for node in nodes
                 if world.tile(node, flags) is not None
                 and world.tile(node, flags) >> 12 in (0, elevation)}
    return nodes



def validate_actor_elevations(world):
    for name in OUTDOORS:
        for obj in world.maps[name]['object_events']:
            floor = world.tile((name, obj['x'], obj['y'])) >> 12
            authored = obj.get('elevation', 0)
            require(not floor or not authored or floor == authored,
                    f'{name}: {obj["script"]} elevation {authored} disagrees with floor {floor}')
    print('PASS: Chapter 2 outdoor actor elevations agree with terrain and interaction approaches')


def trace_slateford_entrance(source, badge):
    """Follow the actual entrance subroutine, including its flag branch."""
    labels = list(re.finditer(r"^([A-Za-z_]\w*)::?\s*$", source, re.M))
    blocks = {label[1]: [line.split("@", 1)[0].strip() for line in
              source[label.end():labels[i + 1].start() if i + 1 < len(labels) else len(source)].splitlines()
              if line.split("@", 1)[0].strip()]
              for i, label in enumerate(labels)}
    label, index, stack, restored = "Virelia_SlatefordCity_ConfigureRainfallGate", 0, [], {}
    for _ in range(128):
        require(label in blocks and index < len(blocks[label]), f"Incomplete entrance callback: {label}")
        instruction = blocks[label][index]
        index += 1
        command, _, operands = instruction.partition(" ")
        args = [value.strip() for value in operands.split(",")]
        if command == "setmetatile":
            require(len(args) == 4, "Malformed entrance restoration metatile command")
            x, y, tile, collision = (int(value, 0) for value in args)
            restored[x, y] = tile, bool(collision)
        elif command in ("goto_if_set", "goto_if_unset"):
            require(args[0] == "FLAG_VIRELIA_FIRST_GYM_COMPLETE", "Entrance callback changed its badge prerequisite")
            if badge == (command == "goto_if_set"):
                label, index = args[1], 0
        elif command == "call":
            stack.append((label, index))
            label, index = args[0], 0
        elif command == "goto":
            label, index = args[0], 0
        elif command == "return":
            if not stack:
                return restored
            label, index = stack.pop()
        else:
            raise ValueError(f"Unsupported entrance callback instruction: {instruction}")
    raise ValueError("Entrance callback did not return within its bounded restoration")


def validate_cached_entrance(world):
    source = world.scripts[CITY]
    require("call Virelia_SlatefordCity_ConfigureRainfallGate" in source,
            "Slateford load callback does not restore the entrance over cached map views")
    # These ten authored cells are inside the 3x4 north patch. Row3 side cells
    # remain original grass, so the callback deliberately does not repaint them.
    footprint = {(x, y) for x in (11, 12, 13) for y in (0, 1, 2)} | {(12, 3)}
    for badge in (False, True):
        restored = trace_slateford_entrance(source, badge)
        require(footprint <= restored.keys(), f"Cached Continue leaves entrance cells unrestored: {sorted(footprint - restored.keys())}")
        for x, y in footprint:
            raw = world.blocks[CITY][y * world.layouts[CITY]['width'] + x]
            expected = (raw & 1023, bool(raw & 0xC00))
            if (x, y) == (12, 2):
                expected = (1, False) if badge else (0x00E, True)
            require(restored[x, y] == expected, f"Cached entrance art/collision mismatch at {(x, y)}, badge={badge}")
        require(all(not restored[12, y][1] and world.behavior((CITY, 12, y)) == 'MB_NORMAL' for y in (0, 1)),
                "The restored trail must reach the north map boundary on ordinary walking tiles")
    print("PASS: actual badge-branch callback restores all10 authored entrance cells over an old cached map view")

def validate_graphics_counts(source=None):
    """Match build-time graphics limits to actual PNG tile capacity."""
    if source is None:
        source = text("src/data/tilesets/graphics.h")
    counts = {}
    for suffix in ("VireliaRainfall", "VireliaCoast", "VireliaLighthouse"):
        symbol = "gTilesetTiles_" + suffix
        declaration = re.search(r"const u32 " + symbol + r"\[\]\s*=\s*INCGFX_U32\((.*?)\);", source, re.S)
        require(declaration, f"Missing graphics declaration: {symbol}")
        path = re.search(r'"([^"\n]+\.png)"', declaration[1])
        count = re.search(r"-num_tiles\s+(\d+)\b", declaration[1])
        require(path and count, f"Missing PNG/count in graphics declaration: {symbol}")
        png = (ROOT / path[1]).read_bytes()
        require(png[:8] == b"\x89PNG\r\n\x1a\n" and len(png) >= 24, f"Invalid PNG header: {path[1]}")
        width, height = struct.unpack(">II", png[16:24])
        require(width > 0 and height > 0 and width % 8 == height % 8 == 0, f"Invalid tile dimensions: {path[1]}")
        actual, declared = width * height // 64, int(count[1])
        require(actual == declared and 0 < actual <= 512,
                f"{symbol}: declared {declared} tiles, PNG has {actual} ({width}x{height})")
        counts["gTileset_" + suffix] = actual
    print("PASS: new tileset PNG capacities match declared graphics counts " + str(counts))
    return counts

def validate_used_palettes(world):
    graphics_counts = validate_graphics_counts()
    rainfall_attrs = world.attrs['gTileset_VireliaRainfall']
    normal = world.behaviors.index('MB_NORMAL')
    bridge_ids = {0x320, 0x321}
    for name, layout in world.layouts.items():
        if layout['secondary_tileset'] == 'gTileset_VireliaRainfall':
            decks = set(range(0x324, 0x32C)) | set(range(0x355, 0x361)) | set(range(0x370, 0x377))
            bridge_ids.update((cell & 1023) for cell in world.blocks[name] if (cell & 1023) in decks)
    for bridge in sorted(bridge_ids):
        require(bridge - 512 < len(rainfall_attrs), f'Missing Rainfall bridge alias {bridge:#x}')
        attribute = rainfall_attrs[bridge - 512]
        require((attribute >> 12) & 3 == 1 and attribute & 255 == normal,
                f'Rainfall bridge {bridge:#x} must use Covered layers and normal walking behavior so actors remain visible')
    limit = int(re.search(r'^#define NUM_PALS_TOTAL\s+(\d+)', text('include/fieldmap.h'), re.M)[1])
    checked = set()
    for name, layout in world.layouts.items():
        used = {value & 1023 for value in world.blocks[name]}
        used.update(value & 1023 for value in words(layout['border_filepath']))
        # Include callback-only beacon/gate metatiles, even before they are shown.
        used.update(int(v, 0) for v in re.findall(r'\bsetmetatile\s+\d+,\s*\d+,\s*(0x[0-9A-Fa-f]+|\d+),', world.scripts[name]))
        for index in used:
            bank = layout['primary_tileset' if index < 512 else 'secondary_tileset']
            offset = index % 512
            require(offset < len(world.attrs[bank]), f'{name}: used metatile {index:#x} has no attributes')
            meta = world.metatiles[bank][offset * 8:(offset + 1) * 8]
            require(len(meta) == 8, f'{name}: used metatile {index:#x} is incomplete')
            require(all(piece >> 12 < limit for piece in meta),
                    f'{name}: used {bank} metatile {index:#x} references unloaded palette(s) '
                    + str(sorted({piece >> 12 for piece in meta if piece >> 12 >= limit})))
            secondary = layout['secondary_tileset']
            if secondary in graphics_counts:
                require(all((piece & 1023) < 512 + graphics_counts[secondary] for piece in meta),
                        f'{name}: used metatile {index:#x} references a secondary graphic beyond its PNG capacity')
            checked.add((bank, index))
    print(f'PASS: {len(checked)} used native metatiles, including callback states, use loaded palettes0-{limit - 1}')


def validate_spruce_art():
    """Check both halves and the foreground crown against the artist's indexed pixels."""
    sheet, _, chunks = read_png(ROOT / 'data/tilesets/secondary/virelia_rainfall/fen.png')
    colors = [tuple(c >> 3 for c in rgb) for rgb in struct.iter_unpack('3B', dict(chunks)[b'PLTE'])]
    tree = [row[64:96] for row in sheet[32:80]]
    overlap = [[crown or base for crown, base in zip(top, bottom)] for top, bottom in zip(tree[:16], tree[32:])]
    rows = tree + overlap
    for bank, first in (('virelia_rainfall', 0x389), ('virelia_coast', 0x248)):
        metatiles = words(f'data/tilesets/secondary/{bank}/metatiles.bin')
        for panel in range(8):
            offset = (first + panel - 512) * 8 + 4
            for q, ref in enumerate(metatiles[offset:offset + 4]):
                x, y = panel % 2 * 16 + q % 2 * 8, panel // 2 * 16 + q // 2 * 8
                expected = tuple(colors[i] if i else None for row in rows[y:y + 8] for i in row[x:x + 8])
                require(rendered_tile(bank, ref) == expected,
                        f'{bank}: spruce metatile {first + panel:#x} quarter {q} differs from fen.png')
    print('PASS: both banks preserve the complete spruce crown, directional shading and stacked overlap')


def validate_gravel_edges(world):
    """A painted trail may expose the verge, but not enclosed specks of dirt."""
    table = world.metatiles['gTileset_VireliaRainfall']
    pixels = {(x,y) for x in range(16) for y in range(16)}
    border = {(x,y) for x,y in pixels if x in (0,15) or y in (0,15)}

    def flood(seeds, allowed):
        seen = set(seeds)
        pending = list(seen)
        while pending:
            x,y = pending.pop()
            for dx,dy in DELTAS:
                neighbor = x+dx,y+dy
                if neighbor in allowed and neighbor not in seen:
                    seen.add(neighbor)
                    pending.append(neighbor)
        return seen

    for tile in (*range(0x3AE,0x3BD), 0x31A,0x31B,0x31E,0x31F):
        start = (tile-512)*8+4
        painted = set()
        for q,ref in enumerate(table[start:start+4]):
            for i,color in enumerate(rendered_tile('virelia_rainfall', ref)):
                if color is not None:
                    painted.add((8*(q%2)+i%8, 8*(q//2)+i//8))

        bare = pixels - painted
        require(painted and flood([min(painted)], painted) == painted,
                f'Gravel {tile:#05x} leaves disconnected fragments on the adjoining surface')
        require(flood(border & bare, bare) == bare, f'Gravel {tile:#05x} has a hole inside its surface')
    print('PASS: connected gravel surfaces and open outer and inner verges throughout the ridge trail')


def validate_tree_footprints(world):
    """Read the drawn trees back from the blockmaps, including shared crown/base rows."""
    for name in OUTDOORS:
        coast = world.layouts[name]['secondary_tileset'] == 'gTileset_VireliaCoast'
        spruce = (0x248, 0x24A, 0x24C, 0x24E) if coast else (0x389, 0x38B, 0x38D, 0x38F)
        families = {0x1D6: (0x1CE, 0x1D6, 0x1E6, 0x1DE), spruce[1]: spruce}
        tree_tiles = {tile + dx for family in families.values() for tile in family for dx in (0, 1)}
        width = world.layouts[name]['width']
        cells = {(i % width, i // width): block for i, block in enumerate(world.blocks[name])
                 if block & 1023 in tree_tiles}
        trunks = {point for point, block in cells.items() if block & 1023 in families}
        require(trunks or name == COAST_APPROACHES, f'{name}: no complete trees')
        covered = set()
        for x, y in trunks:
            crown, trunk, roots, overlap = families[cells[x, y] & 1023]
            above = cells.get((x, y - 2), 0) & 1023 == trunk
            below = cells.get((x, y + 2), 0) & 1023 == trunk
            elevation = cells[x, y] >> 12
            for dy, pair, solid in ((-1, overlap if above else crown, above), (0, trunk, True),
                                    (1, overlap if below else roots, True)):
                for dx in (0, 1):
                    tile = pair + dx
                    point = x + dx, y + dy
                    block = cells.get(point, 0)
                    require(block & 1023 == tile and bool(block & 0xC00) == solid and block >> 12 == elevation,
                            f'{name} {point}: expected tree tile {tile:#05x}, solid={solid}')
                    covered.add(point)
        require(covered == cells.keys(), f'{name}: orphaned tree pieces at {cells.keys() - covered}')
    print('PASS: complete two-by-two tree footprints with walkable crowns or solid stacked overlaps')


def validate_marsh(world):
    # For a concave corner the two adjacent cardinals are wet but their shared diagonal is dry.
    edges = ('NW', 'N', 'NE', 'W', '', 'E', 'SW', 'S', 'SE')
    notches = ('NW', 'NE', 'SW', 'SE')
    shapes = [(set(edge), set()) for edge in edges] + [(set(), {notch}) for notch in notches]
    first = 0x32D
    bank = world.layouts['Virelia_RainfallLower']['secondary_tileset']
    start = first - 512
    attrs = world.attrs[bank][start:start + len(shapes)]
    require(len(attrs) == len(shapes) and all(world.behaviors[a & 255] == 'MB_PUDDLE' and a >> 12 == 1 for a in attrs),
            'All 13 marsh pieces need puddle behavior and the Covered drawing layer')
    cardinals = {'N': (0, -1), 'S': (0, 1), 'W': (-1, 0), 'E': (1, 0)}
    for name in OUTDOORS[:4]:
        width = world.layouts[name]['width']
        marsh = {(i % width, i // width): block for i, block in enumerate(world.blocks[name])
                 if first <= block & 1023 < first + len(shapes)}
        if name == 'Virelia_RainfallLower':
            require(marsh, 'Rainfall Lower has no marsh terrain')
        for (x, y), block in marsh.items():
            dry_edges = {d for d, (dx, dy) in cardinals.items() if (x + dx, y + dy) not in marsh}
            dry_notches = {v + h for v, h in notches if v not in dry_edges and h not in dry_edges
                           and (x + cardinals[h][0], y + cardinals[v][1]) not in marsh}
            tile = block & 1023
            require((dry_edges, dry_notches) == shapes[tile - first],
                    f'{name} {(x, y)}: marsh tile {tile:#05x} does not fit edges {dry_edges}, notches {dry_notches}')
            require(not block & 0xC00 and world.behavior((name, x, y)) == 'MB_PUDDLE',
                    f'{name} {(x, y)}: marsh must be passable shallow ground')
    print('PASS: marsh shores and inverted corners follow the wet footprint')


def validate_rainfall_cascade(world, reached, flags):
    """Trace each rope bridge across the falls, from upstream lip to plunge pool."""
    name = 'Virelia_RainfallUpper'
    width, height = world.layouts[name]['width'], world.layouts[name]['height']
    def tile(x, y):
        return world.blocks[name][y * width + x] & 1023

    levels = set()
    for y in range(1, height - 2):
        for x in range(1, width - 2):
            if tile(x, y) != 0x35B:
                continue
            right = x + 1
            while right < width and tile(right, y) == 0x35C:
                right += 1
            require(right > x + 1 and right < width and tile(right, y) == 0x35D,
                    f'Broken back rail at {(x, y)}')

            def row_matches(yy, first):
                return all(tile(xx, yy) == first + (0 if xx == x else 2 if xx == right else 1)
                           for xx in range(x, right + 1))

            require(row_matches(y + 1, 0x35E), f'Incomplete rope deck at {(x, y)}')
            top, bottom = y - 1, y + 2
            while top > 0 and row_matches(top, 0x34F):
                top -= 1
            while bottom < height - 1 and row_matches(bottom, 0x34F):
                bottom += 1
            require(row_matches(top, 0x34C) and row_matches(bottom, 0x352),
                    f'Water does not flow from lip to pool under bridge at {(x, y)}')
            for yy in range(top, bottom):
                for xx in range(x, right + 1):
                    node = name, xx, yy
                    if yy in (y, y + 1):
                        require(node in reached and world.behavior(node, flags) == 'MB_NORMAL',
                                f'Bridge over falls is impassable at {node}')
                    else:
                        require(world.behavior(node, flags) == 'MB_WATERFALL' and world.tile(node, flags) & 0xC00,
                                f'Broken cascade at {node}')
            levels.add(world.tile((name, x, y), flags) >> 12)
    require(len(levels) >= 2, 'Upper Pass needs playable waterfall spans at different heights')
    print(f'PASS: ridge bridges cross continuous waterfalls at elevations {sorted(levels)}')


def validate_seabreak_coast(world, reachable):
    town = 'Virelia_SeabreakTown'
    layout = world.layouts[town]
    width, height = layout['width'], layout['height']
    sea = {(x, y) for y in range(height) for x in range(width)
           if world.behavior((town, x, y)) in {'MB_OCEAN_WATER', 'MB_DEEP_WATER', 'MB_POND_WATER'}}
    water_ids = {world.tile((town, x, y)) & 1023 for x, y in sea}
    require(all(block & 1023 not in water_ids for block in words(layout['border_filepath'])),
            'Seabreak needs an inland border behind the western Rainfall entrances')
    for _, y, _ in world.contract['arrivals'].values():
        require(all((0, yy) not in sea for yy in range(y - 1, y + 2)),
                f'Seabreak is cut off from the mainland west of row {y}')
    require((0, 0) not in sea, 'The northwestern coastal slopes must continue inland')

    # The sea stack shares one metatile with the cape's inside shoreline corner.
    # All four parts of the rock must survive shoreline autotiling.
    primary = world.metatiles[layout['primary_tileset']]
    for dy, row in enumerate(((0x150, 0x151), (0x158, 0x159))):
        for dx, source in enumerate(row):
            node = (town, 43 + dx, 20 + dy)
            block = world.tile(node)
            tile = block & 1023
            bank = layout['primary_tileset' if tile < 512 else 'secondary_tileset']
            offset = (tile % 512) * 8
            for q in range(4, 8):
                rock = primary[source * 8 + q]
                if rock & 1023:
                    require(world.metatiles[bank][offset + q] == rock,
                            f'{node}: missing offshore rock artwork')
            require(block & 0xC00 and world.behavior(node) == 'MB_NORMAL',
                    f'{node}: offshore rock must block passage')

    # A navigable basin must open east. Trace water past the pier without
    # treating boats, piles or the seawall as part of the shipping channel.
    offshore = {(width - 1, y) for y in range(height)} & sea
    queue = deque(offshore)
    while queue:
        x, y = queue.popleft()
        for dx, dy in DELTAS:
            neighbor = x + dx, y + dy
            if neighbor in sea and neighbor not in offshore:
                offshore.add(neighbor)
                queue.append(neighbor)
    fisher = next(obj for obj in world.maps[town]['object_events'] if obj['script'].endswith('_Fisherman'))
    require(any(abs(x - fisher['x']) + abs(y - fisher['y']) <= 3 for x, y in offshore),
            'The fishing pier must face water connected to the eastern sea')
    for x, y in ((8, 46), (23, 45), (39, 42)):
        require((town, x, y) in reachable, f'Cargo blocks access to the pier head at {(x, y)}')

    # Emerald repeats the same 2x2 border on every side. An on-map strip of
    # ocean must fill the camera beyond the furthest walkable dock or headland.
    for _, x, y in reachable:
        if (width - 1, y) in offshore:
            require(x + 8 < width, f'The inland border is visible beyond the eastern sea at {(x, y)}')
        if (x, height - 1) in offshore:
            require(y + 6 < height, f'The inland border is visible beyond the harbor at {(x, y)}')
    print('PASS: Seabreak joins the western mainland and its fishing pier opens to the eastern sea')


def validate_encounters_and_trainers(world, graph, reached, flags):
    tables = [entry for group in load('src/data/wild_encounters.json')['wild_encounter_groups']
              if group.get('for_maps') for entry in group['encounters']
              if entry['map'] in {world.maps[n]['id'] for n in OUTDOORS[:4]}]
    require(len(tables) == 4 and len({entry['map'] for entry in tables}) == 4,
            'Each Rainfall zone must have one encounter table')
    known = set(re.findall(r'\bSPECIES_[A-Z0-9_]+\b', text('include/constants/species.h')))
    signatures = set()
    for table in tables:
        land = table['land_mons']
        require(0 < land['encounter_rate'] <= 25 and len(land['mons']) == 12,
                f'{table["map"]}: excessive rate or incorrect native land slots')
        for mon in land['mons']:
            require(14 <= mon['min_level'] <= mon['max_level'] <= 19, 'Rainfall wild levels outside14-19')
            require(mon['species'] in known, f'Unknown wild species: {mon["species"]}')
        signatures.add(tuple(m['species'] for m in land['mons']))
        name = world.ids[table['map']]
        require(any(world.behavior(n, flags) in {'MB_TALL_GRASS', 'MB_LONG_GRASS'}
                    for n in reached if n[0] == name), f'{name}: no reachable encounter grass')
    require(len(signatures) >= 3, 'Rock/forest/marsh/coast encounter tables lack meaningful variety')
    teams = dict(re.findall(r'^=== (TRAINER_\w+) ===\n(.*?)(?=^=== |\Z)', text('src/data/trainers.party'), re.M | re.S))
    trainers = [(name, obj) for name in OUTDOORS[:4] for obj in world.maps[name]['object_events']
                if obj['trainer_type'] == 'TRAINER_TYPE_NORMAL']
    require(len(trainers) == 4, 'Rainfall needs four ordinary trainers')
    direction = {'DOWN': (0, 1), 'UP': (0, -1), 'LEFT': (-1, 0), 'RIGHT': (1, 0)}
    for name, obj in trainers:
        source = world.scripts[name]
        event = re.search(r'^' + obj['script'] + r'::?\n(.*?)(?=^[A-Za-z_]\w*:|\Z)', source, re.M | re.S)
        require(event, f'Missing trainer script: {obj["script"]}')
        trainer = re.search(r'trainerbattle_single\s+(TRAINER_\w+)', event[1])
        require(trainer and trainer[1] in teams, f'{name}: trainer lacks an ordinary party')
        levels = [int(v) for v in re.findall(r'^Level: (\d+)', teams[trainer[1]], re.M)]
        require(1 <= len(levels) <= 3 and all(16 <= v <= 20 for v in levels), f'{trainer[1]}: inappropriate team levels')
        dx, dy = direction[obj['movement_type'].rsplit('_', 1)[1]]
        ray = [(name, obj['x'] + dx * i, obj['y'] + dy * i)
               for i in range(1, int(obj['trainer_sight_or_berry_tree_id']) + 1)]
        require(ray and ray[0] in reached, f'{name}: trainer faces blocked or unreachable terrain')
        arrivals = {a for link in world.maps[name]['connections'] for a, b in world.connection_tiles(name, link)
                    if a in reached and a in graph.get(b, ())}
        require(not arrivals.intersection(ray), f'{name}: trainer starts a battle at a map crossing')
    for identity in ('VIKTOR', 'VIKTORIE'):
        for choice, starter in (('GRASS', 'ARBOK'), ('WATER', 'MIGHTYENA'), ('FIRE', 'XATU')):
            key = f'TRAINER_VIRELIA_{identity}_RAINFALL_{choice}'
            require(key in teams and f'SPECIES_{starter}_VIRELIA' in teams[key], f'{key}: wrong evolved counter-starter')
            levels = [int(v) for v in re.findall(r'^Level: (\d+)', teams[key], re.M)]
            require(len(levels) == 3 and all(17 <= v <= 20 for v in levels), f'{key}: incorrect rival progression')
    print('PASS: four modest-sight trainers, all six gender/starter rival parties and four native wild tables')


def validate():
    validate_localized_conditionals()
    groups = load('data/maps/map_groups.json')
    layouts = load('data/layouts/layouts.json')['layouts']
    sections = load('src/data/region_map/region_map_sections.json')['map_sections']
    preserve_baseline(groups, layouts, sections)
    require(len(layouts) >= 810 and layouts[809]['id'] == 'LAYOUT_VIRELIA_SEABREAK_APPROACHES'
            and len(sections) >= 214, 'Chapter 2 layouts/sections must retain their registered order')
    contract = load('tools/virelia/rainfall_contract.json')
    world = World(layouts, contract)
    validate_used_palettes(world)
    validate_spruce_art()
    validate_gravel_edges(world)
    validate_tree_footprints(world)
    validate_marsh(world)
    validate_cached_entrance(world)
    validate_actor_elevations(world)
    all_flags = frozenset(g['flag'] for g in contract['dynamic_tiles'])
    graph = world.graph(all_flags)
    start, end = tuple(contract['entry']), tuple(contract['exit'])
    reached = distances(graph, start)
    validate_route_connections(world, graph, reached)
    # The blind at the flood gauge and the unfinished plank walk must each be
    # accessible on foot and have just one way back to the trail.
    for x, y in ((32, 17), (25, 14)):
        tip = 'Virelia_RainfallLower', x, y
        require(tip in reached and len(graph[tip]) == 1, f'Marsh side trail is missing its dead end at {tip}')
    require(end in reached, 'Chapter cannot be completed without an HM')
    require(set(NAMES) <= {n[0] for n in reached}, 'A chapter map is disconnected')
    for name, entry in world.maps.items():
        layout = world.layouts[name]
        require(entry.get('region') == 'REGION_HOENN' and layout['layout_version'] == 'emerald', f'{name}: incorrect engine region')
        require(len(world.blocks[name]) == layout['width'] * layout['height'], f'{name}: incorrect layout length')
        require((layout['width'] + 15) * (layout['height'] + 14) <= 10240, f'{name}: native map buffer overflow')
        require(len((ROOT / layout['border_filepath']).read_bytes()) == 8, f'{name}: invalid border dimensions')
        if name in ('Virelia_Seabreak_Lighthouse', 'Virelia_Seabreak_LanternRoom'):
            # Building metatile zero shows the palette backdrop around the stepped walls in game.
            require(all(block & 1023 for block in world.blocks[name] + words(layout['border_filepath'])),
                    f'{name}: the room and its repeating border need opaque void tiles')
        if name != CITY:
            require(start in distances(graph, next(n for n in reached if n[0] == name)), f'{name}: backtracking blocked')
        for obj in entry['object_events']:
            require('local_id' not in obj or re.fullmatch(r'[A-Za-z_][A-Za-z0-9_]*', obj['local_id']),
                    f'{name}: local_id must be a C identifier or omitted for an implicit index')
            require(approaches(world, name, obj, all_flags) & reached.keys(), f'{name}: unreachable NPC/item {obj["script"]}')
            if obj['trainer_type'] == 'TRAINER_TYPE_NORMAL':
                require(int(obj['trainer_sight_or_berry_tree_id']) <= 3, f'{name}: trainer sight spans too far')
                require('FACE_' in obj['movement_type'], f'{name}: trainer requires stable sight direction')
        for event in entry['bg_events']:
            require(approaches(world, name, event, all_flags) & reached.keys(), f'{name}: unreachable clue/sign {event["script"]}')
        for warp in entry['warp_events']:
            if warp['dest_map'] not in world.ids:
                continue
            target_name = world.ids[warp['dest_map']]
            target_warps = world.maps[target_name]['warp_events']
            index = int(warp['dest_warp_id'])
            require(0 <= index < len(target_warps), f'{name}: invalid destination warp index')
            require(target_warps[index]['dest_map'] == entry['id'], f'{name}: warp is not reciprocal')
            require(world.warp_access(name, warp, all_flags) in reached, f'{name}: unreachable warp')
            direction = {'MB_NORTH_ARROW_WARP': (0, -1), 'MB_SOUTH_ARROW_WARP': (0, 1),
                         'MB_EAST_ARROW_WARP': (1, 0), 'MB_WEST_ARROW_WARP': (-1, 0),
                         'MB_DEEP_SOUTH_WARP': (0, 1), 'MB_ANIMATED_DOOR': (0, -1)}.get(
                             world.behavior((name, warp['x'], warp['y']), all_flags))
            if direction:
                dx, dy = direction
                approach = name, warp['x'] - dx, warp['y'] - dy
                require(approach in reached, f'{name}: warp has no reachable approach in its native trigger direction')
            require(world.behavior((name, warp['x'], warp['y']), all_flags) in {
                'MB_ANIMATED_DOOR', 'MB_NON_ANIMATED_DOOR', 'MB_LADDER', 'MB_NORTH_ARROW_WARP',
                'MB_SOUTH_ARROW_WARP', 'MB_EAST_ARROW_WARP', 'MB_WEST_ARROW_WARP', 'MB_DEEP_SOUTH_WARP'},
                f'{name}: warp has no native trigger behavior')
    for gate in contract['dynamic_tiles']:
        node = gate['map'], gate['x'], gate['y']
        require(gate['flag'] in world.scripts[gate['map']], f'{node}: map callback lacks persistent gate flag')
        require('setmetatile' in world.scripts[gate['map']], f'{node}: gate has no restoration callback')
    shortcut = contract['shortcut']
    return_start = tuple(shortcut['lower_approach'])
    return_end = tuple(shortcut['foothills_approach'])
    closed_graph = world.graph(all_flags - {shortcut['flag']})
    closed = distances(closed_graph, return_start)
    require(return_end in closed, 'Closing the shortcut must leave the main route available')
    require(return_start in distances(closed_graph, start), 'The winch must be accessible before it is lowered')
    closed_distance = closed[return_end]
    open_distance = distances(graph, return_start)[return_end]
    require(open_distance + 15 < closed_distance, 'Exploring the lower branch does not unlock a useful return shortcut')
    before = all_flags - {'FLAG_VIRELIA_FIRST_GYM_COMPLETE'}
    before_badge = distances(world.graph(before), (CITY, 12, 3))
    require(not any(n[0] in OUTDOORS for n in before_badge), 'Can leave Slateford before the first badge')
    before_rival = all_flags - {'FLAG_VIRELIA_RAINFALL_RIVAL_COMPLETE'}
    before_fork = distances(world.graph(before_rival), start)
    require(not any(n[0] in OUTDOORS[1:] for n in before_fork), 'Can reach a branch before defeating the rival')
    branch_lengths = {}
    town = 'Virelia_SeabreakTown'
    local_graph = world.graph(all_flags, set(NAMES) - {town, COAST_APPROACHES})
    validate_seabreak_coast(world, {n for n in distances(local_graph, end) if n[0] == town})
    branch_entries = {}
    for branch, source_map, excluded in (('upper', OUTDOORS[1], OUTDOORS[2]), ('lower', OUTDOORS[3], OUTDOORS[1])):
        x, y, elevation = contract['arrivals'][branch]
        landing = town, x, y
        require(world.tile(landing) >> 12 == elevation, f'{branch}: former entrance changed terrace')
        link = next(c for c in world.maps[source_map]['connections'] if world.ids[c['map']] == COAST_APPROACHES)
        seen = distances(world.graph(all_flags - {shortcut['flag']}, {excluded, COAST_APPROACHES, town}), start)
        terrace = {n: [b for b in exits if world.tile(b) >> 12 in (0, elevation)] for n, exits in local_graph.items()
                   if world.tile(n) >> 12 in (0, elevation)}
        district = distances(terrace, landing)
        arrivals = {b: seen[a] + 1 for a, b in world.connection_tiles(source_map, link)
                    if a in seen and b in graph[a] and b in district and world.tile(b) >> 12 == elevation}
        require(arrivals, f'{branch}: route fails to enter its terrace on foot without using the other branch')
        local = distances(local_graph, landing)
        for w in world.maps[town]['warp_events']:
            require(world.warp_access(town, w, all_flags) in local, f'{branch}: town stairways fail to connect districts')
        require(end in local, f'{branch}: arrival cannot reach town services')
        branch_entries[branch] = set(arrivals)
        branch_lengths[branch] = min(arrivals.values())
    shoulder = world.graph(all_flags, set(NAMES) - {COAST_APPROACHES})
    for high in branch_entries['upper']:
        require(not branch_entries['lower'].intersection(distances(shoulder, high)),
                'The ridge and estuary trails must stay separate until the town stairways')
    require(branch_lengths['lower'] >= branch_lengths['upper'] + 15, f'Lower trail is not meaningfully longer: {branch_lengths}')
    grass = {name: sum(world.behavior(n, all_flags) in {'MB_TALL_GRASS', 'MB_LONG_GRASS'} for n in reached if n[0] == name)
             for name in OUTDOORS[1:3]}
    require(grass[OUTDOORS[2]] > grass[OUTDOORS[1]], 'Lower trail needs more wild exploration than upper')
    require({world.maps[name]['weather'] for name in OUTDOORS[:4]} >= {'WEATHER_RAIN', 'WEATHER_DOWNPOUR', 'WEATHER_SHADE'},
            'Missing rain/heavy-rain/shelter weather contrast')
    center = world.scripts[INTERIORS[0]]
    for token in ('setrespawn HEAL_LOCATION_VIRELIA_SEABREAK_POKEMON_CENTER', 'special HealPlayerParty',
                  'FLDEFF_POKECENTER_HEAL', 'pokemart', 'ITEM_SUPER_POTION', 'ITEM_REPEL'):
        require(token in center, f'Seabreak service missing {token}')
    from validate_virelia_first_gym import validate_nurse_path
    validate_nurse_path(center, 'Virelia_Seabreak_PokemonCenter_EventScript_Nurse')
    heal = next(h for h in load('src/data/heal_locations.json')['heal_locations']
                if h['id'] == 'HEAL_LOCATION_VIRELIA_SEABREAK_POKEMON_CENTER')
    require(heal['respawn_map'] == world.maps[INTERIORS[0]]['id'], 'Seabreak whiteout uses wrong Center')
    center_door = next(w for w in world.maps[town]['warp_events'] if w['dest_map'] == world.maps[INTERIORS[0]]['id'])
    require((heal['x'], heal['y']) == (center_door['x'], center_door['y'] + 1) == tuple(contract['center_anchor']),
            'Teleport must arrive on the market terrace outside the Center')
    require((INTERIORS[0], heal['respawn_x'], heal['respawn_y']) in reached, 'Whiteout arrival is blocked')
    require(any(world.behavior(n, all_flags) == 'MB_PC' for n in ((INTERIORS[0], x, y)
                for y in range(world.layouts[INTERIORS[0]]['height']) for x in range(world.layouts[INTERIORS[0]]['width']))),
            'Ordinary Pokemon Storage PC missing')
    constants = text('include/constants/virelia_rainfall.h')
    flags = [int(v, 0) for v in re.findall(r'^#define FLAG_VIRELIA_\w+\s+(0x[0-9A-Fa-f]+)', constants, re.M)]
    require(len(flags) == len(set(flags)) and all(0x264 <= f <= 0x27F for f in flags), 'Chapter flag allocation collision')
    require('0x409B' in constants and '0x400F' in constants, 'Unexpected saved/temporary puzzle variable IDs')
    native = text('src/virelia_rainfall.c')
    require('HasTrainerBeenFought(trainer)' in native, 'Rival completion is not gated by an actual win')
    require('!AddBagItem(item, 1)' in native and native.index('!AddBagItem(item, 1)') < native.index('FlagSet(flag)'),
            'One-time rewards are committed before Bag insertion')
    require('VireliaRainfall_ResetSignal' in world.scripts[INTERIORS[2]] and 'FLAG_VIRELIA_LIGHTHOUSE_REPAIRED' in world.scripts[INTERIORS[2]],
            'Lantern-room callback does not reset partial input and restore completion')
    require(load('tools/virelia_prologue_localization.json')['level_bound'] >= 24, 'Czech coverage must extend through level 24')
    validate_rainfall_cascade(world, reached, all_flags)
    validate_encounters_and_trainers(world, graph, reached, all_flags)
    from virelia.rainfall_connection_views import validate_connection_views
    validate_connection_views(world, reached)
    print(f'PASS: {len(reached)} reachable tiles, reciprocal warps, NPC/clue access, native services and no-HM paths')
    print(f'PASS: distinct branch distances {branch_lengths}; accessible grass {grass}; badge/rival gates block correctly')
    print('PASS: weather contrast, persistent callback contracts, dedicated saved flags and item retry safeguards')
    print('Native event/FLASH tests, Czech font-width validation and emulator/Android checks remain separate evidence.')


if __name__ == '__main__':
    try:
        validate()
    except (ValueError, KeyError, OSError, IndexError, StopIteration) as error:
        print(f'FAIL: {error}', file=sys.stderr)
        sys.exit(1)
