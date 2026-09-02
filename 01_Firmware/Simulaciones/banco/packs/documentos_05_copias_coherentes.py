# -*- coding: utf-8 -*-
# POR QUE EXISTE ESTE PACK.
#
# El 02/09, en el mismo commit, README, ESTADO.md y CERTIFICACION_SW.md publicaron TRES
# numeros distintos del mismo banco -950/955, 940/955 y 954/955- y los tres llevaban
# encima la frase "cifras copiadas del acta". El acta decia 955/955. Ademas el README
# publicaba "19 PASS | 1 FALLA | 0 ABORTADO (Exit code 0)", que es un estado
# internamente imposible: con un FALLA la compuerta sale con 1.
#
# Y NO LO CAZO NADIE, por un motivo que hay que entender antes de tocar esto:
# documentos_01 compara los documentos contra el acta, pero deja fuera A PROPOSITO el
# NUMERADOR de "banco por packs" -cuantas cumplen- porque esa cifra la escribe la propia
# corrida: compararla crea un bucle que ningun documento puede cerrar (acta roja ->
# README no coincide -> acta roja). La exclusion es correcta. Lo que faltaba es lo que
# esa exclusion dejo sin vigilar.
#
# LA COMPROBACION QUE SI VALE Y NO CICLA: que las copias digan LO MISMO ENTRE SI.
#
# Que tres documentos coincidan no depende de si el banco paso o fallo, asi que no hay
# bucle. Y una divergencia entre ellos es SIEMPRE un defecto: o alguien copio a mano, o
# alguien actualizo uno y se dejo los otros dos. Es la mitad de N-93 que quedaba abierta.
#
# LO QUE ESTE PACK NO ES, y conviene decirlo: no es la solucion del problema de fondo.
# La auditoria externa del 02/09 dejo la pregunta en la mesa del responsable -por que los
# documentos COPIAN cifras en vez de citarlas-, y mientras se copien hara falta esto. Si
# algun dia los documentos dicen "banco: ver la ultima acta", este pack sobra y se retira.

import re

NOMBRE = "documentos_05_copias_coherentes"
DESCRIPCION = "README, ESTADO y CERTIFICACION dicen lo MISMO entre si, no cada uno lo suyo"

DOCS = ("README.md", "ESTADO.md", "CERTIFICACION_SW.md")

# Cada cifra con el patron que la reconoce en cualquiera de los tres. Se comparan solo
# los documentos que la publican: si uno no la nombra, no es asunto de este pack -de eso
# se encarga documentos_01 contra el acta-.
CIFRAS = (
    # El patron va anclado a "banco" o a "packs" EN LA MISMA LINEA. Sin eso cazaba
    # tambien el 271/271 de la pantalla, y el pack acusaba a README de discrepar
    # consigo mismo: la regla del instrumento dentro del instrumento.
    ("el total del banco",          r"(\d{3,4}/\d{3,4})[^\n]{0,60}?(?:packs|banco)|(?:banco|packs)[^\n]{0,60}?(\d{3,4}/\d{3,4})"),
    ("el recuento de packs",        r"(\d{2,3})\s+packs"),
    ("el flash del Maestro",        r"(\d{2}[.,]\d)\s*%[^|\n]{0,40}(?:Maestro|maestro)"),
    ("las comprobaciones en DOM",   r"(\d{2,3}/\d{2,3})[^|\n]{0,30}(?:jsdom|DOM)"),
)


def correr(b, fw):
    textos = {}
    for d in DOCS:
        try:
            textos[d] = fw.texto_repo(d)
        except Exception:
            raise fw.Abortado(
                "no se pudo leer %s: sin los tres documentos no hay copias que comparar, "
                "y aprobar con dos seria medir de menos en silencio" % d)

    # ---- 1. Cada cifra dice lo mismo alla donde aparece ----
    for etiqueta, patron in CIFRAS:
        halladas = {}
        for d, t in textos.items():
            crudo = re.findall(patron, t)
            vistos = set()
            for m in crudo:
                if isinstance(m, str):
                    vistos.add(m)
                else:
                    vistos.update(x for x in m if x)
            vistos = set(v.replace(",", ".") for v in vistos)
            if vistos:
                halladas[d] = vistos

        if len(halladas) < 2:
            b.reportar("%s no lo publican dos documentos" % etiqueta,
                       ["lo publica: %s" % (", ".join(halladas) or "ninguno"),
                        "sin dos copias no hay nada que comparar aqui; contra el acta "
                        "lo vigila documentos_01"])
            continue

        comun = set.intersection(*halladas.values())
        b.verificar(
            bool(comun),
            "%s: los %d documentos que lo publican coinciden (%s)"
            % (etiqueta, len(halladas), ", ".join(sorted(comun))[:40]),
            "%s: cada documento dice una cosa -> %s. Los tres llevan encima la frase "
            "'copiadas del acta', asi que dos de ellos la llevan sin merecerla: una "
            "cifra que no sale de la ultima corrida se lee como medida"
            % (etiqueta, " | ".join("%s=%s" % (d, ",".join(sorted(v)))
                                    for d, v in sorted(halladas.items()))))

    # ---- 2. Ningun documento publica un estado internamente imposible ----
    #
    # "N PASS | M FALLA ... Exit code 0" con M > 0 no puede haber ocurrido: la compuerta
    # sale con 1 en cuanto hay un FALLA. Se caza aqui y no en documentos_01 porque no
    # hace falta el acta para saberlo: se contradice sola.
    rx = re.compile(r"(\d+)\s*PASS\s*[·|]\s*(\d+)\s*FALLA[^\n]{0,80}?"
                    r"[Ee]xit\s*code:?\s*.{0,3}?(\d)")
    for d, t in textos.items():
        malas = [(p, f, e) for p, f, e in rx.findall(t)
                 if (int(f) > 0) != (e != "0")]
        b.verificar(
            not malas,
            "%s no publica ningun estado de compuerta imposible" % d,
            "%s publica %s: con un FALLA la compuerta sale con 1, y con cero sale con 0. "
            "Una linea que se contradice sola no la escribio una corrida: la escribio "
            "alguien a mano" % (d, malas))

    # ---- 3. CONTROL NEGATIVO: la comparacion sabe distinguir el caso malo ----
    #
    # Sin esto, un patron que no casara con nada aprobaria los tres documentos por
    # coincidir en el vacio, que es como una prueba muerta pasa por vigilante.
    falsos = {"A.md": {"955/955"}, "B.md": {"940/955"}}
    comun_falso = set.intersection(*falsos.values())
    b.control_negativo(
        not comun_falso,
        "el detector distingue dos copias que discrepan: 955/955 contra 940/955 no "
        "tienen nada en comun, asi que sabe marcar el caso malo")

    iguales = {"A.md": {"955/955"}, "B.md": {"955/955"}}
    b.control_negativo(
        bool(set.intersection(*iguales.values())),
        "y NO acusa a dos copias que si coinciden -955/955 contra 955/955-: sin esto "
        "acusaria siempre, y una alarma que suena siempre es una alarma apagada")
