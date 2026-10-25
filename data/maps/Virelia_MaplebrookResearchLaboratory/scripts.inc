Virelia_MaplebrookResearchLaboratory_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, Virelia_MaplebrookResearchLaboratory_OnLoad
#if VIRELIA_PROLOGUE && !VIRELIA_DEVELOPMENT_MODE && !IS_FRLG
	map_script MAP_SCRIPT_ON_TRANSITION, Virelia_MaplebrookResearchLaboratory_OnTransition
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Virelia_MaplebrookResearchLaboratory_OnFrame
#endif
	.byte 0

Virelia_MaplebrookResearchLaboratory_OnLoad:
	callnative VireliaPrologue_SetRivalAppearance
	callnative VireliaPrologue_SyncStarterBallFlags
	callnative VireliaPrologue_SyncRivalPokemon
#if VIRELIA_PROLOGUE && !VIRELIA_DEVELOPMENT_MODE && !IS_FRLG
	checkflag FLAG_VIRELIA_HIDE_RIVAL_POKEMON
	goto_if TRUE, Virelia_MaplebrookResearchLaboratory_EventScript_LoadDone
	setobjectxyperm LOCALID_VIRELIA_LAB_VIKTOR, 6, 8
	setobjectmovementtype LOCALID_VIRELIA_LAB_VIKTOR, MOVEMENT_TYPE_FACE_RIGHT
	setobjectxyperm LOCALID_VIRELIA_LAB_VIKTOR_POKEMON, 5, 8
	setobjectmovementtype LOCALID_VIRELIA_LAB_VIKTOR_POKEMON, MOVEMENT_TYPE_FACE_RIGHT
#endif
Virelia_MaplebrookResearchLaboratory_EventScript_LoadDone:
	end

Virelia_MaplebrookResearchLaboratory_OnTransition::
	setvar VAR_TEMP_0, 0
	checkflag FLAG_VIRELIA_MOM_MET
	goto_if FALSE, Virelia_MaplebrookResearchLaboratory_EventScript_TransitionDone
	checkflag FLAG_VIRELIA_PROFESSOR_MET
	goto_if TRUE, Virelia_MaplebrookResearchLaboratory_EventScript_TransitionDone
	setvar VAR_TEMP_0, 1
Virelia_MaplebrookResearchLaboratory_EventScript_TransitionDone:
	end

Virelia_MaplebrookResearchLaboratory_OnFrame:
	map_script_2 VAR_TEMP_0, 1, Virelia_MaplebrookResearchLaboratory_EventScript_Welcome
	.2byte 0

Virelia_MaplebrookResearchLaboratory_EventScript_Welcome:
	setvar VAR_TEMP_0, 0
	lockall
	hidefollower
	getplayerxy VAR_0x8006, VAR_0x8007
	goto_if_eq VAR_0x8006, 6, Virelia_MaplebrookResearchLaboratory_EventScript_EnterAisle
	applymovement LOCALID_PLAYER, Virelia_MaplebrookResearchLaboratory_Movement_Left1
	waitmovement LOCALID_PLAYER
Virelia_MaplebrookResearchLaboratory_EventScript_EnterAisle:
	applymovement LOCALID_PLAYER, Virelia_MaplebrookResearchLaboratory_Movement_EnterLab
	waitmovement LOCALID_PLAYER
	setflag FLAG_VIRELIA_PROFESSOR_MET
	msgbox Virelia_MaplebrookResearchLaboratory_Text_Introduction, MSGBOX_DEFAULT
	closemessage
	turnobject LOCALID_PLAYER, DIR_WEST
	applymovement LOCALID_VIRELIA_LAB_VIKTOR, Virelia_MaplebrookResearchLaboratory_Movement_GreetPlayer
	waitmovement LOCALID_VIRELIA_LAB_VIKTOR
	playse SE_BALL_OPEN
	playmoncry SPECIES_RATTATA, CRY_MODE_NORMAL
	waitmoncry
	playbgm MUS_HELP, TRUE
	applymovement LOCALID_VIRELIA_LAB_ASSISTANT, Virelia_MaplebrookResearchLaboratory_Movement_Right1
	waitmovement LOCALID_VIRELIA_LAB_ASSISTANT
	setobjectxyperm LOCALID_VIRELIA_LAB_ASSISTANT, 10, 9
	turnobject LOCALID_PLAYER, DIR_SOUTH
	turnobject LOCALID_VIRELIA_LAB_VIKTOR, DIR_SOUTH
	applymovement LOCALID_VIRELIA_LAB_VIKTOR, Virelia_MaplebrookResearchLaboratory_Movement_BlockExit
	waitmovement LOCALID_VIRELIA_LAB_VIKTOR
	applymovement LOCALID_VIRELIA_LAB_RATTATA, Virelia_MaplebrookResearchLaboratory_Movement_RattataEscape
	applymovement LOCALID_VIRELIA_LAB_MEOWTH, Virelia_MaplebrookResearchLaboratory_Movement_MeowthChase
	waitmovementall
	playmoncry SPECIES_MEOWTH, CRY_MODE_NORMAL
	waitmoncry
	msgbox Virelia_MaplebrookResearchLaboratory_Text_Escape, MSGBOX_DEFAULT
	msgbox Virelia_MaplebrookResearchLaboratory_Text_TakeCase, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_VIRELIA_LAB_VIKTOR, Virelia_MaplebrookResearchLaboratory_Movement_ReturnToCase
	waitmovement LOCALID_VIRELIA_LAB_VIKTOR
	applymovement LOCALID_PLAYER, Virelia_MaplebrookResearchLaboratory_Movement_ToCase
	waitmovement LOCALID_PLAYER
	playse SE_DOOR
	fadescreen FADE_TO_BLACK
Virelia_MaplebrookResearchLaboratory_EventScript_OpenCase:
	special ChooseVireliaStarter
	playbgm MUS_HELP, TRUE
	copyvar VAR_0x8004, VAR_RESULT
	call VireliaPrologue_EventScript_TryGiveStarter
	goto_if_eq VAR_RESULT, FALSE, Virelia_MaplebrookResearchLaboratory_EventScript_StarterFailed
	call Virelia_MaplebrookResearchLaboratory_EventScript_RemoveChosenStarter
	playfanfare MUS_OBTAIN_ITEM
	waitfanfare
	msgbox Virelia_MaplebrookResearchLaboratory_Text_ReceivedStarter, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_PLAYER, Virelia_MaplebrookResearchLaboratory_Movement_StepAsideFromCase
	waitmovement LOCALID_PLAYER
	goto Virelia_MaplebrookResearchLaboratory_EventScript_PickRivalStarter

Virelia_MaplebrookResearchLaboratory_EventScript_RemoveChosenStarter:
	callnative VireliaPrologue_SyncStarterBallFlags
	special DrawWholeMapView
	return

Virelia_MaplebrookResearchLaboratory_EventScript_PickRivalStarter:
	msgbox Virelia_MaplebrookResearchLaboratory_Text_RivalChoosing, MSGBOX_DEFAULT
	closemessage
	switch VAR_VIRELIA_STARTER
	case VIRELIA_STARTER_GRASS, Virelia_MaplebrookResearchLaboratory_EventScript_RivalTakesFire
	case VIRELIA_STARTER_WATER, Virelia_MaplebrookResearchLaboratory_EventScript_RivalTakesGrass
	case VIRELIA_STARTER_FIRE, Virelia_MaplebrookResearchLaboratory_EventScript_RivalTakesWater
	goto Virelia_MaplebrookResearchLaboratory_EventScript_EndScene

Virelia_MaplebrookResearchLaboratory_EventScript_RivalTakesFire:
	setflag FLAG_VIRELIA_HIDE_FIRE_STARTER
	goto Virelia_MaplebrookResearchLaboratory_EventScript_RivalTakesFromCase

Virelia_MaplebrookResearchLaboratory_EventScript_RivalTakesGrass:
	setflag FLAG_VIRELIA_HIDE_GRASS_STARTER
	goto Virelia_MaplebrookResearchLaboratory_EventScript_RivalTakesFromCase

Virelia_MaplebrookResearchLaboratory_EventScript_RivalTakesWater:
	setflag FLAG_VIRELIA_HIDE_WATER_STARTER
Virelia_MaplebrookResearchLaboratory_EventScript_RivalTakesFromCase:
	applymovement LOCALID_VIRELIA_LAB_VIKTOR, Virelia_MaplebrookResearchLaboratory_Movement_RivalToCase
	waitmovement LOCALID_VIRELIA_LAB_VIKTOR
	delay 16
	playse SE_BALL_OPEN
	callnative VireliaPrologue_SyncStarterBallFlags
	special DrawWholeMapView
	delay 16
	applymovement LOCALID_VIRELIA_LAB_VIKTOR, Virelia_MaplebrookResearchLaboratory_Movement_RivalFromCase
	waitmovement LOCALID_VIRELIA_LAB_VIKTOR
	@ Release the partner onto the case-front tile the rival just vacated.
	setobjectxyperm LOCALID_VIRELIA_LAB_VIKTOR_POKEMON, 6, 7
	callnative VireliaPrologue_SyncRivalPokemon
	addobject LOCALID_VIRELIA_LAB_VIKTOR_POKEMON
	playse SE_BALL_OPEN
	applymovement LOCALID_VIRELIA_LAB_VIKTOR, Virelia_MaplebrookResearchLaboratory_Movement_Down1
	waitmovement LOCALID_VIRELIA_LAB_VIKTOR
	applymovement LOCALID_VIRELIA_LAB_VIKTOR_POKEMON, Virelia_MaplebrookResearchLaboratory_Movement_Left1
	waitmovement LOCALID_VIRELIA_LAB_VIKTOR_POKEMON
	applymovement LOCALID_VIRELIA_LAB_VIKTOR, Virelia_MaplebrookResearchLaboratory_Movement_Right1
	waitmovement LOCALID_VIRELIA_LAB_VIKTOR
	applymovement LOCALID_VIRELIA_LAB_VIKTOR_POKEMON, Virelia_MaplebrookResearchLaboratory_Movement_Down1
	waitmovement LOCALID_VIRELIA_LAB_VIKTOR_POKEMON
	turnobject LOCALID_VIRELIA_LAB_VIKTOR, DIR_EAST
	turnobject LOCALID_VIRELIA_LAB_VIKTOR_POKEMON, DIR_EAST
	setobjectxyperm LOCALID_VIRELIA_LAB_VIKTOR, 6, 8
	setobjectmovementtype LOCALID_VIRELIA_LAB_VIKTOR, MOVEMENT_TYPE_FACE_RIGHT
	setobjectxyperm LOCALID_VIRELIA_LAB_VIKTOR_POKEMON, 5, 8
	setobjectmovementtype LOCALID_VIRELIA_LAB_VIKTOR_POKEMON, MOVEMENT_TYPE_FACE_RIGHT
	turnobject LOCALID_PLAYER, DIR_WEST
	goto Virelia_MaplebrookResearchLaboratory_EventScript_RescueBattle

Virelia_MaplebrookResearchLaboratory_EventScript_RescueBattle:
	turnobject LOCALID_PLAYER, DIR_SOUTH
	turnobject LOCALID_VIRELIA_LAB_VIKTOR, DIR_SOUTH
	msgbox Virelia_MaplebrookResearchLaboratory_Text_Together, MSGBOX_DEFAULT
	closemessage
	callnative VireliaPrologue_PrepareRescue
	goto_if_eq VAR_RESULT, FALSE, Virelia_MaplebrookResearchLaboratory_EventScript_StarterFailed
	special VireliaPrologue_StartRescueBattle
	callnative VireliaPrologue_FinishRescue
	goto_if_eq VAR_RESULT, FALSE, Virelia_MaplebrookResearchLaboratory_EventScript_TryRescueAgain
	removeobject LOCALID_VIRELIA_LAB_RATTATA
	removeobject LOCALID_VIRELIA_LAB_MEOWTH
	playfanfare MUS_HEAL
	waitfanfare
	msgbox Virelia_MaplebrookResearchLaboratory_Text_RescueHealed, MSGBOX_DEFAULT
	turnobject LOCALID_PLAYER, DIR_WEST
	turnobject LOCALID_VIRELIA_LAB_VIKTOR, DIR_EAST
	goto Virelia_MaplebrookResearchLaboratory_EventScript_RivalBattle

Virelia_MaplebrookResearchLaboratory_EventScript_TryRescueAgain:
	msgbox Virelia_MaplebrookResearchLaboratory_Text_RescueAgain, MSGBOX_DEFAULT
	goto Virelia_MaplebrookResearchLaboratory_EventScript_RescueBattle

Virelia_MaplebrookResearchLaboratory_EventScript_StarterFailed:
	msgbox Virelia_MaplebrookResearchLaboratory_Text_StarterFailed, MSGBOX_DEFAULT
	releaseall
	end

Virelia_MaplebrookResearchLaboratory_EventScript_EndScene:
	releaseall
	end

Virelia_MaplebrookResearchLaboratory_EventScript_Escapee::
	end

Virelia_MaplebrookResearchLaboratory_Movement_EnterLab:
	walk_up
	walk_up
	walk_up
	walk_up
	step_end

Virelia_MaplebrookResearchLaboratory_Movement_Left1:
	walk_left
	step_end

Virelia_MaplebrookResearchLaboratory_Movement_GreetPlayer:
Virelia_MaplebrookResearchLaboratory_Movement_Right1:
	walk_right
	step_end

Virelia_MaplebrookResearchLaboratory_Movement_ToCase:
	walk_up
	step_end

Virelia_MaplebrookResearchLaboratory_Movement_BlockExit:
	walk_down
	walk_down
	walk_down
	walk_right
	face_up
	step_end

Virelia_MaplebrookResearchLaboratory_Movement_ReturnToCase:
	walk_left
	walk_up
	walk_up
	walk_up
	face_right
	step_end

Virelia_MaplebrookResearchLaboratory_Movement_StepAsideFromCase:
	walk_down
	walk_right
	face_left
	step_end

Virelia_MaplebrookResearchLaboratory_Movement_RivalToCase:
	walk_up
	walk_right
	face_up
	step_end

Virelia_MaplebrookResearchLaboratory_Movement_RivalFromCase:
	walk_left
	step_end

Virelia_MaplebrookResearchLaboratory_Movement_Down1:
	walk_down
	step_end

@ Rattata keeps a two-step lead around the compact case and right aisle.
Virelia_MaplebrookResearchLaboratory_Movement_RattataEscape:
	walk_fast_right
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_left
	walk_fast_down
	walk_fast_down
	walk_fast_left
	walk_fast_left
	walk_fast_up
	face_up
	step_end

Virelia_MaplebrookResearchLaboratory_Movement_MeowthChase:
	walk_fast_down
	walk_fast_down
	walk_fast_right
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_left
	walk_fast_down
	walk_fast_left
	face_left
	step_end

Virelia_MaplebrookResearchLaboratory_EventScript_RivalBattle:
	checkplayergender
	goto_if_eq VAR_RESULT, MALE, Virelia_MaplebrookResearchLaboratory_EventScript_RivalIntroFemale
	msgbox Virelia_MaplebrookResearchLaboratory_Text_RivalIntro, MSGBOX_DEFAULT
	goto Virelia_MaplebrookResearchLaboratory_EventScript_ChooseRivalBattle
Virelia_MaplebrookResearchLaboratory_EventScript_RivalIntroFemale:
	msgbox Virelia_MaplebrookResearchLaboratory_Text_RivalIntroFemale, MSGBOX_DEFAULT
Virelia_MaplebrookResearchLaboratory_EventScript_ChooseRivalBattle:
	closemessage
	switch VAR_VIRELIA_STARTER
	case VIRELIA_STARTER_GRASS, Virelia_MaplebrookResearchLaboratory_EventScript_RivalGrass
	case VIRELIA_STARTER_WATER, Virelia_MaplebrookResearchLaboratory_EventScript_RivalWater
	case VIRELIA_STARTER_FIRE, Virelia_MaplebrookResearchLaboratory_EventScript_RivalFire
	goto Virelia_MaplebrookResearchLaboratory_EventScript_EndScene

Virelia_MaplebrookResearchLaboratory_EventScript_Viktor::
	lock
	faceplayer
#if VIRELIA_PROLOGUE && !VIRELIA_DEVELOPMENT_MODE && !IS_FRLG
	checkflag FLAG_VIRELIA_RIVAL_COMPLETE
	goto_if FALSE, Virelia_MaplebrookResearchLaboratory_EventScript_RivalWaiting
#endif
	msgbox Virelia_MaplebrookResearchLaboratory_Text_RivalAfter, MSGBOX_DEFAULT
	release
	end

Virelia_MaplebrookResearchLaboratory_EventScript_RivalWaiting:
	msgbox Virelia_MaplebrookResearchLaboratory_Text_RivalWaiting, MSGBOX_DEFAULT
	release
	end

Virelia_MaplebrookResearchLaboratory_EventScript_ViktorPokemon::
	lock
	faceplayer
	callnative VireliaPrologue_BufferRivalPokemon
	playmoncry VAR_0x8004, CRY_MODE_NORMAL
	waitmoncry
	release
	end

Virelia_MaplebrookResearchLaboratory_EventScript_RivalGrass:
	checkplayergender
	goto_if_eq VAR_RESULT, MALE, Virelia_MaplebrookResearchLaboratory_EventScript_RivalGrassFemale
	trainerbattle_earlyrival TRAINER_VIRELIA_VIKTOR_GRASS, RIVAL_BATTLE_HEAL_AFTER, Virelia_MaplebrookResearchLaboratory_Text_RivalLost, Virelia_MaplebrookResearchLaboratory_Text_RivalWon, LOCALID_VIRELIA_LAB_VIKTOR
	goto Virelia_MaplebrookResearchLaboratory_EventScript_AfterRival

Virelia_MaplebrookResearchLaboratory_EventScript_RivalWater:
	checkplayergender
	goto_if_eq VAR_RESULT, MALE, Virelia_MaplebrookResearchLaboratory_EventScript_RivalWaterFemale
	trainerbattle_earlyrival TRAINER_VIRELIA_VIKTOR_WATER, RIVAL_BATTLE_HEAL_AFTER, Virelia_MaplebrookResearchLaboratory_Text_RivalLost, Virelia_MaplebrookResearchLaboratory_Text_RivalWon, LOCALID_VIRELIA_LAB_VIKTOR
	goto Virelia_MaplebrookResearchLaboratory_EventScript_AfterRival

Virelia_MaplebrookResearchLaboratory_EventScript_RivalFire:
	checkplayergender
	goto_if_eq VAR_RESULT, MALE, Virelia_MaplebrookResearchLaboratory_EventScript_RivalFireFemale
	trainerbattle_earlyrival TRAINER_VIRELIA_VIKTOR_FIRE, RIVAL_BATTLE_HEAL_AFTER, Virelia_MaplebrookResearchLaboratory_Text_RivalLost, Virelia_MaplebrookResearchLaboratory_Text_RivalWon, LOCALID_VIRELIA_LAB_VIKTOR
	goto Virelia_MaplebrookResearchLaboratory_EventScript_AfterRival

Virelia_MaplebrookResearchLaboratory_EventScript_RivalGrassFemale:
	trainerbattle_earlyrival TRAINER_VIRELIA_VIKTORIE_GRASS, RIVAL_BATTLE_HEAL_AFTER, Virelia_MaplebrookResearchLaboratory_Text_RivalLostFemale, Virelia_MaplebrookResearchLaboratory_Text_RivalWon, LOCALID_VIRELIA_LAB_VIKTOR
	goto Virelia_MaplebrookResearchLaboratory_EventScript_AfterRival

Virelia_MaplebrookResearchLaboratory_EventScript_RivalWaterFemale:
	trainerbattle_earlyrival TRAINER_VIRELIA_VIKTORIE_WATER, RIVAL_BATTLE_HEAL_AFTER, Virelia_MaplebrookResearchLaboratory_Text_RivalLostFemale, Virelia_MaplebrookResearchLaboratory_Text_RivalWon, LOCALID_VIRELIA_LAB_VIKTOR
	goto Virelia_MaplebrookResearchLaboratory_EventScript_AfterRival

Virelia_MaplebrookResearchLaboratory_EventScript_RivalFireFemale:
	trainerbattle_earlyrival TRAINER_VIRELIA_VIKTORIE_FIRE, RIVAL_BATTLE_HEAL_AFTER, Virelia_MaplebrookResearchLaboratory_Text_RivalLostFemale, Virelia_MaplebrookResearchLaboratory_Text_RivalWon, LOCALID_VIRELIA_LAB_VIKTOR
	goto Virelia_MaplebrookResearchLaboratory_EventScript_AfterRival

Virelia_MaplebrookResearchLaboratory_EventScript_AfterRival:
	callnative VireliaPrologue_FinishRival
	msgbox Virelia_MaplebrookResearchLaboratory_Text_RivalAfter, MSGBOX_DEFAULT
	turnobject LOCALID_PLAYER, DIR_NORTH
	msgbox Virelia_MaplebrookResearchLaboratory_Text_Nickname, MSGBOX_YESNO
	goto_if_ne VAR_RESULT, YES, Virelia_MaplebrookResearchLaboratory_EventScript_Supplies
	closemessage
	setvar VAR_0x8004, 0
	call Common_EventScript_NameReceivedPartyMon
	goto Virelia_MaplebrookResearchLaboratory_EventScript_Supplies

Virelia_MaplebrookResearchLaboratory_EventScript_Professor::
	lock
	faceplayer
#if VIRELIA_PROLOGUE && !VIRELIA_DEVELOPMENT_MODE && !IS_FRLG
	checkflag FLAG_VIRELIA_MOM_MET
	goto_if FALSE, Virelia_MaplebrookResearchLaboratory_EventScript_SeeMom
	checkflag FLAG_VIRELIA_STARTER_RECEIVED
	goto_if FALSE, Virelia_MaplebrookResearchLaboratory_EventScript_Introduction
	checkflag FLAG_VIRELIA_RIVAL_COMPLETE
	goto_if FALSE, Virelia_MaplebrookResearchLaboratory_EventScript_SeeViktor
	checkflag FLAG_VIRELIA_SUPPLIES_RECEIVED
	goto_if FALSE, Virelia_MaplebrookResearchLaboratory_EventScript_Supplies
#endif
	msgbox Virelia_MaplebrookResearchLaboratory_Text_Heal, MSGBOX_DEFAULT
	special HealPlayerParty
	callnative UpdateFollowingPokemon
	playfanfare MUS_HEAL
	waitfanfare
	msgbox Virelia_MaplebrookResearchLaboratory_Text_Healed, MSGBOX_DEFAULT
	release
	end

Virelia_MaplebrookResearchLaboratory_EventScript_SeeMom:
	msgbox Virelia_MaplebrookResearchLaboratory_Text_SeeMom, MSGBOX_DEFAULT
	release
	end

Virelia_MaplebrookResearchLaboratory_EventScript_Introduction:
	setflag FLAG_VIRELIA_PROFESSOR_MET
	msgbox Virelia_MaplebrookResearchLaboratory_Text_Introduction, MSGBOX_DEFAULT
	release
	end

Virelia_MaplebrookResearchLaboratory_EventScript_SeeViktor:
	msgbox Virelia_MaplebrookResearchLaboratory_Text_SeeViktor, MSGBOX_DEFAULT
	release
	end

Virelia_MaplebrookResearchLaboratory_EventScript_Supplies:
	call VireliaPrologue_EventScript_TryGiveSupplies
	goto_if_eq VAR_RESULT, FALSE, Virelia_MaplebrookResearchLaboratory_EventScript_BagFull
	msgbox Virelia_MaplebrookResearchLaboratory_Text_Supplies, MSGBOX_DEFAULT
	playfanfare MUS_OBTAIN_ITEM
	waitfanfare
	msgbox Virelia_MaplebrookResearchLaboratory_Text_Journey, MSGBOX_DEFAULT
	releaseall
	end

Virelia_MaplebrookResearchLaboratory_EventScript_BagFull:
	msgbox Virelia_MaplebrookResearchLaboratory_Text_BagFull, MSGBOX_DEFAULT
	releaseall
	end

Virelia_MaplebrookResearchLaboratory_EventScript_Assistant::
	msgbox Virelia_MaplebrookResearchLaboratory_Text_Assistant, MSGBOX_NPC
	end

Virelia_MaplebrookResearchLaboratory_EventScript_StarterCase::
	lockall
#if VIRELIA_PROLOGUE && !VIRELIA_DEVELOPMENT_MODE && !IS_FRLG
	msgbox Virelia_MaplebrookResearchLaboratory_Text_AlreadyChosen, MSGBOX_DEFAULT
#else
	fadescreen FADE_TO_BLACK
	special ChooseVireliaStarter
	msgbox Virelia_MaplebrookResearchLaboratory_Text_Exhibition, MSGBOX_DEFAULT
#endif
	releaseall
	end

Virelia_MaplebrookResearchLaboratory_Text_Introduction:
	.string "JAVOROVÁ: Vítej, {PLAYER}.\n"
	.string "Právě dorazil nový vzorek.$"

Virelia_MaplebrookResearchLaboratory_Text_SeeMom:
	.string "Nejdřív se ozvi doma mámě.\n"
	.string "Ať nemusí hledat i tebe.$"

Virelia_MaplebrookResearchLaboratory_Text_Heal:
	.string "JAVOROVÁ: Odpočiňte si tu.\n"
	.string "Výzkum má rád živé účastníky.$"

Virelia_MaplebrookResearchLaboratory_Text_Healed:
	.string "Tví Pokémoni jsou plní sil.\n"
	.string "Whisperwood Trail čeká na severu.$"

Virelia_MaplebrookResearchLaboratory_Text_Assistant:
	.string "Tým otevřeš v menu POKéMON.\n"
	.string "Předměty najdeš v BATOHU.\p"
	.string "Hru ukládej přes ULOŽIT.\n"
	.string "Máma i profesorka tým uzdraví.$"

Virelia_MaplebrookResearchLaboratory_Text_GrassStarter:
	.string "POOCHYENA Z VIRELIE\n"
	.string "TRÁVA / TEMNOTA\p"
	.string "Hlídá si své lidi i svůj kout.\n"
	.string "Se stíny si zatím jen hraje.$"

Virelia_MaplebrookResearchLaboratory_Text_WaterStarter:
	.string "NATU Z VIRELIE\n"
	.string "VODA / PSYCHIKA\p"
	.string "Poskakuje podél vodní hladiny.\n"
	.string "Jeho peří se třpytí kapkami.$"

Virelia_MaplebrookResearchLaboratory_Text_FireStarter:
	.string "EKANS Z VIRELIE\n"
	.string "OHEŇ / JED\p"
	.string "Je zvídavý a příjemně hřeje.\n"
	.string "Na nošení v kapse ale není.$"

Virelia_MaplebrookResearchLaboratory_Text_ConfirmStarter:
	.string "Chceš tohoto Pokémona\n"
	.string "za svého prvního parťáka?$"

Virelia_MaplebrookResearchLaboratory_Text_CancelChoice:
	.string "Žádný spěch. Prohlédni si\n"
	.string "i ostatní dva Pokémony.$"

Virelia_MaplebrookResearchLaboratory_Text_ReceivedStarter:
	.string "Pokémon se přidal do tvého týmu!$"

Virelia_MaplebrookResearchLaboratory_Text_AlreadyChosen:
	.string "Zbývající parťák tu počká\n"
	.string "na dalšího trenéra.$"

Virelia_MaplebrookResearchLaboratory_Text_StarterFailed:
	.string "JAVOROVÁ: Nejdřív si musíme\n"
	.string "ujasnit, kdo už je ve tvém týmu.$"

Virelia_MaplebrookResearchLaboratory_Text_Exhibition:
	.string "Výzkumná expozice. Výběr patří\n"
	.string "do příběhového začátku hry.$"

Virelia_MaplebrookResearchLaboratory_Text_SeeViktor:
	.string "{RIVAL} čeká u východu.\n"
	.string "Zkuste souboj. Pak vás uzdravím.$"

Virelia_MaplebrookResearchLaboratory_Text_RivalWaiting:
	.string "{RIVAL}: Hej v klidu si vyber.\n"
	.string "Já zkusím takovou taktiku.$"

Virelia_MaplebrookResearchLaboratory_Text_RivalChoosing:
	.string "{RIVAL}: Teď jsem na řadě já.\n"
	.string "Už vím, kdo půjde se mnou.$"

Virelia_MaplebrookResearchLaboratory_Text_RivalIntro:
	.string "{RIVAL}: Spolu nám to šlo.\n"
	.string "A teď proti sobě!\p"
	.string "Typovou výhodu jsem si nechal.\n"
	.string "Pro jistotu. Věřím totiž v sebe.\p"
	.string "JAVOROVÁ: Jen cvičně. A prosím\n"
	.string "dál od přístrojů.$"

Virelia_MaplebrookResearchLaboratory_Text_RivalIntroFemale:
	.string "{RIVAL}: Spolu nám to šlo.\n"
	.string "A teď proti sobě!\p"
	.string "Typovou výhodu jsem si nechala.\n"
	.string "Pro jistotu. Věřím totiž v sebe.\p"
	.string "JAVOROVÁ: Jen cvičně. A prosím\n"
	.string "dál od přístrojů.$"

Virelia_MaplebrookResearchLaboratory_Text_RivalLost:
	.string "Dobre hej, jsem tě nechal.$"

Virelia_MaplebrookResearchLaboratory_Text_RivalLostFemale:
	.string "Dobre hej, jsem tě nechala.$"

Virelia_MaplebrookResearchLaboratory_Text_RivalWon:
	.string "Vyšlo to! Ale příště se\n"
	.string "klidně může karta obrátit.$"

Virelia_MaplebrookResearchLaboratory_Text_RivalAfter:
	.string "{RIVAL}: To bylo jen zahřátí.\n"
	.string "Příště už nebudu tolik mluvit.\p"
	.string "JAVOROVÁ: Oba týmy jsou v pořádku.\n"
	.string "Výsledek není přijímací zkouška.$"

Virelia_MaplebrookResearchLaboratory_Text_Nickname:
	.string "JAVOROVÁ: Teď už se trochu znáte.\n"
	.string "Chceš dát parťákovi jméno?$"

Virelia_MaplebrookResearchLaboratory_Text_Supplies:
	.string "JAVOROVÁ: Tady máš POKéDEX,\n"
	.string "5 Poké Ballů a 3 Potiony.\p"
	.string "Divokého Pokémona nejdřív oslab\n"
	.string "a pak ho zkus chytit Ballem.\p"
	.string "Pokédex zaznamená tvoje objevy.\n"
	.string "Cizí Pokémony ale nech trenérům.$"

Virelia_MaplebrookResearchLaboratory_Text_Journey:
	.string "Po Whisperwood Trail dojdeš\n"
	.string "na sever do Slateford City.\p"
	.string "Lidé z lomu hlásí podivné věci.\n"
	.string "Nejdřív se tam porozhlédni.$"

Virelia_MaplebrookResearchLaboratory_Text_BagFull:
	.string "V batohu není místo na výbavu.\n"
	.string "Uvolni ho a vrať se pro ni.$"

Virelia_MaplebrookResearchLaboratory_Text_Escape:
	.string "JAVOROVÁ: Rattata utekla!\n"
	.string "A Meowth ji honí!\p"
	.string "{RIVAL}: Hej, dveře hlídám!\n"
	.string "Co budeme dělat s tím zbytkem?$"

Virelia_MaplebrookResearchLaboratory_Text_TakeCase:
	.string "JAVOROVÁ: V tom kufříku\n"
	.string "jsou Pokémoni!\p"
	.string "Vy dva, vezměte si každý jednoho.\n"
	.string "Pomozte nám je uklidnit!$"

Virelia_MaplebrookResearchLaboratory_Text_Together:
	.string "{RIVAL}: Tentokrát spolu.\n"
	.string "Jdeme na ně!$"

Virelia_MaplebrookResearchLaboratory_Text_RescueHealed:
	.string "JAVOROVÁ: Oba jsou v bezpečí.\n"
	.string "A vaši parťáci jsou plní sil.\p"
	.string "První společný problém máte\n"
	.string "za sebou. Pokémoni vám zůstanou.\p"
	.string "Přizpůsobili se životu ve Virelii.\n"
	.string "Proto mají jiné podoby i typy.$"

Virelia_MaplebrookResearchLaboratory_Text_RescueAgain:
	.string "JAVOROVÁ: Ještě pobíhají po labu!\n"
	.string "Zkusme je znovu uklidnit.$"
