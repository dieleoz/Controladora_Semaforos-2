# ===== banco/packs/costura_11_lcd_sin_bus.py =====
#
# LOS TRES HILOS DE LA PANTALLA NO SE CONDUCEN: EL ESP32 VIVE EN ESE CONECTOR.
#
# ESTO NO ES LIMPIEZA. Es que no haya un reloj de SPI conmutando dentro del mismo
# conector por el que pasa el enlace serie del ESP32.
#
# EL DATO, medido en el cobre. 03_Hardware_Tarjeta/MAPEO_TARJETA_KICAD.md:349-350
# reparte UN SOLO conector entre dos cosas distintas:
#
#   LCD ST7920 (3 hilos desde N-76)   PB3 PB4 PB5     ->  J17  p4, p1, p5
#   Modulo Bluetooth / ESP32          PB6 TX PB7 RX   ->  J17  p3, p2
#
# Y :378 anade el detalle que lo vuelve urgente: PB3 es SCL (p4) y CONMUTA EN CADA
# BIT. Un reloj de software corriendo a velocidad SPI pegado al RX/TX del ESP32,
# dentro del mismo mazo, es exactamente lo que produce corrupcion intermitente en
# el serie: la que no se diagnostica nunca, porque aparece y desaparece segun lo
# que la pantalla este dibujando en ese instante.
#
# Ademas son fisicamente excluyentes: es un conector. En cuanto el ESP32 ocupa J17
# la pantalla ya no esta, se retire su codigo o no. Lo que quedaba era el firmware
# conduciendo tres hilos hacia un modulo que ya no esta enchufado.
#
# D-32 (1), 13/09/2026 - ESTE PACK SE REPARTIO, NO SE BORRO. Y hay que leer por que,
# porque la mitad que cambia es la que EXIGIA EL COMPORTAMIENTO QUE HOY SE RETIRA.
#
# LO QUE DECIA AQUI HASTA HOY, literal y ahora derogado: "No pide que la pantalla se
# retire. Se decidio expresamente NO retirarla, y el motivo es vial: menu.cpp:215 del
# Esclavo es UNA DE LAS TRES VIAS que lo sacan del Modo Degradado (...) Por eso la
# prueba 4 exige que el dibujo SIGA VIVO."
#
# DOS COSAS LO TUMBAN, y las dos son medidas, no decisiones:
#
#   1. D-32 (1): el responsable retira el LCD del firmware -"solo retirar el lcd"-.
#   2. Y LA VIA QUE ESA FRASE PROTEGIA YA ESTABA MUERTA cuando se escribio. Para
#      llegar a menu.cpp:215 -la salida del Degradado por pantalla- hacia falta
#      botonAceptar(), que es `return false;` en botones.cpp desde el 31/08 (D-2: sus
#      pines son camaras). El cursor del Esclavo no podia bajar del listado inicial.
#      O sea que la prueba 4 estaba conservando una via de seguridad INALCANZABLE, y
#      cobrandose a cambio el derecho de veto sobre retirar la pantalla. Es el caso de
#      CLAUDE.md 9 en su forma cara: una prueba madura que EXIGE el comportamiento
#      defectuoso porque se escribio cuando parecia inevitable.
#      Las vias vivas de salida del Degradado del Esclavo son las otras dos que la
#      propia frase nombraba -mando.cpp y la puerta automatica de main.cpp- mas la
#      orden por app de D-18, que ya esta construida en su despachador de Bluetooth.
#
# QUE SE HIZO CON CADA MITAD:
#   - 1.1, 1.2 y 2.1 SE CONSERVAN TAL CUAL. Son la propiedad de verdad -que nadie
#     conduzca PB3/PB4/PB5- y hoy son mas fuertes, no mas debiles: el unico fichero
#     que podia conducirlos ya no existe.
#   - 3.1 y 4.1 SE INVIERTEN. Donde pedian "el constructor de u8g2 no recibe pines" y
#     "el framebuffer sigue vivo", ahora piden que no quede ni libreria ni pantalla.
#     Una inversion sin control aprueba cualquier cosa, asi que los controles
#     negativos del final se rehicieron contra ficheros que SI existen.
#
# (Las cifras "271 comprobaciones de Validacion_LCD" que este comentario repetia
# estaban ademas caducadas: el acta del 13/09 daba 287. El arnes ya no existe.)

import re

NOMBRE = "costura_11_lcd_sin_bus"
DESCRIPCION = ("los tres hilos del LCD no se conducen: el ESP32 comparte J17 y un "
               "reloj SPI ahi corrompe el enlace serie")

PUNTAS = ("Maestro", "Esclavo")

# Los tres hilos de DATOS del LCD, que son los que van a J17 p4/p1/p5. PB6 y PB7 no
# estan en esta lista porque ya se los llevo el USART1 del Bluetooth en N-76.
MACROS = ("LCD_SCLK", "LCD_CS", "LCD_SID")

# Lo que el mapa de la tarjeta dice que son esos tres. Se comprueba, no se supone:
# si alguien remapea la pantalla a otros pines, esto tiene que saltar y que una
# persona lo mire, porque el reparto de J17 deja de ser el medido.
PINES_J17 = {"PB3", "PB4", "PB5"}

# Las formas de CONDUCIR un pin en Arduino. Leer tambien cuenta: pinMode(INPUT) sobre
# uno de estos hilos volveria a atarlo al periferico y es un cambio que hay que ver.
CONDUCTORAS = ("pinMode", "digitalWrite", "digitalRead", "analogWrite", "analogRead")


def _pines_del_lcd(punta):
    """Los tres pines del LCD leidos del pines.h de esa punta.

    SIN VALOR POR DEFECTO: si una macro no aparece se devuelve None y la prueba 1
    falla. Un banco que cae a una lista escrita a mano seguiria dando PASS el dia
    que alguien renombre la macro, midiendo pines que el firmware ya no usa."""
    if punta == "Maestro":
        txt = _texto_pines_maestro()
    else:
        txt = _texto_pines_esclavo()
    hallados = {}
    for macro in MACROS:
        m = re.search(r"#define\s+%s\s+(P[A-Z]\d+)" % macro, txt)
        if m:
            hallados[macro] = m.group(1)
    return hallados


# Las rutas van en TUPLAS LITERALES y no armadas con una variable: la guarda de
# rutas de compuerta.py censa las tuplas por texto, y una construida a trozos la
# deja inventando rutas que no existen -y entonces la compuerta ABORTA, que es
# peor que cualquier fallo que este pack pudiera encontrar-.
def _texto_pines_maestro():
    return _FW.texto("Maestro", "include", "pines.h")


def _texto_pines_esclavo():
    return _FW.texto("Esclavo", "include", "pines.h")


# D-32 (1): aqui vivian _codigo_lcd_maestro() y _codigo_lcd_esclavo(), las dos unicas
# tuplas de ruta hacia el lcd.cpp de cada punta que este pack aportaba al censo de
# compuerta.py. Se van con el fichero: dejarlas ABORTA la guarda de rutas entera.


_FW = None


def _conducciones(codigo, tokens):
    """Llamadas que conducen alguno de esos tokens. Devuelve la lista de textos."""
    encontradas = []
    for fn in CONDUCTORAS:
        for m in re.finditer(r"\b%s\s*\(\s*([A-Za-z0-9_]+)" % fn, codigo):
            if m.group(1) in tokens:
                encontradas.append("%s(%s" % (fn, m.group(1)))
    return encontradas


def correr(b, fw):
    global _FW
    _FW = fw

    b.titulo("J17 COMPARTIDO - los tres hilos del LCD no se conducen")

    # -- 1. Los tres pines se leen del C++ y son los que el mapa de J17 dice ------
    print("\n-- 1.1 Las tres macros del LCD se leen de pines.h en las dos puntas --")
    faltan = []
    mapeo = {}
    for punta in PUNTAS:
        hallados = _pines_del_lcd(punta)
        mapeo[punta] = hallados
        for macro in MACROS:
            if macro not in hallados:
                faltan.append("%s: no se pudo leer %s de pines.h" % (punta, macro))

    b.verificar(
        not faltan,
        "las %d macros de datos del LCD (%s) se leen del pines.h de las dos puntas: "
        "el pack mide los pines que el firmware nombra hoy, no una lista escrita a "
        "mano" % (len(MACROS), ", ".join(MACROS)),
        "NO se pudieron leer del C++: %s. Sin esos nombres este pack no sabe que "
        "pines vigilar y aprobaria sin haber mirado" % faltan)

    print("\n-- 1.2 Esas macros siguen siendo los tres hilos de J17 --")
    desviados = []
    for punta in PUNTAS:
        valores = set(mapeo[punta].values())
        if valores and valores != PINES_J17:
            desviados.append((punta, sorted(valores)))

    b.verificar(
        not desviados,
        "en las dos puntas las tres macros valen exactamente %s, que es el reparto "
        "de J17 medido en MAPEO_TARJETA_KICAD.md:349-350"
        % sorted(PINES_J17),
        "el LCD ya no esta en %s sino en %s: el reparto de J17 que justifica este "
        "pack ha cambiado y hay que volver a medirlo en el cobre antes de fiarse "
        "de nada de lo que hay debajo" % (sorted(PINES_J17), desviados))

    # -- 2. Nadie conduce esos pines, en ninguna punta y en ningun fichero --------
    #
    # Se censa el DIRECTORIO y no una lista de ficheros: una lista se queda corta el
    # dia que alguien anade un .cpp, y entonces la prueba aprueba sin haber mirado
    # donde hacia falta.
    print("\n-- 2.1 Ningun fichero de ninguna punta conduce PB3/PB4/PB5 --")
    culpables = []
    ficheros_mirados = 0
    for punta in PUNTAS:
        tokens = set(MACROS) | PINES_J17
        for carpeta, ext in (("src", ".cpp"), ("include", ".h")):
            for nombre in fw.fuentes_de(punta, carpeta, ext):
                ficheros_mirados += 1
                cod = fw.codigo(punta, carpeta, nombre)
                for uso in _conducciones(cod, tokens):
                    culpables.append("%s/%s/%s: %s" % (punta, carpeta, nombre, uso))

    b.verificar(
        not culpables,
        "ninguno de los %d ficheros de las dos puntas hace pinMode ni "
        "digitalWrite/Read sobre los tres hilos del LCD: PB3, PB4 y PB5 quedan en "
        "alta impedancia y no hay un reloj conmutando en el conector del ESP32"
        % ficheros_mirados,
        "HAY CODIGO CONDUCIENDO LOS HILOS DE J17: %s. PB3 es SCL y conmuta en cada "
        "bit; con el ESP32 en p2/p3 del mismo conector eso corrompe el enlace serie "
        "de forma intermitente, que es la averia que no se diagnostica nunca"
        % culpables)

    # -- 3. INVERTIDA POR D-32 (1): ya no hay libreria que pueda atar los pines ----
    #
    # Hasta el 13/09 esta prueba leia el constructor U8G2_* de lcd.cpp y exigia que
    # todos sus argumentos de pin fueran U8X8_PIN_NONE. Era el camino que no se ve: el
    # constructor no aparece como un pinMode pero entrega los pines a la libreria, y a
    # partir de ahi los conduce ella.
    #
    # Retirado el LCD, ese camino se cierra un nivel MAS ARRIBA y de forma que no
    # depende de como este escrito un argumento: U8g2 ya no se enlaza. Se comprueba en
    # el platformio.ini, que es donde vive la decision, y no por la ausencia de
    # lcd.cpp -que se puede recrear sin que nadie lo note-.
    print("\n-- 3.1 Ninguna punta enlaza U8g2: no hay libreria que pueda atar pines --")
    # SE LEE EL BLOQUE lib_deps, NO EL FICHERO ENTERO. CLAUDE.md 7.1: el patron cuenta
    # comentarios, y el comentario que explica la retirada de la libreria la NOMBRA. La
    # primera version de esta prueba preguntaba por el texto entero y daba FALLA sobre
    # dos .ini correctos, acusando al firmware de su propio patron ciego.
    con_libreria = []
    for punta in PUNTAS:
        texto = fw.texto(punta, "platformio.ini")
        m = re.search(r"^lib_deps\s*=(.*?)(?=^\w|^\[|\Z)", texto, re.S | re.M)
        cuerpo = re.sub(r";[^\n]*", " ", m.group(1)) if m else ""
        if "U8g2" in cuerpo or "u8g2" in cuerpo:
            con_libreria.append(punta)

    b.verificar(
        not con_libreria,
        "ninguna de las dos puntas declara U8g2 en su platformio.ini: no queda "
        "libreria de pantalla que pueda tomar PB3/PB4/PB5 por su cuenta, asi que la "
        "prueba 2.1 ya no tiene una puerta trasera por la que colarsele",
        "%s vuelve(n) a declarar U8g2. Con el ESP32 en J17 eso devuelve el reloj SPI "
        "al conector del enlace serie por un camino que ningun pinMode ensena: la "
        "libreria conduce los pines que le entregue el constructor" % con_libreria)

    # -- 4. INVERTIDA POR D-32 (1): no queda dibujo en ninguna punta --------------
    #
    # ESTA ERA LA PRUEBA QUE CELEBRABA EL DEFECTO (CLAUDE.md 9). Exigia que drawStr y
    # sendBuffer SIGUIERAN existiendo en los dos lcd.cpp, y su motivo escrito -guardar
    # la via de salida del Degradado de menu.cpp:215- llevaba muerto desde el 31/08,
    # cuando botonAceptar() paso a ser `return false;`. Se invierte entera.
    #
    # Y NO MIRA SOLO EL RESULTADO. Comprobar "lcd.cpp no existe" aprobaria igual de
    # bien un arbol al que le falta el fichero por accidente que uno del que se retiro
    # la pantalla: se mira ademas que NADIE haya heredado el dibujo, que es por donde
    # volveria de verdad -un drawStr mudado a menu.cpp o a un modo-.
    print("\n-- 4.1 No queda pantalla: ni lcd.cpp, ni dibujo heredado por otro --")
    restos = []
    for punta in PUNTAS:
        for nombre in fw.fuentes_de(punta, "src", ".cpp"):
            if nombre == "lcd.cpp":
                restos.append("%s: vuelve a existir src/lcd.cpp" % punta)
                continue
            cod = fw.codigo(punta, "src", nombre)
            for token in ("drawStr", "sendBuffer", "u8g2", "U8G2"):
                if re.search(r"\b%s\b" % token, cod):
                    restos.append("%s/src/%s nombra %s" % (punta, nombre, token))
        for nombre in fw.fuentes_de(punta, "include", ".h"):
            if nombre == "lcd.h":
                restos.append("%s: vuelve a existir include/lcd.h" % punta)

    b.verificar(
        not restos,
        "ninguna de las dos puntas tiene lcd.cpp ni lcd.h, y ningun otro fichero de "
        "src ha heredado drawStr, sendBuffer ni el objeto u8g2: la pantalla esta "
        "retirada del firmware y no se ha mudado a otro sitio (D-32 (1))",
        "QUEDA PANTALLA EN EL FIRMWARE: %s. D-32 (1) la retira; si vuelve, vuelve con "
        "ella el reloj SPI en el conector del ESP32 que este pack existe para "
        "impedir, y hay que rehacer la medida de J17 antes de aceptarlo" % restos)

    # -- Controles negativos: la prueba sabe distinguir el caso malo --------------
    #
    # Se mutan copias en memoria de un fuente REAL, no bloques sinteticos: un control
    # negativo sobre texto inventado demuestra que el regex funciona sobre texto
    # inventado, que no es lo que hace falta saber. D-32 (1): el fuente real ya no
    # puede ser lcd.cpp -no existe-, asi que se muta main.cpp del Maestro, que es el
    # fichero al que de verdad volveria el dibujo si alguien lo reintrodujera.
    cod_m = fw.codigo("Maestro", "src", "main.cpp")

    mutado = cod_m + "\nvoid _falso() { pinMode(LCD_SCLK, OUTPUT); }\n"
    b.control_negativo(
        bool(_conducciones(mutado, set(MACROS) | PINES_J17)),
        "devolver un pinMode(LCD_SCLK, OUTPUT) se detecta")
    b.control_negativo(
        not _conducciones(cod_m, set(MACROS) | PINES_J17),
        "y el mismo fichero SIN mutar no lo dispara: la prueba 2.1 distingue")

    # La 3.1 invertida tiene que saber ver una libreria que vuelve, Y tiene que saber
    # NO verla cuando solo esta nombrada en un comentario. Las dos mitades, porque el
    # segundo caso es el que rompio esta prueba el dia que se escribio.
    def _lib_deps(txt):
        mm = re.search(r"^lib_deps\s*=(.*?)(?=^\w|^\[|\Z)", txt, re.S | re.M)
        return re.sub(r";[^\n]*", " ", mm.group(1)) if mm else ""

    _ini_real = fw.texto("Maestro", "platformio.ini")
    # La libreria se reintroduce DENTRO del bloque lib_deps, que es donde volveria de
    # verdad: pegarla al final del fichero no la mete en el bloque y el control
    # negativo aprobaria por no encontrarla, que es el fallo contrario.
    _ini_mutado = re.sub(r"^lib_deps\s*=", "lib_deps =\n    olikraus/U8g2@^2.35.19",
                         _ini_real, count=1, flags=re.M)
    b.control_negativo(
        "U8g2" in _lib_deps(_ini_mutado) and "U8g2" not in _lib_deps(_ini_real),
        "reintroducir olikraus/U8g2 en lib_deps se detecta, y hoy no esta en lib_deps")
    b.control_negativo(
        "U8g2" in _ini_real and "U8g2" not in _lib_deps(_ini_real),
        "y un U8g2 que solo aparece en un COMENTARIO del .ini no cuenta como "
        "dependencia: es el patron ciego de CLAUDE.md 7.1, medido sobre el fichero real")

    # Y la 4.1 invertida tiene que saber ver el dibujo mudado a otro fichero, que es
    # la forma por la que la pantalla volveria sin recrear lcd.cpp.
    heredado = cod_m + "\nvoid _falso() { u8g2.drawStr(0, 0, \"x\"); u8g2.sendBuffer(); }\n"
    b.control_negativo(
        bool(re.search(r"\bdrawStr\b", heredado)) and
        not re.search(r"\bdrawStr\b", cod_m),
        "un drawStr mudado a main.cpp se detecta, y hoy main.cpp no lo tiene")
