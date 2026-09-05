# ===== banco/packs/costura_13_ambar_ordenado.py =====
#
# N-134 — EL AMBAR DEL ESCLAVO SE ORDENA, Y LA RED QUE LO CUBRIA SIGUE ARMADA.
#
# QUE PASO, MEDIDO EN BANCO Y NO LEIDO
# ------------------------------------
# Reportado el 04/09: "si le vuelvo a ambar, ese cambia ambar pero este no" -y 25 s
# despues, si-. El ambar del Esclavo NO LO ORDENABA NADIE: era el Esclavo rindiendose
# por orfandad (SFTY-6) al dejar de oir al Maestro, que se calla al entrar en el modo.
# El estado final era el correcto POR CASUALIDAD AFORTUNADA, y una casualidad no es una
# garantia: el dia que alguien toque el umbral de silencio, el ambar del cruce cambia de
# tiempo sin que nadie lo relacione con ese numero.
#
# LAS CUATRO PROPIEDADES QUE ESTE PACK VIGILA, Y POR QUE NINGUNA ES "existe el comando"
# -------------------------------------------------------------------------------------
# Un pack que solo supiera decir "existe CMD_GO_AMBAR" es basura el dia que se renombre.
# Aqui NO hay ni un nombre de comando escrito a mano: el comando del ambar SE DEDUCE del
# firmware -es aquel que la rama del despachador del Esclavo atiende llamando a la misma
# puerta por la que esa punta entra en ambar por orfandad-, y con ese nombre deducido se
# mira lo que hace el Maestro. Si el lector no encuentra lo que busca, ABORTA.
#
#   1. EL ROJO VA PRIMERO Y EL AMBAR DESPUES. No es estetico: el todo-rojo es el
#      intermedio seguro. Si la orden de ambar se pierde, el Esclavo queda PARADO -no
#      dando paso- hasta que la orfandad lo saque. Invertirlo abre una ventana en la que
#      el Esclavo esta en ambar mientras el Maestro aun no ha parado nada.
#   2. LA ORDEN NO DESARMA LA RED. La rama nueva NO refresca la marca de "ultimo
#      comando": si la refrescara, el Esclavo creeria que el Maestro sigue hablando y la
#      orfandad -que es justo la red cuando esta orden se pierde- se desarmaria
#      INMEDIATAMENTE DESPUES DE USARLA. Es la mas facil de romper sin darse cuenta y la
#      mas cara, porque el firmware seguiria funcionando en el caso bueno.
#      Y va con su mitad positiva: las ramas de GOBIERNO si la refrescan. Sin esa mitad
#      esto no mide una red, mide una tapia -un firmware que no refrescara la marca en
#      ningun sitio pasaria igual de bien-.
#   3. EL AMBAR ORDENADO Y EL AMBAR POR ORFANDAD TERMINAN EN LA MISMA FUNCION. Una
#      segunda forma de encender la misma lampara es la garantia de que el dia que se
#      toque una se olvidara la otra.
#   4. EL VETO DE SFTY-21 SE RESPETA IGUAL QUE EN LA ORFANDAD. Con el Modo Degradado
#      gobernando la luz no se obedece: alli decide el reloj con la configuracion que el
#      Maestro dejo verificada en las dos puntas. Se mide como SUBCONJUNTO -toda guarda
#      que protege la caida por orfandad protege tambien la orden-, no como una lista de
#      condiciones escrita aqui, que caducaria en cuanto se anadiera una quinta.
#
# LO QUE NO SE REPITE PORQUE YA ESTA CUBIERTO (y repetirlo seria la tercera copia de la
# misma tabla que este repositorio ya pago tres veces)
# -------------------------------------------------------------------------------------
#   - QUE LOS CODIGOS NO COLISIONEN: costura_03_comandos, primera comprobacion, sobre
#     TODOS los #define CMD_* de protocolo.h.
#   - QUE LAS DOS PUNTAS DECLAREN EL MISMO CODIGO: costura_01_contratos compara
#     include/protocolo.h byte a byte (SHA-256 del fichero entero) entre proyectos.
#   - QUE LO QUE EMITE EL MAESTRO LO ATIENDA EL ESCLAVO: costura_03_comandos censa
#     protocolo_enviarPaquete(CMD_*) en Maestro/src contra pkt.command == CMD_* en
#     Esclavo/src, sin lista a mano, asi que la orden nueva ya entra en ese censo sola.
#   - QUE LA ORDEN NO SAQUE DEL DEGRADADO POR LA LISTA DE GOBIERNO: costura_06_reanudacion
#     fija el literal de los tres comandos que llaman a degradado_salir(). Meter el ambar
#     ahi caeria alli, no aqui.
#   - QUE EL UMBRAL DE SILENCIO VIVA UNA SOLA VEZ: costura_08_silencio.
#   - QUE NINGUN FICHERO PONGA AMBAR SIN ENTERARSE DEL DEGRADADO: esclavo_08, pero A NIVEL
#     DE FICHERO. main.cpp consulta degradado_gobiernaLuz() en varios sitios, asi que
#     aquel pack lo da por bueno aunque la rama nueva no lo consultara: la comprobacion 4
#     de aqui es POR RAMA, que es la granularidad donde vive este defecto.
#
# LO QUE ESTE PACK NO PUEDE COMPROBAR — y va escrito aqui para que su verde no se lea
# como permiso (CLAUDE.md, "verde no es entregable")
# -------------------------------------------------------------------------------------
#   - NO EJERCE NADA. Lee el TEXTO del C++; no hay maquina de estados ni un solo tick.
#     Que el cruce entero llegue a ambar, y en cuanto tiempo, no lo dice este pack: lo
#     dice el banco con las dos tarjetas cargadas.
#   - NO MIDE EL AIRE. La orden va por radio a 2.4 kbps y no espera ACK a proposito. Que
#     llegue -y que llegue antes de que el Maestro se calle- es un hecho de la radio, no
#     del fuente. La red de orfandad existe justamente porque puede no llegar.
#   - NO MIDE EL TIEMPO ENTRE EL ROJO Y EL AMBAR. Comprueba el ORDEN de las dos
#     emisiones, no el hueco entre ellas ni si el Esclavo alcanza a aplicar el rojo antes
#     de recibir el ambar.
#   - NO DECIDE SI EL AMBAR ORDENADO ES LO QUE UN CONDUCTOR DEBE VER. Eso lo ve alguien
#     de pie en el cruce y es decision del responsable.
#   - LA PROPIEDAD 1 MIRA EL ORDEN DENTRO DE UNA FUNCION, no el orden en que las tramas
#     salen del transceptor. Si algun dia el envio se encolara y reordenara, esto seguiria
#     en verde midiendo el fuente.
#   - LA COMPROBACION 3 CENSA LLAMADAS DIRECTAS, DE UN NIVEL. Una segunda forma de
#     encender la lampara metida DENTRO de otra funcion -la rama llama a f(), y f() mueve
#     la luz- no se ve desde aqui. El paso de todo-rojo del Maestro si se resuelve a un
#     nivel de indireccion; el de la lampara no, y decirlo vale mas que fingir que si.
#   - NO MIDE QUE LA ORDEN SALGA SIEMPRE. Comprueba DONDE esta la emision, no si alguna
#     guarda futura pudiera saltarsela dentro de esa misma funcion.
#
# EJERCE SFTY-6: la caida a ambar por silencio de radio sigue siendo la red del ambar
#                ordenado -la orden no refresca la marca que la arma-.
# EJERCE SFTY-21: el ambar ordenado se veta con el Modo Degradado gobernando la luz,
#                 exactamente igual que la caida por orfandad.

import re

NOMBRE = "costura_13_ambar_ordenado"
DESCRIPCION = ("el ambar del Esclavo se ordena tras el todo-rojo y no desarma la "
               "orfandad que lo cubre (N-134)")

ESCLAVO_MAIN = ("Esclavo", "src", "main.cpp")

# El unico nombre del PROTOCOLO que este pack escribe es el del umbral de silencio, y
# solo para RECONOCER el bloque de la orfandad. Si desaparece, el pack ABORTA: sin ese
# bloque no hay nada contra lo que comparar la orden, y aprobar entonces seria fabricar
# un PASS. costura_08 ya vigila que ese numero exista y viva una sola vez.
CTE_SILENCIO = "SFTY6_SILENCIO_MS"

# La barrera de salidas (CLAUDE.md §6): la unica API por la que se mueve una luz. Se
# nombra el prefijo, no una funcion concreta, para que el censo siga valiendo cuando
# aparezca una funcion nueva.
PREFIJO_LUZ = "semaforo_"

# El paso de todo-rojo se DEDUCE: una funcion del Maestro que ponga su propia luz en
# rojo Y mande una orden por radio. Este si es un nombre concreto, y por eso su ausencia
# ABORTA en vez de aprobar.
ROJO_LOCAL = "semaforo_forzarRojo"

RE_EMISION = re.compile(r"\bprotocolo_enviarPaquete\s*\(\s*(CMD_\w+)\s*\)")
# Una llamada escrita como SENTENCIA. Se admiten argumentos -sin parentesis dentro-
# porque una segunda forma de encender la lampara puede llevarlos: buscar solo "f();"
# dejaria pasar un semaforo_forzarAmbar(true) sin que la comprobacion 3 lo viera.
RE_LLAMADA_STMT = re.compile(r"\b(\w+)\s*\([^;()]*\)\s*;")
RE_DEF = re.compile(r"(?m)^[A-Za-z_][\w\s\*&:]*?\b(\w+)\s*\([^;{]*\)\s*\{")
RE_RAMA = re.compile(r"^\s*pkt\s*\.\s*command\s*==\s*(CMD_\w+)\s*$")


# --------------------------------------------------------------------------------
# Lectores. Con parentesis y llaves equilibradas, no con expresiones regulares: las
# condiciones de main.cpp llevan llamadas a funcion dentro, y partirlas por el primer
# ')' dejaria fuera justo la mitad donde vive la guarda que se busca -el pack aprobaria
# por no haber leido, que es la forma exacta de la prueba muerta de §3.bis-.
# --------------------------------------------------------------------------------

def _cierre(cod, i, abre, cierra):
    """Indice del delimitador que cierra al que empieza en i. None si no cierra."""
    prof = 0
    for j in range(i, len(cod)):
        if cod[j] == abre:
            prof += 1
        elif cod[j] == cierra:
            prof -= 1
            if prof == 0:
                return j
    return None


def _bloques_if(cod):
    """[(condicion, ini_cuerpo, fin_cuerpo)] de cada 'if' del texto.

    Incluye los 'else if': se reconocen por el mismo 'if' y su cuerpo es el suyo. Un
    cuerpo sin llaves llega hasta el ';', porque una guarda de una linea veta igual."""
    fuera = []
    for m in re.finditer(r"\bif\s*\(", cod):
        i = m.end() - 1
        fin_cond = _cierre(cod, i, "(", ")")
        if fin_cond is None:
            continue
        cond = cod[i + 1:fin_cond]
        k = fin_cond + 1
        while k < len(cod) and cod[k].isspace():
            k += 1
        if k < len(cod) and cod[k] == "{":
            fin = _cierre(cod, k, "{", "}")
            if fin is not None:
                fuera.append((cond, k + 1, fin))
        else:
            fin = cod.find(";", k)
            if fin != -1:
                fuera.append((cond, k, fin + 1))
    return fuera


def _definiciones(cod):
    """[(nombre, ini_cuerpo, fin_cuerpo)] de las funciones definidas en un .cpp."""
    fuera = []
    for m in RE_DEF.finditer(cod):
        k = cod.find("{", m.end() - 1)
        if k == -1:
            continue
        fin = _cierre(cod, k, "{", "}")
        if fin is not None:
            fuera.append((m.group(1), k + 1, fin))
    return fuera


def _atomos(cond):
    """La condicion partida por los '&&' de primer nivel, normalizada.

    No se tocan los parentesis: los dos lados de la comparacion se normalizan igual, y
    recortarlos convertiria '!degradado_gobiernaLuz()' en algo que ya no se reconoce."""
    partes, prof, act, i = [], 0, "", 0
    while i < len(cond):
        c = cond[i]
        if c == "(":
            prof += 1
        elif c == ")":
            prof -= 1
        if prof == 0 and cond.startswith("&&", i):
            partes.append(act)
            act = ""
            i += 2
            continue
        act += c
        i += 1
    partes.append(act)
    return {re.sub(r"\s+", " ", p).strip() for p in partes if p.strip()}


def _guardas_de(bloques, idx):
    """Todos los atomos de todos los 'if' que envuelven al indice idx."""
    fuera = set()
    for cond, ini, fin in bloques:
        if ini <= idx < fin:
            fuera |= _atomos(cond)
    return fuera


def _llamadas_stmt(txt, prefijo=""):
    """Llamadas escritas como SENTENCIA: 'f();' o 'f(x);'.

    Con el ';' a proposito. Sin el, 'semaforo_estado() != S_FALLO' -un getter dentro de
    una guarda- contaria como una forma de encender la luz, y la comparacion entre la
    puerta ordenada y la de la orfandad mediria ruido."""
    return {m.group(1) for m in RE_LLAMADA_STMT.finditer(txt)
            if m.group(1).startswith(prefijo)}


# --------------------------------------------------------------------------------
# La medida. Una sola funcion, para que el firmware real y el firmware con el defecto
# inyectado pasen EXACTAMENTE por el mismo lector. Si midieran por caminos distintos,
# el control negativo demostraria que funciona otra cosa.
# --------------------------------------------------------------------------------

def _medir(fw):
    cod_e = fw.codigo(*ESCLAVO_MAIN)
    bloques_e = _bloques_if(cod_e)

    # ---- El bloque de la orfandad: se reconoce por el umbral, no por su posicion ----
    orf = [(c, i, f) for c, i, f in bloques_e if CTE_SILENCIO in c]
    if len(orf) != 1:
        raise fw.Abortado(
            "en Esclavo/src/main.cpp hay %d 'if' cuya condicion nombre %s. Ese bloque es "
            "la caida a ambar por orfandad, y es LA REFERENCIA contra la que se mide la "
            "orden: con ninguno no hay red que comparar y con varios el pack no sabria "
            "cual es. En los dos casos estaria midiendo otra cosa"
            % (len(orf), CTE_SILENCIO))
    cond_orf, ini_orf, fin_orf = orf[0]
    cuerpo_orf = cod_e[ini_orf:fin_orf]

    # ---- La marca de "ultimo comando": se lee de la propia condicion ----
    m = re.search(r"millis\s*\(\s*\)\s*-\s*(\w+)\s*>", re.sub(r"\s+", " ", cond_orf))
    if not m:
        raise fw.Abortado(
            "la condicion de la orfandad (%r) ya no tiene la forma 'millis() - marca > "
            "umbral'. La marca de ultimo comando es lo que la comprobacion 2 vigila; sin "
            "poder leerla de ahi habria que escribir su nombre a mano, que es exactamente "
            "el valor por defecto que este banco no admite" % cond_orf.strip())
    MARCA = m.group(1)

    # ---- La puerta del ambar: la funcion de luz que llama la orfandad ----
    puertas_orf = _llamadas_stmt(cuerpo_orf, PREFIJO_LUZ)
    if len(puertas_orf) != 1:
        raise fw.Abortado(
            "el bloque de la orfandad llama a %d funciones %s*() como sentencia (%s). La "
            "puerta del ambar tiene que ser UNA para poder exigir que la orden use esa "
            "misma; con varias no hay 'la misma' que exigir"
            % (len(puertas_orf), PREFIJO_LUZ, sorted(puertas_orf)))
    PUERTA = sorted(puertas_orf)[0]

    # ---- El despachador, partido en ramas por su condicion exacta ----
    ramas = {}
    for cond, ini, fin in bloques_e:
        mr = RE_RAMA.match(re.sub(r"\s+", " ", cond))
        if mr:
            ramas[mr.group(1)] = (ini, fin)
    if len(ramas) < 3:
        raise fw.Abortado(
            "el despachador de Esclavo/src/main.cpp solo dio %d rama(s) con la forma "
            "'pkt.command == CMD_X' (%s). Si cambio de forma, este pack estaria midiendo "
            "un conjunto casi vacio, que aprueba cualquier cosa"
            % (len(ramas), sorted(ramas)))

    # ---- El comando del ambar SE DEDUCE: la rama que usa la puerta de la orfandad ----
    cand = sorted(c for c, (i, f) in ramas.items()
                  if PUERTA in _llamadas_stmt(cod_e[i:f], PREFIJO_LUZ))
    if len(cand) != 1:
        raise fw.Abortado(
            "hay %d rama(s) del despachador del Esclavo que llaman a %s() (%s). Con "
            "ninguna el ambar del Esclavo NO SE ORDENA -vuelve a depender de la orfandad, "
            "que es el defecto de N-134- y con varias hay dos ordenes distintas para la "
            "misma lampara" % (len(cand), PUERTA, cand))
    CMD_AMBAR = cand[0]
    ini_ra, fin_ra = ramas[CMD_AMBAR]
    cuerpo_rama = cod_e[ini_ra:fin_ra]

    # ---- Quien refresca la marca, rama por rama ----
    re_marca = re.compile(r"\b%s\s*=" % re.escape(MARCA))
    refrescan = sorted(c for c, (i, f) in ramas.items() if re_marca.search(cod_e[i:f]))

    # ---- La emision de la orden en el Maestro. Censando el DIRECTORIO ----
    emisiones = []
    for f_ in fw.fuentes_de("Maestro", "src"):
        cod_m = fw.codigo("Maestro", "src", f_)
        for me in RE_EMISION.finditer(cod_m):
            if me.group(1) == CMD_AMBAR:
                emisiones.append((f_, cod_m, me.start()))
    if len(emisiones) != 1:
        raise fw.Abortado(
            "en Maestro/src hay %d emision(es) de %s (%s). El Esclavo atiende esa orden, "
            "asi que con ninguna la rama del Esclavo es codigo muerto y con varias el "
            "ambar del cruce se pide desde dos sitios: el orden rojo-ambar solo se puede "
            "medir sobre uno" % (len(emisiones), CMD_AMBAR, [e[0] for e in emisiones]))
    fich_m, cod_m, pos_ambar = emisiones[0]

    # ---- La funcion que la emite, y el paso de todo-rojo que debe precederla ----
    dueno = [(n, i, f) for n, i, f in _definiciones(cod_m) if i <= pos_ambar < f]
    if len(dueno) != 1:
        raise fw.Abortado(
            "no se pudo situar la emision de %s dentro de UNA funcion de %s (candidatas: "
            "%s). Sin saber que funcion es, el orden rojo-ambar no se puede leer"
            % (CMD_AMBAR, fich_m, [d[0] for d in dueno]))
    n_dueno, ini_d, fin_d = dueno[0]
    cuerpo_setup = cod_m[ini_d:fin_d]

    # Un paso de todo-rojo es una funcion del Maestro que pone SU luz en rojo y ademas
    # manda una orden por radio: las dos puntas paradas. Se deduce del fuente; escribir
    # su nombre aqui seria dar por sabido lo que se mide.
    pasos_rojo = {}
    for f_ in fw.fuentes_de("Maestro", "src"):
        c_ = fw.codigo("Maestro", "src", f_)
        for n, i, f in _definiciones(c_):
            cuerpo = c_[i:f]
            if ROJO_LOCAL in _llamadas_stmt(cuerpo, PREFIJO_LUZ) and RE_EMISION.search(cuerpo):
                pasos_rojo[n] = f_
    if not pasos_rojo:
        raise fw.Abortado(
            "no se hallo en Maestro/src ninguna funcion que llame a %s() y ademas emita "
            "una orden por radio. Ese es el paso de todo-rojo -el intermedio seguro- y "
            "sin encontrarlo el pack no puede decir si el ambar va antes o despues de el"
            % ROJO_LOCAL)

    # Posiciones, DENTRO de la funcion del ambar, de cada paso de todo-rojo.
    pos_rojos = sorted(mm.start() for n in pasos_rojo
                       for mm in re.finditer(r"\b%s\s*\(" % re.escape(n), cuerpo_setup))
    # Relativa al cuerpo, calculada de la posicion ABSOLUTA que ya se midio: buscar otra
    # vez el literal la haria depender de como este escrito el espaciado.
    pos_amb_rel = pos_ambar - ini_d

    return {
        "PUERTA": PUERTA, "MARCA": MARCA, "CMD_AMBAR": CMD_AMBAR,
        "ramas": sorted(ramas), "refrescan": refrescan,
        "pasos_rojo": pasos_rojo, "n_dueno": n_dueno, "fich_m": fich_m,
        "cuerpo_rama": cuerpo_rama, "cuerpo_setup": cuerpo_setup,
        # --- Las cuatro propiedades, ya resueltas a booleano ---
        "orden_ok": bool(pos_rojos) and all(p < pos_amb_rel for p in pos_rojos),
        "n_rojos_en_setup": len(pos_rojos),
        "refresca_la_orden": CMD_AMBAR in refrescan,
        "puertas_orfandad": puertas_orf,
        "puertas_ordenada": _llamadas_stmt(cuerpo_rama, PREFIJO_LUZ),
        "guardas_orfandad": {a for a in _guardas_de(bloques_e, ini_orf + cuerpo_orf.index(PUERTA))
                             if "millis" not in a},
        "guardas_ordenada": _guardas_de(bloques_e, ini_ra + cuerpo_rama.index(PUERTA)),
    }


# --------------------------------------------------------------------------------
# El lector con el defecto inyectado EN MEMORIA (CLAUDE.md §8.bis).
#
# Los .cpp reales NO se tocan: un arnes que edita el firmware para probarse deja el
# arbol sucio si algo revienta a mitad, y aqui hay cuatro inyecciones. Se envuelve el
# lector y se devuelve el fuente parcheado.
#
# Y REVIENTA SI UN PARCHE NO ENCUENTRA SU ANCLA. Sin eso, un ancla caducada dejaria el
# control negativo "fallando bien" sin haber inyectado defecto alguno: el pack diria que
# sabe detectar y estaria detectando el firmware sano. Es la prueba muerta de §3.bis
# aplicada al propio control negativo.
# --------------------------------------------------------------------------------

class _Parcheado:
    def __init__(self, fw, parches):
        self._fw = fw
        self._parches = parches      # {(punta, carpeta, fichero): [(ancla, reemplazo)]}
        # Se anota QUE parche se aplico, no CUANTAS veces: un mismo fichero se lee
        # varias veces en una medida -el censo del directorio pasa por el- y contar
        # aplicaciones daria un numero que depende del recorrido del lector, no de si
        # el defecto entro. Lo que hay que exigir es que ningun parche se quede fuera.
        self.aplicados = set()
        self.Abortado = fw.Abortado

    def _aplicar(self, partes, t):
        for ancla, rep in self._parches.get(tuple(partes), ()):
            if ancla not in t:
                raise self._fw.Abortado(
                    "el parche del control negativo no encontro su ancla en %s. El ancla "
                    "se extrae del fuente REAL en esta misma corrida, asi que no "
                    "encontrarla significa que el lector cambio de idea entre dos "
                    "lecturas: el control negativo no habria inyectado nada y su verde no "
                    "valdria nada" % "/".join(partes))
            t = t.replace(ancla, rep, 1)
            self.aplicados.add((tuple(partes), ancla))
        return t

    def codigo(self, *partes):
        return self._aplicar(partes, self._fw.codigo(*partes))

    def texto(self, *partes):
        return self._fw.texto(*partes)

    def fuentes_de(self, *a, **k):
        return self._fw.fuentes_de(*a, **k)


def _con_defecto(fw, parches):
    """Mide sobre el firmware parcheado y exige que TODOS los parches se aplicaran."""
    p = _Parcheado(fw, parches)
    try:
        med = _medir(p)
    except fw.Abortado:
        # Un parche que deja el fuente ilegible NO es "detecto el defecto": es que el
        # pack dejo de saber leer. Se distingue devolviendo None, y el control negativo
        # lo cuenta como roto.
        med = None
    pedidos = {(k, a) for k, v in parches.items() for a, _ in v}
    if p.aplicados != pedidos:
        raise fw.Abortado(
            "el control negativo pidio %d parche(s) y solo entro(aron) %d. Un parche que "
            "nunca llega a aplicarse deja el control negativo aprobando sobre el firmware "
            "SANO, y su verde diria que el pack sabe detectar un defecto que jamas se "
            "inyecto" % (len(pedidos), len(p.aplicados)))
    return med


def correr(b, fw):
    b.titulo("N-134: el ambar del Esclavo se ORDENA, y la orfandad sigue siendo su red")

    d = _medir(fw)

    # El censo se publica pero NO cuenta: es la medida sobre la que se apoyan las
    # comprobaciones, no una comprobacion. Contarlo inflaria el total con una linea que
    # ningun firmware puede fallar.
    b.reportar(
        "deducido del firmware, sin un solo nombre de comando escrito en el pack",
        ["puerta del ambar (la de la orfandad):  %s()" % d["PUERTA"],
         "marca de ultimo comando:               %s" % d["MARCA"],
         "orden del ambar (rama que usa la puerta): %s" % d["CMD_AMBAR"],
         "la emite %s() en Maestro/src/%s" % (d["n_dueno"], d["fich_m"]),
         "pasos de todo-rojo hallados en el Maestro: %s"
         % ", ".join("%s() [%s]" % (n, f) for n, f in sorted(d["pasos_rojo"].items())),
         "ramas del despachador que refrescan la marca: %s" % ", ".join(d["refrescan"])])

    # ---- 1. El rojo va PRIMERO y el ambar despues -------------------------------
    b.verificar(
        d["orden_ok"],
        "%s() manda el todo-rojo ANTES de la orden de ambar (%d paso(s) de todo-rojo, "
        "todos por delante de la emision de %s): si la orden de ambar se pierde, el "
        "Esclavo queda PARADO hasta que la orfandad lo saque, que es la direccion segura"
        % (d["n_dueno"], d["n_rojos_en_setup"], d["CMD_AMBAR"]),
        "SFTY: en %s() la orden de %s NO va precedida del todo-rojo (%d paso(s) de "
        "todo-rojo hallados). Queda una ventana en la que el Esclavo esta en ambar "
        "-invitando a negociar el paso- mientras el Maestro aun no ha parado nada"
        % (d["n_dueno"], d["CMD_AMBAR"], d["n_rojos_en_setup"]))

    # ---- 2a. La orden NO desarma la red ------------------------------------------
    b.verificar(
        not d["refresca_la_orden"],
        "la rama de %s NO asigna %s: la orden no le hace creer al Esclavo que el Maestro "
        "sigue hablando, asi que la caida por orfandad -la red para cuando esta misma "
        "orden se pierda- sigue armada justo despues de usarla"
        % (d["CMD_AMBAR"], d["MARCA"]),
        "la rama de %s refresca %s. La orden DESARMA su propia red: el Maestro se calla a "
        "continuacion a proposito, y el Esclavo acaba de reiniciar la cuenta de silencio "
        "que era lo unico que lo cubria si la orden no hubiera llegado"
        % (d["CMD_AMBAR"], d["MARCA"]))

    # ---- 2b. Y la mitad que impide que esto sea una tapia ------------------------
    # Sin esta linea, un firmware que NO refrescara la marca en ningun sitio pasaria la
    # de arriba igual de bien. No se estaria midiendo una red: se estaria midiendo una
    # tapia (CLAUDE.md §8.sexies).
    otras = [c for c in d["refrescan"] if c != d["CMD_AMBAR"]]
    b.verificar(
        len(otras) >= 2,
        "y la marca SI se refresca desde las %d ramas de gobierno (%s): la de arriba mide "
        "una excepcion deliberada, no un firmware que no refresque nunca"
        % (len(otras), ", ".join(otras)),
        "solo %d rama(s) del despachador refrescan %s (%s). Con la marca casi sin "
        "refrescar, el 'no la refresca' de arriba no dice nada: el Esclavo caeria a ambar "
        "por silencio con el Maestro hablando" % (len(otras), d["MARCA"], otras))

    # ---- 3. La misma puerta, no una segunda lampara ------------------------------
    b.verificar(
        d["puertas_ordenada"] == d["puertas_orfandad"] and len(d["puertas_ordenada"]) == 1,
        "el ambar ordenado y el ambar por orfandad terminan en la MISMA funcion, %s(), y "
        "es la unica: no hay una segunda forma de encender esa lampara que se quede atras "
        "el dia que se toque la otra" % d["PUERTA"],
        "el ambar ordenado usa %s y la orfandad %s. Son dos caminos hasta la misma "
        "lampara: el dia que alguien cambie uno, el otro se queda como estaba y nadie lo "
        "relacionara" % (sorted(d["puertas_ordenada"]), sorted(d["puertas_orfandad"])))

    # ---- 4. El veto de SFTY-21, medido como subconjunto --------------------------
    faltan = sorted(d["guardas_orfandad"] - d["guardas_ordenada"])
    b.verificar(
        not faltan,
        "toda guarda que protege la caida por orfandad protege tambien la orden (%s): con "
        "el Modo Degradado gobernando la luz no se obedece, igual que alli"
        % ", ".join(sorted(d["guardas_orfandad"])),
        "SFTY-21: la rama de %s no repite %s, que si veta(n) la caida por orfandad. Una "
        "orden por radio contradiria al Modo Degradado, que es justo lo que el Degradado "
        "existe para no tener que atender" % (d["CMD_AMBAR"], faltan))

    _controles(b, fw, d)


def _controles(b, fw, d):
    """§8.bis: se rompe el firmware a proposito -en memoria- y se exige que caiga.

    Cada control mira LAS DOS CARAS: que el defecto se detecte, y que la comprobacion no
    acuse al firmware sano. Un detector que senala siempre es tan inutil como uno que no
    senala nunca, y solo la segunda cara distingue una comprobacion de una tapia."""
    b.titulo("Controles negativos: el firmware roto a proposito, en memoria")

    amb = "protocolo_enviarPaquete(%s);" % d["CMD_AMBAR"]

    # --- 1. Invertir el orden rojo/ambar ---
    # Se construye sobre el cuerpo REAL de la funcion, extraido en esta misma corrida:
    # el ancla no puede caducar sin que el pack lo grite.
    rojo_stmt = None
    for n in d["pasos_rojo"]:
        if "%s();" % n in d["cuerpo_setup"]:
            rojo_stmt = "%s();" % n
            break
    if rojo_stmt is None or amb not in d["cuerpo_setup"]:
        raise fw.Abortado(
            "no se pudo construir el parche del orden rojo/ambar: en %s() no aparecen "
            "como sentencia el paso de todo-rojo y la emision de %s. Sin poder inyectar "
            "el defecto, el control negativo no demuestra nada"
            % (d["n_dueno"], d["CMD_AMBAR"]))
    invertido = d["cuerpo_setup"].replace(amb, "", 1).replace(rojo_stmt, amb + " " + rojo_stmt, 1)
    m1 = _con_defecto(fw, {("Maestro", "src", d["fich_m"]): [(d["cuerpo_setup"], invertido)]})
    b.control_negativo(
        m1 is not None and not m1["orden_ok"] and d["orden_ok"],
        "con la orden de ambar movida DELANTE del todo-rojo, la comprobacion 1 cae — y "
        "sobre el firmware de hoy no acusa")

    # --- 2. Refrescar la marca en la rama nueva ---
    # Es el defecto mas facil de introducir sin darse cuenta: una linea que parece
    # simetrica con las otras ramas del despachador.
    con_refresco = d["cuerpo_rama"].replace("{", "{ %s = millis();" % d["MARCA"], 1) \
        if "{" in d["cuerpo_rama"] else ("%s = millis();" % d["MARCA"]) + d["cuerpo_rama"]
    m2 = _con_defecto(fw, {ESCLAVO_MAIN: [(d["cuerpo_rama"], con_refresco)]})
    b.control_negativo(
        m2 is not None and m2["refresca_la_orden"] and not d["refresca_la_orden"],
        "con un '%s = millis();' colado en la rama del ambar, la comprobacion 2a cae — y "
        "sobre el firmware de hoy no acusa" % d["MARCA"])

    # --- 3. Encender el ambar por otro camino ---
    otra = "%sponerAmbarOrdenado" % PREFIJO_LUZ
    por_otro = d["cuerpo_rama"].replace("%s();" % d["PUERTA"], "%s();" % otra, 1)
    if por_otro == d["cuerpo_rama"]:
        raise fw.Abortado(
            "no se pudo sustituir la llamada a %s() dentro de la rama de %s: sin inyectar "
            "la segunda lampara, el control de la comprobacion 3 no demuestra nada"
            % (d["PUERTA"], d["CMD_AMBAR"]))
    m3 = _con_defecto(fw, {ESCLAVO_MAIN: [(d["cuerpo_rama"], por_otro)]})
    # Al desaparecer la puerta compartida, la rama deja de ser deducible por lo que hace:
    # el lector ABORTA, que es la respuesta correcta -"ya no se cual es la orden del
    # ambar"- y NO un PASS. Se exige ese aborto, no un veredicto inventado.
    b.control_negativo(
        m3 is None,
        "con la rama encendiendo el ambar por %s() en vez de por la puerta de la "
        "orfandad, el lector ya no puede deducir cual es la orden del ambar y ABORTA — no "
        "aprueba" % otra)

    # --- 3.bis. La segunda lampara ANADIDA, no sustituida ---
    # Este es el caso que de verdad ejerce la comprobacion 3: la rama sigue llamando a la
    # puerta compartida -asi que el lector la sigue deduciendo- y ademas enciende por otro
    # camino. Sin este control, la 3 solo se habria visto ABORTAR, y un ABORTADO demuestra
    # que el lector se rinde, no que la comprobacion sepa acusar.
    doble = d["cuerpo_rama"].replace("%s();" % d["PUERTA"],
                                     "%s(); %s();" % (d["PUERTA"], otra), 1)
    m3b = _con_defecto(fw, {ESCLAVO_MAIN: [(d["cuerpo_rama"], doble)]})
    b.control_negativo(
        m3b is not None
        and m3b["puertas_ordenada"] != m3b["puertas_orfandad"]
        and d["puertas_ordenada"] == d["puertas_orfandad"],
        "con un %s() ANADIDO al lado de la puerta compartida, la comprobacion 3 cae "
        "-son dos formas de encender la misma lampara- y sobre el firmware de hoy no "
        "acusa" % otra)

    # --- 4. Retirar el veto del Modo Degradado de la rama ordenada ---
    sin_veto = None
    for a in sorted(d["guardas_orfandad"]):
        if "degradado" in a and a in d["cuerpo_rama"]:
            sin_veto = d["cuerpo_rama"].replace(a + " &&", "", 1)
            if sin_veto == d["cuerpo_rama"]:
                sin_veto = d["cuerpo_rama"].replace(a, "true", 1)
            break
    if sin_veto is None:
        raise fw.Abortado(
            "no se hallo en la rama de %s ninguna guarda del Modo Degradado que retirar "
            "para el control negativo de la comprobacion 4" % d["CMD_AMBAR"])
    m4 = _con_defecto(fw, {ESCLAVO_MAIN: [(d["cuerpo_rama"], sin_veto)]})
    b.control_negativo(
        m4 is not None and bool(m4["guardas_orfandad"] - m4["guardas_ordenada"]),
        "retirado el veto del Modo Degradado de la rama del ambar, la comprobacion 4 cae "
        "— y sobre el firmware de hoy no acusa")
