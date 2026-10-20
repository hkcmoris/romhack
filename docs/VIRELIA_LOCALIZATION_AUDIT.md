# Lokalizační audit prologu Virelia

Zdrojová kontrola v níže uvedeném rozsahu prošla. Nativní snímky obrazovek a herní průchod ještě čekají na kontrolu. Samotný počet přeložených řetězců nepotvrzuje úplnou lokalizaci ani průchod hrou.

Strojový seznam překladů je v [manifestu](../tools/virelia_prologue_localization.json). Kontrolu spouští `python tools/validate_virelia_prologue_text.py`. Používá skutečné šířky znaků z `src/fonts.c` a mapování znaků z `charmap.txt`. Makro `LOCALIZED_TEXT` zachovává původní anglické texty pro sestavení `TESTING`.

```json
{
  "schema": 1,
  "language": "cs",
  "status": "source_checked_runtime_pending",
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
  "native_screenshot_verification": "pending",
  "expanded_placeholder_layout": "source_pass",
  "baked_ui_graphics": "rendered_native_review_pending",
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

Nativní snímky první sestavené verze potvrdily čitelnost hlavních záložek, názvů útoků, schopností a typových štítků. Odhalily také anglické symboly úrovně a chybné spojení názvu druhu s výzvou k přezdívce. Zdrojové opravy jsou zahrnuty v tomto auditu; novou sestavenou ROM je ještě nutné zkontrolovat.

Vyšší úrovně lze získat opakovanými souboji. Tento audit proto nepotvrzuje překlad všech útoků a evolucí při neomezeném trénování. Nepokrývá jiné oblasti, multiplayer ani předměty a útoky dostupné až mimo uvedený rozsah. Úplná česká lokalizace celé hry není cílem tohoto seznamu. Nativní ověření nové ROM musí ještě potvrdit čitelnost grafických popisků a rozložení otevřených obrazovek. Délka hraní 15 až 20 minut zatím nebyla ručně změřena.
