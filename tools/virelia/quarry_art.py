"""Native-sized quarry props assembled from Emerald and FRLG stock parts.

Keep the donors at 1:1 pixel scale. Recolor into the quarry's existing 4bpp
ramps, retaining the stock facets, bevels and clusters rather than scaling art.
"""
from PIL import Image, ImageChops, ImageDraw

from stock_art import stock_part


def stamp(target, part, position=(0, 0)):
    mask = Image.frombytes('L', part.size, bytes(255 if p else 0 for p in part.tobytes()))
    target.paste(part, position, mask)


def floor(seed):
    # Rusturf's actual gravel clusters, offset for four seamless variations.
    # Preserve its indices: the quarry limestone palette recolors this bank.
    image = stock_part('general', 'rusturf_tunnel', 0x201, [(0, 0, 0)] * 16,
                       index_map={i: {10: 3, 11: 4}.get(i, i) for i in range(1, 16)})
    return ImageChops.offset(image, seed % 2 * 8, seed // 2 % 2 * 8)


def crate(palette):
    # Keep the S.S. Anne box's perspective, but construct individual planks:
    # the donor's smooth cardboard-like faces did not read as wooden cargo.
    image = Image.new('P', (16, 16))
    d = ImageDraw.Draw(image)
    d.polygon(((1, 4), (4, 1), (12, 1), (15, 3), (15, 13), (12, 15), (1, 13)), fill=6)
    d.polygon(((2, 4), (4, 2), (12, 2), (14, 3), (11, 5)), fill=9)
    d.polygon(((12, 5), (14, 4), (14, 12), (12, 13)), fill=7)
    d.rectangle((2, 5, 11, 12), fill=8)
    plank = stock_part('general', 'slateport', 0x336, palette, layers=(1,))
    stamp(image, plank.crop((2, 3, 11, 5)), (2, 7))
    for y in (7, 10):
        d.line((2, y, 11, y), fill=6)
        d.line((2, y + 1, 10, y + 1), fill=9)
    # Short staggered grain, a knot, end grain on the lid and shaded side.
    for line in ((3, 6, 5, 6), (8, 9, 10, 9), (4, 12, 7, 12),
                 (5, 2, 3, 4), (9, 2, 7, 4), (12, 3, 10, 4),
                 (13, 6, 13, 8), (13, 10, 13, 11)):
        d.line(line, fill=7)
    d.point((8, 6), fill=6)
    d.point((9, 6), fill=10)
    d.line((2, 5, 2, 12), fill=10)
    d.line((11, 5, 11, 13), fill=6)
    # A diagonal nailed brace separates cargo from a closed cardboard carton.
    d.line((3, 12, 10, 5), fill=6, width=3)
    d.line((3, 11, 9, 5), fill=10)
    d.line((4, 12, 10, 6), fill=8)
    for point in ((3, 5), (10, 5), (3, 12), (10, 12)):
        d.point(point, fill=4)
    d.line((3, 14, 11, 14), fill=1)
    return image


def machine(kind, palette, on=False):
    image = Image.new('P', (16, 16))
    d = ImageDraw.Draw(image)
    if kind == 'pump':
        # Centrifugal water pump: elbow outlet, round volute, ribbed motor,
        # pressure gauge and two bolted feet. Its pipe remains visible off.
        d.line((2, 14, 14, 14), fill=1)
        d.rectangle((4, 12, 12, 13), fill=2)
        d.point((4, 12), fill=5)
        d.point((12, 12), fill=5)
        motor = stock_part('building', 'battle_factory', 0x2da, palette, layers=(1,))
        stamp(image, motor.crop((0, 8, 5, 14)), (10, 7))
        d.rectangle((10, 7, 14, 11), fill=2)
        d.line((10, 7, 13, 7), fill=5)
        for x in (10, 12, 14):
            d.line((x, 8, x, 10), fill=3)
        d.ellipse((3, 5, 10, 12), fill=1)
        d.ellipse((3, 5, 9, 11), fill=11)
        d.arc((3, 5, 9, 11), 180, 300, fill=13)
        d.ellipse((5, 7, 8, 10), fill=12)
        d.point((6, 8), fill=5)
        d.point((7, 9), fill=11)
        # Broad bent pipe and open downward spout; no generic tank silhouette.
        d.line((6, 5, 6, 2, 1, 2, 1, 6), fill=2, width=3)
        d.line((6, 5, 6, 2, 1, 2, 1, 5), fill=4)
        d.line((1, 1, 5, 1), fill=5)
        d.line((0, 6, 2, 6), fill=1)
        d.ellipse((9, 2, 12, 5), fill=2)
        d.rectangle((10, 3, 11, 4), fill=5)
        d.point((11, 3 if on else 4), fill=14)
        d.point((13, 8), fill=13 if on else 14)
        if on:
            d.line((1, 7, 1, 9), fill=12)
            d.point((1, 8), fill=13)
    else:
        # Squared electrical cabinet: yellow lightning, breaker handle,
        # porcelain terminals and heavy outgoing cable match its interaction.
        d.rectangle((2, 2, 12, 13), fill=1)
        d.rectangle((3, 2, 10, 11), fill=3)
        d.line((3, 1, 10, 1, 12, 3), fill=5)
        d.line((3, 3, 3, 11), fill=4)
        d.line((11, 3, 11, 12), fill=2)
        d.line((3, 12, 10, 12), fill=2)
        d.rectangle((4, 4, 8, 10), fill=2)
        d.polygon(((7, 4), (5, 7), (7, 7), (5, 10), (8, 6), (6, 6)), fill=10)
        d.point((7, 4), fill=15)
        d.line((10, 6, 10, 9), fill=1)
        d.line((9, 6 if on else 9, 11, 6 if on else 9), fill=5)
        for x in (5, 8):
            d.point((x, 0), fill=4)
            d.point((x, 1), fill=2)
        d.point((9, 3), fill=13 if on else 14)
        d.line((12, 9, 15, 9), fill=1, width=3)
        d.line((12, 8, 15, 8), fill=8)
        d.line((12, 7, 15, 7), fill=10)
        d.line((3, 14, 12, 14), fill=1)
    return image


def rail(kind, wood):
    image = Image.new('P', (16, 16))
    draw = ImageDraw.Draw(image)
    mask = Image.new('1', (16, 16))
    md = ImageDraw.Draw(mask)
    if kind in ('h', 'v'):
        tie = wood.crop((2, 7, 14, 10))
        for y in (2, 10):
            stamp(image, tie, (2, y))
            draw.line((2, y + 2, 13, y + 2), fill=6)
        for x in (4, 10):
            md.rectangle((x, 0, x + 1, 15), fill=1)
        if kind == 'h':
            image = image.transpose(Image.Transpose.ROTATE_90)
            mask = mask.transpose(Image.Transpose.ROTATE_90)
    else:
        draw.line((3, 2, 13, 12), fill=6, width=3)
        draw.line((3, 1, 13, 11), fill=8)
        # Two-pixel rails at 4..5 and 10..11 stay symmetric when flipped.
        md.polygon(((4, 0), (5, 0), (5, 3), (12, 10), (15, 10),
                    (15, 11), (11, 11), (4, 4)), fill=1)
        md.polygon(((10, 0), (11, 0), (11, 1), (14, 4), (15, 4),
                    (15, 5), (13, 5), (10, 2)), fill=1)
        if 'w' in kind:
            image = image.transpose(Image.Transpose.FLIP_LEFT_RIGHT)
            mask = mask.transpose(Image.Transpose.FLIP_LEFT_RIGHT)
        if 's' in kind:
            image = image.transpose(Image.Transpose.FLIP_TOP_BOTTOM)
            mask = mask.transpose(Image.Transpose.FLIP_TOP_BOTTOM)
    # Shade AFTER orienting the geometry, keeping highlights upper-left.
    # Border continuations do not receive end-cap highlights.
    for y in range(16):
        for x in range(16):
            if mask.getpixel((x, y)):
                lit = (y > 0 and not mask.getpixel((x, y - 1))) or (x > 0 and not mask.getpixel((x - 1, y)))
                image.putpixel((x, y), 4 if lit else 2)
    return image


def props(machinery, minerals):
    images = {}
    for kind in ('pump', 'feed'):
        for on in (False, True):
            images[f'{kind}_{"on" if on else "off"}'] = machine(kind, machinery, on), 7, False
    for i, name in enumerate(('floor_a', 'floor_b', 'floor_c')):
        images[name] = floor(i + 31), 6, True

    wood = crate(machinery)
    # The same native cargo-box planks tie the sleepers, props and supports
    # together. Rail highlights stay on the lit edge; dark ballast sits below.
    for kind in ('h', 'v', 'ne', 'nw', 'se', 'sw'):
        images['rail_' + kind] = rail(kind, wood), 7, False

    images['board'] = stock_part('general', 'slateport', 0x294, machinery, layers=(1,)), 7, False
    images['crate'] = wood, 7, False
    for name in ('timber', 'lamp', 'water', 'drain', 'cable_h', 'cable_v'):
        im = Image.new('P', (16, 16))
        d = ImageDraw.Draw(im)
        if name == 'timber':
            d.rectangle((3, 0, 7, 15), fill=6)
            d.line((4, 0, 4, 15), fill=9)
            d.line((5, 0, 5, 15), fill=8)
            d.line((6, 0, 6, 15), fill=7)
            d.line((5, 1, 5, 2), fill=7)
            d.line((5, 7, 5, 9), fill=6)
            d.point((6, 8), fill=9)
            for y in (3, 11):
                d.rectangle((2, y, 8, y + 2), fill=2)
                d.line((2, y, 7, y), fill=4)
                d.point((3, y + 1), fill=5)
        elif name == 'lamp':
            stamp(im, wood.crop((4, 2, 7, 15)), (7, 3))
            # A native rounded metal cap over warm glass and a shaded cage.
            cap = stock_part('general', 'inside_ship', 0x235, machinery, layers=(1,))
            stamp(im, cap.crop((3, 0, 13, 5)), (3, 1))
            d.rectangle((4, 5, 11, 11), fill=2)
            d.rectangle((5, 5, 10, 9), fill=10)
            d.line((5, 5, 8, 5), fill=15)
            d.line((5, 6, 5, 8), fill=15)
            d.line((8, 5, 8, 9), fill=7)
            d.line((4, 10, 10, 10), fill=4)
        elif name == 'water':
            # Emerald water clusters clipped into a shallow, irregular pool.
            water = stock_part('general', 'cave', 0x275, machinery,
                               index_map={i: (11 if i < 10 else 12) for i in range(1, 16)})
            mask = Image.new('L', (16, 16))
            md = ImageDraw.Draw(mask)
            md.polygon(((1, 6), (4, 3), (10, 3), (14, 6), (15, 10), (12, 13), (3, 12), (0, 9)), fill=255)
            im.paste(water, (0, 0), mask)
            d.line((3, 5, 5, 4, 9, 4), fill=12)
            d.line((3, 12, 11, 13, 14, 11), fill=2)
            d.point((11, 9), fill=13)
        elif name == 'drain':
            vent = stock_part('building_frlg', 'power_plant_frlg', 0x2f1, machinery,
                               layers=(1,), index_map={7: 1, 10: 1, 11: 3, 13: 2, 15: 4})
            stamp(im, vent.crop((2, 5, 14, 14)), (2, 5))
            d.line((2, 14, 13, 14), fill=1)
        else:
            d.line((0, 9, 15, 9), fill=1, width=3)
            d.line((0, 8, 15, 8), fill=8)
            d.line((0, 7, 15, 7), fill=10)
            for x in (3, 12):
                d.line((x, 7, x, 10), fill=3)
                d.point((x, 7), fill=5)
            if name == 'cable_v':
                im = im.transpose(Image.Transpose.ROTATE_90)
        images[name] = im, 7, False

    for color, index, seam_index, mapping in (
            ('blue', 0x31e, 0x31f, {8: 10, 9: 9, 10: 8, 11: 6}),
            ('red', 0x327, 0x326, {8: 15, 12: 14, 13: 13, 14: 11})):
        # Lift only the native gemstones from Emerald's cave rock face.
        gems = stock_part('general', 'cave', index, minerals, layers=(0,), index_map=mapping)
        seam = stock_part('general', 'cave', seam_index, minerals, layers=(0,), index_map=mapping)
        # A chipped stone socket seats the gems in the wall; isolated bright
        # facets without the donor's surrounding rock looked suspended in air.
        embedded = Image.new('P', (16, 16))
        for y in range(16):
            for x in range(16):
                if seam.getpixel((x, y)):
                    for dx, dy, shade in ((-1, 0, 4), (0, -1, 4), (1, 0, 2), (0, 1, 2)):
                        nx, ny = x + dx, y + dy
                        if 0 <= nx < 16 and 0 <= ny < 16:
                            embedded.putpixel((nx, ny), shade)
        stamp(embedded, seam)
        images[f'{color}_seam'] = embedded, 8, False
        # Loose ore uses the same facets set in a stock boulder foot/shadow.
        ore = stock_part('general', 'rusturf_tunnel', 0x203, minerals, layers=(1,),
                          index_map={1: 5, 2: 5, 3: 4, 4: 4, 5: 3, 6: 2, 7: 1,
                                     8: 5, 9: 5, 10: 4, 11: 3, 12: 2, 13: 2, 14: 1, 15: 1})
        stamp(ore, gems)
        images[f'{color}_ore'] = ore, 8, False
    return images


def cart(palette):
    # Reuse the crate's native perspective for a steel ore hopper.
    image = crate(palette).point([0, 1, 2, 3, 4, 5, 1, 2, 3, 4, 5, 11, 12, 13, 14, 15] + list(range(16, 256)))
    rock = stock_part('general', 'rusturf_tunnel', 0x203, palette, layers=(1,),
                      index_map={1: 5, 2: 4, 3: 4, 4: 3, 5: 3, 6: 2, 7: 1,
                                 8: 5, 9: 4, 10: 4, 11: 3, 12: 2, 13: 2, 14: 1, 15: 1})
    stamp(image, rock.crop((2, 1, 14, 7)), (2, 0))
    d = ImageDraw.Draw(image)
    # Steel panels get broad faces and rivets, not the wooden crate's grain
    # or diagonal brace, which became a noisy checkerboard after recoloring.
    d.rectangle((2, 8, 10, 12), fill=3)
    d.line((11, 8, 11, 12), fill=2)
    d.line((3, 10, 10, 10), fill=2)
    d.point((3, 8), fill=5)
    d.point((10, 8), fill=4)
    d.line((2, 7, 13, 7), fill=5)
    for x in (3, 10):
        d.rectangle((x, 12, x + 2, 15), fill=1)
        d.point((x + 1, 13), fill=4)
    return image


def shaft(palette):
    return stock_part('general', 'cave', 0x205, palette)
