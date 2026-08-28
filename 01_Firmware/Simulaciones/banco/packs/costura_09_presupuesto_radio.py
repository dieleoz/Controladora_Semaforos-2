# ===== banco/packs/costura_09_presupuesto_radio.py =====
#
# EJERCE SFTY-6: el techo de silencio tiene que caber por encima de los reintentos.
#
# N-71. El 27/08 llego un reporte de campo: el cruce se va a Modo Degradado "cada nada
# cuando llueve". Al medir el codigo aparecio que no hacia falta la lluvia para que el
# mecanismo de recuperacion no sirviera de nada.
#
# LA RELACION QUE NADIE VIGILABA.
#
# SFTY6_SILENCIO_MS no es un numero independiente: es el TECHO de todo el presupuesto de
# radio, porque quien primero llegue manda. Si el ciclo tarda mas en agotar sus
# reintentos que lo que el fallback de orfandad aguanta, el ambar salta ANTES y los
# ultimos reintentos no se ejecutan jamas.
#
# Eso era exactamente lo que pasaba:
#
#     coordinador.cpp:  5 reintentos x TIMEOUT_ACK_MS (3500 ms)  = 17,5 s
#     + cadencia del latido (lo mas tarde que arranca)           =  3,0 s
#     ---------------------------------------------------------------------
#     peor caso                                                  = 20,5 s
#     SFTY6_SILENCIO_MS                                          = 12,0 s   <-- TECHO
#
# Los reintentos 4 y 5 eran CODIGO MUERTO dentro del mecanismo de recuperacion. Y el
# comentario que los acompanaba decia "Fallo tras 5 reintentos (12.5s)": cierto con el
# TIMEOUT_ACK_MS de 2500 ms que se retiro el 31/07/2026, falso desde entonces.
#
# POR QUE ESTO NO LO CAZABA NADA.
#
# Los tres numeros viven en ficheros distintos y su relacion solo estaba escrita en
# PROSA, dentro de un comentario. Un comentario no falla cuando alguien cambia un
# numero: se queda ahi, describiendo un equipo que ya no existe -y encima con la
# autoridad de una cuenta hecha-. Este pack recalcula la desigualdad desde las
# constantes del C++ en cada corrida, que es la unica forma de que envejezca bien.
#
# Y ES BIDIRECCIONAL A PROPOSITO. No basta con que el techo sea alto: un techo
# desproporcionado tambien es un defecto, porque es tiempo en el que una punta sigue con
# la configuracion vieja creyendo que la otra la acompana. Se exige que quepa el peor
# caso Y que no lo desborde sin sentido.

import re

NOMBRE = "costura_09_presupuesto_radio"
DESCRIPCION = "el techo de orfandad de SFTY-6 cabe por encima del peor caso de reintentos, y no muy por encima"

COORD = ("Maestro", "src", "coordinador.cpp")
PROTO = ("Maestro", "include", "protocolo.h")

# Cuanto puede tardar el envio de un intercambio completo antes de quedarse esperando.
# Esta medido en el comentario de coordinador.cpp -4 tramas x 3 copias de rafaga a 9600
# baudios, mas la conmutacion del MAX485- y es el UNICO valor de este pack que no se lee
# del C++, porque no existe como constante: es tiempo de cable, no de programa.
# Se anota aqui, a la vista, en vez de esconderlo dentro de una suma.
ENVIO_S = 0.06


def _num(fw, partes, patron, que):
    m = re.search(patron, fw.codigo(*partes))
    if m is None:
        raise fw.Abortado(
            "no se encuentra %s en %s. Sin ese numero este pack no puede calcular el "
            "presupuesto, y una suma con un sumando inventado aprueba cualquier cosa"
            % (que, "/".join(partes)))
    return int(m.group(1))


def correr(b, fw):
    b.titulo("Presupuesto de radio: los reintentos tienen que caber bajo el techo")

    techo_ms = _num(fw, PROTO, r"#define\s+SFTY6_SILENCIO_MS\s+(\d+)UL",
                    "SFTY6_SILENCIO_MS")
    timeout_ms = _num(fw, COORD, r"TIMEOUT_ACK_MS\s*=\s*(\d+)", "TIMEOUT_ACK_MS")
    latido_ms = _num(fw, COORD, r"LATIDO_MS\s*=\s*(\d+)", "LATIDO_MS")
    reintentos = _num(fw, COORD, r"CICLO_MAX_REINTENTOS\s*=\s*(\d+)",
                      "CICLO_MAX_REINTENTOS")
    sync_intentos = _num(fw, COORD, r"SYNC_MAX_INTENTOS\s*=\s*(\d+)",
                         "SYNC_MAX_INTENTOS")

    techo = techo_ms / 1000.0
    paso = timeout_ms / 1000.0 + ENVIO_S
    peor_ciclo = latido_ms / 1000.0 + reintentos * paso
    peor_sync = latido_ms / 1000.0 + sync_intentos * paso

    b.verificar(
        True,
        "leido del C++: techo %.1f s | latido %.1f s | timeout %.1f s | %d reintentos "
        "de ciclo | %d intentos de sync" % (techo, latido_ms / 1000.0,
                                            timeout_ms / 1000.0, reintentos,
                                            sync_intentos),
        "no deberia llegarse aqui")

    # ---- 1. El ciclo tiene que poder agotar SUS reintentos ----
    # Es la propiedad que estaba rota. Un reintento que no se alcanza no es una
    # precaucion de mas: es la diferencia entre recuperar el enlace y no recuperarlo.
    b.verificar(
        peor_ciclo <= techo,
        "los %d reintentos del ciclo caben bajo el techo: %.1f s de peor caso contra "
        "%.1f s, margen %.1f s" % (reintentos, peor_ciclo, techo, techo - peor_ciclo),
        "el peor caso del ciclo son %.1f s (%.1f s de cadencia de latido + %d x %.2f s) "
        "y el techo de orfandad son %.1f s. El ambar por orfandad salta ANTES de que el "
        "ciclo agote sus reintentos: los ultimos NO SE EJECUTAN NUNCA, y ninguna prueba "
        "lo delata porque el equipo hace algo razonable -irse a ambar-. Es la forma de "
        "N-71: una racha de perdidas que los reintentos habrian recuperado se convierte "
        "en un ambar espurio"
        % (peor_ciclo, latido_ms / 1000.0, reintentos, paso, techo))

    # ---- 2. La sincronizacion horaria, igual ----
    # Con la diferencia de que aqui pasarse NO es un ambar perdido sino un ambar
    # PROVOCADO: el intercambio calla el latido, asi que un sync largo se parece a un
    # enlace muerto visto desde el temporizador de orfandad.
    b.verificar(
        peor_sync <= techo,
        "los %d intentos de sincronizacion caben: %.1f s contra %.1f s"
        % (sync_intentos, peor_sync, techo),
        "el intercambio de sincronizacion puede durar %.1f s y el techo son %.1f s. "
        "Durante ese intercambio se suprime el latido (SFTY-13), asi que un sync que se "
        "pase del techo se lee desde el otro lado como un enlace muerto: una "
        "sincronizacion fallida se convierte en una caida de enlace FALSA y manda el "
        "cruce a ambar sin motivo" % (peor_sync, techo))

    # ---- 3. Y el techo no puede ser desproporcionado ----
    # La direccion contraria. El techo es el tiempo que una punta sigue con la
    # configuracion vieja creyendo que la otra la acompana; subirlo "por si acaso" hasta
    # que todo quepa holgadamente convierte una regla de seguridad en un adorno.
    b.verificar(
        techo <= peor_ciclo * 2.0,
        "el techo (%.1f s) no desborda al peor caso (%.1f s): cubre el presupuesto sin "
        "convertirse en una espera indefinida" % (techo, peor_ciclo),
        "el techo son %.1f s para un peor caso de %.1f s: mas del doble. Ese tiempo de "
        "mas no compra ninguna recuperacion -los reintentos ya se agotaron- y es tiempo "
        "en el que una punta sigue el ciclo con la otra posiblemente muerta"
        % (techo, peor_ciclo))

    # ---- 4. Ningun literal suelto de los que forman el presupuesto ----
    # Los tres numeros tienen que tener nombre para que este pack pueda leerlos. Un
    # literal desnudo no se puede vigilar: es como estaba el 3000 del latido.
    codigo = fw.codigo(*COORD)
    for literal, quien in ((str(latido_ms), "la cadencia del latido"),
                           (str(reintentos), "el numero de reintentos del ciclo")):
        sueltos = re.findall(r"[<>]=?\s*%s\b" % literal, codigo)
        b.verificar(
            not sueltos,
            "%s no aparece como literal desnudo en una comparacion" % quien,
            "%s vuelve a estar escrita como el literal %s dentro de una comparacion de "
            "coordinador.cpp. Un numero sin nombre no lo puede leer este pack, asi que "
            "el presupuesto dejaria de vigilarse en silencio" % (quien, literal))

    # ---- 5. Controles negativos ----
    b.control_negativo(
        (3.0 + 5 * 3.56) > 12.0,
        "la cuenta del presupuesto detecta el caso roto que motivo N-71: 5 reintentos "
        "de 3,5 s no caben bajo un techo de 12 s")
    # Este control ya nacio roto una vez, y se deja anotado: la primera version
    # buscaba el patron dentro de la cadena "// TIMEOUT_ACK_MS = 9999" y esperaba no
    # hallarlo. Lo hallaba, claro -la regex no sabe de comentarios-. Quien los quita es
    # fw.codigo(), asi que lo que hay que demostrar es que ESE filtro esta actuando
    # sobre este fichero, no que una expresion regular haga algo que no hace.
    b.control_negativo(
        "//" in fw.texto(*COORD) and "//" not in codigo,
        "las constantes se leen del fuente SIN comentarios: coordinador.cpp los tiene y "
        "el texto que este pack analiza no, asi que un numero citado en una nota no "
        "puede colarse como si fuera codigo")
    b.control_negativo(
        bool(re.search(r"[<>]=?\s*3000\b", "if (millis() - tUltimoPing > 3000)")) and
        not re.search(r"[<>]=?\s*3000\b", "if (millis() - tUltimoPing > LATIDO_MS)"),
        "el detector de literales desnudos distingue el numero de la constante")
