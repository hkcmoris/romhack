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
	.string "Listen carefully to the grass.\n"
	.string "It is full of little footsteps!\p"
	.string "The eastern clearing is a nice\n"
	.string "place to take a short detour.$"

Virelia_Route01_Text_SouthSign:
	.string "ROUTE 01 - WHISPERWOOD TRAIL\n"
	.string "South: MAPLEBROOK TOWN\p"
	.string "North: SLATEFORD CITY\n"
	.string "Watch for wild POKéMON!$"

Virelia_Route01_Text_NorthSign:
	.string "SLATEFORD CITY lies ahead.\n"
	.string "MAPLEBROOK TOWN lies south.$"
