Virelia_Route01_MapScripts::
	.byte 0

Virelia_Route01_EventScript_Naturalist::
	msgbox Virelia_Route01_Text_Naturalist, MSGBOX_NPC
	end

Virelia_Route01_EventScript_SouthSign::
	msgbox Virelia_Route01_Text_SouthSign, MSGBOX_SIGN
	end

Virelia_Route01_EventScript_NorthSign::
	msgbox Virelia_Route01_Text_NorthSign, MSGBOX_SIGN
	end

Virelia_Route01_Text_Naturalist:
	.string "Zaposlouchej se do trávy.\n"
	.string "Ozývá se z ní spousta krůčků!\p"
	.string "Na východě je pěkná mýtina.\n"
	.string "Stojí za malou zacházku.$"

Virelia_Route01_Text_SouthSign:
	.string "CESTA 01 - ŠEPTAJÍCÍ STEZKA\n"
	.string "Jih: MĚSTEČKO MAPLEBROOK\p"
	.string "Sever: MĚSTO SLATEFORD\n"
	.string "Pozor na divoké Pokémony!$"

Virelia_Route01_Text_NorthSign:
	.string "Před tebou leží město Slateford.\n"
	.string "Na jihu je městečko Maplebrook.$"
