#!/usr/bin/env python3
"""Replace only the title's prompt strip, preserving its copyright and palette."""

import re

from localize_type_icons import ROOT, read_png, write_png


def main():
    path = ROOT / "graphics/title_screen/press_start.png"
    pixels, depth, chunks = read_png(path)
    assert len(pixels) == 24 and len(pixels[0]) == 160
    copyright_pixels = [row[:] for row in pixels[8:]]
    font, _, _ = read_png(ROOT / "graphics/fonts/latin_small_narrower.png")
    fonts = (ROOT / "src/fonts.c").read_text(encoding="utf-8")
    data = re.search(r"gFontSmallNarrowerLatinGlyphWidths\[\] = \{(.*?)\n\};", fonts, re.S)
    widths = [int(value) for value in re.findall(r"\d+", data[1])]
    codes = {" ": 0, **{chr(ord("A") + i): 0xBB + i for i in range(26)}}
    label = "STISKNI START"
    # Double the native small glyphs horizontally to match the wide title text.
    width = sum(widths[codes[char]] * 2 for char in label)
    x = (160 - width) // 2
    assert x >= 8 and x + width <= 160
    for y in range(8):
        pixels[y] = [0] * 160
    for char in label:
        code = codes[char]
        for y in range(8):
            for gx in range(widths[code]):
                value = font[(code // 16) * 16 + y + 4][(code % 16) * 16 + gx]
                if value in (1, 2):
                    # Keep the existing white-to-grey title palette gradient.
                    color = (5 if y < 3 else 4 if y < 5 else 3) if value == 1 else 1
                    pixels[y][x + gx * 2:x + gx * 2 + 2] = [color, color]
        x += widths[code] * 2
    assert pixels[8:] == copyright_pixels
    write_png(path, pixels, depth, chunks)
    print("Rendered STISKNI START in the original 160x8 strip; copyright unchanged.")


if __name__ == "__main__":
    main()
