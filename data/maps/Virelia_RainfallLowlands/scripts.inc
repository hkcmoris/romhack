Virelia_RainfallLowlands_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, Virelia_RainfallLowlands_OnLoad
	.byte 0

Virelia_RainfallLowlands_OnLoad:
	callnative VireliaPrologue_SetRivalAppearance
	goto_if_set FLAG_VIRELIA_RAINFALL_RIVAL_COMPLETE, Virelia_RainfallLowlands_OpenCoast
	setmetatile 15, 13, 0x00E, 1
	end

Virelia_RainfallLowlands_OpenCoast:
	setmetatile 15, 13, 0x121, 0
	end

Virelia_RainfallLowlands_EventScript_CoastGate::
	msgbox Virelia_RainfallLowlands_Text_CoastGate, MSGBOX_SIGN
	end

Virelia_RainfallLowlands_EventScript_Rival::
	lock
	faceplayer
	goto_if_set FLAG_VIRELIA_RAINFALL_RIVAL_COMPLETE, Virelia_RainfallLowlands_RivalAfter
	msgbox Virelia_RainfallLowlands_Text_RivalIntro, MSGBOX_DEFAULT
	switch VAR_VIRELIA_STARTER
	case VIRELIA_STARTER_GRASS, Virelia_RainfallLowlands_RivalGrass
	case VIRELIA_STARTER_WATER, Virelia_RainfallLowlands_RivalWater
	case VIRELIA_STARTER_FIRE, Virelia_RainfallLowlands_RivalFire
	release
	end

Virelia_RainfallLowlands_RivalGrass:
	checkplayergender
	goto_if_eq VAR_RESULT, MALE, Virelia_RainfallLowlands_RivalGrassFemale
	trainerbattle_single TRAINER_VIRELIA_VIKTOR_RAINFALL_GRASS, Virelia_RainfallLowlands_Text_RivalChallenge, Virelia_RainfallLowlands_Text_RivalDefeat, Virelia_RainfallLowlands_RivalWon
	goto Virelia_RainfallLowlands_RivalWon

Virelia_RainfallLowlands_RivalGrassFemale:
	trainerbattle_single TRAINER_VIRELIA_VIKTORIE_RAINFALL_GRASS, Virelia_RainfallLowlands_Text_RivalChallenge, Virelia_RainfallLowlands_Text_RivalDefeat, Virelia_RainfallLowlands_RivalWon
	goto Virelia_RainfallLowlands_RivalWon

Virelia_RainfallLowlands_RivalWater:
	checkplayergender
	goto_if_eq VAR_RESULT, MALE, Virelia_RainfallLowlands_RivalWaterFemale
	trainerbattle_single TRAINER_VIRELIA_VIKTOR_RAINFALL_WATER, Virelia_RainfallLowlands_Text_RivalChallenge, Virelia_RainfallLowlands_Text_RivalDefeat, Virelia_RainfallLowlands_RivalWon
	goto Virelia_RainfallLowlands_RivalWon

Virelia_RainfallLowlands_RivalWaterFemale:
	trainerbattle_single TRAINER_VIRELIA_VIKTORIE_RAINFALL_WATER, Virelia_RainfallLowlands_Text_RivalChallenge, Virelia_RainfallLowlands_Text_RivalDefeat, Virelia_RainfallLowlands_RivalWon
	goto Virelia_RainfallLowlands_RivalWon

Virelia_RainfallLowlands_RivalFire:
	checkplayergender
	goto_if_eq VAR_RESULT, MALE, Virelia_RainfallLowlands_RivalFireFemale
	trainerbattle_single TRAINER_VIRELIA_VIKTOR_RAINFALL_FIRE, Virelia_RainfallLowlands_Text_RivalChallenge, Virelia_RainfallLowlands_Text_RivalDefeat, Virelia_RainfallLowlands_RivalWon
	goto Virelia_RainfallLowlands_RivalWon

Virelia_RainfallLowlands_RivalFireFemale:
	trainerbattle_single TRAINER_VIRELIA_VIKTORIE_RAINFALL_FIRE, Virelia_RainfallLowlands_Text_RivalChallenge, Virelia_RainfallLowlands_Text_RivalDefeat, Virelia_RainfallLowlands_RivalWon
	goto Virelia_RainfallLowlands_RivalWon

Virelia_RainfallLowlands_RivalWon::
	call VireliaRainfall_EventScript_FinishRival
	call Virelia_RainfallLowlands_OpenCoastTile
	special DrawWholeMapView
Virelia_RainfallLowlands_RivalAfter:
	msgbox Virelia_RainfallLowlands_Text_RivalAfter, MSGBOX_DEFAULT
	closemessage
	removeobject LOCALID_VIRELIA_RAINFALL_RIVAL
	release
	end

Virelia_RainfallLowlands_OpenCoastTile:
	setmetatile 15, 13, 0x121, 0
	return

Virelia_RainfallLowlands_Text_RivalIntro:
	.string "Takže už máš první odznak?\n"
	.string "Pěkné. Můj tým taky nezahálel.\p"
	.string "A než se zeptáš: trénink proběhl.\n"
	.string "Počítání kroků se nepočítá.$"

Virelia_RainfallLowlands_Text_RivalChallenge:
	.string "Ukaž, co tě Mara naučila!$"

Virelia_RainfallLowlands_Text_RivalDefeat:
	.string "Dobře. Ten odznak není náhoda.$"

Virelia_RainfallLowlands_Text_RivalAfter:
	.string "Dobrý souboj! V Seabreak prý\n"
	.string "bouřka vyřadila celý přístav.\p"
	.string "Uvolním ti sestup. Tam dole se\n"
	.string "potkáme znovu. A usušíme boty.$"

Virelia_RainfallLowlands_Text_CoastGate:
	.string "Nejdřív promluv s rivalem\n"
	.string "u sestupu. Čeká na váš souboj.$"

Virelia_RainfallLowlands_EventScript_Weather::
	trainerbattle_single TRAINER_VIRELIA_RAINFALL_WEATHER, Virelia_RainfallLowlands_Text_WeatherIntro, Virelia_RainfallLowlands_Text_WeatherDefeat
	msgbox Virelia_RainfallLowlands_Text_WeatherAfter, MSGBOX_AUTOCLOSE
	end

Virelia_RainfallLowlands_Text_WeatherIntro:
	.string "MILAN: Pod útesem déšť ustává.\n"
	.string "Moje předpověď? Napínavý souboj!$"

Virelia_RainfallLowlands_Text_WeatherDefeat:
	.string "Tohle mi v měření chybělo.$"

Virelia_RainfallLowlands_Text_WeatherAfter:
	.string "Prší víc, než místní pamatují.\n"
	.string "Zatím nevíme proč. Měřit dál!$"
