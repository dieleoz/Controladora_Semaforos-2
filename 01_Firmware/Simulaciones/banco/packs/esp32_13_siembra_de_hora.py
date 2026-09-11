# ===== banco/packs/esp32_13_siembra_de_hora.py =====
#
# D-20 / D-26 (11/09): EL ESP32 LE DA LA HORA A SU STM32. CUANDO, CON QUE, Y QUIEN NO.
#
# "Son los ESP32 los que tienen el reloj y deben comandar la hora" (el responsable,
# 11/09). Lo construido hasta hoy no lo cumplia: el telefono le mandaba la hora al STM32
# y el ESP32 nunca le sembraba la suya (roadmap 3.16-B). Desde hoy siembra.cpp manda
# CMD:HORA_ESP32:YYYY-MM-DD,HH:MM:SS al arrancar, tras cada SET_RTC bueno y cada ~5 min.
#
# QUIEN MIDE QUE. esp32_05 mide la BARRERA: que esta sea la unica orden hacia el STM32
# y que se componga solo de reloj_leer(). Este pack mide lo demas, que es lo que hace
# que la siembra sirva o que haga dano:
#
#   1. D-26 (2): ~~A-15: la cadencia es EL MISMO NUMERO que INTERVALO_SYNC_MS del
#      Maestro~~ -> REVISADA UNA POR UNA el 11/09 (CLAUDE.md 9). Afirmaba DOS cosas:
#        (i)  "hay dos copias de la cadencia en dos binarios y tienen que valer lo mismo"
#             -> SE MUDA: la otra copia ya no es INTERVALO_SYNC_MS del Maestro -ese sigue
#             siendo el reenvio por radio, otro numero para otra cosa- sino lo que el STM32
#             ESPERA (HORA_ESP32_CADENCIA_MS, reloj.h de las dos puntas), de la que cuelga
#             su alarma de D-26 (5). Esa igualdad se recalcula aqui.
#        (ii) "la cadencia es una hora" -> SE BORRA: documentaba la decision que D-26
#             corrigio. En su sitio va la RELACION que la sustituye, recalculada del C++ y
#             no una igualdad: la deriva del HSI de las dos puntas entre siembras cabe en
#             el margen del cruce (N-71), y una siembra normal no manda el Degradado a rojo.
#   1.bis el umbral del salto de hora de D-26 (4) sale del despeje en las dos puntas y
#      coincide con el desfase que el cruce aguanta, barrido con el modelo de costura.
#   2. los reintentos de arranque llegan DESPUES de que el STM32 abra J17, recalculado
#      desde el setup() de las dos puntas;
#   3. los tres disparadores tienen llamador (CLAUDE.md 6.1: declarar no es ejercer);
#   4. tras SET_RTC se siembra SOLO si el DS3231 la acepto y se releyo, y los dos $ACK
#      dependen de lo que devolvio la siembra -la barrera de CLAUDE.md 2 hasta lo que el
#      equipo contesta-;
#   5. ANTI-SUPLANTACION: la linea del telefono con HORA_ESP32 no cruza, y se contesta;
#   6. la linea cabe en el buffer del STM32, con la cota derivada de los rangos;
#   6.bis LA COSTURA DE FORMATO: todo lo que el ESP32 puede componer -los bordes de sus
#      rangos, por su literal- pasa el PATRON_ISO y la regla de rango de las DOS puntas.
#      Si no, cada siembra de ese borde seria la alarma de D-26 (5) con el circuito sano;
#   7. 🔴 las dos puntas del STM32 la reconocen ANTES de su guarda de PIN. Sin eso cada
#      siembra le devuelve al telefono "$ERR,CMD:AUTH_FAILED,DESC:PIN_INVALIDO" en rojo,
#      en cada cadencia y en cada arranque: el FALLA PERMANENTE de CLAUDE.md 1 -un rechazo
#      que nadie puede apagar ensena a ignorar los de verdad-.
#
# LO QUE NO MIDE, Y NO PUEDE: Python leyendo .cpp. No ejecuta el firmware, no ha visto
# un DS3231 ni un STM32 recibir esta linea. Que el STM32 la extrapole bien, la declare
# vieja o la propague al Esclavo es de su firmware y de sus packs. El lazo entero
# app -> puente -> STM32 real lo ejerce el escenario D20 de simulador_puente_esp32.py.
#
# SIN ETIQUETA SFTY: roza SFTY-18 -de la hora cuelga el Degradado- pero no ejerce la
# regla; mide la forma de un emisor. Una fila cubierta por una prueba que no la ejerce
# es peor que una vacia.

import re

from banco.modelos.costura import (luz_maestro, luz_esclavo, VERDE, SEGUNDOS_DEL_DIA,
                                   E_AMARILLO_MS, DEG_VERDE_SEG, DEG_DESPEJE_SEG)

NOMBRE = "esp32_13_siembra_de_hora"
DESCRIPCION = ("la siembra de hora ESP32->STM32: cadencia de D-26 contra el margen del cruce, "
               "reintentos, disparadores, $ACK que mira, anti-suplantacion y la costura de "
               "formato y de receptor con el STM32")

ROL = "ESP32_Expansion"
CONTRATO = ("ESP32_Expansion", "include", "contrato.h")
SIEMBRA = ("ESP32_Expansion", "src", "siembra.cpp")
DESPACHADOR = ("ESP32_Expansion", "src", "despachador.cpp")
MAIN = ("ESP32_Expansion", "src", "main.cpp")
PUNTAS = ("Maestro", "Esclavo")
FORMATO = "FORMATO_HORA_ESP32"
PREDICADO = "despachador_esParaElPuente"
ATENDER = "despachador_atender"
# D-26 (2): la cadencia en el ESP32 y lo que cada STM32 espera. Otro binario cada uno.
RE_SIEMBRA_ESP32 = r"#define\s+SIEMBRA_INTERVALO_MS\s+(\d+)UL"
RE_CADENCIA_STM32 = r"\bHORA_ESP32_CADENCIA_MS\s*=\s*(\d+)UL"
# La espera de la alarma se escribe como PRODUCTO de la cadencia: se lee el factor, para
# que "tres cadencias" sea el numero y no una frase.
RE_ESPERA_STM32 = r"\bHORA_ESP32_ESPERA_MAX_MS\s*=\s*(\d+)UL\s*\*\s*HORA_ESP32_CADENCIA_MS"
RE_HSI = r"\bHSI_PPM_PEOR\s*=\s*(\d+)UL"
DEG_M = ("Maestro", "src", "modo_degradado.cpp")
DEG_E = ("Esclavo", "src", "modo_degradado.cpp")
RELOJ_H = {p: (p, "include", "reloj.h") for p in PUNTAS}
RELOJ_C = {p: (p, "src", "reloj.cpp") for p in PUNTAS}
# El segundo entero que se pierde en CADA siembra: la linea lleva segundos enteros y el
# STM32 ancla su base al recibirla (ver la nota de "SEGUNDOS ENTEROS" en los reportar de
# abajo). Es el mismo RESIDUO_SYNC_S con el que costura_12 descuenta la hora por radio.
RESIDUO_SIEMBRA_S = 1
# 2 ppm por DS3231 (su ficha, TCXO de 0 a 40 C): lo que se separan dos postes puestos en
# hora por separado. Es de D-26, no del firmware -ningun C++ lo usa-, y solo alimenta un
# reportar(), que no cuenta: dice cuanto margen le queda a esa discrepancia.
DS3231_PPM = 2


# --- Bloques literales de packs ya probados (esp32_12), no reescritos --------------
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


def _contenidos(codigo, cuerpoPred):
    """{funcion: literal} de los criterios 'contiene' del predicado (de esp32_12)."""
    fuera = {}
    for nombre in sorted(set(re.findall(r"\b([A-Za-z_]\w*)\s*\(\s*linea\s*\)", cuerpoPred))):
        if nombre in ("strcmp", "strstr", "strncmp"):
            continue
        cuerpoF = _cuerpo(codigo, r"\b%s\s*\(\s*const\s+char\s*\*\s*linea\s*\)" % re.escape(nombre))
        if cuerpoF is None:
            continue
        m = re.search(r'strstr\s*\(\s*linea\s*,\s*"([^"]+)"\s*\)', cuerpoF)
        if m:
            fuera[nombre] = m.group(1)
    return fuera


# --- Lo propio de este pack --------------------------------------------------------
def _arranque_stm32(setup, espera_lse):
    """Cota INFERIOR de lo que tarda una punta en abrir J17: (ms, desglose).

    EL BORDE, ESCRITO (CLAUDE.md 7): cuenta los delay() LITERALES del setup() que van
    antes de bluetooth_setup(), mas ESPERA_LSE_MS si reloj_setup() va antes -con Y2
    muerto, esa espera se agota entera-. NO cuenta lo que tarda cada *_setup() en si,
    que nadie ha medido. Por eso es una cota inferior, y por eso contrato.h pide a
    REINTENTO_1 MAS DEL DOBLE y deja un REINTENTO_2 para lo que esta cota no ve."""
    i = setup.find("bluetooth_setup(")
    if i < 0:
        return None, "no hay bluetooth_setup() en setup()"
    antes = setup[:i]
    delays = [int(x) for x in re.findall(r"\bdelay\s*\(\s*(\d+)\s*\)", antes)]
    ms = sum(delays)
    partes = ["delay(%d)" % d for d in delays]
    if re.search(r"\breloj_setup\s*\(", antes):
        if espera_lse is None:
            return None, "reloj_setup() va antes de bluetooth_setup() y no se leyo ESPERA_LSE_MS"
        ms += espera_lse
        partes.append("ESPERA_LSE_MS=%d" % espera_lse)
    return ms, " + ".join(partes) or "nada"


def _largo_maximo(literal, maximos):
    """Largo de la linea en el peor caso, conversion a conversion.

    Solo sabe %0Nd con enteros NO NEGATIVOS (los rangos de contrato.h empiezan en 0 o
    mas): el ancho es el mayor entre N y las cifras del maximo del campo. Cualquier otra
    conversion devuelve None y el pack ABORTA en vez de medir de menos (N-108)."""
    convs = re.findall(r"%[^%]*?[a-zA-Z]", literal)
    if len(convs) != len(maximos):
        return None
    fijo = len(re.sub(r"%[^%]*?[a-zA-Z]", "", literal))
    total = fijo
    for c, mx in zip(convs, maximos):
        m = re.match(r"^%0?(\d*)d$", c)
        if not m:
            return None
        ancho = int(m.group(1) or 0)
        total += max(ancho, len(str(mx)))
    return total


def _receptor_antes_del_pin(codigo_bt, prefijo):
    """(ok, detalle): el STM32 reconoce `prefijo` en procesarComando() ANTES de su guarda
    de PIN. Se admite el literal o una constante que lo contenga."""
    cuerpo = _cuerpo(codigo_bt, r"static\s+void\s+procesarComando\s*\([^)]*\)")
    if cuerpo is None:
        return None, "no se hallo procesarComando()"
    mPin = re.search(r'strncmp\s*\(\s*cmd\s*,\s*"CMD:PIN:', cuerpo)
    if mPin is None:
        return None, "no se hallo la guarda de PIN (strncmp(cmd, \"CMD:PIN:...\"))"
    posiciones = [m.start() for m in re.finditer(r'"%s' % re.escape(prefijo), cuerpo)]
    for nombre in re.findall(r'\b(\w+)\s*\[\s*\]\s*=\s*"%s[^"]*"' % re.escape(prefijo),
                             codigo_bt):
        posiciones += [m.start() for m in re.finditer(r"\b%s\b" % re.escape(nombre), cuerpo)]
    if not posiciones:
        return False, "no aparece %r en procesarComando()" % prefijo
    if min(posiciones) > mPin.start():
        return False, "aparece DESPUES de la guarda de PIN (%d > %d)" % (min(posiciones),
                                                                      mPin.start())
    return True, "antes de la guarda de PIN (%d < %d)" % (min(posiciones), mPin.start())


def _siembra_tras_set_rtc(atender):
    """(ok, detalle): siembra_ahora() se llama UNA vez, dentro del RELOJ_OK final y
    detras de una relectura buena, y gobierna los dos $ACK de SET_RTC."""
    llamadas = [m.start() for m in re.finditer(r"\bsiembra_ahora\s*\(", atender)]
    if len(llamadas) != 1:
        return False, "%d llamadas a siembra_ahora() en %s(): tiene que haber UNA" % (
            len(llamadas), ATENDER)
    mNoOk = re.search(r"else\s+if\s*\(\s*r\s*!=\s*RELOJ_OK\s*\)\s*\{", atender)
    if mNoOk is None:
        return False, "no se hallo la rama `else if (r != RELOJ_OK)`"
    fin = mNoOk.end() - 1 + len(_bloque(atender, mNoOk.end() - 1) or "") + 1
    mElse = re.match(r"\s*else\s*\{", atender[fin + 1:])
    if mElse is None:
        return False, "detras de `r != RELOJ_OK` no hay un `else {` final: el RELOJ_OK"
    ramaOk = _bloque(atender, fin + 1 + mElse.end() - 1)
    if ramaOk is None or "siembra_ahora" not in ramaOk:
        return False, ("siembra_ahora() no vive en la rama RELOJ_OK: se sembraria el STM32 "
                       "con una hora que el DS3231 rechazo -3.16-C-")
    mLee = re.search(r"if\s*\(\s*!\s*reloj_leer\s*\(\s*&\s*\w+\s*\)\s*\)\s*\{", ramaOk)
    mSie = re.search(r"else\s+if\s*\(\s*!\s*siembra_ahora\s*\(\s*\)\s*\)\s*\{", ramaOk)
    if mLee is None or mSie is None or mSie.start() < mLee.start():
        return False, ("en la rama RELOJ_OK no esta `if (!reloj_leer(&x)) {...} else if "
                       "(!siembra_ahora()) {...}` en ese orden: se sembraria sin haber "
                       "releido, o el resultado de la siembra no se mira")
    sinProp = _bloque(ramaOk, mSie.end() - 1) or ""
    tras = ramaOk[mSie.end() - 1 + len(sinProp) + 2:]
    mUlt = re.match(r"\s*else\s*\{", tras)
    okRama = _bloque(tras, mUlt.end() - 1) if mUlt else None
    if "RESULT:HORA_PUESTA_SIN_PROPAGAR" not in sinProp:
        return False, "el $ACK SIN_PROPAGAR no vive en la rama `!siembra_ahora()`"
    if okRama is None or '"$ACK,NODE:PUENTE,CMD:SET_RTC,RESULT:OK' not in okRama:
        return False, "el $ACK RESULT:OK no vive en el `else` de la siembra"
    return True, ("UNA siembra, en la rama RELOJ_OK, detras de la relectura, y los dos $ACK "
                  "colgando de lo que devolvio")


def _suplantacion_primero(atender, marca_fn, token):
    """(ok, detalle): la PRIMERA rama de atender() es la anti-suplantacion, con su $ERR."""
    mSup = re.search(r"if\s*\(\s*%s\s*\(\s*linea\s*\)\s*\)\s*\{" % re.escape(marca_fn), atender)
    if mSup is None:
        return False, "no hay `if (%s(linea)) {` en %s()" % (marca_fn, ATENDER)
    rama = _bloque(atender, mSup.end() - 1) or ""
    if '"$ERR,NODE:PUENTE,CMD:%s,' % token not in rama or "return" not in rama:
        return False, "la rama no contesta $ERR,NODE:PUENTE,CMD:%s,... y vuelve" % token
    otras = [m.start() for m in re.finditer(r"\bstrcmp\s*\(\s*linea|\baccion\w*\s*\(\s*linea",
                                             atender)]
    if otras and min(otras) < mSup.start():
        return False, "hay otra rama ANTES de la anti-suplantacion"
    return True, "es la primera rama de %s(), contesta $ERR y vuelve" % ATENDER


def _primer_solape(sentido, verde, despeje, amarillo_s):
    """Primer desfase, en segundos, con las DOS puntas en verde a la vez.

    BLOQUE LITERAL de costura_12_margen_deriva, sobre luz_maestro() y luz_esclavo() del
    modelo de costura -las que costura_02 contrasta contra ciclo_degradado.h-: el margen
    del cruce no se escribe aqui, se barre igual que alli."""
    ciclo = 2 * (verde + despeje)
    for skew in range(1, ciclo):
        for s in range(3600, 3600 + ciclo):
            otro = (s + sentido * skew) % SEGUNDOS_DEL_DIA
            if (luz_maestro(s, verde, despeje) == VERDE and
                    luz_esclavo(otro, verde, despeje, amarillo_s) == VERDE):
                return skew
    return None


def _aguante(verde, despeje, amarillo_s):
    """El desfase que el cruce aguanta: el peor sentido menos el ultimo segundo que ya
    rompe, menos el residuo sub-segundo de la hora en segundos enteros. Es TOLERADO de
    costura_12, con la misma cuenta."""
    a = _primer_solape(+1, verde, despeje, amarillo_s)
    t = _primer_solape(-1, verde, despeje, amarillo_s)
    if a is None or t is None:
        return None
    return min(a, t) - 1 - RESIDUO_SIEMBRA_S


def _deriva_cadencia_s(cadencia_ms, ppm):
    """Lo que el HSI puede separar la hora de UNA punta durante una cadencia, redondeado
    hacia ARRIBA -la misma cuenta que el static_assert de Maestro/src/modo_degradado.cpp-."""
    return (cadencia_ms // 1000 * ppm + 999999) // 1000000


def _umbral_salto(fw):
    """El umbral de D-26 (4) del Maestro, EN SEGUNDOS, leido de su expresion en el C++.

    Solo sabe dos formas -"(uint32_t)DEG_DESPEJE_SEG - N" y un literal- y ABORTA con
    cualquier otra: leer un numero que no se sabe evaluar es medir de menos (N-108)."""
    t = fw.codigo(*DEG_M)
    m = re.search(r"SALTO_SIN_ROJO_MAX_S\s*=\s*([^;]+);", t)
    if m is None:
        raise fw.Abortado("no se hallo SALTO_SIN_ROJO_MAX_S en Maestro/src/modo_degradado.cpp: "
                          "sin umbral no hay regla de D-26 (4) que medir")
    expr = m.group(1).strip()
    md = re.fullmatch(r"\(uint32_t\)\s*DEG_DESPEJE_SEG\s*-\s*(\d+)UL", expr)
    if md:
        return DEG_DESPEJE_SEG - int(md.group(1))
    ml = re.fullmatch(r"(\d+)UL?", expr)
    if ml:
        return int(ml.group(1))
    raise fw.Abortado("SALTO_SIN_ROJO_MAX_S = %r no es una forma que este pack sepa evaluar"
                      % expr)


def _patron_ok(patron, s):
    """Replica de isoBienFormado() de reloj.cpp: '0' es cifra, lo demas literal, y nada
    detras. Se ejerce contra el patron LEIDO de cada punta, nunca contra uno escrito aqui."""
    if len(s) != len(patron):
        return False
    return all(c.isdigit() if p == "0" else c == p for p, c in zip(patron, s))


def correr(b, fw):
    b.titulo("La siembra de hora ESP32 -> STM32 (D-20 / D-26)")

    contrato = fw.texto(*CONTRATO)
    sie = fw.codigo(*SIEMBRA)
    desp = fw.codigo(*DESPACHADOR)
    main = fw.codigo(*MAIN)

    # =====================================================================
    b.titulo("1. D-26 (2): la cadencia contra el margen del cruce, no un numero repetido")
    # =====================================================================
    esp = fw.constante(CONTRATO, RE_SIEMBRA_ESP32, "la cadencia de la siembra en el ESP32")
    stm = {p: fw.constante(RELOJ_H[p], RE_CADENCIA_STM32,
                           "la cadencia que el %s espera de su ESP32" % p) for p in PUNTAS}
    factor = {p: fw.constante(RELOJ_H[p], RE_ESPERA_STM32,
                              "cuantas cadencias espera el %s antes de alarmar" % p)
              for p in PUNTAS}
    hsi = fw.constante(RELOJ_H["Maestro"], RE_HSI, "la deriva del HSI en su peor caso")

    # (i) SE MUDA: las copias de la cadencia en los dos binarios valen lo mismo.
    b.verificar(
        all(v == esp for v in stm.values()),
        "SIEMBRA_INTERVALO_MS del ESP32 (%d ms) es lo que las dos puntas esperan "
        "(HORA_ESP32_CADENCIA_MS: %s)" % (esp, stm),
        "el ESP32 siembra cada %d ms y el STM32 espera %s. Con cadencias distintas la "
        "alarma de D-26 (5) salta con el enlace sano -si esperan menos- o no salta con el "
        "enlace caido -si esperan de mas-; son dos binarios y la unica red es esta "
        "comparacion" % (esp, stm))

    # (i.bis) y la alarma espera VARIAS cadencias: una siembra perdida no es una averia.
    b.verificar(
        all(n >= 2 for n in factor.values()),
        "la alarma de D-26 (5) espera %s cadencias sin una HORA_ESP32 buena: una siembra "
        "perdida -el ESP32 reiniciando, un byte comido- no la dispara" % factor,
        "la alarma de D-26 (5) espera %s cadencia(s): con una sola, cualquier siembra "
        "perdida la dispara, y una alarma que salta con el circuito sano ensena a "
        "ignorarla" % factor)

    # (ii) SE BORRA la igualdad con INTERVALO_SYNC_MS y en su sitio va la RELACION, desde
    # el C++: lo que se pueden separar las DOS puntas por su HSI entre dos siembras -una
    # adelanta y la otra atrasa, en el peor caso-, mas el segundo entero de cada siembra,
    # tiene que caber en lo que el cruce aguanta. Es la cuenta que A-15 no hacia: a una
    # hora serian 2 x 90 + 2 = 182 s contra 29.
    aguante = _aguante(DEG_VERDE_SEG, DEG_DESPEJE_SEG, E_AMARILLO_MS // 1000)
    if aguante is None:
        raise fw.Abortado("el barrido del modelo de costura no encontro solape: sin aguante "
                          "no hay contra que comparar la cadencia")
    deriva1 = _deriva_cadencia_s(esp, hsi)
    deriva_rel = 2 * deriva1 + 2 * RESIDUO_SIEMBRA_S
    b.verificar(
        deriva_rel < aguante,
        "con una siembra cada %d s y el HSI a %d ppm, cada punta se aparta como mucho %d s "
        "entre siembras; las dos, en sentidos opuestos y con el segundo entero de cada "
        "siembra, %d s: por debajo de los %d s que el cruce aguanta (barrido del modelo de "
        "costura sobre DEG_VERDE_SEG, DEG_DESPEJE_SEG y el amarillo del Esclavo)"
        % (esp // 1000, hsi, deriva1, deriva_rel, aguante),
        "con una siembra cada %d s y el HSI a %d ppm las dos puntas se pueden separar %d s "
        "entre siembras y el cruce aguanta %d. En Degradado cada punta cicla por su hora: "
        "eso son dos verdes a la vez sin que falle nada" % (esp // 1000, hsi, deriva_rel,
                                                             aguante))

    presupuesto = aguante - deriva_rel
    b.reportar(
        "D-26 (2): lo que le queda a la discrepancia entre los dos DS3231",
        ["Aguanta el cruce %d s; la deriva del HSI de las dos puntas entre siembras se come "
         "%d. Quedan %d s para lo que difieran los DOS DS3231, que el sistema NO acota: se "
         "ponen en hora por separado (D-26 (3): sin radio el Esclavo toma la de SU ESP32) y "
         "la radio no escribe el DS3231 del Esclavo (D-26 lo deja como mejora)."
         % (aguante, deriva_rel, presupuesto),
         "A %d ppm por DS3231 (su ficha, TCXO), en sentidos opuestos, se separan %.2f s al "
         "dia: esos %d s se agotan en unos %.0f dias sin volver a poner en hora los dos "
         "postes -D-26 decia 'del orden de 10 s/mes'-. NO cuenta el error de quien los puso: "
         "dos telefonos con un segundo de diferencia ya lo restan."
         % (DS3231_PPM, 2 * DS3231_PPM * 86400 / 1e6, presupuesto,
            presupuesto / (2 * DS3231_PPM * 86400 / 1e6)),
         "Es la cota que D-26 acepta y la regla (4) no la mejora: un salto dentro del margen "
         "se aplica directo. Lo que la acotaria es la 'cadena completa' que D-26 aparca."])

    # (iii) Y LA CADENCIA NO MANDA EL DEGRADADO A ROJO: una siembra NORMAL salta menos que
    # el umbral de D-26 (4). La misma desigualdad que el static_assert del Maestro, aqui
    # recalculada con la cadencia DEL ESP32 -el static_assert solo ve la copia del STM32-.
    umbral = _umbral_salto(fw)
    b.verificar(
        deriva1 + RESIDUO_SIEMBRA_S < umbral,
        "una siembra normal salta como mucho %d s (%d de HSI + %d del segundo entero) y el "
        "Degradado solo pasa por rojo por encima de %d s: la cadencia no para el cruce"
        % (deriva1 + RESIDUO_SIEMBRA_S, deriva1, RESIDUO_SIEMBRA_S, umbral),
        "una siembra normal puede saltar %d s y el umbral de D-26 (4) es %d: el Degradado "
        "pasaria por rojo -30 s y un verde perdido- en cada siembra"
        % (deriva1 + RESIDUO_SIEMBRA_S, umbral))

    # =====================================================================
    b.titulo("1.bis D-26 (4): el umbral del salto sale del despeje y es el margen")
    # =====================================================================
    # EL BORDE, ESCRITO: el umbral es el mayor salto MEDIDO que no puede llevar del verde
    # de una punta al de la otra sin rojo por medio. Con segundos enteros eso es el
    # despeje menos uno, y coincide con el aguante del barrido. Se comprueba que el C++ lo
    # ESCRIBE asi en las dos puntas -desde DEG_DESPEJE_SEG en el Maestro y desde el despeje
    # recibido en el Esclavo, que el Maestro manda tal cual-, no solo que valga 29 hoy.
    tm = fw.codigo(*DEG_M)
    te = fw.codigo(*DEG_E)
    formula_m = re.search(r"SALTO_SIN_ROJO_MAX_S\s*=\s*\(uint32_t\)DEG_DESPEJE_SEG\s*-\s*1UL\s*;",
                          tm) is not None
    cuerpo_e = _cuerpo(te, r"static\s+uint32_t\s+saltoSinRojoMaxS\s*\(\s*\)") or ""
    formula_e = (re.search(r"=\s*config_despejeSegundos\s*\(\s*\)\s*;", cuerpo_e) is not None
                 and re.search(r"despeje\s*-\s*1UL", cuerpo_e) is not None)
    b.verificar(
        formula_m and formula_e and umbral == aguante,
        "el umbral del salto es DEG_DESPEJE_SEG - 1 en el Maestro y config_despejeSegundos() "
        "- 1 en el Esclavo, y vale %d s: exactamente el desfase que el cruce aguanta" % umbral,
        "el umbral del salto no se deriva del despeje en las dos puntas (Maestro: %s, "
        "Esclavo: %s) o no coincide con el aguante (%s contra %d). Un umbral por encima deja "
        "pasar directo un salto que cruza el despeje entero; uno escrito a mano deja de "
        "seguirlo el dia que el despeje cambie" % (formula_m, formula_e, umbral, aguante))

    revisar = _cuerpo(sie, r"void\s+siembra_revisar\s*\(\s*\)")
    if revisar is None:
        raise fw.Abortado("no se hallo siembra_revisar() en %s/src/siembra.cpp" % ROL)
    mHora = re.search(r">=\s*SIEMBRA_INTERVALO_MS\s*\)\s*\{", revisar)
    b.verificar(
        mHora is not None and "siembra_ahora" in (_bloque(revisar, mHora.end() - 1) or "")
        and not re.search(r"\b%d\b" % esp, sie),
        "siembra_revisar() siembra cuando pasa SIEMBRA_INTERVALO_MS -la constante, no un "
        "numero tecleado- desde la ultima",
        "siembra_revisar() no dispara la siembra con `>= SIEMBRA_INTERVALO_MS`, o siembra.cpp "
        "lleva la cadencia tecleada como numero: la cuenta de arriba mediria un numero que "
        "el firmware no usa")

    # =====================================================================
    b.titulo("2. Los reintentos de arranque llegan con el STM32 ya escuchando")
    # =====================================================================
    r1 = fw.constante(CONTRATO, r"#define\s+SIEMBRA_REINTENTO_1_MS\s+(\d+)UL", "el reintento 1")
    r2 = fw.constante(CONTRATO, r"#define\s+SIEMBRA_REINTENTO_2_MS\s+(\d+)UL", "el reintento 2")
    peor, desgloses = 0, []
    for p in PUNTAS:
        setup = _cuerpo(fw.codigo(p, "src", "main.cpp"), r"void\s+setup\s*\(\s*\)")
        if setup is None:
            raise fw.Abortado("no se hallo setup() en %s/src/main.cpp" % p)
        lse = None
        if re.search(r"\breloj_setup\s*\(", setup[:max(0, setup.find("bluetooth_setup("))]):
            lse = fw.constante((p, "src", "reloj.cpp"), r"ESPERA_LSE_MS\s*=\s*(\d+)",
                               "la espera del cristal LSE del %s" % p)
        ms, desg = _arranque_stm32(setup, lse)
        if ms is None:
            raise fw.Abortado("no se pudo acotar el arranque del %s: %s" % (p, desg))
        peor = max(peor, ms)
        desgloses.append("%s >= %d ms (%s)" % (p, ms, desg))
    b.verificar(
        r1 > 2 * peor and r2 > r1 and r2 < esp,
        "REINTENTO_1 = %d ms > 2 x %d ms, lo minimo que tarda un STM32 en abrir J17 [%s]; "
        "REINTENTO_2 = %d ms va detras y antes de la cadencia" % (r1, peor, "; ".join(desgloses), r2),
        "los reintentos no cuadran con el arranque del STM32: REINTENTO_1 = %d, REINTENTO_2 "
        "= %d, CADENCIA = %d, y el STM32 tarda al menos %d ms en abrir J17 [%s]. La "
        "primera siembra sale casi seguro contra un puerto cerrado; si el reintento tambien, "
        "el STM32 se queda sin hora hasta la cadencia siguiente" % (r1, r2, esp, peor,
                                                                  "; ".join(desgloses)))

    b.verificar(
        re.search(r"REINTENTOS_MS\s*\[\s*\]\s*=\s*\{\s*SIEMBRA_REINTENTO_1_MS\s*,\s*"
                  r"SIEMBRA_REINTENTO_2_MS\s*\}", sie) is not None
        and "REINTENTOS_MS[reintentosHechos]" in revisar.replace(" ", ""),
        "siembra_revisar() usa los dos reintentos de contrato.h, en orden",
        "siembra.cpp no usa SIEMBRA_REINTENTO_1_MS y _2_MS como calendario de arranque: la "
        "desigualdad de arriba mediria unos numeros que el firmware no usa")

    # =====================================================================
    b.titulo("3. Los disparadores tienen llamador (declarar no es ejercer)")
    # =====================================================================
    setupE = _cuerpo(main, r"void\s+setup\s*\(\s*\)") or ""
    loopE = _cuerpo(main, r"void\s+loop\s*\(\s*\)")
    if loopE is None:
        raise fw.Abortado("no se hallo loop() en %s/src/main.cpp" % ROL)
    b.verificar(
        "siembra_revisar" in loopE and "siembra_revisar" not in setupE
        and loopE.find("reloj_revisar") < loopE.find("siembra_revisar"),
        "siembra_revisar() se llama en loop(), detras de reloj_revisar(), y NO en setup(): "
        "en setup() no se pone un byte en ningun cable (6.4)",
        "siembra_revisar() no esta en loop(), o esta en setup(), o va antes de "
        "reloj_revisar(). Sin llamador la siembra periodica es codigo muerto -N-73-; en "
        "setup() seria un saludo contra un STM32 que aun no escucha")

    b.verificar(
        len(re.findall(r"\bsiembra_ahora\s*\(", desp)) == 1,
        "siembra_ahora() tiene su llamador en el despachador: la siembra inmediata tras "
        "SET_RTC existe",
        "siembra_ahora() no se llama desde despachador.cpp (o se llama mas de una vez): tras "
        "un SET_RTC el STM32 esperaria hasta una cadencia entera a recibir la hora nueva")

    # =====================================================================
    b.titulo("4. Tras SET_RTC: se siembra solo lo aceptado, y el $ACK lo dice")
    # =====================================================================
    atender = _cuerpo(desp, r"void\s+%s\s*\([^)]*\)" % re.escape(ATENDER))
    if atender is None:
        raise fw.Abortado("no se hallo %s() en %s/src/despachador.cpp" % (ATENDER, ROL))
    ok, det = _siembra_tras_set_rtc(atender)
    b.verificar(
        ok,
        "SET_RTC: %s" % det,
        "SET_RTC: %s. Es la barrera del $ACK (CLAUDE.md 2): un 'hora puesta' que no "
        "depende de lo que devolvio la siembra deja al tecnico creyendo que el cruce esta "
        "en hora" % det)

    # =====================================================================
    b.titulo("5. ANTI-SUPLANTACION: la linea de hora solo la origina el puente")
    # =====================================================================
    mFmt = re.search(r'static\s+const\s+char\s+%s\[\]\s*=\s*"(CMD:([A-Z0-9_]+):[^"]*)"'
                     % FORMATO, sie)
    if mFmt is None:
        raise fw.Abortado("no se hallo %s = \"CMD:...\" en %s/src/siembra.cpp" % (FORMATO, ROL))
    literal, token = mFmt.group(1), mFmt.group(2)
    cuerpoPred = _cuerpo(desp, r"bool\s+%s\s*\([^)]*\)" % re.escape(PREDICADO))
    if cuerpoPred is None:
        raise fw.Abortado("no se hallo %s() en %s/src/despachador.cpp" % (PREDICADO, ROL))
    marcas = [f for f, l in _contenidos(desp, cuerpoPred).items() if l == token]
    b.verificar(
        len(marcas) == 1,
        "el predicado se queda toda linea del telefono que CONTENGA %r -el nombre de la "
        "orden de siembra, leido de %s-: no cruza (%s)" % (token, FORMATO, marcas[0] if marcas else "?"),
        "el predicado NO se queda las lineas del telefono con %r dentro (criterios leidos: "
        "%s). Entonces cualquiera con Bluetooth escribe CMD:%s:... y pone la hora del cruce "
        "SIN PIN: el STM32 no puede pedirlo, porque el puente no lo conoce" %
        (token, _contenidos(desp, cuerpoPred), token))

    if marcas:
        ok, det = _suplantacion_primero(atender, marcas[0], token)
    else:
        ok, det = False, "sin criterio de marca no hay rama que mirar"
    b.verificar(
        ok,
        "la anti-suplantacion %s: una linea que traiga HORA_ESP32 y SET_RTC a la vez se "
        "trata por lo que puede hacer dano" % det,
        "la anti-suplantacion no va primero o no contesta: %s. Una orden que desaparece "
        "sin respuesta se lee como equipo colgado; una que se trata como SET_RTC por ir "
        "detras, cruza con otro nombre" % det)

    # =====================================================================
    b.titulo("6. La linea cabe en el buffer del STM32, con la cota DERIVADA")
    # =====================================================================
    maximos, minimos = [], []
    for campo in ("ANIO", "MES", "DIA", "HORA", "MIN", "SEG"):
        lo = fw.constante(CONTRATO, r"#define\s+RTC_%s_MIN\s+(\d+)" % campo, "RTC_%s_MIN" % campo)
        hi = fw.constante(CONTRATO, r"#define\s+RTC_%s_MAX\s+(\d+)" % campo, "RTC_%s_MAX" % campo)
        if lo < 0:
            raise fw.Abortado("RTC_%s_MIN es negativo: la cota de abajo no sabe de signos" % campo)
        maximos.append(hi)
        minimos.append(lo)
    largo = _largo_maximo(literal, maximos)
    if largo is None:
        raise fw.Abortado("el formato %r tiene conversiones que la cota no sabe acotar" % literal)
    tope = fw.constante(CONTRATO, r"#define\s+TRAMA_MAX_UTIL\s+(\d+)", "TRAMA_MAX_UTIL")
    bufs = {p: fw.constante((p, "src", "bluetooth.cpp"), r"static\s+char\s+btBufIn\[(\d+)\]",
                            "btBufIn del %s" % p) for p in PUNTAS}
    b.verificar(
        largo <= tope and all(largo <= n - 1 for n in bufs.values())
        and re.search(r"char\s+linea\s*\[\s*TRAMA_MAX_UTIL\s*\+\s*1\s*\]", sie) is not None,
        "la siembra mide como mucho %d caracteres (rangos de contrato.h) <= %d utiles del "
        "puente y <= btBufIn-1 de las dos puntas (%s)" % (largo, tope, bufs),
        "la siembra puede medir %d caracteres y el tope es %d (puente) / %s (STM32). El "
        "STM32 TRUNCA en silencio y compara la linea recortada: una hora cortada que casa "
        "con el prefijo es una hora mutilada sembrada sin aviso" % (largo, tope, bufs))

    # =====================================================================
    b.titulo("6.bis La costura de FORMATO: lo que el ESP32 compone, el STM32 lo acepta")
    # =====================================================================
    # EL CRUCE QUE FALTABA ENTRE LOS DOS BINARIOS. La 6 mide la LONGITUD; esto mide la
    # FORMA. El ESP32 compone con su literal y sus rangos; el STM32 rechaza por su
    # PATRON_ISO y por su regla de rango. Si difieren en un borde -un %d donde el patron
    # pide dos cifras, un rango de dia que el STM32 no admite-, la siembra de ESE borde se
    # rechaza en cada cadencia y la alarma de D-26 (5) acusa al circuito sano.
    #
    # EL BORDE, ESCRITO: se componen las 64 combinaciones de minimo y maximo de los seis
    # campos de contrato.h -la forma de un %0Nd solo cambia en los extremos de su rango- con
    # el literal del ESP32 y el `%` de Python, que para enteros no negativos y %0Nd hace lo
    # mismo que snprintf. Lo que queda tras el prefijo se pasa por el patron LEIDO de cada
    # reloj.cpp y por la regla de rango LEIDA de su reloj_ajustarConAcuse() (hora, minuto,
    # segundo y dia: el anio y el mes el STM32 los descarta).
    patrones = {}
    rangos = {}
    for p in PUNTAS:
        rc = fw.codigo(*RELOJ_C[p])
        mp = re.search(r'\bPATRON_ISO\s*\[\s*\]\s*=\s*"([^"]*)"', rc)
        if mp is None:
            raise fw.Abortado("%s/src/reloj.cpp no tiene PATRON_ISO: sin el no se sabe que "
                              "forma acepta el receptor de la siembra" % p)
        patrones[p] = mp.group(1)
        rg = {}
        for var in ("hora", "minuto", "segundo", "dia"):
            mr = re.search(r"if\s*\(\s*%s\s*<\s*(\d+)\s*\|\|\s*%s\s*>\s*(\d+)\s*\)\s*return\s+false"
                           % (var, var), rc)
            if mr is None:
                raise fw.Abortado("no se leyo la regla de rango de `%s` en %s/src/reloj.cpp"
                                  % (var, p))
            rg[var] = (int(mr.group(1)), int(mr.group(2)))
        rangos[p] = rg
    fueras = []
    compuestas = 0
    for i in range(64):
        vals = tuple(maximos[k] if (i >> k) & 1 else minimos[k] for k in range(6))
        linea = literal % vals
        compuestas += 1
        if not linea.startswith(("CMD:%s:" % token)):
            fueras.append((linea, "no empieza por el prefijo"))
            continue
        cola = linea[len(("CMD:%s:" % token)):]
        anio, mes, dia, h, mi, s = vals
        for p in PUNTAS:
            rg = rangos[p]
            if not _patron_ok(patrones[p], cola):
                fueras.append((linea, "%s: no casa PATRON_ISO %r" % (p, patrones[p])))
            elif not (rg["hora"][0] <= h <= rg["hora"][1] and rg["minuto"][0] <= mi <= rg["minuto"][1]
                      and rg["segundo"][0] <= s <= rg["segundo"][1]
                      and rg["dia"][0] <= dia <= rg["dia"][1]):
                fueras.append((linea, "%s: fuera de su regla de rango %s" % (p, rg)))
    b.verificar(
        not fueras,
        "las %d lineas de borde que el ESP32 puede componer (%r) casan el PATRON_ISO de las "
        "dos puntas (%s) y su regla de rango: ninguna siembra buena acaba en alarma"
        % (compuestas, literal, patrones),
        "%d linea(s) que el ESP32 SI compone no las acepta el STM32, empezando por %r (%s). "
        "Esa siembra se rechaza en cada cadencia y D-26 (5) alarma con el circuito sano"
        % (len(fueras), fueras[0][0] if fueras else "", fueras[0][1] if fueras else ""))

    # =====================================================================
    b.titulo("7. 🔴 La costura: las dos puntas del STM32 la reconocen ANTES del PIN")
    # =====================================================================
    prefijo = "CMD:%s:" % token
    for p in PUNTAS:
        ok, det = _receptor_antes_del_pin(fw.codigo(p, "src", "bluetooth.cpp"), prefijo)
        if ok is None:
            raise fw.Abortado("%s/src/bluetooth.cpp: %s" % (p, det))
        b.verificar(
            ok,
            "el %s reconoce %r en procesarComando() %s" % (p, prefijo, det),
            "el %s NO reconoce %r antes de su guarda de PIN: %s. Cada siembra -en cada "
            "arranque y cada cadencia- caeria en $ERR,CMD:AUTH_FAILED,DESC:PIN_INVALIDO, que "
            "sube a la app en ROJO acusando al operario de una clave que no tecleo. Es la "
            "otra mitad de D-20 y vive en el firmware del STM32" % (p, prefijo, det))

    # =====================================================================
    b.titulo("LO QUE ESTE LADO NO PUEDE CERRAR (no cuenta)")
    # =====================================================================
    # 11/09: ERA UNA ACUSACION -"decision del responsable, no de este pack"- Y EL
    # RESPONSABLE DECIDIO. Se queda como NOTA que cita la fila, no se borra: el dia que
    # D-26 (1) cambie, este es el sitio donde alguien vera que la hora se pone sin PIN.
    b.reportar(
        "D-26 (1): la hora del controlador se pone SIN PIN - RIESGO ACEPTADO",
        ["El SET_RTC del telefono ya no cruza: lo atiende el puente, que busca 'SET_RTC:' "
         "dentro de la linea porque NO CONOCE el PIN -esp32_09 exige que no aparezca en su "
         "fuente-. 'SET_RTC:AAAA-MM-DD,hh:mm:ss' por Bluetooth, con cualquier PIN o sin el, "
         "pone el DS3231 y la siembra lleva esa hora al STM32 (y el Maestro la propaga al "
         "Esclavo). La anti-suplantacion de HORA_ESP32 no cambia esto.",
         "ACEPTADO por el responsable el 11/09 (DECISIONES.md, D-26 (1)): 'hace falta tener "
         "la app, y la app no la maneja cualquiera que este en la via'. Lo que lo acota: un "
         "salto de hora en Degradado pasa por rojo (D-26 (4))."])
    b.reportar(
        "La siembra no sabe si el STM32 la acepto",
        ["`true` es 'la linea salio entera por J17'. Un STM32 que no escucha -arrancando, "
         "colgado- la pierde sin que el puente lo sepa; lo cubren los reintentos, la cadencia "
         "siguiente y, del lado del STM32, la alarma de D-26 (5) si no llega ninguna buena en "
         "tres cadencias. No un acuse.",
         "Y la hora va en SEGUNDOS ENTEROS: la siembra puede llegar hasta ~1 s por detras de "
         "la hora del DS3231 (el segundo en curso se trunca). No acumula: cada siembra "
         "sobreescribe."])

    # =====================================================================
    b.titulo("CONTROLES NEGATIVOS")
    # =====================================================================
    # Con los MISMOS patrones de la 1 sobre textos sinteticos: si el patron del STM32 no
    # leyera el numero -o leyera el de otra constante-, esto no distinguiria nada.
    contratoFalso = "#define SIEMBRA_INTERVALO_MS       600000UL\n"
    relojFalso = "static const unsigned long HORA_ESP32_CADENCIA_MS = 300000UL;\n"
    mE = re.search(RE_SIEMBRA_ESP32, contratoFalso)
    mM = re.search(RE_CADENCIA_STM32, relojFalso)
    b.control_negativo(
        mE is not None and mM is not None and int(mE.group(1)) != int(mM.group(1)),
        "una cadencia de 10 min en el ESP32 contra los 5 que espera el STM32 se lee con los "
        "mismos patrones de la 1 y sale distinta: compara valores leidos, no nombres")

    # LA RELACION DE LA 1 TIENE QUE SABER CAER, Y CON EL NUMERO QUE SE CORRIGIO: con la
    # cadencia de A-15 (una hora) la misma cuenta supera el aguante.
    derivaA15 = 2 * _deriva_cadencia_s(3600000, hsi) + 2 * RESIDUO_SIEMBRA_S
    b.control_negativo(
        derivaA15 >= aguante,
        "con la cadencia de A-15 -una hora- la misma cuenta da %d s de separacion posible "
        "contra %d de aguante: la 1 caza la cadencia que D-26 corrigio" % (derivaA15, aguante))

    b.control_negativo(
        _aguante(DEG_VERDE_SEG, 5, E_AMARILLO_MS // 1000) < aguante,
        "con un despeje de 5 s el mismo barrido da menos aguante: el margen de la 1 sale del "
        "ciclo, no de una cifra copiada")

    b.control_negativo(
        not _patron_ok(patrones["Maestro"], "2026-1-05,08:00:00")
        and not _patron_ok(patrones["Maestro"], "2026-01-05,08:00:0")
        and not _patron_ok(patrones["Maestro"], "2026-01-05,08:00:00X")
        and _patron_ok(patrones["Maestro"], "2026-01-05,08:00:00"),
        "la 6.bis distingue: un mes de una cifra -un %d donde iba %02d-, un segundo cortado "
        "y basura detras NO casan el patron, y la linea bien formada si")

    setupLento = "botones_setup(); delay(9000); reloj_setup(); bluetooth_setup();"
    msL, _ = _arranque_stm32(setupLento, 2000)
    b.control_negativo(
        msL == 11000 and not (r1 > 2 * msL),
        "un STM32 con delay(9000) en el arranque deja al REINTENTO_1 corto: la 2 lee los "
        "delay() y la espera del LSE, y reacciona")

    b.control_negativo(
        _largo_maximo(literal + ",RELLENO_QUE_NO_CABE_EN_UNA_LINEA_DE_SESENTA_Y_TRES",
                      maximos) > tope,
        "un formato alargado sale por encima del tope: la 6 cuenta el literal, no lo da "
        "por bueno")

    malSiembra = ('{ ResultadoReloj r = reloj_ajustar(&f); siembra_ahora(); '
                  'if (r == RELOJ_ERR_RANGO) { e("$ERR"); } '
                  'else if (r != RELOJ_OK) { e("$ERR"); } '
                  'else { FechaHora l; if (!reloj_leer(&l)) { e("$ERR"); } '
                  'else { e("$ACK,NODE:PUENTE,CMD:SET_RTC,RESULT:OK,"); } } }')
    b.control_negativo(
        not _siembra_tras_set_rtc(malSiembra)[0],
        "una siembra ANTES de mirar lo que devolvio reloj_ajustar() se caza: es 3.16-C, el "
        "STM32 sembrado con una hora que el DS3231 rechazo")

    ackMudo = ('{ ResultadoReloj r = reloj_ajustar(&f); '
               'if (r == RELOJ_ERR_RANGO) { e("$ERR"); } '
               'else if (r != RELOJ_OK) { e("$ERR"); } '
               'else { FechaHora l; if (!reloj_leer(&l)) { e("$ERR"); } '
               'else { siembra_ahora(); e("$ACK,NODE:PUENTE,CMD:SET_RTC,RESULT:OK,"); } } }')
    b.control_negativo(
        not _siembra_tras_set_rtc(ackMudo)[0],
        "una siembra dentro del RELOJ_OK pero con el $ACK OK sin mirar lo que devolvio se "
        "caza: 'hora puesta' con la linea sin salir es la mentira con formato de exito")

    bien = ('{ ResultadoReloj r = reloj_ajustar(&f); '
            'if (r == RELOJ_ERR_RANGO) { e("$ERR"); } '
            'else if (r != RELOJ_OK) { e("$ERR"); } '
            'else { FechaHora l; if (!reloj_leer(&l)) { e("$ERR"); } '
            'else if (!siembra_ahora()) { e("$ACK,X,RESULT:HORA_PUESTA_SIN_PROPAGAR,"); } '
            'else { e("$ACK,NODE:PUENTE,CMD:SET_RTC,RESULT:OK,"); } } }')
    b.control_negativo(
        _siembra_tras_set_rtc(bien)[0],
        "y la forma buena del mismo sintetico PASA la 4: el detector distingue, no acusa a "
        "todo el que siembra")

    tarde = ('{ if (strcmp(linea, C) == 0) { } const char* p = accionSetRtc(linea); '
             'if (marca(linea)) { e("$ERR,NODE:PUENTE,CMD:HORA_ESP32,D"); return; } }')
    b.control_negativo(
        not _suplantacion_primero(tarde, "marca", "HORA_ESP32")[0],
        "una anti-suplantacion colocada DETRAS de la consulta y de SET_RTC se caza: tiene "
        "que ser la primera rama")

    stmTarde = ('static void procesarComando(const char* cmd) { '
                'if (strncmp(cmd, "CMD:PIN:1234:", 13) == 0) { } '
                'if (strncmp(cmd, "CMD:HORA_ESP32:", 15) == 0) { } }')
    stmBien = ('static void procesarComando(const char* cmd) { '
               'if (strncmp(cmd, "CMD:HORA_ESP32:", 15) == 0) { return; } '
               'if (strncmp(cmd, "CMD:PIN:1234:", 13) == 0) { } }')
    b.control_negativo(
        _receptor_antes_del_pin(stmTarde, "CMD:HORA_ESP32:")[0] is False
        and _receptor_antes_del_pin(stmBien, "CMD:HORA_ESP32:")[0] is True,
        "un receptor del STM32 colocado DETRAS de la guarda de PIN se caza y uno DELANTE "
        "pasa: la 7 mide la posicion, no la presencia")
