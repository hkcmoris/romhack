# Whisperwood old tree

The original master was generated with the built-in ImageGen tool for the
hidden forest sanctuary. Its exact prompt is saved in `old-tree-prompt.txt`.
The current native tree reuses that master's gnarled trunk and root silhouette,
with a canopy assembled from existing Emerald forest artwork.

- `old-tree-source.png`: original generated artwork, preserved unchanged.
- `../../data/tilesets/secondary/whisperwood/old_tree.png`: native 80 × 80 sprite,
  representing five columns and five rows of 16 × 16 map tiles.
- `../../tools/virelia/build_whisperwood_tree.py`: deterministic native
  composition and 15-color palette; run it before `build_tileset()` in the
  sanctuary builder when changing the tree artwork. Do not run the sanctuary
  layout builder to update art on maps edited in Porymap.

The broad canopy uses Emerald General foliage fragments at their original
pixel scale. Donor ground shadows are excluded so the tree has one contact
shadow beneath its spreading roots. The trunk retains five bark tones, with
isolated color speckles removed. Its palette combines four stock foliage greens, six stone
colors, and five bark colors so the shrine's mossy stones and paw impressions
can share the same dedicated palette.

The final sprite uses transparent palette index zero. The tileset builder
packs the sprite and its day/night palette into the dedicated Whisperwood
secondary tileset. Existing stock tile art is preserved through deduplication.
