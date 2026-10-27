#!/usr/bin/env bash
# Convert the exact tile layouts used by the game and round-trip their indices.
set -euo pipefail
cd "$(dirname "$0")/../.."
make -C tools/gbagfx
task_out=build/assets/mightyena-v5-check
task_src=art/virelia_mightyena_v5/game-ready
mkdir -p "$task_out"
for task_name in front front_2 anim_front back icon-art icon; do
    tools/gbagfx/gbagfx "$task_src/$task_name.png" "$task_out/$task_name.4bpp"
    task_width=8
    task_palette="$task_src/normal.pal"
    if [[ "$task_name" = icon || "$task_name" = icon-art ]]; then task_width=4; fi
    if [[ "$task_name" = icon ]]; then task_palette="$task_src/icon.pal"; fi
    tools/gbagfx/gbagfx "$task_out/$task_name.4bpp" "$task_out/$task_name.png" \
        -width "$task_width" -palette "$task_palette" -object
done
tools/gbagfx/gbagfx "$task_src/overworld.png" "$task_out/overworld.4bpp" -mwidth 4 -mheight 4
tools/gbagfx/gbagfx "$task_out/overworld.4bpp" "$task_out/overworld.png" \
    -width 24 -mwidth 4 -mheight 4 -palette "$task_src/overworld_normal.pal" -object
for task_name in normal overworld_normal icon; do
    tools/gbagfx/gbagfx "$task_src/$task_name.pal" "$task_out/$task_name.gbapal"
done
wc -c "$task_out"/*.4bpp "$task_out"/*.gbapal
