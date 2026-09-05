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
DESCRIPCION = "una trama entra entera y sale entera -salvo el hueco de HORA (N-145)-, validada antes de retransmitir y con lo descartado contado"

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

    # ---- 6. N-145: EL SELLO DE HORA, QUE ES LA UNICA EXCEPCION A "SALE ENTERA" --
    #
    # 🔴 POR QUE ESTAS COMPROBACIONES ESTAN AQUI Y NO EN UN PACK NUEVO.
    #
    # Este es el pack que dice "una trama entra entera y sale entera". Desde N-145 eso
    # tiene UNA excepcion, y una excepcion que vive en otro fichero es una excepcion que
    # nadie vuelve a mirar: es la lista de HUERFANOS_CONOCIDOS de N-122 con otra forma.
    # El sitio de la excepcion es al lado de la regla que acota.
    #
    # Y no miden lo mismo que el simulador. El simulador ejerce el LAZO -el Maestro real
    # emite el hueco, la app real acepta el sello-, pero su puente es un modelo en
    # Python: un defecto metido en este C++ no le mueve la cuenta ni un poco. Lo que
    # sigue son propiedades del FUENTE, y son las unicas que caen si alguien rompe el
    # sello de verdad. Se comprobo inyectando (CLAUDE.md 8.bis).
    sello = _cuerpo(puente, r"static\s+bool\s+sellarHoraSiFaltaba\s*\([^)]*\)")
    if sello is None:
        raise fw.Abortado(
            "no se hallo sellarHoraSiFaltaba() en %s/src/puente.cpp. Es el unico sitio "
            "donde este puente modifica una trama del equipo: sin poder leerlo no se "
            "puede comprobar que no inventa una hora, y eso es ABORTADO, no PASS" % ROL)

    # 6.1 LA BARRERA DEL RELOJ MANDA, Y LA ESCRITURA VA DETRAS DE ELLA.
    #
    # Es la comprobacion que N-144 pago: aquel dia el equipo se declaro EN HORA con el
    # reloj parado en ceros y publico HORA:00:00:00, y de esa bandera cuelga la
    # autorizacion del Modo Degradado. Aqui el equivalente seria escribir en el hueco lo
    # que el chip devuelva sin preguntar a reloj_leer(), que ya lleva reloj_enHora()
    # delante y no tiene variante "damela igual".
    iLeer = sello.find("reloj_leer")
    iEscribe = sello.find("memcpy")
    b.verificar(
        iLeer >= 0 and iEscribe >= 0 and iLeer < iEscribe
        and re.search(r"if\s*\(\s*!\s*reloj_leer\s*\([^)]*\)\s*\)\s*return\s+false\s*;",
                      sello) is not None,
        "N-145: el sello pregunta a reloj_leer() ANTES de escribir en la trama, y se "
        "va sin tocar nada si la barrera esta abajo (leer=%d, escribir=%d)"
        % (iLeer, iEscribe),
        "el sello escribe en la trama sin que la barrera del reloj lo autorice "
        "(leer=%d, escribir=%d). Un DS3231 sin pila devuelve una fecha PERFECTAMENTE "
        "FORMADA y falsa: rellenar el hueco con ella es N-144 mudado de micro, y un cero "
        "que parece una hora es peor que un hueco porque el hueco no engana a nadie"
        % (iLeer, iEscribe))

    # 6.2 EL CHECKSUM SE RECALCULA, Y DESPUES DE ESCRIBIR LA HORA.
    #
    # El orden es la comprobacion: recalcular antes de sellar deja la trama con la hora
    # nueva y el checksum de la vieja, o sea descartada por la app EN SILENCIO. El
    # sintoma no seria "la hora esta mal": seria el tablero congelado, que manda a
    # diagnosticar el cable.
    iCrc = sello.find("trama_checksum")
    b.verificar(
        iCrc >= 0 and iEscribe >= 0 and iEscribe < iCrc,
        "N-145: el checksum se recalcula DESPUES de sellar la hora (escribir=%d, "
        "checksum=%d): la app valida el XOR-8 en la bajada y una trama sellada con el "
        "checksum viejo no se pinta" % (iEscribe, iCrc),
        "el sello no recalcula el checksum despues de escribir la hora (escribir=%d, "
        "checksum=%d). parseNmeaTelemetry() -> juzgarTrama() -> validarTrama() la "
        "descarta sin decir nada, y el tablero se queda congelado" % (iEscribe, iCrc))

    # 6.3 EL SELLO ES NEUTRO EN LONGITUD, Y LA CUENTA SE REHACE DESDE EL FUENTE.
    #
    # Los tres numeros se releen: el hueco que se busca, el formato con que se rellena y
    # el guardia que compara. Ninguno se teclea aqui. Si dejaran de coincidir, el C++
    # escribiria fuera del hueco que reservo -y `largo` dejaria de valer en
    # desdeElEquipo(), asi que el terminador se repondria en el sitio equivocado-.
    mHueco = re.search(r'#define\s+HUECO_HORA\s+"([^"]*)"', puente)
    mFmt = re.search(r'snprintf\s*\(\s*hhmmss\s*,[^,]*,\s*"([^"]*)"', sello)
    mGuardia = re.search(r"n\s*!=\s*(\d+)", sello)
    if mHueco is None or mFmt is None or mGuardia is None:
        raise fw.Abortado(
            "no se pudieron releer del fuente los tres numeros del sello de N-145 "
            "(hueco=%s, formato=%s, guardia=%s) en %s/src/puente.cpp. Sin ellos esta "
            "desigualdad se estaria escribiendo a mano, que es justo lo que un banco "
            "que no puede fallar no demuestra"
            % (mHueco is not None, mFmt is not None, mGuardia is not None, ROL))

    largoHueco = len(mHueco.group(1).partition(":")[2])
    largoSello = len(re.sub(r"%02d", "XX", mFmt.group(1)))
    largoGuardia = int(mGuardia.group(1))
    b.verificar(
        largoHueco == largoSello == largoGuardia,
        "N-145: el sello no cambia la longitud de la trama: el hueco %r mide %d, el "
        "formato %r rinde %d y el guardia exige %d"
        % (mHueco.group(1), largoHueco, mFmt.group(1), largoSello, largoGuardia),
        "EL SELLO NO ES NEUTRO EN LONGITUD: el hueco mide %d, lo que se escribe mide %d "
        "y el guardia exige %d. Sellar EN SITIO solo vale si los tres son iguales; si no "
        "lo son, el memcpy pisa lo que hay detras del hueco y `largo` deja de valer en "
        "desdeElEquipo()" % (largoHueco, largoSello, largoGuardia))

    # 6.4 Y SE SELLA DENTRO DE LA VENTANA CORRECTA: despues de validar, antes de enviar.
    #
    # Sellar antes de trama_valida() obligaria a recalcular el checksum de algo que puede
    # ser ruido de cable: el puente estaria FABRICANDO tramas validas a partir de basura,
    # que es exactamente lo que el sentido de vuelta existe para no hacer.
    iSella = hacia.find("sellarHoraSiFaltaba")
    b.verificar(
        iSella >= 0 and iValida < iSella < iEnvia,
        "N-145: se sella DESPUES de validar el checksum y ANTES de enviar (validar=%d, "
        "sellar=%d, enviar=%d)" % (iValida, iSella, iEnvia),
        "el sello no esta entre la validacion y el envio (validar=%d, sellar=%d, "
        "enviar=%d). Antes de validar, el puente recalcularia el checksum de ruido de "
        "cable y lo convertiria en una trama bien formada; despues de enviar, no serviria "
        "de nada" % (iValida, iSella, iEnvia))

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

    # N-145: los tres controles del sello. Cada uno es el defecto EXACTO que su
    # comprobacion de arriba vigila, escrito aparte para que la prueba tenga que saber
    # decir que no. Sin ellos, las cuatro de N-145 podrian estar aprobando cualquier cosa
    # -que es lo que hacia la prueba 2.8 de N-51 con su break siempre cierto-.
    sinBarrera = ('{ char* h = strstr(l, HUECO_HORA); memcpy(h + 5, chip, 8); '
                  'reloj_leer(&fh); }')
    b.control_negativo(
        not (sinBarrera.find("reloj_leer") < sinBarrera.find("memcpy")),
        "un sello que escribe en la trama ANTES de preguntar a la barrera del reloj se "
        "detecta: es N-144 mudado de micro")

    crcAntes = '{ trama_checksum(l + 1); memcpy(h + 5, hhmmss, 8); }'
    b.control_negativo(
        not (crcAntes.find("memcpy") < crcAntes.find("trama_checksum")),
        "un sello que recalcula el checksum ANTES de escribir la hora se detecta: la "
        "trama saldria bien formada y la app la tiraria en silencio")

    b.control_negativo(
        len(re.sub(r"%02d", "XX", "%02d:%02d:%02d.%1d")) != largoHueco,
        "un formato que rindiera mas caracteres que el hueco se detecta: la cuenta "
        "reacciona al formato, no lo da por bueno")
