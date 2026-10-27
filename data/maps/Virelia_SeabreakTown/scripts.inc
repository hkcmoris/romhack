Virelia_SeabreakTown_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, Virelia_SeabreakTown_OnLoad
	.byte 0

Virelia_SeabreakTown_OnLoad:
	callnative VireliaPrologue_SetRivalAppearance
	callnative VireliaRainfall_SyncTownActors
	end

Virelia_SeabreakTown_EventScript_HarborMaster::
	lock
	faceplayer
	goto_if_set FLAG_VIRELIA_RAINFALL_COMPLETE, Virelia_SeabreakTown_HarborAfter
	goto_if_set FLAG_VIRELIA_SHIPPING_RECORD, Virelia_SeabreakTown_CreditScene
	goto_if_set FLAG_VIRELIA_LIGHTHOUSE_REPAIRED, Virelia_SeabreakTown_NeedRecord
	goto_if_set FLAG_VIRELIA_HARBOR_REPORT, Virelia_SeabreakTown_HarborReminder
	call VireliaRainfall_EventScript_HarborReport
	msgbox Virelia_SeabreakTown_Text_HarborReport, MSGBOX_DEFAULT
	release
	end

Virelia_SeabreakTown_HarborReminder:
	msgbox Virelia_SeabreakTown_Text_HarborReminder, MSGBOX_DEFAULT
	release
	end

Virelia_SeabreakTown_NeedRecord:
	msgbox Virelia_SeabreakTown_Text_NeedRecord, MSGBOX_DEFAULT
	release
	end

Virelia_SeabreakTown_CreditScene:
	call VireliaRainfall_EventScript_CompleteChapter
	goto_if_eq VAR_RESULT, FALSE, Virelia_SeabreakTown_HarborReminder
	clearflag FLAG_VIRELIA_HIDE_VACEK
	clearflag FLAG_VIRELIA_HIDE_HARBOR_BELA
	addobject 5
	addobject 4
	msgbox Virelia_SeabreakTown_Text_CreditScene, MSGBOX_DEFAULT
	msgbox Virelia_SeabreakTown_Text_BelaSuspicion, MSGBOX_DEFAULT
	msgbox Virelia_SeabreakTown_Text_HarborAfter, MSGBOX_DEFAULT
	release
	end

Virelia_SeabreakTown_HarborAfter:
	msgbox Virelia_SeabreakTown_Text_HarborAfter, MSGBOX_DEFAULT
	release
	end

Virelia_SeabreakTown_EventScript_Fisherman::
	lock
	faceplayer
	goto_if_set FLAG_VIRELIA_BOTTLE_REWARDED, Virelia_SeabreakTown_BottleAfter
	goto_if_eq VAR_VIRELIA_BOTTLE_STAGE, 4, Virelia_SeabreakTown_BottleReward
	goto_if_ge VAR_VIRELIA_BOTTLE_STAGE, 1, Virelia_SeabreakTown_BottleHint
	call VireliaRainfall_EventScript_BeginBottle
	msgbox Virelia_SeabreakTown_Text_BottleStart, MSGBOX_DEFAULT
	release
	end

Virelia_SeabreakTown_BottleHint:
	msgbox Virelia_SeabreakTown_Text_BottleHint, MSGBOX_DEFAULT
	release
	end

Virelia_SeabreakTown_BottleReward:
	call VireliaRainfall_EventScript_RewardBottle
	goto_if_eq VAR_RESULT, FALSE, VireliaRainfall_EventScript_BagFull
	playfanfare MUS_OBTAIN_ITEM
	msgbox Virelia_SeabreakTown_Text_BottleReward, MSGBOX_DEFAULT
	waitfanfare
	release
	end

Virelia_SeabreakTown_BottleAfter:
	msgbox Virelia_SeabreakTown_Text_BottleAfter, MSGBOX_DEFAULT
	release
	end

Virelia_SeabreakTown_EventScript_AnchorClue::
	lockall
	goto_if_lt VAR_VIRELIA_BOTTLE_STAGE, 1, Virelia_SeabreakTown_NoBottle
	setvar VAR_0x8004, 2
	call VireliaRainfall_EventScript_BottleClue
	msgbox Virelia_SeabreakTown_Text_AnchorClue, MSGBOX_DEFAULT
	releaseall
	end

Virelia_SeabreakTown_EventScript_CoastClue::
	lockall
	goto_if_lt VAR_VIRELIA_BOTTLE_STAGE, 2, Virelia_SeabreakTown_NoBottle
	setvar VAR_0x8004, 3
	call VireliaRainfall_EventScript_BottleClue
	msgbox Virelia_SeabreakTown_Text_CoastClue, MSGBOX_DEFAULT
	releaseall
	end

Virelia_SeabreakTown_NoBottle:
	msgbox Virelia_SeabreakTown_Text_NoBottle, MSGBOX_DEFAULT
	releaseall
	end

Virelia_SeabreakTown_EventScript_Rival::
	lock
	faceplayer
	goto_if_set FLAG_VIRELIA_RAINFALL_COMPLETE, Virelia_SeabreakTown_RivalAfter
	msgbox Virelia_SeabreakTown_Text_Rival, MSGBOX_DEFAULT
	release
	end
Virelia_SeabreakTown_RivalAfter:
	msgbox Virelia_SeabreakTown_Text_RivalAfter, MSGBOX_DEFAULT
	release
	end

Virelia_SeabreakTown_EventScript_Sailor::
	lock
	faceplayer
	goto_if_set FLAG_VIRELIA_HARBOR_REOPENED, Virelia_SeabreakTown_SailorAfter
	msgbox Virelia_SeabreakTown_Text_Sailor, MSGBOX_DEFAULT
	release
	end
Virelia_SeabreakTown_SailorAfter:
	msgbox Virelia_SeabreakTown_Text_SailorAfter, MSGBOX_DEFAULT
	release
	end

Virelia_SeabreakTown_Text_HarborReport:
	.string "SPRÁVCE: Vítej v Seabreak.\n"
	.string "Přístav držíme zavřený.\p"
	.string "Bouře vyřadila maják. Lodě čekají\n"
	.string "a několik zásilek chybí.\p"
	.string "Běla Křížová už prohlíží rozvody.\n"
	.string "Zkus jí pomoci v majáku.\p"
	.string "Stojí na skalnatém výběžku\n"
	.string "na východě města.$"

Virelia_SeabreakTown_Text_HarborReminder:
	.string "SPRÁVCE: Běla čeká v majáku.\n"
	.string "Bez jeho světla lodě nevyplují.$"

Virelia_SeabreakTown_Text_NeedRecord:
	.string "SPRÁVCE: Světlo zase svítí!\n"
	.string "Ještě prověř lodní knihu nahoře.\p"
	.string "Běla hledá původ ztracené zásilky.\n"
	.string "Pak můžeme obnovit provoz.$"

Virelia_SeabreakTown_Text_CreditScene:
	.string "VACEK: Výborně! Podle plánu.\n"
	.string "Můj tým situaci zvládl.\p"
	.string "BĚLA: Pane detektive, vždyť\n"
	.string "jste právě dorazil.\p"
	.string "VACEK: Načasování je při\n"
	.string "vyšetřování zásadní.\p"
	.string "REPORTÉR: Detektiv Radim Vacek!\n"
	.string "Další úspěch po Slatefordu?\p"
	.string "VACEK: Přesně tak. Do zprávy\n"
	.string "napište: zásah v pravý okamžik.$"

Virelia_SeabreakTown_Text_BelaSuspicion:
	.string "BĚLA: Maják svítí díky tobě.\n"
	.string "A díky tvým Pokémonům.\p"
	.string "Začínám chápat, proč tě ve\n"
	.string "Slatefordu všichni znají. Díky.$"

Virelia_SeabreakTown_Text_HarborAfter:
	.string "SPRÁVCE: Přístav zase funguje.\n"
	.string "Maják nám bezpečně ukazuje cestu.\p"
	.string "Na druhém stadionu teď kontrolují\n"
	.string "škody po bouři. Zatím je zavřený.\p"
	.string "Odpočiň si, prozkoumej pobřeží.\n"
	.string "Do Slatefordu se vrátíš kdykoli.$"

Virelia_SeabreakTown_Text_BottleStart:
	.string "RYBÁŘ OTA: Bouře přinesla láhev.\n"
	.string "Uvnitř je stará mapa pobřeží.\p"
	.string "Jmenuju se po dědovi. Ten se\n"
	.string "pohádal se strážcem majáku.\p"
	.string "Na mapě stojí: Kotva. Bílá čára.\n"
	.string "Pak dům, kde světlo odpočívá.\p"
	.string "Prohlédni starou kotvu na západě.\n"
	.string "Třeba ten vzkaz konečně pochopím.$"

Virelia_SeabreakTown_Text_BottleHint:
	.string "OTA: Mapa říká: Kotva, bílá čára,\n"
	.string "dům strážce. Půjdeš po pobřeží.\p"
	.string "Kotva leží v západním koutě.\n"
	.string "Značky hledej na pevnině.$"

Virelia_SeabreakTown_Text_BottleReward:
	.string "OTA: Děda se bál, že strážce\n"
	.string "opustil maják a nechal ho na moři.\p"
	.string "A on mu zatím zachránil bratra…\n"
	.string "Ten dopis přišel o mnoho let pozdě.\p"
	.string "Díky. Tohle nosí naše rodina.\n"
	.string "Teď pomůže tvému parťákovi.\p"
	.string "{PLAYER} získává Mystic Water!$"

Virelia_SeabreakTown_Text_BottleAfter:
	.string "OTA: Dopis dám k dědově fotce.\n"
	.string "Některé bouře doznívají dlouho.$"

Virelia_SeabreakTown_Text_AnchorClue:
	.string "Na kotvě je vyrytá šipka.\n"
	.string "Míří na bílou čáru pod útesem.\p"
	.string "Malý nápis: Příliv si pamatuje.\n"
	.string "Jdi podél pobřeží k bílé značce.$"

Virelia_SeabreakTown_Text_CoastClue:
	.string "Bílá čára značí dávnou bouři.\n"
	.string "V kameni je datum a malý klíč.\p"
	.string "Nápis: Den, kdy světlo zhaslo.\n"
	.string "Záznam zůstal v domě strážce.$"

Virelia_SeabreakTown_Text_NoBottle:
	.string "Stará pobřežní značka.\n"
	.string "Rybář u přístavu by mohl vědět víc.$"

Virelia_SeabreakTown_Text_Rival:
	.string "Tady se i lavičky tváří mokře.\n"
	.string "Já zatím prohlédnu přístav.\p"
	.string "V Center si vyléčíme týmy.\n"
	.string "Další souboj si necháme na jindy.$"

Virelia_SeabreakTown_Text_RivalAfter:
	.string "Maják svítí a Vacek má fotografii.\n"
	.string "Ty máš zase další dobrý skutek.\p"
	.string "Na stadionu prý opravují střechu.\n"
	.string "Čas navíc využiju k tréninku!$"

Virelia_SeabreakTown_Text_Sailor:
	.string "NÁMOŘNÍK: Bez světla nevypluju.\n"
	.string "Moře si na hrdiny nehraje.$"

Virelia_SeabreakTown_Text_SailorAfter:
	.string "NÁMOŘNÍK: Bedny už míří na lodě.\n"
	.string "Dnes konečně vyplujeme bezpečně!$"

Virelia_SeabreakTown_EventScript_Bela::
	msgbox Virelia_SeabreakTown_Text_Bela, MSGBOX_NPC
	end

Virelia_SeabreakTown_Text_Bela:
	.string "BĚLA: Zásilka vodivých minerálů\n"
	.string "vyjela ze Slateford Quarry.\p"
	.string "Příjemce v knize chybí. To je\n"
	.string "důvod k šetření, ne k závěrům.\p"
	.string "A Vackovu zprávu si ještě přečtu.\n"
	.string "Velmi pozorně.$"

Virelia_SeabreakTown_EventScript_Vacek::
	msgbox Virelia_SeabreakTown_Text_Vacek, MSGBOX_NPC
	end

Virelia_SeabreakTown_Text_Vacek:
	.string "VACEK: Radim Vacek, detektiv.\n"
	.string "Povýšení po zásahu ve Slatefordu.\p"
	.string "Úspěch vyžaduje správné lidi.\n"
	.string "A detektiva na správné fotografii.$"

Virelia_SeabreakTown_EventScript_Reporter::
	msgbox Virelia_SeabreakTown_Text_Reporter, MSGBOX_NPC
	end

Virelia_SeabreakTown_Text_Reporter:
	.string "REPORTÉR: Maják, bouře a detektiv.\n"
	.string "Titulek se dnes píše skoro sám.$"

Virelia_SeabreakTown_EventScript_TownSign::
	msgbox Virelia_SeabreakTown_Text_TownSign, MSGBOX_SIGN
	end

Virelia_SeabreakTown_Text_TownSign:
	.string "Seabreak Town\n"
	.string "Světlo mezi mořem a horami.$"

Virelia_SeabreakTown_EventScript_GymClosed::
	msgbox Virelia_SeabreakTown_Text_GymClosed, MSGBOX_SIGN
	end

Virelia_SeabreakTown_Text_GymClosed:
	.string "Stadion je dočasně uzavřený.\n"
	.string "Probíhá kontrola střechy po bouři.\p"
	.string "Termín otevření oznámí město.$"
