## Bill of Materials (BOM)

The circuit uses only inexpensive and commonly available components.
Equivalent parts may be substituted where appropriate.

| Ref.   | Component             |               Value / Type | Qty. | Notes                                              |
| ------ | --------------------- | -------------------------: | ---: | -------------------------------------------------- |
| C1, C2 | Ceramic capacitor     |             10 nF / ≥400 V |    2 | High-voltage side                                  |
| C3, C4 | Capacitor             |                     820 nF |    2 | Timing capacitors                                  |
| C5     | Ceramic capacitor     |                      10 nF |    1 | Vishay A103K15X7RF5TAA or equivalent               |
| D1–D3  | Rectifier diode       |                      FR157 |    3 | DO-15                                              |
| GL1    | Neon glow lamp        |                          – |    1 | Experimental light-sensitive glow lamp             |
| L1     | Inductor              |                    4.7 mH* |    1 | *Value should be checked against the assembled PCB |
| R1     | Resistor              |                       10 Ω |    1 |                                                    |
| R2     | Resistor              |                      20 MΩ |    1 | May be implemented as 2 × 10 MΩ in series          |
| R3     | Resistor              |                      10 MΩ |    1 |                                                    |
| R4     | Resistor              |                     220 kΩ |    1 |                                                    |
| R6     | Resistor              |                       1 kΩ |    1 |                                                    |
| S1     | SPDT switch           |                          – |    1 |                                                    |
| SW1    | Push button           | TS-1103S-C-K or equivalent |    1 |                                                    |
| U1     | Microcontroller board |         Wemos LOLIN32 Lite |    1 | ESP32                                              |
| U2     | NPN transistor        |                     MPSA42 |    1 | High-voltage transistor, TO-92                     |
| U3     | Li-ion cell           |                      14500 |    1 | Rechargeable cell                                  |
| U4     | Piezo buzzer          |                          – |    1 | Acoustic indication                                |
| J1–J4  | Pin / wire connection |                    2.54 mm |    4 | Battery and switch connections                     |

### Selected supplier part numbers

Where known, the following parts were used in the prototype:

| Component | Manufacturer / Supplier | Part number      |
| --------- | ----------------------- | ---------------- |
| C1, C2    | ReliaPro                | C3709 / 103M-2KV |
| C5        | Vishay                  | A103K15X7RF5TAA  |
| SW1       | XKB Connectivity        | TS-1103S-C-K     |
| U2        | EIC                     | MPSA42           |
| U1        | Wemos                   | LOLIN32 Lite     |

The exact component types are generally **not critical**, except where voltage rating, resistance value or mechanical dimensions affect the operation of the circuit. In particular, capacitors connected to the high-voltage section must have an appropriate voltage rating.

The original BOM exported from the PCB design software is also included in this repository.
