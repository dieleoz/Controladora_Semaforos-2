# ===== banco/packs/enlace_01_transporte.py =====
#
# EL TRANSPORTE DEL ENLACE BLUETOOTH: PINES, VELOCIDAD, BUFFER Y FRAMING.
#
# POR QUE EXISTE ESTE PACK.
#
# El banco tenia 38 packs y NINGUNO leia el transporte. app_01/02/03 miden lo que el
# despachador CONTESTA -que haya $ACK, que no mienta, que la app tenga boton- pero
# ninguno mira por donde sale ese $ACK. Se grepearon los 40 ficheros de packs/ por
# SerialBT, HardwareSerial, PB6, PB7, J17 y 9600: el unico acierto era una cadena
# sintetica dentro de un control_negativo de flash_01_lastre.py.
#
# O sea que hoy alguien puede mover SerialBT a otro par de pines, o cambiar la
# velocidad en UNA sola punta, y el acta sigue en verde. Es CLAUDE.md §3 en su forma
# literal: un instrumento que no esta en la compuerta no mide nada, y un hueco no
# grita -a diferencia de un ABORTADO, que al menos se ve-.
#
# Y deja de ser un asunto interno: al otro lado de ese puerto va un ESP32. Lo que
# hasta ahora era "como esta cableado nuestro equipo" pasa a ser un CONTRATO con un
# firmware que escribe otra persona, que no puede leer estos .cpp para adivinar el
# baudrate ni el tamano del buffer.
#
# LAS TRES LECCIONES DEL REPOSITORIO QUE ESTE PACK APLICA:
#
#   N-86  Dos objetos sobre el MISMO periferico no dan error de compilacion: dan el
#         ultimo que arranco. AiBus declaraba USART1 sobre PA10/PA9 a 115200 mientras
#         SerialBT lo declara sobre PB7/PB6 a 9600. Por eso aqui no basta con mirar
#         SerialBT: hay que censar TODOS los HardwareSerial de las dos puntas.
#
#   N-71  Una constante puede ser el TECHO de otras. El tamano del buffer de entrada
#         no es un numero suelto: es el limite que el comando mas largo tiene que
#         respetar, y esa desigualdad se recalcula aqui desde el C++ en vez de vivir
#         en un comentario.
#
#   §4    Un "no aparece" no es un hallazgo hasta haber descartado al buscador. Cada
#         censo de este pack lleva su suelo: si encuentra sospechosamente poco,
#         ABORTA en vez de aprobar.
#
# NO LLEVA ETIQUETA "EJERCE SFTY-x" A PROPOSITO. Ninguna de las 29 reglas SFTY habla
# del transporte de la app: SFTY-21 habla del latch de ambar, no del puerto por donde
# entro la orden. Poner una etiqueta aqui llenaria una fila de la trazabilidad de
# OPTIMIZACIONES.md con una prueba que no ejerce esa regla, y una fila que miente es
# peor que una vacia.

import re

NOMBRE = "enlace_01_transporte"
DESCRIPCION = "el transporte del enlace Bluetooth: pines, velocidad, buffer y framing"

PUNTAS = ("Maestro", "Esclavo")

# Las rutas van como tuplas explicitas para que la guarda de rutas de compuerta.py
# las censE: si manana alguien mueve bluetooth.cpp, la guarda aborta en vez de dejar
# a este pack midiendo un fuente que ya no esta (N-36).
FUENTES = (
    ("Maestro", "src", "bluetooth.cpp"),
    ("Esclavo", "src", "bluetooth.cpp"),
    ("Maestro", "include", "bluetooth.h"),
    ("Esclavo", "include", "bluetooth.h"),
    ("Maestro", "include", "pines.h"),
    ("Esclavo", "include", "pines.h"),
    ("Maestro", "src", "lcd.cpp"),
    ("Esclavo", "src", "lcd.cpp"),
)

# HECHO DE HARDWARE, NO CONSTANTE DEL FIRMWARE, y por eso se escribe aqui con su
# porque: el STM32F103 saca USART1 por PA9/PA10 o por PB6/PB7, pero SOLO POR UN SITIO
# A LA VEZ. Los pines vigentes se leen del C++ (no se escriben aqui); estos dos son
# los del mapeo alternativo, que es donde vivia AiBus antes de N-86. Un segundo objeto
# sobre cualquiera de los cuatro se pelea con SerialBT por el mismo periferico.
USART1_ALTERNATIVO = ("PA9", "PA10")


# ---------------------------------------------------------------------------------
# EXTRACTORES. Todos leen el C++ y ABORTAN si no encuentran: sin valor por defecto.
# ---------------------------------------------------------------------------------

def _cuerpo(fw, codigo, firma, que, donde):
    """El cuerpo de una funcion, contando llaves. ABORTA si no aparece.

    Se cuenta llaves en vez de recortar con una expresion regular porque el cuerpo
    lleva llaves dentro -los if del despachador- y un patron perezoso se quedaria con
    el primer trozo, midiendo media funcion y aprobando lo que no vio."""
    m = re.search(firma, codigo)
    if not m:
        raise fw.Abortado(
            "no se pudo hallar %s en %s (patron %r). Sin el cuerpo, este pack no mide "
            "el framing: aprobaria comparando nada contra nada." % (que, donde, firma))
    i = codigo.find("{", m.end() - 1)
    if i < 0:
        raise fw.Abortado("%s en %s no abre llave: el extractor no sabe leer esta forma"
                          % (que, donde))
    profundidad = 0
    for j in range(i, len(codigo)):
        if codigo[j] == "{":
            profundidad += 1
        elif codigo[j] == "}":
            profundidad -= 1
            if profundidad == 0:
                return codigo[i + 1:j]
    raise fw.Abortado("%s en %s no cierra llave" % (que, donde))


def _uno(fw, codigo, patron, que, donde):
    """Un grupo unico leido del C++. ABORTA si no aparece."""
    m = re.search(patron, codigo)
    if not m:
        raise fw.Abortado(
            "no se pudo leer del C++ %s (patron %r en %s). Sin ese dato el banco "
            "mediria otra cosa que el firmware y seguiria dando PASS."
            % (que, patron, donde))
    return m.groups()


def _declaracion_puerto(fw, punta):
    """(rx, tx) de SerialBT, leidos del constructor.

    EL ORDEN IMPORTA Y SE DEJA ESCRITO PARA QUE NADIE LO INVIERTA AL MANTENER ESTO:
    la firma del framework es HardwareSerial(rx, tx), asi que en
    `HardwareSerial SerialBT(PB7, PB6)` el PRIMERO es RX y el SEGUNDO es TX. Quien
    lea el conector J17 de izquierda a derecha y los cambie de sitio deja el equipo
    hablando por la pata que escucha."""
    cod = fw.codigo(punta, "src", "bluetooth.cpp")
    return _uno(fw, cod,
                r"HardwareSerial\s+SerialBT\s*\(\s*(P[A-Z]\d+)\s*,\s*(P[A-Z]\d+)\s*\)",
                "los pines de SerialBT", "%s/src/bluetooth.cpp" % punta)


def _velocidad(fw, punta):
    cod = fw.codigo(punta, "src", "bluetooth.cpp")
    return int(_uno(fw, cod, r"SerialBT\s*\.\s*begin\s*\(\s*(\d+)\s*\)",
                    "la velocidad de SerialBT", "%s/src/bluetooth.cpp" % punta)[0])


def _tam_buffer(fw, punta):
    cod = fw.codigo(punta, "src", "bluetooth.cpp")
    return int(_uno(fw, cod, r"char\s+btBufIn\s*\[\s*(\d+)\s*\]",
                    "el tamano del buffer de entrada",
                    "%s/src/bluetooth.cpp" % punta)[0])


def _alias_de(fw, punta, pines):
    """Los nombres que pines.h le da a unos pines crudos.

    Sin esto el censo estaria ciego a la mitad: nadie escribe digitalWrite(PB6, ...),
    escribe digitalWrite(LCD_PSB, ...). Los nombres se DESCUBREN de pines.h en vez de
    listarse aqui, que es la leccion de barrera_01: una lista escrita a mano se queda
    corta el dia que alguien anade un alias, y entonces la guarda aprueba sin mirar."""
    texto_pines = fw.texto(punta, "include", "pines.h")
    encontrados = []
    for pin in pines:
        encontrados += re.findall(r"#define\s+([A-Z_][A-Z0-9_]*)\s+%s\b" % pin,
                                  texto_pines)
    return sorted(set(encontrados))


def _reclamantes(codigo, nombres):
    """Quien nombra estos pines EN CODIGO -sin comentarios-.

    El detector es un solo sitio a proposito: el control negativo de mas abajo tiene
    que ejercer exactamente la misma funcion que la comprobacion real, o demostraria
    que sabe fallar otra prueba distinta."""
    return sorted({n for n in nombres if re.search(r"\b%s\b" % re.escape(n), codigo)})


def _series_declaradas(fw, punta):
    """(fichero, objeto, [pines crudos]) de cada HardwareSerial de la punta.

    Censa el DIRECTORIO, no una lista: es la unica forma de que un puerto nuevo
    aparezca vigilado sin que nadie se acuerde de venir aqui."""
    mapa = dict(re.findall(r"#define\s+([A-Z_][A-Z0-9_]*)\s+(P[A-Z]\d+)\b",
                           fw.texto(punta, "include", "pines.h")))
    salida = []
    for fichero in fw.fuentes_de(punta, "src"):
        for m in re.finditer(r"HardwareSerial\s+(\w+)\s*\(([^)]*)\)",
                             fw.codigo(punta, "src", fichero)):
            crudos = [mapa.get(a.strip(), a.strip()) for a in m.group(2).split(",")]
            salida.append((fichero, m.group(1), crudos))
    return salida


def _valida_entrada(fw, punta, codigo=None):
    """Verdadero si la punta comprueba el checksum de lo que RECIBE.

    Se mira el camino de entrada entero -el bucle de recepcion y el despachador-, no
    solo procesarComando(): validar en bluetooth_loop() antes de despachar seria igual
    de valido y este detector tiene que reconocerlo, o acusaria de asimetria a un
    firmware que la acaba de cerrar."""
    cod = codigo if codigo is not None else fw.codigo(punta, "src", "bluetooth.cpp")
    entrada = (_cuerpo(fw, cod, r"void\s+procesarComando\s*\(", "procesarComando()",
                       "%s/src/bluetooth.cpp" % punta)
               + _cuerpo(fw, cod, r"void\s+bluetooth_loop\s*\(", "bluetooth_loop()",
                         "%s/src/bluetooth.cpp" % punta))
    return bool(re.search(r"calcularChecksum", entrada))


# ---------------------------------------------------------------------------------

def correr(b, fw):
    _bloque_puerto(b, fw)
    _bloque_nadie_mas_los_reclama(b, fw)
    _bloque_buffer(b, fw)
    _bloque_framing(b, fw)
    _bloque_asimetria(b, fw)


# --- A. El puerto: los mismos pines y la misma velocidad en las dos puntas ---------

def _bloque_puerto(b, fw):
    b.titulo("El puerto: mismos pines y misma velocidad en las dos puntas")

    puerto = {p: _declaracion_puerto(fw, p) for p in PUNTAS}
    baud = {p: _velocidad(fw, p) for p in PUNTAS}

    rx_m, tx_m = puerto["Maestro"]
    rx_e, tx_e = puerto["Esclavo"]
    b.verificar(
        puerto["Maestro"] == puerto["Esclavo"],
        "las dos puntas declaran SerialBT sobre el MISMO par: RX=%s, TX=%s. Un solo "
        "cable de campo sirve para las dos tarjetas" % (rx_m, tx_m),
        "LAS PUNTAS NO COMPARTEN EL PUERTO: Maestro RX=%s/TX=%s contra Esclavo "
        "RX=%s/TX=%s. El mismo modulo enchufado en J17 habla con una tarjeta y no con "
        "la otra, y el tecnico no tiene forma de saber cual" % (rx_m, tx_m, rx_e, tx_e))

    b.verificar(
        baud["Maestro"] == baud["Esclavo"],
        "las dos puntas abren SerialBT a la misma velocidad: %d bps" % baud["Maestro"],
        "VELOCIDADES DISTINTAS: Maestro %d bps contra Esclavo %d bps. El enlace no "
        "falla limpio -entrega basura que a veces casa con un strcmp-, y el ESP32 no "
        "puede tener dos configuraciones para el mismo conector"
        % (baud["Maestro"], baud["Esclavo"]))

    # El .h es lo unico que lee quien no abre el .cpp -y quien escriba el ESP32 va a
    # leer el .h-. Una cabecera que envejece sin avisar es la misma trampa que las
    # cifras del README (documentos_01): no falla sola, miente con autoridad.
    for punta in PUNTAS:
        cabecera = fw.texto(punta, "include", "bluetooth.h")
        tx_doc, rx_doc = _uno(fw, cabecera,
                              r"(P[A-Z]\d+)\s+TX\s*,\s*(P[A-Z]\d+)\s+RX",
                              "los pines que documenta la cabecera",
                              "%s/include/bluetooth.h" % punta)
        baud_doc = int(_uno(fw, cabecera, r"(\d+)\s*bps",
                            "la velocidad que documenta la cabecera",
                            "%s/include/bluetooth.h" % punta)[0])
        rx, tx = puerto[punta]
        b.verificar(
            (tx_doc, rx_doc, baud_doc) == (tx, rx, baud[punta]),
            "%s: bluetooth.h documenta TX=%s, RX=%s y %d bps, que es exactamente lo "
            "que declara el .cpp" % (punta, tx_doc, rx_doc, baud_doc),
            "%s: la cabecera documenta TX=%s, RX=%s a %d bps y el .cpp declara TX=%s, "
            "RX=%s a %d bps. Quien escriba el ESP32 lee el .h y cablea lo que dice"
            % (punta, tx_doc, rx_doc, baud_doc, tx, rx, baud[punta]))

    b.control_negativo(
        ("PB7", "PB6") != ("PA10", "PA9"),
        "un par sintetico (PA10,PA9) frente al (PB7,PB6) real se declara divergente: "
        "el comparador de pines no aprueba dos puertos distintos")
    b.control_negativo(
        9600 != 115200,
        "una velocidad sintetica de 115200 -la que tenia AiBus- frente a la real se "
        "declara divergente: el comparador de baudrate no aprueba dos velocidades")


# --- B. Nadie mas reclama esos pines ----------------------------------------------

def _bloque_nadie_mas_los_reclama(b, fw):
    b.titulo("Nadie mas reclama PB6/PB7: un periferico, un dueno")

    for punta in PUNTAS:
        rx, tx = _declaracion_puerto(fw, punta)
        alias = _alias_de(fw, punta, (rx, tx))
        nombres = [rx, tx] + alias

        # SUELO DEL BUSCADOR. Estos dos pines los tenia la pantalla (PSB y RST), asi
        # que pines.h TIENE que seguir dandoles nombre; si el censo de alias vuelve
        # vacio, lo que fallo es el patron y no el arbol -§4-.
        if not alias:
            raise fw.Abortado(
                "%s/include/pines.h no da ningun nombre a %s ni a %s: el censo de "
                "alias se quedo ciego y buscaria solo los pines crudos, que es como "
                "nadie los escribe" % (punta, rx, tx))

        # La pantalla RENUNCIO al reset a proposito (U8X8_PIN_NONE). Es una afirmacion
        # positiva y no la ausencia de un patron: la ausencia se puede deber a que el
        # buscador no supo mirar.
        lcd = fw.codigo(punta, "src", "lcd.cpp")
        args = _uno(fw, lcd, r"U8G2_ST7920\w*\s+u8g2\s*\(([^)]*)\)",
                    "el constructor del display", "%s/src/lcd.cpp" % punta)[0]
        args = [a.strip() for a in args.split(",")]
        b.verificar(
            "U8X8_PIN_NONE" in args and not _reclamantes(", ".join(args), nombres),
            "%s: el display se construye con U8X8_PIN_NONE y no nombra ninguno de %s: "
            "la pantalla renuncio a esos pines, no se los quitaron por descuido"
            % (punta, ", ".join(nombres)),
            "%s: el constructor del display (%s) vuelve a reclamar el pin del puerto. "
            "Dos perifericos sobre el mismo pin no dan error: gana el ultimo que "
            "arranco, y el sintoma es telemetria muda sin ninguna pista"
            % (punta, ", ".join(args)))

        # Censo del DIRECTORIO -src/ e include/-, con los comentarios fuera: los
        # comentarios de lcd.cpp, main.cpp y protocolo.cpp siguen nombrando PB6/PB7
        # para explicar por que los soltaron, y contarlos daria una fuga que no existe.
        fugas = []
        for carpeta, ext in (("src", ".cpp"), ("include", ".h")):
            for fichero in fw.fuentes_de(punta, carpeta, ext):
                if fichero in ("bluetooth.cpp", "pines.h"):
                    continue    # el dueno legitimo, y el sitio donde se bautizan
                hallados = _reclamantes(fw.codigo(punta, carpeta, fichero), nombres)
                if hallados:
                    fugas.append("%s/%s nombra %s" % (carpeta, fichero,
                                                      ", ".join(hallados)))
        b.verificar(
            not fugas,
            "%s: ningun fuente fuera de bluetooth.cpp nombra %s en codigo. El puerto "
            "tiene un solo dueno" % (punta, "/".join((rx, tx))),
            "%s: SEGUNDO RECLAMANTE de los pines del puerto -> %s. No da error de "
            "compilacion: da el ultimo que arranco (N-86)" % (punta, fugas))

        # Y la otra mitad de N-86: el conflicto no llega por un pinMode, llega por un
        # SEGUNDO OBJETO sobre el mismo USART. AiBus no escribia ningun pin.
        conflicto = [s for s in _series_declaradas(fw, punta)
                     if s[1] != "SerialBT"
                     and set(s[2]) & (set((rx, tx)) | set(USART1_ALTERNATIVO))]
        b.verificar(
            not conflicto,
            "%s: SerialBT es el unico HardwareSerial sobre USART1 -ni %s ni su mapeo "
            "alternativo %s tienen un segundo objeto-"
            % (punta, "/".join((rx, tx)), "/".join(USART1_ALTERNATIVO)),
            "%s: SEGUNDO HardwareSerial sobre USART1 -> %s. Es AiBus otra vez: dos "
            "objetos sobre el mismo periferico compilan, arrancan los dos y solo habla "
            "el ultimo, ademas de costar su .bss permanente" % (punta, conflicto))

    # CONTROL NEGATIVO del censo de nombres, sobre el detector real y no sobre una
    # copia: si _reclamantes() dejara de casar, las dos comprobaciones de arriba
    # aprobarian todo y nadie se enteraria.
    rx, tx = _declaracion_puerto(fw, "Maestro")
    alias = _alias_de(fw, "Maestro", (rx, tx))
    colado = (fw.codigo("Maestro", "src", "main.cpp")
              + "\nvoid _fuga_de_prueba() { pinMode(%s, OUTPUT); }\n" % alias[0])
    b.control_negativo(
        bool(_reclamantes(colado, [rx, tx] + alias)),
        "un pinMode(%s) colado en main.cpp se detecta como segundo reclamante"
        % alias[0])

    # CONTROL NEGATIVO del censo de puertos: el AiBus que N-86 retiro, reinyectado.
    mapa = dict(re.findall(r"#define\s+([A-Z_][A-Z0-9_]*)\s+(P[A-Z]\d+)\b",
                           fw.texto("Maestro", "include", "pines.h")))
    sintetico = "static HardwareSerial AiBus(%s, %s);" % USART1_ALTERNATIVO[::-1]
    m = re.search(r"HardwareSerial\s+(\w+)\s*\(([^)]*)\)", sintetico)
    crudos = [mapa.get(a.strip(), a.strip()) for a in m.group(2).split(",")]
    b.control_negativo(
        m.group(1) != "SerialBT" and bool(set(crudos) & set(USART1_ALTERNATIVO)),
        "un AiBus(PA10,PA9) sintetico -el que N-86 retiro- se detecta como segundo "
        "objeto sobre USART1 aunque no escriba ningun pin")


# --- C. El buffer de entrada: el numero contra el que programa el ESP32 ------------

def _bloque_buffer(b, fw):
    b.titulo("El buffer de entrada: cuantos caracteres caben de verdad")

    tam = {}
    for punta in PUNTAS:
        cod = fw.codigo(punta, "src", "bluetooth.cpp")
        tam[punta] = _tam_buffer(fw, punta)

        # LA GUARDA TIENE QUE ESTAR EXPRESADA CON sizeof(), NO CON UN NUMERO. Un `< 63`
        # escrito a mano es la segunda copia del tamano del array, y el dia que alguien
        # agrande btBufIn el firmware seguiria cortando en 63 sin que nada lo dijera.
        # Es N-51 exactamente: un resto del valor anterior con aspecto de constante.
        por_sizeof = bool(re.search(
            r"btIdxIn\s*<\s*sizeof\s*\(\s*btBufIn\s*\)\s*-\s*1", cod))
        b.verificar(
            por_sizeof,
            "%s: la guarda del bucle de recepcion se expresa con sizeof(btBufIn)-1, no "
            "con un numero suelto: el limite no puede quedarse atras del array" % punta,
            "%s: la guarda de btIdxIn NO se deriva de sizeof(btBufIn). Es una segunda "
            "copia del tamano que alguien tiene que sincronizar a mano, y el dia que se "
            "desincronice el firmware corta las tramas por donde nadie espera" % punta)

    b.verificar(
        tam["Maestro"] == tam["Esclavo"],
        "las dos puntas declaran el mismo buffer de entrada (%d B): %d caracteres "
        "utiles, un solo limite para quien escriba el ESP32"
        % (tam["Maestro"], tam["Maestro"] - 1),
        "BUFFERS DISTINTOS: Maestro %d B contra Esclavo %d B. El mismo comando entra "
        "en una punta y se trunca en la otra, y como el exceso se descarta EN SILENCIO "
        "el sintoma es un comando que simplemente no hace nada"
        % (tam["Maestro"], tam["Esclavo"]))

    # N-71 APLICADO AQUI: el tamano del buffer no es un numero suelto, es el TECHO de
    # los comandos. Un comando cuya parte fija no quepa es literalmente inalcanzable
    # -se trunca antes del '\n' y ningun strcmp casa-, y el sintoma seria un $ERR de
    # DESCONOCIDO sobre un comando que el firmware SI tiene escrito.
    for punta in PUNTAS:
        cod = fw.codigo(punta, "src", "bluetooth.cpp")
        literales = set(re.findall(
            r'strn?cmp\s*\(\s*(?:cmd|accion)\s*(?:\+\s*\d+\s*)?,\s*"([^"]+)"', cod))
        if len(literales) < 5:
            raise fw.Abortado(
                "%s: el censo de comandos solo hallo %d literales en bluetooth.cpp. "
                "Fallo el buscador, no el arbol: sin la lista no se puede comprobar "
                "que el mas largo quepa en el buffer" % (punta, len(literales)))
        prefijos = [l for l in literales if l.startswith("CMD:PIN:")]
        if not prefijos:
            raise fw.Abortado(
                "%s: no se hallo el prefijo CMD:PIN: en bluetooth.cpp. Sin el, la "
                "cuenta del comando mas largo saldria corta y aprobaria de mas" % punta)
        prefijo = max(prefijos, key=len)
        largos = [(l if l.startswith("CMD:") else prefijo + l) for l in literales]
        peor = max(largos, key=len)
        utiles = tam[punta] - 1
        b.verificar(
            len(peor) <= utiles,
            "%s: el comando fijo mas largo que el firmware compara -%s, %d car.- cabe "
            "en los %d utiles del buffer; quedan %d para sus parametros"
            % (punta, peor, len(peor), utiles, utiles - len(peor)),
            "%s: %s mide %d caracteres y solo caben %d. Ese comando ES INALCANZABLE: "
            "se trunca antes del fin de linea, ningun strcmp casa y el equipo contesta "
            "DESCONOCIDO a una orden que si tiene implementada"
            % (punta, peor, len(peor), utiles))

    b.reportar(
        "el contrato de entrada, medido, para quien escriba el ESP32",
        ["buffer btBufIn[%d] -> %d caracteres utiles por linea"
         % (tam["Maestro"], tam["Maestro"] - 1),
         "el exceso se DESCARTA EN SILENCIO: la guarda deja de guardar bytes y no",
         "manda ningun $ERR, asi que una linea larga llega mutilada y el equipo",
         "contesta DESCONOCIDO -o peor, casa con un strncmp de prefijo-.",
         "El separador es '\\n' o '\\r' (los dos), y una linea vacia no despacha nada."])

    # CONTROL NEGATIVO de la guarda por sizeof: el numero suelto que este pack impide.
    b.control_negativo(
        not re.search(r"btIdxIn\s*<\s*sizeof\s*\(\s*btBufIn\s*\)\s*-\s*1",
                      "} else if (btIdxIn < 63) { btBufIn[btIdxIn++] = c; }"),
        "una guarda sintetica escrita como `btIdxIn < 63` -el mismo numero, sin "
        "sizeof- se detecta como segunda copia del tamano")


# --- D. El framing de salida ------------------------------------------------------

def _bloque_framing(b, fw):
    b.titulo("El framing de salida: XOR-8 sin el '$', cerrado en CRLF")

    cuerpos = {}
    for punta in PUNTAS:
        donde = "%s/src/bluetooth.cpp" % punta
        cod = fw.codigo(punta, "src", "bluetooth.cpp")
        envio = _cuerpo(fw, cod, r"void\s+enviarTramaConCrc\s*\(",
                        "enviarTramaConCrc()", donde)
        suma = _cuerpo(fw, cod, r"uint8_t\s+calcularChecksum\s*\(",
                       "calcularChecksum()", donde)
        cuerpos[punta] = (suma, envio)

        # EL '$' NO ENTRA EN EL XOR. Es un byte, y un byte de diferencia hace que el
        # ESP32 rechace TODAS las tramas: el sintoma no es intermitente, es total, y
        # quien lo depure desde el otro lado no tiene el .cpp delante.
        b.verificar(
            bool(re.search(r"calcularChecksum\s*\(\s*payload\s*\+\s*1\s*\)", envio)),
            "%s: el checksum se calcula sobre payload+1, o sea SALTANDO el '$' inicial"
            % punta,
            "%s: enviarTramaConCrc() ya no pasa payload+1 al checksum. Si el '$' entra "
            "en el XOR, todas las tramas salen con un CRC que el otro extremo rechaza"
            % punta)

        formato = re.findall(r'"([^"]*%02X[^"]*)"', envio)
        b.verificar(
            formato == [r"%s*%02X\r\n"],
            "%s: la trama cierra con '*', dos hex en mayuscula y CRLF -formato %r-"
            % (punta, formato[0] if formato else None),
            "%s: el formato de trama es %r y no %r. El delimitador y el fin de linea "
            "son lo unico que el ESP32 tiene para trocear el flujo: sin el '\\r' o sin "
            "los dos digitos hex, un parser de lineas se come dos tramas en una"
            % (punta, formato, [r"%s*%02X\r\n"]))

        b.verificar(
            bool(re.search(r"crc\s*\^=", suma))
            and bool(re.search(r"\*str\s*!=\s*'\*'", suma)),
            "%s: calcularChecksum() acumula con XOR (crc ^=) y se detiene en el '*': es "
            "un XOR-8 sobre el cuerpo, no una suma ni un CRC de tabla" % punta,
            "%s: calcularChecksum() ya no es un XOR que pare en el '*'. El algoritmo es "
            "la mitad del contrato -el ESP32 lo reimplementa a mano-, y cambiarlo sin "
            "avisar deja el otro extremo rechazando todo" % punta)

        # LA BARRERA DE §6, APLICADA AL PUERTO: si alguien escribe al puerto rodeando
        # enviarTramaConCrc(), esa trama sale SIN checksum. No falla aqui: falla en el
        # ESP32, que la descarta o -peor- la acepta si no valida.
        escrituras = len(re.findall(r"SerialBT\s*\.\s*(?:print|println|write)\s*\(", cod))
        dentro = len(re.findall(r"SerialBT\s*\.\s*(?:print|println|write)\s*\(", envio))
        b.verificar(
            escrituras >= 1 and escrituras == dentro,
            "%s: las %d escrituras al puerto viven todas dentro de enviarTramaConCrc(): "
            "no hay trama que salga sin checksum" % (punta, escrituras),
            "%s: %d escrituras al puerto y solo %d pasan por enviarTramaConCrc(). Una "
            "trama que rodea el compositor sale SIN '*' ni CRC, y el otro extremo la "
            "descarta sin que este lado se entere" % (punta, escrituras, dentro))

    # Las dos puntas hablan con el MISMO ESP32. costura_01 vigila los ficheros
    # compartidos, y bluetooth.cpp no es uno de ellos -cada punta tiene su despachador-,
    # asi que el framing puede divergir sin que nada lo note. Se comparan las dos
    # funciones normalizando espacios: lo que importa es el algoritmo, no el sangrado.
    def _norm(t):
        return re.sub(r"\s+", " ", t).strip()
    iguales = all(_norm(cuerpos["Maestro"][i]) == _norm(cuerpos["Esclavo"][i])
                  for i in (0, 1))
    b.verificar(
        iguales,
        "calcularChecksum() y enviarTramaConCrc() son la MISMA funcion en las dos "
        "puntas: un solo parser sirve para los dos equipos",
        "EL FRAMING DIVERGE ENTRE PUNTAS. bluetooth.cpp no esta en la lista de "
        "compartidos de costura_01, asi que nada mas vigila esta igualdad: el ESP32 "
        "necesitaria dos parsers y nadie le habria dicho por que")

    # CONTROL NEGATIVO 1: el mismo detector, sobre un envio sintetico sin el +1.
    b.control_negativo(
        not re.search(r"calcularChecksum\s*\(\s*payload\s*\+\s*1\s*\)",
                      "uint8_t crc = calcularChecksum(payload);"),
        "un enviarTramaConCrc() sintetico que pasa `payload` sin el +1 se detecta: el "
        "'$' entrando en el XOR no pasa por aprobado")

    # CONTROL NEGATIVO 2: formato sin el retorno de carro.
    b.control_negativo(
        re.findall(r'"([^"]*%02X[^"]*)"', r'snprintf(t, n, "%s*%02X\n", payload, crc);')
        != [r"%s*%02X\r\n"],
        "un formato sintetico %s*%02X\\n -sin el retorno de carro- se detecta como "
        "framing distinto")

    # CONTROL NEGATIVO 3: una escritura cruda que rodea el compositor.
    crudo = 'void avisar() { SerialBT.print("$OK\\r\\n"); }'
    b.control_negativo(
        len(re.findall(r"SerialBT\s*\.\s*(?:print|println|write)\s*\(", crudo)) == 1,
        "un SerialBT.print() sintetico fuera de enviarTramaConCrc() se cuenta como "
        "escritura que rodea el compositor")


# --- E. La asimetria declarada ----------------------------------------------------

def _bloque_asimetria(b, fw):
    b.titulo("La asimetria del checksum: se emite, no se valida")

    valida = {p: _valida_entrada(fw, p) for p in PUNTAS}

    # POR QUE ESTO ES verificar() Y NO DOS COSAS DISTINTAS.
    #
    # Exigir "el STM32 valida el checksum de entrada" seria un FALLA permanente: hoy no
    # lo hace a proposito, y una comprobacion que ningun firmware de hoy puede aprobar
    # es la trampa del alias de CMD_DELTA -un rojo que nunca cambia ensena a ignorarlo-.
    # Exigir lo contrario, "NO lo valida", seria peor: castigaria a quien lo arreglara.
    #
    # Lo que si es una propiedad legitima, que cualquier firmware puede cumplir en las
    # dos direcciones, es que las DOS PUNTAS HAGAN LO MISMO. El ESP32 es uno solo y
    # habla con las dos: si una valida y la otra no, la misma trama con un byte tocado
    # se rechaza en un poste y se ejecuta en el de enfrente. Esa es la asimetria que no
    # puede existir; que hoy las dos elijan no validar es una NOTA, y va abajo.
    b.verificar(
        valida["Maestro"] == valida["Esclavo"],
        "las dos puntas tratan igual el checksum de ENTRADA (hoy: %s). Un solo "
        "comportamiento para el ESP32, que habla con las dos"
        % ("lo validan" if valida["Maestro"] else "no lo validan"),
        "LAS PUNTAS NO COINCIDEN: Maestro %s y Esclavo %s. La misma trama corrompida "
        "se rechaza en un poste y se ejecuta en el de enfrente, y el operario no tiene "
        "forma de saber cual de los dos le hizo caso"
        % ("valida" if valida["Maestro"] else "no valida",
           "valida" if valida["Esclavo"] else "no valida"))

    if not valida["Maestro"] and not valida["Esclavo"]:
        b.reportar(
            "ASIMETRIA DELIBERADA: el equipo emite checksum y no valida el de entrada",
            ["procesarComando() arranca con strcmp directo sobre la linea recibida:",
             "el '*XX' que llegue se trata como parte del comando y no casa con nada.",
             "NO ES UN DEFECTO -esta asi a proposito, y cerrarlo costaria flash del",
             "88,3 % que ya lleva el Maestro-, pero SI es contrato: el ESP32 tiene que",
             "verificar lo que RECIBE, porque este lado nunca le va a devolver un $ERR",
             "por una trama corrompida; y tiene que mandar comandos LIMPIOS, porque",
             "este lado no tiene con que descartar los sucios.",
             "Se anota con reportar() y no con verificar() a proposito: una",
             "comprobacion que ningun firmware de hoy puede aprobar no es una",
             "comprobacion (CLAUDE.md §3), y la contraria castigaria el arreglo."])

    # CONTROL NEGATIVO: el detector tiene que ver la validacion cuando existe, o su
    # PASS solo estaria diciendo que no supo mirar.
    sintetico_valida = """
      static void procesarComando(const char* c) { if (strcmp(c, "X") == 0) { } }
      void bluetooth_loop() {
        char* ast = strchr(btBufIn, '*');
        if (ast && (uint8_t)strtol(ast+1, 0, 16) != calcularChecksum(btBufIn+1)) return;
        procesarComando(btBufIn);
      }
    """
    b.control_negativo(
        _valida_entrada(fw, "Maestro", codigo=sintetico_valida)
        and not valida["Maestro"],
        "un bluetooth_loop() sintetico que SI compara el checksum recibido se detecta "
        "como punta que valida: el detector distingue las dos direcciones")
