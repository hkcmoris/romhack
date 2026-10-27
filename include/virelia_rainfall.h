#ifndef GUARD_VIRELIA_RAINFALL_H
#define GUARD_VIRELIA_RAINFALL_H

void VireliaRainfall_BeginChapter(void);
void VireliaRainfall_FinishRival(void);
u16 VireliaRainfall_RivalTrainer(u32 choice);
u16 VireliaRainfall_TrainerFlag(u16 trainerId);
void VireliaRainfall_OpenShortcut(void);
void VireliaRainfall_HarborReport(void);
void VireliaRainfall_InspectDamage(void);
void VireliaRainfall_ResetSignal(void);
void VireliaRainfall_SignalSwitch(void);
void VireliaRainfall_ReadManifest(void);
void VireliaRainfall_CompleteChapter(void);
void VireliaRainfall_SyncTownActors(void);
void VireliaRainfall_BeginBottle(void);
void VireliaRainfall_BottleClue(void);
void VireliaRainfall_RewardBottle(void);
void VireliaRainfall_TakeRouteItem(void);

#endif