---
name: pokeemerald-development
description: Implement and test gameplay changes in this Virelia pokeemerald-expansion fork, including species, evolutions, learnsets, moves, abilities, trainer parties and story scripts. Use for engine behavior and game data. Use porymap-mapping for map geometry and gba-tileset-pipeline for terrain assets.
---

# Virelia gameplay development

Work from the repository root. The fork started at expansion 1.17.1. Check the active feature switches in `include/config/` and trace a comparable feature through its callers and tests. The upstream guides explain the architecture, but their examples can predate the enums, file layout and build settings in this checkout.

## Species and regional forms

- Trace a form and an independent evolution side by side. Run `rg -n 'SPECIES_NATU_VIRELIA|SPECIES_NIMBATU' include src data test`. The custom block in `include/constants/species.h` ends before `SPECIES_CUSTOM_END`, which also determines `SPECIES_EGG` and `NUM_SPECIES`. Check `P_VIRELIA_STARTERS` in `include/config/species_enabled.h` when changing those families.
- Edit the species record in `src/data/pokemon/species_info/virelia_families.h`. It connects stats, abilities, graphics, learnsets and evolution targets. Follow its referenced symbols to the assets and move lists the game loads. Virelia level-up lists live in `src/data/pokemon/level_up_learnsets/virelia.h`. Check names, dex entries, icons and overworld graphics when changing identity. [Expansion species guide](https://rh-hideout.github.io/pokeemerald-expansion/tutorials/how_to_new_pokemon.html)
- A regional form can share a National Dex number with its base and use a base-first table in `src/data/pokemon/form_species_tables.h`. An independent evolution can have its own entry in `include/constants/pokedex.h`. Trace both regional and original evolution paths, and check the resulting egg species. For a replacement, update all current producers, consumers and tests together. Follow `AGENTS.md` on save compatibility. Validate the current build with a fresh save while preserving its own save/load, storage, breeding, trade and link behavior.

## Moves, abilities and trainers

- Follow a similar move from `include/constants/moves.h` into `src/data/moves_info.h`. Reuse an existing effect when its timing and targeting match. For a distinct mechanic, trace its effect through `src/data/battle_move_effects.h` to the battle script and C handler. Check who can learn or use the move and how the player sees its animation and description. [Expansion move guide](https://rh-hideout.github.io/pokeemerald-expansion/tutorials/how_to_new_move.html)
- For abilities, distinguish the ID and display record in `include/constants/abilities.h` and `src/data/abilities.h` from the code that reacts during battle. For example, `rg -n 'ABILITY_WATER_ABSORB' src test/battle` finds the absorption logic, AI reasoning and test cases. Exercise both a triggering and a non-triggering case at the affected battle phase.
- Change trainer party source in `src/data/trainers.party` for Emerald or the corresponding FRLG source when appropriate. Trace the trainer ID from its encounter script before editing a shared party. Check levels, moves and availability at the point in the story where the battle occurs.

## Story scripts

Follow the object, warp or map-script entry point through its label in `data/maps/<map>/scripts.inc` or a shared event in `data/scripts/`. The research lab's `data/maps/Virelia_MaplebrookResearchLaboratory/scripts.inc` shows the distinction between on-load setup, transition state and a one-time scene. Follow its `callnative` targets into `src/virelia_prologue.c` when investigating starter state. Match the local story/development-mode guards. For persistent events, verify first interaction, re-entry and reload. Check the matching `test/virelia_*_events.c` for executable examples. For changes to covered Czech prologue text, run `python3 tools/validate_virelia_prologue_text.py` to check display constraints.

## Verify the changed behavior

In the configured Linux build environment, select a test by its name. For a regional evolution change, run `make -j4 check TESTS="Virelia and original evolution thresholds"` to cover the level boundary and the original species. For breeding ancestry, boxing and FLASH persistence, use `make -j4 check TESTS="Virelia species"`. These tests live in `test/virelia_starters.c`. For battle scenarios, find a comparable test in `test/battle/` and use `docs/tutorials/how_to_testing_system.md` for assertions on turns, messages and battle state. Confirm the filter ran the intended tests.

The test jobs in `.github/workflows/build.yml` define the full story and `VIRELIA_DEVELOPMENT_MODE=1` checks. Use the applicable suite when touching shared logic or mode boundaries. For a player-visible interaction, use the `mgba-playtesting` workflow to reproduce it in the matching ROM. Include the build mode and checks actually run when reporting the result.
