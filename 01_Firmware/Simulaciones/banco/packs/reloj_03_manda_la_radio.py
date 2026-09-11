# ===== banco/packs/reloj_03_manda_la_radio.py =====
#
# LA HORA DEL ESP32 ENTRA POR SU RAMA, CON SU FORMA EXACTA, Y EN EL ESCLAVO SOLO SI LA RADIO
# NO MANDA (D-26 (3)); Y SI NO LLEGA O NO SIRVE, ALARMA (D-26 (5)).
#
# LA PROPIEDAD, EN UNA LINEA: la rama CMD:HORA_ESP32 de las dos puntas entra sin PIN, no
# contesta, no renueva nada del Degradado y solo siembra una linea con la forma exacta que
# el ESP32 compone; en el Esclavo la hora de su ESP32 solo entra si la radio no manda -"con
# radio, la del Maestro; sin radio, la de su propio ESP32"-, con "sin radio" definido por
# el mismo silencio que dispara FALLO_RF; y la fuente de la hora la marcan SOLO los dos
# sembradores, sin que la guarda se cuele en el camino que comparten la radio y el Maestro.
#
# POR QUE EXISTE [11/09]. D-20 y A-15 decidieron que cada ESP32 siembre a su STM32 desde su
# DS3231. En el Esclavo eso abre DOS fuentes -la radio, que trae la hora del Maestro, y su
# propio ESP32-.
#
# 🔴 REVISADO UNA POR UNA el 11/09 por la tarde (CLAUDE.md 9), porque D-26 (3) cambio la
# regla que este pack media -"la del ESP32 solo entra SIN HORA"-:
#   - la 5 (la guarda va ANTES y la siembra en su else) -> SE CONSERVA: la regla nueva
#     tambien es una guarda delante del sembrador; cambia la pregunta, no el orden;
#   - la 6 (la guarda no se cuela en el camino compartido) -> SE CONSERVA con otra bandera:
#     ya no es horaValida -que contesta "hay hora?"- sino fuenteHora -"de quien es?"-, que
#     el sembrador y la radio ESCRIBEN (eso se permite) y no pueden LEER;
#   - "la del ESP32 solo entra sin hora" -> SE INVIERTE en la 8: la guarda es
#     reloj_radioManda(), y se exige que "sin radio" sea el MISMO silencio de la orfandad.
#     La regla vieja dejaba al Esclavo sin radio extrapolando con el HSI todo el Degradado
#     (hasta 90 s por hora contra 29 s de margen); D-26 acepta a cambio el salto entre los
#     dos DS3231 y lo acota con la regla (4) y con la alarma de radio;
#   - y lo que la regla nueva necesita para ser VERDAD entra con su medida: quien avisa de
#     la radio (9), quien marca la fuente (10), que el doble del arnes del puente rechace lo
#     mismo que el firmware (11) y la alarma de D-26 (5) (12).
#
# LO QUE SE MIDE AQUI, Y POR QUE CADA COSA:
#
#   - que la guarda vaya ANTES que la siembra y la siembra viva en su `else` (CLAUDE.md 9:
#     una prueba que solo mira el RESULTADO aprueba las barreras en el orden equivocado);
#   - que la guarda NO se cuele en el sembrador ni en el validador: los comparten las dos
#     puntas y la radio. Metida ahi, el Maestro dejaria de aceptar la hora de su ESP32 -D-20
#     dice que la acepta siempre- y la radio dejaria de SOBRESCRIBIR la del Esclavo;
#   - que la siembra solo lea la forma exacta "YYYY-MM-DD,HH:MM:SS": este STM32 no
#     comprueba checksum de entrada, y sscanf("%d") da seis campos con una linea truncada;
#   - y lo de las dos ramas: sin PIN, sin acuse, sin renovar las 48 h, con el prefijo
#     contado bien.
#
# LO QUE ESTE PACK NO MIDE, ESCRITO PARA QUE NADIE LO LEA COMO PERMISO. Es un pack de
# TEXTO: nadie EJECUTA reloj.cpp en el PC -ningun arnes lo enlaza, porque incluye
# <STM32RTC.h> y <stm32f1xx_hal.h>-, asi que un defecto del TIEMPO o de un bucle aqui no se
# ve: se ve la FORMA de la decision. La mitad ESP32 -que el puente componga la linea con la
# hora RELEIDA y la tire si viene del telefono- es del otro firmware y la miden los esp32_*.
#
# SIN ETIQUETA SFTY, Y ES DELIBERADO: roza SFTY-23 (la hora comun de las dos puntas) pero no
# la EJERCE -no mide un desfase ni una sincronizacion-. Una fila cubierta por una prueba que
# no la ejerce es peor que una vacia.

import re

NOMBRE = "reloj_03_manda_la_radio"
DESCRIPCION = ("la hora del ESP32 entra sin PIN, sin acuse, con forma exacta y sin renovar "
               "el Degradado; en el Esclavo solo si la radio no manda (D-26 (3)), y si no "
               "llega o no sirve, alarma (D-26 (5))")

PUNTAS = ("Maestro", "Esclavo")
BLUETOOTH = {"Maestro": ("Maestro", "src", "bluetooth.cpp"),
             "Esclavo": ("Esclavo", "src", "bluetooth.cpp")}
RELOJ = {"Maestro": ("Maestro", "src", "reloj.cpp"),
         "Esclavo": ("Esclavo", "src", "reloj.cpp")}
MAIN_E = ("Esclavo", "src", "main.cpp")
ARNES_PUENTE = ("Simulaciones", "puente_esp32", "arnes_puente.cpp")
SIEMBRA_ESP32 = ("ESP32_Expansion", "src", "siembra.cpp")
CONTRATO_ESP32 = ("ESP32_Expansion", "include", "contrato.h")
ORQ_DEG = ("Validacion_Automatico", "dos_puntas", "orquestador_degradado.cpp")
ADAPT_E = ("Validacion_Automatico", "dos_puntas", "adaptador_esclavo.cpp")

# Los sujetos, por su nombre. Todos se comprueban presentes antes de medir (N-96).
RAMA = "CMD:HORA_ESP32:"
SEMBRADOR = "reloj_sembrarDesdeIso"
GUARDA = "reloj_radioManda"
PUNTA_GUARDA = "Esclavo"
BANDERA = "fuenteHora"          # lo que GUARDA lee y el camino compartido solo ESCRIBE
NOTAR = "reloj_notarRadio"
RADIO_AJUSTA = "reloj_ajustar"  # el sembrador de la radio: CMD_HORA_S de main.cpp
ALARMA = "bluetooth_reportarAlarma"
# ~~La longitud de la linea entera, 34, escrita aqui como contrato~~ -> 11/09: se DERIVA
# del literal del ESP32 (FORMATO_HORA_ESP32) con los maximos de sus rangos: el contrato es
# lo que el otro firmware compone, no un numero copiado de el.
# Lo que renueva la autorizacion del Degradado. Una hora del DS3231 de un poste no demuestra
# que las dos puntas sigan en fase: estas dos llamadas son de la RADIO.
RENUEVAN_48H = ("degradado_registrarSync", "respaldo_marcarSync")


# ---------------------------------------------------------------------------------
# LECTURA POR LLAVES. _bloque y _rama vienen LITERALES de reloj_02_siembra_que_miente
# -que a su vez los trajo de app_03-: reescribirlos para renombrar es como se cuelan los
# errores en un cambio que no debe cambiar nada.
# ---------------------------------------------------------------------------------

def _bloque(texto, i):
    """El interior del bloque que abre en texto[i] == '{'. None si no cierra."""
    if i < 0 or i >= len(texto) or texto[i] != "{":
        return None
    prof = 0
    for j in range(i, len(texto)):
        if texto[j] == "{":
            prof += 1
        elif texto[j] == "}":
            prof -= 1
            if prof == 0:
                return texto[i + 1:j]
    return None


def _fin_del_bloque(texto, i):
    """Indice del '}' que cierra el '{' de texto[i]. -1 si no cierra."""
    prof = 0
    for j in range(i, len(texto)):
        if texto[j] == "{":
            prof += 1
        elif texto[j] == "}":
            prof -= 1
            if prof == 0:
                return j
    return -1


def _rama(codigo, etiqueta):
    """El bloque de la rama `strn?cmp(accion|cmd, "<etiqueta>", n)`. Literal de reloj_02."""
    m = re.search(r'strn?cmp\s*\(\s*(?:accion|cmd)\s*,\s*"%s"' % re.escape(etiqueta),
                  codigo)
    if not m:
        return None
    llave = codigo.find("{", m.end())
    if llave < 0:
        return None
    return _bloque(codigo, llave)


def _cuerpo(codigo, nombre):
    """El cuerpo de la DEFINICION de `nombre` (tipo nombre(...) { ... }). None si no esta."""
    for m in re.finditer(r"\b%s\s*\([^;{)]*\)\s*\{" % re.escape(nombre), codigo):
        antes = codigo[:m.start()].rstrip()
        if not re.search(r"[\w\*&]$", antes):
            continue                      # es una llamada, no una definicion
        return _bloque(codigo, m.end() - 1)
    return None


def _despachador(codigo):
    m = re.search(r"\bprocesarComando\s*\([^)]*\)\s*\{", codigo)
    return _bloque(codigo, m.end() - 1) if m else None


# ---------------------------------------------------------------------------------
# LAS PREGUNTAS, UNA FUNCION CADA UNA. Las comprobaciones y sus controles negativos pasan
# por la MISMA funcion: un control que ejercita otro camino no controla nada.
# ---------------------------------------------------------------------------------

def _orden_guarda_siembra(rama, guarda, sembrador):
    """None si la guarda va primero y la siembra vive en su `else`; si no, el motivo."""
    mg = re.search(r"\bif\s*\(\s*%s\s*\(\s*\)\s*\)\s*\{" % re.escape(guarda), rama)
    if not mg:
        return "la rama no abre con `if (%s()) {`" % guarda
    ms = re.search(r"\b%s\s*\(" % re.escape(sembrador), rama)
    if not ms:
        return "la rama no llama a %s()" % sembrador
    if ms.start() < mg.start():
        return "%s() se llama ANTES de preguntar a %s()" % (sembrador, guarda)
    fin = _fin_del_bloque(rama, mg.end() - 1)
    if fin < 0:
        return "el bloque de la guarda no cierra"
    if ms.start() < fin:
        return "%s() esta DENTRO del bloque en que %s() dijo que si" % (sembrador, guarda)
    if not re.match(r"\s*else\b", rama[fin + 1:]):
        return ("detras del bloque de la guarda no hay `else`: %s() corre tambien cuando la "
                "punta ya tiene hora" % sembrador)
    return None


def _consulta_la_guarda(cuerpo, guarda, bandera):
    """Lo que en `cuerpo` LEE la guarda: llamadas a ella o lecturas de su bandera.

    Una ASIGNACION a la bandera -"horaValida = true" del validador- no es consultarla: es
    lo que el validador tiene que hacer. Lo que no puede es MIRARLA antes de sembrar."""
    fuera = ["%s()" % guarda] if re.search(r"\b%s\s*\(" % re.escape(guarda), cuerpo) else []
    for m in re.finditer(r"\b%s\b" % re.escape(bandera), cuerpo):
        if not re.match(r"\s*=(?!=)", cuerpo[m.end():]):
            fuera.append(bandera)
            break
    return fuera


def _formato_estricto(reloj_c, sembrador):
    """(None | motivo, patron). Que la siembra rechace por la FORMA EXACTA antes de sscanf.

    Se exigen tres cosas, leidas del fuente: que la primera guarda del sembrador llame a una
    funcion de este mismo fichero ANTES del sscanf; que esa funcion recorra un patron
    literal y compruebe el terminador detras -"y NADA detras"-; y que el patron tenga la
    forma de "YYYY-MM-DD,HH:MM:SS", cifra por cifra y separador por separador."""
    cuerpo = _cuerpo(reloj_c, sembrador)
    if cuerpo is None:
        return "no se hallo la definicion de %s()" % sembrador, None
    msc = re.search(r"\bsscanf\s*\(", cuerpo)
    if not msc:
        return "%s() ya no parsea con sscanf" % sembrador, None
    antes = cuerpo[:msc.start()]
    llamadas = [n for n in re.findall(r"!\s*([A-Za-z_]\w*)\s*\(", antes)]
    comprobador = next((n for n in llamadas if _cuerpo(reloj_c, n) is not None), None)
    if comprobador is None:
        return ("%s() no rechaza con ninguna comprobacion de forma antes del sscanf"
                % sembrador), None
    cc = _cuerpo(reloj_c, comprobador)
    mp = re.search(r"\b([A-Z_][A-Z0-9_]*)\s*\[", cc)
    if not mp:
        return "%s() no recorre ningun patron" % comprobador, None
    nombre_p = mp.group(1)
    ml = re.search(r"\b%s\s*\[\s*\]\s*=\s*\"([^\"]*)\"" % re.escape(nombre_p), reloj_c)
    if not ml:
        return "el patron %s no es un literal de este fichero" % nombre_p, None
    patron = ml.group(1)
    if not re.search(r"sizeof\s*\(\s*%s\s*\)\s*-\s*1\s*\]\s*==\s*'\\0'" % re.escape(nombre_p),
                     cc):
        return ("%s() no comprueba que detras del patron se acabe la cadena: una linea con "
                "basura pegada pasaria" % comprobador), patron
    if not re.fullmatch(r"0000-00-00,00:00:00", patron):
        return "el patron %r no tiene la forma YYYY-MM-DD,HH:MM:SS" % patron, patron
    return None, patron


def _antes_del_pin(despachador, etiqueta):
    """True si la rama de `etiqueta` se compara ANTES que el filtro del PIN."""
    mr = re.search(r'strn?cmp\s*\(\s*cmd\s*,\s*"%s"' % re.escape(etiqueta), despachador)
    mp = re.search(r'strncmp\s*\(\s*cmd\s*,\s*"CMD:PIN:', despachador)
    return bool(mr) and bool(mp) and mr.start() < mp.start()


def _prefijo_cuadra(codigo, rama, etiqueta):
    """(ok, n_del_strncmp, [desplazamientos]) de la rama frente a len(etiqueta)."""
    m = re.search(r'strncmp\s*\(\s*cmd\s*,\s*"%s"\s*,\s*(\d+)\s*\)' % re.escape(etiqueta),
                  codigo)
    n = int(m.group(1)) if m else None
    desp = [int(x) for x in re.findall(r"\bcmd\s*\+\s*(\d+)", rama or "")]
    ok = n == len(etiqueta) and bool(desp) and all(d == len(etiqueta) for d in desp)
    return ok, n, desp


def _contesta(rama):
    return re.findall(r'"(\$(?:ACK|ERR)[^"]*)"', rama)


# --- 11/09 (D-26): lo que la regla nueva necesita para ser verdad -----------------

def _sin_comentarios(t):
    """El fuente sin comentarios: aqui los comentarios CITAN lo que explican
    ("reloj_ajustar() ... CMD_HORA_S de main.cpp"), y un censo de llamadores que los
    contara saldria inflado. Se respetan las cadenas, que tambien pueden llevar '//'."""
    fuera, i, n = [], 0, len(t)
    while i < n:
        c = t[i]
        if c == '"':
            j = i + 1
            while j < n and t[j] != '"':
                j += 2 if t[j] == "\\" else 1
            fuera.append(t[i:j + 1])
            i = j + 1
        elif t.startswith("//", i):
            j = t.find("\n", i)
            i = n if j < 0 else j
        elif t.startswith("/*", i):
            j = t.find("*/", i + 2)
            i = n if j < 0 else j + 2
        else:
            fuera.append(c)
            i += 1
    return "".join(fuera)


def _funciones(codigo):
    """[(nombre, inicio_del_cuerpo, fin_del_cuerpo)] de las DEFINICIONES del fichero."""
    fuera = []
    for m in re.finditer(r"(?m)^[A-Za-z_][\w \t\*&:<>]*?\b([A-Za-z_]\w*)\s*\([^;{}]*\)\s*\{",
                         codigo):
        if m.group(1) in ("if", "while", "for", "switch"):
            continue
        fin = _fin_del_bloque(codigo, m.end() - 1)
        if fin > 0:
            fuera.append((m.group(1), m.end() - 1, fin))
    return fuera


def _donde(codigo, patron):
    """[(funcion, posicion)] de cada aparicion de `patron` en el codigo SIN comentarios."""
    limpio = _sin_comentarios(codigo)
    funcs = _funciones(limpio)
    fuera = []
    for m in re.finditer(patron, limpio):
        f = next((nombre for nombre, a, z in funcs if a < m.start() < z), None)
        fuera.append((f, m.start(), limpio))
    return fuera


def _llamadas(codigos, nombre):
    """[(fichero, funcion_que_llama)] de las LLAMADAS a `nombre` en {fichero: codigo}.

    Sin comentarios y sin la definicion ni las declaraciones -llevan el tipo delante-."""
    fuera = []
    for f, cod in sorted(codigos.items()):
        limpio = _sin_comentarios(cod)
        funcs = _funciones(limpio)
        for m in re.finditer(r"\b%s\s*\(" % re.escape(nombre), limpio):
            antes = limpio[:m.start()].rstrip()
            if re.search(r"\b(?:void|bool|int|uint\d+_t|unsigned\s+long|char)\s*\**$", antes):
                continue
            quien = next((n for n, a, z in funcs if a < m.start() < z), None)
            fuera.append((f, quien))
    return fuera


def _guarda_de_radio(cuerpo, bandera, constante):
    """(ok, motivo): el cuerpo de la guarda es UN return que compara `bandera` con la fuente
    de la radio y un instante contra `constante` -la definicion de "sin radio"-."""
    m = re.fullmatch(r"\s*return\s+([^;]+);\s*", cuerpo or "")
    if not m:
        return False, "no es un unico `return`"
    expr = m.group(1)
    if not re.search(r"\b%s\s*>=?\s*FH_RADIO\b|\b%s\s*==\s*FH_RADIO\b" % (bandera, bandera), expr):
        return False, "no pregunta si la hora es de radio (%s frente a FH_RADIO)" % bandera
    if not re.search(r"millis\s*\(\s*\)\s*-\s*\w+\s*\)?\s*<=\s*%s\b" % constante, expr):
        return False, "no mide el silencio de la radio contra %s" % constante
    return True, expr.strip()


def _normalizado(t):
    """Texto sin comentarios y sin espacios: para comparar dos copias de un bloque."""
    return re.sub(r"\s+", "", _sin_comentarios(t))


def _renueva(rama):
    return [f for f in RENUEVAN_48H if re.search(r"\b%s\s*\(" % f, rama)]


def _escrituras_rtc(reloj_c):
    """(escrituras, ajenas) del RTC hardware en un reloj.cpp. Ver el borde en la 14.

    `escrituras` son todas las llamadas que escriben el contador o el calendario del RTC
    (rtc.set<Algo> menos setClockSource, HAL_RTC_Set*, RTC_WriteTimeCounter), en el fuente
    sin comentarios; `ajenas`, las que NO son la de reloj_fijarEnero() detras de su
    `if (tBaseMillis > 0) return;`."""
    limpio = _sin_comentarios(reloj_c)
    funcs = _funciones(limpio)
    escrituras = []
    for m in re.finditer(r"\brtc\.(set\w+)\s*\(|\b(HAL_RTC_Set\w+|RTC_WriteTimeCounter)\s*\(",
                         limpio):
        if m.group(1) == "setClockSource":
            continue
        quien = next((n for n, a, z in funcs if a < m.start() < z), None)
        escrituras.append((quien, m.group(1) or m.group(2), m.start()))
    ajenas = []
    for quien, que, pos in escrituras:
        if quien == "reloj_fijarEnero":
            fe = next(((a, z) for n, a, z in funcs if n == quien), None)
            cuerpo_fe = limpio[fe[0]:fe[1]] if fe else ""
            guarda = re.search(r"if\s*\(\s*tBaseMillis\s*>\s*0\s*\)\s*return\s*;", cuerpo_fe)
            if guarda is not None and fe[0] + guarda.start() < pos:
                continue
        ajenas.append("%s en %s()" % (que, quien))
    return escrituras, ajenas


# ---------------------------------------------------------------------------------

def correr(b, fw):
    b.titulo("D-20 / D-26: la hora del ESP32 entra por su rama, y en el Esclavo solo si la "
             "radio no manda")

    # ---- 0. LOS SUJETOS EXISTEN ------------------------------------------------
    bt = {p: fw.codigo(*BLUETOOTH[p]) for p in PUNTAS}
    rl = {p: fw.codigo(*RELOJ[p]) for p in PUNTAS}
    ramas = {p: _rama(bt[p], RAMA) for p in PUNTAS}
    for p in PUNTAS:
        if ramas[p] is None:
            raise fw.Abortado(
                "%s: no se hallo la rama %s en bluetooth.cpp. Es por donde la hora del ESP32 "
                "entra a esta punta desde el 11/09: sin ella este pack no tiene nada que medir"
                % (p, RAMA))
        for f in (SEMBRADOR, "reloj_ajustarConAcuse"):
            if _cuerpo(rl[p], f) is None:
                raise fw.Abortado("%s: no se hallo la definicion de %s() en reloj.cpp" % (p, f))
    cg = _cuerpo(rl[PUNTA_GUARDA], GUARDA)
    if cg is None or not re.search(r"\b%s\b" % BANDERA, cg):
        raise fw.Abortado(
            "%s: no se hallo %s() en reloj.cpp, o ya no lee %s. La comprobacion 6 busca esa "
            "bandera en el camino compartido, y con otra forma estaria buscando un nombre "
            "que ya no decide nada" % (PUNTA_GUARDA, GUARDA, BANDERA))

    # LA LINEA DEL CONTRATO, DERIVADA DEL OTRO FIRMWARE: el literal del ESP32 compuesto con
    # los maximos de sus rangos. Antes era un 34 escrito aqui.
    mf = re.search(r'static\s+const\s+char\s+FORMATO_HORA_ESP32\[\]\s*=\s*"([^"]*)"',
                   fw.codigo(*SIEMBRA_ESP32))
    if mf is None:
        raise fw.Abortado("no se hallo FORMATO_HORA_ESP32 en ESP32_Expansion/src/siembra.cpp: "
                          "sin el no hay contra que medir la forma que el STM32 acepta")
    maxs = tuple(fw.constante(CONTRATO_ESP32, r"#define\s+RTC_%s_MAX\s+(\d+)" % c,
                              "RTC_%s_MAX" % c)
                 for c in ("ANIO", "MES", "DIA", "HORA", "MIN", "SEG"))
    LARGO_LINEA = len(mf.group(1) % maxs)
    for f in RENUEVAN_48H:
        if not any(re.search(r"\b%s\s*\(" % f, fw.codigo(p, "include", h))
                   for p in PUNTAS for h in fw.fuentes_de(p, "include", ".h")):
            raise fw.Abortado(
                "%s() no esta declarada en ningun include: es sujeto de la comprobacion 4 y "
                "una regla sobre un sujeto que no existe no mide nada (N-96)" % f)

    # ---- 1..4. LA RAMA DE LAS DOS PUNTAS ---------------------------------------
    for p in PUNTAS:
        desp = _despachador(bt[p]) or ""
        b.verificar(
            _antes_del_pin(desp, RAMA),
            "%s / %s: se compara ANTES que el filtro del PIN: la linea del ESP32 no cae en "
            "AUTH_FAILED" % (p, RAMA),
            "%s / %s: la rama esta detras del filtro del PIN, o no hay filtro. El ESP32 la "
            "manda sin PIN -el puente la tira si viene del telefono- y cada hora sacaria "
            "$ERR,CMD:AUTH_FAILED: el rechazo que nadie puede apagar" % (p, RAMA))

        ok, n, desp_ = _prefijo_cuadra(bt[p], ramas[p], RAMA)
        b.verificar(
            ok,
            "%s / %s: el strncmp corta %d y la siembra lee desde cmd + %d: casan con los %d "
            "caracteres del prefijo" % (p, RAMA, n, desp_[0] if desp_ else -1, len(RAMA)),
            "%s / %s: el prefijo mide %d y el strncmp corta %s con desplazamiento(s) %s. Un "
            "numero contado a mano que no casa hace que la siembra lea a partir de otro "
            "caracter" % (p, RAMA, len(RAMA), n, desp_))

        contesta = _contesta(ramas[p])
        b.verificar(
            not contesta,
            "%s / %s: la rama no contesta ni $ACK ni $ERR: no la origino el telefono y "
            "nadie espera acuse" % (p, RAMA),
            "%s / %s: la rama contesta %s. Esa respuesta le llega al telefono sin que nadie "
            "la pidiera -y un $ERR por hora sale en rojo en la app: el rechazo que nadie "
            "puede apagar-" % (p, RAMA, contesta))

        renueva = _renueva(ramas[p])
        b.verificar(
            not renueva,
            "%s / %s: la rama no renueva el limite de 48 h del Degradado (ni %s)"
            % (p, RAMA, " ni ".join("%s()" % f for f in RENUEVAN_48H)),
            "%s / %s: la rama llama a %s. Una hora del DS3231 de este poste no demuestra "
            "que las dos puntas sigan en fase, y renovar ahi las 48 h autorizaria el "
            "Degradado sobre una sincronizacion con el Maestro que no ocurrio"
            % (p, RAMA, ", ".join(renueva)))

    # ---- 5. EN EL ESCLAVO, LA GUARDA VA ANTES Y LA SIEMBRA VIVE EN SU ELSE -------
    motivo = _orden_guarda_siembra(ramas[PUNTA_GUARDA], GUARDA, SEMBRADOR)
    b.verificar(
        motivo is None,
        "%s / %s: se pregunta a %s() ANTES de sembrar, y %s() solo corre en su else: con "
        "la radio mandando -hora del Maestro y radio oida- la del ESP32 no entra"
        % (PUNTA_GUARDA, RAMA, GUARDA, SEMBRADOR),
        "%s / %s: %s. Con la radio viva, la del DS3231 de este poste pisaria la del "
        "Maestro, y los dos DS3231 no se sincronizan nunca entre si (D-26 (3))"
        % (PUNTA_GUARDA, RAMA, motivo))

    # ---- 6. LA GUARDA NO SE CUELA EN EL CAMINO COMPARTIDO -----------------------
    #
    # El sembrador lo comparten las dos puntas -el mismo fichero en las dos-, y el validador
    # lo usa ademas la radio del Esclavo (reloj_ajustar). Una consulta a la guarda ahi dentro
    # haria que el Maestro ignorase a su ESP32 -D-20: la acepta siempre- y que la radio
    # dejara de sobrescribir la hora del Esclavo. La guarda es de la RAMA del Esclavo.
    for p in PUNTAS:
        coladas = {f: _consulta_la_guarda(_cuerpo(rl[p], f), GUARDA, BANDERA)
                   for f in (SEMBRADOR, "reloj_ajustarConAcuse")}
        coladas = {f: v for f, v in coladas.items() if v}
        b.verificar(
            not coladas,
            "%s: ni %s() ni reloj_ajustarConAcuse() preguntan de quien es la hora (%s() / "
            "%s): el Maestro acepta siempre la de su ESP32 y la radio sobrescribe siempre la "
            "del Esclavo" % (p, SEMBRADOR, GUARDA, BANDERA),
            "%s: %s consulta(n) la guarda del Esclavo. Metida en el camino compartido, el "
            "Maestro ignoraria a su ESP32 una vez en hora (D-20 dice que la acepta siempre) "
            "y la radio dejaria de sobrescribir la hora del Esclavo" % (
                p, ", ".join("%s() -> %s" % (f, "/".join(v)) for f, v in coladas.items())))

    # ---- 7. LA SIEMBRA SOLO LEE LA FORMA EXACTA --------------------------------
    patrones = {}
    for p in PUNTAS:
        mot, patrones[p] = _formato_estricto(rl[p], SEMBRADOR)
        b.verificar(
            mot is None and len(RAMA) + len(patrones[p] or "") == LARGO_LINEA,
            "%s: %s() rechaza por la forma exacta antes de sscanf -patron %r, terminador "
            "comprobado- y prefijo + patron = %d caracteres, la linea del contrato"
            % (p, SEMBRADOR, patrones[p], LARGO_LINEA),
            "%s: %s. Este STM32 no comprueba checksum de entrada: una linea truncada, o "
            "pegada a un $LATIDO, da seis campos a sscanf con los segundos mal (prefijo %d + "
            "patron %d contra los %d del contrato)"
            % (p, mot or "el patron no suma la linea del contrato", len(RAMA),
               len(patrones[p] or ""), LARGO_LINEA))

    # ---- 8. D-26 (3): "SIN RADIO" ES EL MISMO SILENCIO QUE LA ALARMA FALLO_RF ----------
    #
    # EL BORDE, ESCRITO: la guarda es UN return que pregunta si la hora es de radio
    # (fuenteHora frente a FH_RADIO) y si la radio se oyo dentro de SFTY6_SILENCIO_MS. Y esa
    # constante es la MISMA con la que main.cpp declara la orfandad y publica $ALARM
    # FALLO_RF -la alarma que manda al usuario a poner la hora a este poste (D-26 (5))-: con
    # dos umbrales habria un hueco en el que la alarma ya salio y la hora que el usuario
    # pone se sigue ignorando.
    ok8, mot8 = _guarda_de_radio(cg, BANDERA, "SFTY6_SILENCIO_MS")
    main_e = _sin_comentarios(fw.codigo(*MAIN_E))
    m_orf = re.search(r"if\s*\([^{;]*millis\s*\(\s*\)\s*-\s*tUltimoComando\s*>\s*"
                      r"(SFTY6_SILENCIO_MS)\s*\)\s*\{", main_e)
    bloque_orf = _bloque(main_e, m_orf.end() - 1) if m_orf else ""
    alarma_rf = re.search(r'\b%s\s*\(\s*"FALLO_RF"' % ALARMA, bloque_orf or "") is not None
    b.verificar(
        ok8 and m_orf is not None and alarma_rf,
        "Esclavo: %s() es `return %s;` -la radio manda solo si la hora es suya y se oyo en "
        "SFTY6_SILENCIO_MS-, y main.cpp declara la orfandad con esa MISMA constante y publica "
        "ahi $ALARM FALLO_RF: cuando la alarma sale, la hora del ESP32 entra" % (GUARDA, mot8),
        "Esclavo: %s. Si 'sin radio' no es el silencio de FALLO_RF (orfandad leida: %s, "
        "FALLO_RF en su bloque: %s), la alarma que manda al usuario a poner la hora y la "
        "regla que la deja entrar miden cosas distintas" % (
            mot8 if not ok8 else "la guarda esta bien pero la orfandad no casa",
            m_orf is not None, alarma_rf))

    # ---- 9. QUIEN AVISA DE QUE LA RADIO LLEGA: main.cpp, con CADA trama ---------------
    srcs_e = {f: fw.codigo("Esclavo", "src", f) for f in fw.fuentes_de("Esclavo", "src", ".cpp")}
    ll_notar = _llamadas(srcs_e, NOTAR)
    m_pkt = re.search(r"if\s*\(\s*protocolo_hayPaqueteDisponible\s*\(\s*&\s*pkt\s*\)\s*\)\s*\{",
                      main_e)
    primero = ""
    if m_pkt:
        primero = (_bloque(main_e, m_pkt.end() - 1) or "").strip().split(";")[0]
    cuerpo_notar = _cuerpo(rl["Esclavo"], NOTAR) or ""
    b.verificar(
        ll_notar == [("main.cpp", "loop")] and re.fullmatch(r"%s\s*\(\s*\)" % NOTAR, primero)
        and not re.search(r"\b%s\b" % BANDERA, cuerpo_notar),
        "Esclavo: %s() tiene UN llamador -loop() de main.cpp- y es lo PRIMERO del bloque de "
        "cada trama valida de la radio, antes de mirar el comando; y no toca la fuente de la "
        "hora" % NOTAR,
        "Esclavo: %s() se llama desde %s, y lo primero del bloque de la trama es %r. Si "
        "no avisa con CADA trama, 'sin radio' se da con la radio sana y la hora del ESP32 pisa "
        "la del Maestro; si avisa desde otro sitio, el silencio que mide ya no es el de la radio"
        % (NOTAR, ll_notar, primero))

    # ---- 10. QUIEN MARCA LA FUENTE: SOLO LOS DOS SEMBRADORES, Y SOLO SI ENTRO ---------
    #
    # La fuente dice DE QUIEN es la hora. Si la marcara otro -o la marcara aunque la hora no
    # entrara-, la guarda de la 8 contestaria sobre una hora que no es la que hay. Y
    # reloj_ajustar() solo significa "radio" mientras su UNICO llamador sea CMD_HORA_S: esa
    # afirmacion del reloj.h se mide aqui.
    marcas = {}
    for fuente in ("FH_RADIO", "FH_ESP32"):
        marcas[fuente] = [f for f, _, _ in
                          _donde(fw.codigo(*RELOJ["Esclavo"]),
                                 r"\b%s\s*=\s*%s\s*;" % (BANDERA, fuente))]
    cuerpo_aj = _cuerpo(rl["Esclavo"], RADIO_AJUSTA) or ""
    cuerpo_se = _cuerpo(rl["Esclavo"], SEMBRADOR) or ""
    # Los argumentos llevan sus casts -"(int)hora"-: un nivel de parentesis dentro.
    radio_bajo_if = re.search(r"if\s*\(\s*reloj_ajustarConAcuse\s*\((?:[^()]|\([^()]*\))*\)\s*\)"
                              r"\s*\{\s*%s\s*=\s*FH_RADIO\s*;" % BANDERA, cuerpo_aj) is not None
    esp_bajo_if = re.search(r"const\s+bool\s+(\w+)\s*=\s*reloj_ajustarConAcuse\s*\([^)]*\)\s*;"
                            r"\s*if\s*\(\s*\1\s*\)\s*%s\s*=\s*FH_ESP32\s*;\s*return\s+\1\s*;"
                            % BANDERA, cuerpo_se) is not None
    ll_aj = _llamadas(srcs_e, RADIO_AJUSTA)
    m_hs = re.search(r"pkt\.command\s*==\s*CMD_HORA_S\s*\)\s*\{", main_e)
    rama_hs = _bloque(main_e, m_hs.end() - 1) if m_hs else ""
    ll_se = _llamadas(srcs_e, SEMBRADOR)
    b.verificar(
        marcas["FH_RADIO"] == [RADIO_AJUSTA] and marcas["FH_ESP32"] == [SEMBRADOR]
        and radio_bajo_if and esp_bajo_if
        and ll_aj == [("main.cpp", "loop")] and re.search(r"\b%s\s*\(" % RADIO_AJUSTA, rama_hs or "")
        and ll_se == [("bluetooth.cpp", "procesarComando")],
        "Esclavo: FH_RADIO la marca solo %s() y FH_ESP32 solo %s(), las dos DENTRO del si de "
        "su validador; %s() tiene un solo llamador -la rama CMD_HORA_S de main.cpp- y %s() "
        "otro -la rama %s-: la fuente dice la verdad" % (RADIO_AJUSTA, SEMBRADOR, RADIO_AJUSTA,
                                                         SEMBRADOR, RAMA),
        "Esclavo: la fuente de la hora no la marcan solo los dos sembradores o no solo si "
        "entro (FH_RADIO en %s, bajo el if: %s; FH_ESP32 en %s, bajo el if: %s), o "
        "%s() tiene otros llamadores (%s; CMD_HORA_S: %s) o %s() los tiene (%s)"
        % (marcas["FH_RADIO"], radio_bajo_if, marcas["FH_ESP32"], esp_bajo_if, RADIO_AJUSTA,
           ll_aj, bool(rama_hs), SEMBRADOR, ll_se))

    # ---- 11. EL DOBLE DEL ARNES DEL PUENTE RECHAZA LO MISMO QUE EL FIRMWARE -----------
    #
    # simulador_puente_esp32.py compila el bluetooth.cpp REAL contra un reloj de arnes. Si
    # su sembrador aceptara una linea que el firmware tira, la rama de la alarma de D-26 (5)
    # se ejerceria por su camino de exito. Se comparan PATRON_ISO e isoBienFormado() de las
    # tres copias, sin comentarios ni espacios.
    arnes = fw.codigo(*ARNES_PUENTE)
    copias = {}
    for nombre, cod in (("arnes", arnes), ("Maestro", rl["Maestro"]), ("Esclavo", rl["Esclavo"])):
        mp = re.search(r'\bPATRON_ISO\s*\[\s*\]\s*=\s*"[^"]*"', cod)
        cu = _cuerpo(cod, "isoBienFormado")
        se = _cuerpo(cod, SEMBRADOR) or ""
        usa = re.search(r"!\s*isoBienFormado\s*\(\s*str\s*\)", se) is not None
        copias[nombre] = (_normalizado(mp.group(0)) if mp else None,
                          _normalizado(cu) if cu else None, usa)
    iguales = (copias["arnes"][0] is not None and copias["arnes"][1] is not None
               and copias["arnes"][:2] == copias["Maestro"][:2] == copias["Esclavo"][:2]
               and all(c[2] for c in copias.values()))
    b.verificar(
        iguales,
        "el reloj de arnes del simulador del puente lleva el MISMO PATRON_ISO y el MISMO "
        "isoBienFormado() que las dos puntas, y su sembrador lo pregunta como ellas: el doble "
        "rechaza lo que el firmware rechaza",
        "el doble del arnes del puente no rechaza lo mismo que el firmware (patron/cuerpo/uso "
        "por copia: %s). El simulador mediria la rama CMD:HORA_ESP32 por un camino que el "
        "equipo no toma" % {k: (v[0] is not None, v[1] is not None, v[2])
                            for k, v in copias.items()})

    # ---- 12. D-26 (5): LA ALARMA DEL ENLACE ESP32 <-> STM32 EN LAS DOS PUNTAS ----------
    #
    # Tres causas en el vigilante -cada una con su literal EN SU RAMA, N-89- mas la del
    # rechazo en la rama del despachador; la espera es la constante de reloj.h, la
    # distincion J17_MUDO usa el umbral de J17 que ya existe, y el vigilante corre en cada
    # vuelta de bluetooth_loop(). En el Esclavo, ademas, la alarma de la hora no puede dejar
    # armada la bandera de la radio: si no, cada una sacaria un "ENLACE_RF RECUPERADO"
    # falso con la siguiente trama.
    CAUSAS = ("RECHAZADA_FORMATO", "J17_MUDO", "SIN_HORA_DEL_ESP32")
    for p in PUNTAS:
        bt = _sin_comentarios(fw.codigo(*BLUETOOTH[p]))
        vig = _cuerpo(bt, "horaEsp32Vigilar") or ""
        lp = _cuerpo(bt, "bluetooth_loop") or ""
        literales_vig = re.findall(r'\b%s\s*\(\s*"HORA_ESP32"\s*,\s*"([A-Z0-9_]+)"' % ALARMA, vig)
        rechazo_en_rama = re.search(r'\belse\s*\{[^}]*\b%s\s*\(\s*"HORA_ESP32"\s*,\s*'
                                    r'"RECHAZADA_FORMATO"' % ALARMA,
                                    _rama(bt, RAMA) or "", re.S) is not None
        ok12 = (sorted(literales_vig) == sorted(CAUSAS)
                and "HORA_ESP32_ESPERA_MAX_MS" in vig and "J17_SILENCIO_MIN_MS" in vig
                and len(re.findall(r"\bhoraEsp32Vigilar\s*\(\s*ahora\s*\)", lp)) == 1
                and rechazo_en_rama)
        if p == "Esclavo":
            ra = _cuerpo(bt, ALARMA) or ""
            ok12 = ok12 and re.search(r'if\s*\(\s*strcmp\s*\(\s*evento\s*,\s*"FALLO_RF"\s*\)\s*'
                                      r'==\s*0\s*\)\s*enlaceCaidoAnunciado\s*=\s*true\s*;', ra) \
                is not None and not re.search(r"(?m)^\s*enlaceCaidoAnunciado\s*=\s*true\s*;", ra)
        b.verificar(
            ok12,
            "%s: la alarma de D-26 (5) tiene sus tres causas en el vigilante (%s) y la del "
            "rechazo en la rama; espera HORA_ESP32_ESPERA_MAX_MS, distingue J17_MUDO con "
            "J17_SILENCIO_MIN_MS y corre en cada bluetooth_loop()%s"
            % (p, ", ".join(sorted(literales_vig)),
               "" if p != "Esclavo" else "; y solo FALLO_RF arma la bandera de la radio"),
            "%s: la alarma de D-26 (5) no esta entera -causas en el vigilante %s, rechazo en la "
            "rama: %s-. Un J17 suelto dejaria a esta punta extrapolando con el HSI sin que "
            "nadie lo sepa, o la alarma de la hora pintaria una radio recuperada que no cayo"
            % (p, literales_vig, rechazo_en_rama))

    # ---- 13. D-26 (4): EL INSTRUMENTO QUE EJECUTA EL SALTO EXISTE ---------------------
    #
    # Un pack de texto no ve un defecto del TIEMPO (CLAUDE.md 6.3): el salto de hora que pasa
    # por rojo lo MIDE el bloque E del arnes del Degradado sobre el modo_degradado.cpp REAL de
    # las dos puntas. Aqui solo se vigila que ese bloque siga ahi y salte las DOS puntas.
    orq = fw.codigo(*ORQ_DEG)
    adapt = fw.codigo(*ADAPT_E)
    b.verificar(
        re.search(r'MAESTRO\.orden\(\s*"desviar_rtc"\s*,\s*SALTO_GRANDE\s*\)', orq)
        and re.search(r'ESCLAVO\.orden\(\s*"desviar_rtc"\s*,\s*SALTO_GRANDE\s*\)', orq)
        and re.search(r'strcmp\s*\(\s*que\s*,\s*"desviar_rtc"\s*\)', adapt),
        "el bloque E de orquestador_degradado.cpp salta la hora del Maestro y la del Esclavo "
        "con el Degradado ciclando: D-26 (4) se mide ejecutando, no leyendo",
        "el bloque E del arnes del Degradado ya no salta las dos puntas: la regla de D-26 (4) "
        "se quedaria sin instrumento que la EJECUTE, y un pack de texto no ve el tiempo")

    # ---- 14. LA SIEMBRA NO ESCRIBE EL RTC HARDWARE (D-26, 11/09) -------------------------
    #
    # Con una siembra cada ~5 min, escribir el RTC en cada una es lo que medio la cinta del
    # Sisga (179DB0): cada rtc.setX() espera RTOFF hasta 1 s con el RTC parado, ~3 s por
    # siembra con el perro en 4, y ademas reescribe el CNT que fecha el respaldo. Ningun arnes
    # enlaza reloj.cpp -incluye <STM32RTC.h>-, asi que esta es la unica red contra quien
    # vuelva a poner el bloque "if (rtcOperativo) { rtc.setHours(); ... }".
    #
    # EL BORDE, ESCRITO: cuenta como ESCRITURA del RTC cualquier llamada rtc.set<Algo>( y
    # cualquier HAL_RTC_Set*/RTC_WriteTimeCounter, en el fuente SIN comentarios -los de este
    # fichero citan el bloque retirado-. La unica que no escribe el contador es
    # setClockSource (elige el oscilador), y se admite. La unica escritura admitida es la de
    # reloj_fijarEnero() -el mes del calendario del RTC-, y SOLO detras de su
    # `if (tBaseMillis > 0) return;`: con base sembrada no corre nunca.
    for p in PUNTAS:
        escrituras, ajenas = _escrituras_rtc(rl[p])
        b.verificar(
            not ajenas,
            "%s: reloj.cpp no escribe el RTC hardware en ningun camino de la siembra (%d "
            "escritura(s) de contador en todo el fichero; la unica admitida es la de "
            "reloj_fijarEnero() detras de `if (tBaseMillis > 0) return;`)" % (p, len(escrituras)),
            "%s: reloj.cpp vuelve a ESCRIBIR el RTC hardware fuera de la unica escritura "
            "admitida: %s. Con la hora sembrada cada ~5 min, cada rtc.setX() con el RTC parado "
            "espera RTOFF hasta 1 s -~3 s por siembra con el perro en 4 s, la cinta del Sisga- y "
            "reescribe el CNT que fecha las 48 h del Degradado" % (p, ", ".join(ajenas)))

    # ---- CONTROLES NEGATIVOS ---------------------------------------------------
    al_reves = '{ if (sembrar(cmd + 15)) { a(); } else if (enHora()) { b(); } }'
    dentro = '{ if (enHora()) { sembrar(cmd + 15); } }'
    sin_else = '{ if (enHora()) { a(); } if (sembrar(cmd + 15)) { b(); } }'
    bien = '{ if (enHora()) { a(); } else if (sembrar(cmd + 15)) { b(); } else { c(); } }'
    b.control_negativo(
        _orden_guarda_siembra(al_reves, "enHora", "sembrar") is not None
        and _orden_guarda_siembra(dentro, "enHora", "sembrar") is not None
        and _orden_guarda_siembra(sin_else, "enHora", "sembrar") is not None
        and _orden_guarda_siembra(bien, "enHora", "sembrar") is None,
        "la 5 acusa la siembra antes de la guarda, dentro de su bloque y detras sin `else`, y "
        "deja pasar la cadena if / else if / else del firmware: mide el ORDEN, no la presencia")

    b.control_negativo(
        _consulta_la_guarda(" if (horaValida) return false; return val(h); ",
                            "enHora", "horaValida") == ["horaValida"]
        and _consulta_la_guarda(" if (enHora()) return false; ", "enHora", "horaValida")
        == ["enHora()"]
        and not _consulta_la_guarda(" x = 1; horaValida = true; return true; ",
                                    "enHora", "horaValida"),
        "la 6 acusa un sembrador que mira la bandera o llama a la guarda, y NO acusa al "
        "validador por ASIGNAR horaValida, que es su trabajo")

    FALLO_FORMA = (
        'static const char PAT[] = "0000-00-00,00:00:00";\n'
        'static bool bien(const char* s) {\n'
        '  for (int i = 0; i < 19; i++) { if (PAT[i] != s[i]) return false; }\n'
        '  return s[sizeof(PAT) - 1] == \'\\0\';\n}\n'
        'bool sembrar(const char* str) {\n  int a, b, c, h, m, s;\n'
        '  if (sscanf(str, "%d-%d-%d,%d:%d:%d", &a, &b, &c, &h, &m, &s) != 6) return false;\n'
        '  if (!bien(str)) return false;\n  return val(h, m, s, c);\n}\n')
    CORTO = FALLO_FORMA.replace('"0000-00-00,00:00:00"', '"0000-00-00,00:00:0"').replace(
        '  if (sscanf', '  if (!bien(str)) return false;\n  if (sscanf', 1)
    SIN_FIN = CORTO.replace('"0000-00-00,00:00:0"', '"0000-00-00,00:00:00"').replace(
        "  return s[sizeof(PAT) - 1] == '\\0';\n", "  return true;\n")
    BUENO = SIN_FIN.replace("  return true;\n", "  return s[sizeof(PAT) - 1] == '\\0';\n")
    b.control_negativo(
        _formato_estricto(FALLO_FORMA, "sembrar")[0] is not None
        and _formato_estricto(CORTO, "sembrar")[0] is not None
        and _formato_estricto(SIN_FIN, "sembrar")[0] is not None
        and _formato_estricto(BUENO, "sembrar")[0] is None,
        "la 7 acusa la comprobacion de forma DETRAS del sscanf, un patron con una cifra de "
        "menos y un comprobador que no mira el terminador, y aprueba el que hace las tres")

    b.control_negativo(
        not _antes_del_pin('if (strncmp(cmd, "CMD:PIN:1234:", 13) != 0) { r(); } '
                           'if (strncmp(cmd, "CMD:HORA_ESP32:", 15) == 0) { x(); }', RAMA)
        and not _prefijo_cuadra('strncmp(cmd, "CMD:HORA_ESP32:", 14)', "{ f(cmd + 14); }",
                                RAMA)[0]
        and _contesta('{ e("$ERR,CMD:HORA_ESP32,DESC:X"); }')
        and _renueva('{ degradado_registrarSync(); }'),
        "una rama detras del PIN cae en la 1, un strncmp con uno de menos en la 2, un $ERR en "
        "la 3 y una renovacion de las 48 h en la 4")

    # 11/09 - LOS CONTROLES DE LO NUEVO, por los MISMOS helpers que las comprobaciones.
    # (8) la regla vieja -"solo sin hora"- y una "sin radio" con OTRO umbral caen; la buena
    # pasa. Son las dos formas de volver atras sin que se note.
    vieja = " return horaValida; "
    otro_umbral = (" return fuenteHora >= FH_RADIO && radioOida && "
                   "(uint32_t)(millis() - tUltimaRadio) <= 7200000UL; ")
    buena = (" return fuenteHora >= FH_RADIO && radioOida && "
             "(uint32_t)(millis() - tUltimaRadio) <= SFTY6_SILENCIO_MS; ")
    b.control_negativo(
        not _guarda_de_radio(vieja, BANDERA, "SFTY6_SILENCIO_MS")[0]
        and not _guarda_de_radio(otro_umbral, BANDERA, "SFTY6_SILENCIO_MS")[0]
        and _guarda_de_radio(buena, BANDERA, "SFTY6_SILENCIO_MS")[0],
        "la 8 acusa la guarda vieja ('return horaValida', solo sin hora) y la de las '2 h "
        "sin sembrar' que D-26 sustituyo, y aprueba la que mide el silencio de FALLO_RF")

    # (9/10) el censo de llamadores no cuenta comentarios -aqui CITAN lo que explican- ni la
    # definicion, y SI cuenta una llamada nueva desde otro fichero.
    sint = {"a.cpp": "// reloj_ajustar(1,2,3,4) en un comentario\n"
                     "void reloj_ajustar(uint8_t h) {\n  x();\n}\n",
            "b.cpp": "void loop() {\n  if (y) {\n    reloj_ajustar(1, 2, 3, 4);\n  }\n}\n",
            "c.cpp": "static void menu() {\n  reloj_ajustar(0, 0);\n}\n"}
    b.control_negativo(
        _llamadas(sint, "reloj_ajustar") == [("b.cpp", "loop"), ("c.cpp", "menu")],
        "el censo de la 10 ve los dos llamadores reales -uno nuevo desde otro fichero- y no "
        "cuenta ni el comentario que lo cita ni la definicion: un segundo llamador de "
        "reloj_ajustar() haria que 'hora de radio' mintiera, y se veria")

    # (11) una copia del patron con una cifra de menos en el arnes se distingue.
    b.control_negativo(
        _normalizado('static const char PATRON_ISO[] = "0000-00-00,00:00:0";')
        != _normalizado('static const char PATRON_ISO[] = "0000-00-00,00:00:00";')
        and _normalizado("x = 1; // comentario\n  y = 2;") == _normalizado("x=1;y=2;"),
        "la 11 compara las copias sin comentarios ni espacios, y un patron con una cifra de "
        "menos en el doble del arnes NO sale igual")

    # (14) el bloque retirado, puesto otra vez en el validador, se caza; el mismo setMonth
    # con su guarda en reloj_fijarEnero() pasa, sin ella no; y el que solo lo CITA en un
    # comentario no cuenta. Por el MISMO helper que la comprobacion.
    fe_bien = ("void reloj_fijarEnero() {\n  if (!horaValida) return;\n"
               "  if (tBaseMillis > 0) return;\n"
               "  if (rtcOperativo && rtc.getMonth() != 1) rtc.setMonth(1);\n}\n")
    fe_sin = fe_bien.replace("  if (tBaseMillis > 0) return;\n", "")
    vuelve = ("bool reloj_ajustarConAcuse(int hora, int m, int s, int dia) {\n"
              "  horaValida = true;\n"
              "  if (rtcOperativo) { rtc.setHours((uint8_t)hora); rtc.setMinutes(1); }\n"
              "  return true;\n}\n")
    citado = ("bool reloj_ajustarConAcuse(int hora, int m, int s, int dia) {\n"
              "  // aqui habia rtc.setHours((uint8_t)hora);\n  return true;\n}\n")
    arranque = ("void reloj_setup() {\n  rtc.setClockSource(STM32RTC::LSE_CLOCK);\n}\n")
    b.control_negativo(
        _escrituras_rtc(fe_bien + arranque)[1] == []
        and _escrituras_rtc(fe_sin)[1] == ["setMonth en reloj_fijarEnero()"]
        and _escrituras_rtc(vuelve)[1] == ["setHours en reloj_ajustarConAcuse()",
                                          "setMinutes en reloj_ajustarConAcuse()"]
        and _escrituras_rtc(citado + arranque) == ([], []),
        "la 14 acusa el bloque retirado vuelto a poner en el validador y el setMonth de "
        "reloj_fijarEnero() sin su guarda de base sembrada; y no acusa ni ese setMonth con la "
        "guarda, ni setClockSource, ni un comentario que cita el bloque")
