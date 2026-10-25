#include "global.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "test/test.h"
#include "test/overworld_script.h"
#include "virelia_prologue.h"
#include "constants/virelia_prologue.h"

static void ResetRivalPokemon(void)
{
    InitEventData();
    VarSet(VAR_VIRELIA_STARTER, VIRELIA_STARTER_NONE);
}

#if VIRELIA_PROLOGUE && !VIRELIA_DEVELOPMENT_MODE && !IS_FRLG && P_VIRELIA_STARTERS

TEST("Virelia rival Pokemon appears only after its own starter Ball is picked up")
{
    u32 choice = VIRELIA_STARTER_GRASS;
    for (u32 i = 0; i < VIRELIA_STARTER_COUNT; i++)
        PARAMETRIZE { choice = i; }
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
    VarSet(VAR_VIRELIA_STARTER, choice);
    VireliaPrologue_SyncRivalPokemon();
    EXPECT(FlagGet(FLAG_VIRELIA_HIDE_RIVAL_POKEMON));

    FlagSet(FLAG_VIRELIA_STARTER_RECEIVED);
    FlagSet(playerBallFlags[choice]);
    VireliaPrologue_SyncRivalPokemon();
    EXPECT(FlagGet(FLAG_VIRELIA_HIDE_RIVAL_POKEMON));
    EXPECT_EQ(VarGet(VAR_OBJ_GFX_ID_0), GetGraphicsIdForMon(GetVireliaRivalSpecies(choice), FALSE, FALSE));

    FlagSet(rivalBallFlags[choice]);
    VireliaPrologue_SyncRivalPokemon();
    EXPECT(!FlagGet(FLAG_VIRELIA_HIDE_RIVAL_POKEMON));
    EXPECT(!FlagGet(FLAG_VIRELIA_RIVAL_COMPLETE));
    RUN_OVERWORLD_SCRIPT(callnative VireliaPrologue_BufferRivalPokemon;);
    EXPECT_EQ(gSpecialVar_0x8004, GetVireliaRivalSpecies(choice));

    // Map transitions clear temporary flags but must retain the companion,
    // both on battle return and when revisiting after the rival is complete.
    for (u32 completed = 0; completed < 2; completed++)
    {
        if (completed)
            FlagSet(FLAG_VIRELIA_RIVAL_COMPLETE);
        ClearTempFieldEventData();
        VarSet(VAR_OBJ_GFX_ID_0, 0);
        VireliaPrologue_SyncRivalPokemon();
        EXPECT(!FlagGet(FLAG_VIRELIA_HIDE_RIVAL_POKEMON));
        EXPECT_EQ(VarGet(VAR_OBJ_GFX_ID_0), GetGraphicsIdForMon(GetVireliaRivalSpecies(choice), FALSE, FALSE));
    }
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
