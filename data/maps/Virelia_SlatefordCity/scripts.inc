Virelia_SlatefordCity_MapScripts::
#if VIRELIA_PROLOGUE && !VIRELIA_DEVELOPMENT_MODE && !IS_FRLG
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Virelia_SlatefordCity_OnFrame
#endif
	.byte 0

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
	switch VAR_0x8008
	case DIR_NORTH, Virelia_SlatefordCity_EventScript_RocketUp
	case DIR_SOUTH, Virelia_SlatefordCity_EventScript_RocketDown
	case DIR_WEST, Virelia_SlatefordCity_EventScript_RocketLeft
	applymovement LOCALID_VIRELIA_SLATEFORD_ROCKET, Virelia_SlatefordCity_Movement_RocketRight
	goto Virelia_SlatefordCity_EventScript_RocketGone

Virelia_SlatefordCity_EventScript_RocketUp:
	applymovement LOCALID_VIRELIA_SLATEFORD_ROCKET, Virelia_SlatefordCity_Movement_RocketUp
	goto Virelia_SlatefordCity_EventScript_RocketGone

Virelia_SlatefordCity_EventScript_RocketDown:
	applymovement LOCALID_VIRELIA_SLATEFORD_ROCKET, Virelia_SlatefordCity_Movement_RocketDown
	goto Virelia_SlatefordCity_EventScript_RocketGone

Virelia_SlatefordCity_EventScript_RocketLeft:
	applymovement LOCALID_VIRELIA_SLATEFORD_ROCKET, Virelia_SlatefordCity_Movement_RocketLeft

Virelia_SlatefordCity_EventScript_RocketGone:
	waitmovement 0
	removeobject LOCALID_VIRELIA_SLATEFORD_ROCKET
	goto Virelia_SlatefordCity_EventScript_ResolveRocket

Virelia_SlatefordCity_EventScript_ResolveRocket::
#if VIRELIA_PROLOGUE && !VIRELIA_DEVELOPMENT_MODE && !IS_FRLG
	lockall
	clearflag FLAG_VIRELIA_HIDE_POLICEMAN
	addobject LOCALID_VIRELIA_SLATEFORD_POLICEMAN
	applymovement LOCALID_VIRELIA_SLATEFORD_POLICEMAN, Virelia_SlatefordCity_Movement_Policeman
	waitmovement 0
	msgbox Virelia_SlatefordCity_Text_Policeman, MSGBOX_DEFAULT
	callnative VireliaPrologue_Complete
	msgbox Virelia_SlatefordCity_Text_PrologueComplete, MSGBOX_DEFAULT
	releaseall
#endif
	end

Virelia_SlatefordCity_EventScript_Policeman::
	msgbox Virelia_SlatefordCity_Text_PoliceAfter, MSGBOX_NPC
	end

Virelia_SlatefordCity_Movement_RocketUp:
	walk_fast_up
	walk_fast_up
	step_end

Virelia_SlatefordCity_Movement_RocketDown:
	walk_fast_down
	walk_fast_down
	step_end

Virelia_SlatefordCity_Movement_RocketLeft:
	walk_fast_left
	walk_fast_left
	step_end

Virelia_SlatefordCity_Movement_RocketRight:
	walk_fast_right
	walk_fast_right
	step_end

Virelia_SlatefordCity_Movement_Policeman:
	walk_fast_up
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
	.string "Tohle město vyrostlo z kamene,\n"
	.string "ale domov z něj dělají lidé.$"

Virelia_SlatefordCity_Text_MinerReport:
	.string "HORNÍK: Zmizely nám vzorky\n"
	.string "vodivých minerálů z lomu.\p"
	.string "Někdo se vydává za údržbáře\n"
	.string "a ptá se na další zásilku.\p"
	.string "Stojí u cesty k lomu. Poznáš ho.\n"
	.string "Má čisté boty. Tady podezřelé.$"

Virelia_SlatefordCity_Text_MinerAfter:
	.string "Horník: Za vzorek díky. Až se\n"
	.string "lom otevře, zjistíme víc.\p"
	.string "Stadion je zatím zavřený.\n"
	.string "Jedna výprava za druhou, dobře?$"

Virelia_SlatefordCity_Text_Resident:
	.string "Maplebrook Town leží jižně,\n"
	.string "za Whisperwood Trail.\p"
	.string "Máma i profesorka tam tvým\n"
	.string "Pokémonům dopřejí odpočinek.$"

Virelia_SlatefordCity_Text_WorkerBusy:
	.string "ÚDRŽBÁŘ: Mám práci. Zeptej se\n"
	.string "horníka u lomu, ten ví všechno.$"

Virelia_SlatefordCity_Text_Maintenance:
	.string "ÚDRŽBÁŘ: Probíhá průzkum lomu.\n"
	.string "Vstup zatím není povolený.$"

Virelia_SlatefordCity_Text_Worker:
	.string "ÚDRŽBÁŘ: Ukradené minerály?\n"
	.string "Já jen kontroluji jejich odvoz.\p"
	.string "To R na rukávu? Revize.\n"
	.string "Velmi důkladná revize…$"

Virelia_SlatefordCity_Text_RocketIntro:
	.string "POSKOK: Dobře. Tým Rocket.\n"
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
	.string "Ten štítek si zaslouží prověřit.$"

Virelia_SlatefordCity_Text_PrologueComplete:
	.string "První výprava je za tebou!\p"
	.string "Další stopa vede do lomu.\n"
	.string "Potom přijde na řadu stadion.\p"
	.string "Lom teď hlídá policie. Než ho\n"
	.string "otevřou, prozkoumej okolí.$"

Virelia_SlatefordCity_Text_PoliceAfter:
	.string "POLICISTA: Vyšetřování běží.\n"
	.string "Papíry bohužel rychlejší nejsou.$"

Virelia_SlatefordCity_Text_Stone:
	.string "Označený vzorek z lomu.\n"
	.string "Nech ho prosím pracovníkům.$"

Virelia_SlatefordCity_Text_GymSign:
	.string "STADION - Slateford City\n"
	.string "Chystá se na první trenéry.$"

Virelia_SlatefordCity_Text_CitySign:
	.string "Slateford City\n"
	.string "Pevné kořeny v kameni.$"

Virelia_SlatefordCity_Text_QuarrySign:
	.string "LOM - Slateford City\n"
	.string "Do ukončení průzkumu uzavřeno.$"

Virelia_SlatefordCity_Text_OfficeSign:
	.string "DŮLNÍ SPRÁVA\n"
	.string "Brzy otevřeme.$"

Virelia_SlatefordCity_Text_ClosedGym:
	.string "Stadion ještě není otevřený.\n"
	.string "První výzva teprve přijde.$"

Virelia_SlatefordCity_Text_ClosedHouse:
	.string "Dveře jsou zamčené.\n"
	.string "Obyvatelé jsou pryč.$"

Virelia_SlatefordCity_Text_ClosedOffice:
	.string "Důlní správa se připravuje\n"
	.string "na příchod nového týmu.$"
