#!/usr/bin/env python3
"""Recompose the ancient tree from native foliage and its gnarled root master.

The original generated master supplies only the distinctive trunk silhouette.
Native Emerald leaf clusters and colors keep the sanctuary's tree, mossy
stones and earth impressions consistent with the surrounding forest.
"""
from collections import Counter
from pathlib import Path

from PIL import Image, ImageDraw, ImageFilter

from stock_art import stock_part

ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / 'art/whisperwood/old-tree-source.png'
TARGET = ROOT / 'data/tilesets/secondary/whisperwood/old_tree.png'


def sanctuary_palette():
    def colors(bank, slot):
        return [tuple(int(c) // 8 * 8 for c in line.split()) for line in
                (ROOT / f'data/tilesets/{bank}/palettes/{slot:02}.pal').read_text().splitlines()[3:19]]

    forest = colors('primary/general', 2)
    rock = colors('primary/general', 3)
    frlg = colors('secondary/viridian_forest_frlg', 8)
    # 1..4 foliage, 5..10 stone, 11..15 bark; index zero stays transparent.
    return [(24, 40, 80), *forest[1:5], *rock[9:15],
            forest[5], forest[7], frlg[7], frlg[6], forest[8]]


def trunk():
    """Retain the original broad roots and branch fork at their new native size."""
    source = Image.open(SOURCE).convert('RGBA')
    bounds = source.getchannel('A').point(lambda value: 255 if value >= 128 else 0).getbbox()
    if bounds is None:
        raise ValueError('Old tree has no opaque pixels')
    source = source.crop(bounds)
    source = source.crop((0, int(source.height * .38), source.width, source.height))
    source.thumbnail((68, 55), Image.Resampling.NEAREST)
    master = Image.new('RGBA', (80, 80))
    master.paste(source, ((80 - source.width) // 2, 79 - source.height))
    wood = {(x, y) for y in range(80) for x in range(80)
            if (pixel := master.getpixel((x, y)))[3] >= 128
            and (y >= 57 or pixel[0] >= pixel[1] * .95 and pixel[0] > pixel[2] * 1.12)}
    # Keep dark bark and moss immediately adjoining the wood, without bringing
    # the generated master canopy into the new native foliage composition.
    surface = wood | {(x + dx, y + dy) for x, y in wood
                      for dx, dy in ((-1, 0), (1, 0), (0, -1), (0, 1))
                      if 0 <= x + dx < 80 and 0 <= y + dy < 80}
    result = Image.new('P', (80, 80), 0)
    for x, y in sorted(surface):
        r, g, b, a = master.getpixel((x, y))
        if a < 128:
            continue
        light = (r * 3 + g * 6 + b) // 10
        if g > r * .95 and g > b * 1.3:
            index = 2 if light > 140 else 3 if light > 90 else 4
        else:
            index = 11 if light > 155 else 12 if light > 120 else 13 if light > 85 else 14 if light > 45 else 15
        result.putpixel((x, y), index)
    # Remove isolated color speckles while keeping the branching silhouette.
    original = result.copy()
    for y in range(25, 79):
        for x in range(1, 79):
            current = original.getpixel((x, y))
            if not current:
                continue
            neighbors = [original.getpixel((x + dx, y + dy))
                         for dy in (-1, 0, 1) for dx in (-1, 0, 1) if dx or dy]
            opaque = [value for value in neighbors if value]
            if len(opaque) >= 6 and neighbors.count(current) < 2:
                result.putpixel((x, y), Counter(opaque).most_common(1)[0][0])
    remaining = {(x, y) for y in range(80) for x in range(80) if result.getpixel((x, y))}
    components = []
    while remaining:
        start = min(remaining)
        remaining.remove(start)
        component, queue = {start}, [start]
        while queue:
            x, y = queue.pop()
            for dy in (-1, 0, 1):
                for dx in (-1, 0, 1):
                    point = x + dx, y + dy
                    if point in remaining:
                        remaining.remove(point)
                        component.add(point)
                        queue.append(point)
        components.append(component)
    for component in sorted(components, key=len)[:-1]:
        for point in component:
            result.putpixel(point, 0)
    return result


def canopy(palette):
    """Compose only leaf fragments, excluding each donor's dark ground skirt."""
    crown = Image.new('P', (32, 48), 0)
    for row, ids in enumerate(((0x1ce, 0x1cf), (0x1d4, 0x1d5), (0x1e4, 0x1e5))):
        for column, index in enumerate(ids):
            foliage = stock_part('general', 'fortree', index, palette,
                                 index_map={i: i for i in range(1, 5)})
            crown.paste(foliage, (column * 16, row * 16))
    # Rows below 34 contain the stock tree's ground/shadow skirt. They never
    # enter this canopy; the old repeated skirts caused the doubled shadow.
    leaves = crown.crop((0, 12, 32, 34))
    silhouette = Image.new('L', (80, 80), 0)
    draw = ImageDraw.Draw(silhouette)
    for bounds in ((18, 2, 59, 37), (4, 14, 40, 42), (42, 10, 75, 41),
                   (14, 22, 66, 46), (0, 24, 25, 40), (59, 24, 79, 41)):
        draw.ellipse(bounds, fill=255)
    interior = silhouette.filter(ImageFilter.MinFilter(3))
    result = Image.new('P', (80, 80), 0)
    result.paste(4, (0, 0, 80, 80), silhouette)
    result.paste(3, (0, 0, 80, 80), interior)
    for position in ((19, 0), (3, 10), (43, 7), (18, 16), (37, 21),
                     (0, 22), (55, 22), (15, 25), (34, 27)):
        layer = Image.new('P', (80, 80), 0)
        layer.paste(leaves, position)
        for y in range(47):
            for x in range(80):
                pixel = layer.getpixel((x, y))
                if not pixel or not interior.getpixel((x, y)):
                    continue
                # Keep the upper-left crown lit and the lower-right lobes in
                # shade, using only Emerald's four native foliage colors.
                if pixel == 1 and (x > 58 or y > 36):
                    pixel = 2
                elif pixel == 2 and y > 40:
                    pixel = 3
                elif pixel == 3 and y > 42 and x > 40:
                    pixel = 4
                result.putpixel((x, y), pixel)
    return result


def build_tree():
    palette = sanctuary_palette()
    tree = Image.new('P', (80, 80), 0)
    tree.putpalette([c for color in palette for c in color] + [0] * 720)
    # One low contact shadow belongs to the whole trunk and exposed roots.
    # No floating shadow copies accompany the individual canopy fragments.
    ImageDraw.Draw(tree).ellipse((12, 68, 68, 77), fill=4)
    for part in (trunk(), canopy(palette)):
        mask = Image.frombytes('L', part.size, bytes(255 if p else 0 for p in part.tobytes()))
        tree.paste(part, (0, 0), mask)
    tree.info['transparency'] = 0
    return tree


def main():
    tree = build_tree()
    TARGET.parent.mkdir(parents=True, exist_ok=True)
    tree.save(TARGET, transparency=0, bits=4)
    output = ROOT / 'outputs/whisperwood'
    output.mkdir(parents=True, exist_ok=True)
    tree.convert('RGBA').resize((640, 640), Image.Resampling.NEAREST).save(output / 'old-tree-preview.png')
    print('Built 80x80 ancient tree with one canopy, gnarled roots and one ground shadow.')


if __name__ == '__main__':
    main()
