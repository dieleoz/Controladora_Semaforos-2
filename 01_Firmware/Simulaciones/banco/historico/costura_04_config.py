# ===== banco/packs/costura_04_config.py =====
#
# SECUENCIA DE CONFIGURACION: EL ESCLAVO ACUSA SOLO CON LA ULTIMA DEL PAR
#
# La otra cara del 30/31 del Esclavo, vista desde la costura: el Maestro da el
# envio por bueno al recibir el ACK, pone pendConfig=false y NO reintenta. El fallo
# queda tapado en la punta que podria corregirlo.

from banco.modelos.costura import *          # noqa: F401,F403

NOMBRE = "costura_04_config"
DESCRIPCION = "el Esclavo acusa solo con la ultima trama del par"


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

    b.titulo("SECUENCIA DE CONFIGURACION: EL ESCLAVO ACUSA SOLO CON LA ULTIMA DEL PAR")


    # FASE 2 (03/08/2026): el ACK ya no se emite DENTRO de la rama de DESPEJE.
    # config_ciclo.cpp decide si el par se cerro y DEVUELVE ese si/no; main.cpp acusa al
    # recibirlo, que es donde vive la maquinaria de cortesia de SFTY-17.
    #
    # El comportamiento es identico -el ACK sale exactamente cuando el par se cierra- y
    # la propiedad comprobada tampoco cambia: la rama de VERDE no acusa, y el par
    # cerrado acusa UNA sola vez. Lo unico que se mueve es donde hay que mirarlo.
    acusa_en_verde = bool(rama_verde and "programarRespuesta" in rama_verde.group(1))
    acks_en_despeje = len(re.findall(
        r"if\s*\(\s*config_rxDespeje\([^)]*\)\s*\)\s*\{\s*\n\s*programarRespuesta\(\s*CMD_ACK_CONFIG",
        T_E_MAIN_C))
    verificar(not acusa_en_verde and acks_en_despeje == 1,
              "en el fuente: la rama CMD_CONFIG_VERDE no programa ninguna respuesta y la de "
              "CMD_CONFIG_DESPEJE programa exactamente un CMD_ACK_CONFIG",
              f"la rama de VERDE acusa={acusa_en_verde}, ACKs en DESPEJE={acks_en_despeje}: el "
              "Maestro recibiria un ACK sobrante que ocupa el canal half-duplex")

    # El Maestro manda las dos SEGUIDAS y espera UNO.
    envio_seguido = bool(re.search(
        r"protocolo_enviarPaquete\(CMD_CONFIG_VERDE[^;]*\);\s*\n\s*protocolo_enviarPaquete\(CMD_CONFIG_DESPEJE",
        T_M_COORD_C))
    espera_uno = bool(re.search(r"estadoSync\s*==\s*SY_ESPERA_ACK_CONFIG\s*&&\s*pkt->command\s*==\s*CMD_ACK_CONFIG",
                                T_M_COORD_C))
    verificar(envio_seguido and espera_uno,
              "el Maestro envia VERDE y DESPEJE seguidas, sin espera entre ellas, y cierra el "
              "intercambio con UN solo CMD_ACK_CONFIG",
              "el Maestro no envia el par seguido o no espera un unico ACK")


    def esclavo_acks(separacion_ms, acusa_las_dos):
        """Modela la ranura UNICA de respuesta del Esclavo (programarRespuesta()).

        `acusa_las_dos` conmuta la version DEFECTUOSA -la que acusaba cada trama- y
        la correcta, para demostrar que el modelo las distingue. Sin las dos, un
        PASS aqui no probaria nada.
        """
        pendiente = None      # (comando, instante_de_envio)
        emitidos = 0
        eventos = {0: "VERDE", separacion_ms: "DESPEJE"}
        for t in range(0, separacion_ms + 2 * RETARDO_RESPUESTA_MS + 10):
            # atenderRespuestaPendiente(): sale cuando vence el retardo de cortesia.
            if pendiente is not None and t >= pendiente[1]:
                emitidos += 1
                pendiente = None
            ev = eventos.get(t)
            if ev == "DESPEJE" or (ev == "VERDE" and acusa_las_dos):
                # La nueva PISA a la anterior: solo cabe una respuesta pendiente.
                pendiente = ("ACK", t + RETARDO_RESPUESTA_MS)
        if pendiente is not None:
            emitidos += 1
        return emitidos


    # Barrido COMPLETO de la separacion entre las dos tramas, de 0 a 1 s. La
    # separacion real depende de la rafaga, del repetidor y de si hubo retransmision:
    # no es un numero que nadie controle.
    malos_correcto = [d for d in range(0, 1001) if esclavo_acks(d, acusa_las_dos=False) != 1]
    verificar(not malos_correcto,
              "version ACTUAL: exactamente 1 ACK para CUALQUIER separacion entre las dos tramas "
              "(barrido de 0 a 1000 ms). Ya no depende de que lleguen juntas",
              f"la version actual emite un numero de ACK distinto de 1 en {malos_correcto[:5]} ms")

    dobles = [d for d in range(0, 1001) if esclavo_acks(d, acusa_las_dos=True) == 2]
    verificar(bool(dobles) and min(dobles) == RETARDO_RESPUESTA_MS,
              f"version DEFECTUOSA (acusar las dos): sale 1 ACK mientras las tramas caben en los "
              f"{RETARDO_RESPUESTA_MS} ms de cortesia y salen 2 a partir de {min(dobles) if dobles else '?'} ms. "
              "El modelo reproduce el accidente y distingue el caso bueno del malo",
              "el modelo no distingue acusar una vez de acusar dos: la prueba no vale")

    # ===========================================================================
    # 5. EL LIMITE DE 48 H: ¿caen a la vez o escalonadas?
    # ===========================================================================
    # LA PREGUNTA ABIERTA. Cada unidad cuenta desde SU PROPIA ultima sincronizacion,
    # y esas dos marcas NO se ponen en el mismo instante:
    #
    #   Esclavo: al APLICAR la terna de hora (CMD_HORA_S).
    #   Maestro: al RECIBIR el CMD_ACK_HORA, que sale del Esclavo DESPUES del
    #            retardo de cortesia de SFTY-17 y tarda su tiempo de aire.
    #
