#!/usr/bin/env python3
"""Export edited, recolor-first grass follower atlases into GBA walking sheets.

The imagegen authoring atlases are local working material under
outputs/prologue-merged/grass-art/<name>-edited.png. Their first two rows hold
six copied-original walking poses in a three-column grid; the third row holds
battle-art references and is never imported. Original game assets are untouched.
Requires Pillow; exported PNGs/palettes are self-contained build inputs.
"""

import argparse
import json
from pathlib import Path
import shutil

from PIL import Image, ImageDraw

ROOT = Path(__file__).resolve().parents[2]
SPECS = (
    ("poochyena", "poochyena/virelia", (24, 23)),
    ("mightyena", "mightyena/virelia", (28, 28)),
    ("wraithena", "wraithena", (30, 29)),
)


def colors(path):
    lines = path.read_text().splitlines()
    assert lines[:3] == ["JASC-PAL", "0100", "16"]
    return [tuple(map(int, line.split())) for line in lines[3:19]]


def indexed(size, palette):
    image = Image.new("P", size, 0)
    image.putpalette([c for rgb in palette for c in rgb])
    image.info["transparency"] = 0
    return image


def extract(atlas, frame, name):
    # The editor can move a tuft past a nominal cell boundary. These crop
    # regions follow the empty gutters in the reviewed 1254px authoring sheets.
    col, row = frame % 3, frame // 3
    xs = (0, 418, 836, 1254)
    ys = (0, 418, 836)
    if name == "mightyena":
        xs = (0, 435, 870, 1254) if row else (0, 418, 840, 1254)
        ys = (0, 435, 840)
    elif name == "wraithena":
        ys = (0, 455, 845)
    view = atlas.crop((round(xs[col] * atlas.width / 1254), round(ys[row] * atlas.height / 1254),
                       round(xs[col + 1] * atlas.width / 1254), round(ys[row + 1] * atlas.height / 1254)))
    view.putalpha(view.getchannel("A").point(lambda a: 255 if a >= 235 else 0))
    box = view.getchannel("A").getbbox()
    assert box, f"Empty authoring frame {frame}"
    return view.crop(box)


def export(source_dir):
    review = Image.new("RGB", (768, len(SPECS) * 290), (55, 62, 72))
    labels = ImageDraw.Draw(review)
    report = {}
    for row, (name, folder, fit) in enumerate(SPECS):
        target = ROOT / "graphics/pokemon" / folder
        palette = colors(target / "normal.pal")
        shiny = colors(target / "shiny.pal")
        atlas = Image.open(source_dir / f"{name}-edited.png").convert("RGBA")
        views = [extract(atlas, frame, name) for frame in range(6)]
        scale = min(fit[0] / max(v.width for v in views), fit[1] / max(v.height for v in views))
        sheet = indexed((192, 32), palette)
        bounds = []
        cache = {}
        for frame, view in enumerate(views):
            size = (max(1, round(view.width * scale)), max(1, round(view.height * scale)))
            view = view.resize(size, Image.Resampling.NEAREST)
            pixels = []
            for pixel in view.get_flattened_data():
                if pixel[3] == 0:
                    pixels.append(0)
                    continue
                rgb = pixel[:3]
                if rgb not in cache:
                    cache[rgb] = min(range(1, 16), key=lambda i: sum((a - b) ** 2 for a, b in zip(rgb, palette[i])))
                pixels.append(cache[rgb])
            cell = indexed(size, palette)
            cell.putdata(pixels)
            # The original follower sheets all ground their feet at row 29.
            pos = ((32 - size[0]) // 2, 30 - size[1])
            assert pos[0] >= 0 and pos[1] >= 0
            sheet.paste(cell, (frame * 32 + pos[0], pos[1]))
            bounds.append([*pos, pos[0] + size[0], 30])
        for frame in (0, 2, 4):
            assert sheet.crop((frame * 32, 0, (frame + 1) * 32, 32)).tobytes() != sheet.crop(((frame + 1) * 32, 0, (frame + 2) * 32, 32)).tobytes(), (name, frame)
        sheet.save(target / "overworld.png", bits=4, transparency=0)
        shutil.copyfile(target / "normal.pal", target / "overworld_normal.pal")
        shutil.copyfile(target / "shiny.pal", target / "overworld_shiny.pal")
        labels.text((8, row * 290), name + " / normal, shiny", fill="white")
        for variant, pal in enumerate((palette, shiny)):
            display = sheet.copy()
            display.putpalette([c for rgb in pal for c in rgb])
            display = display.convert("RGBA").resize((768, 128), Image.Resampling.NEAREST)
            review.paste(display, (0, row * 290 + 20 + variant * 130), display)
        report[name] = {"source": str(source_dir / f"{name}-edited.png"), "bounds": bounds}
    review.save(source_dir / "grass-overworld-review.png")
    (source_dir / "export-report.json").write_text(json.dumps(report, indent=2) + "\n")
    print("Exported three grass-family overworld sheets (18 walking frames), normal and shiny palettes.")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source-dir", type=Path, default=ROOT / "outputs/prologue-merged/grass-art")
    export(parser.parse_args().source_dir)
