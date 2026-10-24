#!/usr/bin/env python3
"""Convert the generated old-tree master to one GBA-native 48x64 indexed sprite."""
from pathlib import Path
from PIL import Image

ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / 'art/whisperwood/old-tree-source.png'
TARGET = ROOT / 'data/tilesets/secondary/whisperwood/old_tree.png'


def main():
    source = Image.open(SOURCE).convert('RGBA')
    alpha = source.getchannel('A')
    bounds = alpha.point(lambda value: 255 if value >= 128 else 0).getbbox()
    if bounds is None:
        raise ValueError('Old tree has no opaque pixels')
    source = source.crop(bounds)
    source.thumbnail((46, 62), Image.Resampling.NEAREST)
    tree = Image.new('RGBA', (48, 64), (0, 0, 0, 0))
    tree.paste(source, ((48 - source.width) // 2, 63 - source.height))
    opaque = [tree.getpixel((x, y))[:3] for y in range(64) for x in range(48) if tree.getpixel((x, y))[3] >= 128]
    sample = Image.new('RGB', (len(opaque), 1))
    sample.putdata(opaque)
    reduced = sample.quantize(colors=15, method=Image.Quantize.MEDIANCUT, dither=Image.Dither.NONE)
    palette = reduced.getpalette()[:45]
    colors = [tuple((palette[i + channel] // 8) * 8 for channel in range(3)) for i in range(0, 45, 3)]
    result = Image.new('P', (48, 64), 0)
    palette = [24, 40, 80] + [channel for color in colors for channel in color]
    result.putpalette(palette + [0] * (768 - len(palette)))
    for y in range(64):
        for x in range(48):
            pixel = tree.getpixel((x, y))
            if pixel[3] < 128:
                continue
            index = min(range(15), key=lambda i: sum((pixel[c] - colors[i][c]) ** 2 for c in range(3)))
            result.putpixel((x, y), index + 1)
    TARGET.parent.mkdir(parents=True, exist_ok=True)
    result.save(TARGET, transparency=0, bits=4)
    preview = Image.open(TARGET).convert('RGBA').resize((384, 512), Image.Resampling.NEAREST)
    output = ROOT / 'outputs/whisperwood'
    output.mkdir(parents=True, exist_ok=True)
    preview.save(output / 'old-tree-preview.png')
    print('Wrote one 48x64 old-tree sprite: 3x4 map tiles, 15 colors plus transparency.')


if __name__ == '__main__':
    main()
