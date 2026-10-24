#include "global.h"
#include "battle.h"
#include "battle_setup.h"
#include "bg.h"
#include "data.h"
#include "dma3.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "field_weather.h"
#include "main.h"
#include "menu.h"
#include "overworld.h"
#include "palette.h"
#include "pokemon.h"
#include "sprite.h"
#include "starter_choose.h"
#include "string_util.h"
#include "task.h"
#include "trainer_pokemon_sprites.h"
#include "virelia_prologue.h"
#include "window.h"
#include "test/test.h"
#include "constants/battle_partner.h"
#include "constants/battle_ai.h"
#include "constants/opponents.h"
#include "constants/event_objects.h"
#include "constants/trainers.h"
#include "constants/virelia_prologue.h"

#if !IS_FRLG && P_VIRELIA_STARTERS

static const struct Trainer sLabPartners[DIFFICULTY_COUNT][PARTNER_COUNT] =
{
#include "../src/data/battle_partners.h"
};

static void ReceiveLabStarter(u32 choice)
{
    InitEventData();
    ZeroPlayerPartyMons();
    FlagClear(FLAG_VIRELIA_STARTER_RECEIVED);
    FlagClear(FLAG_VIRELIA_MOM_MET);
    FlagClear(FLAG_VIRELIA_RIVAL_COMPLETE);
    FlagClear(FLAG_VIRELIA_LAB_RESCUE_COMPLETE);
    FlagClear(FLAG_VIRELIA_HIDE_LAB_ESCAPEES);
    VireliaPrologue_MeetMom();
    gSpecialVar_0x8004 = choice;
    VireliaPrologue_GiveStarter();
    EXPECT_EQ(gSpecialVar_Result, TRUE);
}

TEST("Virelia rescue prepares two wild opponents and the selected Viktor partner without changing ownership")
{
    u32 choice = 0;
    u32 gender = MALE;
    for (u32 i = 0; i < VIRELIA_STARTER_COUNT; i++)
        for (u32 j = MALE; j <= FEMALE; j++)
            PARAMETRIZE { choice = i; gender = j; }
    u32 previousGender = gSaveBlock2Ptr->playerGender;
    gSaveBlock2Ptr->playerGender = gender;
    ReceiveLabStarter(choice);
    struct Pokemon original = gParties[B_TRAINER_PLAYER][0];
    VireliaPrologue_PrepareRescue();
    EXPECT_EQ(gSpecialVar_Result, TRUE);
    EXPECT_EQ(gPartiesCount[B_TRAINER_PLAYER], 1);
    EXPECT_EQ(memcmp(&original, &gParties[B_TRAINER_PLAYER][0], sizeof(original)), 0);
    EXPECT_EQ(gPartiesCount[B_TRAINER_OPPONENT_A], 2);
    EXPECT_EQ(GetMonData(&gParties[B_TRAINER_OPPONENT_A][0], MON_DATA_SPECIES), SPECIES_MEOWTH);
    EXPECT_EQ(GetMonData(&gParties[B_TRAINER_OPPONENT_A][1], MON_DATA_SPECIES), SPECIES_RATTATA);
    EXPECT_EQ(GetMonData(&gParties[B_TRAINER_OPPONENT_A][0], MON_DATA_LEVEL), VIRELIA_LAB_WILD_LEVEL);
    EXPECT_EQ(GetMonData(&gParties[B_TRAINER_OPPONENT_A][1], MON_DATA_LEVEL), VIRELIA_LAB_WILD_LEVEL);
    EXPECT_EQ(gPartnerTrainerId, GetVireliaRescuePartner(choice));
    EXPECT_GE(gPartnerTrainerId, TRAINERS_COUNT);
    EXPECT(IsPartnerTrainerId(gPartnerTrainerId));
    const struct Trainer *partner = &sLabPartners[DIFFICULTY_NORMAL][GetPartnerIdFromTrainerId(gPartnerTrainerId)];
    EXPECT_EQ((u32)partner->partySize, 1);
    EXPECT_EQ(partner->party[0].species, GetVireliaRivalSpecies(choice));
    EXPECT_EQ(partner->party[0].lvl, VIRELIA_RIVAL_LEVEL);
    EXPECT_EQ((u32)partner->gender, gender == MALE ? FEMALE : MALE);
    EXPECT_EQ((u32)partner->trainerPic, gender == MALE ? TRAINER_PIC_RS_MAY : TRAINER_PIC_RS_BRENDAN);
    EXPECT_EQ(StringCompare(partner->trainerName, GetVireliaRivalName()), 0);
    EXPECT(!FlagGet(FLAG_VIRELIA_LAB_RESCUE_COMPLETE));
    EXPECT(!FlagGet(FLAG_VIRELIA_RIVAL_COMPLETE));
    gSaveBlock2Ptr->playerGender = previousGender;
}

TEST("Virelia rival name and overworld appearance use the opposite gender without truncating Viktorie")
{
    u32 gender = MALE;
    PARAMETRIZE { gender = MALE; }
    PARAMETRIZE { gender = FEMALE; }
    u32 previousGender = gSaveBlock2Ptr->playerGender;
    gSaveBlock2Ptr->playerGender = gender;
    InitEventData();
    VireliaPrologue_SetRivalAppearance();
    u32 expectedGfx = gender == MALE ? OBJ_EVENT_GFX_RIVAL_MAY_NORMAL : OBJ_EVENT_GFX_RIVAL_BRENDAN_NORMAL;
    EXPECT_EQ(GetVireliaRivalGender(), gender == MALE ? FEMALE : MALE);
    EXPECT_EQ(VarGet(VAR_OBJ_GFX_ID_0), expectedGfx);
    EXPECT_EQ(GetObjectEventGraphicsInfo(OBJ_EVENT_GFX_VAR_0), GetObjectEventGraphicsInfo(expectedGfx));
    const u8 *expectedName = gender == MALE ? COMPOUND_STRING("VIKTORIE") : COMPOUND_STRING("VIKTOR");
    StringExpandPlaceholders(gStringVar4, COMPOUND_STRING("{RIVAL}"));
    EXPECT_EQ(StringCompare(GetVireliaRivalName(), expectedName), 0);
    EXPECT_EQ(StringCompare(gStringVar4, expectedName), 0);
    gSaveBlock2Ptr->playerGender = previousGender;
}

TEST("Virelia rescue heals after either result and clears its ally state before the single duel")
{
    u32 outcome = B_OUTCOME_WON;
    PARAMETRIZE { outcome = B_OUTCOME_WON; }
    PARAMETRIZE { outcome = B_OUTCOME_LOST; }
    PARAMETRIZE { outcome = B_OUTCOME_DREW; }
    ReceiveLabStarter(VIRELIA_STARTER_WATER);
    VireliaPrologue_PrepareRescue();
    gBattleTypeFlags = BATTLE_TYPE_DOUBLE | BATTLE_TYPE_MULTI | BATTLE_TYPE_INGAME_PARTNER;
    gParties[B_TRAINER_PARTNER][0] = gParties[B_TRAINER_PLAYER][0];
    gPartiesCount[B_TRAINER_PARTNER] = 1;
    u16 hp = 1;
    SetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_HP, &hp);
    u32 exp = GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_EXP) + 1;
    SetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_EXP, &exp);
    gBattleOutcome = outcome;
    VireliaPrologue_FinishRescue();
    EXPECT_EQ(gSpecialVar_Result, TRUE);
    EXPECT(FlagGet(FLAG_VIRELIA_LAB_RESCUE_COMPLETE));
    EXPECT(FlagGet(FLAG_VIRELIA_HIDE_LAB_ESCAPEES));
    EXPECT(!FlagGet(FLAG_VIRELIA_RIVAL_COMPLETE));
    EXPECT_EQ(gPartiesCount[B_TRAINER_PLAYER], 1);
    EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_SPECIES), SPECIES_NATU_VIRELIA);
    EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_EXP), exp);
    EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_HP), GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_MAX_HP));
    EXPECT_EQ(gPartiesCount[B_TRAINER_PARTNER], 0);
    EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PARTNER][0], MON_DATA_SPECIES), SPECIES_NONE);
    EXPECT_EQ(gPartnerTrainerId, 0);
    EXPECT_EQ(gBattleTypeFlags, 0);
    VireliaPrologue_PrepareRescue();
    EXPECT_EQ(gSpecialVar_Result, FALSE);
}

TEST("Virelia fleeing the rescue cannot complete it or unlock the duel rewards")
{
    ReceiveLabStarter(VIRELIA_STARTER_GRASS);
    VireliaPrologue_PrepareRescue();
    gBattleOutcome = B_OUTCOME_RAN;
    VireliaPrologue_FinishRescue();
    EXPECT_EQ(gSpecialVar_Result, FALSE);
    EXPECT(!FlagGet(FLAG_VIRELIA_LAB_RESCUE_COMPLETE));
    EXPECT(!FlagGet(FLAG_VIRELIA_HIDE_LAB_ESCAPEES));
    EXPECT(!FlagGet(FLAG_VIRELIA_RIVAL_COMPLETE));
    VireliaPrologue_PrepareRescue();
    EXPECT_EQ(gSpecialVar_Result, TRUE);
    FlagClear(FLAG_VIRELIA_STARTER_RECEIVED);
    VireliaPrologue_PrepareRescue();
    EXPECT_EQ(gSpecialVar_Result, FALSE);
    EXPECT_EQ(GetVireliaRescuePartner(VIRELIA_STARTER_NONE), 0);
}

static void CaseFrame(u16 keys)
{
    gMain.newKeys = keys;
    gMain.newAndRepeatedKeys = keys;
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    UpdatePaletteFade();
    ClearDma3Requests();
    ClearSpriteCopyRequests();
}

static void WaitCaseFrames(u32 count)
{
    for (u32 i = 0; i < count; i++)
        CaseFrame(0);
}

TEST("Virelia briefcase previews all starters and handles No and B before returning a confirmed choice")
{
    u32 choice = 0;
    for (u32 i = 0; i < VIRELIA_STARTER_COUNT; i++)
        PARAMETRIZE { choice = i; }
    MainCallback originalMain = gMain.callback2;
    MainCallback originalSaved = gMain.savedCallback;
    IntrCallback originalVblank = gMain.vblankCallback;
    ResetTasks();
    ResetSpriteData();
    ResetAllPicSprites();
    FreeAllSpritePalettes();
    ResetPreservedPalettesInWeather();
    ResetBgsAndClearDma3BusyFlags(0);
    static const struct WindowTemplate noWindows[] = {DUMMY_WIN_TEMPLATE};
    InitWindows(noWindows);
    gSpecialVar_Result = 0xFFFF;
    ChooseVireliaStarter();
    gMain.callback2(); // Load the actual case, windows, sprites and input task.
    MainCallback caseMain = gMain.callback2;
    WaitCaseFrames(24);
    if (choice == VIRELIA_STARTER_GRASS)
        CaseFrame(DPAD_LEFT);
    else if (choice == VIRELIA_STARTER_FIRE)
        CaseFrame(DPAD_RIGHT);
    WaitCaseFrames(4);
    CaseFrame(A_BUTTON);
    WaitCaseFrames(40);
    CaseFrame(B_BUTTON);
    WaitCaseFrames(4);
    bool32 cancelled = gMain.callback2 == caseMain && gSpecialVar_Result == 0xFFFF;
    CaseFrame(A_BUTTON);
    WaitCaseFrames(40);
    CaseFrame(DPAD_DOWN);
    CaseFrame(A_BUTTON); // No.
    WaitCaseFrames(4);
    bool32 declined = gMain.callback2 == caseMain && gSpecialVar_Result == 0xFFFF;
    CaseFrame(A_BUTTON);
    WaitCaseFrames(40);
    CaseFrame(A_BUTTON); // Yes.
    u32 result = gSpecialVar_Result;
    bool32 returned = gMain.callback2 == CB2_ReturnToFieldContinueScriptPlayMapMusic;
    ClearDma3Requests();
    ClearSpriteCopyRequests();
    ClearScheduledBgCopiesToVram();
    FreeAllWindowBuffers();
    ResetAllPicSprites();
    ResetTasks();
    ResetSpriteData();
    FreeAllSpritePalettes();
    ResetPreservedPalettesInWeather();
    gMain.newKeys = 0;
    gMain.newAndRepeatedKeys = 0;
    gMain.callback2 = originalMain;
    gMain.savedCallback = originalSaved;
    SetVBlankCallback(originalVblank);
    EXPECT(returned);
    EXPECT(cancelled);
    EXPECT(declined);
    EXPECT_EQ(result, choice);
}

#endif

#include "test/battle.h"

#if !IS_FRLG && P_VIRELIA_STARTERS
TWO_VS_ONE_BATTLE_TEST("Virelia lab rescue supports one starter per ally and two opponents in one party")
{
    u32 choice = 0;
    for (u32 i = 0; i < VIRELIA_STARTER_COUNT; i++)
        PARAMETRIZE { choice = i; }
    GIVEN {
        PLAYER(GetVireliaStarterSpecies(choice)) { Level(5); Moves(MOVE_TACKLE); }
        PARTNER(GetVireliaRivalSpecies(choice)) { Level(5); Moves(MOVE_TACKLE); }
        OPPONENT(SPECIES_MEOWTH) { Level(3); Moves(MOVE_GROWL); }
        OPPONENT(SPECIES_RATTATA) { Level(3); Moves(MOVE_TAIL_WHIP); }
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_TACKLE, target: opponentLeft);
            MOVE(playerRight, MOVE_TACKLE, target: opponentRight);
            MOVE(opponentLeft, MOVE_GROWL);
            MOVE(opponentRight, MOVE_TAIL_WHIP);
        }
    } THEN {
        EXPECT_EQ(gPartiesCount[B_TRAINER_PLAYER], 1);
        EXPECT_EQ(gPartiesCount[B_TRAINER_PARTNER], 1);
        EXPECT_EQ(gPartiesCount[B_TRAINER_OPPONENT_A], 2);
        EXPECT_LT(opponentLeft->hp, opponentLeft->maxHP);
        EXPECT_LT(opponentRight->hp, opponentRight->maxHP);
    }
}
#endif
