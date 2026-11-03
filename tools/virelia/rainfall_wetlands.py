"""The lower Stormcrest crossing, from the peat islands to the tidal landing."""

import struct

from PIL import Image, ImageDraw

from native_terrain import paint_river, plant_trees


# Each corner sees two adjacent cardinal edges and their common diagonal.
# A dry diagonal only matters if the adjacent cardinals are both still water.
QUADRANTS = ((1, 8, 16), (1, 2, 32), (4, 8, 128), (4, 2, 64))
PEAT_SLICES = {
    9: 0x377, 1: 0x378, 3: 0x379, 8: 0x37A, 0: 0x37B, 2: 0x37C,
    12: 0x37D, 4: 0x37E, 6: 0x37F, 16: 0x380, 32: 0x381, 128: 0x382, 64: 0x383,
}
COMPOUND = [mask for mask in range(256) if mask not in PEAT_SLICES
            and all(not mask & diagonal or not mask & (a | b) for a, b, diagonal in QUADRANTS)]
SHORE = PEAT_SLICES.copy()
SHORE.update((mask, 0x3C0 + i) for i, mask in enumerate(COMPOUND))
MOSS_SLICES = {mask: tile + (0x391 - 0x377) for mask, tile in PEAT_SLICES.items()}
MOSS = MOSS_SLICES.copy()
MOSS.update((mask, 0x3F2+i) for i, mask in enumerate((5,10,7,11,13,14,18,20,36,40,65,72,129,130)))


def pack_shore_corners(root):
    # These 34 shapes use only quarters from the 13 original shore metatiles.
    # A one-tile mud spit can show an exposed edge AND the opposite inlet.
    folder = root / "data/tilesets/secondary/virelia_rainfall"
    definition_file = folder / "metatiles.bin"
    attribute_file = folder / "metatile_attributes.bin"
    definitions = bytearray(definition_file.read_bytes())
    attributes = bytearray(attribute_file.read_bytes())
    # Boardwalk end caps for the east/west transitions on the lower route.
    length = 512
    definitions.extend(bytes(max(0, length * 16 - len(definitions))))
    attributes.extend(bytes(max(0, length * 2 - len(attributes))))
    for full, slices, attr in ((SHORE, PEAT_SLICES, 0x1010), (MOSS, MOSS_SLICES, 0x1000)):
        for mask in full.keys() - slices.keys():
            dest = full[mask] - 512
            for q, (a, b, diagonal) in enumerate(QUADRANTS):
                source = slices[(mask & (a | b)) or (mask & diagonal)] - 512
                for layer in (0, 4):
                    start = source * 16 + (layer + q) * 2
                    end = dest * 16 + (layer + q) * 2
                    definitions[end:end+2] = definitions[start:start+2]
            struct.pack_into("<H", attributes, dest * 2, attr)
    primary = (root / "data/tilesets/primary/general/metatiles.bin").read_bytes()
    for start, backdrop in ((0x3E4, 1), (0x3EB, 0x170)):
        for i in range(7):
            dest, src = (start+i-512)*16, (0x370+i-512)*16
            definitions[dest:dest+8] = primary[backdrop*16:backdrop*16+8]
            definitions[dest+8:dest+16] = definitions[src+8:src+16]
            struct.pack_into("<H", attributes, (start+i-512)*2, 0x1000)
    for tile, source, attr in ((0x3E2, 0x3E4, 0x1063), (0x3E3, 0x3E6, 0x1062)):
        dest, src = (tile - 512) * 16, (source - 512) * 16
        definitions[dest:dest+16] = definitions[src:src+16]
        struct.pack_into("<H", attributes, (tile-512)*2, attr)
    definition_file.write_bytes(definitions)
    attribute_file.write_bytes(attributes)


def banks(t, outlines):
    mask = Image.new("1", (t.w, t.h))
    draw = ImageDraw.Draw(mask)
    for outline in outlines:
        draw.polygon(outline, fill=1)
    return {(x, y) for y in range(t.h) for x in range(t.w) if mask.getpixel((x, y))}


def edge_mask(x, y, outside):
    mask = sum(bit for bit, dx, dy in ((1,0,-1), (2,1,0), (4,0,1), (8,-1,0))
               if outside(x+dx, y+dy))
    for (a, b, diagonal), (dx, dy) in zip(QUADRANTS, ((-1,-1),(1,-1),(-1,1),(1,1))):
        if not mask & (a | b) and outside(x+dx, y+dy):
            mask |= diagonal
    return mask


def peat(t, land):
    def dry(x, y):
        return (x, y) in land
    for y in range(t.h):
        for x in range(t.w):
            if dry(x, y):
                continue
            mask = edge_mask(x,y,dry)
            tile = SHORE[mask] if mask else 0x368 if (13*x + 7*y + x*y) % 23 == 0 else 0x367
            t.put(x, y, tile, True)


def reed_beds(t, beds):
    """Draw rush beds around sheltered banks; keep the shore contours intact."""
    for x, y, rx, ry, plant in beds:
        for yy in range(max(0, y - ry), min(t.h, y + ry + 1)):
            for xx in range(max(0, x - rx), min(t.w, x + rx + 1)):
                if t.g[yy][xx] & 1023 not in (0x367, 0x368):
                    continue
                radius = ((xx - x) / rx) ** 2 + ((yy - y) / ry) ** 2
                grain = (17*xx + 11*yy + xx*yy) % 13
                if radius < 1 and grain < 9 - 5*radius:
                    t.put(xx, yy, plant, True)


def footway(t, *points):
    """One plank across. Junctions and turns have the square pegged deck."""
    def board(x, y, piece):
        ground = t.g[y][x] & 1023
        if ground in (1, 2) or 0x3E4 <= ground <= 0x3EA:
            piece += 0x3E4 - 0x370
        elif 0x170 <= ground <= 0x19A or 0x3EB <= ground <= 0x3F1:
            piece += 0x3EB - 0x370
        t.put(x, y, piece)

    for (x1, y1), (x2, y2) in zip(points, points[1:]):
        if y1 == y2:
            lo, hi = sorted((x1, x2))
            for x in range(lo, hi + 1):
                board(x, y1, 0x370 if x == lo else 0x372 if x == hi else 0x371)
        elif x1 == x2:
            lo, hi = sorted((y1, y2))
            for y in range(lo, hi + 1):
                board(x1, y, 0x373 if y == lo else 0x375 if y == hi else 0x374)
        else:
            raise ValueError(f"Boardwalk must turn at a post: {(x1, y1), (x2, y2)}")
    for x, y in points[1:-1]:
        board(x, y, 0x376)


def terrace(t, start_x, crest, drops):
    """The green lip, wet vertical face, and low mossy foot of a gorge wall."""
    x = start_x
    for length, offset, depth in drops:
        for j in range(length):
            top = crest + offset
            t.put(x, top, 0x342, True)
            for y in range(top + 1, top + depth):
                tile = 0x34B if ((x + 5*y) % 9 == 0) else 0x34A if ((3*x + y) % 5 == 0) else 0x345
                t.put(x, y, tile, True)
            t.put(x, top + depth, 0x348, True)
            x += 1
    first_y = crest + drops[0][1]
    last_y = crest + drops[-1][1]
    for col, y, depth, side in ((start_x, first_y, drops[0][2], 0), (x-1, last_y, drops[-1][2], 2)):
        t.put(col, y, 0x341 + side, True)
        for yy in range(y + 1, y + depth):
            t.put(col, yy, 0x344 + side, True)
        t.put(col, y + depth, 0x347 + side, True)


def fit_peat_to_cliff(t, land):
    """Let peat lap against exposed rock; keep grassy banks along actual turf."""
    walls = {(x,y) for y,row in enumerate(t.g) for x,block in enumerate(row)
             if 0x344 <= (block & 1023) <= 0x34B}
    water_ids = set(SHORE.values()) | set(range(0x367,0x36D))
    wet = {(x,y) for y,row in enumerate(t.g) for x,block in enumerate(row) if block & 1023 in water_ids}
    shore = set()
    for x,y in walls:
        adjacent = {(x+dx,y+dy) for dy in (-1,0,1) for dx in (-1,0,1)} & wet
        if not adjacent:
            continue
        shore.update(adjacent)
        tile = t.g[y][x] & 1023
        if tile in (0x347,0x348,0x349):
            if (x+1,y) in wet:
                tile = 0x305
            elif (x-1,y) in wet:
                tile = 0x2FD
            elif (x,y+1) in wet:
                tile = {0x347:0x2FD, 0x349:0x305}.get(tile, tile)
        elif tile == 0x344 and (x-1,y) in wet:
            tile = 0x30D
        t.g[y][x] = (t.g[y][x] & ~1023) | tile
    turf = land - walls
    for x,y in shore:
        mask = edge_mask(x,y,lambda xx,yy: (xx,yy) in turf)
        block = t.g[y][x]
        if mask or (block & 1023) in SHORE.values():
            t.g[y][x] = (block & ~1023) | (SHORE[mask] if mask else 0x367)


def cascades(t, x, y, height):
    for dy in range(height):
        base = 0x34C if dy == 0 else 0x352 if dy == height - 1 else 0x34F
        for dx in range(3):
            t.put(x + dx, y + dy, base + dx, True)


def meadow(t, patches):
    """Edges of the walkable islets carry sedges and iris, with occasional bare mud."""
    for cx, cy, rx, ry in patches:
        for y in range(max(0, cy-ry), min(t.h, cy+ry+1)):
            for x in range(max(0, cx-rx), min(t.w, cx+rx+1)):
                ground = t.g[y][x] & 1023
                if ground not in (1, 0x395):
                    continue
                r = ((x-cx)/rx)**2 + ((y-cy)/ry)**2
                if r <= 1:
                    seed = (13*x + 19*y + x*y) % 17
                    rush = 0x39E if ground == 0x395 else 0x36D
                    tile = rush + (1 if seed < 3 else 2 if seed > 12 else 0)
                    t.put(x, y, tile)


def moss(t, swaths):
    cells = {(x,y) for y in range(t.h) for x in range(t.w) if t.g[y][x] & 1023 == 1
             and any(((x-cx)/rx)**2 + ((y-cy)/ry)**2 <= 1 for cx,cy,rx,ry in swaths)}
    def outside(x,y):
        return (x,y) not in cells
    # Thin tips around roots or a winding shore recede to the grass shoulder
    # if they would require a more complex shape than the packed moss kit.
    while True:
        cut = {p for p in cells if edge_mask(*p, outside) not in MOSS}
        if not cut:
            break
        cells -= cut
    for x,y in cells:
        t.put(x,y,MOSS[edge_mask(x,y,outside)])


def lower(Terrain):
    t = Terrain("RainfallLower", 64, 54)
    land = banks(t, (
        # Shaded rock and spring above the western grotto.
        ((0,0),(26,0),(26,4),(20,8),(19,11),(13,13),(9,16),(3,16),(0,13)),
        ((1,20),(2,14),(4,14),(4,18),(6,20),(7,23),(7,26),(5,28),(2,27)),
        # The entry peninsula and two isolated hummocks at the first fork.
        ((1,34),(4,32),(8,32),(9,35),(15,37),(15,41),(11,43),(7,45),
         (6,49),(3,51),(1,48),(2,44),(1,42)),
        ((18,36),(23,34),(24,31),(27,31),(29,34),(28,36),(28,41),
         (25,42),(22,45),(19,44),(17,40)),
        ((24,48),(30,46),(33,47),(33,50),(30,52),(26,51)),
        # Winch island, its western cache and the central blind.
        ((11,18),(15,17),(19,18),(22,21),(24,24),(22,28),(18,29),
         (15,27),(12,27),(10,24)),
        ((36,24),(38,23),(40,25),(39,28),(36,29),(35,27)),
        # The route doubles south around the great reed bed.
        ((33,35),(36,33),(40,34),(42,32),(45,32),(46,37),(43,40),
         (39,42),(35,41),(32,38)),
        ((51,24),(53,22),(57,22),(59,26),(58,30),(61,33),(60,36),
         (57,39),(53,38),(51,35),(53,31)),
        # North reed bar and the last bridge to the east gate.
        ((28,9),(32,7),(35,8),(37,12),(41,11),(43,10),(46,12),(47,16),
         (45,19),(47,22),(47,25),(44,25),(42,22),(42,19),(39,17),
         (35,16),(32,14),(28,13)),
        ((50,4),(54,2),(60,3),(62,6),(61,10),(62,14),(61,19),(59,21),(54,19),
         (53,16),(50,15),(49,11)),
        # Distant forest pockets and rock islets set the edge of the basin.
        ((0,52),(6,52),(10,53),(0,53)), ((12,3),(16,2),(16,5),(13,6)),
        ((35,1),(38,0),(42,1),(42,3),(39,4),(36,3)),
        ((4,7),(8,7),(8,9),(4,9)), ((30,3),(32,2),(33,3),(31,4)),
        ((48,45),(52,43),(54,46),(54,49),(51,50),(47,48)),
        ((60,43),(63,41),(63,52),(59,52),(57,49)),
        ((13,47),(15,47),(17,49),(15,51),(13,50)),
    ))
    # The western arm of the fen separates the long entry boardwalk from the
    # cable crossing. Its taper and small peat hags use the regular shore kit.
    for y, last in enumerate((6,8,10,9,7,5,5,4,3,3,4,5,7,8,7,5,3,2,1), 16):
        land.difference_update((x,y) for x in range(last+1))
    # The bordering Foothills continue onto dry turf. Include that shore
    # when picking outer banks and inside corners in the connected camera view.
    land.update((-1,y) for y in range(42))
    # The dry north lip of the estuary meets the fen across its eastern seam.
    land.update((t.w,y) for y in range(6))
    peat(t, land)
    terrace(t, 1, 10, ((4,1,4),(4,0,4),(5,-1,5),(5,-3,5),(4,-4,4),(3,-6,4)))
    # Spring water, no longer stained by peat, drops past the sealed grotto.
    pool = {(x,y) for y, left, right in ((13,6,12),(14,5,13),(15,5,13),
                                          (16,5,13),(17,6,12),(18,7,11)) for x in range(left,right+1)}
    paint_river(t.g, pool, elevation=3)
    cascades(t, 7, 10, 6)
    t.put(3,13,0x09F,True)
    t.put(3,14,0x0A7,True)
    t.put(3,15,1)
    # The southern branch ends on an observation islet. Across the second
    # crossing, turning west leads to the cache and the sound of the falls.
    footway(t, (13,40), (19,40))
    footway(t, (23,43), (23,48), (28,48))
    footway(t, (24,33), (24,28), (21,28))
    footway(t, (12,24), (7,24))
    footway(t, (22,24), (27,24), (27,21), (35,21), (35,26), (38,26))
    footway(t, (31,21), (31,17), (33,17))
    t.put(31,21,0x376)
    footway(t, (37,27), (37,35))
    footway(t, (45,36), (53,36))
    footway(t, (46,24), (54,24))
    t.bridge(47, 13, 4)
    footway(t, (31,11), (25,11), (25,14))
    # An unfinished spur ends at open water, in sight of the far reed bar.
    footway(t, (58,32), (61,32), (61,28))
    reed_beds(t, (
        (31,23,6,3,0x36A),(33,18,4,2,0x36C),(28,26,4,2,0x36B),
        (7,30,8,3,0x36A),(14,34,5,3,0x369),(21,37,7,3,0x36B),(12,46,4,4,0x36B),
        (20,49,4,4,0x369),(31,43,5,4,0x36C),(35,31,7,2,0x36A),
        (25,19,6,5,0x36B),(15,15,5,3,0x36C),(2,30,3,4,0x369),
        (47,30,6,5,0x36A),(42,28,3,3,0x369),(48,22,4,4,0x36C),
        (31,17,6,2,0x369),(56,20,6,2,0x36A),(49,7,4,5,0x36B),
        (22,7,6,4,0x36C),(44,5,5,3,0x369),(37,45,4,4,0x369),
        (55,45,5,4,0x36B),(4,47,5,4,0x36C),(4,19,3,2,0x36A),
    ))
    # Shallow, traversable side pools within the dry banks use the marsh
    # nine-slice. The open peat outside their banks is visibly darker and deep.
    t.marsh(34, [(4,6), (3,7), (3,7), (4,6)])
    t.marsh(5, [(56,59),(55,60),(55,60),(56,59)])
    # The trail changes map where the boardwalk disappears around a bank.
    # The visible stretch behind each threshold continues across the water.
    footway(t, (1,42), (1,40), (10,40))
    footway(t, (55,18), (61,18), (61,16))
    plant_trees(t.g, ((1,1),(1,3),(4,0),(9,0),(11,1),(13,3),(17,2),
                      (22,1),(24,0),(2,6),(4,7),(11,5),(15,4),
                      (54,3),(59,10)), pieces=(0x389, 0x38B, 0x38D, 0x38F))
    plant_trees(t.g, (
        (6,2),(6,4),(15,0),(19,0),(8,5),(19,3),(37,1),(40,1),
        (52,7),(59,14),
        (30,8),(34,12),(42,13),(44,17),
        (13,19),(13,22),(15,24),(19,25),(20,21),
        (7,32),(10,36),(2,43),(4,44),(2,46),
        (18,37),(19,41),(26,32),(26,36),(24,38),(26,39),(26,49),(29,48),
        (33,36),(35,38),(39,38),(42,34),(42,36),
        (54,25),(56,26),(55,31),(58,33),(54,35),(57,36),
        (49,45),(51,46),(60,45),(60,47),(60,49),(14,48),
    ))
    moss(t, (
        (4,6,6,4),(12,2,8,4),(21,2,6,3),(4,42,9,6),(9,39,6,4),
        (22,38,7,6),(27,49,4,3),(16,21,8,5),(37,26,3,3),(40,37,7,6),
        (55,30,6,8),(42,16,6,5),(32,12,5,3),(55,13,8,8),(50,47,4,3),
    ))
    meadow(t, (
        (4,8,4,3),(6,44,5,3),(12,39,4,3),(22,36,6,3),(25,41,4,2),
        (29,49,5,2),(14,20,4,3),(17,26,4,2),(35,27,3,2),(42,37,5,3),
        (54,27,4,4),(59,34,4,3),(45,22,3,4),(33,11,4,3),(54,11,5,4),
        (58,15,5,2),(51,46,3,2),
    ))
    # Flood gauge marks the seaward end of the blind; the item sits in the
    # shallow rock alcove, at the top of the western spur.
    t.put(33,17,0x3BD,True)
    for x,y in ((20,35),(21,24),(35,27),(45,35),(55,25),(54,17)):
        if t.g[y][x] & 1023 in (1,0x36D,0x36E,0x36F,0x395,0x39E,0x39F,0x3A0):
            t.put(x,y,0x3BE,True)
    for x,y in ((23,48),(38,28),(26,13),(49,12),(56,40),(5,17),(24,5)):
        if t.g[y][x] & 1023 == 1:
            t.put(x,y,0x0E0,True)
    # The last grassy pixel-island at this corner used to show as a bright
    # square under the bluff. Continue the sloping toe down to the water.
    t.put(19,10,0x345,True)
    t.put(19,11,0x349)
    fit_peat_to_cliff(t, land)
    return t


def west_marsh_trails(t):
    # The main boardwalk turns south over the flood channel to the trailhead
    # island. Deep peat separates it from the winch span nine tiles inland.
    footway(t, (0,22),(2,22),(2,28),(1,28),(1,35),(2,35),(2,40))
    footway(t, (0,19), (11,19))
    for y in (19,22):
        t.put(0,y,0x31D)
    footway(t, (4,19), (4,15))
    t.put(4,19,0x376)
    t.put(9,19,0x367,True)
    footway(t, (59,18), (63,18))
    t.put(63,18,0x371)


def lowlands(Terrain):
    t = Terrain("RainfallLowlands", 64, 42)
    land = banks(t, (
        # River-mouth bluff and the sheltered landing from the gorge.
        ((0,0),(18,0),(18,4),(15,7),(16,11),(13,14),(9,15),
         (6,13),(3,15),(1,12),(1,7),(0,5)),
        ((19,10),(24,8),(29,9),(31,12),(28,15),(24,16),(23,20),
         (20,20),(18,17)),
        ((6,21),(9,18),(14,18),(16,20),(15,23),(13,25),(9,25)),
        ((12,30),(18,28),(22,30),(25,29),(27,32),(26,35),(21,37),
         (17,36),(14,38),(10,35)),
        # Broad coastal bar between the reed delta and the open river mouth.
        ((34,0),(48,0),(48,4),(44,6),(47,9),(46,13),(43,14),(45,17),
         (42,20),(41,23),(44,26),(42,30),(40,33),(43,36),(43,41),
         (34,41),(33,37),(35,34),(32,31),(32,27),(34,23),(32,20),
         (34,17),(32,14),(34,11),(32,8),(35,6)),
        ((48,23),(50,20),(53,20),(54,22),(54,26),(55,29),(54,32),(55,36),(53,40),(52,40),(49,38),
         (48,34),(47,30),(49,27)),
        # Shingle and turf join the estuary trestle to Seabreak's coastal shelf.
        ((55,29),(55,26),(56,24),(57,22),(58,19),(59,16),(61,14),(63,13),
         (63,21),(61,22),(59,24),(58,26),(57,29)),
        # Outlying hags and low wooded banks frame the backwater.
        ((1,29),(4,28),(7,31),(5,35),(2,35)), ((19,1),(24,0),(28,2),(27,5),(23,6)),
        ((25,22),(29,21),(30,24),(28,26),(24,25)),
    ))
    peat(t, land)
    # East of the bar the final current opens into blue tidal water. There
    # is no row of peat/sea seams through the water; land divides the basins.
    tidal = {(x,y) for y in range(-1,t.h+1) for x in range(38,t.w+1)
             if (x,y) not in land and not (x == t.w and 13 <= y <= 21)}
    paint_river(t.g, tidal, elevation=3)
    for x,y in tidal:
        if not (0 < x < t.w-1 and 0 < y < t.h-1):
            continue
        if (x,y-1) in tidal and (x-1,y) not in tidal and (x+1,y) not in tidal:
            end = (x,y+1) not in tidal
            t.put(x,y,0x2D3 if end else 0x2CB,True)
            if end and t.g[y+1][x] & 1023 == 2:
                t.put(x,y+1,1)
    terrace(t, 1, 4, ((5,-1,3),(5,0,3),(4,-1,3),(3,-2,3)))
    terrace(t, 35, 8, ((4,0,3),(3,-1,4),(4,-3,3)))
    footway(t, (13,12), (21,12))
    footway(t, (20,18), (20,21), (14,21))
    footway(t, (11,23), (11,30), (16,30))
    footway(t, (24,32), (34,32))
    footway(t, (23,31), (23,25), (26,25))
    footway(t, (41,30), (48,30))
    footway(t, (38,19), (44,19), (44,17))
    reed_beds(t, (
        (6,17,6,3,0x36A),(18,7,5,4,0x36B),(17,26,4,4,0x369),
        (17,22,5,2,0x36C),(26,19,6,2,0x369),(5,28,4,4,0x36B),
        (30,13,4,4,0x36A),(30,34,4,4,0x36B),(23,39,6,3,0x36C),
        (4,37,4,3,0x369),(23,3,5,3,0x36A),
    ))
    t.marsh(30, [(16,19),(15,20),(15,20),(16,19)])
    footway(t, (2,9), (2,10), (8,10))
    footway(t, (49,32), (56,32), (56,34))
    # Breakers beyond the landing sit in the open estuary, far enough offshore
    # that the peat backing used west of the gorge cannot appear in this view.
    for x,y in ((54,15), (61,28), (59,38)):
        for dy in (0,1):
            for dx in (0,1):
                t.put(x+dx,y+dy,0x150+8*dy+dx,True)
    plant_trees(t.g, (
        (1,0),(4,0),(6,1),(9,0),(12,0),(21,2),(24,1),
        (3,11),(9,8),(11,12),(22,10),(25,10),(27,12),(20,15),
        (10,19),(12,20),(12,22),(12,31),(12,34),(21,33),(24,33),(16,34),
        (2,30),(4,31),(36,2),(36,0),(40,1),(44,2),(36,4),
        (37,12),(40,12),(34,15),(36,16),(38,21),(34,24),
        (38,24),(36,26),(34,28),(38,31),(35,35),(36,38),(40,38),
        (52,22),(51,25),(53,27),(49,34),(52,35),
    ))
    moss(t, (
        (11,11,6,4),(22,13,7,5),(11,21,5,4),(19,34,7,4),
        (38,4,5,3),(39,15,5,4),(38,26,6,4),(38,34,5,5),(52,29,6,7),
    ))
    meadow(t, ((7,12,4,2),(22,14,6,4),(9,22,4,2),(21,34,6,3),
               (35,18,3,4),(40,25,4,3),(38,34,4,3),(53,30,3,3)))
    t.put(50,31,0x3BE,True)
    t.put(5,9,0x3BE,True)
    # A timber trestle reaches Seabreak's landward quay. Tidal water shows
    # beneath its braces and south of the spit.
    footway(t, (0,18),(2,18),(2,10),(4,10))
    t.put(0,18,0x371)
    footway(t, (56,32),(56,27))
    for x in range(60,64):
        t.put(x,16,0x118 if x == 60 else 0x119)
        t.put(x,17,0x128 if x == 60 else 0x129)
    for x,y in ((58,23),(61,18),(62,15)):
        t.put(x,y,0x004)
    fit_peat_to_cliff(t, land)
    return t


def build_wetlands(terrain, root):
    pack_shore_corners(root)
    for build, left, right in ((lower, (19,22), (18,)), (lowlands, (18,), (16,))):
        t = build(terrain)
        if build == lower:
            west_marsh_trails(t)
        t.finish((), {"left": left, "right": right}, root=root)
        border = root / "data/layouts" / t.name / "border.bin"
        border.write_bytes(t.cell(0x367, True).to_bytes(2, "little") * 4)
