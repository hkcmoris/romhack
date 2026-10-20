#if !IS_FRLG
@ A development-only gateway. The existing Duo guide owns the initial lock
@ and checks Frontier status before offering this destination.
#if VIRELIA_WORLD_TEST
VireliaWorldTest_EventScript_OfferTravel::
	call VireliaWorldTest_EventScript_CheckEntry
	goto_if_eq VAR_RESULT, FALSE, VireliaWorldTest_EventScript_NeedPokemon
	msgbox VireliaWorldTest_Text_Travel, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, DuoMultiplayerTest_EventScript_Cancel
	closemessage
	warp MAP_VIRELIA_MAPLEBROOK_TOWN, 14, 21
	waitstate
	end

@ Separate from UI so the native script test can exercise entry eligibility.
VireliaWorldTest_EventScript_CheckEntry::
	setvar VAR_0x8004, PARTY_SIZE
	specialvar VAR_RESULT, CountPartyAliveNonEggMons_IgnoreVar0x8004Slot
	goto_if_eq VAR_RESULT, 0, VireliaWorldTest_EventScript_CannotEnter
	setvar VAR_RESULT, TRUE
	return

VireliaWorldTest_EventScript_CannotEnter:
	setvar VAR_RESULT, FALSE
	return

VireliaWorldTest_EventScript_NeedPokemon:
	msgbox VireliaWorldTest_Text_NeedPokemon, MSGBOX_DEFAULT
	release
	end

VireliaWorldTest_Text_Visit:
	.string "VISIT VIRELIA$"
VireliaWorldTest_Text_Travel:
	.string "Explore the VIRELIA map prototype?\n"
	.string "The trail has wild POKéMON.\p"
	.string "A guide in MAPLEBROOK TOWN can\n"
	.string "bring you back here. Travel now?$"
VireliaWorldTest_Text_NeedPokemon:
	.string "Bring a healthy POKéMON first.\n"
	.string "The trail has wild POKéMON.$"
#endif

@ Keep the exit available even when development entry is disabled. This
@ prevents a save already inside the prototype from losing its way to Hoenn.
VireliaWorldTest_EventScript_ReturnToLittleroot::
	lock
	faceplayer
	frontier_get FRONTIER_DATA_CHALLENGE_STATUS
	goto_if_ne VAR_RESULT, 0, DuoMultiplayerTest_EventScript_ChallengeBusy
	msgbox VireliaWorldTest_Text_Return, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, DuoMultiplayerTest_EventScript_Cancel
	closemessage
	warp MAP_LITTLEROOT_TOWN, 17, 13
	waitstate
	end

VireliaWorldTest_Text_Return:
	.string "Return to LITTLEROOT TOWN?\n"
	.string "Your party and items travel with you.$"
#endif
