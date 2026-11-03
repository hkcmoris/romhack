#!/usr/bin/env python3
"""Author the four outdoor quarry sections and pack their shared Aseprite bank.

This explicit authoring command replaces ONLY the four quarry layouts. Do not
run it over subsequent Porymap edits unless intentionally rebuilding the quarry.
"""
from pathlib import Path
import copy
import json
import shutil
import struct

try:
    from .localize_type_icons import read_png, write_png
    from .quarry_outdoor_rim import pack_rim, plant_rim
    from . import quarry_outdoor_surfaces as surfaces, quarry_outdoor_walls as walls, quarry_outdoor_water as water
except ImportError:
    from localize_type_icons import read_png, write_png
    from quarry_outdoor_rim import pack_rim, plant_rim
    import quarry_outdoor_surfaces as surfaces, quarry_outdoor_walls as walls, quarry_outdoor_water as water

ROOT = Path(__file__).resolve().parents[2]
BANK = ROOT / 'data/tilesets/secondary/virelia_quarry_outdoor'
ART = ROOT / 'art/slateford_quarry_outdoor_v1'
PREFIX = 'Virelia_Slateford_Quarry'
YARD, BASIN, HAUL, TERRACES = (PREFIX + s for s in ('Entrance', 'Depths', 'Gallery', 'Terraces'))
NAMES = (YARD, TERRACES, BASIN, HAUL)
ORIGINS = {YARD: (0, 0), TERRACES: (36, 0), BASIN: (36, 40), HAUL: (0, 32)}
SIZES = {YARD: (36, 32), TERRACES: (40, 40), BASIN: (40, 32), HAUL: (36, 40)}
GATES = {HAUL: ((26, 8), (27, 8), (27, 24), (27, 25)), BASIN: ((20, 22), (21, 22))}
CONTROLS = {'pump_control': (22, 12), 'electrical_feed': (10, 12)}
CONNECTIONS = {
    YARD: [('right', 0, TERRACES), ('down', 0, HAUL)],
    TERRACES: [('left', 0, YARD), ('down', 0, BASIN)],
    BASIN: [('up', 0, TERRACES), ('left', -8, HAUL)],
    HAUL: [('up', 0, YARD), ('right', 8, BASIN)],
}


def load(path):
    return json.loads((ROOT / path).read_text(encoding='utf-8'))


def save(path, data):
    path = ROOT / path
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(data, ensure_ascii=False, indent=2) + '\n', encoding='utf-8', newline='\n')


def words(path):
    raw = path.read_bytes()
    return list(struct.unpack(f'<{len(raw)//2}H', raw))


def write_words(path, values):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(struct.pack(f'<{len(values)}H', *values))


def build_quarry():
    BANK.mkdir(parents=True, exist_ok=True)
    (BANK / 'palettes').mkdir(exist_ok=True)
    for path in (ROOT / 'data/tilesets/secondary/virelia_quarry/palettes').glob('*.pal'):
        shutil.copyfile(path, BANK / 'palettes' / path.name)
    # Native outdoor cliff graphics keep their indices; only their warm stone
    # ramp changes. Primary grass, water and sign palettes remain untouched.
    stone = [(int(c) // 8) * 8 for line in (ROOT / 'data/tilesets/primary/general/palettes/03.pal').read_text().splitlines()[3:19] for c in line.split()]
    for index, color in enumerate(((208,216,200),(176,184,176),(152,160,160),(120,128,136),(96,104,112),(72,72,80)), 9):
        stone[index*3:index*3+3] = color
    stone[45:48] = surfaces.PALETTE[5]  # Dust visible in clipped rock corners.
    (BANK / 'palettes/06.pal').write_text('JASC-PAL\n0100\n16\n' + ''.join(' '.join(map(str, stone[i:i+3]))+'\n' for i in range(0,48,3)), newline='\n')
    pixels, _, chunks = read_png(ART / 'exports/quarry-outdoor.png')
    canvas = [[0] * 128 for _ in range(256)]
    tiles, meta, attrs, ids, assets = {}, [], [], {}, {}
    tiles[(0,) * 64] = 0  # General already supplies the transparent graphic.
    graphic_count = 0

    def graphic(data):
        nonlocal graphic_count
        data = tuple(data)
        if data not in tiles:
            index = graphic_count
            if index >= 496:
                raise ValueError('Outdoor quarry graphics overlap native door animation RAM')
            graphic_count += 1
            for y in range(8):
                canvas[index//16*8+y][index%16*8:index%16*8+8] = data[y*8:y*8+8]
            # GBA tile entries can mirror either axis without additional VRAM.
            for flip_y in (False, True):
                for flip_x in (False, True):
                    mirrored = tuple(data[(7-y if flip_y else y)*8 + (7-x if flip_x else x)]
                                     for y in range(8) for x in range(8))
                    tiles.setdefault(mirrored, (index + 512) | (flip_x << 10) | (flip_y << 11))
        return tiles[data]

    def quarters(image):
        return [0x7000 | graphic(image[q//2*8+y][q%2*8+x] for y in range(8) for x in range(8)) for q in range(4)]

    def add(name, values, attr=0x1000):
        index = 512 + len(attrs)
        if index >= 1024:
            raise ValueError('Outdoor quarry exceeds secondary metatile capacity')
        ids[name] = index
        meta.extend(values)
        attrs.append(attr)
        return index

    surfaces.write_surface_palette(BANK)
    floor = surfaces.surface_quarters(surfaces.base_surface('GROUND'), quarters)
    add('GROUND', floor + [0]*4)
    add('ROUGH', surfaces.surface_quarters(surfaces.base_surface('ROUGH'), quarters) + [0]*4, 0x1008)
    add('ROAD', surfaces.surface_quarters(surfaces.base_surface('ROAD'), quarters) + [0]*4)
    add('CITY_EXIT', floor+[0]*4, 0x1065)
    general = words(ROOT / 'data/tilesets/primary/general/metatiles.bin')
    for code in (0x68,0x69,0x6a,0x70,0x71,0x72,0x78,0x79,0x7a,0x80,0x81,0x82,0x88,0x89,0x8a,0x90,0x91,0x92,0xAF,0xCF):
        values = [(v & 0xfff) | (0x6000 if v>>12 == 3 else v & 0xf000) for v in general[code*8:code*8+8]]
        add(f'NATIVE_{code:03X}', values, 0x1000)
    manifest = load('art/slateford_quarry_outdoor_v1/manifest.json')
    for asset in manifest['assets']:
        x,y,w,h = asset['rect']
        versions = ('off','on') if asset['id'] in CONTROLS else ('base',)
        for version in versions:
            grid = []
            for row in range(h//16):
                line = []
                for col in range(w//16):
                    image = [[pixels[y+row*16+j][x+col*16+i] for i in range(16)] for j in range(16)]
                    if version == 'on':
                        image = [[13 if p == 14 else p for p in line] for line in image]
                    canopy = (asset['id'] == 'timber_derrick' and row < 2) or (asset['id'] == 'quarry_lantern' and row < 2)
                    line.append(add(f'{asset["id"].upper()}_{version.upper()}_{col}_{row}', floor+quarters(image), 0 if canopy else 0x1000))
                grid.append(line)
            assets[asset['id'] + ('_'+version if version != 'base' else '')] = grid
    # Retractable freight bollards. Each cell needs a complete silhouette:
    # pairs close east-west and north-south approaches using the same callback.
    bollard = (
        '0000000000000000', '0000011111000000', '0001435554310000', '0001444444310000',
        '0001223332210000', '000126aa66210000', '00016aa66a610000', '0001aa66aa610000',
        '0001223332210000', '0001434332310000', '0001332322210000', '0001222222210000',
        '0014333333331000', '0015432222341000', '0001111111110000', '0000000000000000',
    )
    add('GATE', floor + quarters([[int(p, 16) for p in row] for row in bollard]))
    # A lens of pale quartz and oxidized iron follows a joint through two
    # adjoining blocks. Stamp only the mineral; retain the local face below.
    vein = [[0] * 32 for _ in range(16)]
    for x, y, stroke in (
        (21, 3, '999aa5'), (18, 4, '89aaa998'), (14, 5, '358859'),
        (12, 6, '43554'), (11, 7, '934'), (7, 8, '9aa58'),
        (4, 9, '898598'), (2, 10, '938'), (11, 10, '388'),
        (15, 9, '99895'), (20, 8, '9388'), (24, 7, '88'),
    ):
        vein[y][x:x+len(stroke)] = [int(p, 16) for p in stroke]
    for part in range(2):
        add(f'QUARTZ_{part}', floor + quarters([row[16*part:16*part+16] for row in vein]))
    surfaces.pack_surfaces(add, quarters)
    surfaces.pack_work_floors(add, quarters, BANK)
    cut_tile = walls.pack_walls(add, graphic)
    primary_pixels, _, _ = read_png(ROOT / 'data/tilesets/primary/general/tiles.png')
    pack_rim(add, graphic, ids, meta, general, primary_pixels, BANK)
    water.pack_sump(add, graphic, general, BANK)
    bank_chunks = [(k, struct.pack('>IIBBBBB',128,256,4,3,0,0,0) if k == b'IHDR' else v)
                   for k,v in chunks if k != b'tRNS']
    inlays = []
    def embed(art, terrain):
        name = f'INLAY_{art:03X}_{terrain:03X}'
        if name not in ids:
            top = (art - 512) * 8 + 4
            bottom = (terrain - 512) * 8
            if any(word & 1023 for word in meta[bottom+4:bottom+8]):
                raise ValueError(f'Inlay {name} would erase the terrain overlay')
            index = add(name, meta[bottom:bottom+4] + meta[top:top+4], attrs[art-512])
            inlays.append(dict(index=index, art=art, terrain=terrain))
        return ids[name]

    build(ids, assets, embed, cut_tile)
    write_png(BANK / 'tiles.png', canvas, 4, bank_chunks)
    write_words(BANK / 'metatiles.bin', meta)
    write_words(BANK / 'metatile_attributes.bin', attrs)
    save('data/tilesets/secondary/virelia_quarry_outdoor/packing.json',
         {'ids':ids,'assets':assets,'graphics':graphic_count,'inlays':inlays})


def build(ids, assets, embed, cut_tile):
    originals = {n: load(f'data/maps/{n}/map.json') for n in (YARD,BASIN,HAUL)}
    # Authoring data is kept separately so moving Bruno/item out of the yard is
    # idempotent across rebuilds without rereading an obsolete generated map.
    actors_path = ROOT / 'tools/virelia/quarry_outdoor_actors.json'
    if not actors_path.exists():
        save('tools/virelia/quarry_outdoor_actors.json', {n: e['object_events'] for n,e in originals.items()})
    actors = load('tools/virelia/quarry_outdoor_actors.json')
    by_script = {o['script'].rsplit('_',1)[-1]: copy.deepcopy(o) for objects in actors.values() for o in objects}
    by_script['Guide'] = copy.deepcopy(next(o for o in actors[YARD] if o['script'].endswith('_Guide')))
    doc = load('data/layouts/layouts.json')
    groups = load('data/maps/map_groups.json')
    if TERRACES not in groups['gMapGroup_VireliaOverworld']:
        groups['gMapGroup_VireliaOverworld'].append(TERRACES)
    maps = {}
    grids = {}

    for name in NAMES:
        w,h = SIZES[name]
        grids[name] = [[0x3400 | ids['NATIVE_079']] * w for _ in range(h)]
        entry = copy.deepcopy(originals.get(name, originals[YARD]))
        entry.update(id='MAP_VIRELIA_SLATEFORD_QUARRY_' + name.removeprefix(PREFIX).upper(), name=name,
                     layout='LAYOUT_VIRELIA_SLATEFORD_QUARRY_' + name.removeprefix(PREFIX).upper(),
                     map_type='MAP_TYPE_ROUTE', weather='WEATHER_SUNNY', allow_escaping=False,
                     show_map_name=False, connections=[{'direction':d,'offset':o,'map':'MAP_VIRELIA_SLATEFORD_QUARRY_'+n.removeprefix(PREFIX).upper()} for d,o,n in CONNECTIONS[name]],
                     object_events=[], warp_events=[], coord_events=[], bg_events=[])
        maps[name] = entry

    def rect(name,x,y,w,h,tile=None,e=3,solid=False):
        for yy in range(y,y+h):
            for xx in range(x,x+w):
                grids[name][yy][xx] = e<<12 | (0x400 if solid else 0) | (ids['GROUND'] if tile is None else tile)

    cut_faces = []
    def cliff(name,x,y,w,height=3):
        cut_faces.append((name, x, y, w, height))
        rect(name,x,y,w,1,ids['NATIVE_071'],solid=True)
        rect(name,x,y+1,w,height-1,ids['NATIVE_079'],solid=True)
        for yy in range(y,y+height):
            grids[name][yy][x] = 0x3400 | ids['NATIVE_070' if yy == y else 'NATIVE_078']
            grids[name][yy][x+w-1] = 0x3400 | ids['NATIVE_072' if yy == y else 'NATIVE_07A']

    def stairs(name,x,y,height):
        for yy in range(y,y+height):
            for dx,key in ((0,'NATIVE_0AF'),(1,'NATIVE_0CF')):
                grids[name][yy][x+dx] = ids[key]  # Native elevation-zero transition.

    placements = []
    embedded_art = {}
    rock_markers = walls.wall_markers(ids)
    work_floors = {ids[f'{kind}_{half}'] for kind in ('SLAB','DECK') for half in range(2)}
    def work_floor(name, kind, spans, elevation):
        for y, start, end in spans:
            for x in range(start, end + 1):
                rect(name,x,y,1,1,ids[f'{kind}_{x % 2}'],elevation)

    def stamp(name,asset,x,y,e=3,blocked=True):
        grid = assets[asset]
        for row,line in enumerate(grid):
            for col,index in enumerate(line):
                canopy = asset == 'timber_derrick' and row < 2 or asset == 'quarry_lantern' and row < 2
                original = grids[name][y+row][x+col] & 1023
                on_track_bed = asset.startswith('bend_') and original == ids['GROUND']
                on_cliff = asset in ('framed_adit', 'caved_in_adit', 'exposed_slate') and original in rock_markers
                if original in work_floors or original == ids['ROAD'] or on_track_bed or on_cliff:
                    embedded_art[name, x+col, y+row] = (original, index)
                rect(name,x+col,y+row,1,1,index,e,blocked and not canopy)
        placements.append({'map':name,'asset':asset,'x':x,'y':y,'elevation':e})

    def rail_bend(name, direction, x, y, elevation=3):
        # x,y is the intersection of the two straight track centerlines.
        # Each 32px curve starts one cell back along both adjoining tracks.
        left = x - ('w' in direction)
        top = y - ('n' in direction)
        stamp(name, 'bend_' + direction, left, top, elevation, blocked=False)

    def actor(name,key,x,y,e=3,local=None):
        obj = copy.deepcopy(by_script[key])
        obj.update(x=x,y=y,elevation=e)
        if local:
            obj['local_id'] = local
        maps[name]['object_events'].append(obj)
        return obj

    def sign(name,x,y,script):
        maps[name]['bg_events'].append(dict(type='sign',x=x,y=y,elevation=0,player_facing_dir='BG_EVENT_PLAYER_FACING_ANY',script=script))

    # Yard: broad sorting apron, elevated tool ledge, city exit and two seams.
    rect(YARD,2,3,32,27)
    rect(YARD,2,3,32,7,e=5)
    cliff(YARD,2,10,32)
    stairs(YARD,8,9,5)
    rect(YARD,34,14,2,2)
    rect(YARD,26,29,2,3)
    rect(YARD,9,29,1,1,ids['CITY_EXIT'])
    rect(YARD,8,25,3,4,ids['ROAD'])
    rect(YARD,8,24,21,3,ids['ROAD'])
    rect(YARD,27,14,3,12,ids['ROAD'])
    rect(YARD,28,14,8,2,ids['ROAD'])
    rect(YARD,26,27,2,5,ids['ROAD'])
    maps[YARD]['warp_events'] = [dict(x=9,y=29,elevation=0,dest_map='MAP_VIRELIA_SLATEFORD_CITY',dest_warp_id='2')]
    actor(YARD,'Guide',11,26)
    actor(YARD,'Wrench',12,6,5)
    work_floor(YARD,'SLAB',[(16,17,24),(17,16,24),(18,16,24),(19,16,24),
                            (20,18,21),(21,18,19),(22,18,19)],3)
    stamp(YARD,'cut_slate_stack',23,17)
    # The loaded wagon covers two rail cells; the track emerges at its wheels.
    stamp(YARD,'loaded_cart',18,23)
    stamp(YARD,'quarry_lantern',5,24)
    stamp(YARD,'cut_slate_stack',17,16)
    stamp(YARD,'slate_cutting_bench',19,17)
    stamp(YARD,'cut_slate_stack',28,21)
    stamp(YARD,'slate_rubble',4,18)
    stamp(YARD,'slate_rubble',29,16)
    stamp(YARD,'slate_rubble',20,5,5)
    stamp(YARD,'framed_adit',19,10)
    stamp(YARD,'quarry_lantern',18,13)
    sign(YARD,20,12,'Virelia_Slateford_QuarryEntrance_EventScript_Equipment')
    sign(YARD,23,18,'Virelia_Slateford_QuarryEntrance_EventScript_Equipment')
    stamp(YARD,'buffer_vertical',15,21)
    stamp(YARD,'vertical',15,22,blocked=False)
    rail_bend(YARD,'ne',15,24)
    for x in (17,20,21,22,23,24): stamp(YARD,'horizontal',x,24,blocked=False)
    stamp(YARD,'buffer_horizontal',25,24)
    rect(YARD,25,4,6,3,ids['ROUGH'],5)

    # Working benches, with a low western haul road and two native stairways.
    rect(TERRACES,2,3,36,34)
    rect(TERRACES,8,3,30,6,e=7)
    rect(TERRACES,8,12,30,12,e=5)
    cliff(TERRACES,8,9,30)
    cliff(TERRACES,8,24,30)
    for y in range(3,27): rect(TERRACES,7,y,1,1,ids['NATIVE_070'],solid=True)
    stairs(TERRACES,28,8,5)
    stairs(TERRACES,10,23,5)
    rect(TERRACES,0,14,2,2)
    rect(TERRACES,10,37,2,3)
    rect(TERRACES,0,14,6,2,ids['ROAD'])
    rect(TERRACES,3,14,3,16,ids['ROAD'])
    rect(TERRACES,4,28,8,3,ids['ROAD'])
    rect(TERRACES,10,29,2,11,ids['ROAD'])
    rect(TERRACES,10,14,20,3,ids['ROAD'],5)
    rect(TERRACES,10,17,3,6,ids['ROAD'],5)
    # The eastern wall steps down into the lower yard in three cuts. Leave
    # a broad apron around the stair foot and the two-cell southern exit.
    for x, top, lip, width in ((23,32,33,4), (27,30,32,5), (32,28,31,6)):
        rect(TERRACES,x,top,width,lip-top,ids['NATIVE_079'],solid=True)
        cliff(TERRACES,x,lip,width)
    # Cut courses below the working floor, interrupted by rough native face.
    for x in (14,16,18,24,26,28,30,32): stamp(TERRACES,'exposed_slate',x,25)
    actor(TERRACES,'Bruno',17,18,5)['movement_type'] = 'MOVEMENT_TYPE_FACE_DOWN'
    actor(TERRACES,'Item',30,5,7, 'LOCALID_VIRELIA_QUARRY_ITEM')
    work_floor(TERRACES,'DECK',[(18,27,31),(19,26,32),(20,26,32),(21,27,29)],5)
    stamp(TERRACES,'timber_derrick',28,16,5)
    stamp(TERRACES,'loaded_cart',20,21,5)
    stamp(TERRACES,'slate_rubble',34,5,7)
    stamp(TERRACES,'cut_slate_stack',15,13,5)
    stamp(TERRACES,'cut_slate_stack',34,20,5)
    stamp(TERRACES,'slate_rubble',21,4,7)
    stamp(TERRACES,'slate_rubble',24,18,5)
    stamp(TERRACES,'buffer_vertical',15,19,5)
    stamp(TERRACES,'vertical',15,20,5,False)
    rail_bend(TERRACES,'ne',15,22,5)
    for x in (17,18,19,22,23,24,25,26): stamp(TERRACES,'horizontal',x,22,5,False)
    rail_bend(TERRACES,'nw',28,22,5)
    stamp(TERRACES,'vertical',28,20,5,False)
    stamp(TERRACES,'quarry_lantern',14,29)
    sign(TERRACES,29,19,'Virelia_Slateford_QuarryEntrance_EventScript_Equipment')

    # Basin: both controls in one map, open approaches, enclosed freight plot.
    rect(BASIN,2,3,36,27)
    rect(BASIN,10,0,2,3)
    rect(BASIN,0,16,2,2)
    rect(BASIN,10,0,2,10,ids['ROAD'])
    rect(BASIN,10,8,15,3,ids['ROAD'])
    rect(BASIN,14,10,3,12,ids['ROAD'])
    rect(BASIN,0,16,15,2,ids['ROAD'])
    rect(BASIN,15,20,7,2,ids['ROAD'])
    water.paint_sump(grids[BASIN], ids)
    for asset,(x,y) in CONTROLS.items():
        stamp(BASIN,asset+'_off',x,y)
        sign(BASIN,x,y+1,'Virelia_Slateford_QuarryDepths_EventScript_'+('Pump' if asset == 'pump_control' else 'Feed'))
    rect(BASIN,16,16,1,1,0x003,solid=True)
    sign(BASIN,16,16,'Virelia_Slateford_QuarryDepths_EventScript_Instructions')
    actor(BASIN,'Lenka',15,19)['movement_type'] = 'MOVEMENT_TYPE_FACE_DOWN'
    actor(BASIN,'Evidence',21,26)
    actor(BASIN,'Crate',31,25)
    for x in range(17,27):
        rect(BASIN,x,22,1,1,ids['NATIVE_071'],solid=True)
        rect(BASIN,x,30,1,1,ids['NATIVE_079'],solid=True)
    for y in range(23,30):
        for x in (17,26): rect(BASIN,x,y,1,1,ids['NATIVE_079'],solid=True)
    rect(BASIN,4,23,8,4,ids['ROUGH'])
    stamp(BASIN,'slate_rubble',6,8)
    stamp(BASIN,'cut_slate_stack',3,5)
    stamp(BASIN,'loaded_cart',32,22)
    stamp(BASIN,'slate_rubble',33,27)
    stamp(BASIN,'quarry_lantern',24,25)

    # Old haul terrace: two sealed vestibules, no way around either barrier.
    rect(HAUL,2,10,24,26)
    rect(HAUL,24,10,4,4)
    rect(HAUL,18,34,7,4)
    rect(HAUL,26,0,2,13)
    rect(HAUL,26,24,10,2)
    rect(HAUL,26,0,2,13,ids['ROAD'])
    rect(HAUL,12,12,16,2,ids['ROAD'])
    rect(HAUL,12,13,2,17,ids['ROAD'])
    rect(HAUL,13,24,23,2,ids['ROAD'])
    cliff(HAUL,18,16,7)
    cliff(HAUL,18,30,7)
    stamp(HAUL,'framed_adit',5,9)
    stamp(HAUL,'caved_in_adit',20,32)
    stamp(HAUL,'slate_rubble',22,35)
    stamp(HAUL,'quarry_lantern',17,30)
    stamp(HAUL,'cut_slate_stack',4,30)
    stamp(HAUL,'exposed_slate',20,17)
    stamp(HAUL,'slate_rubble',3,13)
    stamp(HAUL,'slate_rubble',23,28)
    stamp(HAUL,'quarry_lantern',8,11)
    actor(HAUL,'Sableye',14,29)
    actor(HAUL,'Hammer',8,18)
    actor(HAUL,'Pick',19,13)
    actor(HAUL,'Hoard',15,28)
    sign(HAUL,21,34,'Virelia_Slateford_QuarryGallery_EventScript_DeepAdit')
    rect(HAUL,25,6,1,1,0x003,solid=True)
    sign(HAUL,25,6,'Virelia_Slateford_QuarryGallery_EventScript_Sign')
    # The abandoned spur starts at the boarded adit, doubles back along the
    # old stockyard, and finishes against a timber stop below the rubble.
    for y in range(12,21): stamp(HAUL,'vertical',6,y,blocked=False)
    rail_bend(HAUL,'ne',6,22)
    for x in range(8,21): stamp(HAUL,'horizontal',x,22,blocked=False)
    # East end of the siding: the two broad curves meet on the same tangent.
    rail_bend(HAUL,'sw',22,22)
    rail_bend(HAUL,'nw',22,25)
    for x in range(6,21): stamp(HAUL,'horizontal',x,25,blocked=False)
    rail_bend(HAUL,'ne',4,25)
    stamp(HAUL,'buffer_vertical',4,22)
    stamp(HAUL,'vertical',4,23,blocked=False)
    for name, cells in GATES.items():
        for x,y in cells: rect(name,x,y,1,1,ids['GATE'],solid=True)

    # Stepped excavation cuts break the rectangular perimeter of each bench.
    # Each run follows the bedrock inward from an existing wall. Keep stamped
    # infrastructure, haul roads, stairs and encounter patches intact.
    cuts = {
        YARD: [(2,3,5), (2,4,3), (2,5,1), (32,3,2),
               (2,27,2), (2,28,3), (2,29,5), (31,29,3)],
        TERRACES: [(34,3,4), (36,4,2), (37,5,1), (35,34,3), (33,35,5),
                   (31,36,7), (2,34,1), (2,35,2), (2,36,4)],
        BASIN: [(2,3,5), (2,4,3), (36,3,2), (2,27,1), (2,28,2), (2,29,4),
                (36,28,2), (34,29,4)],
        HAUL: [(2,10,3), (2,11,1), (2,32,1), (2,33,2), (2,34,4), (2,35,6),
               (22,37,3), (24,36,1)],
    }
    for name, runs in cuts.items():
        for start, y, length in runs:
            for x in range(start, start + length):
                if grids[name][y][x] & 1023 == ids['GROUND']:
                    grids[name][y][x] = 0x3400 | ids['NATIVE_079']

    # Solve connected contours before compositing overlays. This keeps
    # both sides of a road joined underneath sleepers, carts and buildings.
    exposed = {}
    for (name, x, y), (marker, art) in embedded_art.items():
        cell = grids[name][y][x]
        if cell & 1023 == art:
            exposed[name, x, y] = art
            grids[name][y][x] = (cell & ~1023) | marker
    walls.place_walls(grids, ids, cut_faces, ORIGINS, cut_tile)
    surfaces.dress_surfaces(grids, ids, ORIGINS)
    for name, x, y in ((TERRACES,28,33), (YARD,26,11), (HAUL,21,31)):
        for part in range(2):
            cell = grids[name][y][x + part]
            grids[name][y][x + part] = (cell & ~1023) | embed(ids[f'QUARTZ_{part}'], cell & 1023)
    for (name, x, y), art in exposed.items():
        cell = grids[name][y][x]
        grids[name][y][x] = (cell & ~1023) | embed(art, cell & 1023)

    # Most of the yard is actively quarried. The largest surviving groves
    # occupy the unworked rim above the old haul road and below the terraces.
    plant_rim(grids[YARD], ids, [(2,0,'FIR'), (4,1,'SAPLING'), (7,1,'TALUS'),
                                 (30,30,'SAPLING'), (33,29,'FIR'), (32,30,'TALUS')])
    plant_rim(grids[TERRACES], ids, [(34,0,'FIR'), (37,1,'SAPLING'), (38,3,'TALUS'),
                                     (1,37,'FIR'), (3,38,'SAPLING'), (6,38,'TALUS'),
                                     (35,37,'FIR'), (38,36,'SAPLING'), (31,38,'TALUS')])
    plant_rim(grids[BASIN], ids, [(3,0,'FIR'), (5,1,'SAPLING'), (7,1,'TALUS'),
                                  (1,29,'SAPLING'), (4,30,'TALUS'), (35,30,'FIR')])
    plant_rim(grids[HAUL], ids, [(1,3,'SAPLING'), (2,5,'FIR'), (4,4,'SAPLING'),
                                 (5,5,'FIR'), (8,5,'TALUS'), (17,2,'FIR'),
                                 (19,1,'SAPLING'), (20,3,'TALUS'), (30,12,'FIR'),
                                 (32,11,'SAPLING'), (30,15,'TALUS'), (33,18,'FIR'),
                                 (28,31,'SAPLING'), (29,32,'FIR'), (31,34,'TALUS'),
                                 (3,36,'FIR'), (5,37,'SAPLING'), (8,38,'TALUS')])

    for name,entry in maps.items():
        layout = next((l for l in doc['layouts'] if l['id'] == entry['layout']), None)
        if layout is None:
            layout = dict(id=entry['layout'],name=name+'_Layout',width=SIZES[name][0],height=SIZES[name][1],
                          primary_tileset='gTileset_General',secondary_tileset='gTileset_VireliaQuarryOutdoor',
                          border_filepath=f'data/layouts/{name}/border.bin',blockdata_filepath=f'data/layouts/{name}/map.bin',layout_version='emerald')
            doc['layouts'].append(layout)
        layout.update(width=SIZES[name][0],height=SIZES[name][1],primary_tileset='gTileset_General',secondary_tileset='gTileset_VireliaQuarryOutdoor')
        save(f'data/maps/{name}/map.json', entry)
        write_words(ROOT / layout['blockdata_filepath'], [v for row in grids[name] for v in row])
        write_words(ROOT / layout['border_filepath'], [0x3400 | ids[f'ROCK_BED_{i}'] for i in range(4)])
    save('data/layouts/layouts.json', doc)
    save('data/maps/map_groups.json', groups)
    save('tools/virelia/quarry_outdoor_contract.json', {'maps':list(NAMES),'sizes':SIZES,'gates':GATES,'controls':CONTROLS,'placements':placements,'ids':ids,'assets':assets})
    header = ['// Generated by tools/virelia/build_quarry_outdoor.py.', '#ifndef GUARD_CONSTANTS_VIRELIA_QUARRY_OUTDOOR_H', '#define GUARD_CONSTANTS_VIRELIA_QUARRY_OUTDOOR_H', '']
    for key,value in ids.items(): header.append(f'#define VQ_{key} 0x{value:03X}')
    header += ['', '#define LOCALID_VIRELIA_QUARRY_ITEM 2', '', '#endif', '']
    (ROOT / 'include/constants/virelia_quarry_outdoor.h').write_text('\n'.join(header), newline='\n')
    print(f'PASS: authored four outdoor quarry sections, {sum(w*h for w,h in SIZES.values())} cells')


if __name__ == '__main__':
    build_quarry()
