# ===== banco/packs/costura_03_comandos.py =====
#
# COMANDOS: QUE LO EMITIDO POR UNA PUNTA LO ATIENDA LA OTRA
#
# Un comando que una punta emite y la otra no contempla se pierde en silencio: no
# hay error, no hay aviso, simplemente no pasa nada. Se cotejan las dos tablas.

import re
from banco.modelos.costura import *          # noqa: F401,F403

NOMBRE = "costura_03_comandos"
DESCRIPCION = "que lo que emite una punta lo atienda la otra"


def correr(b, fw):
    verificar = b.verificar

    b.titulo("COMANDOS: QUE LO EMITIDO POR UNA PUNTA LO ATIENDA LA OTRA")

    # Los codigos no pueden repetirse. Ya paso una vez: CMD_ACK_RED colisionaba con
    # CMD_PING y hubo que reasignarlo a 0x06.
    valores = {}
    colisiones = []
    for nombre, valor in CMD.items():
        if valor in valores:
            colisiones.append((valores[valor], nombre, hex(valor)))
        valores[valor] = nombre
    verificar(not colisiones,
              f"los {len(CMD)} codigos de protocolo.h son todos distintos: ningun comando puede "
              "confundirse con otro (la colision CMD_ACK_RED/CMD_PING ya costo una vez)",
              f"COLISION de codigos: {colisiones}")

    # Emitidos por el Maestro: cualquier protocolo_enviarPaquete() censando TODOS los .cpp
    # del directorio src/ del Maestro (sin lista fija escrita a mano).
    emite_maestro = set()
    for f_ in fw.fuentes_de("Maestro", "src"):
        c_ = fw.codigo("Maestro", "src", f_)
        emite_maestro |= set(re.findall(r"protocolo_enviarPaquete\(\s*(CMD_[A-Z_]+)", c_))

    # Atendidos por el Esclavo: sus comparaciones sobre pkt.command censando TODOS los .cpp
    # de src/ del Esclavo.
    atiende_esclavo = set()
    for f_ in fw.fuentes_de("Esclavo", "src"):
        c_ = fw.codigo("Esclavo", "src", f_)
        atiende_esclavo |= set(re.findall(r"pkt(?:\.|->)command\s*==\s*(CMD_[A-Z_]+)", c_))

    # Emitidos por el Esclavo: sus respuestas programadas (SFTY-17) y envios directos
    # censando TODOS los .cpp del directorio src/ del Esclavo (incluyendo demanda.cpp, etc.).
    emite_esclavo = set()
    for f_ in fw.fuentes_de("Esclavo", "src"):
        c_ = fw.codigo("Esclavo", "src", f_)
        emite_esclavo |= set(re.findall(r"programarRespuesta\(\s*(CMD_[A-Z_]+)", c_))
        emite_esclavo |= set(re.findall(r"protocolo_enviarPaquete\(\s*(CMD_[A-Z_]+)", c_))

    # Atendidos por el Maestro: censando todos los .cpp de src/ del Maestro (coordinador.cpp,
    # etc.). Conviven las dos formas de acceso -pkt.command y pkt->command- mas respuestaEsperada.
    atiende_maestro = set()
    for f_ in fw.fuentes_de("Maestro", "src"):
        c_ = fw.codigo("Maestro", "src", f_)
        atiende_maestro |= set(re.findall(r"pkt(?:\.|->)command\s*==\s*(CMD_[A-Z_]+)", c_))
        atiende_maestro |= set(re.findall(r"respuestaEsperada\s*=\s*(CMD_[A-Z_]+)", c_))

    # CMD_PONG es un caso aparte y hay que decirlo en vez de tragarselo: el Maestro
    # lo emite SOLO como respuesta a un CMD_PING entrante, y el Esclavo no emite
    # PING en ningun sitio. Es una rama que nunca se ejecuta, no un comando
    # desatendido. Se comprueba que efectivamente sea inalcanzable; si algun dia el
    # Esclavo empezara a hacer PING, esto dejaria de excusarse y la prueba lo veria.
    texto_coord = fw.codigo("Maestro", "src", "coordinador.cpp")
    pong_solo_responde_a_ping = bool(re.search(
        r"if\s*\(pkt\.command\s*==\s*CMD_PING\)\s*\{\s*\n?\s*protocolo_enviarPaquete\(CMD_PONG\);",
        texto_coord))
    esclavo_nunca_hace_ping = "CMD_PING" not in emite_esclavo
    inalcanzables = set()
    if pong_solo_responde_a_ping and esclavo_nunca_hace_ping:
        inalcanzables.add("CMD_PONG")

    huerfanos_ida = sorted(emite_maestro - atiende_esclavo - inalcanzables)
    huerfanos_vuelta = sorted(emite_esclavo - atiende_maestro)

    verificar(not huerfanos_ida,
              f"los {len(emite_maestro - inalcanzables)} comandos que EMITE el Maestro hacia el "
              f"Esclavo los ATIENDE el Esclavo, uno por uno: "
              f"{sorted(emite_maestro - inalcanzables)}",
              f"el Maestro emite comandos que el Esclavo ignora en silencio: {huerfanos_ida}")

    verificar(inalcanzables == {"CMD_PONG"},
              "CMD_PONG en el Maestro es rama muerta comprobada: solo responde a un CMD_PING "
              "entrante y el Esclavo nunca emite PING. No es un comando desatendido",
              "CMD_PONG ha dejado de ser rama muerta en el Maestro, o el Esclavo empezo a emitir "
              "PING: hay que revisar quien atiende que")

    verificar(not huerfanos_vuelta,
              f"las {len(emite_esclavo)} respuestas que EMITE el Esclavo -incluidos todos los "
              f"ACK y demandas- las ATIENDE el Maestro: {sorted(emite_esclavo)}",
              f"el Esclavo responde con comandos que el Maestro no espera: {huerfanos_vuelta}")

    # Controles negativos:
    # 1. Distingue un comando inventado en la ida (que el Esclavo no atiende).
    # 2. Distingue una emision inventada en la vuelta (que el Maestro no atiende).
    # 3. Demuestra que el censo dinamico censa submodulos fuera de main.cpp (CMD_DEMANDA en demanda.cpp).
    ctrl_neg_ida = bool({"CMD_INVENTADO"} - atiende_esclavo)
    ctrl_neg_vuelta = bool((emite_esclavo | {"CMD_INVENTADO_SUBMODULO"}) - atiende_maestro)
    ctrl_censo_demanda = ("CMD_DEMANDA" in emite_esclavo)
    verificar(ctrl_neg_ida and ctrl_neg_vuelta and ctrl_censo_demanda,
              "la comprobacion de cobertura distingue comandos huerfanos y censa submodulos "
              "fuera de main.cpp (control negativo y deteccion de CMD_DEMANDA)",
              "la comprobacion de cobertura acepta cualquier cosa o no censa submodulos fuera de main.cpp")

    # Cada ACK tiene que corresponder a una orden y viceversa. Se comprueba el par
    # completo, que es lo que ninguna punta puede ver sola.
    PARES = [("CMD_GO_GREEN", "CMD_ACK_GREEN"), ("CMD_GO_RED", "CMD_ACK_RED"),
             ("CMD_PING", "CMD_PONG"), ("CMD_HORA_S", "CMD_ACK_HORA"),
             ("CMD_DELTA", "CMD_DELTA_RESP"), ("CMD_CONFIG_DESPEJE", "CMD_ACK_CONFIG")]
    pares_rotos = [(o, a) for o, a in PARES
                   if not (o in emite_maestro and o in atiende_esclavo
                           and a in emite_esclavo and a in atiende_maestro)]
    verificar(not pares_rotos,
              f"los {len(PARES)} pares orden/acuse estan cerrados en los dos sentidos: la orden "
              "sale de una punta, la otra la atiende, responde, y la primera espera esa respuesta",
              f"pares orden/acuse incompletos: {pares_rotos}")
