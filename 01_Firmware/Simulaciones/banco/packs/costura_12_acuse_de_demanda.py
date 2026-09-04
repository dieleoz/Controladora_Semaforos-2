# ===== banco/packs/costura_12_acuse_de_demanda.py =====
#
# UN ACUSE QUE NO DEPENDE DE LO QUE PASO, REPARTIDO ENTRE DOS PLACAS (N-130)
#
# La barrera de salidas -CLAUDE.md §6- ya prohibe que un despachador conteste "$ACK"
# sin mirar lo que la llamada devolvio. Este pack vigila la version que esa regla NO
# cubria: la mentira que necesita LAS DOS PUNTAS para existir.
#
#   Esclavo   SOLICITAR_PASO  ->  "$ACK,CMD:SOLICITAR_PASO,RESULT:PEDIDO_AL_MAESTRO"
#                                 y manda CMD_DEMANDA por radio
#   Maestro   recibe CMD_DEMANDA  ->  demandaRemotaPendiente = true   + CMD_ACK_DEMANDA
#                                     y esa bandera la lee UN SOLO fichero
#
# Hasta el 04/09 el Maestro armaba la bandera SIEMPRE, y solo modo_inteligente.cpp la
# consumia. En Manual y en Automatico el operario de pie junto al Poste 2 pulsaba, leia
# la confirmacion, y el cruce no se movia. Volvia a pulsar.
#
# POR QUE NO LO VIO NADIE, que es lo reutilizable: leida por separado, NINGUNA de las
# dos ramas esta mal escrita. La del Esclavo dice la verdad -la peticion salio-. La del
# Maestro acusa un paquete que efectivamente recibio. El defecto solo existe en la
# COSTURA, y ningun pack de una punta puede verlo. Por eso este vive aqui.
#
# Y ES LA MISMA FORMA QUE N-73 UNA CAPA ARRIBA: alli una funcion declarada sin
# llamador; aqui una BANDERA armada sin lector en el modo en que se arma. La pregunta
# no es "¿existe la bandera?" sino "¿hay alguien que la mire cuando se enciende?".
#
# LO QUE ESTE PACK NO PUEDE COMPROBAR, y va escrito para que no se lea como permiso:
# que el Maestro atienda de verdad la demanda cuando dice que si. Eso lo ejerce
# Validacion_Automatico sobre el C++ real; aqui solo se exige que lo que CONTESTA
# corresponda con lo que va a HACER.

import re

NOMBRE = "costura_12_acuse_de_demanda"
DESCRIPCION = "el acuse de la demanda dice si el Maestro la va a atender, y la bandera solo se arma si alguien la lee"

COORD = ("Maestro", "src", "coordinador.cpp")
ESCLAVO_MAIN = ("Esclavo", "src", "main.cpp")


def _protocolo(fw, punta):
    return fw.codigo(punta, "include", "protocolo.h")


def correr(b, fw):
    b.titulo("EL ACUSE DE LA DEMANDA, Y LA BANDERA QUE NADIE MIRABA")

    coord = fw.codigo(*COORD)

    # ---- 1. Los motivos existen y valen lo mismo en las dos puntas ---------------
    #
    # protocolo.h es contrato compartido: costura_01 ya exige que sean identicos. Aqui
    # se comprueba ademas que los DOS NUMEROS se leen de verdad del fuente, sin valor
    # por defecto: si el lector no los encuentra, el pack ABORTA en vez de suponerlos.
    motivos = {}
    for punta in ("Maestro", "Esclavo"):
        txt = _protocolo(fw, punta)
        for nombre in ("DEMANDA_ACEPTADA", "DEMANDA_RECHAZADA"):
            m = re.search(r"#define\s+%s\s+(\d+)" % nombre, txt)
            if not m:
                raise fw.Abortado(
                    "no se halla %s en %s/include/protocolo.h. Sin los dos motivos no "
                    "hay con que distinguir un acuse que atiende de uno que no, y "
                    "suponerlos seria inventar el contrato" % (nombre, punta))
            motivos.setdefault(nombre, []).append(int(m.group(1)))

    b.verificar(
        motivos["DEMANDA_ACEPTADA"][0] == motivos["DEMANDA_ACEPTADA"][1]
        and motivos["DEMANDA_RECHAZADA"][0] == motivos["DEMANDA_RECHAZADA"][1],
        "los dos motivos del acuse valen lo mismo en las dos puntas (aceptada=%d, "
        "rechazada=%d)" % (motivos["DEMANDA_ACEPTADA"][0], motivos["DEMANDA_RECHAZADA"][0]),
        "los motivos del acuse DIFIEREN entre puntas: aceptada=%s rechazada=%s. Un "
        "numero que significa cosas distintas a cada lado de la radio es peor que no "
        "tenerlo: el Esclavo leeria 'no atendida' donde el Maestro dijo que si"
        % (motivos["DEMANDA_ACEPTADA"], motivos["DEMANDA_RECHAZADA"]))

    # ---- 2. ACEPTADA vale 0, y eso NO es cosmetica ------------------------------
    #
    # protocolo_enviarPaquete(cmd, param = 0) pone cero por defecto. Un Maestro con
    # firmware viejo -que no conoce estos motivos- manda cero, y el Esclavo tiene que
    # leerlo como "aceptada", que es el comportamiento de siempre. Si ACEPTADA fuera
    # otro numero, actualizar UNA punta convertiria cada demanda en una falsa alarma.
    b.verificar(
        motivos["DEMANDA_ACEPTADA"][0] == 0,
        "DEMANDA_ACEPTADA vale 0: un Maestro sin actualizar manda el param por defecto "
        "y se sigue leyendo como el comportamiento de siempre",
        "DEMANDA_ACEPTADA vale %d y no 0. protocolo_enviarPaquete() pone param=0 por "
        "defecto, asi que un Maestro con firmware viejo mandaria un valor que esta "
        "punta leeria como RECHAZADA: cada demanda atendida se anunciaria como no "
        "atendida" % motivos["DEMANDA_ACEPTADA"][0])

    # ---- 3. La bandera NO se arma incondicionalmente ----------------------------
    #
    # Se lee el bloque de la rama, no el fichero entero: lo que importa es que la
    # asignacion viva DENTRO de una condicion, no que exista un 'if' en alguna parte.
    m = re.search(r"pkt\.command\s*==\s*CMD_DEMANDA\s*\)\s*\{(.*?)\n    \}",
                  coord, re.S)
    if not m:
        raise fw.Abortado(
            "no se halla la rama de CMD_DEMANDA en Maestro/src/coordinador.cpp. El "
            "lector se quedo ciego, y aprobar un acuse que no se ha leido es "
            "exactamente lo que este pack existe para impedir")
    rama = m.group(1)

    # SE MIDE POR PROFUNDIDAD DE LLAVES, NO POR LA FORMA DEL TEXTO.
    #
    # La primera version de esta comprobacion buscaba el patron literal
    # "if (...) { demandaRemotaPendiente = true;" y FALLO sobre el firmware CORRECTO:
    # la guarda real calcula el booleano en una linea aparte -por legibilidad- y el
    # regex no lo reconocia. Un detector que acusa a un firmware bueno es tan inutil
    # como uno que aprueba a uno malo, y ademas ensena a ignorar el rojo (§3, el FALLA
    # permanente). Lo que decide no es COMO este escrita la condicion sino si la
    # asignacion cuelga de ALGUNA: se cuentan las llaves abiertas antes de ella.
    armados = [m.start() for m in
               re.finditer(r"demandaRemotaPendiente\s*=\s*true\s*;", rama)]

    def _profundidad(pos):
        trozo = rama[:pos]
        return trozo.count("{") - trozo.count("}")

    sueltos = [p for p in armados if _profundidad(p) <= 0]
    b.verificar(
        bool(armados) and not sueltos,
        "la bandera de demanda remota solo se arma DENTRO de una condicion: no se "
        "enciende para un modo que no va a mirarla",
        "demandaRemotaPendiente se arma incondicionalmente. La lee un solo fichero "
        "-modo_inteligente.cpp-, asi que en los demas modos queda encendida sin lector "
        "mientras el Esclavo ya le dijo a la app que la peticion iba camino del "
        "Maestro: el operario pulsa, lee la confirmacion y el cruce no se mueve")

    # ---- 4. Y el acuse VIAJA CON MOTIVO -----------------------------------------
    b.verificar(
        re.search(r"protocolo_enviarPaquete\s*\(\s*CMD_ACK_DEMANDA\s*,", rama) is not None,
        "el acuse de la demanda lleva motivo: dice si se va a atender, no solo que "
        "llego",
        "el acuse sale como protocolo_enviarPaquete(CMD_ACK_DEMANDA) a secas. El "
        "Esclavo no puede distinguir 'te la atiendo' de 'me llego y la tiro', asi que "
        "no tiene con que desmentir el ACK que ya le dio a la app")

    # ---- 5. La condicion nombra el MISMO modo que consume la bandera ------------
    #
    # Esta es la comprobacion que de verdad envejece bien. Las cuatro de arriba
    # seguirian pasando si manana alguien anadiera un consumidor en modo_automatico.cpp
    # y se olvidara de ampliar la guarda: la bandera volveria a armarse de menos, y el
    # sintoma seria el contrario -una demanda legitima ignorada- pero igual de mudo.
    #
    # El censo es grep de las llamadas, no lectura (§3.ter).
    consumidores = []
    for fichero in ("modo_inteligente.cpp", "modo_automatico.cpp", "modo_manual.cpp",
                    "coordinador.cpp", "modo_degradado.cpp", "main.cpp"):
        try:
            codigo = fw.codigo("Maestro", "src", fichero)
        except Exception:
            continue
        if fichero == "coordinador.cpp":
            continue          # es quien la declara y la arma, no un consumidor
        if re.search(r"coordinador_hayDemandaRemota\s*\(", codigo):
            consumidores.append(fichero)

    b.reportar(
        "quien consume hoy la bandera de demanda remota",
        ", ".join(consumidores) if consumidores else "NADIE")

    # MODO_INTELIGENTE se lee de la guarda, no se supone.
    modos_en_guarda = set(re.findall(r"modoActual_get\s*\(\s*\)\s*==\s*(MODO_[A-Z]+)", rama))
    b.verificar(
        consumidores == ["modo_inteligente.cpp"] and modos_en_guarda == {"MODO_INTELIGENTE"},
        "la guarda deja pasar exactamente los modos que tienen consumidor: %s arma, y "
        "%s lee" % (sorted(modos_en_guarda), consumidores),
        "la guarda y los consumidores NO cuadran: la guarda arma en %s y quien lee la "
        "bandera es %s. Si sobra un modo en la guarda, el equipo vuelve a decir que si "
        "a algo que no hara; si falta, una demanda legitima se ignora sin que nadie lo "
        "sepa" % (sorted(modos_en_guarda), consumidores or "NADIE"))

    # ---- 6. Y el Esclavo hace algo con el motivo --------------------------------
    #
    # Esta rama estaba VACIA, con un comentario describiendo lo que no hacia. Una rama
    # vacia es la version silenciosa de la prueba muerta: parece que se atiende.
    esclavo = fw.codigo(*ESCLAVO_MAIN)
    m2 = re.search(r"pkt\.command\s*==\s*CMD_ACK_DEMANDA\s*\)\s*\{(.*?)\n    \}",
                   esclavo, re.S)
    if not m2:
        raise fw.Abortado(
            "no se halla la rama de CMD_ACK_DEMANDA en Esclavo/src/main.cpp")
    rama_e = m2.group(1)
    b.verificar(
        "DEMANDA_RECHAZADA" in rama_e
        and re.search(r"bluetooth_reportarEvento\s*\(", rama_e) is not None,
        "el Esclavo mira el motivo y AVISA cuando el Maestro no la atiende: es el unico "
        "sitio donde se puede desmentir el $ACK que ya salio",
        "la rama de CMD_ACK_DEMANDA del Esclavo no mira DEMANDA_RECHAZADA o no avisa. "
        "El $ACK,RESULT:PEDIDO_AL_MAESTRO ya se mando cientos de milisegundos antes y "
        "no se puede retirar: si aqui no se avisa, la negativa del Maestro no llega a "
        "nadie y el operario se queda pulsando")

    # ---- CONTROLES NEGATIVOS ----------------------------------------------------
    b.control_negativo(
        re.search(r"^\s*demandaRemotaPendiente\s*=\s*true\s*;",
                  "      demandaRemotaPendiente = true;\n", re.M) is not None,
        "el detector de armado incondicional reconoce la forma defectuosa cuando se le "
        "pone delante: no aprueba por no saber mirar")

    b.control_negativo(
        re.search(r"protocolo_enviarPaquete\s*\(\s*CMD_ACK_DEMANDA\s*,",
                  "protocolo_enviarPaquete(CMD_ACK_DEMANDA);") is None,
        "y el del acuse sin motivo distingue la llamada pelada de la que lleva motivo")

    b.control_negativo(
        re.search(r"modoActual_get\s*\(\s*\)\s*==\s*(MODO_[A-Z]+)",
                  "if (modoActual_get() == MODO_AUTOMATICO) {").group(1) == "MODO_AUTOMATICO",
        "el lector de la guarda extrae el modo REAL que aparece escrito, no uno "
        "supuesto: si manana cambia, la comparacion con los consumidores lo acusa")
