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

Virelia_MaplebrookTown_EventScript_CottageSign::
	msgbox Virelia_MaplebrookTown_Text_CottageSign, MSGBOX_SIGN
	end

Virelia_MaplebrookTown_EventScript_EmptyHouseSign::
	msgbox Virelia_MaplebrookTown_Text_EmptyHouseSign, MSGBOX_SIGN
	end

Virelia_MaplebrookTown_EventScript_FishermansHouseSign::
	msgbox Virelia_MaplebrookTown_Text_FishermansHouseSign, MSGBOX_SIGN
	end

Virelia_MaplebrookTown_EventScript_Noticeboard::
#if VIRELIA_PROLOGUE && !VIRELIA_DEVELOPMENT_MODE && !IS_FRLG
	checkflag FLAG_VIRELIA_LAB_RESCUE_COMPLETE
	goto_if TRUE, Virelia_MaplebrookTown_EventScript_NoticeAfterLab
#endif
	msgbox Virelia_MaplebrookTown_Text_Noticeboard, MSGBOX_SIGN
	end
Virelia_MaplebrookTown_EventScript_NoticeAfterLab:
	msgbox Virelia_MaplebrookTown_Text_NoticeAfterLab, MSGBOX_SIGN
	end

Virelia_MaplebrookTown_EventScript_Courier::
	lock
	faceplayer
#if VIRELIA_PROLOGUE && !VIRELIA_DEVELOPMENT_MODE && !IS_FRLG
	checkflag FLAG_VIRELIA_ROCKET_DEFEATED
	goto_if TRUE, Virelia_MaplebrookTown_EventScript_CourierAfterRocket
	checkflag FLAG_VIRELIA_LAB_RESCUE_COMPLETE
	goto_if TRUE, Virelia_MaplebrookTown_EventScript_CourierAfterLab
#endif
	msgbox Virelia_MaplebrookTown_Text_Courier, MSGBOX_DEFAULT
	release
	end
Virelia_MaplebrookTown_EventScript_CourierAfterLab:
	msgbox Virelia_MaplebrookTown_Text_CourierAfterLab, MSGBOX_DEFAULT
	release
	end
Virelia_MaplebrookTown_EventScript_CourierAfterRocket:
	msgbox Virelia_MaplebrookTown_Text_CourierAfterRocket, MSGBOX_DEFAULT
	release
	end

Virelia_MaplebrookTown_EventScript_Fisherman::
	msgbox Virelia_MaplebrookTown_Text_Fisherman, MSGBOX_NPC
	end

Virelia_MaplebrookTown_EventScript_Psyduck::
	lock
	faceplayer
	playmoncry SPECIES_PSYDUCK, CRY_MODE_NORMAL
	applymovement LOCALID_VIRELIA_MAPLEBROOK_PSYDUCK, Virelia_MaplebrookTown_Movement_Hop
	waitmovement LOCALID_VIRELIA_MAPLEBROOK_PSYDUCK
	msgbox Virelia_MaplebrookTown_Text_Psyduck, MSGBOX_DEFAULT
	waitmoncry
	release
	end

Virelia_MaplebrookTown_EventScript_Ema::
	msgbox Virelia_MaplebrookTown_Text_Ema, MSGBOX_NPC
	end

Virelia_MaplebrookTown_EventScript_Poochyena::
	lock
	faceplayer
	playmoncry SPECIES_POOCHYENA, CRY_MODE_NORMAL
	msgbox Virelia_MaplebrookTown_Text_Poochyena, MSGBOX_DEFAULT
	waitmoncry
	release
	end

Virelia_MaplebrookTown_EventScript_Wurmple::
	lock
	faceplayer
	playmoncry SPECIES_WURMPLE, CRY_MODE_NORMAL
	msgbox Virelia_MaplebrookTown_Text_Wurmple, MSGBOX_DEFAULT
	waitmoncry
	release
	end

Virelia_MaplebrookTown_EventScript_Nursery::
	msgbox Virelia_MaplebrookTown_Text_Nursery, MSGBOX_SIGN
	end

Virelia_MaplebrookTown_EventScript_GroveFind::
	lockall
#if VIRELIA_PROLOGUE && !VIRELIA_DEVELOPMENT_MODE && !IS_FRLG
	checkflag FLAG_VIRELIA_MAPLEBROOK_GROVE_FIND
	goto_if TRUE, Virelia_MaplebrookTown_EventScript_GroveEmpty
	call Virelia_MaplebrookTown_EventScript_TryFindAntidote
	goto_if_eq VAR_RESULT, FALSE, Virelia_MaplebrookTown_EventScript_GroveBagFull
	playfanfare MUS_OBTAIN_ITEM
	msgbox Virelia_MaplebrookTown_Text_GroveFound, MSGBOX_DEFAULT
	waitfanfare
	releaseall
	end
#endif
Virelia_MaplebrookTown_EventScript_GroveEmpty:
	msgbox Virelia_MaplebrookTown_Text_GroveEmpty, MSGBOX_DEFAULT
	releaseall
	end
Virelia_MaplebrookTown_EventScript_GroveBagFull:
	msgbox Virelia_MaplebrookTown_Text_GroveBagFull, MSGBOX_DEFAULT
	releaseall
	end

Virelia_MaplebrookTown_EventScript_TryFindAntidote::
	setvar VAR_RESULT, FALSE
#if VIRELIA_PROLOGUE && !VIRELIA_DEVELOPMENT_MODE && !IS_FRLG
	checkflag FLAG_VIRELIA_MAPLEBROOK_GROVE_FIND
	goto_if TRUE, Virelia_MaplebrookTown_EventScript_FindReturn
	additem ITEM_ANTIDOTE, 1
	goto_if_eq VAR_RESULT, FALSE, Virelia_MaplebrookTown_EventScript_FindReturn
	setflag FLAG_VIRELIA_MAPLEBROOK_GROVE_FIND
#endif
Virelia_MaplebrookTown_EventScript_FindReturn:
	return

Virelia_MaplebrookTown_Movement_Hop:
	jump_in_place_down
	step_end

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

Virelia_MaplebrookTown_Text_CottageSign:
	.string "U ILONY\n"
	.string "Bobule, návštěvy a jedna Skitty.$"

Virelia_MaplebrookTown_Text_EmptyHouseSign:
	.string "Dům je zatím prázdný.\n"
	.string "Dveře jsou zamčené.$"

Virelia_MaplebrookTown_Text_FishermansHouseSign:
	.string "RYBÁŘŮV DŮM\n"
	.string "Když nejsem doma, jsem u řeky.$"

Virelia_MaplebrookTown_Text_Noticeboard:
	.string "OBECNÍ NÁSTĚNKA\n"
	.string "Dnes se zametá náves.\p"
	.string "Ilona vítá návštěvy.\n"
	.string "Skitty vítá hlavně svačiny.\p"
	.string "V hájku za kamenem občas\n"
	.string "něco zůstane po výpravách.$"

Virelia_MaplebrookTown_Text_NoticeAfterLab:
	.string "DNEŠNÍ ZPRÁVA Z LABORATOŘE\n"
	.string "Meowth už Rattatu nepronásleduje.\p"
	.string "Prosíme, nezkoušejte na dveřích\n"
	.string "cedulku: Pozor, volný výběh.\p"
	.string "Ilona má otevřeno jako vždy.\n"
	.string "Svačiny jsou pod dohledem Skitty.$"

Virelia_MaplebrookTown_Text_Courier:
	.string "KURÝR: Zásilku mám pro Javorovou.\n"
	.string "Je na ní nápis: Neobracet.\p"
	.string "A šipky na všech stranách.\n"
	.string "Věda je plná záhad.$"

Virelia_MaplebrookTown_Text_CourierAfterLab:
	.string "KURÝR: Ten rámus z laboratoře?\n"
	.string "Tentokrát to nebyl můj balík.\p"
	.string "Tohle si napíšu do hlášení.\n"
	.string "Pro jistotu dvakrát.$"

Virelia_MaplebrookTown_Text_CourierAfterRocket:
	.string "KURÝR: Ze Slatefordu přišlo\n"
	.string "varování před falešnými dělníky.\p"
	.string "Teď chtějí podpis i po mně.\n"
	.string "A já tam nosím poštu každý den.$"

Virelia_MaplebrookTown_Text_Fisherman:
	.string "U řeky mám všechno po ruce.\n"
	.string "I domů to mám jen pár kroků.\p"
	.string "Psyduck sem chodí rybařit taky.\n"
	.string "Jen neví, co je to prut.$"

Virelia_MaplebrookTown_Text_Psyduck:
	.string "PSYDUCK: Psy?\p"
	.string "Nadšeně předvádí svůj úlovek.\n"
	.string "Je to mokrý list.$"

Virelia_MaplebrookTown_Text_Ema:
	.string "EMA: Učíme se povel Sedni!\n"
	.string "Zatím spíš cvičím já.\p"
	.string "Do hájku chodíme odpočívat.\n"
	.string "Hledej tam i u starého kamene.$"

Virelia_MaplebrookTown_Text_Poochyena:
	.string "POOCHYENA: Haf!\p"
	.string "Čeká na další hru.\n"
	.string "Výcvik se zřejmě vydařil.$"

Virelia_MaplebrookTown_Text_Wurmple:
	.string "WURMPLE: Prrr…\p"
	.string "Pilně zkoumá spodní stranu listu.$"

Virelia_MaplebrookTown_Text_Nursery:
	.string "OBECNÍ ŠKOLKA SAZENIC\n"
	.string "Zalévat. Neochutnávat.\p"
	.string "Pod cedulkou je poznámka:\n"
	.string "To platí i pro Skitty.$"

Virelia_MaplebrookTown_Text_GroveFound:
	.string "Za kamenem se leskne lahvička.\n"
	.string "Do batohu ukládáš Antidote.$"

Virelia_MaplebrookTown_Text_GroveEmpty:
	.string "Ve stínu je klid a suché listí.\n"
	.string "Wurmple tu má všechno potřebné.$"

Virelia_MaplebrookTown_Text_GroveBagFull:
	.string "Za kamenem leží Antidote.\n"
	.string "Nejdřív uvolni místo v batohu.$"
