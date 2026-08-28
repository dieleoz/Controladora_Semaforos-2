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
