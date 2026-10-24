#include "global.h"
#include "agb_flash.h"
#include "event_data.h"
#include "fieldmap.h"
#include "item.h"
#include "main.h"
#include "overworld.h"
#include "save.h"
#include "script.h"
#include "test/test.h"
#include "test/overworld_script.h"
#include "constants/virelia_prologue.h"
#include "constants/virelia_sanctuary.h"

#if !IS_FRLG

static void ResetSanctuary(void)
{
    InitEventData();
    SetBagItemsPointers();
    ClearBag();
    FlagClear(FLAG_VIRELIA_SIDEQUEST_STARTED);
    FlagClear(FLAG_VIRELIA_SIDEQUEST_RESOLVED);
    FlagClear(FLAG_VIRELIA_SIDEQUEST_REWARDED);
    FlagClear(FLAG_VIRELIA_SANCTUARY_SHORTCUT_OPEN);
    FlagClear(FLAG_VIRELIA_SANCTUARY_BELL_RECEIVED);
    VarSet(VAR_VIRELIA_SANCTUARY_STONE, 0);
}

static u16 PressStone(u16 stone)
{
    gSpecialVar_0x8004 = stone;
    RUN_OVERWORLD_SCRIPT(call VireliaSanctuary_EventScript_TouchStone;);
    return gSpecialVar_Result;
}

static void SolveSanctuary(void)
{
    FlagSet(FLAG_VIRELIA_SIDEQUEST_REWARDED);
    PressStone(VIRELIA_SANCTUARY_SEED);
    PressStone(VIRELIA_SANCTUARY_RAIN);
    PressStone(VIRELIA_SANCTUARY_ROOT);
}

static void LoadSanctuaryMap(u16 map)
{
    SetWarpDestination(MAP_GROUP(map), MAP_NUM(map), WARP_ID_NONE, 7, 15);
    WarpIntoMap();
    InitMap();
}

static void ExpectGateAccess(bool32 hanaComplete, bool32 shortcutOpen)
{
    LoadSanctuaryMap(MAP_VIRELIA_ROUTE01);
    for (u32 y = 16; y <= 17; y++)
    {
        EXPECT_EQ(MapGridGetCollisionAt(1 + MAP_OFFSET, y + MAP_OFFSET) == 0, hanaComplete);
        EXPECT_EQ(MapGridGetCollisionAt(-2 + MAP_OFFSET, y + MAP_OFFSET) == 0, hanaComplete);
    }
    for (u32 y = 8; y <= 9; y++)
    {
        EXPECT_EQ(MapGridGetCollisionAt(1 + MAP_OFFSET, y + MAP_OFFSET) == 0, hanaComplete && shortcutOpen);
        EXPECT_EQ(MapGridGetCollisionAt(-6 + MAP_OFFSET, y + MAP_OFFSET) == 0, hanaComplete && shortcutOpen);
    }
    LoadSanctuaryMap(MAP_VIRELIA_WHISPERWOOD_SANCTUARY);
    for (u32 y = 10; y <= 11; y++)
    {
        EXPECT_EQ(MapGridGetCollisionAt(20 + MAP_OFFSET, y + MAP_OFFSET) == 0, hanaComplete);
        EXPECT_EQ(MapGridGetCollisionAt(23 + MAP_OFFSET, y + MAP_OFFSET) == 0, hanaComplete);
    }
    for (u32 y = 2; y <= 3; y++)
    {
        EXPECT_EQ(MapGridGetCollisionAt(16 + MAP_OFFSET, y + MAP_OFFSET) == 0, hanaComplete && shortcutOpen);
        EXPECT_EQ(MapGridGetCollisionAt(23 + MAP_OFFSET, y + MAP_OFFSET) == 0, hanaComplete && shortcutOpen);
    }
}

TEST("Virelia sanctuary raw connected layouts seal both entrances before load callbacks")
{
    ResetSanctuary();
    static const u8 gates[][2] = {{1, 8}, {1, 9}, {1, 16}, {1, 17},
                                 {16, 2}, {16, 3}, {20, 10}, {20, 11}};
    for (u32 map = 0; map < 2; map++)
    {
        LoadSanctuaryMap(map ? MAP_VIRELIA_WHISPERWOOD_SANCTUARY : MAP_VIRELIA_ROUTE01);
        for (u32 i = map * 4; i < map * 4 + 4; i++)
        {
            u16 tile = gMapHeader.mapLayout->map[gates[i][1] * gMapHeader.mapLayout->width + gates[i][0]];
            EXPECT_NE(UNPACK_COLLISION(tile), 0);
            EXPECT_EQ(UNPACK_METATILE(tile), 0xE);
        }
    }
}

TEST("Virelia sanctuary Hana gate waits for the actual reward including full Bag retries")
{
    ResetSanctuary();
    ExpectGateAccess(FALSE, FALSE);
    FlagSet(FLAG_VIRELIA_SIDEQUEST_STARTED);
    ExpectGateAccess(FALSE, FALSE);
    RUN_OVERWORLD_SCRIPT(call VireliaPrologue_EventScript_FindZigzagoon;);
    ExpectGateAccess(FALSE, FALSE);
    struct BagPocket *pocket = &gBagPockets[POCKET_ITEMS];
    for (u32 i = 0; i < pocket->capacity; i++)
        BagPocket_SetSlotItemIdAndCount(pocket, i, ITEM_ANTIDOTE, MAX_BAG_ITEM_CAPACITY);
    RUN_OVERWORLD_SCRIPT(call VireliaPrologue_EventScript_TryRewardHana;);
    EXPECT_EQ(gSpecialVar_Result, FALSE);
    EXPECT(!FlagGet(FLAG_VIRELIA_SIDEQUEST_REWARDED));
    ExpectGateAccess(FALSE, FALSE);
    // Even a shortcut flag alone must never bypass Hana's completion gate.
    FlagSet(FLAG_VIRELIA_SANCTUARY_SHORTCUT_OPEN);
    ExpectGateAccess(FALSE, TRUE);
    FlagClear(FLAG_VIRELIA_SANCTUARY_SHORTCUT_OPEN);
    ClearBag();
    RUN_OVERWORLD_SCRIPT(call VireliaPrologue_EventScript_TryRewardHana;);
    EXPECT_EQ(gSpecialVar_Result, TRUE);
    EXPECT(FlagGet(FLAG_VIRELIA_SIDEQUEST_REWARDED));
    ExpectGateAccess(TRUE, FALSE);
    SolveSanctuary();
    ExpectGateAccess(TRUE, TRUE);
}

TEST("Virelia sanctuary pawprints stay below the player while the ancient tree remains above")
{
    ResetSanctuary();
    LoadSanctuaryMap(MAP_VIRELIA_WHISPERWOOD_SANCTUARY);
    static const u8 prints[][2] = {{9, 10}, {10, 11}, {11, 13}};
    for (u32 i = 0; i < ARRAY_COUNT(prints); i++)
    {
        s32 x = prints[i][0] + MAP_OFFSET, y = prints[i][1] + MAP_OFFSET;
        EXPECT_EQ(MapGridGetMetatileIdAt(x, y), 0x318);
        EXPECT_EQ(MapGridGetMetatileLayerTypeAt(x, y), METATILE_LAYER_TYPE_COVERED);
    }
    EXPECT_EQ(MapGridGetMetatileLayerTypeAt(9 + MAP_OFFSET, 9 + MAP_OFFSET), METATILE_LAYER_TYPE_NORMAL);
}

TEST("Virelia sanctuary accepts only seed rain root and allows retries")
{
    u32 first = 1, second = 1, third = 1;
    for (u32 a = 1; a <= 3; a++)
        for (u32 b = 1; b <= 3; b++)
            for (u32 c = 1; c <= 3; c++)
                PARAMETRIZE { first = a; second = b; third = c; }
    ResetSanctuary();
    EXPECT_EQ(PressStone(VIRELIA_SANCTUARY_SEED), VIRELIA_SANCTUARY_WRONG);
    EXPECT_EQ(VarGet(VAR_VIRELIA_SANCTUARY_STONE), 0);
    FlagSet(FLAG_VIRELIA_SIDEQUEST_REWARDED);
    PressStone(first);
    PressStone(second);
    PressStone(third);
    bool32 solved = first == 1 && second == 2 && third == 3;
    EXPECT_EQ(FlagGet(FLAG_VIRELIA_SANCTUARY_SHORTCUT_OPEN), solved);
    if (!solved)
    {
        RUN_OVERWORLD_SCRIPT(call VireliaSanctuary_EventScript_ResetStones;);
        SolveSanctuary();
    }
    EXPECT(FlagGet(FLAG_VIRELIA_SANCTUARY_SHORTCUT_OPEN));
    EXPECT_EQ(PressStone(0), VIRELIA_SANCTUARY_ALREADY_SOLVED);
    EXPECT(FlagGet(FLAG_VIRELIA_SANCTUARY_SHORTCUT_OPEN));
}

TEST("Virelia sanctuary resets unfinished stones on reentry and preserves the solved shortcut")
{
    ResetSanctuary();
    FlagSet(FLAG_VIRELIA_SIDEQUEST_REWARDED);
    EXPECT_EQ(PressStone(1), VIRELIA_SANCTUARY_PROGRESS);
    EXPECT_EQ(PressStone(1), VIRELIA_SANCTUARY_WRONG);
    EXPECT_EQ(VarGet(VAR_VIRELIA_SANCTUARY_STONE), 0);
    PressStone(1);
    PressStone(2);
    LoadSanctuaryMap(MAP_VIRELIA_WHISPERWOOD_SANCTUARY);
    RunOnTransitionMapScript();
    EXPECT_EQ(VarGet(VAR_VIRELIA_SANCTUARY_STONE), 0);
    EXPECT_EQ(PressStone(3), VIRELIA_SANCTUARY_WRONG);
    SolveSanctuary();
    RUN_OVERWORLD_SCRIPT(call VireliaSanctuary_EventScript_ResetStones;);
    EXPECT(FlagGet(FLAG_VIRELIA_SANCTUARY_SHORTCUT_OPEN));
}

TEST("Virelia sanctuary native map callbacks restore both sides of each gate")
{
    u32 state = 0;
    PARAMETRIZE { state = 0; }
    PARAMETRIZE { state = 1; }
    PARAMETRIZE { state = 2; }
    ResetSanctuary();
    if (state >= 1) FlagSet(FLAG_VIRELIA_SIDEQUEST_REWARDED);
    if (state >= 2) FlagSet(FLAG_VIRELIA_SANCTUARY_SHORTCUT_OPEN);
    ExpectGateAccess(state >= 1, state >= 2);
}

TEST("Virelia sanctuary restores all twelve custom tree tiles over older cached map views")
{
    ResetSanctuary();
    FlagSet(FLAG_VIRELIA_SIDEQUEST_REWARDED);
    LoadSanctuaryMap(MAP_VIRELIA_WHISPERWOOD_SANCTUARY);
    for (u32 y = 6; y < 10; y++)
        for (u32 x = 8; x < 11; x++)
            MapGridSetMetatileIdAt(x + MAP_OFFSET, y + MAP_OFFSET, 0x001);
    RunOnLoadMapScript();
    for (u32 y = 0; y < 4; y++)
        for (u32 x = 0; x < 3; x++)
        {
            EXPECT_EQ(MapGridGetMetatileIdAt(x + 8 + MAP_OFFSET, y + 6 + MAP_OFFSET), 0x31D + y * 3 + x);
            EXPECT_NE(MapGridGetCollisionAt(x + 8 + MAP_OFFSET, y + 6 + MAP_OFFSET), 0);
        }
}

TEST("Virelia sanctuary bell waits for the puzzle and a free bag slot without duplicating")
{
    ResetSanctuary();
    RUN_OVERWORLD_SCRIPT(call VireliaSanctuary_EventScript_TryTakeBell;);
    EXPECT_EQ(gSpecialVar_Result, FALSE);
    EXPECT(!FlagGet(FLAG_VIRELIA_SANCTUARY_BELL_RECEIVED));
    SolveSanctuary();
    struct BagPocket *pocket = &gBagPockets[POCKET_ITEMS];
    for (u32 i = 0; i < pocket->capacity; i++)
        BagPocket_SetSlotItemIdAndCount(pocket, i, ITEM_ANTIDOTE, MAX_BAG_ITEM_CAPACITY);
    EXPECT(!CheckBagHasSpace(ITEM_SOOTHE_BELL, 1));
    RUN_OVERWORLD_SCRIPT(call VireliaSanctuary_EventScript_TryTakeBell;);
    EXPECT_EQ(gSpecialVar_Result, FALSE);
    EXPECT(!FlagGet(FLAG_VIRELIA_SANCTUARY_BELL_RECEIVED));
    EXPECT(FlagGet(FLAG_VIRELIA_SANCTUARY_SHORTCUT_OPEN));
    ClearBag();
    RUN_OVERWORLD_SCRIPT(call VireliaSanctuary_EventScript_TryTakeBell;);
    EXPECT_EQ(gSpecialVar_Result, TRUE);
    EXPECT(FlagGet(FLAG_VIRELIA_SANCTUARY_BELL_RECEIVED));
    RUN_OVERWORLD_SCRIPT(call VireliaSanctuary_EventScript_TryTakeBell;);
    EXPECT_EQ(gSpecialVar_Result, FALSE);
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_SOOTHE_BELL), 1);
}

TEST("Virelia sanctuary shortcut and claimed or unclaimed bell survive native flash reload")
{
    u32 claimed = 0;
    PARAMETRIZE { claimed = 0; }
    PARAMETRIZE { claimed = 1; }
    ResetSanctuary();
    SolveSanctuary();
    if (claimed)
        RUN_OVERWORLD_SCRIPT(call VireliaSanctuary_EventScript_TryTakeBell;);
    ClearTempFieldEventData();
    IntrFunc originalTimer3Handler, unusedFlashHandler;
    u16 originalIme = REG_IME;
    REG_IME = 0;
    u16 originalIe = REG_IE;
    originalTimer3Handler = gIntrTable[2];
    u16 originalTimer3Low = REG_TM3CNT_L, originalTimer3High = REG_TM3CNT_H;
    REG_TM3CNT_H = 0;
    REG_IF = INTR_FLAG_TIMER3;
    SetFlashTimerIntr(3, &gIntrTable[2]);
    REG_IME = originalIme;
    ClearSaveData();
    Save_ResetSaveCounters();
    u8 saveStatus = TrySavingData(SAVE_NORMAL), loadStatus = SAVE_STATUS_ERROR;
    if (saveStatus == SAVE_STATUS_OK)
    {
        FlagClear(FLAG_VIRELIA_SIDEQUEST_REWARDED);
        FlagClear(FLAG_VIRELIA_SANCTUARY_SHORTCUT_OPEN);
        FlagClear(FLAG_VIRELIA_SANCTUARY_BELL_RECEIVED);
        ClearBag();
        loadStatus = LoadGameSave(SAVE_NORMAL);
    }
    REG_IME = 0;
    REG_TM3CNT_H = 0;
    REG_IF = INTR_FLAG_TIMER3;
    SetFlashTimerIntr(2, &unusedFlashHandler);
    gIntrTable[2] = originalTimer3Handler;
    REG_TM3CNT_L = originalTimer3Low;
    REG_TM3CNT_H = originalTimer3High;
    REG_IE = originalIe;
    REG_IME = originalIme;
    EXPECT_EQ(saveStatus, SAVE_STATUS_OK);
    EXPECT_EQ(loadStatus, SAVE_STATUS_OK);
    EXPECT(FlagGet(FLAG_VIRELIA_SIDEQUEST_REWARDED));
    EXPECT(FlagGet(FLAG_VIRELIA_SANCTUARY_SHORTCUT_OPEN));
    EXPECT_EQ(FlagGet(FLAG_VIRELIA_SANCTUARY_BELL_RECEIVED), claimed);
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_SOOTHE_BELL), claimed);
    RUN_OVERWORLD_SCRIPT(call VireliaSanctuary_EventScript_TryTakeBell;);
    EXPECT_EQ(gSpecialVar_Result, claimed ? FALSE : TRUE);
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_SOOTHE_BELL), 1);
}

#endif // !IS_FRLG
