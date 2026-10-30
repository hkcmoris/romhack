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
	warp MAP_VIRELIA_MAPLEBROOK_TOWN, 9, 47
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
	.string "NAVŠTÍVIT VIRELII$"
VireliaWorldTest_Text_Travel:
	.string "Prozkoumáš prototyp VIRELIE?\n"
	.string "Na stezce žijí divocí Pokémoni.\p"
	.string "Průvodce v Maplebrooku tě\n"
	.string "vezme zpátky. Vyrazíme?$"
VireliaWorldTest_Text_NeedPokemon:
	.string "Vezmi s sebou zdravého Pokémona.\n"
	.string "Na stezce žijí divocí Pokémoni.$"
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
	.string "Vrátit se do městečka LITTLEROOT?\n"
	.string "Pokémoni i předměty půjdou s tebou.$"
#endif
