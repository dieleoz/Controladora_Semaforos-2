# ===== 01_Firmware/Simulaciones/simulador_app_bluetooth.py =====
#
# APP MOVIL <-> FIRMWARE BLUETOOTH: LA TRAMA QUE SALE DEL MICRO CONTRA EL PARSER QUE
# LA APP CARGA DE VERDAD, EJERCIDOS, NO LEIDOS.
#
# QUE MIDE ESTE INSTRUMENTO Y QUE NO, QUE ES LO PRIMERO QUE HAY QUE SABER DE EL:
#
#   - El lado del TELEFONO no es una copia: es 05_Funcional/App_Semaforo/js/nmea_parser.js
#     ejecutado con node. Si ese fichero deja de validar el checksum o deja de recuperar
#     un campo, esto se cae. Nada mas en el repositorio ejerce ese fichero -los
#     "unitarios" de la app llevan su PROPIO parser copiado dentro, asi que aprueban una
#     copia; y los packs del banco lo leen como texto, que es otra cosa-.
#   - El lado del MICRO si es un modelo en Python, y por eso todo lo que puede salir del
#     C++ sale del C++ en cada corrida: el PIN, el desplazamiento del prefijo, el formato
#     literal de la trama $STATUS y los literales $ACK/$ERR. Sin valor por defecto: si
#     algo no se puede extraer, esto ABORTA en vez de suponerlo (CLAUDE.md la seccion 3.bis).
#   - No mide el modulo Bluetooth fisico ni el firmware cargado en la tarjeta. Un PASS
#     de aqui dice que el protocolo cierra en el PC.
#
# POR QUE SE REESCRIBIO (censo del 31/08, tres agujeros medidos):
#
#   1. EL "5/5" ERA UN LITERAL. La linea final era un print fijo: el detector de la
#      cuarta cara de N-46 -que exige x == y sobre la cuenta publicada- no podia fallar
#      jamas sobre ella, porque no habia cuenta que comparar. Ahora cada comprobacion
#      suma y la ultima linea imprime el total contado.
#   2. LA PRUEBA DEL COURIER RTC ERA UNA TAUTOLOGIA: abs(x - x) == 0 con la MISMA
#      expresion a los dos lados. Retirada; el porque, abajo en el bloque COURIER.
#   3. EL FUZZING NO COMPROBABA NADA: contaba descartes y los imprimia, sin un solo
#      assert. Con el parser aceptando basura habria impreso "0/10000 descartadas" y la
#      suite habria seguido en verde. Ahora exige las DOS direcciones, que es lo que
#      falta casi siempre: que se rechace todo lo corrupto Y que NO se rechace lo bueno
#      -un parser que dice que no a todo tambien pasaria un umbral mal puesto-.
#
# Y una que no estaba en el censo pero salio al tirar del hilo: el cliente de la app era
# un tercer parser NMEA escrito a mano en Python. Se retiro entero. Un instrumento que
# compara dos copias suyas no mide el programa que se instala en el celular.

import json
import os
import random
import re
import shutil
import subprocess
import sys
import tempfile

RAIZ = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
BLUETOOTH_CPP = os.path.join(RAIZ, "01_Firmware", "Maestro", "src", "bluetooth.cpp")
APP_DIR = os.path.join(RAIZ, "05_Funcional", "App_Semaforo")
PARSER_JS = os.path.join(APP_DIR, "js", "nmea_parser.js")
APP_JS = os.path.join(APP_DIR, "app.js")

SEMILLA = 20260831  # un ataque que no se puede repetir no es evidencia


# ---------------------------------------------------------------------------
# Contador. Las mismas primitivas que los packs del banco, y por el mismo motivo:
# `reportar` NO cuenta, porque un hallazgo que ningun firmware puede "aprobar" no es
# una comprobacion (N-46, tercera cara).
# ---------------------------------------------------------------------------
_HECHAS = 0
_FALLIDAS = 0


class Abortar(Exception):
    """No se pudo medir. ABORTADO no es PASS, y tampoco es FALLA: no dice nada del
    firmware ni de la app."""


def verificar(nombre, condicion, detalle=""):
    global _HECHAS, _FALLIDAS
    _HECHAS += 1
    if condicion:
        print(f"   [OK] {nombre}" + (f" -> {detalle}" if detalle else ""))
    else:
        _FALLIDAS += 1
        print(f"   [FALLA] {nombre} -> {detalle}")
    return bool(condicion)


def control_negativo(nombre, condicion, detalle=""):
    """Exige que la comprobacion de al lado SEPA fallar. Cuenta igual que las demas."""
    return verificar(f"[control negativo] {nombre}", condicion, detalle)


def reportar(texto):
    print(f"   [nota] {texto}")


# ---------------------------------------------------------------------------
# Lectura de constantes. Todo lo que sigue sale del fuente en cada corrida.
# ---------------------------------------------------------------------------
def _leer(ruta, que):
    if not os.path.isfile(ruta):
        raise Abortar(f"no existe {que}: {ruta}")
    with open(ruta, "r", encoding="utf-8", errors="replace") as f:
        return f.read()


def extraer_del_firmware():
    """Devuelve el contrato que el C++ define hoy. Cualquier fallo aqui es ABORTADO:
    un banco que rellena con valores por defecto lo que no supo leer no demuestra
    nada (N-51)."""
    src = _leer(BLUETOOTH_CPP, "bluetooth.cpp del Maestro")

    m = re.search(r'strncmp\(\s*cmd\s*,\s*"(CMD:PIN:(\d+):)"\s*,\s*(\d+)\s*\)', src)
    if not m:
        raise Abortar("no se hallo el filtro de PIN strncmp(cmd, \"CMD:PIN:...\", n) en bluetooth.cpp")
    prefijo, pin, desplazamiento = m.group(1), m.group(2), int(m.group(3))

    fmt = re.search(r'"(\$STATUS,[^"]*)"', src)
    if not fmt:
        raise Abortar("no se hallo el formato literal de la trama $STATUS en bluetooth.cpp")

    literales = re.findall(r'enviarTramaConCrc\(\s*"(\$(?:ACK|ERR)[^"]*)"\s*\)', src)
    if not literales:
        raise Abortar("no se hallo ninguna respuesta literal $ACK/$ERR en bluetooth.cpp")

    # El despachador NO tiene tabla ni enum: el contrato ES la cadena de strcmp sobre
    # `accion`. Se censa igual que app_01_comandos, que es el pack que vigila esa
    # frontera; aqui solo se usa para saber contra que se compara el modelo.
    censo = set(re.findall(r'strcmp\(\s*accion\s*,\s*"([^"]+)"\s*\)', src))
    censo |= set(re.findall(r'strncmp\(\s*accion\s*,\s*"([^"]+)"\s*,\s*\d+\s*\)', src))
    if not censo:
        raise Abortar("no se hallo ninguna rama strcmp(accion, ...) en bluetooth.cpp")

    return {
        "prefijo": prefijo,
        "pin": pin,
        "desplazamiento": desplazamiento,
        "formato_status": fmt.group(1),
        "literales": literales,
        "censo": censo,
    }


def extraer_de_la_app():
    """El PIN que la app manda de verdad. No es el de config.js: app.js lleva el suyo
    en state.correctPin y es ese el que se pega al comando (app.js:199-206)."""
    js = _leer(APP_JS, "app.js de la aplicacion")
    m = re.search(r"correctPin\s*:\s*'(\d+)'", js)
    if not m:
        raise Abortar("no se hallo state.correctPin en app.js: sin el no se sabe que PIN manda la app")
    pin_app = m.group(1)

    cfg = _leer(os.path.join(APP_DIR, "js", "config.js"), "config.js de la aplicacion")
    m2 = re.search(r"DEFAULT_PIN\s*:\s*'(\d+)'", cfg)
    if not m2:
        raise Abortar("no se hallo DEFAULT_PIN en config.js")
    return pin_app, m2.group(1)


# ---------------------------------------------------------------------------
# Puente con el parser REAL de la app. node no es opcional: sin el no hay medida, y
# eso es ABORTADO (N-44: un instrumento que existe no es un instrumento que mide).
# ---------------------------------------------------------------------------
_PUENTE_JS = r"""
// Puente minimo. NO reimplementa nada: carga el parser que la app carga y le da de
// comer las tramas que el instrumento genera.
const fs = require('fs');
const P = require(process.argv[2]);
const tramas = JSON.parse(fs.readFileSync(process.argv[3], 'utf8'));
const salida = tramas.map(function (t) {
  let v;
  try { v = P.validarTrama(t); } catch (e) { return { valida: false, excepcion: String(e && e.message) }; }
  if (!v || !v.valida) return { valida: false };
  const r = { valida: true };
  try { const s = P.parseStatus(v.payload); if (s) r.status = s; }
  catch (e) { r.excepcion = String(e && e.message); }
  return r;
});
fs.writeFileSync(process.argv[4], JSON.stringify(salida));
"""


class ParserDeLaApp:
    def __init__(self):
        self.node = shutil.which("node")
        if self.node is None:
            raise Abortar("no hay node en el PATH: sin el no se puede ejercer el parser real de la app")
        if not os.path.isfile(PARSER_JS):
            raise Abortar(f"no existe el parser de la app: {PARSER_JS}")
        self.tmp = tempfile.mkdtemp(prefix="sim_app_bt_")
        self.puente = os.path.join(self.tmp, "puente.js")
        with open(self.puente, "w", encoding="utf-8") as f:
            f.write(_PUENTE_JS)
        # Antes de fiarse, se le exige que RESPONDA. Es lo mismo que hace compuerta.py
        # con gcc: no pregunta "hay node", le exige que enlace.
        #
        # Y se comprueba SOLO la tuberia -que vuelve un veredicto por trama, sin
        # excepcion-, nunca el CONTENIDO del veredicto. La primera version de esto
        # exigia que "$X*58" saliera valida y "basura" invalida, y al inyectar un parser
        # que acepta todo el instrumento salio ABORTADO en vez de FALLA: un defecto de
        # la app disfrazado de "no se pudo medir", que es la puerta abierta de la seccion 3.quater.
        # Quien juzga al parser son las comprobaciones que cuentan, no su portero.
        prueba = self.validar(["$X*58", "basura"])
        if len(prueba) != 2 or any("valida" not in r or "excepcion" in r for r in prueba):
            raise Abortar("el puente a nmea_parser.js no devuelve un veredicto por trama")

    def validar(self, tramas):
        entrada = os.path.join(self.tmp, "in.json")
        salida = os.path.join(self.tmp, "out.json")
        with open(entrada, "w", encoding="utf-8") as f:
            json.dump(tramas, f)
        p = subprocess.run([self.node, self.puente, PARSER_JS, entrada, salida],
                           capture_output=True, text=True, errors="replace")
        if p.returncode != 0 or not os.path.isfile(salida):
            err = (p.stderr or "").strip().splitlines()
            raise Abortar("node no pudo ejecutar el parser de la app"
                          + (f": {err[-1][:90]}" if err else ""))
        with open(salida, "r", encoding="utf-8") as f:
            return json.load(f)

    def cerrar(self):
        # Cada corrida vuelca varios MB de JSON al temporal. Sin esto la compuerta va
        # dejando un directorio por pasada.
        shutil.rmtree(self.tmp, ignore_errors=True)


def crc_nmea(payload):
    """XOR de 8 bits entre '$' y '*'. Es la misma cuenta que calcularChecksum() del
    C++ y que NMEAParser.calcularChecksum(): aqui hace falta para FABRICAR tramas, no
    para juzgarlas -de eso se encarga el parser real-."""
    c = 0
    for ch in payload:
        c ^= ord(ch)
    return "%02X" % c


def cerrar_trama(payload_con_dolar):
    return f"{payload_con_dolar}*{crc_nmea(payload_con_dolar[1:])}\r\n"


# ---------------------------------------------------------------------------
# El modelo del micro. Lo poco que sabe hacer lo hace con el contrato leido del C++.
# ---------------------------------------------------------------------------
_ESPECIFICADOR = re.compile(r"%[-+ #0-9.]*(?:hh|h|ll|l|z|j|t)?[a-zA-Z]")

# Como se compara cada campo de la trama con lo que el parser de la app devuelve.
# Si el firmware estrena un campo que no este aqui, esto ABORTA: preferimos no medir a
# medir de menos en silencio, que es como se pierden los campos nuevos.
CAMPOS = {
    "NODE":   ("node",     lambda v: v),
    "SERIE":  ("serie",    lambda v: v),
    "MODO":   ("modo",     lambda v: v),
    "ESTADO": ("estado",   lambda v: v),
    "T":      ("restante", lambda v: int(re.match(r"\d+", v).group(0))),
    "RF":     ("rf",       lambda v: int(re.match(r"\d+", v).group(0))),
    "RTT":    ("rtt",      lambda v: int(re.match(r"\d+", v).group(0))),
    "BAT":    ("bat",      lambda v: float(re.match(r"[\d.]+", v).group(0))),
    "HORA":   ("hora",     lambda v: v),
}


class MicroModelado:
    """Modelo del Maestro visto desde el cable Bluetooth. Solo estado; ningun texto
    del protocolo esta escrito aqui."""

    def __init__(self, contrato):
        self.c = contrato
        self.modo = "AUTO"
        self.estado = "V1_R2"
        self.serie = "M-2026-A1B2"
        self.rf = 98
        self.rtt = 85
        self.hora_seg = 14 * 3600 + 32 * 60 + 5   # 14:32:05
        self.dia = 26
        self.millis_ms = 0                        # el contador de 32 bits del micro
        self.tramas_emitidas = 0
        self.plantilla, self.claves = self._compilar_formato(contrato["formato_status"])

    @staticmethod
    def _compilar_formato(fmt):
        """Convierte el snprintf del C++ en una plantilla por CLAVE, no por posicion.
        Por posicion, reordenar dos campos en el firmware dejaria a este modelo
        rellenando el hueco equivocado sin que nada chillase."""
        piezas, claves = [], []
        for token in fmt.split(","):
            protegido = token.replace("%%", "\x00")
            especificadores = _ESPECIFICADOR.findall(protegido)
            if not especificadores:
                piezas.append(("literal", protegido))
                continue
            if len(especificadores) != 1:
                raise Abortar(f"el campo '{token}' de $STATUS lleva {len(especificadores)} "
                              f"especificadores; este instrumento solo sabe rellenar uno")
            clave = token.split(":", 1)[0]
            if clave not in CAMPOS:
                raise Abortar(f"el firmware emite el campo '{clave}' en $STATUS y este "
                              f"instrumento no sabe con que compararlo")
            piezas.append(("campo", clave, _ESPECIFICADOR.sub("{}", protegido, count=1)))
            claves.append(clave)
        return piezas, claves

    def valores(self):
        """Lo que el modelo cree que vale cada campo AHORA. Es el lado 'ida' del viaje
        de ida y vuelta: lo que salga del parser tiene que coincidir con esto."""
        h, resto = divmod(self.hora_seg, 3600)
        m, s = divmod(resto, 60)
        return {
            "SERIE": self.serie,
            "MODO": self.modo,
            "ESTADO": self.estado,
            "T": str((self.millis_ms // 1000) % 60),
            "RF": str(self.rf),
            "RTT": str(self.rtt),
            "HORA": f"{h:02d}:{m:02d}:{s:02d}",
        }

    def avanzar_segundo(self):
        self.hora_seg = (self.hora_seg + 1) % 86400
        if self.hora_seg == 0:
            self.dia = (self.dia % 31) + 1
        # 2^32 ms = 49,7 dias. El micro NO lo reinicia: da la vuelta, y como
        # 4.294.967,296 s no es multiplo de 60, la cuenta T: da un salto en cada
        # vuelta. Se modela porque una corrida de 180 dias la cruza tres veces.
        self.millis_ms = (self.millis_ms + 1000) % (2 ** 32)

    def trama_status(self):
        vals = self.valores()
        salida = []
        for pieza in self.plantilla:
            if pieza[0] == "literal":
                salida.append(pieza[1])
            else:
                salida.append(pieza[2].format(vals[pieza[1]]))
        self.tramas_emitidas += 1
        return cerrar_trama(",".join(salida).replace("\x00", "%"))

    def campos_emitidos(self):
        """Clave -> valor tal y como viaja en la trama, incluidos los que el C++ fija
        como literales (NODE:MAESTRO, BAT:12.6)."""
        vals = self.valores()
        emitidos = {}
        for pieza in self.plantilla:
            texto = pieza[1].replace("\x00", "%") if pieza[0] == "literal" \
                else pieza[2].format(vals[pieza[1]]).replace("\x00", "%")
            if ":" in texto and not texto.startswith("$"):
                k, v = texto.split(":", 1)
                emitidos[k] = v
        return emitidos

    def respuesta_literal(self, comando):
        """La contestacion que el C++ tiene escrita para ese comando. Si no aparece
        ninguna, el modelo esta desincronizado del firmware y eso es ABORTADO, no un
        'ERR' inventado: el instrumento ya no sabe con que compara."""
        candidatas = [l for l in self.c["literales"] if f",CMD:{comando}," in l]
        if not candidatas:
            raise Abortar(f"el modelo espera contestar a '{comando}' y bluetooth.cpp no "
                          f"tiene ningun literal $ACK/$ERR para ese comando")
        return candidatas[0]

    def procesar(self, crudo):
        """Solo la puerta: el PIN. Lo de dentro se modela para los comandos que este
        instrumento ejerce; el censo completo lo vigila el pack app_01_comandos, y
        reescribir aqui las 17 ramas seria una segunda copia del firmware a mano."""
        crudo = crudo.strip()
        if not crudo.startswith(self.c["prefijo"]):
            return cerrar_trama(self.respuesta_literal("AUTH_FAILED"))
        accion = crudo[self.c["desplazamiento"]:]

        if accion in ("SET_MODO:AUTO", "SET_MODO:MANUAL", "SET_MODO:AMBAR"):
            self.modo = accion.split(":")[1]
            if self.modo == "AMBAR":
                self.estado = "AMBAR_FAIL"
            return cerrar_trama(self.respuesta_literal(accion))
        if accion == "FORZAR_ROJO":
            self.estado = "R1_R2"
            return cerrar_trama(self.respuesta_literal(accion))
        if accion == "TEST_LEDS":
            return cerrar_trama(self.respuesta_literal(accion))
        return cerrar_trama(self.respuesta_literal("DESCONOCIDO"))


# ---------------------------------------------------------------------------
# Las comprobaciones
# ---------------------------------------------------------------------------
def comprobar_constantes(contrato, pin_app, pin_config):
    print("\n[>] CONSTANTES DEL PROTOCOLO (releidas del C++ y del .js en esta corrida)")

    verificar(
        "el prefijo de PIN y su desplazamiento dicen lo mismo",
        len(contrato["prefijo"]) == contrato["desplazamiento"],
        f'"{contrato["prefijo"]}" mide {len(contrato["prefijo"])} y el strncmp corta en '
        f'{contrato["desplazamiento"]}: si difieren, el micro parte el comando por donde no es')

    verificar(
        "el PIN que la app manda es el que el micro exige",
        pin_app == contrato["pin"],
        f'app.js manda {pin_app} y bluetooth.cpp exige {contrato["pin"]}'
        if pin_app != contrato["pin"] else f"los dos dicen {pin_app}")

    if pin_config != contrato["pin"]:
        reportar(f"config.js declara DEFAULT_PIN={pin_config} y el micro exige "
                 f'{contrato["pin"]}, pero app.js no lee esa constante: manda '
                 f"state.correctPin. Es una tercera copia que no gobierna nada.")
    else:
        reportar("config.js declara el mismo DEFAULT_PIN, pero app.js no lo lee: manda "
                 "state.correctPin. Es una copia que hoy coincide por casualidad.")


def comprobar_telemetria(micro, parser, dias=180, paso=1789):
    """Ida y vuelta: lo que el modelo pone en la trama tiene que salir del parser real.

    El paso de muestreo NO es multiplo de 60 a proposito. Con el 1800 de antes, T: caia
    siempre en el mismo valor y el campo se comprobaba una sola vez repetida 8.640
    veces."""
    print(f"\n[>] TELEMETRIA: {dias} dias de $STATUS contra js/nmea_parser.js (node)")

    hora_inicial = micro.hora_seg
    tramas, esperados = [], []
    segundos = dias * 86400
    for s in range(0, segundos, paso):
        for _ in range(paso):
            micro.avanzar_segundo()
        tramas.append(micro.trama_status())
        emitido = micro.campos_emitidos()
        # La referencia se calcula en CERRADO desde el indice del bucle; el modelo la
        # acumula segundo a segundo. Son dos caminos distintos al mismo numero: si la
        # acumulacion pierde una vuelta de reloj o de millis, dejan de coincidir.
        n = s + paso
        seg_ref = (hora_inicial + n) % 86400
        h, resto = divmod(seg_ref, 3600)
        m, sg = divmod(resto, 60)
        emitido["_HORA_REF"] = f"{h:02d}:{m:02d}:{sg:02d}"
        emitido["_T_REF"] = str(((n * 1000) % (2 ** 32)) // 1000 % 60)
        esperados.append(emitido)

    resultados = parser.validar(tramas)
    if len(resultados) != len(tramas):
        raise Abortar("el puente devolvio menos resultados que tramas enviadas")

    invalidas = [i for i, r in enumerate(resultados) if not r["valida"]]
    verificar(
        "el parser de la app acepta todas las tramas que el micro emite",
        not invalidas,
        f"{len(tramas)} tramas muestreadas a lo largo de {dias} dias; "
        + (f"{len(invalidas)} rechazadas (la primera: {tramas[invalidas[0]].strip()})"
           if invalidas else "0 rechazadas"))

    discrepancias = comparar_campos(resultados, esperados)
    verificar(
        "el parser recupera cada campo con el valor que viajaba en la trama",
        not discrepancias,
        f'{len(CAMPOS)} campos x {len(tramas)} tramas; ' +
        (f"{len(discrepancias)} discrepancias (la primera: {discrepancias[0]})"
         if discrepancias else "sin discrepancias, con 3 vueltas de millis() y "
         f"{dias} cambios de dia dentro"))

    verificar(
        "la hora y la cuenta de fase resisten 180 dias de acumulacion",
        not [e for e in esperados if e["HORA"] != e["_HORA_REF"] or e["T"] != e["_T_REF"]],
        "el reloj acumulado segundo a segundo coincide con la cuenta cerrada en las "
        f"{len(esperados)} muestras")

    # Que la comparacion de arriba SEPA fallar. Se altera un campo de una trama buena,
    # se le recalcula el CRC -o sea que es una trama perfectamente valida- y se exige
    # que la comparacion la delate. Sin esto, "sin discrepancias" no significa nada.
    buena = tramas[0]
    payload = buena.strip()[1:].split("*")[0]
    trucada = cerrar_trama("$" + payload.replace("MODO:AUTO", "MODO:MANUAL"))
    res_trucada = parser.validar([trucada])
    control_negativo(
        "un campo cambiado en una trama valida se detecta",
        bool(comparar_campos(res_trucada, esperados[:1])),
        "MODO:AUTO -> MODO:MANUAL con el CRC rehecho: la trama es valida y aun asi "
        "la comparacion de campos la rechaza")


def comparar_campos(resultados, esperados):
    fallos = []
    for r, e in zip(resultados, esperados):
        if not r["valida"] or "status" not in r:
            fallos.append("trama sin $STATUS parseado")
            continue
        st = r["status"]
        for clave, valor in e.items():
            if clave.startswith("_"):
                continue
            destino, normaliza = CAMPOS[clave]
            if destino not in st:
                fallos.append(f"{clave}: el parser no devuelve nada")
            elif st[destino] != normaliza(valor):
                fallos.append(f"{clave}: viajaba {valor!r} y el parser dio {st[destino]!r}")
    return fallos


def comprobar_barrera_pin(micro, contrato, intentos=50000):
    print(f"\n[>] BARRERA DE PIN: {intentos} intentos de fuerza bruta")
    rng = random.Random(SEMILLA)
    modo_antes = micro.modo
    lanzados = rechazados = 0
    for _ in range(intentos):
        candidato = f"{rng.randint(0, 9999):04d}"
        if candidato == contrato["pin"]:
            continue      # acertar el PIN no es un ataque: no cuenta en el denominador
        lanzados += 1
        resp = micro.procesar(f"CMD:PIN:{candidato}:SET_MODO:MANUAL")
        if "AUTH_FAILED" in resp:
            rechazados += 1

    verificar(
        "ningun PIN invalido atraviesa la barrera",
        rechazados == lanzados and micro.modo == modo_antes,
        f"{rechazados}/{lanzados} rechazados y el modo sigue en {micro.modo}")

    # La otra direccion, que es donde se esconden los muros: una barrera que rechaza
    # TODO tambien daria 100% arriba, y dejaria el equipo sin mando.
    resp = micro.procesar(f'{contrato["prefijo"]}SET_MODO:MANUAL')
    control_negativo(
        "el PIN correcto si entra",
        "RESULT:OK" in resp and micro.modo == "MANUAL",
        f"respuesta literal del C++: {resp.strip()}")
    micro.procesar(f'{contrato["prefijo"]}SET_MODO:AUTO')


def corpus_valido(micro, contrato):
    """Tramas que el equipo emite de verdad: telemetria y las respuestas literales del
    despachador, cerradas con su CRC."""
    corpus = [micro.trama_status()]
    for lit in contrato["literales"]:
        corpus.append(cerrar_trama(lit))
    return corpus


def comprobar_fuzzing(micro, contrato, parser, n=10000):
    print(f"\n[>] FUZZING DEL PARSER: {n} mutaciones + {n} de ruido + corpus valido")
    rng = random.Random(SEMILLA)
    base = corpus_valido(micro, contrato)

    # DIRECCION 1: lo bueno se acepta. Va primero porque es la que se olvida, y la que
    # convierte el 100% de la direccion 2 en una cifra que significa algo.
    res_base = parser.validar(base)
    rechazadas_buenas = [b for b, r in zip(base, res_base) if not r["valida"]]
    verificar(
        "el parser acepta TODAS las tramas legitimas del equipo",
        not rechazadas_buenas,
        f"{len(base)} tramas (1 de telemetria + {len(contrato['literales'])} respuestas "
        f"literales de bluetooth.cpp); " +
        (f"rechazadas {len(rechazadas_buenas)}, la primera {rechazadas_buenas[0].strip()}"
         if rechazadas_buenas else "0 rechazadas"))

    # DIRECCION 2: lo corrupto se rechaza, y el umbral es el 100% POR CONSTRUCCION, no
    # por ser un numero redondo. Las cinco clases de mutacion se eligieron porque un
    # XOR de 8 bits TIENE que verlas:
    #   - cambiar un byte: cambia el XOR (a ^ b != 0 si a != b).
    #   - borrar un byte imprimible: le quita su valor al XOR, que no es 0.
    #   - tocar un digito del CRC: el campo deja de coincidir con la cuenta.
    #   - quitar el '$' o cortar antes del '*': la trama deja de tener forma.
    # Lo que se deja FUERA a proposito son las permutaciones: intercambiar dos
    # caracteres NO cambia un XOR, asi que exigir que se detecten seria exigir lo
    # imposible -N-46, tercera cara: eso no es una comprobacion, es una nota-.
    mutadas, colados = [], 0
    for i in range(n):
        trama = base[rng.randrange(len(base))].strip()
        payload = trama[1:].split("*")[0]
        clase = i % 5
        if clase == 0:
            j = rng.randrange(len(payload))
            nuevo = chr(32 + ((ord(payload[j]) - 32 + rng.randint(1, 90)) % 95))
            mutadas.append(f"${payload[:j]}{nuevo}{payload[j + 1:]}*{crc_nmea(payload)}\r\n")
        elif clase == 1:
            j = rng.randrange(len(payload))
            mutadas.append(f"${payload[:j]}{payload[j + 1:]}*{crc_nmea(payload)}\r\n")
        elif clase == 2:
            crc = crc_nmea(payload)
            j = rng.randrange(2)
            otro = rng.choice([c for c in "0123456789ABCDEF" if c != crc[j]])
            mutadas.append(f"${payload}*{crc[:j]}{otro}{crc[j + 1:]}\r\n")
        elif clase == 3:
            mutadas.append(f"{payload}*{crc_nmea(payload)}\r\n")
        else:
            mutadas.append("$" + payload[:rng.randrange(1, len(payload))] + "\r\n")

    for r in parser.validar(mutadas):
        if r["valida"]:
            colados += 1
    verificar(
        "ninguna trama corrupta pasa por buena",
        colados == 0,
        f"{len(mutadas) - colados}/{len(mutadas)} descartadas (5 clases de mutacion que "
        f"un XOR de 8 bits tiene que ver por construccion)")
    reportar("permutar dos caracteres NO cambia un XOR de 8 bits: ese agujero lo obliga "
             "el protocolo y por eso no esta en el corpus. Si algun dia se cambia a un "
             "CRC-16, esta nota se convierte en una comprobacion mas.")

    # Ruido puro: lo que llega por un serial con interferencia. Aqui no hay garantia
    # matematica de rechazo -una cadena al azar podria dar un CRC bueno por casualidad-,
    # asi que la semilla es fija y el resultado, reproducible.
    ruido = []
    for _ in range(n):
        longitud = rng.randint(1, 80)
        ruido.append("".join(chr(rng.randint(32, 126)) for _ in range(longitud)))
    aceptadas = [t for t, r in zip(ruido, parser.validar(ruido)) if r["valida"]]
    verificar(
        "el ruido serial no se confunde con telemetria",
        not aceptadas,
        f"{n} cadenas de ASCII imprimible al azar (semilla {SEMILLA}); " +
        (f"{len(aceptadas)} aceptadas, la primera {aceptadas[0]!r}"
         if aceptadas else "0 aceptadas, y ninguna excepcion en el parser"))


def comprobar_respuestas(micro, contrato, parser):
    """El modelo no puede inventarse lo que el equipo contesta: cada respuesta suya es
    un literal que esta HOY en bluetooth.cpp, y el parser real tiene que reconocerla."""
    print("\n[>] RESPUESTAS AL MANDO: literales del C++ y su viaje de vuelta")
    resp = micro.procesar(f'{contrato["prefijo"]}FORZAR_ROJO')
    literal = resp.strip().split("*")[0]
    res = parser.validar([resp, micro.trama_status()])
    verificar(
        "el rojo total contesta con el literal del firmware y la app lo entiende",
        literal in contrato["literales"] and micro.estado == "R1_R2"
        and res[0]["valida"] and res[1]["valida"],
        f"{literal} (leido de bluetooth.cpp), estado {micro.estado}, y la telemetria "
        f"posterior sigue siendo valida")

    atendidos = sum(1 for c in ("SET_MODO:AUTO", "SET_MODO:MANUAL", "SET_MODO:AMBAR",
                                "FORZAR_ROJO", "TEST_LEDS") if c in contrato["censo"])
    reportar(f"este modelo ejerce {atendidos} de los {len(contrato['censo'])} comandos "
             f"censados en el despachador. No se completan los demas a proposito: "
             f"modelarlos con sus guardas seria una segunda copia del firmware escrita "
             f"a mano. El censo entero lo vigila el pack app_01_comandos.")


# ---------------------------------------------------------------------------
# COURIER RTC: por que aqui ya no hay ninguna comprobacion
#
# La que habia era esta, literal:
#
#     hora_inyeccion_calculada = hora_captura + segundos_viaje
#     error_desfase = abs(hora_inyeccion_calculada - (hora_captura + segundos_viaje))
#     assert error_desfase == 0
#
# La misma expresion a los dos lados del menos. Ningun firmware y ninguna app podian
# hacerla fallar: no media el Courier, media que la resta funciona. Es el mismo defecto
# que N-62 arreglo en test_funcional_app.py, dejado en el fichero de al lado.
#
# NO SE SUSTITUYE POR OTRA, y el motivo es que lo que pretendia medir YA SE MIDE, y se
# mide mejor de lo que se podria aqui:
#
#   - test_funcional_app.py (suite 4) exige sobre el js/courier_rtc.js REAL que la
#     inyeccion sume el tiempo transcurrido, que la interfaz llame al calculo, y lleva
#     dos controles negativos -uno de ellos para el caso peligroso de verdad: calcular
#     el viaje y no sumarlo-.
#   - test_unitarios_app.js ejerce calculateCourierCompensation con un viaje de 225 s.
#
# Reimplementar aqui la compensacion en Python seria una CUARTA copia del mismo
# algoritmo, y una copia que se compara consigo misma es exactamente de donde salio la
# tautologia. Una comprobacion menos y honesta vale mas que un PASS que no puede fallar.
# ---------------------------------------------------------------------------


def main():
    print("=" * 78)
    print(" APP MOVIL <-> FIRMWARE BLUETOOTH: protocolo ejercido contra el parser real")
    print("=" * 78)

    contrato = extraer_del_firmware()
    pin_app, pin_config = extraer_de_la_app()
    parser = ParserDeLaApp()
    micro = MicroModelado(contrato)

    try:
        comprobar_constantes(contrato, pin_app, pin_config)
        comprobar_telemetria(micro, parser)
        comprobar_barrera_pin(micro, contrato)
        comprobar_fuzzing(micro, contrato, parser)
        comprobar_respuestas(micro, contrato, parser)
    finally:
        parser.cerrar()

    print("\n" + "=" * 78)
    print(f"RESULTADO GLOBAL: {_HECHAS - _FALLIDAS}/{_HECHAS} comprobaciones PASS "
          f"(app movil vs firmware Bluetooth)")
    print("AVISO: esto mide el protocolo en el PC. El lado del micro es un modelo; el")
    print("       lado del telefono es el nmea_parser.js real. No sustituye la prueba")
    print("       con el modulo Bluetooth fisico delante: ver ESTADO.md, tarea BANCO.")
    print("=" * 78)
    return 1 if _FALLIDAS else 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except Abortar as e:
        # ABORTADO no es PASS y tampoco es FALLA: no se pudo medir, y eso no dice nada
        # del firmware ni de la app.
        # Sin linea de cuenta: una cifra x/y aqui invitaria a leer un total donde no se
        # midio nada.
        print(f"\n[ABORTADO] {e}")
        sys.exit(2)
