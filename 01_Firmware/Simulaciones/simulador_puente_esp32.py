#!/usr/bin/env python3
# ===== 01_Firmware/Simulaciones/simulador_puente_esp32.py =====
#
# SIMULADOR DEL LAZO   app  <->  SPP  <->  ESP32  <->  J17  <->  PB7/PB6  <->  STM32
#
# ---------------------------------------------------------------------------------
# QUE ES CODIGO REAL Y QUE ES MODELO. Se declara aqui arriba y no en una nota al pie,
# porque un simulador que no dice donde acaban sus bordes se lee como un permiso.
# ---------------------------------------------------------------------------------
#
#   PUNTA DE LA APP        CODIGO REAL. index.html, js/*.js y app.js corriendo en
#                          jsdom (puente_esp32/arnes_app.js). Los comandos los compone
#                          enviarComandoFirmware() de verdad, pulsando los botones de
#                          verdad; las tramas de vuelta las parsea parseNmeaTelemetry()
#                          de verdad y el tablero se repinta.
#                          MODELO dentro de esa punta: el troceado del plugin de
#                          Cordova -subscribe('\n', cb) entrega TROZOS, no bytes-.
#
#   EL ESP32               MODELO EN PYTHON, y es el unico que lo es por derecho: su
#                          C++ tiene cero lineas. Cuando exista, se compila igual que
#                          las otras dos puntas y este simulador se queda sin ningun
#                          modelo escrito a mano.
#
#   PUNTA DEL STM32        CODIGO REAL COMPILADO. bluetooth.cpp de las dos puntas mas
#                          semaforo.cpp, coordinador.cpp, modo_automatico.cpp,
#                          mando.cpp, modos.cpp, demanda.cpp e identidad.cpp
#                          (puente_esp32/arnes_puente.cpp + compilar.ps1). El que
#                          decide si un comando casa, si una linea se trunca, que
#                          checksum lleva la respuesta y que pin se mueve es el mismo
#                          .cpp que se carga en la tarjeta.
#                          SUSTITUIDOS ahi, con su motivo escrito en el arnes:
#                          reloj.cpp (arrastra STM32RTC y el HAL), modo_degradado
#                          (597 lineas con la pantalla dentro), menu, lcd, botones,
#                          protocolo, respaldo y Arduino.h/pines.h.
#
# POR QUE ESTA FORMA Y NO LA OBVIA. Hoy hay CUATRO copias del STM32 escritas a mano en
# este repositorio -simulador_sistema_v7_6.py, simulador_app_bluetooth.py,
# App_Semaforo/servidor_puente_simulador.py y el modelo del banco-, ninguna releida del
# C++. Una quinta habria demostrado que el quinto Python se comporta, no que el
# firmware lo haga. Es CLAUDE.md 8 en su forma literal.
#
# LO QUE MIDE, Y QUE NO MEDIA NADIE. Las dos puntas tienen instrumento por separado
# -el arnes de DOM ejercita la app, los packs leen el firmware-, y EN MEDIO no hay
# nada: el contrato de bytes del doc 18 3 vive justo ahi. Y de las dos direcciones,
# la de VUELTA -el equipo emite, el puente transporta, la app parsea y pinta- no la
# ejercia ningun instrumento de punta a punta. Es donde vive $EVENT.
#
# LAS CONSTANTES SE RELEEN DE LOS TRES LENGUAJES en cada corrida. Si alguna no se
# puede extraer, esto ABORTA. Sin valor por defecto, nunca.
#
# LA CUENTA ES CALCULADA, NUNCA LITERAL. simulador_app_bluetooth.py publica "5/5"
# escrito a mano, y por eso el detector de N-71 de la compuerta -que exige x == y
# sobre la cuenta publicada- no puede fallar jamas sobre el.
#
# Y NO SUSTITUYE LA PRUEBA DE BANCO. Es un PC.
#
# Codigos de salida: 0 PASS, 1 FALLA, 2 ABORTADO. ABORTADO NO ES PASS.

import atexit
import os
import re
import shutil
import subprocess
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from banco import fuente as fw  # noqa: E402

try:
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")
except Exception:
    pass

AQUI = os.path.dirname(os.path.abspath(__file__))
ARNESES = os.path.join(AQUI, "puente_esp32")
BUILD = os.path.join(ARNESES, "build")


# =====================================================================================
# EL CONTADOR
#
# No se reutiliza banco/contador.py aunque se le parezca, y el motivo es concreto: la
# compuerta detecta el fallo de un instrumento suelto por la marca literal "[FALLA]" y
# por la cuenta x/y de la ultima linea. Los packs imprimen "FALLA " sin corchetes
# porque quien los agrega es correr.py; aqui no hay agregador, asi que la marca la
# tiene que poner este fichero o la compuerta se fia solo del codigo de salida, que es
# exactamente el defecto de N-46.
# =====================================================================================

class Contador:
    def __init__(self):
        self.total = 0
        self.pasadas = 0
        self.fallos = []
        self.hallazgos = []

    def titulo(self, t):
        print("\n-- %s %s" % (t, "-" * max(0, 74 - len(t))))

    def verificar(self, condicion, msg_ok, msg_mal):
        self.total += 1
        if condicion:
            self.pasadas += 1
            print("   [OK]     %s" % msg_ok)
        else:
            self.fallos.append(msg_mal)
            print("   [FALLA]  %s" % msg_mal)
        return bool(condicion)

    def control_negativo(self, detecta, que):
        """Exige que la comprobacion SEPA FALLAR. Sin esto, una prueba que aprueba
        todo se lee como cobertura: es la prueba muerta de N-51."""
        return self.verificar(
            detecta,
            "control negativo: %s - la prueba distingue el caso malo" % que,
            "CONTROL NEGATIVO ROTO: %s. La comprobacion aprueba tambien el caso "
            "defectuoso, asi que su PASS no vale nada" % que)

    def propiedad(self, condicion, msg_ok, msg_roto):
        """Propiedad de seguridad que el simulador INTENTA ROMPER - y a veces rompe.

        Se marca ROTA y no FALLA a proposito, porque no significan lo mismo: FALLA se
        lee como 'el instrumento esta mal'; ROTA dice lo que de verdad ocurre -el
        escenario existe, se reprodujo contra el firmware real, y no lo resiste-. La
        cuenta baja igual y el codigo de salida cambia igual: disimularlo seria dar
        PASS a algo que se ha visto romperse."""
        self.total += 1
        if condicion:
            self.pasadas += 1
            print("   [OK]     %s" % msg_ok)
        else:
            self.fallos.append("PROPIEDAD ROTA: " + msg_roto)
            print("   [FALLA]  PROPIEDAD ROTA: %s" % msg_roto)
        return bool(condicion)

    def reportar(self, titulo, lineas):
        """Hallazgo que NO cuenta como comprobacion.

        Aqui va una pregunta que ningun firmware puede contestar porque nadie la ha
        decidido. Contarla seria un FALLA permanente, y un codigo de salida que jamas
        cambia ensena a ignorarlo (CLAUDE.md 3)."""
        self.hallazgos.append((titulo, lineas))
        print("   >>>      %s" % titulo)
        for l in lineas:
            print("            %s" % l)


# =====================================================================================
# LO QUE SE RELEE DEL FUENTE. SI FALTA, ABORTA.
# =====================================================================================

PUNTAS = ("Maestro", "Esclavo")

# Rutas como tuplas explicitas para que la guarda de rutas de compuerta.py las censE:
# si manana alguien mueve bluetooth.cpp, la guarda aborta en vez de dejar a este
# simulador midiendo un fuente que ya no esta (N-36).
FUENTES = (
    ("Maestro", "src", "bluetooth.cpp"),
    ("Esclavo", "src", "bluetooth.cpp"),
    ("Maestro", "src", "modo_automatico.cpp"),
    ("Maestro", "include", "protocolo.h"),
)

APP_JS = ("05_Funcional", "App_Semaforo", "app.js")
PARSER_JS = ("05_Funcional", "App_Semaforo", "js", "nmea_parser.js")


def _uno(texto, patron, que, donde, grupo=1):
    m = re.search(patron, texto)
    if not m:
        raise fw.Abortado(
            "no se pudo leer %s en %s (patron %r). Sin ese dato el simulador mediria "
            "otra cosa que el firmware y seguiria dando PASS." % (que, donde, patron))
    return m.group(grupo)


def _exige(texto, patron, que, donde):
    """Como _uno(), pero para lo que no lleva cifra: la FORMA de un bloque.

    Existe separado a proposito. Una guarda o un bucle no se leen como un numero: o
    estan escritos tal cual o el contrato es otro, y darlos por buenos "porque se
    parecen" es como se cuela un modelo que mide un firmware que ya no existe."""
    if not re.search(patron, texto):
        raise fw.Abortado(
            "no se encontro %s en %s (patron %r). El contrato que este simulador "
            "ejerce da por hecho esa forma: si cambio, hay que actualizar el modelo, "
            "no ignorarlo." % (que, donde, patron))


class Contrato:
    """El contrato de bytes, leido de los tres lenguajes. Nada escrito a mano."""

    def __init__(self):
        self.buffer, self.util, self.baudios, self.pines = {}, {}, {}, {}
        self.periodo_ms, self.envoltorio = {}, {}
        self.prefijo_pin, self.largo_pin = {}, {}
        self.sin_pin, self.acciones, self.prefijos_salida = {}, {}, {}
        self.codigo = {}

        for p in PUNTAS:
            self._leer_punta(p)

        self.sfty6_ms = int(_uno(
            fw.texto("Maestro", "include", "protocolo.h"),
            r"#define\s+SFTY6_SILENCIO_MS\s+(\d+)UL",
            "el umbral de silencio de radio SFTY-6", "Maestro/include/protocolo.h"))

        self.timeout_app_ms = int(_uno(
            fw.texto_repo(*APP_JS),
            r"const\s+TIMEOUT_ENLACE_MS\s*=\s*(\d+)\s*;",
            "el plazo con el que la app declara el enlace perdido",
            "App_Semaforo/app.js"))

        # LOS RANGOS DE SET_TIEMPOS, RELEIDOS. Sin esto, el simulador tendria que
        # escribir "30,30,15" a mano y el firmware lo rechazaria por rango -el verde
        # va en MINUTOS, no en segundos-. Ya paso: F7 media una rafaga que nunca
        # ocurria porque el comando que la provoca estaba mal formado, y el escenario
        # acusaba al firmware de no emitir un $EVENT que no tenia por que emitir.
        auto = fw.codigo("Maestro", "src", "modo_automatico.cpp")
        self.rangos = {}
        for nombre in ("VERDE_MIN", "ROJO_MIN", "DESPEJE_SEG"):
            lo = int(_uno(auto, r"%s_MIN\s*=\s*(\d+)" % nombre,
                          "el minimo de %s" % nombre, "Maestro/src/modo_automatico.cpp"))
            hi = int(_uno(auto, r"%s_MAX\s*=\s*(\d+)" % nombre,
                          "el maximo de %s" % nombre, "Maestro/src/modo_automatico.cpp"))
            self.rangos[nombre] = (lo, hi)

    def tiempos_validos(self):
        """Un SET_TIEMPOS que el ciclo REAL acepta, con los rangos leidos del C++."""
        return ",".join(str((lo + hi) // 2) for lo, hi in
                        (self.rangos["VERDE_MIN"], self.rangos["ROJO_MIN"],
                         self.rangos["DESPEJE_SEG"]))

    def _leer_punta(self, punta):
        ruta = (punta, "src", "bluetooth.cpp")
        donde = "%s/src/bluetooth.cpp" % punta
        txt, cod = fw.texto(*ruta), fw.codigo(*ruta)
        self.codigo[punta] = cod

        # --- E-2: el buffer y su guarda -------------------------------------------
        n = int(_uno(cod, r"static\s+char\s+btBufIn\[(\d+)\]\s*;",
                     "el tamano de btBufIn", donde))
        self.buffer[punta] = n
        # La guarda no se da por supuesta: se exige el texto literal. Si alguien la
        # cambiara por "<= sizeof(btBufIn)" el limite util dejaria de ser n-1, y este
        # simulador tiene que enterarse en vez de deducirlo.
        _exige(cod, r"btIdxIn\s*<\s*sizeof\(btBufIn\)\s*-\s*1",
               "la guarda de desbordamiento del buffer de entrada", donde)
        self.util[punta] = n - 1

        self.baudios[punta] = int(_uno(cod, r"SerialBT\.begin\((\d+)\)",
                                       "la velocidad del puerto", donde))
        m = re.search(r"HardwareSerial\s+SerialBT\((P[A-Z]\d+)\s*,\s*(P[A-Z]\d+)\)", cod)
        if not m:
            raise fw.Abortado("no se pudieron leer los pines de SerialBT en %s" % donde)
        self.pines[punta] = (m.group(1), m.group(2))

        self.periodo_ms[punta] = int(_uno(
            cod, r"ahora\s*-\s*tUltimaTelemetria\s*>=\s*(\d+)",
            "la cadencia de $STATUS", donde))

        # --- S-1 / S-2: el envoltorio de salida ------------------------------------
        # Tres piezas por separado: cada una es una regla distinta del doc 18 3.3 y
        # romper cualquiera cambia lo que el ESP32 tiene que leer.
        _exige(cod, r"while\s*\(\*str\s*&&\s*\*str\s*!=\s*'\*'\)",
               "el bucle del checksum, que tiene que PARAR en el '*'", donde)
        _exige(cod, r"crc\s*\^=\s*\(uint8_t\)\(\*str\)",
               "el XOR-8 del checksum", donde)
        _exige(cod, r"calcularChecksum\(payload\s*\+\s*1\)",
               "el salto del '$' inicial al calcular el checksum", donde)
        self.envoltorio[punta] = _uno(
            txt, r'snprintf\(tramaCompleta,\s*sizeof\(tramaCompleta\),\s*"([^"]+)"',
            "el formato de la trama de salida", donde)

        # --- 3.5: la barrera de PIN, releida ---------------------------------------
        m = re.search(r'strncmp\(cmd,\s*"(CMD:PIN:\d+:)",\s*(\d+)\)', cod)
        if not m:
            raise fw.Abortado("no se pudo leer el prefijo de PIN en %s" % donde)
        self.prefijo_pin[punta] = m.group(1)
        self.largo_pin[punta] = int(m.group(2))

        # --- 3.6: el censo de comandos, extraido del fuente -------------------------
        cuerpo = self._cuerpo_despachador(cod, donde)

        sin_pin = list(re.findall(r'strcmp\(cmd,\s*"([^"]+)"\)', cuerpo))

        # AB-1 (04/09): el latido NO es un comando exento de PIN, es una LINEA RESERVADA.
        # Este censo lee los strcmp(cmd, ...) que hay antes de la guarda de PIN, y el
        # latido esta ahi -tiene que estarlo, para salir sin actuar y sin contestar-, asi
        # que entraba como si fuera una orden. Las comprobaciones de N3 exigen despues que
        # el firmware CONTESTE algo, y el latido no contesta a proposito: contestar cada
        # dos segundos seria el mismo ruido que el $ERR que esta rama evita.
        sin_pin = [x for x in sin_pin if x != "$LATIDO"]
        for c in re.findall(r'strcmp\(cmd\s*\+\s*4,\s*"([^"]+)"\)', cuerpo):
            sin_pin.append("CMD:" + c)
        if not sin_pin:
            raise fw.Abortado(
                "el censo no hallo ni un comando comparado contra `cmd` en %s: fallo "
                "el buscador, no el firmware" % donde)
        self.sin_pin[punta] = sin_pin

        acciones = []
        for m in re.finditer(r'str(n?)cmp\(accion,\s*"([^"]+)"(?:,\s*(\d+))?\)', cuerpo):
            acciones.append((m.group(2), m.group(1) == "", m.start()))
        if not acciones:
            raise fw.Abortado(
                "el censo no hallo ni una accion tras la barrera de PIN en %s" % donde)

        # QUE CONTESTA CADA RAMA, LEIDO DEL BLOQUE Y NO DE MEMORIA. Es la tecnica de
        # app_03_sin_ok_mudo. Sirve para la asimetria de N4 -TEST_LEDS lo acepta el
        # Maestro y lo RECHAZA el Esclavo- sin lista escrita a mano, que es como se
        # queda vieja. N-89 al lado: si un compositor mudara esos literales a otro
        # fichero, esto dejaria de verlos; por eso lleva control negativo.
        cortes = [a[2] for a in acciones] + [len(cuerpo)]
        self.acciones[punta] = [
            (lit, exacto, self._clases(cuerpo[ini:cortes[i + 1]]))
            for i, (lit, exacto, ini) in enumerate(acciones)]

        emitidos = set(re.findall(r'"(\$[A-Z]+)', cod))

        # AB-1 (04/09): $LATIDO se descuenta porque las puntas NO lo emiten, lo RECIBEN.
        # Este censo busca literales '$...' en el fuente y no sabe distinguir uno que se
        # ENVIA de uno que se COMPARA; el latido solo aparece en el strcmp del despachador
        # -la linea reservada que el puente manda y que se ignora sin contestar-, asi que
        # entraba como si fuera una trama de salida y el simulador abortaba pidiendo un
        # ejemplo para retransmitirla a la app. A la app no llega nunca: muere en el
        # despachador del STM32.
        #
        # Se descuenta por su literal exacto y no por una regla general: una trama de
        # SALIDA nueva tiene que seguir abortando este censo hasta que alguien le de un
        # ejemplo, que es justo lo que impide "medir solo los que ya conocia".
        emitidos.discard("$LATIDO")

        if len(emitidos) < 3:
            raise fw.Abortado(
                "el censo solo hallo %d prefijos de salida en %s: fallo el buscador"
                % (len(emitidos), donde))
        self.prefijos_salida[punta] = emitidos

    @staticmethod
    def _clases(bloque):
        c = set()
        if '"$ACK' in bloque:
            c.add("ACK")
        if '"$ERR' in bloque:
            c.add("ERR")
        return c

    @staticmethod
    def _cuerpo_despachador(cod, donde):
        i = cod.find("static void procesarComando(")
        if i < 0:
            raise fw.Abortado(
                "no se encontro procesarComando() en %s: sin el despachador no hay "
                "contrato que ejercer" % donde)
        j = cod.find("void bluetooth_loop", i)
        return cod[i:j if j > 0 else len(cod)]


def checksum(payload_sin_dolar):
    """XOR-8, saltando el '$' y PARANDO en el '*'. Las dos condiciones se releen del
    C++ arriba; aqui se reproducen para poder FABRICAR tramas de prueba. Las que el
    firmware emite las calcula el firmware, no esto."""
    crc = 0
    for ch in payload_sin_dolar:
        if ch == "*":
            break
        crc ^= ord(ch)
    return crc


def envolver(c, punta, payload):
    fmt = c.envoltorio[punta]
    if fmt != "%s*%02X\\r\\n":
        raise fw.Abortado(
            "el envoltorio de salida de %s es %r y este simulador solo sabe fabricar "
            "'%%s*%%02X\\\\r\\\\n'. Cambiar el formato cambia lo que el ESP32 parsea: "
            "se actualiza el modelo, no se ignora" % (punta, fmt))
    return "%s*%02X\r\n" % (payload, checksum(payload[1:]))


# =====================================================================================
# LAS DOS PUNTAS REALES, COMO PROCESOS
# =====================================================================================

# Los arneses son procesos hijos, y un hijo huerfano SE QUEDA CON EL .exe ABIERTO: el
# siguiente enlazado falla con "Permission denied" y parece un fallo del toolchain que
# no lo es. Si este proceso muere por donde sea -un timeout, un Ctrl-C-, se van con el.
_VIVOS = []


@atexit.register
def _matar_huerfanos():
    for p in _VIVOS:
        if p.poll() is None:
            p.kill()


class Proceso:
    """Un arnes vivo. Todo lo que entra y sale va en hexadecimal a proposito: el
    contrato de bytes incluye los terminadores, y una consola que los interpretara los
    perderia por el camino."""

    def __init__(self, orden, nombre, cwd=None):
        self.nombre = nombre
        try:
            self.p = subprocess.Popen(
                orden, stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                stderr=subprocess.DEVNULL, text=True, bufsize=1,
                encoding="utf-8", errors="replace", cwd=cwd)
        except OSError as e:
            raise fw.Abortado("no se pudo lanzar %s: %s" % (nombre, e))
        _VIVOS.append(self.p)
        self.saludo = self.p.stdout.readline().strip()
        if self.saludo.startswith("ABORT") or not self.saludo.startswith("LISTO"):
            raise fw.Abortado("%s no arranco: %r" % (nombre, self.saludo))

    def pedir(self, orden):
        """Manda una orden y devuelve (lineas TX/DOM, linea final OK/ERR)."""
        if self.p.poll() is not None:
            raise fw.Abortado("%s murio antes de %r" % (self.nombre, orden))
        self.p.stdin.write(orden + "\n")
        self.p.stdin.flush()
        eventos = []
        while True:
            l = self.p.stdout.readline()
            if not l:
                raise fw.Abortado("%s cerro la salida durante %r" % (self.nombre, orden))
            l = l.strip()
            if l.startswith("OK") or l.startswith("ERR") or l.startswith("ABORT"):
                return eventos, l
            eventos.append(l)

    def tx(self, orden):
        """Lo que la punta escribio por su canal serie, TRAMA A TRAMA.

        Se parte por el terminador y no se devuelve el volcado entero, y eso no es
        cosmetica: una vuelta de bluetooth_loop() puede imprimir $ACK y $EVENT en la
        misma tanda, y treinta $STATUS caben en un solo volcado. La primera version
        contaba eso como UNA trama y F5 daba 1 donde tocaban 30. El instrumento medi a
        volcados y el escenario hablaba de tramas: el que estaba mal era el buscador."""
        eventos, _ = self.pedir(orden)
        crudo = "".join(bytes.fromhex(e[3:]).decode("latin-1")
                        for e in eventos if e.startswith("TX "))
        return [t + "\r\n" for t in crudo.split("\r\n") if t]

    def cerrar(self):
        try:
            self.p.stdin.write("QUIT\n")
            self.p.stdin.flush()
            self.p.wait(timeout=5)
        except Exception:
            self.p.kill()


def hexa(s):
    return "".join("%02X" % ord(ch) for ch in s)


class Stm32Real:
    """La punta STM32: bluetooth.cpp compilado, no una copia."""

    def __init__(self, punta):
        exe = os.path.join(BUILD, "arnes_%s.exe" % punta.lower())
        if not os.path.isfile(exe):
            raise fw.Abortado(
                "no existe %s. El arnes del STM32 no esta compilado, asi que este "
                "simulador no mide el firmware: es ABORTADO, no PASS. Se construye "
                "con puente_esp32/compilar.ps1" % os.path.basename(exe))
        self.pr = Proceso([exe], "arnes del STM32 (%s)" % punta)
        self.punta = punta
        self.entregas = 0

    def rx(self, datos):
        """Mete bytes en la cola del SerialBT real y corre bluetooth_loop()."""
        self.entregas += 1
        return self.pr.tx("RXHEX " + hexa(datos))

    def avanzar(self, ms):
        return self.pr.tx("MS %d" % ms)

    def pines(self):
        eventos, ok = self.pr.pedir("PINS")
        linea = ok if ok.startswith("PINS") else next(
            (e for e in eventos if e.startswith("PINS")), "")
        return dict(re.findall(r"(\w+)=(-?\d+)", linea))

    def dato(self, orden, patron):
        eventos, ok = self.pr.pedir(orden)
        for l in eventos + [ok]:
            m = re.search(patron, l)
            if m:
                return m.group(1)
        return None

    def ajustar(self, orden):
        self.pr.pedir(orden)

    def cerrar(self):
        self.pr.cerrar()


class AppReal:
    """La punta de la app: index.html + js/*.js + app.js en jsdom."""

    def __init__(self):
        node = shutil.which("node")
        if node is None:
            raise fw.Abortado(
                "no hay node en el PATH. Sin el, la punta de la app no se puede "
                "ejercer con su propio codigo y habria que imitarla: ABORTADO")
        self.pr = Proceso([node, os.path.join(ARNESES, "arnes_app.js")],
                          "arnes de la app (jsdom)")

    def pulsar_id(self, elemento):
        return self.pr.tx("BTN " + elemento)

    def pulsar_cmd(self, orden):
        return self.pr.tx("CMD " + orden)

    def entregar(self, datos):
        """Entrega bytes por el canal serie y devuelve el tablero que la app pinto."""
        eventos, ok = self.pr.pedir("RXHEX " + hexa(datos))
        dom = next((e[4:] for e in eventos if e.startswith("DOM ")), "{}")
        import json
        return json.loads(dom), ok

    def tablero(self):
        eventos, _ = self.pr.pedir("DOM")
        import json
        return json.loads(next((e[4:] for e in eventos if e.startswith("DOM ")), "{}"))

    def depuracion(self):
        """Lo que el tecnico lee al abrir la pestana de depuracion de la app REAL."""
        eventos, _ = self.pr.pedir("DEPU")
        return next((e[5:] for e in eventos if e.startswith("DEPU ")), "")

    def juzgar(self, linea):
        """El veredicto de NMEAParser.validarTrama() REAL, sin entregar nada al canal.

        Sirve para comparar la regla de CRC de la app con la del puente. Desde el
        31/08 hay DOS capas que validan el mismo checksum: si no juzgan igual, existe
        una trama que una deja pasar y la otra tira -o nadie tira, o el operario lee
        dos motivos distintos de la misma trama-."""
        eventos, _ = self.pr.pedir("JUZGAR " + hexa(linea))
        import json
        crudo = next((e[7:] for e in eventos if e.startswith("JUICIO ")), None)
        if crudo is None:
            raise fw.Abortado(
                "el arnes de la app no devolvio veredicto para %r. Sin el, la "
                "comparacion de las dos reglas de CRC no se puede hacer y este "
                "simulador estaria publicando una cuenta que no midio" % linea[:40])
        return json.loads(crudo)

    def cerrar(self):
        self.pr.cerrar()


# =====================================================================================
# EL PUENTE ESP32 — LO QUE SE ESTA ESPECIFICANDO
#
# Su C++ tiene cero lineas: esto es su pliego, ejecutable. Cada regla lleva escrito de
# donde sale, porque quien escriba el firmware las va a discutir una a una y "lo dice
# el simulador" no es una razon.
# =====================================================================================

MARCA_PROPIA = "ORIGEN:ESP32"


class Puente:
    def __init__(self, util_max):
        self.util_max = util_max
        self.buf_app = ""
        self.buf_stm = ""
        self.hacia_stm32 = []   # una entrada por ESCRITURA, no por trama
        self.hacia_app = []
        self.rechazos_crc = 0
        self.rechazos_largo = 0
        self.crc_malos_del_cable = 0

    # ---- app -> STM32 --------------------------------------------------------
    def desde_app(self, datos):
        self.buf_app += datos
        while True:
            pos = [p for p in (self.buf_app.find("\n"), self.buf_app.find("\r")) if p >= 0]
            if not pos:
                return
            i = min(pos)
            linea, self.buf_app = self.buf_app[:i].strip(), self.buf_app[i + 1:]
            if not linea:
                continue                 # E-3: una linea vacia no es nada
            self._del_telefono(linea)

    def _del_telefono(self, linea):
        # 3.4: EL ESP32 SI VALIDA. Medido en el fuente: no hay una sola llamada a
        # calcularChecksum() en el camino de recepcion del STM32, y parseNmeaTelemetry()
        # de la app tampoco valida nada. Si el puente no mira el checksum, no lo mira
        # NADIE en toda la cadena.
        util = self._validar(linea)
        if util is None:
            self.rechazos_crc += 1
            self._responder("$ERR,CMD:CHECKSUM,DESC:TRAMA_DESCARTADA_EN_EL_PUENTE")
            return
        # F3 / E-2: se mide ANTES de reenviar. El STM32, si se lo mandan, no protesta:
        # trunca y compara, y una linea truncada puede casar con un prefijo valido.
        if not self._cabe(util):
            self.rechazos_largo += 1
            self._responder("$ERR,CMD:LONGITUD,DESC:EXCEDE_%d_%s"
                            % (self.util_max, MARCA_PROPIA))
            return
        self._entregar(util)

    @staticmethod
    def _validar(linea):
        """Devuelve el comando desnudo, o None si la trama no es de fiar.

        Acepta tambien la linea SIN envoltorio NMEA, porque es lo que la app manda
        hoy de verdad -enviarComandoFirmware() no pone checksum-. Esa asimetria no la
        decide este fichero: esta MEDIDA y queda formulada como pregunta abierta al
        final del simulador."""
        if not linea.startswith("$"):
            return linea if "*" not in linea else None
        if "*" not in linea:
            return None
        cuerpo, _, cola = linea[1:].partition("*")
        if len(cola) < 2:
            return None
        try:
            recibido = int(cola[:2], 16)
        except ValueError:
            return None
        return cuerpo if recibido == checksum(cuerpo) else None

    def _cabe(self, util):
        """F3 - LA MEDIDA DE LONGITUD, EN UNA LINEA A PROPOSITO.

        Va aislada para que romperla sea un cambio de una linea: es una de las dos
        inyecciones de 8.bis con las que se comprobo que este simulador sabe caer. Lo
        que cierra no es el rechazo de un comando largo: es que el STM32 TRUNCA EN
        SILENCIO y la linea cortada sigue casando con las ramas de strncmp
        -SET_TIEMPOS: y SET_RTC:-, de modo que el equipo obedece una orden con los
        parametros mutilados sin decir nada."""
        return len(util) <= self.util_max

    def _entregar(self, util):
        """UNA sola escritura, con terminador. 6.3: el puente no parte ni une."""
        self.hacia_stm32.append(util + "\n")

    # ---- STM32 -> app --------------------------------------------------------
    def desde_stm32(self, datos):
        self.buf_stm += datos
        while "\r\n" in self.buf_stm:
            linea, _, self.buf_stm = self.buf_stm.partition("\r\n")
            if not linea:
                continue
            # 3.4, sentido de vuelta: el ruido de cable no sube al telefono. Y aqui
            # no hay red detras: parseNmeaTelemetry() de la app NO valida el checksum
            # -NMEAParser.validarTrama() existe y no la llama nadie-, asi que una
            # trama corrupta que suba se pinta en el tablero como si fuera buena.
            if self._validar(linea) is None:
                self.crc_malos_del_cable += 1
                continue
            self._subir(linea + "\r\n")

    def _subir(self, linea):
        """F2/F7 - UNA TRAMA, UNA ESCRITURA, EN UNA LINEA A PROPOSITO.

        Aislada por lo mismo que _cabe(): es la otra inyeccion de 8.bis. No es mania
        de estilo: la app no lee bytes, lee los TROZOS que le recorta el plugin de
        Cordova por el delimitador '\\n'. Dos tramas pegadas sin terminador entre
        medias llegan como un solo trozo, y parseNmeaTelemetry() se queda con lo que
        hay antes del primer '*': la segunda desaparece sin que nada avise."""
        self.hacia_app.append(linea)

    def _responder(self, payload):
        """P-3 / 3.4: cuando el puente rechaza algo, lo DICE, y lo dice como suyo.

        Un $ERR del puente tiene que ser distinguible de uno del STM32, o el tecnico
        que lee el telefono no sabe a cual de los dos equipos culpar."""
        cuerpo = payload[1:] + "," + MARCA_PROPIA
        self.hacia_app.append("$%s*%02X\r\n" % (cuerpo, checksum(cuerpo)))


def comandos_censados(c, punta):
    """Todo lo que el despachador de esa punta atiende, tal como llega por el cable."""
    lista = list(c.sin_pin[punta])
    for literal, exacto, _clases in c.acciones[punta]:
        # Los de strncmp llevan parametro detras, y tiene que ser uno que el firmware
        # ACEPTE: con un valor fuera de rango el comando cae en $ERR y el escenario
        # mediria un rechazo creyendo medir una aceptacion. Los rangos se releen del
        # C++, no se escriben aqui.
        cola = "" if exacto else (c.tiempos_validos() if "TIEMPOS" in literal
                                  else "2026-08-31,23:59:59")
        lista.append(c.prefijo_pin[punta] + literal + cola)
    return lista


def clase_de(linea):
    """De que clase es una trama de vuelta, mirando su prefijo."""
    return linea.split(",")[0] if linea.startswith("$") else "?"


def sin_comentarios_js(texto):
    """El mismo texto con los comentarios fuera. Devuelve blancos en su sitio para que
    los indices sigan valiendo -aqui se compara el ORDEN de dos patrones-.

    HACE FALTA, Y NO ES CELO. app.js documenta el defecto que acaba de arreglar citando
    el codigo viejo: dentro de parseNmeaTelemetry() hay un comentario que dice
    "Antes aqui habia ... line.split('*')[0], que TIRA EL CHECKSUM SIN MIRARLO". Buscar
    esa huella sin quitar los comentarios la encuentra EN LA PROSA y acusa al fichero de
    conservar lo que precisamente explica haber quitado. Es CLAUDE.md 4 otra vez: el
    buscador respondia, y aun asi no sabia encontrar."""
    fuera = []
    i, n = 0, len(texto)
    while i < n:
        if texto.startswith("//", i):
            j = texto.find("\n", i)
            j = n if j < 0 else j
            fuera.append(" " * (j - i))
            i = j
        elif texto.startswith("/*", i):
            j = texto.find("*/", i + 2)
            j = n if j < 0 else j + 2
            fuera.append("".join(" " if ch != "\n" else "\n" for ch in texto[i:j]))
            i = j
        else:
            fuera.append(texto[i])
            i += 1
    return "".join(fuera)


def cuerpo_js(texto, nombre):
    """El cuerpo de una funcion de app.js, acotado por la SIGUIENTE del mismo nivel.

    Se acota a proposito, y no se busca el patron en el fichero entero: `app.js` habla
    de sus propias funciones en los comentarios -"NMEAParser.validarTrama() llevaba
    meses escrita"-, asi que un `in app_js` a secas casa con la PROSA y da por buena una
    llamada que no existe. Es la regla del instrumento (CLAUDE.md 4) aplicada a un
    fichero que se documenta a si mismo: el buscador respondia, y aun asi no sabia
    distinguir codigo de comentario."""
    m = re.search(r"\n(\s*)function %s\(" % re.escape(nombre), texto)
    if not m:
        raise fw.Abortado(
            "no se encontro function %s() en app.js. Este simulador mide el ORDEN de "
            "las barreras dentro de esa funcion: si cambio de nombre o de sitio, hay "
            "que actualizar el instrumento, no ignorarlo." % nombre)
    ini = m.end()
    sig = re.search(r"\n%sfunction \w+\(" % m.group(1), texto[ini:])
    return texto[ini:ini + sig.start()] if sig else texto[ini:]


def _contadores_app(texto):
    """(tramas rechazadas, rechazos por CHECKSUM) tal como los publica la pestana.

    Se lee el TEXTO que ve el tecnico y no la estructura de datos de dentro: lo que no
    llega a la pantalla no lo puede usar nadie delante de un poste."""
    n = re.search(r"(\d+) rechazadas", texto)
    k = re.search(r"CHECKSUM x(\d+)", texto)
    return (int(n.group(1)) if n else 0), (int(k.group(1)) if k else 0)


def _acks_de(salidas):
    """TODOS los comandos que los $ACK de esa tanda confirman.

    Hace falta la lista y no el primero: al purgar el buffer con un terminador suelto
    salen DOS $ACK -el del huerfano, que se dispara al vaciarse, y el del comando
    nuevo- y quedarse con el primero mediria el del huerfano y concluiria que la purga
    no sirve. Es la regla del instrumento: el buscador miraba una sola respuesta donde
    habia dos."""
    salida = []
    for s in salidas:
        if s.startswith("$ACK"):
            m = re.search(r"\$ACK,CMD:([^,*]+)", s)
            salida.append(m.group(1) if m else "?")
    return salida


def _ack_de(salidas):
    """El comando que un $ACK confirma, o None si no hubo $ACK.

    Existe para poder preguntar lo unico que importa en F1: el equipo ha confirmado
    ESTA orden, o ha confirmado OTRA? Contar "hubo $ACK" a secas no distingue una
    aceptacion legitima de una que celebra un comando que nadie mando."""
    for s in salidas:
        if s.startswith("$ACK"):
            m = re.search(r"\$ACK,CMD:([^,*]+)", s)
            return m.group(1) if m else "?"
    return None


# =====================================================================================
# ESCENARIOS
# =====================================================================================

def escenario_contrato(t, c, maestro, esclavo, app):
    t.titulo("El contrato releido de los tres lenguajes, y los arneses vivos")

    t.verificar(
        c.buffer["Maestro"] == c.buffer["Esclavo"],
        "btBufIn mide %d B en las dos puntas -> %d caracteres utiles (E-2)"
        % (c.buffer["Maestro"], c.util["Maestro"]),
        "btBufIn mide %d B en el Maestro y %d en el Esclavo: el ESP32 no puede servir "
        "a las dos con el mismo limite" % (c.buffer["Maestro"], c.buffer["Esclavo"]))

    t.verificar(
        c.baudios["Maestro"] == c.baudios["Esclavo"],
        "las dos puntas abren SerialBT a %d bps" % c.baudios["Maestro"],
        "las puntas divergen en velocidad: Maestro %d, Esclavo %d. El mismo ESP32 no "
        "puede servir a las dos sin recompilar"
        % (c.baudios["Maestro"], c.baudios["Esclavo"]))

    # EL ARNES CONFIRMA LO QUE EL FUENTE DICE, ejecutandolo. No es redundante: el
    # objeto SerialBT es `static` y se busca POR SUS PINES, asi que si alguien lo
    # moviera a otro par el arnes no lo encontraria. Que lo encuentre y que ademas
    # traiga el baudrate que el .cpp declara es la prueba de que se esta midiendo ESE
    # puerto y no otro (N-86: dos objetos sobre el mismo periferico dan el ultimo).
    for punta, pr in (("Maestro", maestro), ("Esclavo", esclavo)):
        baud = pr.dato("PUERTO", r"baud=(\d+)")
        n = pr.dato("PUERTO", r"n=(\d+)")
        t.verificar(
            baud == str(c.baudios[punta]) and n == "1",
            "%s: el binario abre UN solo HardwareSerial, en los pines de J17 y a %s "
            "bps, que es lo que declara el .cpp" % (punta, baud),
            "%s: el arnes encontro %s puerto(s) y el que casa con J17 va a %s bps "
            "cuando el fuente dice %d. O SerialBT se movio, o hay dos objetos sobre "
            "el mismo periferico" % (punta, n, baud, c.baudios[punta]))

    # bluetooth_setup() REAL apaga el receptor de U2 para liberar el puerto. Se mide
    # sobre el pin que el firmware escribio, no sobre el comentario que lo explica.
    dere = maestro.pines().get("DERE")
    t.verificar(
        dere == "1",
        "bluetooth_setup() real deja RS485_IN_DE_RE en HIGH: el receptor de U2 queda "
        "apagado y el puerto libre para el ESP32",
        "RS485_IN_DE_RE quedo en %r tras bluetooth_setup(): si el receptor de U2 sigue "
        "encendido, esa linea pelea con el ESP32 por el mismo par" % dere)

    t.verificar(
        "delimitador=0A" in app.pr.saludo,
        "la app REAL se suscribe al canal serie con delimitador '\\n': es ELLA quien "
        "fija el troceado, no el arnes",
        "la app se suscribio con %r. El delimitador decide como llegan las tramas al "
        "parser; si cambia, F2 mide otra cosa" % app.pr.saludo)

    return c.util["Maestro"]


def escenario_n1(t, c, maestro, app, util_max):
    t.titulo("N1 - un comando legitimo de punta a punta, con las dos puntas reales")

    puente = Puente(util_max)

    # LA APP LO COMPONE. Se pulsa su boton de verdad; nadie fabrica la cadena aqui.
    compuestos = app.pulsar_id("btn-op-auto")
    t.verificar(
        len(compuestos) == 1 and compuestos[0].endswith("\r\n"),
        "la app REAL compone la orden al pulsar su boton: %r"
        % (compuestos[0] if compuestos else None),
        "la app no escribio una sola orden terminada al pulsar AUTOMATICO: %r. Sin "
        "eso no hay nada que llevar al equipo" % (compuestos,))

    puente.desde_app(compuestos[0])
    t.verificar(
        len(puente.hacia_stm32) == 1 and puente.rechazos_crc == 0,
        "el puente la acepta y la entrega en UNA escritura",
        "el puente no entrego la orden: %d escrituras, %d rechazos por checksum"
        % (len(puente.hacia_stm32), puente.rechazos_crc))

    entregado = puente.hacia_stm32[0]
    t.verificar(
        entregado.endswith("\n"),
        "lo entregado acaba en terminador: E-1 cumplida, el despachador dispara",
        "lo entregado no acaba en \\r ni en \\n: sin terminador el despachador no "
        "dispara NUNCA y el comando se queda mudo en el buffer")

    # EL FIRMWARE REAL LO DESPACHA.
    salidas = maestro.rx(entregado)
    acks = [s for s in salidas if s.startswith("$ACK")]
    t.verificar(
        len(acks) == 1 and "SET_MODO:AUTO" in acks[0],
        "procesarComando() REAL acusa: %r" % (acks[0].strip() if acks else None),
        "el firmware no contesto un $ACK de SET_MODO:AUTO: %r" % (salidas,))

    # Y EL EFECTO, que es lo que un $ACK no demuestra. El modo del equipo cambia.
    modo = maestro.dato("MODO", r"MODO (\d+)")
    t.verificar(
        modo is not None and modo != "0",
        "y el EFECTO existe: el modo vigente del equipo paso a %s, no solo contesto"
        % modo,
        "el firmware contesto $ACK y el modo siguio en %r. Un $ACK que no mueve nada "
        "es una mentira con formato de exito" % modo)

    # LA VUELTA: la app REAL parsea lo que el firmware compuso.
    for s in salidas:
        puente.desde_stm32(s)
    # Se mira el tablero DESPUES DE CADA trama y no solo al final: una vuelta de
    # bluetooth_loop() manda $ACK y $EVENT, asi que el ultimo apunte del registro es
    # el segundo. Quedarse con el final media la trama equivocada.
    vistos = []
    for w in puente.hacia_app:
        tablero, _ = app.entregar(w)
        vistos.append(tablero.get("ultimo") or "")
    t.verificar(
        any("ACEPTADA" in v.upper() for v in vistos),
        "la app REAL pinta la confirmacion del equipo: %r"
        % next((v for v in vistos if "ACEPTADA" in v.upper()), None),
        "el $ACK llego al telefono y la app no lo pinto: %r. El operario ve 'orden "
        "enviada' -que es lo que sabe la app- y nunca 'orden aceptada'" % (vistos,))


def escenario_n2(t, c, maestro, app, util_max):
    t.titulo("N2 - las CINCO tramas de vuelta, y la app las tiene que pintar")

    emitidos = c.prefijos_salida["Maestro"] | c.prefijos_salida["Esclavo"]
    t.verificar(
        len(emitidos) == 5,
        "el censo del C++ halla %d prefijos de salida: %s"
        % (len(emitidos), " ".join(sorted(emitidos))),
        "el censo halla %d prefijos (%s) y el contrato del doc 18 3.7 dice cinco. Un "
        "puente que filtre por una lista vieja se come lo que sobra"
        % (len(emitidos), " ".join(sorted(emitidos))))

    ejemplos = {
        "$STATUS": "$STATUS,NODE:MAESTRO,SERIE:A0EF0D,MODO:AUTO,ESTADO:V1_R2,T:38,"
                   "RF:98%,RTT:82ms,BAT:12.6,HORA:18:25:00",
        "$ACK": "$ACK,CMD:TEST_LEDS,RESULT:STARTING_6S",
        "$ERR": "$ERR,CMD:DESCONOCIDO,DESC:COMANDO_NO_SOPORTADO",
        "$ALARM": "$ALARM,NODE:MAESTRO,EVENTO:ORFANDAD,CAUSA:SILENCIO_25000ms,"
                  "ACCION:AMBAR,HORA:18:25:00",
        "$EVENT": "$EVENT,NODE:MAESTRO,ORIGEN:APP_BLUETOOTH,DETALLE:SET_MODO_AUTO,"
                  "HORA:18:25:00",
    }
    faltan = sorted(emitidos - set(ejemplos))
    if faltan:
        raise fw.Abortado(
            "el C++ emite %s y este simulador no tiene ejemplo para ese prefijo: "
            "medir solo los que ya conocia seria aprobar sin mirar" % ", ".join(faltan))

    puente = Puente(util_max)
    for pref in sorted(emitidos):
        puente.desde_stm32(envolver(c, "Maestro", ejemplos[pref]))
    t.verificar(
        len(puente.hacia_app) == len(emitidos),
        "el puente sube las %d clases sin filtrar ninguna" % len(emitidos),
        "el puente subio %d de %d clases: la que falta desaparece del telefono sin "
        "dejar rastro" % (len(puente.hacia_app), len(emitidos)))

    # Y LA APP REAL TIENE QUE HACER ALGO CON CADA UNA. Se cuenta lo que su propio
    # registro de eventos crece: si una clase no la maneja nadie, el contador no sube.
    antes = app.tablero()["eventos"]
    crecimiento = {}
    for w in puente.hacia_app:
        previo = app.tablero()["eventos"]
        tablero, _ = app.entregar(w)
        crecimiento[clase_de(w)] = tablero["eventos"] - previo
    mudas = [k for k, v in crecimiento.items() if k != "$STATUS" and v <= 0]
    t.verificar(
        not mudas,
        "la app REAL registra en su bitacora las cuatro clases de aviso "
        "(%s); $STATUS repinta el tablero sin apuntar linea, que es lo correcto"
        % ", ".join("%s+%d" % (k, v) for k, v in sorted(crecimiento.items())
                    if k != "$STATUS"),
        "la app no apunta nada al recibir %s: esa clase de trama llega y se pierde"
        % ", ".join(mudas))

    t.verificar(
        crecimiento.get("$EVENT", 0) > 0,
        "$EVENT llega al telefono y la app lo pinta: la bitacora del equipo no se "
        "pierde -catorce ramas del Maestro la emiten-",
        "$EVENT no dejo rastro en la app. Un puente o un parser que la filtre se come "
        "la bitacora entera, que es lo que costo N-73")

    # El caso que el doc 18 3.7 senala: un puente escrito contra una lista de CUATRO.
    lista_vieja = sorted(emitidos - {"$EVENT"})
    colados = [w for w in puente.hacia_app if clase_de(w) in lista_vieja]
    t.control_negativo(
        len(colados) < len(puente.hacia_app),
        "un puente que filtrara por los cuatro prefijos de la lista de partida dejaria "
        "fuera %d trama(s)" % (len(puente.hacia_app) - len(colados)))

    t.verificar(
        app.tablero()["eventos"] > antes,
        "y el tablero de la app quedo con mas lineas que antes de la rafaga",
        "la app termino con las mismas lineas que antes: no oyo nada de lo que subio")


def escenario_n3(t, c, maestro, esclavo, app, util_max):
    t.titulo("N3 - los comandos exentos de PIN pasan sin que el puente pida nada")

    total = 0
    for punta, pr in (("Maestro", maestro), ("Esclavo", esclavo)):
        for literal in c.sin_pin[punta]:
            total += 1
            puente = Puente(util_max)
            puente.desde_app(literal + "\r\n")
            entregado = puente.hacia_stm32[0] if puente.hacia_stm32 else ""
            salidas = pr.rx(entregado) if entregado else []
            # Se exige que NO caiga en AUTH_FAILED. Que conteste $ACK o $ERR depende
            # de la rama -el FORZAR_ROJO del Esclavo esta renombrado a proposito- y
            # eso lo mide N4; aqui lo que importa es que la barrera no lo pare.
            auth = [s for s in salidas if "AUTH_FAILED" in s]
            t.verificar(
                len(puente.hacia_stm32) == 1 and not auth and salidas,
                "%s: %r entra sin PIN y el firmware contesta %s"
                % (punta, literal, (salidas[0].split(",")[0] if salidas else "?")),
                "%s: %r fue parado (%d entregas, salidas %r). Una caida segura que "
                "pide clave no es una caida segura"
                % (punta, literal, len(puente.hacia_stm32), salidas))

    if total < 3:
        raise fw.Abortado(
            "el censo solo hallo %d comandos exentos de PIN y el doc 18 3.5 mide "
            "cuatro: fallo el buscador, no el firmware" % total)

    # Y LA APP REAL LOS MANDA SIN PIN, que es la otra mitad del contrato: si la app
    # los envolviera en CMD:PIN: el firmware los rechazaria.
    sin_pin_app = app.pulsar_id("btn-op-emergency")
    t.verificar(
        sin_pin_app and "PIN" not in sin_pin_app[0],
        "la app REAL manda el rojo de emergencia SIN PIN: %r"
        % (sin_pin_app[0].strip() if sin_pin_app else None),
        "la app envolvio el rojo de emergencia con PIN (%r): delante de un accidente "
        "la caida segura necesitaria recordar una clave" % (sin_pin_app,))

    exento = c.sin_pin["Esclavo"][0]
    t.control_negativo(
        not exento.startswith(c.prefijo_pin["Esclavo"]),
        "un puente que exigiera el prefijo %r a todo bloquearia %r, que el firmware "
        "exime a proposito" % (c.prefijo_pin["Esclavo"], exento))


def escenario_n4(t, c, maestro, esclavo, util_max):
    t.titulo("N4 - la asimetria entre puntas: el puente transporta bytes, no normaliza")

    for accion in ("TEST_LEDS", "FORZAR_ROJO"):
        linea = c.prefijo_pin["Maestro"] + accion
        respuestas = {}
        for punta, pr in (("Maestro", maestro), ("Esclavo", esclavo)):
            puente = Puente(util_max)
            puente.desde_app(linea + "\r\n")
            salidas = pr.rx(puente.hacia_stm32[0]) if puente.hacia_stm32 else []
            respuestas[punta] = [s.strip() for s in salidas if s.startswith("$")]

        m = [s for s in respuestas["Maestro"] if s.startswith(("$ACK", "$ERR"))]
        e = [s for s in respuestas["Esclavo"] if s.startswith(("$ACK", "$ERR"))]
        t.verificar(
            m and e and m[0].startswith("$ACK") and e[0].startswith("$ERR"),
            "%s: los MISMOS bytes por el mismo puente -> Maestro %r / Esclavo %r"
            % (accion, m[0][:40] if m else None, e[0][:60] if e else None),
            "%s dejo de ser asimetrico: Maestro %r, Esclavo %r. Si las dos puntas "
            "contestan igual, o cambio el firmware o el puente normalizo algo que era "
            "deliberado" % (accion, m, e))

    # El motivo del rechazo lo pone el firmware, y tiene que llegar entero: es lo que
    # el tecnico lee para saber que hacer en su lugar.
    puente = Puente(util_max)
    puente.desde_app(c.prefijo_pin["Esclavo"] + "FORZAR_ROJO\r\n")
    salidas = esclavo.rx(puente.hacia_stm32[0])
    err = next((s for s in salidas if s.startswith("$ERR")), "")
    t.verificar(
        "RENOMBRADO" in err,
        "y el rechazo trae su motivo entero: %r" % err.strip(),
        "el $ERR del Esclavo no explica por que: %r. Un rechazo sin motivo manda al "
        "tecnico a adivinar" % err)

    t.control_negativo(
        c.acciones["Esclavo"] != c.acciones["Maestro"],
        "el censo distingue las dos listas de comandos: no son la misma y el puente "
        "no puede unificarlas")


def escenario_f1(t, c, maestro, util_max):
    t.titulo("F1 - el puente se cuelga a mitad de trama, y el HUERFANO se concatena")

    # Media linea SIN terminador: el bucle receptor REAL no dispara.
    salidas = maestro.rx(c.prefijo_pin["Maestro"] + "FORZAR_ROJ")
    t.verificar(
        not salidas,
        "media linea sin terminador NO dispara el despachador real (E-1)",
        "media linea produjo %r: E-1 dice que sin terminador el comando se queda mudo "
        "en el buffer" % (salidas,))

    # EL COROLARIO, QUE ES LO QUE MUERDE: btBufIn no se limpia. Cuando el puente
    # vuelve, el siguiente comando se pega detras del huerfano y produce una linea que
    # SI acaba en \n. Se exige que caiga en DESCONOCIDO y no que un strcmp acierte.
    salidas = maestro.rx(c.prefijo_pin["Maestro"] + "TEST_LEDS\n")
    t.verificar(
        any("DESCONOCIDO" in s for s in salidas),
        "el comando concatenado con el huerfano cae en $ERR,CMD:DESCONOCIDO - no casa "
        "por accidente",
        "la linea concatenada produjo %r en vez de DESCONOCIDO: un strcmp acerto por "
        "accidente sobre una orden que nadie dio" % (salidas,))

    # Y NO VALE CON UN CASO. Se barre el censo entero contra si mismo sobre el
    # FIRMWARE REAL. La propiedad exigible no es "que caiga en DESCONOCIDO" -un $ERR de
    # cualquier motivo es un rechazo, y rechazar es seguro-: es que NINGUNA linea
    # concatenada produzca un $ACK. Un $ACK sobre una concatenacion dice dos mentiras a
    # la vez -confirma una orden que nadie dio en esa forma, y tapa que la orden que el
    # operario acaba de pulsar se ha perdido-.
    censo = comandos_censados(c, "Maestro")
    pares, aceptados, motivos = 0, [], {}
    for huerfano in censo:
        for siguiente in censo:
            pares += 1
            maestro.rx("\n")                        # buffer limpio antes de cada par
            maestro.rx(huerfano)                    # se queda a medias
            salidas = maestro.rx(siguiente + "\n")  # y se pega el siguiente
            ack = _ack_de(salidas)
            motivos["ACK" if ack else ("ERR" if any(s.startswith("$ERR")
                                                    for s in salidas) else "MUDO")] = \
                motivos.get("ACK" if ack else ("ERR" if any(s.startswith("$ERR")
                                                            for s in salidas)
                                               else "MUDO"), 0) + 1
            # LA PROPIEDAD SE MIDE COMPARANDO EL $ACK CON LA ORDEN QUE SE MANDO. Un
            # $ACK no es malo por si mismo; lo que no puede pasar es que confirme un
            # comando que NO es el que el operario acaba de pulsar.
            if ack and ack not in siguiente:
                aceptados.append((huerfano, siguiente, ack,
                                  [x.strip() for x in salidas][:1]))

    t.propiedad(
        not aceptados,
        "barridos los %d pares huerfano+comando contra el firmware REAL: ningun $ACK "
        "confirma una orden distinta de la que se mando (%s)"
        % (pares, ", ".join("%s=%d" % kv for kv in sorted(motivos.items()))),
        "%d de %d pares huerfano+comando producen un $ACK que confirma OTRA orden "
        "(%s). El primero: se pulso %r con %r a medias en el buffer, y el equipo "
        "contesta %r.\n"
        "        LA CAUSA, MEDIDA: las ramas que casan por strncmp -SET_RTC:, "
        "SET_TIEMPOS:- parsean con sscanf, y sscanf NO exige que la cadena se acabe: "
        "lee sus campos y se desentiende de la basura pegada detras. El equipo ejecuta "
        "el comando VIEJO y CONTESTA OK, mientras el que el operario acaba de pulsar "
        "-que puede ser CMD:FORZAR_ROJO- se pierde sin un solo aviso."
        % (len(aceptados), pares,
           ", ".join("%s=%d" % kv for kv in sorted(motivos.items())),
           aceptados[0][1] if aceptados else None,
           aceptados[0][0] if aceptados else None,
           aceptados[0][3] if aceptados else None))

    # LA MITIGACION QUE SI ESTA EN MANO DEL PUENTE, medida y no propuesta.
    #
    # Un puente que anteponga un terminador suelto a cada comando CIERRA el huerfano en
    # vez de dejar que se pegue al siguiente. Lo que ocurre entonces, medido y no
    # supuesto: el huerfano se despacha COMO SU PROPIA LINEA -que es la orden que el
    # operario mando de verdad, solo que tarde- y el comando nuevo se despacha detras.
    # No es que el huerfano desaparezca: es que deja de contaminar al siguiente.
    #
    # La propiedad exigible es la que de verdad importa: LA ORDEN QUE EL OPERARIO ACABA
    # DE PULSAR RECIBE SU PROPIO $ACK. Hoy se pierde en silencio.
    #
    # Y se acepta como respuesta valida tanto un $ACK como un $ERR que NOMBRE ese
    # comando: un rechazo razonado -"EN_MARCHA_PARE_EL_MODO"- es una respuesta, y la
    # propiedad que se vigila es que la orden no se pierda CALLANDO, no que se acepte.
    # Exigir $ACK habria mezclado el defecto con el estado en que quedo el equipo tras
    # los comandos anteriores del propio barrido.
    con_purga = []
    for huerfano, siguiente, _ack, _ in aceptados:
        maestro.rx("\n")
        maestro.rx(huerfano)
        salidas = maestro.rx("\n" + siguiente + "\n")   # el puente cierra primero
        respondidos = [m.group(1) for s in salidas
                       for m in [re.search(r"\$(?:ACK|ERR),CMD:([^,*]+)", s)] if m]
        if any(r in siguiente for r in respondidos):
            con_purga.append((huerfano, siguiente))
    t.verificar(
        not aceptados or len(con_purga) == len(aceptados),
        "y con el puente anteponiendo UN terminador suelto, los %d pares rotos pasan a "
        "responder POR SU PROPIA ORDEN: el huerfano se despacha aparte y el comando "
        "nuevo recibe su respuesta. La mitigacion cuesta 1 byte" % len(aceptados),
        "la purga con terminador suelto solo recupera %d de %d pares: no es la "
        "mitigacion completa y hay que decirlo ANTES de escribirla en la "
        "especificacion" % (len(con_purga), len(aceptados)))

    # Control negativo: el barrido tiene que saber ver un $ACK legitimo. Se le da un
    # caso que SI acierta -un comando entero detras de un buffer vacio- y se exige que
    # lo distinga del concatenado.
    maestro.rx("\n")
    limpio = maestro.rx(c.prefijo_pin["Maestro"] + "TEST_LEDS\n")
    t.control_negativo(
        any(s.startswith("$ACK") for s in limpio),
        "el mismo comando, SIN huerfano delante, si produce $ACK: el barrido mide la "
        "concatenacion y no un rechazo universal")

    if aceptados:
        t.reportar(
            "QUIEN ARREGLA LA CONCATENACION: hay DOS candidatos y no es decision de un "
            "instrumento",
            ["(a) EL FIRMWARE. Las ramas de sscanf pueden exigir que la linea se",
             "    consuma entera -sscanf devuelve cuantos campos leyo, no si sobro",
             "    texto-. Es la cura de raiz y vale para cualquier fuente de bytes,",
             "    incluido quien pinche un hilo en J17 p2 sin pasar por el ESP32.",
             "(b) EL PUENTE. Anteponer un terminador suelto a cada comando cierra el",
             "    huerfano por 1 byte, y arriba esta medido que funciona. Pero solo",
             "    protege lo que entra POR EL PUENTE, y tiene un efecto que hay que",
             "    escribir al lado: el huerfano NO se tira, se despacha como su propia",
             "    linea. O sea que la orden que se quedo a medias se ejecuta TARDE, y",
             "    el operario puede haber cambiado de idea entre una y otra.",
             "",
             "Las dos son compatibles y no se excluyen. Cual entra, y en que orden, lo",
             "decide el responsable: (a) toca firmware de campo y (b) toca una",
             "especificacion que todavia no tiene codigo detras."])


def escenario_f2(t, c, maestro, app, util_max):
    t.titulo("F2 - partir y unir tramas: las dos rompen a la app")

    entera = envolver(c, "Maestro", "$STATUS,NODE:MAESTRO,SERIE:A0EF0D,MODO:AUTO,"
                                    "ESTADO:V1_R2,T:38,RF:98%,RTT:82ms,BAT:12.6,"
                                    "HORA:18:25:00")

    # (a) EL PUENTE NO PARTE. Se le da la trama en dos pedazos por el cable -que es lo
    #     que hace un UART- y se exige que salga en UNA sola escritura.
    puente = Puente(util_max)
    puente.desde_stm32(entera[:5])
    puente.desde_stm32(entera[5:])
    t.verificar(
        len(puente.hacia_app) == 1,
        "una trama que llega partida por el cable sale del puente entera, en UNA "
        "escritura",
        "el puente produjo %d escritura(s) para una sola trama: partir '$STAT' + "
        "'US,NODE:...' produce dos lineas que no son nada" % len(puente.hacia_app))

    tablero, _ = app.entregar(puente.hacia_app[0])
    t.verificar(
        tablero["contador"] == "38",
        "y la app REAL la pinta: el contador del tablero quedo en %r"
        % tablero["contador"],
        "la app no pinto la trama reunida: contador %r. Lo que el puente entrega tiene "
        "que ser lo que la app sabe leer" % tablero["contador"])

    # (b) LO QUE PASA SI SE PARTE DE VERDAD, medido sobre la app REAL. Se le entrega
    #     media trama CON terminador -que es lo que produce un puente que trocea- y se
    #     exige que el tablero NO se mueva: media trama no es un dato.
    antes = app.tablero()
    app.entregar(entera[:20] + "\r\n")
    despues = app.tablero()
    t.verificar(
        despues["contador"] == antes["contador"],
        "media trama con terminador NO mueve el tablero: la app no pinta un dato que "
        "no recibio (contador sigue en %r)" % despues["contador"],
        "media trama cambio el tablero de %r a %r: la app esta pintando campos de una "
        "trama incompleta" % (antes["contador"], despues["contador"]))

    # (c) EL PUENTE NO UNE. Dos tramas seguidas tienen que llegar como DOS.
    puente = Puente(util_max)
    a = envolver(c, "Maestro", "$STATUS,NODE:MAESTRO,ESTADO:V1_R2,T:11")
    bb = envolver(c, "Maestro", "$STATUS,NODE:MAESTRO,ESTADO:R1_V2,T:22")
    puente.desde_stm32(a + bb)
    t.verificar(
        len(puente.hacia_app) == 2,
        "dos $STATUS seguidos salen como DOS escrituras",
        "el puente produjo %d escritura(s) para dos tramas" % len(puente.hacia_app))

    for w in puente.hacia_app:
        tablero, _ = app.entregar(w)
    t.verificar(
        tablero["contador"] == "22",
        "y la app REAL ve las dos: su contador acabo en la ULTIMA (%r)"
        % tablero["contador"],
        "la app acabo en %r y la ultima trama decia 22: se perdio una" % tablero["contador"])

    # CONTROL NEGATIVO SOBRE LA APP REAL: se le da la union CRUDA -sin terminador
    # entre medias, que es lo que produce un puente que agrupa- y se exige que la
    # segunda trama SE PIERDA. Si no se perdiera, la comprobacion de arriba no mide.
    app.entregar(envolver(c, "Maestro", "$STATUS,NODE:MAESTRO,ESTADO:V1_R2,T:11")
                 .replace("\r\n", "")
                 + envolver(c, "Maestro", "$STATUS,NODE:MAESTRO,ESTADO:R1_V2,T:99"))
    unido = app.tablero()
    t.control_negativo(
        unido["contador"] != "99",
        "dos tramas pegadas sin terminador llegan como UN solo trozo y la app se queda "
        "con la primera: la segunda desaparece (contador %r, no 99)"
        % unido["contador"])


def escenario_f3(t, c, maestro, util_max):
    t.titulo("F3 - el comando de 64+ que el STM32 trunca EN SILENCIO")

    # Se construye sobre una rama de strncmp, que es donde el truncado es peligroso: la
    # linea cortada SIGUE CASANDO con la rama y lo que cambia son los PARAMETROS.
    prefijo = next((lit for lit, exacto, _ in c.acciones["Maestro"] if not exacto), None)
    if prefijo is None:
        raise fw.Abortado(
            "el censo no hallo ni una rama de strncmp en el Maestro: sin una rama que "
            "case por prefijo no se puede ejercer el truncado peligroso de F3")

    base = c.prefijo_pin["Maestro"] + prefijo
    largo = base + "9" * (util_max + 8 - len(base))
    if len(largo) <= util_max:
        raise fw.Abortado(
            "el comando de prueba mide %d y el limite es %d: no excede, asi que F3 no "
            "mediria nada" % (len(largo), util_max))

    # (a) SIN puente: el firmware REAL trunca y aun asi contesta. No es hipotesis.
    maestro.rx("\n")
    salidas = maestro.rx(largo + "\n")
    respuesta = next((s.strip() for s in salidas if s.startswith("$")), "")
    t.verificar(
        respuesta and "DESCONOCIDO" not in respuesta,
        "medido sobre el firmware REAL: un comando de %d B llega cortado a %d y AUN "
        "ASI casa con la rama %r -> %r. El exceso se pierde sin un solo aviso"
        % (len(largo), util_max, prefijo, respuesta[:60]),
        "el comando de %d B no reprodujo el truncado peligroso: contesto %r. Si no se "
        "reproduce, F3 no esta midiendo lo que dice medir" % (len(largo), respuesta))

    # (b) CON puente: se mide antes de reenviar, no se reenvia, y el $ERR es SUYO.
    puente = Puente(util_max)
    puente.desde_app(largo + "\r\n")
    t.verificar(
        puente.rechazos_largo == 1 and len(puente.hacia_stm32) == 0,
        "el puente mide ANTES de reenviar: rechaza y el STM32 no ve un solo byte",
        "el puente dejo pasar el comando largo: %d rechazos, %d entregas"
        % (puente.rechazos_largo, len(puente.hacia_stm32)))

    propia = puente.hacia_app[0] if puente.hacia_app else ""
    literales_cpp = re.findall(r'"(\$ERR[^"]*)"', c.codigo["Maestro"]) \
        + re.findall(r'"(\$ERR[^"]*)"', c.codigo["Esclavo"])
    t.verificar(
        MARCA_PROPIA in propia and not any(MARCA_PROPIA in l for l in literales_cpp),
        "el $ERR del rechazo lleva %r y ninguno de los %d literales $ERR del C++ lo "
        "lleva: el tecnico sabe que fue el puente y no el equipo"
        % (MARCA_PROPIA, len(literales_cpp)),
        "el $ERR del puente no se distingue de uno del STM32 (%r). Un rechazo sin "
        "firma manda al tecnico a revisar el equipo equivocado" % propia)

    # EL MARGEN DE HOY ES CASUALIDAD DE LOS LITERALES DE HOY, y por eso se mide.
    censo = comandos_censados(c, "Maestro") + comandos_censados(c, "Esclavo")
    mas_largo = max(censo, key=len)
    t.verificar(
        len(mas_largo) <= util_max,
        "el comando mas largo del censo mide %d B y el limite es %d: caben, con %d B "
        "de margen" % (len(mas_largo), util_max, util_max - len(mas_largo)),
        "el comando mas largo del censo mide %d B y el limite es %d: YA NO CABE. El "
        "firmware lo truncaria y lo compararia como si estuviera completo (%r)"
        % (len(mas_largo), util_max, mas_largo))

    # Control negativo sobre el firmware REAL: el mismo comando dentro del limite tiene
    # que llegar ENTERO. Si tambien se truncara, la medida de arriba no distinguiria.
    maestro.rx("\n")
    corto = base + "30,30,15"
    salidas = maestro.rx(corto + "\n")
    t.control_negativo(
        len(corto) <= util_max and any(s.startswith("$") for s in salidas),
        "el mismo comando dentro del limite (%d B) llega entero y se despacha: la "
        "prueba distingue el que cabe del que no" % len(corto))


def escenario_f4(t, c, maestro, app, util_max):
    t.titulo("F4 - checksum corrupto, en las DOS direcciones")

    # (a) app -> ESP32: se descarta, se contesta, y NO se reenvia.
    buena = "$" + c.prefijo_pin["Maestro"] + "SET_MODO:AUTO"
    buena = envolver(c, "Maestro", buena)
    mala = buena[:-4] + "00\r\n"
    puente = Puente(util_max)
    puente.desde_app(mala)
    t.verificar(
        puente.rechazos_crc == 1 and len(puente.hacia_stm32) == 0,
        "app->ESP32 con checksum malo: descartada, el STM32 no la ve",
        "el puente reenvio una trama con checksum malo (%d rechazos, %d entregas). El "
        "STM32 no valida el checksum de entrada: si el puente tampoco, no lo valida "
        "NADIE" % (puente.rechazos_crc, len(puente.hacia_stm32)))

    tablero, _ = app.entregar(puente.hacia_app[0])
    t.verificar(
        MARCA_PROPIA.split(":")[1] in (tablero.get("ultimo") or "").upper()
        or "CHECKSUM" in (tablero.get("ultimo") or "").upper(),
        "el puente contesta el rechazo y la app REAL lo pinta: %r"
        % (tablero.get("ultimo") or "")[:80],
        "el rechazo del puente no aparecio en la app: %r. Un descarte que no se dice "
        "se lee como que la orden entro" % (tablero.get("ultimo") or ""))

    # (b) STM32 -> app: el ruido de cable NO sube al telefono.
    puente = Puente(util_max)
    ruido = envolver(c, "Maestro", "$STATUS,NODE:MAESTRO,ESTADO:V1_R2,T:77")[:-4] + "FF\r\n"
    puente.desde_stm32(ruido)
    t.verificar(
        puente.crc_malos_del_cable == 1 and len(puente.hacia_app) == 0,
        "STM32->app con checksum malo: el puente recalcula y no lo sube",
        "el puente subio ruido de cable al telefono: %d malos detectados, %d "
        "escrituras" % (puente.crc_malos_del_cable, len(puente.hacia_app)))

    puente.desde_stm32(envolver(c, "Maestro", "$STATUS,NODE:MAESTRO,ESTADO:R1_V2,T:44"))
    t.verificar(
        len(puente.hacia_app) == 1,
        "y la siguiente trama sana SI sube: el puente no se queda atascado tras un "
        "descarte",
        "tras descartar una corrupta el puente dejo de subir las buenas (%d "
        "escrituras)" % len(puente.hacia_app))

    # =================================================================================
    # (c) QUIEN VALIDA EL CRC DE VUELTA. ESTE BLOQUE CAMBIO DE SIGNO EL 01/09.
    #
    # Hasta el 31/08 aqui habia dos comprobaciones que EXIGIAN el defecto, y lo hacian
    # con honestidad: "validarTrama() no tiene un solo llamador" y "entregada a la app
    # una trama con CRC malo, la pinta igual". Eran ciertas -era N-73 en JavaScript- y
    # documentaban el coste: si el puente no filtraba, no filtraba NADIE.
    #
    # El 31/08 app.js conecto validarTrama() dentro de juzgarTrama(), y las dos
    # cayeron. CLAUDE.md 8.quater y 8.sexies mandan contar cuantas cosas afirmaba cada
    # una antes de tocarlas, y afirmaban dos:
    #
    #   la del censo   "la funcion existe" (sigue valiendo) + "nadie la llama" (falso
    #                  hoy) ------> SE INVIERTE la segunda mitad, y se CONSERVA la
    #                  primera como precondicion: sin la funcion no hay nada que llamar.
    #   la ejercida    "se ejerce contra la app REAL" (sigue valiendo, es el metodo) +
    #                  "y la pinta" (falso hoy) ------> SE INVIERTE.
    #
    # Y LO QUE NO ESTABA Y AHORA HACE FALTA, que es la parte que N-83 dice que se
    # olvida: una inversion que solo mira el RESULTADO -"no se pinto"- la aprueba
    # tambien un tablero que no pinte NADA. Hacen falta las dos que faltaban:
    #   - el caso que exige que SI pase lo que debe pasar (la misma trama con el CRC
    #     bueno tiene que pintar), o esto no mide una barrera, mide una tapia;
    #   - y el ORDEN, no el resultado: que el rechazo ocurra ANTES de tocar el estado.
    #     Una app que pintara y luego se arrepintiera daria el mismo tablero final en
    #     este caso y uno distinto en cuanto un campo no se sobreescribiera.
    # =================================================================================

    # LOS TRES TEXTOS SE LEEN SIN COMENTARIOS. Ver sin_comentarios_js(): este fichero
    # documenta el defecto citando el codigo viejo, y buscar la huella en la prosa
    # acusaria a la app de conservar lo que explica haber quitado.
    app_js = sin_comentarios_js(fw.texto_repo(*APP_JS))
    parser_js = fw.texto_repo(*PARSER_JS)
    juez = cuerpo_js(app_js, "juzgarTrama")
    telemetria = cuerpo_js(app_js, "parseNmeaTelemetry")

    # [INVERTIDA] El censo de N-73, del reves: la huerfana GANO llamador. Se mide como
    # un censo -declaracion contra llamadas- y no con un `in` sobre el fichero, porque
    # app.js habla de validarTrama() en sus comentarios. Ver cuerpo_js().
    llamadas_juez = len(re.findall(r"(?<!function )\bjuzgarTrama\(", app_js))
    t.verificar(
        "validarTrama(" in parser_js
        and "NMEAParser.validarTrama(linea)" in juez
        and llamadas_juez >= 1,
        "MEDIDO: NMEAParser.validarTrama() ya NO es huerfana: la llama juzgarTrama() y "
        "juzgarTrama() tiene %d llamador(es). La app valida el CRC de vuelta por su "
        "cuenta, asi que el ESP32 dejo de ser el UNICO sitio donde se podia"
        % llamadas_juez,
        "validarTrama() volvio a quedarse sin llamador vivo en app.js (dentro de "
        "juzgarTrama: %s, llamadores de juzgarTrama: %d). Si la app deja de validar, "
        "el unico filtro de la cadena vuelve a ser el puente y este bloque mide otra "
        "cosa" % ("NMEAParser.validarTrama(linea)" in juez, llamadas_juez))

    # [EL ORDEN, QUE ES LO QUE N-83 DEMOSTRO QUE HAY QUE MIRAR] La barrera esta ANTES de
    # pintar, no despues. Dos huellas, y las dos en el cuerpo de parseNmeaTelemetry():
    # la rama de rechazo VUELVE antes de la primera escritura de estado, y el
    # `line.split('*')[0]` que tiraba el checksum sin leerlo -la huella literal del
    # defecto- ya no esta.
    corte = re.search(r"if \(!veredicto\.aceptada\)[\s\S]*?\breturn;", telemetria)
    pintura = re.search(r"state\.\w+\s*=[^=]", telemetria)
    resto_del_defecto = re.search(r"line\s*\.\s*split\('\*'\)", telemetria)
    t.verificar(
        corte is not None and pintura is not None
        and corte.end() < pintura.start() and resto_del_defecto is None,
        "y la llama EN ORDEN: la rama de rechazo vuelve en el byte %d del cuerpo y la "
        "primera escritura de estado esta en el %d -la barrera va DELANTE-, y el "
        "line.split('*')[0] que tiraba el CRC sin leerlo ya no aparece"
        % (corte.end() if corte else -1, pintura.start() if pintura else -1),
        "el orden de parseNmeaTelemetry() no es el que dice defenderse: rechazo en %s, "
        "primera escritura de estado en %s, resto del defecto %r. Una app que pinte y "
        "luego se arrepienta deja campos viejos escritos con bytes que llegaron rotos"
        % (corte.end() if corte else None, pintura.start() if pintura else None,
           resto_del_defecto.group(0) if resto_del_defecto else None))

    # [INVERTIDA Y EJERCIDA] Se conserva el metodo -entregarselo a la app REAL- y se
    # invierte lo que se exige.
    #
    # SE MIDE POR EL VALOR QUE TRAE LA TRAMA ROTA (77) Y NO POR "el contador no cambio",
    # y eso salio de verlo: entre dos escenarios pasa tiempo de reloj real, y el
    # watchdog de enlace de la app -TIMEOUT_ENLACE_MS, suyo, medido en F5- puede borrar
    # el contador a '--' por su cuenta en medio. Comparar contra el valor anterior
    # mediria ese watchdog y fallaria a ratos sin que nadie hubiera tocado la barrera.
    # El 77 solo puede aparecer si la trama rota se pinto.
    antes = app.tablero()["contador"]
    depu_antes = app.depuracion()
    tablero, _ = app.entregar(ruido)
    t.verificar(
        antes != "77" and tablero["contador"] != "77",
        "y se ejerce: entregada a la app REAL la MISMA trama con CRC malo, ya NO la "
        "pinta -el contador se queda en %r y no salta al 77 que traia la trama rota-"
        % tablero["contador"],
        "la app pinto una trama con el checksum malo: el contador paso de %r a %r. "
        "Desde el 31/08 juzgarTrama() tiene que pararla, y si no la para, un byte que "
        "cambio la radio se lee como estado del cruce" % (antes, tablero["contador"]))

    # [EL CONTROL QUE LE FALTA A TODA INVERSION - CLAUDE.md 8.sexies]
    #
    # Una guarda que no dejara pasar NADA haria pasar la linea de arriba igual de bien
    # que la guarda correcta. Sin este caso no se esta midiendo una barrera: se esta
    # midiendo una tapia. Es la MISMA trama, byte a byte, con el CRC que le toca.
    sana = envolver(c, "Maestro", "$STATUS,NODE:MAESTRO,ESTADO:V1_R2,T:77")
    tablero, _ = app.entregar(sana)
    t.verificar(
        tablero["contador"] == "77",
        "y la MISMA trama con el CRC bueno SI pinta (contador %r): la app rechaza por "
        "el checksum, no por sistema" % tablero["contador"],
        "la app tampoco pinta la trama SANA (contador %r): entonces la linea de arriba "
        "no mide una barrera, mide una tapia -cualquier app que no pintara nada la "
        "pasaria igual-" % tablero["contador"])

    # [QUIEN LO RECHAZA Y CON QUE MOTIVO, que es la pregunta que el encargo del 01/09
    # obliga a hacer] "Se descarta" no basta: un descarte mudo es indistinguible de una
    # trama que nunca llego, y esas dos averias se diagnostican distinto delante del
    # poste. Se lee por la pestana REAL, que es por donde lo lee el tecnico.
    depu_despues = app.depuracion()
    rech_antes, crc_antes = _contadores_app(depu_antes)
    rech_despues, crc_despues = _contadores_app(depu_despues)
    t.verificar(
        rech_despues == rech_antes + 1 and crc_despues == crc_antes + 1,
        "y lo DICE, con nombre: los contadores de la app pasan de %d a %d rechazadas y "
        "de %d a %d por CHECKSUM. El tecnico lee 'llegaba basura', que no es lo mismo "
        "que 'no llegaba nada'" % (rech_antes, rech_despues, crc_antes, crc_despues),
        "la app descarto la trama SIN decirlo: rechazadas %d -> %d, CHECKSUM %d -> %d. "
        "Un descarte mudo se lee como un hueco de enlace y manda a buscar la averia al "
        "sitio equivocado" % (rech_antes, rech_despues, crc_antes, crc_despues))

    # [EL REPARTO, MEDIDO: LAS DOS CAPAS TIENEN QUE JUZGAR IGUAL]
    #
    # Desde el 31/08 hay DOS validadores del mismo XOR-8 en la cadena de vuelta: el del
    # puente y el de la app. Dos copias de una regla es la forma que este repositorio
    # paga cara (CLAUDE.md 8.ter), y aqui los dos sentidos del desacuerdo tienen precio:
    #
    #   la APP mas laxa que el puente   una corrupta que el puente deje pasar la pinta
    #                                   la app: no la rechaza NADIE.
    #   la APP mas dura que el puente   la trama sube y muere arriba, asi que el
    #                                   operario puede leer el motivo de una capa hoy y
    #                                   el de la otra manana, para la misma averia.
    #
    # SOLO EL SENTIDO DE VUELTA. La comparacion se acota a lineas que empiezan por '$',
    # y no es una comodidad: el puente acepta A PROPOSITO la linea desnuda sin '$' -es
    # lo que la app manda de SUBIDA, medido en el escenario del asterisco-, mientras la
    # app la juzgaria SIN_FORMA. Comparar las dos direcciones daria un desacuerdo que no
    # significa nada.
    cuerpo_sano = "$STATUS,NODE:MAESTRO,ESTADO:R1_V2,T:44"
    sano = envolver(c, "Maestro", cuerpo_sano).strip()
    frontera = [
        ("sana", sano),
        ("CRC cambiado", sano[:-2] + "FF"),
        ("sin el '*XX'", cuerpo_sano),
        ("CRC en minusculas", sano[:-2] + sano[-2:].lower()),
        ("un byte de mas tras el CRC", sano + "Z"),
        ("payload vacio", "$*00"),
        ("solo el dolar", "$"),
    ]
    desacuerdos = []
    for etiqueta, linea in frontera:
        del_puente = Puente(util_max)._validar(linea) is not None
        de_la_app = app.juzgar(linea)["valida"]
        if del_puente != de_la_app:
            desacuerdos.append("%s: puente=%s app=%s" % (etiqueta, del_puente, de_la_app))
    t.verificar(
        not desacuerdos,
        "y las DOS capas juzgan igual el mismo XOR-8: los %d casos frontera de vuelta "
        "reciben el mismo veredicto del puente y de NMEAParser.validarTrama() REAL. Ni "
        "hay corrupta que las dos dejen pasar, ni una que rechacen las dos con dos "
        "motivos" % len(frontera),
        "el puente y la app NO juzgan igual el mismo checksum: %s. Una trama que una "
        "capa pasa y la otra tira es, o un agujero -si la laxa esta al final-, o dos "
        "mensajes distintos por la misma trama" % " | ".join(desacuerdos))

    # ---- Los dos controles negativos ------------------------------------------
    t.control_negativo(
        Puente(util_max)._validar(mala.strip()) is None
        and Puente(util_max)._validar(buena.strip()) is not None,
        "el validador del puente distingue la trama corrupta de la sana")

    # Y el de la comparacion de arriba, que sin esto seria una lista de verdades que
    # coinciden por casualidad: contra un validador deliberadamente laxo -uno que se
    # conforma con ver un '$' y un '*'- la misma comparacion tiene que cazar el
    # desacuerdo. Si no lo cazara, su PASS no valdria nada (N-51).
    laxo = lambda l: l.startswith("$") and "*" in l          # noqa: E731
    cazados = [e for e, l in frontera if laxo(l) != app.juzgar(l)["valida"]]
    t.control_negativo(
        bool(cazados),
        "la comparacion de las dos reglas caza a un validador laxo: le encuentra %d "
        "desacuerdo(s) (%s)" % (len(cazados), ", ".join(cazados)))

    # ---- LO QUE LA MEDIDA DEJA ABIERTO, y no lo decide un instrumento ----------
    p_mudo = Puente(util_max)
    depu_pre = app.depuracion()
    p_mudo.desde_stm32(sano + "\r\n")                 # sana: sube
    p_mudo.desde_stm32(sano[:-2] + "FF\r\n")          # corrupta: NO sube, y NO avisa
    for w in p_mudo.hacia_app:
        app.entregar(w)
    rech_con_puente = _contadores_app(app.depuracion())[0] - _contadores_app(depu_pre)[0]
    t.reportar(
        "EL DESCARTE DE BAJADA DEL PUENTE ES MUDO, Y ESO CAMBIA EL DIAGNOSTICO SEGUN "
        "HAYA ESP32 O NO",
        ["Medido en este mismo escenario, con la misma trama corrompida:",
         "",
         "  CON puente   el puente la cuenta (%d por CRC) y NO escribe nada hacia el"
         % p_mudo.crc_malos_del_cable,
         "               telefono. La app suma %d rechazos: para ella no ha pasado"
         % rech_con_puente,
         "               nada, y el hueco se lee como 'no llegaba nada'.",
         "  SIN puente   -que es la topologia de campo de HOY, JDY-31 directo- la app",
         "               la caza ella y la NOMBRA: CHECKSUM. 'Llegaba basura'.",
         "",
         "Las dos averias son distintas y se buscan en sitios distintos, y el doc 18",
         "3.4 no dice cual de las dos lecturas quiere. En la SUBIDA el puente si avisa",
         "-manda su $ERR con NODE:PUENTE, ejercido arriba-; en la BAJADA se calla.",
         "Esa asimetria no la ha decidido nadie por escrito.",
         "",
         "Y la otra mitad de la misma pregunta, tambien medida arriba: en el sentido",
         "de SUBIDA no hay checksum en ningun sitio -la app no firma, el puente no",
         "valida y el STM32 tampoco-, asi que un byte que cambie en J17 subiendo no lo",
         "rechaza NADIE. El doc 18 3.4.b lo declara deliberado y esta razonado; lo que",
         "no esta escrito es que el agujero exista solo ahi.",
         "",
         "Va en reportar() y no en verificar() porque es una politica sin dueno, no un",
         "defecto: ningun firmware puede 'aprobar' una decision que nadie ha tomado."])


def escenario_f5(t, c, maestro, app, util_max):
    t.titulo("F5 - TRES silencios que NO significan lo mismo")

    # (a) EL ESP32 SE CUELGA. El STM32 sigue ciclando y NO SE ENTERA.
    #
    # No es una opinion sobre el diseno: se mide en el fuente y se ejerce en el binario.
    for punta in PUNTAS:
        cod = c.codigo[punta]
        loop = cod[cod.find("void bluetooth_loop"):]
        marcas = re.findall(r"millis\(\)", loop)
        guarda = re.search(r"tUltimaRx|SILENCIO|silencio|timeout|TIMEOUT", loop)
        t.verificar(
            len(marcas) == 1 and guarda is None,
            "%s: bluetooth_loop() lee millis() %d vez y no tiene NINGUNA guarda de "
            "silencio sobre la recepcion -> si el ESP32 muere, el equipo no se entera"
            % (punta, len(marcas)),
            "%s: bluetooth_loop() tiene %d millis() y/o una guarda (%r). Si alguien "
            "anadio un watchdog del puerto Bluetooth, este escenario cambia de "
            "significado" % (punta, len(marcas), guarda.group(0) if guarda else None))

    # Se ejerce sobre el binario REAL: radio viva, puente muerto -nadie lee-. El equipo
    # tiene que seguir emitiendo su telemetria y NO irse a ambar.
    #
    # PRIMERO SE PONE EN UN MODO DE OPERACION, y no es un detalle del arnes: el propio
    # coordinador.cpp trata el silencio distinto en C_MENU_IDLE -alli cae a ambar sin
    # levantar alarma- que en marcha. Medir SFTY-6 con el equipo en el menu habria dado
    # "no hay alarma" y la conclusion habria sido falsa: la regla existe, lo que no
    # existia era el escenario. Las dos mitades de F5 se miden en el MISMO modo, para
    # que lo unico que cambie entre ellas sea la radio.
    maestro.ajustar("RADIO 1")
    puente_auto = Puente(util_max)
    puente_auto.desde_app(c.prefijo_pin["Maestro"] + "SET_MODO:AUTO\r\n")
    maestro.rx(puente_auto.hacia_stm32[0])
    salidas = maestro.avanzar(c.sfty6_ms + 5000)
    status = [s for s in salidas if s.startswith("$STATUS")]
    esperados = (c.sfty6_ms + 5000) // c.periodo_ms["Maestro"]
    t.verificar(
        len(status) >= esperados - 1,
        "con el puente muerto %d s, el firmware REAL emite igual sus %d $STATUS: "
        "sigue ciclando" % ((c.sfty6_ms + 5000) // 1000, len(status)),
        "el firmware emitio %d $STATUS y tocaban ~%d: si deja de hablar cuando el "
        "puente muere, este escenario mide otro equipo" % (len(status), esperados))

    alarmas = [s for s in salidas if s.startswith("$ALARM")]
    t.verificar(
        not alarmas,
        "y NO levanta ninguna alarma por eso: el silencio del puente no tiene "
        "consecuencia vial",
        "el firmware levanto %d alarma(s) con la radio viva y el puente muerto: %r. "
        "Confundir los dos silencios manda un cruce sano a ambar"
        % (len(alarmas), alarmas[:1]))

    # (b) LA RADIO LoRa SE CAE. SFTY-6 SI dispara, sobre coordinador.cpp REAL.
    t.verificar(
        "SFTY6_SILENCIO_MS" not in c.codigo["Maestro"]
        and "SFTY6_SILENCIO_MS" not in c.codigo["Esclavo"],
        "el umbral de radio son %d ms y NO aparece en ningun bluetooth.cpp: los dos "
        "silencios son instrumentos distintos" % c.sfty6_ms,
        "SFTY6_SILENCIO_MS aparece dentro de bluetooth.cpp: si el watchdog de radio se "
        "alimentara del puerto del telefono, un ESP32 colgado mandaria el cruce a "
        "ambar y un telefono conectado lo salvaria")

    mudo = Stm32Real("Maestro")
    try:
        mudo.ajustar("RADIO 0")
        # El mismo modo de operacion que en la mitad (a). Ver la nota de alli.
        p2 = Puente(util_max)
        p2.desde_app(c.prefijo_pin["Maestro"] + "SET_MODO:AUTO\r\n")
        mudo.rx(p2.hacia_stm32[0])
        salidas = mudo.avanzar(c.sfty6_ms + 3000)
        alarmas = [s for s in salidas if s.startswith("$ALARM")]
        t.verificar(
            bool(alarmas),
            "con la RADIO muda, coordinador.cpp REAL si levanta la alarma antes de "
            "%d ms: %r" % (c.sfty6_ms + 3000,
                           alarmas[0].strip()[:80] if alarmas else None),
            "con la radio muda %d ms el firmware no levanto ninguna alarma. SFTY-6 "
            "tiene que disparar por silencio de radio, y este arnes lo esta midiendo "
            "sobre coordinador.cpp de verdad" % (c.sfty6_ms + 3000))
        t.verificar(
            any("SILENCIO" in a for a in alarmas),
            "y la causa que publica nombra el silencio, que es lo que el tecnico lee",
            "la alarma no nombra el silencio: %r" % (alarmas[:1],))
    finally:
        mudo.cerrar()

    # (c) EL TELEFONO SE VA. No pasa NADA en el equipo; lo declara la app, y sola.
    t.verificar(
        "TIMEOUT_ENLACE_MS" not in c.codigo["Maestro"]
        and "TIMEOUT_ENLACE_MS" not in c.codigo["Esclavo"],
        "el plazo de %d ms con el que la app declara el enlace perdido vive SOLO en "
        "app.js: que el telefono se vaya no cambia nada en el poste" % c.timeout_app_ms,
        "el plazo de la app aparece en el firmware: el equipo no puede depender de que "
        "haya alguien mirando un telefono")

    t.verificar(
        c.sfty6_ms != c.timeout_app_ms,
        "los tres silencios tienen tres plazos distintos y tres duenos: radio %d ms "
        "(protocolo.h) / app %d ms (app.js) / puente: NINGUNO, nadie lo vigila"
        % (c.sfty6_ms, c.timeout_app_ms),
        "el plazo de radio (%d) y el de la app (%d) coinciden: dos cosas que se miden "
        "igual acaban tratandose igual" % (c.sfty6_ms, c.timeout_app_ms))

    t.reportar(
        "EL TERCER SILENCIO SE MIDE DESDE EL 31/08, PERO SIGUE SIN VIGILARSE - y no es "
        "lo mismo",
        ["Los dos primeros silencios tienen dueno y plazo medido. El TERCERO -el del",
         "propio ESP32- cambio a medias con A3 (commit d44048c), y la diferencia entre",
         "MEDIR y VIGILAR es justo lo que queda abierto:",
         "",
         "  lo que A3 SI hizo   j17RegistrarLinea() cierra el silencio cuando llega una",
         "                      linea y publica $EVENT,ORIGEN:J17,DETALLE:MUDO:Ns.",
         "                      Ejercido en F6. Es un dato de DIAGNOSTICO, y bueno: el",
         "                      tecnico que se conecta lee cuanto llevaba mudo el puerto.",
         "  lo que NO hizo      no hay umbral, ni alarma, ni comprobacion periodica. La",
         "                      guarda de tiempo en bluetooth_loop() sigue sin existir",
         "                      -medido arriba, en este mismo escenario-, asi que el",
         "                      equipo NO se entera mientras el puente esta muerto: solo",
         "                      puede contarlo DESPUES, y solo a quien vuelva a hablarle.",
         "",
         "Y la app sigue sin distinguir 'el equipo callado' de 'el puente colgado': las",
         "dos cosas le llegan como %d ms sin trama, y un tecnico delante del poste vera"
         % c.timeout_app_ms,
         "'Sin enlace' en las dos. El $EVENT de A3 no llega a tiempo para eso, porque",
         "por construccion sale cuando el silencio YA se acabo.",
         "Que el hueco que queda se cierre con un latido propio del ESP32, con un campo",
         "en la trama, o con nada, es una decision con dueno y no la toma un",
         "instrumento."])


def escenario_f6(t, c, maestro, util_max):
    t.titulo("F6 - dos operarios con ordenes contradictorias por el mismo puente")

    puente = Puente(util_max)
    maestro.rx("\n")
    for accion in ("SET_MODO:AUTO", "SET_MODO:AMBAR"):
        puente.desde_app(c.prefijo_pin["Maestro"] + accion + "\r\n")
    salidas = []
    for w in puente.hacia_stm32:
        salidas.extend(maestro.rx(w))

    acks = [s.strip() for s in salidas if s.startswith("$ACK")]
    t.verificar(
        len(acks) == 2 and "AUTO" in acks[0] and "AMBAR" in acks[1],
        "el firmware REAL acepta las dos en orden de llegada: EL ULTIMO GANA, en "
        "silencio (%s)" % " | ".join(a[:34] for a in acks),
        "las ordenes contradictorias no se despacharon en orden de llegada: %r" % (acks,))

    # EL EQUIPO NO INVENTA ARBITRAJE, y eso tambien se mide: no hay una sola nocion de
    # sesion, cliente o token en el despachador de ninguna de las dos puntas.
    for punta in PUNTAS:
        cuerpo = Contrato._cuerpo_despachador(c.codigo[punta], punta)
        rastro = re.search(r"sesion|session|cliente|token|ultimoOrigen|arbitr", cuerpo)
        t.verificar(
            rastro is None,
            "%s: el despachador no tiene ninguna nocion de sesion ni de cliente - no "
            "arbitra, y no finge que arbitra" % punta,
            "%s: aparece %r en el despachador. Si el firmware empezo a arbitrar entre "
            "telefonos, esta comprobacion mide otra cosa"
            % (punta, rastro.group(0) if rastro else None))

    # =================================================================================
    # $EVENT EXISTE PARA ESTO, Y AQUI SE MIDE HASTA DONDE LLEGA.
    #
    # ESTA COMPROBACION SE PARTIO EN DOS EL 01/09, Y NO PORQUE FALLARA EL FIRMWARE.
    # Hasta el 31/08 decia `len(eventos) == 2 and all("ORIGEN:" in e ...)`, y eso
    # afirmaba DOS cosas de una vez (CLAUDE.md 8.sexies):
    #
    #   la que era su tema      cada cambio de modo deja su linea en la bitacora, y la
    #                           linea dice de donde vino la orden.
    #   la que se colo de tapadillo   "y no hay ningun otro $EVENT en toda la tanda".
    #                           Eso nunca fue el tema de F6: era un efecto lateral de
    #                           contar con `== 2`.
    #
    # El 31/08 A3 -commit d44048c- hizo que el STM32 cuente el silencio de J17 y publique
    # un $EVENT,ORIGEN:J17 por cada linea recibida. La segunda mitad dejo de ser cierta
    # y arrastro a la primera, que sigue siendo verdad y que nadie mas mide.
    #
    # No se relaja el numero hasta que pase: SE REPARTE. La mitad que era el tema se
    # queda aqui y ademas se ENDURECE -antes solo exigia que la palabra ORIGEN: saliera;
    # ahora exige de que canal vino cada una y en que ORDEN llegaron-, y la otra mitad
    # se convierte en lo que de verdad hay que vigilar del flujo nuevo: uno por linea,
    # y en un canal distinguible del mando.
    # =================================================================================
    eventos = [s.strip() for s in salidas if s.startswith("$EVENT")]

    def _campo(trama, clave):
        m = re.search(r"%s:([^,*]+)" % clave, trama)
        return m.group(1) if m else None

    def _bitacora_del_mando(lista):
        """Los $EVENT que dejan las DOS ordenes, en orden de llegada.

        Se pide la lista completa -canal y detalle de cada uno- y no un recuento: dos
        lineas con ORIGEN: escrito y el detalle cambiado no valen para distinguir a dos
        operarios, que es lo unico que F6 existe para medir."""
        return [(_campo(e, "ORIGEN"), _campo(e, "DETALLE"))
                for e in lista if _campo(e, "ORIGEN") == "APP_BLUETOOTH"]

    del_mando = _bitacora_del_mando(eventos)
    t.verificar(
        del_mando == [("APP_BLUETOOTH", "SET_MODO_AUTO"),
                      ("APP_BLUETOOTH", "SET_MODO_AMBAR")],
        "y cada cambio deja su linea en la bitacora, con el canal Y la orden, en orden "
        "de llegada: %s" % " | ".join("%s/%s" % p for p in del_mando),
        "los dos cambios no dejaron su rastro en la bitacora del mando: %r (de %d "
        "$EVENT en la tanda). Sin bitacora de origen, dos operarios contradiciendose "
        "son indistinguibles de un equipo que hace cosas solo"
        % (del_mando, len(eventos)))

    # El control que le falta a toda comprobacion de ORDEN: la de arriba tiene que saber
    # distinguir las dos ordenes CAMBIADAS DE SITIO. Con `all("ORIGEN:" in e)` -lo de
    # antes- una bitacora que confesara las dos ordenes al reves pasaba igual, y es
    # justo el caso que F6 mide: cual gano.
    al_reves = list(reversed(eventos))
    t.control_negativo(
        _bitacora_del_mando(al_reves) != del_mando
        and _bitacora_del_mando([e.replace("ORIGEN:APP_BLUETOOTH", "ORIGEN:J17")
                                 for e in eventos]) == [],
        "la bitacora se lee por canal Y por orden: con las mismas lineas al reves da "
        "otra cosa, y con el canal cambiado no da ninguna")

    # 🔴 ESTA COMPROBACION SE INVIERTE EL 04/09 (AB-1), Y NO PORQUE ESTUVIERA MAL.
    #
    # Exigia UN $EVENT ORIGEN:J17 POR LINEA entregada, y era lo correcto mientras cada
    # linea que llegaba era un dedo en la app: un silencio era un dato en si mismo y
    # publicarlos todos era la unica forma de que el tecnico viera el suyo. Se escribio
    # con esa razon y la razon se cumplia.
    #
    # Lo que cambio es lo que mide. Con el latido del puente cada LATIDO_MS, el puerto
    # habla SOLO: un silencio de un latido es el reposo, no un suceso. A 2 s serian 1.800
    # lineas identicas por hora en la misma bitacora donde hay que encontrar el fallo de
    # campo -N-73 por inundacion en vez de por filtro-. La propia frase del fallo lo
    # anticipaba: "uno de mas por linea llena la bitacora de ruido".
    #
    # LO QUE SE EXIGE AHORA, y es mas fuerte, no mas debil: que se publique SOLO lo que
    # supera el umbral, que ese umbral salga del periodo del latido -no de un numero
    # elegido-, y que el contador N siga avanzando con TODAS las lineas. Los contadores
    # cuentan todo; lo que se acota es lo que se publica.
    del_puerto = [e for e in eventos if _campo(e, "ORIGEN") == "J17"]
    ns = [int(m.group(1)) for m in
          (re.search(r"N:(\d+)", e) for e in del_puerto) if m]
    t.verificar(
        len(del_puerto) <= len(puente.hacia_stm32)
        and len(ns) == len(del_puerto) and ns == sorted(set(ns)),
        "y el silencio de J17 deja SU propia linea solo cuando pasa del umbral: %d "
        "$EVENT ORIGEN:J17 para %d lineas entregadas, con N: %s -el contador avanza con "
        "todas, la bitacora recoge las que importan-"
        % (len(del_puerto), len(puente.hacia_stm32), ns),
        "el registro de silencio de J17 no cuadra: %d $EVENT ORIGEN:J17 para %d lineas, "
        "N: %s. Mas eventos que lineas es imposible; y si el contador N no avanza o se "
        "repite, dos sucesos distintos se leen como uno"
        % (len(del_puerto), len(puente.hacia_stm32), ns))

    # Y LA MITAD QUE LA INVERSION SE HABRIA LLEVADO POR DELANTE SI NO SE ANOTA (8.sexies):
    # con "<=" sola, un firmware que no publicara NINGUN evento pasaria igual de bien. El
    # umbral tiene que salir del latido, y eso se lee del C++ de las dos puntas.
    umbrales = set()
    for punta in ("Maestro", "Esclavo"):
        cod = fw.codigo(punta, "src", "bluetooth.cpp")
        for m in re.finditer(r"J17_SILENCIO_MIN_MS\s*=\s*(\d+)UL", cod):
            umbrales.add(int(m.group(1)))
    latido = None
    con = fw.codigo("ESP32_Expansion", "include", "contrato.h")
    m = re.search(r"#define\s+LATIDO_MS\s+(\d+)UL", con)
    if m:
        latido = int(m.group(1))
    t.verificar(
        len(umbrales) == 1 and latido is not None
        and latido < list(umbrales)[0] < latido * 2,
        "el umbral de publicacion (%s ms) sale del periodo del latido (%s ms) y es el "
        "mismo en las dos puntas: por debajo esta el reposo, por encima algo se perdio"
        % (sorted(umbrales), latido),
        "el umbral de J17 y el periodo del latido no se sostienen: umbrales %s, latido "
        "%s. Si el umbral no esta entre un latido y dos, o difiere entre puntas, deja de "
        "significar lo que su nombre dice y vuelve a ser un numero que nadie decidio"
        % (sorted(umbrales) or "(no se hallan)", latido)) 

    origenes = set(re.findall(r'bluetooth_reportarEvento\("([^"]+)"',
                              c.codigo["Maestro"] + c.codigo["Esclavo"]))
    llamadores = len(re.findall(r"bluetooth_reportarEvento\(",
                                c.codigo["Maestro"] + c.codigo["Esclavo"]))
    t.reportar(
        "EL ORIGEN DE $EVENT DISTINGUE EL CANAL, NO EL TELEFONO - y la politica no "
        "esta decidida",
        ["MEDIDO: los %d llamadores de bluetooth_reportarEvento() de las dos puntas"
         % llamadores,
         "pasan %d literal(es) distinto(s): %s." % (len(origenes),
                                                    ", ".join(sorted(origenes))),
         "La bitacora dice 'vino de la app' y NO dice de que telefono. Con dos",
         "operarios contradiciendose, la traza los mezcla en una sola linea.",
         "",
         "Lo que este simulador MIDE, y da por bueno: el ultimo gana, en silencio, y",
         "el equipo no inventa arbitraje -que es lo correcto, porque un arbitraje",
         "improvisado en el poste seria una regla vial que nadie escribio-.",
         "Si hace falta distinguir telefonos, bloquear al segundo, o avisar al",
         "primero, es una decision del responsable. Aqui se mide y se reporta."])


def escenario_f7(t, c, maestro, app, util_max):
    t.titulo("F7 - saturacion: la rafaga contra el caudal real del canal")

    caudal = c.baudios["Maestro"] / 10.0     # 8N1: 10 bits por byte con arranque y parada

    # LA RAFAGA SE PIDE AL FIRMWARE REAL, no se fabrica: un comando que se acepta
    # produce $ACK + $EVENT, y el $STATUS periodico cae encima.
    maestro.rx("\n")
    puente = Puente(util_max)
    # Los valores salen de los rangos leidos de modo_automatico.cpp. Con "30,30,15"
    # -que es lo que decia la primera version- el firmware contesta $ERR,DESC:RANGO:
    # el verde va en MINUTOS. La rafaga que este escenario mide no llegaba a existir, y
    # el escenario acusaba al firmware de no emitir un $EVENT que no tocaba emitir.
    puente.desde_app(c.prefijo_pin["Maestro"] + "SET_TIEMPOS:" + c.tiempos_validos()
                     + "\r\n")
    salidas = maestro.rx(puente.hacia_stm32[0]) if puente.hacia_stm32 else []
    salidas += maestro.avanzar(c.periodo_ms["Maestro"] + 1)
    tramas = [s for s in salidas if s.startswith("$")]
    bytes_rafaga = sum(len(s) for s in tramas)

    t.verificar(
        len(tramas) >= 3 and bytes_rafaga > 0,
        "el firmware REAL produce una rafaga de %d tramas y %d B: %s"
        % (len(tramas), bytes_rafaga,
           " + ".join(sorted({t2.split(",")[0] for t2 in tramas}))),
        "la rafaga esperada -$ACK + $EVENT + $STATUS- no aparecio: %r" % (tramas,))

    for s in salidas:
        puente.desde_stm32(s)
    subidas = [w for w in puente.hacia_app if w.startswith("$")
               and MARCA_PROPIA not in w]
    t.verificar(
        len(subidas) == len(tramas),
        "el puente la sube como %d escrituras: NO agrupa telemetria" % len(subidas),
        "el puente entrego %d escritura(s) para %d tramas: agrupar dos $STATUS hace "
        "que la app pierda una en silencio" % (len(subidas), len(tramas)))

    vistas = 0
    for w in subidas:
        antes = app.tablero()["eventos"]
        tab, _ = app.entregar(w)
        if tab["eventos"] > antes or w.startswith("$STATUS"):
            vistas += 1
    t.verificar(
        vistas == len(subidas),
        "y la app REAL procesa las %d: ninguna se pierde en el camino" % vistas,
        "la app solo acuso %d de %d tramas de la rafaga" % (vistas, len(subidas)))

    ms_rafaga = bytes_rafaga * 1000.0 / caudal
    t.verificar(
        ms_rafaga < c.timeout_app_ms,
        "la rafaga ocupa %.0f ms de cable a %d bps (%.0f B/s) y el plazo de la app es "
        "%d ms: cabe" % (ms_rafaga, c.baudios["Maestro"], caudal, c.timeout_app_ms),
        "la rafaga ocupa %.0f ms y la app declara el enlace perdido a los %d ms: una "
        "rafaga normal bastaria para dar por muerto a un equipo sano"
        % (ms_rafaga, c.timeout_app_ms))

    mayor = max((len(s) for s in tramas), default=0)
    ocupacion = mayor * (1000.0 / c.periodo_ms["Maestro"]) / caudal * 100.0
    t.verificar(
        ocupacion < 100.0,
        "la trama mas grande que el firmware emitio mide %d B; a una por %d ms ocupa "
        "el %.1f %% del canal" % (mayor, c.periodo_ms["Maestro"], ocupacion),
        "la telemetria sola ocuparia el %.1f %% del canal: no cabe el trafico de "
        "comandos" % ocupacion)

    # P-1: el puente NO ORIGINA. Sin nada del telefono, no escribe hacia el STM32.
    quieto = Puente(util_max)
    for _ in range(50):
        quieto.desde_stm32(envolver(c, "Maestro", "$STATUS,NODE:MAESTRO,ESTADO:V1_R2"))
    t.verificar(
        len(quieto.hacia_stm32) == 0,
        "50 tramas de telemetria despues, el puente no ha escrito un solo byte hacia "
        "el STM32: no origina trafico propio (P-1)",
        "el puente escribio %d vez/veces hacia el STM32 sin que nadie se lo pidiera. "
        "El margen del enlace es del equipo, no del accesorio"
        % len(quieto.hacia_stm32))

    t.control_negativo(
        len(subidas) == len(tramas) and len(tramas) > 1,
        "la cuenta de escrituras distingue una rafaga de varias tramas de una sola: "
        "%d tramas -> %d escrituras" % (len(tramas), len(subidas)))


def escenario_asterisco(t, c, maestro, app, util_max):
    t.titulo("LA PREGUNTA SIN DECIDIR - que hace el puente con el *XX de un comando "
             "VALIDO")

    # 1. MEDIDO en el fuente: el STM32 no valida el checksum de entrada.
    for punta in PUNTAS:
        cuerpo = Contrato._cuerpo_despachador(c.codigo[punta], punta)
        cod = c.codigo[punta]
        loop = cod[cod.find("void bluetooth_loop"):]
        t.verificar(
            "calcularChecksum" not in cuerpo and "calcularChecksum" not in loop,
            "%s: ni una llamada a calcularChecksum() en el camino de recepcion "
            "-ni en procesarComando() ni en bluetooth_loop()-" % punta,
            "%s: aparece calcularChecksum() en el camino de recepcion. Si el firmware "
            "empezo a validar la entrada, esta pregunta esta contestada y hay que "
            "reescribir el escenario" % punta)

    # 2. LAS DOS POLITICAS, EJERCIDAS CONTRA EL FIRMWARE REAL. No se argumenta: se
    #    barre el censo entero y se cuenta.
    censo = comandos_censados(c, "Maestro")
    con_asterisco, sin_asterisco = 0, 0
    motivos = {}
    for cmd in censo:
        maestro.rx("\n")
        completa = envolver(c, "Maestro", "$" + cmd).strip()
        s = maestro.rx(completa + "\n")           # politica A: se reenvia con $ y *XX
        if _ack_de(s) is None:                    # rechazado, sea cual sea el motivo
            con_asterisco += 1
        for x in s:
            m = re.search(r"\$ERR,CMD:([^,*]+)", x)
            if m:
                motivos[m.group(1)] = motivos.get(m.group(1), 0) + 1
                break
        maestro.rx("\n")
        s2 = maestro.rx(cmd + "\n")               # politica B: comando desnudo
        if s2 and not any("DESCONOCIDO" in x or "AUTH_FAILED" in x for x in s2):
            sin_asterisco += 1

    # OJO AL MOTIVO, QUE EL DOC 18 3.4 SE QUEDA CORTO Y ESO IMPORTA. Alli se dice que
    # el *XX "hace que el comando no case, cayendo en $ERR,CMD:DESCONOCIDO". Medido
    # contra el firmware: los que llevan PIN caen ANTES, en $ERR,CMD:AUTH_FAILED,
    # porque el '$' de delante rompe el strncmp del prefijo y ni se llega a mirar la
    # accion. La consecuencia practica es la misma -no se ejecuta ninguno- pero el
    # motivo que veria el tecnico en el telefono NO es el que el documento anuncia, y
    # un tecnico leyendo "PIN invalido" ira a buscar un problema de clave que no hay.
    t.verificar(
        con_asterisco == len(censo),
        "MEDIDO sobre el firmware REAL: si el puente reenvia el *XX, NINGUNO de los %d "
        "comandos del censo se ejecuta - los %d rechazados, con estos motivos: %s"
        % (len(censo), con_asterisco,
           ", ".join("%s=%d" % kv for kv in sorted(motivos.items()))),
        "con el *XX puesto %d de %d comandos SI se ejecutan: el envoltorio NMEA no "
        "bloquea lo que se creia, y la pregunta abierta cambia de forma"
        % (len(censo) - con_asterisco, len(censo)))

    t.verificar(
        sin_asterisco == len(censo),
        "MEDIDO: si el puente entrega el comando desnudo, los %d llegan a su rama"
        % len(censo),
        "sin el *XX solo llegan a su rama %d de %d: el censo del fuente y el "
        "despachador real no coinciden" % (sin_asterisco, len(censo)))

    # 3. Y LA APP DE HOY TIENE DOS CAMINOS DE TX QUE NO COINCIDEN. Sale de grep y de
    #    ejecutar la app, no de leer.
    app_js = fw.texto_repo(*APP_JS)
    parser_js = fw.texto_repo(*PARSER_JS)
    llamadores = len(re.findall(r"NMEAParser\.generarComando\(", app_js))
    compuesto = app.pulsar_id("btn-op-auto")
    real = compuesto[0].strip() if compuesto else ""
    t.verificar(
        "generarComando(" in parser_js and llamadores == 0 and "*" not in real,
        "MEDIDO ejecutando la app REAL: compone %r, SIN checksum. Y "
        "js/nmea_parser.js tiene generarComando() -que si lo pone- con %d llamadores "
        "en app.js" % (real, llamadores),
        "el censo de los dos caminos de TX no da lo esperado (compuesto real %r, "
        "generador presente %r, llamadores %d). Si alguien los unifico, la pregunta "
        "del *XX cambia de forma"
        % (real, "generarComando(" in parser_js, llamadores))

    # CONTROL NEGATIVO: el MISMO detector, sobre el MISMO comando, tiene que dar
    # respuestas distintas segun lleve envoltorio o no. Si diera lo mismo en los dos
    # casos, las dos cuentas de arriba estarian midiendo el detector y no el firmware.
    uno = c.prefijo_pin["Maestro"] + "SET_MODO:AUTO"
    maestro.rx("\n")
    con = _ack_de(maestro.rx(envolver(c, "Maestro", "$" + uno).strip() + "\n"))
    maestro.rx("\n")
    sin = _ack_de(maestro.rx(uno + "\n"))
    t.control_negativo(
        con is None and sin is not None,
        "el mismo comando %r da $ACK=%r desnudo y $ACK=%r con envoltorio: el detector "
        "distingue las dos politicas, no las cuenta iguales" % (uno, sin, con))

    t.reportar(
        "NADIE HA DECIDIDO si el puente reenvia el *XX de un comando VALIDO",
        ["El doc 18 3.4 dice que el ESP32 valida el checksum y DESCARTA lo malo. No",
         "dice que hace con lo BUENO: si lo reenvia tal cual -con $ y con *XX- o si",
         "entrega el comando desnudo que el strcmp del STM32 espera.",
         "",
         "MEDIDO arriba contra el firmware REAL, y las dos cifras son media respuesta:",
         "  - reenviando el *XX : %d/%d comandos RECHAZADOS, ninguno se ejecuta."
         % (con_asterisco, len(censo)),
         "  - entregando desnudo: %d/%d llegan a su rama." % (sin_asterisco,
                                                              len(censo)),
         "",
         "Y una correccion al doc 18 3.4, medida: alli se dice que el *XX los hace",
         "caer en $ERR,CMD:DESCONOCIDO. Los motivos reales son %s."
         % ", ".join("%s=%d" % kv for kv in sorted(motivos.items())),
         "Los que llevan PIN caen ANTES, en AUTH_FAILED, porque el '$' de delante",
         "rompe el strncmp del prefijo y la accion ni se llega a mirar. El efecto es",
         "el mismo -no se ejecuta ninguno- pero el tecnico leeria 'PIN invalido' y se",
         "pondria a buscar un problema de clave que no existe.",
         "",
         "Y hay una tercera pieza que enreda la pregunta, medida ejecutando la app:",
         "enviarComandoFirmware() (app.js:191) manda 'CMD:...\\r\\n' SIN checksum -que",
         "es lo que el STM32 espera- mientras generarComando() (js/nmea_parser.js:129)",
         "SI lo pone y NO tiene un solo llamador en produccion: solo lo ejercen los",
         "tests. Es la forma de N-73, y su hermana validarTrama() esta igual.",
         "",
         "Este simulador acepta las dos formas en la entrada del puente y entrega el",
         "comando DESNUDO, porque es la unica con la que el firmware de hoy responde.",
         "Esa eleccion es de este fichero, NO de la especificacion, y no vale como",
         "decision: mientras el doc 18 3.4 no lo diga, quien escriba el .cpp del ESP32",
         "puede elegir la otra y TODOS los comandos moriran en DESCONOCIDO sin que",
         "nada lo avise hasta el banco.",
         "",
         "LA PREGUNTA, PARA QUE SE CONTESTE EN UNA LINEA DE ESPECIFICACION:",
         "  el ESP32, tras validar el XOR-8 de un comando entrante, escribe hacia PB7",
         "  la linea COMPLETA ($ + payload + *XX), o solo el PAYLOAD desnudo?",
         "  Y en el segundo caso, quien responde de que la app mande siempre lo",
         "  mismo? -hoy manda dos cosas distintas segun el camino de codigo-.",
         "",
         "Va en reportar() y no en verificar() a proposito: una comprobacion que",
         "ningun firmware puede aprobar porque nadie ha decidido la respuesta no es",
         "una comprobacion, es una nota (CLAUDE.md 3)."])


# =====================================================================================

def main():
    print("=" * 78)
    print(" SIMULADOR DEL PUENTE ESP32 - app REAL <-> modelo del ESP32 <-> STM32 REAL")
    print("=" * 78)

    t = Contador()
    vivos = []
    try:
        c = Contrato()

        print("\n Contrato leido del fuente (nada escrito a mano):")
        print("   buffer de entrada  : %d B -> %d caracteres utiles (E-2)"
              % (c.buffer["Maestro"], c.util["Maestro"]))
        print("   transporte         : %d bps, RX %s / TX %s (USART1 remapeado, J17)"
              % (c.baudios["Maestro"], c.pines["Maestro"][0], c.pines["Maestro"][1]))
        print("   telemetria         : $STATUS cada %d ms" % c.periodo_ms["Maestro"])
        print("   envoltorio         : %r" % c.envoltorio["Maestro"])
        print("   barrera de PIN     : %r (%d B)"
              % (c.prefijo_pin["Maestro"], c.largo_pin["Maestro"]))
        print("   comandos censados  : Maestro %d, Esclavo %d"
              % (len(comandos_censados(c, "Maestro")),
                 len(comandos_censados(c, "Esclavo"))))
        print("   silencio de radio  : %d ms (SFTY-6, protocolo.h)" % c.sfty6_ms)
        print("   plazo de la app    : %d ms (app.js)" % c.timeout_app_ms)

        maestro = Stm32Real("Maestro"); vivos.append(maestro)
        esclavo = Stm32Real("Esclavo"); vivos.append(esclavo)
        app = AppReal(); vivos.append(app)
        print("\n Arneses vivos:")
        print("   %s" % maestro.pr.saludo)
        print("   %s" % esclavo.pr.saludo)
        print("   %s" % app.pr.saludo)

        util_max = escenario_contrato(t, c, maestro, esclavo, app)
        escenario_n1(t, c, maestro, app, util_max)
        escenario_n2(t, c, maestro, app, util_max)
        escenario_n3(t, c, maestro, esclavo, app, util_max)
        escenario_n4(t, c, maestro, esclavo, util_max)
        escenario_f1(t, c, maestro, util_max)
        escenario_f2(t, c, maestro, app, util_max)
        escenario_f3(t, c, maestro, util_max)
        escenario_f4(t, c, maestro, app, util_max)
        escenario_f5(t, c, maestro, app, util_max)
        escenario_f6(t, c, maestro, util_max)
        escenario_f7(t, c, maestro, app, util_max)
        escenario_asterisco(t, c, maestro, app, util_max)

    except fw.Abortado as e:
        print("\n[ABORTADO] %s" % e)
        return 2
    except Exception as e:  # noqa: BLE001
        print("\n[ABORTADO] excepcion a mitad de la medida: %s: %s"
              % (type(e).__name__, e))
        return 2
    finally:
        for v in vivos:
            try:
                v.cerrar()
            except Exception:
                pass

    print("\n" + "=" * 78)
    if t.fallos:
        print(" NO CUMPLEN:")
        for f in t.fallos:
            print("   - %s" % f)
    if t.hallazgos:
        print(" PREGUNTAS ABIERTAS (no cuentan como comprobacion): %d"
              % len(t.hallazgos))
        for titulo, _ in t.hallazgos:
            print("   - %s" % titulo)
    # LA CUENTA SE CALCULA. Nunca literal: simulador_app_bluetooth.py publica "5/5"
    # escrito a mano y por eso el detector de N-71 no puede fallar jamas sobre el.
    print(" RESULTADO: %d/%d comprobaciones del contrato del puente en PASS"
          % (t.pasadas, t.total))
    print("=" * 78)
    return 1 if t.fallos else 0


if __name__ == "__main__":
    sys.exit(main())
