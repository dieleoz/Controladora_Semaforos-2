# ===== banco/packs/costura_12_margen_deriva.py =====
#
# EL MARGEN DEL MODO DEGRADADO CONTRA LA DERIVA ENTRE LOS DOS RELOJES.
#
# El Modo Degradado es el modo que se usa cuando la radio MUERE. Ahi el verde de cada
# punta sale de SU PROPIO RELOJ y no hay nadie que coordine: es el unico modo del
# equipo en el que un choque frontal no lo impide un enclavamiento, sino una
# DESIGUALDAD NUMERICA:
#
#     despeje ampliado  >  deriva acumulada durante el limite duro
#
# Esa desigualdad vivia SOLO EN PROSA, dentro de un comentario de los dos
# modo_degradado.cpp, con las cuentas ya hechas: "~8,6 s al dia", "~3,5 dias de margen
# teorico", "48 h deja un factor de seguridad de 2". Es exactamente la forma de N-71:
# un comentario no falla cuando alguien cambia un numero, se queda describiendo un
# equipo que ya no existe Y con la autoridad de una cuenta hecha.
#
# Este pack la recalcula desde el C++ en cada corrida. Y su hermano en C++
# -Validacion_Automatico/compilar_degradado.ps1- la MIDE ejecutando el firmware real de
# las dos puntas con relojes independientes. Los dos hacen falta y no se solapan:
#
#   este pack   relaciona CONSTANTES. Corre en un segundo, en cualquier maquina, y
#               falla si alguien sube el limite duro o baja el despeje.
#   el arnes    ejecuta el C++ y mira los pines. Es el que dice si el firmware hace lo
#               que la constante promete.
#
# EJERCE SFTY-21: que el colchon del ciclo degradado cubra la deriva que el limite duro
# permite acumular, recalculado desde el C++ y en LOS DOS SENTIDOS de la deriva.

import os
import re

from banco.modelos.costura import *          # noqa: F401,F403

NOMBRE = "costura_12_margen_deriva"
DESCRIPCION = "el despeje del Degradado contra la deriva que el limite de 48 h permite"

ARNES_GUION = ("Validacion_Automatico", "compilar_degradado.ps1")
ARNES_DIR = ("Validacion_Automatico", "dos_puntas")
ARNES_ORQ = "orquestador_degradado.cpp"

# El ciclo se repite cada CICLO segundos, asi que para saber si dos puntas desfasadas
# llegan a solaparse basta recorrer UN ciclo lejos de medianoche. Barrer las 86.400
# posiciones por cada uno de los 120 desfases seria diez millones de vueltas para
# responder lo mismo. La guarda de medianoche -que fuerza todo-rojo- la mide
# costura_02; aqui se evita a proposito, porque ahi NINGUNA punta puede dar verde y el
# barrido no encontraria nunca el limite que busca.
SEGUNDO_DE_REFERENCIA = 3600


def _lee(fw, *partes):
    p = os.path.join(fw.FIRMWARE, *partes)
    if not os.path.isfile(p):
        raise fw.Abortado("no existe %s. Sin el, este pack no puede medir si el margen "
                          "del Degradado se sigue vigilando" % os.path.join(*partes))
    with open(p, "r", encoding="utf-8", errors="replace") as f:
        return f.read()


def _primer_solape(sentido, verde, despeje, amarillo_s):
    """Primer desfase, en segundos, con las DOS puntas en verde a la vez.

    sentido = +1 Esclavo adelantado, -1 Esclavo atrasado.

    Usa luz_maestro() y luz_esclavo() de banco/modelos/costura.py TAL CUAL: son las
    mismas dos funciones que costura_02 contrasta linea a linea contra el C++ de
    ciclo_degradado.h. Escribir aqui una tercera version del ciclo seria la segunda
    copia del firmware a mano que este banco lleva un mes retirando.
    """
    ciclo = 2 * (verde + despeje)
    for skew in range(1, ciclo):
        for s in range(SEGUNDO_DE_REFERENCIA, SEGUNDO_DE_REFERENCIA + ciclo):
            otro = (s + sentido * skew) % SEGUNDOS_DEL_DIA
            if (luz_maestro(s, verde, despeje) == VERDE and
                    luz_esclavo(otro, verde, despeje, amarillo_s) == VERDE):
                return skew
    return None


def correr(b, fw):
    b.titulo("EL MARGEN DEL DEGRADADO CONTRA LA DERIVA ENTRE LOS DOS RELOJES")

    # =======================================================================
    # 1. LA DERIVA DECLARADA, QUE NO ES UNA CONSTANTE SINO UNA FRASE
    # =======================================================================
    #
    # Toda la cuenta se apoya en un numero -los segundos que dos cristales de
    # 32.768 kHz sin calibrar se separan al dia- que NO existe como constante en
    # ningun sitio del firmware: vive en un comentario, escrito dos veces con dos
    # redacciones distintas. Se lee de las DOS y se exige que digan lo mismo: dos
    # puntas que declararan derivas distintas estarian describiendo dos equipos.
    PAT_DERIVA = r"(\d+),(\d+) s(?:/| al )dia"
    m_der = re.search(PAT_DERIVA, T_M_DEG_C)
    e_der = re.search(PAT_DERIVA, T_E_DEG_C)
    if not m_der or not e_der:
        raise fw.Abortado(
            "no se encuentra la deriva diaria declarada en los comentarios de "
            "modo_degradado.cpp de alguna de las dos puntas. TODA la cuenta de este "
            "pack se apoya en ese numero: sin el no se puede medir, y suponerlo seria "
            "fabricar un PASS. Si la frase cambio, actualiza el patron; si el numero "
            "desaparecio, la desigualdad de SFTY-21 se quedo sin base escrita")

    deriva_m = int(m_der.group(1)) + int(m_der.group(2)) / 10.0
    deriva_e = int(e_der.group(1)) + int(e_der.group(2)) / 10.0

    b.verificar(
        deriva_m == deriva_e,
        f"las dos puntas declaran la MISMA deriva entre cristales ({deriva_m:.1f} s/dia), "
        "leida por separado del comentario de cada modo_degradado.cpp",
        f"cada punta declara una deriva distinta: Maestro {deriva_m:.1f} s/dia, Esclavo "
        f"{deriva_e:.1f} s/dia. La desigualdad que sostiene SFTY-21 no puede tener dos "
        "valores segun el fichero que se abra")

    DERIVA_DIA = deriva_m
    HORAS_LIMITE = M_LIMITE_DURO_MS / 3600000.0
    DERIVA_LIMITE = DERIVA_DIA * HORAS_LIMITE / 24.0

    b.reportar(
        "la deriva de la que cuelga SFTY-21 no es una constante: es una frase",
        [f"{DERIVA_DIA:.1f} s/dia esta escrito en un COMENTARIO de "
         "Maestro/src/modo_degradado.cpp y de Esclavo/src/modo_degradado.cpp, no en un "
         "static const que alguien pueda releer.",
         "Este pack lo lee del comentario y ABORTA si la frase cambia, que es lo maximo "
         "que se puede hacer sin tocar el firmware. Pero un comentario no falla cuando "
         "alguien cambia el cristal, el proveedor o el rango de temperatura: se queda "
         "describiendo un equipo que ya no existe (N-71).",
         "EN LA CALLE: si el cristal real derivara mas de lo que dice la frase, nada en "
         "el banco lo notaria y el todo-rojo dejaria de cubrir las 48 h."])

    # =======================================================================
    # 2. EL DESFASE QUE EL CRUCE AGUANTA, EN LOS DOS SENTIDOS
    # =======================================================================
    #
    # Cual de los dos cristales adelanta es un accidente del silicio, no una eleccion
    # del diseno, asi que el margen del equipo es EL PEOR DE LOS DOS SENTIDOS.
    AMARILLO_S = E_AMARILLO_MS // 1000
    adelantado = _primer_solape(+1, DEG_VERDE_SEG, DEG_DESPEJE_SEG, AMARILLO_S)
    atrasado = _primer_solape(-1, DEG_VERDE_SEG, DEG_DESPEJE_SEG, AMARILLO_S)

    b.verificar(
        adelantado is not None and atrasado is not None,
        f"el barrido encuentra el desfase que rompe el cruce en los DOS sentidos: "
        f"{adelantado} s con el Esclavo adelantado, {atrasado} s con el Esclavo atrasado",
        "el barrido no encuentra solape en algun sentido ni con un ciclo entero de "
        "desfase. Un barrido que no puede encontrar el fallo no mide un margen: mide "
        "una tapia, y su PASS no vale nada")

    if adelantado is None or atrasado is None:
        return

    peor = min(adelantado, atrasado)

    b.verificar(
        peor - 1 == DEG_DESPEJE_SEG,
        f"en el sentido MALO el cruce aguanta exactamente el despeje ampliado "
        f"({DEG_DESPEJE_SEG} s) y rompe al segundo siguiente. El colchon que el diseno "
        "declara es real, y no hay ni un segundo de mas",
        f"el sentido malo aguanta {peor - 1} s y el despeje declarado son "
        f"{DEG_DESPEJE_SEG} s. El colchon del ciclo degradado no es el que dice el "
        "diseno, y toda la cuenta de las 48 h se apoya en el")

    b.verificar(
        adelantado - atrasado == AMARILLO_S,
        f"los dos sentidos NO son simetricos, y la diferencia es exactamente el amarillo "
        f"del Esclavo ({AMARILLO_S} s): esa punta empieza su verde por ambar, asi que el "
        "ambar le come el principio de su verde y protege SOLO cuando va adelantada",
        f"la asimetria entre sentidos ({adelantado - atrasado} s) no coincide con el "
        f"amarillo del Esclavo ({AMARILLO_S} s): la explicacion de por que un sentido "
        "aguanta mas que el otro ya no es la que dice este pack, y el numero publicado "
        "podria salir de otra cosa")

    # --- El hallazgo: el barrido que ya existia solo miraba a un lado --------
    #
    # costura_02_fase_ciclo.py hace este mismo barrido con `for skew in range(...)`, o
    # sea SOLO con el Esclavo adelantado, y publica ese numero como "el margen real
    # contra la deriva entre relojes... el colchon que justifica el limite de 48 h".
    # Es el sentido BUENO, el que el amarillo protege.
    t_costura = _lee(fw, "Simulaciones", "banco", "packs", "costura_02_fase_ciclo.py")
    barrido_una_direccion = bool(
        re.search(r"for\s+skew\s+in\s+range\(0,\s*2\s*\*\s*\(DEG_VERDE_SEG", t_costura))
    b.hallazgo(
        barrido_una_direccion and adelantado > atrasado,
        "el barrido de deriva que ya existia solo mira un sentido, y publica el bueno",
        [f"costura_02_fase_ciclo.py barre `for skew in range(0, ...)` -solo el Esclavo "
         f"adelantado- y llama a esos {adelantado} s 'el margen real contra la deriva "
         f"entre relojes'.",
         f"Con el Esclavo ATRASADO, que es igual de probable, el solape aparece a los "
         f"{atrasado} s: {adelantado - atrasado} s antes.",
         "EN LA CALLE: el numero que sostiene la eleccion de las 48 h estaba tomado del "
         "sentido favorable. Sigue habiendo margen -lo dice la comprobacion de abajo-, "
         "pero es menor del publicado, y quien decidiera alargar el plazo apoyandose en "
         "el numero grande se quedaria corto."])

    # =======================================================================
    # 3. EL RESIDUO SUB-SEGUNDO DE LA SINCRONIZACION
    # =======================================================================
    #
    # El barrido de arriba trabaja con segundos enteros, y las dos puntas NO quedan
    # alineadas al segundo: la hora viaja por radio en SEGUNDOS ENTEROS y el Esclavo la
    # aplica al recibirla, de modo que tras una sincronizacion perfecta queda un residuo
    # de hasta un segundo. Ese residuo empuja la frontera del sentido malo hacia dentro,
    # y hay que restarlo del margen.
    #
    # El arnes en C++ lo MIDE: los solapes de las dos primeras roturas duran 450 ms y
    # 550 ms y suman exactamente 1000 ms, o sea un unico segundo repartido entre los dos
    # lados. Aqui se comprueba la CAUSA, que es estructural y esta en el fuente.
    hora_en_segundos_enteros = bool(
        re.search(r"protocolo_enviarPaquete\(CMD_HORA_S,\s*s\);", T_M_COORD_C))
    aplica_al_recibir = bool(
        re.search(r"reloj_ajustar\(bufHora,\s*bufMinuto,\s*pkt\.param,\s*bufDia\);", T_E_MAIN_C))
    b.verificar(
        hora_en_segundos_enteros and aplica_al_recibir,
        "la hora viaja en SEGUNDOS ENTEROS (CMD_HORA_S lleva reloj_segundo()) y el "
        "Esclavo la aplica al recibirla: tras una sincronizacion perfecta queda un "
        "residuo de hasta 1 s entre los dos relojes, y ese segundo se descuenta del "
        "margen porque nadie lo puede medir con CMD_DELTA",
        "no se reconoce como viaja la hora: sin saber si el transporte es de segundos "
        "enteros o mas fino, el residuo que hay que descontar del margen es una "
        "suposicion, y una suposicion no sostiene una desigualdad de seguridad")
    RESIDUO_SYNC_S = 1

    TOLERADO = peor - 1 - RESIDUO_SYNC_S

    # =======================================================================
    # 4. LA DESIGUALDAD (N-71), RECALCULADA Y NO ESCRITA EN PROSA
    # =======================================================================
    #
    # La deriva que el equipo puede llegar a tener encima tiene DOS sumandos, y el
    # segundo se olvida siempre: la puerta de entrada acepta hasta TOLERANCIA_DESFASE_S
    # de desfase MEDIDO, asi que el modo puede arrancar ya con ese error puesto.
    DERIVA_POSIBLE = DERIVA_LIMITE + M_TOLERANCIA_S

    print(f"    deriva de los cristales en {HORAS_LIMITE:.0f} h : {DERIVA_LIMITE:.1f} s")
    print(f"    error que la puerta admite al entrar   : {M_TOLERANCIA_S} s")
    print(f"    residuo sub-segundo de la sincronizacion: {RESIDUO_SYNC_S} s")
    print(f"    DERIVA POSIBLE TOTAL                   : {DERIVA_POSIBLE:.1f} s")
    print(f"    DESFASE QUE EL CRUCE AGUANTA           : {TOLERADO} s")
    print(f"    MARGEN                                 : {TOLERADO - DERIVA_POSIBLE:.1f} s "
          f"(factor {TOLERADO / DERIVA_POSIBLE:.2f})")

    b.verificar(
        TOLERADO > DERIVA_POSIBLE,
        f"el desfase que el cruce aguanta ({TOLERADO} s) supera todo lo que el equipo "
        f"puede acumular dentro de su limite duro: {DERIVA_LIMITE:.1f} s de deriva entre "
        f"cristales en {HORAS_LIMITE:.0f} h mas los {M_TOLERANCIA_S} s que la propia "
        f"puerta admite al entrar = {DERIVA_POSIBLE:.1f} s. Margen "
        f"{TOLERADO - DERIVA_POSIBLE:.1f} s (factor {TOLERADO / DERIVA_POSIBLE:.2f})",
        f"PELIGRO: el cruce aguanta {TOLERADO} s de desfase y el equipo puede acumular "
        f"{DERIVA_POSIBLE:.1f} s antes de rendirse. Los dos verdes se tocan ANTES de que "
        "el limite duro mande a ambar: en Modo Degradado, con la radio muerta, las dos "
        "puntas darian paso a la vez y nadie lo veria hasta el choque")

    # El limite duro es la variable de la que cuelga todo. Se dice cuanto CABRIA, para
    # que quien quiera alargarlo tenga el numero delante en vez de la frase.
    horas_que_caben = (TOLERADO - M_TOLERANCIA_S) * 24.0 / DERIVA_DIA
    b.verificar(
        horas_que_caben > HORAS_LIMITE,
        f"el limite duro de {HORAS_LIMITE:.0f} h cabe dentro del margen: con este despeje "
        f"y esta deriva se aguantarian hasta {horas_que_caben:.0f} h. Subirlo por encima "
        "de esa cifra sin ampliar el despeje hace FALLAR esta linea, que es justo lo que "
        "un comentario no sabe hacer",
        f"el limite duro son {HORAS_LIMITE:.0f} h y el despeje solo cubre "
        f"{horas_que_caben:.0f} h de deriva. El plazo que el firmware se concede es mas "
        "largo que el colchon que lo justifica")

    # --- Las cuentas que el comentario ya trae hechas ------------------------
    m_dias = re.search(r"margen teorico[^.]*?(\d+),(\d+) dias", T_M_DEG_C)
    e_dias = re.search(r"solapen son ~(\d+),(\d+) dias", T_E_DEG_C)
    dias_declarados = None
    if m_dias:
        dias_declarados = int(m_dias.group(1)) + int(m_dias.group(2)) / 10.0
    dias_reales = (peor - 1) / DERIVA_DIA
    b.verificar(
        dias_declarados is not None and abs(dias_declarados - dias_reales) < 0.1,
        f"la cuenta que el comentario trae hecha -'~{dias_declarados} dias de margen "
        f"teorico'- se recalcula desde el C++ y cuadra: {peor - 1} s de colchon entre "
        f"{DERIVA_DIA:.1f} s/dia = {dias_reales:.2f} dias",
        f"el comentario declara ~{dias_declarados} dias de margen teorico y la cuenta "
        f"desde el C++ da {dias_reales:.2f}. Una cuenta hecha dentro de un comentario "
        "tiene la autoridad de un dato y no se recalcula sola")

    # El factor de seguridad SI se desvia, y se reporta en vez de contarlo: corregirlo
    # es editar una frase de un fichero de firmware, y eso lo decide quien lleva el
    # documento, no este pack.
    factor_declarado = 2.0
    factor_bruto = dias_reales * 24.0 / HORAS_LIMITE
    factor_util = TOLERADO / DERIVA_POSIBLE
    if abs(factor_bruto - factor_declarado) > 0.05:
        b.reportar(
            "el 'factor de seguridad 2' que declaran los dos comentarios no es 2",
            [f"Maestro/src/modo_degradado.cpp y Esclavo/src/modo_degradado.cpp dicen que "
             f"'el limite duro de 48 h deja un factor de seguridad de 2' sobre los "
             f"~{dias_reales:.1f} dias de margen teorico.",
             f"Recalculado: {dias_reales:.2f} dias contra {HORAS_LIMITE / 24:.0f} dias de "
             f"limite = factor {factor_bruto:.2f}, no {factor_declarado:.0f}.",
             f"Y contando lo que de verdad se descuenta -los {M_TOLERANCIA_S} s que la "
             f"puerta admite y el segundo del transporte de la hora- el factor util es "
             f"{factor_util:.2f}.",
             "EN LA CALLE: sigue habiendo margen, pero es la mitad del que la frase "
             "anuncia. Quien decida alargar el plazo apoyandose en ese '2' se estaria "
             "gastando un colchon que no existe. Se cierra corrigiendo la frase o "
             "ampliando el despeje; las dos son decisiones de quien firma el diseno."])

    # =======================================================================
    # 5. CONTROLES NEGATIVOS: que este pack sepa fallar
    # =======================================================================
    #
    # La desigualdad de arriba pasa hoy. Sin esto, pasaria igual el dia que el barrido
    # dejara de encontrar nada o la cuenta se quedara fija, y nadie lo notaria.
    peor_falso = min(_primer_solape(+1, DEG_VERDE_SEG, 5, AMARILLO_S),
                     _primer_solape(-1, DEG_VERDE_SEG, 5, AMARILLO_S))
    tolerado_falso = peor_falso - 1 - RESIDUO_SYNC_S
    b.control_negativo(
        tolerado_falso < DERIVA_POSIBLE,
        f"con un despeje de 5 s en vez de {DEG_DESPEJE_SEG} el mismo barrido devuelve "
        f"{tolerado_falso} s de aguante y la desigualdad se cae ({tolerado_falso} < "
        f"{DERIVA_POSIBLE:.1f}): el pack distingue un colchon que cubre de uno que no")

    limite_falso_h = 30 * 24.0
    deriva_falsa = DERIVA_DIA * limite_falso_h / 24.0 + M_TOLERANCIA_S
    b.control_negativo(
        TOLERADO < deriva_falsa,
        f"y con un limite duro de 30 dias en vez de {HORAS_LIMITE / 24:.0f} la misma "
        f"cuenta da {deriva_falsa:.0f} s de deriva posible contra {TOLERADO} s de "
        "aguante: el pack tambien caza el plazo estirado, que es la otra mitad de la "
        "desigualdad")

    # =======================================================================
    # 6. EL VIGILANTE DEL ARNES QUE LO MIDE SOBRE EL C++
    # =======================================================================
    #
    # Un instrumento que no esta en la compuerta no mide nada, y no deja rastro de que
    # falta (N-43). Este bloque es a compilar_degradado.ps1 lo que barrera_04 es a
    # compilar_dos_puntas.ps1.
    dir_arnes = os.path.join(fw.FIRMWARE, *ARNES_DIR)
    guion_ok = os.path.isfile(os.path.join(fw.FIRMWARE, *ARNES_GUION))
    b.verificar(
        os.path.isdir(dir_arnes) and guion_ok,
        "existe el arnes que EJECUTA las dos puntas en Modo Degradado con relojes "
        "independientes (Validacion_Automatico/compilar_degradado.ps1). Este pack "
        "relaciona constantes; aquel dice si el firmware hace lo que la constante promete",
        "NO existe Validacion_Automatico/compilar_degradado.ps1. Sin el, el margen del "
        "Degradado vuelve a estar cubierto SOLO por modelos de Python escritos a mano "
        "-este incluido-, que es lo que el apartado 8 de CLAUDE.md avisa que no prueba "
        "el codigo")
    if not (os.path.isdir(dir_arnes) and guion_ok):
        return

    guion = _lee(fw, *ARNES_GUION)

    # La guarda de rutas del arnes nuevo (regla 5): los instrumentos leen el fuente por
    # ruta, y mover o renombrar un .cpp rompe uno sin que nada lo diga.
    rutas = re.findall(r"'src\\(\w+\.cpp)'", guion)
    faltan = [n for n in sorted(set(rutas))
              if not any(fw.existe(p, "src", n) for p in ("Maestro", "Esclavo"))]
    b.verificar(
        len(rutas) >= 10 and not faltan,
        f"el guion del arnes nombra {len(rutas)} fuentes del firmware por ruta y todos "
        "siguen existiendo",
        f"el guion nombra {len(rutas)} rutas y estas ya no estan: {faltan}. Mover o "
        "renombrar un .cpp rompe un instrumento; el movimiento y la actualizacion de "
        "rutas van en el MISMO commit")

    # Lo que hace distinto a este arnes: compila EL modo_degradado.cpp DE LAS DOS
    # PUNTAS. Sin el del Maestro no hay "las dos en Degradado", que es la propiedad.
    for punta in ("Maestro", "Esclavo"):
        bloque = re.search(r"\$fuentes%s\s*=\s*@\((.*?)\)\s*\n" % punta.capitalize(),
                           guion, re.S)
        b.verificar(
            bloque is not None and "modo_degradado.cpp" in bloque.group(1),
            f"el arnes compila {punta}/src/modo_degradado.cpp REAL",
            f"el arnes NO compila {punta}/src/modo_degradado.cpp. Sin las dos, lo que "
            "mide no es 'las dos puntas en Degradado a la vez' sino una punta ciclando "
            "por reloj contra otra en otro modo")

    # Y relee del C++ las constantes de las que sale el numero que publica. Un literal
    # escrito a mano ahi haria que el arnes siguiera midiendo contra el ciclo viejo.
    orq = _lee(fw, *ARNES_DIR, ARNES_ORQ)
    for nombre in ("DEG_VERDE_SEG", "DEG_DESPEJE_SEG", "LIMITE_DURO_MS",
                   "TOLERANCIA_DESFASE_S"):
        b.verificar(
            re.search(r"leerNumero\([^;]*%s" % re.escape(nombre), orq, re.S) is not None,
            f"el orquestador del Degradado RELEE {nombre} del C++ real",
            f"el orquestador no relee {nombre}: dimensionaria el barrido con otra cosa "
            "que el firmware y seguiria publicando un numero")

    b.verificar(
        "compilar_degradado.ps1" in _lee(fw, "compuerta.py"),
        "compuerta.py llama al arnes del Degradado: lo que mide entra en el acta",
        "compuerta.py NO llama a compilar_degradado.ps1. El arnes corre a mano y no deja "
        "rastro en ninguna acta, asi que el margen contra la deriva vuelve a no "
        "vigilarlo nadie en la verificacion oficial. Se cierra dando de alta la suite "
        "igual que las otras:\n"
        "        d = os.path.join(RAIZ, 'Validacion_Automatico')\n"
        "        subprocess.run([... '-File', os.path.join(d, 'compilar_degradado.ps1')])\n"
        "        anotar('arnes de las dos puntas en Degradado', PASS si returncode==0, "
        "la linea RESULTADO)")

    # Control negativo de la guarda de rutas, sobre un guion SINTETICO. Con el real,
    # mutarlo apagaria tambien la comprobacion de arriba, y una linea que no puede
    # fallar sola no es una comprobacion: es un adorno.
    guion_falso = ("$fuentesMaestro = @(\n"
                   "    (Join-Path $MAESTRO 'src\\semaforo.cpp'),\n"
                   "    (Join-Path $MAESTRO 'src\\NO_EXISTE.cpp')\n"
                   ")\n")
    leidas = re.findall(r"'src\\(\w+\.cpp)'", guion_falso)
    b.control_negativo(
        "semaforo.cpp" in leidas and
        [n for n in leidas
         if not any(fw.existe(p, "src", n) for p in ("Maestro", "Esclavo"))] ==
        ["NO_EXISTE.cpp"],
        "la guarda de rutas del arnes del Degradado SI senala un fuente que el guion "
        "nombra y que no existe, y NO senala al que si existe")
