Virelia_Slateford_QuarryGallery_MapScripts::
	.byte 0

@ Return-only wrappers also let native tests execute the compiled helpers.
VireliaFirstGym_EventScript_BeginMissingTools::
	callnative VireliaFirstGym_BeginMissingTools
	return

VireliaFirstGym_EventScript_CollectTool::
	callnative VireliaFirstGym_CollectTool
	return

VireliaFirstGym_EventScript_RecoverTools::
	callnative VireliaFirstGym_RecoverTools
	return

VireliaFirstGym_EventScript_FinishToolsBattle::
	specialvar VAR_0x8004, GetBattleOutcome
	call VireliaFirstGym_EventScript_RecoverTools
	return

VireliaFirstGym_EventScript_RewardTools::
	callnative VireliaFirstGym_RewardTools
	return

Virelia_Slateford_QuarryGallery_EventScript_Miner::
	lock
	faceplayer
	goto_if_set FLAG_VIRELIA_MISSING_TOOLS_REWARDED, Virelia_Slateford_QuarryGallery_EventScript_Thanks
	goto_if_set FLAG_VIRELIA_MISSING_TOOLS_RECOVERED, Virelia_Slateford_QuarryGallery_EventScript_ReturnTools
	goto_if_set FLAG_VIRELIA_MISSING_TOOLS_STARTED, Virelia_Slateford_QuarryGallery_EventScript_Reminder
	msgbox Virelia_Slateford_QuarryGallery_Text_Request, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Virelia_Slateford_QuarryGallery_EventScript_Release
	call VireliaFirstGym_EventScript_BeginMissingTools
Virelia_Slateford_QuarryGallery_EventScript_Reminder:
	msgbox Virelia_Slateford_QuarryGallery_Text_Reminder, MSGBOX_DEFAULT
	goto Virelia_Slateford_QuarryGallery_EventScript_Release

Virelia_Slateford_QuarryGallery_EventScript_ReturnTools:
	call VireliaFirstGym_EventScript_RewardTools
	goto_if_eq VAR_RESULT, FALSE, Virelia_Slateford_QuarryGallery_EventScript_BagFull
	playfanfare MUS_OBTAIN_ITEM
	waitfanfare
	msgbox Virelia_Slateford_QuarryGallery_Text_Reward, MSGBOX_DEFAULT
Virelia_Slateford_QuarryGallery_EventScript_Thanks:
	msgbox Virelia_Slateford_QuarryGallery_Text_Thanks, MSGBOX_DEFAULT
	goto Virelia_Slateford_QuarryGallery_EventScript_Release

Virelia_Slateford_QuarryGallery_EventScript_BagFull:
	msgbox Virelia_Slateford_QuarryGallery_Text_BagFull, MSGBOX_DEFAULT
	goto Virelia_Slateford_QuarryGallery_EventScript_Release

Virelia_Slateford_QuarryGallery_EventScript_Wrench::
	lock
	setvar VAR_0x8004, 1
	call VireliaFirstGym_EventScript_CollectTool
	goto_if_eq VAR_RESULT, FALSE, Virelia_Slateford_QuarryGallery_EventScript_AskMiner
	removeobject VAR_LAST_TALKED
	msgbox Virelia_Slateford_QuarryGallery_Text_Wrench, MSGBOX_DEFAULT
	goto Virelia_Slateford_QuarryGallery_EventScript_Release

Virelia_Slateford_QuarryGallery_EventScript_Hammer::
	lock
	setvar VAR_0x8004, 2
	call VireliaFirstGym_EventScript_CollectTool
	goto_if_eq VAR_RESULT, FALSE, Virelia_Slateford_QuarryGallery_EventScript_AskMiner
	removeobject VAR_LAST_TALKED
	msgbox Virelia_Slateford_QuarryGallery_Text_Hammer, MSGBOX_DEFAULT
	goto Virelia_Slateford_QuarryGallery_EventScript_Release

Virelia_Slateford_QuarryGallery_EventScript_Pick::
	lock
	setvar VAR_0x8004, 3
	call VireliaFirstGym_EventScript_CollectTool
	goto_if_eq VAR_RESULT, FALSE, Virelia_Slateford_QuarryGallery_EventScript_AskMiner
	removeobject VAR_LAST_TALKED
	msgbox Virelia_Slateford_QuarryGallery_Text_Pick, MSGBOX_DEFAULT
	goto Virelia_Slateford_QuarryGallery_EventScript_Release

Virelia_Slateford_QuarryGallery_EventScript_AskMiner:
	msgbox Virelia_Slateford_QuarryGallery_Text_AskMiner, MSGBOX_DEFAULT
	goto Virelia_Slateford_QuarryGallery_EventScript_Release

Virelia_Slateford_QuarryGallery_EventScript_Sableye::
	lock
	faceplayer
	goto_if_unset FLAG_VIRELIA_MISSING_TOOLS_STARTED, Virelia_Slateford_QuarryGallery_EventScript_AskMiner
	goto_if_unset FLAG_VIRELIA_TOOL_WRENCH, Virelia_Slateford_QuarryGallery_EventScript_MoreTools
	goto_if_unset FLAG_VIRELIA_TOOL_HAMMER, Virelia_Slateford_QuarryGallery_EventScript_MoreTools
	goto_if_unset FLAG_VIRELIA_TOOL_PICK, Virelia_Slateford_QuarryGallery_EventScript_MoreTools
	msgbox Virelia_Slateford_QuarryGallery_Text_Reveal, MSGBOX_DEFAULT
	closemessage
	dynmultichoice 18, 6, FALSE, 3, 0, DYN_MULTICHOICE_CB_NONE, Virelia_Slateford_QuarryGallery_Text_Fight, Virelia_Slateford_QuarryGallery_Text_Catch, Virelia_Slateford_QuarryGallery_Text_Later
	goto_if_eq VAR_RESULT, 0, Virelia_Slateford_QuarryGallery_EventScript_Fight
	goto_if_ne VAR_RESULT, 1, Virelia_Slateford_QuarryGallery_EventScript_Release
	msgbox Virelia_Slateford_QuarryGallery_Text_CatchAdvice, MSGBOX_DEFAULT
	goto Virelia_Slateford_QuarryGallery_EventScript_Battle

Virelia_Slateford_QuarryGallery_EventScript_Fight:
	msgbox Virelia_Slateford_QuarryGallery_Text_FightAdvice, MSGBOX_DEFAULT
Virelia_Slateford_QuarryGallery_EventScript_Battle:
	closemessage
	playmoncry SPECIES_SABLEYE, CRY_MODE_ENCOUNTER
	waitmoncry
	@ Both choices use an ordinary, catchable wild battle. No gift Pokemon.
	setwildbattle SPECIES_SABLEYE, 13
	dowildbattle
	call VireliaFirstGym_EventScript_FinishToolsBattle
	goto_if_eq VAR_RESULT, FALSE, Virelia_Slateford_QuarryGallery_EventScript_Retry
	removeobject LOCALID_VIRELIA_GALLERY_SABLEYE
	msgbox Virelia_Slateford_QuarryGallery_Text_Recovered, MSGBOX_DEFAULT
	goto Virelia_Slateford_QuarryGallery_EventScript_Release

Virelia_Slateford_QuarryGallery_EventScript_Retry:
	msgbox Virelia_Slateford_QuarryGallery_Text_Retry, MSGBOX_DEFAULT
	goto Virelia_Slateford_QuarryGallery_EventScript_Release

Virelia_Slateford_QuarryGallery_EventScript_MoreTools:
	msgbox Virelia_Slateford_QuarryGallery_Text_MoreTools, MSGBOX_DEFAULT
Virelia_Slateford_QuarryGallery_EventScript_Release:
	release
	end

Virelia_Slateford_QuarryGallery_EventScript_Hoard::
	goto_if_set FLAG_VIRELIA_MISSING_TOOLS_RECOVERED, Virelia_Slateford_QuarryGallery_EventScript_EmptyHoard
	msgbox Virelia_Slateford_QuarryGallery_Text_Hoard, MSGBOX_NPC
	end

Virelia_Slateford_QuarryGallery_EventScript_EmptyHoard:
	msgbox Virelia_Slateford_QuarryGallery_Text_EmptyHoard, MSGBOX_NPC
	end

Virelia_Slateford_QuarryGallery_EventScript_Sign::
	msgbox Virelia_Slateford_QuarryGallery_Text_Sign, MSGBOX_SIGN
	end

Virelia_Slateford_QuarryGallery_Text_Request:
	.string "Proud běží, ale mizí nám nářadí!\n"
	.string "Všichni zase viní Rakeťáky.\p"
	.string "Jenže rudu tentokrát nechali.\n"
	.string "Ztrácí se jen lesklé nástroje.\p"
	.string "Zjistíš, kdo je bere?$"

Virelia_Slateford_QuarryGallery_Text_Reminder:
	.string "U severozápadního žebříku\n"
	.string "se něco leskne. Začni tam.\p"
	.string "Vozíky už proud odsunul.\n"
	.string "Stará štola vede až k rozvodně.\p"
	.string "Posbírej po cestě tři nástroje\n"
	.string "a najdi zbytek výbavy.$"

Virelia_Slateford_QuarryGallery_Text_Wrench:
	.string "Nacházíš lesklý montážní klíč.\n"
	.string "Odkládáš ho stranou pro horníka.\p"
	.string "Drobné stopy vedou k žebříku\n"
	.string "na severozápadě. Žádné boty...$"

Virelia_Slateford_QuarryGallery_Text_Hammer:
	.string "Zvedáš hornické kladivo.\n"
	.string "Rukojeť je celá okousaná!\p"
	.string "Další záblesk je na severu.\n"
	.string "Rakeťáci snad koušou do železa?$"

Virelia_Slateford_QuarryGallery_Text_Pick:
	.string "Nacházíš malý krumpáč.\n"
	.string "Zase jen vyleštěný kov.\p"
	.string "Z východního výklenku se ozývá\n"
	.string "cinkání a tiché chichotání.$"

Virelia_Slateford_QuarryGallery_Text_Reveal:
	.string "Sableye si leští ukradenou pilu!\n"
	.string "Mezi šrouby se třpytí kamínky.\p"
	.string "Žádní Rakeťáci. Jen sběratel,\n"
	.string "kterému se líbí všechno lesklé.\p"
	.string "Zahnat ho soubojem, nebo\n"
	.string "zkusit získat nového parťáka?$"

Virelia_Slateford_QuarryGallery_Text_Fight:
	.string "Souboj$"
Virelia_Slateford_QuarryGallery_Text_Catch:
	.string "Chytit$"
Virelia_Slateford_QuarryGallery_Text_Later:
	.string "Později$"

Virelia_Slateford_QuarryGallery_Text_FightAdvice:
	.string "Poraz Sableye a vezmi nářadí.\n"
	.string "Pokud utečeš, můžeš to zkusit zas.$"

Virelia_Slateford_QuarryGallery_Text_CatchAdvice:
	.string "Použij v boji Poké Ball.\n"
	.string "Oslabeného Sableye chytíš snáz.\p"
	.string "I když omylem padne, nářadí\n"
	.string "můžeš horníkovi vrátit.$"

Virelia_Slateford_QuarryGallery_Text_Recovered:
	.string "Bereš pilu a zbylé nářadí.\n"
	.string "Záhada zmizelých nástrojů končí!\p"
	.string "Vrať je horníkovi u vchodu.\n"
	.string "Jižní žebříky tvoří zkratku.$"

Virelia_Slateford_QuarryGallery_Text_Retry:
	.string "Sableye se vrátil ke své sbírce.\n"
	.string "Nářadí tu stále čeká.$"

Virelia_Slateford_QuarryGallery_Text_MoreTools:
	.string "Sableye chrastí hromadou kovu.\n"
	.string "Nejdřív posbírej tři nástroje:\p"
	.string "u vchodu do lomu, na západě\n"
	.string "štoly a v severní chodbě.$"

Virelia_Slateford_QuarryGallery_Text_AskMiner:
	.string "Lesklé hornické nářadí...\n"
	.string "Zeptej se horníka u vchodu.$"

Virelia_Slateford_QuarryGallery_Text_Reward:
	.string "Takže to byl Sableye! A my už\n"
	.string "zase podezírali Rakeťáky.\p"
	.string "Díky za všechno nářadí.\n"
	.string "Vezmi si tenhle Hard Stone.\p"
	.string "Získáváš Hard Stone!\n"
	.string "Držiteli posílí kamenné útoky.$"

Virelia_Slateford_QuarryGallery_Text_Thanks:
	.string "Sableye má rád lesk, ne zločin.\n"
	.string "Příště mu necháme lesklý kamínek.\p"
	.string "Štola spojuje oba úseky lomu.\n"
	.string "Aspoň ušetříme pár kroků!$"

Virelia_Slateford_QuarryGallery_Text_BagFull:
	.string "Nářadí je v bezpečí. Na odměnu\n"
	.string "si ale uvolni místo v batohu.\p"
	.string "Hard Stone ti zatím schovám.$"

Virelia_Slateford_QuarryGallery_Text_Gate:
	.string "STARÁ ŠTOLA - servisní žebřík\n"
	.string "Vozík odsune elektrický pohon.$"

Virelia_Slateford_QuarryGallery_Text_Hoard:
	.string "Pila, šrouby a lesklé kamínky.\n"
	.string "Někdo je srovnal podle třpytu.$"

Virelia_Slateford_QuarryGallery_Text_EmptyHoard:
	.string "Nářadí je pryč. Ve sbírce\n"
	.string "zůstaly jen lesklé kamínky.$"

Virelia_Slateford_QuarryGallery_Text_Sign:
	.string "STARÁ SPOJOVACÍ ŠTOLA\n"
	.string "Západ: vstup. Východ: rozvodna.$"
