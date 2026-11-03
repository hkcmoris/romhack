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


def _cut_pixel(x, y, section, phase, cap):
    """Three vertical courses of quarried shale, with shear joints and bedding."""
    xx = x + 16 * phase
    yy = 16 * section + y
    if cap:
        dx = x if cap == 'W' else 15 - x
        bevel = max(0, 4 - yy, yy - 43)
        if dx < 1 + bevel:
            return 15
        if dx == 1 + bevel:
            return 14
        if dx == 2 + bevel:
            return 10 if cap == 'W' else 13
    if yy < 5:
        return _bed_pixel(x, y, 0)
    dip = (0, 0, 1, 1, 1, 2, 2, 1, 0, 0, -1, -1, -1, 0, 0, 0)[xx // 4]
    depth = yy - dip
    if depth < 7:
        return 10
    if depth < 9:
        return 9 if xx % 19 > 3 else 11
    if yy >= 45:
        return 14 if yy < 47 else 13
    # Wide saw-cut planes are broken by shear joints, offset from one bed
    # to the next. Keep the lowest seam in shade to anchor the whole bench.
    for bottom, seam in ((19, 5), (26, 45), (39, 21), (47, 58)):
        if depth < bottom:
            joint = (xx - seam + (bottom - depth) // 3) % 64
            if joint == 0 or joint == 1 and depth > bottom - 4:
                return 14
            if depth == bottom - 1:
                return 14 if joint < 24 else 13
            if depth == bottom - 2:
                return 13 if joint < 32 else 12
            if depth == bottom - 3:
                return 10 if 8 < joint < 22 else 12
            if joint < (bottom - depth) // 2:
                return 13
            if bottom == 19 and joint > 20 or bottom == 39 and 7 < joint < 24:
                return 11
            return 12 if bottom < 47 else 13
    return 14


def pack_walls(add, graphic):
    def register(name, sample):
        entries = []
        for ox, oy in ((0, 0), (8, 0), (0, 8), (8, 8)):
            tile = (sample(x + ox, y + oy) for y in range(8) for x in range(8))
            entries.append(0x6000 | graphic(tile))
        return add(name, entries + [0] * 4)

    for variant in range(8):
        register(f'ROCK_BED_{variant}', lambda x, y: _rock_pixel(x, y, 0, variant))
    for mask in sorted({normalize(bits) for bits in range(1, 256)}):
        register(f'ROCK_EDGE_{mask:02X}', lambda x, y: _rock_pixel(x, y, mask, 0))
    # Phase and end-cap combinations depend on the length and position of a
    # bench. Allocate the ones that appear in the layout, including under props.
    @lru_cache(None)
    def cut_tile(section, phase, cap):
        return register(f'ROCK_CUT_{section}_{phase}{cap}',
                        lambda x, y: _cut_pixel(x, y, section, phase, cap))

    return cut_tile


def wall_markers(ids):
    return {ids[f'NATIVE_{code:03X}'] for code in (0x70, 0x71, 0x72, 0x78, 0x79, 0x7A)}


def place_walls(grids, ids, cut_faces, origins, cut_tile):
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
                key = f'ROCK_EDGE_{mask:02X}' if mask else f'ROCK_BED_{variant}'
                row[x] = (block & ~1023) | ids[key]

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
