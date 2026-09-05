# ===== banco/packs/camara_03_vigilante.py =====
#
# EL VIGILANTE DE LAS CAMARAS: D-13 FASE 1, LA QUE NO PUEDE DEGRADAR NADA.
#
# ---------------------------------------------------------------------------------
# POR QUE ESTA FASE VA PRIMERA, QUE ES LO QUE ESTE PACK TIENE QUE PROTEGER
#
# A-6, con sus palabras: "con INPUT pelado y pull-down, EL PIN NO DISTINGUE SILENCIO DE
# VIA LIBRE". Una camara desconectada y una camara que no ve a nadie dan el mismo nivel
# bajo. Todo lo que se apoye en ese bit -y el veto de la fase 2 se apoya entero- esta
# apoyado en un dato que no sabe decir que no sabe.
#
# El vigilante le ensena a decirlo, y ademas CUENTA cuantas veces habria actuado el veto
# antes de que nadie le de autoridad. Es el instrumento de laboratorio de D-13, y su
# unica salida son eventos: no toca el ciclo, no toca la pluma, no da ni quita verde.
#
# ---------------------------------------------------------------------------------
# LO QUE ESTE PACK NO PUEDE VER, ESCRITO ARRIBA PARA QUE NADIE LO LEA COMO PERMISO
#
# Es Python que PARSEA C++. Comprueba la forma: que los umbrales existan, que se
# relacionen entre si como tienen que relacionarse, que el silencio no se acumule con el
# cruce parado, que nada de aqui mueva una luz y que las tramas quepan.
#
# NO EJERCE EL TIEMPO. Que a las 6 h de paso abierto salga el $ALARM, que a los 20 min de
# contacto fijo salga el otro, y que una camara sana con trafico normal NO alarme nunca,
# eso solo lo demuestra una tarjeta -o un arnes que compile este .cpp-. Lo que el pack si
# puede hacer, y hace, es comprobar las tres piezas SIN LAS CUALES una camara sana
# alarmaria igual: que el flanco reinicie el cronometro de silencio, que el flanco
# reinicie el de nivel, y que el silencio no corra con la pluma abajo. Si alguna de las
# tres se cae, el vigilante pasa a ser una tapia que alarma siempre (CLAUDE.md 8.sexies),
# y las tres se han visto fallar inyectando el defecto en el .cpp real.
#
# ---------------------------------------------------------------------------------
# POR QUE NO LLEVA ETIQUETA "EJERCE SFTY-x"
#
# La regla que este codigo rozaria es SFTY-29 -"presencia en el tramo: un VETO, nunca un
# atajo"-, y OPTIMIZACIONES.md la declara DISENO, NO IMPLEMENTADO. La fase 1 no construye
# el veto: lo CUENTA. Etiquetarla aqui pondria una fila verde en la tabla de trazabilidad
# por una regla que sigue sin construirse, y CLAUDE.md lo dice sin rodeos: una regla que
# aparece cubierta por una prueba que no la ejerce es peor que una fila vacia, porque la
# vacia no miente.
#
# La barrera de salidas aplicada a botones.cpp -SFTY-2- ya la mide camara_02_j16 sobre el
# fichero entero, asi que cubre el codigo nuevo sin que haga falta repetirla. Aqui solo se
# anade lo que aquel no puede saber: que las funciones DEL VIGILANTE no llamen a nada que
# ordene.

import re

NOMBRE = "camara_03_vigilante"
DESCRIPCION = "el vigilante de camaras avisa y cuenta, no actua, y sus dos umbrales se sostienen"

PUNTAS = ("Maestro", "Esclavo")

# Las funciones del vigilante. Si alguna se renombra el pack ABORTA en vez de aprobar: un
# patron que no encuentra nada NO demuestra que no haya nada (CLAUDE.md 4).
VIGILANTE = ("camara_estado", "camara_vetosPluma", "camara_alarmar",
             "camara_recuperada", "vigilante_flanco", "vigilante_nivel",
             "vigilante_tick")

# Nada de esto puede aparecer dentro de una funcion del vigilante. La fase 1 no actua:
# cuenta y avisa. Si alguna vez hace falta que actue, eso es la fase 2, contradice SFTY-28
# y necesita derogacion escrita del responsable (A-1.bis).
ORDENAN = ("coordinador_pedirCambio", "coordinador_configurar", "coordinador_iniciarModo",
           "semaforo_forzarRojo", "semaforo_forzarVerde", "semaforo_iniciarFallo",
           "semaforo_toggle", "semaforo_iniciarTransicionAVerde", "digitalWrite",
           "MOTOR_TALANQUERA", "TALANQUERA_ABRIR", "TALANQUERA_CERRAR")

# Los cuatro valores del campo CAM: de D-13. Un getter que solo supiera decir "OK" seria
# el enum de un solo valor de 3.septies con otra ropa: una guarda que no puede dar las dos
# respuestas.
VALORES_CAM = ("OK", "CIEGA", "PEGADA", "?")

# La hora que las dos tramas llevan al final: formato fijo HH:MM:SS o --:--:--, ocho
# caracteres en los dos casos. Es lo unico del presupuesto que no se puede leer de un
# buffer, asi que se escribe aqui y se dice por que.
LARGO_HORA = 8

_DEF = re.compile(
    r"^(?:static\s+)?(?:void|bool|int|uint8_t|uint16_t|unsigned\s+long|"
    r"const\s+char\s*\*|EstadoCamara)\s+(\w+)\s*\([^)]*\)\s*\{", re.M)


# ---------------------------------------------------------------------------------
# HERRAMIENTAS. El delimitador de bloques y el clasificador por funcion vienen LITERALES
# de camara_02_j16, que a su vez los trajo de maestro_09_test_leds: no se reescribe logica
# ya probada para renombrar llamadas, que es como se cuelan los errores en un cambio que
# no debe cambiar comportamiento.

def _bloque(codigo, i):
    """[inicio, fin] del bloque que abre en codigo[i] == '{'. None si no cierra."""
    nivel = 0
    for j in range(i, len(codigo)):
        if codigo[j] == "{":
            nivel += 1
        elif codigo[j] == "}":
            nivel -= 1
            if nivel == 0:
                return (i, j + 1)
    return None


def _funciones(codigo):
    """[(nombre, inicio, fin)] de cada definicion de funcion del fichero."""
    fuera = []
    for m in _DEF.finditer(codigo):
        i = codigo.index("{", m.end() - 1)
        tramo = _bloque(codigo, i)
        if tramo:
            fuera.append((m.group(1), tramo[0], tramo[1]))
    return fuera


def _cuerpo(codigo, nombre):
    for n, ini, fin in _funciones(codigo):
        if n == nombre:
            return codigo[ini:fin]
    return None


def _cte(fw, punta, nombre):
    """Un unsigned long del botones.cpp de una punta. ABORTA si no aparece.

    Sin valor por defecto, nunca: un banco que no puede fallar no demuestra nada, y el
    dia que alguien renombre el umbral esto seguiria dando PASS midiendo el viejo."""
    return fw.constante((punta, "src", "botones.cpp"),
                        r"%s\s*=\s*(\d+)UL" % nombre,
                        "el umbral %s del vigilante del %s" % (nombre, punta))


def _peor(fmt, nombres, digitos_u=5):
    """El caso peor de un snprintf del vigilante, sustituyendo cada marca por su maximo.

    %s se sustituye por el nombre de camara mas largo -son los unicos %s que el vigilante
    pasa- y %u por 65535, que es donde satura el contador."""
    salida = fmt.replace("%s", max(nombres, key=len)).replace("%u", "9" * digitos_u)
    return len(salida)


def correr(b, fw):
    b.titulo("El vigilante de camaras: cuenta y avisa, y sus umbrales se sostienen")

    codigo = {p: fw.codigo(p, "src", "botones.cpp") for p in PUNTAS}
    texto = {p: fw.texto(p, "src", "botones.cpp") for p in PUNTAS}
    cabecera = {p: fw.codigo(p, "include", "botones.h") for p in PUNTAS}

    cuerpos = {}
    for punta in PUNTAS:
        for fn in VIGILANTE:
            c = _cuerpo(codigo[punta], fn)
            if c is None:
                raise fw.Abortado(
                    "no se encuentra %s() en %s/src/botones.cpp. O se renombro, o el "
                    "patron se quedo ciego; en los dos casos este pack no puede medir el "
                    "vigilante y su PASS no valdria nada" % (fn, punta))
            cuerpos[(punta, fn)] = c

    # =============================================================================
    # 1. LOS DOS UMBRALES, Y LAS DOS DESIGUALDADES QUE LOS SOSTIENEN
    # =============================================================================
    #
    # Es N-71 literal. Alli el techo de silencio de SFTY-6 vivia en 12 s mientras el ciclo
    # necesitaba 20,5 s para agotar sus cinco reintentos: los reintentos 4 y 5 NO PODIAN
    # EJECUTARSE JAMAS, y nada lo delataba porque la relacion entre los tres numeros vivia
    # SOLO EN PROSA, dentro de un comentario. Los comentarios no fallan cuando alguien
    # cambia un numero: se quedan describiendo un equipo que ya no existe, con la
    # autoridad de una cuenta hecha.
    pegada = {p: _cte(fw, p, "CAM_PEGADA_MS") for p in PUNTAS}
    ciega = {p: _cte(fw, p, "CAM_CIEGA_MS") for p in PUNTAS}

    b.verificar(
        pegada["Maestro"] == pegada["Esclavo"] and ciega["Maestro"] == ciega["Esclavo"],
        "los dos umbrales del vigilante son los mismos en las dos puntas (%d ms de "
        "contacto fijo, %d ms de paso abierto sin flanco)"
        % (pegada["Maestro"], ciega["Maestro"]),
        "los umbrales no coinciden entre puntas: PEGADA %s, CIEGA %s. Las dos camaras "
        "son el mismo modelo con la misma configuracion (D-13): dos criterios distintos "
        "harian que el mismo fallo se anunciara en un poste y no en el otro"
        % (pegada, ciega))

    # -- 1.1 El techo de PEGADA sale de los maximos del ciclo, releidos del C++ --
    #
    # La camara vigila el BARRIDO DE LA PLUMA, no la zona de espera: lo unico que ocupa la
    # region es alguien cruzando, y solo se cruza con la pluma arriba, o sea durante un
    # verde y su despeje. El techo de eso son los maximos de limites_ciclo.h.
    #
    # POR QUE SE LEE DEL MAESTRO Y VALE PARA LOS DOS: limites_ciclo.h existe solo en el
    # Maestro, que es quien decide el ciclo (SFTY-27). El Esclavo obedece, asi que el
    # maximo de verde que puede ver es el mismo numero.
    verde_max = fw.constante(("Maestro", "include", "limites_ciclo.h"),
                             r"VERDE_MIN_MAX\s*=\s*(\d+)",
                             "el maximo de verde por sentido")
    despeje_max = fw.constante(("Maestro", "include", "limites_ciclo.h"),
                               r"DESPEJE_SEG_MAX\s*=\s*(\d+)",
                               "el maximo de despeje")
    techo_ms = (verde_max * 60 + despeje_max) * 1000

    b.verificar(
        pegada["Maestro"] > techo_ms,
        "el umbral de PEGADA (%d ms) esta por encima del paso abierto mas largo que el "
        "ciclo admite -%d min de verde + %d s de despeje = %d ms-: ninguna cola de "
        "vehiculos puede tener el contacto cerrado tanto tiempo sin abrirse ni una vez"
        % (pegada["Maestro"], verde_max, despeje_max, techo_ms),
        "el umbral de PEGADA (%d ms) NO supera el paso abierto mas largo del ciclo (%d "
        "ms, de VERDE_MIN_MAX=%d min + DESPEJE_SEG_MAX=%d s). Una cola larga cruzando en "
        "el verde mas largo configurable levantaria la alarma de camara pegada sin que "
        "haya nada averiado, y una alarma que salta sola deja de leerse a los tres dias"
        % (pegada["Maestro"], techo_ms, verde_max, despeje_max))

    # -- 1.2 Y CIEGA tiene que llegar DESPUES que PEGADA --
    #
    # Una camara pegada tampoco produce flancos, asi que su cronometro de silencio corre
    # igual. Si CIEGA fuera el umbral menor, un contacto trabado se anunciaria como CIEGA
    # -el diagnostico CONTRARIO- y el tecnico saldria a buscar un cable cortado teniendo
    # un rele cerrado. No es una preferencia de estilo: es la unica forma de que las dos
    # alarmas signifiquen dos cosas distintas.
    b.verificar(
        ciega["Maestro"] > pegada["Maestro"],
        "el umbral de CIEGA (%d ms) llega despues que el de PEGADA (%d ms): un contacto "
        "trabado se anuncia por lo que es y no como una camara muda"
        % (ciega["Maestro"], pegada["Maestro"]),
        "el umbral de CIEGA (%d ms) es MENOR o igual que el de PEGADA (%d ms). Una camara "
        "pegada tampoco da flancos: con este orden se anunciaria CIEGA, que manda a "
        "buscar un cable cortado cuando lo que hay es un contacto cerrado"
        % (ciega["Maestro"], pegada["Maestro"]))

    # =============================================================================
    # 2. LAS TRES PIEZAS SIN LAS CUALES UNA CAMARA SANA ALARMARIA IGUAL
    # =============================================================================
    #
    # CLAUDE.md 8.sexies: un vigilante que alarme siempre no mide nada. Aqui esta el caso
    # que NO debe alarmar, en la unica forma que un pack de texto puede comprobarlo: las
    # tres condiciones cuya ausencia convierte al vigilante en una tapia.
    for punta in PUNTAS:
        flanco = cuerpos[(punta, "vigilante_flanco")]

        # 2.1 Un flanco reinicia el cronometro de SILENCIO. Sin esto, una camara sana con
        #     trafico normal acumularia hasta CIEGA igual que una muda.
        b.verificar(
            re.search(r"camSinFlancoMs\[i\]\s*=\s*0\s*;", flanco) is not None,
            "%s: un flanco pone a cero el cronometro de silencio: una camara que ve NO "
            "puede llegar nunca a CIEGA" % punta,
            "%s: vigilante_flanco() no reinicia camSinFlancoMs[]. Entonces el contador "
            "solo sube, y la alarma de camara ciega la acabaria dando TAMBIEN una camara "
            "sana con trafico normal: el vigilante deja de distinguir y pasa a ser una "
            "tapia que alarma siempre" % punta)

        # 2.2 Y reinicia el de NIVEL. Sin esto, una camara que ve mucho -contacto cerrando
        #     y abriendo- acumularia hasta PEGADA por el mero hecho de funcionar.
        b.verificar(
            re.search(r"camAltoDesde\[i\]\s*=\s*ahora\s*;", flanco) is not None,
            "%s: un flanco reinicia el cronometro de nivel sostenido: cada deteccion "
            "nueva empieza a contar de cero" % punta,
            "%s: vigilante_flanco() no reinicia camAltoDesde[]. Sin ese reinicio el "
            "cronometro de PEGADA arrancaria en la primera deteccion y no volveria a "
            "ponerse a cero: la camara mas ocupada del cruce seria la primera en "
            "anunciarse averiada" % punta)

        # 2.3 Y el silencio NO corre con la pluma abajo.
        #
        #     Este es el "con el ciclo corriendo" de D-13, y es la puerta que impide que el
        #     vigilante acuse a una camara callada mientras el cruce esta parado -menu,
        #     rojo total, esta punta sin turno-: con la pluma abajo nadie puede cruzar el
        #     barrido, asi que no ver nada es la respuesta correcta.
        tick = cuerpos[(punta, "vigilante_tick")]
        m = re.search(r"if\s*\(\s*arriba\s*\)\s*\{", tick)
        dentro = None
        if m:
            tramo = _bloque(tick, tick.index("{", m.end() - 1))
            if tramo:
                dentro = tick[tramo[0]:tramo[1]]
        b.verificar(
            dentro is not None and "camSinFlancoMs[i] +=" in dentro
            and re.search(r"arriba\s*=\s*semaforo_plumaArriba\s*\(", tick) is not None,
            "%s: el silencio se acumula SOLO con la pluma arriba, y 'arriba' sale de "
            "semaforo_plumaArriba(): con el cruce parado una camara callada no es una "
            "camara rota" % punta,
            "%s: la acumulacion de silencio no esta dentro del 'if (arriba)', o 'arriba' "
            "no sale de semaforo_plumaArriba(). Entonces el vigilante cuenta tiempo de "
            "RELOJ: un equipo en menu o en rojo total un fin de semana entero saldria "
            "acusando de ciegas a dos camaras que estan perfectamente" % punta)

    # =============================================================================
    # 3. LA FASE 1 NO ACTUA: NI UNA LUZ, NI LA PLUMA
    # =============================================================================
    #
    # Es el limite que el encargo puso y el que A-1.bis explica: un veto que deje la pluma
    # arriba en rojo rompe el invariante de SFTY-28 y de Validacion_Automatico, y hace
    # falta derogarlo POR ESCRITO. Mientras eso no exista, esto solo puede contar.
    for punta in PUNTAS:
        ordena = []
        for fn in VIGILANTE:
            cuerpo = cuerpos[(punta, fn)]
            for n in ORDENAN:
                if re.search(r"\b%s\b" % re.escape(n), cuerpo):
                    ordena.append("%s() -> %s" % (fn, n))
        b.verificar(
            not ordena,
            "%s: ninguna de las %d funciones del vigilante llama a nada que mueva una luz "
            "ni la pluma: la fase 1 cuenta y avisa" % (punta, len(VIGILANTE)),
            "%s: el vigilante ACTUA: %s. La fase 1 de D-13 tiene cero efecto vial por "
            "definicion; lo que ahi se ha metido es la fase 2, contradice SFTY-28 y "
            "necesita la derogacion escrita de A-1.bis antes de existir"
            % (punta, ", ".join(ordena)))

        # Lo unico que el vigilante puede saber del semaforo es LEERLO, y por un solo
        # sitio: semaforo_plumaArriba(), que devuelve lo que escribirPines() dejo puesto y
        # no una segunda formula de SFTY-28 que alguien tendria que mantener igual.
        llamadas = set()
        for fn in VIGILANTE:
            llamadas |= set(re.findall(r"\b(semaforo_\w+)\s*\(", cuerpos[(punta, fn)]))
        b.verificar(
            llamadas == {"semaforo_plumaArriba"},
            "%s: del semaforo el vigilante solo usa semaforo_plumaArriba(), que es un "
            "getter de lo que ya se escribio en el pin" % punta,
            "%s: el vigilante usa %s del semaforo. Lo unico que puede hacer con el "
            "semaforo es LEERLO, y por un solo sitio: cualquier otra cosa es la fase 2 o "
            "una segunda copia de la condicion de SFTY-28"
            % (punta, sorted(llamadas) or "(nada: fallo el buscador, no el firmware)"))

    # =============================================================================
    # 4. EL CAMPO CAM: SABE DECIR LAS CUATRO COSAS, Y EL "?" ES UNA DE ELLAS
    # =============================================================================
    for punta in PUNTAS:
        cuerpo = cuerpos[(punta, "camara_estado")]
        devueltos = set(re.findall(r'return\s+"([^"]*)"\s*;', cuerpo))
        b.verificar(
            devueltos == set(VALORES_CAM),
            "%s: camara_estado() puede devolver los cuatro valores de D-13 (%s), el '?' "
            "incluido" % (punta, ", ".join(sorted(VALORES_CAM))),
            "%s: camara_estado() devuelve %s y D-13 pide %s. Un getter que no pueda decir "
            "'?' obliga a inventarse un estado mientras no se sabe, que es exactamente lo "
            "que el pin no puede distinguir (A-6); y uno que no pueda decir todos los "
            "demas es una guarda que no da las dos respuestas (CLAUDE.md 3.septies)"
            % (punta, sorted(devueltos) or "(nada)", sorted(VALORES_CAM)))

        # Y el enum de gravedad tiene cuatro valores DE VERDAD: si se quedara en uno, el
        # mayor-que de camara_estado() seria constante y el campo diria siempre lo mismo.
        m = re.search(r"enum\s+EstadoCamara\s*\{([^}]*)\}", codigo[punta])
        vals = [v.strip() for v in m.group(1).split(",") if v.strip()] if m else []
        b.verificar(
            len(vals) == 4,
            "%s: EstadoCamara tiene los cuatro valores (%s) y su orden ES la gravedad"
            % (punta, ", ".join(vals)),
            "%s: EstadoCamara tiene %d valor(es): %s. Con menos de cuatro, el mayor-que "
            "de camara_estado() deja de poder elegir y el campo CAM: publica siempre lo "
            "mismo -que es el enum de un solo valor de 3.septies, aqui con cuatro nombres "
            "prometidos en botones.h-" % (punta, len(vals), vals or "(no se halla)"))

        # -- 4.bis LAS DOS ALARMAS SE CIERRAN. Una que no se cierra deja al operario sin
        #    saber si aquello se arreglo, y ademas taparia a la otra para siempre en el
        #    campo CAM:, que publica la peor de las dos.
        cierra_ciega = "camara_recuperada(i)" in cuerpos[(punta, "vigilante_flanco")]
        cierra_pegada = "camara_recuperada(i)" in cuerpos[(punta, "vigilante_nivel")]
        b.verificar(
            cierra_ciega and cierra_pegada,
            "%s: las dos alarmas se cierran por su prueba contraria -CIEGA con un flanco, "
            "PEGADA con el contacto abriendose- y las dos lo publican" % punta,
            "%s: CIEGA se cierra=%s, PEGADA se cierra=%s. Una alarma sin salida deja al "
            "operario sin saber si aquello se arreglo, y como el campo CAM: publica la "
            "PEOR de las dos camaras, una que se queda enganchada tapa a la otra hasta el "
            "siguiente corte de corriente" % (punta, cierra_ciega, cierra_pegada))

    # =============================================================================
    # 5. LA VENTANA DE "PRESENCIA VIGENTE" NO ES UN NUMERO NUEVO
    # =============================================================================
    #
    # A-7 esta abierta justo por esto: el "~1 s del rele" se lo invento este proyecto y
    # despues se cito a si mismo en NUEVE sitios. El vigilante no anade un decimo. Para
    # decidir si una deteccion "sigue en pie" en el instante en que la pluma baja reutiliza
    # el UNICO plazo que este firmware ya tiene para esa misma pregunta.
    for punta in PUNTAS:
        tick = cuerpos[(punta, "vigilante_tick")]
        b.verificar(
            "demanda_ventanaMs()" in tick
            and re.search(r"camUltimoFlanco\[i\]\)\s*<=\s*\d", tick) is None,
            "%s: la vigencia de una deteccion se pregunta a demanda_ventanaMs(), no a un "
            "numero escrito aqui: un solo plazo para las dos preguntas" % punta,
            "%s: vigilante_tick() decide la vigencia con un numero propio en vez de con "
            "demanda_ventanaMs(). Son dos definiciones de 'todavia en pie' que solo la "
            "disciplina mantiene iguales, y ese es el defecto que A-7 lleva abierto desde "
            "el 04/09 -un numero nuestro que luego nos citamos-" % punta)

        # Y ese getter devuelve LA constante, no una copia suya.
        dem = fw.codigo(punta, "src", "demanda.cpp")
        b.verificar(
            re.search(r"unsigned\s+long\s+demanda_ventanaMs\s*\(\s*\)\s*\{\s*"
                      r"return\s+SILENCIO_MS\s*;\s*\}", dem) is not None,
            "%s: demanda_ventanaMs() devuelve SILENCIO_MS y no una copia del numero"
            % punta,
            "%s: demanda_ventanaMs() no devuelve SILENCIO_MS. Un getter que devuelve una "
            "constante propia es la segunda copia que N-137 persigue: el dia que difieran "
            "gana la que NO lleva encima el comentario que la explica" % punta)

        # -- 5.bis N-26 APLICADO AL CONTADOR: sin la bandera de "ya hubo flanco", millis()
        #    casi en cero contra un camUltimoFlanco[] tambien en cero cae DENTRO de la
        #    ventana, y la primera bajada de pluma tras el arranque contaria un veto que
        #    nadie provoco. Es el mismo agujero que el ACEPTAR fantasma de N-26, y el
        #    contador de la fase 1 existe justo para que su numero se pueda creer.
        b.verificar(
            "camHuboFlanco[i]" in tick,
            "%s: el contador de vetos exige que HAYA HABIDO un flanco antes de creerse la "
            "ventana: el arranque no regala un veto" % punta,
            "%s: vigilante_tick() no consulta camHuboFlanco[]. Con millis() cerca de cero "
            "y camUltimoFlanco[] en cero la resta cae dentro de la ventana, asi que la "
            "primera bajada de pluma tras cada arranque contaria una presencia que no "
            "existio - y el numero que tiene que decidir la fase 2 empezaria mintiendo "
            "hacia arriba" % punta)

        # -- 5.ter EL CONTADOR SATURA. Un contador que vuelve a cero miente hacia abajo
        #    justo cuando lo que dice es "esto pasa mucho".
        b.verificar(
            re.search(r"if\s*\(\s*camVetos\s*<\s*65535\s*\)", tick) is not None,
            "%s: el contador de vetos satura en 65535 en vez de dar la vuelta" % punta,
            "%s: el contador de vetos no satura. Al desbordar volveria a cero, y el unico "
            "numero que la fase 5 tiene para decidir diria 'casi nunca pasa' justo despues "
            "de que haya pasado 65.536 veces" % punta)

    # =============================================================================
    # 6. LAS TRAMAS CABEN. ES N-108, REHECHO SOBRE LOS LITERALES DE ESTE VIGILANTE
    # =============================================================================
    #
    # N-108 costo un $ALARM que se truncaba POR LA HORA sin que nada lo dijera: una trama
    # cortada es una que el otro extremo descarta por checksum, o sea una alarma que
    # desaparece justo cuando hace falta. La cuenta se rehace aqui, y con el PEOR CASO POR
    # TIPO de cada sumando, no con el que suele salir.
    for punta in PUNTAS:
        bt = fw.codigo(punta, "src", "bluetooth.cpp")
        nombres = re.search(
            r"CAM_NOMBRE\[2\]\s*=\s*\{\s*\"([^\"]+)\"\s*,\s*\"([^\"]+)\"\s*\}",
            codigo[punta])
        alarmas = re.findall(
            r"camara_alarmar\(\s*i\s*,\s*\"([^\"]+)\"\s*,\s*\"([^\"]+)\"\s*\)",
            codigo[punta])
        accion = re.search(
            r"bluetooth_reportarAlarma\(\s*evento\s*,\s*causa\s*,\s*\"([^\"]+)\"\s*\)",
            cuerpos[(punta, "camara_alarmar")])
        if not (nombres and alarmas and accion):
            raise fw.Abortado(
                "no se pudieron leer del C++ los literales del vigilante del %s -nombres "
                "de camara, alarmas o la accion-. Sin ellos el presupuesto de bytes "
                "compararia nada contra nada y saldria verde" % punta)
        nombres = list(nombres.groups())

        # El emisor: plantilla, buffer del payload y buffer del tramo de enlace.
        emisor = _cuerpo(bt, "bluetooth_reportarAlarma")
        plantilla = re.search(r"snprintf\(payload,\s*sizeof\(payload\),\s*\"([^\"]+)\"",
                              emisor or "")
        cap_payload = re.search(r"char\s+payload\[(\d+)\]", emisor or "")
        cap_tramo = re.search(r"char\s+tramo\[(\d+)\]", emisor or "")
        if not (plantilla and cap_payload and cap_tramo):
            raise fw.Abortado(
                "no se pudo leer del C++ el emisor del $ALARM del %s (plantilla, payload "
                "o tramo). El presupuesto de N-108 no se puede rehacer sin los tres"
                % punta)

        fijo = len(plantilla.group(1)) - 2 * plantilla.group(1).count("%s")
        peor = (fijo
                + max(len(e) for e, _ in alarmas)                       # EVENTO
                + max(len(n) for n in nombres) + 1
                + max(len(m) for _, m in alarmas)                       # CAUSA
                + int(cap_tramo.group(1)) - 1                           # el tramo, a tope
                + len(accion.group(1))                                  # ACCION
                + LARGO_HORA)
        b.verificar(
            peor + 1 <= int(cap_payload.group(1)),
            "%s: el peor $ALARM del vigilante mide %d B y cabe en payload[%s] con su NUL"
            % (punta, peor, cap_payload.group(1)),
            "%s: el peor $ALARM del vigilante mide %d B y payload[] son %s. Se truncaria "
            "POR EL FINAL -o sea por la HORA-, el checksum se calcularia sobre lo que "
            "quedo y el otro extremo la descartaria: una alarma que desaparece justo "
            "cuando hace falta. Es N-108 otra vez; acorta EVENTO o CAUSA, no ensanches el "
            "buffer" % (punta, peor, cap_payload.group(1)))

        # -- 6.bis Y EL $EVENT, que es por donde sale el contador de la fase 1 --
        emisor_ev = _cuerpo(bt, "bluetooth_reportarEvento")
        plant_ev = re.search(r"snprintf\(payload,\s*sizeof\(payload\),\s*\"([^\"]+)\"",
                             emisor_ev or "")
        cap_ev = re.search(r"char\s+payload\[(\d+)\]", emisor_ev or "")
        origenes = re.findall(r"bluetooth_reportarEvento\(\s*\"([^\"]+)\"", codigo[punta])
        detalles = re.findall(
            r"snprintf\(\s*detalle\s*,\s*sizeof\(detalle\)\s*,\s*\"([^\"]+)\"",
            codigo[punta])
        if not (plant_ev and cap_ev and origenes and detalles):
            raise fw.Abortado(
                "no se pudo leer del C++ el emisor del $EVENT del %s o los literales que "
                "el vigilante le pasa. Sin ellos el presupuesto no mide nada" % punta)

        fijo_ev = len(plant_ev.group(1)) - 2 * plant_ev.group(1).count("%s")
        peor_ev = (fijo_ev
                   + max(len(o) for o in origenes)
                   + max(_peor(d, nombres) for d in detalles)
                   + LARGO_HORA)
        b.verificar(
            peor_ev + 1 <= int(cap_ev.group(1)),
            "%s: el peor $EVENT del vigilante mide %d B y cabe en payload[%s] con su NUL "
            "-el contador va con sus cinco cifras de saturacion-"
            % (punta, peor_ev, cap_ev.group(1)),
            "%s: el peor $EVENT del vigilante mide %d B y payload[] son %s. El que se "
            "trunca es justo el que lleva el numero de la fase 1, o sea el unico dato que "
            "esta fase produce" % (punta, peor_ev, cap_ev.group(1)))

        # -- 6.ter Y LOS BUFFERS LOCALES DEL VIGILANTE, que son los primeros en cortar --
        #
        # CADA FUNCION LLEVA SU PROPIO REPERTORIO DE SUSTITUCIONES, y no uno comun, porque
        # un repertorio comun MIENTE EN LAS DOS DIRECCIONES: aplicado a camara_alarmar()
        # -donde los dos %s son un nombre de camara y un motivo- se quedaria corto si solo
        # tuviera los nombres, y aplicado a camara_recuperada() -donde el unico %s es un
        # nombre- se pasaria de largo si tuviera los motivos, acusando al firmware de un
        # desbordamiento imposible. Es CLAUDE.md 4.quinquies: cuando un instrumento compara
        # contra un borde, hay que escribir CUAL es ese borde y por que es el correcto.
        pools = {
            "camara_alarmar": nombres + [m for _, m in alarmas],
            "camara_recuperada": nombres,
            "vigilante_tick": nombres,
        }
        for fn, pool in sorted(pools.items()):
            cuerpo = cuerpos[(punta, fn)]
            for var, cap in re.findall(r"char\s+(\w+)\[(\d+)\]", cuerpo):
                fmts = re.findall(
                    r"snprintf\(\s*%s\s*,\s*sizeof\(%s\)\s*,\s*\"([^\"]+)\"" % (var, var),
                    cuerpo)
                if not fmts:
                    continue
                largo = max(_peor(f, pool) for f in fmts)
                b.verificar(
                    largo + 1 <= int(cap),
                    "%s: %s() compone %d B como mucho en %s[%s]: cabe con su NUL"
                    % (punta, fn, largo, var, cap),
                    "%s: %s() puede componer %d B en %s[%s]. snprintf trunca en silencio, "
                    "asi que la alarma saldria al aire nombrando media camara o media "
                    "causa - bien formada y falsa" % (punta, fn, largo, var, cap))

    # =============================================================================
    # 7. EL VIGILANTE ES EL MISMO EN LAS DOS PUNTAS (N-97, aplicado al codigo nuevo)
    # =============================================================================
    #
    # Las dos camaras llevan LA MISMA CONFIGURACION (D-13) y el mismo cableado en J16. Dos
    # vigilantes parecidos que solo la disciplina mantiene iguales son exactamente lo que
    # costo N-97 -y antes el `amarillo = false` de mas del Esclavo en SFTY-2-.
    for fn in VIGILANTE:
        m = re.sub(r"\s+", " ", cuerpos[("Maestro", fn)]).strip()
        e = re.sub(r"\s+", " ", cuerpos[("Esclavo", fn)]).strip()
        b.verificar(
            m == e,
            "%s() es identica en las dos puntas (%d caracteres de codigo sin comentarios)"
            % (fn, len(m)),
            "EL VIGILANTE DIVERGE en %s(). Las dos camaras son el mismo modelo con la "
            "misma configuracion: dos criterios distintos hacen que el mismo fallo se "
            "vea en un poste y no en el otro.\n        Maestro: %s\n        Esclavo: %s"
            % (fn, m[:170], e[:170]))

    # =============================================================================
    # 8. EL GETTER DEL CAMPO CAM: ESTA LISTO Y TODAVIA NO CONECTADO - Y ESO SE MIDE
    # =============================================================================
    #
    # Es el MOTIVO de su excepcion en costura_10_funciones_muertas, y CLAUDE.md 3.bis exige
    # que un motivo se MIDA y no se redacte: "una lista de excepciones con motivos sin
    # verificar es una lista de defectos con permiso". El motivo escrito alli es que el
    # campo CAM: no cabe hoy en el $STATUS -162 B por tipo contra un techo de 155, la
    # cuenta esta en Maestro/src/bluetooth.cpp-, asi que lo comprobable es esto: que el
    # getter exista declarado en las dos puntas y que el $STATUS siga sin el campo.
    #
    # SI ESTA COMPROBACION TE HA TRAIDO AQUI EN ROJO, lo mas probable es que acabes de
    # conectar CAM:. Entonces toca, EN EL MISMO COMMIT: quitar esta comprobacion, sacar
    # camara_estado del CONOCIDAS de costura_10 -que tambien va a fallar, y por lo mismo- y
    # rehacer el peor caso del $STATUS.
    for punta in PUNTAS:
        b.verificar(
            re.search(r"const\s+char\s*\*\s*camara_estado\s*\(\s*\)\s*;", cabecera[punta])
            is not None,
            "%s: camara_estado() esta declarada y lista para el campo CAM:" % punta,
            "%s: camara_estado() no se declara en botones.h. El campo CAM: de D-13 se "
            "queda sin fuente, y sin el 'no llega bit' y 'no hay nadie' vuelven a ser "
            "indistinguibles" % punta)

    en_status = [p for p in PUNTAS
                 if re.search(r"\$STATUS[^\"]*CAM:", fw.codigo(p, "src", "bluetooth.cpp"))]
    b.verificar(
        not en_status,
        "el campo CAM: todavia no esta en la plantilla del $STATUS de ninguna punta: es el "
        "motivo MEDIDO de la excepcion de camara_estado en costura_10, no una frase",
        "el $STATUS de %s ya publica CAM:. Entonces camara_estado() tiene llamador y su "
        "excepcion en costura_10 caduco: hay que retirarla en el mismo commit, y rehacer "
        "el peor caso del $STATUS -que ya no cabia por tipo antes de este campo-"
        % ", ".join(en_status))

    # =============================================================================
    # 9. CONTROLES NEGATIVOS
    # =============================================================================
    b.control_negativo(
        re.search(r"camSinFlancoMs\[i\]\s*=\s*0\s*;",
                  "static void vigilante_flanco(int i, unsigned long ahora) { "
                  "camUltimoFlanco[i] = ahora; camAltoDesde[i] = ahora; }") is None,
        "el detector ve un vigilante_flanco() al que le falta el reinicio del cronometro "
        "de silencio, que es el que convierte a una camara sana en una acusada")

    # El de la puerta de la pluma se ejerce con la MISMA extraccion que la comprobacion
    # 2.3, sobre un tick al que se le ha sacado la acumulacion fuera del if. Si se probara
    # con un `in` a secas sobre el texto entero, el control negativo pasaria sin haber
    # ejercido el delimitador de bloques, que es justo la pieza que puede quedarse ciega.
    _tick_malo = ("static void vigilante_tick(unsigned long ahora) { "
                  "const bool arriba = semaforo_plumaArriba(); "
                  "if (arriba) { nada(); } camSinFlancoMs[i] += dt; }")
    _m_malo = re.search(r"if\s*\(\s*arriba\s*\)\s*\{", _tick_malo)
    _tr_malo = _bloque(_tick_malo, _tick_malo.index("{", _m_malo.end() - 1))
    b.control_negativo(
        "camSinFlancoMs[i] +=" not in _tick_malo[_tr_malo[0]:_tr_malo[1]],
        "y el de la puerta de la pluma distingue una acumulacion que se salio del "
        "'if (arriba)' y pasa a contar tiempo de reloj")

    b.control_negativo(
        set(re.findall(r'return\s+"([^"]*)"\s*;',
                       'const char* camara_estado(){ return "OK"; }')) != set(VALORES_CAM),
        "el censo del campo CAM: NO acepta un getter que solo sabe decir OK")

    b.control_negativo(
        [n for n in ORDENAN
         if re.search(r"\b%s\b" % n,
                      "static void vigilante_tick(){ coordinador_pedirCambio(); }")]
        == ["coordinador_pedirCambio"],
        "el censo de la barrera ve un vigilante que ordena en vez de contar")

    b.control_negativo(
        len([v.strip() for v in
             re.search(r"enum\s+EstadoCamara\s*\{([^}]*)\}",
                       "enum EstadoCamara { CAM_OK };").group(1).split(",")
             if v.strip()]) == 1,
        "el lector del enum ve uno que se quedo con un solo valor, que es el campo CAM: "
        "publicando siempre lo mismo")

    b.control_negativo(
        re.search(r"if\s*\(\s*camVetos\s*<\s*65535\s*\)",
                  "if (camVetos < 65535) { camVetos++; }") is not None
        and re.search(r"if\s*\(\s*camVetos\s*<\s*65535\s*\)", "camVetos++;") is None,
        "el detector de saturacion NO da por saturado un contador que solo incrementa")

    # El presupuesto de bytes tiene que saber decir que NO cabe: se le pasa una plantilla
    # con un buffer imposible y se exige que la cuenta lo vea.
    _fmt = "$ALARM,NODE:MAESTRO,EVENTO:%s,CAUSA:%s,%s,ACCION:%s,HORA:%s"
    b.control_negativo(
        (len(_fmt) - 2 * _fmt.count("%s") + 13 + 19 + 43 + 7 + LARGO_HORA) + 1 > 100,
        "la cuenta del presupuesto ve que el peor caso NO cabe cuando el payload es de "
        "100 B, que es el tamano con el que N-108 truncaba la alarma por la hora")
