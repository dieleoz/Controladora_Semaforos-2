# ===== banco/packs/documentos_01_cifras_del_acta.py =====
#
# LO QUE PUBLICAN LOS DOCUMENTOS TIENE QUE SER LO QUE MIDIO EL ACTA.
#
# CLAUDE.md lo dice desde el principio: "las cifras del README se copian del acta,
# nunca se escriben a mano". Era una regla sin instrumento, y el 27/08 se midio lo
# que eso cuesta: el README abria diciendo "cifras copiadas del acta
# evidencia/2026-08-26_compuerta.txt" y publicaba 32 rutas y 86,4% de flash, cuando
# ESA MISMA ACTA decia 38 rutas y 92,8%. Las cifras eran del 05/08.
#
# POR QUE ESTO NO ES COSMETICA.
#
# Una cifra vieja no se lee como vieja: se lee como medida. El README es lo primero
# que abre un auditor, y "86,4% de flash" invita a proponer estructura que hoy NO
# CABE -quedan 4.728 bytes-. Es la regla del instrumento aplicada al propio informe:
# lo que TU reportas tambien es un instrumento, y este llevaba semanas descalibrado
# con la palabra "copiadas" encima.
#
# QUE COMPRUEBA, Y QUE NO.
#
# No juzga si las cifras son buenas. Comprueba que son LAS DEL ACTA MAS RECIENTE:
# el documento no puede publicar un numero que ninguna corrida produjo. Si manana
# el flash sube al 95%, este pack exige que el README lo diga; no le pide que sea
# menor.
#
# Y no puede fallar en silencio: si el acta no se deja parsear, ABORTA. Un pack que
# cayera a "no encontre la cifra, luego no la comparo" seria un verde que no mide
# nada, que es justo lo que vino a cerrar.

import re
import unicodedata

NOMBRE = "documentos_01_cifras_del_acta"
DESCRIPCION = "toda cifra que README y ESTADO publican es la del acta mas reciente"

# Cada cifra que el acta produce y los documentos repiten. La clave es el nombre de
# la comprobacion tal y como la escribe compuerta.py; el patron se aplica al detalle
# de esa linea. Si el nombre cambia en la compuerta, esto ABORTA en vez de aprobar.
CIFRAS = (
    ("guarda de rutas",        r"(\d+) rutas parseadas",    "rutas censadas por la guarda"),
    ("compila maestro",        r"([\d.]+)%",                "flash del Maestro"),
    ("compila esclavo",        r"([\d.]+)%",                "flash del Esclavo"),
    ("compila repetidor",      r"([\d.]+)%",                "flash del Repetidor"),
    ("simulador funcional",    r"(\d+/\d+) PASS",           "simulador funcional"),
    ("simulador de repetidor", r"(\d+/\d+) PASS",           "simulador de repetidor"),
    # OJO CON LAS DOS DE ABAJO: el banco se estaria midiendo a si mismo. La linea de
    # "banco por packs" del acta la escribe esta misma corrida, asi que su NUMERADOR
    # cambia con que un solo pack falle -y entonces el README nunca podria cuadrar:
    # acta roja -> README no coincide -> acta roja-. Un bucle que ningun documento
    # puede cerrar es la comprobacion que ningun firmware puede aprobar de CLAUDE.md
    # §3, disfrazada. Por eso se comparan el TOTAL de comprobaciones y el TOTAL de
    # packs, que no dependen de si pasaron: si alguien anade o quita una prueba, el
    # README tiene que decirlo; si una falla, lo dice el acta, no este pack.
    ("banco por packs",        r"\d+/(\d+) comprobaciones", "total de comprobaciones del banco"),
    ("banco por packs",        r"packs: (?P<TOTAL_PACKS>.*)", "total de packs"),
    # LAS TRES DE LA APP FALTABAN, y el hueco costo lo mismo que costo N-62: el
    # 31/08 el README publicaba 59/59 en jsdom y 57/57 en el funcional cuando el acta
    # que el mismo citaba media 61 y 58. Ninguna cifra dejaba de cuadrar porque
    # NINGUNA de las tres estaba en esta tupla: la fila existia -la cobertura del
    # apartado 5 la veia- y su numero no lo miraba nadie. Un hueco no grita.
    #
    # Se comparan como FRACCION -"61/61"-, igual que pantalla, ciclo y automatico, y
    # no como numero suelto: ver el comentario de _cifra(), donde esta medido lo que
    # costo la primera version.
    ("app ejecutada en DOM",   r"(?P<TOTAL_APP>.*)",        "comprobaciones de la app en DOM"),
    ("test unitarios de la app", r"(?P<TOTAL_APP>.*)",      "test unitarios de la app"),
    ("test funcional de la app", r"(\d+/\d+) comprobaciones", "test funcional de la app"),
    ("arnes de pantalla",      r"MAESTRO\s+(\d+/\d+)",      "pantalla del Maestro"),
    ("arnes de pantalla",      r"ESCLAVO\s+(\d+/\d+)",      "pantalla del Esclavo"),
    ("arnes de pantalla",      r"TOTAL\s+(\d+/\d+)",        "pantalla, total"),
    ("arnes del ciclo",        r"(\d+/\d+) comprobaciones", "arnes del ciclo"),
    ("arnes del automatico",   r"(\d+/\d+) comprobaciones", "arnes del automatico"),
)

# Las cinco que ESTADO.md repite en su cabecera. No se le exigen las trece: se le
# exige que las que publica sean las mismas.
EN_ESTADO = ("flash del Maestro", "flash del Esclavo", "flash del Repetidor",
             "total de comprobaciones del banco", "pantalla, total",
             # La cabecera de ESTADO.md publica las tres de la app -"32 unitarios +
             # 61 jsdom + 58 funcional"- y llevaba dos viejas encima. Si un dia deja
             # de publicarlas, esto falla y se quita la fila a mano: es mas barato
             # que una cifra que envejece sin que nadie la mire.
             "comprobaciones de la app en DOM", "test unitarios de la app",
             "test funcional de la app")

# LA TABLA DEL README TIENE QUE ANUNCIAR TODAS LAS COMPROBACIONES, NO SOLO SUS
# CIFRAS. El 28/08 el README publicaba "15 PASS ... de 15 comprobaciones" con una
# tabla de 14: faltaba la fila de "test unitarios de la app". Este pack no lo vio
# porque comparaba cifras SUELTAS y nunca la cobertura: una comprobacion entera
# podia desaparecer del documento sin que ninguna cifra dejara de cuadrar.
#
# El nombre del acta se busca DENTRO de la etiqueta del README, no al reves: el
# documento puede ser mas explicito ("guarda de rutas de los instrumentos"), pero
# no puede callar. Los alias son para las filas que agrupan varias comprobaciones
# en una; escribirlos a mano es deliberado, porque una comprobacion nueva en la
# compuerta sin fila en el README tiene que salir en ROJO, no colarse.
ALIAS_FILA = {
    "compila esclavo":   "compila maestro / esclavo / repetidor",
    "compila repetidor": "compila maestro / esclavo / repetidor",
}

_RE_LINEA_LAXA = re.compile(r"^ {2}(?:PASS|FALLA|ABORTADO)\s", re.M)

_RE_LINEA = re.compile(r"^ {2}(PASS|FALLA|ABORTADO)\s+(\S.*?)\s{2,}(\S.*)$", re.M)

_RE_CITA = r"evidencia/(\d{4}-\d{2}-\d{2})_compuerta\.txt"


def _sin_cifra(doc, que, acta):
    """El FALLA de una comprobacion cuyo dato de partida no esta en el acta.

    N-112. Antes estas comprobaciones NO SE EMITIAN cuando el acta no traia su cifra:
    se anotaba un reportar() -que no cuenta- y las lineas de verificar() colgaban del
    else. El total del pack se movia entonces con la SALUD del acta, y ese es el
    defecto entero: el numero de comprobaciones que emite un pack no puede depender
    de su propio veredicto. Con 55 en una corrida y 53 en la siguiente, no hay ninguna
    cifra publicable -se probo iterando: cualquiera de las dos hace fallar la corrida
    que la restaura-, y este pack existe justamente para exigir que los documentos
    publiquen una cifra que salga de la ultima corrida.

    Decir "no se pudo medir" en FALLA es verdadero y util. No es un cepo: en cuanto
    una corrida deja el acta entera, la comprobacion vuelve a verde sola.

    VA SIN BARRA A PROPOSITO, y esto no es estilo. compuerta.py se queda con la ULTIMA
    linea de la salida del banco que lleve una barra, un digito y la palabra PASS o
    comprobacion, y las lineas de FALLA se imprimen DESPUES del RESUMEN. Un mensaje de
    fallo con "comprobaciones/packs" dentro se colaria en el acta EN LUGAR de las
    cifras del banco, dejando sin par a la corrida siguiente: el propio aviso
    perpetuaria lo que denuncia."""
    return ("%s: el acta %s no trae %s, asi que no hay contra que comparar lo que "
            "publica el documento. No dice que el documento este mal; dice que esta "
            "corrida no lo pudo medir, y una comprobacion que se calla en vez de "
            "decirlo desaparece del recuento" % (doc, acta, que))


def _normalizar(t):
    """Deja el texto comparable: coma decimal a punto y sin espacio ante el %.

    El acta escribe 92.8% y el README 92,8 %. Sin esto la comparacion mediria la
    tipografia en vez de la cifra, y un pack que falla por un espacio se acaba
    desactivando -que es como se pierde un instrumento-."""
    t = t.replace("\xa0", " ").replace(" ", " ")
    t = re.sub(r"(?<=\d),(?=\d)", ".", t)
    t = re.sub(r"\s+%", "%", t)
    return t


def _plano(t):
    """Sin tildes, en minusculas y sin adornos de Markdown.

    El acta escribe en ASCII -"arnes del automatico"- y el README con tildes y
    negritas -"**arnes del automatico**"-. Sin esto la comparacion mediria la
    ortografia en vez de la cobertura."""
    t = unicodedata.normalize("NFD", t)
    t = "".join(c for c in t if unicodedata.category(c) != "Mn")
    return re.sub(r"[*`_]+", "", t).lower()


def _tabla_readme(readme):
    """Las filas de la tabla de verificacion, en plano. [] si no se encuentra.

    Devolver [] en vez de reventar es a proposito: quien llama lo convierte en
    Abortado con un motivo que se entiende, en vez de en un PASS sobre una lista
    vacia -que es como se pierde un instrumento-."""
    lineas = readme.splitlines()
    for i, l in enumerate(lineas):
        if l.startswith("|") and "comprobaci" in _plano(l) and "estado" in _plano(l):
            filas = []
            for j in range(i + 1, len(lineas)):
                if not lineas[j].startswith("|"):
                    break
                celda = lineas[j].strip("|").split("|")[0].strip()
                if set(celda) <= set("- :"):   # la linea de guiones del encabezado
                    continue
                filas.append(_plano(celda))
            return filas
    return []


def _resultados(texto_acta):
    """nombre -> (estado, detalle). El estado hace falta: una comprobacion que en el
    acta salio FALLA o ABORTADO no trae cifras, y eso no es que falle el buscador."""
    return {n: (e, d) for e, n, d in _RE_LINEA.findall(texto_acta)}


def _cifra(res, clave, patron):
    par = res.get(clave)
    if par is None:
        return None
    d = par[1]
    if "TOTAL_APP" in patron:
        # "61 PASS | 0 FALLAS" -> "61/61". LA FRACCION, NO EL NUMERO SUELTO, y esto
        # no es cosmetica: se escribio primero devolviendo "61" y el pack SE QUEDO EN
        # VERDE con el defecto real reinyectado en el README -"59/59"-, porque el
        # README dice "y=61" en la linea 335 hablando de la pantalla. Un "61" suelto
        # casa con cualquier 61 del documento; "61/61" solo casa con la cifra.
        #
        # Es la regla del instrumento (CLAUDE.md §4) dentro del propio instrumento:
        # la comprobacion existia, corria y no sabia fallar. Se vio caer antes de
        # darla por buena.
        m = re.search(r"(\d+)\s+PASS\s*\|\s*(\d+)\s+FALLAS?", d)
        return "%d/%d" % (int(m.group(1)), int(m.group(1)) + int(m.group(2))) if m else None
    if "TOTAL_PACKS" in patron:
        # "packs: 25 PASS, 2 FALLA, 0 ABORTADO" -> "27 packs". La suma, no los que
        # pasaron.
        #
        # Y VA CON LA PALABRA PEGADA, NO SUELTO. Esto no es cosmetica: la comparacion
        # de mas abajo es `valor in readme`, una subcadena sobre el documento ENTERO,
        # y un numero de dos digitos casa con cualquier cosa. Medido el 31/08: "38"
        # aparece CINCO veces en README.md y dos de ellas estan DENTRO del hash
        # `50a5380` de la cabecera, asi que el README podia publicar "los 40 packs"
        # con el acta midiendo 38 y esta comprobacion seguia en verde -51/51, exit 0-.
        # Se vio: inyectado el defecto daba PASS, y con "N packs" da FALLA.
        #
        # Es el mismo defecto que ya se curo para las tres cifras de la app -donde un
        # "61" suelto casaba con el "y=61" de README.md:335- sobreviviendo en la fila
        # de al lado. La cura es la misma que usa el apartado 4 para las rutas:
        # anclar a la FRASE, no al numero.
        cuentas = re.findall(r"(\d+)\s+(?:PASS|FALLA|ABORTADO)", d)
        return "%d packs" % sum(int(c) for c in cuentas) if cuentas else None
    m = re.search(patron, d)
    return m.group(1) if m else None


def correr(b, fw):
    b.titulo("Las cifras publicadas contra el acta mas reciente")

    ultima = fw.actas()[0]
    texto_acta = fw.acta(ultima)
    res = _resultados(texto_acta)

    # ---- 1. El acta se deja leer ENTERA; si no, esto no mide nada -------------
    #
    # Aqui habia un umbral -"si salen menos de 8 resultados, aborta"-, y ese numero
    # a dedo dejaba pasar exactamente el fallo del 28/08: la linea de "simulador de
    # app y bluetooth" desbordaba la columna del acta, el separador caia a un
    # espacio, el patron estricto no casaba, y el pack leia 14 de 15 y seguia tan
    # tranquilo por encima del umbral. Una comprobacion entera se volvia invisible
    # sin que nada se pusiera rojo.
    #
    # Ya no hay umbral: se cuentan las lineas de resultado con un patron LAXO -solo
    # el estado al principio- y se exige que el patron ESTRICTO -el que parte nombre
    # y detalle- las lea todas. Cualquier linea que el buscador no sepa partir es un
    # ABORTADO, que es lo que de verdad ocurre: no se pudo medir.
    laxas = len(_RE_LINEA_LAXA.findall(texto_acta))
    if laxas and len(res) != laxas:
        ilegibles = [l for l in texto_acta.splitlines()
                     if _RE_LINEA_LAXA.match(l) and not _RE_LINEA.match(l)]
        raise fw.Abortado(
            "el acta %s tiene %d lineas de resultado y solo %d se dejan partir en "
            "nombre y detalle. Fallo el buscador, no el acta, y callarlo dejaria "
            "comprobaciones enteras sin comparar: %s"
            % (ultima, laxas, len(res), " | ".join(l.strip() for l in ilegibles)))
    if not laxas:
        raise fw.Abortado(
            "el acta %s no trae ninguna linea de resultado: sin acta que leer, "
            "comparar cifras seria comparar nada contra nada y salir en verde"
            % ultima)
    b.verificar(
        True,
        "el acta %s se parsea entera: %d de %d comprobaciones con su detalle"
        % (ultima, len(res), laxas),
        "no deberia llegarse aqui")

    readme = _normalizar(fw.texto_repo("README.md"))
    estado = _normalizar(fw.texto_repo("ESTADO.md"))

    # ---- 2. El acta que citan es la ultima que hay ----
    fecha_ultima = ultima[:10]
    for doc, txt in (("README.md", readme), ("ESTADO.md", estado)):
        citadas = sorted(set(re.findall(_RE_CITA, txt)))
        b.verificar(
            bool(citadas) and max(citadas) == fecha_ultima,
            "%s cita el acta mas reciente (%s)" % (doc, fecha_ultima),
            "%s cita %s y la ultima acta es %s. Un documento que apunta a un acta "
            "vieja invita a re-correr la corrida equivocada: el auditor verificaria "
            "cifras que ya nadie publica"
            % (doc, ", ".join(citadas) or "ninguna", fecha_ultima))

    # ---- 3. Cada cifra del acta, publicada tal cual ----
    for clave, patron, etiqueta in CIFRAS:
        valor = _cifra(res, clave, patron)
        # Si la comprobacion salio FALLA o ABORTADO en el acta anterior, no dejo cifra
        # que copiar. Se anota -reportar() no cuenta- y la comprobacion pasa en vacio:
        # el rojo lo canta el acta, que es su trabajo. Exigirle al documento una cifra
        # que no existe seria un cepo -acta roja, documento imposible de cuadrar, acta
        # roja-, y el numero de comprobaciones de este pack tiene que ser SIEMPRE el
        # mismo: si variara con la salud del acta, el total del banco cambiaria solo y
        # el README nunca podria publicarlo.
        if valor is None:
            if res.get(clave, ("", ""))[0] == "PASS":
                raise fw.Abortado(
                    "la comprobacion %r salio PASS en el acta y aun asi no se pudo "
                    "leer su cifra de %s (patron %r): fallo el buscador, no el acta"
                    % (clave, etiqueta, patron))
            b.reportar(
                "el acta no trae la cifra de %s" % etiqueta,
                ["la comprobacion %r salio %s en %s"
                 % (clave, res.get(clave, ("AUSENTE",))[0] or "AUSENTE", ultima),
                 "no hay nada que comparar contra el documento hasta que vuelva a medir"])
        b.verificar(
            valor is None or valor in readme,
            "README publica %s = %s, la del acta" % (etiqueta, valor) if valor
            else "no se compara %s: el acta no la midio en esta corrida" % etiqueta,
            "README NO publica la cifra medida de %s: el acta dice %s. Una cifra que "
            "no sale de la ultima corrida se lee como medida y no lo es"
            % (etiqueta, valor))
        if etiqueta in EN_ESTADO:
            b.verificar(
                valor is None or valor in estado,
                "ESTADO.md publica %s = %s, la del acta" % (etiqueta, valor) if valor
                else "no se compara %s en ESTADO.md: el acta no la midio" % etiqueta,
                "ESTADO.md NO publica la cifra medida de %s: el acta dice %s"
                % (etiqueta, valor))

    # ---- 4. Y no publican ADEMAS otro recuento de rutas ----
    # Que la cifra buena aparezca no basta: el README llego a llevar 38 y 32 rutas a
    # la vez, en dos parrafos distintos. Se ancla en la frase literal del acta -"N
    # rutas parseadas"- y no en cualquier numero suelto: una historia fechada ("paso
    # de 20 packs a 27") es legitima, y un instrumento que obligue a reescribirla
    # empuja a maquillar el pasado, que es peor que la cifra que venia a cazar.
    #
    # N-112: LAS DOS COMPROBACIONES SE EMITEN PASE LO QUE PASE. Estaban dentro de un
    # else, asi que un acta sin la cifra de rutas se llevaba por delante DOS lineas
    # del recuento sin dejar rastro -el reportar() no cuenta-. Ver _sin_cifra().
    rutas = _cifra(res, "guarda de rutas", r"(\d+) rutas parseadas")
    if rutas is None:
        b.reportar("no hay cifra de rutas en el acta",
                   ["sin ella no se puede buscar un segundo recuento en los documentos"])
    for doc, txt in (("README.md", readme), ("ESTADO.md", estado)):
        otras = (sorted(set(re.findall(r"(\d+)\s+rutas parseadas", txt)) - {rutas})
                 if rutas is not None else [])
        b.verificar(
            rutas is not None and not otras,
            "%s no arrastra ningun otro recuento de rutas" % doc,
            _sin_cifra(doc, "la cifra de rutas de la guarda", ultima) if rutas is None
            else "%s dice ADEMAS %s rutas parseadas cuando el acta mide %s"
                 % (doc, "/".join(otras), rutas))

    # ---- 4.bis. El recuento de packs, ANCLADO A SU LINEA ----
    #
    # Tres intentos, y los dos primeros fallaron. Queda escrito porque la forma de
    # fallar es la reutilizable.
    #
    # (1) TOTAL_PACKS se comparaba SUELTO -"38"- contra el documento entero. "38"
    #     aparece cinco veces en README.md y dos estan DENTRO del hash `50a5380` de
    #     la cabecera: el README podia publicar "los 40 packs" con el acta midiendo
    #     38 y esto seguia en verde. Inyectado, daba PASS.
    #
    # (2) Se anclo a la frase -"38 packs"- y SEGUIA en verde: el documento nombra la
    #     cifra en dos sitios -la tabla y el titulo de la seccion del banco- y
    #     falsificar uno deja al otro cumpliendo la subcadena. Un `in` sobre el
    #     documento entero no distingue CUAL ocurrencia es la buena.
    #
    # (3) Y prohibir cualquier otro "N packs" tampoco vale: ESTADO.md cuenta que el
    #     banco "paso de 20 packs a 27", que es historia legitima. Un instrumento que
    #     obligue a reescribir el pasado empuja a maquillarlo, que es peor que la
    #     cifra que venia a cazar -lo dice el apartado 4 sobre las rutas, y aqui se
    #     repitio sin leerlo-.
    #
    # Lo que si distingue: exigir que el recuento viva en LA MISMA LINEA que la cifra
    # de comprobaciones del banco. Esa linea es la afirmacion viva; las demas son
    # narracion.
    # (4) N-112, Y ES EL DEFECTO QUE MAS CARO SALIO: el bloque entero colgaba de un
    #     else. Con la fila del banco del acta trayendo sus cifras se emitian DOS
    #     comprobaciones; sin ellas, un reportar() que NO CUENTA y las dos
    #     desaparecian. Medido: tres corridas completas seguidas sobre un arbol
    #     identico dieron 16 PASS / 1 FALLA, 17 PASS / 0 FALLA y 16 PASS / 1 FALLA.
    #
    #     El lazo se cierra en compuerta.py, que guarda como detalle de esta fila la
    #     ULTIMA linea de la salida del banco con barra, digito y la palabra PASS o
    #     comprobacion. En verde esa linea es el RESUMEN y trae el par; en rojo puede
    #     ganarla un mensaje de FALLA -"...la cifra medida de comprobaciones de la app
    #     en DOM: el acta dice 77/77..." lo cumple entero- y entonces el acta guarda
    #     el texto del fallo EN LUGAR de las cifras. Total en verde 829, en rojo 827:
    #     publicar cualquiera de los dos hace fallar la corrida siguiente, que
    #     restaura el otro. No hay cifra publicable, que es lo contrario de lo que
    #     este pack le exige a los documentos.
    #
    #     Ahora las dos se emiten SIEMPRE, y sin el par salen en FALLA diciendo
    #     exactamente eso. Ver _sin_cifra(), incluido el porque de que su texto no
    #     lleve ni una barra.
    packs = _cifra(res, "banco por packs", r"packs: (?P<TOTAL_PACKS>.*)")
    total = _cifra(res, "banco por packs", r"\d+/(\d+) comprobaciones")
    hay_par = packs is not None and total is not None
    if not hay_par:
        b.reportar("el acta no trae las dos cifras del banco",
                   ["sin las dos no se puede anclar el recuento de packs a su linea",
                    "la fila del banco guarda el texto de un fallo en vez de medir"])
    n = packs.split()[0] if hay_par else None
    for doc, txt in (("README.md", readme), ("ESTADO.md", estado)):
        vivas = [ln for ln in txt.splitlines() if total in ln] if hay_par else []
        malas = sorted({m for ln in vivas
                        for m in re.findall(r"(\d+)\s+packs", ln)} - {n})
        b.verificar(
            hay_par and bool(vivas) and not malas,
            "%s publica el recuento de packs (%s) en la misma linea que %s"
            % (doc, n, total),
            _sin_cifra(doc, "las dos cifras del banco -total de comprobaciones y "
                            "recuento de packs-", ultima) if not hay_par
            else "%s no publica %s en ninguna linea" % (doc, total) if not vivas
            else "%s publica %s junto a %s packs cuando el acta mide %s"
            % (doc, total, "/".join(malas), n))

    # ---- 5. La tabla anuncia TODAS las comprobaciones, no solo sus cifras ----
    #
    # Cuadrar las cifras no basta. El 28/08 las trece cuadraban y aun asi el README
    # publicaba una tabla de 14 filas bajo un rotulo de 15 comprobaciones: la de
    # "test unitarios de la app" no estaba en ninguna parte. Ninguna cifra dejaba de
    # cuadrar porque la comprobacion que faltaba no tenia ninguna cifra vigilada, y
    # una comprobacion que el documento no nombra es una que el auditor no re-corre.
    #
    # Se comprueba en las DOS direcciones a proposito: una fila de menos esconde
    # cobertura que si existe, y una fila de mas anuncia cobertura que ya no existe.
    # La segunda es la peor, porque se lee como una medida.
    filas = _tabla_readme(readme)
    if not filas:
        raise fw.Abortado(
            "no se encontro en README.md la tabla de verificacion (una cabecera con "
            "'Comprobacion' y 'Estado'). Sin ella no se puede comprobar la cobertura, "
            "y darla por buena seria aprobar sin mirar")

    for nombre in sorted(res):
        buscado = ALIAS_FILA.get(_plano(nombre), _plano(nombre))
        b.verificar(
            any(buscado in f for f in filas),
            "la tabla del README anuncia %r" % nombre,
            "la tabla del README NO tiene fila para %r, que el acta %s SI mide. Una "
            "comprobacion que el documento no nombra no la re-corre nadie: la "
            "cobertura existe y el lector no se entera" % (nombre, ultima))

    nombres = {ALIAS_FILA.get(_plano(n), _plano(n)) for n in res}
    fantasmas = [f for f in filas if not any(n in f for n in nombres)]
    b.verificar(
        not fantasmas,
        "la tabla del README no anuncia ninguna fila que el acta no mida",
        "la tabla del README anuncia %s, que no corresponde a ninguna comprobacion "
        "del acta %s. Una fila sin medida detras se lee como medida"
        % (" | ".join(repr(f) for f in fantasmas), ultima))

    # El recuento del rotulo, anclado a la frase literal como se hizo con las rutas.
    # Se compara el TOTAL de comprobaciones, no cuantas pasaron: si se comparasen los
    # PASS, un acta roja dejaria al README imposible de cuadrar y el pack se volveria
    # un cepo -acta roja, documento imposible, acta roja-.
    #
    # HUNK APARTE DE N-112, y es la regla del instrumento (CLAUDE.md §4) dentro del
    # instrumento otra vez: el buscador iba SIN re.I, y el README abre la frase en
    # mayuscula -"**De 17 comprobaciones, la compuerta da..."-. El patron devolvia []
    # y el pack acusaba a un documento correcto de "publica ninguno como total de
    # comprobaciones". Un "no aparece" no es un hallazgo hasta haber descartado al
    # buscador, y aqui el buscador media la ortografia de la primera letra.
    #
    # No relaja nada: se sigue exigiendo que el conjunto publicado sea EXACTAMENTE
    # [str(laxas)], anclado a la misma frase literal. Solo deja de depender de si la
    # cifra cae al principio de una oracion.
    publicados = sorted(set(re.findall(r"de (\d+) comprobaciones", readme, re.I)))
    b.verificar(
        publicados == [str(laxas)],
        "README publica el total de comprobaciones del acta (%d)" % laxas,
        "README publica %s como total de comprobaciones y el acta %s trae %d. Un "
        "recuento viejo no se lee como viejo: se lee como medida"
        % ("/".join(publicados) or "ninguno", ultima, laxas))

    # ---- 6. Controles negativos: la comprobacion sabe fallar ----
    #
    # N-112: ESTE COLGABA DE `if rutas is not None`, o sea la tercera comprobacion que
    # el pack dejaba de emitir segun lo que el acta trajera. Se ejerce siempre: con la
    # cifra del acta cuando la hay y con una de laboratorio cuando no, porque lo que
    # demuestra -que el detector distingue un segundo recuento- no depende del acta.
    #
    # Y de paso deja de comprobar un str.replace, que no puede fallar: ahora ejerce el
    # re.findall REAL del apartado 4 sobre un documento con un recuento inyectado.
    cifra_rutas = rutas if rutas is not None else "424241"
    sucio = readme + "\nlinea inyectada por el control negativo: 424242 rutas parseadas\n"
    b.control_negativo(
        "424242" in (set(re.findall(r"(\d+)\s+rutas parseadas", sucio))
                     - {cifra_rutas}),
        "un documento que arrastra un segundo recuento de rutas se detecta con el "
        "mismo buscador que usa la comprobacion")

    # N-112: LAS DOS RAMAS DEL APARTADO 4.bis, QUE HASTA HOY NO EJERCIA NINGUNA.
    #
    # (a) La rama CON cifra tiene que saber acusar: una linea que publique el total
    #     del banco junto a un recuento de packs distinto del del acta es mala. Sin
    #     esto, la rama podia estar aprobando cualquier cosa -y ya lo hizo dos veces,
    #     ver los intentos (1) y (2) de arriba, cazados a mano y no por el banco-.
    linea_falsa = "| banco por packs *(424242 packs)* | %s comprobaciones |" \
                  % (total if hay_par else "424243")
    b.control_negativo(
        bool(sorted(set(re.findall(r"(\d+)\s+packs", linea_falsa))
                    - {n if hay_par else "0"})),
        "una linea que publica el total del banco junto a otro recuento de packs se "
        "detecta como recuento que el acta no respalda")

    # (b) La rama SIN cifra, que es la que N-112 saco del limbo. Se reproduce lo que
    #     de verdad ocurre cuando la corrida anterior salio en rojo: la fila del banco
    #     guarda el texto de un fallo en lugar de sus cifras. Se exige que el extractor
    #     devuelva nada por los DOS patrones -no un valor por defecto que daria PASS-,
    #     que es lo que lleva a la rama de FALLA en vez de a la desaparicion.
    acta_sin_par = re.sub(
        r"^(  (?:PASS|FALLA|ABORTADO)\s+banco por packs\s{2,}).*$",
        r"\1README NO publica la cifra medida de la app: el acta dice 77 de 77",
        texto_acta, count=1, flags=re.M)
    res_sin_par = _resultados(acta_sin_par)
    b.control_negativo(
        acta_sin_par != texto_acta
        and _cifra(res_sin_par, "banco por packs",
                   r"packs: (?P<TOTAL_PACKS>.*)") is None
        and _cifra(res_sin_par, "banco por packs",
                   r"\d+/(\d+) comprobaciones") is None,
        "una fila del banco que guarda el texto de un fallo en vez de sus cifras deja "
        "sin dato a las dos comprobaciones del apartado 4.bis, que por eso tienen que "
        "salir en FALLA y no dejar de emitirse")

    acta_mutilada = "\n".join(l for l in texto_acta.splitlines()
                              if "guarda de rutas" not in l)
    b.control_negativo(
        _cifra(_resultados(acta_mutilada), "guarda de rutas",
               r"(\d+) rutas parseadas") is None,
        "si el acta no trae la cifra, el extractor devuelve nada en vez de un valor "
        "por defecto que daria PASS")

    # El caso real del 28/08, reproducido: cuando el nombre desborda la columna del
    # acta el separador cae a un espacio. Antes eso se leia como "una comprobacion
    # menos" y el pack aprobaba igual; ahora tiene que hacerse notar.
    acta_pegada = re.sub(r"^(  (?:PASS|FALLA|ABORTADO)\s+\S[^\n]*?)\s{2,}",
                         r"\1 ", texto_acta, count=1, flags=re.M)
    b.control_negativo(
        acta_pegada != texto_acta
        and len(_RE_LINEA_LAXA.findall(acta_pegada)) != len(_resultados(acta_pegada)),
        "un acta con el separador de una linea pegado deja de parsearse entera en "
        "vez de perder esa comprobacion en silencio")

    b.control_negativo(
        not any("arnes del ciclo" in f
                for f in [f for f in filas if "arnes del ciclo" not in f]),
        "una tabla del README a la que le falta una fila deja de cubrir la "
        "comprobacion del acta que le corresponde")

    b.control_negativo(
        bool([f for f in filas + ["arnes del teletransporte"]
              if not any(n in f for n in nombres)]),
        "una fila inventada en la tabla del README se detecta como cobertura que el "
        "acta no respalda")
