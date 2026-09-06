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
TABLA_PLUMA = "PLUMA_LEYENDA"
TABLA_CAM = "CAM_LEYENDA"


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


def _literales_de_pluma(fw, punta):
    """Los dos literales que el C++ puede poner en el campo PLUMA de $STATUS.

    N-153. No salen de un switch: van escritos en el ternario que alimenta al snprintf,
    que es la MISMA forma que ya obligo a censar aparte el MODO:SUBORDINADO del Esclavo.
    Un censo que solo mirara switches no los veria, y esta lista se compara contra una
    tabla de la app que decide lo que se pinta de una BARRERA FISICA: el conductor le
    hace mas caso a la barrera que a la lampara, asi que un literal que la app no sepa
    pintar deja en pantalla un texto en crudo justo donde hay que decidir si se cruza."""
    codigo = fw.codigo(punta, "src", "bluetooth.cpp")
    m = re.search(r'semaforo_plumaArriba\(\)\s*\?\s*"([^"]*)"\s*:\s*"([^"]*)"', codigo)
    return [m.group(1), m.group(2)] if m else None


def _modo_fijo_del_esclavo(fw):
    """El literal que el Esclavo escribia DENTRO del snprintf: MODO:SUBORDINADO.

    A-11 (05/09): DESDE HOY DEVUELVE None LA MAYORIA DE LAS VECES, Y ESO NO ES UN FALLO.
    Esa punta gano su propio obtenerNombreModo() al poder entrar en Modo Degradado por
    Bluetooth, asi que su MODO: pasa de literal fijo a "%s" y sus valores ya no viven en
    la plantilla. Quien llama distingue los dos casos: un literal fijo se censa aparte,
    un diccionario se lee como el del Maestro.

    LA FUNCION SE CONSERVA EN VEZ DE BORRARSE porque sigue midiendo algo -que si alguna
    punta VUELVE a escribir el modo dentro de la plantilla, ese valor no se quede fuera
    del censo-. Es la clase de valor que un censo que solo mire switches no ve, y es el
    que el badge de la app no conocia."""
    codigo = fw.codigo("Esclavo", "src", "bluetooth.cpp")
    m = re.search(r'"\$STATUS,[^"]*?MODO:([A-Z_]+)', codigo)
    return m.group(1) if m else None


def _modos_de_la_punta(fw, punta):
    """Los literales de MODO: que esa punta puede emitir, del switch o de la plantilla.

    Devuelve (con_nombre, comodines, fuente). Se prueban las DOS formas y manda la del
    switch.

    🔴 Y QUE UNA PUNTA NO TENGA LAS DOS A LA VEZ NO LO COMPRUEBA ESTE PACK, lo cual se
    escribe porque la primera version de esta frase decia que si y era una afirmacion
    sobre el codigo sin verificar. Lo mide app_02_modos_simetricos, que decide por el
    "MODO:%s" de la PLANTILLA y falla nombrando el desajuste: "una que publica un modo
    FIJO y mantiene un diccionario tiene una lista que nadie usa". Medido inyectando el
    defecto: con la plantilla vuelta a MODO:SUBORDINADO y el switch puesto, app_02 cae de
    12/12 a 7/8 y este pack se queda en 23/23 -mide el switch, que sigue estando-. Aqui
    no se duplica esa comprobacion: se dice de quien es.

    Los COMODINES -el rotulo del `default:`- van APARTE de los modos con nombre. Los dos
    viajan en MODO: y los dos necesitan entrada en la tabla de la app, pero el comodin no
    es un modo: es lo que la telemetria dice cuando NO sabe donde esta el equipo, asi que
    es correcto que las dos puntas lo llamen igual y no cuenta como solape."""
    cuerpo = _cuerpo_funcion(fw.codigo(punta, "src", "bluetooth.cpp"),
                             "obtenerNombreModo")
    fijo = _modo_fijo_del_esclavo(fw) if punta == "Esclavo" else None
    if cuerpo is not None:
        casos, pordefecto, _ = _valores_de_switch(cuerpo)
        if not casos:
            raise fw.Abortado(
                "%s: obtenerNombreModo() no dio ni un `case X: return \"Y\";`: fallo "
                "el buscador o cambio la forma del switch" % punta)
        return set(casos), set(pordefecto), "obtenerNombreModo()"
    if fijo is not None:
        return {fijo}, set(), "literal fijo dentro de la plantilla del $STATUS"
    raise fw.Abortado(
        "%s: no se pudo leer NI un obtenerNombreModo() NI un literal fijo de MODO: en "
        "su snprintf de $STATUS. Toda punta publica un modo de una de las dos formas; "
        "sin ninguna, el badge de la app se compararia contra un conjunto vacio y "
        "saldria aprobado" % punta)


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

    # ---- 2. El enum de MODO: de las DOS puntas, cada una por donde lo declare ----
    #
    # A-11 (05/09). ANTES ESTO ERA "el switch del Maestro mas el literal fijo del
    # Esclavo", y esa asimetria estaba escrita a mano. Al ganar el Esclavo su propio
    # obtenerNombreModo() -entra en Degradado por Bluetooth y hay que poder verlo- la
    # frase se quedaba vieja, asi que se PREGUNTA a cada punta por donde publica su modo
    # en vez de darlo por sabido. Es el mismo criterio de app_02_modos_simetricos, que ya
    # decide por el "MODO:%s" de la plantilla y no por el nombre de la punta.
    modos = {}
    comodines = {}
    fuentes = {}
    for p in PUNTAS:
        modos[p], comodines[p], fuentes[p] = _modos_de_la_punta(fw, p)

    emitibles_modo = sorted(set().union(*modos.values(), *comodines.values()))

    # EL CENSO DEMUESTRA QUE HA LEIDO LAS DOS FUENTES, QUE ES LO QUE ESTA LINEA MEDIA.
    #
    # Aqui ponia "el literal fijo del Esclavo (SUBORDINADO) NO sale de ningun switch",
    # y su motivo escrito era exacto: "un censo que solo mirara los `case` se lo dejaria
    # fuera". La propiedad no era sobre el firmware, era SOBRE ESTE PACK -que sabe leer
    # las dos fuentes-, y sigue haciendo falta ahora que las dos puntas tienen switch:
    # si el lector del Esclavo se quedara ciego, el censo perderia SUBORDINADO y RENDIDO
    # y aprobaria a la app por no pedirselos.
    #
    # SE MIDE PIDIENDO QUE CADA PUNTA APORTE ALGO QUE LA OTRA NO. Si una de las dos
    # aportara solo literales que la otra ya trae, este pack no podria distinguir
    # "las lei las dos" de "lei una y la otra devolvio vacio".
    #
    # 🔴 Y NO SE EXIGE QUE NO SE SOLAPEN, QUE FUE EL PRIMER INTENTO Y ERA FALSO. Las dos
    # puntas emiten DEGRADADO y esta BIEN: significa lo mismo en las dos -este poste esta
    # operando por su reloj, sin confirmacion del otro extremo-, y el badge de la app lo
    # pinta con un solo texto con toda la razon. Un pack que lo prohibiera estaria
    # midiendo un borde que no es el que importa; el que importa -que un literal
    # compartido signifique lo mismo- no se puede leer del fuente y se dice aqui.
    solo_m = sorted(modos["Maestro"] - modos["Esclavo"])
    solo_e = sorted(modos["Esclavo"] - modos["Maestro"])
    b.verificar(
        bool(solo_m) and bool(solo_e),
        "el censo leyo las DOS fuentes de MODO: y cada punta aporta literales propios "
        "-Maestro %s (%s), Esclavo %s (%s)-; compartidos a proposito: %s; comodines: %s"
        % (solo_m, fuentes["Maestro"], solo_e, fuentes["Esclavo"],
           sorted(modos["Maestro"] & modos["Esclavo"]) or "ninguno",
           sorted(set().union(*comodines.values())) or "ninguno"),
        "el censo de MODO: no aporto literales propios de %s. O esa punta dejo de "
        "publicar un modo suyo, o su lector se quedo ciego -y entonces este pack estaria "
        "aprobando a la app por no pedirle valores que nunca llego a censar-"
        % ("las dos puntas" if not solo_m and not solo_e
           else "el Maestro" if not solo_m else "el Esclavo"))

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

    # ---- 4.bis. Y lo mismo con PLUMA:, que decide lo que se pinta de una BARRERA ----
    #
    # N-153 (05/09). El campo es nuevo y su dominio son DOS literales, asi que la
    # tentacion es darlo por evidente. Se mide por lo mismo que ESTADO y MODO: no hay
    # nada en el firmware que impida cambiar "ARRIBA" por otra palabra, y la app se
    # quedaria pintando el literal en crudo -que es lo correcto que hace, pero es un
    # sintoma, no un tablero-. MEDIDO: se inyecto ese cambio exacto el 05/09 y NINGUN
    # instrumento lo vio; el simulador de app no puede, porque su modelo se inventa el
    # valor en vez de leerlo del C++.
    literales = {}
    for punta in PUNTAS:
        lit = _literales_de_pluma(fw, punta)
        if lit is None:
            raise fw.Abortado(
                "%s: no se pudo leer del C++ el ternario que rellena PLUMA: en el "
                "$STATUS. Es el unico sitio donde estan esos dos literales; sin ellos "
                "este pack compararia la tabla de la app contra un conjunto vacio y la "
                "aprobaria entera" % punta)
        literales[punta] = lit

    b.verificar(
        literales["Maestro"] == literales["Esclavo"],
        "las dos puntas escriben los MISMOS literales en PLUMA y en el mismo orden: %s"
        % ", ".join(literales["Maestro"]),
        "el Maestro puede emitir PLUMA:%s y el Esclavo PLUMA:%s. La app es UNA, y las "
        "dos placas son la misma placa: si el vocabulario difiere, el tecnico ve la "
        "barrera de un poste dibujada y la del otro en crudo"
        % ("/".join(literales["Maestro"]), "/".join(literales["Esclavo"])))

    emitibles_pluma = sorted(set(literales["Maestro"]) | set(literales["Esclavo"]))
    claves_pluma = _claves_de_tabla(js, TABLA_PLUMA)
    if not claves_pluma:
        raise fw.Abortado(
            "no se hallo en app.js la tabla `const %s = { ... }` con sus claves. Es "
            "donde la app declara que sabe dibujar la pluma; si se movio o cambio de "
            "forma, este pack no esta midiendo la pantalla de nadie" % TABLA_PLUMA)

    faltan_p = [v for v in emitibles_pluma if v not in claves_pluma]
    b.verificar(
        not faltan_p,
        "la app sabe dibujar los %d valores de PLUMA que el firmware puede emitir (%s)"
        % (len(emitibles_pluma), ", ".join(emitibles_pluma)),
        "el firmware puede emitir PLUMA:%s y la app no tiene entrada para %s. Se pinta "
        "el literal en crudo -que es lo unico honesto que puede hacer-, pero eso es un "
        "sintoma en la pantalla de operacion, no un tablero"
        % ("/".join(emitibles_pluma), ", ".join(faltan_p)))

    sobran_p = [v for v in claves_pluma if v not in emitibles_pluma]
    b.verificar(
        not sobran_p,
        "la app no sabe dibujar ninguna posicion de la pluma que el firmware no pueda "
        "emitir",
        "la app sabe dibujar PLUMA:%s y ninguna punta lo emite: es una rama que no se "
        "ejecuta nunca ocupando el sitio de las que si hacen falta, y el sitio por "
        "donde entra un vocabulario que no es el del equipo" % ", ".join(sobran_p))

    b.control_negativo(
        _literales_de_pluma.__doc__ is not None and
        re.search(r'semaforo_plumaArriba\(\)\s*\?\s*"([^"]*)"\s*:\s*"([^"]*)"',
                  'x = semaforo_plumaArriba() ? "ARRIBA" : "ABAJO";').group(1) == "ARRIBA"
        and re.search(r'semaforo_plumaArriba\(\)\s*\?\s*"([^"]*)"\s*:\s*"([^"]*)"',
                      'x = otraCosa() ? "ARRIBA" : "ABAJO";') is None,
        "el lector del ternario de PLUMA saca los dos literales y NO acepta un ternario "
        "de otra funcion: si el campo dejara de salir de semaforo_plumaArriba(), el "
        "pack aborta en vez de medir el ternario de al lado")

    # ---- 4.ter. Y lo mismo con CAM:, donde ademas hay un valor que NO ES UN ESTADO ----
    #
    # D-13 fase 1 (05/09). Las dos direcciones se miden igual que con ESTADO, MODO y
    # PLUMA. Lo que hace a este campo distinto -y lo que obliga a la comprobacion de
    # abajo- es que uno de sus cuatro valores no dice como esta la camara: dice que NO SE
    # SABE. El "?" es el estado con el que arranca el vigilante, cuando por esos pines
    # todavia no ha pasado nada.
    #
    # POR QUE ESO NO SE PUEDE PINTAR COMO OK, y por que vale una comprobacion propia: en
    # fase 1 el vigilante NO toca el ciclo -solo cuenta-, asi que una camara que no ve no
    # se nota en la calle. Esta pantalla es el unico sitio donde alguien se entera. Un
    # "?" pintado como OK le dice al operario que la deteccion esta comprobada cuando lo
    # unico que consta es que nadie la ha ejercido, y de ese dato cuelga en fase 2 si la
    # barrera baja o no.
    literales_cam = {}
    for punta in PUNTAS:
        cuerpo = _cuerpo_funcion(fw.codigo(punta, "src", "botones.cpp"), "camara_estado")
        if cuerpo is None:
            raise fw.Abortado(
                "%s: no se hallo camara_estado() en botones.cpp. Es el unico sitio donde "
                "estan los literales que viajan en CAM:; sin ellos este pack compararia "
                "la tabla de la app contra un conjunto vacio y la aprobaria entera" % punta)
        lits = re.findall(r'return\s+"([^"]*)"\s*;', cuerpo)
        if not lits:
            raise fw.Abortado(
                "%s: camara_estado() no dio ni un `return \"...\";`. O cambio de forma o "
                "el buscador se quedo atras -y medir cero valores sale en verde-" % punta)
        literales_cam[punta] = lits

    b.verificar(
        literales_cam["Maestro"] == literales_cam["Esclavo"],
        "las dos puntas emiten los MISMOS literales en CAM y en el mismo orden: %s"
        % ", ".join(literales_cam["Maestro"]),
        "el Maestro puede emitir CAM:%s y el Esclavo CAM:%s. La app es UNA y las dos "
        "placas son la misma: si el vocabulario difiere, el tecnico ve el estado de la "
        "deteccion dibujado en un poste y en crudo en el otro"
        % ("/".join(literales_cam["Maestro"]), "/".join(literales_cam["Esclavo"])))

    emitibles_cam = sorted(set(literales_cam["Maestro"]) | set(literales_cam["Esclavo"]))
    claves_cam = _claves_de_tabla(js, TABLA_CAM)
    if not claves_cam:
        raise fw.Abortado(
            "no se hallo en app.js la tabla `const %s = { ... }` con sus claves. Es "
            "donde la app declara que sabe leer el estado de las camaras; si se movio o "
            "cambio de forma, este pack no esta midiendo la pantalla de nadie" % TABLA_CAM)

    faltan_c = [v for v in emitibles_cam if v not in claves_cam]
    b.verificar(
        not faltan_c,
        "la app sabe pintar los %d valores de CAM que el firmware puede emitir (%s)"
        % (len(emitibles_cam), ", ".join(emitibles_cam)),
        "el firmware puede emitir CAM:%s y la app no tiene entrada para %s. Se pinta el "
        "marcador de valor imposible -que es lo unico honesto que puede hacer-, pero eso "
        "es un sintoma en la pantalla, no un tablero: y en fase 1 esta pantalla es el "
        "UNICO sitio donde una camara pegada se nota"
        % ("/".join(emitibles_cam), ", ".join(faltan_c)))

    sobran_c = [v for v in claves_cam if v not in emitibles_cam]
    b.verificar(
        not sobran_c,
        "la app no sabe pintar ningun estado de camara que el firmware no pueda emitir",
        "la app sabe pintar CAM:%s y ninguna punta lo emite: es una rama que no se "
        "ejecuta nunca ocupando el sitio de las que si hacen falta, y el sitio por donde "
        "entra un vocabulario que no es el del equipo" % ", ".join(sobran_c))

    # LA QUE DE VERDAD IMPORTA: que la entrada del "?" no lo venda como una camara sana.
    # Se mira el TEXTO de esa entrada y su COLOR, y las dos mitades hacen falta: una app
    # que escribiera "SIN COMPROBAR" en verde lampara pasaria la primera sola, y una que
    # dijera "OK (sin comprobar)" en gris pasaria la segunda.
    m_desc = re.search(r"'\?'\s*:\s*\{", js)
    if not m_desc:
        raise fw.Abortado(
            "no se hallo la entrada del valor '?' en la tabla %s de app.js. Es el estado "
            "de arranque del vigilante y el unico cuyo mal manejo no se ve: sin ella este "
            "pack no puede comprobar que no se pinte como una camara sana" % TABLA_CAM)
    entrada = _bloque(js, m_desc.end() - 1)
    b.verificar(
        entrada is not None
        and re.search(r"\bOK\b", entrada, re.I) is None
        and "green" not in entrada.lower(),
        "la entrada del '?' ni dice OK ni se pinta en verde: la camara sin comprobar no "
        "se vende como comprobada",
        "la entrada del '?' de %s dice OK o se pinta en verde. '?' es el estado de "
        "arranque del vigilante -por esos pines todavia no ha pasado nada-, y en fase 1 "
        "el vigilante NO toca el ciclo: una camara que no ve no se nota en la calle y "
        "esta pantalla es el unico sitio donde alguien se entera. Pintarlo como sano es "
        "decirle al operario que la deteccion esta comprobada cuando lo unico que consta "
        "es que nadie la ha ejercido: %r" % (TABLA_CAM, (entrada or "")[:160]))

    b.control_negativo(
        re.search(r"\bOK\b", "{ texto: 'OK', frase: 'las dos ven' }", re.I) is not None
        and re.search(r"\bOK\b", "{ texto: 'SIN COMPROBAR', frase: 'no consta' }", re.I)
        is None,
        "el lector de la entrada del '?' distingue una que dice OK de una que no: si "
        "midiera por otra cosa, aprobaria las dos igual")

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
            ", ".join(sorted(set().union(*comodines.values()))) or "-"),
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
