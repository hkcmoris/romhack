@ Small, immediate subroutines are shared by the playable events and native tests.
VireliaPrologue_EventScript_TryGiveStarter::
	callnative VireliaPrologue_GiveStarter
	return

VireliaPrologue_EventScript_TryGiveSupplies::
	callnative VireliaPrologue_GrantSupplies
	return

VireliaPrologue_EventScript_FindZigzagoon::
	setflag FLAG_VIRELIA_SIDEQUEST_RESOLVED
	return

VireliaPrologue_EventScript_TryRewardHana::
	setvar VAR_RESULT, FALSE
	checkflag FLAG_VIRELIA_SIDEQUEST_REWARDED
	goto_if TRUE, VireliaPrologue_EventScript_RewardHanaReturn
	checkflag FLAG_VIRELIA_SIDEQUEST_RESOLVED
	goto_if FALSE, VireliaPrologue_EventScript_RewardHanaReturn
	additem ITEM_POTION, 1
	compare VAR_RESULT, FALSE
	goto_if_eq VireliaPrologue_EventScript_RewardHanaReturn
	setflag FLAG_VIRELIA_SIDEQUEST_REWARDED
	return

VireliaPrologue_EventScript_RewardHanaReturn:
	setvar VAR_RESULT, FALSE
	return

VireliaPrologue_EventScript_TryTakePotion::
	setvar VAR_RESULT, FALSE
	checkflag FLAG_ITEM_VIRELIA_ROUTE01_POTION
	goto_if TRUE, VireliaPrologue_EventScript_TakePotionReturn
	additem ITEM_POTION, 1
	compare VAR_RESULT, FALSE
	goto_if_eq VireliaPrologue_EventScript_TakePotionReturn
	setflag FLAG_ITEM_VIRELIA_ROUTE01_POTION
	return

VireliaPrologue_EventScript_TakePotionReturn:
	setvar VAR_RESULT, FALSE
	return
