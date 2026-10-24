# Whisperwood's hidden sanctuary

The western Zigzagoon detour now leads to an optional forest sanctuary after
Hana's existing quest. It adds exploration and a small folklore reward without
changing the main story, trainer teams, route encounters or starter progression.

## Discovery and puzzle

1. Find Zigzagoon at `(3, 16)` on Whisperwood Trail and inspect the Wurmple nest.
2. Return to Hana in Maplebrook and accept her Potion. If the bag is full, make
   space and speak to her again. Her new dialogue points behind the nest.
3. Return to Zigzagoon. Branches at the western edge now reveal an overgrown
   path. Follow it south, west and north into the hidden clearing.
4. Inspect the enormous pawprints and the ancient tree. Its inscription gives
   the order: **seed, rain, root**. The tracks disappear into the roots; the
   creature's identity remains unknown.
5. Press the seed stone at `(6, 8)`, rain stone at `(12, 8)`, then root stone at
   `(9, 12)`. Each correct press sounds a tone. A wrong press resets the whole
   unfinished sequence. Declining the prompt only inspects the stone.
6. The roots at `(16, 2)` and `(16, 3)` withdraw. Walk northeast to return to the
   main trail through its western opening at rows `8` and `9`.
7. Inspect the ancient tree again to collect **Soothe Bell** and read the short
   Virelian story of the guardian sleeping beneath the forest. The inscription
   remains readable. A full bag leaves the bell in place for a later visit.

The shortcut works in both directions and stays open after leaving, blackout,
and saving. An unfinished sequence resets on map entry. The sanctuary is a
quiet exploration space: its overgrown grass has no separate encounter table.
The original route still has its existing encounters, two trainers and Potion.

## Map and assets

- Map: `Virelia_WhisperwoodSanctuary`, appended to VireliaOverworld as group `75`, map `3`.
- Layout: `LAYOUT_VIRELIA_WHISPERWOOD_SANCTUARY`, appended without moving IDs.
- Size: `22 × 26`; shares Whisperwood's existing region section.
- West connection from Route 01: offset `6`; east return: offset `-6`.
- The long approach joins route rows `16–17`; the shortcut joins rows `8–9`.
- Both maps keep passable raw seam cells. Gates sit one tile inward so the
  engine's connected-map buffer can copy the neighbor before its load callback.
- Saved flags: `FLAG_VIRELIA_SANCTUARY_SHORTCUT_OPEN` (`0x3B`) and
  `FLAG_VIRELIA_SANCTUARY_BELL_RECEIVED` (`0x3C`). Existing save sizes stay fixed.
- Unfinished stone progress: temporary variable `0x400E`.

The sanctuary uses General plus the shared VireliaOutdoor secondary tileset,
also used by Maplebrook, Whisperwood Trail and Slateford. Shared IDs, graphics
and palettes keep neighboring grass and buildings correct at seamless crossings.
The ancient tree is an original **48 × 64 pixel** sprite, split into **3 × 4 map
tiles** at `(8, 6)` through `(10, 9)`. Its twelve contiguous metatiles are
`0x31D–0x328`, arranged row by row. One continuous canopy, gnarled trunk and
spreading roots replace the earlier stock tree assembly. The native asset is
`data/tilesets/secondary/whisperwood/old_tree.png`: 15 colors plus transparency,
with a dedicated day palette and night variant. Map loading restores all twelve
tiles over older cached map views; the tree interaction stays at `(9, 9)`.

The generated master and its exact ImageGen prompt are retained under
`art/whisperwood/`. `tools/virelia/build_whisperwood_tree.py` converts the master
to the native sprite using nearest-neighbor sizing and GBA color reduction.
The sanctuary source builder deduplicates identical stock 8 × 8 pixel patterns before
packing the new artwork, preserving every Fortree metatile and keeping the
secondary bank within 512 tiles. The existing footprint and carved stone IDs
stay fixed. The paw impression reuses the native Arcanine footprint graphic.
The outdoor packer then combines the used source metatiles with town artwork,
preserving their pixels and behavior while assigning unique shared IDs. The
ancient tree keeps IDs `0x31D–0x328` and uses shared palette 12.

The surrounding forest is painted with complete 2 × 3 trees after paths are
carved; narrow fringes and gates use standalone shrubs. Shared tilesets and
the source footprint are unchanged. Carved stones use the existing forest
palette. Soothe Bell's bag description is included in Czech localization.

`tools/virelia/build_whisperwood_sanctuary.py` rebuilds these checked-in assets
with Python and Pillow. It patches only the eight western route cells belonging
to the two entrances, preserving other Route 01 edits and actors. It does not
regenerate the older towns or layouts. Porymap edits within the new sanctuary
will be replaced if this builder is run again.

## Validation and playtest

```sh
python3 tools/validate_virelia_sanctuary.py
python3 tools/validate_virelia_world.py
python3 tools/validate_virelia_tilesets.py
python3 tools/validate_virelia_prologue_text.py
make -j4 check TESTS="Virelia sanctuary"
```

The sanctuary validator checks the Hana gate, interactions, closed and open
shortcut states, return paths, reciprocal connections and native asset bounds.
Native tests run the real compiled stone and reward helpers, all 27 three-stone
sequences, callback collisions, retry behavior and FLASH save/load with the bell
both unclaimed and claimed. The existing world tests cover the new map header
and horizontal connection.

In an emulator, also check both seamless west/east crossings, the branch hint
before and after Hana, canceling stone prompts, the wrong-order sound, opening
the roots, a full bag, giving the bell to a Pokémon and returning after reload.
Old saves that already accepted Hana's Potion should reveal the path on their
next Route 01 load. Check that ordinary route trainers and the eastern Potion
remain accessible throughout.

The earlier uncommitted playtest ROM placed the sanctuary at `76,7`. For that
prototype, leave the sanctuary and save on Route 01 before loading this corrected
build, which uses `75,3`. All maps predating the sanctuary retain their IDs.
