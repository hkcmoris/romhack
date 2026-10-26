# Virelian Poochyena v3 candidate

This is a regional edit using the repository's original GBA Poochyena anatomy,
with new pixel details drawn through Aseprite MCP. It is not a wholly scratch-
drawn sprite. The first standalone scratch study was rejected and is retained
as `source/rejected-scratch-study.aseprite` for an honest record of the process.

The game has been restored to its pre-v2 assets. V3 is a separate review
candidate in `game-ready/`; it is not installed into the runtime graphics.

## Study and direction

I read the [Cave of Dragonflies spriting guide](https://www.dragonflycave.com/spriting-guide/),
including Scratch Sprites, Outlining, Shading and the actual fur illustrations.
The working rules were: clean one-pixel contours, softer lit outlines, one
upper-left light, small highlights, and restrained jagged transitions following
hair direction. The guide's example sprites were studied, not incorporated.

V2's independently constructed anatomy did not pass visual review. V3 keeps
the original GBA poses and adds smaller leaf tufts, a leafy tail accent and
restrained coat texture. The source remains identifiable as Poochyena rather
than introducing another generic canine silhouette.

## Contents

- `source/front.aseprite`: two 64x64 poses with independently editable regional layers.
- `source/back.aseprite`: the matching GBA rear view with leaf-tipped fur.
- `source/overworld.aseprite`: six 32x32 cells, down/down step/up/up step/left/left step.
- `source/icon.aseprite`: two 32x32 party-icon frames.
- `exports/`: eleven native-size MCP exports and a full-color icon sheet.
- `game-ready/`: six 4-bit indexed PNGs and four 16-entry JASC palettes.
- `previews/review.png`: pre-v2/v2/v3 comparison, complete candidate set and shiny view.
- `previews/front.gif`: the two battle poses in motion.
- `authoring.json`: palette, donor mappings and regional pixel-row records.
- `verification.json`: current file hashes, layout checks and proposed species offsets.

The normal palette uses sage fur, charcoal muzzle/paws and fern leaves. Shiny
colors use blue-gray fur and violet leaves. The runtime icon candidate is
mapped to unchanged shared icon palette 1; its full-color source is preserved.
Right-facing walking poses use the engine's normal mirroring. Ordinary
Poochyena's footprints and cry can continue to be used.

The anatomy donors are the existing `graphics/pokemon/poochyena/` files
`anim_front_gba.png`, `back_gba.png`, `overworld.png`, and `icon_gba.png`.
These retain their original Pokemon/Game Freak attribution and the project's
existing asset-credit conventions. No illustration from the guide is included.

## Repeatable exports

Export edited Aseprite frames into `exports/` using the existing filenames,
then run `python art/virelia_poochyena_v3/export_assets.py` with Pillow available.
This packs existing pixels without resizing or quantizing the battle/overworld
art. Only the party icon is remapped to its required shared palette.

Run `bash art/virelia_poochyena_v3/validate-native.sh` in the Linux build
environment, followed by `python art/virelia_poochyena_v3/validate_native.py`
with Pillow to check exact GBA tile and palette round trips.

The working files use RGB because this MCP server's row-painting tool does
not reliably write the intended colors into indexed layers. Final game files
are explicitly packed as indexed images with transparent index zero.

No ROM build or emulator playtest was performed. Project npm formatting and
lint commands are unavailable because this C/GBA repository has no package.json.
