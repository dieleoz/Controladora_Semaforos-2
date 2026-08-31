# ===== banco/packs/esp32_09_contrato_de_bytes.py =====
#
# EL PACK DE COSTURA: EL CONTRATO DE J17 NO HA DERIVADO ENTRE LAS TRES PUNTAS.
#
# POR QUE ESTE ES EL MAS IMPORTANTE DE LOS NUEVE Y EL QUE MAS FACIL SE OLVIDA.
#
# btBufIn[64] y 9600 viven en el C++ del STM32. Desde hoy viven OTRA VEZ en el C++ del
# ESP32. Son dos copias del mismo contrato que alguien tiene que sincronizar a mano, y
# este repositorio ya sabe como acaba eso:
#
#   N-36  el validador leia un fichero que ya no existia
#   N-39  el arnes medía ncenB10 mientras el codigo dibujaba en 7x14B
#   cfgVerdeRecibido  una variable contestando a dos preguntas, arreglar una rompia la otra
#
# La comparacion de totales entre puntas es lo que salvo la Fase 2 -"36/41 contra los
# 37/41 de siempre"-, y es la unica red para esta clase de deriva: la guarda de rutas
# vigila ficheros que desaparecen, NO numeros que se separan.
#
# 🔴 Y EL PIN NO SE CRUZA. 18_...md 3.5 lo dice sin maquillarlo: el ESP32 lo TRANSPORTA,
# no lo mejora, no lo sustituye, NO LO ALMACENA y no lo compara. Un puente que llevara
# "1234" dentro seria una segunda copia del contrato de autenticacion, y el dia que
# difirieran un comando funcionaria por una puerta y seria rechazado por la otra. Aqui
# se comprueba por la via mas simple que existe: que esa cadena no aparezca.
#
# NO LLEVA ETIQUETA SFTY. Vigila una costura de contrato, no una regla de seguridad.

import re

NOMBRE = "esp32_09_contrato_de_bytes"
DESCRIPCION = "el contrato de J17 -baudio, tope de linea, terminador, checksum- es el mismo en las dos puntas"

ROL = "ESP32_Expansion"
CONTRATO = ("ESP32_Expansion", "include", "contrato.h")
PUNTAS = ("Maestro", "Esclavo")


def correr(b, fw):
    b.titulo("La costura de J17: el mismo contrato en el C++ de las dos orillas")

    # ---- 1. EL TOPE DE LINEA == sizeof(btBufIn) - 1 --------------------------
    #
    # No se compara contra un 63 escrito aqui: se lee el tamano del array del STM32 y se
    # le resta uno, que es exactamente lo que hace su guarda
    # -btIdxIn < sizeof(btBufIn) - 1-. Si alguien agranda el buffer del STM32 y no toca
    # el del puente, esto lo dice.
    topeEsp32 = fw.constante(CONTRATO, r"#define\s+TRAMA_MAX_UTIL\s+(\d+)",
                             "el tope de linea util del puente")
    for punta in PUNTAS:
        tam = fw.constante((punta, "src", "bluetooth.cpp"),
                           r"char\s+btBufIn\[(\d+)\]",
                           "el buffer de entrada del %s" % punta)
        b.verificar(
            topeEsp32 == tam - 1,
            "%s: el tope del puente (%d) es exactamente sizeof(btBufIn)-1 (%d-1)"
            % (punta, topeEsp32, tam),
            "DERIVA DE CONTRATO con el %s: el puente corta a %d caracteres y el STM32 "
            "acepta %d. Si el puente corta de menos, mutila ordenes buenas; si corta de "
            "mas, el STM32 TRUNCA EN SILENCIO y compara la linea recortada con strcmp "
            "como si estuviera entera" % (punta, topeEsp32, tam - 1))

        # Y la guarda del STM32 sigue siendo la que se supone. Si alguien la cambia por
        # `<= sizeof(...)`, el numero de arriba deja de significar lo mismo.
        codigo = fw.codigo(punta, "src", "bluetooth.cpp")
        b.verificar(
            re.search(r"btIdxIn\s*<\s*sizeof\(btBufIn\)\s*-\s*1", codigo) is not None,
            "%s: la guarda del receptor sigue siendo btIdxIn < sizeof(btBufIn)-1, que es "
            "de donde sale el numero de arriba" % punta,
            "%s: la guarda del receptor cambio de forma. El tope de %d que el puente "
            "respeta se dedujo de ella; si ya no es esa, el numero puede estar midiendo "
            "otra cosa" % (punta, topeEsp32))

    # ---- 2. EL BAUDIO Y EL FORMATO -------------------------------------------
    baudio = fw.constante(CONTRATO, r"#define\s+ENLACE_BAUDIO\s+(\d+)",
                          "el baudio del puente")
    for punta in PUNTAS:
        suyo = fw.constante((punta, "src", "bluetooth.cpp"),
                            r"SerialBT\.begin\((\d+)\)", "el baudio del %s" % punta)
        b.verificar(
            baudio == suyo,
            "%s: las dos orillas a %d bps" % (punta, baudio),
            "DERIVA DE VELOCIDAD con el %s: puente %d, equipo %d. No se entienden, y el "
            "sintoma no es un error: es basura o silencio" % (punta, baudio, suyo))

    contrato = fw.texto(*CONTRATO)
    b.verificar(
        re.search(r"#define\s+ENLACE_FORMATO\s+SERIAL_8N1", contrato) is not None,
        "el formato va EXPLICITO en el puente (SERIAL_8N1): es lo unico escrito que ata "
        "las dos puntas, porque en el STM32 esa eleccion es un default del framework",
        "el puente no fija el formato explicitamente. SerialBT.begin(9600) del STM32 se "
        "llama con un solo argumento y los 8N1 los pone el framework: ese default no se "
        "puede leer de ningun sitio, asi que si el puente tampoco lo escribe NADIE lo ha "
        "elegido en ninguna de las dos orillas")

    enlace = fw.codigo("ESP32_Expansion", "src", "enlace_stm32.cpp")
    b.verificar(
        "ENLACE_FORMATO" in enlace,
        "y el arranque del puerto lo usa: la constante no es un adorno",
        "ENLACE_FORMATO esta definido y el arranque del puerto NO lo usa. Una constante "
        "declarada y no usada es la prueba muerta de siempre: dice que alguien eligio el "
        "formato cuando en realidad sigue eligiendolo el framework")

    # ---- 3. EL TERMINADOR QUE EL PUENTE EMITE ES UNO DE LOS QUE EL STM32 ACEPTA
    for punta in PUNTAS:
        codigo = fw.codigo(punta, "src", "bluetooth.cpp")
        b.verificar(
            re.search(r"c\s*==\s*'\\n'\s*\|\|\s*c\s*==\s*'\\r'", codigo) is not None,
            "%s: el receptor sigue delimitando por '\\n' o '\\r'" % punta,
            "%s: el receptor ya no delimita por '\\n' o '\\r'. El puente pone CRLF al "
            "reenviar; si el STM32 espera otra cosa, el despachador NO DISPARA NUNCA y "
            "las lineas se quedan mudas en el buffer" % punta)

    b.verificar(
        re.search(r"linea\[n\]\s*=\s*'\\r'", enlace) is not None
        and re.search(r"linea\[n \+ 1\]\s*=\s*'\\n'", enlace) is not None,
        "el puente termina cada linea con CRLF, que casa con los dos delimitadores del "
        "STM32 -el segundo cae con el indice a cero y no hace nada-",
        "el puente no termina las lineas con CRLF. Sin terminador el despachador del "
        "STM32 no dispara NUNCA: el comando se queda en btBufIn y el siguiente se le "
        "pega detras")

    # ---- 4. EL CHECKSUM: mismo algoritmo, mismos dos matices ------------------
    #
    # Los dos matices son del fuente del STM32, no de una convencion general: la llamada
    # es calcularChecksum(payload + 1) -salta el '$'- y el bucle es
    # while (*str && *str != '*') -para en el '*'-. Copiar "un XOR de la cadena" sin
    # ellos produce un checksum que nunca casa y un puente que descarta todo.
    trama = fw.codigo("ESP32_Expansion", "src", "trama.cpp")
    b.verificar(
        re.search(r"trama_checksum\s*\(\s*[A-Za-z_]\w*\s*\+\s*1\s*\)", trama) is not None,
        "el puente salta el '$' al calcular el checksum, igual que "
        "calcularChecksum(payload + 1) del STM32",
        "el puente NO salta el '$'. El XOR daria otro valor y toda trama del equipo se "
        "descartaria como ruido de cable: el telefono se quedaria sin telemetria con el "
        "equipo emitiendo perfectamente")

    b.verificar(
        re.search(r"while\s*\(\s*\*p\s*&&\s*\*p\s*!=\s*'\*'\s*\)", trama) is not None,
        "y para en el '*', igual que el bucle de calcularChecksum() del STM32",
        "el puente no para en el '*'. Meteria el propio checksum dentro del calculo del "
        "checksum, y ninguna trama casaria nunca")

    for punta in PUNTAS:
        codigo = fw.codigo(punta, "src", "bluetooth.cpp")
        b.verificar(
            re.search(r"calcularChecksum\(payload \+ 1\)", codigo) is not None
            and re.search(r"while\s*\(\s*\*str\s*&&\s*\*str\s*!=\s*'\*'\s*\)", codigo)
            is not None,
            "%s: sigue emitiendo con XOR-8 saltando el '$' y parando en el '*'" % punta,
            "%s: el algoritmo de checksum del equipo cambio de forma. El del puente se "
            "copio de ahi: si uno de los dos se mueve, el puente empieza a descartar "
            "telemetria buena y lo llama ruido" % punta)

    # ---- 5. El buffer de entrada del puente aguanta la trama mas larga -------
    bufEsp32 = fw.constante(CONTRATO, r"#define\s+BUF_ENTRADA_STM32\s+(\d+)",
                            "el buffer de recepcion del puente")
    peor = 0
    for punta in PUNTAS:
        tam = fw.constante((punta, "src", "bluetooth.cpp"),
                           r"char tramaCompleta\[(\d+)\]",
                           "el envoltorio de trama del %s" % punta)
        peor = max(peor, tam)
    b.verificar(
        bufEsp32 >= peor,
        "el puente recibe en %d B y la trama mas larga que el equipo puede componer cabe "
        "en %d B" % (bufEsp32, peor),
        "el buffer de recepcion del puente (%d B) es menor que el envoltorio del equipo "
        "(%d B). Las tramas largas se marcarian como desbordadas y se descartarian: el "
        "puente convertiria telemetria buena en un contador de descartes"
        % (bufEsp32, peor))

    # ---- 6. LOS CINCO PREFIJOS. Ni cuatro, ni los que uno recuerde ------------
    #
    # El censo del puente se cruza contra lo que las dos puntas emiten DE VERDAD. Si el
    # equipo estrena un $XYZ, esto lo dice: el puente no filtra por prefijo, pero sus
    # contadores de diagnostico dejarian de nombrar una trama que existe.
    emitidos = set()
    for punta in PUNTAS:
        codigo = fw.codigo(punta, "src", "bluetooth.cpp")
        for m in re.finditer(r'"(\$[A-Z]+)[,"]', codigo):
            emitidos.add(m.group(1))

    m = re.search(r"PREFIJOS_STM32\[PREFIJOS_STM32_N\]\s*=\s*\{([^}]*)\}", trama)
    censo = set(re.findall(r'"(\$[A-Z]+)"', m.group(1))) if m else set()
    n = fw.constante(CONTRATO, r"#define\s+PREFIJOS_STM32_N\s+(\d+)",
                     "el numero de prefijos del censo")

    b.verificar(
        len(censo) == n and censo == emitidos,
        "el censo del puente son los %d prefijos que las dos puntas emiten de verdad: %s"
        % (n, ", ".join(sorted(censo))),
        "EL CENSO DE PREFIJOS NO CUADRA. El puente lista %s y las puntas emiten %s. "
        "$EVENT es la que se cae de las listas escritas de memoria -el Maestro lo emite "
        "desde catorce ramas y la app lo consume como bitacora-, y perderla es la misma "
        "clase de agujero silencioso que costo N-73: un registro que cuatro documentos "
        "describen y que nadie puede mirar cuando hay que diagnosticar un fallo"
        % (sorted(censo), sorted(emitidos)))

    # ---- 7. 🔴 EL PIN NO VIVE EN EL PUENTE -----------------------------------
    pin = None
    for punta in PUNTAS:
        m = re.search(r'"CMD:PIN:(\d+):"', fw.codigo(punta, "src", "bluetooth.cpp"))
        if m:
            pin = m.group(1)
            break
    if pin is None:
        raise fw.Abortado(
            "no se pudo leer el literal del PIN del C++ del STM32. Sin el, comprobar "
            "que el puente NO lo lleva seria buscar una cadena inventada: el pack diria "
            "que todo esta bien sin haber mirado el numero que importa")

    conPin = []
    for carpeta, ext in (("src", ".cpp"), ("include", ".h")):
        for f in fw.fuentes_de("ESP32_Expansion", carpeta, ext):
            codigo = fw.codigo("ESP32_Expansion", carpeta, f)
            if re.search(r'"[^"]*CMD:PIN:', codigo) or ('"%s"' % pin) in codigo:
                conPin.append("%s/%s" % (carpeta, f))

    b.verificar(
        not conPin,
        "el PIN (%s) no aparece en ningun fuente del puente: se transporta sin conocerlo"
        % pin,
        "EL PUENTE LLEVA EL PIN DENTRO (%s). Es una segunda copia del contrato de "
        "autenticacion que alguien tendria que sincronizar, y el dia que difieran un "
        "comando funcionaria por una puerta y seria rechazado por la otra. El puente "
        "transporta el PIN: no lo mejora, no lo sustituye, no lo almacena y no lo "
        "compara" % ", ".join(conPin))

    # ---- CONTROLES NEGATIVOS -------------------------------------------------
    b.control_negativo(
        topeEsp32 != 128,
        "un tope de linea de 128 en el puente dejaria de casar con sizeof(btBufIn)-1")

    b.control_negativo(
        re.search(r"trama_checksum\s*\(\s*[A-Za-z_]\w*\s*\+\s*1\s*\)",
                  "uint8_t c = trama_checksum(payload);") is None,
        "un checksum que NO salta el '$' se detecta: es la diferencia entre descartar "
        "todo y no descartar nada")

    b.control_negativo(
        bool(re.search(r'"[^"]*CMD:PIN:', 'strncmp(x, "CMD:PIN:1234:", 13)')),
        "un prefijo de PIN colado en el fuente del puente se detecta")

    b.control_negativo(
        {"$STATUS", "$ACK", "$ERR", "$ALARM"} != emitidos,
        "un censo de CUATRO prefijos deja de cuadrar contra lo que las puntas emiten: "
        "$EVENT no se puede perder por olvido")
