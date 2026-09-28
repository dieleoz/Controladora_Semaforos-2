# ===== entrega_apk.py =====
#
# La APK del paquete, para generar_entrega_v9_0.py: la mas reciente de 05_Funcional/,
# con el sufijo _SIN_BANCO y con su assets/public/ comprobado por CRC contra
# App_Semaforo/www/ y los tres extras de Capacitor. Sale de ese fichero el 28/09 en
# un commit de CORTE, sin cambio de comportamiento: el texto de abajo es el mismo,
# linea por linea, salvo las firmas. FUNCIONAL y Aborta los pasa quien llama, con
# esos mismos nombres, para que el cuerpo no cambie y el Aborta sea el suyo.

import os
import zipfile

# Los tres ficheros que Capacitor mete en `assets/public/` y NO vienen de `www/`.
# `bluetoothSerial.js` es el plugin por el que la app abre el socket SPP: si falta,
# la APK compila igual y arranca sin poder conectar por Bluetooth (skill 2.bis,
# sexta trampa). Se exige que esten, no se toleran como sobrantes.
EXTRAS_CORDOVA = (
    "cordova.js",
    "cordova_plugins.js",
    "plugins/cordova-plugin-bluetooth-serial/www/bluetoothSerial.js",
)


def _crc_de_bytes(datos):
    return zipfile.crc32(datos) & 0xFFFFFFFF


def _crc_de(ruta):
    return _crc_de_bytes(open(ruta, "rb").read())


def _fuente_web_del_repositorio(FUNCIONAL, Aborta):
    """CRC de TODO `App_Semaforo/www/`, no de tres ficheros elegidos a mano.

    La lista corta de tres -`app.js`, `index.html`, `style.css`- es la quinta trampa
    de la skill `entregar` 2.bis: deja fuera `manifest.json`, `sw.js`,
    `css/variables.css` y los SIETE `js/*.js` que `index.html` carga con siete
    `<script src=>`. Una lista envejece cada vez que la app gana un modulo y falla
    en silencio: la APK compila igual y arranca rota.
    """
    raiz = os.path.join(FUNCIONAL, "App_Semaforo", "www")
    if not os.path.isdir(raiz):
        raise Aborta("no existe %s: sin el fuente web no se puede comprobar que la "
                     "APK lleve dentro lo que su nombre promete" % raiz)
    fuente = {}
    for dirpath, _, ficheros in os.walk(raiz):
        for f in ficheros:
            ruta = os.path.join(dirpath, f)
            rel = os.path.relpath(ruta, raiz).replace(os.sep, "/")
            fuente[rel] = _crc_de(ruta)
    if not fuente:
        raise Aborta("%s esta vacio" % raiz)
    return fuente


def apk_verificada(FUNCIONAL, Aborta):
    """La APK mas reciente, Y con su contenido comprobado contra el repositorio.

    No basta con encontrar un .apk: la skill `entregar` seccion 2.bis lo deja medido -dos
    APK del mismo contenido NO tienen el mismo md5, asi que la comparacion util es
    por CRC de las entradas-. Si el fuente web del repositorio ha cambiado despues
    de compilarla, la APK del disco NO lleva lo que su nombre promete y el paquete
    NO sale: se recompila. Es la diferencia entre entregar una version y entregar
    un binario parecido.

    Y se comparan LOS TRECE ficheros de `www/`, no tres. Ver `_fuente_web_del_repositorio`.
    """
    candidatas = sorted(f for f in os.listdir(FUNCIONAL)
                        if f.startswith("IOT_VIAL_Semaforos_") and f.endswith(".apk"))
    if not candidatas:
        raise Aborta("no hay ninguna APK en 05_Funcional/. Compilala con la receta de "
                     "la skill `entregar` seccion 2.bis; el paquete no sale sin ella")
    nombre = candidatas[-1]
    ruta = os.path.join(FUNCIONAL, nombre)

    # El sufijo `_SIN_BANCO` viaja PEGADO al fichero y su ausencia se lee como
    # permiso (CLAUDE.md seccion 13). El 07/09 una APK lo perdio al renombrarse y el
    # parte publicaba su tamano, que no delataba nada.
    if "_SIN_BANCO" not in nombre:
        raise Aborta("la APK %s ha perdido el sufijo `_SIN_BANCO`.\n        Es la unica "
                     "marca que impide que alguien la suba a campo creyendola validada, "
                     "y este paquete no ha pasado banco" % nombre)

    prefijo = "assets/public/"
    with zipfile.ZipFile(ruta) as z:
        dentro = {i.filename[len(prefijo):]: i.CRC for i in z.infolist()
                  if i.filename.startswith(prefijo) and not i.filename.endswith("/")}

    faltan_extras = [e for e in EXTRAS_CORDOVA if e not in dentro]
    if faltan_extras:
        raise Aborta(
            "a la APK %s le faltan %s dentro de assets/public/.\n"
            "        No vienen de `www/`: los pone Capacitor, y `bluetoothSerial.js` es\n"
            "        por donde la app abre el socket SPP. Sin el la APK arranca y NO\n"
            "        conecta con ningun equipo. Recompilala sin borrar `assets/public/`."
            % (nombre, ", ".join(faltan_extras)))

    fuente = _fuente_web_del_repositorio(FUNCIONAL, Aborta)
    ausentes = sorted(f for f in fuente if f not in dentro)
    if ausentes:
        raise Aborta(
            "la APK %s no lleva %d fichero(s) del fuente web: %s.\n"
            "        Se copia el ARBOL entero -`cp -r www/. android/app/src/main/assets/public/`-,\n"
            "        no una lista de ficheros (skill `entregar` 2.bis, quinta trampa)."
            % (nombre, len(ausentes), ", ".join(ausentes)))

    desfasados = sorted(f for f in fuente if fuente[f] != dentro[f])
    if desfasados:
        raise Aborta(
            "la APK %s NO lleva el fuente que hay hoy en el repositorio: difieren %s.\n"
            "        Recompilala (skill `entregar` seccion 2.bis) y renombrala con la fecha y el\n"
            "        commit. Meterla asi entregaria una app sin los botones que el firmware\n"
            "        ya atiende, con un nombre que promete lo contrario."
            % (nombre, ", ".join(desfasados)))
    return nombre, ruta, len(fuente)
