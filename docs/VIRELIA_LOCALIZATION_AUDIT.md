# Lokalizační audit prologu Virelia

Stav: rozpracovaná kontrola. Samotný počet přeložených řetězců nepotvrzuje úplnou lokalizaci ani průchod hrou.

Strojový seznam překladů je v [manifestu](../tools/virelia_prologue_localization.json). Kontrolu spouští `python tools/validate_virelia_prologue_text.py`. Používá skutečné šířky znaků z `src/fonts.c` a mapování znaků z `charmap.txt`. Makro `LOCALIZED_TEXT` zachovává původní anglické texty pro sestavení `TESTING`.

```json
{
  "schema": 1,
  "language": "cs",
  "status": "in_progress",
  "manifest": "tools/virelia_prologue_localization.json",
  "level_bound": 15,
  "wild_species": ["WURMPLE", "ZIGZAGOON", "POOCHYENA", "SEEDOT", "TAILLOW", "SHROOMISH", "RALTS"],
  "normal_evolutions": ["SILCOON", "CASCOON", "BEAUTIFLY", "DUSTOX", "NUZLEAF"],
  "trainer_species": ["RATTATA", "ZUBAT"],
  "regional_species_owner": "Virelia core implementation",
  "native_screenshot_verification": "pending",
  "expanded_placeholder_layout": "pending",
  "baked_ui_graphics": "in_progress",
  "unbounded_grinding": "outside_audit_boundary"
}
```

Audit zahrnuje hlavní nabídku, nastavení, batoh, party, souhrn Pokémona, změnu přezdívky, opětovné učení útoků, Pokédex a kartu trenéra. Data pokrývají výše uvedené druhy do úrovně 15, jejich útoky a schopnosti, odměny schopnosti Sběr na úrovních 1 až 15 a dosažitelné držené předměty. Anglické vlastní názvy druhů a míst zůstávají zachovány.

Vyšší úrovně lze získat opakovanými souboji. Tento audit proto nepotvrzuje překlad všech útoků a evolucí při neomezeném trénování. Úplná česká lokalizace celé hry není cílem tohoto seznamu. Na úrovni prologu je ještě nutné dokončit kontrolu dynamických zástupných hodnot, hlášek po porážce a textů přímo v grafice. Délka hraní 15 až 20 minut zatím nebyla ručně změřena.
