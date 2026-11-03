"""Pack the pump sump's cut banks around General's animated water."""

try:
    from .quarry_contours import NEIGHBORS, normalize
except ImportError:
    from quarry_contours import NEIGHBORS, normalize


# Four broad cuts leave room for the pump intake at the west bank. Endpoints
# are inclusive; a shelf is at least two tiles wide at each change in depth.
SUMP = frozenset((x, y) for y1, y2, x1, x2 in (
    (6, 8, 31, 34), (9, 11, 28, 34), (12, 14, 26, 34),
    (15, 17, 26, 35), (18, 20, 26, 31),
) for y in range(y1, y2 + 1) for x in range(x1, x2 + 1))
INTAKE = (26, 13)


def shore_mask(x, y):
    return normalize(sum(1 << i for i, (dx, dy) in enumerate(NEIGHBORS)
                         if (x + dx, y + dy) not in SUMP))


def shore_pixel(mask, x, y):
    """Signed distance into the cut, in 64ths of its visible face depth."""
    west, north = x < 8, y < 8
    side_x, side_y = (8 if west else 2), (1 if north else 4)
    diagonal = (16 if west else 32) if north else (128 if west else 64)
    # The north wall shows its strata; the near (south) lip hides its face.
    dx = ((x if west else 15 - x) * 2 + 1) * 32 // 5
    dy = ((y if north else 15 - y) * 2 + 1) * 32 // (8 if north else 4)
    if mask & side_x and mask & side_y:
        depth = min(dx, dy, (dx + dy) // 2 - 20)
    elif mask & side_y:
        depth = dy
    elif mask & side_x:
        depth = dx
    elif mask & diagonal:
        depth = dx + dy
    else:
        return 0

    if depth < 12:
        return 5  # continues the dusty apron outside the cut
    if depth < 21:
        return 6
    if depth < 40:
        return 4 if (x + 2 * y) % 13 < 4 else 3
    if depth < 52:
        return 2
    if depth < 61:
        return 1
    if depth < 68:
        return 12  # damp stone meeting the water
    return 0


def pipe_pixel(x, y, intake):
    # A low iron line with bolted collars and a slotted strainer at its mouth.
    if intake:
        if 4 <= x <= 11 and 3 <= y <= 11:
            if x in (4, 11) or y in (3, 11):
                return 2
            return 1 if y in (6, 8, 10) else 4
        if x >= 4:
            return 0
    if 5 <= y <= 10:
        if x in (2, 3, 12, 13):
            return (2, 6, 4, 3, 2, 1)[y - 5]
        return (0, 4, 3, 2, 1, 0)[y - 5]
    return 0


def pack_sump(add, graphic, general, bank):
    # Frames for the two graphics in the calm-water metatile use indices
    # 11..14. Give them a mineral-green ramp in this bank's unused slot 9.
    water_palette = [(0, 0, 0)] * 16
    # The mid-height working shelf uses indices 1-10 of this slot. Its
    # floor and road shoulders share index 5, including underneath track.
    water_palette[1:11] = [(80,88,88), (112,120,120), (144,152,144), (160,168,152),
                           (184,184,168), (200,200,176), (216,216,192), (208,192,168),
                           (160,152,128), (184,168,144)]
    water_palette[11:15] = [(152, 208, 192), (112, 184, 184),
                            (80, 152, 160), (64, 128, 144)]
    rows = (' '.join(map(str, color)) for color in water_palette)
    (bank / 'palettes/09.pal').write_text('JASC-PAL\n0100\n16\n' + '\n'.join(rows) + '\n', newline='\n')
    water = [(value & 0xfff) | 0x9000 for value in general[0x170 * 8:0x170 * 8 + 4]]

    def quarters(pixel):
        result = []
        for y0 in (0, 8):
            for x0 in (0, 8):
                data = tuple(pixel(x0+x, y0+y) for y in range(8) for x in range(8))
                result.append(0x8000 | graphic(data) if any(data) else 0)
        return result

    for mask in sorted({shore_mask(x, y) for x, y in SUMP}):
        bank_face = quarters(lambda x, y: shore_pixel(mask, x, y))
        add(f'SHORE_{mask:02X}', water + bank_face)
    mask = shore_mask(*INTAKE)
    mouth = quarters(lambda x, y: pipe_pixel(x, y, True) or shore_pixel(mask, x, y))
    add('SUMP_INTAKE', water + mouth)
    add('SUMP_PIPE', quarters(lambda x, y: pipe_pixel(x, y, False) or 5) + [0]*4)


def paint_sump(grid, ids):
    for x, y in SUMP:
        grid[y][x] = 0x3400 | ids[f'SHORE_{shore_mask(x, y):02X}']
    intake_x, y = INTAKE
    grid[y][intake_x] = 0x3400 | ids['SUMP_INTAKE']
    for x in range(24, intake_x):
        grid[y][x] = 0x3000 | ids['SUMP_PIPE']
