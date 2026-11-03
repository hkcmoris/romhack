"""The ridge road and the estuary causeway on the landward shoulder of Seabreak."""

import struct
from cliff_transitions import coast_contact
from cliff_contours import coast_outline, shared_crest, turns

from native_terrain import close_edge, paint_river, plant_trees


def build_approaches(root, bank, heath, spruce, rock, surfaces):
    width, height = 16, 56
    grid = [[0] * width for _ in range(height)]

    def put(x, y, tile, elevation, solid=False):
        grid[y][x] = (elevation << 12) | (0x400 if solid else 0) | tile

    # Two arms of the estuary reach in from the west. The lower road stays on
    # the rocky spit between them, and the ridge road overlooks both.
    shoreline = dict(enumerate((0,2,5,7,8,9,9,8,6,5,2,1,0), 20))
    shoreline.update(enumerate((1,3,4,6,8,8,7,6,6,5,3,2,1,1,1), 42))
    shelf = (13,13,12,12,13,14,15,15,16,16,15,15,14,14,13,13)
    # The native lawn stays with the earth trail until the first whole
    # paving courses. Each verge then sweeps into the coastal heath.
    shoulder = (9,9,10,10,11,11,12,13,14,14,14,13,12,11,10,10,
                9,9,9,10,10,10,9,9,9,10,10,10,10,10,10,10,
                10,11,12,13,14,14,13,12,12,11,10,9,9,9,9,10,
                10,10,10,10,9,9,9,9)
    for y in range(height):
        for x in range(width):
            elev = 7 if y <= shelf[x] else 5 if y < 27 else 3
            put(x, y, heath if x >= shoulder[y] else 1, elev,
                y in (0, height-1))
    wet = {(x,y) for y,reach in shoreline.items() for x in range(-1, reach+1)}
    paint_river(grid, wet, elevation=1, south_bank=False)
    soil = [[block & 1023 for block in row] for row in grid]

    # Exposed rock changes from warm foothill stone to coastal schist at the
    # buttresses. The fault separates the two trails throughout this map.
    def exposed_rock(tile, x, course):
        # The fracture leans east as it descends. Leave the eight western
        # columns in General so the Rainfall camera draws them correctly.
        stage = x - 8 - (course // 2)
        if stage < 0:
            return tile
        attribute = bank.general_attrs[tile] if x < 10 else bank.attrs[rock[tile]-512]
        return coast_contact(bank, tile, stage, attribute) if stage < 4 else rock[tile]

    def rock_wall(profile, depth, elevation):
        for x, crest in enumerate(profile):
            west = profile[max(0,x-1)]
            east = profile[min(width-1,x+1)]
            sides = turns(west, crest, east)
            cap = shared_crest(sides) if x < 8 else 0x087
            toe = 0x078 if west < crest else 0x07A if east < crest else 0x079
            for dy in range(depth):
                tile = cap if dy == 0 else toe if dy == depth-1 else 0x0A9
                if x < 8:
                    surface = tile
                else:
                    surface = exposed_rock(tile, x, dy)
                    if dy in (0, depth-1):
                        gy = crest if dy == 0 else min(height-1,crest+depth)
                        ground = grid[gy][x] & 1023
                        surface = coast_outline(surfaces,surface,ground,sides,foot=dy != 0)
                put(x, crest+dy, surface, elevation, True)

    rock_wall(shelf, 5, 5)
    rock_wall((0,0,1,2,3,3,2,1,1,0,0,1,2,2,1,1), 2, 7)

    def lane(profile, depth, level):
        path = {(x,y+dy) for x,y in enumerate(profile) for dy in range(depth)}
        for x,y in path:
            top = (x,y-1) not in path
            bottom = (x,y+1) not in path
            left = x > 0 and (x-1,y) not in path
            right = x < width-1 and (x+1,y) not in path
            col = 0 if left else 2 if right else 1
            tile = (0x118 if top else 0x128 if bottom else 0x120) + col
            if level == 7 and x >= 11:
                paved = bank.tile("slateport", (0x208 if top else 0x218 if bottom else 0x210) + col)
                tile = surfaces.worn_landing(paved, tile, top, bottom) if x == 11 else paved
            put(x, y, tile, level)

    lane((8,8,8,7,7,7,7,7,8,8,8,8,8,8,8,8), 2, 7)
    lane((36,36,36,36,35,35,35,35,36,36,36,36,36,36,36,36), 2, 3)
    # The paved landing widens into the quay. Its low south wall keeps
    # travelers on the upper lip as they pass the bollard into town.
    for x in range(11,16):
        paved = bank.tile("slateport", 0x202)
        put(x,36,surfaces.worn_landing(paved,0x119,top=True) if x == 11 else paved,3)
        face = bank.tile("slateport", 0x25D)
        put(x,37,face,3,True)
    for x in (11,12,13,14):
        put(x,35,bank.tile("slateport", 0x202),3)
    put(12,35,bank.tile("slateport", 0x238),3,True)
    for x in (0,1):
        put(x,36,0x121,3)

    # Broadleaves belong to the sheltered inlet. The exposed headland uses
    # the same heath and spruce crowns as the keeper's lane across the street.
    plant_trees(grid, ((2,4),(5,10)), elevation=7)
    plant_trees(grid, ((2,38),(6,32)), elevation=3)
    plant_trees(grid, ((12,4),(11,27),(13,40),(11,49)), pieces=spruce, ground=heath)
    for x,y,e in ((8,5,7),(4,11,7),(7,38,3),(9,33,3)):
        put(x,y,0x004,e,True)
    for side in ('left','right'):
        close_edge(grid, side, (8,9,36))
    surfaces.finish(grid, soil, native_west=8)
    folder = root / 'data/layouts/Virelia_SeabreakApproaches'
    folder.mkdir(parents=True, exist_ok=True)
    (folder/'map.bin').write_bytes(b''.join(struct.pack('<16H', *row) for row in grid))
    (folder/'border.bin').write_bytes(struct.pack('<4H', *([0x3400 | heath] * 4)))
