#include "global.h"
#include "agb_flash.h"
#include "battle.h"
#include "battle_setup.h"
#include "data.h"
#include "event_data.h"
#include "fieldmap.h"
#include "metatile_behavior.h"
#include "script.h"
#include "heal_location.h"
#include "item.h"
#include "main.h"
#include "task.h"
#include "new_game.h"
#include "overworld.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "save.h"
#include "script_pokemon_util.h"
#include "sound.h"
#include "string_util.h"
#include "text.h"
#include "trainer_pools.h"
#include "virelia_prologue.h"
#include "virelia_first_gym.h"
#include "test/test.h"
#include "constants/abilities.h"
#include "constants/battle_ai.h"
#include "constants/heal_locations.h"
#include "constants/items.h"
#include "constants/moves.h"
#include "constants/opponents.h"
#include "constants/virelia_prologue.h"
#include "constants/virelia_first_gym.h"

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

TEST("Virelia actual city map load keeps chapter entrances locked until their independent prerequisites")
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
    InitMap();
    RunOnLoadMapScript();
    u32 gymBehavior = MapGridGetMetatileBehaviorAt(22 + MAP_OFFSET, 9 + MAP_OFFSET);
    u32 quarryBehavior = MapGridGetMetatileBehaviorAt(25 + MAP_OFFSET, 16 + MAP_OFFSET);
    EXPECT_EQ(MetatileBehavior_IsWarpDoor(gymBehavior), checkpoint >= 2);
    EXPECT_NE(MapGridGetCollisionAt(22 + MAP_OFFSET, 9 + MAP_OFFSET), 0);
    EXPECT_EQ(MetatileBehavior_IsNonAnimDoor(quarryBehavior), checkpoint >= 1);
    EXPECT_EQ(MapGridGetCollisionAt(25 + MAP_OFFSET, 16 + MAP_OFFSET) != 0, checkpoint < 1);
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
    u32 x = puzzle ? 6 : 11, y = 6;
    if (solved) FlagSet(flag);
    SetWarpDestination(MAP_GROUP(map), MAP_NUM(map), WARP_ID_NONE, puzzle ? 5 : 3, puzzle ? 18 : 19);
    WarpIntoMap();
    InitMap();
    VarSet(var, 1);
    RunOnTransitionMapScript();
    RunOnLoadMapScript();
    EXPECT_EQ(VarGet(var), 0);
    EXPECT_EQ(FlagGet(flag), solved);
    EXPECT_EQ(MapGridGetCollisionAt(x + MAP_OFFSET, y + MAP_OFFSET) != 0, !solved);
    if (solved) EXPECT_EQ(MapGridGetMetatileIdAt(x + MAP_OFFSET, y + MAP_OFFSET), 0x201);
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
    SetLastHealLocationWarp(HEAL_LOCATION_VIRELIA_SLATEFORD_POKEMON_CENTER);
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

TEST("Virelia chapter puzzle and reward flags survive native FLASH save and Continue")
{
    CompleteOldPrologue(VIRELIA_STARTER_GRASS);
    VireliaFirstGym_BeginChapter();
    SolveQuarry();
    SolveGym();
    VireliaFirstGym_AwardTM();
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
    for (u32 i = 0; i < ARRAY_COUNT(savedFlags); i++) EXPECT(FlagGet(savedFlags[i]));
    for (u32 i = 0; i < ARRAY_COUNT(sChapterTrainers); i++) EXPECT(HasTrainerBeenFought(sChapterTrainers[i]));
    EXPECT_EQ(VarGet(VAR_VIRELIA_PROLOGUE_STATE), VIRELIA_STATE_COMPLETE);
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_TM_ROCK_TOMB), 1);
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_SUPER_POTION), 1);
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_HARD_STONE), 1);
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
