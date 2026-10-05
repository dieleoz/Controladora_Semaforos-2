# ===== banco/modelos/esclavo.py =====
#
# MODELO DEL ESCLAVO — portado funcion a funcion del C++.
#
# Separado de las pruebas (venia de validador_esclavo.py, 1.805 lineas): el modelo imita al
# C++ y los packs le exigen. Donde simplifica se dice en el comentario: no hay pines, ni CRC,
# ni RTC de verdad.
#
# ⚠️ ES UNA COPIA DEL FIRMWARE ESCRITA A MANO y puede quedarse atras (N-36, N-39): cada
# constante se RELEE del C++ en cada corrida; la logica, no. Rehecho para D-53 (05/10) desde
# semaforo.cpp, main.cpp, modo_degradado.cpp y coordinador.cpp.

from banco import fuente as _fw

# --------------------------------------------------------------------------
# Constantes leidas del firmware real (anti-deriva modelo/firmware).
# Si alguna no se puede leer, banco.fuente ABORTA: sin valor por defecto, nunca.
# --------------------------------------------------------------------------
_ESC_MAIN = ("Esclavo", "src", "main.cpp")
_ESC_SEM = ("Esclavo", "src", "semaforo.cpp")
_ESC_DEG = ("Esclavo", "src", "modo_degradado.cpp")
_ESC_PROTO = ("Esclavo", "include", "protocolo.h")
_ESC_CICLO = ("Esclavo", "include", "ciclo_degradado.h")
_MAE_COORD = ("Maestro", "src", "coordinador.cpp")

AMARILLO_CIERRE_MS = 1000 * _fw.constante(_ESC_PROTO, r"#define\s+AMARILLO_SEG\s+(\d+)UL",
                                          "amarillo de cierre (D-45)")
# D-53: el ROJO+AMARILLO antes de cada verde (semaforo.cpp: ROJO_AMARILLO_SEG * 1000UL).
ROJO_AMARILLO_SEG = _fw.constante(_ESC_PROTO, r"#define\s+ROJO_AMARILLO_SEG\s+(\d+)UL",
                                  "rojo+amarillo (D-53)")
ROJO_AMARILLO_MS = 1000 * ROJO_AMARILLO_SEG
M_ROJO_AMARILLO_MS = 1000 * _fw.constante(("Maestro", "include", "protocolo.h"),
                                          r"#define\s+ROJO_AMARILLO_SEG\s+(\d+)UL",
                                          "rojo+amarillo del Maestro (D-53)")

VENTANA_HORA_MS = _fw.constante(_ESC_MAIN, r"VENTANA_HORA_MS\s*=\s*(\d+)", "caducidad del buffer de hora")

# N-41: la ventana de vigencia del VERDE del par de configuracion, leida del C++ (sin ella
# el modelo media la conducta pegajosa de antes del arreglo).
_ESC_CONFIG = ("Esclavo", "src", "config_ciclo.cpp")
VENTANA_CONFIG_MS = _fw.constante(_ESC_CONFIG, r"VENTANA_CONFIG_MS\s*=\s*(\d+)",
                                  "ventana de vigencia del VERDE a la espera del DESPEJE")
RETARDO_RESPUESTA_MS = _fw.constante(_ESC_MAIN, r"RETARDO_RESPUESTA_MS\s*=\s*(\d+)", "retardo de cortesia")
# N-69: el umbral dejo de ser un literal en el .cpp y vive ahora una sola vez en el
# contrato compartido -protocolo.h-, para que las dos puntas no puedan divergir. El
# modelo lo lee de ahi, que es donde esta la verdad; leerlo del sitio viejo daria
# ABORTADO, que es justo lo que hizo el dia del cambio y por eso se entero nadie.
SILENCIO_A_AMBAR_MS = _fw.constante(("Esclavo", "include", "protocolo.h"),
                                    r"#define\s+SFTY6_SILENCIO_MS\s+(\d+)UL",
                                    "caida a ambar por silencio de radio")
# D-34: el margen con que el Esclavo suelta su verde antes de ese silencio, y el valor del
# param del PONG que lo avisa. Se leen de protocolo.h, donde el firmware los lee.
AVISO_AMBAR_TIMEOUT_MS = _fw.constante(("Esclavo", "include", "protocolo.h"),
                                       r"#define\s+AVISO_AMBAR_TIMEOUT_MS\s+(\d+)UL",
                                       "margen de suelta del verde (D-34)")
PONG_VERDE_SOLTADO = _fw.comando(("Esclavo", "include", "protocolo.h"), "PONG_VERDE_SOLTADO")

LIMITE_SIN_SYNC_H = _fw.constante(_ESC_DEG, r"LIMITE_SIN_SYNC_MS\s*=\s*(\d+)UL\s*\*\s*3600UL", "limite duro sin sync")
AVISO_SIN_SYNC_H = _fw.constante(_ESC_DEG, r"AVISO_SIN_SYNC_MS\s*=\s*(\d+)UL\s*\*\s*3600UL", "aviso de proximidad")
ROJO_MINIMO_MS = _fw.constante(_ESC_DEG, r"ROJO_MINIMO_MS\s*=\s*(\d+)", "suelo del todo-rojo")
PERIODO_FASE_MS = _fw.constante(_ESC_DEG, r"PERIODO_FASE_MS\s*=\s*(\d+)", "periodo de recalculo de fase")
LIMITE_SIN_SYNC_MS = LIMITE_SIN_SYNC_H * 3600 * 1000
AVISO_SIN_SYNC_MS = AVISO_SIN_SYNC_H * 3600 * 1000

SEGUNDOS_DEL_DIA = _fw.constante(_ESC_CICLO, r"SEGUNDOS_DEL_DIA\s*=\s*(\d+)UL", "segundos del dia")

TIMEOUT_ACK_MS = _fw.constante(_MAE_COORD, r"TIMEOUT_ACK_MS\s*=\s*(\d+)", "timeout de ACK del Maestro")
LATIDO_MS = _fw.constante(_MAE_COORD, r"LATIDO_MS\s*=\s*(\d+)",
                          "cadencia del latido del Maestro")
# N-71: antes se leia del literal de la comparacion -"retryCount >= 5"-. Al darle
# nombre a ese 5 el patron dejo de encontrarlo y este modelo ABORTO, que es lo que
# tenia que hacer: la alternativa habria sido un modelo midiendo un numero fantasma.
REINTENTOS_MAX = _fw.constante(_MAE_COORD, r"CICLO_MAX_REINTENTOS\s*=\s*(\d+)",
                               "reintentos del ciclo antes de C_FALLO")
MAESTRO_SIN_RX_MS = _fw.constante(("Maestro", "include", "protocolo.h"),
                                  r"#define\s+SFTY6_SILENCIO_MS\s+(\d+)UL",
                                  "silencio del Esclavo que tumba al Maestro")

CMD = {n: _fw.comando(_ESC_PROTO, n) for n in (
    "CMD_GO_GREEN", "CMD_GO_RED", "CMD_ACK_GREEN", "CMD_PING", "CMD_PONG", "CMD_ACK_RED",
    "CMD_HORA_H", "CMD_HORA_M", "CMD_HORA_S", "CMD_ACK_HORA",
    "CMD_DELTA", "CMD_DELTA_RESP",
    "CMD_CONFIG_VERDE", "CMD_CONFIG_DESPEJE", "CMD_ACK_CONFIG")}
DELTA_FUERA_DE_RANGO = -128



# ==========================================================================
# 1. MODELO DEL ESCLAVO
# ==========================================================================

class Semaforo:
    """Puerto de src/semaforo.cpp (D-30: sin la senal de confirmacion del mando)."""

    def __init__(self, nodo):
        self.nodo = nodo
        self.estado = "S_ROJO"
        self.tCambio = 0
        self.pines = (False, False, False)   # rojo, amarillo, verde EN EL POSTE
        self.ult = (False, False, False)
        self.verde_en_pines_alguna_vez = False

    def _escribir_pines(self, r, a, v):
        self.pines = (r, a, v)
        if v:
            self.verde_en_pines_alguna_vez = True

    def _aplicar(self, r, a, v):
        # SFTY-2: enclavamiento logico. El rojo siempre gana AL VERDE; el amarillo no se toca
        # (aplicarSalidas() del C++), y D-53 pide rojo y amarillo a la vez.
        if r:
            v = False
        elif v:
            r = False
        if r and v:
            v = False
        # `ult` se guarda igual que en el C++: es lo ultimo que pidio la logica, y de
        # ella cuelga la reentrada de la pluma (D-33). Lo que ya no hay es el desvio
        # `if senalActiva: return` que la senal del mando metia justo aqui.
        self.ult = (r, a, v)
        self._escribir_pines(r, a, v)

    def forzar_rojo(self):
        # D-45: sobre un verde arranca el amarillo de cierre; un cierre en curso no se reinicia.
        if self.estado == "S_VERDE":
            self.estado = "S_AMARILLO"
            self.tCambio = self.nodo.t
            self._aplicar(False, True, False)
            return
        if self.estado == "S_AMARILLO":
            return
        self.estado = "S_ROJO"   # D-53: tambien desde S_ROJO_AMARILLO, directo: no hubo verde
        self._aplicar(True, False, False)

    def forzar_verde(self):
        # D-53: abre por S_ROJO_AMARILLO y el VERDE lo pone actualizar() a ROJO_AMARILLO_MS;
        # repetida no reinicia (repinta). Con el cierre en curso no se reabre (D-45).
        if self.estado == "S_AMARILLO":
            return
        if self.estado == "S_VERDE":
            self._aplicar(False, False, True)
            return
        if self.estado != "S_ROJO_AMARILLO":
            self.estado = "S_ROJO_AMARILLO"
            self.tCambio = self.nodo.t
        self._aplicar(True, True, False)

    def iniciar_fallo(self):
        self.estado = "S_FALLO"
        self.tCambio = self.nodo.t
        self._aplicar(False, False, False)

    def estable(self):
        return self.estado in ("S_ROJO", "S_VERDE", "S_FALLO")   # S_ROJO_AMARILLO no

    def nombre_estado(self):
        """semaforo_nombreEstado(): el ESTADO del $STATUS."""
        return {"S_ROJO": "ROJO", "S_VERDE": "VERDE", "S_AMARILLO": "AMARILLO",
                "S_FALLO": "FALLO COM", "S_ROJO_AMARILLO": "ROJO+AMAR"}.get(self.estado, "")

    def actualizar(self):
        ahora = self.nodo.t
        if self.estado == "S_AMARILLO" and (ahora - self.tCambio) >= AMARILLO_CIERRE_MS:
            self.estado = "S_ROJO"
            self._aplicar(True, False, False)
        elif self.estado == "S_ROJO_AMARILLO" and (ahora - self.tCambio) >= ROJO_AMARILLO_MS:
            self.estado = "S_VERDE"   # D-53: el R+A acaba en VERDE
            self._aplicar(False, False, True)
        elif self.estado == "S_FALLO":
            if ahora - self.tCambio >= 500:
                self.tCambio = ahora
                self.nodo._ambar_status = not getattr(self.nodo, "_ambar_status", False)
                self._aplicar(False, self.nodo._ambar_status, False)


_NOMBRES_FASE = ("FD_ROJO_AMARILLO_MAESTRO", "FD_VERDE_MAESTRO", "FD_AMARILLO_MAESTRO",
                 "FD_DESPEJE_A", "FD_ROJO_AMARILLO_ESCLAVO", "FD_VERDE_ESCLAVO",
                 "FD_AMARILLO_ESCLAVO", "FD_DESPEJE_B")


def ciclo_degradado_fase(seg_dia, verde, despeje):
    """Puerto de include/ciclo_degradado.h: el MISMO espejo de costura (que lo contrasta
    contra el C++ huella a huella), con los nombres de la fase en vez de su indice."""
    from banco.modelos.costura import fase as _fase
    return _NOMBRES_FASE[_fase(seg_dia, verde, despeje)]


def ciclo_degradado_restante(seg_dia, verde, despeje):
    """Puerto de ciclo_degradado_restante(): segundos hasta que cambia la fase (0 = no cambia
    dentro del tope)."""
    from banco.modelos.costura import fase as _fase, AMARILLO_S as a
    if verde == 0 or despeje == 0:
        return 0
    actual = _fase(seg_dia, verde, despeje)
    r = ROJO_AMARILLO_SEG
    tope = 2 * (r + verde + a + despeje) + despeje + a + r + 2
    for t in range(1, tope + 1):
        if _fase((seg_dia + t) % SEGUNDOS_DEL_DIA, verde, despeje) != actual:
            return t
    return 0


class ModoDegradado:
    """Puerto de src/modo_degradado.cpp."""

    def __init__(self, nodo):
        self.nodo = nodo
        self.estado = "DEG_INACTIVO"
        self.tCambioEstado = 0
        self.rendicion_en_curso = False
        self.verde_aplicado = False
        self.hubo_sync = False
        self.tUltimaSync = 0
        self.sync_vencida = False
        self.fase_cache = "FD_DESPEJE_A"
        self.tFaseCache = 0

    def _rojo_obligatorio_ms(self):
        ms = self.nodo.config_despeje_segundos() * 1000
        return ROJO_MINIMO_MS if ms < ROJO_MINIMO_MS else ms

    def _calcular_fase(self):
        if self.nodo.t - self.tFaseCache >= PERIODO_FASE_MS:
            self.tFaseCache = self.nodo.t
            self.fase_cache = ciclo_degradado_fase(self.nodo.reloj_segundos_del_dia(),
                                                   self.nodo.config_verde_segundos(),
                                                   self.nodo.config_despeje_segundos())
        return self.fase_cache

    def _verde_con_tiempo(self):
        """verdeConTiempo() (D-53): en el R+A siempre; en el VERDE solo si le quedan mas de
        ROJO_AMARILLO_SEG (sin la fase cacheada: el C++ la recalcula)."""
        s = self.nodo.reloj_segundos_del_dia()
        v, d = self.nodo.config_verde_segundos(), self.nodo.config_despeje_segundos()
        f = ciclo_degradado_fase(s, v, d)
        return f == "FD_ROJO_AMARILLO_ESCLAVO" or \
            (f == "FD_VERDE_ESCLAVO" and ciclo_degradado_restante(s, v, d) > ROJO_AMARILLO_SEG)

    def _aplicar_luz(self, verde):
        if verde == self.verde_aplicado:
            return
        if verde:
            self.nodo.semaforo.forzar_verde()   # D-53: por ROJO+AMARILLO
        else:
            self.nodo.semaforo.forzar_rojo()
        self.verde_aplicado = verde

    def _iniciar_salida(self, rendicion):
        self.nodo.semaforo.forzar_rojo()
        self.verde_aplicado = False
        self.rendicion_en_curso = rendicion
        self.estado = "DEG_SALIENDO"
        self.tCambioEstado = self.nodo.t

    def registrar_sync(self):
        self.hubo_sync = True
        self.tUltimaSync = self.nodo.t
        self.sync_vencida = False
        if self.estado == "DEG_RENDIDO":
            self.estado = "DEG_INACTIVO"

    def comprobar(self):
        if not self.nodo.reloj_en_hora:
            return "DEG_RECHAZO_SIN_HORA"
        if not self.nodo.config_verde_recibido() or not self.nodo.config_despeje_recibido():
            return "DEG_RECHAZO_SIN_CONFIG"
        if self.nodo.config_verde_segundos() == 0 or self.nodo.config_despeje_segundos() == 0:
            return "DEG_RECHAZO_CICLO_NULO"
        if not self.hubo_sync:
            return "DEG_RECHAZO_SIN_SYNC"
        if self.sync_vencida:
            return "DEG_RECHAZO_SYNC_VENCIDA"
        # R-4: CON UN AMBAR DE LA APP VIGENTE NO SE ENTRA AL DEGRADADO. La unica entrada es
        # SET_MODO:DEGRADADO por app (D-18), que no revoca el ambar: pregunta y se lleva
        # DEG_RECHAZO_AMBAR_VIGENTE (el mando lo revocaba y podia dejar el equipo sin nada).
        if self.nodo._ambar_emergencia():
            return "DEG_RECHAZO_AMBAR_VIGENTE"
        return "DEG_ACEPTADO"

    def entrar(self):
        if self.estado in ("DEG_ENTRANDO", "DEG_ACTIVO"):
            return "DEG_ACEPTADO"
        r = self.comprobar()
        if r != "DEG_ACEPTADO":
            return r
        self.nodo.semaforo.forzar_rojo()
        self.verde_aplicado = False
        self.estado = "DEG_ENTRANDO"
        self.rendicion_en_curso = False
        self.tCambioEstado = self.nodo.t
        self.tFaseCache = self.nodo.t - PERIODO_FASE_MS
        return "DEG_ACEPTADO"

    def salir(self):
        if self.estado == "DEG_RENDIDO":
            self.estado = "DEG_INACTIVO"
            return
        if self.estado not in ("DEG_ENTRANDO", "DEG_ACTIVO"):
            return
        self._iniciar_salida(False)

    def actualizar(self):
        ahora = self.nodo.t
        if self.hubo_sync and not self.sync_vencida and (ahora - self.tUltimaSync) >= LIMITE_SIN_SYNC_MS:
            self.sync_vencida = True
        if self.sync_vencida and self.estado in ("DEG_ENTRANDO", "DEG_ACTIVO"):
            self._iniciar_salida(True)
            return
        if self.estado == "DEG_ENTRANDO":
            # D-53: ni en el verde ni en el R+A que lo abre
            if (ahora - self.tCambioEstado) >= self._rojo_obligatorio_ms() and \
                    self._calcular_fase() not in ("FD_VERDE_ESCLAVO", "FD_ROJO_AMARILLO_ESCLAVO"):
                self.estado = "DEG_ACTIVO"
                self.tCambioEstado = ahora
        elif self.estado == "DEG_ACTIVO":
            f = self._calcular_fase()
            self._aplicar_luz(f in ("FD_ROJO_AMARILLO_ESCLAVO", "FD_VERDE_ESCLAVO") and
                              (self.verde_aplicado or self._verde_con_tiempo()))   # D-53, C2
        elif self.estado == "DEG_SALIENDO":
            if (ahora - self.tCambioEstado) >= self._rojo_obligatorio_ms():
                if self.rendicion_en_curso:
                    self.estado = "DEG_RENDIDO"
                    self.nodo.semaforo.iniciar_fallo()
                else:
                    self.estado = "DEG_INACTIVO"
                self.tCambioEstado = ahora

    def gobierna_luz(self):
        return self.estado in ("DEG_ENTRANDO", "DEG_ACTIVO", "DEG_SALIENDO")



class AmbarEmergencia:
    """Puerto del latch de ambar de la app: `ambarEmergencia` de src/bluetooth.cpp.

    D-30: sustituye a `class Mando` (mando.cpp). El latch de la app se queda y lo leen las
    mismas guardas de main.cpp; se REAPUNTA para que la propiedad -con un ambar puesto,
    ninguna orden de luz por radio lo pisa ni se acusa- siga teniendo quien la EJECUTE.
    NO modela el dialogo de CANCELAR_AMBAR con el Maestro (costura_14, esclavo_07).
    """

    def __init__(self, nodo):
        self.nodo = nodo
        self.armado = False

    def pedir(self):
        """CMD:AMBAR_EMERGENCIA. Arma el latch y enciende el ambar en la misma vuelta.

        Sin condiciones y desde cualquier estado: es la regla que impide que nadie
        quede atrapado con un semaforo en estado raro a 5 m de altura.
        """
        n = self.nodo
        self.armado = True
        if n.degradado.gobierna_luz():
            # Salida ORDENADA por el todo-rojo de despedida del Degradado. El ambar lo
            # enciende despues el sostenedor, no este salto.
            n.degradado.salir()
        else:
            n.semaforo.iniciar_fallo()

    def cancelar(self):
        """CMD:CANCELAR_AMBAR. Quita el latch; NO enciende ni apaga nada por su cuenta.

        La luz la mueve luego quien mande: el Maestro con su siguiente orden, o la
        caida por silencio. Apagar aqui seria decidir una luz que este comando no pide.
        """
        self.armado = False

    def actualizar(self):
        """El SOSTENEDOR de bluetooth.cpp, al final de la vuelta.

        Se RE-ARMA en cada vuelta: la orden sobrevive al todo-rojo de salida del Degradado,
        que termina en INACTIVO y no en ambar. Dos guardas (la del mando se fue con el).
        """
        n = self.nodo
        if self.armado and not n.degradado.gobierna_luz() and \
                n.semaforo.estado != "S_FALLO":
            n.semaforo.iniciar_fallo()


class Esclavo:
    """Puerto del loop() de src/main.cpp, con su misma secuencia de llamadas.

    El ORDEN importa y por eso se respeta: las camaras primero, luego las luces, la
    radio en medio y el SOSTENEDOR DEL AMBAR al final, que es lo que hace que la
    ultima palabra de cada vuelta sea del latch y no de una orden de radio que acabe
    de llegar. Hasta el 14/09 el que cerraba la vuelta era mando_actualizar(), en el
    mismo sitio y por el mismo motivo.
    """

    def __init__(self, obedece_ambar_emergencia=True):
        self.t = 0
        self.semaforo = Semaforo(self)
        self.degradado = ModoDegradado(self)
        self.ambar = AmbarEmergencia(self)
        self.rx = []            # tramas que llegan del Maestro
        self.tx = []            # (instante, comando, param) que salen al aire
        self.respuesta_pendiente = None
        self.respuesta_param = 0
        self.tEnviarRespuesta = 0
        self.tUltimoComando = 0
        self.verde_soltado_por_margen = False   # D-34
        self.ack_verde_enviado = False
        self.tInicioVerde = 0
        self.estado_luz_ant = "S_ROJO"
        self.ultimo_rechazo = "DEG_ACEPTADO"
        # D-32 (1): aqui iban `self.interfaz_arrancada = True` y `self.menu.setup()`.

        # Reloj
        self.reloj_en_hora = True
        self.reloj_h, self.reloj_m, self.reloj_s = 12, 0, 0
        self.reloj_dia = 10

        # Buffer de hora (SFTY-23)
        self.buf_hora = 0
        self.buf_minuto = 0
        self.tiene_hora = False
        self.tiene_minuto = False
        self.tBufHora = 0
        self.horas_aplicadas = []

        # Configuracion del ciclo
        self.cfg_verde = 0
        self.cfg_despeje = 0
        self.cfg_verde_recibido = False
        # El 30/31: cfg_verde_recibido mezclaba dos hechos -"la radio entrego el
        # par" (getters) y "hay un VERDE sin emparejar" (cierre del par)-. Se separan:
        # esta se CONSUME al cerrar; la otra no. Port de config_ciclo.cpp.
        self.cfg_verde_pendiente = False
        self.cfg_despeje_recibido = False
        # N-41: marca de tiempo del VERDE. El modelo no la tenia y por eso medía un
        # firmware que ya no se comporta asi. Ver config_ciclo.cpp.
        self.t_cfg_verde = 0
        self.respaldo_verde = 0
        self.respaldo_despeje = 0
        self.respaldo_hay_ciclo = False
        self.respaldo_guardados = []

        # Interruptor del CONTROL NEGATIVO del bloque 1: con el a False se modela un
        # firmware SIN la desobediencia, y la prueba tiene que cazarlo.
        self.obedece_ambar_emergencia = obedece_ambar_emergencia

    # --- reloj -----------------------------------------------------------
    def reloj_segundos_del_dia(self):
        if not self.reloj_en_hora:
            return 0
        return self.reloj_h * 3600 + self.reloj_m * 60 + self.reloj_s

    def poner_hora(self, seg_dia):
        seg_dia %= SEGUNDOS_DEL_DIA
        self.reloj_h = seg_dia // 3600
        self.reloj_m = (seg_dia % 3600) // 60
        self.reloj_s = seg_dia % 60

    def reloj_ajustar(self, h, m, s):
        if h > 23 or m > 59 or s > 59:
            return
        self.reloj_h, self.reloj_m, self.reloj_s = h, m, s
        self.reloj_en_hora = True
        self.horas_aplicadas.append((h, m, s))

    # --- configuracion del ciclo -----------------------------------------
    def cfg_radio_completo(self):
        return self.cfg_verde_recibido and self.cfg_despeje_recibido

    def verde_de_este_envio(self):
        """Port de verdeDeEsteEnvio() de config_ciclo.cpp.

        N-41: EL MODELO NO TENIA ESTO. Comprobaba cfg_verde_recibido a secas, que es la
        conducta PEGAJOSA DE ANTES DEL ARREGLO, asi que reportaba la mezcla del par
        tambien en escenarios donde el firmware ya la rechaza. Medido con un hueco de
        10 s: el firmware rechaza y el modelo mezclaba (30,25) Y ADEMAS lo acusaba.

        Misma familia que N-36, N-39 y N-40 -el instrumento se queda en una version
        anterior del codigo- pero al reves de lo habitual: aquellos hacian acusar EN
        FALSO; este hacia acusar DE MAS."""
        return (self.cfg_verde_pendiente
                and (self.t - self.t_cfg_verde) <= VENTANA_CONFIG_MS)

    def config_verde_segundos(self):
        return self.cfg_verde if self.cfg_radio_completo() else self.respaldo_verde

    def config_despeje_segundos(self):
        return self.cfg_despeje if self.cfg_radio_completo() else self.respaldo_despeje

    def config_verde_recibido(self):
        return self.cfg_radio_completo() or self.respaldo_hay_ciclo

    def config_despeje_recibido(self):
        return self.cfg_radio_completo() or self.respaldo_hay_ciclo

    def _respaldo_guardar_ciclo(self, verde, despeje):
        if verde == 0 or despeje == 0:
            return
        self.respaldo_verde = verde
        self.respaldo_despeje = despeje
        self.respaldo_hay_ciclo = True
        self.respaldo_guardados.append((verde, despeje))

    # --- radio -----------------------------------------------------------
    def programar_respuesta(self, cmd, param=0):
        self.respuesta_pendiente = cmd
        self.respuesta_param = param
        self.tEnviarRespuesta = self.t + RETARDO_RESPUESTA_MS

    def _atender_respuesta_pendiente(self):
        if self.respuesta_pendiente is None:
            return
        if self.t < self.tEnviarRespuesta:
            return
        self.tx.append((self.t, self.respuesta_pendiente, self.respuesta_param))
        self.respuesta_pendiente = None
        self.respuesta_param = 0

    def _caducar_buffer_hora(self):
        if not self.tiene_hora and not self.tiene_minuto:
            return
        if self.t - self.tBufHora > VENTANA_HORA_MS:
            self.tiene_hora = False
            self.tiene_minuto = False

    def _calcular_desfase(self, segundo_maestro):
        if not self.reloj_en_hora:
            return DELTA_FUERA_DE_RANGO
        if segundo_maestro > 59:
            return DELTA_FUERA_DE_RANGO
        d = segundo_maestro - self.reloj_s
        if d > 30:
            d -= 60
        elif d < -30:
            d += 60
        if d > 127 or d < -127:
            return DELTA_FUERA_DE_RANGO
        return d

    def _ambar_emergencia(self):
        # Puerto de `!bluetooth_ambarEmergencia()`, el termino que queda en las tres
        # guardas de main.cpp desde que el del mando se fue (D-30).
        return self.ambar.armado and self.obedece_ambar_emergencia

    def _procesar(self, pkt):
        cmd, param = pkt
        if self.degradado.gobierna_luz() and cmd in (CMD["CMD_PING"], CMD["CMD_GO_RED"], CMD["CMD_GO_GREEN"]):
            self.degradado.salir()

        if cmd == CMD["CMD_PING"]:
            if self.semaforo.estado != "S_FALLO":
                self.tUltimoComando = self.t
            # D-34: el param dice si esta punta solto su verde por margen
            self.programar_respuesta(CMD["CMD_PONG"],
                                     PONG_VERDE_SOLTADO if self.verde_soltado_por_margen else 0)
        elif cmd == CMD["CMD_GO_RED"]:
            self.tUltimoComando = self.t
            self.verde_soltado_por_margen = False   # D-34
            if not self._ambar_emergencia():
                # D-45: el ACK_RED acusa el ROJO ENCENDIDO; tras el amarillo, en loop().
                self.semaforo.forzar_rojo()
                if self.semaforo.estado == "S_ROJO":
                    self.programar_respuesta(CMD["CMD_ACK_RED"])
                else:
                    self.ack_rojo_pendiente = True
        elif cmd == CMD["CMD_GO_GREEN"]:
            # D-34: la repeticion (luz ya en ambar, verde o, D-53, R+A) no refresca el silencio
            if self.semaforo.estado not in ("S_AMARILLO", "S_VERDE", "S_ROJO_AMARILLO"):
                self.tUltimoComando = self.t
            self.verde_soltado_por_margen = False   # D-34
            # D-45: con el cierre en curso ni se reabre ni se acusa. D-53: el ACK_GREEN acusa
            # el VERDE ENCENDIDO: la orden nueva abre por R+A sin acusar (acusa el vigilante del
            # final del bucle), repetida en el R+A ni reinicia ni acusa, sobre S_VERDE re-acusa.
            luz = self.semaforo.estado
            if not self._ambar_emergencia() and luz not in ("S_AMARILLO", "S_ROJO_AMARILLO"):
                if luz != "S_VERDE":
                    self.semaforo.forzar_verde()
                    self.ack_verde_enviado = False
                    self.ack_rojo_pendiente = False
                else:
                    self.programar_respuesta(CMD["CMD_ACK_GREEN"])
        elif cmd == CMD["CMD_HORA_H"]:
            if param <= 23:
                self.buf_hora = param
                self.tiene_hora = True
                self.tBufHora = self.t
        elif cmd == CMD["CMD_HORA_M"]:
            if param <= 59:
                self.buf_minuto = param
                self.tiene_minuto = True
                if not self.tiene_hora:
                    self.tBufHora = self.t
        elif cmd == CMD["CMD_HORA_S"]:
            if self.tiene_hora and self.tiene_minuto and param <= 59:
                self.reloj_ajustar(self.buf_hora, self.buf_minuto, param)
                self.degradado.registrar_sync()
                self.programar_respuesta(CMD["CMD_ACK_HORA"])
            self.tiene_hora = False
            self.tiene_minuto = False
        elif cmd == CMD["CMD_DELTA"]:
            self.programar_respuesta(CMD["CMD_DELTA_RESP"], self._calcular_desfase(param))
        elif cmd == CMD["CMD_CONFIG_VERDE"]:
            # Port de config_rxVerde(). NO se acusa aqui: el Maestro espera UN solo ACK
            # del par y confirmar las dos por separado le devolveria uno sobrante.
            self.cfg_verde = param
            self.cfg_verde_recibido = True
            self.cfg_verde_pendiente = True
            self.t_cfg_verde = self.t          # N-41: el par tiene que cerrarse en la ventana
        elif cmd == CMD["CMD_CONFIG_DESPEJE"]:
            # Port de config_rxDespeje(). N-41: el par solo se cierra si el VERDE es de
            # ESTE envio. Si no, se descarta lo que hubiera y SE CALLA -el silencio es
            # lo que provoca el reintento del Maestro-.
            if not self.verde_de_este_envio():
                self.cfg_verde_pendiente = False   # basura para este par
            else:
                self.cfg_despeje = param
                self.cfg_despeje_recibido = True
                # El 30/31: el VERDE se CONSUME al cerrar el par. Se apaga la bandera de
                # EMPAREJAR, no la de "la radio hablo": esa sostiene a los getters.
                # Port de la misma linea que arregla config_ciclo.cpp -sin ella el
                # modelo seguiria midiendo el firmware de ANTES del arreglo, que es
                # justo la clase de deriva de N-36/N-39/N-40 al reves.
                self.cfg_verde_pendiente = False
                self._respaldo_guardar_ciclo(self.cfg_verde, self.cfg_despeje)
                self.programar_respuesta(CMD["CMD_ACK_CONFIG"])

        if not self._ambar_emergencia() and self.semaforo.estado == "S_FALLO" and cmd == CMD["CMD_GO_RED"]:
            self.semaforo.forzar_rojo()

    # --- bucle principal --------------------------------------------------
    def loop(self, dt=10):
        self.t += dt

        # D-44: botones_actualizar() ya solo lee las camaras de J16; p5/p8 no se leen.

        self.semaforo.actualizar()
        self._atender_respuesta_pendiente()
        self._caducar_buffer_hora()
        self.degradado.actualizar()

        if self.rx:
            self._procesar(self.rx.pop(0))

        # D-34: el verde se suelta antes que el ambar; D-45: por su amarillo, que empieza un
        # AMARILLO_SEG antes para que el rojo caiga donde caia
        # D-53: y el ROJO+AMARILLO, que es el principio de ese verde
        if (not self.degradado.gobierna_luz() and self.semaforo.estado in ("S_VERDE", "S_ROJO_AMARILLO")
                and (self.t - self.tUltimoComando) + AVISO_AMBAR_TIMEOUT_MS + AMARILLO_CIERRE_MS
                > SILENCIO_A_AMBAR_MS):
            self.semaforo.forzar_rojo()
            self.verde_soltado_por_margen = True
        if not self.degradado.gobierna_luz() and (self.t - self.tUltimoComando) > SILENCIO_A_AMBAR_MS:
            if self.semaforo.estado != "S_FALLO":
                self.semaforo.iniciar_fallo()
                self.verde_soltado_por_margen = False   # D-34: desde aqui manda SFTY-9

        luz = self.semaforo.estado
        if luz != self.estado_luz_ant:
            if luz == "S_VERDE":
                self.tInicioVerde = self.t
            self.estado_luz_ant = luz

        if getattr(self, "ack_rojo_pendiente", False) and self.semaforo.estado != "S_AMARILLO":
            if self.semaforo.estado == "S_ROJO":
                self.programar_respuesta(CMD["CMD_ACK_RED"])
            self.ack_rojo_pendiente = False

        if not self.degradado.gobierna_luz() and self.semaforo.estable() and \
                self.semaforo.estado == "S_VERDE" and not self.ack_verde_enviado:
            self.programar_respuesta(CMD["CMD_ACK_GREEN"])
            self.ack_verde_enviado = True

        # D-32 (1): aqui iba `if self.interfaz_arrancada: self.menu.loop()`.
        self.ambar.actualizar()

    # --- utilidades del banco --------------------------------------------
    def correr(self, ms, paso=10):
        n = max(1, int(ms // paso))
        for _ in range(n):
            self.loop(paso)

    def pedir_ambar_emergencia(self):
        """CMD:AMBAR_EMERGENCIA desde la app, y una vuelta para que la luz lo siga."""
        self.ambar.pedir()
        self.correr(50)

    def cancelar_ambar_emergencia(self):
        """CMD:CANCELAR_AMBAR desde la app."""
        self.ambar.cancelar()
        self.correr(50)

    def verde_encendido(self):
        return self.semaforo.pines[2]

    def acks_de_luz(self):
        return [c for (_, c, _) in self.tx if c in (CMD["CMD_ACK_RED"], CMD["CMD_ACK_GREEN"])]


# ==========================================================================
# 2. MODELO MINIMO DEL MAESTRO ESPERANDO UN ACK
# ==========================================================================
# Responde UNA pregunta: si el Esclavo calla, el Maestro cae a C_FALLO o espera para
# siempre. Dos vias independientes: a) reintentos del ACK (TIMEOUT_ACK_MS x REINTENTOS_MAX;
# D-53: la PRIMERA espera lleva el ROJO+AMARILLO dentro, coordinador.cpp) y b) el silencio
# total del Esclavo (MAESTRO_SIN_RX_MS). El resto del coordinador no se modela.
class MaestroEsperandoAck:
    def __init__(self, esclavo, ping_activo=True, limite_reintentos=True):
        self.esc = esclavo
        self.t = 0
        self.estado = "C_ESPERANDO_ACK_GREEN"
        self.tEsperandoAck = 0
        self.retry = 0
        # El enlace venia SANO: hasta que el operario armo el ambar, el Esclavo
        # contestaba los latidos con normalidad. Arrancar con tUltimaRx=0 y sin
        # historial haria caer al Maestro en el primer milisegundo y la prueba
        # mediria un arranque en frio, no la desobediencia del Esclavo.
        self.tUltimaRx = 0
        self.hubo_rx = True
        self.tUltimoPing = 0
        self.ping_activo = ping_activo
        self.limite_reintentos = limite_reintentos
        self.esc.rx.append((CMD["CMD_GO_GREEN"], 0))

    def loop(self, dt):
        self.t += dt
        # Lo que llega del Esclavo
        while self.esc.tx:
            _, cmd, _ = self.esc.tx.pop(0)
            self.tUltimaRx = self.t
            self.hubo_rx = True
            if self.estado == "C_ESPERANDO_ACK_GREEN" and cmd == CMD["CMD_ACK_GREEN"]:
                self.estado = "C_IDLE"

        # SFTY-13: el PING se SUPRIME mientras se espera un ACK, para no chocar
        # con el en el bus half-duplex. Es lo que hace que el silencio del
        # Esclavo se note: si el Maestro siguiera pingando, el PONG mantendria
        # vivo el enlace mientras la orden de luz se ignora.
        if self.ping_activo and self.estado not in ("C_ESPERANDO_ACK_GREEN",) and \
                (self.t - self.tUltimoPing) > LATIDO_MS:   # N-71: leido del C++
            self.tUltimoPing = self.t
            self.esc.rx.append((CMD["CMD_PING"], 0))

        # SFTY-6: la caida por silencio exige que ALGUNA VEZ se haya recibido algo
        # (o que hayan pasado ya los primeros 12 s), para no tumbar al Maestro
        # durante el arranque en frio.
        tiene_comunicacion = self.hubo_rx and (self.t - self.tUltimaRx) <= MAESTRO_SIN_RX_MS
        if not tiene_comunicacion and (self.hubo_rx or self.t > MAESTRO_SIN_RX_MS) and \
                self.estado != "C_FALLO":
            self.estado = "C_FALLO"

        espera = TIMEOUT_ACK_MS + (M_ROJO_AMARILLO_MS if self.retry == 0 else 0)   # D-53
        if self.estado == "C_ESPERANDO_ACK_GREEN" and (self.t - self.tEsperandoAck) > espera:
            self.retry += 1
            if self.limite_reintentos and self.retry >= REINTENTOS_MAX:
                self.estado = "C_FALLO"
            else:
                self.esc.rx.append((CMD["CMD_GO_GREEN"], 0))
                self.tEsperandoAck = self.t


# --------------------------------------------------------------------------
# ESCENARIOS DE PARTIDA compartidos por los packs.
#
# Viven con el modelo porque describen COMO SE PONE EL NODO en un estado.
# --------------------------------------------------------------------------
def preparar_nodo(**kw):
    """Esclavo en operacion normal: en hora, con ciclo configurado por radio y
    con una sincronizacion reciente. Es el punto de partida desde el que tiene
    sentido intentar romper algo."""
    e = Esclavo(**kw)
    e.reloj_en_hora = True
    e.poner_hora(12 * 3600)
    e.cfg_verde = 30
    e.cfg_despeje = 30
    e.cfg_verde_recibido = True
    e.cfg_despeje_recibido = True
    e.degradado.registrar_sync()
    e.tUltimoComando = e.t
    return e


def _llevar_a(e, estado):
    """Coloca el nodo en un estado de partida realista."""
    if estado == "rojo":
        e.rx.append((CMD["CMD_GO_RED"], 0))
        e.correr(1000)
    elif estado == "verde":
        e.rx.append((CMD["CMD_GO_GREEN"], 0))
        e.correr(AMARILLO_CIERRE_MS + 1000)
    elif estado == "degradado_activo":
        e.degradado.entrar()
        e.correr(e.config_despeje_segundos() * 1000 + 2000, paso=100)
    elif estado == "fallo":
        e.semaforo.iniciar_fallo()
        e.correr(1000)


# --------------------------------------------------------------------------
