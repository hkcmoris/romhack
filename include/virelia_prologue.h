#ifndef GUARD_VIRELIA_PROLOGUE_H
#define GUARD_VIRELIA_PROLOGUE_H

struct Pokemon;

void InitVireliaPrologueNewGame(void);
void VireliaPrologue_SyncStarterBallFlags(void);
u32 GetVireliaStarterCaseMask(void);
u32 GetVireliaStarterSpecies(u32 choice);
u32 GetVireliaRivalSpecies(u32 choice);
u32 GetVireliaRivalGender(void);
const u8 *GetVireliaRivalName(void);
void VireliaPrologue_SetRivalAppearance(void);
void VireliaPrologue_MeetMom(void);
void VireliaPrologue_GiveStarter(void);
u32 GetVireliaRescuePartner(u32 choice);
void VireliaPrologue_PrepareRescue(void);
void VireliaPrologue_StartRescueBattle(void);
void VireliaPrologue_FinishRescue(void);
void VireliaPrologue_MatchRivalLevel(struct Pokemon *party, u32 trainerId);
void VireliaPrologue_CompleteRival(void);
void VireliaPrologue_FinishRival(void);
void VireliaPrologue_GrantBasics(void);
void VireliaPrologue_GrantSupplies(void);
void VireliaPrologue_FinishRocket(void);
void VireliaPrologue_Complete(void);

#endif // GUARD_VIRELIA_PROLOGUE_H
