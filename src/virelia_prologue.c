#include "global.h"
#include "battle.h"
#include "event_data.h"
#include "item.h"
#include "overworld.h"
#include "pokemon.h"
#include "script_pokemon_util.h"
#include "virelia_prologue.h"
#include "constants/heal_locations.h"
#include "constants/items.h"
#include "constants/maps.h"
#include "constants/oraculum_duo_test.h"
#include "constants/virelia_prologue.h"

u32 GetVireliaStarterSpecies(u32 choice)
{
#if P_VIRELIA_STARTERS
    static const u16 sSpecies[VIRELIA_STARTER_COUNT] =
    {
        [VIRELIA_STARTER_GRASS] = SPECIES_POOCHYENA_VIRELIA,
        [VIRELIA_STARTER_WATER] = SPECIES_SPINARAK_VIRELIA,
        [VIRELIA_STARTER_FIRE] = SPECIES_EKANS_VIRELIA,
    };
    if (choice < ARRAY_COUNT(sSpecies))
        return sSpecies[choice];
#endif
    return SPECIES_NONE;
}

u32 GetVireliaRivalSpecies(u32 choice)
{
    // Grass -> Fire, Water -> Grass, Fire -> Water, all at level 5.
    static const u8 sRivalChoices[VIRELIA_STARTER_COUNT] =
    {
        [VIRELIA_STARTER_GRASS] = VIRELIA_STARTER_FIRE,
        [VIRELIA_STARTER_WATER] = VIRELIA_STARTER_GRASS,
        [VIRELIA_STARTER_FIRE] = VIRELIA_STARTER_WATER,
    };
    if (choice < ARRAY_COUNT(sRivalChoices))
        return GetVireliaStarterSpecies(sRivalChoices[choice]);
    return SPECIES_NONE;
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

    for (u32 flag = FLAG_VIRELIA_MOM_MET; flag <= FLAG_VIRELIA_PROFESSOR_MET; flag++)
        FlagClear(flag);
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
    if (gBattleOutcome != B_OUTCOME_WON && gBattleOutcome != B_OUTCOME_LOST)
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
