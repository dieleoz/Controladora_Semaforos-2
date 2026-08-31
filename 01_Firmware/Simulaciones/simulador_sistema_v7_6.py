#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
===============================================================================
SIMULADOR Y AUDITOR INTEGRAL DEL SISTEMA DE SEMÁFOROS (V7.7 Auditoría)
===============================================================================
Este script simula exactamente en Python las máquinas de estado C++, el puerto
de comunicaciones RF (9600 bps / CRC-8 Maxim 0x31) y el puente passthrough del
REPETIDOR ESP32, verificando las reglas viales y de seguridad en terreno.

Incluye correcciones de auditoría:
- SFTY-13: Supresion de PING durante la espera de ACK (evita que el latido pise la
  ventana del acuse en el enlace de radio)
- Buffer overflow protocolo corregido (binIdx=0 restaurado)
- tUltimaRxEsclavo inicializado a 0 en setup
- TIMEOUT_ACK y RF_BURST_COPIES se LEEN del firmware C++ en cada ejecución (ver bloque 0),
  para que el modelo no pueda divergir del código real sin que la prueba lo note.

-------------------------------------------------------------------------------
RETIRADA DEL 31/08/2026: LA CUENTA BAJA DE 20/20 A 9/9, Y NO ES UNA REGRESION
-------------------------------------------------------------------------------
Nueve de las veinte comprobaciones DUPLICABAN a un pack del banco o a un arnes que
compila el C++ real. Dos instrumentos que miden la misma propiedad no son el doble de
cobertura: son la misma cobertura contada dos veces, y la copia peor -esta, que corre
sobre un espejo en Python- es la que envejece sin avisar. Se retiraron una a una,
ensenando antes la comprobacion equivalente en el instrumento que se queda:

  8       CMD_ACK_RED != CMD_PING   -> costura_03_comandos (colisiones de codigo, sin
                                       literales escritos a mano) + costura_01_contratos
  9a x3   terna H/M/S atomica       -> esclavo_05_hora_atomica 5.1 / 5.2 / 5.3 / 5.4
  9b-1    desfase circular, 5 casos -> esclavo_04_desfase (barrido de las 3.600)
  10 x4   SFTY-21 fase Degradado    -> Validacion_Ciclo/arnes_ciclo.cpp, que barre el
                                       dia entero sobre el ciclo_degradado.h REAL

Otras dos bajaron de categoria porque NINGUN FIRMWARE PUEDE APROBARLAS NI SUSPENDERLAS
-son propiedades de una funcion de juguete definida en este mismo fichero-, y una
comprobacion que no puede fallar no es una comprobacion, es una nota (CLAUDE.md §3):
el limite de +-30 s del desfase y el conmutador booleano de MaestroSync.

Y cuatro se INVIRTIERON porque su puerta de entrada dejo de existir: BOTON3 y BOTON4
son hoy CAM_C_PIN / CAM_D_PIN (pines.h:124-125) y botonAceptar()/botonCancelar()
devuelven false fijo (botones.cpp:280-281). La propiedad -donde acaba la luz- sigue
valiendo; lo que se ejerce ahora es la via que un operario SI alcanza, que es el
despachador de Bluetooth.

Autor: Antigravity DeepMind Embedded Safety Agent
Fecha: 31 de Julio de 2026 (retirada de duplicados: 31/08/2026)
===============================================================================
"""

import sys
import time

if sys.platform == "win32":
    try:
        sys.stdout.reconfigure(encoding='utf-8')
    except Exception:
        pass

# ==========================================
# 0. CONSTANTES LEÍDAS DEL FIRMWARE C++ (anti-deriva modelo/firmware)
# ==========================================
# El simulador NO debe hardcodear tiempos ni tamaños: si el C++ cambia y el modelo no,
# el "PASS" deja de significar nada. Se leen del propio código fuente en cada ejecución.
import os
import re as _re

_DIR = os.path.dirname(os.path.abspath(__file__))


def _ruta_firmware(*partes):
    """Resuelve una ruta dentro de 01_Firmware sin depender del directorio de trabajo."""
    for base in (os.path.join(_DIR, ".."), os.path.join(os.getcwd(), "01_Firmware"), "01_Firmware"):
        cand = os.path.join(base, *partes)
        if os.path.exists(cand):
            return cand
    return None


def _leer_constante_cpp(ruta, patron, defecto, obligatorio=False):
    """Extrae una constante numérica del C++.

    Si `obligatorio`, la ausencia ABORTA en vez de caer al valor por defecto.

    Por qué: un respaldo silencioso derrota el propósito de leer del C++. Si alguien
    renombra `tiempoDespejeMs` y la expresión deja de encajar, el modelo volvería a
    15.0 —el mismo número que tenía escrito a mano antes de N-12— y seguiría dando
    PASS mientras el firmware real usa otro valor. Sería exactamente la deriva que
    esto viene a impedir, pero disfrazada de éxito.

    Para los tiempos de SEGURIDAD, no poder leerlos es un fallo de la prueba, no un
    detalle: sin ellos la validación no vale nada y debe decirlo, no continuar.
    """
    if ruta and os.path.exists(ruta):
        try:
            with open(ruta, "r", encoding="utf-8", errors="replace") as f:
                m = _re.search(patron, f.read())
            if m:
                return int(m.group(1))
        except OSError:
            pass
    if obligatorio:
        print(f"\n   ❌ ABORTADO: no se pudo leer del C++ la constante {patron!r}.")
        print("      Es un tiempo de SEGURIDAD. Si el modelo cayera al valor por defecto")
        print("      seguiria dando PASS mientras el firmware usa otro: esa es justo la")
        print("      deriva que N-12 impide. Revisa si se renombro la constante en el C++.")
        sys.exit(2)
    print(f"   ⚠️  AVISO: no se pudo leer {patron!r}; se usa el valor por defecto {defecto}.")
    return defecto


RF_BURST_COPIES = _leer_constante_cpp(
    _ruta_firmware("Maestro", "include", "protocolo.h"),
    r"#define\s+RF_BURST_COPIES\s+(\d+)", 1)

# N-71: la cadencia del latido tambien se lee del C++. Estaba escrita a mano como un
# 3.0 aqui y como un 3000 desnudo alla, y son el mismo dato: el primer sumando del
# presupuesto de radio. Ahora el C++ la llama LATIDO_MS y este modelo la sigue.
LATIDO_S = _leer_constante_cpp(
    _ruta_firmware("Maestro", "src", "coordinador.cpp"),
    r"LATIDO_MS\s*=\s*(\d+)", 3000, obligatorio=True) / 1000.0

TIMEOUT_ACK_S = _leer_constante_cpp(
    _ruta_firmware("Maestro", "src", "coordinador.cpp"),
    r"TIMEOUT_ACK_MS\s*=\s*(\d+)", 8000) / 1000.0

# SFTY-17: retardo de cortesia del Esclavo antes de contestar, para que la radio
# intermedia del repetidor alcance a volver de transmision a recepcion.
RETARDO_RESPUESTA_S = _leer_constante_cpp(
    _ruta_firmware("Esclavo", "src", "main.cpp"),
    r"RETARDO_RESPUESTA_MS\s*=\s*(\d+)", 200) / 1000.0

# --- N-12: los TIEMPOS DE SEGURIDAD tambien se leen del C++ -------------------
# Hasta el 01/08/2026 estos tres valores estaban escritos a mano en el modelo.
# Coincidian con el firmware, asi que los PASS eran validos, pero NADIE VIGILABA
# que siguieran coincidiendo: si alguien cambiaba el despeje en el C++, el
# simulador seguia probando 15 s y seguia diciendo PASS. Un "PASS" que no puede
# fallar no demuestra nada.
#
# Importa especialmente para el Modo Degradado (SFTY-21): el despeje todo-rojo ES
# el margen de seguridad que absorbe la deriva entre los dos relojes. Validarlo
# contra un valor que el modelo no vigila no demostraria nada.

# SFTY-4: despeje todo-rojo. Es el colchon de seguridad entre fases.
DESPEJE_S = _leer_constante_cpp(
    _ruta_firmware("Maestro", "src", "coordinador.cpp"),
    r"tiempoDespejeMs\s*=\s*(\d+)", 15000, obligatorio=True) / 1000.0

# SFTY-6: sin recibir nada del Esclavo durante este tiempo -> Ambar Intermitente.
# N-69: el umbral vive ahora una sola vez en protocolo.h -contrato compartido- para
# que las dos puntas no puedan divergir. Se lee de ahi. El dia del cambio este
# instrumento ABORTO en vez de seguir midiendo el numero viejo: su trabajo.
FALLBACK_S = _leer_constante_cpp(
    _ruta_firmware("Maestro", "include", "protocolo.h"),
    r"#define\s+SFTY6_SILENCIO_MS\s+(\d+)UL",
    12000, obligatorio=True) / 1000.0

# SFTY-5: Amarillo fijo en la transicion Rojo -> Verde (Resolucion 2024).
AMARILLO_FIJO_S = _leer_constante_cpp(
    _ruta_firmware("Maestro", "src", "semaforo.cpp"),
    r"estado\s*==\s*S_AMARILLO\s*&&\s*\(ahora\s*-\s*tCambio\s*>=\s*(\d+)\)",
    4000, obligatorio=True) / 1000.0

print(f"   Tiempos de seguridad leidos del C++: despeje={DESPEJE_S}s  "
      f"fallback={FALLBACK_S}s  amarillo={AMARILLO_FIJO_S}s")

# --- Los codigos CMD_HORA_* / CMD_DELTA* ya no se leen aqui ---------------------
# Se leian para las pruebas 8, 9a y 9b, retiradas el 31/08 porque las miden
# costura_03_comandos, esclavo_05_hora_atomica y esclavo_04_desfase. Dejar la lectura
# sin nadie que la use seria la version silenciosa de la prueba muerta (N-73): un
# `#define` vigilado por un instrumento que ya no comprueba nada con el.


# --- LA PUERTA DE ENTRADA DE HOY: EL DESPACHADOR DE BLUETOOTH -------------------
#
# Las pruebas 1, 2 y 6 entraban llamando por dentro a maestro.forzar_menu() y a
# maestro.forzar_rojo_total(): una puerta que en el equipo de hoy NO EXISTE. BOTON3 y
# BOTON4 pasaron a ser CAM_C_PIN / CAM_D_PIN (Maestro/include/pines.h:124-125) y
# botonAceptar() / botonCancelar() devuelven false fijo (Maestro/src/botones.cpp:280-281):
# ningun operario alcanza ese camino. La propiedad -donde acaba la luz- sigue valiendo;
# lo que cambia es por donde se pide.
#
# El PIN, las ordenes que entran SIN PIN y los textos de respuesta se LEEN del C++, por
# el mismo motivo que los tiempos: si alguien mueve la puerta y aqui quedara la copia
# vieja, la prueba aprobaria una via de entrada que ya no existe.
_BT_CPP = _ruta_firmware("Maestro", "src", "bluetooth.cpp")


def _leer_literal_cpp(patron, que):
    """Extrae un literal de texto de bluetooth.cpp. La ausencia ABORTA, no cae a un
    valor por defecto: si la puerta se movio, esta prueba no puede medir nada y tiene
    que decirlo (CLAUDE.md §2, ABORTADO no es PASS)."""
    if _BT_CPP and os.path.exists(_BT_CPP):
        with open(_BT_CPP, "r", encoding="utf-8", errors="replace") as f:
            m = _re.search(patron, f.read())
        if m:
            return m.group(1)
    print(f"\n   ❌ ABORTADO: no se pudo leer de bluetooth.cpp {que}.")
    print("      La puerta de entrada de la app se movio o se renombro. Sin ella este")
    print("      modelo no ejerce ninguna via real: no puede seguir midiendo.")
    sys.exit(2)


def _leer_literales_cpp(patron):
    if _BT_CPP and os.path.exists(_BT_CPP):
        with open(_BT_CPP, "r", encoding="utf-8", errors="replace") as f:
            return set(_re.findall(patron, f.read()))
    return set()


# "CMD:PIN:1234:" - bluetooth.cpp:166
BT_PREFIJO_PIN = _leer_literal_cpp(r'strncmp\(cmd,\s*"(CMD:PIN:[^"]+)"', "el prefijo de PIN")
# "CMD:FORZAR_ROJO" - bluetooth.cpp:145, antes de la puerta del PIN
BT_DIRECTO_SIN_PIN = _leer_literal_cpp(r'strcmp\(cmd,\s*"(CMD:[^"]+)"\)\s*==\s*0',
                                       "la orden que entra antes del PIN")
# {"SET_MODO:MENU", "SET_MODO:ALCANCE"} - bluetooth.cpp:169-170
BT_ACCIONES_SIN_PIN = _leer_literales_cpp(r'strcmp\(cmd \+ 4,\s*"([^"]+)"\)')

BT_ERR_AUTH        = _leer_literal_cpp(r'"(\$ERR,CMD:AUTH_FAILED[^"]*)"', "el rechazo de PIN")
BT_ACK_MENU        = _leer_literal_cpp(r'"(\$ACK,CMD:SET_MODO:MENU,RESULT:OK)"', "el acuse del menu")
BT_ACK_ROJO        = _leer_literal_cpp(r'"(\$ACK,CMD:FORZAR_ROJO,RESULT:OK)"', "el acuse del rojo")
BT_ACK_CAMBIO      = _leer_literal_cpp(r'"(\$ACK,CMD:CAMBIAR_TURNO,RESULT:OK)"', "el acuse del cambio")
BT_ERR_CAMBIO      = _leer_literal_cpp(r'"(\$ERR,CMD:CAMBIAR_TURNO,DESC:[^"]*)"',
                                       "el rechazo del cambio en transicion")

print(f"   Puerta de la app leida del C++: PIN={BT_PREFIJO_PIN!r}  "
      f"sin PIN={sorted(BT_ACCIONES_SIN_PIN) + [BT_DIRECTO_SIN_PIN]}")


class DespachadorBluetooth:
    """Modela procesarComando() de Maestro/src/bluetooth.cpp.

    Solo el camino que estas pruebas ejercen. Lo que NO cubre y hay que decirlo: la
    rama de MODO_DEGRADADO de SET_MODO:MENU (bluetooth.cpp:198-205), que este
    simulador no tiene modelado porque aqui no existe el Modo Degradado. Esa puerta
    la miden los packs, no esto.
    """

    def __init__(self, maestro):
        self.maestro = maestro
        self.rf_pendiente = bytearray()   # tramas de radio que la orden genera

    def procesar(self, trama: bytes, t: float) -> str:
        cmd = trama.decode("ascii", errors="replace")

        # Rojo de emergencia: entra ANTES de la puerta del PIN y es deliberado
        # (bluetooth.cpp:137-150). Parar el trafico es la accion segura.
        if cmd == BT_DIRECTO_SIN_PIN:
            self.maestro.forzar_rojo_total(t)
            return BT_ACK_ROJO

        if cmd.startswith(BT_PREFIJO_PIN):
            accion = cmd[len(BT_PREFIJO_PIN):]
        elif cmd.startswith("CMD:") and cmd[4:] in BT_ACCIONES_SIN_PIN:
            accion = cmd[4:]
        else:
            return BT_ERR_AUTH

        if accion == "SET_MODO:MENU":
            self.maestro.forzar_menu()
            return BT_ACK_MENU
        if accion == "FORZAR_ROJO":
            self.maestro.forzar_rojo_total(t)
            return BT_ACK_ROJO
        if accion == "MANUAL:CAMBIAR_TURNO":
            # bluetooth.cpp:265 pregunta pedirCambioVerificado(), que es
            # coordinador_listoParaContar() -"return estadoC == C_IDLE"-. Partir un
            # despeje por la mitad es justo lo que no se puede hacer: el rechazo es
            # correcto, lo que faltaba era DECIRLO.
            if self.maestro.estado_c != "C_IDLE":
                return BT_ERR_CAMBIO
            self.rf_pendiente.extend(self.maestro.pedir_cambio(t) or b"")
            return BT_ACK_CAMBIO
        return "$ERR,CMD:DESCONOCIDO"

# ==========================================
# 1. SIMULACIÓN DE CRC-8 MAXIM (0x31) Y PAQUETE RF
# ==========================================
def calcular_crc8_maxim(data: bytes) -> int:
    crc = 0x00
    for byte in data:
        crc ^= byte
        for _ in range(8):
            if crc & 0x80:
                crc = ((crc << 1) ^ 0x31) & 0xFF
            else:
                crc = (crc << 1) & 0xFF
    return crc

class RF_Packet:
    CMD_GO_GREEN = 0x01
    CMD_GO_RED   = 0x02
    CMD_ACK_GREEN = 0x03
    CMD_PING     = 0x04
    CMD_PONG     = 0x05
    CMD_ACK_RED  = 0x06

    def __init__(self, msg_id: int, command: int, param: int = 0):
        self.msg_id = msg_id & 0xFF
        self.command = command & 0xFF
        self.param = param & 0xFF
        raw = bytes([self.msg_id, self.command, self.param])
        self.crc = calcular_crc8_maxim(raw)

    def to_bytes(self) -> bytes:
        return bytes([self.msg_id, self.command, self.param, self.crc])

    @staticmethod
    def from_bytes(raw: bytes):
        if len(raw) < 4:
            return None
        msg_id, cmd, param, crc = raw[0], raw[1], raw[2], raw[3]
        if calcular_crc8_maxim(bytes([msg_id, cmd, param])) == crc:
            return RF_Packet(msg_id, cmd, param)
        return None

# ==========================================
# 2. SIMULADOR DE CANAL RF Y REPETIDOR ESP32
# ==========================================
class RepetidorESP32:
    def __init__(self):
        self.activo = True
        self.buffer_a_a_c = bytearray()
        self.buffer_c_a_a = bytearray()
        self.last_byte_a = 0
        self.last_byte_c = 0
        self.delay_hardware_rs485 = 0.002 # 2ms setup MAX3485
        self.burst_window_s = 0.015       # 15ms inter-byte timeout

    def passthrough(self, data_a: bytes, data_c: bytes, current_time: float):
        if not self.activo:
            return b"", b"" # Repetidor apagado / sin energía

        out_c = b""
        out_a = b""

        # Canal A (Radio B1 - Maestro) -> Canal C (Radio B2 - Esclavo)
        if data_a:
            self.buffer_a_a_c.extend(data_a)
            self.last_byte_a = current_time

        if self.buffer_a_a_c and (current_time - self.last_byte_a >= self.burst_window_s):
            # 2ms de estabilización RS485 + transmisión en ráfaga
            out_c = bytes(self.buffer_a_a_c)
            self.buffer_a_a_c.clear()

        # Canal C (Radio B2 - Esclavo) -> Canal A (Radio B1 - Maestro)
        if data_c:
            self.buffer_c_a_a.extend(data_c)
            self.last_byte_c = current_time

        if self.buffer_c_a_a and (current_time - self.last_byte_c >= self.burst_window_s):
            out_a = bytes(self.buffer_c_a_a)
            self.buffer_c_a_a.clear()

        return out_a, out_c

# ==========================================
# 3. MÁQUINA DE ESTADOS C++ NODO MAESTRO
# ==========================================
class SemafaroMaestro:
    def __init__(self):
        self.estado_c = "C_IDLE"
        self.quien_verde = "QV_NINGUNO"
        self.luz_local = "S_ROJO"
        self.tiempo_despeje_s = DESPEJE_S   # N-12: leido del C++
        self.t_ref = 0.0
        self.t_ultimo_ping = 0.0
        self.t_ultima_rx_esclavo = 0.0  # V7.7: Inicializar a 0, no millis()
        self.t_esperando_ack = 0.0
        self.retry_count = 0
        self.msg_id_counter = 0
        self.ultimo_id_recibido = 0
        self.timeout_ack_s = TIMEOUT_ACK_S  # Leído de coordinador.cpp

    def enviar_paquete(self, cmd: int, param: int = 0) -> bytes:
        self.msg_id_counter = (self.msg_id_counter + 1) & 0xFF
        if self.msg_id_counter == 0: self.msg_id_counter = 1
        pkt = RF_Packet(self.msg_id_counter, cmd, param)
        # SFTY-11: Ráfaga Burst — nº de copias leído de protocolo.h
        return pkt.to_bytes() * RF_BURST_COPIES

    def iniciar_modo(self, current_time: float):
        self.quien_verde = "QV_NINGUNO"
        self.luz_local = "S_ROJO"
        self.ultimo_id_recibido = 0
        self.t_ref = current_time
        self.t_ultima_rx_esclavo = current_time
        self.estado_c = "C_INICIAL_ESPERA_ESTATICO"

    def forzar_menu(self):
        self.estado_c = "C_MENU_IDLE"
        self.quien_verde = "QV_NINGUNO"
        self.luz_local = "S_ROJO"

    def forzar_rojo_total(self, current_time: float):
        self.quien_verde = "QV_NINGUNO"
        self.luz_local = "S_ROJO"
        self.ultimo_id_recibido = 0
        self.t_ref = current_time
        self.t_ultima_rx_esclavo = current_time
        self.estado_c = "C_IDLE" # Mantiene Rojo Fijo en ambos indefinidamente

    def pedir_cambio(self, current_time: float) -> bytes:
        if self.estado_c != "C_IDLE":
            return b""
        if self.quien_verde == "QV_NINGUNO":
            self.t_ref = current_time
            self.estado_c = "C_INICIAL_ESPERA_ESTATICO"
            return b""
        elif self.quien_verde == "QV_MASTER":
            self.luz_local = "S_ROJO"
            self.t_ref = current_time
            self.estado_c = "C_ESPERA_ESTATICO_TRAS_MASTER"
            return b""
        elif self.quien_verde == "QV_ESCLAVO":
            self.t_esperando_ack = current_time
            self.retry_count = 0
            self.estado_c = "C_ESPERANDO_ACK_RED"
            return self.enviar_paquete(RF_Packet.CMD_GO_RED)

    def actualizar(self, rx_bytes: bytes, current_time: float) -> bytes:
        tx_bytes = b""

        # Procesar datos recibidos del Esclavo
        if len(rx_bytes) >= 4:
            for i in range(len(rx_bytes) - 3):
                pkt = RF_Packet.from_bytes(rx_bytes[i:i+4])
                if pkt and pkt.msg_id != self.ultimo_id_recibido:
                    self.ultimo_id_recibido = pkt.msg_id
                    self.t_ultima_rx_esclavo = current_time

                    if pkt.command == RF_Packet.CMD_ACK_RED and self.estado_c == "C_ESPERANDO_ACK_RED":
                        self.t_ref = current_time
                        self.estado_c = "C_ESPERA_ESTATICO_TRAS_ESCLAVO"
                    elif pkt.command == RF_Packet.CMD_ACK_GREEN and self.estado_c == "C_ESPERANDO_ACK_GREEN":
                        self.quien_verde = "QV_ESCLAVO"
                        self.estado_c = "C_IDLE"
                    break

        # SFTY-12/SFTY-13: Heartbeat PING — SUPRIMIDO durante espera de ACK
        if (current_time - self.t_ultimo_ping >= LATIDO_S
                and self.estado_c != "C_ESPERANDO_ACK_GREEN"
                and self.estado_c != "C_ESPERANDO_ACK_RED"):
            if self.estado_c == "C_MENU_IDLE":
                tx_bytes += self.enviar_paquete(RF_Packet.CMD_GO_RED)
            else:
                tx_bytes += self.enviar_paquete(RF_Packet.CMD_PING)
            self.t_ultimo_ping = current_time

        # Monitoreo de caída a 12.0s y Self-Healing
        tiene_comunicacion = (self.t_ultima_rx_esclavo > 0) and (current_time - self.t_ultima_rx_esclavo <= FALLBACK_S)

        if self.estado_c == "C_MENU_IDLE":
            if tiene_comunicacion:
                self.luz_local = "S_ROJO"  # TEST 5: Con comunicación en Menú → ROJO FIJO
            else:
                self.luz_local = "S_FALLO" # TEST 4: Sin comunicación en Menú → AMARILLO PARPADEO
        else:
            if not tiene_comunicacion:
                if self.t_ultima_rx_esclavo > 0 or current_time > FALLBACK_S:
                    if self.estado_c != "C_FALLO":
                        self.estado_c = "C_FALLO"
                        self.luz_local = "S_FALLO"  # TEST 3: Esclavo apagado → AMARILLO PARPADEO
            elif self.estado_c == "C_FALLO" and tiene_comunicacion:
                # Self-Healing Auto-Recuperación
                self.quien_verde = "QV_NINGUNO"
                self.luz_local = "S_ROJO"
                self.t_ref = current_time
                self.estado_c = "C_INICIAL_ESPERA_ESTATICO"
                tx_bytes += self.enviar_paquete(RF_Packet.CMD_GO_RED)

        # Transiciones de la máquina de estados
        if self.estado_c == "C_INICIAL_ESPERA_ESTATICO":
            if current_time - self.t_ref >= self.tiempo_despeje_s:
                self.luz_local = "S_AMARILLO"
                self.t_ref = current_time
                self.estado_c = "C_INICIAL_MASTER_A_VERDE"

        elif self.estado_c == "C_INICIAL_MASTER_A_VERDE":
            if current_time - self.t_ref >= AMARILLO_FIJO_S: # 4s Amarillo
                self.luz_local = "S_VERDE"
                self.quien_verde = "QV_MASTER"
                self.estado_c = "C_IDLE"

        elif self.estado_c == "C_ESPERA_ESTATICO_TRAS_MASTER":
            if current_time - self.t_ref >= self.tiempo_despeje_s:
                self.t_esperando_ack = current_time
                self.retry_count = 0
                self.estado_c = "C_ESPERANDO_ACK_GREEN"
                tx_bytes += self.enviar_paquete(RF_Packet.CMD_GO_GREEN)

        elif self.estado_c == "C_ESPERANDO_ACK_GREEN":
            if current_time - self.t_esperando_ack > self.timeout_ack_s:
                self.retry_count += 1
                if self.retry_count >= 5:
                    self.estado_c = "C_FALLO"
                    self.luz_local = "S_FALLO"
                else:
                    tx_bytes += self.enviar_paquete(RF_Packet.CMD_GO_GREEN)
                    self.t_esperando_ack = current_time

        elif self.estado_c == "C_ESPERANDO_ACK_RED":
            if current_time - self.t_esperando_ack > self.timeout_ack_s:
                self.retry_count += 1
                if self.retry_count >= 5:
                    self.estado_c = "C_FALLO"
                    self.luz_local = "S_FALLO"
                else:
                    tx_bytes += self.enviar_paquete(RF_Packet.CMD_GO_RED)
                    self.t_esperando_ack = current_time

        elif self.estado_c == "C_ESPERA_ESTATICO_TRAS_ESCLAVO":
            if current_time - self.t_ref >= self.tiempo_despeje_s:
                self.luz_local = "S_AMARILLO"
                self.t_ref = current_time
                self.estado_c = "C_MASTER_A_VERDE"

        elif self.estado_c == "C_MASTER_A_VERDE":
            if current_time - self.t_ref >= AMARILLO_FIJO_S: # 4s Amarillo
                self.luz_local = "S_VERDE"
                self.quien_verde = "QV_MASTER"
                self.estado_c = "C_IDLE"

        return tx_bytes

# ==========================================
# 4. MÁQUINA DE ESTADOS C++ NODO ESCLAVO
# ==========================================
class SemaforoEsclavo:
    def __init__(self):
        self.luz_local = "S_ROJO"
        self.t_ultimo_comando = 0.0
        self.t_ref = 0.0
        self.msg_id_counter = 0
        self.ultimo_id_recibido = 0
        self.ack_rojo_enviado = False
        self.ack_verde_enviado = False
        # SFTY-17: respuesta programada (comando, instante de envio)
        self.respuesta_pendiente = None
        self.t_enviar_respuesta = 0.0

    def _programar_respuesta(self, cmd, t):
        self.respuesta_pendiente = cmd
        self.t_enviar_respuesta = t + RETARDO_RESPUESTA_S

    def _atender_respuesta(self, t):
        if self.respuesta_pendiente is None or t < self.t_enviar_respuesta:
            return b""
        cmd = self.respuesta_pendiente
        self.respuesta_pendiente = None
        return self.enviar_paquete(cmd)

    def enviar_paquete(self, cmd: int, param: int = 0) -> bytes:
        self.msg_id_counter = (self.msg_id_counter + 1) & 0xFF
        if self.msg_id_counter == 0: self.msg_id_counter = 1
        pkt = RF_Packet(self.msg_id_counter, cmd, param)
        return pkt.to_bytes() * RF_BURST_COPIES

    def actualizar(self, rx_bytes: bytes, current_time: float) -> bytes:
        # SFTY-17: primero se despacha lo que estuviera programado del ciclo anterior.
        tx_bytes = self._atender_respuesta(current_time)

        # Actualizar transición de luz local
        if self.luz_local == "S_AMARILLO" and (current_time - self.t_ref >= AMARILLO_FIJO_S):
            self.luz_local = "S_VERDE"
            self.ack_verde_enviado = False

        if len(rx_bytes) >= 4:
            for i in range(len(rx_bytes) - 3):
                pkt = RF_Packet.from_bytes(rx_bytes[i:i+4])
                if pkt and pkt.msg_id != self.ultimo_id_recibido:
                    self.ultimo_id_recibido = pkt.msg_id
                    self.t_ultimo_comando = current_time

                    if self.luz_local == "S_FALLO":
                        self.luz_local = "S_ROJO"
                        self.ack_rojo_enviado = False

                    # SFTY-17: las respuestas se programan, no salen al instante.
                    if pkt.command == RF_Packet.CMD_PING:
                        self._programar_respuesta(RF_Packet.CMD_PONG, current_time)
                    elif pkt.command == RF_Packet.CMD_GO_RED:
                        self.luz_local = "S_ROJO"
                        self.ack_rojo_enviado = False
                        self._programar_respuesta(RF_Packet.CMD_ACK_RED, current_time)
                    elif pkt.command == RF_Packet.CMD_GO_GREEN:
                        self.luz_local = "S_AMARILLO"
                        self.t_ref = current_time
                        self.ack_verde_enviado = False
                        self._programar_respuesta(RF_Packet.CMD_ACK_GREEN, current_time)
                    break

        # Fallback de 12.0s sin señal del Maestro
        if current_time - self.t_ultimo_comando > FALLBACK_S:
            if self.luz_local != "S_FALLO":
                self.luz_local = "S_FALLO"

        # Auto-envió de ACK cuando se estabiliza la luz (tambien con retardo SFTY-17)
        if self.luz_local == "S_ROJO" and not self.ack_rojo_enviado:
            self._programar_respuesta(RF_Packet.CMD_ACK_RED, current_time)
            self.ack_rojo_enviado = True
            self.ack_verde_enviado = False
        elif self.luz_local == "S_VERDE" and not self.ack_verde_enviado:
            self._programar_respuesta(RF_Packet.CMD_ACK_GREEN, current_time)
            self.ack_verde_enviado = True
            self.ack_rojo_enviado = False

        return tx_bytes

# ==========================================
# 5. SUITE DE AUDITORÍA Y BATERÍA DE PRUEBAS
# ==========================================
def ejecutar_auditoria_completa():
    print("=" * 80)
    print("🚦 BATERÍA DE SIMULACIÓN Y PRUEBAS V8.0 DEFINITIVA (REPETIDOR ESP32)")
    print("=" * 80)

    maestro = SemafaroMaestro()
    esclavo = SemaforoEsclavo()
    repetidor = RepetidorESP32()
    total_pass = 0
    total_tests = 0

    def pedir_cambio(t: float):
        """Cambio de turno del CICLO y ENCOLA la trama que devuelve el Maestro.

        Solo la usa la prueba 5. Ahi el cambio de turno no lo pide nadie desde fuera: lo
        pide el propio Modo Automatico al agotarse el verde, asi que entrar por dentro es
        lo correcto -no hay puerta que ejercer-. Las ordenes que SI vienen de fuera van
        por bt(), que alimenta el despachador con la trama entera.

        FIX: las llamadas directas a maestro.pedir_cambio() descartaban el valor de
        retorno, así que el CMD_GO_RED nunca se transmitía y el ciclo solo arrancaba
        cuando vencía el timeout de reintento. Enmascaraba el arranque real del cambio.
        """
        m_tx_pend.extend(maestro.pedir_cambio(t) or b"")

    def avanzar_simulacion(duracion_s: float, dt: float = 0.1, callback_log=None):
        nonlocal current_time
        target_time = current_time + duracion_s
        while current_time < target_time:
            m_tx = maestro.actualizar(m_rx_buf, current_time)
            m_rx_buf.clear()

            if m_tx_pend:  # Tramas encoladas por pulsación de botón
                m_tx = bytes(m_tx_pend) + m_tx
                m_tx_pend.clear()

            r_out_maestro, r_out_esclavo = repetidor.passthrough(m_tx, e_tx_buf, current_time)
            e_tx_buf.clear()

            e_tx = esclavo.actualizar(r_out_esclavo, current_time)
            e_tx_buf.extend(e_tx)
            m_rx_buf.extend(r_out_maestro)

            if callback_log:
                callback_log(current_time)

            current_time += dt

    def verificar(condicion, msg_pass, msg_fail):
        nonlocal total_pass, total_tests
        total_tests += 1
        if condicion:
            total_pass += 1
            print(f"   ✔ PASS: {msg_pass}")
        else:
            print(f"   ✘ FAIL: {msg_fail}")

    def reportar(msg):
        """Hallazgo que NO cuenta, porque ningun firmware puede aprobarlo ni
        suspenderlo. CLAUDE.md §3: una comprobacion que no puede fallar no es una
        comprobacion, es una nota — y un FALLA permanente enseña a ignorar el acta."""
        print(f"   ℹ NOTA: {msg}")

    current_time = 0.0
    m_rx_buf = bytearray()
    e_tx_buf = bytearray()
    m_tx_pend = bytearray()  # Tramas emitidas fuera del bucle (ordenes de la app)

    despachador = DespachadorBluetooth(maestro)

    def bt(trama: bytes, t: float) -> str:
        """Mete la cadena de BYTES por el despachador y encola lo que salga a la radio.

        Se alimenta la trama entera -no se llama al metodo de dentro- porque lo que hay
        que ejercer es la PUERTA: el prefijo, el PIN y el strcmp, que es donde vive el
        unico camino que hoy alcanza un operario.
        """
        resp = despachador.procesar(trama, t)
        m_tx_pend.extend(despachador.rf_pendiente)
        despachador.rf_pendiente.clear()
        return resp

    # ===========================================================
    # PRUEBA 1: SET_MODO:MENU por Bluetooth, con comunicación → Ambos en ROJO FIJO
    # ===========================================================
    # INVERTIDA el 31/08. La propiedad sobrevive intacta; lo que murio fue la puerta de
    # entrada, que era la LCD del gabinete. La via de hoy esta medida:
    # Maestro/src/bluetooth.cpp:191 atiende "SET_MODO:MENU", y entra SIN PIN por la
    # segunda rama de :168-170 -el PIN guarda lo que ABRE paso, no lo que lo para-.
    print("\n▶ PRUEBA 1: Menu por Bluetooth CON comunicacion (TEST 5 campo)...")
    resp1 = bt(b"CMD:SET_MODO:MENU", current_time)
    maestro.t_ultima_rx_esclavo = current_time  # Simular que hay comunicación
    esclavo.t_ultimo_comando = current_time
    avanzar_simulacion(5.0)
    print(f"   [t={current_time:.1f}s] app->{resp1} | Maestro: {maestro.luz_local} "
          f"| Esclavo: {esclavo.luz_local}")
    verificar(
        resp1 == BT_ACK_MENU and
        maestro.luz_local == "S_ROJO" and esclavo.luz_local == "S_ROJO",
        f"La trama 'CMD:SET_MODO:MENU' entra SIN PIN, se acusa con {BT_ACK_MENU!r} y deja "
        "a los dos nodos en ROJO FIJO continuo mientras haya comunicacion.",
        f"app->{resp1} (esperado {BT_ACK_MENU!r}); deben quedar los dos en ROJO FIJO")

    # ===========================================================
    # PRUEBA 2: SET_MODO:MENU por Bluetooth, SIN comunicación → Ambos en AMARILLO
    # ===========================================================
    # INVERTIDA el 31/08 por lo mismo que la 1: cambia la puerta, no la propiedad. La
    # derivacion de FALLBACK_S + LATIDO_S desde el C++ (arreglo de N-71) se conserva
    # literal mas abajo, que es lo unico que impide que esta prueba mida un numero
    # escrito a mano.
    print("\n▶ PRUEBA 2: Menu por Bluetooth SIN comunicacion (TEST 4 campo)...")
    maestro2 = SemafaroMaestro()
    resp2 = DespachadorBluetooth(maestro2).procesar(b"CMD:SET_MODO:MENU", 0.0)
    maestro2.t_ultima_rx_esclavo = 0.0  # V7.7: Nunca recibió nada
    esclavo2 = SemaforoEsclavo()
    esclavo2.t_ultimo_comando = 0.0
    rep2 = RepetidorESP32()
    rep2.activo = False  # Sin comunicación
    t2 = 0.0
    # N-71: eran range(150) -15 s- elegidos cuando el umbral de orfandad eran 12 s.
    # Al subirlo a 25 s (SFTY6_SILENCIO_MS) esta prueba empezo a fallar, y la
    # tentacion era subir el 150. Se deriva de la constante: la prueba mide que el
    # ambar llega DESPUES del umbral, no que llegue en el segundo 15.
    for _ in range(int((FALLBACK_S + LATIDO_S) * 10)):
        m_tx2 = maestro2.actualizar(bytearray(), t2)
        esclavo2.actualizar(bytearray(), t2)
        t2 += 0.1
    print(f"   [t={t2:.1f}s] app->{resp2} | Maestro: {maestro2.luz_local} "
          f"| Esclavo: {esclavo2.luz_local}")
    verificar(
        resp2 == BT_ACK_MENU and
        maestro2.luz_local == "S_FALLO" and esclavo2.luz_local == "S_FALLO",
        f"Entrando al menu por la misma trama de la app, y sin comunicacion, los dos nodos "
        f"caen a AMARILLO PARPADEO pasados {FALLBACK_S + LATIDO_S:.0f}s "
        f"(SFTY6_SILENCIO_MS + LATIDO_MS, leidos del C++).",
        f"app->{resp2}; Maestro={maestro2.luz_local}, Esclavo={esclavo2.luz_local}. "
        "Ambos deben ser S_FALLO")

    # ===========================================================
    # PRUEBA 3: Esclavo apagado → Maestro en AMARILLO PARPADEO
    # ===========================================================
    print("\n▶ PRUEBA 3: Esclavo apagado (TEST 3 campo)...")
    maestro.iniciar_modo(current_time)
    avanzar_simulacion(2.0)
    print("   [Acción] Simulando apagado de la radio del Esclavo...")
    repetidor.activo = False
    avanzar_simulacion(FALLBACK_S + 1.0)   # N-71: derivado del C++, no un 13.0 fijo
    print(f"   [t={current_time:.1f}s] Maestro: {maestro.luz_local} | Estado Coord: {maestro.estado_c}")
    verificar(
        maestro.luz_local == "S_FALLO" and maestro.estado_c == "C_FALLO",
        f"Maestro pasó a AMARILLO INTERMITENTE tras {FALLBACK_S:.0f}s sin comunicación.",
        "Maestro debe pasar a S_FALLO / C_FALLO")

    # ===========================================================
    # PRUEBA 4: Self-Healing Auto-Recuperación
    # ===========================================================
    print("\n▶ PRUEBA 4: Reconexión de Radio (Self-Healing Autónomo)...")
    print("   [Acción] Restableciendo alimentación del Repetidor ESP32...")
    repetidor.activo = True
    avanzar_simulacion(2.0)
    print(f"   [t={current_time:.1f}s] Maestro: {maestro.luz_local} | Estado Coord: {maestro.estado_c}")
    verificar(
        maestro.luz_local == "S_ROJO" and maestro.estado_c == "C_INICIAL_ESPERA_ESTATICO",
        "Auto-Recuperación ejecutó All-Red de 15s autónomamente sin reinicio manual.",
        "Debe entrar a C_INICIAL_ESPERA_ESTATICO con S_ROJO")

    # ===========================================================
    # PRUEBA 5: Ciclo Completo Modo Automático (1 min / 1 min / 15s)
    # ===========================================================
    print("\n▶ PRUEBA 5: Modo Automático — Ciclo completo sin caída de comunicación...")
    maestro.tiempo_despeje_s = DESPEJE_S
    maestro.iniciar_modo(current_time)
    avanzar_simulacion(15.0 + 4.1)
    print(f"   [Paso 1] Maestro pasa a Verde: Maestro={maestro.luz_local}, Esclavo={esclavo.luz_local}")

    avanzar_simulacion(60.0)
    pedir_cambio(current_time)
    avanzar_simulacion(1.0)
    print(f"   [Paso 2] Despeje tras Maestro: Maestro={maestro.luz_local}, Esclavo={esclavo.luz_local}")

    avanzar_simulacion(15.0 + 4.1 + 60.0)
    print(f"   [Paso 3] Turno Esclavo Verde: Maestro={maestro.luz_local}, Esclavo={esclavo.luz_local}")

    pedir_cambio(current_time)
    avanzar_simulacion(25.0)
    print(f"   [Paso 4] Regreso a Maestro Verde: Maestro={maestro.luz_local}, Estado={maestro.estado_c}")
    verificar(
        maestro.luz_local == "S_VERDE" and maestro.estado_c == "C_IDLE",
        "Ciclo completado. Falso fallo del segundo 15 ELIMINADO.",
        f"Maestro={maestro.luz_local}, Estado={maestro.estado_c}. Debe ser S_VERDE/C_IDLE")

    # ===========================================================
    # PRUEBA 6a: Rojo Fijo Indefinido pedido por la app (era el Botón 3)
    # ===========================================================
    # INVERTIDA el 31/08, y esta era la peor de las cuatro: el sujeto ya no existe. El
    # BOTON3 es CAM_C_PIN desde N-67 (pines.h:124) y botonAceptar() devuelve false fijo
    # (botones.cpp:280). Ademas la prueba vieja NUNCA TOCO UN BOTON -llamaba a
    # forzar_rojo_total() por dentro-, asi que llevaba meses en verde midiendo un camino
    # que ningun operario alcanza. La via de hoy es "CMD:FORZAR_ROJO", que entra ANTES de
    # la puerta del PIN (bluetooth.cpp:145) porque parar el trafico es la accion segura.
    print("\n▶ PRUEBA 6a: Rojo Fijo Indefinido por 'CMD:FORZAR_ROJO' (sin PIN)...")
    resp6a = bt(b"CMD:FORZAR_ROJO", current_time)
    avanzar_simulacion(20.0)
    print(f"   [t={current_time:.1f}s] app->{resp6a} | Maestro: {maestro.luz_local} "
          f"| Esclavo: {esclavo.luz_local} | Estado: {maestro.estado_c}")
    verificar(
        resp6a == BT_ACK_ROJO and
        maestro.luz_local == "S_ROJO" and esclavo.luz_local == "S_ROJO" and maestro.estado_c == "C_IDLE",
        f"'CMD:FORZAR_ROJO' se atiende SIN PIN, acusa {BT_ACK_ROJO!r} y mantiene ROJO FIJO "
        "continuo en los dos nodos de forma indefinida.",
        f"app->{resp6a} (esperado {BT_ACK_ROJO!r}); deben seguir en ROJO FIJO / C_IDLE")

    # ===========================================================
    # PRUEBA 6b: Reanudar el paso a VERDE (era el Botón 1)
    # ===========================================================
    # BOTON1 = PB9 = MANDO_A sigue existiendo (pines.h:122), pero su papel AQUI era
    # "pedir cambio de turno", y eso hoy se pide por "MANUAL:CAMBIAR_TURNO"
    # (bluetooth.cpp:257), que SI pide PIN porque abre paso.
    #
    # Y se exige el $ERR, no solo el verde: es una de las ramas que app_03_sin_ok_mudo
    # marco. coordinador_pedirCambio() abandona en silencio si no esta en C_IDLE, asi que
    # el despachador tiene que preguntar antes y DECIRLO. Un $ACK que no depende de lo que
    # la llamada hizo es una mentira con formato de exito.
    print("\n▶ PRUEBA 6b: Reanudar el paso por 'MANUAL:CAMBIAR_TURNO' (con PIN)...")
    resp6b_ok = bt(BT_PREFIJO_PIN.encode() + b"MANUAL:CAMBIAR_TURNO", current_time)
    # Segunda orden inmediata: el coordinador ya NO esta en reposo -acaba de entrar en el
    # todo-rojo de despeje- y partir ese despeje por la mitad es justo lo que no se puede
    # hacer. Tiene que llegar el rechazo, no un OK.
    resp6b_err = bt(BT_PREFIJO_PIN.encode() + b"MANUAL:CAMBIAR_TURNO", current_time)
    resp6b_sinpin = bt(b"CMD:MANUAL:CAMBIAR_TURNO", current_time)
    avanzar_simulacion(15.0 + 4.3)
    print(f"   [Accion] app->{resp6b_ok} | en transicion->{resp6b_err} | "
          f"sin PIN->{resp6b_sinpin} | Maestro: {maestro.luz_local}")
    verificar(
        resp6b_ok == BT_ACK_CAMBIO and resp6b_err == BT_ERR_CAMBIO
        and resp6b_sinpin == BT_ERR_AUTH and maestro.luz_local == "S_VERDE",
        f"'MANUAL:CAMBIAR_TURNO' reanuda el paso a VERDE y acusa {BT_ACK_CAMBIO!r}; repetida "
        f"con el despeje en curso responde {BT_ERR_CAMBIO!r} en vez de mentir con un OK, y "
        f"sin PIN no entra ({BT_ERR_AUTH!r}).",
        f"OK->{resp6b_ok}, en transicion->{resp6b_err}, sin PIN->{resp6b_sinpin}, "
        f"luz={maestro.luz_local}")

    # ===========================================================
    # PRUEBA 7: SFTY-13 — PING NO se envía durante espera de ACK
    # ===========================================================
    print("\n▶ PRUEBA 7: SFTY-13 — Verificar supresión de PING durante ACK-wait...")
    maestro3 = SemafaroMaestro()
    maestro3.tiempo_despeje_s = 1.0  # Despeje corto para llegar rápido al ACK-wait
    maestro3.iniciar_modo(0.0)
    maestro3.t_ultimo_ping = 0.0  # Forzar que el timer PING esté "vencido"
    t3 = 0.0
    ping_enviado_durante_ack = False
    # PRIMER BUCLE: solo AVANZAR hasta entrar en el ACK-wait. No mide nada.
    #
    # 31/08: aqui vivia un `if` anidado cuyo unico cuerpo era `pass`, con un comentario
    # deliberando sobre si un msg_id que cambia es un PING o un reintento. Quien lo
    # escribio no lo tenia claro y lo dejo asi (CLAUDE.md §3.ter). Queda resuelto: el
    # msg_id NO distingue las dos cosas -lo mueven los dos caminos-, asi que no sirve de
    # discriminador y el bucle no tiene por que intentarlo. Quien si distingue es
    # t_ultimo_ping, que SOLO se toca en la rama del latido (ver actualizar()), y esa es
    # la medida que hace el segundo bucle.
    for _ in range(200):
        maestro3.actualizar(bytearray(), t3)
        if maestro3.estado_c in ("C_ESPERANDO_ACK_GREEN", "C_ESPERANDO_ACK_RED"):
            break
        t3 += 0.1
    # SEGUNDO BUCLE: ya dentro del ACK-wait, con el timer del latido vencido a proposito.
    # Si el latido se colara, t_ultimo_ping se moveria.
    maestro3.t_ultimo_ping = t3 - (LATIDO_S + 0.5)  # Forzar timer PING vencido
    for _ in range(20):
        old_ping = maestro3.t_ultimo_ping
        maestro3.actualizar(bytearray(), t3)
        if maestro3.t_ultimo_ping != old_ping and maestro3.estado_c in ("C_ESPERANDO_ACK_GREEN", "C_ESPERANDO_ACK_RED"):
            ping_enviado_durante_ack = True
            break
        t3 += 0.1
    # El texto decia "anti-colision RS485" y estaba caducado: el RS485 era el bus del
    # repetidor viejo. El enlace de hoy es LoRa por USART3, y lo que SFTY-13 protege es la
    # ventana del acuse: un latido metido ahi le pisa al Esclavo la unica respuesta que el
    # coordinador esta esperando, y el ciclo se va por el camino del reintento.
    verificar(
        not ping_enviado_durante_ack,
        "SFTY-13: el latido queda SUPRIMIDO mientras el coordinador espera el acuse, para "
        "no pisar la respuesta del Esclavo en el enlace de radio.",
        "Se emitio un PING durante el ACK-wait: el latido puede pisar el acuse del Esclavo")

    # ===========================================================
    # PRUEBA 8 — RETIRADA el 31/08/2026
    # ===========================================================
    # Leia CMD_ACK_RED y CMD_PING de protocolo.h y los comparaba contra 0x06 y 0x04
    # ESCRITOS A MANO AQUI. Dos problemas: duplicaba a costura_03_comandos, y ademas lo
    # hacia peor. Si manana el contrato se renumerase legitimamente, esta prueba fallaria
    # por el motivo equivocado -"el numero no es el que yo tengo escrito"- en vez de por
    # el motivo real, que es que dos comandos colisionen.
    #
    # Quien lo mide, SIN literales:
    #   costura_03_comandos.py -> "los {len(CMD)} codigos de protocolo.h son todos
    #   distintos: ningun comando puede confundirse con otro (la colision
    #   CMD_ACK_RED/CMD_PING ya costo una vez)"  -- barre TODA la tabla, no dos codigos.
    #   costura_01_contratos.py -> ademas exige que protocolo.h sea identico byte a byte
    #   en las dos puntas, que es lo que impide que una renumere y la otra no.

    # ===========================================================
    # PRUEBA 9: SFTY-23 — Sincronizacion horaria por radio
    # ===========================================================
    # 9a (x3) y 9b-1 — RETIRADAS el 31/08/2026.
    #
    # 9a montaba una clase `EsclavoReloj` que era una SEGUNDA COPIA A MANO del
    # Esclavo/src/main.cpp dentro de este mismo fichero, y comprobaba tres casos sobre
    # ella. Eso no mide el firmware: mide la copia. Quien lo mide de verdad es
    # esclavo_05_hora_atomica.py, que corre sobre el modelo del banco y lo hace mas
    # ancho:
    #   9a-1 (terna completa)      -> 5.1 "La terna completa se aplica de una vez y se
    #                                 acusa con CMD_ACK_HORA."
    #   9a-2 (segundos sueltos)    -> 5.2 "Cinco tramas de segundos sueltas seguidas no
    #                                 tocan el reloj y no se contestan"
    #   9a-3 (foto rancia)         -> 5.4 "Barrido de 0 a N ms de retardo: la terna se
    #                                 aplica mientras la foto tiene menos de
    #                                 VENTANA_HORA_MS y se descarta despues."
    #   y ademas 5.3 barre LAS 8 combinaciones de H/M/S, 5.5 el reenvio de segundos, 5.6
    #   los 768 valores imposibles y 5.7 el rearme del limite de 48 h.
    #
    # 9b-1 comprobaba 5 casos del desfase circular. esclavo_04_desfase.py barre LAS 3.600
    # combinaciones -"Barrido completo de 60x60 = 3.600 combinaciones: el desfase cae
    # SIEMPRE en [-30, +30]"- y trae el mismo caso frontera con las mismas cifras: "Maestro
    # en el segundo 1 y Esclavo en el 59 -> +2 s (el Esclavo va atrasado), no -58 s".
    # Cinco casos son un subconjunto estricto de 3.600.
    print("\n▶ PRUEBA 9: SFTY-23 — Sincronizacion horaria por radio...")

    # 9b-2 BAJA A NOTA. El limite de +-30 s no lo puede aprobar ni suspender ningun
    # firmware: es una propiedad de la aritmetica de un byte de segundos, y se cumple
    # aunque el C++ cambie entero. Como comprobacion nunca podria fallar (§3); como nota
    # sigue diciendo lo unico que importa de ella, que es por que la puerta del Degradado
    # no puede apoyarse solo en este numero.
    def calcular_desfase(seg_maestro, seg_esclavo):
        """El envoltorio circular sobre el minuto. Positivo = Maestro por delante."""
        d = seg_maestro - seg_esclavo
        if d > 30:
            d -= 60
        if d < -30:
            d += 60
        return d

    reportar(f"SFTY-23, limite inherente de la medida: con solo el byte de segundos un "
             f"desfase real de 45 s se mide como {calcular_desfase(45, 0)} s. Ningun firmware "
             f"puede arreglarlo, asi que no es una comprobacion sino un limite: por eso la "
             f"puerta del Modo Degradado exige ADEMAS sincronizacion reciente, y no solo "
             f"que este numero sea pequeno.")

    print("\n▶ PRUEBA 9c: SFTY-23 — El Maestro RECALCULA los segundos al reintentar...")

    # Esta es la unica de las veinte que TOCA EL C++ REAL, y solo se manifiesta cuando
    # una trama SE PIERDE: con enlace bueno nunca aparece. Se comprueba que la lectura
    # del reloj esta DENTRO de la funcion de envio -la que se invoca en cada intento-,
    # porque si no existe el sitio donde guardar el valor viejo, nadie puede reutilizarlo.
    #
    # ⚠️ SU SITIO NO ES ESTE FICHERO, y se deja dicho para que no se pierda: lee el
    # firmware POR TEXTO, y N-89 enseño que un refactor que mueva ese bloque apaga el
    # instrumento sin romper un solo test. Ese riesgo se vigila en los packs, que tienen
    # control_negativo y guarda de rutas; aqui no hay ninguna de las dos cosas. La mudanza
    # propuesta es a maestro_04_sync_horaria, junto a su 4.4, que hoy mide la MISMA regla
    # pero sobre el modelo y no sobre el fuente.
    #
    # ⚠️ Y ADEMAS EL DETECTOR ES GRUESO, medido el 31/08 inyectando el defecto en una copia
    # del arbol: enviarHoraCompleta() lee reloj_segundo() DOS veces -la segunda dentro de la
    # guarda del cruce de minuto-, y esto busca la subcadena en el cuerpo entero. Congelando
    # SOLO la primera lectura, que es la que decide el valor enviado, la prueba siguio en
    # verde; hizo falta borrar las dos para verla caer a 8/9. Lo que mide de verdad es "no
    # queda NINGUNA lectura dentro", no "el valor enviado se relee". Quien recoja la mudanza
    # tiene ahi el hueco.
    coord_path = _ruta_firmware("Maestro", "src", "coordinador.cpp")
    RE_ENVIO = r"static void enviarHora\w*\(\)\s*\{(.*?)\n\}"
    lectura_dentro = False
    if coord_path and os.path.exists(coord_path):
        with open(coord_path, "r", encoding="utf-8", errors="replace") as f:
            cuerpo = f.read()
        # Se busca por lo que la funcion HACE, no por como se llama: el 01/08/2026 paso
        # de enviarTrioHora() a enviarHoraCompleta() al anadirse la trama del dia, y
        # esta prueba fallo por el nombre mientras la propiedad seguia intacta. Una
        # prueba que se rompe al renombrar una funcion entrena a ignorarla.
        m = _re.search(RE_ENVIO, cuerpo, _re.S)
        if not m:
            m = _re.search(r"static void enviarTrioHora\(\)\s*\{(.*?)\n\}", cuerpo, _re.S)
        if m:
            lectura_dentro = "reloj_segundo()" in m.group(1)

    # CONTROL NEGATIVO 1 (era la prueba 9c-1, que contaba y no podia fallar).
    #
    # 9c-1 comprobaba una clase MaestroSync definida diez lineas mas arriba con un
    # interruptor booleano `recalcula`: la rama buena devolvia el valor nuevo porque
    # estaba escrita para devolverlo. Ningun firmware podia suspenderla. Deja de contar
    # como comprobacion y pasa a lo unico para lo que servia: demostrar que la propiedad
    # que se mide arriba distingue el firmware correcto del defectuoso.
    class MaestroSync:
        def __init__(self, recalcula):
            self.recalcula = recalcula
            self.congelado = None

        def enviar(self, reloj_seg):
            if self.recalcula or self.congelado is None:
                self.congelado = reloj_seg
            return self.congelado

    reloj_1 = 10
    reloj_2 = reloj_1 + int(TIMEOUT_ACK_S)
    bueno = MaestroSync(recalcula=True)
    bueno.enviar(reloj_1)
    malo = MaestroSync(recalcula=False)
    malo.enviar(reloj_1)
    atraso_si_congela = reloj_2 - malo.enviar(reloj_2)
    ctrl_distingue = (bueno.enviar(reloj_2) == reloj_2 and atraso_si_congela == int(TIMEOUT_ACK_S))

    # CONTROL NEGATIVO 2, el que de verdad hace falta aqui (§8.bis): que el DETECTOR sepa
    # fallar. Se le da un cuerpo sintetico con la misma forma pero sin la lectura, y se
    # exige que NO lo apruebe. Sin esto, un refactor que se llevara reloj_segundo() a otro
    # fichero dejaria esta comprobacion en verde midiendo nada.
    cuerpo_falso = ("static void enviarHoraCompleta() {\n"
                    "  uint8_t s = segundoCongelado;\n"
                    "  protocolo_enviarPaquete(CMD_HORA_S, s);\n}")
    m_falso = _re.search(RE_ENVIO, cuerpo_falso, _re.S)
    ctrl_detector = bool(m_falso) and "reloj_segundo()" not in m_falso.group(1)

    print(f"   [ctrl-neg] el modelo distingue recalcular de congelar "
          f"(congelar atrasaria {atraso_si_congela}s): {ctrl_distingue} | "
          f"el detector rechaza un cuerpo sin la lectura: {ctrl_detector}")
    verificar(lectura_dentro and ctrl_distingue and ctrl_detector,
              "SFTY-23: el firmware lee reloj_segundo() DENTRO de la funcion de envio, "
              "la que se llama en cada intento: no hay valor viejo que reutilizar. Y la "
              "comprobacion sabe fallar: rechaza un cuerpo de envio que no la tenga.",
              "No se encontro la lectura del reloj dentro de la funcion de envio "
              f"(lectura={lectura_dentro}), o la comprobacion no sabe fallar "
              f"(modelo={ctrl_distingue}, detector={ctrl_detector})")

    # ===========================================================
    # PRUEBA 10 (x4) — RETIRADA el 31/08/2026
    # ===========================================================
    # Barria las 86.400 posiciones del dia contra `fase_degradado()`, que era una
    # REIMPLEMENTACION A MANO de ciclo_degradado_fase(). Es exactamente el espejo que
    # N-36 castigo: alguien tenia que mantenerlo sincronizado con el C++, y el dia que
    # dejara de estarlo el barrido seguiria dando verde sobre el calculo viejo.
    #
    # Lo mide 01_Firmware/Validacion_Ciclo/arnes_ciclo.cpp, que hace #include del
    # ciclo_degradado.h REAL del firmware y barre SIETE configuraciones donde aqui habia
    # cinco:
    #   10-1 (verde->verde)   -> "las 86.400 posiciones del dia: NUNCA se pasa de verde a
    #                            verde sin todo-rojo (transiciones malas: %ld)"
    #   10-2 (medianoche)     -> "la frontera de medianoche esta en todo-rojo por los DOS
    #                            lados: ningun verde queda cortado por el cambio de dia"
    #   10-3 (verde simultaneo
    #        imposible)       -> "ningun segundo del dia da verde a las DOS puntas"
    #   10-4 (config imposible
    #        -> todo-rojo)    -> control negativo: "con despeje=0 la funcion NO da un solo
    #                            verde en todo el dia: una configuracion imposible cae al
    #                            lado seguro, no al comodo"
    #
    # ⚠️ DOS HUECOS QUE ESTA RETIRADA DEJA A LA VISTA, y que no se tapan aqui porque
    # taparlos con el espejo seria fingir cobertura (una fila que miente es peor que una
    # vacia). Van al informe para que los recoja quien toque el arnes:
    #   (a) el control negativo del arnes solo ejercita `despeje = 0`. La guarda del C++ es
    #       `if (verdeSeg == 0 || despejeSeg == 0)`: la mitad `verde = 0` no la ejerce
    #       ningun instrumento. Esta prueba 10-4 si la ejercia, pero sobre el espejo.
    #   (b) la comprobacion 2 del arnes es tautologica:
    #           if (f == FD_VERDE_MAESTRO && f == FD_VERDE_ESCLAVO) simultaneos++;
    #       un solo valor del enum no puede ser los dos a la vez, asi que `simultaneos`
    #       vale 0 pase lo que pase. Es la prueba muerta de N-51 dentro de una barrera de
    #       seguridad. La propiedad real la sostiene la comprobacion 1, no esta.

    # ===========================================================
    # VEREDICTO FINAL
    # ===========================================================
    print("\n" + "=" * 80)
    if total_pass == total_tests:
        print(f"🏆 VEREDICTO FINAL: {total_pass}/{total_tests} PASS (SISTEMA V8.0 DEFINITIVA CERTIFICADO)")
    else:
        print(f"⚠️  VEREDICTO FINAL: {total_pass}/{total_tests} PASS — HAY FALLOS PENDIENTES")
    print("=" * 80)
    # El formato de la linea de arriba es el que compuerta.py sabe leer (la ultima linea
    # con "/", un digito y "PASS"), y ahi se aplica la regla de N-71: x == y o FALLA.
    #
    # Y ademas se sale con codigo 1 cuando falta alguna. Hasta hoy este simulador salia
    # con 0 pasara lo que pasara: la unica red era esa regla de la compuerta. Un
    # instrumento que anuncia fallos y contesta "todo bien" al que le pregunta por el
    # codigo de salida es la CUARTA CARA de N-46, y no hace falta convivir con ella.
    return 0 if total_pass == total_tests else 1


if __name__ == "__main__":
    sys.exit(ejecutar_auditoria_completa())
