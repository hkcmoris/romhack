#!/usr/bin/env python3
"""Localize shared LV/LV_2/NO font symbols without changing their text codes."""
from pathlib import Path
import re
import sys
import tempfile
from localize_type_icons import read_png, write_png

ROOT = Path(__file__).resolve().parents[2]
FONT_NAMES = {
    "SmallNarrow": "small_narrow", "Small": "small", "Narrow": "narrow",
    "Short": "short", "Normal": "normal", "Narrower": "narrower",
    "SmallNarrower": "small_narrower", "ShortNarrow": "short_narrow",
    "ShortNarrower": "short_narrower",
}


def main():
    check = "--check" in sys.argv
    source, _, _ = read_png(ROOT / "graphics/fonts/latin_small_narrower.png")
    font_source = (ROOT / "src/fonts.c").read_text(encoding="utf-8")
    widths = {m[1]: [int(n) for n in re.findall(r"\d+", m[2])] for m in re.finditer(r"gFont(\w+)LatinGlyphWidths\[\] = \{(.*?)\n\};", font_source, re.S)}
    codes = {c: int(n, 16) for c, n in re.findall(r"^'(.)'\s*=\s*([0-9A-F]{2})", (ROOT / "charmap.txt").read_text(encoding="utf-8").split("@ Hiragana")[0], re.M)}
    czech = (ROOT / "src/data/fonts/czech.inc").read_text(encoding="utf-8")
    glyphs = [int(x, 16) for x in re.findall(r"0x[0-9A-F]{4}", re.search(r"gFontSmallNarrowerCzechGlyphs\[\]\[32\] = \{(.*?)\n\};", czech, re.S)[1])]

    def source_pixel(code, x, y):
        if 0x3B <= code <= 0x4E:
            return glyphs[(code - 0x3B) * 32 + y // 8 * 16 + x // 8 * 8 + y % 8] >> (14 - x % 8 * 2) & 3
        return source[code // 16 * 16 + y][code % 16 * 16 + x]

    for name, filename in FONT_NAMES.items():
        path = ROOT / "graphics/fonts" / ("latin_" + filename + ".png")
        pixels, depth, chunks = read_png(path)
        zero = codes["0"]
        baseline = max(y for y in range(16) if any(pixels[zero // 16 * 16 + y][zero % 16 * 16 + x] == 1 for x in range(16)))
        source_baseline = max(y for y in range(16) if any(source_pixel(codes["r"], x, y) == 1 for x in range(4)))
        shift = baseline - source_baseline
        for target, text in [(0x34, "úr."), (0x105, "úr."), (0x108, "č.")]:
            width = 10 if target != 0x108 else widths[name][target]
            total = sum(widths["SmallNarrower"][codes[c]] for c in text)
            assert total <= width, (name, text, total, width)
            left, top = target % 16 * 16, target // 16 * 16
            for y in range(16):
                for x in range(16):
                    pixels[top + y][left + x] = 0
            start = (width - total) // 2
            for char in text:
                code = codes[char]
                for y in range(16):
                    for x in range(widths["SmallNarrower"][code]):
                        value = source_pixel(code, x, y)
                        if value in (1, 2):
                            assert 0 <= y + shift < 16, (name, char, y, shift)
                            pixels[top + y + shift][left + start + x] = value
                start += widths["SmallNarrower"][code]
        if check:
            with tempfile.TemporaryDirectory() as directory:
                generated = Path(directory) / path.name
                write_png(generated, pixels, depth, chunks)
                assert generated.read_bytes() == path.read_bytes(), f"Regenerate {path.relative_to(ROOT)}"
        else:
            write_png(path, pixels, depth, chunks)

    def change_widths(match):
        values = 0
        def number(item):
            nonlocal values
            index = values
            values += 1
            return "10" if index in (0x34, 0x105) else item[0]
        return match[1] + re.sub(r"\d+", number, match[2]) + match[3]
    updated = re.sub(r"(gFont\w+LatinGlyphWidths\[\] = \{)(.*?)(\n\};)", change_widths, font_source, flags=re.S)
    if check:
        assert updated == font_source, "Regenerate shared level-symbol widths"
    else:
        (ROOT / "src/fonts.c").write_text(updated, encoding="utf-8", newline="\n")
    print("Checked shared Czech úr./č. symbols in all nine Latin fonts." if check else "Rendered shared Czech úr./č. symbols in all nine Latin fonts.")


if __name__ == "__main__":
    main()
