Virelia_RainfallLowlands_MapScripts::
	.byte 0

Virelia_RainfallLowlands_EventScript_Weather::
	trainerbattle_single TRAINER_VIRELIA_RAINFALL_WEATHER, Virelia_RainfallLowlands_Text_WeatherIntro, Virelia_RainfallLowlands_Text_WeatherDefeat
	msgbox Virelia_RainfallLowlands_Text_WeatherAfter, MSGBOX_AUTOCLOSE
	end

Virelia_RainfallLowlands_Text_WeatherIntro:
	.string "MILAN: Pod útesem déšť ustává.\n"
	.string "Moje předpověď? Napínavý souboj!$"

Virelia_RainfallLowlands_Text_WeatherDefeat:
	.string "Tohle mi v měření chybělo.$"

Virelia_RainfallLowlands_Text_WeatherAfter:
	.string "Prší víc, než místní pamatují.\n"
	.string "Zatím nevíme proč. Měřit dál!$"
