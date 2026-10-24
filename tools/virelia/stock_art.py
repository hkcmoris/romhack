"""Recompose native Emerald/FRLG metatiles into a destination 4bpp palette."""
from functools import lru_cache
from pathlib import Path
import struct

from PIL import Image

ROOT = Path(__file__).resolve().parents[2] / 'data/tilesets'


@lru_cache(None)
def source_image(bank):
    with Image.open(ROOT / bank / 'tiles.png') as image:
        return image.copy()


@lru_cache(None)
def source_words(bank):
    data = (ROOT / bank / 'metatiles.bin').read_bytes()
    return struct.unpack(f'<{len(data) // 2}H', data)


@lru_cache(None)
def source_palette(bank, slot):
    return tuple(tuple(int(c) // 8 * 8 for c in line.split()) for line in
                 (ROOT / bank / f'palettes/{slot:02}.pal').read_text().splitlines()[3:19])


def stock_part(primary, secondary, index, target_palette, layers=(0, 1), index_map=None):
    """Extract a 16px stock metatile, optionally keeping only its foreground.

    Bank names omit ``primary/`` and ``secondary/``. FRLG uses a 640-tile
    primary bank and seven primary palettes. Color zero always stays
    transparent; opaque colors map only to destination entries 1 through 15.
    Flip bits and each quadrant's own palette are honored before remapping.
    An explicit index map instead selects/remaps source indices; omitted
    indices become transparent, useful for lifting gems out of a rock face.
    """
    if len(target_palette) != 16 or any(layer not in (0, 1) for layer in layers):
        raise ValueError('Stock parts need a 16-color palette and layers 0/1')
    split, palette_split = (640, 7) if primary.endswith('_frlg') else (512, 6)
    primary, secondary = 'primary/' + primary, 'secondary/' + secondary
    bank, local = (primary, index) if index < split else (secondary, index - split)
    values = source_words(bank)[local * 8:(local + 1) * 8]
    if len(values) != 8:
        raise ValueError(f'No stock metatile {index:#x} in {bank}')
    image = Image.new('P', (16, 16), 0)
    image.putpalette([c for color in target_palette for c in color] + [0] * 720)
    image.info['transparency'] = 0
    matches = {}
    for layer in layers:
        for quadrant, value in enumerate(values[layer * 4:layer * 4 + 4]):
            tile, slot = value & 0x3ff, value >> 12
            pixels = source_image(primary if tile < split else secondary)
            tile = tile if tile < split else tile - split
            colors = source_palette(primary if slot < palette_split else secondary, slot)
            for y in range(8):
                for x in range(8):
                    sx = tile % 16 * 8 + (7 - x if value & 0x400 else x)
                    sy = tile // 16 * 8 + (7 - y if value & 0x800 else y)
                    if sx >= pixels.width or sy >= pixels.height:
                        raise ValueError(f'Stock metatile {index:#x} references missing tile {tile:#x}')
                    pixel = pixels.getpixel((sx, sy))
                    if not pixel:
                        continue
                    if index_map is not None:
                        target = index_map.get(pixel, 0)
                    else:
                        color = colors[pixel]
                        if color not in matches:
                            matches[color] = min(range(1, 16), key=lambda i: sum(
                                (color[c] - target_palette[i][c]) ** 2 for c in range(3)))
                        target = matches[color]
                    if not 0 <= target < 16:
                        raise ValueError('Stock index map exceeds the 4bpp palette')
                    if target:
                        image.putpixel((quadrant % 2 * 8 + x, quadrant // 2 * 8 + y), target)
    return image
