#include "global.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "fieldmap.h"
#include "field_weather.h"
#include "overworld.h"
#include "palette.h"
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

#if OW_POKEMON_OBJECT_EVENTS && OW_PKMN_OBJECTS_SHARE_PALETTES == FALSE

#define DECLARE_VIRELIA_OVERWORLD(name) \
    extern const u32 gObjectEventPic_##name[]; \
    extern const u16 gOverworldPalette_##name[]; \
    extern const u16 gShinyOverworldPalette_##name[];

DECLARE_VIRELIA_OVERWORLD(PoochyenaVirelia)
DECLARE_VIRELIA_OVERWORLD(MightyenaVirelia)
DECLARE_VIRELIA_OVERWORLD(Wraithena)
DECLARE_VIRELIA_OVERWORLD(NatuVirelia)
DECLARE_VIRELIA_OVERWORLD(XatuVirelia)
DECLARE_VIRELIA_OVERWORLD(Nimbatu)
DECLARE_VIRELIA_OVERWORLD(EkansVirelia)
DECLARE_VIRELIA_OVERWORLD(ArbokVirelia)
DECLARE_VIRELIA_OVERWORLD(Infernabra)

#undef DECLARE_VIRELIA_OVERWORLD

struct VireliaOverworldExpectation
{
    enum Species species;
    enum Species original;
    const u32 *tiles;
    const u16 *normalPalette;
    const u16 *shinyPalette;
};

static const struct VireliaOverworldExpectation sVireliaOverworldExpectations[] =
{
    { SPECIES_POOCHYENA_VIRELIA, SPECIES_POOCHYENA, gObjectEventPic_PoochyenaVirelia, gOverworldPalette_PoochyenaVirelia, gShinyOverworldPalette_PoochyenaVirelia },
    { SPECIES_MIGHTYENA_VIRELIA, SPECIES_MIGHTYENA, gObjectEventPic_MightyenaVirelia, gOverworldPalette_MightyenaVirelia, gShinyOverworldPalette_MightyenaVirelia },
    { SPECIES_WRAITHENA, SPECIES_MIGHTYENA, gObjectEventPic_Wraithena, gOverworldPalette_Wraithena, gShinyOverworldPalette_Wraithena },
    { SPECIES_NATU_VIRELIA, SPECIES_NATU, gObjectEventPic_NatuVirelia, gOverworldPalette_NatuVirelia, gShinyOverworldPalette_NatuVirelia },
    { SPECIES_XATU_VIRELIA, SPECIES_XATU, gObjectEventPic_XatuVirelia, gOverworldPalette_XatuVirelia, gShinyOverworldPalette_XatuVirelia },
    { SPECIES_NIMBATU, SPECIES_XATU, gObjectEventPic_Nimbatu, gOverworldPalette_Nimbatu, gShinyOverworldPalette_Nimbatu },
    { SPECIES_EKANS_VIRELIA, SPECIES_EKANS, gObjectEventPic_EkansVirelia, gOverworldPalette_EkansVirelia, gShinyOverworldPalette_EkansVirelia },
    { SPECIES_ARBOK_VIRELIA, SPECIES_ARBOK, gObjectEventPic_ArbokVirelia, gOverworldPalette_ArbokVirelia, gShinyOverworldPalette_ArbokVirelia },
    { SPECIES_INFERNABRA, SPECIES_ARBOK, gObjectEventPic_Infernabra, gOverworldPalette_Infernabra, gShinyOverworldPalette_Infernabra },
};

TEST("Virelia overworld art uses dedicated graphics and palettes for all nine stages")
{
    u32 index = 0;
    for (u32 i = 0; i < ARRAY_COUNT(sVireliaOverworldExpectations); i++)
        PARAMETRIZE { index = i; }

    const struct VireliaOverworldExpectation *expected = &sVireliaOverworldExpectations[index];
    const struct SpeciesInfo *species = &gSpeciesInfo[expected->species];
    const struct SpeciesInfo *original = &gSpeciesInfo[expected->original];
    const struct ObjectEventGraphicsInfo *graphics = SpeciesToGraphicsInfo(expected->species, FALSE, FALSE);
    EXPECT_EQ(graphics, &species->overworldData);
    EXPECT_NE(graphics->images, original->overworldData.images);
    EXPECT_EQ(graphics->images[0].data, expected->tiles);
    EXPECT_NE(graphics->images[0].data, original->overworldData.images[0].data);
    EXPECT_EQ(graphics->images[0].size, 32 * 32 / 2);
    EXPECT(graphics->images[0].relativeFrames);
    EXPECT_EQ(graphics->width, 32);
    EXPECT_EQ(graphics->height, 32);
    EXPECT_EQ((u32)graphics->compressed, OW_GFX_COMPRESS);
    EXPECT_EQ(species->overworldPalette, expected->normalPalette);
    EXPECT_EQ(species->overworldShinyPalette, expected->shinyPalette);
    EXPECT_NE(species->overworldPalette, original->overworldPalette);
    EXPECT_NE(species->overworldShinyPalette, original->overworldShinyPalette);
    EXPECT_NE(species->overworldPalette, species->palette);
    EXPECT_NE(species->overworldShinyPalette, species->shinyPalette);

    // Neither gender nor shininess may fall back to the original form's art.
    for (u32 shiny = FALSE; shiny <= TRUE; shiny++)
        for (u32 female = FALSE; female <= TRUE; female++)
            EXPECT_EQ(SpeciesToGraphicsInfo(expected->species, shiny, female), graphics);

    for (u32 i = 0; i < ARRAY_COUNT(sVireliaOverworldExpectations); i++)
    {
        if (i == index)
            continue;
        const struct SpeciesInfo *other = &gSpeciesInfo[sVireliaOverworldExpectations[i].species];
        EXPECT_NE(graphics->images[0].data, other->overworldData.images[0].data);
        EXPECT_NE(species->overworldPalette, other->overworldPalette);
        EXPECT_NE(species->overworldShinyPalette, other->overworldShinyPalette);
    }
}

TEST("Virelia player followers load their dedicated normal and shiny overworld palettes")
{
    u32 index = 0;
    bool32 shiny = FALSE;
    for (u32 i = 0; i < ARRAY_COUNT(sVireliaOverworldExpectations); i++)
    {
        PARAMETRIZE { index = i; shiny = FALSE; }
        PARAMETRIZE { index = i; shiny = TRUE; }
    }

    const struct VireliaOverworldExpectation *expected = &sVireliaOverworldExpectations[index];
    const u16 *palette = shiny ? expected->shinyPalette : expected->normalPalette;
    InitPlayerFollowerTest();
    CreateMonWithIVs(&gParties[B_TRAINER_PLAYER][0], expected->species, 50,
                    shiny ? 0 : 0x100, OTID_STRUCT_PRESET(0), 10);
    gPartiesCount[B_TRAINER_PLAYER] = 1;
    EXPECT_EQ(IsMonShiny(&gParties[B_TRAINER_PLAYER][0]), shiny);

    UpdateFollowingPokemon();
    struct ObjectEvent *follower = GetFollowerObject();
    EXPECT_NE(follower, NULL);
    EXPECT_EQ(OW_SPECIES(follower), expected->species);
    EXPECT_EQ((u32)follower->shiny, shiny);
    struct Sprite *sprite = &gSprites[follower->spriteId];
    EXPECT(sprite->inUse);
    EXPECT_EQ(sprite->images, gSpeciesInfo[expected->species].overworldData.images);
    const u32 paletteSlot = IndexOfSpritePaletteTag(GetGraphicsIdForMon(expected->species, shiny, FALSE));
    EXPECT_LT(paletteSlot, 16);
    EXPECT_EQ((u32)sprite->oam.paletteNum, paletteSlot);
    for (u32 color = 0; color < 16; color++)
        EXPECT_EQ(gPlttBufferUnfaded[OBJ_PLTT_ID(paletteSlot) + color], palette[color]);
    FinishPlayerFollowerTest();
}

#endif // OW_POKEMON_OBJECT_EVENTS && OW_PKMN_OBJECTS_SHARE_PALETTES == FALSE

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
