# ===== banco/packs/app_03_sin_ok_mudo.py =====
#
# NADIE CONTESTA "OK" SIN HABER MIRADO SI PUDO.
#
# LA PROPIEDAD, EN UNA LINEA: ninguna rama del despachador de Bluetooth que llame a
# una funcion CON VALOR DE RETORNO -o con una guarda que la hace rechazar en
# silencio- responde $ACK ... RESULT:OK sin haber mirado el resultado.
#
# EL CASO REAL, MEDIDO EL 28/08, QUE ES POR LO QUE ESTE PACK EXISTE:
#
#   bluetooth.cpp, rama SET_RTC:
#       reloj_ajustar((uint8_t)h, (uint8_t)mi, (uint8_t)s, (uint8_t)d);
#       coordinador_sincronizarHora();
#       enviarTramaConCrc("$ACK,CMD:SET_RTC,RESULT:OK");
#
#   reloj.cpp, primera linea util de reloj_ajustar():
#       if (!rtcOperativo) return;      // N-24: sin oscilador NO se acepta el ajuste
#
#   coordinador.cpp, primera linea util de coordinador_sincronizarHora():
#       if (!reloj_enHora()) return false;
#
# Las dos negativas son CORRECTAS y estan razonadas donde deben: aceptar la hora sin
# cristal dejaria una hora falsa que se cree buena, y sobre esa mentira el Maestro
# empujaria la hora al Esclavo y autorizaria el Modo Degradado. Lo que esta mal es
# lo de arriba: el despachador no mira NINGUNA de las dos y contesta OK igual. Con
# N-17 confirmado en hardware -el cristal Y2 que no oscila-, hoy ese comando dice
# que si y no hace nada, y el tecnico se va del poste creyendo que dejo el reloj
# puesto.
#
# Y NO ES UN CASO SUELTO. La misma rama SET_RTC del ESCLAVO tiene el defecto
# identico, y MANUAL:CAMBIAR_TURNO llama a coordinador_pedirCambio(), que empieza
# con `if (estadoC != C_IDLE) return;`: pedido a mitad de transicion se descarta y
# el telefono recibe RESULT:OK.
#
# LA REFERENCIA DE COMO SE HACE BIEN ESTA EN EL MISMO FICHERO, y este pack la mide
# para saber distinguir bien de mal en vez de solo saber acusar:
#
#   SET_TIEMPOS pregunta modoAutomatico_enMarcha() y !modoAutomatico_fijarTiempos()
#   dentro del `if`, y tiene un $ERR por cada motivo de rechazo.
#   SOLICITAR_PASO (Esclavo) hace `if (demanda_solicitar())` con su $ERR en el else,
#   y su comentario lo dice: "no se finge un envio que no ocurrio".
#
# COMO SE MIDE, SIN NINGUNA LISTA ESCRITA A MANO.
#
# La lista de funciones a vigilar NO se teclea: se deriva del C++ en cada corrida,
# por dos caminos que el propio fuente declara.
#
#   (a) DEVUELVE ALGO: su declaracion en <punta>/include/*.h no es `void`. Ignorar
#       ese valor es tirar la unica respuesta que la funcion sabe dar.
#   (b) RECHAZA EN SILENCIO: es `void`, pero su definicion en <punta>/src/*.cpp
#       tiene un `return;` con codigo detras -o sea, una guarda que aborta la
#       llamada a mitad-. Sin valor de retorno, el llamador no puede enterarse: la
#       unica defensa es que compruebe la precondicion el mismo.
#
# Un valor "consumido" es el que entra en un `if`, en una asignacion, en un `return`
# o como argumento. Uno "ignorado" es la llamada suelta como sentencia. Si una rama
# ignora un resultado Y ADEMAS emite $ACK, esta prometiendo algo que no comprobo.
#
# SOBRE LAS ETIQUETAS SFTY: este pack NO lleva ninguna. Roza SFTY-18 -el OK mudo de
# SET_RTC se apoya justo en la barrera que SFTY-18 monta- pero no EJERCE esa barrera:
# no comprueba reloj_enHora() ni el ano marcador, comprueba la forma del despachador.
# Figurar en la tabla de trazabilidad sin ejercer la regla es peor que una fila
# vacia, porque la vacia no miente.

import re

NOMBRE = "app_03_sin_ok_mudo"
DESCRIPCION = "ninguna rama del despachador contesta RESULT:OK sin mirar si la llamada pudo"

PUNTAS = ("Maestro", "Esclavo")

# La rama que se usa como PATRON DE LO BIEN HECHO. No es decorativa: si el detector
# la marcara mal, sus acusaciones no valdrian nada, y si desaparece del fuente este
# pack se queda sin calibrar y ABORTA en vez de seguir midiendo a ciegas.
REFERENCIA_BUENA = ("Maestro", "SET_TIEMPOS:")

# Declaracion de funcion a principio de linea en un .h. El tipo se captura entero
# -perezoso- para que "unsigned long foo()" y "const char* bar()" no se lean como si
# el nombre de la funcion fuera "long" o "char".
_DECL = re.compile(r"^[ \t]*([A-Za-z_][\w \t\*]*?)[ \t]+(\w+)[ \t]*\(", re.M)

# Las ramas del despachador: la cadena de strcmp/strncmp que ES el contrato. No hay
# tabla ni enum que leer, la cadena de comparaciones es todo lo que existe.
_RAMA = re.compile(r'\bstrn?cmp\s*\(\s*(?:accion|cmd)\s*,\s*"([^"]+)"')

# El prefijo del PIN no es una rama de comando: es el filtro de autenticacion.
_PIN = re.compile(r"^CMD:PIN:\d+:$")


def _bloque(texto, i):
    """El interior del bloque que abre en texto[i] == '{'. None si no cierra."""
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


def _declaradas(fw, punta):
    """{nombre: tipo de retorno} de todo lo que la punta declara en include/*.h."""
    decl = {}
    for h in fw.fuentes_de(punta, "include", ".h"):
        for m in _DECL.finditer(fw.codigo(punta, "include", h)):
            tipo = re.sub(r"\s+", " ", m.group(1)).strip()
            decl[m.group(2)] = tipo
    return decl


def _es_void(tipo):
    return tipo.replace("inline", "").replace(" ", "") == "void"


def _con_guarda(fw, punta, candidatas):
    """Las `void` que abandonan a mitad: un `return;` con codigo detras.

    Es la segunda mitad del censo y la menos evidente. reloj_ajustar() no devuelve
    nada, asi que "mirar el resultado" no es posible: el llamador tiene que preguntar
    la precondicion por su cuenta -reloj_hayCristal()- o dejar de prometer OK."""
    conGuarda = set()
    for c in fw.fuentes_de(punta, "src"):
        codigo = fw.codigo(punta, "src", c)
        for n in candidatas:
            for m in re.finditer(r"\bvoid\s+%s\s*\([^)]*\)\s*\{" % re.escape(n), codigo):
                cuerpo = _bloque(codigo, m.end() - 1)
                if cuerpo is None:
                    continue
                for r in re.finditer(r"\breturn\s*;", cuerpo):
                    # Un `return;` con codigo util detras es una salida temprana. El
                    # que cierra la funcion no lo es.
                    if re.search(r"[^\s};]", cuerpo[r.end():]):
                        conGuarda.add(n)
                        break
    return conGuarda


def _despachador(fw, punta):
    """El cuerpo de procesarComando() de esa punta."""
    codigo = fw.codigo(punta, "src", "bluetooth.cpp")
    m = re.search(r"\bprocesarComando\s*\([^)]*\)\s*\{", codigo)
    if not m:
        return None
    return _bloque(codigo, m.end() - 1)


def _ramas(cuerpo):
    """[(etiqueta, texto del bloque)] en el orden en que el firmware las compara."""
    fuera = []
    for m in _RAMA.finditer(cuerpo):
        etiqueta = m.group(1)
        if _PIN.match(etiqueta):
            continue
        i = cuerpo.find("{", m.end())
        if i < 0:
            continue
        bloque = _bloque(cuerpo, i)
        if bloque is None:
            continue
        fuera.append((etiqueta, bloque))
    return fuera


def _consumido(bloque, pos):
    """True si el resultado de la llamada que empieza en `pos` va a alguna parte.

    Se mira el ultimo caracter util ANTES del nombre. Si es ';', '{' o '}' la
    llamada es una sentencia suelta y lo que devuelva se pierde; si es '(', '=',
    ',' o un operador, alguien lo esta usando. El '!' se salta a proposito, que es
    justo como esta escrito el caso bien hecho: !modoAutomatico_fijarTiempos(...)."""
    i = pos - 1
    while i >= 0 and (bloque[i].isspace() or bloque[i] == "!"):
        i -= 1
    if i < 0:
        return False
    return bloque[i] not in ";{}"


def _ignorados(bloque, vigiladas):
    """Las llamadas vigiladas cuyo resultado esta rama tira a la basura."""
    fuera = []
    for n in sorted(vigiladas):
        for m in re.finditer(r"\b%s\s*\(" % re.escape(n), bloque):
            if not _consumido(bloque, m.start()):
                fuera.append(n)
                break
    return fuera


def _bloque_hacia_atras(texto, cierre):
    """El '{' que abre el '}' que hay en `cierre`. -1 si no cuadra."""
    prof = 0
    i = cierre
    while i >= 0:
        if texto[i] == "}":
            prof += 1
        elif texto[i] == "{":
            prof -= 1
            if prof == 0:
                return i
        i -= 1
    return -1


def _todos_los_lados_contestan(bloque):
    """True si el bloque parte en if/else y LOS DOS lados mandan una trama.

    QUE ARREGLA ESTO, Y POR QUE NO ES UNA EXCEPCION (CLAUDE.md 4.quinquies).

    La propiedad que este pack defiende no es "hay un $ERR": es QUE NINGUN CAMINO DEJE
    AL TELEFONO SIN RESPUESTA. El $ERR era el sustituto, y funcionaba mientras el unico
    motivo para mirar un resultado fuera decidir si se rechaza.

    Con N-146 aparecio el caso que el sustituto no sabe ver: SET_MODO:AMBAR mira
    modoActual_get() para saber si el equipo YA estaba en ambar -y entonces re-arma- o si
    entra ahora. Los dos finales son exitos y los dos se contestan, con literales
    distintos a proposito para que el diario de ordenes los pueda separar. No hay camino
    mudo, y sin embargo faltaba el $ERR.

    Cobrarle un $ERR a esa rama habria empujado a inventarse un rechazo que no existe, o
    -peor- a quitar la distincion entre los dos finales para que el detector callara.
    Ajustar el firmware hasta que el instrumento de verde es exactamente lo que este
    repositorio castiga, asi que se afila el instrumento: se mide la propiedad, no su
    sustituto.

    LO QUE NO SE AFLOJA: un solo lado que conteste no basta, y una rama sin else tampoco.
    Con eso, la rama que llama y calla en el camino malo -el defecto original- sigue
    cayendo igual. Lo comprueba el control negativo de este mismo pack."""
    for m in re.finditer(r"\}\s*else\s*\{", bloque):
        cierre = bloque.index("}", m.start())
        abre = _bloque_hacia_atras(bloque, cierre)
        if abre < 0:
            continue
        lado_if = bloque[abre:cierre + 1]
        j = bloque.index("{", m.start() + 1)
        lado_else = _bloque(bloque, j)
        if lado_else is None:
            continue
        contesta = lambda t: ('"$ACK' in t) or ('"$ERR' in t)
        if contesta(lado_if) and contesta(lado_else):
            return True
    return False


def _veredicto(bloque, vigiladas):
    """(hay_algo_que_verificar, motivo_de_fallo o None)."""
    llamadas = [n for n in sorted(vigiladas)
                if re.search(r"\b%s\s*\(" % re.escape(n), bloque)]
    if not llamadas:
        return False, None
    if '"$ACK' not in bloque:
        # Una rama que no promete nada no puede mentir. El rechazo seco de TEST_LEDS
        # en el Esclavo es exactamente eso y esta bien asi.
        return True, None
    ign = _ignorados(bloque, vigiladas)
    if ign:
        return True, ("llama a %s y TIRA lo que devuelve -o no comprueba su guarda- "
                      "y aun asi manda $ACK" % ", ".join("%s()" % n for n in ign))
    if '"$ERR' not in bloque and not _todos_los_lados_contestan(bloque):
        return True, ("mira el resultado de %s y deja un camino SIN RESPUESTA: ni hay "
                      "$ERR ni los dos lados del if/else contestan, asi que el telefono "
                      "se queda esperando o da por bueno el $ACK anterior"
                      % ", ".join("%s()" % n for n in llamadas))
    return True, None


def correr(b, fw):
    b.titulo("Sin OK mudo: quien promete RESULT:OK ha tenido que mirar")

    vigiladas = {}
    ramas = {}
    for p in PUNTAS:
        decl = _declaradas(fw, p)
        if not decl:
            raise fw.Abortado(
                "%s: no se leyo ni una declaracion de funcion en include/*.h. La "
                "lista de funciones a vigilar sale de ahi y solo de ahi; con la lista "
                "vacia este pack aprobaria cualquier despachador sin haber mirado una "
                "sola llamada" % p)
        devuelven = {n for n, t in decl.items() if not _es_void(t)}
        conGuarda = _con_guarda(fw, p, {n for n, t in decl.items() if _es_void(t)})
        vigiladas[p] = devuelven | conGuarda

        cuerpo = _despachador(fw, p)
        if cuerpo is None:
            raise fw.Abortado(
                "%s: no se hallo procesarComando() en bluetooth.cpp. Es el unico "
                "sitio donde vive el contrato de comandos: sin el este pack no tiene "
                "nada que medir" % p)
        ramas[p] = _ramas(cuerpo)
        if not ramas[p]:
            raise fw.Abortado(
                "%s: procesarComando() no dio ni una rama strcmp(accion|cmd, \"...\"). "
                "O el despachador dejo de ser una cadena de comparaciones o el "
                "buscador se quedo atras; medir cero ramas saldria en verde" % p)

        b.verificar(
            bool(devuelven) and bool(conGuarda),
            "%s: censo leido del C++ -%d funciones que devuelven algo y %d `void` "
            "que abandonan a mitad (%s)-, sobre %d ramas del despachador"
            % (p, len(devuelven), len(conGuarda), ", ".join(sorted(conGuarda)[:4]) +
               ("..." if len(conGuarda) > 4 else ""), len(ramas[p])),
            "%s: el censo salio a medias -%d con retorno, %d con guarda-. Una de las "
            "dos mitades no se esta leyendo, y la que falta deja pasar sin mirar "
            "todas sus llamadas" % (p, len(devuelven), len(conGuarda)))

    # ---- La calibracion: la rama que se hace BIEN tiene que salir bien ----
    puntaRef, etiquetaRef = REFERENCIA_BUENA
    bloqueRef = dict(ramas[puntaRef]).get(etiquetaRef)
    if bloqueRef is None:
        raise fw.Abortado(
            "no esta la rama %s del %s, que es el patron con el que este pack sabe "
            "distinguir una respuesta honesta de un OK mudo. Sin ella el detector se "
            "queda sin calibrar y sus acusaciones no se pueden creer"
            % (etiquetaRef, puntaRef))
    tieneQueVerificar, motivoRef = _veredicto(bloqueRef, vigiladas[puntaRef])
    b.verificar(
        tieneQueVerificar and motivoRef is None,
        "calibrado contra %s del %s, que es como se hace bien: pregunta dentro del "
        "`if` y tiene un $ERR por cada motivo de rechazo" % (etiquetaRef, puntaRef),
        "el detector marca como defectuosa la rama %s del %s, que es la que esta "
        "BIEN hecha (%s). Con la referencia buena en rojo, ningun otro veredicto de "
        "este pack vale nada: se arregla el pack" % (etiquetaRef, puntaRef, motivoRef))

    # ---- Una comprobacion por rama que tenga algo que prometer ----
    for p in PUNTAS:
        for etiqueta, bloque in ramas[p]:
            tiene, motivo = _veredicto(bloque, vigiladas[p])
            if not tiene:
                continue
            b.verificar(
                motivo is None,
                "%s / %s: la respuesta que manda depende de lo que la llamada "
                "contesto" % (p, etiqueta),
                "%s / %s: %s. El tecnico recibe una confirmacion de algo que puede "
                "no haber ocurrido, se va del poste, y el equipo se queda como "
                "estaba sin que nada lo diga" % (p, etiqueta, motivo))

    # ---- Controles negativos: el detector tiene que saber fallar ----
    #
    # Se ejerce contra dos bloques sinteticos construidos con las MISMAS funciones
    # reales, uno con el defecto y otro sin el. Si el detector aprobara el primero,
    # todos los OK de arriba serian decoracion.
    vig = vigiladas["Maestro"] | vigiladas["Esclavo"]
    malo = ('{ reloj_ajustar(1, 2, 3, 4); '
            'enviarTramaConCrc("$ACK,CMD:SET_RTC,RESULT:OK"); }')
    bueno = ('{ if (demanda_solicitar()) { '
             'enviarTramaConCrc("$ACK,CMD:X,RESULT:OK"); } else { '
             'enviarTramaConCrc("$ERR,CMD:X,DESC:NO_PUDO"); } }')
    b.control_negativo(
        _veredicto(malo, vig)[1] is not None,
        "una rama que llama a reloj_ajustar() y contesta $ACK sin mirar nada se "
        "detecta como OK mudo")
    b.control_negativo(
        _veredicto(bueno, vig) == (True, None),
        "una rama que pregunta `if (demanda_solicitar())` y tiene su $ERR en el else "
        "NO se marca: el detector distingue, no acusa a todo el que llama")
