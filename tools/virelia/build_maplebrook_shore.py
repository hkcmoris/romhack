#!/usr/bin/env python3
"""Keep Maplebrook's straight bank artwork below actors without editing General."""
from pathlib import Path
import struct

ROOT = Path(__file__).resolve().parents[2]
BANK = ROOT / 'data/tilesets/secondary/virelia_outdoor'
SOURCE_ID = 0x0F6
SHORE_ID = 0x367
SHORE_IDS = (SHORE_ID,)


def words(path):
    raw = path.read_bytes()
    return list(struct.unpack(f'<{len(raw)//2}H', raw))


def build():
    primary = words(ROOT / 'data/tilesets/primary/general/metatiles.bin')
    attrs_primary = words(ROOT / 'data/tilesets/primary/general/metatile_attributes.bin')
    meta = words(BANK / 'metatiles.bin')
    attrs = words(BANK / 'metatile_attributes.bin')
    index = SHORE_ID - 512
    source = primary[SOURCE_ID * 8:(SOURCE_ID + 1) * 8]
    if len(attrs) > index and meta[index * 8:index * 8 + 8] not in (source, [0]*8):
        raise ValueError('Shore alias would replace unrelated shared art')
    meta.extend([0] * max(0, (index + 1) * 8 - len(meta)))
    attrs.extend([0] * max(0, index + 1 - len(attrs)))
    meta[index * 8:index * 8 + 8] = source
    attrs[index] = attrs_primary[SOURCE_ID] & 0x0FFF | 0x1000
    path = ROOT / 'data/layouts/Virelia_MaplebrookTown/map.bin'
    cells = words(path)
    cells = [(value & ~0x3FF) | SHORE_ID if value & 0x3FF == SOURCE_ID else value for value in cells]
    for path, data in ((BANK / 'metatiles.bin', meta), (BANK / 'metatile_attributes.bin', attrs), (path, cells)):
        path.write_bytes(struct.pack(f'<{len(data)}H', *data))
    print('PASS: straight Maplebrook banks draw below actors; corner art and stock General remain unchanged')


if __name__ == '__main__':
    build()
