# ===== banco/packs/esclavo_03_par_config.py =====
#
# cfgRadioCompleto() — EL PAR VERDE+DESPEJE TIENE QUE SER INDIVISIBLE.
#
# ESTE PACK LLEVA EL UNICO FALLO ABIERTO DEL VALIDADOR DEL ESCLAVO (el 30/31), y por
# eso se migro el primero: extraerlo convierte un fallo enterrado en 1.805 lineas en
# algo que se corre solo, en un segundo, con:
#
#     python banco/correr.py --pack esclavo_03
#
# QUE SE ROMPE. Las banderas cfgVerdeRecibido / cfgDespejeRecibido son PEGAJOSAS: una
# vez puestas no se bajan nunca. cfgRadioCompleto() comprueba que las dos hayan
# llegado ALGUNA VEZ, no que pertenezcan al MISMO envio. Si en una reconfiguracion se
# pierde la trama de VERDE, el Esclavo se queda con el verde de un envio y el despeje
# de otro -una duracion de ciclo que no calculo nadie-, LO ACUSA con CMD_ACK_CONFIG y
# lo GRABA en la pila, de modo que la mezcla sobrevive al corte de energia.
#
# DONDE VIVE EL DEFECTO: verdeDeEsteEnvio() y VENTANA_CONFIG_MS, dentro de
# Esclavo/src/main.cpp. Que la logica de configuracion este enterrada en el punto de
# entrada es justo lo que la Fase 2 del plan de arquitectura va a sacar a
# config_ciclo.cpp.

from banco.modelos.esclavo import (      # noqa: F401
    CMD, SEGUNDOS_DEL_DIA, Esclavo, ciclo_degradado_fase,
)

# EJERCE SFTY-23: el par verde+despeje de la configuracion por radio.

NOMBRE = "esclavo_03_par_config"
DESCRIPCION = "el par verde+despeje no puede mezclarse entre envios (30/31)"


def correr(b, fw):
    # Se enlazan las primitivas del validador viejo a las del contador comun, para
    # poder traer el bloque LITERAL. Reescribir 150 lineas de logica ya probada solo
    # para cambiarles el nombre a las llamadas es como se cuelan los errores en una
    # migracion que se supone que no cambia comportamiento.
    verificar = b.verificar
    propiedad = b.propiedad
    hallazgo = b.reportar
    titulo = b.titulo
    b.titulo("cfgRadioCompleto() - el par verde+despeje tiene que ser INDIVISIBLE")


    print("\n-- 3.1 En una sesion limpia, el par nunca se mezcla con el respaldo --")
    # Barrido de las 4 combinaciones de tramas recibidas, con respaldo cargado.
    mezclas = []
    for llega_verde in (False, True):
        for llega_despeje in (False, True):
            e = Esclavo()
            e.respaldo_verde, e.respaldo_despeje, e.respaldo_hay_ciclo = 60, 20, True
            if llega_verde:
                e.rx.append((CMD["CMD_CONFIG_VERDE"], 45))
                e.correr(100)
            if llega_despeje:
                e.rx.append((CMD["CMD_CONFIG_DESPEJE"], 25))
                e.correr(100)
            par = (e.config_verde_segundos(), e.config_despeje_segundos())
            if par not in ((45, 25), (60, 20)):
                mezclas.append((llega_verde, llega_despeje, par))
    verificar(not mezclas,
              "Con el respaldo cargado, las 4 combinaciones de llegada dan SIEMPRE un par "
              "entero: o el de radio (45,25) o el guardado (60,20). Nunca uno de cada.",
              "Se obtuvieron pares mezclados radio/respaldo: %s" % mezclas)

    print("\n-- 3.2 ATAQUE: segunda configuracion con la trama de VERDE perdida --")
    # Las banderas cfgVerdeRecibido / cfgDespejeRecibido son PEGAJOSAS: una vez
    # puestas no se bajan nunca. cfgRadioCompleto() comprueba que hayan llegado
    # las dos alguna vez, NO que pertenezcan al MISMO envio.
    e = Esclavo()
    e.rx.append((CMD["CMD_CONFIG_VERDE"], 30))     # sesion 1: el par bueno
    e.correr(100)
    e.rx.append((CMD["CMD_CONFIG_DESPEJE"], 20))
    e.correr(500)
    par_1 = (e.config_verde_segundos(), e.config_despeje_segundos())
    acks_1 = [c for (_, c, _) in e.tx if c == CMD["CMD_ACK_CONFIG"]]

    # El operario cambia el ciclo en el Maestro: (45, 25). La trama de VERDE se
    # pierde -ruido, repetidor, CRC- y solo llega la de DESPEJE.
    e.rx.append((CMD["CMD_CONFIG_DESPEJE"], 25))
    e.correr(500)
    par_2 = (e.config_verde_segundos(), e.config_despeje_segundos())
    acks_2 = [c for (_, c, _) in e.tx if c == CMD["CMD_ACK_CONFIG"]]

    mezclado = par_2 == (30, 25)
    acuso = len(acks_2) > len(acks_1)
    respaldo_envenenado = e.respaldo_guardados[-1] == (30, 25)

    if mezclado:
        # ¿Cuanto tarda en notarse? Se compara la fase que calcula cada punta.
        maestro = (45, 25)
        esclavo = par_2
        primer_solape = None
        for s in range(0, SEGUNDOS_DEL_DIA):
            fm = ciclo_degradado_fase(s, *maestro)
            fe = ciclo_degradado_fase(s, *esclavo)
            if fm == "FD_VERDE_ESCLAVO" and fe == "FD_VERDE_ESCLAVO":
                continue
            if (fm == "FD_VERDE_MAESTRO" and fe == "FD_VERDE_ESCLAVO"):
                primer_solape = s
                break
        hallazgo(
            "El par verde+despeje NO es indivisible entre envios: una segunda "
            "configuracion con la trama de VERDE perdida deja un par mezclado Y SE ACUSA",
            ["Reproduccion:",
             "  1. El Maestro configura (30,20). Llegan las dos tramas -> par %s. OK." % (par_1,),
             "  2. El operario cambia el ciclo a (45,25).",
             "  3. La trama CMD_CONFIG_VERDE se pierde; llega solo CMD_CONFIG_DESPEJE=25.",
             "  4. cfgVerdeRecibido sigue en true DESDE EL ENVIO ANTERIOR, asi que",
             "     cfgRadioCompleto() da true y el Esclavo usa el par %s." % (par_2,),
             "     Verde de un envio, despeje de otro: una duracion de ciclo que no",
             "     calculo nadie.",
             "  5. main.cpp contesta CMD_ACK_CONFIG en la rama de DESPEJE SIN comprobar",
             "     que el VERDE de ESTE par llegara: el Maestro da el envio por bueno,",
             "     pone pendConfig=false y NO reintenta. El fallo queda tapado.",
             "  6. respaldo_guardarCiclo() graba %s en la pila, de modo que la mezcla"
             % (e.respaldo_guardados[-1],),
             "     SOBREVIVE al corte de energia y se usaria para reanudar el Degradado.",
             "Consecuencia medida: el Maestro cicla con %s (%d s) y el Esclavo con %s (%d s)."
             % (maestro, 2 * sum(maestro), esclavo, 2 * sum(esclavo)),
             "El primer instante del dia en que el Maestro se da verde mientras el Esclavo",
             "cree tener el suyo aparece en el segundo %s del dia: LAS DOS PUNTAS EN VERDE."
             % (primer_solape if primer_solape is not None else "n/d"),
             "Es exactamente el solape 'en minutos' que ciclo_degradado.h existe para impedir,",
             "y entra por la puerta de al lado: no por el calculo de la fase, sino por la",
             "configuracion con la que se calcula.",
             "Arreglo posible (NO aplicado, esto solo reporta): bajar cfgVerdeRecibido al",
             "recibir CMD_CONFIG_DESPEJE tras acusar, o marcar el par con un numero de",
             "envio, y acusar solo cuando las dos tramas del MISMO par hayan llegado."])
    propiedad(not mezclado,
              "El par no se puede mezclar entre envios distintos.",
              "PROPIEDAD ROTA — el par se mezclo: verde=%d del envio ANTERIOR con despeje=%d "
              "del nuevo. El Esclavo lo acuso (%s) y lo guardo en la pila (%s). Reproducido "
              "arriba; se reporta, no se arregla."
              % (par_2[0], par_2[1], acuso, respaldo_envenenado))

    print("\n-- 3.3 Control negativo del ataque: con el par completo no hay mezcla --")
    e2 = Esclavo()
    for v, d in ((30, 20), (45, 25)):
        e2.rx.append((CMD["CMD_CONFIG_VERDE"], v))
        e2.correr(100)
        e2.rx.append((CMD["CMD_CONFIG_DESPEJE"], d))
        e2.correr(500)
    verificar((e2.config_verde_segundos(), e2.config_despeje_segundos()) == (45, 25),
              "Cuando las dos tramas del par llegan, el Esclavo se queda con el par nuevo "
              "entero: la prueba 3.2 no esta acusando a un mecanismo que falle siempre.",
              "Ni con el par completo se queda el valor correcto")

    print("\n-- 3.4 Primer envio de la vida con el VERDE perdido: se acusa igual --")
    e3 = Esclavo()
    e3.rx.append((CMD["CMD_CONFIG_DESPEJE"], 25))
    e3.correr(500)
    acuso_sin_verde = CMD["CMD_ACK_CONFIG"] in [c for (_, c, _) in e3.tx]
    sabe_el_ciclo = e3.config_verde_recibido()
    if acuso_sin_verde and not sabe_el_ciclo:
        hallazgo(
            "El ACK de configuracion se emite aunque no haya llegado ninguna trama de VERDE",
            ["Reproduccion: equipo recien encendido; llega solo CMD_CONFIG_DESPEJE.",
             "El Esclavo contesta CMD_ACK_CONFIG y el Maestro marca la configuracion como",
             "entregada (pendConfig = false, sin reintentos).",
             "Pero config_verdeRecibido() sigue en false: el Esclavo NO sabe la duracion del",
             "ciclo y rechazara el Modo Degradado con DEG_RECHAZO_SIN_CONFIG.",
             "Este caso cae del lado seguro -el modo no entra- pero deja al Maestro",
             "convencido de que la punta esta configurada, y la pantalla del Maestro no",
             "tiene forma de desmentirlo hasta que alguien intente entrar en Degradado.",
             "Es el mismo defecto de 3.2 visto desde el otro lado: se acusa la ULTIMA trama",
             "del par en vez del par."])
    # Lo que si tiene que cumplirse pase lo que pase: sin saber la duracion del
    # ciclo, el Modo Degradado NO se autoriza. Es la barrera que salva este caso.
    e3.reloj_en_hora = True
    e3.degradado.registrar_sync()
    verificar(e3.degradado.comprobar() == "DEG_RECHAZO_SIN_CONFIG",
              "Aunque el Maestro crea que la configuracion llego, el Esclavo rechaza el Modo "
              "Degradado con DEG_RECHAZO_SIN_CONFIG: sin la duracion del ciclo no entra, que "
              "es el lado correcto en el que caer.",
              "El Esclavo autorizo el Degradado sin haber recibido la duracion del verde "
              "(motivo devuelto: %s)" % e3.degradado.comprobar())

    print("\n-- 3.5 El ciclo a cero nunca se acepta ni se guarda --")
    malos = []
    for v, d in ((0, 30), (30, 0), (0, 0)):
        e4 = Esclavo()
        e4.reloj_en_hora = True
        e4.rx.append((CMD["CMD_CONFIG_VERDE"], v))
        e4.correr(100)
        e4.rx.append((CMD["CMD_CONFIG_DESPEJE"], d))
        e4.correr(500)
        e4.degradado.registrar_sync()
        if e4.degradado.comprobar() == "DEG_ACEPTADO":
            malos.append(("acepto el Degradado con", v, d))
        if e4.respaldo_hay_ciclo:
            malos.append(("guardo en la pila", v, d))
    verificar(not malos,
              "Un ciclo con verde o despeje en cero ni autoriza el Modo Degradado ni llega a "
              "guardarse en la pila.",
              "Configuraciones nulas aceptadas: %s" % malos)
