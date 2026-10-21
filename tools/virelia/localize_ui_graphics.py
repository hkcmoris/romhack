#!/usr/bin/env python3
"""Render fixed Czech UI labels from the repository's native bitmap font.

Indexed palettes, image dimensions and GBA tile positions remain unchanged.
Sprite frames use the native one dimensional OBJ tile order. Compact eight
pixel labels share a baseline and compress only the high accent area.
"""
from collections import Counter
from pathlib import Path
import re
import struct
import sys
import tempfile
from localize_type_icons import read_png, write_png as encode_png

ROOT = Path(__file__).resolve().parents[2]


def write_png(path, pixels, depth, chunks):
    if "--check" not in sys.argv:
        encode_png(path, pixels, depth, chunks)
        return
    with tempfile.TemporaryDirectory() as directory:
        generated = Path(directory) / path.name
        encode_png(generated, pixels, depth, chunks)
        assert generated.read_bytes() == path.read_bytes(), f"Regenerate {path.relative_to(ROOT)}"


def main():
    font, _, _ = read_png(ROOT / "graphics/fonts/latin_small_narrower.png")
    fonts = (ROOT / "src/fonts.c").read_text(encoding="utf-8")
    widths = [int(x) for x in re.findall(r"\d+", re.search(r"gFontSmallNarrowerLatinGlyphWidths\[\] = \{(.*?)\n\};", fonts, re.S)[1])]
    czech = (ROOT / "src/data/fonts/czech.inc").read_text(encoding="utf-8")
    glyphs = [int(x, 16) for x in re.findall(r"0x[0-9A-F]{4}", re.search(r"gFontSmallNarrowerCzechGlyphs\[\]\[32\] = \{(.*?)\n\};", czech, re.S)[1])]
    codes = {c: int(n, 16) for c, n in re.findall(r"^'(.)'\s*=\s*([0-9A-F]{2})", (ROOT / "charmap.txt").read_text(encoding="utf-8").split("@ Hiragana")[0], re.M)}

    def glyph(code, x, y):
        if 0x3B <= code <= 0x4E:
            word = glyphs[(code - 0x3B) * 32 + y // 8 * 16 + x // 8 * 8 + y % 8]
            return word >> (14 - x % 8 * 2) & 3
        return font[code // 16 * 16 + y][code % 16 * 16 + x]

    def label(pixels, box, text, ink, background=None, compact=False, centered=False):
        left, top, width, height = box
        total = sum(widths[codes[c]] for c in text)
        assert total <= width, (text, total, width)
        if background is None:
            background = Counter(pixels[y][x] for y in range(top, top + height) for x in range(left, left + width)).most_common(1)[0][0]
        for y in range(top, top + height):
            for x in range(left, left + width):
                pixels[y][x] = background
        start = left + ((width - total) // 2 if centered else 0)
        for char in text:
            code = codes[char]
            accent_rows = [y for y in range(4) if any(glyph(code, x, y) == 1 for x in range(widths[code]))]
            accent_top = min(accent_rows, default=0)
            for y in range(11):
                # One-tile headings have eight rows. Reserve two distinct
                # accent rows so a caron remains a V, not a horizontal bar.
                # Compress the seven native body rows to six below them.
                if compact:
                    target_y = min(1, max(0, y - accent_top)) if y < 4 else 2 + (y - 4) * 6 // 7
                else:
                    target_y = y
                if target_y >= height:
                    continue
                for x in range(widths[code]):
                    if glyph(code, x, y) == 1:
                        pixels[top + target_y][start + x] = ink
            start += widths[code]

    assets = {
        "graphics/pokedex/menu.png": [
            ((16, 16, 24, 16), "OBLAST", 15),
            ((58, 16, 24, 16), "HLAS", 15),
            ((96, 16, 24, 16), "VELIK.", 15),
            ((18, 32, 38, 16), "ZPĚT", 1),
            ((64, 32, 40, 16), "ZPĚT", 15),
            ((0, 48, 72, 16), "ZPĚT NA SEZNAM", 1),
            ((0, 64, 48, 16), "ZAČÁTEK", 1),
            ((0, 80, 64, 16), "KONEC", 1),
            ((0, 96, 84, 16), "ZPĚT DO DEXU", 1),
            ((0, 112, 80, 16), "ZAVŘÍT DEX", 1),
        ],
        "graphics/pokedex/search_menu.png": [
            ((5, 18, 30, 12), "REŽIM", 3),
            ((45, 18, 30, 12), "ŘAZENÍ", 3),
            ((85, 18, 30, 12), "JMÉNO", 3),
            ((5, 34, 30, 12), "BARVA", 3),
            ((45, 34, 30, 12), "TYP", 3),
            ((85, 34, 30, 12), "ZPĚT", 3),
            ((45, 50, 30, 12), "ZMĚNIT", 3),
            ((85, 50, 30, 12), "HLEDAT", 3),
        ],
        "graphics/summary_screen/tiles.png": [
            ((2, 48, 11, 8), "ZK.", 39),
            ((2, 64, 66, 8), "PROFIL", 2),
            ((2, 72, 66, 8), "SCHOPNOST", 2),
            ((74, 64, 50, 8), "PŘEDMĚT", 2),
            ((74, 72, 50, 8), "STUHY", 2),
            ((74, 80, 50, 8), "STAT.", 2),
            ((74, 88, 34, 8), "ZKUŠ.", 2),
            ((2, 88, 66, 8), "STAV", 2),
            ((2, 96, 66, 8), "ÚTOKY", 2),
            ((2, 104, 66, 8), "POPIS", 2),
            ((2, 112, 82, 8), "POZNÁMKY", 2),
            ((82, 96, 44, 8), "ÚČINEK", 2),
        ],
        "graphics/trainer_card/tiles.png": [
            ((48, 0, 42, 8), "ODZNAKY", 1),
            ((2, 68, 88, 8), "KARTA TRENÉRA", 15),
        ],
    }
    for relative, labels in assets.items():
        path = ROOT / relative
        pixels, depth, chunks = read_png(path)
        for box, text, ink in labels:
            label(pixels, box, text, ink, compact=box[3] == 8)
        write_png(path, pixels, depth, chunks)

    # Pokedex interface labels are OBJ frames, not adjacent image rectangles.
    path = ROOT / "graphics/pokedex/interface.png"
    pixels, depth, chunks = read_png(path)
    columns = len(pixels[0]) // 8
    for first_tile, frame_width, frame_height, box, text in [
        (40, 32, 16, (0, 2, 32, 12), "HLEDAT"),
        (56, 32, 16, (0, 2, 32, 12), "MENU"),
        (64, 64, 32, (20, 3, 32, 11), "SPATŘENÍ"),
        (96, 64, 32, (20, 3, 32, 11), "CHYCENÍ"),
        (168, 32, 16, (0, 1, 32, 12), "NÁRODNÍ"),
    ]:
        frame = [[0] * frame_width for _ in range(frame_height)]
        for y in range(frame_height):
            for x in range(frame_width):
                tile = first_tile + y // 8 * (frame_width // 8) + x // 8
                frame[y][x] = pixels[tile // columns * 8 + y % 8][tile % columns * 8 + x % 8]
        label(frame, box, text, 1, background=0)
        for y in range(frame_height):
            for x in range(frame_width):
                tile = first_tile + y // 8 * (frame_width // 8) + x // 8
                pixels[tile // columns * 8 + y % 8][tile % columns * 8 + x % 8] = frame[y][x]
    write_png(path, pixels, depth, chunks)
    # The unknown-area panel consists of three adjacent 32x32 OBJ sprites.
    path = ROOT / "graphics/pokedex/area_unknown.png"
    pixels, depth, chunks = read_png(path)
    frame = [[pixels[(x // 32) * 32 + y][x % 32] for x in range(96)] for y in range(32)]
    label(frame, (6, 8, 80, 12), "NEZNÁMÁ OBLAST", 15)
    for y in range(32):
        for x in range(96):
            pixels[(x // 32) * 32 + y][x % 32] = frame[y][x]
    write_png(path, pixels, depth, chunks)

    # Party and summary share 32x8 OBJ status frames. Healthboxes use 24x8
    # frames with four different palette indices (one per battler). Keep those
    # indices, the rounded ends and the surrounding healthbox tiles unchanged.
    # PKRS is the proper name Pokérus, rather than an English status label.
    status_labels = ("OTR", "PAR", "SPÍ", "MRZ", "POP", None, "MDL", "OMR")
    path = ROOT / "graphics/interface/status_icons.png"
    pixels, depth, chunks = read_png(path)
    for index, text in enumerate(status_labels):
        if text is not None:
            label(pixels, (8, index * 8, 17, 8), text, 2,
                  compact=True, centered=True)
    write_png(path, pixels, depth, chunks)

    for suffix in ("", "2", "3", "4"):
        path = ROOT / f"graphics/battle_interface/status{suffix}.png"
        pixels, depth, chunks = read_png(path)
        for index, text in enumerate(("OTR", "PAR", "SPÍ", "MRZ", "POP", "OMR")):
            label(pixels, (2, index * 8, 16, 8), text, 2,
                  compact=True, centered=True)
        write_png(path, pixels, depth, chunks)

    # A 32x32 OBJ frame stores the button pictogram above MOVE INFO. One
    # Czech word fits without touching either the button or the window border.
    for button in ("l", "r"):
        path = ROOT / f"graphics/battle_interface/move_info_window_{button}.png"
        pixels, depth, chunks = read_png(path)
        label(pixels, (5, 10, 22, 13), "POPIS", 5, background=13, centered=True)
        write_png(path, pixels, depth, chunks)
    # Storage labels span non-adjacent source tiles. Render them in their
    # native screen layout, then scatter changed pixels back to the shared
    # menu tiles. Tile IDs, palettes and storage/save layouts stay unchanged.
    path = ROOT / "graphics/pokemon_storage/menu.png"
    pixels, depth, chunks = read_png(path)
    for tilemap, columns, labels in [
        ("pkmn_data.bin", 8, [
            ((4, 2, 56, 11), "ÚDAJE", 14, 2),
            ((4, 18, 56, 11), "ÚDAJE", 14, 2),
        ]),
        ("close_box_button.bin", 9, [
            ((5, 2, 61, 11), "ZAVŘÍT BOX", 14, 8),
            ((5, 18, 61, 11), "ZAVŘÍT BOX", 9, 8),
        ]),
        ("party_menu.bin", 12, [
            ((56, 138, 32, 12), "ZPĚT", 24, 23),
            ((4, 162, 62, 11), "TÝM", 24, 23),
        ]),
    ]:
        raw = (ROOT / "graphics/pokemon_storage" / tilemap).read_bytes()
        tiles = struct.unpack("<" + "H" * (len(raw) // 2), raw)
        height, width = len(tiles) // columns * 8, columns * 8
        frame = [[0] * width for _ in range(height)]
        locations = {}
        for y in range(height):
            for x in range(width):
                tile = tiles[y // 8 * columns + x // 8]
                index = (tile & 0x3FF) - 0x100
                tx = 7 - x % 8 if tile & 0x400 else x % 8
                ty = 7 - y % 8 if tile & 0x800 else y % 8
                sx, sy = index % 16 * 8 + tx, index // 16 * 8 + ty
                frame[y][x] = pixels[sy][sx]
                locations[x, y] = sx, sy
        before = [row[:] for row in frame]
        for box, text, ink, background in labels:
            label(frame, box, text, ink, background=background, centered=True)
        for y in range(height):
            for x in range(width):
                if frame[y][x] != before[y][x]:
                    sx, sy = locations[x, y]
                    pixels[sy][sx] = frame[y][x]
    write_png(path, pixels, depth, chunks)
    print("Rendered Czech Pokédex, summary, status, battle and storage labels with native font glyphs.")


if __name__ == "__main__":
    main()
