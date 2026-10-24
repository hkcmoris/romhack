"""Compose Slateford's shared Center/Mart facade from native Emerald parts.

The 112x80 building uses General palettes 0..2, with Rustboro palette 8 only
for the existing city paving. A cell contains eight
``(64-byte indexed 8x8 pixels, palette slot)`` pairs: four backing quadrants,
then four foreground quadrants. Flips are baked into the returned pixels.
The two original Center door cells remain exact, for native door animation.
"""
from functools import lru_cache
from pathlib import Path
import struct

from PIL import Image

ROOT = Path(__file__).resolve().parents[2]
WIDTH, HEIGHT = 7, 5
DOOR = (3, 4)
UPPER_DOOR = (3, 3)

# Existing Rustboro exteriors use the General buildings over city paving.
CENTER = ((0x256, 0x25e, 0x25f, 0x257),
          (0x050, 0x051, 0x052, 0x053),
          (0x058, 0x059, 0x05a, 0x05b),
          (0x266, 0x061, 0x062, 0x27e))
MART = ((0x325, 0x267, 0x267, 0x326),
        (0x030, 0x031, 0x032, 0x033),
        (0x038, 0x039, 0x03a, 0x03b),
        (0x266, 0x041, 0x042, 0x27f))


@lru_cache(None)
def _pixels(bank):
    with Image.open(ROOT / f'data/tilesets/{bank}/tiles.png') as image:
        return image.copy()


@lru_cache(None)
def _cell(index):
    bank = 'primary/general' if index < 512 else 'secondary/rustboro'
    raw = (ROOT / f'data/tilesets/{bank}/metatiles.bin').read_bytes()
    words = struct.unpack_from('<8H', raw, (index % 512) * 16)
    result = []
    for value in words:
        tile, slot = value & 1023, value >> 12
        assert slot < 6 or slot == 8, 'Unexpected service exterior source palette'
        bank = 'primary/general' if tile < 512 else 'secondary/rustboro'
        tile %= 512
        source = _pixels(bank)
        data = bytes(source.getpixel((tile % 16 * 8 + (7 - x if value & 0x400 else x),
                                      tile // 16 * 8 + (7 - y if value & 0x800 else y)))
                     for y in range(8) for x in range(8))
        result.append((data, slot))
    return tuple(result)


def _part(x, y, mart=False):
    """Return both native layers for one 8px facade piece."""
    tiles = MART if mart else CENTER
    cell = _cell(tiles[y // 2][x // 2])
    quadrant = y % 2 * 2 + x % 2
    return cell[quadrant], cell[quadrant + 4]


def build_exterior():
    """Return five rows of seven native-format metatile cells."""
    columns = (0, 1, 2, 3, 4, 3, 4, 3, 4, 3, 4, 5, 6, 7)
    frame = (0, 1, 1, 1, 1, 1, 2, 3, 4, 5, 6, 6, 6, 7)
    facade = (0, 1, 5, 6, 1, 1, 2, 3, 4, 1, 5, 6, 1, 7)
    parts = [[None] * 14 for _ in range(10)]
    for y in range(10):
        for x in range(14):
            if y < 2:
                part = _part(columns[x], y)
            elif y < 4:
                # Each destination column continues its own rib profile.
                # Repeating one donor here shifts the ribs at the row seam.
                backing, (pixels, slot) = _part(columns[x], 1)
                part = backing, (pixels[-8:] * 8, slot)
                if x in (0, 13):
                    part = _part(0 if x == 0 else 7, 2)
            elif y < 7:
                part = _part(frame[x], y - 2)
            else:
                part = _part(facade[x], y - 2)
            parts[y][x] = part
    # Continue the same ribs into the last roof row. Retain only the native
    # pediment's changes from its own backing profile, so its diagonals sit on
    # the continuous roof instead of importing a second, misaligned pattern.
    for x in range(1, 13):
        backing, (pixels, slot) = parts[4][x]
        profile = parts[1][x][1][0][-8:]
        donor_profile = _part(frame[x], 1)[1][0][-8:]
        pixels = bytes(pixel if pixel != donor_profile[i % 8] else profile[i % 8]
                       for i, pixel in enumerate(pixels))
        parts[4][x] = backing, (pixels, slot)
    # Join the repeated middle roof peaks into one continuous native ridge.
    # Preserve the two outer bevels; use the stock horizontal cap's outline
    # and continue each rib's own shade immediately below it.
    ridge = _part(1, 1)[1][0][:8]
    for x in range(4, 10):
        backing, (pixels, slot) = parts[0][x]
        pixels = bytearray(pixels)
        pixels[:8] = ridge
        pixels[8:16] = pixels[16:24]
        parts[0][x] = backing, (bytes(pixels), slot)
    # Healing label on the left, shop label on the right, sharing one door.
    for y in (7, 8, 9):
        for x in (10, 11):
            parts[y][x] = _part(x - 5, y - 2, mart=True)
    # General's red and blue building palettes share the same pixel indices.
    # Color the whole right slope and eave blue, with one shared red/white
    # badge at the entrance. Its two original door cells stay byte-exact for
    # the stock sliding animation; paving and the facade retain their colors.
    for y in range(7):
        for x in range(7, 14):
            if y in (5, 6) and x in (7, 8):
                continue
            parts[y][x] = tuple((pixels, 0 if slot == 1 else slot)
                                for pixels, slot in parts[y][x])
    cells = []
    for y in range(HEIGHT):
        row = []
        for x in range(WIDTH):
            quadrants = [parts[y * 2 + dy][x * 2 + dx] for dy in (0, 1) for dx in (0, 1)]
            row.append(tuple(piece[layer] for layer in (0, 1) for piece in quadrants))
        cells.append(row)
    assert cells[UPPER_DOOR[1]][UPPER_DOOR[0]] == _cell(0x059)
    assert cells[DOOR[1]][DOOR[0]] == _cell(0x061)
    return cells


def render_exterior(cells):
    """Render returned cells to a native-size RGBA image for visual review."""
    palettes = {}
    for slot in (*range(6), 8):
        bank = 'secondary/rustboro' if slot == 8 else 'primary/general'
        palettes[slot] = [tuple(map(int, line.split())) for line in
                          (ROOT / f'data/tilesets/{bank}/palettes/{slot:02}.pal').read_text().splitlines()[3:19]]
    image = Image.new('RGBA', (WIDTH * 16, HEIGHT * 16))
    for y, row in enumerate(cells):
        for x, cell in enumerate(row):
            for n, (pixels, slot) in enumerate(cell):
                for py in range(8):
                    for px in range(8):
                        color = pixels[py * 8 + px]
                        if color:
                            image.putpixel((x * 16 + n % 2 * 8 + px,
                                            y * 16 + n % 4 // 2 * 8 + py),
                                           (*palettes[slot][color], 255))
    return image


if __name__ == '__main__':
    output = ROOT / 'art/services'
    output.mkdir(parents=True, exist_ok=True)
    render_exterior(build_exterior()).save(output / 'center-mart-exterior.png')
    print('Built 112x80 shared Center/Mart facade; native Center door cells preserved.')
