// ===== 01_Firmware/ESP32_Expansion/src/reloj_ds3231.cpp =====

#include "reloj_ds3231.h"
#include "contrato.h"
#include <Wire.h>

// LA HORA NACE NO FIABLE. Estas dos lineas son la barrera entera.
//
// No hay valor por defecto optimista y no hay ruta que las salte: reloj_enHora() no
// devuelve true hasta que una lectura confirme que el oscilador no se ha parado. Un
// arranque que empezara en true tendria una ventana -corta, pero real- en la que
// cualquier consumidor de la hora recibiria una fecha inventada.
static bool sePuso = false;
static MotivoSinHora motivo = SIN_HORA_NUNCA_SE_PUSO;
static unsigned long tUltimaRevision = 0;

static uint8_t aBcd(int v) { return (uint8_t)(((v / 10) << 4) | (v % 10)); }
static int deBcd(uint8_t v) { return ((v >> 4) * 10) + (v & 0x0F); }

// Lee un registro. Devuelve false si el bus no contesta.
//
// El bool no es prolijidad: es la diferencia entre "el reloj dice que son las tres" y
// "el bus esta mudo y la variable conserva el valor de hace un rato". Wire devuelve 0
// en endTransmission() solo cuando alguien hizo ACK en esa direccion.
static bool leerReg(uint8_t reg, uint8_t* destino, uint8_t cuantos) {
  Wire.beginTransmission(DS3231_DIR);
  Wire.write(reg);
  if (Wire.endTransmission() != 0) return false;

  if (Wire.requestFrom((uint8_t)DS3231_DIR, cuantos) != cuantos) return false;
  for (uint8_t i = 0; i < cuantos; i++) {
    if (!Wire.available()) return false;
    destino[i] = (uint8_t)Wire.read();
  }
  return true;
}

static bool escribirReg(uint8_t reg, const uint8_t* datos, uint8_t cuantos) {
  Wire.beginTransmission(DS3231_DIR);
  Wire.write(reg);
  for (uint8_t i = 0; i < cuantos; i++) Wire.write(datos[i]);
  return Wire.endTransmission() == 0;
}

// Actualiza la barrera leyendo el registro de estado. Es lo unico que puede poner
// sePuso en true, y solo con el OSF a cero.
static void revisarOsf() {
  uint8_t estado;
  if (!leerReg(DS3231_REG_ESTADO, &estado, 1)) {
    sePuso = false;
    motivo = SIN_HORA_BUS_MUDO;
    return;
  }
  if (estado & DS3231_BIT_OSF) {
    // R-2: una hora con OSF puesto se declara NO FIABLE aunque los registros traigan
    // una fecha con pinta razonable. Esa es justo la trampa: un DS3231 sin pila
    // devuelve una fecha bien formada, y "bien formada" no es "cierta".
    sePuso = false;
    motivo = SIN_HORA_OSCILADOR_PARADO;
    return;
  }
  sePuso = true;
  motivo = SIN_HORA_NINGUNO;
}

void reloj_setup() {
  // El modulo ZS-042 YA TRAE SUS PULL-UPS. No se activan las internas del ESP32 ni se
  // anaden externas: dos juegos en paralelo bajan la resistencia efectiva y el flanco
  // de subida deja de llegar a nivel alto dentro del tiempo del bus.
  Wire.begin(DS3231_SDA, DS3231_SCL);

  // R-1: el OSF se lee EN EL ARRANQUE, antes de publicar ninguna hora.
  //
  // 🔴 R-3: y AQUI NO SE LIMPIA. Limpiarlo "para dejarlo limpio" al arrancar es
  // fabricar una autorizacion: el bit solo dice la verdad mientras nadie lo borre sin
  // haber puesto la hora. Se limpia unicamente tras una escritura confirmada.
  revisarOsf();
  tUltimaRevision = millis();
}

void reloj_revisar() {
  // R-4. La pila se puede agotar con el equipo en marcha; un reloj que solo se
  // comprueba al arrancar declara fiable para siempre una hora que dejo de serlo.
  unsigned long ahora = millis();
  if (ahora - tUltimaRevision < RELOJ_RELECTURA_MS) return;
  tUltimaRevision = ahora;
  revisarOsf();
}

bool reloj_enHora() { return sePuso; }
MotivoSinHora reloj_motivo() { return motivo; }

bool reloj_rangoValido(const FechaHora* f) {
  // R-7: POR BARRIDO. Los seis campos, uno detras de otro, sin atajos y sin un
  // "&& hora valida" que se de por bueno el resto. La prueba 2.7 de N-51 marcaba los
  // diez pares posibles sin llamar nunca al checksum real; una validacion que comprueba
  // "la hora" y da por buenos los minutos es exactamente eso.
  if (f == NULL) return false;
  if (f->anio    < RTC_ANIO_MIN || f->anio    > RTC_ANIO_MAX) return false;
  if (f->mes     < RTC_MES_MIN  || f->mes     > RTC_MES_MAX)  return false;
  if (f->dia     < RTC_DIA_MIN  || f->dia     > RTC_DIA_MAX)  return false;
  if (f->hora    < RTC_HORA_MIN || f->hora    > RTC_HORA_MAX) return false;
  if (f->minuto  < RTC_MIN_MIN  || f->minuto  > RTC_MIN_MAX)  return false;
  if (f->segundo < RTC_SEG_MIN  || f->segundo > RTC_SEG_MAX)  return false;
  return true;
}

bool reloj_leer(FechaHora* fh) {
  if (fh == NULL) return false;

  // LA BARRERA VA DELANTE. No hay variante "damela igual": si la hora no es fiable, el
  // que llama no la recibe. Devolverla con una bandera al lado seria confiar en que
  // todos los llamadores miren la bandera, y ese es el reparto que fallo en el STM32.
  //
  // Y SE PREGUNTA POR LA FUNCION, NO POR LA VARIABLE. Mirar `sePuso` aqui dentro seria
  // mas corto y dejaria a reloj_enHora() SIN UN SOLO LLAMADOR dentro del modulo: una
  // funcion "tengo hora?" declarada, documentada en cuatro sitios y que nadie llama es
  // la Caja Negra de Alarmas de N-73 otra vez. La barrera tiene que ser la que todos
  // usan, o deja de ser una barrera y pasa a ser una convencion.
  if (!reloj_enHora()) return false;

  uint8_t r[7];
  if (!leerReg(DS3231_REG_HORA, r, 7)) {
    sePuso = false;
    motivo = SIN_HORA_BUS_MUDO;
    return false;
  }
  fh->segundo = deBcd(r[0] & 0x7F);
  fh->minuto  = deBcd(r[1] & 0x7F);
  fh->hora    = deBcd(r[2] & 0x3F);   // se fuerza el modo 24 h al escribir
  fh->dia     = deBcd(r[4] & 0x3F);
  fh->mes     = deBcd(r[5] & 0x1F);
  fh->anio    = 2000 + deBcd(r[6]);
  return true;
}

ResultadoReloj reloj_ajustar(const FechaHora* f) {
  // R-6: SE VALIDA ANTES DE ESCRIBIR, con la trama entera en la mano. Rechazar a mitad
  // seria la escritura a medias que R-5 prohibe.
  if (!reloj_rangoValido(f)) return RELOJ_ERR_RANGO;

  // El bus se comprueba ANTES de componer nada. Si no contesta, no hay nada que
  // deshacer: es el caso "modulo ausente o SDA/SCL cruzados", y el tecnico necesita ese
  // motivo y no "no se pudo".
  uint8_t estado;
  if (!leerReg(DS3231_REG_ESTADO, &estado, 1)) {
    sePuso = false;
    motivo = SIN_HORA_BUS_MUDO;
    return RELOJ_ERR_SIN_RELOJ;
  }

  // R-5: LA TERNA SE ESCRIBE ATOMICA. Los siete registros van en UNA transaccion I2C:
  // o entran todos o no entra ninguno. Escribir hora, luego minuto, luego segundo deja
  // el reloj -si algo falla en medio- en una hora que nadie pidio y con pinta de valida,
  // que es peor que dejarlo como estaba.
  uint8_t r[7];
  r[0] = aBcd(f->segundo);
  r[1] = aBcd(f->minuto);
  r[2] = aBcd(f->hora);              // bit 6 a cero = modo 24 h
  r[3] = 1;                          // dia de la semana: no se usa, pero el registro existe
  r[4] = aBcd(f->dia);
  r[5] = aBcd(f->mes);
  r[6] = aBcd(f->anio - 2000);
  if (!escribirReg(DS3231_REG_HORA, r, 7)) return RELOJ_ERR_ESCRITURA;

  // R-3: EL OSF SE LIMPIA AQUI Y SOLO AQUI, despues de una escritura que el bus
  // confirmo. Antes de este punto el bit dice la verdad; borrarlo antes seria fabricar
  // la autorizacion que la escritura todavia no ha ganado.
  uint8_t limpio = (uint8_t)(estado & ~DS3231_BIT_OSF);
  if (!escribirReg(DS3231_REG_ESTADO, &limpio, 1)) return RELOJ_ERR_ESCRITURA;

  // R-8: SE RELEE Y SE COMPARA. Es lo que hace ajustarRelojVerificado() del Maestro
  // (:127-133): un ACK que no ha vuelto a leer solo demuestra que el bus dijo que si.
  uint8_t v[7];
  if (!leerReg(DS3231_REG_HORA, v, 7)) return RELOJ_ERR_NO_QUEDO_PUESTA;

  // LOS SEGUNDOS NO SE COMPARAN, y el motivo esta razonado en el Maestro (:129-132):
  // entre escribir y releer el RTC puede haber avanzado uno. Exigir igualdad exacta
  // convertiria un reloj sano en un rechazo aleatorio, y ese es el lado INSEGURO del
  // error: el tecnico repetiria el comando hasta que "colara", aprendiendo a ignorar el
  // rechazo. Los otros cinco campos si tienen que coincidir.
  if (deBcd(v[1] & 0x7F) != f->minuto ||
      deBcd(v[2] & 0x3F) != f->hora   ||
      deBcd(v[4] & 0x3F) != f->dia    ||
      deBcd(v[5] & 0x1F) != f->mes    ||
      (2000 + deBcd(v[6])) != f->anio) {
    return RELOJ_ERR_NO_QUEDO_PUESTA;
  }

  // Y por ultimo se vuelve a mirar el OSF. Si sigue puesto, el oscilador no arranco:
  // la hora esta escrita en unos registros que nadie hace avanzar, que es exactamente
  // el equipo que el STM32 tiene hoy con Y2 muerto. Contestar OK ahi es la mentira.
  if (!leerReg(DS3231_REG_ESTADO, &estado, 1)) return RELOJ_ERR_NO_QUEDO_PUESTA;
  if (estado & DS3231_BIT_OSF) {
    sePuso = false;
    motivo = SIN_HORA_OSCILADOR_PARADO;
    return RELOJ_ERR_OSF_SIGUE;
  }

  sePuso = true;
  motivo = SIN_HORA_NINGUNO;
  tUltimaRevision = millis();
  return RELOJ_OK;
}
