# ===== banco/packs/documentos_03_trama_status.py =====
#
# EL CONTRATO DE $STATUS: FIRMWARE, MANUAL Y APP DICIENDO LO MISMO.
#
# La trama $STATUS es lo unico que une tres cosas que se escriben por separado: el
# C++ que la emite, el Manual 10 que la congela como especificacion, y el JavaScript
# de la app que la parsea. Tres copias del mismo contrato, mantenidas a mano. Es la
# situacion que este repositorio ya conoce -"una segunda copia del firmware escrita a
# mano que alguien sincroniza"- y que fallo cuatro veces en una semana.
#
# LO QUE SE MIDIO EL 27/08, QUE ES POR LO QUE ESTE PACK EXISTE.
#
#  1. El firmware emite SERIE: en las dos puntas desde f7d613f. El Manual 10 sigue
#     documentando la trama SIN ese campo, formula y ejemplo incluidos. La spec que
#     se declaro "congelada" describe una trama que ya no sale del micro.
#  2. La app lee SITE y PAIR de $STATUS. Ninguna punta los emite. No es un error
#     visible: es un dato que nunca llega y una pantalla que se queda vacia sin decir
#     por que.
#  3. HORA lleva HH:MM:SS -dos ':' dentro del valor- y el parser de la app parte cada
#     campo por ':' sin limite, asi que el reloj en vivo muestra "18". Medido
#     reproduciendo el split en Python, no leyendo el codigo.
#
# NINGUNO DE LOS TRES ES UN FALLO VIAL, y conviene decirlo: no encienden una luz. Lo
# que rompen es la confianza en la consola de servicio, que es de donde el tecnico
# saca sus decisiones cuando esta a 5 m del suelo con el celular en la mano.
#
# POR QUE ADEMAS COMPARA LAS COPIAS DE LA APP.
#
# La APK se construye desde www/, no desde el app.js que se edita. El 27/08 esos dos
# ficheros ya no eran el mismo -mismo numero de lineas, contenido distinto-, asi que
# lo que se prueba en el navegador y lo que se instala en el celular del tecnico eran
# dos programas. Un pack no puede compilar la APK, pero si puede exigir que haya UNA
# sola copia.

import re

NOMBRE = "documentos_03_trama_status"
DESCRIPCION = "$STATUS dice lo mismo en las dos puntas, en el Manual 10 y en la app"

PUNTAS = ("Maestro", "Esclavo")

MANUAL = ("05_Funcional", "10_Manual_Modulo_Bluetooth_Telemetria.md")
APP = ("05_Funcional", "App_Semaforo", "app.js")
APP_HTML = ("05_Funcional", "App_Semaforo", "index.html")
COPIAS_APP = (
    ("05_Funcional", "App_Semaforo", "www", "app.js"),
    ("05_Funcional", "App_Semaforo", "android", "app", "src", "main", "assets",
     "public", "app.js"),
)


def _crc(payload):
    """El XOR de 8 bits que define el apartado 4.1 del propio manual."""
    c = 0
    for ch in payload:
        c ^= ord(ch)
    return "%02X" % c


def _campos(trama):
    """Los nombres de campo de una trama, en orden, sin el $STATUS de cabecera."""
    partes = [p for p in trama.split(",") if p]
    return [p.split(":")[0].strip() for p in partes[1:]]


def _trama_del_cpp(fw, punta):
    codigo = fw.codigo(punta, "src", "bluetooth.cpp")
    m = re.search(r'"(\$STATUS,[^"]*)"', codigo)
    return m.group(1) if m else None


def _trama_del_manual(fw):
    texto = fw.texto_repo(*MANUAL)
    m = re.search(r"^(\$STATUS,[^\n*]*)", texto, re.M)
    return m.group(1) if m else None


def _rama_status_de_la_app(fw):
    texto = fw.texto_repo(*APP)
    m = re.search(r"if \(header === '\$STATUS'\)(.*?)\} else if \(header === '\$ALARM'\)",
                  texto, re.S)
    return m.group(1) if m else None


def correr(b, fw):
    b.titulo("El contrato de $STATUS entre el firmware, el Manual 10 y la app")

    tramas = {p: _trama_del_cpp(fw, p) for p in PUNTAS}
    if any(t is None for t in tramas.values()):
        raise fw.Abortado(
            "no se encontro la trama $STATUS en bluetooth.cpp de %s: fallo el "
            "buscador o la trama se construye de otra forma. Sin ella, comparar el "
            "manual contra nada lo aprobaria entero"
            % ", ".join(p for p, t in tramas.items() if t is None))

    campos = {p: _campos(t) for p, t in tramas.items()}

    # ---- 1. Las dos puntas emiten el mismo contrato ----
    b.verificar(
        campos["Maestro"] == campos["Esclavo"],
        "las dos puntas emiten los mismos campos y en el mismo orden: %s"
        % ", ".join(campos["Maestro"]),
        "el Maestro emite %s y el Esclavo %s. La app es una sola: si el contrato "
        "difiere entre puntas, el tecnico ve una consola distinta en cada poste sin "
        "que nada se lo advierta"
        % (", ".join(campos["Maestro"]), ", ".join(campos["Esclavo"])))

    emitidos = campos["Maestro"]

    # ---- 2. El Manual 10 documenta EXACTAMENTE lo que sale del micro ----
    trama_manual = _trama_del_manual(fw)
    if trama_manual is None:
        raise fw.Abortado(
            "no se hallo ningun ejemplo de trama $STATUS en el Manual 10: o cambio de "
            "formato o se movio de fichero, y en los dos casos esto no esta midiendo "
            "la especificacion")
    doc = _campos(trama_manual)
    b.verificar(
        doc == emitidos,
        "el Manual 10 documenta los mismos campos que emite el firmware: %s"
        % ", ".join(doc),
        "el Manual 10 documenta %s y el firmware emite %s. Faltan en el manual: %s. "
        "Sobran: %s. Un manual que promete una trama que no sale es peor que una "
        "pagina en blanco: el que escribe un parser contra el se lo cree"
        % (", ".join(doc), ", ".join(emitidos),
           ", ".join(c for c in emitidos if c not in doc) or "-",
           ", ".join(c for c in doc if c not in emitidos) or "-"))

    # ---- 3. La app no lee campos que nadie emite ----
    rama = _rama_status_de_la_app(fw)
    if rama is None:
        raise fw.Abortado(
            "no se hallo la rama de $STATUS en app.js: fallo el buscador. Aprobar "
            "aqui seria dar por bueno un parser que no se ha leido")
    leidos = sorted(set(re.findall(r"data\.([A-Z_]+)", rama)))
    fantasmas = [c for c in leidos if c not in emitidos]
    b.verificar(
        not fantasmas,
        "la app solo lee campos que el firmware emite (%s)" % ", ".join(leidos),
        "la app lee de $STATUS %s y ninguna punta los emite. No da error: deja la "
        "pantalla vacia para siempre, que es la forma silenciosa de este fallo"
        % ", ".join(fantasmas))

    # ---- 4. Los valores con ':' exigen un parser que los respete ----
    # HORA es HH:MM:SS. Si el parser parte por ':' sin limite, el valor se queda en
    # las dos primeras cifras y la app ensena una hora que no es. Se comprueba la
    # propiedad, no el sintoma: o ningun valor lleva ':', o el split lleva limite.
    # De donde sale "este campo lleva ':' en el valor": del EJEMPLO del manual, que es
    # el unico sitio donde hay valores de verdad. En el C++ la trama es una plantilla
    # -HORA:%s- y contarle los ':' daria cero SIEMPRE: una comprobacion que ningun
    # firmware puede suspender, o sea una nota disfrazada de prueba.
    valores_con_dos_puntos = [c for c, p in zip(doc, trama_manual.split(",")[1:])
                              if p.count(":") > 1 and c in emitidos]
    parte_sin_limite = bool(re.search(r"\.split\(':'\)(?!\s*,)", rama))
    b.verificar(
        not (valores_con_dos_puntos and parte_sin_limite),
        "el parser de la app respeta los valores que llevan ':' (%s)"
        % (", ".join(valores_con_dos_puntos) or "ninguno"),
        "la trama lleva %s con ':' dentro del valor y app.js parte cada campo con "
        "split(':') sin limite: el valor llega cortado. Reproducido: "
        "'HORA:18:25:00' entra como '18'" % ", ".join(valores_con_dos_puntos))

    # ---- 5. Una sola app: lo que se prueba es lo que se instala ----
    fuente = fw.texto_repo(*APP)
    for copia in COPIAS_APP:
        b.verificar(
            fw.texto_repo(*copia) == fuente,
            "%s es identico a app.js" % "/".join(copia[2:]),
            "%s NO es identico a app.js. La APK se construye desde esa copia, asi "
            "que lo que se prueba en el navegador y lo que se instala en el celular "
            "del tecnico son dos programas distintos" % "/".join(copia[2:]))

    # ---- 6. El PIN que ofrece la app tiene que existir en el firmware ----
    aceptados = set()
    for punta in PUNTAS:
        aceptados |= set(re.findall(r'"CMD:PIN:(\d+):"', fw.codigo(punta, "src", "bluetooth.cpp")))
    if not aceptados:
        raise fw.Abortado(
            "no se pudo leer del C++ ningun PIN de autorizacion: sin ese dato, "
            "comparar lo que ofrece la app seria compararlo contra nada")
    ofrecidos = set(re.findall(r'name="bt_pin_opt"\s+value="(\d+)"', fw.texto_repo(*APP_HTML)))
    b.verificar(
        ofrecidos and ofrecidos <= aceptados,
        "los PIN que ofrece la app (%s) los acepta el firmware" % ", ".join(sorted(ofrecidos)),
        "la app ofrece los PIN %s y el firmware solo acepta %s. Elegir %s garantiza "
        "un $ERR,AUTH_FAILED en todos los comandos, y el Manual 14 lo recomienda "
        "justamente como remedio del PIN incorrecto"
        % (", ".join(sorted(ofrecidos)), ", ".join(sorted(aceptados)),
           ", ".join(sorted(ofrecidos - aceptados)) or "-"))

    # ---- 7. Los ejemplos del manual cuadran con su propio checksum ----
    # El apartado 4.1 explica como se calcula el XOR y los dos ejemplos de debajo lo
    # incumplian: *4F y *3B cuando eran *42 y *43. Quien escribe un parser contra el
    # manual descarta la trama BUENA y se pone a buscar el fallo donde no esta.
    ejemplos = re.findall(r"^\$([A-Z]+,[^\n*]*)\*([0-9A-Fa-f]{2})", fw.texto_repo(*MANUAL), re.M)
    b.verificar(
        bool(ejemplos),
        "el Manual 10 trae %d tramas de ejemplo con checksum" % len(ejemplos),
        "no se hallo ni una trama de ejemplo con checksum en el Manual 10: fallo el "
        "buscador, no el manual")
    malos = [(p, c) for p, c in ejemplos if _crc(p) != c.upper()]
    b.verificar(
        not malos,
        "los %d ejemplos del Manual 10 cuadran con el XOR que el propio manual define"
        % len(ejemplos),
        "%d ejemplo(s) del Manual 10 llevan un checksum que no es el suyo: %s"
        % (len(malos), "; ".join("*%s deberia ser *%s" % (c, _crc(p)) for p, c in malos)))

    # ---- 8. Controles negativos ----
    b.control_negativo(
        _campos("$STATUS,NODE:X,MODO:Y") != emitidos,
        "una trama a la que le faltan campos deja de coincidir con la del firmware")
    b.control_negativo(
        bool(re.search(r"\.split\(':'\)(?!\s*,)", "x.split(':')")) and
        not re.search(r"\.split\(':'\)(?!\s*,)", "x.split(':', 2)"),
        "el detector de split sin limite distingue split(':') de split(':', 2)")
