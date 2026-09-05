# ===== banco/packs/app_11_rangos_de_tiempos.py =====
#
# LOS RANGOS DE TIEMPO VIVEN EN TRES SITIOS Y EN TRES LENGUAJES, Y NADIE LOS CRUZABA.
#
#   Maestro/src/modo_automatico.cpp:32-34   C++    la guarda de verdad
#   App_Semaforo/app.js  enRango(...)       JS     lo que la app deja teclear
#   App_Semaforo/index.html  min= / max=    HTML   lo que el teclado numerico ofrece
#
# El 04/09 el responsable subio el minimo de verde y rojo de 1 a 3 minutos -"tres minutos
# es la minima distancia de seguridad"-, y al ir a cambiarlo aparecio que los seis numeros
# estaban escritos a mano en los tres sitios, sin nada que los atara.
#
# Es exactamente lo que contrato.h llama R-9 y este repositorio ya se ha cobrado tres
# veces (N-36, N-39, cfgVerdeRecibido): "repetir los rangos en dos lados es una segunda
# copia que alguien tiene que sincronizar, y el dia que difieran una punta deja pasar lo
# que la otra rechaza".
#
# QUE PASA SI DIVERGEN, para que se entienda por que esto no es cosmetica:
#   - app MAS PERMISIVA que el firmware -> el operario teclea un valor, la app lo acepta,
#     el equipo lo rechaza con $ERR y el tecnico se queda mirando un boton que no hizo
#     nada. Es el "OK mudo" al reves.
#   - app MAS ESTRICTA que el firmware -> hay configuraciones legitimas que nadie puede
#     poner desde la unica interfaz que existe.
#   - HTML distinto del JS -> el teclado del movil ofrece un rango y la validacion rechaza
#     otro. El operario pelea con su propio telefono.
#
# LO QUE ESTE PACK NO PUEDE COMPROBAR, y va escrito para que no se lea como permiso: que
# 3 minutos sea el numero CORRECTO. Eso es una decision vial y la tomo el responsable con
# su motivo. Aqui solo se exige que las tres copias digan lo mismo que el firmware.

import re

NOMBRE = "app_11_rangos_de_tiempos"
DESCRIPCION = "los rangos de tiempos dicen lo mismo en el C++, en app.js y en el HTML"

# N-137 (04/09): los limites se mudaron a include/limites_ciclo.h. Este pack ABORTO en
# la corrida siguiente, que es §5 haciendo su trabajo: los instrumentos leen el fuente
# POR RUTA, y mover contenido rompe al que lee. Un ABORTADO grita; lo que no se puede
# permitir es que siguiera midiendo sobre un fichero que ya no los tiene.
CPP = ("Maestro", "include", "limites_ciclo.h")
APP_JS = ("05_Funcional", "App_Semaforo", "app.js")
APP_HTML = ("05_Funcional", "App_Semaforo", "index.html")


def correr(b, fw):
    b.titulo("Los rangos de tiempos, releidos de los tres lenguajes")

    cpp = fw.codigo(*CPP)

    # SIN VALOR POR DEFECTO EN NINGUNO: si el lector no encuentra la constante, el pack
    # ABORTA. Suponer un rango seria aprobar sobre un numero inventado, que es justo lo
    # que este pack existe para impedir.
    def constante(nombre):
        m = re.search(r"%s\s*=\s*(\d+)" % nombre, cpp)
        if not m:
            raise fw.Abortado(
                "no se halla %s en Maestro/include/limites_ciclo.h. Sin el rango del C++ "
                "no hay contra que comparar, y comparar contra un valor supuesto seria "
                "inventar la referencia" % nombre)
        return int(m.group(1))

    v_min, v_max = constante("VERDE_MIN_MIN"), constante("VERDE_MIN_MAX")
    r_min, r_max = constante("ROJO_MIN_MIN"), constante("ROJO_MIN_MAX")
    d_min, d_max = constante("DESPEJE_SEG_MIN"), constante("DESPEJE_SEG_MAX")

    # reportar() ITERA el detalle linea a linea, asi que una cadena suelta se imprimia
    # CARACTER A CARACTER -medido el 05/09: 44 lineas de una letra donde iba un
    # renglon-. El hallazgo estaba, y era ilegible: un instrumento que publica su medida
    # de forma que nadie la lee no la ha publicado.
    b.reportar(
        "los rangos que manda el firmware",
        ["verde %d-%d min - rojo %d-%d min - despeje %d-%d s"
         % (v_min, v_max, r_min, r_max, d_min, d_max)])

    # ---- 1. La validacion de app.js -------------------------------------------
    js = fw.texto_repo(*APP_JS)
    m = re.search(r"enRango\(\s*verde\s*,\s*(\d+)\s*,\s*(\d+)\s*\)", js)
    m2 = re.search(r"enRango\(\s*rojo\s*,\s*(\d+)\s*,\s*(\d+)\s*\)", js)
    m3 = re.search(r"enRango\(\s*despeje\s*,\s*(\d+)\s*,\s*(\d+)\s*\)", js)
    if not (m and m2 and m3):
        raise fw.Abortado(
            "no se hallan las tres llamadas a enRango() en app.js. El lector se quedo "
            "ciego, y dar por buenos unos rangos que no se han leido es peor que no "
            "mirarlos")

    js_v = (int(m.group(1)), int(m.group(2)))
    js_r = (int(m2.group(1)), int(m2.group(2)))
    js_d = (int(m3.group(1)), int(m3.group(2)))

    b.verificar(
        js_v == (v_min, v_max) and js_r == (r_min, r_max) and js_d == (d_min, d_max),
        "app.js valida con los MISMOS rangos que el firmware: verde %s, rojo %s, "
        "despeje %s" % (js_v, js_r, js_d),
        "app.js y el firmware NO dicen lo mismo. C++: verde (%d,%d) rojo (%d,%d) despeje "
        "(%d,%d). app.js: verde %s rojo %s despeje %s. Si la app es mas permisiva, el "
        "operario teclea un valor que el equipo rechazara con $ERR y se queda sin saber "
        "por que; si es mas estricta, hay configuraciones legitimas que nadie puede poner"
        % (v_min, v_max, r_min, r_max, d_min, d_max, js_v, js_r, js_d))

    # ---- 2. Los min/max del HTML ----------------------------------------------
    html = fw.texto_repo(*APP_HTML)

    def limites(campo):
        m = re.search(r'id="num-tiempo-%s"[^>]*?min="(\d+)"[^>]*?max="(\d+)"' % campo, html)
        if not m:
            raise fw.Abortado(
                "no se hallan min/max del campo '%s' en index.html: el lector no puede "
                "decir que ofrece el teclado del movil" % campo)
        return (int(m.group(1)), int(m.group(2)))

    h_v, h_r, h_d = limites("verde"), limites("rojo"), limites("despeje")

    b.verificar(
        h_v == (v_min, v_max) and h_r == (r_min, r_max) and h_d == (d_min, d_max),
        "el HTML ofrece los MISMOS limites: verde %s, rojo %s, despeje %s"
        % (h_v, h_r, h_d),
        "los min/max del HTML no cuadran con el firmware. C++: verde (%d,%d) rojo (%d,%d) "
        "despeje (%d,%d). HTML: verde %s rojo %s despeje %s. El operario pelearia con su "
        "propio telefono: el teclado le ofrece un rango y la validacion le rechaza otro"
        % (v_min, v_max, r_min, r_max, d_min, d_max, h_v, h_r, h_d))

    # ---- 3. El valor POR DEFECTO tiene que ser valido --------------------------
    #
    # No es un detalle: al subir el minimo de 1 a 3, el value="2" que traia el formulario
    # se quedo fuera de rango. El operario abre la pantalla, no toca nada, pulsa guardar
    # y el equipo le rechaza SU PROPIO valor por defecto.
    for campo, (lo, hi) in (("verde", (v_min, v_max)), ("rojo", (r_min, r_max)),
                            ("despeje", (d_min, d_max))):
        m = re.search(r'id="num-tiempo-%s"[^>]*?value="(\d+)"' % campo, html)
        val = int(m.group(1)) if m else None
        b.verificar(
            val is not None and lo <= val <= hi,
            "el valor por defecto de '%s' (%s) esta dentro del rango %d-%d"
            % (campo, val, lo, hi),
            "el valor por defecto de '%s' es %s y el rango es %d-%d: el operario abre la "
            "pantalla, no toca nada, pulsa guardar y el equipo rechaza el valor que la "
            "propia app le puso delante" % (campo, val, lo, hi))

    # ---- 4. Y NO PUEDE HABER UNA CUARTA COPIA ESCONDIDA -------------------------
    #
    # ESTE PACK NACIO DICIENDO "TRES SITIOS" Y HABIA CUATRO. Lo encontro una revision
    # cruzada el mismo 04/09: js/config.js declaraba un LIMITES_TIEMPO con
    # VERDE_MIN_MIN: 1 bajo el rotulo "Rangos de Tiempos Permitidos por Firmware".
    #
    # Y era la peor de las cuatro, porque NO LA LEIA NADIE -cero consumidores de
    # IOT_CONFIG en toda la app- y aun asi index.html la carga. Una cifra caducada que
    # nadie usa no falla nunca: solo espera a que alguien la lea y se la crea. Es §3.bis
    # -la prueba muerta- aplicada a una constante, con una frase encima que la presenta
    # como medida del firmware.
    #
    # Se borro en vez de corregirse: actualizarla habria creado otra copia a mano que
    # sincronizar. Lo que se vigila desde aqui es que no VUELVA -y de paso, que no
    # aparezca en ningun otro fichero de la app-.
    sospechosos = []
    for sub, fichero in (("js", "config.js"),):
        try:
            txt = fw.texto_repo("05_Funcional", "App_Semaforo", sub, fichero)
        except Exception:
            continue
        # SE QUITAN LOS COMENTARIOS ANTES DE BUSCAR, Y NO ES UN DETALLE.
        #
        # La primera version de esta comprobacion FALLO sobre el fichero ya arreglado:
        # el comentario que documenta el defecto retirado CITA los nombres viejos para
        # explicar que se borro, y el regex casaba dentro de esa cita. Un buscador que
        # no distingue codigo de comentario acusa a la documentacion de ser el defecto
        # que documenta, y el arreglo obvio -no explicar nada- es peor que el problema.
        # Es §4 sobre mi propio instrumento.
        codigo = re.sub(r"//[^\n]*", "", txt)

        # Se busca la FORMA -un nombre de limite con un numero pegado-, no un valor
        # concreto: corregir el 1 a 3 y dejarla ahi seria exactamente el defecto.
        m = re.search(r"(VERDE_MIN_MIN|ROJO_MIN_MIN|DESPEJE_MIN_SEG|VERDE_MAX_MIN|"
                      r"ROJO_MAX_MIN|DESPEJE_MAX_SEG)\s*:\s*\d+", codigo)
        if m:
            sospechosos.append("%s/%s -> %s" % (sub, fichero, m.group(0)))

    b.verificar(
        not sospechosos,
        "no hay una cuarta copia de los rangos escondida en la app: los limites viven "
        "solo donde alguien los usa",
        "vuelve a haber limites de tiempo escritos a mano donde nadie los lee: %s. Una "
        "cifra que ningun codigo consume no falla nunca cuando se queda vieja; solo "
        "espera a que alguien la lea y se la crea. Si hacen falta ahi, tienen que "
        "consumirse desde ahi y entrar en esta comprobacion" % sospechosos)

    # ---- 5. Y EL FIRMWARE NO SE CONTRADICE A SI MISMO ---------------------------
    #
    # LA GUARDA DE 3 MINUTOS ERA MEDIA GUARDA, Y LO ENCONTRO UNA REVISION CRUZADA EL
    # MISMO DIA QUE SE ESCRIBIO.
    #
    # VERDE_MIN_MIN/ROJO_MIN_MIN solo los cruzaba SET_TIEMPOS. Habia CINCO sitios mas en
    # modo_automatico.cpp con el numero prohibido escrito a mano: el inicializador
    # estatico, el reset de modoAutomatico_setup() y los topes de los tres campos del
    # menu -piso 1 min, 1 min y 5 s-. O sea: un equipo que arranca y al que nadie le
    # manda SET_TIEMPOS corria con UN MINUTO por sentido, y el despeje se podia dejar en
    # 5 s por pantalla, la MITAD del minimo vial. El 04/09 aparecio un tercer agujero
    # igual: modo_inteligente.cpp configuraba el coordinador con `maxVerde = 2` minutos.
    #
    # =====================================================================
    # EL BORDE DE ESTA COMPROBACION, Y POR QUE ES EL QUE ES (§4.quinquies)
    # =====================================================================
    #
    # Esta comprobacion llevaba dentro una LISTA DE CUATRO NOMBRES DE VARIABLE
    # -minRojo, minVerde, segEstatico, maxVerde- y juzgaba las lineas que casaran
    # `nombre = <digitos>`. MEDIDO EL 05/09 SOBRE EL FIRMWARE DE HOY: esa lista juzgaba
    # CERO lineas. Ni una. Los cuatro nombres siguen ahi, pero desde N-137 todos se
    # asignan de una CONSTANTE -`maxVerde = VERDE_MIN_MIN`- y el regex pedia digitos. La
    # comprobacion salia verde porque no miraba nada: es la prueba muerta de §3.bis,
    # producida esta vez por el propio arreglo que la dejo sin sujetos.
    #
    # Y a la vez habia un defecto REAL en el alcance del fichero y fuera del de la lista:
    # `Maestro/src/modo_inteligente.cpp` corta el verde con
    #
    #     if (tiempoActual >= 15000UL) {          // "Regla 1: Minimo 15 segundos"
    #
    # QUINCE SEGUNDOS, donde el responsable fijo TRES MINUTOS (D-5). El fichero estaba
    # dentro del alcance; la linea no, porque `tiempoActual` no era ninguno de los cuatro
    # nombres. Anadir "tiempoActual" a la lista seria EL MISMO DEFECTO CON UN NOMBRE MAS.
    # Lo que se hace es cambiar la pregunta:
    #
    #   NO  "¿se llama la variable como una de las que conozco?"
    #   SI  "¿este numero DECIDE cuando empieza o termina una fase del cruce?"
    #
    # Y esa pregunta tiene dos formas mecanicas, que son las dos mitades de abajo:
    #
    #   5.a  el numero entra al coordinador COMO CONFIGURACION. Los nombres NO se
    #        listan: se CENSAN de los argumentos de coordinador_configurar(), asi que un
    #        quinto modo con un quinto nombre entra solo.
    #   5.b  el numero es un LITERAL DE TIEMPO comparado contra el cronometro de la fase
    #        en curso, DENTRO de una funcion que puede terminarla -o sea, que llama a
    #        coordinador_pedirCambio(), la unica puerta del firmware para cortar un verde
    #        o un rojo en marcha, censada en coordinador.h-.
    #
    # DONDE ACABA EL BORDE, ESCRITO PARA QUE NO HAYA QUE ADIVINARLO:
    #
    #   - Solo el MAESTRO. El Esclavo no decide fases: cero apariciones de pedirCambio()
    #     y de coordinador_configurar() en Esclavo/src. Sigue al coordinador, y un numero
    #     escrito alli no puede acortar un verde.
    #   - Solo LITERALES. `tiempoActual >= duracionMaxima` queda fuera A PROPOSITO:
    #     duracionMaxima sale de la constante, que es justo lo que se quiere que pase.
    #   - Solo dentro de la puerta. En todo Maestro/src hay CUATRO literales de tiempo
    #     comparados contra un cronometro; tres NO son fases del ciclo -un refresco de
    #     LCD, el rojo previo al ambar del Degradado y el troceo de tramas del bus-. Esos
    #     tres se REPORTAN, no se acusan: quedan a la vista para que nadie tenga que
    #     fiarse de que el pack los descarto por buenos motivos.
    import os

    # Se conservan los SALTOS DE LINEA al quitar comentarios. fw.codigo() sustituye el
    # bloque /* */ por un espacio, y con eso el pack ya no sabe en que linea esta lo que
    # acusa: publicaria un numero de linea inventado, que es §4 sobre mi propio
    # instrumento. Un acusado sin direccion no se puede ir a mirar.
    def _sin_comentarios(t):
        t = re.sub(r"/\*.*?\*/", lambda m: "\n" * m.group(0).count("\n"), t, flags=re.S)
        return re.sub(r"//[^\n]*", "", t)

    def _linea(t, pos):
        return t.count("\n", 0, pos) + 1

    _dir = os.path.dirname(fw.ruta("Maestro", "src", "modo_automatico.cpp"))
    CPPS = [(n, _sin_comentarios(fw.texto("Maestro", "src", n)))
            for n in sorted(x for x in os.listdir(_dir) if x.endswith(".cpp"))]

    def _argumentos(txt, fn):
        """Los argumentos de cada llamada a fn(), con los parentesis CASADOS.

        Con un `[^)]*` no valdria: los tres argumentos que importan llevan un cast
        `(unsigned long)` dentro, asi que el primer parentesis de cierre esta a mitad del
        primer argumento y el lector se quedaria con basura. Un lector que corta donde no
        debe es un buscador ciego con otra forma (§4)."""
        for m in re.finditer(r"\b%s\s*\(" % re.escape(fn), txt):
            i = m.end() - 1
            prof, j = 0, i
            while j < len(txt):
                if txt[j] == "(":
                    prof += 1
                elif txt[j] == ")":
                    prof -= 1
                    if prof == 0:
                        break
                j += 1
            piezas, prof, act = [], 0, ""
            for ch in txt[i + 1:j]:
                if ch == "(":
                    prof += 1
                elif ch == ")":
                    prof -= 1
                if ch == "," and prof == 0:
                    piezas.append(act)
                    act = ""
                else:
                    act += ch
            piezas.append(act)
            yield i, [p.strip() for p in piezas]

    # -- 5.a  Los tiempos que entran al coordinador COMO CONFIGURACION -----------
    #
    # Que rango le toca a cada posicion NO se escribe aqui: se lee de la firma del
    # header, por el NOMBRE del parametro. Si manana la firma cambia de orden, este
    # lector se entera; una lista de posiciones escrita a mano, no.
    _h = fw.codigo("Maestro", "include", "coordinador.h")
    _firma = re.search(r"void\s+coordinador_configurar\s*\(([^)]*)\)", _h)
    if not _firma:
        raise fw.Abortado(
            "no se halla la firma de coordinador_configurar() en Maestro/include/"
            "coordinador.h. Sin ella no se sabe que posicion es el despeje y cual el "
            "verde, y suponerlo seria juzgar cada numero contra el rango de otro")

    POSICIONES = []
    for _p in _firma.group(1).split(","):
        _pl = _p.lower()
        if "despeje" in _pl or "estatico" in _pl:
            POSICIONES.append(("despeje", (d_min, d_max), 1000, "s"))
        elif "rojo" in _pl:
            POSICIONES.append(("rojo", (r_min, r_max), 60000, "min"))
        elif "verde" in _pl:
            POSICIONES.append(("verde", (v_min, v_max), 60000, "min"))
        else:
            raise fw.Abortado(
                "un parametro de coordinador_configurar() -%r- no dice a que tiempo de "
                "ciclo corresponde. El pack no adivina cual es su rango: aborta, que es "
                "lo unico que no miente" % _p.strip())

    TIPO_C = r"^(const\s+)?(unsigned\s+|signed\s+)?(long|int|short|char|bool|uint\d+_t|float)\b"
    conf_vars = {}          # nombre de variable -> (etiqueta, rango, unidad)
    lineas_malas = []

    for _n, _t in CPPS:
        for _pos, _args in _argumentos(_t, "coordinador_configurar"):
            if len(_args) == len(POSICIONES) and all(re.match(TIPO_C, a) for a in _args):
                continue          # es la DEFINICION, no una llamada
            if len(_args) != len(POSICIONES):
                raise fw.Abortado(
                    "%s:%d llama a coordinador_configurar() con %d argumentos y el "
                    "header declara %d. El lector no sabe que rango aplicar a cual"
                    % (_n, _linea(_t, _pos), len(_args), len(POSICIONES)))
            for _a, (_et, _rango, _factor, _u) in zip(_args, POSICIONES):
                _lim = re.sub(r"\(\s*(unsigned\s+|signed\s+)?\w+\s*\)", "", _a)
                _mv = re.search(r"(\w+)\s*\*\s*(\d+)", _lim)
                if _mv:
                    # El factor DICE LA UNIDAD, y por eso se exige: si alguien pasa
                    # minutos por donde van segundos, cada numero es correcto por
                    # separado y el cruce corre sesenta veces mas largo. Es N-71: la
                    # relacion entre dos numeros se recalcula, no se narra.
                    if int(_mv.group(2)) != _factor:
                        lineas_malas.append(
                            "%s:%d el argumento '%s' de coordinador_configurar() "
                            "convierte con *%s y el header lo declara en %s (factor %d)"
                            % (_n, _linea(_t, _pos), _et, _mv.group(2), _u, _factor))
                    conf_vars[_mv.group(1)] = (_et, _rango, _u)
                    continue
                _ml = re.fullmatch(r"[\dUuLl\s*+()]+", _lim)
                if _ml and re.search(r"\d", _lim):
                    _ms = int(re.search(r"\d+", _lim).group(0))
                    _val = _ms / float(_factor)
                    if not (_rango[0] <= _val <= _rango[1]):
                        lineas_malas.append(
                            "%s:%d coordinador_configurar() recibe el %s escrito a mano: "
                            "%d ms = %.2f %s, y el rango es %d-%d %s"
                            % (_n, _linea(_t, _pos), _et, _ms, _val, _u,
                               _rango[0], _rango[1], _u))
                    continue
                raise fw.Abortado(
                    "%s:%d pasa a coordinador_configurar() un %s que este lector no sabe "
                    "leer: %r. Dar por bueno un tiempo de ciclo que no se ha sabido leer "
                    "es exactamente lo que este pack existe para impedir"
                    % (_n, _linea(_t, _pos), _et, _a))

    if not conf_vars:
        raise fw.Abortado(
            "el censo de coordinador_configurar() no ha encontrado NI UNA variable de "
            "tiempo de ciclo en Maestro/src. O el firmware cambio de forma, o el lector "
            "se quedo ciego; en los dos casos, aprobar seria inventar")

    # Y ahora si: esas variables -CENSADAS, no listadas- no pueden llevar un literal
    # fuera de rango en ningun .cpp del Maestro. Se admiten `=`, `==` y las cuatro
    # desigualdades: el defecto de N-131 estaba en los TOPES del menu, que son `<` y `>`,
    # y el regex viejo tampoco sabia leer `<=` ni `>=`.
    for _n, _t in CPPS:
        for _v, (_et, _rango, _u) in conf_vars.items():
            for m in re.finditer(r"\b%s\b\s*(=|==|<=|>=|<|>)\s*(\d+)" % re.escape(_v), _t):
                _val = int(m.group(2))
                if not (_rango[0] <= _val <= _rango[1]):
                    lineas_malas.append(
                        "%s:%d '%s' (%s del ciclo) con %d %s escrito a mano, y el rango "
                        "es %d-%d %s" % (_n, _linea(_t, m.start()), _v, _et, _val, _u,
                                         _rango[0], _rango[1], _u))

    b.verificar(
        not lineas_malas,
        "ningun tiempo de ciclo escrito a mano se sale de los limites: las %d variables "
        "que entran a coordinador_configurar() (%s) salen de las mismas constantes que "
        "la guarda de SET_TIEMPOS"
        % (len(conf_vars), ", ".join(sorted(conf_vars))),
        "hay tiempos de ciclo escritos a mano FUERA del rango que declara "
        "limites_ciclo.h: %s. Un minimo vial que solo cruza SET_TIEMPOS no protege el "
        "arranque ni el menu, y un equipo al que nadie manda tiempos corre con el valor "
        "de mesa de pruebas" % lineas_malas)

    # -- 5.b  Los limites de fase escritos como LITERAL DE TIEMPO ----------------
    #
    # LA PUERTA SE CENSA, NO SE LISTA: coordinador_pedirCambio() es la unica funcion
    # publica que corta una fase en marcha, y se comprueba que siga existiendo en
    # coordinador.h. Si desapareciera o cambiara de nombre, este censo no encontraria
    # ninguna funcion que mirar y saldria VERDE POR VACIO -que es la forma exacta en que
    # la lista de cuatro nombres llevaba meses aprobando-. Por eso aborta en vez de pasar.
    PUERTA = "coordinador_pedirCambio"
    if re.search(r"\b%s\s*\(" % PUERTA, _h) is None:
        raise fw.Abortado(
            "no se halla %s() en Maestro/include/coordinador.h. Ese es el nombre por el "
            "que este pack reconoce 'aqui se puede terminar una fase'; sin el, el censo "
            "de limites de fase no mira nada y saldria verde por vacio" % PUERTA)

    CLAVES = ("if", "for", "while", "switch", "else", "do", "return", "case")

    def _funciones(txt):
        """(nombre, inicio, fin) de cada definicion a nivel de fichero.

        Hace falta el CUERPO y no el fichero entero: el borde de 5.b es "dentro de una
        funcion que puede terminar la fase", y un fichero con una funcion que la termina
        y otra que solo refresca la pantalla no es lo mismo que un fichero donde todo la
        termina."""
        for m in re.finditer(r"(?m)^[A-Za-z_][\w \t\*&:<>,]*?\b(\w+)\s*\([^;]*?\)\s*\{",
                             txt):
            if m.group(1) in CLAVES:
                continue
            prof, j = 0, m.end() - 1
            while j < len(txt):
                if txt[j] == "{":
                    prof += 1
                elif txt[j] == "}":
                    prof -= 1
                    if prof == 0:
                        break
                j += 1
            yield m.group(1), m.start(), j

    # El cronometro de la fase NO se reconoce por su nombre: se reconoce por DE DONDE
    # SALE -millis() menos una marca-. Los nombres se descubren en cada funcion, asi que
    # da igual como los llame quien escriba el modo siguiente.
    CRONO = r"millis\s*\(\s*\)\s*-\s*\w+"
    F_MIN, F_MAX = min(v_min, r_min), max(v_max, r_max)

    def _censar_fases(txt):
        """(acusadas, vistas_fuera_de_la_puerta) sobre un fuente cualquiera."""
        malas, fuera = [], []
        for _fn, _ini, _fin in _funciones(txt):
            _cuerpo = txt[_ini:_fin]
            _en_puerta = re.search(r"\b%s\s*\(" % PUERTA, _cuerpo) is not None
            _pats = [CRONO + r"\s*(>=|<=|>|<)\s*(\d+)"]
            for _c in set(m.group(1) for m in
                          re.finditer(r"(\w+)\s*=\s*[^;]*?" + CRONO, _cuerpo)):
                _pats.append(r"\b%s\b\s*(>=|<=|>|<)\s*(\d+)" % re.escape(_c))
            for _p in _pats:
                for m in re.finditer(_p, _cuerpo):
                    _ms = int(m.group(2))
                    _dentro = F_MIN <= _ms / 60000.0 <= F_MAX
                    (malas if (_en_puerta and not _dentro) else fuera).append(
                        (_fn, _ini + m.start(), m.group(0).strip(), _ms, _en_puerta))
        return malas, fuera

    fases_malas, fuera_de_puerta, fases_ficheros = [], [], set()
    for _n, _t in CPPS:
        _malas, _fuera = _censar_fases(_t)
        for _fn, _p, _txt, _ms, _ in _malas:
            fases_ficheros.add(_n)
            fases_malas.append(
                "%s:%d dentro de %s(): '%s' -> %d ms = %.2f min, y el minimo vial es "
                "%d min (rango %d-%d min)"
                % (_n, _linea(_t, _p), _fn, _txt, _ms, _ms / 60000.0, F_MIN, F_MIN, F_MAX))
        for _fn, _p, _txt, _ms, _ep in _fuera:
            fuera_de_puerta.append(
                "%s:%d %s() -> '%s' = %d ms  [%s]"
                % (_n, _linea(_t, _p), _fn, _txt, _ms,
                   "dentro del rango vial" if _ep else
                   "no llama a %s(): no puede cortar una fase" % PUERTA))

    if fuera_de_puerta:
        b.reportar(
            "literales de tiempo que el censo VIO y no acusa, con el motivo de cada uno",
            fuera_de_puerta + [
                "Se publican para que el borde de 5.b se pueda discutir mirandolo y no",
                "creyendoselo: lo que un instrumento decide NO mirar es donde fallaron",
                "los tres censos del 04/09, y ninguno lo llevaba escrito."])

    # El "por que duele" se ARMA con lo que se acuso, no se escribe fijo. Una coletilla
    # fija sobre las camaras pegada a un hallazgo de otro fichero seria una frase que
    # sostiene un veredicto y que no comprueba nadie -§2.ter-, y el pack tiene que poder
    # acusar a cualquier modo sin mentir sobre cual.
    _duele = ("Esa comparacion decide cuando se puede CORTAR un verde en marcha y no "
              "pasa ni por SET_TIEMPOS ni por el menu, asi que ninguna de las dos "
              "guardas la toca: el cruce puede alternar por debajo del minimo sin que "
              "nadie haya configurado nada. ")
    if "modo_inteligente.cpp" in fases_ficheros:
        _duele += (
            "Y esta en modo_inteligente.cpp, que es el UNICO modo que usa las camaras: "
            "con una camara pegada en 'hay presencia' esa punta recibe verdes de ese "
            "tamano ciclo tras ciclo mientras la otra corre a %d minutos, y con las dos "
            "ruidosas el cruce alterna al minimo indefinidamente. Estaba escrito ANTES "
            "de comprar las camaras: el hardware nuevo no lo trae, lo encuentra. " % F_MIN)

    b.verificar(
        not fases_malas,
        "ningun limite de fase escrito como literal de tiempo baja del minimo vial: las "
        "funciones que pueden cortar un verde o un rojo miden contra las constantes de "
        "limites_ciclo.h, no contra un numero suelto",
        "HAY UN LIMITE DE FASE POR DEBAJO DEL MINIMO VIAL, ESCRITO A MANO Y FUERA DE "
        "TODA GUARDA: %s. %sEl responsable fijo %d minutos por sentido el 04/09 (D-5) "
        "porque por debajo el conductor se convence de que el semaforo esta averiado y "
        "adelanta en rojo. EL ARREGLO ES DEL FIRMWARE -sacar el numero de "
        "limites_ciclo.h-, no de este pack: bajarle el listo al instrumento para que se "
        "ponga verde es ajustar la medida hasta que de el resultado que gusta"
        % (fases_malas, _duele, F_MIN))

    # ---- CONTROLES NEGATIVOS ---------------------------------------------------
    b.control_negativo(
        re.search(r"enRango\(\s*verde\s*,\s*(\d+)", "if (!enRango(verde, 1, 15))")
        .group(1) == "1",
        "el lector de app.js extrae el numero real de la llamada, no uno supuesto")

    b.control_negativo(
        re.search(r'id="num-tiempo-verde"[^>]*?min="(\d+)"',
                  '<input id="num-tiempo-verde" min="9" max="15">') is not None,
        "el lector del HTML encuentra el min de un campo aunque valga otra cosa: "
        "compara, no da por bueno")

    b.control_negativo(
        re.search(r"VERDE_MIN_MIN\s*=\s*(\d+)", "static const uint8_t OTRA = 7;") is None,
        "si la constante del C++ cambiara de nombre, el lector NO la encuentra y el pack "
        "ABORTA en vez de aprobar sobre un rango que ya no existe")

    # Los tres de 5.b van sobre cuerpos SINTETICOS a proposito, y ejercen las TRES
    # fronteras del borde por separado -el numero, el nombre de la variable y la puerta-.
    # Ninguno de los tres censos que fallaron el 04/09 llevaba esto escrito.
    _MALO = ("void modoX_loop() {\n"
             "  unsigned long llevaAsi = millis() - tDesde;\n"
             "  if (llevaAsi >= 15000UL) { coordinador_pedirCambio(); }\n}\n")
    _BUENO = ("void modoX_loop() {\n"
              "  unsigned long llevaAsi = millis() - tDesde;\n"
              "  if (llevaAsi >= 180000UL) { coordinador_pedirCambio(); }\n}\n")
    _AJENO = ("void pantallaX_loop() {\n"
              "  if (millis() - tUltimoRefresco >= 5000) { lcd_repintar(); }\n}\n")

    b.control_negativo(
        _censar_fases(_MALO)[0] != [],
        "el censo de 5.b acusa un limite de fase de 15 s escrito con un nombre de "
        "variable ('llevaAsi') que este pack NO conoce: el borde es la FORMA -un literal "
        "contra el cronometro de la fase, dentro de la puerta-, no una lista de nombres. "
        "Con la lista de cuatro nombres que habia antes, esto pasaba de largo")

    b.control_negativo(
        _censar_fases(_BUENO)[0] == [],
        "el mismo cuerpo con 180000 ms -los tres minutos- NO se acusa: el pack sabe "
        "estar verde, asi que su rojo dice algo. Un censo que acusa a todo no mide nada")

    b.control_negativo(
        _censar_fases(_AJENO)[0] == [] and _censar_fases(_AJENO)[1] != [],
        "un refresco de pantalla de 5 s se VE pero NO se acusa: la puerta -llamar a "
        "coordinador_pedirCambio()- es lo que separa un limite de fase de cualquier otro "
        "temporizador, y lo descartado queda publicado en vez de desaparecer en silencio")
