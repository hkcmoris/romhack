Text_BootUpPC:
#if TESTING
	.string "{PLAYER} booted up the PC.$"
#else
	.string "{PLAYER} zapíná PC.$"
#endif

gText_WhichPCShouldBeAccessed::
#if TESTING
	.string "Which PC should be accessed?$"
#else
	.string "Které PC chceš otevřít?$"
#endif

gText_AccessedSomeonesPC::
#if TESTING
	.string "Accessed SOMEONE'S PC.$"
#else
	.string "Otevírá se úložiště Pokémonů.$"
#endif

gText_StorageSystemOpened::
#if TESTING
	.string "POKéMON Storage System opened.$"
#else
	.string "Úložiště Pokémonů je připraveno.$"
#endif

gText_AccessedPlayersPC::
#if TESTING
	.string "Accessed {PLAYER}'s PC.$"
#else
	.string "Otevírá se PC: {PLAYER}.$"
#endif

gText_AccessedLanettesPC::
#if TESTING
	.string "Accessed LANETTE's PC.$"
#else
	.string "Otevírá se PC Lanette.$"
#endif

gText_AccessedBillsPC::
#if TESTING
	.string "Accessed BILL's PC.$"
#else
	.string "Otevírá se PC Billa.$"
#endif
