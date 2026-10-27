#!/usr/bin/env python3
"""Install the approved canine v5 pixels, preserving runtime palette roles.

Requires Pillow, as do the art-set exporters. Run from any working directory.
Poochyena's diivi-overworld sheet supersedes its older game-ready walking sheet.
No drawing, resizing or nearest-color quantization is performed here.
"""
from pathlib import Path
import shutil

from PIL import Image

ROOT = Path(__file__).resolve().parents[2]
# Full-art Poochyena index -> shared engine icon palette 1 index.
POOCHYENA_ICON = (0, 15, 15, 15, 1, 15, 1, 4, 5, 12, 9, 10, 13, 13, 2, 3)
# Retain the established blue shiny: charcoal shades, then all five leaf shades
# in increasing brightness (old grass-family indices 1,2,3,4,5,6,14,7,8).
SHINY_COAT = (
    (24, 24, 40), (32, 32, 48), (48, 56, 80), (72, 80, 104),
    (32, 40, 80), (48, 64, 120), (56, 80, 144), (72, 104, 168),
    (112, 152, 216),
)


def read_palette(path):
    lines = path.read_text(encoding="utf-8").splitlines()
    assert lines[:3] == ["JASC-PAL", "0100", "16"] and len(lines) == 19, path
    return [tuple(map(int, line.split())) for line in lines[3:]]


def write_palette(path, colors):
    lines = ["JASC-PAL", "0100", "16"] + [" ".join(map(str, c)) for c in colors]
    path.write_bytes(("\r\n".join(lines) + "\r\n").encode("ascii"))


def install(species):
    source = ROOT / "art" / f"virelia_{species}_v5" / "game-ready"
    target = ROOT / "graphics/pokemon" / species / "virelia"
    for name in ("front.png", "front_2.png", "anim_front.png", "back.png", "icon.png",
                 "normal.pal", "overworld_normal.pal"):
        shutil.copyfile(source / name, target / name)
    walking = source
    if species == "poochyena":
        walking = source.parent / "diivi-overworld"
        assert read_palette(walking / "overworld_normal.pal") == read_palette(source / "normal.pal")
        art_icon = Image.open(source / "icon.png")
        icon = Image.new("P", art_icon.size)
        icon.putdata([POOCHYENA_ICON[index] for index in art_icon.get_flattened_data()])
        icon.putpalette([c for rgb in read_palette(ROOT / "graphics/pokemon/icon_palettes/pal1.pal") for c in rgb])
        icon.save(target / "icon.png", bits=4, transparency=0)
    shutil.copyfile(walking / "overworld.png", target / "overworld.png")
    normal = read_palette(target / "normal.pal")
    shiny = normal.copy()
    shiny[1:10] = SHINY_COAT
    # Keep the approved eyes, nose, teeth, and Mightyena's mint aura unchanged.
    write_palette(target / "shiny.pal", shiny)
    write_palette(target / "overworld_shiny.pal", shiny)
    assert read_palette(target / "overworld_normal.pal") == normal
    for name, size in (("front", (64, 64)), ("front_2", (64, 64)),
                       ("anim_front", (64, 128)), ("back", (64, 64)),
                       ("overworld", (192, 32)), ("icon", (32, 64))):
        path = target / f"{name}.png"
        im = Image.open(path)
        assert im.mode == "P" and im.size == size, path
        assert path.read_bytes()[24:26] == bytes((4, 3)), path
        assert im.info.get("transparency") == 0, path
        assert max(im.get_flattened_data()) < 16, path
        expected = normal if name != "icon" else read_palette(ROOT / "graphics/pokemon/icon_palettes/pal1.pal")
        assert im.getpalette() == [c for rgb in expected for c in rgb], path
    print(f"Installed {species}: battle frames, back, six walking frames, shared-palette icon and normal/shiny palettes")


if __name__ == "__main__":
    for species in ("poochyena", "mightyena"):
        install(species)
