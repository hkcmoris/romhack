Virelia_Seabreak_KeepersHouse_MapScripts::
	.byte 0

Virelia_Seabreak_KeepersHouse_EventScript_Journal::
	lockall
	goto_if_lt VAR_VIRELIA_BOTTLE_STAGE, 3, Virelia_Seabreak_KeepersHouse_JournalHint
	setvar VAR_0x8004, 4
	call VireliaRainfall_EventScript_BottleClue
	msgbox Virelia_Seabreak_KeepersHouse_Text_Journal, MSGBOX_DEFAULT
	releaseall
	end
Virelia_Seabreak_KeepersHouse_JournalHint:
	msgbox Virelia_Seabreak_KeepersHouse_Text_JournalHint, MSGBOX_DEFAULT
	releaseall
	end

Virelia_Seabreak_KeepersHouse_Text_Journal:
	.string "DENÍK STRÁŽCE - datum z pobřeží\p"
	.string "Čočka praskla. Na moři zůstala\n"
	.string "loď se třemi rybáři.\p"
	.string "Vzal jsem člun a odvedl je podle\n"
	.string "bílých značek ke klidnému břehu.\p"
	.string "Oto, tvůj bratr je v bezpečí.\n"
	.string "Mrzí mě, že jsi čekal u světla.\p"
	.string "Dopis schovaný u deníku zůstal\n"
	.string "neodeslaný. Rybář Ota ho musí vidět.$"

Virelia_Seabreak_KeepersHouse_Text_JournalHint:
	.string "Starý deník nemá čitelný rok.\n"
	.string "Datum by mohlo být někde u pobřeží.$"

Virelia_Seabreak_KeepersHouse_EventScript_Keeper::
	msgbox Virelia_Seabreak_KeepersHouse_Text_Keeper, MSGBOX_NPC
	end

Virelia_Seabreak_KeepersHouse_Text_Keeper:
	.string "PEČOVATELKA: O tenhle dům pečuje\n"
	.string "celé město. Strážce byl můj strýc.\p"
	.string "Za té bouře nezůstal u lampy.\n"
	.string "Vyrazil zachraňovat rybáře.\p"
	.string "Jeho deník leží na stole.\n"
	.string "V Seabreak si minulost necháváme.$"
