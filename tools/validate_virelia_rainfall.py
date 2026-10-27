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

ROOT = Path(__file__).resolve().parents[1]
BASELINE = '492267dadc0f9b54322a39038c479d717eba958a'
CITY = 'Virelia_SlatefordCity'
OUTDOORS = tuple('Virelia_' + name for name in (
    'RainfallFoothills', 'RainfallUpper', 'RainfallLower', 'RainfallLowlands', 'SeabreakTown'))
INTERIORS = tuple('Virelia_Seabreak_' + name for name in (
    'PokemonCenter', 'Lighthouse', 'LanternRoom', 'KeepersHouse'))
NAMES = (CITY, *OUTDOORS, *INTERIORS)
DELTAS = ((1, 0), (-1, 0), (0, 1), (0, -1))
PROTECTED = {
    'include/constants/species.h': '40580ee3c7b6900859639df465dad05a5bdb7e40d32712ca173a8107a8880b48',
    'data/layouts/Virelia_WhisperwoodSanctuary/map.bin': 'ed93585f671788c530f6d51a74923de0444936d8e445a40ff7faa3b21bb88787',
    'data/layouts/Virelia_Route01/map.bin': '78d06cbbe0af06e478f44c009a48c5e1fac892907d9c418114e72540ab931c9c',
    'data/layouts/Virelia_MaplebrookTown/map.bin': '4b60f469443567330ddf3fefd98b56100be35b8ffd248ad22d235172724f0dcf',
    'data/layouts/Virelia_Slateford_Gym/map.bin': 'ac4dc01fd25aad0565fd245e5b181053a3b66193b39e24672d0a14f37a40395b',
    'data/layouts/Virelia_Slateford_QuarryEntrance/map.bin': 'fdc99a85e99b52284697b85e8c1bbd92e9cc258f519edf2d5acce035f8ccb007',
    'data/layouts/Virelia_Slateford_QuarryDepths/map.bin': '92a1f50e5d1638599a941fba113ceacd540412f32476fddb593072d8b335918f',
    'data/layouts/Virelia_Slateford_QuarryGallery/map.bin': '884dd950eca80b6c4a655a0d7b3aab6dd12d98e85d4fa561f522a4d4c9dc1944',
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
    require(groups['gMapGroup_VireliaOverworld'][4:] == list(OUTDOORS), 'Incorrect Chapter 2 overworld map order')
    require(groups['gMapGroup_VireliaInteriors'][8:] == list(INTERIORS), 'Incorrect Chapter 2 interior map order')
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
    print(f'PASS: IDs, starter species, authored first-chapter geometry and My Boy fix retained from {BASELINE}')


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
        for gate in self.contract['dynamic_tiles']:
            if (name, x, y) == (gate['map'], gate['x'], gate['y']):
                key = 'open_block' if gate['flag'] in flags else 'closed_block'
                return gate[key]
        return self.blocks[name][y * layout['width'] + x]

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
            require(name == CITY or not entry.get('connections'), f'{name}: use conventional warps between independent tilesets')
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
        shortcut = self.contract.get('shortcut')
        if shortcut and shortcut['flag'] in flags:
            for source_key, destination_key in (('lower', 'foothills_destination'), ('foothills', 'lower_destination')):
                name, x, y = shortcut[source_key]
                destination = tuple(shortcut[destination_key])
                if name in excluded or destination[0] in excluded:
                    continue
                for dx, dy in DELTAS:
                    source = name, x + dx, y + dy
                    if source in edges and destination in edges:
                        edges[source].append(destination)
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


def approaches(world, name, event, flags):
    x, y = event['x'], event['y']
    nodes = {(name, x + dx, y + dy) for dx, dy in DELTAS}
    for dx, dy in DELTAS:
        if world.behavior((name, x + dx, y + dy), flags) == 'MB_COUNTER':
            nodes.add((name, x + 2 * dx, y + 2 * dy))
    return nodes




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
            if (x, y) == (12, 2) and badge:
                expected = 1, False
            require(restored[x, y] == expected, f"Cached entrance art/collision mismatch at {(x, y)}, badge={badge}")
        require(restored[12, 1] == (0x0A7, False), "Continue must restore the native passable cave-door warp")
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


def validate_encounters_and_trainers(world, reached, flags):
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
        arrivals = {world.warp_access(name, w, flags) for w in world.maps[name]['warp_events']}
        require(not arrivals.intersection(ray), f'{name}: trainer starts battle on the warp arrival')
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
    require(len(layouts) == 806 and len(sections) == 214, 'Chapter 2 layouts/sections must be appended once')
    contract = load('tools/virelia/rainfall_contract.json')
    world = World(layouts, contract)
    validate_used_palettes(world)
    validate_cached_entrance(world)
    all_flags = frozenset(g['flag'] for g in contract['dynamic_tiles']) | frozenset(contract.get('completed_flags', []))
    graph = world.graph(all_flags)
    start, end = tuple(contract['entry']), tuple(contract['exit'])
    reached = distances(graph, start)
    require(end in reached, 'Chapter cannot be completed without an HM')
    require(set(NAMES) <= {n[0] for n in reached}, 'A chapter map is disconnected')
    for name, entry in world.maps.items():
        layout = world.layouts[name]
        require(entry.get('region') == 'REGION_HOENN' and layout['layout_version'] == 'emerald', f'{name}: incorrect engine region')
        require(len(world.blocks[name]) == layout['width'] * layout['height'], f'{name}: incorrect layout length')
        require((layout['width'] + 15) * (layout['height'] + 14) <= 10240, f'{name}: native map buffer overflow')
        require(len((ROOT / layout['border_filepath']).read_bytes()) == 8, f'{name}: invalid border dimensions')
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
            require(world.behavior((name, warp['x'], warp['y']), all_flags) in {
                'MB_ANIMATED_DOOR', 'MB_NON_ANIMATED_DOOR', 'MB_LADDER', 'MB_NORTH_ARROW_WARP',
                'MB_SOUTH_ARROW_WARP', 'MB_EAST_ARROW_WARP', 'MB_WEST_ARROW_WARP', 'MB_DEEP_SOUTH_WARP'},
                f'{name}: warp has no native trigger behavior')
    for gate in contract['dynamic_tiles']:
        node = gate['map'], gate['x'], gate['y']
        require(gate['flag'] in world.scripts[gate['map']], f'{node}: map callback lacks persistent gate flag')
        require('setmetatile' in world.scripts[gate['map']], f'{node}: gate has no restoration callback')
    shortcut = contract['shortcut']
    for source_key, destination_key in (('lower', 'foothills_destination'), ('foothills', 'lower_destination')):
        source_map = shortcut[source_key][0]
        destination = tuple(shortcut[destination_key])
        script = world.scripts[source_map]
        require(shortcut['flag'] in script, f'{source_map}: shortcut lacks persistent flag check')
        expected = (r'warp\s+' + world.maps[destination[0]]['id'] + r',\s*WARP_ID_NONE,\s*'
                    + str(destination[1]) + r',\s*' + str(destination[2]) + r'\b')
        require(re.search(expected, script), f'{source_map}: scripted shortcut destination disagrees with actual map contract')
        require(destination in reached, f'{source_map}: shortcut arrives on blocked terrain')
    return_start = tuple(shortcut['lower_destination'])
    return_end = tuple(shortcut['foothills_destination'])
    closed_distance = distances(world.graph(all_flags - {shortcut['flag']}), return_start)[return_end]
    open_distance = distances(graph, return_start)[return_end]
    require(open_distance + 15 < closed_distance, 'Exploring the lower branch does not unlock a useful return shortcut')
    before = all_flags - {'FLAG_VIRELIA_FIRST_GYM_COMPLETE'}
    require(end not in distances(world.graph(before), (CITY, 12, 3)), 'First badge prerequisite can be bypassed')
    before_rival = all_flags - {'FLAG_VIRELIA_RAINFALL_RIVAL_COMPLETE'}
    require(end not in distances(world.graph(before_rival), start), 'Rival can be bypassed')
    branch_lengths = {}
    for branch, excluded in (('upper', OUTDOORS[2]), ('lower', OUTDOORS[1])):
        seen = distances(world.graph(all_flags - {'FLAG_VIRELIA_LOWER_SHORTCUT'}, {excluded}), start)
        require(end in seen, f'{branch}: mandatory route needs the other branch or an HM')
        branch_lengths[branch] = seen[end]
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
    validate_encounters_and_trainers(world, reached, all_flags)
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