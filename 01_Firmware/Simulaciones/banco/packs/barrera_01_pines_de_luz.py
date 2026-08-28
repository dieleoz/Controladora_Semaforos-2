# ===== banco/packs/barrera_01_pines_de_luz.py =====
#
# LA BARRERA DE SALIDAS: NINGUN PIN DE LUZ SE ESCRIBE FUERA DE semaforo.cpp.
#
# ES LA REGLA QUE UN AUDITOR VA A QUERER VER, y es la unica que puede impedir que un
# modo nuevo encienda una luz por su cuenta. Un verde dado por un camino que nadie
# vigila es exactamente el fallo que mata a alguien en un cruce.
#
# LA BARRERA YA EXISTIA, y conviene decirlo bien: semaforo.cpp concentra todas las
# salidas en un unico escribirPines() estatico, y los destellos del mando INTERCEPTAN
# las escrituras en vez de rodearlas -se hizo asi tras descartar lo obvio, ignorar las
# llamadas, que colgaba al coordinador esperando un S_VERDE que no llegaria-. Lo que
# faltaba no era construir la capa: era poder decir que NO TIENE EXCEPCIONES.
#
# TENIA UNA, y era codigo muerto: iniciarParpadeoFallo() en Maestro/src/main.cpp
# escribia ROJO1/ROJO2/VERDE1/VERDE2 con digitalWrite() directo. Era static y no la
# llamaba nadie -resto de la vieja maquina HANDSHAKE-, asi que la Fase 1 no fue mover
# codigo: fue borrarlo. El flash no cambio ni un byte, lo que confirma que el
# compilador ya la descartaba.
#
# LOS PINES SE DESCUBREN, NO SE ESCRIBEN AQUI.
#
# Esta es la leccion de N-36, N-39 y N-40, aplicada por adelantado: si la lista de
# pines viviera escrita a mano en este fichero, el dia que alguien anada un
# AMARILLO_PEATON la guarda seguiria dando PASS sin vigilarlo. Se leen de pines.h y se
# clasifican por nombre, asi que un pin de luz nuevo entra bajo custodia solo.
#
# ROJO_PEATON y VERDE_PEATON existen y estaban SIN CUSTODIA en la primera version de
# esta regla, que solo miraba las dos cabezas. Un verde peatonal fantasma es tan
# maniobra como cualquier otro.

import re

# EJERCE SFTY-2: el enclavamiento: ninguna luz se enciende fuera de semaforo.cpp.

NOMBRE = "barrera_01_pines_de_luz"
DESCRIPCION = "ningun pin de luz se escribe fuera de semaforo.cpp, en las dos puntas"

PUNTAS = ("Maestro", "Esclavo")

# Un pin es DE LUZ si su nombre lo dice. Se prefiere el nombre al numero porque es lo
# que el autor de pines.h controla, y porque asi un pin nuevo queda vigilado sin que
# nadie tenga que acordarse de venir aqui.
ES_LUZ = re.compile(r"^(ROJO|AMARILLO|VERDE)")

# Formas de tocar un pin directamente. digitalWrite es la habitual; las otras dos
# aparecen en codigo de bajo nivel y se vigilan por si alguien intenta el atajo.
ESCRITURAS = (r"digitalWrite\s*\(\s*%s\b", r"HAL_GPIO_WritePin\s*\([^)]*\b%s\b",
              r"analogWrite\s*\(\s*%s\b")

# Ficheros donde SI puede aparecer un pin de luz.
PERMITIDOS = ("semaforo.cpp",)


def _pines_de_luz(fw, punta):
    pines = re.findall(r"#define\s+([A-Z_][A-Z0-9_]*)\s+P[A-Z]\d+",
                       fw.texto(punta, "include", "pines.h"))
    return [p for p in pines if ES_LUZ.match(p)]


def _fuentes(fw, punta):
    import os
    base = os.path.join(fw.FIRMWARE, punta, "src")
    return sorted(f for f in os.listdir(base) if f.endswith(".cpp"))


def correr(b, fw):
    b.titulo("La barrera de salidas: solo semaforo.cpp escribe pines de luz")

    for punta in PUNTAS:
        luces = _pines_de_luz(fw, punta)

        b.verificar(
            len(luces) >= 6,
            f"{punta}: pines.h declara {len(luces)} pines de luz y todos entran bajo "
            f"custodia sin escribirlos aqui: {', '.join(luces)}",
            f"{punta}: solo se hallaron {len(luces)} pines de luz en pines.h "
            f"({luces}). O el fichero cambio de formato, o la guarda se quedo ciega: "
            "un patron que no encuentra nada NO demuestra que no haya nada")

        fugas = []
        for fichero in _fuentes(fw, punta):
            if fichero in PERMITIDOS:
                continue
            codigo = fw.codigo(punta, "src", fichero)   # sin comentarios
            for pin in luces:
                for forma in ESCRITURAS:
                    if re.search(forma % re.escape(pin), codigo):
                        fugas.append(f"{punta}/src/{fichero} escribe {pin}")

        b.verificar(
            not fugas,
            f"{punta}: ningun fichero fuera de semaforo.cpp escribe un pin de luz. La "
            "barrera no tiene excepciones",
            f"FUGA DE LA BARRERA en {punta}: {fugas}. Una sola escritura por fuera "
            "convierte el enclavamiento en decorativo: ese camino no pasa por "
            "escribirPines() y puede encender una luz que nadie autorizo")

    # CONTROL NEGATIVO. Sin esto, el dia que el patron dejara de casar -por un cambio
    # de formato en el fuente- la guarda aprobaria todo y nadie se enteraria. Es la
    # forma exacta en que se perdio la cobertura del Maestro en N-27.
    luces_m = _pines_de_luz(fw, "Maestro")
    mutado = fw.codigo("Maestro", "src", "main.cpp") + \
        "\nvoid _fuga_de_prueba() { digitalWrite(%s, HIGH); }\n" % luces_m[0]
    detecta = bool(re.search(ESCRITURAS[0] % re.escape(luces_m[0]), mutado))
    b.control_negativo(
        detecta,
        f"un digitalWrite({luces_m[0]}) colado en main.cpp se detecta")
