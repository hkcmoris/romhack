#!/usr/bin/env python3
"""Pack approved image_gen Fire-family atlases into native GBA sprite assets.

This is a deterministic export step, not an art generator: it crops the four
atlas front/icon views and a separate corrected back view, converts alpha to
the GBA transparent index, uniformly downsizes
with nearest-neighbor sampling, and maps colors to the declared 16-color
palettes. Original Ekans and Arbok assets are never changed.

The authoring atlases and prompts live under outputs/virelia/fire-sprites/.
They are local working material; the exported game assets are self-contained.
Requires Pillow. Run from any directory, with --source-dir for another atlas
folder. Each species requires <name>-final.png and <name>-back-corrected.png.
The atlas back cell is deliberately unused. A diagnostic preview and exact
native bounds are written to --review-dir.
"""

import argparse
import hashlib
import json
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parents[2]
ALPHA_THRESHOLD = 240
NORMAL_HEX = (
    '000000', '201828', '382038', '582840', '783860', 'A05080',
    'C87898', '784830', 'B87840', 'E8B868', 'F8E0A0', '902830',
    'D84020', 'F87828', 'F8C840', 'F8F0C8',
)
NORMAL = [tuple(bytes.fromhex(color)) for color in NORMAL_HEX]
SHINY = list(NORMAL)
SHINY[1:7] = [tuple(bytes.fromhex(color)) for color in (
    '181818', '202830', '384048', '505058', '707078', '98A0A0',
)]
ICON_MAP = (0, 15, 15, 14, 11, 11, 12, 6, 6, 7, 3, 10, 10, 13, 9, 3)
SPECS = {
    'ekans': {'label': 'Virelian Ekans', 'directory': 'ekans/virelia', 'front_fit': (52, 52), 'front_bottom': 60},
    'arbok': {'label': 'Virelian Arbok', 'directory': 'arbok/virelia', 'front_fit': (60, 60), 'front_bottom': 62},
    'infernabra': {'label': 'Infernabra', 'directory': 'infernabra', 'front_fit': (62, 62), 'front_bottom': 63},
}
REGIONS = (
    (0, 0, 512, 640), (512, 0, 1024, 640), (1024, 0, 1536, 640),
    (0, 640, 512, 1024), (512, 640, 1024, 1024),
)


def read_palette(path):
    lines = path.read_text(encoding='utf-8').splitlines()
    assert lines[:3] == ['JASC-PAL', '0100', '16'], path
    colors = [tuple(map(int, line.split())) for line in lines[3:19]]
    assert len(colors) == 16 and all(len(color) == 3 for color in colors)
    return colors


def write_palette(path, colors):
    text = 'JASC-PAL\n0100\n16\n' + ''.join(' '.join(map(str, color)) + '\n' for color in colors)
    path.write_text(text, encoding='ascii', newline='\n')


def indexed_image(size, colors=NORMAL):
    image = Image.new('P', size, 0)
    image.putpalette([channel for color in colors for channel in color])
    image.info['transparency'] = 0
    return image


def trim_view(atlas, region):
    view = atlas.crop(region)
    mask = view.getchannel('A').point(lambda alpha: 255 if alpha >= ALPHA_THRESHOLD else 0)
    box = mask.getbbox()
    assert box, f'Empty atlas region: {region}'
    # Convert alpha once, so no diffuse glow becomes part of the native sprite.
    view.putalpha(mask)
    return view.crop(box), box


def quantize(view):
    """Map opaque pixels to indices 1..15; index 0 is exclusively transparency."""
    result = indexed_image(view.size)
    indices = []
    cache = {}
    opaque = []
    for pixel in view.get_flattened_data():
        if pixel[3] == 0:
            indices.append(0)
            opaque.append(False)
            continue
        rgb = pixel[:3]
        if rgb not in cache:
            cache[rgb] = min(range(1, 16), key=lambda index: sum(
                (a - b) ** 2 for a, b in zip(rgb, NORMAL[index])))
        indices.append(cache[rgb])
        opaque.append(True)
    assert all((index != 0) == visible for index, visible in zip(indices, opaque))
    result.putdata(indices)
    return result


def place_views(views, canvas_size, fit, bottoms):
    """Keep one scale across an animation pair, preserving the source aspect."""
    scale = min(fit[0] / max(view.width for view in views),
                fit[1] / max(view.height for view in views))
    results = []
    for view, bottom in zip(views, bottoms):
        size = (max(1, round(view.width * scale)), max(1, round(view.height * scale)))
        assert size[0] <= fit[0] and size[1] <= fit[1]
        resized = view.resize(size, Image.Resampling.NEAREST)
        packed = quantize(resized)
        canvas = indexed_image(canvas_size)
        position = ((canvas_size[0] - packed.width) // 2, bottom - packed.height)
        assert position[0] >= 0 and position[1] >= 0
        canvas.paste(packed, position)
        results.append(canvas)
    return results


def stack_frames(frames, colors=NORMAL):
    sheet = indexed_image((frames[0].width, sum(frame.height for frame in frames)), colors)
    y = 0
    for frame in frames:
        sheet.paste(frame, (0, y))
        y += frame.height
    return sheet


def remap_icon(frame, colors):
    icon = indexed_image(frame.size, colors)
    icon.putdata([ICON_MAP[index] for index in frame.get_flattened_data()])
    return icon


def image_bounds(image):
    mask = Image.new('L', image.size)
    mask.putdata([255 if index else 0 for index in image.get_flattened_data()])
    return mask.getbbox()


def native_bounds(frames):
    boxes = [image_bounds(frame) for frame in frames]
    assert all(boxes)
    union = (min(box[0] for box in boxes), min(box[1] for box in boxes),
             max(box[2] for box in boxes), max(box[3] for box in boxes))
    rounded = (((union[2] - union[0] + 7) // 8) * 8,
               ((union[3] - union[1] + 7) // 8) * 8)
    return {'frame_bboxes': boxes, 'bbox_union': union,
            'coords_size': rounded, 'y_offset': 64 - union[3]}


def save_and_verify(image, path, colors):
    image.save(path, bits=4, transparency=0, optimize=False)
    packed = path.read_bytes()
    assert packed[:8] == b'\x89PNG\r\n\x1a\n' and packed[24] == 4 and packed[25] == 3, path
    loaded = Image.open(path)
    assert loaded.mode == 'P' and loaded.size == image.size and loaded.info['transparency'] == 0, path
    assert loaded.tobytes() == image.tobytes(), path
    assert max(loaded.get_flattened_data()) < 16, path
    assert loaded.getpalette()[:48] == [channel for color in colors for channel in color], path


def preview(assets, path, icon_colors):
    scale = 3
    column_width = 200
    row_height = 232
    margin = 18
    labels = ['Front 1', 'Front 2', 'Back', 'Shiny front', 'Shiny back', 'Party icons']
    canvas = Image.new('RGB', (margin * 2 + column_width * len(labels), 48 + row_height * len(assets)), '#e9e5de')
    draw = ImageDraw.Draw(canvas)
    font = ImageFont.load_default(size=18)
    title_font = ImageFont.load_default(size=20)
    for index, label in enumerate(labels):
        draw.text((margin + index * column_width, 12), label, font=font, fill='#27242b')
    for row, (name, frames) in enumerate(assets.items()):
        y = 48 + row * row_height
        draw.text((margin, y), SPECS[name]['label'], font=title_font, fill='#27242b')
        shiny_front = frames['front.png'].copy()
        shiny_back = frames['back.png'].copy()
        for image in (shiny_front, shiny_back):
            image.putpalette([channel for color in SHINY for channel in color])
        displays = [frames['front.png'], frames['front_2.png'], frames['back.png'],
                    shiny_front, shiny_back, frames['icon.png']]
        for column, image in enumerate(displays):
            rgba = image.convert('RGBA').resize((image.width * scale, image.height * scale), Image.Resampling.NEAREST)
            x = margin + column * column_width + (192 - rgba.width) // 2
            canvas.paste(rgba, (x, y + 30), rgba)
    canvas.save(path)


def export(source_dir, review_dir):
    # Require every standalone back before any output is changed.
    required = [source_dir / f'{name}{suffix}.png'
                for name in SPECS for suffix in ('-final', '-back-corrected')]
    missing = [str(path) for path in required if not path.is_file()]
    if missing:
        raise FileNotFoundError('Missing required sprite sources: ' + ', '.join(missing))
    icon_colors = read_palette(ROOT / 'graphics/pokemon/icon_palettes/pal5.pal')
    review_dir.mkdir(parents=True, exist_ok=True)
    metadata = {'source': 'image_gen built-in; approved atlases', 'alpha_threshold': ALPHA_THRESHOLD,
                'resampling': 'nearest-neighbor, uniform scale per animation pair',
                'icon_palette_index': 5, 'species': {}}
    all_assets = {}
    for name, spec in SPECS.items():
        source = source_dir / f'{name}-final.png'
        atlas = Image.open(source).convert('RGBA')
        assert atlas.size == (1536, 1024), (source, atlas.size)
        trimmed = {index: trim_view(atlas, REGIONS[index]) for index in (0, 1, 3, 4)}
        views = {index: entry[0] for index, entry in trimmed.items()}
        front = place_views([views[0], views[1]], (64, 64), spec['front_fit'], [spec['front_bottom']] * 2)
        back_source = source_dir / f'{name}-back-corrected.png'
        back_art = Image.open(back_source).convert('RGBA')
        back_view, back_source_bbox = trim_view(back_art, (0, 0, *back_art.size))
        back = place_views([back_view], (64, 64), (60, 60), [63])[0]
        icons = place_views([views[3], views[4]], (32, 32), (28, 28), [30, 29])
        icons = [remap_icon(frame, icon_colors) for frame in icons]
        assert front[0].tobytes() != front[1].tobytes(), f'{name}: identical front frames'
        assert icons[0].tobytes() != icons[1].tobytes(), f'{name}: identical icon frames'
        frames = {'front.png': front[0], 'front_2.png': front[1],
                  'anim_front.png': stack_frames(front), 'back.png': back,
                  'icon.png': stack_frames(icons, icon_colors)}
        directory = ROOT / 'graphics/pokemon' / spec['directory']
        directory.mkdir(parents=True, exist_ok=True)
        for filename, image in frames.items():
            save_and_verify(image, directory / filename, icon_colors if filename == 'icon.png' else NORMAL)
        write_palette(directory / 'normal.pal', NORMAL)
        write_palette(directory / 'shiny.pal', SHINY)
        all_assets[name] = frames
        metadata['species'][name] = {
            'directory': directory.relative_to(ROOT).as_posix(),
            'source_sha256': hashlib.sha256(source.read_bytes()).hexdigest(),
            'atlas_regions_used': [0, 1, 3, 4],
            'back_source': back_source.name,
            'back_source_sha256': hashlib.sha256(back_source.read_bytes()).hexdigest(),
            'back_source_size': back_art.size, 'back_source_bbox': back_source_bbox,
            'source_regions': REGIONS, 'source_bboxes_in_regions': [trimmed[index][1] if index in trimmed else None
                                                                          for index in range(len(REGIONS))],
            'front': native_bounds(front), 'back': native_bounds([back]),
            'icon_bboxes': [image_bounds(icon) for icon in icons],
            'front_frames_distinct': True, 'icon_frames_distinct': True,
            'png_bit_depth': 4, 'palette_entries': 16,
        }
    preview(all_assets, review_dir / 'preview.png', icon_colors)
    (review_dir / 'metadata.json').write_text(json.dumps(metadata, indent=2) + '\n', encoding='utf-8', newline='\n')
    print(json.dumps(metadata, indent=2))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source-dir', type=Path, default=ROOT / 'outputs/virelia/fire-sprites/sources')
    parser.add_argument('--review-dir', type=Path, default=ROOT / 'outputs/virelia/fire-sprites')
    args = parser.parse_args()
    export(args.source_dir, args.review_dir)


if __name__ == '__main__':
    main()
