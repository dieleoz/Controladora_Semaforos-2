// ===== include/lcd.h =====
#pragma once
#include <Arduino.h>
#include "reloj.h"   // RelojDiag, para la pantalla de consulta del reloj (N-45)

void lcd_setup();

void lcd_dibujarBienvenida();
// Menu de lista. Sirve para el menu principal y para el submenu CONFIGURACION
// (V8.7): mismo dibujo, distinto array. Sin titulo se pinta "MODO SEMAFORO", que es
// lo que ya esperaban todas las llamadas anteriores.
void lcd_dibujarMenu(int cursor, const char* opciones[], int cantidad, const char* titulo = 0);
void lcd_dibujarManual(const char* nombreEstado);
void lcd_dibujarAutomatico(const char* nombreEstado, int minRojo, int minVerde);
void lcd_dibujarInteligente(const char* nombreEstado, int autosEsperando, bool iaActiva);
void lcd_dibujarConfigValor(const char* etiqueta, int valor, const char* unidad);

// Retiradas el 01/08/2026 por no tener ni un solo llamador: lcd_dibujarNoDisponible(),
// lcd_dibujarConfigMinutos() y lcd_dibujarTextoRecibido(). Verificado antes de
// borrarlas que tampoco las usaba el arnes de validacion ni el Esclavo.
// Ocupaban flash en un Maestro que va al 82 %, y ademas eran ruido: quien lea esta
// cabecera buscando que pantallas existen no debe encontrar tres que nadie dibuja.

// Pantalla de AJUSTE DE HORA (V8.6, SFTY-18).
//
// Se edita DIGITO A DIGITO, no el valor completo: con un solo boton de subir,
// poner los minutos costaria hasta 59 pulsaciones. Por digito son 9 como maximo.
// Ademas asi funciona igual con el mando de reles, que solo entrega pulsos y no
// permite una repeticion por mantener pulsado.
//
// digito: 0 = decena de hora, 1 = unidad de hora, 2 = decena de minuto,
//         3 = unidad de minuto. El activo se subraya.
// enHora: false si el reloj nunca se puso o la pila se agoto; se avisa en pantalla
// porque una hora no fiable no debe usarse para decidir nada.
// hayCristal: N-24, false cuando el oscilador Y2 no arranco. Con valor por defecto
// para no romper al arnes de validacion, que no conoce este caso.
void lcd_dibujarAjusteHora(uint8_t hora, uint8_t minuto, uint8_t digito, bool enHora,
                           bool hayCristal = true);

// Resultado del envio de la hora al Esclavo, N-23.
//
// Poner el reloj no es sincronizar: sincronizar es que el Esclavo lo acuse. Hasta
// esta pantalla el operario no tenia forma de saber cual de las dos cosas habia
// conseguido, y descubria la diferencia mucho despues, al ser rechazado por el Modo
// Degradado con un "nunca hubo sincronizacion RF" que no sabia a que atribuir.
//
// esperando  true mientras el intercambio esta en curso; ok se ignora.
// ok         resultado una vez terminado: el Esclavo aplico la hora o no contesto.
// sinReloj: N-30, true cuando el reloj propio no esta en hora. Entonces NO se envio
// nada y culpar al Esclavo seria falso; tiene prioridad sobre los otros dos.
void lcd_dibujarSyncHora(bool esperando, bool ok, bool sinReloj = false);

// --- MODO DEGRADADO (V8.7, SFTY-21) ---------------------------------------
//
// fase           texto grande: la luz que toca ahora ("VERDE" / "ROJO")
// detalle        que esta pasando ("Paso maestro", "Despeje total", ...)
// restanteSeg    cuenta atras al siguiente cambio, de ciclo_degradado_restante()
// minutosSinSync antiguedad de la ultima sincronizacion confirmada. Es el dato que
//                mide la deriva acumulada entre los dos relojes, y por tanto el
//                unico que dice si el modo sigue siendo defendible
// aviso          texto de alarma para la fila inferior, o 0 si no hay nada que avisar
void lcd_dibujarDegradado(const char* fase, const char* detalle,
                          unsigned long restanteSeg, unsigned long minutosSinSync,
                          bool syncVencida = false, const char* aviso = 0);

// Entrada rechazada: dice CUAL de las condiciones falta, no un "no se puede" mudo.
// N-31 — resultado del reinicio del dominio de respaldo.
// arranco=true: el estado sucio ERA la causa y el oscilador ya corre; hay que volver
// a poner la hora, porque el reinicio la borro. false: no era el estado, es Y2.
void lcd_dibujarReinicioReloj(bool arranco);

// N-45 — CONSULTA DEL RELOJ. Los bits tal cual, para que el tecnico decida.
//
// Sustituye a "Revisa Y2, pila y R5", que era una conclusion escrita a mano: el
// firmware nunca midio la pila -el F103 no tiene canal de ADC para VBAT- ni el
// cristal. Aqui no se acusa a ningun componente; se ensena lo que el micro ve.
// `latido` alterna en cada repintado y pinta un punto en la esquina. Sin el, un
// contador detenido y una pantalla congelada se ven EXACTAMENTE igual, que es la
// duda que esta pantalla existe para resolver.
void lcd_dibujarDiagnosticoReloj(const RelojDiag& d, bool latido);

void lcd_dibujarDegradadoRechazo(const char* linea1, const char* linea2);

// Ambar intermitente con motivo: limite duro de 48 h, o peticion desde el mando.
void lcd_dibujarDegradadoAmbar(const char* linea1, const char* linea2);

// Pantalla dedicada de PRUEBA DE ALCANCE (V8.1, ampliada en V8.2).
// calidadPct: 0..100, o -1 si aun no hay muestras.
// bytes/validas: contadores de linea (SFTY-15) para distinguir "no llega nada"
// de "llega basura" sin necesidad de instrumentos.
void lcd_dibujarAlcance(int calidadPct, unsigned long rttMs, int latidosPerdidos,
                        unsigned long bytes, unsigned long validas);