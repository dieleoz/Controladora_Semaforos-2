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

// LA DUDA SE PEGA, Y ESA ES TODA SU RAZON DE SER.
//
// Sin esta bandera, una escritura que se corta a mitad baja la barrera... y la
// relectura periodica de R-4 la VUELVE A SUBIR sesenta segundos despues, porque lo
// unico que mira es el OSF -y el OSF sigue a cero: la escritura fallida no lo toco-.
// La barrera se levantaria sola sobre unos registros medio escritos, y nadie habria
// hecho nada mal.
//
// Solo una escritura verificada de punta a punta la apaga. El limite -que no
// sobrevive al reset- esta declarado en la cabecera, no disimulado aqui.
static bool escrituraDudosa = false;

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

  // R-10: EL OSF NO ES EL UNICO BIT QUE DECIDE SI LA HORA VALE.
  //
  // El OSF contesta "el oscilador no se paro". No contesta "el numero que vas a leer
  // significa lo que crees". El bit 6 del registro de horas contesta esa segunda
  // pregunta, y hasta hoy nadie la hacia: se enmascaraba con 0x3F, que solo es
  // correcto en modo 24 h, sobre un modulo que puede venir en modo 12 h con el
  // oscilador perfectamente sano -pila buena, puesto en hora por otro equipo-.
  //
  // Se comprueba AQUI, en el mismo sitio que el OSF, para que R-1 y R-4 lo cubran sin
  // escribir una segunda ronda de reglas: lo que se lee al arrancar y cada minuto es
  // "tengo hora", no "tengo oscilador".
  //
  // Y se DECLARA NO FIABLE en vez de reescribirle el bit al chip: forzar el modo 24 h
  // significa escribir el registro de horas, o sea cambiarle la hora a un equipo que
  // esta en la calle sin que nadie lo haya pedido. Un SET_RTC lo arregla de paso,
  // porque escribe el registro entero con el bit a cero.
  uint8_t horas;
  if (!leerReg(DS3231_REG_HORA + DS3231_OFS_HORA, &horas, 1)) {
    sePuso = false;
    motivo = SIN_HORA_BUS_MUDO;
    return;
  }
  if (horas & DS3231_BIT_12H) {
    sePuso = false;
    motivo = SIN_HORA_FORMATO_12H;
    return;
  }

  // La duda de una escritura a medias no la levanta una relectura limpia: el OSF esta
  // a cero porque la escritura fallida nunca lo toco, no porque la hora sea buena.
  if (escrituraDudosa) {
    sePuso = false;
    motivo = SIN_HORA_ESCRITURA_A_MEDIAS;
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
  // EL BIT QUE HACE HONESTA A LA MASCARA SE MIRA ANTES DE APLICARLA, Y AQUI NO CUESTA
  // NI UNA TRANSACCION: el byte que dice el formato es el mismo que se va a enmascarar.
  //
  // La version anterior de esta linea decia "se fuerza el modo 24 h al escribir", y era
  // cierta de las horas que escribimos NOSOTROS. De las que ya estaban dentro cuando el
  // modulo llego, no dice nada -y son justo las que se leen antes del primer SET_RTC-.
  if (r[2] & DS3231_BIT_12H) {
    sePuso = false;
    motivo = SIN_HORA_FORMATO_12H;
    return false;
  }

  fh->segundo = deBcd(r[0] & 0x7F);
  fh->minuto  = deBcd(r[1] & 0x7F);
  fh->hora    = deBcd(r[2] & 0x3F);   // valido: el bit 12/24 se acaba de comprobar
  fh->dia     = deBcd(r[4] & 0x3F);
  fh->mes     = deBcd(r[5] & 0x1F);
  fh->anio    = 2000 + deBcd(r[6]);

  // R-11: LO QUE SALE DEL CHIP SE VALIDA CON LA MISMA VARA QUE LO QUE ENTRA.
  //
  // reloj_rangoValido() se usaba solo para filtrar lo que manda la app, y el camino de
  // LECTURA se fiaba de los registros sin mirar. Un BCD corrupto -0x99 en el mes, un
  // 0x00 en el dia- se decodifica a un entero perfectamente formado y sale por esta
  // funcion como una fecha. Es la unica defensa que tambien esta en pie DESPUES DE UN
  // RESET, cuando las dos banderas de RAM ya no estan y el OSF no vio nada raro.
  //
  // Y LLEVA MOTIVO PROPIO, que no es lo mismo que "escritura a medias": unos registros
  // incoherentes pueden venir de una escritura cortada, pero tambien de un modulo
  // defectuoso o de un bus que devuelve basura. Al tecnico le sirve la diferencia -uno
  // se arregla repitiendo el SET_RTC, el otro cambiando el modulo-, y meterlos en el
  // mismo cajon es el "no se pudo" generico que este enum existe para no dar.
  if (!reloj_rangoValido(fh)) {
    sePuso = false;
    motivo = SIN_HORA_REGISTROS_INCOHERENTES;
    return false;
  }
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

  // 🔴 LA BARRERA SE BAJA AQUI, ANTES DE TOCAR EL BUS, Y NO EN CADA RAMA DE ERROR.
  //
  // Lo que sigue puede quedarse a medias, y desde este punto hasta el final de la
  // funcion NO SE SABE QUE HAY EN LOS REGISTROS. Bajarla una sola vez, arriba, es lo
  // que hace que una rama de error NUEVA -la que alguien anada dentro de seis meses-
  // no pueda olvidarse de bajarla: para dejar la hora como fiable hay que llegar hasta
  // abajo del todo, que es donde se vuelve a subir.
  //
  // La forma anterior -un `return RELOJ_ERR_ESCRITURA` sin tocar la bandera- dejaba a
  // reloj_enHora() diciendo que si sobre unos registros medio escritos: el despachador
  // contestaba $ERR, correctamente, y la barrera contestaba lo contrario a todo el que
  // preguntara despues. Es N-80 una capa mas abajo, donde no hay tecnico que lo lea.
  sePuso = false;
  motivo = SIN_HORA_ESCRITURA_A_MEDIAS;
  escrituraDudosa = true;

  // R-5: LOS SIETE REGISTROS VAN EN UNA SOLA TRANSACCION I2C, y eso es todo lo que se
  // puede afirmar. NO SON ATOMICOS: aqui ponia "o entran todos o no entra ninguno", y
  // eso es una propiedad que el bus no da. El DS3231 autoincrementa su puntero y guarda
  // cada byte segun lo va reconociendo, asi que un NACK a mitad -modulo que se
  // desengancha, ruido en el cable- deja los primeros registros escritos y los ultimos
  // como estaban: una hora que nadie pidio y con pinta de valida.
  //
  // Lo que si esta en nuestra mano es no fiarnos: por eso la bandera de arriba, por eso
  // la relectura de R-8 va ANTES de limpiar el OSF, y por eso el que no cuadra no
  // recupera la barrera hasta que una escritura entera vuelva a verificarse.
  uint8_t r[7];
  r[0] = aBcd(f->segundo);
  r[1] = aBcd(f->minuto);
  r[2] = aBcd(f->hora);              // bit 6 a cero = modo 24 h
  r[3] = 1;                          // dia de la semana: no se usa, pero el registro existe
  r[4] = aBcd(f->dia);
  r[5] = aBcd(f->mes);
  r[6] = aBcd(f->anio - 2000);
  if (!escribirReg(DS3231_REG_HORA, r, 7)) return RELOJ_ERR_ESCRITURA;

  // R-8: SE RELEE Y SE COMPARA. Es lo que hace ajustarRelojVerificado() del Maestro
  // (:127-133): un ACK que no ha vuelto a leer solo demuestra que el bus dijo que si.
  //
  // 🔴 Y VA ANTES DE LIMPIAR EL OSF, QUE ES EL ORDEN QUE DE VERDAD IMPORTA.
  //
  // Estaba al reves, y el reves tenia una consecuencia que ninguna bandera de RAM puede
  // tapar: el OSF se borraba justo despues de que el bus dijera que si, o sea ANTES de
  // saber si la hora habia entrado. Cuando la relectura no cuadraba, la funcion
  // devolvia RELOJ_ERR_NO_QUEDO_PUESTA -bien- sobre un chip al que ya se le habia
  // BORRADO el unico bit que sobrevive al corte de corriente. En el siguiente arranque
  // el OSF valia cero, la barrera subia sola, y el modulo declaraba fiable una hora que
  // el propio modulo sabia media hora antes que no lo era.
  //
  // R-3 dice "solo se limpia tras una escritura CONFIRMADA". Confirmada por el BUS no
  // es confirmada por el RELOJ: lo primero solo demuestra que alguien hizo ACK.
  uint8_t v[7];
  if (!leerReg(DS3231_REG_HORA, v, 7)) {
    motivo = SIN_HORA_BUS_MUDO;
    return RELOJ_ERR_NO_QUEDO_PUESTA;
  }

  // LOS SEGUNDOS NO SE COMPARAN, y el motivo esta razonado en el Maestro (:129-132):
  // entre escribir y releer el RTC puede haber avanzado uno. Exigir igualdad exacta
  // convertiria un reloj sano en un rechazo aleatorio, y ese es el lado INSEGURO del
  // error: el tecnico repetiria el comando hasta que "colara", aprendiendo a ignorar el
  // rechazo. Los otros cinco campos si tienen que coincidir.
  //
  // Y EL BIT 12/24 SE COMPARA COMO UN CAMPO MAS: se pidio modo 24 h -r[2] lo lleva a
  // cero- y si no quedo asi, lo que el chip guarda no significa lo que se mando, por
  // mucho que los otros cinco numeros coincidan.
  if ((v[2] & DS3231_BIT_12H) != 0    ||
      deBcd(v[1] & 0x7F) != f->minuto ||
      deBcd(v[2] & 0x3F) != f->hora   ||
      deBcd(v[4] & 0x3F) != f->dia    ||
      deBcd(v[5] & 0x1F) != f->mes    ||
      (2000 + deBcd(v[6])) != f->anio) {
    return RELOJ_ERR_NO_QUEDO_PUESTA;
  }

  // R-3: EL OSF SE LIMPIA AQUI Y SOLO AQUI, con la hora ya releida y cuadrada. El
  // estado se vuelve a leer en vez de reusar el de arriba: entre aquella lectura y esta
  // van dos transacciones, y devolver al registro una copia vieja reescribiria banderas
  // que hayan cambiado por el camino.
  if (!leerReg(DS3231_REG_ESTADO, &estado, 1)) {
    motivo = SIN_HORA_BUS_MUDO;
    return RELOJ_ERR_NO_QUEDO_PUESTA;
  }
  uint8_t limpio = (uint8_t)(estado & ~DS3231_BIT_OSF);
  if (!escribirReg(DS3231_REG_ESTADO, &limpio, 1)) return RELOJ_ERR_ESCRITURA;

  // Y por ultimo se vuelve a mirar el OSF. Si sigue puesto, el oscilador no arranco:
  // la hora esta escrita en unos registros que nadie hace avanzar, que es exactamente
  // el equipo que el STM32 tiene hoy con Y2 muerto. Contestar OK ahi es la mentira.
  if (!leerReg(DS3231_REG_ESTADO, &estado, 1)) {
    motivo = SIN_HORA_BUS_MUDO;
    return RELOJ_ERR_NO_QUEDO_PUESTA;
  }
  if (estado & DS3231_BIT_OSF) {
    motivo = SIN_HORA_OSCILADOR_PARADO;
    return RELOJ_ERR_OSF_SIGUE;
  }

  // LA UNICA SALIDA QUE VUELVE A SUBIR LA BARRERA, y esta detras de las seis negativas.
  sePuso = true;
  motivo = SIN_HORA_NINGUNO;
  escrituraDudosa = false;
  tUltimaRevision = millis();
  return RELOJ_OK;
}
