# ===== banco/packs/app_05_sin_exito_mudo.py =====
#
# LA APP NO ANUNCIA EN PASADO LO QUE NO SABE SI PASO. Es app_03 con las puntas
# cambiadas: alli el que mentia era el firmware contestando $ACK sin mirar; aqui el
# que miente es el telefono, y el tecnico se va del poste igual.
#
# LA PROPIEDAD, EN UNA LINEA: ningun bloque que llame a enviarComandoFirmware() puede
# anunciar el resultado -en verde o en pasado- si no puede saber siquiera que la orden
# SALIO.
#
# EL CASO REAL, MEDIDO EL 31/08, QUE ES POR LO QUE ESTE PACK EXISTE.
#
# enviarComandoFirmware() se planta y NO ESCRIBE UN BYTE cuando falta el PIN:
#
#     if (!SIN_PIN.includes(comando) && !state.pinVerificado) { ...; return; }
#
# Tres manejadores llamaban y seguian de largo, imprimiendo su linea de exito EN VERDE
# Y EN PASADO justo debajo:
#
#     enviarComandoFirmware('SET_RTC', `${today},${now}`);
#     showToast(`Reloj RTC DS3231 sincronizado a las ${now}`);
#     addEvent('green', `Reloj del Semaforo ajustado con el celular: ${now}.`);
#
# Ninguno de los tres comandos -SET_RTC dos veces y SET_TIEMPOS- esta en SIN_PIN, asi
# que sin PIN verificado la app anunciaba tres cosas que no habia enviado. Y no eran
# tres anuncios cualesquiera: uno de ellos es el reloj, que es lo que autoriza el Modo
# Degradado.
#
# LAS DOS MITADES DE LA PROPIEDAD, Y POR QUE HACEN FALTA LAS DOS.
#
#   (a) EL VERDE ES DEL EQUIPO, NO DE LA APP. En este fichero el verde lo emite la
#       rama de $ACK -"Equipo: orden [X] ACEPTADA"-, o sea que significa "el equipo
#       dijo que si". Un addEvent('green', ...) escrito por el mismo bloque que manda
#       la orden le pone al telefono el color con el que habla el poste. Esto no se
#       da por supuesto: se COMPRUEBA leyendo la rama de $ACK, y si esa rama dejara de
#       usar el verde este pack aborta en vez de seguir midiendo con una premisa vieja.
#
#   (b) ANUNCIAR DESPUES DE MANDAR SOLO VALE SI EL BLOQUE NO PUEDE QUEDARSE MUDO. Hay
#       tres formas de que no pueda, y las tres estan en el fichero: mirar el bool que
#       devuelve el envio, poner la guarda de PIN delante, o mandar un comando de los
#       que el firmware acepta sin PIN. Si no hay ninguna, el anuncio es una promesa
#       sobre algo que puede no haber ocurrido.
#
# LA REFERENCIA DE LO BIEN HECHO YA ESTABA EN EL MISMO FICHERO, y este pack la mide
# para saber distinguir en vez de solo acusar: la botonera de campo del operario, que
# tiene la doctrina escrita encima -"PULSAR UN BOTON NO ES SABER QUE EL EQUIPO
# OBEDECIO"-, pone la guarda de PIN delante y dice "orden enviada" en cyan. Si el
# detector marcara ESA como defectuosa, ninguna de sus acusaciones valdria nada.
#
# NADA DE ESTO SE LEE DE UNA LISTA ESCRITA A MANO salvo el vocabulario de pasado, que
# va declarado abajo palabra por palabra con su motivo -es una lista sobre el espanol,
# no sobre el firmware, y no se puede releer de ningun .cpp-.
#
# SOBRE LAS ETIQUETAS SFTY: este pack NO lleva ninguna. Vigila la honestidad de una
# interfaz, no una barrera del equipo.

import re

NOMBRE = "app_05_sin_exito_mudo"
DESCRIPCION = "ningun bloque de la app anuncia en verde o en pasado una orden que quiza no salio"

APP_JS = ("05_Funcional", "App_Semaforo", "app.js")

ENVIO = "enviarComandoFirmware"

# La rama que se usa como PATRON DE LO BIEN HECHO, por el literal de su comando. Si
# desaparece del fuente, este pack se queda sin calibrar y ABORTA.
REFERENCIA_BUENA = "SET_MODO"

# EL VOCABULARIO DE PASADO, DECLARADO PALABRA POR PALABRA CON SU MOTIVO.
#
# La regla para admitir una: dicha por la app justo despues de mandar una orden,
# afirma algo que solo el equipo puede saber. No entran los verbos del ENVIO
# -"enviada", "mandada"-, que es lo unico que la app sabe de verdad y es justo como
# esta escrita la botonera de campo.
PASADO = {
    "sincroniz": "afirma que el equipo quedo en hora; eso lo dice su $ACK",
    "ajustad":   "igual: el ajuste puede haberse rechazado por SIN_CRISTAL",
    "guardad":   "afirma que el equipo grabo los tiempos; SET_TIEMPOS tiene $ERR",
    "aplicad":   "afirma que ya rige el valor nuevo",
    "exitos":    "afirma el resultado sin haberlo recibido",
    "con éxito": "lo mismo, en su otra forma",
    "correctamente": "adverbio de resultado: solo el equipo sabe si fue correcto",
    "realizad":  "afirma que la maniobra ocurrio en el poste",
    "ejecutad":  "idem",
    "completad": "idem",
}


def _sin_comentarios(js):
    """El JavaScript sin comentarios, por el motivo de fuente.codigo().

    Aqui es imprescindible en las dos direcciones: los comentarios de este fichero
    CITAN el codigo defectuoso que se retiro -incluido un addEvent('green', ...) de
    ejemplo-, asi que leerlos como codigo acusaria de un defecto ya arreglado; y a la
    vez explican la doctrina con palabras en pasado que dispararian la lista de
    arriba."""
    js = re.sub(r"/\*.*?\*/", " ", js, flags=re.S)
    js = re.sub(r"(?m)//[^\n]*", " ", js)
    return js


def _bloque(texto, i):
    prof = 0
    for j in range(i, len(texto)):
        if texto[j] == "{":
            prof += 1
        elif texto[j] == "}":
            prof -= 1
            if prof == 0:
                return i, j
    return None


def _bloque_que_contiene(js, pos):
    """(inicio, fin) del bloque de llaves mas interno que contiene `pos`.

    Se recorre el fichero de una vez llevando una pila de llaves abiertas. No se
    delimita por 'addEventListener' ni por ningun otro nombre a proposito: el dia que
    la app cuelgue un envio de otra cosa -un temporizador, una promesa- el censo lo
    seguiria viendo."""
    pila = []
    for j, c in enumerate(js):
        if j >= pos and pila:
            return pila[-1], None
        if c == "{":
            pila.append(j)
        elif c == "}" and pila:
            pila.pop()
    return None, None


def _consumido(js, pos):
    """True si el resultado de la llamada que empieza en `pos` va a alguna parte.

    Mismo criterio que app_03, y a proposito: si es ';', '{' o '}' la llamada es una
    sentencia suelta y lo que devuelva se pierde. El '!' se salta, que es como esta
    escrito el caso bien hecho: if (!enviarComandoFirmware(...)) return;"""
    i = pos - 1
    while i >= 0 and (js[i].isspace() or js[i] == "!"):
        i -= 1
    return i >= 0 and js[i] not in ";{}"


def _sin_pin(js):
    """Los comandos que el firmware acepta sin PIN, leidos de la constante de la app.

    No es una lista de este pack: es la misma constante que decide el comportamiento,
    asi que si alguien mete un comando ahi, la excepcion de este pack crece con el y
    queda a la vista en el diff del fichero que importa."""
    m = re.search(r"const\s+SIN_PIN\s*=\s*\[([^\]]*)\]", js)
    if not m:
        return None
    return set(re.findall(r"'([^']+)'", m.group(1)))


def _rama_ack(js):
    """El bloque de la rama de $ACK del parser de telemetria."""
    m = re.search(r"else if \(header === '\$ACK'\)\s*\{", js)
    if not m:
        return None
    lim = _bloque(js, js.index("{", m.end() - 1))
    return js[lim[0]:lim[1]] if lim else None


def _envios(js):
    """[(pos, comando)] de cada llamada a enviarComandoFirmware, sin su definicion."""
    fuera = []
    for m in re.finditer(r"\b%s\s*\(" % ENVIO, js):
        antes = js[max(0, m.start() - 40):m.start()]
        if re.search(r"\bfunction\s*$", antes):
            continue
        lit = re.match(r"\s*'([^']*)'", js[m.end():])
        fuera.append((m.start(), lit.group(1) if lit else None))
    return fuera


def _veredicto(js, pos, comando, sinPin):
    """(motivo de fallo, None si el bloque es honesto)."""
    ini, _ = _bloque_que_contiene(js, pos)
    if ini is None:
        return "no se pudo delimitar el bloque que contiene la llamada"
    lim = _bloque(js, ini)
    if lim is None:
        return "el bloque que contiene la llamada no cierra"
    cuerpo = js[lim[0]:lim[1]]
    rel = pos - lim[0]
    antes, despues = cuerpo[:rel], cuerpo[rel:]

    # (a) El verde es del equipo. Vale en cualquier punto del bloque: no depende de si
    # va antes o despues del envio, sino de quien lo escribe.
    if re.search(r"addEvent\(\s*'green'", cuerpo):
        return ("pinta un addEvent('green', ...), que en este fichero es el color con "
                "el que habla el EQUIPO -lo usa la rama de $ACK-. Un verde escrito por "
                "el bloque que manda la orden dice 'aceptada' sin que nadie la haya "
                "aceptado")

    # (b) Si no anuncia nada despues de mandar, no hay nada que comprobar.
    if not re.search(r"\b(addEvent|showToast)\s*\(", despues):
        return None

    protegido = (_consumido(js, pos)
                 or "state.pinVerificado" in antes
                 or (comando is not None and comando in sinPin))
    if not protegido:
        return ("anuncia algo despues de llamar a %s() sin mirar si la orden salio: no "
                "consume el bool que devuelve, no tiene la guarda de state.pinVerificado "
                "delante, y %s no esta en SIN_PIN. Sin PIN verificado la llamada no "
                "escribe un byte y el anuncio se imprime igual"
                % (ENVIO, ("'%s'" % comando) if comando else "el comando"))

    # (c) Y aunque no pueda quedarse mudo, no puede hablar en pasado del resultado: que
    # la orden salga no es que el equipo la haya hecho.
    anuncios = re.findall(r"\b(?:addEvent|showToast)\s*\((.{0,400}?)\)\s*;", despues, re.S)
    for texto in anuncios:
        for palabra, motivo in PASADO.items():
            if palabra.lower() in texto.lower():
                return ("anuncia en pasado (%r): %s. Que la orden SALGA no es que el "
                        "equipo la haya ejecutado; eso lo dicen $ACK y $STATUS, que ya "
                        "tienen quien los pinte" % (palabra, motivo))
    return None


def correr(b, fw):
    b.titulo("La app no anuncia en verde ni en pasado lo que no sabe si paso")

    js = _sin_comentarios(fw.texto_repo(*APP_JS))

    sinPin = _sin_pin(js)
    if sinPin is None:
        raise fw.Abortado(
            "no se hallo la constante SIN_PIN en app.js. Es la que dice que comandos "
            "salen sin autorizacion, o sea cuales NO pueden quedarse mudos; sin ella "
            "este pack acusaria a los mandos de emergencia, que son justo los que estan "
            "bien")

    envios = _envios(js)
    if not envios:
        raise fw.Abortado(
            "no se hallo ni una llamada a %s() en app.js. O la app dejo de mandar "
            "ordenes o cambio la puerta de salida y este censo se quedo atras -que ya "
            "paso una vez, en N-75, y el pack de turno se quedo en CERO comandos sin "
            "decirlo-" % ENVIO)

    # ---- 1. La premisa del verde se comprueba, no se supone ----
    ack = _rama_ack(js)
    if ack is None:
        raise fw.Abortado(
            "no se hallo la rama de $ACK en app.js: es de donde sale la premisa de que "
            "el verde significa 'lo dijo el equipo'. Sin ella, la mitad (a) de este "
            "pack estaria acusando por una convencion que ya no existe")
    b.verificar(
        bool(re.search(r"addEvent\(\s*'green'", ack)) and ENVIO not in ack,
        "calibrado: el verde lo emite la rama de $ACK -y esa rama no manda ordenes-, "
        "asi que en este fichero 'green' significa 'lo dijo el equipo'",
        "la rama de $ACK ya no pinta en verde, o ademas manda ordenes. La regla (a) de "
        "este pack cuelga de esa convencion: si cambio, se cambia el pack antes de "
        "creerle una sola acusacion")

    # ---- 2. La referencia de lo bien hecho tiene que salir bien ----
    ref = [(p, c) for p, c in envios if c == REFERENCIA_BUENA]
    if not ref:
        raise fw.Abortado(
            "no esta el envio de %s en app.js, que es el patron con el que este pack "
            "sabe distinguir un anuncio honesto de una promesa. Sin el, el detector se "
            "queda sin calibrar" % REFERENCIA_BUENA)
    motivoRef = _veredicto(js, ref[0][0], ref[0][1], sinPin)
    b.verificar(
        motivoRef is None,
        "calibrado contra la botonera de campo (%s), que es como se hace bien: guarda "
        "de PIN delante y 'orden enviada' en cyan" % REFERENCIA_BUENA,
        "el detector marca como defectuosa la botonera de campo (%s), que es la que "
        "esta BIEN hecha y lleva la doctrina escrita encima (%s). Con la referencia "
        "buena en rojo ningun otro veredicto de este pack vale nada: se arregla el pack"
        % (REFERENCIA_BUENA, motivoRef))

    b.verificar(
        len(envios) >= 8,
        "censadas %d llamadas a %s() en app.js, leidas del fuente y no de una lista"
        % (len(envios), ENVIO),
        "solo se censaron %d llamadas a %s() y la app tiene mas. Un censo corto aprueba "
        "por no haber mirado" % (len(envios), ENVIO))

    # ---- 3. Una comprobacion por bloque que manda ----
    #
    # Por BLOQUE y no por llamada: dos envios en el mismo manejador comparten el
    # anuncio, y contarlos dos veces inflaria el total con la misma comprobacion.
    vistos = set()
    for pos, comando in envios:
        ini, _ = _bloque_que_contiene(js, pos)
        if ini in vistos:
            continue
        vistos.add(ini)
        motivo = _veredicto(js, pos, comando, sinPin)
        etiqueta = comando or "comando en variable"
        b.verificar(
            motivo is None,
            "%s: lo que anuncia no promete mas de lo que la app sabe" % etiqueta,
            "%s: %s" % (etiqueta, motivo))

    # ---- 4. Controles negativos ----
    #
    # Se ejercen contra bloques sinteticos escritos con las MISMAS funciones reales.
    # Los dos primeros son, palabra por palabra, el codigo que la app tenia el 31/08.
    def juzgar(bloque, comando):
        pos = bloque.index(ENVIO)
        return _veredicto(bloque, pos, comando, sinPin)

    malo_mudo = ("{ " + ENVIO + "('SET_RTC', `${today},${now}`); "
                 "showToast(`Reloj sincronizado a las ${now}`); }")
    b.control_negativo(
        juzgar(malo_mudo, "SET_RTC") is not None,
        "el bloque que tenia la app el 31/08 -mandar SET_RTC y anunciar sin mirar- se "
        "detecta")

    malo_verde = ("{ if (!state.pinVerificado) return; " + ENVIO + "('SET_TIEMPOS', '1,2,3'); "
                  "addEvent('green', 'Ajustes aplicados.'); }")
    b.control_negativo(
        juzgar(malo_verde, "SET_TIEMPOS") is not None,
        "y el que SI tiene la guarda de PIN pero pinta en verde tambien se detecta: el "
        "verde no es del telefono aunque la orden haya salido")

    malo_pasado = ("{ if (!state.pinVerificado) return; " + ENVIO + "('SET_TIEMPOS', '1,2,3'); "
                   "addEvent('cyan', 'Tiempos guardados en el equipo.'); }")
    b.control_negativo(
        juzgar(malo_pasado, "SET_TIEMPOS") is not None,
        "y el que esta protegido y en cyan pero habla en PASADO del resultado tambien: "
        "que la orden salga no es que el equipo la haya hecho")

    bueno_guarda = ("{ if (!state.pinVerificado) { pedirPin(); return; } "
                    + ENVIO + "('SET_MODO', 'AUTO'); "
                    "addEvent('cyan', 'Operario: orden MODO AUTOMATICO enviada al equipo.'); }")
    b.control_negativo(
        juzgar(bueno_guarda, "SET_MODO") is None,
        "y NO se marca la botonera de campo: guarda delante y 'orden enviada' en cyan. "
        "El detector distingue, no acusa a todo el que manda")

    bueno_bool = ("{ if (!" + ENVIO + "('SET_RTC', `${today},${now}`)) return; "
                  "addEvent('cyan', 'Orden SET_RTC enviada al equipo.'); }")
    b.control_negativo(
        juzgar(bueno_bool, "SET_RTC") is None,
        "ni el que mira el bool que devuelve el envio: las tres formas de no quedarse "
        "mudo se reconocen, no solo la de la botonera")

    bueno_sin_pin = ("{ " + ENVIO + "('FORZAR_ROJO'); "
                     "addEvent('red', 'ALERTA: orden ROJO TOTAL enviada al MAESTRO.'); }")
    b.control_negativo(
        juzgar(bueno_sin_pin, "FORZAR_ROJO") is None,
        "ni los mandos de emergencia, que van en SIN_PIN y por eso no pueden quedarse "
        "mudos: una caida segura que pide clave no es una caida segura")
