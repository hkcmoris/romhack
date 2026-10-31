@ Shared compiled events are exercised directly by native tests.
VireliaRainfall_EventScript_BeginChapter::
	callnative VireliaRainfall_BeginChapter
	return

VireliaRainfall_EventScript_FinishRival::
	callnative VireliaRainfall_FinishRival
	return

VireliaRainfall_EventScript_OpenShortcut::
	callnative VireliaRainfall_OpenShortcut
	return

VireliaRainfall_EventScript_HarborReport::
	callnative VireliaRainfall_HarborReport
	return

VireliaRainfall_EventScript_InspectDamage::
	callnative VireliaRainfall_InspectDamage
	return

VireliaRainfall_EventScript_ResetSignal::
	callnative VireliaRainfall_ResetSignal
	return

VireliaRainfall_EventScript_SignalSwitch::
	callnative VireliaRainfall_SignalSwitch
	return

VireliaRainfall_EventScript_ReadManifest::
	callnative VireliaRainfall_ReadManifest
	return

VireliaRainfall_EventScript_CompleteChapter::
	callnative VireliaRainfall_CompleteChapter
	return

VireliaRainfall_EventScript_BeginBottle::
	callnative VireliaRainfall_BeginBottle
	return

VireliaRainfall_EventScript_BottleClue::
	callnative VireliaRainfall_BottleClue
	return

VireliaRainfall_EventScript_RewardBottle::
	callnative VireliaRainfall_RewardBottle
	return

VireliaRainfall_EventScript_TakeRouteItem::
	callnative VireliaRainfall_TakeRouteItem
	return

VireliaRainfall_EventScript_Release:
	releaseall
	end

VireliaRainfall_EventScript_BagFull:
	msgbox VireliaRainfall_Text_BagFull, MSGBOX_DEFAULT
	releaseall
	end

VireliaRainfall_Text_BagFull:
	.string "Brašna je plná. Udělej místo\n"
	.string "a vrať se. Odměna tu počká.$"


VireliaRainfall_EventScript_MaraUpdate:
	goto_if_set FLAG_VIRELIA_RAINFALL_COMPLETE, VireliaRainfall_MaraAfter
	msgbox VireliaRainfall_Text_Mara, MSGBOX_DEFAULT
	return
VireliaRainfall_MaraAfter:
	msgbox VireliaRainfall_Text_MaraAfter, MSGBOX_DEFAULT
	return
VireliaRainfall_EventScript_MomUpdate:
	goto_if_set FLAG_VIRELIA_RAINFALL_COMPLETE, VireliaRainfall_MomAfter
	msgbox VireliaRainfall_Text_Mom, MSGBOX_DEFAULT
	return
VireliaRainfall_MomAfter:
	msgbox VireliaRainfall_Text_MomAfter, MSGBOX_DEFAULT
	return
VireliaRainfall_EventScript_ProfessorUpdate:
	goto_if_set FLAG_VIRELIA_RAINFALL_COMPLETE, VireliaRainfall_ProfessorAfter
	msgbox VireliaRainfall_Text_Professor, MSGBOX_DEFAULT
	return
VireliaRainfall_ProfessorAfter:
	msgbox VireliaRainfall_Text_ProfessorAfter, MSGBOX_DEFAULT
	return
VireliaRainfall_Text_Mara:
	.string "MARA: Na severu začíná\n"
	.string "Rainfall Pass. Cesta k Seabreak.\p"
	.string "Kámen tam ustupuje lesům a vodě.\n"
	.string "Teď prší víc, než bývá obvyklé.\p"
	.string "Odznak potvrzuje, že cestu\n"
	.string "zvládneš. A zase se někdy stav.$"
VireliaRainfall_Text_MaraAfter:
	.string "MARA: Ze Seabreak přišla zpráva.\n"
	.string "Maják zase svítí. Dobrá práce.\p"
	.string "Vodivé minerály dál prověříme.\n"
	.string "Každá stopa potřebuje souvislosti.$"
VireliaRainfall_Text_Mom:
	.string "MÁMA: První odznak! To si zaslouží\n"
	.string "čaj a aspoň dvě sušenky.\p"
	.string "K moři si vezmi něco suchého.\n"
	.string "A vrať se, kdykoli budeš chtít.$"
VireliaRainfall_Text_MomAfter:
	.string "MÁMA: Ve zprávách byl maják\n"
	.string "a nějaký důležitý detektiv.\p"
	.string "Já mám radost hlavně z tebe.\n"
	.string "Boty nech prosím u dveří.$"
VireliaRainfall_Text_Professor:
	.string "JAVOROVÁ: Změny deště sledujeme\n"
	.string "i tady. Rainfall Pass je zajímavý.\p"
	.string "Pozoruj různé břehy a výšky.\n"
	.string "Déšť svědčí i vodním Pokémonům.\p"
	.string "Laboratoř zůstává otevřená.\n"
	.string "Můžeš sem kdykoli přinést poznatky.$"
VireliaRainfall_Text_ProfessorAfter:
	.string "JAVOROVÁ: Díky za zprávu o moři.\n"
	.string "Samotná bouře ještě není důkaz.\p"
	.string "Budeme měřit dál. A na tvé další\n"
	.string "poznatky tu místo vždycky najdeme.$"
