"""Lossless indexed packing and review layout for the front-only approved-concept translation."""

from collections import deque
from hashlib import sha256
import json
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

HERE = Path(__file__).resolve().parent
SPEC = json.loads((HERE / "pixel-design.json").read_text())
PALETTE = [tuple(bytes.fromhex(c[1:])) for c in SPEC["palette"]]


def main():
    with Image.open(HERE / "front-raw.png") as source:
        rgba = source.convert("RGBA")
    assert rgba.size == (64, 64)
    lookup = {color: index for index, color in enumerate(PALETTE) if index}
    pixels = []
    for r, g, b, a in rgba.get_flattened_data():
        assert a in (0, 255)
        assert not a or (r, g, b) in lookup
        pixels.append(lookup[r, g, b] if a else 0)
    image = Image.new("P", (64, 64), 0)
    image.putpalette([channel for rgb in PALETTE for channel in rgb])
    image.putdata(pixels)
    image.save(HERE / "front.png", bits=4, transparency=0)
    (HERE / "normal.pal").write_text("JASC-PAL\n0100\n16\n" +
                                      "".join("%d %d %d\n" % rgb for rgb in PALETTE))
    with Image.open(HERE / "front.png") as packed:
        assert packed.mode == "P" and packed.info.get("transparency") == 0
        assert packed.tobytes() == image.tobytes()
    raw = (HERE / "front.png").read_bytes()
    assert raw[24:26] == bytes((4, 3))
    assert all(channel % 8 == 0 for rgb in PALETTE for channel in rgb)
    occupied = {(x, y) for y in range(64) for x in range(64)
                if rgba.getpixel((x, y))[3]}
    queue = deque([next(iter(occupied))])
    reached = set(queue)
    while queue:
        x, y = queue.popleft()
        for dx, dy in ((-1, -1), (0, -1), (1, -1), (-1, 0), (1, 0), (-1, 1), (0, 1), (1, 1)):
            point = (x + dx, y + dy)
            if point in occupied and point not in reached:
                reached.add(point)
                queue.append(point)
    assert reached == occupied, "Detached pixels/clusters"
    bounds = rgba.getbbox()
    assert bounds and bounds[0] > 0 and bounds[1] > 0 and bounds[2] < 64 and bounds[3] < 64

    # Enlargement is only for presentation; the authored/game sprite remains 64x64.
    review = Image.new("RGB", (568, 458), "#E8E8E0")
    draw = ImageDraw.Draw(review)
    font = ImageFont.truetype("C:/Windows/Fonts/consola.ttf", 16)
    draw.text((20, 13), "FRONT / APPROVED DESIGN", font=font, fill="#303038")
    review.paste(rgba.resize((384, 384), Image.Resampling.NEAREST), (16, 45), rgba.resize((384, 384), Image.Resampling.NEAREST))
    draw.text((442, 174), "1x", font=font, fill="#303038")
    review.paste(rgba, (430, 202), rgba)
    draw.text((20, 431), "6x", font=font, fill="#303038")
    review.save(HERE / "front-review.png")
    report = {"size": [64, 64], "frames": 1, "palette_entries": 16,
              "png_depth": 4, "binary_alpha": True, "transparent_index": 0,
              "connected_components": 1, "bounds": bounds,
              "sha256": sha256(raw).hexdigest(), "runtime_installed": False}
    (HERE / "verification.json").write_text(json.dumps(report, indent=2) + "\n")
    print("PASS: one 64x64 front frame, 16 colors, 4bpp, binary alpha, no detached pixels or clipped edges")


if __name__ == "__main__":
    main()
