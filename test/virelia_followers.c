#include "global.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "test/test.h"
#include "test/overworld_script.h"
#include "virelia_prologue.h"
#include "constants/event_objects.h"
#include "constants/virelia_prologue.h"

static void ResetRivalPokemon(void)
{
    InitEventData();
    VarSet(VAR_VIRELIA_STARTER, VIRELIA_STARTER_NONE);
}

#if VIRELIA_PROLOGUE && !VIRELIA_DEVELOPMENT_MODE && !IS_FRLG && P_VIRELIA_STARTERS

TEST("Virelia rival Pokemon follows its case pickup without changing rival identity or rescue progress")
{
    u32 choice = VIRELIA_STARTER_GRASS;
    u32 gender = MALE;
    for (u32 i = 0; i < VIRELIA_STARTER_COUNT; i++)
        for (u32 j = MALE; j <= FEMALE; j++)
            PARAMETRIZE { choice = i; gender = j; }
    static const u16 rivalBallFlags[VIRELIA_STARTER_COUNT] =
    {
        FLAG_VIRELIA_HIDE_FIRE_STARTER,
        FLAG_VIRELIA_HIDE_GRASS_STARTER,
        FLAG_VIRELIA_HIDE_WATER_STARTER,
    };
    static const u16 playerBallFlags[VIRELIA_STARTER_COUNT] =
    {
        FLAG_VIRELIA_HIDE_GRASS_STARTER,
        FLAG_VIRELIA_HIDE_WATER_STARTER,
        FLAG_VIRELIA_HIDE_FIRE_STARTER,
    };

    ResetRivalPokemon();
    u32 previousGender = gSaveBlock2Ptr->playerGender;
    gSaveBlock2Ptr->playerGender = gender;
    VireliaPrologue_SetRivalAppearance();
    u32 rivalGraphics = gender == MALE ? OBJ_EVENT_GFX_RIVAL_MAY_NORMAL : OBJ_EVENT_GFX_RIVAL_BRENDAN_NORMAL;
    VarSet(VAR_VIRELIA_STARTER, choice);
    VireliaPrologue_SyncRivalPokemon();
    EXPECT(FlagGet(FLAG_VIRELIA_HIDE_RIVAL_POKEMON));
    EXPECT(!FlagGet(FLAG_VIRELIA_LAB_RESCUE_COMPLETE));
    EXPECT_EQ(VarGet(VAR_OBJ_GFX_ID_0), rivalGraphics);

    FlagSet(FLAG_VIRELIA_STARTER_RECEIVED);
    FlagSet(playerBallFlags[choice]);
    VireliaPrologue_SyncRivalPokemon();
    EXPECT(FlagGet(FLAG_VIRELIA_HIDE_RIVAL_POKEMON));
    EXPECT_EQ(VarGet(VAR_OBJ_GFX_ID_1), GetGraphicsIdForMon(GetVireliaRivalSpecies(choice), FALSE, FALSE));
    EXPECT_EQ(VarGet(VAR_OBJ_GFX_ID_0), rivalGraphics);

    FlagSet(rivalBallFlags[choice]);
    VireliaPrologue_SyncRivalPokemon();
    EXPECT(!FlagGet(FLAG_VIRELIA_HIDE_RIVAL_POKEMON));
    EXPECT(!FlagGet(FLAG_VIRELIA_RIVAL_COMPLETE));
    EXPECT(!FlagGet(FLAG_VIRELIA_LAB_RESCUE_COMPLETE));
    RUN_OVERWORLD_SCRIPT(callnative VireliaPrologue_BufferRivalPokemon;);
    EXPECT_EQ(gSpecialVar_0x8004, GetVireliaRivalSpecies(choice));

    // Map transitions clear temporary flags but must retain the companion,
    // both on battle return and when revisiting after the rival is complete.
    for (u32 completed = 0; completed < 2; completed++)
    {
        if (completed)
        {
            FlagSet(FLAG_VIRELIA_LAB_RESCUE_COMPLETE);
            FlagSet(FLAG_VIRELIA_RIVAL_COMPLETE);
        }
        ClearTempFieldEventData();
        VarSet(VAR_OBJ_GFX_ID_1, 0);
        VireliaPrologue_SyncRivalPokemon();
        EXPECT(!FlagGet(FLAG_VIRELIA_HIDE_RIVAL_POKEMON));
        EXPECT_EQ(VarGet(VAR_OBJ_GFX_ID_1), GetGraphicsIdForMon(GetVireliaRivalSpecies(choice), FALSE, FALSE));
        EXPECT_EQ(VarGet(VAR_OBJ_GFX_ID_0), rivalGraphics);
        EXPECT_EQ(FlagGet(FLAG_VIRELIA_LAB_RESCUE_COMPLETE), completed);
    }
    gSaveBlock2Ptr->playerGender = previousGender;
}

TEST("Virelia rival Pokemon stays hidden for invalid choices and hidden story actors")
{
    u32 choice = VIRELIA_STARTER_NONE;
    bool32 hideActors = FALSE;
    PARAMETRIZE { choice = VIRELIA_STARTER_NONE; hideActors = FALSE; }
    PARAMETRIZE { choice = VIRELIA_STARTER_COUNT; hideActors = FALSE; }
    for (u32 i = 0; i < VIRELIA_STARTER_COUNT; i++)
        PARAMETRIZE { choice = i; hideActors = TRUE; }

    ResetRivalPokemon();
    VarSet(VAR_VIRELIA_STARTER, choice);
    FlagSet(FLAG_VIRELIA_STARTER_RECEIVED);
    FlagSet(FLAG_VIRELIA_HIDE_GRASS_STARTER);
    FlagSet(FLAG_VIRELIA_HIDE_WATER_STARTER);
    FlagSet(FLAG_VIRELIA_HIDE_FIRE_STARTER);
    if (hideActors)
        FlagSet(FLAG_VIRELIA_HIDE_STORY_ACTORS);
    VireliaPrologue_SyncRivalPokemon();
    EXPECT(FlagGet(FLAG_VIRELIA_HIDE_RIVAL_POKEMON));
}

#else

TEST("Virelia nonstory exhibition never exposes the rival Pokemon")
{
    ResetRivalPokemon();
    VarSet(VAR_VIRELIA_STARTER, VIRELIA_STARTER_WATER);
    FlagSet(FLAG_VIRELIA_STARTER_RECEIVED);
    FlagSet(FLAG_VIRELIA_RIVAL_COMPLETE);
    FlagSet(FLAG_VIRELIA_HIDE_GRASS_STARTER);
    VireliaPrologue_SyncRivalPokemon();
    EXPECT(FlagGet(FLAG_VIRELIA_HIDE_RIVAL_POKEMON));
}

#endif
