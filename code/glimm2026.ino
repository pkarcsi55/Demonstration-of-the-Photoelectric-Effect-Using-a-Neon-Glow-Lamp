// Glimmlámpás demonstrációs eszköz a fényelektromos jelenség tanulmányozásához
// Hardver: WeMos LOLIN32 Lite (ESP32)
// Programverzió: 1.1
// Dátum: 2026.08.30.
// Készítette: Piláth Károly.

#include <math.h>
#include <Preferences.h>

// Hardvercsatlakozások
const uint8_t MERES_GOMB_PIN = 32;
const uint8_t FESZULTSEG_BEMENET_PIN = 27;
const uint8_t NAGYFESZULTSEG_JEL_PIN = 25;
const uint8_t CSIPOGO_PIN = 33;

// Mérési és hangjelzési beállítások
const uint16_t MERESI_IDOKOZ_MS = 20;  // 50 Hz-es soros adatküldés
const uint8_t ATLAGOLT_MINTAK_SZAMA = 16;
const float ADC_REFERENCIA_FESZULTSEG_V = 3.30f;
const float ADC_SZINTEK_SZAMA = 4096.0f;
const float FESZULTSEGOSZTO_ATTETELE = 0.021526f;  // 10 MOhm / 220 kOhm osztó
const float MINIMALIS_KISULESI_ESES_V = 0.50f;
const float KISULESI_ZAJ_SZORZO = 6.0f;
const float MINIMALIS_EMELKEDES_V = 0.02f;
const float EMELKEDESI_ZAJ_SZORZO = 2.0f;
const uint8_t UJRAELESITESHEZ_SZUKSEGES_MINTAK = 3;
const uint16_t CSIPOGO_FREKVENCIA_HZ = 1000;
const uint16_t GOMBJELZO_HANG_HOSSZA_MS = 50;
const uint16_t KISULESI_HANG_HOSSZA_MS = 30;
const uint16_t GOMB_PERGESMENTESITES_MS = 30;
const uint8_t PWM_FELBONTAS_BIT = 8;
const uint16_t ALAPERTELMEZETT_NAGYFESZ_FREKVENCIA_HZ = 500;
const uint16_t MIN_NAGYFESZ_FREKVENCIA_HZ = 300;
const uint16_t MAX_NAGYFESZ_FREKVENCIA_HZ = 700;
const uint8_t STATISZTIKAI_ABLAK_MERETE = 30;

enum class CsucsfigyeloAllapot {
  CSUCSOT_KERES,
  UJ_EMELKEDESRE_VAR
};

Preferences taroltBeallitasok;

uint16_t nagyfeszultsegFrekvenciaHz =
  ALAPERTELMEZETT_NAGYFESZ_FREKVENCIA_HZ;
bool nagyfeszultsegKikapcsolva = true;
bool kisulesiHangAktiv = false;
uint32_t kisulesiHangKezdeteMs = 0;

uint32_t elozoMeresIdejeMs = 0;
uint32_t gombAllapotvaltozasIdejeMs = 0;
bool elozoNyersGombAllapot = HIGH;
bool stabilGombAllapot = HIGH;
float aktualisCsurfeszultsegV = 0.0f;
float gyujtottCsurfeszultsegV = 0.0f;
float legutobbiCsurfeszultsegV = 0.0f;
float elozoCsurfeszultsegV = 0.0f;
bool vanElozoFeszultsegMinta = false;
uint8_t egymasUtaniEmelkedoMintak = 0;
float csucsfeszultsegMintakV[STATISZTIKAI_ABLAK_MERETE] = {};
uint8_t kovetkezoStatisztikaiMintaIndex = 0;
uint8_t statisztikaiMintakSzama = 0;
double csucsfeszultsegOsszegV = 0.0;
double csucsfeszultsegNegyzetOsszegV2 = 0.0;
float csucsfeszultsegAtlagV = 0.0f;
float csucsfeszultsegSzorasV = 0.0f;
CsucsfigyeloAllapot csucsfigyeloAllapot =
  CsucsfigyeloAllapot::UJ_EMELKEDESRE_VAR;

void rovidHangjelzes();
void duplaHangjelzes();
void sorosParancsFeldolgozasa(String parancs);
void csucsfeszultsegHozzaadasaAStatisztikahoz(float csucsfeszultsegV);
void statisztikaNullazasa();

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }

  taroltBeallitasok.begin("glimm", false);
  nagyfeszultsegFrekvenciaHz = taroltBeallitasok.getUShort(
    "hv_freq",
    ALAPERTELMEZETT_NAGYFESZ_FREKVENCIA_HZ
  );

  if (nagyfeszultsegFrekvenciaHz < MIN_NAGYFESZ_FREKVENCIA_HZ ||
      nagyfeszultsegFrekvenciaHz > MAX_NAGYFESZ_FREKVENCIA_HZ) {
    nagyfeszultsegFrekvenciaHz = ALAPERTELMEZETT_NAGYFESZ_FREKVENCIA_HZ;
    taroltBeallitasok.putUShort("hv_freq", nagyfeszultsegFrekvenciaHz);
  }

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);  // Jelzi, hogy a program elindult.

  pinMode(CSIPOGO_PIN, OUTPUT);
  pinMode(NAGYFESZULTSEG_JEL_PIN, OUTPUT);

  // ESP32 Arduino Core 3.x LEDC API: pin, frekvencia, felbontás.
  ledcAttach(
    NAGYFESZULTSEG_JEL_PIN,
    nagyfeszultsegFrekvenciaHz,
    PWM_FELBONTAS_BIT
  );
  ledcAttach(CSIPOGO_PIN, CSIPOGO_FREKVENCIA_HZ, PWM_FELBONTAS_BIT);

  pinMode(MERES_GOMB_PIN, INPUT_PULLUP);
  delay(200);

  elozoNyersGombAllapot = digitalRead(MERES_GOMB_PIN);
  stabilGombAllapot = elozoNyersGombAllapot;

  elozoMeresIdejeMs = millis();
}

void loop() {
  if (kisulesiHangAktiv &&
      millis() - kisulesiHangKezdeteMs >= KISULESI_HANG_HOSSZA_MS) {
    ledcWriteTone(CSIPOGO_PIN, 0);
    kisulesiHangAktiv = false;
  }

  // Frekvenciabeállítás: F300 ... F700, sorvége karakterrel lezárva.
  if (Serial.available() > 0) {
    String beolvasottParancs = Serial.readStringUntil('\n');
    sorosParancsFeldolgozasa(beolvasottParancs);
  }

  // Pergésmentesített állapotfigyelés. A kapcsolás csak a gomb stabil
  // lenyomásakor történik meg; nyomva tartás közben nem ismétlődik.
  const bool nyersGombAllapot = digitalRead(MERES_GOMB_PIN);

  if (nyersGombAllapot != elozoNyersGombAllapot) {
    gombAllapotvaltozasIdejeMs = millis();
    elozoNyersGombAllapot = nyersGombAllapot;
  }

  if (millis() - gombAllapotvaltozasIdejeMs >= GOMB_PERGESMENTESITES_MS &&
      nyersGombAllapot != stabilGombAllapot) {
    stabilGombAllapot = nyersGombAllapot;

    if (stabilGombAllapot == LOW) {
      nagyfeszultsegKikapcsolva = !nagyfeszultsegKikapcsolva;

      // Kapcsoláskor új mérési ciklust kezdünk, ezért egy be- vagy kikapcsolási
      // feszültségugrás nem minősül glimmlámpa-kisülésnek.
      vanElozoFeszultsegMinta = false;
      gyujtottCsurfeszultsegV = 0.0f;
      egymasUtaniEmelkedoMintak = 0;
      csucsfigyeloAllapot = CsucsfigyeloAllapot::UJ_EMELKEDESRE_VAR;

      if (nagyfeszultsegKikapcsolva) {
        ledcWriteTone(NAGYFESZULTSEG_JEL_PIN, 0);
        rovidHangjelzes();       // OFF: egy rövid hang
      } else {
        ledcWriteTone(
          NAGYFESZULTSEG_JEL_PIN,
          nagyfeszultsegFrekvenciaHz
        );
        duplaHangjelzes();       // ON: két rövid hang
      }
    }
  }

  const uint32_t aktualisIdoMs = millis();
  if (aktualisIdoMs - elozoMeresIdejeMs >= MERESI_IDOKOZ_MS) {
    elozoMeresIdejeMs = aktualisIdoMs;

    uint32_t adcOsszeg = 0;
    uint64_t adcNegyzetOsszeg = 0;
    for (uint8_t mintaIndex = 0; mintaIndex < ATLAGOLT_MINTAK_SZAMA; mintaIndex++) {
      const uint16_t adcMinta = analogRead(FESZULTSEG_BEMENET_PIN);
      adcOsszeg += adcMinta;
      adcNegyzetOsszeg += static_cast<uint32_t>(adcMinta) * adcMinta;
    }

    const float atlagosAdcErtek =
      static_cast<float>(adcOsszeg) / ATLAGOLT_MINTAK_SZAMA;

    const float mertFeszultsegV =
      ADC_REFERENCIA_FESZULTSEG_V * atlagosAdcErtek / ADC_SZINTEK_SZAMA;

    aktualisCsurfeszultsegV = mertFeszultsegV / FESZULTSEGOSZTO_ATTETELE;

    const float adcNegyzetAtlag =
      static_cast<float>(adcNegyzetOsszeg) / ATLAGOLT_MINTAK_SZAMA;
    float adcVariancia =
      adcNegyzetAtlag - atlagosAdcErtek * atlagosAdcErtek;
    if (adcVariancia < 0.0f) {
      adcVariancia = 0.0f;
    }

    const float adcSzoras = sqrtf(adcVariancia);
    const float egyAdcSzintLampafeszultsegV =
      ADC_REFERENCIA_FESZULTSEG_V /
      ADC_SZINTEK_SZAMA /
      FESZULTSEGOSZTO_ATTETELE;

    // A 16 minta átlagának zajából becsüljük két egymást követő mérési pont
    // különbségének szórását.
    const float atlagZajSzorasV =
      adcSzoras /
      sqrtf(static_cast<float>(ATLAGOLT_MINTAK_SZAMA)) *
      egyAdcSzintLampafeszultsegV;
    const float ketMintaKulonbsegZajSzorasV = sqrtf(2.0f) * atlagZajSzorasV;

    const float zajAlapuKisulesiKuszobV =
      KISULESI_ZAJ_SZORZO * ketMintaKulonbsegZajSzorasV;
    const float adaptivKisulesiKuszobV =
      zajAlapuKisulesiKuszobV > MINIMALIS_KISULESI_ESES_V
        ? zajAlapuKisulesiKuszobV
        : MINIMALIS_KISULESI_ESES_V;

    const float zajAlapuEmelkedesiKuszobV =
      EMELKEDESI_ZAJ_SZORZO * ketMintaKulonbsegZajSzorasV;
    const float adaptivEmelkedesiKuszobV =
      zajAlapuEmelkedesiKuszobV > MINIMALIS_EMELKEDES_V
        ? zajAlapuEmelkedesiKuszobV
        : MINIMALIS_EMELKEDES_V;

    if (!vanElozoFeszultsegMinta) {
      elozoCsurfeszultsegV = aktualisCsurfeszultsegV;
      gyujtottCsurfeszultsegV = aktualisCsurfeszultsegV;
      vanElozoFeszultsegMinta = true;
    } else {
      const float feszultsegValtozasV =
        aktualisCsurfeszultsegV - elozoCsurfeszultsegV;

      if (csucsfigyeloAllapot == CsucsfigyeloAllapot::CSUCSOT_KERES) {
        if (aktualisCsurfeszultsegV > gyujtottCsurfeszultsegV) {
          gyujtottCsurfeszultsegV = aktualisCsurfeszultsegV;
        }

        const float visszaesesACsucstolV =
          gyujtottCsurfeszultsegV - aktualisCsurfeszultsegV;

        if (visszaesesACsucstolV > adaptivKisulesiKuszobV) {
          legutobbiCsurfeszultsegV = gyujtottCsurfeszultsegV;
          csucsfeszultsegHozzaadasaAStatisztikahoz(
            legutobbiCsurfeszultsegV
          );
          csucsfigyeloAllapot = CsucsfigyeloAllapot::UJ_EMELKEDESRE_VAR;
          egymasUtaniEmelkedoMintak = 0;

          ledcWriteTone(CSIPOGO_PIN, CSIPOGO_FREKVENCIA_HZ);
          kisulesiHangKezdeteMs = millis();
          kisulesiHangAktiv = true;
        }
      } else {
        if (feszultsegValtozasV > adaptivEmelkedesiKuszobV) {
          egymasUtaniEmelkedoMintak++;
        } else {
          egymasUtaniEmelkedoMintak = 0;
        }

        if (egymasUtaniEmelkedoMintak >= UJRAELESITESHEZ_SZUKSEGES_MINTAK) {
          csucsfigyeloAllapot = CsucsfigyeloAllapot::CSUCSOT_KERES;
          gyujtottCsurfeszultsegV = aktualisCsurfeszultsegV;
          egymasUtaniEmelkedoMintak = 0;
        }
      }

      elozoCsurfeszultsegV = aktualisCsurfeszultsegV;
    }

    Serial.print(aktualisCsurfeszultsegV, 2);
    Serial.print(",");
    Serial.print(legutobbiCsurfeszultsegV, 2);
    Serial.print(",");
    Serial.print(csucsfeszultsegAtlagV, 2);
    Serial.print(",");
    Serial.println(csucsfeszultsegSzorasV, 2);
  }
}

void rovidHangjelzes() {
  ledcWriteTone(CSIPOGO_PIN, CSIPOGO_FREKVENCIA_HZ);
  delay(GOMBJELZO_HANG_HOSSZA_MS);
  ledcWriteTone(CSIPOGO_PIN, 0);
}

void duplaHangjelzes() {
  rovidHangjelzes();
  delay(GOMBJELZO_HANG_HOSSZA_MS);
  rovidHangjelzes();
}

void sorosParancsFeldolgozasa(String parancs) {
  parancs.trim();

  // Az R parancs egy új mérési sorozathoz nullázza a statisztikát.
  if (parancs.equalsIgnoreCase("R")) {
    statisztikaNullazasa();
    return;
  }

  // A megengedett alak pontosan Fxxx vagy fxxx, ahol xxx három számjegy.
  if (parancs.length() != 4 ||
      (parancs.charAt(0) != 'F' && parancs.charAt(0) != 'f')) {
    return;
  }

  for (uint8_t karakterIndex = 1; karakterIndex < 4; karakterIndex++) {
    if (!isDigit(parancs.charAt(karakterIndex))) {
      return;
    }
  }

  const uint16_t ujFrekvenciaHz = parancs.substring(1).toInt();
  if (ujFrekvenciaHz < MIN_NAGYFESZ_FREKVENCIA_HZ ||
      ujFrekvenciaHz > MAX_NAGYFESZ_FREKVENCIA_HZ) {
    return;
  }

  if (ujFrekvenciaHz != nagyfeszultsegFrekvenciaHz) {
    nagyfeszultsegFrekvenciaHz = ujFrekvenciaHz;
    taroltBeallitasok.putUShort("hv_freq", nagyfeszultsegFrekvenciaHz);
  }

  // Kikapcsolt állapotban csak eltároljuk az értéket. A nagyfeszültséget
  // továbbra is kizárólag a nyomógomb kapcsolhatja be.
  if (!nagyfeszultsegKikapcsolva) {
    ledcWriteTone(NAGYFESZULTSEG_JEL_PIN, nagyfeszultsegFrekvenciaHz);
  }
}

void csucsfeszultsegHozzaadasaAStatisztikahoz(float csucsfeszultsegV) {
  if (statisztikaiMintakSzama == STATISZTIKAI_ABLAK_MERETE) {
    const float kiesőCsucsfeszultsegV =
      csucsfeszultsegMintakV[kovetkezoStatisztikaiMintaIndex];
    csucsfeszultsegOsszegV -= kiesőCsucsfeszultsegV;
    csucsfeszultsegNegyzetOsszegV2 -=
      static_cast<double>(kiesőCsucsfeszultsegV) * kiesőCsucsfeszultsegV;
  } else {
    statisztikaiMintakSzama++;
  }

  csucsfeszultsegMintakV[kovetkezoStatisztikaiMintaIndex] = csucsfeszultsegV;
  kovetkezoStatisztikaiMintaIndex =
    (kovetkezoStatisztikaiMintaIndex + 1) % STATISZTIKAI_ABLAK_MERETE;

  csucsfeszultsegOsszegV += csucsfeszultsegV;
  csucsfeszultsegNegyzetOsszegV2 +=
    static_cast<double>(csucsfeszultsegV) * csucsfeszultsegV;

  // Csak a teljes, 30 csúcsot tartalmazó ablakot jelenítjük meg.
  if (statisztikaiMintakSzama < STATISZTIKAI_ABLAK_MERETE) {
    csucsfeszultsegAtlagV = 0.0f;
    csucsfeszultsegSzorasV = 0.0f;
    return;
  }

  const double mintakSzama = STATISZTIKAI_ABLAK_MERETE;
  const double atlagV = csucsfeszultsegOsszegV / mintakSzama;
  double mintaVarianciaV2 =
    (csucsfeszultsegNegyzetOsszegV2 -
     csucsfeszultsegOsszegV * csucsfeszultsegOsszegV / mintakSzama) /
    (mintakSzama - 1.0);

  if (mintaVarianciaV2 < 0.0) {
    mintaVarianciaV2 = 0.0;
  }

  csucsfeszultsegAtlagV = static_cast<float>(atlagV);
  csucsfeszultsegSzorasV = static_cast<float>(sqrt(mintaVarianciaV2));
}

void statisztikaNullazasa() {
  kovetkezoStatisztikaiMintaIndex = 0;
  statisztikaiMintakSzama = 0;
  csucsfeszultsegOsszegV = 0.0;
  csucsfeszultsegNegyzetOsszegV2 = 0.0;
  csucsfeszultsegAtlagV = 0.0f;
  csucsfeszultsegSzorasV = 0.0f;
}
