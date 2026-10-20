@ Temporary access to the unchanged Battle Tower Link Multi challenge.
@ The shared Frontier status must be idle before entering another facility.
DuoMultiplayerTest_EventScript_TravelToTower::
	lock
	faceplayer
	frontier_get FRONTIER_DATA_CHALLENGE_STATUS
	goto_if_ne VAR_RESULT, 0, DuoMultiplayerTest_EventScript_ChallengeBusy
#if ORACULUM_DUO_TEST_QUICK_START && !IS_FRLG
#if VIRELIA_WORLD_TEST
	dynmultichoice 13, 4, FALSE, 4, 0, DYN_MULTICHOICE_CB_NONE, DuoMultiplayerTest_Text_Travel, DuoMultiplayerTest_Text_TestTeam, VireliaWorldTest_Text_Visit, gText_Cancel
#else
	dynmultichoice 13, 4, FALSE, 3, 0, DYN_MULTICHOICE_CB_NONE, DuoMultiplayerTest_Text_Travel, DuoMultiplayerTest_Text_TestTeam, gText_Cancel
#endif
	switch VAR_RESULT
	case 0, DuoMultiplayerTest_EventScript_OfferTravel
	case 1, DuoMultiplayerTest_EventScript_OptionalTeam
#if VIRELIA_WORLD_TEST
	case 2, VireliaWorldTest_EventScript_OfferTravel
#endif
	goto DuoMultiplayerTest_EventScript_Cancel

DuoMultiplayerTest_EventScript_OfferTravel:
#elif VIRELIA_WORLD_TEST && !IS_FRLG
	dynmultichoice 13, 4, FALSE, 3, 0, DYN_MULTICHOICE_CB_NONE, DuoMultiplayerTest_Text_Travel, VireliaWorldTest_Text_Visit, gText_Cancel
	switch VAR_RESULT
	case 0, DuoMultiplayerTest_EventScript_OfferTravel
	case 1, VireliaWorldTest_EventScript_OfferTravel
	goto DuoMultiplayerTest_EventScript_Cancel

DuoMultiplayerTest_EventScript_OfferTravel:
#endif
	getpartysize
	goto_if_lt VAR_RESULT, 2, DuoMultiplayerTest_EventScript_NeedParty
	msgbox DuoMultiplayerTest_Text_TravelToTower, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, DuoMultiplayerTest_EventScript_Cancel
	closemessage
	warp MAP_BATTLE_FRONTIER_BATTLE_TOWER_LOBBY, 18, 6
	waitstate
	end

DuoMultiplayerTest_EventScript_ReturnToLittleroot::
	lock
	faceplayer
	frontier_get FRONTIER_DATA_CHALLENGE_STATUS
	goto_if_ne VAR_RESULT, 0, DuoMultiplayerTest_EventScript_ChallengeBusy
	msgbox DuoMultiplayerTest_Text_ReturnToLittleroot, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, DuoMultiplayerTest_EventScript_Cancel
	closemessage
	warp MAP_LITTLEROOT_TOWN, 17, 13
	waitstate
	end

DuoMultiplayerTest_EventScript_ChallengeBusy:
	msgbox DuoMultiplayerTest_Text_ChallengeBusy, MSGBOX_DEFAULT
	release
	end

DuoMultiplayerTest_EventScript_NeedParty:
	msgbox DuoMultiplayerTest_Text_NeedParty, MSGBOX_DEFAULT
	release
	end

DuoMultiplayerTest_EventScript_Cancel:
	release
	end

DuoMultiplayerTest_Text_TravelToTower:
	.string "This is the DUO LINK test.\n"
	.string "Back up your save before testing!\p"
	.string "The BATTLE TOWER requires saving.\n"
	.string "Bring two eligible POKéMON each.\p"
	.string "Speak to the rightmost attendant.\n"
	.string "Choose LINK MULTI on both games.\p"
	.string "The scientist there can bring you\n"
	.string "back. Travel to the TOWER?$"

DuoMultiplayerTest_Text_ReturnToLittleroot:
	.string "Finished testing?\n"
	.string "Return to LITTLEROOT TOWN?$"

DuoMultiplayerTest_Text_ChallengeBusy:
	.string "Nejprve dokonči současnou výzvu\n"
	.string "v Bitevní zóně. Pak můžeš cestovat.$"

DuoMultiplayerTest_Text_NeedParty:
	.string "Bring at least two POKéMON.\n"
	.string "The TOWER checks eligibility.$"

#if ORACULUM_DUO_TEST_QUICK_START && !IS_FRLG
@ Only NewGameInitData marks this save pending. CONTINUE never does so.
DuoMultiplayerTest_EventScript_FirstEntry::
	lockall
	getpartysize
	goto_if_ne VAR_RESULT, 0, DuoMultiplayerTest_EventScript_DeferTeam
	call DuoMultiplayerTest_EventScript_ChooseTeam
	goto_if_eq VAR_RESULT, FALSE, DuoMultiplayerTest_EventScript_DeferTeam
	msgbox DuoMultiplayerTest_Text_TeamReady, MSGBOX_DEFAULT
	releaseall
	end

DuoMultiplayerTest_EventScript_DeferTeam:
	setvar VAR_ORACULUM_DUO_TEST_STATE, 0
	msgbox DuoMultiplayerTest_Text_UseOptionalSetup, MSGBOX_DEFAULT
	releaseall
	end

DuoMultiplayerTest_EventScript_OptionalTeam:
	goto_if_ge VAR_ORACULUM_DUO_TEST_STATE, ORACULUM_DUO_TEST_TEAM_A, DuoMultiplayerTest_EventScript_AlreadyReceived
	getpartysize
	goto_if_gt VAR_RESULT, 4, DuoMultiplayerTest_EventScript_NoRoom
	msgbox DuoMultiplayerTest_Text_ConfirmTeam, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, DuoMultiplayerTest_EventScript_Cancel
	closemessage
	call DuoMultiplayerTest_EventScript_ChooseTeam
	goto_if_eq VAR_RESULT, FALSE, DuoMultiplayerTest_EventScript_NoRoom
	msgbox DuoMultiplayerTest_Text_TeamReady, MSGBOX_DEFAULT
	release
	end

DuoMultiplayerTest_EventScript_AlreadyReceived:
	msgbox DuoMultiplayerTest_Text_AlreadyReceived, MSGBOX_DEFAULT
	release
	end

DuoMultiplayerTest_EventScript_NoRoom:
	msgbox DuoMultiplayerTest_Text_NoRoom, MSGBOX_DEFAULT
	release
	end

DuoMultiplayerTest_EventScript_ChooseTeam:
	msgbox DuoMultiplayerTest_Text_DifferentTeams, MSGBOX_DEFAULT
	message DuoMultiplayerTest_Text_SelectTeam
	waitmessage
	dynmultichoice 17, 6, TRUE, 2, 0, DYN_MULTICHOICE_CB_NONE, DuoMultiplayerTest_Text_TeamA, DuoMultiplayerTest_Text_TeamB
	closemessage
	goto_if_eq VAR_RESULT, 0, DuoMultiplayerTest_EventScript_GiveTeamA
	goto DuoMultiplayerTest_EventScript_GiveTeamB

@ Check both capacity and the persisted receipt before either grant. givemon
@ appends into empty slots; it never targets an occupied slot or sends to PC here.
DuoMultiplayerTest_EventScript_CanGiveTeam:
	goto_if_ge VAR_ORACULUM_DUO_TEST_STATE, ORACULUM_DUO_TEST_TEAM_A, DuoMultiplayerTest_EventScript_CannotGiveTeam
	getpartysize
	goto_if_gt VAR_RESULT, 4, DuoMultiplayerTest_EventScript_CannotGiveTeam
	setvar VAR_RESULT, TRUE
	return

DuoMultiplayerTest_EventScript_CannotGiveTeam:
	setvar VAR_RESULT, FALSE
	return

	.macro duo_test_givemon species:req, nature:req, ability:req, item:req, hp:req, atk:req, speed:req, spatk:req, spdef:req, move1:req, move2:req, move3:req
	givemon \species, 50, item=\item, nature=\nature, abilityNum=\ability, hpEv=\hp, atkEv=\atk, defEv=0, speedEv=\speed, spAtkEv=\spatk, spDefEv=\spdef, hpIv=31, atkIv=31, defIv=31, speedIv=31, spAtkIv=31, spDefIv=31, move1=\move1, move2=\move2, move3=\move3, move4=MOVE_PROTECT
	.endm

DuoMultiplayerTest_EventScript_GiveTeamA::
	call DuoMultiplayerTest_EventScript_CanGiveTeam
	goto_if_eq VAR_RESULT, FALSE, DuoMultiplayerTest_EventScript_CannotGiveTeam
	duo_test_givemon SPECIES_GARDEVOIR, NATURE_MODEST, 2, ITEM_SITRUS_BERRY, 4, 0, 252, 252, 0, MOVE_PSYCHIC, MOVE_DAZZLING_GLEAM, MOVE_HELPING_HAND
	duo_test_givemon SPECIES_BRELOOM, NATURE_ADAMANT, 2, ITEM_FOCUS_SASH, 4, 252, 252, 0, 0, MOVE_SPORE, MOVE_SEED_BOMB, MOVE_MACH_PUNCH
	setvar VAR_ORACULUM_DUO_TEST_STATE, ORACULUM_DUO_TEST_TEAM_A
	setflag FLAG_SYS_POKEMON_GET
	setvar VAR_RESULT, TRUE
	return

DuoMultiplayerTest_EventScript_GiveTeamB::
	call DuoMultiplayerTest_EventScript_CanGiveTeam
	goto_if_eq VAR_RESULT, FALSE, DuoMultiplayerTest_EventScript_CannotGiveTeam
	duo_test_givemon SPECIES_SWAMPERT, NATURE_ADAMANT, 0, ITEM_LEFTOVERS, 252, 252, 0, 0, 4, MOVE_WATERFALL, MOVE_EARTHQUAKE, MOVE_ICE_PUNCH
	duo_test_givemon SPECIES_MANECTRIC, NATURE_TIMID, 1, ITEM_MAGNET, 4, 0, 252, 252, 0, MOVE_THUNDERBOLT, MOVE_FLAMETHROWER, MOVE_SNARL
	setvar VAR_ORACULUM_DUO_TEST_STATE, ORACULUM_DUO_TEST_TEAM_B
	setflag FLAG_SYS_POKEMON_GET
	setvar VAR_RESULT, TRUE
	return

DuoMultiplayerTest_Text_TestTeam:
	.string "TEST TEAM SETUP$"
DuoMultiplayerTest_Text_TeamA:
	.string "TEAM A$"
DuoMultiplayerTest_Text_TeamB:
	.string "TEAM B$"
DuoMultiplayerTest_Text_SelectTeam:
	.string "DUO TEST: SELECT TEAM$"
DuoMultiplayerTest_Text_DifferentTeams:
	.string "Use different teams on each game!\p"
	.string "TEAM A: GARDEVOIR and BRELOOM\n"
	.string "TEAM B: SWAMPERT and MANECTRIC$"
DuoMultiplayerTest_Text_TeamReady:
	.string "Your two Lv. 50 POKéMON are ready!\n"
	.string "Save normally to keep this setup.\p"
	.string "Talk to the scientist to travel\n"
	.string "to the LINK MULTI attendant.$"
DuoMultiplayerTest_Text_ConfirmTeam:
	.string "Add two Lv. 50 test POKéMON?\n"
	.string "Your current POKéMON stay with you.\p"
	.string "This is a one-time test setup.\n"
	.string "Add a test team?$"
DuoMultiplayerTest_Text_AlreadyReceived:
	.string "This save already received a team.\n"
	.string "No more test POKéMON were added.$"
DuoMultiplayerTest_Text_NoRoom:
	.string "Make two empty party slots first.\n"
	.string "Store POKéMON in a PC, then return.$"
DuoMultiplayerTest_Text_UseOptionalSetup:
	.string "Your existing party was kept.\n"
	.string "Ask the scientist for TEST TEAM SETUP.$"
#endif

#if (ORACULUM_DUO_TEST_QUICK_START || VIRELIA_WORLD_TEST) && !IS_FRLG
DuoMultiplayerTest_Text_Travel:
	.string "TRAVEL TO TOWER$"
#endif
