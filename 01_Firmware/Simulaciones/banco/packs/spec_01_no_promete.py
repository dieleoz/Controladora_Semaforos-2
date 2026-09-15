# -*- coding: utf-8 -*-
# ===== banco/packs/spec_01_no_promete.py =====
#
# POR QUE EXISTE ESTE PACK.
#
# Censado el 14/09: NINGUN instrumento del repositorio abria un 05_Funcional/SPEC_*.md.
# Cero. Las spec son "el documento DEL PRODUCTO" (CLAUDE.md 15.1) y eran lo unico de la
# lista de prioridades que nadie media. Ese mismo dia costo CUATRO afirmaciones falsas
# vivas a la vez, en cuatro documentos, sobre el mismo hecho: tres las cazo un revisor
# humano en tres vueltas y la cuarta salio por casualidad.
#
# Lo que se vigila es UN defecto concreto y mecanizable, el unico que un pack de TEXTO
# puede juzgar sin mentir:
#
#   UNA SPEC QUE NIEGA UN SIMBOLO QUE SI ESTA EN EL FUENTE, o que PROMETE uno que no.
#
# ---------------------------------------------------------------------------------
# EL BORDE, ESCRITO Y JUSTIFICADO (CLAUDE.md 7). LEASE ANTES QUE EL VERDE.
# ---------------------------------------------------------------------------------
#
# 1. UN PACK DE TEXTO NO CERTIFICA COMPORTAMIENTO (CLAUDE.md 6.3). Este pack NO dice
#    que la spec sea cierta, ni que este completa, ni que el firmware haga lo que ella
#    cuenta. Dice una sola cosa: que las frases de las spec que NOMBRAN UN SIMBOLO y
#    afirman o niegan su EXISTENCIA cuadran con el arbol de hoy. Un verde aqui NO es
#    "la spec esta certificada"; es "ninguna de las N frases de esta forma miente".
#
# 2. SE MIDIO EL PATRON ANTES DE ESCRIBIRLO, Y SALIO ESTRECHO A PROPOSITO. Sobre las
#    ocho SPEC del 14/09 (1.984 lineas vivas):
#      - "negacion en cualquier parte de la linea + simbolo en code span" da 280 avisos.
#      - la misma regla por FRASE, 130.
#      - acotada a verbos de existencia y por frase, 38 frases; de ellas SOLO 5 nombran
#        un simbolo, y de esas 5 TRES son falsos positivos: la negacion no habla del
#        simbolo sino de otra cosa de la frase ("`MAX_VERDE_BACKSTOP_MS` esta dimensionado
#        contra un maximo que ya no existe", "sobre un acuse que ya no existe, y
#        `CMD_CANCELA_AMBAR_ESCLAVO` NO se acusa", "tres motivos que no existen en el
#        Maestro ... (`rojoObligatorioMs()`)").
#    Por eso el molde NO acepta "negacion y simbolo en la misma frase": exige que el
#    simbolo este PEGADO al verbo de existencia. Con esa exigencia los tres falsos
#    positivos caen y quedan los dos casos reales. Un instrumento que da rojos falsos
#    se acaba ignorando, y entonces protege MENOS que no existir.
#
# 3. LO QUE ESTE PACK NO HABRIA CAZADO, y hay que decirlo porque es justo lo que paso
#    el 14/09. Las cuatro frases falsas de aquel dia, literales, y por que escapan:
#      (a) "LA PLUMA BAJA EN EL MISMO INSTANTE DEL ROJO. El retardo NO ESTA IMPLEMENTADO
#          ... no hay temporizador, ni estado intermedio, ni constante que ajustar"
#          -> NIEGA SIN NOMBRAR NINGUN SIMBOLO. Existia PLUMA_RETARDO_BAJADA_MS en las
#          dos puntas y la frase no lo escribe: no hay nada contra lo que medir.
#      (b) "NINGUNA CAMARA PROTEGE LA PLUMA ... `escribirPines()` no lee ninguna camara"
#          -> el simbolo existe y la spec NO dice que no exista: dice que no LEE algo.
#          Eso es una propiedad del CUERPO de una funcion, no de su existencia.
#      (c) "El veto es `A-1.bis`, abierto y sin construir" -> `A-1.bis` es un ancla de
#          decision, no un simbolo de C/JS. Eso lo mide decisiones_01_anclas.
#      (d) "la app pide ajuste de camara" -> promete una conducta sin nombrar simbolo.
#    Las cuatro estan CENSADAS abajo y se publican en cada corrida como hallazgo que NO
#    cuenta, para que el verde de este pack no se lea nunca como "aquello ya no puede
#    volver a pasar". Puede. Este pack cierra la forma con ancla; la forma SIN ancla
#    sigue abierta y su cuenta esta medida (apartado "el hueco", abajo).
#
# 4. EL CORPUS ES EL PRODUCTO, NO LOS INSTRUMENTOS. Se leen las tres puntas + el puente
#    (src/ e include/) y la app canonica (App_Semaforo/*.js y js/*.js). NO se leen
#    Validacion_*/ ni Simulaciones/: una spec es el documento del PRODUCTO, y un simbolo
#    que solo vive en un arnes no esta en el equipo. NO se leen las copias de app.js de
#    android/ ni de build/ (CLAUDE.md 14: son tres copias que pueden mentir; la canonica
#    es la de la raiz de la app).
#
# 5. LOS COMENTARIOS NO CUENTAN (CLAUDE.md 7.1). Aqui los comentarios CITAN lo que
#    explican -"semaforo_toggle() -que no tiene ningun llamador-"- y un grep crudo sale
#    inflado. El corpus va con los comentarios fuera. Efecto medido y deliberado: un
#    simbolo que solo se nombra en un comentario cuenta como AUSENTE, porque un
#    comentario no se compila.
#
# 6. LO TACHADO NO SE JUZGA. ~~...~~ es historia corregida a proposito: una frase falsa
#    dentro de un tachado es CORRECTA. El bloque viene literal de
#    documentos_04_cifras_sin_vigilante (CLAUDE.md 4: se trae el bloque probado, no se
#    reescribe). NOTA para quien lo busque: el encargo lo situaba en
#    documentos_01_cifras_del_acta y alli NO esta; esta en el 04.
#
# 7. EL NUMERO DE COMPROBACIONES NO DEPENDE DE LO QUE DIGAN LAS SPEC. Cada molde emite
#    UNA comprobacion que agrega todos sus casos, no una por frase. Hay tres agentes
#    reescribiendo las spec ahora mismo: con una comprobacion por frase, el total del
#    banco se moveria con cada edicion de un .md y documentos_01 no podria cuadrar
#    ninguna cifra (es el defecto de N-112, visto desde el otro lado).
#
# 8. LAS SPEC SE ENUMERAN POR PATRON, NUNCA POR LISTA. SPEC_*.md en 05_Funcional, primer
#    nivel. Una spec nueva entra sola; una que se parta o se renombre no deja un hueco
#    mudo. La lista escrita a mano es una afirmacion sobre el arbol y caduca (CLAUDE.md
#    14), y hoy el arbol se esta moviendo bajo los pies del pack.
#
# 9. SIN VALOR POR DEFECTO. Si no hay spec que leer, o el corpus del producto sale
#    vacio o por debajo de su suelo, esto ABORTA. ABORTADO NO ES PASS: no dice nada de
#    las spec. Un pack que cayera a "no encontre frases, luego todo cuadra" seria un
#    verde que no mide nada.

import os
import re
import unicodedata

NOMBRE = "spec_01_no_promete"
DESCRIPCION = "ninguna SPEC niega un simbolo que existe ni promete uno que no esta"

# ---------------------------------------------------------------------------------
# EL CORPUS DEL PRODUCTO
# ---------------------------------------------------------------------------------

PUNTAS = ("Maestro", "Esclavo", "Repetidor", "ESP32_Expansion")
CARPETAS = ("src", "include")

# La app canonica. Se excluyen por PREFIJO los ficheros que no son producto: los
# arneses de medida y el simulador de puente viven en la misma carpeta que app.js.
APP_DIR = ("05_Funcional", "App_Semaforo")
APP_FUERA = ("test_", "herramientas_", "servidor_")

# SUELOS. No son cifras bonitas: son el punto por debajo del cual el buscador se ha
# quedado ciego y tiene que decirlo en vez de aprobar (CLAUDE.md 7). El censo del
# 14/09 media 8 spec y 108 ficheros de producto; los suelos se ponen holgados porque
# hay agentes partiendo y renombrando spec a la vez, y un suelo pegado al techo
# abortaria por un movimiento legitimo.
SUELO_SPEC = 4
SUELO_FICHEROS = 40


def _sin_comentarios(t):
    """El fuente con los comentarios fuera. Vale igual para C++ y para JS."""
    t = re.sub(r"/\*.*?\*/", " ", t, flags=re.S)
    t = re.sub(r"//[^\n]*", " ", t)
    return t


def _corpus(fw):
    """(texto del producto sin comentarios, {nombre de fichero}). ABORTA si sale corto."""
    trozos = []
    nombres = set()
    for punta in PUNTAS:
        for carpeta in CARPETAS:
            if not os.path.isdir(os.path.join(fw.FIRMWARE, punta, carpeta)):
                continue
            for ext in (".cpp", ".h", ".ino"):
                for n in fw.fuentes_de(punta, carpeta, ext):
                    trozos.append(_sin_comentarios(fw.texto(punta, carpeta, n)))
                    nombres.add(n)
    base = os.path.join(fw.RAIZ_REPO, *APP_DIR)
    if not os.path.isdir(base):
        raise fw.Abortado("no existe %s: sin la app no se puede juzgar lo que las spec "
                          "afirman de ella" % os.path.join(*APP_DIR))
    for sub in ("", "js"):
        d = os.path.join(base, sub) if sub else base
        if not os.path.isdir(d):
            continue
        for n in sorted(os.listdir(d)):
            if not n.endswith(".js") or n.startswith(APP_FUERA):
                continue
            with open(os.path.join(d, n), "r", encoding="utf-8", errors="replace") as f:
                trozos.append(_sin_comentarios(f.read()))
            nombres.add(n)
    if len(trozos) < SUELO_FICHEROS:
        raise fw.Abortado(
            "el censo del producto solo encontro %d ficheros (suelo %d). Un corpus corto "
            "no aprueba las spec: hace que todo simbolo negado parezca ausente y todo "
            "simbolo prometido parezca inventado" % (len(trozos), SUELO_FICHEROS))
    return "\n".join(trozos), nombres


def _specs(fw):
    """[(nombre, texto)] de 05_Funcional/SPEC_*.md, POR PATRON. ABORTA si salen pocas."""
    d = os.path.join(fw.RAIZ_REPO, "05_Funcional")
    if not os.path.isdir(d):
        raise fw.Abortado("no existe 05_Funcional/: no hay spec que leer")
    nombres = sorted(n for n in os.listdir(d)
                     if n.startswith("SPEC_") and n.endswith(".md"))
    if len(nombres) < SUELO_SPEC:
        raise fw.Abortado(
            "solo se encontraron %d fichero(s) SPEC_*.md en 05_Funcional (suelo %d). O se "
            "renombraron, o el patron dejo de casar: un cero de censo no es 'no hay spec'"
            % (len(nombres), SUELO_SPEC))
    return [(n, fw.texto_repo("05_Funcional", n)) for n in nombres]


# ---------------------------------------------------------------------------------
# QUE ES UN SIMBOLO, Y QUE NO
# ---------------------------------------------------------------------------------
#
# Solo lo que tiene forma de identificador de C/JS. Medido sobre las spec del 14/09, lo
# que hay que dejar FUERA y que un patron ingenuo se traga: `SPEC_5` y `CLAUDE.md`
# (punteros a documentos, con forma de constante), `D-17.bis` y `A-1.bis` (anclas de
# decision), `STM32 -> ESP32` (un camino), `J16`/`PB0` (pines), `CMD:LEER_RTC` y
# `$STATUS` (trozos de protocolo). Ninguno de esos se puede buscar como identificador.

_RE_FICHERO = re.compile(r"[A-Za-z_][A-Za-z0-9_]*\.(?:cpp|h|ino|js)\Z")
_RE_CONSTANTE = re.compile(r"[A-Z][A-Z0-9]*(?:_[A-Z0-9]+)+\Z")
_RE_FUNCION = re.compile(r"[a-z][A-Za-z0-9_]*[a-z][A-Z][A-Za-z0-9_]*\Z")
_RE_SNAKE = re.compile(r"[a-z][a-z0-9]*(?:_[a-z0-9]+)+\Z")

# SPEC_1..SPEC_9 tienen forma de constante y son punteros a un documento. Se excluyen
# por patron, no por lista: manana habra SPEC_8 y SPEC_9.
_RE_PUNTERO_DOC = re.compile(r"SPEC_\d+\Z")


def simbolo(texto):
    """(clase, nombre) si el contenido de un code span es un identificador; None si no."""
    s = re.sub(r"\s*\(\s*\)\Z", "", texto.strip())
    if _RE_FICHERO.match(s):
        return ("fichero", s)
    if _RE_PUNTERO_DOC.match(s):
        return None
    if _RE_CONSTANTE.match(s):
        return ("constante", s)
    if _RE_FUNCION.match(s) or _RE_SNAKE.match(s):
        return ("funcion", s)
    return None


# ---------------------------------------------------------------------------------
# LO TACHADO NO SE JUZGA
# ---------------------------------------------------------------------------------
# Bloque traido LITERAL de documentos_04_cifras_sin_vigilante, con su motivo.

_RE_TACHADO = re.compile(r"~~.*?~~")


def _vivo(linea):
    """La linea sin sus tramos tachados.

    Lo tachado es historia declarada, y la historia NO se juzga: prohibirla empuja a
    borrarla, y una via descartada que desaparece en silencio se vuelve a proponer."""
    return _RE_TACHADO.sub(" ", linea)


# ---------------------------------------------------------------------------------
# LOS MOLDES
# ---------------------------------------------------------------------------------
#
# Una frase de spec se normaliza FUERA de los code spans -sin tildes, sin negritas, en
# minusculas- y los spans se dejan intactos, para que el molde case con la prosa y el
# simbolo salga con sus mayusculas. Sin quitar los `**` de Markdown, "no existe como
# tal** (`ENVIO_TRAMA_MS`)" no casaba con nada: la negrita se mete entre el verbo y su
# sujeto.

_RE_SPAN = re.compile(r"`([^`\n]+)`")


def _plano(t):
    t = unicodedata.normalize("NFD", t)
    t = "".join(c for c in t if unicodedata.category(c) != "Mn")
    # El fichero se mantiene en ASCII puro (CLAUDE.md 13: la consola de Windows viene en
    # cp1252). Los caracteres que si hacen falta van por su escape \\uXXXX, que el modulo
    # re entiende dentro del patron: \u2022 es la vineta de las listas de Markdown.
    return re.sub(r"[*_>|#\u2022]+", " ", t).lower()


def normalizar(frase):
    """La frase con la prosa en plano y los code spans intactos."""
    fuera = []
    i = 0
    for m in _RE_SPAN.finditer(frase):
        fuera.append(_plano(frase[i:m.start()]))
        fuera.append(m.group(0))
        i = m.end()
    fuera.append(_plano(frase[i:]))
    return "".join(fuera)


def frases(linea):
    """Parte una linea de Markdown en frases.

    Se corta por . ; : ! ? pero NUNCA dentro de un code span: `app.js`, `D-17.bis` y
    `CMD:LEER_RTC` llevan puntos y dos puntos dentro, y cortar ahi parte el sujeto de
    su verbo. Los spans se tapan con \\x00 para buscar los cortes y se corta sobre la
    linea original."""
    hueco = _RE_SPAN.sub(lambda m: "\x00" * len(m.group(0)), linea)
    cortes = [0]
    for m in re.finditer(r"[.;:!?]\s+", hueco):
        cortes.append(m.end())
    cortes.append(len(linea))
    return [linea[cortes[i]:cortes[i + 1]] for i in range(len(cortes) - 1)]


# Relleno admitido entre el verbo de existencia y el simbolo. Es corto y CERRADO a
# proposito: es lo unico que separa "no existen `camara.cpp`" -donde la negacion habla
# del simbolo- de "un maximo que ya no existe ... `MAX_VERDE_BACKSTOP_MS`", donde habla
# de otra cosa. Cada palabra de esta lista se leyo en una spec del 14/09.
_RELLENO = (r"(?:(?:como\s+tal|en\s+el\s+firmware|en\s+el\s+codigo|en\s+el\s+fuente|"
            r"en\s+ninguna\s+punta|ningun|ninguna|un|una|el|la|los|las|de|ni|y)\s+){0,3}")
# Adornos que sobreviven a _plano: parentesis, comillas y espacios.
_ADORNO = r"[\s,:(\u00ab\u00bb\"']*"

# M1 - LA SPEC NIEGA QUE EL SIMBOLO EXISTA.
_M1 = (
    re.compile(r"\bno\s+existen?\b" + _ADORNO + _RELLENO + _ADORNO + r"`([^`\n]+)`"),
    re.compile(r"`([^`\n]+)`" + _ADORNO +
               r"(?:no\s+existe\b|no\s+est[ae]n?\s+implementad|no\s+se\s+ha\s+escrito)"),
)

# M2 - LA SPEC PROMETE QUE EL SIMBOLO EXISTE.
# El (?<!no ) y el (?<!ya no ) no son adorno: sin ellos M2 casa dentro de cada frase
# que M1 ya juzga, y el mismo simbolo saldria a la vez negado y prometido.
_M2 = (
    re.compile(r"(?<!no )(?<!ya no )\bexisten?\b" + _ADORNO + _RELLENO + _ADORNO +
               r"`([^`\n]+)`"),
    re.compile(r"`([^`\n]+)`" + _ADORNO +
               r"(?:si\s+existe\b|ya\s+existe\b|est[ae]n?\s+construid|"
               r"est[ae]n?\s+implementad)"),
)

# M3 - LA SPEC NIEGA QUE NADIE LLAME AL SIMBOLO (CLAUDE.md 6.1, la pregunta 1).
# Es la unica afirmacion sobre el USO que se puede medir sin leer el cuerpo de nada.
_M3 = (
    re.compile(r"`([^`\n]+)`" + _ADORNO +
               r"(?:no\s+tiene\s+(?:ningun\s+)?llamador|"
               r"no\s+se\s+usa\s+en\s+ninguna\s+linea|"
               r"no\s+(?:lo|la)\s+llama\s+nadie|nadie\s+(?:lo|la)\s+llama|"
               r"sin\s+(?:ningun\s+)?llamador)"),
    re.compile(r"(?:nadie\s+llama\s+a|sin\s+llamador\s+de)" + _ADORNO + r"`([^`\n]+)`"),
)

# Cadena: "No existen `camara.cpp` ni `talanquera.cpp`" son DOS sujetos del mismo verbo.
# Se sigue la cadena solo por ni / y / , y solo si lo que hay en medio es un span.
_RE_CADENA = re.compile(r"\A[\s,]*(?:ni|y|\u00b7)?[\s,]*`([^`\n]+)`")


def _cadena(resto):
    """Los spans encadenados a un span que acaba de casar."""
    fuera = []
    while True:
        m = _RE_CADENA.match(resto)
        if not m:
            return fuera
        fuera.append(m.group(1))
        resto = resto[m.end():]


def casos(nombre, texto, moldes):
    """[(fichero, linea, clase, simbolo, frase)] de un molde sobre una spec."""
    fuera = []
    for n, linea in enumerate(texto.splitlines(), 1):
        for frase in frases(_vivo(linea)):
            norma = normalizar(frase)
            for rx in moldes:
                for m in rx.finditer(norma):
                    crudos = [m.group(1)]
                    if rx is moldes[0]:
                        crudos += _cadena(norma[m.end():])
                    for c in crudos:
                        s = simbolo(c)
                        if s:
                            fuera.append((nombre, n, s[0], s[1], frase.strip()))
    # Un mismo simbolo puede casar por los dos moldes de la pareja; se cuenta una vez.
    vistos, unicos = set(), []
    for c in fuera:
        clave = (c[0], c[1], c[3])
        if clave not in vistos:
            vistos.add(clave)
            unicos.append(c)
    return unicos


# ---------------------------------------------------------------------------------
# LA MEDIDA CONTRA EL ARBOL
# ---------------------------------------------------------------------------------
#
# LA BUSQUEDA NO ES LA MISMA EN LAS DOS DIRECCIONES, Y ESO ES DELIBERADO:
#
#   M1 (la spec NIEGA) falla cuando el simbolo APARECE -> se busca ESTRICTO, por token
#   exacto. Generoso daria rojos falsos: `AMBAR_VIGENTE` "aparece" dentro de
#   `DEG_RECHAZO_AMBAR_VIGENTE` sin ser el mismo simbolo.
#
#   M2 (la spec PROMETE) falla cuando el simbolo NO aparece -> se busca GENEROSO: token
#   exacto, o como COMPONENTE de un identificador mas largo en frontera de guion bajo.
#   Sin eso, `GO_GREEN` (que el firmware escribe `CMD_GO_GREEN`) y `motivoL2` (que es
#   `modo_degradado_motivoL2`) saldrian inventados, y las spec citan por la parte corta.
#
# En las dos, la duda va a favor de la spec: un pack que da rojos falsos se desactiva.


def presente_estricto(corpus, nombres, clase, s):
    if clase == "fichero":
        return s in nombres
    return re.search(r"(?<![A-Za-z0-9_])%s(?![A-Za-z0-9_])" % re.escape(s),
                     corpus) is not None


def presente_generoso(corpus, nombres, clase, s):
    if presente_estricto(corpus, nombres, clase, s):
        return True
    if clase == "fichero":
        return False
    return re.search(r"(?<![A-Za-z0-9])_?%s(?![A-Za-z0-9])" % re.escape(s),
                     corpus) is not None


# Un USO es una aparicion del simbolo seguida de '(' que NO es su declaracion ni su
# definicion. Se distinguen por lo que va JUSTO ANTES: una declaracion lleva delante su
# tipo -un identificador-, y una llamada lleva delante un operador, un separador o el
# principio de una sentencia. Las palabras clave que pueden preceder a una llamada y
# parecen un tipo van en _NO_TIPO.
_NO_TIPO = ("return", "else", "do", "case", "and", "or", "not", "new", "delete",
            "await", "typeof", "throw")
_RE_ANTES_LLAMADA = re.compile(r"(?:[=(,!&|+\-*/<>{};:?\[\]]|\A|\n)\s*\Z")


def usos(corpus, s):
    """Cuantas veces se LLAMA al simbolo en el producto (sin contar declaracion)."""
    n = 0
    for m in re.finditer(r"(?<![A-Za-z0-9_])%s\s*\(" % re.escape(s), corpus):
        antes = corpus[max(0, m.start() - 80):m.start()]
        if _RE_ANTES_LLAMADA.search(antes):
            n += 1
            continue
        ult = re.search(r"([A-Za-z_][A-Za-z0-9_]*)\s*\Z", antes)
        if ult and ult.group(1) in _NO_TIPO:
            n += 1
    return n


# ---------------------------------------------------------------------------------
# EL HUECO, MEDIDO: LAS NEGACIONES QUE NO LLEVAN ANCLA
# ---------------------------------------------------------------------------------
#
# Es la parte del defecto del 14/09 que este pack NO cierra, y se publica con su CUENTA
# porque una cuenta se puede seguir y una frase no (CLAUDE.md 1: lo que se vigila es la
# cuenta). Una negacion sin simbolo no se puede volver a medir por nadie: es una
# AFIRMACION SOBRE EL CODIGO escrita de forma que ningun instrumento la puede heredar
# (CLAUDE.md 6, la excepcion). El 14/09 eran 33 de 38.

_NEGACIONES = (
    r"no (?:esta|estan) implementad", r"sin construir", r"no existen?", r"ni existen?",
    r"no se ha (?:escrito|construido|implementado|programado)",
    r"no hay (?:constante|temporizador|estado|funcion|codigo)",
    r"no se construye", r"nadie (?:lo|la) (?:llama|construye|implementa)",
)
_RE_NEGACION = re.compile("|".join(_NEGACIONES))


def negaciones_sin_ancla(nombre, texto):
    fuera = []
    for n, linea in enumerate(texto.splitlines(), 1):
        for frase in frases(_vivo(linea)):
            norma = normalizar(frase)
            if not _RE_NEGACION.search(_plano(re.sub(r"`[^`\n]*`", " ", norma))):
                continue
            if any(simbolo(m.group(1)) for m in _RE_SPAN.finditer(frase)):
                continue
            fuera.append((nombre, n, frase.strip()))
    return fuera


# Las cuatro del 14/09, literales y en ASCII. Se publican en cada corrida (ver el borde,
# punto 3): son el recordatorio de que el verde de este pack cubre UNA forma, no el
# defecto entero.
CUATRO_DEL_14 = (
    ("SPEC_1, retirada en ef0fb4a",
     "\"El retardo NO ESTA IMPLEMENTADO ... no hay temporizador, ni estado intermedio, "
     "ni constante que ajustar\" -- existia PLUMA_RETARDO_BAJADA_MS en las dos puntas",
     "NIEGA SIN NOMBRAR SIMBOLO: no hay ancla contra la que medir"),
    ("SPEC_5, retirada en 96a30e5",
     "\"NINGUNA CAMARA PROTEGE LA PLUMA ... escribirPines() no lee ninguna camara\" -- "
     "la leia",
     "el simbolo EXISTE y la spec no niega su existencia: niega lo que hace su CUERPO"),
    ("SPEC_5, retirada en 96a30e5",
     "\"El veto de la pluma es A-1.bis: ABIERTO Y SIN CONSTRUIR\" -- construido",
     "A-1.bis es un ancla de decision, no un simbolo de C/JS (eso es decisiones_01)"),
    # La cuarta NO se retiro: se CUMPLIO construyendo. El 14/09 era falsa -app.js no
    # trataba el aviso- y desde f57a401 lo traduce js/aviso_camara_pluma.js. Se queda
    # en la lista porque lo que publica es la ceguera del pack, y esa sigue: si la app
    # volviera a tirar el aviso, este pack seguiria en verde.
    ("D-33 y SPEC_4 hueco 9, cumplida construyendo en f57a401",
     "\"la app pide ajuste de camara\" -- el 14/09 app.js no trataba ese aviso",
     "PROMETE una conducta sin nombrar simbolo"),
)


def correr(b, fw):
    corpus, nombres = _corpus(fw)
    specs = _specs(fw)

    # ---- 1. El censo ----
    b.titulo("el censo: las spec por patron y el producto por directorio")
    b.verificar(
        len(specs) >= SUELO_SPEC,
        "censadas %d spec por patron SPEC_*.md (suelo %d): %s"
        % (len(specs), SUELO_SPEC, ", ".join(n for n, _ in specs)),
        "solo %d spec censadas" % len(specs))
    b.verificar(
        len(nombres) >= SUELO_FICHEROS and len(corpus) > 100000,
        "censados %d ficheros de producto, %d caracteres sin comentarios"
        % (len(nombres), len(corpus)),
        "el corpus del producto salio corto (%d ficheros, %d caracteres): con el corpus "
        "corto este pack no mide las spec, mide su propia ceguera"
        % (len(nombres), len(corpus)))

    # ---- 2. M1: lo que la spec NIEGA no puede estar en el fuente ----
    m1 = [c for n, t in specs for c in casos(n, t, _M1)]
    b.titulo("M1 - la spec niega un simbolo (%d frase(s))" % len(m1))
    malos = []
    for doc, ln, clase, s, frase in m1:
        hay = presente_estricto(corpus, nombres, clase, s)
        # LA FRASE SE IMPRIME AQUI Y NO EN EL MENSAJE DE FALLA, y no es estilo.
        # compuerta.py se queda con la ULTIMA linea de la salida del banco que lleve una
        # barra, un digito y la palabra PASS o comprobacion, y los mensajes de FALLA se
        # imprimen DESPUES del RESUMEN: un trozo de spec con "13/14 comprobaciones"
        # dentro se colaria en el acta EN LUGAR de las cifras del banco. Es la trampa
        # que documentos_01 dejo medida. Aqui la frase sale ANTES del RESUMEN, donde no
        # puede suplantar nada, y el mensaje de FALLA lleva solo fichero y linea.
        print("         %s:%d  %s %s -> %s | %s"
              % (doc, ln, clase, s, "ESTA EN EL FUENTE" if hay else "ausente, ok",
                 frase[:110]))
        if hay:
            malos.append("%s:%d dice que no existe `%s` y esta en el producto"
                         % (doc, ln, s))
    b.verificar(
        not malos,
        "los %d simbolo(s) que las spec declaran INEXISTENTES no estan en el producto"
        % len(m1),
        "%d spec niegan un simbolo QUE SI ESTA: %s. Una spec que niega lo construido "
        "manda a construir lo que ya existe, y el que la exporte firma algo falso "
        "(CLAUDE.md 15)" % (len(malos), " | ".join(malos)))

    # ---- 3. M2: lo que la spec PROMETE tiene que estar ----
    m2 = [c for n, t in specs for c in casos(n, t, _M2)]
    b.titulo("M2 - la spec promete un simbolo (%d frase(s))" % len(m2))
    malos = []
    for doc, ln, clase, s, frase in m2:
        hay = presente_generoso(corpus, nombres, clase, s)
        print("         %s:%d  %s %s -> %s | %s"
              % (doc, ln, clase, s, "esta, ok" if hay else "NO APARECE", frase[:110]))
        if not hay:
            malos.append("%s:%d promete `%s` y no aparece en el producto"
                         % (doc, ln, s))
    b.verificar(
        not malos,
        "los %d simbolo(s) que las spec declaran EXISTENTES estan en el producto"
        % len(m2),
        "%d spec prometen un simbolo QUE NO ESTA: %s. Lo no implementado va a HUECOS "
        "MEDIDOS, nunca escrito como si existiera (CLAUDE.md 1 y 15)"
        % (len(malos), " | ".join(malos)))

    # ---- 4. M3: lo que la spec declara SIN LLAMADOR no puede tenerlo ----
    m3 = [c for n, t in specs for c in casos(n, t, _M3)]
    b.titulo("M3 - la spec declara un simbolo sin llamador (%d frase(s))" % len(m3))
    malos = []
    for doc, ln, clase, s, frase in m3:
        n_usos = usos(corpus, s)
        print("         %s:%d  %s -> %d llamada(s) en el producto | %s"
              % (doc, ln, s, n_usos, frase[:110]))
        if n_usos:
            malos.append("%s:%d dice que nadie llama a `%s` y tiene %d llamada(s)"
                         % (doc, ln, s, n_usos))
    b.verificar(
        not malos,
        "los %d simbolo(s) que las spec declaran SIN LLAMADOR no lo tienen" % len(m3),
        "%d spec declaran huerfano un simbolo QUE SI SE LLAMA: %s. Es el trinquete de "
        "CLAUDE.md 6.1 al reves: una huerfana que GANA llamador y sigue en la lista"
        % (len(malos), " | ".join(malos)))

    # ---- 5. Controles negativos ----
    b.titulo("controles negativos: el pack sabe fallar, y sabe no fallar")

    def juzgar(md, moldes):
        return [(c[2], c[3]) for c in casos("x.md", md, moldes)]

    # (a) M1 acusa una negacion FALSA y perdona una CIERTA. El caso malo es el del
    #     14/09 escrito con su ancla: la constante existe en las dos puntas.
    falsa = "No existe `PLUMA_RETARDO_BAJADA_MS` en ninguna punta.\n"
    cierta = "No existen `camara.cpp` ni `talanquera.cpp`: la camara vive en `botones.cpp`.\n"
    ac = juzgar(falsa, _M1)
    ci = juzgar(cierta, _M1)
    b.control_negativo(
        len(ac) == 1 and ac[0][1] == "PLUMA_RETARDO_BAJADA_MS"
        and presente_estricto(corpus, nombres, *ac[0])
        and {s for _, s in ci} == {"camara.cpp", "talanquera.cpp"}
        and not any(presente_estricto(corpus, nombres, c, s) for c, s in ci),
        "M1: 'No existe `PLUMA_RETARDO_BAJADA_MS`' se acusa -la constante esta en el "
        "fuente- y 'No existen `camara.cpp` ni `talanquera.cpp`' se perdona; y la cadena "
        "por 'ni' coge los DOS sujetos sin arrastrar el `botones.cpp` de despues de los "
        "dos puntos, que la frase AFIRMA")

    # (b) M2 acusa una promesa vacia y perdona una cumplida.
    vacia = juzgar("Ya existe `PLUMA_RETARDO_QUE_NADIE_ESCRIBIO_MS` en el firmware.\n", _M2)
    llena = juzgar("Ya existe `PLUMA_RETARDO_BAJADA_MS` en el firmware.\n", _M2)
    b.control_negativo(
        len(vacia) == 1 and not presente_generoso(corpus, nombres, *vacia[0])
        and len(llena) == 1 and presente_generoso(corpus, nombres, *llena[0]),
        "M2: una promesa de constante inventada se acusa y la misma frase con "
        "`PLUMA_RETARDO_BAJADA_MS` se perdona")

    # (c) M3 acusa declarar huerfano a quien tiene llamadores, y perdona al que no los
    #     tiene. Los dos sujetos son reales: escribirPines() es la barrera de salidas y
    #     semaforo_toggle() es el huerfano medido que la propia SPEC_1 publica.
    con_llamador = juzgar("`escribirPines()` no tiene ningun llamador.\n", _M3)
    sin_llamador = juzgar("`semaforo_toggle()` no tiene ningun llamador.\n", _M3)
    b.control_negativo(
        len(con_llamador) == 1 and usos(corpus, con_llamador[0][1]) > 0
        and len(sin_llamador) == 1 and usos(corpus, sin_llamador[0][1]) == 0,
        "M3: declarar huerfano a `escribirPines()` se acusa (%d llamadas) y declararlo de "
        "`semaforo_toggle()` se perdona (0)" % usos(corpus, "escribirPines"))

    # (d) Lo tachado no se juzga. La MISMA frase, dentro y fuera de ~~.
    b.control_negativo(
        len(juzgar(falsa, _M1)) == 1
        and len(juzgar("~~No existe `PLUMA_RETARDO_BAJADA_MS`~~ -> lo corrigio D-33.\n",
                       _M1)) == 0,
        "el tachado: la misma frase falsa se acusa suelta y se perdona dentro de ~~...~~, "
        "que es como este repositorio marca lo corregido a proposito")

    # (e) Un simbolo que SOLO vive en un comentario cuenta como ausente. Es el filtro
    #     que evita medir el propio ruido del repositorio (CLAUDE.md 7.1).
    solo_comentario = "SIMBOLO_QUE_SOLO_VIVE_EN_UN_COMENTARIO"
    crudo = "// " + solo_comentario + "\nint x;\n/* " + solo_comentario + " */\n"
    b.control_negativo(
        solo_comentario in crudo and solo_comentario not in _sin_comentarios(crudo),
        "el corpus va con los comentarios fuera: un simbolo que solo aparece en un // o "
        "en un /* */ cuenta como AUSENTE, porque un comentario no se compila")

    # (f) El clasificador de simbolos no se traga lo que no es un identificador. Los
    #     cinco casos estan medidos en las spec del 14/09.
    no_simbolos = ("SPEC_5", "D-17.bis", "A-1.bis", "STM32 -> ESP32", "J16", "PB0",
                   "CMD:LEER_RTC", "$STATUS", "CLAUDE.md", "PRUEBA ALCANCE")
    si_simbolos = ("PLUMA_RETARDO_BAJADA_MS", "escribirPines()", "app.js",
                   "semaforo.cpp", "modo_degradado_motivoL2")
    ciegos = [x for x in no_simbolos if simbolo(x)] + \
             [x for x in si_simbolos if not simbolo(x)]
    b.control_negativo(
        not ciegos,
        "el clasificador deja fuera punteros a documento, anclas de decision, pines, "
        "trozos de protocolo y rotulos de pantalla (%d casos), y deja dentro constantes, "
        "funciones y ficheros (%d)%s"
        % (len(no_simbolos), len(si_simbolos),
           (" -> CIEGO EN: %s" % ciegos) if ciegos else ""))

    # ---- 6. El hueco, y las cuatro del 14/09. NO CUENTAN (CLAUDE.md 4) ----
    sin_ancla = [x for n, t in specs for x in negaciones_sin_ancla(n, t)]
    con_ancla = len(m1)
    b.reportar(
        "%d negacion(es) de existencia SIN ANCLA frente a %d con ancla: este pack es "
        "CIEGO a las primeras, y son la mayoria" % (len(sin_ancla), con_ancla),
        ["%s:%d  %s" % (d, l, f[:120]) for d, l, f in sin_ancla[:40]]
        + (["... y %d mas" % (len(sin_ancla) - 40)] if len(sin_ancla) > 40 else [])
        + ["-> una negacion sin simbolo es una afirmacion sobre el codigo que NADIE puede "
           "volver a medir. Si esta cuenta sube y la de con-ancla no, las spec se estan "
           "escribiendo de forma que ningun instrumento las hereda (CLAUDE.md 6)"])
    b.reportar(
        "las CUATRO afirmaciones falsas del 14/09, y por que este pack NO habria cazado "
        "ninguna: su verde NO dice que aquello no pueda repetirse",
        ["%s: %s" % (d, por) for d, _, por in CUATRO_DEL_14]
        + ["-> lo que cierra este pack es la forma CON ancla. La forma sin ancla, y las "
           "afirmaciones sobre el CUERPO de una funcion, siguen sin instrumento"])
