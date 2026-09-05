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
    "$ACK,NODE:PUENTE,CMD:SET_RTC,RESULT:HORA_PUESTA_SIN_PROPAGAR,":
        "motivo 7: la hora entro aqui y la linea no llego entera al equipo",
    "$ACK,NODE:PUENTE,CMD:SET_RTC,RESULT:OK,":
        "la hora entro, se releyo y va camino del equipo",
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
}


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

    # ---- 2. La puerta no tiene NI UN literal de trama -------------------------
    literales = re.findall(r'"(\$[^"]*)"', puerta)
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

    # ---- 5. Todo literal '$' del proyecto esta revisado a mano ----------------
    todos = set()
    for f in fuentes:
        todos.update(re.findall(r'"(\$[^"]*)"', fw.codigo("ESP32_Expansion", "src", f)))

    revisados = set(EMITE_HACIA_LA_APP) | set(NO_SON_EMISIONES)
    sinRevisar = sorted(todos - revisados)
    b.verificar(
        not sinRevisar,
        "los %d literales de trama del proyecto estan en la lista revisada a mano"
        % len(todos),
        "LITERAL DE TRAMA NO REVISADO: %s. Puede ser inofensivo, pero nadie lo ha "
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
