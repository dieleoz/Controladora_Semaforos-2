# ===== 01_Firmware/Simulaciones/banco/fuente.py =====
#
# LECTURA DEL FIRMWARE REAL — una sola copia, compartida por todos los packs.
#
# POR QUE EXISTE ESTE FICHERO.
#
# Los tres validadores llevaban cada uno su propia version de esto: _ruta/_fuente/
# _codigo/cte en el Maestro, _ruta_firmware/_texto/_leer/_leer_hex en el Esclavo,
# ruta/texto/num en el de costura. Tres copias de la misma idea, y por tanto tres
# sitios donde arreglar el mismo fallo -y dos que se olvidan-.
#
# Es EXACTAMENTE el defecto que los validadores denuncian del firmware: codigo
# duplicado entre puntas que solo la disciplina mantiene igual. Un instrumento que
# comete el fallo que mide no es creible.
#
# LA REGLA QUE NO SE NEGOCIA: SIN VALOR POR DEFECTO.
#
# Si una constante no se puede leer del C++, esto ABORTA. No cae a un numero
# escrito a mano "que casualmente coincide": un banco que no puede fallar no
# demuestra nada, y el dia que alguien renombre la constante seguiria dando PASS
# midiendo el valor viejo mientras el firmware usa otro.

import hashlib
import os
import re

_AQUI = os.path.dirname(os.path.abspath(__file__))
FIRMWARE = os.path.normpath(os.path.join(_AQUI, "..", ".."))


class Abortado(Exception):
    """No se pudo MEDIR. No dice nada del firmware.

    Se lanza en vez de sys.exit() para que el corredor pueda seguir con los demas
    packs y reportar al final cual no pudo correr. Con sys.exit(), un pack roto se
    llevaba por delante a los diecinueve siguientes y el resumen decia mucho menos
    de lo que sabia."""


def ruta(*partes):
    """Resuelve una ruta dentro de 01_Firmware. ABORTA si no existe.

    Antes esto devolvia None en silencio y el fallo aparecia mas tarde, disfrazado
    de otra cosa. Un fuente que falta es N-36: el instrumento midiendo algo que ya
    no esta."""
    p = os.path.join(FIRMWARE, *partes)
    if not os.path.isfile(p):
        raise Abortado("no existe el fuente %s" % os.path.join(*partes))
    return p


def existe(*partes):
    """Como ruta(), pero preguntando en vez de abortando.

    Necesaria para la migracion a lib/Common: tras mover un fichero, la prueba deja
    de ser "las dos copias son iguales" y pasa a ser "NO existe copia local que
    tape a la comun". Eso hay que poder preguntarlo sin morir."""
    return os.path.isfile(os.path.join(FIRMWARE, *partes))


def texto(*partes):
    with open(ruta(*partes), "r", encoding="utf-8", errors="replace") as f:
        return f.read()


def codigo(*partes):
    """El fuente con los comentarios fuera.

    Sin esto, un patron puede acertar dentro de un comentario y dar por presente una
    guarda que no se compila. Ya paso: se dio por bueno un segundo filtro que era
    codigo muerto."""
    t = texto(*partes)
    t = re.sub(r"/\*.*?\*/", " ", t, flags=re.S)
    t = re.sub(r"//[^\n]*", " ", t)
    return t


def constante(partes, patron, que, base=10, factor=1):
    """Lee un numero del C++. ABORTA si no aparece. Sin valor por defecto, nunca."""
    m = re.search(patron, texto(*partes))
    if not m:
        raise Abortado(
            "no se pudo leer del C++ la constante de %s (patron %r en %s). Sin ese "
            "numero el banco mediria otra cosa que el firmware y seguiria dando PASS."
            % (que, patron, os.path.join(*partes)))
    return int(m.group(1), base) * factor


def comando(partes, nombre):
    """Codigo de comando del protocolo, en hexadecimal."""
    return constante(partes, r"#define\s+%s\s+0x([0-9A-Fa-f]+)" % nombre,
                     "el comando %s" % nombre, base=16)


def huella(*partes):
    """SHA-256 del contenido COMPLETO.

    Del fichero entero y no de una constante: la igualdad entre puntas tiene que
    romperse por cualquier byte que cambie, no solo por los que alguien penso en
    vigilar."""
    with open(ruta(*partes), "rb") as f:
        return hashlib.sha256(f.read()).hexdigest()


# ---------------------------------------------------------------------------------
# LOS DOCUMENTOS TAMBIEN SON UN FUENTE QUE SE PARSEA.
#
# El README, ESTADO.md y OPTIMIZACIONES.md publican cifras que dicen venir de una
# medida -"copiadas del acta", "se levanta buscando las etiquetas"-. Mientras nadie
# las comprobara, esa frase era una promesa: el 27/08 el README publicaba 32 rutas
# y 86,4% de flash contra las 38 rutas y el 92,8% que decia el acta que el propio
# README nombraba. La cifra no envejece sola; envejece SIN AVISAR, que es lo que la
# vuelve peligrosa cuando alguien la lee como permiso.
#
# Por eso los documentos se leen desde el banco igual que un .cpp: por ruta, sin
# valor por defecto y abortando si faltan.

RAIZ_REPO = os.path.normpath(os.path.join(FIRMWARE, ".."))


def ruta_repo(*partes):
    """Como ruta(), pero desde la raiz del repositorio."""
    p = os.path.join(RAIZ_REPO, *partes)
    if not os.path.isfile(p):
        raise Abortado("no existe el documento %s" % os.path.join(*partes))
    return p


def texto_repo(*partes):
    with open(ruta_repo(*partes), "r", encoding="utf-8", errors="replace") as f:
        return f.read()


def actas():
    """Las actas de evidencia/, de la mas nueva a la mas vieja.

    Ordenadas por su NOMBRE, que lleva la fecha, y no por la marca de tiempo del
    sistema de ficheros: copiar el repositorio cambia las fechas de fichero y no
    cambia lo que el acta dice."""
    d = os.path.join(RAIZ_REPO, "evidencia")
    if not os.path.isdir(d):
        raise Abortado("no existe evidencia/: no hay ninguna acta contra la que "
                       "contrastar lo que publican los documentos")
    nombres = sorted((n for n in os.listdir(d) if n.endswith("_compuerta.txt")),
                     reverse=True)
    if not nombres:
        raise Abortado("evidencia/ no tiene ninguna acta *_compuerta.txt")
    return nombres


def acta(nombre):
    with open(os.path.join(RAIZ_REPO, "evidencia", nombre), "r",
              encoding="utf-8", errors="replace") as f:
        return f.read()


def fuentes_de(punta, carpeta, ext=".cpp"):
    """Los ficheros de una carpeta de una punta, censando el DIRECTORIO.

    Existe para que una comprobacion del tipo "nadie mas escribe este pin" no lleve
    una lista de ficheros escrita a mano: esa lista se queda corta el dia que alguien
    anade un .cpp, y entonces la prueba aprueba sin haber mirado donde hacia falta.

    N-73: admite extension porque el censo de funciones sin llamador necesita los .h.
    El valor por defecto se mantiene en .cpp para no cambiar el significado de las
    llamadas que ya existen -devolver de pronto .h a quien pedia .cpp haria que packs
    verdes empezaran a medir otra cosa sin que nadie lo pidiera-."""
    d = os.path.join(FIRMWARE, punta, carpeta)
    if not os.path.isdir(d):
        raise Abortado("no existe el directorio %s" % os.path.join(punta, carpeta))
    return sorted(n for n in os.listdir(d) if n.endswith(ext))


# ---------------------------------------------------------------------------------
# LA CONDICION DE LA PLUMA, QUE DESDE D-33 YA NO CABE EN UNA LINEA
# ---------------------------------------------------------------------------------
#
# Vive AQUI y no en un pack porque la necesitan TRES: barrera_03_talanquera lee las dos
# puntas, maestro_09_test_leds evalua la tabla de verdad y camara_03_vigilante mira quien
# la consulta. Tres copias de este parser serian el defecto que este fichero existe para
# no cometer -y la peor version de el, porque un parser que se queda viejo no da error:
# deja de encontrar y el pack aprueba una barrera que no ha mirado.


def bloque(codigo, i):
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


def bandera_publicada(codigo):
    """El nombre de la bandera que devuelve semaforo_plumaArriba(), leido del fuente.

    Es la que el $STATUS publica (N-153) y la que el ternario asigna: sin ella no se
    puede saber si la asignacion que envuelve la condicion es la legitima o cualquier
    otra que alguien haya metido por el camino."""
    m = re.search(r"bool\s+semaforo_plumaArriba\s*\([^)]*\)\s*\{([^}]*)\}", codigo)
    if not m:
        return None
    r = re.search(r"return\s+([A-Za-z_]\w*)\s*;", m.group(1))
    return r.group(1) if r else None


def apertura_de_la_pluma(cuerpo, publicada, local):
    """Reune la condicion de la pluma, que D-33 partio en dos el 14/09/2026.

    Hasta ese dia el ternario de MOTOR_TALANQUERA llevaba dentro la tabla de verdad
    entera y un `in` bastaba para auditarla. D-33 la saco a una cadena de tres ramas,
    porque el retardo de bajada y el veto de la camara necesitan saber si la pluma YA
    ESTABA arriba:

        const bool luzPideArriba = (verde && !testLedsActivo) || estado == S_FALLO;
        bool plumaArriba;
        if (luzPideArriba)        { ...; plumaArriba = true;  }   <- LA UNICA APERTURA
        else if (!plumaAbierta)   { ...; plumaArriba = false; }   <- ya abajo: se queda
        else                      { ...retardo y veto...      }   <- solo RETIENE
        digitalWrite(MOTOR_TALANQUERA, (plumaAbierta = plumaArriba) ? ABRIR : CERRAR);

    ESTO NO RELAJA NINGUNA COMPROBACION (CLAUDE.md 9). La tabla de verdad no
    desaparecio: se MUDO, y los packs siguen auditandola letra por letra sobre
    `tabla`. Lo que se anade es la pregunta que la forma nueva hace posible y la vieja
    no: `cierra` dice si la rama de en medio -pluma ya abajo, luz que no la pide- la
    deja abajo EN SECO. Si de ahi colgara el veto, una deteccion de camara LEVANTARIA
    la barrera con la luz en rojo, y la tabla de verdad de la apertura seguiria intacta:
    ninguna de las comprobaciones de antes de D-33 lo veria.

    Devuelve None si la condicion sigue escrita en linea -entonces no hay nada que
    seguir y el pack la audita como siempre- o si la cadena no tiene la forma de
    arriba, que es lo mismo que decir "no la entiendo": quien llama ABORTA, nunca
    aprueba."""
    if not re.fullmatch(r"[A-Za-z_]\w*", (local or "").strip()) or not publicada:
        return None
    local = local.strip()
    pone_true = re.compile(r"(?<![A-Za-z_])%s\s*=\s*true\s*;" % re.escape(local))
    pone_false = re.compile(r"(?<![A-Za-z_])%s\s*=\s*false\s*;" % re.escape(local))
    for m in re.finditer(r"if\s*\(\s*([A-Za-z_]\w*)\s*\)\s*\{", cuerpo):
        tramo = bloque(cuerpo, m.end() - 1)
        if not tramo or not pone_true.search(cuerpo[tramo[0]:tramo[1]]):
            continue
        guarda = m.group(1)
        ini = re.search(r"bool\s+%s\s*=\s*([^;]+);" % re.escape(guarda), cuerpo)
        if not ini:
            return None
        resto = cuerpo[tramo[1]:]
        me = re.match(r"\s*else\s+if\s*\(\s*!\s*%s\s*\)\s*\{" % re.escape(publicada),
                      resto)
        cierra = False
        if me:
            t2 = bloque(resto, me.end() - 1)
            if t2:
                cierra = bool(pone_false.search(resto[t2[0]:t2[1]]))
        return {"tabla": ini.group(1).strip(), "guarda": guarda, "cierra": cierra}
    return None


def sin_asignacion(expr, codigo):
    """Desenvuelve `(plumaAbierta = X)` y devuelve X. Solo si la bandera es la publicada.

    N-153: el valor que viaja en el campo PLUMA del $STATUS sale del MISMO parentesis
    que mueve el pin, para que no haya una copia de la formula al lado. Se acepta esa
    asignacion y NINGUNA otra: con cualquier otro identificador esto devuelve la
    expresion tal cual y quien llama la vera como no entendida, que es lo correcto."""
    m = re.match(r"^\(\s*([A-Za-z_]\w*)\s*=\s*(.+)\)$", expr.strip())
    if not m:
        return expr
    if m.group(1) != bandera_publicada(codigo):
        return expr
    return m.group(2).strip()
