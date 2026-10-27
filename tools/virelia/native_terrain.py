"""Small stock-General terrain assemblers; no graphics or registry mutation.

Coordinates in ``cells`` include water's edge cells, not just its open center.
Helpers return every painted coordinate so a later forest pass can protect it.
All artwork is existing Emerald General; callers own topology/event safeguards.
"""


def _word(tile, elevation, blocked):
    return tile | (elevation << 12) | (0x400 if blocked else 0)


def _inside(grid, point):
    x, y = point
    return 0 <= y < len(grid) and 0 <= x < len(grid[y])


def _paint(grid, point, tile, elevation, blocked, preserve, painted):
    if point in preserve or not _inside(grid, point):
        return
    x, y = point
    grid[y][x] = _word(tile, elevation, blocked)
    painted.add(point)


def paint_river(
    grid, cells, *, elevation=1, blocked=True, preserve=(), south_bank=True, land_elevation=3
):
    """Paint real native ocean-water art (170), with rock/grass shore contours.

    ``cells`` includes the 188/189/18A north caps and 190/192 side-bank cells.
    It may contain preserved bridges: these remain in the wet mask for joins,
    but their metatiles are not overwritten. An optional stock 002 south-facing
    grass edge is painted only over ordinary passable 001/002 grass.
    """
    wet, preserve, painted = set(cells), set(preserve), set()
    for x, y in sorted(wet, key=lambda p: (p[1], p[0])):
        north, east = (x, y - 1) in wet, (x + 1, y) in wet
        south, west = (x, y + 1) in wet, (x - 1, y) in wet
        if not north:
            tile = 0x188 if not west else 0x18A if not east else 0x189
        elif not west:
            tile = 0x179 if not south else 0x190
        elif not east:
            tile = 0x178 if not south else 0x192
        else:
            # These native corner shadows join an inward north-side bank step.
            nw, ne = (x - 1, y - 1) in wet, (x + 1, y - 1) in wet
            tile = 0x199 if not nw and not ne else 0x198 if not nw else 0x19A if not ne else 0x170
        _paint(grid, (x, y), tile, elevation, blocked, preserve, painted)
        bank = (x, y + 1)
        if south_bank and not south and bank not in wet and _inside(grid, bank):
            bx, by = bank
            old = grid[by][bx]
            if old & 1023 in (0x001, 0x002) and not old & 0xC00:
                _paint(grid, bank, 0x002, land_elevation, False, preserve, painted)
    return painted


def paint_pond(grid, cells, **kwargs):
    """Real water with the same native bank family, for a nonrectangular pool."""
    return paint_river(grid, cells, **kwargs)


def paint_puddle(grid, cells, *, elevation=3, blocked=False, preserve=()):
    """Small marsh puddles only: C8..DA are MB_PUDDLE, not deep river water."""
    wet, preserve, painted = set(cells), set(preserve), set()
    for x, y in sorted(wet, key=lambda p: (p[1], p[0])):
        n, e, s, w = ((x, y - 1) in wet, (x + 1, y) in wet, (x, y + 1) in wet, (x - 1, y) in wet)
        if not n:
            tile = 0x0C8 if not w else 0x0CA if not e else 0x0C9
        elif not s:
            tile = 0x0D8 if not w else 0x0DA if not e else 0x0D9
        else:
            tile = 0x0D0 if not w else 0x0D2 if not e else 0x0D1
        _paint(grid, (x, y), tile, elevation, blocked, preserve, painted)
    return painted


def paint_rock_outcrop(grid, cells, *, elevation=0, preserve=()):
    """Solid native cliff face with real rim/side/foot pieces, never 071 fill.

    A stepped/rounded mask gives a more natural outcrop than a large rectangle.
    Native Route120's exposed rim is 074/07C/089; 075/073 are sides, 0A9 is
    the interior face and 078/079/07A form the grass-facing foot.
    """
    rock, preserve, painted = set(cells), set(preserve), set()
    for x, y in sorted(rock, key=lambda p: (p[1], p[0])):
        n, e, s, w = (
            (x, y - 1) in rock,
            (x + 1, y) in rock,
            (x, y + 1) in rock,
            (x - 1, y) in rock,
        )
        if not n:
            tile = 0x074 if not w else 0x089 if not e else 0x07C
        elif not s:
            tile = 0x078 if not w else 0x07A if not e else 0x079
        elif not w:
            tile = 0x075
        elif not e:
            tile = 0x073
        else:
            tile = 0x0A9
        _paint(grid, (x, y), tile, elevation, True, preserve, painted)
    return painted


def paint_cliff_band(grid, x, y, width, height, *, elevation=0, preserve=()):
    """Convenience rectangular native cliff band; height/width at least three."""
    if width < 3 or height < 3:
        raise ValueError("A native cliff band needs at least a 3x3 footprint")
    cells = {(xx, yy) for yy in range(y, y + height) for xx in range(x, x + width)}
    return paint_rock_outcrop(grid, cells, elevation=elevation, preserve=preserve)
