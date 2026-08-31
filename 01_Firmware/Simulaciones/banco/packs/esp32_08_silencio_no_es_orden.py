# ===== banco/packs/esp32_08_silencio_no_es_orden.py =====
#
# CON EL TX DEL PUENTE MUDO, AUSENTE O EN REPOSO: NINGUNA ACCION.
#
# POR QUE ES UNA PROPIEDAD Y NO UNA OBVIEDAD.
#
# Un enlace serie en reposo esta EN ALTO, y un pin flotante tambien puede leerse alto.
# Desde el STM32, "el puente no dice nada" y "el puente no esta" NO SON DISTINGUIBLES.
# Por eso ninguno de los dos lados puede deducir nada de un silencio, y por eso el
# puente no puede tener un estado de arranque que signifique algo: cualquier cosa que
# mande sin que se la pidan entra por el MISMO camino por el que entran las ordenes del
# operario, a un micro que gobierna un cruce y que no valida quien habla.
#
# LAS TRES FORMAS EN QUE ESTO SE ROMPE, Y LAS TRES SE VIGILAN AQUI:
#
#   1. UN SALUDO EN setup(). Parece inofensivo -"para que el equipo sepa que estoy"- y
#      es una orden que nadie pidio, mandada antes de que haya nadie escuchando.
#   2. UN MODO POR DEFECTO. Un puente que al arrancar "deja el equipo en un estado
#      conocido" esta decidiendo por el operario.
#   3. UNA ACCION COLGADA DE LA AUSENCIA DE BYTES. "Si no llega nada en N ms, hacer X"
#      convierte un cable suelto en una orden. Es el caso mas facil de escribir con
#      buena intencion y el unico de los tres que ademas dispara solo.
#
# Y LA CUARTA, QUE ES DEL STM32 Y EL PUENTE NO DEBE COMPENSAR: un byte suelto sin
# terminador no dispara el despachador (E-1). Anadirle terminadores por su cuenta seria
# el puente completando comandos a medias, y un comando a medias que se completa es un
# comando que el operario no escribio.

import re

NOMBRE = "esp32_08_silencio_no_es_orden"
DESCRIPCION = "el puente no saluda, no tiene modo por defecto y nada cuelga de la ausencia de bytes"

ROL = "ESP32_Expansion"

# Todo lo que pone bytes en un cable. Si aparece en un arranque, es un saludo.
EMISORES = (r"enlace_escribirLinea\s*\(", r"transporte_escribir\s*\(",
            r"puente_emitirPropio\s*\(", r"\.write\s*\(", r"\.print(?:ln)?\s*\(")


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


def _emite(texto):
    return [p for p in EMISORES if re.search(p, texto)]


def correr(b, fw):
    b.titulo("Silencio no es orden: ni saludo, ni modo por defecto, ni accion por ausencia")

    main = fw.codigo("ESP32_Expansion", "src", "main.cpp")
    puente = fw.codigo("ESP32_Expansion", "src", "puente.cpp")
    enlace = fw.codigo("ESP32_Expansion", "src", "enlace_stm32.cpp")

    # ---- 1. setup() no manda NADA --------------------------------------------
    setup = _cuerpo(main, r"void\s+setup\s*\(\s*\)")
    if setup is None:
        raise fw.Abortado(
            "no se hallo setup() en %s/src/main.cpp. Es donde vive la propiedad "
            "'el puente no saluda'; sin poder leerlo, aprobarla seria aprobar sin mirar"
            % ROL)

    emisores = _emite(setup)
    b.verificar(
        not emisores,
        "setup() no pone un solo byte en ningun cable: el puente no saluda",
        "setup() EMITE (%s). Un saludo del puente es una orden que nadie pidio, entrando "
        "por el mismo camino por el que entran las que si se piden, a un micro que "
        "gobierna un cruce y que no valida quien habla" % ", ".join(emisores))

    # ---- 2. Los arranques de los dos modulos del camino de datos, tampoco -----
    for fichero, codigo, firma in (
            ("puente.cpp", puente, r"void\s+puente_setup\s*\(\s*\)"),
            ("enlace_stm32.cpp", enlace, r"void\s+enlace_setup\s*\(\s*\)")):
        cuerpo = _cuerpo(codigo, firma)
        if cuerpo is None:
            raise fw.Abortado(
                "no se hallo el arranque de %s en %s/src: sin leerlo no se puede "
                "comprobar que no manda nada al abrir" % (fichero, ROL))
        # begin() abre el puerto, no emite. Se excluye por nombre y no por patron para
        # que un .write() colado al lado siga saltando.
        sinBegin = re.sub(r"\.begin\s*\(", ".ABRIR(", cuerpo)
        emisores = _emite(sinBegin)
        b.verificar(
            not emisores,
            "el arranque de %s abre el puerto y no manda nada por el" % fichero,
            "el arranque de %s EMITE (%s). Abrir un puerto y saludar por el no son lo "
            "mismo: lo primero es necesario y lo segundo es una orden"
            % (fichero, ", ".join(emisores)))

    # ---- 3. NADA CUELGA DE LA AUSENCIA DE BYTES ------------------------------
    #
    # Se busca el patron completo: una condicion sobre "no hay datos" o sobre un
    # temporizador de silencio que lleve a una emision. Sin reloj en el camino de datos
    # -que esp32_07 comprueba por su lado- la segunda mitad no se puede ni escribir, y
    # aqui se vigila la primera.
    sospechosos = []
    for m in re.finditer(r"if\s*\(\s*!\s*(?:transporte_disponible|enlace_disponible)"
                         r"\s*\(\s*\)\s*\)\s*\{", puente):
        cuerpo = _bloque(puente, m.end() - 1)
        if cuerpo and _emite(cuerpo):
            sospechosos.append(cuerpo.strip()[:60])
    for m in re.finditer(r"(?:transporte_disponible|enlace_disponible)\s*\(\s*\)\s*=="
                         r"\s*0\s*\)\s*\{", puente):
        cuerpo = _bloque(puente, puente.find("{", m.end() - 1))
        if cuerpo and _emite(cuerpo):
            sospechosos.append(cuerpo.strip()[:60])

    b.verificar(
        not sospechosos,
        "ninguna rama del bombeo actua sobre la AUSENCIA de bytes: un cable suelto no "
        "se convierte en una orden",
        "hay accion colgada de la ausencia de bytes: %s. 'Si no llega nada, hacer X' "
        "convierte un conector flojo en un comando, y ademas dispara solo"
        % " | ".join(sospechosos))

    # ---- 4. No hay modo por defecto que aplicar ------------------------------
    #
    # El puente no guarda estado del equipo, asi que no puede tener uno "conocido" al que
    # volver. Se comprueba por la via que lo hace imposible: todo lo que sale hacia el
    # STM32 viene del buffer de entrada, y el arranque no lo rellena.
    b.verificar(
        re.search(r"deApp\s*\[\s*0\s*\]\s*=", _cuerpo(puente, r"void\s+puente_setup\s*\(\s*\)")) is None,
        "el arranque no precarga el buffer de entrada: no hay un comando 'de fabrica' "
        "esperando a salir en la primera vuelta",
        "el arranque escribe en el buffer de entrada. Un comando precargado sale en la "
        "primera vuelta del bombeo como si lo hubiera mandado el operario, y no hay "
        "forma de distinguirlo desde el STM32")

    # ---- 5. El puente no completa comandos a medias --------------------------
    #
    # E-1 es una propiedad del STM32 -sin terminador el despachador no dispara- y el
    # puente NO debe compensarla. El terminador se pone al reenviar UNA LINEA COMPLETA,
    # nunca a un byte suelto: la guarda es que la emision viva dentro de la rama del
    # terminador recibido.
    ida = _cuerpo(puente, r"static\s+void\s+desdeLaApp\s*\(\s*\)")
    if ida is None:
        raise fw.Abortado(
            "no se hallo desdeLaApp() en %s/src/puente.cpp: es donde se decide cuando "
            "una linea esta completa" % ROL)
    m = re.search(r"if\s*\(\s*c\s*==\s*'\\r'\s*\|\|\s*c\s*==\s*'\\n'\s*\)\s*\{", ida)
    ramaTerminador = _bloque(ida, m.end() - 1) if m else None
    b.verificar(
        ramaTerminador is not None
        and "enlace_escribirLinea" in ramaTerminador
        and ida.count("enlace_escribirLinea") == ramaTerminador.count("enlace_escribirLinea"),
        "solo se reenvia cuando llego el terminador: un byte suelto no se convierte en "
        "comando anadiendole un '\\n' por nuestra cuenta",
        "hay reenvios fuera de la rama del terminador. El puente estaria completando "
        "comandos a medias, y un comando a medias que se completa es un comando que el "
        "operario no escribio")

    # ---- CONTROLES NEGATIVOS -------------------------------------------------
    b.control_negativo(
        bool(_emite('{ vigilante_armar(); enlace_escribirLinea("X", 1); }')),
        "un saludo colado en setup() se detecta")

    porAusencia = ('{ if (!transporte_disponible()) { '
                   'enlace_escribirLinea(porDefecto, 8); } }')
    mm = re.search(r"if\s*\(\s*!\s*transporte_disponible\s*\(\s*\)\s*\)\s*\{", porAusencia)
    b.control_negativo(
        mm is not None and bool(_emite(_bloque(porAusencia, mm.end() - 1))),
        "una accion colgada de 'no llega nada' se detecta aunque lo que mande no sea un "
        "literal")

    b.control_negativo(
        not _emite("{ SerialBT.ABRIR(9600); }") and bool(_emite("{ SerialBT.print(x); }")),
        "el filtro distingue ABRIR un puerto de ESCRIBIR por el: abrir es necesario, "
        "escribir al arrancar es un saludo")
