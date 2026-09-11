# ===== banco/packs/reloj_04_hora_que_caduca.py =====
#
# D-21 (1) - "UNA HORA QUE NO ES FIABLE NO ES SIN HORA: ES UNA HORA QUE MIENTE, Y SE
# RESPONDE CON AMBAR INTERMITENTE EN LA PUNTA QUE LA TIENE". LA CADUCIDAD DE LA SIEMBRA.
#
# POR QUE EXISTE (11/09). H1 del veredicto del arquitecto sobre D-26: con el J17 de una punta
# mudo, su hora corre sobre el HSI -hasta 90 s por hora- mientras la otra se siembra de su
# DS3231; en Degradado eso es verde-verde en cada ciclo, y el firmware lo detectaba
# ($ALARM HORA_ESP32,J17_MUDO) sin hacer nada. La guarda de D-21 que ya existia en el bucle
# del Degradado preguntaba reloj_enHora(), que con el J17 mudo sigue en true. Desde hoy la
# puerta y el bucle del Degradado de las dos puntas preguntan reloj_horaFiable(): hora
# sembrada hace como mucho HORA_CADUCA_MS (reloj.h).
#
# QUE MIDE ESTE PACK, Y QUE NO:
#   - LA DESIGUALDAD (N-71). El plazo no se escoge: HORA_CADUCA_MS se escribe en reloj.h como
#     EXPRESION de la cadencia y del HSI, y aqui se EVALUA esa expresion -con aritmetica de
#     32 bits sin signo, la del Cortex-M3- y se contrasta contra el aguante del cruce, barrido
#     con el modelo de costura por el MISMO codigo que esp32_13 (se importa, no se copia).
#     Un comentario no falla cuando alguien cambia un numero; esto si.
#   - QUE LA REGLA TIENE SUJETO Y LLAMADOR (CLAUDE.md 6.1, N-96): la funcion existe en las dos
#     puntas, compara contra la constante, la rejuvenece SOLO una siembra, y la preguntan la
#     puerta y el bucle del Degradado de las dos, con el ambar que ya habia y la alarma.
#   - NO MIDE EL TIEMPO. Un pack de texto no ve un defecto del tiempo (CLAUDE.md 6.3). Que la
#     hora caduque de verdad en su frontera, que la punta pase a ambar sin verde-verde y que
#     no vuelva sola lo EJECUTA el bloque F de Validacion_Automatico/dos_puntas/
#     orquestador_degradado.cpp, sobre el reloj.cpp REAL de las dos puntas. Aqui solo se
#     vigila que ese bloque siga ahi y que el arnes siga compilando el reloj real.
#
# SIN ETIQUETA SFTY: roza SFTY-18 y SFTY-21 pero lo que EJERCE la regla es el arnes, no este
# pack. Una fila cubierta por una prueba que no la ejerce es peor que una vacia.

import ast
import re

from banco.packs.esp32_13_siembra_de_hora import (   # la cuenta del cruce YA PROBADA: se
    _aguante, _deriva_cadencia_s, RESIDUO_SIEMBRA_S,  # importa entera, no se reescribe
    DS3231_PPM, RE_SIEMBRA_ESP32,
)
from banco.modelos.costura import E_AMARILLO_MS, DEG_VERDE_SEG, DEG_DESPEJE_SEG

NOMBRE = "reloj_04_hora_que_caduca"
DESCRIPCION = ("D-21 (1): la caducidad de la siembra se deriva del C++ y cabe entre la "
               "cadencia del ESP32 y el aguante del cruce; la preguntan la puerta y el bucle "
               "del Degradado de las dos puntas, y el arnes que la ejecuta existe")

PUNTAS = ("Maestro", "Esclavo")
RELOJ_H = {p: (p, "include", "reloj.h") for p in PUNTAS}
RELOJ_C = {p: (p, "src", "reloj.cpp") for p in PUNTAS}
DEG_C = {p: (p, "src", "modo_degradado.cpp") for p in PUNTAS}
CONTRATO = ("ESP32_Expansion", "include", "contrato.h")
PROTOCOLO_E = ("Esclavo", "include", "protocolo.h")
ORQ = ("Validacion_Automatico", "dos_puntas", "orquestador_degradado.cpp")
ADAPT = {"Maestro": ("Validacion_Automatico", "dos_puntas", "adaptador_maestro_deg.cpp"),
         "Esclavo": ("Validacion_Automatico", "dos_puntas", "adaptador_esclavo.cpp")}
SCRIPT = ("Validacion_Automatico", "compilar_degradado.ps1")
BT_C = {p: (p, "src", "bluetooth.cpp") for p in PUNTAS}

PREDICADO = "reloj_horaFiable"
# Las constantes que forman el plazo, EN EL ORDEN en que se definen: cada una solo puede
# usar las anteriores.
SIMBOLOS = ("HORA_ESP32_CADENCIA_MS", "HSI_PPM_PEOR", "HORA_DERIVA_S", "HORA_CADUCA_MS")
# Donde se pregunta, por punta: (funcion de la PUERTA, funcion del BUCLE, camino de ambar
# que YA existia y que la guarda tiene que tomar).
DONDE = {"Maestro": ("modo_degradado_evaluarEntrada", "modo_degradado_loop", "irAAmbar"),
         "Esclavo": ("degradado_comprobar", "degradado_actualizar", "iniciarSalida")}

UINT32 = 1 << 32


# --- EL EVALUADOR DE LA EXPRESION DEL C++ -----------------------------------------------
#
# EL BORDE, ESCRITO (CLAUDE.md 7): solo sabe sumas, restas, productos y divisiones de enteros
# NO NEGATIVOS con sufijo U/UL/L y nombres ya evaluados, y trunca cada paso a 32 bits sin
# signo -unsigned long en el Cortex-M3-. Si un paso intermedio DESBORDA, no lo trunca en
# silencio: lo devuelve como fallo, porque en la tarjeta ese numero seria otro. Cualquier
# otra forma (casts, ternarios, llamadas) ABORTA en vez de medir de menos (N-108).

class _NoSabe(Exception):
    pass


def _eval_nodo(n, valores, desbordes):
    if isinstance(n, ast.Constant) and isinstance(n.value, int):
        return n.value
    if isinstance(n, ast.Name):
        if n.id not in valores:
            raise _NoSabe("nombre %s sin valor" % n.id)
        return valores[n.id]
    if isinstance(n, ast.BinOp):
        a = _eval_nodo(n.left, valores, desbordes)
        b = _eval_nodo(n.right, valores, desbordes)
        if isinstance(n.op, ast.Add):
            r = a + b
        elif isinstance(n.op, ast.Sub):
            r = a - b
        elif isinstance(n.op, ast.Mult):
            r = a * b
        elif isinstance(n.op, ast.FloorDiv):
            if b == 0:
                raise _NoSabe("division por cero")
            r = a // b
        else:
            raise _NoSabe("operador %s" % type(n.op).__name__)
        if r < 0 or r >= UINT32:
            desbordes.append(r)
            r %= UINT32
        return r
    raise _NoSabe("nodo %s" % type(n).__name__)


def evaluar_c(expr, valores):
    """(valor, desbordes) de una expresion entera del C++ con aritmetica de 32 bits."""
    limpia = re.sub(r"\b(\d+)(?:UL|LU|U|L)\b", r"\1", expr.strip())
    if not re.fullmatch(r"[\w\s+\-*/()]+", limpia):
        raise _NoSabe("caracteres fuera de lo que este evaluador sabe: %r" % expr)
    arbol = ast.parse(limpia.replace("/", "//"), mode="eval")
    desbordes = []
    return _eval_nodo(arbol.body, valores, desbordes), desbordes


def _constantes(fw, punta):
    """{simbolo: valor} de reloj.h de la punta, evaluando cada expresion en orden."""
    t = fw.codigo(*RELOJ_H[punta])
    valores, desb = {}, {}
    for s in SIMBOLOS:
        m = re.search(r"static\s+const\s+unsigned\s+long\s+%s\s*=\s*([^;]+);" % s, t)
        if m is None:
            raise fw.Abortado("%s/include/reloj.h no define %s: sin el no hay plazo que medir"
                              % (punta, s))
        try:
            valores[s], desb[s] = evaluar_c(m.group(1), valores)
        except _NoSabe as e:
            raise fw.Abortado("%s/include/reloj.h: %s = %r no es una forma que este pack sepa "
                              "evaluar (%s)" % (punta, s, m.group(1).strip(), e))
    return valores, desb


# --- LECTURA POR LLAVES (bloque literal de esp32_13: _bloque / _cuerpo) ------------------
def _bloque(texto, i):
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


def _cuerpo(codigo, firma):
    m = re.search(firma + r"\s*\{", codigo)
    return None if not m else _bloque(codigo, m.end() - 1)


def _cuerpo_fn(codigo, nombre):
    return _cuerpo(codigo, r"\b[\w\s\*]*\b%s\s*\([^;{)]*\)" % re.escape(nombre))


# --- LAS DOS MITADES DE LA DESIGUALDAD. La comprobacion y sus controles negativos pasan por
# ESTAS funciones: un control que ejercita otro camino no controla nada.
def _suelo_ms(cad_ms, ppm):
    """Lo que tarda, en el millis() de un STM32 con el HSI en su extremo rapido, la siembra
    que el ESP32 manda cada cad_ms con su propio reloj."""
    return cad_ms * (1000000 + ppm) / 1000000.0


def _relativa_s(plazo_ms, ppm):
    """Lo que se pueden separar DOS puntas que dan verdes con la hora de hasta plazo_ms:
    la deriva del HSI de cada una en sentidos opuestos, mas el segundo entero de cada
    siembra. La misma cuenta que esp32_13 hace con la cadencia."""
    return 2 * _deriva_cadencia_s(plazo_ms, ppm) + 2 * RESIDUO_SIEMBRA_S


def _guarda_del_bucle(cuerpo_bucle, camino):
    """(ok, motivo): en el bucle, un `if (!reloj_horaFiable() ...) {` cuyo bloque publica la
    alarma HORA_ESP32/CADUCADA DETRAS de `if (reloj_enHora())` y toma el camino de ambar que
    ya habia."""
    m = re.search(r"if\s*\(\s*!\s*%s\s*\(\s*\)[^{]*\)\s*\{" % PREDICADO, cuerpo_bucle or "")
    if m is None:
        return False, "no hay `if (!%s() ...) {` en el bucle" % PREDICADO
    bloque = _bloque(cuerpo_bucle, m.end() - 1) or ""
    ma = re.search(r"if\s*\(\s*reloj_enHora\s*\(\s*\)\s*\)\s*\{", bloque)
    alarma = _bloque(bloque, ma.end() - 1) if ma else None
    if alarma is None or not re.search(
            r'bluetooth_reportarAlarma\s*\(\s*"HORA_ESP32"\s*,\s*"CADUCADA"', alarma):
        return False, ("la guarda no publica $ALARM HORA_ESP32,CADUCADA detras de "
                       "`if (reloj_enHora())`")
    if not re.search(r"\b%s\s*\(" % re.escape(camino), bloque):
        return False, "la guarda no toma el camino de ambar que ya habia (%s())" % camino
    return True, "guarda -> alarma CADUCADA -> %s()" % camino


def correr(b, fw):
    b.titulo("D-21 (1): la hora que caduca")

    # =====================================================================
    b.titulo("1. El plazo sale del C++, igual en las dos puntas y en el ESP32")
    # =====================================================================
    cte, desb = {}, {}
    for p in PUNTAS:
        cte[p], desb[p] = _constantes(fw, p)
    esp = fw.constante(CONTRATO, RE_SIEMBRA_ESP32, "la cadencia de la siembra en el ESP32")
    iguales = all(cte["Maestro"][s] == cte["Esclavo"][s] for s in SIMBOLOS)
    sin_desborde = not any(desb[p][s] for p in PUNTAS for s in SIMBOLOS)
    b.verificar(
        iguales and sin_desborde and cte["Maestro"]["HORA_ESP32_CADENCIA_MS"] == esp,
        "las dos puntas evaluan lo mismo -%s- sin desbordar 32 bits en ningun paso, y la "
        "cadencia es la del ESP32 (%d ms)" % (cte["Maestro"], esp),
        "las dos puntas no dicen lo mismo (%s / %s), o algun paso desborda 32 bits (%s), o la "
        "cadencia no es la del ESP32 (%d). Con dos plazos distintos una punta se rinde y la otra "
        "sigue dando verdes con la misma hora; con un desborde, la tarjeta calcula otro numero"
        % (cte["Maestro"], cte["Esclavo"], desb, esp))

    c = cte["Maestro"]
    P_ms = c["HORA_CADUCA_MS"]
    cad_ms = c["HORA_ESP32_CADENCIA_MS"]
    ppm = c["HSI_PPM_PEOR"]

    # =====================================================================
    b.titulo("2. LA DESIGUALDAD: por encima de la cadencia, por debajo del aguante")
    # =====================================================================
    # EL SUELO. Entre dos siembras el ESP32 cuenta SIEMBRA_INTERVALO_MS con su reloj (el de
    # un ESP32, que es de cristal); el STM32 lo cuenta con millis() sobre el HSI, que en su
    # extremo rapido cuenta (1 + ppm) veces mas. Una siembra normal tiene que llegar ANTES
    # de que la hora caduque, o el Degradado caeria a ambar con el J17 sano.
    suelo_ms = _suelo_ms(cad_ms, ppm)
    b.verificar(
        P_ms > suelo_ms,
        "HORA_CADUCA_MS = %d ms > %.0f ms: una siembra normal -cada %d ms del ESP32, que el "
        "STM32 con el HSI a +%d ppm cuenta como %.0f- llega antes de que la hora caduque"
        % (P_ms, suelo_ms, cad_ms, ppm, suelo_ms),
        "HORA_CADUCA_MS = %d ms y una siembra normal puede tardar %.0f ms en el reloj del STM32: "
        "el Degradado se rendiria a ambar con el J17 sano, en cada cadencia" % (P_ms, suelo_ms))

    # EL TECHO. Con la caducidad, una punta que da verdes tiene como mucho la deriva del
    # HSI durante P, mas el segundo entero de la siembra; las DOS en el peor caso, en sentidos
    # opuestos, tienen que caber en lo que el cruce aguanta. Es la cuenta de esp32_13 con P en
    # vez de con la cadencia: alli era una SUPOSICION -"cada punta se siembra cada cadencia"-,
    # y la caducidad es lo que la hace cumplir.
    aguante = _aguante(DEG_VERDE_SEG, DEG_DESPEJE_SEG, E_AMARILLO_MS // 1000)
    if aguante is None:
        raise fw.Abortado("el barrido del modelo de costura no encontro solape: sin aguante "
                          "no hay contra que comparar el plazo")
    deriva_P = _deriva_cadencia_s(P_ms, ppm)
    deriva_cad = _deriva_cadencia_s(cad_ms, ppm)
    relativa_P = _relativa_s(P_ms, ppm)
    b.verificar(
        relativa_P < aguante,
        "con la caducidad en %d ms, cada punta que da verdes se aparta de su DS3231 como mucho "
        "%d s; las dos en sentidos opuestos, con el segundo entero de cada siembra, %d s: por "
        "debajo de los %d s que el cruce aguanta (barrido del modelo de costura)"
        % (P_ms, deriva_P, relativa_P, aguante),
        "con la caducidad en %d ms las dos puntas pueden separarse %d s antes de que ninguna "
        "caduque, y el cruce aguanta %d: el plazo deja pasar el verde-verde que viene a cerrar"
        % (P_ms, relativa_P, aguante))

    # Y NO SE COME EL PRESUPUESTO DE LOS DS3231. esp32_13 reserva lo que queda del aguante
    # para lo que difieran los dos relojes con pila, que nadie acota. Si la caducidad
    # concediera mas deriva que UNA cadencia, ese margen menguaria sin que nadie lo decidiera.
    b.verificar(
        deriva_P <= deriva_cad,
        "la deriva que la caducidad concede (%d s) es la de UNA cadencia (%d s): el margen que "
        "esp32_13 deja a los dos DS3231 (%d s) no cambia" % (deriva_P, deriva_cad,
                                                           aguante - 2 * deriva_cad - 2 * RESIDUO_SIEMBRA_S),
        "la caducidad concede %d s de deriva y la cuenta de esp32_13 supone %d: el margen de los "
        "dos DS3231 mengua en %d s sin decision de nadie" % (deriva_P, deriva_cad,
                                                           2 * (deriva_P - deriva_cad)))

    # =====================================================================
    b.titulo("3. La regla tiene sujeto y llamador (CLAUDE.md 6.1)")
    # =====================================================================
    for p in PUNTAS:
        rc = fw.codigo(*RELOJ_C[p])
        cuerpo = _cuerpo_fn(rc, PREDICADO) or ""
        compara = re.search(r"millis\s*\(\s*\)\s*-\s*tBaseMillis\s*\)\s*>\s*HORA_CADUCA_MS", cuerpo)
        mira_hora = re.search(r"if\s*\(\s*!\s*horaValida\s*\)\s*return\s+false\s*;", cuerpo)
        aj = _cuerpo_fn(rc, "reloj_ajustarConAcuse") or ""
        rejuvenece = re.findall(r"\bsiembraCaducada\s*=\s*false\s*;", rc)
        rejuvenece_siembra = re.search(r"horaValida\s*=\s*true\s*;\s*siembraCaducada\s*=\s*false\s*;",
                                       aj)
        act = _cuerpo_fn(rc, "reloj_actualizar") or ""
        pos_tick = act.find(PREDICADO)
        pos_salida = act.find("if (rtcOperativo) return;")
        ok = (compara and mira_hora and rejuvenece_siembra and 0 <= pos_tick < pos_salida
              and all(_cuerpo_fn(rc, f) is not None and
                      re.search(r"siembraCaducada\s*=\s*false", _cuerpo_fn(rc, f) or "")
                      for f in ("reloj_setup",))
              and len(rejuvenece) >= 2)
        b.verificar(
            bool(ok),
            "%s: %s() compara la edad de la base contra HORA_CADUCA_MS, exige hora puesta, se "
            "rejuvenece SOLO con una siembra buena (dentro de reloj_ajustarConAcuse(), detras de "
            "horaValida = true) y reloj_actualizar() la mira en cada vuelta antes de su salida "
            "temprana" % (p, PREDICADO),
            "%s: %s() no tiene la forma que la regla necesita (compara con la constante: %s, "
            "exige hora: %s, la rejuvenece la siembra: %s, reloj_actualizar() la mira antes de "
            "salir: %s). Sin cualquiera de las cuatro, la hora caducada vuelve a parecer fiable"
            % (p, PREDICADO, bool(compara), bool(mira_hora), bool(rejuvenece_siembra),
               0 <= pos_tick < pos_salida))

        dg = fw.codigo(*DEG_C[p])
        puerta, bucle, camino = DONDE[p]
        cp = _cuerpo_fn(dg, puerta) or ""
        cb = _cuerpo_fn(dg, bucle) or ""
        puerta_ok = re.search(r"if\s*\(\s*!\s*%s\s*\(\s*\)\s*\)\s*return\b" % PREDICADO, cp)
        vieja = re.search(r"if\s*\(\s*!\s*reloj_enHora\s*\(\s*\)", cp + cb)
        ok_b, mot_b = _guarda_del_bucle(cb, camino)
        b.verificar(
            bool(puerta_ok) and ok_b and not vieja,
            "%s: la PUERTA (%s) y el BUCLE (%s) del Degradado preguntan %s(), y el bucle %s; "
            "ninguno pregunta ya reloj_enHora() a secas" % (p, puerta, bucle, PREDICADO, mot_b),
            "%s: puerta con %s(): %s; bucle: %s; queda alguna guarda con reloj_enHora() a secas: "
            "%s. Una puerta que mira otra cosa que el bucle contesta $ACK a un Degradado que se "
            "rinde en la vuelta siguiente (CLAUDE.md 2); un bucle sin la guarda da verdes con la "
            "hora que miente" % (p, PREDICADO, bool(puerta_ok), mot_b, bool(vieja)))

    # =====================================================================
    b.titulo("4. El instrumento que EJECUTA existe y compila el reloj REAL")
    # =====================================================================
    orq = fw.codigo(*ORQ)
    script = fw.texto(*SCRIPT)
    reales = all(re.search(r"Join-Path\s+\$%s\s+'src\\reloj\.cpp'" % v, script)
                 for v in ("MAESTRO", "ESCLAVO"))
    bloques = all(x in orq for x in ('"hora_fiable"', '"siembra_esp32"', '"hsi_ppm"',
                                     '"alarmas_caducada"', "probarBorde(", '"radio_manda"'))
    b.verificar(
        reales and bloques and "-DARNES_RELOJ_REAL" in script,
        "compilar_degradado.ps1 compila Maestro/src/reloj.cpp y Esclavo/src/reloj.cpp REALES, "
        "y el bloque F del orquestador ejerce la caducidad, el HSI, la alarma, los bordes de "
        "D-26 (4) y reloj_radioManda(): la regla se mide EJECUTANDO",
        "el arnes del Degradado ya no compila el reloj.cpp real (%s) o el bloque F perdio sus "
        "ordenes (%s): la caducidad se quedaria sin instrumento que la ejecute, y un pack de "
        "texto no ve el tiempo" % (reales, bloques))

    # La rama CMD:HORA_ESP32 esta TRANSCRITA en los dos adaptadores -bluetooth.cpp no se
    # compila alli-. Si el firmware cambia el ORDEN de lo que decide, la transcripcion miente.
    orden = {}
    for p in PUNTAS:
        bt = fw.codigo(*BT_C[p])
        mr = re.search(r'strncmp\s*\(\s*cmd\s*,\s*"CMD:HORA_ESP32:"', bt)
        rama = _bloque(bt, bt.find("{", mr.end())) if mr else None
        ad = _cuerpo_fn(fw.codigo(*ADAPT[p]), "ramaHoraEsp32") or ""
        claves = ("reloj_radioManda", "reloj_sembrarDesdeIso", "coordinador_sincronizarHora")

        def secuencia(t):
            pos = [(t.find(k), k) for k in claves if t.find(k) >= 0]
            return [k for _, k in sorted(pos)]
        orden[p] = (secuencia(rama or ""), secuencia(ad.replace("sembrarDirecto", "reloj_sembrarDesdeIso")))
    b.verificar(
        all(orden[p][0] and orden[p][0] == orden[p][1] for p in PUNTAS),
        "la rama CMD:HORA_ESP32 transcrita en los dos adaptadores decide en el MISMO orden que "
        "la de bluetooth.cpp (%s)" % {p: orden[p][0] for p in PUNTAS},
        "la rama transcrita ya no decide como la del firmware (%s): el arnes sembraria por un "
        "camino que el equipo no toma" % orden)

    # =====================================================================
    b.titulo("LO QUE EL PLAZO NO PUEDE COMPRAR (no cuenta)")
    # =====================================================================
    perdida_ms = 2 * cad_ms
    rel_perdida = _relativa_s(perdida_ms, ppm)
    silencio = fw.constante(PROTOCOLO_E, r"#define\s+SFTY6_SILENCIO_MS\s+(\d+)UL",
                            "SFTY6_SILENCIO_MS del Esclavo")
    relevo_ms = cad_ms + silencio + cad_ms
    # El mayor plazo que todavia cabria con los DS3231 IGUALES, y lo que costaria.
    p_max = cad_ms
    while _relativa_s(p_max + 1000, ppm) < aguante:
        p_max += 1000
    margen_hoy = aguante - _relativa_s(cad_ms, ppm)
    margen_pmax = aguante - _relativa_s(p_max, ppm)
    # Probabilidad de caer en el relevo: A = lo que hace de la ultima hora por radio al
    # callarse (0..cad), B = la espera a la primera siembra del ESP32 tras el silencio
    # (0..cad), uniformes e independientes. La hora caduca si A + silencio + B > P.
    lim = (P_ms - silencio) / float(cad_ms)
    prob = 1.0 - (lim * lim / 2.0 if lim <= 1.0 else 1.0 - (2.0 - lim) ** 2 / 2.0)
    b.reportar(
        "D-21 (1): lo que la caducidad NO tolera, con la cuenta",
        ["UNA SIEMBRA PERDIDA manda el Degradado a ambar. Tolerarla pediria un plazo de al menos "
         "dos cadencias (%d ms), y con el las dos puntas podrian separarse %d s contra %d de "
         "aguante: NO CABE, ni con los dos DS3231 iguales. Y el ambar no se levanta solo (D-21): "
         "una siembra comida -el ESP32 reiniciando, un byte- cuesta el Degradado hasta que vuelva "
         "una persona." % (perdida_ms, rel_perdida, aguante),
         "EL RELEVO DE D-26 (3) EN EL ESCLAVO: al callarse la radio, su hora puede tener hasta una "
         "cadencia de la ultima propagacion + SFTY6_SILENCIO_MS + una cadencia hasta la primera "
         "siembra de SU ESP32 = %d ms, MAS que el plazo (%d). Con fases al azar, en torno al %.0f "
         "%% de las veces el Esclavo NO PUEDE ENTRAR en Degradado durante esos primeros minutos "
         "(la puerta dice SIN HORA VALIDA) y, si ya estaba dentro, se rinde. No hay verde-verde: "
         "hay un cruce que no arranca el Degradado hasta que llega la siembra."
         % (relevo_ms, P_ms, 100.0 * prob),
         "EL MAYOR PLAZO QUE CABRIA con los dos DS3231 iguales es %d ms; subirlo de %d a %d deja "
         "el margen de los DS3231 de %d s en %d s (a %d ppm por reloj, de ~%.0f dias a ~%.0f) y "
         "no compra ni la siembra perdida ni el relevo. Por eso el plazo es el de una cadencia: "
         "cualquier otro es una DECISION -que margen de los DS3231 se cede- y no una derivacion."
         % (p_max, P_ms, p_max, margen_hoy, margen_pmax, DS3231_PPM,
            margen_hoy / (2 * DS3231_PPM * 86400 / 1e6),
            margen_pmax / (2 * DS3231_PPM * 86400 / 1e6))])

    # =====================================================================
    b.titulo("CONTROLES NEGATIVOS")
    # =====================================================================
    # El techo sabe caer: con el plazo de dos cadencias la misma cuenta supera el aguante.
    b.control_negativo(
        not (_relativa_s(2 * cad_ms, ppm) < aguante),
        "con un plazo de dos cadencias la cuenta de la 2 da %d s contra %d de aguante: el techo "
        "caza un plazo que tolere una siembra perdida" % (_relativa_s(2 * cad_ms, ppm), aguante))
    # El suelo sabe caer: un plazo igual a la cadencia no deja llegar la siembra con HSI rapido.
    b.control_negativo(
        not (cad_ms > _suelo_ms(cad_ms, ppm)),
        "un plazo igual a la cadencia no pasa el suelo de la 2: con el HSI rapido la siembra "
        "llega %.0f ms despues de caducar" % (_suelo_ms(cad_ms, ppm) - cad_ms))
    # El evaluador lee la expresion REAL y sabe ver un desborde de 32 bits.
    v, d = evaluar_c("(300000UL / 1000UL * 25000UL + 999999UL) / 1000000UL", {})
    v2, d2 = evaluar_c("300000UL * 25000UL / 1000000UL", {})
    b.control_negativo(
        v == 8 and not d and d2 and v2 != 7500,
        "el evaluador da 8 con la forma que usa reloj.h y DETECTA el desborde de 32 bits de "
        "'300000UL * 25000UL' (en la tarjeta daria %d, no 7500)" % v2)
    # La 3 sabe caer: la guarda vieja -reloj_enHora() a secas, sin alarma- no pasa.
    ok_v, _ = _guarda_del_bucle(
        '{ if (!reloj_enHora()) { irAAmbar("x", "y"); return; } }', "irAAmbar")
    ok_s, _ = _guarda_del_bucle(
        '{ if (!reloj_horaFiable()) { bluetooth_reportarAlarma("HORA_ESP32", "CADUCADA", "A"); '
        'irAAmbar("x", "y"); return; } }', "irAAmbar")
    ok_b2, _ = _guarda_del_bucle(
        '{ if (!reloj_horaFiable()) { if (reloj_enHora()) { bluetooth_reportarAlarma('
        '"HORA_ESP32", "CADUCADA", "A"); } irAAmbar("x", "y"); return; } }', "irAAmbar")
    b.control_negativo(
        not ok_v and not ok_s and ok_b2,
        "la 3 acusa la guarda vieja (reloj_enHora() a secas) y una alarma que acusaria de "
        "CADUCADA a una hora borrada por REINICIAR_RELOJ, y aprueba la forma buena")
