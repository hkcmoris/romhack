gText_HowMayIServeYou::
#if TESTING
	.string "Welcome!\p"
	.string "How may I serve you?$"
#else
	.string "Vítej!\pCo pro tebe mohu udělat?$"
#endif

gText_PleaseComeAgain::
#if TESTING
	.string "Please come again!$"
#else
	.string "Zase se zastav!$"
#endif

gText_PlayerWhatCanIDoForYou::
	.string "{PLAYER}{KUN}, welcome!\p"
	.string "What can I do for you?$"

Pokemart_DefaultItemList::
	.2byte ITEM_ULTRA_BALL
	.2byte ITEM_HYPER_POTION
	.2byte ITEM_MAX_POTION
	.2byte ITEM_FULL_RESTORE
	.2byte ITEM_FULL_HEAL
	.2byte ITEM_REVIVE
	.2byte ITEM_MAX_REPEL
	pokemartlistend
