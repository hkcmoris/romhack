Virelia_Slateford_PokemonCenter_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Virelia_Slateford_PokemonCenter_OnTransition
	.byte 0

Virelia_Slateford_PokemonCenter_OnTransition:
	setrespawn HEAL_LOCATION_VIRELIA_SLATEFORD_POKEMON_CENTER
	end

@ This local Center has no Union Room initialization or upstairs service.
@ Reuse the ordinary heal effect and nurse messages without reading that room's
@ uninitialized name buffer (the shared nurse assumes CableClub_OnResume ran).
Virelia_Slateford_PokemonCenter_EventScript_Nurse::
	lock
	faceplayer
	setvar VAR_0x800B, LOCALID_VIRELIA_SLATEFORD_NURSE
	setvar VAR_0x8004, 0
	msgbox gText_WouldYouLikeToRestYourPkmn, MSGBOX_YESNO
	goto_if_ne VAR_RESULT, YES, Virelia_Slateford_PokemonCenter_NurseGoodbye
	incrementgamestat GAME_STAT_USED_POKECENTER
	message gText_IllTakeYourPkmn
	waitmessage
	hidefollower 0
	applymovement VAR_0x800B, Movement_PkmnCenterNurse_Turn
	waitmovement 0
	dofieldeffect FLDEFF_POKECENTER_HEAL
	waitfieldeffect FLDEFF_POKECENTER_HEAL
	applymovement VAR_0x800B, Common_Movement_WalkInPlaceFasterDown
	waitmovement 0
	special HealPlayerParty
	callnative UpdateFollowingPokemon
	goto_if_set FLAG_POKERUS_EXPLAINED, Virelia_Slateford_PokemonCenter_NurseReturn
	specialvar VAR_RESULT, IsPokerusInParty
	goto_if_ne VAR_RESULT, TRUE, Virelia_Slateford_PokemonCenter_NurseReturn
	setflag FLAG_POKERUS_EXPLAINED
	message gText_PokerusExplanation
	goto Virelia_Slateford_PokemonCenter_NurseFinish

Virelia_Slateford_PokemonCenter_NurseReturn:
	call EventScript_PkmnCenterNurse_ReturnPkmn
	goto Virelia_Slateford_PokemonCenter_NurseFinish

Virelia_Slateford_PokemonCenter_NurseGoodbye:
	call EventScript_PkmnCenterNurse_Goodbye

Virelia_Slateford_PokemonCenter_NurseFinish:
	waitmessage
	waitbuttonpress
	release
	end

Virelia_Slateford_PokemonCenter_EventScript_PCAdvice::
	msgbox Virelia_Slateford_PokemonCenter_Text_PCAdvice, MSGBOX_NPC
	end

Virelia_Slateford_PokemonCenter_EventScript_Advice::
	msgbox Virelia_Slateford_PokemonCenter_Text_Advice, MSGBOX_NPC
	end

Virelia_Slateford_PokemonCenter_Text_PCAdvice:
	.string "PC v rohu slouží všem trenérům.\n"
	.string "Uložíš do něj své Pokémony.\p"
	.string "Kamení tam prosím neukládej.\n"
	.string "Správce by se divil.$"

Virelia_Slateford_PokemonCenter_Text_Advice:
	.string "Sestra Joy vyléčí celý tým.\n"
	.string "A účtuje pořád stejně: nic.\p"
	.string "Před výpravou si také ulož hru.\n"
	.string "V lomu bývá špatný signál.\p"
	.string "Tedy, signál na přestávku.$"

Virelia_Slateford_PokemonCenter_EventScript_Clerk::
	lock
	faceplayer
	message gText_HowMayIServeYou
	waitmessage
	pokemart Virelia_Slateford_PokemonCenter_Pokemart
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
	release
	end

	.align 2
Virelia_Slateford_PokemonCenter_Pokemart:
	.2byte ITEM_POKE_BALL
	.2byte ITEM_POTION
	.2byte ITEM_SUPER_POTION
	.2byte ITEM_ANTIDOTE
	.2byte ITEM_PARALYZE_HEAL
	.2byte ITEM_AWAKENING
	.2byte ITEM_REPEL
	.2byte ITEM_ESCAPE_ROPE
	pokemartlistend

Virelia_Slateford_PokemonCenter_EventScript_ShopWoman::
	msgbox Virelia_Slateford_PokemonCenter_Text_ShopWoman, MSGBOX_NPC
	end

Virelia_Slateford_PokemonCenter_EventScript_ShopBoy::
	msgbox Virelia_Slateford_PokemonCenter_Text_ShopBoy, MSGBOX_NPC
	end

Virelia_Slateford_PokemonCenter_Text_ShopWoman:
	.string "Do lomu beru Escape Rope.\n"
	.string "Svačina cestu zpět neukáže.\p"
	.string "Repel zas odradí slabší divoké\n"
	.string "Pokémony. Horníky bohužel ne.$"

Virelia_Slateford_PokemonCenter_Text_ShopBoy:
	.string "Potion vrací ztracené zdraví.\n"
	.string "Antidote léčí otravu.\p"
	.string "Poké Ball se hodí na nového\n"
	.string "parťáka. Doma se bude divit.$"
