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

    def trails(self, *, stone=False):
        for x, y in self.path:
            row = 0 if (x, y - 1) not in self.path else 2 if (x, y + 1) not in self.path else 1
            col = 0 if (x - 1, y) not in self.path else 2 if (x + 1, y) not in self.path else 1
            tile = 0x3AE + 3 * row + col if stone else 0x118 + 8 * row + col
            if row == col == 1:
                notches = tuple(i for i, (dx, dy) in enumerate(((-1, -1), (1, -1), (-1, 1), (1, 1)))
                                if (x + dx, y + dy) not in self.path)
                if notches:
                    tile = (0x3B7 if stone else 0x33A) + TRAIL_NOTCHES.index(notches)
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

    def finish(self, trees, directions, *, root=None, border=1):
        plant_trees(self.g, trees)
        root = ROOT if root is None else root
        doc = json.loads((root / "data/maps" / self.name / "map.json").read_text(encoding="utf-8"))
        for obj in doc["object_events"]:
            if self.g[obj["y"]][obj["x"]] & 0xC00:
                raise ValueError(f"{self.name}: terrain blocks {obj['script']}")
        for warp, tile in zip(doc["warp_events"], directions, strict=True):
            x, y = warp["x"], warp["y"]
            self.g[y][x] = (self.g[y][x] & 0xF000) | tile

        folder = root / "data/layouts" / self.name
        write(folder / "map.bin", [c for row in self.g for c in row])
        write(folder / "border.bin", [self.cell(border, True)] * 4)

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


def profile(points):
    """Interpolate the hand-drawn crest, toe or shoreline between its surveyed bends."""
    result = {}
    for (start, a), (end, b) in zip(points, points[1:]):
        for index in range(start, end + 1):
            result[index] = a + round((b - a) * (index - start) / (end - start))
    return result


def escarpment(t, crest, toe, *, stairs=(), moss=(), fractures=(), inset=0):
    """Draw the visible vertical surface between two independently shaped contours."""
    tops, feet = profile(crest), profile(toe)
    first, last = crest[0][0] + inset, crest[-1][0] - inset
    for x in range(first, last + 1):
        top = tops[x]
        bottom = feet[x]
        for y in range(top, bottom + 1):
            west = x > first and tops[x - 1] <= y <= feet[x - 1]
            east = x < last and tops[x + 1] <= y <= feet[x + 1]
            edge = 0 if not west else 2 if not east else 1
            tile = (0x341 if y == top else 0x347 if y == bottom else 0x344) + edge
            t.put(x, y, tile, True)
    for x, length in moss:
        if not first <= x <= last:
            continue
        for y in range(tops[x] + 1, min(tops[x] + length + 1, feet[x])):
            t.put(x, y, 0x34B, True)
    for x, y in fractures:
        t.put(x, y, 0x34A, True)
    for x, width in stairs:
        for dx in range(width):
            for y in range(tops[x + dx], feet[x + dx] + 1):
                tile = 0x340 if width == 1 else (0x0AF if dx == 0 else 0x0CF)
                t.put(x + dx, y, tile)
    return tops, feet


def channel(t, *courses):
    """The spring flares into plunge pools and contracts again at each fall."""
    water = set()
    for sections in courses:
        left = profile([(y, x) for y, x, _ in sections])
        right = profile([(y, x) for y, _, x in sections])
        water.update((x, y) for y in left for x in range(left[y], right[y] + 1))
    paint_river(t.g, water, elevation=t.e, land_elevation=t.e)


def cascade(t, x, lip, plunge, width=3):
    for y in range(lip, plunge + 1):
        first = 0x34C if y == lip else 0x352 if y == plunge else 0x34F
        for dx in range(width):
            section = 0 if dx == 0 else 2 if dx == width - 1 else 1
            t.put(x + dx, y, first + section, True)


def hanging_shelf(t, x, y, spans):
    """A grassy step partway down a face, with a shaded back and a lower rock lip."""
    ledge = footprint(y, [(x + a, x + b) for a, b in spans])
    for xx, yy in ledge:
        north, south = (xx, yy - 1) in ledge, (xx, yy + 1) in ledge
        side = 0 if (xx - 1, yy) not in ledge else 2 if (xx + 1, yy) not in ledge else 1
        if not north:
            t.put(xx, yy, 0x347 + side, True)
        elif not south:
            t.put(xx, yy, 0x341 + side, True)
        else:
            t.put(xx, yy, 1)


def buttresses(t, anchors):
    # Each group of four is one continuous, two-storey fracture in the slate.
    for x, y in anchors:
        for dy in range(2):
            for dx in range(2):
                if (t.g[y+dy][x+dx] & 1023) in (0x345, 0x34A, 0x34B):
                    t.put(x+dx, y+dy, 0x3A1 + dx + 2*dy, True)


def cloudline(t):
    """The four shoulders of the summit disappear into the same cloud bank."""
    west = profile([(0, 3), (5, 4), (12, 2), (20, 1), (28, 3), (34, 2), (41, 1), (47, 3)])
    east = profile([(0, 60), (10, 62), (17, 62), (25, 60), (32, 62), (40, 61), (47, 60)])
    north = profile([(0, 2), (6, 3), (13, 2), (19, 1), (24, 1), (30, 2),
                     (38, 1), (47, 0), (54, 2), (60, 3), (63, 2)])
    south = profile([(0, 44), (7, 46), (13, 45), (22, 46), (28, 45), (34, 44),
                     (40, 46), (47, 46), (53, 44), (58, 45), (63, 44)])
    for y, row in enumerate(t.g):
        for x, block in enumerate(row):
            distances = (south[x] - y, y - north[x], x - west[y], east[y] - x)
            margin = min(distances)
            if margin <= 0:
                material = block & 1023
                rock = 0x341 <= material <= 0x34B or 0x3A1 <= material <= 0x3A4
                tile = 0x3A5 if margin < 0 else 0x3A6 + 2 * distances.index(0) + int(rock)
                if margin == 0 and distances[0] == 0 and 0x34C <= material <= 0x354:
                    tile = 0x3BF
                t.put(x, y, tile, True)


def suspension_span(t, x, end, y, falls_x, elevation, falls_width=3):
    for xx in range(x, end + 1):
        if falls_x <= xx < falls_x + falls_width:
            section = 0 if xx == falls_x else 2 if xx == falls_x + falls_width - 1 else 1
            back, front = 0x35B + section, 0x35E + section
        else:
            section = 0 if xx == x else 2 if xx == end else 1
            back, front = 0x355 + section, 0x358 + section
        t.g[y][xx] = elevation << 12 | back
        t.g[y + 1][xx] = elevation << 12 | front


def uplift(t, crest, stairs, toe):
    """Northern benches share one height. Level-zero steps join their lower landing."""
    for x, top in crest.items():
        for y in range(top):
            t.g[y][x] = 0x7000 | (t.g[y][x] & 0xFFF)
    for x in stairs:
        for y in range(crest[x], toe[x] + 1):
            t.g[y][x] &= 0xFFF


def lookout(t, x, y):
    for dy in range(2):
        for dx in range(3):
            t.put(x + dx, y + dy, 0x361 + dy * 3 + dx, True)


def foothills():
    t = Terrain("RainfallFoothills", 50, 42)
    # The apron above Slateford narrows to a single rock stair below the fork.
    t.trail((14, 38), (14, 36), (10, 34), (10, 32), (14, 30), (14, 25),
            (16, 23), (18, 22), (20, 20))
    t.trail((19, 21), (23, 18), (27, 18), (28, 16), (28, 11),
            (33, 10), (35, 8), (38, 8), (38, 6))
    t.trail((18, 22), (23, 21), (25, 22), (29, 22), (31, 23),
            (39, 23), (43, 21), (46, 22))
    t.trail((28, 10), (24, 9), (21, 9))
    t.trails()
    channel(t, [(6, 35, 36), (7, 33, 37), (9, 32, 37), (11, 33, 37),
                (15, 33, 37), (20, 33, 37), (21, 32, 39), (23, 32, 39),
                (25, 33, 38), (26, 33, 37), (29, 33, 37), (33, 33, 37),
                (34, 32, 38), (35, 31, 39), (37, 30, 39), (38, 31, 37), (39, 32, 35)])
    escarpment(t,
        [(0, 6), (6, 5), (12, 7), (15, 6), (20, 4), (25, 4), (29, 3),
         (31, 2), (37, 2), (42, 3), (46, 5), (49, 4)],
        [(0, 9), (5, 8), (10, 11), (15, 9), (20, 6), (25, 7), (30, 4),
         (36, 4), (40, 6), (43, 6), (47, 9), (49, 8)],
        inset=2, moss=[(3, 1), (11, 2), (17, 1), (24, 2), (44, 2)])
    escarpment(t,
        [(0, 17), (5, 18), (10, 16), (14, 12), (18, 11), (22, 12),
         (25, 14), (28, 13), (29, 13), (32, 12), (34, 13), (36, 13),
         (39, 12), (43, 15), (46, 16), (49, 14)],
        [(0, 21), (4, 22), (10, 20), (14, 17), (19, 15), (23, 16), (27, 17),
         (29, 17), (33, 19), (37, 19), (40, 17), (45, 20), (49, 19)],
        inset=2, stairs=[(28, 2)], moss=[(2, 3), (8, 1), (14, 2), (23, 1), (30, 2), (39, 2), (44, 2)],
        fractures=[(5, 21), (17, 14), (26, 15), (40, 14), (46, 18)])
    cascade(t, 34, 13, 19)
    t.bridge(32, 22, 8)
    t.bridge(32, 9, 6)
    escarpment(t,
        [(0, 27), (5, 26), (9, 24), (13, 25), (15, 25), (19, 26),
         (24, 28), (28, 27), (31, 28), (36, 29), (40, 26), (44, 27), (49, 25)],
        [(0, 31), (4, 30), (8, 29), (13, 29), (15, 29), (19, 31),
         (24, 34), (29, 32), (33, 34), (37, 33), (42, 31), (47, 32), (49, 30)],
        inset=2, stairs=[(14, 1)], moss=[(1, 2), (6, 1), (16, 3), (21, 4), (25, 2), (35, 2), (41, 3), (48, 1)],
        fractures=[(8, 27), (17, 28), (29, 30), (39, 30), (46, 29)])
    t.put(14, 28, 0x00E, True)
    t.put(13, 29, 0x003, True)
    t.put(18, 21, 0x003, True)
    for y in range(4, 7):
        t.put(38, y, 0x340)
    escarpment(t, [(9,39),(11,38),(13,37),(15,37),(17,38),(20,39)],
               [(9,40),(11,40),(13,39),(15,39),(17,40),(20,40)], stairs=[(14, 1)])
    lookout(t, 21, 7)
    cascade(t, 34, 29, 34)
    hanging_shelf(t, 17, 27, [(0, 3), (0, 4), (1, 4)])
    hanging_shelf(t, 42, 27, [(1, 4), (0, 4), (0, 3)])
    escarpment(t, [(1,37),(3,36),(5,37)], [(1,38),(3,39),(5,38)])
    buttresses(t, [(1, 29), (5, 19), (5, 28), (8, 18), (10, 26), (19, 13),
                   (23, 29), (26, 30), (31, 16), (38, 29), (43, 17), (45, 17)])
    t.grass(32, [(3, 5), (2, 6), (2, 6), (3, 4)])
    t.grass(21, [(10, 12), (9, 12), (10, 12)])
    t.grass(9, [(42, 44), (41, 45), (41, 44)])
    t.grass(33, [(42, 46), (41, 46), (41, 44), (42, 43)])
    for x, y in ((11, 36), (10, 30), (12, 30), (16, 32), (14, 24), (16, 21),
                 (19, 20), (26, 20), (29, 9), (31, 25), (40, 21), (44, 20),
                 (21, 33), (6, 37), (39, 34), (18, 25)):
        t.put(x, y, 0x36F)
    for x, y in ((13, 34), (14, 33), (17, 20), (24, 20), (25, 11), (42, 24), (40, 35)):
        t.put(x, y, 0x004)
    return t


def upper():
    t = Terrain("RainfallUpper", 64, 48, 5)
    # Follow the first hanging bench, climb the central spur, then cross a
    # second tributary above the first. Both high banks terminate at real falls.
    t.trail((6, 40), (7, 38), (7, 32), (9, 29), (12, 28),
            (13, 25), (15, 24), (29, 24), (31, 23), (32, 21), (32, 16),
            (33, 13), (35, 12), (51, 12), (54, 13), (56, 15), (61, 14))
    t.trail((34, 13), (34, 10), (32, 8), (29, 8))
    t.trail((32, 8), (34, 7), (34, 5))
    t.trail((25, 24), (27, 22), (28, 21))
    t.trails(stone=True)
    channel(t, [(2, 20, 21), (3, 19, 22), (4, 18, 22), (6, 18, 22),
                (13, 18, 22), (15, 16, 23), (17, 17, 23), (19, 18, 22),
                (29, 18, 22), (31, 16, 24), (32, 15, 25), (33, 15, 24),
                (35, 18, 23), (37, 20, 24), (39, 23, 27), (41, 26, 31),
                (42, 28, 35), (43, 30, 43), (46, 33, 42)],
               [(1, 43, 45), (3, 42, 48), (5, 41, 49), (8, 42, 48),
                (17, 42, 48), (19, 41, 49), (20, 39, 47), (21, 39, 45),
                (22, 40, 46), (23, 41, 47), (25, 43, 47), (27, 43, 47), (34, 43, 47),
                (36, 42, 49), (37, 40, 49), (39, 39, 47), (41, 37, 44),
                (42, 36, 43), (43, 34, 42), (46, 33, 42)])
    upper_rim, upper_toe = escarpment(t,
        [(0, 11), (5, 13), (10, 12), (14, 11), (17, 9), (18, 8), (22, 8),
         (24, 12), (27, 14), (28, 14), (31, 17), (34, 17), (37, 16), (39, 16),
         (40, 15), (42, 10), (43, 8), (47, 8), (49, 15), (50, 16), (53, 18),
         (56, 18), (59, 20), (63, 18)],
        [(0, 16), (5, 18), (9, 17), (13, 18), (17, 15), (18, 14), (22, 14),
         (24, 16), (28, 19), (31, 21), (34, 21), (37, 23), (40, 21),
         (43, 18), (47, 18), (49, 21), (52, 23), (56, 25), (60, 26), (63, 24)],
        stairs=[(32, 2)], moss=[(1, 2), (6, 2), (12, 3), (17, 1), (23, 3),
         (26, 2), (34, 3), (36, 2), (39, 3), (49, 2), (54, 3), (60, 4)],
        fractures=[(3, 15), (11, 15), (16, 14), (24, 14), (30, 19), (38, 19),
                   (50, 19), (52, 22), (59, 22), (62, 21)])
    escarpment(t,
        [(0, 34), (4, 32), (7, 30), (8, 30), (12, 31), (14, 29), (16, 27),
         (18, 20), (22, 20), (24, 25), (26, 29), (30, 29), (34, 31), (37, 30),
         (40, 28), (43, 27), (47, 27), (50, 29), (55, 31), (58, 29), (63, 31)],
        [(0, 39), (4, 38), (7, 35), (8, 35), (11, 37), (14, 36), (17, 31),
         (18, 30), (22, 30), (24, 32), (27, 35), (31, 36), (36, 37), (40, 35),
         (43, 34), (47, 34), (51, 36), (55, 38), (60, 35), (63, 36)],
        stairs=[(7, 2)], moss=[(2, 2), (5, 3), (11, 3), (13, 1), (16, 3),
         (24, 3), (27, 2), (31, 2), (35, 3), (40, 2), (48, 3), (55, 4), (61, 3)],
        fractures=[(3, 36), (10, 34), (14, 33), (25, 31), (29, 32),
                   (37, 34), (49, 33), (52, 34), (59, 32)])
    # The spillway can be seen again much farther down the windward slope.
    cascade(t, 19, 8, 14)
    cascade(t, 19, 20, 30)
    cascade(t, 43, 8, 18, 5)
    cascade(t, 44, 27, 34)
    # The lookout projects over the lower drainage. Small hanging benches and
    # broad jointed buttresses interrupt the sheer faces between its two falls.
    hanging_shelf(t, 2, 13, [(1, 4), (0, 5), (0, 4), (1, 3)])
    hanging_shelf(t, 8, 31, [(2, 4), (1, 5), (1, 4), (2, 3)])
    hanging_shelf(t, 27, 30, [(2, 5), (0, 5), (0, 6), (1, 4)])
    hanging_shelf(t, 53, 19, [(1, 4), (0, 6), (0, 6), (2, 5)])
    hanging_shelf(t, 48, 30, [(1, 3), (0, 6), (1, 4), (2, 3)])
    buttresses(t, [(2, 35), (4, 33), (10, 13), (12, 32), (14, 13), (15, 29),
                   (24, 14), (24, 28), (26, 32), (29, 16), (31, 34), (35, 18),
                   (35, 34), (38, 19), (39, 31), (49, 17), (52, 20), (54, 34),
                   (57, 31), (59, 21), (62, 33)])
    escarpment(t, [(0,46),(7,45),(10,44),(15,45),(19,44),(25,45),(32,43),
                   (37,44),(42,45),(47,44),(54,42),(58,43),(63,42)],
                  [(0,47),(63,47)], moss=[(33,1),(54,2)])
    cascade(t, 37, 44, 47)
    # A second rock spine closes the spring basins from behind. Only the
    # weather-station bench is accessible from the upper landing.
    escarpment(t, [(0,0),(63,0)],
        [(0,5),(5,7),(9,5),(13,5),(17,3),(18,2),(23,2),(26,4),(29,4),
         (33,2),(37,3),(41,2),(48,1),(51,3),(56,6),(60,7),(63,6)],
        moss=[(5,3),(10,2),(27,2),(52,2),(59,3)])
    escarpment(t, [(0,40),(3,38),(5,39),(7,39),(10,41),(12,40)],
               [(0,43),(3,42),(5,41),(7,41),(10,43),(12,42)], stairs=[(6, 1)])
    lookout(t, 29, 6)
    t.put(33, 8, 0x003, True)
    t.grass(20, [(6, 8), (5, 8), (5, 9), (7, 9)])
    t.grass(9, [(53, 57), (54, 57), (54, 56)])
    t.grass(37, [(32, 33), (31, 34), (31, 34), (32, 33)])
    for x, y in ((9, 37), (10, 28), (12, 25), (14, 23), (26, 26), (29, 23),
                 (31, 16), (35, 15), (36, 11), (40, 10), (40, 14), (49, 10),
                 (50, 15), (52, 14), (53, 11), (55, 16), (59, 13), (32, 10),
                 (26, 37), (49, 39)):
        t.put(x, y, 0x36F)
    for x, y in ((10, 25), (8, 27), (26, 22), (31, 15), (32, 5), (52, 11), (50, 14)):
        t.put(x, y, 0x004)
    uplift(t, upper_rim, (32, 33), upper_toe)
    suspension_span(t, 14, 26, 24, 19, 5)
    suspension_span(t, 38, 52, 12, 43, 7, 5)
    cloudline(t)
    return t

# Hand-placed crown origins. The spruce stands thicken along the higher ledges.
GROVES = {
    "Virelia_RainfallFoothills": (
        [
            (0, 25), (48, 25), (0, 1), (2, 1), (0, 3), (4, 0), (7, 2), (7, 0),
            (13, 0), (15, 2), (17, 0), (20, 1), (24, 1), (40, 0), (44, 1), (46, 0),
            (48, 2), (1, 10), (3, 11), (1, 12), (5, 12), (7, 11), (3, 14), (13, 19),
            (15, 18), (19, 16), (45, 11), (48, 9), (48, 11), (3, 23), (6, 22), (1, 22),
            (21, 24), (23, 25), (25, 24), (27, 24), (7, 30), (7, 32), (0, 32), (0, 34),
            (0, 39), (5, 39), (7, 37), (7, 39), (16, 34), (18, 33), (18, 35), (20, 34),
            (22, 36), (21, 39), (24, 38), (27, 34), (29, 34), (24, 36), (26, 38), (28, 37),
            (40, 37), (42, 37), (44, 38), (46, 37), (48, 38), (47, 34), (48, 31),
        ],
        [
            (9, 1), (11, 2), (29, 0), (19, 8), (27, 7), (10, 12), (30, 19), (43, 23),
            (48, 19), (48, 22),
        ],
    ),
    "Virelia_RainfallUpper": (
        [
            (4, 8), (6, 8), (6, 10), (2, 18), (2, 20), (3, 23), (5, 23), (4, 26),
            (6, 26), (8, 24), (4, 28), (10, 38), (13, 41), (15, 38), (15, 40), (28, 37),
            (35, 38), (50, 38), (52, 38), (56, 38), (58, 37),
        ],
        [
            (9, 7), (12, 6), (12, 8), (15, 7), (24, 6), (26, 7), (26, 9), (30, 10),
            (28, 11), (24, 9), (36, 4), (38, 5), (38, 7), (36, 7), (50, 5), (52, 6),
            (54, 6), (58, 8), (58, 10), (60, 9), (60, 11), (60, 16), (9, 18), (12, 19),
            (14, 18), (14, 20), (10, 21), (24, 18), (26, 19), (24, 20), (35, 23), (37, 24),
            (39, 24), (37, 26), (34, 27), (50, 23), (50, 25), (53, 25), (56, 26), (5, 42),
            (17, 37), (13, 38), (54, 39), (59, 40),
        ],
    ),
}


def pass_approaches():
    bank = ROOT / "data/tilesets/secondary/virelia_rainfall"
    metas, attrs = words(bank / "metatiles.bin"), words(bank / "metatile_attributes.bin")
    for target, source, behavior in ((0x385, 0x340, 0x64), (0x386, 0x3B2, 0x62)):
        i = target - 512
        j = source - 512
        metas[i*8:(i+1)*8] = metas[j*8:(j+1)*8]
        attrs[i] = behavior
    write(bank / "metatiles.bin", metas)
    write(bank / "metatile_attributes.bin", attrs)


def build():
    from pack_rainfall_marsh import pack_marsh
    from pack_stormcrest import pack_stormcrest
    from rainfall_wetlands import build_wetlands

    bridges()
    pack_marsh(ROOT)
    trail_corners()
    pack_stormcrest(ROOT)
    pass_approaches()
    for t, directions in ((foothills(), (0x323, 0x385, 0x32C)),
                          (upper(), (0x323, 0x386))):
        broadleaf, spruce = GROVES[t.name]
        plant_trees(t.g, spruce, pieces=(0x389, 0x38B, 0x38D, 0x38F))
        t.finish(broadleaf, directions, border=0x3A5 if t.name == "Virelia_RainfallUpper" else 1)
    build_wetlands(Terrain, ROOT)


if __name__ == "__main__":
    build()
