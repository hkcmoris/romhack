#!/usr/bin/env python3
"""Prepare copied/recolored fire-family sheets and pack imagegen detail edits.

The original overworld assets are read-only. Six frames remain in native order:
down idle/step, up idle/step, left idle/step. Requires Pillow for art processing.
"""
import argparse
from pathlib import Path
import shutil

from PIL import Image

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / "outputs/prologue-merged/fire-overworld"
FAMILIES = {
    "ekans": ("ekans", "ekans/virelia", [0, 3, 6, 5, 1, 7, 10, 9, 4]),
    "arbok": ("arbok", "arbok/virelia", [0, 2, 4, 6, 1, 15, 12, 3, 9]),
    "infernabra": ("arbok", "infernabra", [0, 2, 4, 6, 1, 15, 12, 3, 9]),
}


def colors(path):
    return [tuple(map(int, line.split())) for line in path.read_text().splitlines()[3:19]]


def indexed(pixels, palette):
    image = Image.new("P", pixels.size)
    image.putpalette([c for color in palette for c in color] + [0] * (768 - 48))
    image.putdata(list(pixels.getdata()))
    image.info["transparency"] = 0
    return image


def grid(sheet, scale=16):
    result = Image.new("RGBA", (64, 96))
    rgba = sheet.convert("RGBA")
    for frame in range(6):
        result.paste(rgba.crop((32 * frame, 0, 32 * (frame + 1), 32)),
                     ((frame % 2) * 32, (frame // 2) * 32))
    return result.resize((64 * scale, 96 * scale), Image.Resampling.NEAREST)


def prepare(name):
    donor, folder, mapping = FAMILIES[name]
    target = ROOT / "graphics/pokemon" / folder
    source = Image.open(ROOT / "graphics/pokemon" / donor / "overworld.png")
    pixels = Image.new("P", source.size)
    pixels.putdata([mapping[value] if value < len(mapping) else 1 for value in source.getdata()])
    sheet = indexed(pixels, colors(target / "normal.pal"))
    sheet.save(OUT / f"{name}-recolored.png", bits=4, transparency=0)
    grid(sheet).save(OUT / f"{name}-reference-grid.png")
    battle = Image.open(target / "front.png").convert("RGBA")
    battle.save(OUT / f"{name}-battle-reference.png")


def pack(name, generated):
    _, folder, _ = FAMILIES[name]
    target = ROOT / "graphics/pokemon" / folder
    palette = colors(target / "normal.pal")
    source = Image.open(generated).convert("RGBA")
    small = source.resize((64, 96), Image.Resampling.NEAREST)
    sheet = Image.new("P", (192, 32))
    sheet.putpalette([c for color in palette for c in color] + [0] * (768 - 48))
    for frame in range(6):
        for y in range(32):
            for x in range(32):
                r, g, b, a = small.getpixel(((frame % 2) * 32 + x, (frame // 2) * 32 + y))
                value = 0 if a < 128 else min(range(1, 16), key=lambda i: sum((c - d) ** 2 for c, d in zip((r, g, b), palette[i])))
                sheet.putpixel((frame * 32 + x, y), value)
    sheet.save(target / "overworld.png", bits=4, transparency=0)
    shutil.copyfile(target / "normal.pal", target / "overworld_normal.pal")
    shutil.copyfile(target / "shiny.pal", target / "overworld_shiny.pal")
    grid(sheet, 8).save(OUT / f"{name}-review.png")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("name", choices=FAMILIES)
    parser.add_argument("--generated", type=Path)
    args = parser.parse_args()
    OUT.mkdir(parents=True, exist_ok=True)
    if args.generated:
        pack(args.name, args.generated)
    else:
        prepare(args.name)
