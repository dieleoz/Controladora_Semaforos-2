# ===== banco/packs/maestro_08_set_tiempos.py =====
#
# LOS TIEMPOS DEL CICLO DESDE EL CELULAR: QUE EL FIRMWARE SEA QUIEN DIGA QUE NO.
#
# N-69. Hasta hoy los tiempos solo se tocaban subiendo a la pantalla del poste. Ahora
# entran por Bluetooth, y eso cambia quien puede mandarlos: la app de hoy, una app
# vieja que alguien no actualizo, o cualquiera con un terminal serie y el PIN.
#
# POR ESO LOS LIMITES VAN EN EL C++ Y NO EN LA INTERFAZ. Que la app valide rangos es
# comodidad para el usuario; la garantia es que el firmware rechace lo que no cabe.
# Una interfaz se reemplaza; este fichero es el que se queda.
#
# QUE SE COMPRUEBA, Y POR QUE ESTAS COSAS Y NO OTRAS.
#
# Lo facil seria comprobar que un valor bueno se acepta. Eso no demuestra nada: un
# firmware que aceptara TODO tambien lo pasaria. Lo que hay que exigir es que
# RECHACE -y que rechace ANTES de tocar ninguna variable-, porque una validacion que
# corrige a medias deja el ciclo con un verde nuevo y un despeje viejo, que es peor
# que no haber cambiado nada.
#
# Y el caso que de verdad importa: CON EL CICLO EN MARCHA NO SE TOCAN. La duracion se
# recalcula en cada vuelta a partir de esas variables, asi que bajar un tiempo a mitad
# de fase ACORTA LA FASE EN CURSO -y una de esas fases es el todo-rojo de despeje, que
# es lo unico que garantiza que el tramo quedo vacio-.

import re

NOMBRE = "maestro_08_set_tiempos"
DESCRIPCION = "los limites de los tiempos viven en el C++, rechazan antes de tocar nada y no se aplican en marcha"

FUENTE = ("Maestro", "src", "modo_automatico.cpp")
BT = ("Maestro", "src", "bluetooth.cpp")

LIMITES = ("VERDE_MIN_MIN", "VERDE_MIN_MAX", "ROJO_MIN_MIN", "ROJO_MIN_MAX",
           "DESPEJE_SEG_MIN", "DESPEJE_SEG_MAX")


def _cuerpo(codigo, firma):
    i = codigo.find(firma)
    if i < 0:
        return None
    j = codigo.find("{", i)
    nivel = 0
    for k in range(j, len(codigo)):
        if codigo[k] == "{":
            nivel += 1
        elif codigo[k] == "}":
            nivel -= 1
            if nivel == 0:
                return codigo[j:k + 1]
    return None


def correr(b, fw):
    b.titulo("SET_TIEMPOS: los limites mandan, y mandan antes de tocar nada")

    codigo = fw.codigo(*FUENTE)

    # ---- 1. Los seis limites se LEEN del C++, sin valor por defecto ----
    valores = {}
    for nombre in LIMITES:
        m = re.search(r"%s\s*=\s*(\d+)" % nombre, codigo)
        if m:
            valores[nombre] = int(m.group(1))
    if len(valores) != len(LIMITES):
        raise fw.Abortado(
            "faltan limites en modo_automatico.cpp: %s. Sin ellos este pack compararia "
            "contra numeros inventados y aprobaria cualquier rango"
            % ", ".join(n for n in LIMITES if n not in valores))
    b.verificar(
        True,
        "los seis limites se leen del C++: verde %d-%d min, rojo %d-%d min, despeje "
        "%d-%d s" % tuple(valores[n] for n in LIMITES),
        "no deberia llegarse aqui")

    # ---- 2. Que los rangos tengan sentido ----
    for lo, hi, que in (("VERDE_MIN_MIN", "VERDE_MIN_MAX", "el verde"),
                        ("ROJO_MIN_MIN", "ROJO_MIN_MAX", "el rojo"),
                        ("DESPEJE_SEG_MIN", "DESPEJE_SEG_MAX", "el despeje")):
        b.verificar(
            0 < valores[lo] < valores[hi],
            "%s tiene un rango util: %d..%d" % (que, valores[lo], valores[hi]),
            "%s tiene el rango %d..%d, que es vacio o empieza en cero: un minimo de 0 "
            "deja pasar un tiempo nulo" % (que, valores[lo], valores[hi]))

    # El despeje es el unico de los tres que es seguridad vial: es lo que garantiza
    # que el tramo quedo vacio. Su minimo no puede ser simbolico.
    b.verificar(
        valores["DESPEJE_SEG_MIN"] >= 10,
        "el despeje no se puede bajar de %d s: por debajo, el margen que vacia el "
        "tramo desaparece" % valores["DESPEJE_SEG_MIN"],
        "el despeje admite bajar hasta %d s. Ese es el tiempo que impide que dos "
        "vehiculos coincidan en el tramo: un minimo simbolico convierte un ajuste de "
        "comodidad en un riesgo vial" % valores["DESPEJE_SEG_MIN"])

    # ---- 3. Los tres parametros se validan, uno por uno ----
    cuerpo = _cuerpo(codigo, "bool modoAutomatico_fijarTiempos(")
    if cuerpo is None:
        raise fw.Abortado(
            "no se encuentra modoAutomatico_fijarTiempos() en el C++: o cambio de "
            "nombre o se retiro, y este pack estaria midiendo el aire")
    plano = cuerpo.replace(" ", "").replace("\n", "")
    for arg, lo, hi in (("verdeMin", "VERDE_MIN_MIN", "VERDE_MIN_MAX"),
                        ("rojoMin", "ROJO_MIN_MIN", "ROJO_MIN_MAX"),
                        ("despejeSeg", "DESPEJE_SEG_MIN", "DESPEJE_SEG_MAX")):
        b.verificar(
            ("%s<%s" % (arg, lo)) in plano and ("%s>%s" % (arg, hi)) in plano,
            "%s se compara contra sus dos limites" % arg,
            "%s no se compara contra %s y %s. Un parametro sin guarda entra tal cual, "
            "y basta uno para desajustar el ciclo" % (arg, lo, hi))

    # ---- 4. Y NADA se asigna antes de haber validado ----
    # Es la propiedad que separa una validacion de un adorno: si la primera asignacion
    # ocurriera antes del ultimo return false, un valor malo dejaria el ciclo a medio
    # cambiar -verde nuevo, despeje viejo-, que es peor que rechazarlo entero.
    pos_ultimo_rechazo = plano.rfind("returnfalse;")
    pos_primera_asignacion = min(
        [p for p in (plano.find("minVerde="), plano.find("minRojo="),
                     plano.find("segEstatico=")) if p >= 0] or [-1])
    b.verificar(
        pos_ultimo_rechazo >= 0 and 0 <= pos_ultimo_rechazo < pos_primera_asignacion,
        "ninguna variable se toca antes del ultimo rechazo: o entran los tres valores "
        "o no entra ninguno",
        "hay una asignacion ANTES del ultimo 'return false'. Un valor invalido dejaria "
        "el ciclo a medio cambiar -por ejemplo verde nuevo con despeje viejo-, que es "
        "peor que rechazarlo entero")

    # ---- 5. Con el ciclo EN MARCHA, no se tocan ----
    b.verificar(
        "modoAutomatico_enMarcha()" in plano and "returnfalse;" in plano,
        "con el ciclo corriendo se rechaza: bajar un tiempo a mitad de fase acortaria "
        "la fase EN CURSO, incluido un todo-rojo ya empezado",
        "no hay guarda de 'en marcha'. La duracion se recalcula en cada vuelta desde "
        "estas variables, asi que un cambio en caliente puede acortar el todo-rojo que "
        "esta corriendo — el unico tiempo que garantiza que el tramo quedo vacio")

    # ---- 6. Una sola copia de los rangos ----
    # Si el despachador de Bluetooth repitiera los limites, habria dos listas que
    # alguien tendria que sincronizar, y el dia que difieran la app dejaria pasar lo
    # que el ciclo rechaza -o al reves-.
    bt = fw.codigo(*BT)
    b.verificar(
        not any(re.search(r"%s" % n, bt) for n in LIMITES),
        "el despachador de Bluetooth no repite los rangos: solo traduce texto a "
        "numeros y deja decidir al ciclo",
        "bluetooth.cpp nombra los limites. Dos copias de un rango es una que se queda "
        "atras: el ciclo y la puerta de entrada acabarian aceptando cosas distintas")

    # ---- 7. Controles negativos ----
    b.control_negativo(
        _cuerpo("void otra(){ int x; }", "bool modoAutomatico_fijarTiempos(") is None,
        "el extractor no encuentra un cuerpo donde no hay tal funcion")
    b.control_negativo(
        "despejeSeg<DESPEJE_SEG_MIN" not in "if(despejeSeg<5)return false;".replace(" ", ""),
        "una guarda escrita con un numero a mano NO cuenta como comparada contra el "
        "limite con nombre")
