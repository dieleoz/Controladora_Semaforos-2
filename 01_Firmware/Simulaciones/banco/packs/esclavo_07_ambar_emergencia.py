# ===== banco/packs/esclavo_07_ambar_emergencia.py =====
#
# EL NOMBRE DE UN COMANDO ES PARTE DE SU CONTRATO — N-83.
#
# El Esclavo atendia CMD:FORZAR_ROJO llamando a semaforo_iniciarFallo(), y S_FALLO no
# es rojo: es ambar intermitente a 500 ms con la TALANQUERA ARRIBA -decision del cliente
# y del PMT del 27/08, razonada dentro de escribirPines()-. El equipo contestaba
# "$ACK,CMD:FORZAR_ROJO,RESULT:OK" y subia la pluma. El comportamiento es el correcto y
# no se toca; lo que estaba mal era que el equipo DECLARARA otra cosa de la que hace.
#
# Un nombre no lo compila nadie, no lo enlaza nadie y no lo comprueba ningun tipo: es la
# clase de defecto que solo caza un instrumento que lea el texto. Y lo lee mal quien solo
# busque el literal viejo, porque el literal TIENE que seguir estando -rechazandose-.
#
# LAS TRES PROPIEDADES, Y POR QUE LA DEL MEDIO ES LA QUE IMPORTA:
#
#   1. Ningun comando ATENDIDO del Esclavo se llama FORZAR_ROJO, y ese literal se
#      rechaza nombrando el nuevo. Rechazar en silencio -o caer al DESCONOCIDO generico-
#      conservaria la mentira en la punta de quien tiene la app vieja.
#   2. El $ACK/$ERR de CADA rama nombra el MISMO comando que la rama atiende. Esta es la
#      general: la 1 mira un nombre concreto y caduca el dia que se arregle; esta impide
#      que el defecto vuelva con cualquier otro nombre, en cualquier rama, incluidas las
#      que aun no existen.
#   3. El ambar pedido por Bluetooth SOBREVIVE a un CMD_GO_RED. Medido: sin latch, la
#      guarda de main.cpp solo protegia el ambar del mando, asi que el pedido desde la
#      app se lo llevaba el siguiente latido -unos 3 s- y el operario veia al equipo
#      obedecer y volverse atras solo.
#
# NADA DE ESTO LLEVA EL NOMBRE NUEVO ESCRITO A MANO. El nombre se DEDUCE del C++ -la
# rama sin PIN que llama a semaforo_iniciarFallo()- y con el se comprueba el motivo del
# rechazo. Escribirlo aqui seria un valor por defecto disfrazado: el dia que alguien
# renombrara otra vez, el pack seguiria midiendo el nombre de ayer.
#
# EJERCE SFTY-21: el ambar pedido por Bluetooth pesa lo mismo que el del mando; ninguna
# orden de luz por radio lo revoca.

import re

NOMBRE = "esclavo_07_ambar_emergencia"
DESCRIPCION = ("el nombre de un comando dice lo que hace, y el ambar pedido por "
               "Bluetooth sobrevive al siguiente latido")

BT_ESCLAVO = ("Esclavo", "src", "bluetooth.cpp")
BT_ESCLAVO_H = ("Esclavo", "include", "bluetooth.h")
MAIN_ESCLAVO = ("Esclavo", "src", "main.cpp")
PROTOCOLO = ("Esclavo", "include", "protocolo.h")
BT_MAESTRO = ("Maestro", "src", "bluetooth.cpp")

# Las llamadas que cambian el estado del equipo. Una rama que toca alguna de estas
# ATIENDE el comando; una que solo contesta lo RECHAZA. Se listan por prefijo y no por
# nombre completo a proposito: el censo tiene que seguir valiendo cuando alguien anada
# una funcion nueva a cualquiera de esos modulos.
PREFIJOS_DE_ACCION = ("semaforo_", "demanda_", "reloj_", "config_", "coordinador_",
                      "degradado_", "modoActual_", "menu_")


def _condiciones_if(codigo):
    """Todas las condiciones de un 'if', con los parentesis equilibrados.

    Con una expresion regular no sale bien: las condiciones de main.cpp llevan
    parentesis dentro -llamadas a funcion- y se parten por el primer ')'. Partirlas
    dejaria fuera justo la mitad donde vive la guarda que se busca, y el pack aprobaria
    por no haber leido."""
    fuera = []
    for m in re.finditer(r"\bif\s*\(", codigo):
        i = m.end() - 1
        prof = 0
        for j in range(i, len(codigo)):
            if codigo[j] == "(":
                prof += 1
            elif codigo[j] == ")":
                prof -= 1
                if prof == 0:
                    fuera.append(codigo[i + 1:j])
                    break
    return fuera


def _ramas(codigo):
    """El despachador de Bluetooth, partido en ramas. Leido del C++, sin lista a mano.

    No hay tabla ni enum: el contrato ES la cadena de strcmp(). Cada rama va desde su
    comparacion hasta la siguiente, o hasta su 'return', lo que llegue antes -sin el
    corte por 'return' el filtro de PIN, que vive entre dos ramas, caeria dentro de la
    de arriba y se le achacaria un $ERR que no es suyo-.

    Devuelve [(clave, cuerpo)] en orden de aparicion."""
    cabezas = []
    for m in re.finditer(r'str(n?)cmp\s*\(\s*(cmd|accion)\s*,\s*"([^"]+)"', codigo):
        lit = m.group(3)
        # "CMD:PIN:1234:" no es una rama: es el filtro de PIN. Se reconoce por llevar
        # el PIN dentro, no por su posicion, que cambia entre las dos puntas.
        if "PIN:" in lit:
            continue
        clave = (lit[4:] if lit.startswith("CMD:") else lit).rstrip(":")
        # Ni "CMD:" a secas: en el Maestro esa comparacion es la OTRA mitad del filtro
        # de PIN -la que deja entrar SET_MODO:MENU y SET_MODO:ALCANCE sin clave-, y
        # colarla daba una rama de nombre vacio a la que se le achacaba el
        # $ERR,CMD:AUTH_FAILED del propio filtro. Era un hallazgo inventado por el
        # lector: se caza al buscador antes de acusar al firmware.
        if not clave:
            continue
        cabezas.append((m.start(), m.end(), clave))

    # El else final del despachador. Todo lo que venga despues no pertenece a ninguna
    # rama, y meterlo en la ultima le colgaria el $ERR,CMD:DESCONOCIDO del catch-all.
    mdesc = re.search(r'"\$ERR,CMD:DESCONOCIDO', codigo)
    fin_cadena = mdesc.start() if mdesc else len(codigo)

    ramas = []
    for k, (ini, fin_cabeza, clave) in enumerate(cabezas):
        tope = cabezas[k + 1][0] if k + 1 < len(cabezas) else fin_cadena
        tope = min(tope, fin_cadena) if ini < fin_cadena else tope
        cuerpo = codigo[fin_cabeza:tope]
        mret = re.search(r"\breturn\s*;", cuerpo)
        if mret:
            cuerpo = cuerpo[:mret.end()]
        ramas.append((clave, cuerpo))
    return ramas


def _nombres_en_respuestas(cuerpo):
    """Los comandos que las respuestas de una rama NOMBRAN. [(tipo, nombre)]."""
    return [(m.group(1), m.group(2))
            for m in re.finditer(r'"\$(ACK|ERR),CMD:([A-Z0-9_:]+)', cuerpo)]


def _atiende(cuerpo):
    """La rama ATIENDE el comando (lo ejecuta) o solo contesta."""
    if re.search(r'"\$ACK,', cuerpo):
        return True
    return any(re.search(r"\b%s\w+\s*\(" % p, cuerpo) for p in PREFIJOS_DE_ACCION)


def _incoherentes(codigo):
    """Ramas cuya respuesta nombra un comando distinto del que atienden."""
    malas = []
    for clave, cuerpo in _ramas(codigo):
        for tipo, nombrado in _nombres_en_respuestas(cuerpo):
            if nombrado != clave:
                malas.append((clave, tipo, nombrado))
    return malas


def correr(b, fw):
    b.titulo("N-83: el nombre de un comando es parte de su contrato")

    codigo = fw.codigo(*BT_ESCLAVO)          # sin comentarios: los comentarios de este
    ramas = _ramas(codigo)                   # fichero nombran FORZAR_ROJO a proposito
    if not ramas:
        raise fw.Abortado(
            "no se hallo ni una rama en el despachador del bluetooth.cpp del Esclavo. "
            "El contrato es una cadena de strcmp() y si cambio de forma este pack "
            "estaria midiendo un conjunto vacio, que aprueba cualquier cosa")

    claves = [c for c, _ in ramas]
    atendidos = sorted({c for c, cu in ramas if _atiende(cu)})
    rechazados = sorted({c for c, cu in ramas if not _atiende(cu)})
    b.verificar(
        True,
        "despachador del Esclavo leido del C++: atiende %s | rechaza con motivo %s"
        % (atendidos, rechazados),
        "no deberia llegarse aqui")

    # ---- El nombre nuevo se DEDUCE del C++, no se escribe aqui -------------------
    # La rama sin PIN que llama a semaforo_iniciarFallo() es, por definicion, la del
    # ambar de emergencia. Se busca por lo que HACE, que es lo unico que no puede
    # mentir; buscarla por su nombre seria dar por sabido justo lo que se comprueba.
    emergencia = sorted({c for c, cu in ramas if "semaforo_iniciarFallo" in cu})
    if len(emergencia) != 1:
        raise fw.Abortado(
            "el despachador del Esclavo tiene %d ramas que llaman a "
            "semaforo_iniciarFallo() (%s). Con ninguna no hay nombre nuevo que leer, y "
            "con varias el pack no sabria cual es el bueno: en los dos casos mediria "
            "otra cosa" % (len(emergencia), emergencia))
    NUEVO = emergencia[0]

    # ---- 1. Ningun comando ATENDIDO del Esclavo se llama FORZAR_ROJO -------------
    b.verificar(
        "FORZAR_ROJO" not in atendidos,
        "ningun comando que el Esclavo ATIENDE se llama FORZAR_ROJO: el ambar de "
        "emergencia se pide como %r, que es lo que el equipo hace de verdad "
        "-S_FALLO, ambar intermitente y talanquera ARRIBA-" % NUEVO,
        "el Esclavo sigue atendiendo FORZAR_ROJO y lo que hace es ambar intermitente "
        "con la pluma ARRIBA. El nombre promete detener el cruce y el equipo abre "
        "paso: quien lo mande creera que paro el trafico")

    b.verificar(
        not any(n == "FORZAR_ROJO" for _, cu in ramas
                for t, n in _nombres_en_respuestas(cu) if t == "ACK"),
        "ningun $ACK del Esclavo nombra FORZAR_ROJO: no queda una confirmacion que "
        "diga rojo mientras la luz va en ambar",
        "hay un $ACK,CMD:FORZAR_ROJO en el Esclavo. El acuse es lo unico que el "
        "operario ve en el telefono, asi que ahi la mentira llega entera")

    # ---- 2. El literal viejo se rechaza, y el motivo ENSENA el nombre nuevo ------
    # Las dos puertas: la forma sin PIN -que se compara contra 'cmd' entero- y la forma
    # con PIN -contra 'accion'-. Una app vieja las usa las dos y las dos tienen que
    # contestar lo mismo; si solo una diera el motivo, el mismo error se explicaria de
    # dos maneras segun por donde entre.
    puertas = {"cmd": r'strcmp\s*\(\s*cmd\s*,\s*"CMD:FORZAR_ROJO"\s*\)',
               "accion": r'strcmp\s*\(\s*accion\s*,\s*"FORZAR_ROJO"\s*\)'}
    faltan = sorted(p for p, pat in puertas.items() if not re.search(pat, codigo))
    b.verificar(
        not faltan,
        "el literal FORZAR_ROJO se sigue reconociendo por sus DOS puertas (sin PIN y "
        "con PIN): quien tenga la app o el manual viejos recibe una respuesta pensada "
        "para el, no el DESCONOCIDO generico del final",
        "el Esclavo ya no reconoce FORZAR_ROJO por %s. Cae al catch-all y contesta "
        "COMANDO_NO_SOPORTADO_EN_ESCLAVO, que no le dice a nadie como se llama ahora: "
        "un rechazo mudo conserva la mentira en la punta de quien mando la orden"
        % faltan)

    motivos = [n for c, cu in ramas if c == "FORZAR_ROJO"
               for n in re.findall(r'"\$ERR,CMD:FORZAR_ROJO,DESC:([^"]+)"', cu)]
    b.verificar(
        bool(motivos) and all(NUEVO in m for m in motivos),
        "el rechazo de FORZAR_ROJO ENSENA el nombre bueno en el motivo (%s): el que lo "
        "manda se entera de que existe y de como se llama" % sorted(set(motivos)),
        "el rechazo de FORZAR_ROJO no nombra %r en su DESC (motivos hallados: %s). "
        "Rechazar sin decir el nombre nuevo deja al operario con un comando muerto y "
        "sin saber que la funcion sigue ahi" % (NUEVO, motivos))

    b.verificar(
        all(not any(re.search(r"\b%s\w+\s*\(" % p, cu) for p in PREFIJOS_DE_ACCION)
            for c, cu in ramas if c == "FORZAR_ROJO"),
        "las ramas de FORZAR_ROJO no mueven nada: rechazan y lo dicen. No es un alias "
        "silencioso del nombre nuevo, que habria conservado la mentira intacta",
        "una rama de FORZAR_ROJO sigue ejecutando la accion. Un alias que obedece con "
        "el nombre viejo es exactamente el defecto de N-83 con una capa de pintura")

    # ---- 3. LA GENERAL: cada rama nombra el comando que atiende ------------------
    malas = _incoherentes(codigo)
    b.verificar(
        not malas,
        "las %d ramas del despachador del Esclavo nombran en su $ACK/$ERR el MISMO "
        "comando que atienden. Es la propiedad que impide que N-83 vuelva con otro "
        "nombre, en una rama que hoy no existe" % len(ramas),
        "hay ramas cuya respuesta nombra otro comando: %s. El acuse es el unico "
        "recibo que le queda a quien mando la orden; si nombra otra cosa, el operario "
        "no puede saber que le contestaron" % malas)

    # El mismo censo sobre el Maestro. NO cuenta -no se toca el Maestro en este
    # trabajo-, pero callarlo seria dejar el hallazgo sin rastro: una nota que no se
    # escribe se vuelve a descubrir dentro de seis meses.
    malas_m = _incoherentes(fw.codigo(*BT_MAESTRO))
    if malas_m:
        b.reportar(
            "el MAESTRO tiene %d rama(s) cuyo acuse nombra otro comando" % len(malas_m),
            ["Censo con el mismo lector, sobre Maestro/src/bluetooth.cpp:"] +
            ["  la rama %r contesta $%s,CMD:%s" % (c, t, n) for c, t, n in malas_m] +
            ["La app manda CMD:PIN:1234:MANUAL:CAMBIAR_TURNO y recibe un acuse que",
             "nombra CAMBIAR_TURNO: una app que empareje acuses por nombre no lo casa.",
             "No se arregla aqui -este trabajo no toca el Maestro- y no se cuenta como",
             "comprobacion: queda anotado para quien lo lleve."])

    # ---- 4. El ambar pedido por Bluetooth sobrevive a un CMD_GO_RED --------------
    # El latch se lee del C++: la variable que la rama del ambar pone a true, y el
    # getter que la publica. Nada de nombres escritos aqui.
    cuerpo_emergencia = [cu for c, cu in ramas if c == NUEVO]
    latches = sorted({m.group(1) for cu in cuerpo_emergencia
                      for m in re.finditer(r"\b(\w+)\s*=\s*true\s*;", cu)})
    if len(latches) != 1:
        raise fw.Abortado(
            "la rama %r no arma exactamente un latch (%s). Sin saber cual es la "
            "bandera, la comprobacion de que el ambar sobrevive no se puede escribir "
            "-y escribir el nombre a mano seria un valor por defecto-" % (NUEVO, latches))
    LATCH = latches[0]

    mget = re.search(r"bool\s+(\w+)\s*\(\s*\)\s*\{\s*return\s+%s\s*;" % LATCH, codigo)
    if not mget:
        raise fw.Abortado(
            "el latch %r no lo publica ninguna funcion en bluetooth.cpp. Una bandera "
            "que main.cpp no puede consultar no protege nada, y el pack no tendria "
            "que buscar en las guardas" % LATCH)
    GETTER = mget.group(1)

    b.verificar(
        re.search(r"bool\s+%s\s*\(\s*\)\s*;" % GETTER, fw.codigo(*BT_ESCLAVO_H))
        is not None,
        "%s() esta declarado en bluetooth.h: el latch es parte del contrato del modulo, "
        "no una variable que main.cpp adivina" % GETTER,
        "%s() se define en el .cpp y no se declara en bluetooth.h. Es la forma "
        "silenciosa de N-73 al reves: una funcion que existe y que nadie puede llamar"
        % GETTER)

    # Que exista y que se llame son dos cosas distintas (N-73).
    principal = fw.codigo(*MAIN_ESCLAVO)
    b.verificar(
        principal.count("%s()" % GETTER) > 0,
        "main.cpp llama a %s() %d vez/veces: el latch no es una funcion huerfana"
        % (GETTER, principal.count("%s()" % GETTER)),
        "nadie llama a %s(). Declarada, definida, documentada y sin un solo llamador "
        "es exactamente N-73: el ambar de la app se seguiria revocando solo" % GETTER)

    # CMD_GO_RED se relee del protocolo. Si ese #define desapareciera, la guarda que se
    # busca mas abajo no existiria y este pack no puede fingir que la midio.
    fw.comando(PROTOCOLO, "CMD_GO_RED")

    condiciones = _condiciones_if(principal)
    vetos_mando = [c for c in condiciones if "mando_ambarLocal()" in c]
    if not vetos_mando:
        raise fw.Abortado(
            "no se hallo en main.cpp ni una guarda con mando_ambarLocal(). Ese es el "
            "patron que el ambar de Bluetooth tiene que igualar; sin el, el pack no "
            "sabe donde mirar")

    sin_bluetooth = [c.split("\n")[0].strip() for c in vetos_mando
                     if "%s()" % GETTER not in c]
    b.verificar(
        not sin_bluetooth,
        "las %d guardas de main.cpp que respetan el ambar del mando respetan TAMBIEN "
        "el pedido por Bluetooth. Una orden de emergencia vale lo mismo la haya dado "
        "un dedo en el gabinete o un dedo en el telefono" % len(vetos_mando),
        "hay %d guarda(s) que solo miran mando_ambarLocal(): %s. Por ahi se cuela la "
        "revocacion: el ambar de la app dura hasta el siguiente latido del Maestro "
        "-unos 3 s- y el operario ve el equipo obedecer y volverse atras solo"
        % (len(sin_bluetooth), sin_bluetooth))

    # La guarda concreta que revocaba: la que se dispara con un CMD_GO_RED encontrando
    # el nodo ya en S_FALLO. Se comprueba aparte de la de arriba porque es un 'if'
    # INDEPENDIENTE -no un else-: proteger solo el primero deja este intacto.
    revocacion = [c for c in condiciones
                  if "S_FALLO" in c and "CMD_GO_RED" in c]
    b.verificar(
        bool(revocacion) and all("%s()" % GETTER in c for c in revocacion),
        "la guarda que revocaba el ambar -CMD_GO_RED sobre un nodo ya en S_FALLO- "
        "consulta el latch de Bluetooth: el ambar pedido desde la app SOBREVIVE al "
        "siguiente CMD_GO_RED",
        "la guarda de la revocacion (%s) no consulta %s(). Es la que se lleva el "
        "ambar de la app: llega un CMD_GO_RED, el nodo esta en S_FALLO y vuelve a "
        "rojo por su cuenta" % (revocacion, GETTER))

    # ---- 5. Un latch persistente sin salida seria peor que el defecto ------------
    # Si el ambar de Bluetooth no se pudiera revocar, el nodo quedaria sordo al Maestro
    # hasta el siguiente corte de corriente. La salida tiene que estar en el C++, no en
    # la intencion de quien lo escribio.
    b.verificar(
        re.search(r"\b%s\s*=\s*false\s*;" % LATCH, codigo) is not None,
        "el latch %r tiene salida escrita en el C++ (se pone a false): el ambar "
        "persistente se puede revocar, igual que el del mando se revoca con A.A.A"
        % LATCH,
        "el latch %r no se pone a false en ningun sitio. Un ambar del que no se sale "
        "deja el nodo sordo al Maestro hasta el proximo corte de corriente, que es "
        "peor que la revocacion que esto venia a arreglar" % LATCH)

    # ---- 6. Controles negativos --------------------------------------------------
    # Sin esto, todo lo de arriba podria estar aprobando por no encontrar nada.
    FALSO = '''
      if (strcmp(cmd, "CMD:PARAR_TODO") == 0) {
        semaforo_iniciarFallo();
        enviarTramaConCrc("$ACK,CMD:FORZAR_ROJO,RESULT:OK");
        return;
      }
      if (strncmp(cmd, "CMD:PIN:1234:", 13) != 0) {
        enviarTramaConCrc("$ERR,CMD:AUTH_FAILED,DESC:PIN_INVALIDO");
        return;
      }
      if (strcmp(accion, "FORZAR_ROJO") == 0) {
        semaforo_iniciarFallo();
        enviarTramaConCrc("$ACK,CMD:FORZAR_ROJO,RESULT:OK");
      } else {
        enviarTramaConCrc("$ERR,CMD:DESCONOCIDO,DESC:NO");
      }
    '''
    ramas_falsas = _ramas(FALSO)
    b.control_negativo(
        ("PARAR_TODO", "ACK", "FORZAR_ROJO") in _incoherentes(FALSO),
        "sobre un despachador con un $ACK que nombra otro comando, el censo de "
        "coherencia lo senala; no aprueba por no mirar")
    b.control_negativo(
        "FORZAR_ROJO" in {c for c, cu in ramas_falsas if _atiende(cu)},
        "el lector distingue una rama que ATIENDE FORZAR_ROJO de una que lo rechaza "
        "con motivo — que es la diferencia entera de este arreglo")
    b.control_negativo(
        ("AUTH_FAILED" not in [n for _, cu in ramas_falsas
                               for _, n in _nombres_en_respuestas(cu)] and
         "DESCONOCIDO" not in [n for _, cu in ramas_falsas
                               for _, n in _nombres_en_respuestas(cu)]),
        "el filtro de PIN y el catch-all no se le cuelgan a ninguna rama: sin ese "
        "corte, el censo acusaria de incoherentes a dos ramas correctas")
    b.control_negativo(
        _condiciones_if("if (!a() && b(x, y) && c()) {") == ["!a() && b(x, y) && c()"],
        "el lector de condiciones no se parte en el primer ')': las guardas de main.cpp "
        "llevan llamadas dentro y leer media condicion dejaria fuera justo la mitad "
        "donde vive el latch")
