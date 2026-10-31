Virelia_RainfallLower_MapScripts::
	.byte 0

Virelia_RainfallLower_EventScript_Shortcut::
	lockall
	goto_if_set FLAG_VIRELIA_LOWER_SHORTCUT, Virelia_RainfallLower_ShortcutOpen
	call VireliaRainfall_EventScript_OpenShortcut
	playse SE_BANG
	msgbox Virelia_RainfallLower_Text_ShortcutOpened, MSGBOX_DEFAULT
Virelia_RainfallLower_ShortcutOpen:
	msgbox Virelia_RainfallLower_Text_Shortcut, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, VireliaRainfall_EventScript_Release
	warp MAP_VIRELIA_RAINFALL_FOOTHILLS, WARP_ID_NONE, 25, 23
	waitstate
	releaseall
	end

Virelia_RainfallLower_EventScript_Item::
	lockall
	goto_if_set FLAG_VIRELIA_RAINFALL_ITEM_SUPER_POTION, Virelia_RainfallLower_ItemTaken
	setvar VAR_0x8004, 2
	call VireliaRainfall_EventScript_TakeRouteItem
	goto_if_eq VAR_RESULT, FALSE, VireliaRainfall_EventScript_BagFull
	playfanfare MUS_OBTAIN_ITEM
	msgbox Virelia_RainfallLower_Text_Item, MSGBOX_DEFAULT
	waitfanfare
	releaseall
	end
Virelia_RainfallLower_ItemTaken:
	msgbox Virelia_RainfallLower_Text_ItemTaken, MSGBOX_DEFAULT
	releaseall
	end

Virelia_RainfallLower_Text_ShortcutOpened:
	.string "Páka spouští zpáteční lávku.\n"
	.string "Zkratka zůstane otevřená.$"

Virelia_RainfallLower_Text_Shortcut:
	.string "Vrátit se lávkou do podhůří?$"

Virelia_RainfallLower_Text_Item:
	.string "V suché schránce je Super Potion.\n"
	.string "{PLAYER} si ho bere.$"

Virelia_RainfallLower_Text_ItemTaken:
	.string "Prázdná schránka.\n"
	.string "Voda se dovnitř naštěstí nedostala.$"

Virelia_RainfallLower_EventScript_StreamClue::
	msgbox Virelia_RainfallLower_Text_StreamClue, MSGBOX_SIGN
	end

Virelia_RainfallLower_Text_StreamClue:
	.string "Rysky ukazují nedávný vzestup vody.\n"
	.string "Pod útesem je proud klidnější.\p"
	.string "Cesta po břehu je bezpečná.\n"
	.string "Nemusíš vstupovat do vody.$"

Virelia_RainfallLower_EventScript_Fisherman::
	trainerbattle_single TRAINER_VIRELIA_RAINFALL_FISHERMAN, Virelia_RainfallLower_Text_FishermanIntro, Virelia_RainfallLower_Text_FishermanDefeat
	msgbox Virelia_RainfallLower_Text_FishermanAfter, MSGBOX_AUTOCLOSE
	end

Virelia_RainfallLower_Text_FishermanIntro:
	.string "DUŠAN: Ryby dnes neberou.\n"
	.string "Zkusím, jestli zabere souboj.$"

Virelia_RainfallLower_Text_FishermanDefeat:
	.string "Tak aspoň zkušenost mám!$"

Virelia_RainfallLower_Text_FishermanAfter:
	.string "Páka u lávky tě vrátí do podhůří.\n"
	.string "Stačí ji tady spustit jednou.$"
