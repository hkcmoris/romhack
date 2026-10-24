#include "global.h"
#include "battle.h"
#include "event_data.h"
#include "fieldmap.h"
#include "item.h"
#include "overworld.h"
#include "script.h"
#include "test/test.h"
#include "test/overworld_script.h"
#include "constants/battle.h"
#include "constants/items.h"
#include "constants/virelia_prologue.h"
#include "constants/virelia_first_gym.h"

#if !IS_FRLG && VIRELIA_PROLOGUE && !VIRELIA_DEVELOPMENT_MODE

static void ResetTools(void)
{
    InitEventData();
    SetBagItemsPointers();
    ClearBag();
    for (u32 flag = FLAG_VIRELIA_MOM_MET; flag <= FLAG_VIRELIA_TOOL_PICK; flag++)
        FlagClear(flag);
    FlagSet(FLAG_VIRELIA_PROLOGUE_COMPLETE);
}

static void CollectTools(void)
{
    for (u32 tool = 1; tool <= 3; tool++)
    {
        gSpecialVar_0x8004 = tool;
        RUN_OVERWORLD_SCRIPT(call VireliaFirstGym_EventScript_CollectTool;);
    }
}

TEST("Virelia Missing Tools compiled events require power and the miner request")
{
    ResetTools();
    RUN_OVERWORLD_SCRIPT(call VireliaFirstGym_EventScript_BeginMissingTools;);
    EXPECT_EQ(gSpecialVar_Result, FALSE);
    EXPECT(!FlagGet(FLAG_VIRELIA_MISSING_TOOLS_STARTED));
    CollectTools();
    EXPECT(!FlagGet(FLAG_VIRELIA_TOOL_WRENCH));
    FlagSet(FLAG_VIRELIA_QUARRY_POWER_RESTORED);
    CollectTools();
    EXPECT(!FlagGet(FLAG_VIRELIA_TOOL_PICK));
    RUN_OVERWORLD_SCRIPT(call VireliaFirstGym_EventScript_BeginMissingTools;);
    EXPECT_EQ(gSpecialVar_Result, TRUE);
    CollectTools();
    EXPECT(FlagGet(FLAG_VIRELIA_TOOL_WRENCH));
    EXPECT(FlagGet(FLAG_VIRELIA_TOOL_HAMMER));
    EXPECT(FlagGet(FLAG_VIRELIA_TOOL_PICK));
    EXPECT(!FlagGet(FLAG_VIRELIA_QUARRY_EVIDENCE));
    EXPECT(!FlagGet(FLAG_VIRELIA_QUARRY_COMPLETE));
}

TEST("Virelia Missing Tools victory and capture recover tools while escape and defeat allow retry")
{
    u32 outcome = B_OUTCOME_WON;
    PARAMETRIZE { outcome = B_OUTCOME_WON; }
    PARAMETRIZE { outcome = B_OUTCOME_CAUGHT; }
    PARAMETRIZE { outcome = B_OUTCOME_LOST; }
    PARAMETRIZE { outcome = B_OUTCOME_RAN; }
    PARAMETRIZE { outcome = B_OUTCOME_PLAYER_TELEPORTED; }
    ResetTools();
    FlagSet(FLAG_VIRELIA_QUARRY_POWER_RESTORED);
    RUN_OVERWORLD_SCRIPT(call VireliaFirstGym_EventScript_BeginMissingTools;);
    gBattleOutcome = outcome;
    RUN_OVERWORLD_SCRIPT(call VireliaFirstGym_EventScript_FinishToolsBattle;);
    EXPECT_EQ(gSpecialVar_Result, FALSE);
    CollectTools();
    gBattleOutcome = outcome;
    RUN_OVERWORLD_SCRIPT(call VireliaFirstGym_EventScript_FinishToolsBattle;);
    bool32 success = outcome == B_OUTCOME_WON || outcome == B_OUTCOME_CAUGHT;
    EXPECT_EQ(gSpecialVar_Result, success);
    EXPECT_EQ(FlagGet(FLAG_VIRELIA_MISSING_TOOLS_RECOVERED), success);
    EXPECT(!FlagGet(FLAG_VIRELIA_MISSING_TOOLS_REWARDED));
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_HARD_STONE), 0);
}

TEST("Virelia Missing Tools full Bag retries the Hard Stone reward exactly once")
{
    ResetTools();
    FlagSet(FLAG_VIRELIA_QUARRY_POWER_RESTORED);
    RUN_OVERWORLD_SCRIPT(call VireliaFirstGym_EventScript_RewardTools;);
    EXPECT_EQ(gSpecialVar_Result, FALSE);
    RUN_OVERWORLD_SCRIPT(call VireliaFirstGym_EventScript_BeginMissingTools;);
    CollectTools();
    gSpecialVar_0x8004 = B_OUTCOME_CAUGHT;
    RUN_OVERWORLD_SCRIPT(call VireliaFirstGym_EventScript_RecoverTools;);
    struct BagPocket *pocket = &gBagPockets[POCKET_ITEMS];
    for (u32 i = 0; i < pocket->capacity; i++)
        BagPocket_SetSlotItemIdAndCount(pocket, i, ITEM_ANTIDOTE, MAX_BAG_ITEM_CAPACITY);
    RUN_OVERWORLD_SCRIPT(call VireliaFirstGym_EventScript_RewardTools;);
    EXPECT_EQ(gSpecialVar_Result, FALSE);
    EXPECT(!FlagGet(FLAG_VIRELIA_MISSING_TOOLS_REWARDED));
    EXPECT(FlagGet(FLAG_VIRELIA_MISSING_TOOLS_RECOVERED));
    ClearBag();
    RUN_OVERWORLD_SCRIPT(call VireliaFirstGym_EventScript_RewardTools;);
    EXPECT_EQ(gSpecialVar_Result, TRUE);
    EXPECT(FlagGet(FLAG_VIRELIA_MISSING_TOOLS_REWARDED));
    RUN_OVERWORLD_SCRIPT(call VireliaFirstGym_EventScript_RewardTools;);
    EXPECT_EQ(gSpecialVar_Result, FALSE);
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_HARD_STONE), 1);
    EXPECT(!FlagGet(FLAG_VIRELIA_QUARRY_COMPLETE));
}

TEST("Virelia Missing Tools actual map loads gate both gallery ladders on electricity")
{
    u32 entrance = TRUE, powered = FALSE;
    PARAMETRIZE { entrance = TRUE; powered = FALSE; }
    PARAMETRIZE { entrance = TRUE; powered = TRUE; }
    PARAMETRIZE { entrance = FALSE; powered = FALSE; }
    PARAMETRIZE { entrance = FALSE; powered = TRUE; }
    ResetTools();
    if (powered) FlagSet(FLAG_VIRELIA_QUARRY_POWER_RESTORED);
    u16 map = entrance ? MAP_VIRELIA_SLATEFORD_QUARRY_ENTRANCE : MAP_VIRELIA_SLATEFORD_QUARRY_DEPTHS;
    u32 x = entrance ? 4 : 3, y = entrance ? 3 : 7;
    SetWarpDestination(MAP_GROUP(map), MAP_NUM(map), WARP_ID_NONE, 7, entrance ? 7 : 10);
    WarpIntoMap();
    InitMap();
    RunOnLoadMapScript();
    EXPECT_EQ(MapGridGetCollisionAt(x + MAP_OFFSET, y + MAP_OFFSET) != 0, !powered);
    EXPECT_EQ(MapGridGetMetatileIdAt(x + MAP_OFFSET, y + MAP_OFFSET), powered ? 0x227 : 0x203);
}

#endif
