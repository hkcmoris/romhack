# Virelia regional starters: playable prototype v0.1

## Scope and branch

This milestone adds nine playable species to the Emerald prototype on
`pokeemerald-expansion` 1.17.1. Work is isolated on `codex/virelia-starters`,
branched from the fully passing Duo commit `88804db31be54312220797aa0f13e52d32b40cad`.
The original `codex/duo-link-poc` branch and its working checkout are not modified.

The existing Duo quick start and Link Multi test remain available. This milestone
adds no Virelia maps, story, signature moves, new abilities, Mega Evolutions, or
final artwork. The Fire/Poison family is provisional and may be replaced.

## Species and evolution chains

Species IDs are appended in the engine's custom-species range. Existing playable
species IDs are unchanged. Types apply to all three stages of each family.

| Species identifier | ID | Types | Evolves into | Required level | National dex |
| --- | ---: | --- | --- | ---: | ---: |
| `SPECIES_POOCHYENA_VIRELIA` | 1573 | Grass/Ghost | Virelian Mightyena | 18 | 261 |
| `SPECIES_MIGHTYENA_VIRELIA` | 1574 | Grass/Ghost | Wraithena | 38 | 262 |
| `SPECIES_WRAITHENA` | 1575 | Grass/Ghost | Final stage | — | 1026 |
| `SPECIES_SPINARAK_VIRELIA` | 1576 | Water/Bug | Virelian Ariados | 14 | 167 |
| `SPECIES_ARIADOS_VIRELIA` | 1577 | Water/Bug | Arachnaut | 34 | 168 |
| `SPECIES_ARACHNAUT` | 1578 | Water/Bug | Final stage | — | 1027 |
| `SPECIES_EKANS_VIRELIA` | 1579 | Fire/Poison | Virelian Arbok | 16 | 23 |
| `SPECIES_ARBOK_VIRELIA` | 1580 | Fire/Poison | Cindracoil | 36 | 24 |
| `SPECIES_CINDRACOIL` | 1581 | Fire/Poison | Final stage | — | 1028 |

Wraithena and Cindracoil are temporary names; Arachnaut is a working name.
Regional forms retain the short original species names in the game's name fields;
use their types, description, or the test NPC's family selection to identify them.
The form tables pair each original with its regional variant, with the original
first. Final evolutions are independent species, not forms of the original second
stages. No automatic form-change mechanic is added.

Original Poochyena still evolves into ordinary Mightyena at level 18, Spinarak
into ordinary Ariados at level 22, and Ekans into ordinary Arbok at level 22.
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
| Virelian Spinarak | 55 | 55 | 60 | 35 | 55 | 40 | 300 |
| Virelian Ariados | 75 | 75 | 80 | 45 | 75 | 55 | 405 |
| Arachnaut | 94 | 102 | 100 | 58 | 98 | 78 | 530 |
| Virelian Ekans | 45 | 40 | 45 | 70 | 45 | 65 | 310 |
| Virelian Arbok | 60 | 55 | 60 | 90 | 65 | 85 | 415 |
| Cindracoil | 78 | 72 | 75 | 119 | 88 | 98 | 530 |

| Family | Normal ability | Hidden ability | Intended role |
| --- | --- | --- | --- |
| Grass/Ghost | Overgrow | Infiltrator | Fast physical attacker; evasion and recovery |
| Water/Bug | Torrent | Water Absorb | Bulky physical attacker; lower the opponent's Speed |
| Fire/Poison | Blaze | Corrosion | Fast special attacker; status pressure |

All nine use Medium Slow growth, a 12.5% female ratio, 20 egg cycles, and catch
rate 45. Egg groups are Field (grass), Bug (water), and Field/Dragon (fire).
Each family uses one preliminary level-up learnset across its three stages.
Native creation initializes the last four eligible moves, normal PP, Trainer
identity, and valid Pokemon data; granting a high-level first stage does not
skip its evolution chain.

### Grass/Ghost learnset

| Level | Move |
| ---: | --- |
| 1 | Leafage, Astonish |
| 5 | Growl |
| 9 | Shadow Sneak |
| 12 | Razor Leaf |
| 15 | Leech Seed |
| 18 | Shadow Claw |
| 22 | Double Team |
| 26 | Seed Bomb |
| 30 | Synthesis |
| 34 | Phantom Force |
| 40 | Leaf Blade |
| 46 | Leech Life |
| 50 | Swords Dance |

### Water/Bug learnset

| Level | Move |
| ---: | --- |
| 1 | Aqua Jet, Bug Bite |
| 5 | String Shot |
| 10 | Flip Turn |
| 14 | Pounce |
| 18 | Bulldoze |
| 22 | Aqua Tail |
| 26 | Leech Life |
| 30 | Scary Face |
| 34 | Liquidation |
| 38 | Lunge |
| 42 | Sticky Web |
| 46 | Waterfall |
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

## Battle graphics and placeholder assets

Virelian Poochyena, Virelian Mightyena and Wraithena use custom battle graphics
and normal/shiny palettes. Each `anim_front.png` contains two vertically stacked
64x64 frames (a 64x128 sheet); each `back.png` is 64x64. The shiny palettes color
the sage fur deep blue while retaining the same sprite pixels.

| Species | Battle graphics directory |
| --- | --- |
| Virelian Poochyena | `graphics/pokemon/poochyena/virelia/` |
| Virelian Mightyena | `graphics/pokemon/mightyena/virelia/` |
| Wraithena | `graphics/pokemon/wraithena/` |

The other two families still use placeholder battle sprites and palettes. Party
icons, footprints, cries and applicable overworld graphics for all three families
continue to reference these existing assets. Original image files are not
recolored or overwritten.

| Prototype stages | Placeholder asset source |
| --- | --- |
| Virelian Poochyena | Poochyena |
| Virelian Mightyena, Wraithena | Mightyena |
| Virelian Spinarak | Spinarak |
| Virelian Ariados, Arachnaut | Ariados |
| Virelian Ekans | Ekans |
| Virelian Arbok, Cindracoil | Arbok |

Arachnaut and Cindracoil consequently still look like their second stages in
battle; Wraithena has its own final-stage art. Height, weight and display scale
remain provisional: Wraithena 1.0 m/37.0 kg, Arachnaut 1.1 m/33.5 kg, Cindracoil
3.5 m/65.0 kg.

## Development access and testing

`VIRELIA_STARTER_TEST` in `include/config/general.h` explicitly enables the
development NPC for this prototype. Set it to numeric `0` to remove test access;
this does not delete already obtained Pokemon or remove their species data.
Access is Emerald-only and is separate from `ORACULUM_DUO_TEST_QUICK_START`.
The feature's data configuration requires the full Generation 9 dex and the
original Poochyena, Spinarak and Ekans families. FireRed and LeafGreen retain
normal species and dex behavior and do not expose the test NPC.

1. Open the Emerald release ROM from this branch's successful GitHub Actions run.
2. Choose **NEW GAME**, select gender and name, then finish the existing Duo team
   prompt. The quick start places you at Littleroot `(17, 13)`.
3. Speak to the separate scientist at `(18, 12)`, immediately east of the Duo
   guide at `(17, 12)`. Approach from `(18, 13)`. If continuing an older save
   inside Littleroot, leave and re-enter town once to load the new NPC template.
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
| Virelian Spinarak, level 10 | Level 13 (3 candies) | Level 14: Virelian Ariados |
| Virelian Ariados, level 30 | Level 33 (3 candies) | Level 34: Arachnaut |
| Virelian Ekans, level 10 | Level 15 (5 candies) | Level 16: Virelian Arbok |
| Virelian Arbok, level 30 | Level 35 (5 candies) | Level 36: Cindracoil |

After each evolution, recheck the two types and moves. Also create first stages
at the exact threshold and one of the offered higher levels: they should retain
the selected stage until the next eligible level-up. Test all three stages
through party-to-PC deposit, withdrawal, saving, emulator reset and Continue.

Trading uses the ordinary species/ownership data path. Both peers must run the
same feature ROM; old ROMs and FireRed/LeafGreen builds do not define these new
species. Do not trade them to a ROM that lacks their IDs. No link protocol,
Link Battle, or Battle Tower code is changed.

## Engine adaptations and save compatibility

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
  uses a map-local temporary flag. Existing saves can use the added NPC when they
  reach Littleroot; the inherited quick start itself applies to new games.
- Save compatibility is with the selected expansion baseline and configuration,
  not retail Emerald or unrelated hacks. Back up a save before changing ROMs.
  Once it contains custom species, loading it in an older build that lacks those
  definitions is unsupported even though the serialized structure size is equal.

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
TM/egg-move review, final names (especially the provisional fire family), custom
battle sprites/palettes for the Water and Fire families, custom icons, footprints,
cries and overworld graphics for all families, final physical dimensions and dex
prose, signature mechanic design/testing, and a future Virelia regional dex.
