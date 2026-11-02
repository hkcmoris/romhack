#!/usr/bin/env python3
"""Validate the prepared Aseprite kit without changing game assets."""
import json
from pathlib import Path
import struct
import sys
import zlib

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from virelia.localize_type_icons import read_png


def require(ok, message):
    if not ok:
        raise ValueError(message)


def aseprite_pixels(path):
    data = path.read_bytes()
    size, magic, frames, width, height, depth = struct.unpack_from('<I5H', data)
    require(size == len(data) and magic == 0xA5E0, 'Invalid Aseprite source')
    require(frames == 1 and depth == 8, 'Source must be one indexed frame')
    canvas = [[0] * width for _ in range(height)]
    frame_size, frame_magic, old_count = struct.unpack_from('<IHH', data, 128)
    new_count = struct.unpack_from('<I', data, 140)[0]
    require(frame_magic == 0xF1FA and 128 + frame_size == len(data), 'Invalid source frame')
    offset, layers, cels, slices = 144, [], [], {}
    for _ in range(new_count or old_count):
        length, kind = struct.unpack_from('<IH', data, offset)
        require(length >= 6 and offset + length <= len(data), 'Invalid source chunk')
        chunk = data[offset + 6:offset + length]
        if kind == 0x2004:  # Native layer flags and opacity.
            flags, layer_type = struct.unpack_from('<HH', chunk)
            require(layer_type == 0 and chunk[12] == 255, 'Expected ordinary opaque layers')
            layers.append(bool(flags & 1))
        elif kind == 0x2005:
            layer, x, y, opacity, cel_type = struct.unpack_from('<HhhBH', chunk)
            require(opacity == 255 and cel_type in (0, 2), 'Unsupported source cel')
            w, h = struct.unpack_from('<HH', chunk, 16)
            pixels = zlib.decompress(chunk[20:]) if cel_type == 2 else chunk[20:]
            require(len(pixels) == w * h, 'Malformed source pixels')
            cels.append((layer, x, y, w, h, pixels))
        elif kind == 0x2022:
            keys, flags = struct.unpack_from('<II', chunk)
            name_len = struct.unpack_from('<H', chunk, 12)[0]
            name = chunk[14:14 + name_len].decode('utf-8')
            require(keys == 1 and flags == 0 and name not in slices, 'Unexpected slice keys')
            frame, x, y, w, h = struct.unpack_from('<IiiII', chunk, 14 + name_len)
            require(frame == 0, 'Slice must belong to first frame')
            slices[name] = [x, y, w, h]
        offset += length
    require(offset == len(data), 'Unparsed source data')
    for layer, x, y, w, h, pixels in sorted(cels):
        if layers[layer]:
            for j in range(h):
                for i in range(w):
                    value = pixels[j * w + i]
                    if value:
                        require(0 <= x + i < width and 0 <= y + j < height, 'Cel outside canvas')
                        canvas[y + j][x + i] = value
    return canvas, slices


def validate():
    manifest = json.loads((HERE / 'manifest.json').read_text())
    pixels, depth, chunks = read_png(HERE / manifest['export'])
    require([len(pixels[0]), len(pixels)] == manifest['canvas'] == [128, 128], 'Atlas size changed')
    require(depth in (4, 8) and all(0 <= p < 16 for row in pixels for p in row), 'Atlas exceeds 4bpp indices')
    native = [tuple(map(int, line.split())) for line in
              (ROOT / manifest['palette_source']).read_text().splitlines()[3:19]]
    chunks = dict(chunks)
    colors = list(zip(*[iter(chunks[b'PLTE'])] * 3))
    require(colors == native, 'Export must preserve the exact existing quarry palette')
    require(chunks.get(b'tRNS') == bytes([0] + [255] * 15), 'Only index zero may be transparent')
    require(all(c % 8 == 0 for color in colors for c in color), 'Palette is not GBA-exact')
    source, slices = aseprite_pixels(HERE / manifest['source'])
    require(source == pixels, 'PNG export differs from editable Aseprite pixels')
    expected = {a['id']: a['rect'] for a in manifest['assets']}
    require(len(expected) == 17 and slices == expected, 'Manifest and Aseprite named slices differ')
    occupied = set()
    for name, (x, y, w, h) in expected.items():
        require(all(v % 16 == 0 for v in (x, y, w, h)), f'{name}: not metatile-aligned')
        require(0 <= x < x + w <= 128 and 0 <= y < y + h <= 128, f'{name}: outside atlas')
        area = {(xx, yy) for yy in range(y, y + h) for xx in range(x, x + w)}
        require(not occupied & area, f'{name}: overlaps another slice')
        occupied |= area
        require(any(pixels[yy][xx] for xx, yy in area), f'{name}: empty art')
    require(all(not p or (x, y) in occupied for y, row in enumerate(pixels) for x, p in enumerate(row)),
            'Opaque pixels outside named slices')
    # All rail modules share the same two three-pixel rail endpoints, including
    # mirrored corners; sleepers never cross a tile boundary between the rails.
    rail_edges = {
        'horizontal': ('left', 'right'), 'vertical': ('top', 'bottom'),
        'bend_ne': ('top', 'right'), 'bend_nw': ('top', 'left'),
        'bend_se': ('bottom', 'right'), 'bend_sw': ('bottom', 'left'),
    }
    for name, edges in rail_edges.items():
        x, y, _, _ = expected[name]
        for edge in edges:
            values = [pixels[y + (0 if edge == 'top' else 15)][x + i] if edge in ('top', 'bottom')
                      else pixels[y + i][x + (0 if edge == 'left' else 15)] for i in range(16)]
            require({i for i, p in enumerate(values) if p} == {3, 4, 5, 10, 11, 12},
                    f'{name}: mismatched {edge} rail seam')
    tiles = {tuple(pixels[y + j][x + i] for j in range(8) for i in range(8))
             for y in range(0, 128, 8) for x in range(0, 128, 8)}
    tiles.discard((0,) * 64)
    print(f'PASS: 17 named Aseprite slices, exact source/export pixels, native palette and rail seams; '
          f'{len(tiles)} unique nonblank 8x8 graphics before terrain packing')


if __name__ == '__main__':
    try:
        validate()
    except (ValueError, KeyError, IndexError, OSError, AssertionError, struct.error, zlib.error) as error:
        print(f'FAIL: {error}', file=sys.stderr)
        sys.exit(1)
