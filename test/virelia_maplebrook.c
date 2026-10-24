#include "global.h"
#include "event_data.h"
#include "fieldmap.h"
#include "item.h"
#include "overworld.h"
#include "test/test.h"
#include "test/overworld_script.h"
#include "constants/virelia_maplebrook.h"
#include "constants/virelia_prologue.h"

#if !IS_FRLG

static void ResetTownRewards(void)
{
    InitEventData();
    SetBagItemsPointers();
    ClearBag();
}

static void TryTownReward(bool32 berry)
{
    if (berry)
        RUN_OVERWORLD_SCRIPT(call Virelia_MaplebrookCottage_EventScript_TryGiveBerry;);
    else
        RUN_OVERWORLD_SCRIPT(call Virelia_MaplebrookTown_EventScript_TryFindAntidote;);
}

TEST("Virelia Maplebrook berry gift waits for a starter and both optional rewards are awarded once")
{
    bool32 berry = FALSE;
    PARAMETRIZE { berry = FALSE; }
    PARAMETRIZE { berry = TRUE; }
    ResetTownRewards();
    u32 flag = berry ? FLAG_VIRELIA_MAPLEBROOK_BERRY_GIFT : FLAG_VIRELIA_MAPLEBROOK_GROVE_FIND;
    enum Item item = berry ? ITEM_ORAN_BERRY : ITEM_ANTIDOTE;
    if (berry)
    {
        TryTownReward(TRUE);
        EXPECT_EQ(gSpecialVar_Result, FALSE);
        EXPECT(!FlagGet(flag));
        EXPECT(!CheckBagHasItem(item, 1));
    }
    FlagSet(FLAG_VIRELIA_STARTER_RECEIVED);
    TryTownReward(berry);
    bool32 story = VIRELIA_PROLOGUE && !VIRELIA_DEVELOPMENT_MODE;
    EXPECT_EQ(gSpecialVar_Result, story);
    EXPECT_EQ(FlagGet(flag), story);
    EXPECT_EQ(CountTotalItemQuantityInBag(item), story ? 1 : 0);
    TryTownReward(berry);
    EXPECT_EQ(gSpecialVar_Result, FALSE);
    EXPECT_EQ(CountTotalItemQuantityInBag(item), story ? 1 : 0);
}

TEST("Virelia Maplebrook full pockets leave optional rewards available for a later retry")
{
    bool32 berry = FALSE;
    PARAMETRIZE { berry = FALSE; }
    PARAMETRIZE { berry = TRUE; }
    ResetTownRewards();
    FlagSet(FLAG_VIRELIA_STARTER_RECEIVED);
    enum Item item = berry ? ITEM_ORAN_BERRY : ITEM_ANTIDOTE;
    u32 flag = berry ? FLAG_VIRELIA_MAPLEBROOK_BERRY_GIFT : FLAG_VIRELIA_MAPLEBROOK_GROVE_FIND;
    struct BagPocket *pocket = &gBagPockets[berry ? POCKET_BERRIES : POCKET_ITEMS];
    for (u32 i = 0; i < pocket->capacity; i++)
        BagPocket_SetSlotItemIdAndCount(pocket, i, item, MAX_BAG_ITEM_CAPACITY);
    TryTownReward(berry);
    EXPECT_EQ(gSpecialVar_Result, FALSE);
    EXPECT(!FlagGet(flag));
    ClearBag();
    TryTownReward(berry);
    bool32 story = VIRELIA_PROLOGUE && !VIRELIA_DEVELOPMENT_MODE;
    EXPECT_EQ(gSpecialVar_Result, story);
    EXPECT_EQ(FlagGet(flag), story);
    EXPECT_EQ(CountTotalItemQuantityInBag(item), story ? 1 : 0);
}

TEST("Virelia Maplebrook keeps the professor garden fence solid and the north trail open")
{
    SetWarpDestination(MAP_GROUP(MAP_VIRELIA_MAPLEBROOK_TOWN), MAP_NUM(MAP_VIRELIA_MAPLEBROOK_TOWN), WARP_ID_NONE, 14, 21);
    WarpIntoMap();
    InitMap();
    for (u32 x = 16; x <= 19; x++)
    {
        EXPECT_NE(MapGridGetCollisionAt(x + MAP_OFFSET, 15 + MAP_OFFSET), 0);
        EXPECT_NE(MapGridGetCollisionAt(x + MAP_OFFSET, 20 + MAP_OFFSET), 0);
    }
    for (u32 y = 16; y <= 19; y++)
        EXPECT_NE(MapGridGetCollisionAt(16 + MAP_OFFSET, y + MAP_OFFSET), 0);
    for (u32 x = 14; x <= 15; x++)
        for (u32 y = 0; y <= 22; y++)
            EXPECT_EQ(MapGridGetCollisionAt(x + MAP_OFFSET, y + MAP_OFFSET), 0);
}

#endif
