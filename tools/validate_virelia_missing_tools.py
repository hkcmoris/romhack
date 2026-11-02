#!/usr/bin/env python3
"""Audit the outdoor quarry's locked/restored loop and unchanged tools quest."""
from pathlib import Path
from virelia.build_quarry_outdoor import YARD,TERRACES,BASIN,HAUL,NAMES,GATES,CONTROLS
from virelia.quarry_outdoor_graph import QuarryGraph,require

ROOT=Path(__file__).resolve().parents[1]


def validate(world=None):
    if world is None:
        world=QuarryGraph()
    locked,opened=world.reached(False),world.reached(True)
    require({n for n,x,y in opened}==set(NAMES),'Outdoor quarry component is disconnected')
    for name,entry in world.maps.items():
        for obj in entry['object_events']:
            near=world.approaches(name,obj)
            require(near & opened,f'{name}: unreachable {obj["script"]}')
            protected=name==HAUL or obj['script'].endswith('_Evidence')
            if protected: require(not near & locked,f'{name}: power gate bypass reaches {obj["script"]}')
            else: require(near & locked,f'{name}: first-visit object is blocked')
            require(world.cell((name,obj['x'],obj['y']))>>12==obj['elevation'],f'{name}: actor elevation mismatch')
            if obj['trainer_type']=='TRAINER_TYPE_NORMAL':
                dx,dy={'DOWN':(0,1),'UP':(0,-1),'LEFT':(-1,0),'RIGHT':(1,0)}[obj['movement_type'].rsplit('_',1)[-1]]
                facing=(name,obj['x']+dx,obj['y']+dy)
                require(facing in locked and world.same_level((name,obj['x'],obj['y']),facing),f'{name}: trainer faces blocked terrain')
        for event in entry['bg_events']:
            require(world.approaches(name,event)&opened,f'{name}: inaccessible sign/control {event["script"]}')
        start=next(node for node in opened if node[0]==name)
        require((YARD,9,28) in world.reached(True,start),f'{name}: cannot return to Slateford')
    for asset,(x,y) in CONTROLS.items():
        require((BASIN,x,y+2) in locked,f'{asset}: south-facing control approach blocked')
    excluded=frozenset((frozenset((YARD,TERRACES)),))
    require((YARD,9,28) in world.reached(True,(BASIN,10,4),excluded),'Powered gallery is not a two-way shortcut')
    require((YARD,9,28) not in world.reached(False,(BASIN,10,4),excluded),'Unpowered return loop bypasses its barriers')
    for name,cells in GATES.items():
        for x,y in cells:
            require(world.cell((name,x,y))&0xC00,f'{name}: authored gate starts open')
    gallery=(ROOT/f'data/maps/{HAUL}/scripts.inc').read_text(encoding='utf-8')
    for helper in ('BeginMissingTools','CollectTool','RecoverTools','RewardTools'):
        require(f'callnative VireliaFirstGym_{helper}' in gallery,f'Missing tools helper {helper}')
    require('setwildbattle SPECIES_SABLEYE, 13' in gallery and 'dowildbattle' in gallery,'Sableye must stay catchable')
    require('dynmultichoice' in gallery and all('Text_'+n in gallery for n in ('Fight','Catch','Later')),'Sableye encounter choices changed')
    require(world.maps[HAUL]['object_events'][0]['flag']=='FLAG_VIRELIA_MISSING_TOOLS_RECOVERED','Resolved Sableye must stay hidden')
    require(len(world.maps[YARD]['warp_events'])==1 and all(not world.maps[n]['warp_events'] for n in (TERRACES,BASIN,HAUL)),'Outdoor sections must use native seams, not ladders')
    print('PASS: four connected outdoor sections, both controls before power, gated evidence/tools, powered two-way return loop')


if __name__=='__main__':
    validate()
