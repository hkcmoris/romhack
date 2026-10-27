#include "global.h"
#include "agb_flash.h"
#include "battle.h"
#include "battle_setup.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "field_control_avatar.h"
#include "field_player_avatar.h"
#include "fieldmap.h"
#include "heal_location.h"
#include "item.h"
#include "main.h"
#include "money.h"
#include "overworld.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "save.h"
#include "sound.h"
#include "sprite.h"
#include "task.h"
#include "test/test.h"
#include "test/overworld_script.h"
#include "virelia_prologue.h"
#include "virelia_first_gym.h"
#include "virelia_rainfall.h"
#include "constants/heal_locations.h"
#include "constants/items.h"
#include "constants/pokemon.h"
#include "constants/metatile_behaviors.h"
#include "constants/opponents.h"
#include "constants/virelia_prologue.h"
#include "constants/virelia_first_gym.h"
#include "constants/virelia_sanctuary.h"
#include "constants/virelia_rainfall.h"

#if !IS_FRLG && VIRELIA_PROLOGUE && !VIRELIA_DEVELOPMENT_MODE

static const u16 sRainfallTrainers[] =
{
    TRAINER_VIRELIA_RAINFALL_HIKER, TRAINER_VIRELIA_RAINFALL_RANGER,
    TRAINER_VIRELIA_RAINFALL_FISHERMAN, TRAINER_VIRELIA_RAINFALL_WEATHER,
    TRAINER_VIRELIA_VIKTOR_RAINFALL_GRASS, TRAINER_VIRELIA_VIKTOR_RAINFALL_WATER,
    TRAINER_VIRELIA_VIKTOR_RAINFALL_FIRE, TRAINER_VIRELIA_VIKTORIE_RAINFALL_GRASS,
    TRAINER_VIRELIA_VIKTORIE_RAINFALL_WATER, TRAINER_VIRELIA_VIKTORIE_RAINFALL_FIRE,
};

static void ResetRainfall(void)
{
    InitEventData();
    SetBagItemsPointers();
    ClearBag();
    ZeroPlayerPartyMons();
    for (u32 flag = FLAG_VIRELIA_RAINFALL_STARTED; flag <= FLAG_VIRELIA_TRAINER_RAINFALL_RIVAL; flag++)
        FlagClear(flag);
    FlagClear(FLAG_VIRELIA_FIRST_GYM_COMPLETE);
    VarSet(VAR_VIRELIA_SIGNAL_INPUT, 0);
    VarSet(VAR_VIRELIA_BOTTLE_STAGE, 0);
    VarSet(VAR_VIRELIA_STARTER, VIRELIA_STARTER_WATER);
    VarSet(VAR_VIRELIA_PROLOGUE_STATE, VIRELIA_STATE_COMPLETE);
}

static void LoadRainfallMap(u32 map)
{
    SetWarpDestination(map >> 8, map & 0xFF, WARP_ID_NONE, 7, 8);
    WarpIntoMap();
    LoadObjEventTemplatesFromHeader();
    InitMap();
    RunOnLoadMapScript();
}

TEST("Rainfall compiled chapter entry requires Mara and preserves earlier progression")
{
    ResetRainfall();
    FlagSet(FLAG_VIRELIA_PROLOGUE_COMPLETE);
    FlagSet(FLAG_VIRELIA_QUARRY_COMPLETE);
    FlagSet(FLAG_VIRELIA_SANCTUARY_SHORTCUT_OPEN);
    RUN_OVERWORLD_SCRIPT(call VireliaRainfall_EventScript_BeginChapter;);
    EXPECT_EQ(gSpecialVar_Result, FALSE);
    EXPECT(!FlagGet(FLAG_VIRELIA_RAINFALL_STARTED));
    RUN_OVERWORLD_SCRIPT(call VireliaRainfall_EventScript_HarborReport;);
    RUN_OVERWORLD_SCRIPT(call VireliaRainfall_EventScript_InspectDamage;);
    gSpecialVar_0x8004 = 1;
    RUN_OVERWORLD_SCRIPT(call VireliaRainfall_EventScript_SignalSwitch;);
    RUN_OVERWORLD_SCRIPT(call VireliaRainfall_EventScript_ReadManifest;);
    RUN_OVERWORLD_SCRIPT(call VireliaRainfall_EventScript_CompleteChapter;);
    EXPECT(!FlagGet(FLAG_VIRELIA_RAINFALL_COMPLETE));
    EXPECT(!FlagGet(FLAG_VIRELIA_LIGHTHOUSE_DAMAGE));
    FlagSet(FLAG_VIRELIA_FIRST_GYM_COMPLETE);
    RUN_OVERWORLD_SCRIPT(call VireliaRainfall_EventScript_BeginChapter;);
    EXPECT_EQ(gSpecialVar_Result, TRUE);
    EXPECT(FlagGet(FLAG_VIRELIA_RAINFALL_STARTED));
    EXPECT(FlagGet(FLAG_VIRELIA_PROLOGUE_COMPLETE));
    EXPECT(FlagGet(FLAG_VIRELIA_QUARRY_COMPLETE));
    EXPECT(FlagGet(FLAG_VIRELIA_SANCTUARY_SHORTCUT_OPEN));
    EXPECT_EQ(VarGet(VAR_VIRELIA_PROLOGUE_STATE), VIRELIA_STATE_COMPLETE);
    for (u32 flag = FLAG_BADGE01_GET; flag <= FLAG_BADGE08_GET; flag++)
        EXPECT(!FlagGet(flag));
}

TEST("Rainfall compiled lighthouse sequence rejects wrong input and restores a completed beacon")
{
    ResetRainfall();
    FlagSet(FLAG_VIRELIA_FIRST_GYM_COMPLETE);
    RUN_OVERWORLD_SCRIPT(call VireliaRainfall_EventScript_BeginChapter;);
    RUN_OVERWORLD_SCRIPT(call VireliaRainfall_EventScript_HarborReport;);
    EXPECT_EQ(gSpecialVar_Result, FALSE);
    FlagSet(FLAG_VIRELIA_RAINFALL_RIVAL_COMPLETE);
    RUN_OVERWORLD_SCRIPT(call VireliaRainfall_EventScript_HarborReport;);
    RUN_OVERWORLD_SCRIPT(call VireliaRainfall_EventScript_InspectDamage;);
    EXPECT_EQ(gSpecialVar_Result, TRUE);
    gSpecialVar_0x8004 = 2;
    RUN_OVERWORLD_SCRIPT(call VireliaRainfall_EventScript_SignalSwitch;);
    EXPECT_EQ(VarGet(VAR_VIRELIA_SIGNAL_INPUT), 0);
    gSpecialVar_0x8004 = 1;
    RUN_OVERWORLD_SCRIPT(call VireliaRainfall_EventScript_SignalSwitch;);
    EXPECT_EQ(VarGet(VAR_VIRELIA_SIGNAL_INPUT), 1);
    LoadRainfallMap(MAP_VIRELIA_SEABREAK_LANTERN_ROOM);
    RunOnTransitionMapScript();
    EXPECT_EQ(VarGet(VAR_VIRELIA_SIGNAL_INPUT), 0);
    EXPECT(!FlagGet(FLAG_VIRELIA_LIGHTHOUSE_REPAIRED));
    for (u32 i = 1; i <= 3; i++)
    {
        gSpecialVar_0x8004 = i;
        RUN_OVERWORLD_SCRIPT(call VireliaRainfall_EventScript_SignalSwitch;);
    }
    EXPECT(FlagGet(FLAG_VIRELIA_LIGHTHOUSE_REPAIRED));
    ClearTempFieldEventData();
    RunOnTransitionMapScript();
    RunOnLoadMapScript();
    EXPECT_EQ(MapGridGetMetatileIdAt(6 + MAP_OFFSET, 2 + MAP_OFFSET), 0x20D);
    EXPECT_EQ(VarGet(VAR_VIRELIA_SIGNAL_INPUT), 0);
    RUN_OVERWORLD_SCRIPT(call VireliaRainfall_EventScript_CompleteChapter;);
    EXPECT_EQ(gSpecialVar_Result, FALSE);
    RUN_OVERWORLD_SCRIPT(call VireliaRainfall_EventScript_ReadManifest;);
    RUN_OVERWORLD_SCRIPT(call VireliaRainfall_EventScript_CompleteChapter;);
    EXPECT_EQ(gSpecialVar_Result, TRUE);
    EXPECT(FlagGet(FLAG_VIRELIA_SHIPPING_RECORD));
    EXPECT(FlagGet(FLAG_VIRELIA_HARBOR_REOPENED));
    EXPECT(FlagGet(FLAG_VIRELIA_RAINFALL_COMPLETE));
}

TEST("Rainfall optional bottle clues require exploration and full Bag rewards retry exactly once")
{
    ResetRainfall();
    FlagSet(FLAG_VIRELIA_FIRST_GYM_COMPLETE);
    RUN_OVERWORLD_SCRIPT(call VireliaRainfall_EventScript_BeginBottle;);
    EXPECT_EQ(VarGet(VAR_VIRELIA_BOTTLE_STAGE), 1);
    gSpecialVar_0x8004 = 4;
    RUN_OVERWORLD_SCRIPT(call VireliaRainfall_EventScript_BottleClue;);
    EXPECT_EQ(VarGet(VAR_VIRELIA_BOTTLE_STAGE), 1);
    RUN_OVERWORLD_SCRIPT(call VireliaRainfall_EventScript_RewardBottle;);
    EXPECT_EQ(gSpecialVar_Result, FALSE);
    for (u32 i = 2; i <= 4; i++)
    {
        gSpecialVar_0x8004 = i;
        RUN_OVERWORLD_SCRIPT(call VireliaRainfall_EventScript_BottleClue;);
        EXPECT_EQ(VarGet(VAR_VIRELIA_BOTTLE_STAGE), i);
    }
    struct BagPocket *pocket = &gBagPockets[POCKET_ITEMS];
    for (u32 i = 0; i < pocket->capacity; i++)
        BagPocket_SetSlotItemIdAndCount(pocket, i, ITEM_ANTIDOTE, MAX_BAG_ITEM_CAPACITY);
    EXPECT(!CheckBagHasSpace(ITEM_MYSTIC_WATER, 1));
    RUN_OVERWORLD_SCRIPT(call VireliaRainfall_EventScript_RewardBottle;);
    EXPECT(!FlagGet(FLAG_VIRELIA_BOTTLE_REWARDED));
    EXPECT_EQ(VarGet(VAR_VIRELIA_BOTTLE_STAGE), 4);
    ClearBag();
    RUN_OVERWORLD_SCRIPT(call VireliaRainfall_EventScript_RewardBottle;);
    EXPECT_EQ(gSpecialVar_Result, TRUE);
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_MYSTIC_WATER), 1);
    RUN_OVERWORLD_SCRIPT(call VireliaRainfall_EventScript_RewardBottle;);
    EXPECT_EQ(gSpecialVar_Result, FALSE);
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_MYSTIC_WATER), 1);
    EXPECT(!FlagGet(FLAG_VIRELIA_RAINFALL_COMPLETE));
}

static MainCallback FinishRainfallNativeBattle(u16 trainerId, u32 outcome)
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

TEST("Rainfall route pickups and lower shortcut persist without duplicate rewards")
{
    u32 item = 1;
    PARAMETRIZE { item = 1; }
    PARAMETRIZE { item = 2; }
    ResetRainfall();
    FlagSet(FLAG_VIRELIA_FIRST_GYM_COMPLETE);
    gSpecialVar_0x8004 = item;
    RUN_OVERWORLD_SCRIPT(call VireliaRainfall_EventScript_TakeRouteItem;);
    EXPECT_EQ(gSpecialVar_Result, TRUE);
    u16 itemId = item == 1 ? ITEM_REPEL : ITEM_SUPER_POTION;
    EXPECT_EQ(CountTotalItemQuantityInBag(itemId), 1);
    RUN_OVERWORLD_SCRIPT(call VireliaRainfall_EventScript_TakeRouteItem;);
    EXPECT_EQ(gSpecialVar_Result, FALSE);
    EXPECT_EQ(CountTotalItemQuantityInBag(itemId), 1);
    RUN_OVERWORLD_SCRIPT(call VireliaRainfall_EventScript_OpenShortcut;);
    ClearTempFieldEventData();
    EXPECT(FlagGet(FLAG_VIRELIA_LOWER_SHORTCUT));
}
TEST("Rainfall all rival starter and gender variants white out on loss and persist only victory")
{
    u32 choice = 0, gender = 0;
    for (u32 i = 0; i < VIRELIA_STARTER_COUNT; i++)
        for (u32 j = 0; j < 2; j++)
            PARAMETRIZE { choice = i; gender = j; }
    ResetRainfall();
    FlagSet(FLAG_VIRELIA_FIRST_GYM_COMPLETE);
    VarSet(VAR_VIRELIA_STARTER, choice);
    gSaveBlock2Ptr->playerGender = gender;
    u16 trainer = VireliaRainfall_RivalTrainer(choice);
    EXPECT_EQ(trainer, (gender == FEMALE ? TRAINER_VIRELIA_VIKTOR_RAINFALL_GRASS : TRAINER_VIRELIA_VIKTORIE_RAINFALL_GRASS) + choice);
    LoadRainfallMap(MAP_VIRELIA_RAINFALL_LOWLANDS);
    CreateRandomMonWithIVs(&gParties[B_TRAINER_PLAYER][0], GetVireliaStarterSpecies(choice), 17, 10);
    gPartiesCount[B_TRAINER_PLAYER] = 1;
    SetLastHealLocationWarp(HEAL_LOCATION_VIRELIA_SLATEFORD_POKEMON_CENTER);
    EXPECT(FinishRainfallNativeBattle(trainer, B_OUTCOME_LOST) == CB2_WhiteOut);
    RUN_OVERWORLD_SCRIPT(call VireliaRainfall_EventScript_FinishRival;);
    EXPECT(!FlagGet(FLAG_VIRELIA_RAINFALL_RIVAL_COMPLETE));
    EXPECT(!HasTrainerBeenFought(trainer));
    EXPECT(FinishRainfallNativeBattle(trainer, B_OUTCOME_WON) == CB2_ReturnToFieldContinueScriptPlayMapMusic);
    RUN_OVERWORLD_SCRIPT(call VireliaRainfall_EventScript_FinishRival;);
    EXPECT(FlagGet(FLAG_VIRELIA_RAINFALL_RIVAL_COMPLETE));
    EXPECT(HasTrainerBeenFought(trainer));
    EXPECT(FlagGet(FLAG_VIRELIA_TRAINER_RAINFALL_RIVAL));
    RunOnLoadMapScript();
    EXPECT_EQ(MapGridGetCollisionAt(15 + MAP_OFFSET, 13 + MAP_OFFSET), 0);
    EXPECT_EQ(VarGet(VAR_VIRELIA_STARTER), choice);
}

TEST("Rainfall trainers use dedicated victory bits without touching Hoenn badges")
{
    u32 i = 0;
    for (u32 j = 0; j < ARRAY_COUNT(sRainfallTrainers); j++)
        PARAMETRIZE { i = j; }
    ResetRainfall();
    u16 expectedFlag = i < 4 ? FLAG_VIRELIA_TRAINER_RAINFALL_HIKER + i : FLAG_VIRELIA_TRAINER_RAINFALL_RIVAL;
    EXPECT_EQ(VireliaFirstGym_TrainerFlag(sRainfallTrainers[i]), expectedFlag);
    EXPECT(!HasTrainerBeenFought(sRainfallTrainers[i]));
    SetTrainerFlag(sRainfallTrainers[i]);
    EXPECT(HasTrainerBeenFought(sRainfallTrainers[i]));
    EXPECT(FlagGet(expectedFlag));
    EXPECT(!FlagGet(TRAINER_FLAGS_START + sRainfallTrainers[i]));
    EXPECT_EQ(TRAINER_FLAGS_START, 0x500);
    EXPECT_EQ(FLAG_BADGE01_GET, 0x867);
}

TEST("Rainfall native Slateford gate reload follows only the existing first Gym completion flag")
{
    ResetRainfall();
    LoadRainfallMap(MAP_VIRELIA_SLATEFORD_CITY);
    EXPECT_NE(MapGridGetCollisionAt(12 + MAP_OFFSET, 2 + MAP_OFFSET), 0);
    EXPECT(!FlagGet(FLAG_VIRELIA_RAINFALL_STARTED));
    FlagSet(FLAG_VIRELIA_FIRST_GYM_COMPLETE);
    RunOnLoadMapScript();
    EXPECT_EQ(MapGridGetCollisionAt(12 + MAP_OFFSET, 2 + MAP_OFFSET), 0);
    EXPECT(FlagGet(FLAG_VIRELIA_RAINFALL_STARTED));
    LoadRainfallMap(MAP_VIRELIA_RAINFALL_LOWLANDS);
    EXPECT_NE(MapGridGetCollisionAt(15 + MAP_OFFSET, 13 + MAP_OFFSET), 0);
}

TEST("Rainfall native wooden bridges keep avatar and follower above the deck on both paths")
{
    u32 lower = FALSE;
    PARAMETRIZE { lower = FALSE; }
    PARAMETRIZE { lower = TRUE; }
    ResetRainfall();
    LoadRainfallMap(lower ? MAP_VIRELIA_RAINFALL_LOWER : MAP_VIRELIA_RAINFALL_UPPER);
    for (u32 i = 0; i < 2; i++)
    {
        s32 x = lower ? 21 : 8 + i;
        s32 y = lower ? 20 + i : 14;
        EXPECT_EQ(MapGridGetMetatileLayerTypeAt(x + MAP_OFFSET, y + MAP_OFFSET), METATILE_LAYER_TYPE_COVERED);
        EXPECT_EQ(MapGridGetCollisionAt(x + MAP_OFFSET, y + MAP_OFFSET), 0);
        EXPECT_EQ(MapGridGetMetatileBehaviorAt(x + MAP_OFFSET, y + MAP_OFFSET), MB_NORMAL);
        EXPECT_EQ(MapGridGetElevationAt(x + MAP_OFFSET, y + MAP_OFFSET), lower ? 3 : 5);
    }
}

TEST("Rainfall native Continue restores the whole north exit over an older cached Slateford view")
{
    u32 badge = FALSE;
    PARAMETRIZE { badge = FALSE; }
    PARAMETRIZE { badge = TRUE; }
    ResetRainfall();
    if (badge) FlagSet(FLAG_VIRELIA_FIRST_GYM_COMPLETE);
    LoadRainfallMap(MAP_VIRELIA_SLATEFORD_CITY);
    // Continue restores mapView after loading its layout. Reproduce the old
    // impassable tree entries, then exercise the actual on-load callback.
    for (u32 y = 0; y <= 3; y++)
        for (u32 x = 11; x <= 13; x++)
            MapGridSetMetatileIdAt(x + MAP_OFFSET, y + MAP_OFFSET, 0x1E4 | MAPGRID_IMPASSABLE);
    EXPECT_NE(MapGridGetCollisionAt(12 + MAP_OFFSET, 1 + MAP_OFFSET), 0);
    RunOnLoadMapScript();
    EXPECT_EQ(MapGridGetMetatileIdAt(12 + MAP_OFFSET, 1 + MAP_OFFSET), 0x0A7);
    EXPECT_EQ(MapGridGetCollisionAt(12 + MAP_OFFSET, 1 + MAP_OFFSET), 0);
    EXPECT_EQ(MapGridGetMetatileIdAt(12 + MAP_OFFSET, 3 + MAP_OFFSET), 0x001);
    EXPECT_EQ(MapGridGetCollisionAt(12 + MAP_OFFSET, 3 + MAP_OFFSET), 0);
    EXPECT_EQ(MapGridGetCollisionAt(12 + MAP_OFFSET, 2 + MAP_OFFSET) == 0, badge);
    EXPECT_NE(MapGridGetCollisionAt(12 + MAP_OFFSET, 0 + MAP_OFFSET), 0);
    for (u32 x = 11; x <= 13; x += 2)
        for (u32 y = 0; y <= 2; y++)
            EXPECT_NE(MapGridGetCollisionAt(x + MAP_OFFSET, y + MAP_OFFSET), 0);
}

extern const u8 Virelia_Seabreak_PokemonCenter_EventScript_Nurse[];
extern const u8 Virelia_Seabreak_PokemonCenter_EventScript_Clerk[];

TEST("Rainfall Seabreak native Center installs a safe whiteout nurse and accessible shop")
{
    ResetRainfall();
    LoadRainfallMap(MAP_VIRELIA_SEABREAK_POKEMON_CENTER);
    RunOnTransitionMapScript();
    EXPECT_EQ(GetHealLocationIndexByWarpData(&gSaveBlock1Ptr->lastHealLocation), HEAL_LOCATION_VIRELIA_SEABREAK_POKEMON_CENTER);
    struct WarpData respawn = {0};
    SetWhiteoutRespawnWarpAndHealerNPC(&respawn);
    EXPECT_EQ(respawn.mapGroup, MAP_GROUP(MAP_VIRELIA_SEABREAK_POKEMON_CENTER));
    EXPECT_EQ(respawn.mapNum, MAP_NUM(MAP_VIRELIA_SEABREAK_POKEMON_CENTER));
    EXPECT_EQ(respawn.x, 7);
    EXPECT_EQ(respawn.y, 4);
    EXPECT_EQ(gSpecialVar_LastTalked, LOCALID_VIRELIA_SEABREAK_NURSE);
    for (u32 role = 0; role < 2; role++)
    {
        const u8 *script = role ? Virelia_Seabreak_PokemonCenter_EventScript_Clerk : Virelia_Seabreak_PokemonCenter_EventScript_Nurse;
        const struct ObjectEventTemplate *service = NULL;
        for (u32 i = 0; i < gMapHeader.events->objectEventCount; i++)
            if (gMapHeader.events->objectEvents[i].script == script)
                service = &gMapHeader.events->objectEvents[i];
        EXPECT_NE(service, NULL);
        EXPECT_EQ(MapGridGetCollisionAt(service->x + MAP_OFFSET, service->y + 2 + MAP_OFFSET), 0);
        EXPECT_EQ(MapGridGetMetatileBehaviorAt(service->x + MAP_OFFSET, service->y + 1 + MAP_OFFSET), MB_COUNTER);
    }
    CreateRandomMonWithIVs(&gParties[B_TRAINER_PLAYER][0], SPECIES_NATU_VIRELIA, 18, 10);
    gPartiesCount[B_TRAINER_PLAYER] = 1;
    u32 zero = 0, poison = STATUS1_POISON;
    SetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_HP, &zero);
    SetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_STATUS, &poison);
    SetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_PP1, &zero);
    RUN_OVERWORLD_SCRIPT(special HealPlayerParty;);
    EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_HP), GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_MAX_HP));
    EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_STATUS), 0);
    EXPECT_GT(GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_PP1), 0);
    ResetPokemonStorageSystem();
    struct Pokemon original, withdrawn;
    CreateRandomMonWithIVs(&original, SPECIES_WOOPER, 16, 10);
    VarSet(VAR_PC_BOX_TO_SEND_MON, 0);
    EXPECT_EQ(CopyMonToPC(&original), MON_GIVEN_TO_PC);
    BoxMonAtToMon(0, 0, &withdrawn);
    EXPECT_EQ(GetMonData(&withdrawn, MON_DATA_SPECIES), SPECIES_WOOPER);
    EXPECT_EQ(GetMonData(&withdrawn, MON_DATA_SANITY_IS_BAD_EGG), FALSE);
}

TEST("Rainfall FLASH restores intermediate story bottle stages trainers and safe Center checkpoint")
{
    u32 checkpoint = 0;
    for (u32 i = 0; i < 6; i++)
        PARAMETRIZE { checkpoint = i; }
    ResetRainfall();
    FlagSet(FLAG_VIRELIA_FIRST_GYM_COMPLETE);
    VireliaRainfall_BeginChapter();
    VarSet(VAR_VIRELIA_BOTTLE_STAGE, min(checkpoint, 4));
    if (checkpoint >= 1)
    {
        SetTrainerFlag(TRAINER_VIRELIA_VIKTOR_RAINFALL_WATER);
        FlagSet(FLAG_VIRELIA_RAINFALL_RIVAL_COMPLETE);
    }
    if (checkpoint >= 2)
    {
        VireliaRainfall_HarborReport();
        VireliaRainfall_InspectDamage();
        VireliaRainfall_OpenShortcut();
    }
    if (checkpoint >= 3)
    {
        for (u32 i = 1; i <= 3; i++)
        {
            gSpecialVar_0x8004 = i;
            VireliaRainfall_SignalSwitch();
        }
    }
    if (checkpoint >= 4) VireliaRainfall_ReadManifest();
    if (checkpoint >= 5)
    {
        VireliaRainfall_CompleteChapter();
        VireliaRainfall_RewardBottle();
    }
    LoadRainfallMap(MAP_VIRELIA_SEABREAK_POKEMON_CENTER);
    RunOnTransitionMapScript();
    SetMoney(&gSaveBlock1Ptr->money, 2450);
    EXPECT(AddBagItem(ITEM_GREAT_BALL, 2));
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
        for (u32 flag = FLAG_VIRELIA_RAINFALL_STARTED; flag <= FLAG_VIRELIA_TRAINER_RAINFALL_RIVAL; flag++) FlagClear(flag);
        VarSet(VAR_VIRELIA_BOTTLE_STAGE, 0);
        for (u32 i = 0; i < ARRAY_COUNT(sRainfallTrainers); i++) ClearTrainerFlag(sRainfallTrainers[i]);
        ClearBag();
        SetMoney(&gSaveBlock1Ptr->money, 0);
        memset(&gSaveBlock1Ptr->lastHealLocation, 0, sizeof(gSaveBlock1Ptr->lastHealLocation));
        SetWarpDestination(MAP_GROUP(MAP_VIRELIA_MAPLEBROOK_TOWN),
                           MAP_NUM(MAP_VIRELIA_MAPLEBROOK_TOWN), WARP_ID_NONE, 14, 27);
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
    EXPECT_EQ(gSaveBlock1Ptr->location.mapGroup, MAP_GROUP(MAP_VIRELIA_SEABREAK_POKEMON_CENTER));
    EXPECT_EQ(gSaveBlock1Ptr->location.mapNum, MAP_NUM(MAP_VIRELIA_SEABREAK_POKEMON_CENTER));
    EXPECT_EQ(GetHealLocationIndexByWarpData(&gSaveBlock1Ptr->lastHealLocation), HEAL_LOCATION_VIRELIA_SEABREAK_POKEMON_CENTER);
    EXPECT(FlagGet(FLAG_VIRELIA_FIRST_GYM_COMPLETE));
    EXPECT(FlagGet(FLAG_VIRELIA_RAINFALL_STARTED));
    EXPECT_EQ(FlagGet(FLAG_VIRELIA_RAINFALL_RIVAL_COMPLETE), checkpoint >= 1);
    EXPECT_EQ(FlagGet(FLAG_VIRELIA_LIGHTHOUSE_DAMAGE), checkpoint >= 2);
    EXPECT_EQ(FlagGet(FLAG_VIRELIA_LOWER_SHORTCUT), checkpoint >= 2);
    EXPECT_EQ(FlagGet(FLAG_VIRELIA_LIGHTHOUSE_REPAIRED), checkpoint >= 3);
    EXPECT_EQ(FlagGet(FLAG_VIRELIA_SHIPPING_RECORD), checkpoint >= 4);
    EXPECT_EQ(FlagGet(FLAG_VIRELIA_RAINFALL_COMPLETE), checkpoint >= 5);
    EXPECT_EQ(VarGet(VAR_VIRELIA_BOTTLE_STAGE), min(checkpoint, 4));
    EXPECT_EQ(GetMoney(&gSaveBlock1Ptr->money), 2450);
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_GREAT_BALL), 2);
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_MYSTIC_WATER), checkpoint >= 5);
    if (checkpoint >= 5)
    {
        VireliaRainfall_RewardBottle();
        EXPECT_EQ(gSpecialVar_Result, FALSE);
        EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_MYSTIC_WATER), 1);
    }
}

#endif
