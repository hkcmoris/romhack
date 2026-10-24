"""Eight-neighbor cliff contours, independent of image tooling."""

# Cardinal floor neighbors, then diagonals clockwise from northwest.
NEIGHBORS = ((0, -1), (1, 0), (0, 1), (-1, 0),
             (-1, -1), (1, -1), (1, 1), (-1, 1))


def normalize(mask):
    # A diagonal only adds an inset corner when both adjacent sides are rock.
    for diagonal, sides in ((16, 1 | 8), (32, 1 | 2), (64, 2 | 4), (128, 4 | 8)):
        if mask & sides:
            mask &= ~diagonal
    return mask


# Native Rusturf straight edges, convex caps and concave corners.
CLIFF_TILES = {0: 0x211, 1: 0x209, 2: 0x212, 4: 0x219, 8: 0x210,
               1 | 8: 0x208, 1 | 2: 0x20a, 4 | 8: 0x218, 4 | 2: 0x21a,
               16: 0x220, 32: 0x222, 64: 0x21b, 128: 0x21c}
for index, pattern in enumerate(sorted({normalize(mask) for mask in range(256)} - CLIFF_TILES.keys()), 0x280):
    CLIFF_TILES[pattern] = index


def cliff_at(floor, x, y):
    height, width = len(floor), len(floor[0])
    mask = sum(1 << bit for bit, (dx, dy) in enumerate(NEIGHBORS)
               if 0 <= x + dx < width and 0 <= y + dy < height and floor[y + dy][x + dx])
    return CLIFF_TILES[normalize(mask)]


def quadrants(mask):
    """Native 8px quarters for compound corners and one-cell rock ridges."""
    return tuple(both if mask & a and mask & b else edge_a if mask & a
                 else edge_b if mask & b else inset if mask & diagonal else 0x211
                 for a, b, diagonal, both, edge_a, edge_b, inset in (
                     (1, 8, 16, 0x208, 0x209, 0x210, 0x220),
                     (1, 2, 32, 0x20a, 0x209, 0x212, 0x222),
                     (4, 8, 128, 0x218, 0x219, 0x210, 0x21c),
                     (4, 2, 64, 0x21a, 0x219, 0x212, 0x21b)))
