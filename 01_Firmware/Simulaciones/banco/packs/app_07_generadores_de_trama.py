# ===== banco/packs/app_07_generadores_de_trama.py =====
#
# NO PUEDE HABER DOS GENERADORES DE TRAMA EN EL MISMO ARBOL QUE NO COINCIDAN.
#
# Es N-73 -la funcion que nadie llama- con un agravante que la vuelve otra cosa: la
# huerfana de este pack NO ESTA INERTE, ESTA CERTIFICADA. Vive en un fichero con nombre
# de especificacion, con pruebas verdes encima, y es el primero que abrira quien tenga
# que escribir un puente. Una funcion muerta que nadie mira es deuda; una funcion muerta
# que un test aprueba es una ESPECIFICACION FALSA.
#
# LO QUE ESTE PACK MIDIO AL ESCRIBIRSE, Y POR ESO EXISTE.
#
#   js/nmea_parser.js  generarComando() ->  $CMD:PIN:1234:SET_MODO:AUTO*XX\r\n
#   app.js             enviarComandoFirmware() ->  CMD:PIN:1234:SET_MODO:AUTO\r\n
#
# Los dos dicen componer "el comando que se le manda al equipo", y solo uno de los dos
# entra. El despachador del firmware compara el buffer TAL CUAL llega -bluetooth_loop()
# acumula bytes hasta el \r o el \n y llama a procesarComando(btBufIn) sin quitar nada-,
# y su primera comparacion es strcmp(cmd, "CMD:FORZAR_ROJO") / strncmp(cmd,
# "CMD:PIN:1234:", 13). Con el '$' delante NO CASA NINGUNA, asi que toda la trama de
# NMEAParser cae en el $ERR,CMD:AUTH_FAILED,DESC:PIN_INVALIDO.
#
# Y las pruebas que lo certifican no podian verlo: tests/test_unitarios.js afirma
#   assert(cmdAuto.includes('CMD:PIN:1234:SET_MODO:AUTO'), ...)
# con includes(), que es cierto tambien de la trama envuelta. Una prueba de subcadena no
# puede detectar un envoltorio: mide que el payload esta dentro, no que la trama sea esa.
#
# COMO SE MIDE, SIN NINGUNA LISTA ESCRITA A MANO.
#
#   (a) EL CONTRATO DEL CABLE SE LEE DEL C++. Los literales que cada despachador compara
#       contra `cmd` -o sea, contra el byte 0 de la trama- son el contrato entero: no hay
#       tabla ni enum. De ahi sale el conjunto de comienzos aceptados.
#   (b) SE COMPRUEBA QUE EL FIRMWARE NO DESNATA NADA antes de comparar. Si algun dia el
#       lazo de recepcion quitara el '$' y el checksum, todo el razonamiento de arriba se
#       cae, y este pack tiene que enterarse en vez de seguir acusando. Se exige leer en
#       el .cpp que el byte que se guarda es el byte que se leyo y que procesarComando()
#       recibe el buffer sin desplazar.
#   (c) LOS EMISORES DE LA APP SE CENSAN DESDE index.html, no de una lista: los <script
#       src> son los modulos que de verdad se cargan. Un modulo nuevo entra al censo
#       solo; uno que se quite, sale.
#   (d) DE CADA EMISOR SE SACA SU MOLDE -el texto literal con un hueco donde el codigo
#       interpola- y, cuando la trama se termina en otra funcion, SE COMPONE UN NIVEL.
#       Sin ese paso el defecto es invisible: generarComando() por si sola compone un
#       payload correcto, y quien mete el '$' es formatearTrama(). El defecto no esta en
#       ninguna de las dos: esta en la COMPOSICION, que es justo lo que nadie lee.
#
# LA SEGUNDA PROPIEDAD, Y POR QUE ES UN TRINQUETE Y NO UN ABSOLUTO.
#
# Los modulos que index.html carga y que nadie llama. Aqui se copia el criterio de
# costura_10: exigir "cero huerfanos" seria falso -puede haber obra a medias declarada
# como tal- asi que la lista de conocidos esta CONGELADA abajo y lo que falla es un
# huerfano NUEVO, o uno de la lista que GANA llamador y no sale de ella.
#
# La diferencia con costura_10, y el motivo de que esto no sea una ampliacion suya: alli
# el sujeto son funciones declaradas en los include/*.h de las dos puntas del firmware y
# el censo es "declaracion contra llamadas en los .cpp". Aqui el sujeto son modulos de
# JavaScript, el censo arranca de las etiquetas <script> de un .html, y la propiedad que
# de verdad importa -que los generadores coincidan- no tiene nada que ver con la
# orfandad. Meterlo dentro habria obligado a costura_10 a llevar dos parsers para dos
# lenguajes y a que su DESCRIPCION dejara de ser cierta.
#
# SOBRE LAS ETIQUETAS SFTY: este pack NO lleva ninguna. Vigila la forma de una trama y
# el censo de modulos de una interfaz; no ejerce ninguna barrera del equipo. Figurar en
# la tabla de trazabilidad sin ejercer la regla es peor que una fila vacia.

import re

NOMBRE = "app_07_generadores_de_trama"
DESCRIPCION = "un solo generador de trama de comando, con la forma que compara el despachador, y ningun modulo cargado sin llamador"

APP_HTML = ("05_Funcional", "App_Semaforo", "index.html")
BT_MAESTRO = ("Maestro", "src", "bluetooth.cpp")
BT_ESCLAVO = ("Esclavo", "src", "bluetooth.cpp")

# Los modulos que index.html carga y que HOY no llama nadie, con su motivo. La lista es
# un trinquete: sobra tanto uno que aparece como uno que desaparece.
HUERFANOS_CONOCIDOS = {
    # Su generarComando() es el segundo generador de trama, y su formato no entra en el
    # firmware. Que ademas no lo llame nadie es lo unico que ha impedido que rompa la
    # app; no es un atenuante, es la razon de que lleve meses sin que nadie lo note.
    "NMEAParser": "js/nmea_parser.js - parser y generador de tramas, sin un solo uso",
    # Capa de transporte SPP/BLE/Serial escrita entera. app.js habla por window.
    # bluetoothSerial y por fetch() al puente, sin pasar por aqui.
    "BluetoothDriver": "js/bluetooth_driver.js - transporte alternativo sin conectar",
    # Constantes del protocolo -PIN, baudrate, limites de tiempo, UUIDs BLE-. app.js
    # lleva las suyas propias, que es la segunda copia de siempre.
    "IOT_CONFIG": "js/config.js - constantes duplicadas en app.js",
}

# EL MARCADOR DE INTERPOLACION. Se usa un caracter que no puede aparecer en un fuente
# JavaScript legible, para que un molde nunca se confunda con texto del programa.
HUECO = "\x00"

# Palabras que llevan parentesis y llave detras sin ser una funcion. Sin este filtro el
# censo daria por funcion cada `if (...) {` del fichero y los moldes se repartirian mal.
_NO_SON_FUNCIONES = {"if", "for", "while", "switch", "catch", "function", "return",
                     "else", "do", "with", "typeof"}

_FUNCION = re.compile(r"(?:^|[\s,;{}(])(?:function\s+)?([A-Za-z_$][\w$]*)\s*"
                      r"\(([^()]*)\)\s*\{")

# Un enlace entre dos literales que los pega en la MISMA trama: ' + x + '. Cualquier otra
# cosa entre medias -una coma, un ':' de ternario, un parentesis- son dos tramas
# distintas y se dejan separadas a proposito.
_PEGADOS = re.compile(r"^\s*\+\s*(?:[A-Za-z_$][\w$.\[\]]*\s*\+\s*)*$")

_LLAMADA_DE_RETORNO = re.compile(r"\breturn\s+(?:this\.)?([A-Za-z_$][\w$]*)\s*\("
                                 r"([^()]*)\)\s*;")


# ---------------------------------------------------------------------------------
# LECTURA DEL CONTRATO DEL CABLE, DEL C++ Y SOLO DEL C++
# ---------------------------------------------------------------------------------

def _prefijos_del_cable(fw, partes):
    """Los literales que el despachador compara contra `cmd`, o sea contra el byte 0.

    No hay tabla ni enum en el firmware: la cadena de strcmp/strncmp ES el contrato.
    Se leen tal cual, sin normalizar, porque lo que importa es con que empieza una
    trama que el equipo va a reconocer."""
    codigo = fw.codigo(*partes)
    return set(re.findall(r'\bstrn?cmp\s*\(\s*cmd\s*,\s*"([^"]+)"', codigo))


def _entrega_el_buffer_tal_cual(fw, partes):
    """El lazo de recepcion NO quita nada antes de comparar.

    Esta es la premisa de la que cuelga todo lo demas. Si manana el firmware
    desnatara el '$' y el checksum, una trama NMEA entraria perfectamente y este pack
    estaria acusando a la app de un defecto que ya no existe. Se comprueba en el .cpp,
    no se da por sabido."""
    codigo = fw.codigo(*partes)
    m = re.search(r"\bprocesarComando\s*\(\s*([A-Za-z_]\w*)\s*\)", codigo)
    if not m:
        return None, "no se hallo ninguna llamada a procesarComando(buffer)"
    buf = m.group(1)
    # El byte que se guarda tiene que ser el byte que se leyo, en la posicion que toca.
    if not re.search(r"\b%s\s*\[\s*\w+\s*\+\+\s*\]\s*=\s*([A-Za-z_]\w*)\s*;" % re.escape(buf),
                     codigo):
        return None, ("el buffer %s no se llena con el byte leido tal cual: el lazo de "
                      "recepcion hace algo por el camino" % buf)
    return buf, None


# ---------------------------------------------------------------------------------
# LOS MOLDES DE LA APP
# ---------------------------------------------------------------------------------

def _sin_comentarios(js):
    """Un molde que casa dentro de un comentario no llega al cable.

    Es la misma razon por la que fuente.codigo() existe para el C++: nmea_parser.js
    lleva el formato defectuoso escrito TAMBIEN en su comentario de cabecera, y
    contarlo dos veces -una real y una de prosa- inflaria el censo con un emisor que
    no emite."""
    js = re.sub(r"/\*.*?\*/", " ", js, flags=re.S)
    return re.sub(r"//[^\n]*", " ", js)


def _literales(js):
    """[(ini, fin, molde, nombres)] de cada literal de cadena o plantilla.

    El molde conserva el TEXTO FUENTE de los escapes -'\\r\\n' son cuatro caracteres,
    no dos-, que es exactamente lo que hay que comparar: lo que se mide es la forma
    que el programador escribio, no el byte que el motor produce."""
    fuera = []
    i, n = 0, len(js)
    while i < n:
        c = js[i]
        if c in "'\"":
            j = i + 1
            while j < n:
                if js[j] == "\\":
                    j += 2
                    continue
                if js[j] == c:
                    break
                if js[j] == "\n":       # cadena sin cerrar: no es un literal
                    break
                j += 1
            if j < n and js[j] == c:
                fuera.append((i, j + 1, js[i + 1:j], []))
                i = j + 1
                continue
            i += 1
            continue
        if c == "`":
            j = i + 1
            molde, nombres = [], []
            while j < n:
                if js[j] == "\\":
                    molde.append(js[j:j + 2])
                    j += 2
                    continue
                if js[j] == "`":
                    break
                if js[j] == "$" and j + 1 < n and js[j + 1] == "{":
                    prof, k = 1, j + 2
                    while k < n and prof:
                        if js[k] == "{":
                            prof += 1
                        elif js[k] == "}":
                            prof -= 1
                        k += 1
                    nombres.append(js[j + 2:k - 1].strip())
                    molde.append(HUECO)
                    j = k
                    continue
                molde.append(js[j])
                j += 1
            if j < n and js[j] == "`":
                fuera.append((i, j + 1, "".join(molde), nombres))
                i = j + 1
                continue
            i += 1
            continue
        i += 1
    return fuera


def _encadenar(js, literales):
    """Pega los literales que el '+' une en una sola trama.

        'CMD:' + comando + '\\r\\n'   ->   un molde: CMD:<>\\r\\n

    Sin esto el emisor vivo de app.js -que compone la orden sin PIN concatenando- se
    leeria como dos moldes sueltos, ninguno de los cuales tiene forma de trama, y el
    censo aprobaria por no haber encontrado nada que mirar."""
    fuera = []
    for ini, fin, molde, nombres in literales:
        if fuera:
            pini, pfin, pmolde, pnombres = fuera[-1]
            hueco = js[pfin:ini]
            if _PEGADOS.match(hueco):
                sueltos = re.findall(r"[A-Za-z_$][\w$.\[\]]*", hueco)
                fuera[-1] = (pini, fin,
                             pmolde + HUECO * len(sueltos) + molde,
                             pnombres + sueltos + nombres)
                continue
        fuera.append((ini, fin, molde, nombres))
    return fuera


def _bloque(texto, i):
    """El interior del bloque que abre en texto[i] == '{'. None si no cierra.

    Traido literal de app_03_sin_ok_mudo, que ya lo tenia probado. Reescribirlo para
    renombrar una variable es como se cuelan los errores en un cambio que no debe
    cambiar comportamiento."""
    if i < 0 or i >= len(texto) or texto[i] != "{":
        return None
    prof = 0
    for j in range(i, len(texto)):
        if texto[j] == "{":
            prof += 1
        elif texto[j] == "}":
            prof -= 1
            if prof == 0:
                return j
    return None


def _funciones(js):
    """{nombre: (params, ini_cuerpo, fin_cuerpo)} de las funciones del fichero."""
    fuera = {}
    for m in _FUNCION.finditer(js):
        nombre = m.group(1)
        if nombre in _NO_SON_FUNCIONES:
            continue
        abre = js.find("{", m.end() - 1)
        cierra = _bloque(js, abre)
        if cierra is None:
            continue
        params = [p.split("=")[0].strip() for p in m.group(2).split(",") if p.strip()]
        fuera[nombre] = (params, abre + 1, cierra)
    return fuera


def _duena(funciones, pos):
    """La funcion MAS INTERNA que contiene esa posicion, o None."""
    mejor, ancho = None, None
    for nombre, (_, ini, fin) in funciones.items():
        if ini <= pos < fin and (ancho is None or fin - ini < ancho):
            mejor, ancho = nombre, fin - ini
    return mejor


def _destino(js, ini, dentro):
    """El nombre al que se asigna el molde, o 'return' si se devuelve.

    Hace falta para componer: sin saber que la plantilla de CMD: acaba en una variable
    llamada `payload` no se puede seguir esa variable hasta la funcion que la envuelve.

    EL SEPARADOR DE SENTENCIA SE BUSCA FUERA DE LOS LITERALES, y no es un detalle: la
    plantilla `CMD:PIN:${pin}:${comando}` lleva llaves DENTRO, asi que un rfind('}') a
    secas se paraba en medio del literal anterior. En el ternario de generarComando()
    eso dejaba la segunda rama sin destino, la composicion no encontraba a que hueco
    meter el payload y el pack acusaba dos veces con un molde a medias -cierto en el
    veredicto y falso en el detalle, que es la peor clase de mensaje-."""
    corte = -1
    for j in range(ini - 1, -1, -1):
        if js[j] in ";{}\n" and not dentro[j]:
            corte = j
            break
    cabeza = js[corte + 1:ini]
    if re.search(r"\breturn\b", cabeza):
        return "return"
    m = re.search(r"([A-Za-z_$][\w$]*)\s*\+?=[^=]*$", cabeza)
    return m.group(1) if m else None


def _terminada(molde):
    """La trama lleva el terminador que el lazo de recepcion reconoce."""
    return molde.endswith("\\n") or molde.endswith("\\r")


def _emisores(js):
    """Los moldes de trama de comando del fichero, compuestos hasta el cable.

    Devuelve [(funcion, molde_final, como)] donde `como` explica el camino, para que
    el mensaje de fallo diga DONDE mirar y no solo que algo esta mal."""
    js = _sin_comentarios(js)
    crudos = _literales(js)
    # Mascara de "esto es texto de un literal, no programa". La usa _destino() para no
    # confundir una llave de `${...}` con el final de la sentencia anterior.
    dentro = bytearray(len(js))
    for ini, fin, _, _ in crudos:
        for j in range(ini, fin):
            dentro[j] = 1
    lits = _encadenar(js, crudos)
    funcs = _funciones(js)

    moldes = []
    for ini, fin, molde, nombres in lits:
        moldes.append({"ini": ini, "molde": molde, "nombres": nombres,
                       "fn": _duena(funcs, ini), "dest": _destino(js, ini, dentro)})

    # Los moldes que una funcion devuelve tal cual: son los que envuelven a otros.
    devueltos = {}
    for m in moldes:
        if m["fn"] and m["dest"] == "return":
            devueltos.setdefault(m["fn"], m)

    fuera = []
    for m in moldes:
        if "CMD:" not in m["molde"]:
            continue
        if _terminada(m["molde"]):
            fuera.append((m["fn"], m["molde"], "la compone y la termina"))
            continue
        # La trama se acaba en otra parte. Se sigue UN nivel: el `return G(x)` de la
        # misma funcion, con G definida en el mismo fichero.
        if not m["fn"]:
            continue
        _, ini, fin = funcs[m["fn"]]
        llam = _LLAMADA_DE_RETORNO.search(js[ini:fin])
        if not llam:
            fuera.append((m["fn"], m["molde"], "compone un payload sin terminador y no "
                                               "se pudo seguir hasta el cable"))
            continue
        g = llam.group(1)
        args = [a.strip() for a in llam.group(2).split(",") if a.strip()]
        if g not in funcs or g not in devueltos:
            fuera.append((m["fn"], m["molde"], "entrega el payload a %s(), que no "
                                               "compone ninguna trama legible" % g))
            continue
        gparams, _, _ = funcs[g]
        envoltura = devueltos[g]
        # El hueco de G que corresponde al argumento con el que se le pasa este molde.
        compuesto, k = [], 0
        for trozo in envoltura["molde"].split(HUECO):
            compuesto.append(trozo)
            if k < len(envoltura["nombres"]):
                nombre = envoltura["nombres"][k]
                pos = gparams.index(nombre) if nombre in gparams else -1
                if pos >= 0 and pos < len(args) and args[pos] == m["dest"]:
                    compuesto.append(m["molde"])
                else:
                    compuesto.append(HUECO)
                k += 1
        fuera.append((m["fn"], "".join(compuesto),
                      "compone el payload y lo envuelve con %s()" % g))
    # Las dos ramas de un ternario componen la MISMA trama con distinto numero de
    # campos: contarlas dos veces inflaria el total con la misma comprobacion.
    vistos, unicos = set(), []
    for e in fuera:
        if e not in vistos:
            vistos.add(e)
            unicos.append(e)
    return unicos


def _entra_en_el_equipo(molde, prefijos):
    """(entra, motivo). El molde tiene la forma que el despachador sabe comparar."""
    cuerpo = molde
    while _terminada(cuerpo):
        cuerpo = cuerpo[:-2]
    if not cuerpo:
        return False, "la trama esta vacia"
    # El comienzo literal, hasta el primer hueco: es lo unico que el despachador puede
    # comparar sin conocer los valores.
    cabeza = cuerpo.split(HUECO)[0]
    if not any(cabeza.startswith(p) or p.startswith(cabeza) for p in prefijos):
        return False, ("empieza por %r y el despachador solo compara %s desde el byte "
                       "0: no casa ni un strcmp y la orden entera cae en "
                       "$ERR,CMD:AUTH_FAILED" % (cabeza, sorted(prefijos)))
    # Y nada colgando detras: el firmware hace strcmp EXACTO contra la accion, asi que
    # un checksum pegado al final convierte SET_MODO:AUTO en SET_MODO:AUTO*7A, que no
    # es ninguna de las acciones que conoce.
    if "*" in cuerpo:
        return False, ("lleva un '*' de checksum pegado a la trama, y el despachador "
                       "compara la accion con strcmp EXACTO: con el checksum detras no "
                       "casa ninguna accion y cae en $ERR,CMD:DESCONOCIDO")
    return True, None


# ---------------------------------------------------------------------------------
# EL CENSO DE MODULOS
# ---------------------------------------------------------------------------------

def _scripts(html):
    """Los .js locales que index.html carga, en orden. El censo sale de aqui y no de
    una lista: un modulo nuevo entra solo, y uno retirado sale solo."""
    fuera = []
    for src in re.findall(r'<script[^>]*\bsrc="([^"]+)"', html):
        if re.match(r"^[A-Za-z0-9_./-]+\.js$", src) and not src.startswith("/"):
            fuera.append(src)
    return fuera


def _exporta(js):
    """El identificador que ese modulo PUBLICA, leido de su propio pie de fichero.

    Los cinco modulos de js/ acaban en `module.exports = X;`, que es donde cada uno
    declara como se llama. Buscar en cambio "el primer const en mayusculas" -que fue el
    primer intento- da por modulo cualquier constante local: en app.js sacaba SIN_PIN,
    que es una lista de literales dentro de una IIFE, y el pack acusaba a la app de
    cargar un modulo huerfano que no existe. app.js no exporta nada porque no es un
    modulo: es el que los consume, y asi queda fuera del censo por su propio contenido
    y no por una excepcion escrita a mano."""
    m = re.search(r"\bmodule\.exports\s*=\s*([A-Za-z_$][\w$]*)\s*;", js)
    return m.group(1) if m else None


def _usos(nombre, texto):
    """Las veces que alguien USA el modulo: X.algo, new X, o X( .

    La declaracion propia no cuenta -es `const X = {`- y por eso se exige que detras
    del nombre venga un punto, un parentesis o que delante venga `new`."""
    return (len(re.findall(r"\b%s\s*\." % re.escape(nombre), texto))
            + len(re.findall(r"\bnew\s+%s\b" % re.escape(nombre), texto)))


# ---------------------------------------------------------------------------------

def correr(b, fw):
    b.titulo("Un solo generador de trama, y con la forma que el equipo compara")

    # ---- 1. El contrato del cable, leido del C++ ----
    prefijos = _prefijos_del_cable(fw, BT_MAESTRO) | _prefijos_del_cable(fw, BT_ESCLAVO)
    if not prefijos:
        raise fw.Abortado(
            "no se leyo del C++ ni un literal comparado contra `cmd`. Ese conjunto ES "
            "el contrato del cable: con el vacio este pack aprobaria cualquier trama, "
            "incluida una que ninguna punta reconoce")

    for partes in (BT_MAESTRO, BT_ESCLAVO):
        buf, motivo = _entrega_el_buffer_tal_cual(fw, partes)
        if buf is None:
            raise fw.Abortado(
                "%s: %s. Todo lo que este pack acusa cuelga de que el despachador "
                "compare la trama TAL CUAL llega; si el firmware desnatara el '$' y el "
                "checksum, la trama NMEA entraria y las acusaciones de abajo serian "
                "falsas" % (partes[0], motivo))

    b.verificar(
        all(p.startswith("CMD:") for p in prefijos),
        "contrato leido del C++: la trama que el equipo compara empieza por %s, y el "
        "buffer llega sin desnatar en las dos puntas" % sorted(prefijos),
        "los despachadores comparan %s: ya no hay un comienzo comun, asi que la app no "
        "puede tener un solo generador aunque quiera" % sorted(prefijos))

    # ---- 2. Los modulos que index.html carga ----
    html = fw.texto_repo(*APP_HTML)
    srcs = _scripts(html)
    if len(srcs) < 2:
        raise fw.Abortado(
            "index.html declara %d <script src> locales. La app se reparte en varios "
            "modulos desde N-75: con uno o ninguno el censo no esta leyendo el HTML y "
            "aprobaria sin haber mirado un solo fichero" % len(srcs))

    fuentes = {}
    for src in srcs:
        fuentes[src] = fw.texto_repo("05_Funcional", "App_Semaforo", *src.split("/"))

    # ---- 3. LA PROPIEDAD DURA: todos los emisores producen la MISMA trama ----
    emisores = []
    for src, js in fuentes.items():
        for fn, molde, como in _emisores(js):
            emisores.append((src, fn, molde, como))

    if not emisores:
        raise fw.Abortado(
            "no se hallo en los %d modulos de la app ni un solo molde de trama con "
            "'CMD:'. La app manda comandos desde antes de N-75: fallo el buscador, no "
            "la app, y medir cero emisores saldria en verde" % len(fuentes))

    for src, fn, molde, como in emisores:
        entra, motivo = _entra_en_el_equipo(molde, prefijos)
        legible = molde.replace(HUECO, "<>")
        b.verificar(
            entra,
            "%s / %s(): %s -> %r, que es la forma que el despachador compara"
            % (src, fn, como, legible),
            "%s / %s(): %s y la trama que sale es %r, que %s. Hay DOS generadores de "
            "trama en el arbol y no coinciden: el que esta vivo y este. Quien escriba "
            "un puente abrira el fichero con nombre de especificacion, no el que de "
            "verdad emite" % (src, fn, como, legible, motivo))

    # ---- 3.bis Y LA PROPIEDAD QUE IMPORTA MAS QUE LA ORFANDAD ----
    #
    # Que cada emisor por separado entre en el equipo no es lo mismo que que los
    # emisores COINCIDAN, y esta es la que no es un trinquete: dos formatos
    # incompatibles en el mismo arbol no tienen version legitima.
    #
    # "Coincidir" NO es "ser identicos". El firmware acepta a proposito dos puertas
    # -CMD:X para las ordenes sin PIN y CMD:PIN:1234:X para las demas, y el porque
    # esta razonado en bluetooth.cpp-, asi que la app compone las dos y hace bien. Lo
    # que se exige es que un comienzo sea PREFIJO DEL OTRO: eso distingue dos puertas
    # de la misma gramatica de dos gramaticas distintas, que es justo lo que separa
    # 'CMD:' de 'CMD:PIN:' -compatibles- de '$' contra 'CMD:' -incompatibles-.
    cabezas = sorted({m.split(HUECO)[0] for _, _, m, _ in emisores})
    renidas = sorted((a, c) for a in cabezas for c in cabezas
                     if a < c and not (a.startswith(c) or c.startswith(a)))
    b.verificar(
        not renidas,
        "los %d emisores de la app componen tramas de la MISMA gramatica: %s"
        % (len(emisores), cabezas),
        "hay comienzos de trama INCOMPATIBLES en el mismo arbol: %s. No es que uno "
        "tenga mas campos que otro -eso son dos puertas de la misma gramatica-: es que "
        "no pueden ser los dos el formato del cable, uno entra y el otro no, y nada en "
        "el arbol dice cual" % ["%r vs %r" % p for p in renidas])

    # ---- 4. El trinquete de modulos huerfanos ----
    modulos = {}
    for src, js in fuentes.items():
        nombre = _exporta(js)
        if nombre:
            modulos[nombre] = src

    if not modulos:
        raise fw.Abortado(
            "ninguno de los %d modulos de la app publica un identificador de primer "
            "nivel reconocible. El censo de huerfanos compara ese nombre contra los "
            "demas ficheros; sin nombres no compara nada" % len(fuentes))

    llamadas = {}
    for nombre, propio in modulos.items():
        llamadas[nombre] = sum(_usos(nombre, js) for src, js in fuentes.items()
                               if src != propio)

    # CONTROL POSITIVO, y no es adorno: si NINGUN modulo tuviera usos, la respuesta
    # correcta no es "todos son huerfanos" sino "el buscador no sabe encontrar". Un
    # "no aparece" no es un hallazgo hasta haber descartado al buscador.
    if not any(llamadas.values()):
        raise fw.Abortado(
            "el censo no halla NI UN uso de NINGUNO de los %d modulos. Alguno se usa "
            "-SiteManager y CourierRTC estan llamados desde app.js-: fallo el buscador"
            % len(modulos))

    huerfanos = {n for n, c in llamadas.items() if c == 0}
    nuevos = sorted(huerfanos - set(HUERFANOS_CONOCIDOS))
    b.verificar(
        not nuevos,
        "%d modulos cargados por index.html, %d sin un solo llamador y todos conocidos"
        % (len(modulos), len(huerfanos)),
        "index.html carga %s y NADIE los usa, y no estaban en la lista. O es codigo "
        "recien escrito que no se ejecuta, o -peor- algo que si se usaba y acaba de "
        "quedarse sin llamador mientras los documentos lo siguen describiendo"
        % ", ".join(nuevos))

    revividos = sorted(n for n in HUERFANOS_CONOCIDOS
                       if n in llamadas and llamadas[n] > 0)
    b.verificar(
        not revividos,
        "ninguno de los %d huerfanos conocidos ha ganado llamador"
        % len(HUERFANOS_CONOCIDOS),
        "%s YA se usa desde la app y sigue en la lista de huerfanos conocidos. Hay que "
        "sacarlo: una lista que acumula nombres obsoletos deja de poder fallar, que es "
        "la unica forma que tiene de servir para algo" % ", ".join(revividos))

    desaparecidos = sorted(n for n in HUERFANOS_CONOCIDOS if n not in modulos)
    b.verificar(
        not desaparecidos,
        "los %d huerfanos de la lista siguen cargandose desde index.html"
        % len(HUERFANOS_CONOCIDOS),
        "%s esta en la lista de huerfanos conocidos y index.html ya no lo carga. Se "
        "retiro el modulo y no la lista: queda vigilando el aire"
        % ", ".join(desaparecidos))

    # ---- 5. Lo que acompana al hallazgo y no cuenta como comprobacion ----
    #
    # Va en reportar() a proposito: no es una propiedad del firmware ni de la app, es
    # un aviso para cuando alguien arregle lo de arriba. CLAUDE.md 8.quater -al
    # arreglar un defecto, busca las pruebas que lo celebraban- y aqui hay tres.
    if any(not _entra_en_el_equipo(m, prefijos)[0] for _, _, m, _ in emisores):
        b.reportar(
            "las pruebas que certifican la trama mala usan includes(), que no puede "
            "detectar un envoltorio",
            ["tests/test_unitarios.js afirma tres veces cmdX.includes('CMD:PIN:1234:...')",
             "includes() es cierto tambien de $CMD:PIN:1234:...*XX, asi que la prueba",
             "estaba verde midiendo que el payload esta DENTRO, no que la trama sea esa.",
             "Al corregir el generador hay que revisarlas una a una (CLAUDE.md 8.quater):",
             "una comparacion exacta contra la trama entera si sabe distinguirlo."])

    # ---- 6. Controles negativos ----
    b.control_negativo(
        _entra_en_el_equipo("$CMD:PIN:1234:SET_MODO:AUTO*7A\\r\\n", prefijos)[0] is False,
        "una trama con el '$' delante y el checksum detras se detecta como incapaz de "
        "entrar en el equipo")
    b.control_negativo(
        _entra_en_el_equipo("CMD:PIN:" + HUECO + ":" + HUECO + "\\r\\n", prefijos)[0],
        "y la trama que la app manda de verdad NO se marca: el detector distingue, no "
        "acusa a todo el que compone una orden")
    b.control_negativo(
        _emisores("function f(p){ return g(p); }\n"
                  "function g(x){ return `$${x}*00\\r\\n`; }\n"
                  "function h(){ const p = `CMD:PIN:${n}:${c}`; return f(p); }")
        != [],
        "el compositor sigue el payload UN nivel hasta la funcion que lo envuelve: sin "
        "eso el defecto es invisible, porque el payload por si solo es correcto")
    b.control_negativo(
        _usos("NoExiste", "const NoExiste = {}; algo.otro();") == 0,
        "el contador de usos no encuentra un modulo que nadie toca")
    b.control_negativo(
        _usos("SiteManager", "SiteManager.filtrarCruces(x);") == 1,
        "y SI cuenta un uso real, que es lo que separa un huerfano de un modulo vivo")
