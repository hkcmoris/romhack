"""Pack Aseprite MCP exports into exact GBA formats; never redraw the artwork.

Run with a Python environment containing Pillow. The editable Aseprite files
and their native-resolution MCP exports are the artwork source of truth.
"""

from hashlib import sha256
import json
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[1]
TARGET = ROOT / "graphics/pokemon/poochyena/virelia"
SPEC = json.loads((HERE / "pixel-design.json").read_text())
NORMAL = [tuple(bytes.fromhex(color[1:])) for color in SPEC["palette"]]
SHINY = [tuple(bytes.fromhex(color[1:])) for color in SPEC["shinyPalette"]]
ICON = [tuple(map(int, line.split())) for line in
        (ROOT / "graphics/pokemon/icon_palettes/pal1.pal").read_text().splitlines()[3:19]]
# Preserve the grass identity in the shared palette: RGB-nearest matching would
# turn the muted sage fur gray. Keep the engine's palette 1 completely unchanged.
ICON_MAP = [0, 15, 15, 14, 4, 5, 11, 15, 4, 5, 9, 11, 9, 10, 3, 9]


def palette_file(path, colors):
    path.write_text("JASC-PAL\n0100\n16\n" +
                    "".join("%d %d %d\n" % rgb for rgb in colors), encoding="ascii")


def indexed(source, colors=NORMAL):
    """Preserve every MCP-exported pixel; only the shared icon palette is remapped."""
    rgba = source.convert("RGBA")
    lookup = {rgb: i for i, rgb in enumerate(colors) if i}
    values = []
    for r, g, b, alpha in rgba.get_flattened_data():
        assert alpha in (0, 255), "Unexpected antialiasing/translucency"
        if not alpha:
            values.append(0)
        else:
            assert (r, g, b) in lookup, f"Color outside the authored palette: {(r, g, b)}"
            values.append(lookup[r, g, b])
    result = Image.new("P", rgba.size, 0)
    result.putpalette([v for rgb in colors for v in rgb])
    result.putdata(values)
    result.info["transparency"] = 0
    return result


def load(name):
    with Image.open(HERE / "exports" / name) as image:
        return indexed(image)


def sheet(frames, vertical=True):
    w, h = frames[0].size
    result = Image.new("P", (w if vertical else w * len(frames),
                             h * len(frames) if vertical else h), 0)
    result.putpalette(frames[0].getpalette())
    result.info["transparency"] = 0
    for i, frame in enumerate(frames):
        result.paste(frame, (0, h * i) if vertical else (w * i, 0))
    return result


def save(image, name, directory=TARGET):
    image.save(directory / name, bits=4, transparency=0, optimize=False)


def variant(image, colors):
    result = image.copy()
    result.putpalette([v for rgb in colors for v in rgb])
    return result


def review(front, back, icons, walkers):
    board = Image.new("RGB", (1032, 860), "#17242B")
    draw = ImageDraw.Draw(board)
    font = ImageFont.truetype("C:/Windows/Fonts/consola.ttf", 17)
    title = ImageFont.truetype("C:/Windows/Fonts/consolab.ttf", 24)

    def label(x, y, text, big=False):
        draw.text((x, y), text, font=title if big else font, fill="#DAE6D6")

    def tile(image, x, y, scale):
        rgba = image.convert("RGBA")
        enlarged = rgba.resize((rgba.width * scale, rgba.height * scale), Image.Resampling.NEAREST)
        draw.rectangle((x, y, x + enlarged.width - 1, y + enlarged.height - 1), fill="#26363D")
        board.paste(enlarged, (x, y), enlarged)

    label(24, 18, "VIRELIAN POOCHYENA / V2", True)
    label(24, 49, "ASEPRITE MCP  |  SAGE FUR / FERN RUFF / CHARCOAL PAWS")
    for i, image in enumerate([*front, back]):
        label(24 + i * 276, 83, ["FRONT 01", "FRONT 02", "BACK / BATTLE CROP"][i])
        tile(image, 24 + i * 276, 110, 4)
    label(870, 83, "PARTY ICON")
    for i in range(2):
        tile(icons.crop((0, 32 * i, 32, 32 * (i + 1))), 882, 118 + 118 * i, 3)
    label(24, 388, "OVERWORLD / SIX 32x32 FRAMES / RIGHT-FACING IS MIRRORED BY THE ENGINE")
    for i, image in enumerate(walkers):
        label(24 + i * 166, 420, ["DOWN 1", "DOWN 2", "UP 1", "UP 2", "LEFT 1", "LEFT 2"][i])
        tile(image, 24 + i * 166, 447, 4)
    label(24, 598, "SHINY / BLUE SAGE AND VIOLET LEAVES")
    tile(variant(front[0], SHINY), 24, 630, 3)
    tile(variant(back, SHINY), 240, 630, 3)
    for i, image in enumerate(walkers):
        tile(variant(image, SHINY), 456 + i * 90, 674, 2)
    for colors, x in ((NORMAL, 24), (SHINY, 456)):
        for i, rgb in enumerate(colors[1:]):
            draw.rectangle((x + i * 25, 830, x + i * 25 + 22, 844), fill=rgb)
    board.save(HERE / "previews" / "review.png")

    compare = Image.new("RGB", (864, 380), "#26363D")
    text = ImageDraw.Draw(compare)
    text.text((16, 8), "BEFORE", fill="white", font=title)
    text.text((448, 8), "V2 / ASEPRITE MCP", fill="white", font=title)
    for name, image, x in (("front.png", front[0], 0), ("back.png", back, 216)):
        with Image.open(HERE / "before" / name) as old:
            old = old.convert("RGBA").resize((192, 192), Image.Resampling.NEAREST)
            compare.paste(old, (16 + x, 70), old)
        new = image.convert("RGBA").resize((192, 192), Image.Resampling.NEAREST)
        compare.paste(new, (448 + x, 70), new)
    text.text((16, 300), "Original files preserved in before/", fill="white", font=font)
    text.text((448, 300), "All game-ready assets installed", fill="white", font=font)
    compare.save(HERE / "previews" / "before-after.png")


def audit(files, front, back, walkers, icon_frames):
    records = {}
    expected = {"front.png": (64, 64), "front_2.png": (64, 64),
                "anim_front.png": (64, 128), "back.png": (64, 64),
                "overworld.png": (192, 32), "icon.png": (32, 64)}
    for name, size in expected.items():
        path = TARGET / name
        raw = path.read_bytes()
        assert raw[24:26] == bytes((4, 3)), f"{name}: not 4-bit indexed PNG"
        with Image.open(path) as image:
            assert image.mode == "P" and image.size == size
            assert image.info.get("transparency") == 0
            assert len(image.getpalette()) == 48
            assert max(image.get_flattened_data()) < 16
            assert image.getpalette() == [v for rgb in (ICON if name == "icon.png" else NORMAL) for v in rgb]
            if name != "icon.png":
                assert image.convert("RGBA").tobytes() == files[name].convert("RGBA").tobytes()
            records[name] = {"size": size, "depth": 4,
                             "sha256": sha256(raw).hexdigest()}
    for images, name in ((front, "front"), (icon_frames, "icons")):
        assert images[0].tobytes() != images[1].tobytes(), f"Identical {name} poses"
    for start in (0, 2, 4):
        assert walkers[start].tobytes() != walkers[start + 1].tobytes()
    assert front[0].crop((0, 49, 64, 64)).tobytes() == front[1].crop((0, 49, 64, 64)).tobytes(), "Front paws moved"
    assert back.convert("RGBA").getbbox()[3] == 64, "Back portrait must meet the bottom crop"
    for frame in [*front, *walkers, *icon_frames]:
        bounds = frame.convert("RGBA").getbbox()
        assert bounds and bounds[0] > 0 and bounds[1] > 0
        assert bounds[2] < frame.width and bounds[3] < frame.height
    for frame in walkers:
        assert frame.convert("RGBA").getbbox()[3] == 30, "Follower ground line changed"
    for name in ("normal.pal", "shiny.pal", "overworld_normal.pal", "overworld_shiny.pal"):
        path = TARGET / name
        assert len(path.read_text().splitlines()) == 19
        records[name] = {"sha256": sha256(path.read_bytes()).hexdigest()}
    records["checks"] = {"binary_alpha": True, "exact_palette_match": True,
                         "front_paws_stationary": True, "distinct_animation_pairs": True,
                         "follower_ground_y": 30, "back_bottom_crop": True,
                         "battle_and_overworld_colors_rgb555_exact": all(c % 8 == 0 for rgb in NORMAL + SHINY for c in rgb),
                         "icon_shared_palette": 1}
    assert records["checks"]["battle_and_overworld_colors_rgb555_exact"]
    (HERE / "verification.json").write_text(json.dumps(records, indent=2) + "\n")
    print("PASS: 11 frames, 6 indexed PNGs, 4 palettes; exact colors, transparency, packing, bounds and distinct poses")


def main():
    front = [load(f"front-{i}.png") for i in (1, 2)]
    back = load("back-1.png")
    walkers = [load(f"overworld-{i}.png") for i in range(1, 7)]
    icon_frames = [load(f"icon-{i}.png") for i in (1, 2)]
    art_icons = sheet(icon_frames)
    save(art_icons, "icon_full_palette.png", HERE / "exports")
    game_icons = art_icons.copy()
    game_icons.putpalette([v for rgb in ICON for v in rgb])
    game_icons.putdata([ICON_MAP[index] for index in art_icons.get_flattened_data()])
    files = {"front.png": front[0], "front_2.png": front[1],
             "anim_front.png": sheet(front), "back.png": back,
             "overworld.png": sheet(walkers, False),
             "icon.png": game_icons}
    for name, image in files.items():
        save(image, name)
    for name in ("normal.pal", "overworld_normal.pal"):
        palette_file(TARGET / name, NORMAL)
    for name in ("shiny.pal", "overworld_shiny.pal"):
        palette_file(TARGET / name, SHINY)
    review(front, back, files["icon.png"], walkers)
    audit(files, front, back, walkers, icon_frames)


if __name__ == "__main__":
    main()
