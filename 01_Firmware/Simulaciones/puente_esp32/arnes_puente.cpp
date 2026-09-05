// ===== Simulaciones/puente_esp32/arnes_puente.cpp =====
//
// LA PUNTA STM32 DEL SIMULADOR DEL PUENTE — CODIGO REAL, NO UNA COPIA.
//
// Este fichero NO reimplementa procesarComando() ni el bucle receptor. Compila el
// `bluetooth.cpp` REAL de una de las dos puntas -la que diga -DPUNTA_MAESTRO o
// -DPUNTA_ESCLAVO- y lo expone como un proceso que habla por su entrada y su salida
// estandar. El simulador de Python es quien mueve los bytes.
//
// POR QUE UN PROCESO Y NO UNA COPIA EN PYTHON.
//
// Hoy hay CUATRO modelos del STM32 escritos a mano en este repositorio, y ninguno
// releido del C++. Un quinto habria probado que el quinto Python se comporta. Aqui
// el que decide si un comando casa, si una trama se trunca, que checksum lleva la
// respuesta y que pin se mueve es el mismo .cpp que se carga en la tarjeta.
//
// LO QUE SI ES SUSTITUTO, Y ES EL PUNTO CIEGO DE ESTE ARNES:
//
//   - Arduino.h / pines.h : no hay placa. HardwareSerial es un par de colas de bytes.
//   - reloj.cpp           : NO se compila. Incluye <STM32RTC.h> y <stm32f1xx_hal.h>,
//                           que no existen fuera del framework. Se sustituye por un
//                           reloj de arnes con los MISMOS getters, gobernado desde
//                           fuera con RTC <cristal> <enhora>. Consecuencia honesta:
//                           las tres ramas de SET_RTC se ejercen de verdad -sin
//                           cristal / formato malo / bien-, pero lo que se ejerce es
//                           el DESPACHADOR, no reloj.cpp.
//   - modo_degradado      : NO se compila (597 lineas y arrastra la pantalla). Su
//                           puerta de entrada se sustituye por un motivo gobernable
//                           desde fuera, que es lo unico que bluetooth.cpp le pide.
//   - menu / lcd / botones / protocolo : no-ops. Ninguno decide nada del contrato.
//
// LO QUE SI ES REAL, y por eso el EFECTO se puede medir y no solo la respuesta:
//   bluetooth.cpp, semaforo.cpp, coordinador.cpp, modo_automatico.cpp, mando.cpp,
//   modos.cpp, demanda.cpp e identidad.cpp de la punta que toque. Cuando un comando
//   de la app mueve una luz, la mueve escribirPines() de verdad.
//
// EL PROTOCOLO DE LINEA, hacia el simulador:
//   RX <texto>   mete esos bytes en la cola de SerialBT y llama a bluetooth_loop()
//   RXHEX <hex>  igual, pero en hexadecimal: asi viajan \r y \n sin que la consola
//                los toque -que es justo lo que F1 y F2 necesitan medir-
//   MS <n>       avanza millis() n ms y llama a bluetooth_loop() una vez
//   LOOP         llama a bluetooth_loop() sin mover el reloj
//   PINS         imprime el estado de los pines de luz
//   PUERTO       imprime pines y baudios del HardwareSerial que se encontro
//   RTC <c> <h>  fija si hay cristal y si el reloj esta en hora
//   MDG <n>      fija el motivo que devuelve la puerta del Modo Degradado
//   MODO         imprime el modo vigente
//   QUIT
//
// Todo lo que bluetooth.cpp imprime por SerialBT sale como lineas `TX <hex>`. En
// hexadecimal a proposito: el contrato de bytes incluye los terminadores, y un
// arnes que los perdiera por el camino no podria medir S-1.

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string>

#include "Arduino.h"
#include "uid_arnes.h"
#include "pines.h"
#include "bluetooth.h"
#include "semaforo.h"
#include "identidad.h"
#include "demanda.h"

#include "protocolo.h"

#if defined(PUNTA_MAESTRO)
#include "coordinador.h"
#include "modo_automatico.h"
#include "modos.h"
#include "modo_degradado.h"
#endif

#if defined(PUNTA_ESCLAVO)
// N-106: el bluetooth.cpp del Esclavo pasa a consultar el Modo Degradado, asi que este
// arnes necesita el enum de estados. Es el modo_degradado.h REAL de esta punta -no hay
// copia local, igual que con los otros diez headers-: si alguien anadiera un estado, los
// sustitutos de mas abajo dejarian de cubrir la tabla y habria que venir aqui, que es
// exactamente lo que se quiere que pase.
#include "modo_degradado.h"
#endif

// --- El estado que Arduino.h declara y alguien tiene que definir ------------------
int arnes_pines[64];
int arnes_entradas[64];
unsigned long arnes_escrituras = 0;
unsigned long arnes_millis_valor = 0;
HardwareSerial* arnes_puertos[8];
int arnes_n_puertos = 0;

// El UID de mentira que identidad.cpp REAL lee por -DUID_BASE=arnes_uid. Ver
// uid_arnes.h: se le da memoria legible al fuente de verdad en vez de sustituirlo.
const uint32_t arnes_uid[3] = { 0x00350041UL, 0x35375116UL, 0x30393538UL };

// --- Sustitutos declarados, no disimulados ---------------------------------------
//
// Cada uno de estos existe porque su .cpp real no compila en el PC o porque arrastra
// la mitad del firmware sin decidir nada del contrato de bytes. Van todos juntos y
// con su motivo al lado para que el punto ciego de este arnes se lea de un vistazo,
// en vez de descubrirse revisando el .ps1.

// reloj.cpp: <STM32RTC.h> y <stm32f1xx_hal.h> no existen fuera del framework.
static bool rlj_cristal = true;
static bool rlj_enHora = true;
static uint8_t rlj_h = 18, rlj_m = 25, rlj_s = 0, rlj_dia = 31;
static uint32_t rlj_cnt = 1000;

bool reloj_enHora() { return rlj_enHora; }

// N-144: la contrapartida de reloj_ajustar(). Este arnes compila el bluetooth.cpp REAL de
// las dos puntas, y ese fichero la llama cuando el ajuste NO quedo puesto.
//
// EL DOBLE NO ES VACIO: apaga la bandera de verdad, sobre el mismo rlj_enHora que dobla
// reloj_enHora() aqui arriba. Un stub que no hiciera nada dejaria sin ejercer justo el
// camino que N-144 arreglo -el equipo declarandose EN HORA con el reloj parado en ceros,
// que es de donde cuelga la autorizacion del Modo Degradado-, y este arnes existe para
// EJECUTAR ese fichero, no para enlazarlo.
void reloj_invalidarHora() { rlj_enHora = false; }
bool reloj_hayCristal() { return rlj_cristal; }
uint8_t reloj_hora() { return rlj_h; }
uint8_t reloj_minuto() { return rlj_m; }
uint8_t reloj_segundo() { return rlj_s; }
uint8_t reloj_dia() { return rlj_dia; }
uint32_t reloj_contadorSegundos() { return rlj_cnt; }
uint32_t reloj_segundosDelDia() { return rlj_h * 3600UL + rlj_m * 60UL + rlj_s; }
void reloj_setup() {}
void reloj_actualizar() {}
void reloj_fijarEnero() {}
void reloj_ajustar(uint8_t h, uint8_t m, uint8_t s, uint8_t d) {
  // Se replica la unica negativa que el despachador puede observar: si no hay
  // cristal, reloj_ajustar() abandona en silencio -"if (!rtcOperativo) return;"-.
  // Sin esta rama, el $ACK que mira lo que devolvio la llamada no se podria ejercer.
  if (!rlj_cristal) return;
  rlj_h = h; rlj_m = m; rlj_s = s;
  if (d) rlj_dia = d;
  rlj_enHora = true;
}
bool reloj_reiniciarDominioRespaldo() { return rlj_cristal; }
void reloj_ajustarFranjaNocturna(uint8_t, uint8_t) {}
uint8_t reloj_inicioNoche() { return 22; }
uint8_t reloj_finNoche() { return 5; }
bool reloj_esHorarioNocturno() { return false; }
const char* reloj_textoHora() { return "18:25:00"; }

// D-13 fase 1 (05/09): el campo CAM: del $STATUS, sustituido en las DOS puntas.
//
// botones.cpp NO se compila aqui -este arnes mide el contrato del puente, no el
// vigilante de las camaras-, asi que la funcion que rellena el campo la pone el
// arnes. DEVUELVE EL ESTADO DE ARRANQUE, que es lo que el firmware real publica
// cuando nadie ha ejercido todavia esos pines: camEstado[] nace en CAM_DESCONOCIDA
// y camara_estado() publica la PEOR de las dos. Un "OK" aqui seria modelar un
// arranque que el equipo no tiene, y ademas dejaria sin viajar el unico valor de
// este campo que un lector descuidado colapsa.
//
// Los cuatro valores SI se ejercen, pero en el instrumento que puede: el viaje de
// ida y vuelta con dominio cerrado lo mide simulador_app_bluetooth.py, y que la
// lista sea la del C++ lo mide camara_03_vigilante.
const char* camara_estado() { return "?"; }

#if defined(PUNTA_MAESTRO)
#include "reloj.h"   // struct RelojDiag: solo la declara el reloj.h del Maestro
// N-114 - EL SUSTITUTO DE LA CONSULTA DEL RELOJ, Y POR QUE DEVUELVE UN IMPOSIBLE.
//
// reportarBitsDelReloj() saca por $EVENT los seis bits que RCC->BDCR trae de verdad, y
// esos bits AQUI NO EXISTEN: no hay BDCR, no hay dominio de respaldo y no hay cristal.
// Este arnes mide el CONTRATO DE BYTES -que la trama se forme, quepa y lleve su
// checksum-, no el reloj.
//
// Por eso no se devuelve un RelojDiag de aspecto sano. Un sustituto con lseOn=1, lseRdy=1 y
// rtcSel=1 saldria al cable como un diagnostico plausible, y el dia que alguien
// escribiera una comprobacion sobre esa trama estaria midiendo un dato inventado con
// cara de medida: es la prueba muerta de este repositorio, y aqui ademas con la palabra
// "diagnostico" encima.
//
// Lo que se devuelve es IMPOSIBLE EN SILICIO, por tres sitios a la vez, para que nadie
// lo confunda con una lectura:
//
//   lseOn=0 con lseRdy=1   el oscilador no esta pedido y sin embargo esta listo. El
//                          hardware no puede dar esa pareja: RDY solo sube detras de ON.
//   rtcSel=255             RTCSEL son DOS BITS de BDCR, enmascarados en reloj.cpp: su
//                          valor real esta entre 0 y 3 y nunca puede llegar a 255.
//   rtcEn=0 con cntLeido=0 el propio firmware no lee el contador sin RTCEN, asi que la
//                          trama sale con CNT:-- y no con una cifra que parezca un conteo.
//
// De regalo, el 255 ejerce el PEOR CASO de ancho del campo SEL -tres cifras-, que es
// justo el que la cuenta de reloj_01_consulta_por_bluetooth dimensiona: si algun dia la
// trama se truncara por ahi, este arnes lo ve con la trama mas larga posible delante.
void reloj_diagnostico(RelojDiag* d) {
  if (d == nullptr) return;
  d->lseOn = false;
  d->lseRdy = true;
  d->lseByp = true;
  d->rtcSel = 255;
  d->rtcEn = false;
  d->cntLeido = false;
  d->cnt = 0;
  d->configurado = false;
  d->anio = 0;
}
#endif

// protocolo.cpp: abre un segundo HardwareSerial sobre la radio LoRa. Aqui no hay
// radio, y compilarlo metería un puerto mas en el registro sin aportar nada al
// contrato con el ESP32.
void protocolo_setup() {}

// LA RADIO LoRa, QUE ES LA MITAD (b) DE F5 Y POR ESO NO ES UN NO-OP.
//
// coordinador.cpp es REAL, asi que si nadie le contesta un PONG, SFTY-6 dispara solo
// a los SFTY6_SILENCIO_MS y manda el cruce a ambar. Eso es lo que separa el silencio
// de la RADIO -que si tiene consecuencia vial- del silencio del PUENTE -que no la
// tiene-. Con la radio MUDA el arnes no simula el ambar: lo provoca en el codigo de
// verdad. Se gobierna desde fuera con RADIO 0|1.
static bool radio_viva = true;
static bool rf_hay = false;
static RF_Packet rf_pkt = { 0, 0, 0, 0 };
static unsigned long rf_entrega = 0;

void protocolo_resetReplayProtection() {}

void protocolo_enviarPaquete(uint8_t cmd, uint8_t param) {
  (void)param;
  if (!radio_viva) return;              // orfandad real: nadie contesta nada
  RF_Packet resp = { 0, 0, 0, 0 };
  switch (cmd) {
    case CMD_GO_GREEN: resp.command = CMD_ACK_GREEN; break;
    case CMD_GO_RED:   resp.command = CMD_ACK_RED;   break;
    case CMD_PING:     resp.command = CMD_PONG;      break;
    default: return;                    // hora, delta, config: fuera de este arnes
  }
  rf_hay = true;
  rf_pkt = resp;
  rf_entrega = arnes_millis_valor;
}

bool protocolo_hayPaqueteDisponible(RF_Packet* destino) {
  if (!rf_hay || arnes_millis_valor < rf_entrega) return false;
  *destino = rf_pkt;
  rf_hay = false;
  return true;
}


#if defined(PUNTA_MAESTRO)
// menu.cpp arrastra la pantalla entera; bluetooth.cpp solo llama a su setup.
void menu_setup() {}

// modo_degradado.cpp son 597 lineas con la pantalla dentro. De toda esa maquina,
// bluetooth.cpp solo consulta la PUERTA -evaluarEntrada- y los dos textos del motivo.
// El motivo se gobierna desde fuera para poder ejercer la rama de rechazo, que es la
// que compone un $ERR con snprintf en un buffer de 80.
static int mdg_motivo = 0;   // 0 = MDG_OK
MotivoDegradado modo_degradado_evaluarEntrada() { return (MotivoDegradado)mdg_motivo; }
const char* modo_degradado_motivoL1(MotivoDegradado) { return "FALTA_HORA"; }
const char* modo_degradado_motivoL2(MotivoDegradado) { return "PONGA_LA_HORA"; }
bool modo_degradado_pedirSalida() { return true; }

// modo_ambar.cpp y modo_degradado.cpp: los llama mando.cpp al reconocer su secuencia.
void modo_ambar_setup() {}
void modo_ambar_fijarMotivo(const char*, const char*) {}
void modo_degradado_setup() {}

// respaldo.cpp escribe en los registros de respaldo del RTC. Sin RTC no hay donde.
void respaldo_marcarSync(uint32_t) {}

// N-133/N-135: los tiempos del ciclo, doblados CON MEMORIA y no con un stub vacio.
// Uno que devolviera siempre "no hay nada guardado" dejaria sin ejercer el camino de
// recuperacion de modo_automatico.cpp, que es uno de los .cpp que este arnes compila.
static uint8_t _bkR = 0, _bkV = 0, _bkD = 0;
void respaldo_guardarTiemposCiclo(uint8_t r, uint8_t v, uint8_t d) {
  if (r == 0 || v == 0 || d == 0) return;   // misma negativa que el real
  _bkR = r; _bkV = v; _bkD = d;
}
bool respaldo_tiemposCiclo(uint8_t* r, uint8_t* v, uint8_t* d) {
  if (_bkR == 0 || _bkV == 0 || _bkD == 0) return false;
  *r = _bkR; *v = _bkV; *d = _bkD;
  return true;
}

// lcd.cpp y botones.cpp: los pide modo_automatico.cpp, que si es real.
void lcd_dibujarAutomatico(const char*, int, int) {}
void lcd_dibujarConfigValor(const char*, int, const char*) {}
void botones_setup() {}
void botones_actualizar() {}
bool botonArriba() { return false; }
bool botonAbajo() { return false; }
bool botonAceptar() { return false; }
bool botonCancelar() { return false; }
#endif

#if defined(PUNTA_ESCLAVO)
// mando.cpp del Esclavo arrastra su menu.h y su modo_degradado.h, y de todo eso
// bluetooth.cpp solo consulta el VETO: mientras un operario pidio ambar desde el
// gabinete, una orden de radio no saca a esta punta del ambar (SFTY-21). El veto se
// gobierna desde fuera para poder ejercerlo; el latch de ambar por Bluetooth, que es
// lo que este arnes mide de verdad, vive dentro del bluetooth.cpp REAL.
static bool mando_ambar = false;
bool mando_ambarLocal() { return mando_ambar; }

// N-106 - EL MODO DEGRADADO DEL ESCLAVO, SUSTITUIDO Y GOBERNABLE DESDE FUERA.
//
// Son las mismas 400 lineas con la pantalla y el RTC dentro que ya estaban fuera de este
// arnes. Lo que bluetooth.cpp consulta de todo eso son cuatro funciones, y las cuatro se
// sustituyen aqui SOBRE UN SOLO ESTADO -deg_estado-, no como cuatro no-op independientes:
// devolver constantes sueltas dejaria la tabla de S4.5.2 sin poder ejercerse y el arnes
// aprobaria cualquier bluetooth.cpp -es la prueba muerta de N-51-.
//
// gobiernaLuz() se DERIVA del estado con la misma regla que el fichero real
// (modo_degradado.cpp:367): ENTRANDO || ACTIVO || SALIENDO. Copiar el criterio y no
// derivarlo seria una segunda copia que alguien tendria que sincronizar.
//
// salir() imita la guarda real: desde ENTRANDO o ACTIVO pasa a SALIENDO; desde RENDIDO
// baja el cartel a INACTIVO; desde INACTIVO y SALIENDO no hace nada.
//
// Se gobiernan con DEG <estado> y DEG_REND 0|1.
static int deg_estado = (int)DEG_INACTIVO;
static bool deg_rendicion = false;

EstadoDegradado degradado_estado() { return (EstadoDegradado)deg_estado; }
bool degradado_rendicionEnCurso() { return deg_rendicion; }

bool degradado_gobiernaLuz() {
  return deg_estado == (int)DEG_ENTRANDO || deg_estado == (int)DEG_ACTIVO ||
         deg_estado == (int)DEG_SALIENDO;
}

void degradado_salir() {
  if (deg_estado == (int)DEG_RENDIDO) { deg_estado = (int)DEG_INACTIVO; return; }
  if (deg_estado != (int)DEG_ENTRANDO && deg_estado != (int)DEG_ACTIVO) return;
  deg_estado = (int)DEG_SALIENDO;
  deg_rendicion = false;
}

// N-108 - LOS CONTADORES DE LINEA DE SFTY-15, que protocolo.cpp lleva y este arnes
// sustituye. Suben con cada paquete que el arnes entrega, para que el $EVENT de enlace y
// el tramo del $ALARM tengan algo real que publicar en vez de tres ceros fijos.
static unsigned long cnt_bytes = 0, cnt_validas = 0, cnt_ruido = 0;
unsigned long protocolo_bytesRecibidos()    { return cnt_bytes; }
unsigned long protocolo_tramasValidas()     { return cnt_validas; }
unsigned long protocolo_tramasDescartadas() { return cnt_ruido; }
#endif

// --- El puerto del ESP32, buscado POR SUS PINES ----------------------------------
static HardwareSerial* puerto_esp32() {
  for (int i = 0; i < arnes_n_puertos; i++) {
    if (arnes_puertos[i]->arnes_pinRx() == PB7 &&
        arnes_puertos[i]->arnes_pinTx() == PB6) {
      return arnes_puertos[i];
    }
  }
  return 0;
}

static void volcar_tx() {
  HardwareSerial* p = puerto_esp32();
  if (!p) return;
  std::string s = p->arnes_sacar();
  if (s.empty()) return;
  // En hexadecimal: los terminadores \r\n son PARTE del contrato (S-1) y una
  // consola que los interpretara los perderia.
  printf("TX ");
  for (size_t i = 0; i < s.size(); i++) printf("%02X", (unsigned char)s[i]);
  printf("\n");
}

static int hex1(char c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  return -1;
}

int main(void) {
  for (int i = 0; i < 64; i++) { arnes_pines[i] = -1; arnes_entradas[i] = HIGH; }

  bluetooth_setup();
  semaforo_setup();
#if defined(PUNTA_MAESTRO)
  coordinador_setup();
#endif

  HardwareSerial* p = puerto_esp32();
  if (!p) {
    // Es la regla del instrumento: si el arnes no encuentra el puerto, no puede
    // medir nada y tiene que DECIRLO, no seguir dando resultados sobre otra cosa.
    printf("ABORT no se encontro ningun HardwareSerial en PB7/PB6\n");
    return 2;
  }
  printf("LISTO %s rx=%d tx=%d baud=%lu puertos=%d\n",
#if defined(PUNTA_MAESTRO)
         "MAESTRO",
#else
         "ESCLAVO",
#endif
         p->arnes_pinRx(), p->arnes_pinTx(), p->arnes_baudios(), arnes_n_puertos);
  fflush(stdout);

  char linea[4096];
  while (fgets(linea, sizeof(linea), stdin)) {
    size_t n = strlen(linea);
    while (n > 0 && (linea[n - 1] == '\n' || linea[n - 1] == '\r')) linea[--n] = 0;

    if (strncmp(linea, "RXHEX ", 6) == 0) {
      const char* h = linea + 6;
      std::string datos;
      for (size_t i = 0; h[i] && h[i + 1]; i += 2) {
        int a = hex1(h[i]), b = hex1(h[i + 1]);
        if (a < 0 || b < 0) break;
        datos.push_back((char)(a * 16 + b));
      }
      p->arnes_meter(datos.data(), datos.size());
      bluetooth_loop();
      volcar_tx();
      printf("OK %d\n", (int)datos.size());

    } else if (strncmp(linea, "RX ", 3) == 0) {
      p->arnes_meter(linea + 3, strlen(linea + 3));
      bluetooth_loop();
      volcar_tx();
      printf("OK %d\n", (int)strlen(linea + 3));

    } else if (strncmp(linea, "MS ", 3) == 0) {
      // EL ORDEN ES EL DE main.cpp, no uno comodo: bluetooth_loop() primero,
      // semaforo_actualizar() SIEMPRE -main.cpp lo llama en todos los modos y sin
      // excepcion, y esa nota lleva su porque escrito al lado- y el coordinador
      // detras. Si el arnes moviera el equipo en otro orden, el EFECTO que mide no
      // seria el que ocurre en la tarjeta.
      //
      // Y avanza de MILISEGUNDO EN MILISEGUNDO, no de un salto. Un salto de 30 s
      // dejaria una sola vuelta de bucle: el $STATUS periodico saldria UNA vez en vez
      // de treinta y SFTY-6 no llegaria a evaluarse nunca. La primera version hacia
      // justo eso y F5 media 1 donde tocaban 30.
      unsigned long paso = (unsigned long)strtoul(linea + 3, 0, 10);
      for (unsigned long i = 0; i < paso; i++) {
        arnes_millis_valor++;
        bluetooth_loop();
        semaforo_actualizar();
#if defined(PUNTA_MAESTRO)
        coordinador_actualizar();
#endif
      }
      volcar_tx();
      printf("OK %lu\n", arnes_millis_valor);

    } else if (strcmp(linea, "LOOP") == 0) {
      bluetooth_loop();
      volcar_tx();
      printf("OK\n");

    } else if (strcmp(linea, "PINS") == 0) {
      printf("PINS R1=%d A1=%d V1=%d R2=%d A2=%d V2=%d RP=%d VP=%d DERE=%d esc=%lu\n",
             arnes_pines[ROJO1], arnes_pines[AMARILLO1], arnes_pines[VERDE1],
             arnes_pines[ROJO2], arnes_pines[AMARILLO2], arnes_pines[VERDE2],
             arnes_pines[ROJO_PEATON], arnes_pines[VERDE_PEATON],
             arnes_pines[RS485_IN_DE_RE], arnes_escrituras);
      printf("OK\n");

    } else if (strcmp(linea, "PUERTO") == 0) {
      printf("PUERTO rx=%d tx=%d baud=%lu n=%d\n",
             p->arnes_pinRx(), p->arnes_pinTx(), p->arnes_baudios(),
             arnes_n_puertos);
      printf("OK\n");

    } else if (strncmp(linea, "RTC ", 4) == 0) {
      int c = 1, h = 1;
      sscanf(linea + 4, "%d %d", &c, &h);
      rlj_cristal = (c != 0);
      rlj_enHora = (h != 0);
      printf("OK cristal=%d enhora=%d\n", (int)rlj_cristal, (int)rlj_enHora);

    } else if (strncmp(linea, "RADIO ", 6) == 0) {
#if defined(PUNTA_MAESTRO)
      radio_viva = (atoi(linea + 6) != 0);
      printf("OK radio=%d\n", (int)radio_viva);
#else
      printf("OK radio=n/a\n");
#endif

    } else if (strncmp(linea, "MDG ", 4) == 0) {
#if defined(PUNTA_MAESTRO)
      mdg_motivo = atoi(linea + 4);
      printf("OK mdg=%d\n", mdg_motivo);
#else
      printf("OK mdg=n/a\n");
#endif

    } else if (strncmp(linea, "DEG_REND ", 9) == 0) {
#if defined(PUNTA_ESCLAVO)
      deg_rendicion = (atoi(linea + 9) != 0);
      printf("OK deg_rend=%d\n", (int)deg_rendicion);
#else
      printf("OK deg_rend=n/a\n");
#endif

    } else if (strncmp(linea, "DEG ", 4) == 0) {
#if defined(PUNTA_ESCLAVO)
      // El numero es el enum de modo_degradado.h: 0 INACTIVO, 1 ENTRANDO, 2 ACTIVO,
      // 3 SALIENDO, 4 RENDIDO. Se imprime tambien gobiernaLuz() porque es lo que decide
      // la rama, y verlo evita depurar contra un estado que no es el que se creia.
      deg_estado = atoi(linea + 4);
      printf("OK deg=%d gobierna=%d\n", deg_estado, (int)degradado_gobiernaLuz());
#else
      printf("OK deg=n/a\n");
#endif

    } else if (strncmp(linea, "RXCNT ", 6) == 0) {
#if defined(PUNTA_ESCLAVO)
      // Los tres contadores de SFTY-15 de una vez: bytes, validas y ruido.
      cnt_bytes = (unsigned long)atol(linea + 6);
      cnt_validas = cnt_bytes / 6;
      cnt_ruido = cnt_bytes % 7;
      printf("OK rx=%lu ok=%lu ruido=%lu\n", cnt_bytes, cnt_validas, cnt_ruido);
#else
      printf("OK rxcnt=n/a\n");
#endif

    } else if (strcmp(linea, "MODO") == 0) {
#if defined(PUNTA_MAESTRO)
      // obtenerNombreModo() es static dentro de bluetooth.cpp y no se puede llamar
      // desde aqui. Se imprime el ENUM, que es el dato duro; el NOMBRE viaja de todas
      // formas dentro del $STATUS que el propio fuente compone, y ahi si se lee.
      printf("MODO %d\n", (int)modoActual_get());
#else
      printf("MODO 0 SUBORDINADO\n");
#endif
      printf("OK\n");

    } else if (strcmp(linea, "ESTADO") == 0) {
      printf("ESTADO %s\n", semaforo_nombreEstado());
      printf("OK\n");

    } else if (strcmp(linea, "QUIT") == 0) {
      break;

    } else if (n == 0) {
      continue;

    } else {
      printf("ERR orden desconocida: %s\n", linea);
    }
    fflush(stdout);
  }
  return 0;
}
