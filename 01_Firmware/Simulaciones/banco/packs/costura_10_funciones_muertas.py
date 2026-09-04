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
        # Getters de telemetria que la pantalla dejo de pedir. No danan; se anotan.
        "bluetooth_testLedsActivo", "protocolo_tramasDescartadas",
        # N-86 retiro las tres del puerto de camara IA (protocolo_actualizarAI,
        # protocolo_obtenerAutosEsperandoAI, protocolo_obtenerUltimoTiempoAI). Ya no
        # se declaran en ningun header, asi que salen de la lista: la comprobacion de
        # "desaparecidas" las pediria por su nombre y estaria vigilando el aire.
        "respaldo_valido", "respaldo_verdeSeg", "respaldo_despejeSeg",
        "reloj_textoHora", "semaforo_toggle",
        # SFTY-20, franja nocturna: OPTIMIZACIONES.md la declara "DISENO, NO
        # IMPLEMENTADO" y el protocolo de pruebas "especificada, sin construir".
        # El documento NO miente, asi que esto no es un hallazgo: es obra a medias
        # declarada como tal.
        "reloj_ajustarFranjaNocturna", "reloj_esHorarioNocturno",
        "reloj_inicioNoche", "reloj_finNoche",
        # API del coordinador que quedo del diseno anterior de reconexion.
        # N-108 (31/08): coordinador_comunicacionPerdida SALE de la lista. Ya tiene
        # llamador -bluetooth.cpp la consulta para publicar el $EVENT de cambio de
        # estado del enlace-, y una huerfana que gano llamador y se queda anotada es
        # justo lo que este pack persigue: la lista dejaria de poder fallar.
        "coordinador_intentarHandshake",
        "coordinador_medirDesfase", "coordinador_reiniciarConexion",
    },
    "Esclavo": {
        # N-133 (04/09): los dos accesos a los tiempos del ciclo AUTOMATICO.
        #
        # EL MOTIVO ES COMPROBABLE, que es lo que §3.bis exige de una excepcion: no se
        # acepta un huerfano "porque si". Aqui son dos hechos que otro pack ya mide:
        #
        #   1. respaldo.cpp y respaldo.h son GEMELOS BYTE A BYTE entre las dos puntas,
        #      y lo exige maestro_02_respaldo -"un respaldo escrito por una punta
        #      podria no validarlo la otra"-. O sea que estas dos funciones no PUEDEN
        #      no estar aqui: retirarlas del Esclavo separaria a los gemelos y ese
        #      pack caeria en el acto.
        #   2. El ciclo automatico solo existe en el Maestro: no hay
        #      Esclavo/src/modo_automatico.cpp. El Esclavo no elige tiempos; los
        #      recibe por radio. Asi que en esta punta no hay a quien llamarlas.
        #
        # Si algun dia el Esclavo gana ciclo propio, ganaran llamador y este pack lo
        # dira -"una huerfana que gano llamador y se queda anotada"-, que es justo lo
        # que persigue.
        "respaldo_guardarTiemposCiclo", "respaldo_tiemposCiclo",
        # N-108 (31/08): protocolo_tramasDescartadas SALE de la lista en esta punta.
        # El $ALARM de la caida y el $EVENT de la vuelta publican los contadores de
        # SFTY-15, que es para lo que se escribieron: separan "no llega nada" de
        # "llega basura" de "enlace marginal". Sigue huerfana en el MAESTRO, donde
        # nadie los publica todavia, y por eso alli se queda anotada.
        "bluetooth_testLedsActivo",
        # N-86: mismas tres del puerto de camara IA, retiradas tambien en esta punta.
        "protocolo_reiniciarContadores",
        "respaldo_valido", "reloj_dia", "semaforo_toggle",
        # El Esclavo NO enciende luces por su cuenta: rechaza TEST_LEDS y no fuerza
        # verde. Que estas dos no tengan llamador es la barrera funcionando.
        "semaforo_iniciarTestLeds", "semaforo_forzarVerde",
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


def _cuerpo(fw, punta):
    return "".join(fw.codigo(punta, "src", f) for f in fw.fuentes_de(punta, "src")
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
