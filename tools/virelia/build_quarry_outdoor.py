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
except ImportError:
    from localize_type_icons import read_png, write_png

ROOT = Path(__file__).resolve().parents[2]
BANK = ROOT / 'data/tilesets/secondary/virelia_quarry_outdoor'
ART = ROOT / 'art/slateford_quarry_outdoor_v1'
PREFIX = 'Virelia_Slateford_Quarry'
YARD, BASIN, HAUL, TERRACES = (PREFIX + s for s in ('Entrance', 'Depths', 'Gallery', 'Terraces'))
NAMES = (YARD, TERRACES, BASIN, HAUL)
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


def pack():
    BANK.mkdir(parents=True, exist_ok=True)
    (BANK / 'palettes').mkdir(exist_ok=True)
    for path in (ROOT / 'data/tilesets/secondary/virelia_quarry/palettes').glob('*.pal'):
        shutil.copyfile(path, BANK / 'palettes' / path.name)
    # Native outdoor cliff graphics keep their indices; only their warm stone
    # ramp changes. Primary grass, water and sign palettes remain untouched.
    stone = [(int(c) // 8) * 8 for line in (ROOT / 'data/tilesets/primary/general/palettes/03.pal').read_text().splitlines()[3:19] for c in line.split()]
    for index, color in enumerate(((208,216,200),(176,184,176),(152,160,160),(120,128,136),(96,104,112),(72,72,80)), 9):
        stone[index*3:index*3+3] = color
    (BANK / 'palettes/06.pal').write_text('JASC-PAL\n0100\n16\n' + ''.join(' '.join(map(str, stone[i:i+3]))+'\n' for i in range(0,48,3)), newline='\n')
    pixels, _, chunks = read_png(ART / 'exports/quarry-outdoor.png')
    canvas = [[0] * 128 for _ in range(256)]
    tiles, meta, attrs, ids, assets = {}, [], [], {}, {}

    def graphic(data):
        data = tuple(data)
        if data not in tiles:
            index = len(tiles)
            if index >= 496:
                raise ValueError('Outdoor quarry graphics overlap native door animation RAM')
            tiles[data] = index + 512
            for y in range(8):
                canvas[index//16*8+y][index%16*8:index%16*8+8] = data[y*8:y*8+8]
        return tiles[data]

    def quarters(image):
        return [0x7000 | graphic(image[q//2*8+y][q%2*8+x] for y in range(8) for x in range(8)) for q in range(4)]

    def add(name, values, attr=0x1000):
        index = 512 + len(attrs)
        ids[name] = index
        meta.extend(values)
        attrs.append(attr)
        return index

    ground = [[5] * 16 for _ in range(16)]
    ground[4][3:5] = [4,4]
    ground[12][11:13] = [4,4]
    floor = quarters(ground)
    add('GROUND', floor + [0]*4)
    rough = [row[:] for row in ground]
    for x,y in ((2,3),(9,2),(6,9),(12,12),(2,13)):
        rough[y][x:x+2] = [3,4]
        rough[y+1][x] = 4
    add('ROUGH', quarters(rough)+[0]*4, 0x1008)  # Native land encounters, including outdoors.
    road = [[4] * 16 for _ in range(16)]
    road[5][2:4] = [5,5]
    road[11][10:12] = [3,4]
    add('ROAD', quarters(road)+[0]*4)
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
    # The gate is a visible cart, with ordinary floor restored after power.
    ids['GATE'] = assets['loaded_cart'][1][0]
    bank_chunks = [(k, struct.pack('>IIBBBBB',128,256,4,3,0,0,0) if k == b'IHDR' else v)
                   for k,v in chunks if k != b'tRNS']
    write_png(BANK / 'tiles.png', canvas, 4, bank_chunks)
    write_words(BANK / 'metatiles.bin', meta)
    write_words(BANK / 'metatile_attributes.bin', attrs)
    save('data/tilesets/secondary/virelia_quarry_outdoor/packing.json', {'ids':ids,'assets':assets,'graphics':len(tiles)})
    return ids, assets


def build(ids, assets):
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

    def cliff(name,x,y,w,height=3):
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
    def stamp(name,asset,x,y,e=3,blocked=True):
        grid = assets[asset]
        for row,line in enumerate(grid):
            for col,index in enumerate(line):
                canopy = asset == 'timber_derrick' and row < 2 or asset == 'quarry_lantern' and row < 2
                rect(name,x+col,y+row,1,1,index,e,blocked and not canopy)
        placements.append({'map':name,'asset':asset,'x':x,'y':y,'elevation':e})

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
    stamp(YARD,'cut_slate_stack',23,17)
    stamp(YARD,'loaded_cart',17,22)
    stamp(YARD,'quarry_lantern',5,24)
    stamp(YARD,'cut_slate_stack',17,16)
    stamp(YARD,'cut_slate_stack',28,21)
    stamp(YARD,'slate_rubble',4,18)
    stamp(YARD,'slate_rubble',29,16)
    stamp(YARD,'slate_rubble',20,5,5)
    stamp(YARD,'framed_adit',19,10)
    stamp(YARD,'quarry_lantern',18,13)
    sign(YARD,20,12,'Virelia_Slateford_QuarryEntrance_EventScript_Equipment')
    sign(YARD,23,18,'Virelia_Slateford_QuarryEntrance_EventScript_Equipment')
    for x in range(15,25): stamp(YARD,'horizontal',x,24,blocked=False)
    stamp(YARD,'buffer_horizontal',25,24)
    stamp(YARD,'buffer_vertical',18,26)
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
    for x in range(14,36,2): stamp(TERRACES,'exposed_slate',x,25)
    actor(TERRACES,'Bruno',17,18,5)['movement_type'] = 'MOVEMENT_TYPE_FACE_DOWN'
    actor(TERRACES,'Item',30,5,7, 'LOCALID_VIRELIA_QUARRY_ITEM')
    stamp(TERRACES,'timber_derrick',28,16,5)
    stamp(TERRACES,'loaded_cart',20,20,5)
    stamp(TERRACES,'slate_rubble',34,5,7)
    stamp(TERRACES,'cut_slate_stack',15,13,5)
    stamp(TERRACES,'cut_slate_stack',34,20,5)
    stamp(TERRACES,'slate_rubble',21,4,7)
    stamp(TERRACES,'slate_rubble',24,18,5)
    for x in range(18,28): stamp(TERRACES,'horizontal',x,22,5,False)
    stamp(TERRACES,'bend_nw',28,22,5,False)
    for y in range(20,22): stamp(TERRACES,'vertical',28,y,5,False)
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
    rect(BASIN,27,7,9,14,0x0D1,solid=True)
    for x in range(27,36):
        rect(BASIN,x,7,1,1,0x0C9,solid=True)
        rect(BASIN,x,20,1,1,0x0D9,solid=True)
    for y in range(8,20):
        rect(BASIN,27,y,1,1,0x0D0,solid=True)
        rect(BASIN,35,y,1,1,0x0D2,solid=True)
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
    stamp(HAUL,'framed_adit',20,32)
    stamp(HAUL,'slate_rubble',21,34)
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
    sign(HAUL,21,35,'Virelia_Slateford_QuarryGallery_EventScript_DeepAdit')
    rect(HAUL,25,6,1,1,0x003,solid=True)
    sign(HAUL,25,6,'Virelia_Slateford_QuarryGallery_EventScript_Sign')
    for x in range(5,23): stamp(HAUL,'horizontal',x,25,blocked=False)
    stamp(HAUL,'bend_ne',4,25,blocked=False)
    stamp(HAUL,'bend_se',4,22,blocked=False)
    stamp(HAUL,'bend_sw',22,22,blocked=False)
    for x in range(5,22): stamp(HAUL,'horizontal',x,22,blocked=False)
    for y in (23,24):
        stamp(HAUL,'vertical',4,y,blocked=False)
        stamp(HAUL,'vertical',22,y,blocked=False)
    for name, cells in GATES.items():
        for x,y in cells: rect(name,x,y,1,1,ids['GATE'],solid=True)

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
        write_words(ROOT / layout['border_filepath'], [0x3400 | ids['NATIVE_079']]*4)
    save('data/layouts/layouts.json', doc)
    save('data/maps/map_groups.json', groups)
    save('tools/virelia/quarry_outdoor_contract.json', {'maps':list(NAMES),'sizes':SIZES,'gates':GATES,'controls':CONTROLS,'placements':placements,'ids':ids,'assets':assets})
    header = ['// Generated by tools/virelia/build_quarry_outdoor.py.', '#ifndef GUARD_CONSTANTS_VIRELIA_QUARRY_OUTDOOR_H', '#define GUARD_CONSTANTS_VIRELIA_QUARRY_OUTDOOR_H', '']
    for key,value in ids.items(): header.append(f'#define VQ_{key} 0x{value:03X}')
    header += ['', '#define LOCALID_VIRELIA_QUARRY_ITEM 2', '', '#endif', '']
    (ROOT / 'include/constants/virelia_quarry_outdoor.h').write_text('\n'.join(header), newline='\n')
    print(f'PASS: authored four outdoor quarry sections, {sum(w*h for w,h in SIZES.values())} cells')


if __name__ == '__main__':
    build(*pack())
