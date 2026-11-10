// ===== src/semaforo_test_leds.inc (MAESTRO) =====
// EL TEST DE LAMPARAS (N-82, N-82.bis): cuando se admite, como se arma y como se termina. No escribe ningun pin:
// termina por los setters de la luz de semaforo.cpp, y las tres fases las pinta semaforo_actualizar() por
// aplicarSalidas(), que se queda en semaforo.cpp con escribirPines() (SFTY-2).
//
// NO SE COMPILA SOLO: lo incluye semaforo.cpp en su sitio, dentro de su unidad de compilacion, y por eso usa sus
// static (estado, testLedsActivo, iniciarTransicionARojo). Salio de semaforo.cpp por el tope de 500 lineas
// (roadmap, "Partir"), literal; los instrumentos lo leen dentro de semaforo.cpp (banco/fuente.py, expandir_inc).
// N-82.bis (cinta del 28/09, serie 4D2007): EL TEST SOLO CORRE FUERA DE SERVICIO.
//
// La fase verde enciende VERDE1/VERDE2 sin mirar el ciclo y SFTY-2 no lo impide (solo mira
// ESTE poste). Medido con el arnes del automatico: en AUTO el test dejo el verde del Maestro
// 2 s con ESC:VERDE a la vez; por eso el Esclavo rechaza TEST_LEDS desde siempre (SPEC_4).
// Admitido solo en MENU y ALCANCE (D-44: los DOS postes en rojo fijo por
// coordinador_forzarMenu()) y con S_ROJO aqui: sin enlace pasan a S_FALLO, y un verde ahi
// seria un verde contra un ambar que invita a pasar.
//
// Y EL ROJO DEL ESCLAVO TIENE QUE CONSTAR (coordinador_rojoEsclavoConfirmado()): al entrar
// en MENU desde AUTO la luz de aqui pasa a S_ROJO en el acto, pero el Esclavo sigue en
// verde hasta que le llega el GO_RED. Se pregunta en CADA vuelta, asi que el test tambien
// se corta si el coordinador deja C_MENU_IDLE, y con el enlace caido la luz pasa a S_FALLO.
static bool testLedsAdmitido() {
  const ModoSistema m = modoActual_get();
  return (m == MENU || m == MODO_ALCANCE) && estado == S_ROJO &&
         coordinador_rojoEsclavoConfirmado();
}

// Al acabar el test -o al cortarlo- la lampara vuelve a decir lo que dice 'estado', por
// el setter de ese estado y no con una luz fija: el final de antes era un rojo fijo, y
// con el Maestro en S_VERDE dejaba luz roja, pluma abajo y $STATUS en VERDE a la vez
// hasta la siguiente transicion (cinta del 28/09). S_FALLO recupera su parpadeo porque
// semaforo_iniciarFallo() rearma su reloj.
static void terminarTestLeds() {
  testLedsActivo = false;
  switch (estado) {
    case S_ROJO:     semaforo_forzarRojo(); break;
    case S_VERDE:    semaforo_forzarVerde(); break;
    case S_AMARILLO: iniciarTransicionARojo(false); break;   // el cierre sigue su reloj
    case S_FALLO:    semaforo_iniciarFallo(); break;
    case S_ROJO_AMARILLO: semaforo_forzarVerde(); break;   // D-53: el R+A sigue su reloj
    case S_AMBAR_INT: case S_ROJO_INT: break;   // D-110: inalcanzable (el test pide S_ROJO)
  }
}

void semaforo_iniciarTestLeds() {
  // EL RECHAZO NO ES MUDO: esta funcion no devuelve nada, pero bluetooth.cpp pregunta
  // semaforo_testLedsEnCurso() DESPUES de llamarla y contesta $ERR si el test no quedo
  // armado (molde SET_TIEMPOS, CLAUDE.md 2). Un test en curso que ya no esta admitido
  // se corta aqui mismo, para que esa pregunta no lea la bandera de antes.
  if (!testLedsAdmitido()) {
    if (testLedsActivo) terminarTestLeds();
    return;
  }
  testLedsActivo = true;
  tInicioTest = millis();
}

bool semaforo_testLedsEnCurso() {
  return testLedsActivo;
}
