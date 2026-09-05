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

    # ---- 4. EL PEOR SEGUNDO REALISTA CABE ------------------------------------
    #
    # $STATUS a su cadencia, mas una rafaga de $ACK, $EVENT y $ALARM coincidiendo. No es
    # el caso medio: es un comando que dispara una alarma y una entrada de bitacora justo
    # cuando toca telemetria, que es exactamente cuando mas informacion hace falta.
    peor = topeStatus * porSegundo + topeEvento + topeAlarma + topeStatus
    ocupacion = 100.0 * peor / caudal
    b.verificar(
        peor < caudal,
        "el peor segundo son %d B de %d B/s (%.1f%%): la rafaga de $STATUS + $EVENT + "
        "$ALARM + $ACK cabe" % (peor, caudal, ocupacion),
        "EL PEOR SEGUNDO NO CABE: %d B contra %d B/s (%.1f%%). Las tramas se encolan y "
        "llegan tarde; pasados los 5 s de TIMEOUT_ENLACE_MS la app declara el enlace "
        "perdido de un equipo que esta emitiendo" % (peor, caudal, ocupacion))

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
    salida = fw.constante(CONTRATO, r"#define\s+BUF_SALIDA_APP\s+(\d+)",
                          "el buffer de salida hacia la app")
    rafaga = topeStatus + topeEvento + topeAlarma
    b.verificar(
        salida >= rafaga,
        "P-2: el buffer hacia la app (%d B) aguanta la rafaga de %d B sin descartar"
        % (salida, rafaga),
        "P-2 ROTA: el buffer hacia la app son %d B y la rafaga %d B. Una rafaga que "
        "coincida con un $STATUS descartaria tramas justo cuando mas hay que contar"
        % (salida, rafaga))

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

    b.control_negativo(
        "CUENTA_ATRAS_MAX_SEG" not in
        'if (faseRestanteSeg == SIN_CUENTA_ATRAS) { strncpy(tTxt, "--", 3); }',
        "un emisor que declarase la cota y NO la comparase se detecta: es el hueco de "
        "2.ter, y un buffer ajustado sin guarda es un truncamiento con permiso")
