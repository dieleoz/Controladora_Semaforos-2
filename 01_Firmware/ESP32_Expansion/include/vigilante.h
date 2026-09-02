// ===== 01_Firmware/ESP32_Expansion/include/vigilante.h =====
//
// EL WATCHDOG. VA PRIMERO, Y NO ES UN ORDEN ARBITRARIO.
//
// Las otras dos funciones -el reloj y el puente- cuelgan de que este modulo siga vivo,
// y hoy nada garantiza que lo este:
//
//   MEDIDO  Maestro/src/main.cpp:53    IWatchdog.begin(4000000)  -> 4 s, con reload en :130
//   MEDIDO  Esclavo/src/main.cpp:238   IWatchdog.begin(4000000)  -> 4 s, con reload en :318
//   MEDIDO  Repetidor/src/  grep -rniE "watchdog|esp_task_wdt|WDT"  ->  CERO
//
// Los dos STM32 tienen perro. El ESP32 de este proyecto no tenia ninguno, y hay
// precedente escrito de uno de este mismo proyecto clavado tumbando el enlace
// (01_Firmware/TROUBLESHOOTING.md:48 y :55, 31/07/2026).
//
// Y LO QUE ESTO NO CUBRE, ESCRITO AL LADO PORQUE ES LA MITAD DEL PROBLEMA:
// un watchdog rescata al ESP32 COLGADO. No hace nada por uno MUERTO, desalimentado o
// desenchufado. Con la pantalla, los pulsadores y el mando retirados, eso deja el
// equipo seguro pero SIN NINGUNA SUPERFICIE DE MANDO. Sigue abierto como AB-2 y tiene
// dueno: no se cierra desde el firmware.

#ifndef VIGILANTE_H
#define VIGILANTE_H

#include <Arduino.h>

// W-5: se arma en setup(), ANTES de abrir el transporte de la app y ANTES de tocar el
// I2C. Si el DS3231 cuelga el bus en el arranque -un SDA en corto basta- tiene que
// haber ya quien reinicie; un watchdog que se armara despues del I2C no veria nunca
// ese caso, que es justo uno de los que justifica tenerlo.
void vigilante_armar();

// W-2: UNA VEZ POR VUELTA DEL BUCLE EXTERIOR, despues de haber atendido las dos
// direcciones.
//
// 🔴 W-3: NO se llama desde dentro del while interior de ningun sentido. Esa es la
// forma exacta del fallo del 31/07: con ruido continuo el bucle interior nunca termina,
// y un reset ahi dentro alimentaria al perro para siempre mientras el puente no
// progresa. Un watchdog que un flujo de basura mantiene contento no vigila nada.
void vigilante_alimentar();

// Para el diagnostico: si el registro fallo, el resto del firmware corre creyendose
// vigilado y no lo esta. Un esp_task_wdt_init() sin su reset en el sitio correcto es
// CAM_UMBRAL_PIN con otro nombre -un pinMode() sin digitalRead(), con documentacion
// encima-, y esta funcion existe para que ese caso se pueda ver en vez de suponerse.
//
// HASTA HOY NO TENIA UN SOLO LLAMADOR. Era N-73 exacto: declarada, definida,
// documentada con su motivo, y huerfana. El parte de arranque de aqui abajo es quien
// la llama, y por eso el campo PERRO del parte no es adorno: es lo unico que distingue
// "el modulo volvio" de "el modulo volvio y ademas SIGUE sin vigilancia".
bool vigilante_armado();

// ===========================================================================
// LA OTRA MITAD: UN PUENTE QUE REVIVE EN SILENCIO ESCONDE EL FALLO QUE HAY QUE CONTAR
// ===========================================================================
//
// El watchdog de arriba levanta al modulo colgado. Eso, solo, es peligroso: desde la
// app y desde el STM32, un ESP32 que se reinicia cada dos segundos y uno que funciona
// bien SON INDISTINGUIBLES mientras el reinicio sea rapido -la telemetria se corta un
// instante y vuelve-. La sesion de banco terminaria con el equipo "funcionando" y con
// el numero que importa -cuantas veces se cayo- perdido para siempre.
//
// Por eso el modulo declara POR QUE arranco y CUANTAS VECES lleva arrancando.
//
// LA CAUSA SE LEE DEL CHIP, NO SE DEDUCE. esp_reset_reason() devuelve lo que el
// hardware apunto en el dominio RTC; deducirla de una bandera propia solo funcionaria
// en los reinicios que el firmware ve venir, que son justo los que no importan.
//
// DONDE SOBREVIVE LA CUENTA, Y COMO SE COMPROBO QUE SOBREVIVE (regla del instrumento:
// no se da por hecho). Medido en el header del propio IDF que trae este framework,
//
//   C:/.platformio/packages/framework-arduinoespressif32/
//       tools/sdk/esp32/include/esp_common/include/esp_attr.h
//
//   :77   RTC_DATA_ATTR    ".rtc.data"    "keep its value during a deep sleep / wake
//                                          cycle"                  <- NO dice reinicio
//   :102  RTC_NOINIT_ATTR  ".rtc_noinit"  "keep its value AFTER RESTART or during a
//                                          deep sleep / wake cycle"
//
// Solo la segunda promete lo que hace falta, asi que es la que se usa. Y su precio va
// escrito al lado porque es la trampa: ".rtc_noinit" NO SE INICIALIZA NUNCA, tampoco
// en la primera subida de tension. Sin una marca de validez, la cuenta arrancaria
// valiendo lo que hubiera en esa RAM -un contador que puede empezar en 3.417.882 no es
// un contador, es un adorno con cifras-.
//
// LO QUE ESTA CUENTA NO ES, DECLARADO PARA QUE NADIE LA LEA DE MAS: son los arranques
// DESDE LA ULTIMA SUBIDA DE TENSION, no desde que el equipo se instalo. Un corte de
// luz la pone a cero, y se pone a cero A PROPOSITO -ver vigilante.cpp-. Guardarla en
// la NVS la haria sobrevivir tambien al corte, y ese es justo el cambio que NO se hace:
// un modulo en bucle de reinicio a 2 s escribiria la flash 1.800 veces por hora y se
// comeria la NVS en dias. La memoria RTC no se desgasta.
//
// COMO SE PREGUNTA. El parte se compone aparte -vigilante_parteDeArranque()- de quien
// lo emite, y eso no es simetria: es lo que permite que la misma respuesta salga por
// los dos caminos que YA existen, sin un segundo formato que alguien tenga que
// sincronizar. Hoy lo emite vigilante_declarar() en cada conexion nueva; una rama del
// despachador que lo pida por comando reusa este mismo compositor.

// El buffer donde se arma el parte. La desigualdad que gobierna este numero
//
//   VIGILANTE_PARTE_MAX  >=  texto fijo del formato + la causa mas larga + el estado
//                            del perro mas largo + los digitos del contador + el nulo
//
// NO vive en este comentario: la recalcula el pack esp32_10_parte_de_arranque desde el
// propio formato y desde los literales de vigilante.cpp en cada corrida. Un comentario
// no falla cuando alguien anade una causa con un nombre mas largo (N-71); lo que
// pasaria entonces es que snprintf truncaria el parte y el checksum saldria calculado
// sobre el trozo, que es la clase de mentira bien formada que este proyecto persigue.
#define VIGILANTE_PARTE_MAX   144

// Lee del CHIP la causa de ESTE arranque y actualiza la cuenta de arranques.
//
// Va SEPARADA de vigilante_armar() y no por estetica: el censo mira al PASADO -por que
// estamos aqui- y el armado al FUTURO -quien nos reinicia la proxima vez-. Separadas,
// el censo corre aunque el armado falle, y un modulo que no logro armar el perro es
// precisamente el que mas falta hace contar.
//
// NO EMITE NADA. Se llama desde setup(), y setup() no pone un byte en ningun cable:
// un saludo del puente es una orden que nadie pidio (6.4, esp32_08).
void vigilante_censarArranque();

// Compone el parte de arranque en el buffer dado. Devuelve los bytes escritos, o 0 si
// no cabia -y entonces NO se manda nada: una trama truncada sale bien formada hasta la
// mitad y con el checksum calculado sobre otra cosa, que es peor que no mandarla-.
size_t vigilante_parteDeArranque(char* destino, size_t capacidad);

// Emite el parte hacia la APP -nunca hacia el STM32- una vez por conexion.
//
// ESPERA A QUE HAYA ALGUIEN ESCUCHANDO, y no es cortesia: transporte_escribir()
// devuelve 0 sin telefono conectado (transporte_app.cpp:59), asi que un parte emitido
// en el arranque se perderia entero y el operario nunca sabria que el modulo se cayo.
//
// Se rearma al caer el enlace, asi que reconectar vuelve a preguntar. Esa es la unica
// forma de preguntarlo que no anade trafico hacia el STM32: una linea de comando se
// reenviaria VERBATIM al equipo (B-1) y el STM32 contestaria $ERR,CMD:DESCONOCIDO a
// una pregunta que no era para el.
void vigilante_declarar();

#endif // VIGILANTE_H
