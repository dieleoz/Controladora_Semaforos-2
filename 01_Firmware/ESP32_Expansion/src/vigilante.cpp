// ===== 01_Firmware/ESP32_Expansion/src/vigilante.cpp =====

#include "vigilante.h"
#include "contrato.h"
#include <esp_task_wdt.h>

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
