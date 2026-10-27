# Virelia regional starters: playable prototype v0.1

The [v0.2 prologue](VIRELIA_PROLOGUE.md) adds the normal story start and corrects
first-stage Virelian Poochyena to Grass/Dark. The evolved stages stay Grass/Ghost;
their existing IDs and level-18/38 evolution requirements are preserved.

## Scope and branch

This milestone adds nine playable species to the Emerald prototype on
`pokeemerald-expansion` 1.17.1. Work is isolated on `codex/virelia-starters`,
branched from the fully passing Duo commit `88804db31be54312220797aa0f13e52d32b40cad`.
The original `codex/duo-link-poc` branch and its working checkout are not modified.

The existing Duo quick start and Link Multi test remain available. This milestone
adds no Virelia maps, story, signature moves, new abilities, Mega Evolutions, or
final artwork. The Fire/Poison family is provisional and may be replaced.

## Species and evolution chains

The replacement line occupies IDs 1576-1578 in the custom-species range.
Older saves are unsupported during prototype development. The grass family's first stage differs from its evolutions.

| Species identifier | ID | Types | Evolves into | Required level | National dex |
| --- | ---: | --- | --- | ---: | ---: |
| `SPECIES_POOCHYENA_VIRELIA` | 1573 | Grass/Dark | Virelian Mightyena | 18 | 261 |
| `SPECIES_MIGHTYENA_VIRELIA` | 1574 | Grass/Ghost | Wraithena | 38 | 262 |
| `SPECIES_WRAITHENA` | 1575 | Grass/Ghost | Final stage | — | 1026 |
| `SPECIES_NATU_VIRELIA` | 1576 | Water/Psychic | Virelian Xatu | 14 | 177 |
| `SPECIES_XATU_VIRELIA` | 1577 | Water/Psychic | Nimbatu | 34 | 178 |
| `SPECIES_NIMBATU` | 1578 | Water/Psychic | Final stage | — | 1027 |
| `SPECIES_EKANS_VIRELIA` | 1579 | Fire/Poison | Virelian Arbok | 16 | 23 |
| `SPECIES_ARBOK_VIRELIA` | 1580 | Fire/Poison | Infernabra | 36 | 24 |
| `SPECIES_INFERNABRA` | 1581 | Fire/Poison | Final stage | — | 1028 |

Infernabra now uses `SPECIES_INFERNABRA` and `NATIONAL_DEX_INFERNABRA`
throughout the engine; no obsolete compatibility alias remains.
Regional forms retain the short original species names in the game's name fields;
use their types, description, or the test NPC's family selection to identify them.
The form tables pair each original with its regional variant, with the original
first. Final evolutions are independent species, not forms of the original second
stages. No automatic form-change mechanic is added.

Original Poochyena still evolves into ordinary Mightyena at level 18, Natu
into ordinary Xatu at level 25, and Ekans into ordinary Arbok at level 22.
Ordinary Spinarak and Ariados retain their original data and evolution.
The original second stages still have no further evolution. Regional evolution
tables only target their regional chain. Native breeding traces those chains
back to their regional first stages.

## Initial balance

Stats below use HP / Attack / Defense / Special Attack / Special Defense / Speed.
These are base stats, not the calculated stats shown on an individual Pokemon.

| Species | HP | Atk | Def | SpA | SpD | Spe | BST |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| Virelian Poochyena | 50 | 65 | 45 | 40 | 45 | 70 | 315 |
| Virelian Mightyena | 65 | 90 | 60 | 50 | 60 | 100 | 425 |
| Wraithena | 78 | 112 | 82 | 65 | 85 | 108 | 530 |
| Virelian Natu | 55 | 35 | 60 | 55 | 55 | 40 | 300 |
| Virelian Xatu | 75 | 45 | 80 | 75 | 75 | 55 | 405 |
| Nimbatu | 94 | 58 | 100 | 102 | 98 | 78 | 530 |
| Virelian Ekans | 45 | 40 | 45 | 70 | 45 | 65 | 310 |
| Virelian Arbok | 60 | 55 | 60 | 90 | 65 | 85 | 415 |
| Infernabra | 78 | 72 | 75 | 119 | 88 | 98 | 530 |

| Family | Normal ability | Hidden ability | Intended role |
| --- | --- | --- | --- |
| Grass/Dark → Grass/Ghost | Overgrow | Infiltrator | Fast physical attacker; evasion and recovery |
| Water/Psychic | Torrent | Water Absorb | Bulky special attacker; Water and Psychic coverage |
| Fire/Poison | Blaze | Corrosion | Fast special attacker; status pressure |

All nine use Medium Slow growth, a 12.5% female ratio, 20 egg cycles, and catch
rate 45. Egg groups are Field (grass), Flying (water), and Field/Dragon (fire).
The water and fire families share their preliminary level-up learnsets. The
grass first stage uses a Dark-type opening move; evolved grass stages retain
their Ghost-type opening move.
Native creation initializes the last four eligible moves, normal PP, Trainer
identity, and valid Pokemon data; granting a high-level first stage does not
skip its evolution chain.

### Grass family learnsets (updated in v0.2)

| Level | Poochyena (Grass/Dark) | Mightyena / Wraithena (Grass/Ghost) |
| ---: | --- | --- |
| 1 | Leafage, Growl | Leafage, Astonish |
| 5 | Bite | Growl |
| 9 | Shadow Sneak | Shadow Sneak |
| 12 | Razor Leaf | Razor Leaf |
| 15 | Leech Seed | Leech Seed |
| 18 | Crunch | Shadow Claw |
| 22 | Double Team | Double Team |
| 26 | Seed Bomb | Seed Bomb |
| 30 | Synthesis | Synthesis |
| 34 | Foul Play | Phantom Force |
| 40 | Leaf Blade | Leaf Blade |
| 46 | Leech Life | Leech Life |
| 50 | Swords Dance | Swords Dance |

### Water/Psychic learnset

| Level | Move |
| ---: | --- |
| 1 | Water Gun, Confusion |
| 5 | Growl |
| 10 | Water Pulse |
| 14 | Psybeam |
| 18 | Icy Wind |
| 22 | Brine |
| 26 | Extrasensory |
| 30 | Calm Mind |
| 34 | Surf |
| 38 | Psychic |
| 42 | Tailwind |
| 46 | Hydro Pump |
| 50 | Protect |

### Fire/Poison learnset

| Level | Move |
| ---: | --- |
| 1 | Ember, Acid |
| 5 | Smokescreen |
| 10 | Incinerate |
| 14 | Acid Spray |
| 18 | Will-O-Wisp |
| 22 | Flame Burst |
| 26 | Venoshock |
| 30 | Glare |
| 34 | Flamethrower |
| 38 | Sludge Bomb |
| 42 | Nasty Plot |
| 46 | Heat Wave |
| 50 | Toxic |

Teachable moves temporarily reuse each source species' existing compatibility
list; finals reuse the original second stage's list. These lists need a later
balance pass. No new TM or egg move is introduced.

## Battle graphics and reused assets

All three starter families use custom battle graphics, normal/shiny palettes
and party icons, including their final stages Wraithena, Nimbatu and Infernabra.
Each `anim_front.png` contains two vertically stacked 64x64 frames (a 64x128
sheet); each `back.png` is 64x64. Shiny forms use the same pixel indices with
a different palette. The grass family's shiny palette colors its sage fur
deep blue. The fire family uses purple scales and a tan underside; its shiny
palette changes the purple scales to charcoal while keeping the warm lava colors.
The Water/Psychic family uses the approved aquatic Natu/Xatu/Nimbatu art.
Dedicated shiny colors are pending; shiny forms currently use the normal palette.

| Species | Battle graphics directory |
| --- | --- |
| Virelian Poochyena | `graphics/pokemon/poochyena/virelia/` |
| Virelian Mightyena | `graphics/pokemon/mightyena/virelia/` |
| Wraithena | `graphics/pokemon/wraithena/` |
| Virelian Natu | `graphics/pokemon/natu/virelia/` |
| Virelian Xatu | `graphics/pokemon/xatu/virelia/` |
| Nimbatu | `graphics/pokemon/nimbatu/` |
| Virelian Ekans | `graphics/pokemon/ekans/virelia/` |
| Virelian Arbok | `graphics/pokemon/arbok/virelia/` |
| Infernabra | `graphics/pokemon/infernabra/` |

Virelian Poochyena and Mightyena use the approved v5 battle, walking and party
icon sets. Editable Aseprite sources and review exports are in
`art/virelia_poochyena_v5/` and `art/virelia_mightyena_v5/`. Poochyena's walking
sheet comes from `art/virelia_poochyena_v5/diivi-overworld/`, which supersedes
its older `game-ready/overworld.png`. Mightyena retains its original GBA
open-mouth roaring pose in front frame 2, broad pointed leaf fur and mint wisps.
The older Poochyena v2 and v3 candidates remain in their art folders.

Run `python tools/virelia/prepare_canine_v5_assets.py` after updating those
approved exports to install the runtime files. It copies the battle pixels and
walking sheets, maps Poochyena's icon to shared palette 1 by material role, and
uses Mightyena's prepared shared-palette icon. It keeps the existing blue shiny
coat and leaf ramp in the new index order; eyes, noses, teeth and ghost wisps
retain their approved colors. Battle and walking normal/shiny palettes match.
Poochyena's front/back bounds are 48x42/47x42 with Y offset 11; Mightyena's
standing front/back bounds are 58x56/60x56 with Y offset 4. The roaring frame
reaches the top row, as in the original GBA pose.
The fire family follows the same original-sprite-first approach with Ekans
and Arbok, then adds the concept's volcanic cracks, flames and hood markings.
Infernabra has a larger, rocky hood and its own final-stage silhouette.
The water family starts from recolored original Natu and Xatu sprites,
then adapts them to the aquatic concept. Nimbatu has its own battle silhouette.
Fire-family back sprites follow the original GBA battle framing: Ekans shows
its rear head and upper neck; Arbok and Infernabra show their rear head and hood.
They do not include the full coiled body or tail. Water-family back sprites show the head, shoulders and folded wings.
Each runtime icon is a 32x64 indexed sheet with two 32x32
animation frames. The grass icons use the existing green icon palette (index 1);
the water icons use palette 3, and the fire icons use the existing purple/orange
icon palette (index 5).

Fire-family artwork was prepared with the built-in image generator: recolor the
original Ekans and Arbok sheets first, then adapt them using the supplied concept.
The local authoring sheets and exact prompts are saved under
`outputs/virelia/fire-sprites/`; they are not needed to compile the game.
`python tools/virelia/build_fire_sprite_assets.py` re-exports the local sheets
and standalone corrected back views into indexed 4-bit PNGs and 16-entry palettes. It checks transparency, palette
indices, frame differences and canvas bounds, and writes a scaled review sheet
and exact sprite bounds to the same output directory.

Water-family artwork follows the same recolor-first, concept-adaptation process.
Its authoring sheets and prompts are under `art/virelia_natu_line/`.
After refreshing the art exports, run `python tools/virelia/prepare_natu_assets.py`
to produce `icon_game.png` using shared icon palette 3. The original `icon.png`
retains the full art palette. Battle art is loaded directly from the prepared PNGs.

All nine stages also have dedicated overworld walking sheets and normal/shiny
palettes in their battle-art directories. Each sheet has six 32x32 cells ordered
down A/B, up A/B and left A/B; east-facing movement mirrors left. The canine v5
sheets use their approved native-grid diivi art. The remaining stages adapt the
original walking frames with regional details: flowing tails, aquatic crests
and plumage, or volcanic seams and flame tails. The final
stages have distinct silhouettes. The water family's shiny overworld palette
matches its current normal battle colors until dedicated shiny colors are made.
Ordinary species retain their original graphics and palettes.

Footprints and cries continue to reference the sources below.

| Prototype stages | Reused asset source |
| --- | --- |
| Virelian Poochyena | Poochyena |
| Virelian Mightyena, Wraithena | Mightyena |
| Virelian Natu | Natu |
| Virelian Xatu, Nimbatu | Xatu |
| Virelian Ekans | Ekans |
| Virelian Arbok, Infernabra | Arbok |

Wraithena, Nimbatu and Infernabra each have their own final-stage battle art.
Height, weight and display scale remain
provisional: Wraithena 1.0 m/37.0 kg, Nimbatu 1.9 m/26.0 kg, Infernabra
3.5 m/65.0 kg.

## Development access and testing

`VIRELIA_STARTER_TEST` in `include/config/general.h` explicitly enables the
development NPC for this prototype. Set it to numeric `0` to remove test access;
this does not delete already obtained Pokemon or remove their species data.
Access is Emerald-only and is separate from `ORACULUM_DUO_TEST_QUICK_START`.
The feature's data configuration requires the full Generation 9 dex and the
original Poochyena, Natu and Ekans families. FireRed and LeafGreen retain
normal species and dex behavior and do not expose the test NPC.

1. For the current v0.2 code, build with `VIRELIA_DEVELOPMENT_MODE=1` before
   following this prototype checklist. The default release now starts the
   [story prologue](VIRELIA_PROLOGUE.md). A historical v0.1 prototype ROM also
   supports the checklist below.
2. Choose **NEW GAME**, select gender and name, then finish the existing Duo team
   prompt. The quick start places you at Littleroot `(17, 13)`.
3. Speak to the separate scientist at `(18, 12)`, immediately east of the Duo
   guide at `(17, 12)`. Approach from `(18, 13)`.
4. Choose a family, a stage, and a level. Available levels are **5, 10, 14, 16, 18,
   20, 30, 34, 36, 38, 40, 50**. Scroll the menu to see later entries.
5. Open **POKEMON → SUMMARY** to inspect name, types, calculated stats, ability,
   ownership and moves. Test each STAB move in an ordinary battle.
6. Use the NPC's Rare Candy option to obtain evolution-testing supplies. Use
   candies from the Bag to trigger the normal level-up and evolution sequence.

Grants append to the existing party. A full party is refused without replacing,
deleting, or silently boxing a Pokemon. Deposit a party member through the normal
PC interface before requesting more. Cancelling a menu does not grant a Pokemon.
Rare Candies use the normal bag-capacity check and can be requested again.

### Exact evolution boundary checks

Do not give an Everstone to the Pokemon being tested. Evolution requires a normal
level-up event; creating a Pokemon at or above its threshold does not immediately
evolve it. Cancel with **B** only when deliberately testing cancellation.

| Starting selection | Raise to just below boundary | Next level must produce |
| --- | --- | --- |
| Virelian Poochyena, level 10 | Level 17 (7 candies) | Level 18: Virelian Mightyena |
| Virelian Mightyena, level 30 | Level 37 (7 candies) | Level 38: Wraithena |
| Virelian Natu, level 10 | Level 13 (3 candies) | Level 14: Virelian Xatu |
| Virelian Xatu, level 30 | Level 33 (3 candies) | Level 34: Nimbatu |
| Virelian Ekans, level 10 | Level 15 (5 candies) | Level 16: Virelian Arbok |
| Virelian Arbok, level 30 | Level 35 (5 candies) | Level 36: Infernabra |

After each evolution, recheck the two types and moves. Also create first stages
at the exact threshold and one of the offered higher levels: they should retain
the selected stage until the next eligible level-up. Test all three stages
through party-to-PC deposit, withdrawal, saving, emulator reset and Continue.

Trading uses the ordinary species/ownership data path. Both peers must run the
same feature ROM; old ROMs and FireRed/LeafGreen builds do not define these new
species. Do not trade them to a ROM that lacks their IDs. No link protocol,
Link Battle, or Battle Tower code is changed.

## Engine adaptations and playtest save policy

- New IDs 1573–1581 fit the existing 11-bit stored species field (maximum 2047).
  Native `Pokemon` and `BoxPokemon` layouts and encryption remain unchanged.
- Regional forms use the real `formSpeciesIdTable` API and inline `EVOLUTION`
  definitions. No unsupported custom region flag or invented evolution method is
  added. The final stages have separate National Dex entries and sort entries.
- The current full dex has 1025 entries. Adding three creates 1028 entries;
  `(NATIONAL_DEX_COUNT + 1)` rounds to **129 flag bytes** in both cases. Existing
  SaveBlock sizes therefore do not need to change. The standard save-layout
  regression expectations are retained.
- `USE_DEXNAV_SEARCH_LEVELS` remains disabled as in the base. Enabling its
  per-species saved array is a separate save-layout decision, not part of this
  milestone. Reduced-generation or disabled source-family configurations disable
  the prototype data rather than silently expanding their dex save allocation.
- Event scripts do not import the C Boolean macros. The new NPC script resolves
  its species/configuration guard to a numeric value in a temporary, local
  Boolean context, then restores the previous macros. This keeps the guide
  enabled when its species exist without changing unrelated script conditions.
- No new persistent development flags or variables are allocated. NPC visibility
  uses a map-local temporary flag; the inherited quick start applies to new games.
- Before the stable release, discard saves from previous playtest versions and
  choose **NEW GAME** whenever switching versions. Cross-version save
  compatibility is not promised, even when serialized structure sizes match.
  Saving and loading within the current build must still preserve Pokemon,
  items and progression correctly.

## Planned mechanics: not implemented

- **Forest Ambush:** two-turn physical Grass move, 85 power, 100 accuracy, 10 PP;
  hide on turn one, attack on turn two, recover 50% of damage dealt.
- **Tidal Web:** physical Water move, 70 power, 100 accuracy; after a successful
  hit, lower the target's Speed by one stage.
- **Water-family signature ability:** bonus Water damage against targets whose
  Speed has been lowered. No multiplier or custom ability is implemented.
- **Burning Venom:** Burn plus a temporary additional poison-like damage effect.
  No simultaneous major-status behavior is implemented. Current moves retain
  their ordinary engine status rules.

## Validation and remaining work

The native regression suite in `test/virelia_starters.c` covers species metadata,
evolution boundaries and original paths, learnsets, normal creation/ownership,
regional breeding roots, PC data and save/load persistence, and safe test grants.
The unchanged standard save tests check structure sizes. The existing CI workflow
also builds Emerald development/release, FireRed and LeafGreen and validates docs.
ROM compilation is performed only on GitHub Actions; no local toolchain is added.

A successful CI run validates compilation and automated behavior. Manual visual
checks of the NPC menus, summary pages, battle sprites, and evolution animations
remain a playtest task; no completed emulator playthrough is claimed here.

Remaining work: starter-vs-starter and campaign balance, per-stage move pacing,
TM/egg-move review, remaining working names, custom footprints, cries and
overworld graphics for all families, final physical
dimensions and dex prose, signature mechanic design/testing, and a future
Virelia regional dex.
