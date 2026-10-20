#include "global.h"
#include "agb_flash.h"
#include "battle.h"
#include "battle_setup.h"
#include "event_data.h"
#include "heal_location.h"
#include "item.h"
#include "main.h"
#include "new_game.h"
#include "overworld.h"
#include "pokedex.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "save.h"
#include "string_util.h"
#include "task.h"
#include "virelia_prologue.h"
#include "test/test.h"
#include "constants/heal_locations.h"
#include "constants/items.h"
#include "constants/oraculum_duo_test.h"
#include "constants/opponents.h"
#include "constants/virelia_prologue.h"

#if !IS_FRLG && P_VIRELIA_STARTERS

static void InitPrologueTest(void)
{
    InitEventData();
    ZeroPlayerPartyMons();
    ResetPokemonStorageSystem();
    ClearBag();
    SetBagItemsPointers();
    SetTrainerId(0x1234ABCD, gSaveBlock2Ptr->playerTrainerId);
    StringCopy(gSaveBlock2Ptr->playerName, COMPOUND_STRING("Virelia"));
    gSaveBlock2Ptr->playerGender = FEMALE;
    InitVireliaPrologueNewGame();
}

static void ReceivePrologueStarter(u32 choice)
{
    VireliaPrologue_MeetMom();
    gSpecialVar_0x8004 = choice;
    VireliaPrologue_GiveStarter();
    EXPECT_EQ(gSpecialVar_Result, TRUE);
}

static void CompletePrologueRival(u32 outcome)
{
    gBattleOutcome = outcome;
    VireliaPrologue_CompleteRival();
    EXPECT_EQ(gSpecialVar_Result, TRUE);
}

TEST("Virelia prologue starts at home without prefilled Pokemon or development gifts")
{
    InitPrologueTest();
    EXPECT_EQ(gSaveBlock1Ptr->location.mapGroup, MAP_GROUP(MAP_VIRELIA_MAPLEBROOK_PLAYERS_HOUSE));
    EXPECT_EQ(gSaveBlock1Ptr->location.mapNum, MAP_NUM(MAP_VIRELIA_MAPLEBROOK_PLAYERS_HOUSE));
    EXPECT_EQ(gSaveBlock1Ptr->pos.x, 5);
    EXPECT_EQ(gSaveBlock1Ptr->pos.y, 6);
    EXPECT_EQ(gPartiesCount[B_TRAINER_PLAYER], 0);
    EXPECT_EQ(VarGet(VAR_ORACULUM_DUO_TEST_STATE), 0);
    EXPECT_EQ(VarGet(VAR_VIRELIA_PROLOGUE_STATE), VIRELIA_STATE_HOME);
    EXPECT_EQ(VarGet(VAR_VIRELIA_STARTER), VIRELIA_STARTER_NONE);
    EXPECT(!FlagGet(FLAG_VIRELIA_MOM_MET));
    EXPECT(!FlagGet(FLAG_VIRELIA_STARTER_RECEIVED));
    EXPECT(!FlagGet(FLAG_VIRELIA_RIVAL_COMPLETE));
    EXPECT(!FlagGet(FLAG_VIRELIA_ROCKET_DEFEATED));
    EXPECT(!FlagGet(FLAG_VIRELIA_SUPPLIES_RECEIVED));
    EXPECT(!FlagGet(FLAG_VIRELIA_PROLOGUE_COMPLETE));
    EXPECT(FlagGet(FLAG_VIRELIA_HIDE_DEV_GUIDE));
    EXPECT(FlagGet(FLAG_VIRELIA_HIDE_POLICEMAN));
    EXPECT(!FlagGet(FLAG_VIRELIA_HIDE_STORY_ACTORS));
    EXPECT(!FlagGet(FLAG_SYS_POKEMON_GET));
    EXPECT_EQ(GetHealLocationIndexByWarpData(&gSaveBlock1Ptr->lastHealLocation),
              HEAL_LOCATION_VIRELIA_MAPLEBROOK_PLAYERS_HOUSE);
    EXPECT(IsLastHealLocationPlayerHouse());
    struct WarpData respawn;
    SetWhiteoutRespawnWarpAndHealerNPC(&respawn);
    EXPECT_EQ(respawn.mapGroup, MAP_GROUP(MAP_VIRELIA_MAPLEBROOK_PLAYERS_HOUSE));
    EXPECT_EQ(respawn.mapNum, MAP_NUM(MAP_VIRELIA_MAPLEBROOK_PLAYERS_HOUSE));
    EXPECT_EQ(respawn.x, 2);
    EXPECT_EQ(respawn.y, 7);
    EXPECT_EQ(gSpecialVar_LastTalked, LOCALID_VIRELIA_MOM);
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_POKE_BALL), 0);
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_POTION), 0);
    EXPECT_EQ(GetTrainerId(gSaveBlock2Ptr->playerTrainerId), 0x1234ABCD);
    EXPECT_EQ(gSaveBlock2Ptr->playerGender, FEMALE);
}

#if VIRELIA_PROLOGUE
TEST("Virelia normal NEW GAME uses the story initializer and keeps selected player identity")
{
    StringCopy(gSaveBlock2Ptr->playerName, COMPOUND_STRING("Virelia"));
    gSaveBlock2Ptr->playerGender = FEMALE;
    gSaveFileStatus = SAVE_STATUS_OK;
    NewGameInitData();
    EXPECT_EQ(StringCompare(gSaveBlock2Ptr->playerName, COMPOUND_STRING("Virelia")), 0);
    EXPECT_EQ(gSaveBlock2Ptr->playerGender, FEMALE);
    EXPECT_EQ(gSaveBlock1Ptr->location.mapGroup, MAP_GROUP(MAP_VIRELIA_MAPLEBROOK_PLAYERS_HOUSE));
    EXPECT_EQ(gSaveBlock1Ptr->location.mapNum, MAP_NUM(MAP_VIRELIA_MAPLEBROOK_PLAYERS_HOUSE));
    EXPECT_EQ(gPartiesCount[B_TRAINER_PLAYER], 0);
    EXPECT_EQ(VarGet(VAR_ORACULUM_DUO_TEST_STATE), 0);
    EXPECT_EQ(VarGet(VAR_LITTLEROOT_INTRO_STATE), 7);
    EXPECT_EQ(VarGet(VAR_BIRCH_LAB_STATE), 5);
    EXPECT_EQ(VarGet(VAR_VIRELIA_PROLOGUE_STATE), VIRELIA_STATE_HOME);
    EXPECT(FlagGet(FLAG_VIRELIA_HIDE_DEV_GUIDE));
    EXPECT(!FlagGet(FLAG_VIRELIA_STARTER_RECEIVED));
}
#endif

TEST("Virelia starters are unique native local-owned level 5 gifts after speaking to Mom")
{
    u32 choice = VIRELIA_STARTER_GRASS;
    for (u32 i = 0; i < VIRELIA_STARTER_COUNT; i++)
        PARAMETRIZE { choice = i; }

    InitPrologueTest();
    gSpecialVar_0x8004 = choice;
    VireliaPrologue_GiveStarter();
    EXPECT_EQ(gSpecialVar_Result, FALSE);
    EXPECT_EQ(gPartiesCount[B_TRAINER_PLAYER], 0);
    ReceivePrologueStarter(choice);
    struct Pokemon original = gParties[B_TRAINER_PLAYER][0];
    u32 species = GetVireliaStarterSpecies(choice);
    EXPECT_EQ(gPartiesCount[B_TRAINER_PLAYER], 1);
    EXPECT_EQ(GetMonData(&original, MON_DATA_SPECIES), species);
    EXPECT_EQ(GetMonData(&original, MON_DATA_LEVEL), VIRELIA_STARTER_LEVEL);
    EXPECT_EQ(GetMonData(&original, MON_DATA_OT_ID), 0x1234ABCD);
    EXPECT_EQ(GetMonData(&original, MON_DATA_OT_GENDER), FEMALE);
    EXPECT_EQ(GetMonData(&original, MON_DATA_SANITY_IS_BAD_EGG), FALSE);
    EXPECT_EQ(GetMonData(&original, MON_DATA_IS_EGG), FALSE);
    EXPECT_EQ(IsTradedMon(&original), FALSE);
    EXPECT_EQ(GetMonData(&original, MON_DATA_HP), GetMonData(&original, MON_DATA_MAX_HP));
    EXPECT_EQ(VarGet(VAR_VIRELIA_STARTER), choice);
    EXPECT_EQ(VarGet(VAR_VIRELIA_PROLOGUE_STATE), VIRELIA_STATE_STARTER);
    EXPECT(FlagGet(FLAG_SYS_POKEMON_GET));
    EXPECT_EQ(GetSetPokedexFlag(SpeciesToNationalPokedexNum(species), FLAG_GET_CAUGHT), TRUE);
    u8 otName[PLAYER_NAME_LENGTH + 1];
    GetMonData(&original, MON_DATA_OT_NAME, otName);
    EXPECT_EQ(StringCompare(otName, gSaveBlock2Ptr->playerName), 0);
    for (u32 slot = 0; slot < MAX_MON_MOVES; slot++)
    {
        enum Move move = GetMonData(&original, MON_DATA_MOVE1 + slot);
        if (move != MOVE_NONE)
            EXPECT_EQ(GetMonData(&original, MON_DATA_PP1 + slot), gMovesInfo[move].pp);
    }
    gSpecialVar_0x8004 = (choice + 1) % VIRELIA_STARTER_COUNT;
    VireliaPrologue_GiveStarter();
    EXPECT_EQ(gSpecialVar_Result, FALSE);
    EXPECT_EQ(gPartiesCount[B_TRAINER_PLAYER], 1);
    EXPECT_EQ(memcmp(&original, &gParties[B_TRAINER_PLAYER][0], sizeof(original)), 0);
    EXPECT_EQ(VarGet(VAR_VIRELIA_STARTER), choice);
}

TEST("Virelia invalid starter choices and nonempty parties cannot consume the story gift")
{
    u32 choice = VIRELIA_STARTER_NONE, partyCount = 0;
    PARAMETRIZE { choice = VIRELIA_STARTER_NONE; partyCount = 0; }
    PARAMETRIZE { choice = VIRELIA_STARTER_COUNT; partyCount = 0; }
    PARAMETRIZE { choice = VIRELIA_STARTER_GRASS; partyCount = 1; }
    PARAMETRIZE { choice = VIRELIA_STARTER_WATER; partyCount = PARTY_SIZE; }
    InitPrologueTest();
    VireliaPrologue_MeetMom();
    for (u32 i = 0; i < partyCount; i++)
        CreateRandomMonWithIVs(&gParties[B_TRAINER_PLAYER][i], SPECIES_WOBBUFFET, 5, 10);
    gPartiesCount[B_TRAINER_PLAYER] = partyCount;
    gSpecialVar_0x8004 = choice;
    VireliaPrologue_GiveStarter();
    EXPECT_EQ(gSpecialVar_Result, FALSE);
    EXPECT_EQ(gPartiesCount[B_TRAINER_PLAYER], partyCount);
    EXPECT(!FlagGet(FLAG_VIRELIA_STARTER_RECEIVED));
    EXPECT_EQ(VarGet(VAR_VIRELIA_STARTER), VIRELIA_STARTER_NONE);
}

TEST("Virelia rival has the specified opposing starter for all three choices")
{
    EXPECT_EQ(GetVireliaRivalSpecies(VIRELIA_STARTER_GRASS), SPECIES_EKANS_VIRELIA);
    EXPECT_EQ(GetVireliaRivalSpecies(VIRELIA_STARTER_WATER), SPECIES_POOCHYENA_VIRELIA);
    EXPECT_EQ(GetVireliaRivalSpecies(VIRELIA_STARTER_FIRE), SPECIES_SPINARAK_VIRELIA);
    EXPECT_EQ(GetVireliaRivalSpecies(VIRELIA_STARTER_NONE), SPECIES_NONE);
    EXPECT_EQ(GetVireliaRivalSpecies(VIRELIA_STARTER_COUNT), SPECIES_NONE);
    EXPECT_EQ(VIRELIA_RIVAL_LEVEL, 5);
}

TEST("Virelia introductory rival win and loss both heal the team and advance exactly once")
{
    u32 outcome = B_OUTCOME_WON;
    PARAMETRIZE { outcome = B_OUTCOME_WON; }
    PARAMETRIZE { outcome = B_OUTCOME_LOST; }
    InitPrologueTest();
    ReceivePrologueStarter(VIRELIA_STARTER_GRASS);
    u32 hp = 0, status = STATUS1_POISON, pp = 0;
    SetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_HP, &hp);
    SetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_STATUS, &status);
    SetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_PP1, &pp);
    CompletePrologueRival(outcome);
    EXPECT(FlagGet(FLAG_VIRELIA_RIVAL_COMPLETE));
    EXPECT_EQ(VarGet(VAR_VIRELIA_RIVAL_RESULT), outcome);
    EXPECT_EQ(VarGet(VAR_VIRELIA_PROLOGUE_STATE), VIRELIA_STATE_RIVAL);
    EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_HP),
              GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_MAX_HP));
    EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_STATUS), 0);
    enum Move move = GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_MOVE1);
    EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_PP1), gMovesInfo[move].pp);
    gBattleOutcome = B_OUTCOME_RAN;
    VireliaPrologue_CompleteRival();
    EXPECT_EQ(gSpecialVar_Result, TRUE);
    EXPECT_EQ(VarGet(VAR_VIRELIA_RIVAL_RESULT), outcome);
    EXPECT_EQ(gPartiesCount[B_TRAINER_PLAYER], 1);
}

TEST("Virelia heal-only rival battles keep normal input while the original tutorial stays enabled")
{
    u32 flags = 0, earlyRival = TRUE;
    bool32 expectedTutorial = FALSE;
    PARAMETRIZE { flags = 0; earlyRival = TRUE; expectedTutorial = FALSE; }
    PARAMETRIZE { flags = RIVAL_BATTLE_HEAL_AFTER; earlyRival = TRUE; expectedTutorial = FALSE; }
    PARAMETRIZE { flags = RIVAL_BATTLE_TUTORIAL; earlyRival = TRUE; expectedTutorial = TRUE; }
    PARAMETRIZE { flags = RIVAL_BATTLE_TUTORIAL; earlyRival = FALSE; expectedTutorial = FALSE; }
    InitPrologueTest();
    ReceivePrologueStarter(VIRELIA_STARTER_GRASS);
    TrainerBattleParameter originalParams = gTrainerBattleParameter;
    MainCallback originalCallback = gMain.savedCallback;
    InitTrainerBattleParameter();
    TRAINER_BATTLE_PARAM.earlyRival = earlyRival;
    TRAINER_BATTLE_PARAM.rivalBattleFlags = flags;
    TRAINER_BATTLE_PARAM.opponentA = TRAINER_BRENDAN_ROUTE_103_TREECKO;
    gNoOfApproachingTrainers = 0;
    BattleSetup_StartTrainerBattle();
    u32 battleFlags = gBattleTypeFlags;
    // Only inspect native setup, without running its asynchronous transition.
    ResetTasks();
    gMain.savedCallback = originalCallback;
    gTrainerBattleParameter = originalParams;
    EXPECT(battleFlags & BATTLE_TYPE_TRAINER);
    EXPECT_EQ(!!(battleFlags & BATTLE_TYPE_FIRST_BATTLE), expectedTutorial);
}

TEST("Virelia supplies require the rival and are an idempotent complete bundle")
{
    InitPrologueTest();
    VireliaPrologue_GrantBasics();
    EXPECT_EQ(gSpecialVar_Result, FALSE);
    EXPECT(!FlagGet(FLAG_VIRELIA_SUPPLIES_RECEIVED));
    ReceivePrologueStarter(VIRELIA_STARTER_WATER);
    CompletePrologueRival(B_OUTCOME_WON);
    VireliaPrologue_GrantBasics();
    EXPECT_EQ(gSpecialVar_Result, TRUE);
    EXPECT(FlagGet(FLAG_VIRELIA_SUPPLIES_RECEIVED));
    EXPECT(FlagGet(FLAG_SYS_POKEDEX_GET));
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_POKE_BALL), VIRELIA_STARTING_POKE_BALLS);
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_POTION), VIRELIA_STARTING_POTIONS);
    VireliaPrologue_GrantBasics();
    EXPECT_EQ(gSpecialVar_Result, TRUE);
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_POKE_BALL), VIRELIA_STARTING_POKE_BALLS);
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_POTION), VIRELIA_STARTING_POTIONS);
}

TEST("Virelia a full item pocket cannot partially grant supplies and allows retry")
{
    InitPrologueTest();
    ReceivePrologueStarter(VIRELIA_STARTER_FIRE);
    CompletePrologueRival(B_OUTCOME_LOST);
    // Fill every slot in the ordinary item pocket while leaving Balls empty.
    EXPECT(AddBagItem(ITEM_POTION, MAX_BAG_ITEM_CAPACITY));
    for (u32 item = ITEM_NONE + 1; item < ITEMS_COUNT; item++)
        if (GetItemPocket(item) == POCKET_ITEMS && item != ITEM_POTION)
            AddBagItem(item, MAX_BAG_ITEM_CAPACITY);
    EXPECT(!CheckBagHasSpace(ITEM_POTION, VIRELIA_STARTING_POTIONS));
    VireliaPrologue_GrantBasics();
    EXPECT_EQ(gSpecialVar_Result, FALSE);
    EXPECT(!FlagGet(FLAG_VIRELIA_SUPPLIES_RECEIVED));
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_POKE_BALL), 0);
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_POTION), MAX_BAG_ITEM_CAPACITY);
    ClearBag();
    VireliaPrologue_GrantBasics();
    EXPECT_EQ(gSpecialVar_Result, TRUE);
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_POKE_BALL), VIRELIA_STARTING_POKE_BALLS);
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_POTION), VIRELIA_STARTING_POTIONS);
}

TEST("Virelia Rocket and finale require the real progression prerequisites")
{
    InitPrologueTest();
    gBattleOutcome = B_OUTCOME_WON;
    VireliaPrologue_FinishRocket();
    EXPECT_EQ(gSpecialVar_Result, FALSE);
    VireliaPrologue_Complete();
    EXPECT_EQ(gSpecialVar_Result, FALSE);
    ReceivePrologueStarter(VIRELIA_STARTER_GRASS);
    CompletePrologueRival(B_OUTCOME_WON);
    VireliaPrologue_FinishRocket();
    EXPECT_EQ(gSpecialVar_Result, FALSE);
    VireliaPrologue_GrantBasics();
    gBattleOutcome = B_OUTCOME_LOST;
    VireliaPrologue_FinishRocket();
    EXPECT_EQ(gSpecialVar_Result, FALSE);
    EXPECT(!FlagGet(FLAG_VIRELIA_ROCKET_DEFEATED));
    gBattleOutcome = B_OUTCOME_WON;
    VireliaPrologue_FinishRocket();
    EXPECT_EQ(gSpecialVar_Result, TRUE);
    EXPECT(FlagGet(FLAG_VIRELIA_ROCKET_DEFEATED));
    VireliaPrologue_Complete();
    EXPECT_EQ(gSpecialVar_Result, TRUE);
    EXPECT(FlagGet(FLAG_VIRELIA_PROLOGUE_COMPLETE));
    EXPECT_EQ(VarGet(VAR_VIRELIA_PROLOGUE_STATE), VIRELIA_STATE_COMPLETE);
    VireliaPrologue_MeetMom();
    VireliaPrologue_CompleteRival();
    VireliaPrologue_FinishRocket();
    EXPECT_EQ(VarGet(VAR_VIRELIA_PROLOGUE_STATE), VIRELIA_STATE_COMPLETE);
}

TEST("Virelia prologue identity starter progress and rewards survive native flash save and load")
{
    u32 checkpoint = 0;
    PARAMETRIZE { checkpoint = 0; } // At home, before Mom or any gifts.
    PARAMETRIZE { checkpoint = 1; } // Starter chosen, rival not fought.
    PARAMETRIZE { checkpoint = 2; } // Rival lost, party healed, supplies obtained.
    PARAMETRIZE { checkpoint = 3; } // Rocket/finale, optional quest and trainers done.
    InitPrologueTest();
    if (checkpoint >= 1)
        ReceivePrologueStarter(VIRELIA_STARTER_WATER);
    if (checkpoint >= 2)
    {
        CompletePrologueRival(B_OUTCOME_LOST);
        VireliaPrologue_GrantBasics();
    }
    if (checkpoint >= 3)
    {
        gBattleOutcome = B_OUTCOME_WON;
        VireliaPrologue_FinishRocket();
        VireliaPrologue_Complete();
        FlagSet(FLAG_VIRELIA_SIDEQUEST_STARTED);
        FlagSet(FLAG_VIRELIA_SIDEQUEST_RESOLVED);
        FlagSet(FLAG_VIRELIA_SIDEQUEST_REWARDED);
        FlagSet(FLAG_ITEM_VIRELIA_ROUTE01_POTION);
        SetTrainerFlag(TRAINER_VIRELIA_ELA);
        SetTrainerFlag(TRAINER_VIRELIA_OTA);
    }
    ClearTempFieldEventData();
    struct Pokemon original = gParties[B_TRAINER_PLAYER][0];
    u32 originalPartyCount = gPartiesCount[B_TRAINER_PLAYER];
    u32 originalStarter = VarGet(VAR_VIRELIA_STARTER);
    u32 originalRivalResult = VarGet(VAR_VIRELIA_RIVAL_RESULT);
    u32 originalState = VarGet(VAR_VIRELIA_PROLOGUE_STATE);
    bool8 originalFlags[FLAG_VIRELIA_PROFESSOR_MET - FLAG_VIRELIA_MOM_MET + 1];
    for (u32 i = 0; i < ARRAY_COUNT(originalFlags); i++)
        originalFlags[i] = FlagGet(FLAG_VIRELIA_MOM_MET + i);
    u32 originalBalls = CountTotalItemQuantityInBag(ITEM_POKE_BALL);
    u32 originalPotions = CountTotalItemQuantityInBag(ITEM_POTION);

    // Borrow Timer 3 rather than the test runner's Timer 2 watchdog, restoring
    // all interrupt state before an assertion can exit nonlocally.
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
        ZeroPlayerPartyMons();
        InitEventData();
        ClearBag();
        StringCopy(gSaveBlock2Ptr->playerName, COMPOUND_STRING("Other"));
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
    EXPECT_EQ(StringCompare(gSaveBlock2Ptr->playerName, COMPOUND_STRING("Virelia")), 0);
    EXPECT_EQ(GetTrainerId(gSaveBlock2Ptr->playerTrainerId), 0x1234ABCD);
    EXPECT_EQ(gPartiesCount[B_TRAINER_PLAYER], originalPartyCount);
    EXPECT_EQ(memcmp(&original, &gParties[B_TRAINER_PLAYER][0], sizeof(original)), 0);
    EXPECT_EQ(VarGet(VAR_VIRELIA_STARTER), originalStarter);
    EXPECT_EQ(VarGet(VAR_VIRELIA_RIVAL_RESULT), originalRivalResult);
    EXPECT_EQ(VarGet(VAR_VIRELIA_PROLOGUE_STATE), originalState);
    for (u32 i = 0; i < ARRAY_COUNT(originalFlags); i++)
        EXPECT_EQ(FlagGet(FLAG_VIRELIA_MOM_MET + i), originalFlags[i]);
    EXPECT_EQ(FlagGet(FLAG_ITEM_VIRELIA_ROUTE01_POTION), checkpoint >= 3);
    EXPECT_EQ(HasTrainerBeenFought(TRAINER_VIRELIA_ELA), checkpoint >= 3);
    EXPECT_EQ(HasTrainerBeenFought(TRAINER_VIRELIA_OTA), checkpoint >= 3);
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_POKE_BALL), originalBalls);
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_POTION), originalPotions);
    VireliaPrologue_GiveStarter();
    EXPECT_EQ(gSpecialVar_Result, FALSE);
    VireliaPrologue_GrantBasics();
    EXPECT_EQ(gSpecialVar_Result, checkpoint >= 2);
    EXPECT_EQ(gPartiesCount[B_TRAINER_PLAYER], originalPartyCount);
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_POKE_BALL), originalBalls);
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_POTION), originalPotions);
    EXPECT_EQ(VarGet(VAR_VIRELIA_PROLOGUE_STATE), originalState);
}

#endif // !IS_FRLG && P_VIRELIA_STARTERS
