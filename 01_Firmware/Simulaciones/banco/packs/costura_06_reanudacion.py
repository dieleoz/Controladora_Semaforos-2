# ===== banco/packs/costura_06_reanudacion.py =====
#
# REANUDACION TRAS UN CORTE: LA PUNTA QUE SE REINICIA VUELVE EN FASE
#
# Tras un corte, la punta que arranca reconstruye su fase desde la pila. Si volviera
# desfasada respecto a la que nunca se apago, las dos estarian en el mismo ciclo
# creyendo cosas distintas.

from banco.modelos.costura import *          # noqa: F401,F403

# EJERCE SFTY-21: que la punta que se reinicia vuelva EN FASE con la otra.

NOMBRE = "costura_06_reanudacion"
DESCRIPCION = "la punta que se reinicia, ¿vuelve en fase?"


# =================================================================================
# LAS CONDICIONES DE REANUDACION SE MIDEN POR PROPIEDAD, NO POR FORMA
#
# POR QUE ESTE LECTOR NO VOLVERA A ACUSAR AL FIRMWARE CUANDO ALGUIEN MUEVA LA
# EXPRESION. Hasta el 12/09 esta seccion leia las dos puntas con dos expresiones
# LITERALES -"reloj_enHora() && respaldo_hayCiclo() && horas != ... && horas < ..."
# en el Maestro, "sigueVigente = (horas != ...) && (horas < ...)" en el Esclavo-. Eso
# no media la propiedad: media el ORDEN de los &&, los parentesis y el nombre de la
# variable intermedia. D-29 reestructuro las dos funciones -saco la vigencia de la
# sync a su propio bool y giro la puerta del Esclavo a la forma negada- SIN QUITAR
# NINGUNA CONDICION, y el pack acuso al firmware de un defecto que no tenia: es
# CLAUDE.md 5 en su forma pura, contenido que se MUDA dentro del mismo fichero.
#
# Lo que se mide ahora es lo que la comprobacion siempre dijo medir: que las cuatro
# condiciones ESTAN, en las dos puntas, dentro del cuerpo de la funcion que reanuda, y
# que el centinela se compara APARTE del numero. Nada de eso depende de como esten
# encadenados los &&, asi que la proxima reestructuracion no lo rompe. Lo que SI lo
# rompe -y debe romperlo- es que una condicion desaparezca.
#
# LAS TRES COSAS QUE LE DAN DIENTES, porque sin ellas seria un grep con adornos:
#
#  1. SE ACOTA EL CUERPO DE LA FUNCION. Es lo unico que hace que el control negativo
#     muerda: RESPALDO_SYNC_CADUCADA y LIMITE_*_H tambien viven en
#     msDesdeSyncEfectivo(), en el mismo .cpp. Buscandolos en el fichero entero, quitar
#     el centinela de la reanudacion seguiria dando verde.
#  2. SOLO SE MIRAN LAS DECISIONES DE PRIMER NIVEL del cuerpo. Ese es EL BORDE, y se
#     escribe aqui cual es y por que es el correcto (CLAUDE.md 7): D-29 metio DENTRO
#     de la rama de rechazo un segundo 'if' -el diferimiento, con las mismas cuatro
#     condiciones repetidas- y una condicion borrada de la puerta seguiria apareciendo
#     alli. Una decision SUBORDINADA no puede contestar por la puerta. Medido: con la
#     puerta del Esclavo sin respaldo_hayCiclo(), mirando todo el cuerpo el pack daba
#     verde; mirando solo el primer nivel, cae.
#  3. LAS BANDERAS LOCALES SE EXPANDEN. 'ok', 'syncVigente', 'sigueVigente' son nombres
#     de paso; se sustituyen por TODO lo que se les asigna en el cuerpo, a cualquier
#     profundidad. Asi la misma lectura vale para la forma encadenada de ayer, para la
#     de dos banderas de hoy y para la secuencial del Esclavo -que asignaba la misma
#     bandera dos veces-, sin una linea por forma.
#
# LO QUE ESTE LECTOR NO MIDE, DICHO AQUI PARA QUE NADIE LO LEA COMO CUBIERTO: que la
# puerta se EJERCITE. Es un pack de texto (CLAUDE.md 6.3); quien ejecuta esta funcion
# de verdad es el arnes de dos_puntas. Aqui se mide presencia y simetria, nada mas.
#
# Y LO QUE NO ES UNA EXCEPCION: las dos puntas ganaron con D-29 una guarda de abandono
# DISTINTA -el Esclavo mira mando_ambarLocal(), el Maestro modoActual_get() != MENU-.
# Este lector no la mira y no le hace falta, porque no es una de las cuatro
# condiciones: no hay aqui ninguna frase que excuse un verde. Se anota el motivo por
# si alguien viene a preguntarselo, MEDIDO el 12/09 con fuente.codigo() sobre
# Maestro/{src,include}: mando_ambarLocal aparece 0 veces en el CODIGO del Maestro -un
# 'grep' pelado da 2, y las dos son texto de comentario, que es CLAUDE.md 7.1-, porque
# el B.B.B. de esa punta hace un cambio de MODO y no levanta un cerrojo.
# =================================================================================

def _bloque(texto, i):
    """El interior del bloque que abre en texto[i]. None si no cierra.

    Bloque traido LITERAL de app_04_valores_de_status.py: es la misma idea que ya
    corre en una treintena de packs y no se reescribe aqui otra vez."""
    apertura = texto[i]
    cierre = {"{": "}", "(": ")", "[": "]"}[apertura]
    prof = 0
    for j in range(i, len(texto)):
        if texto[j] == apertura:
            prof += 1
        elif texto[j] == cierre:
            prof -= 1
            if prof == 0:
                return texto[i + 1:j]
    return None


def _cuerpo_funcion(cod, nombre):
    """El cuerpo de una funcion C++ por su NOMBRE. None si no esta.

    Se direcciona por nombre y no por la forma de su cabecera porque un nombre es lo
    unico de una funcion que no cambia al reordenar sus condiciones."""
    m = re.search(r"\b%s\s*\([^)]*\)\s*\{" % re.escape(nombre), cod)
    return None if not m else _bloque(cod, m.end() - 1)


def _alias_bool(cuerpo):
    """{bandera local: todo lo que se le asigna en el cuerpo}.

    Se recogen TODAS las asignaciones y a cualquier profundidad, no solo la
    declaracion: el Esclavo de antes de D-29 declaraba sigueVigente con dos de las
    condiciones y le asignaba las otras dos mas abajo, dentro de un if. Quedarse con
    la declaracion habria perdido la mitad de la puerta."""
    alias = {}
    for m in re.finditer(r"\bbool\s+(\w+)\s*=\s*([^;]*);", cuerpo):
        alias.setdefault(m.group(1), []).append(m.group(2))
    for n in list(alias):
        for m in re.finditer(r"(?<![\w.>])%s\s*=(?!=)\s*([^;]*);" % re.escape(n), cuerpo):
            alias[n].append(m.group(1))
    return {n: " ".join(v) for n, v in alias.items()}


def _decisiones_raiz(cuerpo):
    """Las condiciones de los 'if' de PRIMER NIVEL del cuerpo.

    La profundidad se cuenta por llaves. Vale aqui porque en estos dos ficheros no hay
    literales de cadena con llaves dentro; si algun dia los hubiera, el borde habria
    que contarlo con un tokenizador y no con count()."""
    fuera = []
    for m in re.finditer(r"\bif\s*\(", cuerpo):
        if cuerpo.count("{", 0, m.start()) - cuerpo.count("}", 0, m.start()) != 0:
            continue
        cond = _bloque(cuerpo, m.end() - 1)
        if cond is not None:
            fuera.append(cond)
    return fuera


def _con_alias(cond, alias):
    """La condicion mas el texto de las banderas locales que nombra, transitivamente.

    No reconstruye la expresion -no hace falta-: solo junta texto para poder preguntar
    si una condicion esta o no esta. El conjunto 'visto' es lo que impide que una
    bandera que se nombra a si misma deje el bucle dando vueltas."""
    visto, pend, out = set(), [cond], []
    while pend:
        t = pend.pop()
        out.append(t)
        for n, v in alias.items():
            if n not in visto and re.search(r"\b%s\b" % re.escape(n), t):
                visto.add(n)
                pend.append(v)
    return " ".join(out)


def _horas_limite(cod, nombre_h):
    """Las horas del limite, DERIVADAS de la constante que manda.

    El limite en horas se declara siempre como <X>_MS / 3600000UL -costura_07 lo
    comprueba en las dos puntas-, asi que aqui se lee la constante en ms y se divide.
    Leer el _H directamente seria leer una division sin hacer; escribir 48 seria un
    numero a mano, que es lo que CLAUDE.md 4 prohibe."""
    nombre_ms = nombre_h[:-2] + "_MS" if nombre_h.endswith("_H") else nombre_h
    return producto(cod, nombre_ms) // 3600000


def _condiciones_reanudacion(fw, punta, funcion):
    """Las condiciones que la funcion de reanudacion de una punta exige de verdad.

    Devuelve un dict con una entrada por condicion, mas el limite que encontro. No
    poder acotar el cuerpo es un ABORTADO y no un FALLA: significa que el instrumento
    no midio -la funcion se renombro o se mudo-, y eso no dice nada del firmware
    (CLAUDE.md 1)."""
    cod = fw.codigo(punta, "src", "modo_degradado.cpp")
    cuerpo = _cuerpo_funcion(cod, funcion)
    if cuerpo is None:
        raise fw.Abortado(
            "no se pudo acotar el cuerpo de %s() en %s/src/modo_degradado.cpp: la "
            "funcion se renombro o se mudo de fichero. Sin el cuerpo no se puede "
            "medir la simetria de las condiciones de reanudacion." % (funcion, punta))

    # El numero de horas se sigue desde SU ORIGEN, no por el nombre 'horas': lo que la
    # propiedad exige es que el centinela y el limite se comprueben sobre la MISMA
    # lectura del contador de RTC, y no sobre dos numeros distintos que casualmente se
    # llamen igual.
    mo = re.search(r"\b(\w+)\s*=\s*respaldo_horasDesdeSync\s*\(\s*"
                   r"reloj_contadorSegundos\s*\(\s*\)\s*\)", cuerpo)
    if not mo:
        return {"antiguedad": False, "indicador": False, "hora": False,
                "ciclo": False, "centinela": False, "limite": None, "juntas": False}

    h = re.escape(mo.group(1))
    r_cent = r"(?:%s\s*!=\s*RESPALDO_SYNC_CADUCADA|RESPALDO_SYNC_CADUCADA\s*!=\s*%s)" % (h, h)
    # Se admiten las dos escrituras de "por debajo del limite" -la directa y la negada-
    # porque las dos dicen lo mismo y elegir una seria volver a medir la forma.
    r_lim = (r"(?:%s\s*(?:<|>=)\s*(LIMITE_\w*_H)\b|\b(LIMITE_\w*_H)\s*(?:>|<=)\s*%s)" % (h, h))

    decisiones = [_con_alias(c, _alias_bool(cuerpo)) for c in _decisiones_raiz(cuerpo)]
    r = {"antiguedad": True, "hora": False, "ciclo": False, "centinela": False,
         "limite": None, "juntas": False,
         # El indicador vive en su propia puerta de salida temprana, asi que se pide en
         # CUALQUIER decision de primer nivel y no en la misma que las otras tres.
         "indicador": any(re.search(r"\brespaldo_degradadoActivo\s*\(\s*\)", d)
                          for d in decisiones)}
    for d in decisiones:
        hora = bool(re.search(r"\breloj_enHora\s*\(\s*\)", d))
        ciclo = bool(re.search(r"\brespaldo_hayCiclo\s*\(\s*\)", d))
        cent = bool(re.search(r_cent, d))
        ml = re.search(r_lim, d)
        # Presencia y reunion se apuntan POR SEPARADO, y no es cosmetica: si se marcara
        # todo a la vez, quitar UNA condicion haria que el fallo acusara a las cuatro y
        # el mensaje mentiria sobre lo que se midio. Un rojo que nombra mal lo que le
        # falta manda a mirar donde no es.
        r["hora"] = r["hora"] or hora
        r["ciclo"] = r["ciclo"] or ciclo
        r["centinela"] = r["centinela"] or cent
        if ml and not r["limite"]:
            r["limite"] = ml.group(1) or ml.group(2)
        # LAS CUATRO EN LA MISMA DECISION, no repartidas por el cuerpo: cuatro
        # condiciones que no se juntan nunca no son una puerta, son cuatro notas.
        if hora and ciclo and cent and ml:
            r["juntas"] = True
    if r["limite"]:
        r["horas_limite"] = _horas_limite(cod, r["limite"])
    return r


def correr(b, fw):
    # Bloque traido LITERAL, solo reindentado.
    verificar = b.verificar

    def hallazgo(reproducido, titulo, detalle, consecuencia):
        """El hallazgo de costura lleva CUATRO argumentos y SI cuenta como
        comprobacion: aqui la comprobacion ES reproducir el desajuste, asi que si el
        modelo no lo reprodujera seria el modelo el que esta mal. En el validador del
        Esclavo la misma palabra significa otra cosa y NO cuenta -alli acompana a una
        propiedad() que ya cuenta por su cuenta-. Dos cosas distintas con el mismo
        nombre: por eso 37/41 y 30/31 no se podian sumar."""
        b.hallazgo(reproducido, titulo, [detalle, f"EN LA CALLE: {consecuencia}"])

    b.titulo("REANUDACION TRAS UN CORTE: LA PUNTA QUE SE REINICIA VUELVE EN FASE")


    # La punta que reanuda entra por DEG_ENTRADA_ROJO / DEG_ENTRANDO, que exige DOS
    # condiciones: un despeje completo Y que la fase no sea la de SU propio verde.
    # Se barre el instante de arranque por TODO el dia: si hubiera una sola posicion
    # en la que la punta reanudada se enganchara a mitad de su verde, ahi estaria el
    # fallo.
    m_dos_condiciones = bool(re.search(
        r"millis\(\)\s*-\s*tEstado\s*>=\s*ROJO_TRANSICION_MS\s*&&\s*fase\s*!=\s*FD_VERDE_MAESTRO", T_M_DEG_C))
    e_dos_condiciones = bool(re.search(
        r"\(ahora\s*-\s*tCambioEstado\)\s*>=\s*rojoObligatorioMs\(\)\s*&&\s*\n?\s*calcularFase\(\)\s*!=\s*FD_VERDE_ESCLAVO",
        T_E_DEG_C))
    verificar(m_dos_condiciones and e_dos_condiciones,
              "las dos puntas exigen lo MISMO para abandonar el todo-rojo de entrada: un despeje "
              "completo Y que la fase no sea la de su propio verde. El primer verde tras el corte "
              "es siempre un verde entero contado desde su principio",
              "las condiciones de salida del todo-rojo de entrada NO son simetricas entre puntas")

    # Barrido completo: arranque en cada segundo del dia, para las dos direcciones.
    fallos_reanudacion = []
    for quien in ("MAESTRO", "ESCLAVO"):
        for t0 in range(SEGUNDOS_DEL_DIA):
            # Todo-rojo obligatorio y despues la primera posicion en la que la punta
            # reanudada acepta ceder el rojo.
            t = t0 + DEG_DESPEJE_SEG
            propio = FD_VERDE_MAESTRO if quien == "MAESTRO" else FD_VERDE_ESCLAVO
            vueltas = 0
            while fase(t % SEGUNDOS_DEL_DIA, DEG_VERDE_SEG, DEG_DESPEJE_SEG) == propio:
                t += 1
                vueltas += 1
                if vueltas > 2 * (DEG_VERDE_SEG + DEG_DESPEJE_SEG) + DEG_DESPEJE_SEG + 2:
                    break
            # Desde ese instante la punta reanudada sigue la fase. La otra nunca se
            # reinicio y tambien sigue la fase. Como las dos leen la MISMA funcion,
            # basta comprobar que no hay solape en el ciclo siguiente.
            for d in range(2 * (DEG_VERDE_SEG + DEG_DESPEJE_SEG) + 1):
                s = (t + d) % SEGUNDOS_DEL_DIA
                if luz_maestro(s, DEG_VERDE_SEG, DEG_DESPEJE_SEG) == VERDE and \
                   luz_esclavo(s, DEG_VERDE_SEG, DEG_DESPEJE_SEG, E_AMARILLO_MS // 1000) == VERDE:
                    fallos_reanudacion.append((quien, t0, s))
                    break
            if fallos_reanudacion:
                break
        if fallos_reanudacion:
            break

    verificar(not fallos_reanudacion,
              f"barrido de los {SEGUNDOS_DEL_DIA} instantes de arranque posibles, para las dos "
              "direcciones: la punta que reanuda queda SIEMPRE en fase con la que no se reinicio. "
              "El ciclo se ancla a la hora de pared, no a un contador propio",
              f"la reanudacion deja las puntas desfasadas: {fallos_reanudacion[:3]}")

    # --- 6b. Una reanuda y la otra NO puede ----------------------------------
    # El caso feo. Las condiciones de reanudacion son las MISMAS en las dos puntas
    # -eso esta bien-, pero se evaluan sobre datos que pueden diferir. Y el
    # calendario es uno de ellos.
    # Se mide la PROPIEDAD, no la forma: ver la cabecera de este fichero, donde esta
    # escrito por que este lector no vuelve a acusar al firmware cuando alguien mueva
    # la expresion, y cual es el borde que usa -las decisiones de primer nivel-.
    _COND = {"indicador": "el indicador de la pila (respaldo_degradadoActivo)",
             "hora": "el reloj propio en hora (reloj_enHora)",
             "ciclo": "el ciclo acordado en la pila (respaldo_hayCiclo)",
             "centinela": "el centinela RESPALDO_SYNC_CADUCADA, aparte del numero",
             "limite": "la antiguedad por debajo del limite (LIMITE_*_H)"}
    puntas = {
        "Maestro": _condiciones_reanudacion(fw, "Maestro", "modo_degradado_reanudarTrasCorte"),
        "Esclavo": _condiciones_reanudacion(fw, "Esclavo", "degradado_reanudarTrasCorte"),
    }
    faltan = []
    for p, r in puntas.items():
        if not r.get("antiguedad"):
            # Va aparte y primero: sin origen no hay numero que comparar, y las otras
            # cuatro caerian detras diciendo algo que no es el hallazgo.
            faltan.append("%s: la antiguedad no sale de "
                          "respaldo_horasDesdeSync(reloj_contadorSegundos())" % p)
        faltan += ["%s: no exige %s" % (p, t) for k, t in _COND.items() if not r.get(k)]
        if r.get("hora") and r.get("ciclo") and r.get("centinela") and r.get("limite") \
                and not r.get("juntas"):
            faltan.append("%s: las cuatro estan, pero NO se juntan en una sola decision "
                          "de la puerta: repartidas por el cuerpo no vetan juntas" % p)

    # Y "las MISMAS": cada punta nombra su propia constante -LIMITE_DURO_H en el
    # Maestro, LIMITE_SIN_SYNC_H en el Esclavo-, asi que la simetria no se puede pedir
    # por el nombre. Se pide por el VALOR, derivado de la constante en ms de cada
    # fichero. Sin esto la linea diria "son las mismas" mientras una punta aguanta 48 h
    # y la otra 24, que es exactamente el desajuste que este pack existe para cazar.
    horas = {p: r.get("horas_limite") for p, r in puntas.items()}
    if all(v is not None for v in horas.values()) and len(set(horas.values())) != 1:
        faltan.append("los limites no valen lo mismo: %s" % horas)

    verificar(not faltan,
              "las cuatro condiciones de reanudacion son las mismas en las dos puntas -indicador, "
              "reloj en hora, ciclo en la pila, antiguedad fechable y por debajo del limite-, y las "
              "dos comprueban el centinela CADUCADA aparte del numero. Medido sobre el CUERPO de "
              "cada funcion, no sobre la forma de sus &&: Maestro %s y Esclavo %s, las dos de %s h"
              % (puntas["Maestro"]["limite"], puntas["Esclavo"]["limite"],
                 horas["Maestro"]),
              "las condiciones de reanudacion difieren entre puntas -> " + " | ".join(faltan))

    # RETIRADO el 05/08 (N-49 T1) — SE INVIERTE, no se borra sin dejar rastro.
    #
    # Antes se comprobaba aqui que cada calendario se siembra por separado
    # (rtc.setDay(1)/setMonth(1)) y que la fecha no viaja por el protocolo -las dos
    # cosas siguen siendo ciertas, se pueden ver en T_M_RELOJ_C/T_E_RELOJ_C-, pero ya
    # no son la premisa de ningun hallazgo: lo que importaba de ellas era que
    # alimentaban una reanudacion atada al dia del mes, y eso es lo que T1 quito.
    #
    # Este hallazgo documentaba que, como cada calendario es independiente, la
    # reanudacion podia depender de en que dia del mes cayera cada punta:
    # respaldo_horasDesdeSync() declaraba CADUCADA en cuanto el DIA BAJABA -el cruce
    # de fin de mes-. T1 no acoto el caso: le quito el mecanismo por el que
    # importaba. La marca ya no es dia+segundo, es reloj_contadorSegundos() -un
    # contador monotono del RTC-, y la comparacion siempre resta DOS LECTURAS DE LA
    # MISMA UNIDAD (nunca la de una punta contra el calendario de la otra). Que los
    # calendarios (rtc.setDay(1)) sigan siendo independientes ya no puede afectar a
    # la reanudacion, porque la reanudacion no vuelve a mirar el calendario.
    #
    # Se invierte en un verificar() que exige la firma nueva: si algun dia
    # respaldo_horasDesdeSync() vuelve a tomar una fecha de calendario, esto tiene
    # que volver a fallar.
    firma_contador_unico = bool(re.search(
        r"uint32_t\s+respaldo_horasDesdeSync\s*\(\s*uint32_t\s+segundosRtcAhora\s*\)", T_M_RESP_C))
    verificar(firma_contador_unico,
              "respaldo_horasDesdeSync() recibe UN SOLO contador de RTC, no un dia y un segundo: "
              "la reanudacion ya no puede depender de en que calendario -propio de cada unidad- "
              "caiga cada punta, aunque los calendarios (rtc.setDay(1)) sigan siendo independientes.",
              "respaldo_horasDesdeSync() ha vuelto a tomar una fecha de calendario: el riesgo que "
              "N-49 T1 cerro -reanudacion atada a calendarios independientes entre puntas- podria "
              "estar de vuelta")

    # Lo que SI rescata el caso, y conviene tenerlo escrito: si el radio vive, la
    # punta que no reanuda vuelve al menu, el coordinador emite CMD_GO_RED cada 3 s y
    # eso saca al Esclavo del Degradado por la via ordenada.
    menu_emite_rojo = bool(re.search(r"estadoC\s*==\s*C_MENU_IDLE\s*\|\|\s*estadoC\s*==\s*C_FALLO\)\s*\{\s*\n\s*protocolo_enviarPaquete\(CMD_GO_RED\)",
                                     T_M_COORD_C))
    gobierno_saca = bool(re.search(r"degradado_gobiernaLuz\(\)\s*&&\s*\n?\s*\(pkt\.command\s*==\s*CMD_PING\s*\|\|\s*pkt\.command\s*==\s*CMD_GO_RED\s*\|\|\s*pkt\.command\s*==\s*CMD_GO_GREEN\)\)\s*\{\s*\n\s*degradado_salir\(\)",
                                   T_E_MAIN_C))
    verificar(menu_emite_rojo and gobierno_saca,
              "con el radio vivo, la asimetria se cura sola: la punta que arranca en el menu emite "
              "CMD_GO_RED cada 3 s y el Esclavo sale del Degradado por todo-rojo. Solo las tramas de "
              "GOBIERNO lo sacan; las de servicio no",
              "una punta en el menu no saca a la otra del Degradado ni con el radio vivo")

    # --- 6c. La configuracion del ciclo SE REENCOLA al volver el enlace --------
    # N-49 T1 (§8.quater): modo_degradado_publicarConfig() solo se llama en setup(),
    # pero al recuperarse la conexion coordinador_reiniciarConexion() pone pendHora = true,
    # pendConfig = true y configConfirmada = false (coordinador.cpp:498-500).
    # Ademas, modo_degradado_evaluarEntrada() exige MDG_SIN_CONFIG (modo_degradado.cpp).
    # Se invierte el hallazgo en un verificar() que exige estas tres garantias.
    reencola_hora = bool(re.search(r"pendHora\s*=\s*true;", T_M_COORD_C))
    reencola_config = bool(re.search(r"pendConfig\s*=\s*true;\s*\n\s*configConfirmada\s*=\s*false;", T_M_COORD_C))
    gate_maestro_config = bool(re.search(r"MDG_SIN_CONFIG", T_M_DEG_C))

    verificar(reencola_hora and reencola_config and gate_maestro_config,
              "al recuperarse el enlace, el Maestro reencola HORA (pendHora = true) y CONFIGURACION "
              "(pendConfig = true) y su puerta de entrada exige MDG_SIN_CONFIG",
              "el Maestro no reencola la configuracion al reconectar o su puerta no exige MDG_SIN_CONFIG")

    # ===========================================================================
    # 7. LOS MOTIVOS DE RECHAZO
