Virelia_Route01_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, Virelia_Route01_OnLoad
	.byte 0

Virelia_Route01_OnLoad:
	setmetatile 1, 16, 0xE, 1
	setmetatile 1, 17, 0xE, 1
	goto_if_unset FLAG_VIRELIA_SIDEQUEST_REWARDED, Virelia_Route01_CheckSanctuaryShortcut
	setmetatile 1, 16, 0xD, 0
	setmetatile 1, 17, 0xD, 0
Virelia_Route01_CheckSanctuaryShortcut:
	setmetatile 1, 8, 0xE, 1
	setmetatile 1, 9, 0xE, 1
	goto_if_unset FLAG_VIRELIA_SANCTUARY_SHORTCUT_OPEN, Virelia_Route01_LoadDone
	setmetatile 1, 8, 0x1, 0
	setmetatile 1, 9, 0x1, 0
Virelia_Route01_LoadDone:
	end

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
	goto_if_set FLAG_VIRELIA_SIDEQUEST_REWARDED, Virelia_Route01_EventScript_ZigzagoonSanctuary
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
	.string "V trávě ležel Potion.\n"
	.string "Ukládáš ho do batohu.$"

Virelia_Route01_Text_PotionBagFull:
	.string "V batohu není na Potion místo.\n"
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

Virelia_Route01_EventScript_ZigzagoonSanctuary:
	playmoncry SPECIES_ZIGZAGOON, CRY_MODE_NORMAL
	msgbox Virelia_Route01_Text_ZigzagoonSanctuary, MSGBOX_DEFAULT
	waitmoncry
	release
	end

Virelia_Route01_EventScript_OvergrownPath::
	msgbox Virelia_Route01_Text_OvergrownPath, MSGBOX_SIGN
	end

Virelia_Route01_EventScript_SanctuaryShortcut::
	msgbox Virelia_Route01_Text_SanctuaryShortcut, MSGBOX_SIGN
	end

Virelia_Route01_Text_ZigzagoonSanctuary:
	.string "Zigzagoon uhýbá větvičkami.\n"
	.string "Za hnízdem vede zarostlá pěšina.\p"
	.string "V mechu je otisk obrovské tlapy.\n"
	.string "Co dalšího tu asi hlídá?$"

Virelia_Route01_Text_OvergrownPath:
	.string "Husté větve skrývají pěšinu.\n"
	.string "Zigzagoon bedlivě hlídá okolí.\p"
	.string "Nejdřív dej Haně vědět,\n"
	.string "že je její parťák v pořádku.$"

Virelia_Route01_Text_SanctuaryShortcut:
	.string "Propletené kořeny tvoří stěnu.\n"
	.string "Z druhé strany zní tichý tón.$"
