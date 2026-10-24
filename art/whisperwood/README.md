# Whisperwood old tree

Generated with the built-in ImageGen tool for the hidden forest sanctuary.
The exact generation prompt is saved in `old-tree-prompt.txt`.

- `old-tree-source.png`: original generated artwork, preserved unchanged.
- `../../data/tilesets/secondary/whisperwood/old_tree.png`: native 48 × 64 sprite,
  representing three columns and four rows of 16 × 16 map tiles.
- `../../tools/virelia/build_whisperwood_tree.py`: deterministic native-size and
  15-color conversion; run before the sanctuary layout builder when changing
  the master artwork.

The final sprite uses transparent palette index zero. The tileset builder
packs the sprite and its day/night palette into the dedicated Whisperwood
secondary tileset. Existing stock tile art is preserved through deduplication.
