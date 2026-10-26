-- Native-grid construction from the approved concept; only one front frame.
local reference=app.open("C:/Users/morav/Documents/github/romhack/art/virelia_poochyena_v5/references/up.png")
local source=reference.cels[1].image
local sw,sh=source.width,source.height
local pc=app.pixelColor
local outside,queue={},{}
local head=1
local function push(x,y)
 local i=y*sw+x+1
 if outside[i] then return end
 local c=source:getPixel(x,y)
 local r,g,b=pc.rgbaR(c),pc.rgbaG(c),pc.rgbaB(c)
 if math.min(r,g,b)>=230 and math.max(r,g,b)-math.min(r,g,b)<25 then
  outside[i]=true
  queue[#queue+1]=i
 end
end
for x=0,sw-1 do push(x,0);push(x,sh-1) end
for y=0,sh-1 do push(0,y);push(sw-1,y) end
while head<=#queue do
 local index=queue[head]-1;head=head+1
 local x,y=index%sw,math.floor(index/sw)
 if x>0 then push(x-1,y) end
 if x<sw-1 then push(x+1,y) end
 if y>0 then push(x,y-1) end
 if y<sh-1 then push(x,y+1) end
end
local minx,miny,maxx,maxy=sw,sh,0,0
for y=0,sh-1 do for x=0,sw-1 do
 if not outside[y*sw+x+1] then
  minx=math.min(minx,x);miny=math.min(miny,y)
  maxx=math.max(maxx,x);maxy=math.max(maxy,y)
 end
end end
local colors={{144,184,168},{24,32,32},{40,48,40},{64,72,64},{104,112,96},{56,72,40},{80,96,56},{112,128,72},{152,168,96},{192,200,128},{184,136,48},{248,208,104},{128,48,64},{200,80,104},{176,184,192},{248,240,216}}
local width,height,ox,oy=18,24,7,6
local samples=7
local indices,mask={},{}
local function nearest(r,g,b)
 local best,dist=1,1e20
 for i=2,#colors do
  local c=colors[i]
  local d=(r-c[1])^2+2*(g-c[2])^2+(b-c[3])^2
  if d<dist then best,dist=i-1,d end
 end
 return best
end
for y=0,height-1 do for x=0,width-1 do
 local rr,gg,bb={},{},{}
 for sy=0,samples-1 do for sx=0,samples-1 do
  local px=math.min(maxx,math.floor(minx+(x+(sx+0.5)/samples)*(maxx-minx+1)/width))
  local py=math.min(maxy,math.floor(miny+(y+(sy+0.5)/samples)*(maxy-miny+1)/height))
  if not outside[py*sw+px+1] then
   local c=source:getPixel(px,py)
   rr[#rr+1]=pc.rgbaR(c);gg[#gg+1]=pc.rgbaG(c);bb[#bb+1]=pc.rgbaB(c)
  end
 end end
 local i=y*width+x+1
 if #rr>=samples*samples*0.48 then
  table.sort(rr);table.sort(gg);table.sort(bb)
  local mid=math.floor(#rr/2)+1
  mask[i]=true;indices[i]=nearest(rr[mid],gg[mid],bb[mid])
 end
end end
local function occupied(x,y)
 return x>=0 and y>=0 and x<width and y<height and mask[y*width+x+1]
end
local sprite=Sprite(32,32,ColorMode.RGB)
sprite.layers[1].name="Concept pixel-over / base clusters"
local img=sprite.cels[1].image
for y=0,height-1 do for x=0,width-1 do
 local i=y*width+x+1
 if mask[i] then
  local q=indices[i]
  local up,left=not occupied(x,y-1),not occupied(x-1,y)
  local down,right=not occupied(x,y+1),not occupied(x+1,y)
  if up or left or down or right then
   if q==12 or q==13 then q=12
   elseif down and not up then q=1
   elseif right and not up and not left then q=1
   elseif q>=5 and q<=9 then q=5
   else q=2 end
  end
  local c=colors[q+1]
  img:drawPixel(x+ox,y+oy,pc.rgba(c[1],c[2],c[3],255))
 end
end end
sprite:saveAs("C:/Users/morav/Documents/github/romhack/art/virelia_poochyena_v5/source/overworld-up.aseprite")
reference:close()
print("overworld-up: 18x24 content on 32 canvas; exact approved palette")
print("Source illustration bounds: "..minx..","..miny..","..maxx..","..maxy)
