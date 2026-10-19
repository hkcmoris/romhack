@ Temporary access to the unchanged Battle Tower Link Multi challenge.
@ The shared Frontier status must be idle before entering another facility.
DuoMultiplayerTest_EventScript_TravelToTower::
	lock
	faceplayer
	frontier_get FRONTIER_DATA_CHALLENGE_STATUS
	goto_if_ne VAR_RESULT, 0, DuoMultiplayerTest_EventScript_ChallengeBusy
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
	.string "Finish your current FRONTIER\n"
	.string "challenge before using this trip.$"

DuoMultiplayerTest_Text_NeedParty:
	.string "Bring at least two POKéMON.\n"
	.string "The TOWER checks eligibility.$"
