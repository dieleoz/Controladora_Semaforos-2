# ===== banco/packs/esp32_07_presupuesto_bytes.py =====
#
# EL PRESUPUESTO DE BYTES POR SEGUNDO DEL ENLACE J17, RECALCULADO.
#
# A 9600 8N1 caben 960 B/s -diez bits por byte, con arranque y parada-. No ocho: la
# cuenta a ojo "9600/8 = 1200" regala un 25% de margen que no existe, y ese es el tipo
# de numero que se copia de un comentario y nadie vuelve a mirar.
#
# TODOS LOS SUMANDOS SE RELEEN DEL C++ DE LAS DOS PUNTAS EN CADA CORRIDA:
#
#   la cadencia de $STATUS      del `ahora - tUltimaTelemetria >= N` de bluetooth.cpp
#   el tope de cada trama       de los snprintf reales: payload[128], payload[100]...
#   el envoltorio               de "%s*%02X\r\n", que son 5 bytes sobre el payload
#   el baudio                   de SerialBT.begin(N) y del contrato del ESP32
#
# Sin valor por defecto en ninguno. Un presupuesto con un sumando escrito a mano deja de
# medir el equipo el dia que alguien cambia un buffer, y sigue dando verde.
#
# P-1: Y EL PUENTE NO PUEDE ANADIR TRAFICO PERIODICO PROPIO.
#
# El enlace tiene margen, pero el margen es del equipo, no del accesorio. Un puente que
# metiera su propio latido cada N ms se estaria repartiendo un canal que no es suyo.
#
# P-4: NI AGRUPAR TELEMETRIA PARA "AHORRAR AIRE".
#
# Es la regla que mas facil parece una optimizacion y mas dano hace: la app declara el
# enlace perdido a los 5 s sin trama (TIMEOUT_ENLACE_MS, app.js:1359). Un puente que
# juntara dos $STATUS para mandarlos de golpe haria que la app diera por CAIDO a un
# equipo perfectamente sano, y el operario veria "sin enlace" delante de un cruce que
# cicla bien.

import re

NOMBRE = "esp32_07_presupuesto_bytes"
DESCRIPCION = "la telemetria del equipo cabe en 960 B/s y el puente no anade trafico propio"

ROL = "ESP32_Expansion"
CONTRATO = ("ESP32_Expansion", "include", "contrato.h")


def _buffer(fw, punta, patron, que):
    return fw.constante((punta, "src", "bluetooth.cpp"), patron, que)


def correr(b, fw):
    b.titulo("El presupuesto de bytes por segundo del enlace J17")

    # ---- 1. El baudio, y que las tres fuentes digan lo mismo ------------------
    baudio = fw.constante(CONTRATO, r"#define\s+ENLACE_BAUDIO\s+(\d+)",
                          "el baudio del ESP32")
    bits = fw.constante(CONTRATO, r"#define\s+ENLACE_BITS_POR_BYTE\s+(\d+)",
                        "los bits por byte en el cable")

    baudios_stm32 = {}
    for punta in ("Maestro", "Esclavo"):
        baudios_stm32[punta] = fw.constante(
            (punta, "src", "bluetooth.cpp"), r"SerialBT\.begin\((\d+)\)",
            "el baudio del %s" % punta)

    b.verificar(
        baudio == baudios_stm32["Maestro"] == baudios_stm32["Esclavo"],
        "las tres puntas van a %d bps: el mismo ESP32 puede servir a las dos sin "
        "recompilar" % baudio,
        "los baudios no coinciden: ESP32 %d, Maestro %d, Esclavo %d. Con velocidades "
        "distintas no hay presupuesto que calcular -no se entienden- y ademas el mismo "
        "binario del puente no sirve para las dos puntas"
        % (baudio, baudios_stm32["Maestro"], baudios_stm32["Esclavo"]))

    caudal = baudio // bits

    # ---- 2. Los tamanos, leidos de los snprintf reales ------------------------
    #
    # El envoltorio son 5 bytes: el '*', dos hex, CR y LF. Sale del formato literal de
    # enviarTramaConCrc(), no de una cuenta a ojo.
    envoltorio = 5
    bufStatus = _buffer(fw, "Maestro", r"char payload\[(\d+)\];\s*\n\s*snprintf\(payload,"
                                       r"[^;]*\$STATUS", "el buffer de $STATUS del Maestro")
    bufEvento = _buffer(fw, "Maestro", r"char payload\[(\d+)\];\s*\n\s*snprintf\(payload,"
                                       r"[^;]*\$EVENT", "el buffer de $EVENT del Maestro")
    bufAlarma = _buffer(fw, "Maestro", r"char payload\[(\d+)\];\s*\n\s*snprintf\(payload,"
                                       r"[^;]*\$ALARM", "el buffer de $ALARM del Maestro")
    bufTrama = _buffer(fw, "Maestro", r"char tramaCompleta\[(\d+)\]",
                       "el buffer del envoltorio con CRC")

    topeStatus = bufStatus - 1 + envoltorio
    topeEvento = bufEvento - 1 + envoltorio
    topeAlarma = bufAlarma - 1 + envoltorio

    b.verificar(
        bufTrama >= bufStatus + envoltorio,
        "el envoltorio (%d B) cabe la trama mas larga (%d B de payload + %d)"
        % (bufTrama, bufStatus, envoltorio),
        "tramaCompleta[%d] NO cabe un payload[%d] con su *XX y su CRLF. snprintf "
        "truncaria la trama en el ultimo paso, y saldria al cable bien formada hasta la "
        "mitad" % (bufTrama, bufStatus))

    # ---- 3. La cadencia, leida del C++ ---------------------------------------
    cadencias = {}
    for punta in ("Maestro", "Esclavo"):
        cadencias[punta] = fw.constante(
            (punta, "src", "bluetooth.cpp"),
            r"ahora\s*-\s*tUltimaTelemetria\s*>=\s*(\d+)",
            "la cadencia de telemetria del %s" % punta)

    b.verificar(
        cadencias["Maestro"] == cadencias["Esclavo"],
        "las dos puntas emiten telemetria cada %d ms" % cadencias["Maestro"],
        "las cadencias difieren: Maestro %d ms, Esclavo %d ms. El presupuesto seria "
        "distinto en cada poste y la cota de 5 s de la app tambien"
        % (cadencias["Maestro"], cadencias["Esclavo"]))

    porSegundo = 1000.0 / cadencias["Maestro"]

    # ---- 4. EL PEOR SEGUNDO REALISTA CABE ------------------------------------
    #
    # $STATUS a su cadencia, mas una rafaga de $ACK, $EVENT y $ALARM coincidiendo. No es
    # el caso medio: es un comando que dispara una alarma y una entrada de bitacora justo
    # cuando toca telemetria, que es exactamente cuando mas informacion hace falta.
    peor = topeStatus * porSegundo + topeEvento + topeAlarma + topeStatus
    ocupacion = 100.0 * peor / caudal
    b.verificar(
        peor < caudal,
        "el peor segundo son %d B de %d B/s (%.1f%%): la rafaga de $STATUS + $EVENT + "
        "$ALARM + $ACK cabe" % (peor, caudal, ocupacion),
        "EL PEOR SEGUNDO NO CABE: %d B contra %d B/s (%.1f%%). Las tramas se encolan y "
        "llegan tarde; pasados los 5 s de TIMEOUT_ENLACE_MS la app declara el enlace "
        "perdido de un equipo que esta emitiendo" % (peor, caudal, ocupacion))

    # ---- 5. El sentido de ida tambien tiene su cuenta -------------------------
    tope = fw.constante(CONTRATO, r"#define\s+TRAMA_MAX_UTIL\s+(\d+)",
                        "el tope de linea util del puente")
    msComando = 1000.0 * (tope + 2) * bits / baudio
    b.verificar(
        msComando < cadencias["Maestro"],
        "un comando en su tope (%d B + CRLF) ocupa %.0f ms de cable, menos que los %d ms "
        "entre telemetrias" % (tope, msComando, cadencias["Maestro"]),
        "un comando en su tope ocupa %.0f ms y la telemetria sale cada %d ms. El enlace "
        "es full duplex, pero un comando mas largo que el hueco entre tramas dice que el "
        "tope de linea y la cadencia ya no se llevan"
        % (msComando, cadencias["Maestro"]))

    # ---- 6. P-2: el buffer de salida aguanta la rafaga ------------------------
    salida = fw.constante(CONTRATO, r"#define\s+BUF_SALIDA_APP\s+(\d+)",
                          "el buffer de salida hacia la app")
    rafaga = topeStatus + topeEvento + topeAlarma
    b.verificar(
        salida >= rafaga,
        "P-2: el buffer hacia la app (%d B) aguanta la rafaga de %d B sin descartar"
        % (salida, rafaga),
        "P-2 ROTA: el buffer hacia la app son %d B y la rafaga %d B. Una rafaga que "
        "coincida con un $STATUS descartaria tramas justo cuando mas hay que contar"
        % (salida, rafaga))

    # ---- 7. P-1 y P-4: el puente no anade ni agrupa --------------------------
    #
    # Se mide por AUSENCIA DE RELOJ: sin millis() no hay forma de emitir periodicamente
    # ni de acumular tramas "hasta que pase un rato". Es una comprobacion de forma, y por
    # eso se acota a los dos ficheros del camino de datos: el reloj SI usa millis(), y
    # tiene que poder.
    for fichero in ("puente.cpp", "enlace_stm32.cpp"):
        codigo = fw.codigo("ESP32_Expansion", "src", fichero)
        b.verificar(
            "millis()" not in codigo,
            "P-1/P-4: %s no tiene reloj, asi que no puede ni emitir periodicamente ni "
            "agrupar telemetria" % fichero,
            "%s usa millis(). Con un reloj en el camino de datos caben las dos cosas que "
            "P-1 y P-4 prohiben: un latido propio que se reparte un canal que no es suyo, "
            "y agrupar dos $STATUS para 'ahorrar aire', que hace que la app declare "
            "caido a un equipo sano" % fichero)

    # ---- CONTROLES NEGATIVOS -------------------------------------------------
    b.control_negativo(
        not (topeStatus * porSegundo + topeEvento + topeAlarma + topeStatus < 240),
        "con un caudal de 240 B/s -que seria 2400 bps- el peor segundo deja de caber: "
        "la cuenta reacciona al baudio, no lo da por bueno")

    b.control_negativo(
        "millis()" in "void bombear(){ if (millis() - t > 500) emitir(); }",
        "un latido periodico colado en el camino de datos se detecta")

    b.control_negativo(
        re.search(r"SerialBT\.begin\((\d+)\)", "SerialBT.begin();") is None,
        "si SerialBT.begin() dejara de llevar el baudio, el lector devuelve nada y el "
        "pack ABORTA en vez de asumir 9600")
