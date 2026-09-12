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
# D-26 (2), 11/09 POR LA NOCHE: LA CADENCIA BAJA A ~2 MIN Y EL PLAZO SE REDERIVA.
# La derivacion vieja -"el plazo es el tiempo en que el HSI acumula la deriva de UNA
# cadencia"- era casi una TAUTOLOGIA: ese tiempo ES la cadencia, y los 20 s de holgura que
# parecia dar con 300 s salian del redondeo a segundos enteros (7,5 -> 8). Con 120 s el
# redondeo no da nada (3,0 exactos) y el plazo salia IGUAL a la cadencia. Desde hoy el plazo
# se deriva del RELEVO de fuente de D-26 (3), que es el caso peor que tiene que sobrevivir y
# que contiene al de una siembra perdida. Las lineas de este pack que EXIGIAN que no cupiera
# ninguno de los dos se revisaron una por una (CLAUDE.md 9) y estan invertidas abajo.
#
# D-28 (2), 12/09: EL PLAZO PASA A DERIVARSE DE DOS SIEMBRAS PERDIDAS, Y ESTE PACK ES QUIEN
# LE PUSO LA MEDIDA DELANTE AL RESPONSABLE. El reportar() de abajo decia, con la derivacion
# del relevo, que tolerar DOS siembras perdidas seguidas CABRIA bajo el aguante del cruce y
# que no hacerlo era una decision -cuanto margen de los dos DS3231 se cede-, no una
# derivacion. El responsable la tomo: se compran, y el margen baja de 13 s a 7 s. Lo que aqui
# cambia es el SUJETO de la minimalidad -el plazo sigue siendo el MENOR que cubre su caso
# peor, y el caso peor ya no es el relevo (271 s) sino las tres cadencias (369 s)- y la linea
# que exigia que dos perdidas NO cupieran, que se INVIERTE (CLAUDE.md 9). Su control esta
# abajo: con el plazo derivado del relevo, la inversion cae.
#
# QUE MIDE ESTE PACK, Y QUE NO:
#   - LA DESIGUALDAD (N-71). El plazo no se escoge: HORA_CADUCA_MS se escribe en reloj.h como
#     EXPRESION de la cadencia, del HSI y del silencio de SFTY-6, y aqui se EVALUA esa
#     expresion -con aritmetica de 32 bits sin signo, la del Cortex-M3- y se contrasta contra
#     el aguante del cruce, barrido con el modelo de costura por el MISMO codigo que esp32_13
#     (se importa, no se copia). Un comentario no falla cuando alguien cambia un numero;
#     esto si.
#   - Y QUE LA CADENA SIGA SIENDO UNA DERIVACION Y NO UN NUMERO ESCRITO A MANO. Un
#     HORA_RELEVO_MS = 271000UL pasaria todas las desigualdades de abajo y habria cortado el
#     lazo con la cadencia: la proxima vez que alguien la toque, el plazo se quedaria quieto.
#     Por eso se exige que cada eslabon NOMBRE a los anteriores (CLAUDE.md 1: un rojo no se
#     apaga escribiendo lo que el instrumento quiere leer).
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
# usar las anteriores (y SFTY6_SILENCIO_MS, que viene del protocolo.h de su punta).
SIMBOLOS = ("HORA_ESP32_CADENCIA_MS", "HSI_PPM_PEOR", "HORA_RELEVO_MS",
            "HORA_DOS_PERDIDAS_MS", "HORA_DERIVA_S", "HORA_CADUCA_MS")
# Que tiene que NOMBRAR cada eslabon para seguir siendo una derivacion y no un numero
# escrito a mano. Vale el nombre directo o el de un eslabon que ya lo arrastre.
#
# HORA_RELEVO_MS SIGUE EXIGIENDOSE aunque desde D-28 (2) el plazo ya no cuelgue de el: es el
# unico eslabon que nombra a SFTY6_SILENCIO_MS, y su static_assert es lo que vigila ese
# termino. Un suelo que deja de mandar no deja de medir (es el molde del suelo de "una
# siembra normal", que lleva sin mandar desde el 11/09).
NOMBRA = {"HORA_RELEVO_MS": ("HORA_ESP32_CADENCIA_MS", "SFTY6_SILENCIO_MS"),
          "HORA_DOS_PERDIDAS_MS": ("HORA_ESP32_CADENCIA_MS", "HSI_PPM_PEOR"),
          "HORA_DERIVA_S": ("HORA_DOS_PERDIDAS_MS",),
          "HORA_CADUCA_MS": ("HORA_DERIVA_S",)}
# D-21 (1): SFTY6_SILENCIO_MS entra en la derivacion del plazo, asi que reloj.h tiene que
# INCLUIR el protocolo.h de su punta. Sin el include, el numero seria una copia.
RE_SFTY6 = r"#define\s+SFTY6_SILENCIO_MS\s+(\d+)UL"
PROTOCOLO = {p: (p, "include", "protocolo.h") for p in PUNTAS}
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
    # Una expresion del C++ puede venir partida en varias lineas (HORA_RELEVO_MS lo esta):
    # se colapsan los blancos ANTES de parsear, o Python la lee como un bloque indentado y
    # el pack aborta por una forma que es correcta.
    limpia = re.sub(r"\b(\d+)(?:UL|LU|U|L)\b", r"\1", " ".join(expr.split()))
    if not re.fullmatch(r"[\w\s+\-*/()]+", limpia):
        raise _NoSabe("caracteres fuera de lo que este evaluador sabe: %r" % expr)
    arbol = ast.parse(limpia.replace("/", "//"), mode="eval")
    desbordes = []
    return _eval_nodo(arbol.body, valores, desbordes), desbordes


def _constantes(fw, punta):
    """({simbolo: valor}, {simbolo: desbordes}, {simbolo: expresion}) de reloj.h de la punta.

    SFTY6_SILENCIO_MS se SIEMBRA desde el protocolo.h de la MISMA punta -no se escribe aqui-
    porque el plazo lo usa y reloj.h lo incluye. Es la unica semilla de fuera del fichero."""
    t = fw.codigo(*RELOJ_H[punta])
    silencio = fw.constante(PROTOCOLO[punta], RE_SFTY6,
                            "SFTY6_SILENCIO_MS del %s" % punta)
    valores, desb, expr = {"SFTY6_SILENCIO_MS": silencio}, {}, {}
    for s in SIMBOLOS:
        m = re.search(r"static\s+const\s+unsigned\s+long\s+%s\s*=\s*([^;]+);" % s, t)
        if m is None:
            raise fw.Abortado("%s/include/reloj.h no define %s: sin el no hay plazo que medir"
                              % (punta, s))
        expr[s] = " ".join(m.group(1).split())
        try:
            valores[s], desb[s] = evaluar_c(m.group(1), valores)
        except _NoSabe as e:
            raise fw.Abortado("%s/include/reloj.h: %s = %r no es una forma que este pack sepa "
                              "evaluar (%s)" % (punta, s, m.group(1).strip(), e))
    valores.pop("SFTY6_SILENCIO_MS")
    return valores, desb, expr


def _cadena_derivada(expr, incluye_protocolo):
    """(ok, motivo): cada eslabon del plazo NOMBRA a los suyos, y reloj.h incluye protocolo.h.

    EL BORDE, ESCRITO (CLAUDE.md 7): mira los NOMBRES de la expresion, no su valor. Un
    eslabon escrito como literal -aunque el numero sea el correcto HOY- corta el lazo con la
    cadencia y el plazo se queda quieto la proxima vez que alguien la toque."""
    if not incluye_protocolo:
        return False, 'reloj.h no tiene #include "protocolo.h": SFTY6_SILENCIO_MS seria copia'
    for s, exigidos in NOMBRA.items():
        for n in exigidos:
            if not re.search(r"\b%s\b" % n, expr.get(s, "")):
                return False, ("%s = %r no nombra a %s: es un numero escrito a mano, no una "
                               "derivacion" % (s, expr.get(s, ""), n))
    return True, "los tres eslabones nombran a los suyos y reloj.h incluye protocolo.h"


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


def _perdida_ms(cad_ms, ppm):
    """UNA SIEMBRA PERDIDA: la siguiente hora buena llega en DOS cadencias, medidas por el
    millis() del STM32 con el HSI en su extremo rapido. Pasa por _suelo_ms a proposito -es
    la misma inflacion- para que el control negativo ejercite este mismo camino."""
    return _suelo_ms(2 * cad_ms, ppm)


def _dos_perdidas_ms(cad_ms, ppm):
    """DOS SIEMBRAS PERDIDAS SEGUIDAS: el caso peor desde D-28 (2).

    TRES cadencias y no dos, y es lo que se cuenta mal: la hora buena de partida es la de la
    cadencia 0, se pierden la 1 y la 2, y la que trae hora es la 3. Con dos cadencias solo
    cabe UNA perdida -eso es _perdida_ms-. No hay silencio que sumar: aqui la fuente no
    cambia, es el mismo ESP32 el que vuelve a acertar. Pasa por _suelo_ms por el mismo motivo
    que _perdida_ms: para que el control negativo ejercite este camino y no otro."""
    return _suelo_ms(3 * cad_ms, ppm)


def _relevo_ms(cad_ms, ppm, silencio_ms):
    """EL RELEVO DE FUENTE DE D-26 (3) en el Esclavo, en su propio millis().

    Dos cadencias -la ultima propagacion por radio, que sale en CADA siembra del ESP32 del
    Maestro, y la primera siembra del ESP32 de esta punta- mas el SFTY6_SILENCIO_MS que
    tarda en declarar la radio muda. Las cadencias las cuentan cuarzos y por eso se inflan
    con el HSI; el silencio NO, porque ya lo mide ese mismo millis().

    CONTIENE al de una siembra perdida: es el mismo caso mas la espera de silencio, asi que
    el suelo del plazo es este y no la suma de los dos."""
    return _perdida_ms(cad_ms, ppm) + silencio_ms


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
    cte, desb, expr = {}, {}, {}
    for p in PUNTAS:
        cte[p], desb[p], expr[p] = _constantes(fw, p)
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

    # Y QUE SIGA SIENDO UNA DERIVACION. Sin esto, las desigualdades de la 2 las pasaria igual
    # de bien un plazo escrito a mano que diera la casualidad de cumplirlas hoy.
    cadena = {}
    for p in PUNTAS:
        incl = re.search(r'#include\s+"protocolo\.h"', fw.codigo(*RELOJ_H[p])) is not None
        cadena[p] = _cadena_derivada(expr[p], incl)
    b.verificar(
        all(ok for ok, _ in cadena.values()),
        "el plazo es una CADENA DERIVADA en las dos puntas, no un numero escrito a mano: %s"
        % {p: cadena[p][1] for p in PUNTAS},
        "la cadena del plazo esta cortada (%s). Un eslabon escrito como literal cumple las "
        "desigualdades de hoy y deja de seguir a la cadencia: el dia que D-26 (2) vuelva a "
        "cambiarla, el plazo se queda quieto y nadie lo nota"
        % {p: cadena[p][1] for p in PUNTAS})

    c = cte["Maestro"]
    P_ms = c["HORA_CADUCA_MS"]
    cad_ms = c["HORA_ESP32_CADENCIA_MS"]
    ppm = c["HSI_PPM_PEOR"]
    silencio = fw.constante(PROTOCOLO_E, RE_SFTY6, "SFTY6_SILENCIO_MS del Esclavo")

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

    # ~~Y NO SE COME EL PRESUPUESTO DE LOS DS3231: la caducidad no puede conceder mas deriva
    # que UNA cadencia~~ -> REVISADA UNA POR UNA el 11/09 por la noche (CLAUDE.md 9). Afirmaba
    # DOS cosas:
    #   (i)  "el plazo no concede mas deriva de la NECESARIA" -> SE MUDA aqui abajo, que es
    #        donde sigue valiendo: el plazo tiene que ser el MENOR que cubre el relevo.
    #   (ii) "y lo necesario es UNA cadencia" -> SE BORRA: lo tumba D-26 (2). El plazo tiene
    #        que cubrir el relevo, que son dos cadencias mas el silencio, asi que concede mas
    #        deriva que una cadencia POR DISENO. Lo que no puede es concederla de mas: eso
    #        recorta el margen de los dos DS3231 sin que nadie lo decida, y por eso el suelo
    #        se cuantiza y se comprueba la minimalidad.
    #
    # LA MINIMALIDAD, Y SU SUJETO CAMBIO CON D-28 (2). HORA_DERIVA_S es la deriva -en segundos
    # enteros- que el plazo concede. Con un segundo MENOS el plazo ya no cubriria EL CASO PEOR,
    # que desde el 12/09 son las dos siembras perdidas y ya no el relevo: eso es lo que dice
    # que el numero salio de la derivacion y no de la mano de alguien que queria holgura.
    # EL BORDE, ESCRITO (CLAUDE.md 7): el caso peor es el MAXIMO de los que el plazo cubre, no
    # el relevo por costumbre. Se calcula asi -y no citando (c)- para que el dia que el relevo
    # vuelva a ser el mayor -un SFTY6_SILENCIO_MS mas largo, una cadencia mas corta- la
    # minimalidad se mida contra el que mande ese dia y no contra el que mandaba al escribirla.
    relevo_ms = _relevo_ms(cad_ms, ppm, silencio)
    dos_perdidas_ms = _dos_perdidas_ms(cad_ms, ppm)
    peor_ms = max(relevo_ms, dos_perdidas_ms)
    quien_peor = "las dos siembras perdidas" if dos_perdidas_ms >= relevo_ms else "el relevo"
    quantum_ms = 1000000 // ppm * 1000
    P_menor = (deriva_P - 1) * quantum_ms
    b.verificar(
        P_menor <= peor_ms < P_ms,
        "el plazo es el MENOR que cubre el caso peor -hoy %s, %d ms-: con %d s de deriva "
        "concedida son %d ms, y con uno menos serian %d ms, que NO lo cubren. El margen que le "
        "queda a los dos DS3231 es %d s" % (quien_peor, peor_ms, deriva_P, P_ms, P_menor,
                                            aguante - relativa_P),
        "el plazo (%d ms, %d s de deriva) no es el menor que cubre el caso peor -%s, %d ms-: "
        "con %d ms ya bastaba. Cada segundo de deriva concedido de mas recorta 2 s el margen de "
        "los dos DS3231, que hoy es %d s, y eso es una DECISION -cuanto margen se cede, y esa "
        "la tomo el responsable en D-28 (2)-, no una derivacion"
        % (P_ms, deriva_P, quien_peor, peor_ms, P_menor, aguante - relativa_P))

    # Y LO QUE EL C++ CALCULA NO PUEDE QUEDARSE CORTO CONTRA ESTE MODELO. La expresion de
    # reloj.h trunca en enteros a cada paso; si truncara por debajo del caso real, el plazo
    # saldria derivado de un caso peor mas corto que el de verdad. Se miran LOS DOS terminos,
    # no solo el que manda: el que no manda es un static_assert del C++, y si su expresion se
    # queda corta ese assert deja de proteger sin que nada lo diga.
    b.verificar(
        c["HORA_RELEVO_MS"] >= relevo_ms and c["HORA_DOS_PERDIDAS_MS"] >= dos_perdidas_ms,
        "los dos terminos del C++ no se quedan cortos contra este modelo: HORA_RELEVO_MS = %d "
        "ms >= %.0f y HORA_DOS_PERDIDAS_MS = %d ms >= %.0f. El truncado entero de reloj.h no "
        "acorta ninguno de los dos casos peores"
        % (c["HORA_RELEVO_MS"], relevo_ms, c["HORA_DOS_PERDIDAS_MS"], dos_perdidas_ms),
        "un termino del C++ se queda corto contra el modelo (HORA_RELEVO_MS %d contra %.0f, "
        "HORA_DOS_PERDIDAS_MS %d contra %.0f): el plazo se estaria derivando -o comprobando- "
        "contra un caso peor mas corto que el real"
        % (c["HORA_RELEVO_MS"], relevo_ms, c["HORA_DOS_PERDIDAS_MS"], dos_perdidas_ms))

    # =====================================================================
    b.titulo("2.bis LO QUE EL PLAZO SI TIENE QUE COMPRAR (invertidas por D-26 (2) y D-28 (2))")
    # =====================================================================
    # Las tres lineas de abajo estaban en el reportar() de este pack como residual que ningun
    # firmware podia aprobar -"lo que el plazo NO puede comprar"-. Con la cadencia en ~2 min
    # las dos primeras SI se pueden, y la tercera -dos siembras perdidas- la compro el
    # responsable en D-28 (2) con la medida de este mismo pack delante, asi que dejan de ser
    # nota y pasan a ser comprobacion (CLAUDE.md 9: se INVIERTEN para exigir lo nuevo). Sus
    # controles estan abajo: con la cadencia vieja caen las dos primeras, y con el plazo
    # derivado del relevo cae la tercera.
    perdida_ms = _perdida_ms(cad_ms, ppm)
    b.verificar(
        P_ms > perdida_ms,
        "UNA SIEMBRA PERDIDA CABE: la siguiente hora buena llega a los %.0f ms del millis() de "
        "la punta -dos cadencias de %d ms con el HSI a +%d ppm- y el plazo es %d. Un ESP32 que "
        "se reinicia o un byte comido ya no cuestan el Degradado hasta que vuelva una persona"
        % (perdida_ms, cad_ms, ppm, P_ms),
        "una siembra perdida (%.0f ms) pasa del plazo (%d ms): la punta se va a ambar y NO "
        "vuelve sola (D-21), asi que un byte comido cuesta el Degradado hasta que vaya alguien"
        % (perdida_ms, P_ms))

    b.verificar(
        P_ms > relevo_ms,
        "EL RELEVO DE D-26 (3) CABE: al callarse la radio, la hora del Esclavo puede tener "
        "hasta %.0f ms -dos cadencias de %d ms infladas por el HSI, mas SFTY6_SILENCIO_MS "
        "(%d)- antes de la primera siembra de su ESP32, y el plazo es %d. El Esclavo puede "
        "ENTRAR en Degradado durante el relevo y no se rinde si ya estaba dentro"
        % (relevo_ms, cad_ms, silencio, P_ms),
        "el relevo (%.0f ms) pasa del plazo (%d ms): al caer la radio el Esclavo no puede "
        "entrar en Degradado durante esos minutos, o se rinde si ya estaba dentro. No da "
        "verde-verde: da un cruce que no arranca" % (relevo_ms, P_ms))

    # LA INVERSION DE D-28 (2), y es la razon de ser de este cambio: lo que hasta el 12/09 era
    # el residual que ningun firmware podia aprobar pasa a ser lo que el firmware TIENE que
    # cumplir. El coste esta medido y aceptado, y sale abajo en el reportar().
    b.verificar(
        P_ms > dos_perdidas_ms,
        "DOS SIEMBRAS PERDIDAS SEGUIDAS CABEN (D-28 (2)): la hora buena es la de la TERCERA "
        "cadencia, que llega a los %.0f ms del millis() de la punta -3 x %d ms con el HSI a "
        "+%d ppm- y el plazo es %d. Un ESP32 que se reinicia dos veces, o dos bytes comidos, ya "
        "no declaran la hora caducada: esa punta se iba a ambar y NO volvia sola, y eso se paga "
        "con un viaje al poste" % (dos_perdidas_ms, cad_ms, ppm, P_ms),
        "dos siembras perdidas seguidas (%.0f ms) pasan del plazo (%d ms): la punta declara su "
        "hora caducada, se va a ambar intermitente y NO vuelve sola (D-21). Es lo que D-28 (2) "
        "vino a comprar" % (dos_perdidas_ms, P_ms))

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
    # TRES siembras perdidas seguidas: lo primero que sigue sin caber desde D-28 (2), y el
    # sitio por donde este plazo se rompe. Es el residual que ningun firmware puede aprobar
    # con este aguante. Se calcula por el MISMO camino que los otros casos -la inflacion del
    # HSI sobre N cadencias- para que no sea una cuenta aparte que envejezca sola.
    tres_perdidas_ms = _suelo_ms(4 * cad_ms, ppm)
    # El mayor plazo que todavia cabria bajo el aguante, y lo que costaria en margen.
    p_max = cad_ms
    while _relativa_s(p_max + 1000, ppm) < aguante:
        p_max += 1000
    margen_hoy = aguante - relativa_P
    margen_pmax = aguante - _relativa_s(p_max, ppm)
    dias = lambda m: m / (2 * DS3231_PPM * 86400 / 1e6)
    b.reportar(
        "D-28 (2): lo que se PAGO por las dos siembras perdidas, y lo que sigue SIN comprarse",
        ["LO QUE SE PAGO, DICHO EN CLARO, Y ES LO QUE EL RESPONSABLE ACEPTO EL 12/09. El plazo "
         "concede %d s de deriva por punta y no los %d de UNA cadencia, asi que el margen que "
         "queda para lo que difieran los dos DS3231 baja de %d s a %d s (a %d ppm por reloj, de "
         "~%.0f dias sin tocarlos a ~%.0f). No es un descuido: es el precio de que una punta con "
         "dos siembras perdidas seguidas no se rinda a un ambar del que no vuelve sola. Con la "
         "derivacion del relevo -la de ayer- el margen eran %d s y el plazo %d ms."
         % (deriva_P, deriva_cad, aguante - _relativa_s(cad_ms, ppm), margen_hoy, DS3231_PPM,
            dias(aguante - _relativa_s(cad_ms, ppm)), dias(margen_hoy),
            aguante - _relativa_s(_deriva_cadencia_s(int(relevo_ms), ppm) * quantum_ms, ppm),
            _deriva_cadencia_s(int(relevo_ms), ppm) * quantum_ms),
         "TRES SIEMBRAS PERDIDAS SEGUIDAS siguen mandando el Degradado a ambar, y el ambar no "
         "se levanta solo (D-21): la cuarta cadencia llega a los %.0f ms y el plazo es %d. "
         "LO QUE HAY QUE DECIR AQUI, Y ES UNA PREGUNTA PARA EL RESPONSABLE, NO UNA "
         "DERIVACION: tolerarlas CABRIA. Un plazo de %d ms deja %d s de separacion contra %d "
         "de aguante -el mayor que cabe es %d ms, con %d s de margen-, asi que lo que lo impide "
         "no es el cruce: es la regla de que el plazo sea el MENOR que cubre su caso peor. "
         "Comprarlas costaria bajar el margen de los dos DS3231 de %d s a %d s (~%.0f dias a "
         "~%.0f). Ese cambio es una DECISION -cuanto margen se cede-, y este pack no la toma: "
         "la ultima vez que la puso encima de la mesa se llamo D-28 (2)."
         % (tres_perdidas_ms, P_ms, int(tres_perdidas_ms),
            _relativa_s(tres_perdidas_ms, ppm), aguante, p_max, margen_pmax,
            margen_hoy, aguante - _relativa_s(tres_perdidas_ms, ppm),
            dias(margen_hoy), dias(aguante - _relativa_s(tres_perdidas_ms, ppm))),
         "Y ESTO SIGUE SIN MEDIR EL TIEMPO. Que el plazo caduque de verdad en su frontera, que "
         "el relevo quepa en el equipo y que la punta pase a ambar sin verde-verde lo EJERCE el "
         "bloque F del orquestador del Degradado sobre el reloj.cpp real (CLAUDE.md 6.3). Aqui "
         "solo se recalcula la aritmetica y se vigila que ese bloque siga ahi."])

    # =====================================================================
    b.titulo("CONTROLES NEGATIVOS")
    # =====================================================================
    # El techo sabe caer. OJO AL BORDE, Y ESTA MEDIDO (CLAUDE.md 7): con la cadencia en ~2 min
    # el techo ya NO lo rompe un plazo que tolere dos siembras perdidas -ese CABE bajo el
    # aguante, y es justo el que D-28 (2) escogio-, asi que hay que ejercitarlo con el primer
    # plazo que de verdad se sale, que es p_max mas un escalon. Escrito con el caso viejo, este
    # control salia en verde sin ejercitar nada: medido al escribirlo, y por eso se cambio.
    b.control_negativo(
        not (_relativa_s(p_max + 1000, ppm) < aguante),
        "el primer plazo por encima del mayor que cabe (%d ms) da %d s de separacion contra "
        "%d de aguante: el techo de la 2 sabe cazar un plazo demasiado largo"
        % (p_max + 1000, _relativa_s(p_max + 1000, ppm), aguante))
    # El suelo sabe caer: un plazo igual a la cadencia no deja llegar la siembra con HSI rapido.
    b.control_negativo(
        not (cad_ms > _suelo_ms(cad_ms, ppm)),
        "un plazo igual a la cadencia no pasa el suelo de la 2: con el HSI rapido la siembra "
        "llega %.0f ms despues de caducar" % (_suelo_ms(cad_ms, ppm) - cad_ms))

    # EL CONTROL DE LAS DOS PRIMERAS INVERSIONES DE LA 2.bis (CLAUDE.md 9: una inversion sin
    # escenario de control la aprobaria igual una guarda que no dejara pasar nada). Se rehace
    # la derivacion ENTERA con la cadencia VIEJA de D-26 -300 s- por el mismo camino que la de
    # arriba, y se exige que el relevo NO quepa: es la medida que motivo el cambio de la
    # cadencia, y si algun dia deja de caer es que este pack dejo de mirar.
    CAD_VIEJA_MS = 300000
    P_viejo = _deriva_cadencia_s(CAD_VIEJA_MS, ppm) * quantum_ms   # la derivacion vieja
    relevo_viejo = _relevo_ms(CAD_VIEJA_MS, ppm, silencio)
    perdida_vieja = _perdida_ms(CAD_VIEJA_MS, ppm)
    b.control_negativo(
        not (P_viejo > relevo_viejo) and not (P_viejo > perdida_vieja),
        "con la cadencia vieja (%d ms) y su plazo de una cadencia (%d ms) NI el relevo (%.0f "
        "ms) NI una siembra perdida (%.0f ms) cabian: las dos primeras inversiones de la 2.bis "
        "saben caer, y esa es la medida que bajo la cadencia a %d ms"
        % (CAD_VIEJA_MS, P_viejo, relevo_viejo, perdida_vieja, cad_ms))

    # EL CONTROL DE LA TERCERA, LA DE D-28 (2), Y ES EL QUE IMPORTA HOY: con la cadencia BUENA
    # y la derivacion de AYER -la del relevo, que es lo que este cambio retira- dos siembras
    # perdidas NO cabian. Es el escenario de control que le falta a toda inversion: sin el, la
    # linea "dos perdidas caben" la aprobaria igual un plazo cualquiera que fuese lo bastante
    # largo por otro motivo. La derivacion vieja se rehace aqui ENTERA -no se escribe 280000-
    # para que siga siendo la de ayer el dia que alguien toque la cadencia o el silencio.
    P_del_relevo = _deriva_cadencia_s(int(relevo_ms), ppm) * quantum_ms
    b.control_negativo(
        not (P_del_relevo > dos_perdidas_ms) and P_del_relevo > relevo_ms,
        "con la derivacion de AYER -la del relevo: %.0f ms de relevo -> %d ms de plazo- las dos "
        "siembras perdidas (%.0f ms) NO cabian, y el relevo si: la inversion de D-28 (2) sabe "
        "caer y el control no aprueba por un plazo que no cubre nada"
        % (relevo_ms, P_del_relevo, dos_perdidas_ms))

    # Y LA MINIMALIDAD SABE CAER: un plazo de un segundo de deriva MAS sigue cubriendo el caso
    # peor, o sea que el "<= caso peor" de la 2 lo rechaza por sobrado, no por corto.
    b.control_negativo(
        not (deriva_P * quantum_ms <= peor_ms),
        "un plazo con un segundo de deriva de mas (%d ms) tendria %d ms de sobra sobre el caso "
        "peor (%s, %d ms): la minimalidad de la 2 lo caza en vez de darlo por bueno"
        % ((deriva_P + 1) * quantum_ms, int(deriva_P * quantum_ms - peor_ms), quien_peor,
           peor_ms))

    # Y LA CADENA SABE CAER: un eslabon escrito como literal no pasa, y el bueno si. Se ejerce
    # con el eslabon del que HOY cuelga el plazo -HORA_DOS_PERDIDAS_MS- y con el que ya no
    # manda pero se sigue exigiendo -HORA_RELEVO_MS-, uno en cada caso sintetico.
    ok_lit, _ = _cadena_derivada({"HORA_RELEVO_MS": "271000UL",
                                  "HORA_DOS_PERDIDAS_MS": "3UL * HORA_ESP32_CADENCIA_MS * "
                                                          "HSI_PPM_PEOR",
                                  "HORA_DERIVA_S": "(HORA_DOS_PERDIDAS_MS / 1000UL) ",
                                  "HORA_CADUCA_MS": "HORA_DERIVA_S * 1000UL"}, True)
    ok_lit2, _ = _cadena_derivada({"HORA_RELEVO_MS": "2UL * HORA_ESP32_CADENCIA_MS + "
                                                     "SFTY6_SILENCIO_MS",
                                   "HORA_DOS_PERDIDAS_MS": "369000UL",
                                   "HORA_DERIVA_S": "(HORA_DOS_PERDIDAS_MS / 1000UL) ",
                                   "HORA_CADUCA_MS": "HORA_DERIVA_S * 1000UL"}, True)
    ok_sin, _ = _cadena_derivada(expr["Maestro"], False)
    ok_bien, _ = _cadena_derivada(expr["Maestro"], True)
    b.control_negativo(
        not ok_lit and not ok_lit2 and not ok_sin and ok_bien,
        "la cadena de la 1 acusa un HORA_RELEVO_MS escrito como literal, un "
        "HORA_DOS_PERDIDAS_MS escrito como literal -el eslabon del que hoy cuelga el plazo- y "
        "un reloj.h sin el #include de protocolo.h, y aprueba la cadena real: mide los NOMBRES, "
        "no el valor")
    # El evaluador lee la expresion REAL -la del relevo, que es la forma que usa reloj.h desde
    # D-26 (2)- y sabe ver un desborde de 32 bits.
    v, d = evaluar_c("2UL * 120000UL + 2UL * 120000UL / 1000UL * 25000UL / 1000UL + 25000UL",
                     {})
    v2, d2 = evaluar_c("2UL * 120000UL * 25000UL / 1000000UL", {})
    b.control_negativo(
        v == 271000 and not d and d2 and v2 != 6000,
        "el evaluador da 271000 con la forma del relevo que usa reloj.h y DETECTA el desborde "
        "de 32 bits de '2UL * 120000UL * 25000UL' (en la tarjeta daria %d, no 6000)" % v2)
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
