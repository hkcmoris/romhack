Diagnostická ROM: ověření laboratorních náhledů

Použij oraculum-myboy-event-only-...-release.gba z tohoto ZIPu.
Režim 2 vynechává pouze vytvoření náhledů tří starterů v laboratoři.
Dialogy, ANO/NE, výběr startera, Viktorův pohyb a souboj zůstávají.
V souboji se oba Pokémoni mají zobrazit běžným způsobem.
Ostatní obrázky ani artwork se nemění. Normální vydání tuto výjimku nemá.

Proč tento test:
Předchozí control s původními adresami také zamrzl při Spinarak -> B -> Ekans.
Dodaný stav před A neukazuje poškození živého heapu, oken, palet ani callbacků.
Tento pokus ověří, zda selhání vyžaduje vytvoření laboratorního náhledu.
Sám o sobě nepotvrdí příčinu v dekodéru ani trvalou opravu My Boy!.
Přesný engine je shodný s kontrolní revizí c9630509; mění se diagnostický režim.

Postup na stejném My Boy! a telefonu, se stejným nastavením:
1. Úplně znovu spusť tuto ROM a začni čistou novou hru.
   Save state z kontrolní ROM patří k jinému sestavení; použij novou hru.
2. Zobraz nabídku Spinaraka a odmítni B. Obrázek záměrně neuvidíš.
3. Otevři Ekansův Poké Ball. Má se zobrazit český popis a ANO/NE bez obrázku.
4. Pokud to projde, odmítni Ekanse, vyber Poochyenu a pokračuj přes větu
   profesorky o prvním cvičném souboji až do Viktorovy bitvy.
5. Zaznamenej přesný poslední krok a zda reaguje menu My Boy!.
   Pokud menu funguje, zachovej stav emulátoru .st1 ze stejné ROM.

Log obsahuje posledních 64 událostí v RAM. Běžný herní save jej neobsahuje.
Restart log smaže a úplné zaseknutí aplikace může zabránit jeho zachycení.
Při diagnostické chybě stávající logger zastaví běh bez chybové obrazovky;
bez zachyceného stavu nelze určit, zda tuto větev selhání dosáhlo.
Dekodér: python decode_state.py freeze.st1 > diagnostics.json
Manifest, SHA256SUMS a ELF/MAP/SYM patří přesně k této ROM.
