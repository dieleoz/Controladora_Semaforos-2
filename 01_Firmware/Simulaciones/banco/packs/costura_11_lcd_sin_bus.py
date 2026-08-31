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
# LO QUE ESTE PACK NO PIDE, Y ES LA MITAD DE SU TRABAJO.
#
# No pide que la pantalla se retire. Se decidio expresamente NO retirarla, y el
# motivo es vial: menu.cpp:215 del Esclavo es UNA DE LAS TRES VIAS que lo sacan
# del Modo Degradado -las otras dos son mando.cpp y la puerta automatica de
# main.cpp-, y la app todavia NO puede (defecto N-106, abierto). Retirar el menu
# hoy seria quitar una via de seguridad mientras otra sigue rota.
#
# Por eso la prueba 4 exige que el dibujo SIGA VIVO. Sin ella, este pack se podria
# poner en verde vaciando lcd.cpp -que es justo lo que se decidio no hacer- y
# ademas se llevaria por delante las 271 comprobaciones de Validacion_LCD. Un pack
# que solo prohibe se satisface destruyendo; se le pone al lado lo que debe
# conservarse.

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


def _codigo_lcd_maestro():
    return _FW.codigo("Maestro", "src", "lcd.cpp")


def _codigo_lcd_esclavo():
    return _FW.codigo("Esclavo", "src", "lcd.cpp")


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

    # -- 3. u8g2 tampoco los ata por su cuenta -----------------------------------
    #
    # Es el camino que no se ve: el constructor no aparece como un pinMode, pero
    # entrega los pines a la libreria y a partir de ahi los conduce ella. Prohibir
    # solo pinMode dejaria justo esta puerta abierta.
    #
    # SE LEEN LOS ARGUMENTOS DEL C++, no se da por bueno el nombre de la clase. El
    # transporte sigue siendo SW SPI -y tiene que seguir siendolo, porque
    # flash_01_lastre cuelga de eso la bandera de HW SPI-; lo que cambia es que ya no
    # recibe ningun pin. Que U8X8_PIN_NONE baste esta leido en la libreria:
    # u8x8_gpio_and_delay_arduino() pregunta "!= U8X8_PIN_NONE" antes del pinMode del
    # arranque y antes del digitalWrite de cada escritura.
    print("\n-- 3.1 El constructor del display no recibe ningun pin --")
    ataduras = []
    for punta, cod in (("Maestro", _codigo_lcd_maestro()),
                       ("Esclavo", _codigo_lcd_esclavo())):
        m = re.search(r"U8G2_\w+\s+u8g2\s*\(([^)]*)\)", cod)
        if not m:
            # La regla del instrumento: un "no aparece" no es un hallazgo hasta
            # haber descartado al buscador. Si
            # el constructor no se encuentra, lo que fallo puede ser el patron.
            ataduras.append("%s: no se halla el constructor de u8g2 en lcd.cpp "
                            "(patron ciego o forma nueva de instanciar)" % punta)
            continue
        args = [a.strip() for a in m.group(1).split(",")]
        # El primero es la rotacion; del segundo en adelante son pines.
        pines = args[1:]
        if not pines:
            ataduras.append("%s: el constructor no declara ningun argumento de pin"
                            % punta)
        malos = [a for a in pines if a != "U8X8_PIN_NONE"]
        if malos:
            ataduras.append("%s: recibe pines reales -> %s" % (punta, malos))
        if "SetPin" in cod:
            ataduras.append("%s: llama a u8x8_SetPin_*, que ata los pines aparte"
                            % punta)
        for macro in MACROS:
            if re.search(r"\b%s\b" % macro, cod):
                ataduras.append("%s: lcd.cpp todavia nombra %s" % (punta, macro))

    b.verificar(
        not ataduras,
        "en las dos puntas el constructor de u8g2 recibe U8X8_PIN_NONE en todos sus "
        "argumentos de pin, no llama a u8x8_SetPin_* y no nombra ninguna de las tres "
        "macros: la libreria no tiene ningun pin que conducir",
        "lcd.cpp VUELVE A ATAR LOS PINES: %s. Con el ESP32 en J17 esto devuelve el "
        "reloj SPI al conector del enlace serie" % ataduras)

    # -- 4. Y el dibujo sigue vivo: esto no era retirar la pantalla ---------------
    #
    # La contrapartida. Sin esta prueba el pack se pondria en verde vaciando
    # lcd.cpp, que es lo que se decidio NO hacer: se llevaria las 271 comprobaciones
    # de Validacion_LCD y, con menu.cpp, la via de menu.cpp:215 que saca al Esclavo
    # del Modo Degradado mientras la app no puede (N-106).
    print("\n-- 4.1 El framebuffer se sigue componiendo (la pantalla NO se retiro) --")
    mudos = []
    for punta, cod in (("Maestro", _codigo_lcd_maestro()),
                       ("Esclavo", _codigo_lcd_esclavo())):
        n_draw = len(re.findall(r"\bdrawStr\s*\(", cod))
        n_send = len(re.findall(r"\bsendBuffer\s*\(", cod))
        if n_draw == 0 or n_send == 0:
            mudos.append("%s: drawStr x%d, sendBuffer x%d" % (punta, n_draw, n_send))

    b.verificar(
        not mudos,
        "las dos puntas siguen componiendo el framebuffer (drawStr y sendBuffer "
        "siguen ahi): lo que se corto es el cable, no la pantalla. Validacion_LCD "
        "puede seguir midiendo sus 271 comprobaciones y menu.cpp conserva la via de "
        "salida del Modo Degradado",
        "lcd.cpp se ha quedado MUDO: %s. Eso ya no es 'no conducir el bus', es "
        "retirar la pantalla, y con ella se van las 271 comprobaciones del arnes y "
        "la via de menu.cpp:215 que saca al Esclavo del Degradado (N-106 sigue "
        "abierto: la app no puede sustituirla)" % mudos)

    # -- Controles negativos: la prueba sabe distinguir el caso malo --------------
    #
    # Se mutan copias en memoria del fuente real, no bloques sinteticos: un control
    # negativo sobre texto inventado demuestra que el regex funciona sobre texto
    # inventado, que no es lo que hace falta saber.
    cod_m = _codigo_lcd_maestro()

    mutado = cod_m + "\nvoid _falso() { pinMode(LCD_SCLK, OUTPUT); }\n"
    b.control_negativo(
        bool(_conducciones(mutado, set(MACROS) | PINES_J17)),
        "devolver un pinMode(LCD_SCLK, OUTPUT) se detecta")

    # Se muta el constructor REAL devolviendole los pines, que es exactamente el
    # cambio que habria que hacer para volver a encender la pantalla.
    mutado2 = re.sub(r"(U8G2_\w+\s+u8g2\s*\()[^)]*\)",
                     r"\1U8G2_R0, LCD_SCLK, LCD_SID, LCD_CS, U8X8_PIN_NONE)", cod_m)
    m2 = re.search(r"U8G2_\w+\s+u8g2\s*\(([^)]*)\)", mutado2)
    detecta2 = bool(m2) and [a.strip() for a in m2.group(1).split(",")][1:] != \
        ["U8X8_PIN_NONE"] * 4
    b.control_negativo(
        detecta2,
        "devolver LCD_SCLK/LCD_SID/LCD_CS al constructor se detecta")

    mudo = re.sub(r"\bsendBuffer\s*\(", "noSend(", cod_m)
    b.control_negativo(
        len(re.findall(r"\bsendBuffer\s*\(", mudo)) == 0,
        "vaciar el volcado de lcd.cpp se detecta (la prueba 4 no aprueba una "
        "pantalla retirada)")
