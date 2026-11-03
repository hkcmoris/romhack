"""Cut slate contours for the connected outdoor quarry, in native palette 6."""

from functools import lru_cache

try:
    from .quarry_contours import NEIGHBORS, normalize
except ImportError:
    from quarry_contours import NEIGHBORS, normalize


# Horizontal cleavage follows the bedding; short diagonal fractures cut it.
# These 32px strata swatches cross the 16px cell seams.
BEDDING = (
    '..............................::',
    '...........................:::-.',
    '.........................::-....',
    '.........................-......',
    '................................',
    '................................',
    '................................',
    '................................',
    '................................',
    '................................',
    '................................',
    '................................',
    '..................:.............',
    '.................:-.............',
    '..............:::-..............',
    '.............:--................',
)

OUTCROP = (
    '                     :::-   ',
    '                  ::::---   ',
    '               ++++::---    ',
    '           +++++:::----     ',
    '         :++::::::-----     ',
    '       +++++:::::---==      ',
    '     :++::::::-----==-      ',
    '    :+::::------=====    :- ',
    '   :::::---======--  ::::-- ',
    '   --====-----  :++++::---  ',
    '   ==--     :+++++:::---    ',
    '        :++++::::-----      ',
    '      +++:::::----==        ',
    '    :::-----=====--         ',
    '   ---====-----             ',
    '   =-----                   ',
)

# Rows are a cut face viewed from above: sunlit lip, shallow shelf, alternating
# shadow bands and a dark undercut. Horizontal repeating edges share strata.
FACE = (
    '***+++**********',
    '+++:::++++++:+++',
    '..::...........:',
    '::----..:::-----',
    '--=--:::----===-',
    '...::---..::---.',
    '---===---===----',
    '===----=====--==',
)
INKS = {'.': 12, ':': 11, '-': 13, '=': 14, '+': 10, '*': 9}

# Deliberately sparse landmarks in the unworked spoil. Each pair is the
# upper-left cell of a continuous two-by-two outcrop in the world frame.
OUTCROPS = ((4, 0), (21, 33), (6, 38), (33, 40), (32, 51),
            (47, 38), (65, 39), (29, 65), (73, 69))


def _bed_pixel(x, y, variant):
    width = len(BEDDING[0])
    base = BEDDING[(y + 7 * (variant // 2)) % 16][(x + 16 * (variant % 2)) % width]
    if 4 <= variant < 8:
        # A pair of tilted, exfoliating stone plates spans up to four cells.
        fx, fy = x + 16 * (variant % 2) - 2, y + 16 * ((variant - 4) // 2) - 8
        if 0 <= fy < len(OUTCROP) and 0 <= fx < len(OUTCROP[fy]):
            mark = OUTCROP[fy][fx]
            if mark != ' ':
                base = mark
    return INKS[base]


def _rock_pixel(x, y, mask, variant):
    """Bevel a convex tip or scoop an inset where only the diagonal is open."""
    left = x < 8
    dx = x if left else 15 - x
    side = bool(mask & (8 if left else 2))
    north, south = mask & 1, mask & 4
    notch_n = mask & (16 if left else 32)
    notch_s = mask & (128 if left else 64)
    bevel = max(0, 4 - dx) if side else 0

    # Top and toe are separate contours. The six-to-eight pixel exposure on
    # the south edge reads as a vertical face. Side returns meet it at 45°.
    top = 1 + bevel if north else max(0, 2 - dx) if notch_n else 0
    toe = 14 - bevel if south else 15 - max(0, 2 - dx) if notch_s else 16
    if y < top or y > toe or (side and dx == 0):
        return 15  # Quarry dust matches the floor bank's base color.
    if south:
        lip = toe - 7
    elif notch_s:
        lip = 7 + dx * 3
    else:
        lip = 17
    if y >= lip:
        if y == toe or side and dx == 1:
            return 14
        return INKS[FACE[min(y - lip, 7)][x]]
    if top and y == top:
        return 13
    if top and y == top + 1:
        return 10
    if side and dx <= 2:
        return (13 if dx == 1 else 10) if left else (14 if dx == 1 else 13)
    if notch_n and y < 4 - dx:
        return 11
    return _bed_pixel(x, y, variant)


def _cap_pixel(x, y, section, cap):
    dx = x if cap == 'W' else 15 - x
    yy = 16 * section + y
    bevel = max(0, 4 - yy, yy - 43)
    if dx < 1 + bevel:
        return 15
    if dx == 1 + bevel:
        return 14
    if dx == 2 + bevel:
        return 10 if cap == 'W' else 13
    return 0


# The return exposes a nearly full tile of shaded side plane when a wall
# turns upscreen. Its inner rim fades into the surface in the last 4px.
SIDE_FACE = (
    '8edddddccccb0000', '8edddddccccb0000', '8eddddcccccb0000', '8eddddcccccb0000',
    '8eddddcccccb0000', '8eddddccceeb0000', '8edddceeeddc0000', '8edeeeddddcb0000',
)


def pack_walls(add, graphic, bank, face_pixels):
    # The exposed bench receives warm, chalky daylight. Its shaded vertical
    # face still uses the same indexed art and the colder slot 6 ramp.
    colors = (bank / 'palettes/06.pal').read_text().splitlines()
    colors[4:19] = ['96 96 88', '120 128 112', '160 160 136', '184 184 152', '208 208 176',
                    '128 96 72', '200 160 104', '56 64 72', '240 224 184', '224 216 184',
                    '208 200 168', '192 184 160', '152 144 128', '96 96 96', '136 136 128']
    (bank / 'palettes/12.pal').write_text('\n'.join(colors) + '\n', newline='\n')
    def quarters(sample):
        entries = []
        for ox, oy in ((0, 0), (8, 0), (0, 8), (8, 8)):
            tile = (sample(x + ox, y + oy) for y in range(8) for x in range(8))
            entry = graphic(tile)
            entries.append(0x6000 | entry if entry else 0)
        return entries

    for variant in range(8):
        add(f'ROCK_BED_{variant}', quarters(lambda x, y: _rock_pixel(x, y, 0, variant)) + [0]*4)
    @lru_cache(None)
    def edge_tile(mask):
        return add(f'ROCK_EDGE_{mask:02X}', quarters(lambda x, y: _rock_pixel(x, y, mask, 0)) + [0]*4)
    # Phase and end-cap combinations depend on the length and position of a
    # bench. Allocate the ones that appear in the layout, including under props.
    @lru_cache(None)
    def cut_tile(section, phase, cap):
        # The same edge bevel fits every phase of a stratum. Pack it once on
        # the upper layer so each face can share the untrimmed stone below.
        face = quarters(lambda x, y: face_pixels[section*16+y][phase*16+x])
        edge = quarters(lambda x, y: _cap_pixel(x, y, section, cap)) if cap else [0]*4
        return add(f'ROCK_CUT_{section}_{phase}{cap}', face + edge)

    side_graphics = quarters(lambda x,y: int(SIDE_FACE[y % 8][x], 16))

    @lru_cache(None)
    def bench_tile(course, phase, side, level):
        if course == 'toe' or course == 'lip' and not level:
            return cut_tile(0 if course == 'lip' else 2, phase, side)
        if course == 'lip':
            # The first 8px include the top surface and its sunlit arris.
            # Join that strip to the higher landing; the next quarter of the
            # same swatch immediately falls into the shadow of the cut.
            face = quarters(lambda x,y: face_pixels[y][phase*16+x])
            face[:2] = [(word & 0xfff) | 0xc000 for word in face[:2]]
            edge = quarters(lambda x,y: _cap_pixel(x,y,0,side)) if side else [0]*4
            return add(f'ROCK_BENCH_LIP_{phase}{side}', face + edge)
        # Three distinct top planes: deep blue-grey spoil, buff weathered
        # shale, then a pale sunlit crown. The risers all expose dark slate.
        mask = (1 if course == 'rear' else 0)
        base = quarters(lambda x, y: _rock_pixel(x,y,mask,phase))
        palette = (6,11,12)[level]
        base = [(word & 0xfff) | (palette << 12) for word in base]
        overlay = [0] * 4
        if side:
            overlay = ([side_graphics[q ^ 1] ^ 0x400 for q in range(4)]
                       if side == 'E' else side_graphics)
        return add(f'ROCK_BENCH_{course.upper()}_{level}_{phase}{side}', base + overlay)
    return cut_tile, bench_tile, edge_tile


def wall_markers(ids):
    return {ids[f'NATIVE_{code:03X}'] for code in (0x70, 0x71, 0x72, 0x78, 0x79, 0x7A)}


def place_walls(grids, ids, cut_faces, origins, tile_factories, protected=()):
    """Replace geometry markers, sampling neighbors across the four sections."""
    markers = wall_markers(ids)
    cells, walls = set(), set()
    for name, rows in grids.items():
        ox, oy = origins[name]
        for y, row in enumerate(rows):
            for x, block in enumerate(row):
                point = (ox + x, oy + y)
                cells.add(point)
                if block & 1023 in markers:
                    walls.add(point)

    openings = cells - walls
    cut_tile, bench_tile, edge_tile = tile_factories
    outcrop_cells = {}
    for bx, by in OUTCROPS:
        if all((bx + dx, by + dy) not in openings
               for dy in range(-1, 3) for dx in range(-1, 3)):
            for dy in range(2):
                for dx in range(2):
                    outcrop_cells[bx + dx, by + dy] = 4 + dx + 2 * dy
    for name, rows in grids.items():
        ox, oy = origins[name]
        for y, row in enumerate(rows):
            for x, block in enumerate(row):
                gx, gy = ox + x, oy + y
                if (gx, gy) not in walls:
                    continue
                mask = normalize(sum(1 << bit for bit, (dx, dy) in enumerate(NEIGHBORS)
                                     if (gx + dx, gy + dy) in openings))
                variant = outcrop_cells.get((gx, gy), (gx % 2) + 2 * (gy % 2))
                index = edge_tile(mask) if mask else ids[f'ROCK_BED_{variant}']
                row[x] = (block & ~1023) | index

    # Work each wide stone mass back from its toe. The lowest berm is broad
    # enough for a two-cell vertical exposure; each successive lift leaves
    # an actual tread before the next wall. The shape spans map connections.
    # Stop at the native border, where Porymap repeats the 2x2 border block.
    def inset(points, shoulder, south):
        offsets = [(dx,dy) for dy in range(-1,south+1) for dx in range(-shoulder,shoulder+1)
                   if abs(dx) < shoulder or dy < south-1]
        return {p for p in points if all((p[0]+dx,p[1]+dy) in points for dx,dy in offsets)}

    def platform(points, width, depth):
        # Each patch has at least one full row of tread behind its two face
        # courses. A finger too small to carry the tread stays on the lower
        # level rather than turning into a freestanding fragment of wall.
        offsets = [(dx,dy) for dy in range(depth) for dx in range(width)]
        return {(x+dx,y+dy) for x,y in points
                if all((x+tx,y+ty) in points for tx,ty in offsets) for dx,dy in offsets}

    reserve = {(n, x+dx, y+dy) for n,x,y,w,h in cut_faces for dy in range(h) for dx in range(w)}
    keep = {(origins[n][0]+x,origins[n][1]+y) for n,x,y in reserve}
    # A sign or a stone-mounted fixture at the rim has its own graphic, but
    # the rock behind it does not acquire a one-cell bite in every tier.
    backed = set()
    for name,rows in grids.items():
        ox,oy = origins[name]
        for y,row in enumerate(rows):
            for x,block in enumerate(row):
                p = ox+x,oy+y
                if block & 0xc00 and p not in walls and sum(
                    (p[0]+dx,p[1]+dy) in walls for dx,dy in NEIGHBORS[:4]
                ) >= 3:
                    backed.add(p)
    mass = walls | backed
    middle = platform(inset(mass - keep, 2, 4), 3, 4)
    upper = platform(inset(middle, 2, 3), 3, 3)
    # Complete the entire lowest riser where its column has room; it must
    # not stop halfway along a toe merely because the upper ledge recedes.
    base = platform(mass - keep, 3, 3) & walls
    reserve.update(protected)
    for name, rows in grids.items():
        ox,oy = origins[name]
        for y,row in enumerate(rows):
            for x,block in enumerate(row):
                gx,gy = ox+x,oy+y
                point = (gx,gy)
                if point not in base or (name,x,y) in reserve:
                    continue
                level = 2 if point in upper else 1 if point in middle else 0
                plane = (mass,middle,upper)[level]
                def edge(dx,dy):
                    test = (gx+dx,gy+dy)
                    return test not in plane and (level > 0 or test in cells)
                if edge(0,1):
                    course = 'toe'
                elif edge(0,2):
                    course = 'lip'
                elif edge(0,-1):
                    course = 'rear'
                else:
                    course = 'top'
                # A side return starts at the neighbouring bench's *lip*,
                # two rows ahead of its toe. Otherwise staggered front runs
                # appear suspended over seemingly continuous flat bedrock.
                lookahead = 0 if course == 'toe' else 1 if course == 'lip' else 2
                side = 'W' if edge(-1,lookahead) else 'E' if edge(1,lookahead) else ''
                if not level and course in ('top','rear') and not side:
                    continue
                phase = gx % 4 if course in ('lip','toe') else (0 if side or course=='rear' else gx % 2 + 2*(gy % 2))
                tone = min(1,level) if course in ('lip','toe') else level
                row[x] = (block & ~1023) | bench_tile(course,phase,side,tone)

    for name, x, y, width, height in cut_faces:
        ox, oy = origins[name]
        for dy in range(height):
            section = 0 if dy == 0 else 2 if dy == height - 1 else 1
            for dx in range(width):
                gx, gy = ox + x + dx, oy + y + dy
                if (gx, gy) not in walls:
                    continue
                cap = ('W' if (gx - 1, gy) in openings else
                       'E' if (gx + 1, gy) in openings else '')
                block = grids[name][y + dy][x + dx]
                grids[name][y + dy][x + dx] = (block & ~1023) | cut_tile(section, gx % 4, cap)
