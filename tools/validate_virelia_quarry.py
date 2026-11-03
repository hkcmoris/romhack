#!/usr/bin/env python3
"""Validate the outdoor quarry's native packing, canopy layers and gated routes."""
import argparse
from pathlib import Path
import json
import struct
from virelia.localize_type_icons import read_png
from virelia.build_quarry_outdoor import BANK, ROOT, ART, YARD, TERRACES, BASIN, HAUL, GATES, CONTROLS
from virelia.quarry_outdoor_graph import QuarryGraph, require
from virelia.quarry_outdoor_water import SUMP
from validate_virelia_missing_tools import validate as validate_routes


def words(path):
    raw=path.read_bytes()
    return struct.unpack(f'<{len(raw)//2}H',raw)


def validate_sump_banks(world, metatile, graphic):
    """Sample the shipped shoreline overlay where each tile meets its neighbors."""
    width=world.layouts[BASIN]['width']
    water=set()
    for i,cell in enumerate(world.blocks[BASIN]):
        index=cell & 1023
        if index >= 512 and all(word>>12==9 for word in metatile(index)[:4]):
            water.add((i%width,i//width))
    require(water==SUMP,'Sump water backing does not match its cut footprint')
    for x,y in water:
        layers=metatile(world.cell((BASIN,x,y)) & 1023)
        # Each side midpoint touches one adjacent cell. A corner touches
        # three: even a single dry diagonal needs an inside bank return.
        for px,py,dx,dy in ((8,0,0,-1),(15,8,1,0),(8,15,0,1),(0,8,-1,0),
                            (0,0,-1,-1),(15,0,1,-1),(0,15,-1,1),(15,15,1,1)):
            bank_expected=any(point not in water for point in
                              ((x+dx,y),(x,y+dy),(x+dx,y+dy)))
            word=layers[4 + 2*(py//8) + px//8]
            painted=bool(word and graphic(word)[8*(py%8) + px%8])
            require(painted==bank_expected,
                    f'Sump bank at {(x,y)} faces the wrong way at pixel {(px,py)}')


def validate(run_self_test=False):
    packing=json.loads((BANK/'packing.json').read_text())
    meta,attrs=words(BANK/'metatiles.bin'),words(BANK/'metatile_attributes.bin')
    pixels,depth,_=read_png(BANK/'tiles.png')
    require(depth==4 and len(pixels)==256 and len(pixels[0])==128,'Invalid native bank PNG capacity/header')
    require(len(meta)==len(attrs)*8<=512*8,'Invalid native metatile table size')
    require(packing['graphics']<=496,'Art overlaps native door animation graphics')
    require(all(v>>12<13 and v&1023<1008 for v in meta),'Art uses reserved tile/palette memory')
    primary=read_png(ROOT/'data/tilesets/primary/general/tiles.png')[0]

    def graphic(value):
        t=value&1023
        image=primary if t<512 else pixels
        t%=512
        return tuple(image[t//16*8+(7-y if value&2048 else y)][t%16*8+(7-x if value&1024 else x)] for y in range(8) for x in range(8))

    def metatile(index):
        return meta[(index-512)*8:(index-511)*8]

    floor=metatile(packing['ids']['GROUND'])[:4]
    require(all(all(graphic(v)) for v in floor),'Ground underlay contains transparent holes')
    atlas=read_png(ART/'exports/quarry-outdoor.png')[0]
    manifest=json.loads((ART/'manifest.json').read_text())
    for asset in manifest['assets']:
        x,y,w,h=asset['rect']
        versions=('off','on') if asset['id'] in CONTROLS else ('base',)
        for version in versions:
            key=asset['id']+('_'+version if version!='base' else '')
            grid=packing['assets'][key]
            require((len(grid),len(grid[0]))==(h//16,w//16),'Packed asset footprint changed')
            for row,line in enumerate(grid):
                for col,index in enumerate(line):
                    values=metatile(index)
                    require(tuple(values[:4])==tuple(floor),'Prop lost its opaque ground underlay')
                    canopy=(asset['id']=='timber_derrick' and row<2) or (asset['id']=='quarry_lantern' and row<2)
                    require(attrs[index-512]==(0 if canopy else 0x1000),'Prop canopy/foot layer changed')
                    for q,value in enumerate(values[4:]):
                        expected=tuple(atlas[y+row*16+q//2*8+j][x+col*16+q%2*8+i] for j in range(8) for i in range(8))
                        if version=='on':expected=tuple(13 if p==14 else p for p in expected)
                        require(value>>12==7 and graphic(value)==expected,f'{key}: native art differs from Aseprite export')
    require((BANK/'palettes/07.pal').read_bytes()==(ROOT/manifest['palette_source']).read_bytes(),'Shared machinery palette changed')
    world=QuarryGraph()
    validate_sump_banks(world,metatile,graphic)
    # Compare every entrance cell to its source slice, including cliff inlays.
    placements=json.loads((ROOT/'tools/virelia/quarry_outdoor_contract.json').read_text())['placements']
    overlay={item['index']:item['art'] for item in packing['inlays']}
    cave_in=packing['assets']['caved_in_adit']
    warning=next(e for e in world.maps[HAUL]['bg_events'] if e['script'].endswith('_DeepAdit'))
    door=world.cell((HAUL,warning['x'],warning['y'])) & 1023
    require(overlay.get(door,door)==cave_in[-1][len(cave_in[-1])//2],
            'Deep gallery warning must face the collapsed tunnel mouth')
    for prop in placements:
        if prop['asset'] not in ('framed_adit','caved_in_adit'):
            continue
        for dy,row in enumerate(packing['assets'][prop['asset']]):
            for dx,expected in enumerate(row):
                node=(prop['map'],prop['x']+dx,prop['y']+dy)
                cell=world.cell(node)
                require(overlay.get(cell & 1023, cell & 1023)==expected and cell & 0xC00,
                        f"{node}: missing or passable {prop['asset']} tile")
    used={cell & 1023 for blocks in world.blocks.values() for cell in blocks}
    rocks={value for key,value in packing['ids'].items() if key.startswith('ROCK_')}
    roads={value for key,value in packing['ids'].items() if key=='ROAD' or key.startswith(('ROAD_', 'TRACK_'))}
    platforms={value for key,value in packing['ids'].items() if key.startswith(('SLAB_', 'DECK_'))}
    curves={tile for key,grid in packing['assets'].items() if key.startswith('bend_')
            for row in grid for tile in row}
    inlaid_art,road_overlays=set(),{}
    for item in packing['inlays']:
        index,art,terrain=item['index'],item['art'],item['terrain']
        require(index in used and (terrain in rocks | roads | platforms or
                art in curves and attrs[terrain-512]==0x1000),'Missing embedded art or terrain backing')
        require(metatile(index)==metatile(terrain)[:4]+metatile(art)[4:]
                and attrs[index-512]==attrs[art-512], 'Incorrect terrain-backed prop layers')
        if terrain in rocks:
            inlaid_art.add(art)
        elif terrain in roads:
            road_overlays[index]=art
    for asset in ('exposed_slate','framed_adit'):
        require(all(i in inlaid_art for row in packing['assets'][asset] for i in row),
                f'{asset}: missing stone-backed atlas slices')
    # The old siding crosses the north/south apron, and the yard cart's
    # wheels cover two road cells. Both must display road through the ironwork.
    for node,art in (((HAUL,12,22),packing['assets']['horizontal'][0][0]),
                     ((YARD,18,24),packing['assets']['loaded_cart'][1][0])):
        require(road_overlays.get(world.cell(node)&1023)==art, 'Haul road breaks beneath track or wagon')
    for name,l in world.layouts.items():
        require((l['width']+15)*(l['height']+14)<=10240,'Native map buffer overflow')
        for cell in world.blocks[name]:
            index=cell&1023
            if index>=512:require(index-512<len(attrs),'Map uses nonexistent metatile')
        for obj in world.maps[name]['object_events']:
            require(not world.cell((name,obj['x'],obj['y']))&0xC00,'Actor placed on blocked terrain')
    for name in (YARD,BASIN):
        reached=world.reached(True)
        require(any(n==name and world.behavior((n,x,y))=='MB_CAVE' for n,x,y in reached),'No reachable land-encounter gravel')
    for asset in ('horizontal','vertical','bend_ne','bend_nw','bend_se','bend_sw'):
        require(all(attrs[tile-512]==0x1000 for row in packing['assets'][asset] for tile in row),
                'Haul tracks must remain encounter-free ordinary ground')
    for name in (YARD,TERRACES,BASIN,HAUL):
        source=(ROOT/f'data/maps/{name}/scripts.inc').read_text(encoding='utf-8')
        require('callnative VireliaQuarry_Transition' in source and 'callnative VireliaQuarry_RefreshMap' in source,'Missing load/entry callbacks')
    validate_routes()
    print(f'PASS: {packing["graphics"]}/496 quarry graphics; exact Aseprite props, opaque ground, canopy layers, powered controls and hardware limits')
    if run_self_test:
        self_test(lambda broken:validate_sump_banks(broken,metatile,graphic))


def self_test(check_sump_banks):
    mutations=0
    # Exercise actual graphs with broken source blocks and connection metadata.
    for kind in ('gate','seam','control','elevation','shore corner'):
        world=QuarryGraph()
        if kind=='gate':
            for x,y in GATES[HAUL][:2]:world.blocks[HAUL][y*36+x]&=~0xC00
        elif kind=='seam':
            world.maps[BASIN]['connections'][1]['offset']=-7
        elif kind=='control':
            x,y=CONTROLS['pump_control']
            world.blocks[BASIN][(y+2)*40+x]|=0x400
        elif kind=='elevation':
            for y in range(23,28):
                for x in (10,11):world.blocks[TERRACES][y*40+x]|=0x3000
        else:
            # Flatten the NW turn of the upper sump into the straight bank
            # immediately to its east. The water and collision still agree.
            x,y=min(SUMP,key=lambda point:(point[1],point[0]))
            world.blocks[BASIN][world.layouts[BASIN]['width']*y+x]=world.cell((BASIN,x+1,y))
        try:
            if kind=='shore corner':
                check_sump_banks(world)
            else:
                validate_routes(world)
        except ValueError:
            mutations+=1
        else:
            raise ValueError(f'Outdoor route validator accepted broken {kind} data')
    print(f'PASS: {mutations} broken gate, seam, control approach, stair elevation and shore corner mutations detected')


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--self-test',action='store_true')
    args=parser.parse_args()
    validate(run_self_test=args.self_test)
