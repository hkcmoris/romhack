#include "global.h"
#include "agb_flash.h"
#include "battle.h"
#include "daycare.h"
#include "event_data.h"
#include "new_game.h"
#include "pokedex.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "save.h"
#include "string_util.h"
#include "test/overworld_script.h"
#include "test/test.h"
#include "constants/move_relearner.h"

#if P_VIRELIA_STARTERS

struct VireliaExpectation
{
    enum Species species;
    enum Species firstStage;
    enum Type types[2];
    u16 bst;
    // HP, Attack, Defense, Sp. Attack, Sp. Defense, Speed (design order).
    u8 stats[NUM_STATS];
    enum DamageCategory category;
};

static const struct VireliaExpectation sVireliaExpectations[] =
{
    { SPECIES_POOCHYENA_VIRELIA, SPECIES_POOCHYENA_VIRELIA, { TYPE_GRASS, TYPE_DARK }, 315, { 50, 65, 45, 40, 45, 70 }, DAMAGE_CATEGORY_PHYSICAL },
    { SPECIES_MIGHTYENA_VIRELIA, SPECIES_POOCHYENA_VIRELIA, { TYPE_GRASS, TYPE_GHOST }, 425, { 65, 90, 60, 50, 60, 100 }, DAMAGE_CATEGORY_PHYSICAL },
    { SPECIES_WRAITHENA, SPECIES_POOCHYENA_VIRELIA, { TYPE_GRASS, TYPE_GHOST }, 530, { 78, 112, 82, 65, 85, 108 }, DAMAGE_CATEGORY_PHYSICAL },
    { SPECIES_NATU_VIRELIA, SPECIES_NATU_VIRELIA, { TYPE_WATER, TYPE_PSYCHIC }, 300, { 55, 35, 60, 55, 55, 40 }, DAMAGE_CATEGORY_SPECIAL },
    { SPECIES_XATU_VIRELIA, SPECIES_NATU_VIRELIA, { TYPE_WATER, TYPE_PSYCHIC }, 405, { 75, 45, 80, 75, 75, 55 }, DAMAGE_CATEGORY_SPECIAL },
    { SPECIES_NIMBATU, SPECIES_NATU_VIRELIA, { TYPE_WATER, TYPE_PSYCHIC }, 530, { 94, 58, 100, 102, 98, 78 }, DAMAGE_CATEGORY_SPECIAL },
    { SPECIES_EKANS_VIRELIA, SPECIES_EKANS_VIRELIA, { TYPE_FIRE, TYPE_POISON }, 310, { 45, 40, 45, 70, 45, 65 }, DAMAGE_CATEGORY_SPECIAL },
    { SPECIES_ARBOK_VIRELIA, SPECIES_EKANS_VIRELIA, { TYPE_FIRE, TYPE_POISON }, 415, { 60, 55, 60, 90, 65, 85 }, DAMAGE_CATEGORY_SPECIAL },
    { SPECIES_INFERNABRA, SPECIES_EKANS_VIRELIA, { TYPE_FIRE, TYPE_POISON }, 530, { 78, 72, 75, 119, 88, 98 }, DAMAGE_CATEGORY_SPECIAL },
};

static const u8 sTestLevels[] = { 5, 10, 14, 16, 18, 20, 30, 34, 36, 38, 40, 50 };

static void InitVireliaOwner(void)
{
    SetTrainerId(0x1234ABCD, gSaveBlock2Ptr->playerTrainerId);
    StringCopy(gSaveBlock2Ptr->playerName, COMPOUND_STRING("Virelia"));
    gSaveBlock2Ptr->playerGender = FEMALE;
}

static void ExpectValidVireliaMon(struct Pokemon *mon, enum Species species, u32 level)
{
    u8 otName[PLAYER_NAME_LENGTH + 1];
    bool32 usableStab[2] = { FALSE, FALSE };
    enum DamageCategory category = GetSpeciesType(species, 0) == TYPE_GRASS ? DAMAGE_CATEGORY_PHYSICAL : DAMAGE_CATEGORY_SPECIAL;

    EXPECT_EQ(GetMonData(mon, MON_DATA_SPECIES), species);
    EXPECT_EQ(GetMonData(mon, MON_DATA_LEVEL), level);
    EXPECT_EQ(GetMonData(mon, MON_DATA_SANITY_IS_BAD_EGG), FALSE);
    EXPECT_EQ(GetMonData(mon, MON_DATA_IS_EGG), FALSE);
    EXPECT_EQ(GetMonData(mon, MON_DATA_OT_ID), GetTrainerId(gSaveBlock2Ptr->playerTrainerId));
    EXPECT_EQ(GetMonData(mon, MON_DATA_OT_GENDER), gSaveBlock2Ptr->playerGender);
    GetMonData(mon, MON_DATA_OT_NAME, otName);
    EXPECT_EQ(StringCompare(otName, gSaveBlock2Ptr->playerName), 0);
    EXPECT_EQ(IsTradedMon(mon), FALSE);
    EXPECT_EQ(GetMonData(mon, MON_DATA_HP), GetMonData(mon, MON_DATA_MAX_HP));
    EXPECT_GT(GetMonData(mon, MON_DATA_HP), 0);
    EXPECT_EQ(GetMonData(mon, MON_DATA_STATUS), 0);
    EXPECT_NE(GetMonAbility(mon), ABILITY_NONE);
    EXPECT_LT(GetMonAbility(mon), ABILITIES_COUNT);
    EXPECT(!gSpeciesInfo[species].cannotBeTraded);

    for (u32 i = 0; i < MAX_MON_MOVES; i++)
    {
        enum Move move = GetMonData(mon, MON_DATA_MOVE1 + i);
        if (move == MOVE_NONE)
            continue;
        EXPECT_LT(move, MOVES_COUNT);
        EXPECT_GT(gMovesInfo[move].pp, 0);
        EXPECT_EQ(GetMonData(mon, MON_DATA_PP1 + i), gMovesInfo[move].pp);
        if (gMovesInfo[move].power > 0 && gMovesInfo[move].category == category)
            for (u32 slot = 0; slot < 2; slot++)
                if (gMovesInfo[move].type == GetSpeciesType(species, slot))
                    usableStab[slot] = TRUE;
    }
    EXPECT(usableStab[0]);
    EXPECT(usableStab[1]);
}

TEST("Virelia species have the specified typing, stats, BST, and regional breeding ancestry")
{
    u32 index = 0;
    for (u32 i = 0; i < ARRAY_COUNT(sVireliaExpectations); i++)
        PARAMETRIZE_LABEL("species:%d", sVireliaExpectations[i].species) { index = i; }

    const struct VireliaExpectation *expected = &sVireliaExpectations[index];
    const struct SpeciesInfo *info = &gSpeciesInfo[expected->species];
    EXPECT(IsSpeciesEnabled(expected->species));
    EXPECT_EQ(GetSpeciesType(expected->species, 0), expected->types[0]);
    EXPECT_EQ(GetSpeciesType(expected->species, 1), expected->types[1]);
    EXPECT_EQ(info->baseHP, expected->stats[0]);
    EXPECT_EQ(info->baseAttack, expected->stats[1]);
    EXPECT_EQ(info->baseDefense, expected->stats[2]);
    EXPECT_EQ(info->baseSpAttack, expected->stats[3]);
    EXPECT_EQ(info->baseSpDefense, expected->stats[4]);
    EXPECT_EQ(info->baseSpeed, expected->stats[5]);
    EXPECT_EQ(info->baseHP + info->baseAttack + info->baseDefense
              + info->baseSpAttack + info->baseSpDefense + info->baseSpeed, expected->bst);
    EXPECT_EQ(GetEggSpecies(expected->species), expected->firstStage);
}

TEST("Virelia and original evolution thresholds use the native level evolution rules")
{
    enum Species species = SPECIES_NONE, target = SPECIES_NONE;
    u32 threshold = 0;
    s32 levelOffset = 0;
    struct Pokemon mon;
    bool32 canStopEvo = TRUE;

    for (s32 offset = -1; offset <= 1; offset++)
    {
        PARAMETRIZE { species = SPECIES_POOCHYENA_VIRELIA; target = SPECIES_MIGHTYENA_VIRELIA; threshold = 18; levelOffset = offset; }
        PARAMETRIZE { species = SPECIES_MIGHTYENA_VIRELIA; target = SPECIES_WRAITHENA; threshold = 38; levelOffset = offset; }
        PARAMETRIZE { species = SPECIES_NATU_VIRELIA; target = SPECIES_XATU_VIRELIA; threshold = 14; levelOffset = offset; }
        PARAMETRIZE { species = SPECIES_XATU_VIRELIA; target = SPECIES_NIMBATU; threshold = 34; levelOffset = offset; }
        PARAMETRIZE { species = SPECIES_EKANS_VIRELIA; target = SPECIES_ARBOK_VIRELIA; threshold = 16; levelOffset = offset; }
        PARAMETRIZE { species = SPECIES_ARBOK_VIRELIA; target = SPECIES_INFERNABRA; threshold = 36; levelOffset = offset; }
        PARAMETRIZE { species = SPECIES_POOCHYENA; target = SPECIES_MIGHTYENA; threshold = 18; levelOffset = offset; }
        PARAMETRIZE { species = SPECIES_NATU; target = SPECIES_XATU; threshold = 25; levelOffset = offset; }
        PARAMETRIZE { species = SPECIES_SPINARAK; target = SPECIES_ARIADOS; threshold = 22; levelOffset = offset; }
        PARAMETRIZE { species = SPECIES_EKANS; target = SPECIES_ARBOK; threshold = 22; levelOffset = offset; }
    }

    CreateRandomMonWithIVs(&mon, species, threshold + levelOffset, 10);
    EXPECT_EQ(GetEvolutionTargetSpecies(&mon, EVO_MODE_NORMAL, ITEM_NONE, NULL, &canStopEvo, CHECK_EVO),
              levelOffset < 0 ? SPECIES_NONE : target);
    EXPECT_EQ(GetEvolutionTargetSpecies(&mon, EVO_MODE_NORMAL, ITEM_NONE, NULL, &canStopEvo, DO_EVO),
              levelOffset < 0 ? SPECIES_NONE : target);
    // This also checks that the table uses normal engine restrictions.
    u32 heldItem = ITEM_EVERSTONE;
    SetMonData(&mon, MON_DATA_HELD_ITEM, &heldItem);
    EXPECT_EQ(GetEvolutionTargetSpecies(&mon, EVO_MODE_NORMAL, ITEM_NONE, NULL, &canStopEvo, CHECK_EVO), SPECIES_NONE);
}

TEST("Virelia final stages and original final stages have no unintended extra evolution")
{
    enum Species species = SPECIES_NONE, firstStage = SPECIES_NONE;
    struct Pokemon mon;
    PARAMETRIZE { species = SPECIES_WRAITHENA; firstStage = SPECIES_POOCHYENA_VIRELIA; }
    PARAMETRIZE { species = SPECIES_NIMBATU; firstStage = SPECIES_NATU_VIRELIA; }
    PARAMETRIZE { species = SPECIES_INFERNABRA; firstStage = SPECIES_EKANS_VIRELIA; }
    PARAMETRIZE { species = SPECIES_MIGHTYENA; firstStage = SPECIES_POOCHYENA; }
    PARAMETRIZE { species = SPECIES_XATU; firstStage = SPECIES_NATU; }
    PARAMETRIZE { species = SPECIES_ARIADOS; firstStage = SPECIES_SPINARAK; }
    PARAMETRIZE { species = SPECIES_ARBOK; firstStage = SPECIES_EKANS; }

    CreateRandomMonWithIVs(&mon, species, MAX_LEVEL, 10);
    EXPECT_EQ(GetEvolutionTargetSpecies(&mon, EVO_MODE_NORMAL, ITEM_NONE, NULL, NULL, CHECK_EVO), SPECIES_NONE);
    EXPECT_EQ(GetEggSpecies(species), firstStage);
}

TEST("Virelia regional form tables preserve the original species and typing")
{
    enum Species original = SPECIES_NONE, regional = SPECIES_NONE;
    enum Type firstType = TYPE_NONE, secondType = TYPE_NONE;
    bool32 hasOriginal = FALSE, hasRegional = FALSE;
    PARAMETRIZE { original = SPECIES_POOCHYENA; regional = SPECIES_POOCHYENA_VIRELIA; firstType = TYPE_DARK; secondType = TYPE_DARK; }
    PARAMETRIZE { original = SPECIES_MIGHTYENA; regional = SPECIES_MIGHTYENA_VIRELIA; firstType = TYPE_DARK; secondType = TYPE_DARK; }
    PARAMETRIZE { original = SPECIES_NATU; regional = SPECIES_NATU_VIRELIA; firstType = TYPE_PSYCHIC; secondType = TYPE_FLYING; }
    PARAMETRIZE { original = SPECIES_XATU; regional = SPECIES_XATU_VIRELIA; firstType = TYPE_PSYCHIC; secondType = TYPE_FLYING; }
    PARAMETRIZE { original = SPECIES_EKANS; regional = SPECIES_EKANS_VIRELIA; firstType = TYPE_POISON; secondType = TYPE_POISON; }
    PARAMETRIZE { original = SPECIES_ARBOK; regional = SPECIES_ARBOK_VIRELIA; firstType = TYPE_POISON; secondType = TYPE_POISON; }

    const u16 *forms = gSpeciesInfo[original].formSpeciesIdTable;
    EXPECT_NE(forms, NULL);
    EXPECT_EQ(forms, gSpeciesInfo[regional].formSpeciesIdTable);
    for (u32 i = 0; forms[i] != FORM_SPECIES_END; i++)
    {
        if (forms[i] == original)
            hasOriginal = TRUE;
        if (forms[i] == regional)
            hasRegional = TRUE;
    }
    EXPECT(hasOriginal);
    EXPECT(hasRegional);
    EXPECT_EQ(GetSpeciesType(original, 0), firstType);
    EXPECT_EQ(GetSpeciesType(original, 1), secondType);
}

TEST("Virelia learnsets are valid and provide both role-appropriate damaging STAB types by level 5")
{
    u32 index = 0, count = 0;
    bool32 earlyStab[2] = { FALSE, FALSE };
    for (u32 i = 0; i < ARRAY_COUNT(sVireliaExpectations); i++)
        PARAMETRIZE_LABEL("species:%d", sVireliaExpectations[i].species) { index = i; }

    const struct VireliaExpectation *expected = &sVireliaExpectations[index];
    const struct LevelUpMove *learnset = GetSpeciesLevelUpLearnset(expected->species);
    EXPECT_NE(learnset, NULL);
    for (; count < MAX_RELEARNER_MOVES && learnset[count].move != LEVEL_UP_MOVE_END; count++)
    {
        enum Move move = learnset[count].move;
        EXPECT_GT(move, MOVE_NONE);
        EXPECT_LT(move, MOVES_COUNT);
        EXPECT_LE(learnset[count].level, MAX_LEVEL);
        if (count > 0)
            EXPECT_LE(learnset[count - 1].level, learnset[count].level);
        EXPECT_GT(gMovesInfo[move].pp, 0);
        if (learnset[count].level <= 5 && gMovesInfo[move].power > 0 && gMovesInfo[move].category == expected->category)
        {
            for (u32 slot = 0; slot < 2; slot++)
                if (gMovesInfo[move].type == expected->types[slot])
                    earlyStab[slot] = TRUE;
        }
    }
    EXPECT_GT(count, 0);
    EXPECT_LT(count, MAX_RELEARNER_MOVES - 1);
    EXPECT(earlyStab[0]);
    EXPECT(earlyStab[1]);
}

TEST("Virelia species create valid owned Pokemon at every development test level and survive boxing")
{
    u32 index = 0, level = 5;
    struct Pokemon mon, unboxed;
    for (u32 i = 0; i < ARRAY_COUNT(sVireliaExpectations); i++)
        for (u32 j = 0; j < ARRAY_COUNT(sTestLevels); j++)
            PARAMETRIZE_LABEL("species:%d level:%d", sVireliaExpectations[i].species, sTestLevels[j]) { index = i; level = sTestLevels[j]; }

    InitVireliaOwner();
    CreateRandomMonWithIVs(&mon, sVireliaExpectations[index].species, level, 10);
    ExpectValidVireliaMon(&mon, sVireliaExpectations[index].species, level);
    BoxMonToMon(&mon.box, &unboxed);
    ExpectValidVireliaMon(&unboxed, sVireliaExpectations[index].species, level);
    EXPECT_EQ(memcmp(&mon.box, &unboxed.box, sizeof(mon.box)), 0);
}

TEST("Virelia species, ownership, PC data, and Pokedex flags survive native flash save and load")
{
    struct Pokemon mon;
    struct BoxPokemon originals[ARRAY_COUNT(sVireliaExpectations)];
    IntrFunc originalTimer3Handler, unusedFlashHandler;
    u16 originalIe, originalIme, originalTimer3Low, originalTimer3High;
    u8 saveStatus, loadStatus = SAVE_STATUS_ERROR;

    // New dex numbers must fit existing spare bits. test/save.c also protects
    // the actual SaveBlock and PokemonStorage sizes without updating its baselines.
    EXPECT_EQ(NUM_DEX_FLAG_BYTES, 129);
    InitVireliaOwner();
    ZeroPlayerPartyMons();
    ResetPokemonStorageSystem();
    VarSet(VAR_PC_BOX_TO_SEND_MON, 0);
    for (u32 i = 0; i < ARRAY_COUNT(sVireliaExpectations); i++)
    {
        enum Species species = sVireliaExpectations[i].species;
        CreateRandomMonWithIVs(&mon, species, 50, 10);
        EXPECT_EQ(CopyMonToPC(&mon), MON_GIVEN_TO_PC);
        EXPECT_EQ(gSpecialVar_MonBoxId, 0);
        EXPECT_EQ(gSpecialVar_MonBoxPos, i);
        originals[i] = *GetBoxedMonPtr(0, i);
        if (i < PARTY_SIZE)
            gParties[B_TRAINER_PLAYER][i] = mon;
        GetSetPokedexFlag(SpeciesToNationalPokedexNum(species), FLAG_SET_SEEN);
        GetSetPokedexFlag(SpeciesToNationalPokedexNum(species), FLAG_SET_CAUGHT);
    }
    gPartiesCount[B_TRAINER_PLAYER] = PARTY_SIZE;

    // Flash normally uses the runner's watchdog timer. Borrow Timer 3 and
    // restore every register/handler before assertions can exit nonlocally.
    originalIme = REG_IME;
    REG_IME = 0;
    originalIe = REG_IE;
    originalTimer3Handler = gIntrTable[2];
    originalTimer3Low = REG_TM3CNT_L;
    originalTimer3High = REG_TM3CNT_H;
    REG_TM3CNT_H = 0;
    REG_IF = INTR_FLAG_TIMER3;
    SetFlashTimerIntr(3, &gIntrTable[2]);
    REG_IME = originalIme;
    ClearSaveData();
    Save_ResetSaveCounters();
    saveStatus = TrySavingData(SAVE_NORMAL);
    if (saveStatus == SAVE_STATUS_OK)
    {
        ZeroPlayerPartyMons();
        for (u32 i = 0; i < ARRAY_COUNT(sVireliaExpectations); i++)
            ZeroBoxMonAt(0, i);
        memset(gSaveBlock1Ptr->dexSeen, 0, sizeof(gSaveBlock1Ptr->dexSeen));
        memset(gSaveBlock1Ptr->dexCaught, 0, sizeof(gSaveBlock1Ptr->dexCaught));
        SetTrainerId(0, gSaveBlock2Ptr->playerTrainerId);
        StringCopy(gSaveBlock2Ptr->playerName, COMPOUND_STRING("Other"));
        gSaveBlock2Ptr->playerGender = MALE;
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
    EXPECT_EQ(GetTrainerId(gSaveBlock2Ptr->playerTrainerId), 0x1234ABCD);
    EXPECT_EQ(StringCompare(gSaveBlock2Ptr->playerName, COMPOUND_STRING("Virelia")), 0);
    EXPECT_EQ(gSaveBlock2Ptr->playerGender, FEMALE);
    EXPECT_EQ(gPartiesCount[B_TRAINER_PLAYER], PARTY_SIZE);
    for (u32 i = 0; i < ARRAY_COUNT(sVireliaExpectations); i++)
    {
        enum Species species = sVireliaExpectations[i].species;
        EXPECT_EQ(memcmp(&originals[i], GetBoxedMonPtr(0, i), sizeof(originals[i])), 0);
        BoxMonAtToMon(0, i, &mon);
        ExpectValidVireliaMon(&mon, species, 50);
        if (i < PARTY_SIZE)
            ExpectValidVireliaMon(&gParties[B_TRAINER_PLAYER][i], species, 50);
        EXPECT_EQ(GetSetPokedexFlag(SpeciesToNationalPokedexNum(species), FLAG_GET_SEEN), TRUE);
        EXPECT_EQ(GetSetPokedexFlag(SpeciesToNationalPokedexNum(species), FLAG_GET_CAUGHT), TRUE);
    }
}

#if VIRELIA_STARTER_TEST

static void InitVireliaTestParty(u32 count)
{
    InitVireliaOwner();
    ZeroPlayerPartyMons();
    for (u32 i = 0; i < count; i++)
        CreateRandomMonWithIVs(&gParties[B_TRAINER_PLAYER][i], SPECIES_WOBBUFFET, 20, 10);
    gPartiesCount[B_TRAINER_PLAYER] = count;
    if (count > 0)
    {
        u32 hp = 1, status = STATUS1_POISON;
        SetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_HP, &hp);
        SetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_STATUS, &status);
    }
}

static void GiveVireliaTestMon(enum Species species, u32 level)
{
    gSpecialVar_0x8004 = species;
    gSpecialVar_0x8005 = level;
    RUN_OVERWORLD_SCRIPT(call VireliaStarterTest_EventScript_Give;);
}

TEST("Virelia NPC grants all nine stages at all twelve levels without changing existing party data")
{
    struct Pokemon original;
    u32 index = 0, level = 5;
    for (u32 i = 0; i < ARRAY_COUNT(sVireliaExpectations); i++)
        for (u32 j = 0; j < ARRAY_COUNT(sTestLevels); j++)
            PARAMETRIZE_LABEL("species:%d level:%d", sVireliaExpectations[i].species, sTestLevels[j]) { index = i; level = sTestLevels[j]; }

    InitVireliaTestParty(1);
    original = gParties[B_TRAINER_PLAYER][0];
    GiveVireliaTestMon(sVireliaExpectations[index].species, level);
    EXPECT_EQ(gSpecialVar_Result, MON_GIVEN_TO_PARTY);
    EXPECT_EQ(gPartiesCount[B_TRAINER_PLAYER], 2);
    EXPECT_EQ(memcmp(&original, &gParties[B_TRAINER_PLAYER][0], sizeof(original)), 0);
    ExpectValidVireliaMon(&gParties[B_TRAINER_PLAYER][1], sVireliaExpectations[index].species, level);
}

TEST("Virelia NPC supports empty parties and the last available party slot")
{
    struct Pokemon original[PARTY_SIZE];
    u32 index = 0, count = 0;
    for (u32 i = 0; i < ARRAY_COUNT(sVireliaExpectations); i++)
    {
        PARAMETRIZE { index = i; count = 0; }
        PARAMETRIZE { index = i; count = PARTY_SIZE - 1; }
    }

    InitVireliaTestParty(count);
    memcpy(original, gParties[B_TRAINER_PLAYER], sizeof(original));
    GiveVireliaTestMon(sVireliaExpectations[index].species, 5);
    EXPECT_EQ(gSpecialVar_Result, MON_GIVEN_TO_PARTY);
    EXPECT_EQ(gPartiesCount[B_TRAINER_PLAYER], count + 1);
    EXPECT_EQ(memcmp(original, gParties[B_TRAINER_PLAYER], count * sizeof(original[0])), 0);
    ExpectValidVireliaMon(&gParties[B_TRAINER_PLAYER][count], sVireliaExpectations[index].species, 5);
}

TEST("Virelia NPC refuses full parties without overwriting Pokemon or sending them to the PC")
{
    struct Pokemon original[PARTY_SIZE];
    u32 index = 0;
    for (u32 i = 0; i < ARRAY_COUNT(sVireliaExpectations); i++)
        PARAMETRIZE { index = i; }

    InitVireliaTestParty(PARTY_SIZE);
    ResetPokemonStorageSystem();
    memcpy(original, gParties[B_TRAINER_PLAYER], sizeof(original));
    GiveVireliaTestMon(sVireliaExpectations[index].species, 50);
    EXPECT_EQ(gSpecialVar_Result, MON_CANT_GIVE);
    EXPECT_EQ(gPartiesCount[B_TRAINER_PLAYER], PARTY_SIZE);
    EXPECT_EQ(memcmp(original, gParties[B_TRAINER_PLAYER], sizeof(original)), 0);
    EXPECT_EQ(CountStorageNonEggMons(), 0);
}

TEST("Virelia NPC rejects invalid species and test levels without modifying the party")
{
    struct Pokemon original[PARTY_SIZE];
    enum Species species = SPECIES_POOCHYENA_VIRELIA;
    u32 level = 5;
    PARAMETRIZE { species = SPECIES_NONE; }
    PARAMETRIZE { species = SPECIES_POOCHYENA; }
    PARAMETRIZE { species = SPECIES_NATU; }
    PARAMETRIZE { species = SPECIES_EKANS; }
    PARAMETRIZE { species = NUM_SPECIES; }
    PARAMETRIZE { level = 0; }
    PARAMETRIZE { level = 1; }
    PARAMETRIZE { level = 13; }
    PARAMETRIZE { level = 15; }
    PARAMETRIZE { level = 17; }
    PARAMETRIZE { level = 33; }
    PARAMETRIZE { level = 35; }
    PARAMETRIZE { level = 37; }
    PARAMETRIZE { level = 51; }
    PARAMETRIZE { level = 100; }
    PARAMETRIZE { level = 255; }

    InitVireliaTestParty(1);
    memcpy(original, gParties[B_TRAINER_PLAYER], sizeof(original));
    GiveVireliaTestMon(species, level);
    EXPECT_EQ(gSpecialVar_Result, MON_CANT_GIVE);
    EXPECT_EQ(gPartiesCount[B_TRAINER_PLAYER], 1);
    EXPECT_EQ(memcmp(original, gParties[B_TRAINER_PLAYER], sizeof(original)), 0);
}

#endif // VIRELIA_STARTER_TEST
#endif // P_VIRELIA_STARTERS
