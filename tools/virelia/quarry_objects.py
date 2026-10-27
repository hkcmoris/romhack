"""Pack hand-drawn 16px quarry tools and build the stock ore crate."""
from pathlib import Path

from PIL import Image, ImageDraw

from stock_art import stock_part

ROOT = Path(__file__).resolve().parents[2]


def stamp(image, part, position):
    mask = Image.frombytes('L', part.size, bytes(255 if p else 0 for p in part.tobytes()))
    image.paste(part, position, mask)


def tool(name, colors):
    """Index the native diivi export exactly; never redraw or quantize it."""
    path = ROOT / 'art/quarry_tools_v2/exports' / f'{name}.png'
    rgba = Image.open(path).convert('RGBA')
    if rgba.size != (16, 16):
        raise ValueError(f'{path}: expected a native 16x16 tool')
    lookup = {color: index for index, color in enumerate(colors) if index}
    pixels = []
    for pixel in (rgba.getpixel((x, y)) for y in range(16) for x in range(16)):
        if pixel[3] == 0:
            pixels.append(0)
        elif pixel[3] == 255 and pixel[:3] in lookup:
            pixels.append(lookup[pixel[:3]])
        else:
            raise ValueError(f'{path}: non-palette color or partial alpha {pixel}')
    image = Image.new('P', (16, 16))
    image.putdata(pixels)
    return image


def ore_crate(colors):
    image = Image.new('P', (16, 16))
    d = ImageDraw.Draw(image)
    # Open rim behind the cargo; left lighting, a darker right side and a
    # shallow footprint keep the crate in the map's three-quarter perspective.
    d.polygon(((2, 6), (4, 4), (12, 4), (14, 6), (14, 12),
               (11, 15), (1, 15), (1, 7)), fill=6)
    d.line((4, 4, 11, 4), fill=12)
    d.polygon(((3, 6), (5, 5), (11, 5), (12, 7), (3, 8)), fill=14)
    # Native Rusturf boulder facets, lifted as the exposed ore load.
    ore = stock_part('general', 'rusturf_tunnel', 0x203, colors, layers=(1,),
                     index_map={1: 9, 2: 10, 3: 11, 4: 1, 5: 2, 6: 14, 7: 6,
                                8: 9, 9: 10, 10: 11, 11: 1, 12: 2, 13: 14, 14: 6, 15: 6})
    stamp(image, ore.crop((2, 0, 14, 8)), (2, 1))
    d.polygon(((3, 7), (4, 4), (6, 3), (8, 5), (7, 8)), fill=2)
    d.line((4, 5, 6, 4, 7, 5), fill=9)
    d.line((4, 6, 5, 7), fill=10)
    # Raised rim and three planks. The side follows the same boards, one
    # pixel higher, while sparse broken grain avoids a noisy checker pattern.
    d.polygon(((2, 8), (11, 8), (13, 7), (13, 12), (11, 14), (2, 14)), fill=13)
    d.line((2, 8, 10, 8, 12, 7), fill=12)
    d.line((11, 9, 11, 13), fill=6)
    d.line((12, 9, 12, 12), fill=14)
    d.line((2, 10, 10, 10), fill=6)
    d.line((2, 11, 10, 11), fill=12)
    d.line((2, 13, 10, 13), fill=6)
    d.line((4, 9, 6, 9), fill=12)
    d.line((7, 12, 9, 12), fill=14)
    d.line((4, 12, 8, 8), fill=14)
    d.line((4, 11, 7, 8), fill=12)
    # Corner braces and nail heads catch the same upper-left light.
    for x in (2, 10):
        d.line((x, 8, x, 12), fill=12)
        d.point((x, 9), fill=9)
        d.point((x, 12), fill=2)
    return image


def build_objects():
    folder = ROOT / 'graphics/object_events/pics/quarry'
    folder.mkdir(parents=True, exist_ok=True)
    colors = [tuple(map(int, line.split())) for line in
              (ROOT / 'graphics/object_events/palettes/moving_box.pal').read_text().splitlines()[3:19]]
    for name in ('wrench', 'hammer', 'pick', 'ore_crate', 'tool_hoard'):
        image = ore_crate(colors) if name == 'ore_crate' else tool(name, colors)
        image.putpalette([c for color in colors for c in color] + [0] * 720)
        image.save(folder / f'{name}.png', bits=4, transparency=0)


if __name__ == '__main__':
    build_objects()
