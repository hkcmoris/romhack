#include "global.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "fieldmap.h"
#include "field_weather.h"
#include "overworld.h"
#include "pokemon.h"
#include "script_pokemon_util.h"
#include "sprite.h"
#include "test/test.h"
#include "virelia_prologue.h"
#include "constants/event_objects.h"
#include "constants/maps.h"
#include "constants/virelia_prologue.h"

#if !IS_FRLG && P_VIRELIA_STARTERS

static void InitPlayerFollowerTest(void)
{
    InitEventData();
    ZeroPlayerPartyMons();
    ResetSpriteData();
    FreeAllSpritePalettes();
    ResetPreservedPalettesInWeather();
    memset(gObjectEvents, 0, sizeof(gObjectEvents));
    memset(&gPlayerAvatar, 0, sizeof(gPlayerAvatar));
    gSaveBlock1Ptr->location.mapGroup = MAP_GROUP(MAP_VIRELIA_MAPLEBROOK_RESEARCH_LABORATORY);
    gSaveBlock1Ptr->location.mapNum = MAP_NUM(MAP_VIRELIA_MAPLEBROOK_RESEARCH_LABORATORY);
    gSaveBlock1Ptr->pos.x = 6;
    gSaveBlock1Ptr->pos.y = 9;
    gMapHeader = *Overworld_GetMapHeaderByGroupAndId(gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum);
    gObjectEvents[0].active = TRUE;
    gObjectEvents[0].isPlayer = TRUE;
    gObjectEvents[0].localId = LOCALID_PLAYER;
    gObjectEvents[0].currentElevation = 3;
    gObjectEvents[0].currentCoords.x = gSaveBlock1Ptr->pos.x + MAP_OFFSET;
    gObjectEvents[0].currentCoords.y = gSaveBlock1Ptr->pos.y + MAP_OFFSET;
    VarSet(VAR_VIRELIA_STARTER, VIRELIA_STARTER_NONE);
}

static void FinishPlayerFollowerTest(void)
{
    RemoveFollowingPokemon();
    ClearSpriteCopyRequests();
    ResetSpriteData();
    FreeAllSpritePalettes();
    ResetPreservedPalettesInWeather();
    memset(gObjectEvents, 0, sizeof(gObjectEvents));
    memset(&gPlayerAvatar, 0, sizeof(gPlayerAvatar));
}

TEST("Virelia player followers spawn for each received starter and return after hiding or map reload")
{
    u32 choice = VIRELIA_STARTER_GRASS;
    for (u32 i = 0; i < VIRELIA_STARTER_COUNT; i++)
        PARAMETRIZE { choice = i; }

    InitPlayerFollowerTest();
    UpdateFollowingPokemon();
    EXPECT_EQ(GetFollowerObject(), NULL);
    VireliaPrologue_MeetMom();
    gSpecialVar_0x8004 = choice;
    VireliaPrologue_GiveStarter();
    EXPECT_EQ(gSpecialVar_Result, TRUE);

    UpdateFollowingPokemon();
    struct ObjectEvent *follower = GetFollowerObject();
    EXPECT_NE(follower, NULL);
    EXPECT_EQ(OW_SPECIES(follower), GetVireliaStarterSpecies(choice));
    EXPECT_EQ(follower->movementType, MOVEMENT_TYPE_FOLLOW_PLAYER);
    EXPECT_EQ((u32)follower->currentElevation, 3);
    EXPECT(follower->invisible);
    EXPECT(gSprites[follower->spriteId].inUse);
    EXPECT_NE(SpeciesToGraphicsInfo(OW_SPECIES(follower), FALSE, FALSE),
              SpeciesToGraphicsInfo(SPECIES_NONE, FALSE, FALSE));

    // Refreshing the field must retain the one existing follower object.
    UpdateFollowingPokemon();
    EXPECT_EQ(GetFollowerObject(), follower);
    FlagSet(FLAG_TEMP_HIDE_FOLLOWER);
    UpdateFollowingPokemon();
    EXPECT_EQ(GetFollowerObject(), NULL);
    FlagClear(FLAG_TEMP_HIDE_FOLLOWER);
    UpdateFollowingPokemon();
    EXPECT_NE(GetFollowerObject(), NULL);
    EXPECT_EQ(OW_SPECIES(GetFollowerObject()), GetVireliaStarterSpecies(choice));

    // Map transitions recreate the follower from the current party.
    RemoveFollowingPokemon();
    gSaveBlock1Ptr->location.mapNum = MAP_NUM(MAP_VIRELIA_MAPLEBROOK_TOWN);
    gSaveBlock1Ptr->location.mapGroup = MAP_GROUP(MAP_VIRELIA_MAPLEBROOK_TOWN);
    gMapHeader = *Overworld_GetMapHeaderByGroupAndId(gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum);
    UpdateFollowingPokemon();
    EXPECT_NE(GetFollowerObject(), NULL);
    EXPECT_EQ(OW_SPECIES(GetFollowerObject()), GetVireliaStarterSpecies(choice));
    EXPECT_EQ(GetFollowerObject()->mapNum, MAP_NUM(MAP_VIRELIA_MAPLEBROOK_TOWN));
    EXPECT_EQ(GetFollowerObject()->mapGroup, MAP_GROUP(MAP_VIRELIA_MAPLEBROOK_TOWN));
    FinishPlayerFollowerTest();
}

TEST("Virelia player followers use party order and skip fainted Pokemon and eggs")
{
    InitPlayerFollowerTest();
    CreateRandomMonWithIVs(&gParties[B_TRAINER_PLAYER][0], SPECIES_POOCHYENA_VIRELIA, 5, 10);
    CreateRandomMonWithIVs(&gParties[B_TRAINER_PLAYER][1], SPECIES_NATU_VIRELIA, 5, 10);
    CreateRandomMonWithIVs(&gParties[B_TRAINER_PLAYER][2], SPECIES_EKANS_VIRELIA, 5, 10);
    gPartiesCount[B_TRAINER_PLAYER] = 3;
    EXPECT_EQ(GetFirstLiveMon(), &gParties[B_TRAINER_PLAYER][0]);

    u32 zero = 0;
    SetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_HP, &zero);
    EXPECT_EQ(GetFirstLiveMon(), &gParties[B_TRAINER_PLAYER][1]);
    bool32 isEgg = TRUE;
    SetMonData(&gParties[B_TRAINER_PLAYER][1], MON_DATA_IS_EGG, &isEgg);
    EXPECT_EQ(GetFirstLiveMon(), &gParties[B_TRAINER_PLAYER][2]);
    UpdateFollowingPokemon();
    EXPECT_NE(GetFollowerObject(), NULL);
    EXPECT_EQ(OW_SPECIES(GetFollowerObject()), SPECIES_EKANS_VIRELIA);

    struct Pokemon swapped = gParties[B_TRAINER_PLAYER][0];
    gParties[B_TRAINER_PLAYER][0] = gParties[B_TRAINER_PLAYER][2];
    gParties[B_TRAINER_PLAYER][2] = swapped;
    EXPECT_EQ(GetFirstLiveMon(), &gParties[B_TRAINER_PLAYER][0]);
    HealPlayerParty();
    UpdateFollowingPokemon();
    EXPECT_EQ(OW_SPECIES(GetFollowerObject()), SPECIES_EKANS_VIRELIA);
    swapped = gParties[B_TRAINER_PLAYER][0];
    gParties[B_TRAINER_PLAYER][0] = gParties[B_TRAINER_PLAYER][2];
    gParties[B_TRAINER_PLAYER][2] = swapped;
    UpdateFollowingPokemon();
    EXPECT_EQ(OW_SPECIES(GetFollowerObject()), SPECIES_POOCHYENA_VIRELIA);
    FinishPlayerFollowerTest();
}

TEST("Virelia player followers disappear when no healthy non-egg party member remains")
{
    InitPlayerFollowerTest();
    CreateRandomMonWithIVs(&gParties[B_TRAINER_PLAYER][0], SPECIES_NATU_VIRELIA, 5, 10);
    gPartiesCount[B_TRAINER_PLAYER] = 1;
    UpdateFollowingPokemon();
    EXPECT_NE(GetFollowerObject(), NULL);
    u32 zero = 0;
    SetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_HP, &zero);
    UpdateFollowingPokemon();
    EXPECT_EQ(GetFollowerObject(), NULL);
    EXPECT_EQ(GetFirstLiveMon(), NULL);

    HealPlayerParty();
    bool32 isEgg = TRUE;
    SetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_IS_EGG, &isEgg);
    UpdateFollowingPokemon();
    EXPECT_EQ(GetFollowerObject(), NULL);
    EXPECT_EQ(GetFirstLiveMon(), NULL);
    FinishPlayerFollowerTest();
}

#endif // !IS_FRLG && P_VIRELIA_STARTERS
