// ===========================================================================
// ESP32 DE EXPANSION  —  puente Bluetooth <-> J17  +  reloj DS3231
// ===========================================================================
//
// LA FRASE QUE GOBIERNA TODO EL DISENO:
//
//   El ESP32 es la superficie de mando, y NO es parte del lazo de seguridad.
//
// Las dos mitades importan por separado, y confundirlas es como se cuelan las
// propuestas que hay que rechazar:
//
//   - ES LA SUPERFICIE DE MANDO porque con la pantalla, los cuatro pulsadores y el
//     mando de reles retirados, TODA la operacion pasa por la app y la app pasa por
//     aqui. Un ESP32 colgado deja el equipo seguro pero NO OPERABLE.
//   - NO ES PARTE DEL LAZO DE SEGURIDAD porque el ciclo, el enclavamiento SFTY-2, el
//     todo-rojo y la caida a ambar de SFTY-6 viven enteros en el STM32 y no leen ni un
//     byte de este modulo.
//
// De ahi la regla de rechazo: cualquier propuesta que haga que el semaforo dependa del
// ESP32 para seguir siendo SEGURO se rechaza, por comoda que sea.
//
// 🔴 Y LO QUE NADIE VIGILA, QUE HAY QUE SABER ANTES DE FIARSE DE ESTE FIRMWARE:
//
// SFTY-6 mira la RADIO LoRa, no J17 -coordinador.cpp:656 usa tUltimaRxEsclavo y
// Esclavo/main.cpp:555 usa tUltimoComando; ninguna lee un byte de SerialBT-. Un ESP32
// colgado NO dispara SFTY-6: no dispara NADA. El STM32 sigue ciclando tan tranquilo y
// el unico testigo es la app, a 5 s. Eso esta abierto como AB-1 y es del responsable:
// no se cierra desde aqui.
//
// AB-1 SE INTENTO CONSTRUIR EL 04/09 Y SE PARO. EL PLAN ESCRITO ENFRENTE ES FALSO, Y
// ESTO ES LA MEDIDA QUE LO TUMBA.
//
// Lo pedido: que el puente emita un latido periodico para que los tres contadores de
// silencio de J17 -Maestro/src/bluetooth.cpp:213-223, MUDO/MAX/N- dejen de contar
// "silencios del puerto" y pasen a contar cortes del puente. El problema que ataca es
// real y esta bien planteado: por J17 solo entra lo que un dedo pulsa en la app, asi
// que hoy un puente vivo y un puente muerto no se distinguen desde el STM32.
//
// Lo que NO se sostiene es la segunda mitad de la frase que hay alli -"estos mismos
// tres numeros pasan a ser el registro de cortes de verdad SIN TOCAR UNA LINEA de
// aqui"-. Es una cuenta hecha dentro de un comentario, con la autoridad de un dato, y
// se cae al leer el receptor. MEDIDO en las DOS puntas, mismo codigo:
//
//   Maestro/src/bluetooth.cpp:626-637  ·  Esclavo/src/bluetooth.cpp:605-616
//     al recibir CR o LF:  if (btIdxIn > 0) { procesarComando(btBufIn);
//                                             j17RegistrarLinea(ahora); }
//
// j17RegistrarLinea() vive DENTRO del if de linea no vacia y DESPUES del despachador.
// De ahi salen las dos unicas posibilidades, y ninguna sirve:
//
//   linea VACIA -> btIdxIn == 0: no se despacha, pero TAMPOCO se registra. No cierra
//       ningun silencio, o sea que no mide nada. Seria un adorno que gasta cable.
//   linea NO VACIA -> pasa por procesarComando() SIEMPRE, y procesarComando() contesta
//       a TODO. Lo que no case cae en el ultimo else de la guarda de PIN y devuelve un
//       ERR con CMD AUTH_FAILED y DESC PIN_INVALIDO -Maestro:374, Esclavo:390-.
//
// NO HAY LINEA SILENCIOSA. El latido no seria un comando -no ejecuta nada, y hasta ahi
// 6.4 aguanta- pero SI dispara respuesta, y la respuesta llega al operario: app.js
// 2009-2028 no tiene traduccion para ese par y cae en el ramal generico, que hace
// addEvent en rojo mas showToast "Rechazado por el equipo: PIN_INVALIDO". Un latido
// cada 2 s es un aviso rojo cada 2 s acusando de una clave mala que nadie tecleo. Eso
// no es 6.4 rota por poco: es la leccion del FALLA PERMANENTE -un rechazo que ningun
// operario puede apagar ensena a todos a ignorar los rechazos de verdad-.
//
// Y HAY UN SEGUNDO COSTE QUE TAMPOCO DEPENDE DE ESTE LADO: cada latido cierra un
// silencio, y cerrar un silencio PUBLICA un EVENT de ORIGEN J17 -bluetooth.cpp:248-257,
// sin umbral y a proposito-. A 2 s son 1.800 lineas iguales por hora en el mismo
// registro donde hay que encontrar el fallo de campo. Es N-73 por inundacion en vez de
// por filtro: la bitacora existe y deja de poder leerse.
//
// EL PRESUPUESTO NO ES EL QUE MANDA, Y CONVIENE SABERLO PARA NO CULPARLO. Con la
// cadencia de telemetria ya en 2000 ms, el peor segundo de hoy son 462 B de 960 B/s
// -48,1%, esp32_07-. Cada latido devuelve 43 B del ERR -38 utiles + 5 de envoltorio- y
// hasta 116 B del EVENT -payload[112] - 1 + 5-, o sea 159 B. A 2 s eso suma 79,5 B/s y
// deja el peor segundo en 541,5 B = 56,4%. CABE de sobra. Lo que no cabe es lo que
// llega a la pantalla del que esta de pie en la calzada.
//
// EL PERIODO QUE SALDRIA, PARA QUE LA DECISION NO HAYA QUE REHACERLA: la ventana es
// 1000 ms < T < 3500 ms. Por abajo, el contador publica en segundos enteros
// -silencio / 1000UL-, asi que por debajo de 1000 ms MUDO sale siempre 0 s. Por arriba,
// el corte mas corto que hay que ver es un ciclo entero de perro, ESP32_WDT_MS (2000)
// mas ESP32_ARRANQUE_MS (1500) = 3500 ms; con T >= 3500 un reinicio completo cabe
// dentro de un hueco normal y no se distingue. T = 2000 ms deja el hueco normal en
// MUDO:2s y un reinicio en MUDO:5s o 6s. AVISO: ESP32_ARRANQUE_MS lleva
// ESP32_ARRANQUE_MEDIDO = 0 -AB-3-, asi que ese techo descansa sobre un numero sin
// medir y no se publica como margen.
//
// LO QUE HACE FALTA, Y ES DECISION DEL RESPONSABLE PORQUE TOCA EL MICRO QUE GOBIERNA EL
// CRUCE -son dos cambios en las dos bluetooth.cpp, no cero-:
//   1. una linea reservada que el despachador reconozca ANTES de la guarda de PIN y
//      devuelva sin actuar y SIN CONTESTAR, dejando que j17RegistrarLinea() trabaje;
//   2. que el EVENT de J17 no salga en cada latido, o la bitacora se ahoga.
//
// Mientras eso no exista, EL PUENTE NO EMITE LATIDO. Un latido que hoy solo produce un
// rechazo falso cada dos segundos no acerca el registro de cortes: lo cambia por ruido.
//
// Se deja escrito y no se borra: una causa que se cae se marca refutada, porque la que
// desaparece en silencio se vuelve a proponer y la segunda vez ya nadie recuerda que se
// comprobo.
// ===========================================================================

#include <Arduino.h>
#include "contrato.h"
#include "vigilante.h"
#include "enlace_stm32.h"
#include "transporte_app.h"
#include "reloj_ds3231.h"
#include "puente.h"

void setup() {
  // EL CENSO ANTES QUE EL PERRO, Y ES LA MITAD QUE FALTABA.
  //
  // Un puente que revive en silencio esconde el fallo que hay que contar: desde la app
  // y desde el STM32, un modulo que se reinicia cada dos segundos y uno sano son
  // INDISTINGUIBLES si el reinicio es rapido. Esto lee del chip por que arrancamos y
  // suma uno a la cuenta que vive en memoria RTC.
  //
  // Va antes de armar porque mira al PASADO -por que estamos aqui- y el armado al
  // FUTURO. Separadas, el censo corre aunque el armado falle, que es justo el modulo
  // que mas falta hace contar. NO EMITE NADA: setup() no pone un byte en ningun cable.
  vigilante_censarArranque();

  // W-5: EL PERRO PRIMERO. Antes del SPP y antes de tocar el I2C.
  //
  // El orden no es estetico: si el DS3231 cuelga el bus en el arranque -un SDA en corto
  // basta-, tiene que haber ya quien reinicie. Un watchdog armado despues del I2C no
  // veria nunca ese caso, que es justo uno de los que justifica tenerlo. Es la misma
  // razon por la que el Maestro arma el suyo ANTES de rtc.begin() (main.cpp:53 y :57).
  vigilante_armar();

  // EL PERRO SE ALIMENTA ENTRE ETAPA Y ETAPA, Y NO ES UN DETALLE DE ESTILO.
  //
  // Hasta aqui, entre vigilante_armar() y el primer vigilante_alimentar() de loop() NO
  // habia ni un solo reset. O sea que las CUATRO etapas de abajo compartian UN presupuesto
  // de ESP32_WDT_MS (2000 ms) y, pasado, panic=true reinicia. La mas cara con diferencia es
  // transporte_abrir(): monta la NVS por Preferences y levanta la pila Bluedroid clasica
  // entera -btStart, bluedroid_init, bluedroid_enable, registro del SPP y nombre-.
  //
  // Y CUANTO TARDA ESO NADIE LO HA MEDIDO. Lo dice el propio contrato.h en AB-3, con todas
  // las letras: "Nadie ha medido cuanto tarda este modulo desde el reset hasta volver a
  // pasar bytes". ESP32_ARRANQUE_MS lleva ESP32_ARRANQUE_MEDIDO = 0 justo por eso. Un techo
  // duro de 2 s sobre un arranque de duracion desconocida no es una proteccion: es una
  // apuesta, y si se pierde el modulo se reinicia en bucle. Desde el telefono un modulo que
  // rearranca cada 2 s no se ve como averiado: se ve como que APARECE Y DESAPARECE de la
  // lista, que es exactamente el sintoma del paso 10 del banco del 03-04/09.
  //
  // LO QUE ESTO NO DEBILITA, que es la mitad que hay que comprobar antes de creerselo. W-5
  // pide perro ANTES del I2C y del SPP para que un DS3231 que cuelgue el bus en el arranque
  // tenga quien reinicie. Sigue igual: cada etapa se alimenta ANTES de entrar y la
  // siguiente solo alimenta si la anterior VOLVIO. Una etapa colgada no llega nunca a su
  // reset y el perro muerde a los 2 s, como antes. Lo unico que cambia es que cada etapa
  // tiene sus propios 2 s en vez de repartirse unos solos entre las cuatro.
  //
  // NO VA DENTRO DE NINGUN BUCLE (W-3): son llamadas en linea recta. Un reset dentro de un
  // while alimentaria al perro para siempre mientras el puente no progresa.
  //
  // El puente no manda nada al abrir, en ninguno de los dos sentidos.
  enlace_setup();
  vigilante_alimentar();

  reloj_setup();
  vigilante_alimentar();

  transporte_abrir();
  vigilante_alimentar();

  puente_setup();
}

void loop() {
  // Una vuelta = las dos direcciones + el reloj.
  puente_bombear();
  reloj_revisar();

  // EL PARTE DE ARRANQUE SALE AQUI, NO EN setup(), Y LAS DOS RAZONES SON DISTINTAS.
  //
  // La primera es 6.4: setup() no puede emitir. Un saludo del puente es una orden que
  // nadie pidio entrando por el mismo camino que las que si se piden.
  // La segunda es que en setup() NO HABRIA NADIE ESCUCHANDO -transporte_escribir()
  // devuelve 0 sin telefono conectado-, asi que el parte se perderia justo la vez que
  // hace falta. Esta funcion espera al enlace y se rearma cuando cae.
  vigilante_declarar();

  // W-2: el reset, UNA VEZ POR VUELTA DEL BUCLE EXTERIOR y aqui abajo, despues de haber
  // atendido las dos direcciones.
  //
  // 🔴 W-3: NO va dentro de ninguno de los while interiores de puente.cpp, y esa es la
  // linea entre un watchdog y un adorno. El fallo del 31/07/2026 fue exactamente eso:
  // con ruido continuo el bucle interior nunca terminaba. Un reset ahi dentro
  // alimentaria al perro para siempre mientras el puente no progresa -un watchdog que
  // un flujo de basura mantiene contento no vigila nada-.
  vigilante_alimentar();
}
