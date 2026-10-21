# Pokémon Virelia — Slateford Quarry and first Gym v0.3

## Scope and baseline

Branch: `codex/virelia-first-gym`, based directly on the clean, committed
`codex/virelia-prologue` revision `14a30dafebeba34615f9c8d1d3d42ae019b86739`.
The baseline [CI run](https://github.com/hkcmoris/romhack/actions/runs/39573968690)
passed. The source map, laboratory scene and localization validators passed
before chapter implementation. No older `main` content replaces the prologue.

The earlier native prologue evidence covers the Rocket finale, state 5 and
FLASH Continue on `ee05868`, and the Czech battle detail UI on `14a3132`.
The latest starter Ball pickup/preview and Porymap edits had source/CI coverage
but lacked a recorded native scene check at Phase 0. This is an explicit
verification gap, not evidence that those events are broken. Native checks for
this milestone are recorded below with their exact limits.

This slice adds Slateford services, two connected quarry sections, five ordinary
single battles and Mara's first Gym. The intended additional duration is
20–30 minutes; this is a design target until timed by a human player.
Viktor, starter selection and prologue progression remain unchanged.
Link Multi, Duo Adventure, network synchronization and multiplayer missions
receive no gameplay or protocol changes and remain unverified on My Boy! Wi-Fi.

## Append-only map registry

Existing group/map, layout, species, trainer, heal location and story IDs keep
their values. New interiors append to group 76. All use the existing Slateford
map section; no world-map cell is repurposed.

| Map | Constant | Group, map | Purpose |
| --- | --- | --- | --- |
| Pokémon Center | `MAP_VIRELIA_SLATEFORD_POKEMON_CENTER` | 76, 2 | Nurse Joy, ordinary PC, save/load and whiteout |
| Poké Mart | `MAP_VIRELIA_SLATEFORD_MART` | 76, 3 | Ordinary buy/sell service |
| Slateford Quarry entrance | `MAP_VIRELIA_SLATEFORD_QUARRY_ENTRANCE` | 76, 4 | Miners, exploration and first electrical control |
| Slateford Quarry depths | `MAP_VIRELIA_SLATEFORD_QUARRY_DEPTHS` | 76, 5 | Second control, freight evidence and optional item |
| Slateford Gym | `MAP_VIRELIA_SLATEFORD_GYM` | 76, 6 | Geological puzzle, two trainers and Mara |

The new layouts copy Emerald tiles, palettes and existing service furnishings.
Original Hoenn maps, layouts and graphics remain intact. Layout IDs append to
`data/layouts/layouts.json`; the source validator checks their concrete order.
`tools/virelia/build_first_gym_maps.py` documents the authored layout construction.
Do not run the old world generator over later Porymap edits.

The Center appends `HEAL_LOCATION_VIRELIA_SLATEFORD_POKEMON_CENTER`.
Entering the Center records its ordinary healing/whiteout destination.
Nurse local ID is 1; whiteout position is (7, 4).
The ordinary PC uses native storage; no storage lockdown is implemented.
The upstairs communication area is closed and has no warp.
The Mart stocks Poké Ball, Potion, Super Potion, Antidote, Paralyze Heal,
Escape Rope and Repel. Item names remain English.

## Story and chapter boundary

The completed prologue remains authoritative through
`FLAG_VIRELIA_PROLOGUE_COMPLETE` and `VAR_VIRELIA_PROLOGUE_STATE = 5`.
Entry to the new chapter sets only `FLAG_VIRELIA_CHAPTER_STARTED`.
It never clears or rewrites the prologue. The old Rocket automatic scene is
eligible only at state 4, so a completed save cannot replay it.

1. After the Rocket conclusion, the quarry manager requests help with damaged
   mining equipment. Before that conclusion, the quarry remains closed.
2. Explore two connected working areas with equipment, mineral seams, side
   paths and wild Pokémon. Bruno and Lenka offer ordinary trainer battles.
3. Restore the electrical installation by following its labeled control order.
   Wrong input and leaving the puzzle area reset partial input. Once restored,
   power stays restored across reentry, losses, saving and loading.
4. Inspect the abandoned freight after power is restored and both quarry
   trainers are defeated. Its label matches the grunt's discarded label.
   The stolen minerals were selected for high-capacity electrical conductors.
   The freight's destination and ultimate use remain unknown.
5. Report the evidence to the manager. The manager authorizes reopening the Gym.
   Repeated conversation grants no repeat reward.
6. In the Gym, read the geological clues, operate its two controls in order and
   defeat Ivan and Dana. Then challenge Mara.
7. Win, receive the Lomový odznak and collect Rock Tomb's TM. A full Bag leaves
   the TM pending; talk to Mara again after making space. Then save and Continue.

The quarry reveals no Porygon experiments, laboratory or future Storage attack.

## Characters, battles and level curve

Mara is a geologist who works with the quarry. Her dialogue treats strata and
battle planning as practical disciplines, with occasional dry humor.
Bruno is a miner, Lenka an explorer, and Ivan and Dana train in Mara's Gym.

| Trainer | Team | Explicit moves |
| --- | --- | --- |
| Bruno | Geodude 10, Makuhita 11 | Tackle / Defense Curl / Rock Throw; Tackle / Arm Thrust / Focus Energy |
| Lenka | Sandshrew 11, Aron 12 | Scratch / Defense Curl / Sand Attack; Tackle / Harden / Metal Claw |
| Ivan | Roggenrola 13 | Tackle / Harden / Rock Throw |
| Dana | Geodude 13, Nosepass 14 | Tackle / Defense Curl / Rock Throw; Tackle / Harden / Rock Throw |
| Mara | Roggenrola 14, Nosepass 16 | Headbutt / Rock Throw / Sand Attack / Harden; Tackle / Rock Tomb / Thunder Wave / Block |

All use ordinary abilities and moves, zero fixed IVs and no held items.
AI uses Check Bad Move; Mara additionally uses Try To Faint.
No custom move, ability or damaging Ground move is added to their teams.
Losses use normal whiteout, retain earlier victories and permit retries.
Leader victory and reward collection are separate persistent states.

Trainer IDs 866–870 append after the reserved partner bank.
`MAX_TRAINERS_COUNT_EMERALD` stays 864, preserving all existing partner IDs and
the `SYSTEM_FLAGS` base. A central flag lookup maps only these five new trainers
to reserved flags 0x40–0x44. Existing trainer flags keep their original formula.
The ROM-only trainer slide table grows to cover both trainers and partners.
No save structure or link-state layout changes.

The prologue ends around levels 7–9. The entrance offers levels 8–11, depths
10–13, Gym trainers 13–14, and the leader 14/16. A small party should reach
roughly 11–17 through exploration and normal battles. Actual no-grind pacing
and every starter's combat difficulty require the native/human checks below.

The fixed starter thresholds remain Spinarak 14, Ekans 16 and Poochyena 18.
Poochyena starts Grass/Dark and evolves to Grass/Ghost Mightyena.
Ariados's Water/Bug typing remains vulnerable to Rock attacks; evolution alone
does not remove that matchup. Fire/Poison Ekans can lower Special Defense with
Acid Spray and recruit Makuhita or Sandshrew. Trainers impose no strong Ground
attack that would exploit its quadruple weakness.

## Wild encounters

Both quarry maps use native cave land encounters with encounter rate 12.
Each row below is one native slot; its probability is unchanged by its level.

| Slot | Weight | Species | Entrance | Depths |
| --- | --- | --- | --- | --- |
| 0 | 20% | Geodude | 8–11 | 10–13 |
| 1 | 20% | Roggenrola | 8–11 | 10–13 |
| 2 | 10% | Sandshrew | 8–11 | 10–13 |
| 3 | 10% | Makuhita | 8–11 | 10–13 |
| 4 | 10% | Aron | 8–11 | 10–13 |
| 5 | 10% | Sandshrew | 8–11 | 10–13 |
| 6 | 5% | Makuhita | 8–11 | 10–13 |
| 7 | 5% | Aron | 8–11 | 10–13 |
| 8 | 4% | Geodude | 8–11 | 10–13 |
| 9 | 4% | Roggenrola | 8–11 | 10–13 |
| 10 | 1% | Nosepass | 8–11 | 10–13 |
| 11 | 1% | Nosepass | 8–11 | 10–13 |

The optional Super Potion uses its own pickup flag and retries a full Bag.
The original Whisperwood encounter table and starter learnsets remain unchanged.

## Independent progression and rewards

| Flag | Slot | Meaning |
| --- | --- | --- |
| `FLAG_VIRELIA_CHAPTER_STARTED` | 0x32 | Prologue boundary crossed |
| `FLAG_VIRELIA_QUARRY_POWER_RESTORED` | 0x33 | Quarry puzzle solved permanently |
| `FLAG_VIRELIA_QUARRY_EVIDENCE` | 0x34 | Conductive-mineral freight inspected |
| `FLAG_VIRELIA_QUARRY_COMPLETE` | 0x35 | Manager authorizes Gym |
| `FLAG_VIRELIA_GYM_PUZZLE_COMPLETE` | 0x36 | Gym puzzle solved permanently |
| `FLAG_VIRELIA_BADGE_QUARRY` | 0x37 | Lomový odznak earned |
| `FLAG_VIRELIA_TM_ROCK_TOMB_RECEIVED` | 0x38 | Rock Tomb TM actually fits in Bag |
| `FLAG_VIRELIA_ITEM_QUARRY_SUPER_POTION` | 0x39 | Optional pickup collected |
| `FLAG_VIRELIA_FIRST_GYM_COMPLETE` | 0x3A | Badge and TM collected |
| Trainer flags Bruno–Mara | 0x40–0x44 | Five independent victories |

Partial puzzle inputs use temporary vars 0x400C and 0x400D.
They are not chapter progress and may reset on map entry/Continue.
Completed puzzle flags always restore the open passage.

The badge intentionally grants recognition and chapter progression only.
It grants no implicit Hoenn Attack bonus, obedience threshold or field-move use.
No original Hoenn badge flag is set or repurposed.
The existing eight-slot Trainer Card therefore remains unchanged.
The Gym's post-victory dialogue/statue exposes earned progress.
A future sixteen-badge page must read independent Virelia flags, present regions
separately and retain original Hoenn badge semantics.

## Czech localization

All new dialogue, control labels, acquisition messages and service UI are Czech.
Pokémon species, move, item and proper place names remain English.
The existing charmap, fonts and real-font width checks apply.
See [the extended localization audit](VIRELIA_LOCALIZATION_AUDIT.md) and the
manifest for exact species, evolution, ability, item and move coverage through
level 20. Source validation does not certify every rendered reward/service screen.

## Save compatibility

This milestone preserves existing save structure sizes and all existing IDs.
A completed baseline prologue save can enter the chapter without replaying old
events. Keep an external backup before changing ROM versions. Structural
preservation does not certify arbitrary old development saves or unrelated
configuration changes. Never load a new-map save in an older ROM lacking those IDs.

No load-time blanket initialization clears chapter flags. Rewards commit their
flag only after successful item insertion. Save/load must preserve exact map,
party, storage, money, purchases, trainer victories and story completion.

## Validation status and manual checklist

Source checks and GitHub Actions results must be recorded for the final commit.
Until those results and native checks below are populated, this document does
not certify a completed playtest or balanced 20–30 minute session.

Run:

```sh
python tools/validate_virelia_world.py --base 14a30dafeb
python tools/validate_virelia_lab_scene.py
python tools/validate_virelia_prologue_text.py
python tools/validate_virelia_first_gym.py
python .github/docs_validate/inclusive_summary.py
git diff --check
make -j4 check
make -j4 check VIRELIA_DEVELOPMENT_MODE=1
make -j4 release
make firered -j4
make leafgreen -j4
```

GitHub CI retains Emerald normal/release, both test modes, FireRed, LeafGreen and
documentation jobs. Native test data assertions are not manual combat balancing.
No Android My Boy! playtest or network success is inferred from CI.

1. Back up a genuine completed-prologue FLASH save. Boot the new ROM and Continue.
   Verify exact position/party/items, hidden Rocket and visible officer, and no finale replay.
2. Also start fresh and confirm starter preview, cancellation, pickup, Viktor's
   move sequence, battle return and supplies on the latest baseline assets.
3. Enter Center/Mart; test each exit tile and save/reboot inside each building.
   Heal damaged/statused party and PP. Deposit/withdraw through the ordinary PC.
   Buy and sell items; verify money, item counts and persisted storage.
4. Lose in the quarry and Gym. Verify Center whiteout, nurse positioning, healing,
   previous trainer victories and retry. Also test loss before visiting the Center.
5. Before the prologue, verify blocked quarry; before reporting evidence, blocked Gym.
   Verify that entering/reentering an open service does not advance old story.
6. Explore both quarry paths, catch a useful teammate, beat each trainer, try
   wrong switch order, leave/reset, solve correctly, save/reboot and verify open gate.
7. Check freight before prerequisites, then after; report to manager, revisit and
   verify the Gym opens permanently.
8. Try wrong Gym controls, exit/reset, solve and defeat both trainers. Lose to
   Mara once, return and win. Verify before/after dialogue.
9. Check the Lomový odznak acquisition text and Rock Tomb TM display. Fill the TM
   pocket before victory, then free space and collect exactly once. Revisit,
   save/reboot and check no duplication. Inspect untouched Hoenn badge flags.
10. Repeat combat progression with all three starters and a modest obtainable
    team. Record team levels, captures, optional battles, healing and elapsed time.
11. Read all service, trainer, puzzle and reward text at native 240×160. Check
    diacritics and line widths, level-up moves, evolution and learning prompts
    through level 20.

Stop after the first badge and TM. No next city, second Gym, Porygon reveal,
Storage lockdown, Mega Evolution or multiplayer feature belongs to this slice.
