#include "global.h"
#include "battle_setup.h"
#include "event_data.h"
#include "item.h"
#include "virelia_prologue.h"
#include "virelia_rainfall.h"
#include "constants/items.h"
#include "constants/opponents.h"
#include "constants/virelia_prologue.h"
#include "constants/virelia_first_gym.h"
#include "constants/virelia_rainfall.h"

static bool32 ChapterAvailable(void)
{
#if VIRELIA_PROLOGUE && !VIRELIA_DEVELOPMENT_MODE && !IS_FRLG
    return FlagGet(FLAG_VIRELIA_FIRST_GYM_COMPLETE);
#else
    return FALSE;
#endif
}

u16 VireliaRainfall_TrainerFlag(u16 trainerId)
{
#if !IS_FRLG
    if (trainerId >= TRAINER_VIRELIA_RAINFALL_HIKER && trainerId <= TRAINER_VIRELIA_RAINFALL_WEATHER)
        return FLAG_VIRELIA_TRAINER_RAINFALL_HIKER + trainerId - TRAINER_VIRELIA_RAINFALL_HIKER;
    if (trainerId >= TRAINER_VIRELIA_VIKTOR_RAINFALL_GRASS && trainerId <= TRAINER_VIRELIA_VIKTORIE_RAINFALL_FIRE)
        return FLAG_VIRELIA_TRAINER_RAINFALL_RIVAL;
#endif
    return TRAINER_FLAGS_START + trainerId;
}

u16 VireliaRainfall_RivalTrainer(u32 choice)
{
#if !IS_FRLG
    static const u16 trainers[2][VIRELIA_STARTER_COUNT] =
    {
        [MALE] = {TRAINER_VIRELIA_VIKTOR_RAINFALL_GRASS, TRAINER_VIRELIA_VIKTOR_RAINFALL_WATER, TRAINER_VIRELIA_VIKTOR_RAINFALL_FIRE},
        [FEMALE] = {TRAINER_VIRELIA_VIKTORIE_RAINFALL_GRASS, TRAINER_VIRELIA_VIKTORIE_RAINFALL_WATER, TRAINER_VIRELIA_VIKTORIE_RAINFALL_FIRE},
    };
    if (choice < VIRELIA_STARTER_COUNT)
        return trainers[GetVireliaRivalGender()][choice];
#endif
    return 0;
}

void VireliaRainfall_BeginChapter(void)
{
    gSpecialVar_Result = ChapterAvailable();
    if (gSpecialVar_Result)
        FlagSet(FLAG_VIRELIA_RAINFALL_STARTED);
}

void VireliaRainfall_FinishRival(void)
{
    u16 trainer = VireliaRainfall_RivalTrainer(VarGet(VAR_VIRELIA_STARTER));
    // Ordinary trainer setup writes victory only on a win; losses white out.
    gSpecialVar_Result = ChapterAvailable() && trainer && HasTrainerBeenFought(trainer);
    if (gSpecialVar_Result)
        FlagSet(FLAG_VIRELIA_RAINFALL_RIVAL_COMPLETE);
}

void VireliaRainfall_OpenShortcut(void)
{
    gSpecialVar_Result = ChapterAvailable();
    if (gSpecialVar_Result)
        FlagSet(FLAG_VIRELIA_LOWER_SHORTCUT);
}

void VireliaRainfall_HarborReport(void)
{
    gSpecialVar_Result = ChapterAvailable() && FlagGet(FLAG_VIRELIA_RAINFALL_RIVAL_COMPLETE);
    if (gSpecialVar_Result)
        FlagSet(FLAG_VIRELIA_HARBOR_REPORT);
}

void VireliaRainfall_InspectDamage(void)
{
    gSpecialVar_Result = ChapterAvailable() && FlagGet(FLAG_VIRELIA_HARBOR_REPORT);
    if (gSpecialVar_Result)
        FlagSet(FLAG_VIRELIA_LIGHTHOUSE_DAMAGE);
}

void VireliaRainfall_ResetSignal(void)
{
    VarSet(VAR_VIRELIA_SIGNAL_INPUT, 0);
}

void VireliaRainfall_SignalSwitch(void)
{
    gSpecialVar_Result = FALSE;
    if (!ChapterAvailable() || !FlagGet(FLAG_VIRELIA_LIGHTHOUSE_DAMAGE))
        return;
    if (FlagGet(FLAG_VIRELIA_LIGHTHOUSE_REPAIRED))
    {
        gSpecialVar_Result = TRUE;
        return;
    }
    u32 stage = VarGet(VAR_VIRELIA_SIGNAL_INPUT);
    if (stage >= 3 || gSpecialVar_0x8004 != stage + 1)
    {
        VarSet(VAR_VIRELIA_SIGNAL_INPUT, 0);
        return;
    }
    VarSet(VAR_VIRELIA_SIGNAL_INPUT, ++stage);
    if (stage == 3)
    {
        FlagSet(FLAG_VIRELIA_LIGHTHOUSE_REPAIRED);
        gSpecialVar_Result = TRUE;
    }
}

void VireliaRainfall_ReadManifest(void)
{
    gSpecialVar_Result = ChapterAvailable() && FlagGet(FLAG_VIRELIA_LIGHTHOUSE_REPAIRED);
    if (gSpecialVar_Result)
        FlagSet(FLAG_VIRELIA_SHIPPING_RECORD);
}

void VireliaRainfall_CompleteChapter(void)
{
    gSpecialVar_Result = ChapterAvailable() && FlagGet(FLAG_VIRELIA_LIGHTHOUSE_REPAIRED)
        && FlagGet(FLAG_VIRELIA_SHIPPING_RECORD);
    if (gSpecialVar_Result)
    {
        FlagSet(FLAG_VIRELIA_HARBOR_REOPENED);
        FlagSet(FLAG_VIRELIA_RAINFALL_COMPLETE);
    }
}

void VireliaRainfall_SyncTownActors(void)
{
    if (FlagGet(FLAG_VIRELIA_RAINFALL_COMPLETE))
    {
        FlagClear(FLAG_VIRELIA_HIDE_VACEK);
        FlagClear(FLAG_VIRELIA_HIDE_HARBOR_BELA);
    }
    else
    {
        FlagSet(FLAG_VIRELIA_HIDE_VACEK);
        FlagSet(FLAG_VIRELIA_HIDE_HARBOR_BELA);
    }
}

void VireliaRainfall_BeginBottle(void)
{
    gSpecialVar_Result = ChapterAvailable();
    if (gSpecialVar_Result && VarGet(VAR_VIRELIA_BOTTLE_STAGE) == 0)
        VarSet(VAR_VIRELIA_BOTTLE_STAGE, 1);
}

void VireliaRainfall_BottleClue(void)
{
    u32 stage = VarGet(VAR_VIRELIA_BOTTLE_STAGE);
    gSpecialVar_Result = ChapterAvailable() && stage >= 1 && stage < 4
        && gSpecialVar_0x8004 == stage + 1;
    if (gSpecialVar_Result)
        VarSet(VAR_VIRELIA_BOTTLE_STAGE, stage + 1);
}

static void GrantOnce(u16 flag, u16 item)
{
    gSpecialVar_Result = FALSE;
    if (!ChapterAvailable() || FlagGet(flag) || !AddBagItem(item, 1))
        return;
    FlagSet(flag);
    gSpecialVar_Result = TRUE;
}

void VireliaRainfall_RewardBottle(void)
{
    gSpecialVar_Result = FALSE;
    if (VarGet(VAR_VIRELIA_BOTTLE_STAGE) != 4)
        return;
    GrantOnce(FLAG_VIRELIA_BOTTLE_REWARDED, ITEM_MYSTIC_WATER);
}

void VireliaRainfall_TakeRouteItem(void)
{
    if (gSpecialVar_0x8004 == 1)
        GrantOnce(FLAG_VIRELIA_RAINFALL_ITEM_REPEL, ITEM_REPEL);
    else if (gSpecialVar_0x8004 == 2)
        GrantOnce(FLAG_VIRELIA_RAINFALL_ITEM_SUPER_POTION, ITEM_SUPER_POTION);
    else
        gSpecialVar_Result = FALSE;
}