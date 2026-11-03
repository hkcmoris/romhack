"""Native indexed ground, quarry-floor details and haul-road transitions."""

# Shared by all four maps. Props still use their original slot 7 as the upper layer.
PALETTE = (
    (0, 0, 0), (72, 88, 96), (104, 120, 128), (128, 144, 144),
    (152, 160, 152), (168, 176, 168), (184, 184, 168), (208, 208, 184),
    (144, 136, 120), (168, 160, 144), (184, 176, 152), (200, 192, 168),
    (104, 136, 136), (144, 168, 160), (128, 144, 112), (160, 168, 128),
)


def write_surface_palette(bank):
    rows = [' '.join(map(str, rgb)) for rgb in PALETTE]
    (bank / 'palettes/08.pal').write_text('JASC-PAL\n0100\n16\n' + '\n'.join(rows) + '\n', newline='\n')


def surface_quarters(image, quarters):
    return [(word & 0xfff) | 0x8000 for word in quarters(image)]


def flat(color, width=16, height=16):
    return [[color] * width for _ in range(height)]


def stamp(image, x, y, rows):
    for dy, row in enumerate(rows):
        for dx, pixel in enumerate(row):
            if pixel != '.':
                image[y + dy][x + dx] = int(pixel, 16)


# Small planar shapes rather than isolated noise. The long faces follow the
# same shallow diagonal as the outcrops and catch light on their upper edge.
DETAILS = {
    'SPLIT': [(2, 7, ('........44', '....44443.', '..4433....', '43........')),
              (4, 11, ('6666',))],
    'SHINGLE': [(6, 4, ('..6666..', '.744443.', '...433..')),
                (1, 10, ('.66', '44.'))],
    'FLAKES': [(1, 3, ('..6', '.43')), (9, 11, ('.666', '443.'))],
    'FINES': [(4, 6, ('.66666', '...444')), (12, 10, ('64',))],
    'BALLAST': [(3, 3, ('..767.', '.64332', '..433.')),
                (9, 10, ('.676.', '64432', '..33.')), (0, 13, ('64',))],
    'SCREE': [(8, 1, ('..6767', '.64332', '...43.')),
              (1, 7, ('.766', '6433')), (6, 13, ('.66', '43.'))],
}


def detailed(kind):
    image = flat(5)
    for x, y, rows in DETAILS[kind]:
        stamp(image, x, y, rows)
    return image


def base_surface(kind):
    if kind == 'ROUGH':
        return detailed('BALLAST')
    return flat(10 if kind == 'ROAD' else 5)


# Two-metatile details, placed as complete units. Long saw marks and wet
# laminations should continue through the middle of a tile instead of ending
# at each 16-pixel boundary.
FEATURES = {
    'CUT': (2, 4, (
        '...............666666666....',
        '........666666644444433.....',
        '..6666664444444.............',
        '.444444.....................',
        '......................66666.',
        '...........666666666664443..',
        '....6666666444444...........',
        '...44444....................',
    )),
    'VEIN': (1, 5, (
        '......................666677.',
        '.................66666777643.',
        '.............6667733333......',
        '.........666774433...66......',
        '....666674333......6644......',
        '..674433.......6644..........',
        '.433.........................',
    )),
    'SEEP': (1, 4, (
        '...............dddddddd......',
        '..........ddddddddddccccdd...',
        '.....dddddddccccccccdd......d',
        '..ddddddccc3ccccddd..........',
        '.ddddcc3ccccdd....dd.........',
        '...dd3cccdd...ddddddddd......',
        '......dd..dddddcccdd.........',
        '..........ddcccddd...........',
    )),
    'LICHEN': (0, 4, (
        '................f......ee......',
        '............ff.fee..feee4......',
        '.........fffeee44..ee44........',
        '....ee..eee444.........ff......',
        '...fe4.44.......ff...fee.......',
        '.fee4..........ffe44...........',
        '..............ee4..............',
    )),
}

# Coordinates address the upper-left corner of each 32x16 unit. Keep
# deliberate working surfaces around the derrick, water and abandoned siding.
FEATURE_SITES = {
    'Entrance': {'CUT': [(4, 6), (14, 5), (15, 17), (20, 18), (23, 20), (7, 20), (31, 24)],
                 'VEIN': [(3, 15), (25, 7)]},
    'Terraces': {'VEIN': [(9, 4), (17, 6), (24, 4), (33, 7), (16, 19), (33, 16), (23, 29), (14, 34), (33, 33)],
                 'CUT': [(20, 13), (26, 20), (30, 30)]},
    'Depths': {'SEEP': [(28, 5), (30, 6), (24, 9), (24, 15), (26, 19), (29, 21), (33, 21), (18, 14), (17, 18), (4, 19)],
               'VEIN': [(5, 28), (30, 27)]},
    'Gallery': {'LICHEN': [(4, 11), (11, 11), (20, 14), (6, 20), (8, 21), (17, 23), (10, 27), (3, 33), (16, 35), (20, 36)],
                'VEIN': [(15, 19), (21, 26)]},
}

# Cardinal then diagonal bits, clockwise from north / north-east.
NEIGHBORS = ((0,-1), (1,0), (0,1), (-1,0), (1,-1), (1,1), (-1,1), (-1,-1))
CORNER_BITS = ((1,8,128), (1,2,16), (4,8,64), (4,2,32))


def road_mask(mask):
    # Keep a diagonal only when its two cardinal neighbours also join.
    return (mask & 15) | sum(d for a,b,d in CORNER_BITS if mask & a and mask & b and mask & d)


# These are the four long, single-edge stretches most often visible together.
STRAIGHT_EDGES = {0x6e, 0x9b, 0x37, 0xcd}


def road_image(mask, alternate=False):
    image = flat(10)
    for q, (vertical, horizontal, diagonal) in enumerate(CORNER_BITS):
        for y in range(8):
            for x in range(8):
                # Distances from the two exterior edges of this quadrant.
                dx = 7 - x if q & 1 else x
                dy = 7 - y if q & 2 else y
                # A chipped shoulder, with a wider radius on a convex turn.
                # The adjoining tile shares the same edge profile at its end.
                distance = 9
                profile = (1,2,2,3,3,4,3,2) if alternate else (1,1,2,1,2,2,1,2)
                if not mask & vertical:
                    distance = min(distance, dy - profile[dx])
                if not mask & horizontal:
                    distance = min(distance, dx - profile[dy])
                if not (mask & vertical or mask & horizontal):
                    distance = min(distance, dx + dy - 7)
                if mask & vertical and mask & horizontal and not mask & diagonal:
                    distance = min(distance, dx + dy - 2)
                color = 5 if distance < 0 else 6 if distance == 0 else 9 if distance == 1 else 10
                image[(q // 2) * 8 + y][(q % 2) * 8 + x] = color
    # A broken wheel groove inboard of the shoulder. On vertical roads
    # transpose the marks to follow the cart's travel direction.
    if mask in STRAIGHT_EDGES:
        horizontal = mask & 5 != 5
        if alternate:
            marks = [(0, 8, '999'), (1, 9, 'bb'), (7, 7, '999999'), (9, 8, 'bbbb'), (10, 12, '9a9')]
        else:
            marks = [(0, 7, '9'), (5, 8, '9999'), (6, 9, 'bb'), (14, 8, '99'), (2, 4, '9')]
        flip = bool(mask & (1 if horizontal else 8))
        for along, across, stroke in marks:
            across = 15 - across if flip else across
            for i, color in enumerate(stroke):
                x,y = (along+i,across) if horizontal else (across,along+i)
                image[y][x] = int(color,16)
    return image


def pack_surfaces(add, quarters):
    def tile(name, image, behavior=0x1000):
        add(name, surface_quarters(image, quarters) + [0]*4, behavior)

    for kind in ('SPLIT', 'SHINGLE', 'FLAKES', 'FINES', 'SCREE'):
        tile(kind, detailed(kind), 0x1008 if kind == 'SCREE' else 0x1000)
    for kind, (x, y, rows) in FEATURES.items():
        image = flat(5, width=32)
        stamp(image, x, y, rows)
        for side in range(2):
            tile(f'{kind}_{side}', [row[side*16:side*16+16] for row in image])
    masks = sorted({road_mask(m) for m in range(256)} - {255})
    for mask in masks:
        tile(f'ROAD_{mask:02X}', road_image(mask))
        if mask in STRAIGHT_EDGES:
            tile(f'ROAD_{mask:02X}_B', road_image(mask, alternate=True))
    for name, rows in (
        ('LONG', ('...bbbbbb....', '.99999.......', '...........bb', '.........99..')),
        ('SHORT', ('....bb.', '..999..', '.......', '......b')),
    ):
        image = flat(10)
        stamp(image, 1, 6, rows)
        tile('TRACK_' + name, image)
        tile('TRACK_' + name + '_V', [list(row) for row in zip(*image)])


def pack_work_floors(add, quarters, bank):
    # Mortared, chipped slabs for the saw bed. A second pair of tiles follows
    # the length of the sun-bleached ties bolted beneath the derrick feet.
    colors = [(0,0,0), (96,104,104), (120,136,136), (144,152,144),
              (168,176,168), (192,192,176), (136,144,144), (128,136,128),
              (88,80,72), (112,96,80), (136,120,96), (160,144,112),
              (176,160,128), (192,176,144), (104,104,96), (208,200,168)]
    (bank / 'palettes/11.pal').write_text('JASC-PAL\n0100\n16\n' +
        ''.join(f'{r} {g} {b}\n' for r, g, b in colors), newline='\n')
    slabs = flat(3, width=32)
    boards = flat(11, width=32)
    for y in range(16):
        for x in range(32):
            # Offset the cross joints on each course. Thin contact shadows
            # and chipped lips keep the slab surfaces distinct at 1x.
            joint = (x - (7 if y < 9 else 24)) % 32
            if y in (0,9) or joint == 0:
                slabs[y][x] = 2
            elif y in (1,10) or joint == 1:
                slabs[y][x] = 5 if joint > 4 else 3
            elif y in (8,15) or joint == 31:
                slabs[y][x] = 6
            else:
                slabs[y][x] = 4 if y < 9 and joint > 9 else 3
            course = y % 8
            seam = (x + (16 if y < 8 else 0)) % 32
            if course == 7 or seam == 0:
                boards[y][x] = 9
            elif course == 0:
                boards[y][x] = 13 if seam > 1 else 10
            elif course == 6 or seam == 31:
                boards[y][x] = 10
            elif seam in (2,29) and course == 3:
                boards[y][x] = 8
    stamp(slabs, 14, 5, ('..55.', '56...'))
    stamp(slabs, 2, 12, ('...66', '66...'))
    stamp(boards, 10, 3, ('...aaa', 'aac...'))
    stamp(boards, 20, 12, ('..ccc', 'aa...'))
    for prefix, image in (('SLAB', slabs), ('DECK', boards)):
        for half in range(2):
            art = [row[half*16:half*16+16] for row in image]
            entries = [0xb000 | (word & 0xfff) for word in quarters(art)]
            add(f'{prefix}_{half}', entries + [0]*4)


def grain(x, y):
    # Stable integer mix: changing one section never shuffles all the others.
    h = (x * 0x1f1f1f1f ^ y * 0x6c8e9cf5) & 0xffffffff
    h = ((h ^ (h >> 16)) * 0x45d9f3b) & 0xffffffff
    return h ^ (h >> 16)


def dress_surfaces(grids, ids, origins):
    roads = set()
    for name, grid in grids.items():
        ox, oy = origins[name]
        roads.update((x+ox, y+oy) for y,row in enumerate(grid) for x,v in enumerate(row)
                     if v & 1023 == ids['ROAD'])

    for name, grid in grids.items():
        section = name.split('Quarry')[-1]
        ox, oy = origins[name]
        for y, row in enumerate(grid):
            for x, word in enumerate(row):
                index = word & 1023
                gx, gy = ox + x, oy + y
                choice = grain(gx, gy)
                if index == ids['ROAD']:
                    mask = road_mask(sum(1 << n for n,(dx,dy) in enumerate(NEIGHBORS)
                                         if (gx+dx, gy+dy) in roads))
                    key = 'ROAD' if mask == 255 else f'ROAD_{mask:02X}'
                    if mask in STRAIGHT_EDGES and (choice >> 2) % 3:
                        key += '_B'
                    if mask == 255 and choice % 5 < 2:
                        key = 'TRACK_LONG' if choice & 1 else 'TRACK_SHORT'
                        if (gx+2, gy) not in roads or (gx-2, gy) not in roads:
                            key += '_V'
                elif index == ids['GROUND']:
                    # Fines collect in irregular patches, so sparse chips and
                    # fractures have breathing room between the worked areas.
                    pocket = grain(gx//4, gy//3) % 4
                    kinds = ('SPLIT', 'SHINGLE', 'FLAKES', 'FINES')
                    key = kinds[(choice >> 8) % 4] if choice % 10 < 2 + pocket else 'GROUND'
                elif index == ids['ROUGH']:
                    key = 'ROUGH' if choice & 1 else 'SCREE'
                else:
                    continue
                row[x] = (word & ~1023) | ids[key]

        bare = {ids[k] for k in ('GROUND', 'SPLIT', 'SHINGLE', 'FLAKES', 'FINES')}
        for kind, sites in FEATURE_SITES[section].items():
            for x, y in sites:
                cells = grid[y][x:x+2]
                if len(cells) == 2 and all(v & 1023 in bare for v in cells):
                    grid[y][x:x+2] = [(v & ~1023) | ids[f'{kind}_{i}'] for i,v in enumerate(cells)]
