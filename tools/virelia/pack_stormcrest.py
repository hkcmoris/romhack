"""Pack the native-size Stormcrest pass and fen art into the Rainfall secondary bank."""

import struct
from pathlib import Path
from cliff_transitions import pack_ridge_contact

from PIL import Image


FIRST = 0x341
LAST = 0x3BF


def words(path):
    return [value for value, in struct.iter_unpack("<H", path.read_bytes())]


def spruce_panels(fen):
    """Six panels for a standing spruce, then the two panels shared by stacked trees."""
    tree = fen.crop((64, 32, 96, 80))
    for y in range(0, 48, 16):
        for x in (0, 16):
            yield tree.crop((x, y, x + 16, y + 16))
    for x in (0, 16):
        base = tree.crop((x, 32, x + 16, 48))
        crown = tree.crop((x, 0, x + 16, 16))
        mask = Image.frombytes("L", crown.size, crown.tobytes()).point(lambda n: 255 if n else 0)
        base.paste(crown, mask=mask)
        yield base


def write_palette(path, sheet):
    colors = struct.iter_unpack("3B", bytes(sheet.getpalette()[:48]))
    lines = ["JASC-PAL", "0100", "16", *(f"{r} {g} {b}" for r, g, b in colors)]
    path.write_text("\n".join(lines) + "\n", encoding="ascii")


def pack_stormcrest(root):
    bank = root / "data/tilesets/secondary/virelia_rainfall"
    ridge = Image.open(bank / "stormcrest.png")
    fen = Image.open(bank / "fen.png")
    falls = Image.open(bank / "cascade.png")
    summit = Image.open(bank / "summit.png")
    for sheet, size in ((ridge, (128, 80)), (fen, (96, 160)), (falls, (48, 48)), (summit, (80, 80))):
        if sheet.mode != "P" or sheet.size != size or sheet.getextrema()[1] > 15:
            raise ValueError(f"Expected indexed 4bpp artwork of size {size} in {sheet.filename}")

    general_bank = root / "data/tilesets/primary/general"
    water_colors = (general_bank / "palettes/04.pal").read_text().splitlines()[3:19]
    if falls.getpalette()[:48] != [int(channel) for color in water_colors for channel in color.split()]:
        raise ValueError("Cascade art must use the General animated-water palette")
    primary = words(general_bank / "metatiles.bin")
    definitions = words(bank / "metatiles.bin")
    attributes = words(bank / "metatile_attributes.bin")
    start, stop = FIRST - 512, LAST + 1 - 512
    if len(attributes) < start or len(definitions) != 8 * len(attributes):
        raise ValueError("Pack the marsh and trail corners before the Stormcrest kit")
    definitions.extend([0] * max(0, 8 * stop - len(definitions)))
    attributes.extend([0] * max(0, stop - len(attributes)))
    sheet = Image.open(bank / "tiles.png").copy()

    # The original bank is nearly full on disk but many 8x8 graphics have no
    # metatile references. Allocate deterministically around the earlier art.
    # 318..31F were blank Fortree editor slots. The pass floor kit below
    # owns them, so their previous graphics may be reclaimed on a repack.
    floor_start, floor_stop = (0x318 - 512) * 8, (0x320 - 512) * 8
    cliff_slots = {0x2FD, 0x305, 0x30D, 0x310, 0x311, 0x312, 0x315}
    earlier = [v for i, v in enumerate(definitions[:start * 8])
               if not floor_start <= i < floor_stop and 512 + i // 8 not in cliff_slots]
    if any(v >> 12 in (6, 7) for v in earlier):
        raise ValueError("Stormcrest needs unused Rainfall palette slots 6 and 7")
    # 385..386 retain the route aliases. The toe of the ridge contact uses
    # the former blank slots on either side of this pair.
    gap = slice((0x385 - 512) * 8, (0x387 - 512) * 8)
    old_kit = definitions[start * 8:gap.start] + definitions[gap.stop:stop * 8]
    external = definitions[gap] + definitions[stop * 8:]
    independently_used = {v & 1023 for v in external} - {v & 1023 for v in old_kit}
    used = {v & 1023 for v in earlier} | independently_used
    free = iter(i for i in range(512, 1008) if i not in used)
    lookup = {}

    # Summit art fills seven previously unused colors in Fortree's slot 11.
    # Check the pixels behind every existing reference before editing its palette.
    original = (root / "data/tilesets/secondary/fortree/palettes/11.pal").read_text().splitlines()[3:19]
    summit_colors = list(struct.iter_unpack("3B", bytes(summit.getpalette()[:48])))
    changed = {i for i, color in enumerate(original) if tuple(map(int, color.split())) != summit_colors[i]}
    general = Image.open(general_bank / "tiles.png")
    for index in {v & 1023 for v in earlier if v >> 12 == 11}:
        graphics = general if index < 512 else sheet
        x, y = index % 16 * 8, index % 512 // 16 * 8
        if changed.intersection(graphics.crop((x, y, x + 8, y + 8)).tobytes()):
            raise ValueError("Summit palette would recolor an existing Rainfall metatile")

    def quarters(tile, palette):
        result = []
        for q in range(4):
            x, y = (q % 2) * 8, (q // 2) * 8
            pixels = tile.crop((x, y, x + 8, y + 8))
            data = pixels.tobytes()
            if not any(data):
                result.append(0)
                continue
            if data not in lookup:
                index = next(free, None)
                if index is None:
                    raise ValueError("Stormcrest exceeds the Rainfall graphics capacity")
                col, row = index % 16 * 8, (index - 512) // 16 * 8
                sheet.paste(pixels, (col, row))
                lookup[data] = index
                lookup.setdefault(pixels.transpose(Image.Transpose.FLIP_LEFT_RIGHT).tobytes(), index | 0x400)
                lookup.setdefault(pixels.transpose(Image.Transpose.FLIP_TOP_BOTTOM).tobytes(), index | 0x800)
                lookup.setdefault(pixels.transpose(Image.Transpose.ROTATE_180).tobytes(), index | 0xC00)
            result.append((palette << 12) | lookup[data])
        return result

    def art(image, col, row):
        return image.crop((16 * col, 16 * row, 16 * col + 16, 16 * row + 16))

    def ground(tile):
        return primary[tile * 8:tile * 8 + 4]

    def define(tile, bottom, top, attr=0x1000):
        offset = tile - 512
        definitions[offset * 8:(offset + 1) * 8] = bottom + top
        attributes[offset] = attr

    grass = ground(1)
    empty = [0, 0, 0, 0]
    # In a single-cell tidal channel, both banks must be present. At its
    # head, turn the same two corner pieces inward to close the waterline.
    for tile, capped in ((0x2CB, False), (0x2D3, True)):
        parts = []
        for layer in (0,4):
            for q in range(4):
                right = q % 2
                if capped and q >= 2:
                    source = (0x188, 0x18A)[right]
                    value = primary[source*8 + layer + right] ^ 0x800
                else:
                    source = (0x190, 0x192)[right]
                    value = primary[source*8 + layer + q]
                parts.append(value)
        define(tile, parts[:4], parts[4:], 0x1015)
    for i in range(9):
        define(0x341 + i, grass, quarters(art(ridge, i % 3, i // 3), 6))
    for i in range(2):
        define(0x34A + i, grass, quarters(art(ridge, 6 + i, 0), 6))

    # Stock General animates the lip and downstroke in VRAM. Custom rock,
    # whitewater and bridge rails occupy the foreground of the same block.
    for i in range(9):
        row, col = divmod(i, 3)
        water = ground((0x0B3, 0x0B9, 0x170)[row])
        attr = 0x1013 if row < 2 else 0x1010  # Waterfall above the plunge pool
        define(0x34C + i, water, quarters(art(falls, col, row), 4), attr)
    rock = quarters(art(ridge, 1, 1), 6)
    for i in range(6):
        row, col = divmod(i, 3)
        bridge = art(ridge, col, row + 3)
        define(0x355 + i, rock if col == 1 else grass, quarters(bridge, 6))
        bridge = art(ridge, 1, row + 3)
        behind = art(falls, col, 1).point([13] + list(range(1, 256)))
        define(0x35B + i, quarters(behind, 4), quarters(bridge, 6))
        define(0x361 + i, grass, quarters(art(ridge, col + 3, row + 3), 6),
               0 if row == 0 else 0x1000)

    peat = quarters(art(fen, 0, 0), 7)
    # Where the outcrop meets the fen, show dark water through the same
    # cut stone edge. Other uses of these crags retain their grass backing.
    for exposed, original in ((0x2FD, 0x347), (0x305, 0x349), (0x30D, 0x344)):
        i = (original - 512) * 8
        define(exposed, peat, definitions[i+4:i+8])
    for i in range(6):
        define(0x367 + i, quarters(art(fen, i % 3, i // 3), 7), empty, 0x1010)
    for i in range(3):
        define(0x36D + i, grass, quarters(art(fen, 3 + i, 0), 7),
               0x1002 if i < 2 else 0x1000)
    for i in range(6):
        define(0x370 + i, peat, quarters(art(fen, i % 3, i // 3 + 2), 7))
    define(0x376, peat, quarters(art(fen, 3, 2), 7))
    for i in range(9):
        define(0x377 + i, grass, quarters(art(fen, i % 3, 4 + i // 3), 7), 0x1010)
    for i in range(4):
        define(0x380 + i, grass, quarters(art(fen, i % 2, 7 + i // 2), 7), 0x1010)

    for i, panel in enumerate(spruce_panels(fen)):
        define(0x389 + i, grass, quarters(panel, 7), 0)
    for i in range(9):
        define(0x391 + i, grass, quarters(art(fen, 3 + i % 3, 5 + i // 3), 7))
    for i in range(4):
        define(0x39A + i, grass, quarters(art(fen, 3 + i % 2, 8 + i // 2), 7))
    moss = quarters(art(fen, 4, 6), 7)
    for i in range(3):
        define(0x39E + i, moss, quarters(art(fen, 3 + i, 0), 7), 0x1002 if i < 2 else 0x1000)
    for i in range(4):
        define(0x3A1 + i, grass, quarters(art(ridge, 6 + i % 2, 1 + i // 2), 6))
    define(0x3A5, quarters(art(summit, 3, 0), 11), empty)
    fog = art(summit, 4, 0)
    for i, edge in enumerate((fog, fog.transpose(Image.Transpose.FLIP_TOP_BOTTOM),
                              fog.transpose(Image.Transpose.ROTATE_270), fog.transpose(Image.Transpose.ROTATE_90))):
        define(0x3A6 + 2 * i, grass, quarters(edge, 11))
        define(0x3A7 + 2 * i, rock, quarters(edge, 11))

    for i in range(9):
        define(0x3AE + i, grass, quarters(art(summit, i % 3, i // 3), 11))
    # One-metatile tracks show both grassy shoulders. The boardwalk starts
    # over dirt and crosses a bank beneath its planks before reaching peat.
    thin_h = ground(0x119)[:2] + ground(0x129)[2:]
    thin_v = [ground(0x120 if q % 2 == 0 else 0x122)[q] for q in range(4)]
    define(0x318, thin_h, empty, 0)
    define(0x319, thin_v, empty, 0)
    blend = Image.open(bank / "gravel_to_earth.png")
    if blend.mode != "P" or blend.size != (24, 16) or blend.getpalette()[:48] != summit.getpalette()[:48]:
        raise ValueError("The gravel/earth junction must use the indexed summit palette")
    face = blend.crop((0, 0, 16, 16))
    for i, tile in enumerate((face, face.transpose(Image.Transpose.FLIP_TOP_BOTTOM))):
        bed = ground((0x119, 0x129)[i])
        # The west verge carries its own outline. Grass must show through
        # its transparent lip; the east half thins onto Emerald's dirt.
        west_edge = 2*i
        bed[west_edge] = grass[west_edge]
        define(0x31A + i, bed, quarters(tile, 11))
    define(0x31C, thin_h, quarters(art(fen, 0, 2), 7), 0)
    bank_under_deck = [grass[q] if q % 2 == 0 else peat[q] for q in range(4)]
    define(0x31D, bank_under_deck, quarters(art(fen, 1, 2), 7))
    center = quarters(art(summit, 1, 1), 11)
    for i, corners in enumerate(((0,), (1,), (2,), (3,), (0, 3), (1, 2))):
        inset = center.copy()
        for q in corners:
            inset[q] = quarters(art(summit, q % 2, 3 + q // 2), 11)[q]
        define(0x3B7 + i, grass, inset)
    # The bend just uphill of the saddle keeps its outer and inner grass
    # profiles. Warm the packed surface on its road-facing quarter.
    wear = quarters(blend.crop((16, 0, 32, 16)), 11)[0]
    for i, (original, q) in enumerate(((0x3BC, 3), (0x3B4, 1))):
        pos = (original - 512) * 8
        top = definitions[pos+4:pos+8]
        top[q] = wear ^ (0x800 if i else 0)
        define(0x31E + i, grass, top)
    define(0x3BD, peat, quarters(art(ridge, 6, 3), 6))
    define(0x3BE, moss, quarters(art(ridge, 7, 3), 6))
    define(0x3BF, ground(0x170), quarters(fog, 11), 0x1010)
    pack_ridge_contact(ridge, general, primary, quarters, define)
    # The last exposed buttress shares the continuous granite face and keeps
    # the behavior of the native rock tile it replaces.
    define(0x388, ground(0x0A9), empty, words(general_bank / "metatile_attributes.bin")[0x071])

    # Remove abandoned work pixels after artist iterations; caller-defined warp
    # and compound shore aliases only use graphics already assigned above.
    donor = Image.open(root / "data/tilesets/secondary/fortree/tiles.png")
    referenced = {v & 1023 for v in definitions}
    for index in free:
        if index not in referenced:
            x, y = index % 16 * 8, (index - 512) // 16 * 8
            sheet.paste(donor.crop((x, y, x + 8, y + 8)), (x, y))
    sheet.save(bank / "tiles.png", bits=4)
    (bank / "metatiles.bin").write_bytes(struct.pack(f"<{len(definitions)}H", *definitions))
    (bank / "metatile_attributes.bin").write_bytes(struct.pack(f"<{len(attributes)}H", *attributes))
    for palette, source in ((6, ridge), (7, fen), (11, summit)):
        write_palette(bank / f"palettes/{palette:02}.pal", source)


if __name__ == "__main__":
    pack_stormcrest(Path(__file__).resolve().parents[2])
