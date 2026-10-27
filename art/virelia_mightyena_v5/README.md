# Virelian Mightyena v5 - complete normal sprite set

The set contains eleven art frames, drawn and edited at the native grid through diivi's Aseprite MCP. The approved first front frame is preserved byte for byte. The second front frame uses the original GBA Mightyena's raised-head, open-mouth roaring pose, including its lowered tail and original stance.

The remaining views retain the green leaf-fur coat, charcoal face and lower paws, red nose, yellow/red eyes, ivory claws and restrained mint ghost wisps. Back and walking fur use broad, pointed leaf clumps with short highlights and overlap shadows. The back face coloring follows the brow, cheek and muzzle.

## Runtime-format exports

All paths below are inside game-ready/.

| File | Layout and palette |
| --- | --- |
| front.png | Approved first front, 64x64, normal.pal |
| front_2.png | Original roaring pose adapted to the regional design, 64x64 |
| anim_front.png | Both front frames vertically, 64x128 |
| back.png | Rear battle view, 64x64 |
| overworld.png | Six horizontal 32x32 cells, 192x32 |
| icon.png | Two vertical 32x32 frames, 32x64, engine shared palette 1 |
| icon-art.png | The same icon artwork using the regional art palette |
| normal.pal / overworld_normal.pal | Matching ordered 16-entry art palettes |
| icon.pal | Copy of graphics/pokemon/icon_palettes/pal1.pal for standalone validation |

The walking order is down A, down B, up A, up B, left A, left B. Right-facing movement uses the engine's horizontal mirroring. The shared-palette icon export maps material roles deliberately rather than changing the game's existing icon palette. The art-palette variant remains available for review.

All runtime PNGs use four-bit indexed encoding with transparent index 0. Native pixels are preserved; there is no runtime-art resizing or nearest-color quantization. Palette reduction for the engine icon is an explicit role-to-index mapping recorded in set-verification.json.

## Editable sources and previews

- source/front.aseprite: two layered frames, including the roaring frame.
- source/back.aseprite: rear coat, flat leaf groups and ghost accents.
- source/overworld.aseprite: six frames and Down/Up/Left tags.
- source/icon.aseprite: two frames in the art palette.
- previews/review.png: the full set, plus both icon palette variants.
- previews/front.gif: approved-to-roaring animation.
- previews/walking.gif: down/up/left/mirrored-right walking together.
- previews/walk-down.gif, walk-up.gif and walk-left.gif: separate walking pairs.
- previews/icon.gif and icon-game.gif: art and shared-palette icon previews.
- exports/: direct native RGBA exports and individual frames.
- references/native/: unscaled original Mightyena anatomy/pose references.
- drawing-plan.json: material, leaf, wisp and pose decisions.
- validation/: per-sheet palette/geometry audits and native-size RGB555 proofs.
- native-verification.json: native converter round-trips and roaring landmarks.

The live session for this run is http://localhost:7778/. Select front.aseprite, back.aseprite, overworld.aseprite or icon.aseprite from the Mightyena source folder. Ports are session-specific.

## Validation and review

All seven indexed-sheet audits pass. The native gbagfx converter round-trips every exported sheet with exact pixel indices and expected binary sizes, including the 4x4 metatile walking layout. All three palette binaries match their expected RGB555 words. The approved first front PNG is byte-identical to its accepted source. The roaring frame retains the original opaque geometry and the checked eye, nose, tongue and tooth landmarks.

The original roaring pose has ear-tip pixels on the top row of its 64x64 cell. The audit's top-edge warning is retained and the original pose was inspected rather than cropped or scaled to suppress it.

Static native-size, enlarged, light/dark and RGB555 proofs were inspected. GIF frame counts and durations are verified: front 450/150 ms, walking 180/180 ms and icons 250/250 ms. Timed visual playback could not be independently checked because the browser automation helper timed out; the actual live renders were verified against the exported pixels. Live rendering reports no errors.

The approved set is installed in `graphics/pokemon/mightyena/virelia/` through `tools/virelia/prepare_canine_v5_assets.py`. Battle Y offsets match the new standing front and back. The installer preserves the existing blue shiny coat and leaf ramp in the new palette order, retaining the approved mint aura and facial accents. The body, leaves and aura remain editable in their native sources. No manual emulator playtest is recorded.

## Re-export and native checks

Export the edited native sources through diivi to exports/front-rgba.png, back-rgba.png, overworld-rgba.png and icon-rgba.png, keeping their fixed layouts. Then run:

~~~text
python art/virelia_mightyena_v5/export_set.py
bash art/virelia_mightyena_v5/validate-native.sh
python art/virelia_mightyena_v5/validate_native.py
~~~

The packer uses Pillow and only packs, indexes and proofs the existing drawing. Its first-frame guard compares against front-diivi/front.png to prevent accidental changes to the approved artwork. The native shell check expects the repository's Linux build environment.

npm format and npm lint were attempted; npm is unavailable on this PC. No JavaScript files were modified.
