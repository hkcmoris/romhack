Virelia_MaplebrookEmasHouse_MapScripts::
	.byte 0

Virelia_MaplebrookEmasHouse_EventScript_Mother::
	msgbox Virelia_MaplebrookEmasHouse_Text_Mother, MSGBOX_NPC
	end

Virelia_MaplebrookEmasHouse_EventScript_TrainingNotes::
	msgbox Virelia_MaplebrookEmasHouse_Text_TrainingNotes, MSGBOX_SIGN
	end

Virelia_MaplebrookEmasHouse_EventScript_Treats::
	msgbox Virelia_MaplebrookEmasHouse_Text_Treats, MSGBOX_SIGN
	end

Virelia_MaplebrookEmasHouse_Text_Mother:
	.string "MÁMA: Ema zase trénuje venku.\n"
	.string "Učí Poochyenu povel Sedni!\p"
	.string "Doma už ho zvládá výborně.\n"
	.string "Hlavně když chystám večeři.$"

Virelia_MaplebrookEmasHouse_Text_TrainingNotes:
	.string "EMIN TRÉNINKOVÝ ZÁPISNÍK\n"
	.string "Sedni. Zůstaň. Pusť zápisník.\p"
	.string "Poslední povel je podtržený\n"
	.string "a roh stránky je okousaný.$"

Virelia_MaplebrookEmasHouse_Text_Treats:
	.string "Na stole je dóza s pamlsky.\n"
	.string "Na víčku stojí: Až po tréninku.\p"
	.string "Pod tím je otisk tlapky.$"
