# ===== banco/packs/esclavo_01_latch_ambar.py =====
#
# EL LATCH DE AMBAR DEL ESCLAVO - lo mas delicado de esta punta
#
# Una vez alguien baja al minimo seguro, NADA que llegue por radio puede sacar de ahi
# al Esclavo. Es la salida de emergencia, y una salida de emergencia que el Maestro
# pueda cancelar a distancia no es una salida.
#
# D-30 (14/09) - ESTE PACK SE REAPUNTO, NO SE BORRO, Y ESTE PARRAFO ES EL PORQUE.
#
# Hasta el 14/09 el latch que media era `ambarLocal`, el que armaba la secuencia B.B.B
# del mando de reles. Retirado el mando, ese latch no existe. Pero LA PROPIEDAD NO SE
# FUE CON EL: el firmware conserva el otro, `ambarEmergencia` -el que pide la app-, y
# son LAS MISMAS TRES GUARDAS de main.cpp las que lo leen; hasta ese dia preguntaban
# por los dos y ahora preguntan solo por este.
#
# Borrar el pack habria dejado esa propiedad sin ningun instrumento que la EJECUTE:
# esclavo_07 y costura_14 la miran en el TEXTO de las guardas, que es otra cosa y no
# ve un orden de llamadas equivocado. Asi que se muda el escenario, se conservan los
# bloques que siguen valiendo y se invierte el que cambio de signo (CLAUDE.md 9).
#
# QUE LE PASO A CADA BLOQUE, dicho aqui para que nadie tenga que deducirlo:
#   1.1  REAPUNTADO - el barrido, literal, sobre el latch de la app
#   1.2  REAPUNTADO - su control negativo
#   1.3  BORRADO    - media la ventana de los destellos de confirmacion entre
#                     reconocer la secuencia y armar el latch. La app no tiene esa
#                     ventana: arma el latch y enciende el ambar en la misma llamada,
#                     asi que el hallazgo que documentaba se CERRO al salir el mando
#   1.4  INVERTIDO  - cambio de signo, ver el bloque
#   1.5  REAPUNTADO - la cota del Maestro esperando un ACK que no llega
#   1.6  REAPUNTADO - la revocacion, que ahora es CANCELAR_AMBAR en vez de A.A.A

from banco.modelos.esclavo import *          # noqa: F401,F403
from banco.modelos.esclavo import (          # noqa: F401
    CMD, Esclavo, preparar_nodo, _llevar_a,
)

# EJERCE SFTY-21: el latch de ambar del Esclavo: nada por radio saca al nodo de el.

NOMBRE = "esclavo_01_latch_ambar"
DESCRIPCION = "el latch de ambar de la app: lo mas delicado del Esclavo"


def correr(b, fw):
    verificar = b.verificar
    propiedad = b.propiedad
    hallazgo = b.reportar
    titulo = b.titulo
    b.titulo("EL LATCH DE AMBAR DEL ESCLAVO - lo mas delicado de esta punta")

    titulo("1. EL LATCH DE AMBAR PEDIDO POR LA APP")

    print("\n-- 1.1 Barrido: ninguna orden de luz puede pisar el ambar pedido --")
    # Se prueban TODAS las tramas del protocolo contra el nodo con el latch
    # puesto, y desde varios estados de partida. No es una muestra: son todos los
    # comandos que existen por todos los estados en los que el nodo puede estar.
    comandos = sorted(set(CMD.values()))
    partidas = ["rojo", "verde", "degradado_activo", "fallo"]
    violaciones = []
    for estado_inicial in partidas:
        for cmd in comandos:
            e = preparar_nodo()
            _llevar_a(e, estado_inicial)
            # CMD:AMBAR_EMERGENCIA: alguien pide el minimo seguro desde el telefono
            e.pedir_ambar_emergencia()
            e.correr(1000)
            if not e.ambar.armado:
                violaciones.append(("no se armo el latch", estado_inicial, cmd))
                continue
            marca_tx = len(e.tx)
            for _ in range(6):
                e.rx.append((cmd, 0))
                e.correr(1500)
            if e.verde_encendido():
                violaciones.append(("VERDE con el latch puesto", estado_inicial, cmd))
            acks = [c for (_, c, _) in e.tx[marca_tx:]
                    if c in (CMD["CMD_ACK_RED"], CMD["CMD_ACK_GREEN"])]
            if acks and cmd in (CMD["CMD_GO_RED"], CMD["CMD_GO_GREEN"]):
                violaciones.append(("acuso recibo de una orden que no obedecio", estado_inicial, cmd))

    verificar(not violaciones,
              "Barrido de %d comandos x %d estados de partida: con el ambar de la app puesto "
              "el Esclavo ni enciende verde ni acusa recibo de ninguna orden de luz."
              % (len(comandos), len(partidas)),
              "Se encontraron %d violaciones: %s" % (len(violaciones), violaciones[:4]))

    print("\n-- 1.2 Control negativo: el mismo barrido SIN la desobediencia --")
    # Si la prueba de arriba no distinguiera un firmware con la guarda de uno
    # sin ella, no estaria midiendo la guarda. Se repite con
    # obedece_ambar_emergencia a False y se EXIGE que la prueba falle.
    caza = False
    e = preparar_nodo(obedece_ambar_emergencia=False)
    e.pedir_ambar_emergencia()
    e.correr(1000)
    for _ in range(6):
        e.rx.append((CMD["CMD_GO_GREEN"], 0))
        e.correr(1500)
    if e.verde_encendido() or CMD["CMD_ACK_GREEN"] in e.acks_de_luz():
        caza = True
    verificar(caza,
              "El barrido SI distingue: quitando la guarda 'if (!bluetooth_ambarEmergencia())' "
              "el mismo escenario da verde o ACK, y la prueba lo caza.",
              "PELIGRO METODOLOGICO: la prueba 1.1 da PASS tambien sin la guarda, "
              "asi que no estaba midiendo nada")

    print("\n-- 1.3 El ambar se arma en la MISMA llamada, sin ventana --")
    # BLOQUE INVERTIDO (D-30, 14/09). Lo que habia aqui media una VENTANA: con el
    # mando, el latch no se ponia al reconocer B.B.B sino al EJECUTAR la accion,
    # despues de los destellos de confirmacion, y una orden de verde que cayera en ese
    # hueco se obedecia Y SE ACUSABA. Quedo escrito como hallazgo, no como fallo,
    # porque el equipo terminaba donde debia.
    #
    # La app no tiene ese hueco: arma el latch y enciende el ambar en la misma llamada.
    # Asi que lo que antes era un hallazgo se comprueba ahora como PROPIEDAD, que es lo
    # que la retirada del mando compro: no hay instante entre "se pidio" y "esta puesto"
    # en el que una orden de radio pueda colarse.
    e = preparar_nodo()
    _llevar_a(e, "rojo")
    marca = len(e.tx)
    e.ambar.pedir()                       # sin correr un solo tick despues
    armado_al_instante = e.ambar.armado
    e.rx.append((CMD["CMD_GO_GREEN"], 0))
    e.correr(AMARILLO_A_VERDE_MS + 3000)
    acks = [c for (_, c, _) in e.tx[marca:] if c == CMD["CMD_ACK_GREEN"]]
    verificar(armado_al_instante and not acks and not e.verde_encendido(),
              "El ambar de la app queda armado en la misma llamada que lo pide: una orden de "
              "verde que llegue inmediatamente despues no se obedece ni se acusa. No hay "
              "ventana, que es justo lo que el mando si tenia (sus destellos duraban segundos).",
              "Hay una VENTANA entre pedir el ambar y que el latch proteja: armado=%s, "
              "ACK_GREEN=%d, verde=%s. El Maestro se quedaria creyendo que esta punta tiene "
              "el verde que le concedio." % (armado_al_instante, len(acks), e.verde_encendido()))

    print("\n-- 1.4 Con el ambar vigente, el Degradado se RECHAZA (no revoca el ambar) --")
    # BLOQUE INVERTIDO (D-30, 14/09), y es el que mas cambia de signo.
    #
    # LO QUE EXIGIA ANTES, y era un defecto documentado: con el mando, A.B.A.B bajaba
    # `ambarLocal` DENTRO de ejecutar(ACC_DEGRADADO) y lo bajaba ANTES de saber si
    # degradado_entrar() iba a aceptar. Si las condiciones se caian durante los cuatro
    # destellos, el equipo se quedaba SIN ambar y SIN Degradado: el operario habia
    # contado la senal de HECHO y no se hizo ninguna de las dos cosas.
    #
    # HOY NO PUEDE PASAR, y no por suerte: la unica via de entrada es SET_MODO:DEGRADADO
    # por app (D-18), que no revoca nada -pregunta-, y degradado_comprobar() lleva
    # `if (bluetooth_ambarEmergencia()) return DEG_RECHAZO_AMBAR_VIGENTE`. Con el ambar
    # puesto no se entra, y el ambar SIGUE PUESTO. Se exige justo eso.
    e = preparar_nodo()
    e.pedir_ambar_emergencia()
    e.correr(1000)
    assert e.ambar.armado
    motivo = e.degradado.entrar()
    e.correr(2000)
    verificar(motivo == "DEG_RECHAZO_AMBAR_VIGENTE"
              and e.ambar.armado and not e.degradado.gobierna_luz(),
              "Con un ambar de la app vigente, la entrada al Degradado se RECHAZA nombrando "
              "el motivo (DEG_RECHAZO_AMBAR_VIGENTE) y el ambar SOBREVIVE: el equipo no se "
              "queda ni sin ambar ni sin modo, que es el hueco que dejaba el A.B.A.B del mando.",
              "Pedir el Degradado con el ambar vigente dio %r, latch=%s, gobierna=%s. Si el "
              "latch se cae aqui, una peticion rechazada deja al equipo sin la proteccion que "
              "alguien pidio y sin el modo que pidio despues."
              % (motivo, e.ambar.armado, e.degradado.gobierna_luz()))

    # Control: sin el ambar puesto, el MISMO Degradado entra. Sin esta mitad, el
    # escenario de arriba podria ser simplemente un Degradado que nunca funciona en el
    # modelo, y la guarda no estaria midiendose (CLAUDE.md 9: el escenario nuevo es el
    # control que le falta a toda inversion).
    e2 = preparar_nodo()
    motivo2 = e2.degradado.entrar()
    e2.correr(2000)
    verificar(motivo2 == "DEG_ACEPTADO" and e2.degradado.gobierna_luz(),
              "Control: sin ambar vigente, la MISMA peticion de Degradado entra. El rechazo de "
              "arriba lo causa el latch y no un Degradado que no funcione.",
              "El Degradado no entra ni con las condiciones cumplidas (motivo=%r, estado=%s): "
              "el escenario anterior no probaria lo que dice" % (motivo2, e2.degradado.estado))

    print("\n-- 1.5 Puede el Maestro quedarse esperando el ACK para siempre? --")
    e = preparar_nodo()
    e.pedir_ambar_emergencia()
    e.correr(1000)
    m = MaestroEsperandoAck(e)
    limite = 120000
    while m.estado != "C_FALLO" and m.t < limite:
        e.loop(10)
        m.loop(10)
    cayo = m.estado == "C_FALLO"
    cota_reintentos = TIMEOUT_ACK_MS * REINTENTOS_MAX
    cota = max(cota_reintentos, MAESTRO_SIN_RX_MS)
    verificar(cayo and m.t <= cota + 1000,
              "Con el Esclavo callado, el Maestro cae a C_FALLO en %.1f s. Las dos vias que lo "
              "tumban son independientes: silencio total a los %.1f s y reintentos agotados a "
              "los %.1f s; gana la primera. No hay espera infinita."
              % (m.t / 1000.0, MAESTRO_SIN_RX_MS / 1000.0, cota_reintentos / 1000.0),
              "El Maestro NO cayo a C_FALLO en %d s: se queda esperando un ACK que no llegara"
              % (limite // 1000))

    # Control negativo: sin el contador de reintentos, lo detectaria la prueba?
    e2 = preparar_nodo()
    e2.pedir_ambar_emergencia()
    e2.correr(1000)
    m2 = MaestroEsperandoAck(e2, limite_reintentos=False)
    # Ademas se le regala comunicacion: alguien contesta PONG por el.
    while m2.t < 60000:
        e2.loop(10)
        m2.tUltimaRx = m2.t          # se finge enlace vivo: solo falta el ACK
        m2.loop(10)
    verificar(m2.estado != "C_FALLO",
              "El control negativo confirma que la prueba mide algo: sin el limite de "
              "reintentos y con el enlace fingido vivo, el Maestro SI se quedaria esperando "
              "indefinidamente, y la prueba lo distingue del caso real.",
              "El control negativo tambien cae a C_FALLO: la prueba 1.5 no discrimina")

    print("\n-- 1.6 CANCELAR_AMBAR revoca el latch y devuelve el mando al Maestro --")
    # REAPUNTADO: la revocacion era A.A.A desde el suelo y hoy es CMD:CANCELAR_AMBAR
    # desde la app (R-3). La propiedad es la misma y es la que impide que el latch sea
    # una trampa: se pone a proposito y se quita a proposito, pero se PUEDE quitar.
    e = preparar_nodo()
    e.pedir_ambar_emergencia()
    e.correr(1000)
    assert e.ambar.armado
    e.cancelar_ambar_emergencia()
    latch_bajado = not e.ambar.armado
    e.rx.append((CMD["CMD_GO_GREEN"], 0))
    e.correr(AMARILLO_A_VERDE_MS + 2000)
    verificar(latch_bajado and e.verde_encendido(),
              "CANCELAR_AMBAR revoca el latch y la siguiente orden de verde del Maestro se "
              "obedece: el nodo vuelve a estar bajo mando.",
              "Tras CANCELAR_AMBAR el nodo no volvio a obedecer (latch=%s, verde=%s)"
              % (e.ambar.armado, e.verde_encendido()))
