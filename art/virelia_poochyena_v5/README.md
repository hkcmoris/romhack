# Virelian Poochyena: approved design sprite set

This set extends the approved high-resolution design and corrected front sprite.
The approved front is unchanged, including its mouth/teeth and tail/haunch separation.
New views retain the green leaf-shaped fur, pointed canine ears, charcoal
muzzle/paws, red nose, yellow/red eyes and layered conical leaf-fan tail.

## Finished assets

| File in game-ready/ | Layout |
| --- | --- |
| front.png | Approved front, 64x64 |
| front_2.png | Head/breath and leaf follow-through, 64x64 |
| anim_front.png | Both front frames stacked, 64x128 |
| back.png | Rear three-quarter view, 64x64 |
| overworld.png | Six 32x32 cells horizontally, 192x32 |
| icon.png | Two 32x32 frames stacked, 32x64 |
| normal.pal / overworld_normal.pal | Identical approved 16-entry palettes |

Overworld order is down 1, down 2, up 1, up 2, left 1, left 2.
Right-facing walking uses the engine's normal horizontal mirroring.
Walking alternates paw contact; the party icon moves down one pixel.

All assets use the exact approved palette: fifteen opaque colors plus
transparent index zero. The custom icon palette is preserved; it has not been
remapped to the game's shared icon palette. Runtime installation would need
to assign that palette appropriately.

The front occupies 48x42 pixels at (8,11) in its 64x64 cell.
Front 2 stays within that footprint; the back occupies 47x42 at (9,11).
Walking sprites fit within 24x24 inside 32x32 cells, and icons within 24x21.
Transparent margins remain visible in the review sheet.

## Review and editable sources

- previews/review.png: all eleven frames, with full cell boundaries.
- previews/front.gif: two front poses.
- previews/walking.gif: down, up, left and mirrored right in motion.
- previews/icon.gif: party icon in motion.
- source/front.aseprite: two layered front frames.
- source/back.aseprite: back and native detail layer.
- source/overworld.aseprite: six walking frames with direction tags.
- source/icon.aseprite: two icon frames.
- source/overworld-{down,up,left}.aseprite: individual walking sources.
- exports/: eleven native-size Aseprite MCP exports.
- references/: matching high-resolution view studies and prompts.
- set-verification.json: dimensions, frame bounds, palette and hashes.
- native-verification.json: GBA conversion round-trip results.

Back/up/left reference studies were generated from the approved concept,
then constructed on the native grid in Aseprite. Facial, paw and overlap
details were edited through Aseprite MCP. The forward overworld view was
drawn directly in Aseprite. The front is the approved pixel design.

## Export and validation

Export editable frames through Aseprite MCP to exports/, then run
export_set.py with Pillow. It only packs existing pixels into indexed PNGs,
checks them and makes enlarged presentation previews. It does not quantize
or resize actual assets. The original front file is copied byte for byte.

validate-native.sh runs the repository's gbagfx converter on six sheets and
two palettes. validate_native.py checks exact indices, colors, RGB555 words
and expected binary sizes after conversion.

This is a separate review set; runtime graphics have not been replaced.
No ROM build or emulator playtest was performed. The custom icon palette
needs integration before using that icon in the game.

