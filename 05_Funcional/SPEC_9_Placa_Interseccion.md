# SPEC 9 — LA PLACA NUEVA: ESP32 que manda, STM32 que vigila

**Estado: NO CONSTRUIDA.** Ni esquematico, ni placa, ni firmware. Decision de origen: `D-36` (29/09/2026). Mientras
esta placa no pase banco, el equipo es el de `SPEC_0`..`SPEC_8` y nada de aqui lo cambia.

**Que es.** Una placa controladora para **una interseccion semaforizada de hasta 4 grupos**, que tambien sirve para el
paso alternado de dos postes de hoy (2 grupos). Sale de la placa actual (`01_Firmware/Controladora_Semaforos/`):
**se replica su etapa de potencia y su manejo de E/S**, el ESP32 pasa a mandar y el STM32 pasa a **vigilar**.

**Por que.** La placa actual no da para un cruce simple (10 canales, 3 muertos, `SPEC_5` §2), su cristal de reloj no
arranco y obligo a colgar un ESP32 por fuera (`D-9`, `D-20`), y no sabe si lo que ordeno se encendio de verdad
(`SPEC_0` §5.8). Criterio del responsable: **mejor 2 que 1**.

**Esta rama abre OTRO proyecto** (responsable, 29/09): la placa se disena, se valida y se deja lista para fabricar.
**Queda FUERA:** el firmware de los dos micros (es el proyecto siguiente; aqui solo el minimo de prueba de banco), la
app, el gabinete, la logica de planes y fases, y la carga en campo.

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

## 2. Salidas: 20 canales, y la potencia en TARJETAS DE GRUPO

**Las lamparas pueden ser de 12 V de continua (solucion solar, la de hoy) o de 120 V de alterna** (responsable, 29/09).
**Propuesta:** una **placa base** (micros, monitor, entradas, relojes, radio) y **4 tarjetas de grupo enchufables**, una
por grupo, de 3 canales cada una, en **dos variantes con el MISMO conector**:

| variante | etapa por canal | lectura de vuelta |
|---|---|---|
| **DC 12 V** | la de hoy (`17_...` §1, probada en banco el 04/09 en `J15`): `R` 220 -> opto `TLP127` -> `R` 10 K + `R` 220 -> `IRLZ44N` de lado bajo -> bornera, `1N4148` de rueda libre | opto en el drenador + resistencia de medida y comparador |
| **AC 120 V** | optotriac de paso por cero -> triac, con red de proteccion (snubber) y fusible por tarjeta | opto de alterna en la salida + transformador de corriente y comparador |

- Las dos variantes entregan al conector **las mismas senales digitales**: el monitor y el firmware no distinguen.
- **La variante AC separa 120 V de la logica** con distancias de aislamiento en la placa: norma aplicable en §8.
- **Reparto de los 20 canales:** 12 en las tarjetas de grupo (rojo, ambar, verde; un grupo peatonal usa 2 de 3) +
  **en la placa base, a 12 V:** 4 luces de confirmacion de los pulsadores + 1 pluma + 3 de reserva.
- **Mando desde el ESP32 por registros de desplazamiento** (`74HC595`, 3 en cadena = 24 salidas) con **salida
  habilitada por resistencia a apagado**: en el arranque, el reset o con el ESP32 colgado, nada recibe corriente.
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

**Que hace:** el monitor **corta la potencia de rojos y verdes** en las tarjetas de grupo y **gobierna los ambar por
una linea propia** que destella: **AMBAR INTERMITENTE** (`D-36`). **Enclavado:** solo lo levanta una persona en sitio.
En DC el corte es un rele de transferencia; en AC, un contactor o rele de potencia por cada tarjeta. Mismo conector.

- **Sin energia en el STM32, el rele cae a la posicion de falla.** Un monitor muerto no deja el cruce en verde.
- **Matriz de conflictos por puentes o microinterruptores en la placa**, uno por pareja de grupos: 6 parejas.
- La pluma en averia: la manda `SPEC_8` §5, no esta spec.

## 5. Entradas: 16 aisladas

**Opto en cada entrada** y proteccion contra picos: los cables vienen del poste, a metros (hoy `J16` llega a la pata
del micro sin nada en medio, `SPEC_5` §1). Se leen con dos `74HC165` desde el ESP32.

| entradas | uso |
|---|---|
| 4 | **pulsadores de demanda peatonal**, uno por grupo |
| 3 | camaras (contacto seco, como `D-25`) |
| 1 | **fin de carrera de la pluma** (responsable, 29/09): la pluma deja de ser ciega (`SPEC_0` §5.8) |
| 8 | reserva (detectores vehiculares) |

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

1. **Corriente de cada lampara, en las DOS variantes** (12 V solar y 120 V AC; rojo, ambar, verde): sin ella no se
   dimensiona la medida de corriente, el triac ni la pista. Se mide con pinza en una lampara de cada color.
2. **Norma vial:** que exige el Manual de Senalizacion Vial en falla, en grupos peatonales y en pulsadores. Se
   transcribe a `fuentes/md/`; hasta entonces, sin citar.
3. **Norma electrica para la variante AC 120 V** (aislamiento, protecciones, puesta a tierra): sin cargar.
4. **Modelo de pulsador y de fin de carrera:** su contacto (seco o con tension) fija la etapa de entrada.

## 9. Verificacion de extremo a extremo (en banco, sobre la placa fabricada, con firmware minimo de prueba)

1. Encender con el ESP32 sin firmware: **las 16 salidas a 0 V** en bornera.
2. Puentear dos verdes en conflicto a mano: **ambar intermitente** en todos los grupos y enclavado.
3. Quitar una lampara roja con el ciclo en marcha: **falla** y ambar intermitente.
4. Detener el ESP32 (reset mantenido): el monitor detecta la falta de latido y pasa a **ambar intermitente**.
5. Quitar la alimentacion del STM32: el rele cae a **ambar intermitente**.
6. Pulsar una demanda peatonal: la fase entra **en el siguiente punto permitido**, nunca antes.
7. Quitar la pila y la energia a un reloj: el ESP32 detecta la **discrepancia** entre los dos y avisa.
8. Repetir 1 a 5 con **una tarjeta de cada variante** (DC y AC) en el mismo conector: mismo resultado.
9. Pulsar una demanda: se enciende su **luz de confirmacion**; mover la pluma a mano: cambia el **fin de carrera**.
