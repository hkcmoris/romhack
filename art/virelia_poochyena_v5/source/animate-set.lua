
local root="C:/Users/morav/Documents/github/romhack/art/virelia_poochyena_v5"
local pc=app.pixelColor
local function render(s,f)
 local im=Image(s.width,s.height,ColorMode.RGB);im:drawSprite(s,f);return im
end
local function save(s,name)
 s:saveAs(root.."/source/"..name..".aseprite")
end
local function animated(name,frames,ms)
 local s=app.open(root.."/source/"..name..".aseprite")
 local base=render(s,1)
 local results=frames(base)
 local out=Sprite(s.width,s.height,ColorMode.RGB);out:setPalette(s.palettes[1])
 local layer=out.layers[1];layer.name="Native walking / alternating contact"
 for i,im in ipairs(results) do
  if i>1 then out:newEmptyFrame() end
  out:newCel(layer,out.frames[i],im,Point(0,0));out.frames[i].duration=ms/1000
 end
 save(out,name);s:close();return out
end
local function raised(base,left,up)
 local out=Image(base)
 local minx,maxx=left and 7 or 18,left and 13 or 25
 local miny=up and 26 or 27
 local pixels={}
 for y=miny,29 do for x=minx,maxx do
  local c=base:getPixel(x,y)
  if pc.rgbaA(c)>0 then table.insert(pixels,{x,y,c});out:drawPixel(x,y,0) end
 end end
 for _,p in ipairs(pixels) do out:drawPixel(p[1],p[2]-1,p[3]) end
 return out
end
local down=animated("overworld-down",function(base)return {raised(base,false,false),raised(base,true,false)} end,180)
local up=animated("overworld-up",function(base)return {raised(base,false,true),raised(base,true,true)} end,180)
local side=animated("overworld-left",function(base)
 local out=Image(base);local pixels={}
 for y=24,29 do for x=4,27 do
  local c=base:getPixel(x,y)
  local dark=pc.rgbaA(c)>0 and pc.rgbaR(c)<=104 and pc.rgbaG(c)<=112 and pc.rgbaB(c)<=96
  local fore=x<=15 and y>=25;local rear=x>=20 and y>=26
  if dark and (fore or rear) then table.insert(pixels,{x,y,c,fore and -1 or 1});out:drawPixel(x,y,0) end
 end end
 for _,p in ipairs(pixels) do if pc.rgbaA(out:getPixel(p[1]+p[4],p[2]))==0 then out:drawPixel(p[1]+p[4],p[2],p[3]) end end
 return {base,out}
end,180)
local icon=animated("icon",function(base)
 local out=Image(base.width,base.height,ColorMode.RGB);out:drawImage(base,Point(0,1));return {base,out}
end,240)
icon.layers[1].name="Party icon / two-frame bounce";save(icon,"icon")
local ow=Sprite(32,32,ColorMode.RGB);ow:setPalette(down.palettes[1])
local layer=ow.layers[1];layer.name="Down / up / left, two steps each"
local n=0
for _,s in ipairs({down,up,side}) do for f=1,2 do
 n=n+1;if n>1 then ow:newEmptyFrame() end
 ow:newCel(layer,ow.frames[n],render(s,f),Point(0,0));ow.frames[n].duration=.18
end end
for i,name in ipairs({"Down","Up","Left"}) do local tag=ow:newTag(i*2-1,i*2);tag.name=name end
save(ow,"overworld");print("Created six walking cells and two icon frames")

