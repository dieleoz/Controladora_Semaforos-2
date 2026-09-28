# ===== banco/packs/enlace_02_silencio_j17.py =====
#
# EL REGISTRO DE SILENCIO DE J17: QUE CUENTE, QUE NO ACTUE, Y QUE NO SE ALIMENTE DEL
# SILENCIO DE LA RADIO.
#
# LA PROPIEDAD, EN UNA LINEA: las dos puntas llevan el MISMO registro del puerto del
# puente, sin un solo umbral, sin tocar una luz, y publicando las tres cifras que
# miden.
#
# POR QUE HACE FALTA UN PACK Y NO BASTA CON HABERLO ESCRITO BIEN.
#
# Este registro nace con tres formas de morir en silencio, y las tres son formas que
# este repositorio ya ha pagado:
#
#   1. QUE ALGUIEN LO ENGANCHE A SFTY-6. Es la tentacion evidente -"si el puerto lleva
#      25 s mudo, a ambar"- y seria de calle: SFTY6_SILENCIO_MS vigila la RADIO LoRa, y
#      un ESP32 colgado no es una radio caida. Peor todavia al reves: si el silencio de
#      J17 alimentara al de la radio, un telefono conectado SALVARIA al cruce de una
#      caida de radio real. El propio Esclavo/src/bluetooth.cpp ya lo dejo escrito
#      cuando el simulador del puente tumbo la primera version de su $EVENT de enlace:
#      "son dos silencios y dos instrumentos". Esto lo convierte en comprobacion.
#
#   2. QUE APAREZCA UN UMBRAL. Un silencio se define por sus DOS EXTREMOS -la linea que
#      lo abre y la que lo cierra-, nunca por un limite. Un limite aqui seria un numero
#      que nadie ha decidido gobernando lo que el tecnico ve, exactamente como el
#      "RF < 70% = degradado" que N-108 se nego a escribir. Y es ademas la precondicion
#      de (1): en cuanto haya un umbral, alguien lo comparara con el de la radio.
#
#   3. QUE UN CONTADOR SE ESCRIBA Y NO SE LEA. Es la prueba muerta de N-51 en su version
#      de almacenamiento: una cifra que se actualiza en cada linea, que cuesta RAM en un
#      equipo con 20 KB, y que no sale por ningun sitio. La primera version de este
#      registro estuvo a punto de dejar "el ultimo silencio" asi.
#
# Y UNA CUARTA, QUE ES LA QUE VUELVE VERDE PARA SIEMPRE: que alguien mueva el sello de
# tiempo ANTES de restar. La resta da cero, los contadores siguen contando lineas,
# la trama sigue saliendo, y el registro publica silencios de cero segundos para
# siempre sin que nada proteste. Es la forma exacta de la prueba 2.8 de N-51 -codigo
# correcto, resultado constante, nadie mirando-. El orden se comprueba abajo.
#
# LO QUE ESTE PACK NO PUEDE HACER, ESCRITO PARA QUE NADIE LO LEA COMO PERMISO.
#
# Esto es Python leyendo el .cpp. NO ejecuta el firmware, NO conecta un ESP32 y NO mide
# un silencio real. Y sobre todo no puede comprobar lo unico que de verdad importaria:
# que estos numeros signifiquen "el puente se murio". Hoy NO lo significan, y el motivo
# esta medido en el propio comentario del firmware -el puente no origina nada, la app no
# manda nada periodico, asi que por J17 entra solo lo que un dedo pulsa-. Miden silencio
# DEL PUERTO. Pasan a medir muerte DEL PUENTE el dia que el ESP32 emita un latido propio,
# que es AB-1 y es del responsable.
#
# SOBRE LAS ETIQUETAS SFTY: este pack NO lleva ninguna, y la tentacion era SFTY-6 porque
# la mitad de lo que comprueba es que este registro NO la toque. No la EJERCE: no mide el
# umbral de silencio de radio ni la maniobra de ambar -eso es costura_08_silencio-.
# Comprueba que un instrumento nuevo se mantenga fuera de ella. Una regla que aparece
# cubierta por una prueba que no la ejerce es peor que una fila vacia, porque la vacia no
# miente.

import re

NOMBRE = "enlace_02_silencio_j17"
DESCRIPCION = "el registro de silencio de J17 cuenta, no actua, no tiene umbral y publica lo que guarda"

PUNTAS = ("Maestro", "Esclavo")

# La funcion que cierra cada silencio. Es el sujeto entero de este pack: si cambia de
# nombre o desaparece, esto ABORTA en vez de aprobar midiendo un fichero sin ella -que
# es como el banco entero se quedo en ABORTADO en N-75 con cuatro defectos entrando
# detras-.
REGISTRADOR = "j17RegistrarLinea"

# El sello de tiempo contra el que se mide cada silencio. Se nombra aqui porque es el
# contrato del registro; lo que NO se escribe a mano es ninguno de sus valores.
SELLO = "tUltimaLineaJ17"

# LO QUE SOBREVIVE ENTRE LINEAS. Son dos y no tres: "el ultimo silencio" no se guarda,
# porque se publica en el instante en que se cierra y entre llamadas no lo lee nadie.
# Eso no es una opinion de estilo -MEDIDO: con la variable static puesta,
# arm-none-eabi-nm no la encontraba en .bss, el enlazador ya la habia borrado-. Un
# static que el enlazador borra es un estado anunciado que no existe.
CONTADORES = ("j17Silencios", "j17SilencioMaxMs")

# LAS TRES CIFRAS QUE LA TRAMA TIENE QUE LLEVAR. No coinciden con CONTADORES a
# proposito: la primera es la local que acaba de calcularse -el silencio que esta linea
# cierra, que es el que le interesa al tecnico que acaba de llegar- y las otras dos son
# el estado. Comprobar solo los contadores dejaria fuera justo la cifra que contesta
# "cuanto llevaba mudo esto antes de que yo pulsara".
PUBLICADAS = ("silencio", "j17SilencioMaxMs", "j17Silencios")

# El umbral de la RADIO. No es sujeto de este pack: es justo lo que no puede aparecer.
UMBRAL_RADIO = "SFTY6_SILENCIO_MS"

# Lo que mueve luz o decide modo. Si el registrador llama a cualquiera de estos, ha
# dejado de contar y ha empezado a mandar.
PREFIJOS_QUE_ACTUAN = ("semaforo_", "degradado_", "modo_degradado_", "coordinador_",
                       "modoActual_", "digitalWrite", "mando_", "demanda_")


def _cuerpo(codigo, patron, que, donde):
    """El cuerpo de la funcion que empieza en `patron`, con llaves emparejadas."""
    m = re.search(patron, codigo)
    if not m:
        raise Abortar(que, donde)
    i = codigo.find("{", m.end() - 1)
    if i < 0:
        raise Abortar(que, donde)
    prof = 0
    for j in range(i, len(codigo)):
        if codigo[j] == "{":
            prof += 1
        elif codigo[j] == "}":
            prof -= 1
            if prof == 0:
                return codigo[i + 1:j]
    raise Abortar(que, donde)


class _Falta(Exception):
    def __init__(self, que, donde):
        super().__init__("no se hallo %s en %s. Fallo el buscador o el bloque se movio, "
                         "y en los dos casos aprobar aqui seria aprobar sin mirar"
                         % (que, donde))


def Abortar(que, donde):
    return _Falta(que, donde)


def _argumentos(cola):
    """Los argumentos de un snprintf, partiendo por las comas de primer nivel."""
    cola = cola.strip()
    if cola.startswith(","):
        cola = cola[1:]
    cola = cola.rstrip(")").rstrip()
    partes, prof, act = [], 0, ""
    for c in cola:
        if c in "([":
            prof += 1
        elif c in ")]":
            prof -= 1
        if c == "," and prof == 0:
            partes.append(act.strip())
            act = ""
            continue
        act += c
    if act.strip():
        partes.append(act.strip())
    return partes


def _peor_caso(formato, args):
    """La cadena mas larga que ese printf PUEDE producir.

    NO se dimensiona "por lo que uno espera ver" -ese es el razonamiento con el que a
    N-108 se le truncaba el $ALARM por la HORA sin que nada lo dijera-. Se dimensiona
    por lo que la aritmetica garantiza, que es lo unico que este lado puede garantizar:

      %lu suelto                un unsigned long de 32 bits: 4294967295, DIEZ cifras.
      %lu alimentado por x/1000 x es una resta de millis(), o sea un unsigned long, de
                                modo que el cociente no puede pasar de 4294967: SIETE
                                cifras. No es una esperanza sobre el valor, es el techo
                                del dividendo.

    Reservar diez cifras para el cociente no seria mas prudente: seria dimensionar para
    un valor que la division no puede producir, y esa holgura falsa se paga cuando
    empuja a alguien a recortar el campo que si la necesita."""
    convs = re.findall(r"%l?u", formato)
    if len(convs) != len(args):
        return None
    salida = formato
    for a in args:
        cifras = 7 if re.search(r"/\s*1000UL?\b", a) else 10
        salida = re.sub(r"%l?u", "9" * cifras, salida, count=1)
    return salida


def correr(b, fw):
    b.titulo("El registro de silencio de J17: cuenta, no actua, y no tiene umbral")

    # --- Se lee todo del C++ ANTES de comprobar nada. Si algo no esta, se aborta. ---
    codigo, registrador, recepcion = {}, {}, {}
    try:
        for p in PUNTAS:
            codigo[p] = fw.codigo(p, "src", "bluetooth.cpp")
            registrador[p] = _cuerpo(
                codigo[p], r"static\s+void\s+%s\s*\(" % REGISTRADOR,
                "%s()" % REGISTRADOR, "%s/src/bluetooth.cpp" % p)
            recepcion[p] = _cuerpo(
                codigo[p], r"void\s+bluetooth_loop\s*\(",
                "bluetooth_loop()", "%s/src/bluetooth.cpp" % p)
    except _Falta as e:
        raise fw.Abortado(str(e))

    # ---- 1. Las dos puntas llevan el MISMO registro ----
    #
    # bluetooth.cpp NO es un fichero compartido -cada punta tiene su despachador-, asi
    # que esto puede divergir sin que nada lo note. Es la misma razon por la que
    # enlace_01 compara el framing de las dos puntas a mano, y es como el enclavamiento
    # SFTY-2 estuvo divergiendo entre puntas sin que nadie lo viera.
    b.verificar(
        re.sub(r"\s+", " ", registrador["Maestro"]).strip()
        == re.sub(r"\s+", " ", registrador["Esclavo"]).strip(),
        "%s() es el MISMO codigo en las dos puntas" % REGISTRADOR,
        "%s() ya NO es el mismo en las dos puntas. Un registro que diverge da dos "
        "bitacoras que no se pueden comparar, y el tecnico que cambia de poste no "
        "puede saber cual de las dos le esta mintiendo" % REGISTRADOR)

    for p in PUNTAS:
        # ---- 2. El sello y los tres contadores existen, y son static ----
        #
        # static importa por N-86: un objeto global con constructor corre en cada
        # arranque y su .bss no lo puede tirar el enlazador. Estos son enteros planos
        # con enlace interno, que es lo mas barato que puede haber, y escribirlo aqui
        # impide que manana alguien los convierta en una clase "para ordenarlo".
        declarados = [n for n in (SELLO,) + CONTADORES
                      if re.search(r"static\s+unsigned\s+long\s+%s\b" % n, codigo[p])]
        b.verificar(
            len(declarados) == 1 + len(CONTADORES),
            "%s: el sello y los %d contadores estan declarados como `static unsigned "
            "long` -enteros planos, sin constructor y con enlace interno-"
            % (p, len(CONTADORES)),
            "%s: de %s solo estan como `static unsigned long` %s. Un contador que "
            "cambia de tipo o de enlace deja de ser gratis: un objeto con constructor "
            "corre en cada arranque y su .bss no lo tira el enlazador (N-86)"
            % (p, list((SELLO,) + CONTADORES), declarados))

        # ---- 3. NO HAY UMBRAL: ni un numero desnudo en una comparacion ----
        #
        # Se busca la forma exacta del defecto -el literal comparado-, no cualquier
        # numero: costura_08 ya se equivoco acusando a una constante con nombre que
        # valia lo mismo. Aqui el unico numero permitido es el divisor de ms a
        # segundos, que no es una comparacion sino una conversion de unidades.
        umbrales = re.findall(r"[<>]=?\s*(\d+)", registrador[p])
        b.verificar(
            not umbrales,
            "%s: %s() no compara contra ningun numero: un silencio se define por sus "
            "dos extremos, no por un limite" % (p, REGISTRADOR),
            "%s: %s() compara contra %s. Un umbral aqui es un numero que nadie ha "
            "decidido gobernando lo que el tecnico ve, y es la precondicion para que "
            "alguien acabe alimentando este silencio con el de la radio"
            % (p, REGISTRADOR, umbrales))

        # ---- 4. Y EL SILENCIO DE LA RADIO NO SE NOMBRA EN ESTE FICHERO ----
        #
        # Esta es la comprobacion que el encargo pedia de frente. No basta con que hoy
        # no se use: basta con que aparezca una vez para que la siguiente persona lo
        # compare, y de ahi a que un ESP32 colgado mande el cruce a ambar -o a que un
        # telefono conectado SALVE al cruce de una caida de radio real- hay una linea.
        b.verificar(
            UMBRAL_RADIO not in codigo[p],
            "%s: %s no se nombra en bluetooth.cpp. Son dos silencios y dos "
            "instrumentos: el de la radio lo mide costura_08" % (p, UMBRAL_RADIO),
            "%s: bluetooth.cpp nombra %s. Ese umbral es el de la RADIO LoRa; traerlo al "
            "fichero del puerto del telefono es como un ESP32 colgado acaba mandando el "
            "cruce a ambar, y como un telefono conectado acaba salvandolo de una caida "
            "de radio de verdad" % (p, UMBRAL_RADIO))

        # ---- 5. ESTO CUENTA; NO ACTUA ----
        actuadores = sorted({m for m in re.findall(r"\b(\w+)\s*\(", registrador[p])
                             if m.startswith(PREFIJOS_QUE_ACTUAN)})
        b.verificar(
            not actuadores,
            "%s: %s() no llama a nada que mueva luz ni decida modo" % (p, REGISTRADOR),
            "%s: %s() llama a %s. El ambar automatico queda reservado a los caminos que "
            "ya lo tienen -SFTY-6 y el watchdog-: la maquina no decide sola operar de un "
            "modo que nadie pidio, y menos porque se callara un accesorio"
            % (p, REGISTRADOR, actuadores))

        # ---- 6. EL ORDEN: se resta ANTES de mover el sello ----
        #
        # La que vuelve verde para siempre. Con el sello movido primero la resta da cero,
        # los contadores siguen subiendo y la trama sigue saliendo: un registro que
        # publica silencios de cero segundos y no protesta jamas.
        resta = re.search(r"=\s*ahora\s*-\s*%s\s*;" % SELLO, registrador[p])
        asigna = re.search(r"%s\s*=\s*ahora\s*;" % SELLO, registrador[p])
        b.verificar(
            resta is not None and asigna is not None and resta.start() < asigna.start(),
            "%s: el silencio se calcula CONTRA el sello anterior y solo despues se mueve "
            "el sello" % p,
            "%s: el sello %s se mueve antes de restar contra el -o una de las dos lineas "
            "no esta-. La resta daria cero en cada linea: el registro seguiria contando, "
            "seguiria emitiendo y publicaria silencios de cero segundos para siempre, sin "
            "fallar nunca" % (p, SELLO))

        # ---- 7. LOS TRES CONTADORES SE PUBLICAN ----
        #
        # Un contador que se escribe y no se lee es la prueba muerta en su version de
        # almacenamiento: cuesta RAM en un equipo de 20 KB y no contesta a nadie.
        detalle = re.search(r'snprintf\s*\(\s*det\s*,[^;]*?"([^"]+)"([^;]*);',
                            registrador[p], re.S)
        if detalle is None:
            raise fw.Abortado(
                "%s: no se halla el snprintf que compone el detalle dentro de %s(). Sin "
                "el no se puede comprobar que lo que se guarda se publique, y aprobar "
                "seria aprobar sin mirar" % (p, REGISTRADOR))
        publicados = [c for c in PUBLICADAS if re.search(r"\b%s\b" % c, detalle.group(2))]
        b.verificar(
            len(publicados) == len(PUBLICADAS),
            "%s: las %d cifras del registro salen en la trama: %s"
            % (p, len(PUBLICADAS), ", ".join(publicados)),
            "%s: %s se calcula(n) o se guarda(n) y no sale(n) en ninguna trama. Una "
            "cifra que nadie lee no es un registro: es RAM gastada en un equipo de 20 KB "
            "y un hueco en la bitacora que se lee como 'no paso' en vez de 'no lo apunte'"
            % (p, [c for c in PUBLICADAS if c not in publicados]))

        # ---- 8. Y LA TRAMA CABE, dimensionada para el TIPO ----
        #
        # Las tres medidas se leen del C++, sin valor por defecto: el buffer del detalle,
        # el buffer del $EVENT y el formato de los dos. Es la cuenta que a N-108 le falto
        # hacer, y por eso el $ALARM se truncaba por la HORA sin que nada lo dijera.
        det_buf = fw.constante((p, "src", "bluetooth.cpp"),
                               r"char\s+det\s*\[\s*(\d+)\s*\]\s*;\s*\n\s*snprintf\s*\(\s*det\s*,[^;]*?%s"
                               % re.escape(detalle.group(1)[:6]),
                               "el buffer del detalle de %s" % REGISTRADOR)
        peor_det = _peor_caso(detalle.group(1), _argumentos(detalle.group(2)))
        if peor_det is None:
            raise fw.Abortado(
                "%s: el formato del detalle tiene %d conversiones y el snprintf le pasa "
                "%d argumentos. O el buscador partio mal la llamada o el firmware tiene "
                "un printf descuadrado; en los dos casos medir el peor caso seria "
                "inventarselo" % (p, len(re.findall(r"%l?u", detalle.group(1))),
                                  len(_argumentos(detalle.group(2)))))
        b.verificar(
            len(peor_det) + 1 <= det_buf,
            "%s: el peor detalle que la aritmetica permite son %d B y det[%d] los "
            "aguanta con el nulo"
            % (p, len(peor_det), det_buf),
            "%s: el peor detalle son %d B y det[%d] no los aguanta. snprintf trunca EN "
            "SILENCIO: la cifra se corta y nadie se entera" % (p, len(peor_det), det_buf))

        ev_fmt = re.search(r'char\s+payload\s*\[\s*(\d+)\s*\]\s*;\s*\n\s*snprintf\s*\(\s*payload\s*,[^;]*?"(\$EVENT[^"]*)"',
                           codigo[p], re.S)
        if ev_fmt is None:
            raise fw.Abortado(
                "%s: no se halla el snprintf del $EVENT ni su payload[]. Sin el buffer "
                "real no se puede saber si la trama del registro cabe" % p)
        # El envoltorio con sus tres %s ya resueltos: NODE es literal en el formato, y
        # ORIGEN y DETALLE los pone el registrador. HORA son los 8 de "HH:MM:SS".
        origen = re.search(r'%s\s*\(\s*"([^"]*)"' % re.escape("bluetooth_reportarEvento"),
                           registrador[p])
        if origen is None:
            raise fw.Abortado(
                "%s: %s() ya no llama a bluetooth_reportarEvento() con un ORIGEN "
                "literal. Sin saber por que puerta sale la trama no se puede medir si "
                "cabe -ni si sale-" % (p, REGISTRADOR))
        env = ev_fmt.group(2)
        n_s = env.count("%s")
        if n_s != 3:
            raise fw.Abortado(
                "%s: el formato del $EVENT tiene %d '%%s' y se esperaban 3 -ORIGEN, "
                "DETALLE y HORA-. La cuenta del peor caso saldria de otra trama que la "
                "que el firmware emite" % (p, n_s))
        # NODE va literal DENTRO del formato, asi que ya esta contado en len(env). Lo
        # que se sustituye son los tres %s: el ORIGEN y el DETALLE que pone el registro,
        # y los 8 de "HH:MM:SS" -que es el caso largo: el "--:--:--" mide lo mismo-.
        largo = len(env) - 2 * n_s + len(origen.group(1)) + len(peor_det) + 8
        b.verificar(
            largo + 1 <= int(ev_fmt.group(1)),
            "%s: la trama del registro en su peor caso son %d B y payload[%s] los "
            "aguanta" % (p, largo, ev_fmt.group(1)),
            "%s: la trama del registro son %d B en su peor caso y payload[%s] no los "
            "aguanta. Se truncaria por la HORA, que es exactamente como N-108 encontro "
            "el $ALARM cortandose sin aviso" % (p, largo, ev_fmt.group(1)))

        # ---- 9. Y TIENE LLAMADOR, EN EL CAMINO DE RECEPCION ----
        #
        # N-73: una funcion declarada que nadie llama es la version silenciosa de la
        # prueba muerta, y aqui seria peor -un registro anunciado que no registra-. No
        # basta con que exista un llamador: tiene que estar en el bucle de recepcion,
        # que es lo unico que sabe cuando llego una linea.
        b.verificar(
            re.search(r"%s\s*\(" % REGISTRADOR, recepcion[p]) is not None,
            "%s: %s() se llama desde bluetooth_loop(), que es el unico sitio que sabe "
            "cuando entro una linea" % (p, REGISTRADOR),
            "%s: %s() NO se llama desde bluetooth_loop(). Un registro sin llamador es la "
            "Caja Negra de N-73 otra vez: declarada, documentada y sin un solo sitio que "
            "la invoque" % (p, REGISTRADOR))

        # Y DESPUES de despachar. El orden es vial: la linea que entra puede ser la de
        # emergencia, y anteponerle una trama de bitacora le mete su tiempo de cable por
        # delante. A 9600 bps una trama de ~100 B son ~105 ms de retraso en la orden que
        # para el cruce. Es la leccion de 8.sexies: una inversion que solo mira el
        # resultado final aprueba un firmware con las barreras en el orden equivocado.
        desp = re.search(r"procesarComando\s*\(", recepcion[p])
        reg = re.search(r"%s\s*\(" % REGISTRADOR, recepcion[p])
        b.verificar(
            desp is not None and reg is not None and desp.start() < reg.start(),
            "%s: el registro se anota DESPUES de despachar la linea: la orden no espera "
            "a la bitacora" % p,
            "%s: el registro se anota ANTES de procesarComando(). La linea que entra "
            "puede ser la de emergencia, y a 9600 bps una trama de bitacora por delante "
            "son ~105 ms de retraso en la orden que para el cruce" % p)

    # ---- 10. Controles negativos ----
    #
    # Cada uno reproduce el defecto que su comprobacion de arriba tiene que cazar, sobre
    # un bloque sintetico. Sin esto, el dia que un patron dejara de acertar, todas las
    # comprobaciones de arriba pasarian comparando contra nada.
    malo_umbral = "{ const unsigned long s = ahora - t; if (s > 25000) alarma(); }"
    b.control_negativo(
        bool(re.findall(r"[<>]=?\s*(\d+)", malo_umbral))
        and not re.findall(r"[<>]=?\s*(\d+)", registrador["Maestro"]),
        "un umbral colado en el registrador se detecta, y el registrador real no lo "
        "tiene")

    malo_orden = "{ %s = ahora; const unsigned long s = ahora - %s; }" % (SELLO, SELLO)
    r_m = re.search(r"=\s*ahora\s*-\s*%s\s*;" % SELLO, malo_orden)
    a_m = re.search(r"%s\s*=\s*ahora\s*;" % SELLO, malo_orden)
    b.control_negativo(
        r_m is not None and a_m is not None and a_m.start() < r_m.start(),
        "el detector del ORDEN ve el sello movido antes de la resta -el defecto que "
        "publicaria ceros para siempre sin fallar nunca-")

    malo_actua = "{ semaforo_iniciarFallo(); }"
    b.control_negativo(
        bool({m for m in re.findall(r"\b(\w+)\s*\(", malo_actua)
              if m.startswith(PREFIJOS_QUE_ACTUAN)}),
        "el censo de actuadores caza un semaforo_iniciarFallo() colado en el registrador")

    b.control_negativo(
        UMBRAL_RADIO in "if (ahora - t > SFTY6_SILENCIO_MS) {}",
        "el detector del umbral de radio lo encuentra cuando esta")

    b.control_negativo(
        _peor_caso("A:%lu,B:%lu", ["x / 1000UL", "y"])
        == "A:9999999,B:9999999999",
        "el peor caso da SIETE cifras al %lu que viene de una division por 1000 y DIEZ "
        "al que no: se dimensiona por la aritmetica, no por lo que uno espera ver")
    b.control_negativo(
        _peor_caso("A:%lu,B:%lu", ["x"]) is None,
        "un printf con mas conversiones que argumentos no se mide a ojo: devuelve nada "
        "y el pack aborta en vez de inventarse el peor caso")
    b.control_negativo(
        _argumentos(', a, snprintf(b, sizeof(b), "%d", c), d)') == [
            "a", 'snprintf(b, sizeof(b), "%d", c)', "d"],
        "el partidor de argumentos respeta los parentesis y no corta una llamada "
        "anidada por sus comas")

    b.control_negativo(
        [c for c in PUBLICADAS
         if re.search(r"\b%s\b" % c, "snprintf(det, sizeof(det), \"N:%lu\", j17Silencios);")]
        != list(PUBLICADAS),
        "el censo de publicadas NO da por buena una trama a la que le falta el silencio "
        "que la linea acaba de cerrar")
