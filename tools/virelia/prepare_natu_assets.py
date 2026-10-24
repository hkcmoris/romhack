#!/usr/bin/env python3
"""Map prepared Natu-family party icons to the engine's shared palette 3.

Battle artwork and the original art icons remain unchanged. Run this after
art/virelia_natu_line/export_assets.py when refreshing the approved artwork.
Requires Pillow, as do the other sprite preparation tools.
"""
from pathlib import Path

from PIL import Image

ROOT = Path(__file__).resolve().parents[2]
DIRECTORIES = ("natu/virelia", "xatu/virelia", "nimbatu")


def main():
    palette = [tuple(map(int, line.split())) for line in
               (ROOT / "graphics/pokemon/icon_palettes/pal3.pal").read_text().splitlines()[3:19]]
    for directory in DIRECTORIES:
        folder = ROOT / "graphics/pokemon" / directory
        with Image.open(folder / "icon.png") as source:
            assert source.mode == "P" and source.size == (32, 64)
            assert source.info.get("transparency") == 0
            colors = source.getpalette()
            indices = [0]
            for index in range(1, 16):
                color = colors[index * 3:index * 3 + 3]
                indices.append(min(range(1, 16), key=lambda i:
                                   sum((color[c] - palette[i][c]) ** 2 for c in range(3))))
            result = Image.new("P", source.size)
            result.putpalette([channel for color in palette for channel in color])
            result.putdata([indices[index] for index in source.get_flattened_data()])
            result.save(folder / "icon_game.png", bits=4, transparency=0)
        print(f"Prepared {directory}/icon_game.png (shared palette 3)")


if __name__ == "__main__":
    main()
