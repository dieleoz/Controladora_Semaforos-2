# ===== banco/packs/costura_10_funciones_muertas.py =====
#
# UNA FUNCION QUE NADIE LLAMA ES LA VERSION SILENCIOSA DE LA PRUEBA MUERTA.
#
# N-73. Los manuales anunciaban una "Caja Negra de Alarmas" -"registro inmediato de
# eventos con timestamp para diagnosticar la causa exacta de cualquier caida de radio en
# obra"-. bluetooth_reportarAlarma() estaba declarada en el header, definida en el .cpp,
# documentada con un ejemplo... y SIN UN SOLO SITIO QUE LA INVOCARA, en las dos puntas.
#
# Es exactamente la forma de N-63: un pinMode() sin digitalRead(), pero con cuatro
# documentos encima describiendo lo que hace. Y se pago: el reporte de campo del 27/08
# -"se va a Modo Degradado cada nada cuando llueve"- no se pudo diagnosticar porque no
# habia registro que mirar. El instrumento existia; nadie lo habia enchufado.
#
# QUE HACE ESTE PACK, Y POR QUE NO EXIGE "CERO HUERFANAS".
#
# El censo completo encontro 29 funciones declaradas sin llamador. Exigir cero seria
# falso y ademas ruidoso: la mayoria son getters legitimos y las cuatro de la franja
# nocturna pertenecen a SFTY-20, que OPTIMIZACIONES.md declara honestamente "DISENO, NO
# IMPLEMENTADO". Un documento que dice "sin construir" no miente.
#
# Asi que la propiedad es un TRINQUETE, no un absoluto:
#
#   1. La lista de huerfanas conocidas esta CONGELADA aqui abajo, una por una.
#   2. Una huerfana NUEVA falla: alguien acaba de escribir codigo que nadie ejecuta, o
#      -peor- acaba de dejar sin llamador algo que si se llamaba.
#   3. Una que GANA llamador tambien falla, para que salga de la lista. Sin esto la
#      lista se llena de nombres obsoletos y deja de significar nada.
#   4. Y aparte, se exige que las funciones ANUNCIADAS EN LOS MANUALES tengan llamador.
#      Esa es la clase peligrosa: la que un tecnico espera encontrar funcionando.

import re

NOMBRE = "costura_10_funciones_muertas"
DESCRIPCION = "ninguna funcion nueva se queda sin llamador, y las que los manuales anuncian tienen uno"

PUNTAS = ("Maestro", "Esclavo")

# Las huerfanas CONOCIDAS el 27/08/2026, con su motivo. Si esta lista se queda vieja el
# pack lo dice: sobra tanto una que aparece como una que desaparece.
CONOCIDAS = {
    "Maestro": {
        # D-44: SALEN DE LA LISTA, porque se retiraron del firmware, las huerfanas que
        # el censo del legacy nombro: bluetooth_testLedsActivo, reloj_textoHora,
        # reloj_hayCristal, semaforo_toggle, coordinador_intentarHandshake,
        # coordinador_medirDesfase y coordinador_reiniciarConexion. Dejarlas aqui seria
        # vigilar el aire (comprobacion de "desaparecidas"). reloj_ajustar(), que quedo sin
        # llamador al irse el Modo Hora, se retiro tambien y no entra.
        #
        # respaldo.cpp y respaldo.h son GEMELOS BYTE A BYTE entre las dos puntas (lo exige
        # maestro_02_respaldo): estas tres no tienen lector en el Maestro y no se pueden
        # retirar de una sola punta.
        "respaldo_valido", "respaldo_verdeSeg", "respaldo_despejeSeg",
        # SFTY-20, franja nocturna: OPTIMIZACIONES.md la declara "DISENO, NO
        # IMPLEMENTADO". Andamio que D-46 conserva (se construira, N-175).
        "reloj_ajustarFranjaNocturna", "reloj_esHorarioNocturno",
        "reloj_inicioNoche", "reloj_finNoche",
    },
    "Esclavo": {
        # N-133: respaldo.cpp/.h gemelos byte a byte (maestro_02_respaldo). El ciclo
        # automatico solo existe en el Maestro: aqui no hay a quien llamarlas.
        "respaldo_guardarTiemposCiclo", "respaldo_tiemposCiclo", "respaldo_valido",
        # protocolo.h es identico en las dos puntas; el Maestro si la llama.
        "protocolo_reiniciarContadores",
        # D-26 (11/09): reloj_dia se queda declarada. Su unico lector en esta punta se
        # retiro el 11/09, el dia lo impone la radio (CMD_HORA_D) y su gemela del Maestro
        # SI tiene lector. D-44 no la retira: el arnes del Degradado a dos puntas la lee
        # (reloj_real/rtc_periferico.cpp, frontera de segundo) con su logica real.
        "reloj_dia",
        # D-44: SALEN DE LA LISTA, retiradas del firmware: bluetooth_testLedsActivo,
        # botonArriba/Abajo/Aceptar/Cancelar, semaforo_toggle, semaforo_iniciarTestLeds y
        # los getters de pantalla del Degradado (degradado_textoEstado, textoFase,
        # rendidoPorHora, segundosParaCambio). D-46 retiro ademas degradado_textoRechazo,
        # cuyo unico lector era SET_MODO:DEGRADADO.
    },
}

# Las que los manuales anuncian como funcion existente. Esta es la clase que costo
# N-73, y por eso se comprueba aparte y con mensaje propio.
ANUNCIADAS = ("bluetooth_reportarAlarma", "bluetooth_reportarEvento")

_TIPOS = (r"void|bool|int|uint\d+_t|int\d+_t|unsigned long|long|char|float|"
          r"const char\*|EstadoSemaforo")


def _declaradas(fw, punta):
    """Las funciones que los headers de esa punta publican."""
    fuera = {}
    for h in fw.fuentes_de(punta, "include", ".h"):
        for m in re.finditer(r"\b(?:%s)\s+\*?(\w+)\s*\([^;{]*\)\s*;" % _TIPOS,
                             fw.codigo(punta, "include", h)):
            fuera.setdefault(m.group(1), h)
    return fuera


def _alias(codigo):
    """Lo que hace el preprocesador con `#define A B` (dos identificadores): desde esa
    linea hasta el final del FICHERO, cada `A(` es una llamada a `B(`.

    A-15 (29/09): coordinador.cpp del Maestro lleva `#define protocolo_enviarPaquete
    degAuto_enviar`, y sus llamadas son las de degAuto_enviar. Sin esto la contaba
    huerfana. El borde: solo alias de nombre a nombre, solo en su fichero y solo detras
    de la linea del #define; quitada la linea, degAuto_enviar vuelve a caer."""
    for m in list(re.finditer(r"^[ \t]*#define[ \t]+(\w+)[ \t]+(\w+)[ \t]*$", codigo, re.M)):
        a, b = m.group(1), m.group(2)
        codigo = codigo[:m.end()] + re.sub(r"\b%s(?=\s*\()" % re.escape(a), b,
                                           codigo[m.end():])
    return codigo


def _cuerpo(fw, punta):
    return "".join(_alias(fw.codigo(punta, "src", f)) for f in fw.fuentes_de(punta, "src")
                   if f.endswith(".cpp"))


def correr(b, fw):
    b.titulo("Funciones declaradas que nadie llama: el trinquete")

    for punta in PUNTAS:
        decl = _declaradas(fw, punta)
        if len(decl) < 50:
            raise fw.Abortado(
                "solo se hallaron %d funciones declaradas en los headers del %s. Son mas "
                "de noventa: fallo el buscador, no el firmware, y un censo corto "
                "aprobaria por no encontrar nada" % (len(decl), punta))
        cuerpo = _cuerpo(fw, punta)

        # Una aparicion = solo la definicion. Dos o mas = alguien la llama.
        huerfanas = {fn for fn in decl
                     if len(re.findall(r"\b%s\s*\(" % re.escape(fn), cuerpo)) <= 1}

        nuevas = sorted(huerfanas - CONOCIDAS[punta])
        b.verificar(
            not nuevas,
            "%s: %d funciones declaradas, %d sin llamador y TODAS conocidas"
            % (punta, len(decl), len(huerfanas)),
            "%s: %s se declara(n) y NADIE las llama, y no estaban en la lista. O es "
            "codigo recien escrito que no se ejecuta, o -peor- algo que si se llamaba y "
            "acaba de quedarse sin llamador. La segunda es N-73: una funcion viva que "
            "muere en silencio mientras los documentos siguen anunciandola"
            % (punta, ", ".join(nuevas)))

        # Y la direccion contraria, que es la que mantiene honesta la lista.
        revividas = sorted(fn for fn in CONOCIDAS[punta]
                           if fn in decl and fn not in huerfanas)
        b.verificar(
            not revividas,
            "%s: ninguna de la lista de huerfanas conocidas ha ganado llamador" % punta,
            "%s: %s YA tiene llamador y sigue en la lista de huerfanas conocidas. Hay "
            "que sacarla: una lista que acumula nombres obsoletos deja de poder fallar, "
            "que es la unica forma que tiene de servir para algo"
            % (punta, ", ".join(revividas)))

        desaparecidas = sorted(fn for fn in CONOCIDAS[punta] if fn not in decl)
        b.verificar(
            not desaparecidas,
            "%s: las %d huerfanas de la lista siguen existiendo en los headers"
            % (punta, len(CONOCIDAS[punta])),
            "%s: %s esta en la lista de huerfanas conocidas y ya no se declara en "
            "ningun header. Se retiro el codigo y no la lista: quedan vigilando el aire"
            % (punta, ", ".join(desaparecidas)))

    # ---- Las anunciadas en los manuales: esta es la clase que costo N-73 ----
    for punta in PUNTAS:
        cuerpo = _cuerpo(fw, punta)
        for fn in ANUNCIADAS:
            usos = len(re.findall(r"\b%s\s*\(" % re.escape(fn), cuerpo))
            b.verificar(
                usos >= 2,
                "%s: %s se llama desde %d sitio(s) del firmware"
                % (punta, fn, usos - 1),
                "%s: %s esta definida y NADIE la llama, y los manuales la anuncian como "
                "funcion existente -la Caja Negra que 'registra la causa exacta de "
                "cualquier caida de radio en obra'-. Un tecnico la buscara en el registro "
                "y no habra registro. Es N-63 con documentacion encima" % (punta, fn))

    # ---- Controles negativos ----
    b.control_negativo(
        len(re.findall(r"\bfuncion_inventada\s*\(", "void otra() { foo(); }")) == 0,
        "el contador de llamadas no encuentra una funcion que no esta")
    b.control_negativo(
        len(re.findall(r"\bfoo\s*\(", "void foo() { } void otra() { foo(); }")) == 2,
        "y SI distingue definicion (1 aparicion) de definicion mas llamada (2)")
    b.control_negativo(
        not re.search(r"\b(?:void)\s+\*?(\w+)\s*\([^;{]*\)\s*;", "void foo() { bar(); }"),
        "el lector de headers no confunde una DEFINICION con una declaracion: solo "
        "cuenta las que acaban en punto y coma")
