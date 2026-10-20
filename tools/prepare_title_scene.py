#!/usr/bin/env python3
"""Convert supplied jungle and fog artwork into the title screen's GBA assets.

Run with a Python environment containing Pillow. No artwork is generated here.
The jungle uses palette bank 14, with scenery at indices 1..14 and two detected
eyes at index 15. The fog uses bank 15 and a separate, deduplicated tile atlas.
The optional composite preview models the GBA's additive fog blend (4/16 fog
plus 16/16 scenery); it omits logo, sprites, and palette animation.
"""

import argparse
from collections import deque
from pathlib import Path
import struct

from PIL import Image, ImageOps


SCENE_SIZE = (240, 160)
FOG_SIZE = (128, 64)
SCENE_BANK = 14
FOG_BANK = 15
FOG_TOP_TILE = 12
DEFAULT_EYE_REGION = (95, 84, 141, 109)
EYE_PEAK_RGB = (230, 255, 99)  # Hardware RGB(28, 31, 12), matching the C pulse.


def require(condition, message):
    if not condition:
        raise ValueError(message)


def gba_color(rgb):
    """Round to hardware RGB555 and expand it for a faithful PNG preview."""
    return tuple(round(round(channel * 31 / 255) * 255 / 31) for channel in rgb)


def load_rgba(path, size):
    with Image.open(path) as source:
        return ImageOps.exif_transpose(source).convert("RGBA").resize(
            size, Image.Resampling.LANCZOS
        )


def indexed_image(size, indices, palette):
    require(len(palette) == 16, "A GBA palette must contain exactly 16 entries")
    require(len(indices) == size[0] * size[1], "Image pixel count is inconsistent")
    require(all(0 <= index < 16 for index in indices), "Pixel index exceeds 4bpp")
    result = Image.new("P", size)
    result.putpalette([channel for rgb in palette for channel in rgb])
    result.putdata(indices)
    result.info["transparency"] = 0
    return result


def find_eyes(rgb, region):
    left, top, right, bottom = region
    require(
        0 <= left < right <= rgb.width and 0 <= top < bottom <= rgb.height,
        "Eye region must be a nonempty box inside the 240x160 scene",
    )
    pixels = rgb.load()
    candidates = set()
    for y in range(top, bottom):
        for x in range(left, right):
            red, green, blue = pixels[x, y]
            if (
                red >= 120
                and green >= 180
                and blue <= 160
                and green >= blue + 65
                and red >= blue + 35
            ):
                candidates.add((x, y))

    components = []
    remaining = set(candidates)
    while remaining:
        first = min(remaining, key=lambda point: (point[1], point[0]))
        remaining.remove(first)
        component = {first}
        pending = deque([first])
        while pending:
            x, y = pending.popleft()
            for dy in (-1, 0, 1):
                for dx in (-1, 0, 1):
                    neighbor = (x + dx, y + dy)
                    if neighbor in remaining:
                        remaining.remove(neighbor)
                        component.add(neighbor)
                        pending.append(neighbor)
        components.append(component)

    require(
        len(components) == 2,
        "Expected exactly two bright lime/yellow eye components in "
        f"{tuple(region)}; found {len(components)} with sizes "
        f"{[len(component) for component in components]}. "
        "Check the source artwork or pass --eye-region LEFT TOP RIGHT BOTTOM.",
    )
    require(
        all(2 <= len(component) <= 100 for component in components),
        "Detected eye components must each contain 2..100 native pixels",
    )
    components.sort(key=lambda component: min(x for x, _ in component))
    require(
        max(x for x, _ in components[0]) < min(x for x, _ in components[1]),
        "Detected eye components must form distinct left and right eyes",
    )
    mask = components[0] | components[1]
    return mask, components


def prepare_jungle(source, eye_region):
    rgba = load_rgba(source, SCENE_SIZE)
    require(
        rgba.getchannel("A").getextrema() == (255, 255),
        "Jungle source must be fully opaque",
    )
    rgb = rgba.convert("RGB")
    eyes, components = find_eyes(rgb, eye_region)
    source_pixels = list(rgb.get_flattened_data())
    scenery_pixels = [
        pixel for position, pixel in enumerate(source_pixels)
        if (position % rgb.width, position // rgb.width) not in eyes
    ]
    training = Image.new("RGB", (len(scenery_pixels), 1))
    training.putdata(scenery_pixels)
    quantized = training.quantize(
        colors=14, method=Image.Quantize.MEDIANCUT, dither=Image.Dither.NONE
    )
    fitted = quantized.getpalette()
    scenery_palette = [gba_color(tuple(fitted[index:index + 3]))
                       for index in range(0, 42, 3)]
    palette = [(0, 0, 0), *scenery_palette, EYE_PEAK_RGB]

    # Quantize against only scenery entries, so neither transparency nor the
    # eye animation index can accidentally be selected by ordinary scenery.
    cache = {}
    indices = []
    for position, pixel in enumerate(source_pixels):
        if (position % rgb.width, position // rgb.width) in eyes:
            indices.append(15)
            continue
        if pixel not in cache:
            cache[pixel] = 1 + min(
                range(14),
                key=lambda entry: sum(
                    (pixel[channel] - scenery_palette[entry][channel]) ** 2
                    for channel in range(3)
                ),
            )
        indices.append(cache[pixel])

    require(indices.count(15) == len(eyes), "Eye index leaked into scenery")
    require(0 not in indices, "The jungle must have no transparent pixels")
    tile_count = (SCENE_SIZE[0] // 8) * (SCENE_SIZE[1] // 8)
    require(tile_count * 32 <= 20 * 1024, "Jungle exceeds its 20 KiB VRAM budget")
    tilemap = [SCENE_BANK << 12] * (32 * 32)
    for row in range(20):
        for column in range(30):
            tilemap[row * 32 + column] |= row * 30 + column
    return indexed_image(SCENE_SIZE, indices, palette), palette, tilemap, components


def prepare_fog(source, shades):
    rgba = load_rgba(source, FOG_SIZE)
    pixels = list(rgba.get_flattened_data())
    has_alpha = rgba.getchannel("A").getextrema()[0] < 255
    color_sums = [[0, 0, 0, 0] for _ in range(shades + 1)]
    indices = []
    for red, green, blue, alpha in pixels:
        density = alpha if has_alpha else round(0.2126 * red + 0.7152 * green + 0.0722 * blue)
        # Very faint resampling fringes become transparent, which also keeps
        # the empty space above the fog band fully transparent in the tilemap.
        index = 0 if density <= 12 else min(shades, 1 + (density - 13) * shades // 243)
        indices.append(index)
        if index:
            factor = alpha / 255 if has_alpha else 1
            for channel, value in enumerate((red, green, blue)):
                color_sums[index][channel] += round(value * factor)
            color_sums[index][3] += 1
    require(any(indices), "Fog source contains no visible fog")
    palette = [(0, 0, 0)]
    for index in range(1, 16):
        if index <= shades and color_sums[index][3]:
            total = color_sums[index]
            palette.append(gba_color(tuple(round(value / total[3]) for value in total[:3])))
        else:
            palette.append((0, 0, 0))

    blank = bytes(64)
    tiles = [blank]
    lookup = {blank: 0}
    source_map = []
    for tile_y in range(8):
        for tile_x in range(16):
            tile = bytes(
                indices[(tile_y * 8 + y) * 128 + tile_x * 8 + x]
                for y in range(8) for x in range(8)
            )
            if tile not in lookup:
                lookup[tile] = len(tiles)
                tiles.append(tile)
            source_map.append(lookup[tile])
    atlas_height = ((len(tiles) + 15) // 16) * 8
    require(len(tiles) <= 256, "Fog requires more than 256 unique tiles")
    require(128 * atlas_height // 2 <= 8192, "Fog exceeds its 8 KiB VRAM budget")
    atlas_indices = [0] * (128 * atlas_height)
    for number, tile in enumerate(tiles):
        left, top = number % 16 * 8, number // 16 * 8
        for y in range(8):
            start = (top + y) * 128 + left
            atlas_indices[start:start + 8] = tile[y * 8:y * 8 + 8]
    tilemap = [FOG_BANK << 12] * (32 * 32)
    for row in range(8):
        for column in range(32):
            tilemap[(FOG_TOP_TILE + row) * 32 + column] |= source_map[row * 16 + column % 16]
    atlas = indexed_image((128, atlas_height), atlas_indices, palette)
    fog_band = indexed_image(FOG_SIZE, indices, palette)
    return atlas, palette, tilemap, fog_band, len(tiles)


def write_palette(path, palette):
    content = ["JASC-PAL", "0100", "16"]
    content.extend(" ".join(str(channel) for channel in rgb) for rgb in palette)
    path.write_bytes(("\r\n".join(content) + "\r\n").encode("ascii"))


def write_assets(output_dir, name, image, palette, tilemap):
    image.save(output_dir / f"{name}.png", bits=4, optimize=False, transparency=0)
    write_palette(output_dir / f"{name}.pal", palette)
    require(len(tilemap) == 1024, "Tilemap must contain 32x32 entries")
    (output_dir / f"{name}.bin").write_bytes(struct.pack("<1024H", *tilemap))


def write_previews(preview_dir, jungle, fog_band):
    preview_dir.mkdir(parents=True, exist_ok=True)
    background = jungle.convert("RGB")
    background.save(preview_dir / "title_jungle_preview.png")
    composite = background.copy()
    colors = fog_band.convert("RGB")
    for y in range(FOG_SIZE[1]):
        for x in range(SCENE_SIZE[0]):
            if not fog_band.getpixel((x % FOG_SIZE[0], y)):
                continue
            position = (x, FOG_TOP_TILE * 8 + y)
            scenery = composite.getpixel(position)
            fog = colors.getpixel((x % FOG_SIZE[0], y))
            blended = tuple(
                round(min(31, (scenery[channel] // 8 * 16 + fog[channel] // 8 * 4) // 16)
                      * 255 / 31)
                for channel in range(3)
            )
            composite.putpixel(position, blended)
    composite.save(preview_dir / "title_scene_preview.png")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--jungle-source", required=True, type=Path)
    parser.add_argument("--fog-source", required=True, type=Path)
    parser.add_argument("--output-dir", type=Path, default=Path("graphics/title_screen"))
    parser.add_argument("--preview-dir", type=Path)
    parser.add_argument("--eye-region", nargs=4, type=int, default=DEFAULT_EYE_REGION,
                        metavar=("LEFT", "TOP", "RIGHT", "BOTTOM"))
    parser.add_argument("--fog-shades", type=int, choices=range(3, 8), default=7)
    args = parser.parse_args()
    try:
        jungle, jungle_palette, jungle_map, eyes = prepare_jungle(
            args.jungle_source, args.eye_region
        )
        fog, fog_palette, fog_map, fog_band, fog_tiles = prepare_fog(
            args.fog_source, args.fog_shades
        )
        args.output_dir.mkdir(parents=True, exist_ok=True)
        write_assets(args.output_dir, "wraithyena_jungle", jungle, jungle_palette, jungle_map)
        write_assets(args.output_dir, "fog", fog, fog_palette, fog_map)
        if args.preview_dir:
            write_previews(args.preview_dir, jungle, fog_band)
        eye_bounds = [
            (min(x for x, _ in eye), min(y for _, y in eye),
             max(x for x, _ in eye) + 1, max(y for _, y in eye) + 1)
            for eye in eyes
        ]
        print(f"Jungle: 600 tiles / 19200 bytes, eye bounds {eye_bounds}")
        print(f"Fog: {fog_tiles} unique tiles, padded atlas {fog.width}x{fog.height}, "
              f"{fog.width * fog.height // 2} bytes")
        print(f"Wrote title assets to {args.output_dir.resolve()}")
    except (OSError, ValueError) as error:
        parser.exit(1, f"Title scene conversion failed: {error}\n")


if __name__ == "__main__":
    main()
