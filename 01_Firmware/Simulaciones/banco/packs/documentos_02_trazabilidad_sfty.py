# ===== banco/packs/documentos_02_trazabilidad_sfty.py =====
#
# LA TERCERA COLUMNA DE OPTIMIZACIONES.md, LEVANTADA DE VERDAD.
#
# La tabla de trazabilidad promete esto encima de si misma: "se levanta igual que las
# otras dos: BUSCANDO la etiqueta # EJERCE SFTY-x en banco/packs/, no escribiendola a
# mano". El 27/08 se comprobo y estaba escrita a mano: la fila de SFTY-2 citaba solo
# barrera_01_pines_de_luz cuando ya habia tres packs etiquetados con SFTY-2
# -barrera_02_dos_puntas y esclavo_06_no_abre_paso llevaban dias con su etiqueta-.
#
# POR QUE ESA COLUMNA IMPORTA MAS QUE LAS OTRAS DOS.
#
# "Donde vive" dice que la regla esta escrita. La tercera dice que algo la EJERCE, y
# es la que mira un auditor funcional para saber que se comprobo. Una fila vacia se
# ve de un vistazo y se lee como trabajo pendiente; una fila que cita menos packs de
# los que hay no se ve, y una que cita mas MIENTE -que es peor que la vacia, como el
# propio documento advierte-.
#
# LAS DOS DIRECCIONES, QUE ES LO QUE RETIRO LOS MONOLITOS.
#
# No basta con que cada etiqueta aparezca en la tabla: hace falta ademas que la tabla
# no cite nada que no tenga etiqueta. Es la misma regla con la que se retiraron los
# tres validadores monoliticos -"cero huerfanas en ninguna direccion"-, aplicada a un
# documento en vez de a un recuento de comprobaciones.

import os
import re

NOMBRE = "documentos_02_trazabilidad_sfty"
DESCRIPCION = "la tabla SFTY->pack se corresponde con las etiquetas # EJERCE, en las dos direcciones"

_AQUI = os.path.dirname(os.path.abspath(__file__))

_RE_ETIQUETA = re.compile(r"#\s*EJERCE\s+(SFTY-\d+)\s*:")
_RE_FILA = re.compile(r"^\|\s*\**\s*(SFTY-\d+)\s*\**\s*\|(.*)\|(.*)\|\s*$", re.M)
_RE_PACK = re.compile(r"`([a-z][a-z0-9_]{6,})`")


def _etiquetas():
    """regla -> {packs que la declaran}. Se lee del directorio, no de una lista."""
    fuera = {}
    for n in sorted(os.listdir(_AQUI)):
        if not n.endswith(".py") or n.startswith("_"):
            continue
        with open(os.path.join(_AQUI, n), "r", encoding="utf-8", errors="replace") as f:
            for regla in _RE_ETIQUETA.findall(f.read()):
                fuera.setdefault(regla, set()).add(n[:-3])
    return fuera


def _tabla(texto):
    """regla -> {packs citados en la tercera columna}."""
    fuera = {}
    for regla, _donde, demuestra in _RE_FILA.findall(texto):
        fuera[regla] = set(_RE_PACK.findall(demuestra))
    return fuera


def correr(b, fw):
    b.titulo("Trazabilidad SFTY: etiquetas del banco contra la tabla del documento")

    etiquetas = _etiquetas()
    texto = fw.texto_repo("OPTIMIZACIONES.md")
    tabla = _tabla(texto)

    # ---- 1. Descartar al buscador antes de acusar a nadie ----
    # Un "no aparece" no es un hallazgo hasta haber descartado al buscador: si el
    # censo no encuentra etiquetas o no encuentra tabla, el roto es este pack.
    if not etiquetas:
        raise fw.Abortado(
            "el censo no hallo ni una etiqueta '# EJERCE SFTY-x' en banco/packs: "
            "fallo el buscador, no los packs. Sin etiquetas, comparar la tabla "
            "contra un conjunto vacio la aprobaria entera")
    if len(tabla) < 10:
        raise fw.Abortado(
            "la tabla de trazabilidad de OPTIMIZACIONES.md solo dio %d filas: o "
            "cambio de formato o se movio, y en cualquiera de los dos casos esto no "
            "esta midiendo la tabla" % len(tabla))

    b.verificar(
        True,
        "censadas %d etiquetas '# EJERCE' en %d reglas, y %d filas de tabla"
        % (sum(len(v) for v in etiquetas.values()), len(etiquetas), len(tabla)),
        "no deberia llegarse aqui")

    # ---- 2. Cada regla etiquetada, con EXACTAMENTE sus packs en la tabla ----
    for regla in sorted(etiquetas, key=lambda r: int(r.split("-")[1])):
        citados = tabla.get(regla)
        b.verificar(
            citados == etiquetas[regla],
            "%s: la tabla cita exactamente los packs que la ejercen (%s)"
            % (regla, ", ".join(sorted(etiquetas[regla]))),
            "%s: la tabla cita {%s} y las etiquetas dicen {%s}. Faltan: {%s}. "
            "Sobran: {%s}. Una columna que se dice levantada del grep y no lo esta "
            "es una promesa de cobertura sin cobertura detras"
            % (regla,
               ", ".join(sorted(citados)) if citados else "-",
               ", ".join(sorted(etiquetas[regla])),
               ", ".join(sorted(etiquetas[regla] - (citados or set()))) or "-",
               ", ".join(sorted((citados or set()) - etiquetas[regla])) or "-"))

    # ---- 3. La direccion contraria: nada citado sin etiqueta ni sin fichero ----
    huerfanos = set()
    for regla, packs in tabla.items():
        huerfanos |= {p for p in packs if p not in etiquetas.get(regla, set())}
    b.verificar(
        not huerfanos,
        "ningun pack citado en la tabla carece de su etiqueta '# EJERCE'",
        "la tabla cita {%s} sin que esos packs declaren la regla. Etiquetar de mas "
        "convierte la tabla en un adorno: una regla que aparece cubierta por una "
        "prueba que no la comprueba es PEOR que una fila vacia"
        % ", ".join(sorted(huerfanos)))

    inexistentes = sorted(p for packs in tabla.values() for p in packs
                          if not os.path.isfile(os.path.join(_AQUI, p + ".py")))
    b.verificar(
        not inexistentes,
        "todos los packs que la tabla nombra existen en banco/packs",
        "la tabla nombra packs que ya no existen: %s. Es N-36 en un documento: el "
        "instrumento apuntando a algo que se movio" % ", ".join(inexistentes))

    # ---- 4. Controles negativos ----
    tabla_falsa = dict(tabla)
    alguna = sorted(etiquetas)[0]
    tabla_falsa[alguna] = set(tabla.get(alguna, set())) | {"pack_que_no_existe"}
    b.control_negativo(
        tabla_falsa[alguna] != etiquetas[alguna],
        "una fila con un pack de mas deja de coincidir con las etiquetas")

    b.control_negativo(
        _tabla("| SFTY-99 | `x.cpp` | -- |").get("SFTY-99") == set(),
        "una fila sin ningun pack se lee como fila VACIA, no como fila cubierta")
