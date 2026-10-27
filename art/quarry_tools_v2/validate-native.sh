#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
make -C tools/gbagfx
task_src=graphics/object_events/pics/quarry
task_out=build/assets/quarry-tools-v2-check
mkdir -p "$task_out"
for task_name in wrench hammer pick tool_hoard; do
    tools/gbagfx/gbagfx "$task_src/$task_name.png" "$task_out/$task_name.4bpp"
    tools/gbagfx/gbagfx "$task_out/$task_name.4bpp" "$task_out/$task_name.png" \
        -width 2 -palette graphics/object_events/palettes/moving_box.pal -object
done
tools/gbagfx/gbagfx graphics/object_events/palettes/moving_box.pal "$task_out/moving_box.gbapal"
