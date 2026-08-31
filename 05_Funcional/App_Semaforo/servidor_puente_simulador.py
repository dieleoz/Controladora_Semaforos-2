#!/usr/bin/env python3
# -*- coding: ascii -*-
"""
===============================================================================
 CASCARA HTTP PARA MIRAR LA APP.  NO ES UN INSTRUMENTO.  NO ES UN SIMULADOR.
===============================================================================

QUE ES
  Sirve los ficheros de la app -index.html, app.js, js/, css/, style.css- en
  http://localhost:3000/ para poder ABRIRLA EN UN NAVEGADOR Y MEDIR LA
  INTERFAZ. Eso es todo lo que hace, y es lo unico que valia del fichero que
  habia aqui antes: con esta cascara se midio el desborde de la cabecera a
  cuatro anchos (412 / 390 / 360 / 320 px), que es como se vio que una captura
  a un solo ancho no demuestra nada.

  Ademas publica en /api/telemetria UNA trama $STATUS de muestra, COMPUESTA A
  PARTIR DEL FORMATO DEL C++ (ver "COMO SE COMPONE"), para poder pasarla por el
  parser real de la app y comprobar que los campos cuadran.

QUE NO ES  -- y esto importa mas que lo de arriba
  * NO es un instrumento. NO esta enganchado a compuerta.py y no debe estarlo.
    UN "PASS" SUYO NO EXISTE: este fichero no comprueba nada de nadie.
  * NO es un simulador del STM32. No tiene maquina de estados, no tiene ciclo,
    no tiene hilo. NADA AQUI SE MUEVE SOLO.
  * NO valida comandos. No conoce el PIN, ni los rangos de SET_TIEMPOS, ni
    ningun ACK. No contesta OK a nada.
  * NO da telemetria a la pantalla. Sin equipo delante, la app declara
    "SIN ENLACE" -y eso es la verdad, no una averia de esta cascara-.

POR QUE SE REESCRIBIO  (para que nadie lo restaure del historico creyendo que
                        valia: aqui esta medido por que no valia)
  El fichero anterior era una CUARTA copia a mano del STM32, y hablaba un
  protocolo que ninguna punta habla. Pasado por el parser REAL de la app
  (js/nmea_parser.js), su propia trama daba:

      lo que emitia el servidor:  ... RESTANTE:37  TOT:45   (y sin HORA)
      lo que emite el micro:      ... T:24                  HORA:18:25:00

      campos que se PERDIAN: ["hora", "restante"]

  La trama PASABA EL CHECKSUM, asi que la app la daba por buena y se quedaba
  sin la cuenta atras y sin el reloj -las dos cosas que mira el tecnico- SIN
  DECIR NADA, porque el parser ya no rellena valores por defecto (y hace bien).

  Y tres defectos mas de la misma familia:
    1. Contestaba "RESULT:OK" a SET_RTC incondicionalmente. Es exactamente la
       mentira con formato de exito que el pack app_03_sin_ok_mudo existe para
       prohibir.
    2. Llevaba su propia copia de los rangos de SET_TIEMPOS (1-15 / 10-90), que
       viven en modoAutomatico_fijarTiempos(). Dos copias de un limite es una
       que se queda vieja sin avisar.
    3. Arrancaba un hilo que animaba el ciclo solo. Un tablero que anima un
       cruce que no existe le miente a quien esta decidiendo sobre trafico
       mirandolo. Lo que sustituye a un dato que no se tiene no es una
       simulacion: es DECIRLO.

COMO SE COMPONE LA TRAMA  (el punto entero de la reescritura)
  El formato NO se escribe aqui: se EXTRAE del snprintf de $STATUS de
  01_Firmware/Maestro/src/bluetooth.cpp, y se rellenan sus conversiones en el
  mismo orden. Asi los nombres de campo y su orden no pueden divergir del
  firmware, que es como se colo el defecto de arriba.
  Si el formato no se puede extraer, ESTE FICHERO ABORTA con codigo 2 y no
  sirve nada. No inventa una trama.
===============================================================================
"""

import http.server
import json
import os
import re
import socketserver
import sys

AQUI = os.path.dirname(os.path.abspath(__file__))
# Ruta explicita al fuente. Si alguien mueve bluetooth.cpp, esto aborta en el
# arranque con el nombre del fichero delante, que es la unica forma de que un
# "no aparece" no se lea como "no hay nada que extraer".
BLUETOOTH_CPP = os.path.join(AQUI, "..", "..", "01_Firmware", "Maestro", "src", "bluetooth.cpp")


def abortar(motivo):
    # Codigo 2 = ABORTADO en el idioma de este repositorio: no pudo correr, y no
    # dice nada del firmware. Se sale ruidosamente en vez de servir una demo
    # inventada, que es justo lo que se esta retirando.
    sys.stderr.write("[ABORTADO] " + motivo + "\n")
    sys.exit(2)


def leer_fuente():
    try:
        with open(BLUETOOTH_CPP, "r", encoding="utf-8", errors="replace") as f:
            return f.read()
    except OSError as e:
        abortar("no se puede leer %s (%s)." % (BLUETOOTH_CPP, e))


def extraer(src, patron, que):
    m = re.search(patron, src)
    if not m:
        abortar("no se encuentra %s en %s. El formato de la trama sale de ahi o no "
                "sale: esta cascara no se lo inventa." % (que, BLUETOOTH_CPP))
    return m.group(1)


def guarda_checksum(src):
    # Un algoritmo no se puede "extraer" de un .cpp. Lo que si se puede es exigir
    # que el fuente siga diciendo lo que aqui abajo se reimplemento, y NEGARSE a
    # arrancar si dejo de decirlo: asi la reimplementacion no puede quedarse
    # vieja en silencio, que es como se cuelan estas cosas.
    for aguja, que in (("crc ^= (uint8_t)(*str);", "el XOR-8 byte a byte"),
                       ("calcularChecksum(payload + 1)", "el salto del '$' inicial"),
                       ('"%s*%02X\\r\\n"', "el cierre *XX CRLF")):
        if aguja not in src:
            abortar("%s ya no esta en enviarTramaConCrc(). El checksum del firmware "
                    "cambio de forma y esta cascara lo calcularia mal." % que)


def con_crc(payload):
    # Copia de enviarTramaConCrc(): XOR-8 del payload SALTANDO el '$', cierre *XX\r\n.
    crc = 0
    for b in payload[1:].encode("ascii"):
        crc ^= b
    return "%s*%02X\r\n" % (payload, crc)


CONVERSION = re.compile(r"%%|%[-0-9.]*(?:ll|l|h)?[sdiuxX]")


def componer(fmt, valores):
    # Sustituye cada conversion de printf EN ORDEN. Si sobran o faltan valores es
    # que el firmware cambio de campos: aborta en vez de emitir una trama corta.
    pendientes = list(valores)

    def uno(m):
        if m.group(0) == "%%":
            return "%"
        if not pendientes:
            abortar("el $STATUS del firmware trae mas campos de los que esta cascara "
                    "sabe rellenar. Actualice VALORES_MUESTRA.")
        return pendientes.pop(0)

    trama = CONVERSION.sub(uno, fmt)
    if pendientes:
        abortar("el $STATUS del firmware trae menos campos de los esperados: sobran %d "
                "valores de muestra." % len(pendientes))
    return trama


_SRC = leer_fuente()
guarda_checksum(_SRC)
FORMATO_STATUS = extraer(_SRC, r'"(\$STATUS,[^"]*)"', "el snprintf de $STATUS")
# El literal que el propio firmware emite cuando NO tiene hora. Se relee de ahi
# para no tener una segunda copia de como se dice "no se la hora".
SIN_HORA = extraer(_SRC, r'strncpy\(horaBuf,\s*"([^"]*)"', 'el literal de hora no valida')

# Valores de la muestra, en el orden del snprintf: SERIE, MODO, ESTADO, T, RF,
# RTT, HORA. NO SON TELEMETRIA: no hay equipo detras. Van marcados DEMO a
# proposito -quien lea la trama tiene que ver que no es un cruce- y los numeros
# van a cero porque cero es lo que se sabe. La hora usa el literal del firmware.
# (BAT no aparece: en el C++ es una constante dentro del propio formato.)
VALORES_MUESTRA = ["DEMO", "DEMO", "DEMO", "0", "0", "0", SIN_HORA]

TRAMA_STATUS = con_crc(componer(FORMATO_STATUS, VALORES_MUESTRA))
# Respuesta unica a cualquier comando. NO es un ACK: es el unico hecho real que
# esta cascara conoce -que no hay equipo al otro lado-, con la forma de $ERR del
# firmware (ERR,CMD:<que>,DESC:<motivo>).
TRAMA_ERR = con_crc("$ERR,CMD:PUENTE_DEMO,DESC:NO_HAY_EQUIPO_ESTO_ES_SOLO_LA_INTERFAZ")


class Cascara(http.server.SimpleHTTPRequestHandler):
    def __init__(self, *a, **kw):
        super().__init__(*a, directory=AQUI, **kw)

    def _cuerpo(self, texto, tipo):
        datos = texto.encode("ascii", "replace")
        self.send_response(200)
        self.send_header("Content-Type", tipo)
        self.send_header("Content-Length", str(len(datos)))
        self.end_headers()
        self.wfile.write(datos)

    def do_GET(self):
        if self.path.split("?")[0] == "/api/telemetria":
            self._cuerpo(TRAMA_STATUS, "text/plain; charset=us-ascii")
            return
        # NO existe /api/status_json, y su 404 es la respuesta CORRECTA: la app cae
        # a su .catch(), manda el watchdog y la pantalla declara SIN ENLACE. Servir
        # ahi un estado fabricado seria pintar un cruce inventado en los mismos
        # widgets que la telemetria real - el defecto que se acaba de retirar.
        super().do_GET()

    def do_POST(self):
        if self.path.split("?")[0] == "/api/cmd":
            # El cuerpo se lee y se TIRA sin registrarlo: los comandos de la app
            # llevan el PIN dentro (CMD:PIN:1234:...) y hacerle eco lo pintaria en
            # el registro de eventos de la pantalla.
            n = int(self.headers.get("Content-Length") or 0)
            if n:
                self.rfile.read(n)
            self._cuerpo(json.dumps({"resp_nmea": TRAMA_ERR.strip()}),
                         "application/json; charset=us-ascii")
            return
        self.send_error(404)


def arrancar(puerto):
    socketserver.ThreadingTCPServer.allow_reuse_address = True
    with socketserver.ThreadingTCPServer(("", puerto), Cascara) as httpd:
        print("=" * 78)
        print(" CASCARA DE DEMO - NO ES UN INSTRUMENTO, NO ESTA EN LA COMPUERTA")
        print("=" * 78)
        print("  App .............. http://localhost:%d/" % puerto)
        print("  Trama de muestra . http://localhost:%d/api/telemetria" % puerto)
        print("  Formato leido de . %s" % os.path.normpath(BLUETOOTH_CPP))
        print("    " + FORMATO_STATUS)
        print("  Emite ............ " + TRAMA_STATUS.strip())
        print("")
        print("  Sin ciclo, sin hilo, sin ACK. La app dira SIN ENLACE: es correcto,")
        print("  no hay equipo. Esta cascara sirve para MIRAR Y MEDIR LA INTERFAZ.")
        print("=" * 78)
        httpd.serve_forever()


if __name__ == "__main__":
    arrancar(int(sys.argv[1]) if len(sys.argv) > 1 else 3000)
