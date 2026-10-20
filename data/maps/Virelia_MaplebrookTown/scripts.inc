Virelia_MaplebrookTown_MapScripts::
	.byte 0

Virelia_MaplebrookTown_EventScript_Gardener::
	msgbox Virelia_MaplebrookTown_Text_Gardener, MSGBOX_NPC
	end

Virelia_MaplebrookTown_EventScript_Zigzagoon::
	lock
	faceplayer
	playmoncry SPECIES_ZIGZAGOON, CRY_MODE_NORMAL
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
	.string "V každé zahradě tu necháváme\n"
	.string "místo i pro Pokémony.\p"
	.string "Profesor pomůže tvému týmu\n"
	.string "odpočinout si před výpravou.$"

Virelia_MaplebrookTown_Text_Zigzagoon:
	.string "ZIGZAGOON: Zig! Zig!$"

Virelia_MaplebrookTown_Text_TrailWalker:
	.string "Šeptající stezka vede na sever\n"
	.string "do města Slateford.\p"
	.string "Stojí za to prozkoumat i malé\n"
	.string "lesní pěšiny!$"

Virelia_MaplebrookTown_Text_HomeSign:
	.string "{PLAYER}: budoucí domov\n"
	.string "MĚSTEČKO MAPLEBROOK$"

Virelia_MaplebrookTown_Text_LabSign:
	.string "VÝZKUMNÁ LABORATOŘ MAPLEBROOK\n"
	.string "Vítáme návštěvníky i Pokémony!$"

Virelia_MaplebrookTown_Text_NorthSign:
	.string "CESTA 01 - ŠEPTAJÍCÍ STEZKA\n"
	.string "Na sever: MĚSTO SLATEFORD$"

Virelia_MaplebrookTown_Text_TownSign:
	.string "MĚSTEČKO MAPLEBROOK\n"
	.string "Malý domov pro nové začátky.$"

Virelia_MaplebrookTown_Text_ClosedHome:
	.string "Dům ještě není připravený.\n"
	.string "Dovnitř se podíváš později.$"

Virelia_MaplebrookTown_Text_ClosedCottage:
	.string "Sousedé se právě starají\n"
	.string "o své zahrady.$"
