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
from native_terrain import plant_trees

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
        self.lookup = {}
        self.ids = {}
        self.palette = source("slateport")[3].getpalette()

    def graphic(self, pixels):
        if not any(pixels):
            return 0
        if pixels not in self.lookup:
            self.lookup[pixels] = 512 + len(self.tiles)
            self.tiles.append(pixels)
        return self.lookup[pixels]

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
            if palette >= 6:
                # Selected stock colors must already fit the coast's slots.
                assert (path / f"palettes/{palette:02}.pal").read_bytes() == (
                    COAST / f"palettes/{palette:02}.pal"
                ).read_bytes(), (bank, hex(identifier), palette)
            index = value & 1023
            if index >= 512:
                index -= 512
                pixels = image.crop(
                    (index % 16 * 8, index // 16 * 8, index % 16 * 8 + 8, index // 16 * 8 + 8)
                ).tobytes()
                value = (value & ~1023) | self.graphic(pixels)
            copied.append(value)
        result = 512 + len(self.attrs)
        self.ids[key] = result
        self.meta.extend(copied)
        attribute = attrs[identifier - 512]
        self.attrs.append(attribute if behavior is None else (attribute & ~0x1FF) | behavior)
        return result

    def pier(self, side, end):
        # Route 104's timber boards use seven colors. Keep the existing
        # dock-palette entries 0..7; its upper half can hold all seven exactly.
        key = ("pier", side, end)
        if key in self.ids:
            return self.ids[key]
        folder, metas, _, sheet = source("rustboro")
        src = ((0x346, 0x347) if end else (0x34E, 0x34F))[side] - 512
        old_palette = (folder / "palettes/11.pal").read_text().splitlines()[3:]
        coast_palette = (COAST / "palettes/12.pal").read_text().splitlines()
        indices = (5, 6, 8, 11, 12, 13, 14)
        coast_palette[11:18] = [old_palette[i] for i in indices]
        (COAST / "palettes/12.pal").write_text("\n".join(coast_palette) + "\n")
        translate = bytes(0 if i not in indices else 8 + indices.index(i) for i in range(256))
        pieces = words(PRIMARY / "metatiles.bin")[0x170*8:0x170*8+4]
        for value in metas[src*8+4:src*8+8]:
            index = (value & 1023) - 512
            x, y = index % 16*8, index//16*8
            pixels = sheet.crop((x, y, x+8, y+8)).tobytes().translate(translate)
            pieces.append((12 << 12) | (value & 0xC00) | self.graphic(pixels))
        identifier = 512 + len(self.attrs)
        self.ids[key] = identifier
        self.meta.extend(pieces)
        self.attrs.append(0x1000)
        return identifier

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
    bank = StockBank()
    width, height = 48, 42
    grid = [[0x3570 for x in range(width)] for y in range(height)]
    land = set()

    def cell(x, y, tile, solid=False):
        if 0 <= x < width and 0 <= y < height:
            grid[y][x] = 0x3000 | (0x400 if solid else 0) | tile

    # The north pass widens into town, then breaks into an inlet with a separate
    # rocky point, a narrow neck behind the shipyard and a sheltered harbor.
    def shore(top, bottom, left, right):
        land.update((x, y) for y in range(top, bottom + 1) for x in range(left, right + 1))

    for rows in (
        (0, 0, 8, 25), (1, 1, 6, 28), (2, 3, 4, 30), (4, 6, 2, 30),
        (7, 9, 2, 32), (10, 12, 1, 31), (13, 14, 2, 32),
        (15, 17, 3, 33), (18, 20, 2, 33), (21, 22, 3, 34),
        (23, 24, 3, 36), (25, 25, 4, 35), (26, 26, 5, 30),
        (27, 28, 5, 24), (29, 31, 5, 23), (32, 32, 9, 23),
        (18, 18, 40, 42), (19, 20, 38, 44), (21, 21, 37, 45),
        (22, 25, 35, 45), (26, 29, 34, 46), (30, 33, 34, 45),
        (34, 35, 35, 44), (36, 36, 36, 43), (37, 37, 38, 42),
    ):
        shore(*rows)
    for y in range(height):
        for x in range(width):
            n, w, e = (x, y-1) in land, (x-1, y) in land, (x+1, y) in land
            if (x, y) in land:
                cell(x, y, 0x001 if n else 0x002, y == 0)
            else:
                if n:
                    tile = 0x188 if w else 0x18A if e else 0x189
                elif w or e:
                    tile = 0x190 if w else 0x192
                elif (x-1, y-1) in land:
                    tile = 0x198
                elif (x+1, y-1) in land:
                    tile = 0x19A
                else:
                    tile = 0x170
                cell(x, y, tile, True)

    def slate(x, y, source_tile, solid=False):
        cell(x, y, bank.tile("slateport", source_tile), solid)

    paths = set()

    def road(x, y, w, h):
        paths.update((xx, yy) for yy in range(y, y+h) for xx in range(x, x+w))

    road(13, 1, 3, 24)
    road(7, 7, 17, 2)
    road(22, 7, 3, 10)
    road(7, 14, 22, 3)
    road(7, 13, 3, 2)
    road(13, 20, 10, 3)
    road(5, 19, 5, 4)
    road(5, 21, 10, 2)
    road(35, 23, 2, 11)
    road(35, 32, 8, 2)
    for x, y in sorted(paths):
        n, s, w, e = ((x+dx, y+dy) in paths for dx, dy in ((0, -1), (0, 1), (-1, 0), (1, 0)))
        if not n:
            tile = 0x208 if not w else 0x20A if not e else 0x209
        elif not s:
            tile = 0x218 if not w else 0x21A if not e else 0x219
        else:
            tile = 0x210 if not w else 0x212 if not e else 0x211
        slate(x, y, tile)

    def assembly(map_name, source_bank, sx, sy, w, h, x, y, decorative=False):
        source_map = stock_grid(map_name)
        for dy in range(h):
            for dx in range(w):
                raw = source_map[sy + dy][sx + dx]
                original = raw & 1023
                tile = bank.tile(source_bank, original)
                grid[y + dy][x + dx] = 0x3000 | (raw & 0xC00) | tile
        if decorative:
            # Unenterable backdrop homes retain complete stock doors visually.
            for xx in range(x, x + w):
                grid[y + h - 1][xx] |= 0x400

    # Correct red Center with its original primary-General animated door (061).
    assembly("DewfordTown", "dewford", 1, 7, 4, 4, 7, 10)
    # Blue-tiled coastal homes; the keeper's entrance remains exactly 21,20.
    assembly("DewfordTown", "dewford", 7, 5, 4, 4, 20, 17)
    cell(21, 20, bank.tile("dewford", 0x225, 0x60))
    assembly("DewfordTown", "dewford", 7, 5, 4, 4, 5, 4, True)
    assembly("DewfordTown", "dewford", 16, 11, 4, 4, 25, 10, True)
    # Closed future stadium: the complete native Dewford facade has no warp.
    # Its door is the existing GymClosed interaction at (23,7); the approach
    # notice remains (23,11). Both are reachable from the public street.
    assembly("DewfordTown", "dewford", 5, 13, 6, 5, 20, 3, True)

    # A compact working shipyard overlooks a genuine native stone quay.
    assembly("SlateportCity", "slateport", 24, 32, 8, 7, 25, 16, True)

    # A covered fish stall opens onto the lane above the harbor steps.
    for dy, row in enumerate((
        (0x225, 0x20D, 0x20F, 0x227),
        (0x22D, 0x2F8, 0x2FA, 0x22F),
        (0x26B, 0x28E, 0x28E, 0x26C),
    )):
        for dx, tile in enumerate(row):
            slate(5+dx, 19+dy, tile, True)

    # Two public terraces descend to the quay. A stair breaks the wall on
    # each approach, including the closed Gym door and the central avenue.
    def terrace(y, left, right, stairs):
        for x in range(left, right + 1):
            tile = 0x078 if x == left else 0x07A if x == right else 0x079
            cell(x, y, tile, True)
        for x in stairs:
            cell(x, y, 0x0AF)
            cell(x + 1, y, 0x0CF)

    terrace(8, 3, 29, (13, 22))
    terrace(23, 4, 24, (13, 21))

    # Quay copings follow the stepped floor on the seaward sides; the west
    # opening is the timber fishing pier, the east opens into the cargo yard.
    quay = set()
    for top, bottom, left, right in ((24, 25, 5, 34), (26, 26, 5, 30), (27, 28, 5, 24),
                                     (29, 31, 5, 23), (32, 32, 9, 23)):
        quay.update((x, y) for y in range(top, bottom+1) for x in range(left, right+1))
    for x, y in sorted(quay):
        w, e, s = (x-1, y) in land, (x+1, y) in land, (x, y+1) in land
        if not s:
            t = 0x2FE if not w else 0x2FF if not e else 0x25D
        else:
            t = 0x234 if not w else 0x23C if not e else 0x202
        slate(x, y, t, not w or not e or not s)
    for x in range(25, 35):
        slate(x, 23, 0x202)
    for y in range(24, 26):
        slate(35, y, 0x202)
    for x in (7, 8):
        slate(x, 31, 0x202)
    for y in range(32, 37):
        for dx in (0, 1):
            cell(7+dx, y, bank.pier(dx, y == 36))
    slate(6, 31, 0x238, True)
    for x, y in ((5, 26), (22, 30), (20, 27), (19, 27), (28, 24)):
        slate(x, y, 0x336, True)
    for x, y in ((5, 28), (10, 31), (23, 29)):
        slate(x, y, 0x238, True)
    # Open-water mooring alongside the fishing jetty.
    assembly("SlateportCity", "slateport", 34, 44, 3, 3, 1, 29, True)
    # Whole floating sailing boats, copied from the open berths rather than
    # the row of quay/coping behind the taller mast.
    assembly("SlateportCity", "slateport", 34, 44, 3, 3, 17, 35, True)
    assembly("SlateportCity", "slateport", 33, 36, 3, 3, 26, 30, True)
    cell(26, 30, 0x170, True)
    # A few wave-breaking reefs leave clear water between the lighthouse
    # and the fishing berths. Each is a full native four-quadrant assembly.
    for x, y in ((3, 34), (30, 37), (42, 9), (46, 17), (1, 7)):
        if all((x+dx, y+dy) not in land and grid[y+dy][x+dx] & 1023 == 0x170
               for dx in (0, 1) for dy in (0, 1)):
            for dy, row in enumerate(((0x150, 0x151), (0x158, 0x159))):
                for dx, t in enumerate(row):
                    cell(x+dx, y+dy, t, True)

    # Exposed stone along the south rim lifts the headland above its wave foot.
    for y, left, right in ((35, 35, 44), (36, 36, 43), (37, 38, 42)):
        for x in range(left, right+1):
            if (x, y+1) not in land:
                cell(x, y, 0x07B if x == left else 0x07D if x == right else 0x07C, True)
    bank.lighthouse(grid, 35, 22)
    cell(41, 35, 0x0E2, True)

    plant_trees(grid, (
        (9, 1), (17, 1), (26, 2), (28, 5), (4, 9), (4, 11),
        (16, 10), (29, 12), (5, 16), (10, 18), (17, 17),
        (40, 20), (43, 23), (41, 27),
    ), elevation=3)

    for x, y in ((11, 6), (19, 9), (18, 12), (19, 18), (42, 34), (10, 17)):
        cell(x, y, 0x004, True)
    for x, y in ((3, 13), (30, 9), (43, 21), (44, 30)):
        cell(x, y, 0x0E2, True)
    cell(14, 1, bank.tile("slateport", 0x211, 0x64))
    # The ledge frames a visible, recessed entrance instead of ending a street
    # abruptly at the map border. Enter from the southern cobbles.
    for x in range(10, 20):
        cell(x, 0, 0x078 if x == 10 else 0x07A if x == 19 else 0x079, True)
    cell(14, 0, 0x0A7, True)
    cell(10, 15, 0x003, True)
    cell(23, 7, 0x1CD, True)
    cell(23, 11, 0x003, True)
    bank.finish()
    layout = ROOT / "data/layouts/Virelia_SeabreakTown"
    write_words(layout / "map.bin", [block for row in grid for block in row])
    write_words(layout / "border.bin", [0x3570]*4)
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
