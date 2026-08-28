# ===== banco/packs/costura_08_silencio.py =====
#
# EJERCE SFTY-6: las dos puntas se rinden a ambar con EL MISMO silencio.
#
# POR QUE UN NUMERO REPETIDO ES UN DEFECTO AUNQUE HOY COINCIDA.
#
# Hasta el 27/08 el umbral de silencio de radio estaba escrito TRES VECES como
# literal: Esclavo/src/main.cpp, y Maestro/src/coordinador.cpp dos veces. Los tres
# decian 12000, asi que "funcionaba".
#
# Pero ese numero gobierna las dos puntas de una regla de seguridad: cuanto silencio
# se aguanta antes de rendirse a ambar intermitente. Con tres copias a mano, el dia
# que alguien ajuste una y olvide otra, el Maestro y el Esclavo se rinden en
# instantes distintos y NADIE SE ENTERA: no hay error de compilacion, no hay aviso,
# y en la calle son unos segundos en los que una punta ya esta en ambar y la otra
# sigue el ciclo. Es la misma forma del defecto que costo N-49.
#
# Ahora vive una sola vez, en protocolo.h -el contrato que costura_01 ya compara byte
# a byte entre puntas-. Este pack vigila que siga siendo asi: que exista, que las dos
# puntas lo declaren igual, y sobre todo QUE NADIE VUELVA A ESCRIBIR EL NUMERO A MANO.

import re

NOMBRE = "costura_08_silencio"
DESCRIPCION = "el umbral de silencio de SFTY-6 vive una sola vez y las dos puntas usan el mismo"

PUNTAS = ("Maestro", "Esclavo")

# Los ficheros donde el umbral se USA. No se escriben a mano: se censa el directorio.
NOMBRE_CTE = "SFTY6_SILENCIO_MS"


def _valor(fw, punta):
    """El numero, leido del C++. Sin valor por defecto: si no esta, ABORTA."""
    texto = fw.texto(punta, "include", "protocolo.h")
    m = re.search(r"#define\s+%s\s+(\d+)UL" % NOMBRE_CTE, texto)
    return int(m.group(1)) if m else None


def correr(b, fw):
    b.titulo("SFTY-6: un solo numero para el silencio de radio")

    valores = {p: _valor(fw, p) for p in PUNTAS}
    if any(v is None for v in valores.values()):
        raise fw.Abortado(
            "no se encuentra %s en el protocolo.h de %s. O cambio de nombre o se "
            "retiro, y en los dos casos este pack no esta midiendo el umbral que "
            "gobierna la rendicion a ambar" % (
                NOMBRE_CTE, ", ".join(p for p, v in valores.items() if v is None)))

    # ---- 1. Las dos puntas, el mismo numero ----
    b.verificar(
        valores["Maestro"] == valores["Esclavo"],
        "las dos puntas se rinden con el mismo silencio: %d ms" % valores["Maestro"],
        "el Maestro aguanta %d ms de silencio y el Esclavo %d. Durante la diferencia, "
        "una punta esta en ambar intermitente y la otra sigue el ciclo: el cruce queda "
        "con dos criterios distintos y nadie lo ve" % (valores["Maestro"], valores["Esclavo"]))

    # ---- 2. Y NADIE lo escribe a mano ----
    # Esta es la comprobacion que impide que el defecto vuelva. No basta con que hoy
    # exista la constante: basta con que alguien escriba 12000 en un sitio nuevo para
    # que las dos copias empiecen a divergir otra vez.
    reincidentes = []
    for punta in PUNTAS:
        for fichero in fw.fuentes_de(punta, "src"):
            codigo = fw.codigo(punta, "src", fichero)   # sin comentarios
            # OJO CON LO QUE SE BUSCA: ESTA COMPROBACION YA SE EQUIVOCO UNA VEZ.
            #
            # La primera version buscaba cualquier 12000 y acuso a mando.cpp de las
            # dos puntas. Al mirarlo: es VENTANA_TRIPLE_MS = 12000, la ventana para
            # completar una secuencia del mando de reles. MISMO NUMERO, OTRO
            # SIGNIFICADO -y ademas con nombre, que es la practica correcta-.
            #
            # Lo que hay que cazar no es el numero: es el numero DESNUDO EN UNA
            # COMPARACION, que es la forma exacta que tenia el defecto. Un
            # "static const ... = 12000" con nombre no es deuda; un "t > 12000" si.
            if re.search(r"[<>]=?\s*12000\b", codigo):
                reincidentes.append("%s/src/%s" % (punta, fichero))
    b.verificar(
        not reincidentes,
        "ningun .cpp de las dos puntas escribe el umbral a mano: todos usan %s"
        % NOMBRE_CTE,
        "vuelve a haber un 12000 escrito a mano en %s. Ese numero gobierna las DOS "
        "puntas: en cuanto hay dos copias, una se queda atras el dia que alguien "
        "ajuste la otra" % ", ".join(sorted(set(reincidentes))))

    # ---- 3. Que de verdad se use donde importa ----
    usos = 0
    for punta in PUNTAS:
        for fichero in fw.fuentes_de(punta, "src"):
            usos += len(re.findall(NOMBRE_CTE, fw.codigo(punta, "src", fichero)))
    b.verificar(
        usos >= 3,
        "la constante se usa en %d sitios de las dos puntas -donde antes habia tres "
        "literales-" % usos,
        "la constante solo aparece %d vez/veces en los .cpp. Si se declaro pero no se "
        "usa, el umbral real sigue escondido en otro sitio y este pack estaria "
        "vigilando una decoracion" % usos)

    # ---- 4. Y que el valor tenga sentido operativo ----
    # No se juzga el numero exacto -es una decision de operacion- pero si su orden de
    # magnitud: por debajo de unos segundos, una racha de ruido apaga el cruce; por
    # encima de medio minuto, una punta sigue ciclando con la otra muerta demasiado
    # tiempo. Los limites son anchos a proposito: esto detecta un cero de mas o de
    # menos, no discute la politica.
    v = valores["Maestro"]
    b.verificar(
        5000 <= v <= 30000,
        "el umbral (%d ms) esta en el orden de magnitud razonable: aguanta rachas de "
        "ruido y no deja a una punta ciclando sola mas de medio minuto" % v,
        "el umbral es %d ms. Por debajo de 5 s una racha de ruido apaga el cruce; por "
        "encima de 30 s una punta sigue dando paso con la otra ya muerta" % v)

    # ---- 5. Controles negativos ----
    b.control_negativo(
        re.search(r"#define\s+%s\s+(\d+)UL" % NOMBRE_CTE, "#define OTRA_COSA 5UL") is None,
        "el lector de la constante no acepta cualquier #define que se le parezca")
    b.control_negativo(
        bool(re.search(r"[<>]=?\s*12000\b", "if (t > 12000) {")) and
        not re.search(r"[<>]=?\s*12000\b", "if (t > SFTY6_SILENCIO_MS) {"),
        "el detector distingue el numero desnudo en una comparacion de la constante")
    b.control_negativo(
        not re.search(r"[<>]=?\s*12000\b",
                      "static const unsigned long VENTANA_TRIPLE_MS = 12000;"),
        "y NO acusa a una constante con nombre que valga lo mismo: mando.cpp usa 12000 "
        "para la ventana de secuencia del mando, que es otra cosa")
