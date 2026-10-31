#include "global.h"
#include "battle_setup.h"
#include "event_data.h"
#include "item.h"
#include "fieldmap.h"
#include "overworld.h"
#include "script.h"
#include "pokemon.h"
#include "test/test.h"
#include "test/overworld_script.h"
#include "virelia_first_gym.h"
#include "constants/items.h"
#include "constants/opponents.h"
#include "constants/virelia_prologue.h"
#include "constants/virelia_first_gym.h"
#include "constants/map_scripts.h"

#if !IS_FRLG && VIRELIA_PROLOGUE && !VIRELIA_DEVELOPMENT_MODE

static void ResetChapterEvents(void)
{
    InitEventData();
    SetBagItemsPointers();
    ClearBag();
    for (u32 flag = FLAG_VIRELIA_MOM_MET; flag <= FLAG_VIRELIA_TRAINER_MARA; flag++)
        FlagClear(flag);
    VarSet(VAR_VIRELIA_PROLOGUE_STATE, VIRELIA_STATE_COMPLETE);
    VarSet(VAR_VIRELIA_QUARRY_SWITCH, 0);
    VarSet(VAR_VIRELIA_GYM_SWITCH, 0);
}

TEST("Virelia compiled Cut delivery requires the first badge and retries a full TM pocket exactly once")
{
    ResetChapterEvents();
    RUN_OVERWORLD_SCRIPT(call VireliaFirstGym_EventScript_AwardCut;);
    EXPECT_EQ(gSpecialVar_Result, FALSE);
    FlagSet(FLAG_VIRELIA_BADGE_QUARRY);
    RUN_OVERWORLD_SCRIPT(call VireliaFirstGym_EventScript_AwardCut;);
    EXPECT_EQ(gSpecialVar_Result, FALSE);
    FlagSet(FLAG_VIRELIA_PROLOGUE_COMPLETE);
    FlagClear(FLAG_VIRELIA_BADGE_QUARRY);
    RUN_OVERWORLD_SCRIPT(call VireliaFirstGym_EventScript_AwardCut;);
    EXPECT_EQ(gSpecialVar_Result, FALSE);
    FlagSet(FLAG_VIRELIA_BADGE_QUARRY);
    struct BagPocket *pocket = &gBagPockets[POCKET_TM_HM];
    for (u32 i = 0; i < pocket->capacity; i++)
        BagPocket_SetSlotItemIdAndCount(pocket, i, ITEM_TM_PROTECT, MAX_BAG_ITEM_CAPACITY);
    RUN_OVERWORLD_SCRIPT(call VireliaFirstGym_EventScript_AwardCut;);
    EXPECT_EQ(gSpecialVar_Result, FALSE);
    EXPECT(!FlagGet(FLAG_VIRELIA_HM_CUT_RECEIVED));
    EXPECT(!CheckBagHasItem(ITEM_HM_CUT, 1));
    ClearBag();
    RUN_OVERWORLD_SCRIPT(call VireliaFirstGym_EventScript_AwardCut;);
    EXPECT_EQ(gSpecialVar_Result, TRUE);
    EXPECT(FlagGet(FLAG_VIRELIA_HM_CUT_RECEIVED));
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_HM_CUT), 1);
    RUN_OVERWORLD_SCRIPT(call VireliaFirstGym_EventScript_AwardCut;);
    EXPECT_EQ(gSpecialVar_Result, FALSE);
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_HM_CUT), 1);
    for (u32 flag = FLAG_BADGE01_GET; flag <= FLAG_BADGE08_GET; flag++)
        EXPECT(!FlagGet(flag));
}

extern const u8 Virelia_SlatefordCity_EventScript_CutDelivery[];
extern const u8 Virelia_SlatefordCity_EventScript_CutAide[];

TEST("Virelia native city callback schedules the aide only at the gym exit after victory")
{
    u32 checkpoint = 0;
    for (u32 i = 0; i < 5; i++)
        PARAMETRIZE { checkpoint = i; }
    ResetChapterEvents();
    FlagSet(FLAG_VIRELIA_PROLOGUE_COMPLETE);
    if (checkpoint >= 1)
        FlagSet(FLAG_VIRELIA_BADGE_QUARRY);
    if (checkpoint == 3)
        VireliaFirstGym_AwardCut();
    bool32 exit = checkpoint != 2;
    SetWarpDestination(MAP_GROUP(MAP_VIRELIA_SLATEFORD_CITY), MAP_NUM(MAP_VIRELIA_SLATEFORD_CITY),
                       WARP_ID_NONE, exit ? 22 : 14, checkpoint == 4 ? 9 : exit ? 10 : 27);
    WarpIntoMap();
    LoadObjEventTemplatesFromHeader();
    InitMap();
    RunOnLoadMapScript();
    bool32 delivery = checkpoint == 1 || checkpoint == 4;
    bool32 waiting = checkpoint == 1 || checkpoint == 2 || checkpoint == 4;
    EXPECT_EQ(VarGet(VAR_VIRELIA_CUT_DELIVERY), delivery);
    EXPECT_EQ(FlagGet(FLAG_VIRELIA_HIDE_CUT_AIDE), !waiting);
    const u8 *frameScript = MapHeaderCheckScriptTable(MAP_SCRIPT_ON_FRAME_TABLE);
    EXPECT_EQ(frameScript, delivery ? Virelia_SlatefordCity_EventScript_CutDelivery : NULL);
    const struct ObjectEventTemplate *aide = NULL;
    for (u32 i = 0; i < gMapHeader.events->objectEventCount; i++)
        if (gSaveBlock1Ptr->objectEventTemplates[i].script == Virelia_SlatefordCity_EventScript_CutAide)
            aide = &gSaveBlock1Ptr->objectEventTemplates[i];
    EXPECT_NE(aide, NULL);
    EXPECT_EQ(aide->x, delivery ? 26 : 23);
    EXPECT_EQ(aide->y, 10);
    if (delivery)
    {
        VarSet(VAR_VIRELIA_CUT_DELIVERY, 0);
        EXPECT_EQ(MapHeaderCheckScriptTable(MAP_SCRIPT_ON_FRAME_TABLE), NULL);
    }
}

TEST("Virelia compiled chapter entry requires the completed prologue and never replays it")
{
    ResetChapterEvents();
    RUN_OVERWORLD_SCRIPT(call VireliaFirstGym_EventScript_BeginChapter;);
    EXPECT_EQ(gSpecialVar_Result, FALSE);
    EXPECT(!FlagGet(FLAG_VIRELIA_CHAPTER_STARTED));
    FlagSet(FLAG_VIRELIA_ROCKET_DEFEATED);
    RUN_OVERWORLD_SCRIPT(call VireliaFirstGym_EventScript_BeginChapter;);
    EXPECT_EQ(gSpecialVar_Result, FALSE);
    FlagSet(FLAG_VIRELIA_PROLOGUE_COMPLETE);
    RUN_OVERWORLD_SCRIPT(call VireliaFirstGym_EventScript_BeginChapter;);
    EXPECT_EQ(gSpecialVar_Result, TRUE);
    EXPECT(FlagGet(FLAG_VIRELIA_CHAPTER_STARTED));
    RUN_OVERWORLD_SCRIPT(call VireliaFirstGym_EventScript_BeginChapter;);
    EXPECT_EQ(VarGet(VAR_VIRELIA_PROLOGUE_STATE), VIRELIA_STATE_COMPLETE);
    EXPECT(FlagGet(FLAG_VIRELIA_PROLOGUE_COMPLETE));
}

TEST("Virelia compiled switches reset wrong and partial input while retaining solved puzzles")
{
    u32 puzzle = 0;
    PARAMETRIZE { puzzle = 0; }
    PARAMETRIZE { puzzle = 1; }
    ResetChapterEvents();
    FlagSet(FLAG_VIRELIA_PROLOGUE_COMPLETE);
    RUN_OVERWORLD_SCRIPT(call VireliaFirstGym_EventScript_BeginChapter;);
    if (puzzle) FlagSet(FLAG_VIRELIA_QUARRY_COMPLETE);
    u16 var = puzzle ? VAR_VIRELIA_GYM_SWITCH : VAR_VIRELIA_QUARRY_SWITCH;
    u16 flag = puzzle ? FLAG_VIRELIA_GYM_PUZZLE_COMPLETE : FLAG_VIRELIA_QUARRY_POWER_RESTORED;
    gSpecialVar_0x8004 = 2;
    if (puzzle) RUN_OVERWORLD_SCRIPT(call VireliaFirstGym_EventScript_GymSwitch;);
    else RUN_OVERWORLD_SCRIPT(call VireliaFirstGym_EventScript_QuarrySwitch;);
    EXPECT_EQ(VarGet(var), 0);
    EXPECT(!FlagGet(flag));
    gSpecialVar_0x8004 = 1;
    if (puzzle) RUN_OVERWORLD_SCRIPT(call VireliaFirstGym_EventScript_GymSwitch;);
    else RUN_OVERWORLD_SCRIPT(call VireliaFirstGym_EventScript_QuarrySwitch;);
    EXPECT_EQ(VarGet(var), 1);
    if (puzzle) VireliaFirstGym_ResetGymPuzzle();
    else VireliaFirstGym_ResetQuarryPuzzle();
    EXPECT_EQ(VarGet(var), 0);
    gSpecialVar_0x8004 = 2;
    if (puzzle) RUN_OVERWORLD_SCRIPT(call VireliaFirstGym_EventScript_GymSwitch;);
    else RUN_OVERWORLD_SCRIPT(call VireliaFirstGym_EventScript_QuarrySwitch;);
    EXPECT(!FlagGet(flag));
    gSpecialVar_0x8004 = 1;
    if (puzzle) RUN_OVERWORLD_SCRIPT(call VireliaFirstGym_EventScript_GymSwitch;);
    else RUN_OVERWORLD_SCRIPT(call VireliaFirstGym_EventScript_QuarrySwitch;);
    gSpecialVar_0x8004 = 2;
    if (puzzle) RUN_OVERWORLD_SCRIPT(call VireliaFirstGym_EventScript_GymSwitch;);
    else RUN_OVERWORLD_SCRIPT(call VireliaFirstGym_EventScript_QuarrySwitch;);
    EXPECT_EQ(gSpecialVar_Result, TRUE);
    EXPECT(FlagGet(flag));
    ClearTempFieldEventData();
    if (puzzle) VireliaFirstGym_ResetGymPuzzle();
    else VireliaFirstGym_ResetQuarryPuzzle();
    EXPECT_EQ(VarGet(var), 0);
    EXPECT(FlagGet(flag));
}

TEST("Virelia compiled Gym puzzle cannot advance while the quarry remains unfinished")
{
    ResetChapterEvents();
    FlagSet(FLAG_VIRELIA_PROLOGUE_COMPLETE);
    RUN_OVERWORLD_SCRIPT(call VireliaFirstGym_EventScript_BeginChapter;);
    for (u32 i = 1; i <= 2; i++)
    {
        gSpecialVar_0x8004 = i;
        RUN_OVERWORLD_SCRIPT(call VireliaFirstGym_EventScript_GymSwitch;);
        EXPECT_EQ(gSpecialVar_Result, FALSE);
        EXPECT_EQ(VarGet(VAR_VIRELIA_GYM_SWITCH), 0);
    }
    EXPECT(!FlagGet(FLAG_VIRELIA_GYM_PUZZLE_COMPLETE));
    RUN_OVERWORLD_SCRIPT(call VireliaFirstGym_EventScript_AwardBadge;);
    RUN_OVERWORLD_SCRIPT(call VireliaFirstGym_EventScript_AwardTM;);
    EXPECT(!FlagGet(FLAG_VIRELIA_BADGE_QUARRY));
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_TM_ROCK_TOMB), 0);
}

TEST("Virelia compiled optional quarry pickup retries a full Bag and persists exactly once")
{
    ResetChapterEvents();
    RUN_OVERWORLD_SCRIPT(call VireliaFirstGym_EventScript_TakeQuarryItem;);
    EXPECT_EQ(gSpecialVar_Result, FALSE);
    FlagSet(FLAG_VIRELIA_PROLOGUE_COMPLETE);
    RUN_OVERWORLD_SCRIPT(call VireliaFirstGym_EventScript_BeginChapter;);
    struct BagPocket *pocket = &gBagPockets[POCKET_ITEMS];
    for (u32 i = 0; i < pocket->capacity; i++)
        BagPocket_SetSlotItemIdAndCount(pocket, i, ITEM_ANTIDOTE, MAX_BAG_ITEM_CAPACITY);
    EXPECT(!CheckBagHasSpace(ITEM_SUPER_POTION, 1));
    RUN_OVERWORLD_SCRIPT(call VireliaFirstGym_EventScript_TakeQuarryItem;);
    EXPECT_EQ(gSpecialVar_Result, FALSE);
    EXPECT(!FlagGet(FLAG_VIRELIA_ITEM_QUARRY_SUPER_POTION));
    ClearBag();
    RUN_OVERWORLD_SCRIPT(call VireliaFirstGym_EventScript_TakeQuarryItem;);
    EXPECT_EQ(gSpecialVar_Result, TRUE);
    EXPECT(FlagGet(FLAG_VIRELIA_ITEM_QUARRY_SUPER_POTION));
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_SUPER_POTION), 1);
    RUN_OVERWORLD_SCRIPT(call VireliaFirstGym_EventScript_TakeQuarryItem;);
    EXPECT_EQ(gSpecialVar_Result, FALSE);
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_SUPER_POTION), 1);
}

TEST("Virelia compiled evidence manager and leader reward events retain independent progression")
{
    ResetChapterEvents();
    FlagSet(FLAG_VIRELIA_PROLOGUE_COMPLETE);
    RUN_OVERWORLD_SCRIPT(call VireliaFirstGym_EventScript_BeginChapter;);
    FlagSet(FLAG_VIRELIA_QUARRY_POWER_RESTORED);
    SetTrainerFlag(TRAINER_VIRELIA_MINER_BRUNO);
    SetTrainerFlag(TRAINER_VIRELIA_EXPLORER_LENKA);
    RUN_OVERWORLD_SCRIPT(call VireliaFirstGym_EventScript_ReadEvidence;);
    EXPECT_EQ(gSpecialVar_Result, TRUE);
    RUN_OVERWORLD_SCRIPT(call VireliaFirstGym_EventScript_CompleteQuarry;);
    EXPECT_EQ(gSpecialVar_Result, TRUE);
    FlagSet(FLAG_VIRELIA_GYM_PUZZLE_COMPLETE);
    SetTrainerFlag(TRAINER_VIRELIA_GYM_IVAN);
    SetTrainerFlag(TRAINER_VIRELIA_GYM_DANA);
    SetTrainerFlag(TRAINER_VIRELIA_MARA);
    RUN_OVERWORLD_SCRIPT(call VireliaFirstGym_EventScript_AwardBadge;);
    EXPECT_EQ(gSpecialVar_Result, TRUE);
    RUN_OVERWORLD_SCRIPT(call VireliaFirstGym_EventScript_AwardTM;);
    EXPECT_EQ(gSpecialVar_Result, TRUE);
    EXPECT(FlagGet(FLAG_VIRELIA_FIRST_GYM_COMPLETE));
    RUN_OVERWORLD_SCRIPT(call VireliaFirstGym_EventScript_AwardBadge;);
    EXPECT_EQ(gSpecialVar_Result, FALSE);
    RUN_OVERWORLD_SCRIPT(call VireliaFirstGym_EventScript_AwardTM;);
    EXPECT_EQ(gSpecialVar_Result, FALSE);
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_TM_ROCK_TOMB), 1);
    for (u32 flag = FLAG_BADGE01_GET; flag <= FLAG_BADGE08_GET; flag++)
        EXPECT(!FlagGet(flag));
}

#endif
