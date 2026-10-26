"""Losslessly pack MCP artwork into an indexed review candidate, not runtime."""

from hashlib import sha256
import json
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[1]
READY = HERE / "game-ready"
SPEC = json.loads((HERE / "authoring.json").read_text())
NORMAL = [tuple(bytes.fromhex(c[1:])) for c in SPEC["palette"]]
SHINY = [tuple(bytes.fromhex(c[1:])) for c in SPEC["shinyPalette"]]
ICON = [tuple(map(int, line.split())) for line in
        (ROOT / "graphics/pokemon/icon_palettes/pal1.pal").read_text().splitlines()[3:19]]
ICON_MAP = [0, 11, 11, 5, 15, 4, 14, 14, 15, 5, 9, 4, 7, 15, 3, 15]


def load(name):
    with Image.open(HERE / "exports" / name) as source:
        rgba = source.convert("RGBA")
    lookup = {color: i for i, color in enumerate(NORMAL) if i}
    pixels = []
    for r, g, b, a in rgba.get_flattened_data():
        assert a in (0, 255), f"{name}: nonbinary alpha"
        assert not a or (r, g, b) in lookup, f"{name}: unexpected color {(r, g, b)}"
        pixels.append(lookup[r, g, b] if a else 0)
    result = Image.new("P", rgba.size, 0)
    result.putpalette([v for color in NORMAL for v in color])
    result.putdata(pixels)
    result.info["transparency"] = 0
    visible = lambda image: [pixel if pixel[3] else (0, 0, 0, 0)
                             for pixel in image.convert("RGBA").get_flattened_data()]
    assert visible(result) == visible(rgba)
    return result


def combine(frames, vertical=True):
    w, h = frames[0].size
    image = Image.new("P", (w if vertical else w * len(frames),
                            h * len(frames) if vertical else h), 0)
    image.putpalette(frames[0].getpalette())
    image.info["transparency"] = 0
    for i, frame in enumerate(frames):
        image.paste(frame, (0, i * h) if vertical else (i * w, 0))
    return image


def variant(image):
    result = image.copy()
    result.putpalette([v for rgb in SHINY for v in rgb])
    return result


def preview(front, back, icon, walkers):
    board = Image.new("RGB", (1000, 880), "#202B32")
    draw = ImageDraw.Draw(board)
    font = ImageFont.truetype("C:/Windows/Fonts/consola.ttf", 17)
    title = ImageFont.truetype("C:/Windows/Fonts/consolab.ttf", 23)
    draw.text((20, 14), "VIRELIAN POOCHYENA / V3 REVIEW", font=title, fill="#E4E8DC")

    def label(x, y, text):
        draw.text((x, y), text, font=font, fill="#E4E8DC")

    def tile(image, x, y, scale):
        image = image.convert("RGBA")
        enlarged = image.resize((image.width * scale, image.height * scale), Image.Resampling.NEAREST)
        draw.rectangle((x, y, x + enlarged.width - 1, y + enlarged.height - 1), fill="#E8E8E0")
        board.paste(enlarged, (x, y), enlarged)

    prior = HERE.parent / "virelia_poochyena_v2"
    old = Image.open(prior / "before/front.png").convert("RGBA")
    rejected = Image.open(prior / "exports/front-1.png").convert("RGBA")
    for x, text, image in ((20, "PRE-V2", old), (350, "REJECTED V2", rejected),
                           (680, "V3 / GBA ANATOMY", front[0])):
        label(x, 56, text)
        tile(image, x, 84, 4)
        tile(image, x + 176, 268, 1)
    label(20, 366, "V3 FRONT 2")
    label(250, 366, "V3 BACK")
    label(480, 366, "PARTY ICON")
    label(680, 366, "SHINY")
    tile(front[1], 20, 397, 3)
    tile(back, 250, 397, 3)
    tile(icon, 510, 397, 3)
    tile(variant(front[0]), 680, 397, 3)
    label(20, 620, "OVERWORLD / DOWN 1, DOWN 2, UP 1, UP 2, LEFT 1, LEFT 2")
    for i, frame in enumerate(walkers):
        tile(frame, 20 + 160 * i, 652, 4)
    label(20, 807, "16-COLOR NORMAL PALETTE")
    label(520, 807, "16-COLOR SHINY PALETTE")
    for x, colors in ((20, NORMAL), (520, SHINY)):
        for i, color in enumerate(colors[1:]):
            draw.rectangle((x + i * 29, 840, x + i * 29 + 25, 859), fill=color)
    board.save(HERE / "previews/review.png")


def main():
    READY.mkdir(exist_ok=True)
    front = [load(f"front-{i}.png") for i in (1, 2)]
    back = load("back-1.png")
    walkers = [load(f"overworld-{i}.png") for i in range(1, 7)]
    icons = [load(f"icon-{i}.png") for i in (1, 2)]
    icon = combine(icons)
    icon.save(HERE / "exports/icon_full_palette.png", bits=4, transparency=0)
    game_icon = icon.copy()
    game_icon.putpalette([v for rgb in ICON for v in rgb])
    game_icon.putdata([ICON_MAP[i] for i in icon.get_flattened_data()])
    files = {"front.png": front[0], "front_2.png": front[1],
             "anim_front.png": combine(front), "back.png": back,
             "icon.png": game_icon, "overworld.png": combine(walkers, False)}
    report = {"status": "candidate; not installed", "files": {},
              "checks": {"binary_alpha": True, "exact_colors": True,
                         "transparent_index": 0, "icon_shared_palette": 1}}
    for name, image in files.items():
        path = READY / name
        image.save(path, bits=4, transparency=0)
        raw = path.read_bytes()
        assert raw[24:26] == bytes((4, 3))
        with Image.open(path) as packed:
            assert packed.info.get("transparency") == 0
            assert packed.tobytes() == image.tobytes()
            assert len(packed.getpalette()) == 48 and max(packed.get_flattened_data()) < 16
        report["files"][name] = {"size": image.size, "bounds": image.convert("RGBA").getbbox(),
                                 "sha256": sha256(raw).hexdigest()}
    for stem, colors in (("normal", NORMAL), ("overworld_normal", NORMAL),
                         ("shiny", SHINY), ("overworld_shiny", SHINY)):
        assert all(v % 8 == 0 for rgb in colors for v in rgb)
        (READY / f"{stem}.pal").write_text("JASC-PAL\n0100\n16\n" +
                                          "".join("%d %d %d\n" % c for c in colors))
    assert front[0].tobytes() != front[1].tobytes()
    assert icons[0].tobytes() != icons[1].tobytes()
    for i in (0, 2, 4):
        assert walkers[i].tobytes() != walkers[i + 1].tobytes()
    report["checks"]["distinct_animation_pairs"] = True
    report["proposed_species_graphics"] = {
        "frontPicSize": [56, 48], "frontPicYOffset": 64 - max(f.convert("RGBA").getbbox()[3] for f in front),
        "backPicSize": [56, 48], "backPicYOffset": 64 - back.convert("RGBA").getbbox()[3]}
    (HERE / "verification.json").write_text(json.dumps(report, indent=2) + "\n")
    preview(front, back, game_icon, walkers)
    print("PASS: 11 frames, six 4-bit PNGs, four RGB555 palettes, transparency and distinct animation pairs")


if __name__ == "__main__":
    main()
