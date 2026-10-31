Virelia_Seabreak_LanternRoom_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Virelia_Seabreak_LanternRoom_OnTransition
	map_script MAP_SCRIPT_ON_LOAD, Virelia_Seabreak_LanternRoom_OnLoad
	.byte 0

Virelia_Seabreak_LanternRoom_OnTransition:
	callnative VireliaRainfall_ResetSignal
	end

Virelia_Seabreak_LanternRoom_OnLoad:
	goto_if_set FLAG_VIRELIA_LIGHTHOUSE_REPAIRED, Virelia_Seabreak_LanternRoom_BeaconOn
	setmetatile 6, 2, 0x20C, 1
	end
Virelia_Seabreak_LanternRoom_BeaconOn:
	setmetatile 6, 2, 0x20D, 1
	end

Virelia_Seabreak_LanternRoom_EventScript_Feed::
	lockall
	goto_if_set FLAG_VIRELIA_LIGHTHOUSE_REPAIRED, Virelia_Seabreak_LanternRoom_Already
	goto_if_unset FLAG_VIRELIA_LIGHTHOUSE_DAMAGE, Virelia_Seabreak_LanternRoom_NeedDamage
	msgbox Virelia_Seabreak_LanternRoom_Text_Feed, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, VireliaRainfall_EventScript_Release
	setvar VAR_0x8004, 1
	call VireliaRainfall_EventScript_SignalSwitch
	goto_if_ne VAR_VIRELIA_SIGNAL_INPUT, 1, Virelia_Seabreak_LanternRoom_Wrong
	playse SE_SELECT
	msgbox Virelia_Seabreak_LanternRoom_Text_Step1, MSGBOX_DEFAULT
	releaseall
	end

Virelia_Seabreak_LanternRoom_Text_Feed:
	.string "Zapnout záložní přívod?$"

Virelia_Seabreak_LanternRoom_EventScript_Lens::
	lockall
	goto_if_set FLAG_VIRELIA_LIGHTHOUSE_REPAIRED, Virelia_Seabreak_LanternRoom_Already
	goto_if_unset FLAG_VIRELIA_LIGHTHOUSE_DAMAGE, Virelia_Seabreak_LanternRoom_NeedDamage
	msgbox Virelia_Seabreak_LanternRoom_Text_Lens, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, VireliaRainfall_EventScript_Release
	setvar VAR_0x8004, 2
	call VireliaRainfall_EventScript_SignalSwitch
	goto_if_ne VAR_VIRELIA_SIGNAL_INPUT, 2, Virelia_Seabreak_LanternRoom_Wrong
	playse SE_SELECT
	msgbox Virelia_Seabreak_LanternRoom_Text_Step2, MSGBOX_DEFAULT
	releaseall
	end

Virelia_Seabreak_LanternRoom_Text_Lens:
	.string "Srovnat optickou čočku?$"

Virelia_Seabreak_LanternRoom_EventScript_Beacon::
	lockall
	goto_if_set FLAG_VIRELIA_LIGHTHOUSE_REPAIRED, Virelia_Seabreak_LanternRoom_Already
	goto_if_unset FLAG_VIRELIA_LIGHTHOUSE_DAMAGE, Virelia_Seabreak_LanternRoom_NeedDamage
	msgbox Virelia_Seabreak_LanternRoom_Text_Beacon, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, VireliaRainfall_EventScript_Release
	setvar VAR_0x8004, 3
	call VireliaRainfall_EventScript_SignalSwitch
	goto_if_eq VAR_RESULT, FALSE, Virelia_Seabreak_LanternRoom_Wrong
	setmetatile 6, 2, 0x20D, 1
	special DrawWholeMapView
	playse SE_BANG
	msgbox Virelia_Seabreak_LanternRoom_Text_Solved, MSGBOX_DEFAULT
	releaseall
	end

Virelia_Seabreak_LanternRoom_Text_Beacon:
	.string "Spustit signální světlo?$"

Virelia_Seabreak_LanternRoom_Wrong:
	msgbox Virelia_Seabreak_LanternRoom_Text_Wrong, MSGBOX_DEFAULT
	releaseall
	end
Virelia_Seabreak_LanternRoom_Already:
	msgbox Virelia_Seabreak_LanternRoom_Text_Already, MSGBOX_DEFAULT
	releaseall
	end
Virelia_Seabreak_LanternRoom_NeedDamage:
	msgbox Virelia_Seabreak_LanternRoom_Text_NeedDamage, MSGBOX_DEFAULT
	releaseall
	end

Virelia_Seabreak_LanternRoom_EventScript_Manifest::
	lockall
	call VireliaRainfall_EventScript_ReadManifest
	goto_if_eq VAR_RESULT, FALSE, Virelia_Seabreak_LanternRoom_Dark
	msgbox Virelia_Seabreak_LanternRoom_Text_Manifest, MSGBOX_DEFAULT
	releaseall
	end
Virelia_Seabreak_LanternRoom_Dark:
	msgbox Virelia_Seabreak_LanternRoom_Text_Dark, MSGBOX_DEFAULT
	releaseall
	end

Virelia_Seabreak_LanternRoom_Text_Step1:
	.string "Záložní přívod běží.\n"
	.string "Teď srovnej čočku uprostřed.$"

Virelia_Seabreak_LanternRoom_Text_Step2:
	.string "Čočka míří na plavební dráhu.\n"
	.string "Teď spusť světlo vpravo.$"

Virelia_Seabreak_LanternRoom_Text_Wrong:
	.string "Pojistka vypnula celý postup.\n"
	.string "Začni znovu přívodem vlevo.\p"
	.string "Potom čočka a nakonec světlo.$"

Virelia_Seabreak_LanternRoom_Text_NeedDamage:
	.string "Přívod je odpojený.\n"
	.string "Nejdřív prověř skříň dole s Bělou.$"

Virelia_Seabreak_LanternRoom_Text_Solved:
	.string "Čočka se rozsvítí jasným světlem.\n"
	.string "Maják zase ukazuje lodím cestu!\p"
	.string "Ještě prohlédni lodní knihu\n"
	.string "na stole a vrať se ke správci.$"

Virelia_Seabreak_LanternRoom_Text_Already:
	.string "Světlo běží pravidelně.\n"
	.string "Opravený maják zůstane v provozu.$"

Virelia_Seabreak_LanternRoom_Text_Dark:
	.string "V šeru nejde záznam přečíst.\n"
	.string "Nejdřív obnov světlo majáku.$"

Virelia_Seabreak_LanternRoom_Text_Manifest:
	.string "LODNÍ KNIHA - zpožděné zásilky\p"
	.string "Původ: Slateford Quarry.\n"
	.string "Obsah: vodivé minerály.\p"
	.string "Příjemce: nečitelný otisk razítka.\n"
	.string "Podpis převzetí chybí.\p"
	.string "BĚLA: Číslo souhlasí s hlášením.\n"
	.string "Příjemce tu není čitelný.\p"
	.string "Zatím známe jen původ zásilky.\n"
	.string "Vraťme se ke správci přístavu.$"
