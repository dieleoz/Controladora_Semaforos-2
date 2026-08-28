# ===== banco/packs/identidad_01_serie.py =====
#
# LA SERIE DEL EQUIPO: QUE SALGA DEL SILICIO Y QUE LAS DOS PUNTAS LA DERIVEN IGUAL.
#
# La serie es la base del emparejamiento: el Esclavo acabara guardando la de SU Maestro
# y descartando toda trama que no la lleve. Si el Maestro derivara su serie de una
# forma y el Esclavo comprobara con otra, no casarian NUNCA y las dos puntas se
# quedarian en ambar sin que nadie entendiera por que. Por eso identidad.h y su .cpp
# son contrato compartido -costura_01 los compara byte a byte- y por eso este pack
# mira ADEMAS que la fuente sea la correcta.
#
# ESTE PACK YA CORRIGIO EL DISENO UNA VEZ, Y CONVIENE QUE QUEDE ESCRITO.
#
# La primera version de la serie era de 16 bits. El barrido midio 88 colisiones entre
# 4.096 chips vecinos de una misma oblea y el pack FALLO. La tentacion inmediata fue
# relajar el barrido hasta que pasara; la cuenta dijo otra cosa:
#
#     colisiones esperadas por puro azar = N^2 / (2*M)
#     N=4.096 codigos en M=65.536 huecos -> 128 esperadas, y se midieron 88
#
# El mezclado iba MEJOR que el azar. Lo estrecho era el ANCHO, no la funcion. Y como
# ninguna funcion puede batir esa cota, exigir cero colisiones a 16 bits habria sido
# una comprobacion que NINGUN firmware puede aprobar -lo que CLAUDE.md llama una nota
# disfrazada de prueba-. Se amplio la serie a 24 bits y el barrido volvio a verde.
#
# De ahi la forma que tiene ahora la comprobacion 5: no pide cero colisiones en
# abstracto, pide que el mezclado NO SEA PEOR QUE EL AZAR. Esa si la puede aprobar un
# buen mezclado y la suspende uno malo, que es lo que se queria medir desde el principio.

import re

NOMBRE = "identidad_01_serie"
DESCRIPCION = "la serie sale del UID de silicio, nunca es 000000 y separa chips vecinos"

PUNTAS = ("Maestro", "Esclavo")

# Tamano de flota realista para el que se exige CERO colisiones. No es un numero
# bonito: es el orden de magnitud de equipos que esta casa puede tener a la vez.
FLOTA = 256


def _mezcla_del_cpp(fw):
    """El multiplicador se LEE del C++. Sin valor por defecto: si no esta, se reporta."""
    codigo = fw.codigo("Maestro", "src", "identidad.cpp")
    m = re.search(r"MEZCLA\s*=\s*(\d+)", codigo)
    return int(m.group(1)) if m else None


def _ancho_del_cpp(fw):
    """Los bits utiles se LEEN de la mascara del plegado, no se suponen."""
    codigo = fw.codigo("Maestro", "src", "identidad.cpp")
    m = re.search(r"&\s*0x(F+)UL", codigo)
    return len(m.group(1)) * 4 if m else None


def _xorshift_del_cpp(fw):
    """El desplazamiento interno se LEE del C++, como el multiplicador y el ancho."""
    codigo = fw.codigo("Maestro", "src", "identidad.cpp")
    m = re.search(r"h\s*\^=\s*h\s*>>\s*(\d+)", codigo)
    return int(m.group(1)) if m else None


def _serie(uid, mezcla, ancho, shift):
    """Espejo Python de identidad_serie(). Aritmetica de 32 bits, como el C++.

    Si este espejo se desincroniza del C++ -por ejemplo, olvidando el xorshift- las
    medidas de abajo miden una funcion que no es la que corre en el micro. Por eso las
    TRES constantes se leen del fuente y ninguna esta escrita aqui.
    """
    h = 0
    for w in uid:
        h = ((h + w) * mezcla) & 0xFFFFFFFF
        h ^= h >> shift
    serie = (h ^ (h >> ancho)) & ((1 << ancho) - 1)
    return serie if serie != 0 else 1


def _uid_oblea(x, y):
    """UID plausible del F1: X/Y de la oblea en la palabra 0, lote y oblea en las otras."""
    return [(0x00280000) | (x << 8) | y, 0x53355130, 0x30393436]


def correr(b, fw):
    b.titulo("Serie del equipo derivada del UID de silicio")

    # ---- 1. La fuente es el UID de fabrica, no algo escribible ----
    for punta in PUNTAS:
        codigo = fw.codigo(punta, "src", "identidad.cpp")
        b.verificar(
            "UID_BASE" in codigo,
            f"{punta}: la serie se lee de UID_BASE, la direccion del UID de 96 bits que "
            "define el CMSIS -no una constante a mano ni memoria reescribible-",
            f"{punta}: identidad.cpp no usa UID_BASE. Si la serie sale de otro sitio, o "
            "se puede reescribir -y deja de ser identidad-, o se escribio la direccion a "
            "mano y un cambio de familia de micro la deja leyendo memoria ajena")

    # ---- 2. Las constantes se leen del C++, sin valor por defecto ----
    mezcla = _mezcla_del_cpp(fw)
    ancho = _ancho_del_cpp(fw)
    shift = _xorshift_del_cpp(fw)
    b.verificar(
        mezcla is not None and ancho is not None and shift is not None,
        f"multiplicador, ancho y xorshift se leen de identidad.cpp: {mezcla}, "
        f"{ancho} bits, >>{shift}",
        "no se encuentran MEZCLA, la mascara del plegado o el xorshift en "
        "identidad.cpp. Un banco que cayera a un valor por defecto mediria su propia "
        "copia, no el firmware")
    if mezcla is None or ancho is None or shift is None:
        return

    b.verificar(
        mezcla % 2 == 1,
        f"el multiplicador ({mezcla}) es impar: uno par mete un cero por abajo en cada "
        "vuelta y tras tres vueltas ha perdido bits de entrada",
        f"el multiplicador ({mezcla}) es PAR y el mezclado degenera")

    # ---- 3. Nunca el codigo reservado ----
    ceros = [uid for uid in ([0, 0, 0], [0xFFFFFFFF] * 3)
             if _serie(uid, mezcla, ancho, shift) == 0]
    b.verificar(
        not ceros,
        "ni el UID todo a ceros ni todo a unos producen la serie reservada para "
        "'Esclavo sin matricular'",
        f"hay UIDs que producen el codigo reservado: {ceros}. Un equipo que se presentase "
        "asi se leeria como sin adoptar, que es lo contrario de tener identidad")

    # ---- 4. CERO colisiones en una flota realista ----
    vistas = {}
    for n in range(FLOTA):
        s = _serie(_uid_oblea(n // 64, n % 64), mezcla, ancho, shift)
        vistas.setdefault(s, []).append(n)
    choques = {s: v for s, v in vistas.items() if len(v) > 1}
    b.verificar(
        not choques,
        f"{FLOTA} chips vecinos de una misma oblea producen {len(vistas)} series "
        "distintas: CERO colisiones en una flota realista",
        f"COLISIONAN {len(choques)} series entre {FLOTA} chips vecinos: "
        f"{list(choques.items())[:3]}. Dos equipos comprados juntos tendrian la misma "
        "identidad y el emparejamiento no podria distinguirlos")

    # ---- 5. El mezclado no es PEOR que el azar ----
    # Formulada asi a proposito: exigir cero colisiones en un barrido grande seria pedir
    # lo imposible -ninguna funcion bate la cota del cumpleanos- y una comprobacion que
    # ningun firmware puede aprobar no es una comprobacion. Esta si la suspende un
    # mezclado malo, que es lo que se quiere medir.
    N = 4096
    vistas = {}
    for n in range(N):
        s = _serie(_uid_oblea(n // 64, n % 64), mezcla, ancho, shift)
        vistas.setdefault(s, 0)
        vistas[s] += 1
    medidas = sum(c - 1 for c in vistas.values() if c > 1)
    esperadas = N * N / (2 * (1 << ancho))
    b.verificar(
        medidas <= max(1.0, esperadas),
        f"barrido de {N} chips: {medidas} colisiones frente a {esperadas:.1f} que da el "
        "azar. El mezclado reparte al menos tan bien como una funcion ideal",
        f"barrido de {N} chips: {medidas} colisiones, PEOR que las {esperadas:.1f} del "
        "azar. El mezclado esta agrupando series y no repartiendolas")

    # ---- 6. Un solo bit de diferencia cambia medio codigo ----
    ref = _serie(_uid_oblea(7, 11), mezcla, ancho, shift)
    dist = []
    for bit in range(32):
        uid = _uid_oblea(7, 11)
        uid[0] ^= (1 << bit)
        dist.append(bin(ref ^ _serie(uid, mezcla, ancho, shift)).count("1"))
    media = sum(dist) / len(dist)
    ideal = ancho / 2.0
    b.verificar(
        ideal * 0.6 <= media <= ideal * 1.4,
        f"cambiar UN bit del UID cambia {media:.1f} de los {ancho} bits de la serie "
        f"(ideal {ideal:.0f}): el mezclado reparte, no copia",
        f"cambiar un bit cambia {media:.1f} bits de media, lejos del ideal {ideal:.0f}. "
        "El mezclado esta copiando la entrada y las series se agruparan por lote")

    # ---- CONTROL NEGATIVO ----
    # PRIMER INTENTO FALLIDO, anotado porque la leccion vale: se probo con un XOR plano
    # de las tres palabras, y el control negativo salio ROTO. El motivo es que en este
    # barrido solo varia la palabra 0, asi que un XOR plano sigue siendo inyectivo y no
    # colisiona: estaba "demostrando" que la prueba caza un caso que no era malo.
    #
    # Un mezclado defectuoso de verdad es el que PIERDE informacion. Quedarse con el
    # byte alto de la palabra 0 tira justo las coordenadas X/Y que distinguen a dos
    # chips vecinos, que es el fallo que este barrido existe para cazar.
    def _pierde_bits(uid):
        return ((uid[0] >> 24) & 0xFF) or 1

    vistas_malas = set(_pierde_bits(_uid_oblea(n // 64, n % 64)) for n in range(FLOTA))
    b.control_negativo(
        len(vistas_malas) < FLOTA,
        f"el barrido de flota caza un mezclado que pierde bits: {len(vistas_malas)} "
        f"series para {FLOTA} chips distintos")

    b.control_negativo(
        _serie([1, 2, 3], mezcla, ancho, shift) != _serie([1, 2, 4], mezcla, ancho, shift),
        "el espejo Python distingue dos UIDs que solo difieren en un bit")
