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

# Comandos que el firmware atiende pero que NO tienen por que estar en la app: son
# de servicio o los dispara otra cosa. Se listan aqui para que la comprobacion de la
# direccion contraria signifique algo en vez de aprobar siempre.
SIN_BOTON_A_PROPOSITO = {
    "SET_RTC",       # lo manda el asistente Courier, no un boton suelto
}


def _atiende(fw, punta):
    """Los comandos que el despachador de esa punta reconoce. Leidos del C++."""
    codigo = fw.codigo(punta, "src", "bluetooth.cpp")
    exactos = set(re.findall(r'strcmp\s*\(\s*accion\s*,\s*"([^"]+)"', codigo))
    prefijos = set(re.findall(r'strncmp\s*\(\s*accion\s*,\s*"([^"]+):"', codigo))
    # Tambien la forma sin PIN, que es deliberada para el rojo de emergencia.
    sin_pin = set(re.findall(r'strcmp\s*\(\s*cmd\s*,\s*"CMD:([^"]+)"', codigo))
    return exactos | prefijos | sin_pin


def _envia(fw):
    """Los comandos que la app manda, por TODAS sus puertas.

    Son tres, y olvidar una es acusar a la app de algo que si hace: al escribir este
    pack se miraron solo executeCommand() y data-cmd, y el resultado fue reportar como
    "sin interfaz" un FORZAR_ROJO y un TEST_LEDS que la app manda desde hace meses
    -por openPinModal(), que guarda el comando y lo ejecuta al validar el PIN-.
    Es la regla del instrumento: descartar al buscador antes de acusar."""
    js = fw.texto_repo(*APP_JS)
    html = fw.texto_repo(*APP_HTML)
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
    todos = atiende["Maestro"] | atiende["Esclavo"]
    b.verificar(
        True,
        "despachadores leidos del C++: Maestro %s | Esclavo %s"
        % (sorted(atiende["Maestro"]), sorted(atiende["Esclavo"])),
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
        "los %d comandos que manda la app los atiende alguna punta: %s"
        % (len(envia), sorted(envia)),
        "la app manda %s y NINGUNA punta los conoce. El despachador cae al else y "
        "contesta $ERR,CMD:DESCONOCIDO: el tecnico ve un error que no significa nada, "
        "y si el comando se manda al conectar -como GET_STATUS- lo ve SIEMPRE"
        % huerfanos)

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
    b.control_negativo(
        bool(re.findall(r"executeCommand\(\s*'([^']+)'", "executeCommand('X_INVENTADO')")),
        "el lector de comandos de la app encuentra un executeCommand con literal")
