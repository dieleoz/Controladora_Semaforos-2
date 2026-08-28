# ===== banco/packs/esclavo_02_inhibicion_menu.py =====
#
# INHIBICION DEL MANDO CON EL MENU ABIERTO Y REGRESO AUTOMATICO
#
# Con el menu abierto en la pantalla del gabinete, el mando de reles desde el suelo
# queda inhibido: dos personas dando ordenes contrarias a la vez es peor que
# cualquiera de las dos ordenes. El regreso automatico rearma el mando.

from banco.modelos.esclavo import *          # noqa: F401,F403
from banco.modelos.esclavo import (          # noqa: F401
    CMD, Esclavo, preparar_nodo, _llevar_a,
)

# EJERCE SFTY-21: el mando queda inhibido con el menu abierto.

NOMBRE = "esclavo_02_inhibicion_menu"
DESCRIPCION = "el mando queda inhibido con el menu abierto, y el regreso lo rearma"


def correr(b, fw):
    verificar = b.verificar
    propiedad = b.propiedad
    hallazgo = b.reportar
    titulo = b.titulo
    b.titulo("INHIBICION DEL MANDO CON EL MENU ABIERTO Y REGRESO AUTOMATICO")

    titulo("2. INHIBICION DEL MANDO CON EL MENU ABIERTO Y REGRESO A LOS %d s"
           % (INACTIVIDAD_MS // 1000))

    print("\n-- 2.1 Todas las pantallas por debajo del listado inhiben el mando --")
    # Se recorren TODOS los caminos que llevan a una pantalla, no una muestra:
    # el listado tiene 2 opciones y por debajo hay 4 pantallas alcanzables.
    caminos = {
        "P_ESTADO": [2],
        "P_DEGRADADO": [1, 2],
        "P_CONFIRMAR": [1, 2, 2],
        "P_RECHAZO": None,      # se alcanza confirmando con las condiciones sin cumplir
    }
    fallos_inhibicion = []
    for pantalla, camino in caminos.items():
        e = preparar_nodo()
        if pantalla == "P_RECHAZO":
            e.reloj_en_hora = False       # el Degradado se rechazara
            for b in [1, 2, 2, 2]:
                e.pulsar(b)
                e.correr(100)
        else:
            for b in camino:
                e.pulsar(b)
                e.correr(100)
        if e.menu.pantalla != pantalla:
            fallos_inhibicion.append(("no se alcanzo", pantalla, e.menu.pantalla))
            continue
        if not e.menu.esta_abierto():
            fallos_inhibicion.append(("no inhibe", pantalla))
        # Con la pantalla abierta, el mando no puede reconocer NADA.
        e.secuencia([MANDO_B, MANDO_B, MANDO_B])
        e.correr(3000)
        if e.mando.ambar_local:
            fallos_inhibicion.append(("reconocio B.B.B con la pantalla abierta", pantalla))

    verificar(not fallos_inhibicion,
              "Las 4 pantallas por debajo del listado inhiben el mando: con cualquiera de "
              "ellas abierta, B.B.B no se reconoce.",
              "Fallos de inhibicion: %s" % fallos_inhibicion)

    print("\n-- 2.2 En el listado el mando SI funciona (es el estado de reposo) --")
    e = preparar_nodo()
    verificar(not e.menu.esta_abierto(), "El listado inicial no cuenta como menu abierto.",
              "El listado inicial inhibe el mando, que quedaria mudo SIEMPRE")
    e.secuencia([MANDO_B, MANDO_B, MANDO_B])
    e.correr(DESTELLOS_AMBAR * (DESTELLO_ON_MS + DESTELLO_OFF_MS) + 1000)
    verificar(e.mando.ambar_local,
              "Desde el listado, B.B.B se reconoce y arma el ambar.",
              "B.B.B no funciona ni siquiera desde el listado")

    print("\n-- 2.3 Regreso automatico: barrido del instante exacto --")
    # Se barre el tiempo de espera alrededor del umbral en pasos de 1 s, en las
    # tres pantallas navegables. Lo que importa no es que vuelva "mas o menos":
    # es que NO vuelva antes -el tecnico esta leyendo- y que SI vuelva despues.
    errores_umbral = []
    for pantalla, camino in (("P_ESTADO", [2]), ("P_DEGRADADO", [1, 2]), ("P_CONFIRMAR", [1, 2, 2])):
        for espera in range(INACTIVIDAD_MS - 5000, INACTIVIDAD_MS + 5001, 1000):
            e = preparar_nodo()
            for b in camino:
                e.pulsar(b)
                e.correr(100)
            e.correr(espera, paso=250)
            abierto = e.menu.esta_abierto()
            # El instante exacto depende del paso del bucle; se admite el margen
            # de una vuelta, no mas.
            if espera < INACTIVIDAD_MS - 500 and not abierto:
                errores_umbral.append(("volvio antes de tiempo", pantalla, espera))
            if espera > INACTIVIDAD_MS + 500 and abierto:
                errores_umbral.append(("no volvio", pantalla, espera))
    verificar(not errores_umbral,
              "Barrido de %d s a %d s en las 3 pantallas navegables: ninguna vuelve antes de "
              "los %d s y todas vuelven despues."
              % ((INACTIVIDAD_MS - 5000) // 1000, (INACTIVIDAD_MS + 5000) // 1000,
                 INACTIVIDAD_MS // 1000),
              "Errores en el umbral: %s" % errores_umbral[:5])

    print("\n-- 2.4 El cartel de rechazo tambien caduca y no ancla la pantalla --")
    e = preparar_nodo()
    e.reloj_en_hora = False
    for b in [1, 2, 2, 2]:
        e.pulsar(b)
        e.correr(100)
    assert e.menu.pantalla == "P_RECHAZO"
    e.correr(RECHAZO_MS + 1000, paso=100)
    volvio_a_degradado = e.menu.pantalla == "P_DEGRADADO"
    e.correr(INACTIVIDAD_MS + 2000, paso=250)
    verificar(volvio_a_degradado and not e.menu.esta_abierto(),
              "El cartel de rechazo caduca a los %d s y la cuenta de inactividad sigue "
              "corriendo por debajo: el mando se rearma igual." % (RECHAZO_MS // 1000),
              "El cartel de rechazo dejo la pantalla anclada (pantalla=%s)" % e.menu.pantalla)

    print("\n-- 2.5 ATAQUE: el operario del suelo insiste con el mando --")
    # El rele va EN PARALELO con los pulsadores: el firmware no distingue un dedo
    # de un rele (mando.h lo dice). Asi que los pulsos que el mando manda desde el
    # suelo TAMBIEN cuentan como "actividad" y refrescan tUltimaPulsacion.
    #
    # Se barre cada cuanto insiste el operario. Si insiste con periodo menor que
    # la inactividad, el regreso automatico NUNCA llega.
    periodos_muertos = []
    for periodo_s in range(10, 130, 10):
        e = preparar_nodo()
        for b in [1, 2]:            # deja abierta la pantalla del Degradado
            e.pulsar(b)
            e.correr(100)
        # El operario baja del gabinete y prueba el mando cada `periodo_s`.
        for _ in range(8):
            e.correr(periodo_s * 1000, paso=500)
            e.secuencia([MANDO_A, MANDO_A, MANDO_A], separacion=2000)
        if e.menu.esta_abierto():
            periodos_muertos.append(periodo_s)
    if periodos_muertos:
        hallazgo(
            "Intentar usar el mando IMPIDE el regreso automatico que lo rearmaria",
            ["Reproduccion: se deja abierta la pantalla del Modo Degradado, se baja del",
             "gabinete y se acciona el mando cada %s s." % periodos_muertos,
             "Los pulsos del rele entran por los MISMOS pines que los pulsadores, asi que",
             "menu_loop() los cuenta como 'hay alguien tocando la pantalla' y reinicia la",
             "cuenta de inactividad en cada intento.",
             "Con cualquier periodo por debajo de %d s el listado no vuelve nunca y el mando"
             % (INACTIVIDAD_MS // 1000),
             "sigue inhibido indefinidamente, que es justo lo que el regreso automatico",
             "existe para impedir.",
             "El operario NO tiene forma de saberlo desde el suelo: aqui el menu no detiene",
             "el ciclo, asi que 'las luces ciclan' no dice nada sobre la pantalla.",
             "La salida es contraintuitiva: dejar de pulsar %d s seguidos."
             % (INACTIVIDAD_MS // 1000),
             "Nota: la premisa de mando.cpp -'estar ahi significa que hay una persona",
             "delante del gabinete'- no se sostiene cuando los pulsos vienen del rele."])
    # La otra mitad del barrido: por encima de la ventana de inactividad el mando
    # SI se rearma. Sin esto, "no vuelve nunca" podria significar simplemente que
    # el modelo nunca vuelve, y el hallazgo no valdria nada.
    verificar(120 not in periodos_muertos and 100 > INACTIVIDAD_MS // 1000,
              "Insistiendo con un periodo MAYOR que los %d s de inactividad, el listado si "
              "vuelve y el mando se rearma: el barrido distingue los dos regimenes."
              % (INACTIVIDAD_MS // 1000),
              "El mando tampoco se rearma esperando mas que la ventana de inactividad: el "
              "barrido no distingue nada (periodos muertos: %s)" % periodos_muertos)

    # Control negativo: con la inhibicion desactivada, el mismo escenario SI
    # deberia armar el ambar. Demuestra que 2.1 mide la inhibicion y no otra cosa.
    e = preparar_nodo()
    e.mando.inhibicion_activa = False
    for b in [1, 2]:
        e.pulsar(b)
        e.correr(100)
    e.secuencia([MANDO_B, MANDO_B, MANDO_B])
    e.correr(DESTELLOS_AMBAR * (DESTELLO_ON_MS + DESTELLO_OFF_MS) + 1000)
    verificar(e.mando.ambar_local,
              "Control negativo: sin la inhibicion, B.B.B con la pantalla abierta SI arma el "
              "ambar. La prueba 2.1 distingue las dos versiones.",
              "El control negativo no arma el ambar: la prueba 2.1 no mide la inhibicion")
