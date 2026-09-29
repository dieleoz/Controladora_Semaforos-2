# SPEC 9 — LA PLACA NUEVA: ESP32 que manda, STM32 que vigila

**Estado: NO CONSTRUIDA.** Ni esquematico, ni placa, ni firmware. Decision de origen: `D-36` (29/09/2026). Mientras
esta placa no pase banco, el equipo es el de `SPEC_0`..`SPEC_8` y nada de aqui lo cambia. Revisada por arquitectura el
29/09: **APTO CON CONDICIONES**; las cinco condiciones estan incorporadas abajo (§4) y marcadas `[C1]`..`[C5]`.

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
| **Controlador** | **ESP32-DevKitC-32E** oficial (WROOM-32E; **nunca WROVER**, que usa GPIO 16/17; 32UE si el gabinete es metalico) | ciclo, fases, demandas, radio, Bluetooth SPP y WiFi, relojes, entradas | no decide solo si una salida es segura |
| **Monitor de estados** | STM32F103C8 en la placa, cristal de 8 MHz con CSS e IWDG | lee las salidas REALES, las compara con la matriz de conflictos, corta si no cuadra | no ordena ningun verde, no tiene reloj, **no usa la orden del ESP32 para decidir un conflicto** |
| **Hora** | `DS3231SN` (0x68) + `RV-3028-C7` (0x52), cada uno con su pila | dos relojes con el cristal DENTRO del chip; el ESP32 los compara | ninguno depende de un cristal externo |

- **La DevKit va soldada o con amarre mecanico:** en un poste que vibra, un zocalo suelto es un fallo intermitente.
  Se actualiza por USB accesible con la tapa abierta, u OTA.
- **El STM32 NO usa su reloj de 32 kHz** (el que fallo). Depende de su cristal de 8 MHz: lo vigilan CSS e IWDG.
- **Independencia:** el monitor decide un conflicto **solo con su lectura directa y la matriz de puentes**. La orden
  del ESP32 le sirve para detectar "lo medido no es lo ordenado", nunca para absolver un conflicto.

### 1.1 Enlace entre los dos micros

- **UART** con trama, **CRC** y plazo de vigencia: una trama corrupta o caducada no es una orden.
- **Latido por hardware** del ESP32 al STM32, en una linea propia, independiente de la UART.
- **Linea de FALLA** del STM32 al ESP32: el ESP32 sabe que el monitor corto sin depender de la UART.
- RX de cada lado con pull-up y resistencia en serie: con el otro micro apagado, la linea no flota.
- **El `$ACK` a la app sale solo cuando el monitor confirma** lo medido (`CLAUDE.md` §2).

## 2. Salidas: 20 canales, y la potencia en TARJETAS DE GRUPO

**Las lamparas pueden ser de 12 V de continua (solucion solar, la de hoy) o de 120 V de alterna** (responsable, 29/09).
Una **placa base** (micros, monitor, entradas, relojes, radio) y **4 tarjetas de grupo enchufables**, una por grupo,
de 3 canales cada una, en dos variantes:

| variante | etapa por canal | lectura de vuelta |
|---|---|---|
| **DC 12 V** | la de hoy (`17_...` §1, probada en banco el 04/09 en `J15`): `R` 220 -> opto `TLP127` -> `R` 10 K + `R` 220 -> `IRLZ44N` de lado bajo -> bornera, `1N4148` de rueda libre | opto en el drenador + resistencia de medida y comparador |
| **AC 120 V** | optotriac de paso por cero -> triac, snubber y fusible | opto de alterna **con retencion de nivel** (un opto AC leido en crudo da 0 en parte del ciclo) + transformador de corriente y comparador |

**Regla del conector (revision 29/09):**
- **El conector base-tarjeta lleva SOLO bajo voltaje:** mando, lectura de vuelta, permiso, ambar de falla, presencia
  e identidad. **Ni 12 V de lampara ni 120 V pasan por el.**
- **La potencia de lampara entra por la bornera propia de cada tarjeta**, con su fusible y **su propio elemento de
  corte** (rele en DC, contactor con contacto auxiliar en AC). Asi no hay un corte comun unico.
- **Presencia e identidad:** lazo de presencia + resistencia codificada por variante. El monitor la lee y fija el modo
  DC o AC desde esa lectura; una tarjeta ausente o de variante no configurada es falla.
- **Carcasas y polarizacion distintas por variante.** Si aun asi pueden confundirse, se usan dos conectores distintos.
- **Grupos sin usar** (el paso alternado usa 2): se declaran en la configuracion del monitor, o disparan "grupo sin
  color".

**Reparto de los 20 canales:** 12 en las tarjetas de grupo (rojo, ambar, verde; un grupo peatonal usa 2 de 3) +
**en la placa base, a 12 V:** 4 luces de confirmacion de los pulsadores + 1 pluma + 3 de reserva.

**Mando desde el ESP32 por `74HC595`** (3 en cadena = 24 salidas):
- **OE con pull-up a apagado**, y **el monitor tambien bloquea OE** (OR por diodo): con el ESP32 colgado y OE en bajo,
  el monitor apaga las salidas sin depender de el.
- **Pull-down en cada entrada de etapa:** con OE en alto las salidas flotan, y el pull-down las deja en apagado.
- **SRCLR** definido (pin o fijo a 3,3 V), y la cadena **se refresca entera en cada ciclo**: un pulso espurio no queda.
- **Ninguna senal de salida sale de un pin de arranque del ESP32** (0, 2, 5, 12, 15) ni del 14.

## 3. Lectura de vuelta: lo que de verdad esta encendido

**Dos cadenas de `74HC165`, separadas:** la de **lectura de vuelta** la lee solo el STM32; la de **entradas** (§5), solo
el ESP32. Si el ESP32 tocara la lectura de vuelta, el vigilado se vigilaria a si mismo.

| lectura | por canal | detecta |
|---|---|---|
| **tension en la salida** | opto hacia `74HC165` | salida encendida o apagada, orden o no (MOSFET o triac en corto, canal que no enciende) |
| **corriente de lampara** | medida + comparador hacia `74HC165` | lampara que consume: fundida, cable cortado, conector suelto |

- Minimo: corriente en los **4 rojos**. Objetivo: en los **12 canales de lampara**.
- **Testigos de cadena:** un bit fijo a 1 y otro fijo a 0 en cada cadena. Una linea de datos pegada da todo 0 o todo
  1 con pinta de estado valido; con los testigos, eso es falla de cadena.

## 4. El monitor: cuando corta, que queda y como se levanta

**Salta si:** dos verdes en conflicto segun la matriz · un grupo con dos colores a la vez · un grupo sin ningun
color · un rojo sin corriente (fundido) · lo medido no es lo ordenado pasado un plazo · el ESP32 deja de enviar su
latido · tarjeta ausente o de variante equivocada · realimentacion del corte que no coincide · testigo de cadena mal.

**Que hace:** el monitor **abre el corte de rojos y verdes** en cada tarjeta y pasa los ambar a la **ruta de falla**:
**AMBAR INTERMITENTE** (`D-36`).

- **[C1] El destello de falla NO lo genera el STM32:** lo genera un **destellador independiente** (tipo 555) en la
  ruta de falla, alimentado sin pasar por el STM32. Con el STM32 sin energia o colgado, el ambar sigue destellando.
- **[C2] Un STM32 colgado no sostiene el corte cerrado:** la bobina del corte se alimenta por una **bomba de carga
  acoplada en alterna** desde un pin del STM32. Solo un pulso continuo la mantiene; un nivel fijo, alto o bajo, la
  suelta. Mas IWDG y CSS en el micro.
- **[C3] Un contacto soldado se detecta y no basta para dar paso:** cada corte tiene **contacto espejo o realimentacion
  aguas abajo** que lee el monitor, y un **segundo elemento de corte en serie** (rele o MOSFET).
- **[C4] Tarjeta ausente o equivocada es falla** (§2, presencia e identidad).
- **[C5] El estado de arranque del monitor es SIEMPRE falla.** El enclavado no vive en RAM: tras un corte de energia
  el cruce vuelve en ambar intermitente. **Solo lo levanta un pulsador fisico de reinicio en la placa**, en sitio.

**Matriz de conflictos:** puentes o microinterruptores en la placa, uno por pareja de grupos (6 parejas). **Un puente
ausente o abierto significa CONFLICTO**, nunca libre.

**La norma vial pide otra cosa que `D-36`, y lo decide el responsable (PREGUNTA ABIERTA).** Manual de Senalizacion
Vial 2024, 4.2.3 (p. 378, cotejado en el PDF el 29/09): ante conflicto, fallo de lamparas o activacion erronea,
*"amarillo intermitente para la via principal y en rojo intermitente para la via secundaria"*; y 4.6.5 (p. 416): los
peatonales en intermitente *"deberan exhibir la senal de rojo intermitente"*. **Por eso la ruta de falla se dibuja
capaz de destellar AMBAR O ROJO por grupo, elegido con un puente** (principal / secundaria / peatonal): la placa
cumple las dos lecturas y la decision queda en configuracion, no en cobre.

**Punto unico que queda:** si falla el canal de ambar de un grupo, ese grupo queda oscuro mientras los otros destellan.
El monitor lo detecta pero no lo repara. Que muestra un grupo peatonal en falla: lo dice la norma vial (§8.2).

La pluma en averia la manda `SPEC_8` §5, no esta spec.

## 5. Entradas: 16 aisladas

**Opto en cada entrada** y proteccion contra picos: los cables vienen del poste, a metros (hoy `J16` llega a la pata
del micro sin nada en medio, `SPEC_5` §1). Se leen con dos `74HC165` desde el ESP32, con testigos de cadena (§3).

| entradas | uso |
|---|---|
| 4 | **pulsadores de demanda peatonal**, uno por grupo |
| 3 | camaras (contacto seco, como `D-25`) |
| 1 | **fin de carrera de la pluma** (responsable, 29/09): la pluma deja de ser ciega (`SPEC_0` §5.8) |
| 8 | reserva (detectores vehiculares) |

**Un pulsador REGISTRA la demanda; nunca da el verde.** La fase peatonal entra en el siguiente punto permitido del
ciclo, tras ambar y todo-rojo. **Un pulsador atascado** mas de un plazo = demanda fija en cada ciclo + alarma. Los
pulsadores con luz de confirmacion necesitan su alimentacion en la bornera: se fija con el modelo (§8.4).

## 6. Alimentacion, radio, relojes y bateria

- **12 V de bateria** -> **buck a 5 V** para la DevKit. **El STM32 lleva su propio regulador de 3,3 V.** Proteccion de
  polaridad inversa y TVS en la entrada; sale el `LM7805`. Un corto en una tarjeta lo corta **su fusible**, no el bus.
- **USB de servicio conectado con 12 V presentes:** que no retroalimente el 5 V de la DevKit. A verificar en su
  esquematico (hipotesis).
- **RS485 para la radio E90-DTU:** un `MAX3485`, **DE con pull-down** (flota en el arranque), terminacion por puente.
  `J10` desaparece (telemetria por WiFi).
- **Relojes:** DS3231 y RV-3028 comparten el bus I2C, con pull-ups a 3,3 V. Uno colgado puede tumbar el bus: el
  firmware lo detecta y avisa; si hace falta, un aislador de bus por reloj.
- **Bateria:** divisor resistivo con filtro RC a GPIO36 (ADC1, compatible con WiFi). Cierra `SPEC_0` §5.1.
- **Cabecera SWD del STM32 y BOOT0 definido** en la placa: el monitor se programa sin desmontar.

## 7. Pines

### 7.1 ESP32 (revisado por arquitectura el 29/09; no pisa arranque, flash, consola ni ADC2 analogico)

| funcion | GPIO | nota |
|---|---|---|
| `74HC595` datos, reloj, cerrojo | 23, 18, 19 | |
| `74HC595` OE | 4 | pull-up 10 K a apagado; el monitor lo bloquea por diodo |
| `74HC595` SRCLR | 33 | o fijo a 3,3 V si hace falta el pin |
| `74HC165` entradas: carga, reloj, datos | 25, 26, 34 | 34 solo entrada |
| RS485 radio: TX, DE, RX | 17, 16, 39 | DE con pull-down; 39 solo entrada |
| UART al STM32: TX, RX | 27, 35 | RX con pull-up; resistencia en serie |
| I2C relojes: SDA, SCL | 21, 22 | pull-ups a 3,3 V |
| bateria (ADC1) | 36 | filtro RC |
| latido por hardware al STM32 | 13 | |
| linea de FALLA desde el STM32 | 32 | |

**No queda ningun pin libre.** Lo siguiente que haga falta va por I2C o un expansor.

### 7.2 STM32 — por fijar antes del esquematico

Cadena de lectura de vuelta, lectura de la matriz, bomba de carga del corte, habilitacion de la ruta de falla, bloqueo
de OE, pulsador de reinicio, latido de entrada, linea de FALLA, UART, presencia e identidad de las 4 tarjetas, SWD.

## 8. HUECOS MEDIDOS — sin cerrar no se dibuja esa parte

1. **Corriente de cada lampara, en las DOS variantes** (12 V solar y 120 V AC; rojo, ambar, verde) **y su tipo** (LED
   o incandescente): sin eso no se dimensiona la medida de corriente, el triac ni la pista. Una lampara LED en AC puede
   no llegar a la corriente de mantenimiento del triac, y la fuga del snubber puede encenderla debil. **La variante AC
   no se dibuja hasta cerrar este hueco y el 3.**
2. **Norma vial: TRANSCRITA** en `fuentes/md/Manual_Senalizacion_Vial_semaforos.md` (ANSV 2024, 2.a ed., fe de
   erratas oct. 2025). Abierto: la resolucion que la adopta NO CONSTA en el PDF, asi que su exigibilidad aqui depende
   del contrato. **No regula la luz de confirmacion del pulsador** (NO CONSTA). Pide monitor de conflictos, de fallo
   de lampara y de activacion erronea (4.2.3): esta placa lo cumple por §3 y §4.
3. **Norma electrica para la variante AC 120 V** (aislamiento, protecciones, puesta a tierra): sin cargar.
4. **Modelo de pulsador y de fin de carrera:** su contacto (seco o con tension) y la alimentacion de la luz de
   confirmacion fijan la etapa de entrada.
5. **`74HC595` a 3,3 V contra el opto `TLP127`:** corriente por salida y tope por encapsulado, en hoja de datos.

## 9. Verificacion de extremo a extremo (en banco, sobre la placa fabricada, con firmware minimo de prueba)

1. Encender con el ESP32 sin firmware: **las 20 salidas a 0 V** en bornera.
2. Puentear dos verdes en conflicto a mano: **ambar intermitente** en todos los grupos y enclavado.
3. Quitar una lampara roja con el ciclo en marcha: **falla** y ambar intermitente.
4. Detener el ESP32 (reset mantenido): el monitor detecta la falta de latido y pasa a **ambar intermitente**.
5. Quitar la alimentacion del STM32: **el ambar sigue destellando** por el destellador independiente.
6. Congelar el STM32 con una salida en alto (depurador parado): **el corte cae** y queda ambar intermitente.
7. Puentear a mano el contacto de un corte (simula soldado): el monitor lee la realimentacion y declara **falla**.
8. Sacar una tarjeta de grupo, y poner una de la otra variante: **falla** en los dos casos.
9. Pegar la linea de datos de una cadena `74HC165` a 0 y a 1: **falla de cadena** por los testigos.
10. Tras una falla, cortar y devolver la energia: **sigue en ambar intermitente**; solo el pulsador de reinicio lo
    levanta.
11. Pulsar una demanda peatonal: la fase entra **en el siguiente punto permitido**, nunca antes, y se enciende **su luz
    de confirmacion**. Mover la pluma a mano: cambia el **fin de carrera**.
12. Quitar la pila y la energia a un reloj: el ESP32 detecta la **discrepancia** entre los dos y avisa.
13. Repetir 1 a 10 con **una tarjeta de cada variante** (DC y AC): mismo resultado.
