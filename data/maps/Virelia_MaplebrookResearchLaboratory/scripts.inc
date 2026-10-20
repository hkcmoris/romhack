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
	.string "Vítej v laboratoři ve Virelii!\n"
	.string "Zkoumáme místní Pokémony.\p"
	.string "Náš výzkum teprve začíná.\n"
	.string "Nech svůj tým odpočinout.$"

Virelia_MaplebrookResearchLaboratory_Text_Healed:
	.string "Tví Pokémoni jsou plní sil.\n"
	.string "Užij si průzkum stezky!$"

Virelia_MaplebrookResearchLaboratory_Text_Assistant:
	.string "Tato tři místa jsou vyhrazena\n"
	.string "pro první partnery z Virelie.\p"
	.string "Expozice zatím není připravená\n"
	.string "na výběr prvního Pokémona.$"

Virelia_MaplebrookResearchLaboratory_Text_GrassStarter:
	.string "POOCHYENA Z VIRELIE\n"
	.string "TRÁVA / DUCH\p"
	.string "Místo pro budoucího partnera.\n"
	.string "Pouze výzkumná expozice.$"

Virelia_MaplebrookResearchLaboratory_Text_WaterStarter:
	.string "SPINARAK Z VIRELIE\n"
	.string "VODA / HMYZ\p"
	.string "Místo pro budoucího partnera.\n"
	.string "Pouze výzkumná expozice.$"

Virelia_MaplebrookResearchLaboratory_Text_FireStarter:
	.string "EKANS Z VIRELIE\n"
	.string "OHEŇ / JED\p"
	.string "Místo pro budoucího partnera.\n"
	.string "Pouze výzkumná expozice.$"
