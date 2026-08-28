# ===== banco/packs/costura_01_contratos.py =====
#
# LOS CONTRATOS COMPARTIDOS, BYTE A BYTE.
#
# Maestro y Esclavo son DOS proyectos PlatformIO separados, con copias FISICAS de
# los mismos ficheros. No hay enlace simbolico ni carpeta comun: lo unico que
# mantiene la igualdad es la disciplina de quien edita. Que HOY sean iguales no
# basta; esta prueba tiene que fallar el dia que dejen de serlo.
#
# ⚠️ ESTE PACK CAMBIA DE SIGNIFICADO CON lib/Common (Fase 5 del plan).
#
# Hoy la prueba es "las dos copias son identicas". Cuando estos ficheros se muevan
# a lib/Common ya NO HABRA DOS COPIAS que comparar, y esta prueba aprobaria vacia:
# comparando nada contra nada. En ese momento -en el MISMO commit que el
# movimiento- la prueba pasa a ser "NO existe copia local que tape a la comun".
# Ambas formas estan escritas abajo y el pack elige sola segun lo que encuentre.

NOMBRE = "costura_01_contratos"
DESCRIPCION = "identidad byte a byte de los ficheros compartidos entre puntas"

# (nombre, carpeta, fichero, por que importa)
COMPARTIDOS = [
    ("include/protocolo.h", "include", "protocolo.h",
     "los codigos de comando y el formato de trama"),
    ("include/ciclo_degradado.h", "include", "ciclo_degradado.h",
     "el calculo de la fase: la unica barrera contra el verde simultaneo"),
    ("include/respaldo.h", "include", "respaldo.h",
     "el contrato de lo que sobrevive al corte"),
    ("src/respaldo.cpp", "src", "respaldo.cpp",
     "el reparto de registros y la reconstruccion de antiguedad"),
    ("src/protocolo.cpp", "src", "protocolo.cpp",
     "el CRC, la rafaga y el filtro de repeticion"),
    ("include/identidad.h", "include", "identidad.h",
     "de donde sale la serie del equipo"),
    ("src/identidad.cpp", "src", "identidad.cpp",
     "el mezclado del UID: si cada punta derivara distinto, no casarian nunca"),
]


def correr(b, fw):
    hay_common = fw.existe("lib", "Common")

    if hay_common:
        _modo_biblioteca_comun(b, fw)
    else:
        _modo_dos_copias(b, fw)


def _modo_dos_copias(b, fw):
    """Antes de lib/Common: las dos copias tienen que ser identicas."""
    b.titulo("Contratos compartidos: identidad byte a byte entre proyectos")

    divergentes = []
    for nombre, carpeta, fichero, _porque in COMPARTIDOS:
        m = fw.existe("Maestro", carpeta, fichero)
        e = fw.existe("Esclavo", carpeta, fichero)
        if not m or not e:
            divergentes.append((nombre, "FALTA EN UN PROYECTO"))
            continue
        hm = fw.huella("Maestro", carpeta, fichero)
        he = fw.huella("Esclavo", carpeta, fichero)
        if hm != he:
            divergentes.append((nombre, f"{hm[:12]} vs {he[:12]}"))

    b.verificar(
        not divergentes,
        f"los {len(COMPARTIDOS)} ficheros que deben ser identicos lo son byte a byte "
        "(SHA-256 del contenido completo, no de una constante suelta)",
        f"DIVERGEN entre proyectos: {divergentes}. Dos contratos distintos para un "
        "mismo enlace: cada punta obedece reglas propias sin saberlo")

    # Se altera UN SOLO BYTE de una copia en memoria y se exige que la huella cambie.
    import hashlib
    original = open(fw.ruta("Maestro", "include", "ciclo_degradado.h"), "rb").read()
    mutado = bytearray(original)
    pos = original.find(b"2UL * ((uint32_t)verdeSeg")   # el corazon del calculo
    if pos < 0:
        pos = len(mutado) // 2
    mutado[pos] = (mutado[pos] + 1) % 256
    b.control_negativo(
        hashlib.sha256(original).hexdigest() != hashlib.sha256(bytes(mutado)).hexdigest()
        and len(original) > 1000,
        "un solo byte alterado en ciclo_degradado.h cambia la huella")

    # Si alguien anade un compartido nuevo y no lo mete en la lista, esto lo delata:
    # el requisito no puede vivir solo en la cabeza de quien escribio el fichero.
    import re
    declarados = 0
    for carpeta, fichero in (("include", "ciclo_degradado.h"), ("include", "respaldo.h")):
        if re.search(r"DEBEN?\s+SER\s+IDENTICOS?\s+EN\s+MAESTRO\s+Y\s+ESCLAVO",
                     fw.texto("Maestro", carpeta, fichero)):
            declarados += 1
    b.verificar(
        declarados == 2,
        "los propios ficheros declaran por escrito que deben ser identicos en las dos "
        "puntas: el requisito no vive solo en la cabeza de quien los escribio",
        "algun contrato compartido ya no declara su obligacion de ser identico")


def _modo_biblioteca_comun(b, fw):
    """Tras lib/Common: lo que hay que impedir es que reaparezca una copia local.

    La asimetria deja de evitarse comparando y pasa a ser imposible por
    construccion. Lo unico que puede romperla es que alguien deje un fichero local
    que, por el orden de busqueda de PlatformIO, TAPE al comun sin que nadie lo
    note. Eso es lo que se vigila ahora."""
    b.titulo("Contratos compartidos: viven en lib/Common y nadie los tapa")

    for nombre, carpeta, fichero, _porque in COMPARTIDOS:
        en_comun = (fw.existe("lib", "Common", carpeta, fichero)
                    or fw.existe("lib", "Common", fichero))
        b.verificar(
            en_comun,
            f"{nombre} vive en lib/Common: no puede divergir lo que se compila del "
            "mismo fuente",
            f"{nombre} NO esta en lib/Common y tampoco quedan las dos copias: no hay "
            "nada garantizando que las puntas compartan este contrato")

        tapado = [p for p in ("Maestro", "Esclavo")
                  if fw.existe(p, carpeta, fichero)]
        b.verificar(
            not tapado,
            f"{nombre} no tiene copia local en ninguna punta",
            f"{nombre} tiene copia local en {tapado}: por el orden de busqueda esa "
            "copia TAPA a la de lib/Common y la asimetria vuelve, ahora invisible")
