"""Pack Aseprite-authored pixels losslessly and make presentation-only previews."""
from collections import deque
from hashlib import sha256
import json
from pathlib import Path
import shutil
from PIL import Image, ImageDraw, ImageFont

HERE = Path(__file__).resolve().parent
PALETTE = [tuple(bytes.fromhex(c[1:])) for c in json.loads((HERE / "pixel-design.json").read_text())["palette"]]
LOCKED = "95a081d123f716d1b57a7c7492e5a40a80cdd337a5f0781221cf65866fcaa8e5"

def load(name, n):
    im = Image.open(HERE / "exports" / f"{name}-{n}.png").convert("RGBA")
    im.putdata([p if p[3] else (*PALETTE[0], 0) for p in im.get_flattened_data()])
    return im

def pack(im, name):
    lookup = {rgb: i for i, rgb in enumerate(PALETTE) if i}
    data = []
    for r, g, b, a in im.get_flattened_data():
        assert a in (0, 255), (name, "partial alpha")
        assert not a or (r, g, b) in lookup, (name, (r, g, b))
        data.append(lookup[r, g, b] if a else 0)
    out = Image.new("P", im.size)
    out.putpalette([v for rgb in PALETTE for v in rgb])
    out.putdata(data)
    path = HERE / "game-ready" / f"{name}.png"
    out.save(path, bits=4, transparency=0)
    assert path.read_bytes()[24:26] == bytes((4, 3))
    assert Image.open(path).convert("RGBA").tobytes() == im.tobytes()
    return {"size": im.size, "sha256": sha256(path.read_bytes()).hexdigest()}

def connected(im):
    occupied = {(x, y) for y in range(im.height) for x in range(im.width) if im.getpixel((x, y))[3]}
    unseen = set(occupied)
    components = []
    while unseen:
        seed = unseen.pop()
        queue = deque([seed])
        count = 1
        while queue:
            x, y = queue.popleft()
            for dx, dy in ((-1,-1),(0,-1),(1,-1),(-1,0),(1,0),(-1,1),(0,1),(1,1)):
                point = x+dx, y+dy
                if point in unseen:
                    unseen.remove(point)
                    queue.append(point)
                    count += 1
        components.append(count)
    return sorted(components, reverse=True)

def preview(frames):
    bg = "#E8E8E0"
    ink = "#283028"
    board = Image.new("RGB", (1060, 950), bg)
    draw = ImageDraw.Draw(board)
    font = ImageFont.truetype("C:/Windows/Fonts/consola.ttf", 18)
    title = ImageFont.truetype("C:/Windows/Fonts/consolab.ttf", 25)
    def text(x,y,value,big=False):
        draw.text((x,y),value,font=title if big else font,fill=ink)
    def cell(im,x,y,scale):
        w,h = im.width*scale,im.height*scale
        draw.rectangle((x,y,x+w-1,y+h-1), fill="#F4F4EC", outline="#C8D0C0")
        big=im.resize((w,h),Image.Resampling.NEAREST)
        board.paste(big,(x,y),big)
    text(28,16,"VIRELIAN POOCHYENA / APPROVED DESIGN",True)
    for i, (label,im) in enumerate(zip(("Front 1 (unchanged)","Front 2 / breath","Back"),frames["front"]+frames["back"])):
        x=28+i*346
        text(x,64,label);cell(im,x,96,4)
        board.paste(im,(x+275,190),im)
        text(x+278,261,"1x")
    text(28,385,"OVERWORLD / alternating steps",True)
    for i,im in enumerate(frames["overworld"]):
        x=28+i*171
        text(x,427,("Down 1","Down 2","Up 1","Up 2","Left 1","Left 2")[i])
        cell(im,x,458,4)
    text(28,626,"PARTY ICON / two frames",True)
    for i,im in enumerate(frames["icon"]):
        cell(im,28+i*190,674,4)
        text(28+i*190,814,f"Icon {i+1}")
    text(500,674,"ONE PALETTE / 15 COLORS + TRANSPARENCY")
    for i,rgb in enumerate(PALETTE[1:]):
        x=500+(i%8)*62;y=719+(i//8)*62
        draw.rectangle((x,y,x+45,y+45), fill=rgb)
    text(28,878,"64x64 battle cells / 32x32 walking and icon cells")
    text(28,908,"Original spacing retained. Right-facing walk uses engine mirroring.")
    board.save(HERE/"previews/review.png")
    def gif(name, ims, scale, ms):
        result=[]
        for im in ims:
            enlarged=im.resize((im.width*scale,im.height*scale),Image.Resampling.NEAREST)
            canvas=Image.new("RGB",enlarged.size,bg);canvas.paste(enlarged,(0,0),enlarged);result.append(canvas)
        result[0].save(HERE/f"previews/{name}.gif",save_all=True,append_images=result[1:],duration=ms,loop=0,disposal=2)
    gif("front",frames["front"],6,250)
    gif("icon",frames["icon"],8,240)
    walks=[]
    for step in range(2):
        strip=Image.new("RGBA",(128,32))
        for d in range(3):strip.paste(frames["overworld"][d*2+step],(d*32,0))
        strip.paste(frames["overworld"][4+step].transpose(Image.Transpose.FLIP_LEFT_RIGHT),(96,0))
        walks.append(strip)
    gif("walking",walks,6,180)

def main():
    assert sha256((HERE/"front.png").read_bytes()).hexdigest()==LOCKED
    frames={name:[load(name,n) for n in range(1,count+1)] for name,count in (("front",2),("back",1),("overworld",6),("icon",2))}
    assert frames["front"][0].tobytes()==Image.open(HERE/"front.png").convert("RGBA").tobytes(),"Approved front changed"
    sheets={"front":frames["front"][0],"front_2":frames["front"][1],"back":frames["back"][0]}
    battle=Image.new("RGBA",(64,128));battle.paste(frames["front"][0],(0,0));battle.paste(frames["front"][1],(0,64));sheets["anim_front"]=battle
    ow=Image.new("RGBA",(192,32))
    for n,im in enumerate(frames["overworld"]):ow.paste(im,(n*32,0))
    sheets["overworld"]=ow
    icon=Image.new("RGBA",(32,64));icon.paste(frames["icon"][0],(0,0));icon.paste(frames["icon"][1],(0,32));sheets["icon"]=icon
    records={name:pack(im,name) for name,im in sheets.items()}
    shutil.copyfile(HERE/"front.png",HERE/"game-ready/front.png")
    records["front"]["sha256"]=LOCKED
    for name in ("normal","overworld_normal"):
        shutil.copyfile(HERE/"normal.pal",HERE/f"game-ready/{name}.pal")
    bounds={}
    for name,ims in frames.items():
        bounds[name]=[]
        for im in ims:
            bbox=im.getbbox()
            assert bbox and bbox[0]>0 and bbox[1]>0 and bbox[2]<im.width and bbox[3]<im.height
            comps=connected(im)
            assert len(comps)==1,(name,comps)
            bounds[name].append({"bounds":bbox,"connected_components":len(comps)})
    report={"approved_front_unchanged":True,"approved_front_sha256":LOCKED,"palette":PALETTE,"binary_alpha":True,
            "indexed_png_bits":4,"sprites":records,"frame_bounds":bounds,"overworld_order":["down1","down2","up1","up2","left1","left2"],
            "runtime_installed":False,"icon_uses_custom_approved_palette":True}
    (HERE/"set-verification.json").write_text(json.dumps(report,indent=2)+"\n")
    preview(frames)
    print("PASS: eleven frames, identical approved front, exact palette, 4bpp sheets, connected silhouettes and clear borders")

if __name__=="__main__":
    main()

