# ===== banco/packs/esp32_05_no_origina.py =====
#
# EL PUENTE NO ORIGINA. Cada byte que sale hacia el STM32 vino del buffer de entrada.
#
# POR QUE ESTO ES LA PROPIEDAD MAS IMPORTANTE DEL PUENTE.
#
# Al otro lado de J17 hay un micro que gobierna un cruce. Un literal de comando en el
# fuente del puente es el accesorio mandando ordenes por su cuenta a ese micro, y el
# STM32 no tiene forma de distinguirlas de las que manda el operario: NO VALIDA el
# checksum de entrada -procesarComando() arranca directo con strcmp- y no sabe quien
# habla. La puerta es de una sola direccion y el unico que puede cerrarla es este lado.
#
# LA LISTA BLANCA SE ESCRIBE A MANO, Y ESO ES DELIBERADO.
#
# Es el molde de esclavo_06_no_abre_paso, con su decision de metodo copiada literal:
#
#   "La lista blanca se escribe a mano, y eso es deliberado. [...] si el pack leyera los
#    comandos del propio fuente, un comando nuevo se aprobaria a si mismo."
#
# Igual aqui: cualquiera que anada un literal que el puente emita pasa por este fichero
# y justifica por que no origina nada hacia el equipo.
#
# LA ARQUITECTURA QUE HACE ESTO COMPROBABLE EN VEZ DE CONFIABLE.
#
# Una sola puerta: enlace_stm32.cpp es el UNICO fichero que nombra el puerto serie, y su
# unica funcion de escritura recibe un puntero y una longitud -no hay version que reciba
# un literal-. Es la misma forma que la barrera de salidas del STM32, donde solo
# semaforo.cpp escribe pines de luz, y por la misma razon: una regla que hay que
# respetar en N sitios se rompe en el sitio N+1.
#
# Y B-4: lo que el puente SI emite -hacia la app, nunca hacia el equipo- va marcado con
# NODE:PUENTE. Un $ERR del puente que pareciera del STM32 manda a diagnosticar el poste
# equivocado: el tecnico buscaria en el firmware del semaforo un rechazo que genero el
# accesorio.
#
# =====================================================================================
# 🔴 11/09 - D-20 / A-15: LA BARRERA NO SE RETIRA, SE ESTRECHA A EXCEPCIONES CON NOMBRE.
# =====================================================================================
#
# "Son los ESP32 los que tienen el reloj y deben comandar la hora" (el responsable,
# 11/09). O sea que desde hoy el ESP32 ORIGINA una orden hacia el STM32:
# CMD:HORA_ESP32:YYYY-MM-DD,HH:MM:SS, compuesta en siembra.cpp. La frase de arriba
# -"cada byte que sale hacia el STM32 vino del buffer de entrada"- deja de ser cierta, y
# este pack no puede seguir diciendola.
#
# 🔴 Y AL IR A ESTRECHARLA SALIO QUE YA NO ERA CIERTA DESDE EL 04/09, Y ESTE PACK NO LO
# VEIA. vigilante.cpp manda "$LATIDO" al STM32 cada 2 s (N-127) por
# enlace_escribirLinea(LINEA, ...). Las siete comprobaciones de antes pasaban igual:
#   - la 3 solo buscaba un literal DENTRO de la llamada, y LINEA es una variable;
#   - la 4 solo miraba la PRIMERA llamada de puente.cpp;
#   - la 5 solo censaba literales que empiezan por '$' EN src/, y "$LATIDO" vive en
#     include/contrato.h. Y una orden al STM32 empieza por "CMD:", no por '$': un
#     "CMD:FORZAR_ROJO" metido en un buffer habria pasado las siete sin despeinarlas.
# El latido esta decidido (04/09) y lo acota esp32_10 por su lado; lo que faltaba es que
# LA BARRERA lo supiera. Una regla que enumera sujetos tiene que censarlos (CLAUDE.md 2).
#
# REVISION UNA POR UNA (CLAUDE.md 9), con lo que afirmaba cada una:
#   1 una sola puerta         -> SE CONSERVA: sigue siendo cierta y sigue importando.
#   2 la puerta sin literales -> SE CONSERVA y se ENSANCHA a "CMD:": la puerta tampoco
#                                puede llevar una orden, empiece por lo que empiece.
#   3 nunca con literal       -> SE CONSERVA: afirma una propiedad de forma que sigue en
#                                pie (el literal va en un buffer con nombre, no en la
#                                llamada) -y por eso no bastaba: la 4.bis es la que censa-.
#   4 puente.cpp manda deApp  -> SE CONSERVA: el sentido de ida sigue siendo verbatim.
#   4.bis (NUEVA) -> CADA llamada a enlace_escribirLinea() del proyecto, con el buffer
#                    que lleva, contra una lista escrita a mano de TRES: deApp, el
#                    latido y la siembra. Una cuarta, FALLA. Es la que el pack no tenia.
#   4.ter (NUEVA) -> las dos excepciones llevan lo que su motivo dice: el latido es
#                    LATIDO_LINEA y nada mas; la siembra es UN snprintf con el formato
#                    revisado y SOLO campos de una FechaHora que rellena reloj_leer() con
#                    su resultado mirado antes. Es la excepcion medida, no escrita (§6).
#   5 literales revisados     -> SE CONSERVA y se ENSANCHA: tambien "CMD:..." y tambien
#                                include/. Ahi aparecen "$LATIDO", el formato de la
#                                siembra y CMD:LEER_RTC (que compara, no emite).
#   6 B-4 NODE:PUENTE         -> SE CONSERVA, sobre EMITE_HACIA_LA_APP. Las dos lineas
#                                hacia el STM32 no llevan NODE: van a un micro, no a un
#                                operario.
#   7 no compone $STATUS      -> SE CONSERVA.
# Ninguna se invierte y ninguna se borra: lo que cambia es que las excepciones pasan de
# no existir para el pack a estar escritas y MEDIDAS aqui, con controles que demuestran
# que una tercera orden hacia el STM32 -la que sea- se caza.

import re

NOMBRE = "esp32_05_no_origina"
DESCRIPCION = "ni un literal de comando sale hacia el STM32: todo procede del buffer de entrada"

ROL = "ESP32_Expansion"

# EL PUERTO. Nombrarlo fuera de su fichero es abrir una segunda puerta.
PUERTO = r"\bSerial2\b|\bhaciaSTM32\b"
FICHERO_DE_LA_PUERTA = "enlace_stm32.cpp"

# LO QUE EL PUENTE TIENE DERECHO A EMITIR, HACIA LA APP Y SOLO HACIA LA APP.
# Anadir una entrada aqui exige justificar arriba por que no es originar hacia el equipo.
EMITE_HACIA_LA_APP = {
    "$ERR,NODE:PUENTE,CMD:SET_RTC,DESC:FORMATO_INVALIDO":
        "la fecha que mando la app no se deja leer o esta fuera de rango",
    "$ERR,NODE:PUENTE,CMD:SET_RTC,DESC:SIN_RELOJ_NO_RESPONDE":
        "el bus I2C del DS3231 no contesta: modulo ausente o SDA/SCL cruzados",
    "$ERR,NODE:PUENTE,CMD:SET_RTC,DESC:ESCRITURA_FALLIDA":
        "la escritura I2C fallo a mitad",
    "$ERR,NODE:PUENTE,CMD:SET_RTC,DESC:NO_QUEDO_PUESTA":
        "se escribio y la relectura no coincide: R-8",
    "$ERR,NODE:PUENTE,CMD:SET_RTC,DESC:OSCILADOR_PARADO_CAMBIE_PILA":
        "la hora entro y el oscilador no arranco: el OSF sigue puesto",
    "$ERR,NODE:PUENTE,CMD:SET_RTC,DESC:MOTIVO_NO_CONTEMPLADO":
        "un valor nuevo del enum sin rama: se contesta en vez de aprobarse solo",
    "$ERR,NODE:PUENTE,CMD:DESCONOCIDO,DESC:LINEA_DEMASIADO_LARGA":
        "E-2 por nuestro lado: el STM32 truncaria en silencio, aqui se dice",
    # D-20 (11/09): los dos $ACK de SET_RTC hablan ahora de la SIEMBRA, no de la linea
    # del telefono -que ya no cruza-. Mismo literal, otro hecho detras; el hecho lo mide
    # esp32_13 (que el $ACK dependa de lo que devolvio siembra_ahora()).
    "$ACK,NODE:PUENTE,CMD:SET_RTC,RESULT:HORA_PUESTA_SIN_PROPAGAR,":
        "motivo 7: la hora entro en el DS3231 y la linea CMD:HORA_ESP32 no salio entera "
        "hacia el equipo",
    "$ACK,NODE:PUENTE,CMD:SET_RTC,RESULT:OK,":
        "la hora entro, se releyo, y la linea CMD:HORA_ESP32 salio entera hacia el equipo",
    "$ERR,NODE:PUENTE,CMD:HORA_ESP32,DESC:LINEA_RESERVADA_AL_PUENTE":
        "D-20 anti-suplantacion: una linea del telefono con HORA_ESP32 dentro no cruza y "
        "se dice. Va a la APP; lo que NO sale hacia el STM32 es justo esa linea",
    "$ERR,NODE:PUENTE,CMD:DESCONOCIDO,DESC:RECLAMADA_SIN_RAMA":
        "el predicado se quedo una linea que ninguna rama reconoce: se contesta en vez de "
        "dejarla desaparecer. Hoy inalcanzable; es la rama del criterio que alguien anada "
        "sin escribirle respuesta",
    # ---------------------------------------------------------------------------
    # A-9 (05/09) - LA CONSULTA DE RELOJ. NUEVE LITERALES, Y EL MOTIVO ES EL MISMO
    # PARA LOS NUEVE, MEDIDO Y NO REDACTADO.
    #
    # Los tres hechos que hay que comprobar de cada uno -y que se comprueban en los
    # apartados 2 a 5 de este mismo pack, no aqui-:
    #   1. Salen por puente_emitirPropio(), o sea hacia la APP. Ninguno pasa por
    #      enlace_escribirLinea(), que es la unica puerta hacia el STM32.
    #   2. Llevan NODE:PUENTE, asi que el operario no diagnostica el poste equivocado.
    #   3. NO ORDENAN NADA: LEER_RTC es una consulta de solo lectura. No escribe el
    #      reloj -no hay una sola llamada a reloj_ajustar() en su rama-, no toca una luz
    #      y no cambia un modo. Es el unico comando del puente del que se puede decir
    #      que el equipo queda EXACTAMENTE igual que antes de mandarlo.
    #
    # Y hay una cuarta cosa que este pack NO puede ver y que por eso se mide en
    # esp32_12_consulta_de_reloj: que los ocho $ERR cubran uno a uno los valores del
    # enum MotivoSinHora. Un motivo sin rama dejaria la consulta SIN CONTESTAR, y una
    # consulta muda se lee como equipo colgado.
    # ---------------------------------------------------------------------------
    "$ACK,NODE:PUENTE,CMD:LEER_RTC,RESULT:OK,":
        "la consulta: la hora releida del chip en este instante. No escribe nada",
    "$ERR,NODE:PUENTE,CMD:LEER_RTC,DESC:NUNCA_SE_PUSO_PONGA_LA_HORA":
        "SIN_HORA_NUNCA_SE_PUSO: modulo virgen. Se arregla con un SET_RTC",
    "$ERR,NODE:PUENTE,CMD:LEER_RTC,DESC:OSCILADOR_PARADO_CAMBIE_PILA":
        "SIN_HORA_OSCILADOR_PARADO: OSF==1. Mismo literal que SET_RTC porque es el "
        "mismo arreglo -la pila-, y dos textos para una averia se leen como dos averias",
    "$ERR,NODE:PUENTE,CMD:LEER_RTC,DESC:SIN_RELOJ_NO_RESPONDE":
        "SIN_HORA_BUS_MUDO: el I2C no contesta. Mismo literal que SET_RTC por lo mismo",
    "$ERR,NODE:PUENTE,CMD:LEER_RTC,DESC:ESCRITURA_A_MEDIAS_REPITA_SET_RTC":
        "SIN_HORA_ESCRITURA_A_MEDIAS: la duda se pega y solo la levanta un SET_RTC entero",
    "$ERR,NODE:PUENTE,CMD:LEER_RTC,DESC:MODO_12H_PONGA_LA_HORA":
        "SIN_HORA_FORMATO_12H: hasta 12 h de error con el oscilador sano. No se le "
        "reescribe el bit al chip -seria cambiarle la hora a un equipo de la calle-",
    "$ERR,NODE:PUENTE,CMD:LEER_RTC,DESC:REGISTROS_INCOHERENTES":
        "SIN_HORA_REGISTROS_INCOHERENTES: es el unico DESC que NO nombra el arreglo, "
        "porque hay dos posibles -repetir o cambiar el modulo- y el firmware no los "
        "distingue. Nombrar uno seria elegir la reparacion a cara o cruz",
    "$ERR,NODE:PUENTE,CMD:LEER_RTC,DESC:BARRERA_INCOHERENTE":
        "SIN_HORA_NINGUNO con reloj_leer() en false: la barrera se contradice. Es un "
        "defecto del firmware del puente, no del reloj, y va nombrado distinto para que "
        "no mande a nadie a cambiar una pila sana",
    "$ERR,NODE:PUENTE,CMD:LEER_RTC,DESC:MOTIVO_NO_CONTEMPLADO":
        "un valor nuevo del enum sin rama: se contesta en vez de dejar la consulta muda",

    "$EVENT,NODE:PUENTE,EVT:ARRANQUE,CAUSA:%s,ARRANQUES:%lu,PERRO:%s,WDT_MS:%lu":
        "el parte de arranque: por que arranco el puente y cuantas veces lleva "
        "arrancando. Revisado a mano el 01/09 y aprobado por tres cosas: va a la APP "
        "y no hacia el STM32 -sale por puente_emitirPropio()-, lleva NODE:PUENTE asi "
        "que el operario ve de quien es, y no ORIGINA nada: cuenta un hecho ya "
        "ocurrido, no pide ni ordena. Existe porque un puente que revive en silencio "
        "esconde el fallo que hay que contar",
}

# Literales con '$' que NO son emisiones: reconocimiento y censo. Se listan aparte
# porque confundirlos con emisiones seria acusar a un strncmp de originar tramas.
NO_SON_EMISIONES = {
    "$STATUS,": "strncmp de reconocimiento: de ahi se aprende el rotulo del equipo",
    "$STATUS": "censo PREFIJOS_STM32: contadores de diagnostico, NO un filtro",
    "$ACK": "censo PREFIJOS_STM32",
    "$ERR": "censo PREFIJOS_STM32",
    "$ALARM": "censo PREFIJOS_STM32",
    "$EVENT": "censo PREFIJOS_STM32",
    # 11/09: entra al censar tambien "CMD:...". Es la constante con la que el predicado
    # del despachador COMPARA la linea que llega; no se escribe en ningun cable.
    "CMD:LEER_RTC": "constante de COMPARACION de despachador_esParaElPuente(): la "
                    "consulta que el puente se queda. No sale por ningun sitio",
}

# 🔴 LO QUE EL ESP32 ORIGINA HACIA EL STM32. DOS, CON SU MOTIVO, Y NINGUNO MAS.
#
# Anadir una entrada aqui es anadir una orden que el accesorio manda por su cuenta a un
# micro que gobierna un cruce y que no valida quien habla: exige una decision escrita
# en DECISIONES.md, no un comentario. Cada una tiene ademas su contenido medido en la
# 4.ter, porque un motivo sin medir es un defecto con permiso (CLAUDE.md 6).
EMITE_HACIA_EL_STM32 = {
    "$LATIDO":
        "N-127 (04/09, decision del responsable): el latido. Las dos puntas lo reconocen "
        "ANTES de la guarda de PIN y devuelven sin actuar y sin contestar -lo mide "
        "costura_12-; su unico efecto es cerrar un silencio de J17. Vive en contrato.h "
        "(LATIDO_LINEA) y lo manda vigilante.cpp",
    "CMD:HORA_ESP32:%04d-%02d-%02d,%02d:%02d:%02d":
        "D-20/A-15 (07-08/09, construida el 11/09): la hora del DS3231 hacia su STM32, "
        "que la SOBREESCRIBE. La unica excepcion de ORDEN: la compone siembra.cpp SOLO "
        "con campos de reloj_leer() -la barrera del DS3231 con su releida-, y ni un byte "
        "del telefono, que ademas no puede mandarla (el despachador la descarta)",
}

# LAS UNICAS ESCRITURAS HACIA EL STM32 DEL PROYECTO: (fichero, buffer que se escribe).
#
# Se escribe a mano por lo mismo que la lista blanca: si se leyera del fuente, una
# escritura nueva se aprobaria a si misma. Hasta el 11/09 este pack solo miraba la
# primera de puente.cpp, y el latido lleva saliendo desde el 04/09 por otra.
ESCRITURAS_PERMITIDAS = {
    ("puente.cpp", "deApp"):
        "B-1: el buffer de entrada de la app, verbatim (comprobacion 4)",
    ("vigilante.cpp", "LINEA"):
        "N-127: el latido; su contenido se mide en la 4.ter",
    ("siembra.cpp", "linea"):
        "D-20: la hora releida del DS3231; su contenido se mide en la 4.ter",
}

# Donde vive el formato de la siembra y como se llama. Se nombra porque la 4.ter tiene
# que encontrar EL snprintf que llena el buffer, y comprobar que usa ESTE formato.
SIEMBRA = "siembra.cpp"
FORMATO_SIEMBRA = "FORMATO_HORA_ESP32"
LATIDO_DEF = ("ESP32_Expansion", "include", "contrato.h")


def _partir_args(texto):
    """Parte los argumentos de una llamada por las comas de nivel 0."""
    args, prof, actual = [], 0, ""
    for ch in texto:
        if ch in "([{":
            prof += 1
        elif ch in ")]}":
            prof -= 1
        if ch == "," and prof == 0:
            args.append(actual.strip())
            actual = ""
        else:
            actual += ch
    if actual.strip():
        args.append(actual.strip())
    return args


def _args_de_llamada(codigo, pos_parentesis):
    """El texto entre el '(' que esta en pos_parentesis y su ')' pareja."""
    prof = 0
    for j in range(pos_parentesis, len(codigo)):
        if codigo[j] == "(":
            prof += 1
        elif codigo[j] == ")":
            prof -= 1
            if prof == 0:
                return codigo[pos_parentesis + 1:j]
    return None


def _escrituras_hacia_el_stm32(codigos):
    """[(fichero, primer argumento)] de CADA llamada a enlace_escribirLinea().

    Se salta la definicion y la declaracion -llevan el tipo de retorno delante-, que no
    escriben nada. Todo lo demas cuenta, lleve lo que lleve: un literal, un buffer, una
    expresion. Si el primer argumento no es un identificador, sale tal cual y no casara
    con ninguna entrada de la lista, que es lo que tiene que pasar."""
    fuera = []
    for f, cod in sorted(codigos.items()):
        for m in re.finditer(r"\benlace_escribirLinea\s*\(", cod):
            if re.search(r"\b(?:size_t|void|int|bool)\s*$", cod[:m.start()]):
                continue
            dentro = _args_de_llamada(cod, m.end() - 1)
            args = _partir_args(dentro) if dentro is not None else []
            fuera.append((f, args[0] if args else "?"))
    return fuera


def _funcion_que_contiene(codigo, pos):
    """Desde donde empieza la definicion de funcion que envuelve a pos."""
    ini = 0
    for m in re.finditer(r"\n[A-Za-z_][\w\s\*]*?\b\w+\s*\([^;{}]*\)\s*\{", codigo):
        if m.start() < pos:
            ini = m.start()
        else:
            break
    return ini


def _siembra_honesta(codigo, buf, formato, formatos):
    """La siembra se compone SOLO de reloj_leer(). Devuelve (ok, motivo).

    Lo que tiene que ser verdad, cada cosa por separado porque cada una es un camino
    distinto de meter una hora que no dio la barrera:
      a) al buffer se escribe UNA vez y con snprintf -ni strcpy, ni memcpy, ni un
         buf[i] = ..., que meterian bytes de otro sitio-;
      b) con ESTE formato, y el literal del formato es el revisado;
      c) los campos son TODOS de una misma variable, y son campos -x.anio-, no
         literales ni otras variables;
      d) esa variable es una FechaHora y, antes del snprintf y en la misma funcion,
         `if (!reloj_leer(&x)) return ...`: si la barrera dice que no, no se llega."""
    escrituras = list(re.finditer(
        r"\b(snprintf|sprintf|strcpy|strncpy|strcat|strncat|memcpy|memmove)\s*\(\s*%s\b"
        % re.escape(buf), codigo))
    indices = re.findall(r"\b%s\s*\[[^\]]*\]\s*=(?!=)" % re.escape(buf), codigo)
    if len(escrituras) != 1 or indices:
        return False, ("al buffer %r se escribe %d vez/veces por funcion y %d por indice: "
                       "tiene que ser UN snprintf y nada mas" % (buf, len(escrituras),
                                                                  len(indices)))
    m = escrituras[0]
    if m.group(1) != "snprintf":
        return False, "al buffer %r se escribe con %s(), no con snprintf" % (buf, m.group(1))
    dentro = _args_de_llamada(codigo, codigo.find("(", m.start()))
    args = _partir_args(dentro or "")
    if len(args) < 4 or args[2] != formato:
        return False, "el snprintf de %r no usa %s: usa %r" % (buf, formato,
                                                             args[2] if len(args) > 2 else "?")
    lit = formatos.get(formato)
    if lit not in EMITE_HACIA_EL_STM32:
        return False, "el literal de %s (%r) no es el revisado a mano" % (formato, lit)
    campos = args[3:]
    variables = {c.split(".")[0] for c in campos if re.match(r"^[A-Za-z_]\w*\.\w+$", c)}
    if len(variables) != 1 or not all(re.match(r"^[A-Za-z_]\w*\.\w+$", c) for c in campos):
        return False, ("los campos del snprintf no son todos campos de UNA variable: %s. "
                       "Una hora compuesta con literales o con otra cosa es una hora que "
                       "no dio la barrera" % campos)
    x = variables.pop()
    antes = codigo[_funcion_que_contiene(codigo, m.start()):m.start()]
    if not re.search(r"\bFechaHora\s+%s\s*;" % re.escape(x), antes):
        return False, "%s no se declara FechaHora en la funcion de la siembra" % x
    if not re.search(r"if\s*\(\s*!\s*reloj_leer\s*\(\s*&\s*%s\s*\)\s*\)\s*return\b"
                     % re.escape(x), antes):
        return False, ("%s no pasa por `if (!reloj_leer(&%s)) return` antes del snprintf: "
                       "sin eso la hora puede salir de un buffer sin rellenar, con formato "
                       "perfecto -N-144-" % (x, x))
    return True, "UN snprintf(%s, ..., %s, %s.*) detras de if (!reloj_leer(&%s)) return" % (
        buf, formato, x, x)


def correr(b, fw):
    b.titulo("El puente no origina: una sola puerta y ni un literal hacia el equipo")

    fuentes = fw.fuentes_de("ESP32_Expansion", "src")
    if len(fuentes) < 4:
        raise fw.Abortado(
            "solo se censaron %d fuentes en %s/src. El censo del directorio es lo que "
            "hace que un fichero nuevo entre bajo vigilancia solo; con la lista corta, "
            "una segunda puerta abierta en un .cpp nuevo no la veria nadie"
            % (len(fuentes), ROL))

    # ---- 1. UNA SOLA PUERTA: nadie mas nombra el puerto -----------------------
    intrusos = [f for f in fuentes
                if f != FICHERO_DE_LA_PUERTA and re.search(PUERTO, fw.codigo("ESP32_Expansion", "src", f))]
    b.verificar(
        not intrusos,
        "solo %s nombra el puerto hacia el STM32; los otros %d fuentes no lo tocan"
        % (FICHERO_DE_LA_PUERTA, len(fuentes) - 1),
        "HAY UNA SEGUNDA PUERTA hacia el STM32 en %s. Con mas de un sitio que escriba en "
        "el puerto, 'el puente no origina' deja de poder comprobarse leyendo un fichero "
        "corto y pasa a depender de que todos se acuerden" % ", ".join(intrusos))

    puerta = fw.codigo("ESP32_Expansion", "src", FICHERO_DE_LA_PUERTA)

    # ---- 2. La puerta no tiene NI UN literal de trama NI DE ORDEN -------------
    # 11/09: tambien "CMD:". Una orden al STM32 empieza por ahi, no por '$'.
    literales = re.findall(r'"(\$[^"]*|CMD:[^"]*)"', puerta)
    b.verificar(
        not literales,
        "%s no contiene ni un literal de trama: lo unico que sabe escribir es el buffer "
        "que le pasan" % FICHERO_DE_LA_PUERTA,
        "HAY LITERALES DE TRAMA EN LA PUERTA: %s. Un literal aqui es el accesorio "
        "mandando una orden por su cuenta a un micro que gobierna un cruce, y el STM32 "
        "no tiene con que distinguirla de una del operario: no valida el checksum de "
        "entrada" % literales)

    # ---- 3. La funcion de escritura no se llama NUNCA con un literal ----------
    conLiteral = []
    for f in fuentes:
        codigo = fw.codigo("ESP32_Expansion", "src", f)
        for m in re.finditer(r'enlace_escribirLinea\s*\(\s*"', codigo):
            conLiteral.append("%s: %s" % (f, codigo[m.start():m.start() + 50]))

    llamadas = sum(len(re.findall(r"enlace_escribirLinea\s*\(", fw.codigo("ESP32_Expansion", "src", f)))
                   for f in fuentes)
    b.verificar(
        llamadas >= 2 and not conLiteral,
        "las %d apariciones de enlace_escribirLinea() reciben buffers, nunca literales"
        % llamadas,
        "enlace_escribirLinea() SE LLAMA CON UN LITERAL: %s. Eso es exactamente originar"
        % "; ".join(conLiteral) if conLiteral
        else "solo se hallaron %d apariciones de enlace_escribirLinea(): o el fuente "
             "cambio de forma o el buscador se quedo ciego, y medir cero llamadas "
             "saldria en verde" % llamadas)

    # ---- 4. Lo que sale hacia el STM32 procede del buffer de entrada ----------
    puente = fw.codigo("ESP32_Expansion", "src", "puente.cpp")
    m = re.search(r"enlace_escribirLinea\s*\(\s*([A-Za-z_]\w*)", puente)
    b.verificar(
        m is not None and m.group(1) == "deApp",
        "lo que se entrega al STM32 es el buffer de entrada de la app (%s), no algo "
        "compuesto aqui" % (m.group(1) if m else "?"),
        "lo que se escribe hacia el STM32 no sale del buffer de entrada: es %s. B-1 dice "
        "CADA BYTE, y un intermedio compuesto por el puente puede diferir de lo que el "
        "operario mando sin que nadie lo vea" % (m.group(1) if m else "un literal"))

    # ---- 4.bis 🔴 CADA escritura hacia el STM32, contra la lista de TRES ------
    #
    # LA COMPROBACION QUE ESTE PACK NO TENIA. La 4 mira la primera llamada de puente.cpp
    # y nada mas; el latido salia por vigilante.cpp desde el 04/09 sin que nadie aqui lo
    # viera. Ahora se censan TODAS, en todos los .cpp de src/, con el buffer que llevan.
    codigos = {f: fw.codigo("ESP32_Expansion", "src", f) for f in fuentes}
    escrituras = _escrituras_hacia_el_stm32(codigos)
    vistas = {}
    for e in escrituras:
        vistas[e] = vistas.get(e, 0) + 1
    ajenas = sorted(e for e in vistas if e not in ESCRITURAS_PERMITIDAS)
    repetidas = sorted(e for e, n in vistas.items() if n > 1)
    faltan = sorted(e for e in ESCRITURAS_PERMITIDAS if e not in vistas)
    b.verificar(
        not ajenas and not repetidas and not faltan,
        "las %d escrituras hacia el STM32 del proyecto son exactamente las %d revisadas a "
        "mano: %s" % (len(escrituras), len(ESCRITURAS_PERMITIDAS),
                      ", ".join("%s(%s)" % e for e in sorted(vistas))),
        "LAS ESCRITURAS HACIA EL STM32 NO SON LAS REVISADAS. Ajenas: %s · repetidas: %s · "
        "revisadas que ya no aparecen: %s. Una escritura nueva hacia el micro que gobierna "
        "el cruce es el accesorio mandando por su cuenta, y el STM32 no la distingue de "
        "una del operario; una que desaparece deja un motivo escrito para nada"
        % (ajenas or "-", repetidas or "-", faltan or "-"))

    # ---- 4.ter Las dos excepciones llevan lo que su motivo dice ----------------
    #
    # 🔴 LA EXCEPCION ES EL INSTRUMENTO DE VERDAD (CLAUDE.md 6): un buffer con nombre
    # en la lista blanca no dice nada de lo que lleva dentro. Se mide.
    vig = codigos.get("vigilante.cpp", "")
    latidoDef = re.search(r'#define\s+LATIDO_LINEA\s+"([^"]*)"', fw.codigo(*LATIDO_DEF))
    b.verificar(
        re.search(r"\bLINEA\s*\[\s*\]\s*=\s*LATIDO_LINEA\s*\"\\r\\n\"\s*;", vig) is not None
        and latidoDef is not None and latidoDef.group(1) in EMITE_HACIA_EL_STM32,
        "el latido lleva LATIDO_LINEA y nada mas, y LATIDO_LINEA es %r, la linea reservada "
        "revisada" % (latidoDef.group(1) if latidoDef else "?"),
        "el buffer LINEA de vigilante.cpp no es `LATIDO_LINEA \"\\r\\n\"`, o LATIDO_LINEA "
        "(contrato.h) ya no es la linea revisada (%r). Lo que sale cada 2 s hacia el STM32 "
        "dejaria de ser la linea que las dos puntas ignoran sin contestar"
        % (latidoDef.group(1) if latidoDef else None))

    siembra = codigos.get(SIEMBRA)
    if siembra is None:
        raise fw.Abortado(
            "no existe %s/src/%s. Es donde vive la unica excepcion de ORDEN hacia el "
            "STM32 (D-20); sin leerla, este pack aprobaria una siembra que no ha visto"
            % (ROL, SIEMBRA))
    formatos = dict(re.findall(r'static\s+const\s+char\s+(\w+)\[\]\s*=\s*"([^"]*)"',
                               siembra))
    ok, motivo = _siembra_honesta(siembra, "linea", FORMATO_SIEMBRA, formatos)
    b.verificar(
        ok,
        "D-20: la hora que sale hacia el STM32 se compone SOLO de reloj_leer(): %s" % motivo,
        "LA SIEMBRA NO SE COMPONE SOLO DE LO QUE DA LA BARRERA DEL DS3231: %s. Esa es la "
        "unica razon por la que el puente puede originar esta orden; sin ella es el "
        "accesorio dictando la hora a un micro que se autoriza el Degradado sobre ella"
        % motivo)

    # ---- 5. Todo literal '$' o 'CMD:' del proyecto esta revisado a mano -------
    # 11/09: se ensancha en DOS direcciones, y las dos eran huecos. "CMD:" porque las
    # ordenes al STM32 empiezan asi; include/ porque "$LATIDO" vive alli y no se veia.
    todos = set()
    for carpeta, ext in (("src", ".cpp"), ("include", ".h")):
        for f in fw.fuentes_de("ESP32_Expansion", carpeta, ext):
            todos.update(re.findall(r'"(\$[^"]*|CMD:[^"]*)"',
                                    fw.codigo("ESP32_Expansion", carpeta, f)))

    revisados = set(EMITE_HACIA_LA_APP) | set(EMITE_HACIA_EL_STM32) | set(NO_SON_EMISIONES)
    sinRevisar = sorted(todos - revisados)
    b.verificar(
        not sinRevisar,
        "los %d literales de trama y de orden del proyecto (src/ e include/) estan en la "
        "lista revisada a mano" % len(todos),
        "LITERAL DE TRAMA U ORDEN NO REVISADO: %s. Puede ser inofensivo, pero nadie lo ha "
        "mirado. Anadirlo a este fichero con su motivo es parte de escribirlo: si el "
        "pack leyera los literales del propio fuente, uno nuevo se aprobaria a si mismo"
        % sinRevisar)

    # ---- 6. B-4: lo que el puente emite va MARCADO como suyo ------------------
    sinMarca = sorted(l for l in EMITE_HACIA_LA_APP if "NODE:PUENTE" not in l)
    b.verificar(
        not sinMarca,
        "los %d literales que el puente emite llevan NODE:PUENTE: no se pueden confundir "
        "con los del equipo" % len(EMITE_HACIA_LA_APP),
        "el puente emite tramas SIN MARCAR: %s. Un $ERR del puente que parezca del STM32 "
        "manda a diagnosticar el poste equivocado" % sinMarca)

    # ---- 7. B-3: no compone $STATUS en nombre del equipo ---------------------
    inventa = sorted(l for l in EMITE_HACIA_LA_APP if l.startswith("$STATUS"))
    b.verificar(
        not inventa,
        "el puente no compone ningun $STATUS: sin datos del equipo, la app se entera de "
        "que no los hay en vez de recibir un estado fabricado",
        "el puente compone %s. Lo que sustituye a un dato que no se tiene no es una "
        "simulacion: es decirlo. Un tablero que anima un cruce que no existe le miente a "
        "quien decide sobre el trafico mirandolo" % inventa)

    # ---- CONTROLES NEGATIVOS -------------------------------------------------
    b.control_negativo(
        bool(re.search(r'enlace_escribirLinea\s*\(\s*"',
                       'enlace_escribirLinea("CMD:FORZAR_ROJO", 15);')),
        "una llamada a la puerta con un literal de comando se detecta")

    b.control_negativo(
        bool(re.search(PUERTO, "void f(){ Serial2.print(x); }")),
        "una segunda puerta abierta en otro fichero se detecta")

    b.control_negativo(
        bool({"$STATUS,NODE:MAESTRO,MODO:AUTO"} - revisados),
        "un $STATUS fabricado por el puente saldria como literal no revisado")

    # ---- 11/09: LOS CONTROLES DE LA BARRERA ESTRECHADA ------------------------
    #
    # Lo que el encargo exige y es lo unico que hace creible la excepcion: que CUALQUIER
    # OTRA orden hacia el STM32 siga cayendo. Sobre fuentes SINTETICOS con las mismas
    # funciones reales (N-89: reutilizar el fuente bueno mediria lo mismo que la
    # comprobacion y no demostraria nada).

    # (a) una cuarta escritura, con un BUFFER y no un literal -que es como pasaba el
    # latido por delante de la 3-, en un fichero nuevo.
    otra = dict(codigos)
    otra["mando_remoto.cpp"] = ('static char orden[20];\nvoid f() {\n'
                                '  strcpy(orden, "CMD:FORZAR_ROJO");\n'
                                '  enlace_escribirLinea(orden, 15);\n}\n')
    b.control_negativo(
        ("mando_remoto.cpp", "orden") in _escrituras_hacia_el_stm32(otra)
        and ("mando_remoto.cpp", "orden") not in ESCRITURAS_PERMITIDAS,
        "una cuarta escritura hacia el STM32 -con un buffer, en un fichero nuevo- sale en "
        "el censo de la 4.bis y no esta en la lista: la barrera la caza")

    # (b) y su literal, que la 5 de antes NO veia por no empezar por '$'.
    b.control_negativo(
        "CMD:FORZAR_ROJO" in set(re.findall(r'"(\$[^"]*|CMD:[^"]*)"',
                                            otra["mando_remoto.cpp"])) - revisados,
        "un literal de ORDEN (\"CMD:FORZAR_ROJO\") en src/ sale como no revisado: el "
        "censo de antes solo miraba '$' y lo habria dejado pasar")

    # (c) la siembra DENTRO de la lista pero con una hora que no dio la barrera: la
    # misma escritura, el mismo buffer, el mismo formato... y la hora tecleada.
    inventada = ('static const char FORMATO_HORA_ESP32[] = "%s";\n'
                 'bool siembra_ahora() {\n  FechaHora leida;\n'
                 '  if (!reloj_leer(&leida)) return false;\n  char linea[64];\n'
                 '  int n = snprintf(linea, sizeof(linea), FORMATO_HORA_ESP32, '
                 '2026, 9, 11, 12, 0, 0);\n'
                 '  return enlace_escribirLinea(linea, (size_t)n) > 0;\n}\n'
                 % "CMD:HORA_ESP32:%04d-%02d-%02d,%02d:%02d:%02d")
    fInv = dict(re.findall(r'static\s+const\s+char\s+(\w+)\[\]\s*=\s*"([^"]*)"', inventada))
    b.control_negativo(
        not _siembra_honesta(inventada, "linea", FORMATO_SIEMBRA, fInv)[0],
        "una siembra con la hora TECLEADA -mismo buffer, mismo formato, misma lista "
        "blanca- se caza: la excepcion se mide por lo que lleva, no por como se llama")

    # (d) y con la barrera LLAMADA pero no MIRADA: el resultado de reloj_leer() tirado.
    sinMirar = inventada.replace("  if (!reloj_leer(&leida)) return false;\n",
                                 "  reloj_leer(&leida);\n").replace(
        "2026, 9, 11, 12, 0, 0",
        "leida.anio, leida.mes, leida.dia, leida.hora, leida.minuto, leida.segundo")
    b.control_negativo(
        not _siembra_honesta(sinMirar, "linea", FORMATO_SIEMBRA, fInv)[0],
        "una siembra que llama a reloj_leer() y NO mira lo que devolvio se caza: la hora "
        "saldria de un buffer sin rellenar con formato perfecto, que es N-144")

    # (e) y la version buena de ese mismo sintetico PASA: el detector distingue, no
    # acusa a todo el que siembra.
    buena = sinMirar.replace("  reloj_leer(&leida);\n",
                             "  if (!reloj_leer(&leida)) return false;\n")
    b.control_negativo(
        _siembra_honesta(buena, "linea", FORMATO_SIEMBRA, fInv)[0],
        "la siembra correcta del mismo sintetico PASA la 4.ter: el detector distingue la "
        "hora de la barrera de la hora inventada")
