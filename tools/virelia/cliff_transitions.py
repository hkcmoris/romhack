"""Rock contacts at the coastal fault and the ends of Seabreak's retaining walls."""

# General's granite also provides the diagonal, transparent rock fragments.
# Each reference selects an existing 8px graphic; the color and shape of the
# adjoining beds are independent, as they are in the original Emerald cliffs.
GRANITE, MIXED, SCHIST = 3, 7, 10


def install_coast_rock_palette(primary, coast):
    warm = (primary / "palettes/03.pal").read_text().splitlines()[3:]
    cool = (coast / "palettes/10.pal").read_text().splitlines()[3:]
    path = coast / "palettes/07.pal"
    colors = path.read_text().splitlines()
    for shade in range(10, 15):
        a = map(int, warm[shade].split())
        b = map(int, cool[shade].split())
        colors[3 + shade] = " ".join(str((x + y) // 2) for x, y in zip(a, b))
    path.write_text("\n".join(colors) + "\n")


def coast_contact(bank, native, phase, attribute):
    """One of four stages across a warm granite / green schist contact."""
    key = ("coastal fault", native, phase, attribute)
    if key in bank.ids:
        return bank.ids[key]
    pieces = bank.general[native*8:native*8+8]
    # A seam follows the stones across quarter boundaries. Stagger the front
    # between upper and lower quarters; the toe keeps its original turf.
    ramps = ((GRANITE, MIXED, GRANITE, GRANITE), (GRANITE, MIXED, MIXED, MIXED),
             (MIXED, MIXED, MIXED, SCHIST), (MIXED, SCHIST, SCHIST, SCHIST))
    for i, ref in enumerate(pieces):
        if ref >> 12 == GRANITE:
            pieces[i] = (ramps[phase][i % 4] << 12) | (ref & 0xFFF)
    if native == 0x0A9:
        # Follow the fracture in General graphic 098. Every opaque pixel
        # retains its original value, so only its mineral hue changes.
        vein = bytes(int(c,16) for row in (
            "00000eed", "00000eec", "00000edd", "00000ddd",
            "00000ded", "0000deee", "0000eddd", "000edddd"
        ) for c in row)
        fragment = bank.graphic(vein)
        for q in (1,2):
            shade = ramps[phase][q]
            if shade != SCHIST:
                pieces[4+q] = ((MIXED if shade == GRANITE else SCHIST) << 12) | fragment
    index = 512 + len(bank.attrs)
    bank.ids[key] = index
    bank.meta.extend(pieces)
    bank.attrs.append(attribute)
    return index


def retaining_end(bank, wall, on_left, course, at_foot):
    """Bond the last masonry blocks into bedrock, keeping the coping and turf."""
    key = ("retaining end", wall, on_left, course, at_foot)
    if key in bank.ids:
        return bank.ids[key]
    start = (wall - 512) * 8
    pieces = bank.meta[start:start+8]
    edge = 0 if on_left else 1
    # The donor's first and last half-courses include transparent pixels.
    # The central courses are opaque and can serve as the background layer.
    for row in range(2):
        if (course == 0 and row == 0) or (at_foot and row == 1):
            continue
        q = row * 2 + edge
        pieces[q] = pieces[4+q]
        # Both fragments have a chipped, transparent edge. Flip the edge
        # horizontally at the other end of the wall, never its masonry.
        fragment = 0x055 if course % 2 else (0x050 | 0x400)
        pieces[4+q] = (SCHIST << 12) | (fragment ^ (0 if on_left else 0x400)) | (0x800 if row else 0)
    index = 512 + len(bank.attrs)
    bank.ids[key] = index
    bank.meta.extend(pieces)
    bank.attrs.append(bank.attrs[wall-512])
    return index


def pack_ridge_contact(ridge, general, primary, quarters, define):
    """Expose granite beneath the broken rim, beds and toe of a 32px shale contact."""
    from PIL import Image

    # Width of intact shale on each pixel row. The front follows the projecting
    # strata, with a longer shelf beneath the moss cap and a short rubble toe.
    courses = (
        ((0x310, 0x311), 0x087, (20,20,19,19,20,21,22,20, 15,14,14,12,11,10,11,12)),
        ((0x312, 0x315), 0x0A9, (12,14,15,13,11,10,10,9, 18,20,21,21,19,18,17,16)),
        ((0x384, 0x387), 0x079, (21,19,18,17,17,18,20,20, 14,13,12,12,10,9,10,11)),
    )
    grass = primary[8:12]

    def pixels(ref):
        i = ref & 1023
        x, y = i % 16 * 8, i // 16 * 8
        return general.crop((x,y,x+8,y+8)).tobytes()

    for row, (ids, native, front) in enumerate(courses):
        shale = ridge.crop((16, row*16, 32, row*16+16))
        uncut = quarters(shale, 6)
        lower = primary[native*8:native*8+4]
        upper = primary[native*8+4:(native+1)*8]
        # The granite cap has transparent turf flecks. In mixed quarters
        # both BG layers are needed for stone; retain the original turf in
        # the lower quarter, using the same three greens in General palette 3.
        for half, metatile in enumerate(ids):
            bottom, top = lower.copy(), upper.copy()
            cut = shale.copy()
            mask = bytes(255 if half*16+x < front[y] else 0 for y in range(16) for x in range(16))
            cut.paste(0, mask=Image.frombytes("L", (16,16), mask).point(lambda p: 255-p))
            # Oxide shows in the cracks where the layers separate. Only tint
            # opaque stone; moss and turf keep their source colors.
            for y, depths in ((2,(1,2)), (3,(2,)), (6,(1,)), (11,(2,3)), (12,(1,2)), (13,(1,))):
                for depth in depths:
                    x = front[y] - depth - half*16
                    if 0 <= x < 16 and (x+half*16)//8 == (front[y]-1)//8:
                        shade = cut.getpixel((x,y))
                        if 1 <= shade <= 6:
                            cut.putpixel((x,y), 11 if shade <= 3 else 12)
            exposed = quarters(cut, 6)
            for q in range(4):
                x, y = q%2*8, q//2*8
                coverage = sum(mask[j*16+i] != 0 for j in range(y,y+8) for i in range(x,x+8))
                if coverage == 64:
                    bottom[q], top[q] = grass[q], uncut[q]
                elif coverage:
                    if native == 0x087 and q >= 2:
                        greens = {12:7, 13:15, 14:6}
                        solid = bytes(p if p else greens[g] for p,g in zip(pixels(upper[q]),pixels(lower[q])))
                        cap = Image.new("P", (16,16))
                        cap.paste(Image.frombytes("P",(8,8),solid), (x,y))
                        bottom[q] = quarters(cap, 3)[q]
                    top[q] = exposed[q]
            define(metatile, bottom, top)
