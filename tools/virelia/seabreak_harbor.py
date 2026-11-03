"""Seabreak's sheltered fishing slip, cargo pier and shipyard berths."""

from pathlib import Path

from PIL import Image
from cliff_contours import coast_outline


def build_harbor(bank, grid, cell, slate, rock, surfaces):
    coast = Path(bank.harbor.filename).parent
    dockyard = Image.open(coast / "dockyard.png")
    if dockyard.mode != "P" or dockyard.getextrema()[1] > 15 or dockyard.info.get("transparency") != 0:
        raise ValueError("Dockyard art must be indexed 4bpp with transparent index zero")
    colors = dockyard.getpalette()[:48]
    swatches = [" ".join(map(str, colors[i:i + 3])) for i in range(0, 48, 3)]
    (coast / "palettes/12.pal").write_text("\n".join(["JASC-PAL", "0100", "16", *swatches]) + "\n")
    stone = bank.tile("slateport", 0x202)
    water = 0x170

    def panel(sheet, col, row, ground, *, covered=True, flip=False, behavior=0):
        """Lay a 16px panel over its actual floor using GBA quarter flips."""
        palette = 12 if sheet is dockyard else 11
        key = ("port", palette, col, row, ground, covered, flip, behavior)
        if key in bank.ids:
            return bank.ids[key]
        table = bank.general if ground < 512 else bank.meta
        offset = (ground % 512) * 8
        pieces = table[offset:offset + 8]
        for q in range(4):
            half = 1 - q % 2 if flip else q % 2
            x, y = col * 16 + half * 8, row * 16 + (q // 2) * 8
            pixels = sheet.crop((x, y, x + 8, y + 8)).tobytes()
            if any(pixels):
                under = pieces[4 + q]
                if under & 1023 and 0 in pixels:
                    # Cargo on a pier shares its timber palette. Combine the two
                    # foreground quarters so animated water still shows below.
                    index = (under & 1023) - 512
                    if index < 0:
                        raise ValueError(f"Port panel {(col, row)} overlaps foreground on {ground:#x}")
                    timber = Image.frombytes("P", (8, 8), bank.tiles[index])
                    if timber.getextrema()[0] > 0:
                        pieces[q] = under
                    else:
                        if under >> 12 != palette or flip:
                            raise ValueError(f"Mixed palettes in timber foreground on {ground:#x}")
                        if under & 0x400:
                            timber = timber.transpose(Image.Transpose.FLIP_LEFT_RIGHT)
                        if under & 0x800:
                            timber = timber.transpose(Image.Transpose.FLIP_TOP_BOTTOM)
                        pixels = bytes(p or bg for p, bg in zip(pixels, timber.tobytes()))
                pieces[4 + q] = (palette << 12) | (bank.graphic(pixels) ^ (0x400 if flip else 0))
        result = 512 + len(bank.attrs)
        bank.meta.extend(pieces)
        bank.attrs.append((0x1000 if covered else 0) | behavior)
        bank.ids[key] = result
        return result

    def prop(sheet, source, size, origin, footprint, *, boat=False, flip=False):
        sx, sy = source
        w, h = size
        left, top = origin
        for dy in range(h):
            for dx in range(w):
                col = sx + (w - 1 - dx if flip else dx)
                row = sy + dy
                if not sheet.crop((col * 16, row * 16, col * 16 + 16, row * 16 + 16)).getbbox():
                    continue
                x, y = left + dx, top + dy
                raw = grid[y][x]
                ground = raw & 1023
                if boat and ground != water:
                    raise ValueError(f"Boat at {origin} needs open water at {(x, y)}, found {ground:#x}")
                grounded = footprint[dy][dx] == "#"
                tile = panel(sheet, col, row, ground, covered=boat or grounded, flip=flip)
                cell(x, y, tile, bool(raw & 0xC00) or grounded, elevation=1 if boat else 3)

    # A short notch in the quay takes the trawler alongside the selling floor.
    # Its back edge is solid; the plaza immediately above it hosts the chapter scenes.
    for x in range(12, 18):
        slate(x, 37, 0x25D, True)
        cell(x, 38, 0x189, True, 1)
        cell(x, 39, water, True, 1)
        cell(x, 40, 0x198 if x == 12 else 0x19A if x == 17 else water, True, 1)
    for x, metatile in ((11, 0x23C), (18, 0x234)):
        slate(x, 38, metatile, True)
        slate(x, 39, 0x2FF if x == 11 else 0x2FE, True)

    # A low rock nose meets the paved estuary entrance. Its foot runs above
    # the street, with ordinary fractured stone instead of a recessed portal.
    for x, tile in enumerate((0x087, 0x0A9, 0x0A9, 0x07A), 1):
        piece = rock[tile]
        if x == 1:
            piece = coast_outline(surfaces, piece, surfaces.heath, (1,-1))
        elif x == 4:
            piece = coast_outline(surfaces, piece, stone, (0,-1), foot=True)
        cell(x, 34, piece, True, 3)
    for x, sides in ((1,(-1,0)),(2,(0,0)),(3,(0,-1))):
        cell(x, 35, coast_outline(surfaces, rock[0x079], stone, sides, foot=True), True, 3)
    for x in range(7):
        cell(x, 36, stone, elevation=3)

    def pier(stem, head=None):
        left, top, right, bottom = stem
        cells = {(x, y) for x in range(left, right + 1) for y in range(top, bottom + 1)}
        if head:
            x0, y0, x1, y1 = head
            cells.update((x, y) for x in range(x0, x1 + 1) for y in range(y0, y1 + 1))
        for x, y in sorted(cells):
            col = 0 if (x - 1, y) not in cells else 2 if (x + 1, y) not in cells else 1
            row = 2 if (x, y + 1) not in cells else 0 if y != top and (x, y - 1) not in cells else 1
            ground = stone if y == top else water
            cell(x, y, panel(dockyard, col, row, ground), elevation=3)

    pier((8, 39, 9, 46), (9, 40, 12, 41))
    pier((22, 38, 23, 45), (20, 44, 27, 45))
    pier((36, 36, 37, 43), (34, 40, 40, 43))
    # Wide open seawater between the three heads remains navigable by boat.
    # Slim skiffs sit against the old fishing jetty. A net boat uses the cut-in slip.
    for x, y in ((6, 42), (25, 46)):
        prop(bank.harbor, (4, 6), (2, 1), (x, y), ("..",), boat=True)
    for x, y, reverse in ((10, 43, False), (38, 44, True)):
        prop(bank.harbor, (4, 4), (3, 2), (x, y), ("...", "..."), boat=True, flip=reverse)
    for x, y, reverse in ((13, 39, False), (30, 40, True)):
        prop(bank.harbor, (0, 4), (4, 3), (x, y), ("....",) * 3, boat=True, flip=reverse)

    # Timber lifting points serve each of the three different berths.
    for x, y, reverse in ((9, 35, False), (40, 32, True)):
        prop(bank.harbor, (0, 0), (3, 3), (x, y), ("...", "...", ".##" if reverse else "##."), flip=reverse)
    # A masonry pad takes the derrick load at the end of the shipyard pier.
    # The turntable is on deck while the tackle projects over the seine boat.
    for y in range(40, 43):
        for x in (34, 35):
            cell(x, y, stone, elevation=3)
    prop(bank.harbor, (7, 0), (3, 4), (33, 39), ("...", "...", "...", ".##"))

    # Bulk freight under cover, boxed catch on the selling floor, rope and
    # barrels beside the moorings. The aisles between them reach the quay steps.
    for x, y in ((9, 32), (11, 33), (23, 32), (25, 32), (36, 31)):
        prop(dockyard, (0, 3), (2, 2), (x, y), ("..", "##"))
    for x, y in ((8, 34), (23, 34), (28, 37), (37, 33)):
        prop(dockyard, (2, 4), (2, 1), (x, y), ("##",))
    for x, y, row in ((7, 34, 1), (14, 33, 0), (25, 34, 0), (26, 34, 1),
                       (27, 32, 0), (27, 34, 2), (24, 44, 2), (39, 41, 1)):
        prop(dockyard, (3, row), (1, 1), (x, y), ("#",))
    # Mooring hardware includes the original interactive storm anchor.
    for x, y in ((7, 39), (18, 37), (5, 37)):
        slate(x, y, 0x238, True)
    for x, y in ((6, 37), (20, 38), (37, 35)):
        prop(bank.harbor, (3, 0), (1, 2), (x, y - 1), (".", "#"))
