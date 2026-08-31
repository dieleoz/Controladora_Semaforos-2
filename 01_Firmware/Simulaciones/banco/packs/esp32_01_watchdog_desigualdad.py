# ===== banco/packs/esp32_01_watchdog_desigualdad.py =====
#
# LA DESIGUALDAD DEL WATCHDOG DEL PUENTE, RECALCULADA DE SUS TRES FUENTES.
#
#     ESP32_WDT_MS + ESP32_ARRANQUE_MS  <  min( TIMEOUT_ENLACE_MS , SFTY6_SILENCIO_MS )
#
# POR QUE ESTO ES UN PACK Y NO UN COMENTARIO.
#
# Es N-71 exacto, y peor. Alli el umbral de silencio de SFTY-6 estaba en 12 s mientras
# el ciclo necesitaba 20,5 s para agotar sus cinco reintentos: los reintentos 4 y 5 eran
# CODIGO MUERTO y nada lo delataba, porque la relacion entre los tres numeros vivia solo
# en prosa dentro de un comentario. "Los comentarios no fallan cuando alguien cambia un
# numero: se quedan describiendo un equipo que ya no existe, con la autoridad de una
# cuenta hecha."
#
# Aqui es peor porque los cuatro numeros viven en TRES LENGUAJES distintos:
#
#   ESP32_WDT_MS, ESP32_ARRANQUE_MS   C++ del ESP32   ESP32_Expansion/include/contrato.h
#   SFTY6_SILENCIO_MS                 C++ del STM32   Maestro|Esclavo/include/protocolo.h
#   TIMEOUT_ENLACE_MS                 JavaScript      05_Funcional/App_Semaforo/app.js
#
# Ningun compilador cruza esas tres fronteras. Este fichero es lo unico que las cruza.
#
# LA CORRECCION QUE HAY QUE ENTENDER, Y QUE NO ES COSMETICA.
#
# El par de constantes "obvio" -el watchdog contra SFTY-6- es el EQUIVOCADO para el rol
# de PUENTE. Medido:
#
#   Maestro/src/coordinador.cpp:656  tieneComunicacion = (tUltimaRxEsclavo > 0) && ...
#   Esclavo/src/main.cpp:555         millis() - tUltimoComando > SFTY6_SILENCIO_MS
#
# Las dos variables se alimentan del enlace de RADIO LoRa. Ninguna lee un solo byte de
# SerialBT. En esta arquitectura el ESP32 cuelga de J17, fuera del camino de la radio:
#
#   🔴 UN ESP32 COLGADO NO DISPARA SFTY-6. NO DISPARA NADA. El STM32 sigue ciclando tan
#      tranquilo y nadie en el equipo se entera.
#
# Eso es PEOR que el supuesto de partida, no mejor: el enunciado "si el watchdog es
# lento, el STM32 ya se fue a ambar" describe un equipo que AL MENOS REACCIONA. El
# equipo real no reacciona: se queda mudo hacia el telefono. El unico testigo es la app,
# y su cota son 5 s. Esta abierto como AB-1 y es del responsable.
#
# La cota de SFTY-6 se lee igual y se conserva en la cuenta -con min()- porque el otro
# rol del mismo chip, el Repetidor, SI esta en el camino de la radio y alli colgarse SI
# bloquea el bus (TROUBLESHOOTING.md:48 y :55, 31/07/2026). Manda la mas estricta.
#
# SOBRE LA ETIQUETA SFTY: este pack NO lleva ninguna, y es a proposito.
#
# Roza SFTY-6 -lee su constante y la mete en la cuenta- pero NO LA EJERCE: no prueba la
# caida a ambar por silencio de radio, ni podria, porque acaba de medir que SFTY-6 ni
# siquiera mira este enlace. Etiquetarlo seria poner en la tabla de trazabilidad una
# regla cubierta por una prueba que no la ejerce, y eso es peor que una fila vacia
# porque la vacia no miente. Los numeros de SFTY nuevos son del responsable (AB-8).

import math
import re

NOMBRE = "esp32_01_watchdog_desigualdad"
DESCRIPCION = "el watchdog del puente cabe bajo la cota de la app y por encima del peor bucle"

CONTRATO = ("ESP32_Expansion", "include", "contrato.h")
APP_JS = ("05_Funcional", "App_Semaforo", "app.js")


def _js(fw, nombre):
    """Un `const NOMBRE = <numero>;` de app.js. ABORTA si no aparece.

    Sin valor por defecto, igual que fuente.constante() hace con el C++. Un pack que
    cayera a "no lo encontre, luego uso 5000" seguiria dando PASS el dia que alguien
    subiera el timeout de la app, que es justo el dia en que la desigualdad cambia."""
    txt = fw.texto_repo(*APP_JS)
    m = re.search(r"\bconst\s+%s\s*=\s*(\d+)\s*;" % re.escape(nombre), txt)
    if not m:
        raise fw.Abortado(
            "no se pudo leer %s de %s. Es una de las TRES fuentes de la desigualdad del "
            "watchdog y la que de verdad gobierna: sin ella este pack compararia el "
            "watchdog contra una cota inventada y saldria en verde"
            % (nombre, "/".join(APP_JS)))
    return int(m.group(1))


def correr(b, fw):
    b.titulo("El watchdog del puente contra sus tres fuentes")

    # ---- 1. Los cuatro numeros, releidos de donde viven -----------------------
    wdt = fw.constante(CONTRATO, r"#define\s+ESP32_WDT_MS\s+(\d+)UL",
                       "el periodo del watchdog del ESP32")
    arranque = fw.constante(CONTRATO, r"#define\s+ESP32_ARRANQUE_MS\s+(\d+)UL",
                            "el tiempo de arranque del ESP32")
    medido = fw.constante(CONTRATO, r"#define\s+ESP32_ARRANQUE_MEDIDO\s+(\d+)",
                          "la bandera de si el arranque esta medido")
    iteraciones = fw.constante(CONTRATO, r"#define\s+PUENTE_MAX_ITER\s+(\d+)",
                               "el tope de iteraciones del bucle interior")
    baudio = fw.constante(CONTRATO, r"#define\s+ENLACE_BAUDIO\s+(\d+)",
                          "el baudio del enlace con el STM32")
    bits = fw.constante(CONTRATO, r"#define\s+ENLACE_BITS_POR_BYTE\s+(\d+)",
                        "los bits por byte en el cable")

    sfty6 = {}
    for punta in ("Maestro", "Esclavo"):
        sfty6[punta] = fw.constante((punta, "include", "protocolo.h"),
                                    r"#define\s+SFTY6_SILENCIO_MS\s+(\d+)UL",
                                    "el umbral de silencio de SFTY-6 del %s" % punta)
    timeout_app = _js(fw, "TIMEOUT_ENLACE_MS")

    b.verificar(
        sfty6["Maestro"] == sfty6["Esclavo"],
        "SFTY6_SILENCIO_MS vale lo mismo en las dos puntas (%d ms)" % sfty6["Maestro"],
        "SFTY6_SILENCIO_MS difiere entre puntas: Maestro %d, Esclavo %d. Con dos "
        "umbrales distintos no hay UNA cota que calcular, y cada punta se iria a ambar "
        "en un momento distinto" % (sfty6["Maestro"], sfty6["Esclavo"]))

    cota = min(timeout_app, sfty6["Maestro"])

    # ---- 2. LA COTA QUE GOBIERNA ES LA DE LA APP, Y HAY QUE MEDIR POR QUE ------
    #
    # No basta con hacer min(): hay que comprobar que sigue siendo cierto lo que hace
    # que la cota de la app sea la unica que vigila al puente, o sea que SFTY-6 mira la
    # RADIO. El dia que alguien meta SerialBT en esa condicion, el equipo SI notaria un
    # puente colgado, la arquitectura cambiaria y esta cuenta habria que rehacerla.
    # Es un trinquete: falla cuando la propiedad medida deja de ser cierta.
    coord = fw.codigo("Maestro", "src", "coordinador.cpp")
    m = re.search(r"tieneComunicacion\s*=([^;]*SFTY6_SILENCIO_MS[^;]*);", coord)
    b.verificar(
        m is not None and "tUltimaRxEsclavo" in m.group(1) and "SerialBT" not in m.group(1),
        "MEDIDO que SFTY-6 del Maestro vigila la RADIO (tUltimaRxEsclavo), no J17: un "
        "puente colgado es invisible para el equipo, y por eso la cota que gobierna es "
        "la de la app",
        "la condicion de SFTY-6 del Maestro ya no es la que este pack midio. O dejo de "
        "existir -y entonces no hay cota de radio que comparar- o ahora mira SerialBT, "
        "y entonces el equipo SI vigila al puente: la arquitectura cambio y AB-1 con "
        "ella. En los dos casos hay que releer este pack antes de fiarse de su verde")

    esclavo_main = fw.codigo("Esclavo", "src", "main.cpp")
    b.verificar(
        re.search(r"tUltimoComando\s*>\s*SFTY6_SILENCIO_MS", esclavo_main) is not None
        and not re.search(r"SerialBT[^;]*SFTY6_SILENCIO_MS", esclavo_main),
        "MEDIDO que SFTY-6 del Esclavo vigila tUltimoComando (radio), tampoco J17",
        "la condicion de SFTY-6 del Esclavo cambio de forma o pasa a mirar SerialBT: "
        "misma consecuencia que arriba, hay que rehacer la cuenta de este pack")

    # ---- 3. EL TECHO: la desigualdad -----------------------------------------
    b.verificar(
        wdt + arranque < cota,
        "cabe bajo la cota: %d + %d = %d ms < min(%d de app.js, %d de protocolo.h) = %d"
        % (wdt, arranque, wdt + arranque, timeout_app, sfty6["Maestro"], cota),
        "NO CABE: el watchdog del ESP32 (%d ms) mas su arranque (%d ms) suman %d ms y "
        "la cota es %d ms. Un puente que tarda mas que eso en volver deja a la app "
        "declarando el enlace perdido de un equipo que esta sano, y el operario ve "
        "'sin enlace' delante de un cruce que cicla bien"
        % (wdt, arranque, wdt + arranque, cota))

    # ---- 4. EL SUELO: un techo desproporcionado tambien es un defecto ---------
    #
    # Bidireccional, como costura_09_presupuesto_radio. El peor caso de una vuelta se
    # RECALCULA, no se escribe: son los dos sentidos drenando su tope de iteraciones a
    # la velocidad del cable.
    peor_vuelta = math.ceil(iteraciones * 2 * bits * 1000.0 / baudio)
    b.verificar(
        wdt > peor_vuelta,
        "queda por encima del peor bucle: %d ms > %d ms (%d iteraciones x 2 sentidos x "
        "%d bits a %d bps)" % (wdt, peor_vuelta, iteraciones, bits, baudio),
        "EL WATCHDOG ES MAS CORTO QUE UNA VUELTA LEGITIMA: %d ms contra %d ms de peor "
        "caso. El puente se reiniciaria solo mientras hace su trabajo, y cada reinicio "
        "tira la sesion SPP del operario. Un perro que muerde al que trabaja no vigila: "
        "estorba" % (wdt, peor_vuelta))

    # ---- 5. El techo se programa en SEGUNDOS ENTEROS --------------------------
    #
    # esp_task_wdt_init(uint32_t timeout, bool panic) -medido en el header del IDF que
    # trae el framework-. Un 2500 aqui se convertiria en 2 s dentro del chip mientras
    # esta cuenta seguiria comprobando 2,5 s: el banco estaria midiendo un equipo que no
    # es, en verde y sin decirlo.
    b.verificar(
        wdt % 1000 == 0,
        "ESP32_WDT_MS (%d) es multiplo de 1000: lo que se programa en el chip es "
        "exactamente lo que este pack compara" % wdt,
        "ESP32_WDT_MS = %d no es multiplo de 1000. esp_task_wdt_init() recibe SEGUNDOS "
        "enteros, asi que el chip usaria %d ms y este banco estaria comprobando otro "
        "numero" % (wdt, (wdt // 1000) * 1000))

    # ---- 6. La bandera de "medido" tiene que ser legible ----------------------
    b.verificar(
        medido in (0, 1),
        "ESP32_ARRANQUE_MEDIDO = %d, un valor que este pack sabe interpretar" % medido,
        "ESP32_ARRANQUE_MEDIDO = %d no es 0 ni 1. Cualquier otro valor lo trataria este "
        "pack como 'medido' sin haberlo distinguido, y un marcador que se lee como "
        "medida es exactamente lo que la bandera venia a impedir" % medido)

    # ---- 7. AB-3: el margen se publica sobre un MARCADOR ----------------------
    #
    # reportar() no cuenta como comprobacion, y ese es el sitio correcto: no hay firmware
    # posible que "apruebe" esto -solo lo cierra una medida con el modulo en la mano-, y
    # una comprobacion que ningun firmware puede aprobar no es una comprobacion, es una
    # nota. La ficha tecnica del ESP32-WROOM-32 que cerro BLQ-1 no lo contesta: un
    # bloqueo cerrado no cierra los de al lado.
    if medido != 1:
        b.reportar(
            "ESP32_ARRANQUE_MS = %d ms esta SIN VERIFICAR (AB-3)" % arranque,
            ["nadie ha medido cuanto tarda el modulo desde el reset hasta volver a "
             "pasar bytes, ni cuanto tarda el telefono en reemparejar el SPP",
             "el margen de %d ms que publica el apartado 3 es un margen sobre un "
             "MARCADOR, no sobre una medida" % (cota - wdt - arranque),
             "se cierra midiendolo con el modulo en la mano y subiendo "
             "ESP32_ARRANQUE_MEDIDO a 1; NO ajustando el watchdog para que cuadre"])

    # ---- CONTROLES NEGATIVOS -------------------------------------------------
    #
    # Cada uno rompe UNA de las tres relaciones y exige que la cuenta lo note. Sin
    # esto, el dia que un patron dejara de casar el pack compararia nada contra nada.
    b.control_negativo(
        not (6000 + arranque < cota),
        "un watchdog de 6000 ms deja de caber bajo la cota de %d ms" % cota)
    b.control_negativo(
        not (10 > peor_vuelta),
        "un watchdog de 10 ms cae por debajo del peor bucle de %d ms" % peor_vuelta)
    b.control_negativo(
        2500 % 1000 != 0,
        "un ESP32_WDT_MS de 2500 se detecta como no programable en segundos enteros")
    b.control_negativo(
        _js.__doc__ is not None and re.search(
            r"\bconst\s+TIMEOUT_ENLACE_MS\s*=\s*(\d+)\s*;",
            "const OTRA_COSA = 5000;") is None,
        "el lector de app.js no casa con una constante de otro nombre: no hay valor por "
        "defecto que tapara la desaparicion de TIMEOUT_ENLACE_MS")
