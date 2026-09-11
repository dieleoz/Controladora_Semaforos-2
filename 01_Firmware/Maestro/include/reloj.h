// ===== include/reloj.h =====
#pragma once
#include <Arduino.h>

// ---------------------------------------------------------------------------
// SFTY-18 — Reloj de tiempo real (RTC interno del STM32)
//
// D-20: LA AUTORIDAD DE LA HORA ES EL ESP32 (DS3231).
// El STM32 siembra su base de tiempo de software a partir del ESP32.
// Usa el RTC que el propio microcontrolador lleva dentro, con el cristal Y2 de
// 32.768 kHz que ya viene en la tarjeta y una pila CR2032 en VBAT.
// No ocupa ningun pin: el I2C por hardware esta copado (PB6/PB7 los usa la LCD y
// PB10/PB11 el RS-485), asi que un modulo externo habria obligado a I2C por
// software. Ver 03_Hardware_Tarjeta/MAPEO_TARJETA_KICAD.md seccion 4.
//
// Sirve para la operacion intermitente nocturna: sin saber la hora, el equipo no
// puede decidir cuando entrar en ese modo.
// ---------------------------------------------------------------------------

void reloj_setup();

// El RTC arranca sin fecha valida la primera vez (pila recien puesta o agotada).
// Mientras devuelva false, la funcion nocturna NO debe activarse: un reloj sin
// poner en hora encenderia el modo intermitente a deshora.
bool reloj_enHora();

// N-24 — ¿arranco el oscilador del cristal Y2? Distinto de reloj_enHora(), y la
// diferencia es la que el operario necesita ver:
//
//   enHora=false, hayCristal=true   -> el reloj cuenta, pero nadie lo ha puesto en
//                                      hora. Se arregla desde AJUSTAR HORA.
//   enHora=false, hayCristal=false  -> no hay con que contar. NO se arregla desde el
//                                      menu: es la pila, R5 o el propio cristal.
//
// Sin esta distincion, ajustar la hora parecia funcionar -la pantalla la mostraba- y
// al apagar y encender volvia a ceros, sin nada que explicara por que.
bool reloj_hayCristal();

// N-45 — CONSULTA DEL RELOJ: los bits crudos, sin interpretar.
//
// Hasta aqui, cuando el reloj no arrancaba la pantalla decia "Revisa Y2, pila y R5" y
// "Es Y2: toca hardware". Esos textos estaban ESCRITOS A MANO: el firmware no habia
// medido la pila -el STM32F103 no tiene canal de ADC para VBAT- ni el cristal. Eran
// conclusiones fijas presentadas como diagnostico, y mandaron a cambiar pila, R5 e Y2
// tres veces con el hardware sano.
//
// Esta estructura no concluye nada. Dice lo que el micro VE, y quien mira decide:
//
//   lseOn=1, lseRdy=0  -> el oscilador esta pedido y no arranca. Ahi si mira Y2, sus
//                         condensadores de carga y la soldadura.
//   lseOn=0            -> ni siquiera se esta pidiendo. Es firmware o dominio de
//                         respaldo bloqueado, NO es el cristal.
//   lseByp=1           -> espera reloj externo por OSC32_IN. Con un cristal normal
//                         nunca arrancara, y no es culpa del cristal.
//   rtcSel=0           -> el RTC no esta atado a ninguna fuente: no cuenta aunque el
//                         cristal oscile.
//   rtcEn=0            -> RTC deshabilitado. OJO: con esto en 0 no se leen sus
//                         registros de contador, y por eso cnt no se rellena.
//   cnt cambiando entre dos visitas -> el RTC CUENTA. Distingue "no cuenta" de
//                         "cuenta pero nadie lo ha puesto en hora".
struct RelojDiag {
  bool lseOn;
  bool lseRdy;
  bool lseByp;
  uint8_t rtcSel;    // 0=ninguna, 1=LSE, 2=LSI, 3=HSE/128
  bool rtcEn;
  bool cntLeido;     // false si rtcEn=0: no se toco el periferico
  uint32_t cnt;      // contador crudo del RTC, solo si cntLeido
  bool configurado;  // rtc.isConfigured()
  uint16_t anio;     // 0 si no se pudo leer
};

// Rellena la estructura leyendo RCC->BDCR y, solo si el RTC esta habilitado, su
// contador. No modifica NADA: se puede llamar desde cualquier pantalla sin efectos.
void reloj_diagnostico(RelojDiag* d);

// N-49 — El contador crudo del RTC: 32 bits de SEGUNDOS que mantiene la pila.
//
// Es la unica medida de tiempo de este equipo que sobrevive al corte Y es monotona.
// La hora de pared no sirve para fechar: el calendario esta anclado a enero (ver
// reloj_fijarEnero) y el dia vuelve de 31 a 1, asi que restar dias del mes no
// distingue "ayer" de "hace un mes y un dia". Este contador no vuelve en 136 anos.
//
// Devuelve 0 si el RTC no esta operativo, y ese cero es un valor con significado:
// respaldo_marcarSync() y respaldo_horasDesdeSync() lo tratan como "no hay reloj" y
// se abstienen, en vez de fechar contra un contador que nadie hace avanzar.
uint32_t reloj_contadorSegundos();

// N-25 — reintento en segundo plano del cristal. Se llama desde el loop(). Si el
// oscilador no arranco en el setup, lo vuelve a mirar cada 30 s y ADOPTA el reloj en
// cuanto despierte, sin reiniciar. Un cristal marginal o frio puede tardar mas de los
// 2 s del arranque, y condenarlo por eso seria confundir "lento" con "muerto".
// No hace nada si el RTC ya esta operativo.
void reloj_actualizar();

// N-31 — Reinicia el DOMINIO DE RESPALDO entero y reintenta arrancar el oscilador.
//
// Ultima carta antes de dar por muerto el cristal: si un firmware anterior dejo el
// LSE mal configurado, esos registros sobreviven a los reinicios -viven de la pila- y
// ningun arranque normal los limpia.
//
// BORRA LA HORA Y TODO EL RESPALDO (ciclo acordado, marca de sincronizacion,
// indicador del Degradado). Por eso lo pide una persona desde el menu y no se hace
// solo. Devuelve true si tras el reinicio el oscilador arranca.
bool reloj_reiniciarDominioRespaldo();

uint8_t reloj_hora();      // 0..23
uint8_t reloj_minuto();    // 0..59
uint8_t reloj_segundo();   // 0..59

// Mantiene el calendario anclado a ENERO.
//
// El Esclavo queda fijado a enero en cada sincronizacion, pero el mes del Maestro
// avanzaria solo (ene -> feb -> ...). Y el RTC decide cuando pasa de 31 a 1 SEGUN LA
// LONGITUD DEL MES: en febrero el Maestro volcaria 28->1 mientras el Esclavo, en
// enero, sigue en 29. Los dos calendarios volverian a separarse hasta la siguiente
// sincronizacion, y un corte de energia en esa ventana reproduce la asimetria
// ambar-contra-verde que CMD_HORA_D vino a cerrar.
//
// Anclando las dos puntas a enero, ambas vuelcan en 31 y en el mismo instante. No
// importa que el mes sea falso: el equipo no muestra la fecha ni la interpreta, solo
// resta dias. Lo que importa es que resten IGUAL.
//
// Se llama periodicamente; es idempotente y no toca la hora.
void reloj_fijarEnero();

// Dia del mes, 1..31. Devuelve 0 si el reloj no esta en hora.
//
// Lo necesita el respaldo (N-20) para saber cuanto hace de la ultima
// sincronizacion a traves de un reinicio: con solo los segundos del dia no se
// distingue "hace una hora" de "hace veinticinco". No interesa la fecha en si -el
// equipo no la muestra ni la usa para nada mas-, solo poder restar dias.
uint8_t reloj_dia();

// Segundos transcurridos desde medianoche: 0..86399.
// Devuelve 0 si el reloj no esta en hora.
//
// OJO: esto NO es la base del modo autonomo al perder el radio (SFTY-19 / N-9).
// Aquel modo sincroniza de forma RELATIVA al ultimo mensaje del Maestro y le basta
// millis(); no usa la hora absoluta ni obliga a poner pila en el Esclavo. Anclar
// las dos unidades a su reloj de pared seria peor: dos relojes puestos en hora a
// mano difieren desde el primer dia, mientras que la sincronizacion relativa
// arranca en cero.
// Esta funcion existe para la operacion intermitente NOCTURNA (N-3), aplazada.
uint32_t reloj_segundosDelDia();

// Ajusta el reloj y lo marca como valido.
//
// `dia` (1..31) fija ademas el dia del mes. Con 0 -el valor por defecto- la fecha
// no se toca, que es lo que necesita la pantalla de ajuste: el operario teclea
// HH:MM y no tiene por que saber la fecha.
//
// Por radio SI viaja el dia (CMD_HORA_D), para que las dos puntas cuenten los dias
// con el MISMO numero. No interesa la fecha real: interesa que esten acopladas. Con
// calendarios independientes, un corte el dia del cambio de mes deja a UNA punta
// sin reanudar -en ambar- mientras la otra reanuda y da verde.
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
// punta se acepta siempre (el Esclavo, en cambio, solo si la radio no manda: D-26 (3)).
bool reloj_sembrarDesdeIso(const char* str);

// ---------------------------------------------------------------------------
// D-26 (2) y (5) - LO QUE ESTA PUNTA ESPERA DE SU ESP32. Otro binario: los numeros se
// escriben aqui -y en el reloj.h del Esclavo- y esp32_13 los compara en cada corrida con
// los de ESP32_Expansion/include/contrato.h.
//
// La cadencia con la que el ESP32 siembra (SIEMBRA_INTERVALO_MS alli). Si difieren, la
// alarma de abajo salta con el enlace sano o no salta con el enlace caido.
static const unsigned long HORA_ESP32_CADENCIA_MS = 300000UL;

// D-26 (5): sin una HORA_ESP32 bien formada en TRES cadencias, $ALARM EVENTO:HORA_ESP32.
// Tres y no una: una siembra perdida -el ESP32 reiniciando, un byte comido- no es una
// averia; tres seguidas, si. Y se repite cada tanto mientras dure, porque quien la tiene
// que ver es el tecnico que se conecte DESPUES, no el que estaba conectado cuando empezo.
static const unsigned long HORA_ESP32_ESPERA_MAX_MS = 3UL * HORA_ESP32_CADENCIA_MS;

// Lo que el oscilador interno (HSI) de este micro puede desviarse en el PEOR caso de su
// ficha: +-1 % a 25 C y hasta -2 % / +2,5 % en -40..105 C (STM32F103, "HSI oscillator
// accuracy"). Entre dos siembras la hora se extrapola con millis(), que corre sobre el
// HSI, asi que esto es lo que decide cuanto se separa. Lo usa el static_assert de
// modo_degradado.cpp -una siembra NORMAL no puede mandar el Degradado a rojo- y lo relee
// esp32_13 para la cuenta de la cadencia contra el margen del cruce.
static const unsigned long HSI_PPM_PEOR = 25000UL;

// ---------------------------------------------------------------------------
// D-21 (1) - UNA HORA QUE NO ES FIABLE ES UNA HORA QUE MIENTE. LA CADUCIDAD DE LA SIEMBRA.
//
// Entre dos siembras la hora de esta punta se extrapola con millis(), o sea sobre el HSI.
// La cuenta del cruce que rehace esp32_13 le CONCEDE a cada punta la deriva de UNA cadencia
// (HORA_DERIVA_S, abajo) y deja el resto del aguante para lo que difieran los dos DS3231.
// Hasta hoy eso era una SUPOSICION: nada impedia que una punta con el J17 mudo siguiera
// dando verdes por reloj horas despues, a 36-90 s por hora (H1 del veredicto del 11/09).
//
// EL PLAZO NO SE ESCOGE: es el tiempo en que el HSI, en el peor caso de su ficha, acumula
// EXACTAMENTE la deriva que esa cuenta ya concede. Pasado, la punta esta fuera de lo que el
// cruce tiene presupuestado y su hora deja de ser fiable. Un plazo mayor no compraria nada:
// ni una siembra perdida ni el relevo de fuente de D-26 (3) caben por debajo del aguante
// -la cuenta la publica reloj_04-.
//
// La DESIGUALDAD contra el aguante del cruce la recalcula reloj_04 desde este fichero en
// cada corrida (N-71). El static_assert es solo el SUELO, que no necesita el modelo del
// ciclo: una siembra normal tiene que llegar antes de caducar aun con el HSI corriendo en su
// extremo rapido -si no, el Degradado caeria a ambar con el J17 sano-.
static const unsigned long HORA_DERIVA_S =
    (HORA_ESP32_CADENCIA_MS / 1000UL * HSI_PPM_PEOR + 999999UL) / 1000000UL;
static const unsigned long HORA_CADUCA_MS = HORA_DERIVA_S * 1000000UL / HSI_PPM_PEOR * 1000UL;
static_assert(HORA_CADUCA_MS >
                  HORA_ESP32_CADENCIA_MS + HORA_ESP32_CADENCIA_MS / 1000UL * HSI_PPM_PEOR / 1000UL,
              "D-21 (1): una siembra normal caducaria antes de llegar con el HSI rapido");

// true si esta punta tiene hora Y esa hora esta dentro de su plazo: la ultima siembra buena
// -de su ESP32, de la radio o de la pantalla, cualquiera que pase por reloj_ajustarConAcuse()-
// tiene como mucho HORA_CADUCA_MS. Una vez caducada se QUEDA caducada hasta la siguiente
// siembra (lo mantiene reloj_actualizar() en cada vuelta), para que la vuelta de millis() a
// los 49,7 dias no la rejuvenezca.
//
// EL BORDE, ESCRITO: una hora que vino SOLO del RTC de hardware -sin ninguna siembra en este
// arranque- NO caduca aqui. Esa no corre sobre el HSI sino sobre el cristal Y2, y la cubre el
// limite de 48 h del Degradado; desde D-20 no hay quien la escriba, y con Y2 muerto (N-17)
// ni siquiera nace valida.
//
// NO SUSTITUYE A reloj_enHora(), y son dos preguntas a proposito (CLAUDE.md 8): aquella
// contesta "hay hora?" y la leen la sincronizacion por radio, la medida de desfase y la
// telemetria, que tienen que seguir funcionando con una hora vieja. Esta contesta "puede
// esta hora decidir una luz?", y solo la preguntan la puerta y el bucle del Degradado.
bool reloj_horaFiable();

// 🔴 D-15 (05/09) - ESTA PUNTA YA NO TIENE CAMINO DE ESCRITURA, Y LO QUE ESO COSTO
// SE DEJA ESCRITO AQUI PORQUE ES DONDE HARA FALTA.
//
// Habia dos llamadores de reloj_ajustar(): la rama SET_RTC de bluetooth.cpp y la
// pantalla AJUSTAR HORA. La primera se retiro -el reloj es del DS3231 del ESP32 y solo
// el contesta a SET_RTC-, y la segunda cuelga de botonAceptar(), que es "return false"
// desde que PB14/PB15 pasaron a ser camaras (D-2). O sea que hoy NADIE puede poner en
// hora este RTC, y horaValida solo la puede encender reloj_setup()/reloj_actualizar()
// leyendo el dominio de respaldo -que con Y2 muerto (N-17) no arranca nunca-.
//
// CONSECUENCIA MEDIDA, NO DEDUCIDA: reloj_enHora() de esta punta es hoy FALSO SIEMPRE,
// y de esa bandera cuelga la autorizacion del Modo Degradado
// (modo_degradado.cpp: "if (!reloj_enHora()) return MDG_FALTA_HORA;"), la
// sincronizacion horaria por radio (coordinador_sincronizarHora) y la medida de desfase
// (coordinador_medirDesfase). Los tres estan bloqueados, y lo estaban ya antes de D-15
// por el cristal: D-15 no los rompe, los deja bloqueados POR CONSTRUCCION del fuente en
// vez de por una averia de hardware.
//
// 🔴 CADUCADO EL 07/09 POR D-20, Y SE TACHA EN VEZ DE BORRARSE PORQUE EXPLICA DE DONDE
// VIENE LO DE ABAJO. Los dos parrafos anteriores YA NO SON CIERTOS:
//
//   - "hoy NADIE puede poner en hora este RTC" -> SI se puede. D-20 abrio un camino
//     nuevo, reloj_sembrarDesdeIso(), con llamador real en bluetooth.cpp: hasta el 11/09
//     la rama SET_RTC -los bytes del telefono-, y desde el 11/09 la rama CMD:HORA_ESP32
//     -la hora que el DS3231 de su ESP32 releyo; SET_RTC ya es solo del puente-. No
//     escribe el RTC: siembra una base de software que se extrapola con millis(), que es
//     justo el punto de D-20 -el cristal Y2 no hace falta-.
//   - "reloj_enHora() es hoy FALSO SIEMPRE" -> es TRUE en cuanto entra la primera siembra.
//     Y con ella se desbloquean los tres de la lista, empezando por el Modo Degradado.
//
// UN .h QUE MIENTE SOBRE ESTA BANDERA ES CARO: es lo que lee el siguiente antes de
// tocar el Degradado, y le diria que un modo que ya arranca sigue muerto.
//
// LO QUE SIGUE SIENDO CIERTO, y por eso el bloque se queda: la pantalla AJUSTAR HORA
// sigue colgando de botonAceptar(), que es "return false" desde D-2, asi que por AHI
// no entra nada. Lo que cambio es que hay OTRA puerta, no que se arreglara aquella.
//
// LO QUE SE RETIRO CON EL CAMINO, Y POR QUE NO SE GUARDO DE ADORNO: reloj_invalidarHora()
// existia para N-144 -un ajuste que NO quedaba dejaba horaValida en true, el equipo
// publicaba HORA:00:00:00 y eso no es medianoche, es un contador parado declarandose
// valido-. Sin camino de escritura no hay ajuste fallido que retractar, asi que la
// funcion se quedaba sin sujeto: una huerfana con motivo escrito es "una lista de
// defectos con permiso" (CLAUDE.md 3.bis), y envejece peor que el codigo.
//
// 🔴 EL DIA QUE SE CABLEE EL DS3231 A ESTA PUNTA (via B del Manual 17 3.2, abierta como
// AB-4), N-144 VUELVE CON EL: quien escriba ese camino tiene que releer lo que escribio
// y RETIRAR la bandera si no cuadra, no solo avisar. Es la mitad que costo la cinta de
// campo del 04/09.
//
// 🔴 ESE DIA LLEGO EL 07/09 CON D-20, Y LA PRECONDICION DE ARRIBA NO SE CUMPLIO. El
// camino esta escrito -reloj_sembrarDesdeIso() con llamador real- y reloj_invalidarHora()
// NO se restauro. Medido al auditar (N-160): los unicos "horaValida = false" viven dentro
// de reloj_setup(); despues solo hay "= true". En el Esclavo eso deja la bandera en
// TRINQUETE de una sola direccion -alli ni siquiera existe reloj_reiniciarDominioRespaldo(),
// que es lo unico que la baja en esta punta-, y por eso la guarda de D-21 que el Esclavo
// estreno en su bucle de Degradado ES INALCANZABLE POR CONSTRUCCION.
//
// NO SE ARREGLA AQUI Y A PROPOSITO: cerrarlo bien es la pieza (A) de D-21 -que el OSF del
// DS3231 llegue a reloj_enHora()-, y hoy no hay ni una aparicion de OSF fuera de
// ESP32_Expansion/. Poner un invalidador sin esa fuente seria inventarse el criterio de
// cuando la hora deja de valer. Queda como decision abierta del responsable, con la medida
// delante, en roadmap.md 3.10.bis.

// --- Franja nocturna configurable -----------------------------------------
//
// ATENCION: HOY SE GUARDA SOLO EN RAM. No sobrevive a un corte de energia.
//
// Tras un apagon la HORA si sobrevive gracias a la pila, pero la franja revierte
// EN SILENCIO a 22-05. Y como la hora es valida, reloj_enHora() devuelve true y
// nada impide que el modo nocturno arranque con el horario equivocado de esa obra:
// la guarda de hora fiable NO cubre este caso.
//
// Por eso, mientras no exista la persistencia real (BKP->DR1..DR10; la libreria
// STM32duino RTC 1.9.0 no expone esos registros), la regla es:
//   franja NO confirmada tras el arranque  =>  modo nocturno INHIBIDO.
//
// Este comentario describia antes una persistencia que el codigo nunca tuvo. En un
// proyecto cuya trazabilidad se levanta buscando en los fuentes, esa mentira es
// peor que la carencia. Ver pendiente N-15.
void reloj_ajustarFranjaNocturna(uint8_t horaInicio, uint8_t horaFin);
uint8_t reloj_inicioNoche();
uint8_t reloj_finNoche();

// True si la hora actual cae dentro de la franja nocturna.
// Devuelve SIEMPRE false si el reloj no esta en hora (ver reloj_enHora).
// Contempla franjas que cruzan la medianoche, p. ej. 22:00 -> 05:00.
bool reloj_esHorarioNocturno();

// Texto "HH:MM" para pantalla. Devuelve "--:--" si el reloj no esta en hora.
const char* reloj_textoHora();
