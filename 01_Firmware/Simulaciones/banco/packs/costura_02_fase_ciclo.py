# ===== banco/packs/costura_02_fase_ciclo.py =====
#
# FASE DEL CICLO DEGRADADO - QUE LAS DOS PUNTAS CALCULEN LO MISMO
#
# Barrido de las 86.400 posiciones del dia comparando lo que calcula cada punta.
# Es la unica barrera contra el verde simultaneo: si las dos se creen con derecho
# a verde en el mismo segundo, el cruce queda abierto por los dos lados.

from banco.modelos.costura import *          # noqa: F401,F403

# EJERCE SFTY-21: que las dos puntas calculen la MISMA fase del ciclo degradado.

NOMBRE = "costura_02_fase_ciclo"
DESCRIPCION = "la fase del ciclo: que las dos puntas calculen lo mismo"


def correr(b, fw):
    # Bloque traido LITERAL, solo reindentado.
    verificar = b.verificar
    reportar = b.reportar

    def hallazgo(reproducido, titulo, detalle, consecuencia):
        """El hallazgo de costura lleva CUATRO argumentos y SI cuenta como
        comprobacion: aqui la comprobacion ES reproducir el desajuste, asi que si el
        modelo no lo reprodujera seria el modelo el que esta mal. En el validador del
        Esclavo la misma palabra significa otra cosa y NO cuenta -alli acompana a una
        propiedad() que ya cuenta por su cuenta-. Dos cosas distintas con el mismo
        nombre: por eso 37/41 y 30/31 no se podian sumar."""
        b.hallazgo(reproducido, titulo, [detalle, f"EN LA CALLE: {consecuencia}"])

    b.titulo("FASE DEL CICLO DEGRADADO - QUE LAS DOS PUNTAS CALCULEN LO MISMO")


    FD_VERDE_MAESTRO, FD_DESPEJE_A, FD_VERDE_ESCLAVO, FD_DESPEJE_B = 0, 1, 2, 3


    def fase(seg_dia, verde, despeje):
        """Espejo EXACTO de ciclo_degradado_fase() de ciclo_degradado.h.

        Se copia la estructura linea por linea, incluida la guarda de medianoche en
        los dos sentidos. Mas abajo se comprueba contra el C++ que el espejo sigue
        correspondiendose, para que este espejo no envejezca en silencio.
        """
        if verde == 0 or despeje == 0:
            return FD_DESPEJE_A
        ciclo = 2 * (verde + despeje)
        if seg_dia < despeje:
            return FD_DESPEJE_B
        if SEGUNDOS_DEL_DIA - seg_dia <= despeje:
            return FD_DESPEJE_B
        pos = seg_dia % ciclo
        if pos < verde:
            return FD_VERDE_MAESTRO
        if pos < verde + despeje:
            return FD_DESPEJE_A
        if pos < 2 * verde + despeje:
            return FD_VERDE_ESCLAVO
        return FD_DESPEJE_B


    # --- 2a. El espejo no puede envejecer sin que nadie lo note ----------------
    cuerpo_fase = re.search(r"ciclo_degradado_fase\(uint32_t segDia.*?\n\}", T_M_CICLO_H, re.S)
    huellas_c = [
        r"if\s*\(verdeSeg\s*==\s*0\s*\|\|\s*despejeSeg\s*==\s*0\)\s*return\s+FD_DESPEJE_A",
        r"ciclo\s*=\s*2UL\s*\*\s*\(\(uint32_t\)verdeSeg\s*\+\s*\(uint32_t\)despejeSeg\)",
        r"if\s*\(segDia\s*<\s*despejeSeg\)\s*return\s+FD_DESPEJE_B",
        r"if\s*\(SEGUNDOS_DEL_DIA\s*-\s*segDia\s*<=\s*despejeSeg\)\s*return\s+FD_DESPEJE_B",
        r"pos\s*=\s*segDia\s*%\s*ciclo",
        r"if\s*\(pos\s*<\s*\(uint32_t\)verdeSeg\)\s*return\s+FD_VERDE_MAESTRO",
        r"if\s*\(pos\s*<\s*\(uint32_t\)verdeSeg\s*\+\s*despejeSeg\)\s*return\s+FD_DESPEJE_A",
        r"if\s*\(pos\s*<\s*2UL\s*\*\s*verdeSeg\s*\+\s*despejeSeg\)\s*return\s+FD_VERDE_ESCLAVO",
    ]
    espejo_ok = cuerpo_fase is not None and all(re.search(p, cuerpo_fase.group(0)) for p in huellas_c)
    verificar(espejo_ok,
              "el espejo Python de ciclo_degradado_fase() sigue correspondiendose linea a "
              "linea con el C++: si alguien toca el calculo, el espejo deja de casar y esto FALLA",
              "el espejo Python ya NO corresponde al C++: todo lo que sigue estaria validando "
              "un calculo que el firmware no ejecuta")

    # --- 2b. El Maestro ENVIA lo que USA --------------------------------------
    # El fallo n.2 del proyecto fue no decir quien amplia el despeje. La defensa que
    # se eligio es que el valor enviado y el usado sean EL MISMO SIMBOLO, sin
    # variable intermedia donde alguien pueda volver a multiplicar por dos. Eso se
    # comprueba estructuralmente, en el fuente, no de palabra.
    m_publica = re.search(r"coordinador_enviarConfigCiclo\(([^;]*?)\);", T_M_DEG_C)
    m_usa = re.search(r"ciclo_degradado_fase\(reloj_segundosDelDia\(\),\s*([^;]*?)\);", T_M_DEG_C)
    m_guarda = re.search(r"respaldo_guardarCiclo\(([^;]*?)\);", T_M_DEG_C)


    def simbolos(m):
        """Quita los casts y se queda con los identificadores que se pasan."""
        if not m:
            return None
        crudo = re.sub(r"\(\s*uint\d+_t\s*\)", "", m.group(1))
        return [p.strip() for p in crudo.split(",")]


    s_envia, s_usa, s_guarda = simbolos(m_publica), simbolos(m_usa), simbolos(m_guarda)
    verificar(s_envia is not None and s_envia == s_usa == s_guarda
              and s_envia == ["DEG_VERDE_SEG", "DEG_DESPEJE_SEG"],
              f"el Maestro ENVIA, GUARDA y USA los mismos dos simbolos ({s_envia}): no existe "
              "variable intermedia donde reaplicar la ampliacion del despeje",
              f"el Maestro envia {s_envia}, guarda {s_guarda} y usa {s_usa} para su propia fase: "
              "tres sitios donde el valor puede divergir")

    # Y el Esclavo lo usa TAL CUAL: el byte se guarda sin una sola operacion
    # aritmetica. Si apareciera un *2 aqui, seria el fallo n.2 renacido.
    # FASE 2 (03/08/2026): esto vivia en Esclavo/src/main.cpp y ahora esta en
    # src/config_ciclo.cpp. El parametro se llama `segundos`, no `pkt.param`, porque el
    # modulo ya no conoce el transporte. La propiedad comprobada es la misma: el byte
    # se guarda TAL CUAL, sin una sola operacion aritmetica.
    rama_verde = re.search(r"void\s+config_rxVerde\([^)]*\)\s*\{(.*?)\n\}", T_E_CONFIG_C, re.S)
    rama_despeje = re.search(r"bool\s+config_rxDespeje\([^)]*\)\s*\{(.*?)\n\}", T_E_CONFIG_C, re.S)
    asigna_verde = rama_verde and re.search(r"cfgVerdeSeg\s*=\s*segundos\s*;", rama_verde.group(1))
    asigna_despeje = rama_despeje and re.search(r"cfgDespejeSeg\s*=\s*segundos\s*;", rama_despeje.group(1))
    sin_aritmetica = True
    for r_ in (rama_verde, rama_despeje):
        if r_ and re.search(r"cfg(Verde|Despeje)Seg\s*=\s*[^;]*[\*/+<>][^;]*;", r_.group(1)):
            sin_aritmetica = False
    verificar(bool(asigna_verde) and bool(asigna_despeje) and sin_aritmetica,
              "el Esclavo asigna el byte recibido TAL CUAL (cfgXSeg = pkt.param), sin escalarlo "
              "ni duplicarlo: la ampliacion del despeje se decide en un solo sitio",
              "el Esclavo transforma el valor recibido: dos puntas calcularian ciclos de "
              "DISTINTA DURACION sobre la misma hora y los verdes se solaparian durante minutos")

    # --- 2c. Barrido de las 86.400 posiciones del dia, las dos puntas a la vez -
    # Se modela la LUZ REAL de cada punta, no la fase: el Esclavo mete 4 s de
    # amarillo antes de su verde y el Maestro no, y esa diferencia solo se ve
    # modelando las dos.
    ROJO, AMBAR_FIJO, VERDE, AMBAR_INTERMITENTE = 0, 1, 2, 3


    def luz_maestro(seg_dia, verde, despeje):
        """modo_degradado.cpp (Maestro), DEG_ACTIVO: verde SOLO en FD_VERDE_MAESTRO.
        Usa semaforo_forzarVerde(), que es un salto DIRECTO a verde."""
        return VERDE if fase(seg_dia, verde, despeje) == FD_VERDE_MAESTRO else ROJO


    def luz_esclavo(seg_dia, verde, despeje, amarillo_s):
        """modo_degradado.cpp (Esclavo), DEG_ACTIVO: aplicarLuz(fase == FD_VERDE_ESCLAVO).
        Pero pasa por semaforo_iniciarTransicionAVerde(): amarillo fijo y despues verde.
        Los segundos de amarillo se comen SU verde, nunca el todo-rojo."""
        if fase(seg_dia, verde, despeje) != FD_VERDE_ESCLAVO:
            return ROJO
        ciclo = 2 * (verde + despeje)
        pos = seg_dia % ciclo
        inicio_verde_esclavo = verde + despeje
        return AMBAR_FIJO if (pos - inicio_verde_esclavo) < amarillo_s else VERDE


    # Configuraciones a barrer. Se incluye la REAL del firmware y varias que NO
    # dividen a 86.400, que es donde muerde el salto de medianoche: 86400 % 120 = 0
    # no prueba nada sobre un ciclo de 134 s.
    CONFIGS = [
        (DEG_VERDE_SEG, DEG_DESPEJE_SEG),   # la de verdad, leida del C++
        (30, 37),                            # ciclo 134 s: 86400 % 134 = 44
        (45, 20),                            # ciclo 130 s: 86400 % 130 = 20
        (17, 11),                            # ciclo  56 s: 86400 % 56  = 32
        (120, 30),                           # ciclo 300 s: 86400 % 300 = 0 (divide)
        (7, 3),                              # ciclo  20 s, muy corto
        (255, 255),                          # el tope del byte
    ]
    assert any(86400 % (2 * (v + d)) for v, d in CONFIGS), "hace falta un ciclo no divisor"

    verdes_simultaneos = []
    verde_a_verde = []
    medianoche_con_verde = []
    for verde, despeje in CONFIGS:
        ant_m = ant_e = None
        for s in range(SEGUNDOS_DEL_DIA):
            lm = luz_maestro(s, verde, despeje)
            le = luz_esclavo(s, verde, despeje, M_AMARILLO_MS // 1000)
            if lm == VERDE and le == VERDE:
                verdes_simultaneos.append((verde, despeje, s))
                break
            # Verde de una punta seguido de verde de la otra sin todo-rojo en medio.
            if ant_m == VERDE and le == VERDE:
                verde_a_verde.append((verde, despeje, s))
                break
            if ant_e in (VERDE, AMBAR_FIJO) and lm == VERDE:
                verde_a_verde.append((verde, despeje, s))
                break
            ant_m, ant_e = lm, le
        for s in list(range(SEGUNDOS_DEL_DIA - despeje, SEGUNDOS_DEL_DIA)) + list(range(0, despeje)):
            if luz_maestro(s, verde, despeje) != ROJO or luz_esclavo(s, verde, despeje, M_AMARILLO_MS // 1000) != ROJO:
                medianoche_con_verde.append((verde, despeje, s))
                break

    verificar(not verdes_simultaneos,
              f"barrido de las {SEGUNDOS_DEL_DIA} posiciones del dia en {len(CONFIGS)} configuraciones "
              "(incluidas duraciones que NO dividen a 86.400): NUNCA hay verde en las dos puntas",
              f"PELIGRO: verde simultaneo en {verdes_simultaneos[:3]}")

    verificar(not verde_a_verde,
              "en el mismo barrido, NUNCA se pasa del verde de una punta al de la otra sin "
              "todo-rojo de por medio",
              f"PELIGRO: paso verde->verde sin despeje en {verde_a_verde[:3]}")

    verificar(not medianoche_con_verde,
              "la frontera de medianoche se cruza SIEMPRE con las dos puntas en rojo, tambien "
              "con ciclos que no dividen a 86.400 (es donde el salto de posicion podria "
              "saltarse el despeje)",
              f"PELIGRO: hay luz distinta de rojo al cruzar medianoche en {medianoche_con_verde[:3]}")

    # --- 2d. La prueba tiene que saber reconocer el fallo ---------------------
    # Se modelan varias versiones DEFECTUOSAS -todas variantes del fallo n.2: una
    # punta transforma el valor y la otra no- y se exige que el barrido cace TODAS.
    # Si no fuera capaz, el PASS de arriba no significaria nada.
    #
    # OJO CON EL CRITERIO: no basta buscar verde simultaneo. Con verde=30 y el
    # Esclavo duplicando el despeje, los ciclos quedan en 120 s y 180 s y los verdes
    # NO llegan a solaparse nunca... pero el verde del Esclavo termina en el mismo
    # segundo en que empieza el del Maestro, SIN UN SOLO SEGUNDO DE TODO-ROJO entre
    # medias. Eso es igual de mortal y solo se ve mirando la transicion, no el
    # instante. Por eso el detector mira las dos cosas.
    def analizar(vm, dm, ve, de_):
        """Devuelve (primer verde simultaneo, primer paso verde->verde sin todo-rojo)."""
        amarillo = E_AMARILLO_MS // 1000
        solape = None
        pegado = None
        ant_m = ant_e = None
        for s in range(SEGUNDOS_DEL_DIA):
            lm = luz_maestro(s, vm, dm)
            le = luz_esclavo(s, ve, de_, amarillo)
            if solape is None and lm == VERDE and le == VERDE:
                solape = s
            if pegado is None:
                if ant_m == VERDE and le in (VERDE, AMBAR_FIJO):
                    pegado = s
                elif ant_e in (VERDE, AMBAR_FIJO) and lm == VERDE:
                    pegado = s
            if solape is not None and pegado is not None:
                break
            ant_m, ant_e = lm, le
        return solape, pegado


    DEFECTUOSAS = [
        ("el Esclavo duplica el despeje por su cuenta",
         DEG_VERDE_SEG, DEG_DESPEJE_SEG, DEG_VERDE_SEG, DEG_DESPEJE_SEG * 2),
        ("el Maestro amplia y el Esclavo recibe el valor SIN ampliar",
         DEG_VERDE_SEG, DEG_DESPEJE_SEG, DEG_VERDE_SEG, DEG_DESPEJE_SEG // 2),
        ("al Esclavo le llega un verde distinto (trama vieja)",
         DEG_VERDE_SEG, DEG_DESPEJE_SEG, DEG_VERDE_SEG + 5, DEG_DESPEJE_SEG),
    ]
    no_cazadas = []
    detalle_cazadas = []
    for titulo, vm, dm, ve, de_ in DEFECTUOSAS:
        solape, pegado = analizar(vm, dm, ve, de_)
        if solape is None and pegado is None:
            no_cazadas.append(titulo)
        else:
            detalle_cazadas.append(f"{titulo}: "
                                   + (f"verde simultaneo a los {solape} s" if solape is not None
                                      else f"verde->verde sin todo-rojo a los {pegado} s"))
    verificar(not no_cazadas,
              f"las {len(DEFECTUOSAS)} versiones DEFECTUOSAS modeladas se detectan todas -> "
              + " | ".join(detalle_cazadas),
              f"el barrido NO caza estas variantes del fallo n.2: {no_cazadas}. La prueba no "
              "distingue el caso bueno del malo y su PASS no vale nada")

    # Y la deriva de reloj que el despeje absorbe. Se desplaza el reloj del Esclavo
    # segundo a segundo y se busca el desfase al que aparece el primer solape.
    #
    # CORREGIDO EL 01/09: ESTO BARRIA UN SOLO SENTIDO Y PUBLICABA EL FAVORABLE.
    #
    # El bucle iba de 0 hacia arriba, o sea SOLO con el Esclavo adelantado, y anunciaba
    # ese resultado como "el margen real contra la deriva entre relojes". Pero la deriva
    # de dos cristales no elige sentido: el Esclavo puede atrasarse igual de bien, y ESE
    # es el sentido malo. Los 4 s de ambar con que el Esclavo abre su verde protegen
    # SOLO en un sentido, asi que los dos numeros no son iguales:
    #
    #   Esclavo adelantado  ->  rompe a 35 s   (el que se publicaba)
    #   Esclavo atrasado    ->  rompe a 30 s   <- LA FRONTERA REAL
    #
    # Medido ademas sobre el C++ REAL de las dos puntas -Validacion_Automatico/
    # compilar_degradado.ps1, 01/09-, que da 29 s: el segundo entero con que viaja la
    # hora se come el que falta. Publicar 35 cuando el equipo rompe a 29 es un colchon
    # inflado un 20 % en el modo donde un solape es un choque frontal.
    #
    # Ahora se barren LOS DOS SENTIDOS y manda el peor.
    def _primer_solape(signo):
        for skew in range(0, 2 * (DEG_VERDE_SEG + DEG_DESPEJE_SEG)):
            for s in range(0, SEGUNDOS_DEL_DIA, 1):
                if (luz_maestro(s, DEG_VERDE_SEG, DEG_DESPEJE_SEG) == VERDE and
                        luz_esclavo((s + signo * skew) % SEGUNDOS_DEL_DIA,
                                    DEG_VERDE_SEG, DEG_DESPEJE_SEG,
                                    M_AMARILLO_MS // 1000) == VERDE):
                    return skew
        return None

    adelantado = _primer_solape(+1)
    atrasado = _primer_solape(-1)
    medidos = [x for x in (adelantado, atrasado) if x is not None]
    deriva_critica = min(medidos) if medidos else None

    verificar(deriva_critica is not None and deriva_critica >= DEG_DESPEJE_SEG,
              f"el margen contra la deriva entre relojes es de {deriva_critica} s, medido en "
              f"LOS DOS SENTIDOS -atrasado {atrasado} s, adelantado {adelantado} s-: manda el "
              f"peor, y el despeje configurado son {DEG_DESPEJE_SEG} s",
              f"el margen del peor sentido ({deriva_critica} s) no llega al despeje configurado "
              f"({DEG_DESPEJE_SEG} s): el colchon no es el que dice el diseno")

    reportar("el colchon NO es simetrico, y el limite de 48 h se justifica con el peor",
             [f"Esclavo atrasado rompe a {atrasado} s; adelantado, a {adelantado} s. "
              f"La diferencia son los {E_AMARILLO_MS//1000} s de ambar con que el Esclavo "
              "abre su verde, que protegen solo en un sentido",
              "sobre el C++ real de las dos puntas la frontera baja a 29 s: la hora viaja "
              "en segundos enteros y ese segundo se descuenta",
              "contra 20,2 s que el equipo puede acumular en 48 h -17,2 de deriva mas 3 de "
              "TOLERANCIA_DESFASE_S- el factor es 1,44, NO el 2 que afirmaban los "
              "comentarios de las dos puntas hasta el 01/09"])

    # --- 2e. El tope del byte: el contrato dice SATURAR, el codigo TRUNCA ------
    # protocolo.h: "param = SEGUNDOS, saturado a 255" y ademas afirma que por encima
    # del tope "ambas puntas usan el mismo valor saturado y siguen en fase".
    # El codigo del Maestro hace un cast (uint8_t), que NO satura: da la vuelta. Y
    # para SU PROPIA fase usa el uint16_t entero, sin truncar.
    dice_saturado = bool(re.search(r"saturado a 255", T_M_PROTO_H))
    promete_misma_fase = bool(re.search(r"ambas puntas usan el mismo valor saturado y siguen en\s*//?\s*fase",
                                        T_M_PROTO_H.replace("\n", " ")))
    tipo_constantes = re.search(r"static const (\w+) DEG_VERDE_SEG", T_M_DEG_C)
    usa_cast_sin_saturar = bool(re.search(r"coordinador_enviarConfigCiclo\(\(uint8_t\)DEG_VERDE_SEG", T_M_DEG_C))
    usa_ancho_completo = bool(re.search(r"ciclo_degradado_fase\(reloj_segundosDelDia\(\),\s*DEG_VERDE_SEG",
                                        T_M_DEG_C))

    # Se demuestra numericamente lo que pasaria: un verde de 300 s viaja como 44.
    verde_grande = 300
    enviado = verde_grande & 0xFF
    choque = None
    for s in range(SEGUNDOS_DEL_DIA):
        if (luz_maestro(s, verde_grande, DEG_DESPEJE_SEG) == VERDE and
                luz_esclavo(s, enviado, DEG_DESPEJE_SEG, M_AMARILLO_MS // 1000) == VERDE):
            choque = s
            break

    hallazgo(dice_saturado and usa_cast_sin_saturar and usa_ancho_completo
             and tipo_constantes is not None and tipo_constantes.group(1) == "uint16_t"
             and choque is not None,
             "el contrato promete SATURAR a 255; el codigo TRUNCA con un cast",
             f"protocolo.h dice 'param = SEGUNDOS, saturado a 255' y que por encima del tope "
             f"'ambas puntas usan el mismo valor saturado y siguen en fase'. Pero DEG_VERDE_SEG es "
             f"{tipo_constantes.group(1) if tipo_constantes else '?'} y viaja como (uint8_t): un verde "
             f"de {verde_grande} s saldria como {enviado} s, mientras el Maestro usaria los "
             f"{verde_grande} s enteros para su propia fase. No es saturacion, es dar la vuelta. "
             f"Hoy no muerde porque las constantes valen {DEG_VERDE_SEG}/{DEG_DESPEJE_SEG}, "
             f"y no hay ninguna guarda que impida subirlas.",
             f"si alguien sube el ciclo degradado por encima de 255 s, las dos puntas calcularian "
             f"ciclos de distinta duracion y habria verde simultaneo a los {choque} s del dia. "
             "Es el fallo n.2 del proyecto por otra puerta")

    # --- 2f. El mismo modo, dos secuencias de luz distintas -------------------
    # El Maestro salta a verde directo; el Esclavo pasa por 4 s de amarillo. En modo
    # normal las DOS puntas usan la transicion con amarillo.
    m_directo = bool(re.search(r"if\s*\(fase\s*==\s*FD_VERDE_MAESTRO\)\s*\{\s*semaforo_forzarVerde\(\);", T_M_DEG_C))
    e_transicion = bool(re.search(r"semaforo_iniciarTransicionAVerde\(\);", T_E_DEG_C))
    verde_util_maestro = DEG_VERDE_SEG
    verde_util_esclavo = DEG_VERDE_SEG - E_AMARILLO_MS // 1000

    hallazgo(m_directo and e_transicion and M_AMARILLO_MS == E_AMARILLO_MS,
             "en el MISMO modo, cada punta hace una secuencia de luz distinta",
             f"en Degradado el Maestro usa semaforo_forzarVerde() -salto directo rojo->verde- y el "
             f"Esclavo usa semaforo_iniciarTransicionAVerde() -rojo->amarillo {E_AMARILLO_MS//1000}s->"
             f"verde-. En modo normal, en cambio, las dos puntas pasan por el amarillo. El verde util "
             f"queda en {verde_util_maestro} s en el Maestro y {verde_util_esclavo} s en el Esclavo "
             f"({100*(verde_util_maestro-verde_util_esclavo)//verde_util_maestro}% menos).",
             "no es un riesgo de colision -el amarillo se come el verde del Esclavo, nunca el "
             "todo-rojo-, pero el mismo cruce muestra dos secuencias distintas segun el poste que "
             "se mire, y el manual de campo no puede describir 'la' secuencia del Degradado")

    # El suelo de rojo de entrada tampoco es el mismo en las dos puntas.
    m_rojo_transicion = bool(re.search(r"ROJO_TRANSICION_MS\s*=\s*\(unsigned long\)DEG_DESPEJE_SEG\s*\*\s*1000UL",
                                       T_M_DEG_C))
    verificar(m_rojo_transicion and E_ROJO_MINIMO_MS <= DEG_DESPEJE_SEG * 1000,
              f"el todo-rojo de entrada y salida vale un despeje completo en las dos puntas "
              f"({DEG_DESPEJE_SEG} s); el suelo de {E_ROJO_MINIMO_MS} ms del Esclavo no llega a "
              "actuar con la configuracion real",
              f"el suelo del Esclavo ({E_ROJO_MINIMO_MS} ms) recorta el todo-rojo por debajo del "
              f"despeje del Maestro ({DEG_DESPEJE_SEG*1000} ms): entradas asimetricas")

    # ===========================================================================
    # 3. LOS COMANDOS: lo que una envia y la otra espera
