# ===== banco/packs/esp32_02_watchdog_alimentado.py =====
#
# EL PERRO SE ALIMENTA DESDE LA TAREA QUE SE PUEDE COLGAR, Y NO DESDE DENTRO DEL BUCLE
# QUE EL RUIDO PUEDE HACER ETERNO.
#
# POR QUE ESTE PACK NO ES REDUNDANTE CON EL 01.
#
# El 01 comprueba que el NUMERO cabe. Este comprueba que el mecanismo EXISTE donde
# tiene que existir, y son cosas distintas: un esp_task_wdt_init() con un periodo
# perfectamente calculado y sin su reset en el sitio correcto es CAM_UMBRAL_PIN con
# otro nombre -un pinMode() sin digitalRead(), con documentacion encima-. Cuatro
# documentos describian aquel pin. La documentacion no es la llamada.
#
# LAS CINCO REGLAS QUE SE VIGILAN, Y DE DONDE SALE CADA UNA:
#
#   W-1  esp_task_wdt_add(NULL) sobre LA TAREA QUE BOMBEA BYTES. Registrar una tarea de
#        servicio seria vigilar a un testigo que no se cuelga nunca.
#   W-2  el reset, una vez por vuelta del bucle EXTERIOR, tras atender los dos sentidos.
#   W-3  🔴 el reset NO va dentro del while interior. Es la forma exacta del fallo del
#        31/07/2026 (TROUBLESHOOTING.md:48): "el ESP32 levantaba DE/RE ante cualquier
#        byte y solo lo bajaba tras 5 ms de silencio; con ruido continuo ese silencio
#        nunca llegaba". Un reset dentro de ese bucle alimenta al perro PARA SIEMPRE
#        mientras el puente no progresa. Un watchdog que un flujo de basura mantiene
#        contento no vigila nada.
#   W-4  el bucle interior lleva TOPE DE ITERACIONES, no solo condicion de
#        disponibilidad. Misma razon que W-3, por el otro lado.
#   W-5  el perro se arma ANTES del SPP y ANTES del I2C: si el DS3231 cuelga el bus en
#        el arranque, ya tiene que haber quien reinicie.
#
# NO LLEVA ETIQUETA SFTY. SFTY-1 es el watchdog de los STM32; esta es otra pieza en otro
# micro, y OPTIMIZACIONES.md:55 dice hoy "El Repetidor ESP32 no implementa watchdog".
# Asignar un numero de SFTY es del responsable (AB-8); etiquetar sin regla asignada
# pondria en la tabla de trazabilidad una fila que no corresponde a nada.

import re

NOMBRE = "esp32_02_watchdog_alimentado"
DESCRIPCION = "el watchdog se arma primero y se alimenta desde el bucle exterior, no desde el interior"

ROL = "ESP32_Expansion"


def _bloque(texto, i):
    """El interior del bloque que abre en texto[i] == '{'. None si no cierra."""
    if i < 0 or i >= len(texto) or texto[i] != "{":
        return None
    prof = 0
    for j in range(i, len(texto)):
        if texto[j] == "{":
            prof += 1
        elif texto[j] == "}":
            prof -= 1
            if prof == 0:
                return texto[i + 1:j]
    return None


def _cuerpo(codigo, firma):
    """El cuerpo de una funcion, buscando su llave de apertura."""
    m = re.search(firma + r"\s*\{", codigo)
    if not m:
        return None
    return _bloque(codigo, m.end() - 1)


def _whiles(codigo):
    """[(cabecera, cuerpo)] de cada while del fichero."""
    fuera = []
    for m in re.finditer(r"\bwhile\s*\(", codigo):
        # Cierre del parentesis de la condicion, contando anidamiento.
        prof = 0
        fin = -1
        for j in range(m.end() - 1, len(codigo)):
            if codigo[j] == "(":
                prof += 1
            elif codigo[j] == ")":
                prof -= 1
                if prof == 0:
                    fin = j
                    break
        if fin < 0:
            continue
        cabecera = codigo[m.end():fin]
        i = codigo.find("{", fin)
        if i < 0:
            continue
        cuerpo = _bloque(codigo, i)
        if cuerpo is not None:
            fuera.append((cabecera, cuerpo))
    return fuera


def correr(b, fw):
    b.titulo("El watchdog del puente: armado, registrado y alimentado donde toca")

    vigilante = fw.codigo("ESP32_Expansion", "src", "vigilante.cpp")
    main = fw.codigo("ESP32_Expansion", "src", "main.cpp")
    puente = fw.codigo("ESP32_Expansion", "src", "puente.cpp")

    # ---- W-1: existe el registro, y es de LA TAREA ACTUAL ---------------------
    b.verificar(
        re.search(r"esp_task_wdt_init\s*\(", vigilante) is not None,
        "vigilante.cpp arma el task watchdog con esp_task_wdt_init()",
        "no hay esp_task_wdt_init() en vigilante.cpp: el ESP32 se queda como estaba "
        "-sin perro-, que es como llego a este proyecto un modulo que se clavo tumbando "
        "el enlace el 31/07/2026")

    b.verificar(
        re.search(r"esp_task_wdt_add\s*\(\s*NULL\s*\)", vigilante) is not None,
        "W-1: se registra la TAREA ACTUAL -esp_task_wdt_add(NULL)-, que es la que corre "
        "loop() y por tanto la que bombea bytes",
        "no se registra la tarea actual. Un init() sin add() no vigila a nadie, y un "
        "add() de OTRA tarea vigila a un testigo que no se cuelga nunca: el puente se "
        "queda mudo y el perro sigue contento")

    b.verificar(
        re.search(r"esp_task_wdt_reset\s*\(\s*\)", vigilante) is not None,
        "existe el esp_task_wdt_reset() que alimenta al perro",
        "hay init() y add() y NO hay reset(): el equipo se reiniciaria solo cada %s. Es "
        "el error de forma contrario y se ve igual de tarde" % "periodo")

    # ---- W-2: el reset se llama desde el bucle EXTERIOR ------------------------
    cuerpoLoop = _cuerpo(main, r"void\s+loop\s*\(\s*\)")
    if cuerpoLoop is None:
        raise fw.Abortado(
            "no se hallo loop() en %s/src/main.cpp. Es donde vive el bucle exterior; "
            "sin poder leerlo, este pack no puede decir desde donde se alimenta el "
            "perro y aprobarlo seria aprobar sin mirar" % ROL)

    b.verificar(
        "vigilante_alimentar" in cuerpoLoop,
        "W-2: loop() alimenta al perro una vez por vuelta",
        "loop() NO llama a vigilante_alimentar(). El perro esta armado y nadie lo "
        "alimenta: el modulo se reinicia solo, y en esta arquitectura cada reinicio "
        "tira la sesion SPP del operario")

    # Y va DESPUES de bombear, que es lo que hace que el reset signifique "progreso".
    iBombear = cuerpoLoop.find("puente_bombear")
    iAlimentar = cuerpoLoop.find("vigilante_alimentar")
    b.verificar(
        iBombear >= 0 and iAlimentar > iBombear,
        "W-2: el reset va DESPUES de haber atendido las dos direcciones, asi que "
        "alimentar al perro significa que la vuelta se completo",
        "el reset del watchdog no va detras de puente_bombear(). Alimentar antes de "
        "trabajar convierte el reset en un latido de reloj: se daria igual aunque el "
        "bombeo no volviera nunca")

    # ---- W-3: 🔴 EL RESET NO VIVE DENTRO DE NINGUN while ----------------------
    dentro = []
    for fichero in fw.fuentes_de("ESP32_Expansion", "src"):
        codigo = fw.codigo("ESP32_Expansion", "src", fichero)
        for cabecera, cuerpo in _whiles(codigo):
            if re.search(r"esp_task_wdt_reset\s*\(|vigilante_alimentar\s*\(", cuerpo):
                dentro.append("%s: while (%s)" % (fichero, cabecera.strip()[:40]))

    b.verificar(
        not dentro,
        "W-3: ningun while del proyecto contiene el reset del watchdog",
        "EL RESET DEL WATCHDOG VIVE DENTRO DE UN BUCLE: %s. Es la forma exacta del "
        "fallo del 31/07/2026: con ruido continuo ese bucle no termina, y el reset de "
        "dentro alimentaria al perro para siempre mientras el puente no progresa. Un "
        "watchdog que un flujo de basura mantiene contento no vigila nada"
        % ", ".join(dentro))

    # ---- W-4: todo while del bombeo lleva tope de iteraciones -----------------
    #
    # Se lee el nombre de la constante del C++, no se teclea: si alguien la renombra,
    # esto ABORTA en vez de aprobar por no encontrarla.
    tope = fw.constante(("ESP32_Expansion", "include", "contrato.h"),
                        r"#define\s+PUENTE_MAX_ITER\s+(\d+)",
                        "el tope de iteraciones del bucle interior")
    sinTope = [cab.strip()[:50] for cab, _ in _whiles(puente)
               if "PUENTE_MAX_ITER" not in cab]
    b.verificar(
        _whiles(puente) and not sinTope,
        "W-4: los %d bucles de bombeo llevan tope de %d iteraciones ademas de la "
        "condicion de disponibilidad" % (len(_whiles(puente)), tope),
        "hay bucles de bombeo SIN TOPE: %s. Un `while (hay bytes)` con un flujo continuo "
        "de basura no termina nunca, y ahi dentro no se alimenta al perro: el modulo se "
        "reinicia en bucle en vez de recuperarse" % ", ".join(sinTope) if sinTope
        else "puente.cpp no tiene ni un bucle de bombeo: o el fuente cambio de forma o "
             "el buscador se quedo ciego, y medir cero bucles saldria en verde")

    # ---- W-5: el perro se arma ANTES del SPP y del I2C ------------------------
    cuerpoSetup = _cuerpo(main, r"void\s+setup\s*\(\s*\)")
    if cuerpoSetup is None:
        raise fw.Abortado(
            "no se hallo setup() en %s/src/main.cpp: sin el no se puede comprobar el "
            "orden de armado, que es toda la regla W-5" % ROL)

    iArmar = cuerpoSetup.find("vigilante_armar")
    iReloj = cuerpoSetup.find("reloj_setup")
    iSpp = cuerpoSetup.find("transporte_abrir")
    b.verificar(
        iArmar >= 0 and iReloj > iArmar and iSpp > iArmar,
        "W-5: el watchdog se arma antes del I2C y antes del SPP",
        "el watchdog NO se arma primero (armar=%d, reloj=%d, spp=%d). Si el DS3231 "
        "cuelga el bus I2C en el arranque -un SDA en corto basta- no hay quien "
        "reinicie, y ese es justo uno de los casos que justifican tener perro. Es la "
        "misma razon por la que el Maestro arma el suyo antes de rtc.begin()"
        % (iArmar, iReloj, iSpp))

    # ---- CONTROLES NEGATIVOS -------------------------------------------------
    malo = "void f(){ while (hay() && i < PUENTE_MAX_ITER) { esp_task_wdt_reset(); } }"
    b.control_negativo(
        any(re.search(r"esp_task_wdt_reset\s*\(", c) for _, c in _whiles(malo)),
        "un reset colado dentro de un while se detecta aunque el bucle SI tenga tope")

    sinTopeSintetico = "void f(){ while (transporte_disponible() > 0) { leer(); } }"
    b.control_negativo(
        any("PUENTE_MAX_ITER" not in cab for cab, _ in _whiles(sinTopeSintetico)),
        "un bucle de bombeo sin tope de iteraciones se detecta")

    b.control_negativo(
        _cuerpo("void otra(){ vigilante_armar(); }", r"void\s+setup\s*\(\s*\)") is None,
        "si setup() dejara de existir el pack ABORTA en vez de aprobar sobre un cuerpo "
        "vacio")
