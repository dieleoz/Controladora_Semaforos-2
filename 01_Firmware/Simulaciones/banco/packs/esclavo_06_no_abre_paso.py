# ===== banco/packs/esclavo_06_no_abre_paso.py =====
#
# EL ESCLAVO PIDE; NO ORDENA. NINGUN COMANDO DE BLUETOOTH ABRE PASO EN ESTA PUNTA.
#
# POR QUE EXISTE ESTE PACK.
#
# semaforo_iniciarTestLeds() enciende 6 s de secuencia -rojo, ambar y VERDE- y no mira
# nada: ni el estado del ciclo, ni si hay enlace, ni si el Maestro esta dando paso al
# otro sentido. Durante meses estuvo colgado del Bluetooth del Esclavo, asi que
# cualquiera con un movil a 15 m de un poste lanzaba un verde en una punta mientras la
# otra tenia verde tambien. Dos vehiculos entrando de frente al tramo.
#
# Y LO IMPORTANTE, QUE COSTO VER: conectarse al Esclavo CORRECTO era igual de
# peligroso. El fallo no era equivocarse de poste -eso lo arreglaria una matricula-;
# era que esta punta aceptase mover luces. Por eso la guarda no puede vivir en la app:
# una app se actualiza, se instala otra, se usa una vieja. Vive aqui.
#
# LA REGLA QUE SE VIGILA, en una linea: al Esclavo se le puede PEDIR y se le puede
# PARAR; no se le puede ABRIR.
#
#   PETICION  ->  SOLICITAR_PASO manda CMD_DEMANDA por radio. El MAESTRO decide,
#                 aplica el todo-rojo y ordena. Aqui no se enciende nada.
#   PARADA    ->  FORZAR_ROJO es la direccion segura y ni siquiera pide PIN.
#   APERTURA  ->  no existe, y este pack es lo que impide que vuelva.
#
# LA LISTA BLANCA SE ESCRIBE A MANO, Y ESO ES DELIBERADO.
#
# Es lo contrario de barrera_01, donde los pines se descubren para que uno nuevo entre
# bajo custodia solo. Aqui el descubrimiento seria el defecto: si el pack leyera los
# comandos del propio fuente, un comando nuevo se aprobaria a si mismo. Escribirla a
# mano obliga a que cualquiera que anada un comando al Esclavo pase por este fichero y
# justifique por que no abre paso.

import re

# EJERCE SFTY-2: ningun camino del Bluetooth del Esclavo puede producir un verde.

NOMBRE = "esclavo_06_no_abre_paso"
DESCRIPCION = "el Esclavo pide y para, pero no abre: ningun comando BT enciende un verde"

# Funciones de semaforo.cpp capaces de encender un VERDE. Un comando de Bluetooth que
# llame a cualquiera de estas esta abriendo paso por su cuenta.
ABREN_PASO = ("semaforo_iniciarTestLeds",)

# Lo que el Bluetooth del Esclavo puede atender. Anadir aqui exige justificarlo arriba.
COMANDOS_PERMITIDOS = {
    # N-83: aqui ponia "FORZAR_ROJO: direccion segura: detiene el trafico", y ese motivo
    # era FALSO -es la clase de prueba que documenta el defecto en vez de cazarlo, la de
    # 8.quater-. El comando llamaba a semaforo_iniciarFallo(), o sea ambar intermitente
    # con la TALANQUERA ARRIBA: no detenia el trafico, abria paso a los dos sentidos con
    # precaucion. La lista blanca daba por revisado un comando cuyo motivo escrito
    # describia otra cosa, que es peor que no tenerlo listado.
    #
    # El nombre se corrigio; la entrada se INVIERTE en vez de borrarse -sigue midiendo
    # algo-, y el literal viejo se queda con el motivo de TEST_LEDS, que es el que
    # ahora le corresponde.
    "AMBAR_EMERGENCIA": "ambar intermitente: no le da prioridad a NADIE, no abre paso "
                        "a un sentido contra el otro",
    # N-106 / R-3 (31/08). EL MOTIVO SE ESCRIBE ENTERO, INCLUIDA LA PARTE INCOMODA, que
    # es justo donde N-83 se equivoco: alli la lista blanca dio por revisado un comando
    # cuyo motivo describia otra cosa de la que hacia.
    #
    # Este comando NO enciende nada -no llama a ninguna de ABREN_PASO ni ordena una luz-,
    # pero tampoco es inocuo, y decir "solo quita una bandera" seria el mismo error: al
    # retirar el latch se apagan los tres vetos de main.cpp (:406, :416, :540), y el
    # SIGUIENTE CMD_GO_GREEN del Maestro pasa a obedecerse. O sea que este comando puede
    # terminar en un verde en esta punta.
    #
    # Se acepta por tres razones, y las tres son comprobables en el fuente:
    #   1. El verde lo decide el MAESTRO, no esta punta. Aqui no se elige fase: se vuelve
    #      a obedecer. Es exactamente lo que ya hace el A.A.A del mando (ACC_OBEDECER),
    #      que tambien apaga su latch y devuelve el nodo al gobierno por radio.
    #   2. PIDE PIN. Es la asimetria de mando.cpp leida al derecho: pedir el ambar es la
    #      caida segura y entra sin clave; QUITARLO devuelve el cruce a dar verdes, y eso
    #      es lo que el PIN existe para custodiar.
    #   3. Sin el, el latch no tenia mas salida que subir al gabinete -y ademas se caia
    #      solo, que era peor: la maquina deshaciendo una proteccion que puso una persona.
    #
    # N-152 (05/09): EL MOTIVO SE COMPLETA, NO SE REESCRIBE. La entrada sigue midiendo
    # lo mismo -que este comando no llame a ninguna funcion que encienda un verde-, y eso
    # no ha cambiado. Lo que cambio es lo que el comando HACE ADEMAS: ahora manda una
    # trama por radio pidiendole al Maestro que salga de SU ambar, y el Maestro sale al
    # todo-rojo. Un motivo de lista blanca es una afirmacion sobre el codigo (CLAUDE.md
    # 2.ter): dejarlo describiendo el comando de antes seria un defecto con permiso.
    #
    # Por que se acepta la parte nueva, y es comprobable en costura_14: el Maestro solo
    # sale del ambar QUE PIDIO ESTA PUNTA -no del que pidio una persona del Poste 1-, y
    # sale al todo-rojo que no programa ningun cambio. O sea que sigue sin abrir paso:
    # para el cruce en rojo y deja que decida quien esta alli.
    "CANCELAR_AMBAR":   "retira el latch del ambar de la app; NO enciende nada, pero "
                        "levanta el veto y el siguiente verde del Maestro se obedece. "
                        "Pide PIN, y el verde lo sigue decidiendo el Maestro (R-3). "
                        "N-152: ademas AVISA al Maestro por radio, que sale de su ambar "
                        "-solo si era el que pidio esta punta- al todo-rojo, no al ciclo",
    # A-11 (05/09). LA ENTRADA MAS INCOMODA DE ESTA LISTA, Y SE ESCRIBE ENTERA.
    #
    # LAS OTRAS CINCO CABEN EN "no abre paso". ESTA SI ABRE PASO, y decir otra cosa seria
    # repetir el error de N-83 -una lista blanca dando por revisado un comando cuyo motivo
    # describe otra cosa de la que hace-. El Modo Degradado da VERDE guiandose por el
    # reloj, y lo da SIN confirmacion del otro extremo: es el unico camino del firmware
    # que enciende un verde sin que nadie diga que el otro sentido esta en rojo.
    #
    # NO ES UN AGUJERO NUEVO: el modo existe desde el 31/07 y ya tenia esa propiedad. Lo
    # que hasta el 05/09 no existia era una PUERTA ABRIBLE -sus tres llamadores estaban
    # muertos: el mando sin hardware (D-1), el menu sin botonAceptar() (D-2), y la
    # reanudacion tras corte, que exige haber entrado antes-. O sea que lo que este
    # comando cambia no es lo que el modo hace: es que se pueda pedir.
    #
    # POR QUE SE ACEPTA. Las cuatro razones son comprobables en el fuente, y ninguna es
    # "porque lo decidio el responsable" -eso autoriza el cambio, no lo justifica-:
    #
    #   1. NO ELIGE LUZ. La rama llama a degradado_entrar() y a nada mas. La fase la
    #      calcula ciclo_degradado_fase(), que es el MISMO fichero en las dos puntas
    #      (costura_02_fase_ciclo), a partir de la hora del dia y de la configuracion que
    #      el Maestro dejo verificada. Aqui no hay un verde que elegir.
    #   2. LA PUERTA ES UNA Y NO SE COPIA. degradado_entrar() vuelve a evaluar TODAS las
    #      condiciones -reloj en hora, ciclo conocido y no nulo, sincronizacion recibida y
    #      no caducada, sin ambar de emergencia puesto-. La rama no comprueba nada por su
    #      cuenta: tres vias con tres criterios serian la mas floja de las tres.
    #   3. ENTRA POR TODO-ROJO, SIEMPRE. degradado_entrar() arranca con
    #      semaforo_forzarRojo() y el modo no da su primer verde hasta agotar
    #      rojoObligatorioMs(). O sea que el efecto inmediato de este comando es PARAR,
    #      no abrir.
    #   4. PIDE PIN, y ademas la app lo pone detras de un dialogo con casilla. Es la
    #      misma asimetria de CANCELAR_AMBAR leida al derecho.
    #
    # 🔴 Y LO QUE NO PROTEGE, ESCRITO PARA QUE NO SE LEA COMO QUE SI: con el Maestro vivo
    # este modo no se sostiene. main.cpp de esta punta llama a degradado_salir() al
    # recibir CMD_PING, CMD_GO_RED o CMD_GO_GREEN, y el Maestro emite PING cada
    # LATIDO_MS. Lo que impide que en ese rato salga un verde por reloj es que
    # ROJO_MINIMO_MS sea MAYOR que LATIDO_MS, y esa desigualdad no vive aqui: la
    # recalcula esclavo_08_ambar_en_degradado desde el C++ de las dos puntas (N-71).
    "SET_MODO:DEGRADADO":
                        "ABRE PASO, y por eso el motivo va entero arriba: da verde por "
                        "reloj sin confirmar la otra punta. Se acepta porque no elige "
                        "luz -la fase la calcula el fichero compartido-, porque entra "
                        "por todo-rojo, porque la puerta es degradado_entrar() y no una "
                        "copia de sus condiciones, y porque pide PIN",
    "FORZAR_ROJO":      "presente solo para RECHAZARLO ensenando el nombre nuevo",
    "SOLICITAR_PASO":   "PIDE al Maestro; no enciende nada en esta punta",
    "TEST_LEDS":        "presente solo para RECHAZARLO con un motivo legible",
    "SET_RTC:":         "ajusta el reloj, no las luces",
}


def _comandos_atendidos(codigo):
    """Los comandos que el despachador compara, leidos del fuente."""
    return set(re.findall(r'strn?cmp\s*\(\s*accion\s*,\s*"([^"]+)"', codigo))


def correr(b, fw):
    b.titulo("El Esclavo pide y para, pero no abre paso")

    bt = fw.codigo("Esclavo", "src", "bluetooth.cpp")       # sin comentarios

    # ---- 1. Ninguna funcion que encienda un verde se llama desde el Bluetooth ----
    llamadas = [f for f in ABREN_PASO if re.search(r"\b%s\s*\(" % re.escape(f), bt)]
    b.verificar(
        not llamadas,
        "Esclavo/bluetooth.cpp no llama a ninguna funcion capaz de encender un verde",
        f"EL ESCLAVO ABRE PASO POR BLUETOOTH: {llamadas}. Ese camino enciende luces sin "
        "consultar al Maestro, asi que puede sacar un verde en esta punta mientras la "
        "otra tambien lo tiene. Es la unica forma en que este equipo mata a alguien")

    # ---- 2. El despachador no ha crecido por la espalda ----
    atendidos = _comandos_atendidos(bt)
    b.verificar(
        len(atendidos) >= 3,
        f"se leyeron {len(atendidos)} comandos del despachador del Esclavo",
        f"solo se hallaron {len(atendidos)} comandos ({atendidos}). O el fuente cambio "
        "de forma, o el patron se quedo ciego: una lista vacia daria PASS sin mirar "
        "nada, que es justo la prueba muerta que este banco persigue")

    intrusos = sorted(atendidos - set(COMANDOS_PERMITIDOS))
    b.verificar(
        not intrusos,
        "todos los comandos que atiende el Esclavo estan en la lista blanca revisada: "
        + ", ".join(sorted(atendidos)),
        f"COMANDO NO REVISADO en el Esclavo: {intrusos}. Puede ser inofensivo, pero "
        "nadie lo ha mirado. Anadirlo a COMANDOS_PERMITIDOS con su motivo es parte de "
        "escribirlo")

    # ---- 3. TEST_LEDS esta, pero para rechazarlo ----
    b.verificar(
        "TEST_LEDS" in atendidos and not llamadas,
        "TEST_LEDS se atiende para RECHAZARLO, no para ejecutarlo: el operario recibe "
        "un motivo en vez de un silencio",
        "TEST_LEDS desaparecio del despachador. Un comando que no se contesta se lee "
        "como equipo colgado, y el tecnico lo reintenta")

    # ---- 3.bis A-11: la puerta del Degradado existe, y su respuesta se MIRA ----
    #
    # DOS PROPIEDADES EN UNA COMPROBACION, Y LAS DOS SE PERDIERON ANTES EN ESTE PROYECTO.
    #
    # La primera es que la puerta EXISTA. Hasta el 05/09 degradado_entrar() tenia tres
    # llamadores y los tres estaban muertos o inalcanzables (A-11): el modo estaba
    # construido, probado por un arnes en 18/18, y no habia forma de pedirlo. Un modo sin
    # puerta no da FALLA en ningun sitio -es codigo correcto que nadie ejecuta-, asi que
    # hace falta alguien que cuente los llamadores vivos.
    #
    # La segunda es que la respuesta se USE. degradado_entrar() devuelve un
    # RechazoDegradado con SEIS motivos distintos, no un bool. Una llamada suelta
    # -`degradado_entrar();`- compila igual, entra igual cuando puede, y deja al
    # despachador contestando $ACK,RESULT:OK a un rechazo: la mentira con formato de
    # exito de CLAUDE.md 6. Es exactamente lo que le pasa hoy a mando.cpp:148, que la
    # llama suelta -alli no hay a quien contestar, no hay cable de vuelta-; en el
    # Bluetooth SI lo hay, y por eso la exigencia es de este fichero y no del otro.
    #
    # SE MIDE SOBRE EL FUENTE SIN COMENTARIOS. El patron busca la llamada como SENTENCIA
    # entera -precedida de ';', '{' o '}'-, que es la unica forma en que el valor se tira.
    RE_ENTRAR = re.compile(r"\bdegradado_entrar\s*\(\s*\)")
    RE_ENTRAR_SUELTA = re.compile(r"(?:^|[;{}])\s*degradado_entrar\s*\(\s*\)\s*;")

    entradas = RE_ENTRAR.findall(bt)
    sueltas = RE_ENTRAR_SUELTA.findall(bt)
    b.verificar(
        len(entradas) >= 1 and not sueltas,
        "el Bluetooth del Esclavo abre la puerta del Degradado %d vez/veces y MIRA lo "
        "que devuelve: cada motivo de rechazo puede tener su propio $ERR" % len(entradas),
        "el Bluetooth del Esclavo %s. Sin llamador, el Modo Degradado de esta punta "
        "vuelve a ser una funcion terminada y sin usuario posible (A-11); con la llamada "
        "suelta, el valor de RechazoDegradado se tira y el despachador contesta que si a "
        "un rechazo -el operario se va del poste creyendo que dejo el modo puesto-"
        % ("no llama a degradado_entrar()" if not entradas
           else "llama a degradado_entrar() como sentencia suelta, sin usar el "
                "RechazoDegradado que devuelve"))

    # ---- 4. La demanda sale por UNA sola puerta ----
    # Dos origenes -la camara de PB0 y el boton de la app- significan lo mismo. Si cada
    # uno llevase su temporizador, el limite de ritmo de uno no sabria nada del otro.
    fuera = []
    for fichero in ("main.cpp", "bluetooth.cpp", "modo_degradado.cpp", "menu.cpp"):
        try:
            codigo = fw.codigo("Esclavo", "src", fichero)
        except Exception:
            continue
        if re.search(r"protocolo_enviarPaquete\s*\(\s*CMD_DEMANDA", codigo):
            fuera.append(f"Esclavo/src/{fichero}")

    b.verificar(
        not fuera,
        "CMD_DEMANDA solo se emite desde demanda_solicitar(): los dos origenes "
        "comparten la misma ventana de silencio",
        f"CMD_DEMANDA se emite por fuera de la puerta unica, en {fuera}. Dos origenes "
        "con dos temporizadores acaban divergiendo, y una cola de coches se convierte "
        "en una rafaga de tramas identicas sobre un canal de 2.4 kbps")

    # La puerta vive en demanda.cpp, NO en protocolo.cpp, y el sitio importa:
    # protocolo.h/.cpp son contrato compartido y deben ser identicos byte a byte en las
    # dos puntas -costura_01_contratos lo exige-. El formato de aire lo acuerdan los dos
    # extremos; la ventana de silencio con la que ESTA punta decide cuando pedir es
    # politica local. Ponerla en el contrato lo rompio, y el banco lo cazo.
    puerta = fw.codigo("Esclavo", "src", "demanda.cpp")
    b.verificar(
        re.search(r"demanda_solicitar\s*\(\s*\)\s*\{", puerta) is not None,
        "demanda_solicitar() vive en demanda.cpp -modulo local-, no en el contrato "
        "compartido protocolo.cpp",
        "no se encuentra demanda_solicitar() en Esclavo/src/demanda.cpp")

    contrato = fw.codigo("Esclavo", "src", "protocolo.cpp")
    b.verificar(
        "demanda_solicitar" not in contrato,
        "el contrato compartido protocolo.cpp NO lleva politica local de esta punta",
        "la puerta de demanda volvio a protocolo.cpp: eso rompe la identidad byte a "
        "byte con el Maestro que exige costura_01_contratos")

    b.verificar(
        re.search(r"demanda_solicitar\s*\(", bt) is not None,
        "SOLICITAR_PASO entra por esa misma puerta, no por un camino propio",
        "el Bluetooth del Esclavo no usa demanda_solicitar(): o no pide, o se "
        "ha abierto un segundo camino sin limite de ritmo")

    # ---- CONTROL NEGATIVO ----
    # Sin esto, el dia que el patron dejara de casar -otro formato, otro nombre- el pack
    # aprobaria todo y nadie se enteraria. Es como se perdio la cobertura en N-27.
    mutado = bt + "\nvoid _fuga(){ semaforo_iniciarTestLeds(); }\n"
    b.control_negativo(
        bool(re.search(r"\bsemaforo_iniciarTestLeds\s*\(", mutado)),
        "una llamada a semaforo_iniciarTestLeds() colada en bluetooth.cpp se detecta")

    mutado2 = "void _fuga2(){ protocolo_enviarPaquete(CMD_DEMANDA); }"
    b.control_negativo(
        bool(re.search(r"protocolo_enviarPaquete\s*\(\s*CMD_DEMANDA", mutado2)),
        "un segundo emisor de CMD_DEMANDA fuera de la puerta unica se detecta")

    # A-11: los DOS lectores de 3.bis se ejercen contra texto que trae el defecto, y por
    # separado. Un solo control que mezclara las dos formas no diria cual de los dos
    # patrones se quedo ciego, y el que importa es el segundo: la llamada suelta es
    # SINTACTICAMENTE VALIDA y compila, asi que ningun compilador la delata.
    b.control_negativo(
        not RE_ENTRAR.search("void _sinpuerta(){ enviarTramaConCrc(\"$ACK\"); }"),
        "un bluetooth.cpp sin ningun llamador de degradado_entrar() se detecta: es el "
        "estado en que A-11 encontro esta punta")
    b.control_negativo(
        bool(RE_ENTRAR_SUELTA.search(
            "void _muda(){ degradado_entrar(); "
            "enviarTramaConCrc(\"$ACK,CMD:SET_MODO:DEGRADADO,RESULT:OK\"); }")),
        "una llamada SUELTA a degradado_entrar() con el $ACK detras se detecta: es el "
        "OK mudo que contesta OK a un rechazo")
