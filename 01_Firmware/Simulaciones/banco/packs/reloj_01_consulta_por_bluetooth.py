# ===== banco/packs/reloj_01_consulta_por_bluetooth.py =====
#
# LOS BITS DEL RELOJ TIENEN QUE SALIR POR DONDE EL FIRMWARE MANDA AL TECNICO.
#
# LA PROPIEDAD, EN UNA LINEA: los dos $ERR del Maestro que nombran CONSULTA RELOJ
# llevan pegados, EN SU PROPIA RAMA, los bits que esa consulta ensenaba; esos bits se
# leen del periferico en cada llamada, caben en la trama y llegan enteros a la pantalla
# de la app.
#
# POR QUE EXISTE. El firmware mandaba al tecnico a un instrumento tapiado:
#
#   reloj_diagnostico()   definida en Maestro/src/reloj.cpp, y sus UNICOS consumidores
#                         eran modo_hora.cpp -la pantalla CONSULTA RELOJ de N-45-.
#   MODO_HORA             se arma en UN solo sitio: menu.cpp, el case 1 de NIVEL_CONFIG.
#   para llegar ahi       hacen falta DOS botonAceptar(): uno para bajar al nivel y otro
#                         para entrar en la opcion.
#   botonAceptar()        es hoy "return false" -PB14 y PB15 dejaron de ser pulsadores
#                         para ser camaras-. Y por Bluetooth no existe SET_MODO:HORA.
#
# O sea: doblemente inalcanzable, mientras DOS $ERR de bluetooth.cpp seguian mandando
# alli por su nombre -SIN_CRISTAL_VEA_CONSULTA_RELOJ y SIGUE_PARADO_VEA_CONSULTA_RELOJ-.
# Y no es una curiosidad: es el instrumento del bloqueante del cristal Y2. Sin el,
# SET_RTC distingue "hay reloj" de "no hay reloj" y NO distingue lseOn=0 -firmware o
# dominio de respaldo, donde el cristal es inocente- de lseOn=1 con lseRdy=0, que es el
# unico caso donde se mira Y2. Esa confusion ya costo cambiar pila, R5 e Y2 tres veces
# con el hardware sano (N-45).
#
# LAS TRES FORMAS EN QUE ESTE INSTRUMENTO SE MUERE EN SILENCIO, Y POR ESO SE VIGILAN:
#
#   1. QUE ALGUIEN QUITE LA LLAMADA Y DEJE EL $ERR. El $ERR seguiria nombrando la
#      consulta y el equipo volveria a mandar a una pantalla tapiada, con la compuerta
#      en verde. Por eso no se comprueba que la llamada este "en el fichero": se
#      comprueba que este DENTRO DEL BLOQUE de cada rama, que es la leccion de N-89.
#
#   2. QUE EL DETALLE GANE UNA COMA. _camposNmea() de la app parte la trama por ',' y
#      cada trozo por su PRIMER ':' (app.js:1798-1810), y el pintor de $EVENT solo
#      ensena data.DETALLE. Una coma dentro del detalle convierte todo lo que va detras
#      en campos sueltos que nadie mira: los bits saldrian al cable y NO llegarian a la
#      pantalla. Es la prueba muerta con forma de telemetria -verde, emitiendo, y sin
#      que el dato llegue a un ojo-.
#
#   3. QUE LA TRAMA SE TRUNQUE POR EL FINAL. Es N-108 otra vez: alli el $ALARM se
#      cortaba por la HORA sin que nada lo dijera, y una trama truncada es una que el
#      otro extremo descarta por checksum, o sea un diagnostico que desaparece justo
#      cuando hace falta. La cuenta se rehace aqui en cada corrida, con TODOS sus
#      sumandos releidos del C++ y de reloj.h: el formato del $EVENT, el del detalle, el
#      tamano del payload, el del buffer del contador y el TIPO de cada campo de
#      RelojDiag. Sin un solo valor por defecto.
#
# LO QUE ESTE PACK NO PUEDE HACER, ESCRITO PARA QUE NADIE LO LEA COMO PERMISO. Esto es
# Python leyendo el .cpp: NO ejecuta el firmware, NO enciende un STM32 y NO ha visto un
# solo bit del RCC->BDCR. Que estos seis numeros digan la verdad sobre el cristal solo
# lo puede decir el banco (B5). Aqui se comprueba que el camino existe y que lo que
# viaje por el llegue entero.
#
# SIN ETIQUETA SFTY, Y ES DELIBERADO. La tentacion era ponerle una: esto es diagnostico
# de campo y toca el reloj del que cuelga el Modo Degradado. Pero no EJERCE ninguna
# regla -no mide un enclavamiento, ni un limite, ni una maniobra-, y una regla que
# aparece cubierta por una prueba que no la ejerce es peor que una fila vacia.

import re

NOMBRE = "reloj_01_consulta_por_bluetooth"
DESCRIPCION = ("los dos $ERR que nombran CONSULTA RELOJ sacan los bits del RTC, y esos "
               "bits caben en la trama y llegan enteros a la app")

MAESTRO = ("Maestro", "src", "bluetooth.cpp")
RELOJ_H = ("Maestro", "include", "reloj.h")
ESCLAVO = ("Esclavo", "src", "bluetooth.cpp")

# La marca que los dos $ERR comparten. Es el sujeto entero del pack: si desaparece de
# los dos sitios, esto ABORTA en vez de aprobar midiendo un fichero sin ella.
MARCA = "VEA_CONSULTA_RELOJ"

# El emisor. Se nombra aqui porque es el contrato; lo que NO se escribe a mano es
# ninguno de sus numeros ni ninguno de sus campos.
EMISOR = "reportarBitsDelReloj"

# Los SEIS campos con los que razona la cabecera de reloj.h. No son "los de la
# estructura": son los que su tabla de diagnostico usa para separar las averias. Los dos
# que faltan -configurado y anio- salen de rtcOperativo, que en las dos puertas donde
# esto se emite vale false por construccion: publicarlos serian dos ceros constantes con
# forma de medida, que es el BAT:12.6 de N-108.
CAMPOS = ("lseOn", "lseRdy", "lseByp", "rtcSel", "rtcEn", "cnt")

# Cifras que cada tipo puede llegar a imprimir. Es lo unico escrito a mano de este pack,
# y es aritmetica del lenguaje, no del firmware: un uint32_t no pasa de 4294967295.
CIFRAS = {"bool": 1, "uint8_t": 3, "uint16_t": 5, "uint32_t": 10,
          "unsigned long": 10, "int": 11}


class _Falta(Exception):
    pass


def _abortar(que, donde):
    return _Falta("no se hallo %s en %s. Fallo el buscador o el bloque se movio, y en "
                  "los dos casos aprobar aqui seria aprobar sin mirar" % (que, donde))


def _bloque_que_contiene(codigo, idx):
    """El bloque { ... } mas interno que envuelve a la posicion idx.

    Se lee por LLAVES y no por lineas a proposito: lo que hay que comprobar es que la
    llamada este dentro de LA MISMA rama que el $ERR, y una comprobacion por proximidad
    de lineas aprobaria una llamada colgada del `else` de al lado."""
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
        raise _abortar("el bloque que envuelve al $ERR", "bluetooth.cpp del Maestro")
    prof = 0
    for j in range(ini, len(codigo)):
        if codigo[j] == "{":
            prof += 1
        elif codigo[j] == "}":
            prof -= 1
            if prof == 0:
                return codigo[ini + 1:j]
    raise _abortar("el cierre del bloque que envuelve al $ERR", "bluetooth.cpp del Maestro")


def _cuerpo(codigo, patron, que):
    m = re.search(patron, codigo)
    if not m:
        raise _abortar(que, "bluetooth.cpp del Maestro")
    return _bloque_que_contiene(codigo, codigo.find("{", m.end() - 1) + 1)


def _snprintf(cuerpo, destino, que):
    """(formato, [argumentos]) del snprintf que escribe en `destino`."""
    m = re.search(r'snprintf\(\s*%s\s*,[^,]+,\s*"((?:[^"\\]|\\.)*)"\s*(.*?)\);'
                  % re.escape(destino), cuerpo, re.S)
    if not m:
        raise _abortar("el snprintf que llena %s" % que, "el emisor")
    fmt = m.group(1)
    cola = m.group(2).strip()
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
    return fmt, args


def _buffer(cuerpo, nombre, que):
    m = re.search(r"char\s+%s\s*\[\s*(\d+)\s*\]" % re.escape(nombre), cuerpo)
    if not m:
        raise _abortar("la declaracion de %s" % que, "el emisor")
    return int(m.group(1))


def _tipos_de_relojdiag(fw):
    """El TIPO de cada campo de RelojDiag, leido de reloj.h.

    Se lee y no se supone porque de ahi sale el ancho de la trama: el dia que alguien
    ensanche rtcSel de uint8_t a uint16_t, esta cuenta tiene que moverse sola. Un
    comentario no se mueve."""
    m = re.search(r"struct\s+RelojDiag\s*\{(.*?)\}\s*;", fw.texto(*RELOJ_H), re.S)
    if not m:
        raise _abortar("la struct RelojDiag", "Maestro/include/reloj.h")
    tipos = {}
    for t, n in re.findall(r"(bool|uint8_t|uint16_t|uint32_t)\s+(\w+)\s*;", m.group(1)):
        tipos[n] = t
    return tipos


def _peor(fmt, anchos):
    """La cadena mas larga que ese printf puede producir, con un ancho por conversion."""
    convs = re.findall(r"%[0-9]*l?[usd]", fmt)
    if len(convs) != len(anchos):
        return None
    return len(re.sub(r"%[0-9]*l?[usd]", "", fmt)) + sum(anchos)


def correr(b, fw):
    b.titulo("La CONSULTA RELOJ, por el unico camino que queda abierto")

    try:
        codigo = fw.codigo(*MAESTRO)
        emisor = _cuerpo(codigo, r"static\s+void\s+%s\s*\(\s*\)" % EMISOR,
                         "%s()" % EMISOR)
        evento = _cuerpo(codigo, r"void\s+bluetooth_reportarEvento\s*\(",
                         "bluetooth_reportarEvento()")
        tipos = _tipos_de_relojdiag(fw)
    except _Falta as e:
        raise fw.Abortado(str(e))

    # ---- 1. Las dos puertas que nombran la consulta siguen ahi ----------------
    #
    # Se cuentan ANTES de comprobar nada. Si un dia son cero, el pack no puede aprobar
    # "todas las puertas llevan los bits" sobre un conjunto vacio: eso aprueba siempre.
    puertas = [m.start() for m in re.finditer(
        r'enviarTramaConCrc\(\s*"\$ERR[^"]*%s[^"]*"\s*\)' % MARCA, codigo)]
    if len(puertas) < 2:
        raise fw.Abortado(
            "el Maestro tiene %d $ERR que nombren %s y se esperaban los dos "
            "(SET_RTC y REINICIAR_RELOJ). O el literal cambio o la rama se movio; en "
            "los dos casos este pack estaria midiendo un fichero que ya no es el suyo"
            % (len(puertas), MARCA))

    # ---- 2. CADA UNA lleva los bits DENTRO DE SU PROPIA RAMA -----------------
    #
    # Dentro del bloque, no "en el fichero". Es N-89 aplicado por adelantado: una
    # comprobacion por presencia global seguiria en verde con la llamada colgada de
    # cualquier otra rama, y el tecnico volveria a recibir el "vea CONSULTA RELOJ"
    # a secas.
    for i, pos in enumerate(puertas):
        try:
            rama = _bloque_que_contiene(codigo, pos)
        except _Falta as e:
            raise fw.Abortado(str(e))
        b.verificar(
            ("%s(" % EMISOR) in rama,
            "el $ERR #%d que manda a CONSULTA RELOJ saca los bits en su MISMA rama"
            % (i + 1),
            "el $ERR #%d nombra CONSULTA RELOJ y en su rama NO se llama a %s(). El "
            "equipo manda al tecnico a una pantalla que no se puede abrir -MODO_HORA "
            "pide dos botonAceptar(), y botonAceptar() devuelve false desde que PB14 y "
            "PB15 son camaras-, y ademas se lo dice por su nombre, que es peor que "
            "callarse" % (i + 1, EMISOR))

    # ---- 3. Los bits se LEEN del periferico en cada llamada -------------------
    b.verificar(
        re.search(r"reloj_diagnostico\s*\(\s*&", emisor) is not None,
        "el emisor llama a reloj_diagnostico() y publica lo que el micro VE en ese "
        "instante",
        "el emisor NO llama a reloj_diagnostico(): esta publicando algo que no ha "
        "leido del RCC->BDCR. Un diagnostico con valores guardados o escritos a mano "
        "es exactamente lo que N-45 retiro de la pantalla -'Es Y2: toca hardware' sin "
        "haber medido-")

    # ---- 4. Los seis campos del razonamiento de reloj.h estan, y salen del struct ----
    fmt_det, args_det = None, None
    try:
        fmt_det, args_det = _snprintf(emisor, "det", "el detalle")
    except _Falta as e:
        raise fw.Abortado(str(e))

    juntos = " ".join(args_det) + " " + emisor
    faltan = [c for c in CAMPOS if not re.search(r"\bd\.%s\b" % c, juntos)]
    b.verificar(
        not faltan,
        "los seis campos con los que razona reloj.h viajan en la trama: %s"
        % ", ".join(CAMPOS),
        "faltan %s en lo que sale por Bluetooth. Sin lseOn y lseRdy juntos no se "
        "separa 'el oscilador no se pide' -que es firmware, y el cristal es inocente- "
        "de 'pedido y no arranca', que es el unico caso donde se mira Y2. Es la "
        "confusion que costo cambiar pila, R5 e Y2 con el hardware sano" % faltan)

    # ---- 5. El contador se MARCA cuando no se leyo, no se aplasta a cero ------
    b.verificar(
        re.search(r"cntLeido", emisor) is not None and '"--"' in emisor,
        "con RTCEN=0 el contador sale marcado con '--' y no con un 0: 'no se leyo el "
        "periferico' y 'el contador esta parado en cero' son diagnosticos DISTINTOS",
        "el emisor publica el contador sin mirar cntLeido, o lo marca con algo que no "
        "es '--'. Un 0 ahi es indistinguible de un RTC parado en cero, y manda a mirar "
        "el cristal cuando lo que pasa es que el RTC ni siquiera esta habilitado")

    # ---- 6. EL DETALLE NO LLEVA COMAS -----------------------------------------
    #
    # Esta es la que separa "sale al cable" de "llega a un ojo", y no es una regla de
    # estilo: es el parser de la app, medido.
    b.verificar(
        "," not in fmt_det,
        "el detalle del $EVENT no lleva ni una coma: la app lo pinta ENTERO en el "
        "registro de eventos (%r)" % fmt_det,
        "el detalle lleva una coma (%r). _camposNmea() de la app parte por ',' y el "
        "pintor de $EVENT solo ensena data.DETALLE, asi que todo lo que vaya detras de "
        "esa coma sale al cable y NO llega a la pantalla. Bits emitidos que nadie ve "
        "son la prueba muerta con forma de telemetria" % fmt_det)

    # ---- 7. LA TRAMA CABE, con todos los sumandos releidos del C++ ------------
    #
    # Ninguno de estos numeros esta escrito aqui: el formato del $EVENT y el del
    # detalle salen del .cpp, los tamanos de payload, det y cntTxt de sus
    # declaraciones, y el ancho de cada campo del TIPO que le da reloj.h.
    try:
        cntTxt = _buffer(emisor, "cntTxt", "cntTxt")
        det = _buffer(emisor, "det", "det")
        payload = _buffer(evento, "payload", "el payload del $EVENT")
        fmt_ev, args_ev = _snprintf(evento, "payload", "el $EVENT")
        horaBuf = _buffer(evento, "horaBuf", "horaBuf")
    except _Falta as e:
        raise fw.Abortado(str(e))

    anchos = []
    for a in args_det:
        campo = re.search(r"\bd\.(\w+)\b", a)
        if campo:
            anchos.append(CIFRAS[tipos[campo.group(1)]])
        elif a.strip() == "cntTxt":
            anchos.append(cntTxt - 1)
        else:
            raise fw.Abortado(
                "no se supo de que tipo es el argumento %r del detalle. Sin su ancho "
                "la cuenta de la trama seria una estimacion, y una estimacion aqui es "
                "lo que trunco el $ALARM de N-108" % a)

    peor_det = _peor(fmt_det, anchos)
    if peor_det is None:
        raise fw.Abortado(
            "el formato del detalle (%r) tiene %d conversiones y %d argumentos. Con "
            "esa discrepancia no hay cuenta que hacer" % (fmt_det, 0, len(args_det)))

    b.verificar(
        peor_det <= det - 1,
        "el peor detalle por tipo son %d caracteres y det[%d] los aguanta"
        % (peor_det, det),
        "el peor detalle por tipo son %d caracteres y det[%d] solo guarda %d: snprintf "
        "lo corta y los bits salen a medias" % (peor_det, det, det - 1))

    # La HORA se acota por su BUFFER y no por el formato: es el unico sumando que este
    # fichero no compone: viene de reloj_hora() y compania. Acotar por el buffer es lo
    # que snprintf garantiza, y en una cuenta de desbordamiento se toma siempre la cota
    # que no depende de creerse a nadie.
    origen = re.search(r'bluetooth_reportarEvento\(\s*"([^"]+)"', emisor)
    if origen is None:
        raise fw.Abortado("el emisor no pasa un ORIGEN literal: sin el no hay cuenta")
    anchos_ev = []
    for a in args_ev:
        if "origen" in a:
            anchos_ev.append(len(origen.group(1)))
        elif "detalle" in a:
            anchos_ev.append(peor_det)
        elif "horaBuf" in a:
            anchos_ev.append(horaBuf - 1)
        else:
            raise fw.Abortado(
                "el $EVENT lleva un argumento (%r) que esta cuenta no sabe acotar" % a)

    peor_trama = _peor(fmt_ev, anchos_ev)
    b.verificar(
        peor_trama is not None and peor_trama <= payload - 1,
        "la trama de ORIGEN:%s cabe: %s de %d caracteres utiles en payload[%d], con %s "
        "de margen" % (origen.group(1), peor_trama, payload - 1, payload,
                       (payload - 1 - peor_trama) if peor_trama else "?"),
        "LA TRAMA NO CABE: el peor caso son %s caracteres y payload[%d] guarda %d. "
        "snprintf la trunca por el final -o sea por la HORA-, el checksum se calcula "
        "sobre lo que quedo, y el otro extremo la descarta: el diagnostico desaparece "
        "justo cuando el tecnico esta delante del poste. Es N-108, letra por letra"
        % (peor_trama, payload, payload - 1))

    # ---- 8. Lo que falta en la otra punta, que NO es un fallo de esta ---------
    #
    # Va en reportar() y no en verificar() a proposito: no hay firmware posible que
    # apruebe hoy esta comprobacion en el Esclavo -su reloj.h no declara
    # reloj_diagnostico()-, y una comprobacion que ningun firmware puede aprobar no es
    # una comprobacion, es una nota. La regla del alias de CMD_DELTA, aplicada.
    esclavo = fw.codigo(*ESCLAVO)
    if "reloj_diagnostico" not in fw.texto("Esclavo", "include", "reloj.h"):
        b.reportar(
            "el Esclavo tiene la misma puerta muerta y NO tiene con que contestarla",
            ["Esclavo/src/bluetooth.cpp contesta $ERR,CMD:SET_RTC,DESC:SIN_CRISTAL y "
             "ahi se acaba: esa punta no tiene pantalla, no tiene menu y no tiene "
             "CONSULTA RELOJ que consultar.",
             "Y no puede tenerla todavia: Esclavo/include/reloj.h NO declara "
             "reloj_diagnostico() ni RelojDiag -cero apariciones-, asi que el emisor "
             "del Maestro no se puede copiar aqui sin tocar reloj.cpp.",
             "Lo que hace falta es portar reloj_diagnostico() y struct RelojDiag desde "
             "el Maestro; el bloque de bluetooth.cpp es entonces el mismo letra por "
             "letra. Mientras tanto, el tecnico que sube 5 m al poste del Esclavo "
             "sigue sin poder distinguir lseOn=0 de lseRdy=0.",
             "Emisor presente en el Esclavo: %s"
             % ("SI" if EMISOR in esclavo else "NO")])

    # ---- CONTROLES NEGATIVOS -------------------------------------------------
    #
    # Los tres atacan las tres formas de morir en silencio de la cabecera.
    falso = ('void f(){ if (!reloj_hayCristal()) {'
             ' enviarTramaConCrc("$ERR,CMD:SET_RTC,DESC:SIN_CRISTAL_VEA_CONSULTA_RELOJ");'
             ' } else { %s(); } }' % EMISOR)
    pos = falso.find(MARCA)
    b.control_negativo(
        ("%s(" % EMISOR) not in _bloque_que_contiene(falso, pos),
        "con la llamada colgada del else de al lado -y presente en el fichero-, el "
        "lector de ramas la da por AUSENTE: mide el bloque, no la proximidad")

    b.control_negativo(
        "," in "ON:%u,RDY:%u",
        "un detalle que recupere la coma se detecta: es la diferencia entre salir al "
        "cable y llegar a un ojo")

    b.control_negativo(
        _peor(fmt_ev, anchos_ev) > 100 - 1,
        "con el payload[100] que este $EVENT tenia antes de N-114 la misma cuenta dice "
        "que NO cabe: la aritmetica reacciona al tamano del buffer en vez de darlo por "
        "bueno -y ese es el numero real que obligo a subirlo-")
