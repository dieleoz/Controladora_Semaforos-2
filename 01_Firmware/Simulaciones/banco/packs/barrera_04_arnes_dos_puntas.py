# ===== banco/packs/barrera_04_arnes_dos_puntas.py =====
#
# EL VIGILANTE DEL UNICO INSTRUMENTO QUE EJECUTA LAS DOS PUNTAS A LA VEZ.
#
# El 31/08 una auditoria externa lo dejo por escrito y se verifico: NINGUN instrumento
# ejecutaba el C++ real de las dos puntas a la vez para comprobar que nunca dan verde
# las dos. Lo que cerraba ese lazo era una copia del firmware escrita a mano en Python.
#
# Ese instrumento existe desde hoy: Validacion_Automatico/dos_puntas/. Compila cada
# punta en su propia DLL -Maestro y Esclavo definen los MISMOS simbolos, asi que no
# pueden enlazarse juntos-, las carga las dos en el mismo proceso y les da el mismo
# millis(), de modo que "verde en las dos A LA VEZ" tiene por fin un instante donde
# mirarse. Lo mide sobre los pines que semaforo.cpp escribio, no sobre su logica.
#
# ESTE PACK NO REPITE ESA MEDIDA. Repetirla en Python seria volver al modelo escrito a
# mano que el arnes vino a retirar. Lo que vigila es lo otro, que es donde este
# repositorio se ha hecho dano tres veces: QUE EL ARNES NO SE QUEDE CORTO SIN QUE NADIE
# SE ENTERE.
#
# EL AGUJERO QUE CIERRA, dicho con el ejemplo que lo motiva:
#
#   Hoy hay SEIS ficheros en todo el firmware que pueden encender un verde. El arnes
#   compila cinco. Manana alguien anade un modo nuevo que llama a
#   semaforo_forzarVerde() y NO lo anade al arnes. El arnes sigue compilando, sigue
#   saliendo 42/42 y sigue diciendo "el C++ real de las dos puntas": lo que ya no dice
#   -y nadie preguntaria- es que hay un camino al verde que no recorre. Un ABORTADO al
#   menos grita; un HUECO no (N-43).
#
# ES UN TRINQUETE, NO UN ABSOLUTO (la forma de N-73). Exigir "el arnes compila TODO"
# seria falso: Maestro/src/modo_degradado.cpp esta fuera a proposito y con motivo
# escrito -arrastra lcd.h y menu.h, o sea u8g2 entero-. Lo que falla es un fichero
# NUEVO que gane la capacidad de encender un verde y se quede fuera en silencio, y
# tambien una exclusion RANCIA: una que siga en la lista cuando ya no hace falta,
# porque esa tapa un hueco que ya no existe y deja de tapar el que si.

import os
import re

# ESTE PACK NO LLEVA ETIQUETA "EJERCE SFTY-x", Y ES DELIBERADO.
#
# La tentacion era ponerle "EJERCE SFTY-2": al fin y al cabo vigila al instrumento que
# ejerce el enclavamiento en las dos puntas a la vez. Seria falso. Quien EJERCE SFTY-2
# aqui es el arnes en C++ -que corre el firmware y mira los pines-; este pack no
# enciende una luz ni evalua un enclavamiento: cuenta ficheros. Solo se etiqueta lo que
# el pack comprueba de verdad, porque una regla que aparece cubierta por una prueba que
# no la ejerce es peor que una fila vacia: la vacia no miente.
#
# Si SFTY-2 tiene que citar al arnes de las dos puntas en la tabla de trazabilidad de
# OPTIMIZACIONES.md, la fila apunta al ARNES -Validacion_Automatico/dos_puntas- y no a
# este fichero. Esa tabla se levanta buscando etiquetas en los packs, asi que hoy no
# tiene forma de citar un arnes en C++; darle de alta esa columna es una decision de
# quien lleva el documento, no un efecto colateral de este pack.

NOMBRE = "barrera_04_arnes_dos_puntas"
DESCRIPCION = "el arnes de las dos puntas cubre todo lo que puede encender un verde"

PUNTAS = ("Maestro", "Esclavo")

# Las tres puertas publicas al verde. Si alguna se renombra, el censo se queda ciego y
# este pack aborta en vez de aprobar: un patron que no encuentra nada NO demuestra que
# no haya nada (regla 4).
PUERTAS_AL_VERDE = (
    "semaforo_forzarVerde",
    "semaforo_iniciarTransicionAVerde",
    "semaforo_toggle",
)

# Ficheros que PUEDEN encender un verde y aun asi NO entran en el arnes, con el motivo.
# Cada uno tiene que seguir existiendo, seguir siendo capaz de encender un verde, y su
# motivo tiene que estar ESCRITO EN EL PROPIO ARNES: una exclusion que solo vive aqui es
# una decision que el que abra el arnes no puede ver.
EXCLUIDOS = {
    ("Maestro", "modo_degradado.cpp"):
        "incluye lcd.h y menu.h, o sea u8g2 y la pantalla entera. Consecuencia escrita "
        "en adaptador_maestro.cpp: el Modo Degradado del MAESTRO no se ejerce en este "
        "arnes -el del Esclavo si, y es el que crea las dos autoridades sobre la luz-",
}

# Sin estos tres, el arnes no mide la propiedad aunque compile y de verde.
IMPRESCINDIBLES = (
    ("Maestro", "semaforo.cpp"),
    ("Esclavo", "semaforo.cpp"),
    ("Esclavo", "main.cpp"),      # el despachador de radio: quien obedece un CMD_GO_GREEN
)

ARNES = ("Validacion_Automatico", "dos_puntas")
GUION = ("Validacion_Automatico", "compilar_dos_puntas.ps1")


def _lee(fw, *partes):
    p = os.path.join(fw.FIRMWARE, *partes)
    if not os.path.isfile(p):
        raise fw.Abortado("no existe %s. Sin el, este pack no puede medir si el arnes "
                          "de las dos puntas cubre lo que dice cubrir" %
                          os.path.join(*partes))
    with open(p, "r", encoding="utf-8", errors="replace") as f:
        return f.read()


def _enciende_verde(codigo):
    """True si este codigo puede encender un verde por alguna de las tres puertas."""
    return any(re.search(r"\b%s\s*\(" % re.escape(p), codigo) for p in PUERTAS_AL_VERDE)


def _censo_de_verdes(fw):
    """Los .cpp de cada punta que pueden encender un verde. Se censa el DIRECTORIO.

    Nunca una lista escrita a mano: esa se queda corta el dia que alguien anade un .cpp,
    y entonces la prueba aprueba sin haber mirado donde hacia falta."""
    fuera = []
    for punta in PUNTAS:
        for nombre in fw.fuentes_de(punta, "src"):
            if _enciende_verde(fw.codigo(punta, "src", nombre)):
                fuera.append((punta, nombre))
    return fuera


def correr(b, fw):
    b.titulo("El arnes de las dos puntas cubre todo lo que puede encender un verde")

    # ---- 1. El instrumento existe y su guion apunta a ficheros que existen ----
    dir_arnes = os.path.join(fw.FIRMWARE, *ARNES)
    b.verificar(
        os.path.isdir(dir_arnes),
        "existe Validacion_Automatico/dos_puntas: hay un instrumento que ejecuta el C++ "
        "real de las DOS puntas a la vez",
        "NO existe Validacion_Automatico/dos_puntas. Sin el, la propiedad mas cara del "
        "equipo -verde contra verde en un cierre de carril- vuelve a estar cubierta solo "
        "por una copia del firmware escrita a mano en Python")
    if not os.path.isdir(dir_arnes):
        return

    guion = _lee(fw, *GUION)

    # Los instrumentos leen el fuente por ruta: mover o renombrar un .cpp rompe uno.
    # Esta es la guarda de rutas del arnes nuevo (regla 5).
    rutas = re.findall(r"'src\\(\w+\.cpp)'", guion)
    b.verificar(
        len(rutas) >= 10,
        f"el guion del arnes nombra {len(rutas)} fuentes del firmware por ruta",
        f"solo se leyeron {len(rutas)} rutas del guion. Un censo casi vacio daria PASS "
        "sin mirar nada, que es la prueba muerta que este banco persigue")

    # El guion reparte las rutas en dos variables, una por punta, asi que aqui basta con
    # exigir que cada nombre exista en ALGUNA de las dos; el reparto correcto lo
    # comprueba el bloque 2, que mira dentro de la variable que toca.
    faltan = []
    for nombre in sorted(set(rutas)):
        if not any(fw.existe(p, "src", nombre) for p in PUNTAS):
            faltan.append(nombre)
    b.verificar(
        not faltan,
        "todos los fuentes que el guion del arnes compila siguen existiendo",
        f"el guion compila fuentes que ya no estan: {faltan}. Mover o renombrar un .cpp "
        "rompe un instrumento, y el movimiento y la actualizacion de rutas van en el "
        "MISMO commit")

    # ---- 2. Lo imprescindible entra de verdad ----
    for punta, nombre in IMPRESCINDIBLES:
        # Se busca el nombre en la lista de fuentes de esa punta dentro del guion.
        bloque = re.search(r"\$fuentes%s\s*=\s*@\((.*?)\)\s*\n" % punta.capitalize(),
                           guion, re.S)
        b.verificar(
            bloque is not None and nombre in bloque.group(1),
            f"el arnes compila {punta}/src/{nombre}",
            f"el arnes NO compila {punta}/src/{nombre}. Sin ese fichero no esta midiendo "
            "la propiedad: " +
            ("es el despachador de radio, donde se decide si esta punta obedece un "
             "CMD_GO_GREEN" if nombre == "main.cpp" else
             "es quien escribe los pines de luz de esa punta, y la medida se hace sobre "
             "lo que se escribio en ellos"))

    # ---- 3. EL TRINQUETE: nadie enciende un verde fuera del arnes en silencio ----
    censo = _censo_de_verdes(fw)
    b.verificar(
        len(censo) >= 4,
        f"el censo encontro {len(censo)} ficheros capaces de encender un verde "
        f"({', '.join('%s/%s' % c for c in censo)})",
        f"el censo solo encontro {len(censo)} ficheros. Las tres puertas al verde "
        f"({', '.join(PUERTAS_AL_VERDE)}) o se renombraron o el patron se quedo ciego; "
        "en cualquier caso este pack no puede medir nada y su PASS no valdria")

    huerfanos = []
    for punta, nombre in censo:
        bloque = re.search(r"\$fuentes%s\s*=\s*@\((.*?)\)\s*\n" % punta.capitalize(),
                           guion, re.S)
        compilado = bloque is not None and nombre in bloque.group(1)
        if not compilado and (punta, nombre) not in EXCLUIDOS:
            huerfanos.append("%s/src/%s" % (punta, nombre))
    b.verificar(
        not huerfanos,
        "ningun fichero capaz de encender un verde se quedo fuera del arnes sin "
        "declararlo: los que no entran estan en EXCLUIDOS con su motivo",
        f"HAY UN CAMINO AL VERDE QUE EL ARNES NO RECORRE: {huerfanos}. El arnes seguiria "
        "compilando, seguiria dando verde y seguiria diciendo 'el C++ real de las dos "
        "puntas'. Lo que ya no diria es que ese camino no lo mide nadie.\n"
        "        O se anade al guion, o se anade a EXCLUIDOS de este pack CON el motivo "
        "escrito tambien en el arnes. Lo que no vale es el silencio")

    # ---- 4. Y ninguna exclusion esta rancia ----
    # Una exclusion que sobra tapa un hueco que ya no existe y deja de tapar el que si.
    rancias = []
    for (punta, nombre), _motivo in EXCLUIDOS.items():
        if not fw.existe(punta, "src", nombre):
            rancias.append("%s/src/%s (ya no existe)" % (punta, nombre))
        elif not _enciende_verde(fw.codigo(punta, "src", nombre)):
            rancias.append("%s/src/%s (ya no enciende ningun verde)" % (punta, nombre))
        else:
            bloque = re.search(r"\$fuentes%s\s*=\s*@\((.*?)\)\s*\n" % punta.capitalize(),
                               guion, re.S)
            if bloque is not None and nombre in bloque.group(1):
                rancias.append("%s/src/%s (el arnes ya lo compila: sobra la excusa)" %
                               (punta, nombre))
    b.verificar(
        not rancias,
        f"las {len(EXCLUIDOS)} exclusiones declaradas siguen siendo ciertas y siguen "
        "haciendo falta",
        f"hay exclusiones RANCIAS: {rancias}. Una exclusion que sobra es peor que una "
        "fila vacia, porque la vacia no miente")

    # ---- 5. El motivo de cada exclusion vive TAMBIEN dentro del arnes ----
    # Una decision que solo vive en el banco no la ve quien abre el arnes para ampliarlo.
    adaptadores = ""
    for n in ("adaptador_maestro.cpp", "adaptador_esclavo.cpp", "orquestador.cpp"):
        adaptadores += _lee(fw, *ARNES, n)
    sin_explicar = [f"{p}/src/{n}" for (p, n) in EXCLUIDOS if n not in adaptadores]
    b.verificar(
        not sin_explicar,
        "el motivo de cada exclusion esta escrito DENTRO del arnes, no solo aqui: quien "
        "lo abra para ampliarlo ve por que ese fichero no entra",
        f"estas exclusiones no se mencionan en el arnes: {sin_explicar}. Una decision "
        "que solo vive en el banco es invisible para quien toca el arnes")

    # ---- 6. El arnes esta EN LA COMPUERTA ----
    # Un instrumento que no esta en la compuerta no mide nada, y no deja rastro de que
    # falta. Es la leccion de N-43, y es la unica de este pack que no habla del arnes
    # sino de quien lo llama.
    compuerta = _lee(fw, "compuerta.py")
    b.verificar(
        "compilar_dos_puntas.ps1" in compuerta,
        "compuerta.py llama al arnes de las dos puntas: lo que mide entra en el acta",
        "compuerta.py NO llama a compilar_dos_puntas.ps1. El arnes corre a mano y no deja "
        "rastro en ninguna acta: verde contra verde vuelve a no vigilarlo nadie en la "
        "verificacion oficial. Se cierra dando de alta la suite igual que las otras:\n"
        "        d = os.path.join(RAIZ, 'Validacion_Automatico')\n"
        "        subprocess.run([... '-File', os.path.join(d, 'compilar_dos_puntas.ps1')])\n"
        "        anotar('arnes de las dos puntas', PASS si returncode==0, la linea RESULTADO)")

    # ---- 7. El arnes relee sus constantes del C++, sin valor por defecto ----
    #
    # Los escenarios de perdida de radio se dimensionan con el techo de silencio de
    # SFTY-6 y con el presupuesto de reintentos del ciclo. Si esos numeros estuvieran
    # escritos a mano, el dia que el firmware los cambiara -N-71 los cambio: 12 s a 25 s-
    # el arnes seguiria esperando lo viejo y saliendo en verde sin haber llegado a ver la
    # rendicion a ambar.
    #
    # Se comprueba en las DOS direcciones. La positiva -que los lea- es la que importa;
    # la negativa -que el valor no aparezca en la misma linea que la constante- es la que
    # caza el "lo leo y ademas lo escribo por si acaso". Un literal suelto igual a 25000
    # en otro sitio NO se acusa: seria acusar a una coincidencia, y una prueba que acusa
    # coincidencias se acaba desactivando.
    orq = _lee(fw, *ARNES, "orquestador.cpp")
    CONSTANTES = (
        (("Maestro", "include", "protocolo.h"),
         r"#define\s+SFTY6_SILENCIO_MS\s+(\d+)UL", "SFTY6_SILENCIO_MS"),
        (("Maestro", "src", "coordinador.cpp"),
         r"TIMEOUT_ACK_MS\s*=\s*(\d+)", "TIMEOUT_ACK_MS"),
        (("Maestro", "src", "coordinador.cpp"),
         r"CICLO_MAX_REINTENTOS\s*=\s*(\d+)", "CICLO_MAX_REINTENTOS"),
    )
    for partes, patron, nombre in CONSTANTES:
        valor = fw.constante(partes, patron, "la constante %s" % nombre)
        lee = re.search(r"leerNumero\([^;]*%s" % re.escape(nombre), orq, re.S) is not None
        b.verificar(
            lee,
            f"el orquestador RELEE {nombre} del C++ real (hoy vale {valor})",
            f"el orquestador no relee {nombre} del C++. Sin ese numero dimensionaria los "
            "escenarios de perdida de radio con otra cosa que el firmware, y seguiria "
            "dando un veredicto")
        mal = [l.strip() for l in orq.splitlines()
               if nombre in l and re.search(r"\b%d\b" % valor, l)]
        b.verificar(
            not mal,
            f"y no lleva su valor ({valor}) escrito al lado del nombre",
            f"{nombre} aparece con su valor literal al lado: {mal}. Un numero copiado "
            "junto a la constante que dice releerse es el 'por si acaso' que sobrevive al "
            "cambio del firmware")

    # ---- CONTROLES NEGATIVOS ----
    b.control_negativo(
        _enciende_verde("void modo_nuevo_loop() { semaforo_forzarVerde(); }") and
        not _enciende_verde("void modo_nuevo_loop() { semaforo_forzarRojo(); }"),
        "el censo SI reconoce un fichero nuevo que llama a semaforo_forzarVerde(), y NO "
        "confunde con el a uno que solo fuerza rojo")

    b.control_negativo(
        not _enciende_verde(fw.codigo("Maestro", "src", "identidad.cpp")),
        "el censo NO marca como capaz de encender un verde a un fichero que no lo es "
        "(identidad.cpp): si marcara a todos, el trinquete no distinguiria nada")

    # El control negativo de la guarda de rutas va sobre un guion SINTETICO, no sobre el
    # real mutado. Con el real, retirar un fuente del guion apagaba tambien este control
    # -la mutacion ya no encontraba que sustituir- y una linea que no puede fallar sola
    # no es una comprobacion, es un adorno.
    guion_falso = ("$fuentesEsclavo = @(\n"
                   "    (Join-Path $ESCLAVO 'src\\semaforo.cpp'),\n"
                   "    (Join-Path $ESCLAVO 'src\\NO_EXISTE.cpp')\n"
                   ")\n")
    leidas = re.findall(r"'src\\(\w+\.cpp)'", guion_falso)
    b.control_negativo(
        "semaforo.cpp" in leidas and
        [n for n in leidas if not any(fw.existe(p, "src", n) for p in PUNTAS)] ==
        ["NO_EXISTE.cpp"],
        "la guarda de rutas del arnes SI senala un fuente que el guion nombra y que no "
        "existe, y NO senala al que si existe")
