// ===========================================================================
// DIAGNOSTICO_LCD — firmware minimo para la tarjeta del ESCLAVO (N-22)
// ===========================================================================
//
// EL PROBLEMA QUE VIENE A RESOLVER
//
// La LCD del Esclavo enciende la retroiluminacion (azul) pero no pinta ni un
// pixel. La del Maestro pinta, y las DOS pantallas fisicas funcionan probadas en
// el Maestro. Ya se descarto, comprobandolo:
//
//   · pines.h es identico byte a byte en las dos puntas.
//   · El constructor U8G2_ST7920_128X64_F_SW_SPI lleva los mismos cuatro pines.
//   · lcd_setup() es identico (PSB en LOW, u8g2.begin()).
//   · lcd_dibujarBienvenida() hace clearBuffer / dibuja / sendBuffer en las dos.
//   · El orden de arranque esta igualado, delay(2000) incluido.
//   · Se probo lcd_setup() en primera y en ULTIMA posicion. Los dos extremos
//     fallan igual, asi que el INSTANTE de inicializacion no es la variable.
//
// LO QUE FALTABA, Y ES LA RAZON DE ESTE PROYECTO
//
// Hasta hoy no habia forma de distinguir estas dos situaciones:
//
//     (a) la tarjeta esta en bucle de reinicio y nunca llega a la pantalla
//     (b) la tarjeta corre perfectamente y es la pantalla la que no pinta
//
// Desde fuera las dos se ven igual: una LCD azul y muda. Mientras no se sepa cual
// de las dos es, todo lo demas son conjeturas. Por eso lo PRIMERO que hace este
// firmware es parpadear un testigo: si parpadea, (a) queda descartado y el fallo
// esta en la pantalla o en su cableado; si no parpadea, no hay nada que buscar en
// la LCD porque el micro no esta corriendo este codigo.
//
// QUE HACE Y QUE NO HACE
//
// Solo enciende un testigo y pinta rellenos. Sin radio, sin reloj, sin respaldo,
// sin watchdog, sin menu, sin botones. Si con esto tampoco pinta, el fallo no
// puede estar en ninguna de esas cosas, y eso es informacion aunque no arregle
// nada todavia.
//
// POR QUE RELLENOS Y NO TEXTO
//
// Un relleno total se ve aunque el contraste este casi al minimo. Un texto fino
// de 5 px de trazo, no. Si la pantalla estuviera funcionando con el contraste mal
// ajustado, el texto de bienvenida seria invisible y el sintoma seria exactamente
// el que se esta viendo: "azul y nada". Las fases de este firmware van de lo mas
// visible a lo menos visible justo para separar ese caso.
//
// HIPOTESIS VIVA QUE ESTE FIRMWARE PONE A PRUEBA
//
// Niveles logicos marginales. Con el modulo alimentado a 5 V, el V_IH garantizado
// del ST7920 es 0,7*VDD = 3,5 V y el STM32 entrega 3,3 V. Esta fuera de
// especificacion en LAS DOS puntas: que el Maestro pinte demuestra que aquel chip
// tolera, no que el diseno sea correcto. Un margen asi falla de forma sucia -unas
// unidades si y otras no, o segun temperatura- y depende de los FLANCOS. De ahi
// VARIANTE_LENTA: si al frenar el reloj empieza a pintar, el problema son los
// umbrales, no el cableado.
//
// Ver README.md de esta carpeta para el arbol de decision completo.
// ===========================================================================

#include <Arduino.h>
#include <U8g2lib.h>
#include "pines.h"

// ===========================================================================
// ===  VARIANTES — ESTE ES EL UNICO SITIO QUE HAY QUE TOCAR  ================
// ===========================================================================
//
// Cambie UNA variante cada vez y vuelva a cargar. Cambiar dos a la vez y que
// funcione no dice cual de las dos era.
//
// No hace falta apuntar en un papel que combinacion esta cargada: al arrancar, el
// testigo la deletrea con grupos de parpadeos (ver "FIRMA DE ARRANQUE" abajo).

// --- 1) PSB: modo serie o paralelo ----------------------------------------
//
// El ST7920 lee PSB para elegir interfaz: LOW = serie (que es lo que usa U8g2 con
// SW_SPI), HIGH = paralelo de 8 bits.
//
// Por que hay tres opciones y no dos: muchos modulos traen PSB puenteado en la
// placa con una resistencia o una gota de estano. Si el modulo lo tiene atado a
// VCC y el STM32 lo fuerza a LOW, los dos se estan peleando: el pin del micro
// intenta hundir a masa una linea que la placa sujeta arriba. El resultado
// depende del valor de la resistencia del strap y puede quedarse en tierra de
// nadie, ni LOW ni HIGH, con el controlador en modo paralelo esperando datos que
// nunca llegan por el puerto serie. Esa es exactamente la averia que da una
// pantalla encendida y muda.
//
//   PSB_BAJO    lo que hace el firmware actual del Esclavo. Punto de partida.
//   PSB_ALTO    absurdo a proposito: fuerza modo PARALELO. Sirve de control
//               NEGATIVO. Si con esta variante se ve ALGO distinto en pantalla
//               -basura, lineas, cualquier cosa-, el chip esta vivo y escuchando,
//               y el fallo esta en el modo o en el dato, no en la alimentacion.
//   PSB_SUELTO  deja el pin del STM32 como ENTRADA (alta impedancia): no pelea
//               con nadie y manda el strap del modulo. Es LA prueba del strap.
//               Si con el pin suelto la pantalla pinta, PSB estaba puenteado y el
//               micro estaba estorbando. MIDA PSB CON EL MULTIMETRO EN ESTA
//               VARIANTE: si marca ~5 V o ~3,3 V, hay strap a VCC y el modo serie
//               nunca se selecciono.
#define PSB_BAJO    1
#define PSB_ALTO    2
#define PSB_SUELTO  3
#define VARIANTE_PSB   PSB_BAJO

// --- 2) RST: controlado o no ----------------------------------------------
//
// U8g2 pulsa RST al arrancar. Si esa linea esta en corto a masa -pista danada,
// soldadura puenteada, cable pellizcado-, el ST7920 se queda RETENIDO EN RESET
// para siempre: alimentado, con la retroiluminacion encendida, y sin procesar ni
// un comando. Sintoma identico al de N-22.
//
// Con 1 aqui se pasa U8X8_PIN_NONE y U8g2 no toca RST en absoluto. El modulo
// arranca con su propio circuito de reset interno, que es como funcionan las
// placas que no llevan esa linea cableada.
//
// SI CON ESTO PINTA, el problema es la linea de RST (PB7) y no la pantalla.
// Compruebelo con el multimetro: PB7 contra masa en continuidad.
#define VARIANTE_SIN_RST   0

// --- 3) VELOCIDAD del bit-bang --------------------------------------------
//
// ESTA ES LA VARIANTE CLAVE PARA LA HIPOTESIS DE NIVELES.
//
// COMO SE FRENA U8g2 EN SPI POR SOFTWARE, QUE NO ES COMO PARECE:
//
//   u8g2.setBusClock() NO SIRVE AQUI. Guarda un valor en u8x8.bus_clock que solo
//   leen los transportes por HARDWARE (SPI y I2C del periferico). El transporte
//   de este display es u8x8_byte_arduino_4wire_sw_spi, que mueve los pines con
//   digitalWrite() en un bucle y jamas consulta bus_clock. Llamar a setBusClock
//   compila, no da error y no cambia absolutamente nada: es una trampa comoda
//   para perder una tarde concluyendo que "con reloj lento tampoco pinta".
//
//   LO QUE SI FUNCIONA es sustituir el callback de GPIO y retardo. Entre flanco y
//   flanco, u8x8_byte_arduino_4wire_sw_spi pide un retardo con el mensaje
//   U8X8_MSG_DELAY_NANO, y la implementacion de Arduino lo resuelve casi con un
//   no-operar porque en un STM32 a 72 MHz el propio digitalWrite ya tarda mas que
//   el pulso minimo que pide la hoja de datos. Aqui se envuelve ese callback: se
//   delega en el original para todo, y ademas se inserta un retardo real DESPUES
//   DE CADA CAMBIO DE PIN y en los mensajes de retardo. Con eso el reloj serie
//   baja de cientos de kHz a unos pocos kHz.
//
// QUE CONCLUYE EL OPERARIO:
//
//   Si con VARIANTE_LENTA a 1 la pantalla EMPIEZA A PINTAR (aunque tarde varios
//   segundos en refrescar, que es lo normal a esta velocidad), el problema son
//   los FLANCOS Y LOS UMBRALES, no el cableado ni el firmware: la entrada del
//   ST7920 no alcanza su V_IH de 3,5 V a la velocidad normal y solo consigue
//   discriminar los bits cuando se le da tiempo. La solucion entonces NO es de
//   software: es alimentar el modulo a 3,3 V en lugar de a 5 V, o poner un
//   adaptador de nivel en SCLK, SID y CS.
//
//   Si a 1 sigue sin pintar nada, los niveles quedan descartados como causa
//   UNICA y hay que seguir por PSB y RST.
//
// RETARDO_US: microsegundos por flanco. 20 us da un reloj de unos 12 kHz, muy por
// debajo de cualquier limite del ST7920 y aun asi refresca la pantalla entera en
// menos de un segundo. Si a 20 no cambia nada, pruebe 100 antes de descartar.
#define VARIANTE_LENTA   0
#define RETARDO_US       20

// ===========================================================================
// ===  FIN DE LAS VARIANTES  ================================================
// ===========================================================================

// --- Construccion del display ----------------------------------------------
// Mismos pines y mismo constructor que el Esclavo. Lo unico que cambia segun la
// variante es si RST se entrega o se deja sin controlar.
#if VARIANTE_SIN_RST
static U8G2_ST7920_128X64_F_SW_SPI u8g2(U8G2_R0, LCD_SCLK, LCD_SID, LCD_CS,
                                        U8X8_PIN_NONE);
#else
static U8G2_ST7920_128X64_F_SW_SPI u8g2(U8G2_R0, LCD_SCLK, LCD_SID, LCD_CS,
                                        LCD_RST);
#endif

// --- Callback de GPIO y retardo para VARIANTE_LENTA ------------------------
#if VARIANTE_LENTA
// Se declara extern "C" porque se va a guardar en un puntero a funcion de una
// biblioteca escrita en C: el tipo tiene que coincidir exactamente.
extern "C" uint8_t gpioYRetardoLento(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int,
                                     void *arg_ptr) {
  // Los mensajes de retardo cortos son los que U8g2 usa entre flanco y flanco del
  // reloj serie. Se atienden aqui y no se pasan al original, que los despacharia
  // con un retardo insignificante.
  if (msg == U8X8_MSG_DELAY_NANO || msg == U8X8_MSG_DELAY_100NANO) {
    delayMicroseconds(RETARDO_US);
    return 1;
  }

  // Todo lo demas -mover pines, retardos largos, reset- lo resuelve el callback
  // normal de Arduino. No se reimplementa nada: solo se le anade tiempo.
  uint8_t r = u8x8_gpio_and_delay_arduino(u8x8, msg, arg_int, arg_ptr);

  // Los mensajes de GPIO son los numerados de 64 en adelante. Tras CADA cambio de
  // nivel se espera, que es lo que de verdad ensancha el pulso y da margen a la
  // entrada del ST7920 para cruzar su umbral.
  if (msg >= U8X8_MSG_GPIO(0)) delayMicroseconds(RETARDO_US);
  return r;
}
#endif

// ---------------------------------------------------------------------------
// TESTIGO DE VIDA
// ---------------------------------------------------------------------------
// Es la mitad del valor de este firmware. Sin el, "la pantalla no pinta" y "la
// tarjeta no arranca" son indistinguibles desde fuera.
static void parpadear(uint8_t veces, uint16_t msEncendido, uint16_t msApagado) {
  for (uint8_t i = 0; i < veces; i++) {
    digitalWrite(TESTIGO, HIGH);
    delay(msEncendido);
    digitalWrite(TESTIGO, LOW);
    delay(msApagado);
  }
}

// Espera con el testigo latiendo. Se usa mientras la pantalla mantiene una fase.
//
// No es adorno: si el testigo SE PARA en mitad de una fase, el micro se ha colgado
// dentro de esa operacion de pantalla. Un testigo que solo parpadeara al arrancar
// no distinguiria "se colgo enviando" de "envio bien y no se ve".
static void esperarConLatido(uint16_t ms) {
  uint16_t t = 0;
  while (t < ms) {
    digitalWrite(TESTIGO, HIGH);
    delay(120);
    digitalWrite(TESTIGO, LOW);
    delay(380);
    t += 500;
  }
}

// FIRMA DE ARRANQUE.
//
// Deletrea con parpadeos DOS cosas: que este es el firmware nuevo y que variantes
// lleva compiladas. La primera evita el error clasico de diagnosticar durante
// media hora una tarjeta que en realidad conserva el firmware anterior porque la
// carga fallo y nadie leyo la salida del programador. La segunda evita la
// confusion contraria: creer que se esta probando la variante lenta cuando se
// recompilo sin cambiarla.
//
//   ráfaga rapida de 10   "arranque de DIAGNOSTICO_LCD, ahora mismo"
//   grupo 1 (1..3)        PSB:  1=BAJO   2=ALTO   3=SUELTO
//   grupo 2 (1..2)        RST:  1=controlado   2=sin controlar
//   grupo 3 (1..2)        SPI:  1=normal       2=lento
static void firmaDeArranque() {
  parpadear(10, 60, 60);
  delay(900);

  parpadear(VARIANTE_PSB, 250, 250);
  delay(900);

#if VARIANTE_SIN_RST
  parpadear(2, 250, 250);
#else
  parpadear(1, 250, 250);
#endif
  delay(900);

#if VARIANTE_LENTA
  parpadear(2, 250, 250);
#else
  parpadear(1, 250, 250);
#endif
  delay(900);
}

// ---------------------------------------------------------------------------
void setup() {
  pinMode(TESTIGO, OUTPUT);
  digitalWrite(TESTIGO, LOW);

  // La firma va ANTES de tocar la pantalla, a proposito. Si la inicializacion de
  // la LCD colgara el micro, la firma ya se habria visto y se sabria que el
  // arranque llego hasta aqui. Al reves no se sabria nada.
  firmaDeArranque();

  // PSB segun la variante. Ver el comentario largo de arriba.
#if VARIANTE_PSB == PSB_SUELTO
  // ENTRADA sin resistencia interna: el pin queda en alta impedancia y no impone
  // ningun nivel. Manda lo que el modulo tenga puenteado.
  pinMode(LCD_PSB, INPUT);
#elif VARIANTE_PSB == PSB_ALTO
  pinMode(LCD_PSB, OUTPUT);
  digitalWrite(LCD_PSB, HIGH);   // modo PARALELO: control negativo a proposito
#else
  pinMode(LCD_PSB, OUTPUT);
  digitalWrite(LCD_PSB, LOW);    // modo SERIE, igual que el firmware del Esclavo
#endif

  // Margen tras alimentar antes de hablarle al controlador. El instante de
  // inicializacion ya se descarto como causa (se probo en primera y en ultima
  // posicion y fallaba igual), pero se deja porque no cuesta nada y quita una
  // pregunta de en medio.
  delay(200);

#if VARIANTE_LENTA
  // Se sustituye el callback ANTES de begin(): asi hasta la secuencia de reset y
  // los comandos de inicializacion salen despacio. Si solo se frenara el envio de
  // datos, una inicializacion perdida por flancos sucios seguiria perdiendose.
  u8g2.getU8x8()->gpio_and_delay_cb = gpioYRetardoLento;
#endif

  u8g2.begin();

  // Dos parpadeos LARGOS = "u8g2.begin() retorno".
  //
  // Marca util por si sola: begin() hace la secuencia de reset y manda la
  // inicializacion. Si el testigo se quedara quieto justo aqui -firma completa y
  // luego nada-, el micro se cuelga DENTRO de begin(), lo que apunta a la linea de
  // RST o al pin de reset del modulo, no al dibujo.
  parpadear(2, 700, 300);
}

// ---------------------------------------------------------------------------
// PATRONES
// ---------------------------------------------------------------------------
// Ordenados de MAS visible a MENOS visible. Lo que se ve y lo que no separa las
// causas mejor que cualquier patron aislado.

// Fase 1: pantalla ENTERA en negro. El patron mas visible que existe. Se ve
// aunque el contraste este casi al minimo y aunque el angulo sea malo.
static void faseNegro() {
  u8g2.clearBuffer();
  u8g2.drawBox(0, 0, 128, 64);
  u8g2.sendBuffer();
}

// Fase 2: pantalla ENTERA en blanco. Junto con la anterior demuestra que el
// contenido CAMBIA. Una pantalla que se quedara fija en negro por un fallo de
// alimentacion del panel no alternaria.
static void faseBlanco() {
  u8g2.clearBuffer();
  u8g2.sendBuffer();
}

// Fase 3: damero de 8x8 px. Ejercita la escritura por toda la RAM del display en
// lugar de un relleno uniforme.
//
// Por que importa: la ST7920 direcciona en palabras de 16 bits y con paginas. Un
// relleno total puede verse bien aunque las direcciones esten mal, porque todo
// vale lo mismo. El damero no: si sale corrido, torcido o repetido, el dato llega
// pero el DIRECCIONAMIENTO esta mal, que es una averia distinta.
static void faseDamero() {
  u8g2.clearBuffer();
  for (int y = 0; y < 64; y += 8)
    for (int x = 0; x < 128; x += 8)
      if (((x / 8) + (y / 8)) % 2 == 0) u8g2.drawBox(x, y, 8, 8);
  u8g2.sendBuffer();
}

// Fase 4: solo la MITAD SUPERIOR en negro.
//
// La ST7920 de 128x64 organiza la memoria en dos mitades. Si se ve la de arriba y
// la de abajo no -o al reves, o duplicada-, el fallo esta en como se recorre la
// memoria y no en la comunicacion.
static void faseMitad() {
  u8g2.clearBuffer();
  u8g2.drawBox(0, 0, 128, 32);
  u8g2.sendBuffer();
}

// Fase 5: texto grande.
//
// Es el CONTROL DE CONTRASTE. Si las fases de relleno se ven y esta no, la
// pantalla funciona y lo que falla es el ajuste de contraste del modulo (el
// potenciometro de la placa o la tension de V0). Ese caso explicaria N-22 entero:
// lcd_dibujarBienvenida() solo pinta texto fino, y con el contraste bajo un texto
// fino es invisible mientras un relleno todavia se distingue.
static void faseTexto() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB14_tr);
  u8g2.drawStr(4, 30, "TEXTO OK");
  u8g2.setFont(u8g2_font_6x10_tr);
  u8g2.drawStr(4, 50, "Si lee esto, pinta");
  u8g2.sendBuffer();
}

// ---------------------------------------------------------------------------
void loop() {
  // Antes de cada fase, el testigo dice EN QUE FASE ESTA con parpadeos cortos.
  // Sirve para relacionar lo que se ve en la pantalla con lo que el micro cree que
  // esta pintando, sin depender de contar segundos.
  parpadear(1, 80, 200); faseNegro();  esperarConLatido(3000);
  parpadear(2, 80, 200); faseBlanco(); esperarConLatido(3000);
  parpadear(3, 80, 200); faseDamero(); esperarConLatido(3000);
  parpadear(4, 80, 200); faseMitad();  esperarConLatido(3000);
  parpadear(5, 80, 200); faseTexto();  esperarConLatido(3000);
}
