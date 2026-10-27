Virelia_Seabreak_Lighthouse_MapScripts::
	.byte 0

Virelia_Seabreak_Lighthouse_EventScript_Bela::
	lock
	faceplayer
	goto_if_set FLAG_VIRELIA_LIGHTHOUSE_REPAIRED, Virelia_Seabreak_Lighthouse_BelaAfter
	goto_if_unset FLAG_VIRELIA_HARBOR_REPORT, Virelia_Seabreak_Lighthouse_SeeHarbor
	msgbox Virelia_Seabreak_Lighthouse_Text_Bela, MSGBOX_DEFAULT
	release
	end
Virelia_Seabreak_Lighthouse_SeeHarbor:
	msgbox Virelia_Seabreak_Lighthouse_Text_SeeHarbor, MSGBOX_DEFAULT
	release
	end
Virelia_Seabreak_Lighthouse_BelaAfter:
	msgbox Virelia_Seabreak_Lighthouse_Text_BelaAfter, MSGBOX_DEFAULT
	release
	end

Virelia_Seabreak_Lighthouse_EventScript_Damage::
	lockall
	call VireliaRainfall_EventScript_InspectDamage
	goto_if_eq VAR_RESULT, FALSE, Virelia_Seabreak_Lighthouse_SeeHarbor
	msgbox Virelia_Seabreak_Lighthouse_Text_Damage, MSGBOX_DEFAULT
	releaseall
	end

Virelia_Seabreak_Lighthouse_Text_Bela:
	.string "BĚLA: Běla Křížová, policie.\n"
	.string "Pracuji s detektivem Vackem.\p"
	.string "Ve Slatefordu prý zabránil velké\n"
	.string "krádeži. Má výborné výsledky.\p"
	.string "Tady se přepálil přívod. Prověř\n"
	.string "skříň vlevo nahoře, pak schody.\p"
	.string "Nahoře zapni přívod, srovnej čočku\n"
	.string "a teprve potom spusť světlo.\p"
	.string "Já zajistím volné kabely.\n"
	.string "Na odkryté vodiče nesahej.$"

Virelia_Seabreak_Lighthouse_Text_SeeHarbor:
	.string "BĚLA: Maják zatím kontrolujeme.\n"
	.string "Nejdřív promluv se správcem.\p"
	.string "Čeká dole u přístavu.$"

Virelia_Seabreak_Lighthouse_Text_Damage:
	.string "Přívod je přepálený a svorka\n"
	.string "uvolněná. Žádné stopy po zásahu.\p"
	.string "BĚLA: Svorku zajistím. Náhradní\n"
	.string "přívod je teď bezpečný.\p"
	.string "Nahoře dodrž pořadí na návodu:\n"
	.string "přívod, čočka, světlo.$"

Virelia_Seabreak_Lighthouse_Text_BelaAfter:
	.string "BĚLA: Světlo drží. Ještě přečti\n"
	.string "lodní knihu nahoře na stole.\p"
	.string "Pak vše ohlásíme správci.\n"
	.string "Detektiv už bude jistě na cestě.$"

Virelia_Seabreak_Lighthouse_EventScript_Instructions::
	msgbox Virelia_Seabreak_Lighthouse_Text_Instructions, MSGBOX_SIGN
	end

Virelia_Seabreak_Lighthouse_Text_Instructions:
	.string "NOUZOVÉ SPUŠTĚNÍ\p"
	.string "1. Zapnout záložní přívod.\n"
	.string "2. Srovnat optickou čočku.\p"
	.string "3. Spustit signální světlo.\p"
	.string "Při chybném pořadí pojistka vypne\n"
	.string "celý postup. Začni od přívodu.$"
