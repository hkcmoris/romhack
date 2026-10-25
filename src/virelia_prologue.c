#include "global.h"
#include "battle.h"
#include "event_data.h"
#include "event_object_movement.h"
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

// The rival takes the starter with a type advantage against the player's choice.
static const u8 sRivalChoices[VIRELIA_STARTER_COUNT] =
{
    [VIRELIA_STARTER_GRASS] = VIRELIA_STARTER_FIRE,
    [VIRELIA_STARTER_WATER] = VIRELIA_STARTER_GRASS,
    [VIRELIA_STARTER_FIRE] = VIRELIA_STARTER_WATER,
};

#if VIRELIA_PROLOGUE && !VIRELIA_DEVELOPMENT_MODE && !IS_FRLG
static const u16 sStarterBallFlags[VIRELIA_STARTER_COUNT] =
{
    [VIRELIA_STARTER_GRASS] = FLAG_VIRELIA_HIDE_GRASS_STARTER,
    [VIRELIA_STARTER_WATER] = FLAG_VIRELIA_HIDE_WATER_STARTER,
    [VIRELIA_STARTER_FIRE] = FLAG_VIRELIA_HIDE_FIRE_STARTER,
};
#endif

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
    FlagSet(FLAG_VIRELIA_HIDE_DEV_GUIDE);
    FlagSet(FLAG_VIRELIA_HIDE_POLICEMAN);
    FlagSet(FLAG_VIRELIA_HIDE_RIVAL_POKEMON);
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

#if !IS_FRLG
static void SyncLabStarterBallObjects(void)
{
    if (gSaveBlock1Ptr->location.mapGroup != MAP_GROUP(MAP_VIRELIA_MAPLEBROOK_RESEARCH_LABORATORY)
     || gSaveBlock1Ptr->location.mapNum != MAP_NUM(MAP_VIRELIA_MAPLEBROOK_RESEARCH_LABORATORY)
     || gMapHeader.events == NULL)
        return;

    for (u32 i = 0; i < gMapHeader.events->objectEventCount; i++)
    {
        const struct ObjectEventTemplate *mapTemplate = &gMapHeader.events->objectEvents[i];
        u8 objectEventId;
        if (mapTemplate->localId != LOCALID_VIRELIA_LAB_GRASS_STARTER
         && mapTemplate->localId != LOCALID_VIRELIA_LAB_WATER_STARTER
         && mapTemplate->localId != LOCALID_VIRELIA_LAB_FIRE_STARTER)
            continue;

        // Continue refreshes saved script pointers, but retains old flag IDs.
        // Repair only the three displays; preserve NPC positions and movement.
        for (u32 j = 0; j < OBJECT_EVENT_TEMPLATES_COUNT; j++)
            if (gSaveBlock1Ptr->objectEventTemplates[j].localId == mapTemplate->localId)
                gSaveBlock1Ptr->objectEventTemplates[j].flagId = mapTemplate->flagId;

        // This helper runs during ON_LOAD, before field sprites are created.
        // Continue respawns active saved objects without checking hide flags,
        // so clear their object state without touching stale sprite IDs.
        if (FlagGet(mapTemplate->flagId)
         && !TryGetObjectEventIdByLocalIdAndMap(mapTemplate->localId,
                                               gSaveBlock1Ptr->location.mapNum,
                                               gSaveBlock1Ptr->location.mapGroup,
                                               &objectEventId))
            ClearObjectEvent(&gObjectEvents[objectEventId]);
    }
}
#endif

void VireliaPrologue_SyncStarterBallFlags(void)
{
#if VIRELIA_PROLOGUE && !VIRELIA_DEVELOPMENT_MODE && !IS_FRLG
    u32 choice = VarGet(VAR_VIRELIA_STARTER);
    if (FlagGet(FLAG_VIRELIA_STARTER_RECEIVED) && choice < ARRAY_COUNT(sStarterBallFlags))
    {
        // Recover visibility for older saves. Never clear a story flag here:
        // the rival's pickup persists before the battle is completed.
        FlagSet(sStarterBallFlags[choice]);
        if (FlagGet(FLAG_VIRELIA_RIVAL_COMPLETE))
            FlagSet(sStarterBallFlags[sRivalChoices[choice]]);
    }
#else
    // The development exhibition always offers all three starter previews.
    FlagClear(FLAG_VIRELIA_HIDE_GRASS_STARTER);
    FlagClear(FLAG_VIRELIA_HIDE_WATER_STARTER);
    FlagClear(FLAG_VIRELIA_HIDE_FIRE_STARTER);
#endif
#if !IS_FRLG
    SyncLabStarterBallObjects();
#endif
}

void VireliaPrologue_SyncRivalPokemon(void)
{
    FlagSet(FLAG_VIRELIA_HIDE_RIVAL_POKEMON);
#if VIRELIA_PROLOGUE && !VIRELIA_DEVELOPMENT_MODE && !IS_FRLG
    u32 choice = VarGet(VAR_VIRELIA_STARTER);
    u32 species = GetVireliaRivalSpecies(choice);

    if (species == SPECIES_NONE
     || !FlagGet(FLAG_VIRELIA_STARTER_RECEIVED)
     || FlagGet(FLAG_VIRELIA_HIDE_STORY_ACTORS))
        return;

    VarSet(VAR_OBJ_GFX_ID_0, GetGraphicsIdForMon(species, FALSE, FALSE));
    // Removing Viktor's chosen Ball records the pickup before the battle.
    // His companion remains present when the field is restored after battle.
    if (FlagGet(sStarterBallFlags[sRivalChoices[choice]]))
        FlagClear(FLAG_VIRELIA_HIDE_RIVAL_POKEMON);
#endif
}

void VireliaPrologue_BufferRivalPokemon(void)
{
    gSpecialVar_0x8004 = GetVireliaRivalSpecies(VarGet(VAR_VIRELIA_STARTER));
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
