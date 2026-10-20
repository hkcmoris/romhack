#!/usr/bin/env python3
"""Render Czech labels inside the existing indexed naming-screen buttons."""

import re

from localize_type_icons import ROOT, read_png, write_png


def load_font(style, filename):
    font, _, _ = read_png(ROOT / "graphics/fonts" / filename)
    source = (ROOT / "src/fonts.c").read_text(encoding="utf-8")
    data = re.search(r"gFont" + style + r"LatinGlyphWidths\[\] = \{(.*?)\n\};", source, re.S)
    widths = [int(value) for value in re.findall(r"\d+", data[1])]
    source = (ROOT / "src/data/fonts/czech.inc").read_text(encoding="utf-8")
    data = re.search(r"gFont" + style + r"CzechGlyphs\[\]\[32\] = \{(.*?)\n\};", source, re.S)
    czech = [int(value, 16) for value in re.findall(r"0x[0-9A-F]{4}", data[1])]

    def pixel(code, x, y):
        if 0x3B <= code <= 0x4E:
            word = czech[(code - 0x3B) * 32 + (y // 8) * 16 + (x // 8) * 8 + y % 8]
            return (word >> (14 - (x % 8) * 2)) & 3
        return font[(code // 16) * 16 + y][(code % 16) * 16 + x]

    return widths, pixel


def main():
    text = (ROOT / "charmap.txt").read_text(encoding="utf-8").split("@ Hiragana")[0]
    codes = {char: int(code, 16) for char, code in re.findall(r"^'(.)'\s*=\s*([0-9A-F]{2})", text, re.M)}
    widths, pixel = load_font("Small", "latin_small.png")
    path = ROOT / "graphics/naming_screen/back_button.png"
    image, depth, chunks = read_png(path)
    assert len(image) == 24 and len(image[0]) == 40
    # The gold button and grey frame remain unchanged outside the lettering.
    for y in range(4, 13):
        for x in range(8, 33):
            image[y][x] = 12
    label = "ZPĚT"
    width = sum(widths[codes[char]] + 1 for char in label) - 1
    x = (40 - width) // 2
    ink = []
    for char in label:
        code = codes[char]
        # Five body rows retain the native letter shapes, with separate room
        # above the E for its caron and below the baseline for the shadow.
        for dy, sy in enumerate((4, 6, 7, 8, 10)):
            for gx in range(widths[code]):
                if pixel(code, gx, sy) == 1:
                    ink.append((x + gx, 7 + dy))
        for sy in (1, 2):
            for gx in range(widths[code]):
                if pixel(code, gx, sy) == 1:
                    ink.append((x + gx, sy + 3))
        x += widths[code] + 1
    for x, y in ink:
        assert 8 <= x <= 32 and 4 <= y <= 11
        image[y + 1][x + 1] = 13
    for x, y in ink:
        image[y][x] = 1

    # Preserve the native B-button pictogram; replace only its English hint.
    narrow_widths, narrow_pixel = load_font("SmallNarrower", "latin_small_narrower.png")
    for y in range(16, 23):
        for x in range(11, 36):
            image[y][x] = 15
    x = 11
    for char in "SMAZAT":
        code = codes[char]
        for y in range(7):
            for gx in range(narrow_widths[code]):
                if narrow_pixel(code, gx, y + 4) == 1:
                    image[y + 16][x + gx] = 1
        x += narrow_widths[code]
    assert x <= 36
    write_png(path, image, depth, chunks)

    for name, label, bg, shadow in (
        ("lower", "malá", 9, 8),
        ("upper", "VELKÁ", 6, 7),
        ("others", "další", 11, 10),
    ):
        path = ROOT / ("graphics/naming_screen/page_swap_" + name + ".png")
        image, depth, chunks = read_png(path)
        assert len(image) == 8 and len(image[0]) == 40
        glyph_widths, glyph_pixel = (narrow_widths, narrow_pixel) if name == "upper" else (widths, pixel)
        width = sum(glyph_widths[codes[char]] for char in label)
        assert width <= 24
        for y in range(8):
            for x in range(24):
                image[y][x] = bg
        x = (24 - width) // 2
        ink = []
        for char in label:
            code = codes[char]
            if name == "upper":
                # Keep both native acute-mark rows above five body rows. The
                # eight-pixel sprite has no room for a full-height capital.
                for dy, sy in enumerate((1, 2, 4, 5, 7, 9, 10)):
                    for gx in range(glyph_widths[code]):
                        if glyph_pixel(code, gx, sy) == 1:
                            ink.append((x + gx, dy))
            else:
                for y in range(8):
                    for gx in range(glyph_widths[code]):
                        value = glyph_pixel(code, gx, y + 3)
                        if value in (1, 2):
                            image[y][x + gx] = 1 if value == 1 else shadow
            x += glyph_widths[code]
        for x, y in ink:
            assert x + 1 < 24 and y + 1 < 8
            image[y + 1][x + 1] = shadow
        for x, y in ink:
            image[y][x] = 1
        write_png(path, image, depth, chunks)
    print("Rendered Czech naming labels with original palettes and button geometry.")


if __name__ == "__main__":
    main()
