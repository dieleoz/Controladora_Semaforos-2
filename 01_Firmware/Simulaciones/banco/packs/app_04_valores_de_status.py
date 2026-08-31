# ===== banco/packs/app_04_valores_de_status.py =====
#
# LOS VALORES DE $STATUS, NO SUS NOMBRES DE CAMPO.
#
# LA PROPIEDAD, EN UNA LINEA: todo literal que el C++ puede escribir en ESTADO: y en
# MODO: la app sabe pintarlo, y todo lo que la app sabe pintar lo puede escribir el
# C++.
#
# ES documentos_03 BAJADO UN NIVEL, Y ESE NIVEL ES DONDE VIVIA EL DEFECTO.
#
# documentos_03 compara los NOMBRES DE CAMPO -NODE, SERIE, MODO, ESTADO, T, RF, RTT,
# BAT, HORA- entre el firmware, el Manual 10 y la app, y estaba en verde. Tenia que
# estarlo: los nueve nombres coincidian. Lo que nadie miraba era el CONTENIDO de dos
# de esos campos, y ahi la coincidencia era CERO.
#
# LO MEDIDO EL 31/08, QUE ES POR LO QUE ESTE PACK EXISTE:
#
#   el firmware puede EMITIR en ESTADO: ....  ROJO  VERDE  AMARILLO  "FALLO COM"
#                                             (semaforo_nombreEstado(), identica en
#                                              las dos puntas)
#   la app sabia PINTAR ...................... V1_R2  Y1_R2  R1_R2  ALL_RED  R1_V2
#                                              R1_Y2  AMBAR_FAIL
#
#   INTERSECCION: VACIA.  grep "case 'ROJO'" app.js -> 0
#
# El switch de renderLights() apagaba las seis lamparas y ningun `case` volvia a
# encender ninguna, asi que con el enlace VIVO los dos semaforos quedaban negros y los
# textos conservaban lo anterior -que tras marcarSinEnlace() es "SIN ENLACE - sin
# datos del equipo"-. La pantalla declaraba que no tenia datos mientras los recibia.
#
# El vocabulario que la app leia no salio de la nada: es el del puente de PC
# (05_Funcional/App_Semaforo/servidor_puente_simulador.py), que es un SIMULADOR. O
# sea, la version de interfaz de la prueba muerta: el tablero de campo hablaba el
# idioma del simulador y ninguno del equipo.
#
# Y EL MISMO DEFECTO ESTABA EN MODO:, mas silencioso todavia. La cadena de `if` del
# badge conocia AUTO, MANUAL, AMBAR y un ROJO_TOTAL que no emite nadie; el firmware
# puede emitir diez literales. Sin `else`, los siete restantes dejaban el badge con el
# MODO ANTERIOR pintado y con su color: un modo vencido con aspecto de vigente. El que
# mas duele de esa lista es DEGRADADO, que es el unico modo que da verde sin
# confirmacion del otro extremo.
#
# LAS DOS DIRECCIONES CUENTAN, Y NO SIGNIFICAN LO MISMO:
#
#   falta en la app   ->  el equipo dice algo y la pantalla no lo pinta (o peor, se
#                         queda con lo de antes, que es indistinguible de un dato).
#   sobra en la app   ->  la app sabe pintar algo que ningun firmware emite. Es lo que
#                         estaba pasando, y solo esta direccion lo habria cazado: la
#                         de "falta" tambien fallaba, pero la de "sobra" nombra la
#                         causa -siete casos de un vocabulario ajeno-.
#
# LAS LISTAS SE RELEEN DEL C++ EN CADA CORRIDA. Si alguno de los dos censos sale
# vacio, esto ABORTA: comparar contra un conjunto vacio aprueba cualquier cosa.
#
# SOBRE LAS ETIQUETAS SFTY: este pack NO lleva ninguna. Vigila la fidelidad de un
# tablero, no una barrera del firmware. Roza SFTY-6 -"FALLO COM" es su ambar con la
# talanquera arriba- pero no la EJERCE: no comprueba ni el umbral de silencio ni la
# pluma, comprueba que la app no lo pinte de rojo. Figurar en la tabla de trazabilidad
# sin ejercer la regla es peor que una fila vacia, porque la vacia no miente.

import re

NOMBRE = "app_04_valores_de_status"
DESCRIPCION = "los valores que el C++ puede poner en ESTADO: y MODO:, la app sabe pintarlos"

PUNTAS = ("Maestro", "Esclavo")

APP_JS = ("05_Funcional", "App_Semaforo", "app.js")

# Las dos tablas de la app, por el nombre con el que estan declaradas en el fuente. No
# es una lista de valores escrita a mano -los valores se leen de dentro-: es la
# direccion donde vive la tabla, igual que los packs del firmware direccionan un .cpp
# por su tupla de ruta.
TABLA_ESTADOS = "ESTADOS"
TABLA_MODOS = "MODOS"


def _bloque(texto, i):
    """El interior del bloque que abre en texto[i]. None si no cierra."""
    apertura = texto[i]
    cierre = {"{": "}", "(": ")", "[": "]"}[apertura]
    prof = 0
    for j in range(i, len(texto)):
        if texto[j] == apertura:
            prof += 1
        elif texto[j] == cierre:
            prof -= 1
            if prof == 0:
                return texto[i + 1:j]
    return None


def _cuerpo_funcion(codigo, nombre):
    """El cuerpo de una funcion C++ por su nombre. None si no esta."""
    m = re.search(r"\b%s\s*\([^)]*\)\s*\{" % re.escape(nombre), codigo)
    if not m:
        return None
    return _bloque(codigo, m.end() - 1)


def _valores_de_switch(cuerpo):
    """Los literales que un switch de `case X: return "Y";` puede devolver.

    Se leen del cuerpo y en su orden. El `return ""` del final -la salida por abajo del
    switch, que el compilador exige- se recoge aparte: es un valor que el firmware
    PUEDE emitir, pero no es un estado con nombre y no se le puede pedir a la app una
    entrada de tabla para el. Lo que si se le pide es que tenga declarada una salida
    para lo que no reconoce, y eso se comprueba abajo."""
    con_nombre = re.findall(r'\bcase\s+[A-Za-z_]\w*\s*:\s*return\s+"([^"]*)"\s*;', cuerpo)
    por_defecto = re.findall(r'\bdefault\s*:\s*return\s+"([^"]*)"\s*;', cuerpo)
    cola = re.findall(r'\breturn\s+"([^"]*)"\s*;\s*$', cuerpo.strip())
    return con_nombre, por_defecto, cola


def _claves_de_tabla(js, nombre):
    """Las claves de un objeto literal `const NOMBRE = { 'X': {...}, ... }` del .js.

    Solo el primer nivel: se cuentan las comillas que estan a profundidad 1 dentro del
    objeto, para que los literales de los valores -textos de pantalla, colores- no se
    cuelen como si fueran claves."""
    m = re.search(r"\bconst\s+%s\s*=\s*\{" % re.escape(nombre), js)
    if not m:
        return None
    cuerpo = _bloque(js, m.end() - 1)
    if cuerpo is None:
        return None
    claves = []
    prof = 0
    i = 0
    while i < len(cuerpo):
        c = cuerpo[i]
        if c in "{[(":
            prof += 1
        elif c in "}])":
            prof -= 1
        elif c == "'" and prof == 0:
            j = cuerpo.find("'", i + 1)
            if j < 0:
                break
            # Solo es clave si detras viene el ':' del objeto.
            k = j + 1
            while k < len(cuerpo) and cuerpo[k].isspace():
                k += 1
            if k < len(cuerpo) and cuerpo[k] == ":":
                claves.append(cuerpo[i + 1:j])
            i = j
        i += 1
    return claves


def _modo_fijo_del_esclavo(fw):
    """El literal que el Esclavo escribe DENTRO del snprintf: MODO:SUBORDINADO.

    No sale de ningun switch -esa punta no tiene obtenerNombreModo()-, va escrito en la
    plantilla de la trama. Es exactamente la clase de valor que un censo que solo mire
    switches no ve, y es el que el badge de la app no conocia."""
    codigo = fw.codigo("Esclavo", "src", "bluetooth.cpp")
    m = re.search(r'"\$STATUS,[^"]*?MODO:([A-Z_]+)', codigo)
    return m.group(1) if m else None


def correr(b, fw):
    b.titulo("Los VALORES de ESTADO: y MODO:, del C++ contra lo que la app sabe pintar")

    js = fw.texto_repo(*APP_JS)

    # ---- 1. El enum de ESTADO, leido de las dos puntas ----
    estados = {}
    vacios = {}
    for p in PUNTAS:
        cuerpo = _cuerpo_funcion(fw.codigo(p, "src", "semaforo.cpp"), "semaforo_nombreEstado")
        if cuerpo is None:
            raise fw.Abortado(
                "%s: no se hallo semaforo_nombreEstado() en semaforo.cpp. Es el unico "
                "sitio donde estan los literales que viajan en ESTADO:; sin ella este "
                "pack compararia la app contra un conjunto vacio y la aprobaria entera"
                % p)
        con_nombre, por_defecto, cola = _valores_de_switch(cuerpo)
        if not con_nombre:
            raise fw.Abortado(
                "%s: semaforo_nombreEstado() no dio ni un `case X: return \"Y\";`. O "
                "cambio de forma o el buscador se quedo atras -y medir cero valores "
                "sale en verde-" % p)
        estados[p] = con_nombre
        vacios[p] = por_defecto + cola

    b.verificar(
        estados["Maestro"] == estados["Esclavo"],
        "las dos puntas emiten el MISMO enum de ESTADO y en el mismo orden: %s"
        % ", ".join(estados["Maestro"]),
        "el Maestro puede emitir %s y el Esclavo %s. La app es UNA: si el vocabulario "
        "difiere entre puntas, el tecnico ve un tablero distinto en cada poste y nada "
        "se lo advierte"
        % (", ".join(estados["Maestro"]), ", ".join(estados["Esclavo"])))

    emitibles_estado = sorted(set(estados["Maestro"]) | set(estados["Esclavo"]))

    # ---- 2. El enum de MODO: el switch del Maestro mas el literal fijo del Esclavo ----
    cuerpo_modo = _cuerpo_funcion(fw.codigo("Maestro", "src", "bluetooth.cpp"),
                                  "obtenerNombreModo")
    if cuerpo_modo is None:
        raise fw.Abortado(
            "no se hallo obtenerNombreModo() en Maestro/src/bluetooth.cpp: es de donde "
            "sale todo lo que puede aparecer en MODO:, y sin ella el badge de la app "
            "se compararia contra nada")
    modos_case, modos_default, _ = _valores_de_switch(cuerpo_modo)
    if not modos_case:
        raise fw.Abortado(
            "obtenerNombreModo() no dio ni un `case X: return \"Y\";`: fallo el "
            "buscador o cambio la forma del switch")

    subordinado = _modo_fijo_del_esclavo(fw)
    if subordinado is None:
        raise fw.Abortado(
            "no se pudo leer el literal de MODO: que el Esclavo escribe dentro de su "
            "snprintf de $STATUS. Esa punta no tiene obtenerNombreModo(), asi que ese "
            "literal es su UNICO valor de modo: sin el, el censo se queda corto justo "
            "en el valor que el badge de la app no conocia")

    emitibles_modo = sorted(set(modos_case) | set(modos_default) | {subordinado})
    b.verificar(
        subordinado not in modos_case,
        "el literal fijo del Esclavo (%s) NO sale de ningun switch y se censa aparte: "
        "un censo que solo mirara los `case` se lo dejaria fuera" % subordinado,
        "el literal fijo del Esclavo (%s) coincide con un `case` del Maestro. No es un "
        "fallo del firmware: es que este censo ya no demuestra que sabe leer las dos "
        "fuentes, y hay que revisarlo antes de fiarse de el" % subordinado)

    # ---- 3. La app conoce todo lo que el firmware puede decir ----
    claves_estado = _claves_de_tabla(js, TABLA_ESTADOS)
    claves_modo = _claves_de_tabla(js, TABLA_MODOS)
    for nombre, claves in ((TABLA_ESTADOS, claves_estado), (TABLA_MODOS, claves_modo)):
        if not claves:
            raise fw.Abortado(
                "no se hallo en app.js la tabla `const %s = { ... }` con sus claves. "
                "Es donde la app declara que sabe pintar; si se movio o cambio de "
                "forma, este pack no esta midiendo la pantalla de nadie" % nombre)

    faltan_e = [v for v in emitibles_estado if v not in claves_estado]
    b.verificar(
        not faltan_e,
        "la app sabe pintar los %d valores de ESTADO que el firmware puede emitir (%s)"
        % (len(emitibles_estado), ", ".join(emitibles_estado)),
        "el firmware puede emitir ESTADO:%s y la app no tiene entrada para %s. Con el "
        "enlace VIVO esas lamparas se quedan apagadas y los textos conservan lo "
        "anterior: la pantalla dice que no hay datos mientras los esta recibiendo"
        % ("/".join(emitibles_estado), ", ".join(faltan_e)))

    faltan_m = [v for v in emitibles_modo if v not in claves_modo]
    b.verificar(
        not faltan_m,
        "la app sabe pintar los %d valores de MODO que las dos puntas pueden emitir "
        "(%s)" % (len(emitibles_modo), ", ".join(emitibles_modo)),
        "el firmware puede emitir MODO:%s y el badge de la app no conoce %s. Sin "
        "entrada el badge se queda con el modo ANTERIOR y con su color, que es un modo "
        "vencido con aspecto de vigente -y DEGRADADO, el unico modo que da verde sin "
        "el otro extremo, estaba en esa lista-"
        % ("/".join(emitibles_modo), ", ".join(faltan_m)))

    # ---- 4. Y no sabe pintar nada que el firmware no pueda decir ----
    #
    # Esta es la direccion que nombra la causa. La de arriba dice "faltan cuatro"; esta
    # dice "y ademas hay siete de un vocabulario que no es el del equipo", que es lo
    # que explica de donde salio el defecto en vez de solo constatarlo.
    sobran_e = [v for v in claves_estado if v not in emitibles_estado]
    b.verificar(
        not sobran_e,
        "la app no sabe pintar ningun ESTADO que el firmware no pueda emitir",
        "la app sabe pintar %s y NINGUNA punta lo emite. Un vocabulario que no es el "
        "del equipo en el tablero de campo no es codigo muerto: es el sitio por donde "
        "entra el idioma de un simulador -el puente de PC habla asi- a una pantalla "
        "que decide sobre trafico" % ", ".join(sobran_e))

    sobran_m = [v for v in claves_modo if v not in emitibles_modo]
    b.verificar(
        not sobran_m,
        "el badge no sabe pintar ningun MODO que el firmware no pueda emitir",
        "el badge de la app conoce el modo %s y ninguna punta lo emite: es una rama "
        "que no se ejecuta nunca ocupando el sitio de las que si hacen falta"
        % ", ".join(sobran_m))

    # ---- 5. Y tiene declarada una salida para lo que NO reconozca ----
    #
    # Las dos comprobaciones de arriba se pueden satisfacer hoy y quedarse cortas
    # manana: el dia que el firmware estrene un valor, la app se queda sin entrada. Lo
    # que impide que eso sea silencioso no es la tabla, es el `else`. Y hace falta
    # ademas por un valor que YA existe: los dos switch tienen salida por abajo -un
    # `return ""`- y una cadena vacia no puede tener entrada de tabla.
    for func, tabla in (("renderLights", TABLA_ESTADOS), ("pintarBadgeModo", TABLA_MODOS)):
        m = re.search(r"\bfunction\s+%s\s*\([^)]*\)\s*\{" % re.escape(func), js)
        if not m:
            raise fw.Abortado(
                "no se hallo function %s() en app.js: es quien consulta la tabla %s, y "
                "sin ella no se puede comprobar que exista salida para un valor "
                "desconocido" % (func, tabla))
        cuerpo = _bloque(js, m.end() - 1)
        tiene_salida = cuerpo is not None and (
            re.search(r"if\s*\(\s*!\s*info\s*\)", cuerpo) or re.search(r"\belse\b", cuerpo))
        b.verificar(
            bool(tiene_salida),
            "%s() declara que hace con un valor que la tabla %s no conoce"
            % (func, tabla),
            "%s() consulta la tabla %s y NO tiene salida para un valor desconocido. "
            "Sin ella, el dia que el firmware estrene un literal la pantalla se queda "
            "con lo de antes, y un dato viejo pintado como si fuera de ahora es "
            "indistinguible de uno bueno" % (func, tabla))

    b.reportar(
        "los dos switch del firmware tienen salida por abajo, y esa salida es texto vacio",
        ["semaforo_nombreEstado() acaba en `return \"\";` (%s) y obtenerNombreModo() "
         "en `default: return \"%s\";`."
         % (", ".join(repr(v) for v in vacios["Maestro"]) or "-",
            ", ".join(modos_default) or "-"),
         "ESTADO: vacio es un valor que puede viajar y no puede tener entrada de tabla.",
         "Lo cubre la comprobacion 5, no una fila mas aqui: pedirle a la app una entrada",
         "para la cadena vacia seria pedirle que nombre lo que el equipo declara no saber."])

    # ---- 6. El anillo no decide por las letras del literal ----
    #
    # El color del anillo se elegia con estadoLuces.includes('V') / .includes('Y'). Con
    # el vocabulario del simulador colaba; con el del firmware es falso dos veces:
    # "AMARILLO" no lleva 'Y' y "FALLO COM" tampoco, asi que los dos caian al else y
    # salian en ROJO. Y "FALLO COM" no es rojo ni es apagado: es ambar intermitente CON
    # LA TALANQUERA ARRIBA, o sea que por ahi se pasa. El anillo decia lo contrario.
    por_letras = re.findall(r"estadoLuces\s*\.\s*includes\s*\(", js)
    b.verificar(
        not por_letras,
        "el color del anillo se decide por el VALOR de ESTADO, no por las letras que "
        "lleva dentro",
        "app.js elige el color del anillo con estadoLuces.includes(...) %d vez/veces. "
        "MEDIDO sobre el enum real: 'AMARILLO' no contiene 'Y' y 'FALLO COM' tampoco, "
        "asi que los dos salen en ROJO -el equipo en ambar y el anillo diciendo ESPERA-"
        % len(por_letras))

    # Y la comprobacion de la que cuelga la anterior: que el ambar de FALLO COM sea
    # ambar. Se lee de la propia tabla, no se supone.
    m = re.search(r"'FALLO COM'\s*:\s*\{", js)
    fila_fallo = _bloque(js, m.end() - 1) if m else None
    b.verificar(
        fila_fallo is not None and re.search(r"anillo\s*:\s*'amber'", fila_fallo)
        and re.search(r"lampara\s*:\s*'amber'", fila_fallo),
        "'FALLO COM' se pinta AMBAR en la lampara y en el anillo: es ambar "
        "intermitente con la talanquera ARRIBA, no un rojo y no un apagado",
        "'FALLO COM' no se pinta en ambar en app.js. Es el estado de SFTY-6: ambar "
        "intermitente y talanquera ARRIBA, o sea que por ahi SE PASA con precaucion. "
        "Pintarlo de rojo le dice a quien mira el telefono lo contrario de lo que el "
        "equipo esta haciendo")

    # ---- 7. Controles negativos ----
    b.control_negativo(
        [v for v in emitibles_estado + ["S_INVENTADO"] if v not in claves_estado] != [],
        "un valor de mas en el enum del firmware se detecta como ausente de la app")
    b.control_negativo(
        [v for v in list(claves_estado) + ["V1_R2"] if v not in emitibles_estado] != [],
        "una entrada de mas en la tabla de la app se detecta como vocabulario que "
        "ninguna punta emite -que es exactamente el caso que estaba delante-")
    b.control_negativo(
        _claves_de_tabla("const X = { 'A': { t: 'no soy clave' }, 'B': { u: 'ni yo' } };", "X")
        == ["A", "B"],
        "el lector de tablas devuelve las claves y no los literales de dentro de los "
        "valores")
    b.control_negativo(
        bool(re.findall(r"estadoLuces\s*\.\s*includes\s*\(", "if (state.estadoLuces.includes('V'))"))
        and not re.findall(r"estadoLuces\s*\.\s*includes\s*\(", "const i = ESTADOS[state.estadoLuces];"),
        "el detector del anillo por letras distingue el includes() del acceso por "
        "clave a la tabla")
