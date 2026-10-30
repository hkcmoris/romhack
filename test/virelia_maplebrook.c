#include "global.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "field_move.h"
#include "fieldmap.h"
#include "item.h"
#include "metatile_behavior.h"
#include "overworld.h"
#include "pokemon.h"
#include "test/test.h"
#include "test/overworld_script.h"
#include "constants/virelia_maplebrook.h"
#include "constants/virelia_prologue.h"
#include "constants/metatile_labels.h"
#include "constants/event_bg.h"
#include "constants/event_objects.h"
#include "constants/moves.h"
#include "constants/virelia_first_gym.h"

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

static void EnterTown(void)
{
    SetWarpDestination(MAP_GROUP(MAP_VIRELIA_MAPLEBROOK_TOWN), MAP_NUM(MAP_VIRELIA_MAPLEBROOK_TOWN), WARP_ID_NONE, 9, 47);
    WarpIntoMap();
    InitMap();
}

TEST("Virelia Maplebrook keeps the relocated professor garden fence solid and the north trail open")
{
    EnterTown();
    for (u32 x = 23; x <= 26; x++)
    {
        EXPECT_NE(MapGridGetCollisionAt(x + MAP_OFFSET, 1 + MAP_OFFSET), 0);
        EXPECT_NE(MapGridGetCollisionAt(x + MAP_OFFSET, 6 + MAP_OFFSET), 0);
    }
    for (u32 y = 2; y <= 5; y++)
        EXPECT_NE(MapGridGetCollisionAt(23 + MAP_OFFSET, y + MAP_OFFSET), 0);
    for (u32 x = 14; x <= 15; x++)
        for (u32 y = 0; y <= 2; y++)
            EXPECT_EQ(MapGridGetCollisionAt(x + MAP_OFFSET, y + MAP_OFFSET), 0);
}

TEST("Virelia Maplebrook north gate covers both exit tiles before supplies are received")
{
    EnterTown();
    for (u32 state = 0; state <= 3; state++)
        for (u32 x = 14; x <= 15; x++)
        {
            u32 matches = 0;
            for (u32 i = 0; i < gMapHeader.events->coordEventCount; i++)
            {
                const struct CoordEvent *event = &gMapHeader.events->coordEvents[i];
                if (event->x == x && event->y == 1 && event->trigger == VAR_VIRELIA_PROLOGUE_STATE
                    && event->index == state)
                    matches++;
            }
            EXPECT_EQ(matches, 1);
            // The gate's walk_down movement must land on clear ground.
            EXPECT_EQ(MapGridGetCollisionAt(x + MAP_OFFSET, 2 + MAP_OFFSET), 0);
        }
}

static bool32 TownCanWalk(s32 x, s32 y)
{
    if (x < 0 || y < 0 || x >= 36 || y >= 48)
        return FALSE;
    if (x == VIRELIA_MAPLEBROOK_CUT_TREE_X && y == VIRELIA_MAPLEBROOK_CUT_TREE_Y && !FlagGet(FLAG_TEMP_11))
        return FALSE;
    x += MAP_OFFSET;
    y += MAP_OFFSET;
    return MapGridGetCollisionAt(x, y) == 0
        && !IsElevationMismatchAt(3, x, y)
        && !MetatileBehavior_IsSurfableWaterOrUnderwater(MapGridGetMetatileBehaviorAt(x, y));
}

static EWRAM_DATA bool8 sTownSeen[48][36] = {0};
static EWRAM_DATA u16 sTownQueue[48 * 36] = {0};
static const s8 sTownDirections[][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};

static void FloodTown(void)
{
    memset(sTownSeen, 0, sizeof(sTownSeen));
    u32 head = 0, tail = 0;
    sTownSeen[47][9] = TRUE;
    sTownQueue[tail++] = 47 * 36 + 9;
    while (head < tail)
    {
        u16 point = sTownQueue[head++];
        for (u32 i = 0; i < ARRAY_COUNT(sTownDirections); i++)
        {
            s32 x = point % 36 + sTownDirections[i][0];
            s32 y = point / 36 + sTownDirections[i][1];
            if (!TownCanWalk(x, y) || sTownSeen[y][x])
                continue;
            sTownSeen[y][x] = TRUE;
            sTownQueue[tail++] = y * 36 + x;
        }
    }
}

TEST("Virelia Maplebrook vertical bridge joins the existing paths with a walkable wooden deck")
{
    EnterTown();
    for (u32 y = 20; y <= 23; y++)
        for (u32 x = 13; x <= 14; x++)
        {
            EXPECT(TownCanWalk(x, y));
            EXPECT_EQ(MapGridGetMetatileIdAt(x + MAP_OFFSET, y + MAP_OFFSET),
                      x == 13 ? METATILE_VireliaOutdoor_BridgeVertical_Left : METATILE_VireliaOutdoor_BridgeVertical_Right);
        }
    EXPECT(TownCanWalk(14, 19));
    EXPECT(TownCanWalk(14, 24));
}

TEST("Virelia Maplebrook bottom arrival reaches doors signs and the north trail without Surf")
{
    EnterTown();
    EXPECT_EQ(gMapHeader.mapLayout->width, 36);
    EXPECT_EQ(gMapHeader.mapLayout->height, 48);
    EXPECT_EQ(gSaveBlock1Ptr->pos.x, 9);
    EXPECT_EQ(gSaveBlock1Ptr->pos.y, 47);
    EXPECT(TownCanWalk(9, 47));
    FloodTown();
    EXPECT(sTownSeen[0][14]);
    EXPECT(sTownSeen[0][15]);
    EXPECT_EQ(gMapHeader.events->warpCount, 5);
    for (u32 i = 0; i < gMapHeader.events->warpCount; i++)
    {
        const struct WarpEvent *warp = &gMapHeader.events->warps[i];
        EXPECT(sTownSeen[warp->y + 1][warp->x]);
    }
    for (u32 i = 0; i < gMapHeader.events->bgEventCount; i++)
    {
        const struct BgEvent *event = &gMapHeader.events->bgEvents[i];
        if (event->kind == BG_EVENT_HIDDEN_ITEM)
            continue;
        bool32 approachable = FALSE;
        for (u32 j = 0; j < ARRAY_COUNT(sTownDirections); j++)
        {
            s32 x = event->x + sTownDirections[j][0];
            s32 y = event->y + sTownDirections[j][1];
            if (TownCanWalk(x, y) && sTownSeen[y][x])
                approachable = TRUE;
        }
        EXPECT(approachable);
    }
}

TEST("Virelia Maplebrook woodland item is hidden behind the single native Cut tree")
{
    ResetTownRewards();
    EnterTown();
    const struct ObjectEventTemplate *tree = NULL;
    const struct BgEvent *item = NULL;
    for (u32 i = 0; i < gMapHeader.events->objectEventCount; i++)
        if (gMapHeader.events->objectEvents[i].graphicsId == OBJ_EVENT_GFX_CUTTABLE_TREE)
            tree = &gMapHeader.events->objectEvents[i];
    for (u32 i = 0; i < gMapHeader.events->bgEventCount; i++)
        if (gMapHeader.events->bgEvents[i].kind == BG_EVENT_HIDDEN_ITEM)
            item = &gMapHeader.events->bgEvents[i];
    EXPECT_NE(tree, NULL);
    EXPECT_NE(item, NULL);
    EXPECT_EQ(tree->x, VIRELIA_MAPLEBROOK_CUT_TREE_X);
    EXPECT_EQ(tree->y, VIRELIA_MAPLEBROOK_CUT_TREE_Y);
    EXPECT_EQ(tree->flagId, FLAG_TEMP_11);
    EXPECT_EQ(item->x, VIRELIA_MAPLEBROOK_WOODLAND_ITEM_X);
    EXPECT_EQ(item->y, VIRELIA_MAPLEBROOK_WOODLAND_ITEM_Y);
    EXPECT_EQ((u32)item->bgUnion.hiddenItem.item, ITEM_MIRACLE_SEED);
    EXPECT_EQ(item->bgUnion.hiddenItem.hiddenItemId + FLAG_HIDDEN_ITEMS_START, FLAG_VIRELIA_MAPLEBROOK_WOODLAND_ITEM);
    EXPECT_EQ((u32)item->bgUnion.hiddenItem.quantity, 1);
    FloodTown();
    EXPECT(sTownSeen[VIRELIA_MAPLEBROOK_CUT_TREE_Y][VIRELIA_MAPLEBROOK_CUT_TREE_X - 1]);
    EXPECT(!sTownSeen[VIRELIA_MAPLEBROOK_WOODLAND_ITEM_Y][VIRELIA_MAPLEBROOK_WOODLAND_ITEM_X - 1]);
    FlagSet(FLAG_TEMP_11);
    FloodTown();
    EXPECT(sTownSeen[VIRELIA_MAPLEBROOK_CUT_TREE_Y][VIRELIA_MAPLEBROOK_CUT_TREE_X + 1]);
    EXPECT(sTownSeen[VIRELIA_MAPLEBROOK_WOODLAND_ITEM_Y][VIRELIA_MAPLEBROOK_WOODLAND_ITEM_X - 1]);
    FlagSet(FLAG_VIRELIA_MAPLEBROOK_WOODLAND_ITEM);
    ClearTempFieldEventData();
    EXPECT(!FlagGet(FLAG_TEMP_11));
    EXPECT(FlagGet(FLAG_VIRELIA_MAPLEBROOK_WOODLAND_ITEM));
}

TEST("Virelia Cut requires the Quarry Badge and a learned move while Surf stays locked")
{
    ResetTownRewards();
    EnterTown();
    ZeroPlayerPartyMons();
    EXPECT(CanLearnTeachableMove(SPECIES_PARAS, MOVE_CUT));
    EXPECT(CanLearnTeachableMove(SPECIES_BIDOOF, MOVE_CUT));
    CreateRandomMonWithIVs(&gParties[B_TRAINER_PLAYER][0], SPECIES_PARAS, 15, 10);
    gPartiesCount[B_TRAINER_PLAYER] = 1;
    SetMonMoveSlot(&gParties[B_TRAINER_PLAYER][0], MOVE_CUT, 0);
    FlagSet(FLAG_BADGE01_GET);
    // The script DSL currently requires the numeric value of FIELD_MOVE_CUT.
    RUN_OVERWORLD_SCRIPT(checkfieldmove 0, TRUE;);
    bool32 story = VIRELIA_PROLOGUE && !VIRELIA_DEVELOPMENT_MODE;
    EXPECT_EQ(gSpecialVar_Result, story ? PARTY_SIZE : 0);
    FlagClear(FLAG_BADGE01_GET);
    FlagSet(FLAG_VIRELIA_BADGE_QUARRY);
    RUN_OVERWORLD_SCRIPT(checkfieldmove 0, TRUE;);
    EXPECT_EQ(gSpecialVar_Result, story ? 0 : PARTY_SIZE);
    EXPECT_EQ(IsFieldMoveUnlocked(FIELD_MOVE_CUT), story);
    EXPECT(!IsFieldMoveUnlocked(FIELD_MOVE_SURF));
    SetMonMoveSlot(&gParties[B_TRAINER_PLAYER][0], MOVE_SCRATCH, 0);
    RUN_OVERWORLD_SCRIPT(checkfieldmove 0, TRUE;);
    EXPECT_EQ(gSpecialVar_Result, PARTY_SIZE);
    if (story)
    {
        gMapHeader.regionMapSectionId = MAPSEC_LITTLEROOT_TOWN;
        EXPECT(!IsFieldMoveUnlocked(FIELD_MOVE_CUT));
        FlagSet(FLAG_BADGE01_GET);
        EXPECT(IsFieldMoveUnlocked(FIELD_MOVE_CUT));
    }
}

TEST("Virelia Maplebrook cottage fisherman and Ema entrances use their redesigned house doors")
{
    EnterTown();
    const struct WarpEvent *cottage = &gMapHeader.events->warps[2];
    const struct WarpEvent *fisher = &gMapHeader.events->warps[3];
    const struct WarpEvent *ema = &gMapHeader.events->warps[4];
    EXPECT_EQ(cottage->x, 29);
    EXPECT_EQ(cottage->y, 34);
    EXPECT_EQ(cottage->mapGroup, MAP_GROUP(MAP_VIRELIA_MAPLEBROOK_COTTAGE));
    EXPECT_EQ(cottage->mapNum, MAP_NUM(MAP_VIRELIA_MAPLEBROOK_COTTAGE));
    EXPECT_EQ(fisher->x, 3);
    EXPECT_EQ(fisher->y, 25);
    EXPECT_EQ(fisher->mapGroup, MAP_GROUP(MAP_VIRELIA_MAPLEBROOK_FISHERMANS_HOUSE));
    EXPECT_EQ(fisher->mapNum, MAP_NUM(MAP_VIRELIA_MAPLEBROOK_FISHERMANS_HOUSE));
    EXPECT_EQ(ema->x, 30);
    EXPECT_EQ(ema->y, 20);
    EXPECT_EQ(ema->mapGroup, MAP_GROUP(MAP_VIRELIA_MAPLEBROOK_EMAS_HOUSE));
    EXPECT_EQ(ema->mapNum, MAP_NUM(MAP_VIRELIA_MAPLEBROOK_EMAS_HOUSE));
    EXPECT(MetatileBehavior_IsDoor(MapGridGetMetatileBehaviorAt(cottage->x + MAP_OFFSET, cottage->y + MAP_OFFSET)));
    EXPECT(MetatileBehavior_IsDoor(MapGridGetMetatileBehaviorAt(fisher->x + MAP_OFFSET, fisher->y + MAP_OFFSET)));
    EXPECT(MetatileBehavior_IsDoor(MapGridGetMetatileBehaviorAt(ema->x + MAP_OFFSET, ema->y + MAP_OFFSET)));
}

TEST("Virelia Maplebrook all five authored doors have native interior warps")
{
    EnterTown();
    u32 doors = 0;
    for (s32 y = 0; y < gMapHeader.mapLayout->height; y++)
        for (s32 x = 0; x < gMapHeader.mapLayout->width; x++)
        {
            if (!MetatileBehavior_IsDoor(MapGridGetMetatileBehaviorAt(x + MAP_OFFSET, y + MAP_OFFSET)))
                continue;
            u32 warps = 0;
            for (u32 i = 0; i < gMapHeader.events->warpCount; i++)
                if (gMapHeader.events->warps[i].x == x && gMapHeader.events->warps[i].y == y)
                    warps++;
            EXPECT_EQ(warps, 1);
            doors++;
        }
    EXPECT_EQ(doors, 5);
}

#endif
