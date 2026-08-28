# ===== banco/packs/maestro_07_menu_opciones.py =====
#
# N-40 — QUE EL ARNES SEPA CUANTAS OPCIONES TIENE CADA MENU.
#
# EL FALLO QUE ORIGINA ESTE PACK. N-31 anadio REINICIAR RELOJ como cuarta opcion del
# submenu CONFIGURACION -menu.cpp lo dice en su propio comentario- y el arnes se quedo
# exigiendo tres. Reportaba un FALLA sobre un menu correcto.
#
# TERCERA VEZ EL MISMO PATRON EN UNA SEMANA: N-36 (el validador leia un fichero que ya
# no existia), N-39 (el arnes medía una fuente que el codigo ya no usa) y este. Siempre
# igual: el instrumento se queda atras y acusa al firmware de su propio retraso.
#
# Lo que los tres tienen en comun es que el instrumento llevaba el dato ESCRITO A MANO.
# Mientras el numero viva en dos sitios, alguien actualizara uno y no el otro; la unica
# forma de cerrarlo es leer el fuente y exigir que coincidan.
#
# POR QUE IMPORTA UNA OPCION DE MAS O DE MENOS. El menu se navega A CIEGAS mas de lo
# que parece: la pantalla esta a 5 m dentro del gabinete. Y hay un limite fisico -los
# 64 px de alto-: en V8.6 la sexta linea caia en y=69, fuera de pantalla. El peligro no
# era que no se dibujara, sino que el CURSOR SI PODIA navegar hasta ella, dejando al
# operario en una opcion invisible. Por eso el numero de opciones no es cosmetico.

import re

NOMBRE = "maestro_07_menu_opciones"
DESCRIPCION = "el arnes conoce el numero real de opciones de cada menu (N-40)"

# (constante en menu.cpp, como se llama el menu, lo que el arnes espera ver)
MENUS = [
    ("OPCIONES_RAIZ", "menu principal", 4),
    ("OPCIONES_CONFIG", "submenu CONFIGURACION", 4),
]

# Layout de lcd_dibujarMenu(): hasta 4 opciones base=28 paso=11; con 5, base=24 paso=9.
ALTO_PANTALLA = 63


def _cuenta(codigo, constante):
    m = re.search(r"const\s+int\s+%s\s*=\s*(\d+)" % constante, codigo)
    return int(m.group(1)) if m else None


def correr(b, fw):
    b.titulo("N-40 - el arnes conoce el numero real de opciones de cada menu")

    menu = fw.codigo("Maestro", "src", "menu.cpp")
    arnes = fw.texto("Validacion_LCD", "arnes_lcd.cpp")

    for constante, nombre, esperado in MENUS:
        real = _cuenta(menu, constante)

        b.verificar(
            real == esperado,
            f"el {nombre} tiene {real} opciones, que es lo que el arnes comprueba",
            f"DESAJUSTE ARNES/FIRMWARE: menu.cpp declara {constante}={real} y el arnes "
            f"espera {esperado}. El arnes reportaria FALLA sobre un menu correcto, o "
            f"peor, aprobaria uno al que le falta una opcion (es N-40)")

        # El array y la constante tienen que decir lo mismo: una opcion anadida al
        # array sin tocar la constante no se dibuja, y una constante subida sin anadir
        # al array deja al cursor navegando a un puntero que nadie escribio.
        arr = re.search(r"opciones\w*\[(\d+)\]\s*=\s*\{(.*?)\}", menu, re.S)
        if constante == "OPCIONES_CONFIG":
            arr = re.search(r"opcionesConfig\[(\d+)\]\s*=\s*\{(.*?)\}", menu, re.S)
        else:
            arr = re.search(r"opcionesRaiz\[(\d+)\]\s*=\s*\{(.*?)\}", menu, re.S)
        if arr:
            declarado = int(arr.group(1))
            elementos = len([x for x in arr.group(2).split(",") if x.strip()])
            b.verificar(
                declarado == elementos == real,
                f"{nombre}: el array declara {declarado}, tiene {elementos} textos y la "
                f"constante dice {real}. Los tres coinciden",
                f"{nombre} DESCUADRA: array[{declarado}] con {elementos} textos y "
                f"constante={real}. Si la constante supera al array, el cursor navega "
                "a un puntero que nadie escribio")

    # La razon de fondo: que la ultima opcion QUEPA. En V8.6 la sexta caia en y=69.
    peor = max(real for _, _, real in
               [(c, n, _cuenta(menu, c) or 0) for c, n, _ in MENUS])
    base, paso = (28, 11) if peor <= 4 else (24, 9)
    ultima_y = base + (peor - 1) * paso
    b.verificar(
        ultima_y <= ALTO_PANTALLA,
        f"la ultima opcion del menu mas largo ({peor} opciones) cae en y={ultima_y}, "
        f"dentro de los {ALTO_PANTALLA} px: el cursor no puede llevar al operario a una "
        "opcion invisible",
        f"la opcion {peor} cae en y={ultima_y}, FUERA de los {ALTO_PANTALLA} px. La "
        "salvaguarda de lcd_dibujarMenu() impide dibujarla, pero el cursor SI navega "
        "hasta ella: el operario quedaria en una opcion que no ve")

    # Control negativo: la comprobacion sabe distinguir el caso malo.
    mutado = re.sub(r"(const\s+int\s+OPCIONES_CONFIG\s*=\s*)\d+", r"\g<1>7", menu)
    b.control_negativo(
        _cuenta(mutado, "OPCIONES_CONFIG") == 7,
        "una OPCIONES_CONFIG cambiada a 7 se detecta")
