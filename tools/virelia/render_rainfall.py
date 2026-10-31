from pathlib import Path
from functools import lru_cache
import struct, json
from PIL import Image, ImageDraw

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / "outputs/rainfall"
OUT.mkdir(parents=True, exist_ok=True)


@lru_cache(None)
def words(p):
    d = p.read_bytes()
    return struct.unpack("<%dH" % (len(d) // 2), d)


@lru_cache(None)
def pix(p):
    return Image.open(p).copy()


@lru_cache(None)
def pal(p):
    return [tuple(map(int, l.split())) for l in p.read_text(encoding="utf-8").splitlines()[3:19]]


@lru_cache(None)
def tile(bank, i, primary="general"):
    a = ROOT / "data/tilesets/primary" / primary
    b = ROOT / "data/tilesets/secondary" / bank
    vals = words((a if i < 512 else b) / "metatiles.bin")[(i % 512) * 8 : (i % 512 + 1) * 8]
    im = Image.new("RGB", (16, 16), (40, 48, 56))
    for n, v in enumerate(vals):
        t = v & 1023
        s = v >> 12
        img = pix((a if t < 512 else b) / "tiles.png")
        colors = pal((a if s < 6 else b) / f"palettes/{s:02}.pal")
        t %= 512
        for y in range(8):
            for x in range(8):
                sx = t % 16 * 8 + (7 - x if v & 1024 else x)
                sy = t // 16 * 8 + (7 - y if v & 2048 else y)
                if sx >= img.width or sy >= img.height:
                    continue
                p = img.getpixel((sx, sy))
                if p:
                    im.putpixel((n % 2 * 8 + x, n % 4 // 2 * 8 + y), colors[p])
    return im


def atlas(bank, start=512, end=None):
    if end is None:
        end = 512 + len(words(ROOT / f"data/tilesets/secondary/{bank}/metatile_attributes.bin"))
    im = Image.new("RGB", (16 * 48, ((end - start + 15) // 16) * 48), (40, 48, 56))
    d = ImageDraw.Draw(im)
    for n, i in enumerate(range(start, end)):
        x = n % 16 * 48
        y = n // 16 * 48
        im.paste(tile(bank, i).resize((32, 32)), (x, y))
        d.text((x, y + 32), f"{i:03X}", fill="white")
    im.save(OUT / f"atlas-{bank}-{start:03x}.png")


def render(name):
    ls = json.loads((ROOT / "data/layouts/layouts.json").read_text(encoding="utf-8"))["layouts"]
    l = next(l for l in ls if l["name"] == name + "_Layout")
    bank = l["secondary_tileset"].removeprefix("gTileset_")
    bank = {
        "VireliaRainfall": "virelia_rainfall",
        "VireliaCoast": "virelia_coast",
        "VireliaLighthouse": "virelia_lighthouse",
        "VireliaOutdoor": "virelia_outdoor",
        "VireliaServiceInterior": "virelia_service_interior",
        "GenericBuilding": "generic_building",
    }.get(bank, bank.lower())
    pri = l["primary_tileset"].removeprefix("gTileset_").lower()
    im = Image.new("RGB", (l["width"] * 16, l["height"] * 16))
    for n, c in enumerate(words(ROOT / l["blockdata_filepath"])):
        im.paste(tile(bank, c & 1023, pri), (n % l["width"] * 16, n // l["width"] * 16))
    im.save(OUT / (name + ".png"))


if __name__ == "__main__":
    import sys

    if len(sys.argv) > 1:
        for name in sys.argv[1:]:
            render(name)
    else:
        for b in ("fortree", "slateport", "lilycove", "pacifidlog"):
            atlas(b)
        atlas("fortree", 0, 512)
