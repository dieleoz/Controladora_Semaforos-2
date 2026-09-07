# -*- coding: utf-8 -*-
# POR QUE EXISTE ESTE PACK.
#
# El responsable lo dijo asi: "cambiamos varias veces las spec y el codigo arrastra cosas
# de una spec y de otra; lanzas subagentes y todos desarrollan con una version o con otra,
# y vamos y volvemos". DECISIONES.md corta ese bucle -una decision vigente vive en una
# fila- y el firmware se marco con anclas D-x, un comentario en el punto que la implementa.
# Eso vale mientras las dos mitades casen, y nada lo impedia: una decision nueva sin ancla
# no rompe ninguna compilacion, y un ancla de una DEROGADA se queda ahi para que el
# siguiente agente la lea como vigente.
#
# QUE MIDE, Y SOLO ESTO: que todo D-x VIGENTE de la tabla tenga AL MENOS UNA ancla en el
# fuente, y que todo D-x DEROGADO -tachado con ~~ en esa misma tabla- no tenga ninguna.
#
# QUE NO MIDE, y no es pereza: es la linea que este repositorio no puede cruzar. NO juzga
# si la decision es buena -eso lo decide el responsable- ni si el codigo la implementa bien.
# Lo segundo seria una segunda copia del firmware escrita a mano, que es lo que dos
# auditorias externas llamaron "industria de sustitucion". Mide CORRESPONDENCIA, no verdad.
#
# 🔴 LO QUE UN ANCLA NO DEMUESTRA: un ancla dice que ALGUIEN AFIRMO que ese codigo
# implementa esa decision. No dice que sea verdad. Eso lo sigue decidiendo quien lee el
# codigo, y este pack no lo sustituye ni lo intenta.
#
# EL BORDE DECLARADO, que es donde fallan los censos de este repositorio: se censa SOLO EL
# TEXTO DE LOS COMENTARIOS de los .cpp/.h de las cuatro puntas, en src e include. Las dos
# alternativas estan medidas y son falsas. Sobre CODIGO LIMPIO (fw.codigo) el censo da CERO
# para las dieciseis anclas: un ancla ES un comentario. Sobre TEXTO CRUDO DEL REPOSITORIO se
# cuentan las CITAS -DECISIONES.md, los manuales y los propios packs nombran cada D-x muchas
# veces-, y hablar de una decision no es implementarla: los .py del banco quedan fuera.
#
# LO QUE ESE BORDE DEJA FUERA, para que nadie lo lea como cubierto: un D-x citado de pasada
# dentro de una prosa cuenta igual que uno puesto en la linea que lo implementa.
# Distinguirlos exigiria juzgar el codigo, que es lo que este pack no hace.

import re

NOMBRE = "decisiones_01_anclas"
DESCRIPCION = "cada D-x VIGENTE tiene ancla en el fuente; cada DEROGADO, ninguna"

PUNTAS = ("Maestro", "Esclavo", "Repetidor", "ESP32_Expansion")
CARPETAS = ("src", "include")
RX_FILA = re.compile(r"\*\*(D-\d+(?:\.bis)?)\*\*")
RX_ANCLA = re.compile(r"\bD-(\d+(?:\.bis)?)\b")
RX_COMENTARIO = re.compile(r"/\*.*?\*/|//[^\n]*", re.S)

# La UNICA excepcion que sobrevivio a medirle el motivo. Las otras cuatro que se heredaban
# por escrito se cayeron al comprobarlas: D-7, D-11 y D-19 SI tienen ancla -son de una sola
# punta, y la regla pide una, no dos-, y D-17 tiene trece lineas de implementacion en
# ESP32_Expansion/src/despachador.cpp que nadie marco. Una lista de excepciones con motivos
# sin verificar es una lista de defectos con permiso (N-122): esta se RECALCULA abajo.
EXCEPCIONES = ("D-10",)


def _filas(texto):
    """La tabla de DECISIONES.md -> [(id, derogada)]. Solo la seccion Vigentes."""
    m = re.search(r"^##\s+Vigentes\s*$(.*?)^---\s*$", texto, re.S | re.M)
    if not m:
        return None
    filas = []
    for linea in m.group(1).splitlines():
        celdas = linea.strip().split("|")
        ident = RX_FILA.search(celdas[1]) if linea.strip()[:1] == "|" and len(celdas) > 2 else None
        if ident:
            filas.append((ident.group(1), "~~" in celdas[1]))
    return filas


def _sin_ancla(vigentes, censo):
    return [d for d in vigentes if d not in EXCEPCIONES and not censo.get(d)]


def _con_ancla(derogadas, censo):
    return [d for d in derogadas if censo.get(d)]


def correr(b, fw):
    texto = fw.texto_repo("DECISIONES.md")
    filas = _filas(texto)
    if filas is None or len(filas) < 10:
        raise fw.Abortado(
            "no se pudo parsear la tabla de DECISIONES.md (%s): sin ella este pack no sabe que "
            "decisiones hay ni cuales estan derogadas, y escribir la lista a mano seria el valor "
            "por defecto que este banco no admite"
            % ("seccion no hallada" if filas is None else "%d filas" % len(filas)))
    vigentes = [d for d, dero in filas if not dero]
    derogadas = [d for d, dero in filas if dero]

    # ---- CENSO: comentarios del fuente, el borde declarado en la cabecera ----
    censo, crudo = {}, {}
    for punta in PUNTAS:
        for car in CARPETAS:
            for n in fw.fuentes_de(punta, car, ".cpp") + fw.fuentes_de(punta, car, ".h"):
                t = fw.texto(punta, car, n)
                ruta = "%s/%s/%s" % (punta, car, n)
                crudo[ruta] = t
                for m in RX_ANCLA.finditer("\n".join(RX_COMENTARIO.findall(t))):
                    censo.setdefault("D-" + m.group(1), set()).add(ruta)

    # ---- 1. Cada vigente tiene ancla ----
    b.titulo("las %d decisiones VIGENTES estan ancladas en el fuente" % len(vigentes))
    for d in vigentes:
        if d in EXCEPCIONES:
            continue
        donde = sorted(censo.get(d, ()))
        b.verificar(bool(donde),
                    "%s anclado en %d fichero(s): %s" % (d, len(donde), ", ".join(donde)[:66]),
                    "%s esta VIGENTE en DECISIONES.md y no tiene una sola ancla en el firmware. "
                    "O no esta construida, o lo esta y nadie puede saber donde: el siguiente "
                    "agente la implementa otra vez, o la deshace" % d)

    # ---- 2. Cada derogada tiene CERO ----
    b.titulo("las %d decisiones DEROGADAS no dejan ancla viva" % len(derogadas))
    for d in derogadas:
        donde = sorted(censo.get(d, ()))
        b.verificar(not donde,
                    "%s esta derogado y no queda ninguna ancla suya en el fuente" % d,
                    "%s esta DEROGADO en DECISIONES.md y sigue anclado en %s. Un ancla de una "
                    "decision muerta se lee como vigente, que es el bucle de hacer y deshacer "
                    "que este fichero existe para cortar" % (d, ", ".join(donde)))

    # ---- 3. La excepcion, con su motivo RECALCULADO ----
    b.titulo("la excepcion se mide, no se copia")
    modelo = re.search(r"DS-2CD[0-9A-Za-z-]+", texto)
    if not modelo:
        raise fw.Abortado(
            "D-10 dice que camara se compro y no se pudo leer el modelo de su fila: sin ese "
            "literal no hay como medir si tiene superficie de firmware, y la excepcion pasaria "
            "a ser una frase")
    tocan = sorted(n for n, t in crudo.items() if modelo.group(0) in t)
    b.verificar(not tocan and not censo.get("D-10"),
                "D-10 sigue siendo una compra sin superficie: '%s' no aparece en ningun "
                "fuente del firmware, asi que su falta de ancla es correcta" % modelo.group(0),
                "D-10 dejo de ser una compra sin superficie de firmware -> %s. La excepcion "
                "se retira y esa decision pasa a exigir ancla como las demas"
                % (tocan or "ya tiene ancla en %s" % sorted(censo.get("D-10", ()))))

    # ---- 4. CONTROLES NEGATIVOS: las dos direcciones, con parche EN MEMORIA ----
    b.titulo("controles negativos")
    victima = next((d for d in vigentes if d not in EXCEPCIONES and censo.get(d)), None)
    if victima is None:
        raise fw.Abortado("ninguna vigente tiene ancla: no hay caso bueno que romper, y un "
                          "control negativo sobre el vacio no demuestra nada")
    roto = dict(censo)
    roto.pop(victima)
    b.control_negativo(
        victima in _sin_ancla(vigentes, roto) and victima not in _sin_ancla(vigentes, censo),
        "quitando en memoria el ancla de %s -sin tocar el fuente en disco- el pack pasa a "
        "acusarlo, y antes no lo acusaba" % victima)

    # El censo de partida se toma SIN esa clave, para que este control siga midiendo aunque
    # algun dia la derogada tenga ancla de verdad: si no, se acusaria a si mismo.
    dero = derogadas[0] if derogadas else "D-0"
    sin = {k: v for k, v in censo.items() if k != dero}
    b.control_negativo(
        _con_ancla([dero], dict(sin, **{dero: {"Maestro/src/main.cpp"}})) == [dero]
        and _con_ancla([dero], sin) == [],
        "anadiendo en memoria un ancla de %s -que esta derogado- el pack pasa a acusarlo, y "
        "sin ella no lo acusaba" % dero)

    # Sobre un censo SINTETICO, para que esta linea pueda caer sin que caigan las de arriba:
    # una comprobacion que no puede fallar sola no es comprobacion, es adorno.
    limpio = {"D-98": {"Maestro/src/main.cpp"}}
    b.control_negativo(
        not _sin_ancla(["D-98"], limpio) and not _con_ancla(["D-99"], limpio),
        "y NO acusa al caso bueno -vigente con ancla, derogado sin ella-. Un detector que "
        "acusa siempre es una alarma apagada")

    b.control_negativo(
        _filas("## Vigentes\n\n| # |\n|---|\n| **D-98** |\n| ~~**D-99**~~ |\n---\n")
        == [("D-98", False), ("D-99", True)],
        "el parseo distingue una fila vigente de una tachada con ~~. Sin eso contaria las "
        "derogadas como vigentes y pediria anclas de decisiones muertas")
