Virelia_MaplebrookResearchLaboratory_MapScripts::
	.byte 0

Virelia_MaplebrookResearchLaboratory_EventScript_Professor::
	lock
	faceplayer
	msgbox Virelia_MaplebrookResearchLaboratory_Text_Professor, MSGBOX_DEFAULT
	special HealPlayerParty
	callnative UpdateFollowingPokemon
	playfanfare MUS_HEAL
	waitfanfare
	msgbox Virelia_MaplebrookResearchLaboratory_Text_Healed, MSGBOX_DEFAULT
	release
	end

Virelia_MaplebrookResearchLaboratory_EventScript_Assistant::
	msgbox Virelia_MaplebrookResearchLaboratory_Text_Assistant, MSGBOX_NPC
	end

Virelia_MaplebrookResearchLaboratory_EventScript_GrassStarter::
	msgbox Virelia_MaplebrookResearchLaboratory_Text_GrassStarter, MSGBOX_NPC
	end

Virelia_MaplebrookResearchLaboratory_EventScript_WaterStarter::
	msgbox Virelia_MaplebrookResearchLaboratory_Text_WaterStarter, MSGBOX_NPC
	end

Virelia_MaplebrookResearchLaboratory_EventScript_FireStarter::
	msgbox Virelia_MaplebrookResearchLaboratory_Text_FireStarter, MSGBOX_NPC
	end

Virelia_MaplebrookResearchLaboratory_Text_Professor:
	.string "Welcome to our VIRELIA lab!\n"
	.string "We study local POKéMON.\p"
	.string "Our research is just beginning.\n"
	.string "Let your team rest a moment.$"

Virelia_MaplebrookResearchLaboratory_Text_Healed:
	.string "Your POKéMON are refreshed.\n"
	.string "Enjoy exploring the trail!$"

Virelia_MaplebrookResearchLaboratory_Text_Assistant:
	.string "These three places are reserved\n"
	.string "for VIRELIA's first partners.\p"
	.string "The displays are not ready for\n"
	.string "a starter selection yet.$"

Virelia_MaplebrookResearchLaboratory_Text_GrassStarter:
	.string "VIRELIAN POOCHYENA\n"
	.string "GRASS / GHOST\p"
	.string "A place for a future partner.\n"
	.string "Research display only.$"

Virelia_MaplebrookResearchLaboratory_Text_WaterStarter:
	.string "VIRELIAN SPINARAK\n"
	.string "WATER / BUG\p"
	.string "A place for a future partner.\n"
	.string "Research display only.$"

Virelia_MaplebrookResearchLaboratory_Text_FireStarter:
	.string "VIRELIAN EKANS\n"
	.string "FIRE / POISON\p"
	.string "A place for a future partner.\n"
	.string "Research display only.$"
