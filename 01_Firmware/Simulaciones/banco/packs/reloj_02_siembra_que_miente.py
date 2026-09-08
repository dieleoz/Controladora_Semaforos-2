# ===== banco/packs/reloj_02_siembra_que_miente.py =====
#
# LA SIEMBRA DE LA HORA NO PUEDE DECIR QUE SI POR UNA HORA QUE DESCARTO.
#
# LA PROPIEDAD, EN UNA LINEA: en la cadena de siembra de la hora, un retorno o un
# acuse de exito tiene que DEPENDER de lo que la validacion decidio.
#
# EL DEFECTO QUE ESTE PACK VINE A CAZAR, COPIADO LITERAL DE `git show HEAD:` PORQUE ES
# EL MUTANTE CON EL QUE SE CALIBRA (y esta abajo, en el control negativo):
#
#   bool reloj_sembrarDesdeIso(const char* str) {          // las DOS puntas
#     if (str == nullptr) return false;
#     int anio = 0, mes = 0, dia = 0, h = 0, m = 0, s = 0;
#     if (sscanf(str, "%d-%d-%d,%d:%d:%d", &anio, &mes, &dia, &h, &m, &s) == 6) {
#       reloj_ajustar((uint8_t)h, (uint8_t)m, (uint8_t)s, (uint8_t)dia);
#       return true;                             <-- no depende de NADA
#     }
#     return false;
#   }
#
#   void reloj_ajustar(...) {
#     if (hora > 23 || minuto > 59 || segundo > 59) return;   // no aceptamos basura
#
# El ajustador era `void` y descartaba EN SILENCIO. Ese `return true` no decia "la hora
# quedo puesta", decia "la cadena tenia seis numeros". Son dos cosas distintas y el
# firmware las escribia iguales. Es el patron de CLAUDE.md 2 -un acuse que no depende
# de lo que la llamada devolvio- con un tramo mas de tuberia por medio, y ese tramo es
# lo que lo hacia invisible: el defecto no estaba en el despachador, estaba en la
# funcion intermedia que el despachador si mira.
#
# POR QUE NO LO VEIA NINGUNO DE LOS PACKS QUE YA HABIA. Se comprobo antes de escribir
# este, porque un pack que vuelve a certificar lo ya certificado sustituye trabajo:
#
#   app_03_sin_ok_mudo      mide LAS RAMAS DEL DESPACHADOR. La del Maestro escribe
#                           `if (reloj_sembrarDesdeIso(accion + 8))`, o sea CONSUME el
#                           retorno: para app_03 esa rama esta bien hecha, y lo esta.
#                           El defecto vivia DENTRO de la funcion intermedia, que es
#                           justo donde app_03 ya no mira.
#   esp32_03_ack_que_mira   mide el DESPACHADOR DEL PUENTE contra ResultadoReloj del
#                           DS3231. Otro micro, otro reloj, otra funcion.
#   maestro_04_sync_horaria mide el TRIO que sale por radio y sus ventanas. Da por
#                           puesta la hora; no pregunta si la puesta ocurrio.
#
# Y NO ERA COSMETICA, PORQUE ESE BOOLEANO GOBIERNA UNA PROPAGACION. En el Maestro es la
# puerta de coordinador_sincronizarHora(), que es como D-20 empuja la hora al Esclavo.
# Con la puerta clavada en `true`, una hora que ESTA punta descarto se propagaba igual
# a la otra. D-20 dice "el Maestro manda la hora y el Esclavo hace caso SIEMPRE", asi
# que lo que salga de aqui no lo filtra nadie mas abajo.
#
# ESTE PACK NO SE ATA A NINGUN NOMBRE, Y ESO NO ES ELEGANCIA: ES LO QUE LE PASO EL
# 07/09. Se escribio contra `reloj_ajustar()` y, mientras se escribia, el arreglo N-160
# mudo la validacion a `reloj_ajustarConAcuse()`. El pack ABORTO -que es lo correcto,
# gritar en vez de aprobar-, pero un instrumento que se cae con un renombrado mide un
# nombre, no una propiedad. Ahora el validador se DERIVA: es la funcion a la que el
# sembrador le pasa las variables que sscanf acaba de leer. Si manana se llama de otra
# forma, este pack la sigue.
#
# LO QUE ESTE PACK NO PUEDE HACER, ESCRITO PARA QUE NADIE LO LEA COMO PERMISO.
#
# ES UN PACK DE TEXTO: Python parseando el .cpp. NO ejecuta el firmware, no enciende un
# STM32 y no ha visto un solo `return` de verdad. El barrido de la comprobacion 2 corre
# sobre un MODELO de las dos funciones construido leyendo sus guardas del fuente en
# cada corrida -sin un solo limite escrito a mano-, y un modelo no es la funcion.
#
# Y EL PUNTO CIEGO ESTA MEDIDO, NO SUPUESTO [MEDIDO 07/09]:
#
#   grep -rn "reloj.cpp" 01_Firmware/Validacion_*/ Simulaciones/puente_esp32/
#     -> "reloj.cpp: NO se compila. Incluye <STM32RTC.h> y <stm32f1xx_hal.h>, que no
#        tienen sustituto en el repositorio"   (arnes_puente.cpp, y lo mismo dicen
#        adaptador_esclavo.cpp, adaptador_maestro_deg.cpp y arnes_lcd.cpp)
#
# NINGUN arnes enlaza reloj.cpp de las puntas, y Validacion_Automatico STUBEA las
# funciones de reloj. Nadie EJECUTA reloj_sembrarDesdeIso() en ningun sitio, asi que
# un defecto del TIEMPO -o del compilador con los casts- este pack no lo ve. El
# instrumento que taparia ese hueco es un arnes que compile Maestro/src/reloj.cpp de
# verdad, con un sustituto de STM32RTC.h que hoy no existe: eso es firmware que hay
# que escribir, no una fila que anadir aqui.
#
# QUIEN TIRA EL RETORNO SE MIDE, NO SE AFIRMA, Y ESTA ES LA RAZON [07/09].
#
# Durante esta misma sesion el Esclavo llamaba `reloj_sembrarDesdeIso(accion + 8);` SIN
# `if` -tiraba el booleano- y este pack llevaba escrita esa frase en la cabecera. Horas
# despues esa rama gano su `if/else` con un $EVENT por cada lado, y la frase habria
# quedado FALSA dentro de un comentario que nadie recompila: es el defecto que ya se
# corrigio una vez en reloj_01, una afirmacion sobre el codigo envejeciendo con
# autoridad de dato. Por eso lo que hay abajo no es una frase, es un reportar() que
# vuelve a MEDIR quien consume el retorno en cada corrida y se calla cuando no hay nada
# que decir -hoy se calla-.
#
# Y NO SE COBRA COMO COMPROBACION, que es lo que hay que escribir para que nadie lo lea
# como blandura: la propiedad de este pack es que un acuse de exito DEPENDA de la
# validacion. Una rama que tira el retorno y NO acusa nada -no propaga, no contesta
# $ACK, no toca luces- no puede mentir, porque no afirma. Lo que le falta es otra
# propiedad -"alguien se entera de que la siembra fallo"- que vive en bluetooth.cpp. El
# dia que una de esas ramas gane un $ACK, la comprobacion 4 la exige gobernada sin
# tocar una linea de aqui.
#
# UNA CAUSA REFUTADA, QUE SE DEJA ESCRITA PARA QUE NADIE LA VUELVA A PROPONER [07/09].
# Se sospecho que el otro extremo de la cadena tenia el mismo defecto: Esclavo/src/
# main.cpp, rama CMD_HORA_S, llama al envoltorio `void` -que sigue descartando en
# silencio- y despues hace degradado_registrarSync() y programarRespuesta(CMD_ACK_HORA)
# pase lo que pase. SE MIDIO Y ES FALSO: las tres cifras llegan ya validadas en su
# propia rama -CMD_HORA_D exige `>= 1 && <= 31`, CMD_HORA_H `<= 23`, CMD_HORA_M `<= 59`
# y el segundo `<= 59` en la propia CMD_HORA_S-, asi que por ese camino el ajustador no
# puede rechazar y el ACK no miente. No se cuenta como comprobacion porque no hay
# defecto que contar.
#
# SIN ETIQUETA SFTY, Y ES DELIBERADO. La tentacion era SFTY-18, que vive en
# Maestro/src/reloj.cpp y cuya regla es "la regla de seguridad no es tener reloj, es
# saber cuando NO se tiene". Pero SFTY-18 se EJERCE midiendo el ano marcador y
# reloj_enHora(), y este pack no mira ninguno de los dos: mira si el retorno de la
# siembra depende de la validacion. OPTIMIZACIONES.md ya dice que esa regla "tal como
# esta definida no la ejerce ningun pack", y poner aqui un ✅ que no la ejerce dejaria
# la fila cubierta por una prueba que mide otra cosa: peor que la fila vacia, porque
# la vacia no miente.

import re

NOMBRE = "reloj_02_siembra_que_miente"
DESCRIPCION = ("el retorno y el acuse de la siembra de la hora dependen de lo que la "
               "validacion decidio")

# Las rutas por TUPLA, que es como este banco direcciona el fuente: mover o renombrar
# un fichero tiene que romper el instrumento, no dejarlo midiendo otra cosa.
RELOJ_C = {"Maestro": ("Maestro", "src", "reloj.cpp"),
           "Esclavo": ("Esclavo", "src", "reloj.cpp")}
RELOJ_H = {"Maestro": ("Maestro", "include", "reloj.h"),
           "Esclavo": ("Esclavo", "include", "reloj.h")}
BLUETOOTH = {"Maestro": ("Maestro", "src", "bluetooth.cpp"),
             "Esclavo": ("Esclavo", "src", "bluetooth.cpp")}
BLUETOOTH_H = ("Maestro", "include", "bluetooth.h")

PUNTAS = ("Maestro", "Esclavo")

# EL UNICO NOMBRE ATADO, y lo esta porque es el sujeto: la funcion que la app pone en
# marcha. El VALIDADOR al que llama NO se nombra aqui, se deriva (ver Cadena).
SEMBRADOR = "reloj_sembrarDesdeIso"

# La rama del despachador por la que entra la orden, y el reportador de eventos que esa
# rama usa. Son sujetos ENUMERADOS: una regla que enumera sujetos tiene que comprobar
# que cada uno existe (N-96), y por eso los dos abortan si no aparecen.
RAMA_BT = "SET_RTC:"
REPORTADOR = "bluetooth_reportarEvento"

# EL BORDE DE LO QUE CUENTA COMO "ACUSE DE EXITO", ESCRITO AQUI PORQUE ES EL BORDE.
#
# Los censos que fallaron en este repositorio lo hicieron todos en la frontera que
# decidieron no mirar, y ninguno la llevaba escrita. Esta es la de este pack:
#
#   SI cuenta como afirmacion de exito     "$ACK", "RESULT:OK", "PUESTA"
#   NO cuenta                              "SET_RTC_LO_ACUSA_EL_PUENTE"
#
# El literal que las dos puntas emiten hoy dice QUIEN contesta, no que haya salido
# bien: es informacion de enrutado para el diario de la app (D-15), y cobrarle una
# guarda empujaria a quitarlo o a inventarse un rechazo. El borde queda vigilado por si
# se mueve: el dia que ese literal diga OK o PUESTA, la comprobacion 4 lo exige
# gobernado por el retorno de la siembra.
PALABRAS_DE_EXITO = ("$ACK", "RESULT:OK", "PUESTA")

# Los NEGATIVOS del lenguaje. Es lo unico escrito a mano de la parte que mide, y es
# aritmetica de C++, no del firmware: un `return` de estos es un rechazo lo escriba
# quien lo escriba, y con eso este pack no necesita saber si el validador es `void`
# -como antes de N-160- o `bool` -como ahora-.
_FALSOS = ("", "false", "0", "nullptr", "NULL")

# Palabras que NO son una llamada aunque lo parezcan al leer `nombre(`.
_NO_SON_LLAMADAS = ("if", "while", "for", "switch", "return", "sizeof", "else")


class _NoMide(Exception):
    """No se pudo MEDIR. Se traduce a fw.Abortado en correr().

    Se usa una excepcion propia y no la del banco para que las cadenas SINTETICAS de
    los controles negativos pasen por el mismo constructor que las de verdad: un
    control negativo que ejercita otro camino no controla nada."""


# ---------------------------------------------------------------------------------
# LECTURA DE BLOQUES
#
# Bloque traido LITERAL de app_03_sin_ok_mudo y de reloj_01_consulta_por_bluetooth,
# solo reindentado: es la logica ya probada de leer por LLAVES en vez de por lineas.
# Reescribirla para renombrar las llamadas es como se cuelan los errores en una
# migracion que se supone que no cambia comportamiento.
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


def _abre_del_bloque(texto, idx):
    """Indice del '{' que abre el bloque mas interno que envuelve a idx. -1 si no hay.

    Por llaves y no por proximidad de lineas: la leccion de N-89. Una comprobacion que
    mira "esta en el fichero" o "esta dos lineas mas abajo" aprueba igual una llamada
    colgada del `else` de al lado, que es exactamente el caso peligroso."""
    prof, j = 0, idx - 1
    while j >= 0:
        c = texto[j]
        if c == "}":
            prof += 1
        elif c == "{":
            if prof == 0:
                return j
            prof -= 1
        j -= 1
    return -1


def _cierra_paren(texto, i):
    """Indice del ')' que cierra el '(' de texto[i]. -1 si no cuadra."""
    prof = 0
    for j in range(i, len(texto)):
        if texto[j] == "(":
            prof += 1
        elif texto[j] == ")":
            prof -= 1
            if prof == 0:
                return j
    return -1


def _abre_paren(texto, i):
    """Indice del '(' que abre el ')' de texto[i]. -1 si no cuadra."""
    prof, j = 0, i
    while j >= 0:
        if texto[j] == ")":
            prof += 1
        elif texto[j] == "(":
            prof -= 1
            if prof == 0:
                return j
        j -= 1
    return -1


def _argumentos(texto, i):
    """Los argumentos, sin partir, de la llamada cuyo '(' esta en texto[i]."""
    cp = _cierra_paren(texto, i)
    if cp < 0:
        return None
    args, prof, act = [], 0, ""
    for c in texto[i + 1:cp]:
        if c in "([":
            prof += 1
        elif c in ")]":
            prof -= 1
        if c == "," and prof == 0:
            args.append(act)
            act = ""
            continue
        act += c
    args.append(act)
    return args


def _llama_a(texto, nombre):
    """True si `texto` contiene una llamada a `nombre`, EXACTO.

    🔴 NO ES UN `in`, Y ESA ES LA MITAD DEL VALOR DE ESTA FUNCION. En esta cadena
    `reloj_ajustar` es PREFIJO de `reloj_ajustarConAcuse`, asi que un substring lee
    `return reloj_ajustarConAcuse(...)` como si llamara a `reloj_ajustar()` y da por
    propagado un veredicto que nadie propaga -o al reves, segun cual de los dos se
    haya derivado-. Con dos funciones que se llaman casi igual y hacen cosas
    opuestas, el buscador ciego es el pack. Lo ejerce el quinto control negativo.

    EL BORDE: un limite de palabra por delante -para que ConAcuse no case con ajustar por el final- y
    `\s*\(` por detras -para que ajustar no case con ConAcuse por el principio-. Los
    dos hacen falta: cada uno tapa un sentido del solapamiento."""
    return re.search(r"\b%s\s*\(" % re.escape(nombre), texto) is not None


def _gobernantes(texto, pos):
    """Las condiciones de TODOS los `if` cuyo bloque envuelve a la posicion pos.

    Un `else` no aporta condicion, y eso es lo correcto: una llamada en el else de
    `if (sembrar())` corre justo cuando la siembra dijo que NO, y eso no es estar
    gobernado por ella."""
    conds, i = [], pos
    while True:
        a = _abre_del_bloque(texto, i)
        if a < 0:
            return conds
        antes = texto[:a].rstrip()
        if antes.endswith(")"):
            ap = _abre_paren(antes, len(antes) - 1)
            if ap >= 0 and re.search(r"\b(if|while|for)\s*$", antes[:ap]):
                conds.append(antes[ap + 1:len(antes) - 1])
        i = a


def _bajo_decision(texto, pos, funcion):
    """True si pos esta dentro de un `if`/`else` cuya CONDICION llama a `funcion`.

    ES OTRA PREGUNTA QUE LA DE `_gobernantes`, Y HACEN FALTA LAS DOS.

    Alli el `else` NO cuenta, y es correcto: propagar la hora en el `else` de
    `if (sembrar())` seria empujar al otro poste una hora que esta punta descarto. Aqui
    el `else` SI cuenta, y tambien es correcto: lo que se mide no es que la linea corra
    solo en exito, sino que DEPENDA del veredicto -y una linea de diario en el `else` es
    exactamente la que dice "se rechazo"-. Usar `_gobernantes` para esto acusaria al
    arreglo bueno, que es como se desactiva un instrumento (N-160, 08/09: el primer
    intento de esta comprobacion hizo eso y lo dijo su control negativo)."""
    i = pos
    while True:
        a = _abre_del_bloque(texto, i)
        if a < 0:
            return False
        antes = texto[:a].rstrip()
        if antes.endswith(")"):
            ap = _abre_paren(antes, len(antes) - 1)
            if ap >= 0 and re.search(r"\b(if|while|for)\s*$", antes[:ap]) \
                    and _llama_a(antes[ap + 1:len(antes) - 1], funcion):
                return True
        elif re.search(r"\belse\s*$", antes):
            # Un `else` no lleva condicion propia: la suya es la del `if` que lo precede,
            # y para leerla hay que saltar hacia atras el bloque entero de ese `if`.
            cuerpo = antes[:antes.rfind("else")].rstrip()
            if cuerpo.endswith("}"):
                ai = _abre_del_bloque(cuerpo, len(cuerpo) - 1)
                if ai >= 0:
                    cab = cuerpo[:ai].rstrip()
                    if cab.endswith(")"):
                        ap = _abre_paren(cab, len(cab) - 1)
                        if ap >= 0 and re.search(r"\bif\s*$", cab[:ap]) \
                                and _llama_a(cab[ap + 1:len(cab) - 1], funcion):
                            return True
        i = a


def _consumido(bloque, pos):
    """True si el resultado de la llamada que empieza en `pos` va a alguna parte.

    Bloque traido LITERAL de app_03_sin_ok_mudo. Se mira el ultimo caracter util ANTES
    del nombre: si es ';', '{' o '}' la llamada es una sentencia suelta y lo que
    devuelva se pierde; si es '(', '=', ',' o un operador, alguien lo esta usando. El
    '!' se salta a proposito, que es como esta escrito el caso bien hecho."""
    i = pos - 1
    while i >= 0 and (bloque[i].isspace() or bloque[i] == "!"):
        i -= 1
    if i < 0:
        return False
    return bloque[i] not in ";{}"


# ---------------------------------------------------------------------------------
# LECTURA DE LAS FUNCIONES Y DE SUS GUARDAS
# ---------------------------------------------------------------------------------

def _definicion(codigo, nombre):
    """(tipo, [(tipo, param)], cuerpo) de la DEFINICION de `nombre`. None si no esta.

    Se recorren todas las apariciones y se toma la que va seguida de '{': una
    declaracion adelantada no tiene cuerpo que medir. El tipo se captura entero y
    perezoso para que "unsigned long f()" no se lea como si la funcion se llamara
    "long"; hace falta leerlo -y no darlo por `void`- porque cambiarlo es justo el
    arreglo natural de este defecto."""
    for m in re.finditer(r"(?:^|[;}\s])([A-Za-z_][\w \t\*&]*?)[ \t\*&]+%s\s*\("
                         % re.escape(nombre), codigo):
        ap = codigo.find("(", m.end() - 1)
        cp = _cierra_paren(codigo, ap)
        if cp < 0:
            continue
        resto = codigo[cp + 1:]
        if resto.lstrip()[:1] != "{":
            continue                      # declaracion, no definicion
        llave = codigo.find("{", cp)
        cuerpo = _bloque(codigo, llave)
        if cuerpo is None:
            continue
        params = []
        for p in codigo[ap + 1:cp].split(","):
            ident = re.findall(r"[A-Za-z_]\w*", p.split("=")[0])
            if len(ident) >= 2:
                params.append((" ".join(ident[:-1]), ident[-1]))
            elif ident:
                params.append(("", ident[0]))
        return re.sub(r"\s+", " ", m.group(1)).strip(), params, cuerpo
    return None


def _returns(cuerpo):
    """[(posicion, valor)] de todos los `return` del cuerpo, en orden."""
    return [(m.start(), m.group(1).strip())
            for m in re.finditer(r"\breturn\s*([^;]*);", cuerpo)]


def _guardas(cuerpo):
    """[(condicion, valor)] de los `if (...) return X;` que abandonan a mitad.

    QUE CUENTA COMO RECHAZO, Y DE DONDE SALE ESE BORDE: el valor devuelto tiene que
    ser uno de los NEGATIVOS del lenguaje (_FALSOS). Asi vale igual para el validador
    `void` de antes de N-160 -`return;` a secas- que para el `bool` de ahora -`return
    false`-, sin que este pack tenga que saber cual de los dos esta mirando.

    🔴 LA PRIMERA VERSION DEDUCIA EL VALOR DE EXITO DEL ULTIMO `return` DEL CUERPO, Y
    ERA FALSO: en una funcion que acaba en `return false` -el mutante del control
    negativo, sin ir mas lejos- esa regla se comia TODAS las guardas y las daba por
    inexistentes. Lo caza el tercer control negativo, que fue el que lo encontro."""
    fuera = []
    for m in re.finditer(r"\bif\s*\(", cuerpo):
        cp = _cierra_paren(cuerpo, m.end() - 1)
        if cp < 0:
            continue
        cond = cuerpo[m.end():cp]
        resto = cuerpo[cp + 1:].lstrip()
        if resto.startswith("{"):
            interior = _bloque(resto, 0)
            if interior is None:
                continue
            cabeza = interior.strip()
        else:
            cabeza = resto
        mr = re.match(r"return\s*([^;]*);", cabeza)
        if not mr:
            continue
        val = mr.group(1).strip()
        if val not in _FALSOS:
            continue                      # no es un rechazo, es un atajo al exito
        cola = cuerpo[cp + 1:]
        cola = cola[cola.find(";") + 1:] if ";" in cola else ""
        if re.search(r"[^\s};]", cola):      # queda codigo util detras: es una guarda
            fuera.append((cond, val))
    return fuera


def _es_guarda_de_valor(cond, locales):
    """True si la guarda decide sobre EL VALOR de la hora, y no sobre el formato.

    EL BORDE, ESCRITO AQUI PORQUE ES EL QUE SEPARA LAS DOS MITADES DE ESTA FUNCION:
    una guarda de FORMATO -`str == nullptr`, `sscanf(...) != 6`- ya existe y ya
    rechaza, pero rechaza por otra cosa; contarla como validacion de la hora dejaria
    aprobado un sembrador que no mira ni una cifra. Se reconocen porque llaman a una
    funcion o llevan una cadena literal dentro, que es exactamente lo que una
    comparacion de rango nunca tiene. Y se exige ademas que nombre alguna de las
    variables que llegan al validador: sin eso no puede opinar sobre la hora.

    (El `d` de "%d-%d-%d" dentro del literal del sscanf ya hizo abortar a este pack una
    vez, el 07/09, haciendose pasar por una variable. De ahi sale la mitad del literal.)"""
    if '"' in cond or re.search(r"\b(?!or\b|and\b|not\b)[A-Za-z_]\w*\s*\(", cond):
        return False
    return any(re.search(r"\b%s\b" % re.escape(l), cond) for l in locales)


def _atajos_al_exito(cuerpo):
    """[(condicion, valor)] de los `if (...) return <no falso>;` que SALTAN el cuerpo.

    POR QUE ESTO ES UNA COMPROBACION Y NO UN DETALLE, Y CUAL ES SU BORDE.
    Con la regla de rango viviendo ya dentro del validador (N-160), la forma mas barata
    de romper esta cadena SIN tocar el sembrador es cambiarle a una guarda su
    `return false` por un `return true`: la hora se sigue descartando -el cuerpo que
    siembra queda detras y no se ejecuta- y la funcion contesta que si. El sembrador
    propaga ese si, el Maestro propaga la hora, y todo el mundo en verde.

    EL BORDE: se acusa un `return` NO FALSO que tenga codigo util detras, o sea uno que
    se lleva por delante el trabajo de la funcion. Un `return` de exito AL FINAL no
    cuenta -ese es el exito de verdad- y por eso se exige la cola con codigo. Es el
    mutante que se inyecto en reloj_ajustarConAcuse() el 07/09 para calibrar esto."""
    fuera = []
    for m in re.finditer(r"\bif\s*\(", cuerpo):
        cp = _cierra_paren(cuerpo, m.end() - 1)
        if cp < 0:
            continue
        resto = cuerpo[cp + 1:].lstrip()
        cabeza = _bloque(resto, 0).strip() if resto.startswith("{") else resto
        if cabeza is None:
            continue
        mr = re.match(r"return\s*([^;]*);", cabeza)
        if not mr or mr.group(1).strip() in _FALSOS:
            continue
        cola = cuerpo[cp + 1:]
        cola = cola[cola.find(";") + 1:] if ";" in cola else ""
        if re.search(r"[^\s};]", cola):
            fuera.append((cuerpo[m.end():cp].strip(), mr.group(1).strip()))
    return fuera


def _compilar(cond, nombres, donde):
    """Traduce una condicion de C++ a un predicado de Python. Levanta si no sabe.

    El whitelist de identificadores es la barrera: cualquier cosa que no sea uno de
    los parametros conocidos -una llamada, un campo, una constante de otro fichero-
    hace que este pack ABORTE en vez de modelar a medias. Un modelo a medias da verde
    sin haber mirado, que es la unica forma que tiene un pack de texto de mentir."""
    py = cond.replace("||", " or ").replace("&&", " and ")
    py = re.sub(r"!(?!=)", " not ", py)
    py = re.sub(r"\btrue\b", "True", py)
    py = re.sub(r"\bfalse\b", "False", py)
    py = re.sub(r"\b(\d+)[uUlL]+\b", r"\1", py)
    py = re.sub(r"\(\s*(?:u?int\d*_t|int|unsigned|long|short|char)\s*\)", " ", py)
    for ident in set(re.findall(r"[A-Za-z_]\w*", py)):
        if ident in ("or", "and", "not", "True", "False"):
            continue
        if ident not in nombres:
            raise _NoMide(
                "la guarda %r de %s nombra %r, que no es una de las variables que esta "
                "cadena mueve (%s). Modelarla a medias seria aprobar sin mirar"
                % (cond, donde, ident, ", ".join(nombres)))
    if re.search(r"\b(?!or\b|and\b|not\b)[A-Za-z_]\w*\s*\(", py):
        raise _NoMide("la guarda %r de %s llama a una funcion: no se modela"
                      % (cond, donde))
    if "[" in py or "?" in py:
        raise _NoMide("la guarda %r de %s no es una comparacion simple" % (cond, donde))
    try:
        code = compile(py, "<guarda>", "eval")
    except SyntaxError as e:
        raise _NoMide("no se pudo traducir la guarda %r de %s (%s)" % (cond, donde, e))

    def predicado(vals):
        return bool(eval(code, {"__builtins__": {}}, dict(vals)))   # noqa: S307

    return predicado


def _limites(cond, nombres):
    """{variable: {numeros contra los que se la compara}} dentro de una condicion.

    De aqui salen los BORDES del barrido, y por eso se releen del C++ en cada corrida:
    un limite escrito a mano aqui seguiria dando PASS el dia que el firmware cambiara
    el suyo, midiendo el valor viejo."""
    fuera = {}
    for n in nombres:
        for m in re.finditer(r"\b%s\s*(?:>=|<=|>|<|==|!=)\s*(-?\d+)" % re.escape(n),
                             cond):
            fuera.setdefault(n, set()).add(int(m.group(1)))
        for m in re.finditer(r"(-?\d+)\s*(?:>=|<=|>|<|==|!=)\s*\b%s\b" % re.escape(n),
                             cond):
            fuera.setdefault(n, set()).add(int(m.group(1)))
    return fuera


# ---------------------------------------------------------------------------------
# LA CADENA DE UNA PUNTA, LEIDA ENTERA DEL FUENTE DE ESA PUNTA
# ---------------------------------------------------------------------------------

class Cadena:

    def __init__(self, etiqueta, codigo):
        self.etiqueta = etiqueta
        self.codigo = codigo

        d = _definicion(codigo, SEMBRADOR)
        if d is None:
            raise _NoMide(
                "no se hallo la definicion de %s(). O el bloque se movio de fichero o "
                "el buscador se quedo atras, y en los dos casos aprobar seria aprobar "
                "sin mirar" % SEMBRADOR)
        self.tipo_sem, self.params_sem, self.cuerpo = d

        # LAS VARIABLES QUE SSCANF ESCRIBE. Son la materia prima de la cadena y de
        # ellas se deriva todo lo demas: quien las recibe es el validador.
        ms = re.search(r"\bsscanf\s*\(", self.cuerpo)
        if not ms:
            raise _NoMide(
                "%s() ya no parsea con sscanf(). La cadena de siembra empieza ahi: sin "
                "ese punto de partida este pack no sabe que variables seguir"
                % SEMBRADOR)
        args = _argumentos(self.cuerpo, ms.end() - 1) or []
        self.parseadas = [re.findall(r"[A-Za-z_]\w*", a)[-1]
                          for a in args if a.strip().startswith("&")]
        if len(self.parseadas) < 3:
            raise _NoMide(
                "sscanf() de %s() solo escribe %d variables. Con menos de tres no hay "
                "hora que validar y este pack estaria midiendo otra cosa"
                % (SEMBRADOR, len(self.parseadas)))

        # EL VALIDADOR SE DERIVA, NO SE NOMBRA: es la funcion a la que el sembrador le
        # pasa esas variables. Atarlo a un nombre es lo que rompio la primera version
        # de este pack cuando N-160 renombro reloj_ajustar -> reloj_ajustarConAcuse.
        candidatos = {}
        for m in re.finditer(r"\b([A-Za-z_]\w*)\s*\(", self.cuerpo):
            nombre = m.group(1)
            if nombre in _NO_SON_LLAMADAS or nombre == "sscanf":
                continue
            a = _argumentos(self.cuerpo, m.end() - 1) or []
            usados = [i for i, x in enumerate(a)
                      if any(re.search(r"\b%s\b" % v, x) for v in self.parseadas)]
            if len(usados) >= 3:
                candidatos.setdefault(nombre, m.start())
        if len(candidatos) != 1:
            raise _NoMide(
                "el validador de la hora no se pudo derivar: %s() pasa las variables "
                "de sscanf a %d funciones (%s). Con cero no hay validacion que mirar y "
                "con dos este pack no sabe cual manda; en los dos casos hay que volver "
                "a escribir el pack, no creerse su verde"
                % (SEMBRADOR, len(candidatos), ", ".join(sorted(candidatos)) or "-"))
        self.validador, self.pos_llamada = list(candidatos.items())[0]

        dv = _definicion(codigo, self.validador)
        if dv is None:
            raise _NoMide(
                "%s() llama a %s(), pero su definicion no esta en este fichero: la "
                "validacion se mudo y este pack no la puede leer"
                % (SEMBRADOR, self.validador))
        self.tipo_val, self.params_val, self.cuerpo_val = dv

        # El mapeo argumento -> parametro es POSICIONAL, que es lo unico que el C++
        # garantiza: los nombres locales pueden llamarse como quieran.
        args_ll = _argumentos(self.cuerpo, self.cuerpo.find("(", self.pos_llamada)) or []
        self.mapa = {}
        for i, a in enumerate(args_ll):
            ident = re.findall(r"[A-Za-z_]\w*",
                               re.sub(r"\(\s*[\w ]+\s*\)", " ", a))
            if ident and i < len(self.params_val):
                self.mapa[ident[-1]] = self.params_val[i][1]

        self.g_val = _guardas(self.cuerpo_val)
        self.g_sem = _guardas(self.cuerpo)
        rets = _returns(self.cuerpo)
        self.exitos = [v for _, v in rets if v not in _FALSOS]

    # -- las dos formas honestas de depender ------------------------------------

    def propaga(self):
        """El veredicto del validador llega al retorno del sembrador.

        Tres formas, y las tres valen: devolverlo, preguntarlo en un `if` que gobierna
        el retorno, o guardarlo en una variable que luego se consulta. No se exige una
        de las tres a proposito: un instrumento que exige UNA FORMA empuja a escribir
        lo que el instrumento quiere leer en vez de lo que hace falta."""
        if any(_llama_a(r, self.validador) for r in self.exitos):
            return True
        if not _consumido(self.cuerpo, self.pos_llamada):
            return False
        for cond in _gobernantes(self.cuerpo, self.pos_llamada):
            if _llama_a(cond, self.validador):
                return True
        m = re.search(r"([A-Za-z_]\w*)\s*=\s*[^;]*\b%s\s*\(" % re.escape(self.validador),
                      self.cuerpo)
        if m:
            v = m.group(1)
            if any(re.search(r"\b%s\b" % v, r) for r in self.exitos):
                return True
            if any(re.search(r"\b%s\b" % v, c) for c, _ in self.g_sem):
                return True
        return False

    def guarda_propia(self):
        """El sembrador decide el mismo: guardas suyas sobre los valores de la hora.

        Se cuentan solo las que deciden sobre el VALOR de la hora (ver
        _es_guarda_de_valor): una guarda sobre `str` o sobre el resultado del sscanf es
        de FORMATO, y el formato ya se rechaza aparte."""
        return [c.strip() for c, _ in self.g_sem
                if _es_guarda_de_valor(c, self.mapa)]

    def depende(self):
        return self.propaga() or bool(self.guarda_propia())

    # -- el modelo que se barre --------------------------------------------------

    def bordes(self):
        """El dominio del barrido, y POR QUE es el borde correcto.

        Un `>` o un `<` solo cambian de veredicto entre L y L+1, asi que los bordes son
        cada limite L que las guardas comparan -releido del C++, ninguno escrito aqui-
        y sus vecinos L-1 y L+1.

        Y LOS EXTREMOS SALEN DEL TIPO DEL PARAMETRO, que es lo que decide que puede
        llegar de verdad:
          - parametro con signo (`int`): se anaden -1 y 256. El -1 porque sscanf("%d")
            acepta "-5" sin protestar; el 256 porque es EXACTAMENTE el primer valor que
            un cast a uint8_t convierte en 0, o sea el que colaba una medianoche falsa
            antes de N-160. Los dos quedan barridos para que esa regresion no pueda
            volver en silencio.
          - parametro sin signo de 8 bits: 0 y 255, que son sus extremos reales.
        Barrer un dominio que el tipo no admite mediria una funcion que no existe."""
        lim = {}
        for c, _ in self.g_val:
            for n, vals in _limites(c, [p for _, p in self.params_val]).items():
                lim.setdefault(n, set()).update(vals)
        for c, _ in self.g_sem:
            for l, vals in _limites(c, list(self.mapa)).items():
                if self.mapa.get(l):
                    lim.setdefault(self.mapa[l], set()).update(vals)
        dom = {}
        for tipo, p in self.params_val:
            t = tipo.replace(" ", "")
            if t.startswith("u") or "unsigned" in tipo:
                vals = {0, 255}
            else:
                vals = {-1, 0, 256}
            for L in lim.get(p, ()):
                vals.update((L - 1, L, L + 1))
            dom[p] = sorted(vals)
        return dom

    def barrer(self):
        """(casos, contraejemplos) de: si el validador RECHAZA, el sembrador dice NO.

        La implicacion va en un solo sentido a proposito. El sembrador puede rechazar
        mas -el formato, el anio- y eso no es un defecto; lo que no puede es decir que
        si por una hora que el validador tiro.

        Las guardas del sembrador que hablan SOLO de variables que no llegan al
        validador -el anio, el mes- se evaluan como que NO disparan, y ese es el caso
        peor para el firmware a proposito: una guarda sobre el anio no puede salvar una
        hora imposible, y suponer lo contrario aprobaria por un rechazo que no viene de
        la validacion de la hora."""
        nombres_val = [p for _, p in self.params_val]
        preds_v = [_compilar(c, nombres_val, "%s()" % self.validador)
                   for c, _ in self.g_val]
        preds_s = [_compilar(c, list(self.mapa), "%s()" % SEMBRADOR)
                   for c, _ in self.g_sem if _es_guarda_de_valor(c, self.mapa)]
        propaga = self.propaga()
        dom = self.bordes()
        inverso = {p: l for l, p in self.mapa.items()}

        casos, malos = 0, []
        idx = [0] * len(nombres_val)
        while True:
            vals = {n: dom[n][idx[i]] for i, n in enumerate(nombres_val)}
            casos += 1
            if any(p(vals) for p in preds_v):
                locales = {l: vals[p] for l, p in self.mapa.items()}
                if not (propaga or any(p(locales) for p in preds_s)):
                    malos.append({inverso.get(n, n): v for n, v in vals.items()})
            i = len(nombres_val) - 1
            while i >= 0:
                idx[i] += 1
                if idx[i] < len(dom[nombres_val[i]]):
                    break
                idx[i] = 0
                i -= 1
            if i < 0:
                return casos, malos


# ---------------------------------------------------------------------------------
# LOS LLAMADORES
# ---------------------------------------------------------------------------------

def _rama(codigo, etiqueta):
    """El bloque de la rama `strncmp(accion, "<etiqueta>", n)` del despachador."""
    m = re.search(r'strn?cmp\s*\(\s*(?:accion|cmd)\s*,\s*"%s"' % re.escape(etiqueta),
                  codigo)
    if not m:
        return None
    llave = codigo.find("{", m.end())
    if llave < 0:
        return None
    return _bloque(codigo, llave)


# ---------------------------------------------------------------------------------

def correr(b, fw):
    b.titulo("La siembra de la hora: un `true` que no depende de nada")

    # ---- 0. LOS SUJETOS EXISTEN, Y HAY ALGO QUE MEDIR ------------------------
    #
    # Se censa ANTES de comprobar nada. Un pack que mide un conjunto vacio aprueba
    # siempre, y este mide una cadena que puede haberse mudado entera: si se mudo,
    # ABORTA. Un ABORTADO grita; un hueco no (N-43).
    cadenas = {}
    for p in PUNTAS:
        try:
            cadenas[p] = Cadena(p, fw.codigo(*RELOJ_C[p]))
        except _NoMide as e:
            raise fw.Abortado("%s: %s" % (p, e))

    for p in PUNTAS:
        cad = cadenas[p]
        if not re.search(r"\bbool\s+%s\s*\(" % re.escape(SEMBRADOR),
                         fw.codigo(*RELOJ_H[p])):
            raise fw.Abortado(
                "%s: reloj.h ya no declara `bool %s(...)`. Si el sembrador dejo de "
                "devolver un booleano, la propiedad que este pack mide -que ese retorno "
                "dependa de la validacion- hay que volver a escribirla, no darla por "
                "cumplida" % (p, SEMBRADOR))
        if not cad.g_val:
            raise fw.Abortado(
                "%s: %s() no tiene ni una guarda que rechace. Sin validacion no hay "
                "nada de lo que el retorno pueda depender, y este pack estaria "
                "aprobando sobre un conjunto vacio" % (p, cad.validador))
        if not cad.exitos:
            raise fw.Abortado(
                "%s: %s() no tiene ningun retorno de exito: o es `void` ya, o el cuerpo "
                "se movio" % (p, SEMBRADOR))

    b.verificar(
        all(len(cadenas[p].mapa) == len(cadenas[p].params_val) for p in PUNTAS),
        "censo leido del C++: %s" % " · ".join(
            "%s -> validador derivado %s(%s), `%s`, con %d guarda(s) de rechazo"
            % (p, cadenas[p].validador,
               ", ".join(n for _, n in cadenas[p].params_val),
               cadenas[p].tipo_val, len(cadenas[p].g_val)) for p in PUNTAS),
        "no se pudo mapear cada argumento de la llamada con su parametro en alguna "
        "punta (%s). Sin ese mapeo el barrido de bordes mediria una variable por otra, "
        "que es peor que no medir" % " · ".join(
            "%s: %d de %d" % (p, len(cadenas[p].mapa), len(cadenas[p].params_val))
            for p in PUNTAS))

    # ---- 1. EL RETORNO DE EXITO DEPENDE DE LA VALIDACION ---------------------
    #
    # LA COMPROBACION CENTRAL, y admite las DOS formas honestas de cumplirla: que el
    # veredicto del validador llegue al retorno, o que el sembrador valide el mismo. Lo
    # que no se admite es la tercera: un literal, que no puede depender de nada.
    for p in PUNTAS:
        cad = cadenas[p]
        b.verificar(
            cad.depende(),
            "%s: el retorno de exito de %s() depende de la validacion — %s"
            % (p, SEMBRADOR,
               "propaga el veredicto de %s()" % cad.validador if cad.propaga()
               else "valida el mismo: %s" % " · ".join(cad.guarda_propia())),
            "%s: %s() devuelve `%s` sin que ese valor dependa de nada. %s() es `%s` y "
            "rechaza por su cuenta (%s), asi que ese retorno no dice 'la hora quedo "
            "puesta', dice 'la cadena tenia los campos'. Quien lo mira -en el Maestro, "
            "la puerta de la propagacion al Esclavo- se cree lo primero"
            % (p, SEMBRADOR, " y ".join(cad.exitos), cad.validador, cad.tipo_val,
               " · ".join(c.strip() for c, _ in cad.g_val)))

    # ---- 1.bis. Y EL VALIDADOR NO CONTESTA QUE SI SIN HABER SEMBRADO ---------
    #
    # La 1 mira el sembrador; esta mira el otro extremo del mismo `return`. Desde N-160
    # la regla de rango vive DENTRO del validador, asi que ahi es donde una sola palabra
    # -`false` por `true` en una guarda- vuelve a poner en marcha el defecto entero sin
    # tocar el sembrador: la hora se descarta igual y la funcion contesta exito. Sin
    # esta linea, el pack aprobaria esa version letra por letra.
    for p in PUNTAS:
        cad = cadenas[p]
        atajos = _atajos_al_exito(cad.cuerpo_val)
        # El mensaje se arma ANTES de la llamada -Python evalua los argumentos-, asi que
        # el caso vacio necesita su valor: un IndexError aqui aborta el pack entero y
        # deja sin medir todo lo que venia detras.
        atajo = atajos[0] if atajos else ("", "")
        b.verificar(
            not atajos,
            "%s: ninguna salida temprana de %s() afirma exito saltandose la siembra"
            % (p, cad.validador),
            "%s: %s() sale con `return %s` en la guarda `%s` y se salta el cuerpo que "
            "siembra la hora. Descarta y contesta que si a la vez, y el sembrador "
            "propaga ese si: el defecto entero vuelve con una sola palabra cambiada"
            % (p, cad.validador, atajo[1], atajo[0]))

    # ---- 2. Y LA DEPENDENCIA ES LA CORRECTA EN CADA BORDE --------------------
    #
    # La 1 mide que HAYA dependencia; esta mide que sea la de verdad, barriendo las
    # combinaciones de bordes. Las dos pueden divergir, y ahi esta su valor: una guarda
    # copiada con el limite mal -`h > 24` contra el `hora > 23` del validador- pasa la 1
    # y cae aqui, en el unico valor donde las dos funciones no opinan igual. Lo ejerce
    # el tercer control negativo.
    #
    # Va en propiedad() y no en verificar(): cuando falla, el banco no se ha equivocado,
    # ha ROTO una regla reproduciendo el escenario. Se leen distinto.
    for p in PUNTAS:
        cad = cadenas[p]
        try:
            casos, malos = cad.barrer()
        except _NoMide as e:
            raise fw.Abortado("%s: %s" % (p, e))
        ejemplo = ", ".join("%s=%d" % (k, v) for k, v in sorted(malos[0].items())) \
            if malos else ""
        b.propiedad(
            not malos,
            "%s: %d casos barridos en los bordes de las guardas (%s): ni uno en el que "
            "%s() descarte la hora y %s() devuelva exito"
            % (p, casos, " · ".join("%s en %s" % (n, cad.bordes()[n])
                                    for _, n in cad.params_val),
               cad.validador, SEMBRADOR),
            "%s: con %s, %s() DESCARTA la hora y %s() devuelve exito igual (%d casos de "
            "%d en el barrido de bordes). El equipo se queda con la hora que tenia y la "
            "cadena dice que la puso"
            % (p, ejemplo, cad.validador, SEMBRADOR, len(malos), casos))

    # ---- 3. LA PROPAGACION AL OTRO POSTE ESTA GOBERNADA ----------------------
    #
    # D-20: el Maestro EMPUJA la hora al Esclavo y el Esclavo la acepta sobrescribiendo
    # lo que tuviera. No hay filtro despues, asi que lo que salga de aqui es lo que el
    # cruce va a usar: propagar una hora que ESTA punta descarto es mandar a la otra una
    # hora que nadie adopto.
    #
    # EL BORDE: se exige gobernada TODA llamada de la rama menos dos, y las dos se
    # nombran y se comprueba que existen. El sembrador queda fuera porque es el que
    # decide; el reportador de eventos queda fuera porque su literal no afirma exito
    # -y de eso se ocupa la comprobacion 4, para que el hueco no quede sin vigilar-.
    if not re.search(r"\b%s\s*\(" % re.escape(REPORTADOR), fw.codigo(*BLUETOOTH_H)):
        raise fw.Abortado(
            "Maestro: bluetooth.h no declara %s(), que es una de las dos excepciones "
            "que la comprobacion 3 se permite. Una excepcion cuyo sujeto no existe es "
            "una excepcion que ya no excluye lo que creia (N-96)" % REPORTADOR)

    ramas = {}
    for p in PUNTAS:
        ramas[p] = _rama(fw.codigo(*BLUETOOTH[p]), RAMA_BT)
        if ramas[p] is None:
            raise fw.Abortado(
                "%s: no se hallo la rama %s del despachador de bluetooth.cpp. Es la "
                "puerta por la que la hora entra a esta punta: sin ella este pack no "
                "tiene llamador que medir" % (p, RAMA_BT))

    for p in PUNTAS:
        rama = ramas[p]
        sueltas = []
        for m in re.finditer(r"\b([A-Za-z_]\w*)\s*\(", rama):
            nombre = m.group(1)
            if nombre in _NO_SON_LLAMADAS or nombre in (SEMBRADOR, REPORTADOR):
                continue
            if not any(_llama_a(c, SEMBRADOR) for c in _gobernantes(rama, m.start())):
                sueltas.append(nombre)
        b.verificar(
            not sueltas,
            "%s / %s: todo lo que la rama hace ademas de sembrar esta DENTRO del bloque "
            "que gobierna el retorno de %s()%s"
            % (p, RAMA_BT, SEMBRADOR,
               "" if p == "Maestro" else " (esta punta no propaga nada)"),
            "%s / %s: %s corre SIN que el retorno de %s() lo gobierne. En el Maestro eso "
            "es coordinador_sincronizarHora() empujando al Esclavo una hora que esta "
            "punta puede haber descartado, y D-20 dice que el Esclavo hace caso SIEMPRE: "
            "no hay nada mas abajo que lo pare"
            % (p, RAMA_BT, ", ".join("%s()" % s for s in sorted(set(sueltas))),
               SEMBRADOR))

    # ---- 4. NINGUN LITERAL DE LA RAMA AFIRMA EXITO SIN ESTAR GOBERNADO -------
    #
    # Hoy las dos puntas emiten SET_RTC_LO_ACUSA_EL_PUENTE, que dice QUIEN contesta y no
    # que haya salido bien: por eso esta pasa hoy en las dos. No es una casilla de
    # adorno, es un trinquete -el dia que ese literal diga OK o PUESTA, cae-. El borde
    # esta escrito arriba, en PALABRAS_DE_EXITO, con su motivo.
    for p in PUNTAS:
        rama = ramas[p]
        mentirosos = []
        for m in re.finditer(r'"((?:[^"\\]|\\.)*)"', rama):
            lit = m.group(1)
            if not any(w in lit for w in PALABRAS_DE_EXITO):
                continue
            if not any(_llama_a(c, SEMBRADOR) for c in _gobernantes(rama, m.start())):
                mentirosos.append(lit)
        b.verificar(
            not mentirosos,
            "%s / %s: ningun literal de la rama afirma exito fuera del bloque que "
            "gobierna la siembra" % (p, RAMA_BT),
            "%s / %s: %s afirma exito y sale pase lo que pase con la siembra. El tecnico "
            "se va del poste con una confirmacion de algo que puede no haber ocurrido"
            % (p, RAMA_BT, " · ".join(repr(x) for x in mentirosos)))

    # ---- 5. EL DIARIO DE ORDENES DEPENDE DEL RETORNO, EN LAS DOS PUNTAS -------
    #
    # POR QUE ESTA COMPROBACION EXISTE, Y POR QUE NO LA CUBRE LA 4 [08/09].
    #
    # La 4 mira LITERALES de exito y deja fuera SET_RTC_LO_ACUSA_EL_PUENTE con un motivo
    # escrito arriba: "el literal que LAS DOS PUNTAS emiten hoy dice QUIEN contesta, no que
    # haya salido bien … cobrarle una guarda empujaria a quitarlo o a inventarse un
    # rechazo". Esa razon es una AFIRMACION SOBRE EL CODIGO, y al volver a medirla el 08/09
    # sus dos mitades fallan:
    #
    #   1. las dos puntas ya NO emiten lo mismo. El Esclavo emite DOS literales, y su
    #      propio comentario dice por que: "una siembra RECHAZADA por rango dejaba en el
    #      diario la misma linea que una aceptada: el unico registro que le queda al
    #      tecnico decia que la hora entro cuando no habia entrado";
    #   2. cobrarle la guarda NO empujo a inventarse un rechazo: el Esclavo escribio uno
    #      CIERTO, derivado del retorno.
    #
    # Asi que la excepcion de la 4 se conserva -su borde sigue siendo el bueno para
    # LITERALES- y lo que faltaba se mide aqui, que es otra propiedad: no QUE dice la linea,
    # sino SI DEPENDE del retorno. El diario es el unico registro que le queda al tecnico
    # cuando se baja del poste, y en el Maestro ademas es el que se consulta cuando las dos
    # puntas discrepan, porque es el que propaga.
    #
    # EL BORDE DE ESTA: se exige de TODA llamada al reportador dentro de la rama. Si manana
    # la rama gana una linea de diario que deba salir pase lo que pase -un "SET_RTC
    # RECIBIDO" de traza, por ejemplo-, esta comprobacion la cobra y hay que escribir aqui
    # por que se excluye. Es lo correcto: una excepcion nueva se mide al escribirla.
    for p in PUNTAS:
        rama = ramas[p]
        sueltas = []
        for m in re.finditer(r"\b%s\s*\(" % re.escape(REPORTADOR), rama):
            if not _bajo_decision(rama, m.start(), SEMBRADOR):
                sueltas.append(rama[m.start():m.start() + 90].split("\n")[0])
        b.verificar(
            not sueltas,
            "%s / %s: toda linea del Diario de Ordenes depende del retorno de %s(): el "
            "registro dice si la hora entro" % (p, RAMA_BT, SEMBRADOR),
            "%s / %s: %d llamada(s) a %s() salen FUERA del bloque que gobierna la siembra, "
            "empezando por `%s`. El diario deja la misma linea para una hora aceptada que "
            "para una rechazada, y es el unico registro que le queda al tecnico"
            % (p, RAMA_BT, len(sueltas), REPORTADOR,
               sueltas[0].strip() if sueltas else ""))

    # ---- QUIEN TIRA EL RETORNO, MEDIDO Y NO CONTADO --------------------------
    #
    # Se mide en vez de afirmarse -una frase dentro de un reportar() envejece igual que
    # dentro de una excepcion, y nadie la recompila-. Por que no cuenta: ver la cabecera.
    tiran = []
    for p in PUNTAS:
        m = re.search(r"\b%s\s*\(" % re.escape(SEMBRADOR), ramas[p])
        if m and not _consumido(ramas[p], m.start()):
            tiran.append(p)
    if tiran:
        b.reportar(
            "la rama %s TIRA el retorno de %s() en: %s"
            % (RAMA_BT, SEMBRADOR, ", ".join(tiran)),
            ["El Maestro lo consume -`if (%s(accion + 8))`- y de ese `if` cuelga la "
             "propagacion al Esclavo. La otra punta lo llama como sentencia suelta."
             % SEMBRADOR,
             "NO se cuenta como comprobacion: esa rama no acusa nada ni actua sobre el "
             "resultado, asi que no hay afirmacion que pueda mentir. Cobrarlo aqui "
             "dejaria un rojo que este pack no puede apagar desde su propio fichero, y "
             "un rojo que no se apaga construyendo es el que tienta a decorarlo.",
             "Lo que falta ahi es otra propiedad -que ALGUIEN se entere de que la "
             "siembra fallo- y vive en bluetooth.cpp. Cuando esa rama gane un $ACK, la "
             "comprobacion 4 de este pack la exige gobernada sin tocar una linea."])

    # ---- CONTROLES NEGATIVOS -------------------------------------------------
    #
    # Un arnes que no se ha visto fallar es un adorno que da verde. Los cuatro atacan
    # las cuatro formas en que este pack podria dar un verde vacio: aprobar el defecto,
    # acusar al arreglo, no notar un limite mal copiado, y leer por proximidad en vez de
    # por bloque. Los tres primeros pasan por el MISMO constructor que las cadenas de
    # verdad -incluida la derivacion del validador-, porque un control negativo que
    # ejercita otro camino no controla nada.
    #
    # EL MUTANTE ES EL DEFECTO REAL, COPIADO LITERAL DE `git show HEAD:reloj.cpp` [07/09]
    # antes de que N-160 lo arreglara. Un mutante inventado prueba que el detector
    # detecta algo; este prueba que detecta LO QUE PASO.
    MALO = """
void reloj_ajustar(uint8_t hora, uint8_t minuto, uint8_t segundo, uint8_t dia) {
  if (hora > 23 || minuto > 59 || segundo > 59) return;
  if (dia > 31) return;
  segBaseDelDia = 0;
  horaValida = true;
}
bool reloj_sembrarDesdeIso(const char* str) {
  if (str == nullptr) return false;
  int anio = 0, mes = 0, dia = 0, h = 0, m = 0, s = 0;
  if (sscanf(str, "%d-%d-%d,%d:%d:%d", &anio, &mes, &dia, &h, &m, &s) == 6) {
    reloj_ajustar((uint8_t)h, (uint8_t)m, (uint8_t)s, (uint8_t)dia);
    return true;
  }
  return false;
}
"""
    malo = Cadena("mutante", MALO)
    casos_m, malos_m = malo.barrer()
    b.control_negativo(
        not malo.depende() and any(e["h"] == 24 for e in malos_m),
        "el defecto REAL de antes de N-160 -llamada suelta y `return true` literal- cae "
        "en la 1 y en la 2, con h=24 entre los %d contraejemplos: la guarda de formato "
        "sobre `str` no se confunde con la validacion de la hora" % len(malos_m))

    BUENO = MALO.replace(
        "    reloj_ajustar((uint8_t)h, (uint8_t)m, (uint8_t)s, (uint8_t)dia);\n"
        "    return true;",
        "    return reloj_ajustar((uint8_t)h, (uint8_t)m, (uint8_t)s, (uint8_t)dia);")
    BUENO = BUENO.replace("void reloj_ajustar", "bool reloj_ajustar")
    BUENO = BUENO.replace("segundo > 59) return;", "segundo > 59) return false;")
    BUENO = BUENO.replace("dia > 31) return;", "dia > 31) return false;")
    BUENO = BUENO.replace("horaValida = true;", "horaValida = true;\n  return true;")
    bueno = Cadena("arreglado", BUENO)
    b.control_negativo(
        bueno.depende() and not bueno.barrer()[1],
        "el arreglo que DEVUELVE el veredicto del validador pasa las dos: el detector "
        "distingue, no acusa a todo el que llama")

    COPION = MALO.replace(
        "  if (str == nullptr) return false;",
        "  if (str == nullptr) return false;\n"
        "  if (h > 24 || m > 59 || s > 59) return false;")
    copion = Cadena("copion", COPION)
    casos_c, malos_c = copion.barrer()
    b.control_negativo(
        copion.depende() and any(e.get("h") == 24 for e in malos_c),
        "una guarda propia con el limite copiado mal (`h > 24` contra el `hora > 23` del "
        "validador) PASA la comprobacion 1 y CAE en la 2, exactamente en h=24: el "
        "barrido mide el limite, no la presencia de la guarda")

    # EL SOLAPAMIENTO DE NOMBRES, QUE ES EL BUSCADOR CIEGO DE ESTA CADENA. Las dos
    # funciones que deciden se llaman `reloj_ajustar` y `reloj_ajustarConAcuse`, y la
    # primera es PREFIJO de la segunda: con un `in` en vez de un limite de palabra, el
    # envoltorio sin guardas se lee como el validador y este pack aprueba propagaciones
    # que no existen. Se ejerce en los DOS sentidos porque el solapamiento tiene dos.
    b.control_negativo(
        _llama_a("return reloj_ajustarConAcuse(h, m, s, dia);", "reloj_ajustarConAcuse")
        and not _llama_a("return reloj_ajustarConAcuse(h, m, s, dia);", "reloj_ajustar")
        and not _llama_a("reloj_ajustar(h, m, s, dia);", "reloj_ajustarConAcuse"),
        "`reloj_ajustar` y `reloj_ajustarConAcuse` NO se confunden en ninguno de los "
        "dos sentidos: se busca la llamada exacta, no el substring")

    falso_bloque = ('{ if (reloj_sembrarDesdeIso(x)) { nada(); } '
                    'coordinador_sincronizarHora(); }')
    pos = falso_bloque.find("coordinador_sincronizarHora")
    b.control_negativo(
        not any(_llama_a(c, SEMBRADOR) for c in _gobernantes(falso_bloque, pos)),
        "una propagacion escrita JUSTO DEBAJO del `if` pero fuera de sus llaves se lee "
        "como NO gobernada: se mide el bloque, no la proximidad de lineas (N-89)")

    # La 5 tiene que distinguir el defecto REAL que el Maestro tuvo hasta el 08/09 -diario
    # fuera del `if`- del arreglo -diario en las dos ramas-. Se ejerce en los dos sentidos:
    # una comprobacion que no dejara pasar NADA aprobaria el caso malo igual de bien.
    diario_fuera = ('{ if (reloj_sembrarDesdeIso(x)) { coordinador_sincronizarHora(); } '
                    'bluetooth_reportarEvento("APP_BLUETOOTH", "SET_RTC_LO_ACUSA_EL_PUENTE"); }')
    diario_dentro = ('{ if (reloj_sembrarDesdeIso(x)) { coordinador_sincronizarHora(); '
                     'bluetooth_reportarEvento("APP_BLUETOOTH", "OK"); } '
                     'else { bluetooth_reportarEvento("APP_BLUETOOTH", "RECHAZADO"); } }')

    def _diario_suelto(txt):
        return [m.start() for m in re.finditer(r"\b%s\s*\(" % re.escape(REPORTADOR), txt)
                if not _bajo_decision(txt, m.start(), SEMBRADOR)]

    b.control_negativo(
        len(_diario_suelto(diario_fuera)) == 1 and not _diario_suelto(diario_dentro),
        "el diario FUERA del `if` se acusa y el diario en las DOS ramas pasa: la 5 "
        "distingue el defecto que el Maestro tuvo del arreglo, no acusa a todo el que "
        "escribe en el diario")

    # Y la simetrica, que es la que impide reutilizar el helper equivocado: `_gobernantes`
    # -el de las comprobaciones 3 y 4- da NO GOBERNADA la linea del `else`, y con el la 5
    # acusaria al arreglo bueno de las dos puntas. Se ejerce para que quede medido que las
    # dos preguntas son distintas, y no confiado a que alguien lea el docstring.
    pos_else = diario_dentro.find('bluetooth_reportarEvento("APP_BLUETOOTH", "RECHAZADO")')
    b.control_negativo(
        _bajo_decision(diario_dentro, pos_else, SEMBRADOR)
        and not any(_llama_a(c, SEMBRADOR)
                    for c in _gobernantes(diario_dentro, pos_else)),
        "la linea del `else` DEPENDE del veredicto (la 5 la acepta) y NO esta gobernada "
        "por el exito (la 3 la rechazaria): son dos preguntas distintas y se miden con "
        "helpers distintos")
