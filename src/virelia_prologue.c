#include "global.h"
#include "battle.h"
#include "battle_setup.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "fieldmap.h"
#include "item.h"
#include "overworld.h"
#include "pokemon.h"
#include "script_pokemon_util.h"
#include "virelia_prologue.h"
#include "constants/event_objects.h"
#include "constants/heal_locations.h"
#include "constants/items.h"
#include "constants/maps.h"
#include "constants/oraculum_duo_test.h"
#include "constants/virelia_prologue.h"
#include "constants/virelia_maplebrook.h"
#include "constants/battle_partner.h"
#include "constants/battle_special.h"
#include "constants/opponents.h"

// The rival takes the starter with a type advantage against the player's choice.
static const u8 sRivalChoices[VIRELIA_STARTER_COUNT] =
{
    [VIRELIA_STARTER_GRASS] = VIRELIA_STARTER_FIRE,
    [VIRELIA_STARTER_WATER] = VIRELIA_STARTER_GRASS,
    [VIRELIA_STARTER_FIRE] = VIRELIA_STARTER_WATER,
};

u32 GetVireliaStarterSpecies(u32 choice)
{
#if P_VIRELIA_STARTERS
    static const u16 sSpecies[VIRELIA_STARTER_COUNT] =
    {
        [VIRELIA_STARTER_GRASS] = SPECIES_POOCHYENA_VIRELIA,
        [VIRELIA_STARTER_WATER] = SPECIES_NATU_VIRELIA,
        [VIRELIA_STARTER_FIRE] = SPECIES_EKANS_VIRELIA,
    };
    if (choice < ARRAY_COUNT(sSpecies))
        return sSpecies[choice];
#endif
    return SPECIES_NONE;
}

u32 GetVireliaRivalSpecies(u32 choice)
{
    if (choice < ARRAY_COUNT(sRivalChoices))
        return GetVireliaStarterSpecies(sRivalChoices[choice]);
    return SPECIES_NONE;
}

u32 GetVireliaRivalGender(void)
{
    return gSaveBlock2Ptr->playerGender == MALE ? FEMALE : MALE;
}

const u8 *GetVireliaRivalName(void)
{
    static const u8 sViktor[] = _("VIKTOR");
    static const u8 sViktorie[] = _("VIKTORIE");
    return GetVireliaRivalGender() == FEMALE ? sViktorie : sViktor;
}

void VireliaPrologue_SetRivalAppearance(void)
{
    VarSet(VAR_OBJ_GFX_ID_0, GetVireliaRivalGender() == FEMALE
                         ? OBJ_EVENT_GFX_RIVAL_MAY_NORMAL
                         : OBJ_EVENT_GFX_RIVAL_BRENDAN_NORMAL);
}

void InitVireliaPrologueNewGame(void)
{
#if !IS_FRLG
    // Do not enter the Hoenn truck, rescue or first-rival scripts from a new
    // Virelia save. This deliberately leaves Duo battle/link state untouched.
    VarSet(VAR_LITTLEROOT_INTRO_STATE, 7);
    VarSet(VAR_LITTLEROOT_TOWN_STATE, 4);
    VarSet(VAR_LITTLEROOT_HOUSES_STATE_BRENDAN, 2);
    VarSet(VAR_LITTLEROOT_HOUSES_STATE_MAY, 2);
    VarSet(VAR_LITTLEROOT_RIVAL_STATE, 4);
    VarSet(VAR_ROUTE101_STATE, 3);
    VarSet(VAR_BIRCH_LAB_STATE, 5);
    VarSet(VAR_OLDALE_RIVAL_STATE, 2);
    VarSet(VAR_ORACULUM_DUO_TEST_STATE, 0);
    FlagSet(FLAG_RESCUED_BIRCH);
    FlagSet(FLAG_MET_RIVAL_MOM);
    FlagSet(FLAG_SET_WALL_CLOCK);
    FlagSet(FLAG_RECEIVED_RUNNING_SHOES);
    FlagSet(FLAG_SYS_B_DASH);

    for (u32 flag = FLAG_VIRELIA_MOM_MET; flag <= FLAG_VIRELIA_HIDE_FIRE_STARTER; flag++)
        FlagClear(flag);
    FlagClear(FLAG_VIRELIA_LAB_RESCUE_COMPLETE);
    FlagClear(FLAG_VIRELIA_HIDE_LAB_ESCAPEES);
    FlagClear(FLAG_VIRELIA_MAPLEBROOK_BERRY_GIFT);
    FlagClear(FLAG_VIRELIA_MAPLEBROOK_GROVE_FIND);
    FlagSet(FLAG_VIRELIA_HIDE_DEV_GUIDE);
    FlagSet(FLAG_VIRELIA_HIDE_POLICEMAN);
    FlagClear(FLAG_SYS_POKEMON_GET);
    FlagClear(FLAG_SYS_POKEDEX_GET);
    FlagClear(FLAG_ADVENTURE_STARTED);
    VarSet(VAR_VIRELIA_PROLOGUE_STATE, VIRELIA_STATE_HOME);
    VarSet(VAR_VIRELIA_STARTER, VIRELIA_STARTER_NONE);
    VarSet(VAR_VIRELIA_RIVAL_RESULT, 0);
    SetLastHealLocationWarp(HEAL_LOCATION_VIRELIA_MAPLEBROOK_PLAYERS_HOUSE);
    SetWarpDestination(MAP_GROUP(MAP_VIRELIA_MAPLEBROOK_PLAYERS_HOUSE),
                       MAP_NUM(MAP_VIRELIA_MAPLEBROOK_PLAYERS_HOUSE), WARP_ID_NONE, 5, 6);
    WarpIntoMap();
#endif
}

u32 GetVireliaStarterCaseMask(void)
{
    return (!FlagGet(FLAG_VIRELIA_HIDE_GRASS_STARTER) << VIRELIA_STARTER_GRASS)
         | (!FlagGet(FLAG_VIRELIA_HIDE_WATER_STARTER) << VIRELIA_STARTER_WATER)
         | (!FlagGet(FLAG_VIRELIA_HIDE_FIRE_STARTER) << VIRELIA_STARTER_FIRE);
}

#if !IS_FRLG
static void SyncLabStarterCase(void)
{
    if (gSaveBlock1Ptr->location.mapGroup != MAP_GROUP(MAP_VIRELIA_MAPLEBROOK_RESEARCH_LABORATORY)
     || gSaveBlock1Ptr->location.mapNum != MAP_NUM(MAP_VIRELIA_MAPLEBROOK_RESEARCH_LABORATORY)
     || gBackupMapLayout.map == NULL)
        return;

    u32 base = VIRELIA_LAB_CASE_BASE_METATILE + GetVireliaStarterCaseMask() * 4;
    for (u32 y = 0; y < 2; y++)
        for (u32 x = 0; x < 2; x++)
            MapGridSetMetatileIdAt(VIRELIA_LAB_CASE_X + x + MAP_OFFSET,
                                  VIRELIA_LAB_CASE_Y + y + MAP_OFFSET,
                                  (base + y * 2 + x) | MAPGRID_IMPASSABLE);
}
#endif

void VireliaPrologue_SyncStarterBallFlags(void)
{
#if VIRELIA_PROLOGUE && !VIRELIA_DEVELOPMENT_MODE && !IS_FRLG
    static const u16 sBallFlags[VIRELIA_STARTER_COUNT] =
    {
        [VIRELIA_STARTER_GRASS] = FLAG_VIRELIA_HIDE_GRASS_STARTER,
        [VIRELIA_STARTER_WATER] = FLAG_VIRELIA_HIDE_WATER_STARTER,
        [VIRELIA_STARTER_FIRE] = FLAG_VIRELIA_HIDE_FIRE_STARTER,
    };
    u32 choice = VarGet(VAR_VIRELIA_STARTER);
    if (FlagGet(FLAG_VIRELIA_STARTER_RECEIVED) && choice < ARRAY_COUNT(sBallFlags))
    {
        // Empty slots persist while the field reloads between the rescue and duel.
        // The rival's pickup happens before either battle is complete.
        FlagSet(sBallFlags[choice]);
        if (FlagGet(FLAG_VIRELIA_RIVAL_COMPLETE))
            FlagSet(sBallFlags[sRivalChoices[choice]]);
    }
#else
    // The development exhibition always offers all three starter previews.
    FlagSet(FLAG_VIRELIA_HIDE_LAB_ESCAPEES);
    FlagClear(FLAG_VIRELIA_HIDE_GRASS_STARTER);
    FlagClear(FLAG_VIRELIA_HIDE_WATER_STARTER);
    FlagClear(FLAG_VIRELIA_HIDE_FIRE_STARTER);
#endif
#if !IS_FRLG
    SyncLabStarterCase();
#endif
}

void VireliaPrologue_MeetMom(void)
{
    if (!FlagGet(FLAG_VIRELIA_MOM_MET))
    {
        FlagSet(FLAG_VIRELIA_MOM_MET);
        VarSet(VAR_VIRELIA_PROLOGUE_STATE, VIRELIA_STATE_MOM);
    }
    gSpecialVar_Result = TRUE;
}

void VireliaPrologue_GiveStarter(void)
{
    u32 species = GetVireliaStarterSpecies(gSpecialVar_0x8004);
    gSpecialVar_Result = FALSE;
    if (species == SPECIES_NONE
     || !FlagGet(FLAG_VIRELIA_MOM_MET)
     || FlagGet(FLAG_VIRELIA_STARTER_RECEIVED)
     || gPartiesCount[B_TRAINER_PLAYER] != 0)
        return;

    // Use the ordinary gift routine for local OT, met data, Pokédex ownership,
    // move initialization and checksum. A starter must stay in the party.
    if (ScriptGiveMon(species, VIRELIA_STARTER_LEVEL, ITEM_NONE) != MON_GIVEN_TO_PARTY)
        return;
    VarSet(VAR_VIRELIA_STARTER, gSpecialVar_0x8004);
    VarSet(VAR_VIRELIA_PROLOGUE_STATE, VIRELIA_STATE_STARTER);
    FlagSet(FLAG_VIRELIA_STARTER_RECEIVED);
    FlagSet(FLAG_SYS_POKEMON_GET);
    gSpecialVar_Result = TRUE;
}

u32 GetVireliaRescuePartner(u32 choice)
{
    static const u16 partners[2][VIRELIA_STARTER_COUNT] =
    {
        [MALE] =
        {
            TRAINER_PARTNER(PARTNER_VIRELIA_VIKTOR_GRASS),
            TRAINER_PARTNER(PARTNER_VIRELIA_VIKTOR_WATER),
            TRAINER_PARTNER(PARTNER_VIRELIA_VIKTOR_FIRE),
        },
        [FEMALE] =
        {
            TRAINER_PARTNER(PARTNER_VIRELIA_VIKTORIE_GRASS),
            TRAINER_PARTNER(PARTNER_VIRELIA_VIKTORIE_WATER),
            TRAINER_PARTNER(PARTNER_VIRELIA_VIKTORIE_FIRE),
        },
    };
    return choice < VIRELIA_STARTER_COUNT ? partners[GetVireliaRivalGender()][choice] : 0;
}

void VireliaPrologue_PrepareRescue(void)
{
    u32 choice = VarGet(VAR_VIRELIA_STARTER);
    gSpecialVar_Result = FALSE;
    if (!FlagGet(FLAG_VIRELIA_STARTER_RECEIVED)
     || FlagGet(FLAG_VIRELIA_LAB_RESCUE_COMPLETE)
     || choice >= VIRELIA_STARTER_COUNT
     || gPartiesCount[B_TRAINER_PLAYER] != 1
     || GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_SPECIES) != GetVireliaStarterSpecies(choice))
        return;

    CreateScriptedDoubleWildMon(SPECIES_MEOWTH, VIRELIA_LAB_WILD_LEVEL, ITEM_NONE,
                               SPECIES_RATTATA, VIRELIA_LAB_WILD_LEVEL, ITEM_NONE);
    gPartiesCount[B_TRAINER_OPPONENT_A] = 2;
    gPartnerTrainerId = GetVireliaRescuePartner(choice);
    gSpecialVar_Result = TRUE;
}

void VireliaPrologue_StartRescueBattle(void)
{
    gSpecialVar_0x8005 = MULTI_BATTLE_2_VS_WILD;
    BattleSetup_StartMultiBattle();
}

void VireliaPrologue_FinishRescue(void)
{
    gSpecialVar_Result = FALSE;
    if (!FlagGet(FLAG_VIRELIA_STARTER_RECEIVED)
     || (gBattleOutcome != B_OUTCOME_WON
      && gBattleOutcome != B_OUTCOME_LOST
      && gBattleOutcome != B_OUTCOME_DREW))
        return;

    // The player's starter remains their own mon, including earned experience.
    // The subsequent trainer duel must have no temporary ally state.
    HealPlayerParty();
    ZeroPartyMons(gParties[B_TRAINER_PARTNER]);
    gPartiesCount[B_TRAINER_PARTNER] = 0;
    gPartnerTrainerId = 0;
    gBattleTypeFlags = 0;
    gBattleScripting.specialTrainerBattleType = 0;
    gSpecialVar_0x8005 = 0;
    FlagSet(FLAG_VIRELIA_LAB_RESCUE_COMPLETE);
    FlagSet(FLAG_VIRELIA_HIDE_LAB_ESCAPEES);
    gSpecialVar_Result = TRUE;
}

void VireliaPrologue_MatchRivalLevel(struct Pokemon *party, u32 trainerId)
{
#if VIRELIA_PROLOGUE && !VIRELIA_DEVELOPMENT_MODE && !IS_FRLG
    switch (trainerId)
    {
    case TRAINER_VIRELIA_VIKTOR_GRASS:
    case TRAINER_VIRELIA_VIKTOR_WATER:
    case TRAINER_VIRELIA_VIKTOR_FIRE:
    case TRAINER_VIRELIA_VIKTORIE_GRASS:
    case TRAINER_VIRELIA_VIKTORIE_WATER:
    case TRAINER_VIRELIA_VIKTORIE_FIRE:
        break;
    default:
        return;
    }

    u32 choice = VarGet(VAR_VIRELIA_STARTER);
    if (!FlagGet(FLAG_VIRELIA_STARTER_RECEIVED)
     || !FlagGet(FLAG_VIRELIA_LAB_RESCUE_COMPLETE)
     || FlagGet(FLAG_VIRELIA_RIVAL_COMPLETE)
     || choice >= VIRELIA_STARTER_COUNT
     || gPartiesCount[B_TRAINER_PLAYER] != 1
     || GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_SPECIES) != GetVireliaStarterSpecies(choice)
     || GetMonData(&party[0], MON_DATA_SPECIES) != GetVireliaRivalSpecies(choice))
        return;

    // AI partners receive no EXP. Give the following duel the same level gain
    // as the player's starter, including experience earned before a rescue retry.
    u32 level = max(VIRELIA_RIVAL_LEVEL, GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_LEVEL));
    if (level > GetMonData(&party[0], MON_DATA_LEVEL))
    {
        u32 species = GetMonData(&party[0], MON_DATA_SPECIES);
        u32 exp = gExperienceTables[gSpeciesInfo[species].growthRate][level];
        SetMonData(&party[0], MON_DATA_EXP, &exp);
        CalculateMonStats(&party[0]);
    }
#endif
}

void VireliaPrologue_CompleteRival(void)
{
    gSpecialVar_Result = FALSE;
    if (!FlagGet(FLAG_VIRELIA_STARTER_RECEIVED))
        return;
    if (FlagGet(FLAG_VIRELIA_RIVAL_COMPLETE))
    {
        gSpecialVar_Result = TRUE;
        return;
    }
    if (gBattleOutcome != B_OUTCOME_WON
     && gBattleOutcome != B_OUTCOME_LOST
     && gBattleOutcome != B_OUTCOME_DREW)
        return;

    VarSet(VAR_VIRELIA_RIVAL_RESULT, gBattleOutcome);
    FlagSet(FLAG_VIRELIA_RIVAL_COMPLETE);
    VarSet(VAR_VIRELIA_PROLOGUE_STATE, VIRELIA_STATE_RIVAL);
    HealPlayerParty();
    gSpecialVar_Result = TRUE;
}

void VireliaPrologue_FinishRival(void)
{
    VireliaPrologue_CompleteRival();
}

void VireliaPrologue_GrantBasics(void)
{
    gSpecialVar_Result = FALSE;
    if (!FlagGet(FLAG_VIRELIA_RIVAL_COMPLETE))
        return;
    if (FlagGet(FLAG_VIRELIA_SUPPLIES_RECEIVED))
    {
        gSpecialVar_Result = TRUE;
        return;
    }
    // Separate pockets make these checks independent. Do not partially grant a
    // bundle when either pocket is full; the professor can retry after space is made.
    if (!CheckBagHasSpace(ITEM_POKE_BALL, VIRELIA_STARTING_POKE_BALLS)
     || !CheckBagHasSpace(ITEM_POTION, VIRELIA_STARTING_POTIONS))
        return;
    AddBagItem(ITEM_POKE_BALL, VIRELIA_STARTING_POKE_BALLS);
    AddBagItem(ITEM_POTION, VIRELIA_STARTING_POTIONS);
    FlagSet(FLAG_VIRELIA_SUPPLIES_RECEIVED);
    FlagSet(FLAG_SYS_POKEDEX_GET);
    EnableNationalPokedex();
    FlagSet(FLAG_ADVENTURE_STARTED);
    gSpecialVar_Result = TRUE;
}

void VireliaPrologue_GrantSupplies(void)
{
    VireliaPrologue_GrantBasics();
}

void VireliaPrologue_FinishRocket(void)
{
    gSpecialVar_Result = FALSE;
    if (!FlagGet(FLAG_VIRELIA_RIVAL_COMPLETE)
     || !FlagGet(FLAG_VIRELIA_SUPPLIES_RECEIVED))
        return;
    if (FlagGet(FLAG_VIRELIA_ROCKET_DEFEATED))
    {
        gSpecialVar_Result = TRUE;
        return;
    }
    if (gBattleOutcome != B_OUTCOME_WON)
        return;
    FlagSet(FLAG_VIRELIA_ROCKET_DEFEATED);
    VarSet(VAR_VIRELIA_PROLOGUE_STATE, VIRELIA_STATE_ROCKET);
    gSpecialVar_Result = TRUE;
}

void VireliaPrologue_Complete(void)
{
    gSpecialVar_Result = FALSE;
    if (!FlagGet(FLAG_VIRELIA_MOM_MET)
     || !FlagGet(FLAG_VIRELIA_STARTER_RECEIVED)
     || !FlagGet(FLAG_VIRELIA_RIVAL_COMPLETE)
     || !FlagGet(FLAG_VIRELIA_SUPPLIES_RECEIVED)
     || !FlagGet(FLAG_VIRELIA_ROCKET_DEFEATED))
        return;
    FlagSet(FLAG_VIRELIA_PROLOGUE_COMPLETE);
    VarSet(VAR_VIRELIA_PROLOGUE_STATE, VIRELIA_STATE_COMPLETE);
    gSpecialVar_Result = TRUE;
}
