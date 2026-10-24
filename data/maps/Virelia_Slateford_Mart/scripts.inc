Virelia_Slateford_Mart_MapScripts::
	.byte 0

Virelia_Slateford_Mart_EventScript_Clerk::
	lock
	faceplayer
	message gText_HowMayIServeYou
	waitmessage
	pokemart Virelia_Slateford_Mart_Pokemart
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
	release
	end

	.align 2
Virelia_Slateford_Mart_Pokemart:
	.2byte ITEM_POKE_BALL
	.2byte ITEM_POTION
	.2byte ITEM_SUPER_POTION
	.2byte ITEM_ANTIDOTE
	.2byte ITEM_PARALYZE_HEAL
	.2byte ITEM_AWAKENING
	.2byte ITEM_REPEL
	.2byte ITEM_ESCAPE_ROPE
	pokemartlistend

Virelia_Slateford_Mart_EventScript_Woman::
	msgbox Virelia_Slateford_Mart_Text_Woman, MSGBOX_NPC
	end

Virelia_Slateford_Mart_EventScript_Boy::
	msgbox Virelia_Slateford_Mart_Text_Boy, MSGBOX_NPC
	end

Virelia_Slateford_Mart_Text_Woman:
	.string "Do lomu beru Escape Rope.\n"
	.string "Svačina cestu zpět neukáže.\p"
	.string "Repel zas odradí slabší divoké\n"
	.string "Pokémony. Horníky bohužel ne.$"

Virelia_Slateford_Mart_Text_Boy:
	.string "Potion vrací ztracené zdraví.\n"
	.string "Antidote léčí otravu.\p"
	.string "Poké Ball se hodí na nového\n"
	.string "parťáka. Doma se bude divit.$"
