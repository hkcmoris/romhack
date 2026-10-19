# Duo Adventure: Link Multi feasibility test

## Status and scope

**Manual Android/My Boy! verification: NOT TESTED.** This milestone exposes
Emerald Expansion 1.17.1's existing Battle Tower Link Multi challenge. It does
not establish that My Boy!'s Wi-Fi cable emulation can complete that challenge.
A successful GitHub Actions run proves compilation and its automated checks,
not communication, independent controls, or recovery on two phones.

The hypothesis to test is: two human players, each with their own save and
the identical ROM build, can each control a Pokemon on the same side against
two existing NPC trainers. A first battle followed by normal retirement is
enough to test this; completing the seven-battle challenge is unnecessary.

The temporary ORACULUM quick start skips the overworld introduction and offers
two prepared teams. It is a development utility, not the final introduction for
Project ORÁKULUM. There is no Virelia, custom story battle, progression lock,
custom reward, network protocol, save synchronization, save-format change, or
battle-engine rewrite in this test.

## Implementation and access

The existing two travel NPCs still use the original Tower. The quick start adds
an Emerald-only new-game path and optional team setup to their shared script:

| File | Addition |
| --- | --- |
| `data/maps/LittlerootTown/map.json` | Test travel NPC at tile `(17, 12)`, east of Birch's laboratory |
| `data/maps/BattleFrontier_BattleTowerLobby/map.json` | Return NPC at tile `(22, 8)` |
| `data/scripts/duo_multiplayer_test.inc` | Travel/return scripts, first-entry team choice, and optional one-time team setup |
| `data/event_scripts.s` | Includes the shared test script and its state constants |
| `include/config/general.h` | `ORACULUM_DUO_TEST_QUICK_START`, enabled by default for this test branch |
| `src/new_game.c` | Preserve normal save and Trainer ID initialization; change the Emerald starting warp and initialize only skipped introduction states |
| `src/overworld.c` | Use the ordinary field fade instead of the truck sequence and initialize time-based events without the clock tutorial |
| `data/maps/LittlerootTown/scripts.inc` | First-entry team prompt when a new test save is pending |
| `include/constants/vars.h`, `include/constants/oraculum_duo_test.h` | Name an existing unused saved variable slot and its pending/received values |
| `test/oraculum_duo_test.c` | Automated checks for team eligibility, ownership, health, capacity, repeated-grant protection, and native save/load persistence |

Start **NEW GAME**, select gender and name normally, and arrive outdoors at
`(17, 13)` in Littleroot, directly below the scientist at `(17, 12)`. Select a
preset at **DUO TEST: SELECT TEAM**; agree on **TEAM A** for one phone and
**TEAM B** for the other. Each new save receives exactly two level 50 Pokemon.
Talk to the scientist, choose **TRAVEL TO TOWER**, and accept the trip to arrive
at `(18, 6)` in the original Battle Tower lobby, directly below the rightmost
Link Multi attendant at `(18, 5)`. Finishing Hoenn or obtaining the Frontier Pass
is unnecessary. Talk to that original attendant to start the test.

Both travel NPCs refuse travel while `FRONTIER_DATA_CHALLENGE_STATUS` is
nonzero, so the shortcut cannot abandon an active or suspended Frontier
challenge. Finish or retire from an existing challenge through its ordinary
facility flow before using the shortcut. The Littleroot NPC also requires at
least two party members; the original attendant applies the actual eligibility
rules. Team initialization uses `VAR_ORACULUM_DUO_TEST_STATE` in the existing
saved-variable array at previously unused slot `0x404E`: `0` means not received,
`1` means a new game's first-entry prompt is pending, and `2`/`3` record Team
A/B received. This names existing storage; it does not enlarge the save layout.

After the Tower has completed its normal return dialogue and saving, talk to
the return NPC to land at `(17, 13)` in Littleroot. The original Tower scripts,
battle engine, maps, and existing travel exits are unchanged.

## Temporary quick-start configuration

`ORACULUM_DUO_TEST_QUICK_START` defaults to `1` in
`include/config/general.h`. Its behavior is gated by `!IS_FRLG` and is included
in both Emerald development and release artifacts. It does not depend on the
debug menu. Setting it to `0` restores the original Emerald new-game and
truck flow, removes the automatic team prompt and optional NPC team menu, and
leaves the two original POC travel NPCs available. FireRed and LeafGreen do not
use the quick-start behavior. Use numeric `1`/`0` because the option is shared
by C and event-script preprocessing, where the C-only `TRUE` macro is absent.

Normal name/gender selection, `NewGameInitData`, and `InitPlayerTrainerId`
remain responsible for each new player's identity and save initialization. The
test path changes the initial warp, uses a normal overworld fade, sets the
gender-appropriate home respawn, marks the local introduction states complete,
hides their trucks/tutorial objects, and initializes the clock/time-based
systems without requiring clock interaction. This prevents the moving-truck,
Birch rescue, starter, rival introduction, and running-shoes sequences from
starting when visiting the nearby maps. It enables running shoes and the
Pokemon menu after the grant, but grants no badges, Pokedex completion, or
later Hoenn story unlocks. These saves skip selected introduction states for
testing; do not treat them as the final story's intended starting saves.

Only **NEW GAME** marks first entry pending. **CONTINUE** preserves the loaded
story, party, and setup receipt; it does not rerun new-game initialization. An
older save can use the scientist's **TEST TEAM SETUP** action instead. After an
explicit **YES**, it appends two Pokemon only if at least two party slots are
empty. With five or six party members it refuses; store enough Pokemon in a
PC first. It never replaces party members, sends the new pair into storage,
or heals/changes the existing Pokemon during setup. Declining setup leaves
the save eligible to request it later. Once either preset has been received,
the save cannot request another or change presets, even if those Pokemon are
later boxed or released. Save normally after setup to persist this receipt.

Coordinate opposite presets before choosing. There is no new network
negotiation for team selection; the native Tower rejects shared selected
species. If both choose the same preset, use independent fresh test saves to
choose again, keeping the earlier save backups safe.

## Prepared test teams

All four Pokemon are level 50, have 31 in every IV, 508 total EVs, full HP and
PP, and the local player's OT name, gender, and Trainer ID. The grant uses the
ordinary `givemon` command. Every Pokemon has Protect in its fourth move slot.

| Preset | Pokemon | Nature / ability | Held item | EVs | Other moves |
| --- | --- | --- | --- | --- | --- |
| A | Gardevoir | Modest / Telepathy | Sitrus Berry | 4 HP / 252 Sp. Atk / 252 Speed | Psychic, Dazzling Gleam, Helping Hand |
| A | Breloom | Adamant / Technician | Focus Sash | 4 HP / 252 Attack / 252 Speed | Spore, Seed Bomb, Mach Punch |
| B | Swampert | Adamant / Torrent | Leftovers | 252 HP / 252 Attack / 4 Sp. Def | Waterfall, Earthquake, Ice Punch |
| B | Manectric | Timid / Lightning Rod | Magnet | 4 HP / 252 Sp. Atk / 252 Speed | Thunderbolt, Flamethrower, Snarl |

The four species are distinct and permitted by the original Tower. Coordinate
Earthquake with your partner: Gardevoir's Telepathy avoids ally damage, while
other allies may need Protect. These are useful testing presets; victory is
not guaranteed against every randomly generated NPC pair.

## Relevant source and lifecycle

The Cable Club's `USING_MULTI_BATTLE` is a four-human PvP mode. The test uses
`FRONTIER_MODE_LINK_MULTIS` and the Tower's `USING_BATTLE_TOWER` service, which
require exactly two humans. `FRONTIER_MODE_MULTIS` instead supplies an NPC
partner and does not test the requested cooperation.

| Stage | Source functions or script labels | Responsibility |
| --- | --- | --- |
| Registration | `BattleFrontier_BattleTowerLobby_EventScript_LinkMultisAttendant`, `TryEnterLinkMultisChallenge` in `data/maps/BattleFrontier_BattleTowerLobby/scripts.inc` | Set Tower facility and Link Multi mode; back up the complete local party; choose level mode |
| Eligibility | `CheckPartyIneligibility`, `AppendIfValid` in `src/frontier_util.c`; `GetBattleEntryEligibility`, `CheckBattleEntriesAndGetMessage` in `src/party_menu.c` | Validate two selectable Pokemon and species, level, egg, item restrictions |
| Selection | `ChoosePartyForBattleFrontier`, `CB2_ReturnFromChooseBattleFrontierParty` in `src/script_pokemon_util.c` | Show the original party selection screen and return selected order or cancellation |
| Challenge initialization | `InitTowerChallenge` in `src/battle_tower.c`, dispatched through `CallBattleTowerFunc` | Initialize challenge status and battle number, trainer IDs, streak state, and dynamic return warp |
| Initial save | `SaveBeforeLinkMultisChallenge`, `CableLinkSuccessful` in the lobby script; `SaveForBattleTowerLink` in `src/start_menu.c` | Perform the native pre-challenge save and synchronized link save |
| Cable handshake | `TryBattleLinkup`, `Task_LinkupStart`, `Task_LinkupAwaitConnection`, `Task_LinkupAwaitConfirmation`, `Task_LinkupAwaitTrainerCardData`, `FinishLinkup` in `src/cable_club.c` | Require two players, confirm the connection, exchange player/trainer-card data, and report success or an error |
| Shared species check | `AreBattleTowerLinkSpeciesSame` in `src/cable_club.c` | Reject a selected species appearing on both players' teams |
| Opponent synchronization | `LoadLinkMultiOpponentsData`, `SetNextTowerOpponent` in `src/battle_tower.c` | Exchange challenge difficulty, synchronize the original NPC trainer list, and select the next opponent pair |
| Selected party preparation | `EnterElevator` in the lobby script; `SetSelectedPartyOrder` in `src/frontier_util.c`; `ReducePlayerPartyToSelectedMons` in `src/script_pokemon_util.c` | Back up the complete party, then prepare only the two selections for battle |
| Room and battle start | `BattleFrontier_BattleTowerMultiBattleRoom_EventScript_EnterRoom`, `DoTowerBattle`; common `BattleFrontier_BattleTowerBattleRoom_EventScript_DoTowerBattle` | Run the original room presentation and call `dofacilitytrainerbattle FACILITY_BATTLE_TOWER` |
| Battle type and enemies | `DoFacilityTrainerBattleInternal`, `FillFrontierTrainersParties` in `src/battle_frontier.c` | Set `TRAINER`, `BATTLE_TOWER`, `DOUBLE`, `LINK`, `MULTI`, and `TOWER_LINK_MULTI`; create the two NPC parties |
| Battle connection and data | `HandleLinkBattleSetup` in `src/battle_controllers.c`; `Task_WaitForLinkPlayerConnection` in `src/cable_club.c`; `CB2_HandleStartMultiPartnerBattle` in `src/battle_main.c` | Reopen cable communication, exchange human and NPC parties, select the battle master, and synchronize battle data |
| Human control | `InitBtlControllersInternal`, `GetBattlerTrainer` in `src/battle_controllers.c` | Assign local player and remote partner controllers to separate human parties; the battle master runs the two NPC controllers |
| Transition selection | `GetSpecialBattleTransition` in `src/battle_setup.c` | Choose the Link Multi transition from synchronized opponent IDs instead of independent random choices |
| Battle teardown | `SetBattleEndCallbacks`, `SetLinkBattleEndCallbacks` in `src/battle_controller_player.c`; `CB2_InitEndLinkBattle` in `src/battle_main.c`; `HandleFacilityTrainerBattleEnd` in `src/battle_frontier.c` | Close the wired link and return to the field script after battle |
| Result processing | `DoTowerBattle`, `DefeatedOpponents`, `WarpToLobbyLost` in the battle-room scripts; `SetTowerBattleWon` in `src/battle_tower.c` | Read the outcome, update the native challenge/streak, and continue or exit |
| Continue/retire agreement | `BattleTowerReconnectLink`, `LinkRetireStatusWithBattleTowerPartner`, `Task_LinkRetireStatusWithBattleTowerPartner` in `src/field_specials.c`; `Task_ReconnectWithLinkPlayers` in `src/cable_club.c` | Reconnect the saved participants and exchange each player's continue/retire choice |
| Party and final return | `SavePlayerParty`, `LoadPlayerParty` in `src/load_save.c`; `SaveGameFrontier` in `src/frontier_util.c`; lobby `AskSaveBattle` | Restore the complete original local party, heal it, save the native challenge result, and finish the lobby return |

`CB2_HandleStartMultiPartnerBattle` assigns battlefield IDs 0 and 2 to the two
humans and IDs 1 and 3 to the NPC trainers. It receives each human's Pokemon
into a separate party and synchronizes the NPC parties from player 0. The
controller assignments support one local human selection on each device;
actual behavior must still be checked on the phones.

## Save and compatibility considerations

Use backed-up copies of each player's compatible hack save, or dedicated new
test saves. Copy the normal in-game battery save before starting; an emulator
save state alone is insufficient. **NEW GAME is the normal destructive
new-save path once you save over an existing file.** Use separate test save
locations or external backups before choosing it. Keep each player's save
separate and never copy one player's active save over the other's.

If both phones use the same Google account for My Boy!'s **Sync with Google
Drive**, disable that synchronization for the test or isolate the accounts.
The emulator's [official FAQ](https://sites.google.com/site/fastemulator/frequently-asked-questions)
says it propagates saves between Android devices; keep these two same-ROM saves
independent. Back up each save before changing synchronization settings.

Travel itself never replaces Pokemon. Quick-start/team setup adds the selected
pair once; loading a save does not grant it again. The original Tower
temporarily reduces each in-memory party to its two selections, retaining the
full party through `SavePlayerParty` and `LoadPlayerParty`.
`SaveGameFrontier` restores the full party while writing the save. On normal
exit the party is restored and healed. Verify members, order, moves, held items,
and experience after the test; HP, PP, and status are expected to be healed.

The native Tower writes normal saves, Frontier challenge data, statistics,
selected-party order, and win streaks. Entering therefore updates the current
save; it does not preserve a byte-identical file or a previous Tower streak.
The original seven-win BP and ribbon behavior remains present because the
Tower implementation is reused unchanged. Retire after the first win for this
milestone. No custom reward system has been added.

Both phones must run the exact same ROM file from the same Actions artifact.
Do not mix release and development builds, different commits, or a retail
Emerald ROM. FireRed and LeafGreen builds remain CI compatibility checks;
this early-game access test is in Emerald's Hoenn maps.

The GBA program sees an emulated **Link Cable**. My Boy!'s Wi-Fi transport does
not make this a GBA Wireless Adapter session. The attendant automatically
detects the emulated adapter type; there is no additional in-game "Cable Link"
menu to select. Use the same My Boy! cable-emulation setup that already works
for the players' PvP battles.

## Party eligibility

- Each player selects exactly two eligible Pokemon, one active and one reserve.
- The four selected species must all be different: the original menu rejects
  duplicates within a pair, and the handshake rejects shared species between
  the two players.
- No eggs or species marked `isFrontierBanned` are permitted.
- Within a player's pair, two Pokemon cannot hold the same nonempty item.
  Holding no item is allowed. No cross-player held-item check was found in the
  cable handshake.
- Both players choose the same level mode. **Level 50** permits Pokemon at or
  below level 50; it does not raise early-game Pokemon to level 50, while the
  opponents are level 50. The two presets are already level 50.
- **Open Level** accepts higher levels, but NPC levels have a minimum of 60.
  Avoid it for the first test and use similarly leveled teams on both devices.

Distinct player names make control ownership easier to observe. Separate
trainer identities from independent new saves are recommended; this document
does not establish distinct trainer names or IDs as an additional hard rule.

## Build and download

Use the existing `.github/workflows/build.yml` workflow on
`codex/duo-link-poc`. No local Ubuntu, WSL, or new build environment is needed.

1. Open the repository's
   [Actions workflow](https://github.com/hkcmoris/romhack/actions/workflows/build.yml).
2. Select the run for the intended branch **and commit**. Wait for the complete
   run, including Emerald, FireRed, LeafGreen, release, test, documentation, and
   aggregate build checks.
3. Download `emerald-release` from that run's **Artifacts**, then extract
   `pokeemerald-release.gba`.
4. Transfer that one extracted ROM to both phones. Record the Actions run ID
   and commit; compare file hashes if the transfers are in doubt.
5. Use a separate save for each phone. Choose **NEW GAME** for quick start, or
   **CONTINUE** to retain an existing backed-up compatible save and optionally
   request **TEST TEAM SETUP** from the scientist. Confirm the expected
   character and party before saving over any existing copy.

`emerald-development` is available for development testing, but both devices
must use that identical build if it is chosen. An artifact becoming available
does not mean every workflow job has finished or that multiplayer is verified.
This document is indexed in `docs/SUMMARY.md` for documentation validation.

## Two-phone My Boy! test

1. Back up both normal in-game saves outside the active emulator save location.
   Load the identical test ROM on both phones with independent save files.
   Before choosing **NEW GAME**, ensure saving the new game cannot overwrite
   the only copy of an existing character.
2. Choose **NEW GAME**, then gender and name normally. Each independent new
   game initializes its own Trainer ID and appears outdoors in Littleroot at
   `(17, 13)`, without the truck, wall-clock, rescue, or starter tutorials.
   At **DUO TEST: SELECT TEAM**, choose **TEAM A** on phone A and **TEAM B** on
   phone B. Inspect the two level 50 Pokemon and save normally on each phone.
   For an existing save, use **CONTINUE** instead; no automatic team or skipped
   story states are applied. At the scientist, choose **TEST TEAM SETUP**,
   read the one-time confirmation, and choose **YES** only when ready to append
   the agreed preset to two empty party slots.
3. Restart each ROM and choose **CONTINUE**. Verify the correct player and team
   reload, the team prompt does not recur, and the party has not duplicated.
   Record the full party's members, order, moves, held items, levels, and
   experience as the battle baseline. Include unselected existing Pokemon
   when testing the optional setup path.
4. On both phones, talk to the scientist at `(17, 12)` east of Birch's lab.
   Choose **TRAVEL TO TOWER** and confirm. Both should arrive in the original
   Tower lobby directly below the rightmost Link Multi attendant. Ensure
   neither save has an existing active Frontier challenge.
5. Connect both phones to the same Wi-Fi network or a hotspot. Establish
   My Boy!'s remote cable connection using its menu:
   **Link remote > Wi-Fi (server)** on one phone and **Link remote > Wi-Fi
   (client)** on the other. Enter the server phone's IP address and port on
   the client.
   If one phone provides the hotspot, make that phone the emulator **client**,
   as the emulator's official FAQ directs. Menu wording may depend on version.
6. Talk to the rightmost original Tower attendant on both phones. Choose
   **CHALLENGE**, choose **Level 50** on both, select the two preset Pokemon
   each, and accept the native save prompts. Do not choose an NPC Multi or
   Cable Club four-player mode. Let both games reach their connection prompts.
   The in-game link leader
   confirms when **two** participants are present, following the displayed
   A-button prompts. The My Boy! server/client role and the game's link leader
   are separate concepts; follow the actual screens.
7. Keep the remote emulator link connected during lobby movement, room entry,
   battle setup, and return. The original Tower deliberately closes and
   reopens the emulated cable between phases. Do not disconnect My Boy!'s
   Wi-Fi session when the game changes screens.
8. Observe a double battle against two NPC trainers. Each phone must offer
   choices for its own Pokemon. Select clearly different moves on the two
   phones and verify that both choices occur in the same battle turn. Check
   shared targets, HP, damage, fainting, and outcome agree on both screens.
9. When practical, switch each player's active Pokemon to that player's own
   reserve. Confirm the other phone does not choose that switch for them.
   Normal animations or the need to wait for the other player's input are
   expected; divergent battle state or permanent waiting is a failure.
10. Win the first battle. At the next-opponents prompt choose **RETIRE** on
    both devices and confirm. Complete the native return dialogue and save
    process. Alternatively, losing the battle should take both players back
    through the original loss/return flow.
11. Confirm both have returned to their own Tower lobby and compare the entire
    restored party with the baseline. Expect healing; confirm no party member,
    move, held item, or experience data was lost or replaced.
12. Talk to the return NPC at `(22, 8)` on both phones. Verify each returns to
    Littleroot at `(17, 13)` and can move, use menus, and interact normally.
    Save normally, restart the emulator, and confirm each own save reloads
    without a team prompt or duplicate grants. Fresh quick-start saves skip
    selected story states and are intended for this test milestone.

The network setup above follows the emulator's
[official FAQ](https://sites.google.com/site/fastemulator/frequently-asked-questions).
Use the same known-working Wi-Fi/link settings as the existing PvP baseline.

## Cancellation and failure tests

Repeat these from the backed-up test saves. Record the phase and both screens'
actual behavior; cancellation and an abrupt transport failure are different
cases.

| Test | Procedure | Expected or required observation |
| --- | --- | --- |
| New-save persistence | Select one preset, save, restart with CONTINUE, leave/reenter Littleroot, and request TEST TEAM SETUP again | Correct player and same party reload; no automatic prompt or extra Pokemon; scientist reports the save already received a team |
| Existing-save loading | CONTINUE from a backed-up pre-quick-start save | Existing party and story state remain unchanged; no first-entry team prompt |
| Cancel optional setup | On an existing save, choose TEST TEAM SETUP and answer NO | Party remains unchanged; a later optional setup remains possible |
| Insufficient party space | Request optional setup with five or six party members | Setup refuses without changing Pokemon or consuming the one-time receipt |
| Append to old party | On a backup copy with at most four members, accept optional setup | Exactly two Pokemon are appended; compare all prior Pokemon, including HP, PP, status, moves, and ownership, to confirm setup left them unchanged |
| Receipt after boxing | After receiving a preset and saving, box a test Pokemon, save, restart, and request setup | Existing saved receipt still prevents a second grant |
| Decline travel | Decline the Littleroot NPC's offer | Stay in Littleroot with unchanged party |
| Active challenge guard | Attempt shortcut travel with an existing Frontier challenge | Travel is refused; the existing challenge remains available through its native flow |
| Party cancellation | Cancel the original party selection menu | Return to lobby control with complete party intact |
| Save cancellation | Decline the native pre-challenge save | No battle starts; inspect party and lobby control |
| Handshake cancellation | Before final link confirmation, press B using the native prompt | No co-op battle starts; inspect messages and both parties, then follow the pending-challenge recovery below if travel is refused |
| Different level modes | One player chooses Level 50, the other Open Level | Native connection validation rejects the mismatch; pending-challenge recovery may be needed |
| Shared species | Select one species on both players' pairs | Native handshake rejects the shared species and names the conflict; pending-challenge recovery may be needed |
| One player retires | After a win, one chooses RETIRE and the other GO ON | Native retirement exchange ends the challenge for both; the continuing player should see a partner-retired message |
| Loss | Let the two human teams lose normally | Both complete the stock loss return and regain their full local parties |
| Transport interruption | From backup copies, disable one phone's Wi-Fi or terminate its emulator during handshake, a battle, or return | Record each phase independently; graceful recovery is not guaranteed |

The current configuration defines `BUGFIX` in `include/config/general.h`.
After the pre-link full save, native handshake cancellation or rejection can
leave `CHALLENGE_STATUS_SAVING` set in memory. The new return guide then refuses
travel. This is a native state-handling limitation, not a reason to bypass the
guard or overwrite Frontier state in the shortcut.

For that case, **do not make another in-game save after cancellation**. Reset
both ROM sessions and choose normal **CONTINUE**, avoiding emulator save states.
The pre-link save contains the full party and temporary challenge status `0`;
the lobby's `GetChallengeStatus` and `QuitWithoutSaving` scripts should show the
original interrupted-challenge warning and clear the pending challenge. This
forfeits the native challenge/streak. Dismiss the warning, compare the parties,
use the return guide, then save normally. This recovery is source-derived and
must also be verified on both phones. Restore the external backups if the
expected recovery does not occur.

For an interruption, wait long enough to distinguish normal partner input from
a persistent error or hang. Record which device disconnected, the last action,
the visible message, and whether the other device times out. If either game is
stuck, reset the emulator and reload its last **in-game save**. Inspect both
parties and handle any original Tower interrupted-challenge dialogue before
trying the return NPC. Do not save a suspected damaged session over the only
backup or use an old emulator save state to resume a live link connection.

## Known limitations and remaining risks

- Two-phone My Boy! behavior, including independent control and return, is
  unverified until the manual procedure is completed.
- Existing PvP success is a useful baseline, but Tower Link Multi adds opponent
  data exchange, synchronized NPC control, and repeated cable reconnections.
  It may expose different timing or emulator compatibility problems.
- `Task_WaitForLinkPlayerConnection` has a connection timeout, but not every
  original script/block-exchange wait provides a clean cancellation path.
- `CB2_LinkError` and `CB2_PrintErrorMessage` in `src/link.c` show the native
  communication-error behavior. The wired state does not guarantee an
  automatic return to the overworld; a reset and last-save reload may be
  necessary. This test adds no recovery protocol.
- Pokemon and other save data are not synchronized between players. Each save
  receives only its own native Tower changes. There is no overworld co-op.
- Quick start and optional setup supply one preset per save, with no preset
  reset or exchange action. Both players must choose different presets before
  linking. New-game and save/load UI behavior remains part of the manual test;
  automated grant and native save-data round-trip checks do not exercise the
  complete menu/restart UI or establish My Boy! behavior.
- The quick start skips selected local introduction states only. It is not a
  complete Hoenn unlock or the planned ORÁKULUM introduction, and this milestone
  does not validate playing through the later story from a quick-start save.
- The original seven-battle challenge and its native bookkeeping remain
  accessible. Avoid unrelated Frontier play while isolating this result.
- An upstream update can change link behavior or eligibility. Repeat the
  two-phone test after changing the engine version or multiplayer code.

If testing reveals that reliable play needs substantial engine changes, record
the reproducible failure before expanding the implementation. Stop at this
milestone; custom Rocket/Rival battles, rewards, and story synchronization
depend on a confirmed basic cooperative battle.

## Manual test record

Copy this record for each test attempt. Leave untested cases explicitly marked.

```text
Status: NOT TESTED / PASS / FAIL
Date:
Branch and commit:
Actions run ID and artifact:
ROM SHA-256:
Phone A model, Android version, My Boy! version:
Phone B model, Android version, My Boy! version:
Wi-Fi/hotspot setup; emulator server/client:
Separate save backups and trainer names:
Fresh NEW GAME or existing CONTINUE save:
Preset A/B chosen on each phone:
Direct Littleroot spawn and no tutorial triggers:
Save/restart/reentry: no prompt or duplicate team:
Existing-save optional setup: cancel/capacity/append checks:
Saved receipt after boxing or releasing test Pokemon:
Selected pairs; level mode:
Handshake and room entry:
Independent move selection:
Independent reserve switching:
Matching HP, fainting, and outcome:
First win -> both RETIRE -> lobby:
One RETIRE / one GO ON:
Loss -> lobby:
Full party comparison and healing:
Return to Littleroot; save/restart/reload:
Party/save/handshake cancellation:
Level-mode/shared-species rejection:
Disconnect phase and behavior on each phone:
Recovery and post-reload party comparison:
Screenshots/video and reproducible failure steps:
```
