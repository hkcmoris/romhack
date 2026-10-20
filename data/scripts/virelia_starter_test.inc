@ Species options use TRUE/FALSE from C. Event CPP runs before assembler
@ constants define those names. Evaluate this guard numerically in a local
@ scope, then restore the surrounding script preprocessor environment.
#ifndef TRUE
#define TRUE 1
#define VIRELIA_SCRIPT_UNDEF_TRUE
#endif
#ifndef FALSE
#define FALSE 0
#define VIRELIA_SCRIPT_UNDEF_FALSE
#endif
#if VIRELIA_STARTER_TEST && P_VIRELIA_STARTERS
#define VIRELIA_STARTER_SCRIPT_ENABLED 1
#else
#define VIRELIA_STARTER_SCRIPT_ENABLED 0
#endif
#ifdef VIRELIA_SCRIPT_UNDEF_TRUE
#undef TRUE
#undef VIRELIA_SCRIPT_UNDEF_TRUE
#endif
#ifdef VIRELIA_SCRIPT_UNDEF_FALSE
#undef FALSE
#undef VIRELIA_SCRIPT_UNDEF_FALSE
#endif

@ Development access only; all grants use normal engine gift/Bag commands.
@ FLAG_TEMP_10 is unused elsewhere in Littleroot and adds no saved flag IDs.
VireliaStarterTest_EventScript_SetVisibility::
#if VIRELIA_STARTER_SCRIPT_ENABLED
	clearflag FLAG_TEMP_10
#else
	setflag FLAG_TEMP_10
#endif
	end

@ Saved object events can already exist when resuming an older test save.
VireliaStarterTest_EventScript_ResumeVisibility::
#if !VIRELIA_STARTER_SCRIPT_ENABLED
	setflag FLAG_TEMP_10
	removeobject LOCALID_LITTLEROOT_VIRELIA_TEST_GUIDE
#endif
	end

VireliaStarterTest_EventScript_Guide::
#if VIRELIA_STARTER_SCRIPT_ENABLED
	lock
	faceplayer
	message VireliaStarterTest_Text_Welcome
	waitmessage
	dynmultichoice 12, 1, FALSE, 5, 0, DYN_MULTICHOICE_CB_NONE, VireliaStarterTest_Text_GrassFamily, VireliaStarterTest_Text_WaterFamily, VireliaStarterTest_Text_FireFamily, VireliaStarterTest_Text_Candies, gText_Cancel
	closemessage
	switch VAR_RESULT
	case 0, VireliaStarterTest_EventScript_Grass
	case 1, VireliaStarterTest_EventScript_Water
	case 2, VireliaStarterTest_EventScript_Fire
	case 3, VireliaStarterTest_EventScript_Candies
	goto VireliaStarterTest_EventScript_Cancel
#else
	end
#endif

#if VIRELIA_STARTER_SCRIPT_ENABLED
VireliaStarterTest_EventScript_Grass:
	message VireliaStarterTest_Text_SelectStage
	waitmessage
	dynmultipush VireliaStarterTest_Text_POOCHYENA_VIRELIA, SPECIES_POOCHYENA_VIRELIA
	dynmultipush VireliaStarterTest_Text_MIGHTYENA_VIRELIA, SPECIES_MIGHTYENA_VIRELIA
	dynmultipush VireliaStarterTest_Text_WRAITHENA, SPECIES_WRAITHENA
	dynmultipush gText_Cancel, SPECIES_NONE
	dynmultistack 14, 3, FALSE, 4, FALSE, SPECIES_POOCHYENA_VIRELIA, DYN_MULTICHOICE_CB_NONE
	closemessage
	goto VireliaStarterTest_EventScript_SelectLevel

VireliaStarterTest_EventScript_Water:
	message VireliaStarterTest_Text_SelectStage
	waitmessage
	dynmultipush VireliaStarterTest_Text_SPINARAK_VIRELIA, SPECIES_SPINARAK_VIRELIA
	dynmultipush VireliaStarterTest_Text_ARIADOS_VIRELIA, SPECIES_ARIADOS_VIRELIA
	dynmultipush VireliaStarterTest_Text_ARACHNAUT, SPECIES_ARACHNAUT
	dynmultipush gText_Cancel, SPECIES_NONE
	dynmultistack 14, 3, FALSE, 4, FALSE, SPECIES_SPINARAK_VIRELIA, DYN_MULTICHOICE_CB_NONE
	closemessage
	goto VireliaStarterTest_EventScript_SelectLevel

VireliaStarterTest_EventScript_Fire:
	message VireliaStarterTest_Text_SelectStage
	waitmessage
	dynmultipush VireliaStarterTest_Text_EKANS_VIRELIA, SPECIES_EKANS_VIRELIA
	dynmultipush VireliaStarterTest_Text_ARBOK_VIRELIA, SPECIES_ARBOK_VIRELIA
	dynmultipush VireliaStarterTest_Text_CINDRACOIL, SPECIES_CINDRACOIL
	dynmultipush gText_Cancel, SPECIES_NONE
	dynmultistack 14, 3, FALSE, 4, FALSE, SPECIES_EKANS_VIRELIA, DYN_MULTICHOICE_CB_NONE
	closemessage
	goto VireliaStarterTest_EventScript_SelectLevel

VireliaStarterTest_EventScript_SelectLevel:
	goto_if_eq VAR_RESULT, SPECIES_NONE, VireliaStarterTest_EventScript_Cancel
	goto_if_eq VAR_RESULT, MULTI_B_PRESSED, VireliaStarterTest_EventScript_Cancel
	copyvar VAR_0x8004, VAR_RESULT
	message VireliaStarterTest_Text_SelectLevel
	waitmessage
	dynmultipush VireliaStarterTest_Text_Level5, 5
	dynmultipush VireliaStarterTest_Text_Level10, 10
	dynmultipush VireliaStarterTest_Text_Level14, 14
	dynmultipush VireliaStarterTest_Text_Level16, 16
	dynmultipush VireliaStarterTest_Text_Level18, 18
	dynmultipush VireliaStarterTest_Text_Level20, 20
	dynmultipush VireliaStarterTest_Text_Level30, 30
	dynmultipush VireliaStarterTest_Text_Level34, 34
	dynmultipush VireliaStarterTest_Text_Level36, 36
	dynmultipush VireliaStarterTest_Text_Level38, 38
	dynmultipush VireliaStarterTest_Text_Level40, 40
	dynmultipush VireliaStarterTest_Text_Level50, 50
	dynmultipush gText_Cancel, 0
	@ Six visible rows fit above the text box; the native menu scrolls the rest.
	dynmultistack 20, 0, FALSE, 6, FALSE, 5, DYN_MULTICHOICE_CB_NONE
	closemessage
	goto_if_eq VAR_RESULT, 0, VireliaStarterTest_EventScript_Cancel
	goto_if_eq VAR_RESULT, MULTI_B_PRESSED, VireliaStarterTest_EventScript_Cancel
	copyvar VAR_0x8005, VAR_RESULT
	call VireliaStarterTest_EventScript_Give
	goto_if_ne VAR_RESULT, MON_GIVEN_TO_PARTY, VireliaStarterTest_EventScript_NoRoom
	bufferspeciesname STR_VAR_1, VAR_0x8004
	buffernumberstring STR_VAR_2, VAR_0x8005
	msgbox VireliaStarterTest_Text_Given, MSGBOX_DEFAULT
	release
	end

VireliaStarterTest_EventScript_Candies:
	additem ITEM_RARE_CANDY, 10
	goto_if_eq VAR_RESULT, FALSE, VireliaStarterTest_EventScript_BagFull
	msgbox VireliaStarterTest_Text_CandiesGiven, MSGBOX_DEFAULT
	release
	end

VireliaStarterTest_EventScript_BagFull:
	msgbox VireliaStarterTest_Text_BagFull, MSGBOX_DEFAULT
	release
	end

VireliaStarterTest_EventScript_NoRoom:
	msgbox VireliaStarterTest_Text_NoRoom, MSGBOX_DEFAULT
	release
	end

VireliaStarterTest_EventScript_Cancel:
	release
	end
#endif

@ Callable by the NPC and regression tests. Inputs: VAR_0x8004 species,
@ VAR_0x8005 level. Returns MON_GIVEN_TO_PARTY or MON_CANT_GIVE in VAR_RESULT.
@ Revalidate both inputs and capacity before givemon can append to the party.
VireliaStarterTest_EventScript_Give::
#if VIRELIA_STARTER_SCRIPT_ENABLED
	switch VAR_0x8004
	case SPECIES_POOCHYENA_VIRELIA, VireliaStarterTest_EventScript_ValidateLevel
	case SPECIES_MIGHTYENA_VIRELIA, VireliaStarterTest_EventScript_ValidateLevel
	case SPECIES_WRAITHENA, VireliaStarterTest_EventScript_ValidateLevel
	case SPECIES_SPINARAK_VIRELIA, VireliaStarterTest_EventScript_ValidateLevel
	case SPECIES_ARIADOS_VIRELIA, VireliaStarterTest_EventScript_ValidateLevel
	case SPECIES_ARACHNAUT, VireliaStarterTest_EventScript_ValidateLevel
	case SPECIES_EKANS_VIRELIA, VireliaStarterTest_EventScript_ValidateLevel
	case SPECIES_ARBOK_VIRELIA, VireliaStarterTest_EventScript_ValidateLevel
	case SPECIES_CINDRACOIL, VireliaStarterTest_EventScript_ValidateLevel
	goto VireliaStarterTest_EventScript_CannotGive

VireliaStarterTest_EventScript_ValidateLevel:
	switch VAR_0x8005
	case 5, VireliaStarterTest_EventScript_CheckParty
	case 10, VireliaStarterTest_EventScript_CheckParty
	case 14, VireliaStarterTest_EventScript_CheckParty
	case 16, VireliaStarterTest_EventScript_CheckParty
	case 18, VireliaStarterTest_EventScript_CheckParty
	case 20, VireliaStarterTest_EventScript_CheckParty
	case 30, VireliaStarterTest_EventScript_CheckParty
	case 34, VireliaStarterTest_EventScript_CheckParty
	case 36, VireliaStarterTest_EventScript_CheckParty
	case 38, VireliaStarterTest_EventScript_CheckParty
	case 40, VireliaStarterTest_EventScript_CheckParty
	case 50, VireliaStarterTest_EventScript_CheckParty
	goto VireliaStarterTest_EventScript_CannotGive

VireliaStarterTest_EventScript_CheckParty:
	getpartysize
	goto_if_ge VAR_RESULT, PARTY_SIZE, VireliaStarterTest_EventScript_CannotGive
	@ No slot override, forced IVs, moves, OT, or direct save data writes.
	givemon VAR_0x8004, VAR_0x8005
	goto_if_ne VAR_RESULT, MON_GIVEN_TO_PARTY, VireliaStarterTest_EventScript_Return
	setflag FLAG_SYS_POKEMON_GET
VireliaStarterTest_EventScript_Return:
	return
#endif
VireliaStarterTest_EventScript_CannotGive:
	setvar VAR_RESULT, MON_CANT_GIVE
	return

#if VIRELIA_STARTER_SCRIPT_ENABLED
VireliaStarterTest_Text_Welcome:
	.string "TEST STARTÉRŮ VIRELIE\n"
	.string "Vyber vývojovou řadu nebo pomůcky.$"
VireliaStarterTest_Text_GrassFamily:
	.string "TRÁVA / DUCH$"
VireliaStarterTest_Text_WaterFamily:
	.string "VODA / HMYZ$"
VireliaStarterTest_Text_FireFamily:
	.string "OHEŇ / JED$"
VireliaStarterTest_Text_Candies:
	.string "RARE CANDY x10$"
VireliaStarterTest_Text_SelectStage:
	.string "Vyber vývojové stadium.\n"
	.string "Všechny formy pocházejí z Virelie.$"
VireliaStarterTest_Text_SelectLevel:
	.string "Vyber úroveň pro testování.\n"
	.string "Seznam posouvej šipkami nahoru/dolů.$"
VireliaStarterTest_Text_Given:
	.string "Získáváš: {STR_VAR_1}, úr. {STR_VAR_2}!\n"
	.string "Statistiky a útoky najdeš v PŘEHLEDU.\p"
	.string "RARE CANDY umožní otestovat vývoj.\n"
	.string "Pokémona si ponecháš uložením hry.$"
VireliaStarterTest_Text_NoRoom:
	.string "Nejprve uvolni místo v týmu.\n"
	.string "Ulož Pokémona do PC a vrať se.$"
VireliaStarterTest_Text_CandiesGiven:
	.string "Do batohu přidáno: 10× RARE CANDY.\p"
	.string "Zvýší Pokémonovi úroveň.\n"
	.string "Vývoj probíhá při zvýšení úrovně.$"
VireliaStarterTest_Text_BagFull:
	.string "Na 10 bonbonů není místo.\n"
	.string "Uvolni místo v batohu a vrať se.$"
VireliaStarterTest_Text_POOCHYENA_VIRELIA:
	.string "POOCHYENA$"
VireliaStarterTest_Text_MIGHTYENA_VIRELIA:
	.string "MIGHTYENA$"
VireliaStarterTest_Text_WRAITHENA:
	.string "WRAITHENA$"
VireliaStarterTest_Text_SPINARAK_VIRELIA:
	.string "SPINARAK$"
VireliaStarterTest_Text_ARIADOS_VIRELIA:
	.string "ARIADOS$"
VireliaStarterTest_Text_ARACHNAUT:
	.string "ARACHNAUT$"
VireliaStarterTest_Text_EKANS_VIRELIA:
	.string "EKANS$"
VireliaStarterTest_Text_ARBOK_VIRELIA:
	.string "ARBOK$"
VireliaStarterTest_Text_CINDRACOIL:
	.string "CINDRACOIL$"
VireliaStarterTest_Text_Level5:
	.string "Úr. 5$"
VireliaStarterTest_Text_Level10:
	.string "Úr. 10$"
VireliaStarterTest_Text_Level14:
	.string "Úr. 14$"
VireliaStarterTest_Text_Level16:
	.string "Úr. 16$"
VireliaStarterTest_Text_Level18:
	.string "Úr. 18$"
VireliaStarterTest_Text_Level20:
	.string "Úr. 20$"
VireliaStarterTest_Text_Level30:
	.string "Úr. 30$"
VireliaStarterTest_Text_Level34:
	.string "Úr. 34$"
VireliaStarterTest_Text_Level36:
	.string "Úr. 36$"
VireliaStarterTest_Text_Level38:
	.string "Úr. 38$"
VireliaStarterTest_Text_Level40:
	.string "Úr. 40$"
VireliaStarterTest_Text_Level50:
	.string "Úr. 50$"
#endif

#undef VIRELIA_STARTER_SCRIPT_ENABLED
