Virelia_MaplebrookTown_MapScripts::
	.byte 0

Virelia_MaplebrookTown_EventScript_Gardener::
	msgbox Virelia_MaplebrookTown_Text_Gardener, MSGBOX_NPC
	end

Virelia_MaplebrookTown_EventScript_Zigzagoon::
	lock
	faceplayer
	cry SPECIES_ZIGZAGOON, CRY_MODE_NORMAL
	msgbox Virelia_MaplebrookTown_Text_Zigzagoon, MSGBOX_DEFAULT
	waitmoncry
	release
	end

Virelia_MaplebrookTown_EventScript_TrailWalker::
	msgbox Virelia_MaplebrookTown_Text_TrailWalker, MSGBOX_NPC
	end

Virelia_MaplebrookTown_EventScript_HomeSign::
	msgbox Virelia_MaplebrookTown_Text_HomeSign, MSGBOX_SIGN
	end

Virelia_MaplebrookTown_EventScript_LabSign::
	msgbox Virelia_MaplebrookTown_Text_LabSign, MSGBOX_SIGN
	end

Virelia_MaplebrookTown_EventScript_NorthSign::
	msgbox Virelia_MaplebrookTown_Text_NorthSign, MSGBOX_SIGN
	end

Virelia_MaplebrookTown_EventScript_TownSign::
	msgbox Virelia_MaplebrookTown_Text_TownSign, MSGBOX_SIGN
	end

Virelia_MaplebrookTown_EventScript_ClosedHome::
	msgbox Virelia_MaplebrookTown_Text_ClosedHome, MSGBOX_SIGN
	end

Virelia_MaplebrookTown_EventScript_ClosedCottage::
	msgbox Virelia_MaplebrookTown_Text_ClosedCottage, MSGBOX_SIGN
	end

Virelia_MaplebrookTown_Text_Gardener:
	.string "We leave room for POKéMON\n"
	.string "in every garden here.\p"
	.string "The professor can help your\n"
	.string "team rest before a walk.$"

Virelia_MaplebrookTown_Text_Zigzagoon:
	.string "ZIGZAGOON: Zig! Zig!$"

Virelia_MaplebrookTown_Text_TrailWalker:
	.string "WHISPERWOOD TRAIL winds north\n"
	.string "to SLATEFORD CITY.\p"
	.string "The little forest paths are\n"
	.string "worth exploring, too!$"

Virelia_MaplebrookTown_Text_HomeSign:
	.string "{PLAYER}'s future home\n"
	.string "MAPLEBROOK TOWN$"

Virelia_MaplebrookTown_Text_LabSign:
	.string "MAPLEBROOK RESEARCH LAB\n"
	.string "Visitors and POKéMON welcome!$"

Virelia_MaplebrookTown_Text_NorthSign:
	.string "ROUTE 01 - WHISPERWOOD TRAIL\n"
	.string "North to SLATEFORD CITY$"

Virelia_MaplebrookTown_Text_TownSign:
	.string "MAPLEBROOK TOWN\n"
	.string "A small home for new beginnings.$"

Virelia_MaplebrookTown_Text_ClosedHome:
	.string "The house is being prepared.\n"
	.string "Its rooms will open another day.$"

Virelia_MaplebrookTown_Text_ClosedCottage:
	.string "The neighbours are out tending\n"
	.string "their gardens.$"
