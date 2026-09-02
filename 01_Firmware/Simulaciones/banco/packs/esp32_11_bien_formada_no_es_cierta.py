# ===== banco/packs/esp32_11_bien_formada_no_es_cierta.py =====
#
# "BIEN FORMADA" NO ES "CIERTA", Y EL OSF SOLO CUBRE UNA DE LAS PUERTAS.
#
# QUE MIDE ESTE PACK Y POR QUE NO LO MIDE esp32_04.
#
# esp32_04_osf vigila el bit OSF: que se lea al arrancar, que decida, que se relea, y
# que solo se limpie despues de escribir. Todo eso sigue siendo cierto y no se toca
# aqui. Lo que esp32_04 NO puede ver es que el OSF a cero significa exactamente una
# cosa -"el oscilador no se paro"- y se estaba leyendo como si significara otra: "el
# numero que vas a devolver es la hora". Son tres huecos distintos, y los tres
# terminaban en lo mismo: una fecha PERFECTAMENTE FORMADA y falsa saliendo por la
# barrera con el semaforo en verde.
#
#   H-1  EL ORDEN DE LA LIMPIEZA DEL OSF. esp32_04 exige que se limpie DESPUES de la
#        escritura, y se limpiaba. Pero se limpiaba antes de la RELECTURA que comprueba
#        que la hora entro. Cuando la relectura no cuadraba, reloj_ajustar() devolvia
#        NO_QUEDO_PUESTA -bien- sobre un chip al que ya le habia borrado el unico bit
#        que sobrevive al corte de corriente. Al siguiente arranque el OSF valia cero,
#        la barrera subia sola, y el modulo declaraba fiable una hora que el propio
#        modulo sabia media hora antes que no lo era. "Confirmada por el bus" no es
#        "confirmada por el reloj": lo primero solo demuestra que alguien hizo ACK.
#
#   H-2  EL BIT 12/24. El registro de horas guarda en su bit 6 el FORMATO. Con la
#        mascara 0x3F -correcta solo en modo 24 h- un modulo en modo 12 h con el
#        oscilador sano devuelve numeros bien formados: las 3 de la tarde se leen como
#        las 23. Hasta DOCE HORAS de error sobre el reloj del que cuelga la operacion
#        nocturna, sin una sola senal. El OSF esta a cero y tiene razon.
#
#   H-3  LAS SALIDAS DE ERROR QUE DEJABAN LA BARRERA ARRIBA. reloj_ajustar() volvia con
#        RELOJ_ERR_ESCRITURA o RELOJ_ERR_NO_QUEDO_PUESTA sin bajar sePuso. El
#        despachador contestaba $ERR -correctamente, eso ya lo mide esp32_03- y la
#        barrera contestaba lo contrario a todo el que preguntara despues. Es N-80 una
#        capa mas abajo, donde no hay tecnico que lo lea.
#
# Y LA RAZON DE QUE H-3 NECESITE UNA BANDERA PEGAJOSA, que es lo menos evidente de las
# tres: bajar sePuso no basta, porque la relectura periodica de R-4 LA VUELVE A SUBIR
# sesenta segundos despues -mira el OSF, y el OSF sigue a cero: la escritura fallida no
# lo toco-. La barrera se levantaria sola sobre unos registros medio escritos sin que
# nadie hubiera hecho nada mal.
#
# COMO SE MIDE, Y POR QUE NO HAY NINGUNA LISTA TECLEADA AQUI.
#
# La mascara de horas, el bit de formato y el offset del registro se releen del fuente
# en cada corrida, y la RELACION entre los tres se recalcula: si alguien ensancha la
# mascara a 0x7F se traga el bit de formato y la comprobacion de H-2 se queda midiendo
# nada; si la estrecha a 0x1F, las horas 20 a 23 se leen como 0 a 3. Esa relacion vivia
# en prosa -"se fuerza el modo 24 h al escribir"- y los comentarios no fallan cuando
# alguien cambia un numero (N-71). Los motivos salen del enum, no de una lista.
#
# 🔴 ESTE PACK SI EJERCE SFTY-18 Y AUN ASI NO LLEVA LA ETIQUETA '# EJERCE'. No es un
# descuido: documentos_02_trazabilidad_sfty compara las etiquetas contra la tabla de
# OPTIMIZACIONES.md EN LAS DOS DIRECCIONES, asi que una etiqueta sin su fila pone la
# compuerta roja igual que una fila sin su etiqueta. Van juntas o no va ninguna, y la
# fila vive en un documento que este camino de trabajo no toca. Queda anotado aqui para
# que se anadan las dos a la vez, no para que se olviden las dos a la vez.

# EJERCE SFTY-18: que una hora BIEN FORMADA no se de por cierta. Cubre las tres puertas
#                 que el OSF no ve -la limpieza antes de la relectura, el bit 12/24 y la
#                 barrera que se queda arriba tras un error-, no solo el oscilador parado.

import re

NOMBRE = "esp32_11_bien_formada_no_es_cierta"
DESCRIPCION = "el reloj del puente no publica una fecha bien formada como si fuera cierta"

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


# Las tres de arriba vienen literales de esp32_04_osf: es el bloque que ya esta
# probado, y reescribirlo para renombrar llamadas es como se cuelan los errores en un
# cambio que no debe cambiar comportamiento.


def _funciones(codigo):
    """Censa TODAS las funciones del fichero, con su cuerpo. No una lista escrita.

    Existe para que "nadie aplica la mascara sin mirar el bit de formato" no lleve
    dentro los nombres de las dos funciones que hoy la aplican: esa lista se queda
    corta el dia que alguien anada una tercera, y entonces la comprobacion aprueba sin
    haber mirado donde hacia falta."""
    salida = []
    for m in re.finditer(r"^[A-Za-z_][\w:\*&\s]*?\b(\w+)\s*\([^;{]*\)\s*\{",
                         codigo, re.M):
        cuerpo = _bloque(codigo, m.end() - 1)
        if cuerpo is not None:
            salida.append((m.group(1), cuerpo))
    return salida


def _asignaciones(codigo, patron):
    """Cuenta ASIGNACIONES, no la declaracion que da el valor de partida.

    Es la misma distincion que N-73 hace entre declarar y llamar: `static bool x =
    false;` menciona el nombre y no es una asignacion del programa -es donde nace-. Un
    censo que la contara diria que la bandera se apaga en dos sitios, y acusaria al
    firmware de tener dos puertas de salida cuando solo tiene una."""
    n = 0
    for m in re.finditer(patron, codigo):
        antes = codigo[max(0, m.start() - 24):m.start()]
        if re.search(r"\b(?:bool|static)\s+$", antes):
            continue
        n += 1
    return n


def _valores_del_enum(codigo, nombre):
    m = re.search(r"\benum\s+%s\s*\{" % re.escape(nombre), codigo)
    if not m:
        return []
    cuerpo = _bloque(codigo, m.end() - 1)
    if cuerpo is None:
        return []
    return re.findall(r"\b([A-Z][A-Z0-9_]{2,})\b", cuerpo)


def _llama(codigo, nombre):
    """True si ese texto INVOCA la funcion, no si solo la nombra. De esp32_04."""
    for m in re.finditer(r"\b%s\s*\(\s*\)" % re.escape(nombre), codigo):
        antes = codigo[max(0, m.start() - 40):m.start()]
        if re.search(r"\b(?:bool|void|int|static|inline|MotivoSinHora)\s+$", antes):
            continue
        return True
    return False


def correr(b, fw):
    b.titulo("El reloj sabe cuando NO es de fiar: orden del OSF, formato y escritura a medias")

    cab = ("ESP32_Expansion", "include", "reloj_ds3231.h")
    cpp = ("ESP32_Expansion", "src", "reloj_ds3231.cpp")
    contrato = ("ESP32_Expansion", "include", "contrato.h")

    cabecera = fw.codigo(*cab)
    reloj = fw.codigo(*cpp)

    # ---- 1. Los tres numeros del registro de horas, releidos del fuente ---------
    #
    # Sin valor por defecto: si alguno deja de estar donde el pack lo busca, esto
    # ABORTA en vez de medir con un numero escrito a mano que "casualmente coincide".
    bit12h = fw.constante(cab, r"#define\s+DS3231_BIT_12H\s+0x([0-9A-Fa-f]+)",
                          "el bit 12/24 del registro de horas del DS3231", base=16)
    ofsHora = fw.constante(cab, r"#define\s+DS3231_OFS_HORA\s+(\d+)",
                           "el offset del registro de horas")
    regHora = fw.constante(contrato, r"#define\s+DS3231_REG_HORA\s+0x([0-9A-Fa-f]+)",
                           "el primer registro de hora del DS3231", base=16)
    # La mascara NO se teclea: se lee de la linea que la aplica.
    mascara = fw.constante(cpp, r"deBcd\(r\[2\]\s*&\s*0x([0-9A-Fa-f]+)\)",
                           "la mascara con la que se decodifica la hora", base=16)

    b.verificar(
        regHora + ofsHora == 0x02 and bit12h == 0x40,
        "el bit de formato se busca en el registro 0x%02X con la mascara 0x%02X, que es "
        "donde el datasheet del DS3231 pone el 12/24: bit 6 del registro de horas"
        % (regHora + ofsHora, bit12h),
        "el formato se estaria leyendo del registro 0x%02X con mascara 0x%02X. El "
        "datasheet pone el 12/24 en el bit 6 del registro 0x02: con otra pareja se "
        "estaria mirando otra cosa -alarmas, control, la fecha- y llamandolo formato "
        "de la hora" % (regHora + ofsHora, bit12h))

    # ---- 2. LA RELACION ENTRE LA MASCARA Y EL BIT, RECALCULADA -----------------
    #
    # Es la parte que no puede vivir en un comentario. Dos numeros que se relacionan
    # por una identidad se comprueban recalculandola desde el fuente (N-71): un
    # comentario no falla cuando alguien cambia uno de los dos.
    b.verificar(
        (mascara & bit12h) == 0,
        "la mascara de la hora (0x%02X) NO se traga el bit de formato (0x%02X): el bit "
        "sigue disponible para decidir si esa hora significa algo"
        % (mascara, bit12h),
        "la mascara 0x%02X incluye el bit de formato 0x%02X. Enmascarar con el bit "
        "dentro convierte el modo 12 h en un sumando de la hora y ademas deja la "
        "comprobacion del formato midiendo un bit que ya se consumio: las dos mitades "
        "de esta defensa se caen con el mismo numero" % (mascara, bit12h))

    b.verificar(
        (mascara | bit12h) == 0x7F,
        "mascara y bit de formato cubren juntos los siete bits utiles del registro "
        "(0x%02X | 0x%02X = 0x7F): no queda ningun bit de hora fuera de la mascara"
        % (mascara, bit12h),
        "mascara 0x%02X y bit 0x%02X dejan bits del registro sin reclamar (union = "
        "0x%02X). Una mascara mas estrecha de la cuenta pierde el bit de las veinte "
        "horas y devuelve las 23 como las 3, bien formadas y sin avisar"
        % (mascara, bit12h, mascara | bit12h))

    # ---- 3. H-2: nadie aplica la mascara sin haber mirado el bit ---------------
    #
    # Censo de TODAS las funciones del fichero, no de las dos que hoy la aplican.
    funciones = _funciones(reloj)
    if len(funciones) < 6:
        raise fw.Abortado(
            "el censo solo hallo %d funciones en %s/src/reloj_ds3231.cpp. El fichero "
            "tiene mas: fallo el buscador, no el firmware, y medir sobre un censo "
            "ciego aprobaria cualquier cosa" % (len(funciones), ROL))

    aplican = [(n, c) for n, c in funciones
               if re.search(r"\[2\]\s*&\s*0x%X" % mascara, c, re.I)]
    if not aplican:
        raise fw.Abortado(
            "ninguna funcion aplica la mascara 0x%02X al registro de horas. O el fuente "
            "cambio de forma o el patron dejo de casar; en los dos casos este pack no "
            "esta midiendo H-2" % mascara)

    ciegas = []
    for nombre, cuerpo in aplican:
        iBit = cuerpo.find("DS3231_BIT_12H")
        iMascara = re.search(r"\[2\]\s*&\s*0x%X" % mascara, cuerpo, re.I).start()
        if iBit < 0 or iBit > iMascara:
            ciegas.append(nombre)

    b.verificar(
        not ciegas,
        "las %d funciones que enmascaran la hora (%s) comprueban ANTES el bit de "
        "formato: la mascara se aplica sabiendo que es valida"
        % (len(aplican), ", ".join(n for n, _ in aplican)),
        "estas funciones aplican la mascara sin mirar el bit de formato: %s. Sobre un "
        "modulo en modo 12 h devuelven una hora perfectamente formada y equivocada en "
        "hasta doce horas, con el OSF a cero y con razon" % ", ".join(ciegas))

    # ---- 4. H-2 en el arranque: el formato tambien lo vigila la relectura ------
    revisar = _cuerpo(reloj, r"static\s+void\s+revisarOsf\s*\(\s*\)")
    if revisar is None:
        raise fw.Abortado(
            "no se hallo revisarOsf() en %s/src/reloj_ds3231.cpp: es la funcion que R-1 "
            "y R-4 ejecutan, y donde el formato tiene que decidir igual que el OSF"
            % ROL)
    b.verificar(
        "DS3231_BIT_12H" in revisar,
        "el formato se comprueba en revisarOsf(), o sea en el arranque y en cada "
        "relectura periodica: reloj_enHora() contesta 'tengo hora', no 'tengo oscilador'",
        "revisarOsf() solo mira el OSF. reloj_enHora() diria que si sobre un modulo en "
        "modo 12 h, y todo el que consulte la barrera sin leer la hora -que es lo que "
        "hace el Modo Degradado del STM32- se lo creeria")

    # ---- 5. 🔴 H-1: EL ORDEN. La limpieza del OSF va detras de la RELECTURA -----
    ajustar = _cuerpo(reloj, r"ResultadoReloj\s+reloj_ajustar\s*\([^)]*\)")
    if ajustar is None:
        raise fw.Abortado(
            "no se hallo reloj_ajustar() en %s/src/reloj_ds3231.cpp: es donde vive el "
            "orden entre escribir, releer y limpiar el OSF, que es la propiedad de H-1"
            % ROL)

    iEscritura = ajustar.find("escribirReg(DS3231_REG_HORA")
    iRelectura = ajustar.find("leerReg(DS3231_REG_HORA, v")
    mLimpieza = re.search(r"&\s*~\s*DS3231_BIT_OSF", ajustar)
    iLimpieza = mLimpieza.start() if mLimpieza else -1
    iRechazo = ajustar.find("return RELOJ_ERR_NO_QUEDO_PUESTA")

    if iEscritura < 0 or iRelectura < 0 or iLimpieza < 0 or iRechazo < 0:
        raise fw.Abortado(
            "no se hallaron los cuatro hitos de reloj_ajustar() (escritura=%d, "
            "relectura=%d, limpieza=%d, rechazo=%d). Este pack compara POSICIONES: con "
            "un hito perdido compararia contra -1 y saldria verde"
            % (iEscritura, iRelectura, iLimpieza, iRechazo))

    b.verificar(
        iEscritura < iRelectura < iLimpieza,
        "H-1: el OSF se limpia DESPUES de que la relectura confirme la hora "
        "(escritura=%d < relectura=%d < limpieza=%d)"
        % (iEscritura, iRelectura, iLimpieza),
        "H-1 ROTO: el OSF se limpia en la posicion %d, antes de la relectura que "
        "empieza en %d. Se estaria borrando el unico bit que sobrevive al corte de "
        "corriente sin saber todavia si la hora entro: la escritura que no cuadre "
        "devolvera su error hoy, y en el proximo arranque el modulo declarara fiable "
        "esa misma hora porque el OSF ya no dice nada" % (iLimpieza, iRelectura))

    b.verificar(
        iRechazo < iLimpieza,
        "H-1: el rechazo por relectura que no cuadra sale ANTES de tocar el OSF "
        "(rechazo=%d < limpieza=%d)" % (iRechazo, iLimpieza),
        "el camino que rechaza por relectura discrepante (posicion %d) esta DETRAS de "
        "la limpieza del OSF (posicion %d): se rechaza con el bit ya borrado, que es "
        "justo el estado que R-3 existe para impedir" % (iRechazo, iLimpieza))

    # ---- 6. H-3: la barrera se baja antes de tocar el bus, y solo sube al final -
    iUltimoFalse = ajustar.rfind("sePuso = false")
    nTrue = len(re.findall(r"sePuso\s*=\s*true", ajustar))
    iTrue = ajustar.find("sePuso = true")
    iUltimoErr = ajustar.rfind("return RELOJ_ERR")

    b.verificar(
        iUltimoFalse >= 0 and iUltimoFalse < iEscritura,
        "H-3: la barrera se baja ANTES de la primera escritura (baja=%d < escritura=%d): "
        "una rama de error nueva no puede olvidarse de bajarla, porque para dejar la "
        "hora fiable hay que llegar al final" % (iUltimoFalse, iEscritura),
        "la barrera NO se baja antes de escribir. Cada salida de error tiene entonces "
        "que acordarse de bajarla una por una, y la que se olvide deja a reloj_enHora() "
        "diciendo que si sobre unos registros medio escritos")

    b.verificar(
        nTrue == 1 and iUltimoErr >= 0 and iUltimoErr < iTrue,
        "H-3: hay UNA sola subida de la barrera y esta detras de la ultima salida de "
        "error (ultimo error=%d < subida=%d)" % (iUltimoErr, iTrue),
        "hay %d subidas de la barrera y/o alguna salida de error por detras de la "
        "subida (ultimo error=%d, subida=%d). Una comprobacion que se ejecuta despues "
        "de haber declarado la hora fiable ya no la puede impedir" % (nTrue, iUltimoErr, iTrue))

    # ---- 7. La duda se pega: R-4 no puede resucitar la barrera sola ------------
    #
    # Sin esto, todo lo anterior dura sesenta segundos: revisarOsf() mira el OSF, el
    # OSF sigue a cero porque la escritura fallida no lo toco, y la barrera vuelve a
    # subir sobre los mismos registros medio escritos.
    nArma = _asignaciones(reloj, r"escrituraDudosa\s*=\s*true")
    nDesarma = _asignaciones(reloj, r"escrituraDudosa\s*=\s*false")
    iArma = ajustar.find("escrituraDudosa = true")
    iDesarma = ajustar.find("escrituraDudosa = false")

    b.verificar(
        nArma == 1 and nDesarma == 1 and 0 <= iArma < iEscritura
        and iUltimoErr < iDesarma,
        "la duda de una escritura a medias se PEGA: se arma antes de escribir (%d) y "
        "solo la apaga la salida de exito (%d, detras del ultimo error en %d)"
        % (iArma, iDesarma, iUltimoErr),
        "la bandera de escritura dudosa no esta o no vive donde debe (armados=%d, "
        "apagados=%d, arma=%d, apaga=%d). Sin ella, la relectura periodica de R-4 "
        "vuelve a subir la barrera al minuto siguiente: el OSF esta a cero y tiene "
        "razon -la escritura fallida no lo toco- y aun asi la hora no vale"
        % (nArma, nDesarma, iArma, iDesarma))

    iDudaEnRevisar = revisar.find("escrituraDudosa")
    iSubeEnRevisar = revisar.find("sePuso = true")
    b.verificar(
        0 <= iDudaEnRevisar < iSubeEnRevisar,
        "y revisarOsf() la consulta antes de subir la barrera (duda=%d < sube=%d)"
        % (iDudaEnRevisar, iSubeEnRevisar),
        "revisarOsf() sube la barrera sin consultar la duda (duda=%d, sube=%d). La "
        "bandera existiria, se armaria, y la funcion que la tenia que respetar pasaria "
        "de largo: seria la prueba muerta de N-51 escrita en el firmware"
        % (iDudaEnRevisar, iSubeEnRevisar))

    # ---- 8. Lo que sale del chip se valida con la misma vara que lo que entra --
    leer = _cuerpo(reloj, r"bool\s+reloj_leer\s*\([^)]*\)")
    if leer is None:
        raise fw.Abortado(
            "no se hallo reloj_leer() en %s/src/reloj_ds3231.cpp: es la unica salida "
            "de fechas del modulo y donde vive esta comprobacion" % ROL)
    iDecodifica = leer.find("fh->anio")
    iValida = leer.find("reloj_rangoValido")
    b.verificar(
        0 <= iDecodifica < iValida,
        "reloj_leer() valida por rango lo que decodifica (decodifica=%d, valida=%d): un "
        "BCD corrupto no sale de aqui disfrazado de fecha"
        % (iDecodifica, iValida),
        "reloj_leer() devuelve lo que decodifico sin validarlo (decodifica=%d, "
        "valida=%d). El camino de ENTRADA se barre con reloj_rangoValido() y el de "
        "SALIDA se fiaba de los registros: un 0x99 en el mes se decodifica a 99 y sale "
        "por esta funcion como una fecha. Es ademas la unica de las tres defensas que "
        "sigue en pie despues de un reset, cuando las banderas de RAM ya no estan"
        % (iDecodifica, iValida))

    # ---- 9. Barrido del enum: ningun motivo declarado se queda sin armar -------
    #
    # Los motivos salen del enum del C++, no de una lista aqui. Un valor nuevo que
    # nadie arma es un diagnostico que el tecnico no puede recibir nunca.
    motivos = _valores_del_enum(cabecera, "MotivoSinHora")
    if len(motivos) < 4:
        raise fw.Abortado(
            "solo se leyeron %d valores del enum MotivoSinHora en %s/include/"
            "reloj_ds3231.h. De ahi sale la lista que se barre; con la lista corta "
            "este pack aprobaria un modulo que no supiera decir por que no tiene hora"
            % (len(motivos), ROL))

    sinArmar = [v for v in motivos
                if not re.search(r"motivo\s*=\s*%s\b" % re.escape(v), reloj)]
    b.verificar(
        not sinArmar,
        "los %d motivos del enum se arman en el .cpp (%s): cada uno es un arreglo "
        "distinto para quien esta delante" % (len(motivos), ", ".join(motivos)),
        "hay motivos declarados que NADIE arma: %s. Un valor de enum que no se asigna "
        "nunca es la version silenciosa de la prueba muerta: documenta una capacidad de "
        "diagnostico que el firmware no tiene" % ", ".join(sinArmar))

    # ---- HALLAZGO QUE ACOMPANA AL BARRIDO, Y NO CUENTA COMO COMPROBACION -------
    #
    # No es un verificar porque el arreglo NO vive en este modulo: el llamador que
    # falta hay que escribirlo en el despachador o en main.cpp, y acusar desde aqui
    # pondria roja la compuerta por un fichero que este camino no toca. Se anota para
    # que exista rastro; un hueco sin rastro es peor que un ABORTADO, porque el
    # ABORTADO al menos grita.
    llamadores = []
    for carpeta, ext in (("src", ".cpp"), ("include", ".h")):
        for f in fw.fuentes_de("ESP32_Expansion", carpeta, ext):
            if f == "reloj_ds3231.%s" % ext.lstrip("."):
                continue
            if _llama(fw.codigo("ESP32_Expansion", carpeta, f), "reloj_motivo"):
                llamadores.append("%s/%s" % (carpeta, f))

    if not llamadores:
        b.reportar(
            "los %d motivos se arman y NADIE los lee: reloj_motivo() no tiene llamador "
            "fuera de su propio modulo" % len(motivos),
            ["El censo -grep de la invocacion, no de la mencion- no halla una sola "
             "llamada a reloj_motivo() en el resto del rol %s." % ROL,
             "Es la Caja Negra de Alarmas de N-73 a medio construir: la funcion esta "
             "declarada, definida, documentada en la cabecera y sin quien la use.",
             "Lo que se pierde es concreto y ya se pago una vez: cuando la relectura "
             "periodica de R-4 descubre a las tres de la manana que la hora dejo de "
             "ser fiable, el modulo lo APUNTA en una variable y no lo DICE. El unico "
             "camino que hoy contesta algo es el $ACK de SET_RTC, que solo corre "
             "cuando hay un tecnico delante escribiendo la hora.",
             "El arreglo NO es de este fichero: es un $EVENT en el bucle principal "
             "cuando la barrera cambia de estado. Va anotado, no contado."])

    # ---- CONTROLES NEGATIVOS ---------------------------------------------------
    #
    # Contra bloques sinteticos con la forma defectuosa REAL -la que tenia el fichero
    # antes de este pack-, no contra un caso inventado. Si el detector aprobara estos,
    # los nueve OK de arriba serian decoracion.

    # H-2: la version anterior de reloj_leer(), que enmascaraba sin mirar el formato.
    leerMalo = ("{ if (!leerReg(DS3231_REG_HORA, r, 7)) return false; "
                "fh->hora = deBcd(r[2] & 0x3F); return true; }")
    mMalo = re.search(r"\[2\]\s*&\s*0x%X" % mascara, leerMalo, re.I)
    b.control_negativo(
        mMalo is not None and leerMalo.find("DS3231_BIT_12H") < 0,
        "una funcion que enmascara la hora sin mirar el bit de formato se detecta: "
        "aplicar 0x%02X sin preguntar es lo que devuelve las 15 como las 23" % mascara)

    # H-2 al reves: el detector no acusa a quien SI mira el bit.
    leerBueno = ("{ if (r[2] & DS3231_BIT_12H) return false; "
                 "fh->hora = deBcd(r[2] & 0x3F); return true; }")
    iBitB = leerBueno.find("DS3231_BIT_12H")
    iMascB = re.search(r"\[2\]\s*&\s*0x%X" % mascara, leerBueno, re.I).start()
    b.control_negativo(
        0 <= iBitB < iMascB,
        "y NO se acusa a la que si lo mira antes: el detector distingue el orden, no "
        "acusa a todo el que enmascare")

    # H-1: el orden anterior -limpiar y luego releer- se detecta.
    ajustarMalo = ("{ escribirReg(DS3231_REG_HORA, r, 7); "
                   "uint8_t limpio = estado & ~DS3231_BIT_OSF; "
                   "escribirReg(DS3231_REG_ESTADO, &limpio, 1); "
                   "leerReg(DS3231_REG_HORA, v, 7); "
                   "return RELOJ_ERR_NO_QUEDO_PUESTA; }")
    eM = ajustarMalo.find("escribirReg(DS3231_REG_HORA")
    rM = ajustarMalo.find("leerReg(DS3231_REG_HORA, v")
    lM = re.search(r"&\s*~\s*DS3231_BIT_OSF", ajustarMalo).start()
    b.control_negativo(
        not (eM < rM < lM),
        "un reloj_ajustar() que limpiara el OSF antes de releer se detecta, aunque "
        "cumpla la regla mas floja de que la limpieza va detras de la escritura")

    # H-3: una subida de barrera con una comprobacion por detras se detecta.
    subeAntes = ("{ sePuso = false; escribirReg(DS3231_REG_HORA, r, 7); "
                 "sePuso = true; "
                 "if (estado & DS3231_BIT_OSF) return RELOJ_ERR_OSF_SIGUE; "
                 "return RELOJ_OK; }")
    b.control_negativo(
        subeAntes.rfind("return RELOJ_ERR") > subeAntes.find("sePuso = true"),
        "una barrera que sube antes de la ultima comprobacion se detecta: la "
        "comprobacion que corre despues de declarar fiable la hora ya no la impide")

    # El censo de la bandera distingue DECLARAR de ASIGNAR. Sin esta distincion el
    # pack contaba dos apagados -uno era `static bool escrituraDudosa = false;`- y
    # acusaba al firmware de una segunda puerta de salida que no existe.
    b.control_negativo(
        _asignaciones("static bool escrituraDudosa = false;",
                      r"escrituraDudosa\s*=\s*false") == 0
        and _asignaciones("escrituraDudosa = false; return RELOJ_OK;",
                          r"escrituraDudosa\s*=\s*false") == 1,
        "el censo de la bandera no cuenta su declaracion como si fuera una salida: "
        "donde NACE una bandera no es donde se APAGA")

    # La relacion mascara/bit sabe romperse: una mascara de 0x7F se traga el formato.
    b.control_negativo(
        (0x7F & bit12h) != 0,
        "una mascara ensanchada a 0x7F se detecta: se tragaria el bit 0x%02X y dejaria "
        "las dos mitades de la defensa midiendo nada" % bit12h)
