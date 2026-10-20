# Lokalizační audit prologu Virelia

Zdrojová kontrola i nativní kontrola níže uvedených obrazovek prošly. Samotný počet přeložených řetězců nepotvrzuje úplnou lokalizaci ani celý herní průchod.

Strojový seznam překladů je v [manifestu](../tools/virelia_prologue_localization.json). Kontrolu spouští `python tools/validate_virelia_prologue_text.py`. Používá skutečné šířky znaků z `src/fonts.c` a mapování znaků z `charmap.txt`. U prvního řádku popisu útoku načítá také pozice sloupců, kategorii a její ikonu, písmo a šířku okna přímo ze zdrojů. Dvanáct případů s hodnotami `100`, `999` a `---` kontroluje mezery mezi sloupci i pravý okraj 144 pixelů. Makro `LOCALIZED_TEXT` zachovává původní anglické texty pro sestavení `TESTING`.

```json
{
  "schema": 1,
  "language": "cs",
  "status": "source_and_scoped_native_checked",
  "manifest": "tools/virelia_prologue_localization.json",
  "level_bound": 15,
  "manifest_entries": 710,
  "map_script_texts": 76,
  "expanded_ui_cases": 115,
  "move_names": 64,
  "ability_names": 28,
  "item_names": 28,
  "nature_names": 25,
  "type_names": 21,
  "normal_dex_entries": 14,
  "regional_dex_entries": 9,
  "wild_species": ["WURMPLE", "ZIGZAGOON", "POOCHYENA", "SEEDOT", "TAILLOW", "SHROOMISH", "RALTS"],
  "normal_evolutions": ["SILCOON", "CASCOON", "BEAUTIFLY", "DUSTOX", "NUZLEAF"],
  "trainer_species": ["RATTATA", "ZUBAT"],
  "regional_species_owner": "Virelia core implementation",
  "native_screenshot_verification": {
    "build": "7c120",
    "method": "mGBA libretro, real buttons, fresh Grass playthrough, no RAM writes",
    "passed": ["new_game_naming", "options", "party", "summary", "stats", "battle_moves", "contest_moves", "move_descriptions", "nickname_title", "trainer_card_front", "trainer_card_back"],
    "regional_dex": "Fire2 native playthrough, National Dex and Ekans entry readable",
    "metric_measurement_followup": "ee05868 native_pass: 0,5 m and 13,6 kg",
    "pokedex_lifecycle_followup": "ee05868 native_pass: close full Dex, uncaught capture, nickname, next wild battle and return",
    "pokedex_lifecycle_evidence": "outputs/emulator/capturefixed-proof.json",
    "rocket_finale_followup": "ee05868 native_pass: development and release, clue, police, state 5, closed Gym, reentry and FLASH Continue"
  },
  "expanded_placeholder_layout": "source_pass",
  "baked_ui_graphics": "native_pass_on_listed_screens",
  "unbounded_grinding": "outside_audit_boundary"
}
```

Audit zahrnuje hlavní nabídku včetně poškozené uložené hry a baterie, nastavení, batoh, party, souhrn Pokémona, změnu přezdívky, opětovné učení útoků, Pokédex a kartu trenéra. Zahrnuje také použití léků a repelentů, účinky předmětů bez výsledku, získání úrovně po vzácném bonbonu, evoluci a návrat domů po porážce. Popisy soutěžních účinků jsou přeloženy, protože hráč může otevřít soutěžní záložku souhrnu, i když nemůže vstoupit do soutěže.

Data pokrývají výše uvedené druhy do úrovně 15, jejich útoky a schopnosti, odměny schopnosti Sběr na úrovních 1 až 15 a dosažitelné držené předměty. Útoky zahrnují i nabídku opětovného učení a náhradní útok Zápasení. Anglické vlastní názvy druhů a míst zůstávají zachovány. HP, PP, ID a názvy tlačítek zachovávají běžné herní značení.

Kontrola používá nejdelší možné jméno hráče o sedmi znacích a přezdívku o dvanácti znacích. Dále rozbaluje názvy předmětů, útoků, povah, míst a číselné hodnoty podle kontextu hlášky. Žádný překročený limit se nevynechává. Pevná pole kontroluje i v zakódovaných bajtech včetně koncové značky. Zvlášť měří 144 px pro popis schopnosti v souhrnu, 208 px v běžném dialogu a v Pokédexu, 102 px pro popis předmětu a 86 px pro záhlaví souhrnu. Pro Pokédex používá skutečný `FONT_NORMAL`; tento tisk se automaticky nezmenšuje.

Samostatný překlad 859 bojových řetězců v `src/battle_message.c` není součástí 710 položek manifestu. Funkce `BattleStringExpandPlaceholders` po rozbalení předává text funkci `BreakStringAutomatic` s limity `BATTLE_MSG_MAX_WIDTH` a `BATTLE_MSG_MAX_LINES`. Nativní souboje se startéry a trenéry ověřily použité výzvy, útoky, změny statistik, vítězství, porážku a získání úrovně. Tato kontrola nepotvrzuje šířku všech bojových řetězců ani všechny kombinace jejich zástupných hodnot.

Texty v grafice vytvářejí následující skripty z původních herních bitmapových písem. Zachovávají rozměry obrázků a indexované palety. Osmiřádkové popisky ponechávají dva řádky pro rozlišení háčku a čárky.

- `tools/virelia/localize_type_icons.py`: typy a soutěžní kategorie.
- `tools/virelia/localize_naming_buttons.py`: zpět, mazání a další znaky.
- `tools/virelia/localize_ui_graphics.py`: Pokédex včetně vyhledávání a neznámé oblasti, souhrn Pokémona, karta trenéra, stavové štítky týmu a všech čtyř pozic v souboji a tlačítko `POPIS` útoku pro L i R. Volba `--check` ověří, že opakované vygenerování nezmění uložené soubory.
- `tools/virelia/localize_font_labels.py`: sdílené symboly `Lv.` a `No` nahrazuje `úr.` a `č.` ve všech devíti latinských písmech. Kódy `{LV}`, `{LV_2}` a `{NO}` zůstávají zachovány. Zkratka původního trenéra je `PT/`.

Nativní snímky sestavení `bf4b5254a3` potvrdily čitelnost hlavních záložek, názvů útoků, schopností a typových štítků. Odhalily anglické symboly `Lv.`, `No` a `OT/` a chybné spojení názvu druhu s výzvou k přezdívce. Jejich opravy byly znovu ověřeny v sestavení `7c120`.

Pozdější nativní kontrola release `0.2.36` odhalila ještě grafické štítky `FNT` a `MOVE INFO`. Generátor je nyní nahrazuje českými `MDL` a `POPIS` a současně překládá ostatní stavové štítky: `OTR`, `PAR`, `SPÍ`, `MRZ`, `POP` a `OMR`. Vlastní název Pokérus si ponechává zkratku `PKRS`. Zdrojová kontrola potvrdila původní rozměry, palety, pořadí dlaždic a opakovatelné generování; náhled je v `outputs/czech-status-badges.png`. Nativní kontrola nového sestavení s těmito posledními grafickými změnami zatím čeká.

Kontrola `7c120` proběhla skutečnými stisky tlačítek v emulátoru mGBA libretro. Nová hra s travním startérem prošla tvorbou postavy, pojmenováním, domem a laboratoří. Do RAM se nezapisovaly žádné testovací hodnoty a nepřidávaly se předměty ani Pokémoni. Výsledky obrazovek:

| Obrazovka | Výsledek | Lokální nativní snímek |
| --- | --- | --- |
| Nastavení | Česká pole i hodnoty, bez ořezu | `outputs/emulator/audit-options-ready.png` |
| Party | `úr.5`, české menu a výzvy | `outputs/emulator/audit-party-context.png` |
| Souhrn | `č.001`, `PT/`, `ID č.`, `Setkání na úr.5.`, čitelná schopnost a typy | `outputs/emulator/audit-summary.png` |
| Statistiky | České názvy a rozložení hodnot bez ořezu | `outputs/emulator/audit-stats.png` |
| Útoky a soutěžní útoky | České názvy, štítky a popisy bez ořezu | `outputs/emulator/audit-battle-move-detail.png`, `outputs/emulator/audit-contest-detail.png` |
| Přezdívka | Celá výzva `Poochyena: jméno?`, česká tlačítka | `outputs/emulator/audit-nickname-title.png` |
| Karta trenéra | České titulky, ID a odznaky; prázdná nezasloužená pole na rubu | `outputs/emulator/audit-trainer-card-front.png`, `outputs/emulator/audit-trainer-card-back.png` |
| Regionální Pokédex | Národní seznam a český záznam Ekans bez ořezu po opravě odemčení | `outputs/emulator/fire2-national-dex.png`, `outputs/emulator/fire2-ekans-dex-description.png` |
| Metrické údaje | `ee05868`: výška `0,5 m`, hmotnost `13,6 kg`, desetinná čárka | `outputs/emulator/capturefixed-metric-poochyena.png` |
| Pokédex, odchyt a přezdívka | `ee05868`: odchyt po zavření Pokédexu, přezdívka a další souboj umožní návrat do mapy | `outputs/emulator/capturefixed-nickname.png`, `outputs/emulator/capturefixed-next-battle-return.png`, `outputs/emulator/capturefixed-proof.json` |
| Rocket a závěr | `ee05868`: výhra, česká stopa, příchod policisty, další cíl; ověřeno i v release | `outputs/emulator/releasefinal-policeman.png`, `outputs/emulator/releasefinal-next-objective.png` |
| Načtení po závěru | `ee05868`: restart pouze z FLASH, přesná pozice a tým, policista přítomen, Rocket pryč | `outputs/emulator/releasefinal-flash-world.png`, `outputs/emulator/grassfinal-continued-world.png` |

Snímky jsou místní pracovní výstupy v ignorovaném adresáři `outputs`. Kontrola `ee05868` potvrdila metrické jednotky a desetinnou čárku. Po zavření Pokédexu hráč chytil dosud nechycený druh Zigzagoon, zadal přezdívku `A` a vrátil se do mapy. Pak zahájil divoký souboj s Taillow, utekl a opět se vrátil do mapy. Kontrola RAM potvrdila `sPokedexView = NULL` po zavření Pokédexu a `gBattleStruct = NULL` po obou návratech. Průchod používal skutečné stisky tlačítek a původní uloženou hru z paměti FLASH.

Při kontrole `ee05868` prošly výhra nad Rocketem, všechny tři stránky policistova dialogu a závěrečný cíl. Policista používá původní grafiku, nově dostupnou i v Emerald. Dokončení nastavilo stav 5 a trvalý příznak. Stadion zůstal zavřený. Přechod město → trasa → město neopakoval scénu. Uložení a nové spuštění pouze z FLASH obnovily pozici, tým, předměty i dokončení. Závěr a načtení prošly také v optimalizované release ROM `0.2.36`.

Po opravách uvedených výše zůstává nativně ověřit poslední stavové štítky a tlačítko popisu útoku. Chybové obrazovky poškozené uložené hry, všechny kombinace léčiv a všechny větve učení útoků mají zdrojovou kontrolu; každá jejich obrazovka nebyla ručně vyvolána. Toto omezení platí i po úspěšné kompilaci a automatických testech.

Vyšší úrovně lze získat opakovanými souboji. Tento audit proto nepotvrzuje překlad všech útoků a evolucí při neomezeném trénování. Nepokrývá jiné oblasti, multiplayer ani předměty a útoky dostupné až mimo uvedený rozsah. Úplná česká lokalizace celé hry není cílem tohoto seznamu. Nativní průchod s travním startérem skončil v čase 14:03 bez odchytu; kontrola stadionu a návratu do města jej prodloužila na 14:55. Jde o herní hodiny při automatizovaném ovládání. Cílových 15 až 20 minut s odchytem zatím nebylo změřeno lidským hráčem.
