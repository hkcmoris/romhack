#include "global.h"
#include "agb_flash.h"
#include "battle.h"
#include "battle_setup.h"
#include "data.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "field_control_avatar.h"
#include "field_player_avatar.h"
#include "fieldmap.h"
#include "metatile_behavior.h"
#include "script.h"
#include "heal_location.h"
#include "item.h"
#include "main.h"
#include "money.h"
#include "task.h"
#include "new_game.h"
#include "overworld.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "save.h"
#include "script_pokemon_util.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "text.h"
#include "trainer_pools.h"
#include "virelia_prologue.h"
#include "virelia_first_gym.h"
#include "wild_encounter.h"
#include "constants/virelia_quarry_outdoor.h"
#include "constants/map_types.h"
#include "test/test.h"
#include "constants/abilities.h"
#include "constants/battle_ai.h"
#include "constants/event_objects.h"
#include "constants/heal_locations.h"
#include "constants/items.h"
#include "constants/map_event_ids.h"
#include "constants/moves.h"
#include "constants/metatile_behaviors.h"
#include "constants/opponents.h"
#include "constants/virelia_prologue.h"
#include "constants/virelia_first_gym.h"
#include "constants/virelia_maplebrook.h"

#if !IS_FRLG && P_VIRELIA_STARTERS

static const struct Trainer sFirstGymTrainers[DIFFICULTY_COUNT][TRAINERS_COUNT] =
{
#include "../src/data/trainers.h"
};

static const u16 sChapterTrainers[] =
{
    TRAINER_VIRELIA_MINER_BRUNO, TRAINER_VIRELIA_EXPLORER_LENKA,
    TRAINER_VIRELIA_GYM_IVAN, TRAINER_VIRELIA_GYM_DANA, TRAINER_VIRELIA_MARA,
};

extern const u8 Virelia_Slateford_PokemonCenter_EventScript_Nurse[];
extern const u8 Virelia_Slateford_PokemonCenter_EventScript_Clerk[];

static const struct ObjectEventTemplate *CenterService(const u8 *script)
{
    const struct MapHeader *header = Overworld_GetMapHeaderByGroupAndId(
        MAP_GROUP(MAP_VIRELIA_SLATEFORD_POKEMON_CENTER),
        MAP_NUM(MAP_VIRELIA_SLATEFORD_POKEMON_CENTER));
    for (u32 i = 0; i < header->events->objectEventCount; i++)
        if (header->events->objectEvents[i].script == script)
            return &header->events->objectEvents[i];
    return NULL;
}

static void ResetFirstGymTest(void)
{
    InitEventData();
    ZeroPlayerPartyMons();
    ResetPokemonStorageSystem();
    SetBagItemsPointers();
    ClearBag();
    for (u32 i = FLAG_VIRELIA_MOM_MET; i <= FLAG_VIRELIA_TOOL_PICK; i++)
        FlagClear(i);
    for (u32 i = 0; i < ARRAY_COUNT(sChapterTrainers); i++)
        ClearTrainerFlag(sChapterTrainers[i]);
    VarSet(VAR_VIRELIA_PROLOGUE_STATE, VIRELIA_STATE_HOME);
    VarSet(VAR_VIRELIA_QUARRY_SWITCH, 0);
    VarSet(VAR_VIRELIA_GYM_SWITCH, 0);
}

TEST("Virelia appended trainers preserve the Hoenn flag bank and stay distinct from partners")
{
    ResetFirstGymTest();
    EXPECT_EQ(MAX_TRAINERS_COUNT_EMERALD, 864);
    EXPECT_EQ(TRAINER_FLAGS_START, 0x500);
    EXPECT_EQ(TRAINER_FLAGS_END, 0x85F);
    EXPECT_EQ(FLAG_SYS_POKEMON_GET, 0x860);
    EXPECT_EQ(FLAG_BADGE01_GET, 0x867);
    EXPECT_EQ(FLAG_BADGE08_GET, 0x86E);
    EXPECT_EQ(VireliaFirstGym_TrainerFlag(TRAINER_VIRELIA_ROCKET), TRAINER_FLAGS_START + TRAINER_VIRELIA_ROCKET);
    for (u32 i = 0; i < ARRAY_COUNT(sChapterTrainers); i++)
    {
        u16 trainer = sChapterTrainers[i];
        EXPECT(!IsPartnerTrainerId(trainer));
        EXPECT_LT(trainer, TRAINERS_COUNT);
        EXPECT_EQ(VireliaFirstGym_TrainerFlag(trainer), FLAG_VIRELIA_TRAINER_BRUNO + i);
        EXPECT(!HasTrainerBeenFought(trainer));
        u8 script[TRAINERBATTLE_OPCODE_OFFSET + sizeof(TrainerBattleParameter)] = {0};
        TrainerBattleParameter params = {0};
        params.params.opponentA = trainer;
        memcpy(script + TRAINERBATTLE_OPCODE_OFFSET, &params, sizeof(params));
        EXPECT(!GetTrainerFlagFromScriptPointer(script));
        SetTrainerFlag(trainer);
        EXPECT(GetTrainerFlagFromScriptPointer(script));
        EXPECT(HasTrainerBeenFought(trainer));
        EXPECT(FlagGet(FLAG_VIRELIA_TRAINER_BRUNO + i));
        EXPECT(!FlagGet(TRAINER_FLAGS_START + trainer));
        ClearTrainerFlag(trainer);
        EXPECT(!HasTrainerBeenFought(trainer));
    }
}

// Exercise the actual callback registered by ordinary trainer setup, rather
// than setting the defeated flag from a test helper. Cancel only the pending
// visual transition; neither battle-end branching nor flag writes are mocked.
static MainCallback FinishNativeChapterTrainerBattle(u16 trainerId, u32 outcome)
{
    TrainerBattleParameter originalParams = gTrainerBattleParameter;
    MainCallback originalSavedCallback = gMain.savedCallback;
    MainCallback originalMainCallback = gMain.callback2;
    u8 originalState = gMain.state;
    u32 originalFlags = gBattleTypeFlags;
    u32 originalOutcome = gBattleOutcome;
    bool32 originalDebug = gIsDebugBattle;
    u32 originalApproaching = gNoOfApproachingTrainers;
    u16 originalMusic = GetCurrentMapMusic();
    bool32 originalControlsLocked = ArePlayerFieldControlsLocked();
    InitTrainerBattleParameter();
    TRAINER_BATTLE_PARAM.opponentA = trainerId;
    gNoOfApproachingTrainers = 0;
    gIsDebugBattle = FALSE;
    BattleSetup_StartTrainerBattle();
    MainCallback battleEnd = gMain.savedCallback;
    ResetTasks();
    gBattleOutcome = outcome;
    battleEnd();
    MainCallback nextCallback = gMain.callback2;
    ResetTasks();
    gMain.savedCallback = originalSavedCallback;
    gMain.callback2 = originalMainCallback;
    gMain.state = originalState;
    gTrainerBattleParameter = originalParams;
    gBattleTypeFlags = originalFlags;
    gBattleOutcome = originalOutcome;
    gIsDebugBattle = originalDebug;
    gNoOfApproachingTrainers = originalApproaching;
    ResetMapMusic();
    if (originalMusic)
    {
        PlayNewMapMusic(originalMusic);
        PlayBGM(originalMusic);
    }
    if (originalControlsLocked) LockPlayerFieldControls();
    else UnlockPlayerFieldControls();
    return nextCallback;
}

TEST("Virelia all five trainers white out on defeat and persist only a successful native retry")
{
    u32 index = 0;
    for (u32 i = 0; i < ARRAY_COUNT(sChapterTrainers); i++)
        PARAMETRIZE { index = i; }
    ResetFirstGymTest();
    SetWarpDestination(MAP_GROUP(MAP_VIRELIA_SLATEFORD_CITY),
                       MAP_NUM(MAP_VIRELIA_SLATEFORD_CITY), WARP_ID_NONE, 14, 27);
    WarpIntoMap();
    LoadObjEventTemplatesFromHeader();
    InitMap();
    CreateRandomMonWithIVs(&gParties[B_TRAINER_PLAYER][0], SPECIES_EKANS_VIRELIA, 12, 10);
    gPartiesCount[B_TRAINER_PLAYER] = 1;
    SetLastHealLocationWarp(HEAL_LOCATION_VIRELIA_SLATEFORD_POKEMON_CENTER);
    u16 trainer = sChapterTrainers[index];
    MainCallback afterLoss = FinishNativeChapterTrainerBattle(trainer, B_OUTCOME_LOST);
    EXPECT(afterLoss == CB2_WhiteOut);
    EXPECT(!HasTrainerBeenFought(trainer));
    EXPECT(!FlagGet(FLAG_VIRELIA_TRAINER_BRUNO + index));
    EXPECT(!FlagGet(FLAG_VIRELIA_QUARRY_EVIDENCE));
    EXPECT(!FlagGet(FLAG_VIRELIA_BADGE_QUARRY));
    EXPECT(!FlagGet(FLAG_VIRELIA_TM_ROCK_TOMB_RECEIVED));
    EXPECT_EQ(GetHealLocationIndexByWarpData(&gSaveBlock1Ptr->lastHealLocation),
              HEAL_LOCATION_VIRELIA_SLATEFORD_POKEMON_CENTER);
    MainCallback afterWin = FinishNativeChapterTrainerBattle(trainer, B_OUTCOME_WON);
    EXPECT(afterWin == CB2_ReturnToFieldContinueScriptPlayMapMusic);
    EXPECT(HasTrainerBeenFought(trainer));
    EXPECT(FlagGet(FLAG_VIRELIA_TRAINER_BRUNO + index));
    EXPECT(!FlagGet(TRAINER_FLAGS_START + trainer));
    for (u32 i = 0; i < ARRAY_COUNT(sChapterTrainers); i++)
        EXPECT_EQ(HasTrainerBeenFought(sChapterTrainers[i]), i == index);
    for (u32 flag = FLAG_BADGE01_GET; flag <= FLAG_BADGE08_GET; flag++)
        EXPECT(!FlagGet(flag));
}

TEST("Virelia actual city map load gates the quarry with its manager and the Gym with its door")
{
    u32 checkpoint = 0;
    for (u32 i = 0; i < 3; i++)
        PARAMETRIZE { checkpoint = i; }
    ResetFirstGymTest();
    if (checkpoint >= 1)
    {
        FlagSet(FLAG_VIRELIA_PROLOGUE_COMPLETE);
        VarSet(VAR_VIRELIA_PROLOGUE_STATE, VIRELIA_STATE_COMPLETE);
    }
    if (checkpoint >= 2) FlagSet(FLAG_VIRELIA_QUARRY_COMPLETE);
    SetWarpDestination(MAP_GROUP(MAP_VIRELIA_SLATEFORD_CITY),
                       MAP_NUM(MAP_VIRELIA_SLATEFORD_CITY), WARP_ID_NONE, 14, 27);
    WarpIntoMap();
    LoadObjEventTemplatesFromHeader();
    InitMap();
    RunOnLoadMapScript();
    const struct WarpEvent *quarry = &gMapHeader.events->warps[2];
    EXPECT_EQ(quarry->mapGroup, MAP_GROUP(MAP_VIRELIA_SLATEFORD_QUARRY_ENTRANCE));
    EXPECT_EQ(quarry->mapNum, MAP_NUM(MAP_VIRELIA_SLATEFORD_QUARRY_ENTRANCE));
    s16 quarryX = quarry->x + MAP_OFFSET, quarryY = quarry->y + MAP_OFFSET;
    u32 gymBehavior = MapGridGetMetatileBehaviorAt(22 + MAP_OFFSET, 9 + MAP_OFFSET);
    u32 quarryBehavior = MapGridGetMetatileBehaviorAt(quarryX, quarryY);
    EXPECT_EQ(MetatileBehavior_IsWarpDoor(gymBehavior), checkpoint >= 2);
    EXPECT_NE(MapGridGetCollisionAt(22 + MAP_OFFSET, 9 + MAP_OFFSET), 0);
    EXPECT(MetatileBehavior_IsNonAnimDoor(quarryBehavior));
    EXPECT_EQ(MapGridGetCollisionAt(quarryX, quarryY), 0);
    u16 mouth = gMapHeader.mapLayout->map[quarry->y * gMapHeader.mapLayout->width + quarry->x];
    EXPECT_EQ(MapGridGetMetatileIdAt(quarryX, quarryY), UNPACK_METATILE(mouth));
    const struct ObjectEventTemplate *manager = NULL;
    for (u32 i = 0; i < gMapHeader.events->objectEventCount; i++)
        if (gSaveBlock1Ptr->objectEventTemplates[i].localId == LOCALID_VIRELIA_SLATEFORD_MANAGER)
            manager = &gSaveBlock1Ptr->objectEventTemplates[i];
    EXPECT_NE(manager, NULL);
    EXPECT_EQ(manager->x, quarry->x - (checkpoint >= 1));
    EXPECT_EQ(manager->y, quarry->y + 1);
    EXPECT_EQ(MapGridGetCollisionAt(manager->x + MAP_OFFSET, manager->y + MAP_OFFSET), 0);

    ResetSpriteData();
    ResetObjectEvents();
    gPlayerAvatar.objectEventId = 0;
    struct ObjectEvent *player = &gObjectEvents[0], *blocker = &gObjectEvents[1];
    player->active = TRUE;
    player->localId = LOCALID_PLAYER;
    player->currentCoords.x = quarryX;
    player->currentCoords.y = quarryY + 2;
    player->currentElevation = MapGridGetElevationAt(quarryX, quarryY + 2);
    blocker->active = TRUE;
    blocker->localId = manager->localId;
    blocker->currentCoords.x = blocker->previousCoords.x = manager->x + MAP_OFFSET;
    blocker->currentCoords.y = blocker->previousCoords.y = manager->y + MAP_OFFSET;
    blocker->currentElevation = MapGridGetElevationAt(blocker->currentCoords.x, blocker->currentCoords.y);
    EXPECT_EQ(CheckForObjectEventCollision(player, quarryX, quarryY + 1, DIR_NORTH,
                  MapGridGetMetatileBehaviorAt(quarryX, quarryY + 1)),
              checkpoint >= 1 ? COLLISION_NONE : COLLISION_OBJECT_EVENT);
    SaveMapView();
    InitMapFromSavedGame();
    EXPECT_EQ(manager->x, quarry->x - (checkpoint >= 1));
    EXPECT_EQ(manager->y, quarry->y + 1);
    ResetObjectEvents();
    ResetSpriteData();
    // The gate must not replace the authored cliff cap above the warp.
    u16 cap = gMapHeader.mapLayout->map[(quarry->y - 1) * gMapHeader.mapLayout->width + quarry->x];
    EXPECT_EQ(MapGridGetMetatileIdAt(quarryX, quarryY - 1), UNPACK_METATILE(cap));
    EXPECT_EQ(MapGridGetCollisionAt(quarryX, quarryY - 1), UNPACK_COLLISION(cap));
    // A completed prologue's frame script must not schedule its Rocket finale.
    EXPECT(!TryRunOnFrameMapScript());
    if (checkpoint >= 1)
        EXPECT_EQ(VarGet(VAR_VIRELIA_PROLOGUE_STATE), VIRELIA_STATE_COMPLETE);
}

TEST("Virelia actual quarry and Gym map scripts reset partial input and restore persistent gates")
{
    u32 puzzle = 0, solved = FALSE;
    PARAMETRIZE { puzzle = 0; solved = FALSE; }
    PARAMETRIZE { puzzle = 0; solved = TRUE; }
    PARAMETRIZE { puzzle = 1; solved = FALSE; }
    PARAMETRIZE { puzzle = 1; solved = TRUE; }
    ResetFirstGymTest();
    u16 map = puzzle ? MAP_VIRELIA_SLATEFORD_GYM : MAP_VIRELIA_SLATEFORD_QUARRY_DEPTHS;
    u16 flag = puzzle ? FLAG_VIRELIA_GYM_PUZZLE_COMPLETE : FLAG_VIRELIA_QUARRY_POWER_RESTORED;
    u16 var = puzzle ? VAR_VIRELIA_GYM_SWITCH : VAR_VIRELIA_QUARRY_SWITCH;
    u32 x = puzzle ? 6 : 20, y = puzzle ? 6 : 22;
    if (solved) FlagSet(flag);
    SetWarpDestination(MAP_GROUP(map), MAP_NUM(map), WARP_ID_NONE, puzzle ? 5 : 10, puzzle ? 18 : 4);
    WarpIntoMap();
    InitMap();
    VarSet(var, 1);
    RunOnTransitionMapScript();
    RunOnLoadMapScript();
    EXPECT_EQ(VarGet(var), 0);
    EXPECT_EQ(FlagGet(flag), solved);
    EXPECT_EQ(MapGridGetCollisionAt(x + MAP_OFFSET, y + MAP_OFFSET) != 0, !solved);
    if (solved) EXPECT_EQ(MapGridGetMetatileIdAt(x + MAP_OFFSET, y + MAP_OFFSET), puzzle ? 0x201 : VQ_GROUND);
    if (!puzzle)
    {
        EXPECT_EQ(MapGridGetMetatileIdAt(22 + MAP_OFFSET, 12 + MAP_OFFSET), solved ? VQ_PUMP_CONTROL_ON_0_0 : VQ_PUMP_CONTROL_OFF_0_0);
        EXPECT_EQ(MapGridGetMetatileIdAt(10 + MAP_OFFSET, 12 + MAP_OFFSET), solved ? VQ_ELECTRICAL_FEED_ON_0_0 : VQ_ELECTRICAL_FEED_OFF_0_0);
    }
}

TEST("Virelia outdoor quarry Continue restores whole controls gates and encounter gravel")
{
    u32 stage = 0;
    for (u32 i = 0; i < 3; i++)
        PARAMETRIZE { stage = i; }
    ResetFirstGymTest();
    if (stage == 2) FlagSet(FLAG_VIRELIA_QUARRY_POWER_RESTORED);
    SetWarpDestination(MAP_GROUP(MAP_VIRELIA_SLATEFORD_QUARRY_DEPTHS),
                       MAP_NUM(MAP_VIRELIA_SLATEFORD_QUARRY_DEPTHS), WARP_ID_NONE, 10, 4);
    WarpIntoMap();
    InitMap();
    VarSet(VAR_VIRELIA_QUARRY_SWITCH, stage);
    // Replace both complete controls with stale cached floor, then run the
    // same ON_LOAD callback used after Continue restores the saved map view.
    for (u32 y = 12; y < 14; y++)
        for (u32 x = 10; x < 24; x++)
            MapGridSetMetatileEntryAt(x + MAP_OFFSET, y + MAP_OFFSET, 0x3000 | VQ_GROUND);
    RunOnLoadMapScript();
    EXPECT_EQ(VarGet(VAR_VIRELIA_QUARRY_SWITCH), stage);
    for (u32 row = 0; row < 2; row++)
        for (u32 column = 0; column < 2; column++)
        {
            EXPECT_EQ(MapGridGetMetatileIdAt(22 + column + MAP_OFFSET, 12 + row + MAP_OFFSET),
                      (stage ? VQ_PUMP_CONTROL_ON_0_0 : VQ_PUMP_CONTROL_OFF_0_0) + row * 2 + column);
            EXPECT_EQ(MapGridGetMetatileIdAt(10 + column + MAP_OFFSET, 12 + row + MAP_OFFSET),
                      (stage == 2 ? VQ_ELECTRICAL_FEED_ON_0_0 : VQ_ELECTRICAL_FEED_OFF_0_0) + row * 2 + column);
            EXPECT_NE(MapGridGetCollisionAt(22 + column + MAP_OFFSET, 12 + row + MAP_OFFSET), 0);
            EXPECT_NE(MapGridGetCollisionAt(10 + column + MAP_OFFSET, 12 + row + MAP_OFFSET), 0);
        }
    for (u32 x = 20; x <= 21; x++)
        EXPECT_EQ(MapGridGetCollisionAt(x + MAP_OFFSET, 22 + MAP_OFFSET) != 0, stage != 2);
    EXPECT_EQ(MapGridGetMetatileBehaviorAt(10 + MAP_OFFSET, 18 + MAP_OFFSET), MB_NORMAL);
    EXPECT_EQ(MapGridGetMetatileBehaviorAt(4 + MAP_OFFSET, 23 + MAP_OFFSET), MB_CAVE);
    EXPECT_EQ(gMapHeader.mapType, MAP_TYPE_ROUTE);
}

TEST("Virelia outdoor quarry uses one shared bank native connections and a real city exit")
{
    u32 index = 0;
    for (u32 i = 0; i < 4; i++)
        PARAMETRIZE { index = i; }
    static const u16 maps[] = {MAP_VIRELIA_SLATEFORD_QUARRY_ENTRANCE,
                              MAP_VIRELIA_SLATEFORD_QUARRY_TERRACES,
                              MAP_VIRELIA_SLATEFORD_QUARRY_DEPTHS,
                              MAP_VIRELIA_SLATEFORD_QUARRY_GALLERY};
    ResetFirstGymTest();
    SetWarpDestination(MAP_GROUP(maps[index]), MAP_NUM(maps[index]), WARP_ID_NONE, 10, 15);
    WarpIntoMap();
    InitMap();
    EXPECT_EQ(gMapHeader.mapType, MAP_TYPE_ROUTE);
    EXPECT_EQ(gMapHeader.connections->count, 2);
    EXPECT_EQ(gMapHeader.events->warpCount, index == 0 ? 1 : 0);
    for (u32 i = 0; i < gMapHeader.connections->count; i++)
    {
        const struct MapHeader *other = GetMapHeaderFromConnection(&gMapHeader.connections->connections[i]);
        EXPECT_EQ(gMapHeader.mapLayout->primaryTileset, other->mapLayout->primaryTileset);
        EXPECT_EQ(gMapHeader.mapLayout->secondaryTileset, other->mapLayout->secondaryTileset);
    }
    if (index == 0)
        EXPECT_EQ(MapGridGetMetatileBehaviorAt(9 + MAP_OFFSET, 29 + MAP_OFFSET), MB_SOUTH_ARROW_WARP);
    if (index == 3)
    {
        static const u8 gates[][2] = {{26,8},{27,8},{27,24},{27,25}};
        for (u32 phase = 0; phase < 2; phase++)
        {
            if (phase) FlagSet(FLAG_VIRELIA_QUARRY_POWER_RESTORED);
            // Seed stale opposite-state cache values for every return barrier.
            for (u32 i = 0; i < ARRAY_COUNT(gates); i++)
                MapGridSetMetatileEntryAt(gates[i][0]+MAP_OFFSET, gates[i][1]+MAP_OFFSET,
                                         phase ? 0x3400 | VQ_GATE : 0x3000 | VQ_GROUND);
            RunOnLoadMapScript();
            for (u32 i = 0; i < ARRAY_COUNT(gates); i++)
            {
                EXPECT_EQ(MapGridGetMetatileIdAt(gates[i][0]+MAP_OFFSET, gates[i][1]+MAP_OFFSET), phase ? VQ_GROUND : VQ_GATE);
                EXPECT_EQ(MapGridGetCollisionAt(gates[i][0]+MAP_OFFSET, gates[i][1]+MAP_OFFSET) != 0, !phase);
            }
        }
    }
}

TEST("Virelia outdoor quarry gravel generates the existing early game wild Pokemon")
{
    u32 basin = FALSE;
    PARAMETRIZE { basin = FALSE; }
    PARAMETRIZE { basin = TRUE; }
    ResetFirstGymTest();
    u16 map = basin ? MAP_VIRELIA_SLATEFORD_QUARRY_DEPTHS : MAP_VIRELIA_SLATEFORD_QUARRY_ENTRANCE;
    SetWarpDestination(MAP_GROUP(map), MAP_NUM(map), WARP_ID_NONE, basin ? 4 : 25, basin ? 23 : 4);
    WarpIntoMap();
    InitMap();
    EXPECT(MetatileBehavior_IsLandWildEncounter(MapGridGetMetatileBehaviorAt(
        (basin ? 4 : 25) + MAP_OFFSET, (basin ? 23 : 4) + MAP_OFFSET)));
    EXPECT_EQ(gMapHeader.mapType, MAP_TYPE_ROUTE);
    u16 headerId = GetCurrentMapWildMonHeaderId();
    EXPECT_NE(headerId, HEADER_NONE);
    const struct WildPokemonInfo *info = gWildMonHeaders[headerId].encounterTypes[
        GetTimeOfDayForEncounters(headerId, WILD_AREA_LAND)].landMonsInfo;
    EXPECT_NE(info, NULL);
    ZeroPlayerPartyMons();
    CreateRandomMonWithIVs(&gParties[B_TRAINER_PLAYER][0], SPECIES_TREECKO, 10, 10);
    gPartiesCount[B_TRAINER_PLAYER] = 1;
    EXPECT(TryGenerateWildMon(info, WILD_AREA_LAND, 0));
    u32 level = GetMonData(&gParties[B_TRAINER_OPPONENT_A][0], MON_DATA_LEVEL);
    EXPECT_GE(level, 8);
    EXPECT_LE(level, 13);
}

TEST("Virelia outdoor quarry stairways change native actor elevation through transition tiles")
{
    u32 upper = FALSE;
    PARAMETRIZE { upper = FALSE; }
    PARAMETRIZE { upper = TRUE; }
    ResetFirstGymTest();
    SetWarpDestination(MAP_GROUP(MAP_VIRELIA_SLATEFORD_QUARRY_TERRACES),
                       MAP_NUM(MAP_VIRELIA_SLATEFORD_QUARRY_TERRACES), WARP_ID_NONE, 10, 28);
    WarpIntoMap();
    InitMap();
    struct ObjectEvent object = {0};
    struct Sprite sprite = {0};
    u32 x = upper ? 28 : 10, first = upper ? 13 : 28, last = upper ? 7 : 22;
    object.currentElevation = upper ? 5 : 3;
    object.currentCoords.x = x + MAP_OFFSET;
    object.currentCoords.y = first + MAP_OFFSET;
    for (s32 y = first - 1; y >= last; y--)
    {
        EXPECT(!IsElevationMismatchAt(object.currentElevation, x + MAP_OFFSET, y + MAP_OFFSET));
        object.previousCoords = object.currentCoords;
        object.currentCoords.y = y + MAP_OFFSET;
        ObjectEventUpdateElevation(&object, &sprite);
    }
    EXPECT_EQ((u32)object.currentElevation, upper ? 7 : 5);
}

TEST("Virelia quarry and Gym teams use valid early game data without damaging Ground attacks")
{
    u32 index = 0;
    for (u32 i = 0; i < ARRAY_COUNT(sChapterTrainers); i++)
        PARAMETRIZE { index = i; }
    const struct Trainer *trainer = &sFirstGymTrainers[DIFFICULTY_NORMAL][sChapterTrainers[index]];
    EXPECT_GE((u32)trainer->partySize, 1);
    EXPECT_LE((u32)trainer->partySize, 3);
    EXPECT_NE(trainer->party, NULL);
    EXPECT_EQ((u32)trainer->battleType, TRAINER_BATTLE_TYPE_SINGLES);
    for (u32 i = 0; i < trainer->partySize; i++)
    {
        const struct TrainerMon *mon = &trainer->party[i];
        EXPECT(IsSpeciesEnabled(mon->species));
        EXPECT_GE((u32)mon->lvl, 10);
        EXPECT_LE((u32)mon->lvl, 16);
        for (u32 j = 0; j < MAX_MON_MOVES; j++)
        {
            enum Move move = mon->moves[j];
            if (move == MOVE_NONE)
                continue;
            EXPECT_LT(move, MOVES_COUNT);
            EXPECT_GT((u32)gMovesInfo[move].pp, 0);
            EXPECT(!(gMovesInfo[move].type == TYPE_GROUND && gMovesInfo[move].power > 0));
        }
    }
    if (sChapterTrainers[index] == TRAINER_VIRELIA_MARA)
    {
        EXPECT_EQ((u32)trainer->partySize, 2);
        EXPECT_EQ(trainer->party[0].species, SPECIES_ROGGENROLA);
        EXPECT_EQ(trainer->party[1].species, SPECIES_NOSEPASS);
        EXPECT_EQ((u32)trainer->party[1].lvl, 16);
    }
}

TEST("Virelia Slateford healing sets the native whiteout destination and healer")
{
    ResetFirstGymTest();
    SetWarpDestination(MAP_GROUP(MAP_VIRELIA_SLATEFORD_POKEMON_CENTER),
                       MAP_NUM(MAP_VIRELIA_SLATEFORD_POKEMON_CENTER), 0, 7, 8);
    WarpIntoMap();
    InitMap();
    RunOnTransitionMapScript();
    EXPECT_EQ(GetHealLocationIndexByWarpData(&gSaveBlock1Ptr->lastHealLocation),
              HEAL_LOCATION_VIRELIA_SLATEFORD_POKEMON_CENTER);
    EXPECT(!IsLastHealLocationPlayerHouse());
    struct WarpData respawn = {0};
    SetWhiteoutRespawnWarpAndHealerNPC(&respawn);
    EXPECT_EQ(respawn.mapGroup, MAP_GROUP(MAP_VIRELIA_SLATEFORD_POKEMON_CENTER));
    EXPECT_EQ(respawn.mapNum, MAP_NUM(MAP_VIRELIA_SLATEFORD_POKEMON_CENTER));
    EXPECT_EQ(respawn.x, 7);
    EXPECT_EQ(respawn.y, 4);
    EXPECT_EQ(respawn.warpId, WARP_ID_NONE);
    EXPECT_EQ(gSpecialVar_LastTalked, LOCALID_VIRELIA_SLATEFORD_NURSE);
    EXPECT_EQ(gSpecialVar_0x800B, LOCALID_VIRELIA_SLATEFORD_NURSE);
    CreateRandomMonWithIVs(&gParties[B_TRAINER_PLAYER][0], SPECIES_EKANS_VIRELIA, 12, 10);
    gPartiesCount[B_TRAINER_PLAYER] = 1;
    u32 zero = 0, status = STATUS1_POISON;
    SetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_HP, &zero);
    SetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_STATUS, &status);
    SetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_PP1, &zero);
    SetDefaultFontsPointer();
    const struct FontInfo *originalFonts = gFonts;
    HealPlayerParty();
    EXPECT(gFonts == originalFonts);
    EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_HP),
              GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_MAX_HP));
    EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_STATUS), 0);
    EXPECT_GT(GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_PP1), 0);
}

TEST("Virelia shared Center resolves nurse and shop interactions across their native counters")
{
    u32 shop = FALSE, spritesFull = FALSE;
    PARAMETRIZE { shop = FALSE; spritesFull = FALSE; }
    PARAMETRIZE { shop = TRUE; spritesFull = FALSE; }
    PARAMETRIZE { shop = FALSE; spritesFull = TRUE; }
    PARAMETRIZE { shop = TRUE; spritesFull = TRUE; }
    ResetFirstGymTest();
    // Reproduce render state left by sprite-capacity tests in the full suite.
    ResetSpriteData();
    if (spritesFull)
        for (u32 i = 0; i < MAX_SPRITES; i++)
            EXPECT_EQ(CreateSprite(&gDummySpriteTemplate, 0, 0, 0), i);
    const u8 *script = shop ? Virelia_Slateford_PokemonCenter_EventScript_Clerk
                            : Virelia_Slateford_PokemonCenter_EventScript_Nurse;
    const struct ObjectEventTemplate *service = CenterService(script);
    EXPECT_NE(service, NULL);
    SetWarpDestination(MAP_GROUP(MAP_VIRELIA_SLATEFORD_POKEMON_CENTER),
                       MAP_NUM(MAP_VIRELIA_SLATEFORD_POKEMON_CENTER), WARP_ID_NONE,
                       service->x, service->y + 2);
    WarpIntoMap();
    InitMap();
    LoadObjEventTemplatesFromHeader();
    s16 x = service->x + MAP_OFFSET, y = service->y + MAP_OFFSET;
    EXPECT_EQ(MapGridGetCollisionAt(x, y + 2), 0);
    EXPECT_EQ(MapGridGetMetatileBehaviorAt(x, y + 1), MB_COUNTER);
    EXPECT_NE(MapGridGetCollisionAt(x, y + 1), 0);

    // Use the ordinary field interaction resolver with the player standing
    // south of the counter, so a blocked decorative tile cannot pass this test.
    // ResetObjectEvents allocates reflection sprites. Match normal map loading
    // by clearing sprite slots and affine matrices before object initialization.
    ResetSpriteData();
    ResetObjectEvents();
    gPlayerAvatar.objectEventId = 0;
    gObjectEvents[0].active = TRUE;
    gObjectEvents[0].localId = LOCALID_PLAYER;
    gObjectEvents[0].currentCoords.x = x;
    gObjectEvents[0].currentCoords.y = y + 2;
    gObjectEvents[0].currentElevation = service->elevation;
    gObjectEvents[0].previousElevation = service->elevation;
    gObjectEvents[0].facingDirection = DIR_NORTH;
    gObjectEvents[0].movementDirection = DIR_NORTH;
    gObjectEvents[1].active = TRUE;
    gObjectEvents[1].localId = service->localId;
    gObjectEvents[1].mapGroup = MAP_GROUP(MAP_VIRELIA_SLATEFORD_POKEMON_CENTER);
    gObjectEvents[1].mapNum = MAP_NUM(MAP_VIRELIA_SLATEFORD_POKEMON_CENTER);
    gObjectEvents[1].currentCoords.x = x;
    gObjectEvents[1].currentCoords.y = y;
    gObjectEvents[1].currentElevation = service->elevation;
    EXPECT_EQ(GetObjectEventScriptPointerPlayerFacing(), script);
    EXPECT_EQ(gSpecialVar_LastTalked, service->localId);
    ResetObjectEvents();
    // Object reset recreates reflections; release those before the next test.
    ResetSpriteData();
}

TEST("Virelia regional quarry party can use the ordinary PC storage data path")
{
    ResetFirstGymTest();
    struct Pokemon original, withdrawn;
    CreateRandomMonWithIVs(&original, SPECIES_SANDSHREW, 11, 10);
    VarSet(VAR_PC_BOX_TO_SEND_MON, 0);
    EXPECT_EQ(CopyMonToPC(&original), MON_GIVEN_TO_PC);
    EXPECT_EQ(GetBoxMonDataAt(0, 0, MON_DATA_SPECIES), SPECIES_SANDSHREW);
    BoxMonAtToMon(0, 0, &withdrawn);
    EXPECT_EQ(GetMonData(&withdrawn, MON_DATA_SPECIES), SPECIES_SANDSHREW);
    EXPECT_EQ(GetMonData(&withdrawn, MON_DATA_LEVEL), 11);
    EXPECT_EQ(GetMonData(&withdrawn, MON_DATA_OT_ID), GetMonData(&original, MON_DATA_OT_ID));
    EXPECT_EQ(GetMonData(&withdrawn, MON_DATA_SANITY_IS_BAD_EGG), FALSE);
    ZeroBoxMonAt(0, 0);
    EXPECT_EQ(GetBoxMonDataAt(0, 0, MON_DATA_SPECIES), SPECIES_NONE);
}


#if VIRELIA_PROLOGUE && !VIRELIA_DEVELOPMENT_MODE

static void CompleteOldPrologue(u32 choice)
{
    ResetFirstGymTest();
    SetTrainerId(0x1234ABCD, gSaveBlock2Ptr->playerTrainerId);
    StringCopy(gSaveBlock2Ptr->playerName, COMPOUND_STRING("Virelia"));
    VireliaPrologue_MeetMom();
    gSpecialVar_0x8004 = choice;
    VireliaPrologue_GiveStarter();
    EXPECT_EQ(gSpecialVar_Result, TRUE);
    gBattleOutcome = B_OUTCOME_WON;
    VireliaPrologue_FinishRival();
    VireliaPrologue_GrantBasics();
    VireliaPrologue_FinishRocket();
    VireliaPrologue_Complete();
    EXPECT(FlagGet(FLAG_VIRELIA_PROLOGUE_COMPLETE));
    EXPECT_EQ(VarGet(VAR_VIRELIA_PROLOGUE_STATE), VIRELIA_STATE_COMPLETE);
}

static void SolveQuarry(void)
{
    gSpecialVar_0x8004 = 1;
    VireliaFirstGym_QuarrySwitch();
    gSpecialVar_0x8004 = 2;
    VireliaFirstGym_QuarrySwitch();
    SetTrainerFlag(TRAINER_VIRELIA_MINER_BRUNO);
    SetTrainerFlag(TRAINER_VIRELIA_EXPLORER_LENKA);
    VireliaFirstGym_ReadEvidence();
    VireliaFirstGym_CompleteQuarry();
    EXPECT_EQ(gSpecialVar_Result, TRUE);
}

static void SolveGym(void)
{
    gSpecialVar_0x8004 = 1;
    VireliaFirstGym_GymSwitch();
    gSpecialVar_0x8004 = 2;
    VireliaFirstGym_GymSwitch();
    SetTrainerFlag(TRAINER_VIRELIA_GYM_IVAN);
    SetTrainerFlag(TRAINER_VIRELIA_GYM_DANA);
    SetTrainerFlag(TRAINER_VIRELIA_MARA);
    VireliaFirstGym_AwardBadge();
    EXPECT_EQ(gSpecialVar_Result, TRUE);
}

TEST("Virelia completed prologue enters an independent chapter with all three starter choices")
{
    u32 choice = 0;
    for (u32 i = 0; i < VIRELIA_STARTER_COUNT; i++)
        PARAMETRIZE { choice = i; }
    CompleteOldPrologue(choice);
    struct Pokemon starter = gParties[B_TRAINER_PLAYER][0];
    u32 balls = CountTotalItemQuantityInBag(ITEM_POKE_BALL);
    u32 potions = CountTotalItemQuantityInBag(ITEM_POTION);
    // Alternate old Hoenn badges so accidental set/clear operations both fail.
    for (u32 flag = FLAG_BADGE01_GET; flag <= FLAG_BADGE08_GET; flag++)
    {
        if (flag & 1) FlagSet(flag);
        else FlagClear(flag);
    }
    u8 protectedFlags[sizeof(gSaveBlock1Ptr->flags)];
    memcpy(protectedFlags, gSaveBlock1Ptr->flags, sizeof(protectedFlags));
    VireliaFirstGym_BeginChapter();
    EXPECT_EQ(gSpecialVar_Result, TRUE);
    SolveQuarry();
    SolveGym();
    VireliaFirstGym_AwardTM();
    EXPECT_EQ(gSpecialVar_Result, TRUE);
    EXPECT(FlagGet(FLAG_VIRELIA_FIRST_GYM_COMPLETE));
    VireliaFirstGym_BeginChapter();
    EXPECT_EQ(VarGet(VAR_VIRELIA_PROLOGUE_STATE), VIRELIA_STATE_COMPLETE);
    EXPECT_EQ(VarGet(VAR_VIRELIA_STARTER), choice);
    EXPECT_EQ(memcmp(&starter, &gParties[B_TRAINER_PLAYER][0], sizeof(starter)), 0);
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_POKE_BALL), balls);
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_POTION), potions);
    // Every existing flag outside the new reserved range remains byte-identical.
    for (u32 flag = FLAG_VIRELIA_MOM_MET; flag < sizeof(protectedFlags) * 8; flag++)
        if (flag < FLAG_VIRELIA_CHAPTER_STARTED || flag > FLAG_VIRELIA_TRAINER_MARA)
            EXPECT_EQ(FlagGet(flag), !!(protectedFlags[flag / 8] & (1 << (flag % 8))));
}

TEST("Virelia quarry evidence requires restored power and both ordinary trainer victories")
{
    CompleteOldPrologue(VIRELIA_STARTER_FIRE);
    VireliaFirstGym_BeginChapter();
    for (u32 prerequisite = 0; prerequisite < 3; prerequisite++)
    {
        VireliaFirstGym_ReadEvidence();
        EXPECT_EQ(gSpecialVar_Result, FALSE);
        EXPECT(!FlagGet(FLAG_VIRELIA_QUARRY_EVIDENCE));
        VireliaFirstGym_CompleteQuarry();
        EXPECT_EQ(gSpecialVar_Result, FALSE);
        if (prerequisite == 0) FlagSet(FLAG_VIRELIA_QUARRY_POWER_RESTORED);
        if (prerequisite == 1) SetTrainerFlag(TRAINER_VIRELIA_MINER_BRUNO);
        if (prerequisite == 2) SetTrainerFlag(TRAINER_VIRELIA_EXPLORER_LENKA);
    }
    VireliaFirstGym_ReadEvidence();
    EXPECT_EQ(gSpecialVar_Result, TRUE);
    VireliaFirstGym_CompleteQuarry();
    EXPECT_EQ(gSpecialVar_Result, TRUE);
    EXPECT(FlagGet(FLAG_VIRELIA_QUARRY_COMPLETE));
}

TEST("Virelia Gym badge and TM wait for all requirements and cannot be awarded twice")
{
    CompleteOldPrologue(VIRELIA_STARTER_WATER);
    VireliaFirstGym_BeginChapter();
    VireliaFirstGym_AwardBadge();
    EXPECT_EQ(gSpecialVar_Result, FALSE);
    VireliaFirstGym_AwardTM();
    EXPECT_EQ(gSpecialVar_Result, FALSE);
    SolveQuarry();
    gSpecialVar_0x8004 = 1;
    VireliaFirstGym_GymSwitch();
    gSpecialVar_0x8004 = 2;
    VireliaFirstGym_GymSwitch();
    for (u32 i = 2; i < ARRAY_COUNT(sChapterTrainers); i++)
    {
        VireliaFirstGym_AwardBadge();
        EXPECT_EQ(gSpecialVar_Result, FALSE);
        // Missing native trainer victory flags keep the reward locked.
        gBattleOutcome = B_OUTCOME_LOST;
        EXPECT(!HasTrainerBeenFought(sChapterTrainers[i]));
        SetTrainerFlag(sChapterTrainers[i]);
    }
    VireliaFirstGym_AwardBadge();
    EXPECT_EQ(gSpecialVar_Result, TRUE);
    VireliaFirstGym_AwardBadge();
    EXPECT_EQ(gSpecialVar_Result, FALSE);
    struct BagPocket *pocket = &gBagPockets[POCKET_TM_HM];
    for (u32 i = 0; i < pocket->capacity; i++)
        BagPocket_SetSlotItemIdAndCount(pocket, i, ITEM_TM_PROTECT, MAX_BAG_ITEM_CAPACITY);
    EXPECT(!CheckBagHasSpace(ITEM_TM_ROCK_TOMB, 1));
    VireliaFirstGym_AwardTM();
    EXPECT_EQ(gSpecialVar_Result, FALSE);
    EXPECT(!FlagGet(FLAG_VIRELIA_TM_ROCK_TOMB_RECEIVED));
    EXPECT(!FlagGet(FLAG_VIRELIA_FIRST_GYM_COMPLETE));
    EXPECT(FlagGet(FLAG_VIRELIA_BADGE_QUARRY));
    ClearBag();
    VireliaFirstGym_AwardTM();
    EXPECT_EQ(gSpecialVar_Result, TRUE);
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_TM_ROCK_TOMB), 1);
    VireliaFirstGym_AwardTM();
    EXPECT_EQ(gSpecialVar_Result, FALSE);
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_TM_ROCK_TOMB), 1);
}

TEST("Virelia shared Center shop-wing save restores money items healing destination and chapter flags")
{
    CompleteOldPrologue(VIRELIA_STARTER_GRASS);
    VireliaFirstGym_BeginChapter();
    SolveQuarry();
    SolveGym();
    VireliaFirstGym_AwardTM();
    VireliaFirstGym_AwardCut();
    EXPECT_EQ(gSpecialVar_Result, TRUE);
    FlagSet(FLAG_VIRELIA_MAPLEBROOK_WOODLAND_ITEM);
    EXPECT(AddBagItem(ITEM_MIRACLE_SEED, 1));
    VireliaFirstGym_TakeQuarryItem();
    static const u16 savedFlags[] =
    {
        FLAG_VIRELIA_PROLOGUE_COMPLETE, FLAG_VIRELIA_CHAPTER_STARTED,
        FLAG_VIRELIA_QUARRY_POWER_RESTORED, FLAG_VIRELIA_QUARRY_EVIDENCE,
        FLAG_VIRELIA_QUARRY_COMPLETE, FLAG_VIRELIA_GYM_PUZZLE_COMPLETE,
        FLAG_VIRELIA_BADGE_QUARRY, FLAG_VIRELIA_TM_ROCK_TOMB_RECEIVED,
        FLAG_VIRELIA_ITEM_QUARRY_SUPER_POTION, FLAG_VIRELIA_FIRST_GYM_COMPLETE,
        FLAG_VIRELIA_MISSING_TOOLS_STARTED, FLAG_VIRELIA_TOOL_WRENCH,
        FLAG_VIRELIA_TOOL_HAMMER, FLAG_VIRELIA_TOOL_PICK,
        FLAG_VIRELIA_MISSING_TOOLS_RECOVERED, FLAG_VIRELIA_MISSING_TOOLS_REWARDED,
        FLAG_VIRELIA_HM_CUT_RECEIVED, FLAG_VIRELIA_MAPLEBROOK_WOODLAND_ITEM,
    };
    VireliaFirstGym_BeginMissingTools();
    for (u32 tool = 1; tool <= 3; tool++)
    {
        gSpecialVar_0x8004 = tool;
        VireliaFirstGym_CollectTool();
    }
    gSpecialVar_0x8004 = B_OUTCOME_CAUGHT;
    VireliaFirstGym_RecoverTools();
    VireliaFirstGym_RewardTools();
    const struct ObjectEventTemplate *clerk = CenterService(Virelia_Slateford_PokemonCenter_EventScript_Clerk);
    EXPECT_NE(clerk, NULL);
    SetWarpDestination(MAP_GROUP(MAP_VIRELIA_SLATEFORD_POKEMON_CENTER),
                       MAP_NUM(MAP_VIRELIA_SLATEFORD_POKEMON_CENTER), WARP_ID_NONE,
                       clerk->x, clerk->y + 2);
    WarpIntoMap();
    InitMap();
    RunOnTransitionMapScript();
    EXPECT_EQ(MapGridGetCollisionAt(clerk->x + MAP_OFFSET, clerk->y + 2 + MAP_OFFSET), 0);
    SetMoney(&gSaveBlock1Ptr->money, 3210);
    EXPECT(AddBagItem(ITEM_POTION, 2));
    u32 savedPotions = CountTotalItemQuantityInBag(ITEM_POTION);
    ClearTempFieldEventData();
    IntrFunc originalTimer3Handler, unusedFlashHandler;
    u16 originalIme = REG_IME;
    REG_IME = 0;
    u16 originalIe = REG_IE;
    originalTimer3Handler = gIntrTable[2];
    u16 originalTimer3Low = REG_TM3CNT_L, originalTimer3High = REG_TM3CNT_H;
    REG_TM3CNT_H = 0;
    REG_IF = INTR_FLAG_TIMER3;
    SetFlashTimerIntr(3, &gIntrTable[2]);
    REG_IME = originalIme;
    ClearSaveData();
    Save_ResetSaveCounters();
    u8 saveStatus = TrySavingData(SAVE_NORMAL), loadStatus = SAVE_STATUS_ERROR;
    if (saveStatus == SAVE_STATUS_OK)
    {
        for (u32 i = 0; i < ARRAY_COUNT(savedFlags); i++) FlagClear(savedFlags[i]);
        for (u32 i = 0; i < ARRAY_COUNT(sChapterTrainers); i++) ClearTrainerFlag(sChapterTrainers[i]);
        ClearBag();
        SetMoney(&gSaveBlock1Ptr->money, 0);
        memset(&gSaveBlock1Ptr->lastHealLocation, 0, sizeof(gSaveBlock1Ptr->lastHealLocation));
        SetWarpDestination(MAP_GROUP(MAP_VIRELIA_SLATEFORD_CITY),
                           MAP_NUM(MAP_VIRELIA_SLATEFORD_CITY), WARP_ID_NONE, 14, 27);
        WarpIntoMap();
        loadStatus = LoadGameSave(SAVE_NORMAL);
    }
    REG_IME = 0;
    REG_TM3CNT_H = 0;
    REG_IF = INTR_FLAG_TIMER3;
    SetFlashTimerIntr(2, &unusedFlashHandler);
    gIntrTable[2] = originalTimer3Handler;
    REG_TM3CNT_L = originalTimer3Low;
    REG_TM3CNT_H = originalTimer3High;
    REG_IE = originalIe;
    REG_IME = originalIme;
    EXPECT_EQ(saveStatus, SAVE_STATUS_OK);
    EXPECT_EQ(loadStatus, SAVE_STATUS_OK);
    EXPECT_EQ(gSaveBlock1Ptr->location.mapGroup, MAP_GROUP(MAP_VIRELIA_SLATEFORD_POKEMON_CENTER));
    EXPECT_EQ(gSaveBlock1Ptr->location.mapNum, MAP_NUM(MAP_VIRELIA_SLATEFORD_POKEMON_CENTER));
    EXPECT_EQ(gSaveBlock1Ptr->pos.x, clerk->x);
    EXPECT_EQ(gSaveBlock1Ptr->pos.y, clerk->y + 2);
    EXPECT_EQ(GetMoney(&gSaveBlock1Ptr->money), 3210);
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_POTION), savedPotions);
    EXPECT_EQ(GetHealLocationIndexByWarpData(&gSaveBlock1Ptr->lastHealLocation),
              HEAL_LOCATION_VIRELIA_SLATEFORD_POKEMON_CENTER);
    SetWarpDestination(MAP_GROUP(MAP_VIRELIA_SLATEFORD_POKEMON_CENTER),
                       MAP_NUM(MAP_VIRELIA_SLATEFORD_POKEMON_CENTER), WARP_ID_NONE,
                       clerk->x, clerk->y + 2);
    WarpIntoMap();
    InitMap();
    EXPECT_EQ(MapGridGetCollisionAt(clerk->x + MAP_OFFSET, clerk->y + 2 + MAP_OFFSET), 0);
    EXPECT_EQ(MapGridGetMetatileBehaviorAt(clerk->x + MAP_OFFSET, clerk->y + 1 + MAP_OFFSET), MB_COUNTER);
    for (u32 i = 0; i < ARRAY_COUNT(savedFlags); i++) EXPECT(FlagGet(savedFlags[i]));
    for (u32 i = 0; i < ARRAY_COUNT(sChapterTrainers); i++) EXPECT(HasTrainerBeenFought(sChapterTrainers[i]));
    EXPECT_EQ(VarGet(VAR_VIRELIA_PROLOGUE_STATE), VIRELIA_STATE_COMPLETE);
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_TM_ROCK_TOMB), 1);
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_SUPER_POTION), 1);
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_HARD_STONE), 1);
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_HM_CUT), 1);
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_MIRACLE_SEED), 1);
    VireliaFirstGym_AwardCut();
    EXPECT_EQ(gSpecialVar_Result, FALSE);
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_HM_CUT), 1);
    VireliaFirstGym_RewardTools();
    EXPECT_EQ(gSpecialVar_Result, FALSE);
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_HARD_STONE), 1);
    VireliaFirstGym_ResetQuarryPuzzle();
    VireliaFirstGym_ResetGymPuzzle();
    EXPECT(FlagGet(FLAG_VIRELIA_QUARRY_POWER_RESTORED));
    EXPECT(FlagGet(FLAG_VIRELIA_GYM_PUZZLE_COMPLETE));
    VireliaFirstGym_AwardTM();
    EXPECT_EQ(gSpecialVar_Result, FALSE);
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_TM_ROCK_TOMB), 1);
}

#else

TEST("Virelia development build retains test facilities without starting the story chapter")
{
    ResetFirstGymTest();
    FlagSet(FLAG_VIRELIA_PROLOGUE_COMPLETE);
    VireliaFirstGym_BeginChapter();
    EXPECT_EQ(gSpecialVar_Result, FALSE);
    EXPECT(!FlagGet(FLAG_VIRELIA_CHAPTER_STARTED));
    VireliaFirstGym_AwardBadge();
    VireliaFirstGym_AwardTM();
    EXPECT(!FlagGet(FLAG_VIRELIA_BADGE_QUARRY));
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_TM_ROCK_TOMB), 0);
}

#endif
#endif
