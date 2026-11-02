"""Read real quarry collisions, elevations, objects and native map seams."""
from collections import deque
import json
from pathlib import Path
import re
import struct
from .build_quarry_outdoor import NAMES, GATES, YARD

ROOT = Path(__file__).resolve().parents[2]


def require(ok, message):
    if not ok:
        raise ValueError(message)


class QuarryGraph:
    def __init__(self):
        self.maps = {n: json.loads((ROOT / f'data/maps/{n}/map.json').read_text()) for n in NAMES}
        registry = json.loads((ROOT / 'data/layouts/layouts.json').read_text())['layouts']
        self.layouts = {n: next(l for l in registry if l['id'] == m['layout']) for n,m in self.maps.items()}
        self.blocks, self.occupied = {}, {}
        for n,l in self.layouts.items():
            raw = (ROOT / l['blockdata_filepath']).read_bytes()
            self.blocks[n] = list(struct.unpack(f'<{len(raw)//2}H',raw))
            require(len(self.blocks[n]) == l['width']*l['height'], f'{n}: incorrect native layout size')
            self.occupied[n] = {(o['x'],o['y']) for o in self.maps[n]['object_events']}
        self.attrs = {}
        for bank in ('primary/general','secondary/virelia_quarry_outdoor'):
            raw = (ROOT / f'data/tilesets/{bank}/metatile_attributes.bin').read_bytes()
            self.attrs[bank] = struct.unpack(f'<{len(raw)//2}H',raw)
        self.behaviors = re.findall(r'\b(MB_[A-Z0-9_]+)\s*,',(ROOT / 'include/constants/metatile_behaviors.h').read_text())
        self.ids = {m['id']:n for n,m in self.maps.items()}

    def cell(self, node):
        n,x,y = node
        l = self.layouts[n]
        return self.blocks[n][y*l['width']+x] if 0<=x<l['width'] and 0<=y<l['height'] else None

    def behavior(self,node):
        cell = self.cell(node)
        if cell is None: return None
        index = cell & 1023
        bank = 'primary/general' if index<512 else 'secondary/virelia_quarry_outdoor'
        return self.behaviors[self.attrs[bank][index%512]&255]

    def walkable(self,node,powered):
        n,x,y = node
        cell = self.cell(node)
        if cell is None or (x,y) in self.occupied[n]: return False
        if self.behavior(node) in {'MB_POND_WATER','MB_OCEAN_WATER','MB_WATERFALL'}: return False
        return not cell&0xC00 or powered and (x,y) in GATES.get(n,())

    def same_level(self,a,b):
        x,y = self.cell(a)>>12,self.cell(b)>>12
        return x==y or bool({x,y}&{0,15})

    def graph(self,powered,exclude_connections=frozenset()):
        edges = {}
        for n,l in self.layouts.items():
            require(self.maps[n]['map_type'] == 'MAP_TYPE_ROUTE', f'{n}: quarry must be outdoors')
            require(l['primary_tileset']=='gTileset_General' and l['secondary_tileset']=='gTileset_VireliaQuarryOutdoor', f'{n}: outdoor component bank mismatch')
            for y in range(l['height']):
                for x in range(l['width']):
                    node = (n,x,y)
                    if self.walkable(node,powered):
                        edges[node] = [other for dx,dy in ((0,1),(0,-1),(1,0),(-1,0))
                                       if self.walkable(other:=(n,x+dx,y+dy),powered) and self.same_level(node,other)]
        opposite = dict(up='down',down='up',left='right',right='left')
        for n,entry in self.maps.items():
            w,h = self.layouts[n]['width'],self.layouts[n]['height']
            for c in entry['connections']:
                target = self.ids[c['map']]
                d,o = c['direction'],int(c['offset'])
                require(any(b['map']==entry['id'] and b['direction']==opposite[d] and b['offset']==-o for b in self.maps[target]['connections']), f'{n}: nonreciprocal seam')
                tw,th = self.layouts[target]['width'],self.layouts[target]['height']
                count = 0
                for k in range(w if d in ('up','down') else h):
                    if d in ('up','down'):
                        a,b=(n,k,0 if d=='up' else h-1),(target,k-o,th-1 if d=='up' else 0)
                    else:
                        a,b=(n,0 if d=='left' else w-1,k),(target,tw-1 if d=='left' else 0,k-o)
                    if a in edges and b in edges and self.same_level(a,b):
                        count+=1
                        if frozenset((n,target)) not in exclude_connections: edges[a].append(b)
                require(count>=2,f'{n}: seam to {target} lacks two passable compatible cells')
        return edges

    def reached(self,powered,start=(YARD,9,28),exclude_connections=frozenset()):
        graph=self.graph(powered,exclude_connections)
        require(start in graph,f'Blocked graph start {start}')
        found,queue={start},deque([start])
        while queue:
            for node in graph[queue.popleft()]:
                if node not in found:
                    found.add(node);queue.append(node)
        return found

    def approaches(self,n,event):
        return {(n,event['x']+dx,event['y']+dy) for dx,dy in ((0,1),(0,-1),(1,0),(-1,0))}
