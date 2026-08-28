# ===== banco/modelos/esclavo.py =====
#
# MODELO DEL ESCLAVO — portado funcion a funcion del C++.
#
# POR QUE ESTA SEPARADO DE LAS PRUEBAS.
#
# Vivia dentro de validador_esclavo.py, mezclado con los cinco bloques de pruebas en
# un fichero de 1.805 lineas. Para cambiar UNA comprobacion habia que abrir las 1.805,
# y para reutilizar el modelo desde otro sitio, no habia forma.
#
# Es la misma separacion que el plan pide para el firmware: el modelo por un lado -lo
# que imita al C++- y las comprobaciones por otro -lo que se le exige-. Un pack de 150
# lineas se lee de una sentada; el modelo se toca solo cuando cambia el firmware.
#
# DONDE ESTE MODELO SIMPLIFICA SE DICE EN EL COMENTARIO, para que nadie de por probado
# lo que aqui solo esta esbozado: no hay pines, ni CRC, ni RTC de verdad.
#
# ⚠️ ESTE FICHERO ES UNA COPIA DEL FIRMWARE ESCRITA A MANO, y por tanto puede quedarse
# atras -es N-36 y N-39-. Cada constante que usa se RELEE del C++ en cada corrida, que
# es la unica parte que no puede desincronizarse. La logica, si.

from banco import fuente as _fw

# --------------------------------------------------------------------------
# Constantes leidas del firmware real (anti-deriva modelo/firmware).
# Si alguna no se puede leer, banco.fuente ABORTA: sin valor por defecto, nunca.
# --------------------------------------------------------------------------
_ESC_MANDO = ("Esclavo", "src", "mando.cpp")
_ESC_MENU = ("Esclavo", "src", "menu.cpp")
_ESC_MAIN = ("Esclavo", "src", "main.cpp")
_ESC_SEM = ("Esclavo", "src", "semaforo.cpp")
_ESC_DEG = ("Esclavo", "src", "modo_degradado.cpp")
_ESC_PROTO = ("Esclavo", "include", "protocolo.h")
_ESC_CICLO = ("Esclavo", "include", "ciclo_degradado.h")
_MAE_COORD = ("Maestro", "src", "coordinador.cpp")

VENTANA_TRIPLE_MS = _fw.constante(_ESC_MANDO, r"VENTANA_TRIPLE_MS\s*=\s*(\d+)", "ventana de A.A.A / B.B.B")
VENTANA_CUADRUPLE_MS = _fw.constante(_ESC_MANDO, r"VENTANA_CUADRUPLE_MS\s*=\s*(\d+)", "ventana de A.B.A.B")
DESTELLOS_OBEDECER = _fw.constante(_ESC_MANDO, r"DESTELLOS_OBEDECER\s*=\s*(\d+)", "destellos de A.A.A")
DESTELLOS_AMBAR = _fw.constante(_ESC_MANDO, r"DESTELLOS_AMBAR\s*=\s*(\d+)", "destellos de B.B.B")
DESTELLOS_DEGRADADO = _fw.constante(_ESC_MANDO, r"DESTELLOS_DEGRADADO\s*=\s*(\d+)", "destellos de A.B.A.B")
RECHAZO_AMBAR_MS = _fw.constante(_ESC_MANDO, r"RECHAZO_AMBAR_MS\s*=\s*(\d+)", "ambar rapido de rechazo")

DESTELLO_ON_MS = _fw.constante(_ESC_SEM, r"DESTELLO_ON_MS\s*=\s*(\d+)", "destello encendido")
DESTELLO_OFF_MS = _fw.constante(_ESC_SEM, r"DESTELLO_OFF_MS\s*=\s*(\d+)", "destello apagado")
AMBAR_RAPIDO_MS = _fw.constante(_ESC_SEM, r"AMBAR_RAPIDO_PERIODO_MS\s*=\s*(\d+)", "periodo del ambar rapido")
AMARILLO_A_VERDE_MS = _fw.constante(_ESC_SEM, r"estado\s*==\s*S_AMARILLO\s*&&\s*\(ahora\s*-\s*tCambio\s*>=\s*(\d+)\)", "amarillo previo al verde")

INACTIVIDAD_MS = _fw.constante(_ESC_MENU, r"INACTIVIDAD_MS\s*=\s*(\d+)", "regreso automatico al listado")
REFRESCO_MS = _fw.constante(_ESC_MENU, r"REFRESCO_MS\s*=\s*(\d+)", "repintado periodico")
RECHAZO_MS = _fw.constante(_ESC_MENU, r"RECHAZO_MS\s*=\s*(\d+)", "duracion del cartel de rechazo")

VENTANA_HORA_MS = _fw.constante(_ESC_MAIN, r"VENTANA_HORA_MS\s*=\s*(\d+)", "caducidad del buffer de hora")

# N-41: la ventana de vigencia del VERDE. El modelo NO la tenia -ni la constante ni la
# comprobacion- y por eso medía la conducta pegajosa de ANTES del arreglo. Se lee del
# C++ como todas las demas: si manana alguien cambia los 3 s en config_ciclo.cpp, el
# modelo cambia con el. Escribirla a mano aqui seria repetir la causa de N-36, N-39 y
# N-40 sabiendo ya cual es.
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

MANDO_A, MANDO_B = 0, 1


# ==========================================================================
# 1. MODELO DEL ESCLAVO
# ==========================================================================
# Portado funcion a funcion del C++. Donde el modelo simplifica -los pines, el
# CRC, el reloj RTC- se dice en el comentario, para que nadie de por probado lo
# que aqui solo esta esbozado.

class Semaforo:
    """Puerto de src/semaforo.cpp.

    Se modela con detalle porque el mando DEPENDE de el: la accion confirmada se
    ejecuta cuando terminan los destellos, y esos destellos son los que abren -o
    no- una ventana en la que el latch de ambar todavia no esta puesto.
    """

    def __init__(self, nodo):
        self.nodo = nodo
        self.estado = "S_ROJO"
        self.tCambio = 0
        self.senalActiva = False
        self.senalEsAmbar = False
        self.senalDestellos = 0
        self.senalEncendida = False
        self.tSenal = 0
        self.tSenalInicio = 0
        self.senalDuracion = 0
        self.pines = (False, False, False)   # rojo, amarillo, verde EN EL POSTE
        self.ult = (False, False, False)
        self.verde_en_pines_alguna_vez = False

    def _escribir_pines(self, r, a, v):
        self.pines = (r, a, v)
        if v:
            self.verde_en_pines_alguna_vez = True

    def _aplicar(self, r, a, v):
        # SFTY-2: enclavamiento logico. El rojo siempre gana.
        if r:
            v = False
            a = False
        elif v:
            r = False
        if r and v:
            v = False
        self.ult = (r, a, v)
        if self.senalActiva:
            return
        self._escribir_pines(r, a, v)

    def _terminar_senal(self):
        self.senalActiva = False
        self.senalDestellos = 0
        self.senalEsAmbar = False
        self._escribir_pines(*self.ult)

    def _actualizar_senal(self):
        ahora = self.nodo.t
        if self.senalEsAmbar:
            if ahora - self.tSenal >= AMBAR_RAPIDO_MS:
                self.tSenal = ahora
                self.senalEncendida = not self.senalEncendida
                self._escribir_pines(False, self.senalEncendida, False)
            if ahora - self.tSenalInicio >= self.senalDuracion:
                self._terminar_senal()
            return
        if self.senalEncendida:
            if ahora - self.tSenal >= DESTELLO_ON_MS:
                self.senalEncendida = False
                self._escribir_pines(False, False, False)
                self.tSenal = ahora
                if self.senalDestellos > 0:
                    self.senalDestellos -= 1
                if self.senalDestellos == 0:
                    self._terminar_senal()
        else:
            if ahora - self.tSenal >= DESTELLO_OFF_MS:
                self.senalEncendida = True
                self._escribir_pines(True, False, False)
                self.tSenal = ahora

    def destellos_rojos(self, n):
        if n == 0:
            return
        self.senalActiva = True
        self.senalEsAmbar = False
        self.senalDestellos = n
        self.senalEncendida = False
        self.tSenal = self.nodo.t
        self.tSenalInicio = self.tSenal
        self._escribir_pines(False, False, False)

    def ambar_rapido(self, ms):
        self.senalActiva = True
        self.senalEsAmbar = True
        self.senalDestellos = 0
        self.senalEncendida = True
        self.tSenal = self.nodo.t
        self.tSenalInicio = self.tSenal
        self.senalDuracion = ms
        self._escribir_pines(False, True, False)

    def senal_en_curso(self):
        return self.senalActiva

    def forzar_rojo(self):
        self.estado = "S_ROJO"
        self._aplicar(True, False, False)

    def iniciar_transicion_a_verde(self):
        self.estado = "S_AMARILLO"
        self.tCambio = self.nodo.t
        self._aplicar(False, True, False)

    def iniciar_fallo(self):
        self.estado = "S_FALLO"
        self.tCambio = self.nodo.t
        self._aplicar(False, False, False)

    def estable(self):
        return self.estado in ("S_ROJO", "S_VERDE", "S_FALLO")

    def actualizar(self):
        ahora = self.nodo.t
        if self.senalActiva:
            self._actualizar_senal()
        if self.estado == "S_AMARILLO" and (ahora - self.tCambio) >= AMARILLO_A_VERDE_MS:
            self.estado = "S_VERDE"
            self._aplicar(False, False, True)
        elif self.estado == "S_FALLO":
            if ahora - self.tCambio >= 500:
                self.tCambio = ahora
                self.nodo._ambar_status = not getattr(self.nodo, "_ambar_status", False)
                self._aplicar(False, self.nodo._ambar_status, False)


def ciclo_degradado_fase(seg_dia, verde, despeje):
    """Puerto literal de include/ciclo_degradado.h (identico en las dos puntas)."""
    if verde == 0 or despeje == 0:
        return "FD_DESPEJE_A"
    ciclo = 2 * (verde + despeje)
    if seg_dia < despeje:
        return "FD_DESPEJE_B"
    if SEGUNDOS_DEL_DIA - seg_dia <= despeje:
        return "FD_DESPEJE_B"
    pos = seg_dia % ciclo
    if pos < verde:
        return "FD_VERDE_MAESTRO"
    if pos < verde + despeje:
        return "FD_DESPEJE_A"
    if pos < 2 * verde + despeje:
        return "FD_VERDE_ESCLAVO"
    return "FD_DESPEJE_B"


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

    def _aplicar_luz(self, verde):
        if verde == self.verde_aplicado:
            return
        if verde:
            self.nodo.semaforo.iniciar_transicion_a_verde()
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
            if (ahora - self.tCambioEstado) >= self._rojo_obligatorio_ms() and \
                    self._calcular_fase() != "FD_VERDE_ESCLAVO":
                self.estado = "DEG_ACTIVO"
                self.tCambioEstado = ahora
        elif self.estado == "DEG_ACTIVO":
            self._aplicar_luz(self._calcular_fase() == "FD_VERDE_ESCLAVO")
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


class Menu:
    """Puerto de src/menu.cpp. Solo interesa la NAVEGACION, no lo que dibuja:
    la geometria la valida el arnes de 01_Firmware/Validacion_LCD."""

    N_OPCIONES = 2

    def __init__(self, nodo):
        self.nodo = nodo
        self.pantalla = "P_MENU"
        self.cursor = 0
        self.tRechazo = 0
        self.tRepintado = 0
        self.tUltimaPulsacion = 0
        self.repintados = 0

    def setup(self):
        self.pantalla = "P_MENU"
        self.cursor = 0
        self.tUltimaPulsacion = self.nodo.t
        self.tRepintado = self.nodo.t
        self.repintados += 1

    def esta_abierto(self):
        return self.pantalla != "P_MENU"

    def _ir_a(self, p):
        self.pantalla = p
        self.tRepintado = self.nodo.t
        self.repintados += 1

    def loop(self):
        n = self.nodo
        arriba = n.consumir_boton(0)
        abajo = n.consumir_boton(1)
        aceptar = n.consumir_boton(2)
        cancelar = n.consumir_boton(3)
        hay_pulsacion = arriba or abajo or aceptar or cancelar

        if hay_pulsacion:
            self.tUltimaPulsacion = n.t
        elif self.pantalla != "P_MENU" and (n.t - self.tUltimaPulsacion) >= INACTIVIDAD_MS:
            self.cursor = 0
            self._ir_a("P_MENU")
            return

        cambio = False
        if self.pantalla == "P_MENU":
            if arriba:
                self.cursor = (self.cursor + self.N_OPCIONES - 1) % self.N_OPCIONES
                cambio = True
            if abajo:
                self.cursor = (self.cursor + 1) % self.N_OPCIONES
                cambio = True
            if aceptar:
                self._ir_a("P_ESTADO" if self.cursor == 0 else "P_DEGRADADO")
                return
        elif self.pantalla == "P_ESTADO":
            if cancelar:
                self._ir_a("P_MENU")
                return
        elif self.pantalla == "P_DEGRADADO":
            if cancelar:
                self._ir_a("P_MENU")
                return
            if aceptar:
                if n.degradado.gobierna_luz():
                    n.degradado.salir()
                    cambio = True
                else:
                    self._ir_a("P_CONFIRMAR")
                    return
        elif self.pantalla == "P_CONFIRMAR":
            if cancelar:
                self._ir_a("P_DEGRADADO")
                return
            if aceptar:
                n.ultimo_rechazo = n.degradado.entrar()
                if n.ultimo_rechazo == "DEG_ACEPTADO":
                    self._ir_a("P_DEGRADADO")
                else:
                    self.tRechazo = n.t
                    self._ir_a("P_RECHAZO")
                return
        elif self.pantalla == "P_RECHAZO":
            if hay_pulsacion or (n.t - self.tRechazo) > RECHAZO_MS:
                self._ir_a("P_DEGRADADO")
                return

        vivo = self.pantalla in ("P_ESTADO", "P_DEGRADADO", "P_CONFIRMAR")
        if cambio or (vivo and (n.t - self.tRepintado) >= REFRESCO_MS):
            self.tRepintado = n.t
            self.repintados += 1


class Mando:
    """Puerto de src/mando.cpp."""

    MAX_PULSOS = 4

    def __init__(self, nodo, inhibicion_activa=True, guarda_ambar_en_verde=True):
        self.nodo = nodo
        self.sec = []          # (boton, instante)
        self.pendiente = None
        self.ambar_local = False
        self.rechazos = 0
        # Interruptores para los CONTROLES NEGATIVOS: permiten correr el mismo
        # modelo SIN una salvaguarda y exigir que la prueba lo cace. Si la prueba
        # no distingue las dos versiones, no esta midiendo la salvaguarda.
        self.inhibicion_activa = inhibicion_activa
        self.guarda_ambar_en_verde = guarda_ambar_en_verde

    def _limpiar(self):
        self.sec = []

    def _secuencias_inhibidas(self):
        return self.inhibicion_activa and self.nodo.menu.esta_abierto()

    def _confirmar_y_actuar(self, accion, destellos):
        self.nodo.semaforo.forzar_rojo()
        self.nodo.semaforo.destellos_rojos(destellos)
        self.pendiente = accion
        self._limpiar()

    def _rechazar(self):
        self.rechazos += 1
        self.nodo.semaforo.ambar_rapido(RECHAZO_AMBAR_MS)
        self._limpiar()

    def _purgar_viejos(self, ahora):
        self.sec = [p for p in self.sec if (ahora - p[1]) <= VENTANA_CUADRUPLE_MS]

    def registrar_pulso(self, boton):
        if self._secuencias_inhibidas():
            self._limpiar()
            return
        if self.nodo.semaforo.senal_en_curso() or self.pendiente is not None:
            return
        ahora = self.nodo.t
        self._purgar_viejos(ahora)
        if len(self.sec) >= self.MAX_PULSOS:
            self.sec = self.sec[1:]
        self.sec.append((boton, ahora))

        if len(self.sec) >= 4:
            u = self.sec[-4:]
            if [p[0] for p in u] == [MANDO_A, MANDO_B, MANDO_A, MANDO_B] and \
                    (ahora - u[0][1]) <= VENTANA_CUADRUPLE_MS:
                if self.nodo.degradado.comprobar() == "DEG_ACEPTADO":
                    self._confirmar_y_actuar("ACC_DEGRADADO", DESTELLOS_DEGRADADO)
                else:
                    self._rechazar()
                return
        if len(self.sec) >= 3:
            u = self.sec[-3:]
            if (ahora - u[0][1]) <= VENTANA_TRIPLE_MS:
                if [p[0] for p in u] == [MANDO_A] * 3:
                    self._confirmar_y_actuar("ACC_OBEDECER", DESTELLOS_OBEDECER)
                    return
                if [p[0] for p in u] == [MANDO_B] * 3:
                    self._confirmar_y_actuar("ACC_AMBAR", DESTELLOS_AMBAR)
                    return

    def _ejecutar(self, a):
        n = self.nodo
        if a == "ACC_OBEDECER":
            self.ambar_local = False
            if n.degradado.gobierna_luz():
                n.degradado.salir()
            else:
                n.semaforo.forzar_rojo()
        elif a == "ACC_AMBAR":
            self.ambar_local = True
            if n.degradado.gobierna_luz():
                n.degradado.salir()
            else:
                n.semaforo.iniciar_fallo()
        elif a == "ACC_DEGRADADO":
            self.ambar_local = False
            n.degradado.entrar()

    def actualizar(self):
        n = self.nodo
        if self.pendiente is not None and not n.semaforo.senal_en_curso():
            a = self.pendiente
            self.pendiente = None
            self._ejecutar(a)
        if self.ambar_local and not n.semaforo.senal_en_curso() and \
                (not self.guarda_ambar_en_verde or not n.degradado.gobierna_luz()) and \
                n.semaforo.estado != "S_FALLO":
            n.semaforo.iniciar_fallo()


class Esclavo:
    """Puerto del loop() de src/main.cpp, con su misma secuencia de llamadas.

    El ORDEN importa y por eso se respeta: los flancos primero, luego las luces,
    la radio en medio y el mando al final, que es lo que hace que la ultima
    palabra de cada vuelta sea del mando.
    """

    def __init__(self, obedece_ambar_local=True):
        self.t = 0
        self.flanco = [False] * 4
        self.semaforo = Semaforo(self)
        self.degradado = ModoDegradado(self)
        self.menu = Menu(self)
        self.mando = Mando(self)
        self.rx = []            # tramas que llegan del Maestro
        self.tx = []            # (instante, comando, param) que salen al aire
        self.respuesta_pendiente = None
        self.respuesta_param = 0
        self.tEnviarRespuesta = 0
        self.tUltimoComando = 0
        self.ack_verde_enviado = False
        self.tInicioVerde = 0
        self.estado_luz_ant = "S_ROJO"
        self.ultimo_rechazo = "DEG_ACEPTADO"
        self.interfaz_arrancada = True
        self.menu.setup()

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

        # Interruptor del CONTROL NEGATIVO del bloque 1.
        self.obedece_ambar_local = obedece_ambar_local

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

    # --- botones ---------------------------------------------------------
    def consumir_boton(self, idx):
        v = self.flanco[idx]
        self.flanco[idx] = False
        return v

    def pulsar(self, idx, paso=10):
        self.flanco[idx] = True
        self.loop(paso)

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

    def _ambar_local(self):
        # El interruptor solo existe para el control negativo: con el a False se
        # modela un firmware SIN la desobediencia, y la prueba tiene que cazarlo.
        return self.mando.ambar_local and self.obedece_ambar_local

    def _procesar(self, pkt):
        cmd, param = pkt
        if self.degradado.gobierna_luz() and cmd in (CMD["CMD_PING"], CMD["CMD_GO_RED"], CMD["CMD_GO_GREEN"]):
            self.degradado.salir()

        if cmd == CMD["CMD_PING"]:
            if self.semaforo.estado != "S_FALLO":
                self.tUltimoComando = self.t
            self.programar_respuesta(CMD["CMD_PONG"])
        elif cmd == CMD["CMD_GO_RED"]:
            self.tUltimoComando = self.t
            if not self._ambar_local():
                self.semaforo.forzar_rojo()
                self.programar_respuesta(CMD["CMD_ACK_RED"])
        elif cmd == CMD["CMD_GO_GREEN"]:
            self.tUltimoComando = self.t
            if not self._ambar_local():
                self.semaforo.iniciar_transicion_a_verde()
                self.ack_verde_enviado = False
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

        if not self._ambar_local() and self.semaforo.estado == "S_FALLO" and cmd == CMD["CMD_GO_RED"]:
            self.semaforo.forzar_rojo()

    # --- bucle principal --------------------------------------------------
    def loop(self, dt=10):
        self.t += dt

        # botones_actualizar(): el mando ve los pulsos ANTES que ninguna pantalla
        if self.flanco[0]:
            self.mando.registrar_pulso(MANDO_A)
        if self.flanco[1]:
            self.mando.registrar_pulso(MANDO_B)

        self.semaforo.actualizar()
        self._atender_respuesta_pendiente()
        self._caducar_buffer_hora()
        self.degradado.actualizar()

        if self.rx:
            self._procesar(self.rx.pop(0))

        if not self.degradado.gobierna_luz() and (self.t - self.tUltimoComando) > SILENCIO_A_AMBAR_MS:
            if self.semaforo.estado != "S_FALLO":
                self.semaforo.iniciar_fallo()

        luz = self.semaforo.estado
        if luz != self.estado_luz_ant:
            if luz in ("S_AMARILLO", "S_VERDE"):
                self.tInicioVerde = self.t
            self.estado_luz_ant = luz

        if not self.degradado.gobierna_luz() and self.semaforo.estable() and \
                self.semaforo.estado == "S_VERDE" and not self.ack_verde_enviado:
            self.programar_respuesta(CMD["CMD_ACK_GREEN"])
            self.ack_verde_enviado = True

        if self.interfaz_arrancada:
            self.menu.loop()

        self.mando.actualizar()
        self.flanco = [False] * 4

    # --- utilidades del banco --------------------------------------------
    def correr(self, ms, paso=10):
        n = max(1, int(ms // paso))
        for _ in range(n):
            self.loop(paso)

    def secuencia(self, botones, separacion=2000):
        """Acciona el mando como lo hace el operario: un pulso cada ~2 s, que es
        lo que tarda el rele en conmutar (medido en campo, ver mando.h)."""
        for b in botones:
            self.pulsar(b)
            self.correr(separacion)

    def verde_encendido(self):
        return self.semaforo.pines[2]

    def acks_de_luz(self):
        return [c for (_, c, _) in self.tx if c in (CMD["CMD_ACK_RED"], CMD["CMD_ACK_GREEN"])]


# ==========================================================================
# 2. MODELO MINIMO DEL MAESTRO ESPERANDO UN ACK
# ==========================================================================
# Solo se modela la parte que responde a la pregunta del banco: si el Esclavo
# calla, el Maestro cae a C_FALLO o se queda esperando para siempre. Las dos
# vias que lo tumban son independientes y por eso van las dos:
#
#   a) el contador de reintentos del ACK (TIMEOUT_ACK_MS x REINTENTOS_MAX)
#   b) el silencio total del Esclavo (MAESTRO_SIN_RX_MS)
#
# Todo lo demas del coordinador -el ciclo, la sincronizacion, la telemetria- no
# se toca: no se pretende validar el Maestro aqui, solo comprobar que la
# desobediencia del Esclavo termina en un final acotado.
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

        if self.estado == "C_ESPERANDO_ACK_GREEN" and (self.t - self.tEsperandoAck) > TIMEOUT_ACK_MS:
            self.retry += 1
            if self.limite_reintentos and self.retry >= REINTENTOS_MAX:
                self.estado = "C_FALLO"
            else:
                self.esc.rx.append((CMD["CMD_GO_GREEN"], 0))
                self.tEsperandoAck = self.t


# --------------------------------------------------------------------------
# ESCENARIOS DE PARTIDA compartidos por los packs.
#
# Viven con el modelo y no con las pruebas porque describen COMO SE PONE EL NODO
# en un estado, que es parte de imitar al firmware. Que los cinco bloques
# arrancaran de aqui era una dependencia oculta del fichero monolitico; ahora es
# una importacion explicita.
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
        e.correr(AMARILLO_A_VERDE_MS + 1000)
    elif estado == "degradado_activo":
        e.degradado.entrar()
        e.correr(e.config_despeje_segundos() * 1000 + 2000, paso=100)
    elif estado == "fallo":
        e.semaforo.iniciar_fallo()
        e.correr(1000)


# --------------------------------------------------------------------------
