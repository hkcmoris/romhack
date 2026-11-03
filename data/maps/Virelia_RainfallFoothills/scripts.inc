Virelia_RainfallFoothills_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, Virelia_RainfallFoothills_OnLoad
	.byte 0

Virelia_RainfallFoothills_OnLoad:
	callnative VireliaRainfall_BeginChapter
	callnative VireliaPrologue_SetRivalAppearance
	goto_if_set FLAG_VIRELIA_RAINFALL_RIVAL_COMPLETE, Virelia_RainfallFoothills_OpenCoast
	setmetatile 14, 28, 0x00E, 1
	end

Virelia_RainfallFoothills_OpenCoast:
	setmetatile 14, 28, 0x121, 0
	end

Virelia_RainfallFoothills_EventScript_CoastGate::
	msgbox Virelia_RainfallFoothills_Text_CoastGate, MSGBOX_SIGN
	end

Virelia_RainfallFoothills_EventScript_Rival::
	lock
	faceplayer
	goto_if_set FLAG_VIRELIA_RAINFALL_RIVAL_COMPLETE, Virelia_RainfallFoothills_RivalAfter
	msgbox Virelia_RainfallFoothills_Text_RivalIntro, MSGBOX_DEFAULT
	switch VAR_VIRELIA_STARTER
	case VIRELIA_STARTER_GRASS, Virelia_RainfallFoothills_RivalGrass
	case VIRELIA_STARTER_WATER, Virelia_RainfallFoothills_RivalWater
	case VIRELIA_STARTER_FIRE, Virelia_RainfallFoothills_RivalFire
	release
	end

Virelia_RainfallFoothills_RivalGrass:
	checkplayergender
	goto_if_eq VAR_RESULT, MALE, Virelia_RainfallFoothills_RivalGrassFemale
	trainerbattle_single TRAINER_VIRELIA_VIKTOR_RAINFALL_GRASS, Virelia_RainfallFoothills_Text_RivalChallenge, Virelia_RainfallFoothills_Text_RivalDefeat, Virelia_RainfallFoothills_RivalWon
	goto Virelia_RainfallFoothills_RivalWon

Virelia_RainfallFoothills_RivalGrassFemale:
	trainerbattle_single TRAINER_VIRELIA_VIKTORIE_RAINFALL_GRASS, Virelia_RainfallFoothills_Text_RivalChallenge, Virelia_RainfallFoothills_Text_RivalDefeat, Virelia_RainfallFoothills_RivalWon
	goto Virelia_RainfallFoothills_RivalWon

Virelia_RainfallFoothills_RivalWater:
	checkplayergender
	goto_if_eq VAR_RESULT, MALE, Virelia_RainfallFoothills_RivalWaterFemale
	trainerbattle_single TRAINER_VIRELIA_VIKTOR_RAINFALL_WATER, Virelia_RainfallFoothills_Text_RivalChallenge, Virelia_RainfallFoothills_Text_RivalDefeat, Virelia_RainfallFoothills_RivalWon
	goto Virelia_RainfallFoothills_RivalWon

Virelia_RainfallFoothills_RivalWaterFemale:
	trainerbattle_single TRAINER_VIRELIA_VIKTORIE_RAINFALL_WATER, Virelia_RainfallFoothills_Text_RivalChallenge, Virelia_RainfallFoothills_Text_RivalDefeat, Virelia_RainfallFoothills_RivalWon
	goto Virelia_RainfallFoothills_RivalWon

Virelia_RainfallFoothills_RivalFire:
	checkplayergender
	goto_if_eq VAR_RESULT, MALE, Virelia_RainfallFoothills_RivalFireFemale
	trainerbattle_single TRAINER_VIRELIA_VIKTOR_RAINFALL_FIRE, Virelia_RainfallFoothills_Text_RivalChallenge, Virelia_RainfallFoothills_Text_RivalDefeat, Virelia_RainfallFoothills_RivalWon
	goto Virelia_RainfallFoothills_RivalWon

Virelia_RainfallFoothills_RivalFireFemale:
	trainerbattle_single TRAINER_VIRELIA_VIKTORIE_RAINFALL_FIRE, Virelia_RainfallFoothills_Text_RivalChallenge, Virelia_RainfallFoothills_Text_RivalDefeat, Virelia_RainfallFoothills_RivalWon
	goto Virelia_RainfallFoothills_RivalWon

Virelia_RainfallFoothills_RivalWon::
	call VireliaRainfall_EventScript_FinishRival
	call Virelia_RainfallFoothills_OpenCoastTile
	special DrawWholeMapView
Virelia_RainfallFoothills_RivalAfter:
	msgbox Virelia_RainfallFoothills_Text_RivalAfter, MSGBOX_DEFAULT
	closemessage
	removeobject LOCALID_VIRELIA_RAINFALL_RIVAL
	release
	end

Virelia_RainfallFoothills_OpenCoastTile:
	setmetatile 14, 28, 0x121, 0
	return

Virelia_RainfallFoothills_Text_RivalIntro:
	.string "Takže už máš první odznak?\n"
	.string "Pěkné. Můj tým taky nezahálel.\p"
	.string "A než se zeptáš: trénink proběhl.\n"
	.string "Počítání kroků se nepočítá.$"

Virelia_RainfallFoothills_Text_RivalChallenge:
	.string "Ukaž, co tě Mara naučila!$"

Virelia_RainfallFoothills_Text_RivalDefeat:
	.string "Dobře. Ten odznak není náhoda.$"

Virelia_RainfallFoothills_Text_RivalAfter:
	.string "Dobrý souboj! V Seabreak prý\n"
	.string "bouřka vyřadila celý přístav.\p"
	.string "Cesty se tady dělí. Sejdeme se\n"
	.string "v přístavu. A usušíme boty.$"

Virelia_RainfallFoothills_Text_CoastGate:
	.string "Nejdřív promluv s rivalem\n"
	.string "u rozcestí. Čeká na váš souboj.$"

Virelia_RainfallFoothills_EventScript_Shortcut::
	lockall
	goto_if_unset FLAG_VIRELIA_LOWER_SHORTCUT, Virelia_RainfallFoothills_ShortcutClosed
	msgbox Virelia_RainfallFoothills_Text_Shortcut, MSGBOX_DEFAULT
	releaseall
	end

Virelia_RainfallFoothills_ShortcutClosed:
	msgbox Virelia_RainfallFoothills_Text_ShortcutClosed, MSGBOX_DEFAULT
	releaseall
	end

Virelia_RainfallFoothills_Text_Shortcut:
	.string "Lávka už je dole. Přes ni\n"
	.string "dojdeš rovnou do mokřin.$"

Virelia_RainfallFoothills_Text_ShortcutClosed:
	.string "Lávka je vytažená na druhý břeh.\n"
	.string "Z mokřin půjde spustit.$"

Virelia_RainfallFoothills_EventScript_Sign::
	msgbox Virelia_RainfallFoothills_Text_Sign, MSGBOX_SIGN
	end

Virelia_RainfallFoothills_Text_Sign:
	.string "Rainfall Pass - rozcestí\p"
	.string "Horní stezka: mosty a trenéři.\n"
	.string "Dolní stezka: mokřiny a potoky.\p"
	.string "Horní cesta míří k majáku.\n"
	.string "Dolní končí přímo u doků.$"

Virelia_RainfallFoothills_EventScript_Hiker::
	trainerbattle_single TRAINER_VIRELIA_RAINFALL_HIKER, Virelia_RainfallFoothills_Text_HikerIntro, Virelia_RainfallFoothills_Text_HikerDefeat
	msgbox Virelia_RainfallFoothills_Text_HikerAfter, MSGBOX_AUTOCLOSE
	end

Virelia_RainfallFoothills_Text_HikerIntro:
	.string "PAVEL: Kámen ještě drží sucho.\n"
	.string "Tak toho využijeme k souboji!$"

Virelia_RainfallFoothills_Text_HikerDefeat:
	.string "Teď už mám mokré i sebevědomí.$"

Virelia_RainfallFoothills_Text_HikerAfter:
	.string "Nahoře je to kratší, ale rušnější.\n"
	.string "Dole si cestu hledá voda i ty.$"
