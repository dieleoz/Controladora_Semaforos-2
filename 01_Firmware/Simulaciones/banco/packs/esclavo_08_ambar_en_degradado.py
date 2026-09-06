# ===== banco/packs/esclavo_08_ambar_en_degradado.py =====
#
# N-106 — EL AMBAR DE EMERGENCIA DE LA APP NO SACA AL ESCLAVO DEL MODO DEGRADADO.
#
# ESTE PACK NACE EN ROJO, Y ESO ES EL RESULTADO CORRECTO. No acompana a un arreglo:
# lo precede. Si el firmware se arreglara primero, nadie sabria nunca si el arreglo
# funciono ni si alguien lo deshizo despues; un instrumento que llega detras del
# arreglo no ha visto jamas el defecto que dice vigilar (CLAUDE.md §8.bis).
#
# QUE ES EL DEFECTO, MEDIDO Y NO LEIDO
# ------------------------------------
# El Esclavo tiene DOS vias externas para pedir ambar de emergencia, y el firmware
# declara por escrito que valen lo mismo -bluetooth.cpp, cabecera del latch: "UNA
# EMERGENCIA PEDIDA POR BLUETOOTH VALE LO MISMO QUE UNA DEL MANDO"-. No valen lo mismo:
#
#   mando, B.B.B      case ACC_AMBAR de mando.cpp: pone ambarLocal y, SI el Degradado
#                     gobierna la luz, sale por degradado_salir() -el todo-rojo de
#                     despedida-. Si no gobierna, semaforo_iniciarFallo() y ya.
#   app, AMBAR_EMERG. bluetooth.cpp: semaforo_iniciarFallo() y el latch, sin preguntar
#                     por el Degradado. Y contesta "$ACK,...,RESULT:OK" igual.
#
# El porque es vial y esta escrito en el propio mando.cpp: saltar de un verde por reloj
# directo a ambar intermitente "le daria a quien ya venia lanzado una senal que invita a
# negociar el paso mientras aun cree tener prioridad". Es la misma razon por la que el
# Degradado entra y sale siempre por todo-rojo.
#
# LO QUE ESTE PACK MIDE Y LO QUE NO
# ---------------------------------
# Mide el TEXTO del C++: que la via existe o que falta. NO ejerce el escenario -no hay
# maquina de estados aqui- y por tanto no demuestra la consecuencia dinamica; esa queda
# abajo, en un reportar(), escrita como lo que es. Es la diferencia entre "el firmware
# no llama a degradado_salir()", que es un hecho medido, y "el ambar se cae solo a los
# 30 s", que es un razonamiento sobre codigo que nadie ha corrido.
#
# POR QUE propiedad() Y NO verificar() EN LAS DOS PRIMERAS
# -------------------------------------------------------
# Las dos son SFTY-21: la salida del Modo Degradado. No son un desajuste de forma ni una
# incoherencia de nombres -eso es esclavo_07-: son una regla de seguridad que el banco
# ENCUENTRA INCUMPLIDA en el firmware de hoy. propiedad() imprime ROTA, que dice
# exactamente eso -el firmware no la resiste-, mientras que FALLA se lee como "el banco
# esta mal". El veredicto es el mismo; lo que cambia es a quien senala, y aqui senala al
# firmware. La tercera es una coherencia entre dos ramas, no una regla de seguridad, y
# por eso va como verificar().
#
# Y NO ES LA TRAMPA DE §3: ningun firmware posible puede aprobar el alias de CMD_DELTA,
# pero este defecto lo cierra UNA LINEA -la misma que ya tiene mando.cpp-. Una propiedad
# que el firmware SI puede cumplir es una comprobacion, no una nota.
#
# LO QUE ESTE PACK NO DECIDE
# --------------------------
# Que debe hacer exactamente el ambar de la app cuando el Degradado gobierna la luz
# -salir ordenado como B.B.B, o quedarse- lo ve un conductor, y es decision del
# responsable. Por eso las comprobaciones exigen que la rama SE ENTERE del Degradado
# -que lo consulte y que el fichero sepa salir-, no una implementacion concreta.
#
# EJERCE SFTY-21: la salida del Modo Degradado por las vias externas de ambar. Ninguna
# via que ponga el equipo en ambar puede ignorar quien gobierna la luz.

import re

from banco.packs.esclavo_07_ambar_emergencia import _ramas

NOMBRE = "esclavo_08_ambar_en_degradado"
DESCRIPCION = ("toda via externa que ponga el Esclavo en ambar sabe salir del Modo "
               "Degradado (N-106)")

BT_ESCLAVO = ("Esclavo", "src", "bluetooth.cpp")
MANDO_ESCLAVO = ("Esclavo", "src", "mando.cpp")

# Las llamadas se buscan con el parentesis vacio Y el punto y coma: asi la DEFINICION
# -que termina en '{'- no se cuenta como llamador. Sin ese detalle, semaforo.cpp
# aparecia poniendo el equipo en ambar por definir la funcion, y modo_degradado.cpp
# aparecia saliendo de si mismo.
RE_LLAMA_AMBAR = re.compile(r"\bsemaforo_iniciarFallo\s*\(\s*\)\s*;")
RE_LLAMA_SALIR = re.compile(r"\bdegradado_salir\s*\(\s*\)\s*;")
RE_DEF_AMBAR = re.compile(r"\bvoid\s+semaforo_iniciarFallo\s*\(\s*\)\s*\{")
RE_DEF_SALIR = re.compile(r"\bvoid\s+degradado_salir\s*\(\s*\)\s*\{")
RE_CONSULTA_GOB = re.compile(r"\bdegradado_gobiernaLuz\s*\(\s*\)")

# Las llamadas que CAMBIAN el estado del equipo. Se comparan entre las dos puertas del
# mismo comando; los literales de respuesta y los eventos quedan fuera a proposito,
# porque el detalle del $EVENT SI es distinto entre ellas y debe serlo.
RE_ACCION = re.compile(r"\b((?:semaforo|demanda|reloj|config|coordinador|degradado)"
                       r"_\w+)\s*\(")


def _censo(fw):
    """Por fichero de Esclavo/src: quien pone ambar, quien sale, quien pregunta.

    Se censa el DIRECTORIO, no una lista escrita aqui. Una lista a mano se queda corta
    el dia que alguien anade un .cpp, y entonces la propiedad aprueba sin haber mirado
    donde hacia falta -que es como se cuela lo que este pack persigue-."""
    fuera = {}
    for n in fw.fuentes_de("Esclavo", "src"):
        c = fw.codigo("Esclavo", "src", n)
        fuera[n] = {
            "pone_ambar": len(RE_LLAMA_AMBAR.findall(c)),
            "sale": len(RE_LLAMA_SALIR.findall(c)),
            "consulta": len(RE_CONSULTA_GOB.findall(c)),
            "define_ambar": bool(RE_DEF_AMBAR.search(c)),
            "define_salir": bool(RE_DEF_SALIR.search(c)),
        }
    return fuera


def _ciegas(censo):
    """Ficheros que ponen el equipo en ambar SIN enterarse del Modo Degradado.

    Las dos exenciones se DEDUCEN del fuente, no se escriben:

      - el modulo que DEFINE degradado_salir() es el Degradado mismo. Exigirle que se
        consulte a si mismo no significa nada: su ambar -la rendicion por el limite
        duro- es una transicion interna del propio modo.
      - el modulo que DEFINE semaforo_iniciarFallo() es la barrera de salidas
        (CLAUDE.md §6). Ahi la funcion se declara, no se pide.

    Enterarse vale de las dos formas, y a proposito: llamando a degradado_salir() -salir
    ordenado, como el mando- o consultando degradado_gobiernaLuz() para no pisar al modo
    -como la caida por silencio de SFTY-6, que se suspende en vez de salir-. Las dos son
    respuestas legitimas; ignorarlo no lo es."""
    return sorted(n for n, d in censo.items()
                  if d["pone_ambar"] and not d["define_salir"]
                  and not d["define_ambar"]
                  and not (d["sale"] or d["consulta"]))


def _acciones(cuerpo):
    """El conjunto de llamadas que cambian estado dentro de un bloque."""
    return set(RE_ACCION.findall(cuerpo))


def correr(b, fw):
    b.titulo("N-106: el ambar de la app y la salida del Modo Degradado")

    censo = _censo(fw)

    # ---- Descartar al buscador antes de acusar a nadie (CLAUDE.md §4) -----------
    # Un "no aparece" no es un hallazgo hasta haber descartado al buscador. Si el lector
    # no encuentra la definicion del modo, o no encuentra a nadie poniendo ambar, el
    # roto es este pack y no el firmware: acusar entonces seria inventar un defecto.
    duenos = sorted(n for n, d in censo.items() if d["define_salir"])
    if len(duenos) != 1:
        raise fw.Abortado(
            "en Esclavo/src hay %d fichero(s) que definen degradado_salir() (%s). Con "
            "ninguno el pack no sabria a quien eximir del censo, y con varios el modo "
            "tendria dos duenos: en los dos casos estaria midiendo otra cosa"
            % (len(duenos), duenos))

    ponen = sorted(n for n, d in censo.items() if d["pone_ambar"])
    if len(ponen) < 2:
        raise fw.Abortado(
            "el censo solo hallo %d fichero(s) llamando a semaforo_iniciarFallo() en "
            "Esclavo/src (%s). El ambar de emergencia se pide desde varios sitios; con "
            "un conjunto casi vacio la propiedad aprobaria por no haber mirado"
            % (len(ponen), ponen))

    salen = sorted(n for n, d in censo.items() if d["sale"])

    # El censo se publica pero NO cuenta: es la medida sobre la que se apoyan las
    # comprobaciones de abajo, no una comprobacion en si. Contarlo inflaria el total
    # con una linea que ningun firmware puede fallar.
    b.reportar(
        "censo de Esclavo/src leido del directorio (%d ficheros)" % len(censo),
        ["degradado_salir() lo DEFINE: %s" % duenos[0],
         "degradado_salir() lo LLAMAN:  %s" % (", ".join(salen) or "nadie")] +
        ["ponen el equipo en ambar:     %s"
         % ", ".join("%s (x%d%s)" % (n, censo[n]["pone_ambar"],
                                     "" if censo[n]["consulta"] or censo[n]["sale"]
                                     else ", sin mirar el Degradado")
                     for n in ponen)])

    # ---- 1. Ninguna via de ambar ignora quien gobierna la luz -------------------
    ciegas = _ciegas(censo)
    b.propiedad(
        not ciegas,
        "los %d ficheros de Esclavo/src que ponen el equipo en ambar se enteran del "
        "Modo Degradado: o salen por degradado_salir() o consultan "
        "degradado_gobiernaLuz() para no pisarlo (%s)"
        % (len(ponen), ", ".join(n for n in ponen if n not in ciegas)),
        "SFTY-21 ROTA: %s pone(n) el equipo en ambar SIN preguntar por el Modo "
        "Degradado ni llamar a degradado_salir(). El mando puede sacar al Esclavo del "
        "Degradado y la pantalla puede; esta via NO, y contesta que si. Medido sobre "
        "el fuente; la consecuencia dinamica no se ejerce aqui" % ", ".join(ciegas))

    # ---- 2. La rama del ambar consulta el Degradado, como el molde del mando ----
    # El molde de como se hace bien vive en el mismo repositorio: el case ACC_AMBAR de
    # mando.cpp. Se comprueba que existe ANTES de exigirselo a nadie -si el molde
    # desapareciera, esta comprobacion estaria pidiendo algo que ya no se hace en
    # ningun sitio y habria que replantearla, no imponerla-.
    mando = fw.codigo(*MANDO_ESCLAVO)
    if not (RE_LLAMA_SALIR.search(mando) and RE_CONSULTA_GOB.search(mando)):
        raise fw.Abortado(
            "mando.cpp del Esclavo ya no consulta degradado_gobiernaLuz() ni llama a "
            "degradado_salir(). Ese es el molde contra el que se mide el ambar de la "
            "app; sin el, el pack no tiene con que comparar")

    codigo = fw.codigo(*BT_ESCLAVO)
    ramas = _ramas(codigo)
    if not ramas:
        raise fw.Abortado(
            "no se hallo ni una rama en el despachador de bluetooth.cpp del Esclavo. "
            "El contrato es una cadena de strcmp() y si cambio de forma este pack "
            "estaria midiendo un conjunto vacio, que aprueba cualquier cosa")

    # La rama del ambar se DEDUCE por lo que hace -llamar a semaforo_iniciarFallo()-,
    # no por su nombre. Buscarla por el literal seria dar por sabido justo lo que se
    # mide, y ademas caducaria el dia que el comando se renombre otra vez (N-83).
    puertas = [(c, cu) for c, cu in ramas if RE_LLAMA_AMBAR.search(cu)]
    if not puertas:
        raise fw.Abortado(
            "ninguna rama del despachador del Esclavo llama a semaforo_iniciarFallo(). "
            "Sin la rama del ambar de emergencia no hay nada que comparar con el mando")

    # Se cuentan PUERTAS, no nombres: el ambar de emergencia entra por dos ramas que se
    # llaman igual -sin PIN contra 'cmd' y con PIN contra 'accion'-. Deduplicar por
    # nombre diria "1 puerta" donde hay dos abiertas, y ese uno se lee como si media
    # estuviera arreglada.
    sordas = [c for c, cu in puertas if not RE_CONSULTA_GOB.search(cu)]
    b.propiedad(
        not sordas,
        "las %d puertas del ambar de emergencia por Bluetooth (%s) consultan "
        "degradado_gobiernaLuz() dentro de su bloque, igual que el case ACC_AMBAR del "
        "mando" % (len(puertas), ", ".join(c for c, _ in puertas)),
        "SFTY-21 ROTA: %d de las %d puertas del ambar por Bluetooth (%s) no consultan "
        "degradado_gobiernaLuz(). El firmware declara que una emergencia pedida por "
        "Bluetooth vale lo mismo que una del mando; en Degradado no vale lo mismo, y "
        "el $ACK,RESULT:OK sale igual"
        % (len(sordas), len(puertas), ", ".join(sordas)))

    # ---- 3. Las dos puertas del mismo comando ejecutan lo mismo -----------------
    # Esta SI la cumple el firmware de hoy, y se escribe por lo que viene despues: el
    # ambar de emergencia tiene dos entradas -sin PIN y con PIN- y un arreglo aplicado
    # a una sola dejaria la mitad del defecto en pie, con las dos entradas contestando
    # el mismo $ACK. Es el trinquete del arreglo de N-106, no un adorno.
    porcomando = {}
    for c, cu in puertas:
        porcomando.setdefault(c, []).append(_acciones(cu))
    dispares = sorted(c for c, accs in porcomando.items()
                      if len(accs) > 1 and any(a != accs[0] for a in accs[1:]))
    b.verificar(
        not dispares,
        "las puertas de un mismo comando de ambar ejecutan el MISMO conjunto de "
        "acciones (%s): un arreglo a medias -aplicado a la entrada con PIN y no a la "
        "de sin PIN, o al reves- no puede pasar por aqui"
        % "; ".join("%s -> %s" % (c, sorted(accs[0])) for c, accs in porcomando.items()),
        "las dos entradas de %s ejecutan cosas distintas: %s. La app manda una y el "
        "manual documenta la otra; que hagan cosas distintas segun por donde entre es "
        "el mismo error contestado de dos maneras"
        % (", ".join(dispares),
           {c: [sorted(a) for a in porcomando[c]] for c in dispares}))

    # ---- La consecuencia dinamica: RAZONADA, NO EJERCIDA ------------------------
    # Va en reportar() y no cuenta, y el motivo es el de siempre: aqui no hay maquina de
    # estados, asi que esto es un razonamiento sobre codigo leido. Escribirlo como
    # comprobacion seria vender por medida lo que es una hipotesis; callarlo seria dejar
    # el hallazgo sin rastro para que alguien lo redescubra dentro de seis meses.
    sostenedor = fw.codigo("Esclavo", "src", "modo_degradado.cpp")
    # La REVOCACION, no la declaracion. Buscar "ambarEmergencia" y "= false" en la misma
    # linea casaba primero con 'static bool ambarEmergencia = false;', que es donde nace
    # el latch y no donde se apaga: la nota habria citado la linea equivocada, y una
    # nota que cita mal es la primera mitad de una causa falsa (CLAUDE.md §4).
    revoca = [l.strip() for l in fw.codigo(*BT_ESCLAVO).split("\n")
              if re.search(r"\bif\s*\(.*\bambarEmergencia\s*=\s*false\s*;", l)]
    if ciegas and revoca and "aplicarLuz" in sostenedor:
        b.reportar(
            "el latch del ambar por Bluetooth podria revocarse SOLO en Degradado "
            "(razonado sobre el fuente, NO ejercido)",
            ["La revocacion automatica es: %s" % revoca[0],
             "y su comentario justifica la seguridad diciendo que, con el latch puesto,",
             "main.cpp no obedece ordenes de luz y por tanto nadie saca al nodo de",
             "S_FALLO. Eso vale para la RADIO. El sostenedor del Degradado escribe luz",
             "desde otro sitio -aplicarLuz() de modo_degradado.cpp, llamado por",
             "degradado_actualizar() en cada vuelta del loop- y no lo veta ninguna de",
             "las guardas de bluetooth_ambarEmergencia().",
             "Con aplicarLuz() guardado por 'if (verde == verdeAplicado) return;' la",
             "escritura no es cada vuelta: llega en el siguiente CAMBIO DE FASE. O sea",
             "que el ambar pedido desde el telefono aguantaria hasta ahi y despues",
             "podria caerse solo, con el $ACK,RESULT:OK ya enviado.",
             "NO SE ARREGLA AQUI y no se cuenta: que debe hacer el ambar de la app en",
             "Degradado lo ve un conductor, y es decision del responsable."])

    # ---- 4. Controles negativos --------------------------------------------------
    # Sin esto, todo lo de arriba podria estar aprobando -o acusando- por no haber
    # sabido leer. Cada detector se ejerce contra su caso malo Y contra su caso bueno:
    # un detector que senala siempre es tan inutil como uno que no senala nunca.
    SANO = {"bluetooth.cpp": {"pone_ambar": 2, "sale": 1, "consulta": 1,
                              "define_ambar": False, "define_salir": False},
            "mando.cpp": {"pone_ambar": 2, "sale": 2, "consulta": 3,
                          "define_ambar": False, "define_salir": False},
            "modo_degradado.cpp": {"pone_ambar": 1, "sale": 0, "consulta": 1,
                                   "define_ambar": False, "define_salir": True},
            "semaforo.cpp": {"pone_ambar": 0, "sale": 0, "consulta": 0,
                             "define_ambar": True, "define_salir": False}}
    ENFERMO = {n: dict(d) for n, d in SANO.items()}
    ENFERMO["bluetooth.cpp"].update(sale=0, consulta=0)

    b.control_negativo(
        _ciegas(ENFERMO) == ["bluetooth.cpp"] and _ciegas(SANO) == [],
        "el censo de ficheros senala al que pone ambar sin mirar el Degradado y NO "
        "senala al que si lo mira — sabe distinguir el arreglo del defecto")
    b.control_negativo(
        _ciegas({"modo_degradado.cpp": SANO["modo_degradado.cpp"]}) == [],
        "el dueno del modo no se acusa a si mismo: su ambar es la rendicion por el "
        "limite duro, una transicion interna, no una via externa que lo ignore")

    # ---- 4. A-11: EL ARBITRAJE DEL CICLO, RECALCULADO DE LAS DOS PUNTAS ---------
    #
    # LA PREGUNTA QUE ESTO CONTESTA: si el Esclavo puede entrar en Modo Degradado desde
    # la app (A-11), puede quedarse en el mientras el Maestro corre un ciclo normal? Son
    # dos autoridades decidiendo la misma luz, y el Degradado es el UNICO modo que da
    # verde sin confirmar la otra punta: si la respuesta fuera que si, esa orden no se
    # podria construir.
    #
    # LA RESPUESTA, MEDIDA EN EL C++ DE LAS DOS PUNTAS Y NO RAZONADA:
    #
    #   1. main.cpp del Esclavo llama a degradado_salir() al recibir CMD_PING, CMD_GO_RED
    #      o CMD_GO_GREEN. O sea que el modo NO se sostiene con el Maestro gobernando: el
    #      arbitraje ya estaba resuelto a favor del Maestro, y desde antes de A-11.
    #   2. El Maestro emite un latido cada LATIDO_MS mientras cicla (coordinador.cpp).
    #   3. El Degradado del Esclavo entra por un TODO-ROJO obligatorio de al menos
    #      ROJO_MINIMO_MS antes de su primer verde por reloj (modo_degradado.cpp).
    #
    # DE AHI SALE LA DESIGUALDAD QUE LO HACE SEGURO: ROJO_MINIMO_MS > LATIDO_MS. Con ella,
    # el latido del Maestro llega SIEMPRE durante el todo-rojo de entrada y saca a esta
    # punta antes de que pueda encender nada. Sin ella, un Esclavo puesto en Degradado por
    # error con el Maestro vivo daria un verde por reloj mientras el Maestro da el suyo
    # por ciclo, y las fases no tienen ninguna relacion: los dos sentidos al tramo.
    #
    # POR QUE VA EN UN PACK Y NO EN UN COMENTARIO (N-71): son dos constantes de DOS
    # FICHEROS DE DOS PUNTAS DISTINTAS que nada obliga a mirar juntas. El umbral de
    # silencio de SFTY-6 vivio 12 s contra un ciclo de 20,5 s -los reintentos 4 y 5 no
    # podian ejecutarse jamas- porque su relacion vivia en prosa, y los comentarios no
    # fallan cuando alguien cambia un numero: se quedan describiendo un equipo que ya no
    # existe, con la autoridad de una cuenta hecha.
    #
    # EL MARGEN SE PUBLICA, NO SOLO EL VEREDICTO: hoy es de 1 s sobre 3, y eso es fino.
    # Un veredicto binario dejaria que alguien subiera LATIDO_MS a 3900 y siguiera en
    # verde sin enterarse de que se comio el margen entero.
    LATIDO_MS = fw.constante(
        ("Maestro", "src", "coordinador.cpp"),
        r"LATIDO_MS\s*=\s*(\d+)\s*;",
        "el periodo del latido del Maestro (LATIDO_MS)")
    ROJO_MINIMO_MS = fw.constante(
        ("Esclavo", "src", "modo_degradado.cpp"),
        r"ROJO_MINIMO_MS\s*=\s*(\d+)",
        "el todo-rojo minimo de entrada al Degradado del Esclavo (ROJO_MINIMO_MS)")

    # El primer eslabon no es un numero y por eso se mide aparte: si main.cpp dejara de
    # salir del modo al oir al Maestro, la desigualdad de abajo seguiria cumpliendose y
    # no protegeria de nada. Se exige que las TRES tramas de gobierno esten en la guarda.
    principal = fw.codigo("Esclavo", "src", "main.cpp")
    m = re.search(r"degradado_gobiernaLuz\s*\(\s*\)\s*&&([^{]*)\{\s*degradado_salir",
                  principal)
    gobierno = set(re.findall(r"\bCMD_(PING|GO_RED|GO_GREEN)\b", m.group(1))) if m else set()
    b.propiedad(
        gobierno == {"PING", "GO_RED", "GO_GREEN"},
        "el Esclavo sale del Degradado en cuanto el Maestro vuelve a gobernar: la guarda "
        "de main.cpp llama a degradado_salir() con CMD_PING, CMD_GO_RED y CMD_GO_GREEN",
        "SFTY-21 ROTA: la salida por regreso del Maestro %s. Sin ella el Esclavo puede "
        "quedarse dando verdes por reloj mientras el Maestro corre su ciclo, y las dos "
        "fases no tienen ninguna relacion: son dos autoridades sobre la misma luz"
        % ("no se encuentra en main.cpp" if not m
           else "solo cubre %s y le faltan %s"
                % (sorted(gobierno) or "nada",
                   sorted({"PING", "GO_RED", "GO_GREEN"} - gobierno))))

    b.propiedad(
        ROJO_MINIMO_MS > LATIDO_MS,
        "el todo-rojo de entrada al Degradado (%d ms) es MAYOR que el latido del Maestro "
        "(%d ms): con el Maestro vivo, su latido saca a esta punta antes del primer verde "
        "por reloj. Margen %d ms"
        % (ROJO_MINIMO_MS, LATIDO_MS, ROJO_MINIMO_MS - LATIDO_MS),
        "SFTY-21 ROTA: el todo-rojo de entrada son %d ms y el latido del Maestro %d ms. "
        "Un Esclavo puesto en Degradado con el Maestro ciclando alcanza DEG_ACTIVO antes "
        "de que llegue el latido que lo saca, y da un verde por reloj mientras el Maestro "
        "da el suyo por ciclo. Las dos fases no tienen relacion: los dos sentidos entran "
        "al tramo" % (ROJO_MINIMO_MS, LATIDO_MS))

    b.reportar(
        "lo que esta desigualdad NO cubre, y no se disimula",
        ["se mide el peor caso de ROJO_MINIMO_MS; rojoObligatorioMs() devuelve el MAYOR "
         "de ese suelo y de config_despejeSegundos()*1000, asi que el margen real solo "
         "puede ser mayor",
         "NO cubre la perdida de latidos: si el radio pierde mas de %d ms seguidos de "
         "latido, esta punta puede alcanzar DEG_ACTIVO. La red que queda entonces es que "
         "el Maestro agote reintentos y caiga a C_FALLO, donde emite CMD_GO_RED -que "
         "tambien saca del modo-. No se ejerce aqui: esto lee texto y constantes, no "
         "corre una maquina de estados" % ROJO_MINIMO_MS,
         "NO cubre la orden con el Maestro vivo: el firmware la ACEPTA y el modo dura un "
         "latido. Lo que hace que eso se vea es el campo MODO: del $STATUS, que pasa a "
         "DEGRADADO y vuelve a SUBORDINADO en el tablero"])

    FALSO = '''
      if (strcmp(cmd, "CMD:AMBAR_EMERGENCIA") == 0) {
        semaforo_iniciarFallo();
        enviarTramaConCrc("$ACK,CMD:AMBAR_EMERGENCIA,RESULT:OK");
        return;
      }
      if (strcmp(accion, "AMBAR_EMERGENCIA") == 0) {
        if (degradado_gobiernaLuz()) { degradado_salir(); }
        else { semaforo_iniciarFallo(); }
      } else if (strcmp(accion, "OTRA_COSA") == 0) {
        reloj_ajustar();
      }
    '''
    ramas_falsas = _ramas(FALSO)
    puertas_falsas = [(c, cu) for c, cu in ramas_falsas if RE_LLAMA_AMBAR.search(cu)]

    # N-89: este pack lee POR TEXTO el bloque de cada rama, con el lector de
    # esclavo_07. Si aquel cambiara de forma -o si el despachador dejara de ser una
    # cadena de strcmp()- este control cae y avisa, en vez de dejarnos en verde
    # midiendo nada.
    b.control_negativo(
        len(puertas_falsas) == 2,
        "el lector de ramas importado de esclavo_07 sigue partiendo un despachador "
        "sintetico y hallando sus DOS puertas de ambar — si cambiara de forma, esto "
        "cae en vez de dejar el pack verde sin medir")
    b.control_negativo(
        sorted({c for c, cu in puertas_falsas
                if not RE_CONSULTA_GOB.search(cu)}) == ["AMBAR_EMERGENCIA"],
        "sobre dos puertas del mismo comando -una que consulta el Degradado y otra que "
        "no- el detector senala solo a la sorda: mide dentro del bloque de cada rama, "
        "no en el fichero entero")

    accs_falsas = {}
    for c, cu in puertas_falsas:
        accs_falsas.setdefault(c, []).append(_acciones(cu))
    b.control_negativo(
        any(len(v) > 1 and any(a != v[0] for a in v[1:]) for v in accs_falsas.values()),
        "el comparador de las dos puertas detecta que ejecutan acciones distintas — es "
        "lo unico que impide que el arreglo de N-106 entre por una sola de las dos")
