// ===== include/interruptores.h =====
#pragma once

// ---------------------------------------------------------------------------
// SPEC_8 6 - LOS DOS INTERRUPTORES DE ADMINISTRADOR (D-94..D-97, D-148..D-152).
//
// ESTE FICHERO Y SU .cpp SON IDENTICOS EN MAESTRO Y ESCLAVO. Las ordenes son LOCALES al
// poste conectado y no viajan por radio (D-94): cada punta atiende la suya.
//
// La bandera vive en el respaldo (D-95, respaldo.cpp) y quien la obedece es escribirPines()
// (D-96, semaforo.cpp). Aqui solo esta la orden, lo que se publica y el aviso de arranque.
// ---------------------------------------------------------------------------

// D-94: SET_VETO_CAMARA:<ON|OFF> o SET_BARRERA_SERVICIO:<ON|OFF>, ya sin el "CMD:PIN:...:".
// D-166/D-169 (SPEC_8 9.5): SET_VETO_CAMARA:<C|D>:<ON|OFF>, SET_CAMARA_MALA:<1|2>:<BAJA|ARRIBA> y
// CONSULTA_VETO (sin PIN: el despachador la deja pasar).
// Devuelve la trama que se contesta, o nullptr si la accion no es de este modulo. Molde
// SET_TIEMPOS: el RESULT:OK sale de lo leido DESPUES de escribir y reentrar (D-149).
const char* interruptores_orden(const char* accion);

// D-151: true si el campo CAM: dice NOVETO -alguna camara fuera y camara_estado() en OK o '?'-.
// Con PEGADA o CIEGA se publica eso aunque el veto este fuera. D-166: camara_estado() ya salta las inhibidas.
bool interruptores_camNoVeto();

// D-152: un $EVENT de una sola vez si al arrancar sigue puesta alguna bandera (D-166: por camara). Se llama en
// setup(), despues de respaldo_setup() y de bluetooth_setup().
void interruptores_avisoArranque();
