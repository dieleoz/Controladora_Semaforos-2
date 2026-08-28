# ===== banco/packs/esclavo_04_desfase.py =====
#
# ARITMETICA CIRCULAR DEL DESFASE - barrido de las 3.600 combinaciones
#
# El desfase entre relojes se calcula en aritmetica circular sobre el minuto. Se
# barren LAS 3.600 combinaciones y no una muestra: los fallos de aritmetica
# circular viven justo en el salto de 59 a 0, que un muestreo se salta.

from banco.modelos.esclavo import *          # noqa: F401,F403
from banco.modelos.esclavo import (          # noqa: F401
    CMD, Esclavo, preparar_nodo, _llevar_a,
)

# EJERCE SFTY-23: la aritmetica circular del desfase que reporta CMD_DELTA_RESP.

NOMBRE = "esclavo_04_desfase"
DESCRIPCION = "aritmetica circular del desfase: barrido de las 3.600 combinaciones"


def correr(b, fw):
    verificar = b.verificar
    propiedad = b.propiedad
    hallazgo = b.reportar
    titulo = b.titulo
    b.titulo("ARITMETICA CIRCULAR DEL DESFASE - barrido de las 3.600 combinaciones")

    titulo("4. ARITMETICA CIRCULAR DEL DESFASE — barrido de las 3.600 combinaciones")

    e = Esclavo()
    e.reloj_en_hora = True
    fuera_de_rango = []
    incongruentes = []
    for seg_esclavo in range(60):
        e.reloj_s = seg_esclavo
        for seg_maestro in range(60):
            d = e._calcular_desfase(seg_maestro)
            if d < -30 or d > 30:
                fuera_de_rango.append((seg_maestro, seg_esclavo, d))
            # El resultado tiene que seguir describiendo la MISMA diferencia
            # sobre el circulo de 60 s: si no, el numero seria bonito pero falso.
            if (d - (seg_maestro - seg_esclavo)) % 60 != 0:
                incongruentes.append((seg_maestro, seg_esclavo, d))

    verificar(not fuera_de_rango,
              "Barrido completo de 60x60 = 3.600 combinaciones: el desfase cae SIEMPRE en "
              "[-30, +30]. Nunca da la vuelta.",
              "Hay %d combinaciones fuera de +-30: %s" % (len(fuera_de_rango), fuera_de_rango[:5]))
    verificar(not incongruentes,
              "Las 3.600 combinaciones conservan la diferencia real modulo 60: el camino corto "
              "del circulo, no un numero recortado.",
              "Hay %d resultados que no describen la diferencia real: %s"
              % (len(incongruentes), incongruentes[:5]))

    # Signo: positivo = el Maestro va por delante = el Esclavo esta atrasado.
    e.reloj_s = 59
    verificar(e._calcular_desfase(1) == 2,
              "Maestro en el segundo 1 y Esclavo en el 59 -> +2 s (el Esclavo va atrasado), "
              "no -58 s. Es el caso que motiva toda la correccion circular.",
              "El cruce de minuto da %d en vez de +2" % e._calcular_desfase(1))

    print("\n-- Control negativo: la resta cruda NO pasaria este barrido --")
    def desfase_ingenuo(seg_maestro, seg_esclavo):
        return seg_maestro - seg_esclavo
    malos = [(m, s) for s in range(60) for m in range(60)
             if not (-30 <= desfase_ingenuo(m, s) <= 30)]
    verificar(len(malos) > 0,
              "La resta cruda se sale de +-30 en %d de las 3.600 combinaciones, asi que el "
              "barrido SI distingue la aritmetica correcta de la ingenua." % len(malos),
              "El barrido da PASS tambien a la resta cruda: no esta midiendo nada")

    print("\n-- Sin reloj fiable o con parametro corrupto: fuera de rango, no un numero --")
    e2 = Esclavo()
    e2.reloj_en_hora = False
    sin_hora = e2._calcular_desfase(30)
    e3 = Esclavo()
    e3.reloj_en_hora = True
    corruptos = [e3._calcular_desfase(p) for p in range(60, 256)]
    verificar(sin_hora == DELTA_FUERA_DE_RANGO and all(c == DELTA_FUERA_DE_RANGO for c in corruptos),
              "Sin reloj en hora, y con los 196 valores de param imposibles (60..255), se "
              "responde DELTA_FUERA_DE_RANGO en vez de un numero inventado que el operario "
              "leeria como un desfase real.",
              "Se devolvio un desfase con reloj no fiable (%s) o con param corrupto" % sin_hora)
