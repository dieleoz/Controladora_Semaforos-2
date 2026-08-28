# ===== banco/packs/costura_05_limite_48h.py =====
#
# LIMITE DE 48 H: SEPARACION ENTRE LAS DOS CAIDAS Y QUE PASA EN EL HUECO
#
# Las dos puntas cuentan las 48 h por caminos distintos -el Maestro contrasta con
# la pila, el Esclavo usa millis() con latch-, asi que no se rinden a la vez. En el
# hueco, una puede estar en ambar mientras la otra sigue dando verde por reloj.

from banco.modelos.costura import *          # noqa: F401,F403

NOMBRE = "costura_05_limite_48h"
DESCRIPCION = "las dos puntas no se rinden en el mismo instante"


def correr(b, fw):
    # Bloque traido LITERAL, solo reindentado.
    verificar = b.verificar

    def hallazgo(reproducido, titulo, detalle, consecuencia):
        """El hallazgo de costura lleva CUATRO argumentos y SI cuenta como
        comprobacion: aqui la comprobacion ES reproducir el desajuste, asi que si el
        modelo no lo reprodujera seria el modelo el que esta mal. En el validador del
        Esclavo la misma palabra significa otra cosa y NO cuenta -alli acompana a una
        propiedad() que ya cuenta por su cuenta-. Dos cosas distintas con el mismo
        nombre: por eso 37/41 y 30/31 no se podian sumar."""
        b.hallazgo(reproducido, titulo, [detalle, f"EN LA CALLE: {consecuencia}"])

    b.titulo("LIMITE DE 48 H: SEPARACION ENTRE LAS DOS CAIDAS Y QUE PASA EN EL HUECO")


    marca_esclavo_ok = bool(re.search(r"reloj_ajustar\(bufHora, bufMinuto, pkt\.param, bufDia\);\s*\n(?:.*\n)*?\s*degradado_registrarSync\(\);",
                                      T_E_MAIN_C))
    marca_maestro_ok = bool(re.search(r"pkt->command\s*==\s*CMD_ACK_HORA\)\s*\{\s*\n\s*tUltimaSyncOk\s*=\s*millis\(\);",
                                      T_M_COORD_C))
    verificar(marca_esclavo_ok and marca_maestro_ok,
              "cada punta marca su sincronizacion en el unico instante en que puede afirmarla: "
              "el Esclavo al aplicar la hora, el Maestro al recibir el ACK",
              "las marcas de sincronizacion no se ponen donde el diseno dice")

    verificar(M_LIMITE_DURO_MS == E_LIMITE_MS,
              f"las dos puntas usan el MISMO limite duro ({M_LIMITE_DURO_MS//3600000} h), leido de "
              "cada fuente por separado",
              f"limites distintos: Maestro {M_LIMITE_DURO_MS} ms, Esclavo {E_LIMITE_MS} ms")

    # Tiempo de aire de una trama: RF_BURST_COPIES copias de 4 bytes por el cable al
    # modulo, mas los 2 ms de conmutacion del MAX485 y el bit de parada.
    BITS_POR_BYTE = 10  # 8N1
    aire_ms = (RF_BURST_COPIES * 4 * BITS_POR_BYTE * 1000.0) / BAUD_CABLE + 2 + 1.2
    HUECO_NOMINAL_MS = RETARDO_RESPUESTA_MS + aire_ms

    print(f"    Tiempo de una trama por el cable ({RF_BURST_COPIES} copias @ {BAUD_CABLE} bd): "
          f"{aire_ms:.1f} ms")
    print(f"    HUECO NOMINAL entre las dos marcas = cortesia {RETARDO_RESPUESTA_MS} ms + aire de "
          f"vuelta = {HUECO_NOMINAL_MS:.0f} ms")


    def timeline(t_ms, disparo_m_ms, disparo_e_ms, seg_dia_en_cero):
        """Luz de las dos puntas alrededor de las dos caidas.

        Maestro (irAAmbar): forzarRojo() al instante, y el ambar arranca
          M_ROJO_ANTES_AMBAR_MS despues.
        Esclavo (iniciarSalida(rendicion)): forzarRojo() al instante, DEG_SALIENDO
          durante un despeje completo, y despues DEG_RENDIDO con ambar.
        """
        s = (seg_dia_en_cero + int(t_ms // 1000)) % SEGUNDOS_DEL_DIA
        if t_ms < disparo_m_ms:
            lm = luz_maestro(s, DEG_VERDE_SEG, DEG_DESPEJE_SEG)
        elif t_ms < disparo_m_ms + M_ROJO_ANTES_AMBAR_MS:
            lm = ROJO
        else:
            lm = AMBAR_INTERMITENTE
        if t_ms < disparo_e_ms:
            le = luz_esclavo(s, DEG_VERDE_SEG, DEG_DESPEJE_SEG, E_AMARILLO_MS // 1000)
        elif t_ms < disparo_e_ms + DEG_DESPEJE_SEG * 1000:
            le = ROJO
        else:
            le = AMBAR_INTERMITENTE
        return lm, le


    def peor_caso(hueco_ms, esclavo_primero=True, paso_ms=100, margen_ms=None):
        """Barre TODAS las posiciones del ciclo en las que puede caer el disparo.

        Devuelve (segundos de verde-contra-ambar, segundos de verde simultaneo).
        El disparo puede caer en cualquier punto del ciclo, asi que se recorre el
        ciclo entero: probar una sola hora no diria nada.
        """
        ciclo = 2 * (DEG_VERDE_SEG + DEG_DESPEJE_SEG)
        if margen_ms is None:
            margen_ms = hueco_ms + (DEG_DESPEJE_SEG + 10) * 1000
        d_e = 0 if esclavo_primero else hueco_ms
        d_m = hueco_ms if esclavo_primero else 0
        peor_ambar = 0
        peor_doble = 0
        for offset in range(ciclo):
            ambar_vs_verde = 0
            doble_verde = 0
            t = -2000
            while t < margen_ms:
                lm, le = timeline(t, d_m, d_e, offset)
                if lm == VERDE and le == VERDE:
                    doble_verde += paso_ms
                if (lm == VERDE and le == AMBAR_INTERMITENTE) or (le == VERDE and lm == AMBAR_INTERMITENTE):
                    ambar_vs_verde += paso_ms
                t += paso_ms
            peor_ambar = max(peor_ambar, ambar_vs_verde)
            peor_doble = max(peor_doble, doble_verde)
        return peor_ambar / 1000.0, peor_doble / 1000.0


    # --- 5a. Caso nominal: enlace sano, ninguna punta reiniciada --------------
    amb_nom, doble_nom = peor_caso(int(round(HUECO_NOMINAL_MS)), esclavo_primero=True)
    verificar(doble_nom == 0,
              f"caso NOMINAL (hueco de {HUECO_NOMINAL_MS:.0f} ms): barriendo el ciclo entero, "
              "NUNCA hay verde simultaneo al caer el limite",
              f"PELIGRO: {doble_nom} s de verde simultaneo al vencer el limite")

    verificar(amb_nom == 0,
              f"caso NOMINAL: tampoco hay un solo instante de VERDE en una punta contra AMBAR en la "
              f"otra. Las dos caen a todo-rojo con {HUECO_NOMINAL_MS:.0f} ms de separacion, y el "
              f"todo-rojo del Maestro ({M_ROJO_ANTES_AMBAR_MS/1000:.0f} s) queda ENTERO dentro del "
              f"del Esclavo ({DEG_DESPEJE_SEG} s): los todo-rojos SE SOLAPAN y el hueco es inofensivo",
              f"PELIGRO: {amb_nom} s de verde contra ambar en el caso nominal")

    # Que este barrido sepa reconocer el caso malo: se le da un hueco de una hora.
    amb_malo, _ = peor_caso(3600 * 1000, esclavo_primero=False, paso_ms=1000)
    verificar(amb_malo > 0,
              f"control negativo: con un hueco de 1 h el mismo barrido encuentra {amb_malo:.0f} s de "
              "verde contra ambar. La prueba SI sabe distinguir un hueco inofensivo de uno peligroso",
              "el barrido no detecta el peligro ni con una hora de hueco: no vale como prueba")

    # Nota cuantificada de la asimetria que si existe en el caso nominal, aunque no
    # sea peligrosa: el Maestro llega al ambar 28 s antes que el Esclavo.
    desfase_ambar_s = (DEG_DESPEJE_SEG * 1000 - M_ROJO_ANTES_AMBAR_MS + HUECO_NOMINAL_MS) / 1000.0
    print(f"    Asimetria medida y NO peligrosa: el Maestro pasa a ambar "
          f"{desfase_ambar_s:.1f} s antes que el Esclavo; durante ese rato el Esclavo esta en "
          f"todo-rojo, nunca en verde.")

    # --- 5b. El hueco de verdad: las dos marcas pueden separarse HORAS ---------
    # La puerta de entrada NO es la misma en las dos puntas:
    #   Maestro: exige sincronizacion de menos de SYNC_FRESCA_MS (2 h).
    #   Esclavo: exige que la haya habido y que no pase de 48 h. Nada mas.
    # Si el camino de VUELTA falla y el de IDA no -que es exactamente el sintoma de
    # campo del 31/07/2026 con repetidor: "C<-Esclavo = 1 byte en dos minutos
    # mientras la ida fluia"-, el Esclavo refresca su marca en cada reintento y el
    # Maestro no refresca la suya nunca.
    e_gate_sin_frescura = not re.search(r"SYNC_FRESCA|msDesdeSync\(\)\s*>=", T_E_DEG_C)
    m_gate_frescura = bool(re.search(r"desdeSync\s*>=\s*SYNC_FRESCA_MS\)\s*return\s+MDG_SYNC_VIEJA", T_M_DEG_C))
    reintenta_sin_limpiar = bool(re.search(r"if\s*\(estadoSync\s*==\s*SY_ESPERA_RESP_DELTA\)\s*pendDelta\s*=\s*false;",
                                           T_M_COORD_C)) and not re.search(r"pendHora\s*=\s*false;\s*\n\s*estadoSync\s*=\s*SY_IDLE;\s*\n\s*syncEnBackoff",
                                                                           T_M_COORD_C)
    hueco_max_ms = M_SYNC_FRESCA_MS
    amb_2h, doble_2h = peor_caso(hueco_max_ms, esclavo_primero=False, paso_ms=1000)

    hallazgo(m_gate_frescura and e_gate_sin_frescura and reintenta_sin_limpiar and amb_2h > 0,
             "las dos puertas de entrada no piden la misma frescura: el hueco de las 48 h "
             f"puede nacer ya con {M_SYNC_FRESCA_MS/3600000:.0f} h de separacion",
             f"el Maestro exige sincronizacion de menos de {M_SYNC_FRESCA_MS/3600000:.0f} h "
             f"(MDG_SYNC_VIEJA); el Esclavo solo exige que la haya habido y que no pase de "
             f"{E_LIMITE_MS/3600000:.0f} h. Si el camino de VUELTA falla y el de IDA no -el sintoma "
             f"de campo del repetidor-, cada reintento del Maestro (cada {BACKOFF_SYNC_MS//1000} s, "
             f"porque pendHora NO se limpia al agotar los intentos) hace que el Esclavo aplique la "
             f"hora y refresque SU marca, mientras el Maestro no refresca la suya. Al entrar en "
             f"Degradado la separacion puede ser de casi {M_SYNC_FRESCA_MS/3600000:.0f} h, y ahi se "
             f"congela: en Degradado el Maestro CALLA y ya nadie vuelve a sincronizar.",
             f"el Maestro cae a ambar hasta {M_SYNC_FRESCA_MS/3600000:.0f} h antes que el Esclavo. "
             f"Barriendo el ciclo entero salen hasta {amb_2h:.0f} s -{amb_2h/60:.0f} min- de VERDE en "
             "el Esclavo contra AMBAR en el Maestro: el conductor del lado ambar negocia el paso "
             "mientras el del lado verde cruza confiado. Es el riesgo residual n.2 de SFTY-21, "
             "reproducido sin que nadie toque nada")

    # --- 5c. El reinicio redondea a HORAS ENTERAS -----------------------------
    # Tras un corte, la antiguedad no se conoce en milisegundos: se reconstruye de la
    # pila y respaldo_horasDesdeSync() devuelve HORAS ENTERAS, truncando hacia abajo.
    # La punta que se reinicia se cree mas joven de lo que es, hasta 59 min 59 s.
    # N-49 T1 (05/08): respaldo_horasDesdeSync() dejo de tener una variable 'total' -
    # ahora resta segundosRtcAhora - guardado en la misma linea del return-, asi que
    # el patron viejo dejo de casar y esta prueba caia a "el modelo no reproduce" sin
    # que el truncado a horas hubiera cambiado. Reproducido: el truncado SIGUE ahi
    # (T_M_DEG_C usa horas*3600000UL, T_E_DEG_C siembra tUltimaSync igual). Se
    # actualiza el patron al texto real; el hallazgo sigue siendo el mismo.
    trunca_a_horas = bool(re.search(r"return\s*\(segundosRtcAhora\s*-\s*guardado\)\s*/\s*3600UL;", T_M_RESP_C))
    m_usa_horas = bool(re.search(r"return\s*\(unsigned long\)horas\s*\*\s*3600000UL;", T_M_DEG_C))
    e_usa_horas = bool(re.search(r"tUltimaSync\s*=\s*millis\(\)\s*-\s*horas\s*\*\s*3600000UL;", T_E_DEG_C))
    # Se miden LAS DOS DIRECCIONES: se reinicia el Maestro o se reinicia el Esclavo.
    # El que sobrevive es el que da el verde de mas, asi que el peligro existe en los
    # dos sentidos y hay que quedarse con el peor.
    amb_1h_m, doble_1h_m = peor_caso(3599 * 1000, esclavo_primero=False, paso_ms=1000)
    amb_1h_e, doble_1h_e = peor_caso(3599 * 1000, esclavo_primero=True, paso_ms=1000)
    amb_1h = max(amb_1h_m, amb_1h_e)

    hallazgo(trunca_a_horas and m_usa_horas and e_usa_horas and amb_1h > 0,
             "un reinicio en UNA sola punta regala a esa punta hasta 59 min 59 s de credito",
             "respaldo_horasDesdeSync() devuelve HORAS ENTERAS truncadas (total / 3600) y las dos "
             "puntas la usan asi: el Maestro con horas*3600000 en msDesdeSyncEfectivo(), el Esclavo "
             "sembrando tUltimaSync = millis() - horas*3600000. La punta que NO se reinicio conserva "
             "la antiguedad exacta en milisegundos. El truncado es siempre a favor de quien "
             "arranca, nunca en contra.",
             f"la punta reiniciada aguanta hasta 3599 s mas que la otra. Barriendo el ciclo entero "
             f"en las DOS direcciones: {amb_1h_e:.0f} s de verde del Maestro contra ambar del "
             f"Esclavo si el que se reinicia es el Maestro, y {amb_1h_m:.0f} s de verde del Esclavo "
             f"contra ambar del Maestro si el que se reinicia es el Esclavo. Peor caso "
             f"{amb_1h:.0f} s ({amb_1h/60:.0f} min). La direccion cambia, el peligro no")

    # --- 5d. Un reintento perdido NO separa las marcas ------------------------
    # Vale la pena decir lo que SI esta bien: si se pierde el ACK y el Maestro
    # reenvia la terna COMPLETA, el Esclavo la aplica otra vez y su marca se vuelve
    # a poner. Las dos marcas siguen atadas por el hueco nominal, no por el timeout.
    reenvia_terna_completa = bool(re.search(r"if\s*\(estadoSync\s*==\s*SY_ESPERA_ACK_HORA\)\s*\{\s*\n\s*enviarHoraCompleta\(\);",
                                            T_M_COORD_C))
    reaplica = bool(re.search(r"if\s*\(tieneDia\s*&&\s*tieneHora\s*&&\s*tieneMinuto\s*&&\s*pkt\.param\s*<=\s*59\)", T_E_MAIN_C))
    verificar(reenvia_terna_completa and reaplica,
              f"si el ACK se pierde, el Maestro reenvia la TERNA COMPLETA y el Esclavo la vuelve a "
              f"aplicar: la marca del Esclavo se refresca con la del Maestro y el hueco NO crece "
              f"con los {TIMEOUT_ACK_MS} ms del reintento",
              "un reintento deja las dos marcas separadas por el timeout completo")

    # ===========================================================================
    # 6. LA REANUDACION TRAS UN CORTE
