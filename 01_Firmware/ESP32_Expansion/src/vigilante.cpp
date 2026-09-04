// ===== 01_Firmware/ESP32_Expansion/src/vigilante.cpp =====

#include "vigilante.h"
#include "contrato.h"
#include "puente.h"
#include "transporte_app.h"
#include "enlace_stm32.h"
#include <esp_task_wdt.h>
#include <esp_system.h>
#include <stdio.h>

static bool armado = false;

void vigilante_armar() {
  // El techo del task watchdog del IDF se programa en SEGUNDOS enteros:
  //   esp_err_t esp_task_wdt_init(uint32_t timeout, bool panic);
  // Por eso ESP32_WDT_MS tiene que ser multiplo de 1000, y hay un pack que lo exige:
  // un 2500 aqui se convertiria en 2 s dentro del chip mientras la desigualdad del
  // banco seguiria comprobando 2,5 s. El banco estaria midiendo un equipo que no es.
  const uint32_t segundos = (uint32_t)(ESP32_WDT_MS / 1000UL);

  // panic = true a proposito: se quiere el REINICIO, no un aviso por consola. Un puente
  // colgado que solo imprime queda igual de colgado, y en esta arquitectura eso deja al
  // operario sin ninguna forma de mandar sobre el equipo.
  esp_err_t r = esp_task_wdt_init(segundos, true);

  // El framework de Arduino puede haber inicializado ya el TWDT por su cuenta; en ese
  // caso init devuelve ESP_ERR_INVALID_STATE y NO es un fallo: el perro existe. Lo que
  // no se puede es dar por armado un caso que no se ha distinguido.
  if (r != ESP_OK && r != ESP_ERR_INVALID_STATE) {
    armado = false;
    return;
  }

  // W-1: se registra LA TAREA QUE BOMBEA BYTES, que es esta -el loopTask de Arduino,
  // donde corre loop() y por tanto puente_bombear()-. NULL significa "la tarea actual".
  //
  // Registrar otra -una de servicio, una de telemetria- seria vigilar a un testigo que
  // no se cuelga nunca: el puente se quedaria mudo y el perro seguiria contento. Es el
  // mismo error de forma que un pinMode() sin digitalRead().
  armado = (esp_task_wdt_add(NULL) == ESP_OK);
}

void vigilante_alimentar() {
  // Si el registro fallo no se llama a reset: devolveria ESP_ERR_NOT_FOUND y, sobre
  // todo, disimularia. Que armado() siga en false es lo unico que permite verlo.
  if (!armado) return;
  esp_task_wdt_reset();
}

bool vigilante_armado() {
  return armado;
}

// ===========================================================================
// EL PARTE DE ARRANQUE
// ===========================================================================
//
// LAS DOS VARIABLES QUE TIENEN QUE SOBREVIVIR AL REINICIO, Y POR QUE ESTAN EN ESA
// SECCION Y NO EN OTRA. Medido en el header del IDF que trae este framework
// (tools/sdk/esp32/include/esp_common/include/esp_attr.h:77 y :102): RTC_DATA_ATTR solo
// promete el ciclo de sueno profundo -"during a deep sleep / wake cycle"-, mientras que
// RTC_NOINIT_ATTR promete literalmente "after restart". Un reinicio por watchdog es un
// restart, no un despertar, asi que la unica de las dos que sirve aqui es la segunda.
//
// Y LA MARCA NO ES PARANOIA: ".rtc_noinit" no se inicializa NUNCA -ese es el sentido
// del nombre-, tampoco en la primera subida de tension de un modulo virgen. Sin ella,
// la cuenta arrancaria valiendo lo que hubiera en esa RAM. Un contador que puede
// empezar en un numero cualquiera no mide: decora.
//
// El valor de la marca es arbitrario; lo unico que se le pide es que sea improbable
// como basura.
#define VIGILANTE_MARCA_RTC   0x5EA1F0C0UL

RTC_NOINIT_ATTR static uint32_t marcaRtc;
RTC_NOINIT_ATTR static uint32_t arranques;

static esp_reset_reason_t causa = ESP_RST_UNKNOWN;

// true mientras el parte de ESTA conexion no haya salido. No vive en memoria RTC a
// proposito: es un asunto de la sesion actual, no del historial del modulo.
static bool parteEmitido = false;

// EL FORMATO, EN UN SOLO LITERAL Y CON NOMBRE.
//
// No se parte en literales adyacentes -que es lo que pide la anchura de linea- porque
// el pack esp32_10 lo lee de aqui para recalcular la desigualdad del buffer, y un
// instrumento que tenga que RECONSTRUIR el formato pegando trozos puede reconstruirlo
// mal y seguir dando verde. Es N-89 por el otro lado: alli el compositor escondia los
// literales del pack, aqui se evita darle uno que solo entiende a medias.
static const char FORMATO_PARTE[] =
  "$EVENT,NODE:PUENTE,EVT:ARRANQUE,CAUSA:%s,ARRANQUES:%lu,PERRO:%s,WDT_MS:%lu";

// EL NOMBRE DE CADA CAUSA, UNA RAMA POR VALOR DEL ENUM DEL IDF.
//
// Son los once de esp_reset_reason_t (esp_system.h:41-52 del mismo header medido
// arriba) mas el default. El default NO es relleno: es la rama que impide que un valor
// que Espressif anada manana llegue al operario como un hueco. Es la misma decision
// que MOTIVO_NO_CONTEMPLADO del despachador -un caso nuevo no se aprueba a si mismo-.
//
// Ningun nombre lleva coma, asterisco ni '$': la coma separa campos de la trama y el
// asterisco abre el checksum, asi que una causa con cualquiera de los dos partiria la
// trama por dentro sin que nada lo delatara. El pack lo exige.
static const char* nombreCausa(esp_reset_reason_t c) {
  switch (c) {
    case ESP_RST_POWERON:   return "SUBIDA_DE_TENSION";
    case ESP_RST_EXT:       return "PIN_EXTERNO";
    case ESP_RST_SW:        return "REINICIO_POR_SOFTWARE";
    case ESP_RST_PANIC:     return "EXCEPCION_O_PANICO";
    case ESP_RST_INT_WDT:   return "PERRO_DE_INTERRUPCION";
    case ESP_RST_TASK_WDT:  return "PERRO_DE_TAREAS";
    case ESP_RST_WDT:       return "OTRO_PERRO";
    case ESP_RST_DEEPSLEEP: return "SUENO_PROFUNDO";
    case ESP_RST_BROWNOUT:  return "TENSION_BAJA";
    case ESP_RST_SDIO:      return "SDIO";
    case ESP_RST_UNKNOWN:   return "DESCONOCIDA";
    default:                return "NO_CONTEMPLADA";
  }
}

void vigilante_censarArranque() {
  // DEL CHIP. esp_reset_reason() devuelve lo que el hardware apunto en el dominio RTC
  // antes de que este firmware existiera en esta arrancada. Deducirla de una bandera
  // propia solo acertaria en los reinicios que el firmware ve venir, que son justo los
  // que no hay que contar.
  causa = esp_reset_reason();

  // POR QUE LA SUBIDA DE TENSION PONE LA CUENTA A CERO AUNQUE LA MARCA SIGA VALIDA.
  //
  // La RAM del dominio RTC puede conservar su contenido a traves de un corte corto y
  // perderlo en uno largo. Sin esta linea, la cuenta significaria "desde la ultima
  // subida de tension" unas veces y "desde vaya usted a saber" otras, segun lo que
  // durara el corte. Una variable que contesta a dos preguntas distintas no puede
  // contestar bien a ninguna -es cfgVerdeRecibido otra vez-, asi que se le fija UN
  // significado: arranques desde la ultima subida de tension, siempre.
  if (marcaRtc != VIGILANTE_MARCA_RTC || causa == ESP_RST_POWERON) {
    marcaRtc = VIGILANTE_MARCA_RTC;
    arranques = 0;
  }
  arranques++;
}

size_t vigilante_parteDeArranque(char* destino, size_t capacidad) {
  if (destino == NULL || capacidad == 0) return 0;

  int n = snprintf(destino, capacidad, FORMATO_PARTE,
                   nombreCausa(causa),
                   (unsigned long)arranques,
                   vigilante_armado() ? "ARMADO" : "SIN_ARMAR",
                   (unsigned long)ESP32_WDT_MS);

  // snprintf trunca en silencio y devuelve lo que HABRIA escrito. Un parte truncado
  // saldria por trama_componer() con un checksum perfectamente calculado sobre el
  // trozo: la app lo daria por bueno y ensenaria una causa a medias. Se prefiere no
  // decir nada -y que el pack impida que este caso pueda ocurrir- antes que decir algo
  // que parece medido y esta cortado.
  if (n < 0 || (size_t)n >= capacidad) {
    destino[0] = '\0';
    return 0;
  }
  return (size_t)n;
}

void vigilante_declarar() {
  // SIN NADIE ESCUCHANDO NO SE DECLARA, Y SE REARMA.
  //
  // transporte_escribir() devuelve 0 sin telefono conectado (transporte_app.cpp:59):
  // un parte emitido en el arranque se perderia entero, y como solo se compone una vez
  // el operario no volveria a verlo nunca. Rearmarlo al caer el enlace hace ademas que
  // reconectar sirva para volver a preguntarlo.
  if (!transporte_conectado()) {
    parteEmitido = false;
    return;
  }
  if (parteEmitido) return;

  char parte[VIGILANTE_PARTE_MAX];
  size_t n = vigilante_parteDeArranque(parte, sizeof(parte));

  // Se marca como intentado quepa o no. Reintentarlo en cada vuelta convertiria un
  // parte que no cabe en un bucle que recompone lo mismo para siempre, y eso no lo
  // arregla: lo esconde detras de trabajo. Que no quepa es lo que el pack esp32_10
  // impide recalculando la desigualdad en cada corrida.
  parteEmitido = true;
  if (n == 0) return;

  // HACIA LA APP Y SOLO HACIA LA APP (B-3). El parte habla del puente, no del equipo, y
  // va marcado con NODE:PUENTE porque un $EVENT del accesorio que pareciera del STM32
  // mandaria a diagnosticar el poste equivocado.
  puente_emitirPropio(parte);
}

// ===========================================================================
// EL LATIDO HACIA EL STM32 (AB-1)
// ===========================================================================
//
// POR QUE VIVE AQUI Y NO EN puente.cpp NI EN enlace_stm32.cpp.
//
// No es comodidad: es que esos dos ficheros son EL CAMINO DE DATOS, y el pack
// esp32_07 exige por P-1/P-4 que no tengan reloj -"sin millis() no hay forma de
// emitir periodicamente ni de agrupar telemetria"-. Un reloj alli abriria las dos
// cosas que esa regla cierra. Aqui no hay datos de nadie pasando: este fichero ya
// era el que emite lo propio del puente -el parte de arranque-, asi que el latido
// entra por la puerta que ya estaba abierta y sancionada.
//
// 🔴 Y ESO DEJA UN HUECO QUE HAY QUE DECIR: P-1/P-4 censan `millis()` SOLO en esos
// dos ficheros, asi que este latido les es INVISIBLE. Siguen en verde y siguen
// siendo literalmente ciertas, pero ya no cubren "el puente no emite
// periodicamente" -porque ahora si emite, aqui-. Lo que las mantiene honestas es
// que su enunciado habla del CAMINO DE DATOS, no del puente entero. El pack se
// amplia con una comprobacion propia del latido; sin ella, esta funcion podria
// desaparecer manana y ningun instrumento lo notaria.
//
// 6.4 SIGUE EN PIE, Y ESTA ES LA PARTE QUE HAY QUE MIRAR DESPACIO. La regla dice que
// el puente no origina ORDENES: "un saludo del puente seria una orden que nadie
// pidio entrando por el mismo camino que las que si se piden". El latido entra por
// ese mismo camino, si — pero NO es una orden y no puede llegar a serlo:
//
//   - el STM32 lo reconoce ANTES de la guarda de PIN y devuelve sin actuar;
//   - no ejecuta nada, no mueve una luz, no cambia un modo;
//   - y no CONTESTA, que es lo que lo separa de todo lo demas que entra por J17.
//
// Lo unico que produce es que j17RegistrarLinea() cierre un silencio. O sea: su
// unico efecto es que el equipo pueda contar que el puente esta vivo. Eso es
// exactamente lo que 6.4 protege -que nadie mande sin pedirlo- visto por el otro
// lado: aqui no se manda, se respira.
//
// NO SE EMITE EN setup(). Igual que el parte de arranque, y por la misma razon: en
// setup() no se pone un byte en ningun cable. El primer latido sale en el primer
// loop() que cumpla el plazo.
static unsigned long tUltimoLatido = 0;

void vigilante_latir() {
  const unsigned long ahora = millis();

  // La resta sin signo mide bien UNA vuelta de millis() -49,7 dias-. Un latido
  // perdido en la vuelta no significa nada: el siguiente llega 2 s despues.
  if (ahora - tUltimoLatido < LATIDO_MS) return;
  tUltimoLatido = ahora;

  // Se manda con su terminador porque el STM32 trocea por CR o LF: sin el, la linea
  // no se cierra nunca y el latido se quedaria pegado a lo siguiente que pase.
  static const char LINEA[] = LATIDO_LINEA "\r\n";
  enlace_escribirLinea(LINEA, sizeof(LINEA) - 1);
}
