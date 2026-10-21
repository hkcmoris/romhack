#include "global.h"
#include "agb_flash.h"
#include "battle.h"
#include "battle_setup.h"
#include "battle_transition.h"
#include "data.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "heal_location.h"
#include "graphics.h"
#include "item.h"
#include "main.h"
#include "new_game.h"
#include "overworld.h"
#include "pokedex.h"
#include "pokedex_common.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "save.h"
#include "string_util.h"
#include "task.h"
#include "trainer_pools.h"
#include "virelia_prologue.h"
#include "test/test.h"
#include "constants/event_objects.h"
#include "constants/heal_locations.h"
#include "constants/abilities.h"
#include "constants/battle_ai.h"
#include "constants/items.h"
#include "constants/moves.h"
#include "constants/oraculum_duo_test.h"
#include "constants/opponents.h"
#include "constants/trainers.h"
#include "constants/virelia_prologue.h"

#if !IS_FRLG && P_VIRELIA_STARTERS

// TESTING replaces gTrainers with trainer_control.party's synthetic teams.
// Compile the real generated table separately for these story-data regressions.
static const struct Trainer sVireliaStoryTrainers[DIFFICULTY_COUNT][TRAINERS_COUNT] =
{
#include "../src/data/trainers.h"
};

static void InitPrologueTest(void)
{
    InitEventData();
    ResetPokedex();
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
    EXPECT(!FlagGet(FLAG_VIRELIA_HIDE_GRASS_STARTER));
    EXPECT(!FlagGet(FLAG_VIRELIA_HIDE_WATER_STARTER));
    EXPECT(!FlagGet(FLAG_VIRELIA_HIDE_FIRE_STARTER));
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

TEST("Virelia new game restores all three starter Balls")
{
    InitPrologueTest();
    FlagSet(FLAG_VIRELIA_HIDE_GRASS_STARTER);
    FlagSet(FLAG_VIRELIA_HIDE_WATER_STARTER);
    FlagSet(FLAG_VIRELIA_HIDE_FIRE_STARTER);
    InitVireliaPrologueNewGame();
    EXPECT(!FlagGet(FLAG_VIRELIA_HIDE_GRASS_STARTER));
    EXPECT(!FlagGet(FLAG_VIRELIA_HIDE_WATER_STARTER));
    EXPECT(!FlagGet(FLAG_VIRELIA_HIDE_FIRE_STARTER));
}

#if VIRELIA_PROLOGUE && !VIRELIA_DEVELOPMENT_MODE
TEST("Virelia lab restores old-save Ball visibility for every starter before and after the rival")
{
    u32 choice = VIRELIA_STARTER_GRASS;
    bool32 rivalComplete = FALSE;
    for (u32 i = 0; i < VIRELIA_STARTER_COUNT; i++)
    {
        PARAMETRIZE { choice = i; rivalComplete = FALSE; }
        PARAMETRIZE { choice = i; rivalComplete = TRUE; }
    }
    InitPrologueTest();
    FlagSet(FLAG_VIRELIA_STARTER_RECEIVED);
    VarSet(VAR_VIRELIA_STARTER, choice);
    if (rivalComplete)
        FlagSet(FLAG_VIRELIA_RIVAL_COMPLETE);
    for (u32 visit = 0; visit < 2; visit++)
    {
        VireliaPrologue_SyncStarterBallFlags();
        EXPECT_EQ(FlagGet(FLAG_VIRELIA_HIDE_GRASS_STARTER),
                  choice == VIRELIA_STARTER_GRASS || (rivalComplete && choice == VIRELIA_STARTER_WATER));
        EXPECT_EQ(FlagGet(FLAG_VIRELIA_HIDE_WATER_STARTER),
                  choice == VIRELIA_STARTER_WATER || (rivalComplete && choice == VIRELIA_STARTER_FIRE));
        EXPECT_EQ(FlagGet(FLAG_VIRELIA_HIDE_FIRE_STARTER),
                  choice == VIRELIA_STARTER_FIRE || (rivalComplete && choice == VIRELIA_STARTER_GRASS));
    }
}

TEST("Virelia lab does not hide a Ball without a received valid starter")
{
    u32 choice = VIRELIA_STARTER_NONE;
    bool32 starterReceived = FALSE;
    PARAMETRIZE { choice = VIRELIA_STARTER_NONE; starterReceived = FALSE; }
    for (u32 i = 0; i < VIRELIA_STARTER_COUNT; i++)
        PARAMETRIZE { choice = i; starterReceived = FALSE; }
    PARAMETRIZE { choice = VIRELIA_STARTER_NONE; starterReceived = TRUE; }
    PARAMETRIZE { choice = VIRELIA_STARTER_COUNT; starterReceived = TRUE; }
    InitPrologueTest();
    VarSet(VAR_VIRELIA_STARTER, choice);
    if (starterReceived)
        FlagSet(FLAG_VIRELIA_STARTER_RECEIVED);
    FlagSet(FLAG_VIRELIA_RIVAL_COMPLETE);
    VireliaPrologue_SyncStarterBallFlags();
    EXPECT(!FlagGet(FLAG_VIRELIA_HIDE_GRASS_STARTER));
    EXPECT(!FlagGet(FLAG_VIRELIA_HIDE_WATER_STARTER));
    EXPECT(!FlagGet(FLAG_VIRELIA_HIDE_FIRE_STARTER));
}

TEST("Virelia lab preserves the rival Ball pickup before the battle completes")
{
    u32 choice = VIRELIA_STARTER_GRASS;
    u32 rivalFlag = FLAG_VIRELIA_HIDE_FIRE_STARTER;
    PARAMETRIZE { choice = VIRELIA_STARTER_GRASS; rivalFlag = FLAG_VIRELIA_HIDE_FIRE_STARTER; }
    PARAMETRIZE { choice = VIRELIA_STARTER_WATER; rivalFlag = FLAG_VIRELIA_HIDE_GRASS_STARTER; }
    PARAMETRIZE { choice = VIRELIA_STARTER_FIRE; rivalFlag = FLAG_VIRELIA_HIDE_WATER_STARTER; }
    InitPrologueTest();
    FlagSet(FLAG_VIRELIA_STARTER_RECEIVED);
    VarSet(VAR_VIRELIA_STARTER, choice);
    FlagSet(rivalFlag);
    for (u32 visit = 0; visit < 2; visit++)
    {
        VireliaPrologue_SyncStarterBallFlags();
        EXPECT(FlagGet(rivalFlag));
        EXPECT_EQ(FlagGet(FLAG_VIRELIA_HIDE_GRASS_STARTER), choice != VIRELIA_STARTER_FIRE);
        EXPECT_EQ(FlagGet(FLAG_VIRELIA_HIDE_WATER_STARTER), choice != VIRELIA_STARTER_GRASS);
        EXPECT_EQ(FlagGet(FLAG_VIRELIA_HIDE_FIRE_STARTER), choice != VIRELIA_STARTER_WATER);
        EXPECT(!FlagGet(FLAG_VIRELIA_RIVAL_COMPLETE));
    }
}
#endif

TEST("Virelia lab migrates saved Ball templates and active objects without moving other actors")
{
    u32 choice = VIRELIA_STARTER_NONE;
    bool32 rivalComplete = FALSE;
    PARAMETRIZE { choice = VIRELIA_STARTER_NONE; rivalComplete = FALSE; }
    for (u32 i = 0; i < VIRELIA_STARTER_COUNT; i++)
    {
        PARAMETRIZE { choice = i; rivalComplete = FALSE; }
        PARAMETRIZE { choice = i; rivalComplete = TRUE; }
    }
    static const u8 ballIds[] =
    {
        LOCALID_VIRELIA_LAB_GRASS_STARTER,
        LOCALID_VIRELIA_LAB_WATER_STARTER,
        LOCALID_VIRELIA_LAB_FIRE_STARTER,
    };
    static const u16 ballFlags[] =
    {
        FLAG_VIRELIA_HIDE_GRASS_STARTER,
        FLAG_VIRELIA_HIDE_WATER_STARTER,
        FLAG_VIRELIA_HIDE_FIRE_STARTER,
    };
    InitPrologueTest();
    struct MapHeader originalHeader = gMapHeader;
    struct ObjectEvent originalObjects[OBJECT_EVENTS_COUNT];
    struct ObjectEventTemplate originalTemplates[OBJECT_EVENT_TEMPLATES_COUNT];
    memcpy(originalObjects, gObjectEvents, sizeof(originalObjects));
    memcpy(originalTemplates, gSaveBlock1Ptr->objectEventTemplates, sizeof(originalTemplates));
    struct WarpData originalLocation = gSaveBlock1Ptr->location;
    gSaveBlock1Ptr->location.mapGroup = MAP_GROUP(MAP_VIRELIA_MAPLEBROOK_RESEARCH_LABORATORY);
    gSaveBlock1Ptr->location.mapNum = MAP_NUM(MAP_VIRELIA_MAPLEBROOK_RESEARCH_LABORATORY);
    gMapHeader = *Overworld_GetMapHeaderByGroupAndId(gSaveBlock1Ptr->location.mapGroup,
                                                   gSaveBlock1Ptr->location.mapNum);
    LoadObjEventTemplatesFromHeader();
    for (u32 i = 0; i < OBJECT_EVENTS_COUNT; i++)
        ClearObjectEvent(&gObjectEvents[i]);
    for (u32 i = 0; i < ARRAY_COUNT(ballIds); i++)
    {
        gObjectEvents[i] = (struct ObjectEvent)
        {
            .active = TRUE,
            .localId = ballIds[i],
            .mapGroup = gSaveBlock1Ptr->location.mapGroup,
            .mapNum = gSaveBlock1Ptr->location.mapNum,
        };
        for (u32 j = 0; j < OBJECT_EVENT_TEMPLATES_COUNT; j++)
            if (gSaveBlock1Ptr->objectEventTemplates[j].localId == ballIds[i])
                gSaveBlock1Ptr->objectEventTemplates[j].flagId = 0;
    }
    // A matching local ID from another map must remain untouched.
    gObjectEvents[3] = gObjectEvents[0];
    gObjectEvents[3].mapNum = MAP_NUM(MAP_VIRELIA_MAPLEBROOK_TOWN);
    gObjectEvents[3].mapGroup = MAP_GROUP(MAP_VIRELIA_MAPLEBROOK_TOWN);
    gObjectEvents[4] = gObjectEvents[0];
    gObjectEvents[4].localId = LOCALID_VIRELIA_LAB_VIKTOR;
    const struct ObjectEventTemplate *viktor = GetObjectEventTemplateByLocalIdAndMap(
        LOCALID_VIRELIA_LAB_VIKTOR, gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup);
    struct ObjectEventTemplate originalViktor = *viktor;
    if (choice != VIRELIA_STARTER_NONE)
    {
        VarSet(VAR_VIRELIA_STARTER, choice);
        FlagSet(FLAG_VIRELIA_STARTER_RECEIVED);
    }
    if (rivalComplete)
        FlagSet(FLAG_VIRELIA_RIVAL_COMPLETE);
    VireliaPrologue_SyncStarterBallFlags();
    VireliaPrologue_SyncStarterBallFlags();
    bool32 active[ARRAY_COUNT(ballIds)], hidden[ARRAY_COUNT(ballIds)];
    u16 migratedFlags[ARRAY_COUNT(ballIds)];
    for (u32 i = 0; i < ARRAY_COUNT(ballIds); i++)
    {
        active[i] = gObjectEvents[i].active;
        hidden[i] = FlagGet(ballFlags[i]);
        migratedFlags[i] = GetObjectEventFlagIdByLocalIdAndMap(ballIds[i],
            gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup);
    }
    bool32 otherMapActive = gObjectEvents[3].active;
    bool32 viktorActive = gObjectEvents[4].active;
    bool32 viktorUnchanged = memcmp(viktor, &originalViktor, sizeof(originalViktor)) == 0;
    // Restore global map/object state before an assertion can exit the test.
    gMapHeader = originalHeader;
    gSaveBlock1Ptr->location = originalLocation;
    memcpy(gObjectEvents, originalObjects, sizeof(originalObjects));
    memcpy(gSaveBlock1Ptr->objectEventTemplates, originalTemplates, sizeof(originalTemplates));
    for (u32 i = 0; i < ARRAY_COUNT(ballIds); i++)
    {
        EXPECT_EQ(migratedFlags[i], ballFlags[i]);
        EXPECT_EQ(active[i], !hidden[i]);
    }
    EXPECT(otherMapActive);
    EXPECT(viktorActive);
    EXPECT(viktorUnchanged);
}

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

TEST("Virelia compiled rival teams use the opposing level five starter and fair neutral moves")
{
    u32 choice = VIRELIA_STARTER_GRASS;
    PARAMETRIZE { choice = VIRELIA_STARTER_GRASS; }
    PARAMETRIZE { choice = VIRELIA_STARTER_WATER; }
    PARAMETRIZE { choice = VIRELIA_STARTER_FIRE; }
    static const u16 trainerIds[VIRELIA_STARTER_COUNT] =
    {
        [VIRELIA_STARTER_GRASS] = TRAINER_VIRELIA_VIKTOR_GRASS,
        [VIRELIA_STARTER_WATER] = TRAINER_VIRELIA_VIKTOR_WATER,
        [VIRELIA_STARTER_FIRE] = TRAINER_VIRELIA_VIKTOR_FIRE,
    };
    InitPrologueTest();
    struct Pokemon *party = gParties[B_TRAINER_OPPONENT_A];
    const struct Trainer *trainer = &sVireliaStoryTrainers[DIFFICULTY_NORMAL][trainerIds[choice]];
    u32 previousBattleFlags = gBattleTypeFlags;
    bool32 previousDebugBattle = gIsDebugBattle;
    gBattleTypeFlags = BATTLE_TYPE_TRAINER;
    gIsDebugBattle = FALSE;
    ZeroPartyMons(party);
    CreateNPCTrainerPartyFromTrainer(party, trainer);
    gBattleTypeFlags = previousBattleFlags;
    gIsDebugBattle = previousDebugBattle;
    EXPECT_EQ((u32)trainer->partySize, 1);
    EXPECT_EQ(GetMonData(&party[0], MON_DATA_SPECIES), GetVireliaRivalSpecies(choice));
    EXPECT_EQ(GetMonData(&party[0], MON_DATA_LEVEL), VIRELIA_RIVAL_LEVEL);
    EXPECT_EQ(GetMonData(&party[0], MON_DATA_MOVE1), MOVE_TACKLE);
    EXPECT_EQ(GetMonData(&party[0], MON_DATA_MOVE2), MOVE_GROWL);
    EXPECT_EQ(GetMonData(&party[0], MON_DATA_MOVE3), MOVE_NONE);
    EXPECT_EQ(GetMonData(&party[0], MON_DATA_MOVE4), MOVE_NONE);
    for (u32 i = 0; i < NUM_STATS; i++)
    {
        EXPECT_EQ(GetMonData(&party[0], MON_DATA_HP_IV + i), 0);
        EXPECT_EQ(GetMonData(&party[0], MON_DATA_HP_EV + i), 0);
    }
    EXPECT_EQ(GetMonData(&party[1], MON_DATA_SPECIES), SPECIES_NONE);
}

TEST("Virelia compiled route trainers and Rocket use the approved species levels and party sizes")
{
    u32 team = 0;
    PARAMETRIZE { team = 0; }
    PARAMETRIZE { team = 1; }
    PARAMETRIZE { team = 2; }
    static const u16 trainerIds[] = {TRAINER_VIRELIA_ELA, TRAINER_VIRELIA_OTA, TRAINER_VIRELIA_ROCKET};
    static const enum Species species[][2] =
    {
        {SPECIES_WURMPLE, SPECIES_ZIGZAGOON},
        {SPECIES_TAILLOW, SPECIES_NONE},
        {SPECIES_RATTATA, SPECIES_ZUBAT},
    };
    static const u8 levels[][2] = {{4, 4}, {6, 0}, {7, 6}};
    static const u8 counts[] = {2, 1, 2};
    InitPrologueTest();
    struct Pokemon *party = gParties[B_TRAINER_OPPONENT_A];
    const struct Trainer *trainer = &sVireliaStoryTrainers[DIFFICULTY_NORMAL][trainerIds[team]];
    u32 previousBattleFlags = gBattleTypeFlags;
    bool32 previousDebugBattle = gIsDebugBattle;
    gBattleTypeFlags = BATTLE_TYPE_TRAINER;
    gIsDebugBattle = FALSE;
    ZeroPartyMons(party);
    CreateNPCTrainerPartyFromTrainer(party, trainer);
    gBattleTypeFlags = previousBattleFlags;
    gIsDebugBattle = previousDebugBattle;
    EXPECT_EQ((u32)trainer->partySize, counts[team]);
    for (u32 i = 0; i < counts[team]; i++)
    {
        EXPECT_EQ(GetMonData(&party[i], MON_DATA_SPECIES), species[team][i]);
        EXPECT_EQ(GetMonData(&party[i], MON_DATA_LEVEL), levels[team][i]);
    }
    EXPECT_EQ(GetMonData(&party[counts[team]], MON_DATA_SPECIES), SPECIES_NONE);
}

TEST("Virelia introductory rival win loss and draw heal the team and advance exactly once")
{
    u32 outcome = B_OUTCOME_WON;
    PARAMETRIZE { outcome = B_OUTCOME_WON; }
    PARAMETRIZE { outcome = B_OUTCOME_LOST; }
    PARAMETRIZE { outcome = B_OUTCOME_DREW; }
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

TEST("Virelia introductory rival cannot complete from other battle outcomes")
{
    u32 outcome = 0;
    PARAMETRIZE { outcome = 0; }
    for (u32 i = B_OUTCOME_RAN; i <= B_OUTCOME_MON_TELEPORTED; i++)
        PARAMETRIZE { outcome = i; }
    PARAMETRIZE { outcome = B_OUTCOME_LINK_BATTLE_RAN; }
    InitPrologueTest();
    ReceivePrologueStarter(VIRELIA_STARTER_FIRE);
    gBattleOutcome = outcome;
    VireliaPrologue_CompleteRival();
    EXPECT_EQ(gSpecialVar_Result, FALSE);
    EXPECT(!FlagGet(FLAG_VIRELIA_RIVAL_COMPLETE));
    EXPECT_EQ(VarGet(VAR_VIRELIA_RIVAL_RESULT), 0);
    EXPECT_EQ(VarGet(VAR_VIRELIA_PROLOGUE_STATE), VIRELIA_STATE_STARTER);
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

TEST("Virelia research Pokedex includes every starter through the native National Dex")
{
    u32 choice = VIRELIA_STARTER_GRASS;
    PARAMETRIZE { choice = VIRELIA_STARTER_GRASS; }
    PARAMETRIZE { choice = VIRELIA_STARTER_WATER; }
    PARAMETRIZE { choice = VIRELIA_STARTER_FIRE; }
    InitPrologueTest();
    EXPECT(!IsNationalPokedexEnabled());
    ReceivePrologueStarter(choice);
    enum NationalDexOrder dexNum = SpeciesToNationalPokedexNum(GetVireliaStarterSpecies(choice));
    EXPECT(GetSetPokedexFlag(dexNum, FLAG_GET_SEEN));
    EXPECT(GetSetPokedexFlag(dexNum, FLAG_GET_CAUGHT));
    CompletePrologueRival(B_OUTCOME_LOST);
    VireliaPrologue_GrantBasics();
    EXPECT_EQ(gSpecialVar_Result, TRUE);
    EXPECT(IsNationalPokedexEnabled());
    EXPECT_EQ(gSaveBlock2Ptr->pokedex.mode, DEX_MODE_NATIONAL);
    EXPECT_EQ(GetNationalPokedexCount(FLAG_GET_SEEN), 1);
    EXPECT_EQ(GetNationalPokedexCount(FLAG_GET_CAUGHT), 1);
    EXPECT(!ShouldSkipPokedexListEntry(dexNum));
}

#if VIRELIA_PROLOGUE
TEST("Virelia default Pokedex resolves all nine local forms without requiring the HGSS interface")
{
    u32 index = 0;
    for (u32 i = 0; i < 9; i++)
        PARAMETRIZE { index = i; }
    static const enum Species forms[] =
    {
        SPECIES_POOCHYENA_VIRELIA, SPECIES_MIGHTYENA_VIRELIA, SPECIES_WRAITHENA,
        SPECIES_SPINARAK_VIRELIA, SPECIES_ARIADOS_VIRELIA, SPECIES_ARACHNAUT,
        SPECIES_EKANS_VIRELIA, SPECIES_ARBOK_VIRELIA, SPECIES_CINDRACOIL,
    };
    InitPrologueTest();
    ReceivePrologueStarter(index / 3);
    enum NationalDexOrder dexNum = SpeciesToNationalPokedexNum(forms[index]);
    EXPECT_EQ(NationalPokedexNumToSpeciesForm(dexNum), forms[index]);
    EXPECT_EQ(NationalPokedexNumToSpeciesForm(NATIONAL_DEX_WURMPLE), SPECIES_WURMPLE);
}

TEST("Virelia Pokedex preserves ordinary forms and identifies only the rival form actually encountered")
{
    u32 choice = VIRELIA_STARTER_GRASS;
    PARAMETRIZE { choice = VIRELIA_STARTER_GRASS; }
    PARAMETRIZE { choice = VIRELIA_STARTER_WATER; }
    PARAMETRIZE { choice = VIRELIA_STARTER_FIRE; }
    InitPrologueTest();
    EXPECT_EQ(NationalPokedexNumToSpeciesForm(NATIONAL_DEX_POOCHYENA), SPECIES_POOCHYENA);
    EXPECT_EQ(NationalPokedexNumToSpeciesForm(NATIONAL_DEX_SPINARAK), SPECIES_SPINARAK);
    EXPECT_EQ(NationalPokedexNumToSpeciesForm(NATIONAL_DEX_EKANS), SPECIES_EKANS);
    ReceivePrologueStarter(choice);
    enum Species rival = GetVireliaRivalSpecies(choice);
    enum NationalDexOrder rivalDex = SpeciesToNationalPokedexNum(rival);
    EXPECT_EQ(NationalPokedexNumToSpeciesForm(rivalDex), NationalPokedexNumToSpecies(rivalDex));
    CompletePrologueRival(B_OUTCOME_WON);
    EXPECT_EQ(NationalPokedexNumToSpeciesForm(rivalDex), rival);
    if (choice == VIRELIA_STARTER_FIRE)
    {
        EXPECT_EQ(NationalPokedexNumToSpeciesForm(NATIONAL_DEX_POOCHYENA), SPECIES_POOCHYENA);
        EXPECT_EQ(NationalPokedexNumToSpeciesForm(NATIONAL_DEX_MIGHTYENA), SPECIES_MIGHTYENA);
        EXPECT_EQ(NationalPokedexNumToSpeciesForm(NATIONAL_DEX_ARIADOS), SPECIES_ARIADOS);
    }
}
#endif

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
    // Reuse an occupied slot so the pre-starter save also checks party clearing,
    // independently of which test ran before this one.
    gParties[B_TRAINER_PLAYER][0].maxHP = 198;
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
    VireliaPrologue_SyncStarterBallFlags();
#if VIRELIA_PROLOGUE && !VIRELIA_DEVELOPMENT_MODE
    EXPECT_EQ(FlagGet(FLAG_VIRELIA_HIDE_WATER_STARTER), checkpoint >= 1);
    EXPECT_EQ(FlagGet(FLAG_VIRELIA_HIDE_GRASS_STARTER), checkpoint >= 2);
#else
    EXPECT(!FlagGet(FLAG_VIRELIA_HIDE_WATER_STARTER));
    EXPECT(!FlagGet(FLAG_VIRELIA_HIDE_GRASS_STARTER));
#endif
    EXPECT(!FlagGet(FLAG_VIRELIA_HIDE_FIRE_STARTER));
    // Exercise the actual saved map fields at four distinct story locations.
    // Only update save data here; the test runner's loaded map stays untouched.
    static const u16 checkpointMaps[] =
    {
        MAP_VIRELIA_MAPLEBROOK_PLAYERS_HOUSE,
        MAP_VIRELIA_MAPLEBROOK_RESEARCH_LABORATORY,
        MAP_VIRELIA_ROUTE01,
        MAP_VIRELIA_SLATEFORD_CITY,
    };
    static const s16 checkpointCoords[][2] = {{5, 6}, {4, 7}, {14, 39}, {14, 27}};
    gSaveBlock1Ptr->location = (struct WarpData)
    {
        .mapGroup = MAP_GROUP(checkpointMaps[checkpoint]),
        .mapNum = MAP_NUM(checkpointMaps[checkpoint]),
        .warpId = WARP_ID_NONE,
        .x = checkpointCoords[checkpoint][0],
        .y = checkpointCoords[checkpoint][1],
    };
    gSaveBlock1Ptr->pos.x = checkpointCoords[checkpoint][0];
    gSaveBlock1Ptr->pos.y = checkpointCoords[checkpoint][1];
    ClearTempFieldEventData();
    struct WarpData originalLocation, originalHealLocation;
    memcpy(&originalLocation, &gSaveBlock1Ptr->location, sizeof(originalLocation));
    memcpy(&originalHealLocation, &gSaveBlock1Ptr->lastHealLocation, sizeof(originalHealLocation));
    s16 originalX = gSaveBlock1Ptr->pos.x, originalY = gSaveBlock1Ptr->pos.y;
    struct Pokemon original = gParties[B_TRAINER_PLAYER][0];
    u32 originalPartyCount = gPartiesCount[B_TRAINER_PLAYER];
    u32 originalStarter = VarGet(VAR_VIRELIA_STARTER);
    u32 originalRivalResult = VarGet(VAR_VIRELIA_RIVAL_RESULT);
    u32 originalState = VarGet(VAR_VIRELIA_PROLOGUE_STATE);
    bool8 originalFlags[FLAG_VIRELIA_HIDE_FIRE_STARTER - FLAG_VIRELIA_MOM_MET + 1];
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
        gSaveBlock1Ptr->pos.x = -1;
        gSaveBlock1Ptr->pos.y = -1;
        gSaveBlock1Ptr->location = (struct WarpData)
        {
            .mapGroup = -1, .mapNum = -1, .warpId = WARP_ID_NONE, .x = -1, .y = -1,
        };
        gSaveBlock1Ptr->lastHealLocation = gSaveBlock1Ptr->location;
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
    EXPECT_EQ(gSaveBlock1Ptr->pos.x, originalX);
    EXPECT_EQ(gSaveBlock1Ptr->pos.y, originalY);
    EXPECT_EQ(memcmp(&gSaveBlock1Ptr->location, &originalLocation, sizeof(originalLocation)), 0);
    EXPECT_EQ(memcmp(&gSaveBlock1Ptr->lastHealLocation, &originalHealLocation, sizeof(originalHealLocation)), 0);
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

#if !VIRELIA_PROLOGUE || VIRELIA_DEVELOPMENT_MODE || IS_FRLG
TEST("Virelia nonstory lab restores all starter exhibition Balls")
{
    InitEventData();
    FlagSet(FLAG_VIRELIA_STARTER_RECEIVED);
    FlagSet(FLAG_VIRELIA_RIVAL_COMPLETE);
    VarSet(VAR_VIRELIA_STARTER, VIRELIA_STARTER_WATER);
    FlagSet(FLAG_VIRELIA_HIDE_GRASS_STARTER);
    FlagSet(FLAG_VIRELIA_HIDE_WATER_STARTER);
    FlagSet(FLAG_VIRELIA_HIDE_FIRE_STARTER);
    for (u32 visit = 0; visit < 2; visit++)
    {
        VireliaPrologue_SyncStarterBallFlags();
        EXPECT(!FlagGet(FLAG_VIRELIA_HIDE_GRASS_STARTER));
        EXPECT(!FlagGet(FLAG_VIRELIA_HIDE_WATER_STARTER));
        EXPECT(!FlagGet(FLAG_VIRELIA_HIDE_FIRE_STARTER));
    }
}
#endif
