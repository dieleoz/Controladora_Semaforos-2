// ===== src/respaldo.cpp =====
#include "respaldo.h"

// Registros de respaldo del STM32F1. Son BKP->DR1..DR10, de 16 bits utiles cada
// uno, en el dominio alimentado por VBAT. La libreria STM32duino RTC 1.9.0 NO los
// expone, asi que se accede por registro, que en esta familia es directo.
#include <stm32f1xx_hal.h>

// Reparto. Se documenta aqui y no en cada funcion para que se vea de un vistazo
// que no hay solapes:
//
//   DR1  firma
//   DR2  verde del ciclo degradado, en segundos
//   DR3  despeje del ciclo degradado, en segundos (YA AMPLIADO)
//   DR4  indicadores: bit0 hay ciclo, bit1 hubo sync, bit2 degradado activo
//   DR5  ultima sync: 16 bits ALTOS del contador del RTC
//   DR6  ultima sync: 16 bits BAJOS del contador del RTC
//   DR7  suma de comprobacion de DR2..DR6: 16 bits BAJOS del hash de Horner crudo
//   DR8  suma de comprobacion de DR2..DR6: 16 bits ALTOS del hash de Horner crudo
//
// N-51 — DR7 GUARDABA el hash de Horner PLEGADO de 32 a 16 bits (XOR de las dos
// mitades), y el pliegue tira la mitad de la mezcla: el banco midio 8 pares de
// registros cuya transposicion deja el checksum plegado intacto, uno de ellos
// explotable (FLAGS/SYNC_BAJA puede terminar pareciendo "Degradado activo con
// sincronizacion reciente"). Guardar los 32 bits crudos sin plegar -en DR7 y el
// DR8 que estaba libre- cierra el barrido a 0 pares ciegos sobre el mismo dominio
// alcanzable (medido en el pack maestro_02_respaldo, no supuesto).
//
// N-49 — DR5 y DR6 GUARDABAN dia-del-mes y segundo-del-dia, y esa pareja no podia
// fechar nada. El calendario esta anclado a enero a proposito (ver reloj_fijarEnero),
// asi que el dia va 1..31 y VUELVE a 1. De ahi salian dos agujeros opuestos:
//
//   - la vuelta 31->1 daba una resta NEGATIVA y se declaraba CADUCADA. El Maestro se
//     rendia a ambar y el Esclavo -que cuenta con millis()- seguia dando VERDE. Las
//     dos puntas rindiendose en instantes distintos, 24 dias al ano.
//   - y al reves: "hoy" y "hace 31 dias" producen EL MISMO par de numeros. Una
//     sincronizacion de hace un mes se leia como reciente.
//
// Ninguna aritmetica sobre el dia del mes arregla eso, porque el dato guardado no
// contiene la informacion. Lo que se guarda ahora es el CONTADOR DEL RTC: 32 bits de
// segundos que mantiene la pila, monotonos, sin vuelta en 136 anos. Comparar pasa a
// ser una resta de dos enteros, y los dos agujeros se cierran a la vez.
//
// Se indexan por numero de registro (1..10), no por desplazamiento. El HAL de esta
// familia declara los DRn como uint32_t consecutivos aunque solo 16 bits sean
// utiles, asi que se recorren desde &BKP->DR1 de uno en uno. Solo se guardan
// valores de 16 bits: los superiores se leen como cero y escribirlos no hace nada.
static const uint8_t REG_FIRMA     = 1;
static const uint8_t REG_VERDE     = 2;
static const uint8_t REG_DESPEJE   = 3;
static const uint8_t REG_FLAGS     = 4;
static const uint8_t REG_SYNC_ALTA = 5;
static const uint8_t REG_SYNC_BAJA = 6;
static const uint8_t REG_SUMA_BAJA = 7;
static const uint8_t REG_SUMA_ALTA = 8;   // N-51: DR8 estaba libre, ver reparto arriba

// N-133 (04/09): LOS TIEMPOS DEL CICLO AUTOMATICO, QUE NO SE GUARDABAN EN NINGUN SITIO.
//
// Lo destapo una pregunta del responsable: "una cosa es parametrizar al inicio, luego
// deberia funcionar". No se cumplia. REG_VERDE y REG_DESPEJE de arriba son del MODO
// DEGRADADO -y estan en segundos-, no del ciclo automatico; los del automatico vivian
// SOLO en RAM, asi que un corte de luz -o entrar al modo- devolvia el cruce a los
// minimos sin avisar, despues de haber contestado $ACK a un SET_TIEMPOS.
//
// Caben en los dos DR que quedaban libres. Los tres valores son de un byte -verde y
// rojo van 3..15 minutos, el despeje 10..90 segundos-, asi que rojo y verde comparten
// registro y el despeje se queda con el suyo.
static const uint8_t REG_CICLO_RV      = 9;   // rojo en el byte alto, verde en el bajo
static const uint8_t REG_CICLO_DESPEJE = 10;

// N-49/N-51: LA FIRMA CAMBIA CON EL FORMATO, y no es opcional. Un equipo
// actualizado que encontrara una firma vieja daria por bueno un contenido escrito
// con otra aritmetica -otro par dia/segundo en N-49, un checksum plegado a la
// mitad en N-51-. Al no reconocerla, respaldo_setup() borra y el equipo arranca
// SIN sincronizacion previa -el estado seguro-, a costa de resincronizar a mano.
static const uint16_t FIRMA = 0x5EB2;   // 0x5EB1 no tenia los tiempos del ciclo (N-133)
// La firma sube porque el FORMATO cambio -dos registros mas dentro del checksum-. Un
// equipo actualizado que encontrara la firma vieja leeria DR9/DR10 como tiempos cuando
// alli no hay mas que lo que dejo el arranque anterior. Al no reconocerla, borra y
// arranca limpio, que es lo correcto: se pierden la hora y la autorizacion del
// Degradado, y las dos se vuelven a poner. Un ciclo inventado no se puede deshacer.

static const uint16_t FLAG_CICLO     = 0x0001;
static const uint16_t FLAG_SYNC      = 0x0002;
static const uint16_t FLAG_DEGRADADO = 0x0004;

static bool contenidoValido = false;

// --- Acceso crudo ----------------------------------------------------------
// El dominio de respaldo esta protegido contra escritura para que un programa
// descarrilado no pueda corromper lo unico que sobrevive a un reinicio. Hay que
// levantar esa proteccion en cada escritura.

static inline volatile uint32_t *regPtr(uint8_t n) {
  return &(&BKP->DR1)[n - 1];
}

static inline uint16_t leerReg(uint8_t n) {
  return (uint16_t)(*regPtr(n) & 0xFFFFU);
}

static inline void escribirReg(uint8_t n, uint16_t valor) {
  HAL_PWR_EnableBkUpAccess();
  *regPtr(n) = valor;
  HAL_PWR_DisableBkUpAccess();
}

// Suma de comprobacion de los registros con dato. No es criptografia ni pretende
// serlo: sirve para distinguir un contenido escrito por nosotros de basura que
// quedara en el dominio de respaldo tras un arranque sucio o una pila que se agoto
// a medias. Sin esto, medio dato antiguo pasaria por configuracion valida.
// Cada registro se PONDERA POR SU POSICION. Una suma llana era insensible al
// orden: permutar dos registros con valores distintos daba la misma suma y el
// contenido pasaba por integro. Lo detecto el validador el 01/08/2026, que
// comprobo que los 80 volteos de un bit si se cazaban pero las 9 transposiciones
// pasaban todas.
//
// No es paranoia: intercambiar los indicadores con el dia de sincronizacion
// dejaria una configuracion cruzada que el equipo daria por buena. Multiplicar por
// la posicion rompe la simetria y cuesta nada.
// Mezcla multiplicativa en vez de suma ponderada.
//
// Historia, porque la segunda version tambien fallo. Empezo siendo una suma llana,
// insensible al orden: las 9 transposiciones pasaban. Se paso a pesos pequenos
// (1,3,5,7,11) y quedaron CUATRO ciegas, todas con SYNC_SEG, que es el registro
// ancho: la ponderacion solo rompe la simetria si (peso_a - peso_b) por la
// diferencia de valores no es multiplo de 65536, y con pesos pequenos eso pasa.
//
// El caso concreto que lo delato: FLAGS=7 con SYNC_SEG=32775 -una sincronizacion a
// las 18:12:30, hora corriente-. Permutarlos dejaba la suma INTACTA y producia
// FLAGS=32775, con ciclo, sync y degradado los tres encendidos: un arranque tras
// corte lo habria leido como autorizacion vigente y habria reanudado el Modo
// Degradado sobre contenido corrupto.
//
// Un acumulador multiplicativo depende del orden por construccion, no por la
// aritmetica de los pesos, asi que no hay pares de VALOR que se compensen.
//
// N-51 — PERO SI QUEDABAN PARES DE REPRESENTACION. Esta funcion ANTES devolvia
// los 32 bits de Horner plegados a 16 con un XOR ("(s>>16)^s"), y ese pliegue
// descarta informacion: dos contenidos de 32 bits distintos pueden plegar al
// mismo valor de 16, y una transposicion de registros es precisamente una forma
// barata de encontrar uno. El banco midio 8 pares de registros ciegos bajo el
// pliegue -uno explotable: FLAGS/SYNC_BAJA puede acabar pareciendo "Degradado
// activo con sincronizacion reciente"-. La mezcla de Horner en si no tenia el
// problema; el pliegue final si. Se devuelven los 32 bits CRUDOS y el llamante
// los guarda enteros, en dos registros -ver sellar()-.
static uint32_t calcularSuma() {
  uint32_t s = 0x1F35U;   // semilla, para que el contenido todo a cero no de cero
  s = s * 31U + leerReg(REG_VERDE);
  s = s * 31U + leerReg(REG_DESPEJE);
  s = s * 31U + leerReg(REG_FLAGS);
  s = s * 31U + leerReg(REG_SYNC_ALTA);
  s = s * 31U + leerReg(REG_SYNC_BAJA);
  // N-133: los tiempos del ciclo entran en la suma. Dejarlos fuera seria guardarlos
  // sin proteger: un bit volteado en el dominio de respaldo daria un ciclo distinto
  // del que alguien configuro, y el equipo lo daria por bueno.
  s = s * 31U + leerReg(REG_CICLO_RV);
  s = s * 31U + leerReg(REG_CICLO_DESPEJE);
  return s;
}

static void sellar() {
  const uint32_t s = calcularSuma();
  // N-51: los 32 bits enteros, sin plegar -ver el porque en calcularSuma()-.
  escribirReg(REG_SUMA_ALTA, (uint16_t)(s >> 16));
  escribirReg(REG_SUMA_BAJA, (uint16_t)(s & 0xFFFFU));
  contenidoValido = true;
}

void respaldo_setup() {
  // El reloj del dominio de respaldo y el de PWR pueden venir ya encendidos por
  // reloj_setup(), pero habilitarlos dos veces es inofensivo y no depender del
  // orden de inicializacion evita un fallo que solo aparece si alguien reordena
  // las llamadas en main.cpp.
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_RCC_BKP_CLK_ENABLE();

  // N-51: la suma guardada son los 32 bits crudos repartidos en dos registros,
  // no un solo valor plegado -ver calcularSuma()/sellar()-.
  const uint32_t sumaGuardada = ((uint32_t)leerReg(REG_SUMA_ALTA) << 16) |
                                (uint32_t)leerReg(REG_SUMA_BAJA);
  contenidoValido = (leerReg(REG_FIRMA) == FIRMA) && (sumaGuardada == calcularSuma());

  if (!contenidoValido) {
    // Equipo nuevo, pila agotada o contenido corrupto. Se deja limpio en vez de
    // arrastrar restos: un dato a medias es peor que ninguno, porque parece bueno.
    respaldo_borrar();
  }
}

bool respaldo_valido() { return contenidoValido; }

void respaldo_borrar() {
  escribirReg(REG_VERDE, 0);
  escribirReg(REG_DESPEJE, 0);
  escribirReg(REG_FLAGS, 0);
  escribirReg(REG_SYNC_ALTA, 0);
  escribirReg(REG_SYNC_BAJA, 0);
  escribirReg(REG_FIRMA, FIRMA);
  sellar();
}

// --- Ciclo degradado -------------------------------------------------------

void respaldo_guardarCiclo(uint8_t verdeSeg, uint8_t despejeSeg) {
  // Un ciclo con algun tramo a cero no es configuracion, es ausencia de ella. Se
  // rechaza en vez de guardarse, para que respaldo_hayCiclo() no mienta.
  if (verdeSeg == 0 || despejeSeg == 0) return;
  escribirReg(REG_VERDE, verdeSeg);
  escribirReg(REG_DESPEJE, despejeSeg);
  escribirReg(REG_FLAGS, (uint16_t)(leerReg(REG_FLAGS) | FLAG_CICLO));
  escribirReg(REG_FIRMA, FIRMA);
  sellar();
}

uint8_t respaldo_verdeSeg()   { return contenidoValido ? (uint8_t)leerReg(REG_VERDE) : 0; }
uint8_t respaldo_despejeSeg() { return contenidoValido ? (uint8_t)leerReg(REG_DESPEJE) : 0; }

// --- N-133: los tiempos del ciclo AUTOMATICO -------------------------------
//
// NO SE VALIDA EL RANGO AQUI, Y ES DELIBERADO. El respaldo guarda y devuelve; quien
// sabe cuales son los limites viales es modo_automatico.cpp, que es donde viven
// VERDE_MIN_MIN y compania. Copiar los rangos aqui seria una segunda copia que alguien
// tendria que sincronizar -R-9 de contrato.h, tres veces pagado-, y ademas la copia de
// aqui no llevaria encima el comentario que explica por que son 3 minutos.
//
// Lo que SI se hace es negarse a guardar un cero: un tiempo a cero no es configuracion,
// es ausencia de ella, y respaldo_hayTiemposCiclo() mentiria. Mismo criterio que
// respaldo_guardarCiclo() de arriba.
void respaldo_guardarTiemposCiclo(uint8_t rojoMin, uint8_t verdeMin, uint8_t despejeSeg) {
  if (rojoMin == 0 || verdeMin == 0 || despejeSeg == 0) return;
  escribirReg(REG_CICLO_RV, (uint16_t)(((uint16_t)rojoMin << 8) | verdeMin));
  escribirReg(REG_CICLO_DESPEJE, despejeSeg);
  escribirReg(REG_FIRMA, FIRMA);
  sellar();
}

// Devuelve si HABIA algo guardado. Los tres punteros solo se tocan si devuelve true:
// asi el llamante no puede quedarse con medio dato -dos tiempos del respaldo y uno de
// los suyos-, que seria un ciclo que nadie configuro.
bool respaldo_tiemposCiclo(uint8_t* rojoMin, uint8_t* verdeMin, uint8_t* despejeSeg) {
  if (!contenidoValido) return false;
  const uint16_t rv = leerReg(REG_CICLO_RV);
  const uint8_t d = (uint8_t)leerReg(REG_CICLO_DESPEJE);
  const uint8_t r = (uint8_t)(rv >> 8), v = (uint8_t)(rv & 0xFF);
  // Un dominio de respaldo recien borrado da ceros, y el checksum los aprueba porque
  // son el contenido legitimo de un equipo nuevo. Cero no es un tiempo: es "nadie ha
  // configurado esto todavia", y el llamante tiene que poder distinguirlo.
  if (r == 0 || v == 0 || d == 0) return false;
  *rojoMin = r; *verdeMin = v; *despejeSeg = d;
  return true;
}

bool respaldo_hayCiclo() {
  if (!contenidoValido) return false;
  if ((leerReg(REG_FLAGS) & FLAG_CICLO) == 0) return false;
  // El indicador podria estar puesto y el dato ser cero si algo salio mal a medias.
  return leerReg(REG_VERDE) != 0 && leerReg(REG_DESPEJE) != 0;
}

// --- Sincronizacion --------------------------------------------------------

void respaldo_marcarSync(uint32_t segundosRtc) {
  // El cero es el "no hay reloj" que devuelve reloj_contadorSegundos() cuando el RTC
  // no esta operativo. Fechar con el dejaria una marca de sincronizacion apoyada en
  // un contador que nadie hace avanzar, y sobre esa mentira el Degradado se
  // autorizaria. Sin reloj no se marca, igual que sin oscilador no se ajusta la hora.
  if (segundosRtc == 0) return;
  escribirReg(REG_SYNC_ALTA, (uint16_t)(segundosRtc >> 16));
  escribirReg(REG_SYNC_BAJA, (uint16_t)(segundosRtc & 0xFFFFU));
  escribirReg(REG_FLAGS, (uint16_t)(leerReg(REG_FLAGS) | FLAG_SYNC));
  escribirReg(REG_FIRMA, FIRMA);
  sellar();
}

bool respaldo_haySync() {
  return contenidoValido && (leerReg(REG_FLAGS) & FLAG_SYNC) != 0;
}

uint32_t respaldo_horasDesdeSync(uint32_t segundosRtcAhora) {
  if (!respaldo_haySync()) return RESPALDO_SYNC_CADUCADA;

  // Sin reloj no se fecha. Ver la nota del cero en respaldo_marcarSync().
  if (segundosRtcAhora == 0) return RESPALDO_SYNC_CADUCADA;

  const uint32_t guardado = ((uint32_t)leerReg(REG_SYNC_ALTA) << 16) |
                            (uint32_t)leerReg(REG_SYNC_BAJA);

  // N-49: una resta de dos contadores monotonos, sin casos especiales. Ya no hay
  // ventana de "dias aceptables": el contador no vuelve, asi que una diferencia
  // grande significa de verdad que ha pasado mucho, y el limite duro de 48 h lo
  // aplica quien llama. La ventana de 0..2 dias que habia aqui existia solo para
  // tapar la ambiguedad del dia del mes, y tapaba de mas -la vuelta 31->1- y de
  // menos -"hace 31 dias" leido como "hoy"-.
  //
  // ANTES QUE NADA, el retroceso: si el contador de ahora es menor que el guardado,
  // alguien puso el reloj en hora o el dominio de respaldo se reinicio. La marca ya
  // no significa nada y ninguna resta la rescata.
  if (segundosRtcAhora < guardado) return RESPALDO_SYNC_CADUCADA;

  return (segundosRtcAhora - guardado) / 3600UL;
}

// --- Modo Degradado --------------------------------------------------------

void respaldo_guardarDegradado(bool activo) {
  uint16_t f = leerReg(REG_FLAGS);
  f = activo ? (uint16_t)(f | FLAG_DEGRADADO) : (uint16_t)(f & ~FLAG_DEGRADADO);
  escribirReg(REG_FLAGS, f);
  escribirReg(REG_FIRMA, FIRMA);
  sellar();
}

bool respaldo_degradadoActivo() {
  return contenidoValido && (leerReg(REG_FLAGS) & FLAG_DEGRADADO) != 0;
}
