"""Pack the editable Rainfall marsh sheet without touching any map layouts."""

from pathlib import Path
import struct

from PIL import Image


FIRST_METATILE = 0x32D
PALETTE_SLOT = 12
# These ranges are blank and unreferenced in the original Fortree bank.
GRAPHICS = (
    *range(0x284, 0x290), *range(0x2C9, 0x2D3),
    *range(0x2D9, 0x2E4), *range(0x2E9, 0x2FC),
)


def pack_marsh(root):
    bank = root / "data/tilesets/secondary/virelia_rainfall"
    source = Image.open(bank / "marsh.png")
    if source.mode != "P" or source.size != (48, 80) or source.getextrema()[1] > 15:
        raise ValueError("marsh.png must be a 48x80 indexed 4bpp sheet")
    colors = source.getpalette()[:48]
    palette = ["JASC-PAL", "0100", "16"]
    palette.extend(f"{r} {g} {b}" for r, g, b in struct.iter_unpack("3B", bytes(colors)))

    graphics = Image.open(bank / "tiles.png").copy()
    meta_path = bank / "metatiles.bin"
    attr_path = bank / "metatile_attributes.bin"
    metas = bytearray(meta_path.read_bytes())
    attrs = bytearray(attr_path.read_bytes())
    start = FIRST_METATILE - 512
    stop = start + 13
    other_tiles = metas[:start * 16] + metas[stop * 16:]
    for value, in struct.iter_unpack("<H", other_tiles):
        if value & 1023 in GRAPHICS or value >> 12 == PALETTE_SLOT:
            raise ValueError("another Rainfall metatile uses a reserved marsh graphic or palette")
    if len(attrs) < start * 2 or len(metas) != len(attrs) * 8:
        raise ValueError("build the Rainfall bridge metatiles before packing the marsh")
    metas.extend(bytes(max(0, stop * 16 - len(metas))))
    attrs.extend(bytes(max(0, stop * 2 - len(attrs))))

    grass = (root / "data/tilesets/primary/general/metatiles.bin").read_bytes()[16:24]
    # Classic 3x3 followed by a 2x2 of inverse corners, named by the dry diagonal.
    pieces = [(x, y) for y in range(5) for x in range(3 if y < 3 else 2)]
    for i, (x, y) in enumerate(pieces):
        offset = (start + i) * 16
        metas[offset:offset + 8] = grass
        for q in range(4):
            sx, sy = x * 16 + q % 2 * 8, y * 16 + q // 2 * 8
            graphic = GRAPHICS[i * 4 + q]
            index = graphic - 512
            graphics.paste(source.crop((sx, sy, sx + 8, sy + 8)), (index % 16 * 8, index // 16 * 8))
            struct.pack_into("<H", metas, offset + 8 + q * 2, PALETTE_SLOT << 12 | graphic)
        # MB_PUDDLE on the bottom and middle backgrounds, below the actor.
        struct.pack_into("<H", attrs, (start + i) * 2, 0x1016)

    graphics.save(bank / "tiles.png", bits=4)
    meta_path.write_bytes(metas)
    attr_path.write_bytes(attrs)
    (bank / "palettes/12.pal").write_text("\n".join(palette) + "\n", encoding="ascii")


if __name__ == "__main__":
    pack_marsh(Path(__file__).resolve().parents[2])
