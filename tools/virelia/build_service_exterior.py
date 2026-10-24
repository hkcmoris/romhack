#!/usr/bin/env python3
"""Pack the reusable combined Center/Mart exterior and place it in Slateford.

The source bank feeds the shared outdoor packer. It uses General's original
building colors and Rustboro paving; the entrance keeps the native door IDs.
"""
from pathlib import Path
import json
import struct

ROOT = Path(__file__).resolve().parents[2]
FIRST, WIDTH, HEIGHT = 0x340, 7, 5


def build_source():
    from PIL import Image
    from service_exterior_art import build_exterior

    cells = build_exterior()
    assert len(cells) == HEIGHT and all(len(row) == WIDTH for row in cells)
    bank, indices = [], {}
    raw = bytearray((FIRST + WIDTH * HEIGHT - 512) * 16)
    for y, row in enumerate(cells):
        for x, pieces in enumerate(row):
            assert len(pieces) == 8
            values = []
            for pixels, palette in pieces:
                pixels = bytes(pixels)
                assert len(pixels) == 64 and max(pixels) < 16 and palette in (0, 1, 2, 8)
                if not any(pixels):
                    values.append(0)
                    continue
                if pixels not in indices:
                    indices[pixels] = 512 + len(bank)
                    bank.append(pixels)
                values.append((palette << 12) | indices[pixels])
            struct.pack_into('<8H', raw, (FIRST - 512 + y * WIDTH + x) * 16, *values)
    assert len(bank) <= 496
    target = ROOT / 'data/tilesets/secondary/virelia_services'
    target.mkdir(parents=True, exist_ok=True)
    tiles = Image.new('P', (128, 256))
    tiles.putpalette(Image.open(ROOT / 'data/tilesets/primary/general/tiles.png').getpalette())
    for i, pixels in enumerate(bank):
        tiles.paste(Image.frombytes('P', (8, 8), pixels), (i % 16 * 8, i // 16 * 8))
    tiles.save(target / 'tiles.png')
    (target / 'metatiles.bin').write_bytes(raw)
    (target / 'metatile_attributes.bin').write_bytes(bytes(len(raw) // 8))
    (target / 'palettes').mkdir(exist_ok=True)
    paving = ROOT / 'data/tilesets/secondary/rustboro/palettes/08.pal'
    (target / 'palettes/08.pal').write_bytes(paving.read_bytes())
    print(f'Packed combined Center/Mart exterior: {len(bank)} graphics tiles')


def stamp(grid):
    """Update the two service plots without repainting the authored city."""
    # Both plots use the city's existing pale paving.
    for y in range(17, 21):
        for x in range(5, 9):
            grid[y][x] = 0x32BB
    for y in range(HEIGHT):
        for x in range(WIDTH):
            grid[6 + y][3 + x] = 0x0400 | (FIRST + y * WIDTH + x)
    grid[9][6] = 0x0459
    grid[10][6] = 0x0461


def configure_city():
    path = ROOT / 'data/maps/Virelia_SlatefordCity/map.json'
    city = json.loads(path.read_text(encoding='utf-8'))
    city['warp_events'] = [warp for warp in city['warp_events']
                           if warp['dest_map'] != 'MAP_VIRELIA_SLATEFORD_MART']
    for sign in city['bg_events']:
        if sign['script'] in ('Virelia_SlatefordCity_EventScript_OfficeSign',
                              'Virelia_SlatefordCity_EventScript_ServiceSign'):
            sign.update(x=8, y=10, script='Virelia_SlatefordCity_EventScript_ServiceSign')
    path.write_text(json.dumps(city, ensure_ascii=False, indent=2) + '\n',
                    encoding='utf-8', newline='\n')


def main():
    from build_outdoor_tileset import build, source_blocks

    registry = json.loads((ROOT / 'data/layouts/layouts.json').read_text())['layouts']
    layout = next(x for x in registry if x['name'] == 'Virelia_SlatefordCity_Layout')
    values = source_blocks('Virelia_SlatefordCity', layout)
    grid = [list(values[y * 30:(y + 1) * 30]) for y in range(30)]
    stamp(grid)
    path = ROOT / layout['blockdata_filepath']
    path.write_bytes(struct.pack('<900H', *(v for row in grid for v in row)))
    configure_city()
    build(restored_maps={'Virelia_SlatefordCity'})


if __name__ == '__main__':
    main()
