# Fotoeffektus vizsgálata glimmlámpával

Ez a projekt egy egyszerű és olcsón megvalósítható kísérleti eszközt mutat be a **fényelektromos jelenség glimmlámpával történő vizsgálatához**.

A kísérlet alapja, hogy a glimmlámpára a gyújtási feszültségéhez közeli feszültséget kapcsolunk. A lámpa katódjának megfelelő hullámhosszú fénnyel történő megvilágítása csökkenti a kisülés megindulásához szükséges feszültséget. A hatás függ a megvilágító fény hullámhosszától és intenzitásától.

A projekt célja nem csupán a jelenség kvalitatív bemutatása, hanem annak **mérhető és reprodukálható vizsgálata**.

## A mérőrendszer

A mérőáramkör egy glimmlámpát tartalmazó relaxációs oszcillátoron alapul. A lámpa feszültségét nagy impedanciájú feszültségosztón keresztül egy **ESP32 mikrovezérlő** méri.

A rendszer automatikusan meghatározza a glimmlámpa gyújtási feszültségét, így vizsgálható annak változása különböző megvilágítási körülmények között.

A glimmlámpa periodikus felvillanását hangjelzés is kíséri. Megvilágítás hatására a villanások és a hangjelzések ritmusa megváltozik, ezért a fotoeffektus következménye egyszerre válik **láthatóvá, hallhatóvá és mérhetővé**.

## Vizsgálható jelenségek

A berendezéssel többek között vizsgálható:

* a glimmlámpa gyújtási és kialvási feszültsége;
* a gyújtási feszültség mérésének reprodukálhatósága;
* a megvilágítás hatása a gyújtási feszültségre;
* a hatás hullámhosszfüggése különböző LED-ekkel és lézerekkel;
* a fényintenzitás hatása;
* a katód és az anód célzott megvilágításának különbsége;
* a lámpa polaritásának és előéletének hatása.

A mérési adatok számítógépen is rögzíthetők és statisztikailag kiértékelhetők.

## A repository tartalma

A repository a kísérleti eszköz reprodukálásához és továbbfejlesztéséhez szükséges anyagokat tartalmazza:

* ESP32 firmware;
* [kapcsolási rajz](Hardver/Schematic_Glimm_2026-08-24.pdf);
* [NYÁK-terv és gyártási fájlok](Hardver/Gerber_Glimm_PCB_Glimm_2024-05-13.zip);
* alkatrészlista;
* mérési és adatgyűjtési segédanyagok;
* a kísérletekhez kapcsolódó dokumentáció.

## Oktatási alkalmazás

A kísérlet különösen alkalmas középiskolai és bevezető egyetemi fizikaoktatásra.

A klasszikus fotoeffektus-kísérletekkel szemben nincs szükség speciális fotocellára vagy nagy értékű mérőberendezésre. A glimmlámpa, néhány egyszerű elektronikai alkatrész és egy ESP32 segítségével látványos, ugyanakkor kvantitatív mérésre is alkalmas rendszer építhető.

A kísérlet összekapcsolja

**a fotoeffektust → a gázkisülések fizikáját → az elektronikai méréstechnikát → a mikrovezérlős adatgyűjtést → a statisztikai adatfeldolgozást.**

## A kísérlet részletes leírása

A fizikai háttér, a mérési eljárás, valamint a kísérleti eredmények részletes ismertetése itt található:

**A fényelektromos jelenség bemutatása glimmlámpával**
https://pilath.wordpress.com/2023/08/21/a-fenyelektromos-jelenseg-bemutatasa-glimmlampaval/

## Megjegyzés

A glimmlámpák tulajdonságai típusonként és példányonként is eltérhetnek. Kvantitatív mérések esetén ezért célszerű ugyanazt a lámpát használni, a mérési körülményeket rögzíteni, valamint több mérésből átlagot és szórást meghatározni.

A projekt elsősorban **oktatási és kísérleti célokat** szolgál.
