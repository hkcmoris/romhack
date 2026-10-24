@ Shared by the playable stone events and native script tests.
VireliaSanctuary_EventScript_ResetStones::
	setvar VAR_VIRELIA_SANCTUARY_STONE, 0
	return

VireliaSanctuary_EventScript_TouchStone::
	setvar VAR_RESULT, VIRELIA_SANCTUARY_WRONG
	goto_if_unset FLAG_VIRELIA_SIDEQUEST_REWARDED, VireliaSanctuary_Return
	goto_if_set FLAG_VIRELIA_SANCTUARY_SHORTCUT_OPEN, VireliaSanctuary_AlreadySolved
	switch VAR_0x8004
	case VIRELIA_SANCTUARY_SEED, VireliaSanctuary_Seed
	case VIRELIA_SANCTUARY_RAIN, VireliaSanctuary_Rain
	case VIRELIA_SANCTUARY_ROOT, VireliaSanctuary_Root
	goto VireliaSanctuary_Wrong

VireliaSanctuary_Seed:
	goto_if_ne VAR_VIRELIA_SANCTUARY_STONE, 0, VireliaSanctuary_Wrong
	setvar VAR_VIRELIA_SANCTUARY_STONE, 1
	setvar VAR_RESULT, VIRELIA_SANCTUARY_PROGRESS
	return

VireliaSanctuary_Rain:
	goto_if_ne VAR_VIRELIA_SANCTUARY_STONE, 1, VireliaSanctuary_Wrong
	setvar VAR_VIRELIA_SANCTUARY_STONE, 2
	setvar VAR_RESULT, VIRELIA_SANCTUARY_PROGRESS
	return

VireliaSanctuary_Root:
	goto_if_ne VAR_VIRELIA_SANCTUARY_STONE, 2, VireliaSanctuary_Wrong
	setvar VAR_VIRELIA_SANCTUARY_STONE, 0
	setflag FLAG_VIRELIA_SANCTUARY_SHORTCUT_OPEN
	setvar VAR_RESULT, VIRELIA_SANCTUARY_SOLVED
	return

VireliaSanctuary_Wrong:
	setvar VAR_VIRELIA_SANCTUARY_STONE, 0
	return

VireliaSanctuary_AlreadySolved:
	setvar VAR_RESULT, VIRELIA_SANCTUARY_ALREADY_SOLVED
VireliaSanctuary_Return:
	return

VireliaSanctuary_EventScript_TryTakeBell::
	setvar VAR_RESULT, FALSE
	goto_if_unset FLAG_VIRELIA_SANCTUARY_SHORTCUT_OPEN, VireliaSanctuary_Return
	goto_if_set FLAG_VIRELIA_SANCTUARY_BELL_RECEIVED, VireliaSanctuary_Return
	additem ITEM_SOOTHE_BELL, 1
	goto_if_eq VAR_RESULT, FALSE, VireliaSanctuary_Return
	setflag FLAG_VIRELIA_SANCTUARY_BELL_RECEIVED
	return
