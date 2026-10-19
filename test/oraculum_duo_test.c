#include "global.h"
#include "battle.h"
#include "event_data.h"
#include "frontier_util.h"
#include "new_game.h"
#include "pokemon.h"
#include "save.h"
#include "string_util.h"
#include "test/overworld_script.h"
#include "test/test.h"
#include "constants/battle_frontier.h"
#include "constants/frontier_util.h"
#include "constants/oraculum_duo_test.h"

#if ORACULUM_DUO_TEST_QUICK_START && !IS_FRLG

static void InitDuoTestParty(u32 partySize, u32 state)
{
    ZeroPlayerPartyMons();
    VarSet(VAR_ORACULUM_DUO_TEST_STATE, state);
    SetTrainerId(0x1234ABCD, gSaveBlock2Ptr->playerTrainerId);
    StringCopy(gSaveBlock2Ptr->playerName, COMPOUND_STRING("Duo"));
    gSaveBlock2Ptr->playerGender = FEMALE;

    for (u32 i = 0; i < partySize; i++)
        CreateRandomMonWithIVs(&gParties[B_TRAINER_PLAYER][i], SPECIES_WOBBUFFET, 20, 10);
    gPartiesCount[B_TRAINER_PLAYER] = partySize;
}

static void GiveDuoTestTeam(u32 team)
{
    if (team == ORACULUM_DUO_TEST_TEAM_A)
        RUN_OVERWORLD_SCRIPT(call DuoMultiplayerTest_EventScript_GiveTeamA;);
    else
        RUN_OVERWORLD_SCRIPT(call DuoMultiplayerTest_EventScript_GiveTeamB;);
}

static void ExpectReadyDuoTestMon(struct Pokemon *mon)
{
    u8 otName[PLAYER_NAME_LENGTH + 1];
    enum Species species = GetMonData(mon, MON_DATA_SPECIES);
    u32 evTotal = 0;

    EXPECT_EQ(GetMonData(mon, MON_DATA_LEVEL), 50);
    EXPECT_EQ(GetMonData(mon, MON_DATA_IS_EGG), FALSE);
    EXPECT(!gSpeciesInfo[species].isFrontierBanned);
    EXPECT_EQ(GetMonData(mon, MON_DATA_OT_ID), GetTrainerId(gSaveBlock2Ptr->playerTrainerId));
    EXPECT_EQ(GetMonData(mon, MON_DATA_OT_GENDER), gSaveBlock2Ptr->playerGender);
    GetMonData(mon, MON_DATA_OT_NAME, otName);
    EXPECT_EQ(StringCompare(otName, gSaveBlock2Ptr->playerName), 0);
    EXPECT_EQ(GetMonData(mon, MON_DATA_HP), GetMonData(mon, MON_DATA_MAX_HP));
    EXPECT_GT(GetMonData(mon, MON_DATA_HP), 0);
    EXPECT_EQ(GetMonData(mon, MON_DATA_STATUS), 0);
    EXPECT_NE(GetMonAbility(mon), ABILITY_NONE);

    for (u32 i = 0; i < NUM_STATS; i++)
    {
        EXPECT_EQ(GetMonData(mon, MON_DATA_HP_IV + i), 31);
        EXPECT_LE(GetMonData(mon, MON_DATA_HP_EV + i), 252);
        evTotal += GetMonData(mon, MON_DATA_HP_EV + i);
    }
    EXPECT_EQ(evTotal, 508);

    for (u32 i = 0; i < MAX_MON_MOVES; i++)
    {
        enum Move move = GetMonData(mon, MON_DATA_MOVE1 + i);
        EXPECT_NE(move, MOVE_NONE);
        EXPECT_EQ(GetMonData(mon, MON_DATA_PP1 + i), gMovesInfo[move].pp);
    }
}

TEST("Duo quick start grants two healthy local-owned Level 50 Tower-eligible Pokemon")
{
    u32 team = ORACULUM_DUO_TEST_TEAM_A;
    PARAMETRIZE { team = ORACULUM_DUO_TEST_TEAM_A; }
    PARAMETRIZE { team = ORACULUM_DUO_TEST_TEAM_B; }

    InitDuoTestParty(0, ORACULUM_DUO_TEST_PENDING);
    GiveDuoTestTeam(team);

    EXPECT_EQ(gSpecialVar_Result, TRUE);
    EXPECT_EQ(gPartiesCount[B_TRAINER_PLAYER], 2);
    EXPECT_EQ(VarGet(VAR_ORACULUM_DUO_TEST_STATE), team);
    EXPECT_EQ(FlagGet(FLAG_SYS_POKEMON_GET), TRUE);
    EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_SPECIES),
              team == ORACULUM_DUO_TEST_TEAM_A ? SPECIES_GARDEVOIR : SPECIES_SWAMPERT);
    EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][1], MON_DATA_SPECIES),
              team == ORACULUM_DUO_TEST_TEAM_A ? SPECIES_BRELOOM : SPECIES_MANECTRIC);
    EXPECT_NE(GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_HELD_ITEM),
              GetMonData(&gParties[B_TRAINER_PLAYER][1], MON_DATA_HELD_ITEM));
    ExpectReadyDuoTestMon(&gParties[B_TRAINER_PLAYER][0]);
    ExpectReadyDuoTestMon(&gParties[B_TRAINER_PLAYER][1]);

    // Exercise the attendant's native eligibility check, rather than a copy of its rules.
    VarSet(VAR_FRONTIER_FACILITY, FRONTIER_FACILITY_TOWER);
    VarSet(VAR_FRONTIER_BATTLE_MODE, FRONTIER_MODE_LINK_MULTIS);
    gSpecialVar_Result = FRONTIER_LVL_50;
    gSpecialVar_0x8004 = FRONTIER_UTIL_FUNC_CHECK_INELIGIBLE;
    CallFrontierUtilFunc();
    EXPECT_EQ(gSpecialVar_0x8004, FALSE);
}

TEST("Duo quick start completed state prevents repeated or opposite preset grants")
{
    struct Pokemon original[PARTY_SIZE];
    u32 team = ORACULUM_DUO_TEST_TEAM_A;
    PARAMETRIZE { team = ORACULUM_DUO_TEST_TEAM_A; }
    PARAMETRIZE { team = ORACULUM_DUO_TEST_TEAM_B; }

    InitDuoTestParty(0, ORACULUM_DUO_TEST_PENDING);
    GiveDuoTestTeam(team);
    memcpy(original, gParties[B_TRAINER_PLAYER], sizeof(original));

    GiveDuoTestTeam(ORACULUM_DUO_TEST_TEAM_A);
    EXPECT_EQ(gSpecialVar_Result, FALSE);
    GiveDuoTestTeam(ORACULUM_DUO_TEST_TEAM_B);
    EXPECT_EQ(gSpecialVar_Result, FALSE);
    EXPECT_EQ(gPartiesCount[B_TRAINER_PLAYER], 2);
    EXPECT_EQ(VarGet(VAR_ORACULUM_DUO_TEST_STATE), team);
    EXPECT_EQ(memcmp(original, gParties[B_TRAINER_PLAYER], sizeof(original)), 0);
}

TEST("Duo quick start respects saved completion state even after Pokemon leave the party")
{
    struct Pokemon original[PARTY_SIZE];
    u32 team = ORACULUM_DUO_TEST_TEAM_A;
    PARAMETRIZE { team = ORACULUM_DUO_TEST_TEAM_A; }
    PARAMETRIZE { team = ORACULUM_DUO_TEST_TEAM_B; }

    // This is the state read from the normal saved variable array after loading.
    // Grant eligibility must depend on that marker, not on the current party species.
    InitDuoTestParty(1, team);
    memcpy(original, gParties[B_TRAINER_PLAYER], sizeof(original));
    GiveDuoTestTeam(team);

    EXPECT_EQ(gSpecialVar_Result, FALSE);
    EXPECT_EQ(gPartiesCount[B_TRAINER_PLAYER], 1);
    EXPECT_EQ(VarGet(VAR_ORACULUM_DUO_TEST_STATE), team);
    EXPECT_EQ(memcmp(original, gParties[B_TRAINER_PLAYER], sizeof(original)), 0);
}

TEST("Duo quick start survives native flash save and load without granting another team")
{
    struct Pokemon loadedParty[PARTY_SIZE];
    u32 team = ORACULUM_DUO_TEST_TEAM_A;
    PARAMETRIZE { team = ORACULUM_DUO_TEST_TEAM_A; }
    PARAMETRIZE { team = ORACULUM_DUO_TEST_TEAM_B; }

    InitDuoTestParty(0, ORACULUM_DUO_TEST_PENDING);
    GiveDuoTestTeam(team);
    EXPECT_EQ(gSpecialVar_Result, TRUE);

    // Each runner has its own emulated flash. Remove older slots so their
    // counters cannot select data from a previous parameter or test.
    ClearSaveData();
    Save_ResetSaveCounters();
    EXPECT_EQ(TrySavingData(SAVE_NORMAL), SAVE_STATUS_OK);

    // Destroy the live party and completion marker before using the game's
    // real flash reader and party restoration path.
    ZeroPlayerPartyMons();
    VarSet(VAR_ORACULUM_DUO_TEST_STATE, ORACULUM_DUO_TEST_PENDING);
    SetTrainerId(0, gSaveBlock2Ptr->playerTrainerId);
    StringCopy(gSaveBlock2Ptr->playerName, COMPOUND_STRING("Other"));
    gSaveBlock2Ptr->playerGender = MALE;
    EXPECT_EQ(LoadGameSave(SAVE_NORMAL), SAVE_STATUS_OK);

    EXPECT_EQ(VarGet(VAR_ORACULUM_DUO_TEST_STATE), team);
    EXPECT_EQ(gPartiesCount[B_TRAINER_PLAYER], 2);
    EXPECT_EQ(GetTrainerId(gSaveBlock2Ptr->playerTrainerId), 0x1234ABCD);
    EXPECT_EQ(StringCompare(gSaveBlock2Ptr->playerName, COMPOUND_STRING("Duo")), 0);
    EXPECT_EQ(gSaveBlock2Ptr->playerGender, FEMALE);
    EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_SPECIES),
              team == ORACULUM_DUO_TEST_TEAM_A ? SPECIES_GARDEVOIR : SPECIES_SWAMPERT);
    EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][1], MON_DATA_SPECIES),
              team == ORACULUM_DUO_TEST_TEAM_A ? SPECIES_BRELOOM : SPECIES_MANECTRIC);
    ExpectReadyDuoTestMon(&gParties[B_TRAINER_PLAYER][0]);
    ExpectReadyDuoTestMon(&gParties[B_TRAINER_PLAYER][1]);
    VarSet(VAR_FRONTIER_FACILITY, FRONTIER_FACILITY_TOWER);
    VarSet(VAR_FRONTIER_BATTLE_MODE, FRONTIER_MODE_LINK_MULTIS);
    gSpecialVar_Result = FRONTIER_LVL_50;
    gSpecialVar_0x8004 = FRONTIER_UTIL_FUNC_CHECK_INELIGIBLE;
    CallFrontierUtilFunc();
    EXPECT_EQ(gSpecialVar_0x8004, FALSE);

    // Compare after load because the native loader migrates legacy fields.
    memcpy(loadedParty, gParties[B_TRAINER_PLAYER], sizeof(loadedParty));
    GiveDuoTestTeam(ORACULUM_DUO_TEST_TEAM_A);
    EXPECT_EQ(gSpecialVar_Result, FALSE);
    GiveDuoTestTeam(ORACULUM_DUO_TEST_TEAM_B);
    EXPECT_EQ(gSpecialVar_Result, FALSE);
    EXPECT_EQ(gPartiesCount[B_TRAINER_PLAYER], 2);
    EXPECT_EQ(VarGet(VAR_ORACULUM_DUO_TEST_STATE), team);
    EXPECT_EQ(memcmp(loadedParty, gParties[B_TRAINER_PLAYER], sizeof(loadedParty)), 0);
}

TEST("Duo optional setup appends to an existing party without changing existing Pokemon")
{
    struct Pokemon original[4];
    u32 hp = 1;
    u32 status = STATUS1_POISON;
    u32 team = ORACULUM_DUO_TEST_TEAM_A;
    PARAMETRIZE { team = ORACULUM_DUO_TEST_TEAM_A; }
    PARAMETRIZE { team = ORACULUM_DUO_TEST_TEAM_B; }

    InitDuoTestParty(ARRAY_COUNT(original), 0);
    SetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_HP, &hp);
    SetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_STATUS, &status);
    memcpy(original, gParties[B_TRAINER_PLAYER], sizeof(original));
    GiveDuoTestTeam(team);

    EXPECT_EQ(gSpecialVar_Result, TRUE);
    EXPECT_EQ(gPartiesCount[B_TRAINER_PLAYER], PARTY_SIZE);
    EXPECT_EQ(VarGet(VAR_ORACULUM_DUO_TEST_STATE), team);
    EXPECT_EQ(memcmp(original, gParties[B_TRAINER_PLAYER], sizeof(original)), 0);
    ExpectReadyDuoTestMon(&gParties[B_TRAINER_PLAYER][4]);
    ExpectReadyDuoTestMon(&gParties[B_TRAINER_PLAYER][5]);
}

TEST("Duo optional setup refuses parties with fewer than two free slots without mutation")
{
    struct Pokemon original[PARTY_SIZE];
    u32 partySize = 5;
    PARAMETRIZE { partySize = 5; }
    PARAMETRIZE { partySize = 6; }

    InitDuoTestParty(partySize, 0);
    memcpy(original, gParties[B_TRAINER_PLAYER], sizeof(original));
    GiveDuoTestTeam(ORACULUM_DUO_TEST_TEAM_A);
    EXPECT_EQ(gSpecialVar_Result, FALSE);
    GiveDuoTestTeam(ORACULUM_DUO_TEST_TEAM_B);
    EXPECT_EQ(gSpecialVar_Result, FALSE);
    EXPECT_EQ(gPartiesCount[B_TRAINER_PLAYER], partySize);
    EXPECT_EQ(VarGet(VAR_ORACULUM_DUO_TEST_STATE), 0);
    EXPECT_EQ(memcmp(original, gParties[B_TRAINER_PLAYER], sizeof(original)), 0);
}

#endif // ORACULUM_DUO_TEST_QUICK_START && !IS_FRLG
