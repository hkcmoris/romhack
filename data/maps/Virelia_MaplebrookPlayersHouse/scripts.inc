Virelia_MaplebrookPlayersHouse_MapScripts::
#if VIRELIA_PROLOGUE && !VIRELIA_DEVELOPMENT_MODE && !IS_FRLG
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Virelia_MaplebrookPlayersHouse_OnFrame
#endif
	.byte 0

Virelia_MaplebrookPlayersHouse_OnFrame:
	map_script_2 VAR_VIRELIA_PROLOGUE_STATE, VIRELIA_STATE_HOME, Virelia_MaplebrookPlayersHouse_EventScript_WakeUp
	.2byte 0

Virelia_MaplebrookPlayersHouse_EventScript_WakeUp::
#if VIRELIA_PROLOGUE && !VIRELIA_DEVELOPMENT_MODE && !IS_FRLG
	lockall
	callnative VireliaPrologue_MeetMom
	applymovement LOCALID_PLAYER, Virelia_MaplebrookPlayersHouse_Movement_FaceMom
	waitmovement 0
	msgbox Virelia_MaplebrookPlayersHouse_Text_WakeUp, MSGBOX_DEFAULT
	releaseall
#endif
	end

Virelia_MaplebrookPlayersHouse_Movement_FaceMom:
	face_left
	step_end

Virelia_MaplebrookPlayersHouse_EventScript_TV::
	msgbox Virelia_MaplebrookPlayersHouse_Text_TV, MSGBOX_SIGN
	end

Virelia_MaplebrookPlayersHouse_EventScript_Mom::
	lock
	faceplayer
	checkflag FLAG_VIRELIA_MOM_MET
	goto_if FALSE, Virelia_MaplebrookPlayersHouse_EventScript_FirstTalk
	checkflag FLAG_VIRELIA_STARTER_RECEIVED
	goto_if FALSE, Virelia_MaplebrookPlayersHouse_EventScript_GoToLab
	call_if_set FLAG_VIRELIA_FIRST_GYM_COMPLETE, VireliaRainfall_EventScript_MomUpdate
	msgbox Virelia_MaplebrookPlayersHouse_Text_Heal, MSGBOX_DEFAULT
	special HealPlayerParty
	callnative UpdateFollowingPokemon
	playfanfare MUS_HEAL
	waitfanfare
	msgbox Virelia_MaplebrookPlayersHouse_Text_Rested, MSGBOX_DEFAULT
	release
	end

Virelia_MaplebrookPlayersHouse_EventScript_FirstTalk:
	callnative VireliaPrologue_MeetMom
	msgbox Virelia_MaplebrookPlayersHouse_Text_WakeUp, MSGBOX_DEFAULT
	release
	end

Virelia_MaplebrookPlayersHouse_EventScript_GoToLab:
	msgbox Virelia_MaplebrookPlayersHouse_Text_GoToLab, MSGBOX_DEFAULT
	release
	end

Virelia_MaplebrookPlayersHouse_Text_WakeUp:
	.string "MÁMA: Dobré ráno, {PLAYER}!\n"
	.string "Profesorka Javorová už čeká.\p"
	.string "Laboratoř je na jihovýchodě\n"
	.string "Maplebrook Town.\p"
	.string "Dnes dostaneš prvního Pokémona.\n"
	.string "Svačinu máš. Rozum používej.$"

Virelia_MaplebrookPlayersHouse_Text_GoToLab:
	.string "MÁMA: Laboratoř najdeš na\n"
	.string "jihovýchodě Maplebrook Town.\p"
	.string "U Javorové si vybereš parťáka.\n"
	.string "Já ti zatím vyperu pyžamo.$"

Virelia_MaplebrookPlayersHouse_Text_Heal:
	.string "MÁMA: Pojďte si odpočinout.\n"
	.string "Hrdinství počká na čaj.$"

Virelia_MaplebrookPlayersHouse_Text_Rested:
	.string "Všichni jsou zase plní sil.\n"
	.string "Domů se můžeš vrátit kdykoli.$"

Virelia_MaplebrookPlayersHouse_Text_TV:
	.string "Běží pořad o přírodě Virelie.\n"
	.string "Venku máš lepší rozlišení.$"
