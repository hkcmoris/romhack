Virelia_Slateford_QuarryEntrance_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Virelia_Slateford_QuarryEntrance_OnTransition
	map_script MAP_SCRIPT_ON_LOAD, Virelia_Slateford_QuarryEntrance_OnLoad
	.byte 0

Virelia_Slateford_QuarryEntrance_OnLoad:
	setmetatile 4, 3, 0x203, 1
	goto_if_unset FLAG_VIRELIA_QUARRY_POWER_RESTORED, Virelia_Slateford_QuarryEntrance_LoadDone
	setmetatile 4, 3, 0x227, 0
Virelia_Slateford_QuarryEntrance_LoadDone:
	end

Virelia_Slateford_QuarryEntrance_OnTransition:
	callnative VireliaFirstGym_BeginChapter
	end

@ Public return-only entry points keep native event tests on the real helpers.
VireliaFirstGym_EventScript_BeginChapter::
	callnative VireliaFirstGym_BeginChapter
	return

VireliaFirstGym_EventScript_QuarrySwitch::
	callnative VireliaFirstGym_QuarrySwitch
	return

VireliaFirstGym_EventScript_ReadEvidence::
	callnative VireliaFirstGym_ReadEvidence
	return

VireliaFirstGym_EventScript_CompleteQuarry::
	callnative VireliaFirstGym_CompleteQuarry
	return

VireliaFirstGym_EventScript_GymSwitch::
	callnative VireliaFirstGym_GymSwitch
	return

VireliaFirstGym_EventScript_AwardBadge::
	callnative VireliaFirstGym_AwardBadge
	return

VireliaFirstGym_EventScript_AwardTM::
	callnative VireliaFirstGym_AwardTM
	return

VireliaFirstGym_EventScript_TakeQuarryItem::
	callnative VireliaFirstGym_TakeQuarryItem
	return

Virelia_Slateford_QuarryEntrance_EventScript_Bruno::
	trainerbattle_single TRAINER_VIRELIA_MINER_BRUNO, Virelia_Slateford_QuarryEntrance_Text_BrunoIntro, Virelia_Slateford_QuarryEntrance_Text_BrunoDefeat
	msgbox Virelia_Slateford_QuarryEntrance_Text_BrunoAfter, MSGBOX_AUTOCLOSE
	end

Virelia_Slateford_QuarryEntrance_EventScript_Guide::
	goto_if_set FLAG_VIRELIA_QUARRY_POWER_RESTORED, Virelia_Slateford_QuarryGallery_EventScript_Miner
	msgbox Virelia_Slateford_QuarryEntrance_Text_Guide, MSGBOX_NPC
	end

Virelia_Slateford_QuarryEntrance_EventScript_GalleryGate::
	msgbox Virelia_Slateford_QuarryGallery_Text_Gate, MSGBOX_SIGN
	end

Virelia_Slateford_QuarryEntrance_EventScript_Equipment::
	msgbox Virelia_Slateford_QuarryEntrance_Text_Equipment, MSGBOX_SIGN
	end

Virelia_Slateford_QuarryEntrance_EventScript_Item::
	lock
	faceplayer
	call VireliaFirstGym_EventScript_TakeQuarryItem
	goto_if_eq VAR_RESULT, FALSE, Virelia_Slateford_QuarryEntrance_EventScript_BagFull
	removeobject 2
	playfanfare MUS_OBTAIN_ITEM
	waitfanfare
	msgbox Virelia_Slateford_QuarryEntrance_Text_Item, MSGBOX_DEFAULT
	release
	end

Virelia_Slateford_QuarryEntrance_EventScript_BagFull:
	msgbox Virelia_Slateford_QuarryEntrance_Text_BagFull, MSGBOX_DEFAULT
	release
	end

Virelia_Slateford_QuarryEntrance_Text_BrunoIntro:
	.string "BRUNO: Koleje jsou volné.\n"
	.string "Prověříme i tvůj tým?$"

Virelia_Slateford_QuarryEntrance_Text_BrunoDefeat:
	.string "Tak to byla pořádná směna.$"

Virelia_Slateford_QuarryEntrance_Text_BrunoAfter:
	.string "Dole je Lenka. Zná rozvodnu.\n"
	.string "Nejdřív ale otestuje tvůj tým.\p"
	.string "Voda a tráva lámou kámen.\n"
	.string "Souboj přesto vyhrává celý tým.$"

Virelia_Slateford_QuarryEntrance_Text_Guide:
	.string "Po zlodějích zbyly bedny\n"
	.string "a poškozená rozvodna.\p"
	.string "Hlubší část je za žebříkem\n"
	.string "na severovýchodě.\p"
	.string "Když to začne jiskřit, ustup.\n"
	.string "Hrdinství nehasí kabely.$"

Virelia_Slateford_QuarryEntrance_Text_Equipment:
	.string "Opuštěné nakládací zařízení.\n"
	.string "Čerstvé rýhy vedou hlouběji.\p"
	.string "Někdo odvezl jen tříděnou rudu.\n"
	.string "Obyčejný kámen tu nechal.$"

Virelia_Slateford_QuarryEntrance_Text_Item:
	.string "Nacházíš Super Potion!\n"
	.string "Ukládáš jej do batohu.$"

Virelia_Slateford_QuarryEntrance_Text_BagFull:
	.string "V batohu není místo.\n"
	.string "Super Potion tu zatím počká.$"
