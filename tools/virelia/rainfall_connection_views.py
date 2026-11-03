"""Check tile ownership where Emerald scrolls between the chapter's outdoor banks."""


def visible_neighbor_cells(src, dst, direction, offset, cameras):
    """Project the scrolling camera's buffered view into the adjacent blockmap."""
    origin_x, origin_y = {
        'up': (offset, -dst['height']), 'down': (offset, src['height']),
        'left': (-dst['width'], offset), 'right': (src['width'], offset),
    }[direction]
    visible = set()
    for x, y in cameras:
        # MAP_OFFSET is seven. The east buffer includes an eighth column
        # for the partially visible metatile while the camera scrolls.
        left = max(0, x - 7 - origin_x)
        right = min(dst['width'], x + 9 - origin_x)
        top = max(0, y - 7 - origin_y)
        bottom = min(dst['height'], y + 8 - origin_y)
        if left < right and top < bottom:
            visible.update((tx, ty) for ty in range(top, bottom) for tx in range(left, right))
    return visible


def validate_connection_views(world, reached):
    checked = 0
    for name, entry in world.maps.items():
        source = world.layouts[name]
        for connection in entry.get('connections') or ():
            target = world.ids.get(connection['map'])
            if target is None:
                continue
            dest = world.layouts[target]
            if source['secondary_tileset'] == dest['secondary_tileset']:
                continue
            if source['primary_tileset'] != dest['primary_tileset']:
                raise ValueError(f'{name} and {target} have different primary graphics')
            cameras = ((x, y) for m, x, y in reached if m == name)
            cells = visible_neighbor_cells(source, dest, connection['direction'], int(connection['offset']), cameras)
            # The old BG entries survive the tileset swap. An identical-looking
            # tile with a different graphic ID can still flash for a frame.
            for x, y in sorted(cells):
                tile = world.blocks[target][y * dest['width'] + x] & 1023
                if tile >= 512:
                    raise ValueError(f'{name} camera exposes {target} ({x},{y}) secondary tile {tile:#05x}')
                parts = world.metatiles[dest['primary_tileset']][tile*8:(tile+1)*8]
                if any((p & 1023) >= 512 or (p & 1023 and p >> 12 >= 6) for p in parts):
                    raise ValueError(f'{target} ({x},{y}): primary tile {tile:#05x} depends on secondary graphics')
            checked += len(cells)
    print(f'PASS: {checked} neighboring blocks visible across different outdoor tilesets use shared graphics')
