Virelia_Slateford_PokemonCenter_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Virelia_Slateford_PokemonCenter_OnTransition
	.byte 0

Virelia_Slateford_PokemonCenter_OnTransition:
	setrespawn HEAL_LOCATION_VIRELIA_SLATEFORD_POKEMON_CENTER
	end

Virelia_Slateford_PokemonCenter_EventScript_Nurse::
	setvar VAR_0x800B, LOCALID_VIRELIA_SLATEFORD_NURSE
	call Common_EventScript_PkmnCenterNurse
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
