// ===== src/semaforo_lectura.inc (MAESTRO) =====
// LO QUE LA LUZ PUBLICA: estable, estado, su nombre para el $STATUS y la anotacion de la pluma fuera de servicio.
// Solo LEE los static de semaforo.cpp: aqui no se escribe ningun pin ni se llama a aplicarSalidas() (SFTY-2).
//
// NO SE COMPILA SOLO: lo incluye semaforo.cpp en su sitio, dentro de su unidad de compilacion. Salio de
// semaforo.cpp por el tope de 500 lineas (roadmap, "Partir"), literal, para que escribirPines() tenga donde crecer;
// los instrumentos lo leen dentro de semaforo.cpp, en su sitio (banco/fuente.py, expandir_inc; CLAUDE.md 5).
bool semaforo_estable() {
  return estado == S_ROJO || estado == S_VERDE || estado == S_FALLO || estado == S_AMBAR_INT ||
         estado == S_ROJO_INT;
}

EstadoSemaforo semaforo_estado() {
  return estado;
}

const char* semaforo_nombreEstado() {
  switch (estado) {
    case S_ROJO: return "ROJO";
    case S_VERDE: return "VERDE";
    case S_AMARILLO: return "AMARILLO";
    case S_FALLO: return "FALLO COM";
    case S_ROJO_AMARILLO: return "ROJO+AMAR";   // D-53: 9, como FALLO COM
    case S_AMBAR_INT: return "AMBAR_INT";      // D-110: 9, cabe en estadoStr
    case S_ROJO_INT: return "ROJO_INT";
  }
  return "";
}

// D-150: true si la ultima orden de escribirPines() abrio la pluma por el interruptor de
// barrera fuera de servicio. La anotacion, no la bandera: ver su declaracion.
bool semaforo_plumaFuera() {
  return plumaFuera;
}
