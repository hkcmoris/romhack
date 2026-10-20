#ifndef GUARD_VIRELIA_PROLOGUE_H
#define GUARD_VIRELIA_PROLOGUE_H

void InitVireliaPrologueNewGame(void);
u32 GetVireliaStarterSpecies(u32 choice);
u32 GetVireliaRivalSpecies(u32 choice);
void VireliaPrologue_MeetMom(void);
void VireliaPrologue_GiveStarter(void);
void VireliaPrologue_CompleteRival(void);
void VireliaPrologue_FinishRival(void);
void VireliaPrologue_GrantBasics(void);
void VireliaPrologue_GrantSupplies(void);
void VireliaPrologue_FinishRocket(void);
void VireliaPrologue_Complete(void);

#endif // GUARD_VIRELIA_PROLOGUE_H
