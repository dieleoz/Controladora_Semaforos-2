# ===== banco/packs/camara_02_j16.py =====
#
# J16 DEJA DE SER SOLO BOTONERA: DOS DE SUS CUATRO POSICIONES SON CAMARAS.
#
# Decision del 31/08/2026. El mando se queda con A (BOTON1, PB9, J16 p5) y B (BOTON2,
# PB13, J16 p8); C (PB14, p10) y D (PB15, p12) pasan a ser entradas de camara de contacto
# seco. No es un cambio de nombre: cambia el MODO del pin -INPUT pelado, no
# INPUT_PULLUP-, cambia la POLARIDAD -activo en ALTO, no en BAJO- y cambia QUIEN lo lee.
#
# ---------------------------------------------------------------------------------
# POR QUE ESTE PACK EXISTE, Y POR QUE MIRA LAS DOS PUNTAS A LA VEZ
#
# Es N-97. Hasta el 31/08 la camara de demanda NO ERA LA MISMA ENTRADA en los dos
# equipos, y nadie lo medía:
#
#   Maestro   pinMode(CAM_DEMANDA_PIN, INPUT) DENTRO de modoInteligente_setup(): fuera
#             del Modo Inteligente el pin no estaba ni configurado.
#   Esclavo   pinMode en setup(), incondicional, y lectura por FLANCO sin antirrebote.
#
# Los documentos, mientras tanto, hablaban de "las camaras" como si fueran una sola cosa.
# Es la forma exacta del defecto de SFTY-2 que costo el `amarillo = false` de mas en el
# Esclavo: dos copias que solo la disciplina mantiene iguales, y la disciplina no falla
# cuando alguien cambia una - se limita a no enterarse.
#
# ---------------------------------------------------------------------------------
# LA POLARIDAD NO ES UNA PREFERENCIA: ES LA CUENTA DE N-67, OTRA VEZ
#
# R67 y R68 son 10K A MASA sobre las redes /Boton3 y /Boton4, y J16 saca 3,3 V en p9 y
# p11 -las posiciones de al lado-. Pull-DOWN con la tension a un pin de distancia: el
# contacto seco cierra a 3,3 V, o sea ACTIVO EN ALTO. Con INPUT_PULLUP el pull-up interno
# (~40 kOhm) contra ese 10K deja el pin en 3,3 x 10/50 = 0,66 V, que el micro lee LOW:
# DEMANDA PERMANENTE sin camara conectada, e invertida al cerrarla. Las dos son de calle,
# y el defecto NO SE VE EN EL PC: solo aparece con la bornera cableada, y para entonces ya
# hay alguien subido a un poste.
#
# ---------------------------------------------------------------------------------
# LO QUE ESTE PACK NO PUEDE COMPROBAR, Y VA ESCRITO PARA QUE NADIE LO LEA COMO PERMISO
#
# Que el firmware sea coherente con el NETLIST no demuestra que sea coherente con la
# PLACA SOLDADA.
#
# 🟢 M3 SE CERRO EN BANCO EL 03/09 (paso 20): p10 = 9,93 kOhm a masa, p12 = 9,94, los dos
# a 0 V con energia. El pull-down es REAL y de 10K, y ya se puede cablear camara a J16.
#
# 🔴 Y AQUI ABAJO PONIA UNA COSA QUE ERA FALSA, ASI QUE SE TACHA EN VEZ DE BORRARSE. Decia:
# "si la placa fuera la del netlist, PB9 y PB13 en INPUT_PULLUP estarian en LOW permanente
# y el menu no se podria navegar, Y HAY EVIDENCIA DE BANCO DE QUE SE NAVEGA". Sobre esa
# ultima frase se sostenia que A y B eran un caso distinto de C y D.
#
# NO EXISTE ESA EVIDENCIA. Lo que 17_Arquitectura citaba como tal es un PROTOCOLO -un plan
# de pruebas, no un resultado-. La unica observacion real de banco sobre estos pines es la
# contraria: N-26 (01/08, commit b581000) apunta que "la tarjeta se plantaba sola en la
# pantalla de configuracion del Modo Manual sin que nadie tocara la botonera", que es la
# firma exacta de unos pines en BAJO al arrancar con J16 vacio. Y el paso 29 del 03/09
# puenteo p5/p8 contra masa sin ningun cambio de comportamiento: el pin YA estaba en bajo.
#
# Comprobado ademas que no es una regresion nuestra: el repositorio del que salio este
# firmware -2semaforos_3estados- trae el mismo `== LOW`, el mismo INPUT_PULLUP y el MISMO
# .kicad_pcb byte a byte (md5 088667eac75207e8dcfa0ce5b93adce6), sin una sola R65-R68
# marcada como no montar. La contradiccion es original.
#
# El firmware va primero de todos modos (CLAUDE.md 9.bis) y ese orden es el seguro: un pin
# en INPUT no ejecuta nada, mientras que con el firmware viejo dentro PB14 sigue siendo
# botonAceptar() leido activo en BAJO y cualquier hilo enchufado en J16 p10 pulsa ACEPTAR
# en un equipo que esta en la calle.

import re

# EJERCE SFTY-21: que los dos pines del mando (A y B) se lean con la polaridad que pide el
# conector -INPUT pelado, activo en ALTO- y sigan alimentando mando_registrarPulso(), que
# es de donde cuelga el veto de mando_ambarLocal(). Hasta el 04/09 esta etiqueta decia
# "sigan en INPUT_PULLUP", y con eso la regla que dice ejercer estaba muerta (N-118).

NOMBRE = "camara_02_j16"
DESCRIPCION = "J16: A y B siguen siendo botones, C y D son camaras, y las dos puntas las leen igual"

PUNTAS = ("Maestro", "Esclavo")

# El reparto DECIDIDO el 31/08. No es un valor del firmware -es la decision que el
# firmware tiene que cumplir-, asi que va aqui; lo que se lee del C++ es a que pin fisico
# apunta cada nombre, y eso es lo que se compara contra esto.
MANDO_ESPERADO = {"BOTON1": "PB9", "BOTON2": "PB13"}
CAMARAS_J16_ESPERADAS = {"CAM_C_PIN": "PB14", "CAM_D_PIN": "PB15"}

# Los tres nombres de entrada de camara que el firmware debe conocer. CAM_DEMANDA_PIN es
# la de siempre (PB0, bornera J14); las otras dos son las de J16.
CAMARAS = ("CAM_DEMANDA_PIN", "CAM_C_PIN", "CAM_D_PIN")

# El rele de la camara AcuSense cierra ~1 s por deteccion (Manual 9). La ventana de
# silencio tiene que superarlo o la MISMA deteccion se cuenta dos veces. El numero no se
# escribe aqui: se lee del C++ de cada punta y se comprueba la propiedad.
PULSO_RELE_MS = 1000

# Las funciones que forman el camino de camara. Si alguna se renombra, el pack ABORTA en
# vez de aprobar: un patron que no encuentra nada NO demuestra que no haya nada.
CAMINO_CAMARA = ("camara_leerPin", "camaras_sembrar", "camaras_actualizar")

# Los ocho pines de luz. Ninguno puede aparecer en botones.cpp: solo semaforo.cpp los
# escribe (CLAUDE.md 6). Se nombran aqui y se resuelven contra pines.h, de modo que un
# renombrado en el firmware haga abortar en vez de aprobar por no encontrar nada.
PINES_DE_LUZ = ("ROJO1", "ROJO2", "AMARILLO1", "AMARILLO2", "VERDE1", "VERDE2",
                "ROJO_PEATON", "VERDE_PEATON")

# Los sustitutos de botonAceptar()/botonCancelar() en el Maestro. Esta lista NO es
# decorativa: retirar la unica salida de los ocho modos sin que exista el comando que la
# sustituye deja al operario dentro de un modo sin mas salida que cortar la energia.
SUSTITUTOS_MAESTRO = (
    "SET_MODO:MENU",          # la salida de todos los modos
    "SET_MODO:AUTO", "SET_MODO:MANUAL", "SET_MODO:AMBAR",
    "SET_MODO:ALCANCE", "SET_MODO:INTELIGENTE", "SET_MODO:DEGRADADO",
    "MANUAL:CAMBIAR_TURNO",   # el "dar paso" del Modo Manual
    "SET_TIEMPOS:",           # confirmar tiempos, que hacia el boton 3
    "SET_RTC:", "REINICIAR_RELOJ",
)

_DEF = re.compile(
    r"^(?:static\s+)?(?:void|bool|int|uint8_t|unsigned\s+long|const\s+char\s*\*|"
    r"EstadoSemaforo)\s+(\w+)\s*\([^)]*\)\s*\{", re.M)


# ---------------------------------------------------------------------------------
# HERRAMIENTAS. El clasificador por funcion viene LITERAL de maestro_09_test_leds: se
# reutiliza en vez de reescribirlo porque reescribir logica ya probada para renombrar
# llamadas es como se cuelan los errores en un cambio que no debe cambiar nada.

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


def _quien_contiene(funciones, pos):
    for nombre, ini, fin in funciones:
        if ini <= pos < fin:
            return nombre
    return None


def _cuerpo(codigo, nombre):
    for n, ini, fin in _funciones(codigo):
        if n == nombre:
            return re.sub(r"\s+", " ", codigo[ini:fin]).strip()
    return None


def _pin(fw, punta, nombre):
    """A que pin fisico apunta un #define de pines.h. ABORTA si no aparece.

    Sin valor por defecto: si el nombre cambia, el pack no puede medir, y un pack que no
    puede medir no aprueba - aborta."""
    m = re.search(r"#define\s+%s\s+(P[A-Z]\d+)\b" % re.escape(nombre),
                  fw.texto(punta, "include", "pines.h"))
    if not m:
        raise fw.Abortado(
            "no se pudo leer del C++ a que pin apunta %s en %s/include/pines.h. Sin ese "
            "dato el pack mediria otra cosa que el firmware y seguiria dando PASS"
            % (nombre, punta))
    return m.group(1)


def _fuentes(fw, punta):
    """nombre -> codigo sin comentarios de cada .cpp de la punta, censando el DIRECTORIO.

    Se censa el directorio y no una lista escrita a mano porque una lista se queda corta
    el dia que alguien anade un .cpp, y entonces la prueba aprueba sin haber mirado donde
    hacia falta."""
    return {f: fw.codigo(punta, "src", f) for f in fw.fuentes_de(punta, "src")}


def correr(b, fw):
    b.titulo("J16: dos botones, dos camaras, y las mismas en las dos puntas")

    # =============================================================================
    # 1. EL REPARTO DE J16, LEIDO DEL C++
    # =============================================================================
    mapa = {}
    for punta in PUNTAS:
        pines = fw.texto(punta, "include", "pines.h")
        fuentes = _fuentes(fw, punta)
        todo = "".join(fuentes.values()) + fw.codigo(punta, "include", "botones.h")

        # -- 1.1 PB14 y PB15 ya no son botones, ni por el nombre ni por el uso --
        viejos = [n for n in ("BOTON3", "BOTON4")
                  if re.search(r"#define\s+%s\b" % n, pines)]
        b.verificar(
            not viejos,
            "%s: BOTON3 y BOTON4 ya no se declaran en pines.h" % punta,
            "%s: %s vuelve(n) a declararse en pines.h. PB14 y PB15 son camaras desde el "
            "31/08: un BOTON3 que es una camara es como se cuelan los errores, y ademas "
            "invita a que alguien le vuelva a poner INPUT_PULLUP"
            % (punta, ", ".join(viejos)))

        usos = sorted(f for f, c in fuentes.items()
                      if re.search(r"\bBOTON[34]\b", c))
        b.verificar(
            not usos and not re.search(r"\bBOTON[34]\b", todo),
            "%s: ningun fuente nombra ya BOTON3 ni BOTON4" % punta,
            "%s: BOTON3/BOTON4 siguen usados en %s. El nombre viejo sobreviviendo al "
            "cambio de funcion es la senal de que hay un camino que todavia trata esos "
            "pines como pulsadores" % (punta, usos or "algun header"))

        # -- 1.2 Y se llaman por lo que son, sobre los pines decididos --
        for nombre, esperado in sorted(CAMARAS_J16_ESPERADAS.items()):
            real = _pin(fw, punta, nombre)
            mapa[(punta, nombre)] = real
            b.verificar(
                real == esperado,
                "%s: %s apunta a %s, la posicion de J16 que la decision del 31/08 le da"
                % (punta, nombre, real),
                "%s: %s apunta a %s y la decision del 31/08 dice %s. Si de verdad se "
                "movio la camara de posicion, hay que mover con ella la tabla de "
                "05_Funcional/17_...:1.7 y el aviso de los 12 V de p1"
                % (punta, nombre, real, esperado))

        # -- 1.3 A y B no se han movido: el mando cuelga de ellos --
        for nombre, esperado in sorted(MANDO_ESPERADO.items()):
            real = _pin(fw, punta, nombre)
            mapa[(punta, nombre)] = real
            b.verificar(
                real == esperado,
                "%s: %s sigue en %s - el camino del mando no se toco" % (punta, nombre, real),
                "%s: %s apunta a %s y deberia seguir en %s. Mover A o B mueve las "
                "secuencias del mando de reles, que es lo unico que le queda al operario "
                "que esta en el suelo sin telefono" % (punta, nombre, real, esperado))

    # =============================================================================
    # 2. EL MODO DEL PIN: INPUT PELADO PARA LA CAMARA, PULLUP PARA EL BOTON
    # =============================================================================
    modos_camara = {}
    for punta in PUNTAS:
        fuentes = _fuentes(fw, punta)
        hallados = {}
        for cam in CAMARAS:
            for c in fuentes.values():
                for m in re.finditer(r"pinMode\s*\(\s*%s\s*,\s*(\w+)\s*\)" % cam, c):
                    hallados.setdefault(cam, []).append(m.group(1))
        modos_camara[punta] = hallados

        faltan = [c for c in CAMARAS if c not in hallados]
        b.verificar(
            not faltan,
            "%s: las tres entradas de camara (%s) se declaran" % (punta, ", ".join(CAMARAS)),
            "%s: %s no tiene ningun pinMode en ningun .cpp. Un pin de entrada sin "
            "declarar arranca en el modo por defecto del micro, que no es el que la "
            "placa necesita" % (punta, ", ".join(faltan)))

        malos = {c: ms for c, ms in hallados.items() if any(m != "INPUT" for m in ms)}
        b.verificar(
            hallados and not malos,
            "%s: las %d declaraciones de camara son INPUT a secas - el reposo lo fija el "
            "pull-down de 10K de la placa"
            % (punta, sum(len(v) for v in hallados.values())),
            "%s: %s se declara(n) asi: %s. Con INPUT_PULLUP el pin queda en 0,66 V contra "
            "el pull-down de la placa: demanda permanente sin camara conectada, e "
            "invertida al cerrarla. Es N-67 exacto" % (punta, ", ".join(malos), malos))

        modos_boton = {}
        for bt in sorted(MANDO_ESPERADO):
            for c in fuentes.values():
                for m in re.finditer(r"pinMode\s*\(\s*%s\s*,\s*(\w+)\s*\)" % bt, c):
                    modos_boton.setdefault(bt, []).append(m.group(1))
        # N-118 - ESTA COMPROBACION SE INVIRTIO EL 04/09, Y EL PORQUE IMPORTA MAS QUE EL
        # CAMBIO. Exigia INPUT_PULLUP con este motivo: "SFTY-21 depende de que A y B se
        # lean igual que siempre". Era falso, y de la peor clase: una prueba que EXIGIA el
        # defecto, con una razon que sonaba a seguridad.
        #
        # Lo que la tumbo, medido: R65/R66 son 10K A MASA sobre /Boton1 y /Boton2 -las
        # mismas que R67/R68 sobre C y D-, y J16 reparte 3,3 V en p4 y p7, las posiciones
        # de al lado. Es EXACTAMENTE la cuenta que la cabecera de este pack ya hacia bien
        # para las camaras; lo unico que pasaba es que no se aplicaba a A y B. El banco
        # del 03/09 midio 9,92 kOhm y 0,6 V en p5/p8: el pin estaba clavado en BAJO, nunca
        # habia flanco, y el mando NO SE PODIA PULSAR. SFTY-21 no dependia de esto: estaba
        # MUERTO por esto.
        b.verificar(
            sorted(modos_boton) == sorted(MANDO_ESPERADO)
            and all(m == "INPUT" for ms in modos_boton.values() for m in ms),
            "%s: A y B en INPUT pelado, como C y D - los cuatro pines de J16 son "
            "electricamente identicos y el reposo lo fija el pull-down de 10K" % punta,
            "%s: los pines del mando se declaran %s. Con INPUT_PULLUP el pull-up interno "
            "contra los 10K de R65/R66 deja el pin en 0,6 V -medido en banco el 03/09-, "
            "que el micro lee LOW en permanencia: sin flanco no hay secuencia, y SFTY-21 "
            "se queda sin respaldo fisico" % (punta, modos_boton or "(no se hallan)"))

        # -- 2.bis NINGUN PIN DE CAMARA ENTRA POR EL CAMINO DE BOTON --
        codigo_botones = fw.codigo(punta, "src", "botones.cpp")
        b.verificar(
            not re.search(r"\.pin\s*=\s*CAM_", codigo_botones),
            "%s: ninguna camara se asigna a una estructura Boton: no entra por el "
            "antirrebote de flanco de bajada" % punta,
            "%s: hay una camara asignada a un Boton. Ese camino lee activo en BAJO y "
            "cuenta flancos de bajada: aplicado a una entrada activa en ALTO detecta la "
            "deteccion cuando el coche SE VA" % punta)

    # -- 2.ter Y EL CONJUNTO ES EL MISMO EN LAS DOS PUNTAS (esto es N-97) --
    b.verificar(
        sorted(modos_camara["Maestro"]) == sorted(modos_camara["Esclavo"])
        == sorted(CAMARAS),
        "las dos puntas configuran EXACTAMENTE las mismas tres entradas de camara: N-97 "
        "cerrado por el lado de la configuracion",
        "las dos puntas NO configuran las mismas camaras.\n"
        "        Maestro: %s\n        Esclavo: %s\n"
        "        Es N-97: los documentos hablan de 'las camaras' como si fueran la misma "
        "entrada en los dos equipos, y no lo son"
        % (sorted(modos_camara["Maestro"]), sorted(modos_camara["Esclavo"])))

    # =============================================================================
    # 3. N-97: LA CAMARA NO ES PROPIEDAD DE UN MODO
    # =============================================================================
    # El defecto original: pinMode(CAM_DEMANDA_PIN, INPUT) vivia DENTRO de
    # modoInteligente_setup(). Un pin de entrada declarado por el modo que lo usa solo
    # existe mientras ese modo esta puesto; la otra punta lo declaraba siempre.
    ARRANQUE = ("botones_setup", "setup")
    for punta in PUNTAS:
        fuentes = _fuentes(fw, punta)
        fuera_de_sitio = []
        for f, c in fuentes.items():
            funciones = _funciones(c)
            for cam in CAMARAS:
                for m in re.finditer(r"pinMode\s*\(\s*%s\s*," % cam, c):
                    quien = _quien_contiene(funciones, m.start())
                    if quien not in ARRANQUE:
                        fuera_de_sitio.append("%s -> %s() en %s" % (cam, quien, f))
        b.verificar(
            not fuera_de_sitio,
            "%s: las camaras se declaran solo desde el arranque (%s), nunca desde un modo"
            % (punta, " o ".join(ARRANQUE)),
            "%s: %s. Una entrada fisica existe desde que la tarjeta arranca, la mire "
            "quien la mire. Declararla dentro de un modo es N-97: en el Maestro el pin "
            "solo estaba configurado en Modo Inteligente mientras el Esclavo lo tenia "
            "siempre" % (punta, "; ".join(fuera_de_sitio)))

        # Y el camino de arranque tiene que estar de verdad enganchado a setup().
        main = fw.codigo(punta, "src", "main.cpp")
        cuerpo_setup = _cuerpo(main, "setup")
        b.verificar(
            cuerpo_setup is not None and "botones_setup()" in cuerpo_setup,
            "%s: setup() de main.cpp llama a botones_setup(), asi que lo que se declara "
            "alli se declara en el arranque de verdad" % punta,
            "%s: setup() NO llama a botones_setup(). Entonces los pinMode de botones.cpp "
            "no corren nunca, y la comprobacion de arriba estaria aprobando codigo "
            "muerto" % punta)

    # =============================================================================
    # 4. LA POLARIDAD, MEDIDA SOBRE CADA LECTURA Y CLASIFICADA POR SU FUNCION
    # =============================================================================
    for punta in PUNTAS:
        fuentes = _fuentes(fw, punta)

        # 4.1 TODAS las lecturas de camara de la punta, por nombre o por parametro.
        #
        # Se cuentan las dos clases juntas a proposito. Las nombradas
        # -digitalRead(CAM_DEMANDA_PIN)- solo existen en el Esclavo, porque el Maestro lee
        # su camara de PB0 por camara_leerPin() como las de J16. Preguntar solo por las
        # nombradas daria una lista VACIA en el Maestro, y una lista vacia con un all()
        # delante es la prueba muerta de N-51: PASS sin haber mirado nada.
        codigo_botones = fw.codigo(punta, "src", "botones.cpp")
        funciones = _funciones(codigo_botones)

        contra = []
        for f, c in fuentes.items():
            for cam in CAMARAS:
                for m in re.finditer(
                        r"digitalRead\s*\(\s*%s\s*\)\s*==\s*(\w+)" % cam, c):
                    contra.append(("%s: %s" % (f, cam), m.group(1)))
        for m in re.finditer(r"digitalRead\s*\([^)]*\)\s*==\s*(\w+)", codigo_botones):
            if _quien_contiene(funciones, m.start()) == "camara_leerPin":
                contra.append(("botones.cpp: camara_leerPin()", m.group(1)))

        b.verificar(
            len(contra) >= 2 and all(v == "HIGH" for _, v in contra),
            "%s: las %d lecturas de camara -%d por nombre, %d por el lector compartido- "
            "comparan contra HIGH"
            % (punta, len(contra),
               sum(1 for n, _ in contra if "camara_leerPin" not in n),
               sum(1 for n, _ in contra if "camara_leerPin" in n)),
            "%s: %s. El contacto cierra a 3,3 V; leer LOW invierte la deteccion y da "
            "demanda permanente en reposo"
            % (punta,
               "hay lecturas de camara contra otra cosa: %s"
               % [x for x in contra if x[1] != "HIGH"] if contra
               else "NO SE HALLO NI UNA lectura de camara: fallo el buscador, no el "
                    "firmware, y un censo ciego aprobaria por no encontrar nada"))

        # 4.2 Y el lector compartido, aparte, porque es el unico de las dos de J16.
        por_funcion = {}
        for m in re.finditer(r"digitalRead\s*\([^)]*\)\s*==\s*(\w+)", codigo_botones):
            quien = _quien_contiene(funciones, m.start())
            por_funcion.setdefault(quien, set()).add(m.group(1))

        b.verificar(
            por_funcion.get("camara_leerPin") == {"HIGH"},
            "%s: camara_leerPin() lee ACTIVO EN ALTO y no lee de ninguna otra forma" % punta,
            "%s: camara_leerPin() compara contra %s. Es el unico lector de las dos "
            "camaras de J16: si lee LOW, las dos entradas quedan invertidas a la vez"
            % (punta, por_funcion.get("camara_leerPin", "(no se encuentra la funcion)")))

        botoneras = {q: v for q, v in por_funcion.items()
                     if q in ("actualizar", "botones_setup")}
        # N-118, la otra mitad. Decia "A y B van contra masa", y el cobre dice que no: hay
        # UNA SOLA masa en todo J16 (p2). Un contacto por boton contra masa necesitaria
        # una masa por boton. Lo que el conector reparte es 3,3 V, uno por boton.
        #
        # SE EXIGEN LAS DOS FUNCIONES A LA VEZ -actualizar() y botones_setup()- y no una
        # cualquiera: si la siembra del arranque se quedara en LOW con la lectura en ALTO,
        # un boton suelto se sembraria como "pulsado" y la guarda de N-26 se comeria la
        # PRIMERA pulsacion buena. Medir solo una de las dos dejaria pasar ese caso.
        b.verificar(
            botoneras and all(v == {"HIGH"} for v in botoneras.values()),
            "%s: el camino del boton lee activo en ALTO en %s - la misma polaridad que "
            "las camaras, que es la que pide el conector"
            % (punta, ", ".join(sorted(botoneras))),
            "%s: el camino del boton lee %s. Con los 10K a masa de R65/R66 y los 3,3 V en "
            "el pin de al lado, leer en BAJO deja el mando pulsado en permanencia y sin "
            "un solo flanco" % (punta, botoneras or "(nada)"))

    # =============================================================================
    # 5. LAS DOS PUNTAS LEEN LAS CAMARAS DE J16 CON EL MISMO CODIGO (N-97)
    # =============================================================================
    cuerpos = {}
    for nombre in CAMINO_CAMARA:
        for punta in PUNTAS:
            c = _cuerpo(fw.codigo(punta, "src", "botones.cpp"), nombre)
            cuerpos[(punta, nombre)] = c
            if c is None:
                raise fw.Abortado(
                    "no se encuentra %s() en %s/src/botones.cpp. O se renombro, o el "
                    "patron se quedo ciego; en los dos casos este pack no puede medir el "
                    "camino de camara y su PASS no valdria nada" % (nombre, punta))

    for nombre in CAMINO_CAMARA:
        m, e = cuerpos[("Maestro", nombre)], cuerpos[("Esclavo", nombre)]
        b.verificar(
            m == e,
            "%s() es identica en las dos puntas (%d caracteres de codigo sin comentarios)"
            % (nombre, len(m)),
            "EL CAMINO DE CAMARA DIVERGE en %s(). Es N-97 volviendo: dos lecturas "
            "parecidas de la misma entrada fisica, que solo la disciplina mantiene "
            "iguales.\n        Maestro: %s\n        Esclavo: %s"
            % (nombre, m[:170], e[:170]))

    # -- 5.bis El antirrebote es el mismo numero, leido del C++ de cada punta --
    rebotes = {}
    for punta in PUNTAS:
        m = re.search(r"delay\s*\(\s*(\d+)\s*\)", cuerpos[(punta, "camara_leerPin")])
        if not m:
            raise fw.Abortado(
                "no se pudo leer del C++ el antirrebote de camara_leerPin() en el %s. Sin "
                "ese numero el pack no sabe si hay antirrebote o no, y un banco que no "
                "puede fallar no demuestra nada" % punta)
        rebotes[punta] = int(m.group(1))
    b.verificar(
        rebotes["Maestro"] == rebotes["Esclavo"] and rebotes["Maestro"] > 0,
        "el antirrebote de camara es %d ms en las dos puntas. En J16 es el UNICO que hay: "
        "PB0 lleva el RC de R64+C25 que filtra 1 ms, y PB14/PB15 no llevan mas que el 10K"
        % rebotes["Maestro"],
        "el antirrebote de camara no coincide entre puntas: %s. Un rele que rebota se lee "
        "como dos coches en una punta y como uno en la otra" % rebotes)

    # =============================================================================
    # 6. LA DEMANDA SE TOMA POR FLANCO Y SALE POR LA UNICA PUERTA
    # =============================================================================
    for punta in PUNTAS:
        cuerpo = cuerpos[(punta, "camaras_actualizar")]

        b.verificar(
            "camAnt" in cuerpo and "!camAnt" in cuerpo.replace(" ", ""),
            "%s: la demanda de J16 se toma por FLANCO, comparando contra el nivel "
            "anterior" % punta,
            "%s: camaras_actualizar() no compara contra el nivel anterior. El rele "
            "mantiene el contacto ~1 s: sin flanco, ese segundo son cientos de vueltas "
            "del loop pidiendo lo mismo" % punta)

        # N-26 APLICADO A LAS CAMARAS. Un contacto ya cerrado al encender no es una
        # deteccion: es un estado. Sin sembrar el nivel anterior, la primera vuelta del
        # loop ve nivel alto contra un anterior en false -la definicion de un flanco- y
        # pide paso sin que haya pasado ningun coche. Es el mismo agujero que el ACEPTAR
        # fantasma que se confirmo en banco el 01/08, en la entrada de al lado.
        siembra = cuerpos[(punta, "camaras_sembrar")]
        cuerpo_setup_bot = _cuerpo(fw.codigo(punta, "src", "botones.cpp"), "botones_setup")
        b.verificar(
            "camAnt[i] = camara_leerPin" in siembra.replace(" ", " ")
            and cuerpo_setup_bot is not None
            and "camaras_sembrar()" in cuerpo_setup_bot,
            "%s: botones_setup() SIEMBRA el nivel real de las dos camaras (N-26 aplicado a "
            "J16): un contacto ya cerrado al encender no genera un flanco fantasma" % punta,
            "%s: el nivel anterior de las camaras no se siembra en el arranque, o "
            "botones_setup() no llama a camaras_sembrar(). Con camAnt[] en false y el "
            "contacto ya cerrado -rele en reposo cerrado, contacto trabado, ruido en el "
            "cable-, la primera vuelta del loop pide paso sin que haya pasado nadie"
            % punta)

        b.verificar(
            "demanda_solicitar()" in cuerpo,
            "%s: la camara pide por demanda_solicitar(), la misma puerta que el boton de "
            "la app: un solo limite de ritmo para los dos origenes" % punta,
            "%s: camaras_actualizar() no llama a demanda_solicitar(). Un atajo hasta el "
            "coordinador se saltaria la ventana de silencio, el minimo de verde y el tope "
            "de verde maximo" % punta)

        # La barrera de salidas, aplicada al camino nuevo (CLAUDE.md 6).
        ordena = [n for n in ("coordinador_pedirCambio", "semaforo_forzarVerde",
                              "semaforo_forzarRojo", "semaforo_iniciarFallo",
                              "digitalWrite")
                  if n + "(" in cuerpo]
        b.verificar(
            not ordena,
            "%s: el camino de camara PIDE y no ordena: no llama a nada que mueva luces"
            % punta,
            "%s: camaras_actualizar() llama a %s. Una camara no decide: pide. Ordenar "
            "desde aqui se salta el todo-rojo y el minimo de verde"
            % (punta, ", ".join(ordena)))

    # -- 6.bis Ni un pin de luz asoma por botones.cpp --
    for punta in PUNTAS:
        codigo_botones = fw.codigo(punta, "src", "botones.cpp")
        pines_txt = fw.texto(punta, "include", "pines.h")
        faltan = [n for n in PINES_DE_LUZ
                  if not re.search(r"#define\s+%s\s+P[A-Z]\d+" % n, pines_txt)]
        if faltan:
            raise fw.Abortado(
                "no se encuentran en %s/include/pines.h los pines de luz %s. Sin la lista "
                "resuelta, 'ningun pin de luz asoma por botones.cpp' compararia contra "
                "nada y saldria verde" % (punta, ", ".join(faltan)))
        asomados = [n for n in PINES_DE_LUZ if re.search(r"\b%s\b" % n, codigo_botones)]
        b.verificar(
            not asomados and "digitalWrite" not in codigo_botones,
            "%s: botones.cpp no nombra ninguno de los ocho pines de luz ni escribe ningun "
            "pin: la barrera de salidas sigue siendo de semaforo.cpp" % punta,
            "%s: botones.cpp toca %s. Solo semaforo.cpp escribe pines de luz, y el "
            "conector de la botonera es el ultimo sitio desde el que deberia hacerse"
            % (punta, ", ".join(asomados) or "digitalWrite"))

    # -- 6.ter La ventana de silencio: leida del C++ de cada punta, y la misma --
    silencios = {}
    for punta in PUNTAS:
        silencios[punta] = fw.constante(
            (punta, "src", "demanda.cpp"), r"SILENCIO_MS\s*=\s*(\d+)",
            "la ventana de silencio entre demandas del %s" % punta)
    b.verificar(
        silencios["Maestro"] == silencios["Esclavo"],
        "la ventana de silencio entre demandas es la misma en las dos puntas (%d ms)"
        % silencios["Maestro"],
        "la ventana de silencio no coincide: %s. Con dos camaras nuevas por punta, dos "
        "limites distintos hacen que la misma cola de coches se lea a ritmos distintos en "
        "cada extremo" % silencios)
    b.verificar(
        min(silencios.values()) > PULSO_RELE_MS,
        "la ventana de silencio (%d ms) supera el pulso del rele (%d ms): dos camaras en "
        "el mismo conector no convierten una deteccion en dos peticiones"
        % (silencios["Maestro"], PULSO_RELE_MS),
        "la ventana (%s) NO supera el pulso del rele (%d ms). Con C y D en el mismo "
        "conector, un mismo vehiculo puede cerrar los dos contactos"
        % (silencios, PULSO_RELE_MS))

    # =============================================================================
    # 7. LO QUE SE RETIRO, Y LO QUE TIENE QUE SEGUIR EN PIE PARA PODER RETIRARLO
    # =============================================================================
    for punta in PUNTAS:
        codigo_botones = fw.codigo(punta, "src", "botones.cpp")

        b.verificar(
            re.search(r"bool\s+botonAceptar\s*\(\s*\)\s*\{\s*return\s+false\s*;", codigo_botones)
            and re.search(r"bool\s+botonCancelar\s*\(\s*\)\s*\{\s*return\s+false\s*;", codigo_botones),
            "%s: botonAceptar() y botonCancelar() estan SIN SUJETO y lo dicen devolviendo "
            "false: no queda pin que pueda levantarlas" % punta,
            "%s: botonAceptar()/botonCancelar() vuelven a consumir un flanco. Sus pines "
            "son camaras: si algo se lo puede levantar, es que hay un camino que sigue "
            "tratando PB14/PB15 como pulsadores" % punta)

        cuerpo_act = _cuerpo(codigo_botones, "botones_actualizar")
        tablas = dict(re.findall(r"static\s+bool\s+(flanco|disparadoAnt)\[(\d+)\]",
                                 codigo_botones))
        b.verificar(
            tablas.get("flanco") == "2" and tablas.get("disparadoAnt") == "2"
            and cuerpo_act is not None
            and not re.search(r"flanco\[[23]\]", cuerpo_act)
            and not re.search(r"consumir\s*\(\s*[23]\s*\)", codigo_botones),
            "%s: las tablas de flancos son de DOS y nadie escribe ni consume un tercer "
            "hueco: no queda estado reservado a dos botones que ya no existen" % punta,
            "%s: las tablas son %s y/o queda alguien tocando el hueco 2 o 3. Estado que "
            "nadie puede llenar es estado que alguien acabara llenando por error - y en "
            "este fichero el hueco 2 era el boton que EJECUTABA"
            % (punta, tablas or "(no se hallan)"))

        b.verificar(
            cuerpo_act is not None
            and re.search(r"if\s*\(\s*flanco\[0\]\s*\)\s*mando_registrarPulso\s*\(\s*MANDO_A",
                          cuerpo_act)
            and re.search(r"if\s*\(\s*flanco\[1\]\s*\)\s*mando_registrarPulso\s*\(\s*MANDO_B",
                          cuerpo_act),
            "%s: A y B siguen alimentando mando_registrarPulso() desde botones_actualizar()"
            % punta,
            "%s: el mando dejo de recibir los pulsos de A y B. Es SFTY-21: las secuencias "
            "tienen que verse SIEMPRE, y con C y D retirados el mando es lo unico que le "
            "queda al operario que esta en el suelo sin telefono" % punta)

    # -- 7.bis LOS SUSTITUTOS: sin ellos, esta retirada no se puede hacer --
    bt_maestro = fw.codigo("Maestro", "src", "bluetooth.cpp")
    faltan = [c for c in SUSTITUTOS_MAESTRO if '"%s' % c not in bt_maestro]
    b.verificar(
        not faltan,
        "Maestro: los %d comandos que sustituyen a ACEPTAR y CANCELAR estan en el "
        "despachador de Bluetooth" % len(SUSTITUTOS_MAESTRO),
        "Maestro: FALTA(N) %s en bluetooth.cpp. botonCancelar() era la unica salida de "
        "los ocho modos: retirarla sin el comando que la sustituye deja al operario "
        "dentro de un modo sin mas salida que cortar la energia" % ", ".join(faltan))

    # El Esclavo NO tiene SET_MODO por Bluetooth. Su sustituto no es la app: es el mando
    # de reles, que sigue entero sobre A y B. Si el mando dejara de alcanzar el Degradado,
    # esta punta se quedaria sin ninguna forma de entrar ni de salir.
    mando_esc = fw.codigo("Esclavo", "src", "mando.cpp")
    for fn, que in (("degradado_entrar", "ENTRAR al Modo Degradado"),
                    ("degradado_salir", "SALIR del Modo Degradado")):
        b.verificar(
            re.search(r"\b%s\s*\(" % fn, mando_esc) is not None,
            "Esclavo: el mando de reles sigue pudiendo %s (%s() en mando.cpp)" % (que, fn),
            "Esclavo: el mando ya no llama a %s(). Esta punta NO tiene SET_MODO por "
            "Bluetooth -sus comandos son AMBAR_EMERGENCIA, FORZAR_ROJO, SOLICITAR_PASO, "
            "TEST_LEDS y SET_RTC-, asi que con los botones 3 y 4 retirados el mando es su "
            "UNICA via para %s" % (fn, que))

    # Y el veto de SFTY-21 sigue teniendo quien lo arme: mando_ambarLocal() vale true
    # porque A y B siguen llegando al mando. Retirar el armador de una bandera deja
    # abiertos los if que la usan para vetar, y eso casi nunca es "nada".
    consumidores = sum(
        len(re.findall(r"!\s*mando_ambarLocal\s*\(", fw.codigo("Esclavo", "src", f)))
        for f in fw.fuentes_de("Esclavo", "src"))
    b.verificar(
        consumidores >= 3,
        "Esclavo: los %d vetos de mando_ambarLocal() siguen en pie, y su bandera sigue "
        "pudiendo ser cierta porque A y B no se tocaron (SFTY-21)" % consumidores,
        "Esclavo: quedan %d vetos de mando_ambarLocal() y eran tres. Mientras un operario "
        "pidio ambar local, una orden de radio NO saca a esta punta del ambar: ese es el "
        "veto, y desaparece en silencio si se rompe su camino" % consumidores)

    # =============================================================================
    # 8. CONTROLES NEGATIVOS
    # =============================================================================
    b.control_negativo(
        re.findall(r"pinMode\s*\(\s*CAM_C_PIN\s*,\s*(\w+)\s*\)",
                   "void f(){ pinMode(CAM_C_PIN, INPUT_PULLUP); }") == ["INPUT_PULLUP"],
        "el lector de modos ve un INPUT_PULLUP colado en una entrada de camara")

    b.control_negativo(
        re.findall(r"digitalRead\s*\(\s*CAM_D_PIN\s*\)\s*==\s*(\w+)",
                   "if (digitalRead(CAM_D_PIN) == LOW) {}") == ["LOW"],
        "y el de polaridad ve una camara leida al reves")

    _mut = ("bool camara_leerPin(uint8_t pin) { if (digitalRead(pin) == HIGH) "
            "{ delay(5); return true; } return false; }\n"
            "static void camaras_actualizar() { demanda_solicitar(); }\n")
    b.control_negativo(
        _cuerpo(_mut, "camara_leerPin") != cuerpos[("Maestro", "camara_leerPin")],
        "el comparador entre puntas distingue dos cuerpos que no son el mismo")

    b.control_negativo(
        _quien_contiene(_funciones(
            "void modoInteligente_setup() { pinMode(CAM_DEMANDA_PIN, INPUT); }"),
            len("void modoInteligente_setup() { ")) == "modoInteligente_setup",
        "el clasificador atribuye un pinMode de camara a la funcion que lo contiene, que "
        "es como se detecta N-97 volviendo a meterse dentro de un modo")

    b.control_negativo(
        re.search(r"bool\s+botonAceptar\s*\(\s*\)\s*\{\s*return\s+false\s*;",
                  "bool botonAceptar() { return consumir(2); }") is None,
        "el detector de 'sin sujeto' NO acepta un botonAceptar() que sigue consumiendo un "
        "flanco")

    b.control_negativo(
        '"SET_MODO:MENU' not in 'if (strcmp(accion, "SET_MODO:AUTO") == 0) {}',
        "el censo de sustitutos no da por presente un comando que no esta")

    b.control_negativo(
        dict(re.findall(r"static\s+bool\s+(flanco|disparadoAnt)\[(\d+)\]",
                        "static bool flanco[4] = {0,0,0,0};")).get("flanco") == "4",
        "el lector de tablas ve una tabla de flancos que se quedo con cuatro huecos")

    b.control_negativo(
        "coordinador_pedirCambio(" in
        "static void camaras_actualizar(){ coordinador_pedirCambio(); }",
        "y el censo de la barrera ve una camara que ordena en vez de pedir")

    # ---------------------------------------------------------------------------------
    # LO QUE NO SE PUEDE MEDIR DESDE AQUI, Y NO CUENTA COMO COMPROBACION
    b.reportar(
        "Verde aqui NO autoriza a cablear camara a J16",
        ["La contradiccion de polaridad entre el netlist y el fuente sigue abierta para",
         "los cuatro pines (05_Funcional/17_Arquitectura...:2.2). Este pack comprueba que",
         "el firmware es coherente con lo que dice el NETLIST; que el netlist describa la",
         "PLACA SOLDADA lo decide la medida M3 con ohmimetro, y no un pack.",
         "Y J16 p1 lleva 12 V crudos sin opto ni clamp a nueve posiciones de p10: se tapa",
         "fisicamente antes de enchufar nada (17_...:2.1).",
         "El orden es asimetrico y solo un sentido es seguro: el firmware nuevo tiene que",
         "estar CARGADO EN LA TARJETA antes de que nadie enchufe un hilo, porque con el",
         "viejo dentro PB14 sigue siendo botonAceptar() activo en BAJO."])

    b.reportar(
        "La asimetria que QUEDA entre puntas, y por que es correcta",
        ["Las camaras C y D se leen igual en los dos equipos -este pack lo exige-, pero la",
         "camara de PB0 no: el Maestro la lee POR NIVEL y el Esclavo POR FLANCO.",
         "No es descuido, es SFTY-27 -el Esclavo PIDE y el Maestro DECIDE-. La regla del",
         "Maestro es 'cede el paso si el otro pide Y NO hay cola local', y 'hay cola AHORA'",
         "es un nivel: un flanco no puede contestar a eso. La lectura por flanco del",
         "Esclavo vive en Esclavo/src/main.cpp:350 y no en botones.cpp."])
