Virelia_SlatefordCity_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, Virelia_SlatefordCity_FirstGymOnLoad
#if VIRELIA_PROLOGUE && !VIRELIA_DEVELOPMENT_MODE && !IS_FRLG
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Virelia_SlatefordCity_OnFrame
#endif
	.byte 0

@ The old prologue frame condition remains unchanged. New chapter gates are
@ derived from independent persistent flags and never reset the prologue.
Virelia_SlatefordCity_FirstGymOnLoad:
	callnative VireliaFirstGym_BeginChapter
	setmetatile 22, 9, 0x1C4, 1
	setobjectxyperm LOCALID_VIRELIA_SLATEFORD_MANAGER, 25, 18
	call_if_set FLAG_VIRELIA_PROLOGUE_COMPLETE, Virelia_SlatefordCity_PositionManager
	call_if_set FLAG_VIRELIA_QUARRY_COMPLETE, Virelia_SlatefordCity_OpenGym
	end

Virelia_SlatefordCity_PositionManager:
	setobjectxyperm LOCALID_VIRELIA_SLATEFORD_MANAGER, 24, 18
	return

Virelia_SlatefordCity_OpenQuarry:
	applymovement LOCALID_VIRELIA_SLATEFORD_MANAGER, Virelia_SlatefordCity_Movement_ManagerStepAside
	waitmovement LOCALID_VIRELIA_SLATEFORD_MANAGER
	copyobjectxytoperm LOCALID_VIRELIA_SLATEFORD_MANAGER
	return

Virelia_SlatefordCity_OpenGym:
	setmetatile 22, 9, METATILE_General_Door_Gym, 1
	return

Virelia_SlatefordCity_EventScript_Manager::
	lock
	faceplayer
	goto_if_unset FLAG_VIRELIA_PROLOGUE_COMPLETE, Virelia_SlatefordCity_EventScript_ManagerBefore
	goto_if_set FLAG_VIRELIA_QUARRY_COMPLETE, Virelia_SlatefordCity_EventScript_ManagerAfter
	goto_if_unset FLAG_VIRELIA_QUARRY_EVIDENCE, Virelia_SlatefordCity_EventScript_ManagerTask
	call VireliaFirstGym_EventScript_CompleteQuarry
	goto_if_eq VAR_RESULT, FALSE, Virelia_SlatefordCity_EventScript_ManagerTask
	call Virelia_SlatefordCity_OpenGym
	special DrawWholeMapView
	msgbox Virelia_SlatefordCity_Text_ManagerThanks, MSGBOX_DEFAULT
	release
	end

Virelia_SlatefordCity_EventScript_ManagerBefore:
	msgbox Virelia_SlatefordCity_Text_ManagerBefore, MSGBOX_DEFAULT
	release
	end

Virelia_SlatefordCity_EventScript_ManagerTask:
	checkplayergender
	goto_if_eq VAR_RESULT, FEMALE, Virelia_SlatefordCity_EventScript_ManagerTaskFemale
	msgbox Virelia_SlatefordCity_Text_ManagerTask, MSGBOX_DEFAULT
	release
	end

Virelia_SlatefordCity_EventScript_ManagerTaskFemale:
	msgbox Virelia_SlatefordCity_Text_ManagerTaskFemale, MSGBOX_DEFAULT
	release
	end

Virelia_SlatefordCity_EventScript_ManagerAfter:
	msgbox Virelia_SlatefordCity_Text_ManagerAfter, MSGBOX_DEFAULT
	release
	end

Virelia_SlatefordCity_OnFrame:
	map_script_2 VAR_VIRELIA_PROLOGUE_STATE, VIRELIA_STATE_ROCKET, Virelia_SlatefordCity_EventScript_ResolveRocket
	.2byte 0

Virelia_SlatefordCity_EventScript_Miner::
	lock
	faceplayer
#if VIRELIA_PROLOGUE && !VIRELIA_DEVELOPMENT_MODE && !IS_FRLG
	checkflag FLAG_VIRELIA_PROLOGUE_COMPLETE
	goto_if TRUE, Virelia_SlatefordCity_EventScript_MinerAfter
	setflag FLAG_VIRELIA_MINER_REPORT
	msgbox Virelia_SlatefordCity_Text_MinerReport, MSGBOX_DEFAULT
#else
	msgbox Virelia_SlatefordCity_Text_Miner, MSGBOX_DEFAULT
#endif
	release
	end

Virelia_SlatefordCity_EventScript_MinerAfter:
	goto_if_set FLAG_VIRELIA_QUARRY_COMPLETE, Virelia_SlatefordCity_EventScript_ManagerAfter
	msgbox Virelia_SlatefordCity_Text_MinerAfter, MSGBOX_DEFAULT
	release
	end

Virelia_SlatefordCity_EventScript_Resident::
	msgbox Virelia_SlatefordCity_Text_Resident, MSGBOX_NPC
	end

Virelia_SlatefordCity_EventScript_MaintenanceWorker::
	lock
	faceplayer
#if VIRELIA_PROLOGUE && !VIRELIA_DEVELOPMENT_MODE && !IS_FRLG
	checkflag FLAG_VIRELIA_MINER_REPORT
	goto_if FALSE, Virelia_SlatefordCity_EventScript_WorkerBusy
	checkflag FLAG_VIRELIA_SUPPLIES_RECEIVED
	goto_if FALSE, Virelia_SlatefordCity_EventScript_WorkerBusy
	msgbox Virelia_SlatefordCity_Text_Worker, MSGBOX_DEFAULT
	trainerbattle_single TRAINER_VIRELIA_ROCKET, Virelia_SlatefordCity_Text_RocketIntro, Virelia_SlatefordCity_Text_RocketDefeat, Virelia_SlatefordCity_EventScript_AfterRocket
#else
	msgbox Virelia_SlatefordCity_Text_Maintenance, MSGBOX_DEFAULT
#endif
	release
	end

Virelia_SlatefordCity_EventScript_WorkerBusy:
	msgbox Virelia_SlatefordCity_Text_WorkerBusy, MSGBOX_DEFAULT
	release
	end

Virelia_SlatefordCity_EventScript_AfterRocket:
	callnative VireliaPrologue_FinishRocket
	msgbox Virelia_SlatefordCity_Text_RocketEscape, MSGBOX_DEFAULT
	closemessage
	specialvar VAR_0x8008, GetPlayerFacingDirection
	goto_if_eq VAR_0x8008, DIR_EAST, Virelia_SlatefordCity_EventScript_RocketDetour
	applymovement LOCALID_VIRELIA_SLATEFORD_ROCKET, Virelia_SlatefordCity_Movement_RocketToRoad
	goto Virelia_SlatefordCity_EventScript_RocketEscapeRoad

@ When approached from the west, go around the player before joining the road.
Virelia_SlatefordCity_EventScript_RocketDetour:
	applymovement LOCALID_VIRELIA_SLATEFORD_ROCKET, Virelia_SlatefordCity_Movement_RocketAroundPlayer

Virelia_SlatefordCity_EventScript_RocketEscapeRoad:
	waitmovement LOCALID_VIRELIA_SLATEFORD_ROCKET
	applymovement LOCALID_VIRELIA_SLATEFORD_ROCKET, Virelia_SlatefordCity_Movement_RocketExit

Virelia_SlatefordCity_EventScript_RocketGone:
	waitmovement LOCALID_VIRELIA_SLATEFORD_ROCKET
	@ The southern exit is fully below the camera before the actor is removed.
	removeobject LOCALID_VIRELIA_SLATEFORD_ROCKET
	goto Virelia_SlatefordCity_EventScript_ResolveRocket

Virelia_SlatefordCity_EventScript_ResolveRocket::
#if VIRELIA_PROLOGUE && !VIRELIA_DEVELOPMENT_MODE && !IS_FRLG
	lockall
	hidefollower
	@ Position the hidden template before spawning: no visible teleport/reset.
	setobjectxyperm LOCALID_VIRELIA_SLATEFORD_POLICEMAN, 14, 29
	setobjectxyperm LOCALID_VIRELIA_SLATEFORD_GROWLITHE, 14, 29
	clearflag FLAG_VIRELIA_HIDE_POLICEMAN
	addobject LOCALID_VIRELIA_SLATEFORD_POLICEMAN
	@ Leave one tile between the officer and his companion as they enter.
	applymovement LOCALID_VIRELIA_SLATEFORD_POLICEMAN, Virelia_SlatefordCity_Movement_PolicemanLeadIn
	waitmovement LOCALID_VIRELIA_SLATEFORD_POLICEMAN
	addobject LOCALID_VIRELIA_SLATEFORD_GROWLITHE
	applymovement LOCALID_VIRELIA_SLATEFORD_POLICEMAN, Virelia_SlatefordCity_Movement_Policeman
	applymovement LOCALID_VIRELIA_SLATEFORD_GROWLITHE, Virelia_SlatefordCity_Movement_Growlithe
	waitmovement LOCALID_VIRELIA_SLATEFORD_POLICEMAN
	waitmovement LOCALID_VIRELIA_SLATEFORD_GROWLITHE
	copyobjectxytoperm LOCALID_VIRELIA_SLATEFORD_POLICEMAN
	copyobjectxytoperm LOCALID_VIRELIA_SLATEFORD_GROWLITHE
	turnobject LOCALID_PLAYER, DIR_WEST
	msgbox Virelia_SlatefordCity_Text_Policeman, MSGBOX_DEFAULT
	callnative VireliaPrologue_Complete
	call VireliaFirstGym_EventScript_BeginChapter
	closemessage
	call Virelia_SlatefordCity_OpenQuarry
	msgbox Virelia_SlatefordCity_Text_PrologueComplete, MSGBOX_DEFAULT
	releaseall
#endif
	end

Virelia_SlatefordCity_EventScript_Policeman::
	msgbox Virelia_SlatefordCity_Text_PoliceAfter, MSGBOX_NPC
	end

Virelia_SlatefordCity_EventScript_Growlithe::
	lock
	faceplayer
	playmoncry SPECIES_GROWLITHE, CRY_MODE_NORMAL
	waitmoncry
	release
	end

Virelia_SlatefordCity_Movement_ManagerStepAside:
	walk_left
	face_down
	step_end

Virelia_SlatefordCity_Movement_RocketAroundPlayer:
	walk_fast_up
	walk_fast_left
	walk_fast_left
	walk_fast_down
	step_end

Virelia_SlatefordCity_Movement_RocketToRoad:
	walk_fast_left
	walk_fast_left
	step_end

@ (20,22) -> (17,22) -> (17,24) -> (14,24) -> (14,29), via the quarry steps.
Virelia_SlatefordCity_Movement_RocketExit:
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_down
	walk_fast_down
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	step_end

@ Enter from beyond the bottom of the camera, then walk across the square.
Virelia_SlatefordCity_Movement_PolicemanLeadIn:
	walk_up
	step_end

Virelia_SlatefordCity_Movement_Policeman:
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	face_player
	delay_16
	step_end

@ Follow the same route one tile behind, including the turn onto the square.
Virelia_SlatefordCity_Movement_Growlithe:
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	face_right
	step_end

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

Virelia_SlatefordCity_EventScript_ServiceSign::
	msgbox Virelia_SlatefordCity_Text_ServiceSign, MSGBOX_SIGN
	end

Virelia_SlatefordCity_EventScript_ClosedGym::
	goto_if_set FLAG_VIRELIA_QUARRY_COMPLETE, Virelia_SlatefordCity_EventScript_GymSign
	msgbox Virelia_SlatefordCity_Text_ClosedGym, MSGBOX_SIGN
	end

Virelia_SlatefordCity_EventScript_ClosedHouse::
	msgbox Virelia_SlatefordCity_Text_ClosedHouse, MSGBOX_SIGN
	end

Virelia_SlatefordCity_EventScript_ClosedOffice::
	msgbox Virelia_SlatefordCity_Text_ClosedOffice, MSGBOX_SIGN
	end

Virelia_SlatefordCity_Text_Miner:
	.string "Tohle město vyrostlo z kamene,\n"
	.string "ale domov z něj dělají lidé.$"

Virelia_SlatefordCity_Text_MinerReport:
	.string "HORNÍK: Zmizely nám vzorky\n"
	.string "vodivých minerálů z lomu.\p"
	.string "Někdo se vydává za údržbáře\n"
	.string "a ptá se na další zásilku.\p"
	.string "Stojí u cesty k lomu. Poznáš ho.\p"
	.string "Má čisté boty.\n"
	.string "To je hodně podezřelé.$"

Virelia_SlatefordCity_Text_MinerAfter:
	.string "Horník: Správce čeká u vchodu.\n"
	.string "Rozvodna je pořád mimo provoz.\p"
	.string "Ať nejdřív běží lom.\n"
	.string "Potom se otevře stadion.$"


Virelia_SlatefordCity_Text_Resident:
	.string "Pokémon Center je na severozápadě.\n"
	.string "Poké Mart leží pod ním.\p"
	.string "Sestra Joy tě ráda ošetří.\n"
	.string "Už nemusíš běžet až domů.$"


Virelia_SlatefordCity_Text_WorkerBusy:
	.string "ÚDRŽBÁŘ: Neotravuj, mám práci.\p"
	.string "Zeptej se horníka u lomu,\n"
	.string "ten ví všechno.$"

Virelia_SlatefordCity_Text_Maintenance:
	.string "ÚDRŽBÁŘ: Probíhá průzkum lomu.\n"
	.string "Vstup zatím není povolený.$"

Virelia_SlatefordCity_Text_Worker:
	.string "ÚDRŽBÁŘ: Ukradené minerály?\n"
	.string "Já jen kontroluji jejich odvoz.\p"
	.string "To R na rukávu? To je jen Revize.\n"
	.string "Velmi důkladná revize…$"

Virelia_SlatefordCity_Text_RocketIntro:
	.string "POSKOK: No dobře. Rakeťáci!\n"
	.string "A ten náklad si necháme!$"

Virelia_SlatefordCity_Text_RocketDefeat:
	.string "Tohle v pracovní smlouvě nebylo!$"

Virelia_SlatefordCity_Text_RocketEscape:
	.string "POSKOK: Však my se vrátíme!\p"
	.string "Při útěku mu vypadl vodivý\n"
	.string "minerál a štítek ze zásilky.$"

Virelia_SlatefordCity_Text_Policeman:
	.string "POLICISTA: Výborně. Situaci jsem\n"
	.string "měl celou dobu pod dohledem.\p"
	.string "Ty řešíš souboje.\n"
	.string "Já papíry. Dobrá spolupráce.\p"
	.string "Vzorek předám lidem z lomu.\n"
	.string "Ten štítek si zaslouží prověřit.\p"
	.string "Já jsem tak dobrej…$"

Virelia_SlatefordCity_Text_PrologueComplete:
	.string "První výprava je za tebou!\n"
	.string "Další stopa vede do lomu.\p"
	.string "Potom přijde na řadu stadion.\n"
	.string "Správce čeká před vchodem.\p"
	.string "Promluv s ním a prověř štítek.$"


Virelia_SlatefordCity_Text_PoliceAfter:
	.string "POLICISTA: Vyšetřování běží.\n"
	.string "Papíry bohužel rychlejší nejsou.$"

Virelia_SlatefordCity_Text_Stone:
	.string "Označený vzorek z lomu.\n"
	.string "Nech ho prosím pracovníkům.$"

Virelia_SlatefordCity_Text_GymSign:
	.string "STADION - Slateford City\n"
	.string "Mara, geoložka a vedoucí.$"


Virelia_SlatefordCity_Text_CitySign:
	.string "Slateford City\n"
	.string "Pevné kořeny v kameni.$"

Virelia_SlatefordCity_Text_QuarrySign:
	.string "Slateford Quarry\n"
	.string "Vstup po domluvě se správcem.$"


Virelia_SlatefordCity_Text_ServiceSign:
	.string "Pokémon Centrum a Poké Mart\n"
	.string "Léčení i výbava pod jednou střechou.$"


Virelia_SlatefordCity_Text_ClosedGym:
	.string "Stadion čeká na obnovení provozu\n"
	.string "v lomu. Promluv se správcem.$"


Virelia_SlatefordCity_Text_ClosedHouse:
	.string "Dveře jsou zamčené.\n"
	.string "Obyvatelé jsou pryč.$"

Virelia_SlatefordCity_Text_ClosedOffice:
	.string "Důlní správa se připravuje\n"
	.string "na příchod nového týmu.$"

Virelia_SlatefordCity_Text_ManagerBefore:
	.string "SPRÁVCE: V lomu se kradlo.\n"
	.string "Nejdřív prověř toho údržbáře.\p"
	.string "Do poškozených rozvodů zatím\n"
	.string "nikoho pouštět nebudu.$"

Virelia_SlatefordCity_Text_ManagerTask:
	.string "SPRÁVCE: Tak štítek od poskoka?\n"
	.string "Podobný zůstal i dole v lomu.\p"
	.string "Zmizely jen vodivé minerály.\n"
	.string "Navíc poškodili rozvodnu.\p"
	.string "Bruno a Lenka prověří tvůj tým.\n"
	.string "Pomohou ti se spuštěním.\p"
	.string "Vrať proud a prohlédni sklad.\n"
	.string "Pak mi řekni, co jsi našel.$"

Virelia_SlatefordCity_Text_ManagerTaskFemale:
	.string "SPRÁVCE: Tak štítek od poskoka?\n"
	.string "Podobný zůstal i dole v lomu.\p"
	.string "Zmizely jen vodivé minerály.\n"
	.string "Navíc poškodili rozvodnu.\p"
	.string "Bruno a Lenka prověří tvůj tým.\n"
	.string "Pomohou ti se spuštěním.\p"
	.string "Vrať proud a prohlédni sklad.\n"
	.string "Pak mi řekni, co jsi našla.$"

Virelia_SlatefordCity_Text_ManagerThanks:
	.string "SPRÁVCE: Čísla zásilek sedí.\n"
	.string "Rakeťáci chtěli vodivé minerály.\p"
	.string "Děláme z nich vodiče pro stroje,\n"
	.string "které potřebují hodně proudu.\p"
	.string "Jejich účel ale neznáme.\n"
	.string "Policii předám tvé zjištění.\p"
	.string "Rozvodna funguje. Díky tobě\n"
	.string "můžeme zase otevřít lom.\p"
	.string "Mara se vrací na stadion.\n"
	.string "Dveře už jsou otevřené.\p"
	.string "A ty dnes končíš směnu.\n"
	.string "Výplata je zatím vděčnost.$"

Virelia_SlatefordCity_Text_ManagerAfter:
	.string "SPRÁVCE: Lom zase pracuje.\n"
	.string "Mara tě čeká na stadionu.\p"
	.string "Pořádný souboj je bezpečnější\n"
	.string "než nepořádná elektroinstalace.$"
