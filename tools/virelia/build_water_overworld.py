#!/usr/bin/env python3
"""Pack imagegen-edited original walking sprites into native follower sheets.

This export only samples, registers and palette-encodes approved artwork. It
does not invent or paint details. Source boards were copied from original Natu
and Xatu and edited with built-in imagegen against approved battle references.
The six cells in each authoring board are down/down, up/up and left/left.
"""

import argparse
import hashlib
import json
import shutil
from pathlib import Path

from PIL import Image, ImageDraw

ROOT = Path(__file__).resolve().parents[2]
SPECS = {
    "natu": ("natu", "natu/virelia", (18, 19)),
    "xatu": ("xatu", "xatu/virelia", (23, 25)),
    "nimbatu": ("xatu", "nimbatu", (27, 28)),
}


def palette(path):
    lines = path.read_text().splitlines()
    assert lines[:3] == ["JASC-PAL", "0100", "16"]
    return [tuple(map(int, line.split())) for line in lines[3:19]]


def indexed(size, colors):
    result = Image.new("P", size, 0)
    result.putpalette([channel for color in colors for channel in color])
    result.info["transparency"] = 0
    return result


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def export(name, source_dir, review_dir):
    original, destination, fit = SPECS[name]
    original_path = ROOT / "graphics/pokemon" / original / "overworld.png"
    source = source_dir / (name + "-generated.png")
    target = ROOT / "graphics/pokemon" / destination
    colors = palette(target / "normal.pal")
    board = Image.open(source).convert("RGBA")
    assert board.width * 3 == board.height * 2, board.size
    cell = board.width // 2
    original_image = Image.open(original_path)
    sheet = indexed((192, 32), colors)
    frames = []
    bounds = []

    for index in range(6):
        x, y = index % 2 * cell, index // 2 * cell
        frame = board.crop((x, y, x + cell, y + cell))
        alpha = frame.getchannel("A").point(lambda value: 255 if value >= 240 else 0)
        box = alpha.getbbox()
        assert box is not None, (name, index)
        frame.putalpha(alpha)
        frame = frame.crop(box)
        frames.append(frame)

    # Use one scale for each animation pair. Register to the original center
    # and y=30 ground line without changing the source walking foot poses.
    for pair in range(3):
        pair_frames = frames[pair * 2:pair * 2 + 2]
        scale = min(fit[0] / max(f.width for f in pair_frames),
                    fit[1] / max(f.height for f in pair_frames))
        for step, frame in enumerate(pair_frames):
            index = pair * 2 + step
            width, height = round(frame.width * scale), round(frame.height * scale)
            frame = frame.resize((width, height), Image.Resampling.NEAREST)
            base = original_image.crop((index * 32, 0, (index + 1) * 32, 32))
            base_box = base.point(lambda value: 255 if value else 0).getbbox()
            center = (base_box[0] + base_box[2]) / 2
            left = round(center - width / 2)
            top = 30 - height
            assert 0 < left and left + width < 32 and top >= 1
            native = indexed((width, height), colors)
            values = []
            for pixel in frame.get_flattened_data():
                if pixel[3] == 0:
                    values.append(0)
                else:
                    values.append(min(range(1, 14), key=lambda i: sum(
                        (pixel[channel] - colors[i][channel]) ** 2 for channel in range(3))))
            native.putdata(values)
            sheet.paste(native, (index * 32 + left, top))
            bounds.append([left, top, left + width, top + height])

    output = target / "overworld.png"
    sheet.save(output, bits=4, transparency=0)
    for suffix in ("normal", "shiny"):
        shutil.copyfile(target / "normal.pal", target / ("overworld_" + suffix + ".pal"))
    loaded = Image.open(output)
    assert loaded.mode == "P" and loaded.size == (192, 32)
    assert loaded.info.get("transparency") == 0
    assert max(loaded.get_flattened_data()) < 16
    assert output.read_bytes()[24] == 4, "PNG must encode four-bit palette indices"
    for pair in range(3):
        a = loaded.crop((pair * 64, 0, pair * 64 + 32, 32))
        b = loaded.crop((pair * 64 + 32, 0, pair * 64 + 64, 32))
        assert a.tobytes() != b.tobytes(), (name, pair, "lost walking animation")
    assert digest(target / "overworld_normal.pal") == digest(target / "overworld_shiny.pal")
    loaded.convert("RGBA").resize((1152, 192), Image.Resampling.NEAREST).save(review_dir / (name + "-review.png"))
    return dict(name=name, original=str(original_path.relative_to(ROOT)),
                original_sha256=digest(original_path), source=str(source.relative_to(ROOT)),
                source_sha256=digest(source), output=str(output.relative_to(ROOT)),
                output_sha256=digest(output), frame_bounds=bounds,
                frame_order=["down0", "down1", "up0", "up1", "left0", "left1"],
                palette="Approved normal battle palette; shiny intentionally identical",
                alpha_threshold=240)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source-dir", type=Path, default=ROOT / "outputs/prologue-merged/water-overworld")
    args = parser.parse_args()
    args.source_dir = args.source_dir.resolve()
    rows = [export(name, args.source_dir, args.source_dir) for name in SPECS]
    review = Image.new("RGBA", (1152, 3 * 224), (35, 42, 45, 255))
    draw = ImageDraw.Draw(review)
    for row, name in enumerate(SPECS):
        draw.text((8, row * 224 + 8), name.upper() + " - DOWN 0/1 | UP 0/1 | LEFT 0/1", fill="white")
        strip = Image.open(args.source_dir / (name + "-review.png"))
        review.alpha_composite(strip, (0, row * 224 + 32))
    review.convert("RGB").save(args.source_dir / "water-overworld-review.png")
    (args.source_dir / "water-overworld-manifest.json").write_text(json.dumps(rows, indent=2) + "\n")
    print(json.dumps({"passed": True, "species": len(rows), "frames": 18, "review": str(args.source_dir / "water-overworld-review.png")}))


if __name__ == "__main__":
    main()
