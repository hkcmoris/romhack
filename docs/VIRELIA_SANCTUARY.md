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
The ancient tree is an **80 × 80 pixel** sprite, split into **5 × 5 map
tiles** at `(7, 5)` through `(11, 9)`. Its twenty-five contiguous metatiles are
`0x31D–0x335`, arranged row by row. One broad canopy, gnarled trunk and
spreading roots distinguish it from the surrounding trees. The native asset is
`data/tilesets/secondary/whisperwood/old_tree.png`: 15 colors plus transparency,
with a dedicated day palette and night variant shared by the sanctuary props.
The authored map supplies the complete tree; its interaction stays at `(9, 9)`.
All twenty-five cells block walking, while the stones and root approach remain
reachable on either side and from the south.

The generated master and its exact ImageGen prompt are retained under
`art/whisperwood/`. `tools/virelia/build_whisperwood_tree.py` retains its gnarled
trunk and root silhouette with five bark tones, then assembles the canopy from
native Emerald forest foliage. Their leaf clusters stay at their original
pixel scale. The donor trees' ground shadows are excluded from the canopy;
one contact shadow anchors the new trunk and spreading roots.
The sanctuary source builder deduplicates identical stock 8 × 8 pixel patterns before
packing the new artwork, preserving every Fortree metatile and keeping the
secondary bank within 512 tiles. The existing footprint and carved stone IDs
stay fixed. The paw impression reuses Mightyena's native separated canine toes
and heel, rendered as an earthy depression. The earlier Arcanine graphic was a
single solid silhouette that obscured the pad shapes at native resolution.
Its metatile `0x318` uses **Covered** layers (`0x1000`): grass on BG3 and the
print on BG2, beneath the player. The carved stones and ancient tree retain
Normal layers, including their foreground overlays. Changing map elevation
is not needed to keep the prints below the player.
The outdoor packer then combines the used source metatiles with town artwork,
preserving their pixels and behavior while assigning unique shared IDs. The
ancient tree uses IDs `0x31D–0x335` and shared palette 12.

The surrounding forest is painted with complete 2 × 3 trees after paths are
carved; narrow fringes and gates use standalone shrubs. Stock source assets
remain untouched. Carved stones reuse FRLG General boulder `0x09F`, with
a thin one- to two-pixel moss cap from Emerald's shrub `0x00E` along the upper
ridge. Their carved faces, sides and bases retain the exposed stock rock
facets. Tree, stones and tracks share a palette of four
forest greens, six stone colors and five bark colors. Soothe Bell's bag
description is included in Czech localization.

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

### Footprint rendering validation, 2026-10-24

WSL built `outputs/sanctuary-footprints/after/oraculum-v0.3.0-sanctuary.gba`
from the current source and the existing uncommitted sanctuary layout. That
layout was preserved byte for byte. Only the footprint's source and shared
tileset layer attributes changed; its artwork, collision and behavior did not.
The source builder now emits the same ground-layer attribute on regeneration.
Do not rerun the full sanctuary builder to apply this fix to an edited map:
it would replace Porymap layout edits.

The existing libretro mGBA harness initialized a fresh game and used native
debug warping to enter the sanctuary. Controller input crossed all three
footprints in both directions, with captures during movement and at rest.
The current layout's elevation-4 ground already masks the overlap. Separate
QA copies of both ROMs used elevation 3 for that walking plane to reproduce
the fault: Normal layers painted the print over the avatar; Covered layers
kept the full sprite visible. These QA copies did not modify the delivered
ROM or the source layout. Screenshots and logs are in
`outputs/sanctuary-footprints/`; `footprints-before-after.png` shows the
normal-ground comparison.

All seven native sanctuary tests passed, covering the new loaded-map layer
check as well as puzzle input, resets, callbacks, reward retries and FLASH
save/load. The shared outdoor tileset validator and world connectivity check
passed. The separate sanctuary path audit still reports its existing
`Sanctuary accessible before helping Hana` issue; this rendering fix does not
change those paths. `npm format` and `npm lint` were attempted, but this
C/Makefile repository has no corresponding npm commands.

The earlier uncommitted playtest ROM placed the sanctuary at `76,7`. For that
prototype, leave the sanctuary and save on Route 01 before loading this corrected
build, which uses `75,3`. All maps predating the sanctuary retain their IDs.

### Hana gate correction, 2026-10-24

The follow-up build `outputs/hana-gate/oraculum-v0.3.0-hana-gate.gba` fixes
the access issue reported above. All eight entrance/shortcut cells are closed
standalone shrubs in the raw layouts. Both maps require Hana's reward flag;
finding Zigzagoon without returning to Hana does not grant access, and a full
Bag leaves the gate closed until her Potion can be accepted. The northern
shortcut additionally requires the completed stone puzzle.

`VireliaSanctuary_UpdateConnectionGates` also updates the neighbor tiles copied
into each map's camera buffer. On seamless crossings, the engine restores that
old view after loading the new map; without matching neighbor gates it could
overwrite an open destination gate with a closed cached shrub. The same helper
updates the copied Route 01 gate immediately when the stone puzzle is solved.
This is confined to the two sanctuary connections; the field engine is unchanged.

The source audit now passes. Gate art is checked as standalone shrubs; the
complete ancient tree remains checked separately. The audit accepts stock
forest canopies that overlap or continue beyond Porymap's borders instead of
requiring every generic tree to match the generator's isolated 2 × 3 pattern.
Three raw ancient-tree canopy collision bits were also aligned with the
existing load callback; their graphics and in-game collision are unchanged.

All nine native sanctuary tests pass, including raw ROM gates, copied neighbor
gates, the real Hana reward subroutine and full-Bag retry. A fresh-game mGBA
controller test confirmed blocked access before the quest and after discovery,
then accepted the reward through Hana's actual NPC event. Both rows of the
main entrance worked in both directions afterward; both shortcut rows stayed
closed until the solved-stone checkpoint, then worked in both directions.
The fixture seeds discovery and solved-stone flags and uses native debug warps;
it is a gate playtest rather than a complete story walkthrough. Screenshots,
logs and `gate-playtest.json` are saved under `outputs/hana-gate/`.
The final build also passes the full native suite (5,512 passed, no unexpected
failures), and the three-footprint movement/rendering regression was repeated
in that same delivered ROM.
