# Rainfall Pass and Seabreak — Android / My Boy! checklist

**Chapter verification status: NOT RUN.**

This is the separate manual Android acceptance record for Chapter 2. Passing
source checks, native tests or mGBA does not change this status. The historical
Galaxy A26 / Android 16 / My Boy! 1.8.0 starter-preview result in
[My Boy! compatibility](MYBOY_COMPATIBILITY.md) covers the older tested sequence.
Use the final milestone ROM containing the cached Slateford entrance fix.

## Test identity

Complete these fields for each device/ROM combination. Copy the record for a
new run; record failures with the exact case and reproduction steps.

| Field | Recorded value |
| --- | --- |
| Date, tester | NOT RUN |
| Device model | — |
| Android version | — |
| My Boy! version | — |
| BIOS mode, BIOS identity/hash if external | — |
| Milestone source commit | — |
| GitHub Actions run and artifact | — |
| ROM filename, version and SHA-256 | — |
| Starting save origin and SHA-256 | — |
| New game / backed-up baseline first-Gym FLASH save | — |
| Player gender and original starter choice | — |
| Initial party species, levels and moves | — |
| Test result and evidence directory | NOT RUN |

Use ordinary in-game **Save**, close the emulator and reopen the ROM with only
its matching FLASH save before choosing **Continue**. Emulator save states do
not substitute for this test. Preserve an untouched backup of a baseline save.
The working baseline is `492267dadc0f9b54322a39038c479d717eba958a`; earlier
prototype saves with different species or map arrangements are outside this
compatibility check. All test cases below currently have status **NOT RUN**.

## A. Baseline FLASH Continue and the new Slateford exit

1. In a baseline first-Gym save made inside Slateford, earn the Lomový odznak
   and accept Mara's Rock Tomb reward so `FLAG_VIRELIA_FIRST_GYM_COMPLETE` is
   set. Save normally and close the baseline emulator session.
2. Open that backed-up FLASH save with the final milestone ROM and choose
   Continue directly in Slateford. **Do not leave and re-enter the map first.**
   The save contains the older cached tree view, which previously hid the new
   northern door even though its badge gate opened.
3. Walk from `(12,3)` through `(12,2)` into the native cave doorway at `(12,1)`.
   Enter Rainfall without a detour, debug warp or city reload. Inspect the
   north cliff cap and side shrubs; they must frame the entrance rather than
   leave old tree art over its trigger.
4. Return immediately to Slateford, save there, close the emulator and repeat
   Continue and the northern exit. Verify the southern Whisperwood connection,
   existing Center, quarry entrance and Gym still work.
5. With a separate pre-badge baseline save, repeat Continue in Slateford.
   The approach at `(12,2)` must remain blocked and explain Mara's prerequisite.
   Try both side approaches; neither may bypass the gate.

Record case A independently from a new-game run. A successful fresh map load
alone does not verify restoration over the older saved map view.

## B. Both Rainfall paths and weather

1. Explore the shared rocky foothills and read the route sign. Take the upper
   trail through its narrow wooden bridge and Ranger encounter to lowlands.
   Return along the same path and cross each ordinary warp in both directions.
2. Take the lower wetland trail from the foothills. Explore stream banks,
   optional grass, the Super Potion and the permanent footbridge control.
   Decline the shortcut once, then accept it. Return from the foothills through
   the unlocked shortcut and confirm safe arrival beside the lower control.
3. Save, close and Continue after unlocking the shortcut. Both directions must
   still work without Surf, Waterfall, Strength or another HM. Check the upper
   Repel reward and confirm both route rewards cannot be duplicated.
4. Check light rain on the upper route, downpour below, the sheltered lowlands,
   town rain and dry interiors. Save and Continue in each zone; move, interact
   and battle afterward. Observe animation performance during extended rain on
   the actual device, including consecutive map transitions.
5. Encounter and capture suitable wild Pokémon in each zone. Check levels
   14–19, ordinary capture/flee behavior and the area's expected species.
   Trainer sight must allow ordinary navigation without unavoidable consecutive
   battles. Record collision, elevation, bridge, border or visibility defects.

## C. Rival choices, battle loss and retry

Repeat this group for all three original starter choices and both player
genders, using separate save copies. Female player must meet Viktor; male
player must meet Viktorie. Verify the correct regional evolved counter-starter:
Poochyena choice → Arbok, Natu choice → Mightyena, Ekans choice → Xatu.

1. Approach the lowlands rival and check Czech name, sprite and dialogue.
   Verify the coast is blocked before victory and the original choice has not
   changed since the prologue.
2. Lose normally. Confirm normal whiteout, party healing and a safe return from
   the previously registered Center. The rival and closed descent must remain;
   no completed-chapter flag or NPC reward may appear after defeat.
3. Retry and win. The rival must leave the descent, the coastal tile must open,
   and the correct team must have been used. Save, close and Continue. A repeat
   battle must not occur, and Seabreak's rival must acknowledge the same identity.
4. Record party levels, losses, resource use and any necessary grinding.
   Natu-family Rain Dance must be available through its level-14 learning event;
   choosing another starter must not block weather explanations or progression.

## D. Seabreak Center, shop, Storage and saving

1. Enter the combined Center/Mart, damage/status the party through normal play,
   then heal. Verify HP, PP and status restoration, animation, readable follow-up
   messages and normal field controls after both accepting and declining healing.
2. Buy and sell ordinary items. Check Great Ball, Net Ball, Super Potion and
   Repel availability, correct money/item changes and Czech shop messages.
3. Deposit and withdraw a Pokémon using ordinary Pokémon Storage. Give a held
   item, save inside the Center, close and Continue; confirm party/box/item data.
4. After registering this Center, lose a later battle. Whiteout must restore
   inside Seabreak Center at `(7,4)` with the local nurse rather than an unrelated
   Hoenn destination. Save/Continue and exit normally afterward.
5. Save and Continue in the keeper's house, lighthouse ground floor and lantern
   room. Every building must retain a usable ordinary exit.

## E. Lighthouse progression and puzzle persistence

1. Visit Běla and the controls before the harbor report. The game must explain
   the prerequisite without freezing input or permitting an early repair.
2. Speak to the harbor master, then Běla. Inspect the damaged cabinet and read
   the written emergency instructions. Save and Continue after the report and
   after the damage inspection; both checkpoints must remain actionable.
3. Try the lens or beacon first. The wrong-order message must reset unfinished
   input. Begin with the feed, cancel the next prompt, then continue or restart;
   canceling must not advance a control or commit repair.
4. Leave the lantern room after partial input and return. Restart the explicit
   sequence: feed, lens, beacon. Temporary input may reset; completed repair must
   never reset. Verify the beacon visibly lights after the correct third input.
5. Save and Continue before and after completion. A repaired beacon must restore
   its lit state; repeated control interactions must not restart the puzzle.
6. Read the shipping record only after repair. Confirm Slateford Quarry's
   conductive minerals and obscured recipient appear in Czech. Save before
   returning to the harbor master and verify the record survives Continue.

## F. Optional Message in the Bottle

1. Speak to Ota at `(7,35)` and note his family motivation. Before following the
   map, inspect later clues out of order; they must not skip saved stages.
2. Follow the anchor `(6,31)`, coastal tide mark `(41,35)` and keeper-house
   journal `(3,3)`. Save and Continue after each clue. Read the revelation and
   return to Ota; the lighthouse story must remain independently completable.
3. Fill the relevant Bag pocket before claiming the reward. Confirm the game
   leaves Mystic Water available, then free space and retry. Receive exactly
   one item; give it to a Pokémon, save and Continue, and revisit Ota again.
4. Repeat the lower Super Potion and upper Repel checks with a full Bag if not
   covered earlier. Do not use a successful emulator save-state reload as proof
   of one-time reward persistence.

## G. Harbor reopening, chapter ending and backtracking

1. Return to the harbor master after reading the manifest. Verify Vacek's credit
   scene, Běla's suspicion and the reopened harbor dialogue. Confirm no badge,
   evolutionary item or multiplayer reward is granted for the rival or finale.
2. Save, close and Continue after the scene. Vacek/Běla must remain accessible,
   the beacon functional, the rival/sailor dialogue updated and the one-time
   scene finished. The second Gym must remain unavailable with a natural hint.
3. Backtrack through both route alternatives to Slateford, Whisperwood and
   Maplebrook. Visit Mara, the player's mother and Professor Javorová. Check
   their updated dialogue and retained Gym/research/healing services.
4. Check the sanctuary shortcut, quarry events and existing Ema/Poochyena
   continuity. Re-enter Seabreak and save again without replaying the finale.

## H. Localization, balance and elapsed-time record

Inspect Czech accents, choices, trainer titles and battle/service messages at
240×160. Species, move, item and established place names stay English. Check
new summaries, abilities, moves, items and reachable evolutions through level24.

Record an unassisted complete chapter with each starter family, including the
chosen path, optional content, final levels, defeats and elapsed playtime.
The 30–45-minute chapter and 5–10-minute side-quest targets remain estimates
until measured. Record unexpected stalls, forced type disadvantages, grinding
or rain slowdowns with the exact save checkpoint and reproduction steps.

| Result field | Recorded value |
| --- | --- |
| Cases passed / failed / not run | NOT RUN |
| Upper / lower route coverage | NOT RUN |
| Starter choices and player genders covered | NOT RUN |
| Baseline FLASH Continue exit | NOT RUN |
| Puzzle / quest / ending persistence | NOT RUN |
| Measured chapter / optional quest time | NOT MEASURED |
| Screenshots, logs, failures and reproduction steps | — |

Only the tester's Android results can update this checklist's device-specific
status. Keep mGBA, native-suite and CI evidence in the
[main chapter record](VIRELIA_RAINFALL_SEABREAK.md).