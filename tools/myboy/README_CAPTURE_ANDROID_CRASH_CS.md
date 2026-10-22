# ZĂˇznam pĂˇdu My Boy! pĹ™es USB

Skript uloĹľĂ­ pouze zĂˇznamy pĂˇdu, kterĂ© obsahujĂ­ pĹ™esnĂ˝ nĂˇzev vybranĂ©ho balĂ­ÄŤku My Boy!. NespouĹˇtĂ­ hru a neovlĂˇdĂˇ telefon. NemaĹľe logy, nezapisuje do telefonu a nic neodesĂ­lĂˇ.

## SpuĹˇtÄ›nĂ­

1. PouĹľij jiĹľ nainstalovanĂ© Android SDK Platform Tools. Skript ADB nestahuje.
3. V telefonu povol ladÄ›nĂ­ USB. PĹ™ipoj telefon kabelem, odemkni jej a potvrÄŹ systĂ©movou vĂ˝zvu s RSA klĂ­ÄŤem tohoto poÄŤĂ­taÄŤe. Tyto kroky vyĹľaduje [oficiĂˇlnĂ­ postup ADB](https://developer.android.com/tools/adb).
3. Zopakuj problĂ©m v My Boy!. Poznamenej ÄŤas, aby Ĺˇlo odliĹˇit novĂ˝ pĂˇd od starĹˇĂ­ho zĂˇznamu.
4. V koĹ™eni repozitĂˇĹ™e spusĹĄ nĂˇsledujĂ­cĂ­ pĹ™Ă­kaz. Cestu uprav podle svĂ© instalace ADB.

```powershell
python capture_android_crash.py --adb "C:/Android/platform-tools/adb.exe" --output "myboy-crash"
```

Skript vyĹľaduje prĂˇvÄ› jedno pĹ™ipojenĂ© a autorizovanĂ© zaĹ™Ă­zenĂ­. PĹ™i vĂ­ce zaĹ™Ă­zenĂ­ch odpoj ostatnĂ­ nebo pouĹľij `--serial "VYBRANE_ZARIZENI"`. Skript tento identifikĂˇtor nezobrazuje ani neuklĂˇdĂˇ.

Skript rozpoznĂˇvĂˇ balĂ­ÄŤky `com.fastemulator.gba` a `com.fastemulator.gbafree`. Pokud jsou nainstalovanĂ© oba, skonÄŤĂ­ bez ÄŤtenĂ­ logu. Vyber sprĂˇvnĂ˝ balĂ­ÄŤek pomocĂ­ `--package com.fastemulator.gba` nebo `--package com.fastemulator.gbafree`. BalĂ­ÄŤky My OldBoy! pro GBC se nikdy nevyberou automaticky.

## VĂ˝stup

VĂ˝chozĂ­ novĂˇ sloĹľka mĂˇ podobu `outputs/myboy/android-crash-RRRRMMDDTHHMMSSZ`. ExistujĂ­cĂ­ sloĹľku skript nepĹ™epĂ­Ĺˇe. Jinou novou sloĹľku mĹŻĹľeĹˇ zadat pĹ™es `--output`.

- `myboy-crash-only.log` obsahuje pouze odpovĂ­dajĂ­cĂ­ nativnĂ­ nebo Java pĂˇdy. Pokud ĹľĂˇdnĂ˝ nenajde, tento soubor nevytvoĹ™Ă­.
- `summary.json` obsahuje poÄŤty odpovĂ­dajĂ­cĂ­ch blokĹŻ a prĂˇvÄ› zjiĹˇtÄ›nĂ© `versionName` a `versionCode` vybranĂ© aplikace. NeznĂˇmou verzi oznaÄŤĂ­ jako `null`. Jde o verzi pĹ™i sbÄ›ru; pĹ™i pozdÄ›jĹˇĂ­ aktualizaci aplikace nemusĂ­ odpovĂ­dat starĹˇĂ­mu pĂˇdu.

Skript pĹ™eÄŤte `logcat -b crash -d -v long`. CelĂ˝ naÄŤtenĂ˝ buffer zĹŻstĂˇvĂˇ pouze v pamÄ›ti poÄŤĂ­taÄŤe. PĹ™ed zĂˇpisem rozdÄ›lĂ­ zĂˇznamy podle logovacĂ­ho procesu a jeho vlĂˇkna. NativnĂ­ blok musĂ­ obsahovat identitu `>>> pĹ™esnĂ˝.balĂ­ÄŤek <<<`; Java blok musĂ­ obsahovat Ĺ™Ăˇdek `Process: pĹ™esnĂ˝.balĂ­ÄŤek, PID: ...`. Filtr zachovĂˇ zaÄŤĂˇtek nativnĂ­ho bloku, registry a celĂ˝ dostupnĂ˝ backtrace. ProklĂˇdanĂ© zĂˇznamy jinĂ˝ch procesĹŻ vyĹ™adĂ­. Firmware fingerprint, model a sĂ©riovĂ© Ăşdaje ze znĂˇmĂ˝ch metadat odstranĂ­. ZĂˇmÄ›rnÄ› nepouĹľĂ­vĂˇ `--pid`: nativnĂ­ vĂ˝pis mĹŻĹľe vytvĂˇĹ™et jinĂ˝ proces neĹľ samotnĂˇ aplikace.

Skript dĂˇle pĹ™eÄŤte `dumpsys package` pouze pro vybranĂ˝ balĂ­ÄŤek. Z tohoto vĂ˝pisu uloĹľĂ­ jen dvÄ› pole verze. NeuloĹľĂ­ celĂ˝ vĂ˝pis balĂ­ÄŤku ani seznam zaĹ™Ă­zenĂ­.

## OmezenĂ­ a mĂ­stnĂ­ testy

PrĂˇzdnĂ˝ vĂ˝sledek nevyluÄŤuje pĂˇd ani zamrznutĂ­. Buffer mĹŻĹľe obsahovat jen starĹˇĂ­ udĂˇlosti, zĂˇznam mohl bĂ˝t pĹ™epsĂˇn nebo systĂ©m pĹ™i danĂ©m problĂ©mu ĹľĂˇdnĂ˝ pĂˇd nezaznamenal. Skript neÄŤte ostatnĂ­ systĂ©movĂ© buffery ani ANR zĂˇznamy. Blok bez pĹ™esnĂ© identity, s neznĂˇmĂ˝m formĂˇtem hlaviÄŤky nebo se sufixem procesu `:...` radÄ›ji neuloĹľĂ­.

SyntetickĂ© testy nevyĹľadujĂ­ ADB a nekomunikujĂ­ s telefonem:

```powershell
python -m unittest discover -s tools/myboy -p test_capture_android_crash.py -v
```

Testy ovÄ›Ĺ™ujĂ­ nativnĂ­ a Java pĂˇdy, jinĂ˝ PID procesu `crash_dump32`, proklĂˇdanĂ© cizĂ­ aplikace, vĂ­ce pĂˇdĹŻ se stejnĂ˝m emitujĂ­cĂ­m procesem, pĹ™esnĂ© nĂˇzvy balĂ­ÄŤkĹŻ, vĂ˝bÄ›r zaĹ™Ă­zenĂ­ a verze aplikace. ZkouĹˇka na skuteÄŤnĂ©m telefonu zatĂ­m neprobÄ›hla.

Podklady: [formĂˇt a buffery Logcat](https://developer.android.com/tools/logcat), [USB a autorizace ADB](https://developer.android.com/tools/adb), [nativnĂ­ pĂˇdy Androidu](https://source.android.com/docs/core/tests/debug/native-crash), [cĂ­lenĂ© diagnostickĂ© vĂ˝pisy dumpsys](https://developer.android.com/tools/dumpsys).
