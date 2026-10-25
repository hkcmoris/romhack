Virelia_MaplebrookResearchLaboratory_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, Virelia_MaplebrookResearchLaboratory_OnLoad
#if VIRELIA_PROLOGUE && !VIRELIA_DEVELOPMENT_MODE && !IS_FRLG
	map_script MAP_SCRIPT_ON_TRANSITION, Virelia_MaplebrookResearchLaboratory_OnTransition
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Virelia_MaplebrookResearchLaboratory_OnFrame
#endif
	.byte 0

Virelia_MaplebrookResearchLaboratory_OnLoad:
	callnative VireliaPrologue_SyncStarterBallFlags
	callnative VireliaPrologue_SyncRivalPokemon
#if VIRELIA_PROLOGUE && !VIRELIA_DEVELOPMENT_MODE && !IS_FRLG
	checkflag FLAG_VIRELIA_HIDE_RIVAL_POKEMON
	goto_if FALSE, Virelia_MaplebrookResearchLaboratory_EventScript_LoadRivalPokemon
	checkflag FLAG_VIRELIA_PROFESSOR_MET
	goto_if FALSE, Virelia_MaplebrookResearchLaboratory_EventScript_LoadDone
	checkflag FLAG_VIRELIA_RIVAL_COMPLETE
	goto_if TRUE, Virelia_MaplebrookResearchLaboratory_EventScript_LoadDone
	setobjectxyperm LOCALID_VIRELIA_LAB_VIKTOR, 5, 8
#endif
Virelia_MaplebrookResearchLaboratory_EventScript_LoadDone:
	end

@ Restore the pair together on re-entry; each starter scene ends in these tiles.
Virelia_MaplebrookResearchLaboratory_EventScript_LoadRivalPokemon:
	switch VAR_VIRELIA_STARTER
	case VIRELIA_STARTER_GRASS, Virelia_MaplebrookResearchLaboratory_EventScript_LoadFirePokemon
	case VIRELIA_STARTER_WATER, Virelia_MaplebrookResearchLaboratory_EventScript_LoadGrassPokemon
	case VIRELIA_STARTER_FIRE, Virelia_MaplebrookResearchLaboratory_EventScript_LoadWaterPokemon
	end

Virelia_MaplebrookResearchLaboratory_EventScript_LoadFirePokemon:
	setobjectxyperm LOCALID_VIRELIA_LAB_VIKTOR, 5, 8
	setobjectmovementtype LOCALID_VIRELIA_LAB_VIKTOR, MOVEMENT_TYPE_FACE_LEFT
	setobjectxyperm LOCALID_VIRELIA_LAB_VIKTOR_POKEMON, 6, 8
	setobjectmovementtype LOCALID_VIRELIA_LAB_VIKTOR_POKEMON, MOVEMENT_TYPE_FACE_LEFT
	end

Virelia_MaplebrookResearchLaboratory_EventScript_LoadGrassPokemon:
	setobjectxyperm LOCALID_VIRELIA_LAB_VIKTOR, 5, 8
	setobjectmovementtype LOCALID_VIRELIA_LAB_VIKTOR, MOVEMENT_TYPE_FACE_RIGHT
	setobjectxyperm LOCALID_VIRELIA_LAB_VIKTOR_POKEMON, 4, 8
	setobjectmovementtype LOCALID_VIRELIA_LAB_VIKTOR_POKEMON, MOVEMENT_TYPE_FACE_RIGHT
	end

Virelia_MaplebrookResearchLaboratory_EventScript_LoadWaterPokemon:
	setobjectxyperm LOCALID_VIRELIA_LAB_VIKTOR, 7, 8
	setobjectmovementtype LOCALID_VIRELIA_LAB_VIKTOR, MOVEMENT_TYPE_FACE_RIGHT
	setobjectxyperm LOCALID_VIRELIA_LAB_VIKTOR_POKEMON, 6, 8
	setobjectmovementtype LOCALID_VIRELIA_LAB_VIKTOR_POKEMON, MOVEMENT_TYPE_FACE_RIGHT
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
	turnobject LOCALID_VIRELIA_LAB_PROFESSOR, DIR_SOUTH
	msgbox Virelia_MaplebrookResearchLaboratory_Text_Introduction, MSGBOX_DEFAULT
	closemessage
	turnobject LOCALID_PLAYER, DIR_WEST
	delay 16
	applymovement LOCALID_VIRELIA_LAB_VIKTOR, Virelia_MaplebrookResearchLaboratory_Movement_GreetPlayer
	waitmovement LOCALID_VIRELIA_LAB_VIKTOR
	msgbox Virelia_MaplebrookResearchLaboratory_Text_RivalWaiting, MSGBOX_DEFAULT
	closemessage
	setobjectxyperm LOCALID_VIRELIA_LAB_VIKTOR, 5, 8
	setobjectmovementtype LOCALID_VIRELIA_LAB_VIKTOR, MOVEMENT_TYPE_FACE_RIGHT
	setflag FLAG_VIRELIA_PROFESSOR_MET
	releaseall
	end

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

Virelia_MaplebrookResearchLaboratory_EventScript_GrassStarter::
	lockall
	faceplayer
	setvar VAR_0x8004, VIRELIA_STARTER_GRASS
	goto Virelia_MaplebrookResearchLaboratory_EventScript_ExamineStarter

Virelia_MaplebrookResearchLaboratory_EventScript_WaterStarter::
	lockall
	faceplayer
	setvar VAR_0x8004, VIRELIA_STARTER_WATER
	goto Virelia_MaplebrookResearchLaboratory_EventScript_ExamineStarter

Virelia_MaplebrookResearchLaboratory_EventScript_FireStarter::
	lockall
	faceplayer
	setvar VAR_0x8004, VIRELIA_STARTER_FIRE
	goto Virelia_MaplebrookResearchLaboratory_EventScript_ExamineStarter

@ Guard remaining displays before opening a Pokemon preview.
Virelia_MaplebrookResearchLaboratory_EventScript_ExamineStarter:
#if VIRELIA_PROLOGUE && !VIRELIA_DEVELOPMENT_MODE && !IS_FRLG
	checkflag FLAG_VIRELIA_STARTER_RECEIVED
	goto_if TRUE, Virelia_MaplebrookResearchLaboratory_EventScript_AlreadyChosen
	checkflag FLAG_VIRELIA_MOM_MET
	goto_if FALSE, Virelia_MaplebrookResearchLaboratory_EventScript_SeeMom
#endif
	switch VAR_0x8004
	case VIRELIA_STARTER_GRASS, Virelia_MaplebrookResearchLaboratory_EventScript_PreviewGrassStarter
	case VIRELIA_STARTER_WATER, Virelia_MaplebrookResearchLaboratory_EventScript_PreviewWaterStarter
	case VIRELIA_STARTER_FIRE, Virelia_MaplebrookResearchLaboratory_EventScript_PreviewFireStarter
	releaseall
	end

Virelia_MaplebrookResearchLaboratory_EventScript_PreviewGrassStarter:
	showmonpic SPECIES_POOCHYENA_VIRELIA, 10, 3
	msgbox Virelia_MaplebrookResearchLaboratory_Text_GrassStarter, MSGBOX_DEFAULT
	goto Virelia_MaplebrookResearchLaboratory_EventScript_ChooseStarter

Virelia_MaplebrookResearchLaboratory_EventScript_PreviewWaterStarter:
	showmonpic SPECIES_NATU_VIRELIA, 10, 3
	msgbox Virelia_MaplebrookResearchLaboratory_Text_WaterStarter, MSGBOX_DEFAULT
	goto Virelia_MaplebrookResearchLaboratory_EventScript_ChooseStarter

Virelia_MaplebrookResearchLaboratory_EventScript_PreviewFireStarter:
	showmonpic SPECIES_EKANS_VIRELIA, 10, 3
	msgbox Virelia_MaplebrookResearchLaboratory_Text_FireStarter, MSGBOX_DEFAULT
	goto Virelia_MaplebrookResearchLaboratory_EventScript_ChooseStarter

Virelia_MaplebrookResearchLaboratory_EventScript_ChooseStarter:
#if VIRELIA_PROLOGUE && !VIRELIA_DEVELOPMENT_MODE && !IS_FRLG
	msgbox Virelia_MaplebrookResearchLaboratory_Text_ConfirmStarter, MSGBOX_YESNO
	hidemonpic
	goto_if_ne VAR_RESULT, YES, Virelia_MaplebrookResearchLaboratory_EventScript_CancelChoice
	call VireliaPrologue_EventScript_TryGiveStarter
	goto_if_eq VAR_RESULT, FALSE, Virelia_MaplebrookResearchLaboratory_EventScript_StarterFailed
	closemessage
	getobjectfacingdirection LOCALID_PLAYER, VAR_0x8006
	call Virelia_MaplebrookResearchLaboratory_EventScript_RemoveChosenStarter
	playfanfare MUS_OBTAIN_ITEM
	waitfanfare
	msgbox Virelia_MaplebrookResearchLaboratory_Text_ReceivedStarter, MSGBOX_DEFAULT
	closemessage
	goto Virelia_MaplebrookResearchLaboratory_EventScript_StarterScene
#else
	hidemonpic
	msgbox Virelia_MaplebrookResearchLaboratory_Text_Exhibition, MSGBOX_DEFAULT
#endif
	release
	end

Virelia_MaplebrookResearchLaboratory_EventScript_CancelChoice:
	msgbox Virelia_MaplebrookResearchLaboratory_Text_CancelChoice, MSGBOX_DEFAULT
	release
	end

Virelia_MaplebrookResearchLaboratory_EventScript_AlreadyChosen:
	msgbox Virelia_MaplebrookResearchLaboratory_Text_AlreadyChosen, MSGBOX_DEFAULT
	release
	end

Virelia_MaplebrookResearchLaboratory_EventScript_StarterFailed:
	msgbox Virelia_MaplebrookResearchLaboratory_Text_StarterFailed, MSGBOX_DEFAULT
	release
	end

@ A ball disappears only after its Pokemon was successfully given.
Virelia_MaplebrookResearchLaboratory_EventScript_RemoveChosenStarter:
	switch VAR_VIRELIA_STARTER
	case VIRELIA_STARTER_GRASS, Virelia_MaplebrookResearchLaboratory_EventScript_RemoveGrassStarter
	case VIRELIA_STARTER_WATER, Virelia_MaplebrookResearchLaboratory_EventScript_RemoveWaterStarter
	case VIRELIA_STARTER_FIRE, Virelia_MaplebrookResearchLaboratory_EventScript_RemoveFireStarter
	return

Virelia_MaplebrookResearchLaboratory_EventScript_RemoveGrassStarter:
	removeobject LOCALID_VIRELIA_LAB_GRASS_STARTER
	return

Virelia_MaplebrookResearchLaboratory_EventScript_RemoveWaterStarter:
	removeobject LOCALID_VIRELIA_LAB_WATER_STARTER
	return

Virelia_MaplebrookResearchLaboratory_EventScript_RemoveFireStarter:
	removeobject LOCALID_VIRELIA_LAB_FIRE_STARTER
	return

@ Stage below the table. Its solid surface stays blocked after a pickup.
Virelia_MaplebrookResearchLaboratory_EventScript_StarterScene:
	hidefollower
	switch VAR_0x8006
	case DIR_NORTH, Virelia_MaplebrookResearchLaboratory_EventScript_StageFromBelow
	case DIR_SOUTH, Virelia_MaplebrookResearchLaboratory_EventScript_StageFromAbove
	case DIR_WEST, Virelia_MaplebrookResearchLaboratory_EventScript_StageFromRight
	releaseall
	end

Virelia_MaplebrookResearchLaboratory_EventScript_StageFromBelow:
	applymovement LOCALID_PLAYER, Virelia_MaplebrookResearchLaboratory_Movement_Down1
	goto Virelia_MaplebrookResearchLaboratory_EventScript_WaitForStaging

Virelia_MaplebrookResearchLaboratory_EventScript_StageFromAbove:
	switch VAR_VIRELIA_STARTER
	case VIRELIA_STARTER_GRASS, Virelia_MaplebrookResearchLaboratory_EventScript_AboveGrass
	case VIRELIA_STARTER_WATER, Virelia_MaplebrookResearchLaboratory_EventScript_AboveWater
	applymovement LOCALID_PLAYER, Virelia_MaplebrookResearchLaboratory_Movement_AroundFireTable
	goto Virelia_MaplebrookResearchLaboratory_EventScript_WaitForStaging

Virelia_MaplebrookResearchLaboratory_EventScript_AboveGrass:
	applymovement LOCALID_PLAYER, Virelia_MaplebrookResearchLaboratory_Movement_AroundGrassTable
	goto Virelia_MaplebrookResearchLaboratory_EventScript_WaitForStaging

Virelia_MaplebrookResearchLaboratory_EventScript_AboveWater:
	applymovement LOCALID_PLAYER, Virelia_MaplebrookResearchLaboratory_Movement_AroundWaterTable
	goto Virelia_MaplebrookResearchLaboratory_EventScript_WaitForStaging

Virelia_MaplebrookResearchLaboratory_EventScript_StageFromRight:
	applymovement LOCALID_PLAYER, Virelia_MaplebrookResearchLaboratory_Movement_Down2Left

Virelia_MaplebrookResearchLaboratory_EventScript_WaitForStaging:
	waitmovement LOCALID_PLAYER
	goto Virelia_MaplebrookResearchLaboratory_EventScript_PickRivalStarter

Virelia_MaplebrookResearchLaboratory_EventScript_PickRivalStarter:
	msgbox Virelia_MaplebrookResearchLaboratory_Text_RivalChoosing, MSGBOX_DEFAULT
	closemessage
	switch VAR_VIRELIA_STARTER
	case VIRELIA_STARTER_GRASS, Virelia_MaplebrookResearchLaboratory_EventScript_RivalTakesFire
	case VIRELIA_STARTER_WATER, Virelia_MaplebrookResearchLaboratory_EventScript_RivalTakesGrass
	case VIRELIA_STARTER_FIRE, Virelia_MaplebrookResearchLaboratory_EventScript_RivalTakesWater
	goto Virelia_MaplebrookResearchLaboratory_EventScript_EndScene

Virelia_MaplebrookResearchLaboratory_EventScript_RivalTakesFire:
	applymovement LOCALID_VIRELIA_LAB_VIKTOR, Virelia_MaplebrookResearchLaboratory_Movement_ToFire
	waitmovement LOCALID_VIRELIA_LAB_VIKTOR
	delay 16
	playse SE_BALL_OPEN
	removeobject LOCALID_VIRELIA_LAB_FIRE_STARTER
	delay 16
	setobjectxyperm LOCALID_VIRELIA_LAB_VIKTOR_POKEMON, 8, 7
	call Virelia_MaplebrookResearchLaboratory_EventScript_ReleaseRivalPokemon
	call Virelia_MaplebrookResearchLaboratory_EventScript_FollowRivalLeftFromCounter
	call Virelia_MaplebrookResearchLaboratory_EventScript_FollowRivalLeft
	call Virelia_MaplebrookResearchLaboratory_EventScript_FollowRivalLeft
	setobjectxyperm LOCALID_VIRELIA_LAB_VIKTOR, 5, 8
	setobjectmovementtype LOCALID_VIRELIA_LAB_VIKTOR, MOVEMENT_TYPE_FACE_LEFT
	setobjectxyperm LOCALID_VIRELIA_LAB_VIKTOR_POKEMON, 6, 8
	setobjectmovementtype LOCALID_VIRELIA_LAB_VIKTOR_POKEMON, MOVEMENT_TYPE_FACE_LEFT
	turnobject LOCALID_PLAYER, DIR_EAST
	goto Virelia_MaplebrookResearchLaboratory_EventScript_RivalBattle

Virelia_MaplebrookResearchLaboratory_EventScript_RivalTakesGrass:
	applymovement LOCALID_VIRELIA_LAB_VIKTOR, Virelia_MaplebrookResearchLaboratory_Movement_ToGrass
	waitmovement LOCALID_VIRELIA_LAB_VIKTOR
	delay 16
	playse SE_BALL_OPEN
	removeobject LOCALID_VIRELIA_LAB_GRASS_STARTER
	delay 16
	setobjectxyperm LOCALID_VIRELIA_LAB_VIKTOR_POKEMON, 4, 7
	call Virelia_MaplebrookResearchLaboratory_EventScript_ReleaseRivalPokemon
	call Virelia_MaplebrookResearchLaboratory_EventScript_FollowRivalRightFromCounter
	setobjectxyperm LOCALID_VIRELIA_LAB_VIKTOR, 5, 8
	setobjectmovementtype LOCALID_VIRELIA_LAB_VIKTOR, MOVEMENT_TYPE_FACE_RIGHT
	setobjectxyperm LOCALID_VIRELIA_LAB_VIKTOR_POKEMON, 4, 8
	setobjectmovementtype LOCALID_VIRELIA_LAB_VIKTOR_POKEMON, MOVEMENT_TYPE_FACE_RIGHT
	turnobject LOCALID_PLAYER, DIR_WEST
	goto Virelia_MaplebrookResearchLaboratory_EventScript_RivalBattle

Virelia_MaplebrookResearchLaboratory_EventScript_RivalTakesWater:
	applymovement LOCALID_VIRELIA_LAB_VIKTOR, Virelia_MaplebrookResearchLaboratory_Movement_ToWater
	waitmovement LOCALID_VIRELIA_LAB_VIKTOR
	delay 16
	playse SE_BALL_OPEN
	removeobject LOCALID_VIRELIA_LAB_WATER_STARTER
	delay 16
	setobjectxyperm LOCALID_VIRELIA_LAB_VIKTOR_POKEMON, 6, 7
	call Virelia_MaplebrookResearchLaboratory_EventScript_ReleaseRivalPokemon
	call Virelia_MaplebrookResearchLaboratory_EventScript_FollowRivalRightFromCounter
	setobjectxyperm LOCALID_VIRELIA_LAB_VIKTOR, 7, 8
	setobjectmovementtype LOCALID_VIRELIA_LAB_VIKTOR, MOVEMENT_TYPE_FACE_RIGHT
	setobjectxyperm LOCALID_VIRELIA_LAB_VIKTOR_POKEMON, 6, 8
	setobjectmovementtype LOCALID_VIRELIA_LAB_VIKTOR_POKEMON, MOVEMENT_TYPE_FACE_RIGHT
	turnobject LOCALID_PLAYER, DIR_WEST
	goto Virelia_MaplebrookResearchLaboratory_EventScript_RivalBattle

@ Spawn on the counter tile Viktor has just vacated, then follow his footsteps.
Virelia_MaplebrookResearchLaboratory_EventScript_ReleaseRivalPokemon:
	applymovement LOCALID_VIRELIA_LAB_VIKTOR, Virelia_MaplebrookResearchLaboratory_Movement_Down1
	waitmovement LOCALID_VIRELIA_LAB_VIKTOR
	callnative VireliaPrologue_SyncRivalPokemon
	addobject LOCALID_VIRELIA_LAB_VIKTOR_POKEMON
	playse SE_BALL_OPEN
	return

Virelia_MaplebrookResearchLaboratory_EventScript_FollowRivalLeftFromCounter:
	applymovement LOCALID_VIRELIA_LAB_VIKTOR, Virelia_MaplebrookResearchLaboratory_Movement_Left1
	waitmovement LOCALID_VIRELIA_LAB_VIKTOR
	applymovement LOCALID_VIRELIA_LAB_VIKTOR_POKEMON, Virelia_MaplebrookResearchLaboratory_Movement_Down1
	waitmovement LOCALID_VIRELIA_LAB_VIKTOR_POKEMON
	return

Virelia_MaplebrookResearchLaboratory_EventScript_FollowRivalLeft:
	applymovement LOCALID_VIRELIA_LAB_VIKTOR, Virelia_MaplebrookResearchLaboratory_Movement_Left1
	waitmovement LOCALID_VIRELIA_LAB_VIKTOR
	applymovement LOCALID_VIRELIA_LAB_VIKTOR_POKEMON, Virelia_MaplebrookResearchLaboratory_Movement_Left1
	waitmovement LOCALID_VIRELIA_LAB_VIKTOR_POKEMON
	return

Virelia_MaplebrookResearchLaboratory_EventScript_FollowRivalRightFromCounter:
	applymovement LOCALID_VIRELIA_LAB_VIKTOR, Virelia_MaplebrookResearchLaboratory_Movement_GreetPlayer
	waitmovement LOCALID_VIRELIA_LAB_VIKTOR
	applymovement LOCALID_VIRELIA_LAB_VIKTOR_POKEMON, Virelia_MaplebrookResearchLaboratory_Movement_Down1
	waitmovement LOCALID_VIRELIA_LAB_VIKTOR_POKEMON
	turnobject LOCALID_VIRELIA_LAB_VIKTOR_POKEMON, DIR_EAST
	return

Virelia_MaplebrookResearchLaboratory_EventScript_EndScene:
	releaseall
	end

Virelia_MaplebrookResearchLaboratory_Movement_Down1:
	walk_down
	step_end

Virelia_MaplebrookResearchLaboratory_Movement_EnterLab:
	walk_up
	walk_up
	walk_up
	walk_up
	step_end

Virelia_MaplebrookResearchLaboratory_Movement_GreetPlayer:
	walk_right
	step_end

Virelia_MaplebrookResearchLaboratory_Movement_Down2Left:
	walk_down
	walk_down
	walk_left
	step_end

@ Use the lower aisle to pass Viktor (5,8) and the assistant (9,9).
Virelia_MaplebrookResearchLaboratory_Movement_AroundGrassTable:
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_down
	walk_down
	walk_down
	walk_left
	walk_down
	walk_left
	walk_left
	walk_left
	walk_left
	walk_up
	step_end

Virelia_MaplebrookResearchLaboratory_Movement_AroundWaterTable:
	walk_right
	walk_right
	walk_right
	walk_down
	walk_down
	walk_down
	walk_left
	walk_left
	walk_left
	step_end

Virelia_MaplebrookResearchLaboratory_Movement_AroundFireTable:
	walk_right
	walk_down
	walk_down
	walk_down
	walk_left
	step_end

Virelia_MaplebrookResearchLaboratory_Movement_Left1:
	walk_left
	step_end

Virelia_MaplebrookResearchLaboratory_Movement_ToFire:
	walk_right
	walk_right
	walk_right
	walk_up
	face_up
	step_end

Virelia_MaplebrookResearchLaboratory_Movement_ToGrass:
	walk_left
	walk_up
	face_up
	step_end

Virelia_MaplebrookResearchLaboratory_Movement_ToWater:
	walk_right
	walk_up
	face_up
	step_end

Virelia_MaplebrookResearchLaboratory_EventScript_RivalBattle:
#if VIRELIA_PROLOGUE && !VIRELIA_DEVELOPMENT_MODE && !IS_FRLG
	lockall
	msgbox Virelia_MaplebrookResearchLaboratory_Text_RivalIntro, MSGBOX_DEFAULT
	closemessage
	switch VAR_VIRELIA_STARTER
	case VIRELIA_STARTER_GRASS, Virelia_MaplebrookResearchLaboratory_EventScript_RivalGrass
	case VIRELIA_STARTER_WATER, Virelia_MaplebrookResearchLaboratory_EventScript_RivalWater
	case VIRELIA_STARTER_FIRE, Virelia_MaplebrookResearchLaboratory_EventScript_RivalFire
	releaseall
#endif
	end

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
	trainerbattle_earlyrival TRAINER_VIRELIA_VIKTOR_GRASS, RIVAL_BATTLE_HEAL_AFTER, Virelia_MaplebrookResearchLaboratory_Text_RivalLost, Virelia_MaplebrookResearchLaboratory_Text_RivalWon, LOCALID_VIRELIA_LAB_VIKTOR
	goto Virelia_MaplebrookResearchLaboratory_EventScript_AfterRival

Virelia_MaplebrookResearchLaboratory_EventScript_RivalWater:
	trainerbattle_earlyrival TRAINER_VIRELIA_VIKTOR_WATER, RIVAL_BATTLE_HEAL_AFTER, Virelia_MaplebrookResearchLaboratory_Text_RivalLost, Virelia_MaplebrookResearchLaboratory_Text_RivalWon, LOCALID_VIRELIA_LAB_VIKTOR
	goto Virelia_MaplebrookResearchLaboratory_EventScript_AfterRival

Virelia_MaplebrookResearchLaboratory_EventScript_RivalFire:
	trainerbattle_earlyrival TRAINER_VIRELIA_VIKTOR_FIRE, RIVAL_BATTLE_HEAL_AFTER, Virelia_MaplebrookResearchLaboratory_Text_RivalLost, Virelia_MaplebrookResearchLaboratory_Text_RivalWon, LOCALID_VIRELIA_LAB_VIKTOR
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

Virelia_MaplebrookResearchLaboratory_Text_Introduction:
	.string "JAVOROVÁ: Vítej, {PLAYER}.\n"
	.string "Zkoumám Pokémony Virelie.\p"
	.string "Voda, les a staré doly tu\n"
	.string "změnily i dobře známé druhy.\p"
	.string "Přizpůsobili se zdejšímu životu.\n"
	.string "Proto mají jiné podoby i typy.\p"
	.string "Ve třech Ballech na stole čekají\n"
	.string "tři různé povahy. Prohlédni je.\p"
	.string "Vyber si parťáka, ne trofej.\n"
	.string "Trofej ti svačinu neohlídá.$"

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
	.string "Pokémon se přidal do tvého týmu!\p"
	.string "JAVOROVÁ: Viktor už čeká\n"
	.string "na váš první cvičný souboj.$"

Virelia_MaplebrookResearchLaboratory_Text_AlreadyChosen:
	.string "Svého parťáka už máš. Ostatní\n"
	.string "počkají na další trenéry.$"

Virelia_MaplebrookResearchLaboratory_Text_StarterFailed:
	.string "JAVOROVÁ: Nejdřív si musíme\n"
	.string "ujasnit, kdo už je ve tvém týmu.$"

Virelia_MaplebrookResearchLaboratory_Text_Exhibition:
	.string "Výzkumná expozice. Výběr patří\n"
	.string "do příběhového začátku hry.$"

Virelia_MaplebrookResearchLaboratory_Text_SeeViktor:
	.string "Viktor je u východu. Zkuste si\n"
	.string "souboj. Pak vás oba uzdravím.$"

Virelia_MaplebrookResearchLaboratory_Text_RivalWaiting:
	.string "VIKTOR: Hej v klidu si vyber.\n"
	.string "Já zkusím takovou taktiku.$"

Virelia_MaplebrookResearchLaboratory_Text_RivalChoosing:
	.string "VIKTOR: Tak teď jsem na řadě já.\n"
	.string "Už vím, kdo půjde se mnou.$"

Virelia_MaplebrookResearchLaboratory_Text_RivalIntro:
	.string "VIKTOR: Tak už jsme trenéři!\n"
	.string "Zkusíme, co naši parťáci umí?\p"
	.string "Hej teď ti řeknu takovou věc,\n"
	.string "co asi nikdo nečekal.\p"
	.string "Vzal jsem si typovou výhodu.\n"
	.string "Pro jistotu. Věřím totiž v sebe.\p"
	.string "JAVOROVÁ: Jen cvičně. A prosím\n"
	.string "bez zásahu do grantové žádosti.$"

Virelia_MaplebrookResearchLaboratory_Text_RivalLost:
	.string "Dobre hej, jsem tě nechal.$"

Virelia_MaplebrookResearchLaboratory_Text_RivalWon:
	.string "Vyšlo to! Ale příště se\n"
	.string "klidně může karta obrátit.$"

Virelia_MaplebrookResearchLaboratory_Text_RivalAfter:
	.string "VIKTOR: První souboj máme za sebou.\n"
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
