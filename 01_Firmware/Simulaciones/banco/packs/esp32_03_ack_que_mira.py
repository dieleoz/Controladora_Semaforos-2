# ===== banco/packs/esp32_03_ack_que_mira.py =====
#
# UN $ACK QUE NO DEPENDE DE LO QUE LA LLAMADA DEVOLVIO ES UNA MENTIRA CON FORMATO DE
# EXITO.
#
# ES EL DEFECTO QUE SE CERRO EL 28/08 EN EL STM32 (N-80), Y SE MUDA DE MICRO SI NADIE
# LO ESCRIBE.
#
# Alli la rama SET_RTC llamaba a reloj_ajustar() y a coordinador_sincronizarHora() y
# mandaba "$ACK,CMD:SET_RTC,RESULT:OK" SIN MIRAR NINGUNA DE LAS DOS. Las dos negativas
# eran correctas y estaban razonadas donde debian -"if (!rtcOperativo) return;",
# "if (!reloj_enHora()) return false;"-; lo que estaba mal era el que contestaba. Con Y2
# confirmado muerto en hardware ese era el caso NORMAL, no el raro: el comando decia que
# si, no ponia la hora, y el tecnico se iba del poste creyendo que lo dejo puesto.
#
# Aqui hay MAS motivos de rechazo que alli, no menos: el bus puede no contestar, la
# escritura puede fallar a mitad, la relectura puede no coincidir y el OSF puede seguir
# puesto tras escribir. Cuatro formas distintas de que la hora no quede puesta, y cuatro
# arreglos distintos para quien esta delante -revisar cableado, repetir, cambiar la
# pila-. Un "no se pudo" generico no le sirve a nadie.
#
# EL MOLDE DE COMO SE HACE BIEN NO SE INVENTA: es SET_TIEMPOS del Maestro
# (bluetooth.cpp:275-294), que pregunta DENTRO del `if` y tiene un $ERR por cada motivo.
# Este pack lo mide igual que app_03_sin_ok_mudo.
#
# COMO SE MIDE, SIN LISTA ESCRITA A MANO: los motivos salen del ENUM del C++.
#
# ResultadoReloj se lee de reloj_ds3231.h en cada corrida. Si alguien anade un valor al
# enum y no le cablea una rama, este pack lo dice. Tecleando la lista aqui, un motivo
# nuevo se aprobaria a si mismo.
#
# NO LLEVA ETIQUETA SFTY, por la misma razon que app_03: roza SFTY-18 -el OK mudo se
# apoya justo en la barrera que SFTY-18 monta- pero NO la ejerce; comprueba la forma del
# despachador, no la barrera. Figurar en la tabla sin ejercer la regla es peor que una
# fila vacia, porque la vacia no miente.

import re

NOMBRE = "esp32_03_ack_que_mira"
DESCRIPCION = "ninguna rama del ESP32 contesta $ACK sin haber mirado lo que devolvio la escritura I2C"

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


def _consumido(texto, pos):
    """True si el resultado de la llamada que empieza en `pos` va a alguna parte.

    Copiado de app_03_sin_ok_mudo._consumido() a proposito: es el bloque literal que
    ya esta probado. Reescribir logica probada para renombrar llamadas es como se
    cuelan los errores en un cambio que no debe cambiar comportamiento."""
    i = pos - 1
    while i >= 0 and (texto[i].isspace() or texto[i] == "!"):
        i -= 1
    if i < 0:
        return False
    return texto[i] not in ";{}"


def _valores_del_enum(codigo, nombre):
    m = re.search(r"\benum\s+%s\s*\{" % re.escape(nombre), codigo)
    if not m:
        return []
    cuerpo = _bloque(codigo, m.end() - 1)
    if cuerpo is None:
        return []
    return re.findall(r"\b([A-Z][A-Z0-9_]{2,})\b", cuerpo)


def correr(b, fw):
    b.titulo("El $ACK del reloj mira lo que devolvio la escritura")

    cabecera = fw.codigo("ESP32_Expansion", "include", "reloj_ds3231.h")
    desp = fw.codigo("ESP32_Expansion", "src", "despachador.cpp")

    # ---- 1. Los motivos salen del ENUM, no de una lista tecleada -------------
    motivos = _valores_del_enum(cabecera, "ResultadoReloj")
    if len(motivos) < 3:
        raise fw.Abortado(
            "solo se leyeron %d valores del enum ResultadoReloj en %s/include/"
            "reloj_ds3231.h. De ahi sale la lista de motivos que el despachador tiene "
            "que saber distinguir; con la lista vacia este pack aprobaria un "
            "despachador que contestara OK a todo" % (len(motivos), ROL))

    b.verificar(
        True,
        "censo leido del C++: %d resultados distintos que reloj_ajustar() puede "
        "devolver (%s)" % (len(motivos), ", ".join(motivos)),
        "no deberia llegarse aqui")

    # ---- 2. El retorno de reloj_ajustar() SE CONSUME --------------------------
    llamadas = list(re.finditer(r"\breloj_ajustar\s*\(", desp))
    b.verificar(
        bool(llamadas) and all(_consumido(desp, m.start()) for m in llamadas),
        "el despachador GUARDA lo que devuelve reloj_ajustar() en vez de tirarlo",
        "el despachador llama a reloj_ajustar() como sentencia suelta y tira su "
        "resultado. Es N-80 literal: la unica respuesta que la funcion sabe dar se "
        "pierde, y lo que se conteste despues no puede depender de ella" if llamadas
        else "no hay ni una llamada a reloj_ajustar() en el despachador: o el fuente "
             "cambio de forma o el buscador se quedo ciego, y medir cero llamadas "
             "saldria en verde")

    # ---- 3. CADA valor del enum tiene su rama ---------------------------------
    #
    # Se admite la rama explicita (r == VALOR) o, para el exito, la comparacion
    # complementaria (r != RELOJ_OK) que recoge lo que nadie cableo.
    sinRama = [v for v in motivos
               if not re.search(r"[=!]=\s*%s\b" % re.escape(v), desp)]
    b.verificar(
        not sinRama,
        "los %d resultados del enum tienen rama propia en el despachador" % len(motivos),
        "hay resultados de reloj_ajustar() SIN RAMA: %s. Un motivo sin rama cae por el "
        "else y se contesta como los demas: el caso nuevo se aprueba a si mismo, que es "
        "como se cuelan los defectos en un cambio que 'no cambia nada'"
        % ", ".join(sinRama))

    # ---- 4. 🔴 NINGUN $ACK SIN UN TEST DEL RESULTADO POR DELANTE --------------
    #
    # La forma prohibida es exactamente la de N-80: llamar, y contestar $ACK sin que
    # entre medias haya un `if` que interrogue lo que devolvio.
    if not llamadas:
        raise fw.Abortado(
            "sin llamada a reloj_ajustar() no hay nada entre lo que medir: este pack "
            "compara la POSICION del $ACK contra la de la llamada y la de su test")
    posLlamada = llamadas[0].start()

    mudos = []
    for m in re.finditer(r'"\$ACK', desp):
        if m.start() < posLlamada:
            mudos.append("un $ACK antes de llamar siquiera")
            continue
        entre = desp[posLlamada:m.start()]
        if not re.search(r"\br\s*[=!]=", entre):
            mudos.append(desp[m.start():m.start() + 46])

    b.verificar(
        not mudos,
        "los %d $ACK del despachador viven detras de un test del resultado: la "
        "confirmacion depende de lo que la escritura contesto"
        % len(re.findall(r'"\$ACK', desp)),
        "HAY $ACK MUDOS: %s. El tecnico recibe una confirmacion de algo que puede no "
        "haber ocurrido -bus mudo, pila agotada, escritura a medias-, se va del poste, "
        "y el reloj se queda como estaba sin que nada lo diga" % " | ".join(mudos))

    # ---- 5. Y hay un $ERR por cada motivo, con texto propio -------------------
    descs = set(re.findall(r'DESC:([A-Z0-9_]+)', desp))
    # Los motivos 1 y 2 comparten DESC a proposito -para quien esta delante del telefono
    # son el mismo arreglo, reescribir la fecha- asi que se exige uno menos que ramas.
    b.verificar(
        len(descs) >= len(motivos) - 2,
        "el despachador distingue %d motivos de rechazo por texto (%s)"
        % (len(descs), ", ".join(sorted(descs))),
        "solo hay %d motivos distintos para %d resultados posibles: %s. Un 'no se pudo' "
        "generico no le dice al tecnico si tiene que revisar el cableado, repetir el "
        "comando o cambiar la pila" % (len(descs), len(motivos), sorted(descs)))

    # ---- 6. Y el $ERR de la relectura tambien esta dentro de su `if` ----------
    cuerpo = _cuerpo(desp, r"void\s+despachador_observar\s*\([^)]*\)")
    if cuerpo is None:
        raise fw.Abortado(
            "no se hallo despachador_observar() en %s/src/despachador.cpp. Es el unico "
            "sitio donde vive esta propiedad" % ROL)
    lecturas = list(re.finditer(r"\breloj_leer\s*\(", cuerpo))
    b.verificar(
        bool(lecturas) and all(_consumido(cuerpo, m.start()) for m in lecturas),
        "la relectura con la que se compone el $ACK tambien se interroga: si falla, "
        "sale $ERR y no una hora inventada",
        "reloj_leer() se llama sin mirar lo que devuelve. La hora que iria en el $ACK "
        "seria la de un buffer sin rellenar, y saldria con formato perfecto")

    # ---- CONTROLES NEGATIVOS -------------------------------------------------
    #
    # Contra bloques sinteticos con las MISMAS funciones reales: uno con el defecto de
    # N-80 y otro sin el. Si el detector aprobara el primero, todos los OK de arriba
    # serian decoracion.
    malo = ('{ reloj_ajustar(&f); '
            'puente_emitirPropio("$ACK,NODE:PUENTE,CMD:SET_RTC,RESULT:OK"); }')
    mMalo = re.search(r"\breloj_ajustar\s*\(", malo)
    b.control_negativo(
        not _consumido(malo, mMalo.start()),
        "una rama que llama a reloj_ajustar() como sentencia suelta se detecta como "
        "OK mudo")

    bueno = ('{ ResultadoReloj r = reloj_ajustar(&f); '
             'if (r == RELOJ_ERR_SIN_RELOJ) { emitir("$ERR,DESC:X"); } '
             'else { emitir("$ACK,RESULT:OK"); } }')
    mBueno = re.search(r"\breloj_ajustar\s*\(", bueno)
    posB = mBueno.start()
    b.control_negativo(
        _consumido(bueno, posB)
        and all(re.search(r"\br\s*[=!]=", bueno[posB:m.start()])
                for m in re.finditer(r'"\$ACK', bueno)),
        "una rama que guarda el resultado y lo interroga antes del $ACK NO se marca: "
        "el detector distingue, no acusa a todo el que llama")

    sinTest = ('{ ResultadoReloj r = reloj_ajustar(&f); (void)r; '
               'emitir("$ACK,RESULT:OK"); }')
    posS = re.search(r"\breloj_ajustar\s*\(", sinTest).start()
    b.control_negativo(
        any(not re.search(r"\br\s*[=!]=", sinTest[posS:m.start()])
            for m in re.finditer(r'"\$ACK', sinTest)),
        "guardar el resultado y NO interrogarlo tampoco cuela: consumir el valor no es "
        "lo mismo que mirarlo")
