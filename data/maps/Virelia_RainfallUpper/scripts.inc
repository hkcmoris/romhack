Virelia_RainfallUpper_MapScripts::
	.byte 0

Virelia_RainfallUpper_EventScript_Ranger::
	trainerbattle_single TRAINER_VIRELIA_RAINFALL_RANGER, Virelia_RainfallUpper_Text_RangerIntro, Virelia_RainfallUpper_Text_RangerDefeat
	msgbox Virelia_RainfallUpper_Text_RangerAfter, MSGBOX_AUTOCLOSE
	end

Virelia_RainfallUpper_Text_RangerIntro:
	.string "IRENA: Na mostě se nestrkáme.\n"
	.string "Mimo most si můžeme zabojovat!$"

Virelia_RainfallUpper_Text_RangerDefeat:
	.string "Dobrý tým se pozná i v dešti.$"

Virelia_RainfallUpper_Text_RangerAfter:
	.string "Natu se učí Rain Dance.\n"
	.string "Déšť posílí vodní útoky.\p"
	.string "Ohnivé naopak oslabí.\n"
	.string "Pozor na to, kdo z něj získá víc.$"

Virelia_RainfallUpper_EventScript_WeatherSign::
	msgbox Virelia_RainfallUpper_Text_WeatherSign, MSGBOX_SIGN
	end

Virelia_RainfallUpper_Text_WeatherSign:
	.string "Na mostech neklouzej!\p"
	.string "Za deště rostou vodní útoky.\n"
	.string "Ohnivé útoky zeslábnou.\p"
	.string "Počasí v souboji může změnit\n"
	.string "také Rain Dance. Zkus to.$"

Virelia_RainfallUpper_EventScript_Item::
	lockall
	goto_if_set FLAG_VIRELIA_RAINFALL_ITEM_REPEL, Virelia_RainfallUpper_ItemTaken
	setvar VAR_0x8004, 1
	call VireliaRainfall_EventScript_TakeRouteItem
	goto_if_eq VAR_RESULT, FALSE, VireliaRainfall_EventScript_BagFull
	playfanfare MUS_OBTAIN_ITEM
	msgbox Virelia_RainfallUpper_Text_Item, MSGBOX_DEFAULT
	waitfanfare
	removeobject VAR_LAST_TALKED
	releaseall
	end
Virelia_RainfallUpper_ItemTaken:
	msgbox Virelia_RainfallUpper_Text_ItemTaken, MSGBOX_DEFAULT
	releaseall
	end
Virelia_RainfallUpper_Text_Item:
	.string "{PLAYER} nachází Repel!\n"
	.string "Hodí se na delší průzkum mokřin.$"
Virelia_RainfallUpper_Text_ItemTaken:
	.string "Tady už nic dalšího neleží.$"