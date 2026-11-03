"""Continuous Emerald cliff rims and their exposed feet on the coast."""

import struct


# These formerly empty General metatiles are visible from both sides of three
# map connections. Their quarters are all immutable General graphics.
SHARED_CRESTS = {
    (1, 0): 0x02A, (0, 1): 0x05C,
    (-1, 0): 0x135, (0, -1): 0x13F,
    (-1, 1): 0x1A2, (1, -1): 0x1B6,
}


def turns(west, center, east):
    return ((west > center) - (west < center), (east > center) - (east < center))


def crest_quarters(sides):
    """Connect the midpoint of this rim to the height of each neighbor."""
    # General 05A is a continuous triangular patch of the same dark rock as
    # the wall's face. The stock D5/D6 ends turn *up* underneath a short lip.
    west, east = sides
    left = {-1:(0x05A,0x097), 0:(0,0x088), 1:(0,0x45A)}[west]
    right = {-1:(0x45A,0x099), 0:(0,0x089), 1:(0,0x05A)}[east]
    return [left[0],right[0],left[1],right[1]]


def install_shared_crests(primary):
    path = primary / "metatiles.bin"
    attr_path = primary / "metatile_attributes.bin"
    definitions = bytearray(path.read_bytes())
    attributes = bytearray(attr_path.read_bytes())
    grass = bytes(definitions[16:24])
    walk = bytes(attributes[0x087*2:0x087*2+2])
    dirty = False
    for sides, tile in SHARED_CRESTS.items():
        exposed = [0x3000 | q if q else 0 for q in crest_quarters(sides)]
        new = grass + struct.pack("<4H", *exposed)
        old = definitions[tile*16:(tile+1)*16]
        attr = attributes[tile*2:tile*2+2]
        if old == new and attr == walk:
            continue
        if any(old) or any(attr):
            raise ValueError(f"General cliff slot {tile:03X} is already occupied")
        definitions[tile*16:(tile+1)*16] = new
        attributes[tile*2:tile*2+2] = walk
        dirty = True
    if dirty:
        path.write_bytes(definitions)
        attr_path.write_bytes(attributes)


def shared_crest(sides):
    return 0x087 if sides == (0,0) else SHARED_CRESTS[sides]


def coast_outline(surfaces, template, ground, sides, foot=False):
    """Draw the stone outline over the actual upper shelf or lower street."""
    original = surfaces.pieces(template)
    floor = surfaces.pieces(ground)[:4]
    if foot:
        # Keep the naturally rounded stock foot. Its donor uses meadow in
        # the transparent corner, which becomes a bright square on a street.
        # Extract just the rock/small cast shadow and reveal the actual floor.
        west, east = sides
        outline = []
        for q in range(4):
            tile = 0x078 if q % 2 == 0 and west < 0 else 0x07A if q % 2 and east < 0 else 0x079
            source = surfaces.bank.general[tile*8:tile*8+8]
            outline.append((source[q+4] or source[q]) & 0xFFF)
        if ground in surfaces.paving:
            # At the quay the toe rests on stone, so it has no band of moss
            # or green reflected light. Leave pavers visible at its bevels.
            outline = [0x097,0x098,
                       0x09F if west < 0 else 0x06E,
                       0x09C if east < 0 else 0x06D]
    else:
        outline = crest_quarters(sides)
    def palette(q):
        # Contact metatiles can use different stone colors in each quarter.
        for ref in (original[q+4],original[q],original[q%2+6],original[q%2+2]):
            if ref >> 12 in (3,7,10):
                return ref & 0xF000
        return 0xA000
    top = [palette(q) | ref if ref else 0 for q, ref in enumerate(outline)]
    return surfaces.variant(template, floor + top)
