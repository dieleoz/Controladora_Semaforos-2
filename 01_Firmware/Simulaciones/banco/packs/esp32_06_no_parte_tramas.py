# ===== banco/packs/esp32_06_no_parte_tramas.py =====
#
# UNA TRAMA ENTRA ENTERA Y SALE ENTERA, Y SE VALIDA ANTES DE RETRANSMITIR.
#
# POR QUE PARTIR UNA TRAMA NO ES UN DETALLE DE EFICIENCIA.
#
# El receptor del otro lado delimita por '\r' o '\n' -es la regla E-1, medida en el
# bucle receptor de las dos puntas del STM32-. Si el puente parte una linea en dos
# escrituras, el receptor no recibe "media trama y luego la otra media": recibe DOS
# LINEAS, y las dos son basura. Y la primera puede casar por accidente con un comando
# mas corto que si existe, porque el STM32 compara con strcmp lo que le llegue.
#
# Unirlas es el mismo fallo por el otro lado: dos tramas pegadas sin terminador entre
# medias llegan como una sola linea que no casa con nada.
#
# B-6: SE VALIDA ANTES DE RETRANSMITIR, NO DESPUES.
#
# Es SFTY-16 aplicado al puente, y es lo que ya hace el firmware del Repetidor en la
# otra topologia: "ese ruido se descarta dentro del ESP32 y nunca llega al aire". La
# razon esta pagada en campo el 31/07/2026: un puente que reenvia primero y comprueba
# despues ya ha metido la basura en el canal cuando se entera.
#
# B-7: y lo que se descarta SE CUENTA. Un byte tirado en silencio se lee como que nunca
# existio -es E-2 otra vez, el truncado mudo del STM32-, y el contador es lo unico que
# permite verlo desde fuera cuando alguien tenga que diagnosticar un poste.

import re

NOMBRE = "esp32_06_no_parte_tramas"
DESCRIPCION = "una trama entra entera y sale entera, validada antes de retransmitir y con lo descartado contado"

ROL = "ESP32_Expansion"


def _bloque(texto, i):
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
    m = re.search(firma + r"\s*\{", codigo)
    return None if not m else _bloque(codigo, m.end() - 1)


def correr(b, fw):
    b.titulo("El puente no parte ni une tramas, y valida antes de retransmitir")

    puerta = fw.codigo("ESP32_Expansion", "src", "enlace_stm32.cpp")
    puente = fw.codigo("ESP32_Expansion", "src", "puente.cpp")

    # ---- 1. Hacia el STM32: UNA trama, UNA escritura --------------------------
    escritura = _cuerpo(puerta, r"size_t\s+enlace_escribirLinea\s*\([^)]*\)")
    if escritura is None:
        raise fw.Abortado(
            "no se hallo enlace_escribirLinea() en %s/src/enlace_stm32.cpp. Es la unica "
            "salida hacia el equipo: sin poder leerla no hay forma de comprobar si parte "
            "las tramas" % ROL)

    escrituras = re.findall(r"\.write\s*\(|\.print\s*\(|\.println\s*\(", escritura)
    b.verificar(
        len(escrituras) == 1,
        "B-5: la salida hacia el STM32 hace UNA sola escritura por trama",
        "la salida hacia el STM32 hace %d escrituras (%s). El receptor delimita por "
        "'\\r'/'\\n': una trama partida en dos write() se le entrega como DOS LINEAS, y "
        "las dos son basura -la primera puede ademas casar por accidente con un comando "
        "mas corto-" % (len(escrituras), escrituras))

    # ---- 2. El terminador va DENTRO del mismo buffer, no en otra llamada ------
    b.verificar(
        re.search(r"linea\[n\]\s*=\s*'\\r'", escritura) is not None
        and re.search(r"linea\[n \+ 1\]\s*=\s*'\\n'", escritura) is not None,
        "E-1: el terminador se compone dentro del mismo buffer que la trama, no en una "
        "segunda escritura",
        "el terminador no se compone dentro del buffer de la trama. Mandarlo aparte es "
        "partir la trama en dos escrituras con otro nombre, y con el mismo efecto")

    # ---- 3. Hacia la app: idem, y con el terminador que puso el equipo --------
    hacia = _cuerpo(puente, r"static\s+void\s+desdeElEquipo\s*\(\s*\)")
    if hacia is None:
        raise fw.Abortado(
            "no se hallo desdeElEquipo() en %s/src/puente.cpp: es el sentido "
            "equipo -> app y sin el este pack solo mediria la mitad" % ROL)

    escrituraApp = re.findall(r"transporte_escribir\s*\(", hacia)
    b.verificar(
        len(escrituraApp) == 1,
        "B-5: el sentido equipo -> app tambien hace UNA sola escritura por trama",
        "el sentido equipo -> app hace %d escrituras. El parser de la app trocea por "
        "lineas igual que el STM32: partir un $STATUS por la mitad le entrega dos "
        "fragmentos y ninguno es una trama" % len(escrituraApp))

    b.verificar(
        re.search(r"salida\[largo\]\s*=\s*'\\r'", hacia) is not None
        and re.search(r"salida\[largo \+ 1\]\s*=\s*'\\n'", hacia) is not None,
        "S-1: se reponen los DOS bytes del terminador que el STM32 puso, asi que la app "
        "recibe byte a byte lo que salio del equipo",
        "no se reponen los dos bytes del terminador. El STM32 emite SIEMPRE \"\\r\\n\" "
        "-medido en enviarTramaConCrc()-, y entregarle a la app algo distinto es que el "
        "puente ha cambiado la trama")

    # ---- 4. B-6: la validacion va ANTES de la retransmision -------------------
    #
    # 🔴 SOLO EN EL SENTIDO DE VUELTA, Y ESO ES UNA CORRECCION, NO UNA LAGUNA.
    #
    # La version anterior de este pack exigia validacion en LOS DOS sentidos, siguiendo
    # 18_...md 3.4, que dice -con la palabra MEDIDO encima- que la app anade *XX. Es
    # falso: MEDIDO en app.js:199-207, el emisor vivo compone
    # "CMD:PIN:1234:SET_MODO:AUTO\r\n", sin '$' y sin checksum. El formatearComando()
    # que la spec cita no existe -es formatearTrama(), en un modulo huerfano que la app
    # carga y nadie llama-.
    #
    # Un pack que exigiera validar la ida habria obligado a un puente que descarta el
    # 100% de los comandos reales. Aqui se exige lo contrario y se comprueba abajo.
    iValida = hacia.find("trama_valida")
    iEnvia = hacia.find("transporte_escribir")
    b.verificar(
        iValida >= 0 and iEnvia >= 0 and iValida < iEnvia,
        "B-6 en STM32 -> app: se valida el checksum ANTES de retransmitir (validar=%d, "
        "enviar=%d)" % (iValida, iEnvia),
        "en STM32 -> app la validacion no precede a la retransmision (validar=%d, "
        "enviar=%d). Comprobar despues de haber mandado es enterarse cuando la basura ya "
        "esta en el canal: es el fallo del repetidor del 31/07/2026 con otro nombre"
        % (iValida, iEnvia))

    ida = _cuerpo(puente, r"static\s+void\s+desdeLaApp\s*\(\s*\)")
    if ida is None:
        raise fw.Abortado(
            "no se hallo desdeLaApp() en %s/src/puente.cpp: es el sentido de ida y sin "
            "el no se puede comprobar que viaja verbatim" % ROL)
    b.verificar(
        "trama_valida" not in ida,
        "el sentido app -> STM32 NO valida checksum: la app no lo pone, y exigirlo "
        "descartaria todos los comandos reales",
        "el sentido app -> STM32 VALIDA CHECKSUM. MEDIDO que la app no lo anade "
        "(app.js:199-207): con esta guarda el puente descarta el 100% de los comandos "
        "legitimos y contesta un $ERR propio a cada uno. Es una prueba muerta al reves: "
        "un instrumento que no aprueba nada valido")

    b.verificar(
        not re.search(r"trama_componer\s*\(|checksum", ida),
        "el sentido app -> STM32 tampoco ANADE checksum: los bytes salen verbatim",
        "el sentido app -> STM32 anade checksum a lo que reenvia. El STM32 compara la "
        "linea ENTERA con strcmp: un '*4F' pegado detras hace que no case ningun "
        "comando y todos caen en $ERR,CMD:DESCONOCIDO")

    # ---- 5. B-7: lo descartado se cuenta -------------------------------------
    contadores = re.findall(r"descartadas(?:Crc|Largo)\+\+", puente)
    b.verificar(
        len(contadores) >= 3,
        "B-7: los descartes se cuentan en %d sitios y los contadores son legibles desde "
        "fuera" % len(contadores),
        "solo hay %d incrementos de contador de descartes. Lo que se tira callando se "
        "lee como que nunca existio, y es lo unico que se puede mirar cuando hay que "
        "diagnosticar un poste mudo" % len(contadores))

    expuestos = fw.codigo("ESP32_Expansion", "include", "puente.h")
    b.verificar(
        "puente_descartadasPorCrc" in expuestos and "puente_descartadasPorLargo" in expuestos,
        "los contadores de descarte estan expuestos en la cabecera: se pueden leer",
        "los contadores existen y NO se exponen. Un contador que nadie puede leer es la "
        "version silenciosa de no contar nada")

    # ---- CONTROLES NEGATIVOS -------------------------------------------------
    partida = ('{ haciaSTM32.write(datos, n); haciaSTM32.write("\\r\\n", 2); }')
    b.control_negativo(
        len(re.findall(r"\.write\s*\(|\.print\s*\(", partida)) != 1,
        "una salida que manda la trama y luego el terminador en otra escritura se "
        "detecta como trama partida")

    alReves = '{ transporte_escribir(x, n); if (!trama_valida(x)) return; }'
    b.control_negativo(
        alReves.find("trama_valida") > alReves.find("transporte_escribir"),
        "validar DESPUES de retransmitir se detecta: el orden es la regla, no la "
        "presencia de la llamada")

    b.control_negativo(
        len(re.findall(r"descartadas(?:Crc|Largo)\+\+", "{ return; }")) < 3,
        "un puente que descartara sin contar se detecta")
