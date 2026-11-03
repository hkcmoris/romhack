"""Author the four Rainfall Pass layouts, their trail crossings and marsh bank."""

import json
import struct
from pathlib import Path

from PIL import Image

from native_terrain import paint_river, plant_trees

ROOT = Path(__file__).resolve().parents[2]
TRAIL_NOTCHES = ((0,), (1,), (2,), (3,), (0, 3), (1, 2))


def words(path):
    data = path.read_bytes()
    return list(struct.unpack("<%dH" % (len(data) // 2), data))


def write(path, values):
    path.write_bytes(struct.pack("<%dH" % len(values), *values))


def footprint(first_y, spans):
    """An outline drawn as inclusive horizontal spans, one per row."""
    return {(x, y) for y, (left, right) in enumerate(spans, first_y) for x in range(left, right + 1)}


class Terrain:
    def __init__(self, suffix, width, height, elevation=3):
        self.name = "Virelia_" + suffix
        self.w, self.h, self.e = width, height, elevation
        self.g = [[self.cell(1) for _ in range(width)] for _ in range(height)]
        self.path = set()

    def cell(self, tile, solid=False):
        return tile | self.e << 12 | (0x400 if solid else 0)

    def put(self, x, y, tile, solid=False):
        self.g[y][x] = self.cell(tile, solid)

    def trail(self, *points):
        """Trace a two-tile trail through the authored straight and diagonal bends."""
        for (x, y), (end_x, end_y) in zip(points, points[1:]):
            length = max(abs(end_x - x), abs(end_y - y))
            for step in range(length + 1):
                xx = x + round(step * (end_x - x) / length)
                yy = y + round(step * (end_y - y) / length)
                self.path.update((xx + dx, yy + dy) for dx in (0, 1) for dy in (0, 1)
                                 if 0 <= xx + dx < self.w and 0 <= yy + dy < self.h)

    def trails(self):
        for x, y in self.path:
            row = 0 if (x, y - 1) not in self.path else 2 if (x, y + 1) not in self.path else 1
            col = 0 if (x - 1, y) not in self.path else 2 if (x + 1, y) not in self.path else 1
            tile = 0x118 + 8 * row + col
            if tile == 0x121:
                notches = tuple(i for i, (dx, dy) in enumerate(((-1, -1), (1, -1), (-1, 1), (1, 1)))
                                if (x + dx, y + dy) not in self.path)
                if notches:
                    tile = 0x33A + TRAIL_NOTCHES.index(notches)
            self.put(x, y, tile)

    def grass(self, y, spans, long=False):
        for x, yy in footprint(y, spans):
            if self.g[yy][x] & 1023 != 1:
                raise ValueError(f"{self.name}: encounter grass covers another feature at {(x, yy)}")
            self.put(x, yy, 0x015 if long else 0x00D)

    def meadow(self, y, spans):
        patch = footprint(y, spans)
        for x, yy in patch:
            row = 0 if (x, yy - 1) not in patch else 2 if (x, yy + 1) not in patch else 1
            col = 0 if (x - 1, yy) not in patch else 2 if (x + 1, yy) not in patch else 1
            if self.g[yy][x] & 1023 != 1:
                raise ValueError(f"{self.name}: meadow covers another feature at {(x, yy)}")
            self.put(x, yy, 0x1D0 + 8 * row + col)

    def river(self, spans):
        # Emerald repeats a 2x2 border outside the map. Keep both the spring and
        # receiving pool inside the layout so the water has a shore on every side.
        wet = footprint(2, spans[2:-2])
        for y in (2, self.h - 3):
            left, right = spans[y]
            wet -= {(left, y), (right, y)}
        paint_river(self.g, wet, elevation=self.e, land_elevation=self.e)

    def marsh(self, y, spans):
        wet = footprint(y, spans)
        for x, yy in wet:
            north, south, west, east = [(x + dx, yy + dy) in wet for dx, dy in
                                        ((0, -1), (0, 1), (-1, 0), (1, 0))]
            row = 0 if not north else 2 if not south else 1
            col = 0 if not west else 2 if not east else 1
            tile = 0x32D + 3 * row + col
            if all((north, south, west, east)):
                missing = [i for i, (dx, dy) in enumerate(((-1, -1), (1, -1), (-1, 1), (1, 1)))
                           if (x + dx, yy + dy) not in wet]
                if len(missing) > 1:
                    raise ValueError(f"Marsh bend too narrow at {(x, yy)}")
                if missing:
                    tile = 0x336 + missing[0]
            self.put(x, yy, tile)

    def ledge(self, start_x, top_y, run_lengths, stairs=()):
        """Trace a grassy terrace face. Each (length, offset) locates its crest."""
        profile = [top_y + offset for length, offset in run_lengths for _ in range(length)]
        # Rim above stone, shaded grass foot below. At an offset the two columns
        # share their stone face, with an exposed wall turned toward the low side.
        for col, top in enumerate(profile):
            x = start_x + col
            west = profile[col - 1] if col else top + 3
            east = profile[col + 1] if col + 1 < len(profile) else top + 3
            rim = 0x20C if west > top else 0x20D if east > top else 0x087
            face = 0x203 if west > top + 1 else 0x202 if east > top + 1 else 0x0A9
            foot = 0x078 if west < top or col == 0 else 0x07A if east < top or col == len(profile)-1 else 0x079
            for dy, tile in enumerate((rim, face, foot)):
                self.put(x, top + dy, tile, True)
        for x in stairs:
            top = profile[x - start_x]
            for y in range(top, top + 3):
                self.put(x, y, 0x0AF)
                self.put(x + 1, y, 0x0CF)

    def outcrop(self, x, y, width):
        # These are the four consecutive silhouettes of a rock shelf in General.
        for dy, (left, middle, right) in enumerate(((0x068, 0x069, 0x06A),
                                                    (0x070, 0x071, 0x072),
                                                    (0x07B, 0x07C, 0x07D),
                                                    (0x078, 0x079, 0x07A))):
            for dx in range(width):
                self.put(x + dx, y + dy, left if dx == 0 else right if dx == width-1 else middle, True)

    def bridge(self, x, y, width):
        for dx in range(-1, width + 1):
            if dx == -1:
                top, bottom = 0x328, 0x329
            elif dx == width:
                top, bottom = 0x32A, 0x32B
            elif dx == 0:
                top, bottom = 0x324, 0x325
            elif dx == width - 1:
                top, bottom = 0x326, 0x327
            else:
                top, bottom = 0x320, 0x321
            self.put(x + dx, y, top)
            self.put(x + dx, y + 1, bottom)

    def finish(self, trees):
        plant_trees(self.g, trees, elevation=self.e)
        directions = {
            "Virelia_RainfallFoothills": (0x323, 0x322, 0x32C),
            "Virelia_RainfallUpper": (0x323, 0x322),
            "Virelia_RainfallLower": (0x323, 0x322),
            "Virelia_RainfallLowlands": (0x322, 0x322, 0x323),
        }
        doc = json.loads((ROOT / "data/maps" / self.name / "map.json").read_text(encoding="utf-8"))
        for obj in doc["object_events"]:
            if self.g[obj["y"]][obj["x"]] & 0xC00:
                raise ValueError(f"{self.name}: terrain blocks {obj['script']}")
        for warp, tile in zip(doc["warp_events"], directions[self.name], strict=True):
            self.put(warp["x"], warp["y"], tile)

        folder = ROOT / "data/layouts" / self.name
        write(folder / "map.bin", [c for row in self.g for c in row])
        write(folder / "border.bin", [self.cell(1, True)] * 4)

    def pass_mouth(self, x, y, width):
        """Northbound passage through a stock cliff, with the arrow warp at its mouth."""
        left = x - width // 2
        for dy, row in enumerate(((0x068, 0x069, 0x06A), (0x07B, 0x07C, 0x07D),
                                  (0x078, 0x079, 0x07A))):
            for col in range(width):
                self.put(left + col, y - 2 + dy, row[0 if col == 0 else 2 if col == width-1 else 1], True)
        self.put(x, y - 1, 0x09F, True)
        self.put(x, y, 0x0A7)

    def descent(self, x, y, width):
        left = x - width // 2
        self.ledge(left, y - 1, [(width, 0)])
        for yy in (y - 1, y, y + 1):
            self.put(x, yy, 0x340)


def bridges():
    bank = ROOT / "data/tilesets/secondary/virelia_rainfall"
    src = ROOT / "data/tilesets/secondary/fortree"
    metas, attrs = words(bank / "metatiles.bin"), words(bank / "metatile_attributes.bin")
    original = words(src / "metatiles.bin")
    need = 0x32D - 512
    metas.extend([0] * max(0, need * 8 - len(metas)))
    attrs.extend([0] * max(0, need - len(attrs)))
    for target, source in [(0x320, 0x210), (0x321, 0x218), (0x324, 0x216), (0x325, 0x21E),
                           (0x326, 0x217), (0x327, 0x21F), (0x328, 0x211), (0x329, 0x219),
                           (0x32A, 0x215), (0x32B, 0x21D)]:
        metas[(target - 512)*8:(target - 511)*8] = original[(source - 512)*8:(source - 511)*8]
        attrs[target - 512] = 0x1000
    primary = words(ROOT / "data/tilesets/primary/general/metatiles.bin")
    metas[(0x322 - 512)*8:(0x323 - 512)*8] = primary[0x0A7*8:0x0A8*8]
    metas[(0x32C - 512)*8:(0x32D - 512)*8] = primary[0x121*8:0x122*8]
    attrs[0x32C - 512] = 0x62
    write(bank / "metatiles.bin", metas)
    write(bank / "metatile_attributes.bin", attrs)


def trail_corners():
    bank = ROOT / "data/tilesets/secondary/virelia_rainfall"
    image = Image.open(bank / "tiles.png").copy()
    # An inward grass tip occupies just the corner of one quarter-tile, in
    # General's trail palette. Flip this one tile for all four orientations.
    notch = Image.new("P", (8, 8), 12)
    for y, row in enumerate(((15, 15, 14, 13), (15, 14, 13), (14, 13), (13,))):
        for x, color in enumerate(row):
            notch.putpixel((x, y), color)
    graphic = 0x2FC
    image.paste(notch, ((graphic % 16) * 8, (graphic - 512) // 16 * 8))
    image.save(bank / "tiles.png", bits=4)
    metas, attrs = words(bank / "metatiles.bin"), words(bank / "metatile_attributes.bin")
    need = 0x341 - 512
    metas.extend([0] * max(0, need * 8 - len(metas)))
    attrs.extend([0] * max(0, need - len(attrs)))
    primary = words(ROOT / "data/tilesets/primary/general/metatiles.bin")
    for i, notches in enumerate(TRAIL_NOTCHES, 0x33A - 512):
        pieces = primary[0x121 * 8:0x122 * 8]
        for q in notches:
            pieces[q] = 0x5000 | (q << 10) | graphic
        metas[i*8:(i+1)*8] = pieces
        attrs[i] = 0
    # A one-lane rock stair needs the outer cheek from each side of the
    # two-tile General stair. Its middle row is the existing south arrow warp.
    stair = [primary[(0x0AF if q % 2 == 0 else 0x0CF) * 8 + q] for q in range(8)]
    for tile in (0x323, 0x340):
        metas[(tile - 512)*8:(tile - 511)*8] = stair
    attrs[0x340 - 512] = 0
    write(bank / "metatiles.bin", metas)
    write(bank / "metatile_attributes.bin", attrs)


def foothills():
    t = Terrain("RainfallFoothills", 32, 28)
    # The junction sits below the upper staircase. A returning hiker spots the
    # lower turn across the clearing; the shortcut has its own short spur.
    t.trail((14, 26), (14, 23), (11, 21), (8, 20), (7, 18), (7, 14),
            (9, 12), (9, 10), (7, 8), (7, 3))
    t.trail((9, 12), (13, 12), (16, 13), (20, 13), (23, 16),
            (25, 18), (28, 18))
    t.trail((24, 17), (24, 20), (25, 22), (25, 23))
    # The east arrow leads directly under the tree line beyond the fork.
    t.path.difference_update(((29, 18), (29, 19)))
    t.trails()
    t.ledge(0, 7, [(4, -1), (9, 0), (3, 1), (7, 2), (4, 1), (5, 0)], stairs=(7,))
    t.outcrop(13, 16, 6)
    t.outcrop(23, 0, 5)
    t.grass(13, [(3, 5), (2, 5), (2, 5), (3, 4), (3, 4)])
    t.grass(3, [(15, 18), (14, 19), (14, 19), (16, 18)])
    t.meadow(4, [(24, 27), (23, 27), (23, 26), (24, 25)])
    t.meadow(19, [(20, 22), (18, 22), (16, 18), (15, 16)])
    t.grass(20, [(5, 6), (5, 6), (6, 7)])
    t.put(10, 10, 3, True)
    for x, y in ((22, 5), (27, 6), (19, 19), (5, 18)):
        t.put(x, y, 0x004)
    t.pass_mouth(7, 3, 5)
    t.descent(14, 26, 5)
    for x, y in ((11, 6), (12, 16), (20, 18), (26, 12), (28, 17), (29, 20), (10, 26), (4, 11)):
        t.put(x, y, 0x0E0, True)
    return t


def upper():
    t = Terrain("RainfallUpper", 32, 34, 5)
    t.trail((5, 31), (5, 29), (7, 27), (9, 26), (9, 23), (8, 22),
            (8, 19), (10, 18), (21, 18), (22, 17), (21, 15), (21, 12),
            (23, 10), (25, 8), (25, 6), (23, 4), (23, 3))
    t.trail((8, 19), (7, 17), (7, 14), (9, 12), (9, 10), (10, 9))
    t.trail((20, 18), (20, 23), (22, 26), (24, 28))
    t.trails()
    # Two grassy benches frame the gorge. The river cuts their faces cleanly;
    # its three flowing center columns continue through the lip and plunge pool.
    t.ledge(0, 7, [(4, -2), (8, -1), (6, 2), (6, 3), (8, 2)], stairs=(24,))
    t.ledge(18, 24, [(6, 0), (5, 1), (3, 2)], stairs=(20,))
    t.river([(12, 17)]*3 + [(13, 17)]*11 + [(13, 18)]*3 + [(12, 18)]*5
            + [(12, 17)]*3 + [(11, 17)]*3 + [(11, 16)]*3 + [(12, 16)]*3)
    for dy, row in enumerate(((0x0BB, 0x0B3, 0x0BC), (0x0B9, 0x0B9, 0x0B9),
                              (0x02C, 0x034, 0x03C))):
        for dx, tile in enumerate(row):
            t.put(14 + dx, 9 + dy, tile, True)
    t.bridge(12, 18, 7)
    t.pass_mouth(23, 3, 5)
    t.descent(5, 31, 5)
    t.grass(21, [(3, 4), (2, 5), (2, 5), (3, 4)])
    t.grass(13, [(26, 28), (25, 29), (26, 29), (27, 28)], long=True)
    t.meadow(15, [(9, 11), (9, 11), (10, 11)])
    t.meadow(20, [(22, 24), (22, 24), (23, 25), (23, 25)])
    t.grass(28, [(26, 27), (26, 27), (26, 27)])
    t.put(10, 9, 3, True)
    for x, y in ((6, 10), (11, 12), (10, 22), (11, 27), (19, 13), (24, 13), (25, 30)):
        t.put(x, y, 0x0E0, True)
    return t


def lower():
    t = Terrain("RainfallLower", 40, 40)
    t.trail((4, 36), (4, 35), (7, 32), (10, 31), (12, 28), (12, 26),
            (10, 24), (9, 22), (10, 20), (12, 19), (16, 19), (18, 20),
            (27, 20), (30, 18), (30, 14), (32, 12), (32, 10), (29, 8),
            (24, 8), (22, 7), (15, 7), (12, 6), (10, 4), (6, 4), (4, 3), (4, 2))
    t.trail((12, 28), (16, 28), (17, 26))
    t.trails()
    # The wetland has two crossings. The basalt bench joins the western bank
    # between them, so the far-bank walk is still the longer exploration route.
    t.ledge(0, 13, [(6, 0), (3, -1), (6, -2), (5, -1), (4, 0)])
    t.river([(18, 22)]*3 + [(17, 22)]*3 + [(17, 21)]*4 + [(18, 22)]*2
            + [(19, 24)]*3 + [(20, 25)]*3 + [(21, 25)]*5 + [(20, 25)]*2
            + [(19, 24)]*3 + [(19, 23)]*3 + [(20, 24)]*2 + [(21, 26)]*3
            + [(22, 27)]*4)
    t.bridge(17, 7, 5)
    t.bridge(21, 20, 5)
    t.pass_mouth(4, 2, 5)
    t.descent(4, 36, 5)
    t.marsh(23, [(2, 4), (2, 6), (2, 6), (3, 8), (3, 8), (5, 8), (5, 7)])
    t.marsh(29, [(14, 16), (13, 17), (13, 17), (14, 18), (14, 18), (12, 17), (12, 17), (12, 15)])
    t.marsh(24, [(30, 34), (30, 35), (29, 35), (29, 33), (31, 33)])
    t.grass(16, [(3, 6), (3, 7), (2, 7), (2, 5), (3, 5)], long=True)
    t.grass(23, [(12, 15), (13, 16), (14, 17), (15, 16)], long=True)
    t.grass(32, [(28, 30), (28, 31), (28, 31), (29, 31), (29, 30)], long=True)
    t.grass(10, [(24, 25), (24, 27), (25, 28), (26, 28)], long=True)
    t.grass(4, [(12, 14), (13, 15)], long=True)
    t.meadow(5, [(31, 33), (30, 33), (31, 34)])
    t.meadow(29, [(8, 10), (7, 9), (7, 8)])
    t.meadow(29, [(24, 26), (24, 26), (25, 26)])
    t.grass(2, [(7, 8), (7, 9)])
    t.grass(22, [(29, 32), (30, 32)])
    t.put(16, 22, 0x0C5, True)
    for x, y in ((1, 11), (8, 15), (15, 10), (33, 20), (11, 35), (28, 24), (16, 38)):
        t.put(x, y, 0x0E0, True)
    return t


def lowlands():
    t = Terrain("RainfallLowlands", 32, 26)
    t.trail((4, 3), (4, 5), (6, 7), (9, 7), (12, 9), (15, 9), (15, 12))
    t.trail((25, 3), (25, 4), (24, 6), (21, 7), (19, 9), (15, 9))
    t.trail((15, 14), (15, 17), (17, 19), (19, 19), (22, 20), (24, 20))
    t.trail((17, 19), (16, 21), (15, 22), (15, 23))
    t.trails()
    t.ledge(0, 12, [(4, 1), (5, 0), (4, -1), (7, 0), (4, 1), (4, 2), (4, 1)])
    # The callback replaces this specific throat with path after the rival battle.
    for y in (12, 13, 14):
        t.put(15, y, 0x121)
    t.put(15, 13, 0x00E, True)
    t.put(16, 12, 1)
    t.pass_mouth(4, 3, 5)
    t.pass_mouth(25, 3, 5)
    t.descent(15, 23, 5)
    t.marsh(16, [(5, 7), (4, 8), (4, 8), (6, 10), (6, 10), (7, 9)])
    t.grass(16, [(21, 23), (21, 26), (22, 27), (24, 27)])
    t.grass(4, [(14, 16), (13, 16), (14, 15)])
    t.meadow(4, [(6, 8), (6, 9), (7, 9)])
    t.meadow(7, [(16, 19), (16, 18)])
    t.grass(16, [(9, 10), (10, 11), (10, 11)])
    for x, y in ((9, 3), (11, 10), (13, 16), (12, 21), (21, 22), (28, 9)):
        t.put(x, y, 0x0E0, True)
    return t


# Crown positions are individually composed around landmarks and switchbacks.
# An aligned pair two rows apart shares the rear tree's base.
GROVES = {
    "Virelia_RainfallFoothills": [
        (1, 0), (1, 2), (3, 1), (3, 3), (11, 0), (10, 3), (18, 0), (21, 1), (29, 0), (30, 4),
        (1, 9), (0, 13), (6, 10), (28, 11), (30, 12), (28, 13), (29, 17), (26, 14),
        (1, 18), (3, 20), (1, 20), (0, 23), (3, 22), (5, 23), (3, 25), (8, 24),
        (19, 21), (19, 23), (17, 24), (17, 22), (21, 21), (22, 25), (28, 22), (30, 25),
    ],
    "Virelia_RainfallUpper": [
        (1, 0), (1, 2), (3, 1), (5, 0), (5, 2), (8, 0), (7, 3), (10, 3),
        (18, 1), (20, 4), (27, 0), (29, 2), (28, 5),
        (0, 11), (0, 13), (2, 13), (2, 15), (0, 16), (4, 17), (3, 10), (4, 14),
        (28, 18), (30, 17), (26, 21), (28, 22),
        (0, 25), (2, 26), (0, 27), (0, 29), (0, 31), (8, 31),
        (18, 28), (20, 30), (17, 31), (28, 30), (30, 29), (25, 31),
    ],
    "Virelia_RainfallLower": [
        (0, 1), (1, 5), (4, 8), (7, 7), (9, 0), (12, 1), (15, 0),
        (24, 1), (26, 3), (26, 5), (29, 2), (33, 0), (34, 4), (37, 2),
        (36, 8), (34, 10), (37, 12), (35, 14), (32, 15),
        (0, 17), (0, 20), (10, 14), (12, 14), (12, 16), (14, 15), (16, 15), (18, 16),
        (36, 18), (34, 21), (37, 22), (37, 24), (27, 16),
        (0, 28), (2, 30), (5, 30), (0, 33), (0, 35), (0, 37), (8, 36),
        (10, 37), (18, 36), (27, 29), (32, 29), (35, 28), (38, 31),
        (33, 34), (35, 35), (31, 37), (37, 37),
    ],
    "Virelia_RainfallLowlands": [
        (0, 0), (0, 2), (2, 5), (0, 7), (7, 0), (10, 0), (11, 3), (16, 0), (18, 2),
        (21, 1), (28, 0), (30, 2), (28, 5), (2, 9), (24, 10), (30, 9),
        (1, 16), (0, 20), (3, 22), (5, 23), (10, 22),
        (17, 15), (28, 17), (30, 19), (26, 21), (26, 23), (22, 23), (18, 23),
    ],
}


def build():
    from pack_rainfall_marsh import pack_marsh

    bridges()
    pack_marsh(ROOT)
    trail_corners()
    for make_route in (foothills, upper, lower, lowlands):
        terrain = make_route()
        terrain.finish(GROVES[terrain.name])


if __name__ == "__main__":
    build()
