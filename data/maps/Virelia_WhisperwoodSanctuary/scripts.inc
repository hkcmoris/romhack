Virelia_WhisperwoodSanctuary_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Virelia_WhisperwoodSanctuary_OnTransition
	map_script MAP_SCRIPT_ON_LOAD, Virelia_WhisperwoodSanctuary_OnLoad
	.byte 0

Virelia_WhisperwoodSanctuary_OnTransition:
	call VireliaSanctuary_EventScript_ResetStones
	end

Virelia_WhisperwoodSanctuary_OnLoad:
	@ Restore the whole custom tree over cached views from earlier playtest ROMs.
	setmetatile 8, 6, 0x31D, 1
	setmetatile 9, 6, 0x31E, 1
	setmetatile 10, 6, 0x31F, 1
	setmetatile 8, 7, 0x320, 1
	setmetatile 9, 7, 0x321, 1
	setmetatile 10, 7, 0x322, 1
	setmetatile 8, 8, 0x323, 1
	setmetatile 9, 8, 0x324, 1
	setmetatile 10, 8, 0x325, 1
	setmetatile 8, 9, 0x326, 1
	setmetatile 9, 9, 0x327, 1
	setmetatile 10, 9, 0x328, 1
	setmetatile 20, 10, 0xE, 1
	setmetatile 20, 11, 0xE, 1
	goto_if_unset FLAG_VIRELIA_SIDEQUEST_REWARDED, Virelia_WhisperwoodSanctuary_CheckShortcut
	setmetatile 20, 10, 0xD, 0
	setmetatile 20, 11, 0xD, 0
Virelia_WhisperwoodSanctuary_CheckShortcut:
	setmetatile 16, 2, 0xE, 1
	setmetatile 16, 3, 0xE, 1
	goto_if_unset FLAG_VIRELIA_SANCTUARY_SHORTCUT_OPEN, Virelia_WhisperwoodSanctuary_LoadDone
	call Virelia_WhisperwoodSanctuary_OpenShortcut
Virelia_WhisperwoodSanctuary_LoadDone:
	end

Virelia_WhisperwoodSanctuary_OpenShortcut:
	setmetatile 16, 2, 0x1, 0
	setmetatile 16, 3, 0x1, 0
	return

Virelia_WhisperwoodSanctuary_EventScript_SeedStone::
	lockall
	goto_if_set FLAG_VIRELIA_SANCTUARY_SHORTCUT_OPEN, Virelia_WhisperwoodSanctuary_EventScript_StonesQuiet
	msgbox Virelia_WhisperwoodSanctuary_Text_Seed, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Virelia_WhisperwoodSanctuary_EventScript_Release
	setvar VAR_0x8004, VIRELIA_SANCTUARY_SEED
	goto Virelia_WhisperwoodSanctuary_EventScript_PressStone

Virelia_WhisperwoodSanctuary_EventScript_RainStone::
	lockall
	goto_if_set FLAG_VIRELIA_SANCTUARY_SHORTCUT_OPEN, Virelia_WhisperwoodSanctuary_EventScript_StonesQuiet
	msgbox Virelia_WhisperwoodSanctuary_Text_Rain, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Virelia_WhisperwoodSanctuary_EventScript_Release
	setvar VAR_0x8004, VIRELIA_SANCTUARY_RAIN
	goto Virelia_WhisperwoodSanctuary_EventScript_PressStone

Virelia_WhisperwoodSanctuary_EventScript_RootStone::
	lockall
	goto_if_set FLAG_VIRELIA_SANCTUARY_SHORTCUT_OPEN, Virelia_WhisperwoodSanctuary_EventScript_StonesQuiet
	msgbox Virelia_WhisperwoodSanctuary_Text_Root, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Virelia_WhisperwoodSanctuary_EventScript_Release
	setvar VAR_0x8004, VIRELIA_SANCTUARY_ROOT
Virelia_WhisperwoodSanctuary_EventScript_PressStone:
	call VireliaSanctuary_EventScript_TouchStone
	goto_if_eq VAR_RESULT, VIRELIA_SANCTUARY_WRONG, Virelia_WhisperwoodSanctuary_EventScript_Wrong
	goto_if_eq VAR_RESULT, VIRELIA_SANCTUARY_SOLVED, Virelia_WhisperwoodSanctuary_EventScript_Solved
	playse SE_PIN
	msgbox Virelia_WhisperwoodSanctuary_Text_StoneHums, MSGBOX_DEFAULT
	releaseall
	end

Virelia_WhisperwoodSanctuary_EventScript_Wrong:
	playse SE_FAILURE
	msgbox Virelia_WhisperwoodSanctuary_Text_Wrong, MSGBOX_DEFAULT
	releaseall
	end

Virelia_WhisperwoodSanctuary_EventScript_Solved:
	call Virelia_WhisperwoodSanctuary_OpenShortcut
	special DrawWholeMapView
	playse SE_BANG
	msgbox Virelia_WhisperwoodSanctuary_Text_Solved, MSGBOX_DEFAULT
	releaseall
	end

Virelia_WhisperwoodSanctuary_EventScript_StonesQuiet:
	msgbox Virelia_WhisperwoodSanctuary_Text_StonesQuiet, MSGBOX_DEFAULT
Virelia_WhisperwoodSanctuary_EventScript_Release:
	releaseall
	end

Virelia_WhisperwoodSanctuary_EventScript_AncientTree::
	lockall
	goto_if_unset FLAG_VIRELIA_SANCTUARY_SHORTCUT_OPEN, Virelia_WhisperwoodSanctuary_EventScript_Clue
	goto_if_set FLAG_VIRELIA_SANCTUARY_BELL_RECEIVED, Virelia_WhisperwoodSanctuary_EventScript_Folklore
	call VireliaSanctuary_EventScript_TryTakeBell
	goto_if_eq VAR_RESULT, FALSE, Virelia_WhisperwoodSanctuary_EventScript_BagFull
	playfanfare MUS_OBTAIN_ITEM
	waitfanfare
	msgbox Virelia_WhisperwoodSanctuary_Text_Bell, MSGBOX_DEFAULT
Virelia_WhisperwoodSanctuary_EventScript_Folklore:
	msgbox Virelia_WhisperwoodSanctuary_Text_Folklore, MSGBOX_DEFAULT
	releaseall
	end

Virelia_WhisperwoodSanctuary_EventScript_Clue:
	msgbox Virelia_WhisperwoodSanctuary_Text_Clue, MSGBOX_DEFAULT
	releaseall
	end

Virelia_WhisperwoodSanctuary_EventScript_BagFull:
	msgbox Virelia_WhisperwoodSanctuary_Text_BagFull, MSGBOX_DEFAULT
	releaseall
	end

Virelia_WhisperwoodSanctuary_EventScript_Pawprints::
	msgbox Virelia_WhisperwoodSanctuary_Text_Pawprints, MSGBOX_SIGN
	end

Virelia_WhisperwoodSanctuary_EventScript_ShortcutRoots::
	msgbox Virelia_WhisperwoodSanctuary_Text_ShortcutRoots, MSGBOX_SIGN
	end

Virelia_WhisperwoodSanctuary_Text_Seed:
	.string "Mech skrývá vyryté semínko.\n"
	.string "Zatlačit na kámen?$"

Virelia_WhisperwoodSanctuary_Text_Rain:
	.string "Na kameni jsou kapky deště.\n"
	.string "Zatlačit na kámen?$"

Virelia_WhisperwoodSanctuary_Text_Root:
	.string "Pod mechem je znak kořene.\n"
	.string "Zatlačit na kámen?$"

Virelia_WhisperwoodSanctuary_Text_StoneHums:
	.string "Kámen tiše zavibroval.\n"
	.string "Od stromu zazněl slabý tón.$"

Virelia_WhisperwoodSanctuary_Text_Wrong:
	.string "Tón ztichl. Kameny se vrátily\n"
	.string "na místo. Zkus pořadí znovu.$"

Virelia_WhisperwoodSanctuary_Text_Solved:
	.string "Tři tóny se spojily v jeden.\n"
	.string "Kořeny na severovýchodě ustoupily!\p"
	.string "Zkratka vede zpět na hlavní cestu.\n"
	.string "U paty stromu se něco zalesklo.$"

Virelia_WhisperwoodSanctuary_Text_StonesQuiet:
	.string "Kámen je teplý a klidný.\n"
	.string "Zkratka zůstává otevřená.$"

Virelia_WhisperwoodSanctuary_Text_Clue:
	.string "Prastarý strom objímá tři kameny.\n"
	.string "Pod mechem čteš vyrytá slova:\p"
	.string "Semínko čeká na déšť.\n"
	.string "Déšť probudí kořen.\p"
	.string "Kořen otevře cestu tomu,\n"
	.string "kdo naslouchá lesu.$"

Virelia_WhisperwoodSanctuary_Text_Bell:
	.string "Mezi kořeny leží Soothe Bell!\n"
	.string "Opatrně jej ukládáš do batohu.\p"
	.string "Pod zvonkem je starý nápis.$"

Virelia_WhisperwoodSanctuary_Text_Folklore:
	.string "Ve Virelii se vypráví o strážci,\n"
	.string "jenž spí pod kořeny lesa.\p"
	.string "Jeho kroky otřásají zemí,\n"
	.string "ale zvonek utiší i jeho srdce.\p"
	.string "Kdo ochrání nejmenší hnízdo,\n"
	.string "smí projít v jeho stínu.$"

Virelia_WhisperwoodSanctuary_Text_BagFull:
	.string "Na Soothe Bell není v batohu místo.\n"
	.string "Zvonek zůstane mezi kořeny.$"

Virelia_WhisperwoodSanctuary_Text_Pawprints:
	.string "Obrovské otisky tlap! Každý je\n"
	.string "mnohem větší než Zigzagoonův.\p"
	.string "Stopy mizí pod kořeny stromu.\n"
	.string "Žádná cesta dál není vidět…$"

Virelia_WhisperwoodSanctuary_Text_ShortcutRoots:
	.string "Cestu svírají propletené kořeny.\n"
	.string "Mezi nimi se chvěje mech.$"
