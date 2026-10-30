Virelia_MaplebrookFishermansHouse_MapScripts::
	.byte 0

Virelia_MaplebrookFishermansHouse_EventScript_Tackle::
	msgbox Virelia_MaplebrookFishermansHouse_Text_Tackle, MSGBOX_SIGN
	end

Virelia_MaplebrookFishermansHouse_EventScript_Notebook::
	msgbox Virelia_MaplebrookFishermansHouse_Text_Notebook, MSGBOX_SIGN
	end

Virelia_MaplebrookFishermansHouse_Text_Tackle:
	.string "Na stole leží háčky a návnady.\n"
	.string "Rybář je nejspíš zase u řeky.$"

Virelia_MaplebrookFishermansHouse_Text_Notebook:
	.string "RYBÁŘŮV ZÁPISNÍK\n"
	.string "Dnešní úlovek: jeden mokrý list.\p"
	.string "Poznámka: Psyduck se počítá\n"
	.string "jako pomocník, ne jako úlovek.$"
