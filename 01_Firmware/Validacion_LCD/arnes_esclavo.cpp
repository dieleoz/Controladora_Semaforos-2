// ===========================================================================
// ARNES DE VALIDACION DE PANTALLA — LADO ESCLAVO
// ===========================================================================
// Hermano de arnes_lcd.cpp, que cubre el Maestro. Son DOS EJECUTABLES y no uno
// porque los dos proyectos definen los mismos simbolos -lcd_setup(),
// menu_setup(), lcd_dibujarMenu()...- y enlazarlos juntos es imposible. Los dos
// binarios los construye y los ejecuta compilar.ps1, que suma sus resultados.
//
// POR QUE ESTO EXISTE Y POR QUE ES PERMANENTE
// -------------------------------------------
// Las cinco vistas del Esclavo se validaron una vez con un arnes temporal que se
// desecho. Una validacion de usar y tirar comprueba EL MOMENTO, NO EL PROYECTO:
// al dia siguiente cualquiera puede romper una pantalla del Esclavo y nada lo
// dice, mientras el arnes del Maestro sigue dando 83/83 y todo parece verde.
//
// No es teorico. Este mismo arnes, en el lado Maestro, ya cazo tres desbordes
// silenciosos, uno de ellos un texto de 22 caracteres que no cabia. U8G2 RECORTA
// SIN DAR ERROR: un texto mal colocado no falla, simplemente no aparece.
//
// QUE SE COMPILA DE VERDAD
// ------------------------
// Se enlazan TRES ficheros del firmware, sin tocarlos:
//
//   Esclavo/src/lcd.cpp             las cinco vistas
//   Esclavo/src/menu.cpp            la navegacion real y la COMPOSICION de los
//                                   textos que se pintan (es ahi donde se arman
//                                   "NO SE PUEDE. FALTA:" y "VERDE AQUI 42s")
//   Esclavo/src/modo_degradado.cpp  la maquina de estados que produce esos
//                                   textos, con sus rotulos reales
//
// Enlazar los tres en vez de escribir aqui las cadenas a mano es deliberado: una
// copia de los textos en el arnes solo demuestra que el arnes cabe en la
// pantalla. Asi, el dia que alguien alargue "RENDIDO 48h: AMBAR", lo mide esto.
//
// Lo demas -reloj, semaforo, radio, respaldo- se sustituye por muniones
// controlables desde aqui abajo: nada de eso dibuja.
// ===========================================================================

#include <stdio.h>
#include <string.h>

#include "U8g2lib.h"   // sin ARDUINO definido -> solo la clase base U8G2
#include "lcd.h"
#include "menu.h"
#include "modo_degradado.h"
#include "config_ciclo.h"
#include "respaldo.h"
#include "reloj.h"
#include "semaforo.h"
#include "protocolo.h"
#include "botones.h"

// --- Display de validacion: mismo controlador, sin hardware detras ---------
extern "C" uint8_t u8x8_byte_nulo(u8x8_t *, uint8_t, uint8_t, void *) { return 1; }
extern "C" uint8_t u8x8_gpio_nulo(u8x8_t *, uint8_t, uint8_t, void *) { return 1; }

class U8G2_Validacion : public U8G2 {
public:
  U8G2_Validacion() {
    u8g2_Setup_st7920_s_128x64_f(&u8g2, U8G2_R0, u8x8_byte_nulo, u8x8_gpio_nulo);
  }
};

static U8G2_Validacion display;
U8G2 &u8g2 = display;   // lo que lcd.cpp espera con LCD_VALIDACION_NATIVA

// ---------------------------------------------------------------------------
// RELOJ DEL ARNES
//
// El Arduino.h sustituto define millis() como constante 0 para el arnes del
// Maestro, que solo dibuja. Aqui NO vale: menu.cpp mide con millis() el
// repintado por segundo y, sobre todo, el REGRESO AUTOMATICO AL LISTADO a los
// 90 s, y modo_degradado.cpp mide con el el limite duro de 48 h. Con un reloj
// congelado esas dos cosas no se podrian comprobar nunca.
//
// Por eso este arnes se compila con -DARNES_MILLIS_CONTROLADO y el tiempo
// avanza cuando este fichero lo dice. El arnes del Maestro no lleva ese define y
// sigue viendo exactamente el mismo millis() de siempre.
// ---------------------------------------------------------------------------
unsigned long arnes_millis_valor = 1000;

// --- Muniones del resto del firmware ---------------------------------------
// Solo estado y lecturas; ninguna dibuja nada.

static bool     m_enHora   = true;
static uint8_t  m_hora = 14, m_minuto = 32, m_segundo = 7;
static uint32_t m_segDia   = 14UL * 3600UL + 32UL * 60UL + 7UL;
static uint8_t  m_dia      = 10;

bool     reloj_enHora()         { return m_enHora; }
uint8_t  reloj_hora()           { return m_hora; }
uint8_t  reloj_minuto()         { return m_minuto; }
uint8_t  reloj_segundo()        { return m_segundo; }
uint8_t  reloj_dia()            { return m_enHora ? m_dia : 0; }
uint32_t reloj_segundosDelDia() { return m_enHora ? m_segDia : 0; }
// N-49: el fechado ya no usa dia+segundo sino el contador crudo del RTC.
uint32_t reloj_contadorSegundos() { return m_enHora ? (m_dia * 86400UL + m_segDia) : 0; }
void     reloj_setup()          {}
void     reloj_ajustar(uint8_t, uint8_t, uint8_t) {}

// Fija la hora del arnes por segundos del dia, para colocarse en la fase que
// interese del ciclo degradado.
static void ponerHora(uint32_t segDia) {
  m_segDia  = segDia % 86400UL;
  m_hora    = (uint8_t)(m_segDia / 3600UL);
  m_minuto  = (uint8_t)((m_segDia % 3600UL) / 60UL);
  m_segundo = (uint8_t)(m_segDia % 60UL);
}

static uint8_t m_verde = 30, m_despeje = 30;
static bool    m_cfgRecibida = true;

uint8_t config_verdeSegundos()   { return m_verde; }
uint8_t config_despejeSegundos() { return m_despeje; }
bool    config_verdeRecibido()   { return m_cfgRecibida; }
bool    config_despejeRecibido() { return m_cfgRecibida; }

static const char *m_nombreLuz = "ROJO";
const char* semaforo_nombreEstado()      { return m_nombreLuz; }
void        semaforo_forzarRojo()             { m_nombreLuz = "ROJO"; }
void        semaforo_iniciarTransicionAVerde(){ m_nombreLuz = "AMARILLO"; }
void        semaforo_iniciarFallo()           { m_nombreLuz = "FALLO COM"; }

static unsigned long m_bytes = 36, m_validas = 9;
unsigned long protocolo_bytesRecibidos()   { return m_bytes; }
unsigned long protocolo_tramasValidas()    { return m_validas; }
void          protocolo_resetReplayProtection() {}

// El respaldo real habla con los registros BKP del STM32; aqui basta con que
// diga que no hay nada guardado, que es el caso en el que manda la radio.
void     respaldo_guardarDegradado(bool)                { }
bool     respaldo_degradadoActivo()                     { return false; }
bool     respaldo_hayCiclo()                            { return false; }
uint8_t  respaldo_verdeSeg()                            { return 0; }
uint8_t  respaldo_despejeSeg()                          { return 0; }
bool     respaldo_haySync()                             { return false; }
uint32_t respaldo_horasDesdeSync(uint32_t)             { return RESPALDO_SYNC_CADUCADA; }
void     respaldo_marcarSync(uint8_t, uint32_t)         { }
void     respaldo_guardarCiclo(uint8_t, uint8_t)        { }
void     respaldo_setup()                               { }
bool     respaldo_valido()                              { return false; }
void     respaldo_borrar()                              { }

// --- Botonera simulada: se "pulsa" fijando la variable antes de menu_loop() -
static bool p_arriba = false, p_abajo = false, p_ok = false, p_cancelar = false;
static bool consumir(bool &b) { bool v = b; b = false; return v; }
void botones_setup()      {}
void botones_actualizar() {}
bool botonArriba()        { return consumir(p_arriba); }
bool botonAbajo()         { return consumir(p_abajo); }
bool botonAceptar()       { return consumir(p_ok); }
bool botonCancelar()      { return consumir(p_cancelar); }

// ---------------------------------------------------------------------------
// Utilidades de inspeccion del framebuffer (mismas que el arnes del Maestro)
// ---------------------------------------------------------------------------
static const int ANCHO = 128;
static const int ALTO = 64;

// Empaquetado del framebuffer de la ST7920: HORIZONTAL, con el bit mas
// significativo a la izquierda. Cada fila ocupa 128/8 = 16 bytes.
// Se vuelve a calibrar en main() antes de creer ninguna medida.
static bool pixel(int x, int y) {
  if (x < 0 || x >= ANCHO || y < 0 || y >= ALTO) return false;
  const uint8_t *buf = display.getBufferPtr();
  return (buf[y * (ANCHO / 8) + (x / 8)] >> (7 - (x % 8))) & 1;
}

static int tintaEnFila(int y) {
  int n = 0;
  for (int x = 0; x < ANCHO; x++) if (pixel(x, y)) n++;
  return n;
}

static int fallos = 0;
static int comprobaciones = 0;

static void comprobar(bool ok, const char *desc) {
  comprobaciones++;
  if (ok) {
    printf("      [OK]    %s\n", desc);
  } else {
    printf("      [FALLA] %s\n", desc);
    fallos++;
  }
}

// Dibuja la pantalla en modo texto, a media escala vertical para que quepa.
static void volcar(const char *titulo) {
  printf("\n  +------------------------------ %s\n", titulo);
  for (int y = 0; y < ALTO; y += 2) {
    printf("  |");
    for (int x = 0; x < ANCHO; x++) {
      bool a = pixel(x, y), b = pixel(x, y + 1);
      putchar((a && b) ? '#' : (a || b) ? ':' : ' ');
    }
    printf("|\n");
  }
  printf("  +");
  for (int i = 0; i < ANCHO; i++) putchar('-');
  printf("+\n");
}

// El contenido no puede pasar de y=63: por debajo, U8g2 lo recorta callando.
static void comprobarMargenes(const char *pantalla) {
  int ultima = -1;
  for (int y = ALTO - 1; y >= 0; y--) {
    if (tintaEnFila(y) > 0) { ultima = y; break; }
  }
  char desc[190];
  snprintf(desc, sizeof(desc),
           "%s: el contenido termina en y=%d (limite 63)", pantalla, ultima);
  comprobar(ultima >= 0 && ultima <= 63, desc);
}

// Cuenta bandas horizontales de texto separadas por filas vacias.
static int contarBandas(int desde, int hasta) {
  int bandas = 0;
  bool dentro = false;
  for (int y = desde; y <= hasta; y++) {
    bool hay = tintaEnFila(y) > 0;
    if (hay && !dentro) { bandas++; dentro = true; }
    else if (!hay) dentro = false;
  }
  return bandas;
}

// Ultima columna con tinta dentro de una banda de filas.
//
// Es la sonda que caza el recorte SILENCIOSO: un texto que no cabe no da error,
// se corta. Si la tinta llega hasta la ultima columna de la pantalla, lo mas
// probable es que el texto siga -invisible- mas alla del borde.
static int ultimaColumna(int y0, int y1) {
  int xf = -1;
  for (int y = y0; y <= y1; y++)
    for (int x = ANCHO - 1; x > xf; x--)
      if (pixel(x, y)) { xf = x; break; }
  return xf;
}

static int primeraColumna(int y0, int y1) {
  for (int x = 0; x < ANCHO; x++)
    for (int y = y0; y <= y1; y++)
      if (pixel(x, y)) return x;
  return -1;
}

// Hay tinta en la columna del cursor ('>' se dibuja en x=2..7) dentro de la banda.
static bool cursorEnBanda(int y0, int y1) {
  for (int y = y0; y <= y1; y++)
    for (int x = 2; x <= 8; x++)
      if (pixel(x, y)) return true;
  return false;
}

static int pixelesEnCaja(int x0, int x1, int y0, int y1) {
  int n = 0;
  for (int y = y0; y <= y1; y++)
    for (int x = x0; x <= x1; x++)
      if (pixel(x, y)) n++;
  return n;
}

// Perfil por filas: cuanta tinta hay y entre que columnas, fila a fila.
static void perfilFilas(const char *titulo, int desde, int hasta) {
  printf("\n  --- perfil de filas: %s ---\n", titulo);
  for (int y = desde; y <= hasta; y++) {
    int n = tintaEnFila(y);
    int xi = -1, xf = -1;
    for (int x = 0; x < ANCHO; x++) if (pixel(x, y)) { if (xi < 0) xi = x; xf = x; }
    printf("   y=%2d  tinta=%3d", y, n);
    if (n) printf("  x=%d..%d", xi, xf);
    printf("\n");
  }
}

// "4=Menu" vive en x=96..125 de la fila inferior en ESTADO. Nada puede invadir
// el hueco de separacion: salir tiene que poder hacerse siempre, incluso con la
// pantalla dando el peor de los diagnosticos.
static void comprobarNoInvade4Menu(const char *caso) {
  int invasion = pixelesEnCaja(88, 95, 55, 63);
  char d[190];
  snprintf(d, sizeof(d),
           "%s: la fila de enlace no invade '4=Menu' (%d px en x=88..95)", caso, invasion);
  comprobar(invasion == 0, d);
}

// ---------------------------------------------------------------------------
// EL RECORTE DE U8G2 NO DEJA HUELLA, Y POR ESO EL CRITERIO NO PUEDE SER
// "NO TOCA EL BORDE".
//
// La 6x10 dibujada desde x=2 pone cada caracter en x = 2+6k .. 6+6k. La ultima
// celda que cabe entera es la 21a, en x=122..126; la 22a empezaria en x=128 y
// U8G2 NO DIBUJA NI UN PIXEL DE ELLA. Es decir: un texto de 21 caracteres y uno
// de 40 producen EXACTAMENTE la misma imagen. Mirar si la tinta llega a x=127 no
// sirve de nada, porque con esta fuente y este origen nunca llega.
//
// El unico indicio utilizable es que el texto este usando la ultima celda
// disponible: a partir de ahi no se puede afirmar que termine ahi. Por eso las
// lineas que se COMPONEN en tiempo de ejecucion -el rotulo del modo, el detalle
// y el motivo del rechazo- se exigen con al menos una celda libre.
// ---------------------------------------------------------------------------
static const int PRIMERA_COLUMNA_ULTIMA_CELDA_6x10 = 122;

static void comprobarCabeConHolgura(const char *caso, int y0, int y1) {
  int xf = ultimaColumna(y0, y1);
  char d[220];
  snprintf(d, sizeof(d),
           "%s: el texto no llega a la ultima celda que cabe, asi que se sabe que termina donde "
           "parece (acaba en x=%d, la ultima celda empieza en x=%d)",
           caso, xf, PRIMERA_COLUMNA_ULTIMA_CELDA_6x10);
  comprobar(xf >= 0 && xf < PRIMERA_COLUMNA_ULTIMA_CELDA_6x10, d);
}

// Nada de la pantalla puede tocar la ultima columna. Se excluye el ancho
// completo por defecto (lineas separadoras y recuadros SI llegan a x=127 a
// proposito) indicando el rango de filas a mirar.
static void comprobarBordeDerecho(const char *caso, int y0, int y1) {
  int xf = ultimaColumna(y0, y1);
  char d[200];
  snprintf(d, sizeof(d),
           "%s: nada se recorta por el borde derecho (ultima columna con tinta = %d)", caso, xf);
  comprobar(xf >= 0 && xf <= 126, d);
}

// ---------------------------------------------------------------------------
// Avance de tiempo controlado.
//
// Se avanza a pasos y se llama en cada uno a degradado_actualizar(), que es
// exactamente lo que hace main.cpp en cada vuelta del bucle. Un salto seco no
// valdria: la maquina del Modo Degradado cambia de estado POR TRANSICIONES, y
// nadie las veria si el tiempo diera el salto entero de una vez.
// ---------------------------------------------------------------------------
static void avanzarModo(unsigned long ms, unsigned long paso) {
  unsigned long restante = ms;
  while (restante > 0) {
    unsigned long p = (restante < paso) ? restante : paso;
    arnes_millis_valor += p;
    restante -= p;
    degradado_actualizar();
  }
}

// Igual, pero atendiendo tambien al menu: es el bucle real del equipo.
static void avanzarConMenu(unsigned long ms, unsigned long paso) {
  unsigned long restante = ms;
  while (restante > 0) {
    unsigned long p = (restante < paso) ? restante : paso;
    arnes_millis_valor += p;
    restante -= p;
    degradado_actualizar();
    menu_loop();
  }
}

// Una pulsacion, tal y como la ve el equipo: el flanco se pone, el bucle corre.
static void pulsar(bool &boton) {
  boton = true;
  arnes_millis_valor += 50;
  degradado_actualizar();
  menu_loop();
}

// Abre la pantalla del Modo Degradado desde el listado.
//
// Hay que volver a navegar despues de CADA salto largo de tiempo, y no es un
// detalle del arnes: a los 90 s sin pulsar nada, menu.cpp devuelve la pantalla
// al listado el solo. La primera version de este arnes daba por hecho que la
// pantalla seguia donde se la dejo y midio el listado creyendo medir el modo;
// el regreso automatico se comprueba aparte, al final.
static void abrirDegradado() {
  menu_setup();
  pulsar(p_abajo);
  pulsar(p_ok);
}

// ---------------------------------------------------------------------------
int main() {
  printf("===========================================================\n");
  printf(" VALIDACION DE PANTALLA LCD ST7920 (128x64) — ESCLAVO\n");
  printf(" Se ejecutan los mismos lcd.cpp, menu.cpp y modo_degradado.cpp\n");
  printf(" que van al firmware del Esclavo.\n");
  printf("===========================================================\n");

  display.begin();

  printf("\n  --- geometria real del buffer ---\n");
  printf("   pantalla      : %d x %d px\n",
         display.getDisplayWidth(), display.getDisplayHeight());
  printf("   buffer (tiles): %d x %d\n",
         display.getBufferTileWidth(), display.getBufferTileHeight());

  // --- Calibracion del acceso al framebuffer -----------------------------
  // Antes de creer nada, se dibuja un patron conocido y se comprueba que se lee
  // donde toca. Si esto falla, cualquier medida posterior seria basura.
  {
    display.clearBuffer();
    display.drawBox(0, 40, 10, 8);   // rectangulo solido: x 0..9, y 40..47
    printf("\n  --- calibracion del framebuffer ---\n");
    printf("   patron: drawBox(0,40,10,8) -> se espera tinta=10 en y=40..47, 0 fuera\n");
    bool bien = true;
    for (int y = 36; y <= 51; y++) {
      int n = tintaEnFila(y);
      int esperado = (y >= 40 && y <= 47) ? 10 : 0;
      if (n != esperado) bien = false;
      printf("   y=%2d  tinta=%3d  (esperado %d)%s\n", y, n, esperado,
             (n == esperado) ? "" : "   <-- DISCREPANCIA");
    }
    comprobar(bien, "Calibracion: el framebuffer se lee correctamente");
    if (!bien) {
      printf("\n   La lectura del framebuffer no es fiable. Se aborta:\n");
      printf("   las comprobaciones siguientes no significarian nada.\n");
      return 1;
    }
  }

  // --- Autocomprobacion de la sonda de desborde ---------------------------
  // Una comprobacion que no puede fallar no comprueba nada. Antes de dar por
  // buena ninguna pantalla se le dan a lcd.cpp dos textos: uno que cabe y otro
  // que NO, y se exige que la sonda los distinga. Si no los distinguiera, todos
  // los [OK] de mas abajo significarian unicamente que el arnes esta ciego.
  {
    lcd_dibujarRechazoDegradado("12345678901234567890");     // 20: cabe
    int xCabe = ultimaColumna(26, 36);
    lcd_dibujarRechazoDegradado("12345678901234567890123");  // 23: NO cabe
    int xNoCabe = ultimaColumna(26, 36);

    char d[240];
    snprintf(d, sizeof(d),
             "Autocomprobacion: la sonda distingue un texto que cabe de uno que no "
             "(20 caracteres acaban en x=%d, 23 caracteres en x=%d; el umbral esta en x=%d)",
             xCabe, xNoCabe, PRIMERA_COLUMNA_ULTIMA_CELDA_6x10);
    bool distingue = (xCabe >= 0 && xCabe < PRIMERA_COLUMNA_ULTIMA_CELDA_6x10) &&
                     (xNoCabe >= PRIMERA_COLUMNA_ULTIMA_CELDA_6x10);
    comprobar(distingue, d);
    if (!distingue) {
      printf("\n   La sonda de desborde no reacciona ante un texto que no cabe.\n");
      printf("   Se aborta: los [OK] siguientes no significarian nada.\n");
      return 1;
    }
  }

  // =========================================================================
  // REFERENCIA: el menu de DOS opciones, que es el que hay en servicio
  // =========================================================================
  // Mismo criterio que en el arnes del Maestro: si el arnes no da por bueno lo
  // que el equipo muestra correctamente en terreno, el equivocado es el arnes.
  {
    const char *ref[2] = {"ESTADO", "MODO DEGRADADO"};
    lcd_dibujarMenu(0, ref, 2, NULL);
    volcar("REFERENCIA — menu del Esclavo, 2 opciones, sin pie");
    int bandas = contarBandas(18, 63);
    char d[170];
    snprintf(d, sizeof(d),
             "REFERENCIA: el menu de 2 opciones se lee con %d bandas (deben ser 2)", bandas);
    comprobar(bandas == 2, d);
    if (bandas != 2) {
      printf("\n   El arnes no reconoce la pantalla que el equipo muestra hoy.\n");
      printf("   Se aborta: las comprobaciones siguientes no serian fiables.\n");
      return 1;
    }
  }

  // =========================================================================
  // 1/5 — BIENVENIDA
  // =========================================================================
  printf("\n\n===========================================================\n");
  printf(" 1/5  BIENVENIDA\n");
  printf("===========================================================\n");
  lcd_dibujarBienvenida();
  volcar("BIENVENIDA");
  perfilFilas("BIENVENIDA", 0, 63);
  comprobarMargenes("Bienvenida");
  comprobarBordeDerecho("Bienvenida", 0, 63);
  {
    int bandas = contarBandas(0, 63);
    char d[170];
    snprintf(d, sizeof(d),
             "Bienvenida: las 3 lineas del rotulo se leen separadas (%d bandas)", bandas);
    comprobar(bandas == 3, d);
  }

  // =========================================================================
  // 2/5 — MENU
  // =========================================================================
  printf("\n\n===========================================================\n");
  printf(" 2/5  MENU — cursor, pie de aviso y salvaguarda de interlineado\n");
  printf("===========================================================\n");
  {
    const char *op[2] = {"ESTADO", "MODO DEGRADADO"};

    // Layout de 2 opciones: base 28, paso 11 -> y = 28 y 39.
    // El cursor tiene que verse en LAS DOS posiciones. Un cursor que se pinta
    // donde no hay opcion -o que no se pinta- deja al operario navegando a
    // ciegas sobre la unica pantalla que puede cambiar la operacion.
    for (int c = 0; c < 2; c++) {
      lcd_dibujarMenu(c, op, 2, NULL);
      char t[80];
      snprintf(t, sizeof(t), "MENU — cursor en la opcion %d ('%s')", c, op[c]);
      volcar(t);
      comprobarMargenes(t);
      int y = 28 + c * 11;
      char d[170];
      snprintf(d, sizeof(d), "Menu: el cursor es visible sobre la opcion %d ('%s')", c, op[c]);
      comprobar(cursorEnBanda(y - 9, y), d);
    }

    // El titulo se dibuja en x=20 con la 7x14B: 12 caracteres son 84 px y
    // acaban en x=103. Se mide en vez de suponerse porque un titulo mas largo
    // se recortaria sin avisar.
    lcd_dibujarMenu(0, op, 2, NULL);
    {
      int xi = primeraColumna(0, 14), xf = ultimaColumna(0, 14);
      char d[190];
      snprintf(d, sizeof(d),
               "Menu: el titulo 'MENU ESCLAVO' cabe entero (x=%d..%d, limite 0..127)", xi, xf);
      comprobar(xi >= 0 && xf <= 126, d);
    }

    // Pie de aviso. Que el Degradado gobierna la luz tiene que verse SIN entrar
    // a ninguna pantalla, y es el texto mas largo que se pinta ahi abajo.
    lcd_dibujarMenu(1, op, 2, "MODO DEGRADADO ACTIVO");
    volcar("MENU — con el pie 'MODO DEGRADADO ACTIVO'");
    perfilFilas("MENU con pie", 36, 63);
    comprobarMargenes("Menu con pie");
    comprobarBordeDerecho("Menu con pie", 18, 63);
    {
      // El pie va con linea base en y=62; la ultima opcion, en y=39. Entre las
      // dos tiene que quedar hueco: si se tocaran no se leeria ninguna.
      int libres = 0;
      for (int y = 42; y <= 54; y++) if (tintaEnFila(y) == 0) libres++;
      char d[190];
      snprintf(d, sizeof(d),
               "Menu: el pie de aviso no se pega a la ultima opcion (%d filas libres entre ambos)",
               libres);
      comprobar(libres >= 3, d);
    }
    {
      int bandas = contarBandas(18, 63);
      char d[170];
      snprintf(d, sizeof(d),
               "Menu con pie: las 2 opciones y el aviso se leen separados (%d bandas)", bandas);
      comprobar(bandas == 3, d);
    }

    // --- Salvaguarda de interlineado --------------------------------------
    // Hoy el menu tiene 2 opciones y este caso no puede darse. Se comprueba
    // igual porque el fallo que la salvaguarda evita YA OCURRIO EN EL MAESTRO:
    // una opcion de mas caia fuera de pantalla, no se pintaba, Y EL CURSOR SI
    // PODIA LLEGAR HASTA ELLA. Quien anada la tercera opcion del Esclavo tiene
    // derecho a que esto le avise en vez de descubrirlo en el gabinete.
    const char *cinco[6] = {"UNO", "DOS", "TRES", "CUATRO", "CINCO", "SEIS"};

    lcd_dibujarMenu(4, cinco, 5, NULL);
    volcar("MENU — 5 opciones (interlineado apretado: base 24, paso 9)");
    comprobarMargenes("Menu de 5 opciones");
    {
      int bandas = contarBandas(18, 63);
      char d[170];
      snprintf(d, sizeof(d), "Menu de 5 opciones: se dibujan las 5 (%d bandas)", bandas);
      comprobar(bandas == 5, d);
      comprobar(cursorEnBanda(51, 60),
                "Menu de 5 opciones: el cursor sigue siendo visible en la ultima");
    }

    lcd_dibujarMenu(0, cinco, 6, NULL);
    volcar("MENU — 6 opciones: la 6a NO cabe y la salvaguarda la corta");
    comprobarMargenes("Menu de 6 opciones");
    {
      // base 24 + 5*9 = 69 > 63: la salvaguarda de lcd.cpp corta el bucle.
      int bandas = contarBandas(18, 63);
      char d[200];
      snprintf(d, sizeof(d),
               "Menu de 6 opciones: la salvaguarda impide dibujar la 6a fuera de pantalla "
               "(%d bandas dibujadas de 6 pedidas)", bandas);
      comprobar(bandas == 5, d);
    }
  }

  // =========================================================================
  // 3/5 — ESTADO
  // =========================================================================
  printf("\n\n===========================================================\n");
  printf(" 3/5  ESTADO — hora con segundos, antiguedad de sync y linea RF\n");
  printf("===========================================================\n");
  {
    struct CasoEstado {
      bool enHora; uint8_t h, m, s;
      bool huboSync; unsigned long msSync; bool vencida;
      unsigned long bytes, validas;
      const char *luz;
      const char *que;
    };
    // Los casos extremos no son adorno: cada uno es un texto que en algun sitio
    // de la fila puede empujar al de al lado fuera de la pantalla.
    CasoEstado casos[] = {
      { true, 14, 32,  7, true,   45000UL, false,      36,      9, "ROJO",
        "ESTADO — operacion normal" },
      { true, 23, 59, 59, true,       0UL, false,      36,      9, "VERDE",
        "ESTADO — 23:59:59, sync de hace 0 s" },
      { false, 0,  0,  0, false,      0UL, false,       0,      0, "FALLO COM",
        "ESTADO — sin hora, sin sync y sin enlace (NUNCA / RX 0)" },
      { true,  0,  0,  0, true,   90000UL, false,    4512,      0, "AMARILLO",
        "ESTADO — 00:00:00 y linea con BASURA" },
      { true,  9,  5, 41, true, 3600000UL, false,  999999, 249999, "ROJO",
        "ESTADO — contadores de 6 cifras y sync de 1 h" },
      { true, 18, 44, 20, true,      0UL,  true,  999999999UL, 249999999UL, "FALLO COM",
        "ESTADO — caso extremo: >48h y contadores de 9 cifras" },
      { true, 11, 11, 11, true, 172740000UL, false,     36,     9, "ROJO",
        "ESTADO — 47h59m sin sincronizar (el texto mas largo de antiguedad)" },
    };
    for (unsigned i = 0; i < sizeof(casos) / sizeof(casos[0]); i++) {
      lcd_dibujarEstado(casos[i].enHora, casos[i].h, casos[i].m, casos[i].s,
                        casos[i].huboSync, casos[i].msSync, casos[i].vencida,
                        casos[i].bytes, casos[i].validas, casos[i].luz);
      volcar(casos[i].que);
      comprobarMargenes(casos[i].que);
      comprobarBordeDerecho(casos[i].que, 18, 63);
      comprobarNoInvade4Menu(casos[i].que);
    }

    // Perfil de la pantalla completa, para dejar por escrito donde cae cada fila.
    perfilFilas("ESTADO (ultimo caso)", 16, 63);

    // Las cuatro lineas de datos -reloj, sync, luz y fila de enlace- tienen que
    // quedar separadas. Si dos se solapan no se lee ninguna de las dos.
    lcd_dibujarEstado(true, 14, 32, 7, true, 45000UL, false, 36, 9, "ROJO");
    {
      int bandas = contarBandas(18, 63);
      char d[190];
      snprintf(d, sizeof(d),
               "Estado: las 4 lineas de datos no se solapan entre si (%d bandas)", bandas);
      comprobar(bandas == 4, d);
    }
    {
      // El reloj es EL dato de esta pantalla: los segundos son lo unico que
      // permite comparar dos relojes a ojo. Tiene que caber entero.
      int xi = primeraColumna(20, 31), xf = ultimaColumna(20, 31);
      char d[190];
      snprintf(d, sizeof(d),
               "Estado: el reloj HH:MM:SS cabe entero (x=%d..%d)", xi, xf);
      comprobar(xi >= 0 && xf <= 126, d);
    }
  }

  // =========================================================================
  // 4/5 y 5/5 — DEGRADADO, CONFIRMACION y RECHAZO, POR NAVEGACION REAL
  // =========================================================================
  // A partir de aqui no se llama a lcd.cpp directamente: se pulsan botones y se
  // deja que menu.cpp y modo_degradado.cpp compongan lo que toque. Es la unica
  // forma de comprobar los textos DE VERDAD -"NO SE PUEDE. FALTA:", el motivo
  // del rechazo, "VERDE AQUI 42s"- en lugar de copias hechas a mano aqui.
  printf("\n\n===========================================================\n");
  printf(" NAVEGACION REAL — menu.cpp y modo_degradado.cpp del firmware\n");
  printf("===========================================================\n");

  m_bytes = 36; m_validas = 9;
  menu_setup();
  volcar("MENU al arrancar (cursor en ESTADO, que solo lee)");
  comprobar(!menu_estaAbierto(),
            "Al arrancar, el listado es el estado de reposo: el mando NO esta inhibido");

  // ESTADO por navegacion.
  pulsar(p_ok);
  volcar("ESTADO (entrado con el Boton 3 desde el listado)");
  comprobarMargenes("Estado por navegacion");
  comprobarNoInvade4Menu("Estado por navegacion");
  comprobar(menu_estaAbierto(),
            "Con ESTADO abierto el mando queda inhibido (hay alguien delante del gabinete)");

  pulsar(p_cancelar);
  comprobar(!menu_estaAbierto(), "El Boton 4 devuelve al listado y rearma el mando");

  // --- Los cinco motivos de rechazo, cada uno por su camino ---------------
  // El motivo no es un adorno: el operario esta subido a un poste y lo que le
  // falta se arregla de forma distinta en cada caso. Un motivo recortado no
  // sirve para arreglar nada.
  {
    struct CasoRechazo {
      bool enHora; bool cfg; uint8_t verde, despeje;
      const char *que;
    };
    CasoRechazo casos[] = {
      { false, true,  30, 30, "RECHAZO — reloj sin poner en hora" },
      { true,  false, 30, 30, "RECHAZO — el Maestro nunca mando la duracion del ciclo" },
      { true,  true,   0, 30, "RECHAZO — ciclo en cero" },
      { true,  true,  30, 30, "RECHAZO — nunca hubo sincronizacion por radio" },
    };
    for (unsigned i = 0; i < sizeof(casos) / sizeof(casos[0]); i++) {
      m_enHora = casos[i].enHora;
      m_cfgRecibida = casos[i].cfg;
      m_verde = casos[i].verde;
      m_despeje = casos[i].despeje;

      menu_setup();
      pulsar(p_abajo);           // cursor a MODO DEGRADADO
      pulsar(p_ok);              // entrar a la pantalla del modo
      volcar(casos[i].que);
      comprobarMargenes(casos[i].que);
      comprobarCabeConHolgura(casos[i].que, 18, 54);
      comprobarBordeDerecho(casos[i].que, 56, 63);
      {
        int bandas = contarBandas(18, 63);
        char d[200];
        snprintf(d, sizeof(d),
                 "%s: el rotulo, el motivo, el contador de sync y el pie se leen separados (%d bandas)",
                 casos[i].que, bandas);
        comprobar(bandas == 4, d);
      }

      // Y el cartel de RECHAZO propiamente dicho, que es una pantalla aparte a
      // proposito: si el rechazo se mostrara como un aviso discreto el operario
      // podria marcharse creyendo que el modo quedo activo.
      pulsar(p_ok);              // a la pantalla de confirmacion
      pulsar(p_ok);              // confirmar -> degradado_entrar() rechaza
      char t[190];
      snprintf(t, sizeof(t), "CARTEL %s", casos[i].que);
      volcar(t);
      comprobarMargenes(t);
      comprobarBordeDerecho(t, 20, 63);
      {
        int bandas = contarBandas(20, 63);
        char d[200];
        snprintf(d, sizeof(d),
                 "%s: el motivo y las dos lineas fijas del cartel se leen separados (%d bandas)",
                 t, bandas);
        comprobar(bandas == 3, d);
      }
    }
  }

  // --- Pantalla de confirmacion -------------------------------------------
  m_enHora = true; m_cfgRecibida = true; m_verde = 30; m_despeje = 30;
  ponerHora(14UL * 3600UL);
  degradado_registrarSync();     // ya hay base comun: el modo se puede autorizar

  menu_setup();
  pulsar(p_abajo);
  pulsar(p_ok);
  volcar("MODO DEGRADADO — condiciones cumplidas, 'Pulse 3 para entrar'");
  comprobarMargenes("Degradado aceptable");
  comprobarCabeConHolgura("Degradado aceptable", 18, 54);
  comprobarBordeDerecho("Degradado aceptable", 56, 63);

  pulsar(p_ok);
  volcar("CONFIRMAR ENTRADA? — entrar exige DOS pulsaciones");
  comprobarMargenes("Confirmacion de entrada");
  comprobarCabeConHolgura("Confirmacion de entrada", 18, 54);
  comprobarBordeDerecho("Confirmacion de entrada", 56, 63);
  {
    int bandas = contarBandas(18, 63);
    char d[200];
    snprintf(d, sizeof(d),
             "Confirmacion: la pregunta, el aviso de verificar el Maestro, el contador y el pie "
             "se leen separados (%d bandas)", bandas);
    comprobar(bandas == 4, d);
  }

  // --- Entrada, todo-rojo, activo y verde por reloj ------------------------
  pulsar(p_ok);                  // SI entrar
  comprobar(degradado_estado() == DEG_ENTRANDO,
            "El Boton 3 sobre la confirmacion entra al Modo Degradado (todo-rojo primero)");
  volcar("DEGRADADO — ENTRANDO: TODO ROJO");
  comprobarMargenes("Degradado entrando");
  comprobarCabeConHolgura("Degradado entrando", 18, 54);
  comprobarBordeDerecho("Degradado entrando", 56, 63);

  // Se cumple el despeje de entrada y la fase deja de ser nuestro verde.
  avanzarConMenu(31000UL, 500UL);
  comprobar(degradado_estado() == DEG_ACTIVO,
            "Cumplido el todo-rojo de entrada, el modo pasa a ACTIVO");
  volcar("DEGRADADO — ACTIVO (por reloj)");
  comprobarMargenes("Degradado activo");
  comprobarCabeConHolgura("Degradado activo", 18, 54);
  comprobarBordeDerecho("Degradado activo", 56, 63);

  // El pie cambia a "3=Salir": con el modo en marcha, el Boton 3 ya no entra.
  {
    int bandas = contarBandas(18, 63);
    char d[190];
    snprintf(d, sizeof(d),
             "Degradado activo: las 4 lineas (estado, fase+cuenta, sync y pie) no se solapan (%d bandas)",
             bandas);
    comprobar(bandas == 4, d);
  }

  // Fase de NUESTRO verde: es la linea mas larga que compone menu.cpp, porque
  // junta el rotulo de la fase con la cuenta atras en el mismo renglon.
  ponerHora(14UL * 3600UL + 60UL);     // pos 60 de un ciclo de 120 -> verde del Esclavo
  avanzarConMenu(1500UL, 250UL);
  volcar("DEGRADADO — fase VERDE AQUI con cuenta atras");
  comprobarMargenes("Degradado verde aqui");
  comprobarCabeConHolgura("Degradado verde aqui", 18, 54);
  comprobarBordeDerecho("Degradado verde aqui (pie)", 56, 63);

  // Caso extremo de esa misma linea: el ciclo mas largo que cabe en el byte del
  // protocolo (255 s por fase) da la cuenta atras de mas cifras. Es la linea mas
  // larga que compone menu.cpp, porque junta el rotulo de la fase con el numero.
  m_verde = 255; m_despeje = 255;
  ponerHora(12UL * 3600UL);
  avanzarConMenu(1500UL, 250UL);
  volcar("DEGRADADO — caso extremo: ciclo de 255+255 s (cuenta atras de 3-4 cifras)");
  perfilFilas("DEGRADADO extremo", 16, 63);
  comprobarMargenes("Degradado ciclo maximo");
  comprobarCabeConHolgura("Degradado ciclo maximo", 18, 54);
  comprobarBordeDerecho("Degradado ciclo maximo (pie)", 56, 63);
  m_verde = 30; m_despeje = 30;

  // --- Aviso de proximidad al limite de 48 h -------------------------------
  // El aviso se RECUADRA en lugar de escribirse: la pantalla se lee de
  // madrugada, con lluvia y a un metro. El recuadro va en y=40..51 y ocupa el
  // ancho entero, asi que lo que hay que comprobar es que no tapa el texto que
  // enmarca ni se come el pie.
  avanzarModo(40UL * 3600UL * 1000UL, 60UL * 1000UL);
  comprobar(degradado_avisoLimite(),
            "A las 40 h sin sincronizar se levanta el aviso de proximidad al limite");
  abrirDegradado();
  volcar("DEGRADADO — aviso de limite (contador recuadrado)");
  perfilFilas("DEGRADADO con recuadro", 36, 63);
  comprobarMargenes("Degradado con aviso de limite");
  comprobarCabeConHolgura("Degradado con aviso de limite", 18, 39);
  {
    // El recuadro (drawFrame(0,40,128,12)) ocupa y=40..51 y el ancho entero. Lo
    // que hay que comprobar es que RODEA al contador en vez de cruzarlo: que el
    // texto no toque ninguno de los cuatro trazos. Un recuadro que parte por la
    // mitad la linea que enmarca no destaca el dato, lo tacha.
    int izquierda = pixelesEnCaja(1, 1, 41, 50);
    int derecha   = pixelesEnCaja(126, 126, 41, 50);
    char d[220];
    snprintf(d, sizeof(d),
             "Degradado: el contador queda DENTRO del recuadro del aviso, sin tocar sus lados "
             "(%d px pegados al trazo izquierdo, %d al derecho)", izquierda, derecha);
    comprobar(izquierda == 0 && derecha == 0, d);
  }
  {
    // Y no puede comerse a sus vecinas: ni la linea de detalle de arriba ni el
    // pie con "3=Salir 4=Menu" de abajo, que es por donde se sale.
    int libresArriba = 0, libresAbajo = 0;
    for (int y = 38; y <= 39; y++) if (tintaEnFila(y) == 0) libresArriba++;
    for (int y = 52; y <= 55; y++) if (tintaEnFila(y) == 0) libresAbajo++;
    char d[220];
    snprintf(d, sizeof(d),
             "Degradado: el recuadro no toca ni la linea de arriba ni el pie de abajo "
             "(%d filas libres por encima, %d por debajo)", libresArriba, libresAbajo);
    comprobar(libresArriba >= 1 && libresAbajo >= 1, d);
  }
  {
    // El recuadro llega a x=127 por diseno; el pie de abajo no puede.
    comprobarBordeDerecho("Degradado con aviso (pie)", 56, 63);
  }

  // --- Rendicion por el limite duro ---------------------------------------
  avanzarModo(9UL * 3600UL * 1000UL, 60UL * 1000UL);   // se pasan las 48 h
  comprobar(degradado_syncVencida(),
            "A las 48 h sin sincronizar se levanta el latch de sync vencida");
  avanzarModo(35000UL, 500UL);                          // todo-rojo de despedida
  comprobar(degradado_estado() == DEG_RENDIDO,
            "Superado el limite duro, el modo se rinde solo y cae a ambar");
  abrirDegradado();
  volcar("DEGRADADO — RENDIDO 48h: AMBAR (y el motivo del reintento)");
  perfilFilas("DEGRADADO rendido", 16, 63);
  comprobarMargenes("Degradado rendido");
  // Rendido, el aviso de limite tambien esta puesto, asi que el recuadro ocupa
  // y=40..51 de borde a borde por diseno. El borde derecho se mide en las filas
  // de texto, que son las que U8g2 si podria estar recortando en silencio.
  comprobarCabeConHolgura("Degradado rendido (rotulo y motivo)", 18, 39);
  comprobarBordeDerecho("Degradado rendido (pie)", 56, 63);
  {
    // Rendido, el rotulo de arriba cambia a "RENDIDO 48h. FALTA:" y abajo va el
    // motivo. Son los dos textos mas largos que comparte esta pantalla.
    int bandas = contarBandas(18, 63);
    char d[190];
    snprintf(d, sizeof(d),
             "Degradado rendido: el rotulo, el motivo, el contador y el pie no se solapan (%d bandas)",
             bandas);
    comprobar(bandas == 4, d);
  }

  // Y el cartel de rechazo con el motivo de sync vencida, que es el unico que
  // no se podia provocar mas arriba.
  pulsar(p_ok);
  pulsar(p_ok);
  volcar("CARTEL RECHAZO — SYNC CADUCADA >48h");
  comprobarMargenes("Cartel rechazo sync caducada");
  comprobarBordeDerecho("Cartel rechazo sync caducada", 20, 63);

  // --- El cartel de rechazo con el motivo mas largo posible ----------------
  // lcd_dibujarRechazoDegradado() recibe el texto de fuera. Se le da uno de 20
  // caracteres -el maximo que admite la 6x10 desde x=2- para dejar clavado
  // donde esta el borde, porque es justo el caso que U8g2 recortaria callando.
  lcd_dibujarRechazoDegradado("12345678901234567890");
  volcar("RECHAZO — motivo de 20 caracteres (el maximo que cabe)");
  perfilFilas("RECHAZO motivo maximo", 26, 63);
  comprobarMargenes("Rechazo con motivo de 20 caracteres");
  {
    int xf = ultimaColumna(26, 36);
    char d[200];
    snprintf(d, sizeof(d),
             "Rechazo: un motivo de 20 caracteres -el maximo- cabe entero (acaba en x=%d)", xf);
    comprobar(xf >= 0 && xf <= 126, d);
  }
  {
    // La linea fija "Todo sigue igual. 4=Menu" son 24 caracteres de la 5x7
    // desde x=2: es el texto mas ajustado de toda la pantalla del Esclavo.
    int xf = ultimaColumna(52, 63);
    char d[200];
    snprintf(d, sizeof(d),
             "Rechazo: la linea fija 'Todo sigue igual. 4=Menu' cabe entera (acaba en x=%d)", xf);
    comprobar(xf >= 0 && xf <= 126, d);
  }

  // =========================================================================
  // REGRESO AUTOMATICO AL LISTADO (SFTY-21)
  // =========================================================================
  // No es comodidad de interfaz: es lo que sostiene la inhibicion del mando.
  // Con una pantalla olvidada abierta el mando queda mudo, y desde el suelo no
  // hay forma de notarlo, porque aqui el menu NO detiene el ciclo.
  //
  // Se comprueba con el menu.cpp real y con el reloj del arnes, en las TRES
  // pantallas que quedan por debajo del listado.
  printf("\n\n===========================================================\n");
  printf(" REGRESO AUTOMATICO AL LISTADO A LOS 90 s (sostiene el mando)\n");
  printf("===========================================================\n");
  {
    m_enHora = true; m_cfgRecibida = true; m_verde = 30; m_despeje = 30;

    struct CasoInact { int abajo; int oks; const char *que; };
    CasoInact casos[] = {
      { 0, 1, "ESTADO" },
      { 1, 1, "MODO DEGRADADO" },
      { 1, 2, "CONFIRMAR ENTRADA" },
    };
    for (unsigned i = 0; i < sizeof(casos) / sizeof(casos[0]); i++) {
      menu_setup();
      for (int k = 0; k < casos[i].abajo; k++) pulsar(p_abajo);
      for (int k = 0; k < casos[i].oks; k++)   pulsar(p_ok);

      char d[200];
      snprintf(d, sizeof(d), "Con '%s' abierta el mando esta inhibido", casos[i].que);
      comprobar(menu_estaAbierto(), d);

      // A 89 s todavia NO debe volver: si volviera antes, el operario se
      // quedaria sin la pantalla mientras la esta leyendo. Esta mitad de la
      // comprobacion es la que demuestra que la otra mide algo.
      avanzarConMenu(89000UL, 1000UL);
      snprintf(d, sizeof(d),
               "A los 89 s '%s' sigue abierta: el regreso no se adelanta", casos[i].que);
      comprobar(menu_estaAbierto(), d);

      avanzarConMenu(2000UL, 250UL);
      snprintf(d, sizeof(d),
               "A los 90 s sin tocar nada, '%s' vuelve sola al listado y el mando se rearma",
               casos[i].que);
      comprobar(!menu_estaAbierto(), d);
      if (i == 0) {
        volcar("Vuelta automatica al listado tras 90 s de inactividad");
        comprobarMargenes("Listado tras el regreso automatico");
      }
    }

    // El cursor tiene que quedar en la primera opcion, que es la que SOLO LEE.
    // Si volviera apuntando a MODO DEGRADADO, un pulso perdido llevaria directo
    // a la pantalla que puede cambiar la operacion.
    comprobar(cursorEnBanda(20, 28),
              "Tras el regreso automatico el cursor queda sobre ESTADO, que solo lee");
  }

  printf("\n===========================================================\n");
  if (fallos == 0) {
    printf(" RESULTADO: %d/%d comprobaciones OK\n", comprobaciones, comprobaciones);
  } else {
    printf(" RESULTADO: %d de %d comprobaciones FALLARON\n", fallos, comprobaciones);
  }
  printf("===========================================================\n");
  return fallos == 0 ? 0 : 1;
}
