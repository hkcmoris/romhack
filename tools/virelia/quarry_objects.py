"""Build readable 16px quarry collectibles using the shared moving-box palette."""
from pathlib import Path

from PIL import Image, ImageDraw

from stock_art import stock_part

ROOT = Path(__file__).resolve().parents[2]


def stamp(image, part, position):
    mask = Image.frombytes('L', part.size, bytes(255 if p else 0 for p in part.tobytes()))
    image.paste(part, position, mask)


def tool(name):
    image = Image.new('P', (16, 16))
    d = ImageDraw.Draw(image)
    # A short contact shadow under the grip, rather than a solid black wedge.
    d.line((3, 14, 6, 14), fill=6)
    d.point((7, 13), fill=14)
    d.line((4, 12, 10, 6), fill=6, width=3)
    if name == 'wrench':
        d.line((4, 11, 10, 5), fill=10, width=2)
        d.line((5, 11, 9, 7), fill=1)
        d.point((4, 12), fill=7)
        d.polygon(((8, 5), (8, 2), (10, 1), (10, 4), (12, 4),
                   (13, 1), (14, 3), (13, 6), (10, 7)), fill=6)
        d.line((9, 2, 9, 5, 11, 6, 13, 4), fill=10)
        d.line((10, 5, 12, 5, 13, 3), fill=9)
        d.point((13, 2), fill=4)
    else:
        d.line((4, 11, 10, 5), fill=13, width=2)
        d.line((4, 11, 8, 7), fill=12)
        d.point((5, 12), fill=14)
        d.point((6, 9), fill=13)
        if name == 'hammer':
            d.polygon(((5, 4), (8, 1), (14, 5), (11, 8)), fill=6)
            d.polygon(((6, 4), (8, 2), (12, 5), (10, 7)), fill=11)
            d.line((7, 3, 8, 2, 12, 5), fill=9)
            d.line((12, 6, 11, 7), fill=2)
            d.point((6, 4), fill=10)
        else:
            d.line((3, 4, 6, 2, 9, 2, 12, 4, 14, 7), fill=6, width=3)
            d.line((3, 3, 6, 1, 9, 1, 12, 3, 14, 6), fill=9)
            d.line((5, 3, 8, 2, 11, 4, 13, 6), fill=11)
            d.point((14, 7), fill=2)
            d.rectangle((8, 3, 9, 5), fill=2)
            d.line((8, 3, 9, 3), fill=10)
    return image


def cargo(name, colors):
    image = Image.new('P', (16, 16))
    d = ImageDraw.Draw(image)
    # Open rim behind the cargo; left lighting, a darker right side and a
    # shallow footprint keep the crate in the map's three-quarter perspective.
    d.polygon(((2, 6), (4, 4), (12, 4), (14, 6), (14, 12),
               (11, 15), (1, 15), (1, 7)), fill=6)
    d.line((4, 4, 11, 4), fill=12)
    d.polygon(((3, 6), (5, 5), (11, 5), (12, 7), (3, 8)), fill=14)
    if name == 'ore_crate':
        # Native Rusturf boulder facets, lifted as the exposed ore load.
        ore = stock_part('general', 'rusturf_tunnel', 0x203, colors, layers=(1,),
                         index_map={1: 9, 2: 10, 3: 11, 4: 1, 5: 2, 6: 14, 7: 6,
                                    8: 9, 9: 10, 10: 11, 11: 1, 12: 2, 13: 14, 14: 6, 15: 6})
        stamp(image, ore.crop((2, 0, 14, 8)), (2, 1))
        d.polygon(((3, 7), (4, 4), (6, 3), (8, 5), (7, 8)), fill=2)
        d.line((4, 5, 6, 4, 7, 5), fill=9)
        d.line((4, 6, 5, 7), fill=10)
    else:
        # Separate silhouettes remain visible above the rim: an upright
        # wrench, a broad hammer head and their crossing wooden handles.
        d.line((6, 7, 10, 3), fill=6, width=3)
        d.line((6, 6, 10, 2), fill=12)
        d.line((7, 7, 11, 3), fill=13)
        d.polygon(((8, 3), (8, 1), (9, 0), (9, 2), (11, 2),
                   (11, 0), (12, 1), (12, 4), (10, 5)), fill=6)
        d.line((9, 3, 11, 3, 11, 1), fill=9)
        d.line((5, 3, 9, 7), fill=6, width=3)
        d.line((5, 3, 9, 7), fill=13)
        d.line((5, 4, 8, 7), fill=12)
        d.polygon(((1, 3), (3, 1), (7, 3), (5, 5)), fill=6)
        d.line((2, 3, 3, 2, 6, 3), fill=9)
        d.line((3, 3, 5, 4), fill=11)
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
        image = cargo(name, colors) if name in ('ore_crate', 'tool_hoard') else tool(name)
        image.putpalette([c for color in colors for c in color] + [0] * 720)
        image.save(folder / f'{name}.png', bits=4, transparency=0)


if __name__ == '__main__':
    build_objects()
