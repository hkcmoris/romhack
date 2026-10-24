# Virelian Natu line - sprite preparation v4

Normal-color artwork prepared from the repository's Natu and Xatu sprites using the built-in imagegen tool. These assets now supply the playable Water/Psychic starter line: Natu → Xatu at level 14 → Nimbatu at level 34.

## Final assets

| Species | Directory |
| --- | --- |
| Virelian Natu | graphics/pokemon/natu/virelia/ |
| Virelian Xatu | graphics/pokemon/xatu/virelia/ |
| Nimbatu | graphics/pokemon/nimbatu/ |

Each directory contains front.png and front_2.png (64x64 each), anim_front.png (64x128), back.png (64x64), icon.png (32x64, two frames), and normal.pal (16-entry JASC palette).

All art PNGs are indexed, have transparent palette index 0, and share the same palette within one species. The separate runtime icon_game.png files use shared icon palette 3. Each complete art sheet uses 14 colors including transparency. Existing original Natu/Xatu files were preserved.

## Source and edit stages

1. 01_originals/natu and 01_originals/xatu contain unchanged source copies, including the original battle sprites, icons, footprint and overworld references.
2. 02_recolored contains the aquatic palette passes. nimbatu.png is an exact byte-for-byte copy of the recolored xatu.png.
3. 03_concept_edits contains the concept-detail passes. Natu received the paired crest, cheek teardrop and pale belly. Xatu received the crest, navy collar, gold chest marking and tail accent. Nimbatu developed from the copied Xatu with broader wings, gold crest and longer tail.
4. Native export removed translucent presentation halos, packed the cells onto the game grid and encoded consistent palette indices. Nimbatu's front poses received an additional alignment pass.

The raw imagegen sheets are edit history. The engine uses the native files in graphics/pokemon.

## Review

preview.png shows the actual exported files, enlarged with nearest-neighbor sampling. Its columns are front frame 1, front frame 2, back and both party-icon frames. previews/ also contains individual species sheets and animated front-frame GIFs.

imagegen-prompts.json records the final edit prompts and workflow constraints. asset_manifest.json records dimensions, hashes, palette counts and source lineage. export_assets.py performs palette-index normalization and cell packing from the already edited native-grid sheets.

## Integration boundary

The engine registers all three species and activates Natu in the lab. Normal-color battle sprites and party icons are prepared. Dedicated shiny artwork/palettes and edited follower sheets are not included in this pass; copied overworld files remain source references.

The original icon.png files retain each family's custom normal.pal colors. The engine uses separate icon_game.png files mapped to shared palette 3 by tools/virelia/prepare_natu_assets.py. Shiny battle sprites currently reuse normal colors pending dedicated shiny artwork; footprints, cries and followers reuse ordinary Natu/Xatu.

No ROM build, CI run or emulator playtest was started for this artwork pass. Checks covered dimensions, palette-index consistency, binary transparency, distinct front frames, unchanged original source files and the exact Xatu-to-Nimbatu recolor copy.

## v2 correction

Xatu front frame 2 now exposes navy/teal body plumage with gold belly chevrons. The leftover folded-wing panels were removed. Only the central torso pixels in front_2.png and the corresponding lower half of anim_front.png changed. The idle frame, raised outer wings outside the edit rectangle, back sprite, icons, palette and both other species were preserved. See 04_xatu_wing_fix/verification.json for the exact bounds and unchanged-file checks.

## v3 correction

Nimbatu back.png now uses a close battle portrait: head, neck, shoulders and upper wings. The lower body, legs, feet and tail are outside the bottom crop. The 64x64 cell, 16-entry palette and transparent index 0 are preserved. All other native sprite files are byte-identical to v2. See 05_nimbatu_back_framing/verification.json and comparison.png. Artwork was edited with built-in imagegen; the exact prompt is recorded in imagegen-prompts.json.

## v4 correction

Nimbatu back.png now keeps both wings folded against its body. Its head, neck and shoulders retain the close battle framing. The outer silhouette clears both side edges, avoiding clipped spread wings. The torso crop meets the bottom edge. The 64x64 cell, 16-entry palette and transparent index 0 are preserved; all other native assets are byte-identical to v3. See 06_nimbatu_folded_back/verification.json and comparison.png. Built-in imagegen was used; the exact prompt is in imagegen-prompts.json.
