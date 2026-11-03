"""Coastal turf margins, cobblestone corners and eroded paving over earth."""

from pathlib import Path

from PIL import Image


# Dewford's roof uses eight colors in slot 9. Keep those colors and move six
# indices out of the way of General's native dirt/verge pattern (11..15).
ROOF_REINDEX = bytes({10:1, 11:6, 12:5, 13:8, 14:9, 15:10}.get(i, i) for i in range(256))
COBBLES = (0x208,0x209,0x20A, 0x210,0x211,0x212, 0x218,0x219,0x21A)


def floor_palettes(coast, primary, slateport):
    def swatches(path):
        return path.read_text().splitlines()[3:19]

    def save(slot, colors):
        (coast / f'palettes/{slot:02}.pal').write_text('\n'.join(['JASC-PAL','0100','16',*colors]) + '\n')

    # The same Slateport cobble rim can reveal turf (slot 6) or heath (7).
    paving = swatches(slateport / 'palettes/06.pal')
    heath = swatches(coast / 'palettes/10.pal')
    colors = swatches(coast / 'palettes/07.pal')
    colors[2:4] = paving[2:4]
    colors[15] = heath[3]
    save(7, colors)
    original = swatches(slateport / 'palettes/09.pal')
    colors = original.copy()
    for index in (4,7,10,11,12,13,14,15):
        colors[ROOF_REINDEX[index]] = original[index]
    grass = swatches(primary / 'palettes/02.pal')
    colors[11:16] = [heath[2], heath[3], '116 157 133', '112 180 150', grass[13]]
    save(9, colors)


class CoastSurfaces:
    def __init__(self, bank, heath):
        self.bank = bank
        self.heath = heath
        self.sheet = Image.open(Path(bank.harbor.filename).with_name('floors.png'))
        if self.sheet.mode != 'P' or self.sheet.size != (24,8) or self.sheet.info.get('transparency') != 0:
            raise ValueError('floors.png must contain three indexed 8x8 quarters with clear index zero')
        self.art = [bank.graphic(self.sheet.crop((i,0,i+8,8)).tobytes()) for i in range(0,24,8)]
        # The northwest coping stone of the terrace is also the small piece
        # of cobble seen around a round planting hole.
        self.planter_corner = bank.graphic(bank.terrace.crop((40,24,48,32)).tobytes())
        self.stone = bank.tile('slateport', 0x202)
        self.cobble_center = bank.tile('slateport', 0x211)
        self.cobbles = {bank.tile('slateport', t) for t in COBBLES}
        self.bollard = bank.tile('slateport', 0x238)
        self.grass = {1, 2, 4, 0xD}
        self.paving = self.cobbles | {self.stone, self.bollard}

    def pieces(self, tile):
        table = self.bank.general if tile < 512 else self.bank.meta
        offset = tile % 512 * 8
        return table[offset:offset+8]

    def variant(self, source, pieces):
        attr = self.bank.attrs[source-512] if source >= 512 else self.bank.general_attrs[source]
        key = ('floor', attr, *pieces)
        if key not in self.bank.ids:
            self.bank.ids[key] = 512 + len(self.bank.attrs)
            self.bank.attrs.append(attr)
            self.bank.meta.extend(pieces)
        return self.bank.ids[key]

    def worn_landing(self, pavement, soil, top=False, bottom=False):
        """A bed of exposed soil shows through the first rank of laid stones."""
        floor = self.pieces(pavement)
        dust = self.pieces(soil)
        for q, rim in ((0,top), (2,bottom)):
            floor[q] = dust[q]
            floor[q+4] = 0x6000 | (self.art[2 if rim else 0] ^ (0x800 if q == 2 else 0))
        return self.variant(pavement, floor)

    def foundations(self, grid, underlays):
        # Donor buildings carry their original dirt right up to the doorstep.
        # Keep the complete facade and its behavior, but set it on its actual street.
        native_floors = (self.pieces(1)[:4], self.pieces(0x121)[:4])
        for (x,y), under in underlays.items():
            if under not in self.paving:
                continue
            raw = grid[y][x]
            tile = raw & 1023
            original = self.pieces(tile)
            if tile == 4:
                # The four animated General flower quarters are opaque,
                # including their grass. Expose paving at the corners instead
                # of stamping that background onto the street as a green box.
                floor = self.pieces(under)[:4]
                center = self.pieces(self.cobble_center)[:4]
                corners = [0x6000 | (self.planter_corner ^ flip) if floor[q] == center[q] else 0
                           for q, flip in enumerate((0,0x400,0x800,0xC00))]
                grid[y][x] = (raw & ~1023) | self.variant(tile, original[4:] + corners)
                continue
            if original[:4] in native_floors and any(original[4:]):
                grid[y][x] = (raw & ~1023) | self.variant(tile, self.pieces(under)[:4] + original[4:])

    def finish(self, grid, soil, native_west=0):
        original = [[v & 1023 for v in row] for row in grid]
        w, h = len(grid[0]), len(grid)
        grass = self.grass
        turf = grass | {self.heath}
        square = self.pieces(self.stone)[0]

        def at(x,y):
            return original[y][x] if 0 <= x < w and 0 <= y < h else None

        def grassy(x,y):
            return 0 <= x < w and 0 <= y < h and soil[y][x] in (1,2)

        # Retain the native paving stone sizes and brick bonds. Per-quarter
        # placement permits an outside corner and a concave corner in one cell.
        for y, row in enumerate(original):
            for x in range(native_west, w):
                tile = row[x]
                if tile != self.heath and tile not in self.paving:
                    continue
                floor = self.pieces(tile)
                for q, (dx,dy) in enumerate(((-1,-1),(1,-1),(-1,1),(1,1))):
                    side, end = at(x+dx,y), at(x,y+dy)
                    flip = (0x400 if dx > 0 else 0) | (0x800 if dy > 0 else 0)
                    if tile == self.heath:
                        # Follow the actual turf contour beneath the road. A
                        # road touching heath does not create a green verge.
                        lateral, vertical = grassy(x+dx,y), grassy(x,y+dy)
                        if lateral or vertical:
                            graphic = 0xF7 if lateral and vertical else 0x107 if lateral else 0xF8
                            floor[q] = 0x9000 | graphic | flip
                    else:
                        if tile == self.bollard and floor[q] != square:
                            continue
                        # Every cobble quarter uses just stone light/shadow and
                        # a ground color. In worn corners keep the local soil,
                        # including where a door or sign hides the adjacent cell.
                        if tile in self.cobbles and soil[y][x] == self.heath:
                            floor[q] = 0x7000 | (floor[q] & 0xFFF)
                        lateral, vertical = side in turf, end in turf
                        material = 7 if self.heath in (side, end) else 6
                        if lateral or vertical:
                            if tile in self.cobbles:
                                donor = 0x208 if lateral and vertical else 0x210 if lateral else 0x209
                                edge = self.pieces(self.bank.tile('slateport', donor))[0]
                                floor[q] = material << 12 | ((edge & 0xFFF) ^ flip)
                            else:
                                # Keep the quarry's rectangular bond all the way to the
                                # verge. Only the last brick ends reveal the floor below.
                                margin = 2 if lateral and vertical else 0 if lateral else 1
                                ground = self.heath if material == 7 else 1
                                floor[q] = self.pieces(ground)[q]
                                floor[4+q] = 0x6000 | (self.art[margin] ^ flip)
                        elif tile in self.cobbles and side is not None and end is not None:
                            # Native edge pieces carry turf. Where the street reaches
                            # bedrock, a foundation or another paved quarter, lay the
                            # whole last stone rather than leaving a triangular grass nick.
                            floor[q] = self.pieces(self.cobble_center)[q]
                if floor != self.pieces(tile):
                    grid[y][x] = (grid[y][x] & ~1023) | self.variant(tile, floor)
