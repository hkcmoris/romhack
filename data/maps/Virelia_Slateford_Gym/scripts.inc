Virelia_Slateford_Gym_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Virelia_Slateford_Gym_OnTransition
	map_script MAP_SCRIPT_ON_LOAD, Virelia_Slateford_Gym_OnLoad
	.byte 0

Virelia_Slateford_Gym_OnTransition:
	callnative VireliaFirstGym_ResetGymPuzzle
	end

Virelia_Slateford_Gym_OnLoad:
	goto_if_set FLAG_VIRELIA_GYM_PUZZLE_COMPLETE, Virelia_Slateford_Gym_OpenGate
	setmetatile 6, 6, 0x229, 1
	end

Virelia_Slateford_Gym_OpenGate:
	setmetatile 6, 6, 0x201, 0
	end

Virelia_Slateford_Gym_EventScript_Ivan::
	trainerbattle_single TRAINER_VIRELIA_GYM_IVAN, Virelia_Slateford_Gym_Text_IvanIntro, Virelia_Slateford_Gym_Text_IvanDefeat
	msgbox Virelia_Slateford_Gym_Text_IvanAfter, MSGBOX_AUTOCLOSE
	end

Virelia_Slateford_Gym_EventScript_Dana::
	trainerbattle_single TRAINER_VIRELIA_GYM_DANA, Virelia_Slateford_Gym_Text_DanaIntro, Virelia_Slateford_Gym_Text_DanaDefeat
	msgbox Virelia_Slateford_Gym_Text_DanaAfter, MSGBOX_AUTOCLOSE
	end

Virelia_Slateford_Gym_EventScript_Guide::
	msgbox Virelia_Slateford_Gym_Text_Guide, MSGBOX_NPC
	end

Virelia_Slateford_Gym_EventScript_Statue::
	lockall
	goto_if_set FLAG_VIRELIA_BADGE_QUARRY, Virelia_Slateford_Gym_EventScript_StatueWinner
	msgbox Virelia_Slateford_Gym_Text_Statue, MSGBOX_DEFAULT
	releaseall
	end

Virelia_Slateford_Gym_EventScript_StatueWinner:
	checkplayergender
	goto_if_eq VAR_RESULT, FEMALE, Virelia_Slateford_Gym_EventScript_StatueWinnerFemale
	msgbox Virelia_Slateford_Gym_Text_StatueWinner, MSGBOX_DEFAULT
	releaseall
	end

Virelia_Slateford_Gym_EventScript_StatueWinnerFemale:
	msgbox Virelia_Slateford_Gym_Text_StatueWinnerFemale, MSGBOX_DEFAULT
	releaseall
	end

Virelia_Slateford_Gym_EventScript_OldLayer::
	lockall
	goto_if_set FLAG_VIRELIA_GYM_PUZZLE_COMPLETE, Virelia_Slateford_Gym_EventScript_PuzzleAlready
	msgbox Virelia_Slateford_Gym_Text_OldLayer, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Virelia_Slateford_Gym_EventScript_Release
	setvar VAR_0x8004, 1
	call VireliaFirstGym_EventScript_GymSwitch
	goto_if_ne VAR_VIRELIA_GYM_SWITCH, 1, Virelia_Slateford_Gym_EventScript_WrongOrder
	msgbox Virelia_Slateford_Gym_Text_OldLayerSet, MSGBOX_DEFAULT
	releaseall
	end

Virelia_Slateford_Gym_EventScript_YoungLayer::
	lockall
	goto_if_set FLAG_VIRELIA_GYM_PUZZLE_COMPLETE, Virelia_Slateford_Gym_EventScript_PuzzleAlready
	msgbox Virelia_Slateford_Gym_Text_YoungLayer, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Virelia_Slateford_Gym_EventScript_Release
	setvar VAR_0x8004, 2
	call VireliaFirstGym_EventScript_GymSwitch
	goto_if_eq VAR_RESULT, FALSE, Virelia_Slateford_Gym_EventScript_WrongOrder
	setmetatile 6, 6, 0x201, 0
	special DrawWholeMapView
	playse SE_BANG
	msgbox Virelia_Slateford_Gym_Text_PuzzleSolved, MSGBOX_DEFAULT
	releaseall
	end

Virelia_Slateford_Gym_EventScript_WrongOrder:
	msgbox Virelia_Slateford_Gym_Text_WrongOrder, MSGBOX_DEFAULT
	releaseall
	end

Virelia_Slateford_Gym_EventScript_PuzzleAlready:
	msgbox Virelia_Slateford_Gym_Text_PuzzleAlready, MSGBOX_DEFAULT
Virelia_Slateford_Gym_EventScript_Release:
	releaseall
	end

Virelia_Slateford_Gym_EventScript_Mara::
	lock
	faceplayer
	goto_if_set FLAG_VIRELIA_BADGE_QUARRY, Virelia_Slateford_Gym_EventScript_AfterBadge
	goto_if_unset FLAG_VIRELIA_GYM_PUZZLE_COMPLETE, Virelia_Slateford_Gym_EventScript_NotReady
	checktrainerflag TRAINER_VIRELIA_GYM_IVAN
	goto_if FALSE, Virelia_Slateford_Gym_EventScript_NotReady
	checktrainerflag TRAINER_VIRELIA_GYM_DANA
	goto_if FALSE, Virelia_Slateford_Gym_EventScript_NotReady
	trainerbattle_single TRAINER_VIRELIA_MARA, Virelia_Slateford_Gym_Text_MaraIntro, Virelia_Slateford_Gym_Text_MaraDefeat, Virelia_Slateford_Gym_EventScript_Won, NO_MUSIC
	goto Virelia_Slateford_Gym_EventScript_Won

Virelia_Slateford_Gym_EventScript_NotReady:
	msgbox Virelia_Slateford_Gym_Text_NotReady, MSGBOX_DEFAULT
	release
	end

Virelia_Slateford_Gym_EventScript_Won::
	call VireliaFirstGym_EventScript_AwardBadge
	goto_if_eq VAR_RESULT, FALSE, Virelia_Slateford_Gym_EventScript_VerifyBadge
	message Virelia_Slateford_Gym_Text_Badge
	waitmessage
	call Common_EventScript_PlayGymBadgeFanfare
	msgbox Virelia_Slateford_Gym_Text_BadgeInfo, MSGBOX_DEFAULT

Virelia_Slateford_Gym_EventScript_VerifyBadge:
	goto_if_unset FLAG_VIRELIA_BADGE_QUARRY, Virelia_Slateford_Gym_EventScript_NotReady
Virelia_Slateford_Gym_EventScript_AfterBadge:
	goto_if_set FLAG_VIRELIA_TM_ROCK_TOMB_RECEIVED, Virelia_Slateford_Gym_EventScript_MaraAfter
	call VireliaFirstGym_EventScript_AwardTM
	goto_if_eq VAR_RESULT, FALSE, Virelia_Slateford_Gym_EventScript_BagFull
	playfanfare MUS_OBTAIN_TMHM
	msgbox Virelia_Slateford_Gym_Text_TM, MSGBOX_DEFAULT
	waitfanfare
	msgbox Virelia_Slateford_Gym_Text_TMInfo, MSGBOX_DEFAULT
	release
	end

Virelia_Slateford_Gym_EventScript_BagFull:
	msgbox Virelia_Slateford_Gym_Text_BagFull, MSGBOX_DEFAULT
	release
	end

Virelia_Slateford_Gym_EventScript_MaraAfter:
	msgbox Virelia_Slateford_Gym_Text_MaraAfter, MSGBOX_DEFAULT
	release
	end

Virelia_Slateford_Gym_Text_IvanIntro:
	.string "IVAN: Kámen má slabiny.\n"
	.string "Já se je snažím nemít.$"

Virelia_Slateford_Gym_Text_IvanDefeat:
	.string "Dobrá. Jednu jsem právě našel.$"

Virelia_Slateford_Gym_Text_IvanAfter:
	.string "Spodní vrstva je starší.\n"
	.string "Začni u její západní značky.$"

Virelia_Slateford_Gym_Text_DanaIntro:
	.string "DANA: Každá vrstva má příběh.\n"
	.string "Můj teď začíná soubojem!$"

Virelia_Slateford_Gym_Text_DanaDefeat:
	.string "Tenhle zápis bude stručný.$"

Virelia_Slateford_Gym_Text_DanaAfter:
	.string "Mladší vrstva leží nahoře.\n"
	.string "Její značka je na východě.\p"
	.string "Po starší značce stiskni tuhle.\n"
	.string "Most k Maře se uvolní.$"

Virelia_Slateford_Gym_Text_Guide:
	.string "Vítej na stadionu Slateford!\n"
	.string "Mara tu zkoumá kámen i trenéry.\p"
	.string "Poraz Ivana a Danu. Pak seřaď\n"
	.string "značky vrstev od starší k mladší.\p"
	.string "Starší najdeš vlevo, mladší\n"
	.string "vpravo. Most se pak uvolní.\p"
	.string "Voda i tráva pomohou. Zkus také\n"
	.string "víc parťáků a stavové tahy.\p"
	.string "Když prohraješ, vrať se.\n"
	.string "Hotové úkoly ti nevezmeme.$"

Virelia_Slateford_Gym_Text_Statue:
	.string "STADION - Slateford City\n"
	.string "Vedoucí: Mara, geoložka.\p"
	.string "Kámen má paměť.\n"
	.string "Trenér by měl mít taktiku.$"

Virelia_Slateford_Gym_Text_StatueWinner:
	.string "STADION - Slateford City\n"
	.string "Lomový odznak získal: {PLAYER}!$"

Virelia_Slateford_Gym_Text_StatueWinnerFemale:
	.string "STADION - Slateford City\n"
	.string "Lomový odznak získala: {PLAYER}!$"

Virelia_Slateford_Gym_Text_OldLayer:
	.string "STARŠÍ VRSTVA - západní značka\n"
	.string "Stisknout značku?$"

Virelia_Slateford_Gym_Text_OldLayerSet:
	.string "Starší vrstva označena.\n"
	.string "Teď najdi mladší vrstvu vpravo.$"

Virelia_Slateford_Gym_Text_YoungLayer:
	.string "MLADŠÍ VRSTVA - východní značka\n"
	.string "Stisknout značku?$"

Virelia_Slateford_Gym_Text_WrongOrder:
	.string "Značky zhasly. Vrstvy musí jít\n"
	.string "od starší k mladší. Zkus to znovu.$"

Virelia_Slateford_Gym_Text_PuzzleSolved:
	.string "Vrstvy jsou ve správném pořadí.\n"
	.string "Kamenný most k Maře je volný!$"

Virelia_Slateford_Gym_Text_PuzzleAlready:
	.string "Značky souhlasí. Most je volný.$"

Virelia_Slateford_Gym_Text_NotReady:
	.string "MARA: Nejdřív oba trenéři\n"
	.string "a značky vrstev. Pak jsem na řadě.\p"
	.string "Tady nic nepřeskakujeme.\n"
	.string "Ani bezpečnostní školení.$"

Virelia_Slateford_Gym_Text_MaraIntro:
	.string "MARA: Rozvodna už běží. Díky.\n"
	.string "Lom a město drží při sobě.\p"
	.string "Kámen není jen tvrdý.\n"
	.string "Umí se přizpůsobit tlaku.\p"
	.string "Ukaž mi, co pod tlakem udělá\n"
	.string "tvůj tým. Začneme průzkum!$"

Virelia_Slateford_Gym_Text_MaraDefeat:
	.string "Pevný plán. Ten se mi líbí.$"

Virelia_Slateford_Gym_Text_Badge:
	.string "Získáváš Lomový odznak!$"

Virelia_Slateford_Gym_Text_BadgeInfo:
	.string "MARA: Ten odznak je důkaz,\n"
	.string "že se na tvůj tým dá spolehnout.\p"
	.string "Žádný kámen ze srdce.\n"
	.string "Tohle je poctivý vzorek.$"

Virelia_Slateford_Gym_Text_TM:
	.string "Získáváš TM: Rock Tomb!\n"
	.string "TM ukládáš do batohu.$"

Virelia_Slateford_Gym_Text_TMInfo:
	.string "MARA: Rock Tomb zasáhne soupeře\n"
	.string "a zpomalí ho. Tempo je důležité.\p"
	.string "Při těžbě i při souboji.\n"
	.string "Hlavně nezkoušej obojí naráz.$"

Virelia_Slateford_Gym_Text_BagFull:
	.string "MARA: Na TM nemáš místo.\n"
	.string "Odznak už máš. TM tu počká.$"

Virelia_Slateford_Gym_Text_MaraAfter:
	.string "MARA: Výborná práce. Tvoje jméno\n"
	.string "už patří k tomuhle stadionu.\p"
	.string "Teď nech odpočinout tým.\n"
	.string "Kameny naštěstí nikam neutečou.$"
