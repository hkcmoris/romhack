#!/usr/bin/env python3
"""Pack approved image_gen Water-family atlases into native GBA sprite assets.

This is deterministic technical packing: crop the approved atlas regions,
convert alpha to the GBA transparent index, uniformly downsize with nearest-
neighbor sampling, and map colors to shared 16-color palettes. Original
Spinarak and Ariados assets are never changed.

Each atlas supplies two front frames and two party-icon frames. Its original
back cell is unused; a standalone <name>-back-corrected.png supplies the approved
cropped upper-body rear view. Source dimensions and crop regions in SOURCE_LAYOUTS
were measured from the generated atlases.

Requires Pillow. Run from any directory. Source atlases default to
outputs/virelia/water-sprites/sources. A diagnostic preview and exact native
bounds are written to --review-dir.
"""

import argparse
import colorsys
import hashlib
import json
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parents[2]
ALPHA_THRESHOLD = 240
NORMAL_HEX = (
    '000000', '182030', '183850', '205880', '3080B0', '58B0D8',
    '287068', '40A888', '80D0B8', '906028', 'D0A040', 'F0D068',
    'F8F0C8', 'C84050', '90E0F0', 'F8F8F0',
)
NORMAL = [tuple(bytes.fromhex(color)) for color in NORMAL_HEX]
SHINY = list(NORMAL)
SHINY[2:9] = [tuple(bytes.fromhex(color)) for color in (
    '402048', '783068', 'B85090', 'E080B8', '783048', 'B85080', 'E890B0',
)]
# Bubble blues remain blue in shiny sprites; body pixels cannot use these slots.
SHINY[5] = NORMAL[5]
BUBBLE_INDICES = (1, 5, 14, 15)
BODY_INDICES = tuple(index for index in range(1, 16) if index not in (5, 14))
assert all(NORMAL[index] == SHINY[index] for index in BUBBLE_INDICES)
ICON_MAP = (0, 15, 4, 4, 5, 6, 4, 5, 6, 9, 9, 10, 11, 8, 6, 3)
SPECS = {
    'spinarak': {'label': 'Virelian Spinarak', 'directory': 'spinarak/virelia', 'front_fit': (48, 44), 'front_bottom': 58},
    'ariados': {'label': 'Virelian Ariados', 'directory': 'ariados/virelia', 'front_fit': (60, 56), 'front_bottom': 61},
    'arachnaut': {'label': 'Arachnaut', 'directory': 'arachnaut', 'front_fit': (62, 62), 'front_bottom': 63},
}
# Atlas region 2 is retained for provenance but deliberately unused.
SOURCE_LAYOUTS = {
    'spinarak': {'size': (1536, 1024), 'regions': (
        (0, 0, 512, 640), (512, 0, 1024, 640), (1024, 0, 1536, 640),
        (0, 640, 512, 1024), (512, 640, 1024, 1024))},
    'ariados': {'size': (1536, 1024), 'regions': (
        (0, 0, 512, 710), (512, 0, 1024, 710), (1024, 0, 1536, 710),
        (0, 710, 512, 1024), (512, 710, 1024, 1024))},
    'arachnaut': {'size': (1536, 1024), 'regions': (
        (0, 0, 550, 680), (550, 0, 1100, 680), (1100, 0, 1536, 680),
        (0, 680, 512, 1024), (512, 680, 1024, 1024))},
}

# Measured source-space bounds isolate bubble clusters from cyan-colored limbs.
# Classification changes palette indices only; source pixels/alpha are untouched.
BUBBLE_REGIONS = {
    'spinarak': {
        'front': ((112, 215, 243, 356), (240, 229, 302, 282)),
        'front_2': ((642, 213, 793, 341), (809, 254, 869, 309)),
        'back': ((245, 631, 790, 987),),
    },
    'ariados': {
        'front': ((265, 278, 366, 371), (378, 322, 461, 403), (447, 394, 497, 451)),
        'front_2': ((706, 280, 859, 388), (884, 328, 970, 416), (943, 399, 994, 454)),
        'back': ((240, 791, 379, 947), (351, 689, 627, 977),
                 (547, 614, 767, 822), (653, 785, 851, 970)),
    },
    'arachnaut': {
        'front': ((243, 151, 418, 294), (200, 235, 280, 317),
                  (301, 239, 382, 321), (413, 243, 497, 336)),
        'front_2': ((824, 172, 977, 313), (787, 253, 859, 329), (966, 276, 1054, 360)),
        'back': ((310, 659, 785, 1023), (184, 844, 364, 1046), (684, 850, 934, 1086)),
    },
}


def bubble_mask(source, regions):
    """Select existing cyan bubble colors within measured regions, never geometry."""
    mask = Image.new('L', source.size)
    pixels, selected = source.load(), mask.load()
    for left, top, right, bottom in regions:
        for y in range(top, bottom):
            for x in range(left, right):
                red, green, blue, alpha = pixels[x, y]
                if alpha < ALPHA_THRESHOLD:
                    continue
                hue, saturation, value = colorsys.rgb_to_hsv(red / 255, green / 255, blue / 255)
                cyan = 195 <= hue * 360 <= 212 and saturation >= 0.15
                neutral = value <= 0.15 or (value >= 0.75 and saturation < 0.15)
                if cyan or neutral:
                    selected[x, y] = 255
    return mask


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


def quantize(view, bubbles=None):
    """Map opaque pixels to indices 1..15; index 0 is exclusively transparency."""
    result = indexed_image(view.size)
    indices = []
    cache = {}
    opaque = []
    bubble_pixels = bubbles.tobytes() if bubbles is not None else None
    for position, pixel in enumerate(view.get_flattened_data()):
        if pixel[3] == 0:
            indices.append(0)
            opaque.append(False)
            continue
        rgb = pixel[:3]
        # Party icons retain their original unrestricted quantization and mapping.
        candidates = range(1, 16) if bubble_pixels is None else (
            BUBBLE_INDICES if bubble_pixels[position] else BODY_INDICES)
        key = (rgb, tuple(candidates))
        if key not in cache:
            cache[key] = min(candidates, key=lambda index: sum(
                (a - b) ** 2 for a, b in zip(rgb, NORMAL[index])))
        indices.append(cache[key])
        opaque.append(True)
    assert all((index != 0) == visible for index, visible in zip(indices, opaque))
    if bubble_pixels is not None:
        assert any(index in (5, 14) for index, selected in zip(indices, bubble_pixels) if selected), "No native blue bubble pixels selected"
        assert all(index in BUBBLE_INDICES for index, selected in zip(indices, bubble_pixels) if selected and index)
        assert all(index not in (5, 14) for index, selected in zip(indices, bubble_pixels) if not selected)
    result.putdata(indices)
    return result


def place_views(views, canvas_size, fit, bottoms, bubble_masks=None):
    """Keep one scale across an animation pair, preserving the source aspect."""
    scale = min(fit[0] / max(view.width for view in views),
                fit[1] / max(view.height for view in views))
    results = []
    for frame_index, (view, bottom) in enumerate(zip(views, bottoms)):
        size = (max(1, round(view.width * scale)), max(1, round(view.height * scale)))
        assert size[0] <= fit[0] and size[1] <= fit[1]
        resized = view.resize(size, Image.Resampling.NEAREST)
        bubbles = bubble_masks[frame_index].resize(size, Image.Resampling.NEAREST) if bubble_masks is not None else None
        packed = quantize(resized, bubbles)
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


def preview(assets, path):
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
    # Validate every input/layout before any output is changed.
    missing_layouts = [name for name in SPECS if name not in SOURCE_LAYOUTS]
    if missing_layouts:
        raise ValueError('Source layouts require inspection: ' + ', '.join(missing_layouts))
    sources = {}
    for name in SPECS:
        path = source_dir / f'{name}-final.png'
        atlas = Image.open(path).convert('RGBA')
        layout = SOURCE_LAYOUTS[name]
        assert atlas.size == tuple(layout['size']), (path, atlas.size, layout['size'])
        regions = layout['regions']
        assert len(regions) == 5, (name, regions)
        for left, top, right, bottom in regions:
            assert 0 <= left < right <= atlas.width and 0 <= top < bottom <= atlas.height
        trimmed = {index: trim_view(atlas, regions[index]) for index in (0, 1, 3, 4)}
        back_path = source_dir / f'{name}-back-corrected.png'
        back_art = Image.open(back_path).convert('RGBA')
        back_view, back_source_bbox = trim_view(back_art, (0, 0, *back_art.size))
        front_masks = [bubble_mask(atlas, BUBBLE_REGIONS[name][view]).crop(regions[index]).crop(trimmed[index][1])
                       for index, view in enumerate(('front', 'front_2'))]
        back_mask = bubble_mask(back_art, BUBBLE_REGIONS[name]['back']).crop(back_source_bbox)
        sources[name] = (path, atlas.size, regions, trimmed, back_path, back_art.size, back_view, back_source_bbox, front_masks, back_mask)
    icon_colors = read_palette(ROOT / 'graphics/pokemon/icon_palettes/pal3.pal')
    review_dir.mkdir(parents=True, exist_ok=True)
    metadata = {'source': 'image_gen built-in; approved Water-family atlases',
                'alpha_threshold': ALPHA_THRESHOLD,
                'resampling': 'nearest-neighbor, uniform scale per animation pair',
                'icon_palette_index': 3, 'fixed_bubble_indices': BUBBLE_INDICES,
                'bubble_hue_degrees': [195, 212], 'species': {}}
    all_assets = {}
    for name, spec in SPECS.items():
        source, source_size, regions, trimmed, back_source, back_size, back_view, back_source_bbox, front_masks, back_mask = sources[name]
        views = {index: entry[0] for index, entry in trimmed.items()}
        front = place_views([views[0], views[1]], (64, 64), spec['front_fit'], [spec['front_bottom']] * 2, front_masks)
        back = place_views([back_view], (64, 64), (60, 60), [64], [back_mask])[0]
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
            'source_file': source.name, 'source_size': source_size,
            'source_sha256': hashlib.sha256(source.read_bytes()).hexdigest(),
            'atlas_regions_used': [0, 1, 3, 4],
            'back_source': back_source.name,
            'back_source_sha256': hashlib.sha256(back_source.read_bytes()).hexdigest(),
            'back_source_size': back_size, 'back_source_bbox': back_source_bbox,
            'source_regions': regions,
            'source_bboxes_in_regions': [trimmed[index][1] if index in trimmed else None
                                        for index in range(len(regions))],
            'front': native_bounds(front), 'back': native_bounds([back]),
            'bubble_regions': BUBBLE_REGIONS[name],
            'blue_bubble_pixels': {key: sum(index in (5, 14) for index in frames[key].tobytes())
                                   for key in ('front.png', 'front_2.png', 'back.png')},
            'bubble_palette_invariant': 'Selected bubble pixels use identical normal/shiny palette entries',
            'icon_bboxes': [image_bounds(icon) for icon in icons],
            'front_frames_distinct': True, 'icon_frames_distinct': True,
            'png_bit_depth': 4, 'palette_entries': 16,
        }
    preview(all_assets, review_dir / 'preview.png')
    (review_dir / 'metadata.json').write_text(json.dumps(metadata, indent=2) + '\n', encoding='utf-8', newline='\n')
    print(json.dumps(metadata, indent=2))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source-dir', type=Path, default=ROOT / 'outputs/virelia/water-sprites/sources')
    parser.add_argument('--review-dir', type=Path, default=ROOT / 'outputs/virelia/water-sprites')
    args = parser.parse_args()
    export(args.source_dir, args.review_dir)


if __name__ == '__main__':
    main()
