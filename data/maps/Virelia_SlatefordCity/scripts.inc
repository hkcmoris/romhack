Virelia_SlatefordCity_MapScripts::
	.byte 0

Virelia_SlatefordCity_EventScript_Miner::
	msgbox Virelia_SlatefordCity_Text_Miner, MSGBOX_NPC
	end

Virelia_SlatefordCity_EventScript_Resident::
	msgbox Virelia_SlatefordCity_Text_Resident, MSGBOX_NPC
	end

Virelia_SlatefordCity_EventScript_Stone::
	msgbox Virelia_SlatefordCity_Text_Stone, MSGBOX_NPC
	end

Virelia_SlatefordCity_EventScript_GymSign::
	msgbox Virelia_SlatefordCity_Text_GymSign, MSGBOX_SIGN
	end

Virelia_SlatefordCity_EventScript_CitySign::
	msgbox Virelia_SlatefordCity_Text_CitySign, MSGBOX_SIGN
	end

Virelia_SlatefordCity_EventScript_QuarrySign::
	msgbox Virelia_SlatefordCity_Text_QuarrySign, MSGBOX_SIGN
	end

Virelia_SlatefordCity_EventScript_OfficeSign::
	msgbox Virelia_SlatefordCity_Text_OfficeSign, MSGBOX_SIGN
	end

Virelia_SlatefordCity_EventScript_ClosedGym::
	msgbox Virelia_SlatefordCity_Text_ClosedGym, MSGBOX_SIGN
	end

Virelia_SlatefordCity_EventScript_ClosedHouse::
	msgbox Virelia_SlatefordCity_Text_ClosedHouse, MSGBOX_SIGN
	end

Virelia_SlatefordCity_EventScript_ClosedOffice::
	msgbox Virelia_SlatefordCity_Text_ClosedOffice, MSGBOX_SIGN
	end

Virelia_SlatefordCity_Text_Miner:
	.string "Stone built this town, but it\n"
	.string "takes people to make a home.\p"
	.string "Our survey crew is getting\n"
	.string "the quarry ready.$"

Virelia_SlatefordCity_Text_Resident:
	.string "MAPLEBROOK is just beyond\n"
	.string "WHISPERWOOD TRAIL to the south.\p"
	.string "Come back when our new shops\n"
	.string "are ready to open!$"

Virelia_SlatefordCity_Text_Stone:
	.string "A marked quarry sample.\n"
	.string "Please leave it for the crew.$"

Virelia_SlatefordCity_Text_GymSign:
	.string "SLATEFORD CITY POKéMON GYM\n"
	.string "Preparing for its first trainers.$"

Virelia_SlatefordCity_Text_CitySign:
	.string "SLATEFORD CITY\n"
	.string "Strong roots in stone.$"

Virelia_SlatefordCity_Text_QuarrySign:
	.string "SLATEFORD QUARRY\n"
	.string "Survey work in progress.$"

Virelia_SlatefordCity_Text_OfficeSign:
	.string "SLATEFORD MINING OFFICE\n"
	.string "Opening soon.$"

Virelia_SlatefordCity_Text_ClosedGym:
	.string "The GYM is not open yet.\n"
	.string "Its first challenge awaits.$"

Virelia_SlatefordCity_Text_ClosedHouse:
	.string "The door is locked.\n"
	.string "The residents are away.$"

Virelia_SlatefordCity_Text_ClosedOffice:
	.string "The mining office is being\n"
	.string "prepared for its new crew.$"
