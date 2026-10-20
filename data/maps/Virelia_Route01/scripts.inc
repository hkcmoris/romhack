Virelia_Route01_MapScripts::
	.byte 0

Virelia_Route01_EventScript_Naturalist::
	msgbox Virelia_Route01_Text_Naturalist, MSGBOX_NPC
	end

Virelia_Route01_EventScript_Potion::
	lock
	faceplayer
	call VireliaPrologue_EventScript_TryTakePotion
	goto_if_eq VAR_RESULT, FALSE, Virelia_Route01_EventScript_PotionBagFull
	removeobject LOCALID_VIRELIA_ROUTE01_POTION
	playfanfare MUS_OBTAIN_ITEM
	waitfanfare
	msgbox Virelia_Route01_Text_Potion, MSGBOX_DEFAULT
	release
	end

Virelia_Route01_EventScript_PotionBagFull:
	msgbox Virelia_Route01_Text_PotionBagFull, MSGBOX_DEFAULT
	release
	end

Virelia_Route01_EventScript_Ela::
	trainerbattle_single TRAINER_VIRELIA_ELA, Virelia_Route01_Text_ElaIntro, Virelia_Route01_Text_ElaDefeat
	msgbox Virelia_Route01_Text_ElaAfter, MSGBOX_AUTOCLOSE
	end

Virelia_Route01_EventScript_Ota::
	trainerbattle_single TRAINER_VIRELIA_OTA, Virelia_Route01_Text_OtaIntro, Virelia_Route01_Text_OtaDefeat
	msgbox Virelia_Route01_Text_OtaAfter, MSGBOX_AUTOCLOSE
	end

Virelia_Route01_EventScript_LostZigzagoon::
	lock
	faceplayer
	playmoncry SPECIES_ZIGZAGOON, CRY_MODE_NORMAL
	msgbox Virelia_Route01_Text_LostZigzagoon, MSGBOX_DEFAULT
	waitmoncry
	call VireliaPrologue_EventScript_FindZigzagoon
	release
	end

Virelia_Route01_EventScript_SouthSign::
	msgbox Virelia_Route01_Text_SouthSign, MSGBOX_SIGN
	end

Virelia_Route01_EventScript_NorthSign::
	msgbox Virelia_Route01_Text_NorthSign, MSGBOX_SIGN
	end

Virelia_Route01_Text_Naturalist:
	.string "Zaposlouchej se do trávy.\n"
	.string "Ozývá se z ní spousta krůčků!\p"
	.string "Na východě je pěkná mýtina.\n"
	.string "Stojí za malou zacházku.$"

Virelia_Route01_Text_Potion:
	.string "V trávě ležel Lektvar.\n"
	.string "Ukládáš ho do batohu.$"

Virelia_Route01_Text_PotionBagFull:
	.string "V batohu není na Lektvar místo.\n"
	.string "Vrať se pro něj později.$"

Virelia_Route01_Text_ElaIntro:
	.string "ELA: Zkusíme si souboj?\n"
	.string "Moji parťáci už se rozcvičili.$"

Virelia_Route01_Text_ElaDefeat:
	.string "Příště rozcvičím i taktiku.$"

Virelia_Route01_Text_ElaAfter:
	.string "Když to nejde, vrať se k mámě\n"
	.string "nebo do laboratoře pro léčení.$"

Virelia_Route01_Text_OtaIntro:
	.string "OTA: Taillow zná každý strom.\n"
	.string "Já zatím trefím aspoň domů.$"

Virelia_Route01_Text_OtaDefeat:
	.string "Tak dneska radši pěšky.$"

Virelia_Route01_Text_OtaAfter:
	.string "Divoké Pokémony můžeš chytat.\n"
	.string "Mého Taillowa ale ne. Známe se.$"

Virelia_Route01_Text_LostZigzagoon:
	.string "ZIGZAGOON: Zig!\p"
	.string "Pod kořeny se choulí Wurmplové.\n"
	.string "Zigzagoon hlídá jejich hnízdo.\p"
	.string "Není ztracený. Jen bere svou\n"
	.string "stráž opravdu vážně. Řekni Haně.$"

Virelia_Route01_Text_SouthSign:
	.string "CESTA 01 - Whisperwood Trail\n"
	.string "Jih: Maplebrook Town\p"
	.string "Sever: Slateford City\n"
	.string "Pozor na divoké Pokémony!$"

Virelia_Route01_Text_NorthSign:
	.string "Před tebou leží Slateford City.\n"
	.string "Maplebrook Town je na jihu.$"
