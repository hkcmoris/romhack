#!/usr/bin/env python3
"""Render the quarry's actual indexed tiles/metatiles for art review (Pillow)."""
from functools import lru_cache
import json
from pathlib import Path
import struct

from PIL import Image, ImageDraw

ROOT = Path(__file__).resolve().parents[2]
OUTPUT = ROOT / 'outputs/quarry'


def words(path):
    data = path.read_bytes()
    return struct.unpack(f'<{len(data) // 2}H', data)


@lru_cache(None)
def metatile(source, index):
    folder = ROOT / f'data/tilesets/secondary/{source}'
    primary = ROOT / 'data/tilesets/primary/general'
    values = words(folder / 'metatiles.bin')
    image = Image.new('RGB', (16, 16))
    for n, value in enumerate(values[(index - 512) * 8:(index - 511) * 8]):
        tile, palette = value & 1023, value >> 12
        bank = primary if tile < 512 else folder
        tile %= 512
        pixels = Image.open(bank / 'tiles.png')
        pal_bank = primary if palette < 6 else folder
        colors = [tuple(map(int, line.split())) for line in
                  (pal_bank / f'palettes/{palette:02}.pal').read_text().splitlines()[3:19]]
        for y in range(8):
            for x in range(8):
                sx, sy = (7 - x if value & 1024 else x), (7 - y if value & 2048 else y)
                p = pixels.getpixel((tile % 16 * 8 + sx, tile // 16 * 8 + sy))
                # Index zero is transparent on BOTH GBA background layers.
                # Painting its palette RGB here hid holes visible in-game.
                if p:
                    image.putpixel(((n % 4 % 2) * 8 + x, (n % 4 // 2) * 8 + y), colors[p])
    return image


def render(source='virelia_quarry', label=''):
    OUTPUT.mkdir(parents=True, exist_ok=True)
    layouts = json.loads((ROOT / 'data/layouts/layouts.json').read_text())['layouts']
    for suffix in ('Entrance', 'Depths', 'Gallery'):
        name = 'Virelia_Slateford_Quarry' + suffix
        layout = next(item for item in layouts if item['name'] == name + '_Layout')
        image = Image.new('RGB', (layout['width'] * 16, layout['height'] * 16))
        for i, cell in enumerate(words(ROOT / layout['blockdata_filepath'])):
            image.paste(metatile(source, cell & 1023), (i % layout['width'] * 16, i // layout['width'] * 16))
        if source == 'virelia_quarry':
            entry = json.loads((ROOT / f'data/maps/{name}/map.json').read_text())
            for obj in sorted(entry['object_events'], key=lambda obj: obj['y']):
                gfx = obj['graphics_id']
                palette = 'moving_box'
                size = 16
                if gfx.startswith('OBJ_EVENT_GFX_QUARRY_'):
                    path = 'graphics/object_events/pics/quarry/' + gfx.removeprefix('OBJ_EVENT_GFX_QUARRY_').lower() + '.png'
                elif gfx in ('OBJ_EVENT_GFX_HIKER', 'OBJ_EVENT_GFX_WOMAN_2', 'OBJ_EVENT_GFX_ITEM_BALL'):
                    kind, palette = {'OBJ_EVENT_GFX_HIKER': ('people/hiker', 'npc_1'),
                                     'OBJ_EVENT_GFX_WOMAN_2': ('people/woman_2', 'npc_3'),
                                     'OBJ_EVENT_GFX_ITEM_BALL': ('misc/ball_poke', 'npc_3')}[gfx]
                    path = 'graphics/object_events/pics/' + kind + '.png'
                    size = 32
                else:
                    path = 'graphics/pokemon/sableye/overworld.png'
                    palette = None
                    size = 32
                pal_path = (ROOT / f'graphics/object_events/palettes/{palette}.pal' if palette else
                            ROOT / 'graphics/pokemon/sableye/overworld_normal.pal')
                colors = [tuple(map(int, line.split())) for line in pal_path.read_text().splitlines()[3:19]]
                sprite = Image.open(ROOT / path).crop((0, 0, size if palette is None else 16, size))
                sprite.putpalette([channel for color in colors for channel in color] + [0] * (768 - 48))
                mask = Image.frombytes('L', sprite.size, bytes(255 if p else 0 for p in sprite.tobytes()))
                image.paste(sprite.convert('RGB'), (obj['x'] * 16 - (sprite.width - 16) // 2,
                                                   obj['y'] * 16 + 16 - size), mask)
        image.resize((image.width * 3, image.height * 3), Image.Resampling.NEAREST).save(OUTPUT / f'{suffix.lower()}{label}.png')
    count = len(words(ROOT / f'data/tilesets/secondary/{source}/metatile_attributes.bin'))
    atlas = Image.new('RGB', (16 * 64, ((count + 15) // 16) * 64), '#27232b')
    draw = ImageDraw.Draw(atlas)
    for i in range(count):
        x, y = i % 16 * 64, i // 16 * 64
        atlas.paste(metatile(source, 512 + i).resize((48, 48), Image.Resampling.NEAREST), (x, y))
        draw.text((x, y + 48), f'{512 + i:03x}', fill='white')
    atlas.save(OUTPUT / f'atlas{label}.png')


if __name__ == '__main__':
    import sys
    render(*sys.argv[1:])
