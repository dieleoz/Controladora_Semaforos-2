// ===== dos_puntas/orquestador_interruptores.inc =====
//
// BLOQUE S - SPEC_8 6 EN EL ESCLAVO REAL (D-148..D-152): la orden entra por el puerto del
// telefono y la despacha el bluetooth.cpp REAL; la bandera vive en respaldo.cpp REAL, en el
// dominio que sobrevive al microcorte. Lo incluye orquestador.cpp antes de main(); main()
// llama a bloqueInterruptores() tras el bloque N. Las tramas esperadas salen de SPEC_8 6.1.

static const char* const S_PIN = "CMD:PIN:1234:";
static const long S_FLAG_BARRERA_FUERA = 0x0400;   // D-95, de la DECISION: DR4 es el indice 3
static bool g_fueraPorOrdenEsc = false;

// La excepcion de A9/SFTY-28 por NOMBRE (SPEC_8 6.3): solo el Esclavo, con FLAG_BARRERA_FUERA en
// su DR4 REAL, puesta por SU orden contestada con OK, y anotada por escribirPines() (D-150).
static bool excepcionFueraOrq(int i) {
  return i == 1 && g_fueraPorOrdenEsc && (ESCLAVO.domLeer(3) & S_FLAG_BARRERA_FUERA) != 0 &&
         ESCLAVO.orden("pluma_fuera") == 1;
}

static bool sOrden(const std::string& accion, const std::string& espera) {
  ESCLAVO.orden(("bt:" + std::string(S_PIN) + accion).c_str());
  unTick();   // el despachador real atiende la linea en este tick
  const bool ok = ESCLAVO.orden(("ack:" + espera).c_str()) == 1;
  if (ok && espera == "$ACK,CMD:SET_BARRERA_SERVICIO:OFF,RESULT:OK") g_fueraPorOrdenEsc = true;
  if (ok && espera == "$ACK,CMD:SET_BARRERA_SERVICIO:ON,RESULT:OK") g_fueraPorOrdenEsc = false;
  return ok;
}
static bool sArriba() { return ESCLAVO.pin(MOTOR_TALANQUERA) == TALANQUERA_ABRIR; }
static bool sStatus(const char* que) {
  avanzar(2500);   // el $STATUS del Esclavo sale cada 2 s
  return ESCLAVO.orden((std::string("status:") + que).c_str()) == 1;
}

static void bloqueInterruptores() {
  std::printf("\n-- Bloque S: los interruptores de SPEC_8 6 en el Esclavo REAL --\n");
  escenarioLimpio(0);
  microcorte(ESCLAVO);   // D-148: el primer arranque de un equipo nuevo encuentra el respaldo vacio
  MAESTRO.orden("forzar_rojo_total");
  avanzar(20000);
  comprobar(ESCLAVO.estado() == S_ROJO_V && !sArriba(),
            "S0: el Esclavo en S_ROJO estable, pluma abajo, interruptores de fabrica");

  // S1: D-149, el OFF mueve el pin DENTRO de la orden, en S_ROJO estable, y PLUMA:FUERA (D-150)
  const bool s1 = sOrden("SET_BARRERA_SERVICIO:OFF", "$ACK,CMD:SET_BARRERA_SERVICIO:OFF,RESULT:OK");
  comprobar(s1 && sArriba() && ESCLAVO.rojo() && ESCLAVO.orden("pluma_fuera") == 1,
            "S1 (D-96, D-149): SET_BARRERA_SERVICIO:OFF por el bluetooth.cpp REAL contesta OK y el pin "
            "del Esclavo queda en ABRIR en ese tick, con su rojo encendido");
  avanzar(30000);
  comprobar(sArriba() && ESCLAVO.estado() == S_ROJO_V && sStatus("PLUMA:FUERA"),
            "S2 (D-96, D-97, D-150): 30 s despues sigue arriba en rojo, y el $STATUS dice PLUMA:FUERA");

  // S3: D-152, reinicio simulado con la bandera puesta: un $EVENT al arrancar; sin ella, ninguno
  microcorte(ESCLAVO);
  const bool evento = ESCLAVO.orden("evento:ORIGEN:INTERRUPTOR,DETALLE:ARRANQUE_BARRERA_FUERA") == 1;
  avanzar(5000);
  const bool sigue = (ESCLAVO.domLeer(3) & S_FLAG_BARRERA_FUERA) != 0;
  // D-168 (SE INVIERTE «queda ABAJO y PLUMA:ABAJO», CLAUDE.md 9): el arranque deja la pluma ARRIBA con su cierre
  // armado, y la reentrada de D-33 vuelve a pasar por escribirPines() con la bandera ya leida: FUERA (D-150).
  const bool arribaCoherente = sArriba() && ESCLAVO.orden("pluma_fuera") == 1 && sStatus("PLUMA:FUERA");
  comprobar(evento && sigue && arribaCoherente,
            "S3 (D-95, D-152, D-168): tras el microcorte la bandera sigue en el DR4 REAL y el arranque emite "
            "$EVENT ORIGEN:INTERRUPTOR,DETALLE:ARRANQUE_BARRERA_FUERA; la pluma no baja nunca: sale ARRIBA del "
            "arranque y la reentrada la anota FUERA, y el $STATUS dice FUERA (D-150)");
  // S3b (D-149) SE INVIERTE: «bandera puesta y pin abajo» ya no se alcanza tras un corte (D-168); con el estado ya
  // pedido, repetir el OFF es SIN_CAMBIO y no mueve nada.
  const bool s3b = sOrden("SET_BARRERA_SERVICIO:OFF", "$ACK,CMD:SET_BARRERA_SERVICIO:OFF,RESULT:SIN_CAMBIO");
  comprobar(s3b && sArriba(),
            "S3b (D-149, D-168): con la bandera puesta y la pluma ya FUERA desde el arranque, repetir el OFF es "
            "SIN_CAMBIO y la pluma sigue arriba");
  const bool s4 = sOrden("SET_BARRERA_SERVICIO:ON", "$ACK,CMD:SET_BARRERA_SERVICIO:ON,RESULT:OK");
  const bool arribaTrasOn = sArriba();
  unsigned long bajo = 0;
  for (unsigned long h = 0; h < 3UL * g_retardoPlumaMs && sArriba(); h += PASO_MS) { unTick(); bajo = h + PASO_MS; }
  char s4t[240];
  std::snprintf(s4t, sizeof(s4t), "S4 (D-149): SET_BARRERA_SERVICIO:ON en rojo contesta OK y la pluma "
                "baja a los %lu ms (retardo %lu ms), no en el acto", bajo, g_retardoPlumaMs);
  comprobar(s4 && arribaTrasOn && !sArriba() && bajo >= g_retardoPlumaMs &&
            bajo <= g_retardoPlumaMs + 2 * PASO_MS, s4t);
  microcorte(ESCLAVO);
  comprobar(ESCLAVO.orden("evento:INTERRUPTOR") == 0,
            "S5 (D-152): con las dos banderas en ON, el arranque NO emite el aviso");

  // S6: D-148, el respaldo perdido en ESTE arranque: OFF rechazado; el siguiente arranque, aceptado
  ESCLAVO.domEscribir(0, 0x1234);   // DR1: la firma, rota (pila agotada en el ultimo corte)
  microcorte(ESCLAVO);
  avanzar(2000);
  const bool perdido = ESCLAVO.orden("respaldo_perdido") == 1;
  const bool r1 = sOrden("SET_VETO_CAMARA:OFF", "$ERR,CMD:SET_VETO_CAMARA:OFF,DESC:RESPALDO_INVALIDO");
  const bool r2 = sOrden("SET_BARRERA_SERVICIO:ON", "$ACK,CMD:SET_BARRERA_SERVICIO:ON,RESULT:SIN_CAMBIO");
  microcorte(ESCLAVO);
  avanzar(2000);
  const bool r3 = ESCLAVO.orden("respaldo_perdido") == 0 &&
                  sOrden("SET_VETO_CAMARA:OFF", "$ACK,CMD:SET_VETO_CAMARA:OFF,RESULT:OK");
  comprobar(perdido && r1 && r2 && r3,
            "S6 (D-148): con la firma del respaldo rota, ese arranque rechaza el OFF con "
            "RESPALDO_INVALIDO y el ON es SIN_CAMBIO; el arranque siguiente, ya sellado, acepta el OFF");

  // S7: D-151 con el veto fuera: NOVETO solo con '?' u OK; PEGADA y CIEGA se publican tal cual
  const char* const E[] = {"?", "OK", "PEGADA", "CIEGA"};
  const char* const ESPERA[] = {"CAM:NOVETO", "CAM:NOVETO", "CAM:PEGADA", "CAM:CIEGA"};
  bool s7 = true;
  std::string s7d;
  for (int k = 0; k < 4; k++) {
    ESCLAVO.orden("cam_estado", k);
    const bool b = sStatus(ESPERA[k]);
    s7 = s7 && b;
    s7d += std::string(" ") + E[k] + (b ? "=ok" : "=MAL");
  }
  const bool s7on = sOrden("SET_VETO_CAMARA:ON", "$ACK,CMD:SET_VETO_CAMARA:ON,RESULT:OK");
  ESCLAVO.orden("cam_estado", 1);
  comprobar(s7 && s7on && sStatus("CAM:OK"),
            "S7 (D-151): con SET_VETO_CAMARA:OFF el $STATUS del Esclavo dice CAM:NOVETO con '?' y "
            "OK, y CAM:PEGADA / CAM:CIEGA con esas; devuelto el veto, CAM:OK." + s7d);
}
