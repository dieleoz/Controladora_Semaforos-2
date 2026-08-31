// ===== 01_Firmware/ESP32_Expansion/include/contrato.h =====
//
// EL CONTRATO DE BYTES, EN UN SOLO SITIO.
//
// POR QUE ESTAN TODOS AQUI Y NO REPARTIDOS POR LOS .cpp.
//
// Cada numero de este fichero tiene un GEMELO viviendo en otro lenguaje: el baudio y
// el tope de linea estan en el C++ del STM32 (Maestro/src/bluetooth.cpp), y el techo
// del watchdog esta en JavaScript (05_Funcional/App_Semaforo/app.js). Son dos copias
// del mismo contrato que alguien tiene que sincronizar a mano, y este repositorio ya
// sabe como acaba eso: N-36, N-39, cfgVerdeRecibido.
//
// La cura no es un comentario que lo recuerde -"los comentarios no fallan cuando
// alguien cambia un numero: se quedan describiendo un equipo que ya no existe, con la
// autoridad de una cuenta hecha" (N-71)-. La cura es que los numeros vivan en UN
// sitio localizable y que un pack los relea de las TRES fuentes en cada corrida:
//
//   banco/packs/esp32_01_watchdog_desigualdad.py   la desigualdad del watchdog
//   banco/packs/esp32_09_contrato_de_bytes.py      baudio, tope de linea, terminador
//
// Si alguien cambia un numero de aqui sin cambiar su gemelo, la compuerta se pone
// roja. Ese es todo el mecanismo, y es el unico que no envejece.

#ifndef CONTRATO_H
#define CONTRATO_H

#include <Arduino.h>

// ---------------------------------------------------------------------------
// TRANSPORTE HACIA EL STM32 POR J17
//
// GEMELO MEDIDO: Maestro/src/bluetooth.cpp:70 y Esclavo/src/bluetooth.cpp:78,
// ambos SerialBT.begin(9600). Las dos puntas van a la misma velocidad; si algun dia
// divergen, el mismo ESP32 no puede servir a las dos sin recompilar.
//
// EL FORMATO SE ELIGE AQUI PORQUE ALLI NO SE ELIGIO. SerialBT.begin(9600) se llama
// con un solo argumento: los 8 bits, sin paridad y el bit de parada los pone el
// framework por defecto. Ese default no se puede leer de ningun sitio del STM32, asi
// que este literal es LO UNICO ESCRITO que ata las dos puntas.
// ---------------------------------------------------------------------------
#define ENLACE_BAUDIO         9600
#define ENLACE_FORMATO        SERIAL_8N1
#define ENLACE_UART           2
#define ENLACE_PIN_TX         17   // GPIO17 -> J17 p2 -> PB7, que es el RX del micro
#define ENLACE_PIN_RX         16   // GPIO16 <- J17 p3 <- PB6, que es el TX del micro

// Bits en el cable por cada byte: 1 de arranque + 8 de dato + 1 de parada.
// Se escribe como constante porque el presupuesto de bytes/segundo se recalcula con
// ella y una cuenta a ojo -"9600/8"- da un 25% de margen que no existe.
#define ENLACE_BITS_POR_BYTE  10

// ---------------------------------------------------------------------------
// LIMITES DE LINEA
//
// GEMELO MEDIDO: btBufIn[64] con la guarda btIdxIn < sizeof(btBufIn) - 1, identica en
// las dos puntas (Maestro/src/bluetooth.cpp:31 y :397, Esclavo:29 y :303).
//
// EL EXCESO EL STM32 LO DESCARTA EN SILENCIO: no aborta la linea, no contesta nada,
// simplemente deja de guardar caracteres y entrega al despachador una linea TRUNCADA
// que se compara con strcmp como si estuviera completa. Por eso el tope se mide AQUI
// y lo que no quepa se rechaza con un $ERR propio: el STM32 no tiene con que
// protestar, y una orden truncada que casa por accidente con otra es un accidente.
// ---------------------------------------------------------------------------
#define TRAMA_MAX_UTIL        63

// El buffer donde se arma la linea que llega de la app. Un byte mas que el tope util
// para poder DETECTAR el desbordamiento en vez de sufrirlo: si el indice llega aqui,
// la linea sobra y se cuenta (P-3), no se recorta en silencio como hace el STM32.
#define BUF_ENTRADA_APP       (TRAMA_MAX_UTIL + 2)

// La trama mas larga que el STM32 puede poner en el cable son 132 B: payload[128] de
// $STATUS -127 utiles- mas "*XX\r\n". Se deja holgura porque una trama truncada por
// nuestro lado seria ruido fabricado por el puente, que es peor que el ruido de cable.
#define BUF_ENTRADA_STM32     160

// P-2: una rafaga de $ACK + $EVENT + $STATUS son 340 B, o sea 354 ms de cable. Si el
// buffer de salida hacia la app fuese menor, esa rafaga descartaria tramas justo
// cuando mas informacion hay que dar.
#define BUF_SALIDA_APP        512

// ---------------------------------------------------------------------------
// WATCHDOG
//
// LA DESIGUALDAD QUE GOBIERNA, Y LA CORRECCION QUE HAY QUE ENTENDER:
//
//   ESP32_WDT_MS + ESP32_ARRANQUE_MS < min(TIMEOUT_ENLACE_MS, SFTY6_SILENCIO_MS)
//                                    = min(5000, 25000) = 5000 ms
//
// El par que manda NO es SFTY-6. SFTY-6 vigila la radio LoRa -coordinador.cpp:656 usa
// tUltimaRxEsclavo y Esclavo/main.cpp:555 usa tUltimoComando, y NINGUNA de las dos lee
// un byte de SerialBT-. En esta arquitectura el ESP32 esta colgado de J17, fuera del
// camino de la radio: un ESP32 colgado NO dispara SFTY-6, no dispara NADA, y el STM32
// sigue ciclando sin enterarse. El unico testigo es la app, y su cota son 5 s.
//
// Eso es PEOR que el supuesto de partida, no mejor, y esta abierto como AB-1.
//
// El techo del task watchdog del IDF se programa en SEGUNDOS enteros
// (esp_task_wdt_init(uint32_t timeout, bool panic)), asi que este numero tiene que ser
// multiplo de 1000. El pack lo comprueba: un 2500 aqui se convertiria en 2 s dentro
// del chip y la desigualdad estaria midiendo un equipo que no existe.
// ---------------------------------------------------------------------------
#define ESP32_WDT_MS          2000UL

// 🛑 SIN VERIFICAR - AB-3. Nadie ha medido cuanto tarda este modulo desde el reset
// hasta volver a pasar bytes, ni cuanto tarda el telefono en reemparejar el SPP.
//
// LA FICHA TECNICA DEL MODULO NO LO DICE, Y ESO NO ES UN DETALLE: el 31/08 se cerro
// BLQ-1 con la ficha del ESP32-WROOM-32 -Xtensa LX6 dual-core, BR/EDR + BLE, 4 MB de
// flash, CP2102-, y ninguno de esos datos contesta esta pregunta. Un bloqueo cerrado
// no cierra los de al lado.
//
// Se escribe un numero porque la desigualdad tiene que poder recalcularse HOY; NO
// porque este medido. La bandera de abajo es lo que impide que se lea como medida:
// mientras valga 0, el pack lo REPORTA en cada corrida -reportar() no cuenta como
// comprobacion- y el margen que publica es un margen sobre un marcador.
//
// El dia que se mida con el modulo en la mano se sustituye el numero Y se sube la
// bandera. Lo que NO se hace es al reves: ajustar el watchdog para que un numero
// inventado cuadre es fabricar la autorizacion que la medida deberia dar.
#define ESP32_ARRANQUE_MS     1500UL
#define ESP32_ARRANQUE_MEDIDO 0        // 0 = SIN VERIFICAR (AB-3) · 1 = medido en banco

// W-4: tope de iteraciones del bucle INTERIOR de cada sentido.
//
// No es una optimizacion: es la mitad de la defensa. El fallo del 31/07/2026 fue un
// bucle interior que con ruido continuo NUNCA terminaba; alimentar el watchdog ahi
// dentro lo habria mantenido contento para siempre mientras el puente no progresaba.
// Con tope, el bucle exterior recupera el control y el reset del watchdog se da una
// vez por vuelta, que es lo unico que mide progreso de verdad.
#define PUENTE_MAX_ITER       64

// ---------------------------------------------------------------------------
// RELOJ DS3231 (modulo ZS-042)
//
// El modulo ZS-042 YA TRAE SUS PULL-UPS. No se anaden: dos juegos en paralelo bajan la
// resistencia efectiva y el bus deja de poder llegar a nivel alto en el tiempo debido.
//
// La direccion 0x68 es la del datasheet. SIN VERIFICAR sobre el modulo real: no hay
// DS3231 comprado (linea A6 de la lista de compras).
// ---------------------------------------------------------------------------
#define DS3231_DIR            0x68
#define DS3231_SDA            21
#define DS3231_SCL            22
#define DS3231_REG_HORA       0x00   // 7 registros: seg, min, hora, dia sem, dia, mes, anio
#define DS3231_REG_ESTADO     0x0F   // bit 7 = OSF
#define DS3231_BIT_OSF        0x80

// R-4: el OSF se relee periodicamente, no solo al arrancar. La pila del modulo se
// puede agotar con el equipo en marcha, y una hora que dejo de ser fiable a las tres
// de la manana no avisa sola.
#define RELOJ_RELECTURA_MS    60000UL

// R-9: LOS RANGOS VIVEN EN UN SITIO.
//
// Maestro/src/bluetooth.cpp:277-281 da el motivo con todas las letras: repetir los
// rangos en dos lados es una segunda copia que alguien tiene que sincronizar, y el dia
// que difieran una punta deja pasar lo que la otra rechaza. Aqui la copia unica esta
// en estas ocho lineas y la validacion las BARRE todas (R-7): comprobar "la hora" y
// dar por buenos los minutos es exactamente PESOS_SUMA de N-51, un numero que parece
// cubrir todos los casos sin haber evaluado ninguno.
#define RTC_ANIO_MIN          2000
#define RTC_ANIO_MAX          2099
#define RTC_MES_MIN           1
#define RTC_MES_MAX           12
#define RTC_DIA_MIN           1
#define RTC_DIA_MAX           31
#define RTC_HORA_MIN          0
#define RTC_HORA_MAX          23
#define RTC_MIN_MIN           0
#define RTC_MIN_MAX           59
#define RTC_SEG_MIN           0
#define RTC_SEG_MAX           59

// ---------------------------------------------------------------------------
// CENSO DE PREFIJOS QUE EL STM32 EMITE - SON CINCO, NO CUATRO
//
// 🔴 ESTA LISTA NO ES UN FILTRO. El puente retransmite TODA trama bien formada, la
// conozca o no, y eso es deliberado: es lo que ya hace el firmware del Repetidor en la
// otra topologia -valida FORMATO, no comandos-, y su motivo vale igual aqui. Un puente
// que conociera la lista habria que recompilarlo cada vez que el protocolo crece, y el
// dia que alguien olvidara hacerlo la funcion nueva se caeria en silencio.
//
// La lista existe SOLO para los contadores de diagnostico (P-3: lo que se descarta hay
// que poder contarlo y decirlo) y para que un pack pueda cruzarla contra lo que las dos
// puntas emiten de verdad. $EVENT es la que se cae de las listas escritas de memoria
// -catorce ramas del Maestro lo emiten y la app lo consume en app.js:814-, y un puente
// que filtrara por cuatro prefijos se comeria la bitacora entera: exactamente la
// perdida silenciosa que costo N-73.
// ---------------------------------------------------------------------------
#define PREFIJOS_STM32_N      5
extern const char* const PREFIJOS_STM32[PREFIJOS_STM32_N];

// ---------------------------------------------------------------------------
// ROTULO DEL DISPOSITIVO EN LA LISTA DE EMPAREJADOS
//
// identidad.h:42 ya dice para que sirve: "para que la lista de emparejados de Android
// ya diga que equipo es cada uno ANTES de conectar". "SEM-20260826-193412" no le dice
// nada al tecnico parado en el Km 12; SEM-7A3F2C-M si.
//
// EL PROBLEMA QUE ESTO TIENE Y HAY QUE ESCRIBIR: la serie sale del SILICIO DEL STM32
// (identidad_serie() lee el UID del micro). El ESP32 NO PUEDE SABERLA al arrancar.
//
// Solucion, y su coste declarado: el puente APRENDE la serie y el nodo del primer
// $STATUS que retransmite -sin tocarlo- y los guarda; el rotulo de la SIGUIENTE
// arrancada ya es el bueno. En el primer arranque de un modulo virgen el rotulo es el
// provisional de abajo, y eso es VISIBLE en la lista de Android en vez de disimulado.
//
// No se re-rotula en caliente: cambiar el nombre SPP obliga a cerrar y reabrir el
// perfil, o sea a tirar la sesion del operario que esta conectado en ese momento.
// ---------------------------------------------------------------------------
#define ROTULO_PREFIJO        "SEM-"
#define ROTULO_PROVISIONAL    "SEM-SIN-MATRICULA"
#define ROTULO_MAX            32

#endif // CONTRATO_H
