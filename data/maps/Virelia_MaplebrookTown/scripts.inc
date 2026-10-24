Virelia_MaplebrookTown_MapScripts::
	.byte 0

Virelia_MaplebrookTown_EventScript_Gardener::
	lock
	faceplayer
#if VIRELIA_PROLOGUE && !VIRELIA_DEVELOPMENT_MODE && !IS_FRLG
	checkflag FLAG_VIRELIA_SIDEQUEST_REWARDED
	goto_if TRUE, Virelia_MaplebrookTown_EventScript_HanaDone
	checkflag FLAG_VIRELIA_SIDEQUEST_RESOLVED
	goto_if TRUE, Virelia_MaplebrookTown_EventScript_HanaReward
	setflag FLAG_VIRELIA_SIDEQUEST_STARTED
	msgbox Virelia_MaplebrookTown_Text_HanaRequest, MSGBOX_DEFAULT
#else
	msgbox Virelia_MaplebrookTown_Text_Gardener, MSGBOX_DEFAULT
#endif
	release
	end

Virelia_MaplebrookTown_EventScript_HanaReward:
	msgbox Virelia_MaplebrookTown_Text_HanaFound, MSGBOX_DEFAULT
	call VireliaPrologue_EventScript_TryRewardHana
	goto_if_eq VAR_RESULT, FALSE, Virelia_MaplebrookTown_EventScript_HanaBagFull
	playfanfare MUS_OBTAIN_ITEM
	waitfanfare
	msgbox Virelia_MaplebrookTown_Text_HanaReward, MSGBOX_DEFAULT
	release
	end

Virelia_MaplebrookTown_EventScript_HanaBagFull:
	msgbox Virelia_MaplebrookTown_Text_HanaBagFull, MSGBOX_DEFAULT
	release
	end

Virelia_MaplebrookTown_EventScript_HanaDone:
	msgbox Virelia_MaplebrookTown_Text_HanaDone, MSGBOX_DEFAULT
	release
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

Virelia_MaplebrookTown_EventScript_NorthGate::
#if VIRELIA_PROLOGUE && !VIRELIA_DEVELOPMENT_MODE && !IS_FRLG
	checkflag FLAG_VIRELIA_SUPPLIES_RECEIVED
	goto_if TRUE, Virelia_MaplebrookTown_EventScript_GateOpen
	lockall
	checkflag FLAG_VIRELIA_STARTER_RECEIVED
	goto_if TRUE, Virelia_MaplebrookTown_EventScript_GateViktor
	msgbox Virelia_MaplebrookTown_Text_NeedPartner, MSGBOX_DEFAULT
	goto Virelia_MaplebrookTown_EventScript_GateStepBack

Virelia_MaplebrookTown_EventScript_GateViktor:
	msgbox Virelia_MaplebrookTown_Text_FinishLab, MSGBOX_DEFAULT

Virelia_MaplebrookTown_EventScript_GateStepBack:
	closemessage
	applymovement LOCALID_PLAYER, Virelia_MaplebrookTown_Movement_StepBack
	waitmovement 0
	releaseall
#endif
Virelia_MaplebrookTown_EventScript_GateOpen:
	end

Virelia_MaplebrookTown_Movement_StepBack:
	walk_down
	step_end

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

Virelia_MaplebrookTown_EventScript_ClosedCottage::
	msgbox Virelia_MaplebrookTown_Text_ClosedCottage, MSGBOX_SIGN
	end

Virelia_MaplebrookTown_Text_Gardener:
	.string "V každé zahradě tu necháváme\n"
	.string "místo i pro Pokémony.\p"
	.string "Profesorka pomůže tvému týmu\n"
	.string "odpočinout si před výpravou.$"

Virelia_MaplebrookTown_Text_HanaRequest:
	.string "HANA: Můj Zigzagoon se toulá\n"
	.string "po Whisperwood Trail.\p"
	.string "Naposledy běžel k západní\n"
	.string "pěšině, někam do vysoké trávy.\p"
	.string "Podíváš se po něm? Možná jen\n"
	.string "zase hlídá cizí zahrádku.$"

Virelia_MaplebrookTown_Text_HanaFound:
	.string "HANA: Hlídá hnízdo Wurmplů?\n"
	.string "Tak proto se mu nechce domů!\p"
	.string "Dojdu za ním. Díky, že už vím,\n"
	.string "kde se ten malý strážce toulá.$"

Virelia_MaplebrookTown_Text_HanaReward:
	.string "Od Hany dostáváš Potion.\p"
	.string "HANA: Ten je pro tvého parťáka.\n"
	.string "Tobě na škrábance stačí náplast.\p"
	.string "Za hnízdem se propleť větvemi.\n"
	.string "Zigzagoon ti teď ukáže pěšinu.$"

Virelia_MaplebrookTown_Text_HanaBagFull:
	.string "Mám pro tebe Potion, ale batoh\n"
	.string "je plný. Stav se pro něj později.$"

Virelia_MaplebrookTown_Text_HanaDone:
	.string "HANA: Strážci hnízd se hodí.\n"
	.string "Jen ta přesčasová práce…\p"
	.string "Zkus pěšinu za jeho hnízdem.\n"
	.string "Hlídá tam i něco mnohem staršího.$"

Virelia_MaplebrookTown_Text_Zigzagoon:
	.string "ZIGZAGOON: Zig! Zig!$"

Virelia_MaplebrookTown_Text_TrailWalker:
	.string "Whisperwood Trail vede na sever\n"
	.string "do Slateford City.\p"
	.string "Vedlejší pěšiny stojí za pohled.\n"
	.string "Hlavní cesta se ale neztratí.$"

Virelia_MaplebrookTown_Text_NeedPartner:
	.string "Bez Pokémona do trávy nechoď.\n"
	.string "Profesorka čeká v laboratoři.$"

Virelia_MaplebrookTown_Text_FinishLab:
	.string "Nejdřív dokonči cvičný souboj\n"
	.string "a vezmi si výbavu od profesorky.$"

Virelia_MaplebrookTown_Text_HomeSign:
	.string "DOMOV: {PLAYER}\n"
	.string "Maplebrook Town$"

Virelia_MaplebrookTown_Text_LabSign:
	.string "VÝZKUMNÁ LABORATOŘ\n"
	.string "Profesorka Javorová$"

Virelia_MaplebrookTown_Text_NorthSign:
	.string "CESTA 01 - Whisperwood Trail\n"
	.string "Na sever: Slateford City$"

Virelia_MaplebrookTown_Text_TownSign:
	.string "Maplebrook Town\n"
	.string "Malý domov pro nové začátky.$"

Virelia_MaplebrookTown_Text_ClosedCottage:
	.string "Sousedé se právě starají\n"
	.string "o své zahrady.$"
