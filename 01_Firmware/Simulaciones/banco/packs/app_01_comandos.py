# ===== banco/packs/app_01_comandos.py =====
#
# LA COSTURA DE LA APP: TODO LO QUE EL CELULAR MANDA, ALGUNA PUNTA TIENE QUE ATENDERLO.
#
# Es costura_03 aplicada a la otra frontera. Aquella comprueba que lo que una punta
# emite por radio la otra lo atiende; esta comprueba lo mismo entre la app y el
# firmware, que hasta hoy no lo miraba nadie: el JavaScript y el C++ los escribe gente
# distinta, en dias distintos, y el unico sitio donde se encuentran es un string.
#
# LO QUE ENCONTRO AL ESCRIBIRSE, Y POR ESO EXISTE:
#
#   - La app manda CMD:PIN:1234:GET_STATUS nada mas conectar, DOS VECES, y tambien
#     GET_CONFIG. Ninguna de las dos puntas conoce esos comandos: el despachador cae
#     al else final y responde $ERR,CMD:DESCONOCIDO. O sea que lo PRIMERO que ve el
#     tecnico al conectarse es un error, y encima uno que no significa nada.
#   - Y al reves: el Esclavo estreno SOLICITAR_PASO en V9.0 -el funcional pidiendo
#     paso desde cualquier extremo, N-58- y la app NO TIENE BOTON para eso. Firmware
#     sin interfaz es trabajo hecho que nadie puede usar.
#
# POR QUE NO BASTA CON MIRAR LA APP.
#
# El despachador del firmware es una cadena de strcmp() sobre el texto que viene
# despues de "CMD:PIN:1234:". No hay tabla, no hay enum: el contrato ES esa cadena de
# comparaciones. Por eso este pack la lee del .cpp -sin lista escrita a mano- y la
# compara con lo que el .js manda de verdad.

import re

NOMBRE = "app_01_comandos"
DESCRIPCION = "todo comando que la app manda lo atiende alguna punta, y lo que el firmware ofrece tiene interfaz"

PUNTAS = ("Maestro", "Esclavo")

APP_JS = ("05_Funcional", "App_Semaforo", "app.js")
APP_HTML = ("05_Funcional", "App_Semaforo", "index.html")

# EL TERCER DESPACHADOR, Y NO ESTABA (A-9, 05/09).
#
# Este pack decia "todo comando que la app manda, ALGUNA PUNTA tiene que atenderlo", y
# leia dos ficheros: los bluetooth.cpp del Maestro y del Esclavo. Era cierto cuando se
# escribio y dejo de serlo el dia que el ESP32 se puso en medio: desde entonces hay un
# TERCER aparato que atiende comandos de la app -su propio despachador- y este censo no
# lo abria.
#
# LO QUE ESO OCULTABA, MEDIDO Y NO SUPUESTO: `despachador.cpp` atiende SET_RTC: desde el
# 31/08 y este pack nunca lo vio. Que SET_RTC saliera igual en verde era CASUALIDAD -el
# Maestro conserva su rama muda para consumir la orden, y esa rama es lo que el censo
# encontraba-. El dia que esa rama se retirara, este pack habria acusado a la app de
# mandar un comando que nadie atiende... con el puente atendiendolo.
#
# Y con LEER_RTC ya no hay casualidad: es un comando que SOLO el puente puede contestar
# -las dos puntas devuelven $ERR,CMD:AUTH_FAILED,DESC:PIN_INVALIDO, medido sobre su
# bluetooth.cpp compilado-, asi que sin este tercer lector el pack acusaria a la app de
# un huerfano que no lo es.
BT_PUENTE = ("ESP32_Expansion", "src", "despachador.cpp")

# Comandos que el firmware atiende pero que NO tienen por que estar en la app: son
# de servicio o los dispara otra cosa. Se listan aqui para que la comprobacion de la
# direccion contraria signifique algo en vez de aprobar siempre.
SIN_BOTON_A_PROPOSITO = {
    "SET_RTC",       # lo manda el asistente Courier, no un boton suelto
    # 11/09 (D-20 / A-15): NO LA MANDA LA APP, Y ESO ES LA BARRERA, NO UN HUECO. La
    # origina el ESP32 de cada poste desde su DS3231 y le llega al STM32 sin PIN; el
    # puente la TIRA si le llega del telefono. Una app que la mandara pondria la hora sin
    # PIN en cuanto hubiera un puente viejo en medio. Por eso esta excepcion se MIDE
    # (comprobacion 2.bis): no basta con que este aqui, la app no puede mandarla.
    "HORA_ESP32",
}

# La orden que la app NO puede mandar, por el motivo de arriba. Se nombra aparte porque
# no es solo "sin boton": su ausencia en la app es una propiedad que se comprueba.
NO_LA_MANDA_LA_APP = "HORA_ESP32"


def _atiende(fw, punta):
    """Los comandos que el despachador de esa punta reconoce. Leidos del C++."""
    codigo = fw.codigo(punta, "src", "bluetooth.cpp")
    exactos = set(re.findall(r'strcmp\s*\(\s*accion\s*,\s*"([^"]+)"', codigo))
    prefijos = set(re.findall(r'strncmp\s*\(\s*accion\s*,\s*"([^"]+):"', codigo))
    # Tambien la forma sin PIN, que es deliberada para el rojo de emergencia.
    sin_pin = set(re.findall(r'strcmp\s*\(\s*cmd\s*,\s*"CMD:([^"]+)"', codigo))
    # 11/09: y la forma sin PIN POR PREFIJO -CMD:HORA_ESP32:<iso>-, que hasta hoy no
    # existia y este lector no veia: una orden atendida que el censo no cuenta es una
    # orden que nadie revisa. El filtro del PIN tiene la misma forma y no es una orden.
    sin_pin |= {c for c in re.findall(r'strncmp\s*\(\s*cmd\s*,\s*"CMD:([A-Z0-9_]+):"',
                                      codigo) if c != "PIN"}
    return exactos | prefijos | sin_pin


def _atiende_el_puente(fw):
    """Los comandos que el despachador del ESP32 atiende. Leidos del C++.

    Son de DOS formas y las dos cuentan, porque significan cosas distintas:

      strstr(linea, "SET_RTC:")     el puente lo atiende. Hasta el 11/09 la linea
                                    SEGUIA VIAJE al STM32; desde el 11/09 no (D-20):
                                    al STM32 le llega CMD:HORA_ESP32. El lector no
                                    cambia: sigue contando SET_RTC como del puente.
      strcmp(linea, CMD_LEER_RTC)   la linea se queda aqui: es del puente y de nadie
                                    mas (despachador_esParaElPuente).

    La segunda forma se lee de la CONSTANTE, no del strcmp, porque el literal vive en
    un `static const char CMD_X[] = "CMD:X";` y compararlo por nombre de variable
    dejaria fuera justo el texto que viaja por el cable."""
    codigo = fw.codigo(*BT_PUENTE)
    atiende = set()
    # La forma compartida: prefijo con ':' detras, igual que en el STM32.
    atiende |= set(re.findall(r'strstr\s*\(\s*\w+\s*,\s*"([A-Z0-9_]+):"', codigo))
    # La forma exclusiva: la constante con la linea entera, "CMD:" incluido.
    atiende |= set(re.findall(r'static\s+const\s+char\s+\w+\[\]\s*=\s*"CMD:([A-Z0-9_:]+)"',
                              codigo))
    return atiende


def _envia(fw):
    """Los comandos que la app manda, por TODAS sus puertas.

    Son tres, y olvidar una es acusar a la app de algo que si hace: al escribir este
    pack se miraron solo executeCommand() y data-cmd, y el resultado fue reportar como
    "sin interfaz" un FORZAR_ROJO y un TEST_LEDS que la app manda desde hace meses
    -por openPinModal(), que guarda el comando y lo ejecuta al validar el PIN-.
    Es la regla del instrumento: descartar al buscador antes de acusar."""
    return _envia_de(fw.texto_repo(*APP_JS), fw.texto_repo(*APP_HTML))


def _envia_de(js, html):
    """El cuerpo de _envia(), sobre textos: asi el control negativo de la 2.bis pasa por
    el MISMO lector que la app de verdad."""
    literales = set(re.findall(r"executeCommand\(\s*'([^']+)'", js))
    literales |= set(re.findall(r"openPinModal\(\s*'([^']+)'", js))
    # N-75: la puerta de salida se renombro a enviarComandoFirmware() en el rewrite de
    # la interfaz de 2 roles, y este censo se quedo a CERO comandos sin decirlo: acuso
    # a la app de no mandar seis ordenes que manda. Se leen las dos formas, la de un
    # argumento -MANUAL:CAMBIAR_TURNO- y la de dos -SET_MODO + AUTO-, que en el cable
    # se pegan con ':'.
    literales |= set(re.findall(r"enviarComandoFirmware\(\s*'([^']+)'\s*\)", js))
    literales |= set("%s:%s" % (c, a) for c, a in re.findall(
        r"enviarComandoFirmware\(\s*'([^']+)'\s*,\s*'([^']+)'", js))
    # Y la raiz sola cuando el argumento se construye -SET_TIEMPOS con los tres
    # numeros, SET_RTC con la fecha-: ahi el valor no es un literal que leer.
    # ...pero SOLO si ese argumento se construye. Si es un literal, la forma X:Y de
    # arriba ya lo cubre, y anadir ademas la raiz suelta -SET_MODO- inventa un
    # comando que la app no manda: el cable siempre lleva SET_MODO:AUTO.
    literales |= set(re.findall(r"enviarComandoFirmware\(\s*'([^']+)'\s*,\s*[^'\s)]", js))
    del_html = set(re.findall(r'data-cmd="([^"]+)"', html))
    # Los que se CONSTRUYEN con plantilla, que son los que llevan parametros pegados:
    # SET_RTC con la fecha, SET_TIEMPOS con los tres numeros. Se guarda su raiz.
    #
    # Esta linea ya se quedo corta una vez -no veia openPinModal()- y volvio a quedarse
    # corta con SET_TIEMPOS, acusando a la app de no mandar un comando que manda desde
    # una plantilla. Por eso el patron es generico: CUALQUIER raiz en mayusculas
    # seguida de ':' dentro de una plantilla cuenta como enviada.
    # El ':' de una trama va PEGADO a su valor -SET_TIEMPOS:${verde}-, mientras que un
    # texto de pantalla lleva espacio -`RTT: ${data.rtt} ms`-. Sin esa distincion el
    # censo daba por comando el rotulo del RTT y acusaba a la app de mandar una orden
    # que ninguna punta conoce. No se afloja nada: sigue contando cualquier raiz en
    # mayusculas de una plantilla, solo exige que tenga forma de trama y no de rotulo.
    construidos = set(re.findall(r"`([A-Z][A-Z_]+):(?=\S)", js))
    # "CMD" no es un comando: es el prefijo de la propia trama (CMD:PIN:1234:...).
    construidos.discard("CMD")
    return literales | del_html | construidos


def correr(b, fw):
    b.titulo("Costura app <-> firmware: los comandos del Bluetooth")

    atiende = {p: _atiende(fw, p) for p in PUNTAS}
    for p in PUNTAS:
        if not atiende[p]:
            raise fw.Abortado(
                "no se pudo leer del bluetooth.cpp del %s ni un solo comando. El "
                "despachador es una cadena de strcmp() y si cambio de forma este pack "
                "estaria comparando contra un conjunto vacio, que aprueba cualquier "
                "cosa" % p)
    # EL TERCER DESPACHADOR SE LEE APARTE Y SE EXIGE NO VACIO, por la misma razon que
    # los otros dos: un censo vacio aprueba cualquier cosa. Y aqui el suelo importa mas,
    # porque este lector es NUEVO -si su regex se queda atras, el pack volveria a acusar
    # a la app de huerfanos que el puente si atiende, y esta vez con la excusa de que
    # "antes pasaba".
    puente = _atiende_el_puente(fw)
    if not puente:
        raise fw.Abortado(
            "no se pudo leer del despachador del ESP32 (%s) ni un solo comando. Atiende "
            "SET_RTC: desde el 31/08 y LEER_RTC desde el 05/09, asi que un censo vacio "
            "significa que el fuente cambio de forma -no que el puente no atienda nada-, "
            "y aprobar con el conjunto vacio seria acusar a la app de huerfanos que si "
            "tienen quien los conteste" % "/".join(BT_PUENTE))

    todos = atiende["Maestro"] | atiende["Esclavo"] | puente
    b.verificar(
        True,
        "despachadores leidos del C++: Maestro %s | Esclavo %s | PUENTE %s"
        % (sorted(atiende["Maestro"]), sorted(atiende["Esclavo"]), sorted(puente)),
        "no deberia llegarse aqui")

    envia = _envia(fw)
    if not envia:
        raise fw.Abortado(
            "no se hallo ni un comando en app.js/index.html: fallo el buscador, no la "
            "app. Aprobar aqui seria dar por buena una interfaz que no se ha leido")

    # ---- 1. TODO lo que la app manda, alguna punta lo atiende ----
    huerfanos = sorted(c.split(":")[0] if c.startswith("SET_RTC") else c
                       for c in envia if c not in todos and c.split(":")[0] not in todos)
    b.verificar(
        not huerfanos,
        "los %d comandos que manda la app los atiende alguna punta O EL PUENTE: %s"
        % (len(envia), sorted(envia)),
        "la app manda %s y no los conoce NI una punta NI el puente. El despachador que "
        "los reciba cae al else y contesta $ERR,CMD:DESCONOCIDO -o peor, al no llevar "
        "PIN, $ERR,CMD:AUTH_FAILED,DESC:PIN_INVALIDO, que acusa al operario de una clave "
        "que no tecleo-: el tecnico ve un error que no significa nada, y si el comando "
        "se manda al conectar -como GET_STATUS- lo ve SIEMPRE" % huerfanos)

    # ---- 2. Y al reves: lo que el firmware ofrece, la app lo puede usar ----
    # Esta direccion es la que caza el trabajo hecho y no expuesto. No cuenta como
    # fallo del firmware: cuenta como interfaz que falta, y por eso se nombra.
    sin_interfaz = sorted(c for c in todos
                          if c not in envia and c not in SIN_BOTON_A_PROPOSITO
                          and not any(e.startswith(c) for e in envia))
    b.verificar(
        not sin_interfaz,
        "todo comando que el firmware atiende tiene forma de llegar desde la app",
        "el firmware atiende %s y la app no los manda desde ningun sitio. Es firmware "
        "sin interfaz: trabajo hecho que el tecnico no puede usar -SOLICITAR_PASO es "
        "justo la funcion que el Esclavo estreno en V9.0 (N-58)-" % sin_interfaz)

    # ---- 2.bis. La excepcion de HORA_ESP32 se mide: la app NO la manda (11/09) ----
    #
    # La 2 la deja pasar sin boton por estar en SIN_BOTON_A_PROPOSITO. Esa excepcion dice
    # algo sobre el codigo -"no la origina el telefono"- y aqui se comprueba en la
    # direccion que importa: si la app la mandara, un puente anterior al 11/09 la
    # reenviaria y el STM32 pondria la hora sin PIN.
    mandadas = sorted(c for c in envia if c.split(":")[0] == NO_LA_MANDA_LA_APP)
    b.verificar(
        NO_LA_MANDA_LA_APP in todos and not mandadas,
        "%s la atiende el STM32 (%s) y la app NO la manda: la origina el ESP32 de cada "
        "poste, y la excepcion de la 2 es cierta" % (
            NO_LA_MANDA_LA_APP,
            ", ".join(p for p in PUNTAS if NO_LA_MANDA_LA_APP in atiende[p])),
        "%s. La hora del STM32 viene del ESP32 de su poste (D-20): una app que la manda "
        "pone la hora sin PIN en cuanto hay un puente viejo en medio, y un STM32 que ya no "
        "la atiende deja la excepcion de la 2 sin sujeto" % (
            ("la app manda %s" % mandadas) if mandadas else
            ("ninguna punta atiende %s" % NO_LA_MANDA_LA_APP)))

    # ---- 3. El PIN del contrato se lee del C++, no se supone ----
    pines = set()
    for p in PUNTAS:
        pines |= set(re.findall(r'"CMD:PIN:(\d+):"', fw.codigo(p, "src", "bluetooth.cpp")))
    b.verificar(
        len(pines) == 1,
        "las dos puntas exigen el MISMO PIN de comando (%s)" % ", ".join(pines),
        "las puntas exigen PIN distintos: %s. Un tecnico que cambie de poste tendria "
        "que cambiar de PIN sin que nada se lo diga" % sorted(pines))

    # ---- 4. El rojo de emergencia sigue sin pedir PIN, en las dos puntas ----
    for p in PUNTAS:
        codigo = fw.codigo(p, "src", "bluetooth.cpp")
        sin_pin = re.search(r'strcmp\s*\(\s*cmd\s*,\s*"CMD:FORZAR_ROJO"', codigo)
        b.verificar(
            sin_pin is not None,
            "%s: FORZAR_ROJO se atiende TAMBIEN sin PIN -parar la via nunca se "
            "protege con una contrasena que hay que recordar con un accidente delante-"
            % p,
            "%s: el rojo de emergencia ya solo se acepta con PIN. Es la puerta que "
            "SIEMPRE tiene que estar abierta: el PIN guarda lo que ABRE paso, no lo "
            "que lo para" % p)

    # ---- 5. Controles negativos ----
    b.control_negativo(
        "CMD_INVENTADO" not in todos,
        "un comando inventado no aparece como atendido por ninguna punta")
    # EL LECTOR NUEVO TIENE QUE SABER ENCONTRAR Y SABER NO ENCONTRAR. Sin las dos
    # mitades, un regex roto dejaria el censo del puente vacio y el ABORTADO de arriba
    # seria lo unico que lo dijera... el dia que alguien lo mirara.
    b.control_negativo(
        "LEER_RTC" in puente and "SET_RTC" in puente,
        "el lector del tercer despachador encuentra sus DOS formas: la de prefijo "
        "(strstr de SET_RTC:) y la de linea entera (la "
        "constante CMD:LEER_RTC, que se queda en el puente)")
    b.control_negativo(
        bool(re.findall(r"executeCommand\(\s*'([^']+)'", "executeCommand('X_INVENTADO')")),
        "el lector de comandos de la app encuentra un executeCommand con literal")
    # La 2.bis tiene que saber ver a la app mandando la orden prohibida por la puerta que
    # usaria de verdad -enviarComandoFirmware con el argumento construido-, por el MISMO
    # lector que la app real.
    b.control_negativo(
        NO_LA_MANDA_LA_APP in _envia_de(
            "enviarComandoFirmware('%s', `${today},${now}`);" % NO_LA_MANDA_LA_APP, ""),
        "una app que mandara %s por enviarComandoFirmware() se lee como tal: la 2.bis la "
        "acusaria" % NO_LA_MANDA_LA_APP)
