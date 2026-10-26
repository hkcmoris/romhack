#!/usr/bin/env bash
# Check the same tile layouts used by INCGFX; outputs are ignored build artifacts.
set -euo pipefail
cd "$(dirname "$0")/../.."
make -C tools/gbagfx
out=build/assets/poochyena-v2-check
src=graphics/pokemon/poochyena/virelia
mkdir -p "$out"
for name in front front_2 anim_front back icon; do
    tools/gbagfx/gbagfx "$src/$name.png" "$out/$name.4bpp"
    width=8
    palette="$src/normal.pal"
    if [ "$name" = icon ]; then
        width=4
        palette=graphics/pokemon/icon_palettes/pal1.pal
    fi
    tools/gbagfx/gbagfx "$out/$name.4bpp" "$out/$name.png" \
        -width "$width" -palette "$palette" -object
done
tools/gbagfx/gbagfx "$src/overworld.png" "$out/overworld.4bpp" -mwidth 4 -mheight 4
tools/gbagfx/gbagfx "$out/overworld.4bpp" "$out/overworld.png" \
    -width 24 -mwidth 4 -mheight 4 -palette "$src/overworld_normal.pal" -object
for name in normal shiny overworld_normal overworld_shiny; do
    tools/gbagfx/gbagfx "$src/$name.pal" "$out/$name.gbapal"
done
wc -c "$out"/*.4bpp "$out"/*.gbapal
