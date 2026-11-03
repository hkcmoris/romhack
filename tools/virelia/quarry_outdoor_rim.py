"""Cluster windbreak trees and fallen blocks on the unworked quarry rim."""


def pack_rim(add, graphic, ids, meta, general, primary_pixels, bank):
    # Reuse the silhouettes of General's trees, including the flipped right
    # halves. Remove the grass pixels baked into their trunks before layering
    # the trees over slate. Palette 10 gives these upland trees a sage ramp.
    colors = [(0, 0, 0), (176, 192, 136), (128, 160, 112), (88, 120, 88),
              (56, 80, 72), (152, 128, 96), (96, 88, 80), (112, 88, 72),
              (64, 64, 64), (168, 144, 104), (184, 152, 112), (144, 120, 96)]
    colors += [(0, 0, 0)] * (16 - len(colors))
    (bank / 'palettes/10.pal').write_text('JASC-PAL\n0100\n16\n' +
        ''.join(f'{r} {g} {b}\n' for r, g, b in colors), newline='\n')
    offset = (ids['ROCK_BED_0'] - 512) * 8
    stone = meta[offset:offset + 4]

    for shape, sources in [('FIR', (0x01e, 0x01f, 0x026, 0x027)), ('SAPLING', (0x00e, 0x016))]:
        for part, source in enumerate(sources):
            entries = general[source * 8:source * 8 + 8]
            # The canopy is stored on the upper layer; the solid trunk is on
            # the lower layer. Its palette indices 12-15 belong to grass.
            tree = entries[4:] if any(entries[4:]) else entries[:4]
            overlay = []
            for entry in tree:
                tid = entry & 1023
                data = [primary_pixels[tid // 16 * 8 + y][tid % 16 * 8 + x]
                        for y in range(8) for x in range(8)]
                # General's static tree graphics can be referenced directly
                # unless we actually remove grass from this quarter.
                packed = tid if max(data) < 12 else graphic(value if value < 12 else 0 for value in data)
                overlay.append(0xa000 | (packed ^ (entry & 0xc00)))
            add(f'RIM_{shape}_{part}', stone + overlay)
    boulder = general[0xe0 * 8 + 4:0xe1 * 8]
    add('RIM_TALUS', stone + [0x6000 | (entry & 0xfff) for entry in boulder])


def plant_rim(grid, ids, sites):
    bed = {ids[f'ROCK_BED_{i}'] for i in range(8)}
    for x, y, kind in sites:
        width, height = {'FIR': (2, 2), 'SAPLING': (1, 2), 'TALUS': (1, 1)}[kind]
        cells = [(x + dx, y + dy) for dy in range(height) for dx in range(width)]
        if all(grid[yy][xx] & 1023 in bed for xx, yy in cells):
            for part, (xx, yy) in enumerate(cells):
                key = 'RIM_TALUS' if kind == 'TALUS' else f'RIM_{kind}_{part}'
                grid[yy][xx] = (grid[yy][xx] & ~1023) | ids[key]
