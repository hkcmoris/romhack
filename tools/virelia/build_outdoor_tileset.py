#!/usr/bin/env python3
"""Pack the connected Virelia outdoors into one graphics/metatile/palette bank.

The native connection buffer stores only metatile IDs, so every visible neighbor
must interpret them identically. Keep the stock sources untouched. Pillow is
needed only to regenerate the checked-in assets, not to build the ROM.
"""
from pathlib import Path
import json
import struct

ROOT = Path(__file__).resolve().parents[2]
TARGET = ROOT / 'data/tilesets/secondary/virelia_outdoor'
SYMBOL = 'gTileset_VireliaOutdoor'
SERVICE_SOURCE = 'virelia_services'
SERVICE_IDS = range(0x340, 0x363)
VERTICAL_BRIDGE_IDS = (0x363, 0x364)
SOURCES = {
    'Virelia_MaplebrookTown': 'petalburg',
    'Virelia_Route01': 'rustboro',
    'Virelia_SlatefordCity': 'rustboro',
    'Virelia_WhisperwoodSanctuary': 'whisperwood',
}
# Explicit source IDs make regeneration independent of which maps use a tile.
# Petalburg keeps its IDs, including the animated house and laboratory doors.
CITY_IDS = (0x256, 0x257, 0x25E, 0x25F, 0x266, 0x267, 0x27E, 0x27F,
            0x2BB, 0x2BC, 0x2BD, 0x2C4, 0x2C5, 0x2C8, 0x2C9, 0x2CA,
            0x2CB, 0x2D0, 0x2D1, 0x2D2, 0x2D3, 0x2D8, 0x2D9, 0x2DA,
            0x2DB, 0x2E0, 0x2E1, 0x2E2, 0x2E8, 0x2EA, 0x2F0, 0x2F1,
            0x2F2, 0x325, 0x326, 0x32E, 0x336)
FOREST_IDS = (0x208, 0x24B, 0x270, 0x272, 0x278, 0x281, 0x282, 0x283)
REMAP = {
    'petalburg': {i: i for i in range(0x200, 0x290)},
    'rustboro': {tile: 0x290 + i for i, tile in enumerate(CITY_IDS)},
    'whisperwood': {**{tile: 0x300 + i for i, tile in enumerate(FOREST_IDS)},
                   **{i: i for i in range(0x318, 0x336)}},
    SERVICE_SOURCE: {i: i for i in SERVICE_IDS},
}
# These complete, unmodified palettes also serve native door animation frames.
FIXED_PALETTES = {('petalburg', 6): 6, ('petalburg', 8): 8,
                  ('petalburg', 10): 10, ('whisperwood', 6): 12}


def words(path):
    raw = path.read_bytes()
    return struct.unpack(f'<{len(raw) // 2}H', raw)


def source_palette(source, slot):
    if source == 'whisperwood' and slot != 6:
        source = 'fortree'
    path = ROOT / f'data/tilesets/secondary/{source}/palettes/{slot:02}.pal'
    # Compare the actual 15-bit GBA colors, ignoring unused low RGB bits.
    return [tuple(int(c) // 8 * 8 for c in line.split())
            for line in path.read_text().splitlines()[3:19]]


def source_blocks(name, layout, restored=False):
    values = words(ROOT / layout['blockdata_filepath'])
    if restored or layout['secondary_tileset'] != SYMBOL:
        return values
    inverse = {target: source for source, target in REMAP[SOURCES[name]].items()}
    return tuple((v & ~0x3FF) | inverse[v & 0x3FF]
                 if v & 0x3FF >= 512 and v & 0x3FF not in (*SERVICE_IDS, *VERTICAL_BRIDGE_IDS) else v
                 for v in values)


def build(restored_maps=()):
    from PIL import Image
    from build_service_exterior import build_source

    build_source()

    registry_path = ROOT / 'data/layouts/layouts.json'
    registry = json.loads(registry_path.read_text(encoding='utf-8'))
    layouts = {name: next(x for x in registry['layouts'] if x['name'] == name + '_Layout')
               for name in SOURCES}
    blocks = {name: source_blocks(name, layout, name in restored_maps)
              for name, layout in layouts.items()}
    used = {source: set() for source in REMAP}
    for name, values in blocks.items():
        for value in values:
            tile = value & 0x3FF
            if tile >= 512 and tile not in VERTICAL_BRIDGE_IDS:
                source = SERVICE_SOURCE if tile in SERVICE_IDS else SOURCES[name]
                used[source].add(tile)
    # Include door frames' static counterparts and all scripted sanctuary props.
    used['petalburg'].update((0x248, 0x249))
    used['whisperwood'].update(range(0x318, 0x336))
    used[SERVICE_SOURCE].update(SERVICE_IDS)
    primary = Image.open(ROOT / 'data/tilesets/primary/general/tiles.png')
    images = {source: Image.open(ROOT / f'data/tilesets/secondary/{source}/tiles.png')
              for source in used}
    metas = {source: words(ROOT / f'data/tilesets/secondary/{source}/metatiles.bin')
             for source in used}

    def pixels(source, value):
        tile = value & 0x3FF
        image = primary if tile < 512 else images[source]
        tile %= 512
        return image.crop((tile % 16 * 8, tile // 16 * 8,
                           tile % 16 * 8 + 8, tile // 16 * 8 + 8)).tobytes()

    colors = {}
    for source, ids in used.items():
        for tile in sorted(ids):
            if tile not in REMAP[source]:
                raise ValueError(f'Add {source} metatile {tile:#x} to REMAP before packing')
            for value in metas[source][(tile - 512) * 8:(tile - 511) * 8]:
                slot = value >> 12
                indices = set(pixels(source, value)) - {0}
                if slot >= 6 and indices:
                    palette = source_palette(source, slot)
                    colors.setdefault((source, slot), set()).update(palette[i] for i in indices)
    palettes = {slot: source_palette(*key) for key, slot in FIXED_PALETTES.items()}
    assignment = dict(FIXED_PALETTES)
    bins = {}
    for key, needed in sorted(colors.items(), key=lambda item: (-len(item[1]), item[0])):
        if key in assignment:
            continue
        candidates = [(len(existing | needed), slot) for slot, existing in bins.items()
                      if len(existing | needed) <= 15]
        slot = min(candidates)[1] if candidates else next(
            slot for slot in range(6, 13) if slot not in palettes and slot not in bins)
        bins.setdefault(slot, set()).update(needed)
        assignment[key] = slot
    for slot, needed in bins.items():
        palettes[slot] = [(0, 0, 0), *sorted(needed)]
        palettes[slot] += [(0, 0, 0)] * (16 - len(palettes[slot]))

    bank, tile_indices = [], {}

    def add_tile(data):
        if not any(data):
            return 0
        if data not in tile_indices:
            tile_indices[data] = 512 + len(bank)
            bank.append(data)
        return tile_indices[data]

    output_meta = [0] * (0x363 - 512) * 8
    output_attrs = [0] * (0x363 - 512)
    for source, ids in used.items():
        attrs = words(ROOT / f'data/tilesets/secondary/{source}/metatile_attributes.bin')
        for tile in sorted(ids):
            dest = REMAP[source][tile] - 512
            output_attrs[dest] = attrs[tile - 512]
            for i, value in enumerate(metas[source][(tile - 512) * 8:(tile - 511) * 8]):
                data = pixels(source, value)
                slot = value >> 12
                if slot >= 6 and any(data):
                    key = source, slot
                    target_slot = assignment[key]
                    if key not in FIXED_PALETTES:
                        original = source_palette(*key)
                        lookup = {c: j for j, c in enumerate(palettes[target_slot][1:], 1)}
                        data = bytes(lookup[original[p]] if p else 0 for p in data)
                    value = (value & 0xFFF) | (target_slot << 12)
                if not any(data):
                    value = 0
                elif value & 0x3FF >= 512 or data != pixels(source, value):
                    value = (value & ~0x3FF) | add_tile(data)
                output_meta[dest * 8 + i] = value
    # Native door animation uploads use the last 16 BG tiles.
    if len(bank) > 496:
        raise ValueError('Outdoor graphics overlap the door animation tile area')
    TARGET.mkdir(parents=True, exist_ok=True)
    image = Image.new('P', (128, 256))
    image.putpalette(images['petalburg'].getpalette())
    for i, data in enumerate(bank):
        image.paste(Image.frombytes('P', (8, 8), data), (i % 16 * 8, i // 16 * 8))
    image.save(TARGET / 'tiles.png')
    (TARGET / 'metatiles.bin').write_bytes(struct.pack(f'<{len(output_meta)}H', *output_meta))
    (TARGET / 'metatile_attributes.bin').write_bytes(struct.pack(f'<{len(output_attrs)}H', *output_attrs))
    (TARGET / 'palettes').mkdir(exist_ok=True)
    for slot in range(16):
        palette = palettes.get(slot, [(0, 0, 0)] * 16)
        text = 'JASC-PAL\n0100\n16\n' + '\n'.join(' '.join(map(str, c)) for c in palette) + '\n'
        (TARGET / f'palettes/{slot:02}.pal').write_text(text, encoding='ascii')
    for name, layout in layouts.items():
        values = [(v & ~0x3FF) | REMAP[SOURCES[name]][v & 0x3FF]
                  if v & 0x3FF >= 512 and v & 0x3FF not in (*SERVICE_IDS, *VERTICAL_BRIDGE_IDS) else v
                  for v in blocks[name]]
        (ROOT / layout['blockdata_filepath']).write_bytes(struct.pack(f'<{len(values)}H', *values))
        layout['secondary_tileset'] = SYMBOL
    registry_path.write_text(json.dumps(registry, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
    from build_vertical_bridge import build as build_vertical_bridge
    build_vertical_bridge()
    print(f'Packed Virelia outdoors: {len(bank)} tiles, {len(palettes)} secondary palettes')


if __name__ == '__main__':
    build()
