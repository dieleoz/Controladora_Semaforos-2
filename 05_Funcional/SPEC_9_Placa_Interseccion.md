# SPEC 9 — LA PLACA NUEVA: ESP32 que manda, STM32 que vigila

**Estado: NO CONSTRUIDA.** Ni esquematico, ni placa, ni firmware. Decision de origen: `D-36` (29/09/2026). Mientras
esta placa no pase banco, el equipo es el de `SPEC_0`..`SPEC_8` y nada de aqui lo cambia.

**Que es.** Una placa controladora para **una interseccion semaforizada de hasta 4 grupos**, que tambien sirve para el
paso alternado de dos postes de hoy (2 grupos). Sale de la placa actual (`01_Firmware/Controladora_Semaforos/`):
**se replica su etapa de potencia y su manejo de E/S**, el ESP32 pasa a mandar y el STM32 pasa a **vigilar**.

**Por que.** La placa actual no da para un cruce simple (10 canales, 3 muertos, `SPEC_5` §2), su cristal de reloj no
arranco y obligo a colgar un ESP32 por fuera (`D-9`, `D-20`), y no sabe si lo que ordeno se encendio de verdad
(`SPEC_0` §5.8). Criterio del responsable: **mejor 2 que 1**.

**Queda FUERA de esta spec:** el firmware de los dos micros (se porta cuando haya placas), la app, el gabinete, la
logica de planes y fases de una interseccion, y la carga en campo.

## 1. Reparto: quien hace que

| bloque | pieza | hace | no hace |
|---|---|---|---|
| **Controlador** | ESP32 **DevKitC V4 oficial** (WROOM-32E; 32UE si el gabinete es metalico) | ciclo, fases, demandas, radio, Bluetooth SPP y WiFi, relojes | no decide solo si una salida es segura |
| **Monitor de estados** | STM32F103C8 en la placa, con su cristal de 8 MHz | lee las salidas REALES, las compara con la matriz de conflictos y con lo ordenado, corta si no cuadra | no ordena ningun verde, no tiene reloj |
| **Hora** | `DS3231SN` + `RV-3028-C7`, cada uno con su pila | dos relojes con el cristal DENTRO del chip; el ESP32 los compara | ninguno depende de un cristal externo |

- **La DevKit va soldada o con amarre mecanico:** en un poste que vibra, un zocalo suelto es un fallo intermitente.
- **El STM32 NO usa su reloj interno ni su cristal de 32 kHz** (el que fallo): el monitor no necesita hora.
- **Los dos micros hablan por UART** (como hoy `J17`): el ESP32 manda a quien vigila lo que ordeno; el STM32 contesta
  lo que MIDE. **El `$ACK` a la app sale solo cuando el monitor confirma** (`CLAUDE.md` §2).

## 2. Salidas: 16 canales con la etapa de hoy

- **Etapa por canal, la misma de la placa actual** (`17_...` §1, probada en banco el 04/09 en `J15`): senal ->
  `R` 220 -> opto `TLP127` -> `R` 10 K a masa + `R` 220 a la puerta -> `IRLZ44N` de lado bajo -> bornera, con
  `1N4148` de rueda libre. Lamparas de **12 V de continua**, como hoy.
- **Reparto:** 12 canales para 4 grupos (rojo, ambar, verde; un grupo peatonal usa 2 de 3) + 1 pluma + 3 de reserva.
- **Mando desde el ESP32 por registros de desplazamiento** (`74HC595`, 2 en cadena) con **salida habilitada por
  resistencia a apagado**: en el arranque, el reset o con el ESP32 colgado, los 16 optos quedan sin corriente.
- **Ninguna senal de salida sale de un pin de arranque del ESP32** (0, 2, 5, 12, 15) ni del 14.

## 3. Lectura de vuelta: lo que de verdad esta encendido

| lectura | por canal | detecta | la lee |
|---|---|---|---|
| **tension en el drenador** | opto hacia `74HC165` | salida encendida o apagada, orden o no (MOSFET en corto, canal que no enciende) | STM32 |
| **corriente de lampara** | resistencia de medida + comparador hacia `74HC165` | lampara que consume: fundida, cable cortado, conector suelto | STM32 |

- Minimo: corriente en los **4 rojos**. Objetivo: en los **12 canales de lampara**.
- **La cadena de lectura va SOLO al STM32.** El ESP32 no la toca: si la tocara, el vigilado se vigilaria a si mismo.

## 4. El monitor: cuando corta y que queda

**Salta si:** dos verdes en conflicto segun la matriz · un grupo con dos colores a la vez · un grupo sin ningun
color · un rojo sin corriente (fundido) · lo medido no es lo ordenado pasado un plazo · el ESP32 deja de enviar su
latido.

**Que hace:** un **rele de transferencia** quita la alimentacion a rojos y verdes y pasa los ambar a un **destellador
propio del monitor**: **AMBAR INTERMITENTE** (`D-36`). **Enclavado:** solo lo levanta una persona en sitio.

- **Sin energia en el STM32, el rele cae a la posicion de falla.** Un monitor muerto no deja el cruce en verde.
- **Matriz de conflictos por puentes o microinterruptores en la placa**, uno por pareja de grupos: 6 parejas.
- La pluma en averia: la manda `SPEC_8` §5, no esta spec.

## 5. Entradas: 8 aisladas

**Opto en cada entrada** y proteccion contra picos: los cables vienen del poste, a metros (hoy `J16` llega a la pata
del micro sin nada en medio, `SPEC_5` §1). Se leen con un `74HC165` desde el ESP32.

| entradas | uso |
|---|---|
| 4 | **pulsadores de demanda peatonal**, uno por grupo |
| 3 | camaras (contacto seco, como `D-25`) |
| 1 | reserva (detector vehicular o fin de carrera de la pluma) |

**Un pulsador REGISTRA la demanda; nunca da el verde.** La fase peatonal entra en el siguiente punto permitido del
ciclo, tras ambar y todo-rojo. **Un pulsador atascado** mas de un plazo = demanda fija en cada ciclo + alarma.

## 6. Alimentacion, radio y bateria

- **12 V de bateria** -> **buck a 5 V** para la DevKit. **El STM32 lleva su propio regulador de 3,3 V:** el
  monitor no cae si cae el controlador. Proteccion de polaridad inversa y TVS en la entrada. Sale el `LM7805`.
- **RS485 para la radio E90-DTU:** un `MAX3485` con DE/RE, como `J12` hoy. `J10` desaparece (telemetria por WiFi).
- **Bateria:** divisor resistivo a una entrada ADC1 del ESP32. Cierra `SPEC_0` §5.1 (`BAT:` hoy sale `--`).

## 7. Pines del ESP32 — preliminar, a fijar por el arquitecto

| funcion | GPIO | nota |
|---|---|---|
| `74HC595` datos, reloj, cerrojo | 23, 18, 19 | salidas |
| `74HC595` salida habilitada | 4 | resistencia la deja en APAGADO |
| `74HC165` entradas: carga, reloj, datos | 25, 26, 34 | 34 solo entrada |
| RS485 radio: TX, DE, RX | 17, 16, 39 | 39 solo entrada |
| UART al STM32: TX, RX | 27, 35 | 35 solo entrada |
| I2C relojes: SDA, SCL | 21, 22 | |
| bateria (ADC1) | 36 | solo entrada |
| libres | 13, 32, 33 | |

## 8. HUECOS MEDIDOS — sin cerrar no se dibuja esa parte

1. **Corriente de cada lampara** (rojo, ambar, verde): sin ella no se dimensiona la resistencia de medida ni se sabe
   si el `IRLZ44N` y la pista bastan. Se mide con pinza en una lampara de cada color.
2. **Luz de confirmacion en los pulsadores:** 16 o 20 salidas (tercer `74HC595`).
3. **Fin de carrera de la pluma:** hoy no se instala (`D-27`); la entrada de reserva lo admite.
4. **Norma:** que ambar o rojo intermitente exige el Manual de Senalizacion Vial por grupo en falla. Sin citar.

## 9. Verificacion de extremo a extremo (en banco, sobre la placa fabricada)

1. Encender con el ESP32 sin firmware: **las 16 salidas a 0 V** en bornera.
2. Puentear dos verdes en conflicto a mano: **ambar intermitente** en todos los grupos y enclavado.
3. Quitar una lampara roja con el ciclo en marcha: **falla** y ambar intermitente.
4. Detener el ESP32 (reset mantenido): el monitor detecta la falta de latido y pasa a **ambar intermitente**.
5. Quitar la alimentacion del STM32: el rele cae a **ambar intermitente**.
6. Pulsar una demanda peatonal: la fase entra **en el siguiente punto permitido**, nunca antes.
7. Quitar la pila y la energia a un reloj: el ESP32 detecta la **discrepancia** entre los dos y avisa.
