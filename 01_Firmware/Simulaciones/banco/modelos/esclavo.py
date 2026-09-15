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
_ESC_MAIN = ("Esclavo", "src", "main.cpp")
_ESC_SEM = ("Esclavo", "src", "semaforo.cpp")
_ESC_DEG = ("Esclavo", "src", "modo_degradado.cpp")
_ESC_PROTO = ("Esclavo", "include", "protocolo.h")
_ESC_CICLO = ("Esclavo", "include", "ciclo_degradado.h")
_MAE_COORD = ("Maestro", "src", "coordinador.cpp")

# D-30 (14/09): AQUI SE LEIAN SEIS CONSTANTES DE mando.cpp -las dos ventanas, los
# tres recuentos de destellos y el ambar rapido de rechazo- y tres de la senal de
# semaforo.cpp. El mando salio del firmware con sus ficheros, asi que no habia fuente
# que leer: _fw.constante() ABORTA cuando no encuentra la constante -sin valor por
# defecto, nunca- y los cuatro packs que importan este modelo habrian caido en
# ABORTADO, que no dice nada del firmware.
AMARILLO_A_VERDE_MS = _fw.constante(_ESC_SEM, r"estado\s*==\s*S_AMARILLO\s*&&\s*\(ahora\s*-\s*tCambio\s*>=\s*(\d+)\)", "amarillo previo al verde")

# D-32 (1), 13/09: aqui se leian del menu.cpp del Esclavo INACTIVIDAD_MS (regreso
# automatico al listado), REFRESCO_MS (repintado periodico) y RECHAZO_MS (duracion del
# cartel de rechazo). Las tres eran de la NAVEGACION de la pantalla y se van con ella.
#
# ERAN LA CASCADA MAS CARA DE ESTE CAMBIO, y por eso queda escrito: se leian A NIVEL DE
# MODULO y sin valor por defecto, asi que en cuanto dejaran de estar en el C++ el propio
# `import banco.modelos.esclavo` habria lanzado Abortado y habria tumbado en ABORTADO a
# los CINCO packs que importan este modelo -esclavo_01, _02, _03, _04 y _05, 31
# comprobaciones-. ABORTADO no dice nada del firmware: habria sido una puerta abierta,
# no una casilla pendiente.

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
# Portado funcion a funcion del C++. Donde el modelo simplifica -los pines, el
# CRC, el reloj RTC- se dice en el comentario, para que nadie de por probado lo
# que aqui solo esta esbozado.

class Semaforo:
    """Puerto de src/semaforo.cpp.

    D-30 (14/09): de aqui salio la SENAL DE CONFIRMACION -destellos rojos y ambar
    rapido-, que interceptaba la escritura a los pines mientras duraba. Su unico
    armador era mando.cpp; retirado el mando, la bandera no podia volver a valer
    true y la interceptacion era un camino que nadie podia ejercer.
    """

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
        # SFTY-2: enclavamiento logico. El rojo siempre gana.
        if r:
            v = False
            a = False
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
        # R-4: CON UN AMBAR DE LA APP VIGENTE NO SE ENTRA AL DEGRADADO.
        #
        # Faltaba en este modelo, y hasta el 14/09 no se notaba: el A.B.A.B del mando
        # bajaba `ambarLocal` ANTES de llamar aqui, asi que por esa via la guarda nunca
        # se alcanzaba con el latch puesto. Retirado el mando, la UNICA via de entrada
        # es SET_MODO:DEGRADADO por app (D-18), que no revoca nada: pregunta, y con el
        # ambar puesto se lleva un DEG_RECHAZO_AMBAR_VIGENTE.
        #
        # La diferencia importa y por eso se modela: el mando REVOCABA el ambar del
        # operario para entrar -y si luego el modo se rechazaba, el equipo se quedaba
        # sin ambar y sin Degradado-; la app no puede dejar ese hueco.
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


# D-32 (1), 13/09: AQUI VIVIA `class Menu`, el puerto en Python de src/menu.cpp del
# Esclavo: las cinco pantallas (P_MENU, P_ESTADO, P_DEGRADADO, P_CONFIRMAR, P_RECHAZO),
# el cursor, el regreso automatico al listado y el repintado periodico. Se va entera
# porque su sujeto ya no existe: de menu.cpp solo queda menu_estaAbierto().
#
# LO QUE ESE MODELO SOSTENIA: le daba sujeto a esclavo_02_inhibicion_menu (7
# comprobaciones, `# EJERCE SFTY-21: el mando queda inhibido con el menu abierto`), que
# se retiro con ella.
#
# D-30 (14/09): Y LA FRASE QUE SEGUIA A ESTA -"SFTY-21 no se queda sin ejercicio:
# quedan once packs etiquetados"- YA NO ES CIERTA, asi que se corrige en vez de
# heredarse. Retirado el mando entero, lo que quedaba de SFTY-21 en esta punta es el
# LATCH DE AMBAR, y hoy lo arma la app en vez del gabinete: es `ambarEmergencia` de
# bluetooth.cpp, lo ejerce esclavo_01 sobre el modelo reapuntado y lo leen en el texto
# esclavo_07 y costura_14. Lo que NADIE ejerce ya -y se dice para que su ausencia no se
# lea como cobertura- son las secuencias de pulsos y la senal de destellos: no existen.


class AmbarEmergencia:
    """Puerto del latch de ambar de la app: `ambarEmergencia` de src/bluetooth.cpp.

    D-30 (14/09): AQUI VIVIA `class Mando`, el puerto de src/mando.cpp -el buffer de
    pulsos, las ventanas, las tres secuencias y el latch `ambar_local` que armaba
    B.B.B-. Se va con su fichero, y el latch NO se va con el: el firmware conserva el
    de la app, y son LAS MISMAS TRES GUARDAS de main.cpp las que lo leen, que hasta el
    14/09 preguntaban por los dos.

    Por eso este modelo se REAPUNTA en vez de borrarse. La propiedad que el banco
    ejerce -con un ambar pedido puesto, ninguna orden de luz por radio lo pisa ni se
    acusa- es la misma, sobre el unico latch que queda. Borrarlo habria dejado esa
    propiedad sin ningun instrumento que la EJECUTE: esclavo_07 la mira en el texto de
    las guardas, que es otra cosa.

    Lo que NO se modela, y se dice para que su ausencia no se lea como cobertura: el
    dialogo de CANCELAR_AMBAR con el Maestro (CMD_CANCELA_AMBAR_ESCLAVO, el plazo del
    acuse y su reenvio) vive en bluetooth.cpp y lo miden costura_14 y esclavo_07. Aqui
    solo esta el latch y su sostenedor, que es lo que decide la LUZ.
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

        Se RE-ARMA en vez de encenderse una sola vez porque la orden tiene que
        sobrevivir a lo que pase despues -al todo-rojo de salida del Degradado, que
        termina en INACTIVO y no en ambar-; un ambar que se apaga solo no es un estado
        seguro, es un parpadeo.

        Eran TRES guardas y hoy son dos: la tercera, `not senal_en_curso()`, protegia
        los destellos del mando y se fue con ellos.
        """
        n = self.nodo
        if self.armado and not n.degradado.gobierna_luz() and \
                n.semaforo.estado != "S_FALLO":
            n.semaforo.iniciar_fallo()


class Esclavo:
    """Puerto del loop() de src/main.cpp, con su misma secuencia de llamadas.

    El ORDEN importa y por eso se respeta: los flancos primero, luego las luces, la
    radio en medio y el SOSTENEDOR DEL AMBAR al final, que es lo que hace que la
    ultima palabra de cada vuelta sea del latch y no de una orden de radio que acabe
    de llegar. Hasta el 14/09 el que cerraba la vuelta era mando_actualizar(), en el
    mismo sitio y por el mismo motivo.
    """

    def __init__(self, obedece_ambar_emergencia=True):
        self.t = 0
        self.flanco = [False] * 4
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
                self.semaforo.forzar_rojo()
                self.programar_respuesta(CMD["CMD_ACK_RED"])
        elif cmd == CMD["CMD_GO_GREEN"]:
            # D-34: la repeticion (luz ya en ambar o verde) no refresca el silencio
            if self.semaforo.estado not in ("S_AMARILLO", "S_VERDE"):
                self.tUltimoComando = self.t
            self.verde_soltado_por_margen = False   # D-34
            if not self._ambar_emergencia():
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

        if not self._ambar_emergencia() and self.semaforo.estado == "S_FALLO" and cmd == CMD["CMD_GO_RED"]:
            self.semaforo.forzar_rojo()

    # --- bucle principal --------------------------------------------------
    def loop(self, dt=10):
        self.t += dt

        # botones_actualizar(): el mando ve los pulsos ANTES que ninguna pantalla
        # D-30 (14/09): aqui botones_actualizar() pasaba cada flanco de BOTON1/BOTON2
        # al reconocedor de secuencias del mando, ANTES que ninguna pantalla. El
        # firmware dejo de hacerlo el 14/09 y el modulo salio entero: los pines se
        # siguen leyendo, pero ya no alimentan nada que mueva la luz.

        self.semaforo.actualizar()
        self._atender_respuesta_pendiente()
        self._caducar_buffer_hora()
        self.degradado.actualizar()

        if self.rx:
            self._procesar(self.rx.pop(0))

        # D-34: el verde se suelta AVISO_AMBAR_TIMEOUT_MS antes que el ambar, directo a rojo
        if (not self.degradado.gobierna_luz() and self.semaforo.estado in ("S_VERDE", "S_AMARILLO")
                and (self.t - self.tUltimoComando) + AVISO_AMBAR_TIMEOUT_MS > SILENCIO_A_AMBAR_MS):
            self.semaforo.forzar_rojo()
            self.verde_soltado_por_margen = True
        if not self.degradado.gobierna_luz() and (self.t - self.tUltimoComando) > SILENCIO_A_AMBAR_MS:
            if self.semaforo.estado != "S_FALLO":
                self.semaforo.iniciar_fallo()
                self.verde_soltado_por_margen = False   # D-34: desde aqui manda SFTY-9

        luz = self.semaforo.estado
        if luz != self.estado_luz_ant:
            if luz in ("S_AMARILLO", "S_VERDE"):
                self.tInicioVerde = self.t
            self.estado_luz_ant = luz

        if not self.degradado.gobierna_luz() and self.semaforo.estable() and \
                self.semaforo.estado == "S_VERDE" and not self.ack_verde_enviado:
            self.programar_respuesta(CMD["CMD_ACK_GREEN"])
            self.ack_verde_enviado = True

        # D-32 (1): aqui iba `if self.interfaz_arrancada: self.menu.loop()`.
        self.ambar.actualizar()
        self.flanco = [False] * 4

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
