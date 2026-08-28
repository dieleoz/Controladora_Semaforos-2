# ===== banco/packs/maestro_06_fuentes_pantalla.py =====
#
# N-39 — QUE EL ARNES MIDA LA FUENTE QUE EL FIRMWARE USA DE VERDAD.
#
# EL FALLO QUE ORIGINA ESTE PACK. N-38 cambio los titulos de lcd.cpp de ncenB10 a
# 7x14B, y el arnes de pantalla se quedo con la ncenB10 escrita a mano. Durante dias
# midio una fuente que el firmware ya no usaba y reporto dos FALLA sobre codigo que
# estaba bien.
#
# ARREGLAR EL ARNES NO BASTA, y por eso existe este pack: si manana alguien vuelve a
# cambiar la fuente en lcd.cpp, el arnes volveria a medir la vieja y a mentir igual.
# Lo que cierra el agujero es comprobar que las DOS COSAS COINCIDEN, y eso solo se
# puede hacer leyendo el fuente.
#
# ES LA MISMA FAMILIA QUE N-36: alli el instrumento leia un fichero que ya no existia;
# aqui leia el fichero bueno con una suposicion vieja dentro. El instrumento tambien
# envejece, y nadie le pone fecha de caducidad.
#
# POR QUE 7x14B Y NO CUALQUIERA. La ncenB10 es PROPORCIONAL: contar caracteres no dice
# nada de su ancho, y fue justo el error de razonamiento de N-38 -contar a 6 px, que
# vale para la 6x10 de paso fijo-. La 7x14B es de paso fijo, asi que 12 caracteres por
# 7 px son 84 y el ancho vuelve a ser calculable a mano. Esa propiedad es la que se
# protege aqui, no la fuente concreta.

import re

NOMBRE = "maestro_06_fuentes_pantalla"
DESCRIPCION = "el arnes mide la misma fuente que lcd.cpp dibuja (N-39)"

# Titulos grandes de AJUSTAR HORA y la fuente con la que el arnes los mide.
TITULOS = ["SIN RELOJ", "ENVIANDO...", "SINCRONIZADA", "SOLO MAESTRO"]
FUENTE_ESPERADA = "u8g2_font_7x14B_tr"

# Fuentes de paso fijo: su ancho es contable y por eso se pueden revisar sin compilar.
PASO_FIJO = ("u8g2_font_5x7_tr", "u8g2_font_6x10_tr",
             "u8g2_font_7x14_tr", "u8g2_font_7x14B_tr")


def _fuente_activa(codigo, pos):
    """Ultima setFont() antes de la posicion dada: la que estaria puesta al dibujar."""
    ultima = None
    for m in re.finditer(r"setFont\(\s*(u8g2_font_\w+)\s*\)", codigo[:pos]):
        ultima = m.group(1)
    return ultima


def correr(b, fw):
    b.titulo("N-39 - el arnes mide la fuente que lcd.cpp dibuja de verdad")

    lcd = fw.codigo("Maestro", "src", "lcd.cpp")
    arnes = fw.texto("Validacion_LCD", "arnes_lcd.cpp")

    # 1. Cada titulo se dibuja con la fuente que el arnes dice medir.
    desajustes = []
    for t in TITULOS:
        m = re.search(r'drawStr\(\s*\d+\s*,\s*\d+\s*,\s*"%s"\s*\)' % re.escape(t), lcd)
        if not m:
            desajustes.append((t, "no se encuentra el drawStr en lcd.cpp"))
            continue
        f = _fuente_activa(lcd, m.start())
        if f != FUENTE_ESPERADA:
            desajustes.append((t, f"lcd.cpp lo dibuja en {f}"))

    b.verificar(
        not desajustes,
        f"los {len(TITULOS)} titulos de AJUSTAR HORA se dibujan en {FUENTE_ESPERADA}, "
        "que es la fuente con la que el arnes mide sus anchos",
        f"DESAJUSTE ARNES/FIRMWARE: {desajustes}. El arnes mediria una fuente que el "
        "codigo no usa, y su veredicto sobre esos anchos no valdria nada (es N-39)")

    # 2. El arnes declara medirlos con esa misma fuente.
    en_arnes = [t for t in TITULOS
                if re.search(r'\{\s*%s\s*,\s*"[^"]*"\s*,\s*"%s"' % (FUENTE_ESPERADA, re.escape(t)),
                             arnes)]
    b.verificar(
        len(en_arnes) == len(TITULOS),
        f"el arnes mide los {len(TITULOS)} titulos con {FUENTE_ESPERADA}: las dos "
        "puntas de la comprobacion hablan de lo mismo",
        f"el arnes NO mide con {FUENTE_ESPERADA} los titulos "
        f"{[t for t in TITULOS if t not in en_arnes]}: vuelve a medir una fuente que "
        "el firmware no usa")

    # 3. La propiedad que de verdad importa: que el ancho sea CONTABLE.
    b.verificar(
        FUENTE_ESPERADA in PASO_FIJO,
        f"{FUENTE_ESPERADA} es de paso fijo, asi que el ancho de un titulo se puede "
        "calcular contando caracteres y el recuento a mano vuelve a ser valido",
        f"{FUENTE_ESPERADA} es PROPORCIONAL: contar caracteres no dice nada de su "
        "ancho, que es el error de razonamiento exacto que provoco N-38")

    # 4. Control negativo: la comprobacion sabe distinguir el caso malo.
    #
    # Se muta POR POSICION y no sustituyendo una cadena literal. El primer intento
    # buscaba el texto exacto `setFont(...);\n  u8g2.drawStr(...)` y no acertaba nunca,
    # porque fw.codigo() quita los comentarios y con ellos cambia el espaciado: el
    # control negativo salia roto sin que hubiera nada roto. Un control negativo que
    # falla por su propia mecanica es peor que no tenerlo, porque manda a buscar un
    # defecto donde no lo hay.
    m = re.search(r'drawStr\(\s*\d+\s*,\s*\d+\s*,\s*"SINCRONIZADA"\s*\)', lcd)
    detecta = False
    if m:
        previos = list(re.finditer(r"setFont\(\s*u8g2_font_\w+\s*\)", lcd[:m.start()]))
        if previos:
            u = previos[-1]
            mutado = lcd[:u.start()] + "setFont(u8g2_font_ncenB10_tr)" + lcd[u.end():]
            m2 = re.search(r'drawStr\(\s*\d+\s*,\s*\d+\s*,\s*"SINCRONIZADA"\s*\)', mutado)
            detecta = m2 is not None and _fuente_activa(mutado, m2.start()) != FUENTE_ESPERADA
    b.control_negativo(
        detecta,
        "devolver SINCRONIZADA a la ncenB10 en lcd.cpp se detecta")
