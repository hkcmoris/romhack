#!/usr/bin/env python3
"""Check native tile indices, palettes, and the original roaring landmarks."""
import json
import struct
from pathlib import Path
from PIL import Image

ROOT = Path(__file__).resolve().parent
REPO = ROOT.parents[1]
READY = ROOT / "game-ready"
NATIVE = REPO / "build/assets/mightyena-v5-check"

def load(path):
    with Image.open(path) as im:
        return im.copy()

def main():
    result = {"native_round_trip": {}, "palettes": {}, "roaring_reference": {}}
    for name in ("front", "front_2", "anim_front", "back", "overworld", "icon-art", "icon"):
        source = load(READY / (name + ".png"))
        check = load(NATIVE / (name + ".png"))
        assert source.mode == check.mode == "P", name
        assert source.size == check.size, (name, source.size, check.size)
        assert source.tobytes() == check.tobytes(), name
        expected = source.width * source.height // 2
        actual = (NATIVE / (name + ".4bpp")).stat().st_size
        assert expected == actual, (name, expected, actual)
        result["native_round_trip"][name] = {"pass": True, "bytes": actual}
    for name in ("normal", "overworld_normal", "icon"):
        lines = (READY / (name + ".pal")).read_text().splitlines()
        colors = [tuple(map(int, line.split())) for line in lines[3:] if line.strip()]
        expected = [(r >> 3) | ((g >> 3) << 5) | ((b >> 3) << 10) for r,g,b in colors]
        blob = (NATIVE / (name + ".gbapal")).read_bytes()
        assert len(blob) == 32
        assert list(struct.unpack("<16H", blob)) == expected, name
        result["palettes"][name] = {"pass": True, "bytes": 32}
    original = load(ROOT / "references/native/front-2.png").convert("RGBA")
    authored = load(ROOT / "exports/front-2.png").convert("RGBA")
    special = {(255,255,255):(248,240,216), (222,57,57):(200,80,104),
               (164,16,16):(128,48,64), (255,197,90):(248,208,104)}
    count = 0
    for old,new in zip(original.get_flattened_data(), authored.get_flattened_data()):
        if old[3]:
            assert new[3] == 255, "Original roaring silhouette lost a pixel"
            if old[:3] in special:
                assert new[:3] == special[old[:3]], ("Roaring facial landmark recolored",old,new)
                count += 1
    result["roaring_reference"] = {"original_opaque_geometry_preserved": True,
        "teeth_eye_nose_and_tongue_landmarks_checked": count}
    result["approved_front_preserved_byte_for_byte"] = (
        (READY / "front.png").read_bytes() == (ROOT / "front-diivi/front.png").read_bytes())
    assert result["approved_front_preserved_byte_for_byte"]
    (ROOT / "native-verification.json").write_text(json.dumps(result, indent=2) + "\n")
    print(json.dumps(result, indent=2))

if __name__ == "__main__":
    main()
