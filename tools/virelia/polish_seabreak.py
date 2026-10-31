#!/usr/bin/env python3
"""Polish Seabreak using complete stock Emerald assemblies, without drawing art.

Only the SeabreakTown blockmap and its dedicated coast bank are written. Map
metadata, warps, scripts, actors and interaction coordinates stay unchanged.
The existing lighthouse sprite is retained as an acknowledged art placeholder.
Run after build_rainfall_seabreak.py; never regenerate hand-edited maps blindly.
"""

from pathlib import Path
from functools import lru_cache
import json
import struct
from PIL import Image
from polish_rainfall import Terrain
from native_terrain import paint_rock_outcrop

ROOT = Path(__file__).resolve().parents[2]
COAST = ROOT / "data/tilesets/secondary/virelia_coast"
PRIMARY = ROOT / "data/tilesets/primary/general"
BASE_COUNT = 0x39


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
    return [data[y * layout["width"] : (y + 1) * layout["width"]] for y in range(layout["height"])]


class StockBank:
    """Copy existing indexed pixels, flips, palettes and behavior as complete units."""

    def __init__(self):
        original = words(COAST / "metatiles.bin")[: BASE_COUNT * 8]
        self.meta = original[:]
        self.attrs = words(COAST / "metatile_attributes.bin")[:BASE_COUNT]
        image = Image.open(COAST / "tiles.png").copy()
        count = max((v & 1023) - 512 for v in original if (v & 1023) >= 512) + 1
        self.tiles = [
            image.crop((i % 16 * 8, i // 16 * 8, i % 16 * 8 + 8, i // 16 * 8 + 8)).tobytes()
            for i in range(count)
        ]
        self.lookup = {value: 512 + i for i, value in enumerate(self.tiles)}
        self.ids = {}
        self.palette = image.getpalette()
        # Replace the old featureless exit marker with stock path pixels. The
        # north-arrow behavior and existing map ID are deliberately unchanged.
        self.meta[(0x238 - 512) * 8 : (0x239 - 512) * 8] = words(PRIMARY / "metatiles.bin")[
            0x121 * 8 : 0x122 * 8
        ]

    def tile(self, bank, identifier, behavior=None):
        if identifier < 512:
            return identifier
        key = (bank, identifier, behavior)
        if key in self.ids:
            return self.ids[key]
        path, meta, attrs, image = source(bank)
        copied = []
        for value in meta[(identifier - 512) * 8 : (identifier - 511) * 8]:
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
                if not any(pixels):
                    replacement = 0
                else:
                    if pixels not in self.lookup:
                        self.lookup[pixels] = 512 + len(self.tiles)
                        self.tiles.append(pixels)
                    replacement = self.lookup[pixels]
                value = (value & ~1023) | replacement
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
            f"Coast stock pack: {len(self.attrs)} metatiles, {len(self.tiles)}/496 graphics tiles; palette bytes unchanged"
        )


def main():
    bank = StockBank()
    width, height = 48, 42
    grid = [[0x3570 for x in range(width)] for y in range(height)]
    land = set()

    def cell(x, y, tile, solid=False):
        if 0 <= x < width and 0 <= y < height:
            grid[y][x] = 0x3000 | (0x400 if solid else 0) | tile

    def rect(x, y, w, h, tile=1, solid=False):
        for yy in range(y, y + h):
            for xx in range(x, x + w):
                cell(xx, yy, tile, solid)

    def island(x, y, w, h):
        land.update((xx, yy) for yy in range(y, y + h) for xx in range(x, x + w))

    # A village shore, open harbor basin and a separate lighthouse headland.
    island(0, 0, 33, 27)
    island(0, 26, 11, 5)
    island(10, 25, 15, 8)
    island(24, 22, 10, 6)
    island(31, 22, 14, 5)
    island(34, 26, 11, 12)
    for x, y in sorted(land):
        n, s, w, e = [(x + dx, y + dy) in land for dx, dy in [(0, -1), (0, 1), (-1, 0), (1, 0)]]
        # Native sand/water edges, including concave beach corners.
        if not n and not w:
            tile = 0x11B
        elif not n and not e:
            tile = 0x11D
        elif not s and not w:
            tile = 0x12B
        elif not s and not e:
            tile = 0x12D
        elif not n:
            tile = 0x11C
        elif not s:
            tile = 0x12C
        elif not w:
            tile = 0x123
        elif not e:
            tile = 0x125
        elif (x - 1, y - 1) not in land:
            tile = 0x13D
        elif (x + 1, y - 1) not in land:
            tile = 0x13E
        elif (x - 1, y + 1) not in land:
            tile = 0x145
        elif (x + 1, y + 1) not in land:
            tile = 0x146
        else:
            tile = 0x124
        cell(x, y, tile)
    # The town grows from the green pass into a sheltered sandy waterfront.
    rect(0, 0, 32, 22, 1)
    for x in range(0, 32):
        cell(x, 22, 0x119)
    for y in range(0, 22):
        cell(32, y, 0x075, True)
        cell(33, y, 0x190, True)
    cell(32, 22, 0x07D, True)
    cell(33, 22, 0x198, True)

    # Complete native Slateport paths, with the matching rounded grassy edge.
    paths = set()

    def road(x, y, w, h):
        paths.update((xx, yy) for yy in range(y, y + h) for xx in range(x, x + w))

    road(13, 0, 3, 27)
    road(7, 14, 22, 3)
    road(22, 8, 3, 7)
    road(20, 15, 3, 8)
    road(7, 20, 22, 3)
    road(26, 19, 3, 6)
    for x, y in paths:
        n, s, w, e = [(x + dx, y + dy) in paths for dx, dy in [(0, -1), (0, 1), (-1, 0), (1, 0)]]
        if not n:
            tile = 0x208 if not w else 0x20A if not e else 0x209
        elif not s:
            tile = 0x218 if not w else 0x21A if not e else 0x219
        else:
            tile = 0x210 if not w else 0x212 if not e else 0x211
        cell(x, y, bank.tile("slateport", tile))

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
    # The fixed sailor stands south of the complete facade. Its entrance is
    # decorative; this milestone has no shipyard interior.
    # Public harbor promenade: all story actors share a wide unobstructed stage.
    rect(10, 26, 14, 6, bank.tile("slateport", 0x202))
    for x in range(10, 24):
        cell(x, 32, bank.tile("slateport", 0x25D), True)
    cell(9, 32, bank.tile("slateport", 0x2FE), True)
    cell(24, 32, bank.tile("slateport", 0x2FF), True)
    for y in range(27, 32):
        cell(9, y, bank.tile("slateport", 0x234), True)
        cell(24, y, bank.tile("slateport", 0x23C), True)
    for x in range(10, 24):
        cell(x, 33, 0x189, True)
    cell(9, 33, 0x188, True)
    cell(24, 33, 0x198, True)

    # Native wooden pier, visible below the fishermen rather than hidden layers.
    for y in range(29, 36):
        for x in range(5, 9):
            cell(x, y, bank.tile("dewford", 0x228 if x == 5 else 0x229))
    for x in range(5, 9):
        cell(x, 36, bank.tile("dewford", 0x230 if x == 5 else 0x231))
    # A small native rowing boat and a full mast/sail/hull assembly.
    rect(1, 27, 3, 3, 1)
    assembly("SlateportCity", "slateport", 21, 35, 3, 2, 1, 28, True)
    assembly("SlateportCity", "slateport", 33, 35, 3, 4, 26, 31, True)
    # Two source bounding-box cells are the dock behind the mast, not boat art.
    cell(26, 31, 0x170, True)
    cell(27, 31, 0x170, True)
    assembly("SlateportCity", "slateport", 36, 37, 3, 2, 17, 35, True)

    # Headland: native grass, a south-facing two-row rock coast, lighthouse kept.
    rect(35, 23, 9, 13, 1)
    for y in range(26, 36):
        cell(34, y, 0x17A, True)
        cell(44, y, 0x075, True)
        cell(45, y, 0x190, True)
    for x in range(35, 44):
        cell(x, 36, 0x07C, True)
        cell(x, 37, 0x189, True)
    cell(34, 36, 0x07B, True)
    cell(34, 37, 0x188, True)
    cell(44, 36, 0x07D, True)
    cell(44, 37, 0x198, True)
    # Keep the existing 4x8 tower pixels. A stock-art replacement is still needed.
    for dy in range(8):
        for dx in range(4):
            cell(36 + dx, 24 + dy, 0x20E + dy * 4 + dx, True)
    cell(37, 31, 0x22B)
    # Readable approach bends around the west side to the actual south door.
    for y in range(23, 33):
        for x in (35,):
            cell(x, y, 0x121)
    for y in range(32, 35):
        for x in range(35, 43):
            cell(x, y, 0x121)
    for x in range(29, 36):
        for y in range(23, 26):
            cell(x, y, bank.tile("slateport", 0x202))
    # Native cliff fragments and a pale stone mark form the coastal viewpoint.
    cell(41, 35, 0x0E2, True)
    cell(42, 35, 0x004, True)

    # Petalburg's native overlapping inner/outer canopy pieces form connected
    # forest masses. Protect every building/path; do not stamp isolated trees.
    forest = {(x, y) for y in range(0, 23) for x in range(0, 4)}
    forest.update((x, y) for y in range(0, 4) for x in range(0, 12))
    forest.update((x, y) for y in range(0, 3) for x in range(17, 32))
    forest.update((x, y) for y in range(2, 20) for x in range(30, 32))
    forest.update((x, y) for y in range(21, 29) for x in range(40, 44))
    protect = paths | {(x, y) for y in range(4, 8) for x in range(5, 10)}
    protect.update((x, y) for y in range(16, 23) for x in range(25, 33))
    forest -= protect
    terrain = Terrain("SeabreakTown", width, height)
    terrain.g = grid
    terrain.land = {(x, y) for y in range(height) for x in range(width)} - forest
    terrain.path = paths | {(14, 1)}
    terrain.forest()
    # Rocky north-east shoulder keeps the headland distinct from the town.
    paint_rock_outcrop(
        grid,
        {
            (x, y)
            for y in range(19, 23)
            for x in range(35, 42)
            if not (y in (19, 22) and x in (35, 41))
        },
        elevation=3,
    )
    # Small coast gardens use complete two-row native shrubs rather than filler.
    for x, y in [(11, 10), (18, 17), (25, 14)]:
        cell(x, y, 0x016, True)
        cell(x + 1, y, 0x017, True)
        cell(x, y + 1, 0x01E, True)
        cell(x + 1, y + 1, 0x01F, True)
    # Coastal rocks and small planted corners give each plot an edge.
    for x, y in [(3, 25), (1, 25), (10, 24)]:
        cell(x, y, 0x0E2, True)
    for x, y in [(5, 8), (9, 8), (25, 14), (28, 14), (18, 20)]:
        cell(x, y, 0x004, True)
    for x, y in [(11, 27), (22, 27), (22, 30)]:
        cell(x, y, bank.tile("slateport", 0x238), True)
    for x, y in [(20, 27), (20, 28), (23, 30), (25, 24), (29, 24)]:
        cell(x, y, bank.tile("slateport", 0x336), True)

    # Restore every fixed gameplay anchor after decorative passes.
    cell(14, 1, 0x238)
    cell(14, 2, bank.tile("slateport", 0x211))
    cell(10, 15, 0x003, True)
    # The GymClosed background event is on its original blocked door pixels.
    cell(23, 7, 0x1CD, True)
    cell(23, 11, 0x003, True)
    cell(6, 31, bank.tile("slateport", 0x238), True)
    actors = [(14, 28), (7, 35), (20, 15), (17, 29), (15, 29), (12, 29), (27, 23)]
    for x, y in actors:
        if y >= 23 and x >= 10:
            tile = bank.tile("slateport", 0x202)
        elif y == 15:
            tile = bank.tile("slateport", 0x211)
        else:
            tile = bank.tile("dewford", 0x229)
        cell(x, y, tile)
    # The fixed sailor stands on the cargo yard south of the intact depot.
    rect(25, 23, 4, 3, bank.tile("slateport", 0x202))
    cell(27, 23, bank.tile("slateport", 0x202))
    # The town map's metadata and event coordinates are intentionally untouched.
    bank.finish()
    write_words(
        ROOT / "data/layouts/Virelia_SeabreakTown/map.bin",
        [value for row in grid for value in row],
    )
    print("Seabreak map written: 48x42, all 4 warps / 7 NPCs / 5 background events unchanged")


if __name__ == "__main__":
    main()
