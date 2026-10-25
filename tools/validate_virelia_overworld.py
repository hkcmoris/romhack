#!/usr/bin/env python3
"""Validate dedicated Virelian follower sheets, palettes and native pointers.

Runs without Pillow or Git. Pass --base REF in a Git checkout to additionally
verify that existing original-species overworld assets remain byte-identical.
"""
import argparse
from pathlib import Path
import re
import subprocess
import sys

from virelia.localize_type_icons import read_png

ROOT = Path(__file__).resolve().parents[1]
FAMILIES = (
    ("POOCHYENA_VIRELIA", "PoochyenaVirelia", "poochyena/virelia", "poochyena"),
    ("MIGHTYENA_VIRELIA", "MightyenaVirelia", "mightyena/virelia", "mightyena"),
    ("WRAITHENA", "Wraithena", "wraithena", "mightyena"),
    ("NATU_VIRELIA", "NatuVirelia", "natu/virelia", "natu"),
    ("XATU_VIRELIA", "XatuVirelia", "xatu/virelia", "xatu"),
    ("NIMBATU", "Nimbatu", "nimbatu", "xatu"),
    ("EKANS_VIRELIA", "EkansVirelia", "ekans/virelia", "ekans"),
    ("ARBOK_VIRELIA", "ArbokVirelia", "arbok/virelia", "arbok"),
    ("INFERNABRA", "Infernabra", "infernabra", "arbok"),
)


def require(condition, message):
    if not condition:
        raise ValueError(message)


def palette(path):
    lines = path.read_text(encoding="utf-8").splitlines()
    require(lines[:3] == ["JASC-PAL", "0100", "16"] and len(lines) == 19,
            f"{path}: expected a 16-color JASC palette")
    colors = [tuple(map(int, line.split())) for line in lines[3:]]
    require(all(len(color) == 3 and all(0 <= c <= 255 for c in color) for color in colors),
            f"{path}: invalid palette channels")
    return colors


def validate(base=None):
    graphics = (ROOT / "src/data/graphics/pokemon.h").read_text(encoding="utf-8")
    tables = (ROOT / "src/data/object_events/object_event_pic_tables_followers.h").read_text(encoding="utf-8")
    species = (ROOT / "src/data/pokemon/species_info/virelia_families.h").read_text(encoding="utf-8")
    signatures = set()
    for name, symbol, folder, donor in FAMILIES:
        prefix = f"graphics/pokemon/{folder}"
        path = ROOT / prefix / "overworld.png"
        pixels, depth, chunks = read_png(path)
        require(depth == 4 and len(pixels) == 32 and all(len(row) == 192 for row in pixels),
                f"{name}: expected six 32x32 frames in a 192x32 indexed 4bpp sheet")
        chunk_map = dict(chunks)
        require(chunk_map.get(b"tRNS", b"")[:1] == b"\0"
                and all(alpha == 255 for alpha in chunk_map.get(b"tRNS", b"")[1:]),
                f"{name}: only palette index zero may be transparent")
        normal = palette(ROOT / prefix / "overworld_normal.pal")
        palette(ROOT / prefix / "overworld_shiny.pal")
        require(chunk_map[b"PLTE"] == bytes(channel for color in normal for channel in color),
                f"{name}: sheet palette indices must match the normal runtime palette")
        frames = []
        for frame in range(6):
            values = tuple(tuple(row[frame * 32:(frame + 1) * 32]) for row in pixels)
            require(any(value for row in values for value in row), f"{name}: empty frame {frame}")
            require(any(value == 0 for row in values for value in row),
                    f"{name}: frame {frame} has no transparent surrounding space")
            require(all(value < 16 for row in values for value in row), f"{name}: palette overflow")
            frames.append(values)
        for first in (0, 2, 4):
            require(frames[first] != frames[first + 1],
                    f"{name}: walking pair {first // 2} contains identical frames")
        signature = (tuple(map(tuple, pixels)), tuple(normal))
        require(signature not in signatures, f"{name}: sprite duplicates another Virelian stage")
        signatures.add(signature)
        donor_pixels, _, donor_chunks = read_png(ROOT / f"graphics/pokemon/{donor}/overworld.png")
        require(pixels != donor_pixels or chunk_map[b"PLTE"] != dict(donor_chunks)[b"PLTE"],
                f"{name}: sprite still uses the unmodified original appearance")

        required_graphics = (
            rf'gObjectEventPic_{symbol}\[\]\s*=\s*INCGFX_COMP\("{prefix}/overworld.png",\s*".4bpp",\s*"-mwidth 4 -mheight 4"\)',
            rf'gOverworldPalette_{symbol}\[\]\s*=\s*INCGFX_U16\("{prefix}/overworld_normal.pal",\s*".gbapal"\)',
            rf'gShinyOverworldPalette_{symbol}\[\]\s*=\s*INCGFX_U16\("{prefix}/overworld_shiny.pal",\s*".gbapal"\)',
        )
        for pattern in required_graphics:
            require(re.search(pattern, graphics), f"{name}: missing dedicated graphics or palette definition")
        table = re.search(rf'sPicTable_{symbol}\[\]\s*=\s*\{{(.*?)\}};', tables, re.S)
        require(table and re.search(rf'overworld_ascending_frames\(gObjectEventPic_{symbol},\s*4,\s*4\)', table[1]),
                f"{name}: missing dedicated compressed 32x32 frame table")
        record = re.search(rf'\[SPECIES_{name}\]\s*=\s*\{{(.*?)(?=\n\s*\[SPECIES_|\n#endif|\Z)', species, re.S)
        require(record, f"{name}: missing species definition")
        pattern = (rf'OVERWORLD\(\s*sPicTable_{symbol}\s*,\s*SIZE_32x32\s*,'
                   rf'\s*SHADOW_SIZE_\w+\s*,\s*TRACKS_\w+\s*,\s*sAnimTable_Following\s*,'
                   rf'\s*gOverworldPalette_{symbol}\s*,\s*gShinyOverworldPalette_{symbol}\s*\)')
        require(re.search(pattern, record[1]), f"{name}: species does not use its dedicated follower assets")

    if base:
        tracked = subprocess.check_output(["git", "ls-tree", "-r", "--name-only", base, "--", "graphics/pokemon"],
                                          cwd=ROOT, text=True).splitlines()
        originals = [path for path in tracked if re.search(r'/overworld[^/]*\.(png|pal)$', path)]
        require(originals, f"{base}: no original overworld assets found")
        changed = set(subprocess.check_output(["git", "diff", "--name-only", base, "--", "graphics/pokemon"],
                                              cwd=ROOT, text=True).splitlines())
        require(not changed.intersection(originals),
                "Original overworld assets changed: " + ", ".join(sorted(changed.intersection(originals))))
        print(f"PASS: {len(originals)} original overworld assets unchanged from {base}")
    print("PASS: 9 dedicated Virelian follower sheets, 54 frames, 18 palettes and native species pointers")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--base", help="optional Git ref whose original overworld assets must be unchanged")
    args = parser.parse_args()
    try:
        validate(args.base)
    except (AssertionError, KeyError, OSError, ValueError, subprocess.CalledProcessError) as error:
        print(f"FAIL: {error}", file=sys.stderr)
        sys.exit(1)
