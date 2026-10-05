# ===== banco/modelos/costura.py =====
#
# CONSTANTES Y CONTRATOS DE LA COSTURA MAESTRO <-> ESCLAVO.
#
# La costura es donde mas ha encontrado el banco, y no por casualidad: Maestro y
# Esclavo los construyeron agentes distintos, y la costura es donde falla el trabajo
# en paralelo. Aqui viven las constantes leidas de LAS DOS puntas.
#
# CINCO DEPENDENCIAS CRUZADAS entre secciones vivian ocultas en el fichero unico:
# rama_verde y rama_despeje (S2 -> S4), luz_maestro y luz_esclavo (S2 -> S5 y S6) y
# fase (S2 -> S6). Funcionaban solo porque el fichero corria de arriba abajo.
#
# ⚠️ DEUDA CONOCIDA: este modulo conserva sus propios ruta()/texto() en vez de usar
# banco/fuente.py. NO es descuido: el ruta() de aqui devuelve None cuando falta un
# fichero y varias secciones DEPENDEN de ese None para reportar "FALTA EN UN
# PROYECTO", mientras que banco.fuente ABORTA. Unificarlos cambia comportamiento, y
# esta migracion no cambia comportamiento. Se unifica despues, con su propio commit y
# comparando totales.

import os
import re
import sys

from banco import fuente as _fw

# El modulo bajo dos niveles al mudarse a banco/modelos/, asi que el ".." relativo con
# el que resolvia las rutas dejaba de apuntar a 01_Firmware. Se ancla a banco.fuente,
# que ya sabe donde esta el firmware, y asi la ruta no depende ni del cwd ni de la
# profundidad del fichero. Es, en pequeno, la misma trampa que la guarda de rutas de
# la compuerta vigila: mover un fichero deja al instrumento mirando a otro sitio.
_DIR = _fw.FIRMWARE


def ruta(*partes):
    """Resuelve una ruta dentro de 01_Firmware. Devuelve None si no existe.

    OJO: devuelve None a proposito, y NO aborta como banco.fuente.ruta(). Varias
    secciones dependen de ese None para reportar "FALTA EN UN PROYECTO" en vez de
    morirse: es la diferencia entre "este contrato no esta en las dos puntas" -que es
    un hallazgo- y "no puedo medir" -que es un ABORTADO-."""
    cand = os.path.join(_DIR, *partes)
    return os.path.abspath(cand) if os.path.exists(cand) else None


# ---------------------------------------------------------------------------
# ABORTAR SE LANZA, NO SE MATA EL PROCESO. Y es un cambio de fondo.
#
# Estos modelos vienen de ficheros que se ejecutaban solos, donde sys.exit(2) era
# correcto: matar el proceso ERA la forma de decir ABORTADO. Dentro del corredor de
# packs deja de serlo, porque un sys.exit() aqui MATA EL PROCESO ENTERO y se lleva por
# delante a los otros diecinueve packs, que ya no dirian nada de nada.
#
# Es justo lo que banco/correr.py existe para impedir -"un pack que aborta no tumba a
# los demas"- incumplido desde dentro del modelo, donde no se ve. Se lanza
# fuente.Abortado: el corredor lo atrapa, marca ESE pack como ABORTADO y sigue.
# ---------------------------------------------------------------------------

def texto(ruta_fichero):
    """Lee un fuente entero. No poder leerlo ABORTA: sin el fuente no hay
    validacion, y continuar con un valor por defecto seria fabricar un PASS."""
    if not ruta_fichero or not os.path.exists(ruta_fichero):
        raise _fw.Abortado(f"no se encuentra el fuente {ruta_fichero!r}")
    with open(ruta_fichero, "r", encoding="utf-8", errors="replace") as f:
        return f.read()


def num(cuerpo, patron, que):
    """Extrae un entero del C++. La ausencia ABORTA, nunca cae a un defecto.

    POR QUE ABORTAR Y NO USAR UN VALOR POR DEFECTO: si alguien renombra una
    constante de seguridad y el modelo se queda con el numero que tenia escrito,
    el validador sigue diciendo PASS mientras el firmware usa otro valor. Seria
    exactamente la deriva que esto viene a impedir, disfrazada de exito.
    """
    m = re.search(patron, cuerpo)
    if not m:
        raise _fw.Abortado(
            f"no se pudo leer del C++ la constante {que!r} (patron {patron!r}). "
            "Revisa si se renombro en el firmware")
    return int(m.group(1))


def producto(cuerpo, nombre):
    """Lee constantes escritas como producto: `X = 48UL * 3600UL * 1000UL;`.

    El Esclavo escribe asi su limite de 48 h a proposito -para que el 48 se lea
    como horas y no como un numero magico-, y el modelo tiene que entender esa
    forma o no podria compararlo con el del Maestro, que va en milisegundos.
    """
    m = re.search(nombre + r"\s*=\s*([0-9UuLl\s\*]+);", cuerpo)
    if not m:
        raise _fw.Abortado(f"no se pudo leer la expresion de {nombre!r}")
    total = 1
    for factor in m.group(1).split("*"):
        total *= int(re.sub(r"[UuLl\s]", "", factor))
    return total


# --- Rutas de los dos proyectos --------------------------------------------
M_PROTO_H = ruta("Maestro", "include", "protocolo.h")
E_PROTO_H = ruta("Esclavo", "include", "protocolo.h")
M_CICLO_H = ruta("Maestro", "include", "ciclo_degradado.h")
E_CICLO_H = ruta("Esclavo", "include", "ciclo_degradado.h")
M_RESP_H = ruta("Maestro", "include", "respaldo.h")
E_RESP_H = ruta("Esclavo", "include", "respaldo.h")
M_RESP_C = ruta("Maestro", "src", "respaldo.cpp")
E_RESP_C = ruta("Esclavo", "src", "respaldo.cpp")
M_PROTO_C = ruta("Maestro", "src", "protocolo.cpp")
E_PROTO_C = ruta("Esclavo", "src", "protocolo.cpp")

M_DEG_C = ruta("Maestro", "src", "modo_degradado.cpp")
E_DEG_C = ruta("Esclavo", "src", "modo_degradado.cpp")
M_DEG_H = ruta("Maestro", "include", "modo_degradado.h")
E_DEG_H = ruta("Esclavo", "include", "modo_degradado.h")
M_COORD_C = ruta("Maestro", "src", "coordinador.cpp")
E_MAIN_C = ruta("Esclavo", "src", "main.cpp")
M_MAIN_C = ruta("Maestro", "src", "main.cpp")
M_SEM_C = ruta("Maestro", "src", "semaforo.cpp")
E_SEM_C = ruta("Esclavo", "src", "semaforo.cpp")
M_RELOJ_C = ruta("Maestro", "src", "reloj.cpp")
E_RELOJ_C = ruta("Esclavo", "src", "reloj.cpp")



T_M_PROTO_H = texto(M_PROTO_H)
T_M_CICLO_H = texto(M_CICLO_H)
T_M_DEG_C = texto(M_DEG_C)
T_E_DEG_C = texto(E_DEG_C)
T_M_DEG_H = texto(M_DEG_H)
T_E_DEG_H = texto(E_DEG_H)
T_M_COORD_C = texto(M_COORD_C)
T_E_MAIN_C = texto(E_MAIN_C)
T_M_MAIN_C = texto(M_MAIN_C)
T_M_SEM_C = texto(M_SEM_C)
T_E_SEM_C = texto(E_SEM_C)
T_M_RESP_C = texto(M_RESP_C)
T_M_RELOJ_C = texto(M_RELOJ_C)
T_E_RELOJ_C = texto(E_RELOJ_C)
T_M_PROTO_C = texto(M_PROTO_C)

# --- Codigos de comando: del contrato, jamas copiados ----------------------
NOMBRES_CMD = re.findall(r"#define\s+(CMD_[A-Z_]+)\s+0x([0-9A-Fa-f]+)", T_M_PROTO_H)
CMD = {n: int(v, 16) for n, v in NOMBRES_CMD}

RF_BURST_COPIES = num(T_M_PROTO_H, r"#define\s+RF_BURST_COPIES\s+(\d+)", "RF_BURST_COPIES")
SEGUNDOS_DEL_DIA = num(T_M_CICLO_H, r"SEGUNDOS_DEL_DIA\s*=\s*(\d+)", "SEGUNDOS_DEL_DIA")

# --- Ciclo degradado, lado Maestro (unico origen de los dos numeros) -------
DEG_VERDE_SEG = num(T_M_DEG_C, r"DEG_VERDE_SEG\s*=\s*(\d+)", "DEG_VERDE_SEG")
DEG_DESPEJE_SEG = num(T_M_DEG_C, r"DEG_DESPEJE_SEG\s*=\s*(\d+)", "DEG_DESPEJE_SEG")

# --- Puertas y limites del Maestro ----------------------------------------
M_SYNC_FRESCA_MS = num(T_M_DEG_C, r"SYNC_FRESCA_MS\s*=\s*(\d+)", "SYNC_FRESCA_MS")
M_LIMITE_DURO_MS = num(T_M_DEG_C, r"LIMITE_DURO_MS\s*=\s*(\d+)", "LIMITE_DURO_MS")
M_AVISO_LIMITE_MS = num(T_M_DEG_C, r"AVISO_LIMITE_MS\s*=\s*(\d+)", "AVISO_LIMITE_MS")
M_TOLERANCIA_S = num(T_M_DEG_C, r"TOLERANCIA_DESFASE_S\s*=\s*(\d+)", "TOLERANCIA_DESFASE_S")
# Rojo previo al ambar en el Maestro: son 2 s escritos dentro de DEG_AMBAR.
M_ROJO_ANTES_AMBAR_MS = num(
    T_M_DEG_C, r"!ambarArrancado\s*&&\s*millis\(\)\s*-\s*tEstado\s*>=\s*(\d+)",
    "rojo previo al ambar (Maestro)")

# --- Puertas y limites del Esclavo ----------------------------------------
E_LIMITE_MS = producto(T_E_DEG_C, "LIMITE_SIN_SYNC_MS")
E_AVISO_MS = producto(T_E_DEG_C, "AVISO_SIN_SYNC_MS")
E_ROJO_MINIMO_MS = num(T_E_DEG_C, r"ROJO_MINIMO_MS\s*=\s*(\d+)", "ROJO_MINIMO_MS")
E_PERIODO_FASE_MS = num(T_E_DEG_C, r"PERIODO_FASE_MS\s*=\s*(\d+)", "PERIODO_FASE_MS")

# --- Tiempos del enlace ----------------------------------------------------
RETARDO_RESPUESTA_MS = num(T_E_MAIN_C, r"RETARDO_RESPUESTA_MS\s*=\s*(\d+)", "RETARDO_RESPUESTA_MS")
TIMEOUT_ACK_MS = num(T_M_COORD_C, r"TIMEOUT_ACK_MS\s*=\s*(\d+)", "TIMEOUT_ACK_MS")
INTERVALO_SYNC_MS = num(T_M_COORD_C, r"INTERVALO_SYNC_MS\s*=\s*(\d+)", "INTERVALO_SYNC_MS")
BACKOFF_SYNC_MS = num(T_M_COORD_C, r"BACKOFF_SYNC_MS\s*=\s*(\d+)", "BACKOFF_SYNC_MS")
BAUD_CABLE = num(T_M_PROTO_C, r"Bus\.begin\((\d+)\)", "baudios del bus al modulo")

# --- D-45: el amarillo de CIERRE, del contrato de cada punta (AMARILLO_SEG, en s) ---
PAT_AMARILLO = r"#define\s+AMARILLO_SEG\s+(\d+)UL"
M_AMARILLO_MS = 1000 * num(T_M_PROTO_H, PAT_AMARILLO, "amarillo de cierre (Maestro)")
E_AMARILLO_MS = 1000 * num(texto(E_PROTO_H), PAT_AMARILLO, "amarillo de cierre (Esclavo)")
AMARILLO_S = M_AMARILLO_MS // 1000

# --- D-53: el ROJO+AMARILLO antes de cada verde, del contrato de cada punta (ROJO_AMARILLO_SEG) ---
PAT_ROJO_AMARILLO = r"#define\s+ROJO_AMARILLO_SEG\s+(\d+)UL"
M_ROJO_AMARILLO_MS = 1000 * num(T_M_PROTO_H, PAT_ROJO_AMARILLO, "rojo+amarillo (Maestro)")
E_ROJO_AMARILLO_MS = 1000 * num(texto(E_PROTO_H), PAT_ROJO_AMARILLO, "rojo+amarillo (Esclavo)")
RA_S = M_ROJO_AMARILLO_MS // 1000


# --------------------------------------------------------------------------
# PORTS COMPARTIDOS ENTRE SECCIONES.
#
# Las cinco dependencias cruzadas que el fichero unico ocultaba: estaban definidas
# dentro de la seccion [2] y las usaban la [4], la [5] y la [6]. Funcionaban solo
# porque el fichero se ejecutaba de arriba abajo; correr una seccion suelta -que es lo
# que ahora se puede hacer- las habria roto todas en silencio.
#
# fase() y las dos luz_*() son el corazon del asunto: la fase se calcula IGUAL en las
# dos puntas por contrato -ciclo_degradado.h es identico byte a byte-, pero cada punta
# TRADUCE esa fase a luces con su propio codigo. Ahi es donde puede aparecer el verde
# simultaneo, y por eso las tres viven juntas.
#
# AQUI NO HAY NI UNA LLAMADA A verificar(). Un modelo que comprueba cosas al
# importarse decide el veredicto de quien lo use, y ademas lo cuenta una vez por cada
# pack que lo importe.
# --------------------------------------------------------------------------
# D-53 (05/10): ocho fases, en el orden del enum FaseDegradado de ciclo_degradado.h: el
# ROJO+AMARILLO (ROJO_AMARILLO_SEG) delante de cada verde, el amarillo de cierre detras.
(FD_ROJO_AMARILLO_MAESTRO, FD_VERDE_MAESTRO, FD_AMARILLO_MAESTRO, FD_DESPEJE_A,
 FD_ROJO_AMARILLO_ESCLAVO, FD_VERDE_ESCLAVO, FD_AMARILLO_ESCLAVO, FD_DESPEJE_B) = range(8)


def _cruda(pos, v, d, a, r=None):
    """ciclo_degradado_faseCruda(). r = ROJO_AMARILLO_SEG."""
    r = RA_S if r is None else r
    if pos < r:
        return FD_ROJO_AMARILLO_MAESTRO
    if pos < r + v:
        return FD_VERDE_MAESTRO
    if pos < r + v + a:
        return FD_AMARILLO_MAESTRO
    if pos < r + v + a + d:
        return FD_DESPEJE_A
    if pos < 2 * r + v + a + d:
        return FD_ROJO_AMARILLO_ESCLAVO
    if pos < 2 * (r + v) + a + d:
        return FD_VERDE_ESCLAVO
    if pos < 2 * (r + v + a) + d:
        return FD_AMARILLO_ESCLAVO
    return FD_DESPEJE_B


def fase(seg_dia, verde, despeje):
    """Espejo de ciclo_degradado_fase() (D-53): las guardas de medianoche, el verde que no
    cabe tras el despeje de las 00:00, el amarillo que naceria sin verde, el R+A que no cabe
    antes del tramo final y el amarillo forzado al final del dia. Mas abajo, sus huellas en
    el C++ (espejo_ok)."""
    if verde == 0 or despeje == 0:
        return FD_DESPEJE_A
    a, r = AMARILLO_S, RA_S
    ciclo = 2 * (r + verde + a + despeje)
    fin_dia = SEGUNDOS_DEL_DIA - despeje
    if seg_dia < despeje or seg_dia >= fin_dia:
        return FD_DESPEJE_B
    pos = seg_dia % ciclo
    f = _cruda(pos, verde, despeje, a, r)
    if f in (FD_ROJO_AMARILLO_MAESTRO, FD_VERDE_MAESTRO, FD_ROJO_AMARILLO_ESCLAVO, FD_VERDE_ESCLAVO):
        de_m = f in (FD_ROJO_AMARILLO_MAESTRO, FD_VERDE_MAESTRO)
        fin_verde = r + verde if de_m else 2 * (r + verde) + a + despeje
        if seg_dia + (fin_verde - pos) <= despeje + r:
            return FD_DESPEJE_B
    if f in (FD_AMARILLO_MAESTRO, FD_AMARILLO_ESCLAVO):
        inicio = r + verde if f == FD_AMARILLO_MAESTRO else 2 * (r + verde) + a + despeje
        if seg_dia - (pos - inicio) <= despeje + r:
            return FD_DESPEJE_A if f == FD_AMARILLO_MAESTRO else FD_DESPEJE_B
    if f in (FD_ROJO_AMARILLO_MAESTRO, FD_ROJO_AMARILLO_ESCLAVO):
        fin_ra = r if f == FD_ROJO_AMARILLO_MAESTRO else 2 * r + verde + a + despeje
        if seg_dia + (fin_ra - pos) + a >= fin_dia:
            return FD_DESPEJE_B if f == FD_ROJO_AMARILLO_MAESTRO else FD_DESPEJE_A
    if seg_dia + a >= fin_dia:
        f0 = _cruda((fin_dia - a) % ciclo, verde, despeje, a, r)
        encendido = f0 == _cruda((fin_dia - a - 1) % ciclo, verde, despeje, a, r)
        if encendido and f0 == FD_VERDE_MAESTRO:
            return FD_AMARILLO_MAESTRO
        if encendido and f0 == FD_VERDE_ESCLAVO:
            return FD_AMARILLO_ESCLAVO
        if f == FD_VERDE_MAESTRO:
            return FD_DESPEJE_B
        if f == FD_VERDE_ESCLAVO:
            return FD_DESPEJE_A
        if f != f0:
            return FD_DESPEJE_A if f == FD_AMARILLO_MAESTRO else FD_DESPEJE_B
    return f


# --- 2a. El espejo no puede envejecer sin que nadie lo note ----------------
cuerpo_fase = re.search(r"ciclo_degradado_fase\(uint32_t segDia.*?\n\}", T_M_CICLO_H, re.S)
cuerpo_cruda = re.search(r"ciclo_degradado_faseCruda\(uint32_t pos.*?\n\}", T_M_CICLO_H, re.S)
huellas_c = [
    r"if\s*\(verdeSeg\s*==\s*0\s*\|\|\s*despejeSeg\s*==\s*0\)\s*return\s+FD_DESPEJE_A",
    r"ciclo\s*=\s*2UL\s*\*\s*\(r\s*\+\s*\(uint32_t\)verdeSeg\s*\+\s*a\s*\+\s*\(uint32_t\)despejeSeg\)",
    r"finDia\s*=\s*SEGUNDOS_DEL_DIA\s*-\s*despejeSeg",
    r"if\s*\(segDia\s*<\s*despejeSeg\)\s*return\s+FD_DESPEJE_B",
    r"if\s*\(segDia\s*>=\s*finDia\)\s*return\s+FD_DESPEJE_B",
    r"pos\s*=\s*segDia\s*%\s*ciclo",
    r"if\s*\(segDia\s*\+\s*\(finVerde\s*-\s*pos\)\s*<=\s*\(uint32_t\)despejeSeg\s*\+\s*r\)",
    r"if\s*\(segDia\s*-\s*\(pos\s*-\s*inicio\)\s*<=\s*\(uint32_t\)despejeSeg\s*\+\s*r\)",
    r"if\s*\(segDia\s*\+\s*\(finRA\s*-\s*pos\)\s*\+\s*a\s*>=\s*finDia\)",
    r"if\s*\(segDia\s*\+\s*a\s*>=\s*finDia\)",
]
huellas_cruda = [
    r"if\s*\(pos\s*<\s*r\)\s*return\s+FD_ROJO_AMARILLO_MAESTRO",
    r"if\s*\(pos\s*<\s*r\s*\+\s*v\)\s*return\s+FD_VERDE_MAESTRO",
    r"if\s*\(pos\s*<\s*r\s*\+\s*v\s*\+\s*a\)\s*return\s+FD_AMARILLO_MAESTRO",
    r"if\s*\(pos\s*<\s*r\s*\+\s*v\s*\+\s*a\s*\+\s*d\)\s*return\s+FD_DESPEJE_A",
    r"if\s*\(pos\s*<\s*2UL\s*\*\s*r\s*\+\s*v\s*\+\s*a\s*\+\s*d\)\s*return\s+FD_ROJO_AMARILLO_ESCLAVO",
    r"if\s*\(pos\s*<\s*2UL\s*\*\s*\(r\s*\+\s*v\)\s*\+\s*a\s*\+\s*d\)\s*return\s+FD_VERDE_ESCLAVO",
    r"if\s*\(pos\s*<\s*2UL\s*\*\s*\(r\s*\+\s*v\s*\+\s*a\)\s*\+\s*d\)\s*return\s+FD_AMARILLO_ESCLAVO",
]
espejo_ok = (cuerpo_fase is not None and cuerpo_cruda is not None and
             all(re.search(p, cuerpo_fase.group(0)) for p in huellas_c) and
             all(re.search(p, cuerpo_cruda.group(0)) for p in huellas_cruda))

def simbolos(m):
    """Quita los casts y se queda con los identificadores que se pasan."""
    if not m:
        return None
    crudo = re.sub(r"\(\s*uint\d+_t\s*\)", "", m.group(1))
    return [p.strip() for p in crudo.split(",")]

# FASE 2 (03/08/2026): la recepcion del par se mudo de Esclavo/src/main.cpp a
# src/config_ciclo.cpp.
#
# LA COMPROBACION NO ABORTO AL MOVERSE. main.cpp sigue existiendo, asi que la guarda de
# rutas no vio nada: el patron simplemente dejo de encontrarse y la prueba se puso en
# FALLA, acusando al firmware de un defecto que no tiene. Lo cazo la COMPARACION DE
# TOTALES -36/41 contra los 37/41 de siempre-, que es la unica red que ve esta clase de
# deriva.
#
# Queda anotado porque afina la regla: la guarda de rutas vigila ficheros que
# DESAPARECEN; no vigila CONTENIDO que se muda de fichero.
T_E_CONFIG_C = texto(ruta("Esclavo", "src", "config_ciclo.cpp"))
rama_verde = re.search(r"void\s+config_rxVerde\([^)]*\)\s*\{(.*?)\n\}", T_E_CONFIG_C, re.S)
rama_despeje = re.search(r"bool\s+config_rxDespeje\([^)]*\)\s*\{(.*?)\n\}", T_E_CONFIG_C, re.S)

# Codigos de luz. Iban sueltos en la seccion [2] y los usa la [6] al comparar lo
# que enciende cada punta tras un corte. D-53: ROJO_AMARILLO, rojo y amarillo a la vez.
ROJO, AMBAR_FIJO, VERDE, AMBAR_INTERMITENTE, ROJO_AMARILLO = 0, 1, 2, 3, 4


def _luz_de(f, ra, verde, amarillo):
    """La luz de una punta en DEG_ACTIVO segun la fase: semaforo_forzarVerde() abre por
    S_ROJO_AMARILLO (ROJO_AMARILLO_SEG, lo que dura la fase R+A) y semaforo_actualizar() pone
    el VERDE al vencer; fuera, semaforo_forzarRojo(), que sobre el verde da el amarillo de
    cierre (D-45). SIMPLIFICA: sin estado, asi que no ve verdeConTiempo() (una entrada a mitad
    de la fase que no abre un verde sin tiempo); eso lo mide el arnes del Degradado."""
    if f == ra:
        return ROJO_AMARILLO
    if f == verde:
        return VERDE
    return AMBAR_FIJO if f == amarillo else ROJO


def luz_maestro(seg_dia, verde, despeje):
    """modo_degradado.cpp (Maestro), DEG_ACTIVO: abre en FD_ROJO_AMARILLO_MAESTRO y
    FD_VERDE_MAESTRO (D-53); FD_AMARILLO_MAESTRO es su amarillo de cierre."""
    return _luz_de(fase(seg_dia, verde, despeje), FD_ROJO_AMARILLO_MAESTRO, FD_VERDE_MAESTRO,
                   FD_AMARILLO_MAESTRO)


def luz_esclavo(seg_dia, verde, despeje, amarillo_s=None):
    """modo_degradado.cpp (Esclavo), DEG_ACTIVO: aplicarLuz(fase R+A o VERDE del Esclavo)
    (D-53). amarillo_s queda por compatibilidad: no hay ambar que se coma el verde."""
    return _luz_de(fase(seg_dia, verde, despeje), FD_ROJO_AMARILLO_ESCLAVO, FD_VERDE_ESCLAVO,
                   FD_AMARILLO_ESCLAVO)


def paso(luz):
    """Luz COMPROMETIDA en el solape. D-45: el amarillo de cierre es paso abierto -quien no
    puede parar, entra-. D-53: el ROJO+AMARILLO no abre paso, pero no se cuenta como margen
    (SPEC_2 8 (e.ter)): el margen de desfase sigue siendo el despeje, como en vigilar() del
    arnes del Degradado."""
    return luz in (VERDE, AMBAR_FIJO, ROJO_AMARILLO)
