// ===== include/botones.h =====
#pragma once
#include <Arduino.h>

void botones_setup();

// ---------------------------------------------------------------------------
// SFTY-21 (V8.7): la deteccion de flancos se hace UNA VEZ POR ITERACION, aqui, y no
// dentro de cada botonX() como antes.
//
// El motivo es el mando de reles. Sus secuencias (A.A.A, B.B.B, A.B.A.B) tienen que
// verse SIEMPRE, con independencia de que la pantalla en la que este el equipo lea o
// no ese boton. Con la deteccion metida dentro de botonArriba(), un modo que no
// consultase el Boton 1 hacia que sus pulsaciones no existieran para nadie: el
// operario habria pulsado tres veces desde el suelo y el equipo no habria contado
// ninguna.
//
// Efecto secundario bueno: antes, si un modo no consultaba un boton durante un rato,
// la pulsacion quedaba latente y se disparaba en cuanto alguien preguntaba, aunque
// hubiera ocurrido mucho antes. Ahora el flanco vive solo la iteracion en la que
// ocurre.
//
// DEBE llamarse al principio del loop principal, antes de atender ningun modo.
void botones_actualizar();

// Estas siguen consumiendo el flanco, igual que antes: leerlo lo gasta.
bool botonArriba();
bool botonAbajo();

// ACEPTAR y CANCELAR YA NO TIENEN SUJETO (31/08/2026). Sus pines -PB14 y PB15, J16 p10 y
// p12- son camaras desde esa fecha, asi que las dos devuelven siempre false. Se conservan
// declaradas a proposito, y el censo de lo que se pierde y de con que se sustituye cada
// caso esta en botones.cpp, junto a las definiciones. No se llaman en vano: mientras
// existan, "git grep botonCancelar" sigue listando de una sola vez todo lo que la
// retirada de los botones C y D dejo sin mando fisico.
bool botonAceptar();
bool botonCancelar();

// ---------------------------------------------------------------------------
// LAS CAMARAS DE J16 - N-97, 31/08/2026. ESTE BLOQUE ES IDENTICO EN LAS DOS PUNTAS.
//
// J16 p10 (PB14) y p12 (PB15) ya no son botones: son entradas de camara de contacto seco,
// INPUT pelado y ACTIVAS EN ALTO (ver pines.h). Viven en botones.cpp, y no en un modulo
// propio, porque J16 TIENE UN SOLO DUENO: el fichero que declara sus pines es el mismo que
// los lee en cada vuelta. Partir el conector entre dos modulos es como un pin acaba con
// dos pinMode() de modos distintos y gana el que corra el ultimo.
//
// Lo que hacen las dos camaras es PEDIR PASO, por la misma puerta que ya existia en las
// dos puntas -demanda_solicitar()-, que es donde esta escrita la diferencia entre pedir y
// decidir (SFTY-27). No encienden nada: solo semaforo.cpp escribe pines de luz.

// Lectura antirrebotada de una entrada de camara. Devuelve true con el contacto CERRADO.
//
// Es publica porque el Modo Inteligente del Maestro tambien lee asi la camara de PB0: una
// sola definicion de "que es una deteccion" para las tres entradas y para las dos puntas.
bool camara_leerPin(uint8_t pin);

// ---------------------------------------------------------------------------
// LA PRESENCIA SOSTENIDA EN LAS DOS CAMARAS DE J16. 05/09/2026.
//
// ESTE GETTER SI ES DEL MAESTRO SOLO, Y NO ES UNA ASIMETRIA NUEVA: ES SFTY-27. El Esclavo
// PIDE y el Maestro DECIDE, asi que la unica punta que necesita saber "hay cola AHORA" es
// esta; en el Esclavo seria un huerfano por construccion -no tiene Modo Inteligente-. El
// bloque del vigilante de mas abajo si es identico en las dos.
//
// POR QUE HACE FALTA, Y ES LO QUE LA MEDIDA DEL 05/09 SACO: las camaras de J16 ya entraban
// en el Modo Inteligente -por flanco, via demanda_solicitar() y demanda_hayLocal()-, o sea
// que la frase "el modo no ve la camara de J16" era FALSA. Pero PEDIR y SOSTENER no son la
// misma pregunta:
//
//   PEDIR PASO es un suceso, y un flanco lo cuenta bien. Ese camino se queda como estaba.
//   ALARGAR UNA FASE es un nivel -"hay cola AHORA"-, y un flanco no puede contestarlo.
//
// La ventana de demanda_hayLocal() dura 3 s y NO se prolonga con cada deteccion: la
// siguiente peticion dentro de la ventana se descarta como peticion nueva, asi que entre
// dos peticiones aceptadas siempre queda un hueco en el que "hay cola" contesta que no. Y
// modo_inteligente.cpp muestrea esa respuesta en cada vuelta: basta caer en un hueco para
// que la fase termine en el suelo. Medido en el arnes, no razonado (Bloque F).
//
// Es exactamente el mismo reparto que el firmware ya hacia con J14: el Maestro lee PB0 POR
// NIVEL y el Esclavo POR FLANCO, y el comentario de botones.cpp lo explica con estas mismas
// dos palabras. Lo que faltaba era aplicarselo a las camaras nuevas.
bool camara_presenciaJ16();

// ---------------------------------------------------------------------------
// EL VIGILANTE DE LAS DOS CAMARAS - D-13 FASE 1 (A-6). 05/09/2026.
// ESTE BLOQUE ES IDENTICO EN LAS DOS PUNTAS, igual que el de arriba y por el mismo
// motivo: J16 tiene un solo dueno.
//
// POR QUE VA PRIMERO, ANTES QUE CUALQUIER VETO. Con INPUT pelado y el pull-down de 10K
// de la placa, una camara DESCONECTADA lee exactamente lo mismo que una camara que no
// ve a nadie: nivel bajo. EL PIN NO DISTINGUE SILENCIO DE VIA LIBRE. Todo lo que se
// apoye en ese bit -y la fase 2 se apoya entera- esta apoyado en un dato que no sabe
// decir que no sabe. Esto es lo que le ensena a decirlo.
//
// CERO EFECTO VIAL, Y NO COMO PROMESA SINO COMO PROPIEDAD MEDIBLE: desde aqui no se
// escribe ningun pin, no se llama al coordinador y no se toca la pluma. La fase 1 solo
// CUENTA y AVISA. El pack camara_03_vigilante lo censa en cada corrida.
//
// QUE VIGILA, Y QUE NO. Vigila CAM_C_PIN y CAM_D_PIN, que son las dos camaras de D-2.
// NO vigila CAM_DEMANDA_PIN (PB0): no es una de las dos camaras de D-13, y ademas es la
// unica entrada que las dos puntas leen DISTINTO -el Maestro por nivel y el Esclavo por
// flanco, que es SFTY-27 y esta razonado mas arriba-. Un vigilante que contara "flancos"
// de una entrada leida por nivel mediria cosas distintas en cada punta con el mismo
// codigo, y eso es peor que no medirlas.
// ---------------------------------------------------------------------------

// El estado de LA PEOR de las dos camaras, con los cuatro valores que D-13 le da al
// campo CAM: del $STATUS: "OK", "CIEGA", "PEGADA" y el "?" honesto de mientras no se
// sabe. Se publica la peor -y "?" pesa mas que "OK"- porque una camara de la que no se
// sabe nada no puede quedar tapada por la otra: eso seria pintar un dato que no se
// tiene, que es justo lo que este repositorio retiro de la app.
//
// YA TIENE LLAMADOR: el snprintf del $STATUS de bluetooth.cpp, desde D-13 (05/09). Aqui
// ponia "TODAVIA NO TIENE LLAMADOR, Y ESO ESTA MEDIDO, NO OLVIDADO", con la cuenta de
// bytes que en su dia lo impedia. Esa frase se quedo describiendo un equipo que ya no
// existe -es 2.ter: una frase que sostiene un verde y que no comprueba nadie- y se
// corrige en vez de borrarse, para que se vea que el trinquete de costura_10 hizo su
// trabajo cuando el getter gano llamador.
//
// LO QUE PUBLICA, Y LA UNICA VEZ QUE DICE "?": la peor de las dos camaras entre las que
// tienen algo que decir. Con UNA camara por poste el otro pin esta vacio y no puede dar
// un flanco nunca, asi que su "?" no cuenta -si no, el campo no podria decir "OK" en
// ningun equipo real-. Un contacto trabado SI cuenta aunque no haya dado flancos, porque
// PEGADA cuelga del nivel. Y mientras ninguna haya dicho nada -entre el arranque y la
// primera deteccion- el campo dice "?", que es lo unico cierto en ese rato.
const char* camara_estado();

// EL PRODUCTO DE LA FASE 1: cuantas veces HABRIA ACTUADO el veto de la pluma de la
// fase 2 -o sea, cuantas veces la pluma bajo habiendo presencia debajo-. No veta nada:
// cuenta, para que la decision de la fase 5 se tome sobre un numero y no sobre una
// impresion (D-13, orden de ejecucion).
//
// SATURA EN 65535 Y NO DA LA VUELTA. Un contador que vuelve a cero miente hacia abajo
// justo cuando lo que esta diciendo es "esto pasa mucho", que es la unica lectura por
// la que se construyo.
//
// El mismo numero sale ademas en cada $EVENT del vigilante, y sale POR ESTA FUNCION:
// asi lo que publica el aire y lo que devuelve el getter no pueden separarse.
uint16_t camara_vetosPluma();
