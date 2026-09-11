# -*- coding: utf-8 -*-
# ===== banco/packs/documentos_06_no_reabre_lo_cerrado.py =====
#
# POR QUE EXISTE ESTE PACK.
#
# El responsable, 11/09: "cada nada se va y vuelve en un bucle y se cambian las spec
# cuando se va a la documentacion legacy". El mecanismo es siempre el mismo: un agente
# abre un manual, lee una frase que YA ESTA DEROGADA pero que nadie tacho, la toma por
# spec y reabre una decision cerrada. roadmap.md §0 lo dice con un recuadro -"DECIDIDO Y
# CERRADO EL 11/09 - NO SE REABRE DESDE UN DOCUMENTO"-, pero un recuadro es una frase, y
# una frase no vigila nada.
#
# QUE MIDE, Y SOLO ESTO: que ninguna frase DEROGADA por D-20, D-25, D-26 o D-27 aparezca SIN
# MARCAR en un documento VIVO. Marcada es: tachada (~~...~~ en markdown; <s>, <del>,
# <strike> o style="text-decoration:line-through" en html), o dentro de la fe de erratas
# de un html (<... class="decia">), o citada como muerta (ver CITA, abajo). Es la forma en
# que este repositorio dice "esto se decia y ya no".
#
# POR QUE LA LISTA ESTA ESCRITA A MANO, y es deliberado: si se leyera de DECISIONES.md o
# del recuadro, la frase se aprobaria a si misma -el dia que alguien reescribe la fila
# con otras palabras, el pack pasa a buscar las palabras nuevas y deja de ver las viejas,
# que son las que siguen en los manuales-. DECISIONES.md se lee SOLO para comprobar que
# la D-x que derogo cada frase sigue VIGENTE: si una de ellas cae, sus frases pueden
# volver a ser ciertas y la lista se revisa a mano, no en silencio.
#
# QUE NO MIDE, para que nadie lo lea de mas:
#  - NO juzga prosa. Una frase derogada dicha con OTRAS palabras no la ve. La lista caza
#    las formas MEDIDAS en el arbol el 11/09 -por el nombre y por su consecuencia,
#    CLAUDE.md §14-; una forma nueva exige anadirla a mano.
#  - NO lee los .docx (derivados de los .md con convertir_a_word.py; D-27 punto 5 los deja
#    pendientes). Un .docx viejo puede seguir diciendo la frase muerta.
#  - NO lee la app (App_Semaforo/): no es documentacion y no tiene convencion de tachado.
#  - NO lee los comentarios del firmware, aunque el recuadro de roadmap.md §0 los nombra:
#    el 11/09 los botones.cpp/botones.h de las dos puntas dicen "UNA CAMARA POR POSTE" y
#    modo_inteligente.cpp del Maestro "un posible fin de carrera de barrera". Eso es de
#    otro instrumento (decisiones_01_anclas mira anclas, no frases).
#  - Un tachado por una CLASE CSS que no sea "decia" no lo reconoce: medido el 11/09, los
#    html tachan con <s> y nada mas (10 en Camaras_Sisga_4x.html, 82 en Guia_Cableado; cero
#    <del>, cero line-through). Si alguien tacha con otra clase, esto da ROJO -falso rojo,
#    que grita-, no un falso verde.
#
# EL BORDE, escrito al lado porque es donde fallan los censos (CLAUDE.md §7):
#
#  SE LEEN (se CENSAN por directorio, no se listan: un manual nuevo entra solo):
#   - 05_Funcional/*.md y *.html, SOLO el primer nivel.
#   - 04_Manuales/*.md, primer nivel.
#   - la raiz: todo *.md y ARQUITECTURA.map, menos lo excluido abajo. Es mas que la lista
#     del encargo (README, ESTADO, ARQUITECTURA.map, roadmap) a proposito: LEEME_PRIMERO,
#     ORDEN_EJECUCION, INDICE_CRUZADO, OPTIMIZACIONES y CERTIFICACION_SW son justo lo que un
#     agente abre primero, y CLAUDE.md se carga en CADA sesion.
#
#  SE EXCLUYEN, cada uno por un motivo que se puede comprobar:
#   - DECISIONES.md: es la tabla que DEROGA. Su columna "deroga" cita cada frase muerta
#     sin tachar a proposito, y es la fuente contra la que se mide. Lo que su desarrollo
#     largo diga (p.ej. D-13, "sin filtro de objetivo") es de su fila, que firma el
#     responsable (roadmap.md 2.7: "es su fila").
#   - roadmap_hist.md: la CRONICA de lo cerrado. Tiene que seguir contando lo que se
#     decidio en cada momento con las palabras de entonces; roadmap.md la declara "el
#     porque de lo cerrado", no spec.
#   - 05_Funcional/historico/ y 99_Legacy/: lo RETIRADO (sus ficheros llevan _RETIRADA_,
#     _ARCHIVADO_ o _CERRADO en el nombre, o viven en una carpeta cuyo nombre dice que no
#     mandan). Quedan fuera por construccion: el censo no baja de nivel.
#   - dentro de roadmap.md, SOLO el recuadro "DECIDIDO Y CERRADO EL 11/09" (el bloque >
#     que empieza en su titulo): su columna "lo que deroga" cita las frases muertas sin
#     tachar por la misma razon que DECISIONES.md. El resto de roadmap.md SI se lee.

import os
import re
import unicodedata
from html.parser import HTMLParser

NOMBRE = "documentos_06_no_reabre_lo_cerrado"
DESCRIPCION = "ninguna frase derogada por D-20/25/26/27 sigue SIN TACHAR en un documento vivo"

# ---------------------------------------------------------------------------------
# LA LISTA.
#
# Cada entrada: dx (la decision que la derogo), frase (legible), patrones, motivo,
# ejemplo (frase sintetica que el patron TIENE que cazar: su control negativo propio) y
# matadores (las D-x/A-x cuya mencion junto a una cita la marca como muerta).
#
# Los patrones van sobre el texto NORMALIZADO -minusculas, sin tildes, sin * ni `,
# espacios colapsados, sin el > de las citas-, por eso son ASCII. Si un patron lleva el
# grupo (?P<f>...), lo que tiene que estar tachado es ESE NUCLEO y no el contexto que se
# pide para no cazar de mas: "las camaras ... ~~una por poste~~" esta bien tachado aunque
# "camaras" no lo este. Sin el grupo, el tachado se pedia a toda la frase y el pack
# acusaba a 12 lineas bien tachadas (medido el 11/09).
#
# NO ENTRAN, y se dice por que (el encargo las proponia):
#  - HORA_PUESTA_SIN_PROPAGAR: NO esta derogada. Sigue viva en el fuente como MOTIVO 7 del
#    SET_RTC del puente (ESP32_Expansion/src/despachador.cpp). OJO: aqui ponia "rama
#    `!propagada`" y ESA RAMA YA NO EXISTE: medido el 11/09, el parametro `propagada` se
#    retiro con D-26 -lo dice su propio despachador.h- y el motivo 7 cuelga hoy de
#    `!siembra_ahora()`, DENTRO de la rama de SET_RTC y despues de releer el DS3231. La
#    frase no cambia de bando por eso: lo que D-20 tacho fue "falta la propagacion", no el
#    literal, y no es de D-20/25/26/27. Un documento que la nombre puede estar en lo cierto.
#  - "zona donde el vehiculo espera": choca con D-13 (barrido de la pluma), pero D-27 NO
#    deroga la zona (roadmap.md §0, nota al pie del recuadro). No es de estas tres.
#  - Threshold/Sensitivity/1 s/50: D-27 deroga los valores de D-13 SOLO "en lo que
#    choquen con el manual del modelo", y el manual del modelo solo fija el objetivo
#    (misma nota). Solo entra el objetivo.
# ---------------------------------------------------------------------------------

DEROGADAS = (
    dict(dx="D-25", frase="una camara por poste", matadores=("D-25",),
         patrones=(
             r"(?P<f>\b(?:una|1)\s+camara\s+por\s+poste\b)",
             # La forma corta: "SON DOS CAMARAS EN EL CRUCE, UNA POR POSTE". Exige "camara"
             # antes en la MISMA frase; sin eso cazaba "talanquera en J15 (una por poste)",
             # "dos fuentes de hora, una por poste" y "dos placas iguales, una por poste",
             # que son ciertas.
             r"\bcamaras?\b[^.;|]{0,80}?(?P<f>\buna\s+por\s+poste\b)",
         ),
         motivo="D-25: son CUATRO camaras, DOS por poste (p10 y p12 en cada uno)",
         ejemplo="En el cruce hay una camara por poste, en p10."),

    dict(dx="D-25", frase="dos camaras en el cruce / uno de los dos pines esta vacio",
         matadores=("D-25",),
         patrones=(
             r"(?P<f>\bdos\s+camaras\s+en\s+el\s+cruce\b)",
             r"(?P<f>\bcamaras\s+del\s+cruce\s+son\s+dos\b)",
             r"(?P<f>\bdos\s+en\s+todo\s+el\s+cruce\b)",
             # La CONSECUENCIA de "una por poste" (CLAUDE.md §14: se busca por el nombre y
             # por lo que se dedujo de el): el 11/09 salia en tres manuales sin nombrar
             # la frase de origen.
             r"(?P<f>\buno\s+de\s+(?:estos|los)\s+dos\s+pines\b[^.;|]{0,20}?\besta\s+vacio\b)",
         ),
         motivo="D-25: cuatro camaras y los DOS pines de J16 (p10, p12) cableados en cada poste",
         ejemplo="En todo equipo montado uno de los dos pines esta vacio."),

    dict(dx="D-25", frase="p12 vacio / queda vacio / se deja vacio / queda libre",
         matadores=("D-25",),
         patrones=(
             r"(?P<f>\bp12\b[^\w|]{0,6}(?:se\s+)?(?:deja|queda|quedara)\s+(?:vacio|libre)\b)",
             r"(?P<f>\bp12\b\s*(?:<--|->|\u2192)?\s*hoy\s+(?:vacio|libre)\b)",
             r"(?P<f>\bp12\b\s*(?:<--|->|\u2192)\s*(?:vacio|libre)\b)",
             # "para un p12 vacio a proposito" NO: es la cronica de por que se escribio la
             # exencion del vigilante (roadmap.md §3.16), y es cierta.
             r"(?<!para un )(?<!para el )(?P<f>\bp12\b[^\w|]{0,6}vacio\s+a\s+proposito\b)",
         ),
         motivo="D-25: p12 lleva la CAMARA 2 de cada poste (CAM_D_PIN); no queda vacio",
         ejemplo="El p12 queda vacio como segunda entrada."),

    dict(dx="D-25", frase="las 4 camaras se revocaron el 28/08 (ESTADO.md C1)",
         matadores=("D-25",),
         patrones=(
             r"(?P<f>\b(?:4|cuatro)\s+camaras\s+se\s+revocaron\b)",
             r"(?P<f>\brevocad[oa]\s+el\s+28/08\s*:?\s*van\s+dos\b)",
         ),
         motivo="D-25 vuelve a CUATRO camaras; lo revocado el 28/08 quedo derogado",
         ejemplo="Las 4 camaras se revocaron el 28/08."),

    dict(dx="D-26", frase="la siembra ESP32->STM32 cada hora (el numero de A-15)",
         matadores=("D-26",),
         patrones=(
             # "no cada hora" / "ni cada hora" NO: es la correccion (ESTADO.md: "cada ~5
             # min, no cada hora"). "cada hora o cada mes" NO: es un contrafactico
             # (roadmap.md §3.10: "se siembre cada hora o cada mes, eso no cambia").
             r"\bsiembr\w*[^.;|]{0,80}?(?<!\bno )(?<!\bni )(?P<f>\bcada\s+hora\b)(?!\s+o\s+cada\b)",
             r"(?<!\bno )(?<!\bni )(?P<f>\bcada\s+hora\b)(?!\s+o\s+cada\b)[^.;|]{0,40}?\bsiembr",
             r"(?P<f>\bsiembra\s+horaria\b)",
             r"(?P<f>\breus\w*\s+intervalo_sync_ms\b)",
         ),
         motivo="D-26 (2): cada STM32 se siembra desde el DS3231 de su ESP32 cada ~5 min. El "
                "reenvio Maestro->Esclavo por radio (INTERVALO_SYNC_MS, una hora) NO es esto y "
                "sigue vivo: por eso el patron exige 'siembra'",
         ejemplo="El ESP32 siembra la hora al STM32 cada hora."),

    dict(dx="D-26", frase="la siembra cada 2 s", matadores=("D-26", "A-15"),
         patrones=(
             # La cadencia del $STATUS es de 2 s y es cierta (04/09): por eso se exige
             # "siembra" en la misma frase.
             r"\bsiembr\w*[^.;|]{0,80}?(?P<f>\bcada\s+(?:2|dos)\s*s(?:eg\w*)?\b)",
             r"(?P<f>\bcada\s+(?:2|dos)\s*s(?:eg\w*)?\b)[^.;|]{0,40}?\bsiembr",
         ),
         motivo="D-26 (2): ~5 min. Los 2 s ya los corrigio A-15 el 08/09",
         ejemplo="El DS3231 siembra al STM32 cada 2 s."),

    dict(dx="D-26", frase="el Esclavo acepta su ESP32 si la radio lleva 2 h sin sembrar",
         matadores=("D-26",),
         patrones=(r"(?P<f>\b(?:2|dos)\s*h(?:oras?)?\s+sin\s+sembrar\b)",),
         motivo="D-26 (3)+(4)+(5): con radio manda el Maestro; SIN radio, su propio ESP32, "
                "sin plazo de 2 h",
         ejemplo="El Esclavo acepta la de su ESP32 si la radio lleva 2 h sin sembrar."),

    dict(dx="D-27", frase="el fin de carrera va a J14 / J14 reservado al fin de carrera",
         matadores=("D-27",),
         patrones=(
             r"(?P<f>\bfin\s+de\s+carrera\s+(?:va|ira|se\s+conecta|se\s+cablea|se\s+instala)\s+(?:a|en)\s+(?:j14|pb0)\b)",
             r"(?P<f>\breservad[oa]\s+(?:a|al|para)\s+(?:un\s+)?(?:posible\s+)?(?:fin|final)\s+de\s+carrera\b)",
             # "A-2 reserva J14 para el fin de carrera" / "A-2 manda el fin de carrera a
             # J14" / "A-2 lo reserva al final de carrera": el 11/09 era la forma MAS
             # repetida, dentro de los avisos de "conflicto abierto".
             r"(?P<f>\ba-2\b[^.;|]{0,14}?\b(?:manda|reserva|reservo|lo\s+reserva|la\s+reservo|lo\s+quiere|le\s+manda)\b[^.;|]{0,40}?\b(?:fin|final)\s+de\s+carrera\b)",
         ),
         motivo="D-27 (2): J14 queda LIBRE, sin cablear; el fin de carrera NO se instala en "
                "este despliegue (deroga A-2 en esa parte)",
         ejemplo="El fin de carrera va a J14."),

    dict(dx="D-27", frase="J14: conflicto abierto (A-2 contra el firmware)",
         matadores=("D-27",),
         patrones=(
             r"\bj14\b[^.;|]{0,40}?(?P<f>\bconflicto\s+abierto\b)",
             r"(?P<f>\bconflicto\s+abierto\b)[^.;|]{0,80}?\b(?:j14|a-2)\b",
         ),
         motivo="D-27 (2) cierra ese conflicto sin tocar el firmware: J14 libre, nada "
                "conectado mientras el firmware lea PB0 como demanda",
         ejemplo="J14: CONFLICTO ABIERTO, no lo resuelve este manual."),

    dict(dx="D-27", frase="compradas constan 2 camaras",
         matadores=("D-27",),
         patrones=(
             r"(?P<f>\bcompradas\s+(?:constan|hay|son)\s+(?:2|dos)\b)",
             r"(?P<f>\b(?:2|dos)\s+unidades\s+compradas\b)",
         ),
         motivo="D-27 (1): las CUATRO camaras estan compradas. No esta en su columna "
                "'deroga', pero es lo que su punto (1) deja falso",
         ejemplo="Compradas constan 2; las otras 2 SIN VERIFICAR."),

    # LA UNICA DE D-20, Y ENTRA POR LA MISMA PUERTA QUE LAS DEMAS.
    #
    # La frase la tacho su PROPIA fila el 07/09 por la noche -"no hace falta prohibir nada:
    # la barrera es la SOBREESCRITURA"- y D-26 (5) manda hoy lo contrario: con la radio
    # caida el usuario VA al poste 2 y le pone la hora desde el telefono. Se atribuye a
    # D-20 porque es la fila que la derogo, que es lo que el paso 0 comprueba que sigue
    # vigente; los "matadores" llevan las dos, porque los documentos la citan muerta con
    # una o con otra.
    #
    # SE BUSCA POR EL NOMBRE Y POR SUS CONSECUENCIAS (CLAUDE.md 14), que es como estaba
    # escrita de verdad en los ~20 documentos que se corrigieron el 11/09 (fd74121): el
    # rechazo del SET_RTC dirigido al Esclavo, el "no se manda al poste 2", la "segunda
    # fuente", el "prohibe escribir la hora en el poste 2" y la regla de campo "en la
    # puesta en marcha, NO durante la averia" -esta ultima es justo la que D-26 (5)
    # invierte-. Sin las consecuencias, el patron veria una de cada tres.
    dict(dx="D-20", frase="la app no pone la hora en el poste 2, nunca / un SET_RTC al Esclavo se rechaza",
         matadores=("D-20", "D-26"),
         patrones=(
             r"(?P<f>\bno\s+pone\s+la\s+hora\s+en\s+el\s+poste\s+2\b)",
             r"(?P<f>\bdirigid[oa]\s+al\s+(?:esclavo|poste\s+2)\b[^.;|]{0,40}?\bse\s+rechaza\b)",
             r"(?P<f>\bset_rtc\s+no\s+se\s+manda\s+al\s+poste\s+2\b)",
             r"(?P<f>\bal\s+poste\s+2\s+no\s+se\s+manda\b)",
             r"(?P<f>\bnunca\s+en\s+el\s+poste\s+2\b)",
             r"(?P<f>\bprohibe\b[^.;|]{0,40}?\bla\s+hora\s+en\s+el\s+poste\s+2\b)",
             # La regla de campo que se dedujo de ella. El nucleo es solo la negacion: "el
             # poste 2 se pone en hora en la puesta en marcha" SIGUE SIENDO CIERTO y por eso
             # los documentos corregidos tachan desde la coma.
             r"\bposte\s+2\b[^.;|]{0,80}?(?P<f>\bno\s+durante\s+la\s+averia\b)",
             # "poner la hora en el poste 2 es una segunda fuente". El (?<!\bno ) es
             # obligatorio: la correccion de varios manuales dice "el DS3231 del poste 2 NO
             # es una segunda fuente: es la memoria de la unica fuente", que es lo contrario.
             r"\b(?:esclavo|poste\s+2)\b[^.;|]{0,60}?(?<!\bno )(?P<f>\b(?:seria|es)\s+(?:crear\s+)?una\s+segunda\s+fuente\b)",
         ),
         motivo="D-26 (5): sin radio el usuario va al poste 2 y le pone la hora desde el "
                "telefono -y su puente la atiende y siembra su STM32-. La prohibicion ya "
                "estaba tachada en la propia fila D-20: la barrera es la SOBREESCRITURA, no "
                "el rechazo",
         ejemplo="La app NO pone la hora en el poste 2. Nunca."),

    dict(dx="D-27", frase="objetivo: no filtrar / sin filtro de objetivo (vehiculo y persona)",
         matadores=("D-27",),
         patrones=(
             r"\bobjetivo\s*:?\s*(?P<f>no\s+filtrar\b)",
             r"(?P<f>\bsin\s+filtro\s+de\s+objetivo\b)",
             r"(?P<f>\bno\s+filtrar\s*\(\s*vehiculo\s+y\s+persona\b)",
         ),
         motivo="D-27 (3): la configuracion es la del manual del modelo, que fija el objetivo "
                "(solo Vehiculo, sin Humano). Deroga el 'sin filtro' de D-13",
         ejemplo="Objetivo: no filtrar (vehiculo y persona)."),
)

_CANON = DEROGADAS

# LA CITA -la unica excepcion a "sin marcar = FALLA", y va medida-.
#
# Un documento que dice "D-25 deroga de D-13 solo <<una camara por poste>>" no afirma la
# frase: la nombra PARA MATARLA. Exigir que ademas la tache seria pedirle que escriba lo
# que el instrumento quiere leer (CLAUDE.md §1), y el 11/09 eran 24 aciertos asi.
#
# La regla es estrecha a proposito, porque una excepcion es el instrumento de verdad
# (§6): cuenta como cita SOLO si la frase va ENTRE COMILLAS (<< >>, "", '', o las curvas) Y en su
# linea o en la anterior aparece una marca de muerte: la raiz "derog", la raiz "caduc"
# -las dos palabras con que este repositorio rotula lo muerto-, o una de las decisiones
# que la mataron (los "matadores" de su entrada). Las DOS cosas. Una frase sin comillas
# junto a "deroga" sigue siendo FALLA, y una entre comillas sin marca, tambien.
# "revoc" NO es marca: "REVOCADO el 28/08: van DOS" es justo la frase muerta.
# Las citas perdonadas se LISTAN en un reportar() -no cuentan- para que se vean.
_AB, _CE = "\u00ab", "\u00bb"          # << >> espanolas, escritas en ASCII (CLAUDE.md §13)
_COMILLAS = ((_AB, _CE), ('"', '"'), ("'", "'"), ("\u201c", "\u201d"))
_MARCAS = ("derog", "caduc")


def _normalizar(chars):
    """-> (texto normalizado, mapa indice_normalizado -> indice_original)."""
    out, mapa = [], []
    inicio_linea = True
    for i, c in enumerate(chars):
        if c == "\n":
            inicio_linea = True
        if c.isspace():
            if out and out[-1] != " ":
                out.append(" ")
                mapa.append(i)
            continue
        if inicio_linea and c == ">":
            continue
        inicio_linea = False
        if c in "*`":
            continue
        for d in unicodedata.normalize("NFD", c):
            if unicodedata.combining(d):
                continue
            out.append(d.lower())
            mapa.append(i)
    return "".join(out), mapa


def _tachado_md(texto):
    """Por caracter: True si esta dentro de ~~...~~. -> (marcas, [lineas con ~~ huerfano]).

    Los ~~ se emparejan DENTRO de un segmento, nunca a traves de el: cada fila de tabla
    es su propio segmento (un tachado no cruza de fila en fila) y el resto se parte por
    parrafos -linea en blanco, o linea de solo '>' en una cita-. Sin esto, un ~~ suelto
    en una fila se emparejaba con el de la siguiente y daba por tachado lo que no lo esta:
    falso verde. Un segmento con un numero impar de ~~ deja el ultimo sin pareja y NO
    tacha nada con el (se reporta)."""
    marca = [False] * len(texto)
    segmentos, ini, pos = [], None, 0
    for linea in texto.splitlines(keepends=True):
        fin = pos + len(linea)
        cuerpo = linea.strip().lstrip(">").strip()
        if not cuerpo:
            if ini is not None:
                segmentos.append((ini, pos))
                ini = None
        elif cuerpo.startswith("|"):
            if ini is not None:
                segmentos.append((ini, pos))
                ini = None
            segmentos.append((pos, fin))
        elif ini is None:
            ini = pos
        pos = fin
    if ini is not None:
        segmentos.append((ini, pos))
    # Un `~~` entre comillas invertidas es codigo en linea: HABLA del tachado, no tacha
    # (el 11/09, tres de los cinco ~~ huerfanos eran eso: "un `~~` que falta...").
    en_codigo = set()
    for m in re.finditer(r"`[^`\n]*`", texto):
        en_codigo.update(range(m.start(), m.end()))
    huerfanos = []
    for a, z in segmentos:
        tildes = [m.start() + a for m in re.finditer("~~", texto[a:z])
                  if m.start() + a not in en_codigo]
        if len(tildes) % 2:
            huerfanos.append(texto.count("\n", 0, tildes[-1]) + 1)
        for k in range(0, len(tildes) - 1, 2):
            for j in range(tildes[k], tildes[k + 1] + 2):
                marca[j] = True
    return marca, huerfanos


class _Html(HTMLParser):
    """Texto de un html con su marca (tachado o fe de erratas) y su linea."""
    TACHA = ("s", "del", "strike")
    VACIAS = ("br", "img", "hr", "meta", "link", "input", "wbr", "source", "col",
              "area", "base", "embed", "param", "track")

    def __init__(self):
        super().__init__(convert_charrefs=True)
        self.pila = []
        self.chars, self.tach, self.lineas = [], [], []

    def handle_starttag(self, tag, attrs):
        if tag in self.VACIAS:
            return
        a = dict(attrs)
        estilo = (a.get("style") or "").replace(" ", "").lower()
        # class="decia": la fe de erratas de Camaras_Sisga_4x.html. Cada frase vieja va en
        # un <span class="decia"> rotulado "DECIA:" (en rojo) y seguido de "LO CIERTO". Es
        # la marca de muerte de esa guia, tan explicita como un <s>.
        clases = (a.get("class") or "").split()
        self.pila.append((tag, tag in self.TACHA or "line-through" in estilo
                          or "decia" in clases))

    def handle_endtag(self, tag):
        for i in range(len(self.pila) - 1, -1, -1):
            if self.pila[i][0] == tag:
                del self.pila[i:]
                return

    def _texto(self, data, marcado):
        linea = self.getpos()[0]
        for c in data:
            self.chars.append(c)
            self.tach.append(marcado)
            self.lineas.append(linea)
            if c == "\n":
                linea += 1
        # separador: dos trozos de texto de etiquetas distintas no se pegan en una palabra
        self.chars.append(" ")
        self.tach.append(marcado)
        self.lineas.append(linea)

    def handle_data(self, data):
        self._texto(data, any(t for _, t in self.pila))

    def handle_comment(self, data):
        # Un comentario html lo lee quien abre el fuente -un agente-, y no se tacha.
        self._texto(data, False)


def preparar(nombre, texto, excluir=None):
    """-> el documento listo para buscar: texto normalizado y, por caracter, marca y linea."""
    if nombre.lower().endswith(".html"):
        p = _Html()
        p.feed(texto)
        p.close()
        chars, tach, lineas, huerfanos = p.chars, p.tach, p.lineas, []
    else:
        chars = texto
        tach, huerfanos = _tachado_md(texto)
        lineas, n = [], 1
        for c in texto:
            lineas.append(n)
            if c == "\n":
                n += 1
    if excluir:
        a, z = excluir
        tach = list(tach)
        for j in range(a, min(z, len(tach))):
            tach[j] = True
    por_linea = {}
    for c, l in zip(chars, lineas):
        por_linea.setdefault(l, []).append(c)
    por_linea = {l: _normalizar(v)[0] for l, v in por_linea.items()}
    norm, mapa = _normalizar(chars)
    return {"nombre": nombre, "norm": norm, "mapa": mapa, "tach": tach,
            "lineas": lineas, "por_linea": por_linea, "huerfanos": huerfanos}


def _entre_comillas(norm, s, e):
    izq, der = norm[max(0, s - 60):s], norm[e:e + 60]
    for a, z in _COMILLAS:
        ia = izq.rfind(a)
        if ia < 0:
            continue
        if a == z:
            if izq.count(a) % 2 == 1 and z in der:
                return True
        elif z not in izq[ia + 1:] and z in der and a not in der[:der.find(z)]:
            return True
    return False


def _es_cita(doc, s, e, matadores):
    """Entre comillas Y con marca de muerte en su linea o en la anterior."""
    if not _entre_comillas(doc["norm"], s, e):
        return False
    l = doc["lineas"][doc["mapa"][s]]
    trozo = doc["por_linea"].get(l - 1, "") + " " + doc["por_linea"].get(l, "")
    return (any(m in trozo for m in _MARCAS)
            or any(re.search(r"(?<![\w-])%s(?![\w-])" % re.escape(d.lower()), trozo)
                   for d in matadores))


def buscar(doc, entrada):
    """-> [(linea, fragmento, marcado, cita)] de cada acierto de una entrada."""
    out, vistos = [], set()
    norm, mapa, tach, lineas = doc["norm"], doc["mapa"], doc["tach"], doc["lineas"]
    for p in entrada["patrones"]:
        for m in re.finditer(p, norm):
            s, e = m.span("f") if "f" in m.groupdict() and m.group("f") else m.span()
            if s in vistos:
                continue
            vistos.add(s)
            idx = [mapa[k] for k in range(s, e) if norm[k] != " "]
            marcado = all(tach[i] for i in idx)
            cita = (not marcado) and _es_cita(doc, s, e, entrada["matadores"])
            frag = norm[max(0, s - 60):e + 30]
            out.append((s, lineas[mapa[s]], frag, marcado, cita))
    # por POSICION en el documento, no por el texto del fragmento
    return [x[1:] for x in sorted(out)]


def _filas_decisiones(texto):
    """La tabla Vigentes de DECISIONES.md -> {D-x: derogada}. Misma lectura que
    decisiones_01_anclas, para que las dos no discrepen en que es una fila."""
    m = re.search(r"^##\s+Vigentes\s*$(.*?)^---\s*$", texto, re.S | re.M)
    if not m:
        return None
    filas = {}
    for linea in m.group(1).splitlines():
        celdas = linea.strip().split("|")
        if linea.strip()[:1] != "|" or len(celdas) < 3:
            continue
        ident = re.search(r"\*\*(D-\d+(?:\.bis)?)\*\*", celdas[1])
        if ident:
            filas[ident.group(1)] = "~~" in celdas[1]
    return filas


EXCLUIDOS_RAIZ = ("DECISIONES.md", "roadmap_hist.md")
TITULO_RECUADRO = "DECIDIDO Y CERRADO EL 11/09"


def _recuadro(texto):
    """(inicio, fin) del bloque > que empieza en el titulo del recuadro, o None."""
    i = texto.find(TITULO_RECUADRO)
    if i < 0:
        return None
    a = texto.rfind("\n", 0, i) + 1
    pos = a
    for linea in texto[a:].splitlines(keepends=True):
        if not linea.lstrip().startswith(">"):
            break
        pos += len(linea)
    return (a, pos)


def censar(fw):
    """-> [(ruta relativa, texto, excluir)] de los documentos vivos (ver EL BORDE)."""
    raiz = os.path.dirname(fw.ruta_repo("DECISIONES.md"))
    docs = []
    for carpeta, exts in (("05_Funcional", (".md", ".html")), ("04_Manuales", (".md",))):
        d = os.path.join(raiz, carpeta)
        if not os.path.isdir(d):
            raise fw.Abortado("no existe %s: es donde viven los manuales que un agente toma "
                              "por spec, y sin ellos este pack aprobaria por vacio" % carpeta)
        for n in sorted(os.listdir(d)):
            if n.lower().endswith(exts) and os.path.isfile(os.path.join(d, n)):
                docs.append(("%s/%s" % (carpeta, n), fw.texto_repo(carpeta, n), None))
    for n in sorted(os.listdir(raiz)):
        if not (n.endswith(".md") or n == "ARQUITECTURA.map"):
            continue
        if n in EXCLUIDOS_RAIZ or not os.path.isfile(os.path.join(raiz, n)):
            continue
        t = fw.texto_repo(n)
        docs.append((n, t, _recuadro(t) if n == "roadmap.md" else None))
    return docs


def _acusa(doc, entrada):
    """Hay al menos un acierto sin marcar y que no es cita."""
    return any(not m and not c for _, _, m, c in buscar(doc, entrada))


def _perdona(doc, entrada):
    """Hay aciertos y todos estan marcados o son cita."""
    r = buscar(doc, entrada)
    return bool(r) and all(m or c for _, _, m, c in r)


def correr(b, fw):
    # ---- 0. Las D-x de la lista siguen VIGENTES ----
    filas = _filas_decisiones(fw.texto_repo("DECISIONES.md"))
    if not filas or len(filas) < 10:
        raise fw.Abortado("no se pudo leer la tabla Vigentes de DECISIONES.md: sin ella no "
                          "se sabe si las decisiones que derogaron estas frases siguen en pie")
    b.titulo("las decisiones que derogaron las frases siguen vigentes")
    for dx in sorted(set(e["dx"] for e in DEROGADAS)):
        b.verificar(dx in filas and not filas[dx],
                    "%s sigue VIGENTE en DECISIONES.md" % dx,
                    "%s ya no esta vigente en DECISIONES.md (%s): sus frases derogadas pueden "
                    "haber vuelto a ser ciertas. La lista de este pack se revisa A MANO, no se "
                    "deja buscando lo que ya no esta muerto"
                    % (dx, "no existe" if dx not in filas else "tachada"))

    # ---- 1. Censo y borde ----
    crudos = censar(fw)
    total = sum(len(t) for _, t, _ in crudos)
    # Suelo: el 11/09 eran 41 documentos y ~3,6 M caracteres. Se pone por debajo a proposito
    # -rotan manuales- y sigue cazando lo que importa: un censo que se queda ciego.
    if len(crudos) < 30 or total < 1000000:
        raise fw.Abortado(
            "el censo de documentos vivos dio %d ficheros y %d caracteres: no pueden ser "
            "05_Funcional, 04_Manuales y la raiz juntos. Un censo ciego aprobaria todas las "
            "frases por no haber leido nada" % (len(crudos), total))
    rm = [x for x in crudos if x[0] == "roadmap.md"]
    rec = rm[0][2] if rm else None
    b.titulo("el borde")
    b.verificar(rec is not None,
                "el recuadro '%s' esta en roadmap.md y es lo UNICO que se excluye de el "
                "(%d caracteres)" % (TITULO_RECUADRO, (rec[1] - rec[0]) if rec else 0),
                "no se encuentra el recuadro '%s' en roadmap.md: o se quito el candado de lo "
                "cerrado, o cambio su titulo. Sin el no se puede delimitar la exclusion y "
                "roadmap.md se lee entero" % TITULO_RECUADRO)
    fuera = [r for r, _, _ in crudos
             if r in EXCLUIDOS_RAIZ or "/historico/" in r or r.startswith("99_Legacy")]
    b.verificar(not fuera,
                "el censo (%d documentos) no ha entrado en lo excluido (DECISIONES.md, "
                "roadmap_hist.md, historico/, 99_Legacy/)" % len(crudos),
                "el censo ha leido ficheros que el borde excluye: %s" % fuera)
    docs = [preparar(r, t, ex) for r, t, ex in crudos]

    # ---- 2. Cada frase derogada ----
    b.titulo("ninguna frase derogada por D-20/25/26/27 sigue SIN MARCAR (%d documentos)" % len(docs))
    citas = []
    for ent in DEROGADAS:
        malas = []
        for doc in docs:
            for linea, frag, marcado, cita in buscar(doc, ent):
                if marcado:
                    continue
                if cita:
                    citas.append("%s:%d [%s] ...%s..." % (doc["nombre"], linea, ent["dx"], frag))
                    continue
                sitio = "%s:%d" % (doc["nombre"], linea)
                if sitio not in malas:
                    malas.append(sitio)
                print("         %s  ...%s..." % (sitio, frag))
        b.verificar(not malas,
                    "'%s' (%s) no aparece sin marcar en ningun documento vivo"
                    % (ent["frase"], ent["dx"]),
                    "'%s' esta DEROGADA por %s y sigue SIN TACHAR en %d sitio(s): %s. %s. Un "
                    "agente que la lea la toma por spec y reabre lo cerrado: se TACHA y se "
                    "corrige hacia la decision, nunca al reves"
                    % (ent["frase"], ent["dx"], len(malas), ", ".join(malas), ent["motivo"]))
    if citas:
        b.reportar("%d cita(s) de frases derogadas perdonadas: entre comillas y con marca de "
                   "muerte en su linea (no cuentan; ver CITA en la cabecera)" % len(citas), citas)
    huerfanos = ["%s: linea(s) %s" % (d["nombre"], ", ".join(map(str, d["huerfanos"])))
                 for d in docs if d["huerfanos"]]
    if huerfanos:
        b.reportar("parrafos con un ~~ sin pareja: su ultimo ~~ no tacha nada, y si el autor "
                   "creia tachar algo, esta a la vista", huerfanos)

    # ---- 3. CONTROLES NEGATIVOS: el pack sabe fallar, y sabe no fallar ----
    b.titulo("controles negativos")
    ciegos = []
    for ent in DEROGADAS:
        ej = ent["ejemplo"]
        ok = (_acusa(preparar("x.md", "Parrafo.\n\n%s\n" % ej), ent)
              and _perdona(preparar("x.md", "Parrafo.\n\n~~%s~~ -> corregido.\n" % ej), ent)
              and _acusa(preparar("x.html", "<p>%s</p>" % ej), ent)
              and _perdona(preparar("x.html", "<p><s>%s</s> corregido</p>" % ej), ent))
        if not ok:
            ciegos.append("%s '%s'" % (ent["dx"], ent["frase"]))
    b.control_negativo(
        not ciegos,
        "cada una de las %d frases: su ejemplo SIN TACHAR se acusa, y TACHADO (~~ en md, <s> "
        "en html) se perdona%s" % (len(DEROGADAS), (" -> CIEGOS: %s" % ciegos) if ciegos else ""))

    # Por su FRASE, no por su posicion: reordenar la lista no puede cambiar en silencio
    # contra que entrada se miden los controles de abajo.
    una = next((e for e in _CANON if e["frase"] == "una camara por poste"), None)
    fin = next((e for e in _CANON if e["frase"].startswith("el fin de carrera va a J14")), None)
    if una is None or fin is None:
        raise fw.Abortado("los controles negativos de abajo se miden contra dos entradas de la "
                          "lista ('una camara por poste' y 'el fin de carrera va a J14') y una "
                          "de ellas ya no esta: sin ellas no hay caso que romper")
    # El tachado no se hereda de fila en fila de una tabla, y SI cruza lineas dentro de un
    # parrafo (2_Manual_Hardware tacha asi un diagrama entero).
    tabla = preparar("x.md", "| a | ~~uno~~ ~~ |\n| b | el fin de carrera va a J14 ~~ |\n")
    multi = preparar("x.md", "~~Diagrama: una camara por poste, en p10 o en\np12.~~ -> dos.\n")
    b.control_negativo(
        _acusa(tabla, fin) and _perdona(multi, una),
        "un ~~ suelto en una fila de tabla no tacha la fila siguiente, y un ~~ de parrafo que "
        "abre en una linea y cierra en otra si tacha lo de en medio")

    # Solo el NUCLEO tiene que ir tachado; pero el nucleo SI.
    nucleo_ok = preparar("x.md", "Las camaras del cruce van a J16, ~~una por poste~~.\n")
    nucleo_mal = preparar("x.md", "~~Las camaras del cruce~~ van a J16, una por poste.\n")
    b.control_negativo(
        _perdona(nucleo_ok, una) and _acusa(nucleo_mal, una),
        "con el contexto sin tachar y el nucleo tachado se perdona; con el contexto tachado y "
        "el nucleo no, se acusa")

    # La excepcion de cita pide las DOS cosas.
    b.control_negativo(
        _acusa(preparar("x.md", "Aqui dice %suna camara por poste%s.\n" % (_AB, _CE)), una)
        and _acusa(preparar("x.md", "D-25 deroga: hay una camara por poste.\n"), una)
        and _acusa(preparar("x.md", "D-13 dice %suna camara por poste%s.\n" % (_AB, _CE)), una)
        and _perdona(preparar("x.md", "D-25 deroga %suna camara por poste%s.\n" % (_AB, _CE)), una)
        and _perdona(preparar("x.md", "Sus lineas %suna camara por poste%s\ncaducaron.\n" % (_AB, _CE)), una)
        is False
        and _perdona(preparar("x.md", "Nota (D-25):\nya no es %suna camara por poste%s.\n" % (_AB, _CE)), una),
        "la cita pide comillas Y marca de muerte (derog/caduc/su D-x) en su linea o la "
        "anterior: con una sola, o con la marca en la linea SIGUIENTE, se acusa igual; y la "
        "D-x de otra decision (D-13) no marca nada")

    # El recuadro del roadmap se excluye y NADA MAS; y la fe de erratas html (decia) marca.
    rm_sint = ("# R\n\n> ## %s\n> | D-25 | x | %suna camara por poste%s |\n\n"
               "Hoy hay una camara por poste.\n" % (TITULO_RECUADRO, _AB, _CE))
    res = buscar(preparar("roadmap.md", rm_sint, _recuadro(rm_sint)), una)
    decia = preparar("x.html", '<li><span class="decia">DECIA: una camara por poste</span>'
                               '<span class="cierto">una camara por poste</span></li>')
    rd = buscar(decia, una)
    b.control_negativo(
        len(res) == 2 and res[0][2] is True and res[1][2] is False
        and len(rd) == 2 and rd[0][2] is True and rd[1][2] is False,
        "la exclusion del recuadro cubre su bloque > y no la linea de despues, y class=\"decia\" "
        "marca su span y no el de al lado")
