#!/usr/bin/env python3
"""Validate the outdoor quarry's native packing, canopy layers and gated routes."""
import argparse
from pathlib import Path
import json
import struct
from virelia.localize_type_icons import read_png
from virelia.build_quarry_outdoor import BANK, ROOT, ART, YARD, TERRACES, BASIN, HAUL, GATES, CONTROLS
from virelia.quarry_outdoor_graph import QuarryGraph, require
from validate_virelia_missing_tools import validate as validate_routes


def words(path):
    raw=path.read_bytes()
    return struct.unpack(f'<{len(raw)//2}H',raw)


def validate():
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
        require(attrs[packing['assets'][asset][0][0]-512]==0x1000,'Haul tracks must remain encounter-free ordinary ground')
    for name in (YARD,TERRACES,BASIN,HAUL):
        source=(ROOT/f'data/maps/{name}/scripts.inc').read_text(encoding='utf-8')
        require('callnative VireliaQuarry_Transition' in source and 'callnative VireliaQuarry_RefreshMap' in source,'Missing load/entry callbacks')
    validate_routes()
    print(f'PASS: {packing["graphics"]}/496 quarry graphics; exact Aseprite props, opaque ground, canopy layers, powered controls and hardware limits')


def self_test():
    mutations=0
    # Exercise actual graphs with broken source blocks and connection metadata.
    for kind in ('gate','seam','control','elevation'):
        world=QuarryGraph()
        if kind=='gate':
            for x,y in GATES[HAUL][:2]:world.blocks[HAUL][y*36+x]&=~0xC00
        elif kind=='seam':
            world.maps[BASIN]['connections'][1]['offset']=-7
        elif kind=='control':
            x,y=CONTROLS['pump_control']
            world.blocks[BASIN][(y+2)*40+x]|=0x400
        else:
            for y in range(23,28):
                for x in (10,11):world.blocks[TERRACES][y*40+x]|=0x3000
        try:
            validate_routes(world)
        except ValueError:
            mutations+=1
        else:
            raise ValueError(f'Outdoor route validator accepted broken {kind} data')
    print(f'PASS: {mutations} broken gate, seam, control approach and stair-elevation mutations detected')


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--self-test',action='store_true')
    args=parser.parse_args()
    validate()
    if args.self_test:self_test()
