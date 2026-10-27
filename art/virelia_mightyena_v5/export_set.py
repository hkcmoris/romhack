#!/usr/bin/env python3
"""Pack diivi's saved native pixels; no procedural drawing or runtime resampling."""
import hashlib
import json
import shutil
from pathlib import Path
from PIL import Image, ImageDraw

ROOT = Path(__file__).resolve().parent
REPO = ROOT.parents[1]
READY = ROOT / "game-ready"
PREVIEWS = ROOT / "previews"
ICON_ROLE_MAP = [0, 15, 15, 15, 1, 15, 1, 4, 5, 12, 10, 13, 13, 3, 2, 3]

def rgba(path):
    with Image.open(path) as im:
        return im.convert("RGBA")

def same_visible(a, b):
    return a.size == b.size and all(p[3] == q[3] and (not p[3] or p == q)
        for p, q in zip(a.get_flattened_data(), b.get_flattened_data()))

def exact_index(im, colors):
    lookup = {c: i for i, c in enumerate(colors) if i}
    data = bytearray()
    for pixel in im.get_flattened_data():
        assert pixel[3] in (0, 255), "Partial alpha in creature art"
        if not pixel[3]:
            data.append(0)
        else:
            assert pixel[:3] in lookup, f"Unexpected color: {pixel}"
            data.append(lookup[pixel[:3]])
    result = Image.frombytes("P", im.size, bytes(data))
    result.putpalette([v for c in colors for v in c])
    result.info["transparency"] = 0
    assert same_visible(im, result.convert("RGBA"))
    return result

def save(im, name):
    im.save(READY / name, bits=4, transparency=0)

def split(im, w, h):
    return [im.crop((x,y,x+w,y+h)) for y in range(0,im.height,h)
            for x in range(0,im.width,w)]

def panel(im, bg="#E9EFEB"):
    result = Image.new("RGBA", im.size, bg)
    result.alpha_composite(im.convert("RGBA"))
    return result.convert("RGB")

def main():
    READY.mkdir(exist_ok=True)
    PREVIEWS.mkdir(exist_ok=True)
    design = json.loads((ROOT/"front-diivi/front-design.json").read_text())
    colors = [tuple(bytes.fromhex(h[1:])) for h in design["palette"]]
    icon_pal = REPO/"graphics/pokemon/icon_palettes/pal1.pal"
    lines = icon_pal.read_text(encoding="utf-8-sig").splitlines()
    assert lines[:3] == ["JASC-PAL","0100","16"]
    icon_colors = [tuple(map(int,line.split())) for line in lines[3:] if line.strip()]
    assert len(icon_colors) == 16
    specs = {"front":((64,128),64), "back":((64,64),64),
             "overworld":((192,32),32), "icon":((32,64),32)}
    sources, indexed, frames = {}, {}, {}
    for name, (size, cell) in specs.items():
        sources[name] = rgba(ROOT/"exports"/f"{name}-rgba.png")
        assert sources[name].size == size, (name,sources[name].size)
        indexed[name] = exact_index(sources[name], colors)
        frames[name] = split(sources[name],cell,cell)
        for i, frame in enumerate(frames[name],1):
            frame.save(ROOT/"exports"/f"{name}-{i}.png")
    approved = ROOT/"front-diivi/front.png"
    assert same_visible(frames["front"][0],rgba(approved)), "Approved front changed"
    save(indexed["front"],"anim_front.png")
    save(indexed["front"].crop((0,64,64,128)),"front_2.png")
    shutil.copyfile(approved, READY/"front.png")
    assert approved.read_bytes() == (READY/"front.png").read_bytes()
    save(indexed["back"],"back.png")
    save(indexed["overworld"],"overworld.png")
    save(indexed["icon"],"icon-art.png")
    game_icon = Image.frombytes("P", indexed["icon"].size,
        bytes(ICON_ROLE_MAP[i] for i in indexed["icon"].tobytes()))
    game_icon.putpalette([v for c in icon_colors for v in c])
    game_icon.info["transparency"] = 0
    assert all((a==0)==(b==0) for a,b in zip(indexed["icon"].tobytes(),game_icon.tobytes()))
    save(game_icon,"icon.png")
    frames["icon_game"] = split(game_icon.convert("RGBA"),32,32)
    for name in ("normal.pal","overworld_normal.pal"):
        shutil.copyfile(ROOT/"front-diivi/normal.pal", READY/name)
    shutil.copyfile(icon_pal, READY/"icon.pal")
    board = Image.new("RGB",(960,730),"#DEE6DE")
    draw = ImageDraw.Draw(board)
    draw.text((10,5),"Virelian Mightyena v5 - normal sprite set",fill="#182020")
    battle = [("Front - approved",frames["front"][0]),
              ("Front 2 - original roaring pose",frames["front"][1]),
              ("Back - flat leaf clumps",frames["back"][0])]
    for col,(title,im) in enumerate(battle):
        x=col*304+12
        draw.text((x,28),title,fill="#182020")
        board.paste(panel(im).resize((256,256),Image.Resampling.NEAREST),(x,48))
    names = ["Down A","Down B","Up A","Up B","Left A","Left B"]
    for col,(title,im) in enumerate(zip(names,frames["overworld"])):
        x=col*160+10
        draw.text((x,330),title,fill="#182020")
        board.paste(panel(im).resize((128,128),Image.Resampling.NEAREST),(x,350))
    icons = [("Icon art A",frames["icon"][0]),("Icon art B",frames["icon"][1]),
             ("Game palette 1 A",frames["icon_game"][0]),("Game palette 1 B",frames["icon_game"][1])]
    for col,(title,im) in enumerate(icons):
        x=col*200+10
        draw.text((x,510),title,fill="#182020")
        board.paste(panel(im).resize((128,128),Image.Resampling.NEAREST),(x,532))
    draw.text((10,694),"Native pixels preserved. Enlarged panels use nearest-neighbor.",fill="#182020")
    board.save(PREVIEWS/"review.png")
    walk = []
    for phase in range(2):
        canvas=Image.new("RGB",(768,224),"#E4ECE5")
        labels=ImageDraw.Draw(canvas)
        directions=[frames["overworld"][phase],frames["overworld"][phase+2],
                    frames["overworld"][phase+4],
                    frames["overworld"][phase+4].transpose(Image.Transpose.FLIP_LEFT_RIGHT)]
        for col,(name,im) in enumerate(zip(("Down","Up","Left","Right (mirrored)"),directions)):
            labels.text((col*192+8,3),name,fill="#182020")
            canvas.paste(panel(im).resize((192,192),Image.Resampling.NEAREST),(col*192,24))
        walk.append(canvas)
    walk[0].save(PREVIEWS/"walking.gif",save_all=True,append_images=walk[1:],
                 duration=180,loop=0,disposal=2)
    gif = [panel(im).resize((256,256),Image.Resampling.NEAREST) for im in frames["icon_game"]]
    gif[0].save(PREVIEWS/"icon-game.gif",save_all=True,append_images=gif[1:],
                duration=250,loop=0,disposal=2)
    report = {
      "frame_counts":{k:len(v) for k,v in frames.items()},
      "total_art_frames":sum(len(frames[k]) for k in specs),
      "approved_front_preserved_byte_for_byte":True,
      "encoding":"Exact RGB-to-index mapping; four-bit PNG; no art resampling or color quantization",
      "transparent_index":0,"icon_engine_palette_index":1,
      "icon_role_to_engine_index":ICON_ROLE_MAP,
      "shiny":"Not redesigned in this normal-color art set","runtime_installed":False,
      "roaring_pose":"Original GBA second-frame anatomy, including original top-edge ear tips",
      "art_review":"Separate manual inspection; technical checks do not establish drawing quality",
      "sha256":{p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in sorted(READY.iterdir()) if p.is_file()}
    }
    (ROOT/"set-verification.json").write_text(json.dumps(report,indent=2)+"\n")
    print(json.dumps({k:v for k,v in report.items() if k!="sha256"},indent=2))

if __name__ == "__main__":
    main()
