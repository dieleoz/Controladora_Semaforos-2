# ===== banco/packs/maestro_01_mando.py =====
#
# SECUENCIAS DEL MANDO DE RELES (SFTY-21, mando.cpp)
#
# El operario acciona desde el suelo SIN VER LA PANTALLA: esta a 5 m dentro del
# gabinete. Solo se usan botones cuya repeticion accidental es inofensiva, y la
# confirmacion se da en destellos ROJOS contables -el rojo nunca significa pase, asi
# que contar mal deja al operario en el caso seguro-.

from banco.modelos.maestro import *          # noqa: F401,F403
from banco.modelos.maestro import (          # los guiones bajos no
    _codigo, _fuente, _main, _ruta,          # los exporta import *
)

NOMBRE = "maestro_01_mando"
DESCRIPCION = "las secuencias del mando de reles (SFTY-21)"


def correr(b, fw):
    # Bloque traido LITERAL del validador monolitico, solo reindentado. Reescribir
    # logica ya probada para renombrar las llamadas es como se cuelan los errores en
    # una migracion que se supone que no cambia comportamiento.
    verificar = b.verificar
    titulo = b.titulo

    # --- 1.1 -------------------------------------------------------------------
    # El comentario del firmware afirma: "No hay ambiguedad con las otras dos: sus
    # tres ultimos pulsos son B.A.B, que no es ni A.A.A ni B.B.B". Se comprueba, no
    # se cree. Con ~2 s por pulsacion las tres secuencias caben holgadas en su
    # ventana, asi que si hubiera confusion apareceria aqui.
    acc_ababab = accion_de([MANDO_A, MANDO_B, MANDO_A, MANDO_B])
    acc_bbb = accion_de([MANDO_B, MANDO_B, MANDO_B])
    acc_aaa = accion_de([MANDO_A, MANDO_A, MANDO_A])
    verificar(acc_ababab == ACC_DEGRADADO and acc_bbb == ACC_AMBAR and acc_aaa == ACC_AUTOMATICO,
              "Las tres secuencias nominales dan cada una SU accion y solo esa: "
              "A.B.A.B->DEGRADADO, B.B.B->AMBAR, A.A.A->AUTOMATICO.",
              f"Confusion entre secuencias: ABAB={NOMBRE_ACC[acc_ababab]} "
              f"BBB={NOMBRE_ACC[acc_bbb]} AAA={NOMBRE_ACC[acc_aaa]}")

    # --- 1.2 -------------------------------------------------------------------
    # BARRIDO COMPLETO de todos los trenes de 1 a 7 pulsos (254 trenes) a cadencia
    # nominal, contra un modelo INDEPENDIENTE de reconocimiento.
    #
    # El modelo de referencia no reproduce el buffer deslizante ni los tiempos: aplica
    # la regla tal como esta ESCRITA en el fichero -cuadruple primero, triples
    # despues- sobre el tren entero, y se detiene en el primer reconocimiento. Si el
    # buffer de 4 posiciones o el desplazamiento tuvieran un error, las dos versiones
    # discreparian en algun tren.
    #
    # POR QUE IMPORTA: si algun tren de 5, 6 o 7 pulsos entrase en Degradado sin
    # terminar en A.B.A.B, un operario zigzagueando a ciegas podria abrir sin saberlo
    # la unica pantalla que da verde sin confirmar la otra punta.
    def accion_esperada(tr):
        for k in range(1, len(tr) + 1):
            if k >= 4 and tr[k - 4:k] == [MANDO_A, MANDO_B, MANDO_A, MANDO_B]:
                return ACC_DEGRADADO
            if k >= 3:
                if tr[k - 3:k] == [MANDO_A] * 3:
                    return ACC_AUTOMATICO
                if tr[k - 3:k] == [MANDO_B] * 3:
                    return ACC_AMBAR
        return ACC_NINGUNA


    discrepancias = []
    for L in range(1, 8):
        for tr in trenes(L):
            acc = accion_de(tr)
            esp = accion_esperada(tr)
            if acc != esp:
                discrepancias.append((txt(tr), NOMBRE_ACC[acc], NOMBRE_ACC[esp]))
    verificar(not discrepancias,
              "Barrido de los 254 trenes de 1 a 7 pulsos: el buffer deslizante de "
              f"{MAX_PULSOS} posiciones reconoce exactamente lo mismo que la regla escrita, "
              "incluidos los trenes con varias secuencias solapadas.",
              f"El buffer deslizante reconoce algo distinto de la regla escrita en "
              f"{discrepancias[:5]} (tren, obtenido, esperado)")

    # --- 1.3 -------------------------------------------------------------------
    # NINGUNA ACCION SE ENCADENA SOBRE UNA CONFIRMACION EN CURSO. El operario esta
    # contando destellos, no pulsando; si pulsa de nervios, esos pulsos no pueden
    # sumarse a una segunda accion que se aplique antes de que termine la primera.
    #
    # Ojo con lo que NO se exige: un tren largo SI puede producir dos acciones
    # separadas (seis A seguidas dan dos AUTOMATICO). Eso es correcto y es inofensivo,
    # porque cada una vuelve a pasar por todo-rojo. Lo que se prohibe es que la
    # segunda llegue ANTES de que el operario haya visto entera la cuenta de la
    # primera, que es su unica realimentacion.
    DURACION_CONFIRMACION = {
        ACC_AUTOMATICO: DESTELLOS_AUTOMATICO * (DESTELLO_ON_MS + DESTELLO_OFF_MS),
        ACC_AMBAR: DESTELLOS_AMBAR * (DESTELLO_ON_MS + DESTELLO_OFF_MS),
        ACC_DEGRADADO: DESTELLOS_DEGRADADO * (DESTELLO_ON_MS + DESTELLO_OFF_MS),
    }
    encadenadas, pulsos_colados = [], []
    for L in range(1, 8):
        for tr in trenes(L):
            m, _ = correr_tren(tr, 2000)
            for (t1, a1), (t2, _a2) in zip(m.ejecutadas, m.ejecutadas[1:]):
                if t2 - t1 < DURACION_CONFIRMACION[a1]:
                    encadenadas.append((txt(tr), t1, t2))
            # Y todo pulso llegado con el mando ocupado tiene que haberse descartado.
            if m.ignorados and m.n > MAX_PULSOS:
                pulsos_colados.append(txt(tr))
    verificar(not encadenadas and not pulsos_colados,
              "Ningun tren de hasta 7 pulsos encadena una segunda accion antes de que "
              "termine la cuenta de destellos de la primera: los pulsos de nervios se "
              "descartan enteros mientras hay confirmacion en curso.",
              f"Se encadena una accion sobre una confirmacion sin terminar en "
              f"{encadenadas[:5]}")

    # --- 1.4 -------------------------------------------------------------------
    # SECUENCIAS SOLAPADAS. B.A.B.A.B contiene A.B.A.B como sufijo, y A.A.B.A.B
    # tambien. El firmware usa ventana deslizante, asi que ENTRA. No es un fallo
    # -es la definicion de ventana deslizante- pero contradice la lectura literal
    # de "cuatro pulsos alternados" y conviene que quede escrito: cinco pulsos
    # alternados empezando por B tambien abren el Modo Degradado.
    solapadas = {
        "BABAB": [MANDO_B, MANDO_A, MANDO_B, MANDO_A, MANDO_B],
        "AABAB": [MANDO_A, MANDO_A, MANDO_B, MANDO_A, MANDO_B],
        "BBABAB": [MANDO_B, MANDO_B, MANDO_A, MANDO_B, MANDO_A, MANDO_B],
    }
    entran = {k: accion_de(v) == ACC_DEGRADADO for k, v in solapadas.items()}
    verificar(all(entran.values()),
              "Ventana deslizante confirmada: B.A.B.A.B, A.A.B.A.B y B.B.A.B.A.B entran "
              "en DEGRADADO por su sufijo A.B.A.B (comportamiento consistente, no accidental).",
              f"La ventana deslizante no reconoce sufijos de forma uniforme: {entran}")

    # --- 1.5 -------------------------------------------------------------------
    # LA SECUENCIA A CABALLO DEL LIMITE DE LA VENTANA. Se barre la cadencia entera en
    # pasos de 50 ms y se exige que el reconocimiento sea LIMPIO: o entero o nada.
    # Un reconocimiento "a medias" -que la secuencia caiga fuera de ventana y aun asi
    # dispare OTRA accion- seria el peor caso posible: el operario cuenta destellos
    # que no corresponden a lo que pidio.
    malos_triple, malos_quad = [], []
    for cad in range(100, 10001, 50):
        # A.A.A y B.B.B: el tramo medido es el que va del 1er al 3er pulso = 2*cad.
        dentro = (2 * cad) <= VENTANA_TRIPLE_MS
        if (accion_de([MANDO_A] * 3, cad) == ACC_AUTOMATICO) != dentro:
            malos_triple.append(("AAA", cad))
        if (accion_de([MANDO_B] * 3, cad) == ACC_AMBAR) != dentro:
            malos_triple.append(("BBB", cad))
        # A.B.A.B: el tramo es 3*cad y la ventana la cuadruple.
        dentro4 = (3 * cad) <= VENTANA_CUADRUPLE_MS
        acc4 = accion_de([MANDO_A, MANDO_B, MANDO_A, MANDO_B], cad)
        if (acc4 == ACC_DEGRADADO) != dentro4:
            malos_quad.append(("ABAB", cad, NOMBRE_ACC[acc4]))
        # Fuera de ventana no puede caer en OTRA accion distinta de "nada".
        if not dentro4 and acc4 != ACC_NINGUNA:
            malos_quad.append(("ABAB-parcial", cad, NOMBRE_ACC[acc4]))
    verificar(not malos_triple,
              f"Barrido de cadencia 100..10000 ms: A.A.A y B.B.B se reconocen si y solo si "
              f"el tramo cabe en los {VENTANA_TRIPLE_MS} ms, con el corte exacto en el limite.",
              f"Reconocimiento de triples incorrecto en {malos_triple[:5]}")
    verificar(not malos_quad,
              f"Barrido de cadencia: A.B.A.B se reconoce si y solo si cabe en los "
              f"{VENTANA_CUADRUPLE_MS} ms, y fuera de ventana NO cae en ninguna otra accion.",
              f"A.B.A.B se reconoce a medias o fuera de ventana en {malos_quad[:5]}")

    # --- 1.6 -------------------------------------------------------------------
    # GESTOS SEPARADOS QUE NO DEBEN SUMARSE. purgarViejos() existe para que dos
    # gestos distanciados en el tiempo no se peguen en una secuencia que nadie hizo.
    # Se prueba el caso mas peligroso: dos A sueltas, un rato largo, y luego una B,
    # una A y una B. Si el purgado fallara, aparecerian A.B.A.B falsos.
    falsos_positivos = []
    for hueco in range(500, 60001, 500):
        tren = [MANDO_A, MANDO_B]           # gesto 1: subir y bajar
        inst = [0, 2000]
        tren += [MANDO_A, MANDO_B]          # gesto 2, mas tarde: subir y bajar
        inst += [2000 + hueco, 4000 + hueco]
        sem = Semaforo(); sem.ahora = 0
        m = Mando(sem, True)
        eventos = dict(zip(inst, tren))
        t = 0
        while t <= inst[-1] + 5000:
            sem.ahora = t
            if t in eventos:
                m.registrar_pulso(eventos[t], t)
            sem.actualizar(t)
            m.actualizar()
            t += 10
        entro = bool(m.ejecutadas) and m.ejecutadas[0][1] == ACC_DEGRADADO
        # Solo debe entrar si los cuatro pulsos caben en la ventana cuadruple.
        debe = (inst[-1] - inst[0]) <= VENTANA_CUADRUPLE_MS
        if entro != debe:
            falsos_positivos.append((hueco, entro, debe))
    verificar(not falsos_positivos,
              "Dos gestos A.B separados por un hueco de 0 a 60 s solo forman A.B.A.B "
              "cuando de verdad caben en la ventana: purgarViejos() no deja que se sumen.",
              f"Dos gestos separados se suman (o se pierden) en {falsos_positivos[:5]}")

    # --- 1.7 -------------------------------------------------------------------
    # B.B.B DESDE CUALQUIER ESTADO.
    #
    # OPTIMIZACIONES.md lo escribe como invariante: "B.B.B devuelve a ambar desde
    # cualquier estado, sin condiciones. Es la regla que impide que nadie quede
    # atrapado con un semaforo en un estado raro a 5 m de altura".
    #
    # La accion del mando no se ejecuta hasta que TERMINAN los destellos, y los
    # destellos solo avanzan si alguien llama a semaforo_actualizar(). Asi que la
    # pregunta real es: en cada pantalla del Maestro, ?hay alguien llamandola?
    #
    # Quien la llama, segun el codigo:
    #   - main.cpp    -> coordinador_actualizar_background(), EXCEPTO en los modos
    #                    excluidos de la lista que se lee unas lineas mas abajo
    #   - modo_degradado_loop() y modo_ambar_loop() -> la llaman ellos, primera linea
    #   - modoAutomatico_loop() -> solo dentro de "case CORRIENDO"
    #
    # Primero se verifica contra el C++ que la tabla de abajo describe el firmware de
    # hoy y no el de ayer. Si alguien cambia main.cpp, esta prueba cae antes que las
    # que dependen de ella.
    _main = _codigo("Maestro", "src", "main.cpp")
    _m = re.search(r"if\s*\(([^)]*modo\s*!=\s*[^)]*)\)\s*\{?\s*\n?\s*coordinador_actualizar_background",
                   _main)
    excluidos_bg = set(re.findall(r"modo\s*!=\s*(MODO_\w+)", _m.group(1))) if _m else set()
    verificar(excluidos_bg == {"MODO_AUTOMATICO", "MODO_DEGRADADO", "MODO_AMBAR"},
              f"main.cpp excluye del latido de fondo exactamente {sorted(excluidos_bg)}, "
              "que es la lista que este banco modela.",
              f"La lista de modos excluidos de coordinador_actualizar_background() cambio: "
              f"{sorted(excluidos_bg)}. El modelo de abajo ya no describe el firmware.")

    _auto = _codigo("Maestro", "src", "modo_automatico.cpp")
    _pos_corriendo = _auto.find("case CORRIENDO")
    _pos_llamada = _auto.find("coordinador_actualizar()")
    auto_solo_corriendo = (_pos_corriendo != -1 and _pos_llamada != -1 and
                           _pos_llamada > _pos_corriendo)
    verificar(auto_solo_corriendo,
              "modo_automatico.cpp llama a coordinador_actualizar() UNICAMENTE dentro de "
              "'case CORRIENDO': el asistente de configuracion no bombea el semaforo. "
              "(Se comprueba para que el modelo no invente el fallo que va a reportar.)",
              "modo_automatico.cpp ya llama a coordinador_actualizar() fuera de CORRIENDO: "
              "el fallo 1.7 podria estar corregido y esta prueba hay que rehacerla.")

    # Tabla de estados del Maestro. Para cada uno: (bombea semaforo_actualizar,
    # inhibe secuencias, esta ya en estado seguro por si mismo).
    # CORRECCION DEL 01/08/2026, tras arreglarse el fallo 1.7.
    #
    # Las dos premisas de arriba siguen siendo ciertas -main.cpp excluye esos tres modos
    # del latido de fondo, y modo_automatico.cpp solo llama al coordinador en CORRIENDO-,
    # pero YA NO BASTAN para concluir que el asistente no bombea: main.cpp llama ahora a
    # semaforo_actualizar() de forma INCONDICIONAL al principio del loop(), fuera de
    # cualquier if, de modo que la maquina de luces avanza en todos los modos.
    #
    # Se comprueba contra el fuente en vez de darlo por hecho. Si alguien quita esa
    # llamada, esta prueba cae y la tabla vuelve a describir el firmware: el fallo del
    # cabezal a oscuras reaparece y el banco lo detecta.
    _llamada_incondicional = bool(re.search(
        r"void loop\(\)[\s\S]{0,2000}?\n  semaforo_actualizar\(\);", _main))
    verificar(_llamada_incondicional,
              "main.cpp llama a semaforo_actualizar() SIN CONDICION en el loop(), asi que "
              "la maquina de luces avanza en todos los modos. Es lo que cierra el fallo "
              "del cabezal a oscuras en el asistente de Automatico.",
              "main.cpp ya NO llama a semaforo_actualizar() sin condicion: una secuencia "
              "del mando en el asistente de Automatico volveria a dejar las seis salidas "
              "apagadas indefinidamente y el mando sordo.")

    # Con esa llamada, TODOS los estados bombean. La columna se conserva para que el
    # banco siga sirviendo si algun dia se revierte.
    _BOMBEA_SIEMPRE = _llamada_incondicional

    ESTADOS_MAESTRO = [
        # nombre                                   bombea  inhibe  ya_seguro
        ("MENU",                                    True,   True,   True),
        ("MODO_HORA",                               True,   True,   True),
        ("MODO_MANUAL / configuracion",             True,   False,  False),
        ("MODO_MANUAL / corriendo",                 True,   False,  False),
        ("MODO_INTELIGENTE / corriendo",            True,   False,  False),
        ("MODO_ALCANCE",                            True,   False,  True),
        ("MODO_AUTOMATICO / CONFIG_ROJO",           False,  False,  False),
        ("MODO_AUTOMATICO / CONFIG_VERDE",          False,  False,  False),
        ("MODO_AUTOMATICO / CONFIG_ESTATICO",       False,  False,  False),
        ("MODO_AUTOMATICO / CORRIENDO",             True,   False,  False),
        ("MODO_DEGRADADO / DEG_ENTRADA_ROJO",       True,   False,  False),
        ("MODO_DEGRADADO / DEG_ACTIVO",             True,   False,  False),
        ("MODO_DEGRADADO / DEG_AMBAR",              True,   False,  True),
        ("MODO_DEGRADADO / DEG_SALIDA_ROJO",        True,   False,  True),
        ("MODO_DEGRADADO / DEG_RECHAZO",            True,   False,  True),
        ("MODO_AMBAR",                              True,   False,  True),
    ]

    sin_salida, muertos = [], []
    for nombre, bombea_tabla, inhibe, ya_seguro in ESTADOS_MAESTRO:
        bombea = bombea_tabla or _BOMBEA_SIEMPRE
        sem = Semaforo(); sem.ahora = 0
        m = Mando(sem, puerta_ok=True)
        m.inhibido = inhibe
        eventos = {0: MANDO_B, 2000: MANDO_B, 4000: MANDO_B}
        t = 0
        while t <= 600000:      # diez minutos de reloj: de sobra para 3 destellos
            sem.ahora = t
            if t in eventos:
                m.registrar_pulso(eventos[t], t)
            if bombea:
                sem.actualizar(t)
            m.actualizar()
            t += 10
        llego_ambar = ACC_AMBAR in [a for _t, a in m.ejecutadas]
        # "Muerto" = ni ejecuto la accion ni dejo las luces en algo interpretable:
        # todas las salidas apagadas es un cabezal a oscuras.
        apagado = (sem.pines == (0, 0, 0))
        atascado = (m.pendiente != ACC_NINGUNA)
        if not llego_ambar and not (inhibe and ya_seguro):
            sin_salida.append(nombre)
        if atascado or (apagado and not llego_ambar and not inhibe):
            muertos.append((nombre, sem.pines, NOMBRE_ACC[m.pendiente]))

    verificar(not sin_salida,
              "B.B.B lleva a AMBAR INTERMITENTE desde todos los estados en marcha, y en los "
              "dos que lo inhiben (MENU y MODO_HORA) el equipo ya esta en estado seguro.",
              "B.B.B NO tiene salida desde: " + ", ".join(sin_salida))

    verificar(not muertos,
              "Tras B.B.B ningun estado deja la accion colgada ni el cabezal apagado.",
              "B.B.B deja el equipo MUERTO (accion pendiente que no se ejecuta nunca y/o "
              "las seis salidas apagadas) en: " +
              "; ".join(f"{n} pines={p} pendiente={q}" for n, p, q in muertos))

    # --- 1.8 -------------------------------------------------------------------
    # EL MISMO CAMINO, PERO POR LA RAMA DEL RECHAZO. Se comprueba aparte porque el
    # sintoma era distinto y peor de diagnosticar: en vez de quedarse a oscuras, el
    # ambar de rechazo se congelaba FIJO en vez de parpadear 2 s, y un ambar fijo no
    # es ninguna de las senales del contrato -ni rechazo (150 ms), ni fallo (500 ms),
    # ni ciclo-.
    #
    # CORRECCION DEL 01/08/2026: este bucle modelaba a mano "nadie bombea el
    # semaforo", que era cierto cuando se escribio y dejo de serlo con la llamada
    # incondicional de main.cpp. Ahora usa la MISMA premisa leida del fuente que el
    # bloque 1.7, para que las dos pruebas no puedan discrepar sobre lo que hace el
    # firmware. Si alguien revierte la llamada, las dos caen a la vez.
    sem = Semaforo(); sem.ahora = 0
    m = Mando(sem, puerta_ok=False)          # puerta cerrada -> rechazo
    eventos = {0: MANDO_A, 2000: MANDO_B, 4000: MANDO_A, 6000: MANDO_B}
    t = 0
    while t <= 300000:
        sem.ahora = t
        if t in eventos:
            m.registrar_pulso(eventos[t], t)
        if _BOMBEA_SIEMPRE:
            sem.actualizar(t)
        m.actualizar()
        t += 10
    rechazo_congelado = sem.senal_activa or sem.pines == (0, 1, 0)
    verificar(not rechazo_congelado,
              f"El ambar rapido de rechazo ({RECHAZO_AMBAR_MS} ms) termina siempre, tambien "
              "en el asistente del Modo Automatico, y devuelve las luces a lo que la logica "
              "hubiera decidido mientras tanto.",
              f"El ambar de RECHAZO se queda FIJO indefinidamente (pines={sem.pines}, "
              f"senal_activa={sem.senal_activa}): un ambar fijo no es ninguna de las senales "
              f"del contrato -ni rechazo, ni fallo, ni ciclo-.")

    # --- 1.9 -------------------------------------------------------------------
    # Prueba de que el banco distingue: el MISMO B.B.B en un modo que si bombea debe
    # resolverse rapido. Si esto tambien fallara, el fallo estaria en el modelo y no
    # en el firmware, y el hallazgo de 1.7 no valdria nada.
    m_ok, sem_ok = correr_tren([MANDO_B] * 3, 2000, bombea=True, ms_extra=10000)
    duracion_esperada = DESTELLOS_AMBAR * (DESTELLO_ON_MS + DESTELLO_OFF_MS)
    verificar(ACC_AMBAR in [a for _t, a in m_ok.ejecutadas]
              and sem_ok.destellos_vistos == DESTELLOS_AMBAR
              and not sem_ok.senal_activa,
              f"Control del modelo: con el semaforo bombeado, B.B.B da sus "
              f"{DESTELLOS_AMBAR} destellos rojos ({duracion_esperada} ms) y ejecuta el ambar. "
              "El modelo distingue el caso bueno del malo.",
              f"El modelo falla tambien en el caso bueno "
              f"(destellos={sem_ok.destellos_vistos}, ejecutadas={m_ok.ejecutadas}): "
              "el hallazgo de 1.7 seria un artefacto del banco.")

    # --- 1.10 ------------------------------------------------------------------
    # El firmware comprueba la ventana cuadruple DESPUES de purgar, y purgarViejos()
    # ya descarta todo lo que exceda esa misma ventana. La comprobacion de tiempo del
    # A.B.A.B es por tanto SIEMPRE cierta: codigo muerto. No es peligroso, pero si es
    # engañoso, porque hace creer que hay dos filtros donde solo hay uno.
    redundante = True
    for cad in range(50, 12001, 50):
        for tr in ([MANDO_A, MANDO_B, MANDO_A, MANDO_B], [MANDO_A] * 4, [MANDO_A, MANDO_B] * 3):
            sem = Semaforo(); sem.ahora = 0
            m = Mando(sem)
            for i, b in enumerate(tr):
                t = i * cad
                sem.ahora = t
                m.registrar_pulso(b, t)
                # Tras purgar y almacenar, ?puede el pulso mas antiguo del buffer estar
                # fuera de la ventana cuadruple? Si nunca puede, la comprobacion de
                # tiempo del A.B.A.B no filtra nada.
                if m.n >= 4 and (t - m.tiempo[m.n - 4]) > VENTANA_CUADRUPLE_MS:
                    redundante = False
    verificar(redundante,
              "La comprobacion de ventana del A.B.A.B nunca puede ser falsa tras purgarViejos(): "
              "es codigo muerto (inofensivo, pero aparenta un segundo filtro que no existe).",
              "La comprobacion de ventana del A.B.A.B si llega a ser falsa: el purgado no la cubre.")


    # ==========================================================================
    # BLOQUE 2 — RESPALDO EN PILA Y REANUDACION (N-20, respaldo.cpp)
    # ==========================================================================
