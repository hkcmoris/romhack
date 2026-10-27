# Chapter 2 — implementation design

Baseline: `codex/virelia-prologue`, commit
`492267dadc0f9b54322a39038c479d717eba958a` (confirmed by the user).
New branch: `codex/virelia-rainfall-seabreak`.

## Geography and traversal

Only a small entrance footprint at Slateford `(12, 0–3)` changes. A north-facing
rock passage at `(12, 1)` opens after `FLAG_VIRELIA_FIRST_GYM_COMPLETE`.
Existing buildings, southern connection and edited town geometry remain.
Conventional reciprocal warps separate outdoor tilesets safely.

| New map | Size | Role |
| --- | --- | --- |
| RainfallFoothills | 32 × 28 | Rocky approach, fork, Hiker, return shortcut |
| RainfallUpper | 32 × 34 | Short elevated trail, wooden bridges, Ranger |
| RainfallLower | 40 × 40 | Longer stream/marsh loop, Fisherman, items, shortcut |
| RainfallLowlands | 32 × 26 | Sheltered confluence, rival, weather student, coast exit |
| SeabreakTown | 48 × 42 | Harbor, boardwalk, Center/shop, homes, headland |
| Seabreak_PokemonCenter | 24 × 9 | Existing safe local nurse, PC, expanded shop |
| Seabreak_Lighthouse | 16 × 14 | Běla, damaged feed, clear puzzle clue |
| Seabreak_LanternRoom | 13 × 12 | Three signal controls, manifest, working beacon |
| Seabreak_KeepersHouse | 12 × 10 | Coastal history and optional quest context |

The upper and lower trails branch at the foothills and meet only at the
lowlands. Lower exploration opens a persistent return footbridge. Neither
mandatory path requires an HM. The rival encounter guards the final descent,
allows an ordinary loss/whiteout and retries until victory. Backtracking stays
available. Town exploration and the bottle quest remain optional.

## Story and persistent state

Badge → short Mara/exit hint → route fork → rival → Seabreak harbor report →
Běla checks damaged electrical feed → signal puzzle (feed, lens, beacon) →
Slateford conductive-mineral manifest with missing recipient → harbor reopening
and Vacek's credit scene. Second Gym remains closed for storm inspection.
Rival, mother, professor and Mara receive brief progress dialogue.
Ema remains Maplebrook's existing Poochyena owner.

Append overworld maps at group `75`, indices `4–8`; append interiors at group
`76`, indices `8–11`; append layouts `798–806` and sections `212–213`.
Existing maps `75,0–3`, interiors `76,0–7`, layouts `786–797`, species
`1573–1581`, trainers and sections retain their IDs.

Reserve unused saved flags `0x264–0x27F` for chapter start, rival victory,
harbor report, damage inspection, lighthouse completion, manifest, harbor
reopening/chapter completion, bottle stages/reward, shortcut, route items and
new trainer victories. Audit found no active use of these unused slots.
Use unused saved variable `0x409B` for bottle stage; temporary `0x400F` for
unfinished signal inputs. Flag helpers must validate prerequisite order and
commit rewards only after Bag insertion succeeds. No save structure changes.

## Encounters and balance

Four ordinary trainers, two or three Pokémon each at levels `16–20`, distributed
across both alternatives. Rival: evolved regional counter-starter at level 19
and two restrained support Pokémon at levels 17–18; use the existing opposite
gender identity and original starter choice. No exceptional rewards.
Wild tables use forest, marsh and coastal species at levels `14–19` with moderate
encounter rates and walkable grass-free paths. Add Rain Dance at level 14 to the
regional Natu family; retain evolution thresholds and abilities. Audit obtainable
Pokémon/moves/items/abilities through level 24 for Czech descriptions.

## Risks and verification

Use stock GBA artwork and document character/boat placeholders. Do not run old
map generators. Gate only the new passage, preserving normal return paths.
Weather uses map headers: light rain above, heavier rain below, sheltered
lowlands, coastal rain. No weather-engine changes.

Preserve My Boy fix `debddae9eec8b7f9c88d0e77ce3da605cfba3a04`:
`-fno-isolate-erroneous-paths-dereference` in both CFLAGS and LTO linking.
Ordinary builds retain diagnostic mode zero. Existing Android evidence is a
user report on Galaxy A26/Android 16/My Boy 1.8; new chapter Android QA remains
separate and cannot be inferred from mGBA.

Validate map walkability/warps/interaction positions, prerequisite flags,
trainer mappings, rival retry, puzzle callbacks, all intermediate FLASH stages,
full Bag rewards, local services and Czech text width. Run native tests and
the existing CI workflow, compile a ROM and retain exact build evidence.
Estimated additional playtime is 30–45 minutes; measurement requires an
unassisted full playthrough. No measured duration is claimed in advance.
