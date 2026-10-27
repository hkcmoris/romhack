Virelia_RainfallFoothills_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, Virelia_RainfallFoothills_OnLoad
	.byte 0

Virelia_RainfallFoothills_OnLoad:
	callnative VireliaRainfall_BeginChapter
	end

Virelia_RainfallFoothills_EventScript_Shortcut::
	lockall
	goto_if_unset FLAG_VIRELIA_LOWER_SHORTCUT, Virelia_RainfallFoothills_ShortcutClosed
	msgbox Virelia_RainfallFoothills_Text_Shortcut, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, VireliaRainfall_EventScript_Release
	warp MAP_VIRELIA_RAINFALL_LOWER, WARP_ID_NONE, 17, 20
	waitstate
	releaseall
	end

Virelia_RainfallFoothills_ShortcutClosed:
	msgbox Virelia_RainfallFoothills_Text_ShortcutClosed, MSGBOX_DEFAULT
	releaseall
	end

Virelia_RainfallFoothills_Text_Shortcut:
	.string "Lávka vede zpět do mokřin.\n"
	.string "Přejít po ní?$"

Virelia_RainfallFoothills_Text_ShortcutClosed:
	.string "Lávka je vytažená na druhý břeh.\n"
	.string "Z mokřin půjde spustit.$"

Virelia_RainfallFoothills_EventScript_Sign::
	msgbox Virelia_RainfallFoothills_Text_Sign, MSGBOX_SIGN
	end

Virelia_RainfallFoothills_Text_Sign:
	.string "Rainfall Pass - rozcestí\p"
	.string "Horní stezka: mosty a trenéři.\n"
	.string "Dolní stezka: mokřiny a potoky.\p"
	.string "Obě vedou k pobřeží Seabreak.\n"
	.string "Žádná nevyžaduje Surf.$"

Virelia_RainfallFoothills_EventScript_Hiker::
	trainerbattle_single TRAINER_VIRELIA_RAINFALL_HIKER, Virelia_RainfallFoothills_Text_HikerIntro, Virelia_RainfallFoothills_Text_HikerDefeat
	msgbox Virelia_RainfallFoothills_Text_HikerAfter, MSGBOX_AUTOCLOSE
	end

Virelia_RainfallFoothills_Text_HikerIntro:
	.string "PAVEL: Kámen ještě drží sucho.\n"
	.string "Tak toho využijeme k souboji!$"

Virelia_RainfallFoothills_Text_HikerDefeat:
	.string "Teď už mám mokré i sebevědomí.$"

Virelia_RainfallFoothills_Text_HikerAfter:
	.string "Nahoře je to kratší, ale rušnější.\n"
	.string "Dole si cestu hledá voda i ty.$"
