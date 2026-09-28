# ===== entrega_zip.py =====
#
# Lo que generar_entrega_v9_0.py anade al zip y no sale de HEAD: el sello FW_HASH.txt de
# cada proyecto de firmware, y la carpeta a la que sale el paquete. Las funciones de git
# y la excepcion Aborta las pasa el empaquetador: una sola definicion, la suya.

import argparse
import os
import re


def sello_para_el_zip(desde_head, git, Aborta):
    """La linea de FW_HASH.txt: el hash de HEAD cortado al ancho que exige el sello.

    Sin git -el .zip-, el '!' de platformio.ini lee este fichero; sin el, el equipo
    contesta `FW:--`, que es lo que llego del campo el 28/09. El ancho se LEE de la
    orden del .ini en HEAD (`--short=N`) y no se escribe aqui: el ini descarta cualquier
    otro ancho, y `rev-parse --short` puede dar mas si el prefijo es ambiguo. Va SIN
    `SUCIO`: el fuente del zip sale de `git show HEAD:`, asi que ES ese commit aunque el
    arbol de quien empaqueta este sucio.
    """
    ini = desde_head("01_Firmware/Maestro/platformio.ini").decode("utf-8", "replace")
    m = re.search(r"--short=(\d+)", ini)
    if not m or "FW_HASH.txt" not in ini:
        raise Aborta("Maestro/platformio.ini en HEAD no lee FW_HASH.txt o no fija el "
                     "ancho del sello (--short=N): el firmware del zip saldria SIN SELLAR")
    return git("rev-parse", "HEAD")[:int(m.group(1))]


def sellar(z, versionado, sello, desde_head, Aborta):
    """EL SELLO DEL FIRMWARE, junto a cada platformio.ini que lo lee. Devuelve las rutas.

    Un FW_HASH.txt versionado se rechaza: saldria duplicado y con un hash que nadie
    recalcula.
    """
    sellados = []
    for rel, dentro in versionado:
        if dentro.endswith("/FW_HASH.txt"):
            raise Aborta("%s esta versionado: el sello lo escribe este script" % rel)
        if rel.startswith("01_Firmware/") and rel.endswith("/platformio.ini") \
                and b"FW_HASH.txt" in desde_head(rel):
            dentro_txt = dentro[:-len("platformio.ini")] + "FW_HASH.txt"
            z.writestr(dentro_txt, (sello + "\n").encode("ascii"))
            sellados.append(dentro_txt)
    if not sellados:
        raise Aborta("ningun platformio.ini del zip lee FW_HASH.txt: el firmware "
                     "compilado del paquete contestaria FW:--")
    return sellados


def comprobar(z, sellados, sello, Aborta):
    """Relee del zip cada FW_HASH.txt escrito. Devuelve la linea del resumen."""
    malos = [s for s in sellados if z.read(s) != (sello + "\n").encode("ascii")]
    if malos:
        raise Aborta("el sello del zip no es %s en: %s" % (sello, ", ".join(malos)))
    return ("     sello FW_HASH.txt = %s en %d proyectos: %s"
            % (sello, len(sellados), ", ".join(s.split("/")[1] for s in sellados)))


def destino_de_la_linea():
    """La carpeta de salida. Los paquetes salen FUERA del repo (CLAUDE.md 13); --destino
    es para probar."""
    ap = argparse.ArgumentParser()
    ap.add_argument("--destino", default=r"D:\@Proyect\Entregas_Semaforos")
    return ap.parse_args().destino


def ruta_del_zip(dir_destino, zip_name, Aborta):
    if not os.path.isdir(dir_destino):
        raise Aborta("no existe la carpeta de destino %s" % dir_destino)
    return os.path.join(dir_destino, zip_name)
