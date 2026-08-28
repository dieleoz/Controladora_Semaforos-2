# ===== banco/packs/barrera_03_talanquera.py =====
#
# EJERCE SFTY-28: la pluma sale por la misma puerta que las luces y sigue al verde.
#
# POR QUE UNA BARRERA FISICA ES MAS PELIGROSA QUE UNA LAMPARA.
#
# Una luz roja que alguien se salta produce un susto. Una talanquera ARRIBA con la luz
# en rojo produce otra cosa: el conductor **le hace mas caso a la barrera que a la
# lampara** -por eso se pone-, asi que una pluma levantada es una invitacion activa a
# entrar en el tramo. Por eso SFTY-28 dice que la talanquera SIGUE al semaforo, nunca
# lo manda y nunca lo contradice, y por eso la orden sale de semaforo.cpp y de ningun
# otro sitio: es la regla 6 de CLAUDE.md extendida a la barrera fisica.
#
# QUE MIDE ESTE PACK Y QUE MIDE EL ARNES, QUE NO ES LO MISMO.
#
# El arnes del automatico compila semaforo.cpp DE VERDAD y vigila, en cada tick de los
# nueve bloques, que el pin de la pluma nunca este en ABRIR con los dos verdes
# apagados. Eso es comportamiento.
#
# Este pack mira la ESTRUCTURA, que el arnes no puede ver: que no exista otra escritura
# del pin en ningun otro fichero, que la que hay este dentro de escribirPines() -y no
# en una funcion que alguien pueda llamar sola-, y que el arranque la deje cerrada. Un
# fallo de estructura no se manifiesta hasta que alguien anade el camino que lo usa, y
# entonces ya es tarde: es exactamente la trampa a plazo de N-61.

import re

NOMBRE = "barrera_03_talanquera"
DESCRIPCION = "la talanquera solo se escribe en escribirPines(), sigue al verde y arranca cerrada"

PUNTAS = ("Maestro", "Esclavo")

# Ficheros donde NO puede aparecer una escritura de la pluma. No se listan a mano: se
# censa el directorio entero de cada punta y se exceptua semaforo.cpp.
EXCEPTO = "semaforo.cpp"


def _cuerpo_de(codigo, firma):
    """El cuerpo de una funcion, contando llaves. Sobre codigo SIN comentarios."""
    i = codigo.find(firma)
    if i < 0:
        return None
    j = codigo.find("{", i)
    if j < 0:
        return None
    nivel = 0
    for k in range(j, len(codigo)):
        if codigo[k] == "{":
            nivel += 1
        elif codigo[k] == "}":
            nivel -= 1
            if nivel == 0:
                return codigo[j:k + 1]
    return None


def correr(b, fw):
    b.titulo("SFTY-28: la pluma, por la misma puerta que las luces")

    # ---- 1. Los niveles se LEEN del C++, sin valor por defecto ----
    for punta in PUNTAS:
        texto = fw.texto(punta, "include", "pines.h")
        abrir = re.search(r"#define\s+TALANQUERA_ABRIR\s+(\w+)", texto)
        cerrar = re.search(r"#define\s+TALANQUERA_CERRAR\s+(\w+)", texto)
        if not abrir or not cerrar:
            raise fw.Abortado(
                "%s: no se pueden leer TALANQUERA_ABRIR/CERRAR de pines.h. Sin esos "
                "niveles este pack no sabe cual es 'arriba', y compararia contra un "
                "valor inventado" % punta)
        b.verificar(
            cerrar.group(1) == "LOW",
            "%s: el nivel de CERRAR es LOW, que es el de reposo del MOSFET: si el "
            "equipo se apaga o el pin queda flotando, la pluma NO se queda arriba"
            % punta,
            "%s: TALANQUERA_CERRAR es %s. El reposo del hardware tiene que cerrar la "
            "barrera; si cerrar exigiera energia, un equipo muerto dejaria la via "
            "abierta sin regulacion" % (punta, cerrar.group(1)))
        b.verificar(
            abrir.group(1) != cerrar.group(1),
            "%s: abrir y cerrar son niveles distintos" % punta,
            "%s: TALANQUERA_ABRIR y TALANQUERA_CERRAR valen lo mismo (%s): la pluma "
            "no podria distinguir una orden de la otra" % (punta, abrir.group(1)))

    # ---- 2. UNA SOLA PUERTA: nadie mas escribe el pin ----
    for punta in PUNTAS:
        intrusos = []
        for fichero in fw.fuentes_de(punta, "src"):
            if fichero == EXCEPTO:
                continue
            codigo = fw.codigo(punta, "src", fichero)
            if re.search(r"digitalWrite\s*\(\s*MOTOR_TALANQUERA", codigo):
                intrusos.append(fichero)
        b.verificar(
            not intrusos,
            "%s: la talanquera solo se escribe desde semaforo.cpp -censados %d "
            "ficheros de src/-" % (punta, len(fw.fuentes_de(punta, "src"))),
            "%s: %s escribe(n) MOTOR_TALANQUERA por su cuenta. Un modo que mueve la "
            "barrera por libre es una pluma arriba con la luz en rojo esperando a que "
            "coincidan los tiempos" % (punta, ", ".join(intrusos)))

    # ---- 3. Y dentro de escribirPines(), no en una funcion suelta ----
    for punta in PUNTAS:
        codigo = fw.codigo(punta, "src", "semaforo.cpp")
        cuerpo = _cuerpo_de(codigo, "void escribirPines(")
        if cuerpo is None:
            raise fw.Abortado(
                "%s: no se encuentra el cuerpo de escribirPines() en semaforo.cpp. O "
                "cambio de nombre o cambio de forma, y en los dos casos este pack no "
                "esta midiendo la barrera" % punta)
        b.verificar(
            "digitalWrite(MOTOR_TALANQUERA" in cuerpo.replace(" ", ""),
            "%s: la orden de la pluma vive DENTRO de escribirPines(), junto a las seis "
            "lamparas" % punta,
            "%s: escribirPines() no toca la talanquera. Si la orden vive fuera, puede "
            "ejecutarse sin que las luces cambien -y al reves-, que es justo lo que "
            "SFTY-28 prohibe" % punta)
        m = re.search(r"digitalWrite\(\s*MOTOR_TALANQUERA\s*,\s*([^;]+)\)",
                      cuerpo.replace("\n", " "))
        expr = m.group(1).replace(" ", "") if m else ""
        b.verificar(
            m is not None and "verde" in expr and "rojo" not in expr,
            "%s: la pluma se decide con el MISMO 'verde' ya enclavado que enciende la "
            "lampara, no con otra variable" % punta,
            "%s: la talanquera se escribe con %r. Tiene que salir del mismo booleano "
            "que la luz verde: cualquier otra fuente puede desincronizarse de ella"
            % (punta, expr or "(no se pudo leer)"))
        # La excepcion de SFTY-6 se comprueba PRESENTE, no se da por supuesta. Si
        # alguien la borra, la pluma dejaria de subir en ambar intermitente y nadie lo
        # notaria hasta que un equipo se quedara sin enlace en obra -que es cuando
        # menos se puede descubrir-. La politica es del cliente (27/08: ARRIBA), pero
        # que el codigo la siga diciendo es del banco.
        b.verificar(
            "estado==S_FALLO" in expr,
            "%s: la pluma sube tambien en S_FALLO -el ambar intermitente de SFTY-6-, "
            "que es la politica que eligio el cliente" % punta,
            "%s: la orden de la pluma ya no contempla S_FALLO (%r). Con el equipo sin "
            "enlace la barrera se quedaria ABAJO, cerrando la via por completo, que es "
            "la politica CONTRARIA a la decidida" % (punta, expr))

    # ---- 4. El arranque la deja cerrada ----
    for punta in PUNTAS:
        codigo = fw.codigo(punta, "src", "semaforo.cpp")
        setup = _cuerpo_de(codigo, "void semaforo_setup(")
        if setup is None:
            raise fw.Abortado("%s: no se encuentra semaforo_setup()" % punta)
        plano = setup.replace(" ", "").replace("\n", "")
        b.verificar(
            "pinMode(MOTOR_TALANQUERA,OUTPUT)" in plano and
            "digitalWrite(MOTOR_TALANQUERA,TALANQUERA_CERRAR)" in plano,
            "%s: semaforo_setup() declara la pluma y la CIERRA antes de nada" % punta,
            "%s: el arranque no cierra la talanquera. Los dos segundos de bienvenida "
            "la dejarian en el estado en que quedo el pin -y tras un reinicio por "
            "watchdog, ese estado puede ser ABIERTA-" % punta)

    # ---- 5. Las dos puntas, igual ----
    cuerpos = [_cuerpo_de(fw.codigo(p, "src", "semaforo.cpp"), "void escribirPines(")
               for p in PUNTAS]
    b.verificar(
        cuerpos[0] == cuerpos[1],
        "las dos puntas escriben la pluma exactamente igual",
        "el escribirPines() del Maestro y el del Esclavo no coinciden. La barrera de "
        "salidas tiene que ser LA MISMA en las dos puntas: si difieren, un dia una "
        "levanta la pluma donde la otra no, y en silencio (N-61)")

    # ---- 6. Controles negativos ----
    b.control_negativo(
        _cuerpo_de("void escribirPines(bool a) { digitalWrite(X, a); }",
                   "void escribirPines(") == "{ digitalWrite(X, a); }",
        "el extractor de cuerpos devuelve la funcion entera y solo esa")
    b.control_negativo(
        bool(re.search(r"digitalWrite\s*\(\s*MOTOR_TALANQUERA",
                       "digitalWrite(MOTOR_TALANQUERA, HIGH);")) and
        not re.search(r"digitalWrite\s*\(\s*MOTOR_TALANQUERA",
                      "digitalWrite(OTRO_PIN, HIGH);"),
        "el detector de escrituras de la pluma distingue el pin de cualquier otro")
