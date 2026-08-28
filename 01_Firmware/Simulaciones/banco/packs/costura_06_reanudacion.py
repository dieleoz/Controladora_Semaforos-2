# ===== banco/packs/costura_06_reanudacion.py =====
#
# REANUDACION TRAS UN CORTE: LA PUNTA QUE SE REINICIA VUELVE EN FASE
#
# Tras un corte, la punta que arranca reconstruye su fase desde la pila. Si volviera
# desfasada respecto a la que nunca se apago, las dos estarian en el mismo ciclo
# creyendo cosas distintas.

from banco.modelos.costura import *          # noqa: F401,F403

# EJERCE SFTY-21: que la punta que se reinicia vuelva EN FASE con la otra.

NOMBRE = "costura_06_reanudacion"
DESCRIPCION = "la punta que se reinicia, ¿vuelve en fase?"


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

    b.titulo("REANUDACION TRAS UN CORTE: LA PUNTA QUE SE REINICIA VUELVE EN FASE")


    # La punta que reanuda entra por DEG_ENTRADA_ROJO / DEG_ENTRANDO, que exige DOS
    # condiciones: un despeje completo Y que la fase no sea la de SU propio verde.
    # Se barre el instante de arranque por TODO el dia: si hubiera una sola posicion
    # en la que la punta reanudada se enganchara a mitad de su verde, ahi estaria el
    # fallo.
    m_dos_condiciones = bool(re.search(
        r"millis\(\)\s*-\s*tEstado\s*>=\s*ROJO_TRANSICION_MS\s*&&\s*fase\s*!=\s*FD_VERDE_MAESTRO", T_M_DEG_C))
    e_dos_condiciones = bool(re.search(
        r"\(ahora\s*-\s*tCambioEstado\)\s*>=\s*rojoObligatorioMs\(\)\s*&&\s*\n?\s*calcularFase\(\)\s*!=\s*FD_VERDE_ESCLAVO",
        T_E_DEG_C))
    verificar(m_dos_condiciones and e_dos_condiciones,
              "las dos puntas exigen lo MISMO para abandonar el todo-rojo de entrada: un despeje "
              "completo Y que la fase no sea la de su propio verde. El primer verde tras el corte "
              "es siempre un verde entero contado desde su principio",
              "las condiciones de salida del todo-rojo de entrada NO son simetricas entre puntas")

    # Barrido completo: arranque en cada segundo del dia, para las dos direcciones.
    fallos_reanudacion = []
    for quien in ("MAESTRO", "ESCLAVO"):
        for t0 in range(SEGUNDOS_DEL_DIA):
            # Todo-rojo obligatorio y despues la primera posicion en la que la punta
            # reanudada acepta ceder el rojo.
            t = t0 + DEG_DESPEJE_SEG
            propio = FD_VERDE_MAESTRO if quien == "MAESTRO" else FD_VERDE_ESCLAVO
            vueltas = 0
            while fase(t % SEGUNDOS_DEL_DIA, DEG_VERDE_SEG, DEG_DESPEJE_SEG) == propio:
                t += 1
                vueltas += 1
                if vueltas > 2 * (DEG_VERDE_SEG + DEG_DESPEJE_SEG) + DEG_DESPEJE_SEG + 2:
                    break
            # Desde ese instante la punta reanudada sigue la fase. La otra nunca se
            # reinicio y tambien sigue la fase. Como las dos leen la MISMA funcion,
            # basta comprobar que no hay solape en el ciclo siguiente.
            for d in range(2 * (DEG_VERDE_SEG + DEG_DESPEJE_SEG) + 1):
                s = (t + d) % SEGUNDOS_DEL_DIA
                if luz_maestro(s, DEG_VERDE_SEG, DEG_DESPEJE_SEG) == VERDE and \
                   luz_esclavo(s, DEG_VERDE_SEG, DEG_DESPEJE_SEG, E_AMARILLO_MS // 1000) == VERDE:
                    fallos_reanudacion.append((quien, t0, s))
                    break
            if fallos_reanudacion:
                break
        if fallos_reanudacion:
            break

    verificar(not fallos_reanudacion,
              f"barrido de los {SEGUNDOS_DEL_DIA} instantes de arranque posibles, para las dos "
              "direcciones: la punta que reanuda queda SIEMPRE en fase con la que no se reinicio. "
              "El ciclo se ancla a la hora de pared, no a un contador propio",
              f"la reanudacion deja las puntas desfasadas: {fallos_reanudacion[:3]}")

    # --- 6b. Una reanuda y la otra NO puede ----------------------------------
    # El caso feo. Las condiciones de reanudacion son las MISMAS en las dos puntas
    # -eso esta bien-, pero se evaluan sobre datos que pueden diferir. Y el
    # calendario es uno de ellos.
    m_condiciones = bool(re.search(r"reloj_enHora\(\)\s*&&\s*respaldo_hayCiclo\(\)\s*&&\s*\n?\s*horas\s*!=\s*RESPALDO_SYNC_CADUCADA\s*&&\s*horas\s*<\s*LIMITE_DURO_H",
                                   T_M_DEG_C))
    e_condiciones = bool(re.search(r"sigueVigente\s*=\s*\(horas\s*!=\s*RESPALDO_SYNC_CADUCADA\)\s*&&\s*\(horas\s*<\s*LIMITE_SIN_SYNC_H\)",
                                   T_E_DEG_C))
    verificar(m_condiciones and e_condiciones,
              "las cuatro condiciones de reanudacion son las mismas en las dos puntas -indicador, "
              "reloj en hora, ciclo en la pila, antiguedad fechable y por debajo del limite-, y las "
              "dos comprueban el centinela CADUCADA aparte del numero",
              "las condiciones de reanudacion difieren entre puntas")

    # RETIRADO el 05/08 (N-49 T1) — SE INVIERTE, no se borra sin dejar rastro.
    #
    # Antes se comprobaba aqui que cada calendario se siembra por separado
    # (rtc.setDay(1)/setMonth(1)) y que la fecha no viaja por el protocolo -las dos
    # cosas siguen siendo ciertas, se pueden ver en T_M_RELOJ_C/T_E_RELOJ_C-, pero ya
    # no son la premisa de ningun hallazgo: lo que importaba de ellas era que
    # alimentaban una reanudacion atada al dia del mes, y eso es lo que T1 quito.
    #
    # Este hallazgo documentaba que, como cada calendario es independiente, la
    # reanudacion podia depender de en que dia del mes cayera cada punta:
    # respaldo_horasDesdeSync() declaraba CADUCADA en cuanto el DIA BAJABA -el cruce
    # de fin de mes-. T1 no acoto el caso: le quito el mecanismo por el que
    # importaba. La marca ya no es dia+segundo, es reloj_contadorSegundos() -un
    # contador monotono del RTC-, y la comparacion siempre resta DOS LECTURAS DE LA
    # MISMA UNIDAD (nunca la de una punta contra el calendario de la otra). Que los
    # calendarios (rtc.setDay(1)) sigan siendo independientes ya no puede afectar a
    # la reanudacion, porque la reanudacion no vuelve a mirar el calendario.
    #
    # Se invierte en un verificar() que exige la firma nueva: si algun dia
    # respaldo_horasDesdeSync() vuelve a tomar una fecha de calendario, esto tiene
    # que volver a fallar.
    firma_contador_unico = bool(re.search(
        r"uint32_t\s+respaldo_horasDesdeSync\s*\(\s*uint32_t\s+segundosRtcAhora\s*\)", T_M_RESP_C))
    verificar(firma_contador_unico,
              "respaldo_horasDesdeSync() recibe UN SOLO contador de RTC, no un dia y un segundo: "
              "la reanudacion ya no puede depender de en que calendario -propio de cada unidad- "
              "caiga cada punta, aunque los calendarios (rtc.setDay(1)) sigan siendo independientes.",
              "respaldo_horasDesdeSync() ha vuelto a tomar una fecha de calendario: el riesgo que "
              "N-49 T1 cerro -reanudacion atada a calendarios independientes entre puntas- podria "
              "estar de vuelta")

    # Lo que SI rescata el caso, y conviene tenerlo escrito: si el radio vive, la
    # punta que no reanuda vuelve al menu, el coordinador emite CMD_GO_RED cada 3 s y
    # eso saca al Esclavo del Degradado por la via ordenada.
    menu_emite_rojo = bool(re.search(r"estadoC\s*==\s*C_MENU_IDLE\s*\|\|\s*estadoC\s*==\s*C_FALLO\)\s*\{\s*\n\s*protocolo_enviarPaquete\(CMD_GO_RED\)",
                                     T_M_COORD_C))
    gobierno_saca = bool(re.search(r"degradado_gobiernaLuz\(\)\s*&&\s*\n?\s*\(pkt\.command\s*==\s*CMD_PING\s*\|\|\s*pkt\.command\s*==\s*CMD_GO_RED\s*\|\|\s*pkt\.command\s*==\s*CMD_GO_GREEN\)\)\s*\{\s*\n\s*degradado_salir\(\)",
                                   T_E_MAIN_C))
    verificar(menu_emite_rojo and gobierno_saca,
              "con el radio vivo, la asimetria se cura sola: la punta que arranca en el menu emite "
              "CMD_GO_RED cada 3 s y el Esclavo sale del Degradado por todo-rojo. Solo las tramas de "
              "GOBIERNO lo sacan; las de servicio no",
              "una punta en el menu no saca a la otra del Degradado ni con el radio vivo")

    # --- 6c. La configuracion del ciclo SE REENCOLA al volver el enlace --------
    # N-49 T1 (§8.quater): modo_degradado_publicarConfig() solo se llama en setup(),
    # pero al recuperarse la conexion coordinador_reiniciarConexion() pone pendHora = true,
    # pendConfig = true y configConfirmada = false (coordinador.cpp:498-500).
    # Ademas, modo_degradado_evaluarEntrada() exige MDG_SIN_CONFIG (modo_degradado.cpp).
    # Se invierte el hallazgo en un verificar() que exige estas tres garantias.
    reencola_hora = bool(re.search(r"pendHora\s*=\s*true;", T_M_COORD_C))
    reencola_config = bool(re.search(r"pendConfig\s*=\s*true;\s*\n\s*configConfirmada\s*=\s*false;", T_M_COORD_C))
    gate_maestro_config = bool(re.search(r"MDG_SIN_CONFIG", T_M_DEG_C))

    verificar(reencola_hora and reencola_config and gate_maestro_config,
              "al recuperarse el enlace, el Maestro reencola HORA (pendHora = true) y CONFIGURACION "
              "(pendConfig = true) y su puerta de entrada exige MDG_SIN_CONFIG",
              "el Maestro no reencola la configuracion al reconectar o su puerta no exige MDG_SIN_CONFIG")

    # ===========================================================================
    # 7. LOS MOTIVOS DE RECHAZO
