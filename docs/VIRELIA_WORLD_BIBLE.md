# Pokémon Virelia — World, Story & Geography Bible

> **A Pokémon Emerald ROM hack about a young trainer exploring a living region, uncovering the exploitation of Pokémon, and discovering that defeating a criminal organization is not necessarily the same as removing the person who benefits from its defeat.**
>
> **Core world-design principle:** The first visit introduces the place. The second visit changes its story. The third visit reveals a secret.

**Document purpose:** Consolidate the world developed in our design discussions into one self-contained reference for writing, mapping, art direction, and future implementation.

**Audience:** The creator, collaborators, artists, and Codex. This document contains full story spoilers.

**Language:** Planning prose is English. The playable game is localized into Czech, with natural dialogue and correct diacritics. Established English location names and canonical Pokémon names are retained.

**Status:** This is a design bible, not a claim that all described content has been implemented. It incorporates the later Mosswatch / Verdant Hollow revision and supersedes contradictory geography or incidental labels in older concept images. It does not constitute a fresh audit of the live repository.

---

## Contents

1. [Identity, tone, and design commitments](#1-identity-tone-and-design-commitments)
2. [Canon, proposals, and superseded ideas](#2-canon-proposals-and-superseded-ideas)
3. [The landmass and its natural logic](#3-the-landmass-and-its-natural-logic)
4. [Regional positions and compass rules](#4-regional-positions-and-compass-rules)
5. [The main journey and eight-Gym structure](#5-the-main-journey-and-eight-gym-structure)
6. [Routes, connections, and return loops](#6-routes-connections-and-return-loops)
7. [Location atlas: the opening heartland](#7-location-atlas-the-opening-heartland)
8. [Location atlas: Stormcrest and the eastern coast](#8-location-atlas-stormcrest-and-the-eastern-coast)
9. [Location atlas: the southern wetlands and ancient forest](#9-location-atlas-the-southern-wetlands-and-ancient-forest)
10. [Location atlas: the west and northern highlands](#10-location-atlas-the-west-and-northern-highlands)
11. [The main story and Team Rocket](#11-the-main-story-and-team-rocket)
12. [The Pokémon Storage crisis](#12-the-pokémon-storage-crisis)
13. [Giovanni: the useful ally and hidden beneficiary](#13-giovanni-the-useful-ally-and-hidden-beneficiary)
14. [The recurring cast](#14-the-recurring-cast)
15. [Vacek: falling upwards](#15-vacek-falling-upwards)
16. [The regional starters](#16-the-regional-starters)
17. [Starter-specific awakening storylines](#17-starter-specific-awakening-storylines)
18. [Optional exploration and short side quests](#18-optional-exploration-and-short-side-quests)
19. [Duo Adventure and the multiplayer rival](#19-duo-adventure-and-the-multiplayer-rival)
20. [The Hoenn postgame](#20-the-hoenn-postgame)
21. [Visual identity, map composition, and localization](#21-visual-identity-map-composition-and-localization)
22. [Implementation boundaries and development continuity](#22-implementation-boundaries-and-development-continuity)
23. [Open decisions and continuity checklist](#23-open-decisions-and-continuity-checklist)
24. [Source and revision notes](#24-source-and-revision-notes)

---

## 1. Identity, tone, and design commitments

### 1.1 The promise of the game

Virelia is a new Pokémon region built as an Emerald ROM hack. The adventure should feel like a Pokémon game that a nostalgic adult remembers loving, but with more interconnected places, more recurring characters, more environmental storytelling, and more attention to the absurd consequences of a child repeatedly doing the authorities' work.

The protagonist is still a child. This is not a story about a thirty-five-year-old office worker transported into Pokémon, nor a parody in which everyone acknowledges the game engine. The target audience is roughly thirty-five-year-old players with nostalgia; the inhabitants still live sincerely inside the Pokémon world.

The experience rests on five commitments:

- **A real trainer journey.** Exploration, catching Pokémon, building a team, rival battles, Gyms, and the League remain the central adventure.
- **One interconnected region.** Mountains, rivers, settlements, and routes have deliberate positions. New shortcuts repeatedly bring the main story back to familiar places.
- **One evil organization.** Team Rocket is the recurring criminal threat. Its internal factions and personalities provide complexity without introducing several competing teams.
- **Personal starter stories.** Each regional starter has a distinct identity and an optional storyline that unlocks its Hidden Ability.
- **An evolving world.** Towns, NPCs, services, and accessible spaces change after the player acts. Earlier areas are not discarded after their first badge.

### 1.2 Humor

The humor comes from recognizable human behavior in a Pokémon setting: bureaucratic self-preservation, exaggerated confidence, workplace excuses, terrible explanations, and a police officer whose public reputation grows faster than his competence.

Czech comedic timing, local idiom, and occasional allusions to the kind of humor associated with Tři tygři can inform the writing. They should not turn every conversation into a chain of quotations or make the world depend on recognizing a specific sketch. Original jokes must work in context.

A miner can complain that a Rocket grunt's boots are suspiciously clean. A parent can worry about lunch while their child is about to dismantle an international criminal network. Vacek can produce an official report that is technically compatible with events while omitting who actually did the work.

### 1.3 Serious material

The Porygon experiments are a meaningful moral escalation. Pokémon are being exploited and hurt, not merely used as a decorative explanation for a computer puzzle. Electrical Pokémon are also being used as components in the operation.

The scene should be disturbing through confinement, repeated electrical discharges, damaged equipment, exhausted Pokémon, and the perpetrators' indifference. Graphic gore is neither necessary nor part of the intended Pokémon tone.

Humor should not undermine the moment of rescue. A joke can expose the villains' callousness or release tension afterward; it should not ask the player to laugh at a suffering Pokémon.

### 1.4 Familiar technology, unfamiliar places

Poké Balls, Pokémon Centers, storage systems, research laboratories, shipping, industrial equipment, modern communications, and the Pokémon League belong in the setting. Ancient ruins coexist with that modern world.

Later painterly illustrations sometimes drift toward castles, medieval uniforms, and fantasy kingdoms. Their materials, silhouettes, and terrain can be useful references. They do **not** replace the intended modern Pokémon setting with a medieval one.

---

## 2. Canon, proposals, and superseded ideas

Use the following distinctions throughout this document:

| Label | Meaning |
|---|---|
| **Established** | A clear project constraint or a direction explicitly accepted in the discussions. |
| **Working design** | A developed proposal used to make the world coherent, but still open to revision in detail. |
| **Open** | A decision that has not been made and must not be silently treated as final. |
| **Historical implementation** | Something reported or inspected in an earlier build; not proof of the current repository state. |

### 2.1 Established foundations

Virelia is the main campaign region; Hoenn remains the postgame region. The player is a child. The game is localized into Czech. Team Rocket is the single evil organization. The Porygon exploitation and Pokémon Storage disruption are central to its plot; artificial intelligence is not the central theme.

The opening sequence of destinations is Maplebrook → Whisperwood → Slateford → Rainfall Pass → Seabreak. Slateford sits by mountains and has a quarry. Rainfall crosses to the other side of the mountains. Seabreak is a rocky, rainy port spread over three elevation levels. Its upper and lower approaches enter different districts. A later cave connection links the Seabreak side to Slateford's mining area. Seabreak becomes the departure port for Hoenn after the Virelia League.

Mosswatch is the settlement associated with Gym 4. Verdant Hollow is the adjoining ancient rainforest, not a conventional Gym town. Wraithyena's Hidden Ability is unlocked there. Ancient ruins lead eventually to a deeper temple discovered later in the game.

The entire early map layout can be refactored to serve coherent geography. Existing map files are not sacred.

### 2.2 Working design, not immutable canon

The names Stormcrest Range, Reedmere, Mosswatch, Ironwake, Skywatch, Frostgate, and Crownspire are the current working names. Mosswatch's separation from Verdant Hollow is accepted; exact map coordinates and the names of most later routes remain adjustable.

The eight-Gym order and shortcut schedule below provide the current full-campaign framework. Exact levels, leader types beyond Mara, later leader names, the timing of every Rocket event, and the exact finale location remain open.

The supporting cast described below is a developed proposal. Vacek's falling-upwards role is central; not every suggested supporting character has been individually finalized.

### 2.3 Superseded ideas

- **ORÁKULUM** is not the game's final title. Legacy build filenames can retain it without making it an in-world name.
- **Doctor Kód** was rejected as the principal antagonist's name.
- A story primarily about artificial intelligence was rejected in favor of Pokémon exploitation and storage-system control.
- Virelian Spinarak / Ariados / a new final spider was replaced as the Water starter by the Natu line. The spider design is not the current starter plan.
- First-stage Virelian Poochyena is **Grass/Dark**, not Grass/Ghost. Its later stages acquire the Ghost typing.
- Verdant Hollow no longer occupies the role of a Gym town. **Mosswatch Village is Gym 4; Verdant Hollow is wilderness.**
- Rainfall's two paths must not both collapse into one mandatory Seabreak entrance merely because the first implementation did so.
- Seabreak is not a flat, sunny beach resort.
- Healing on knockout, proposed as **Spirit Harvest**, was not selected for Wraithyena; it felt redundant with Forest Ambush.
- Decorative labels, arrows, added buildings, and invented names inside generated images are not authoritative world data.

---

## 3. The landmass and its natural logic

### 3.1 Overall shape

Virelia is conceived as a substantial, mostly continuous landmass with an irregular coastline, a north–south mountain backbone, a southern river-and-forest belt, and smaller offshore islands reserved for optional exploration.

The broad journey circles the southern portion before returning through the center and ascending into the north. This allows the player to see the mountains first as a barrier, later as a network of crossings, and finally as the setting for the League approach.

The exact coastline polygon, total physical size, and whether the League's final peninsula is separated by a narrow channel are not finalized. The planning grid defines relative positions, not kilometers.

### 3.2 Stormcrest Range

Stormcrest separates Slateford's inland side from Seabreak's eastern coast. It is not merely a row of decorative mountains behind town sprites. It determines the region's early transport, mining, rainfall, and watercourses.

Slateford is west of the ridge. Its quarry cuts into the western foothills. The two Rainfall trails cross the same mountainous barrier through different terrain: a higher ridge-and-bridge route and a lower gorge-and-wetland route.

The later karst passage goes **under** that barrier. It is initially unknown or unsafe, not a conveniently open tunnel that everyone inexplicably ignores.

### 3.3 Weather model

Our working climate model places moisture-bearing sea winds on the eastern side. The coastal slopes and Rainfall's eastern approaches are consequently the wettest part of the opening region; Slateford is comparatively sheltered on the inland side.

Treat this as an authored climate premise. Not every location must share the same rainfall or vegetation. Southern forest areas can remain humid through local drainage and a different exposure without making the entire western interior uniformly dry.

Seabreak's lighthouse repair restores navigation. It does not switch off the weather. Heavy rain and rough seas are part of the town's identity, not automatically evidence of a villain controlling the climate.

### 3.4 Water and drainage

Streams begin on higher ground, descend through gorges and waterfalls, and feed lowland pools, rivers, estuaries, or the sea. The lower Rainfall path follows the wetter terrain toward Seabreak's harbor approach.

Reedmere occupies the southeastern delta system farther south. Its canals, floodgates, raised buildings, and transport all derive from living with water. It should not be a random swamp placed between unrelated towns.

Verdant Hollow contains springs, flooded basins, roots, and old waterworks. Its Sunken Temple can be partly inundated without requiring the entire forest to lie below sea level.

### 3.5 Distinct human landscapes

The settlements have different reasons to exist:

| Settlement | Why people live there |
|---|---|
| Maplebrook | Rural homes, local cultivation, and Javorová's ecological research. |
| Slateford | Stoneworking, quarry labor, minerals, and mountain freight. |
| Seabreak | A sheltered eastern harbor, navigation, fishing, and coastal trade. |
| Reedmere | River transport, wetland livelihoods, and flood management. |
| Mosswatch | Research, conservation, guiding, and forest-edge crafts. |
| Ironwake | Western shipping, heavy industry, rail freight, and repair yards. |
| Skywatch | Weather observation, research, mountain services, and high-altitude settlement. |
| Frostgate | Alpine passage, mountain maintenance, and life in a snowy basin. |
| Crownspire | Regional administration, civic life, major infrastructure, and the final Gym. |

The rainforest and mountains retain large spaces that do not exist primarily for human activity. Not every ruin needs a town, and not every major location needs a Gym.

---

## 4. Regional positions and compass rules

### 4.1 Coordinate convention

The saved geography blueprint uses an abstract **0–100 planning grid**:

- X increases east.
- Y increases north.
- North stays at the top of the regional map.
- Elevation is a separate property; “upper terrace” does not automatically mean “north.”

These are regional anchors, not Porymap tile coordinates, pixel positions, precise city boundaries, or Fly destinations. A large change to one anchor should trigger a review of the connecting routes, rather than being hidden behind a warp.

### 4.2 Working position register

The values below preserve the original planning anchors wherever possible. The old Gym-4 anchor is reassigned to Mosswatch as a **working revision**. Verdant Hollow and its internal sites require new, separate footprints; their exact coordinates have not yet been approved.

| Location | Planning anchor (E, N) | Role / qualification |
|---|---:|---|
| Maplebrook Town | (28, 24) | Starting settlement. |
| Whisperwood Trail | (30, 35) | Northbound forest route. |
| Slateford City | (31, 48) | Gym 1, west of Stormcrest. |
| Slateford Quarry | (40, 51) | Separate outdoor excavation area east-northeast of Slateford. |
| Rainfall Fork / Foothills | (45, 55) | Western approach and route choice. |
| Rainfall Upper Pass | (57, 61) | Higher mountain crossing. |
| Rainfall Lower Pass | (59, 43) | Lower gorge crossing. |
| Seabreak upper approach | (73, 54) | Schematic high-district arrival anchor. |
| Seabreak town center | (79, 46) | Gym 2 settlement, eastern coast. |
| Seabreak harbor approach | (74, 39) | Schematic low-district arrival anchor. |
| Reedmere | (67, 21) | Gym 3, southeastern delta. |
| Mosswatch Village | Around (39, 15) | Working reuse of the former Gym-4 anchor; outside the ancient rainforest. |
| Verdant Hollow | Inland / northwest of Mosswatch | New wilderness footprint, not the former city marker. |
| Forgotten Sanctuary | Within Verdant Hollow | Exact submap position open. |
| Sunken Temple | Deeper within Verdant Hollow | Separate late-discovered complex; exact submap position open. |
| Westwind Track | (18, 36) | Northwest route from Maplebrook toward the western coast. |
| Ironwake | (11, 51) | Gym 5, western industrial harbor. |
| Stormcrest Karst | (57, 53) | Schematic underground crossing anchor. |
| Skywatch | (75, 70) | Gym 6, high eastern slopes north of Seabreak. |
| Frostgate | (48, 84) | Gym 7, northern alpine basin. |
| Crownspire | (84, 82) | Gym 8, northeastern plateau. |
| Virelia League | (86, 94) | Northern League approach and final complex. |

Seabreak's three anchors communicate the region graph. They should not be interpreted as requiring an implausibly enormous city or as prescribing three exact map sizes.

### 4.3 Rules for translating geography into playable maps

A normal road leaving east should lead to terrain east of the origin and enter its neighbor from the west. A diagonal regional journey can wind through multiple north/south/east/west segments. A cave, stairway, switchback, station, or boat can connect nonadjacent positions when the transition is visibly motivated.

For example, Slateford → Rainfall can climb northeast, cross east, then descend southeast toward Seabreak. It does not need to be one straight horizontal corridor. It must, however, remain a believable crossing of the same mountain range.

Every inter-map connection should eventually record: source edge, destination edge, actual travel direction, elevation relationship, reciprocal link, access condition, and whether it is seamless or a deliberate warp. Do not infer these from an illustration's arrows.

### 4.4 Geography outranks the prototype

The creator explicitly permits complete changes to existing maps and connections during early development. Do not preserve an awkward portal solely to avoid moving an NPC or rewriting a validator.

Preserve useful gameplay and coherent save behavior where possible, but let the region plan determine the physical layout. Document a required new game or migration when a structural change makes it necessary.

---

## 5. The main journey and eight-Gym structure

### 5.1 Current campaign sequence

| Stage | Destination | Main purpose | Important onward movement |
|---|---|---|---|
| Opening | Maplebrook | Home, Javorová, starter, first rival interaction. | North into Whisperwood. |
| First journey | Whisperwood | First catching, trainers, Hana's Zigzagoon, optional sanctuary. | North to Slateford. |
| Gym 1 | Slateford and Quarry | Rocket theft, mine investigation, Mara, Lomový odznak. | Northeast toward Rainfall. |
| Gym 2 | Seabreak | Mountain crossing, lighthouse, shipping clue, Vacek and Běla. | South/southwest toward the delta. |
| Gym 3 | Reedmere | River communities and flood-control story. | West toward Mosswatch. |
| Gym 4 | Mosswatch | Forest-edge community; introduction to Verdant Hollow. | Enter forest, discover return route to Maplebrook. |
| Mandatory return | Maplebrook | Javorová's new research chapter; personal starter storyline begins. | Northwest via Westwind Track. |
| Gym 5 | Ironwake | Western industry, freight links, deeper Rocket supply evidence. | New rail connection east to Slateford Quarry. |
| Mandatory return | Quarry → Seabreak | Mining breakthrough and karst shortcut; new coastal developments. | North toward Skywatch. |
| Gym 6 | Skywatch | Weather research and highland story. | Return loop to Rainfall, then northwest to Frostgate. |
| Gym 7 | Frostgate | Alpine story and northern infrastructure. | Return through mining network; then east to Crownspire. |
| Gym 8 | Crownspire | Final Gym and regional-scale stakes. | Rocket resolution and northern Victory Road. |
| League | Virelia League | Elite Four and Champion. | Postgame travel from Seabreak. |
| Postgame | Hoenn | Changed familiar region, new League hierarchy, Mega Evolutions. | Further exploration and return travel. |

This sequence fixes the intended broad rhythm, not every cutscene's badge requirement. The final placement of the storage crisis and Rocket headquarters must be integrated into it deliberately.

### 5.2 Gym identities

Mara's first Gym is Rock-themed and linked to Slateford's geology. Its reward is the **Lomový odznak**.

Seabreak is reserved for Gym 2, Reedmere for Gym 3, Mosswatch for Gym 4, Ironwake for Gym 5, Skywatch for Gym 6, Frostgate for Gym 7, and Crownspire for Gym 8. Their exact leader rosters, types, badge names, and puzzles are not finalized.

Do not automatically make every Gym use the most obvious local type. A port does not require a Water Gym, and a snowy settlement's final design should not be inferred from a snowflake drawn in concept art.

### 5.3 Adventure and pacing

The player should spend meaningful time doing ordinary trainer things between criminal incidents. A complete chapter can end with a repaired lighthouse, a changed local relationship, or a newly accessible path rather than a Rocket boss.

Early design targets were approximately 15–20 minutes for the opening prologue, 20–30 additional minutes for the first-Gym chapter, and 30–45 additional minutes for Rainfall and Seabreak. These are scope targets from development planning, not a verified total-game duration.

Beyond the opening, no definitive level curve or campaign playtime has been approved. Route length, optional content, team size, and the friends' regular level-checkpoint duels need to be considered together.

---

## 6. Routes, connections, and return loops

### 6.1 Surface route graph

The following is a **logical itinerary**, not a north-up map:

```text
Maplebrook
    ↕ Whisperwood Trail ── optional Whisperwood Sanctuary
Slateford ── separate Slateford Quarry surface ── underground workings
    ↕ Rainfall Foothills / shared trailhead
    ├── Upper Pass ─────────────────── Seabreak upper terrace
    └── Lower Pass ── river lowlands ── Seabreak harbor
                                         ↕ middle town center
                                         ↕ southern coastal road
                                      Reedmere
                                         ↕ westward riverlands
                                      Mosswatch
                                         ↕ Verdant Hollow
                                         ├── Forgotten Sanctuary
                                         ├── deeper ruins → Sunken Temple, later
                                         └── Old Ranger Trail → Maplebrook, later

Maplebrook → Westwind Track → Ironwake
Ironwake → new freight connection → Slateford Quarry
Quarry → new Stormcrest Karst crossing → Seabreak upper terrace
Seabreak → northern cliff road → Skywatch
Skywatch → alpine saddle → Frostgate → northern ridge → Crownspire
Crownspire → Victory Road → Virelia League
Seabreak harbor → international ship → Hoenn, post-League
```

### 6.2 Main connection register

| Connection | Outbound bearing | Return bearing | Intended access / purpose |
|---|---|---|---|
| Maplebrook → Whisperwood | N | S | Opening after receiving the necessary starter/equipment. |
| Whisperwood → Slateford | N | S | Opening journey. |
| Whisperwood → its sanctuary | W | E | Optional branch associated with Hana's quest. |
| Slateford → Quarry surface | ENE | WSW | First quarry chapter, **before** earning Gym 1's badge. |
| Slateford → Rainfall Foothills | NE | SW | After first-Gym progression. |
| Foothills → Upper Pass | NE | SW | Higher route option. |
| Foothills → Lower Pass | SE | NW | Lower route option. |
| Upper Pass → Seabreak upper district | ESE | WNW | High arrival, near headland access. |
| Lower Pass / lowlands → Seabreak harbor | ESE | WNW | Low arrival beside docks and river mouth. |
| Seabreak → Reedmere | S / SW | N / NE | After the Seabreak chapter and Gym 2. |
| Reedmere → Mosswatch | W, bending slightly south | E, bending slightly north | Riverlands approach after Gym 3. |
| Mosswatch → Verdant Hollow | Inland / NW, working placement | Back toward village / SE | Forest entry; final border alignment open. |
| Verdant Hollow → Maplebrook | NW overall | SE overall | Old Ranger Trail discovered after Gym 4. |
| Maplebrook → Westwind Track | NW | SE | New westward route after the laboratory return. |
| Westwind Track → Ironwake | NNW | SSE | Approach to Gym 5. |
| Seabreak → Skywatch | N, with an inland bend | S, with a coastal bend | New high-cliff journey after the Gym-5 return. |
| Skywatch → Frostgate | NW | SE | Alpine passage after Gym 6. |
| Frostgate → Crownspire | E, slight descent south | W, slight climb north | Northern route after Gym 7. |
| Crownspire → Virelia League | N | S | Eight badges and required story resolution. |

**Correction to the old route JSON:** Slateford's quarry cannot require the first badge, because investigating it is part of the story that precedes Mara's Gym. Any inherited `unlock: Gym 1` on the city-to-quarry edge must be interpreted or rewritten accordingly.

### 6.3 The shortcut schedule

| Approximate unlock | New connection | Why the story brings the player back | Lasting benefit |
|---|---|---|---|
| Early optional exploration | Whisperwood sanctuary loop | Hana's Zigzagoon leads to an overlooked forest place. | Shorter local traversal and future guardian foreshadowing. |
| Rainfall exploration | Lower-trail return footbridge | Explore the wet route and operate the local crossing. | Earlier section becomes easier to revisit. |
| After Gym 3 | Reedmere ↔ Seabreak local skiff | A local river/coast story restores or introduces the service. | Optional coastal travel; not the Hoenn ferry. |
| After Gym 4 | Verdant Hollow ↔ Maplebrook Old Ranger Trail | Bring new forest/starter evidence to Javorová. | A new southern loop, access to home and awakening research. |
| After Gym 5 | Ironwake ↔ Slateford Quarry freight rail | Cargo evidence and a mining breakthrough require a return. | Fast western-port-to-inland travel. |
| After Gym 5 | Quarry ↔ Stormcrest Karst ↔ Seabreak upper terrace | Help establish the newly discovered natural crossing. | Major permanent east–west mountain shortcut. |
| After Gym 6 | Skywatch ↔ Rainfall Upper ridge lift | Reinterpret earlier storm observations with new information. | Access to old overlooks and the Water starter's personal story. |
| After Gym 7 | Frostgate ↔ Slateford mining network | A northern water, power, or underground anomaly brings the player to Mara and the miners. | Alpine-to-inland transit. |
| Late game | Improved regional travel | Visit the research hub and earlier communities during the regional crisis. | Less repetitive movement before the finale. |
| After Virelia League | Seabreak ↔ Hoenn ferry | Begin the second-region adventure. | Reusable interregional travel. |

These badge timings are the working progression plan. The actual opening should follow a believable event, access upgrade, repair, or discovery, not an invisible badge counter acting alone.

The Frostgate connection is a transport network: shafts, station areas, and rail or maintenance passages. It is **not** one implausibly long vertical elevator that secretly travels across half the region.

### 6.4 Return visits must change something

A compulsory return needs a new scene, altered access, a changed relationship, or new information that reinterprets the location. Simply asking the player to deliver a note across a familiar route is not enough.

The player should recognize closed entrances before they open. A draft in a quarry wall, a stopped freight train, a service lift above Rainfall, and an old trail on a ranger's map all prepare later payoffs.

Shortcuts remain useful after their introductory sequence. They must actually save traversal time. Later fast travel should not be withheld indefinitely just to preserve the importance of a tunnel; the tunnel can remain relevant through its own rooms, encounters, and story.

---

## 7. Location atlas: the opening heartland

### 7.1 Maplebrook Town

**Role:** Home, starter town, recurring research hub. **Position:** Southern inland, south of Whisperwood and Slateford.

Maplebrook is small, calm, green, and inhabited. Gardens, a pond or stream, homes, a nursery, paths, and Pokémon living around residents establish a community rather than an empty introductory map.

The essential places are the player's home and Professor Javorová's laboratory. The laboratory studies regional adaptations and becomes important again when the starter's unusual traits emerge and later when the storage network becomes inaccessible.

The player's mother remains emotionally relevant. Early conversations concern ordinary travel preparations. Later she recognizes the child in news coverage and reacts to the increasing danger without becoming a recurring hostage.

Local NPC continuity matters. Hana and her wandering Zigzagoon belong to the opening forest quest. Later prototype material includes Ilona's cottage, a courier, a fisherman with Psyduck, and a local Ema with Poochyena. Do not overwrite them casually when introducing similarly named major characters.

**Revisits:** After Gym 4, the new ranger route delivers the player back here with something Javorová genuinely needs to investigate. During the storage crisis, returning is practical as well as emotional. Home remains a place whose reactions show how far the adventure has progressed.

### 7.2 Professor Javorová's laboratory

The lab is a working research space, not just a starter-distribution counter. It needs readable research furniture, room for Pokémon, practical staff movement, and a connection to local habitats.

The historical revised prologue has a small laboratory incident involving an escaped Rattata and a pursuing Meowth. The rival and player become involved before their first proper competition. Starter selection uses an actual research case rather than unrelated test NPCs.

The exact staging can evolve, but the lab has four continuing functions: introducing the regional starters, interpreting unusual biology, beginning starter-specific quest lines, and providing the physical fallback for team management during the storage lockdown.

Javorová should be competent and willing to revise her theories. Her scientific curiosity gives players reasons to return; it should not reduce Pokémon to interchangeable data samples.

### 7.3 Whisperwood Trail

**Role:** First journey, catching and exploration, early hints of guardian folklore. **Main direction:** North from Maplebrook toward Slateford.

Whisperwood is an approachable forest route with trainers, wild encounters, small branches, and a clear main road. It is not yet the overwhelming primeval wilderness of Verdant Hollow.

Hana's Zigzagoon appears to be missing, but is protecting a Wurmple nest. This establishes a useful Virelia pattern: a Pokémon's behavior has a reason, and the player learns something by investigating rather than automatically battling it.

The route should have local landmarks and connections that remain recognizable when the player returns. Any visible stream or bridge should belong to the larger landscape rather than being decorative noise.

### 7.4 Whisperwood Sanctuary

**Role:** Optional opening mystery; first hint of the later forest storyline. **Position:** West of the main trail.

An overgrown path near the Zigzagoon detour leads to an ancient tree, carved stones, and enormous pawprints that disappear around roots. The creature responsible is not fully explained.

The developed prototype puzzle uses the order **seed → rain → root**, opens a persistent local shortcut, and offers a Soothe Bell. It is a small discovery, not the completion of Wraithyena's story.

This sanctuary and the **Forgotten Sanctuary in Verdant Hollow** are separate places. Their related motifs are intentional. The later quest should make the player remember the early pawprints and carvings without retconning this first clearing into the entire ancient temple complex.

### 7.5 Slateford City

**Role:** First Gym town; mining community; recurring inland junction. **Position:** Western foothills of Stormcrest, north of Whisperwood.

Slateford is built around stoneworking and the quarry economy. It can have masonry, workers' houses, workshops, freight infrastructure, retaining walls, and a rocky backdrop without turning every street into an industrial dungeon.

Mara is a geologist and Rock-type Gym Leader. Her challenge belongs to the town's identity, but she continues to matter after the badge by interpreting mineral evidence and later underground discoveries.

The first Rocket incident concerns a suspicious “maintenance worker” and stolen conductive minerals. The grunt's attempt to explain the conspicuous R as a maintenance-related logo is an example of the intended humor. The player handles the battle; the policeman arrives afterward and establishes the credit-taking pattern.

Slateford connects south to Whisperwood, east/northeast to its separate quarry surface, and northeast to Rainfall's western approach. Later rail and cave routes make it a familiar transport junction.

### 7.6 Slateford Quarry — separate outdoor surface

**Established structural decision:** The quarry deserves its own outdoor map. It is not merely a cave doorway on a city street.

The surface contains stepped excavation terraces, loading areas, freight tracks, equipment, worker buildings, marked mineral seams, and entrances into the underground maps. Its open-pit geometry should visibly cut into the western mountain face.

The early mine story involves restoring damaged equipment, exploring the workings, and finding evidence connected to the stolen cargo. That story precedes the first Gym. The existing characters Bruno and Lenka, the pump/power sequence, and the freight evidence can survive a physical redesign.

On the first visit, show a closed freight siding and signs of deeper cavities: a strange draft, the sound of running water, or rock strata that interest Mara. Those details later justify the Ironwake railway and the karst breakthrough.

The quarry becomes an evolving location, not simply three separate dungeons that happen to share a name.

### 7.7 Quarry Entrance, Depths, and Gallery

The underground sections already have distinct roles in the design: working mine entrance, deeper machinery and cargo evidence, and an optional gallery that creates a traversal loop.

The **Missing Tools** side quest belongs in the gallery. Miners suspect more Rocket theft; the actual culprit is a Sableye gathering objects around its hoard. Clues include scattered tools and traces of activity. A normal wild encounter allows battle or capture; resolving it returns the tools and earns a modest reward such as the developed Hard Stone reward.

The side quest must not be required to unlock Mara's Gym. The loop through the gallery should remain useful independently of its encounter.

The later natural cavern branches from the mining network without erasing the earlier machinery puzzle, Sableye area, or local characters. Engineered supports and rails should gradually give way to irregular natural rock, underground water, and mineral chambers.

---

## 8. Location atlas: Stormcrest and the eastern coast

### 8.1 Rainfall Foothills and the shared trailhead

**Role:** Transition out of Slateford and the point where the mountain crossing becomes a choice.

The foothills begin on the inland side. Stone paths, smaller trees, rising slopes, and the view of higher cliffs prepare the player for a crossing rather than an unrelated forest route.

The mandatory second rival encounter should occur before the two paths divide, or use an equally reliable alternative that preserves both arrivals. The working preference is a shared trailhead encounter. The earlier implementation's rival gate in the combined lowlands must not force the revised routes back together.

The player should understand the choice from both visual design and a short sign or NPC hint: the upper path is more exposed and trainer-oriented; the lower path is wetter and more exploratory. Neither is the “wrong” route.

### 8.2 Rainfall Upper Pass

**Role:** High crossing, vistas, bridges, and a distinct arrival above Seabreak.

The upper route climbs through rock cuts, mossy steps, higher woodland, exposed cliff paths, and bridges. Its principal landmark is a waterfall or gorge that reveals the scale of the mountain crossing. The lower valley may be visible from overlooks, making the alternatives feel like parts of one landscape.

Terrain should vary through shelves and switchbacks, not repeat the same cliff wall around a two-tile corridor. A short side spur can lead to a ranger lookout, a sheltered pool, or a useful item.

This route reaches Seabreak's **upper terrace**, not a shared lowland checkpoint. The lighthouse headland and the town descending toward the sea provide a strong arrival scene.

Later, a weather-service lift connects the upper pass to Skywatch. Returning with new information turns an old viewpoint into a place of investigation and supports Nimbatu's personal storyline.

### 8.3 Rainfall Lower Pass and river lowlands

**Role:** Lower crossing through gorges, streams, marshy shelves, and wet forest.

“Lower” means lower than the ridge route, not flat or automatically at sea level. It can still cross a low saddle or gorge through the mountain barrier before descending along the coastal drainage.

Wooden walkways, irregular pools, reeds, riverbanks, small cascades, and sheltered clearings distinguish it from the upper route. Wild encounters and optional exploration are more prominent; trainers should not simply be copies of those on the high path.

The developed lowlands map can become this route's final descent instead of the compulsory merge point for both routes. It reaches Seabreak's **harbor-side district**, where a stream or river mouth meets the bay.

A local return footbridge or winch crossing can provide an early loop back toward the foothills. This is separate from the much more important midgame tunnel through the range.

### 8.4 Seabreak Town — overall identity

**Role:** Second Gym settlement, recurring coastal hub, lighthouse story, and eventual gateway to Hoenn. **Position:** Eastern windward coast, across Stormcrest from Slateford.

Seabreak is a rocky port adapted to repeated rain and rough weather. It is not a tropical resort pasted onto a grass rectangle. Stone stairs, retaining walls, drainage channels, sheltered doorways, slate or blue-tiled roofs, working docks, and sturdy houses communicate how people live here.

The town descends across **three connected terraces**. Both route entrances must lead to services without requiring the lighthouse story to be completed. A person arriving from below should be able to walk up to the Center; a person arriving from above should be able to descend to the docks.

The settlement is one place even if its districts become separate map files. Shared landmarks, repeated materials, visible neighboring terraces, and consistent stair connections maintain that unity.

### 8.5 Seabreak upper terrace

The upper district receives the Upper Pass. It contains cliffside homes, viewpoints, the lighthouse keeper's area, access to the lighthouse headland, and the eventual cave exit from the quarry crossing.

The cave opening should be present or foreshadowed before it becomes usable. Its earlier explanation can be unstable rock, an inaccessible recess, or an unknown passage. It must not appear suddenly as an unannounced teleport.

A high viewpoint lets the player understand the lower town and working harbor. Access paths should bend logically around the headland rather than cross empty ocean or place the lighthouse in an arbitrary corner.

### 8.6 Seabreak middle terrace

The middle district is the civic and service center: Pokémon Center and shop access, market activity, homes, a public space, local administration, and the future Gym.

A combined Center/Mart follows the later prototype convention, but the world design does not depend on keeping both services in one building. Choose the arrangement for the town's composition and player convenience.

This is a good location for recurring character meetings that do not obstruct the main stairway. It should feel like a place where locals actually gather, not just a healing stop between two story triggers.

### 8.7 Seabreak harbor level

The lower district contains fishing docks, cargo handling, warehouses, workers, small boats, a seawall, and the future international terminal. The Lower Pass arrives near this level.

The harbor must be readable as a working port: boats can reach moorings, cranes face loading areas, crates have sensible storage spaces, and pedestrian routes do not run through every cargo pile. The coastline should form a sheltered working basin, not an unexplained collection of rectangular piers in open surf.

The first major story reopens local operations. The **Hoenn service remains a separate post-League unlock**. A functioning local harbor does not automatically grant overseas passage.

### 8.8 The lighthouse and headland

**Role:** Seabreak's primary visual landmark and the focus of its first main-story problem.

The lighthouse should be recognizable through its silhouette: a pale or weathered stone tower, distinct lantern chamber, durable roof or dome, visible entrance, and a foundation integrated into the rocky headland. Warm beacon light contrasts with the cool storm coast.

Its interior is practical Pokémon-world infrastructure. The existing story needs a lower equipment area and an upper lantern/control area. Four-floor drawings, observation decks, and living quarters are optional art concepts, not a requirement to quadruple the dungeon.

The lighthouse sequence introduces Běla as someone who makes the player's work safer and more effective. She can secure dangerous wiring, explain what is damaged, or arrange access. The player restores the navigation system through a short readable puzzle.

The developed control order is **feed → lens → signal**. Its exact form may improve during a visual redesign; it should become a spatially understandable mechanism rather than a row of arbitrary switches.

After the repair, a shipping record connects Seabreak's missing cargo to Slateford's conductive minerals. The intended recipient is obscured or absent. The scene advances a mystery without revealing the Porygon operation or turning the lighthouse into another Rocket base.

### 8.9 Seabreak's return stories

Seabreak's first visit introduces its people and navigation problem. After Gym 5, the new quarry cavern delivers the player into the familiar upper district with new evidence and a different route through the world. Later visits can involve weather observations, freight investigations, or the regional crisis.

After the League, the harbor's international departure becomes the threshold of a new adventure. The emotional payoff comes from leaving for Hoenn through a town the player already knows, not through a newly invented postgame menu.

### 8.10 Stormcrest Karst Caverns

**Role:** Midgame discovery and permanent cross-mountain shortcut. **Working unlock:** After Gym 5 and the return from Ironwake.

Miners uncover a connection to a natural cavern system while extending or investigating deeper workings. The setup is the familiar “they dug too deep” discovery, but it need not release a supernatural evil. The important revelation is geological and spatial: the mountain contains a passage toward the coast.

The first traversal can involve underground water, unstable sections, new mineral formations, and a small task to make the route safe. The scenery transitions from worked mine to natural cave and finally to coastal daylight.

The exit reaches **Seabreak's upper hillside**, not the harbor floor and not an unrelated northern town. The tunnel's internal changes of elevation must make that emergence believable.

Once established, the usable crossing should be meaningfully shorter than Rainfall. Optional branches, a fossil chamber, or deeper exploration can remain without making routine travel through the shortcut another long dungeon every time.

---

## 9. Location atlas: the southern wetlands and ancient forest

### 9.1 Reedmere

**Role:** Gym 3 and the southern river-delta chapter. **Position:** South/southwest of Seabreak, east of Mosswatch.

Reedmere is a settlement shaped by water rather than simply surrounded by it. Raised homes, boardwalks, local boats, channels, locks, reed beds, and flood-control structures explain how its residents travel and work.

Its local story should establish a community with its own concerns. Flood management, a disrupted canal, or the coexistence of people and wetland Pokémon can carry a chapter without requiring another Rocket facility.

A short local skiff service back to Seabreak becomes an optional reward after this portion of the journey. It can be useful for returning to the coast while the main land route continues west through riverlands toward Mosswatch.

Possible optional places include a drowned bell tower, a disused lock, a quiet marsh boardwalk, and small islands. Real-time tides or nighttime-only requirements are not yet committed mechanics.

### 9.2 Mosswatch Village

**Established revision:** Mosswatch is the human settlement and Gym-4 location. Verdant Hollow is not a city inside the oldest forest.

The working history is a former logging outpost transformed into a conservation, research, and guiding community. That gives it reused buildings and an imperfect past without making every inhabitant a villain.

Mosswatch provides healing, supplies, ordinary homes, a Gym, ranger facilities, and people who understand the forest's boundaries. Its visual identity is small-scale: timber, plants, stone footings, field equipment, sheltered paths, and a watchtower or research lodge.

The Gym Leader could be a ranger or botanist; a Grass-, Bug-, or Poison-oriented team was suggested, but no type or name has been selected. The Gym should reflect the leader's relationship to the forest rather than being an ancient temple appropriated as a sports arena.

The player's first forest visit is supported from Mosswatch. Later, the village responds to discoveries, rescues, and research deeper in the jungle.

### 9.3 Verdant Hollow — the ancient rainforest

**Role:** Major wilderness area, Wraithyena's awakening setting, and a layered late-game discovery zone.

Verdant Hollow is a large protected rainforest adjoining Mosswatch. It contains massive trees, dense canopy, tangled roots, springs, waterfalls, flooded basins, abandoned ranger infrastructure, and remnants of an old culture.

It should feel older, deeper, and less domesticated than Whisperwood. Its scale comes from multiple connected spaces with different identities, not a huge maze of identical trees.

Humans have marked certain outer trails. Beyond those, the forest is less predictable. The first visit reveals pieces of masonry, unusual tracks, an old camp, or a carved stone that turns out to belong to something much larger. It does not reveal the full temple immediately.

Verdant Hollow is useful to every starter choice. The Grass-specific ability reward is one personal story inside a broader wilderness; players who chose Natu or Ekans must not lose access to the region's major ancient dungeon.

### 9.4 First layer: the outer forest

**Approximate timing:** Around Gym 4.

The outer maps introduce ranger work, unusual Pokémon behavior, manageable ruins, and optional branches. A canopy walk, a basin, an old camp, and an overgrown trail can provide distinct landmarks.

The first major return loop is discovered here: the **Old Ranger Trail toward Maplebrook**. It makes the journey back to Javorová an exploration payoff, not a request to walk the original route backward for twenty minutes.

The player finds evidence that echoes Whisperwood's sanctuary, especially the vanishing pawprints and related carvings. This evidence supports the starter research chapter without proving that every forest mystery is about the player's Pokémon.

Deeper access is limited by convincing terrain and information: collapsed paths, flooded sections, roots, unsafe ruins, or a route that requires a later understanding. Avoid a generic person blocking the only doorway with no ecological or story reason.

### 9.5 The Old Ranger Trail

**Role:** Permanent forest-to-home shortcut and a main-story return after Gym 4.

The trail connects Verdant Hollow's outer region toward Maplebrook's southeastern outskirts. Its overall bearing is northwest when returning home.

Its first traversal should reveal traces of former use: ranger markers, a broken shelter, old bridges, or maps that make its relationship to Maplebrook comprehensible. Opening the route may be part of the forest chapter, but routine travel afterward should be simple.

This path establishes Javorová's lab as the campaign's recurring research hub. The compulsory return introduces a personal mystery; completing the full Hidden Ability storyline remains optional.

### 9.6 The Forgotten Sanctuary

**Established role:** The location within Verdant Hollow where Wraithyena's Hidden Ability is unlocked.

The sanctuary is a distinct inner clearing or ruined sacred site tied to an ancient guardian lineage. A great tree, weathered monoliths, roots, reflective pools, and restrained ghostly lights support its identity.

The personal quest begins through Javorová, not by randomly walking up to an altar with any Grass Pokémon. Research connects the player's starter, early Whisperwood clues, and the forest's deeper history.

The proposed trial structure emphasizes understanding and protection: help nesting Pokémon, restore a damaged spring, and navigate misleading spirit paths. Battles can be included, but “defeat a stronger copy of yourself” should not be the entire awakening.

Wraithyena is recognized as a guardian. Its Hidden Ability is an expression of that role, not a generic consumable item found in a chest.

The exact ability remains undecided. The sanctuary's narrative must therefore stay compatible with perception, protection, stealth, or spirit-world movement rather than promising a particular unapproved mechanic.

### 9.7 Ancient ruins throughout Verdant Hollow

The ruins should be distributed, not contained solely inside one clearly marked late-game building. Outer fragments, old water channels, fallen pillars, buried stairs, and partial reliefs form an archaeological trail.

A ranger or researcher can first misunderstand a structure as a shrine, then later recognize it as part of a larger approach. A wall visible across a flooded basin can become reachable on a later visit. This creates anticipation without requiring every early visual hint to grant a quest immediately.

Patterns relating to forest, storm, and subterranean heat can connect the starter lineages. Their exact interpretation is a working mythology, not a complete historical account already known to the characters.

### 9.8 The Sunken Temple

**Established direction:** A substantial ancient temple discovered later, deep in the jungle. **Working timing:** Deep exploration around Gyms 6–7; deepest content later in the campaign or postgame.

The temple is a separate location from the Forgotten Sanctuary. It should be a meaningful dungeon for every player, regardless of starter.

Architecture and nature have become interdependent: roots break through ceilings, underground water fills lower chambers, bridges span collapsed floors, and light enters through gaps far above. An exterior fragment seen earlier may turn out to be the top of a much larger complex.

Puzzles should arise from the place—water levels, surviving mechanisms, routes through collapsed masonry, or interpreting a sequence of environmental clues—rather than unrelated minigames.

A central petrified tree whose roots appear to extend beneath much of the rainforest was proposed as a major reveal. A relationship to Whisperwood could explain recurring motifs, but the literal extent of those roots and any supernatural connection remain open.

The ancient civilization's name, age, full history, and eventual guardian encounter have not been chosen. Do not insert a legendary Pokémon or an entire pantheon without a separate decision.

### 9.9 Forest discovery schedule

| Visit | What changes |
|---|---|
| Around Gym 4 | Outer trails, ranger work, early ruins, the return trail to Maplebrook. |
| After laboratory research and appropriate evolution | Grass-starter personal quest can reach its sanctuary climax. |
| Around Gyms 6–7 | New access or knowledge reveals deeper jungle and the temple approach. |
| Late game / postgame | Deeper temple chambers, major optional discoveries, unresolved historical questions. |

Exact badge gates must remain flexible enough that a player who has not evolved their starter can still advance the main story. Wraithyena's awakening does not become a mandatory key to everybody's temple access.

---

## 10. Location atlas: the west and northern highlands

### 10.1 Westwind Track

**Role:** New northwest journey after returning to Maplebrook. **Position:** Between the home region and Ironwake.

Westwind Track changes the meaning of the starting area. A western route seen or mentioned early becomes the path forward after the Gym-4 research chapter.

The terrain can shift from woodland to exposed escarpments and western coastal approaches. It should reveal a different side of Virelia rather than reuse Whisperwood with stronger encounters.

The route's exact submaps and local story are open. Its geographic function is fixed in the working plan: Maplebrook northwest → Westwind → Ironwake to the north/northwest.

### 10.2 Ironwake

**Role:** Gym 5, western industry, and the freight connection back to Slateford.

Ironwake is a working industrial harbor. Shipyards, cranes, rail yards, warehouses, workshops, workers' neighborhoods, and old freight infrastructure distinguish it from Seabreak's smaller rainy fishing and navigation hub.

The city should show ordinary labor and community life alongside its large-scale machinery. Industry is not automatically villainy. Rocket can exploit trade and logistics without every factory being a secret base.

Ironwake is a natural place to connect the earlier missing-mineral evidence to a broader supply network. It can also support the storage engineer's introduction or an early hint of research being misused.

After Gym 5 and the local story, the freight connection east to Slateford Quarry becomes available to the player. The journey returns them to miners they know, now facing a discovery that matters beyond the first town.

Optional exploration can include a ship graveyard, a repair shop's missing parts, an abandoned rail spur, or a worker's Pokémon problem. The Gym's type and leader are open.

### 10.3 Skywatch

**Role:** Gym 6, weather research, and high eastern mountain travel. **Position:** North of Seabreak, on elevated eastern slopes.

Skywatch is an observatory settlement, not necessarily a lonely telescope on an otherwise inaccessible peak. It needs homes, services, maintenance routes, research staff, and a reason people can survive there.

Its visual identity combines cloud-level walkways, weather instruments, research domes, mountain stone, and practical cable or service infrastructure. It remains a modern Pokémon research community even if the concept art uses ornate architectural forms.

The region's storms can be studied here without turning all weather into a human-created conspiracy. Skywatch provides a good narrative location for Nimbatu's lineage, unusual atmospheric behavior, and the difference between predicting a storm and commanding it.

The working shortcut is a weather-service ridge lift connecting southwest to Rainfall Upper. Its opening sends the player back to a familiar overlook with a different question. The route onward heads northwest toward Frostgate.

### 10.4 Frostgate

**Role:** Gym 7 and the northern alpine chapter. **Position:** A snowy basin northwest of Skywatch.

Frostgate's character comes from mountain life: snow, sheltered paths, maintained crossings, warm interiors, ice, and possibly hot springs. It does not have to be a literal medieval fortress because an illustration depicts a gatehouse.

The northern passages should feel more demanding than Rainfall while remaining geographically related to the same mountain system. A glacial cave, snowmelt channel, or maintenance shaft can reveal why the alpine basin matters to the region below.

The working return loop connects Frostgate's maintenance/ore network south-southwest toward Slateford Quarry. A new underground or infrastructure anomaly brings Mara and the early miners back into the story.

The next main destination is Crownspire to the east. A late forest-temple discovery can become available during this part of the campaign, but it need not be forced into Frostgate's main quest.

### 10.5 Crownspire

**Role:** Gym 8, regional capital, and the approach to final campaign stakes. **Position:** Northeastern plateau, east of Frostgate.

Crownspire houses civic institutions, records, regional authority, major infrastructure, and a more urban population than previous towns. Public plazas, layered streets, archives, and modern administrative buildings can coexist with older stone architecture.

Its grandeur should remain within the Pokémon world. There is no established king, monarchy, cathedral hierarchy, or feudal order behind the concept images.

The city is a plausible place for the final consequences of the storage crisis, official claims about the Rocket investigation, and Vacek's absurd public stature. It is a candidate setting for major endgame operations, not a finalized location for every villain scene.

The eighth Gym's type, leader, and relationship to the political/infrastructure story remain open. The northern exit leads toward Victory Road once the required badges and story conditions are met.

### 10.6 Victory Road and the Virelia League

**Role:** The final trainer challenge of the main region. **Position:** North of Crownspire.

Victory Road should represent the culmination of the player's travel and team-building, not simply a cave full of high-level encounters. Mountain paths, difficult terrain, and a clear final approach can connect it to the region's geography.

The League is an official Pokémon institution. Its final visual identity may use a northern citadel silhouette or a dramatic plateau complex, but the exact buildings, Elite Four, Champion, and battle teams remain undesigned.

The working campaign resolves the major Rocket/storage crisis before the player finishes the League. Defeating the Elite Four and Champion then marks the trainer journey's culmination and unlocks the next phase: Mega Evolution and the voyage to Hoenn.

### 10.7 Optional offshore spaces

Small coastal islands, tide caves, a shipwreck, and remote wildlife habitats are reserved as optional future spaces. Their exact count, names, locations, and traversal methods are open.

Local boats must be distinguished from the international Seabreak–Hoenn ship. A player can explore Virelia's coast before becoming Champion without obtaining early access to the second region.

---

## 11. The main story and Team Rocket

### 11.1 The public story

A child receives a regional starter, travels across Virelia, and seeks badges. Along the way, the player keeps finding local incidents that the authorities fail to resolve effectively.

Initially these incidents are small enough to be mistaken for ordinary theft or incompetence. Minerals disappear. A maintenance worker asks the wrong questions. A shipment lacks a recipient. Equipment is damaged. The same people and symbols begin to recur.

The player is not told on the first route that they must save the world. The larger story emerges from the trainer journey.

### 11.2 The organization

Team Rocket is operating under new leadership. It steals Pokémon, equipment, materials, and access, and uses a network of ordinary logistics to conceal a more invasive project.

There is only one major evil organization. Internal disagreements, opportunistic grunts, a ruthless technical program, and Giovanni's hidden agenda create different perspectives within and around it.

The current leader's name remains open. An earlier idea imagined a brilliant, cynical, abrasive antagonist with a Rick-Sanchez-like attitude. The intended reference is intellectual arrogance and dangerous indifference, not a direct copy of that character or a science-fiction multiverse plot.

### 11.3 Leader and scientist: a working role split

A later proposal separates the current Rocket leader from a chief scientist:

| Role | What they want | Narrative function |
|---|---|---|
| Current Rocket leader | Power, leverage, and control of the organization. | Direct political/organizational enemy. |
| Chief scientist | Proof that their methods work; forced technological control over Pokémon. | Architect and rationalizer of the Porygon experiments. |
| Giovanni | The organization, its surviving assets, and restored authority. | Hidden beneficiary who manipulates the campaign from outside its current hierarchy. |

This split is useful but not fully locked. The leader and scientist can be combined if a tighter cast works better. Do not invent two final names or separate boss campaigns merely because this document describes the option.

Javorová and the scientist form a thematic contrast: she studies and respects adaptation; the scientist forces Pokémon into the requirements of a machine.

### 11.4 The Porygon installation

The central image is a facility where captive Porygons are forcibly interconnected and repeatedly powered or stressed by electrical Pokémon and machinery. Serial/parallel arrangements and compute-farm imagery inform the visual concept.

The in-world purpose is to exploit Porygons' relationship to digital systems and gain control over the Pokémon Storage network. It is not primarily a story about training an AI model. Technical flavor should not overwhelm the Pokémon stakes.

The electrical Pokémon should also be treated as creatures being used, not as morally neutral batteries with no role in the rescue.

The player can see that the project is unstable and harmful before understanding every detail. The villains' language can be cold and procedural; Javorová, Klára, and the rescuers restore the emotional meaning of what the equipment contains.

### 11.5 Broad escalation

| Phase | What the player experiences | What remains hidden |
|---|---|---|
| Opening / Slateford | Theft of conductive minerals; first Rocket grunt. | Why these materials matter. |
| Seabreak | Interrupted shipping and an incomplete cargo record. | The final destination and project. |
| Southern journey | Local adventures, forest research, recurring people. | The full scale of the operation. |
| Western / midgame | Supply-network connections and evidence of organized exploitation. | Giovanni's involvement and exact endgame. |
| Later campaign | Captive Porygons and electrical Pokémon; growing storage failures. | The identity and motive of the mysterious ally. |
| Regional crisis | Storage access is blocked; the player travels through the now-connected region to help. | Who will benefit when current Rocket leadership falls. |
| Finale | The operation is stopped and Pokémon are rescued. | Giovanni's concealed agenda becomes explicit. |
| Aftermath | Recovery, public recognition, Vacek's inflated triumph, League journey. | Team Rocket survives under the returning boss. |

Exact badge placement is open. The framework should not be mistaken for a mandate to put a Rocket battle after every Gym.

---

## 12. The Pokémon Storage crisis

### 12.1 The gameplay premise

**Established:** During the crisis, ordinary PCs in Pokémon Centers cannot be used to change the player's team. The player can physically return to Professor Javorová and swap Pokémon there.

The premise creates a temporary change in planning and gives the laboratory real importance. It must not destroy the player's Pokémon, permanently trap a team, or turn the whole game into an inventory punishment.

### 12.2 What remains functional

Healing at Pokémon Centers should remain available unless a particular local story explicitly says otherwise. Blocking remote storage is not the same as shutting down every medical service.

The player's current party remains usable. Ordinary saving must work. Existing Pokémon and items must never be lost as a side effect of the plot.

At Javorová's lab, a controlled physical/local process supports team changes. The story explanation must be consistent with the final storage implementation: local custody, an isolated safe facility, or an emergency transfer method that genuinely does not rely on the compromised ordinary access path.

Do not solve this by giving the laboratory an identical networked PC with a different-colored screen and no explanation.

### 12.3 Travel and fairness

By the time the lockdown becomes active, the map should already offer several meaningful return routes. The Old Ranger Trail, freight access, and other established shortcuts make the lab a practical hub rather than a long forced walk.

The player must always have a safe way to return. Required progression must not demand a Pokémon or field move that is inaccessible inside blocked storage. No multiplayer partner should be required to rescue a save from a bad team composition.

The precise duration and geographic reach of the crisis are open. The working scope is Virelia; do not claim the entire Pokémon world is disconnected without developing that consequence.

### 12.4 Important unresolved edge cases

The following need explicit design before implementation:

- What happens when the player catches a Pokémon with a full party during the crisis?
- Which boxed Pokémon are accessible at the lab, and how is that access explained?
- How do trades, gift Pokémon, breeding, and any future party-selection menus behave?
- Can all storage-like interfaces accidentally bypass the intended restriction?
- How is the end of the crisis announced and ordinary access safely restored?

The chosen answer must preserve ownership and be understandable. Do not silently discard captures or replace blocked PC access with a technical loophole.

### 12.5 Resolution

The rescue of the Porygons, dismantling of the abusive setup, and restoration of storage are real achievements. They remain true after Giovanni's reveal.

Klára, Javorová, Běla, the player, and potentially the multiplayer partner each have meaningful contributions. Vacek receives public credit far beyond his actual contribution, but he may genuinely help authorize access at a critical moment.

---

## 13. Giovanni: the useful ally and hidden beneficiary

### 13.1 Core twist

**Established direction:** A mysterious man intermittently helps the player against the new Rocket leadership. Later the player discovers that the man is Giovanni and that their successes have enabled him to take control of Team Rocket again.

This is the Virelia story's own use of Giovanni. Its exact placement in official game chronology should be checked separately if strict continuity becomes necessary. The project has discussed a period after his earlier defeats and withdrawal; it has not approved a precise calendar or a multiverse explanation.

### 13.2 Why his assistance is effective

The man gives useful information. His warnings lead to real danger. The captive Pokémon really exist. Routes he reveals really work. He does not need to fabricate every fact because the player and he share an immediate opponent.

His hidden objective is to remove disloyal leadership, gather access and records, identify useful personnel, and preserve parts of the organization that he can later reclaim.

The protagonist is not foolish for accepting help that saves Pokémon. The manipulation lies in what the man does not disclose and in the wider consequences of the player's victories.

### 13.3 Character boundaries

Giovanni is not secretly a reformed good person. He may criticize the current operation as wasteful, uncontrolled, conspicuous, or strategically stupid. That is not the same as adopting the player's ethics.

He should not be omnipotent. He needs the player's actions because he lacks direct control, cannot safely expose himself, or wants rivals removed without destroying the assets he intends to inherit.

His apparent resources and knowledge must leave fair clues without making the reveal obvious at the first meeting. A civil disguise, restrained dialogue, organizational knowledge, and ambiguous references to past management are more useful than immediately displaying every recognizable symbol.

### 13.4 Suggested progression

Initial help can begin after the player has already become a credible obstacle to Rocket. Later information points toward more important operations. During the crisis, his contacts or knowledge become especially useful. Only after the current leadership is defeated does the full purpose become clear.

The preferred timing is a late reveal, after the player has removed the decisive obstacle to his return. Exact Gym timings and individual meetings are still open.

### 13.5 The reveal's meaning

The most important principle is that Giovanni's victory does not erase the player's victories.

The Porygons remain rescued. The destructive facility remains disabled. Storage is restored. Communities are safer. Giovanni wins by taking the remaining organization, its finances, networks, contacts, and loyalty—not by revealing that every rescued Pokémon was secretly doomed anyway.

A proposed line captures the change in approach:

> „Před lety jsem se pokusil jednoho mladého trenéra zastavit. S tebou jsem zvolil jiný přístup.“

A Rocket member's recognition and the line below can deliver the identity reveal without a long explanatory monologue:

> „Samozřejmě, že mě znáš. Koneckonců pořád nosíš moje logo.“

Dialogue is illustrative, not a finalized script.

### 13.6 Giovanni after the reveal

His exact exit, whether he battles immediately, and whether there is a later optional rematch remain open. A postgame invitation was suggested, but is not a requirement.

Do not automatically make him the sole antagonist of Hoenn too. Hoenn deserves its own chapter and changed social world.

Giovanni recognizes Vacek's incompetence as an advantage. A celebrated but ineffective anti-Rocket commander is useful cover for a returning criminal leader. Whether Vacek ever learns who the mysterious man really was is unresolved.

---

## 14. The recurring cast

### 14.1 The player

The protagonist is a child beginning a normal trainer journey. Their competence grows through battles, exploration, and choices. Adults may underestimate them, but the story should not treat every adult as equally incapable.

The player is the emotional center of the starter storyline and the practical agent who connects the region's communities. Dialogue should preserve some room for player interpretation rather than assigning a strongly defined sarcastic personality in every scene.

### 14.2 Viktor / Viktorie — the NPC rival

The revised prototype supports Viktor for a female player and Viktorie for a male player. Retain that established implementation unless intentionally redesigned.

The rival is confident, competitive, a little sarcastic, and ultimately a companion on the same kind of journey. They choose the established counter-starter and return with a developing team, not just the same Pokémon at a higher level.

The rival can react to the first battle's outcome, the first badge, different route choices, and growing Rocket incidents. They are especially irritated by Vacek's undeserved reputation and may repeatedly attempt to expose him, only to make his apparent genius larger.

Their role is separate from the multiplayer rival. Do not replace the real friend with another NPC or make the NPC speak as though they are the second player's avatar.

### 14.3 The multiplayer rival

The friend's trainer is a real protagonist in their own save, a cooperative partner during shared missions, and an opponent in friendly battles.

Both trainers should be framed as competent peers. One must not permanently become the “real hero” while the other is a temporary guest. The rivalry is personal and playful, not an additional villain arc.

### 14.4 Professor Javorová

Javorová studies regional adaptation and gives the player their starter. She is the scientific and practical anchor of Maplebrook.

Her recurring roles are research, interpretation of unusual starter behavior, emergency support during the storage crisis, and helping turn folklore into questions the player can investigate. She should not know the entire ancient history from the beginning.

Her relationship with Matěj can be argumentative and affectionate: she wants evidence; he remembers observations that do not fit her current explanation. Neither must always be right.

### 14.5 The player's mother

The mother grounds the story in home. Her changing dialogue acknowledges the player's badges, absence, and appearance in reports about dangerous incidents.

Her humor is practical rather than incompetent. She can worry about clean clothes and still understand that the situation has become serious. Home remains a welcoming return point rather than a place that only gives tutorial instructions.

### 14.6 Mara

Mara is Slateford's geologist and first Gym Leader. Knowledgeable, direct, and dry-humored, she treats both geology and battles as subjects requiring observation.

She can identify why the stolen minerals were unusual, help interpret the mining breakthrough, and become involved when the northern infrastructure story leads back to Slateford. Her first badge is the start of a relationship, not the end of her relevance.

### 14.7 Radim Vacek

Vacek begins as the recurring policeman after the Slateford incident. His main role is the rapidly expanding gap between public reputation and actual competence.

He is cowardly around danger, opportunistic about credit, and good at protecting himself through paperwork. He is not a secret Rocket agent and should not deliberately endanger children merely to preserve the joke.

His full arc appears in the next section.

### 14.8 Běla Křížová

**Working supporting character:** Vacek's competent junior colleague.

Běla initially respects his record. Over successive incidents, she notices who actually found the clues and won the battles. Her frustration develops into practical strategy: Vacek's reputation can authorize things that genuinely help the player.

She provides real support—safe access, evacuation, evidence handling, coordination—so the police are not uniformly useless. She is not merely a reaction face for Vacek's jokes.

Her eventual promotion choice and exact Pokémon team are open. Earlier suggestions duplicated Growlithe / Arcanine between her and Vacek; their signature teams should be differentiated before finalizing them.

### 14.9 Ema Holubová — proposed reporter

**Working supporting character:** A reporter whose initial coverage helps manufacture Vacek's legend.

Over time she notices the same child in the background of every success and begins investigating the discrepancy. She can also become curious about the mysterious gentleman's appearances around Rocket defeats.

A Chatot repeating inconvenient comments was proposed as a companion gag, but is not finalized.

**Continuity warning:** Maplebrook already has a local NPC named Ema with a Poochyena in the recorded prototype. The reporter must either be a consciously developed version of that character or receive a distinct name. Do not silently turn the existing local girl into an unrelated adult journalist.

### 14.10 Klára Horská

**Working supporting character:** A storage-system engineer and former student of Javorová.

Klára helped develop technology intended to make storage more reliable. Rocket has acquired or repurposed some of that work without her understanding the consequences.

When failures begin, she is blamed or blames herself. Her arc is not simply “the anxious genius causes everything”; she becomes an active part of diagnosing the misuse, helping the player, and restoring the system.

A Magnemite line and a rescued Porygon were proposed as partners. Their final evolution and timing are open.

### 14.11 Matěj Vrbka

**Working supporting character:** An elderly former ranger with decades of observations, journals, and memories of older routes.

Matěj helps connect forest folklore, environmental changes, and the starter lineages. He should know enough to guide research, not enough to make the player's discoveries unnecessary.

His earlier proposed home was Maplebrook's closed cottage. Later prototype work opened a cottage for Ilona. His residence therefore needs a deliberate decision: a separate ranger hut, a role elsewhere, or an intentional relationship to the current household. Do not overwrite Ilona by accident.

An old Tropius, Noctowl, or Shiftry was suggested as a companion. None is a locked final roster.

### 14.12 Ruda “Šroub”

**Working supporting character:** The recurring Rocket logistics employee, ideally developed from the early Slateford grunt rather than added as a wholly unrelated face.

Ruda wants a wage, a tolerable assignment, and to avoid getting electrocuted. Each encounter gives him a worse excuse and a different undesirable job.

At first he is comic relief. Seeing the Porygon operation changes what he is willing to ignore. He may become an informant or try to leave, without immediately turning into a spotless hero.

Giovanni's questions about supplies, staff, and access can unsettle him before he recognizes the former boss. Ruda provides a ground-level view of the organization and a believable route for information.

### 14.13 The current Rocket leader and scientist

The current leader wants power and command. The scientist wants their program to work and treats harm as an engineering inconvenience. Their distinction, names, appearance, signature teams, and final battle sequence remain open.

They should each have a reason to distrust or fear the mysterious ally's interventions, while misunderstanding how much of their trouble is actually caused by the young trainer.

### 14.14 Giovanni

Giovanni is the outsider whose apparent assistance makes immediate problems easier while advancing his own long game. He understands Ruda, Vacek, and the current Rocket hierarchy more clearly than most of the cast.

He should be used sparingly enough that his appearances matter. The twist works through cumulative evidence and omitted motive, not endless lies or constant threatening cutaways.

### 14.15 Relationships, not isolated quest dispensers

The supporting cast should interact with one another:

| Relationship | Dramatic use |
|---|---|
| Javorová ↔ Matěj | Evidence and scientific models versus long observation and folklore. |
| Javorová ↔ Klára | Mentorship, trust, and responsibility during the storage crisis. |
| Vacek ↔ Běla | Inflated authority versus actual competence. |
| Vacek ↔ reporter | The creation and eventual questioning of a public myth. |
| Rival ↔ Vacek | Frustrated attempts to expose obvious credit theft. |
| Mara ↔ miners | A Gym Leader embedded in the town's everyday work. |
| Ruda ↔ scientist | A low-ranking employee confronting what the job really means. |
| Giovanni ↔ Ruda | Organizational knowledge that hints at the stranger's identity. |
| Giovanni ↔ Vacek | A competent manipulator benefits from an incompetent official. |

---

## 15. Vacek: falling upwards

### 15.1 The central joke

The model is the *falling-upwards* dynamic associated with Buggy: a reputation built from misunderstood events becomes vastly larger than the person's competence.

The joke is not limited to Vacek lying. Subordinates, superiors, and witnesses help construct the legend. A delay becomes patience. Fear becomes caution. Being outside the building becomes securing the perimeter. The player defeats the criminals while the official narrative increasingly treats that action as Vacek's plan.

Vacek starts to fear his own promotions because they place him closer to operations he does not know how to handle.

### 15.2 Working escalation ladder

These are story titles, not a researched real-world police rank system.

| Stage | What actually happens | What the public believes | Career change |
|---|---|---|---|
| Slateford | The player defeats the grunt; Vacek arrives after the danger. | He broke a mineral-smuggling operation. | Ordinary officer becomes detective. |
| Early regional incidents | He is late, lost, or waiting somewhere safe. | He avoided alerting the criminals and anticipated their escape. | Growing investigative authority. |
| Midgame facility | He is outside while the player disables the threat. | He coordinated a sophisticated operation. | Task-force leadership. |
| Storage crisis | Specialists and the player restore systems while he speaks publicly. | His recovery plan succeeds exactly on cue. | Regional command or commissioner role. |
| Rocket finale | He arrives after Giovanni leaves with the surviving organization. | He has eliminated Team Rocket permanently. | Prestigious anti-Rocket appointment. |

The exact number of promotions should suit pacing. Repeating the same cutscene after every hurdle would weaken the joke.

### 15.3 Visible escalation

His uniform can acquire more decorations, his introductions become longer, posters and news reports appear, and subordinates become more deferential. Over time the player sees the region responding to a version of events they know is wrong.

His Pokémon can be genuinely capable. A loyal Growlithe that later becomes Arcanine was proposed; it quietly notices danger and rescues its trainer while the audience credits Vacek. The Pokémon's development should be earned and visually tracked rather than used as another arbitrary costume change.

### 15.4 One sincere contribution

Vacek should have at least one moment when his rank becomes genuinely useful. During a crisis he authorizes access, takes responsibility for a risky approval, or signs an order that Běla and Javorová need.

He may be frightened that it will end his career if it fails. The act gives him a small human dimension without retroactively proving he was a genius all along.

Naturally, the success can still earn him a wildly disproportionate promotion.

### 15.5 Illustrative dialogue

> **BĚLA:** „Pane detektive, vždyť jsme právě přijeli.“  
> **VACEK:** „A načasování je při vyšetřování zásadní.“

> **MLADÝ POLICISTA:** „Takže jste celou dobu hlídal východ?“  
> **VACEK:** „Přesně.“  
> **MLADÝ POLICISTA:** „Ale budova má čtyři východy.“  
> **VACEK:** „A právě proto potřebujeme víc lidí.“

These are example beats. Actual script pages should be shorter, localized consistently, and spaced for the GBA text window.

### 15.6 Ending possibilities

An international appointment, a public statue, or a ceremony at which he unknowingly meets Giovanni can be strong payoffs. None is yet the single approved ending.

The essential ending contrast is fixed in spirit: Vacek is praised for destroying an organization that Giovanni has quietly reclaimed.

---

## 16. The regional starters

### 16.1 Shared design philosophy

The starters are regional versions of familiar Pokémon that were **not** originally starter Pokémon. Each familiar two-stage family gains a new third stage in Virelia.

All three have their secondary typing from their earliest regional form, except that Poochyena deliberately changes its secondary type on evolution. Their silhouettes, battle styles, ecological stories, and awakening quests should be distinct.

The current trio is:

| Choice | Regional evolutionary line | Types | Core identity |
|---|---|---|---|
| Grass | Poochyena → Mightyena → Wraithyena | Grass/Dark → Grass/Ghost → Grass/Ghost | A jungle predator becoming a forest guardian. |
| Water | Natu → Xatu → Nimbatu | Water/Psychic throughout | A mystical bird that senses, calls, and eventually commands storms. |
| Fire | Ekans → Arbok → Infernabra | Fire/Poison throughout | A heat-and-venom serpent associated with geothermal or volcanic transformation. |

**Naming note:** Wraithyena is the narrative spelling used in the design discussions. Historical code and assets also use `Wraithena` / `SPECIES_WRAITHENA`. Reconcile the naming deliberately; do not create a second species or silently change IDs.

Nimbatu and Infernabra are working final-stage names, developed enough to use consistently in planning but still subject to an intentional naming decision.

### 16.2 Evolution schedule

The original design staggered the first evolutions at levels 14, 16, and 18. The historical implementation retained that schedule after replacing the Water spider with Natu:

| Line | Recorded first evolution | Recorded final evolution |
|---|---:|---:|
| Natu | 14 | 34 |
| Ekans | 16 | 36 |
| Poochyena | 18 | 38 |

These are recorded prototype tuning values, not biological laws of the world. Earlier Natu brainstorming mentioned different levels. A future balance change should be explicit rather than accidentally mixing those drafts.

A quest can begin before the final evolution, but the final-form Hidden Ability reward should not force a player to grind prematurely to keep the main story moving.

### 16.3 Virelian Poochyena

The Grass starter keeps a recognizable connection to Poochyena but moves toward a leaner jungle predator: dark body, mossy green or olive accents, restrained leaflike forms, and less shaggy bulk than a simple recolor of the original.

The first stage is **Grass/Dark**. This deliberately avoids giving a very early starter the Ghost-type defensive identity before the rest of the opening game is designed around it. It also preserves something of the original Poochyena's nature.

Its development is spiritual as well as physical. Mightyena and Wraithyena become closer to the forest's unseen guardians and acquire Grass/Ghost typing.

### 16.4 Virelian Mightyena and Wraithyena

The stages must differ in silhouette and character, not merely grow longer legs and more fur. Wraithyena has leaflike, wispy, partly spectral features—especially its tail and outline—while remaining a creature the player can recognize as the culmination of the line.

The established sprite direction uses green/olive tones for the normal palette and a blue-toned shiny treatment. The title-screen identity emphasizes glowing eyes and an outline that almost disappears into dark jungle foliage, not a clearly lit ordinary wolf standing in front of trees.

Its battle identity is a fast physical hunter with an unusual relationship to concealment and the boundary between living forest and spirits.

### 16.5 Forest Ambush / Lesní léčka

**Established concept:** A unique Grass- or Ghost-themed two-turn attack. Wraithyena's line disappears into jungle cover or a spectral state on the first turn and strikes on the second.

**Developed working specification:** A physical Grass move, 85 power, 100 accuracy, 10 PP, with recovery equal to 50% of damage dealt. This specification was documented as planned, not demonstrated as a final implemented move.

The mechanic needs its own identity rather than merely renaming an existing two-turn Ghost attack. Exact interactions with protection, weather, substitutes, multi-target battles, and turn interruption require a battle-design pass.

The important established feedback is that its healing already occupies the recovery part of Wraithyena's identity. Its Hidden Ability should add something different.

### 16.6 Wraithyena's Hidden Ability: unresolved

Infiltrator was the historical placeholder: the concept of passing through defensive barriers fits the ghostly theme, but it was considered too situational or insufficiently distinctive as the reward for a personal storyline.

Spirit Harvest, healing after a knockout, was then proposed and rejected as redundant with Forest Ambush.

The following were brainstormed alternatives, **not approved final mechanics**:

| Working ability | Proposed effect | Intended identity |
|---|---|---|
| Phantom Veil | Halve the first damaging super-effective hit against the bearer, once per battle. | Brief protection from the spirit world. |
| Dread Omen | On its first entry, lower the opponent's stronger offensive stat by one stage. | Supernatural intimidation; targeting and ties still need rules. |
| Spectral Hunt | The first successful Ghost-type attack is a guaranteed critical hit. | Seeing a spiritual weak point. |
| Spirit Cycle | Alternating damaging Grass and Ghost attacks gives the next qualifying attack a 30% boost. | Moving between life and death; two-turn timing remains to be defined. |
| Veilwalker | Avoid entry hazards and opposing trapping effects. | Spirit-world mobility; must offer value beyond any already-applicable immunity. |
| Vengeful Spirit | After first taking opposing damage, mark the attacker; the next Ghost attack against that foe gains priority. | A guardian's retaliation; escape, switching, and mark consumption need rules. |

The ability should remain useful outside a contrived boss battle, avoid duplicating Forest Ambush's recovery, and suit both the story campaign and friendly PvP. No option above should be inserted as canon solely because it appears in this file.

### 16.7 Virelian Natu and Xatu

Natu replaces the Water/Bug spider concept because that concept no longer felt distinctive enough alongside an existing water-spider Pokémon.

The new Water line remains mystical and visibly birdlike. Natu is a compact rain-sensing bird. Xatu retains an upright, observant, oracle-like identity. Water and Psychic traits develop through weather perception, ritual movement, and an increasing ability to affect atmospheric conditions.

Teal, blue, indigo, pale rain-like markings, and restrained gold details form the concept palette. Geometric patterning can connect the line to Xatu without turning it into a generic realistic heron.

The design should express a deliberate **Rain Dance** strategy rather than rely entirely on automatic rain upon entry. Earlier code snapshots lacked Rain Dance; the later v0.4 design added it to the intended early learnset. This document records the design identity, not the current compiled move table.

### 16.8 Nimbatu

Nimbatu is an imposing but still elegant bird. Its storm elements appear through feather structures, flowing tail plumage, cloudlike layering, and lightning-like markings—not by replacing its body with a formless storm cloud.

The silhouette must work at GBA sprite scale. A clear head, one anatomically coherent beak, readable wings, and a distinct tail matter more than many tiny decorative details. Earlier concept images contained doubled beaks; those are generation errors, not design features.

The line's intended battle identity is a special attacker and weather controller. Exact base stats, speed tier, and movepool remain balance work; historical prototype stats are not a requirement to preserve the earlier spider's battle role.

### 16.9 Storm Omen

**Established ability concept:** The final Water form's Hidden Ability causes paralysis when it uses Rain Dance.

**Working balancing proposal:** When Nimbatu successfully summons rain through its own Rain Dance, Storm Omen can paralyze one eligible opposing Pokémon. The suggested version is guaranteed against a valid target but limited to once per bearer per battle.

The earlier proposal chose one eligible opponent in a Double Battle rather than both. Random selection was suggested, but exact targeting is not final. Normal immunity/protection handling must be specified. Another Pokémon setting rain, a failed Rain Dance, or simply entering existing rain should not silently count as the same trigger without an intentional rule.

The ability is an alternative to the regular Ability, not a second simultaneously active power. Hydration was the preferred thematic regular-Ability proposal; historical prototypes used Torrent and Water Absorb. These differences require a deliberate ability pass before the awakening quest is implemented.

Storm Omen is a late personal reward. It does not need to appear on an early rival's Natu or Xatu.

### 16.10 Virelian Ekans, Arbok, and Infernabra

The Fire starter is a Fire/Poison serpent. Its visual language combines a familiar coiled snake/cobra silhouette with heat, embers, mineral coloration, fumes, and increasingly powerful venom.

Infernabra's final design should differ meaningfully from a larger Arbok. Its hood, patterning, posture, and heat-related elements can suggest a creature adapted to dangerous underground or volcanic conditions.

The line was developed as a special attacker with status pressure. Corrosion is the historical/proposed Hidden Ability direction, allowing otherwise poison-resistant categories of Pokémon to be poisoned under its intended rules. It is not, by itself, a proposal to remove all type-based damage rules or create simultaneous major status conditions.

The earlier **Burning Venom** idea explored a burn plus an additional poison-like effect. It remains unresolved. Do not describe both ordinary burn and ordinary poison being active together as an already-existing rule of Virelia.

### 16.11 Phanpy: retained idea, not a starter

A special-attacking Water-oriented regional Phanpy line was strongly liked: an elephant firing pressurized Hydro Pump through its trunk, developing into a bulky water-artillery creature.

The working decision was to prioritize Xatu as the starter while reserving Phanpy as a possible regional Pokémon encountered later. Its inclusion, typing, third-stage name, location, and stats are not finalized. It should not silently become a fourth starter.

### 16.12 Shared mechanical cautions

One individual Pokémon has one active Ability at a time in the intended design. A Hidden Ability is an alternative slot unlocked through the personal storyline, not an extra passive stacked on the normal Ability.

Starter balance must be tested across actual type combinations, moves, weather, evolution timing, and PvP. Do not rely on the earlier shorthand claim that the final trio forms a perfectly symmetrical double-STAB triangle.

The Grass and Fire line's ability decisions do not need to mirror Storm Omen's exact once-per-battle trigger. They should offer comparably meaningful but different choices.

---

## 17. Starter-specific awakening storylines

### 17.1 The shared structure

**Established:** The original starter choice determines which personal quest unlocks. The story begins at Professor Javorová's laboratory, creating a reason to return to the opening town. Completing the chosen quest awakens that partner's Hidden Ability.

The working structure is:

1. The player first encounters a local mystery during normal exploration.
2. Around the Gym-4 return, Javorová recognizes unusual behavior in the starter.
3. The laboratory begins the selected personal storyline.
4. The player revisits earlier locations and reaches one or more newly accessible spaces.
5. The quest culminates when the appropriate final form and story conditions are present.
6. Javorová acknowledges the discovery afterward.

The laboratory visit can be part of the main story. The long personal quest and its reward remain optional. A player who wants to continue collecting badges should not be blocked by a starter-specific side dungeon.

### 17.2 Grass: Whispers of the Forgotten Forest

**Partner:** Poochyena → Mightyena → Wraithyena. **Climax:** Forgotten Sanctuary, Verdant Hollow.

The first clues are in Whisperwood: vanishing pawprints, an ancient tree, and carvings that do not yet explain themselves. In Verdant Hollow, the player sees related evidence on a larger scale.

At the lab, Javorová notices that the partner's shadow or response to the forest does not behave as expected. Matěj's observations help identify a route or tradition that modern research has not fully explained.

Returning to Verdant Hollow opens the personal sanctuary path. The proposed trials require protecting life, restoring part of the environment, and learning to navigate the forest's spirit-related behavior. The partner is recognized as a guardian rather than simply declared the strongest predator.

The reward is Wraithyena's chosen Hidden Ability. Its final mechanics remain open. The awakening should feel like new perception, authority, protection, or movement between worlds, not merely more of Forest Ambush's healing.

### 17.3 Water: The Eye of the Storm

**Partner:** Natu → Xatu → Nimbatu. **Working geography:** Rainfall Pass, weather observations around Seabreak, and later Skywatch or a mountain sanctuary.

The line senses a weather pattern before local instruments can explain it. The proposed story contrasts drought in one area with violent storms elsewhere, or otherwise establishes an imbalance that is more specific than “it rains in Rainfall.” Whether that imbalance is unnatural or simply poorly understood is open.

Javorová's laboratory begins the investigation. Old weather monuments, a research record, and new observations connect earlier viewpoints to a later mountain location. Restoring or understanding three weather sites was suggested as the core sequence; the number and exact puzzle design are provisional.

The climax demonstrates Nimbatu moving beyond prediction to controlled storm-calling. Storm Omen is earned through that relationship, not from installing an upgrade into a bird.

The story must avoid implying that every ordinary storm is a malfunction, that all shrines are machines, or that Rain Dance is required as an unreplaceable HM-like lock on the main campaign.

### 17.4 Fire: Heart of the Volcano

**Partner:** Ekans → Arbok → Infernabra. **Working geography:** Deep geothermal passages associated with the mining network, with a possible later volcanic site.

The first clues are unusual heat, irritating fumes, reactive minerals, or a corrosive effect that surprises the laboratory. Javorová and Mara can connect the partner's biology to a deeper environment.

The proposed investigation follows abandoned workings into natural geothermal chambers. The personal challenge is adaptation to heat and toxic conditions, not proving willingness to harm another Pokémon.

Corrosion is the current reward direction. The exact final ability choice and whether a custom signature effect supplements the ordinary move design remain open.

**Geographic reservation:** The full landmass plan has not yet placed a dedicated active volcano. Do not suddenly turn Slateford's entire limestone quarry into a lava pit. A geothermal branch beneath Stormcrest is a working option; a separate volcanic area would need its own approved position and connections. “Heart of the Volcano” is therefore a working quest title, not proof that a volcano is already mapped.

### 17.5 Scope and exclusivity

The personal storyline follows the player's **original starter choice**, not whichever species happens to lead the party at the moment. Trading or catching another starter should not automatically switch the chosen narrative.

The original partner is the preferred recipient because the story is about a relationship begun at Javorová's lab. Depositing, evolving, renaming, or temporarily trading that Pokémon must not cause a permanent quest softlock. Identity tracking and recovery rules are engineering decisions still to be made.

Other players can visit the region's important locations and appreciate their history. The unique awakening is exclusive; the entire rainforest, weather region, or late-game temple is not.

A general Ability Patch should not casually bypass the emotional reward before the quest. Postgame convenience and repeat-Pokémon access can be decided later.

---

## 18. Optional exploration and short side quests

### 18.1 What “optional” means

The region needs many short routes and stories that do not advance the main plot or provide a hidden prerequisite for a badge. They should enrich a place, offer a Pokémon encounter or modest reward, reveal a local relationship, or open a useful small loop.

Optional content is not padding between mandatory dialogue scenes. A player can ignore it and still complete the campaign. A completionist should find more than a Potion at the end of every branch.

A useful target for many small quests is a few minutes, with some longer local mysteries. Rewards should vary among held items, TMs, access, unusual encounters, lore, and changed NPC behavior. The precise reward must suit the point in the level curve.

### 18.2 Developed opening and coastal quests

| Quest | Place | Story / activity | Reward and status |
|---|---|---|---|
| Hana's wandering Zigzagoon | Maplebrook / Whisperwood | Find why the Pokémon has stayed beside a Wurmple nest. | Potion and a lead toward the sanctuary; developed opening quest. |
| Whisperwood's ancient tree | Whisperwood Sanctuary | Interpret seed/rain/root stones and inspect disappearing tracks. | Soothe Bell, folklore, persistent shortcut; developed optional area. |
| The Missing Tools | Slateford Quarry Gallery | Follow tools and clues to Sableye instead of another Rocket thief. | Normal encounter, recovered tools, Hard Stone; developed quest. |
| The Message in the Bottle | Seabreak | Follow coastal clues and an old keeper's record. | Historical revelation; later prototype uses Mystic Water. |

The bottle quest's developed form involves a fisherman, an old anchor, a storm/tide marker, and a journal. Its truth belongs to Seabreak's past, not another secret Rocket plot.

### 18.3 Optional-content reserves by area

These are proposals to choose from, not a promise to implement every entry.

| Area | Optional places and short-story ideas |
|---|---|
| Maplebrook | A nursery problem, a small orchard story, a returning visitor, a hidden grove item, updated family dialogue. |
| Whisperwood | Overlooked nest paths, a small lost-object investigation with Pokémon behavior, deeper guardian clues on return. |
| Slateford Quarry | Rare crystal seam, abandoned lift room, an old worker's account, an optional fossil branch beyond the later breakthrough. |
| Rainfall Upper | Ranger lookout, weather readings, a cliffside rescue, a secluded upland pool. |
| Rainfall Lower | Waterfall grotto, a cave behind spray, wetland observation, a bridge repair that opens only a local loop. |
| Seabreak | Bottle mystery, tidepool path, shipyard help, keeper memories, coastal cave, modest harbor errands with local consequences. |
| Reedmere | Drowned bell tower, lost Wooper, an old canal lock, optional marsh boardwalk, a small boat route. |
| Mosswatch | Ranger requests, medicinal-plant research, visitor mistakes, practical conservation work. |
| Verdant Hollow | Ancient-tree survey, overgrown seed-vault or abandoned greenhouse near the accessible edge, ruins, research-camp development, deeper temple exploration. |
| Ironwake | Ship graveyard, repairer's missing parts, abandoned rail spur, worker-and-Pokémon dispute, optional salvage. |
| Skywatch | Storm-meter readings, upland lake, cloud-cliff rescue, instruments that record something unexpected. |
| Frostgate | Glacial cave, hidden hot spring, fossil chamber, maintenance shelter, stranded traveler. |
| Crownspire | Archives, rooftop traversal, a misleading Vacek monument or ceremony, a civilian story not tied to the finale. |
| Offshore islands | Shipwrecks, rare habitats, local legends, self-contained exploration. |

Seasonal blooms, low tides, and nighttime scenery were discussed as possibilities. They do not automatically commit the ROM to an elaborate real-time calendar, tide simulation, or progression that requires playing at a particular real-world hour.

### 18.4 Make a short quest tell a story

A successful small quest should establish a person or Pokémon with a recognizable need, offer a small discovery or choice, and leave a visible or conversational change afterward.

The Sableye mystery is a good model: the setup resembles Rocket theft, the clues support a different explanation, and the result belongs to the quarry's own wildlife. It expands the setting instead of merely extending the main villain's reach.

Failure, leaving, a full Bag, or postponing an encounter should not make a reasonable optional quest permanently disappear unless that is an explicitly designed choice.

---

## 19. Duo Adventure and the multiplayer rival

### 19.1 The shared-play premise

The project is being made with two friends playing in parallel. Their existing habit is to compare progress and duel at every ten-level milestone. The long-term game design should support that rivalry rather than treat multiplayer as a detached menu feature.

The intended **Duo Adventure** mode contains story-linked cooperative missions through emulated Link Cable functionality. Both players can prepare independently, then meet for a joint challenge.

The long-term ambition is for selected shared missions to be required for progression within that mode. This is not the same as making untested link functionality block the ordinary development build today.

### 19.2 Cooperative missions

The desired mission format includes both human trainers acting together in a 2v2-style battle against enemy trainers, with story context and persistent completion in both saves.

Possible story uses include a Rocket facility or operation requiring both trainers, but individual mission locations and exact scripts have not been approved across the full region.

A shared mission should make both players relevant and acknowledge both on completion. Connection loss, differing progression, repeated rewards, and one player completing the content earlier are design problems to solve before mandatory gates are enabled.

### 19.3 Friendly versus battles

The preferred rhythm is a cooperative mission followed by a player-versus-player match before the friends part ways. Winning can grant a modest rare reward such as a Rare Candy or appropriate evolution item.

The duel should not make the loser unable to progress. Repeated reward farming, rematches, draws, disconnects, and the difference between a competitive reward and a story reward need explicit rules.

The NPC rival still has their own progression and dialogue. They do not absorb the multiplayer rival's role.

### 19.4 Technical status boundary

My Boy! compatibility matters because of the intended Wi-Fi Link Cable workflow. A story-mode freeze was reported fixed during development, but that does not prove cooperative Link Multi behavior, cross-emulator interoperability, or safe synchronization of custom story flags.

The creator previously instructed that Link Multi development remain paused while unverified. Therefore:

- Preserve the existing link proof of concept and test facilities.
- Do not promise that required Duo missions already work.
- Do not gate new ordinary story chapters on an untested connection.
- Treat the eventual mandatory Duo mode as a separate verified milestone.

The world can reserve shared mission locations now without pretending the networking contract is complete.

---

## 20. The Hoenn postgame

### 20.1 Hoenn remains part of the game

The original Hoenn region is retained as the foundation for a substantial postgame, reached after the Virelia League. It should not be deleted to make room for Virelia or treated as a tiny bonus arena.

The broad inspiration is revisiting a familiar region after a new main adventure, with changed roles and a new competitive goal. This is a Virelia design direction, not a requirement to copy another game's exact progression.

### 20.2 Departure from Seabreak

The international terminal belongs to Seabreak from its first visit, even if long-distance passage is unavailable. Storm conditions, security concerns, schedules, or access requirements can explain the restriction without claiming no boat in Virelia can move before the League.

After the Virelia championship and resolution of the main regional crisis, the voyage becomes available. The player leaves from a familiar harbor whose meaning has changed across the campaign.

Slateport is the current candidate arrival point in Hoenn because it serves the planned maritime entry. The exact arrival sequence and downstream Gym order remain open.

### 20.3 A changed League hierarchy

The desired second Elite Four is composed of former Emerald Gym Leaders who have advanced in their careers. The original elite generation may have retired or moved on.

Winona with **Mega Altaria** is an explicit requested feature. Giving each member one Mega was considered positively, but the full membership, Champion, replacement Gym Leaders, teams, and battle order have not been finalized.

Do not silently nominate four leaders and present that roster as approved. The second League is a major part of the postgame and deserves its own design pass.

### 20.4 Mega Evolution

**Established:** Mega Evolution unlocks after completing the Virelia League. It is not an early-game reward and must not creep into Virelia's opening Gyms through imported teams or items.

The acquisition scene, access to stones, and how Hoenn's challenges teach the system remain open. Mega Evolution gives the familiar region a new battle dimension, especially when facing former Gym Leaders in their promoted roles.

### 20.5 Postgame story

The Hoenn chapter's detailed story has not been settled. The working themes are a changing generation, familiar places seen later, stronger trainers, Mega Evolution, and renewed exploration.

Giovanni can remain an unresolved presence or support a later optional rematch, but Hoenn should not automatically repeat Virelia's storage crisis or turn every town into the same Rocket storyline.

Independent Virelia and Hoenn progression, badges, navigation, and travel must be designed together. Retaining original Hoenn map data does not by itself create a coherent playable postgame.

---

## 21. Visual identity, map composition, and localization

### 21.1 The visual target

The target remains **classic GBA Pokémon presentation with some original graphics**. Clear silhouettes, readable paths, compact sprite language, coherent materials, and limited visual noise matter more than the amount of detail in a high-resolution concept image.

Concept paintings are art direction, not ready-to-import tilesets. A dramatic panorama may need a different composition at 240×160. A detailed landmark must be redrawn to work with the actual game camera and tile scale.

The title screen's established direction is a dense, dark jungle with a cold blue atmosphere, glowing Wraithyena eyes near the visual center, and a body outline that melts into foliage. Leafy or wispy elements should distinguish the creature from an ordinary wolf. Virelia replaces Emerald as the game's identity.

### 21.2 Visual identities by region

| Area | Materials, atmosphere, and landmark language |
|---|---|
| Maplebrook | Warm homes, gardens, greenery, research space, gentle water. |
| Whisperwood | Readable forest paths, roots, dappled light, modest early ruins. |
| Slateford | Stone, timber supports, worked minerals, practical civic buildings. |
| Quarry | Excavation terraces, rails, machinery, distinct mineral seams, transitions into natural cave. |
| Rainfall Upper | High rock, mossy stairs, waterfalls, bridges, mist, exposed vistas. |
| Rainfall Lower | Wet forest, pools, reeds, boardwalks, streams, sheltered paths. |
| Seabreak | Cool wet stone, blue/slate roofs, warm lanterns, three terraces, working harbor, strong lighthouse silhouette. |
| Reedmere | Raised timber buildings, reeds, channels, floodgates, small boats. |
| Mosswatch | Small forest-edge homes, ranger structures, practical research and cultivation. |
| Verdant Hollow | Massive old trees, layered canopy, roots, partial ruins, water, quiet mystery. |
| Forgotten Sanctuary | Concentrated guardian motifs, ancient tree, monoliths, restrained spectral light. |
| Sunken Temple | Monumental ruins integrated with roots and water; vertical and submerged spaces. |
| Ironwake | Shipyards, ironwork, freight, dense working districts, western sea. |
| Skywatch | Research domes, instruments, clouds, cliff walkways, service lifts. |
| Frostgate | Snow, protected paths, warm interiors, ice, alpine stone. |
| Crownspire | Civic architecture, public spaces, archives, plateau streets, modern institutions. |
| League | A clear northern destination and a memorable final approach. |

### 21.3 Map composition principles

Begin with landforms and human purpose. Decide where the ridge, river, bay, road, buildings, and landmark belong before filling remaining cells with trees.

A script that stamps rectangles or a regular tree grid can be a useful authoring tool. It is not a substitute for an intentional composition. Conversely, reusing complete stock art is not automatically wrong; its placement, palette relationship, and role in the scene matter.

Every important map should have an identifiable main path, optional areas that are worth entering, a visual landmark, and sensible entrances. Terrain changes should explain the direction and height of travel. Routes can be asymmetric and irregular while retaining readability.

A town should not be enlarged simply to hold more repeated buildings. Its streets, services, work areas, and public spaces should explain its population and geography.

### 21.4 Important custom assets

The priority assets discussed include:

- Seabreak's lighthouse exterior, lantern, foundation, and repaired/damaged equipment.
- Coastal retaining walls, stairways, docks, harbor props, and recognizable clue objects.
- Rainfall's cliff assemblies, bridge variants, waterfall transitions, wet vegetation, and natural paths.
- An outdoor quarry with readable machinery and excavation layers.
- Distinct recurring-character sprites, especially Vacek and Běla, as they become established.
- Starter sprites whose evolutionary stages differ clearly and whose back views use appropriate battle framing rather than showing the full body unnecessarily.

Producing a reference board does not establish that the corresponding original pixel assets exist in the ROM.

### 21.5 Layout concept corrections

Later map-layout pictures contain inconsistencies in compass labels and added content. For example, an image may place a “from Slateford” arrow on an edge that contradicts the regional plan, or invent a Maplebrook shortcut from a town that did not have one in the route graph.

When this happens, **the geography register and accepted prose win**. Correct the artwork or disregard the accidental label. Do not adjust the entire world to accommodate an image-generation error.

Similarly, no generated royal banners, throne rooms, unsolicited Czech translations of town names, or unapproved shrines become canon automatically. “Pobřežní Lhota,” which appeared in one concept, is not Seabreak's adopted localized name.

### 21.6 Czech localization conventions

All ordinary player-facing dialogue, choices, story notices, instructions, and relevant interface text are Czech. Species names, established place names, and the project's chosen English move/item naming convention remain consistent.

Use proper Czech diacritics. Dialogue should sound written in Czech, not translated word for word from English. Account for the player's gender or use natural neutral wording where possible. Rival text must match Viktor/Viktorie correctly.

The small screen demands short pages and deliberate line breaks, not broken grammar or missing accents. A polished exchange can be two or three short text boxes rather than one paragraph squeezed into the window.

Recurring jokes should escalate, vary, and reveal character. Vacek's first credit theft should not be repeated verbatim six times.

---

## 22. Implementation boundaries and development continuity

### 22.1 Project foundation

The project repository is `hkcmoris/romhack`. The development foundation is `pokeemerald-expansion`, with Porymap for maps and Aseprite for pixel art. GitHub Actions supplies the established ROM build and artifact workflow.

This bible does not require installing a new local toolchain. It does not authorize repository edits, branch merges, save migrations, or automatic implementation of the entire region.

### 22.2 Historical build sequence

The development discussions progressed through a Link proof of concept, starter prototypes, a Virelia map foundation, a playable prologue, the first Gym, additional opening side quests, My Boy! compatibility work, and the Rainfall/Seabreak chapter.

Recorded branches include `codex/duo-link-poc`, `codex/virelia-prologue`, `codex/virelia-first-gym`, `codex/starter-prologue-merge`, and `codex/virelia-rainfall-seabreak`. They are historical context, not an instruction to assume which branch is currently authoritative.

An earlier downloaded prologue release was version 0.3.115. That identifies a historical artifact only; this document does not claim it is still the latest ROM.

### 22.3 The current design is newer than some map layouts

The first v0.4 implementation merged both Rainfall paths into a shared lowlands exit. The later world design explicitly changes that: Upper Pass reaches Seabreak's upper terrace; Lower Pass reaches its harbor.

The original city-to-cave entrance also lacks the accepted separate outdoor quarry. Verdant Hollow's old Gym-city marker is superseded by Mosswatch plus a distinct wilderness.

These are intended refactors, not mistakes to preserve in the name of compatibility.

### 22.4 Existing IDs and saves

The creator has stated that the project is early enough for major changes. Preserve IDs and save layout when useful, but do not let an accidental prototype numbering scheme determine world geography forever.

When a change affects stored map locations, species identity, quest state, or layout assumptions, document whether it requires a new game or a migration. Never silently promise arbitrary cross-build save compatibility.

An original starter's identity, awakened Ability, quest progress, ordinary item rewards, and transport unlocks must persist correctly within a supported build.

### 22.5 Region systems

Earlier Virelia maps were registered using Hoenn-compatible engine conventions rather than a complete independent regional map system. A true Virelia Town Map, Fly network, regional display, and two-region progression require their own architecture.

The gameplay design calls for independent Virelia and Hoenn achievements. Do not overwrite Hoenn's eight badges to represent Virelia's badges or assume one unchanged eight-slot display represents both campaigns.

### 22.6 Editing and generation tools

Some prototype map builders and polish scripts regenerate geometry. Running them after hand-editing in Porymap can erase authored changes.

Use a clear source-of-truth workflow: preserve hand-authored layouts, make generators consume deliberate data where appropriate, and separate validation from destructive regeneration. A successful reachability audit does not certify that a map looks good.

### 22.7 Testing boundaries

A compiled ROM, a green automated test suite, or a successful mGBA fixture is not proof of a complete human playthrough, measured pacing, Android acceptance, or working Link Multi.

Test geography and story together: reciprocal warps, both route options, town stairs, NPC access, battle loss and retry, full-Bag rewards, saved shortcuts, weather transitions, and backtracking to the lab. Test visual composition in the actual viewport as well as in full-map renders.

Protect the verified My Boy! compatibility changes when refactoring. Its historical problem does not need to be re-investigated merely because this world bible records the incident.

### 22.8 Development priority implied by this bible

The immediate world-building task is to reconcile the region plan with the early maps: separate the quarry surface, establish a coherent mountain crossing, give Seabreak its two arrivals and three terraces, and reserve the future cave, rail, coastal, and northern links.

Future towns should then be built in playable chapters. Do not implement eight empty town maps simply because their names and positions now exist.

---

## 23. Open decisions and continuity checklist

### 23.1 Decisions still needed

| Area | Unresolved decision |
|---|---|
| Regional cartography | Exact coastline, world-map scale, river courses, island placement, and detailed route footprints. |
| Mosswatch / Verdant Hollow | Final separate coordinates and borders after the accepted town-versus-wilderness revision. |
| Gyms 2–8 | Leaders, types, badge names, puzzles, teams, and level curve. |
| League | Virelia Elite Four, Champion, roster, and exact physical site. |
| Main antagonist | Final name, appearance, and whether leader and scientist are one character or two. |
| Giovanni | Disguise/alias, first appearance, exact reveal scene, immediate confrontation or escape, postgame role. |
| Rocket geography | Final locations of laboratories, major missions, storage-control infrastructure, and headquarters. |
| Storage crisis | Timing, scope, lab fallback logistics, full-party capture handling, and safe restoration. |
| Wraithyena | Final Hidden Ability and consistent display spelling. |
| Nimbatu | Regular Ability, precise Storm Omen timing/targeting, immunity handling, and final balance. |
| Infernabra | Final Hidden Ability direction, Burning Venom rules, and location of its geothermal/volcanic climax. |
| Ancient history | Civilization name, chronology, temple purpose, deeper guardian encounter, and literal versus symbolic forest-root connection. |
| Cast | Which proposed supporting characters recur, signature teams, reporter Ema continuity, and Matěj's residence. |
| Traversal | Field abilities, Fly timing, transport operation, and optional real-time systems. |
| Multiplayer | Verified battle format, synchronization, required missions, disconnect recovery, and PvP reward safeguards. |
| Hoenn | Arrival sequence, replacement Gym Leaders, promoted Elite Four roster, Champion, and central postgame plot. |

### 23.2 Geography and world checks

Before approving a new map or story chapter, check the following:

- Is its position consistent with the region's east/north planning grid?
- Do ordinary exits and reciprocal entries correspond to the journey's actual direction?
- Is height being confused with north/south placement?
- Does the terrain explain the river, settlement, industry, or route?
- Do Rainfall's two options still arrive in different Seabreak districts?
- Is the quarry a separate outdoor place with preserved underground purpose?
- Is Mosswatch the Gym settlement, with Verdant Hollow remaining wilderness?
- Does a newly opened shortcut genuinely reduce travel and support a new story on return?
- Is there useful optional exploration that does not secretly gate the main campaign?
- Does the player's route back to Javorová remain practical during the storage crisis?

### 23.3 Story checks

- The protagonist is still a child in a sincere Pokémon world.
- There is one major evil organization, not an expanding collection of unrelated teams.
- AI remains secondary flavor, not the central subject of the Porygon plot.
- Rescued Pokémon and restored services remain rescued and restored after Giovanni's twist.
- Vacek's reputation rises through misinterpretation and opportunism; he does not become secretly competent at everything.
- Competent allies such as Javorová, Mara, Klára, and Běla still contribute meaningfully.
- The NPC rival and the real multiplayer rival remain distinct.
- A starter-specific quest does not lock other starters out of a major region or temple.
- The Forgotten Sanctuary and Sunken Temple are separate discoveries.
- Mega Evolution stays after the Virelia League.
- Seabreak remains the intended post-League gateway to Hoenn.

### 23.4 Art and implementation checks

- A concept image is not a technical tileset or a canonical route diagram.
- Medieval-looking artwork does not establish a medieval society.
- New assets remain legible at GBA scale.
- Custom buildings and terrain use a coherent local material language.
- A renderer or generator does not silently overwrite newer Porymap work.
- Localized text is tested in real windows, not only validated as source strings.
- Save behavior, gameplay verification, and emulator acceptance are stated separately.
- Proposed content is never described as implemented solely because it appears in a planning file.

---

## 24. Source and revision notes

This document consolidates the Virelia design conversation and the supplied planning artifacts `virelia_geography_blueprint.md` and `virelia_region_routes.json`. Their geographic anchors and shortcut proposals are reproduced here so this file can stand alone without those companion files.

Those earlier artifacts predate the Mosswatch revision. Their `Verdant Hollow = Gym 4 city` entry is superseded. This document reassigns that approximate Gym-4 position to Mosswatch as a working map revision and reserves a distinct rainforest footprint; it does not claim that revised exact coordinates were already approved.

The old graph's city-to-quarry unlock label is also corrected: the first quarry investigation happens before the first Gym badge. Later underground connections remain midgame unlocks.

Historical code observations and prototype quests are included only where they clarify continuity. No live repository inspection was performed for this export, and no repository files were modified.

Generated concept art supplied atmosphere, architecture, and map-composition ideas. Its accidental arrows, labels, duplicated anatomy, extra towns, unapproved titles, and fantasy-castle imagery are not treated as binding decisions.

No outside Pokémon lore research has been added to this consolidation. Strict canon chronology, engine-specific mechanics, and current repository status should be verified when they become implementation requirements.

---

## The world in one paragraph

Virelia is a modern Pokémon region whose mountains, rivers, forests, and working communities shape the adventure. A child leaves Maplebrook, travels north through Whisperwood to Slateford, crosses Stormcrest by one of two Rainfall paths, and discovers Seabreak descending toward a stormy eastern harbor. The journey continues through Reedmere's delta, Mosswatch's forest-edge community, Verdant Hollow's ancient wilderness, Ironwake's western industry, Skywatch's observatories, Frostgate's alpine basin, and Crownspire's civic heights before the League. New trails, freight links, mountain caverns, and service routes repeatedly lead back to earlier places with new stories. Team Rocket's mineral theft grows into the exploitation of Porygons and a storage-system crisis; a mysterious ally turns out to be Giovanni, who uses the player's genuine victories to reclaim the organization. Vacek becomes famous for achievements he mostly arrives too late to witness. The player's chosen regional starter awakens through a personal quest tied to the region's forest, storms, or subterranean heat. After the Virelia League, Seabreak's familiar docks become the departure point for a changed Hoenn and a new era of battles.

**The first visit introduces the place. The second visit changes its story. The third visit reveals a secret.**
