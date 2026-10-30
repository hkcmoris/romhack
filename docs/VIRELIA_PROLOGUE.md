# Pokémon Virelia — playable prologue v0.2

The [v0.4 Rainfall Pass and Seabreak chapter](VIRELIA_RAINFALL_SEABREAK.md)
continues after Mara's Lomový odznak through independent saved flags. It keeps
the prologue, sanctuary and first Gym playable, appends its maps and opens only
the new northern Slateford passage. The historical evidence below remains
specific to its recorded revision; Chapter 2 has a separate verification record.

## Scope and baseline

The [v0.3 first Gym chapter](VIRELIA_FIRST_GYM.md) continues completed prologue
saves through independent chapter flags. Its maps append to the existing registry;
the historical v0.2 scope and native evidence below remain the prologue record.

The normal Emerald new game now starts the Virelia campaign. This milestone
ends after the first Team Rocket encounter in Slateford City. It preserves the
existing title screen, species identifiers, Hoenn maps, save structures and
Link Multi implementation. No multiplayer feature is required for progression.

Branch: `codex/virelia-prologue`. The initial baseline is the last fully passing
`main` revision `6b3a90d807019edb2afed41a9882811ce09e32ff`. The subsequent
battle-message translation `198dc90ec06c83f03b5891065a96c31ff208a1a5` is integrated
as a fast-forward before the prologue commit and checked by the combined CI.

The older [world foundation](VIRELIA_WORLD.md) and
[starter prototype](VIRELIA_STARTERS.md) describe the underlying registries and
species. This document supersedes their development-start instructions.

### Maplebrook town life

The optional town spaces now include Ilona's open cottage, a riverbank with a
fisherman and Psyduck, a community noticeboard and courier, Ema with her
Poochyena, a shaded walking loop with Wurmple, and a small plant nursery.
The noticeboard and courier react to the laboratory rescue; the courier also
reacts to the Rocket encounter. These visits do not gate the lab or north trail.
The professor's garden remains fenced.

Ilona gives one Oran Berry after the starter is received. Investigating behind
the grove's stone finds one Antidote. Their independent flags are
`FLAG_VIRELIA_MAPLEBROOK_BERRY_GIFT` (`0x4A`) and
`FLAG_VIRELIA_MAPLEBROOK_GROVE_FIND` (`0x4B`). Full pockets leave both rewards
available for another attempt. Development exhibitions do not grant these gifts.
The cottage appends interior map index `7` after Slateford's shop moves into the
Pokémon Center. The current prologue's map indices stay unchanged, and the
new cottage door uses Maplebrook warp index `2`.

The redesigned Maplebrook exterior is 36 × 48 tiles. Outdoor development
travel arrives at `(9,47)`, on the bottom edge below the player's house.
Ilona's cottage entrance is beside the nursery at `(29,34)` (warp `2`).
The riverside fisherman's house at `(3,25)` uses warp `3` and a new interior
with fishing gear and a notebook; the fisherman remains outside by the river.
The former cottage at `(30,20)` is closed and its sign identifies it as empty.
The shared outdoor tileset includes north–south wooden bridge metatiles
`0x363`/`0x364`, built by `python tools/virelia/build_vertical_bridge.py` from
the existing bridge deck. They connect the paths at `(13,20)`–`(14,23)`.
All existing staircases remain unchanged. The north gate still covers `(14,1)`
and `(15,1)` through prologue states 0–3.

Lab regression checks follow the active campaign: story uses Viktor/Viktorie,
persistent case slots and post-rescue level matching; development uses Hoenn
rival placeholders, a full three-slot case and authored trainer levels.
`build_lab_case.py --check` accepts LF or CRLF palette line endings, while
checking binary assets exactly. Its unit checks also reject actual color changes.

## Playable sequence

1. Choose **NOVÁ HRA**, gender and trainer name. The engine creates the Trainer
   ID and initializes a fresh save through its ordinary new-game routines.
2. Start inside the player's house in Maplebrook Town. **MÁMA** greets the player
   automatically. She sends the player to the research laboratory and later provides
   unlimited free healing. The home also serves as the blackout destination.
3. Visit **profesorka Javorová**. The player walks into the central aisle.
   Her welcome is interrupted when Rattata escapes and Meowth chases it around
   the right-hand benches. The assistant steps aside and Viktor joins the player.
   The rival is always the opposite gender to the player: female players meet
   Viktor; male players meet Viktorie. The overworld sprite, trainer front/back
   portraits, battle identity and dialogue all follow that choice.
4. Javorová sends both trainers to a compact open research case. Its 2×2 map
   graphic reuses native lab furniture and colors, with three miniature Balls.
   A single north-facing interaction at `(6, 6)` is approached from `(6, 7)`.
   The long table and its three separate Ball objects are removed. The custom
   full-screen briefcase shows three fitted Ball slots, regional species names
   and types. Left/right browses; A opens the animated front-picture preview
   with its cry and a confirmation. No or B returns to browsing. There is no
   countdown. Confirming returns the choice to the ordinary Virelia gift routine,
   granting exactly one locally owned level-5 starter with native met data.
5. The player's case slot becomes empty. The player steps aside and the rival
   approaches the same case interaction spot to take the remaining starter with
   the primary type advantage. The player and AI-controlled Viktor each use one
   starter in a cooperative wild double battle against level-3 Meowth and Rattata.
   Fleeing returns to the confrontation without another starter gift; victory,
   defeat or a draw lets Javorová contain the escapees and heal the player's team.
   Viktor's temporary partner party and battle flags are cleared. His selected
   species is the same one he uses in the next fight.
6. After the commotion, Viktor challenges the player to the original single duel.
   The rival's starter matches the player's starter level after the rescue
   (minimum 5), so a player level-up to 6 is matched by the rival. This applies
   to Viktor and Viktorie and also accounts for EXP earned before a rescue retry.
   Species, moves and IVs stay as authored; ordinary trainer levels are unchanged.
   Either result advances the story and heals the player. His defeat line is
   `Dobre hej, jsem tě nechal.` Javorová then offers the native nickname screen;
   declining or pressing B keeps the current name. She grants a Pokédex,
   **5 Poké Balls and 3 Potions** once. A full Bag delays the entire bundle until
   there is space. The unused third Ball remains inside the case. Later lab visits
   skip the completed opening and keep ordinary healing and exhibition dialogue.
7. Follow **Whisperwood Trail** north. Battle two ordinary trainers, explore
   existing tall grass and find the optional item detour. **Hana** asks about
   her missing Zigzagoon; investigating its secluded spot reveals territorial
   behavior. Return to Hana for one extra Potion. Her thanks reveal an overgrown
   western path to the [hidden sanctuary](VIRELIA_SANCTUARY.md), with three carved
   stones, a permanent shortcut and Quick Claw. These events are optional.
8. Reach **Slateford City**. Miners report missing conductive mineral samples.
   Investigate the suspicious maintenance worker near the mining area, expose
   his Team Rocket uniform and win a normal single battle. Losing permits a
   later retry after healing at home or the laboratory.
9. The grunt escapes and leaves a clue about the stolen minerals. A policeman
   arrives and claims progress. The completed prologue leaves all three outdoor
   areas and both interiors accessible. The quarry investigation and first Gym
   are the next objective; the Gym remains closed in this milestone.

No Hoenn moving-truck sequence, clock lesson, Birch rescue, starter selection or
catching tutorial is required. Hoenn content remains registered for future
postgame work. The Virelia initialization marks the original introduction's
progress variables so that it cannot unexpectedly restart during this campaign.

The new-game field callback also uses an ordinary fade instead of the original
truck animation. Time-based events are initialized without the clock lesson.
The equipment grant enables the engine's National Pokédex, because Natu and
Ekans are outside the inherited Hoenn listing. In story mode, entries prefer the
chosen regional starter family and the rival's actually encountered first stage;
other species keep their ordinary form. The native Pokédex shares seen/caught
bits between forms, so it cannot list ordinary and regional versions as separate
captures. A dedicated Virelia regional listing is reserved for later work.

## Maps and actors

The four existing Virelia map IDs are unchanged. The house is appended to the
interior group; existing map and layout registry entries retain their positions.

| Area | Constant | Group, map | Role |
| --- | --- | --- | --- |
| Maplebrook Town | `MAP_VIRELIA_MAPLEBROOK_TOWN` | 75, 0 | Home, laboratory, route entrance and residents |
| Whisperwood Trail | `MAP_VIRELIA_ROUTE01` | 75, 1 | Wild encounters, two trainers, item detour and optional Hana event |
| Slateford City | `MAP_VIRELIA_SLATEFORD_CITY` | 75, 2 | Mineral theft, Rocket battle, policeman and closed Gym |
| Research laboratory | `MAP_VIRELIA_MAPLEBROOK_RESEARCH_LABORATORY` | 76, 0 | Professor, assistant, compact starter case and rival |
| Player's home | `MAP_VIRELIA_MAPLEBROOK_PLAYERS_HOUSE` | 76, 1 | New-game spawn, mother and free healing |

The home uses existing Emerald house assets. Its spawn is `(5, 6)`, healing
arrival `(2, 7)`, mother local ID `1`, and its paired exit tiles are `(8, 8)` and
`(9, 8)`. The town's original laboratory warp keeps index `0`; the home entrance
is appended at `(7, 9)`. Outdoor north/south connections remain bidirectional.
The world validator checks physical walkability, reciprocal warps and that
objects have an accessible interaction tile.

New names are provisional: **Javorová** (local professor), **Viktor** (recurring
rival), **Ela** and **Ota** (route trainers), **Hana** (Zigzagoon's owner). The
mother, laboratory assistant, miners, policeman and Rocket grunt are unnamed.
The player's name always comes from the save's `{PLAYER}` placeholder.

## Starters and battles

| Player's choice | Species | Type | Rival's starter |
| --- | --- | --- | --- |
| Grass | `SPECIES_POOCHYENA_VIRELIA` | Grass/Dark | `SPECIES_EKANS_VIRELIA` |
| Water | `SPECIES_NATU_VIRELIA` | Water/Psychic | `SPECIES_POOCHYENA_VIRELIA` |
| Fire | `SPECIES_EKANS_VIRELIA` | Fire/Poison | `SPECIES_NATU_VIRELIA` |

Virelian Poochyena evolves into Grass/Ghost Virelian Mightyena at level 18,
then into Grass/Ghost Wraithena at level 38. The existing identifier and
provisional in-game name `Wraithena` are preserved. Original Hoenn Poochyena and
Mightyena retain their original types and evolution. No Forest Ambush ability
or other signature mechanic is added.

Viktor uses a deliberately restrained introductory moveset rather than an
overwhelming super-effective attack. The engine's early-rival battle mode
returns to the script and heals after either win or loss. All starter teams begin
at level 5 for the rescue. The duel matches the player's post-rescue starter level;
the starter choice selects the corresponding counter-type trainer definition.

| Trainer | Team | Persistence |
| --- | --- | --- |
| Viktor / Viktorie | One regional starter, matching the player's level (minimum 5) | Rival-complete flag plus saved outcome |
| Ela | Wurmple 4, Zigzagoon 4 | Ordinary engine trainer-defeated flag |
| Ota | Taillow 6 | Ordinary engine trainer-defeated flag |
| Rocket grunt | Rattata 7, Zubat 6 | Ordinary trainer-defeated flag plus Rocket story flag |

The appended trainer IDs are `855`–`860`; no old trainer ID is renumbered.
Whisperwood Trail keeps its ordinary wild Pokémon and level-2–5 encounter slots:
Wurmple, Zigzagoon, Poochyena, Seedot, Taillow, Shroomish and Ralts. Regional
starters do not replace those encounters. A Potion item ball uses the existing
`FLAG_ITEM_VIRELIA_ROUTE01_POTION`.

## Saved state and idempotency

Before the stable release, discard saves from previous playtest versions and
choose **NEW GAME** whenever switching versions. Cross-version save compatibility
is not promised. Saving and loading within the current build must still preserve
Pokemon, items, position and story state correctly.

Story state uses existing unused flag/variable slots. The story initializer runs
only for a newly created game. Development and story playtests use separate new
games.

Definitions live in `include/constants/virelia_prologue.h`.

| Flag | Slot | Meaning |
| --- | --- | --- |
| `FLAG_VIRELIA_MOM_MET` | `0x21` | Home introduction complete |
| `FLAG_VIRELIA_STARTER_RECEIVED` | `0x22` | Starter grant committed |
| `FLAG_VIRELIA_RIVAL_COMPLETE` | `0x23` | First rival battle returned after win or loss |
| `FLAG_VIRELIA_ROCKET_DEFEATED` | `0x24` | Rocket battle won |
| `FLAG_VIRELIA_SUPPLIES_RECEIVED` | `0x25` | Once-only Pokédex and equipment bundle granted |
| `FLAG_VIRELIA_PROLOGUE_COMPLETE` | `0x26` | Rocket clue and conclusion resolved |
| `FLAG_VIRELIA_SIDEQUEST_STARTED` | `0x27` | Hana's request heard |
| `FLAG_VIRELIA_SIDEQUEST_RESOLVED` | `0x28` | Zigzagoon investigated |
| `FLAG_VIRELIA_SIDEQUEST_REWARDED` | `0x29` | Hana's reward granted |
| `FLAG_VIRELIA_HIDE_DEV_GUIDE` | `0x2A` | Development guide hidden in story |
| `FLAG_VIRELIA_HIDE_POLICEMAN` | `0x2B` | Policeman has not arrived yet |
| `FLAG_VIRELIA_HIDE_STORY_ACTORS` | `0x2C` | Story actors hidden in development mode |
| `FLAG_VIRELIA_MINER_REPORT` | `0x2D` | Mineral theft reported by miners |
| `FLAG_VIRELIA_PROFESSOR_MET` | `0x2E` | First laboratory welcome started |
| `FLAG_VIRELIA_LAB_RESCUE_COMPLETE` | `0x48` | Ally rescue finished before the single duel |
| `FLAG_VIRELIA_HIDE_LAB_ESCAPEES` | `0x49` | Meowth and Rattata contained; shared visibility flag |
| `FLAG_VIRELIA_HIDE_GRASS_STARTER` | `0x2F` | Grass starter Ball taken |
| `FLAG_VIRELIA_HIDE_WATER_STARTER` | `0x30` | Water starter Ball taken |
| `FLAG_VIRELIA_HIDE_FIRE_STARTER` | `0x31` | Fire starter Ball taken |
| `FLAG_VIRELIA_HIDE_RIVAL_POKEMON` | `0x4C` | Rival companion hidden until its starter is taken |

| Variable | Slot | Values |
| --- | --- | --- |
| `VAR_VIRELIA_PROLOGUE_STATE` | `0x4083` | Home `0`, mother `1`, starter `2`, rival `3`, Rocket `4`, complete `5` |
| `VAR_VIRELIA_STARTER` | `0x408B` | Grass `0`, water `1`, fire `2`, none `0xFFFF` |
| `VAR_VIRELIA_RIVAL_RESULT` | `0x4091` | Engine battle outcome of first rival battle |

Native grant functions validate their prerequisites and return success through
`VAR_RESULT`. Flags are the authority for once-only grants; re-entering a map,
repeating dialogue, declining a choice or loading a checkpoint cannot grant
additional starters or bundles. Hana's reward flag is set only after the item
fits into the Bag. The full Bag case leaves the reward available for a retry.

The laboratory synchronizes the case's eight possible slot masks on map load.
Existing pickup flags are never cleared in story mode, and completed story
states do not replay the scene. The entrance sequence is locked through selection and both battles. The rescue
completion and escapee visibility flags survive ordinary saves independently
of the rival-duel result. Development and nonstory builds retain all three case
slots and open the same preview screen without granting a story gift.
`tools/virelia/build_lab_case.py --check` verifies the native tileset composition,
unchanged stock furniture references, slot states and hardware tile budget.

## Overworld companions

The first healthy, non-egg Pokemon in the player's party follows them in the
overworld. Changing party order changes the follower; fainted Pokemon and eggs
are skipped. The existing follower system handles map transitions, returning
from battles and menus, and temporary hiding during scripted scenes.

All nine stages of the Virelia starter families use dedicated regional overworld
art, including normal and shiny palettes. Their walking sheets start from copies
of the original species, recolored and adapted to match the regional battle art.
Evolution updates the follower to the corresponding regional stage. Ordinary
species keep their original sprites. The aquatic family's current shiny colors
match its normal colors, as they do in battle.

Viktor or Viktorie releases their chosen starter in the laboratory. It follows
the rival back from the case and stays beside them through the rescue and duel.
Its species matches the rival's team for all three starter choices. Re-entering
the laboratory or continuing a save from this build restores the pair.
Interacting with the Pokemon plays its cry. The rival currently appears only in
the laboratory. The companion uses a separate graphics variable from the rival,
so both player-gender branches retain the correct human sprite.

The Slateford policeman arrives with a Growlithe following one tile behind him.
Growlithe stays beside the officer after the scene and shares his visibility
flag, including after saving or revisiting the city. Interacting with it plays
Growlithe's cry.

Validate with a new save: choose each starter, finish the rival battle, walk
outside with the player follower, then return to the laboratory. Save and reload
there, and check party reordering and healing after a fainted lead Pokemon.

## Czech localization

All authored story dialogue, choices, signs, trainer reactions, gifts and object
messages are Czech. Proper Pokémon names and Maplebrook Town, Whisperwood Trail,
Slateford City and Virelia remain unchanged. Move and item names retain their
English spelling in every build; their descriptions and surrounding messages
are localized. Czech dialogue inflects item names where needed (`Potiony`,
`Potionů`); quantity labels use the base name (`3× Potion`). The rival class
uses `Rival` (`RIVAL` in uppercase labels). Text reuses the existing Czech
charmap and fonts, including accented uppercase characters.

The shared resources reachable through this slice include battle messages,
menus, saving, Bag, party, summary, naming, Pokédex, relevant items, move
descriptions, abilities, types and statuses. The detailed coverage, exceptions and outstanding
visual checks are maintained in the
[localization audit](VIRELIA_LOCALIZATION_AUDIT.md). That audit is the authority
for localization status; a compiled ROM alone does not certify complete Czech
coverage or correct on-screen wrapping.

The 2026-10-24 merge-gate update resolves all 156 missing bounded-coverage
entries reported after merging the newer route data: 46 moves, 16 abilities,
15 Pokédex records and Sitrus Berry. Including generation-specific description
variants and the berry's quantity label, the manifest now contains 1,231
entries and passes `tools/validate_virelia_prologue_text.py` through level 20.
The existing coverage derivation and font limits remain in force. Move and item
names retain their English literals; Czech descriptions, ability names and
Pokédex text use the existing localization macro. Bind, Encore, Minimize, Taunt
and Sitrus Berry retain their generation guards and English test text.
The accompanying WSL build passes the full native suite: 5,512 tests passed,
with 18 marked known failures, 421 TODOs and nine expected-failure self-tests;
there were no unexpected failures. All 71 Virelia tests passed as well.

Shared battle and data strings retain English test fixtures under `TESTING`
where necessary. Localized strings in normal and release ROMs select Czech at
preprocessing time. This keeps the upstream behavior tests meaningful without duplicating either
language in a playable ROM. Development-only test notices are hidden in story.
The story Pokédex uses metres, kilograms and a decimal comma. Development mode,
FireRed/LeafGreen and upstream test fixtures retain their original unit settings.

## Build configuration

GitHub Actions compiles the ROM; no local Ubuntu, WSL or ARM toolchain is needed
to edit this Windows checkout. The default `emerald-release` artifact starts
the story. `emerald-development` is the normal unoptimized **story** build;
the artifact name describes compiler optimization, not the old quick start.

With the project's existing build environment, use:

```sh
# Normal story, also the default CI release.
make -j4 release

# Original development quick start, starter scientist and Link Multi test tools.
make -j4 VIRELIA_DEVELOPMENT_MODE=1

# Story and original development regression suites.
make -j4 check
make -j4 check VIRELIA_DEVELOPMENT_MODE=1
```

`VIRELIA_DEVELOPMENT_MODE` defaults to `0`. Mode `1` enables the original
`ORACULUM_DUO_TEST_QUICK_START`, `VIRELIA_WORLD_TEST` and `VIRELIA_STARTER_TEST`
facilities through the established configuration. It adds `-devmode` to output
names and object directories, preventing stale object reuse when switching modes.
Use distinct saves for development and story playtests. The development mode
does not modify Cable Club communication or Link Multi battle rules.

The title SELECT shortcut and built-in debug menus are also restricted to
development mode in Emerald. Both optimized and unoptimized story builds keep
those tools hidden. FireRed/LeafGreen retain their existing configuration.

CI retains Emerald, FireRed, LeafGreen, optimized release and documentation
checks. It runs story and explicit development-mode tests in separate jobs.
The prologue used ROM version `0.2.N`, where `N` is the Actions run number.
The first Gym branch advances that version to `0.3.N`.

## Validation and manual playtest

Native QA uses an official mGBA libretro core and real controller inputs at
240 × 160. It does not inject Pokémon, items or story flags. The optimized
release from commit `7c120c18eb` passed fresh female trainer creation and the
automatic home introduction within 120 frames of confirming the name. It also
loaded a genuine in-game FLASH save from the earlier story build. Save tests
restart the core from FLASH, without transferring an emulator savestate.

![Czech release title](images/virelia-prologue/title-7c120.png)
![Mother in the player's home](images/virelia-prologue/home-7c120.png)
![Czech party summary with the regional Grass/Dark starter](images/virelia-prologue/summary-7c120.png)

All three starter/rival matchups were played natively. Winning and deliberately
losing the rival both continued to healing and the initial supplies. Additional
native checks covered catching an ordinary Wurmple, both route trainers, the
optional Potion detour, Hana's once-only reward and its FLASH persistence, and
a Rocket loss returning safely to mother before winning a retry. A capture
after previously opening the full Pokédex exposed a pre-existing stale-pointer
bug: closing the menu freed `sPokedexView` without clearing it, so caught-entry
cleanup could free a live battle allocation. The cleanup now clears the pointer.
The first Rocket finale also exposed an asset configuration error: the police
sprite was registered only in FireRed/LeafGreen, so spawning it reset Emerald.
The existing police art and registry entry are now shared with Emerald, without
changing any graphics or species IDs. Native tests check the graphics pointer
for every object template on all five playable maps. The corrected capture,
nickname and next-battle sequence passed on `ee05868f53` (Actions build
`0.2.36`). The full Rocket finale passed in both unoptimized and optimized
release builds. The officer's three Czech dialogue pages lead to state 5 and
the quarry/Gym objective. The Gym stays closed; leaving Slateford and returning
does not replay the ending. A normal in-game save followed by a fresh core boot
from FLASH restored the exact Slateford position, party, items, hidden Rocket,
visible officer and completion flags. The
[localization audit](VIRELIA_LOCALIZATION_AUDIT.md) records the evidence.

The later `f52456d` native check also verified the Czech `MDL` badge after a
wild Taillow naturally fainted the starter, and the `POPIS` move-detail prompt.
The assembled statistics row has a separate font-metric check for three-digit
values and `---`, column spacing, category icon and the 144-pixel window edge.
Its numeric buffers include space for all three digits and the terminator.
Native build `14a3132` (`0.2.39`) confirmed the complete `PŘES: 100` label
without clipping, followed by closing the detail and returning from battle.

![Metric Czech Pokédex](images/virelia-prologue/pokedex-ee058.png)
![Officer arriving after the Rocket battle in release](images/virelia-prologue/police-ee058.png)
![The next objective in release](images/virelia-prologue/ending-ee058.png)
![Czech fainted badge in the forced party selector](images/virelia-prologue/status-f524.png)
![Complete Czech move statistics in battle](images/virelia-prologue/move-detail-14a3132.png)

Source checks runnable without an ARM toolchain:

```sh
python tools/validate_virelia_world.py
python tools/validate_virelia_lab_scene.py
python tools/validate_virelia_prologue_text.py
python .github/docs_validate/inclusive_summary.py
git diff --check
```

The laboratory scene validator follows the actual map script and movement
commands against collision data, including the concurrent chase. Its 648 opening
cases cover both player genders, both entrances, all starters, all rescue and duel outcomes, fleeing
and retrying, and accepting/declining/cancelling the naming offer. Guard cases
cover repeat visits, prerequisites, failed gifts and display interactions.
`--self-test` must reject representative movement, pickup, battle-order, cleanup
and naming regressions. `tools/virelia/build_briefcase.py --check` verifies the
indexed art and GBA tile budget. The text validator also measures the case's
native prompt and type-label windows.

`test/virelia_lab_opening.c` exercises the actual briefcase input tasks and
preview cancellation, both rival identities and their dialogue placeholders,
the real compiled partner teams, rescue preparation and
cleanup, and the one-mon-per-ally battle layout. Existing flash-save coverage
also preserves the two new rescue flags. A fresh save is required to playtest
the complete revised opening; source and native tests do not substitute for a
manual end-to-end Android playthrough.

The native suites `test/virelia_prologue.c`,
`test/virelia_prologue_events.c`, `test/virelia_world.c` and
`test/virelia_starters.c` cover initialization, prerequisites, all starter and
rival choices, duplicate prevention, types, equipment, battle continuation,
event persistence, save/load and bidirectional warps. Existing multiplayer and
development regressions run with the explicit development configuration.

Before declaring the milestone manually accepted, play a fresh release save
with **each** starter and record the ROM commit, emulator and elapsed time:

- Confirm trainer creation, Czech glyphs in both cases, spawn, the unmissable
  opening conversation with mother and repeatable free healing.
- Approach the compact case from its single front tile. Preview every starter,
  cancel once, then confirm. Check species, level, OT, moves, HP/PP and Pokédex.
- Check that the player steps aside and the rival takes the counter-starter from
  that same spot. The corresponding two case slots become empty. After saving,
  reloading and revisiting, only the unused third Ball remains.
- Play both genders and check the rival tactics line, quarry manager address and
  winner record use the correct Czech gender.
- Win and lose separate rival battles; verify the expected opposing species,
  restored party, Pokédex, five Balls and three Potions without duplicate gifts.
- Catch an ordinary wild Pokémon, inspect its Pokédex entry and summary, use
  and cancel Bag actions, switch party members, heal and save.
- Battle both route trainers; check the item detour and Hana event in either
  order, including revisits and a full Bag reward retry.
- Lose once to Rocket, heal and retry. Win, read the clue and policeman scene,
  leave and return; the conclusion must not repeat and the Gym must stay closed.
- Save and reload within the same build at home, after rival, on the route and
  after Rocket. Confirm party, items, position and story/trainer flags.
- Inspect every reachable screen listed in the localization audit. Check actual
  GBA text windows for clipping, accents, substitutions and remaining English.
- Separately start a mode-1 build and check the original test facilities. Link
  Multi still requires its existing two-emulator manual verification.

The target is **15–20 minutes** at normal text speed, including one capture,
both route trainers, the optional detour and the Rocket scene. The initial
estimate allocates 4–5 minutes to creation/home/lab, 2–3 to the rival/equipment,
5–7 to route exploration/catching and 3–5 to Slateford. This is a content and
interaction estimate, **not a measured human playthrough**. A native Grass run
completed the fixed prologue at an in-game clock of **00:14:03**, including
all three starter descriptions, one selection, Hana's quest, both route trainers
and the Rocket battle, with no capture on that run. Checking the closed Gym and
returning to the city advanced the clock to **00:14:55**. This supports
the estimate for a normal journey with one capture and time to read the ending;
it does not certify human completion time. Record timed release-ROM observations
after manual verification; CI duration is not gameplay duration. Fast text,
skipped exploration or grinding can change it.

The requested `npm format` and `npm lint` commands were invoked, but this C/ASM
repository has no npm package/scripts and npm reports them as unsupported.
The repository's Python validators, native `make check` suites and GitHub
Actions are the applicable checks. No JavaScript or dependency was added.

## Slateford finale staging

The defeated worker now flees via the southern city road to (14, 29), fully
outside the fixed camera before removal. When the player approaches from the
west, he first goes around the player. The other legal approaches share the
direct route to the road. No route crosses the quarry rocks or another actor.

The policeman's hidden template is positioned at the southern entrance before
spawning. He walks up the road and across the square at normal speed (16 frames
per tile), faces the player and pauses before speaking. His stop at (20, 22)
is saved to the object template and used by the map after returning to the city.
The existing battle prerequisites and prologue completion flags are unchanged.

`python3 tools/validate_virelia_slateford_scene.py` checks all three legal
approaches against the actual map, objects, movement scripts and camera limits.
The strict development ROM and all 72 native Virelia tests passed after this
change. Focused mGBA fixtures started from a fresh initialized game, injected
the prerequisites and winning battle result, and ran the compiled post-battle
scene from the west, east and north. Frame traces confirmed the offscreen exit,
normal officer step timing, no player overlap and the final (20, 22) stop.
Returning to the city retained the officer and did not replay the scene.

![Grunt departure and policeman entrance in native mGBA](images/slateford-finale.gif)

## Remaining milestones

The [v0.3 milestone](VIRELIA_FIRST_GYM.md) implements the quarry and first Gym
after this prologue. Further cities, the full Rocket plot, Porygon experiments
and storage incident remain reserved for later milestones. Mega Evolution
remains locked. No new signature ability, cooperative story gate,
PvP reward, network synchronization or Duo Adventure feature is introduced.

Temporary art and names remain documented in the starter prototype. The native
engine's broader postgame, PC and multiplayer facilities are retained; their
full-game localization and manual acceptance are outside this opening slice.

## Android preview investigation

See [the preview freeze investigation](VIRELIA_PREVIEW_FREEZE.md) for checked
sprite allocation, cleanup regressions, unchanged artwork evidence, the
separate battle path and the exact My Boy! verification procedure. A successful
CI or native mGBA run does not certify the affected Android device.

The affected Galaxy J6 / Android 9 still freezes in My Boy! 1.8.0 with both
external and built-in BIOS, including the 0.3.60 candidate. The user's BIOS
matches the standard checksum. Updating My Boy! to 2.1.1 did not resolve the
freeze; the BIOS mode of that later attempt was not specified. On the same phone
the same ROM passes the previews, Poochyena selection and the previously failing
transition into battle in Lemuroid. Battle completion and the full Android
chapter remain unverified. This is a confirmed compatibility difference, not a
completed My Boy! fix; see the investigation for the captured state and native proof.
