# ===== banco/packs/esclavo_01_latch_ambar.py =====
#
# EL LATCH DE AMBAR LOCAL (B.B.B) - lo mas delicado del Esclavo
#
# Una vez el operario baja al minimo seguro con B.B.B, NADA que llegue por radio
# puede sacar de ahi al Esclavo. Es la salida de emergencia, y una salida de
# emergencia que el Maestro pueda cancelar a distancia no es una salida.

from banco.modelos.esclavo import *          # noqa: F401,F403
from banco.modelos.esclavo import (          # noqa: F401
    CMD, Esclavo, preparar_nodo, _llevar_a,
)

# EJERCE SFTY-21: el latch de ambar local: nada por radio saca al Esclavo de el.

NOMBRE = "esclavo_01_latch_ambar"
DESCRIPCION = "el latch de ambar local (B.B.B): lo mas delicado del Esclavo"


def correr(b, fw):
    verificar = b.verificar
    propiedad = b.propiedad
    hallazgo = b.reportar
    titulo = b.titulo
    b.titulo("EL LATCH DE AMBAR LOCAL (B.B.B) - lo mas delicado del Esclavo")

    titulo("1. EL LATCH DE AMBAR LOCAL (B.B.B) — lo mas delicado del Esclavo")

    print("\n-- 1.1 Barrido: ninguna orden de luz puede pisar el ambar del operario --")
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
            # B.B.B: el operario pide ambar desde el suelo
            e.secuencia([MANDO_B, MANDO_B, MANDO_B])
            e.correr(DESTELLOS_AMBAR * (DESTELLO_ON_MS + DESTELLO_OFF_MS) + 1000)
            if not e.mando.ambar_local:
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
              "Barrido de %d comandos x %d estados de partida: con el ambar local puesto el "
              "Esclavo ni enciende verde ni acusa recibo de ninguna orden de luz."
              % (len(comandos), len(partidas)),
              "Se encontraron %d violaciones: %s" % (len(violaciones), violaciones[:4]))

    print("\n-- 1.2 Control negativo: el mismo barrido SIN la desobediencia --")
    # Si la prueba de arriba no distinguiera un firmware con la guarda de uno
    # sin ella, no estaria midiendo la guarda. Se repite con obedece_ambar_local
    # a False y se EXIGE que la prueba falle.
    caza = False
    e = preparar_nodo(obedece_ambar_local=False)
    e.secuencia([MANDO_B, MANDO_B, MANDO_B])
    e.correr(DESTELLOS_AMBAR * (DESTELLO_ON_MS + DESTELLO_OFF_MS) + 1000)
    for _ in range(6):
        e.rx.append((CMD["CMD_GO_GREEN"], 0))
        e.correr(1500)
    if e.verde_encendido() or CMD["CMD_ACK_GREEN"] in e.acks_de_luz():
        caza = True
    verificar(caza,
              "El barrido SI distingue: quitando la guarda 'if (!mando_ambarLocal())' el mismo "
              "escenario da verde o ACK, y la prueba lo caza.",
              "PELIGRO METODOLOGICO: la prueba 1.1 da PASS tambien sin la guarda, "
              "asi que no estaba midiendo nada")

    print("\n-- 1.3 Ventana entre la secuencia y el latch (los destellos) --")
    # El latch NO se pone al reconocer la secuencia: se pone al EJECUTAR la
    # accion, y eso ocurre despues de los destellos de confirmacion. Entre una
    # cosa y la otra pasan DESTELLOS_AMBAR ciclos completos de destello.
    ventana_ms = DESTELLOS_AMBAR * (DESTELLO_ON_MS + DESTELLO_OFF_MS)
    e = preparar_nodo()
    e.secuencia([MANDO_B, MANDO_B, MANDO_B], separacion=2000)
    # Justo despues del tercer pulso, el Maestro ordena verde.
    marca = len(e.tx)
    e.rx.append((CMD["CMD_GO_GREEN"], 0))
    e.correr(ventana_ms + 6000)
    acks = [c for (_, c, _) in e.tx[marca:] if c == CMD["CMD_ACK_GREEN"]]
    obedecio = len(acks) > 0
    termina_en_ambar = e.semaforo.estado == "S_FALLO" and e.mando.ambar_local
    if obedecio:
        hallazgo(
            "El ambar del mando tarda ~%d ms en armarse y una orden de verde que caiga "
            "en ese hueco SI se obedece y SI se acusa" % ventana_ms,
            ["Reproduccion: B.B.B completo y CMD_GO_GREEN dentro de los %d ms de destellos." % ventana_ms,
             "El Esclavo contesta %d ACK_GREEN y arranca la transicion a verde;" % len(acks),
             "al terminar los destellos, mando_actualizar() pone ambarLocal y se va a ambar.",
             "Estado final: luz=%s, ambarLocal=%s (el equipo SI acaba donde debe)."
             % (e.semaforo.estado, e.mando.ambar_local),
             "Consecuencia: el Maestro se queda creyendo que esta punta tiene el verde",
             "que le concedio, y mantiene SU lado en rojo hasta que el ACK deje de llegar.",
             "No es un verde peligroso -las dos puntas no dan paso a la vez- pero retrasa",
             "la caida a C_FALLO en todo lo que dure ese verde imaginario.",
             "Causa: mando.cpp pone ambarLocal dentro de ejecutar(), que corre DESPUES de",
             "los destellos; entre confirmarYActuar() y ejecutar() la guarda no existe."])
    verificar(termina_en_ambar,
              "Pese a la ventana de los destellos, el equipo TERMINA en ambar intermitente con "
              "el latch puesto: el estado final es el que pidio el operario.",
              "El equipo no acabo en ambar tras un B.B.B (luz=%s, latch=%s)"
              % (e.semaforo.estado, e.mando.ambar_local))

    print("\n-- 1.4 A.B.A.B revoca el ambar aunque el Degradado se rechace despues --")
    # mando.h dice que el ambar de B.B.B se queda "hasta que alguien haga A.A.A".
    # Pero ejecutar(ACC_DEGRADADO) tambien lo baja, y lo baja ANTES de saber si
    # degradado_entrar() va a aceptar. Entre la comprobacion (al reconocer la
    # secuencia) y la ejecucion (tras 4 destellos) hay ~%d ms en los que las
    # condiciones pueden dejar de cumplirse.
    e = preparar_nodo()
    e.secuencia([MANDO_B, MANDO_B, MANDO_B])
    e.correr(DESTELLOS_AMBAR * (DESTELLO_ON_MS + DESTELLO_OFF_MS) + 1000)
    assert e.mando.ambar_local
    e.secuencia([MANDO_A, MANDO_B, MANDO_A, MANDO_B])
    # Mientras cuenta los cuatro destellos, vence el limite duro de 48 h: la
    # condicion que degradado_entrar() volvera a comprobar deja de cumplirse.
    e.degradado.sync_vencida = True
    e.correr(DESTELLOS_DEGRADADO * (DESTELLO_ON_MS + DESTELLO_OFF_MS) + 1000)
    revocado_sin_entrar = (not e.mando.ambar_local) and (not e.degradado.gobierna_luz())
    if revocado_sin_entrar:
        hallazgo(
            "A.B.A.B rechazado en el ultimo momento deja el ambar del operario REVOCADO "
            "y el Degradado sin entrar",
            ["Reproduccion: B.B.B (ambar puesto) y despues A.B.A.B con las condiciones",
             "cumplidas en el instante del cuarto pulso; durante los %d ms de destellos"
             % (DESTELLOS_DEGRADADO * (DESTELLO_ON_MS + DESTELLO_OFF_MS)),
             "vence el limite de 48 h (o llega la sync que lo cambia todo).",
             "ejecutar(ACC_DEGRADADO) hace 'ambarLocal = false' ANTES de llamar a",
             "degradado_entrar(), que rechaza. Resultado: ni ambar ni Degradado.",
             "El operario ha contado 4 destellos -la senal de HECHO- y el equipo no hizo",
             "ninguna de las dos cosas. Con el radio muerto vuelve solo a ambar a los %d s"
             % (SILENCIO_A_AMBAR_MS // 1000),
             "por la via de siempre, pero si el radio esta vivo el Maestro recupera el",
             "mando de esta punta sin que nadie lo haya pedido.",
             "mando.h afirma que ese ambar dura 'hasta que alguien haga A.A.A'."])

    # Control: si las condiciones NO cambian durante los destellos, el mismo
    # A.B.A.B entra al Degradado. Sin esta mitad, el escenario de arriba podria
    # ser simplemente un A.B.A.B que nunca funciona en el modelo.
    e2 = preparar_nodo()
    e2.secuencia([MANDO_B, MANDO_B, MANDO_B])
    e2.correr(DESTELLOS_AMBAR * (DESTELLO_ON_MS + DESTELLO_OFF_MS) + 1000)
    e2.secuencia([MANDO_A, MANDO_B, MANDO_A, MANDO_B])
    e2.correr(DESTELLOS_DEGRADADO * (DESTELLO_ON_MS + DESTELLO_OFF_MS) + 1000)
    verificar(e2.degradado.gobierna_luz() and not e2.mando.ambar_local,
              "Control: con las condiciones intactas, A.B.A.B sobre un ambar vigente SI entra "
              "al Modo Degradado. El caso de arriba se debe al cambio de condiciones durante "
              "los destellos, no a que la secuencia no funcione.",
              "A.B.A.B no entra al Degradado ni con las condiciones cumplidas (estado=%s): "
              "el escenario anterior no probaria lo que dice" % e2.degradado.estado)

    print("\n-- 1.5 ¿Puede el Maestro quedarse esperando el ACK para siempre? --")
    e = preparar_nodo()
    e.secuencia([MANDO_B, MANDO_B, MANDO_B])
    e.correr(DESTELLOS_AMBAR * (DESTELLO_ON_MS + DESTELLO_OFF_MS) + 1000)
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

    # Control negativo: sin el contador de reintentos, ¿lo detectaria la prueba?
    e2 = preparar_nodo()
    e2.secuencia([MANDO_B, MANDO_B, MANDO_B])
    e2.correr(DESTELLOS_AMBAR * (DESTELLO_ON_MS + DESTELLO_OFF_MS) + 1000)
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

    print("\n-- 1.6 A.A.A revoca el ambar y devuelve el mando al Maestro --")
    e = preparar_nodo()
    e.secuencia([MANDO_B, MANDO_B, MANDO_B])
    e.correr(DESTELLOS_AMBAR * (DESTELLO_ON_MS + DESTELLO_OFF_MS) + 1000)
    e.secuencia([MANDO_A, MANDO_A, MANDO_A])
    e.correr(DESTELLOS_OBEDECER * (DESTELLO_ON_MS + DESTELLO_OFF_MS) + 1000)
    latch_bajado = not e.mando.ambar_local
    e.rx.append((CMD["CMD_GO_GREEN"], 0))
    e.correr(AMARILLO_A_VERDE_MS + 2000)
    verificar(latch_bajado and e.verde_encendido(),
              "A.A.A revoca el ambar y la siguiente orden de verde del Maestro se obedece: "
              "el nodo vuelve a estar bajo mando.",
              "Tras A.A.A el nodo no volvio a obedecer (latch=%s, verde=%s)"
              % (e.mando.ambar_local, e.verde_encendido()))
