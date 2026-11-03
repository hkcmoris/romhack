#!/usr/bin/env python3
"""Render authored native quarry blocks and actors for visual review (stdlib)."""
from functools import lru_cache
from pathlib import Path
import json
import struct
import zlib
import sys
try:
    from .localize_type_icons import read_png
    from .build_quarry_outdoor import NAMES, ORIGINS, ROOT
except ImportError:
    from localize_type_icons import read_png
    from build_quarry_outdoor import NAMES, ORIGINS, ROOT


@lru_cache(None)
def words(path):
    raw=(ROOT/path).read_bytes()
    return struct.unpack(f'<{len(raw)//2}H',raw)


@lru_cache(None)
def pixels(path):
    return read_png(ROOT/path)[0]


@lru_cache(None)
def palette(path):
    return [tuple(map(int,line.split())) for line in (ROOT/path).read_text().splitlines()[3:19]]


def png(path,rows):
    def chunk(k,d):return struct.pack('>I',len(d))+k+d+struct.pack('>I',zlib.crc32(k+d))
    h,w=len(rows),len(rows[0])//3
    path.parent.mkdir(parents=True,exist_ok=True)
    path.write_bytes(b'\x89PNG\r\n\x1a\n'+chunk(b'IHDR',struct.pack('>IIBBBBB',w,h,8,2,0,0,0))+chunk(b'IDAT',zlib.compress(b''.join(b'\0'+r for r in rows)))+chunk(b'IEND',b''))


def render():
    registry=json.loads((ROOT/'data/layouts/layouts.json').read_text())['layouts']
    images={}
    bank='data/tilesets/secondary/virelia_quarry_outdoor'
    primary='data/tilesets/primary/general'
    for name in NAMES:
        entry=json.loads((ROOT/f'data/maps/{name}/map.json').read_text())
        l=next(x for x in registry if x['id']==entry['layout'])
        rows=[bytearray(l['width']*16*3) for _ in range(l['height']*16)]
        for i,cell in enumerate(words(l['blockdata_filepath'])):
            index=cell&1023
            source=primary if index<512 else bank
            for part,value in enumerate(words(source+'/metatiles.bin')[index%512*8:(index%512+1)*8]):
                tile,slot=value&1023,value>>12
                image=pixels((primary if tile<512 else bank)+'/tiles.png')
                pal=palette((primary if slot<6 else bank)+f'/palettes/{slot:02}.pal')
                tile%=512
                for y in range(8):
                    for x in range(8):
                        p=image[tile//16*8+(7-y if value&2048 else y)][tile%16*8+(7-x if value&1024 else x)]
                        if p:
                            xx=i%l['width']*16+part%2*8+x
                            yy=i//l['width']*16+part%4//2*8+y
                            rows[yy][xx*3:xx*3+3]=bytes(pal[p])
        for obj in sorted(entry['object_events'],key=lambda o:o['y']):
            gfx=obj['graphics_id']
            if gfx.startswith('OBJ_EVENT_GFX_QUARRY_'):
                path='graphics/object_events/pics/quarry/'+gfx.removeprefix('OBJ_EVENT_GFX_QUARRY_').lower()+'.png'
                palpath='graphics/object_events/palettes/moving_box.pal'
                size=16
            elif gfx in ('OBJ_EVENT_GFX_HIKER','OBJ_EVENT_GFX_WOMAN_2','OBJ_EVENT_GFX_ITEM_BALL'):
                kind,palname={'OBJ_EVENT_GFX_HIKER':('people/hiker','npc_1'),'OBJ_EVENT_GFX_WOMAN_2':('people/woman_2','npc_3'),'OBJ_EVENT_GFX_ITEM_BALL':('misc/ball_poke','npc_3')}[gfx]
                path='graphics/object_events/pics/'+kind+'.png';palpath='graphics/object_events/palettes/'+palname+'.pal';size=32
            else:
                path='graphics/pokemon/sableye/overworld.png';palpath='graphics/pokemon/sableye/overworld_normal.pal';size=32
            image=pixels(path);pal=palette(palpath)
            width=min(len(image[0]),32 if 'SABLEYE' in gfx else 16)
            for y in range(size):
                for x in range(width):
                    p=image[y][x]
                    xx=obj['x']*16+(16-width)//2+x;yy=(obj['y']+1)*16-size+y
                    if p and 0<=yy<len(rows) and 0<=xx<len(rows[0])//3:rows[yy][xx*3:xx*3+3]=bytes(pal[p])
        images[name]=rows
        png(ROOT/'outputs/quarry-outdoor'/f'{name}.png',rows)
    width=max(ORIGINS[n][0]*16+len(rows[0])//3 for n,rows in images.items())
    height=max(ORIGINS[n][1]*16+len(rows) for n,rows in images.items())
    composite=[bytearray(bytes((46,52,56))*width) for _ in range(height)]
    for name,(ox,oy) in ORIGINS.items():
        for y,row in enumerate(images[name]): composite[oy*16+y][ox*16*3:ox*16*3+len(row)]=row
    png(ROOT/'outputs/quarry-outdoor/overview.png',composite)
    if '--docs' in sys.argv:
        png(ROOT/'docs/images/virelia-quarry/outdoor-layout.png',composite)
    print('Rendered four native layouts and actors to outputs/quarry-outdoor/')


if __name__=='__main__':render()
