// ===== Validacion_Respaldo/arnes_respaldo.cpp =====
//
// N-29 — ARNES DE respaldo.cpp EN EL PC.
//
// QUE PROBLEMA CIERRA
// -------------------
// validador_maestro.py mantenia su propia reimplementacion en Python de
// calcularSuma(). En un solo dia esa copia divergio DOS veces del C++:
//
//   1. El firmware paso de suma llana a pesos ponderados. El modelo Python no,
//      y el banco siguio diciendo PASS mientras validaba un algoritmo muerto.
//   2. El firmware paso de pesos ponderados a hash de Horner. El modelo se
//      actualizo a medias -PESOS_SUMA quedo como {reg: 1}- y la prueba 2.7
//      empezo a declarar CIEGOS los diez pares de registros porque "todos tienen
//      el mismo peso", sobre un algoritmo que ya no usa pesos y que NO tiene
//      ningun par ciego.
//
// Un FALLA falso cuesta lo mismo que un PASS falso: el siguiente que lea el
// informe no sabe cual de los dos esta mirando.
//
// El patron correcto ya esta resuelto en Validacion_LCD: no se modela el
// firmware, SE COMPILA EL FIRMWARE. Este arnes hace lo mismo con respaldo.cpp.
//
// COMO
// ----
// calcularSuma() es 'static': no se puede enlazar desde fuera. En vez de tocar el
// firmware para exportarla -que seria cambiar el codigo de vuelo para que quepa
// el banco de pruebas, exactamente al reves de lo que hay que hacer- se INCLUYE
// respaldo.cpp en esta unidad de traduccion. Entonces todo lo 'static' queda
// visible aqui dentro y el fuente sigue intacto, byte por byte, incluida la linea
// que incluye <stm32f1xx_hal.h>: la resuelve el sustituto de este directorio.
//
// LO QUE ESTE ARNES NO ES
// -----------------------
// No es un segundo modelo. Si mañana alguien cambia la semilla, el
// multiplicador, el orden de los registros o el plegado final, este binario
// cambia con el fuente en la siguiente compilacion y el validador mide lo nuevo
// sin que nadie tenga que acordarse de nada. Esa es toda la idea.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Instancia real de los registros de respaldo simulados. La declara extern el
// sustituto de <stm32f1xx_hal.h>; tiene que existir en algun sitio y es aqui.
#include "stm32f1xx_hal.h"
BKP_Simulado arnes_bkp;

// EL FUENTE REAL, SIN TOCAR. Los -I del script de compilacion hacen que
// <stm32f1xx_hal.h> y <Arduino.h> se resuelvan contra los sustitutos de este
// directorio y respaldo.h contra el del Maestro.
#include "respaldo.cpp"   // NOLINT: deliberado, ver cabecera

// Los cinco registros CON DATO, en el mismo orden en que los enumera respaldo.cpp.
// El indice del array no es el numero de registro: se traduce por esta tabla para
// que un cambio de reparto en el firmware no obligue a renumerar nada aqui.
static const uint8_t REGS_CON_DATO[5] = {
  REG_VERDE, REG_DESPEJE, REG_FLAGS, REG_SYNC_ALTA, REG_SYNC_BAJA
};

static void cargarRegistros(const unsigned long *v) {
  for (int i = 0; i < 5; i++) {
    escribirReg(REGS_CON_DATO[i], (uint16_t)(v[i] & 0xFFFFUL));
  }
}

// ---------------------------------------------------------------------------
// PROTOCOLO POR stdin/stdout
// ---------------------------------------------------------------------------
// Una orden por linea, una respuesta por linea, y se sale con EOF. Es de
// proposito el protocolo mas aburrido posible: el validador va a lanzarle
// decenas de miles de consultas -los 80 volteos de un bit, las transposiciones,
// el barrido de dominios alcanzables- y lo unico que tiene que hacer es no
// perder ninguna ni inventarse una.
//
//   SUMA <verde> <despeje> <flags> <sync_dia> <sync_seg>
//       -> el valor de calcularSuma() sobre ese contenido, en hexadecimal de 8
//          cifras -N-51: son los 32 bits CRUDOS, ya no un plegado a 16-. Es LA
//          FUNCION DEL FIRMWARE, no una reproduccion suya.
//
//   HORAS <verde> <despeje> <flags> <sync_alta> <sync_baja> <ignorado> <rtc_ahora>
//       N-49: sync_alta/sync_baja son las dos mitades del contador del RTC guardado,
//       y rtc_ahora el contador actual. El sexto queda como relleno para no cambiar
//       el numero de campos que el validador ya escribe.
//       -> respaldo_horasDesdeSync() con ese contenido en la pila. Se ofrece
//          porque es la OTRA funcion pura de este fichero que el validador
//          modela a mano, y modelarla a mano tiene el mismo problema. Sale en
//          decimal, o la palabra CADUCADA para el centinela.
//
//   VALIDO <verde> <despeje> <flags> <sync_alta> <sync_baja> <firma> <suma_alta> <suma_baja>
//       N-51: la suma ya no cabe en un campo -son los 32 bits crudos en dos
//       registros-, asi que este comando gano un octavo campo.
//       -> 1 o 0 segun respaldo_setup() acepte ese dominio de respaldo. Cubre la
//          terna firma+suma_alta+suma_baja de una sola vez, que es como la lee
//          el arranque.
//
//   PING -> PONG. Sirve para que el validador compruebe que tiene delante un
//           binario vivo antes de fiarse de un solo numero suyo.
//
// Cualquier otra cosa produce la linea "ERROR" y NO detiene el proceso: un fallo
// de encuadre en una consulta no puede hacer que las otras cuarenta mil se
// pierdan en silencio.
int main(void) {
  char linea[256];

  // Sin buferado de linea la tuberia se atasca en cuanto el validador espera una
  // respuesta antes de mandar la siguiente consulta.
  setvbuf(stdout, NULL, _IOLBF, 0);

  while (fgets(linea, sizeof(linea), stdin)) {
    unsigned long v[8] = {0, 0, 0, 0, 0, 0, 0, 0};

    // N-43: PowerShell 5.1 antepone un BOM UTF-8 (EF BB BF) al mandar texto a un
    // proceso nativo -medido con od: 'ef bb bf 50 49 4e 47 0d 0a'-, mientras que
    // pwsh 7 y bash mandan la linea limpia. Con esos tres bytes delante ningun
    // strncmp casa, asi que el arnes contestaba ERROR al PING y parecia un binario
    // muerto. Se descarta aqui para que el arnes no dependa de quien lo invoque.
    if ((unsigned char)linea[0] == 0xEF && (unsigned char)linea[1] == 0xBB &&
        (unsigned char)linea[2] == 0xBF) {
      memmove(linea, linea + 3, strlen(linea + 3) + 1);
    }

    if (strncmp(linea, "PING", 4) == 0) {
      printf("PONG\n");

    } else if (strncmp(linea, "SUMA", 4) == 0) {
      if (sscanf(linea + 4, "%lu %lu %lu %lu %lu",
                 &v[0], &v[1], &v[2], &v[3], &v[4]) != 5) {
        printf("ERROR\n");
        continue;
      }
      cargarRegistros(v);
      printf("%08X\n", (unsigned)calcularSuma());

    } else if (strncmp(linea, "HORAS", 5) == 0) {
      if (sscanf(linea + 5, "%lu %lu %lu %lu %lu %lu %lu",
                 &v[0], &v[1], &v[2], &v[3], &v[4], &v[5], &v[6]) != 7) {
        printf("ERROR\n");
        continue;
      }
      cargarRegistros(v);
      // El fechado solo mira contenido que respaldo_setup() haya dado por bueno,
      // asi que se sella y se marca valido igual que hace el firmware al escribir.
      escribirReg(REG_FIRMA, FIRMA);
      sellar();
      // N-49: v[5] y v[6] son las dos mitades del contador guardado; v[6] pasa a ser
      // el contador de AHORA. Ya no son dia del mes ni segundo del dia.
      uint32_t h = respaldo_horasDesdeSync((uint32_t)v[6]);
      if (h == RESPALDO_SYNC_CADUCADA) printf("CADUCADA\n");
      else                             printf("%lu\n", (unsigned long)h);

    } else if (strncmp(linea, "VALIDO", 6) == 0) {
      if (sscanf(linea + 6, "%lu %lu %lu %lu %lu %lu %lu %lu",
                 &v[0], &v[1], &v[2], &v[3], &v[4], &v[5], &v[6], &v[7]) != 8) {
        printf("ERROR\n");
        continue;
      }
      cargarRegistros(v);
      escribirReg(REG_FIRMA, (uint16_t)v[5]);
      // N-51: la suma son los 32 bits crudos en dos registros, no un solo campo.
      escribirReg(REG_SUMA_ALTA, (uint16_t)v[6]);
      escribirReg(REG_SUMA_BAJA, (uint16_t)v[7]);
      // respaldo_setup() borra el dominio cuando lo declara invalido, asi que se
      // consulta el veredicto ANTES de que eso ocurra... y como respaldo_setup()
      // hace las dos cosas, se replica aqui solo la comprobacion, que es lo que
      // se quiere medir. Es la MISMA expresion del fuente y esta dos lineas mas
      // arriba en el mismo binario, no una copia en otro lenguaje.
      const uint32_t sumaGuardada = ((uint32_t)leerReg(REG_SUMA_ALTA) << 16) |
                                    (uint32_t)leerReg(REG_SUMA_BAJA);
      bool ok = (leerReg(REG_FIRMA) == FIRMA) && (sumaGuardada == calcularSuma());
      printf("%d\n", ok ? 1 : 0);

    } else if (linea[0] == '\n' || linea[0] == '\r' || linea[0] == '\0') {
      continue;

    } else {
      printf("ERROR\n");
    }
  }
  return 0;
}
