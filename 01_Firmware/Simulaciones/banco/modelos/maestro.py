# ===== banco/modelos/maestro.py =====
#
# MODELO DEL MANDO Y DEL SEMAFORO DEL MAESTRO — port fiel de mando.cpp + semaforo.cpp,
# mas el respaldo, la puerta del Degradado y la sincronizacion horaria.
#
# Vivia dentro de validador_maestro.py, mezclado con los cinco bloques de pruebas en un
# fichero de 2.106 lineas: el mayor del banco. Para cambiar UNA comprobacion habia que
# abrir las 2.106.
#
# Se porta la logica REAL, no una idealizacion: el buffer deslizante con su
# desplazamiento, purgarViejos() con la ventana CUADRUPLE, el plegado de 32 a 16 bits
# del checksum. Donde simplifica, el comentario lo dice.
#
# ⚠️ ES UNA COPIA DEL FIRMWARE ESCRITA A MANO y puede quedarse atras -es N-36 y N-39-.
# Las constantes se RELEEN del C++ en cada corrida; la logica, no. Esa es justo la
# asimetria que la V8.8 existe para reducir.

import math
import random
import re

from banco import fuente as _fw


def cte(partes, patron, base=10):
    """Constante leida del C++. ABORTA si no aparece: sin valor por defecto, nunca."""
    return _fw.constante(partes, patron, patron, base=base)


# --- mando.cpp (SFTY-21) ---------------------------------------------------
MANDO = ("Maestro", "src", "mando.cpp")
VENTANA_TRIPLE_MS = cte(MANDO, r"VENTANA_TRIPLE_MS\s*=\s*(\d+)")
VENTANA_CUADRUPLE_MS = cte(MANDO, r"VENTANA_CUADRUPLE_MS\s*=\s*(\d+)")
DESTELLOS_AUTOMATICO = cte(MANDO, r"DESTELLOS_AUTOMATICO\s*=\s*(\d+)")
DESTELLOS_AMBAR = cte(MANDO, r"DESTELLOS_AMBAR\s*=\s*(\d+)")
DESTELLOS_DEGRADADO = cte(MANDO, r"DESTELLOS_DEGRADADO\s*=\s*(\d+)")
RECHAZO_AMBAR_MS = cte(MANDO, r"RECHAZO_AMBAR_MS\s*=\s*(\d+)")
MAX_PULSOS = cte(MANDO, r"MAX_PULSOS\s*=\s*(\d+)")

# --- semaforo.cpp ----------------------------------------------------------
SEM = ("Maestro", "src", "semaforo.cpp")
DESTELLO_ON_MS = cte(SEM, r"DESTELLO_ON_MS\s*=\s*(\d+)")
DESTELLO_OFF_MS = cte(SEM, r"DESTELLO_OFF_MS\s*=\s*(\d+)")
AMBAR_RAPIDO_PERIODO_MS = cte(SEM, r"AMBAR_RAPIDO_PERIODO_MS\s*=\s*(\d+)")
AMBAR_FALLO_PERIODO_MS = cte(SEM, r"ahora\s*-\s*tCambio\s*>=\s*(\d+)\)\s*\{\s*\n\s*tCambio")

# --- botones.cpp -----------------------------------------------------------
BOT = ("Maestro", "src", "botones.cpp")
FLANCO_MS = cte(BOT, r"FLANCO_MS\s*=\s*(\d+)")

# --- modo_degradado.cpp (la puerta) ----------------------------------------
DEG = ("Maestro", "src", "modo_degradado.cpp")
SYNC_FRESCA_MS = cte(DEG, r"SYNC_FRESCA_MS\s*=\s*(\d+)")
TOLERANCIA_DESFASE_S = cte(DEG, r"TOLERANCIA_DESFASE_S\s*=\s*(\d+)")
LIMITE_DURO_MS = cte(DEG, r"LIMITE_DURO_MS\s*=\s*(\d+)")
AVISO_LIMITE_MS = cte(DEG, r"AVISO_LIMITE_MS\s*=\s*(\d+)")
DEG_VERDE_SEG = cte(DEG, r"DEG_VERDE_SEG\s*=\s*(\d+)")
DEG_DESPEJE_SEG = cte(DEG, r"DEG_DESPEJE_SEG\s*=\s*(\d+)")
# El firmware lo DERIVA en vez de escribir un 48 aparte; aqui se deriva igual, por
# el mismo motivo: dos numeros que deben ser el mismo acaban siendo distintos.
LIMITE_DURO_H = LIMITE_DURO_MS // 3600000

# --- coordinador.cpp (SFTY-23 y enlace) ------------------------------------
COORD = ("Maestro", "src", "coordinador.cpp")
TIMEOUT_ACK_MS = cte(COORD, r"TIMEOUT_ACK_MS\s*=\s*(\d+)")
SYNC_MAX_INTENTOS = cte(COORD, r"SYNC_MAX_INTENTOS\s*=\s*(\d+)")
BACKOFF_SYNC_MS = cte(COORD, r"BACKOFF_SYNC_MS\s*=\s*(\d+)")
INTERVALO_SYNC_MS = cte(COORD, r"INTERVALO_SYNC_MS\s*=\s*(\d+)")
VIGENCIA_DESFASE_MS = cte(COORD, r"VIGENCIA_DESFASE_MS\s*=\s*(\d+)")
VIGILANCIA_RELOJ_MS = cte(COORD, r"VIGILANCIA_RELOJ_MS\s*=\s*(\d+)")
# N-71: se leia el literal de DENTRO del if -"millis() - tUltimoPing > 3000"-. Al
# darle nombre en el C++ el patron dejo de encajar y este modelo ABORTO, que es su
# trabajo: sin la constante estaria modelando una cadencia inventada.
LATIDO_MS = cte(COORD, r"LATIDO_MS\s*=\s*(\d+)")
# N-69: el umbral vive ahora una sola vez en el contrato compartido, para que las dos
# puntas no puedan divergir. Se lee de ahi, que es donde esta la verdad. Y el dia del
# cambio este modelo ABORTO en vez de seguir midiendo el numero viejo, que es
# exactamente lo que se le pide a un instrumento cuando el fuente se mueve.
ORFANDAD_MS = cte(("Maestro", "include", "protocolo.h"),
                  r"#define\s+SFTY6_SILENCIO_MS\s+(\d+)UL")

# --- respaldo.cpp / respaldo.h (N-20) --------------------------------------
RESP = ("Maestro", "src", "respaldo.cpp")
FIRMA = cte(RESP, r"FIRMA\s*=\s*0x([0-9A-Fa-f]+)", 16)
FLAG_CICLO = cte(RESP, r"FLAG_CICLO\s*=\s*0x([0-9A-Fa-f]+)", 16)
FLAG_SYNC = cte(RESP, r"FLAG_SYNC\s*=\s*0x([0-9A-Fa-f]+)", 16)
FLAG_DEGRADADO = cte(RESP, r"FLAG_DEGRADADO\s*=\s*0x([0-9A-Fa-f]+)", 16)
SEMILLA_SUMA = cte(RESP, r"uint32_t\s+s\s*=\s*0x([0-9A-Fa-f]+)U", 16)
# N-49: ya no hay "/2" que leer. El fechado guarda el contador del RTC en dos
# mitades, y lo que se relee del C++ es el DESPLAZAMIENTO de la mitad alta: si
# alguien lo cambia, el modelo aborta en vez de trocear distinto en silencio.
DESPL_ALTA = cte(RESP, r"segundosRtc\s*>>\s*(\d+)")
SYNC_CADUCADA = cte(("Maestro", "include", "respaldo.h"),
                    r"RESPALDO_SYNC_CADUCADA\s*=\s*0x([0-9A-Fa-f]+)UL", 16)

# --- protocolo.h -----------------------------------------------------------
DELTA_FUERA_DE_RANGO = -cte(("Maestro", "include", "protocolo.h"),
                            r"DELTA_FUERA_DE_RANGO\s*\(\(int8_t\)-(\d+)\)")

SEGUNDOS_DEL_DIA = cte(("Maestro", "include", "ciclo_degradado.h"),
                       r"SEGUNDOS_DEL_DIA\s*=\s*(\d+)UL")

MANDO_A, MANDO_B = 0, 1
UINT32 = 1 << 32

def resumen_constantes():
    """Imprime lo leido del C++ en esta corrida.

    Era codigo a nivel de modulo y se disparaba al IMPORTAR, colandose a mitad
    de la salida de cualquier pack que usara el modelo. Informar es util; hacerlo
    sin que nadie lo pida, no."""
    print("=" * 78)
    print("  VALIDADOR DEL MAESTRO  --  constantes leidas del C++ en esta ejecucion")
    print("=" * 78)
    print(f"   mando     : triple={VENTANA_TRIPLE_MS} ms  cuadruple={VENTANA_CUADRUPLE_MS} ms  "
          f"buffer={MAX_PULSOS}")
    print(f"   destellos : auto={DESTELLOS_AUTOMATICO}  ambar={DESTELLOS_AMBAR}  "
          f"degradado={DESTELLOS_DEGRADADO}  on/off={DESTELLO_ON_MS}/{DESTELLO_OFF_MS} ms")
    print(f"   puerta    : fresca={SYNC_FRESCA_MS} ms  tolerancia=+-{TOLERANCIA_DESFASE_S} s  "
          f"limite={LIMITE_DURO_MS} ms ({LIMITE_DURO_H} h)")
    print(f"   sync      : timeout={TIMEOUT_ACK_MS} ms  intentos={SYNC_MAX_INTENTOS}  "
          f"latido={LATIDO_MS} ms  orfandad={ORFANDAD_MS} ms")
    print(f"   respaldo  : firma=0x{FIRMA:04X}  semilla=0x{SEMILLA_SUMA:04X}  "
          f"caducada=0x{SYNC_CADUCADA:08X}")


# ==========================================================================
# 1. MODELO DEL MANDO Y DEL SEMAFORO  (port fiel de mando.cpp + semaforo.cpp)
# ==========================================================================
#
# Se porta la logica REAL, no una idealizacion. En particular:
#   - el buffer deslizante de MAX_PULSOS con su desplazamiento
#   - purgarViejos() con la ventana CUADRUPLE para todos los pulsos
#   - el orden A.B.A.B antes que los triples, y el return de cada rama
#   - el bloqueo de pulsos mientras hay senal en curso o accion pendiente
#   - la senal del semaforo, que solo avanza si alguien llama a
#     semaforo_actualizar(); esto ultimo es la clave del bloque 1.7

ACC_NINGUNA, ACC_AUTOMATICO, ACC_AMBAR, ACC_DEGRADADO = 0, 1, 2, 3
NOMBRE_ACC = {ACC_NINGUNA: "-", ACC_AUTOMATICO: "AUTOMATICO",
              ACC_AMBAR: "AMBAR", ACC_DEGRADADO: "DEGRADADO"}


class Semaforo:
    """Port de semaforo.cpp limitado a lo que el mando toca: la senal que ocupa
    las salidas y el enclavamiento rojo/verde."""

    def __init__(self):
        self.pines = (0, 0, 0)      # (rojo, ambar, verde) escritos de verdad
        self.ult = (1, 0, 0)        # lo que la logica normal quiere
        self.senal_activa = False
        self.senal_es_ambar = False
        self.senal_destellos = 0
        self.senal_encendida = False
        self.t_senal = 0
        self.t_senal_inicio = 0
        self.senal_duracion = 0
        self.destellos_vistos = 0   # instrumentacion del banco, no del firmware

    def aplicar_salidas(self, r, a, v):
        # SFTY-2: enclavamiento logico. El rojo siempre gana.
        if r:
            v = 0
        elif v:
            r = 0
        self.ult = (r, a, v)
        if self.senal_activa:
            return
        self.pines = (r, a, v)

    def forzar_rojo(self):
        self.aplicar_salidas(1, 0, 0)

    def destellos_rojos(self, n):
        if n == 0:
            return
        self.senal_activa = True
        self.senal_es_ambar = False
        self.senal_destellos = n
        self.senal_encendida = False
        self.t_senal = self.ahora
        self.t_senal_inicio = self.ahora
        self.pines = (0, 0, 0)

    def ambar_rapido(self, ms):
        self.senal_activa = True
        self.senal_es_ambar = True
        self.senal_destellos = 0
        self.senal_encendida = True
        self.t_senal = self.ahora
        self.t_senal_inicio = self.ahora
        self.senal_duracion = ms
        self.pines = (0, 1, 0)

    def _terminar_senal(self):
        self.senal_activa = False
        self.senal_destellos = 0
        self.senal_es_ambar = False
        self.pines = self.ult

    def actualizar(self, ahora):
        """semaforo_actualizar(). SOLO avanza si alguien la llama: ese es
        exactamente el punto que el bloque 1.7 pone a prueba."""
        self.ahora = ahora
        if not self.senal_activa:
            return
        if self.senal_es_ambar:
            if ahora - self.t_senal >= AMBAR_RAPIDO_PERIODO_MS:
                self.t_senal = ahora
                self.senal_encendida = not self.senal_encendida
                self.pines = (0, 1 if self.senal_encendida else 0, 0)
            if ahora - self.t_senal_inicio >= self.senal_duracion:
                self._terminar_senal()
            return
        if self.senal_encendida:
            if ahora - self.t_senal >= DESTELLO_ON_MS:
                self.senal_encendida = False
                self.pines = (0, 0, 0)
                self.t_senal = ahora
                if self.senal_destellos > 0:
                    self.senal_destellos -= 1
                if self.senal_destellos == 0:
                    self._terminar_senal()
        else:
            if ahora - self.t_senal >= DESTELLO_OFF_MS:
                self.senal_encendida = True
                self.pines = (1, 0, 0)   # ROJO: nunca verde para confirmar
                self.t_senal = ahora
                self.destellos_vistos += 1


class Mando:
    """Port de mando.cpp. `puerta_ok` decide que contesta
    modo_degradado_evaluarEntrada(); `inhibido` modela secuenciasInhibidas()."""

    def __init__(self, sem, puerta_ok=True):
        self.sem = sem
        self.puerta_ok = puerta_ok
        self.boton = [0] * MAX_PULSOS
        self.tiempo = [0] * MAX_PULSOS
        self.n = 0
        self.pendiente = ACC_NINGUNA
        self.inhibido = False
        self.ejecutadas = []          # instrumentacion: [(instante, accion)]
        self.rechazos = 0
        self.ignorados = 0            # pulsos descartados por estar ocupado

    def _limpiar(self):
        self.n = 0

    def _purgar(self, ahora):
        primero = 0
        while primero < self.n and (ahora - self.tiempo[primero]) > VENTANA_CUADRUPLE_MS:
            primero += 1
        if primero == 0:
            return
        for i in range(primero, self.n):
            self.boton[i - primero] = self.boton[i]
            self.tiempo[i - primero] = self.tiempo[i]
        self.n -= primero

    def _confirmar_y_actuar(self, accion, destellos):
        # coordinador_forzarRojoTotal() + destellos. Todo-rojo antes de nada.
        self.sem.forzar_rojo()
        self.sem.destellos_rojos(destellos)
        self.pendiente = accion
        self._limpiar()

    def _rechazar(self):
        self.rechazos += 1
        self.sem.ambar_rapido(RECHAZO_AMBAR_MS)
        self._limpiar()

    def registrar_pulso(self, boton, ahora):
        if self.inhibido:
            self._limpiar()
            return
        if self.sem.senal_activa or self.pendiente != ACC_NINGUNA:
            self.ignorados += 1
            return

        self._purgar(ahora)

        if self.n >= MAX_PULSOS:
            for i in range(1, MAX_PULSOS):
                self.boton[i - 1] = self.boton[i]
                self.tiempo[i - 1] = self.tiempo[i]
            self.n = MAX_PULSOS - 1

        self.boton[self.n] = boton
        self.tiempo[self.n] = ahora
        self.n += 1
        n = self.n

        # A.B.A.B primero, tal cual en el C++.
        if n >= 4:
            if (self.boton[n - 4] == MANDO_A and self.boton[n - 3] == MANDO_B and
                    self.boton[n - 2] == MANDO_A and self.boton[n - 1] == MANDO_B and
                    (ahora - self.tiempo[n - 4]) <= VENTANA_CUADRUPLE_MS):
                if self.puerta_ok:
                    self._confirmar_y_actuar(ACC_DEGRADADO, DESTELLOS_DEGRADADO)
                else:
                    self._rechazar()
                return

        if n >= 3:
            tramo = ahora - self.tiempo[n - 3]
            if tramo <= VENTANA_TRIPLE_MS:
                if (self.boton[n - 3] == MANDO_A and self.boton[n - 2] == MANDO_A and
                        self.boton[n - 1] == MANDO_A):
                    self._confirmar_y_actuar(ACC_AUTOMATICO, DESTELLOS_AUTOMATICO)
                    return
                if (self.boton[n - 3] == MANDO_B and self.boton[n - 2] == MANDO_B and
                        self.boton[n - 1] == MANDO_B):
                    self._confirmar_y_actuar(ACC_AMBAR, DESTELLOS_AMBAR)
                    return

    def actualizar(self):
        """mando_actualizar(), al FINAL del loop principal."""
        if self.pendiente == ACC_NINGUNA:
            return
        if self.sem.senal_activa:
            return
        a = self.pendiente
        self.pendiente = ACC_NINGUNA
        self.ejecutadas.append((self.sem.ahora, a))


def correr_tren(tren, cadencia_ms, puerta_ok=True, bombea=True, ms_extra=40000,
                paso_ms=10):
    """Ejecuta un tren de pulsos sobre el modelo completo y devuelve las acciones
    ejecutadas y el estado final.

    `bombea` = si el modo activo llama a semaforo_actualizar() en cada iteracion.
    Es el parametro que distingue un modo normal del asistente de configuracion
    del Modo Automatico (ver bloque 1.7)."""
    sem = Semaforo()
    sem.ahora = 0
    m = Mando(sem, puerta_ok)
    instantes = {i * cadencia_ms: b for i, b in enumerate(tren)}
    fin = (len(tren) - 1) * cadencia_ms + ms_extra
    t = 0
    while t <= fin:
        sem.ahora = t
        if t in instantes:
            m.registrar_pulso(instantes[t], t)
        if bombea:
            sem.actualizar(t)
        m.actualizar()
        t += paso_ms
    return m, sem


def accion_de(tren, cadencia_ms=2000, **kw):
    """Primera accion ejecutada por el tren, que es la que el operario ve."""
    m, _ = correr_tren(tren, cadencia_ms, **kw)
    return m.ejecutadas[0][1] if m.ejecutadas else ACC_NINGUNA


def trenes(longitud):
    """Todos los trenes posibles de esa longitud sobre {A,B}."""
    if longitud == 0:
        yield []
        return
    for resto in trenes(longitud - 1):
        yield resto + [MANDO_A]
        yield resto + [MANDO_B]


def txt(tren):
    return "".join("A" if b == MANDO_A else "B" for b in tren)


# --------------------------------------------------------------------------
# LECTORES DEL FUENTE, con los nombres que usaban los bloques.
# Delegan en banco.fuente: una sola implementacion para todo el banco.
# --------------------------------------------------------------------------
def _ruta(*partes):
    return _fw.ruta(*partes)


def _fuente(*partes):
    return _fw.texto(*partes)


def _codigo(*partes):
    """El fuente SIN comentarios. Sin esto, un patron puede acertar dentro de un
    comentario y dar por presente una guarda que no se compila: ya paso una vez."""
    return _fw.codigo(*partes)


# --------------------------------------------------------------------------
# DERIVA DEL CRISTAL — dato fisico, no de un bloque concreto.
#
# Estaba definida DENTRO del bloque 3 y la usaban el 4 y el 5: un acoplamiento oculto
# que el fichero unico permitia y que al partirlo aparece de golpe. Es exactamente la
# clase de dependencia invisible que justifica la separacion.
# --------------------------------------------------------------------------
DERIVA_PEOR_S_DIA = 8.6


# --------------------------------------------------------------------------
# EL RESPALDO EN PILA: registros, checksum y sus ports.
#
# Estaba dentro del BLOQUE 2 y lo usaba tambien el BLOQUE 3. Como el validador era un
# unico fichero a nivel de modulo, la dependencia no se veia: bastaba con que el
# bloque 2 se hubiera ejecutado antes. Al partirlo aparecieron NUEVE dependencias
# cruzadas como esta. Es la clase de acoplamiento por el que un cambio en una esquina
# rompe otra sin relacion aparente -que es, literalmente, el diagnostico que abrio
# todo este trabajo-.
#
# El ORDEN de los registros se lee del C++ y NO ES DECORATIVO: calcularSuma() es un
# hash de Horner (s = s*31 + reg), no una suma. Con una suma, dos registros
# intercambiados dan el mismo resultado y la corrupcion pasa desapercibida.
# --------------------------------------------------------------------------
# LOS NUMEROS DE REGISTRO SE LEEN DEL C++, NO SE COPIAN (04/09).
#
# Aqui habia cinco numeros escritos a mano y un diccionario de nombres al lado. Al
# anadir los dos registros de N-133 -los tiempos del ciclo automatico- CINCO packs
# cayeron a ABORTADO con un KeyError, porque el mapa no los conocia. El ABORTADO fue
# correcto -grito en vez de medir otra cosa-, pero la causa era esta copia a mano: es
# el mismo motivo por el que los pesos y el orden ya se leian del fuente unas lineas
# mas abajo. Ahora el mapa entero sale del C++ y anadir un registro no rompe nada.
_REG_DEF = dict((m.group(1), int(m.group(2))) for m in re.finditer(
    r"static const uint8_t\s+(REG_\w+)\s*=\s*(\d+)\s*;",
    _codigo("Maestro", "src", "respaldo.cpp")))
if not _REG_DEF:
    raise _fw.Abortado("no se hallan los REG_* en respaldo.cpp: sin el mapa de "
                       "registros el modelo mediria sobre numeros inventados")

REG_VERDE     = _REG_DEF["REG_VERDE"]
REG_DESPEJE   = _REG_DEF["REG_DESPEJE"]
REG_FLAGS     = _REG_DEF["REG_FLAGS"]
REG_SYNC_ALTA = _REG_DEF["REG_SYNC_ALTA"]
REG_SYNC_BAJA = _REG_DEF["REG_SYNC_BAJA"]
NOMBRE_REG = dict((num, nom[4:]) for nom, num in _REG_DEF.items())

# LOS PESOS SE LEEN DEL C++, NO SE COPIAN.
#
# Tras el arreglo del 01/08/2026 calcularSuma() pondera cada registro por una
# constante. Escribirlas aqui a mano seria repetir el error que este banco existe
# para evitar: el dia que alguien cambie un peso, el modelo seguiria validando el
# algoritmo viejo y diria PASS sobre un firmware que hace otra cosa. Se extraen de
# la propia funcion.
#
# N-51: el tipo de retorno paso de uint16_t a uint32_t al dejar de plegar el
# resultado a 16 bits (ver mas abajo). Si alguien lo vuelve a plegar sin tocar
# este regex, el modelo aborta en vez de seguir midiendo el algoritmo viejo.
_cuerpo_suma = re.search(r"static uint32_t calcularSuma\(\)\s*\{(.*?)\n\}",
                         _codigo("Maestro", "src", "respaldo.cpp"), re.S)
if not _cuerpo_suma:
    # Se LANZA en vez de sys.exit(): dentro del corredor de packs, matar el proceso se
    # llevaria por delante a los otros diecinueve, que ya no dirian nada de nada. Es lo
    # que correr.py existe para impedir, incumplido desde dentro del modelo.
    raise _fw.Abortado("no se pudo aislar calcularSuma() en respaldo.cpp")
# ORDEN de los registros dentro de calcularSuma(). Se lee del C++ y el ORDEN IMPORTA:
# el algoritmo es un hash de Horner (s = s*31 + reg), no una suma. Con una suma, dos
# registros intercambiados dan el mismo resultado y la corrupcion pasa desapercibida;
# con Horner, no. Modelarlo como suma seria validar contra el algoritmo ANTERIOR.
ORDEN_SUMA = []
for _n in re.findall(r"leerReg\((REG_\w+)\)", _cuerpo_suma.group(1)):
    if _n not in _REG_DEF:
        raise _fw.Abortado(f"calcularSuma() usa {_n}, que no esta definido como "
                           "registro en respaldo.cpp")
    _r = _REG_DEF[_n]
    if _r not in ORDEN_SUMA:
        ORDEN_SUMA.append(_r)

# LA PROPIEDAD NO ES "SON CINCO": ES "ESTAN TODOS LOS QUE GUARDAN CONTENIDO".
#
# Antes se exigia el numero 5 a secas, y ese numero envejecio en cuanto N-133 anadio
# dos registros. Lo que de verdad importa es que ningun registro de CONTENIDO quede
# fuera del checksum: uno que no entre en la suma se puede cambiar sin que nada lo
# note, que es justo lo que el respaldo existe para impedir. Se excluyen FIRMA -que
# no se protege a si misma- y los dos donde vive la propia suma.
_ESPERADOS = set(v for k, v in _REG_DEF.items()
                 if k not in ("REG_FIRMA", "REG_SUMA_ALTA", "REG_SUMA_BAJA"))
if set(ORDEN_SUMA) != _ESPERADOS:
    _faltan = sorted(NOMBRE_REG.get(r, r) for r in _ESPERADOS - set(ORDEN_SUMA))
    raise _fw.Abortado(
        "calcularSuma() no cubre todos los registros de contenido. Fuera del "
        f"checksum: {_faltan}. Un registro que no entra en la suma se puede cambiar "
        "sin que el equipo lo note")

# El multiplicador tambien se lee, para que cambiarlo en el C++ no deje al validador
# midiendo otra cosa en silencio.
_mult = re.search(r"s\s*\*\s*(\d+)U", _cuerpo_suma.group(1))
if not _mult:
    raise _fw.Abortado("no se pudo leer el multiplicador de calcularSuma()")
MULT_SUMA = int(_mult.group(1))

# N-51: SE DETECTA EL PLIEGUE, NO SE DA POR AUSENTE. calcularSuma() dejo de
# plegar sus 32 bits a 16 el 05/08/2026, pero escribir esa ausencia a mano en
# el modelo seria el mismo error que PESOS_SUMA: si alguien reintrodujera el
# pliegue -por ejemplo, revirtiendo el commit por error-, el modelo seguiria
# midiendo "sin pliegue" y el barrido de 2.7/2.8 dejaria de ver los pares
# ciegos que el pliegue vuelve a abrir. Se busca el patron en el propio cuerpo.
HAY_PLIEGUE_SUMA = bool(re.search(r"s\s*>>\s*16\)\s*\^\s*s", _cuerpo_suma.group(1)))

# N-51: aqui vivia PESOS_SUMA = {_r: 1 for _r in ORDEN_SUMA}, resto de un algoritmo
# ANTERIOR (una suma ponderada) que calcularSuma() dejo de ser el 01/08/2026. Con
# todos los pesos a 1 la resta que las pruebas de transposicion hacian con el
# ("dif_peso == 0") era SIEMPRE 0, asi que las pruebas 2.7 y 2.8 marcaban "ciego"
# por construccion sin llamar una sola vez al checksum real -10 pares ciegos
# reportados donde la medida real da 8-. Se retira: los pesos reales de Horner son
# posicionales (COEF_SUMA, mas abajo) y se derivan de MULT_SUMA y ORDEN_SUMA, que
# si son del C++.


def calcular_suma(regs):
    """Port de calcularSuma(): hash de Horner con la semilla, el multiplicador y el
    ORDEN reales leidos del C++.

    N-51: hasta el 05/08/2026 esto terminaba en 'return ((s>>16)^s)&0xFFFF' -el
    pliegue de 32 a 16 bits que tiraba la mitad de la mezcla y dejaba 8 pares de
    registros con transposiciones ciegas-. calcularSuma() ya no pliega: devuelve
    los 32 bits crudos, que el firmware guarda enteros en dos registros. El port
    aplica el pliegue SOLO SI HAY_PLIEGUE_SUMA lo detecto en el cuerpo real -no
    porque hoy no lo necesite, sino para que una reversion accidental del
    pliegue se note aqui tambien, no solo en el firmware."""
    s = SEMILLA_SUMA
    for n in ORDEN_SUMA:
        s = (s * MULT_SUMA + regs[n]) & 0xFFFFFFFF
    if HAY_PLIEGUE_SUMA:
        s = ((s >> 16) ^ s) & 0xFFFF
    return s


def suma_llana(regs):
    """MUTANTE: la suma SIN ponderar, que es la que habia antes del 01/08/2026 y la
    que resulto insensible al orden. Sirve para demostrar que las pruebas de
    transposicion distinguen un algoritmo del otro. Se deja deliberadamente distinta
    del modelo real: si un dia coincidieran, las pruebas dejarian de probar nada."""
    s = sum(regs[n] for n in ORDEN_SUMA)
    return (s ^ SEMILLA_SUMA) & 0xFFFF


# Dominios REALMENTE alcanzables de cada registro, segun lo que el firmware
# escribe en ellos. Importan porque una transposicion solo es un peligro si los
# valores que la producen pueden existir: buscar colisiones sobre los 65536
# valores de cada registro encontraria casos que el equipo nunca puede generar.
DOMINIO_REG = {
    REG_VERDE:    range(1, 256),        # byte de CMD_CONFIG
    REG_DESPEJE:  range(1, 256),        # byte de CMD_CONFIG
    REG_FLAGS:    range(0, 8),          # tres indicadores
    REG_SYNC_ALTA: range(0, 65536),     # 16 bits altos del contador del RTC
    REG_SYNC_BAJA: range(0, 65536),     # 16 bits bajos del contador del RTC

    # N-133: los tiempos del ciclo automatico. El dominio NO son los 65536 valores:
    # el firmware solo escribe aqui desde modoAutomatico_fijarTiempos(), que ya ha
    # comprobado los rangos viales. Modelar mas de lo que el equipo puede producir
    # haria buscar colisiones en casos que no existen -y ademas tardaria una eternidad-.
    _REG_DEF["REG_CICLO_RV"]:      range(0x0303, 0x0F10),  # rojo<<8 | verde, 3..15 los dos
    _REG_DEF["REG_CICLO_DESPEJE"]: range(10, 91),          # segundos de despeje
}


# --------------------------------------------------------------------------
# BARRIDO DE TRANSPOSICIONES CIEGAS SOBRE EL DOMINIO ALCANZABLE.
#
# calcularSuma() es Horner, no una suma ponderada: cada registro pesa segun su
# POSICION (COEF_SUMA, abajo), y dos posiciones distintas nunca comparten peso.
# Comparar un PESOS_SUMA de compatibilidad (retirado en N-51, ver mas arriba)
# daba 10 pares ciegos sin llamar una sola vez al checksum real.
#
# Este barrido llama al checksum real sobre el dominio alcanzable, con los
# otros tres registros fijos y los dos que se transponen recorriendo TODO su
# rango. Para que recorrer 65536 x 65536 valores (SYNC_ALTA x SYNC_BAJA) no
# cueste horas, se usa la MISMA aritmetica de Horner en forma cerrada -s =
# SEMILLA*MULT^n + suma(reg_k * MULT^(n-1-k)), que es lo que hace el bucle de
# calcularSuma() paso a paso- y se CONTROLA contra la funcion real antes de
# fiarse de ella. Los coeficientes se derivan de MULT_SUMA y ORDEN_SUMA, ya
# leidos del C++: escribirlos a mano seria repetir el error que dejo pasar los
# 10 pares falsos.
#
# N-51 FASE 2: hasta el 05/08/2026 calcularSuma() plegaba sus 32 bits a 16 con
# un XOR, y ese pliegue era lo que dejaba pares ciegos -no la mezcla de Horner
# en si-. Sin pliegue, "ciego" pasa a exigir que los 32 bits CRUDOS coincidan
# exactamente: suma_cruda() ya no necesita un paso de plegado aparte, porque
# calcular_suma() tampoco lo tiene.
# --------------------------------------------------------------------------
_N_SUMA = len(ORDEN_SUMA)
COEF_SUMA = {_r: pow(MULT_SUMA, _N_SUMA - 1 - _i, 1 << 32)
             for _i, _r in enumerate(ORDEN_SUMA)}
_SEMILLA_TERM_SUMA = (SEMILLA_SUMA * pow(MULT_SUMA, _N_SUMA, 1 << 32)) & 0xFFFFFFFF


def suma_cruda(regs):
    """Los 32 bits CRUDOS de calcularSuma() -antes de cualquier pliegue-, como
    combinacion lineal de los coeficientes de Horner en vez de un bucle. Nunca
    pliega por si misma: eso lo decide quien la llama, con HAY_PLIEGUE_SUMA."""
    s = _SEMILLA_TERM_SUMA
    for _r in ORDEN_SUMA:
        s += regs[_r] * COEF_SUMA[_r]
    return s & 0xFFFFFFFF


def _pliegue_si_toca(s):
    """Aplica el pliegue de 32 a 16 bits SOLO SI el cuerpo real de calcularSuma()
    lo tiene (HAY_PLIEGUE_SUMA). Existe para que suma_cruda() -que es pura
    aritmetica lineal, sin pliegue- pueda compararse contra calcular_suma() sin
    importar en que estado este el firmware."""
    return ((s >> 16) ^ s) & 0xFFFF if HAY_PLIEGUE_SUMA else s


# CONTROL: si la combinacion lineal no reproduce calcularSuma() para contenido
# cualquiera -no solo el de las pruebas de abajo- el barrido de 2.7/2.8 estaria
# midiendo una formula que no es el firmware, y eso es peor que no medir nada.
# Semilla fija: el control tiene que ser reproducible, no una loteria distinta
# en cada corrida.
for _k_ctrl in range(500):
    _muestra_ctrl = {_r: random.Random(0x1F35 + _k_ctrl).randrange(0, 65536)
                     for _r in ORDEN_SUMA}
    if _pliegue_si_toca(suma_cruda(_muestra_ctrl)) != calcular_suma(_muestra_ctrl):
        raise _fw.Abortado(
            "la combinacion lineal de Horner (COEF_SUMA) no reproduce "
            "calcularSuma(): no fiarse del barrido de 2.7/2.8 sin arreglar esto antes")


def transposicion_ciega(a, b, base, dominio_a=None, dominio_b=None):
    """Busca un par de valores (va, vb) -en los dominios dados, por defecto los
    DOMINIO_REG completos de a y b- cuya transposicion deje calcular_suma()
    intacta. Devuelve (va, vb), o None si no hay ninguno en los dominios dados.

    DOS CAMINOS, elegidos por HAY_PLIEGUE_SUMA -detectado del C++, no supuesto-:

    SIN PLIEGUE (el estado tras la Fase 2 de N-51): con los otros tres
    registros fijos en cualquier contenido, la diferencia entre calcular_suma()
    antes y despues de transponer a y b es (va-vb)*(coef_a-coef_b) mod 2^32
    -el termino de 'base' se CANCELA en la resta-. "Colision" pasa a exigir que
    esa diferencia sea 0 mod 2^32, es decir que (va-vb) sea multiplo exacto de
    periodo = 2^32 / mcd(coef_a-coef_b, 2^32). Recorrer solo esos multiplos -en
    vez de cada entero- es lo que hace que agotar el dominio (65536 x 65536
    para SYNC_ALTA-SYNC_BAJA) cueste milisegundos, sin dejar de ser una
    busqueda COMPLETA: no es una cota que se conforma con "no lo encontre en
    las primeras decenas", es la unica forma algebraica de que la igualdad se
    cumpla.

    CON PLIEGUE (el estado anterior, y el que vuelve si alguien reintroduce el
    XOR): el atajo de arriba deja de valer -el pliegue es NO LINEAL, y 'base'
    SI influye en si hay colision-, asi que se recorre el producto de los dos
    dominios llamando a suma_cruda()+pliegue de verdad. Es mas lento, pero es
    justo el camino que 'romper el firmware a proposito' tiene que ejercer:
    medido el 05/08/2026, con pliegue toda colision real aparece en los
    primeros intentos, asi que el coste practico sigue siendo bajo."""
    dominio_a = list(dominio_a if dominio_a is not None else DOMINIO_REG[a])
    dominio_b_set = set(dominio_b if dominio_b is not None else DOMINIO_REG[b])
    if not dominio_a or not dominio_b_set:
        return None
    minb, maxb = min(dominio_b_set), max(dominio_b_set)

    if not HAY_PLIEGUE_SUMA:
        diferencia = COEF_SUMA[a] - COEF_SUMA[b]
        if diferencia == 0:
            # No deberia pasar nunca -Horner no repite coeficiente entre
            # posiciones distintas-, pero si pasara CUALQUIER par colisiona.
            for va in dominio_a:
                for vb in dominio_b_set:
                    if vb != va:
                        return (va, vb)
            return None
        periodo = (1 << 32) // math.gcd(abs(diferencia), 1 << 32)
        for va in dominio_a:
            k = periodo
            while va + k <= maxb:
                if va + k in dominio_b_set:
                    return (va, va + k)
                k += periodo
            k = -periodo
            while va + k >= minb:
                if va + k in dominio_b_set:
                    return (va, va + k)
                k -= periodo
        return None

    # CON PLIEGUE: 'base' si importa (el pliegue no es lineal), asi que hay que
    # recorrer el producto de los dominios con la aritmetica real.
    others = [_r for _r in ORDEN_SUMA if _r not in (a, b)]
    base_const = _SEMILLA_TERM_SUMA + sum(base[_r] * COEF_SUMA[_r] for _r in others)
    ca, cb = COEF_SUMA[a], COEF_SUMA[b]
    for va in dominio_a:
        parte_va_en_a = base_const + va * ca      # va se queda en su registro
        parte_va_en_b = base_const + va * cb      # va pasa al otro tras el swap
        for vb in dominio_b_set:
            if vb == va:
                continue
            s_orig = (parte_va_en_a + vb * cb) & 0xFFFFFFFF
            s_swap = (parte_va_en_b + vb * ca) & 0xFFFFFFFF
            if _pliegue_si_toca(s_orig) == _pliegue_si_toca(s_swap):
                return (va, vb)
    return None


def horas_desde_sync(alta_g, baja_g, hay_sync, _ignorado, rtc_ahora):
    """Port exacto de respaldo_horasDesdeSync() TRAS N-49.

    Ya no recibe dia del mes ni segundo del dia: recibe el contador del RTC. La
    firma conserva el hueco del cuarto argumento para no reescribir las decenas de
    llamadas de los packs, y se ignora a proposito -se llama _ignorado para que
    nadie crea que sigue significando algo-."""
    if not hay_sync:
        return SYNC_CADUCADA
    if rtc_ahora == 0:                   # cero = no hay reloj
        return SYNC_CADUCADA
    guardado = ((alta_g & 0xFFFF) << DESPL_ALTA) | (baja_g & 0xFFFF)
    if rtc_ahora < guardado:             # el reloj retrocedio
        return SYNC_CADUCADA
    return (rtc_ahora - guardado) // 3600


def marcar_sync(rtc, _ignorado=None):
    """Port de respaldo_marcarSync() TRAS N-49: devuelve (registro_alto,
    registro_bajo) o None si la marca se descarta."""
    if rtc == 0:
        return None
    return (rtc >> DESPL_ALTA) & 0xFFFF, rtc & 0xFFFF


# El main.cpp del Maestro, leido una vez. Lo definia el BLOQUE 1 y lo usaba el 2.
_main = _codigo("Maestro", "src", "main.cpp")

