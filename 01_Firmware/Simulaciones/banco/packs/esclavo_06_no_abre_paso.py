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
    "FORZAR_ROJO":     "direccion segura: detiene el trafico",
    "SOLICITAR_PASO":  "PIDE al Maestro; no enciende nada en esta punta",
    "TEST_LEDS":       "presente solo para RECHAZARLO con un motivo legible",
    "SET_RTC:":        "ajusta el reloj, no las luces",
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
