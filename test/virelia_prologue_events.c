#include "global.h"
#include "battle.h"
#include "event_data.h"
#include "item.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "test/test.h"
#include "test/overworld_script.h"
#include "virelia_prologue.h"
#include "constants/virelia_prologue.h"

#if !IS_FRLG && P_VIRELIA_STARTERS

static void ResetPrologueEvents(void)
{
    InitEventData();
    ZeroPlayerPartyMons();
    ResetPokemonStorageSystem();
    SetBagItemsPointers();
    ClearBag();
    for (u32 flag = FLAG_VIRELIA_MOM_MET; flag <= FLAG_VIRELIA_HIDE_FIRE_STARTER; flag++)
        FlagClear(flag);
    FlagClear(FLAG_ITEM_VIRELIA_ROUTE01_POTION);
    FlagClear(FLAG_SYS_POKEMON_GET);
    FlagClear(FLAG_SYS_POKEDEX_GET);
    VarSet(VAR_VIRELIA_PROLOGUE_STATE, VIRELIA_STATE_HOME);
    VarSet(VAR_VIRELIA_STARTER, VIRELIA_STARTER_NONE);
}

static void FillItemsPocket(void)
{
    struct BagPocket *pocket = &gBagPockets[POCKET_ITEMS];
    for (u32 i = 0; i < pocket->capacity; i++)
        BagPocket_SetSlotItemIdAndCount(pocket, i, ITEM_ANTIDOTE, MAX_BAG_ITEM_CAPACITY);
}

TEST("Virelia compiled starter event grants one selected level five partner")
{
    u32 choice = 0;
    for (u32 i = 0; i < VIRELIA_STARTER_COUNT; i++)
        PARAMETRIZE { choice = i; }

    ResetPrologueEvents();
    gSpecialVar_0x8004 = choice;
    RUN_OVERWORLD_SCRIPT(call VireliaPrologue_EventScript_TryGiveStarter;);
    EXPECT_EQ(gSpecialVar_Result, FALSE);
    EXPECT_EQ(gPartiesCount[B_TRAINER_PLAYER], 0);

    VireliaPrologue_MeetMom();
    RUN_OVERWORLD_SCRIPT(call VireliaPrologue_EventScript_TryGiveStarter;);
    EXPECT_EQ(gSpecialVar_Result, TRUE);
    EXPECT_EQ(gPartiesCount[B_TRAINER_PLAYER], 1);
    EXPECT_EQ(VarGet(VAR_VIRELIA_STARTER), choice);
    EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_SPECIES), GetVireliaStarterSpecies(choice));
    EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_LEVEL), VIRELIA_STARTER_LEVEL);

    struct Pokemon original = gParties[B_TRAINER_PLAYER][0];
    gSpecialVar_0x8004 = (choice + 1) % VIRELIA_STARTER_COUNT;
    RUN_OVERWORLD_SCRIPT(call VireliaPrologue_EventScript_TryGiveStarter;);
    EXPECT_EQ(gSpecialVar_Result, FALSE);
    EXPECT_EQ(gPartiesCount[B_TRAINER_PLAYER], 1);
    EXPECT_EQ(VarGet(VAR_VIRELIA_STARTER), choice);
    EXPECT_EQ(memcmp(&original, &gParties[B_TRAINER_PLAYER][0], sizeof(original)), 0);
}

TEST("Virelia compiled supply event waits for either rival outcome and grants only once")
{
    u32 outcome = B_OUTCOME_WON;
    PARAMETRIZE { outcome = B_OUTCOME_WON; }
    PARAMETRIZE { outcome = B_OUTCOME_LOST; }

    ResetPrologueEvents();
    VireliaPrologue_MeetMom();
    gSpecialVar_0x8004 = VIRELIA_STARTER_GRASS;
    RUN_OVERWORLD_SCRIPT(call VireliaPrologue_EventScript_TryGiveStarter;);
    RUN_OVERWORLD_SCRIPT(call VireliaPrologue_EventScript_TryGiveSupplies;);
    EXPECT_EQ(gSpecialVar_Result, FALSE);
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_POKE_BALL), 0);
    EXPECT(!FlagGet(FLAG_SYS_POKEDEX_GET));

    gBattleOutcome = outcome;
    VireliaPrologue_FinishRival();
    RUN_OVERWORLD_SCRIPT(call VireliaPrologue_EventScript_TryGiveSupplies;);
    EXPECT_EQ(gSpecialVar_Result, TRUE);
    EXPECT(FlagGet(FLAG_SYS_POKEDEX_GET));
    EXPECT(FlagGet(FLAG_VIRELIA_SUPPLIES_RECEIVED));
    EXPECT_EQ(VarGet(VAR_VIRELIA_RIVAL_RESULT), outcome);
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_POKE_BALL), VIRELIA_STARTING_POKE_BALLS);
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_POTION), VIRELIA_STARTING_POTIONS);

    RUN_OVERWORLD_SCRIPT(call VireliaPrologue_EventScript_TryGiveSupplies;);
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_POKE_BALL), VIRELIA_STARTING_POKE_BALLS);
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_POTION), VIRELIA_STARTING_POTIONS);
}

TEST("Virelia compiled wandering Pokemon quest rewards discovery once and retries a full bag")
{
    ResetPrologueEvents();
    RUN_OVERWORLD_SCRIPT(call VireliaPrologue_EventScript_TryRewardHana;);
    EXPECT_EQ(gSpecialVar_Result, FALSE);
    EXPECT(!FlagGet(FLAG_VIRELIA_SIDEQUEST_REWARDED));
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_POTION), 0);

    RUN_OVERWORLD_SCRIPT(call VireliaPrologue_EventScript_FindZigzagoon;);
    EXPECT(FlagGet(FLAG_VIRELIA_SIDEQUEST_RESOLVED));
    FillItemsPocket();
    EXPECT(!CheckBagHasSpace(ITEM_POTION, 1));
    RUN_OVERWORLD_SCRIPT(call VireliaPrologue_EventScript_TryRewardHana;);
    EXPECT_EQ(gSpecialVar_Result, FALSE);
    EXPECT(!FlagGet(FLAG_VIRELIA_SIDEQUEST_REWARDED));

    ClearBag();
    RUN_OVERWORLD_SCRIPT(call VireliaPrologue_EventScript_TryRewardHana;);
    EXPECT_EQ(gSpecialVar_Result, TRUE);
    EXPECT(FlagGet(FLAG_VIRELIA_SIDEQUEST_REWARDED));
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_POTION), 1);
    RUN_OVERWORLD_SCRIPT(call VireliaPrologue_EventScript_TryRewardHana;);
    EXPECT_EQ(gSpecialVar_Result, FALSE);
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_POTION), 1);
}

TEST("Virelia compiled route pickup remains available after full bag and cannot duplicate")
{
    ResetPrologueEvents();
    FillItemsPocket();
    RUN_OVERWORLD_SCRIPT(call VireliaPrologue_EventScript_TryTakePotion;);
    EXPECT_EQ(gSpecialVar_Result, FALSE);
    EXPECT(!FlagGet(FLAG_ITEM_VIRELIA_ROUTE01_POTION));

    ClearBag();
    RUN_OVERWORLD_SCRIPT(call VireliaPrologue_EventScript_TryTakePotion;);
    EXPECT_EQ(gSpecialVar_Result, TRUE);
    EXPECT(FlagGet(FLAG_ITEM_VIRELIA_ROUTE01_POTION));
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_POTION), 1);
    RUN_OVERWORLD_SCRIPT(call VireliaPrologue_EventScript_TryTakePotion;);
    EXPECT_EQ(gSpecialVar_Result, FALSE);
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_POTION), 1);
}

#endif // !IS_FRLG && P_VIRELIA_STARTERS
