#!/usr/bin/env python3
"""Author Chapter 2 maps. Never regenerate existing Porymap terrain.

Only Slateford's documented 3x4 north passage footprint is patched. Re-running
replaces Chapter 2 maps, so back up subsequent Porymap edits before doing so.
"""

from pathlib import Path
import json
import re
import shutil
import struct
from PIL import Image, ImageDraw

ROOT = Path(__file__).resolve().parents[2]
LAYOUTS = json.loads((ROOT / "data/layouts/layouts.json").read_text(encoding="utf-8-sig"))
GROUPS = json.loads((ROOT / "data/maps/map_groups.json").read_text(encoding="utf-8-sig"))


def jw(p, v):
    p.parent.mkdir(parents=True, exist_ok=True)
    p.write_text(json.dumps(v, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")


def words(p):
    b = p.read_bytes()
    return list(struct.unpack("<%dH" % (len(b) // 2), b))


def ww(p, v):
    p.parent.mkdir(parents=True, exist_ok=True)
    p.write_bytes(struct.pack("<%dH" % len(v), *v))


def grid(name):
    l = next(l for l in LAYOUTS["layouts"] if l["name"] == name + "_Layout")
    v = words(ROOT / l["blockdata_filepath"])
    return [v[y * l["width"] : (y + 1) * l["width"]] for y in range(l["height"])]


def rect(g, x, y, w, h, c):
    for yy in range(y, y + h):
        for xx in range(x, x + w):
            g[yy][xx] = c


def clone_rainfall():
    src = ROOT / "data/tilesets/secondary/fortree"
    dst = ROOT / "data/tilesets/secondary/virelia_rainfall"
    dst.mkdir(parents=True, exist_ok=True)
    shutil.copyfile(src / "tiles.png", dst / "tiles.png")
    shutil.copytree(src / "palettes", dst / "palettes", dirs_exist_ok=True)
    m = words(src / "metatiles.bin")
    a = words(src / "metatile_attributes.bin")
    m += [0] * ((0x324 - 512) * 8 - len(m))
    a += [0] * (0x324 - 512 - len(a))
    # Ordinary walking bridges at authored elevations; no stock bridge toggles.
    for dest, source, attr in [
        (0x320, 0x220, 0x1000),
        (0x321, 0x228, 0x1000),
        (0x322, 0x121, 0x64),
        (0x323, 0x121, 0x65),
    ]:
        sm = words(
            (ROOT / "data/tilesets/primary/general" if source < 512 else src) / "metatiles.bin"
        )
        m[(dest - 512) * 8 : (dest - 511) * 8] = sm[(source % 512) * 8 : (source % 512 + 1) * 8]
        a[dest - 512] = attr
    ww(dst / "metatiles.bin", m)
    ww(dst / "metatile_attributes.bin", a)


class CoastBank:
    def __init__(self):
        self.ids = {}
        self.meta = []
        self.attrs = []
        self.tiles = []
        self.lookup = {}
        self.dst = ROOT / "data/tilesets/secondary/virelia_coast"
        self.dst.mkdir(parents=True, exist_ok=True)
        self.src = ROOT / "data/tilesets/secondary/slateport"
        shutil.copytree(self.src / "palettes", self.dst / "palettes", dirs_exist_ok=True)

    def stock(self, i):
        if i < 512:
            return i
        if i in self.ids:
            return self.ids[i]
        index = 512 + len(self.attrs)
        self.ids[i] = index
        vals = words(self.src / "metatiles.bin")[(i - 512) * 8 : (i - 511) * 8]
        image = Image.open(self.src / "tiles.png")
        for v in vals:
            t = v & 1023
            if t >= 512:
                t -= 512
                data = image.crop(
                    (t % 16 * 8, t // 16 * 8, t % 16 * 8 + 8, t // 16 * 8 + 8)
                ).tobytes()
                v = (v & ~1023) | self.add(data)
            self.meta.append(v)
        self.attrs.append(words(self.src / "metatile_attributes.bin")[i - 512])
        return index

    def add(self, data):
        if not any(data):
            return 0
        if data not in self.lookup:
            self.lookup[data] = 512 + len(self.tiles)
            self.tiles.append(data)
        return self.lookup[data]

    def custom(self, im, attr=0, grass_base=False):
        idx = 512 + len(self.attrs)
        for y in (0, 8):
            for x in (0, 8):
                self.meta.append((11 << 12) | self.add(im.crop((x, y, x + 8, y + 8)).tobytes()))
        if grass_base:
            # Transparent tower quarters need real grass beneath them on GBA.
            overlay = self.meta[-4:]
            ground = words(ROOT / "data/tilesets/primary/general/metatiles.bin")[8:12]
            self.meta[-4:] = ground + overlay
        else:
            self.meta += [0] * 4
        self.attrs.append(attr)
        return idx

    def finish(self):
        assert len(self.tiles) <= 496, (len(self.tiles), "graphics tile limit")
        assert all(value >> 12 <= 12 for value in self.meta), "Coast graphics use unloaded palette"
        im = Image.new("P", (128, 256))
        im.putpalette(Image.open(self.src / "tiles.png").getpalette())
        for i, data in enumerate(self.tiles):
            im.paste(Image.frombytes("P", (8, 8), data), (i % 16 * 8, i // 16 * 8))
        im.save(self.dst / "tiles.png")
        ww(self.dst / "metatiles.bin", self.meta)
        ww(self.dst / "metatile_attributes.bin", self.attrs)
        print("Coast bank:", len(self.tiles), "tiles;", len(self.attrs), "metatiles")


class Map:
    def __init__(self, suffix, w, h, bank="VireliaRainfall", indoor=False, elev=3):
        self.suffix = suffix
        self.name = "Virelia_" + suffix
        self.const = "MAP_VIRELIA_" + re.sub(r"(?<=[a-z])(?=[A-Z])", "_", suffix).upper()
        self.layout = "LAYOUT_" + self.const[4:]
        self.w = w
        self.h = h
        self.bank = bank
        self.indoor = indoor
        self.elev = elev
        self.g = [[0x040E] * w for _ in range(h)]
        self.obj = []
        self.warps = []
        self.signs = []
        self.coords = []
        self.paths = set()
        self.weather = "WEATHER_NONE" if indoor else "WEATHER_RAIN"
        self.music = "MUS_ROUTE119"
        self.section = (
            "MAPSEC_VIRELIA_SEABREAK_TOWN"
            if "Seabreak" in suffix
            else "MAPSEC_VIRELIA_RAINFALL_PASS"
        )

    def block(self, i, solid=False, elev=None):
        return i | (0x400 if solid else 0) | ((self.elev if elev is None else elev) << 12)

    def area(self, x, y, w, h, i=1, solid=False):
        rect(self.g, x, y, w, h, self.block(i, solid))

    def road(self, x, y, w, h):
        self.paths.update((xx, yy) for yy in range(y, y + h) for xx in range(x, x + w))

    def paint(self):
        for x, y in self.paths:
            for yy in range(max(1, y - 1), min(self.h - 1, y + 2)):
                for xx in range(max(1, x - 1), min(self.w - 1, x + 2)):
                    self.g[yy][xx] = self.block(1)
        for x, y in self.paths:
            top = (x, y - 1) not in self.paths
            bottom = (x, y + 1) not in self.paths
            left = (x - 1, y) not in self.paths
            right = (x + 1, y) not in self.paths
            tile = (
                (0x118 if left else 0x11A if right else 0x119)
                if top
                else (
                    (0x128 if left else 0x12A if right else 0x129)
                    if bottom
                    else 0x120 if left else 0x122 if right else 0x121
                )
            )
            self.g[y][x] = self.block(tile)

    def trees(self):
        # Replace shrub filler only with complete native tree silhouettes.
        for y in range(1, self.h - 3, 3):
            for x in range(1, self.w - 2, 2):
                if all(self.g[y + dy][x + dx] == 0x040E for dy in range(3) for dx in range(2)):
                    for dy, row in enumerate(((0x1CE, 0x1CF), (0x1D6, 0x1D7), (0x1E6, 0x1E7))):
                        for dx, v in enumerate(row):
                            self.g[y + dy][x + dx] = 0x400 | v

    def actor(self, gfx, x, y, label, local=None, trainer=False, flag="0", facing="DOWN"):
        o = dict(
            graphics_id="OBJ_EVENT_GFX_" + gfx,
            x=x,
            y=y,
            elevation=self.elev,
            movement_type="MOVEMENT_TYPE_FACE_" + facing,
            movement_range_x=0,
            movement_range_y=0,
            trainer_type="TRAINER_TYPE_NORMAL" if trainer else "TRAINER_TYPE_NONE",
            trainer_sight_or_berry_tree_id="2" if trainer else "0",
            script=self.name + "_EventScript_" + label,
            flag=flag,
        )
        if isinstance(local, str):
            o = dict(local_id=local, **o)
        self.obj.append(o)

    def sign(self, x, y, label):
        self.signs.append(
            dict(
                type="sign",
                x=x,
                y=y,
                elevation=0,
                player_facing_dir="BG_EVENT_PLAYER_FACING_ANY",
                script=self.name + "_EventScript_" + label,
            )
        )

    def warp(self, x, y, dest, index, arrow="north"):
        self.warps.append(dict(x=x, y=y, elevation=0, dest_map=dest, dest_warp_id=str(index)))
        self.g[y][x] = self.block(0x322 if arrow == "north" else 0x323)

    def finish(self):
        folder = "data/layouts/" + self.name
        l = dict(
            id=self.layout,
            name=self.name + "_Layout",
            width=self.w,
            height=self.h,
            primary_tileset="gTileset_Building" if self.indoor else "gTileset_General",
            secondary_tileset="gTileset_" + self.bank,
            border_filepath=folder + "/border.bin",
            blockdata_filepath=folder + "/map.bin",
            layout_version="emerald",
        )
        old = next((x for x in LAYOUTS["layouts"] if x["id"] == self.layout), None)
        if old:
            old.update(l)
        else:
            LAYOUTS["layouts"].append(l)
        ww(ROOT / l["blockdata_filepath"], [v for row in self.g for v in row])
        ww(ROOT / l["border_filepath"], [0x0648 if self.indoor else 0x040E] * 4)
        doc = dict(
            id=self.const,
            name=self.name,
            layout=self.layout,
            music=self.music,
            region="REGION_HOENN",
            region_map_section=self.section,
            requires_flash=False,
            weather=self.weather,
            map_type=(
                "MAP_TYPE_INDOOR"
                if self.indoor
                else "MAP_TYPE_TOWN" if self.suffix == "SeabreakTown" else "MAP_TYPE_ROUTE"
            ),
            allow_cycling=False,
            allow_escaping=False,
            allow_running=True,
            show_map_name=not self.indoor,
            battle_scene="MAP_BATTLE_SCENE_NORMAL",
            connections=None,
            object_events=self.obj,
            warp_events=self.warps,
            coord_events=self.coords,
            bg_events=self.signs,
        )
        jw(ROOT / "data/maps" / self.name / "map.json", doc)
        group = GROUPS[
            "gMapGroup_VireliaInteriors" if self.indoor else "gMapGroup_VireliaOverworld"
        ]
        if self.name not in group:
            group.append(self.name)


def build():
    clone_rainfall()
    f = Map("RainfallFoothills", 32, 28)
    f.weather = "WEATHER_SUNNY"
    for r in [
        (14, 22, 2, 5),
        (7, 21, 9, 2),
        (7, 10, 2, 12),
        (7, 10, 18, 2),
        (7, 3, 2, 8),
        (24, 10, 2, 10),
        (24, 18, 5, 2),
    ]:
        f.road(*r)
    f.paint()
    f.area(10, 14, 9, 5, 0x079, True)
    f.area(11, 14, 7, 1, 0x071, True)
    f.area(11, 19, 7, 1, 0x090, True)
    f.area(3, 13, 3, 5, 0x00D)
    f.area(18, 4, 7, 4, 0x00D)
    f.trees()
    f.warp(14, 26, "MAP_VIRELIA_SLATEFORD_CITY", 3, "south")
    f.warp(7, 3, "MAP_VIRELIA_RAINFALL_UPPER", 0)
    f.warp(28, 18, "MAP_VIRELIA_RAINFALL_LOWER", 0)
    f.actor("HIKER", 8, 16, "Hiker", trainer=True, facing="DOWN")
    f.sign(10, 10, "Sign")
    f.g[10][10] = f.block(3, True)
    f.area(24, 22, 5, 3)
    f.actor("HIKER", 26, 23, "Shortcut")
    f.road(24, 19, 2, 4)
    # Preserve previously painted rocks/trees while adding only the shortcut spur.
    f.area(24, 19, 2, 4, 0x121)

    u = Map("RainfallUpper", 32, 34, elev=5)
    for r in [
        (5, 27, 2, 5),
        (5, 26, 17, 2),
        (20, 19, 2, 9),
        (8, 18, 14, 2),
        (8, 9, 2, 11),
        (8, 8, 17, 2),
        (23, 3, 2, 7),
    ]:
        u.road(*r)
    u.paint()
    u.area(2, 11, 28, 9)
    u.area(2, 13, 28, 4, 0x0D1, True)
    for x in range(2, 30):
        u.g[13][x] = u.block(0x0C9, True)
        u.g[16][x] = u.block(0x0D9, True)
    u.area(8, 10, 2, 3, 0x121)
    u.area(8, 17, 2, 2, 0x121)
    # Cliffs carry a native narrow wooden bridge above the stream.
    for y in (13, 14, 15, 16):
        u.g[y][8] = u.block(0x320)
        u.g[y][9] = u.block(0x321)
    # A native waterfall feeds the western stream, clear of the walking bridge.
    for dy, row in enumerate(((0x0BB, 0x0B3, 0x0BC), (0x0B9, 0x0B9, 0x0B9), (0x02C, 0x034, 0x03C))):
        for dx, tile in enumerate(row):
            u.g[11 + dy][3 + dx] = u.block(tile, True)
    for yy in range(10, 13):
        for xx in (2, 6):
            u.g[yy][xx] = u.block(0x079, True)
    u.area(3, 10, 3, 1, 0x071, True)
    u.area(3, 20, 4, 3, 0x00D)
    u.area(24, 11, 4, 5, 0x015)
    u.area(12, 21, 7, 3, 0x079, True)
    u.area(12, 20, 7, 1, 0x071, True)
    u.trees()
    u.warp(5, 31, "MAP_VIRELIA_RAINFALL_FOOTHILLS", 1, "south")
    u.warp(23, 3, "MAP_VIRELIA_RAINFALL_LOWLANDS", 0)
    u.actor("ITEM_BALL", 27, 12, "Item", flag="FLAG_VIRELIA_RAINFALL_ITEM_REPEL")
    u.actor("WOMAN_2", 20, 23, "Ranger", trainer=True, facing="DOWN")
    u.sign(10, 9, "WeatherSign")
    u.g[9][10] = u.block(3, True)

    lo = Map("RainfallLower", 40, 40)
    lo.weather = "WEATHER_DOWNPOUR"
    for r in [
        (4, 32, 2, 5),
        (4, 31, 14, 2),
        (16, 25, 2, 8),
        (8, 24, 10, 2),
        (8, 15, 2, 11),
        (8, 14, 23, 2),
        (29, 14, 2, 17),
        (29, 29, 7, 2),
        (34, 6, 2, 25),
        (25, 5, 11, 2),
        (25, 2, 2, 5),
        (18, 20, 10, 2),
    ]:
        lo.road(*r)
    lo.road(4, 2, 23, 2)
    lo.paint()
    lo.area(12, 3, 10, 9, 0x0D1, True)
    lo.area(12, 3, 10, 1, 0x0C9, True)
    lo.area(12, 11, 10, 1, 0x0D9, True)
    lo.area(12, 12, 10, 1, 0x00D)
    lo.area(12, 11, 10, 1, 0x00D)
    lo.area(19, 17, 8, 17)
    lo.area(21, 18, 4, 15, 0x0D1, True)
    for y in range(18, 33):
        lo.g[y][21] = lo.block(0x0D0, True)
        lo.g[y][24] = lo.block(0x0D2, True)
    for x in range(21, 25):
        lo.g[20][x] = lo.block(0x320)
        lo.g[21][x] = lo.block(0x321)
    for r in [(3, 20, 4, 9), (11, 16, 5, 6), (25, 8, 6, 5), (31, 31, 5, 5), (19, 34, 6, 3)]:
        lo.area(*r, 0x015)
    lo.area(14, 19, 7, 4)
    lo.trees()
    lo.warp(4, 36, "MAP_VIRELIA_RAINFALL_FOOTHILLS", 2, "south")
    lo.warp(4, 2, "MAP_VIRELIA_RAINFALL_LOWLANDS", 1)
    lo.actor("FISHERMAN", 9, 19, "Fisherman", trainer=True, facing="DOWN")
    lo.actor("HIKER", 18, 20, "Shortcut")
    lo.actor("ITEM_BALL", 6, 23, "Item", flag="FLAG_VIRELIA_RAINFALL_ITEM_SUPER_POTION")
    lo.sign(16, 22, "StreamClue")
    lo.g[22][16] = lo.block(0x0CE, True)
    # Optional exploration lane leads to the lower bridge winch and item.
    lo.area(6, 22, 3, 3, 0x015)
    lo.area(10, 19, 9, 2)
    lo.area(16, 20, 2, 3)

    ll = Map("RainfallLowlands", 32, 26)
    ll.weather = "WEATHER_SHADE"
    for r in [
        (4, 3, 2, 7),
        (4, 8, 12, 2),
        (25, 3, 2, 7),
        (15, 8, 12, 2),
        (15, 8, 2, 4),
        (15, 12, 1, 4),
        (15, 15, 2, 9),
        (15, 20, 12, 2),
    ]:
        ll.road(*r)
    ll.paint()
    ll.area(20, 13, 9, 5, 0x00D)
    ll.area(25, 17, 2, 4, 0x00D)
    ll.area(3, 15, 8, 7, 0x0D1, True)
    ll.trees()
    # A one-tile ledge neck makes the rival checkpoint unavoidable, with room to retreat.
    for y in (11, 12, 13, 14):
        for x in range(12, 20):
            if x != 15:
                ll.g[y][x] = 0x040E
    ll.g[13][15] = 0x040E
    ll.g[12][16] = ll.block(1)
    ll.warp(4, 3, "MAP_VIRELIA_RAINFALL_UPPER", 1)
    ll.warp(25, 3, "MAP_VIRELIA_RAINFALL_LOWER", 1)
    ll.warp(15, 23, "MAP_VIRELIA_SEABREAK_TOWN", 0, "south")
    ll.actor(
        "VAR_0",
        15,
        12,
        "Rival",
        local="LOCALID_VIRELIA_RAINFALL_RIVAL",
        flag="FLAG_VIRELIA_RAINFALL_RIVAL_COMPLETE",
    )
    ll.actor("BOY_1", 23, 20, "Weather", trainer=True, facing="LEFT")
    ll.sign(14, 13, "CoastGate")

    coast = CoastBank()
    # Reserve off/on indicators at fixed IDs 20C/20D for the lantern-room callback.
    colors = [
        (0, 0, 0),
        (40, 48, 64),
        (64, 80, 96),
        (96, 112, 128),
        (136, 152, 160),
        (176, 192, 192),
        (216, 224, 216),
        (240, 240, 224),
        (112, 48, 48),
        (176, 64, 56),
        (224, 104, 72),
        (240, 184, 80),
        (248, 232, 144),
        (40, 88, 112),
        (56, 128, 152),
        (96, 176, 184),
    ]
    (coast.dst / "palettes/11.pal").write_text(
        "JASC-PAL\n0100\n16\n" + "\n".join(" ".join(map(str, c)) for c in colors) + "\n"
    )
    # Slot 13 is UNLOADED padding; real metatiles must use palettes 0..12.
    # Retain deterministic padding bytes without assigning graphics to slot 13.
    shutil.copyfile(coast.dst / "palettes/11.pal", coast.dst / "palettes/13.pal")
    for i in range(12):
        coast.custom(Image.new("P", (16, 16), 1))
    for lit in (False, True):
        im = Image.new("P", (16, 16), 2)
        d = ImageDraw.Draw(im)
        d.rectangle((1, 1, 14, 14), fill=4, outline=1)
        d.rectangle((4, 4, 11, 11), fill=12 if lit else 3)
        coast.custom(im)
    # Lighthouse: coherent 4x8 native pixel silhouette, slate dome and red stripe.
    tower = Image.new("P", (64, 128), 0)
    d = ImageDraw.Draw(tower)
    d.polygon([(12, 24), (18, 16), (46, 16), (52, 24)], fill=2)
    d.rectangle((14, 24, 49, 112), fill=6, outline=2)
    d.rectangle((14, 42, 49, 52), fill=9)
    d.rectangle((14, 84, 49, 92), fill=9)
    d.rectangle((45, 25, 49, 111), fill=4)
    d.rectangle((15, 26, 19, 110), fill=7)
    d.polygon([(7, 16), (16, 5), (47, 5), (56, 16)], fill=2)
    d.line((13, 16, 50, 16), fill=1, width=2)
    d.rectangle((17, 17, 46, 29), fill=14, outline=1)
    for x in (19, 31, 43):
        d.line((x, 17, x, 29), fill=3, width=2)
    d.rectangle((8, 30, 55, 33), fill=2)
    d.rectangle((10, 34, 53, 37), fill=5)
    for y in (60, 99):
        d.rectangle((28, y, 35, y + 9), fill=13, outline=2)
    d.rectangle((26, 111, 37, 127), fill=2)
    d.rectangle((29, 114, 34, 127), fill=3)
    d.line((12, 112, 51, 112), fill=4, width=2)
    TOWER = []
    for y in range(8):
        row = []
        for x in range(4):
            row.append(
                coast.custom(
                    tower.crop((x * 16, y * 16, x * 16 + 16, y * 16 + 16)),
                    0x60 if (x, y) == (1, 7) else 0,
                    grass_base=True,
                )
            )
        TOWER.append(row)
    t = Map("SeabreakTown", 48, 42, bank="VireliaCoast")
    t.weather = "WEATHER_RAIN"
    t.music = "MUS_SLATEPORT"
    # Coastal land is bounded by shaped sandy banks. Harbor and headland are linked by boardwalk.
    t.g = [[0x3170] * 48 for _ in range(42)]
    t.area(2, 2, 30, 25, 1)
    t.area(3, 26, 26, 6, 0x0125)
    t.area(29, 23, 15, 15, 0x0125)
    t.area(30, 19, 12, 8, 1)
    for y in range(2, 26):
        t.g[y][32] = t.block(0x03A, True)
    for x in range(3, 29):
        t.g[32][x] = t.block(0x16E, True)
    for r in [
        (14, 1, 2, 27),
        (7, 14, 19, 2),
        (7, 20, 25, 2),
        (21, 8, 2, 15),
        (30, 20, 10, 2),
        (36, 20, 2, 15),
        (7, 28, 18, 2),
    ]:
        t.road(*r)
    t.paint()
    # Copy stock small house facade from Slateport map, remapping only selected art.
    sp = grid("SlateportCity")

    def copy_coast(source, sx, sy, w, h, x, y):
        for dy in range(h):
            for dx in range(w):
                c = source[sy + dy][sx + dx]
                t.g[y + dy][x + dx] = (c & ~1023) | coast.stock(c & 1023)

    # Generic coastal houses built from the original roof/wall metatiles.
    def house(x, y, door=False):
        rows = [
            [0x208, 0x209, 0x209, 0x20A],
            [0x210, 0x211, 0x211, 0x212],
            [0x218, 0x219, 0x219, 0x21A],
            [0x220, 0x221, 0x222, 0x222],
        ]
        # Primary General's stock red-roof home art includes its original door.
        rows = [
            [0x008, 0x009, 0x009, 0x00A],
            [0x010, 0x011, 0x011, 0x012],
            [0x018, 0x019, 0x019, 0x01A],
            [0x020, 0x021, 0x022, 0x020],
        ]
        for dy, row in enumerate(rows):
            for dx, c in enumerate(row):
                t.g[y + dy][x + dx] = t.block(c, True)
        t.g[y + 3][x + 1] = t.block(0x0A7 if door else 0x023, not door)

    house(4, 4)
    house(23, 4)
    house(20, 17, True)
    # Center/Mart facade uses unmodified primary General art and nonanimated entry.
    for y, row in enumerate(
        [
            [0x030, 0x031, 0x031, 0x031, 0x032],
            [0x038, 0x039, 0x039, 0x039, 0x03A],
            [0x040, 0x041, 0x042, 0x042, 0x043],
            [0x060, 0x061, 0x062, 0x062, 0x063],
        ]
    ):
        for x, c in enumerate(row):
            t.g[10 + y][6 + x] = t.block(c, True)
    t.g[13][8] = t.block(0x0A7)
    # Three long piers, visible cargo, tied fishing boats from stock boat quarters.
    woodcolors = [
        (0, 0, 0),
        (48, 48, 56),
        (80, 64, 56),
        (112, 88, 64),
        (144, 112, 72),
        (176, 144, 96),
        (208, 176, 120),
        (232, 208, 152),
    ] + [(0, 0, 0)] * 8
    (coast.dst / "palettes/12.pal").write_text(
        "JASC-PAL\n0100\n16\n" + "\n".join(" ".join(map(str, c)) for c in woodcolors) + "\n"
    )
    board = Image.new("P", (16, 16), 4)
    bd = ImageDraw.Draw(board)
    for by in (0, 4, 8, 12):
        bd.line((0, by, 15, by), fill=2)
        bd.line((0, by + 1, 15, by + 1), fill=6)
        bd.point((2, by + 2), fill=1)
        bd.point((13, by + 2), fill=1)
    plank = coast.custom(board)
    coast.meta[-8:-4] = [(v & 0x0FFF) | (12 << 12) for v in coast.meta[-8:-4]]
    crate = coast.stock(0x334)
    for x, y, w, h in [(6, 30, 3, 9), (16, 30, 3, 7), (26, 27, 3, 10), (28, 25, 9, 2)]:
        t.area(x, y, w, h, plank)
    for x, y in [(5, 29), (9, 30), (17, 28), (25, 28), (28, 31)]:
        t.g[y][x] = t.block(crate, True)
    for bx, by in [(10, 34), (20, 32)]:
        for dy, row in enumerate([[0x260, 0x265, 0x266, 0x267], [0x278, 0x34B, 0x348, 0x351]]):
            for dx, c in enumerate(row):
                t.g[by + dy][bx + dx] = t.block(coast.stock(c), True)
    # Lighthouse headland tower, approach from south and west.
    for dy, row in enumerate(TOWER):
        for dx, c in enumerate(row):
            t.g[24 + dy][36 + dx] = t.block(c, True)
    t.g[31][37] = t.block(TOWER[7][1])
    t.area(35, 32, 7, 3, 0x121)
    # Boardwalk runs around the beacon with a separate historical viewpoint.
    t.area(31, 25, 3, 10, plank)
    t.area(31, 33, 6, 2, plank)
    t.area(40, 34, 3, 3, plank)
    t.warps = [
        dict(x=14, y=1, elevation=0, dest_map="MAP_VIRELIA_RAINFALL_LOWLANDS", dest_warp_id="2"),
        dict(
            x=8, y=13, elevation=0, dest_map="MAP_VIRELIA_SEABREAK_POKEMON_CENTER", dest_warp_id="0"
        ),
        dict(x=37, y=31, elevation=0, dest_map="MAP_VIRELIA_SEABREAK_LIGHTHOUSE", dest_warp_id="0"),
        dict(
            x=21, y=20, elevation=0, dest_map="MAP_VIRELIA_SEABREAK_KEEPERS_HOUSE", dest_warp_id="0"
        ),
    ]
    # Town boundary arrows are cloned with ordinary native warp behaviors.
    arrow = Image.new("P", (16, 16), 6)
    t.g[1][14] = t.block(coast.custom(arrow, 0x64))
    t.actor("SAILOR", 14, 28, "HarborMaster", 1)
    t.actor("FISHERMAN", 7, 35, "Fisherman", 2)
    t.actor("VAR_0", 20, 15, "Rival", 3)
    t.actor("WOMAN_2", 17, 29, "Bela", 4, flag="FLAG_VIRELIA_HIDE_HARBOR_BELA")
    t.actor("POLICEMAN", 15, 29, "Vacek", 5, flag="FLAG_VIRELIA_HIDE_VACEK")
    t.actor("REPORTER_F", 12, 29, "Reporter", 6)
    t.actor("SAILOR", 27, 23, "Sailor", 7)
    t.sign(10, 15, "TownSign")
    t.g[15][10] = t.block(3, True)
    t.sign(23, 7, "GymClosed")
    t.sign(6, 31, "AnchorClue")
    t.sign(41, 35, "CoastClue")
    t.g[35][41] = t.block(0x0CE, True)
    # Closed gym plot is a storm inspection board, with clear pedestrian access.
    t.area(21, 9, 6, 3, 0x00E, True)
    t.area(21, 12, 6, 2)
    t.sign(23, 11, "GymClosed")

    center = Map("Seabreak_PokemonCenter", 24, 9, "VireliaServiceInterior", True)
    center.g = grid("Virelia_Slateford_PokemonCenter")
    center.music = "MUS_POKE_CENTER"
    old = json.loads(
        (ROOT / "data/maps/Virelia_Slateford_PokemonCenter/map.json").read_text(encoding="utf-8")
    )
    center.obj = old["object_events"]
    for o in center.obj:
        o["script"] = o["script"].replace("Virelia_Slateford", "Virelia_Seabreak")
        if "local_id" in o:
            o["local_id"] = "LOCALID_VIRELIA_SEABREAK_NURSE"
    center.warps = [dict(x=x, y=8, elevation=0, dest_map=t.const, dest_warp_id="1") for x in (7, 6)]

    # Interiors use ordinary Building/GenericBuilding furniture. No Hoenn scripts.
    keeper = Map("Seabreak_KeepersHouse", 12, 10, "GenericBuilding", True)
    keeper.music = "MUS_DEWFORD"
    room = grid("Virelia_MaplebrookCottage")
    keeper.g = [[0x0648] * 12 for _ in range(10)]
    for y in range(9):
        for x in range(10):
            keeper.g[y][x + 1] = room[y][x]
    keeper.actor("EXPERT_M", 6, 4, "Keeper", 1)
    keeper.sign(3, 3, "Journal")
    keeper.warps = [dict(x=x, y=8, elevation=0, dest_map=t.const, dest_warp_id="3") for x in (5, 6)]
    keeper.g[8][5] = 0x3208
    keeper.g[8][6] = 0x3209

    # Build a compact dedicated interior bank from GenericBuilding; primary floor and furniture remain stock.
    li = ROOT / "data/tilesets/secondary/virelia_lighthouse"
    li.mkdir(parents=True, exist_ok=True)
    shutil.copytree(
        ROOT / "data/tilesets/secondary/generic_building/palettes",
        li / "palettes",
        dirs_exist_ok=True,
    )
    shutil.copyfile(ROOT / "data/tilesets/secondary/generic_building/tiles.png", li / "tiles.png")
    lm = words(ROOT / "data/tilesets/secondary/generic_building/metatiles.bin")
    la = words(ROOT / "data/tilesets/secondary/generic_building/metatile_attributes.bin")
    # 20C/20D are replaced by off/on light panels composed from Building's native console quarters.
    for target, source in [(0x20C, 0x2A2), (0x20D, 0x2A3)]:
        lm[(target - 512) * 8 : (target - 511) * 8] = words(
            ROOT / "data/tilesets/secondary/generic_building/metatiles.bin"
        )[(source - 512) * 8 : (source - 511) * 8]
        la[target - 512] = 0
    lm += [0] * ((0x302 - 512) * 8 - len(lm))
    la += [0] * (0x302 - 512 - len(la))
    for target, attr in [(0x300, 0x64), (0x301, 0x65)]:
        lm[(target - 512) * 8 : (target - 511) * 8] = lm[0x23 * 8 : 0x24 * 8]
        la[target - 512] = attr
    # Dedicated console pixels use otherwise unused graphics in this NEW bank only.
    panelcolors = [
        (0, 0, 0),
        (32, 40, 56),
        (56, 72, 88),
        (88, 104, 120),
        (128, 144, 152),
        (176, 192, 192),
        (224, 232, 216),
        (248, 240, 176),
        (120, 48, 40),
        (184, 64, 48),
        (232, 120, 64),
        (240, 192, 72),
        (248, 232, 104),
        (32, 88, 104),
        (40, 136, 144),
        (80, 184, 160),
    ]
    (li / "palettes/12.pal").write_text(
        "JASC-PAL\n0100\n16\n" + "\n".join(" ".join(map(str, c)) for c in panelcolors) + "\n"
    )
    # Slot 13 is UNLOADED padding; the console itself uses loaded slot 12.
    shutil.copyfile(li / "palettes/12.pal", li / "palettes/13.pal")
    paneltiles = Image.open(li / "tiles.png").copy()
    for lit in (False, True):
        panel = Image.new("P", (16, 16), 1)
        pd = ImageDraw.Draw(panel)
        pd.rectangle((1, 1, 14, 14), fill=3, outline=5)
        pd.line((2, 2, 13, 2), fill=6)
        pd.rectangle((3, 4, 12, 10), fill=1)
        pd.rectangle((4, 5, 7, 8), fill=15 if lit else 13)
        pd.rectangle((9, 5, 11, 8), fill=12 if lit else 8)
        pd.line((3, 12, 11, 12), fill=2)
        pd.point((2, 13), fill=1)
        pd.point((13, 13), fill=1)
        for n, (x, y) in enumerate(((0, 0), (8, 0), (0, 8), (8, 8))):
            idx = 480 + int(lit) * 4 + n
            paneltiles.paste(panel.crop((x, y, x + 8, y + 8)), (idx % 16 * 8, idx // 16 * 8))
            lm[(0x20C + int(lit) - 512) * 8 + n] = (12 << 12) | 512 | idx
        lm[(0x20C + int(lit) - 512) * 8 + 4 : (0x20C + int(lit) - 511) * 8] = [0] * 4
    paneltiles.save(li / "tiles.png")
    assert all(value >> 12 <= 12 for value in lm), "Lighthouse graphics use unloaded palette"
    ww(li / "metatiles.bin", lm)
    ww(li / "metatile_attributes.bin", la)

    def interior(suffix, w, h):
        m = Map(suffix, w, h, "VireliaLighthouse", True)
        m.music = "MUS_MT_PYRE"
        m.g = [[0x0648] * w for _ in range(h)]
        # Copy native wooden floor 202, walls and shelving from the keeper cottage.
        m.area(1, 2, w - 2, h - 3, 0x223)
        for x in range(1, w - 1):
            m.g[1][x] = 0x0605
        return m

    light = interior("Seabreak_Lighthouse", 16, 14)
    light.actor("WOMAN_2", 6, 6, "Bela", 1, flag="FLAG_VIRELIA_RAINFALL_COMPLETE")
    light.sign(5, 3, "Damage")
    light.sign(9, 3, "Instructions")
    light.g[3][5] = 0x060D
    light.g[3][9] = 0x060D
    light.warps = [
        dict(x=7, y=12, elevation=0, dest_map=t.const, dest_warp_id="2"),
        dict(x=8, y=12, elevation=0, dest_map=t.const, dest_warp_id="2"),
        dict(
            x=12, y=3, elevation=0, dest_map="MAP_VIRELIA_SEABREAK_LANTERN_ROOM", dest_warp_id="0"
        ),
    ]
    light.g[12][7] = 0x3301
    light.g[12][8] = 0x3301
    light.g[3][12] = 0x3300
    lantern = interior("Seabreak_LanternRoom", 13, 12)
    lantern.warps = [dict(x=6, y=10, elevation=0, dest_map=light.const, dest_warp_id="2")]
    lantern.g[10][6] = 0x3301
    for x, label in [(3, "Feed"), (6, "Lens"), (9, "Beacon")]:
        lantern.sign(x, 5, label)
        lantern.g[5][x] = 0x060C
    lantern.sign(9, 8, "Manifest")
    lantern.g[8][9] = 0x060D
    lantern.g[2][6] = 0x060C

    # Register in a fixed, append-only order, independent of construction order.
    for m in (f, u, lo, ll, t, center, light, lantern, keeper):
        m.finish()
    jw(ROOT / "data/layouts/layouts.json", LAYOUTS)
    jw(ROOT / "data/maps/map_groups.json", GROUPS)
    secpath = ROOT / "src/data/region_map/region_map_sections.json"
    sections = json.loads(secpath.read_text(encoding="utf-8"))
    for ident, name, x, y in [
        ("MAPSEC_VIRELIA_RAINFALL_PASS", "Rainfall Pass", 5, 8),
        ("MAPSEC_VIRELIA_SEABREAK_TOWN", "Seabreak Town", 6, 8),
    ]:
        if not any(s["id"] == ident for s in sections["map_sections"]):
            sections["map_sections"].append(dict(id=ident, name=name, x=x, y=y, width=1, height=1))
    jw(secpath, sections)
    # Patch only the north cleft; the rest of Slateford stays byte-for-byte authored.
    city = grid("Virelia_SlatefordCity")
    for y in (0, 1):
        for x in (11, 12, 13):
            city[y][x] = 0x0479
    city[0][12] = 0x049F
    city[1][12] = 0x30A7
    city[2][12] = 0x040E
    city[2][11] = 0x040E
    city[2][13] = 0x040E
    city[3][12] = 0x3001
    ww(ROOT / "data/layouts/Virelia_SlatefordCity/map.bin", [v for row in city for v in row])
    cityp = ROOT / "data/maps/Virelia_SlatefordCity/map.json"
    doc = json.loads(cityp.read_text(encoding="utf-8"))
    if not any(w["dest_map"] == f.const for w in doc["warp_events"]):
        doc["warp_events"].append(dict(x=12, y=1, elevation=0, dest_map=f.const, dest_warp_id="0"))
    if not any(
        b["script"] == "Virelia_SlatefordCity_EventScript_RainfallGate" for b in doc["bg_events"]
    ):
        doc["bg_events"].append(
            dict(
                type="sign",
                x=12,
                y=2,
                elevation=0,
                player_facing_dir="BG_EVENT_PLAYER_FACING_ANY",
                script="Virelia_SlatefordCity_EventScript_RainfallGate",
            )
        )
    jw(cityp, doc)
    coast.finish()
    # Add only new tileset symbols; all existing declarations and assets are untouched.
    for symbol, folder in [
        ("VireliaRainfall", "virelia_rainfall"),
        ("VireliaCoast", "virelia_coast"),
        ("VireliaLighthouse", "virelia_lighthouse"),
    ]:
        p = ROOT / "include/tilesets.h"
        s = p.read_text(encoding="utf-8")
        needle = "extern const struct Tileset gTileset_" + symbol + ";"
        if needle not in s:
            s = s.replace(
                "extern const struct Tileset gTileset_VireliaOutdoor;",
                "extern const struct Tileset gTileset_VireliaOutdoor;\n" + needle,
            )
            p.write_text(s, encoding="utf-8")
        p = ROOT / "src/data/tilesets/headers.h"
        s = p.read_text(encoding="utf-8")
        decl = "const struct Tileset gTileset_" + symbol
        if decl not in s:
            snippet = (
                "\n"
                + decl
                + " =\n{\n    .isCompressed = TRUE,\n    .isSecondary = TRUE,\n    .tiles = gTilesetTiles_"
                + symbol
                + ",\n    .palettes = gTilesetPalettes_"
                + symbol
                + ",\n    .metatiles = gMetatiles_"
                + symbol
                + ",\n    .metatileAttributes = gMetatileAttributes_"
                + symbol
                + ",\n    .callback = NULL,\n};\n"
            )
            s = s.replace(
                "const struct Tileset gTileset_VireliaOutdoor",
                snippet + "\nconst struct Tileset gTileset_VireliaOutdoor",
            )
            p.write_text(s, encoding="utf-8")
        p = ROOT / "src/data/tilesets/graphics.h"
        s = p.read_text(encoding="utf-8")
        if "gTilesetTiles_" + symbol not in s:
            tile_image = Image.open(ROOT / f"data/tilesets/secondary/{folder}/tiles.png")
            tilecount = tile_image.width * tile_image.height // 64
            snippet = (
                f'const u32 gTilesetTiles_{symbol}[] = INCGFX_U32("data/tilesets/secondary/{folder}/tiles.png", ".4bpp.fastSmol", "-num_tiles {tilecount} -Wnum_tiles");\nconst u16 gTilesetPalettes_{symbol}[][16] =\n{{\n'
                + "".join(
                    f'    INCGFX_U16("data/tilesets/secondary/{folder}/palettes/{i:02}.pal", ".gbapal"),\n'
                    for i in range(16)
                )
                + "};\n\n"
            )
            s = s.replace(
                "const u32 gTilesetTiles_VireliaOutdoor",
                snippet + "const u32 gTilesetTiles_VireliaOutdoor",
            )
            p.write_text(s, encoding="utf-8")
        p = ROOT / "src/data/tilesets/metatiles.h"
        s = p.read_text(encoding="utf-8")
        if "gMetatiles_" + symbol not in s:
            snippet = f'const u16 gMetatiles_{symbol}[] = INCBIN_U16("data/tilesets/secondary/{folder}/metatiles.bin");\nconst u16 gMetatileAttributes_{symbol}[] = INCBIN_U16("data/tilesets/secondary/{folder}/metatile_attributes.bin");\n'
            s = s.replace(
                "const u16 gMetatiles_VireliaOutdoor",
                snippet + "const u16 gMetatiles_VireliaOutdoor",
            )
            p.write_text(s, encoding="utf-8")
    contract = dict(
        dynamic_tiles=[
            dict(
                map="Virelia_SlatefordCity",
                x=12,
                y=2,
                flag="FLAG_VIRELIA_FIRST_GYM_COMPLETE",
                open_block=0x3001,
                closed_block=0x040E,
            ),
            dict(
                map="Virelia_RainfallLowlands",
                x=15,
                y=13,
                flag="FLAG_VIRELIA_RAINFALL_RIVAL_COMPLETE",
                open_block=0x3121,
                closed_block=0x040E,
            ),
        ],
        completed_flags=["FLAG_VIRELIA_LOWER_SHORTCUT"],
        entry=[f.name, 14, 25],
        exit=[t.name, 14, 2],
        branches=dict(upper=u.name, lower=lo.name),
        shortcut=dict(
            flag="FLAG_VIRELIA_LOWER_SHORTCUT",
            lower=[lo.name, 18, 20],
            lower_destination=[lo.name, 17, 20],
            foothills=[f.name, 26, 23],
            foothills_destination=[f.name, 25, 23],
        ),
        center_anchor=[8, 14],
        beacon=[lantern.name, 6, 2, 0x20C, 0x20D],
    )
    jw(ROOT / "tools/virelia/rainfall_contract.json", contract)
    # Final stock-art composition is a separate authoring pass. Never run this
    # full builder over later hand-authored Porymap changes without a backup.
    import polish_rainfall
    import polish_seabreak

    polish_rainfall.ROOT = ROOT
    polish_seabreak.ROOT = ROOT
    polish_seabreak.COAST = ROOT / "data/tilesets/secondary/virelia_coast"
    polish_seabreak.PRIMARY = ROOT / "data/tilesets/primary/general"
    polish_seabreak.source.cache_clear()
    polish_rainfall.build()
    polish_seabreak.main()
    print(
        "Built nine Chapter 2 maps with final stock-art composition; existing town patch confined to north entrance."
    )


if __name__ == "__main__":
    build()
