// ===== include/mando.h (ESCLAVO) =====
#pragma once
#include <Arduino.h>

// ---------------------------------------------------------------------------
// SFTY-21 / N-19 — MANDO DE 4 RELES, LADO ESCLAVO.
//
// Portado del Maestro, donde ya esta validado. Las secuencias, las ventanas de
// tiempo y la realimentacion por destellos son LAS MISMAS; lo que cambia son las
// acciones, porque el Esclavo no tiene modos de operacion propios (ver menu.h).
//
// POR QUE ENTRA HOY, CON EL RECEPTOR SIN COMPRAR
// ----------------------------------------------
// El rele va EN PARALELO con los botones fisicos (PB9, PB13, PB14, PB15; no hay
// entradas dedicadas), asi que el firmware no distingue un dedo de un rele. Si las
// secuencias entran ahora, el dia que se instale el receptor es SOLO CONECTARLO:
// sin volver a flashear y sin subir a 5 m con el equipo en servicio. Hacerlo al
// reves obligaria a una intervencion mas en el gabinete por cada punta.
//
// Y hace falta de verdad: el procedimiento del Modo Degradado exige activarlo EN LAS
// DOS PUNTAS por separado, y el Maestro no puede ordenarselo por radio porque EL
// RADIO MUERTO ES LA RAZON DE ENTRAR AL MODO. Sin mando, esta punta solo se activa
// subiendo al gabinete.
//
// AL COMPRAR EL RECEPTOR: exigir codigo INDEPENDIENTE del mando del Maestro. Si los
// dos responden al mismo mando -y las dos puntas estan a menos de una cuadra-, una
// sola secuencia meteria las dos en Degradado a la vez, saltandose la verificacion
// por separado que justifica todo el diseno.
//
// RESTRICCIONES MEDIDAS EN CAMPO (01/08/2026), no negociables:
//
//   - El rele da UN PULSO POR FLANCO. Sostener el boton 10 s da un solo pulso:
//     LA PULSACION LARGA NO EXISTE, y cualquier diseno que la use es papel mojado.
//   - Cada pulsacion tarda ~2 s en conmutar. Una ventana de 3 s es inviable; hacen
//     falta 12-18 s para 3-4 pulsos.
//   - No hay repeticion automatica: cada pulso exige una pulsacion.
//
// SOLO SE USAN A (Boton 1) Y B (Boton 2). NUNCA C NI D.
//
// El riesgo grave no es el falso positivo: es que la pulsacion llegue cuando el
// sistema esta en un sitio distinto del que el operario cree, y a ciegas eso siempre
// es posible. En el Esclavo, C (ACEPTAR) es el boton que CONFIRMA la entrada al Modo
// Degradado y D (CANCELAR) el que navega hacia atras; A y B solo mueven el cursor
// entre dos opciones y repetirlos no hace absolutamente nada.
// ---------------------------------------------------------------------------

enum PulsoMando { MANDO_A = 0, MANDO_B = 1 };

void mando_setup();

// La llama botones.cpp por cada flanco de A o B, antes de que ninguna pantalla
// consuma el boton. No consume nada: solo observa.
void mando_registrarPulso(uint8_t boton);

// Maquina de la confirmacion y de la accion diferida. Va al FINAL del loop principal.
void mando_actualizar();

// ---------------------------------------------------------------------------
// AMBAR PEDIDO DESDE EL MANDO (secuencia B.B.B)
//
// Mientras vale true, este nodo NO OBEDECE las ordenes de luz del Maestro: se queda
// en ambar intermitente hasta que alguien haga A.A.A. Lo consulta main.cpp.
//
// Es una desobediencia deliberada y tiene que serlo. B.B.B existe para que nadie
// quede atrapado con un semaforo en un estado raro a 5 m de altura, y un "ambar" que
// el Maestro pudiera pisar con el siguiente CMD_GO_GREEN no serviria para nada: el
// operario estaria bajo una luz que vuelve a dar paso mientras el trabaja.
//
// Por eso main.cpp tampoco acusa recibo de esas ordenes. Contestar ACK sin encender
// la luz seria MENTIR al Maestro, que seguiria dando verde a su lado creyendo que
// aqui hay rojo: una punta en verde confiado contra otra en ambar es exactamente la
// asimetria peligrosa que todo SFTY-21 existe para evitar. Callando, el Maestro
// agota sus reintentos, cae a C_FALLO en ~12,5 s y se va tambien a ambar, que es el
// unico final correcto: el operario pidio ambar en una punta y el cruce entero
// termina en ambar.
bool mando_ambarLocal();
