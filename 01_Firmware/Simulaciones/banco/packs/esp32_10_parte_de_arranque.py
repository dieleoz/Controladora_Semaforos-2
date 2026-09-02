# ===== banco/packs/esp32_10_parte_de_arranque.py =====
#
# UN PUENTE QUE REVIVE EN SILENCIO ESCONDE EL FALLO QUE HAY QUE CONTAR.
#
# POR QUE ESTE PACK NO ES REDUNDANTE CON LOS DOS DEL WATCHDOG.
#
#   esp32_01  el NUMERO del watchdog cabe entre sus cotas.
#   esp32_02  el MECANISMO existe donde tiene que existir: armado, registro y reset.
#   esp32_10  y cuando el perro MUERDE, alguien se entera.
#
# Los dos primeros dan por bueno un modulo que se reinicia. Eso, solo, es peligroso:
# desde la app y desde el STM32, un ESP32 que se reinicia cada dos segundos y uno sano
# SON INDISTINGUIBLES mientras el reinicio sea rapido -la telemetria se corta un
# instante y vuelve-. La primera sesion de banco del ESP32 -su primera ejecucion en
# hardware, nunca ha corrido en una tarjeta- terminaria con el equipo "funcionando" y
# con el unico numero que importa perdido para siempre.
#
# LAS TRES COSAS QUE SE VIGILAN, Y DE DONDE SALE CADA UNA:
#
#   A-1  LA CAUSA SE LEE DEL CHIP. esp_reset_reason() devuelve lo que el hardware
#        apunto en el dominio RTC. Deducirla de una bandera propia solo acertaria en
#        los reinicios que el firmware ve venir, que son justo los que no importan.
#   A-2  LA CUENTA SOBREVIVE AL REINICIO, Y ESO SE COMPROBO EN LA DOCUMENTACION DEL
#        PROPIO IDF ANTES DE ESCRIBIRLO. Medido en
#           C:/.platformio/packages/framework-arduinoespressif32/
#               tools/sdk/esp32/include/esp_common/include/esp_attr.h
#           :77   RTC_DATA_ATTR    ".rtc.data"    "keep its value during a deep sleep /
#                                                  wake cycle"    <- NO dice reinicio
#           :102  RTC_NOINIT_ATTR  ".rtc_noinit"  "keep its value AFTER RESTART or
#                                                  during a deep sleep / wake cycle"
#        Un reinicio por watchdog es un restart, no un despertar: solo la segunda sirve.
#        Y comprobado ademas sobre el binario, que es la medida y no la lectura:
#           xtensa-esp32-elf-nm -S firmware.elf -> marcaRtc y arranques en 0x50000200
#           xtensa-esp32-elf-objdump -h         -> .rtc_noinit  0x18 @ 0x50000200
#        Este pack vigila la ELECCION en el fuente; el binario ya se midio una vez.
#   A-3  EL PARTE NO SE PUEDE TRUNCAR. Es la desigualdad de abajo.
#
# LA DESIGUALDAD, QUE ES LA RAZON PRINCIPAL DE QUE ESTO SEA UN PACK Y NO UN COMENTARIO:
#
#   VIGILANTE_PARTE_MAX  >=  texto fijo del formato
#                            + (nombre de texto mas largo) x (cuantos %s haya)
#                            + (digitos del contador)      x (cuantos %lu haya)
#                            + 1 del nulo
#
# Los cuatro sumandos se RECALCULAN del fuente en cada corrida: el formato y los
# literales de vigilante.cpp, y los digitos del TIPO declarado del contador -no de un
# 10 escrito a mano, que dejaria de valer el dia que alguien lo pasara a uint64_t-.
#
# Es N-71 aplicado a un buffer. Alli el umbral de silencio de SFTY-6 estaba en 12 s
# mientras el ciclo necesitaba 20,5 s, y la relacion vivia "solo en prosa, dentro de un
# comentario": los reintentos 4 y 5 eran codigo muerto y nada lo delataba. Aqui, si
# alguien anade una causa con un nombre mas largo, snprintf trunca en silencio, el
# checksum sale calculado sobre el trozo, y la app ensena una causa a medias dandola por
# buena. Un comentario no falla cuando alguien cambia un numero.
#
# LA LISTA DE CAUSAS SE ESCRIBE A MANO, Y ESO ES DELIBERADO.
#
# Es el molde de esp32_05_no_origina, con su decision de metodo copiada: "si el pack
# leyera los comandos del propio fuente, un comando nuevo se aprobaria a si mismo". Si
# esta lista saliera del switch de vigilante.cpp, una causa que faltara faltaria en los
# dos sitios a la vez y el pack seguiria en verde.
#
# Y NO SE LEE DEL HEADER DEL IDF, aunque este ahi y aunque de ahi salgan estos once
# nombres. CLAUDE.md seccion 4: "un instrumento no puede depender del entorno de quien
# lo llama". Un pack que abortara en la maquina que no tenga instalado el framework
# dejaria abierta -por ABORTADO- justo la puerta que vino a cerrar.
#
# NO LLEVA ETIQUETA SFTY, por el mismo motivo que esp32_01 y esp32_02: SFTY-1 es el
# watchdog de los STM32 y asignar numero nuevo es del responsable (AB-8). Etiquetar sin
# regla asignada pondria en la tabla de trazabilidad una fila que no corresponde a nada,
# y una fila que miente es peor que una vacia.

import re

NOMBRE = "esp32_10_parte_de_arranque"
DESCRIPCION = "el ESP32 declara por que arranco y cuantas veces, y el parte no se puede truncar"

ROL = "ESP32_Expansion"
# El rol va ESCRITO, no por la variable ROL, y no es estilo: la guarda de rutas de
# compuerta.py lee estas tuplas POR TEXTO. Con una variable delante solo ve
# ("include", "vigilante.h") -una tupla de dos, que no dice el rol- y entonces la exige
# en LAS DOS PUNTAS. Resultado el 01/09: ABORTO la guarda entera reclamando
# Maestro/include/vigilante.h y Esclavo/include/vigilante.h, que no existen ni deben.
# Un ABORTADO en la guarda apaga la compuerta completa, asi que esto no es cosmetico.
VIGILANTE_H = ("ESP32_Expansion", "include", "vigilante.h")
CONTRATO = ("ESP32_Expansion", "include", "contrato.h")

# LOS ONCE VALORES DE esp_reset_reason_t, ESCRITOS A MANO.
#
# Medidos el 01/09 en tools/sdk/esp32/include/esp_system/include/esp_system.h:41-52 del
# framework-arduinoespressif32 @ 3.20017.241212. Anadir uno aqui obliga a pasar por este
# fichero y a justificar como se llama para el operario; leerlos del switch del firmware
# haria que una causa olvidada se aprobara a si misma.
CAUSAS_DEL_IDF = (
    "ESP_RST_UNKNOWN",
    "ESP_RST_POWERON",
    "ESP_RST_EXT",
    "ESP_RST_SW",
    "ESP_RST_PANIC",
    "ESP_RST_INT_WDT",
    "ESP_RST_TASK_WDT",
    "ESP_RST_WDT",
    "ESP_RST_DEEPSLEEP",
    "ESP_RST_BROWNOUT",
    "ESP_RST_SDIO",
)

# Bytes que NO pueden aparecer dentro de un nombre de causa. La coma separa campos de la
# trama y el '*' abre el checksum: una causa con cualquiera de los dos partiria la trama
# por dentro y el parser de la app leeria un campo que nadie escribio.
PROHIBIDOS = (",", "*", "$", "\r", "\n")


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


def _cuerpo(codigo, firma):
    m = re.search(firma + r"\s*\{", codigo)
    return None if not m else _bloque(codigo, m.end() - 1)


def _literal(codigo, patron, que, fw):
    """Un literal de cadena del C++. ABORTA si no aparece.

    Misma regla que fw.constante() con los numeros: sin valor por defecto, nunca. Un
    formato que no se puede leer dejaria la desigualdad calculada sobre una cadena
    inventada, y saldria en verde."""
    m = re.search(patron, codigo)
    if not m:
        raise fw.Abortado(
            "no se pudo leer del C++ %s (patron %r en %s/src/vigilante.cpp). Sin ese "
            "literal la desigualdad del parte se calcularia sobre otra cosa que el "
            "firmware y seguiria dando PASS" % (que, patron, ROL))
    return m.group(1)


def _expandir(cadena):
    """Los escapes de C que caben en un literal de formato, a su byte real."""
    return cadena.replace("\\r", "\r").replace("\\n", "\n").replace("\\t", "\t")


def correr(b, fw):
    b.titulo("El parte de arranque: causa del chip, cuenta que sobrevive, sin truncar")

    vig = fw.codigo(ROL, "src", "vigilante.cpp")
    main = fw.codigo(ROL, "src", "main.cpp")
    trama = fw.codigo(ROL, "src", "trama.cpp")

    # ---- A-1: LA CAUSA SE LEE DEL CHIP ---------------------------------------
    b.verificar(
        re.search(r"\bcausa\s*=\s*esp_reset_reason\s*\(\s*\)", vig) is not None,
        "A-1: la causa del arranque se lee del chip con esp_reset_reason(), no se deduce",
        "vigilante.cpp NO llama a esp_reset_reason(). Una causa deducida de una bandera "
        "propia solo acierta en los reinicios que el firmware ve venir -los que el "
        "propio firmware provoca-, y son justo los que no hay que contar: el que "
        "importa es el que ocurre CUANDO EL FIRMWARE YA NO ESTABA EJECUTANDO")

    # ---- A-2: LA CUENTA VIVE DONDE SOBREVIVE A UN REINICIO --------------------
    #
    # Se exige la seccion, no la intencion. Un `static uint32_t arranques` normal se
    # pone a cero en cada arranque y el parte diria "ARRANQUES:1" para siempre: la
    # cuenta seria una constante disfrazada de medida, que es el peor de los casos
    # -sale en verde y publica un numero-.
    declaradas = re.findall(r"RTC_NOINIT_ATTR\s+static\s+uint(\d+)_t\s+(\w+)\s*;", vig)
    nombres = [n for _, n in declaradas]
    b.verificar(
        len(declaradas) >= 2,
        "A-2: hay %d variables en .rtc_noinit (%s), la unica seccion que el IDF "
        "documenta como superviviente de un RESTART y no solo de un sueno profundo"
        % (len(declaradas), ", ".join(nombres)),
        "la cuenta de arranques NO esta declarada RTC_NOINIT_ATTR (se hallaron %d de "
        "las 2 que hacen falta: el contador y su marca de validez). RTC_DATA_ATTR solo "
        "promete el ciclo de sueno profundo -esp_attr.h:77- y un static normal no "
        "promete nada: la cuenta se pondria a cero en cada reinicio y el parte diria "
        "'ARRANQUES:1' despues de veinte caidas seguidas" % len(declaradas))

    # ---- A-2.bis: LA MARCA DE VALIDEZ ----------------------------------------
    #
    # ".rtc_noinit" NO se inicializa nunca -ese es el sentido del nombre-, tampoco en la
    # primera subida de tension de un modulo virgen. Sin marca, la cuenta arranca
    # valiendo lo que hubiera en esa RAM.
    censo = _cuerpo(vig, r"void\s+vigilante_censarArranque\s*\(\s*\)")
    if censo is None:
        raise fw.Abortado(
            "no se hallo vigilante_censarArranque() en %s/src/vigilante.cpp. Es donde "
            "vive toda la logica de la cuenta; sin poder leerla, aprobarla seria "
            "aprobar sin mirar" % ROL)

    marcas = [n for n in nombres if re.search(r"\b%s\s*!=" % re.escape(n), censo)]
    b.verificar(
        bool(marcas),
        "A-2: el censo compara una marca de validez (%s) antes de fiarse de la cuenta: "
        ".rtc_noinit no se inicializa nunca, tampoco en un modulo virgen"
        % ", ".join(marcas),
        "el censo NO comprueba ninguna marca de validez antes de usar la cuenta. "
        "'.rtc_noinit' no se inicializa NUNCA -ese es el sentido del nombre-: en la "
        "primera subida de tension la cuenta vale lo que hubiera en esa RAM. Un "
        "contador que puede empezar en un numero cualquiera no mide, decora")

    b.verificar(
        "ESP_RST_POWERON" in censo,
        "A-2: la subida de tension pone la cuenta a cero, asi que ARRANQUES significa "
        "UNA cosa -arranques desde la ultima subida de tension- y no dos segun lo que "
        "durara el corte",
        "el censo no distingue la subida de tension. La RAM del dominio RTC puede "
        "conservar su contenido en un corte corto y perderlo en uno largo: sin esta "
        "rama, ARRANQUES significaria 'desde la ultima subida de tension' unas veces y "
        "'desde vaya usted a saber' otras. Una variable que contesta a dos preguntas "
        "distintas no puede contestar bien a ninguna -es cfgVerdeRecibido otra vez-")

    # ---- Las causas: una rama por valor del enum, y un default ----------------
    casos = re.findall(r'case\s+(ESP_RST_\w+)\s*:\s*return\s+"([^"]*)"\s*;', vig)
    porDefecto = re.search(r'default\s*:\s*return\s+"([^"]*)"\s*;', vig)
    cubiertas = set(n for n, _ in casos)

    faltan = [c for c in CAUSAS_DEL_IDF if c not in cubiertas]
    b.verificar(
        not faltan,
        "las %d causas de esp_reset_reason_t tienen nombre para el operario"
        % len(CAUSAS_DEL_IDF),
        "hay causas del enum SIN RAMA: %s. Cada sujeto de una enumeracion tiene que "
        "existir de verdad, no estar cubierto de forma vacua (N-96): un reinicio por %s "
        "llegaria al operario como el texto del default, y el tecnico no sabria si el "
        "modulo se colgo o si le fallo la alimentacion -que son dos viajes distintos-"
        % (faltan, faltan[0] if faltan else "?"))

    b.verificar(
        porDefecto is not None,
        "hay rama default (%s): un valor que Espressif anada manana no llega al "
        "operario como un hueco" % (porDefecto.group(1) if porDefecto else "?"),
        "el switch de causas NO tiene default. Un valor nuevo del enum devolveria basura "
        "o nada, y un caso nuevo se aprobaria a si mismo. Es la misma decision que "
        "MOTIVO_NO_CONTEMPLADO del despachador, y por el mismo motivo")

    textos = [t for _, t in casos]
    if porDefecto:
        textos.append(porDefecto.group(1))

    sucios = [t for t in textos if any(p in t for p in PROHIBIDOS)]
    b.verificar(
        not sucios,
        "ninguno de los %d nombres de causa lleva coma, asterisco ni '$': no pueden "
        "partir la trama por dentro" % len(textos),
        "hay nombres de causa con bytes que la trama usa como estructura: %s. La coma "
        "separa campos y el '*' abre el checksum: una causa con cualquiera de los dos "
        "hace que el parser de la app lea un campo que nadie escribio, y el checksum "
        "seguiria casando porque se calcula sobre la linea entera" % sucios)

    b.verificar(
        len(set(textos)) == len(textos),
        "los %d nombres de causa son distintos entre si: dos reinicios distintos no se "
        "leen igual" % len(textos),
        "hay nombres de causa REPETIDOS: %s. Dos causas con el mismo texto son "
        "indistinguibles desde el telefono, y una de ellas -'me colgue'- manda a "
        "cambiar el firmware mientras la otra -'me quede sin tension'- manda a mirar la "
        "fuente" % sorted(t for t in set(textos) if textos.count(t) > 1))

    # ---- A-3: LA DESIGUALDAD DEL BUFFER --------------------------------------
    #
    # Los cuatro sumandos se releen del fuente. Ninguno se teclea aqui.
    formato = _literal(
        vig, r'FORMATO_PARTE\s*\[\s*\]\s*=\s*"((?:[^"\\]|\\.)*)"\s*;',
        "el formato del parte de arranque", fw)
    formato = _expandir(formato)

    perro = re.search(r'vigilante_armado\s*\(\s*\)\s*\?\s*"([^"]*)"\s*:\s*"([^"]*)"', vig)
    if perro is None:
        raise fw.Abortado(
            "no se hallo en %s/src/vigilante.cpp el par de textos del estado del perro. "
            "Son dos de los sumandos de la desigualdad del parte: sin ellos se estaria "
            "midiendo un buffer contra una cadena mas corta que la real" % ROL)
    textosPerro = [perro.group(1), perro.group(2)]

    # LOS DIGITOS SALEN DEL TIPO DECLARADO, no de un 10 escrito a mano. El dia que
    # alguien pase el contador a uint64_t, esta cuenta cambia sola.
    bits = max(int(x) for x, _ in declaradas) if declaradas else 0
    if bits <= 0:
        raise fw.Abortado(
            "no se pudo leer el TIPO del contador de arranques en %s/src/vigilante.cpp. "
            "De el salen los digitos que la desigualdad reserva; con un 10 escrito a "
            "mano, el pack seguiria aprobando el dia que el contador fuera uint64_t"
            % ROL)
    digitos = len(str((1 << bits) - 1))

    fijo = re.sub(r"%l?[su]", "", formato)
    n_s = len(re.findall(r"%s", formato))
    n_lu = len(re.findall(r"%lu", formato))
    maxTexto = max(len(t) for t in textos + textosPerro)

    # Conservador a proposito: se reserva el texto MAS LARGO para cada %s sin mirar cual
    # va en cual. Saber el orden obligaria a que el pack llevara una segunda copia de la
    # llamada a snprintf, y esa copia es exactamente lo que se sincroniza mal.
    peor = len(fijo) + n_s * maxTexto + n_lu * digitos + 1   # +1 del nulo

    tope = fw.constante(VIGILANTE_H, r"#define\s+VIGILANTE_PARTE_MAX\s+(\d+)",
                        "el buffer donde se arma el parte de arranque")

    b.verificar(
        tope >= peor,
        "A-3: el parte no se puede truncar: VIGILANTE_PARTE_MAX = %d >= %d (%d de texto "
        "fijo + %d x %d del texto mas largo + %d x %d digitos de un uint%d_t + 1)"
        % (tope, peor, len(fijo), n_s, maxTexto, n_lu, digitos, bits),
        "EL PARTE PUEDE TRUNCARSE: VIGILANTE_PARTE_MAX = %d y el peor caso son %d "
        "bytes. snprintf trunca EN SILENCIO, trama_componer() calcularia el checksum "
        "sobre el trozo, y la app ensenaria una causa a medias dandola por buena. Es la "
        "clase de mentira bien formada que este puente existe para no cometer: subir "
        "VIGILANTE_PARTE_MAX a %d o acortar el nombre mas largo" % (tope, peor, peor))

    # ---- A-3.bis: y lo que sale de ahi cabe en el buffer de salida ------------
    sufijo = _literal(
        trama, r'snprintf\s*\(\s*destino\s*,\s*capacidad\s*,\s*"((?:[^"\\]|\\.)*)"',
        "el formato con que trama_componer() cierra la trama", fw)
    sufijo = _expandir(sufijo).replace("%s", "")
    sobrecoste = len(re.sub(r"%02X", "XX", sufijo))

    salida = fw.constante(CONTRATO, r"#define\s+BUF_SALIDA_APP\s+(\d+)",
                          "el buffer de salida hacia la app")
    total = (tope - 1) + sobrecoste + 1
    b.verificar(
        total <= salida,
        "y la trama entera cabe en el buffer de salida: %d payload + %d de cierre + 1 "
        "= %d <= BUF_SALIDA_APP (%d)" % (tope - 1, sobrecoste, total, salida),
        "EL PARTE COMPUESTO NO CABE EN BUF_SALIDA_APP: %d bytes contra %d. "
        "trama_componer() devolveria 0 y el parte no saldria nunca -en silencio-, que "
        "es exactamente el reinicio mudo que este pack existe para impedir"
        % (total, salida))

    # ---- B-1 y B-3: el parte va a la APP, y solo a la APP ---------------------
    b.verificar(
        "puente_emitirPropio" in vig and "enlace_escribirLinea" not in vig,
        "el parte sale por puente_emitirPropio() -hacia la app- y vigilante.cpp no "
        "nombra la puerta hacia el STM32",
        "vigilante.cpp escribe hacia el STM32 o no emite hacia la app. Un parte del "
        "puente mandado al equipo es el accesorio originando trafico hacia un micro que "
        "gobierna un cruce y que no valida quien habla (B-1); y si no sale por ningun "
        "lado, el reinicio sigue siendo mudo")

    b.verificar(
        "NODE:PUENTE" in formato,
        "B-4: el parte va marcado con NODE:PUENTE, asi que no se puede confundir con un "
        "evento del equipo",
        "el parte de arranque NO lleva NODE:PUENTE. Un $EVENT del accesorio que parezca "
        "del STM32 manda a diagnosticar el poste equivocado: el tecnico buscaria en el "
        "firmware del semaforo un reinicio que fue del puente")

    # ---- El sitio de cada llamada --------------------------------------------
    setup = _cuerpo(main, r"void\s+setup\s*\(\s*\)")
    loop = _cuerpo(main, r"void\s+loop\s*\(\s*\)")
    if setup is None or loop is None:
        raise fw.Abortado(
            "no se hallaron setup() y loop() en %s/src/main.cpp: sin ellos no se puede "
            "decir donde se censa el arranque ni donde se declara, que es la mitad de "
            "este pack" % ROL)

    b.verificar(
        "vigilante_censarArranque" in setup and "vigilante_declarar" not in setup,
        "el censo se hace en setup() y la declaracion NO: setup() no pone un byte en "
        "ningun cable (6.4) y ademas no habria nadie escuchando todavia",
        "o el arranque no se censa en setup(), o el parte se EMITE desde setup(). Lo "
        "segundo rompe dos cosas a la vez: un saludo del puente es una orden que nadie "
        "pidio, y con el SPP recien abierto no hay telefono conectado -"
        "transporte_escribir() devuelve 0-, asi que el parte se perderia justo la vez "
        "que hace falta")

    b.verificar(
        "vigilante_declarar" in loop,
        "la declaracion vive en el bucle exterior, que es donde puede esperar a que haya "
        "alguien al otro lado",
        "loop() NO llama a vigilante_declarar(). El modulo contaria sus reinicios y no "
        "se los diria a nadie: un contador que nadie lee es la version silenciosa de la "
        "prueba muerta")

    declarar = _cuerpo(vig, r"void\s+vigilante_declarar\s*\(\s*\)")
    if declarar is None:
        raise fw.Abortado(
            "no se hallo vigilante_declarar() en %s/src/vigilante.cpp: es donde vive la "
            "espera al enlace, y sin leerla no se puede comprobar" % ROL)

    b.verificar(
        "transporte_conectado" in declarar,
        "el parte espera a que haya telefono conectado antes de salir: "
        "transporte_escribir() devuelve 0 sin cliente y el parte se perderia entero",
        "vigilante_declarar() NO mira si hay alguien conectado. transporte_escribir() "
        "devuelve 0 sin telefono (transporte_app.cpp:59) y el parte se compone UNA sola "
        "vez: se emitiria al vacio en la primera vuelta y el operario no sabria nunca "
        "que el modulo se cayo")

    # ---- N-73: el parte cierra una funcion huerfana ---------------------------
    #
    # vigilante_armado() estaba declarada, definida y documentada con su motivo, y sin
    # un solo llamador. El campo PERRO del parte es su primer consumidor, y no es
    # adorno: es lo unico que distingue "el modulo volvio" de "el modulo volvio y SIGUE
    # sin vigilancia", que es el peor de los dos.
    llamadas = len(re.findall(r"vigilante_armado\s*\(\s*\)", vig))
    b.verificar(
        llamadas >= 2,
        "vigilante_armado() ya no es huerfana: %d apariciones -su definicion y al menos "
        "un llamador-, y el parte publica si el perro esta armado de verdad" % llamadas,
        "vigilante_armado() vuelve a no tener llamador (%d apariciones). Una funcion "
        "declarada, definida y documentada que nadie llama es la version silenciosa de "
        "la prueba muerta (N-73), y aqui cuesta el dato que mas falta hace: un modulo "
        "que reinicia SIN perro armado se ve igual que uno sano" % llamadas)

    # ---- CONTROLES NEGATIVOS -------------------------------------------------
    #
    # Cada uno rompe UNA de las propiedades y exige que el detector lo note. Sin esto,
    # el dia que un patron dejara de casar el pack compararia nada contra nada.
    formatoLargo = formato.replace("CAUSA:", "CAUSA_DEL_ULTIMO_REINICIO_DEL_MODULO:")
    peorLargo = (len(re.sub(r"%l?[su]", "", formatoLargo))
                 + n_s * maxTexto + n_lu * digitos + 1)
    b.control_negativo(
        not (tope >= peorLargo),
        "un formato mas largo rompe la desigualdad del buffer (%d > %d): la cuenta no "
        "esta escrita a mano, se recalcula" % (peorLargo, tope))

    b.control_negativo(
        any(p in "PERRO,DE,TAREAS" for p in PROHIBIDOS),
        "un nombre de causa con una coma dentro se detecta como capaz de partir la trama")

    soloSueno = "RTC_DATA_ATTR static uint32_t arranques;"
    b.control_negativo(
        not re.findall(r"RTC_NOINIT_ATTR\s+static\s+uint(\d+)_t\s+(\w+)\s*;", soloSueno),
        "una cuenta declarada RTC_DATA_ATTR -que el IDF solo promete para el sueno "
        "profundo, no para un reinicio- NO se acepta como superviviente")

    censoSinMarca = "{ causa = esp_reset_reason(); arranques++; }"
    b.control_negativo(
        not [n for n in ("marcaRtc", "arranques")
             if re.search(r"\b%s\s*!=" % n, censoSinMarca)],
        "un censo que suma sin comprobar la marca de validez se detecta")

    b.control_negativo(
        re.search(r"#define\s+VIGILANTE_PARTE_MAX\s+(\d+)",
                  "#define OTRA_COSA 144") is None,
        "el lector del tope no casa con una constante de otro nombre: no hay valor por "
        "defecto que tapara la desaparicion de VIGILANTE_PARTE_MAX")
