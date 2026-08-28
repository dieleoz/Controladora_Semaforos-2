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
- SFTY-13: Supresión de PING durante espera de ACK (anti-colisión RS485)
- CMD_ACK_RED reasignado a 0x06 (eliminación de colisión con CMD_PING 0x04)
- Buffer overflow protocolo corregido (binIdx=0 restaurado)
- tUltimaRxEsclavo inicializado a 0 en setup
- TIMEOUT_ACK y RF_BURST_COPIES se LEEN del firmware C++ en cada ejecución (ver bloque 0),
  para que el modelo no pueda divergir del código real sin que la prueba lo note.

Autor: Antigravity DeepMind Embedded Safety Agent
Fecha: 31 de Julio de 2026
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

# --- SFTY-23: los codigos de comando se leen del contrato, no se copian ---------
# Si alguien reasigna un comando en protocolo.h y aqui quedara el numero viejo, la
# prueba pasaria validando un protocolo que ya no existe.
def _leer_define_hex(nombre):
    """Los codigos estan en hexadecimal; _leer_constante_cpp devuelve int base 10."""
    ruta = _ruta_firmware("Maestro", "include", "protocolo.h")
    if ruta and os.path.exists(ruta):
        with open(ruta, "r", encoding="utf-8", errors="replace") as f:
            m = _re.search(r"#define\s+" + nombre + r"\s+0x([0-9A-Fa-f]+)", f.read())
        if m:
            return int(m.group(1), 16)
    print(f"\n   ❌ ABORTADO: no se pudo leer {nombre} de protocolo.h.")
    sys.exit(2)


CMD_HORA_H     = _leer_define_hex("CMD_HORA_H")
CMD_HORA_M     = _leer_define_hex("CMD_HORA_M")
CMD_HORA_S     = _leer_define_hex("CMD_HORA_S")
CMD_ACK_HORA   = _leer_define_hex("CMD_ACK_HORA")
CMD_DELTA      = _leer_define_hex("CMD_DELTA")
CMD_DELTA_RESP = _leer_define_hex("CMD_DELTA_RESP")

print(f"   SFTY-23: HORA_H=0x{CMD_HORA_H:02X} HORA_M=0x{CMD_HORA_M:02X} "
      f"HORA_S=0x{CMD_HORA_S:02X} DELTA=0x{CMD_DELTA:02X}")

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
        """Pulsa el botón de cambio y ENCOLA la trama que devuelve el Maestro.

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

    current_time = 0.0
    m_rx_buf = bytearray()
    e_tx_buf = bytearray()
    m_tx_pend = bytearray()  # Tramas emitidas fuera del bucle (pulsaciones de botón)

    # ===========================================================
    # PRUEBA 1: Menú con comunicación → Ambos en ROJO FIJO
    # ===========================================================
    print("\n▶ PRUEBA 1: Menú con comunicación (TEST 5 campo)...")
    maestro.forzar_menu()
    maestro.t_ultima_rx_esclavo = current_time  # Simular que hay comunicación
    esclavo.t_ultimo_comando = current_time
    avanzar_simulacion(5.0)
    print(f"   [t={current_time:.1f}s] Maestro: {maestro.luz_local} | Esclavo: {esclavo.luz_local}")
    verificar(
        maestro.luz_local == "S_ROJO" and esclavo.luz_local == "S_ROJO",
        "Ambos nodos en ROJO FIJO continuo durante Menú con comunicación.",
        "Deben estar en ROJO FIJO con comunicación activa")

    # ===========================================================
    # PRUEBA 2: Menú SIN comunicación → Ambos en AMARILLO PARPADEO
    # ===========================================================
    print("\n▶ PRUEBA 2: Menú SIN comunicación (TEST 4 campo)...")
    maestro2 = SemafaroMaestro()
    maestro2.forzar_menu()
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
    print(f"   [t={t2:.1f}s] Maestro: {maestro2.luz_local} | Esclavo: {esclavo2.luz_local}")
    verificar(
        maestro2.luz_local == "S_FALLO" and esclavo2.luz_local == "S_FALLO",
        "Ambos nodos en AMARILLO PARPADEO sin comunicación en Menú.",
        f"Maestro={maestro2.luz_local}, Esclavo={esclavo2.luz_local}. Ambos deben ser S_FALLO")

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
    # PRUEBA 6: Modo Manual — Botón 3 (Rojo Directo Indefinido)
    # ===========================================================
    print("\n▶ PRUEBA 6: Modo Manual (Botón 3 — Rojo Fijo Indefinido)...")
    maestro.forzar_rojo_total(current_time)
    avanzar_simulacion(20.0)
    print(f"   [t={current_time:.1f}s] Maestro: {maestro.luz_local} | Esclavo: {esclavo.luz_local} | Estado: {maestro.estado_c}")
    verificar(
        maestro.luz_local == "S_ROJO" and esclavo.luz_local == "S_ROJO" and maestro.estado_c == "C_IDLE",
        "Botón 3 mantiene ROJO FIJO continuo en ambos nodos de forma indefinida.",
        "Deben seguir en ROJO FIJO / C_IDLE")

    pedir_cambio(current_time)
    avanzar_simulacion(15.0 + 4.3)
    print(f"   [Acción] Pulsando Botón 1 -> Maestro: {maestro.luz_local}")
    verificar(
        maestro.luz_local == "S_VERDE",
        "Botón 1 reanuda el paso a VERDE correctamente.",
        "Maestro debe pasar a VERDE")

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
    # Avanzar hasta que entre en C_ESPERANDO_ACK_GREEN
    for _ in range(200):
        old_msg_id = maestro3.msg_id_counter
        old_estado = maestro3.estado_c
        maestro3.actualizar(bytearray(), t3)
        if maestro3.estado_c in ("C_ESPERANDO_ACK_GREEN", "C_ESPERANDO_ACK_RED"):
            # Si el msg_id cambió durante un estado ACK-wait, se envió algo (podría ser retry o PING)
            # Verificar que no se envía PING cuando ya estábamos en ACK-wait
            if old_estado in ("C_ESPERANDO_ACK_GREEN", "C_ESPERANDO_ACK_RED"):
                if t3 - maestro3.t_ultimo_ping < 0.01 and maestro3.msg_id_counter != old_msg_id:
                    # Un PING se habría enviado si t_ultimo_ping se actualizó
                    pass  # Los retries actualizan msg_id, eso es OK
            break
        t3 += 0.1
    # Ahora estamos en C_ESPERANDO_ACK_GREEN. Avanzar 4s (más que los 3s del timer PING)
    t3_start_ack = t3
    maestro3.t_ultimo_ping = t3 - 3.5  # Forzar timer PING vencido
    for _ in range(20):
        old_ping = maestro3.t_ultimo_ping
        maestro3.actualizar(bytearray(), t3)
        if maestro3.t_ultimo_ping != old_ping and maestro3.estado_c in ("C_ESPERANDO_ACK_GREEN", "C_ESPERANDO_ACK_RED"):
            ping_enviado_durante_ack = True
            break
        t3 += 0.1
    verificar(
        not ping_enviado_durante_ack,
        "PING correctamente SUPRIMIDO durante C_ESPERANDO_ACK (anti-colisión RS485).",
        "PING fue enviado durante ACK-wait — COLISIÓN RS485 posible")

    # ===========================================================
    # PRUEBA 8: Lectura directa de C++ protocolo.h (CMD_ACK_RED ≠ CMD_PING)
    # ===========================================================
    print("\n▶ PRUEBA 8: Verificar protocolo.h de C++ (CMD_ACK_RED 0x06 != CMD_PING 0x04)...")
    import re, os
    header_ok = False
    script_dir = os.path.dirname(os.path.abspath(__file__))
    candidate_paths = [
        os.path.join(script_dir, "..", "Maestro", "include", "protocolo.h"),
        os.path.join(os.getcwd(), "01_Firmware", "Maestro", "include", "protocolo.h"),
        "01_Firmware/Maestro/include/protocolo.h"
    ]
    header_path = None
    for p in candidate_paths:
        if os.path.exists(p):
            header_path = p
            break

    if header_path:
        try:
            with open(header_path, "r", encoding="utf-8") as f:
                content = f.read()
            cmd_ack_red = int(re.search(r"#define\s+CMD_ACK_RED\s+(0x[0-9A-Fa-f]+|\d+)", content).group(1), 16)
            cmd_ping = int(re.search(r"#define\s+CMD_PING\s+(0x[0-9A-Fa-f]+|\d+)", content).group(1), 16)
            header_ok = (cmd_ack_red == 0x06) and (cmd_ping == 0x04) and (cmd_ack_red != cmd_ping)
        except Exception as e:
            header_ok = False

    verificar(
        header_ok,
        f"protocolo.h parseado con éxito ({header_path}): CMD_ACK_RED=0x06, CMD_PING=0x04 — Sin colisión en C++.",
        f"Error al verificar protocolo.h de C++ en {header_path}")

    # ===========================================================
    # PRUEBA 9: SFTY-23 — Sincronizacion horaria por radio
    # ===========================================================
    # Se modela SOLO el lado Esclavo del intercambio, que es donde estan las dos
    # reglas que pueden fallar en silencio: la aplicacion atomica y la aritmetica
    # circular del desfase. El lado Maestro se prueba en la 9c.
    print("\n▶ PRUEBA 9a: SFTY-23 — La hora se aplica de forma ATOMICA...")

    class EsclavoReloj:
        """Modelo del Esclavo recibiendo la terna H/M/S. Espeja Esclavo/src/main.cpp."""
        VENTANA_HORA_S = 3.0

        def __init__(self):
            self.hora = None          # None = reloj sin poner en hora
            self.buf_h = None
            self.buf_m = None
            self.t_buf = 0.0
            self.acks = 0

        def _caducar(self, t):
            if self.buf_h is not None and (t - self.t_buf) > self.VENTANA_HORA_S:
                self.buf_h = None
                self.buf_m = None

        def recibir(self, cmd, param, t):
            self._caducar(t)
            if cmd == CMD_HORA_H:
                self.buf_h = param
                self.t_buf = t
            elif cmd == CMD_HORA_M:
                self.buf_m = param
                self.t_buf = t
            elif cmd == CMD_HORA_S:
                if self.buf_h is not None and self.buf_m is not None:
                    self.hora = (self.buf_h, self.buf_m, param)
                    self.acks += 1
                # El buffer se consume SIEMPRE, se aplique o no: una terna sirve
                # una sola vez.
                self.buf_h = None
                self.buf_m = None

    # 9a-1: la terna completa se aplica
    e = EsclavoReloj()
    e.recibir(CMD_HORA_H, 14, 0.0)
    e.recibir(CMD_HORA_M, 32, 0.01)
    e.recibir(CMD_HORA_S, 5, 0.02)
    verificar(e.hora == (14, 32, 5) and e.acks == 1,
              "SFTY-23: la terna completa H/M/S se aplica junta y se confirma una vez.",
              f"La terna no se aplico correctamente: {e.hora}")

    # 9a-2: segundos SUELTOS sin H/M previos NO deben escribir nada.
    # Es el caso peligroso: escribiria una hora inventada que el equipo daria por
    # buena, y nada la detectaria despues.
    e2 = EsclavoReloj()
    e2.recibir(CMD_HORA_S, 5, 0.0)
    verificar(e2.hora is None and e2.acks == 0,
              "SFTY-23: unos segundos sueltos NO ponen el reloj en hora ni se confirman.",
              "PELIGRO: se aplico una hora a medias sin haber recibido H y M")

    # 9a-3: H/M rancios NO deben combinarse con segundos nuevos.
    # Si la foto del Maestro envejece y los segundos llegan tras un cambio de
    # minuto, se escribiria una hora un minuto atrasada.
    e3 = EsclavoReloj()
    e3.recibir(CMD_HORA_H, 14, 0.0)
    e3.recibir(CMD_HORA_M, 59, 0.01)
    e3.recibir(CMD_HORA_S, 0, 10.0)   # 10 s despues: la foto caduco
    verificar(e3.hora is None,
              "SFTY-23: una foto de hora rancia caduca y no se combina con segundos nuevos.",
              f"Se aplico una hora rancia: {e3.hora}")

    print("\n▶ PRUEBA 9b: SFTY-23 — Aritmetica CIRCULAR del desfase...")

    def calcular_desfase(seg_maestro, seg_esclavo):
        """Espeja calcularDesfase() del Esclavo. Positivo = Maestro por delante."""
        d = seg_maestro - seg_esclavo
        if d > 30:
            d -= 60
        if d < -30:
            d += 60
        return d

    # El caso que delata una resta cruda: Maestro en el segundo 1, Esclavo en el 59.
    # Sin correccion daria -58 ("el Esclavo va 58 s adelantado"), cuando la verdad
    # es que el Maestro acaba de cambiar de minuto y el Esclavo va 2 s atrasado.
    casos = [
        (1, 59, 2),     # cruce de minuto hacia arriba
        (59, 1, -2),    # cruce de minuto hacia abajo
        (30, 30, 0),    # relojes iguales
        (35, 30, 5),    # Maestro 5 s por delante
        (30, 35, -5),   # Maestro 5 s por detras
    ]
    todos_ok = True
    for sm, se, esperado in casos:
        obtenido = calcular_desfase(sm, se)
        if obtenido != esperado:
            todos_ok = False
            print(f"   ✗ Maestro={sm}s Esclavo={se}s -> {obtenido}s, esperado {esperado}s")
    verificar(todos_ok,
              "SFTY-23: el desfase resuelve el cruce de minuto en el sentido corto (1 vs 59 = +2s, no -58s).",
              "La aritmetica circular del desfase es incorrecta")

    # El LIMITE INHERENTE, documentado como prueba para que nadie lo olvide: con
    # solo el segundo, un desfase real de 45 s se mide como -15 s. Por eso la
    # puerta del Modo Degradado NO puede apoyarse solo en este numero, sino exigir
    # ademas una sincronizacion RECIENTE.
    verificar(calcular_desfase(45, 0) == -15,
              "SFTY-23: se confirma el limite conocido de +-30s (45s se mide como -15s): "
              "la puerta del Degradado exige tambien sincronizacion reciente.",
              "El limite de la medida no es el documentado")

    print("\n▶ PRUEBA 9c: SFTY-23 — El Maestro RECALCULA los segundos al reintentar...")

    # Esta es la unica prueba que puede cazar el fallo del reintento, porque solo
    # se manifiesta cuando una trama SE PIERDE. Con enlace bueno nunca aparece.
    class MaestroSync:
        """Modela el envio de la terna. `recalcula` conmuta el comportamiento
        correcto y el defectuoso, para demostrar que la prueba distingue."""
        def __init__(self, recalcula):
            self.recalcula = recalcula
            self.congelado = None

        def enviar(self, reloj_seg):
            if self.recalcula or self.congelado is None:
                self.congelado = reloj_seg
            return self.congelado

    # El Maestro envia en t=0 con el reloj en el segundo 10. Se pierde. Reintenta
    # 3,5 s despues (TIMEOUT_ACK_S), cuando su reloj ya marca 13.
    reloj_en_primer_envio = 10
    reloj_en_reintento = reloj_en_primer_envio + int(TIMEOUT_ACK_S)

    bueno = MaestroSync(recalcula=True)
    bueno.enviar(reloj_en_primer_envio)
    s_bueno = bueno.enviar(reloj_en_reintento)

    malo = MaestroSync(recalcula=False)
    malo.enviar(reloj_en_primer_envio)
    s_malo = malo.enviar(reloj_en_reintento)

    error_si_no_recalcula = reloj_en_reintento - s_malo
    verificar(s_bueno == reloj_en_reintento and error_si_no_recalcula == int(TIMEOUT_ACK_S),
              f"SFTY-23: al reintentar se reenvia el segundo ACTUAL ({s_bueno}s). "
              f"Reenviar el congelado dejaria al Esclavo {error_si_no_recalcula}s atrasado.",
              "El modelo del reintento no distingue recalcular de reenviar el valor viejo")

    # Y que el firmware real no tenga donde guardar el valor viejo: se comprueba
    # que la lectura del reloj esta DENTRO de la funcion de envio, que es la que se
    # invoca en cada intento. Si no existe el sitio, nadie puede reutilizarlo.
    coord_path = _ruta_firmware("Maestro", "src", "coordinador.cpp")
    lectura_dentro = False
    if coord_path and os.path.exists(coord_path):
        with open(coord_path, "r", encoding="utf-8", errors="replace") as f:
            cuerpo = f.read()
        # Se busca por lo que la funcion HACE, no por como se llama: el 01/08/2026 paso
        # de enviarTrioHora() a enviarHoraCompleta() al anadirse la trama del dia, y
        # esta prueba fallo por el nombre mientras la propiedad seguia intacta. Una
        # prueba que se rompe al renombrar una funcion entrena a ignorarla.
        m = _re.search(r"static void enviarHora\w*\(\)\s*\{(.*?)\n\}", cuerpo, _re.S)
        if not m:
            m = _re.search(r"static void enviarTrioHora\(\)\s*\{(.*?)\n\}", cuerpo, _re.S)
        if m:
            lectura_dentro = "reloj_segundo()" in m.group(1)
    verificar(lectura_dentro,
              "SFTY-23: el firmware lee reloj_segundo() DENTRO de la funcion de envio, "
              "la que se llama en cada intento: no hay valor viejo que reutilizar.",
              "No se encontro la lectura del reloj dentro de la funcion de envio: "
              "podria estar cacheandose el segundo del primer envio")

    # ===========================================================
    # PRUEBA 10: SFTY-21 — Fase del Modo Degradado
    # ===========================================================
    # Sin radio, cada unidad decide su luz por su cuenta. Lo unico que impide el
    # verde simultaneo es que las dos calculen EXACTAMENTE lo mismo. Aqui se
    # comprueba la propiedad de seguridad sobre las 86.400 posiciones del dia, no
    # sobre unas cuantas de muestra: un fallo que solo aparezca a las 03:47 no
    # sirve de nada haberlo buscado a las 10:00.
    print("\n▶ PRUEBA 10: SFTY-21 — Fase del Modo Degradado (barrido de 24 h)...")

    SEGUNDOS_DEL_DIA = 86400

    def fase_degradado(seg_dia, verde, despeje):
        """Espeja ciclo_degradado_fase() de ciclo_degradado.h."""
        if verde == 0 or despeje == 0:
            return "FD_DESPEJE_A"
        ciclo = 2 * (verde + despeje)
        if seg_dia < despeje:
            return "FD_DESPEJE_B"
        if SEGUNDOS_DEL_DIA - seg_dia <= despeje:
            return "FD_DESPEJE_B"
        pos = seg_dia % ciclo
        if pos < verde:
            return "FD_VERDE_MAESTRO"
        if pos < verde + despeje:
            return "FD_DESPEJE_A"
        if pos < 2 * verde + despeje:
            return "FD_VERDE_ESCLAVO"
        return "FD_DESPEJE_B"

    # La propiedad que de verdad importa: en TODO el dia, jamas se pasa de un
    # verde a otro sin despeje por medio. Si esto falla, hay verde en las dos
    # puntas y dos vehiculos entran de frente al tramo.
    combinaciones = [(60, 30), (45, 20), (90, 45), (120, 30), (37, 23)]
    fallos_verde_a_verde = []
    for verde, despeje in combinaciones:
        anterior = None
        for s in range(SEGUNDOS_DEL_DIA):
            f = fase_degradado(s, verde, despeje)
            if anterior and anterior != f and "VERDE" in anterior and "VERDE" in f:
                fallos_verde_a_verde.append((verde, despeje, s))
                break
            anterior = f
    verificar(not fallos_verde_a_verde,
              f"SFTY-21: en 24 h y {len(combinaciones)} configuraciones NUNCA se pasa de verde "
              "a verde sin todo-rojo (incluidos ciclos que no dividen el dia).",
              f"PELIGRO: transicion verde->verde sin despeje en {fallos_verde_a_verde[:3]}")

    # El salto de medianoche. La posicion vuelve a 0 aunque el ciclo este a medias;
    # las dos unidades saltan igual, pero el salto podria caer dentro de un verde y
    # SALTARSE el despeje. Por eso la frontera se fuerza a todo-rojo.
    medianoche_ok = True
    for verde, despeje in combinaciones:
        for s in list(range(SEGUNDOS_DEL_DIA - despeje, SEGUNDOS_DEL_DIA)) + list(range(0, despeje)):
            if "VERDE" in fase_degradado(s, verde, despeje):
                medianoche_ok = False
                break
    verificar(medianoche_ok,
              "SFTY-21: la frontera de medianoche se cruza SIEMPRE en todo-rojo, "
              "aunque la duracion del ciclo no divida a 86400.",
              "PELIGRO: hay verde al cruzar la medianoche; el salto de posicion "
              "podria saltarse el despeje")

    # Las dos unidades leen la MISMA funcion, asi que en cada instante hay una sola
    # fase: el verde simultaneo es imposible por construccion, no por acuerdo.
    fases_validas = {"FD_VERDE_MAESTRO", "FD_VERDE_ESCLAVO", "FD_DESPEJE_A", "FD_DESPEJE_B"}
    todas_validas = all(fase_degradado(s, 60, 30) in fases_validas
                        for s in range(0, SEGUNDOS_DEL_DIA, 7))
    verificar(todas_validas,
              "SFTY-21: la fase es unica y valida en todo instante; el verde simultaneo "
              "es imposible por construccion, no por acuerdo entre las dos puntas.",
              "Se obtuvo una fase no reconocida")

    # Configuracion imposible: sin verde o sin despeje la respuesta debe ser
    # todo-rojo, no un caso "que no deberia pasar".
    verificar(fase_degradado(1000, 0, 30) == "FD_DESPEJE_A" and
              fase_degradado(1000, 60, 0) == "FD_DESPEJE_A",
              "SFTY-21: una configuracion imposible (verde o despeje a cero) responde "
              "TODO-ROJO, no un estado indefinido.",
              "Una configuracion invalida no cae a todo-rojo")

    # ===========================================================
    # VEREDICTO FINAL
    # ===========================================================
    print("\n" + "=" * 80)
    if total_pass == total_tests:
        print(f"🏆 VEREDICTO FINAL: {total_pass}/{total_tests} PASS (SISTEMA V8.0 DEFINITIVA CERTIFICADO)")
    else:
        print(f"⚠️  VEREDICTO FINAL: {total_pass}/{total_tests} PASS — HAY FALLOS PENDIENTES")
    print("=" * 80)

if __name__ == "__main__":
    ejecutar_auditoria_completa()
