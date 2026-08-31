# ===== banco/packs/esclavo_05_hora_atomica.py =====
#
# APLICACION ATOMICA DE LA HORA (SFTY-23)
#
# EJERCE SFTY-23: la hora que llega por radio se escribe ENTERA o no se escribe -las
# 8 combinaciones de H/M/S, la caducidad de la foto, el reenvio de los segundos y el
# rango de las tres cifras-, y solo la terna aplicada rearma la cuenta de sincronia.
#
# La etiqueta faltaba desde que se escribio el pack, asi que estas comprobaciones no
# aparecian en la tercera columna de OPTIMIZACIONES.md, que se levanta buscandola.
#
# La hora viaja en tramas separadas. Si se aplicara pieza a pieza, un instante con
# la hora nueva y los minutos viejos daria un reloj que nadie puso -y sobre ese
# reloj se decide el Modo Degradado-. O entra entera, o no entra.

from banco.modelos.esclavo import *          # noqa: F401,F403
from banco.modelos.esclavo import (          # noqa: F401
    CMD, Esclavo, preparar_nodo, _llevar_a,
)

NOMBRE = "esclavo_05_hora_atomica"
DESCRIPCION = "la hora se aplica entera o no se aplica (SFTY-23)"


def correr(b, fw):
    verificar = b.verificar
    propiedad = b.propiedad
    hallazgo = b.reportar
    titulo = b.titulo
    b.titulo("APLICACION ATOMICA DE LA HORA (SFTY-23)")

    titulo("5. APLICACION ATOMICA DE LA HORA (SFTY-23)")

    def nodo_sin_hora():
        e = Esclavo()
        e.reloj_en_hora = False
        e.reloj_h, e.reloj_m, e.reloj_s = 0, 0, 0
        e.horas_aplicadas = []
        return e

    print("\n-- 5.1 La terna completa se aplica y se acusa --")
    e = nodo_sin_hora()
    for c, p in ((CMD["CMD_HORA_H"], 14), (CMD["CMD_HORA_M"], 32), (CMD["CMD_HORA_S"], 7)):
        e.rx.append((c, p))
        e.correr(50)
    e.correr(RETARDO_RESPUESTA_MS + 100)
    verificar(e.horas_aplicadas == [(14, 32, 7)] and
              CMD["CMD_ACK_HORA"] in [c for (_, c, _) in e.tx],
              "La terna completa se aplica de una vez y se acusa con CMD_ACK_HORA.",
              "La terna completa no se aplico o no se acuso (aplicadas=%s)" % e.horas_aplicadas)

    print("\n-- 5.2 Segundos sueltos: ni se aplican ni se acusan --")
    e = nodo_sin_hora()
    for _ in range(5):
        e.rx.append((CMD["CMD_HORA_S"], 7))
        e.correr(500)
    verificar(not e.horas_aplicadas and CMD["CMD_ACK_HORA"] not in [c for (_, c, _) in e.tx],
              "Cinco tramas de segundos sueltas seguidas no tocan el reloj y no se contestan: "
              "el silencio es la respuesta correcta a una orden incompleta.",
              "Unos segundos sueltos llegaron a tocar el reloj: %s" % e.horas_aplicadas)

    print("\n-- 5.3 Ternas incompletas: barrido de las 8 combinaciones --")
    # Se prueban TODOS los subconjuntos de {H, M, S}: solo el completo puede
    # escribir el RTC.
    errores = []
    for mascara in range(8):
        e = nodo_sin_hora()
        if mascara & 1:
            e.rx.append((CMD["CMD_HORA_H"], 14))
            e.correr(50)
        if mascara & 2:
            e.rx.append((CMD["CMD_HORA_M"], 32))
            e.correr(50)
        if mascara & 4:
            e.rx.append((CMD["CMD_HORA_S"], 7))
            e.correr(50)
        e.correr(RETARDO_RESPUESTA_MS + 100)
        aplico = bool(e.horas_aplicadas)
        acuso = CMD["CMD_ACK_HORA"] in [c for (_, c, _) in e.tx]
        completa = (mascara == 7)
        if aplico != completa or acuso != completa:
            errores.append((mascara, aplico, acuso))
    verificar(not errores,
              "Las 8 combinaciones de tramas H/M/S: SOLO la terna completa escribe el reloj y "
              "SOLO ella se acusa. Nunca queda una hora a medias.",
              "Combinaciones que se comportan mal (mascara, aplico, acuso): %s" % errores)

    print("\n-- 5.4 Foto caducada: barrido del retardo entre el minuto y los segundos --")
    # Hora y minuto son una FOTO del reloj del Maestro. Cuanto mas envejecen,
    # mas se alejan de la verdad; el caso feo es el cambio de minuto. Se barre el
    # retardo en pasos de 100 ms alrededor de la ventana.
    errores = []
    for retardo in range(0, VENTANA_HORA_MS + 2001, 100):
        e = nodo_sin_hora()
        e.rx.append((CMD["CMD_HORA_H"], 14))
        e.correr(50)
        e.rx.append((CMD["CMD_HORA_M"], 32))
        e.correr(50)
        e.correr(retardo, paso=50)
        e.rx.append((CMD["CMD_HORA_S"], 7))
        e.correr(200)
        aplico = bool(e.horas_aplicadas)
        # La ventana cuenta desde la PRIMERA cifra (la hora), asi que al retardo
        # hay que sumarle lo que tardo la trama de minutos.
        edad = retardo + 50
        if edad <= VENTANA_HORA_MS - 200 and not aplico:
            errores.append(("descarto una foto fresca", retardo))
        if edad >= VENTANA_HORA_MS + 200 and aplico:
            errores.append(("aplico una foto caducada", retardo))
    verificar(not errores,
              "Barrido de 0 a %d ms de retardo: la terna se aplica mientras la foto tiene "
              "menos de %d ms y se descarta despues. La ventana cuenta desde la PRIMERA cifra, "
              "que es lo que acota cuanto puede envejecer."
              % (VENTANA_HORA_MS + 2000, VENTANA_HORA_MS),
              "Errores en la caducidad de la foto: %s" % errores[:5])

    print("\n-- 5.5 Terna repetida: los segundos reenviados no se aplican dos veces --")
    # Si el ACK se perdiera y el Maestro reenviara solo los segundos, aplicarlos
    # sobre una hora y un minuto viejos meteria un minuto entero de error justo
    # en un cambio de minuto.
    e = nodo_sin_hora()
    for c, p in ((CMD["CMD_HORA_H"], 10), (CMD["CMD_HORA_M"], 59), (CMD["CMD_HORA_S"], 58)):
        e.rx.append((c, p))
        e.correr(50)
    e.correr(500)
    e.rx.append((CMD["CMD_HORA_S"], 3))     # reenvio de los segundos, ya en las 11:00
    e.correr(500)
    verificar(e.horas_aplicadas == [(10, 59, 58)],
              "Un reenvio de la trama de segundos NO se aplica sobre la hora y el minuto ya "
              "consumidos: el buffer se vacia se haya aplicado o no. Sin eso, el reloj se "
              "quedaria en 10:59:03, un minuto entero atrasado.",
              "El reenvio de segundos volvio a escribir el reloj: %s" % e.horas_aplicadas)

    print("\n-- 5.6 Valores imposibles en las tres cifras --")
    # La trama viene de la RADIO: un paquete corrupto que cuele por el CRC puede
    # traer 0xFF. Se barren TODOS los valores de cada cifra.
    errores = []
    for h in range(256):
        e = nodo_sin_hora()
        e.rx.append((CMD["CMD_HORA_H"], h))
        e.correr(50)
        e.rx.append((CMD["CMD_HORA_M"], 30))
        e.correr(50)
        e.rx.append((CMD["CMD_HORA_S"], 30))
        e.correr(200)
        if bool(e.horas_aplicadas) != (h <= 23):
            errores.append(("hora", h))
    for m in range(256):
        e = nodo_sin_hora()
        e.rx.append((CMD["CMD_HORA_H"], 12))
        e.correr(50)
        e.rx.append((CMD["CMD_HORA_M"], m))
        e.correr(50)
        e.rx.append((CMD["CMD_HORA_S"], 30))
        e.correr(200)
        if bool(e.horas_aplicadas) != (m <= 59):
            errores.append(("minuto", m))
    for s in range(256):
        e = nodo_sin_hora()
        e.rx.append((CMD["CMD_HORA_H"], 12))
        e.correr(50)
        e.rx.append((CMD["CMD_HORA_M"], 30))
        e.correr(50)
        e.rx.append((CMD["CMD_HORA_S"], s))
        e.correr(200)
        if bool(e.horas_aplicadas) != (s <= 59):
            errores.append(("segundo", s))
    verificar(not errores,
              "Barrido de los 256 valores posibles de cada cifra (768 casos): solo se escribe "
              "el reloj con 0..23 / 0..59 / 0..59. Una trama corrupta que cuele por el CRC no "
              "puede poner el reloj en hora falsa.",
              "Valores imposibles aceptados: %s" % errores[:8])

    print("\n-- 5.7 Aplicar la hora es lo unico que reinicia el limite de 48 h --")
    # Oir tramas no demuestra que el reloj coincida con el del Maestro. Se
    # comprueba que el trafico de servicio NO rearma la cuenta.
    e = preparar_nodo()
    e.correr(60000, paso=1000)
    t_sync_antes = e.degradado.tUltimaSync
    for c, p in ((CMD["CMD_PING"], 0), (CMD["CMD_DELTA"], 30),
                 (CMD["CMD_CONFIG_VERDE"], 30), (CMD["CMD_CONFIG_DESPEJE"], 30),
                 (CMD["CMD_HORA_H"], 12), (CMD["CMD_HORA_M"], 0)):
        e.rx.append((c, p))
        e.correr(300)
    sin_cambio = e.degradado.tUltimaSync == t_sync_antes
    e.rx.append((CMD["CMD_HORA_S"], 0))
    e.correr(300)
    con_cambio = e.degradado.tUltimaSync > t_sync_antes
    verificar(sin_cambio and con_cambio,
              "Ni el PING, ni el DELTA, ni la configuracion, ni media terna de hora reinician "
              "la cuenta de las 48 h; solo la terna aplicada lo hace.",
              "El trafico de servicio rearmo el limite duro (antes=%s, ahora=%s)"
              % (t_sync_antes, e.degradado.tUltimaSync))
