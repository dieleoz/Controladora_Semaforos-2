# ===== banco/packs/maestro_04_sync_horaria.py =====
#
# SINCRONIZACION HORARIA POR RADIO (SFTY-23)
#
# La hora y la fecha viajan acopladas. Sin sincronizacion no hay Modo Degradado, y
# N-23 enseno que PONER el reloj no es lo mismo que SINCRONIZARLO: encolar el envio
# no es enviarlo si nadie mueve el coordinador.

from banco.modelos.maestro import *          # noqa: F401,F403
from banco.modelos.maestro import (          # los guiones bajos no
    _codigo, _fuente, _main, _ruta,          # los exporta import *
)

NOMBRE = "maestro_04_sync_horaria"
DESCRIPCION = "sincronizacion horaria por radio (SFTY-23)"


def correr(b, fw):
    # Bloque traido LITERAL del validador monolitico, solo reindentado. Reescribir
    # logica ya probada para renombrar las llamadas es como se cuelan los errores en
    # una migracion que se supone que no cambia comportamiento.
    verificar = b.verificar
    titulo = b.titulo


    def _hms(t_ms):
        s = (t_ms // 1000) % SEGUNDOS_DEL_DIA
        return s // 3600, (s % 3600) // 60, s % 60


    def enviar_trio(t_h, t_m, t_s, guarda=True):
        """Port de enviarTrioHora(). Las tres lecturas se toman en instantes
        EXPLICITOS y distintos: ahi es donde vive el riesgo del cruce de minuto, y
        modelarlas como simultaneas ocultaria justo lo que se quiere atacar.

        `guarda=False` es el MUTANTE: el mismo codigo sin la reelectura por s == 0,
        que es el fallo de una sola linea que SFTY-23 dice evitar."""
        h = _hms(t_h)[0]
        m = _hms(t_m)[1]
        s = _hms(t_s)[2]
        if guarda and s == 0:
            # Acabado de cruzar, no puede volver a cruzarse: se releen los tres.
            h, m, s = _hms(t_s)
        return h, m, s


    def error_contra(h, m, s, t_ref):
        """Segundos de diferencia entre el trio enviado y el instante de la ULTIMA
        lectura, que es el mas reciente que el Maestro conoce y por tanto el que
        deberia estar mandando."""
        enviado = h * 3600 + m * 60 + s
        real = (t_ref // 1000) % SEGUNDOS_DEL_DIA
        d = (enviado - real) % SEGUNDOS_DEL_DIA
        if d > SEGUNDOS_DEL_DIA // 2:
            d -= SEGUNDOS_DEL_DIA
        return d


    def barrer_cruces(guarda, retardos):
        """Barre TODAS las fronteras de segundo del dia colocando el cruce en los dos
        huecos posibles entre las tres lecturas. Devuelve (fallos, peor_error)."""
        fallos, peor = 0, 0
        for seg in range(SEGUNDOS_DEL_DIA):
            r = (seg + 1) * 1000                     # instante del cambio de segundo
            for d in retardos:
                # hueco 1: el cruce cae ENTRE la lectura de hora y la de minuto
                # hueco 2: el cruce cae ENTRE la de minuto y la de segundo
                for t_h, t_m in ((r - 1, r), (r - 2, r - 1)):
                    h, m, s = enviar_trio(t_h, t_m, r + d, guarda)
                    e = error_contra(h, m, s, r + d)
                    if e != 0:
                        fallos += 1
                        if abs(e) > abs(peor):
                            peor = e
        return fallos, peor


    # --- 4.1 -------------------------------------------------------------------
    # EL CRUCE DE MINUTO ENTRE LAS TRES LECTURAS. Barrido COMPLETO de las 86400
    # fronteras de segundo del dia, con el cruce cayendo en los DOS huecos posibles
    # entre las tres lecturas y con la ultima lectura llegando 0, 1 o 999 ms despues.
    # Si la guarda de s == 0 hace lo que dice, el trio enviado debe coincidir
    # EXACTAMENTE con el instante de la ultima lectura, sin una sola excepcion.
    fallos_41, peor_41 = barrer_cruces(guarda=True, retardos=(0, 1, 999))
    verificar(fallos_41 == 0,
              f"Barrido de las {SEGUNDOS_DEL_DIA} fronteras de segundo del dia x 2 huecos de "
              "lectura x 3 retardos (518.400 casos): el trio enviado coincide SIEMPRE con el "
              "instante real. La guarda de s == 0 cubre el cruce de minuto Y el de hora.",
              f"El trio enviado no corresponde al instante real en {fallos_41} casos, "
              f"con un error de hasta {peor_41} s")

    # --- 4.2 -------------------------------------------------------------------
    # La prueba anterior tiene que poder fallar, o su PASS no significaria nada. El
    # mutante sin la reelectura debe romperse en las fronteras de minuto (60 s de
    # error) y de hora (3600 s), que son exactamente las que la guarda existe para
    # cubrir.
    fallos_42, peor_42 = barrer_cruces(guarda=False, retardos=(0, 1, 999))
    verificar(fallos_42 > 1000 and abs(peor_42) >= 3600,
              f"Control del modelo: el mutante SIN la guarda de s == 0 falla en {fallos_42} "
              f"de los mismos casos, con errores de hasta {peor_42} s (una hora entera en las "
              "fronteras de hora). La prueba 4.1 puede fallar, luego su PASS significa algo.",
              f"El mutante sin guarda apenas falla ({fallos_42} casos, peor {peor_42} s): "
              "la prueba 4.1 no esta comprobando la guarda")

    # --- 4.3 -------------------------------------------------------------------
    # LIMITE DE LA GUARDA. La reelectura solo se dispara con s == 0, asi que solo
    # protege mientras las tres lecturas quepan DENTRO DEL MISMO SEGUNDO en el que
    # ocurre el cruce. Se busca el punto exacto en el que deja de proteger, para
    # tenerlo documentado y no descubrirlo en campo.
    primer_retardo_malo = None
    for d in (900, 950, 999, 1000, 1100, 2000):
        f, p = barrer_cruces(guarda=True, retardos=(d,))
        if f and primer_retardo_malo is None:
            primer_retardo_malo = (d, f, p)
    verificar(primer_retardo_malo is None or primer_retardo_malo[0] >= 1000,
              "La guarda protege mientras las tres lecturas del reloj se completen en menos "
              "de 1 s desde el cruce, que es holgadisimo frente a tres accesos a registro "
              "seguidos" +
              (f". A partir de {primer_retardo_malo[0]} ms de separacion deja de proteger "
               f"({primer_retardo_malo[1]} casos, hasta {primer_retardo_malo[2]} s de error): "
               "residual conocido, no alcanzable con el bucle real"
               if primer_retardo_malo else "") + ".",
              f"La guarda deja de proteger ya con {primer_retardo_malo} de separacion entre "
              "lecturas: el margen es demasiado estrecho")

    # --- 4.4 -------------------------------------------------------------------
    # EL RECALCULO EN LOS REINTENTOS. La regla critica de SFTY-23: el reloj se lee
    # DENTRO de la funcion de envio, que es la misma del primer envio y de cada
    # retransmision. Se simula un intercambio con la primera trama perdida.
    t0 = 10 * 3600 * 1000                       # 10:00:00
    trio_1 = enviar_trio(t0, t0, t0)
    trio_2 = enviar_trio(t0 + TIMEOUT_ACK_MS, t0 + TIMEOUT_ACK_MS,
                         t0 + TIMEOUT_ACK_MS)   # retransmision tras el timeout
    avance = ((trio_2[0] * 3600 + trio_2[1] * 60 + trio_2[2]) -
              (trio_1[0] * 3600 + trio_1[1] * 60 + trio_1[2]))
    verificar(avance == TIMEOUT_ACK_MS // 1000,
              f"La retransmision vuelve a leer el reloj: entre el primer envio y el reintento "
              f"de {TIMEOUT_ACK_MS} ms el trio avanza los {avance} s que de verdad pasaron. "
              "No hay ninguna variable donde guardar la hora vieja.",
              f"La retransmision envia una hora atrasada {TIMEOUT_ACK_MS//1000 - avance} s: "
              "el error entraria justo por el mecanismo que da robustez")

    # --- 4.5 -------------------------------------------------------------------
    # Lo mismo para la peticion de desfase: si se reenviara el segundo caducado, la
    # medida inventaria una diferencia y quedaria anotada en el acta como cierta.
    def enviar_delta(reloj_ms):
        return ((reloj_ms // 1000) % SEGUNDOS_DEL_DIA) % 60


    d1 = enviar_delta(t0)
    d2 = enviar_delta(t0 + TIMEOUT_ACK_MS)
    verificar((d2 - d1) % 60 == (TIMEOUT_ACK_MS // 1000) % 60,
              "La peticion de desfase tambien relee el segundo en cada reintento: no se mide "
              "el desfase contra un segundo caducado.",
              "El reintento de la peticion de desfase reutiliza el segundo viejo")

    # --- 4.6 -------------------------------------------------------------------
    # EL PRESUPUESTO DE TIEMPO. Durante el intercambio se suprime el latido, asi que
    # el Esclavo puede pasar ese rato sin recibir nada. La cuenta que el firmware
    # documenta: latido (3 s) + intercambio completo debe quedar POR DEBAJO del
    # fallback de orfandad de 12 s. Se rehace con los numeros de hoy.
    peor_silencio = LATIDO_MS + SYNC_MAX_INTENTOS * TIMEOUT_ACK_MS
    verificar(peor_silencio < ORFANDAD_MS,
              f"Presupuesto de canal: {LATIDO_MS} ms de cadencia de latido + "
              f"{SYNC_MAX_INTENTOS} intentos x {TIMEOUT_ACK_MS} ms = {peor_silencio} ms, "
              f"por debajo de los {ORFANDAD_MS} ms de orfandad. Una sincronizacion fallida "
              "no puede convertirse en un ambar espurio.",
              f"Una sincronizacion fallida deja al Esclavo {peor_silencio} ms sin recibir, "
              f"por encima de los {ORFANDAD_MS} ms de orfandad: se provocaria un AMBAR "
              "espurio y una salida a campo cada vez que se pierda una trama de hora")

    # N-71: ESTA COMPROBACION EXIGIA "EL MAXIMO QUE CABE", Y SE INVIRTIO.
    #
    # Tenia sentido mientras el techo de orfandad eran 12 s: alli el techo era la
    # restriccion que mandaba, apretaba a 2 intentos, y comprobar que se usaban los 2
    # era comprobar que nadie habia elegido un numero a ojo. Al subir el techo a 25 s
    # esa restriccion deja de morder -caben 6- y "usa el maximo" pasa a ser un consejo
    # MALO: cada intento fallido ocupa el canal y se lo roba al ciclo y al latido.
    #
    # Lo que de verdad acota hoy no es que quepa, sino cuanto del techo se come: durante
    # el intercambio se suprime el latido (SFTY-13), asi que el Esclavo pasa ese rato a
    # ciegas. Se exige que no consuma mas del 60 % del techo, dejando el 40 % restante
    # para que el ciclo note la caida y reaccione. Con los numeros de hoy eso permite
    # exactamente 3 intentos y rechaza 4, asi que el numero sigue saliendo de una cuenta.
    TOPE_OCUPACION = 0.60
    ocupa = LATIDO_MS + SYNC_MAX_INTENTOS * TIMEOUT_ACK_MS
    ocupa_uno_mas = LATIDO_MS + (SYNC_MAX_INTENTOS + 1) * TIMEOUT_ACK_MS
    verificar(ocupa <= ORFANDAD_MS * TOPE_OCUPACION and
              ocupa_uno_mas > ORFANDAD_MS * TOPE_OCUPACION,
              f"Y {SYNC_MAX_INTENTOS} es el numero que cabe en el {TOPE_OCUPACION:.0%} del "
              f"techo ({ocupa} ms de {int(ORFANDAD_MS*TOPE_OCUPACION)} ms): con "
              f"{SYNC_MAX_INTENTOS+1} serian {ocupa_uno_mas} ms y el intercambio se comeria "
              "el margen que el ciclo necesita para notar la caida. Sale de la cuenta.",
              f"El numero de intentos no respeta el tope de ocupacion del canal: con "
              f"{SYNC_MAX_INTENTOS} el intercambio ocupa {ocupa} ms de un techo de "
              f"{ORFANDAD_MS} ms. O se pasa del {TOPE_OCUPACION:.0%} -y deja al ciclo sin "
              f"margen para notar la caida- o se queda corto pudiendo reintentar mas")

    # --- 4.7 -------------------------------------------------------------------
    # El backoff tras agotar los intentos tiene que ser mayor que el intercambio, o
    # el reintento sin freno robaria el canal al ciclo y al latido.
    verificar(BACKOFF_SYNC_MS > SYNC_MAX_INTENTOS * TIMEOUT_ACK_MS,
              f"El backoff de {BACKOFF_SYNC_MS} ms es mayor que el intercambio completo "
              f"({SYNC_MAX_INTENTOS * TIMEOUT_ACK_MS} ms): tras fallar se espera de verdad.",
              f"El backoff ({BACKOFF_SYNC_MS} ms) no cubre el intercambio: se reintentaria "
              "casi sin freno")

    verificar(INTERVALO_SYNC_MS * 2 == VIGENCIA_DESFASE_MS == SYNC_FRESCA_MS,
              f"Las tres ventanas encajan: resincronizacion cada {INTERVALO_SYNC_MS/3600000:.0f} h, "
              f"y tanto la vigencia del desfase como la frescura de la puerta valen el doble "
              f"({SYNC_FRESCA_MS/3600000:.0f} h). Un solo intercambio perdido no cierra la "
              "puerta; dos seguidos si.",
              f"Las ventanas ya no encajan: intervalo={INTERVALO_SYNC_MS} "
              f"vigencia={VIGENCIA_DESFASE_MS} frescura={SYNC_FRESCA_MS}. Un solo intercambio "
              "perdido puede cerrar la puerta del Degradado, o dos perdidos pueden no cerrarla")

    # --- 4.8 -------------------------------------------------------------------
    # LA RED DE RESPALDO DEL RELOJ. vigilarCambioDeHora() compara el RTC contra lo que
    # deberia marcar segun millis(), con tolerancia de +-3 s. Se barre un ajuste
    # manual de -3600 a +3600 s y se exige que detecte todo lo que sea un ajuste real
    # y nada de lo que sea deriva de la base de tiempo.
    def vigilar(seg_visto, transcurrido_ms, seg_actual):
        esperado = (seg_visto + transcurrido_ms // 1000) % SEGUNDOS_DEL_DIA
        dif = (seg_actual + SEGUNDOS_DEL_DIA - esperado) % SEGUNDOS_DEL_DIA
        return dif > 3 and dif < (SEGUNDOS_DEL_DIA - 3)


    perdidos, falsos = [], []
    for ajuste in range(-3600, 3601):
        visto = 12 * 3600
        actual = (visto + VIGILANCIA_RELOJ_MS // 1000 + ajuste) % SEGUNDOS_DEL_DIA
        detecta = vigilar(visto, VIGILANCIA_RELOJ_MS, actual)
        if abs(ajuste) > 3 and not detecta:
            perdidos.append(ajuste)
        if abs(ajuste) <= 3 and detecta:
            falsos.append(ajuste)
    verificar(not perdidos and not falsos,
              "Barrido de un ajuste manual de -1 h a +1 h: la red de respaldo del reloj "
              "detecta todo lo que supera los +-3 s de tolerancia y nada de lo que no. "
              "Nadie corrige un reloj tres segundos a mano.",
              f"La vigilancia del reloj pierde ajustes reales {perdidos[:5]} o inventa "
              f"cambios {falsos[:5]}")

    # La deriva de la propia base de tiempo en el periodo de vigilancia tiene que
    # caber en la tolerancia, o la red de respaldo empujaria la hora sin motivo.
    deriva_vigilancia = DERIVA_PEOR_S_DIA * (VIGILANCIA_RELOJ_MS / 86400000.0)
    verificar(deriva_vigilancia < 3,
              f"En los {VIGILANCIA_RELOJ_MS/60000:.0f} min del periodo de vigilancia se "
              f"acumulan {deriva_vigilancia:.3f} s de deriva, muy por debajo de los 3 s de "
              "tolerancia: la red de respaldo no dispara sola.",
              f"La deriva en el periodo de vigilancia ({deriva_vigilancia:.2f} s) roza la "
              "tolerancia de 3 s: la red de respaldo empujaria la hora sin que nadie la toque")


    # ==========================================================================
    # BLOQUE 5 — CICLO DEGRADADO Y CIERRE
    # ==========================================================================
