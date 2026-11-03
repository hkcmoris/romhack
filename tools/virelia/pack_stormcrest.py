"""Pack the native-size Stormcrest pass and fen art into the Rainfall secondary bank."""

import struct
from pathlib import Path

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
    earlier = definitions[:start * 8]
    if any(v >> 12 in (6, 7) for v in earlier):
        raise ValueError("Stormcrest needs unused Rainfall palette slots 6 and 7")
    # 384..388 belong to the route warps. Later compound shores may alias our
    # graphics, while any other graphics they reference must stay allocated.
    gap = slice((0x384 - 512) * 8, (0x389 - 512) * 8)
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
    center = quarters(art(summit, 1, 1), 11)
    for i, corners in enumerate(((0,), (1,), (2,), (3,), (0, 3), (1, 2))):
        inset = center.copy()
        for q in corners:
            inset[q] = quarters(art(summit, q % 2, 3 + q // 2), 11)[q]
        define(0x3B7 + i, grass, inset)
    define(0x3BD, peat, quarters(art(ridge, 6, 3), 6))
    define(0x3BE, moss, quarters(art(ridge, 7, 3), 6))
    define(0x3BF, ground(0x170), quarters(fog, 11), 0x1010)

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
