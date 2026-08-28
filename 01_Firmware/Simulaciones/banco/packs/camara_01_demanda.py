# ===== banco/packs/camara_01_demanda.py =====
#
# LA DEMANDA DE CAMARA: COMO ENTRA, Y QUE PASA CON EL PIN QUE NO SE LEE.
#
# Abre la familia camara_* que la matriz de cobertura tenia vacia: hasta hoy, las
# camaras eran la unica entrada del firmware sin una sola comprobacion detras.
#
# ---------------------------------------------------------------------------------
# EL CABLE TRAMPA DE CAM_UMBRAL_PIN, Y POR QUE NO ES UNA PRUEBA AL REVES
#
# ACTUALIZADO EL 27/08 (N-64), Y LA CORRECCION VALE MAS QUE EL TEXTO ANTERIOR.
#
# Este bloque decia que PB8 era la entrada de la camara de umbral, "cableada y en
# reposo". Al trazar el esquematico BUENO -habia dos, y el que se leia estaba
# incompleto- se midio que PB8 va por R16 1K a un LED (D5). No hay bornera, no hay
# opto: no es una entrada. Asi que la camara de umbral no esta "en reposo": NO TIENE
# DONDE ENTRAR, y hacen falta un hilo y un comando de radio, no un pinMode.
#
# El problema no era el pin: era que CUATRO documentos afirmaban que funcionaba. Un
# manual que promete una funcion inexistente es peor que una pagina en blanco, porque
# el auditor la da por buena y el instalador cablea esperando algo.
#
# ASI QUE ESTA COMPROBACION EXISTE PARA CAERSE. El dia que alguien implemente el
# umbral, este pack FALLA -y esa es su unica razon de ser-: obliga a actualizar los
# manuales EN EL MISMO COMMIT en que se implementa. Es la §8.quater al reves: en vez de
# una prueba que celebra un defecto y hay que invertir cuando se arregle, es una prueba
# que sujeta una ausencia y hay que retirar cuando se llene.
#
# Si te ha traido aqui un FALLA, lo que toca es: quitar esta comprobacion, actualizar
# los Manuales 1, 2 y 9, y escribir el pack que mida el conteo de verdad.

import re

NOMBRE = "camara_01_demanda"
DESCRIPCION = "la demanda entra por flanco y con ventana de silencio; el umbral sigue en reposo"

PUNTAS = ("Maestro", "Esclavo")

# El rele de la camara AcuSense cierra ~1 s por deteccion (Manual 9, paso 3 de la
# parametrizacion). La ventana de silencio tiene que superar ese pulso o la MISMA
# deteccion se contaria dos veces. No se escribe el valor aqui: se lee del C++ y se
# comprueba la propiedad.
PULSO_RELE_MS = 1000


def correr(b, fw):
    b.titulo("Demanda de camara: flanco, ventana de silencio y umbral en reposo")

    # ---- 1. PB8 NO se declara como entrada de camara (N-64) ----
    # Esta comprobacion esta INVERTIDA a proposito el 27/08. Antes exigia que
    # CAM_UMBRAL_PIN existiera -documentaba que el pin estaba reservado-. Al trazar el
    # esquematico bueno se midio que PB8 va por R16 1K a un LED (D5): no es una bornera
    # ni una entrada optoacoplada. Un nombre que dice "camara" sobre un pin que enciende
    # un testigo es la clase de mentira que hizo falta N-59 para descubrir, asi que
    # ahora el pack exige lo contrario: que ese nombre NO vuelva.
    for punta in PUNTAS:
        pines = fw.texto(punta, "include", "pines.h")
        b.verificar(
            re.search(r"#define\s+CAM_UMBRAL_PIN", pines) is None,
            f"{punta}: PB8 ya no se llama CAM_UMBRAL_PIN -no es una entrada de camara-",
            f"{punta}: volvio CAM_UMBRAL_PIN a pines.h. PB8 alimenta el LED D5 por R16 "
            "1K: si de verdad hay ahora una entrada de umbral, tiene que ser OTRO pin "
            "-un hilo desde PA11/PA12/PA15/PC13, o el pad de PB8 sin R16/D5- y hay que "
            "actualizar los Manuales 1, 2 y 9 en el mismo commit")
        b.verificar(
            re.search(r"#define\s+LED_TESTIGO\s+PB8", pines) is not None,
            f"{punta}: PB8 se declara por lo que es: LED_TESTIGO",
            f"{punta}: no se encuentra LED_TESTIGO en PB8. El pin existe en la placa y "
            "enciende D5; dejarlo sin nombre invita a que alguien lo reutilice sin saber "
            "que hay un diodo colgando")

    # ---- 2. CABLE TRAMPA: nadie lo lee ----
    lectores = []
    for punta in PUNTAS:
        import os
        base = os.path.join(fw.FIRMWARE, punta, "src")
        for fichero in sorted(f for f in os.listdir(base) if f.endswith(".cpp")):
            codigo = fw.codigo(punta, "src", fichero)      # sin comentarios
            if re.search(r"digitalRead\s*\(\s*(CAM_UMBRAL_PIN|LED_TESTIGO)", codigo):
                lectores.append(f"{punta}/src/{fichero}")

    b.verificar(
        not lectores,
        "nadie lee PB8 como si fuera una entrada: coincide con la placa -es un LED- y "
        "con los manuales, que dicen que la camara de umbral no esta en V9.0",
        f"PB8 YA SE LEE, en {lectores}. Ahi hay un LED por R16 1K, no un contacto seco: "
        "leerlo devuelve el estado del diodo, no el de ninguna camara. Si se ha cableado "
        "una entrada de verdad en otro pin, cambia el nombre y actualiza los Manuales 1, "
        "2 y 9 en el mismo commit")

    # ---- 2.bis LA POLARIDAD, QUE ES DE LA PLACA Y NO DEL GUSTO DE NADIE (N-67) ----
    #
    # PB0 lleva R64 de 10k A MASA y la bornera J14 lo saca junto a 3,3 V: el contacto
    # seco cierra a 3,3 V, o sea ACTIVO EN ALTO. Con INPUT_PULLUP el pull-up interno
    # (~40k) contra ese 10k deja el pin en 0,66 V -LOW- y el firmware habria visto
    # demanda permanente sin camara conectada, y "sin demanda" al cerrarla.
    #
    # Esta comprobacion existe porque el defecto no se ve en el PC: solo aparece con la
    # bornera cableada, y para entonces ya hay alguien subido a un poste.
    for punta in PUNTAS:
        halladas = []
        for fichero in fw.fuentes_de(punta, "src"):
            codigo = fw.codigo(punta, "src", fichero)
            if "CAM_DEMANDA_PIN" not in codigo:
                continue
            halladas += re.findall(r"pinMode\s*\(\s*CAM_DEMANDA_PIN\s*,\s*(\w+)\s*\)", codigo)
        b.verificar(
            halladas and all(m == "INPUT" for m in halladas),
            "%s: la entrada de camara se declara INPUT a secas -el reposo lo fija el "
            "pull-down de 10k de la placa-" % punta,
            "%s: la entrada de camara se declara %s. Con INPUT_PULLUP el pin queda en "
            "0,66 V contra el pull-down de la placa: demanda permanente sin camara "
            "conectada, e invertida al cerrarla" % (punta, halladas or "(no se halla)"))

    lecturas = []
    for punta in PUNTAS:
        for fichero in fw.fuentes_de(punta, "src"):
            codigo = fw.codigo(punta, "src", fichero)
            lecturas += re.findall(
                r"digitalRead\s*\(\s*(?:CAM_DEMANDA_PIN|pin)\s*\)\s*==\s*(\w+)", codigo)
    b.verificar(
        lecturas and all(v == "HIGH" for v in lecturas),
        "las %d lecturas de la camara comparan contra HIGH: activo en alto, como la "
        "placa" % len(lecturas),
        "hay lecturas de camara contra %s. La placa cierra el contacto a 3,3 V: leer "
        "LOW invierte la deteccion" % sorted(set(lecturas)))

    # ---- 2.ter EL PIN DE PRESENCIA NO SE DECLARA HASTA QUE SE LEA (SFTY-29) ----
    #
    # PA11 esta elegido como entrada de presencia y el cobre confirma que esta libre,
    # pero EL HILO NO EXISTE TODAVIA. Declararlo en pines.h ahora seria repetir
    # exactamente el defecto que costo N-59 y N-64: un pin con nombre de funcion y sin
    # funcion detras, que acaba en cuatro manuales como si sirviera.
    #
    # Asi que esta comprobacion no exige que el pin exista: exige que si ALGUIEN LO
    # DECLARA, tambien lo lea. El dia que se cablee, se declara y se lee en el mismo
    # commit, y esto pasa solo.
    for punta in PUNTAS:
        pines = fw.texto(punta, "include", "pines.h")
        declarado = re.search(r"#define\s+(PRESENCIA\w*|CAM_PRESENCIA\w*)\s+P[A-Z]\d+", pines)
        leido = False
        if declarado:
            nombre = declarado.group(1)
            for fichero in fw.fuentes_de(punta, "src"):
                if re.search(r"digitalRead\s*\(\s*%s" % nombre, fw.codigo(punta, "src", fichero)):
                    leido = True
                    break
        b.verificar(
            (declarado is None) or leido,
            "%s: el pin de presencia %s" % (
                punta,
                "no esta declarado todavia -el hilo aun no existe- y eso es correcto"
                if declarado is None else "se declara Y se lee"),
            "%s: %s esta declarado en pines.h y NINGUN .cpp lo lee. Es el defecto de "
            "N-59 otra vez: un pin con nombre de funcion y sin funcion detras. O se "
            "lee, o se retira hasta que exista el hilo"
            % (punta, declarado.group(1) if declarado else "?"))

    # ---- 3. La demanda se toma por FLANCO, no por nivel ----
    # El rele mantiene el contacto ~1 s: leer el nivel repetiria la peticion en cada
    # vuelta del loop durante todo ese segundo.
    main_esc = fw.codigo("Esclavo", "src", "main.cpp")
    hay_lectura = re.search(r"digitalRead\s*\(\s*CAM_DEMANDA_PIN", main_esc) is not None
    hay_anterior = re.search(r"demandaCamaraAnt", main_esc) is not None

    b.verificar(
        hay_lectura and hay_anterior,
        "el Esclavo compara la lectura de CAM_DEMANDA_PIN contra la anterior: pide por "
        "FLANCO, no por nivel",
        "el Esclavo no guarda el estado anterior de CAM_DEMANDA_PIN. Sin flanco, el "
        "segundo que dura el contacto del rele son cientos de vueltas del loop pidiendo "
        "lo mismo")

    # ---- 4. La ventana de silencio se lee del C++, sin valor por defecto ----
    puerta = fw.codigo("Esclavo", "src", "demanda.cpp")
    m = re.search(r"SILENCIO_MS\s*=\s*(\d+)", puerta)
    b.verificar(
        m is not None,
        "la ventana de silencio se lee de demanda.cpp, no esta escrita a mano en el pack",
        "no se encuentra SILENCIO_MS en Esclavo/src/demanda.cpp. Un banco que cae a un "
        "valor por defecto no demuestra nada: aborta antes que aprobar a ciegas")

    if m:
        silencio = int(m.group(1))
        b.verificar(
            silencio > PULSO_RELE_MS,
            f"la ventana de silencio ({silencio} ms) supera el pulso del rele "
            f"({PULSO_RELE_MS} ms): una misma deteccion no se cuenta dos veces",
            f"la ventana ({silencio} ms) NO supera el pulso del rele ({PULSO_RELE_MS} "
            "ms). El mismo cierre de contacto puede generar dos peticiones")

    # ---- 5. La primera demanda tras el arranque no se traga ----
    # millis() vale ~0 al arrancar; restar contra un tUltima tambien en 0 daria "dentro
    # de la ventana" y se perderia la peticion del primer coche.
    b.verificar(
        re.search(r"\bprimera\b", puerta) is not None,
        "demanda.cpp distingue la PRIMERA demanda tras el arranque: no se pierde la "
        "peticion del primer coche por el millis() en cero",
        "demanda.cpp no trata el arranque como caso aparte. Con millis() ~0 la primera "
        "peticion cae dentro de la ventana y se descarta en silencio")

    # ---- CONTROL NEGATIVO ----
    b.control_negativo(
        bool(re.search(r"digitalRead\s*\(\s*(CAM_UMBRAL_PIN|LED_TESTIGO)",
                       "void _x(){ if (digitalRead(LED_TESTIGO)) {} }")),
        "una lectura de PB8 colada en un .cpp se detecta, con el nombre viejo o el nuevo")

    b.control_negativo(
        re.search(r"SILENCIO_MS\s*=\s*(\d+)", "static const unsigned long SILENCIO_MS = 500;")
        .group(1) == "500",
        "el lector de SILENCIO_MS extrae el valor real y no uno fijo")
