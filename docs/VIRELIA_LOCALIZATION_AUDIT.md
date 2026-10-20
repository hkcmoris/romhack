# Lokalizační audit prologu Virelia

Zdrojová kontrola i nativní kontrola níže uvedených obrazovek prošly. Samotný počet přeložených řetězců nepotvrzuje úplnou lokalizaci ani celý herní průchod.

Strojový seznam překladů je v [manifestu](../tools/virelia_prologue_localization.json). Kontrolu spouští `python tools/validate_virelia_prologue_text.py`. Používá skutečné šířky znaků z `src/fonts.c` a mapování znaků z `charmap.txt`. Makro `LOCALIZED_TEXT` zachovává původní anglické texty pro sestavení `TESTING`.

```json
{
  "schema": 1,
  "language": "cs",
  "status": "source_and_scoped_native_checked",
  "manifest": "tools/virelia_prologue_localization.json",
  "level_bound": 15,
  "manifest_entries": 707,
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
    "metric_measurement_followup": "source_changed_after_7c120_not_yet_native_checked"
  },
  "expanded_placeholder_layout": "source_pass",
  "baked_ui_graphics": "native_pass_on_listed_screens",
  "unbounded_grinding": "outside_audit_boundary"
}
```

Audit zahrnuje hlavní nabídku včetně poškozené uložené hry a baterie, nastavení, batoh, party, souhrn Pokémona, změnu přezdívky, opětovné učení útoků, Pokédex a kartu trenéra. Zahrnuje také použití léků a repelentů, účinky předmětů bez výsledku, získání úrovně po vzácném bonbonu, evoluci a návrat domů po porážce. Popisy soutěžních účinků jsou přeloženy, protože hráč může otevřít soutěžní záložku souhrnu, i když nemůže vstoupit do soutěže.

Data pokrývají výše uvedené druhy do úrovně 15, jejich útoky a schopnosti, odměny schopnosti Sběr na úrovních 1 až 15 a dosažitelné držené předměty. Útoky zahrnují i nabídku opětovného učení a náhradní útok Zápasení. Anglické vlastní názvy druhů a míst zůstávají zachovány. HP, PP, ID a názvy tlačítek zachovávají běžné herní značení.

Kontrola používá nejdelší možné jméno hráče o sedmi znacích a přezdívku o dvanácti znacích. Dále rozbaluje názvy předmětů, útoků, povah, míst a číselné hodnoty podle kontextu hlášky. Žádný překročený limit se nevynechává. Pevná pole kontroluje i v zakódovaných bajtech včetně koncové značky. Zvlášť měří 144 px pro popis schopnosti v souhrnu, 208 px v běžném dialogu a v Pokédexu, 102 px pro popis předmětu a 86 px pro záhlaví souhrnu. Pro Pokédex používá skutečný `FONT_NORMAL`; tento tisk se automaticky nezmenšuje.

Texty v grafice vytvářejí následující skripty z původních herních bitmapových písem. Zachovávají rozměry obrázků a indexované palety. Osmiřádkové popisky ponechávají dva řádky pro rozlišení háčku a čárky.

- `tools/virelia/localize_type_icons.py`: typy a soutěžní kategorie.
- `tools/virelia/localize_naming_buttons.py`: zpět, mazání a další znaky.
- `tools/virelia/localize_ui_graphics.py`: Pokédex včetně vyhledávání a neznámé oblasti, souhrn Pokémona a karta trenéra. Volba `--check` ověří, že opakované vygenerování nezmění uložené soubory.
- `tools/virelia/localize_font_labels.py`: sdílené symboly `Lv.` a `No` nahrazuje `úr.` a `č.` ve všech devíti latinských písmech. Kódy `{LV}`, `{LV_2}` a `{NO}` zůstávají zachovány. Zkratka původního trenéra je `PT/`.

Nativní snímky sestavení `bf4b5254a3` potvrdily čitelnost hlavních záložek, názvů útoků, schopností a typových štítků. Odhalily anglické symboly `Lv.`, `No` a `OT/` a chybné spojení názvu druhu s výzvou k přezdívce. Jejich opravy byly znovu ověřeny v sestavení `7c120`.

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

Snímky jsou místní pracovní výstupy v ignorovaném adresáři `outputs`. V `7c120` zůstávají v Pokédexu imperiální jednotky. Následná zdrojová změna přepíná prolog na metry, kilogramy a desetinnou čárku; její nativní kontrola je ještě nutná.

Vyšší úrovně lze získat opakovanými souboji. Tento audit proto nepotvrzuje překlad všech útoků a evolucí při neomezeném trénování. Nepokrývá jiné oblasti, multiplayer ani předměty a útoky dostupné až mimo uvedený rozsah. Úplná česká lokalizace celé hry není cílem tohoto seznamu. Délka hraní 15 až 20 minut zatím nebyla ručně změřena.
