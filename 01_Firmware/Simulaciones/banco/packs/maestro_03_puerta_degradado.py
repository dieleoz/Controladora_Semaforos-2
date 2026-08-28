# ===== banco/packs/maestro_03_puerta_degradado.py =====
#
# PUERTA DE ENTRADA DEL MODO DEGRADADO
#
# La guarda que decide si el Modo Degradado puede entrar. Es la barrera que impide
# dar VERDE sin radio: un verde equivocado es mas peligroso que un ambar ambiguo,
# porque el conductor llega confiado en vez de alerta.
# Cada prueba lleva su MUTANTE al lado -la version defectuosa- para demostrar que
# sabe distinguir el caso bueno del malo.

from banco.modelos.maestro import *          # noqa: F401,F403
from banco.modelos.maestro import (          # los guiones bajos no
    _codigo, _fuente, _main, _ruta,          # los exporta import *
)

NOMBRE = "maestro_03_puerta_degradado"
DESCRIPCION = "la puerta de entrada del Modo Degradado"


def correr(b, fw):
    # Bloque traido LITERAL del validador monolitico, solo reindentado. Reescribir
    # logica ya probada para renombrar las llamadas es como se cuelan los errores en
    # una migracion que se supone que no cambia comportamiento.
    verificar = b.verificar
    titulo = b.titulo
    control_negativo = b.control_negativo
    reportar = b.reportar

    (MDG_OK, MDG_FALTA_HORA, MDG_NUNCA_SYNC, MDG_SYNC_VIEJA, MDG_SIN_DESFASE,
     MDG_DESFASE_ALTO, MDG_SIN_CONFIG) = range(7)
    NOMBRE_MDG = {MDG_OK: "OK", MDG_FALTA_HORA: "FALTA_HORA", MDG_NUNCA_SYNC: "NUNCA_SYNC",
                  MDG_SYNC_VIEJA: "SYNC_VIEJA", MDG_SIN_DESFASE: "SIN_DESFASE",
                  MDG_DESFASE_ALTO: "DESFASE_ALTO", MDG_SIN_CONFIG: "SIN_CONFIG"}

    # El orden de las comprobaciones se LEE del fuente, no se supone. Importa porque el
    # motivo que se muestra en pantalla es el de la PRIMERA condicion que falla, y ese
    # texto es lo unico que tiene el tecnico subido a 5 m para saber a que ha ido.
    _orden_puerta = re.findall(r"return (MDG_\w+);",
                               re.search(r"MotivoDegradado modo_degradado_evaluarEntrada\(\)"
                                         r"\s*\{(.*?)\n\}",
                                         _codigo("Maestro", "src", "modo_degradado.cpp"),
                                         re.S).group(1))
    verificar(_orden_puerta == ["MDG_FALTA_HORA", "MDG_NUNCA_SYNC", "MDG_SYNC_VIEJA",
                               "MDG_SIN_CONFIG", "MDG_SIN_DESFASE", "MDG_DESFASE_ALTO",
                               "MDG_OK"],
              f"La puerta comprueba, en este orden: {_orden_puerta[:-1]}. Es el orden que "
              "modela el banco, e incluye la condicion MDG_SIN_CONFIG anadida el 01/08/2026.",
              f"El orden o el juego de condiciones de la puerta cambio a {_orden_puerta}: "
              "el modelo del banco ya no la describe")

    # La puerta ya no llama a coordinador_msDesdeUltimaSync() sino a msDesdeSyncEfectivo(),
    # que contrasta la RAM contra el reloj de pared. Se verifica en el fuente porque de esa
    # premisa dependen las cuatro pruebas del desbordamiento.
    _cuerpo_puerta = re.search(r"MotivoDegradado modo_degradado_evaluarEntrada\(\)\s*\{(.*?)\n\}",
                               _codigo("Maestro", "src", "modo_degradado.cpp"), re.S).group(1)
    verificar("msDesdeSyncEfectivo()" in _cuerpo_puerta
              and "coordinador_msDesdeUltimaSync()" not in _cuerpo_puerta,
              "La puerta pide la antiguedad a msDesdeSyncEfectivo() y NO a "
              "coordinador_msDesdeUltimaSync(): la medida que se desborda a los 49,7 dias "
              "ya no decide sola.",
              "La puerta ha vuelto a leer coordinador_msDesdeUltimaSync() directamente: el "
              "desbordamiento de 49,7 dias volveria a abrirla")


    def evaluar_entrada(en_hora, ms_efectivo, config_confirmada, hay_desfase, desfase):
        """Port exacto de modo_degradado_evaluarEntrada() tras el arreglo del
        01/08/2026. El primer argumento de antiguedad es ya el EFECTIVO."""
        if not en_hora:
            return MDG_FALTA_HORA
        if ms_efectivo == 0xFFFFFFFF:
            return MDG_NUNCA_SYNC
        if ms_efectivo >= SYNC_FRESCA_MS:
            return MDG_SYNC_VIEJA
        if not config_confirmada:
            return MDG_SIN_CONFIG
        if not hay_desfase:
            return MDG_SIN_DESFASE
        if desfase > TOLERANCIA_DESFASE_S or desfase < -TOLERANCIA_DESFASE_S:
            return MDG_DESFASE_ALTO
        return MDG_OK


    def evaluar_entrada_mutante(en_hora, ms_efectivo, config_confirmada, hay_desfase, desfase):
        """MUTANTE: el que trata 'nunca sincronizado' como 'hace 0 ms', que es el
        error que el comentario del firmware avisa expresamente."""
        if not en_hora:
            return MDG_FALTA_HORA
        ms = 0 if ms_efectivo == 0xFFFFFFFF else ms_efectivo
        if ms >= SYNC_FRESCA_MS:
            return MDG_SYNC_VIEJA
        if not config_confirmada:
            return MDG_SIN_CONFIG
        if not hay_desfase:
            return MDG_SIN_DESFASE
        if desfase > TOLERANCIA_DESFASE_S or desfase < -TOLERANCIA_DESFASE_S:
            return MDG_DESFASE_ALTO
        return MDG_OK


    # --- Calendario real -------------------------------------------------------
    # respaldo_horasDesdeSync() solo conoce el DIA DEL MES, asi que para atacar el
    # desbordamiento de millis() hace falta saber en que dia del mes cae cada instante.
    # reloj_ajustar() fija mes 1 si la fecha no es valida y el ano de marca es 2026,
    # que no es bisiesto: el calendario del RTC es el de un ano comun.
    MESES = (31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31)
    DIAS_ANIO = sum(MESES)
    _DIA_DEL_MES = []
    for _m, _dur in enumerate(MESES):
        _DIA_DEL_MES.extend(range(1, _dur + 1))


    def dia_del_mes(dia_absoluto):
        return _DIA_DEL_MES[dia_absoluto % DIAS_ANIO]


    def _rtc_leido(v):
        """Espejo del remap de reloj_contadorSegundos() (Maestro/reloj.cpp:222-237,
        identico en el Esclavo): un contador que lee EXACTAMENTE cero -el primer
        segundo tras un reinicio del dominio de respaldo- se declara 1, para que el
        centinela de 'no hay reloj' nunca choque con una medida real.

        Sin este espejo, dia_abs_sync=0 y seg_sync=0 le pasan a marcar_sync() un 0
        crudo, que el modelo (fiel al firmware) rechaza como 'no hay reloj'. Eso NO
        es un hallazgo del firmware: es alimentar el barrido con un valor que la RTC
        real jamas entrega. Se detecto el 05/08/2026 al reproducir a mano el
        'veto de 5 escenarios' que se habia anotado como regresion (roadmap.md,
        N-49): los 5 eran el MISMO caso, dia_abs_sync=0 seg_sync=0."""
        return 1 if v == 0 else v

    def ms_desde_sync_efectivo_v2(ms_ram, dia_abs_sync, seg_sync, dia_abs_ahora, seg_ahora,
                                  hay_sync=True):
        """Port de msDesdeSyncEfectivo() TAL COMO QUEDO tras el arreglo.

        Contrasta la medida de RAM -que se desborda- contra la marca de reloj de pared
        del respaldo -que no-, y se queda con la mayor. Con la RAM caducada cae a la
        pila, como antes."""
        reg = marcar_sync(_rtc_leido(dia_abs_sync * 86400 + seg_sync)) if hay_sync else None
        if reg is None:
            horas_pila = SYNC_CADUCADA
        else:
            horas_pila = horas_desde_sync(reg[0], reg[1], True, None,
                                          _rtc_leido(dia_abs_ahora * 86400 + seg_ahora))

        if ms_ram != 0xFFFFFFFF:
            if horas_pila == SYNC_CADUCADA:
                return 0xFFFFFFFF
            ms_pila = horas_pila * 3600000
            return ms_pila if ms_pila > ms_ram else ms_ram

        if horas_pila == SYNC_CADUCADA:
            return 0xFFFFFFFF
        if horas_pila >= LIMITE_DURO_H:
            return 0xFFFFFFFF
        return horas_pila * 3600000


    def escenario(dia_abs_sync, seg_sync, transcurrido_ms, sincronizo=True):
        """Traduce 'la sync ocurrio el dia X a la hora Y y han pasado Z ms' a lo que
        ven las dos fuentes: la RAM (con su desbordamiento) y el reloj de pared."""
        ms_ram = 0xFFFFFFFF if not sincronizo else (transcurrido_ms % UINT32)
        total_seg = seg_sync + transcurrido_ms // 1000
        dia_abs_ahora = dia_abs_sync + total_seg // SEGUNDOS_DEL_DIA
        seg_ahora = total_seg % SEGUNDOS_DEL_DIA
        return ms_desde_sync_efectivo_v2(ms_ram, dia_abs_sync, seg_sync,
                                         dia_abs_ahora, seg_ahora, sincronizo)


    # --- 3.1 -------------------------------------------------------------------
    # El centinela. 0xFFFFFFFF significa "NUNCA sincronizo", no "hace 0 ms". Se prueba
    # en el valor exacto y en sus vecinos, y se comprueba que el mutante que lo lee
    # como cero SI entra: si el mutante tambien fuera rechazado, esta prueba no
    # estaria midiendo nada.
    r_centinela = evaluar_entrada(True, 0xFFFFFFFF, True, True, 0)
    r_mutante = evaluar_entrada_mutante(True, 0xFFFFFFFF, True, True, 0)
    verificar(r_centinela == MDG_NUNCA_SYNC and r_mutante == MDG_OK,
              "El centinela 0xFFFFFFFF se rechaza como NUNCA_SYNC, y el mutante que lo lee "
              "como 'hace 0 ms' SI entra: la prueba discrimina.",
              f"El centinela no se trata como nunca sincronizado: {NOMBRE_MDG[r_centinela]}")

    malos_frescura = []
    for ms in [0, 1, SYNC_FRESCA_MS - 1, SYNC_FRESCA_MS, SYNC_FRESCA_MS + 1,
               LIMITE_DURO_MS, 0xFFFFFFFE, 0xFFFFFFFF]:
        r = evaluar_entrada(True, ms, True, True, 0)
        debe = MDG_NUNCA_SYNC if ms == 0xFFFFFFFF else (
            MDG_OK if ms < SYNC_FRESCA_MS else MDG_SYNC_VIEJA)
        if r != debe:
            malos_frescura.append((ms, NOMBRE_MDG[r]))
    verificar(not malos_frescura,
              f"El corte de frescura cae exactamente en {SYNC_FRESCA_MS} ms y 0xFFFFFFFE "
              "(49,7 dias menos 1 ms) se rechaza como vieja, no como centinela.",
              f"El corte de frescura no cae donde debe: {malos_frescura}")

    verificar(evaluar_entrada(False, 0, True, True, 0) == MDG_FALTA_HORA,
              "Sin reloj en hora la puerta se cierra antes de mirar nada mas (SFTY-18): "
              "una fase calculada sobre una hora inventada seria una fase inventada.",
              "La puerta deja pasar sin reloj en hora")

    # --- 3.1 bis ---------------------------------------------------------------
    # LA CONDICION NUEVA: el Esclavo tiene que haber ACUSADO el ciclo. Sin ella el
    # Maestro aceptaba y daba verde por reloj mientras el Esclavo rechazaba por falta
    # de configuracion y caia a ambar por orfandad. Se comprueba que es CONDICION, no
    # adorno: con todo lo demas perfecto, la puerta se cierra igual.
    verificar(evaluar_entrada(True, 0, False, True, 0) == MDG_SIN_CONFIG and
              evaluar_entrada(True, 0, True, True, 0) == MDG_OK,
              "Sin el acuse CMD_ACK_CONFIG del Esclavo la puerta se cierra con MDG_SIN_CONFIG, "
              "aunque el reloj, la frescura y el desfase esten perfectos: las dos puntas no "
              "pueden dar respuestas distintas a la misma peticion.",
              "La puerta acepta sin que conste que el Esclavo tiene el ciclo")

    # Y el indicador tiene que BAJARSE cuando al otro lado puede haber otra unidad, o
    # seria un permiso que se concede una vez y ya nunca caduca.
    _cod_coord = _codigo("Maestro", "src", "coordinador.cpp")
    _bajadas = len(re.findall(r"configConfirmada\s*=\s*false", _cod_coord))
    _subidas = len(re.findall(r"configConfirmada\s*=\s*true", _cod_coord))
    verificar(_subidas == 1 and _bajadas >= 2,
              f"configConfirmada se pone a true en un unico sitio (el acuse) y se baja en "
              f"{_bajadas}: al reiniciar la conexion y al recuperarse el enlace. Lo que "
              "confirmo la unidad anterior no dice nada de la que haya ahora.",
              f"configConfirmada se sube en {_subidas} sitios y se baja en {_bajadas}: o se "
              "concede por mas de un camino o no caduca cuando cambia el interlocutor")

    # --- 3.2 -------------------------------------------------------------------
    # EL ALIAS DEL DESFASE. CMD_DELTA transporta SOLO el segundo (0..59), asi que el
    # Esclavo resuelve por el camino corto y el resultado cae siempre en +-30 s. La
    # pregunta es si un desfase REAL mayor puede colarse por la puerta.
    #
    # Se barre el desfase real de -2 h a +2 h segundo a segundo (14401 casos) y se
    # mira cuantos, siendo mayores que la tolerancia, la puerta acepta.
    def delta_medido(desfase_real_s):
        """Lo que el Esclavo puede reportar: la diferencia circular de segundos, en
        el rango [-30, +29]."""
        return ((desfase_real_s + 30) % 60) - 30


    colados = []
    for real in range(-7200, 7201):
        d = delta_medido(real)
        if evaluar_entrada(True, 0, True, True, d) == MDG_OK \
                and abs(real) > TOLERANCIA_DESFASE_S:
            colados.append(real)
    # Esta comprobacion afirmaba "no se cuela ninguno" y se dejaba FALLANDO a proposito
    # para que el limite no se olvidara. Era la tercera cara del mismo error que este
    # banco persigue: primero se conto un ABORTADO como PASS, luego un FALLA como PASS,
    # y aqui un FALLA PERMANENTE que ninguna correccion puede apagar -CMD_DELTA lleva un
    # solo byte de segundos y ninguna aritmetica distingue 0 de 60-. Una compuerta que
    # NUNCA puede salir en verde ensena a ignorar su codigo de salida, que es justo lo
    # que hace peligroso un banco.
    #
    # Asi que se invierte: no se exige lo imposible, se exige que el agujero sea
    # EXACTAMENTE el que el protocolo obliga y ni un caso mas. Si manana se cuela un
    # desfase que NO es un alias de 60 s, eso si es un defecto y esto lo caza.
    def es_alias_de_minuto(r):
        """Cerca de un multiplo de 60 s. Se calcula por distancia al multiplo, no
        repitiendo el modulo circular de delta_medido(): si se reusara la misma
        formula, la comprobacion solo se estaria comparando consigo misma."""
        return min(abs(r - 60 * k) for k in range(-121, 122)) <= TOLERANCIA_DESFASE_S

    intrusos = [r for r in colados if not es_alias_de_minuto(r)]
    verificar(not intrusos,
              f"Barrido del desfase real de -2 h a +2 h: los {len(colados)} que pasan la "
              f"puerta son TODOS alias de un multiplo de 60 s -lo unico que un byte de "
              f"segundos no puede distinguir-. Ninguno se cuela por otra via.",
              f"{len(intrusos)} desfases que NO son alias de 60 s PASAN la puerta: "
              f"{sorted(intrusos, key=abs)[:6]} s. Eso ya no es el limite del protocolo, "
              f"es un defecto de la comprobacion de cordura")

    # Y que sepa fallar: si el Esclavo reportara siempre 0 en vez de la medida circular,
    # se colarian desfases que no son alias y la comprobacion de arriba tiene que verlo.
    _mutante_acepta = evaluar_entrada(True, 0, True, True, 0) == MDG_OK
    _colados_mutante = [r for r in range(-7200, 7201)
                        if _mutante_acepta and abs(r) > TOLERANCIA_DESFASE_S]
    control_negativo(any(not es_alias_de_minuto(r) for r in _colados_mutante),
                     "con un Esclavo que reportase 0 s siempre, entran desfases que no son "
                     "alias de minuto y la comprobacion de arriba los distingue")

    reportar("RESIDUAL INHERENTE DEL PROTOCOLO, NO REGRESION: el alias de +-60 s",
             [f"{len(colados)} desfases reales mayores de +-{TOLERANCIA_DESFASE_S} s pasan la "
              f"comprobacion de cordura. Los mas pequenos: {sorted(colados, key=abs)[:6]} s.",
              "La regla es que TODO multiplo de 60 s se lee como cero, no solo los grandes.",
              "CMD_DELTA transporta un solo byte de segundos: ninguna aritmetica puede "
              "distinguir 0 de 60. No tiene arreglo sin cambiar el protocolo.",
              "Lo que lo CONTIENE es la frescura (3.3), que sigue en pie: con la deriva "
              "conocida no da tiempo a acumular un minuto dentro de la ventana."])

    # --- 3.2 bis ---------------------------------------------------------------
    # LA DOCUMENTACION DEL ALIAS. El comentario original ponia de ejemplo un desfase de
    # 45 s "que pasaria por bueno", y era FALSO: 45 s se lee como -15 s y SI se detecta.
    # Quien leyera ese comentario para decidir la tolerancia trabajaria sobre una idea
    # equivocada de que deja pasar. Se comprueba que el texto de hoy nombra el caso
    # real -el multiplo de 60 s- porque un comentario erroneo en una funcion de
    # seguridad es un defecto, no una errata.
    _txt_deg = _fuente("Maestro", "src", "modo_degradado.cpp")
    _lineas_45 = [i + 1 for i, l in enumerate(_txt_deg.splitlines())
                  if "45 s" in l and "pasaria por bueno" in l]
    _menciona_minuto = bool(re.search(r"multiplo de 60", _txt_deg))
    verificar(_menciona_minuto and not _lineas_45,
              "El comentario del alias nombra el caso real (todo multiplo de 60 s se lee como "
              "cero) y ya no queda ningun sitio afirmando que 45 s pasaria por bueno.",
              f"LA CORRECCION SE HIZO A MEDIAS Y EL FICHERO SE CONTRADICE. Se anadio la "
              f"explicacion correcta dentro de evaluarEntrada, pero la afirmacion FALSA "
              f"original sigue viva en la linea {_lineas_45} -la cabecera que explica por que "
              f"la garantia es la frescura-, que es justamente el bloque que alguien lee para "
              f"entender el diseno. Ahora el mismo fichero dice que 45 s 'pasaria por bueno' "
              f"(linea {_lineas_45[0] if _lineas_45 else '-'}) y que 45 s 'si se detecta'. Dos "
              f"comentarios contradictorios en una funcion de seguridad son peores que uno "
              f"equivocado: el que corrija manana no sabra cual creer.")

    # --- 3.3 -------------------------------------------------------------------
    # La contramedida documentada para el alias es que la sincronizacion sea FRESCA:
    # con la deriva conocida no da tiempo a acumular 60 s en 2 h. Se comprueba la
    # cuenta con el numero del propio firmware (~8,6 s/dia en el peor caso, que es de
    # donde salen los 30 s de despeje y las 48 h de limite).
    DERIVA_PEOR_S_DIA = 8.6
    deriva_en_frescura = DERIVA_PEOR_S_DIA * (SYNC_FRESCA_MS / 86400000.0)
    verificar(deriva_en_frescura < 60 - TOLERANCIA_DESFASE_S,
              f"Con la ventana de frescura de {SYNC_FRESCA_MS/3600000:.0f} h y una deriva peor "
              f"de {DERIVA_PEOR_S_DIA} s/dia solo se acumulan {deriva_en_frescura:.2f} s: no da "
              "tiempo a alcanzar el minuto que produciria el alias. La puerta se sostiene "
              "sobre la FRESCURA, no sobre el numero.",
              f"En la ventana de frescura caben {deriva_en_frescura:.1f} s de deriva: el alias "
              "de 60 s deja de ser inalcanzable")

    # --- 3.4 -------------------------------------------------------------------
    # EL DESBORDAMIENTO DE 49,7 DIAS, CONTRA EL ARREGLO DEL 01/08/2026.
    #
    # La medida de RAM sigue dando la vuelta -eso no ha cambiado ni puede cambiar-,
    # pero ahora se contrasta contra la marca de reloj de pared de la pila y se toma
    # LA MAYOR. La pregunta no es si la RAM miente, sino si la pila la desmiente
    # SIEMPRE, y eso no se puede razonar de cabeza: respaldo_horasDesdeSync() solo
    # conoce el DIA DEL MES, asi que su veredicto depende de en que dia del mes
    # empezo la cuenta y de cuantos meses se cruzan.
    #
    # Se barre con calendario REAL: los 365 dias posibles de inicio x las horas del
    # dia x el entorno de los SIETE primeros multiplos del desbordamiento (49,7 dias,
    # 99,4, 149,1... hasta casi un ano). Si algun multiplo cae a menos de dos dias de
    # un numero entero de meses, el dia del mes cuadraria y la pila NO desmentiria.
    falsas_frescas = []
    for k in range(1, 8):
        centro = k * UINT32                       # ms de cada vuelta del contador
        for dia_ini in range(0, DIAS_ANIO):
            for hora_ini in (0, 7, 13, 19):
                seg_ini = hora_ini * 3600
                for delta_min in range(-180, 181, 30):
                    real = centro + delta_min * 60000
                    if real < 0:
                        continue
                    efec = escenario(dia_ini, seg_ini, real)
                    if efec != 0xFFFFFFFF and efec < SYNC_FRESCA_MS:
                        falsas_frescas.append((k, dia_ini, hora_ini, delta_min))
    verificar(not falsas_frescas,
              f"Barrido con calendario real (365 dias de inicio x 4 horas x 7 vueltas del "
              f"contador, {365*4*7*13} escenarios): en NINGUNO el desbordamiento de millis() "
              "consigue que la puerta declare fresca una sincronizacion de 49,7 dias o mas. "
              "El contraste contra el reloj de pared la desmiente siempre, porque 49,7 dias "
              "nunca caen a menos de dos dias de un numero entero de meses.",
              f"El desbordamiento SIGUE abriendo la puerta en {len(falsas_frescas)} "
              f"escenarios, p.ej. {falsas_frescas[:3]} (vuelta, dia de inicio, hora, "
              "desviacion en min). El contraste contra la pila no lo cierra.")

    # --- 3.5 -------------------------------------------------------------------
    # LO MISMO PARA EL LIMITE DURO EN MARCHA, que es un umbral MUCHO mas ancho: la
    # puerta exige frescura de 2 h, pero el limite solo exige no llegar a 48 h. Basta
    # con que el contraste de la pila devuelva CUALQUIER valor por debajo de 48 h para
    # que el modo siga corriendo.
    #
    # Y ahi esta el limite del arreglo: respaldo_horasDesdeSync() solo compara EL DIA
    # DEL MES. Si una vuelta del contador cae cerca de un numero entero de meses, el
    # dia del mes cuadra y la pila CONFIRMA la mentira de la RAM en vez de desmentirla.
    limite_burlado = []
    for k in range(1, 10):
        centro = k * UINT32
        for dia_ini in range(0, DIAS_ANIO):
            for delta_h in range(-60, 61, 6):
                real = centro + delta_h * 3600000
                if real < 0:
                    continue
                if escenario(dia_ini, 12 * 3600, real) < LIMITE_DURO_MS:
                    limite_burlado.append((k, dia_del_mes(dia_ini), delta_h,
                                           round(real / 86400000.0, 1)))
    _vueltas_malas = sorted({v[0] for v in limite_burlado})
    verificar(not limite_burlado,
              f"El limite duro de {LIMITE_DURO_H} h se dispara tambien en el entorno de las "
              "nueve primeras vueltas del contador, para los 365 dias de inicio.",
              f"EL ARREGLO NO CIERRA EL LIMITE DURO: queda burlado en {len(limite_burlado)} "
              f"escenarios, concentrados en las vueltas {_vueltas_malas} del contador "
              f"({', '.join(f'{v*49.71:.0f} d' for v in _vueltas_malas)}). Causa: el contraste "
              f"lo hace respaldo_horasDesdeSync(), que SOLO compara el dia del mes; cuando una "
              f"vuelta cae a menos de dos dias de un numero entero de meses, el dia cuadra y la "
              f"pila CONFIRMA la mentira de la RAM en vez de desmentirla. Ejemplos "
              f"(vuelta, dia del mes de la sync, desviacion en h, dias reales): "
              f"{limite_burlado[:3]}. "
              f"ATENUANTE: para llegar ahi el equipo tendria que estar YA dentro del Degradado, "
              f"y la puerta -que exige 2 h de frescura- si esta cerrada (ver 3.4), asi que no "
              f"es explotable por si solo. Pero significa que en esas ventanas la proteccion "
              f"tiene UNA sola capa, no dos.")

    # --- 3.6 -------------------------------------------------------------------
    # EL ARREGLO TIENE UN PRECIO Y HAY QUE MEDIRLO. Ahora la pila puede VETAR a la
    # RAM: si respaldo_horasDesdeSync() dice CADUCADA, msDesdeSyncEfectivo() devuelve
    # el maximo AUNQUE la RAM tenga una medida perfectamente valida y reciente.
    #
    # Y CADUCADA no significa solo "muy vieja": tambien significa "no fechable", y el
    # caso mas comun de no fechable es EL CAMBIO DE MES, porque el numero de dia baja.
    #
    # Se barre un equipo COMPLETAMENTE SANO -radio vivo, sincronizado hace menos de la
    # ventana de frescura- a lo largo del ano entero, y se exige que la puerta lo deje
    # entrar. Si la pila veta a la RAM en algun dia del ano, aparece aqui.
    vetos_indebidos = []
    for dia_ini in range(0, DIAS_ANIO):
        for seg_ini in range(0, SEGUNDOS_DEL_DIA, 1800):
            for antig_min in (1, 30, 60, 90, 119):
                real = antig_min * 60000
                if real >= SYNC_FRESCA_MS:
                    continue
                efec = escenario(dia_ini, seg_ini, real)
                if efec == 0xFFFFFFFF or efec >= SYNC_FRESCA_MS:
                    vetos_indebidos.append((dia_del_mes(dia_ini), seg_ini // 3600, antig_min,
                                            "CADUCADA" if efec == 0xFFFFFFFF else efec))
    verificar(not vetos_indebidos,
              f"Barrido del ano completo ({DIAS_ANIO} dias x 48 horas de inicio x 5 "
              "antiguedades): un equipo sano y sincronizado dentro de la ventana de frescura "
              "entra SIEMPRE. La pila no veta nunca a una RAM valida.",
              f"REGRESION DEL ARREGLO: en {len(vetos_indebidos)} escenarios la pila VETA a una "
              f"RAM valida y la puerta rechaza un equipo sano. Todos son cruces de mes: la "
              f"sincronizacion ocurrio el ultimo dia del mes y la consulta cae ya en el dia 1, "
              f"asi que respaldo_horasDesdeSync() devuelve CADUCADA -el numero de dia baja- y "
              f"msDesdeSyncEfectivo() lo traduce al maximo. Ejemplos (dia del mes, hora de la "
              f"sync, antiguedad en min, efectivo): {vetos_indebidos[:4]}. El tecnico lee "
              f"'Falta: nunca hubo sincronizacion RF' sobre un equipo que sincronizo hace "
              f"minutos.")

    # --- 3.7 -------------------------------------------------------------------
    # Y EL CASO PEOR DEL MISMO EFECTO: no en la puerta, sino EN MARCHA. Dentro de
    # modo_degradado_loop() el limite duro se evalua en cada iteracion con la misma
    # funcion, y en Degradado el Maestro CALLA en la radio, asi que la marca de la
    # pila ya no se refresca nunca. Al cruzar la medianoche de fin de mes, la marca
    # pasa a ser no fechable y el modo se declara agotado de golpe.
    #
    # Se simula la entrada en Degradado a cada hora de cada dia del ano y se sigue la
    # sesion hasta el limite duro, buscando el instante en que el modo se rinde.
    rendiciones_prematuras = []
    for dia_ini in range(0, DIAS_ANIO):
        for hora_ini in (20, 23):
            seg_ini = hora_ini * 3600
            # La sesion arranca con una sincronizacion de 1 h: bien dentro de todo.
            edad_sync_ms = 3600000
            for avance_min in range(0, LIMITE_DURO_MS // 60000, 20):
                real = edad_sync_ms + avance_min * 60000
                if real >= LIMITE_DURO_MS:
                    break
                if escenario(dia_ini, seg_ini, real) >= LIMITE_DURO_MS:
                    rendiciones_prematuras.append(
                        (dia_del_mes(dia_ini), hora_ini, real / 3600000.0))
                    break
    verificar(not rendiciones_prematuras,
              "Sesiones de Degradado arrancadas a cada hora de cada dia del ano: ninguna se "
              f"rinde antes de agotar las {LIMITE_DURO_H} h reales.",
              f"REGRESION GRAVE: {len(rendiciones_prematuras)} sesiones de Degradado caen a "
              f"AMBAR antes de tiempo. La mas temprana lo hace con la sincronizacion a solo "
              f"{min((r[2] for r in rendiciones_prematuras), default=float('nan')):.2f} h de "
              f"antiguedad, contra un limite de {LIMITE_DURO_H} h. Ejemplos (dia del mes, hora "
              f"de entrada, horas reales al rendirse): {rendiciones_prematuras[:4]}")

    # --- 3.8 -------------------------------------------------------------------
    # ?Y QUE HACE LA OTRA PUNTA MIENTRAS TANTO? El Esclavo vigila su limite duro con
    # un latch sobre millis() PURO -"huboSyncAlguna && !syncVencidaLatch && (ahora -
    # tUltimaSync) >= LIMITE"-, sin consultar la pila. Se lee del fuente y se modela,
    # porque si las dos puntas se rinden por criterios distintos, se rinden en
    # instantes distintos, y eso es exactamente la asimetria que este modo evita.
    _esc_deg = _codigo("Esclavo", "src", "modo_degradado.cpp")
    _esc_limite_es_ram = bool(re.search(
        r"huboSyncAlguna\s*&&\s*!syncVencidaLatch\s*&&\s*\(ahora\s*-\s*tUltimaSync\)\s*>=",
        _esc_deg))
    _mae_limite_usa_pila = "msDesdeSyncEfectivo()" in _codigo("Maestro", "src",
                                                              "modo_degradado.cpp")


    def esclavo_rendido(real_ms):
        """Limite duro del Esclavo: RAM pura con latch pegajoso."""
        return real_ms >= LIMITE_DURO_MS


    discrepancias_puntas = []
    if _esc_limite_es_ram and _mae_limite_usa_pila:
        for dia_ini in range(0, DIAS_ANIO):
            seg_ini = 23 * 3600
            for avance_min in range(0, LIMITE_DURO_MS // 60000, 20):
                real = 3600000 + avance_min * 60000
                if real >= LIMITE_DURO_MS:
                    break
                mae = escenario(dia_ini, seg_ini, real) >= LIMITE_DURO_MS
                esc = esclavo_rendido(real)
                if mae != esc:
                    discrepancias_puntas.append((dia_del_mes(dia_ini), real / 3600000.0))
                    break
    verificar(not discrepancias_puntas,
              "Maestro y Esclavo se rinden por el mismo criterio y en el mismo instante: "
              "ninguna sesion termina con una punta en ambar y la otra ciclando.",
              f"LAS DOS PUNTAS SE RINDEN EN INSTANTES DISTINTOS en {len(discrepancias_puntas)} "
              f"dias del ano. El Maestro consulta la pila (y el cruce de mes se la invalida); "
              f"el Esclavo usa millis() puro con latch y NO se entera del cambio de mes. "
              f"Resultado: MAESTRO EN AMBAR mientras el ESCLAVO SIGUE DANDO VERDE por reloj. "
              f"Es el riesgo residual n.2 de SFTY-21 -el conductor negocia el ambar de un lado "
              f"y cruza confiado el verde del otro- reintroducido por el arreglo. "
              f"Ejemplos (dia del mes, horas de sesion): {discrepancias_puntas[:4]}")

    # --- 3.9 -------------------------------------------------------------------
    # Prueba de que el banco distingue: si el arreglo se hubiera hecho tomando el
    # MENOR de los dos valores en vez del mayor, el desbordamiento seguiria abierto.
    # Si esta prueba no cayera, la 3.4 no estaria midiendo nada.
    def efectivo_mutante_menor(ms_ram, dia_abs_sync, seg_sync, dia_abs_ahora, seg_ahora):
        reg = marcar_sync(_rtc_leido(dia_abs_sync * 86400 + seg_sync))
        hp = horas_desde_sync(reg[0], reg[1], True, None, _rtc_leido(dia_abs_ahora * 86400 + seg_ahora))
        if ms_ram != 0xFFFFFFFF:
            if hp == SYNC_CADUCADA:
                return ms_ram              # el mutante se fia de la RAM
            ms_pila = hp * 3600000
            return ms_pila if ms_pila < ms_ram else ms_ram
        return 0xFFFFFFFF if hp == SYNC_CADUCADA else hp * 3600000


    # Ambos modelos (real y mutante) reciben EXACTAMENTE los mismos 5 parametros de entrada:
    # ms_ram = 0 (RAM ha dado la vuelta), sync a las 12:00 del dia 0, y RTC en dia 49 a las 17:00 (1181 h transcurridas).
    # El modelo real toma max(1181h, 0ms) = 1181h -> VENCIDA (>= 2h).
    # El mutante toma min(1181h, 0ms) = 0ms -> FRESCA (< 2h).
    mut = efectivo_mutante_menor(0, 0, 12 * 3600, 49, 17 * 3600)
    real = ms_desde_sync_efectivo_v2(0, 0, 12 * 3600, 49, 17 * 3600, True)
    verificar(mut < SYNC_FRESCA_MS and real >= SYNC_FRESCA_MS,
              "Control del modelo: un arreglo que tomase el MENOR de los dos valores dejaria "
              f"el agujero abierto ({mut} ms = 'fresca') donde el real devuelve el maximo. "
              "El banco distingue el arreglo bueno del malo.",
              "El banco no distingue tomar el mayor de tomar el menor: la prueba 3.4 no mide "
              "el arreglo")

    # --- 3.10 ------------------------------------------------------------------
    # Un reinicio sigue cerrando la puerta por su cuenta: tras un reset millis() vuelve
    # a cero y syncAlgunaVez es false.
    verificar(escenario(0, 12 * 3600, 0, sincronizo=False) == 0xFFFFFFFF and
              evaluar_entrada(True, escenario(0, 12 * 3600, 0, sincronizo=False),
                              True, True, 0) == MDG_NUNCA_SYNC,
              "Tras un reinicio sin marca en la pila la puerta se cierra sola (centinela).",
              "Tras un reinicio la puerta no devuelve el centinela")

    # --- 3.11 ------------------------------------------------------------------
    # LA PANTALLA. Sigue SIN enclavamiento: la firma del Maestro no tiene el parametro
    # 'vencida' que si tiene la del Esclavo, asi que pinta lo que le den. Hoy lo que le
    # dan ya viene contrastado contra la pila, de modo que el numero no miente; pero la
    # defensa esta en el llamante y no en el que pinta, y eso es una capa menos.
    _lcd_mae = _codigo("Maestro", "src", "lcd.cpp")
    _lcd_esc = _codigo("Esclavo", "src", "lcd.cpp")
    _mae_tiene_vencida = bool(re.search(r"lcd_dibujarDegradado\([^)]*vencid", _lcd_mae, re.I))
    _esc_tiene_vencida = bool(re.search(r"lcd_dibujarDegradado\([^)]*[Vv]encida", _lcd_esc))
    verificar(_mae_tiene_vencida and _esc_tiene_vencida,
              "Las dos pantallas reciben un indicador de vencimiento y no solo un numero: "
              "ninguna puede pintar una antiguedad pequena por su cuenta.",
              f"ASIMETRIA QUE SIGUE ABIERTA: lcd_dibujarDegradado() del Esclavo recibe el "
              f"indicador 'vencida' ({_esc_tiene_vencida}) y muestra '>48h'; la del Maestro no "
              f"({_mae_tiene_vencida}) y pinta el numero que le den, divididos los minutos por "
              f"60. Hoy el llamante ya se lo da contrastado, asi que el numero no miente, pero "
              f"la pantalla NO es una defensa: si alguien vuelve a pasarle una medida cruda, "
              f"volveria a mostrar 'Sin sync: 0h00m' tras mes y medio sin sincronizar y nadie "
              f"lo notaria.")


    # ==========================================================================
    # BLOQUE 4 — SINCRONIZACION HORARIA POR RADIO (SFTY-23)
    # ==========================================================================
