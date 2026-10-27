#include "global.h"
#include "battle_setup.h"
#include "event_data.h"
#include "item.h"
#include "virelia_first_gym.h"
#include "virelia_rainfall.h"
#include "constants/items.h"
#include "constants/battle.h"
#include "constants/opponents.h"
#include "constants/virelia_prologue.h"
#include "constants/virelia_first_gym.h"

// Preserve the old trainer flag bank, SYSTEM_FLAGS, and all partner identifiers.
u16 VireliaFirstGym_TrainerFlag(u16 trainerId)
{
#if !IS_FRLG
    if (trainerId >= TRAINER_VIRELIA_MINER_BRUNO && trainerId <= TRAINER_VIRELIA_MARA)
        return FLAG_VIRELIA_TRAINER_BRUNO + trainerId - TRAINER_VIRELIA_MINER_BRUNO;
#endif
    return VireliaRainfall_TrainerFlag(trainerId);
}

static bool32 ChapterAvailable(void)
{
#if VIRELIA_PROLOGUE && !VIRELIA_DEVELOPMENT_MODE && !IS_FRLG
    return FlagGet(FLAG_VIRELIA_PROLOGUE_COMPLETE);
#else
    return FALSE;
#endif
}

void VireliaFirstGym_BeginChapter(void)
{
    gSpecialVar_Result = ChapterAvailable();
    if (gSpecialVar_Result)
        FlagSet(FLAG_VIRELIA_CHAPTER_STARTED);
    // Never reset or advance the completed prologue on entry or Continue.
}

void VireliaFirstGym_ResetQuarryPuzzle(void)
{
    VarSet(VAR_VIRELIA_QUARRY_SWITCH, 0);
}

static void AdvancePuzzle(u16 var, u16 flag, bool32 available)
{
    gSpecialVar_Result = FALSE;
    if (!available)
        return;
    if (FlagGet(flag))
    {
        gSpecialVar_Result = TRUE;
        return;
    }
    u32 stage = VarGet(var);
    if (gSpecialVar_0x8004 != stage + 1 || stage >= 2)
    {
        VarSet(var, 0);
        return;
    }
    VarSet(var, ++stage);
    if (stage == 2)
    {
        FlagSet(flag);
        gSpecialVar_Result = TRUE;
    }
}

void VireliaFirstGym_QuarrySwitch(void)
{
    AdvancePuzzle(VAR_VIRELIA_QUARRY_SWITCH, FLAG_VIRELIA_QUARRY_POWER_RESTORED,
                  ChapterAvailable() && FlagGet(FLAG_VIRELIA_CHAPTER_STARTED));
}

void VireliaFirstGym_ReadEvidence(void)
{
    gSpecialVar_Result = ChapterAvailable()
        && FlagGet(FLAG_VIRELIA_QUARRY_POWER_RESTORED)
        && HasTrainerBeenFought(TRAINER_VIRELIA_MINER_BRUNO)
        && HasTrainerBeenFought(TRAINER_VIRELIA_EXPLORER_LENKA);
    if (gSpecialVar_Result)
        FlagSet(FLAG_VIRELIA_QUARRY_EVIDENCE);
}

void VireliaFirstGym_CompleteQuarry(void)
{
    gSpecialVar_Result = ChapterAvailable() && FlagGet(FLAG_VIRELIA_QUARRY_EVIDENCE);
    if (gSpecialVar_Result)
        FlagSet(FLAG_VIRELIA_QUARRY_COMPLETE);
}

void VireliaFirstGym_ResetGymPuzzle(void)
{
    VarSet(VAR_VIRELIA_GYM_SWITCH, 0);
}

void VireliaFirstGym_GymSwitch(void)
{
    AdvancePuzzle(VAR_VIRELIA_GYM_SWITCH, FLAG_VIRELIA_GYM_PUZZLE_COMPLETE,
                  ChapterAvailable() && FlagGet(FLAG_VIRELIA_QUARRY_COMPLETE));
}

void VireliaFirstGym_AwardBadge(void)
{
    gSpecialVar_Result = FALSE;
    if (!ChapterAvailable()
     || !FlagGet(FLAG_VIRELIA_QUARRY_COMPLETE)
     || !FlagGet(FLAG_VIRELIA_GYM_PUZZLE_COMPLETE)
     || !HasTrainerBeenFought(TRAINER_VIRELIA_GYM_IVAN)
     || !HasTrainerBeenFought(TRAINER_VIRELIA_GYM_DANA)
     || !HasTrainerBeenFought(TRAINER_VIRELIA_MARA)
     || FlagGet(FLAG_VIRELIA_BADGE_QUARRY))
        return;
    FlagSet(FLAG_VIRELIA_BADGE_QUARRY);
    gSpecialVar_Result = TRUE;
    // Recognition only: no implicit Hoenn stat boost, obedience or field move.
}

static void GrantOnce(u16 flag, u16 item)
{
    gSpecialVar_Result = FALSE;
    if (FlagGet(flag) || !AddBagItem(item, 1))
        return;
    FlagSet(flag);
    gSpecialVar_Result = TRUE;
}

void VireliaFirstGym_AwardTM(void)
{
    gSpecialVar_Result = FALSE;
    if (!ChapterAvailable() || !FlagGet(FLAG_VIRELIA_BADGE_QUARRY))
        return;
    GrantOnce(FLAG_VIRELIA_TM_ROCK_TOMB_RECEIVED, ITEM_TM_ROCK_TOMB);
    if (gSpecialVar_Result)
        FlagSet(FLAG_VIRELIA_FIRST_GYM_COMPLETE);
}

void VireliaFirstGym_TakeQuarryItem(void)
{
    gSpecialVar_Result = FALSE;
    if (!ChapterAvailable() || !FlagGet(FLAG_VIRELIA_CHAPTER_STARTED))
        return;
    GrantOnce(FLAG_VIRELIA_ITEM_QUARRY_SUPER_POTION, ITEM_SUPER_POTION);
}

static bool32 MissingToolsAvailable(void)
{
    return ChapterAvailable() && FlagGet(FLAG_VIRELIA_QUARRY_POWER_RESTORED);
}

void VireliaFirstGym_BeginMissingTools(void)
{
    gSpecialVar_Result = MissingToolsAvailable();
    if (gSpecialVar_Result)
        FlagSet(FLAG_VIRELIA_MISSING_TOOLS_STARTED);
}

void VireliaFirstGym_CollectTool(void)
{
    gSpecialVar_Result = FALSE;
    if (!MissingToolsAvailable() || !FlagGet(FLAG_VIRELIA_MISSING_TOOLS_STARTED)
     || gSpecialVar_0x8004 < 1 || gSpecialVar_0x8004 > 3)
        return;
    FlagSet(FLAG_VIRELIA_TOOL_WRENCH + gSpecialVar_0x8004 - 1);
    gSpecialVar_Result = TRUE;
}

void VireliaFirstGym_RecoverTools(void)
{
    gSpecialVar_Result = FALSE;
    if (!MissingToolsAvailable() || !FlagGet(FLAG_VIRELIA_MISSING_TOOLS_STARTED)
     || !FlagGet(FLAG_VIRELIA_TOOL_WRENCH) || !FlagGet(FLAG_VIRELIA_TOOL_HAMMER)
     || !FlagGet(FLAG_VIRELIA_TOOL_PICK)
     || (gSpecialVar_0x8004 != B_OUTCOME_WON && gSpecialVar_0x8004 != B_OUTCOME_CAUGHT))
        return;
    FlagSet(FLAG_VIRELIA_MISSING_TOOLS_RECOVERED);
    gSpecialVar_Result = TRUE;
}

void VireliaFirstGym_RewardTools(void)
{
    gSpecialVar_Result = FALSE;
    if (!MissingToolsAvailable() || !FlagGet(FLAG_VIRELIA_MISSING_TOOLS_RECOVERED))
        return;
    GrantOnce(FLAG_VIRELIA_MISSING_TOOLS_REWARDED, ITEM_HARD_STONE);
}
