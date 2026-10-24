Diagnostická ROM: návrat adres callbacků k chování 0.3.58

Použij soubor oraculum-myboy-control-...-release.gba z tohoto ZIPu.
Manifest uvádí přesný commit a SHA-256. Režim 1 zachovává všechny obrázky.
Vrací se pouze normalizace adres; opravy prostředků i dekomprese z ROM zůstávají.
Nejde o potvrzenou opravu My Boy! ani o původní binární ROM 0.3.58.

Postup na stejném My Boy! a telefonu:
1. Úplně ukonči předchozí hru. Spusť tuto ROM a čistou novou hru.
   Nenačítej save state z jiné ROM. Zachovej stejné nastavení emulátoru.
2. Sleduj i první odchod z domova. V laboratoři zobraz Spinaraka, odmítni B
   a otevři Ekansův Poké Ball. Zaznamenej, zda se správně zobrazí náhled.
3. Pokud to projde, odmítni Ekanse, vyber Poochyenu a pokračuj přes větu
   profesorky o Viktorově prvním cvičném souboji až do bitvy.
4. Pokud menu funguje, ulož stav emulátoru těsně před problémovým krokem
   a znovu při zamrznutí nebo diagnostickém zastavení. Pošli původní .st1,
   přesný poslední krok, verzi My Boy!, stav hudby a reakci menu.
5. Pokud vše projde, opakuj Spinarak -> B -> Ekans několikrát z nové hry.
   Jediný úspěšný průchod nepotvrzuje odstranění občasného selhání.

Log je kruhový buffer v RAM: zachovává posledních 64 událostí, callbacky,
skriptové příkazy, náhledy, bitvu a první zjištěnou chybu. Je v save state,
nikoli v běžném herním savu. Cold boot jej smaže. Při úplném zaseknutí
aplikace bez možnosti uložit stav tento logger získání dat nezaručuje.
Rollback může obnovit možnost zachytit stav, ale to teprve ověří Android.

Přiložený dekodér lze spustit v Pythonu 3:
    python decode_state.py freeze.st1 > diagnostics.json
Žádný nalezený log není totéž co průkaz nepřítomnosti chyby.
ELF/MAP/SYM patří přesně k této ROM; nepoužívej adresy z jiného sestavení.
