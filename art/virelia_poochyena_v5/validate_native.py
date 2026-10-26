"""Compare gbagfx round trips with the runtime assets, including palette words."""

from hashlib import sha256
import json
from pathlib import Path
import struct

from PIL import Image

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[1]
SOURCE = HERE / "game-ready"
BUILD = ROOT / "build/assets/poochyena-v5-check"


def main():
    records = {}
    sizes = {"front": 2048, "front_2": 2048, "anim_front": 4096,
             "back": 2048, "icon": 1024, "overworld": 3072}
    for name, size in sizes.items():
        with Image.open(SOURCE / f"{name}.png") as source, Image.open(BUILD / f"{name}.png") as result:
            assert source.size == result.size, name
            assert source.tobytes() == result.tobytes(), f"{name}: tile/index mismatch"
            assert source.convert("RGBA").tobytes() == result.convert("RGBA").tobytes(), f"{name}: palette mismatch"
        raw = (BUILD / f"{name}.4bpp").read_bytes()
        assert len(raw) == size, name
        records[name] = {"binary_bytes": size, "round_trip_indices_identical": True,
                         "round_trip_rgba_identical": True, "binary_sha256": sha256(raw).hexdigest()}
    for name in ("normal", "overworld_normal"):
        colors = [tuple(map(int, row.split())) for row in
                  (SOURCE / f"{name}.pal").read_text().splitlines()[3:19]]
        expected = struct.pack("<16H", *[(r >> 3) | ((g >> 3) << 5) | ((b >> 3) << 10)
                                        for r, g, b in colors])
        raw = (BUILD / f"{name}.gbapal").read_bytes()
        assert raw == expected, name
        records[name + "_palette"] = {"binary_bytes": len(raw), "rgb555_words_identical": True}
    records["scope"] = "Native graphics conversion only; no full ROM build or emulator playtest"
    (HERE / "native-verification.json").write_text(json.dumps(records, indent=2) + "\n")
    print("PASS: six native sprite index/RGBA round trips and two exact RGB555 palettes")


if __name__ == "__main__":
    main()
