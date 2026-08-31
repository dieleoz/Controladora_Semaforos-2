# ===== banco/packs/esp32_04_osf.py =====
#
# LA HORA NACE NO FIABLE, Y EL BIT QUE LO DICE NO SE BORRA POR PROLIJIDAD.
#
# EJERCE SFTY-18: la barrera "se cuando NO tengo hora" del reloj del puente.
#
# POR QUE ESTE PACK EXISTE.
#
# OPTIMIZACIONES.md:72, sobre el reloj del STM32: "La regla de seguridad no es tener
# reloj, es saber cuando no se tiene [...] Un reloj sin poner en hora que se cree valido
# es peor que no tener reloj: activaria la operacion nocturna a deshora." Alli eso costo
# inventar un ano marcador y comprobar que sobreviviera al apagado.
#
# El DS3231 lo trae de fabrica: el OSF -oscillator stop flag, bit 7 del registro 0x0F-
# se pone a 1 en cuanto el oscilador se para en algun momento. Sale gratis, Y HAY QUE
# COGERLO: un DS3231 sin pila devuelve una hora PERFECTAMENTE FORMADA y completamente
# falsa. "Bien formada" no es "cierta", y el OSF es lo unico que las distingue.
#
# LAS CUATRO REGLAS, Y LA QUE MAS FACIL SE INCUMPLE POR ORDEN Y LIMPIEZA:
#
#   R-1  el OSF se lee EN EL ARRANQUE, antes de publicar ninguna hora.
#   R-2  una hora con OSF puesto se declara NO FIABLE aunque la fecha parezca razonable.
#   R-3  🔴 el OSF se limpia SOLO tras una escritura confirmada, NUNCA en el arranque
#        "para dejarlo limpio". Limpiarlo sin poner la hora es FABRICAR UNA
#        AUTORIZACION: el bit deja de decir la verdad y nadie se entera.
#   R-4  se relee periodicamente. La pila se puede agotar con el equipo en marcha, y una
#        hora que dejo de ser fiable a las tres de la manana no avisa sola.
#
# Y LA QUINTA, QUE NO ES DEL DS3231 SINO DE N-73: la funcion "tengo hora?" tiene que
# tener LLAMADORES. Una funcion declarada, documentada con ejemplo y sin un solo llamador
# es la Caja Negra de Alarmas otra vez: cuatro manuales la describian, nadie la llamaba, y
# se pago cuando un fallo de campo no se pudo diagnosticar porque no habia registro que
# mirar. El censo es grep de la declaracion contra las llamadas, no lectura.

import re

NOMBRE = "esp32_04_osf"
DESCRIPCION = "el OSF decide si la hora del puente es fiable, y solo se limpia tras escribir"

ROL = "ESP32_Expansion"


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


def _llama(codigo, nombre):
    """True si ese texto INVOCA la funcion, no si solo la nombra.

    La distincion es toda la regla de N-73: una declaracion en el .h y la propia
    definicion en el .cpp mencionan el nombre y no son llamadas. Un censo que las
    contara diria que la Caja Negra de Alarmas tenia dos llamadores."""
    for m in re.finditer(r"\b%s\s*\(\s*\)" % re.escape(nombre), codigo):
        antes = codigo[max(0, m.start() - 40):m.start()]
        # "bool reloj_enHora()" -> declaracion o definicion, no llamada.
        if re.search(r"\b(?:bool|void|int|static|inline)\s+$", antes):
            continue
        return True
    return False


def correr(b, fw):
    b.titulo("El bit OSF: leido al arrancar, decisivo, y limpiado solo tras escribir")

    contrato = ("ESP32_Expansion", "include", "contrato.h")
    reloj = fw.codigo("ESP32_Expansion", "src", "reloj_ds3231.cpp")

    # ---- 1. Las constantes del chip se releen del C++ -------------------------
    bit = fw.constante(contrato, r"#define\s+DS3231_BIT_OSF\s+0x([0-9A-Fa-f]+)",
                       "el bit OSF del DS3231", base=16)
    reg = fw.constante(contrato, r"#define\s+DS3231_REG_ESTADO\s+0x([0-9A-Fa-f]+)",
                       "el registro de estado del DS3231", base=16)
    relectura = fw.constante(contrato, r"#define\s+RELOJ_RELECTURA_MS\s+(\d+)UL",
                             "el periodo de relectura del OSF")

    b.verificar(
        bit == 0x80 and reg == 0x0F,
        "el OSF se lee del registro 0x%02X con la mascara 0x%02X, que es lo que dice el "
        "datasheet del DS3231" % (reg, bit),
        "el OSF esta declarado en el registro 0x%02X con mascara 0x%02X. El datasheet "
        "pone el oscillator-stop flag en el bit 7 del registro 0x0F: con otra pareja se "
        "estaria leyendo otra cosa -alarmas, control- y llamandola fiabilidad de la hora"
        % (reg, bit))

    # ---- 2. R-1: se lee en el arranque ----------------------------------------
    setup = _cuerpo(reloj, r"void\s+reloj_setup\s*\(\s*\)")
    if setup is None:
        raise fw.Abortado(
            "no se hallo reloj_setup() en %s/src/reloj_ds3231.cpp. R-1 dice que el OSF "
            "se lee en el arranque; sin poder leer el arranque no hay nada que medir"
            % ROL)
    b.verificar(
        "revisarOsf" in setup or "DS3231_REG_ESTADO" in setup,
        "R-1: el arranque lee el OSF antes de publicar ninguna hora",
        "reloj_setup() NO lee el OSF. La barrera arrancaria sin haber preguntado nunca "
        "si el oscilador se paro, y una hora bien formada de un modulo sin pila entraria "
        "como valida")

    # ---- 3. R-3: 🔴 EL ARRANQUE NO LIMPIA EL BIT ------------------------------
    #
    # Es la regla que mas facil se incumple, porque incumplirla parece prolijidad.
    limpieza = r"&\s*~\s*DS3231_BIT_OSF"
    b.verificar(
        re.search(limpieza, setup) is None,
        "R-3: el arranque NO limpia el OSF",
        "reloj_setup() LIMPIA EL OSF. Limpiarlo sin haber puesto la hora es fabricar una "
        "autorizacion: el bit deja de decir la verdad, el modulo declara fiable una "
        "fecha inventada, y nadie tiene ya con que enterarse")

    ajustar = _cuerpo(reloj, r"ResultadoReloj\s+reloj_ajustar\s*\([^)]*\)")
    if ajustar is None:
        raise fw.Abortado(
            "no se hallo reloj_ajustar() en %s/src/reloj_ds3231.cpp: es el unico sitio "
            "donde el OSF puede limpiarse legitimamente, y sin leerlo este pack no "
            "puede comprobar que la limpieza vive ahi" % ROL)

    iLimpieza = ajustar.find("~ DS3231_BIT_OSF")
    if iLimpieza < 0:
        m = re.search(limpieza, ajustar)
        iLimpieza = m.start() if m else -1
    iEscritura = ajustar.find("escribirReg(DS3231_REG_HORA")
    b.verificar(
        iLimpieza > 0 and iEscritura >= 0 and iLimpieza > iEscritura,
        "R-3: el OSF se limpia dentro de reloj_ajustar() y DESPUES de la escritura de "
        "la hora, no antes",
        "la limpieza del OSF no va detras de la escritura de la hora (limpieza=%d, "
        "escritura=%d). Borrar el bit antes de escribir deja una ventana en la que el "
        "modulo se declara fiable sin tener todavia nada dentro"
        % (iLimpieza, iEscritura))

    # ---- 4. R-2: el OSF DECIDE, no acompana -----------------------------------
    #
    # No basta con leerlo: la barrera tiene que apagarse cuando el bit esta puesto. Se
    # comprueba que la funcion que lo evalua pone la bandera a false en esa rama.
    revisar = _cuerpo(reloj, r"static\s+void\s+revisarOsf\s*\(\s*\)")
    if revisar is None:
        raise fw.Abortado(
            "no se hallo revisarOsf() en %s/src/reloj_ds3231.cpp: es donde el bit se "
            "convierte en decision, que es la propiedad de R-2" % ROL)
    m = re.search(r"if\s*\(\s*estado\s*&\s*DS3231_BIT_OSF\s*\)\s*\{([^}]*)\}", revisar)
    b.verificar(
        m is not None and re.search(r"sePuso\s*=\s*false", m.group(1)) is not None,
        "R-2: con el OSF puesto la barrera se apaga -sePuso = false-, aunque los "
        "registros traigan una fecha con pinta razonable",
        "el OSF se lee y NO decide: no hay una rama que apague la barrera cuando el bit "
        "esta puesto. Un DS3231 sin pila devuelve una hora perfectamente formada y "
        "completamente falsa, y sin esta rama entraria como buena")

    # ---- 5. R-4: se relee periodicamente --------------------------------------
    revisarPub = _cuerpo(reloj, r"void\s+reloj_revisar\s*\(\s*\)")
    b.verificar(
        revisarPub is not None and "RELOJ_RELECTURA_MS" in revisarPub
        and "revisarOsf" in revisarPub,
        "R-4: el OSF se relee cada %d ms, no solo al arrancar" % relectura,
        "no hay relectura periodica del OSF. La pila se puede agotar con el equipo en "
        "marcha: un reloj que solo se comprueba al arrancar declara fiable para siempre "
        "una hora que dejo de serlo")

    main = fw.codigo("ESP32_Expansion", "src", "main.cpp")
    loop = _cuerpo(main, r"void\s+loop\s*\(\s*\)")
    b.verificar(
        loop is not None and "reloj_revisar" in loop,
        "R-4: y alguien la llama -loop() la invoca cada vuelta-",
        "reloj_revisar() existe y NADIE LA LLAMA. Es la mitad silenciosa de la prueba "
        "muerta: una funcion de seguridad declarada, documentada y sin llamador. La "
        "relectura no ocurriria nunca y el pack de arriba seguiria en verde")

    # ---- 6. N-73: la barrera tiene llamadores ---------------------------------
    #
    # El censo es grep de la declaracion contra las llamadas, en TODO el arbol del rol.
    # Una funcion "tengo hora?" sin llamadores es la Caja Negra de Alarmas: declarada,
    # documentada con ejemplo, y sin nadie que la use.
    llamadores = []
    for carpeta, ext in (("src", ".cpp"), ("include", ".h")):
        for f in fw.fuentes_de("ESP32_Expansion", carpeta, ext):
            if _llama(fw.codigo("ESP32_Expansion", carpeta, f), "reloj_enHora"):
                llamadores.append("%s/%s" % (carpeta, f))

    b.verificar(
        bool(llamadores),
        "reloj_enHora() tiene llamador de verdad en %s: la barrera es la que se usa, no "
        "una convencion escrita al lado" % ", ".join(sorted(set(llamadores))),
        "reloj_enHora() esta DECLARADA, DEFINIDA, DOCUMENTADA Y SIN UN SOLO LLAMADOR. Es "
        "N-73 literal: la Caja Negra de Alarmas que cuatro manuales describian y que "
        "nadie invocaba. Una barrera que nadie cruza no protege de nada, y encima "
        "aparenta cobertura")

    # ---- CONTROLES NEGATIVOS -------------------------------------------------
    setupMalo = "{ Wire.begin(21,22); uint8_t e=0; e = e & ~ DS3231_BIT_OSF; }"
    b.control_negativo(
        re.search(limpieza, setupMalo) is not None,
        "un arranque que limpiara el OSF 'para dejarlo limpio' se detecta")

    revisarMalo = "{ if (estado & DS3231_BIT_OSF) { motivo = X; } sePuso = true; }"
    mm = re.search(r"if\s*\(\s*estado\s*&\s*DS3231_BIT_OSF\s*\)\s*\{([^}]*)\}",
                   revisarMalo)
    b.control_negativo(
        mm is not None and re.search(r"sePuso\s*=\s*false", mm.group(1)) is None,
        "un OSF que se lee, se anota y NO apaga la barrera se detecta: leerlo no es "
        "usarlo")

    b.control_negativo(
        not _llama("bool reloj_enHora();", "reloj_enHora")
        and _llama("if (!reloj_enHora()) return false;", "reloj_enHora"),
        "el censo distingue DECLARAR de LLAMAR: un .h que solo declara la barrera no la "
        "cuenta como usada, y un `if (!reloj_enHora())` si")
