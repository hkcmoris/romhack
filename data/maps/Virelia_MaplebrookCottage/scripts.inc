Virelia_MaplebrookCottage_MapScripts::
	.byte 0

Virelia_MaplebrookCottage_EventScript_Ilona::
	lock
	faceplayer
#if VIRELIA_PROLOGUE && !VIRELIA_DEVELOPMENT_MODE && !IS_FRLG
	checkflag FLAG_VIRELIA_MAPLEBROOK_BERRY_GIFT
	goto_if TRUE, Virelia_MaplebrookCottage_EventScript_IlonaAfter
	checkflag FLAG_VIRELIA_STARTER_RECEIVED
	goto_if FALSE, Virelia_MaplebrookCottage_EventScript_IlonaBefore
	msgbox Virelia_MaplebrookCottage_Text_BerryOffer, MSGBOX_DEFAULT
	call Virelia_MaplebrookCottage_EventScript_TryGiveBerry
	goto_if_eq VAR_RESULT, FALSE, Virelia_MaplebrookCottage_EventScript_BagFull
	playfanfare MUS_OBTAIN_ITEM
	msgbox Virelia_MaplebrookCottage_Text_BerryReceived, MSGBOX_DEFAULT
	waitfanfare
	goto Virelia_MaplebrookCottage_EventScript_IlonaAfter
#endif
Virelia_MaplebrookCottage_EventScript_IlonaBefore:
	msgbox Virelia_MaplebrookCottage_Text_IlonaBefore, MSGBOX_DEFAULT
	release
	end

Virelia_MaplebrookCottage_EventScript_IlonaAfter:
	msgbox Virelia_MaplebrookCottage_Text_IlonaAfter, MSGBOX_DEFAULT
	release
	end

Virelia_MaplebrookCottage_EventScript_BagFull:
	msgbox Virelia_MaplebrookCottage_Text_BagFull, MSGBOX_DEFAULT
	release
	end

@ Keep the reward atomic and testable without dialogue or overworld state.
Virelia_MaplebrookCottage_EventScript_TryGiveBerry::
	setvar VAR_RESULT, FALSE
#if VIRELIA_PROLOGUE && !VIRELIA_DEVELOPMENT_MODE && !IS_FRLG
	checkflag FLAG_VIRELIA_STARTER_RECEIVED
	goto_if FALSE, Virelia_MaplebrookCottage_EventScript_RewardReturn
	checkflag FLAG_VIRELIA_MAPLEBROOK_BERRY_GIFT
	goto_if TRUE, Virelia_MaplebrookCottage_EventScript_RewardReturn
	additem ITEM_ORAN_BERRY, 1
	goto_if_eq VAR_RESULT, FALSE, Virelia_MaplebrookCottage_EventScript_RewardReturn
	setflag FLAG_VIRELIA_MAPLEBROOK_BERRY_GIFT
#endif
Virelia_MaplebrookCottage_EventScript_RewardReturn:
	return

Virelia_MaplebrookCottage_EventScript_Matej::
	msgbox Virelia_MaplebrookCottage_Text_Matej, MSGBOX_NPC
	end

Virelia_MaplebrookCottage_EventScript_Skitty::
	lock
	faceplayer
	playmoncry SPECIES_SKITTY, CRY_MODE_NORMAL
	msgbox Virelia_MaplebrookCottage_Text_Skitty, MSGBOX_DEFAULT
	waitmoncry
	release
	end

Virelia_MaplebrookCottage_EventScript_Notebook::
	msgbox Virelia_MaplebrookCottage_Text_Notebook, MSGBOX_SIGN
	end

Virelia_MaplebrookCottage_Text_IlonaBefore:
	.string "ILONA: Dveře tu nezamykáme.\n"
	.string "Skitty by stejně chtěla ven.\p"
	.string "Pěstuju bobule pro zdejší týmy.\n"
	.string "Stav se pak i se svým parťákem.$"

Virelia_MaplebrookCottage_Text_BerryOffer:
	.string "ILONA: Tak už jste na to dva!\n"
	.string "Tady je něco na první cestu.$"

Virelia_MaplebrookCottage_Text_BerryReceived:
	.string "Od Ilony dostáváš Oran Berry!$"

Virelia_MaplebrookCottage_Text_IlonaAfter:
	.string "ILONA: Dej bobuli Pokémonovi\n"
	.string "do držení. Použije ji v nouzi.\p"
	.string "Já si na cesty beru koláč.\n"
	.string "Ten ale Skitty většinou najde.$"

Virelia_MaplebrookCottage_Text_BagFull:
	.string "ILONA: V batohu už není místo.\n"
	.string "Bobule na tebe počká tady.$"

Virelia_MaplebrookCottage_Text_Matej:
	.string "MATĚJ: V hájku máme Wurmpla.\n"
	.string "Každý den mu počítám nožičky.\p"
	.string "Pokaždé se pohne.\n"
	.string "Výzkum je těžká práce.$"

Virelia_MaplebrookCottage_Text_Skitty:
	.string "SKITTY: Mňau!\p"
	.string "Zvědavě očichává košík s bobulemi.$"

Virelia_MaplebrookCottage_Text_Notebook:
	.string "ILONIN ZÁPISNÍK\n"
	.string "Bobule pro malé cestovatele\p"
	.string "Drženou bobuli použije Pokémon\n"
	.string "sám, až ji bude potřebovat.\p"
	.string "Poznámka na okraji:\n"
	.string "Skitty už není malý cestovatel.$"
