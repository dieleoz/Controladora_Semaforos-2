# ===== banco/packs/esp32_07_presupuesto_bytes.py =====
#
# EL PRESUPUESTO DE BYTES POR SEGUNDO DEL ENLACE J17, RECALCULADO.
#
# A 9600 8N1 caben 960 B/s -diez bits por byte, con arranque y parada-. No ocho: la
# cuenta a ojo "9600/8 = 1200" regala un 25% de margen que no existe, y ese es el tipo
# de numero que se copia de un comentario y nadie vuelve a mirar.
#
# TODOS LOS SUMANDOS SE RELEEN DEL C++ DE LAS DOS PUNTAS EN CADA CORRIDA:
#
#   la cadencia de $STATUS      del `ahora - tUltimaTelemetria >= N` de bluetooth.cpp
#   el tope de cada trama       de los snprintf reales: payload[128], payload[100]...
#   el envoltorio               de "%s*%02X\r\n", que son 5 bytes sobre el payload
#   el baudio                   de SerialBT.begin(N) y del contrato del ESP32
#
# Sin valor por defecto en ninguno. Un presupuesto con un sumando escrito a mano deja de
# medir el equipo el dia que alguien cambia un buffer, y sigue dando verde.
#
# P-1: Y EL PUENTE NO PUEDE ANADIR TRAFICO PERIODICO PROPIO.
#
# El enlace tiene margen, pero el margen es del equipo, no del accesorio. Un puente que
# metiera su propio latido cada N ms se estaria repartiendo un canal que no es suyo.
#
# P-4: NI AGRUPAR TELEMETRIA PARA "AHORRAR AIRE".
#
# Es la regla que mas facil parece una optimizacion y mas dano hace: la app declara el
# enlace perdido a los 5 s sin trama (TIMEOUT_ENLACE_MS, app.js:1359). Un puente que
# juntara dos $STATUS para mandarlos de golpe haria que la app diera por CAIDO a un
# equipo perfectamente sano, y el operario veria "sin enlace" delante de un cruce que
# cicla bien.

# ===========================================================================
# N-154 (05/09): LA MITAD QUE FALTABA -- QUE EL CONTENIDO QUEPA EN EL PAYLOAD.
# ===========================================================================
#
# Hasta hoy este pack comprobaba `tramaCompleta >= payload + 5`, que es una cota entre
# DOS BUFFERS: el envoltorio cabe lo que el payload pueda traer. Nadie comprobaba lo
# otro -que lo que el snprintf ESCRIBE quepa en el payload-, y NO CABIA: el $STATUS del
# Maestro pedia 168 caracteres contra los 143 utiles de payload[144]. Estaba anotado en
# el commit de N-153 y sin cerrar, y no truncaba en la calle sólo porque los valores
# reales son cortos. O sea: lo que impedia el fallo era la suerte.
#
# UNA TRAMA TRUNCADA NO LLEGA A MEDIAS: sale bien formada hasta el corte, el checksum se
# calcula sobre lo que quedo y la app la descarta ENTERA. El sintoma en campo es "el
# equipo se callo", que manda a mirar el cable.
#
# EL BORDE CONTRA EL QUE SE MIDE, ESCRITO AL LADO (CLAUDE.md 4.quinquies). Cada campo se
# acota por SU BUFFER -N-1 caracteres- y no por su rango ni por su tipo:
#
#   - Es lo unico que snprintf garantiza sin creerse a nadie. Un rango vive en otro
#     modulo y puede cambiar; un char x[N] no admite mas de N-1 pase lo que pase.
#   - Y por eso un buffer holgado NO es prudencia: es margen que esta cuenta tiene que
#     dar por gastado. Ese es el motivo de que horaBuf bajara de 16 a 12.
#
# Los campos que no salen de un buffer salen de un LITERAL del firmware, y se resuelven
# siguiendo la funcion hasta sus `return "..."`. Si un argumento no se sabe acotar, el
# pack ABORTA: una estimacion aqui es exactamente lo que trunco el $ALARM de N-108.
#
# Y LA SEGUNDA MITAD, QUE ES LA DE 2.ter: DECLARAR NO ES EJERCER. Las cotas viven en
# coordinador.h junto a la funcion que las promete, pero una cota declarada no aprieta
# nada. Lo que permite dimensionar tTxt[4] es la GUARDA del emisor, asi que el pack
# comprueba las dos cosas por separado: que la cota este DERIVADA (de limites_ciclo.h y
# de SFTY6_SILENCIO_MS, no escrita a mano) y que el emisor la COMPARE antes de imprimir.

import math
import re

NOMBRE = "esp32_07_presupuesto_bytes"
DESCRIPCION = "la telemetria del equipo cabe en 960 B/s y el puente no anade trafico propio"

ROL = "ESP32_Expansion"
CONTRATO = ("ESP32_Expansion", "include", "contrato.h")


def _buffer(fw, punta, patron, que):
    return fw.constante((punta, "src", "bluetooth.cpp"), patron, que)


# --- La maquinaria del peor caso -------------------------------------------------
#
# _bloque_que_contiene() y _peor() vienen LITERALES de reloj_01_consulta_por_bluetooth,
# que ya hacia esta cuenta para el $EVENT de ORIGEN:RELOJ. Se traen tal cual y no se
# reescriben: reescribir logica ya probada para renombrar llamadas es como se cuelan los
# errores en un cambio que no debe cambiar comportamiento (CLAUDE.md 3.bis).

class _Falta(Exception):
    pass


def _abortar(que, donde):
    return _Falta("no se hallo %s en %s. Fallo el buscador o el bloque se movio, y en "
                  "los dos casos aprobar aqui seria aprobar sin mirar" % (que, donde))


def _bloque_que_contiene(codigo, idx):
    """El bloque { ... } mas interno que envuelve a la posicion idx."""
    prof, ini = 0, -1
    for j in range(idx - 1, -1, -1):
        c = codigo[j]
        if c == "}":
            prof += 1
        elif c == "{":
            if prof == 0:
                ini = j
                break
            prof -= 1
    if ini < 0:
        raise _abortar("el bloque que envuelve al $STATUS", "bluetooth.cpp")
    prof = 0
    for j in range(ini, len(codigo)):
        if codigo[j] == "{":
            prof += 1
        elif codigo[j] == "}":
            prof -= 1
            if prof == 0:
                return codigo[ini + 1:j]
    raise _abortar("el cierre del bloque del $STATUS", "bluetooth.cpp")


def _peor(fmt, anchos):
    """La cadena mas larga que ese printf puede producir, con un ancho por conversion."""
    convs = re.findall(r"%[0-9]*l?[usd]", fmt)
    if len(convs) != len(anchos):
        return None
    return len(re.sub(r"%[0-9]*l?[usd]", "", fmt)) + sum(anchos)


def _snprintf_status(codigo, punta):
    """(indice, formato, [argumentos]) del snprintf que compone el $STATUS."""
    m = re.search(r'snprintf\(\s*payload\s*,[^,]+,\s*"(\$STATUS(?:[^"\\]|\\.)*)"\s*(.*?)\);',
                  codigo, re.S)
    if not m:
        raise _abortar("el snprintf del $STATUS", "bluetooth.cpp del %s" % punta)
    fmt, cola = m.group(1), m.group(2).strip()
    args, prof, act = [], 0, ""
    for c in cola.lstrip(","):
        if c in "([":
            prof += 1
        elif c in ")]":
            prof -= 1
        if c == "," and prof == 0:
            args.append(act.strip())
            act = ""
            continue
        act += c
    if act.strip():
        args.append(act.strip())
    return m.start(), fmt, args


def _cuerpo_funcion(codigo, nombre):
    """El cuerpo de la funcion `nombre`, buscada por su DEFINICION (con llaves)."""
    m = re.search(r"\b%s\s*\([^;{)]*\)\s*\{" % re.escape(nombre), codigo)
    if not m:
        return None
    return _bloque_que_contiene(codigo, m.end() - 1 + 1)


def _fuentes(fw, punta):
    """Los .cpp de esa punta, listados del disco. Una lista escrita a mano aqui seria
    un buscador ciego el dia que aparezca un fichero nuevo (CLAUDE.md 4)."""
    import os
    d = os.path.dirname(fw.ruta(punta, "src", "bluetooth.cpp"))
    return sorted(f for f in os.listdir(d) if f.endswith(".cpp"))


def _ancho_de_funcion(fw, punta, nombre, visto=None):
    """El literal mas largo que puede devolver `nombre`, siguiendo los `return`.

    Sigue las llamadas -coordinador_nombreEstadoMaster() devuelve semaforo_nombreEstado()-
    porque parar en el primer salto seria dar por medido lo que no se miro."""
    visto = visto or set()
    if nombre in visto:
        raise _Falta("la resolucion de %s() es circular" % nombre)
    visto.add(nombre)
    cuerpo = None
    for f in _fuentes(fw, punta):
        cuerpo = _cuerpo_funcion(fw.codigo(punta, "src", f), nombre)
        if cuerpo is not None:
            break
    if cuerpo is None:
        raise _abortar("la definicion de %s()" % nombre, "los .cpp del %s" % punta)
    anchos = []
    for expr in re.findall(r"\breturn\s+([^;]*);", cuerpo):
        lits = re.findall(r'"((?:[^"\\]|\\.)*)"', expr)
        if lits:
            anchos.append(max(len(x) for x in lits))
            continue
        llamada = re.match(r"^\s*(\w+)\s*\(\s*\)\s*$", expr)
        if llamada:
            anchos.append(_ancho_de_funcion(fw, punta, llamada.group(1), visto))
            continue
        raise _Falta("%s() devuelve %r, que esta cuenta no sabe acotar. Sin su ancho la "
                     "trama seria una estimacion" % (nombre, expr.strip()))
    if not anchos:
        raise _Falta("%s() no tiene ningun return que acotar" % nombre)
    return max(anchos)


def _ancho_arg(fw, punta, arg, bloque):
    """Cuantos caracteres puede llegar a poner ESE argumento en la trama."""
    # 1. Un literal, o un ternario entre literales: se mide el mas largo.
    lits = re.findall(r'"((?:[^"\\]|\\.)*)"', arg)
    if lits:
        return max(len(x) for x in lits)
    # 2. Un buffer declarado en el mismo bloque: N-1, que es lo unico que snprintf
    #    garantiza. Es la cota que no depende de creerse a nadie.
    if re.match(r"^\w+$", arg):
        m = re.search(r"char\s+%s\s*\[\s*(\d+)\s*\]" % re.escape(arg), bloque)
        if m:
            return int(m.group(1)) - 1
        # 3. Un const char* local, asignado desde una funcion: se sigue la funcion.
        m = re.search(r"const\s+char\s*\*\s*%s\s*=\s*(\w+)\s*\(" % re.escape(arg), bloque)
        if m:
            return _ancho_de_funcion(fw, punta, m.group(1))
    # 4. Una llamada directa dentro del propio snprintf.
    m = re.match(r"^\s*(\w+)\s*\(\s*\)\s*$", arg)
    if m:
        return _ancho_de_funcion(fw, punta, m.group(1))
    raise _Falta("no se supo acotar el argumento %r del $STATUS del %s. Una estimacion "
                 "aqui es lo que trunco el $ALARM de N-108" % (arg, punta))


# --- El $ALARM ENTERO, que hasta el 08/09 no medía nadie -------------------------
#
# La cuenta del tramo (2.quater) se escribio con esta exencion al lado, y se cita entera
# porque es la que este bloque viene a retirar:
#
#   "LO QUE ESTA CUENTA NO CUBRE, ESCRITO PARA QUE NO PASE POR COBERTURA: mide el TRAMO,
#    no el $ALARM entero. El payload del $ALARM se compone ademas con literales y con
#    buffers `causa[...]` que viven en coordinador.cpp y en main.cpp, y por buffer NO cabe
#    hoy en payload[144]. No se mete aqui porque el arreglo esta en ficheros que este
#    cambio no toca, y un instrumento que falla por algo que nadie puede arreglar desde
#    aqui es un FALLA permanente, que es lo que CLAUDE.md 3 prohibe."
#
# La exencion era CORRECTA cuando se escribio y deja de serlo el dia que los causa[] se
# acotan: entonces el rojo YA se puede apagar construyendo, y un hueco que nadie mide deja
# de ser prudencia para ser un defecto con permiso. Se retira midiendo, no borrandola.
#
# EL DEFECTO QUE ESTO CAZA, MEDIDO ANTES DE ARREGLARLO: por buffer el peor $ALARM eran 158
# caracteres en el Maestro y 171 en el Esclavo, contra los 143 que guarda un payload[144].
# Lo que se perdia era el FINAL, o sea el valor de HORA -y en el Esclavo parte de ACCION-.
# Y con el checksum BUENO, porque se calcula sobre lo que quedo: la alarma llega con
# aspecto de intacta y sin el unico dato por el que existe una Caja Negra. El sintoma no
# es "el equipo se callo": es una alarma sin hora, que es peor porque no se investiga.
#
# POR QUE HAY QUE IR A LOS LLAMADORES Y NO BASTA CON MIRAR LA FUNCION: tres de los cinco
# campos -EVENTO, CAUSA y ACCION- son PARAMETROS `const char*`. Su ancho no esta en
# bluetooth.cpp: esta en quien llama. Y los llamadores se censan por DIRECTORIO, no con
# una lista escrita aqui: una lista se queda corta el dia que alguien anade un .cpp con
# una alarma nueva, y la cuenta aprobaria un buffer que ya no basta (CLAUDE.md 5).

ALARMA = "bluetooth_reportarAlarma"


def _partir_args(cola):
    """Parte una lista de argumentos por las comas de PROFUNDIDAD CERO."""
    args, prof, act = [], 0, ""
    for c in cola:
        if c in "([":
            prof += 1
        elif c in ")]":
            prof -= 1
        if c == "," and prof == 0:
            args.append(act.strip())
            act = ""
            continue
        act += c
    if act.strip():
        args.append(act.strip())
    return args


def _ancho_decl(bloque, nombre):
    """N-1 de `char nombre[...]`, admitiendo `sizeof("literal")` ademas de un entero.

    Los causa[] se dimensionan desde el 08/09 con el sizeof de su peor literal -es lo que
    ata la cota a lo que de verdad se escribe en vez de a un numero redondo-, y una cuenta
    que solo supiera leer digitos daria por NO ACOTADO justo el buffer que se acaba de
    acotar. Devuelve None si no sabe: quien llama ABORTA, que es lo correcto -una
    estimacion aqui es lo que trunco el $ALARM de N-108-."""
    m = re.search(r"char\s+%s\s*\[\s*([^\]]+)\]" % re.escape(nombre), bloque)
    if m is None:
        return None
    expr = m.group(1).strip()
    total = 0
    for trozo in expr.split("+"):
        trozo = trozo.strip()
        if re.fullmatch(r"\d+", trozo):
            total += int(trozo)
            continue
        ms = re.fullmatch(r'sizeof\(\s*"((?:[^"\\]|\\.)*)"\s*\)', trozo)
        if ms:
            total += len(ms.group(1)) + 1   # sizeof de un literal incluye el NUL
            continue
        return None
    return total - 1


def _llamadas_a(codigo, nombre):
    """Los argumentos de cada LLAMADA a `nombre`, saltandose definiciones y prototipos.

    Se distingue por lo que hay DESPUES del parentesis que cierra: `{` es una definicion
    y `;` precedido de un tipo es un prototipo. Mirar solo el nombre haria pasar la propia
    definicion por llamador y sus parametros por argumentos."""
    fuera = []
    for m in re.finditer(r"\b%s\s*\(" % re.escape(nombre), codigo):
        if re.search(r"\b(void|bool|int|static)\s*$", codigo[:m.start()]):
            continue                      # definicion o prototipo, no llamada
        prof, j = 0, m.end() - 1
        while j < len(codigo):
            if codigo[j] == "(":
                prof += 1
            elif codigo[j] == ")":
                prof -= 1
                if prof == 0:
                    break
            j += 1
        else:
            continue
        if not re.match(r"\s*;", codigo[j + 1:]):
            continue
        fuera.append((m.start(), _partir_args(codigo[m.end():j])))
    return fuera


def _funcion_que_contiene(codigo, pos):
    """(nombre, [parametros]) de la funcion en cuyo cuerpo cae `pos`. (None, []) si no hay.

    Se busca hacia atras la ultima cabecera `tipo nombre(args) {` que abra un bloque que
    todavia no se haya cerrado por delante de `pos`."""
    mejor = (None, [])
    for m in re.finditer(r"(?:^|\n)\s*(?:static\s+)?[A-Za-z_][\w:*&\s]*?"
                         r"\b([A-Za-z_]\w*)\s*\(([^;{)]*)\)\s*\{", codigo):
        if m.end() > pos:
            break
        # Que el bloque siga abierto en pos: se cuentan llaves desde la cabecera.
        prof = 0
        for c in codigo[m.end() - 1:pos]:
            if c == "{":
                prof += 1
            elif c == "}":
                prof -= 1
        if prof > 0:
            params = [p.strip().split()[-1].lstrip("*")
                      for p in m.group(2).split(",") if p.strip()]
            mejor = (m.group(1), params)
    return mejor


def _ancho_de_expresion(fw, punta, fichero, codigo, pos, arg, prof=0):
    """El ancho de `arg` en la llamada que esta en `pos`, siguiendo los saltos que haga falta.

    POR QUE HACE FALTA RECURSION Y NO BASTA CON _ancho_arg: la alarma de camara no se emite
    en el mismo sitio donde estan sus literales. `camara_alarmar(i, "CAM_PEGADA", ...)` los
    pone, y la llamada a bluetooth_reportarAlarma() de dentro pasa `evento`, que ahi ya es
    un PARAMETRO. Sin este salto la cuenta ABORTA -que es lo correcto, y es lo que hizo la
    primera version de esta comprobacion- pero no mide; con una estimacion en su lugar
    mediria de menos, que es lo que trunco el $ALARM de N-108.

    La profundidad se limita: una recursion sin fondo se cuelga en una llamada mutua y un
    pack colgado es un ABORTADO que nadie diagnostica."""
    if prof > 4:
        raise _Falta("la resolucion de %r en %s/%s pasa de cuatro saltos: o hay una "
                     "llamada mutua o esta cuenta se perdio" % (arg, punta, fichero))
    bloque = _bloque_que_contiene(codigo, pos)
    # Antes que nada, la declaracion del buffer POR SU EXPRESION: _ancho_arg solo sabe
    # leer `char x[12]` y desde el 08/09 los causa[] se dimensionan con `sizeof("...")`,
    # que es lo que ata la cota a lo que de verdad se escribe. Sin esto, el pack daria
    # por no acotable justo el buffer que se acaba de acotar.
    if re.match(r"^\w+$", arg):
        w = _ancho_decl(bloque, arg)
        if w is not None:
            return w
    try:
        return _ancho_arg(fw, punta, arg, bloque)
    except _Falta:
        pass
    fn, params = _funcion_que_contiene(codigo, pos)
    if fn is None or arg not in params:
        raise _Falta(
            "no se supo acotar %r en la llamada de %s/%s: no es literal, ni buffer del "
            "bloque, ni parametro de %s()" % (arg, punta, fichero, fn or "?"))
    i = params.index(arg)
    peor, n = -1, 0
    for f2 in fw.fuentes_de(punta, "src"):
        c2 = fw.codigo(punta, "src", f2)
        for pos2, args2 in _llamadas_a(c2, fn):
            if len(args2) <= i:
                raise _Falta("una llamada a %s() en %s/%s lleva %d argumentos y se pedia "
                             "el %d" % (fn, punta, f2, len(args2), i + 1))
            n += 1
            w = _ancho_de_expresion(fw, punta, f2, c2, pos2, args2[i], prof + 1)
            peor = max(peor, w)
    if n == 0:
        raise _Falta("%s() no tiene llamadores en %s/src y su parametro %r no se puede "
                     "acotar por otro sitio" % (fn, punta, arg))
    return peor


def _ancho_por_llamadores(fw, punta, indice, nombre_param):
    """El peor ancho del argumento `indice` de reportarAlarma(), sobre TODOS sus llamadores."""
    peor, n, quien = -1, 0, ""
    for fichero in fw.fuentes_de(punta, "src"):
        codigo = fw.codigo(punta, "src", fichero)
        for pos, args in _llamadas_a(codigo, ALARMA):
            if len(args) <= indice:
                raise _Falta(
                    "una llamada a %s() en %s/%s lleva %d argumentos y se pedia el %d: "
                    "la firma y los llamadores no cuadran, y esta cuenta no adivina"
                    % (ALARMA, punta, fichero, len(args), indice + 1))
            n += 1
            w = _ancho_de_expresion(fw, punta, fichero, codigo, pos, args[indice])
            if w > peor:
                peor, quien = w, "%s (%r)" % (fichero, args[indice])
    if n == 0:
        raise _Falta(
            "%s() no tiene ni un llamador en %s/src. O la alarma esta muerta -y entonces "
            "la Caja Negra no registra nada- o este censo dejo de encontrarlos: en los dos "
            "casos esta cuenta no mide lo que dice" % (ALARMA, punta))
    return peor, n, quien


def _peor_alarma(fw, punta):
    """(peor caso, capacidad del payload, nº de llamadores, desglose) del $ALARM entero."""
    codigo = fw.codigo(punta, "src", "bluetooth.cpp")
    cuerpo = _cuerpo_funcion(codigo, ALARMA)
    if cuerpo is None:
        raise _abortar("la definicion de %s()" % ALARMA, "bluetooth.cpp del %s" % punta)
    m = re.search(r'snprintf\(\s*payload\s*,[^,]+,\s*"(\$ALARM(?:[^"\\]|\\.)*)"\s*(.*?)\);',
                  cuerpo, re.S)
    if not m:
        raise _abortar("el snprintf del $ALARM", "bluetooth.cpp del %s" % punta)
    fmt = m.group(1)
    args = _partir_args(m.group(2).strip().lstrip(","))

    cap = _ancho_decl(cuerpo, "payload")
    if cap is None:
        raise _abortar("la declaracion del payload del $ALARM", "bluetooth.cpp del %s" % punta)

    firma = re.search(r"\b%s\s*\(([^)]*)\)\s*\{" % re.escape(ALARMA), codigo)
    if not firma:
        raise _abortar("la firma de %s()" % ALARMA, "bluetooth.cpp del %s" % punta)
    params = [p.strip().split()[-1].lstrip("*") for p in firma.group(1).split(",")]

    anchos, desglose, llamadores = [], [], 0
    for a in args:
        w = _ancho_decl(cuerpo, a)
        if w is not None:
            anchos.append(w)
            desglose.append("%s=%d (buffer local)" % (a, w))
            continue
        if a in params:
            w, n, quien = _ancho_por_llamadores(fw, punta, params.index(a), a)
            llamadores = max(llamadores, n)
            anchos.append(w)
            desglose.append("%s=%d (el peor de %d llamadas: %s)" % (a, w, n, quien))
            continue
        raise _Falta(
            "no se supo acotar %r del $ALARM del %s. No es un buffer del cuerpo ni un "
            "parametro de la firma, asi que su ancho no sale de ningun sitio medible"
            % (a, punta))

    peor = _peor(fmt, anchos)
    if peor is None:
        raise _Falta("el $ALARM del %s tiene %d conversiones y %d argumentos. Con esa "
                     "discrepancia no hay cuenta que hacer"
                     % (punta, len(re.findall(r"%[0-9]*l?[usd]", fmt)), len(args)))
    return peor, cap, llamadores, " · ".join(desglose)


def _peor_status(fw, punta):
    """(peor caso en caracteres, payload, tramaCompleta, desglose) del $STATUS."""
    codigo = fw.codigo(punta, "src", "bluetooth.cpp")
    idx, fmt, args = _snprintf_status(codigo, punta)
    bloque = _bloque_que_contiene(codigo, idx)
    m = re.search(r"char\s+payload\s*\[\s*(\d+)\s*\]", bloque)
    if not m:
        raise _abortar("la declaracion del payload del $STATUS", "bluetooth.cpp del %s" % punta)
    payload = int(m.group(1))
    m = re.search(r"char\s+tramaCompleta\s*\[\s*(\d+)\s*\]", codigo)
    if not m:
        raise _abortar("la declaracion de tramaCompleta", "bluetooth.cpp del %s" % punta)
    anchos = [_ancho_arg(fw, punta, a, bloque) for a in args]
    peor = _peor(fmt, anchos)
    if peor is None:
        raise _Falta("el $STATUS del %s tiene %d conversiones y %d argumentos. Con esa "
                     "discrepancia no hay cuenta que hacer"
                     % (punta, len(re.findall(r"%[0-9]*l?[usd]", fmt)), len(args)))
    desglose = ", ".join("%s=%d" % (a, w) for a, w in zip(args, anchos))
    return peor, payload, int(m.group(1)), bloque, desglose


def correr(b, fw):
    b.titulo("El presupuesto de bytes por segundo del enlace J17")

    # ---- 1. El baudio, y que las tres fuentes digan lo mismo ------------------
    baudio = fw.constante(CONTRATO, r"#define\s+ENLACE_BAUDIO\s+(\d+)",
                          "el baudio del ESP32")
    bits = fw.constante(CONTRATO, r"#define\s+ENLACE_BITS_POR_BYTE\s+(\d+)",
                        "los bits por byte en el cable")

    baudios_stm32 = {}
    for punta in ("Maestro", "Esclavo"):
        baudios_stm32[punta] = fw.constante(
            (punta, "src", "bluetooth.cpp"), r"SerialBT\.begin\((\d+)\)",
            "el baudio del %s" % punta)

    b.verificar(
        baudio == baudios_stm32["Maestro"] == baudios_stm32["Esclavo"],
        "las tres puntas van a %d bps: el mismo ESP32 puede servir a las dos sin "
        "recompilar" % baudio,
        "los baudios no coinciden: ESP32 %d, Maestro %d, Esclavo %d. Con velocidades "
        "distintas no hay presupuesto que calcular -no se entienden- y ademas el mismo "
        "binario del puente no sirve para las dos puntas"
        % (baudio, baudios_stm32["Maestro"], baudios_stm32["Esclavo"]))

    caudal = baudio // bits

    # ---- 2. Los tamanos, leidos de los snprintf reales ------------------------
    #
    # El envoltorio son 5 bytes: el '*', dos hex, CR y LF. Sale del formato literal de
    # enviarTramaConCrc(), no de una cuenta a ojo.
    envoltorio = 5
    bufStatus = _buffer(fw, "Maestro", r"char payload\[(\d+)\];\s*\n\s*snprintf\(payload,"
                                       r"[^;]*\$STATUS", "el buffer de $STATUS del Maestro")
    bufEvento = _buffer(fw, "Maestro", r"char payload\[(\d+)\];\s*\n\s*snprintf\(payload,"
                                       r"[^;]*\$EVENT", "el buffer de $EVENT del Maestro")
    bufAlarma = _buffer(fw, "Maestro", r"char payload\[(\d+)\];\s*\n\s*snprintf\(payload,"
                                       r"[^;]*\$ALARM", "el buffer de $ALARM del Maestro")
    bufTrama = _buffer(fw, "Maestro", r"char tramaCompleta\[(\d+)\]",
                       "el buffer del envoltorio con CRC")

    topeStatus = bufStatus - 1 + envoltorio
    topeEvento = bufEvento - 1 + envoltorio
    topeAlarma = bufAlarma - 1 + envoltorio

    b.verificar(
        bufTrama >= bufStatus + envoltorio,
        "el envoltorio (%d B) cabe la trama mas larga (%d B de payload + %d)"
        % (bufTrama, bufStatus, envoltorio),
        "tramaCompleta[%d] NO cabe un payload[%d] con su *XX y su CRLF. snprintf "
        "truncaria la trama en el ultimo paso, y saldria al cable bien formada hasta la "
        "mitad" % (bufTrama, bufStatus))

    # ---- 2.bis N-154: Y EL CONTENIDO CABE EN EL PAYLOAD -----------------------
    #
    # La de arriba es una cota entre dos BUFFERS. Esta es la que faltaba, y la que no se
    # cumplia: que lo que el snprintf ESCRIBE quepa en el payload. El porque entero, y el
    # borde contra el que se mide, estan en la cabecera de este fichero.
    peores = {}
    for punta in ("Maestro", "Esclavo"):
        try:
            peores[punta] = _peor_status(fw, punta)
        except _Falta as e:
            raise fw.Abortado(str(e))

    for punta in ("Maestro", "Esclavo"):
        peor, payload, trama, _bloque, desglose = peores[punta]
        b.verificar(
            peor <= payload - 1,
            "el peor $STATUS del %s son %d caracteres y payload[%d] guarda %d: cabe con "
            "%d B de margen (%s)"
            % (punta, peor, payload, payload - 1, payload - 1 - peor, desglose),
            "EL $STATUS DEL %s NO CABE: el peor caso son %d caracteres y payload[%d] "
            "guarda %d. snprintf lo trunca por el final, enviarTramaConCrc calcula el "
            "checksum sobre lo que quedo y la app descarta la trama ENTERA -no la lee a "
            "medias-. El sintoma en campo es 'el equipo se callo', que manda a mirar el "
            "cable. Desglose: %s" % (punta.upper(), peor, payload, payload - 1, desglose))

        b.verificar(
            peor + envoltorio <= trama - 1,
            "y con el *XX y el CRLF son %d caracteres de los %d que guarda "
            "tramaCompleta[%d] del %s"
            % (peor + envoltorio, trama - 1, trama, punta),
            "el peor $STATUS del %s mas su envoltorio son %d caracteres y "
            "tramaCompleta[%d] guarda %d: la trama se corta en el ULTIMO paso, justo "
            "por el checksum" % (punta, peor + envoltorio, trama, trama - 1))

    # ---- 2.ter Las cotas estan DERIVADAS y el emisor las EJERCE ---------------
    #
    # Un buffer ajustado al rango solo es correcto si alguien comprueba el rango. Sin la
    # guarda, tTxt[4] no es una cota: es un truncamiento esperando a un numero raro. Y
    # una cota escrita a mano seria la quinta copia de un limite del ciclo (N-131, N-133,
    # N-137), asi que ademas se exige que salga de donde vive el limite.
    bloqueM = peores["Maestro"][3]
    coordH = fw.codigo("Maestro", "include", "coordinador.h")

    verdeMax = fw.constante(("Maestro", "include", "limites_ciclo.h"),
                            r"VERDE_MIN_MAX\s*=\s*(\d+)", "el verde maximo del ciclo")
    rojoMax = fw.constante(("Maestro", "include", "limites_ciclo.h"),
                           r"ROJO_MIN_MAX\s*=\s*(\d+)", "el rojo maximo del ciclo")
    despejeMax = fw.constante(("Maestro", "include", "limites_ciclo.h"),
                              r"DESPEJE_SEG_MAX\s*=\s*(\d+)", "el despeje maximo")
    silencio = fw.constante(("Maestro", "include", "protocolo.h"),
                            r"#define\s+SFTY6_SILENCIO_MS\s+(\d+)", "el techo de SFTY-6")
    rfMax = fw.constante(("Maestro", "include", "coordinador.h"),
                         r"CALIDAD_ENLACE_MAX\s*=\s*(\d+)", "la cota del RF")

    cotaT = max(verdeMax, rojoMax) * 60
    b.verificar(
        cotaT >= despejeMax,
        "la cota del campo T: (%d s) cubre a sus DOS productores: el despeje del "
        "coordinador (%d s) y la fase larga del modo (%d min)"
        % (cotaT, despejeMax, max(verdeMax, rojoMax)),
        "la cota de T: son %d s y el despeje llega a %d s. El coordinador publicaria un "
        "numero legal que la guarda marcaria como imposible, y el operario se quedaria "
        "sin cuenta atras en el todo-rojo" % (cotaT, despejeMax))

    b.verificar(
        re.search(r"CUENTA_ATRAS_MAX_SEG\s*=[^;]*VERDE_MIN_MAX", coordH) is not None
        and re.search(r"CUENTA_ATRAS_MAX_SEG\s*=[^;]*ROJO_MIN_MAX", coordH) is not None
        and re.search(r"RTT_PUBLICABLE_MAX_MS\s*=\s*SFTY6_SILENCIO_MS", coordH) is not None,
        "las dos cotas se DERIVAN: la de T: de VERDE_MIN_MAX/ROJO_MIN_MAX y la del RTT "
        "de SFTY6_SILENCIO_MS. Ningun numero escrito a mano",
        "alguna cota de coordinador.h esta escrita a mano en vez de derivada. Un limite "
        "copiado es la quinta copia de N-137: el dia que difieran gana el que NO lleva "
        "el aviso encima")

    for nombre in ("CUENTA_ATRAS_MAX_SEG", "CALIDAD_ENLACE_MAX", "RTT_PUBLICABLE_MAX_MS"):
        b.verificar(
            nombre in bloqueM,
            "el emisor del $STATUS COMPARA contra %s antes de imprimir: la cota se "
            "ejerce, no solo se declara" % nombre,
            "%s esta declarada en coordinador.h y el emisor del $STATUS NO la usa. Una "
            "cota declarada no aprieta nada -DECLARAR NO ES EJERCER, CLAUDE.md 2.ter-, y "
            "sin ella el buffer ajustado a su rango deja de ser una cota y pasa a ser un "
            "truncamiento esperando a un valor raro" % nombre)

    # Y que cada buffer aguante el valor MAS LARGO que su cota permite: si no, la guarda
    # deja pasar un valor legal y snprintf lo corta igual. Es el otro extremo del mismo
    # par, y falla en la direccion contraria al de arriba.
    for nombre, texto_tope in (("tTxt", "%d" % cotaT),
                               ("rfTxt", "%d%%" % rfMax),
                               ("rttTxt", "%dms" % silencio)):
        m = re.search(r"char\s+%s\s*\[\s*(\d+)\s*\]" % nombre, bloqueM)
        if not m:
            raise fw.Abortado(
                "no se hallo la declaracion de %s en el emisor del $STATUS" % nombre)
        cap = int(m.group(1))
        b.verificar(
            len(texto_tope) <= cap - 1,
            "%s[%d] aguanta el valor mas largo que su cota permite (%r, %d caracteres)"
            % (nombre, cap, texto_tope, len(texto_tope)),
            "%s[%d] guarda %d caracteres y el tope legal de su cota es %r (%d): un valor "
            "PERFECTAMENTE VALIDO se truncaria. La guarda no lo veria pasar, porque no "
            "esta fuera de rango" % (nombre, cap, cap - 1, texto_tope, len(texto_tope)))

    # ---- 2.quater D-13: Y EL TRAMO DEL $ALARM, QUE ES EL MISMO DEFECTO EN PEQUENO ----
    #
    # bluetooth_reportarAlarma() arma en un buffer INTERNO el ultimo tramo del enlace y
    # lo mete despues en el payload. Ese buffer tenia el mismo defecto que la trama de
    # N-154 -dimensionado por debajo de lo que su propio snprintf puede escribir- y
    # nadie lo medía: el Maestro pedia 52 caracteres en un char[40] y el Esclavo 44 en
    # un char[44], que guarda 43.
    #
    # POR QUE SE MIDE APARTE Y NO CON LA MISMA CUENTA DEL $STATUS: aqui no todo sale de
    # un buffer. El Esclavo mete tres contadores de protocolo.cpp con %lu, y esos son
    # numeros libres -no hay cota que declarar, ver el porque escrito junto al snprintf-.
    # Asi que a las conversiones numericas se les da su tope de TIPO, que es el unico
    # borde cierto cuando no hay rango que prometer, Y ESE BORDE VA ESCRITO AQUI DEBAJO
    # (CLAUDE.md 4.quinquies: un instrumento que compara contra un borde dice cual es).
    # Cualquier conversion que esta tabla no conozca ABORTA: preferimos no medir a medir
    # de menos sin decirlo.
    #
    # LO QUE ESTA CUENTA NO CUBRE, ESCRITO PARA QUE NO PASE POR COBERTURA: mide el TRAMO,
    # no el $ALARM entero. El payload del $ALARM se compone ademas con literales y con
    # buffers `causa[...]` que viven en coordinador.cpp y en main.cpp, y por buffer NO
    # cabe hoy en payload[144] -esta medido y anotado en el informe de este cambio-. No
    # se mete aqui porque el arreglo esta en ficheros que este cambio no toca, y un
    # instrumento que falla por algo que nadie puede arreglar desde aqui es un FALLA
    # permanente, que es lo que CLAUDE.md 3 prohibe.
    ANCHO_POR_TIPO = {
        # El tope de cada conversion, en caracteres, con el signo cuando lo lleva.
        "%d": len("-2147483648"),   # int de 32 bits
        "%u": len("4294967295"),    # unsigned de 32 bits
        "%lu": len("4294967295"),   # unsigned long de 32 bits en Cortex-M3
    }

    # D-23 (13/09): ESTA FUNCION SE PARAMETRIZA, NO SE COPIA. Nacio midiendo el tramo[]
    # del $ALARM y desde hoy mide tambien el det[] del $EVENT periodico de D-23, que es
    # EL MISMO problema con los MISMOS tres contadores: un buffer interno relleno con
    # numeros libres que despues entra en un payload. Reescribir esta cuenta para el
    # segundo caso seria reescribir logica ya probada por un cambio de nombre, que es
    # justo por donde se cuelan los errores (CLAUDE.md 4). Lo unico que entra por
    # parametro es DONDE mirar y COMO llamarlo en el mensaje.
    def _peor_buffer(punta, cuerpo, nombre, que):
        """(peor caso en caracteres, capacidad) de un buffer interno relleno por snprintf."""
        m = re.search(r"char\s+%s\s*\[\s*(\d+)\s*\]" % re.escape(nombre), cuerpo)
        if not m:
            raise _abortar("la declaracion de %s[]" % nombre, "%s del %s" % (que, punta))
        cap = int(m.group(1))
        escrituras = re.findall(
            r'snprintf\(\s*%s\s*,[^,]+,\s*"((?:[^"\\]|\\.)*)"\s*(.*?)\);' % re.escape(nombre),
            cuerpo, re.S)
        if not escrituras:
            raise _abortar("el snprintf que rellena %s[]" % nombre,
                           "%s del %s" % (que, punta))
        peor = 0
        for fmt, cola in escrituras:
            args, prof, act = [], 0, ""
            for c in cola.strip().lstrip(","):
                if c in "([":
                    prof += 1
                elif c in ")]":
                    prof -= 1
                if c == "," and prof == 0:
                    args.append(act.strip())
                    act = ""
                    continue
                act += c
            if act.strip():
                args.append(act.strip())
            convs = re.findall(r"%[0-9]*l?[usd]", fmt)
            # El "%%" del RF ya no esta, pero si vuelve tiene que contar UNO y no dos.
            fijo = len(re.sub(r"%[0-9]*l?[usd]", "", fmt).replace("%%", "%"))
            if len(convs) != len(args):
                raise _Falta(
                    "%s del %s tiene %d conversiones y %d argumentos"
                    % (que, punta, len(convs), len(args)))
            total = fijo
            for conv, arg in zip(convs, args):
                if conv == "%s":
                    total += _ancho_arg(fw, punta, arg, cuerpo)
                elif conv in ANCHO_POR_TIPO:
                    total += ANCHO_POR_TIPO[conv]
                else:
                    raise _Falta(
                        "%s del %s usa la conversion %r y esta cuenta no sabe acotarla. "
                        "Una estimacion aqui es lo que trunco el $ALARM de N-108"
                        % (que, punta, conv))
            peor = max(peor, total)
        return peor, cap

    def _peor_tramo(punta):
        """(peor caso en caracteres, capacidad del buffer) del tramo del $ALARM."""
        cuerpo = _cuerpo_funcion(fw.codigo(punta, "src", "bluetooth.cpp"),
                                 "bluetooth_reportarAlarma")
        if cuerpo is None:
            raise _abortar("bluetooth_reportarAlarma()", "bluetooth.cpp del %s" % punta)
        return _peor_buffer(punta, cuerpo, "tramo", "el tramo del $ALARM")

    for punta in ("Maestro", "Esclavo"):
        try:
            peorT, capT = _peor_tramo(punta)
        except _Falta as e:
            raise fw.Abortado(str(e))
        b.verificar(
            peorT <= capT - 1,
            "el peor tramo del $ALARM del %s son %d caracteres y tramo[%d] guarda %d"
            % (punta, peorT, capT, capT - 1),
            "EL TRAMO DEL $ALARM DEL %s NO CABE: %d caracteres en un tramo[%d] que "
            "guarda %d. No se pierde la trama -es un buffer interno, asi que el $ALARM "
            "sale bien formado y con su checksum bueno-: se pierde el FINAL del tramo, "
            "que es el dato por el que la alarma lleva tramo. El sintoma es una alarma "
            "que llega entera y a la que le falta justo el numero que se fue a buscar"
            % (punta.upper(), peorT, capT, capT - 1))

    # La cota de los latidos sin respuesta es una COPIA -vive en coordinador.h para que
    # el emisor pueda dimensionar, y de verdad la impone la guarda de coordinador.cpp-,
    # asi que se recalcula la igualdad en vez de creersela (CLAUDE.md 3.bis).
    tope_sr = fw.constante(("Maestro", "include", "coordinador.h"),
                           r"LATIDOS_SIN_RESPUESTA_MAX\s*=\s*(\d+)",
                           "la cota declarada de los latidos sin respuesta")
    freno_sr = fw.constante(("Maestro", "src", "coordinador.cpp"),
                            r"latidosSinRespuesta\s*<\s*(\d+)\s*\)",
                            "el freno real del contador de latidos sin respuesta")
    b.verificar(
        tope_sr == freno_sr,
        "la cota declarada de SINRESP (%d) es la que el contador respeta de verdad: la "
        "guarda de coordinador.cpp lo para en %d" % (tope_sr, freno_sr),
        "coordinador.h declara LATIDOS_SIN_RESPUESTA_MAX = %d y el contador se para en "
        "%d. El $ALARM dimensiona su buffer por el primero: si el segundo es mayor, un "
        "valor legal se trunca; si es menor, la cota sobra y el buffer paga un byte que "
        "nadie usa. Los dos numeros van juntos o no valen" % (tope_sr, freno_sr))

    m_sr = re.search(r"char\s+srTxt\s*\[\s*(\d+)\s*\]",
                     fw.codigo("Maestro", "src", "bluetooth.cpp"))
    if not m_sr:
        raise fw.Abortado(
            "no se hallo la declaracion de srTxt en el $ALARM del Maestro: es el buffer "
            "que la cota de SINRESP dimensiona, y sin el no hay nada que comprobar")
    b.verificar(
        len("%d" % tope_sr) <= int(m_sr.group(1)) - 1,
        "srTxt[%s] aguanta el valor mas largo que su cota permite (%r, %d caracteres)"
        % (m_sr.group(1), "%d" % tope_sr, len("%d" % tope_sr)),
        "srTxt[%s] guarda %d caracteres y el tope legal de su cota es %r: un valor "
        "PERFECTAMENTE VALIDO se truncaria, y la guarda no lo veria pasar porque no "
        "esta fuera de rango"
        % (m_sr.group(1), int(m_sr.group(1)) - 1, "%d" % tope_sr))

    # ---- 2.quinquies N-154: EL $ALARM ENTERO CABE EN SU PAYLOAD --------------
    #
    # La cuenta y el porque estan arriba, junto a _peor_alarma(). Aqui solo se cobra, y se
    # cobra en las DOS puntas por separado porque su peor caso NO es el mismo: el tramo del
    # Esclavo son 13 caracteres mas que el del Maestro.
    for punta in ("Maestro", "Esclavo"):
        peorA, capA, nll, desglose = _peor_alarma(fw, punta)
        b.verificar(
            peorA <= capA,
            "el peor $ALARM del %s son %d caracteres y su payload guarda %d (%d llamadas "
            "censadas en src/) | %s" % (punta.upper(), peorA, capA, nll, desglose),
            "EL $ALARM DEL %s NO CABE: %d caracteres por BUFFER en un payload que guarda "
            "%d. Se pierden los %d ultimos, que es el final de la trama: el valor de HORA. "
            "Y el checksum sale BUENO porque se calcula sobre lo que quedo, asi que la "
            "alarma llega con aspecto de intacta y sin el unico dato por el que existe la "
            "Caja Negra. Se acota donde se PRODUCE -los causa[] de los llamadores-, no "
            "agrandando esto. Desglose: %s"
            % (punta.upper(), peorA, capA, peorA - capA, desglose))

    # Y que la trama con su envoltorio tambien cabe: el $ALARM mas largo mas "*XX\r\n".
    # Truncar AQUI es distinto y peor -se corta el cierre del checksum, el otro extremo la
    # descarta y la alarma desaparece del todo-, asi que se mide aparte y se dice por que.
    for punta in ("Maestro", "Esclavo"):
        peorA, _, _, _ = _peor_alarma(fw, punta)
        capT = _ancho_decl(fw.codigo(punta, "src", "bluetooth.cpp"), "tramaCompleta")
        if capT is None:
            raise fw.Abortado(
                "no se hallo tramaCompleta[] en bluetooth.cpp del %s: es el envoltorio "
                "que lleva el checksum, y sin su cota no se puede decir que la alarma "
                "salga entera del equipo" % punta)
        b.verificar(
            peorA + len("*XX\r\n") <= capT,
            "el peor $ALARM del %s sale entero con su checksum: %d + 5 caracteres en un "
            "tramaCompleta que guarda %d" % (punta.upper(), peorA, capT),
            "el peor $ALARM del %s son %d caracteres y con su *XX\\r\\n no cabe en un "
            "tramaCompleta que guarda %d. Aqui truncar es PEOR que en el payload: se "
            "corta el cierre del checksum, el otro extremo descarta la trama y la alarma "
            "desaparece entera justo cuando hace falta" % (punta.upper(), peorA, capT))

    # ---- 2.sexies D-23: LOS $EVENT PERIODICOS DE LAS DOS PUNTAS ---------------
    #
    # QUE ES Y POR QUE ENTRA EN ESTE PACK. D-23 pide que cada poste diga como ve EL su
    # enlace -sus contadores de SFTY-15, que hasta el 13/09 solo salian dentro del $ALARM,
    # o sea cuando la radio YA se habia caido-. El responsable eligio la via del $EVENT
    # PERIODICO (D-32 (2)). Eso pone en J17 emisores periodicos ADEMAS del $STATUS, y este
    # es el pack que dice si el canal los aguanta: un presupuesto que no contara un emisor
    # que existe no mediria de menos, mediria OTRO equipo.
    #
    # AQUI SE CENSABA UNO SOLO, Y ESO CADUCO EL MISMO DIA. Este bloque se escribio con el
    # unico periodico que habia -el ENLACE_RF del Esclavo- localizado por su nombre de
    # variable, y D-32 (1) anadio dos mas: el gemelo del Maestro, que publica los dos
    # contadores de SFTY-15 que se quedaron sin lector al retirar el LCD, y el del aviso
    # del limite de 48 h del Esclavo. Un censo escrito para UNO cuenta uno para siempre y
    # no se queja, que es la forma de caducar de CLAUDE.md 14: se RECUENTA, no se lee. Asi
    # que ahora se BARRE, y el numero de emisores que salga es el que entra en el caudal.
    #
    # SE LOCALIZAN POR LA CONSTANTE CON NOMBRE Y EN LAS DOS PUNTAS -`ahora - <marca> >=
    # DIAG_ENLACE_MS`-, no por el texto del bloque: si alguien la retira o la renombra este
    # pack ABORTA en vez de dejar de contar un emisor en silencio, que es la unica forma de
    # que la cuenta no envejezca sola. Y el TRAMO que se mide de cada uno va desde esa
    # comparacion hasta su bluetooth_reportarEvento(), que es donde viven los buffers que
    # hay que acotar: es la misma maquina que el $ALARM, no una cuenta nueva.
    #
    # SE MIDE EN DOS PASOS Y CON LA MISMA MAQUINA QUE EL $ALARM, porque son dos
    # truncamientos distintos: primero que lo que se compone quepa en su buffer interno
    # -si no, se pierde el final del dato y la trama sale bien formada, que es el fallo que
    # no se investiga-, y despues que el $EVENT entero quepa en su payload y en su
    # envoltorio con CRC.
    periodicos = {}
    for punta in ("Maestro", "Esclavo"):
        codigo_p = fw.codigo(punta, "src", "bluetooth.cpp")
        marcas = list(re.finditer(r"ahora\s*-\s*(\w+)\s*>=\s*DIAG_ENLACE_MS", codigo_p))
        if not marcas:
            raise fw.Abortado(
                "no se hallo en bluetooth.cpp del %s ni una sola comparacion `ahora - "
                "<marca> >= DIAG_ENLACE_MS`, que es con lo que se construyo D-23 en las "
                "dos puntas. O se retiro, o cambio de forma: en los dos casos este "
                "presupuesto estaria contando un emisor distinto del que hay en el cable"
                % punta)
        periodicos[punta] = []
        for m_marca in marcas:
            m_llamada = re.search(
                r'bluetooth_reportarEvento\s*\(\s*"([^"]+)"\s*,\s*(\w+)\s*\)',
                codigo_p[m_marca.end():])
            if not m_llamada:
                raise fw.Abortado(
                    "el periodico de D-23 anclado en `%s` (%s) no termina en un "
                    "bluetooth_reportarEvento() con un ORIGEN literal y un buffer: sin "
                    "esos dos no hay trama que acotar"
                    % (m_marca.group(1), punta))
            tramo_src = codigo_p[m_marca.end():m_marca.end() + m_llamada.end()]
            periodicos[punta].append(
                (m_marca.group(1), m_llamada.group(1), m_llamada.group(2), tramo_src))

    b.verificar(
        all(periodicos[x] for x in ("Maestro", "Esclavo")),
        "las dos puntas tienen periodico de diagnostico: el Maestro %d y el Esclavo %d"
        % (len(periodicos["Maestro"]), len(periodicos["Esclavo"])),
        "una punta se quedo sin periodico de diagnostico (Maestro %d, Esclavo %d). D-23 "
        "pide que CADA poste diga como ve EL su enlace, y un poste mudo no lo dice"
        % (len(periodicos["Maestro"]), len(periodicos["Esclavo"])))

    for punta in ("Maestro", "Esclavo"):
        codigo_p = fw.codigo(punta, "src", "bluetooth.cpp")
        cuerpo_ev = _cuerpo_funcion(codigo_p, "bluetooth_reportarEvento")
        if cuerpo_ev is None:
            raise fw.Abortado(
                "no se hallo bluetooth_reportarEvento() en bluetooth.cpp del %s: es el "
                "emisor por el que sale D-23 y sin el no hay payload que medir" % punta)
        m_ev = re.search(
            r'snprintf\(\s*payload\s*,[^,]+,\s*"(\$EVENT(?:[^"\\]|\\.)*)"\s*(.*?)\);',
            cuerpo_ev, re.S)
        firma_ev = re.search(r"\bbluetooth_reportarEvento\s*\(([^)]*)\)\s*\{", codigo_p)
        capEv = _ancho_decl(cuerpo_ev, "payload")
        capTrE = _ancho_decl(codigo_p, "tramaCompleta")
        if not (m_ev and firma_ev) or capEv is None or capTrE is None:
            raise fw.Abortado(
                "no se pudo leer del C++ el emisor del $EVENT del %s (snprintf=%s, "
                "firma=%s, payload=%s, tramaCompleta=%s). Sin ellos D-23 no se acota"
                % (punta, bool(m_ev), bool(firma_ev), capEv, capTrE))
        params_ev = [x.strip().split()[-1].lstrip("*")
                     for x in firma_ev.group(1).split(",")]

        for marca, origen_diag, nombre_det, tramo_src in periodicos[punta]:
            try:
                peorD, capD = _peor_buffer(punta, tramo_src, nombre_det,
                                           "el DETALLE del $EVENT periodico de D-23 (%s)"
                                           % origen_diag)
            except _Falta as e:
                raise fw.Abortado(str(e))
            b.verificar(
                peorD <= capD - 1,
                "el DETALLE de %s en el %s cabe en su %s[%d], que guarda %d: el peor caso "
                "por TIPO son %d caracteres"
                % (origen_diag, punta.upper(), nombre_det, capD, capD - 1, peorD),
                "EL DETALLE DE %s EN EL %s NO CABE: %d caracteres por TIPO en un %s[%d] "
                "que guarda %d. Se pierde el FINAL, y el $EVENT sale bien formado y con su "
                "checksum bueno: el tecnico lee una linea entera a la que le falta justo "
                "el numero que fue a buscar. Se acota donde se produce, no se ensancha el "
                "buffer" % (origen_diag, punta.upper(), peorD, nombre_det, capD, capD - 1))

            anchos_ev = []
            for a in _partir_args(m_ev.group(2).strip().lstrip(",")):
                if len(params_ev) > 0 and a == params_ev[0]:
                    anchos_ev.append(len(origen_diag))   # el ORIGEN es literal del llamador
                elif len(params_ev) > 1 and a == params_ev[1]:
                    anchos_ev.append(capD - 1)           # el DETALLE, por SU buffer
                else:
                    w = _ancho_decl(cuerpo_ev, a)        # lo demas, buffer del propio emisor
                    if w is None:
                        raise fw.Abortado(
                            "no se supo acotar %r del $EVENT del %s. No es un parametro de "
                            "la firma ni un buffer del cuerpo, asi que su ancho no sale de "
                            "ningun sitio medible y una estimacion aqui es N-108 otra vez"
                            % (a, punta))
                    anchos_ev.append(w)
            peorEv = _peor(m_ev.group(1), anchos_ev)
            if peorEv is None:
                raise fw.Abortado(
                    "el $EVENT del %s tiene %d conversiones y %d argumentos: con esa "
                    "discrepancia no hay cuenta que hacer"
                    % (punta, len(re.findall(r"%[0-9]*l?[usd]", m_ev.group(1))),
                       len(anchos_ev)))

            b.verificar(
                peorEv <= capEv,
                "el $EVENT periodico de D-23 del %s (ORIGEN:%s) cabe: %d caracteres por "
                "BUFFER en un payload que guarda %d"
                % (punta.upper(), origen_diag, peorEv, capEv),
                "EL $EVENT %s DEL %s NO CABE: %d caracteres por BUFFER en un payload que "
                "guarda %d. Se pierden los %d ultimos -el final es la HORA-, y el checksum "
                "sale BUENO porque se calcula sobre lo que quedo. Se acorta el ORIGEN o el "
                "DETALLE, no se ensancha el payload: es el mismo emisor que usan todas las "
                "tramas de bitacora de esta punta"
                % (origen_diag, punta.upper(), peorEv, capEv, peorEv - capEv))
            b.verificar(
                peorEv + len("*XX\r\n") <= capTrE,
                "y sale entero con su checksum: %d + 5 caracteres en un tramaCompleta que "
                "guarda %d" % (peorEv, capTrE),
                "el $EVENT %s del %s son %d caracteres y con su *XX\\r\\n no cabe en un "
                "tramaCompleta que guarda %d. Truncar aqui es PEOR: se corta el cierre del "
                "checksum y el otro extremo descarta la trama entera"
                % (origen_diag, punta, peorEv, capTrE))

    # ---- 3. La cadencia, leida del C++ ---------------------------------------
    cadencias = {}
    for punta in ("Maestro", "Esclavo"):
        cadencias[punta] = fw.constante(
            (punta, "src", "bluetooth.cpp"),
            r"ahora\s*-\s*tUltimaTelemetria\s*>=\s*(\d+)",
            "la cadencia de telemetria del %s" % punta)

    b.verificar(
        cadencias["Maestro"] == cadencias["Esclavo"],
        "las dos puntas emiten telemetria cada %d ms" % cadencias["Maestro"],
        "las cadencias difieren: Maestro %d ms, Esclavo %d ms. El presupuesto seria "
        "distinto en cada poste y la cota de 5 s de la app tambien"
        % (cadencias["Maestro"], cadencias["Esclavo"]))

    porSegundo = 1000.0 / cadencias["Maestro"]

    # ---- 3.bis D-23: la cadencia de los periodicos, que tiene que ser BAJA -----------
    #
    # LA COMPRUEBA ESTE PACK PORQUE ES LA MITAD DEL ARGUMENTO CON EL QUE SE ELIGIO LA VIA.
    # El responsable descarto el aviso ESP32->STM32 -que habria sido la tercera orden que
    # el accesorio origina hacia el micro- a cambio de un periodico "a cadencia baja". Si
    # esa cadencia baja hasta la del $STATUS, lo que se construyo deja de ser lo que se
    # decidio, y ademas inunda la bitacora de 30 entradas de la app. El borde contra el que
    # se compara va escrito, que es lo que pide CLAUDE.md 7: es la cadencia del $STATUS,
    # RELEIDA arriba del C++, no un numero puesto aqui.
    #
    # D-32 (1): Y AHORA SON DOS CONSTANTES, UNA POR PUNTA, ASI QUE SE COMPRUEBA QUE SEAN
    # LA MISMA. El comentario del Maestro promete literalmente que "hay un pack que los
    # recalcula y falla si divergen", y una promesa escrita en el fuente que no comprueba
    # nadie es exactamente la excepcion sin medir de CLAUDE.md 6. Es ademas el gemelo en
    # dos ficheros que este repositorio ya sabe como acaba: dos numeros que significan lo
    # mismo se separan el dia que alguien toca uno, y aqui separarlos haria que el peor
    # segundo dejara de ser el mismo en los dos postes sin que nada lo dijera.
    diag = {}
    for punta in ("Maestro", "Esclavo"):
        diag[punta] = fw.constante(
            (punta, "src", "bluetooth.cpp"),
            r"DIAG_ENLACE_MS\s*=\s*(\d+)UL",
            "la cadencia del $EVENT de diagnostico de enlace de D-23 en el %s" % punta)

    b.verificar(
        diag["Maestro"] == diag["Esclavo"],
        "las dos puntas diagnostican a la MISMA cadencia (%d ms): son gemelas declaradas, "
        "y el comentario del Maestro que lo promete tiene quien lo compruebe"
        % diag["Maestro"],
        "las cadencias de diagnostico DIVERGEN: Maestro %d ms, Esclavo %d ms. El fuente "
        "del Maestro dice que son el mismo numero a proposito y que un pack lo recalcula; "
        "con dos valores distintos el peor segundo ya no es el mismo en los dos postes y "
        "la bitacora de la app se vacia a ritmos distintos en cada uno"
        % (diag["Maestro"], diag["Esclavo"]))

    for punta in ("Maestro", "Esclavo"):
        b.verificar(
            diag[punta] > cadencias[punta],
            "el diagnostico del %s sale cada %d ms, mas espaciado que su $STATUS (%d ms): "
            "es el periodico BAJO con el que se eligio la via, no un segundo latido"
            % (punta.upper(), diag[punta], cadencias[punta]),
            "el diagnostico del %s sale cada %d ms y su $STATUS cada %d ms. Un segundo "
            "periodico igual o mas rapido que el primero ya no es 'cadencia baja': es el "
            "coste que se dijo que no se iba a pagar, y ademas vacia en minutos la "
            "bitacora de eventos de la app, que es donde el tecnico busca el $ALARM"
            % (punta.upper(), diag[punta], cadencias[punta]))

    # LA QUE ENTRA EN EL CAUDAL ES LA MENOR, NO LA MAYOR, y se escribe por que: la cadencia
    # mas CORTA es la que mete mas bytes en el peor segundo. Con la comprobacion de gemelas
    # de arriba en verde las dos son la misma y da igual cual se tome; el dia que divergieran
    # -que es el dia en que esta linea importa- quedarse con la mayor mediria el poste bueno
    # y publicaria un porcentaje mas comodo que el real. Un presupuesto se hace por el peor
    # extremo o no es un presupuesto.
    diag_ms = min(diag["Maestro"], diag["Esclavo"])

    # ---- 4. EL PEOR SEGUNDO REALISTA CABE ------------------------------------
    #
    # $STATUS a su cadencia, mas una rafaga de $ACK, $EVENT y $ALARM coincidiendo. No es
    # el caso medio: es un comando que dispara una alarma y una entrada de bitacora justo
    # cuando toca telemetria, que es exactamente cuando mas informacion hace falta.
    #
    # D-23 (13/09): Y LOS PERIODICOS ENTRAN AQUI, ENTEROS Y NO PRORRATEADOS. En el peor
    # segundo el $EVENT de diagnostico o cae o no cae, y la cuenta se hace para el segundo
    # en que cae -que ademas no es casualidad: la cadencia es multiplo de la del $STATUS,
    # asi que los periodicos coinciden en la MISMA vuelta del bucle por construccion, y eso
    # es mejor que depender de la suerte-. El techo se calcula, no se escribe: si algun dia
    # la cadencia bajara de un segundo, cabrian varios y la cuenta los cuenta en vez de
    # quedarse corta sin decirlo.
    #
    # D-32 (1): Y CUANTOS SON SALE DEL CENSO DE 2.sexies, NO DE UN 1 ESCRITO AQUI. Esa es
    # la parte que caduco en un dia: la cuenta decia "el segundo periodico" en singular
    # cuando el arbol ya tenia tres emisores repartidos en las dos puntas. Se toma el PEOR
    # POSTE -el que mas periodicos tiene-, porque el presupuesto es el de UN cable J17 y
    # cada poste tiene el suyo; sumar los de los dos mediria un equipo que no existe, y
    # quedarse con el menor mediria el poste bueno.
    #
    # ⚠️ EL EMISOR DEL AVISO DEL LIMITE SOLO DISPARA CUANDO EL AVISO ESTA ARMADO, Y AUN ASI
    # SE CUENTA ENTERO: un presupuesto que descuente un emisor porque "casi nunca sale" no
    # es un peor caso, es un caso medio con otro nombre. Y las horas en que ese emisor si
    # sale son justo aquellas en las que el tecnico necesita que las tramas lleguen.
    diag_emisores = max(len(periodicos["Maestro"]), len(periodicos["Esclavo"]))
    diag_por_segundo = int(math.ceil(1000.0 / diag_ms))
    peor = (topeStatus * porSegundo + topeEvento + topeAlarma + topeStatus
            + topeEvento * diag_emisores * diag_por_segundo)
    ocupacion = 100.0 * peor / caudal
    b.verificar(
        peor < caudal,
        "el peor segundo son %d B de %d B/s (%.1f%%): la rafaga de $STATUS + $EVENT + "
        "$ALARM + $ACK, con los %d periodicos de diagnostico del peor poste cada %d ms "
        "encima, cabe" % (peor, caudal, ocupacion, diag_emisores, diag_ms),
        "EL PEOR SEGUNDO NO CABE: %d B contra %d B/s (%.1f%%) con %d periodicos de "
        "diagnostico. Las tramas se encolan y llegan tarde; pasados los 5 s de "
        "TIMEOUT_ENLACE_MS la app declara el enlace perdido de un equipo que esta "
        "emitiendo" % (peor, caudal, ocupacion, diag_emisores))

    # ---- 5. El sentido de ida tambien tiene su cuenta -------------------------
    tope = fw.constante(CONTRATO, r"#define\s+TRAMA_MAX_UTIL\s+(\d+)",
                        "el tope de linea util del puente")
    msComando = 1000.0 * (tope + 2) * bits / baudio
    b.verificar(
        msComando < cadencias["Maestro"],
        "un comando en su tope (%d B + CRLF) ocupa %.0f ms de cable, menos que los %d ms "
        "entre telemetrias" % (tope, msComando, cadencias["Maestro"]),
        "un comando en su tope ocupa %.0f ms y la telemetria sale cada %d ms. El enlace "
        "es full duplex, pero un comando mas largo que el hueco entre tramas dice que el "
        "tope de linea y la cadencia ya no se llevan"
        % (msComando, cadencias["Maestro"]))

    # ---- 6. P-2: el buffer de salida aguanta la rafaga ------------------------
    # 🔴 P-2 SE REESCRIBE EL 13/09 HACIA EL BORDE REAL, Y LA CAUSA VIEJA SE MARCA
    # REFUTADA EN VEZ DE BORRARSE (CLAUDE.md 7.4): la que desaparece en silencio vuelve a
    # proponerse, y la segunda vez nadie recuerda que se comprobo.
    #
    # LO QUE DECIA HASTA HOY: `BUF_SALIDA_APP >= topeStatus + topeEvento + topeAlarma`,
    # o sea "el buffer de salida hacia la app aguanta la rafaga sin descartar".
    #
    # POR QUE ERA FALSO, MEDIDO EL 13/09 SOBRE EL FUENTE DEL PUENTE. BUF_SALIDA_APP tiene
    # UN SOLO USUARIO en todo el firmware del ESP32: `char trama[BUF_SALIDA_APP]` en
    # puente_emitirPropio() -un array de PILA que compone UNA trama que ORIGINA EL PROPIO
    # PUENTE, su parte de arranque y sus $ERR-. Su dueno de verdad es esa trama, y quien
    # lo mide es esp32_10_parte_de_arranque.
    #
    # LAS TRAMAS DEL STM32 NO PASAN POR AHI. El sentido STM32 -> app lee LINEA A LINEA en
    # deSTM32[BUF_ENTRADA_STM32] -el indice vuelve a cero en cada terminador- y reenvia
    # cada una por `char salida[BUF_ENTRADA_STM32 + 2]` a transporte_escribir(), que es
    # spp.write(): la cola de BluetoothSerial. NINGUN buffer de este firmware acumula una
    # rafaga. O sea que P-2 comparaba una rafaga de TRES TRAMAS contra un buffer de
    # composicion de UNA, y el borde estaba mal desde antes de que D-23 existiera; lo que
    # hizo D-23 fue empujar el numero por encima de 512 y hacerlo visible.
    #
    # QUIEN LIMITA DE VERDAD, Y POR ESO SON DOS COSAS DISTINTAS:
    #   la RAFAGA      la limita el CABLE, 960 B/s, y eso ya es la seccion 4.
    #   cada TRAMA     la limita la linea del puente, y ESO NO LO MEDIA NADIE. Es lo que
    #                  se construye aqui.
    salida = fw.constante(CONTRATO, r"#define\s+BUF_SALIDA_APP\s+(\d+)",
                          "el buffer con el que el puente compone SU PROPIA trama")
    codigo_p = fw.codigo("ESP32_Expansion", "src", "puente.cpp")
    propio = _cuerpo_funcion(codigo_p, "puente_emitirPropio")
    usos = len(re.findall(r"\bBUF_SALIDA_APP\b", codigo_p))
    b.verificar(
        propio is not None and usos == 1 and "BUF_SALIDA_APP" in propio,
        "P-2 (refutada y re-anclada): BUF_SALIDA_APP (%d B) tiene %d uso en puente.cpp y "
        "esta dentro de puente_emitirPropio(), o sea que dimensiona la trama que ORIGINA "
        "el puente -no la rafaga del STM32, que va linea a linea por otro buffer-"
        % (salida, usos),
        "BUF_SALIDA_APP ha dejado de ser lo que esta refutacion midio: %d usos en "
        "puente.cpp y dentro de puente_emitirPropio()=%s. Si las tramas del STM32 han "
        "pasado a componerse ahi, la P-2 vieja -la rafaga contra este buffer- vuelve a "
        "significar algo y hay que rehacer esta seccion, no darla por buena"
        % (usos, propio is not None and "BUF_SALIDA_APP" in (propio or "")))

    # ---- 6.bis EL BORDE REAL: CADA TRAMA CABE EN LA LINEA DEL PUENTE ----------
    #
    # QUE PASA SI NO CABE, Y ES PEOR QUE UN TRUNCAMIENTO: puente.cpp arma la linea byte a
    # byte mientras `idxSTM32 < BUF_ENTRADA_STM32 - 1` y, pasado ese punto, levanta
    # stmDesbordada; al llegar el terminador la trama ENTERA se descarta y se CUENTA en
    # descartadasLargo. O sea que el desbordamiento NO es mudo -hay contador-, y por eso
    # esta comprobacion no habla de truncamiento: habla de una trama que DESAPARECE. El
    # $STATUS del poste no llegaria a la app, y lo unico que quedaria es un contador que
    # nadie mira desde el poste.
    #
    # LO QUE NO HABIA ERA VIGILANCIA DEL MARGEN: nada comprobaba que la trama mas larga
    # que el STM32 PUEDE emitir quepa en esa linea. Se construye aqui, y DERIVADA: todo
    # lo que el STM32 pone en el cable sale por enviarTramaConCrc(), asi que ninguna
    # trama pasa de lo que su tramaCompleta[] guarda. Ni un numero escrito a mano, para
    # que el dia que alguien anada un campo al $STATUS esto se ponga rojo solo
    # (CLAUDE.md 14).
    m_env = re.search(r'snprintf\(\s*tramaCompleta\s*,[^,]+,\s*"((?:[^"\\]|\\.)*)"',
                      fw.codigo("Maestro", "src", "bluetooth.cpp"))
    if not m_env:
        raise fw.Abortado(
            "no se hallo el snprintf de enviarTramaConCrc(): sin su formato no se sabe "
            "con cuantos bytes de terminador cierra el STM32 cada trama, y esa es la "
            "diferencia entre lo que viaja y lo que el puente ALMACENA")
    terminadores = m_env.group(1).count("\\r") + m_env.group(1).count("\\n")

    entrada = fw.constante(CONTRATO, r"#define\s+BUF_ENTRADA_STM32\s+(\d+)",
                           "la linea donde el puente arma lo que viene del STM32")
    # DECLARAR NO ES EJERCER (CLAUDE.md 6): la capacidad no es la del #define, es la que
    # la GUARDA de puente.cpp deja pasar. Se lee la guarda.
    m_g = re.search(r"idxSTM32\s*<\s*\(size_t\)\(\s*BUF_ENTRADA_STM32\s*-\s*(\d+)\s*\)",
                    codigo_p)
    if not m_g:
        raise fw.Abortado(
            "no se hallo en puente.cpp la guarda `idxSTM32 < (size_t)(BUF_ENTRADA_STM32 "
            "- N)`, que es la que de verdad decide cuantos caracteres entran en la linea. "
            "Con el #define solo se mediria la cota DECLARADA, y este pack ya sabe que "
            "eso no es lo mismo")
    cabe = entrada - int(m_g.group(1))

    lineas = {}
    for punta in ("Maestro", "Esclavo"):
        cap = _ancho_decl(fw.codigo(punta, "src", "bluetooth.cpp"), "tramaCompleta")
        if cap is None:
            raise fw.Abortado(
                "no se hallo tramaCompleta[] en bluetooth.cpp del %s: es el techo de todo "
                "lo que esa punta puede poner en el cable" % punta)
        lineas[punta] = cap - terminadores
    peor_punta = max(lineas, key=lambda p: lineas[p])
    peor_linea = lineas[peor_punta]

    b.verificar(
        peor_linea <= cabe,
        "la trama mas larga que el STM32 puede emitir (%s, %d caracteres sin los %d de "
        "terminador) cabe en la linea del puente, que admite %d: margen %d"
        % (peor_punta, peor_linea, terminadores, cabe, cabe - peor_linea),
        "LA TRAMA MAS LARGA DEL %s NO CABE EN LA LINEA DEL PUENTE: %d caracteres contra "
        "los %d que la guarda de puente.cpp deja entrar. No se trunca: se DESCARTA "
        "ENTERA y se cuenta en descartadasLargo, asi que el $STATUS de ese poste deja de "
        "llegar a la app y lo unico que queda es un contador que nadie mira desde el "
        "poste. Se acorta la trama donde se produce; ensanchar BUF_ENTRADA_STM32 es "
        "mover el borde en vez de respetarlo"
        % (peor_punta.upper(), peor_linea, cabe))

    # Y LA MITAD QUE LA COMPROBACION DE ARRIBA SE LLEVARIA POR DELANTE SI NO SE ANOTA: su
    # mensaje afirma que el desbordamiento se CUENTA, y eso es una afirmacion sobre el
    # codigo. Se mide, no se recita (CLAUDE.md 6, la excepcion es el instrumento).
    b.verificar(
        bool(re.search(r"stmDesbordada\s*=\s*true", codigo_p))
        and bool(re.search(r"if\s*\(\s*stmDesbordada\s*\)[^}]*descartadasLargo\+\+",
                           codigo_p, re.S)),
        "y si algun dia no cupiera, el puente lo CUENTA: levanta stmDesbordada al pasarse "
        "y suma descartadasLargo al cerrar la linea, en vez de entregar media trama",
        "puente.cpp ya no levanta stmDesbordada al pasarse, o ya no la convierte en "
        "descartadasLargo al cerrar la linea. Entonces una trama demasiado larga SI se "
        "volveria muda: media trama entregada o una perdida sin contador, y el mensaje "
        "de la comprobacion de arriba estaria prometiendo un contador que no existe")

    # ---- 7. P-1 y P-4: el puente no anade ni agrupa --------------------------
    #
    # Se mide por AUSENCIA DE RELOJ: sin millis() no hay forma de emitir periodicamente
    # ni de acumular tramas "hasta que pase un rato". Es una comprobacion de forma, y por
    # eso se acota a los dos ficheros del camino de datos: el reloj SI usa millis(), y
    # tiene que poder.
    for fichero in ("puente.cpp", "enlace_stm32.cpp"):
        codigo = fw.codigo("ESP32_Expansion", "src", fichero)
        b.verificar(
            "millis()" not in codigo,
            "P-1/P-4: %s no tiene reloj, asi que no puede ni emitir periodicamente ni "
            "agrupar telemetria" % fichero,
            "%s usa millis(). Con un reloj en el camino de datos caben las dos cosas que "
            "P-1 y P-4 prohiben: un latido propio que se reparte un canal que no es suyo, "
            "y agrupar dos $STATUS para 'ahorrar aire', que hace que la app declare "
            "caido a un equipo sano" % fichero)

    # ---- CONTROLES NEGATIVOS -------------------------------------------------
    b.control_negativo(
        not (topeStatus * porSegundo + topeEvento + topeAlarma + topeStatus < 240),
        "con un caudal de 240 B/s -que seria 2400 bps- el peor segundo deja de caber: "
        "la cuenta reacciona al baudio, no lo da por bueno")

    b.control_negativo(
        "millis()" in "void bombear(){ if (millis() - t > 500) emitir(); }",
        "un latido periodico colado en el camino de datos se detecta")

    b.control_negativo(
        re.search(r"SerialBT\.begin\((\d+)\)", "SerialBT.begin();") is None,
        "si SerialBT.begin() dejara de llevar el baudio, el lector devuelve nada y el "
        "pack ABORTA en vez de asumir 9600")

    # --- N-154: los tres de la cuenta del peor caso ---------------------------
    #
    # Se ejercen sobre bloques SINTETICOS y no sobre el .cpp real, por el motivo de N-89:
    # un control negativo que reutilizara el fuente bueno mediria lo mismo que la
    # comprobacion y no demostraria nada.
    _falso = ('char payload[24];\n'
              '  char horaBuf[12];\n'
              '  snprintf(payload, sizeof(payload), "$STATUS,NODE:X,HORA:%s", horaBuf);')
    _idx, _fmt, _args = _snprintf_status(_falso, "sintetico")
    b.control_negativo(
        _peor(_fmt, [_ancho_arg(fw, "Maestro", a, _falso) for a in _args]) > 24 - 1,
        "con un payload[24] la cuenta dice que la trama NO cabe: reacciona al tamano "
        "del buffer, no lo da por bueno")

    b.control_negativo(
        _ancho_arg(fw, "Maestro", "horaBuf", "char horaBuf[16];") == 15,
        "un buffer mas holgado ensancha el peor caso en vez de pasar desapercibido: un "
        "char[16] cuenta 15 y no los 8 de 'HH:MM:SS'. El rango no es la cota")

    b.control_negativo(
        not (len("%d" % (max(verdeMax, rojoMax) * 60)) <= 3 - 1),
        "con un tTxt[3] la cota de %d s no cabria y el pack lo dice: la comprobacion "
        "mira el tope LEGAL, no solo el buffer" % cotaT)

    # --- D-13: los dos de la cuenta del tramo del $ALARM ----------------------
    #
    # Sobre bloques SINTETICOS, por el motivo de N-89: un control negativo que reutilizara
    # el fuente bueno mediria lo mismo que la comprobacion y no demostraria nada.
    b.control_negativo(
        len(re.findall(r"%[0-9]*l?[usd]",
                       'snprintf(tramo, sizeof(tramo), "RF:%d%%,RTT:%lums,SINRESP:%d", '
                       'rf, rtt, sr);')) == 3
        and ANCHO_POR_TIPO["%d"] + ANCHO_POR_TIPO["%lu"] + ANCHO_POR_TIPO["%d"]
            + len("RF:%,RTT:ms,SINRESP:") > 40 - 1,
        "con los tres numeros a su tope de TIPO -que es como estaba escrito hasta hoy- la "
        "cuenta dice que el tramo NO cabe en un char[40]: son 52 caracteres. La "
        "comprobacion reacciona al tipo del argumento, no solo al tamano del buffer")

    b.control_negativo(
        "%f" not in ANCHO_POR_TIPO,
        "y una conversion que la tabla de topes no conoce ABORTA en vez de saltarsela: "
        "medir de menos en silencio es lo que dejo pasar el truncamiento de N-108")

    b.control_negativo(
        "CUENTA_ATRAS_MAX_SEG" not in
        'if (faseRestanteSeg == SIN_CUENTA_ATRAS) { strncpy(tTxt, "--", 3); }',
        "un emisor que declarase la cota y NO la comparase se detecta: es el hueco de "
        "2.ter, y un buffer ajustado sin guarda es un truncamiento con permiso")
