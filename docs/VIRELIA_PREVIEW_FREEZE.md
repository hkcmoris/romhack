# Virelia: zamrznutí náhledu startéra na My Boy!

## Rozsah a jistota závěru

Oprava patří do větve `codex/virelia-prologue`, základ `14a30dafeb`.
Hlášený problém: stisk A u Poochyeny v laboratoři profesorky Javorové
zastaví hru na Androidu/My Boy! ještě před zobrazením obrázku či textu.
Spinarak a Ekans údajně fungují; Windows a iPhone problém údajně nemají.
Tato hlášení nejsou přímým měřením na postiženém zařízení.

Zdrojová analýza prokázala několik nezávislých chyb v obsluze prostředků.
Oprava je odstraňuje. Konkrétní příčinu Android freeze zatím nelze potvrdit:
není k dispozici postižený telefon, jeho nastavení ani diagnostický záznam.
Úspěch CI nebo mGBA nelze vydávat za úspěšný test My Boy!.

## Cesta od skriptu ke grafice

`showmonpic` -> `ScriptMenu_ShowPokemonPic` -> `CreateMonSprite_PicBox`
-> `CreateMonPicSprite` -> `CreatePicSprite` -> `TryLoadSpecialPokePic`.
Výběr druhu, pohlaví, Unown a náhradních obrázků používá společný resolver
s původním `LoadSpecialPokePicIsEgg`. Obě přední bitmapy se dekomprimují
před vytvořením sprite, přestože náhled přes `gAnims_MonPic` ukazuje snímek 0.
Druhová animace `frontAnimFrames` se v tomto obyčejném náhledu nepoužívá.

Boj používá `HandleLoadSpecialPokePic` a stejný výběr dat/dekompresor.
Obchází však registr PicData, preview task, okno a jeho weather-paletu.
Chyby těchto náhledových prostředků proto samy o sobě nedokazují chybu
bojového sprite. Dosavadní void API a VRAM dekódování ponechávají původní
chování při nedostatku pomocné paměti; nová obnovitelná cesta patří PicSprite.

## Prokázané chyby a změny

- `CreateSprite` a `Alloc` jsou fail-fast obálky. Původní následné kontroly
  nedokázaly zachytit jejich selhání. Oba tvůrci PicSprite používají skutečné
  unchecked alokace, kontrolují sentinel a při chybě uvolní částečné prostředky.
- Původní kód mohl indexovat `gSprites[MAX_SPRITES]` nebo `gSprites[0xFFFF]`.
  PicBox i field-move helper nyní ověřují ID před přístupem k paletě.
  Do PicData se uloží pouze platný sprite.
- SMOL potřebuje další dočasnou alokaci až po rámech a jejich tabulce.
  `TryLoadSpecialPokePic` kontroluje hlavičku, kapacitu a zarovnání cíle;
  při nedostatku scratch paměti vrátí FALSE a umožní úklid a další pokus.
  Není to obecný validátor libovolného poškozeného komprimovaného proudu.
- Tabulky snímků měly neinicializované `relativeFrames`. Každý záznam se
  nyní nastaví výslovně. U náhledového snímku 0 jsou obě větve kopírování
  ekvivalentní; tento nález sám nevysvětluje hlášené okamžité zamrznutí.
- Načtená paleta používala druhový tag, zatímco sprite požadoval jiný tag.
  Nově se používá požadovaný tag, kontroluje se kapacita a rozlišuje vlastní
  a vypůjčená paleta. Sdílená paleta přežije zánik prvního PicSprite.
- Úklid ignoruje neplatná a neaktivní ID, chrání cizí znovupoužitý sprite,
  uvolní rámce, tabulku, vlastní paletu a affine matici. Při plné tabulce
  affine matic se nepoškodí matice 0 jiného sprite.
- Preview kontroluje celou tabulku tasků: `CreateTask` při zaplnění vrací 0,
  což je zároveň platné ID. Kontroluje také WINDOW_NONE a vrací částečně
  vytvořené okno/task. AddWindow má obnovitelné alokace a při rollbacku
  odstraní i vlastní BG tilemap pointer.
- Okamžité `hidemonpic` dříve mohlo přejít ze stavu 0 do čekajícího stavu 1
  a čekat navždy. Opakované volání mohlo přeskočit uvolnění sprite.
  Skrytí nyní nastaví úklidový stav idempotentně. Po neúspěšném show není
  instalováno čekání na neexistující task; skript pokračuje bez obrázku.

Save formát, druhová ID, artwork, palety, české texty, výběr startéra,
příběh a Link Multi kód se nemění. Metadata PicData jsou pouze dočasná RAM.

## Porovnání grafiky

Analyzovaná původní ROM: release 0.2.43, commit `14a30dafeb`.
Všechna data jsou zarovnaná na čtyři bajty. Přední PNG mají 64 x 128 pixelů,
dva snímky 64 x 64, indexovaný režim a 16 barev. `anim_front.png` je přesné
spojení front a front_2; normální i shiny palety mají 16 platných barev.
Registrace odkazuje na vlastní data všech tří druhů.

| Druh | Front: komprimovaná / rozbalená velikost | Back | SMOL scratch pro front |
| --- | --- | --- | --- |
| Poochyena Virelia | 1332 / 4096 B | 720 / 2048 B | 1728 B |
| Spinarak Virelia | 940 / 4096 B | 496 / 2048 B | 1352 B |
| Ekans Virelia | 1608 / 4096 B | 744 / 2048 B | 2056 B |

Všechny tři fronty používají SMOL režim 6. Nezávislé dekódování skutečných
ROM dat ověřilo délky, zpětné odkazy, hranice bitstreamu a přesnou shodu
výstupu se zdrojovými pixely. Poochyena tedy nemá největší scratch ani
odlišný kompresní režim. Není prokázaný důvod měnit artwork nebo kompresi.

SMOL i rychlé LZ77 kopírují ARM instrukce do pracovní paměti a vykonávají je.
Neshoda s dynamickou rekompilací emulátoru je pouze hypotéza, nikoli potvrzená
příčina ani tvrzení o známé chybě konkrétní verze My Boy!.

## Regresní pokrytí

- `test/pokemon_pic_preview.c`: skutečné vyčerpání tasků, oken, spritů,
  palet a heapu; rollback BG pointeru; 120 cyklů všech tří startérů;
  okamžité a opakované skrytí a skutečný `ScrCmd_hidemonpic` po selhání.
- `test/pic_sprite.c`: regular/affine, front/back, normální/shiny, vlastní
  i vypůjčené palety, plné PicData/OBJ tiles/OAM matice, neplatná ID,
  otrávený znovupoužitý heap a selhání každé ze tří alokací s retry.
- `test/pokemon_pic_decompress.c`: hashe obou předních snímků i zadních
  bitmap proti nezávislým zdrojovým datům, všechny palety, canary hranice,
  krátký/NULL/nezarovnaný cíl a SMOL scratch OOM.
- `test/battle/virelia_preview_graphics.c`: běžný bojový engine načte každého
  startéra současně na straně hráče i soupeře a dokončí tah s poškozením.

Existující CI nadále sestavuje Emerald, release, FireRed i LeafGreen,
spouští příběhové a vývojové testy včetně původních regresí a kontrolu dokumentace.
Výsledek pro dodaný commit ověřuje
[workflow větve](https://github.com/hkcmoris/romhack/actions?query=branch%3Acodex%2Fvirelia-prologue).
Konkrétní build, hash ROM a nativní výsledky patří také do předávacího přehledu.

## Přesný test na My Boy!

1. Zálohuj běžný herní save i stavy emulátoru. Zaznamenej model telefonu,
   Android, přesnou verzi My Boy!, BIOS, nastavení vykreslování a případné
   volby CPU. Zaznamenej název a SHA-256 testované ROM.
2. Použij finální release ROM z tohoto commitu. Spusť ji úplně znovu.
   Pokračuj z kopie běžného herního savu před výběrem startéra, nebo založ
   novou hru. Nenačítej save state vytvořený starší ROM. Obnovený stav by
   mohl zachovat staré tasky, pointery či dekódovaný kód místo čistého testu.
3. V laboratoři u Poochyeny stiskni A jednou, bez turba a rychlého přetáčení.
   Musí se objevit správný obrázek a český popis. Zruš volbu přes B/NE.
   Musí zmizet okno a hráč se musí znovu pohybovat.
4. Otevři a zruš stejný náhled dvacetkrát. Potom střídavě otevři Poochyenu,
   Spinaraka a Ekanse. Zkus i rychlé potvrzení a zrušení. Nesmí přibývat
   zamrzání, chybné palety, zbytky okna ani mizet okolní postavy.
5. Na samostatné kopii savu vyber Poochyenu, ověř zisk jediného Pokémona,
   odchod Viktora a první souboj. Zadní bojový sprite musí být správný.
   S vodním startérem ověř také soupeřovu Poochyenu v prvním souboji;
   tím se zvlášť otestuje její přední bojový sprite.
6. Ulož přes herní nabídku, ukonči emulaci a znovu načti běžný save.
   Ověř tým, pozici a postup. Stejný test proveď pro původně používané
   nastavení emulátoru; případné změny nastavuj jednotlivě a přesně je zapiš.
7. Pokud freeze trvá, uveď poslední dokončený krok, zda selže první nebo
   až další otevření, zda problém nastane i v boji a zda jej vyvolá nová hra.
   Přilož hash ROM, výše uvedené nastavení a krátké video. Save před problémem
   pomůže odlišit stav hry od rozdílu emulátoru. Samotný freeze neprokazuje
   neúspěšnou alokaci ani chybu artworku.
