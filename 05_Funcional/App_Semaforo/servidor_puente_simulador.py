#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
servidor_puente_simulador.py
Servidor HTTP + Puente de Telemetría NMEA en Tiempo Real para IOT-VIAL V9.0.

Conecta directamente el Frontend Web de la App con el Simulador del Controlador STM32 (FirmwareBluetoothSimulator).
- Sirve los archivos estáticos de la App en http://localhost:3000/
- Endpoint POST /api/cmd: Recibe comandos de la App y los ejecuta en el simulador STM32.
- Endpoint GET /api/telemetria: Devuelve la trama $STATUS NMEA en vivo con checksum XOR.
- Endpoint GET /api/status_json: Devuelve el estado parseado del controlador.
"""

import http.server
import socketserver
import json
import urllib.parse
import os
import sys
import threading
import time

# Importar o definir el simulador del controlador
class FirmwareSTM32Simulator:
    def __init__(self):
        self.node = "MAESTRO"
        self.modo = "AUTO"
        self.estado = "V1_R2" # V1_R2, Y1_R2, R1_R2, R1_V2, R1_Y2, AMBAR_FAIL
        self.countdown = 38
        self.countdown_max = 45
        self.tiempo_verde_min = 2
        self.tiempo_rojo_min = 2
        self.tiempo_despeje_seg = 15
        self.rf_calidad = 98
        self.rtt_ms = 76
        self.bateria_v = 12.6
        self.serie = "M-2026-A1B2"
        self.ultimo_evento = "Simulador STM32 iniciado en Modo AUTO."
        self.lock = threading.Lock()

    def calcular_checksum(self, payload):
        crc = 0
        for char in payload:
            crc ^= ord(char)
        return f"{crc:02X}"

    def tick(self):
        with self.lock:
            if self.modo == "AUTO":
                self.countdown -= 1
                if self.countdown <= 0:
                    if self.estado == "V1_R2":
                        self.estado = "Y1_R2"
                        self.countdown = 4
                        self.countdown_max = 4
                    elif self.estado == "Y1_R2":
                        self.estado = "R1_R2"
                        self.countdown = self.tiempo_despeje_seg
                        self.countdown_max = self.tiempo_despeje_seg
                    elif self.estado == "R1_R2":
                        self.estado = "R1_V2"
                        self.countdown = self.tiempo_rojo_min * 60
                        self.countdown_max = self.tiempo_rojo_min * 60
                    elif self.estado == "R1_V2":
                        self.estado = "R1_Y2"
                        self.countdown = 4
                        self.countdown_max = 4
                    elif self.estado == "R1_Y2":
                        self.estado = "V1_R2"
                        self.countdown = self.tiempo_verde_min * 60
                        self.countdown_max = self.tiempo_verde_min * 60

    def procesar_comando(self, raw_cmd):
        with self.lock:
            raw_cmd = raw_cmd.strip()
            print(f"  [SIMULADOR RX]: {raw_cmd}")

            # Soporte comando de emergencia sin PIN
            if raw_cmd == "CMD:FORZAR_ROJO":
                self.modo = "ROJO_TOTAL"
                self.estado = "R1_R2"
                self.countdown = 0
                self.ultimo_evento = "Rojo Total de Emergencia accionado."
                payload = "ACK,CMD:FORZAR_ROJO,RESULT:OK"
                return f"${payload}*{self.calcular_checksum(payload)}\r\n"

            # Validación obligatoria de PIN 1234
            if not raw_cmd.startswith("CMD:PIN:1234:"):
                payload = "ERR,CMD:AUTH_FAILED,DESC:PIN_INVALIDO"
                return f"${payload}*{self.calcular_checksum(payload)}\r\n"

            accion = raw_cmd[13:]

            if accion == "SET_MODO:AUTO":
                self.modo = "AUTO"
                self.estado = "V1_R2"
                self.countdown = self.tiempo_verde_min * 60
                self.countdown_max = self.tiempo_verde_min * 60
                self.ultimo_evento = "Modo Automático reanudado."
                payload = "ACK,CMD:SET_MODO:AUTO,RESULT:OK"

            elif accion == "SET_MODO:MANUAL":
                self.modo = "MANUAL"
                self.ultimo_evento = "Modo Manual activado."
                payload = "ACK,CMD:SET_MODO:MANUAL,RESULT:OK"

            elif accion == "SET_MODO:AMBAR":
                self.modo = "AMBAR"
                self.estado = "AMBAR_FAIL"
                self.countdown = 0
                self.ultimo_evento = "Modo Ámbar Precaución activado."
                payload = "ACK,CMD:SET_MODO:AMBAR,RESULT:OK"

            elif accion == "FORZAR_ROJO":
                self.modo = "ROJO_TOTAL"
                self.estado = "R1_R2"
                self.countdown = 0
                self.ultimo_evento = "Rojo Total de Emergencia accionado."
                payload = "ACK,CMD:FORZAR_ROJO,RESULT:OK"

            elif accion == "MANUAL:CAMBIAR_TURNO":
                self.modo = "MANUAL"
                if self.estado == "V1_R2":
                    self.estado = "R1_R2"
                    self.countdown = self.tiempo_despeje_seg
                    self.countdown_max = self.tiempo_despeje_seg
                    self.ultimo_evento = "Maniobra: Despeje para cambio de turno."
                else:
                    self.estado = "V1_R2"
                    self.countdown = self.tiempo_verde_min * 60
                    self.countdown_max = self.tiempo_verde_min * 60
                    self.ultimo_evento = "Maniobra: Paso concedido a Sentido 1."
                payload = "ACK,CMD:CAMBIAR_TURNO,RESULT:OK"

            elif accion == "TEST_LEDS":
                self.ultimo_evento = "Test de Luces iniciado (6s)."
                payload = "ACK,CMD:TEST_LEDS,RESULT:STARTING_6S"

            elif accion.startswith("SET_TIEMPOS:"):
                # SET_TIEMPOS:V,R,D
                try:
                    parts = accion[12:].split(",")
                    v, r, d = int(parts[0]), int(parts[1]), int(parts[2])
                    if 1 <= v <= 15 and 1 <= r <= 15 and 10 <= d <= 90:
                        self.tiempo_verde_min = v
                        self.tiempo_rojo_min = r
                        self.tiempo_despeje_seg = d
                        self.countdown = v * 60
                        self.countdown_max = v * 60
                        self.ultimo_evento = f"Tiempos cambiados: V={v}m, R={r}m, D={d}s"
                        payload = "ACK,CMD:SET_TIEMPOS,RESULT:OK"
                    else:
                        payload = "ERR,CMD:SET_TIEMPOS,DESC:RANGO_INVALIDO"
                except Exception:
                    payload = "ERR,CMD:SET_TIEMPOS,DESC:FORMATO_INVALIDO"

            elif accion.startswith("SET_RTC:"):
                self.ultimo_evento = f"Reloj RTC sincronizado: {accion[8:]}"
                payload = "ACK,CMD:SET_RTC,RESULT:OK"

            elif accion == "SOLICITAR_PASO":
                self.ultimo_evento = "Petición de paso recibida desde Esclavo."
                payload = "ACK,CMD:SOLICITAR_PASO,RESULT:FORWARDED_TO_MASTER"

            else:
                payload = "ERR,CMD:DESCONOCIDO,DESC:NO_SOPORTADO"

            return f"${payload}*{self.calcular_checksum(payload)}\r\n"

    def generar_trama_status(self):
        with self.lock:
            payload = f"STATUS,NODE:{self.node},MODO:{self.modo},ESTADO:{self.estado},RESTANTE:{self.countdown},TOT:{self.countdown_max},BAT:{self.bateria_v:.1f},RF:{self.rf_calidad},RTT:{self.rtt_ms},SERIE:{self.serie}"
            return f"${payload}*{self.calcular_checksum(payload)}\r\n"

# Instancia global del simulador
simulador_controlador = FirmwareSTM32Simulator()

def cycle_ticker_loop():
    while True:
        simulador_controlador.tick()
        time.sleep(1.0)

# Ticker en segundo plano
ticker_thread = threading.Thread(target=cycle_ticker_loop, daemon=True)
ticker_thread.start()

class BridgeHTTPHandler(http.server.SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        directory = os.path.dirname(os.path.abspath(__file__))
        super().__init__(*args, directory=directory, **kwargs)

    def do_GET(self):
        parsed = urllib.parse.urlparse(self.path)

        if parsed.path == "/api/telemetria":
            # Devuelve trama NMEA pura $STATUS
            trama = simulador_controlador.generar_trama_status()
            self.send_response(200)
            self.send_header("Content-Type", "text/plain; charset=utf-8")
            self.send_header("Access-Control-Allow-Origin", "*")
            self.end_headers()
            self.wfile.write(trama.encode("utf-8"))
            return

        elif parsed.path == "/api/status_json":
            # Devuelve JSON parseado del simulador
            with simulador_controlador.lock:
                data = {
                    "node": simulador_controlador.node,
                    "modo": simulador_controlador.modo,
                    "estado": simulador_controlador.estado,
                    "countdown": simulador_controlador.countdown,
                    "countdown_max": simulador_controlador.countdown_max,
                    "bat": simulador_controlador.bateria_v,
                    "rf": simulador_controlador.rf_calidad,
                    "rtt": simulador_controlador.rtt_ms,
                    "serie": simulador_controlador.serie,
                    "ultimo_evento": simulador_controlador.ultimo_evento,
                    "trama_nmea": simulador_controlador.generar_trama_status().strip()
                }
            self.send_response(200)
            self.send_header("Content-Type", "application/json; charset=utf-8")
            self.send_header("Access-Control-Allow-Origin", "*")
            self.end_headers()
            self.wfile.write(json.dumps(data).encode("utf-8"))
            return

        # Archivos estáticos HTML/JS/CSS normales
        super().do_GET()

    def do_POST(self):
        parsed = urllib.parse.urlparse(self.path)

        if parsed.path == "/api/cmd":
            content_length = int(self.headers.get('Content-Length', 0))
            body = self.rfile.read(content_length).decode('utf-8')
            
            try:
                data = json.loads(body)
                cmd_str = data.get("cmd", "")
            except Exception:
                cmd_str = body

            # Enviar comando al simulador de firmware
            respuesta_nmea = simulador_controlador.procesar_comando(cmd_str)

            self.send_response(200)
            self.send_header("Content-Type", "application/json; charset=utf-8")
            self.send_header("Access-Control-Allow-Origin", "*")
            self.end_headers()
            
            resp_data = {
                "cmd_enviado": cmd_str,
                "resp_nmea": respuesta_nmea.strip(),
                "modo_actual": simulador_controlador.modo,
                "estado_actual": simulador_controlador.estado
            }
            self.wfile.write(json.dumps(resp_data).encode("utf-8"))
            return

        self.send_response(404)
        self.end_headers()

    def do_OPTIONS(self):
        self.send_response(200)
        self.send_header("Access-Control-Allow-Origin", "*")
        self.send_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS")
        self.send_header("Access-Control-Allow-Headers", "Content-Type")
        self.end_headers()

def run_server(port=3000):
    socketserver.ThreadingTCPServer.allow_reuse_address = True
    with socketserver.ThreadingTCPServer(("", port), BridgeHTTPHandler) as httpd:
        print("=" * 80)
        print(f"[OK] SERVIDOR PUENTE MULTIHILO STM32 ACTIVO EN: http://localhost:{port}/")
        print("=" * 80)
        print("  * Frontend App Web:     http://localhost:3000/")
        print("  * Telemetria NMEA ($):  http://localhost:3000/api/telemetria")
        print("  * Estado en Vivo JSON:  http://localhost:3000/api/status_json")
        print("  * Receptor de Comandos: POST http://localhost:3000/api/cmd")
        print("=" * 80)
        httpd.serve_forever()

if __name__ == "__main__":
    port = int(sys.argv[1]) if len(sys.argv) > 1 else 3000
    run_server(port)
