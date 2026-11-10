# E90-DTU (433L37)-V8 User Manual (EBYTE) - transcripcion literal

pdf: evidencia/091120261523/Saf0e770220924608a9744716c546324aB.pdf
sha256: 7c5d68651561ec3e4618b59cd165d7cb22e1fe5fb33c8aa4f1a27226a7de9cfd

Copia de trabajo. Para citar, usar numeral y pagina del manual: "E90-DTU (433L37)-V8 User Manual, numeral 6, p. 10".
El texto es el del original en ingles, transcrito tal cual, con sus erratas. Las figuras no se copian: se indican.

## Ficha

| Campo | Valor |
|---|---|
| Titulo | E90-DTU (433L37)-V8 User Manual (Wireless Modem) |
| Modelo | E90-DTU (433L37)-V8 (transceptor de datos 433 MHz, RS232/RS485) |
| Fabricante | Chengdu Ebyte Electronic Technology Co., Ltd. (EBYTE) |
| Edicion | Copyright 2025; revision history: V1.0 (LC), V1.1 (Hao) |
| Procedencia | PDF entregado por el responsable el 09/11/2026, desde la ficha de compra: `evidencia/091120261523/Saf0e770220924608a9744716c546324aB.pdf` |
| SHA-256 del PDF | 7c5d68651561ec3e4618b59cd165d7cb22e1fe5fb33c8aa4f1a27226a7de9cfd |
| Paginas del PDF | 20: portada, indice (p. I), aviso (p. II) y manual (pp. 1-17) |
| Numeracion | `[p. N]` es el numero impreso del manual. Pagina del PDF = impresa + 3 |
| Nota | Copia de trabajo. No sustituye al original; para citar, numeral y pagina del manual. |

## Portada

Wireless Modem - User Manual - E90-DTU (433L37)-V8. Imagen del modulo con etiqueta "Data Transceiver"
(sin numero de pagina).
"All rights to interpret and modify this manual belong to Chengdu Ebyte Electronic Technology Co.,Ltd."

## Indice [p. I]

Figura: tabla de contenidos del manual (numerales 1 a 11, Revision history y About us, con paginas).
Transcripcion de los titulos:

- 1. Introduction ........ 1; 1.1. Brief Introduction ........ 1; 1.2. Features ........ 1
- 2. Operation ........ 2
- 3. Installation Specifications ........ 4; 3.1. Structure ........ 4; 3.2. Dimension ........ 6
- 4. Interface Defination ........ 7; 4.1. Power interface definition ........ 7;
  4.2. RS232 Interface definition ........ 7; 4.3. RS485 Interface definition ........ 7
- 5. Technical indicators ........ 8; 5.1. Model specifications ........ 8; 5.2. General specification
  parameters ........ 8; 5.3. Frequency range and channels ........ 9; 5.4. Transmit power level ........ 9;
  5.5. Air data rate ........ 9; 5.6. Current parameters ........ 9;
  5.7. Transceiver Length and Sub-packing Mode ........ 10
- 6. Operating mode ........ 10
- 7. Program the DTU ........ 11; 7.1. diagrammatic drawing ........ 11; 7.2. Parameter setting instruction ........ 12
- 8. Connection diagram in test and application ........ 13
- 9. Related Products ........ 13
- 10. Practical application ........ 14
- 11. Precautions for Use ........ 15
- Revision history ........ 16
- About us ........ 错误!未定义书签。 (texto de error de Word en el original; la pagina impresa es 17)

## Aviso (Disclaimer) [p. II]

EBYTE reserves all rights to this document and the information contained herein. Products, names, logos and
designs described herein may in whole or in part be subject to intellectual property rights. Reproduction, use,
modification or disclosure to third parties of this document or any part thereof without the express permission of
EBYTE is strictly prohibited.

The information contained herein is provided "as is" and EBYTE assumes no liability for the use of the information.
No warranty, either express or implied, is given, including but not limited to, with respect to the accuracy,
correctness, reliability and fitness for a particular purpose of the information. This document may be revised by
EBYTE at any time. For most recent documents, visit www.cdebyte.com.

## 1. Introduction [p. 1]

### 1.1. Brief Introduction [p. 1]

E90-DTU(433L37)-V8 is a wireless data transceiver of 433M with standard RS232/RS485 connectors. They are
half-duplex. TX & RX modems with LoRa technology and transparent transmission mode. Voltage supply ranges from
8V to 28V, Working frequency: 410~441MHz (Default:433MHz).

The LoRa direct sequence spread spectrum technology enables the longer communication distance and better power
density concentration as well as superior anti-interference ability. The FEC algorism enables higher coding
efficiency and correction ability. The interfered data packets will be corrected proactively upon sudden
interference, which significantly improves reliability and communication distance. Without FEC, the interfered data
packet will be dropped. The transceivers feature data encryption and compression. The data transmitted in air
features randomness, the rigorous algorism makes data interception meaningless. The data compression function has
possibility to reduce the data transmission time, which in turn reduces the possibility of being interfered, thus
improves the reliability and communication efficiency.

### 1.2. Features [p. 1]

- All core components are originally imported, our transceiver modems have much advanced functions with smaller
  size and lower cost.
- The top TX power is 100mW, all technical parameters meet European industrial standards.
- Temperature compensators are adopted to make the frequency stability better than ±1.5PPM.
- Operation temperature range: -40°C ~ +85°C, applicable for various harsh environment, it is real industrial grade
  products.
- Aluminum alloy case, compact size, great heat dispersion; good shielding, prime electromagnetic compatibility and
  strong anti-interference.
- Power reverse & overload protection and antenna surge protection functions significantly improve the reliability.
- Parameters can be configured by programming, such as TX power, frequency point, air data rate, address and so on.
- Ultra-low power consumption, standby current is only 57mA (even lower under power-saving and sleep modes).
- Embedded watch-dog and precise time layout, modem will restart automatically upon abnormal situation and work
  with previous parameters.
- All the core components are imported originally, compared with the current imports of digital transceiver, we are
  the most advanced, most cost effective and the smallest one.

## 2. Operation [p. 2]

Figura: "Main parts" - cuatro fotos: DTU, Power Adapter, antenna, Serial converter.

1、 First step is to mount antenna, then battery, making sure the dial switch is on its right status. User gets on
the power by choosing either VCC/GND or power adapter.
Figura: vista trasera y lateral del equipo, con los terminales VCC/GND y la toma DC marcados en rojo.

2、 Using USB-(RS232) converter or USB-(RS485) converter or other way to link computer and DTU.
Figura: PC conectado a dos DTU, cada uno con su antena.

3、 Firing up two XCOMs, choosing Baud rate 9600bps, 8N1, the setting which serial port transmission can be
achieved. [p. 2-3]
Figura: dos ventanas XCOM V2.6 (9600, 8N1, Open).

4、 User needs to open the mode switch first before link DTU with computer if the user want to modify parameters.
Firing up to configuration Tool modify related parameters. The mode switch must be reopened to achieve
transmission after the configuration. [p. 3]
Figura: dos fotos del conmutador DIP: "MODE 0 Default" y "MODE 3 Parameter setting".

## 3. Installation Specifications [p. 4]

### 3.1. Structure [p. 4]

Figura: vistas frontal y lateral con callouts ① a ⑧, y etiquetas RS232, 485_A, 485_B, GND, VCC, PWR, TXD, RXD,
DC, M0, M1, ANT.

Tabla de pines [p. 4-5]:

| Pin NO. | Name | Function | Description |
|---|---|---|---|
| 1 | DB-9 female socket | RS-232 interface | Standard RS-232 interface |
| 2 | 3.81 terminal block | RS-485, power interface | Standard RS-485 interface and pressure line power interface |
| 3 | PWR-LED | Power LED | Red, lit when the power is on |
| 4 | TXD-LED | Transmit LED | Yellow, blinks when sending data |
| 5 | RXD-LED | Receive LED | Yellow, blinks when receiving data |
| 6 | DC power interface | Power interface | In-line round hole, outer diameter 5.5mm, diameter 2.5mm |
| 7 | DIP switch | DIP switch | Controlled by working mode |
| 8 | Antenna interface | SMA-K interface | external thread, 10mm, 50Ωcharacteristic impedance |

### 3.2. Dimension [p. 6]

Figura: dibujo acotado (vista superior, laterales y frontales). Unidad: mm. Cotas visibles incluyen 102, 45, 105,
95, 79, 37.3, 30.5, 19.5, 16, 15.9, 13, 12.8, 14.2, 25, 11, 10.4, 10, 9, 5.

## 4. Interface Defination [p. 7]

### 4.1. Power interface definition [p. 7]

Figura: mismos callouts ① a ⑧ que en 3.1.

Users can choose ⑥ DC power interface, using the power adapter supply with the interface of the 5.5mm outer
diameter , 2.5mm diameter ;
Also choose the VCC and GND terminal power supply, only choose any one of the power supply is OK;
DTU can use 8~ 28V DC power supply, but it is recommended to use 12V or 24V DC power supply.

### 4.2. RS232 Interface definition [p. 7]

The DTU can be connected to the device via RS-232 using the standard DB-9 interface.

### 4.3. RS485 Interface definition [p. 7-8]

DTU can connect the 485_A terminal and 485_B terminal with the device RS-485 A terminal and B terminal.
Figura: bornera de 4 pines numerados 1 a 4.

| Pin NO. | Definition | Function | Description |
|---|---|---|---|
| 1 | VCC | Crimping power interface, positive | 8 ~ 28V DC, recommended 12V or 24V |
| 2 | GND | Crimping power interface, negative | The power supply negative pole is connected to the system ground and the housing |
| 3 | 485_B | RS-485 interface, interface B | The RS-485 interface B is connected to the device interface B |
| 4 | 485_A | RS-485 interface, interface A | The RS-485 interface A is connected to the device interface A. |

★ Note: The transceiver will be in poor communication when connected to multiple devices , it is recommended to be
connected to a single device, please try to use parallel 120Ω resistor between 485_A terminal and 485_B.

## 5. Technical indicators [p. 8]

### 5.1. Model specifications [p. 8]

| Model | Frequency (Hz) | Transmit power (W) | Distance (km) | Specifications | Application |
|---|---|---|---|---|---|
| E90-DTU (433L37)-V8 | 433 | 0.1 | 3 | Strong penetration, anti - interference | To the environment with small data, long distance, many obstacles |

★ Note: Test condition: in clear and open air without shelters, 12V /2A power supply, 5dBi gain sucker antenna over 2
meters height from the ground, with the factory default parameters.

### 5.2. General specification parameters [p. 8]

| NO. | Model | Specification | Description |
|---|---|---|---|
| 1 | Size（H*W*D） | 82 * 62 * 25 mm | See more at 3.2 Dimension |
| 2 | Weight | 131g | Tolerance: 4.5g |
| 3 | Temperature | -40°C～+70°C | Meet industrial level |
| 4 | Antenna impedance | 50 Ω | Standard 50 Ω characteristic impedance |
| 5 | Supply voltage | +8 ~ +28V DC | It is recommended to use 12V or 24V |
| 6 | Communication interface | RS232/RS485 | Standard DB9 hole / 3.81 terminal block |
| 7 | Baud rate | Default 9600 | from 1200 to 115200 bps |
| 8 | Address | Default 0 | 65536 configurable addresses |

### 5.3. Frequency range and channels [p. 9]

| Model | Default frequency (MHz) | Frequency range (MHz) | Channel spacing (MHz) | Channels |
|---|---|---|---|---|
| E90-DTU (433L37)-V8 | 433 | 410~441 | 1 | 32, half duplex |

★ Note: In the same area when multiple data transceivers are communicating one to one at the same time, it is
recommended to set the channel spacing between each group of data transceivers at 2MHz or more.

### 5.4. Transmit power level [p. 9]

| Model | 135mW | 250mW | 500mW | 1000mW |
|---|---|---|---|---|
| E90-DTU (433L37)-V8 | √ | √ | √ | √ |

★ Note: The lower the transmit power, the closer the transmission distance, but the working current won't be declined
in exact proportion ,it is recommended to use the maximum transmit power.

### 5.5. Air data rate [p. 9]

| Model | Default air data rate (bps) | Levels | Air data rate (bps) |
|---|---|---|---|
| E90-DTU (433L37)-V8 | 2.4 | 6 | 0.3、1.2、2.4、4.8、9.6、19.2 |

★ Note: The higher the air data rate, the faster the transmission rate, the transmission distance is also closer; when
the rate meets the requirements, the lower air data rate, the better quality.

### 5.6. Current parameters [p. 9]

| Model | Transmitting current (mA) 12V | Transmitting current (mA) 24V | Standby current (mA) 12V | Standby current (mA) 24V |
|---|---|---|---|---|
| E90-DTU (433L37)-V8 | 1713 | 814 | 57 | 32 |

★ Note: It is recommended to retain more than 50% of the current margin when selecting the power supply, which will help
the data transceiver to work steadily for a long time.

### 5.7. Transceiver Length and Sub-packing Mode [p. 10]

| Model | Buffer | Sub-package |
|---|---|---|
| E90-DTU (433L37)-V8 | 512Byte | Automatically send 58 bytes per package |

★ Note:
1. When the receiving data is more than a single packet capacity, the beyond part will be automatically assigned to the
   second transmission until it is completed;
2. The data transceiver can not receive data which is more than the buffer capacity;

## 6. Operating mode [p. 10]

DTU has four operating modes, if low power consumption is not required, normal communication is recommended to
configure the data transceiver for the normal mode (mode 0);
The factory default is normal mode (mode 0).

| | Categories | M1 | M0 | Description |
|---|---|---|---|---|
| Mode 0 | Normal Mode | ON | ON | Open UART and RF，transparent transmission is on |
| Mode 1 | Wake-up Mode | ON | OFF | Air wake-up mode, the packet comes with a wake-up code, |
| Mode 2 | Power-saving Mode | OFF | ON | The air wake-up receive mode, saving receive power, the mode can not be transmitted |
| Mode 3 | Sleep Mode | OFF | OFF | Parameter setting using the configuration software |

Figura: cuatro fotos del conmutador DIP en MODE 0, MODE 1, MODE 2 y MODE 3.

★ Note: no need to care about the wake-up mode (mode 1) and power saving mode (mode 2) if it doesn't request low power
consumption.

## 7. Program the DTU [p. 11]

### 7.1. diagrammatic drawing [p. 11]

Figura: PC (o PLC/RTU) con data lines al transceptor; "Connect a 50Ω antenna or a dummy load"; "Pick one of two power
supplies": Power Adapter o 10~28V DC power supply.

| | Mode | M1 | M0 | Description |
|---|---|---|---|---|
| Mode 3 | Sleep Mode | Off | Off | Only be programmed using the configuration software in the current mode |

Figura: foto del conmutador DIP en MODE 3 (ambos OFF).

★ Note:
1.programming can only be carried on in a specific mode(see above), if fails, please confirm the work mode.
2. Open the configuration tool to configure the digital radio.

### 7.2. Parameter setting instruction [p. 12]

Figura: captura de la aplicacion "E90 RF_Setting V2.0" (botones ClosePort, GetParam, SetParam, Preset; campos UartRate,
Parity, Fixed mode, AirRate, Power, Channel, WOR timing, Address). Algunos campos van ocultos en la imagen original.

| Parameter | Details |
|---|---|
| Baud rate | Baud rate when operating 1200bps~115200bps |
| Parity | 8N1:none；8E1: odd；8O1:even；8 data bit，1 stop bit. |
| Air data rate | Wireless communication rate, also called air baud rate. The air rate is high, the data transmission speed is fast, and the time delay for transmitting the same data is small, but the transmission distance will be shortened. |
| Frequency channel | The working frequency of the DTU, each channel corresponds to its different working frequency, in theory, different frequency channels cannot communicate with each other. If there are multiple groups of wireless data transmission stations in the same area, it is recommended that the communication frequency be separated by 2~5MHz. |
| TX power | The output power is the power radiated to the outside. In order to ensure the working efficiency, it is recommended to use the maximum power. If the transmit power is reduced, the communication distance will be shortened, and the current consumed will be reduced. |
| DTU address | The internal address of the DTU has nothing to do with the Modbus address. Only DTU with the same radio address can communicate with each other. This feature can be used to realize software filtering and grouping. Input range: 0~65535, decimal number. |
| TX method | Transparent transmission, Fixed point: send data to fixed point in Hex format |
| Wake-up time | It is not directly related to the communication delay. If customers need low-power applications, they need to adjust this option as required. In the power saving mode, the longer the wake-up time, the lower the power consumption of the receiver, and the greater the communication delay. |

## 8. Connection diagram in test and application [p. 13]

Figura: PC (o PLC/RTU) conectado al transceptor con data lines; antena de 50Ω o carga ficticia; Power Adapter o
10~28V DC power supply.

## 9. Related Products [p. 13-14]

| Model | Interface Type | Frequency (Hz) | Transmit power (dBm) | Distance (km) | Features |
|---|---|---|---|---|---|
| E32-DTU (433L37) -V8 | RS232 RS485 | 410-441M | 37 | 20 | LoRa spread spectrum, long-distance anti-interference |
| E32-DTU (900L30) -V8 | RS232 RS485 | 862-930M | 30 | 8 | LoRa spread spectrum, long-distance anti-interference |
| E32-DTU (900L20)-V8 | RS232 RS485 | 862-930M | 20 | 3 | LoRa spread spectrum, long-distance anti-interference |
| E32-DTU (433L30) -V8 | RS232 RS485 | 410-441M | 30 | 8 | LoRa spread spectrum, long-distance anti-interference |
| E32-DTU (433L20) -V8 | RS232 RS485 | 410-441M | 20 | 3 | LoRa spread spectrum, long-distance anti-interference |
| E90-DTU (433L37) -V8 | RS232 RS485 | 410-441M | 37 | 20 | LoRa spread spectrum, long-distance anti-interference |
| E90-DTU (433L20) -V8 | RS232 RS485 | 410-441M | 20 | 3 | LoRa spread spectrum, long-distance anti-interference |
| E90-DTU (433L30) -V8 | RS232 RS485 | 410-441M | 30 | 8 | LoRa spread spectrum, long-distance anti-interference |
| E95-DTU (433L20-485)-V8 | RS485 | 410-441M | 20 | 3 | LoRa spread spectrum, long-distance anti-interference |
| E95-DTU (433L30-485)-V8 | RS485 | 410-441M | 30 | 8 | LoRa spread spectrum, long-distance anti-interference |
| E96-DTU (433L20-485)-V8 | RS485 | 410-441M | 20 | 3 | LoRa spread spectrum, long-distance anti-interference |
| E96-DTU (433L30-485)-V8 | RS485 | 410-441M | 30 | 8 | LoRa spread spectrum, long-distance anti-interference |
| E800-DTU (400SL20-485)-V8 | RS485 | 410-441M | 20 | 3 | LoRa spread spectrum, long-distance anti-interference |
| E800-DTU (400SL30-485)-V8 | RS485 | 410-441M | 30 | 8 | LoRa spread spectrum, long-distance anti-interference |

Nota de transcripcion: las filas E95-DTU (433L30-485)-V8 a E800-DTU cruzan la pagina 13 a la 14; la celda "Features"
de E95-DTU (433L30-485)-V8 se corta en "LoRa spread spectrum," en p. 13 y continua en p. 14 con
"long-distance anti-interference".

## 10. Practical application [p. 14]

The data transceiver of CDEBYTE is applied for all kinds of point to point, one point to multiple points wireless data
transmission system, such as smart home, Internet of things transformation, power load monitoring, distribution
network automation, hydrological and hydrological forecasting, water pipe network monitoring, urban street lamps
Monitoring, air defense alarm control, railway signal monitoring, centralized control of railway water supply, oil
supply pipe network monitoring, GPS system, remote meter reading, electronic crane, automatic reporting, seismic
forecasting, fire prevention, environmental monitoring and other industrial automation system, as shown below:

Figura: diagrama de red punto a multipunto: torre central con antena, torres laterales con antena, sensores e
instrumentos conectados a RTU, transceptores "DATA TRANSCEIVER" y un PC.

## 11. Precautions for Use [p. 15]

1. Please take good care of the warranty card of the device. The warranty card contains the factory number (and
   important technical parameters) of the device, which has important reference value for the user's future
   maintenance and new equipment.
2. During the warranty period, if the DTU is damaged due to the quality of the product itself rather than man-made
   damage or natural disasters such as lightning strikes, it enjoys free warranty; please do not repair by yourself,
   and contact our company if there is a problem. Ebyte provides first-class After-sales service.
3. Do not operate this DTU in the vicinity of some flammable places (such as coal mines) or explosive dangerous objects
   (such as detonators for detonation).
4. A suitable DC stabilized power supply should be selected, which requires strong anti-high frequency interference,
   small ripple, and sufficient load capacity; preferably, it should also have over-current, over-voltage protection
   and lightning protection functions to ensure that the DTU is normal jobs.
5. Do not use it in a working environment that exceeds the environmental characteristics of the DTU, such as high
   temperature, humidity, low temperature, strong electromagnetic field or dusty environment.
6. Don't let the DTU continuously be in full load transmitting state, otherwise the transmitter may be burnt out.
7. The ground wire of the DTU should be well connected with the ground wire of the external equipment (such as PC,
   PLC, etc.) and the ground wire of the power supply, otherwise the communication interface will be burnt easily; do
   not plug or unplug the serial port with power on.
8. When testing a DTU, you must connect a matching antenna or a 50Ω dummy load, otherwise the transmitter will be easily
   damaged; if the antenna is connected, the distance between the human body and the antenna should be more than 2
   meters to avoid injury. Touch the antenna when transmitting.
9. Wireless data transmission stations often have different communication distances in different environments. The
   communication distance is often affected by temperature, humidity, obstacle density, obstacle volume, and
   electromagnetic environment; in order to ensure stable communication, it is recommended to reserve more than 50%
   The communication distance margin.
10. If the measured communication distance is not ideal, it is recommended to analyze and improve the communication
    distance from the antenna quality and antenna installation method. You can also contact support@cdebyte.com for
    help.
11. When selecting the power supply, in addition to keeping 50% of the current margin as recommended, it should also be
    noted that its ripple must not exceed 100mV.
12. Wireless communication products need to be connected to an impedance-matched antenna to work normally. Even
    short-term tests cannot be omitted. Product damage caused by this reason will not be covered by the warranty.

## Revision history [p. 16]

| Version | Description | Issued by |
|---|---|---|
| V1.0 | Initial version | LC |
| V1.1 | Content revision | Hao |
| | | |

## About us [p. 17]

Technical support: support@cdebyte.com
Documents and RF Setting download link: www.cdebyte.com
Thank you for using Ebyte products! Please contact us with any questions or suggestions: info@cdebyte.com
Web: www.cdebyte.com
Address: B5 Mould Industrial Park, 199# Xiqu Ave, High tech Zone, Chengdu, Sichuan, China
