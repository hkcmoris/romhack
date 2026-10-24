#!/usr/bin/env python3
"""Compose a compact lab case from the native lab furniture, colors and Ball parts.

The stock lab bank stays intact. Eight case states share one new palette and
deduplicated tiles; a single north-facing map event opens the starter screen.
Regeneration and --check use only Python's standard library.
"""
import argparse
from pathlib import Path
import struct
import zlib
import re

from localize_type_icons import read_png

ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "data/tilesets/secondary/lab"
TARGET = ROOT / "data/tilesets/secondary/virelia_lab"
CASE_BASE = 0x29E


def asset_matches(name, actual, expected):
    # Git checks .pal files out as CRLF; source archives can retain LF.
    # Normalize only text palettes, keeping every binary byte significant.
    if name.endswith(".pal"):
        return actual.replace(b"\r\n", b"\n") == expected.replace(b"\r\n", b"\n")
    return actual == expected


def colors(slot):
    return [tuple(map(int, line.split())) for line in
            (SOURCE / f"palettes/{slot:02}.pal").read_text().splitlines()[3:19]]


def build():
    source, depth, chunks = read_png(SOURCE / "tiles.png")
    primary, _, _ = read_png(ROOT / "data/tilesets/primary/building/tiles.png")
    tables = [(ROOT / "data/tilesets/primary/building/metatiles.bin").read_bytes(),
              (SOURCE / "metatiles.bin").read_bytes()]
    header = (ROOT / "include/constants/virelia_prologue.h").read_text()
    assert int(re.search(r"#define\s+VIRELIA_LAB_CASE_BASE_METATILE\s+(0x[0-9A-F]+)",header)[1],0) == CASE_BASE
    assert all(value >> 12 != 12 for value in struct.unpack(f"<{len(tables[1])//2}H",tables[1])), "stock furniture uses the case palette"
    palettes = [colors(i) if i >= 6 else [tuple(map(int, line.split())) for line in
                (ROOT / f"data/tilesets/primary/building/palettes/{i:02}.pal").read_text().splitlines()[3:19]]
                for i in range(16)]
    palette = [palettes[7][0], palettes[7][1], palettes[7][2], palettes[8][2],
               palettes[8][3], palettes[7][4], palettes[7][8], palettes[7][12],
               palettes[8][10], palettes[8][11], palettes[8][7], palettes[8][5],
               palettes[8][6], palettes[6][7], palettes[6][9], palettes[6][0]]

    def nearest(rgb):
        return min(range(1, 16), key=lambda i: sum((a-b)**2 for a,b in zip(rgb,palette[i])))

    def metatile(number):
        bank = int(number >= 512)
        result = [[0]*16 for _ in range(16)]
        offset = (number - 512 if bank else number) * 16
        for k,value in enumerate(struct.unpack_from("<8H",tables[bank],offset)):
            tile,slot = value & 1023,value >> 12
            pixels = source if tile >= 512 else primary
            tile %= 512
            for y in range(8):
                for x in range(8):
                    dx,dy = (7-x if value&1024 else x),(7-y if value&2048 else y)
                    pixel = pixels[tile//16*8+dy][tile%16*8+dx]
                    if pixel or k < 4:
                        result[k%4//2*8+y][k%2*8+x] = nearest(palettes[slot][pixel])
        return result

    # The research bench provides the metal shell; bed-head parts provide its
    # raised lid/handle. The lining uses the native chair's green palette.
    shell = [a+b for a,b in zip(metatile(0x293),metatile(0x295))]
    lid = [a+b for a,b in zip(metatile(0x26C),metatile(0x26D))]
    def case(mask):
        image = [[0]*32 for _ in range(32)]
        for y in range(3,15):
            image[y][1:31] = lid[min(y-3,7)][1:31]
        for y in range(16):
            image[y+14] = shell[y][:]
        for y in range(7,13):
            image[y][5:27] = [8 if y in (7,12) else 1]*22
        for y in range(16,27):
            image[y][4:28] = [1]*24
        image[15][4:28] = [3]*24
        image[27][4:28] = [3]*24
        for x in (6,24):
            image[13][x:x+2] = [11,12]
            image[28][x:x+2] = [11,10]
        # Miniature versions of the lab's native red/white Ball motif.
        ball = ("..ddd..", ".drrrd.", "drrrrrd", "dddWddd", "dwwwwwd", ".dwwwd.", "..ddd..")
        ink = {"d":1,"r":6,"W":5,"w":14}
        for slot,x in enumerate((4,12,20)):
            if mask & (1 << slot):
                for y,row in enumerate(ball):
                    for dx,char in enumerate(row):
                        if char in ink:
                            image[y+18][x+dx] = ink[char]
        return image

    # Existing indexed tiles keep their positions, flips and palette references.
    tiles = [tuple(source[y+dy][x+dx] for dy in range(8) for dx in range(8))
             for y in range(0,len(source),8) for x in range(0,128,8)]
    used = {value & 1023 for value in struct.unpack(f"<{len(tables[1])//2}H",tables[1])}
    used = {i-512 for i in used if i >= 512}
    assert max(used) < 500
    free = [i for i in range(500) if i not in used]
    by_pixels = {tiles[i]:i for i in used}
    metatiles = bytearray(tables[1])
    attrs = bytearray((SOURCE / "metatile_attributes.bin").read_bytes())
    assert len(metatiles)//16+512 == CASE_BASE
    floor = struct.unpack_from("<8H",tables[1],(0x202-512)*16)[:4]
    previews = []
    for mask in range(8):
        image = case(mask)
        previews.append(image)
        for my in range(2):
            for mx in range(2):
                overlay = []
                for dy in range(2):
                    for dx in range(2):
                        pixels = tuple(image[my*16+dy*8+y][mx*16+dx*8+x] for y in range(8) for x in range(8))
                        if pixels not in by_pixels:
                            assert free, "case overlaps reserved animation tiles"
                            index = free.pop(0)
                            by_pixels[pixels] = index
                            tiles[index] = pixels
                            used.add(index)
                        overlay.append((512+by_pixels[pixels]) | (12 << 12))
                metatiles.extend(struct.pack("<8H",*floor,*overlay))
                attrs.extend(struct.pack("<H",0x1000)) # ground layers, below actors
    pixels = [[0]*128 for _ in range((len(tiles)+15)//16*8)]
    for i,tile in enumerate(tiles):
        for y in range(8):
            pixels[i//16*8+y][i%16*8:i%16*8+8] = tile[y*8:y*8+8]
    chunks = [(kind,struct.pack(">IIBBBBB",128,len(pixels),depth,3,0,0,0) if kind == b"IHDR" else data)
              for kind,data in chunks]
    def chunk(kind,data):
        return struct.pack(">I",len(data))+kind+data+struct.pack(">I",zlib.crc32(kind+data))
    assert depth == 4
    raw = b"".join(b"\0"+bytes((row[x]<<4)|row[x+1] for x in range(0,128,2)) for row in pixels)
    png = b"\x89PNG\r\n\x1a\n" + b"".join(chunk(kind,data) for kind,data in chunks if kind not in (b"IDAT",b"IEND"))
    png += chunk(b"IDAT",zlib.compress(raw,9))+chunk(b"IEND",b"")
    pal = "JASC-PAL\n0100\n16\n" + "\n".join(" ".join(map(str,c)) for c in palette) + "\n"
    return {"tiles.png":png,"metatiles.bin":bytes(metatiles),
            "metatile_attributes.bin":bytes(attrs),"palettes/12.pal":pal.replace("\n", "\r\n").encode()},previews,palette,len(used)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check",action="store_true")
    parser.add_argument("--preview",type=Path)
    args = parser.parse_args()
    files,previews,palette,count = build()
    for name,content in files.items():
        path = TARGET/name
        if args.check:
            assert asset_matches(name, path.read_bytes(), content), f"stale lab case: {name}"
        else:
            path.parent.mkdir(parents=True,exist_ok=True)
            if not path.exists() or path.read_bytes() != content:
                path.write_bytes(content)
    if args.preview:
        # Optional Pillow preview is not required for generation or CI checks.
        from PIL import Image
        im=Image.new("RGB",(32*8,32),(40,48,48))
        for mask,pixels in enumerate(previews):
            for y,row in enumerate(pixels):
                for x,color in enumerate(row):
                    if color:
                        im.putpixel((mask*32+x,y),palette[color])
        im.resize((1024,128),Image.Resampling.NEAREST).save(args.preview)
    print(f"PASS: compact lab case, eight slot states, {count}/500 native graphics tiles")


if __name__ == "__main__":
    main()
