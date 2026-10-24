Virelia_Slateford_QuarryDepths_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Virelia_Slateford_QuarryDepths_OnTransition
	map_script MAP_SCRIPT_ON_LOAD, Virelia_Slateford_QuarryDepths_OnLoad
	.byte 0

Virelia_Slateford_QuarryDepths_OnTransition:
	callnative VireliaFirstGym_ResetQuarryPuzzle
	end

Virelia_Slateford_QuarryDepths_OnLoad:
	setmetatile 3, 7, 0x203, 1
	setmetatile 20, 14, 0x25C, 1
	setmetatile 4, 9, 0x25E, 1
	goto_if_set FLAG_VIRELIA_QUARRY_POWER_RESTORED, Virelia_Slateford_QuarryDepths_OpenGate
	goto_if_eq VAR_VIRELIA_QUARRY_SWITCH, 1, Virelia_Slateford_QuarryDepths_LoadPump
	setmetatile 11, 6, 0x203, 1
	end

Virelia_Slateford_QuarryDepths_LoadPump:
	setmetatile 20, 14, 0x25D, 1
	setmetatile 11, 6, 0x203, 1
	end

Virelia_Slateford_QuarryDepths_OpenGate:
	setmetatile 20, 14, 0x25D, 1
	setmetatile 4, 9, 0x25F, 1
	setmetatile 11, 6, 0x201, 0
	setmetatile 3, 7, 0x214, 0
	end

Virelia_Slateford_QuarryDepths_EventScript_Lenka::
	trainerbattle_single TRAINER_VIRELIA_EXPLORER_LENKA, Virelia_Slateford_QuarryDepths_Text_LenkaIntro, Virelia_Slateford_QuarryDepths_Text_LenkaDefeat
	msgbox Virelia_Slateford_QuarryDepths_Text_LenkaAfter, MSGBOX_AUTOCLOSE
	end

Virelia_Slateford_QuarryDepths_EventScript_Pump::
	lockall
	goto_if_set FLAG_VIRELIA_QUARRY_POWER_RESTORED, Virelia_Slateford_QuarryDepths_EventScript_PowerAlready
	msgbox Virelia_Slateford_QuarryDepths_Text_Pump, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Virelia_Slateford_QuarryDepths_EventScript_Release
	setvar VAR_0x8004, 1
	call VireliaFirstGym_EventScript_QuarrySwitch
	goto_if_ne VAR_VIRELIA_QUARRY_SWITCH, 1, Virelia_Slateford_QuarryDepths_EventScript_WrongOrder
	setmetatile 20, 14, 0x25D, 1
	special DrawWholeMapView
	playse SE_SWITCH
	msgbox Virelia_Slateford_QuarryDepths_Text_PumpRunning, MSGBOX_DEFAULT
	releaseall
	end

Virelia_Slateford_QuarryDepths_EventScript_Feed::
	lockall
	goto_if_set FLAG_VIRELIA_QUARRY_POWER_RESTORED, Virelia_Slateford_QuarryDepths_EventScript_PowerAlready
	msgbox Virelia_Slateford_QuarryDepths_Text_Feed, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Virelia_Slateford_QuarryDepths_EventScript_Release
	setvar VAR_0x8004, 2
	call VireliaFirstGym_EventScript_QuarrySwitch
	goto_if_eq VAR_RESULT, FALSE, Virelia_Slateford_QuarryDepths_EventScript_WrongOrder
	setmetatile 20, 14, 0x25D, 1
	setmetatile 4, 9, 0x25F, 1
	setmetatile 11, 6, 0x201, 0
	setmetatile 3, 7, 0x214, 0
	special DrawWholeMapView
	playse SE_BANG
	msgbox Virelia_Slateford_QuarryDepths_Text_PowerRestored, MSGBOX_DEFAULT
	releaseall
	end

Virelia_Slateford_QuarryDepths_EventScript_WrongOrder:
	setmetatile 20, 14, 0x25C, 1
	setmetatile 4, 9, 0x25E, 1
	special DrawWholeMapView
	playse SE_CLICK
	msgbox Virelia_Slateford_QuarryDepths_Text_WrongOrder, MSGBOX_DEFAULT
	releaseall
	end

Virelia_Slateford_QuarryDepths_EventScript_PowerAlready:
	msgbox Virelia_Slateford_QuarryDepths_Text_PowerAlready, MSGBOX_DEFAULT
Virelia_Slateford_QuarryDepths_EventScript_Release:
	releaseall
	end

Virelia_Slateford_QuarryDepths_EventScript_Instructions::
	msgbox Virelia_Slateford_QuarryDepths_Text_Instructions, MSGBOX_SIGN
	end

Virelia_Slateford_QuarryDepths_EventScript_GalleryGate::
	msgbox Virelia_Slateford_QuarryGallery_Text_Gate, MSGBOX_SIGN
	end

Virelia_Slateford_QuarryDepths_EventScript_Crate::
	msgbox Virelia_Slateford_QuarryDepths_Text_Crate, MSGBOX_NPC
	end

Virelia_Slateford_QuarryDepths_EventScript_Evidence::
	lock
	faceplayer
	goto_if_set FLAG_VIRELIA_QUARRY_EVIDENCE, Virelia_Slateford_QuarryDepths_EventScript_EvidenceAlready
	call VireliaFirstGym_EventScript_ReadEvidence
	goto_if_eq VAR_RESULT, FALSE, Virelia_Slateford_QuarryDepths_EventScript_NeedInspection
	msgbox Virelia_Slateford_QuarryDepths_Text_Evidence, MSGBOX_DEFAULT
	release
	end

Virelia_Slateford_QuarryDepths_EventScript_NeedInspection:
	msgbox Virelia_Slateford_QuarryDepths_Text_NeedInspection, MSGBOX_DEFAULT
	release
	end

Virelia_Slateford_QuarryDepths_EventScript_EvidenceAlready:
	msgbox Virelia_Slateford_QuarryDepths_Text_EvidenceAlready, MSGBOX_DEFAULT
	release
	end

Virelia_Slateford_QuarryDepths_Text_LenkaIntro:
	.string "LENKA: Pod zemí potřebujeme\n"
	.string "spolehlivé parťáky. Ukaž své!$"

Virelia_Slateford_QuarryDepths_Text_LenkaDefeat:
	.string "Dobře. Tebe bych vzala do směny.$"

Virelia_Slateford_QuarryDepths_Text_LenkaAfter:
	.string "Nejdřív zapni odvodnění vpravo.\n"
	.string "Pak přívod proudu vlevo.\p"
	.string "Opačně spadne jistič.\n"
	.string "Údržba tomu říká poučení.\p"
	.string "K bedně na severu jdi až potom.\n"
	.string "Ať její štítek neskončí ve vodě.$"

Virelia_Slateford_QuarryDepths_Text_Pump:
	.string "ODVODNĚNÍ - pravý panel\n"
	.string "Zapnout čerpadlo?$"

Virelia_Slateford_QuarryDepths_Text_PumpRunning:
	.string "Čerpadlo odvádí vodu.\n"
	.string "Teď zapni levý přívod proudu.$"

Virelia_Slateford_QuarryDepths_Text_Feed:
	.string "PŘÍVOD PROUDU - levý panel\n"
	.string "Zapnout napájení tratě?$"

Virelia_Slateford_QuarryDepths_Text_WrongOrder:
	.string "Cvak! Jistič vypnul oba okruhy.\n"
	.string "Nejdřív odvodnění, pak proud.$"

Virelia_Slateford_QuarryDepths_Text_PowerRestored:
	.string "Rozvodna opět pracuje!\n"
	.string "Pohon odsunul kamenný náklad.\p"
	.string "Cesta k severnímu skladu\n"
	.string "je volná. Prohlédni jeho bednu.\p"
	.string "Odsunuly se i vozíky u žebříků.\n"
	.string "Horník u vchodu tě shání.$"

Virelia_Slateford_QuarryDepths_Text_PowerAlready:
	.string "Rozvodna běží. Náklad je pryč.\n"
	.string "Další mačkání není údržba.$"

Virelia_Slateford_QuarryDepths_Text_Instructions:
	.string "NOUZOVÉ SPUŠTĚNÍ\n"
	.string "1. Odvodnění na východě.\p"
	.string "2. Přívod proudu na západě.\n"
	.string "Záměna pořadí vypne jistič.\p"
	.string "Při odchodu se dílčí nastavení\n"
	.string "vynuluje. Oprava zůstává.$"

Virelia_Slateford_QuarryDepths_Text_Crate:
	.string "Prázdné přepravní bedny.\n"
	.string "Kabel je vytržený z rozvodny.\p"
	.string "Zloději si s opatrností\n"
	.string "očividně hlavu nelámali.$"

Virelia_Slateford_QuarryDepths_Text_NeedInspection:
	.string "Nejdřív požádej Bruna i Lenku,\n"
	.string "ať prověří bezpečnost provozu.\p"
	.string "Oba si nejprve ověří tvůj tým.$"

Virelia_Slateford_QuarryDepths_Text_Evidence:
	.string "Na bedně zůstal přepravní štítek.\n"
	.string "Stejné číslo jako u poskoka!\p"
	.string "Obsah: minerály pro elektrické\n"
	.string "vodiče s vysokou kapacitou.\p"
	.string "Obyčejnou rudu nechali ležet.\n"
	.string "Hledali právě tenhle materiál.\p"
	.string "Na co potřebují tolik proudu?\n"
	.string "Štítek to bohužel neprozradí.\p"
	.string "Vrať se za správcem před lomem.\n"
	.string "Tohle musí vidět.$"

Virelia_Slateford_QuarryDepths_Text_EvidenceAlready:
	.string "Štítek spojuje krádeže v lomu\n"
	.string "s poskokem Rakeťáků.\p"
	.string "Správce čeká před vchodem.$"
