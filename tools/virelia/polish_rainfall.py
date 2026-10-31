"""Polish Chapter 2 route terrain using unmodified Emerald metatile artwork.

This deliberately edits only the four new route layouts and reused Fortree
metatile aliases. Event JSON, older hand-authored maps and source pixels stay
untouched. Run this after the original chapter authoring tool, never before.
"""

from pathlib import Path
import json
import struct
from native_terrain import paint_river, paint_pond, paint_rock_outcrop

ROOT = Path(__file__).resolve().parents[2]


def words(p):
    b = p.read_bytes()
    return list(struct.unpack("<%dH" % (len(b) // 2), b))


def write(p, a):
    p.write_bytes(struct.pack("<%dH" % len(a), *a))


class Terrain:
    def __init__(self, suffix, w, h, elevation=3):
        self.name = "Virelia_" + suffix
        self.w, self.h, self.e = w, h, elevation
        self.g = [[self.cell(1) for x in range(w)] for y in range(h)]
        self.land = set()
        self.path = set()

    def cell(self, tile, solid=False):
        return tile | self.e << 12 | (0x400 if solid else 0)

    def rect(self, x, y, w, h, tile=1, solid=False):
        for yy in range(y, y + h):
            for xx in range(x, x + w):
                self.g[yy][xx] = self.cell(tile, solid)
                self.land.add((xx, yy))

    def clearing(self, x, y, w, h):
        self.rect(x, y, w, h)

    def road(self, x, y, w, h):
        self.path.update((xx, yy) for yy in range(y, y + h) for xx in range(x, x + w))

    def trails(self):
        # Native dirt edges follow the footprint; a grass shoulder separates
        # trail edges from tree trunks rather than isolated shrub filler.
        for x, y in sorted(self.path):
            for yy in range(max(0, y - 1), min(self.h, y + 2)):
                for xx in range(max(0, x - 1), min(self.w, x + 2)):
                    self.land.add((xx, yy))
        for x, y in self.path:
            top, bottom = (x, y - 1) not in self.path, (x, y + 1) not in self.path
            left, right = (x - 1, y) not in self.path, (x + 1, y) not in self.path
            t = (
                (0x118 if left else 0x11A if right else 0x119)
                if top
                else (
                    (0x128 if left else 0x12A if right else 0x129)
                    if bottom
                    else 0x120 if left else 0x122 if right else 0x121
                )
            )
            self.g[y][x] = self.cell(t)

    def forest(self):
        # Stock Petalburg Woods construction: trees overlap every two rows.
        # Inner canopy pieces are distinct from exposed outer edge caps.
        # All placements are complete 2x3 trees, with adjoining crowns merged.
        cells = set()
        for y in range(-2, self.h, 2):
            for x in range(0, self.w, 2):
                footprint = {
                    (xx, yy)
                    for yy in range(y, y + 3)
                    for xx in range(x, x + 2)
                    if 0 <= yy < self.h and xx < self.w
                }
                if footprint and not footprint & self.land:
                    cells.add((x, y))
        for x, y in sorted(cells, key=lambda p: (p[1], p[0])):
            left, right = (x - 2, y) in cells, (x + 2, y) in cells
            above, below = (x, y - 2) in cells, (x, y + 2) in cells
            top = (0x1DC, 0x1DD) if above else (0x1CE, 0x1CF)
            middle = (0x1D4 if left else 0x1D6, 0x1D5 if right else 0x1D7)
            bottom = (0x1E4 if left else 0x1E6, 0x1E5 if right else 0x1E7)
            for dy, row in enumerate((top, middle, bottom)):
                if dy == 2 and below:
                    continue
                for dx, t in enumerate(row):
                    if 0 <= y + dy < self.h and x + dx < self.w:
                        self.g[y + dy][x + dx] = self.cell(t, True)
        # Thin strips where a complete crown cannot fit are grass shoulders,
        # closed to walking only at the map's physical boundary.
        for y in range(self.h):
            for x in range(self.w):
                if (x in (0, self.w - 1) or y in (0, self.h - 1)) and (x, y) not in self.path:
                    self.g[y][x] |= 0x400

    def grass(self, rows, x, y, long=False):
        for dy, row in enumerate(rows):
            for dx, c in enumerate(row):
                if c == "#":
                    self.rect(x + dx, y + dy, 1, 1, 0x015 if long else 0x00D)

    def river(self, spans):
        wet = {(x, y) for y, (left, right) in spans.items() for x in range(left, right + 1)}
        self.land.update(paint_river(self.g, wet, elevation=self.e, land_elevation=self.e))
        # At a physical map edge the channel continues, without a false cap.
        for y in (0, self.h - 1):
            if y in spans:
                l, r = spans[y]
                for x in range(l, r + 1):
                    self.rect(x, y, 1, 1, 0x190 if x == l else 0x192 if x == r else 0x170, True)

    def pond(self, spans):
        wet = {(x, y) for y, (left, right) in spans.items() for x in range(left, right + 1)}
        self.land.update(paint_pond(self.g, wet, elevation=self.e, land_elevation=self.e))

    def rock(self, x, y, w, h):
        # The top and foot step inward so exposed rock is a terrain mass,
        # rather than a rectangle filled with repeated cliff-edge artwork.
        shape = {
            (xx, yy)
            for yy in range(y, y + h)
            for xx in range(x, x + w)
            if not (yy in (y, y + h - 1) and xx in (x, x + w - 1))
        }
        self.land.update(paint_rock_outcrop(self.g, shape, elevation=self.e))

    def bridge(self, x, y, width):
        # Horizontal Fortree bridge, both complete deck rows and end rails.
        for dx in range(width):
            top, bottom = (
                (0x324, 0x325) if dx == 0 else (0x326, 0x327) if dx == width - 1 else (0x320, 0x321)
            )
            self.rect(x + dx, y, 1, 1, top)
            self.rect(x + dx, y + 1, 1, 1, bottom)
        for xx, top, bottom in [(x - 1, 0x328, 0x329), (x + width, 0x32A, 0x32B)]:
            self.rect(xx, y, 1, 1, top)
            self.rect(xx, y + 1, 1, 1, bottom)

    def save(self):
        # Existing portals/events remain authoritative; preserve their IDs and
        # coordinates while making all event floors match authored elevation.
        doc = json.loads((ROOT / "data/maps" / self.name / "map.json").read_text())
        for obj in doc["object_events"]:
            x, y = obj["x"], obj["y"]
            if self.g[y][x] & 0xC00:
                raise ValueError(f"{self.name}: terrain blocks object {x},{y}")
        directions = {
            "RainfallFoothills": (0x323, 0x322, 0x32C),
            "RainfallUpper": (0x323, 0x322),
            "RainfallLower": (0x323, 0x322),
            "RainfallLowlands": (0x322, 0x322, 0x323),
        }
        for index, warp in enumerate(doc["warp_events"]):
            tile = directions[self.name.removeprefix("Virelia_")][index]
            self.g[warp["y"]][warp["x"]] = self.cell(tile)
        folder = ROOT / "data/layouts" / self.name
        write(folder / "map.bin", [c for row in self.g for c in row])
        write(folder / "border.bin", [self.cell(t, True) for t in (0x1D4, 0x1D5, 0x1DC, 0x1DD)])


def bridges():
    bank = ROOT / "data/tilesets/secondary/virelia_rainfall"
    src = ROOT / "data/tilesets/secondary/fortree"
    metas, attrs = words(bank / "metatiles.bin"), words(bank / "metatile_attributes.bin")
    original = words(src / "metatiles.bin")
    for target, source in [
        (0x320, 0x210),
        (0x321, 0x218),
        (0x324, 0x216),
        (0x325, 0x21E),
        (0x326, 0x217),
        (0x327, 0x21F),
        (0x328, 0x211),
        (0x329, 0x219),
        (0x32A, 0x215),
        (0x32B, 0x21D),
    ]:
        need = target - 511
        metas += [0] * max(0, need * 8 - len(metas))
        attrs += [0] * max(0, need - len(attrs))
        metas[(target - 512) * 8 : (target - 511) * 8] = original[
            (source - 512) * 8 : (source - 511) * 8
        ]
        attrs[target - 512] = 0x1000
    # Existing path art, native east-facing warp at the lower trail fork.
    primary = words(ROOT / "data/tilesets/primary/general/metatiles.bin")
    metas += primary[0x121 * 8 : 0x122 * 8]
    attrs += [0x62]
    metas = metas[: (0x32D - 512) * 8]
    attrs = attrs[: 0x32D - 512]
    metas[(0x32C - 512) * 8 : (0x32D - 512) * 8] = primary[0x121 * 8 : 0x122 * 8]
    attrs[0x32C - 512] = 0x62
    write(bank / "metatiles.bin", metas)
    write(bank / "metatile_attributes.bin", attrs)


def build():
    bridges()
    f = Terrain("RainfallFoothills", 32, 28)
    for r in [
        (14, 22, 2, 5),
        (7, 21, 9, 2),
        (7, 10, 2, 12),
        (7, 10, 18, 2),
        (7, 3, 2, 8),
        (24, 10, 2, 10),
        (24, 18, 5, 2),
        (24, 19, 2, 4),
    ]:
        f.road(*r)
    for r in [
        (8, 9, 4, 3),
        (5, 14, 3, 5),
        (13, 22, 4, 3),
        (24, 17, 6, 4),
        (24, 22, 5, 3),
        (6, 2, 4, 4),
    ]:
        f.clearing(*r)
    f.road(7, 0, 2, 4)
    f.road(14, 26, 2, 2)
    f.road(28, 18, 4, 2)
    f.trails()
    f.rock(12, 14, 9, 5)
    f.rock(14, 3, 6, 4)
    # Slateford-facing ridge: a native stone stair cut, forest above and
    # grassy foothills below, makes the descent visible before the fork.
    for x in range(32):
        for y in range(7, 9):
            f.rect(x, y, 1, 1, 0x07C if y == 7 else 0x091, True)
    for y in (7, 8):
        f.rect(7, y, 1, 1, 0x0AF)
        f.rect(8, y, 1, 1, 0x0CF)
    f.grass([".###", "####", "###.", "##.."], 3, 14)
    f.grass([".#####", "######", "####.."], 20, 4)
    f.rect(10, 10, 1, 1, 3, True)
    f.forest()
    f.save()

    u = Terrain("RainfallUpper", 32, 34, 5)
    for r in [
        (5, 27, 2, 5),
        (5, 26, 5, 2),
        (8, 18, 2, 10),
        (8, 18, 13, 2),
        (20, 8, 2, 19),
        (20, 8, 5, 2),
        (23, 3, 2, 7),
    ]:
        u.road(*r)
    for r in [
        (4, 28, 4, 4),
        (6, 18, 4, 4),
        (19, 18, 4, 3),
        (21, 7, 5, 4),
        (22, 11, 7, 5),
        (22, 2, 4, 4),
        (8, 7, 4, 4),
    ]:
        u.clearing(*r)
    # Sightseeing spur on the upper west bank; the sign is a real viewpoint.
    for r in [(8, 8, 2, 11), (8, 8, 5, 2)]:
        u.road(*r)
    u.road(5, 31, 2, 3)
    u.road(23, 0, 2, 4)
    u.trails()
    spans = {
        y: ((13, 17) if y < 10 else (13, 17) if y < 23 else (14, 18) if y < 29 else (13, 17))
        for y in range(34)
    }
    u.river(spans)
    # Native cross-cliff waterfall; three water columns, solid side walls.
    for x in range(11, 20):
        for y in range(9, 12):
            u.rect(x, y, 1, 1, 0x07C if y == 9 else 0x0A9, True)
    for dy, row in enumerate(((0x0BB, 0x0B3, 0x0BC), (0x0B9, 0x0B9, 0x0B9), (0x02C, 0x034, 0x03C))):
        for dx, t in enumerate(row):
            u.rect(14 + dx, 10 + dy, 1, 1, t, True)
    u.bridge(12, 18, 7)
    # Complete a three-row bank, including its river-facing side and foot.
    terrace = {(x, y) for y in range(24, 27) for x in range(19, 32)}
    u.land.update(paint_rock_outcrop(u.g, terrace, elevation=u.e))
    for y in range(24, 27):
        u.rect(20, y, 1, 1, 0x0AF)
        u.rect(21, y, 1, 1, 0x0CF)
    u.grass([".###", "####", "###."], 3, 21)
    u.grass(["..###", "#####", "#####", ".####"], 24, 11, True)
    u.rect(10, 9, 1, 1, 3, True)
    u.forest()
    u.save()

    lo = Terrain("RainfallLower", 40, 40)
    for r in [
        (4, 32, 2, 5),
        (4, 31, 14, 2),
        (16, 25, 2, 8),
        (8, 24, 10, 2),
        (8, 15, 2, 11),
        (8, 14, 23, 2),
        (29, 14, 2, 17),
        (29, 29, 7, 2),
        (34, 6, 2, 25),
        (25, 5, 11, 2),
        (25, 2, 2, 5),
        (4, 2, 23, 2),
        (18, 20, 10, 2),
    ]:
        lo.road(*r)
    for r in [
        (3, 33, 5, 5),
        (14, 29, 5, 5),
        (6, 22, 5, 4),
        (7, 17, 5, 5),
        (10, 16, 7, 6),
        (28, 13, 5, 4),
        (29, 28, 7, 4),
        (32, 6, 5, 7),
        (24, 4, 5, 4),
        (3, 1, 5, 5),
        (15, 19, 5, 4),
    ]:
        lo.clearing(*r)
    lo.road(4, 36, 2, 4)
    lo.road(4, 0, 2, 3)
    lo.trails()
    # A winding stream divides the two long exploration loops. Its open ends
    # continue beyond the map rather than stopping in a rectangular basin.
    spans = {
        y: ((20, 24) if y < 12 else (21, 25) if y < 27 else (22, 26) if y < 34 else (23, 27))
        for y in range(7, 40)
    }
    lo.river(spans)
    # A broad source pool merges naturally into the wetland stream.
    pool = {y: (15, 23) if y in (6, 11) else (14, 24) for y in range(6, 12)}
    wet = {(x, y) for y, (l, r) in spans.items() for x in range(l, r + 1)}
    wet.update((x, y) for y, (l, r) in pool.items() for x in range(l, r + 1))
    lo.land.update(paint_river(lo.g, wet, elevation=lo.e, land_elevation=lo.e))
    lo.bridge(20, 20, 7)
    for rows, x, y in [
        ([".###", "####", "####", "###.", "####", "###.", "##.."], 3, 21),
        ([".####", "#####", "#####", "###.."], 11, 16),
        (["..####", "######", "#####.", "####.."], 25, 8),
        ([".####", "#####", "#####", "###.."], 31, 32),
        ([".####", "#####", "###.."], 17, 35),
    ]:
        lo.grass(rows, x, y, True)
    lo.rect(10, 19, 9, 2)
    lo.rect(16, 20, 2, 3)
    lo.rect(16, 22, 1, 1, 0x0C5, True)
    lo.forest()
    lo.save()

    ll = Terrain("RainfallLowlands", 32, 26)
    for r in [
        (4, 3, 2, 7),
        (4, 8, 12, 2),
        (25, 3, 2, 7),
        (15, 8, 12, 2),
        (15, 8, 2, 4),
        (15, 12, 1, 4),
        (15, 15, 2, 9),
        (15, 20, 12, 2),
    ]:
        ll.road(*r)
    for r in [
        (3, 2, 4, 5),
        (24, 2, 4, 5),
        (13, 8, 6, 3),
        (14, 15, 4, 6),
        (21, 19, 6, 4),
        (22, 14, 7, 4),
    ]:
        ll.clearing(*r)
    ll.road(4, 0, 2, 4)
    ll.road(25, 0, 2, 4)
    ll.road(15, 23, 2, 3)
    ll.trails()
    # Sheltered spring west of the rival clearing, native water and shoreline.
    ll.pond({16: (5, 8), 17: (4, 9), 18: (4, 9), 19: (4, 9), 20: (5, 8)})
    ll.grass([".#######", "########", "#######.", ".#####.."], 20, 14)
    ll.grass(["##", "##", "##"], 25, 18)
    ll.forest()
    # A coherent cliff face spans the map; the one-tile sheltered notch is the
    # existing rival gate. Its flag callback and saved state remain unchanged.
    ridge = set()
    for x in range(32):
        top, bottom = (
            (12, 15) if x < 8 else (11, 14) if x < 20 else (12, 15) if x < 26 else (13, 15)
        )
        ridge.update((x, y) for y in range(top, bottom + 1))
    paint_rock_outcrop(ll.g, ridge, elevation=ll.e)
    for y in range(11, 15):
        ll.g[y][14] = ll.cell(0x073, True)
        ll.g[y][16] = ll.cell(0x075, True)
    ll.g[11][15] = ll.cell(0x121)
    ll.g[12][15] = ll.cell(0x121)
    ll.g[13][15] = ll.cell(0x00E, True)
    ll.g[14][15] = ll.cell(0x121)
    ll.g[12][16] = ll.cell(1)
    ll.save()


if __name__ == "__main__":
    build()
