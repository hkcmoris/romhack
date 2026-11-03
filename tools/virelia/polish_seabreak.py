#!/usr/bin/env python3
"""Build Seabreak's harbor, lighthouse and native tile assemblies.

The Seabreak harbor and its dedicated coast bank are rebuilt together. Map
metadata, warps, scripts, actors and interaction coordinates stay unchanged.
Run after build_rainfall_seabreak.py; never regenerate hand-edited maps blindly.
"""

from pathlib import Path
from functools import lru_cache
import json
import struct
from PIL import Image, ImageDraw
from native_terrain import close_edge, paint_river, plant_trees
from pack_stormcrest import spruce_panels, write_palette
from seabreak_harbor import build_harbor
from seabreak_approaches import build_approaches
from seabreak_surfaces import CoastSurfaces, ROOF_REINDEX, floor_palettes
from cliff_transitions import install_coast_rock_palette, retaining_end
from cliff_contours import coast_outline, install_shared_crests, turns

ROOT = Path(__file__).resolve().parents[2]
COAST = ROOT / "data/tilesets/secondary/virelia_coast"
PRIMARY = ROOT / "data/tilesets/primary/general"
LIGHTHOUSE_WIDTH = 5
LIGHTHOUSE_HEIGHT = 10
LIGHTHOUSE_BASE = (".###.", "#####", ".#D#.")


def words(path):
    data = path.read_bytes()
    return list(struct.unpack("<%dH" % (len(data) // 2), data))


def write_words(path, values):
    path.write_bytes(struct.pack("<%dH" % len(values), *values))


@lru_cache(None)
def source(bank):
    path = ROOT / "data/tilesets/secondary" / bank
    return (
        path,
        words(path / "metatiles.bin"),
        words(path / "metatile_attributes.bin"),
        Image.open(path / "tiles.png").copy(),
    )


def stock_grid(name):
    layouts = json.loads((ROOT / "data/layouts/layouts.json").read_text(encoding="utf-8-sig"))[
        "layouts"
    ]
    layout = next(entry for entry in layouts if entry["name"] == name + "_Layout")
    data = words(ROOT / layout["blockdata_filepath"])
    return [data[y * layout["width"]: (y + 1) * layout["width"]] for y in range(layout["height"])]


class StockBank:
    """Copy existing indexed pixels, flips, palettes and behavior as complete units."""

    def __init__(self):
        # The first 50 metatiles are the editable lighthouse, in sheet order.
        self.attrs = [0] * (LIGHTHOUSE_WIDTH * LIGHTHOUSE_HEIGHT)
        self.meta = [0] * (8 * len(self.attrs))
        self.tiles = []
        # General's immutable 0x0BF is an opaque quarter of color 5.
        self.lookup = {bytes([5]) * 64: 0x0BF}
        self.ids = {}
        self.palette = source("slateport")[3].getpalette()
        self.harbor = Image.open(COAST / "harbor.png")
        self.terrace = Image.open(COAST / "terrace.png")
        self.general = words(PRIMARY / "metatiles.bin")
        self.general_attrs = words(PRIMARY / "metatile_attributes.bin")
        self.general_image = Image.open(PRIMARY / "tiles.png").copy()
        self.general_attrs = words(PRIMARY / "metatile_attributes.bin")

    def graphic(self, pixels):
        """Return the 8×8 graphic ID and the GBA flip bits that reproduce these pixels."""
        if not any(pixels):
            return 0
        if pixels not in self.lookup:
            graphic = 512 + len(self.tiles)
            rows = [pixels[y:y + 8] for y in range(0, 64, 8)]
            self.lookup[pixels] = graphic
            self.lookup.setdefault(b"".join(row[::-1] for row in rows), graphic | 0x400)
            self.lookup.setdefault(b"".join(reversed(rows)), graphic | 0x800)
            self.lookup.setdefault(pixels[::-1], graphic | 0xC00)
            self.tiles.append(pixels)
        return self.lookup[pixels]

    def harbor_panel(self, col, row, ground, covered=False):
        return self.prop_panel(self.harbor, col, row, ground, covered)

    def prop_panel(self, sheet, col, row, ground, covered):
        key = (sheet.filename, col, row, ground, covered)
        if key not in self.ids:
            offset = (ground % 512) * 8
            source_meta = self.general if ground < 512 else self.meta
            pieces = source_meta[offset:offset + 8]
            for q in range(4):
                x, y = 16 * col + 8 * (q % 2), 16 * row + 8 * (q // 2)
                pixels = sheet.crop((x, y, x + 8, y + 8)).tobytes()
                if any(pixels):
                    if pieces[4 + q] & 1023 and 0 in pixels:
                        raise ValueError(f"{Path(sheet.filename).name} panel {(col, row)} overlaps foreground on {ground:#x}")
                    pieces[4 + q] = 0xB000 | self.graphic(pixels)
            self.ids[key] = 512 + len(self.attrs)
            self.meta.extend(pieces)
            self.attrs.append(0x1000 if covered else 0)
        return self.ids[key]

    def terrace_fitting(self, grid, x, y, kind, width=None):
        """Place the market fountain, an overlook railing, or a cliff-wall culvert."""
        left, top, w, h = {
            "fountain": (0, 0, 3, 2),
            "railing": (3, 0, width, 1),
            "drain": (3, 1, 2, 2),
        }[kind]
        for dy in range(h):
            for dx in range(w):
                previous = grid[y + dy][x + dx]
                ground = 1 if kind == "drain" else previous & 1023
                col = left + (dx % 2 if kind == "railing" else dx)
                tile = self.prop_panel(self.terrace, col, top + dy, ground, covered=dy == h - 1)
                solid = kind != "fountain" or dy == 1 or dx == 1
                grid[y + dy][x + dx] = (previous & ~1023) | tile | (0x400 if solid else 0)

    def cliff_landscape(self):
        # This coast-only slot recolors General's warm crags into Stormcrest
        # schist without spending any additional 8x8 graphics on the faces.
        colors = (PRIMARY / "palettes/03.pal").read_text().splitlines()
        colors[4:9] = ["152 176 144", "136 160 136", "120 144 120", "104 132 112", "80 108 96"]
        colors[11:18] = ["48 56 72","200 208 192","166 181 174","136 157 155",
                         "111 135 139","86 108 119","65 84 101"]
        (COAST / "palettes/10.pal").write_text("\n".join(colors)+"\n")
        variants = {}
        for tile in (0x068,0x069,0x06A,0x070,0x071,0x072,0x073,0x075,0x078,0x079,0x07A,
                     0x07B,0x07C,0x07D,0x087,0x09F,0x0A9,0x0D5,0x0D6,0x0E0,0x0E2):
            parts = self.general[tile*8:tile*8 + 8]
            variants[tile] = 512 + len(self.attrs)
            self.meta.extend((p & 0x0FFF) | 0xA000 if p >> 12 == 3 else p for p in parts)
            self.attrs.append(0x1000)
        # A low-contrast, opaque moor tile joins the repeating western border.
        grain = bytes(int(pixel) for pixel in
                      "3333333333233333333334333333433333333333343333333333332333333333")
        graphic = self.graphic(grain)
        heath = 512 + len(self.attrs)
        self.meta.extend([0xA000 | (graphic ^ flip) for flip in (0, 0x400, 0xC00, 0x800)] + [0] * 4)
        self.attrs.append(0x1000)
        # Keep Stormcrest's directional lighting and bark colors over the
        # coastal heath. The outdoor buildings and doors leave slot 8 free.
        fen = Image.open(COAST.parent / "virelia_rainfall/fen.png")
        write_palette(COAST / "palettes/08.pal", fen)
        first = 512 + len(self.attrs)
        heath_base = self.meta[(heath - 512)*8:(heath - 512)*8 + 4]
        for panel in spruce_panels(fen):
            self.meta.extend(heath_base)
            for q in range(4):
                sx, sy = 8*(q % 2), 8*(q // 2)
                pixels = panel.crop((sx, sy, sx + 8, sy + 8)).tobytes()
                self.meta.append(0x8000 | self.graphic(pixels))
            self.attrs.append(0)
        floor_palettes(COAST, PRIMARY, source('slateport')[0])

        # Capped headwater, falling curtain and foam. Mirroring the two edge
        # quarters uses six 8x8 graphics from the same source as Rainfall.
        cascade = Image.open(COAST.parent / "virelia_rainfall/cascade.png")
        falls = []
        for texture, ys, attr in ((0x0B3,(0,16),0x1013), (0x0B9,(16,16),0x1013),
                                  (0x170,(16,40),0x1010)):
            for flip in (False,True):
                falls.append(512 + len(self.attrs))
                self.meta.extend(self.general[8*texture:8*texture+4])
                for q in range(4):
                    sx = (q % 2 if not flip else 1-q % 2)*8
                    sy = ys[q//2]
                    bits = cascade.crop((sx,sy,sx+8,sy+8)).tobytes()
                    self.meta.append(0x4000 | (self.graphic(bits) ^ (0x400 if flip else 0)))
                self.attrs.append(attr)
        return variants, heath, tuple(first + i for i in (0,2,4,6)), falls

    def lighthouse(self, grid, left, top):
        tower = Image.open(COAST / "lighthouse.png")
        if (tower.mode != "P" or tower.size != (16 * LIGHTHOUSE_WIDTH, 16 * LIGHTHOUSE_HEIGHT)
                or tower.getextrema()[1] > 15 or tower.info.get("transparency") != 0):
            raise ValueError("lighthouse.png must be a 5x10 metatile sheet with 4bpp indices and transparent index 0")
        colors = (COAST / "palettes/11.pal").read_text().splitlines()[3:19]
        if tower.getpalette()[:48] != [int(channel) for color in colors for channel in color.split()]:
            raise ValueError("Lighthouse sheet colors must match coast palette 11")
        primary = words(PRIMARY / "metatiles.bin")
        for row in range(LIGHTHOUSE_HEIGHT):
            for col in range(LIGHTHOUSE_WIDTH):
                index = row * LIGHTHOUSE_WIDTH + col
                x, y = col * 16, row * 16
                panel = tower.crop((x, y, x + 16, y + 16))
                if not panel.getbbox():
                    self.meta[index * 8:index * 8 + 8] = primary[8:16]  # Grass in the editor.
                    continue
                block = grid[top + row][left + col]
                ground = block & 1023
                table = primary if ground < 512 else self.meta
                offset = ground % 512 * 8
                pieces = table[offset:offset + 8]
                for q in range(4):
                    x, y = (q % 2) * 8, (q // 2) * 8
                    pixels = panel.crop((x, y, x + 8, y + 8)).tobytes()
                    if any(pixels):
                        if pieces[4 + q] & 1023 and 0 in pixels:
                            raise ValueError(f"Lighthouse obscures a terrain overlay at {(left + col, top + row, q)}")
                        pieces[4 + q] = 0xB000 | self.graphic(pixels)
                self.meta[index * 8:index * 8 + 8] = pieces
                # Like Slateport, the raised tower can hide a passing actor.
                # The ground-level foundation draws behind actors at the door.
                base_row = row - (LIGHTHOUSE_HEIGHT - len(LIGHTHOUSE_BASE))
                footing = LIGHTHOUSE_BASE[base_row][col] if base_row >= 0 else "."
                self.attrs[index] = (0x1000 if base_row > 0 else 0) | (0x60 if footing == "D" else 0)
                collision = 0x400 if footing == "#" else 0
                grid[top + row][left + col] = (block & ~1023) | (512 + index) | collision

    def tile(self, bank, identifier, behavior=None):
        if identifier < 512:
            return identifier
        key = (bank, identifier, behavior)
        if key in self.ids:
            return self.ids[key]
        path, meta, attrs, image = source(bank)
        copied = []
        for value in meta[(identifier - 512) * 8: (identifier - 511) * 8]:
            palette = value >> 12
            index = value & 1023
            if index < 512 and palette < 6:
                copied.append(value)
                continue
            sheet = self.general_image if index < 512 else image
            x, y = index % 16 * 8, index % 512 // 16 * 8
            pixels = sheet.crop((x, y, x + 8, y + 8)).tobytes()
            remapped = pixels.translate(ROOF_REINDEX) if palette == 9 else pixels
            if palette >= 6:
                # A donor can also color General graphics with a secondary palette.
                original = (path / f"palettes/{palette:02}.pal").read_text().splitlines()[3:]
                packed = (COAST / f"palettes/{palette:02}.pal").read_text().splitlines()[3:]
                assert all(original[i] == packed[j] for i, j in set(zip(pixels, remapped)) if i), (
                    bank, hex(identifier), palette)
            if palette == 6 and pixels == bytes([7]) * 64:
                # Slot 7's native roof shade 5 is also Slateport shade 7.
                native = (COAST / "palettes/07.pal").read_text().splitlines()[3:]
                assert original[7] == native[5]
                value = 0x70BF
            elif index >= 512 or remapped != pixels:
                value = (value & ~1023) ^ self.graphic(remapped)
            copied.append(value)
        result = 512 + len(self.attrs)
        self.ids[key] = result
        self.meta.extend(copied)
        attribute = attrs[identifier - 512]
        self.attrs.append(attribute if behavior is None else (attribute & ~0x1FF) | behavior)
        return result

    def finish(self):
        assert len(self.tiles) <= 496, ("reserved door animation graphics", len(self.tiles))
        assert len(self.attrs) <= 512
        assert all(
            value >> 12 <= 12 for value in self.meta
        ), "Only loaded palettes 0..12 may be referenced"
        image = Image.new("P", (128, 256))
        image.putpalette(self.palette)
        for i, pixels in enumerate(self.tiles):
            image.paste(Image.frombytes("P", (8, 8), pixels), (i % 16 * 8, i // 16 * 8))
        image.save(COAST / "tiles.png")
        write_words(COAST / "metatiles.bin", self.meta)
        write_words(COAST / "metatile_attributes.bin", self.attrs)
        print(
            f"Seabreak coast pack: {len(self.attrs)} metatiles, {len(self.tiles)}/496 graphics tiles"
        )


def main():
    install_shared_crests(PRIMARY)
    bank = StockBank()
    width, height = 58, 56
    grid = [[0x1570] * width for _ in range(height)]

    # Each section gives the last land column at this latitude. The moors
    # continue west and northwest to Rainfall; the entire bay opens east.
    coast = ((1, 54), (2, 48), (3, 45), (4, 44), (5, 42), (7, 43), (8, 45), (10, 46),
             (13, 45), (14, 44), (15, 42), (16, 39), (17, 36), (19, 35),
             (20, 37), (22, 39), (24, 42), (26, 43), (27, 41), (29, 39),
             (31, 42), (33, 43), (35, 44), (36, 42), (37, 39), (38, 30),
             (39, 24), (40, 5), (42, 4), (45, 3), (48, 4), (50,5), (51,7), (53,9), (55,11))
    east = [next(x for last_y, x in coast if y <= last_y) for y in range(height)]
    land = {(x, y) for y in range(height) for x in range(east[y] + 1)}

    # Cross-sections of the Stormcrest slope, read from the inland edge to
    # the cape. Each pair is the crest and foot of that column of cliff.
    def section(spans):
        return [next((crest, toe) for end, crest, toe in spans if x <= end) for x in range(width)]

    back = section(((3, 5, 9), (6, 3, 6), (9, 2, 5), (12, 1, 4),
                    (15, 2, 5), (18, 3, 7), (21, 4, 8), (23, 4, 7),
                    (27, 5, 8), (28, 3, 6), (34, 0, 3), (37, 1, 4),
                    (40, 1, 6), (44, 2, 7), (57, 1, 7)))
    upper = section(((3, 11, 16), (6, 12, 16), (9, 13, 16),
                     (14, 14, 17), (16, 13, 16), (18, 12, 15),
                     (21, 11, 15), (24, 12, 16), (26, 13, 16),
                     (28, 14, 17), (30, 15, 18), (34, 16, 18),
                     (37, 17, 21), (40, 15, 20), (43, 13, 18), (57, 11, 16)))
    lower = section(((2, 29, 34), (5, 30, 34), (7, 29, 33), (10, 28, 31),
                     (13, 28, 32), (15, 27, 30), (20, 26, 28), (21, 27, 30),
                     (24, 28, 31), (26, 27, 30), (29, 26, 29), (34, 25, 29),
                     (36, 26, 29), (39, 27, 30), (57, 25, 28)))

    def level(x, y):
        return 7 if y <= upper[x][0] else 5 if y <= lower[x][0] else 3

    def cell(x, y, tile, solid=False, elevation=None):
        grid[y][x] = ((level(x, y) if elevation is None else elevation) << 12) | (0x400 if solid else 0) | tile

    def slate(x, y, tile, solid=False):
        cell(x, y, bank.tile("slateport", tile), solid)

    rock, heath, spruce, falls = bank.cliff_landscape()
    surfaces = CoastSurfaces(bank, heath)
    # Round the upland margin at the root of each road cut instead of
    # separating grass and heath along one straight map column.
    verge = (4,5,5,4,4,3,2,2,1,1,2,3,4,5,5,4,3,2,1,1,2,2,1,0,0,1,2,3,
             3,2,3,3,4,3,2,1,1,2,3,4)
    for y in range(height):
        for x in range(width):
            n = (x, y - 1) in land or y == 0 and x <= east[y]
            w = (x - 1, y) in land or x == 0
            e = (x + 1, y) in land
            if (x, y) in land:
                if y <= back[x][0] or y >= len(verge) or x <= verge[y]:
                    tile = heath
                else:
                    tile = 0x001 if n else 0x002
                cell(x, y, tile, x == 0 or y <= back[x][0] or y >= 49)
            elif n or w or e or (x - 1, y - 1) in land or (x + 1, y - 1) in land:
                if n:
                    tile = 0x188 if w else 0x18A if e else 0x189
                elif w or e:
                    tile = 0x190 if w else 0x192
                else:
                    tile = 0x198 if (x - 1, y - 1) in land else 0x19A
                cell(x, y, tile, True, 1)

    soil = [[block & 1023 for block in row] for row in grid]
    # The masonry quay begins below the last shelf and ends at the open bay.
    # The dry western hillside and the estuary approach stay on the mainland.
    for x, y in sorted(land):
        if y <= lower[x][1] or x < 3 or y > 39:
            continue
        e, s = (x + 1, y) in land, (x, y + 1) in land
        tile = (0x2FF if not e else 0x25D) if not s else (0x23C if not e else 0x202)
        slate(x, y, tile, not e or not s)
    for x in range(28, 36):
        slate(x, 37, 0x202)

    paths = set()
    def pave(x, y, w, h):
        paths.update((xx, yy) for yy in range(y, y + h) for xx in range(x, x + w))

    # Keeper's lane follows the back of the high ledge, then doglegs out along
    # the lighthouse headland. The two stair landings meet different alleys.
    for stretch in ((5,11,10,2),(10,10,7,2),(14,9,15,2),(25,9,3,5),
                    (25,12,5,2),(28,13,9,3),(13,12,2,7),(27,14,2,7),
                    (6,21,12,3),(10,23,13,3),(13,20,8,3),(17,21,12,4),
                    (27,21,7,2),(8,25,3,6),(22,24,2,8),(31,23,7,2),
                    (3,26,5,2),(9,23,1,3)):
        pave(*stretch)
    for x, y in sorted(paths):
        n, s, w, e = ((x + dx, y + dy) in paths for dx, dy in ((0,-1),(0,1),(-1,0),(1,0)))
        tile = (0x208 if not w else 0x20A if not e else 0x209) if not n else (
            (0x218 if not w else 0x21A if not e else 0x219) if not s else
            (0x210 if not w else 0x212 if not e else 0x211))
        slate(x, y, tile)

    underlays = {}
    def building(donor, tileset, source_x, source_y, w, h, x, y, decorative=False):
        source_map = stock_grid(donor)
        for dy in range(h):
            for dx in range(w):
                raw = source_map[source_y + dy][source_x + dx]
                if dy == h - 1:
                    underlays[x + dx, y + dy] = grid[y + dy][x + dx] & 1023
                cell(x + dx, y + dy, bank.tile(tileset, raw & 1023),
                     bool(raw & 0xC00) or (decorative and dy == h - 1))

    # Follow the full depth of each cliff. Civil retaining walls and stairs
    # occupy the straight street fronts; elsewhere the rock remains exposed.
    ridge = ((0,8),(1,8),(2,8),(3,7),(4,7),(5,6),(6,6),(7,5),(8,5),(9,4),
             (10,4),(11,4),(12,3),(13,3),(14,3),(15,3),(16,3),(17,4),
             (18,4),(19,4),(20,4),(21,4),(22,5),(23,5),(24,6),(25,6),(26,7))
    ridge_top = dict(ridge)

    def escarpment(profile, masonry=(), stairs=(), road_cut=False):
        # Work with the drawn western profile for both sides of each turn.
        # Neighboring columns must agree on where the rock actually starts.
        profile = [(min(toe, crest + 5 - x) if x < 5 else crest, toe)
                   for x,(crest,toe) in enumerate(profile)]
        for x, (crest, toe) in enumerate(profile):
            left = profile[max(0, x - 1)][0]
            right = profile[min(width - 1, x + 1)][0]
            engineered = any(a <= x <= b for a, b in masonry)
            if x == 0 or x == 1:
                continue
            for y in range(crest, toe + 1):
                if (x, y) not in land:
                    continue
                road = ridge_top.get(x) if road_cut else None
                if road is not None and y == road-1 and y > crest:
                    # The upper rock face ends at the paved surface of the cut.
                    road_toe = lambda col: min(profile[col][1],ridge_top.get(col, height)-1)
                    sides = turns(road_toe(max(0,x-1)), y, road_toe(min(width-1,x+1)))
                    tile = coast_outline(surfaces, rock[0x079], bank.tile("slateport",0x211),
                                         sides, foot=True)
                elif road is not None and y == road+2:
                    # The lane is raised on a coursed stone plinth where the
                    # natural cliff continues below it. Keep the road surface
                    # under the coping instead of a bright strip of grass.
                    tile = bank.harbor_panel(4 + x % 3, 0, bank.tile("slateport",0x211), covered=True)
                elif engineered:
                    row = 0 if y == crest else 2 if y == toe else 1
                    tile = bank.harbor_panel(4 + x % 3, row, 1, covered=True)
                    for first, last in masonry:
                        if x == first and profile[x-1][0] <= y <= profile[x-1][1]:
                            tile = retaining_end(bank, tile, True, y-crest, y == toe)
                        if x == last and profile[x+1][0] <= y <= profile[x+1][1]:
                            tile = retaining_end(bank, tile, False, y-crest, y == toe)
                elif y == crest:
                    tile = coast_outline(surfaces, rock[0x087], grid[y][x] & 1023,
                                         turns(left, crest, right))
                elif y == toe:
                    sides = turns(profile[max(0,x-1)][1], toe, profile[min(width-1,x+1)][1])
                    ground = grid[min(height-1,y+1)][x] & 1023
                    tile = coast_outline(surfaces, rock[0x079], ground, sides, foot=True)
                else:
                    tile = rock[0x0A9]
                cell(x, y, tile, True)
        for x in stairs:
            crest, toe = profile[x]
            assert profile[x+1] == (crest,toe), "stair cheeks must land together"
            for y in range(crest, toe + 1):
                cell(x, y, 0x0AF, elevation=0)
                cell(x + 1, y, 0x0CF, elevation=0)

    escarpment(back, road_cut=True)
    escarpment(upper, masonry=((10,15),(22,24),(27,34)), stairs=(13,27))
    escarpment(lower, masonry=((7,12),(16,24),(29,34)), stairs=(8,22))

    # The lighthouse footing projects beyond the built terraces. Trace its
    # seaward rock face to the tide line, with surf below each exposed shelf.
    headland = set()
    for x in range(36,47):
        cap = max(y for y in range(16) if (x, y) in land)
        depth = 4 if 39 <= x <= 43 else 3
        cell(x, cap, rock[0x07C], True)
        for dy in range(1, depth + 1):
            if (x, cap + dy) not in land:
                headland.add((x,cap+dy))
                cell(x, cap + dy, rock[0x0A9], True, 1)
    # Refit the waterline to the *foot* of the cape, including the sea-facing
    # sides and inside elbows. Its schist shadow lies on live animated water.
    coast_land = land | headland
    shore = {(x+dx,y+dy) for x,y in headland for dx in (-1,0,1) for dy in (-1,0,1)}
    for x,y in sorted(shore - coast_land):
        n = (x,y-1) in coast_land
        w, e = (x-1,y) in coast_land, (x+1,y) in coast_land
        if n:
            tile = 0x188 if w else 0x18A if e else 0x189
        elif w or e:
            tile = 0x190 if w else 0x192
        else:
            tile = 0x198 if (x-1,y-1) in coast_land else 0x19A if (x+1,y-1) in coast_land else 0x170
        if tile != 0x170:
            pieces = surfaces.pieces(tile)
            pieces = [(v & 0xFFF) | 0xA000 if v >> 12 == 3 else v for v in pieces]
            tile = surfaces.variant(tile,pieces)
        cell(x,y,tile,True,1)
    # Small rock aprons tie the western map edge into the interior slopes.
    # The capped southern reef also keeps the distant border outside the camera.
    for x, y, w in ((0,2,4),(18,0,5),(0,26,3),(0,43,4),(1,48,4)):
        for dx in range(w):
            edge = 0 if dx == 0 else 2 if dx == w-1 else 1
            for dy, row in enumerate(((0x068,0x069,0x06A),(0x070,0x071,0x072),(0x078,0x079,0x07A))):
                if (x+dx,y+dy) in land:
                    cell(x+dx,y+dy,rock[row[edge]],True)
    # Stone sink and outfall notch in the far cape. Water drops into the
    # cove on the east of town and does not cross any walking route.
    springs = ((39,41),(39,42),(39,43),(40,43),(40,43),(40,43),(40,43))
    pool = {(x, y) for y, (left,right) in enumerate(springs,8) for x in range(left,right+1)}
    paint_river(grid, pool, elevation=1, south_bank=False)
    for y in range(15,20):
        for dx in (0,1):
            index = 0 if y == 15 else 4 if y == 19 else 2
            cell(41+dx,y,falls[index+dx],True,1)

    building("DewfordTown","dewford",7,5,4,4,10,6)
    cell(11,9,bank.tile("dewford",0x225,0x60))
    building("DewfordTown","dewford",16,11,4,4,5,8,True)
    building("DewfordTown","dewford",7,5,4,4,16,7,True)
    building("DewfordTown","dewford",1,7,4,4,7,18)
    building("DewfordTown","dewford",5,13,6,5,20,17,True)
    building("DewfordTown","dewford",7,5,4,4,31,20,True)
    building("DewfordTown","dewford",16,11,4,4,2,22,True)
    # Weathered green slate on the two west cottages, using the same native
    # roof clusters and a separate material ramp above their plaster facades.
    for base_x, base_y in ((5,8),(2,22)):
        for yy in range(base_y, base_y+2):
            for xx in range(base_x, base_x+4):
                old = grid[yy][xx] & 1023
                key = ("heath-slate", old)
                if key not in bank.ids:
                    i = (old-512)*8
                    bank.ids[key] = 512 + len(bank.attrs)
                    bank.meta.extend((p & 0x0FFF) | 0xA000 if p and p >> 12 == 0 else p for p in bank.meta[i:i+8])
                    bank.attrs.append(bank.attrs[old-512])
                grid[yy][xx] = (grid[yy][xx] & ~1023) | bank.ids[key]
    building("DewfordTown","dewford",16,11,4,4,37,21,True)
    building("SlateportCity","slateport",24,32,8,7,28,30,True)
    for dy, row in enumerate(((0x225,0x20D,0x20F,0x227), (0x22D,0x2F8,0x2FA,0x22F),
                              (0x26B,0x28E,0x28E,0x26C))):
        for dx, tile in enumerate(row):
            slate(5+dx,23+dy,tile,True)

    # The cliff path slips behind the upper cottages and rounds the keeper's
    # shoulder, arriving at the original landing beside the lighthouse.
    ridge_walk = {(x, yy) for x, y in ridge for yy in (y, y+1)}
    for x, y in ridge_walk:
        north = (x, y-1) in ridge_walk
        south = (x, y+1) in ridge_walk
        col = 0 if x and (x-1,y) not in ridge_walk else 2 if (x+1,y) not in ridge_walk else 1
        tile = (0x208 if not north else 0x218 if not south else 0x210) + col
        cell(x, y, bank.tile("slateport", tile), elevation=7)
    bank.lighthouse(grid, 29, 4)

    build_harbor(bank, grid, cell, slate, rock, surfaces)

    bank.terrace_fitting(grid,16,23,"fountain")
    bank.terrace_fitting(grid,34,15,"railing",width=5)
    bank.terrace_fitting(grid,16,25,"railing",width=5)
    bank.terrace_fitting(grid,19,27,"drain")
    bank.terrace_fitting(grid,32,17,"drain")
    for x,y in ((12,13),(34,14),(12,24),(10,27),(24,27)):
        for row in range(2):
            ground = grid[y-1+row][x] & 1023
            cell(x, y-1+row, bank.harbor_panel(3,row,ground), row==1)

    # The headland clips the northwest corner of the offshore stack.
    # Carry the bank and its animated water through the open quarters of the rock.
    for dy, row in enumerate(((0x150, 0x151), (0x158, 0x159))):
        for dx, tile in enumerate(row):
            x, y = 43 + dx, 20 + dy
            ground = grid[y][x] & 1023
            if ground != 0x170:
                shore, stack = surfaces.pieces(ground), surfaces.pieces(tile)
                for q in range(4):
                    if shore[q + 4] & 1023:
                        if stack[q + 4] & 1023:
                            raise ValueError(f'Sea stack overlaps the cliff face at {(x, y)}')
                        stack[q], stack[q + 4] = shore[q], shore[q + 4]
                tile = surfaces.variant(tile, stack)
            cell(x, y, tile, True, 1)

    # Stagger small groves on the sheltered shelves; leave the street
    # fronts and the two portal approach tiles visible to an arriving player.
    plant_trees(grid, ((4,0),(16,0),(23,0),(25,1),(27,0),
                       (0,5),(0,30),(1,40),(2,51),(5,52)), pieces=spruce, ground=heath)
    plant_trees(grid, ((14,6),(35,5),(37,7),(38,12)), elevation=7)
    plant_trees(grid, ((3,17),(11,18)), elevation=5)
    for x, y in ((9,10),(14,9),(20,10),(35,11),(6,20),(11,23),(25,21),(29,23)):
        underlays.setdefault((x,y), grid[y][x] & 1023)
        cell(x, y, 0x004, True)
    cell(10,22,0x003,True)
    cell(23,21,0x1CD,True)
    cell(24,23,0x003,True)
    cell(36,25,0x0E2,True)
    close_edge(grid, "left", (8,9,36))
    surfaces.foundations(grid, underlays)
    surfaces.finish(grid, soil)
    build_approaches(ROOT, bank, heath, spruce, rock, surfaces)
    bank.finish()
    install_coast_rock_palette(PRIMARY, COAST)
    layout = ROOT / "data/layouts/Virelia_SeabreakTown"
    write_words(layout / "map.bin", [v for row in grid for v in row])
    write_words(layout / "border.bin", [0x3400 | heath] * 4)
    lighthouse_rooms()


def lighthouse_rooms():
    """Keep the paired stair warps, cabinet, logbook and beacon readable."""
    folder = ROOT / "data/tilesets/secondary/virelia_lighthouse"
    table = words(folder / "metatiles.bin")
    # Both stair landings show the recessed steps, retaining their respective
    # north/south-arrow behaviors. The lower room has a separate two-cell exit rug.
    for target in (0x300, 0x301):
        offset = (target - 512) * 8
        table[offset:offset + 8] = table[0x0B*8:0x0C*8]

    # Weathered oak instead of the original yellow striped floor. This palette
    # belongs only to the lighthouse interior bank.
    palette = (folder / "palettes/06.pal").read_text().splitlines()
    palette[8:12] = ["160 136 112", "180 156 128", "112 96 88", "144 120 104"]
    (folder / "palettes/06.pal").write_text("\n".join(palette) + "\n")

    # The two fixed callback slots hold the unlit and lit Fresnel lens.
    sheet = Image.open(folder / "tiles.png").copy()
    # Quarter the staggered plank joints. The upper pair also backs the stock
    # furniture. The two lower pieces use free graphics just before door VRAM.
    planks = Image.new("P", (16, 16), 5)
    board = ImageDraw.Draw(planks)
    for y, seam in ((0, 3), (8, 12)):
        board.line((0, y+7, 15, y+7), fill=8)
        board.line((seam, y, seam, y+6), fill=8)
        board.line((seam+2, y+1, min(15, seam+5), y+1), fill=6)
        board.point(((seam+9) % 16, y+4), fill=6)
    for n, index in enumerate((0x30, 0x31, 493, 494)):
        x, y = n % 2*8, n//2*8
        sheet.paste(planks.crop((x, y, x+8, y+8)), (index % 16*8, index//16*8))
        table[0x23*8+n] = 0x6000 | (512+index)
    write_words(folder / "metatiles.bin", table)
    for state in (0, 1):
        lens = Image.new("P", (16, 16), 1)
        d = ImageDraw.Draw(lens)
        d.rectangle((1, 1, 14, 14), outline=4)
        d.rectangle((3, 2, 12, 3), fill=5)
        d.ellipse((3, 3, 12, 12), fill=11 if state else 13, outline=2)
        d.ellipse((5, 4, 10, 10), fill=12 if state else 14)
        d.line((5, 6, 10, 6), fill=7 if state else 15)
        d.line((4, 9, 11, 9), fill=7 if state else 13)
        d.rectangle((4, 12, 11, 13), fill=3)
        for n in range(4):
            index = 480 + 4*state + n
            x, y = n % 2*8, n//2*8
            sheet.paste(lens.crop((x, y, x+8, y+8)), (index % 16*8, index//16*8))
    sheet.save(folder / "tiles.png")

    def room(name, width, height, spans, fixtures):
        # Building 001 is opaque black (slot 5, color 15), as in the keeper's
        # home. Transparent 000 would expose the GBA backdrop color instead.
        void = 0x0401
        grid = [[void] * width for _ in range(height)]
        for y, left, right in spans:
            for x in range(left, right+1):
                grid[y][x] = 0x3223
                if y == 1 or grid[y-1][x] == void:
                    grid[y][x] = 0x3605
        for x, y, tile, solid in fixtures:
            grid[y][x] = 0x3000 | (0x400 if solid else 0) | tile
        layout = ROOT / ("data/layouts/Virelia_Seabreak_" + name)
        write_words(layout / "map.bin", [v for row in grid for v in row])
        write_words(layout / "border.bin", [void] * 4)

    # The corners of both floors step inward around the masonry tower wall.
    room("Lighthouse", 16, 14,
         [(y, 4 if y in (1, 12) else 2 if y in (2, 11) else 1,
           11 if y in (1, 12) else 13 if y in (2, 11) else 14) for y in range(1, 13)],
         (
             (5, 2, 0x205, True), (5, 3, 0x268, True),    # power cabinet
             (9, 2, 0x205, True), (9, 3, 0x207, True),    # framed operating instructions
             (11, 3, 0x20A, True), (12, 3, 0x300, False),
             (3, 3, 0x281, True), (4, 3, 0x281, True),
             (12, 8, 0x22A, True), (13, 8, 0x268, True),
             (7, 12, 0x208, False), (8, 12, 0x209, False),
         ))
    room("LanternRoom", 13, 12,
         [(y, 4 if y in (1, 10) else 2 if y in (2, 9) else 1,
           8 if y in (1, 10) else 10 if y in (2, 9) else 11) for y in range(1, 11)],
         (
             (5, 1, 0x206, True), (7, 1, 0x206, True),
             (6, 2, 0x20C, True),                      # switched Fresnel lens
             (3, 5, 0x268, True), (6, 5, 0x22A, True), (9, 5, 0x004, True),
             (9, 8, 0x2D1, True),                      # open shipping register
             (5, 10, 0x20A, True), (6, 10, 0x301, False),
         ))


if __name__ == "__main__":
    main()
