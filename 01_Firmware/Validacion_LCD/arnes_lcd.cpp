// ===========================================================================
// ARNES DE VALIDACION DE PANTALLA — se compila en el PC, no en la tarjeta
// ===========================================================================
// Compila el MISMO lcd.cpp que va al firmware, pero conectado a un framebuffer
// en memoria en lugar de a la ST7920 real. Con eso se puede:
//
//   1. Volcar cada pantalla como dibujo de texto y verla antes de flashear.
//   2. Comprobar automaticamente que nada se sale de los 128x64 px.
//
// U8g2 recorta en silencio lo que cae fuera de pantalla: un texto mal colocado
// no da error, simplemente no aparece. Ese fue justamente el fallo de la cuarta
// opcion del menu (caia en y=72). Esta comprobacion lo detecta sola.
//
// Uso:  compilar.ps1
// ===========================================================================

#include <stdio.h>
#include <string.h>

#include "U8g2lib.h"   // sin ARDUINO definido -> solo la clase base U8G2
#include "lcd.h"
#include "menu.h"      // se compila el menu.cpp REAL para simular la navegacion
#include "modos.h"     // el enum y modoActual_get(): ya no viven en menu.h

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

// --- Sustitutos del coordinador: la telemetria se inyecta a mano -----------
static int    g_calidad = 100;
static unsigned long g_rtt = 340;
static int    g_fallos = 0;

int           coordinador_calidadEnlace()      { return g_calidad; }
unsigned long coordinador_tiempoRespuestaMs()  { return g_rtt; }
int           coordinador_latidosSinRespuesta(){ return g_fallos; }
void          coordinador_forzarMenu()         { /* sin efecto en el arnes */ }

// --- Contadores de linea simulados (SFTY-15) -------------------------------
static unsigned long g_bytes = 36, g_validas = 9;
unsigned long protocolo_bytesRecibidos()      { return g_bytes; }
unsigned long protocolo_tramasValidas()       { return g_validas; }
unsigned long protocolo_tramasDescartadas()   { return 0; }
void          protocolo_reiniciarContadores() { g_bytes = 0; g_validas = 0; }

// --- Reloj simulado --------------------------------------------------------
// N-31: menu.cpp llama a reloj_reiniciarDominioRespaldo() desde la cuarta opcion
// del submenu. La implementacion real vive en reloj.cpp, que este arnes NO compila:
// toca registros de respaldo del STM32 y no tiene sentido en el PC. Aqui se sustituye
// por un valor que el arnes puede fijar, para poder dibujar las DOS respuestas
// -el cristal arranco o no- sin depender de la tarjeta.
static bool g_cristalArranca = true;
bool reloj_reiniciarDominioRespaldo() { return g_cristalArranca; }

// --- Botonera simulada: se "pulsa" fijando la variable antes de menu_loop() -
static bool p_arriba = false, p_abajo = false, p_ok = false, p_cancelar = false;

// Cada lectura consume la pulsacion, igual que el antirrebote real.
static bool consumir(bool &b) { bool v = b; b = false; return v; }
void botones_setup()   {}
bool botonArriba()     { return consumir(p_arriba); }
bool botonAbajo()      { return consumir(p_abajo); }
bool botonAceptar()    { return consumir(p_ok); }
bool botonCancelar()   { return consumir(p_cancelar); }

// ---------------------------------------------------------------------------
// Utilidades de inspeccion del framebuffer
// ---------------------------------------------------------------------------
static const int ANCHO = 128;
static const int ALTO = 64;

// Empaquetado del framebuffer de la ST7920: HORIZONTAL, con el bit mas
// significativo a la izquierda. Cada fila ocupa 128/8 = 16 bytes.
//   byte = buf[y * 16 + x/8]      bit = 7 - (x % 8)
// Verificado con drawBox(0,40,10,8): buf[640]=0xFF y buf[641]=0xC0, paso de 16.
// (Al principio se supuso empaquetado vertical por tiles y la lectura daba basura;
//  de ahi la calibracion obligatoria antes de dar por buena ninguna medida.)
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

// Comprueba que el contenido dibujado no toca el borde inferior ni queda
// pegado a el (sintoma tipico de texto a punto de recortarse).
static void comprobarMargenes(const char *pantalla) {
  int ultimaFilaConTinta = -1;
  for (int y = ALTO - 1; y >= 0; y--) {
    if (tintaEnFila(y) > 0) { ultimaFilaConTinta = y; break; }
  }
  char desc[160];
  snprintf(desc, sizeof(desc),
           "%s: el contenido termina en y=%d (limite 63)", pantalla, ultimaFilaConTinta);
  comprobar(ultimaFilaConTinta >= 0 && ultimaFilaConTinta <= 63, desc);
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
// Sirve para cazar el recorte SILENCIOSO de U8g2: un texto que no cabe no da error,
// se corta. Si la tinta llega hasta la ultima columna de la pantalla, lo mas probable
// es que el texto siga -invisible- mas alla del borde.
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

// Comprueba que en (x, yBase) hay EXACTAMENTE el texto dado con esa fuente, y no
// solo "algo de tinta". Se dibuja la cadena de referencia sola, en la misma
// posicion, y se compara byte a byte con la banda correspondiente del framebuffer
// real. Es la unica forma de exigir ">48h" en vez de intuirlo por la forma del
// dibujo.
//
// Los limites verticales de la banda salen de getFontAscent()/getFontDescent() de
// la propia fuente -no se cuentan filas a mano-: la misma regla que ya aplica este
// arnes a los anchos (getStrWidth). Cuando el instrumento puede preguntarle a U8g2,
// no se le pregunta a la vista.
//
// OJO: esta funcion REDIBUJA sobre `display` para construir la referencia, asi que
// destruye lo que hubiera en el framebuffer. Se llama la ULTIMA entre las
// comprobaciones de un caso, nunca antes de otra que necesite el dibujo real.
static bool textoEnPosicion(const uint8_t *fuente, int x, int yBase, const char *texto) {
  display.setFont(fuente);
  int y0 = yBase - display.getFontAscent();
  int y1 = yBase - display.getFontDescent();
  if (y0 < 0) y0 = 0;
  if (y1 >= ALTO) y1 = ALTO - 1;
  const int bytesPorFila = ANCHO / 8;
  const int filas = y1 - y0 + 1;
  static const int FILAS_MAX = 24;   // de sobra para cualquier fuente de este arnes
  if (filas <= 0 || filas > FILAS_MAX) return false;

  uint8_t real[FILAS_MAX * (ANCHO / 8)];
  memcpy(real, display.getBufferPtr() + y0 * bytesPorFila, (size_t)filas * bytesPorFila);

  display.clearBuffer();
  display.setFont(fuente);
  display.drawStr(x, yBase, texto);
  uint8_t esperado[FILAS_MAX * (ANCHO / 8)];
  memcpy(esperado, display.getBufferPtr() + y0 * bytesPorFila, (size_t)filas * bytesPorFila);

  return memcmp(real, esperado, (size_t)filas * bytesPorFila) == 0;
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

// ---------------------------------------------------------------------------
int main() {
  printf("===========================================================\n");
  printf(" VALIDACION DE PANTALLA LCD ST7920 (128x64) — compilada en PC\n");
  printf(" Se ejecuta el mismo lcd.cpp que va al firmware.\n");
  printf("===========================================================\n");

  display.begin();

  printf("\n  --- geometria real del buffer ---\n");
  printf("   pantalla      : %d x %d px\n",
         display.getDisplayWidth(), display.getDisplayHeight());
  printf("   buffer (tiles): %d x %d  -> %d bytes por fila de tiles\n",
         display.getBufferTileWidth(), display.getBufferTileHeight(),
         display.getBufferTileWidth() * 8);

  // --- Calibracion del acceso al framebuffer -----------------------------
  // Antes de creer nada, se dibuja un patron conocido y se comprueba que se lee
  // donde toca. Si esto falla, cualquier medida posterior seria basura.
  display.clearBuffer();
  display.drawBox(0, 40, 10, 8);   // rectangulo solido: x 0..9, y 40..47
  {
    // Volcado de bytes crudos: revela el empaquetado real sin suponerlo.
    printf("\n  --- bytes no nulos tras drawBox(0,40,10,8) ---\n");
    {
      const uint8_t *b = display.getBufferPtr();
      int total = display.getBufferTileWidth() * 8 * display.getBufferTileHeight();
      int mostrados = 0;
      for (int i = 0; i < total && mostrados < 24; i++) {
        if (b[i]) { printf("   buf[%4d] = 0x%02X\n", i, b[i]); mostrados++; }
      }
      printf("   (tamano total del buffer: %d bytes)\n", total);
    }

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

  // --- REFERENCIA: el menu de 3 opciones que YA FUNCIONABA en campo -------
  // Criterio de validacion: si el arnes no da por bueno lo que el equipo mostraba
  // correctamente en terreno, el equivocado es el arnes, no el firmware. Esta
  // pantalla es el patron contra el que se contrasta todo lo demas.
  {
    const char *previas[3] = {"MANUAL", "AUTOMATICO", "INTELIGENTE"};
    lcd_dibujarMenu(0, previas, 3);
    volcar("REFERENCIA — menu de 3 opciones (version validada en campo)");
    int bandas = contarBandas(18, 63);
    char d[140];
    snprintf(d, sizeof(d),
             "REFERENCIA: el menu de 3 opciones que funciona en campo se lee con %d bandas "
             "(deben ser 3)", bandas);
    comprobar(bandas == 3, d);
    if (bandas != 3) {
      printf("\n   El arnes no reconoce una pantalla que SI funciona en terreno.\n");
      printf("   Se aborta: las comprobaciones siguientes no serian fiables.\n");
      return 1;
    }
  }

  // --- Menu principal con las 4 opciones ---------------------------------
  const char *opciones[4] = {"MANUAL", "AUTOMATICO", "INTELIGENTE", "PRUEBA ALCANCE"};
  lcd_dibujarMenu(0, opciones, 4);
  volcar("MENU PRINCIPAL (4 opciones, cursor en la 1a)");
  perfilFilas("MENU", 18, 63);
  comprobarMargenes("Menu");
  {
    int bandas = contarBandas(20, 63);   // por debajo del titulo y la linea
    char d[120];
    snprintf(d, sizeof(d), "Menu: se ven %d opciones de 4 (bandas de texto detectadas)", bandas);
    comprobar(bandas == 4, d);
  }

  lcd_dibujarMenu(3, opciones, 4);
  volcar("MENU PRINCIPAL (cursor en PRUEBA ALCANCE)");
  {
    // El cursor '>' se dibuja en x=2..7; debe existir tinta ahi en la 4a banda.
    bool cursorAbajo = false;
    for (int y = 50; y <= 63; y++)
      for (int x = 2; x <= 8; x++)
        if (pixel(x, y)) cursorAbajo = true;
    comprobar(cursorAbajo, "Menu: el cursor es visible sobre la 4a opcion");
  }

  // --- Pantalla de prueba de alcance --------------------------------------
  g_calidad = 100; g_rtt = 340; g_fallos = 0;
  lcd_dibujarAlcance(g_calidad, g_rtt, g_fallos, g_bytes, g_validas);
  volcar("PRUEBA ALCANCE — enlace sano");
  perfilFilas("ALCANCE 100%, banda de la barra", 38, 50);
  comprobarMargenes("Alcance");
  {
    int barra = 0;
    for (int x = 0; x < ANCHO; x++) if (pixel(x, 37)) barra++;
    char d[120];
    snprintf(d, sizeof(d), "Alcance: la barra al 100%% ocupa %d px de ancho", barra);
    comprobar(barra > 90, d);
  }

  g_calidad = 40; g_rtt = 890; g_fallos = 2;
  lcd_dibujarAlcance(g_calidad, g_rtt, g_fallos, g_bytes, g_validas);
  volcar("PRUEBA ALCANCE — enlace degradado (40%)");
  {
    int barra = 0;
    for (int x = 0; x < ANCHO; x++) if (pixel(x, 37)) barra++;
    char d[120];
    snprintf(d, sizeof(d), "Alcance: al 40%% la barra baja a %d px (debe ser menor que al 100%%)", barra);
    comprobar(barra > 20 && barra < 70, d);
  }

  g_calidad = 0; g_rtt = 0; g_fallos = 5;
  lcd_dibujarAlcance(g_calidad, g_rtt, g_fallos, g_bytes, g_validas);
  volcar("PRUEBA ALCANCE — sin enlace");
  comprobarMargenes("Alcance sin enlace");

  g_calidad = -1;
  lcd_dibujarAlcance(g_calidad, 0, 0, g_bytes, g_validas);
  volcar("PRUEBA ALCANCE — midiendo (aun sin muestras)");

  // --- SFTY-15: los tres diagnosticos de linea que antes se veian igual ----
  printf("\n\n===========================================================\n");
  printf(" SFTY-15 — diagnostico de linea en la fila inferior\n");
  printf("===========================================================\n");
  {
    struct Caso { unsigned long bytes, validas; int calidad; const char *que; };
    Caso casos[] = {
      {    0,  0,  0, "NO LLEGA NADA  -> cobertura, canal o antena" },
      { 4512,  0,  0, "LLEGA BASURA   -> cableado, linea flotando, radio atascada" },
      {   36,  9,100, "ENLACE CORRECTO" },
    };
    for (unsigned i = 0; i < sizeof(casos) / sizeof(casos[0]); i++) {
      g_bytes = casos[i].bytes; g_validas = casos[i].validas;
      g_calidad = casos[i].calidad; g_rtt = 340; g_fallos = (casos[i].calidad == 0) ? 5 : 0;
      lcd_dibujarAlcance(g_calidad, g_rtt, g_fallos, g_bytes, g_validas);
      volcar(casos[i].que);
      comprobarMargenes(casos[i].que);
    }
    // La fila inferior no debe invadir el "4=Menu" ni en el caso de mas digitos.
    g_bytes = 999999; g_validas = 0; g_calidad = 0;
    lcd_dibujarAlcance(0, 0, 9, g_bytes, g_validas);
    volcar("SFTY-15 caso extremo: 999999 bytes, 0 tramas");
    int invasion = 0;
    for (int y = 55; y <= 63; y++)
      for (int x = 90; x < 96; x++)
        if (pixel(x, y)) invasion++;
    char d[140];
    snprintf(d, sizeof(d), "SFTY-15 basura: la fila no invade '4=Menu' (%d px)", invasion);
    comprobar(invasion == 0, d);

    // Mismo limite para el caso NORMAL con contadores grandes, que produce el
    // texto mas largo de los tres ("RX ... tramas").
    g_bytes = 999999; g_validas = 249999;
    lcd_dibujarAlcance(100, 340, 0, g_bytes, g_validas);
    volcar("SFTY-15 caso extremo normal: 999999 bytes, 249999 tramas");
    invasion = 0;
    for (int y = 55; y <= 63; y++)
      for (int x = 90; x < 96; x++)
        if (pixel(x, y)) invasion++;
    snprintf(d, sizeof(d), "SFTY-15 normal: la fila no invade '4=Menu' con contadores de 6 cifras (%d px)", invasion);
    comprobar(invasion == 0, d);
  }
  g_bytes = 36; g_validas = 9;

  // --- Modos de operacion con la linea de enlace --------------------------
  g_calidad = 100; g_rtt = 340; g_fallos = 0;
  lcd_dibujarAutomatico("VERDE", 1, 1);
  volcar("MODO AUTOMATICO");
  comprobarMargenes("Automatico");

  lcd_dibujarInteligente("ROJO", 3, true);
  volcar("MODO INTELIGENTE");
  comprobarMargenes("Inteligente");

  lcd_dibujarManual("ROJO");
  volcar("MODO MANUAL");
  comprobarMargenes("Manual");

  // Caso extremo: tiempo de respuesta de 4 cifras, para ver si la linea de
  // enlace invade el "4=Menu" de la derecha.
  g_rtt = 3500;
  lcd_dibujarAutomatico("AMARILLO", 99, 99);
  volcar("MODO AUTOMATICO — caso extremo (99m / 3500ms)");
  perfilFilas("AUTOMATICO extremo, banda inferior", 52, 63);
  {
    // "4=Menu" arranca en x=96. La linea de enlace no debe llegar hasta ahi.
    int invasion = 0;
    for (int y = 56; y <= 63; y++)
      for (int x = 88; x < 96; x++)
        if (pixel(x, y)) invasion++;
    char d[140];
    snprintf(d, sizeof(d), "Automatico extremo: separacion entre la linea RF y '4=Menu' (%d px invadidos)", invasion);
    comprobar(invasion == 0, d);
  }

  // =========================================================================
  // AJUSTAR HORA (V8.6, SFTY-18) — se edita digito a digito
  // =========================================================================
  printf("\n\n===========================================================\n");
  printf(" AJUSTAR HORA — subrayado del digito activo y margenes\n");
  printf("===========================================================\n");

  // El subrayado debe caer bajo el digito que se esta editando, y solo bajo ese.
  // Es la unica realimentacion de "donde estoy" que tiene el operario: si se
  // dibuja en el sitio equivocado, edita a ciegas.
  {
    // PASO es el ancho de la fuente 7x14; no confundir con ANCHO (128 px de pantalla).
    const int X0 = 46, PASO = 7, YSUB = 43;   // deben coincidir con lcd.cpp
    const int posCadena[4] = {0, 1, 3, 4};
    for (int dig = 0; dig < 4; dig++) {
      lcd_dibujarAjusteHora(14, 32, (uint8_t)dig, true);
      char t[64];
      snprintf(t, sizeof(t), "AJUSTAR HORA — digito %d subrayado", dig);
      volcar(t);
      comprobarMargenes(t);

      int esperadoX = X0 + posCadena[dig] * PASO;
      int dentro = 0, fuera = 0;
      for (int x = 0; x < ANCHO; x++) {
        if (!pixel(x, YSUB)) continue;
        if (x >= esperadoX && x < esperadoX + PASO) dentro++;
        else fuera++;
      }
      char d[150];
      snprintf(d, sizeof(d),
               "Ajuste hora: el subrayado del digito %d cae donde debe (%d px dentro, %d px fuera)",
               dig, dentro, fuera);
      comprobar(dentro > 0 && fuera == 0, d);
    }
  }

  // Extremos del rango: 00:00 y 23:59 deben caber igual.
  lcd_dibujarAjusteHora(0, 0, 0, true);
  volcar("AJUSTAR HORA — 00:00");
  comprobarMargenes("Ajuste hora 00:00");

  lcd_dibujarAjusteHora(23, 59, 3, true);
  volcar("AJUSTAR HORA — 23:59");
  comprobarMargenes("Ajuste hora 23:59");

  // Reloj sin poner en hora: el aviso NO debe pisar la fila de ayuda inferior.
  // Si se solapan, el operario no lee ninguna de las dos.
  lcd_dibujarAjusteHora(0, 0, 0, false);
  volcar("AJUSTAR HORA — reloj sin poner en hora");
  comprobarMargenes("Ajuste hora sin reloj");
  {
    // El aviso se dibuja con linea base en y=52 y la ayuda en y=62. Entre el
    // descendente del aviso y el ascendente de la ayuda debe quedar hueco.
    int filaLibre = 0;
    for (int y = 53; y <= 55; y++) {
      bool vacia = true;
      for (int x = 0; x < ANCHO; x++)
        if (pixel(x, y)) { vacia = false; break; }
      if (vacia) filaLibre++;
    }
    char d[140];
    snprintf(d, sizeof(d),
             "Ajuste hora: el aviso de reloj sin poner no se pega a la ayuda inferior (%d filas libres)",
             filaLibre);
    comprobar(filaLibre >= 1, d);
  }

  // --- N-24: reloj sin cristal ---------------------------------------------
  // Es un caso DISTINTO de "reloj sin poner en hora" y por eso tiene aviso propio:
  // uno se arregla desde esta misma pantalla y el otro no se arregla desde el
  // firmware en absoluto. El texto es el mas largo de los dos (25 caracteres contra
  // 23), asi que es el que decide si la fila cabe.
  lcd_dibujarAjusteHora(0, 0, 0, false, false);
  volcar("AJUSTAR HORA — N-24: el cristal Y2 no arranco");
  comprobarMargenes("Ajuste hora sin cristal");
  {
    // El aviso ocupa la banda de la linea base y=52 con la fuente 5x7.
    int xf = ultimaColumna(45, 52);
    char d[170];
    snprintf(d, sizeof(d),
             "Ajuste hora sin cristal: 'SIN CRISTAL: Y2, PILA, R5' cabe entero "
             "(ultima columna con tinta = %d, limite 127)", xf);
    comprobar(xf >= 0 && xf <= 127, d);
  }
  {
    // Igual que en el caso de "reloj sin poner": el aviso no puede pegarse a la
    // fila de ayuda de abajo, o no se lee ninguna de las dos.
    int filaLibre = 0;
    for (int y = 53; y <= 55; y++) {
      bool vacia = true;
      for (int x = 0; x < ANCHO; x++)
        if (pixel(x, y)) { vacia = false; break; }
      if (vacia) filaLibre++;
    }
    char d[150];
    snprintf(d, sizeof(d),
             "Ajuste hora sin cristal: el aviso no se pega a la ayuda inferior (%d filas libres)",
             filaLibre);
    comprobar(filaLibre >= 1, d);
  }
  {
    // Los dos avisos son excluyentes. Si se pintaran los dos, se solaparian en la
    // misma linea base y quedaria un borron ilegible: sin cristal manda sobre
    // sin poner en hora, porque explica la causa en vez del sintoma.
    int bandasAviso = contarBandas(44, 55);
    char d[150];
    snprintf(d, sizeof(d),
             "Ajuste hora sin cristal: se pinta UN solo aviso, no los dos superpuestos (%d bandas)",
             bandasAviso);
    comprobar(bandasAviso == 1, d);
  }

  // =========================================================================
  // SINCRONIZACION DE LA HORA CON EL ESCLAVO (N-23, N-30)
  // =========================================================================
  // Estas pantallas se escribieron con los anchos CONTADOS A MANO, porque el dia que
  // se hicieron se creyo que no habia gcc de host y el arnes no pudo correr. Lo
  // habia: no estaba en el PATH (ver la cabecera de compilar.ps1). Un primer borrador
  // llevaba "El esclavo no contesto" -22 caracteres con la 6x10- y se salia de los
  // 128 px; se cazo contando. Aqui se mide, que es lo que habia que haber hecho.
  printf("\n\n===========================================================\n");
  printf(" SINCRONIZAR HORA — resultado del envio al Esclavo (N-23 / N-30)\n");
  printf("===========================================================\n");
  {
    struct CasoSync { bool esperando, ok, sinReloj; int bandas; const char *que; };
    CasoSync casos[] = {
      { true,  false, false, 2, "SYNC — ENVIANDO... (intercambio en curso)" },
      { false, true,  false, 2, "SYNC — SINCRONIZADA (el esclavo la aplico)" },
      { false, false, false, 3, "SYNC — SOLO MAESTRO (el esclavo no contesta)" },
      { false, false, true,  3, "SYNC — SIN RELOJ (N-30: no se llego a enviar nada)" },
    };
    for (unsigned i = 0; i < sizeof(casos) / sizeof(casos[0]); i++) {
      lcd_dibujarSyncHora(casos[i].esperando, casos[i].ok, casos[i].sinReloj);
      volcar(casos[i].que);
      comprobarMargenes(casos[i].que);

      // Recorte silencioso por la derecha: el sintoma es tinta pegada al borde.
      int xf = ultimaColumna(18, 63);
      char d[190];
      snprintf(d, sizeof(d),
               "%s: ninguna linea se recorta por el borde derecho (ultima columna con tinta = %d)",
               casos[i].que, xf);
      comprobar(xf >= 0 && xf <= 126, d);

      // Las lineas de texto tienen que leerse separadas. Si el titulo grande y la
      // explicacion se tocasen, el operario no distingue una de otra justo cuando
      // esta decidiendo si el problema esta aqui o en la otra punta.
      int bandas = contarBandas(18, 63);
      snprintf(d, sizeof(d), "%s: se leen %d lineas separadas (esperadas %d)",
               casos[i].que, bandas, casos[i].bandas);
      comprobar(bandas == casos[i].bandas, d);
    }
  }

  // =========================================================================
  // CONSULTA DEL RELOJ (N-45)
  // =========================================================================
  // Esta pantalla nacio por lo contrario de lo habitual: no porque faltara una
  // funcion, sino porque la que habia MENTIA. Decia "Revisa Y2, pila y R5" sin haber
  // medido ninguna de las tres cosas, y mando a cambiar los tres componentes con el
  // hardware sano.
  //
  // Sus anchos tambien se contaron a mano al escribirla, y el arnes siguio dando
  // 115/115 -el mismo numero de antes-, que es exactamente como se cuela una pantalla
  // sin cubrir. Aqui se mide.
  //
  // Los casos no son decorativos: llevan el contador de 10 digitos y el ano de 2, que
  // es el texto mas largo que la linea CNT puede producir, y las cinco pistas
  // posibles, incluidas las dos de 20 caracteres.
  printf("\n\n===========================================================\n");
  printf(" CONSULTA RELOJ — los bits del RTC, sin interpretar (N-45)\n");
  printf("===========================================================\n");
  {
    struct CasoDiag { RelojDiag d; bool latido; const char *que; };
    CasoDiag casos[] = {
      // lseOn lseRdy lseByp rtcSel rtcEn cntLeido cnt      cfg    anio
      { { false, false, false, 0, false, false, 0,          false, 0  }, false,
        "DIAG — todo apagado (pista: LSE no se pide)" },
      { { true,  false, false, 1, false, false, 0,          false, 0  }, true,
        "DIAG — pedido y no oscila (el caso de banco de hoy)" },
      { { true,  false, true,  1, false, false, 0,          false, 0  }, false,
        "DIAG — LSE en BYPASS (espera reloj externo)" },
      { { true,  true,  false, 2, true,  true,  4294967295UL, true,  99 }, true,
        "DIAG — contador y ano al maximo (pista de 20: RTC no atado)" },
      { { true,  true,  false, 1, true,  true,  4294967295UL, true,  99 }, false,
        "DIAG — oscila y atado a LSE (pista de 20 caracteres)" },
      { { true,  true,  false, 1, false, false, 0,          true,  26 }, true,
        "DIAG — sin RTCEN: no se leyo el contador, y se dice" },
    };
    for (unsigned i = 0; i < sizeof(casos) / sizeof(casos[0]); i++) {
      lcd_dibujarDiagnosticoReloj(casos[i].d, casos[i].latido);
      volcar(casos[i].que);
      comprobarMargenes(casos[i].que);

      // Recorte silencioso por la derecha. Con un contador de 10 digitos esta es la
      // pantalla del firmware con mas riesgo de pasarse, y U8g2 no avisa: recorta.
      int xf = ultimaColumna(18, 63);
      char d[190];
      snprintf(d, sizeof(d),
               "%s: ninguna linea se recorta por el borde derecho (ultima columna con tinta = %d)",
               casos[i].que, xf);
      comprobar(xf >= 0 && xf <= 126, d);

      // Las cuatro filas de datos tienen que leerse separadas: son numeros que el
      // tecnico APUNTA desde 5 m, y dos filas pegadas se copian mal.
      int bandas = contarBandas(18, 63);
      snprintf(d, sizeof(d), "%s: se leen %d filas de datos separadas (esperadas 4)",
               casos[i].que, bandas);
      comprobar(bandas == 4, d);
    }
  }

  // --- Anchos MEDIDOS, no contados -----------------------------------------
  // Lo anterior comprueba el dibujo ya compuesto. Esto comprueba el material: se le
  // pregunta a U8g2 cuanto mide cada cadena con su fuente real. Es la diferencia
  // entre "no vi que se saliera" y "no cabe, y aqui esta el numero".
  //
  // Margen izquierdo 2 px, pantalla de 128 -> el limite es 2 + ancho <= 128.
  {
    struct CadenaMedida { const uint8_t *fuente; const char *nombreFuente;
                          const char *texto; int x; };
    CadenaMedida cadenas[] = {
      { u8g2_font_5x7_tr,  "5x7",  "SIN CRISTAL: Y2, PILA, R5", 2 },
      { u8g2_font_5x7_tr,  "5x7",  "RELOJ SIN PONER EN HORA",   2 },
      { u8g2_font_5x7_tr,  "5x7",  "1=+ 2=- 3=sig 4=salir",     2 },
      // N-50: "Sin sync: >48h" a la misma x=2 que usa lcd_dibujarDegradado() en
      // lcd.cpp. Es el texto fijo de la rama syncVencida=true; el mas largo de los
      // dos formatos ("Sin sync: >48h" contra "Sin sync: NNhNNm") tiene que medirse
      // igual que los demas, no darse por bueno porque "se ve mas corto".
      { u8g2_font_5x7_tr,  "5x7",  "Sin sync: >48h",            2 },
      { u8g2_font_6x10_tr, "6x10", "Esperando al esclavo",      2 },
      { u8g2_font_6x10_tr, "6x10", "El esclavo la aplico",      2 },
      { u8g2_font_6x10_tr, "6x10", "Esclavo no responde",       2 },
      { u8g2_font_6x10_tr, "6x10", "No habra Degradado",        2 },
      { u8g2_font_6x10_tr, "6x10", "No se envio nada",          2 },
      { u8g2_font_6x10_tr, "6x10", "Revisa Y2, pila y R5",      2 },
      // N-39: ESTOS CUATRO SE MEDIAN CON LA FUENTE EQUIVOCADA.
      //
      // N-38 cambio los titulos de lcd.cpp de ncenB10 a 7x14B -de paso fijo, para que
      // el ancho volviera a ser calculable- pero AQUI se quedo escrita la ncenB10. El
      // arnes seguia midiendo una fuente que el firmware ya no usa, y por eso daba
      // dos FALLA sobre un codigo que estaba bien.
      //
      // Es la misma familia que N-36: alli el instrumento leia un fichero que ya no
      // existia; aqui lee el fichero bueno con una suposicion vieja dentro. La
      // leccion es que el instrumento tambien envejece, y nadie le pone fecha de
      // caducidad. La guarda que lo impide de verdad esta en el pack
      // maestro_06_fuentes_pantalla: comprueba contra lcd.cpp que la fuente sea la
      // que aqui se dice.
      { u8g2_font_7x14B_tr, "7x14B", "ENVIANDO...",  4 },
      { u8g2_font_7x14B_tr, "7x14B", "SINCRONIZADA", 4 },
      { u8g2_font_7x14B_tr, "7x14B", "SOLO MAESTRO", 4 },
      { u8g2_font_7x14B_tr, "7x14B", "SIN RELOJ",    4 },
    };
    printf("\n  --- anchos medidos con getStrWidth() ---\n");
    for (unsigned i = 0; i < sizeof(cadenas) / sizeof(cadenas[0]); i++) {
      display.setFont(cadenas[i].fuente);
      int w = display.getStrWidth(cadenas[i].texto);
      int fin = cadenas[i].x + w;
      printf("   %-5s x=%d  ancho=%3d  termina en %3d  \"%s\"\n",
             cadenas[i].nombreFuente, cadenas[i].x, w, fin, cadenas[i].texto);
      char d[200];
      snprintf(d, sizeof(d), "Ancho medido: \"%s\" (%s) termina en x=%d y cabe en 128",
               cadenas[i].texto, cadenas[i].nombreFuente, fin);
      comprobar(fin <= 128, d);
    }

    // CONTROL NEGATIVO DE N-38: por que hubo que cambiar de fuente.
    //
    // Se exige que los dos titulos que se salian NO quepan con la ncenB10 original.
    // Sin esto, el cambio a 7x14B queda como una preferencia estetica y nada impide
    // que alguien lo revierta "porque se ve mejor". Con esto, revertirlo pone el
    // arnes en rojo y dice exactamente por que.
    const char *desbordaban[] = { "SINCRONIZADA", "SOLO MAESTRO" };
    display.setFont(u8g2_font_ncenB10_tr);
    for (unsigned i = 0; i < 2; i++) {
      int fin = 4 + display.getStrWidth(desbordaban[i]);
      char d[200];
      snprintf(d, sizeof(d),
               "Control negativo: \"%s\" con la ncenB10 de antes de N-38 termina en "
               "x=%d y NO cabe: por eso se cambio a 7x14B, no por gusto",
               desbordaban[i], fin);
      comprobar(fin > 128, d);
    }
  }

  // --- El instrumento tiene que saber decir que NO ---------------------------
  // Se le da al arnes la cadena que de verdad se salio -"El esclavo no contesto",
  // el primer borrador de N-23- y se exige que la rechace. Una comprobacion de
  // ancho que aprueba todo no comprueba nada: esto deja clavado que el limite
  // existe y que esta donde se cree.
  {
    display.setFont(u8g2_font_6x10_tr);
    int wMalo = display.getStrWidth("El esclavo no contesto");
    int wBueno = display.getStrWidth("Esclavo no responde");
    char d[200];
    snprintf(d, sizeof(d),
             "Control: el borrador descartado \"El esclavo no contesto\" NO cabe "
             "(termina en x=%d, pasa de 128) y el texto que quedo si (x=%d)",
             2 + wMalo, 2 + wBueno);
    comprobar(2 + wMalo > 128 && 2 + wBueno <= 128, d);
  }

  // =========================================================================
  // NAVEGACION REAL: se ejecuta el menu.cpp del firmware, pulsando botones
  // =========================================================================
  printf("\n\n===========================================================\n");
  printf(" NAVEGACION — ejecutando el menu.cpp real con botonera simulada\n");
  printf("===========================================================\n");

  // V8.7 (SFTY-21): el menu es de DOS NIVELES.
  //
  //   MENU PRINCIPAL (4)              SUBMENU "CONFIGURACION" (3)
  //     MANUAL                          PRUEBA ALCANCE
  //     AUTOMATICO                      AJUSTAR HORA
  //     INTELIGENTE                     MODO DEGRADADO
  //     CONFIGURACION ->
  //
  // El motivo es este arnes tanto como la ergonomia: una sexta opcion en lista plana
  // caia en y=69, fuera de los 64 px. Y lo peligroso no era que no se pintase -la
  // salvaguarda de lcd_dibujarMenu() lo impide- sino que el CURSOR SI PODIA LLEGAR
  // hasta ella. Con dos niveles, el principal vuelve a las 4 opciones del layout
  // validado en campo y el submenu se queda en 3.
  //
  // Lo que se comprueba aqui es exactamente eso: que ningun nivel pinta mas lineas de
  // las que caben y que el cursor SIEMPRE esta sobre una opcion visible.
  g_calidad = 100; g_rtt = 340; g_fallos = 0;
  menu_setup();
  volcar("MENU PRINCIPAL al entrar (cursor arriba del todo)");
  comprobarMargenes("Menu principal");
  {
    int bandas = contarBandas(20, 63);
    char d[130];
    snprintf(d, sizeof(d), "Menu principal: se ven %d opciones de 4", bandas);
    comprobar(bandas == 4, d);
  }

  // Bajar 5 veces: debe recorrer las 4 opciones y volver a la primera. En CADA paso se
  // comprueba que el cursor esta sobre una opcion DIBUJADA: es la comprobacion que
  // habria cazado la sexta opcion invisible.
  const int N_RAIZ = 4;
  const char *nombresRaiz[N_RAIZ] = {"MANUAL", "AUTOMATICO", "INTELIGENTE",
                                     "CONFIGURACION"};
  {
    bool cursorSiempreVisible = true;
    for (int i = 1; i <= N_RAIZ + 1; i++) {
      p_abajo = true;
      menu_loop();
      int pos = i % N_RAIZ;
      printf("\n  Boton 2 (abajo) x%d  ->  cursor esperado en '%s'\n", i, nombresRaiz[pos]);
      volcar(nombresRaiz[pos]);
      // Layout de 4 opciones: base 28, paso 11 -> y = 28, 39, 50, 61
      int y = 28 + pos * 11;
      if (!cursorEnBanda(y - 8, y)) cursorSiempreVisible = false;
      if (!(y <= 63)) cursorSiempreVisible = false;
    }
    comprobar(cursorSiempreVisible,
              "Menu principal: el cursor esta SIEMPRE sobre una opcion dibujada y dentro de pantalla");
  }

  // Subir desde la PRIMERA opcion: debe dar la vuelta hasta la ultima.
  menu_setup();
  p_arriba = true;
  menu_loop();
  volcar("Boton 1 (arriba) desde la 1a opcion -> debe saltar a la ULTIMA");
  comprobar(cursorEnBanda(50, 63),
            "Navegacion: subir desde la primera opcion da la vuelta hasta la ultima");

  // Boton 3 sobre CONFIGURACION: NO arranca ningun modo, baja de nivel.
  p_ok = true;
  menu_loop();
  volcar("SUBMENU CONFIGURACION (entrado con el Boton 3)");
  {
    char d[150];
    snprintf(d, sizeof(d),
             "CONFIGURACION no arranca ningun modo: el sistema sigue en MENU (obtenido=%d, esperado=%d)",
             (int)modoActual_get(), (int)MENU);
    comprobar(modoActual_get() == MENU, d);
  }
  comprobarMargenes("Submenu CONFIGURACION");
  {
    // N-40: EL SUBMENU TIENE CUATRO OPCIONES, NO TRES.
    //
    // N-31 anadio REINICIAR RELOJ como cuarta -menu.cpp lo dice en su propio
    // comentario: "N-31: la cuarta, REINICIAR RELOJ"- y esta prueba se quedo
    // exigiendo 3. Reportaba un FALLA sobre un menu correcto, igual que N-39 con la
    // fuente. Tercera vez el mismo patron en una semana: el instrumento se queda atras
    // y acusa al firmware de su propio retraso.
    //
    // El numero NO se vuelve a escribir a mano en dos sitios: el pack
    // maestro_07_menu_opciones lee OPCIONES_CONFIG de menu.cpp y exige que coincida
    // con el que hay aqui. Si manana entra una quinta opcion, salta el pack.
    const int OPCIONES_SUBMENU = 4;
    int bandas = contarBandas(20, 63);
    char d[130];
    snprintf(d, sizeof(d), "Submenu: se ven %d opciones de %d", bandas, OPCIONES_SUBMENU);
    comprobar(bandas == OPCIONES_SUBMENU, d);
  }
  comprobar(cursorEnBanda(20, 28), "Submenu: el cursor arranca sobre la 1a opcion y es visible");
  {
    // El titulo propio del submenu se centra sobre su ancho real. Si se saliera por un
    // lado, U8g2 lo recortaria en silencio.
    int xi = primeraColumna(1, 14), xf = ultimaColumna(1, 14);
    char d[150];
    snprintf(d, sizeof(d),
             "Submenu: el titulo 'CONFIGURACION' cabe centrado (x=%d..%d, limite 0..127)", xi, xf);
    comprobar(xi >= 10 && xf <= 118, d);
  }

  // Boton 4 en el submenu: vuelve al menu principal, no sale a un modo, y deja el
  // cursor sobre CONFIGURACION.
  p_cancelar = true;
  menu_loop();
  volcar("Boton 4 en el submenu -> vuelve al MENU PRINCIPAL");
  {
    int bandas = contarBandas(20, 63);
    char d[150];
    snprintf(d, sizeof(d),
             "Boton 4 en el submenu vuelve al menu principal (%d opciones) sin arrancar modo (modo=%d)",
             bandas, (int)modoActual_get());
    comprobar(bandas == 4 && modoActual_get() == MENU, d);
  }
  comprobar(cursorEnBanda(50, 63),
            "Al volver del submenu el cursor queda sobre CONFIGURACION");

  // Cada posicion del MENU PRINCIPAL selecciona el modo que corresponde.
  {
    ModoSistema esperados[3] = {MODO_MANUAL, MODO_AUTOMATICO, MODO_INTELIGENTE};
    bool todos = true;
    for (int i = 0; i < 3; i++) {
      menu_setup();
      for (int k = 0; k < i; k++) { p_abajo = true; menu_loop(); }
      p_ok = true; menu_loop();
      if (modoActual_get() != esperados[i]) todos = false;
      printf("   principal %d ('%s') -> modo %d %s\n", i, nombresRaiz[i], (int)modoActual_get(),
             (modoActual_get() == esperados[i]) ? "OK" : "<-- NO CORRESPONDE");
    }
    comprobar(todos, "Menu principal: las 3 opciones de operacion seleccionan su modo correcto");
  }

  // Cada posicion del SUBMENU selecciona el modo que corresponde. La tercera es
  // MODO_DEGRADADO: la que da verde por reloj, y por tanto la que no puede quedar
  // colgando de una opcion mal indexada.
  {
    ModoSistema esperados[3] = {MODO_ALCANCE, MODO_HORA, MODO_DEGRADADO};
    const char *nombresCfg[3] = {"PRUEBA ALCANCE", "AJUSTAR HORA", "MODO DEGRADADO"};
    bool todos = true;
    for (int i = 0; i < 3; i++) {
      menu_setup();
      for (int k = 0; k < N_RAIZ - 1; k++) { p_abajo = true; menu_loop(); } // hasta CONFIGURACION
      p_ok = true; menu_loop();                                            // entrar al submenu
      for (int k = 0; k < i; k++) { p_abajo = true; menu_loop(); }
      p_ok = true; menu_loop();
      if (modoActual_get() != esperados[i]) todos = false;
      printf("   submenu %d ('%s') -> modo %d %s\n", i, nombresCfg[i], (int)modoActual_get(),
             (modoActual_get() == esperados[i]) ? "OK" : "<-- NO CORRESPONDE");
    }
    comprobar(todos, "Submenu: las 3 opciones seleccionan cada una su modo correcto (incluido MODO_DEGRADADO)");
  }

  // =========================================================================
  // COMPORTAMIENTO DE LA PANTALLA DE ALCANCE AL ALEJARSE
  // =========================================================================
  printf("\n\n===========================================================\n");
  printf(" PRUEBA DE ALCANCE — evolucion al alejar el equipo\n");
  printf("===========================================================\n");
  {
    struct Paso { int calidad; unsigned long rtt; int fallos; const char *sitio; };
    Paso recorrido[] = {
      { -1,    0, 0, "recien entrado, aun sin medir" },
      { 100, 320, 0, "junto al otro semaforo" },
      { 100, 350, 0, "a media obra" },
      {  90, 380, 1, "empieza a perder alguno" },
      {  70, 420, 1, "al limite util" },
      {  40, 500, 2, "muy degradado" },
      {  10, 700, 4, "a punto de perderse" },
      {   0,   0, 7, "sin enlace: hasta aqui llega" },
    };
    for (unsigned i = 0; i < sizeof(recorrido) / sizeof(recorrido[0]); i++) {
      g_calidad = recorrido[i].calidad;
      g_rtt = recorrido[i].rtt;
      g_fallos = recorrido[i].fallos;
      lcd_dibujarAlcance(g_calidad, g_rtt, g_fallos, g_bytes, g_validas);
      char t[120];
      snprintf(t, sizeof(t), "%s  (calidad=%d)", recorrido[i].sitio, recorrido[i].calidad);
      volcar(t);
      comprobarMargenes(recorrido[i].sitio);
    }
  }

  // =========================================================================
  // MODO DEGRADADO (V8.7, SFTY-21)
  // =========================================================================
  // Es la unica pantalla que acompana a un VERDE dado sin confirmacion del otro
  // extremo. Si un dato de aqui se recorta en silencio, el tecnico decide sobre una
  // informacion que no esta viendo entera.
  printf("\n\n===========================================================\n");
  printf(" MODO DEGRADADO — fase, cuenta atras, horas sin sync y aviso\n");
  printf("===========================================================\n");
  {
    struct CasoDeg {
      const char *fase, *detalle;
      unsigned long restante, minSync;
      const char *aviso;
      const char *que;
    };
    CasoDeg casos[] = {
      { "VERDE", "Paso por el maestro", 29,    3, 0,
        "DEGRADADO — verde del maestro, recien sincronizado" },
      { "ROJO",  "Despeje total",       30,   65, 0,
        "DEGRADADO — despeje todo-rojo" },
      { "ROJO",  "Paso por el esclavo", 12,  610, 0,
        "DEGRADADO — verde del esclavo (aqui, rojo)" },
      { "ROJO",  "Entrando: todo rojo", 120, 119, 0,
        "DEGRADADO — todo-rojo de entrada, cuenta atras de 3 cifras" },
      { "ROJO",  "Despeje total",       30, 2759, "AVISO: LIMITE 48h",
        "DEGRADADO — caso extremo bajo el limite: 45h59m sin sync y aviso" },
      // N-50: LOS DOS CASOS QUE FALTABAN. El minSync mas alto de arriba es 2759
      // (45h59m), por debajo de las 2880 (48h) que exige LIMITE_DURO_MS: el
      // argumento casos[i].minSync >= 2880 de mas abajo era SIEMPRE false y la rama
      // syncVencida=true del firmware jamas se ejercia. Estos dos casos cruzan el
      // limite -uno justo en el, otro muy por encima- para que ese >= dependa de
      // verdad del dato y no sea un adorno.
      { "ROJO",  "Despeje total",       30, 2880, "AVISO: LIMITE 48h",
        "DEGRADADO — limite duro justo alcanzado (48h00m, syncVencida)" },
      { "ROJO",  "Despeje total",       30, 4200, "AVISO: LIMITE 48h",
        "DEGRADADO — muy por encima del limite (70h00m, syncVencida)" },
    };
    for (unsigned i = 0; i < sizeof(casos) / sizeof(casos[0]); i++) {
      bool vencida = casos[i].minSync >= 2880;
      lcd_dibujarDegradado(casos[i].fase, casos[i].detalle, casos[i].restante,
                           casos[i].minSync, vencida, casos[i].aviso);
      volcar(casos[i].que);
      comprobarMargenes(casos[i].que);

      // Nada puede tocar la ultima columna: en U8g2 eso es el sintoma de un texto que
      // sigue fuera de pantalla, recortado sin avisar.
      int xf = ultimaColumna(18, 63);  // desde 18: la linea separadora de y=15 ocupa el ancho entero
      char d[170];
      snprintf(d, sizeof(d), "%s: nada se recorta por el borde derecho (ultima columna con tinta = %d)",
               casos[i].que, xf);
      comprobar(xf >= 0 && xf <= 126, d);

      // La linea de sync tiene que decir EXACTAMENTE lo que corresponde al caso: el
      // texto fijo ">48h" si esta vencida, o el "Nh MMm" de siempre si no. No basta
      // con que haya tinta en la fila -eso ya lo comprueban los margenes-: hace
      // falta el contenido. Se llama al final porque textoEnPosicion() redibuja el
      // framebuffer para construir la referencia.
      char textoEsperado[24];
      if (vencida) {
        snprintf(textoEsperado, sizeof(textoEsperado), "Sin sync: >48h");
      } else {
        snprintf(textoEsperado, sizeof(textoEsperado), "Sin sync: %luh%02lum",
                 casos[i].minSync / 60UL, casos[i].minSync % 60UL);
      }
      bool coincide = textoEnPosicion(u8g2_font_5x7_tr, 2, 54, textoEsperado);
      snprintf(d, sizeof(d), "%s: la linea de sync dice exactamente \"%s\"",
               casos[i].que, textoEsperado);
      comprobar(coincide, d);
    }

    // La cuenta atras se pinta en x=88; el "4=Menu" vive en x=96..125 de la fila
    // inferior. Ni el aviso mas largo puede invadirlo: salir tiene que poder hacerse
    // aunque la pantalla este dando una alarma.
    lcd_dibujarDegradado("ROJO", "Despeje total", 120, 2759, false, "AVISO: LIMITE 48h");
    perfilFilas("DEGRADADO extremo, fila inferior", 55, 63);
    {
      int invasion = 0;
      for (int y = 55; y <= 63; y++)
        for (int x = 88; x < 96; x++)
          if (pixel(x, y)) invasion++;
      char d[150];
      snprintf(d, sizeof(d),
               "Degradado: el aviso de limite no invade '4=Menu' (%d px invadidos)", invasion);
      comprobar(invasion == 0, d);
    }
    {
      // Las cuatro bandas de informacion (fase+cuenta, detalle, sync, fila inferior)
      // deben quedar separadas: si dos se solapan, no se lee ninguna de las dos.
      int bandas = contarBandas(18, 63);
      char d[150];
      snprintf(d, sizeof(d),
               "Degradado: las 4 lineas de datos no se solapan entre si (%d bandas)", bandas);
      comprobar(bandas == 4, d);
    }

    // Entrada rechazada: tiene que decir CUAL condicion falta, en dos lineas legibles.
    struct CasoRech { const char *l1, *l2, *que; };
    CasoRech rechazos[] = {
      { "Falta: reloj sin",  "poner en hora",     "RECHAZO — reloj sin poner en hora" },
      { "Falta: nunca hubo", "sincronizacion RF", "RECHAZO — nunca sincronizado" },
      { "Falta: la ultima",  "sync es muy vieja", "RECHAZO — sincronizacion caducada" },
      { "Falta: sin medida", "de desfase valida", "RECHAZO — sin medida de desfase" },
      { "Desfase fuera de",  "tolerancia (+-3s)", "RECHAZO — desfase fuera de tolerancia" },
    };
    for (unsigned i = 0; i < sizeof(rechazos) / sizeof(rechazos[0]); i++) {
      lcd_dibujarDegradadoRechazo(rechazos[i].l1, rechazos[i].l2);
      volcar(rechazos[i].que);
      comprobarMargenes(rechazos[i].que);
      int xf = ultimaColumna(18, 63);  // desde 18: la linea separadora de y=15 ocupa el ancho entero
      char d[170];
      snprintf(d, sizeof(d), "%s: el motivo cabe entero (ultima columna con tinta = %d)",
               rechazos[i].que, xf);
      comprobar(xf >= 0 && xf <= 126, d);
    }
    {
      int bandas = contarBandas(18, 63);
      char d[150];
      snprintf(d, sizeof(d),
               "Rechazo: RECHAZADO y las dos lineas del motivo se leen separadas (%d bandas)", bandas);
      comprobar(bandas == 3, d);
    }

    // Caida a ambar: por el limite duro de 48 h o por peticion desde el mando.
    struct CasoAmb { const char *l1, *l2, *que; };
    CasoAmb ambares[] = {
      { "Limite 48h sin sync",  "Revise el radio",        "AMBAR — limite duro de 48 h agotado" },
      { "Reloj no fiable",      "Degradado detenido",     "AMBAR — el reloj dejo de ser fiable" },
      { "Ambar pedido desde",   "el mando (B.B.B)",       "AMBAR — pedido desde el mando de reles" },
      { "Saliendo: todo rojo",  "Vea las dos puntas",     "SALIDA — todo-rojo antes de volver al menu" },
    };
    for (unsigned i = 0; i < sizeof(ambares) / sizeof(ambares[0]); i++) {
      lcd_dibujarDegradadoAmbar(ambares[i].l1, ambares[i].l2);
      volcar(ambares[i].que);
      comprobarMargenes(ambares[i].que);
      int xf = ultimaColumna(18, 63);  // desde 18: la linea separadora de y=15 ocupa el ancho entero
      char d[170];
      snprintf(d, sizeof(d), "%s: el texto cabe entero (ultima columna con tinta = %d)",
               ambares[i].que, xf);
      comprobar(xf >= 0 && xf <= 126, d);
    }
    {
      // Limite real de la fila: la fuente 6x10 dibujada desde x=2 admite 20
      // caracteres (2 + 20*6 = 122). Se prueba con una linea DE 20 para dejar clavado
      // donde esta el borde, porque es el caso que U8g2 recortaria sin decir nada.
      lcd_dibujarDegradadoAmbar("Saliendo: todo rojo", "12345678901234567890");
      int xf = ultimaColumna(37, 44);
      char d[170];
      snprintf(d, sizeof(d),
               "Ambar: una linea de 20 caracteres -el maximo- todavia cabe entera (acaba en x=%d)", xf);
      comprobar(xf >= 0 && xf <= 126, d);
    }
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
