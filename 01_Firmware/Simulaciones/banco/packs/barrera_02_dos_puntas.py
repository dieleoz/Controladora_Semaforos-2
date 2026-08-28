# ===== banco/packs/barrera_02_dos_puntas.py =====
#
# LAS DOS PUNTAS TIENEN QUE TENER LA MISMA BARRERA.
#
# barrera_01 vigila que ningun fichero escriba un pin de luz fuera de semaforo.cpp.
# Este vigila lo otro, que faltaba: que semaforo.cpp DIGA LO MISMO en las dos puntas.
#
# POR QUE IMPORTA MAS DE LO QUE PARECE.
#
# Este sistema regula un carril alternado: cuando una punta esta en verde, la otra
# TIENE que estar en rojo. La unica barrera contra el verde simultaneo es el
# enclavamiento SFTY-2 de aplicarSalidas(). Si las dos puntas enclavan distinto, la
# barrera deja de ser una barrera y pasa a ser dos barreras parecidas -y "parecidas"
# no es una propiedad que sirva para nada en seguridad vial-.
#
# EL DEFECTO QUE MOTIVO ESTE PACK, encontrado el 26/08 comparando los dos ficheros:
#
#     if (rojo) {
#       verde = false;
#       amarillo = false; // Opcional: forzar amarillo apagado si rojo esta encendido...
#       // Wait, in S_ROJO_AMARILLO state, the code explicitly passes (HIGH, HIGH, LOW).
#       // So if rojo is HIGH, amarillo CAN be HIGH. But verde MUST be LOW.
#     } else if (verde) {
#       rojo = false;
#       // If verde is HIGH, amarillo usually is LOW, but let's just force rojo LOW.
#     }
#
# Estaba SOLO en el Esclavo, dentro del enclavamiento. Tres cosas a la vez:
#
#   1. `S_ROJO_AMARILLO` NO EXISTE. Los estados son S_ROJO, S_VERDE, S_AMARILLO y
#      S_FALLO. El comentario delibera sobre un estado inventado y, sobre esa premisa
#      falsa, anade una sentencia viva a la barrera.
#   2. Hoy no cambiaba el comportamiento -ninguna de las 8 llamadas pasa rojo y ambar
#      a la vez-, asi que era CODIGO MUERTO DENTRO DE UNA REGLA DE SEGURIDAD: no
#      falla, no se nota, y nadie lo mide.
#   3. Era una trampa a plazo. El dia que alguien anada una transicion rojo+ambar -que
#      es practica corriente y bien puede pedirla el auditor-, el Maestro la mostraria
#      y el Esclavo no, en silencio y sin que ninguna prueba se enterase.
#
# LA COMPARACION ES POR CODIGO, NO BYTE A BYTE.
#
# Al contrario que costura_01, aqui la identidad literal seria demasiado estricta: los
# comentarios de las dos puntas divergen a proposito -el del Esclavo menciona que quien
# espera el estado es "el Maestro por radio o el Modo Degradado"-, y eso esta bien. Se
# compara fw.codigo(), que llega sin comentarios, y normalizando espacios. Lo que tiene
# que ser identico es lo que el micro ejecuta.

import re

# EJERCE SFTY-2: que el enclavamiento sea EL MISMO en las dos puntas.

NOMBRE = "barrera_02_dos_puntas"
DESCRIPCION = "el enclavamiento SFTY-2 es identico en Maestro y Esclavo"

PUNTAS = ("Maestro", "Esclavo")

# Las funciones que forman la barrera. Si alguna se renombra, el pack aborta en vez de
# aprobar: un patron que no encuentra nada NO demuestra que no haya nada.
BARRERA = ("aplicarSalidas", "escribirPines")


def _cuerpo(codigo, nombre):
    """Extrae el cuerpo de una funcion, normalizando espacios."""
    m = re.search(r"\b%s\s*\([^)]*\)\s*\{" % re.escape(nombre), codigo)
    if not m:
        return None
    i = codigo.index("{", m.start())
    nivel, j = 0, i
    while j < len(codigo):
        if codigo[j] == "{":
            nivel += 1
        elif codigo[j] == "}":
            nivel -= 1
            if nivel == 0:
                break
        j += 1
    return re.sub(r"\s+", " ", codigo[i:j + 1]).strip()


def _llamadas(codigo):
    """Los argumentos de cada aplicarSalidas(...), normalizados a booleanos."""
    fuera = []
    for args in re.findall(r"aplicarSalidas\s*\(([^)]*)\)", codigo):
        piezas = [p.strip().upper() for p in args.split(",")]
        if len(piezas) != 3:
            continue
        norm = tuple("1" if p in ("HIGH", "TRUE") else
                     "0" if p in ("LOW", "FALSE") else p for p in piezas)
        fuera.append(norm)
    return fuera


def correr(b, fw):
    b.titulo("La barrera SFTY-2 es la misma en las dos puntas")

    codigos = {p: fw.codigo(p, "src", "semaforo.cpp") for p in PUNTAS}

    # ---- 1. Cada funcion de la barrera existe en las dos puntas ----
    cuerpos = {}
    for nombre in BARRERA:
        for punta in PUNTAS:
            c = _cuerpo(codigos[punta], nombre)
            cuerpos[(punta, nombre)] = c
            b.verificar(
                c is not None,
                f"{punta}: {nombre}() localizada en semaforo.cpp",
                f"{punta}: NO se encuentra {nombre}() en semaforo.cpp. O se renombro, o "
                "el patron se quedo ciego. En cualquier caso este pack no puede medir la "
                "barrera y su PASS no valdria nada")

    if any(v is None for v in cuerpos.values()):
        return

    # ---- 2. Y es IDENTICA entre puntas ----
    for nombre in BARRERA:
        m, e = cuerpos[("Maestro", nombre)], cuerpos[("Esclavo", nombre)]
        b.verificar(
            m == e,
            f"{nombre}() es identica en las dos puntas (comparado el codigo sin "
            f"comentarios, {len(m)} caracteres)",
            f"LA BARRERA DIVERGE en {nombre}(). Este sistema regula un carril alternado: "
            "cuando una punta esta en verde la otra TIENE que estar en rojo, y el "
            "enclavamiento es lo unico que lo impide. Dos enclavamientos parecidos no "
            "son un enclavamiento.\n"
            f"        Maestro: {m[:160]}\n"
            f"        Esclavo: {e[:160]}")

    # ---- 3. Las llamadas tambien son las mismas ----
    lm, le = _llamadas(codigos["Maestro"]), _llamadas(codigos["Esclavo"])
    b.verificar(
        len(lm) >= 6,
        f"se leyeron {len(lm)} llamadas a aplicarSalidas() en el Maestro",
        f"solo {len(lm)} llamadas halladas. Una lista casi vacia daria PASS sin mirar "
        "nada, que es la prueba muerta que este banco persigue")
    b.verificar(
        sorted(lm) == sorted(le),
        f"las {len(lm)} combinaciones que cada punta pide son las mismas: no hay un "
        "estado que una muestre y la otra no",
        f"LAS PUNTAS PIDEN COMBINACIONES DISTINTAS.\n"
        f"        solo en Maestro: {sorted(set(lm) - set(le))}\n"
        f"        solo en Esclavo: {sorted(set(le) - set(lm))}")

    # ---- 4. Nadie pide rojo y verde a la vez ----
    prohibidas = [c for c in lm + le if c[0] == "1" and c[2] == "1"]
    b.verificar(
        not prohibidas,
        "ninguna llamada pide rojo y verde a la vez: el enclavamiento no tiene que "
        "corregir a nadie, es la ultima red y no la primera",
        f"hay llamadas que piden rojo Y verde: {prohibidas}. El enclavamiento las "
        "corrige, pero que la logica de arriba lo intente es un defecto por si solo")

    # ---- 5. El enclavamiento hace lo que dice ----
    # No basta con que las dos copias sean iguales: podrian ser iguales y estar mal.
    cuerpo = cuerpos[("Maestro", "aplicarSalidas")]
    b.verificar(
        re.search(r"if\s*\(\s*rojo\s*\)\s*\{\s*verde\s*=\s*false", cuerpo) is not None,
        "con rojo pedido, el enclavamiento apaga el verde -el rojo siempre gana-",
        "el enclavamiento ya no apaga el verde cuando se pide rojo. Es la regla que "
        "impide el verde simultaneo en las dos puntas")
    b.verificar(
        re.search(r"if\s*\(\s*rojo\s*&&\s*verde\s*\)", cuerpo) is not None,
        "queda la segunda red: si aun asi llegaran rojo y verde juntos, gana el rojo",
        "desaparecio la comprobacion redundante rojo && verde. Era la red de la red")

    # ---- CONTROL NEGATIVO ----
    # El defecto real del 26/08, reinyectado sobre el texto: el pack tiene que verlo.
    mutado = cuerpos[("Esclavo", "aplicarSalidas")].replace(
        "verde = false;", "verde = false; amarillo = false;", 1)
    b.control_negativo(
        mutado != cuerpos[("Maestro", "aplicarSalidas")],
        "una sentencia de mas en el enclavamiento de una sola punta -el defecto real de "
        "N-61- rompe la comparacion")

    b.control_negativo(
        _llamadas("aplicarSalidas(HIGH, LOW, HIGH);") == [("1", "0", "1")],
        "el lector de llamadas normaliza HIGH/LOW y detectaria una que pida rojo y verde")
