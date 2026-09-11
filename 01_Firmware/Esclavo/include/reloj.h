// ===== include/reloj.h (ESCLAVO) =====
#pragma once
#include <Arduino.h>

// ---------------------------------------------------------------------------
// D-20: LA AUTORIDAD DE LA HORA ES EL ESP32 (DS3231).
// El STM32 siembra su base de tiempo de software a partir del ESP32 / radio.
// D-9: LA HORA DEL POSTE NO LA PONE ESTE MODULO. La lleva el reloj con pila del modulo de
// expansion, y este micro no tiene ninguno que sirva: su cristal esta CONFIRMADO MUERTO
// en banco (N-17). Lo de abajo describe el diseno con el que se escribio el fichero y se
// deja como esta -tachado, no borrado- porque el codigo sigue compilando y el contador
// crudo se sigue usando para fechar; lo que ya no es cierto es la premisa de que este sea
// el reloj del equipo.
//
// D-15: y por eso esta punta tampoco ACUSA la orden de poner la hora. El unico que
// contesta es quien tiene el reloj, y no es este. Cuando contestaban los dos, una sola
// orden producia dos acuses opuestos y los dos eran ciertos.
//
// OJO AL LEER LO DE ABAJO: dice que el cristal "ya viene en la tarjeta" y que la via para
// poner la hora es la radio. Lo primero es cierto en el cobre y falso como capacidad; lo
// segundo describe un camino que hoy no corre, y el porque medido esta escrito en la rama
// del despachador de Bluetooth que atiende esa orden.
//
// SFTY-18 / SFTY-23 — Reloj de tiempo real (RTC interno del STM32)
//
// Portado del Maestro. Usa el RTC que el propio microcontrolador lleva dentro,
// con el cristal Y2 de 32.768 kHz que ya viene en la tarjeta y una pila CR2032
// en VBAT. No ocupa ningun pin, cosa que aqui importa tanto o mas que en el
// Maestro: el I2C por hardware esta copado y un modulo externo habria obligado a
// I2C por software. Ver 03_Hardware_Tarjeta/MAPEO_TARJETA_KICAD.md seccion 4.
//
// POR QUE EL ESCLAVO NECESITA RELOJ (SFTY-23):
// no es para decidir nada por su cuenta hoy, sino para poder ser PUESTO EN HORA
// desde el Maestro por radio y para poder MEDIR su propio desfase. Ajustar a mano
// las dos puntas dejaba hasta 59 s de diferencia el primer dia, casi cuatro veces
// el todo-rojo, y dos pantallas en HH:MM no pueden detectarlo. El dia que exista
// el Modo Degradado, esa base de tiempo comun es lo que permitira que las dos
// puntas sigan en fase sin radio.
//
// DIFERENCIAS RESPECTO A LA VERSION DEL MAESTRO:
//
//  1. NO se porta la franja nocturna (reloj_ajustarFranjaNocturna,
//     reloj_inicioNoche, reloj_finNoche, reloj_esHorarioNocturno).
//     La operacion intermitente por horario es una decision de CICLO, y el ciclo
//     lo decide el Maestro: el Esclavo solo obedece ordenes de luz. Duplicar aqui
//     una franja que ademas solo vive en RAM crearia una segunda fuente de verdad
//     que tras un apagon revertiria en silencio a 22-05 y podria discrepar de la
//     del Maestro. Dos puntas decidiendo por separado cuando es de noche es
//     exactamente el fallo que este proyecto no se puede permitir. Si algun dia el
//     Esclavo tiene que entrar en intermitente nocturno, lo hara porque el Maestro
//     se lo ordena, no porque mire su propio calendario.
//
//  2. NO se porta reloj_textoHora(): esa funcion existe para pintar "HH:MM" en la
//     LCD, y el Esclavo no tiene pantalla ni menu.
//
// Se conserva reloj_segundosDelDia() porque el Modo Degradado necesitara una base
// absoluta de tiempo para computar la fase del ciclo sin radio.
// ---------------------------------------------------------------------------

void reloj_setup();

// El RTC arranca sin fecha valida la primera vez (pila recien puesta o agotada).
// Mientras devuelva false, la hora de este nodo NO es de fiar y no debe usarse
// para nada: un reloj sin poner en hora es peor que no tener reloj. En SFTY-23
// esto es lo que obliga a contestar DELTA_FUERA_DE_RANGO en vez de un numero
// inventado que el operario leeria como un desfase real.
bool reloj_enHora();

// N-49 — El contador crudo del RTC: 32 bits de SEGUNDOS que mantiene la pila.
//
// Es la unica medida de tiempo de esta punta que sobrevive al corte Y es monotona.
// La hora de pared no sirve para fechar: el calendario esta anclado a enero y el dia
// vuelve de 31 a 1. Devuelve 0 si el RTC no esta operativo, y ese cero significa
// "no hay reloj": respaldo_marcarSync() y respaldo_horasDesdeSync() se abstienen.
//
// Tiene que ser IDENTICA a la del Maestro. Las dos puntas fechan la misma
// sincronizacion, y que cada una lo hiciera a su manera es de donde salio N-49.
uint32_t reloj_contadorSegundos();

// N-25 — reintento en segundo plano del cristal. Se llama desde el loop(). Adopta el
// reloj si el oscilador despierta despues del arranque, sin reiniciar.
void reloj_actualizar();

uint8_t reloj_hora();      // 0..23
uint8_t reloj_minuto();    // 0..59
uint8_t reloj_segundo();   // 0..59

// Dia del mes, 1..31. Devuelve 0 si el reloj no esta en hora.
//
// ~~Lo necesita el respaldo (N-20) para saber cuanto hace de la ultima sincronizacion a
// traves de un reinicio~~ -> CADUCADO: desde N-49 el respaldo fecha con
// reloj_contadorSegundos(), y el 11/09 (D-26) se retiro su ultimo lector en esta punta
// -la copia de la hora al RTC en reloj_actualizar()-. HOY NADIE LA LLAMA EN EL ESCLAVO, y
// se deja declarada a proposito: el dia lo IMPONE la radio (CMD_HORA_D) y las dos puntas
// tienen que poder contarlo igual; su gemela del Maestro si tiene lector
// (enviarHoraCompleta). costura_10 la lleva en su lista con este motivo: si gana un
// llamador aqui, el pack lo dira.
uint8_t reloj_dia();

// Segundos transcurridos desde medianoche: 0..86399.
// Devuelve 0 si el reloj no esta en hora.
//
// OJO: esto NO es la base del modo autonomo al perder el radio (SFTY-19 / N-9).
// Aquel modo sincroniza de forma RELATIVA al ultimo mensaje del Maestro y le basta
// millis(). Esta funcion queda para el Modo Degradado (aplazado), que si necesita
// tiempo absoluto comun a las dos puntas.
uint32_t reloj_segundosDelDia();

// Ajusta el reloj y lo marca como valido.
// En el Esclavo la llama UNICAMENTE el manejador de CMD_HORA_S de main.cpp, y solo
// con las CUATRO cifras completas: aqui no hay teclado con el que un operario pueda
// ponerlo en hora, la unica via es la radio.
//
// D-20 (11/09): la radio SOBRESCRIBE siempre, tenga esta punta hora o no: es "el Maestro
// manda la hora y el Esclavo hace caso". ~~La hora del ESP32 de este poste, en cambio,
// solo entra si no hay hora~~ -> D-26 (3), 11/09 tarde: la del ESP32 entra si la radio NO
// manda, ver reloj_radioManda() abajo y la rama CMD:HORA_ESP32 de bluetooth.cpp.
//
// D-26 (3): ES LA UNICA FUNCION QUE MARCA LA HORA COMO "DE RADIO" (FH_RADIO en
// reloj.cpp), y lo es porque en esta punta su UNICO llamador es la rama CMD_HORA_S de
// main.cpp. Esa afirmacion la recalcula reloj_03 en cada corrida: si reloj_ajustar()
// gana otro llamador, la fuente de la hora dejaria de decir la verdad.
//
// `dia` (1..31) fija ademas el dia del mes. Con 0 -el valor por defecto- la fecha no
// se toca; se mantiene ese caso para que la firma sea la misma que en el Maestro,
// donde la pantalla de ajuste teclea solo HH:MM. Un dia fuera de 0..31 descarta la
// llamada entera, igual que una hora imposible: por radio la trama pudo llegar
// corrupta y colar por el CRC, y media hora escrita es peor que ninguna.
//
// POR QUE VIAJA EL DIA (CMD_HORA_D): para que las dos puntas cuenten los dias con el
// MISMO numero. No interesa la fecha real, interesa que esten ACOPLADAS. Hasta ahora
// cada unidad sembraba su propio dia 1 la primera vez que se ponia en hora, y los
// calendarios quedaban desacoplados para siempre. Consecuencia medida por el
// validador de costura: un corte de energia el dia en que el calendario de UNA punta
// pasa de 31 a 1 hace que respaldo_horasDesdeSync() declare CADUCADA solo en esa
// punta; esa no reanuda el Modo Degradado y se queda en AMBAR, mientras la otra
// reanuda en fase y sigue dando VERDE cada ciclo. Ambar contra verde es el peor
// resultado posible. Con los calendarios acoplados las dos fallan a la vez:
// simetrico y seguro.
void reloj_ajustar(uint8_t hora, uint8_t minuto, uint8_t segundo = 0, uint8_t dia = 0);

// D-20 / N-160 - LA FUNCION QUE SI PUEDE CONTESTAR, Y POR QUE HUBO QUE ANADIRLA.
//
// reloj_ajustar() rechaza en silencio -"if (hora > 23 ...) return;"- y es void, asi que
// quien la llama no puede saber si la hora entro. Sobre eso se construyo un $ACK que no
// dependia de lo que la llamada hizo, que es el patron que mas defectos ha dado aqui.
//
// LA FIRMA DE reloj_ajustar() NO SE PUDO CAMBIAR A bool, y esto es una MEDIDA, no una
// opinion: tres arneses la DOBLAN con la firma void y compilan contra este mismo
// header -Validacion_Automatico/dos_puntas/adaptador_esclavo.cpp y
// adaptador_maestro_deg.cpp con las cuatro cifras, Validacion_LCD/arnes_esclavo.cpp con
// tres-. Cambiar el retorno aqui los rompe con "ambiguating new declaration", medido
// con g++. Por eso la regla de rango se MUDA a esta funcion y reloj_ajustar() pasa a ser
// su envoltorio: la regla sigue viviendo en UN SOLO SITIO -no hay copia que se pueda
// desincronizar- y el que contesta ya tiene a quien preguntar.
//
// TOMA int Y NO uint8_t A PROPOSITO. El unico llamador que necesita el acuse parsea con
// sscanf("%d"), y castear a uint8_t ANTES de validar convierte un h=256 en 0 -o sea en
// medianoche-, que pasa la guarda sin que nadie note nada. Validando el int se ve el
// valor que de verdad llego. dia = 0 sigue significando "no toques la fecha", asi que es
// valido y devuelve true.
bool reloj_ajustarConAcuse(int hora, int minuto, int segundo, int dia);

// D-20: siembra desde "YYYY-MM-DD,HH:MM:SS" -exactamente esa forma, 19 caracteres: ver
// isoBienFormado() en reloj.cpp-. Desde el 11/09 su unico llamador es la rama
// CMD:HORA_ESP32 de bluetooth.cpp: la hora del DS3231 de SU PROPIO ESP32, que en esta
// punta solo se siembra si reloj_radioManda() dice que no (D-26 (3)). Si entra, la hora
// pasa a ser "del ESP32" (FH_ESP32).
bool reloj_sembrarDesdeIso(const char* str);

// ---------------------------------------------------------------------------
// D-26 (3) - QUIEN MANDA LA HORA EN ESTA PUNTA: "CON RADIO, LA DEL MAESTRO; SIN RADIO,
// LA DE SU PROPIO ESP32".
//
// POR QUE NO SE REUSA horaValida (reloj_enHora()). Esa bandera contesta "hay hora?", y
// esto pregunta "DE QUIEN es la hora que hay?". Son dos preguntas y por eso son dos
// variables (CLAUDE.md 8): horaValida puede nacer en true de un RTC de hardware que
// arranco con valores plausibles -o congelado, con Y2 muerto-, y con la regla "solo si no
// hay hora" ese reloj habria vetado la del ESP32 para siempre. La fuente vive en reloj.cpp
// (NINGUNA < RTC_HW < ESP32 < RADIO) y la marcan SOLO los dos sembradores: reloj_ajustar()
// -la radio- y reloj_sembrarDesdeIso() -el ESP32-.
//
// "SIN RADIO", DEFINIDO CON UNA CONSTANTE QUE YA EXISTE: ninguna trama valida del Maestro
// en SFTY6_SILENCIO_MS (protocolo.h), el MISMO silencio con el que main.cpp manda esta
// punta a ambar y publica $ALARM FALLO_RF. Es a proposito que sean el mismo numero: esa
// alarma es la que manda al usuario a este poste a ponerle la hora con el telefono
// (D-26 (5)), y esa hora tiene que ENTRAR. Con dos umbrales habria un hueco en el que la
// alarma ya salio y la hora que el usuario pone se sigue ignorando.
//
// Y NO SE MIDE CON tUltimoComando DE main.cpp, aunque use la misma constante: aquel
// contesta "el Maestro GOBIERNA el cruce?" -no lo refrescan las tramas de servicio ni un
// PING en ambar- y esto contesta "LLEGA la radio del Maestro?". Con el Esclavo en ambar
// por la app y el Maestro latiendo, tUltimoComando envejece y la radio esta viva: medido
// con aquel, esta punta alternaria cada pocos minutos entre la hora de su ESP32 y la del
// Maestro. Por eso cualquier trama valida del Maestro cuenta aqui (reloj_notarRadio()).

// La llama main.cpp con CADA trama valida que entrega la radio, de cualquier comando.
void reloj_notarRadio();

// true si la hora de esta punta la manda la radio: la ultima siembra fue del Maestro Y
// su radio se oyo en los ultimos SFTY6_SILENCIO_MS. Con la radio callada, o sin haber
// recibido nunca la hora por radio, devuelve false y la del ESP32 entra.
bool reloj_radioManda();

// ---------------------------------------------------------------------------
// D-26 (2) y (5) - LO QUE ESTA PUNTA ESPERA DE SU ESP32. Otro binario: los numeros se
// escriben aqui y esp32_13 los compara en cada corrida con los de
// ESP32_Expansion/include/contrato.h.
//
// La cadencia con la que el ESP32 siembra (SIEMBRA_INTERVALO_MS alli). IDENTICA en las
// dos puntas y en el ESP32: si difieren, la alarma de abajo salta con el enlace sano o no
// salta con el enlace caido.
static const unsigned long HORA_ESP32_CADENCIA_MS = 300000UL;

// D-26 (5): sin una HORA_ESP32 bien formada en TRES cadencias, $ALARM EVENTO:HORA_ESP32.
// Tres y no una: una siembra perdida -el ESP32 reiniciando, un byte comido- no es una
// averia; tres seguidas, si. Y se repite cada tanto mientras dure, porque el que la tiene
// que ver es el tecnico que se conecte DESPUES, no el que estaba conectado cuando empezo.
static const unsigned long HORA_ESP32_ESPERA_MAX_MS = 3UL * HORA_ESP32_CADENCIA_MS;

// Lo que el HSI de este micro puede desviarse en el peor caso de su ficha. El MISMO numero
// que en el reloj.h del Maestro -el micro es el mismo STM32F103- y con el mismo porque
// (alli, sobre HSI_PPM_PEOR). Faltaba aqui porque ningun calculo de esta punta lo usaba;
// desde D-21 (1) lo usa la caducidad de abajo, y reloj_04 exige que las dos copias valgan
// lo mismo.
static const unsigned long HSI_PPM_PEOR = 25000UL;

// ---------------------------------------------------------------------------
// D-21 (1) - LA CADUCIDAD DE LA SIEMBRA. Gemela de la del Maestro, con el porque entero en
// su reloj.h. El plazo es el tiempo en que el HSI acumula en el peor caso la deriva de UNA
// cadencia, que es lo que la cuenta del cruce (esp32_13) le concede a cada punta; reloj_04
// recalcula la desigualdad contra el aguante y exige que las dos puntas digan lo mismo.
//
// AQUI PESA DISTINTO QUE EN EL MAESTRO, y se dice: esta punta tiene DOS sembradores. Con
// radio la siembra es la del Maestro (CMD_HORA_S) y la de su ESP32 se IGNORA; sin radio,
// al reves (D-26 (3)). Cualquiera de las dos que entre renueva el plazo, porque las dos
// pasan por reloj_ajustarConAcuse(). En el RELEVO -la radio calla y la primera siembra del
// ESP32 aun no ha llegado- la hora puede tener hasta una cadencia de la radio mas la espera
// de SFTY6_SILENCIO_MS mas una cadencia del ESP32: MAS que este plazo. En ese rato esta
// punta NO entra en Degradado, y si ya estaba dentro se rinde. La cuenta la publica reloj_04.
static const unsigned long HORA_DERIVA_S =
    (HORA_ESP32_CADENCIA_MS / 1000UL * HSI_PPM_PEOR + 999999UL) / 1000000UL;
static const unsigned long HORA_CADUCA_MS = HORA_DERIVA_S * 1000000UL / HSI_PPM_PEOR * 1000UL;
static_assert(HORA_CADUCA_MS >
                  HORA_ESP32_CADENCIA_MS + HORA_ESP32_CADENCIA_MS / 1000UL * HSI_PPM_PEOR / 1000UL,
              "D-21 (1): una siembra normal caducaria antes de llegar con el HSI rapido");

// true si esta punta tiene hora Y su ultima siembra buena tiene como mucho HORA_CADUCA_MS.
// Caducada se queda caducada hasta la siguiente siembra. Una hora que vino SOLO del RTC de
// hardware no caduca aqui (no corre sobre el HSI): el borde y el porque, en el reloj.h del
// Maestro. Solo la preguntan la puerta y el bucle del Degradado; reloj_enHora() sigue
// contestando "hay hora?" para todo lo demas.
bool reloj_horaFiable();
