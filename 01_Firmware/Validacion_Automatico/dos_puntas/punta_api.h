// ===== Validacion_Automatico/dos_puntas/punta_api.h =====
//
// EL CONTRATO ENTRE EL ORQUESTADOR Y CADA PUNTA.
//
// Cada punta se compila en su PROPIA DLL. El orquestador las carga con LoadLibrary y
// resuelve estas funciones con GetProcAddress, de modo que nunca tiene en su tabla de
// simbolos ni un solo nombre del firmware: por eso puede haber dos semaforo_estado()
// -uno del Maestro, otro del Esclavo- vivos a la vez en el MISMO proceso sin que
// choquen. Ver la cabecera de orquestador.cpp para por que se descartaron las otras
// tres formas de resolver ese choque.
//
// TODAS LAS FUNCIONES SON extern "C". No es cosmetico: GetProcAddress busca por nombre
// literal, y el mangling de C++ de MinGW no es un nombre que se pueda escribir a mano
// sin equivocarse.
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#ifdef PUNTA_EXPORTA
#define PUNTA_API __declspec(dllexport)
#else
#define PUNTA_API
#endif

// Como se llama esta punta ("MAESTRO" / "ESCLAVO"). El orquestador lo compara contra
// la DLL que creia estar cargando: cargar dos veces la misma por un error de ruta
// daria un arnes que mide una punta contra si misma y no lo notaria nadie.
PUNTA_API const char* punta_nombre(void);

// Arranque de la punta. Ver cada adaptador para QUE ejecuta exactamente (el Esclavo
// llama a su setup() real; el Maestro no tiene un setup() aislable y arranca por el
// mismo camino que el arnes de una punta).
PUNTA_API void punta_arrancar(void);

// Una vuelta del bucle de esta punta, con el reloj simulado puesto en 'ms'.
//
// EL RELOJ LO PONE EL ORQUESTADOR, Y ES EL MISMO NUMERO PARA LAS DOS PUNTAS. Ese es
// el mecanismo que hace que "verde a la vez" signifique algo: las dos avanzan al
// mismo instante y se observan entre tick y tick.
PUNTA_API void punta_tick(unsigned long ms);

// --- OBSERVACION -----------------------------------------------------------
// Lo que semaforo.cpp ESCRIBIO en el pin. No lo que la logica pretendia.
PUNTA_API int punta_pin(int pin);
PUNTA_API int punta_estado(void);              // EstadoSemaforo como int
PUNTA_API unsigned long punta_escrituras(void);

// --- RADIO -----------------------------------------------------------------
// La punta no habla con la otra: habla con el orquestador, que es el canal. Asi se
// pueden tirar tramas, retrasarlas o cortar el enlace en un instante elegido.
//
// La trama son los 4 bytes de RF_Packet tal cual (msgID, command, param, crc). El
// adaptador comprueba en tiempo de compilacion que RF_Packet siga midiendo 4.
PUNTA_API int  punta_tx(unsigned char* trama4);        // 1 si habia algo que enviar
PUNTA_API void punta_rx(const unsigned char* trama4);  // entrega una trama a la punta

// --- ESCENARIO -------------------------------------------------------------
PUNTA_API void punta_entrada(int pin, int nivel);   // mueve una entrada digital
PUNTA_API void punta_pulsar(int boton);             // 1=A/Arriba 2=B/Abajo 3=Aceptar 4=Cancelar

// Consultas y ordenes con nombre. Cada punta expone cosas distintas -el Maestro tiene
// coordinador, el Esclavo tiene Modo Degradado- y una API fija con un hueco por cada
// getter de cada punta seria una tabla que envejece sola.
//
// CONTRATO DURO: una clave que la punta no conoce devuelve PUNTA_DESCONOCIDO y el
// orquestador ABORTA. Devolver 0 en silencio convertiria una errata en un PASS.
#define PUNTA_DESCONOCIDO ((long)0x7FFFFFFF)
PUNTA_API long punta_mando(const char* que, long arg);

// --- LO QUE SOBREVIVE AL CORTE DE ENERGIA -----------------------------------
//
// UN MICROCORTE EN ESTE ARNES ES UN FreeLibrary + LoadLibrary DE ESA PUNTA. Windows
// vuelve a mapear la DLL con su .data reinicializada y su .bss a cero, o sea que TODAS
// las estaticas del firmware -las de semaforo.cpp, las del despachador, las del Modo
// Degradado- vuelven a su valor de arranque. Es un arranque en frio de verdad, y no
// una funcion reset() escrita a mano que puede olvidarse una variable: la garantia la
// da el cargador del sistema, no una lista que alguien mantiene.
//
// Pero un corte de energia NO borra el dominio de respaldo: la pila CR2032 mantiene
// BKP->DR1..DR10 y el contador del RTC, y de eso depende que el Modo Degradado pueda
// REANUDAR en vez de caer a ambar (N-20). Por eso el orquestador vuelca ese dominio
// antes de descargar la DLL y lo repone despues: lo que se pierde es la RAM, no la
// pila.
//
// Indices: 0..9 = BKP->DR1..DR10. 10 = contador de segundos del RTC. 11 = el RTC
// esta en hora (0/1). 12 = segundos del dia. 13 = dia del mes.
#define PUNTA_DOMINIO_PALABRAS 14
PUNTA_API long punta_dominio_leer(int indice);
PUNTA_API void punta_dominio_escribir(int indice, long valor);

#ifdef __cplusplus
}
#endif
