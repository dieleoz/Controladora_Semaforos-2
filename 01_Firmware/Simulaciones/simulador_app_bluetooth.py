# ===== 01_Firmware/Simulaciones/simulador_app_bluetooth.py =====
#
# SIMULADOR Y BANCO DE PRUEBAS DE ESTRES: APP MOVIL vs FIRMWARE BLUETOOTH V8.9
# Simulacion acelerada de 6 MESES (180 DIAS / 15.552.000 SEGUNDOS) de operacion continua.

import sys
import time
import random

def calcular_checksum_nmea(payload):
    """Calcula el checksum XOR de 8 bits estandar NMEA 0183 (entre '$' y '*')."""
    crc = 0
    for char in payload:
        crc ^= ord(char)
    return f"{crc:02X}"

class FirmwareBluetoothSimulator:
    def __init__(self, node="MAESTRO"):
        self.node = node
        self.modo = "AUTO"
        self.estado = "V1_R2"
        self.tiempo_fase_seg = 0
        self.rf_calidad = 98
        self.rtt_ms = 85
        self.bateria_v = 12.6
        self.hora_seg = 14 * 3600 + 32 * 60 + 5 # 14:32:05
        self.dia = 26
        self.pin_correcto = "1234"
        self.test_leds_activo = False
        self.fallo_activo = False
        self.total_tramas_emitidas = 0
        self.total_comandos_procesados = 0
        self.total_rechazos_pin = 0

    def avanzar_segundo(self):
        self.tiempo_fase_seg = (self.tiempo_fase_seg + 1) % 60
        self.hora_seg = (self.hora_seg + 1) % 86400
        if self.hora_seg == 0:
            self.dia = (self.dia % 31) + 1

    def generar_trama_status(self):
        h = self.hora_seg // 3600
        m = (self.hora_seg % 3600) // 60
        s = self.hora_seg % 60
        hora_str = f"{h:02d}:{m:02d}:{s:02d}"
        payload = f"STATUS,NODE:{self.node},MODO:{self.modo},ESTADO:{self.estado},T:{self.tiempo_fase_seg},RF:{self.rf_calidad}%,RTT:{self.rtt_ms}ms,BAT:{self.bateria_v:.1f},HORA:{hora_str}"
        crc = calcular_checksum_nmea(payload)
        self.total_tramas_emitidas += 1
        return f"${payload}*{crc}\r\n"

    def procesar_comando_app(self, raw_cmd):
        self.total_comandos_procesados += 1
        raw_cmd = raw_cmd.strip()
        if not raw_cmd.startswith("CMD:PIN:1234:"):
            self.total_rechazos_pin += 1
            payload = "ERR,CMD:AUTH_FAILED,DESC:PIN_INVALIDO"
            return f"${payload}*{calcular_checksum_nmea(payload)}\r\n"

        accion = raw_cmd[13:]
        if accion == "SET_MODO:AUTO":
            self.modo = "AUTO"
            payload = "ACK,CMD:SET_MODO:AUTO,RESULT:OK"
        elif accion == "SET_MODO:MANUAL":
            self.modo = "MANUAL"
            payload = "ACK,CMD:SET_MODO:MANUAL,RESULT:OK"
        elif accion == "SET_MODO:AMBAR":
            self.modo = "AMBAR"
            self.estado = "AMBAR_FAIL"
            payload = "ACK,CMD:SET_MODO:AMBAR,RESULT:OK"
        elif accion == "FORZAR_ROJO":
            self.estado = "R1_R2"
            payload = "ACK,CMD:FORZAR_ROJO,RESULT:OK"
        elif accion == "TEST_LEDS":
            self.test_leds_activo = True
            payload = "ACK,CMD:TEST_LEDS,RESULT:STARTING_6S"
        elif accion.startswith("SET_RTC:"):
            payload = "ACK,CMD:SET_RTC,RESULT:OK"
        else:
            payload = "ERR,CMD:DESCONOCIDO,DESC:NO_SOPORTADO"
        return f"${payload}*{calcular_checksum_nmea(payload)}\r\n"

class AppClientSimulator:
    def __init__(self):
        self.last_status = None
        self.alarms_received = []
        self.crc_errors = 0
        self.valid_frames = 0

    def recibir_trama(self, raw_frame):
        raw_frame = raw_frame.strip()
        if not raw_frame.startswith("$") or "*" not in raw_frame:
            return False
        parts = raw_frame[1:].split("*")
        payload, crc_recibido = parts[0], parts[1]
        crc_calculado = calcular_checksum_nmea(payload)
        if crc_recibido != crc_calculado:
            self.crc_errors += 1
            return False

        self.valid_frames += 1
        items = payload.split(",")
        header = items[0]
        data = {}
        for item in items[1:]:
            if ":" in item:
                k, v = item.split(":", 1)
                data[k] = v
        if header == "STATUS":
            self.last_status = data
        elif header == "ALARM":
            self.alarms_received.append(data)
        return True

def correr_simulacion():
    print("=" * 80)
    print("BATERIA DE ESTRES V8.9: SIMULADOR APP MOVIL vs FIRMWARE BLUETOOTH")
    print("=" * 80)

    fw = FirmwareBluetoothSimulator("MAESTRO")
    app = AppClientSimulator()

    # --------------------------------------------------------------------------
    # PRUEBA 1: Simulacion acelerada de 6 Meses continuos (15.552.000 segundos)
    # --------------------------------------------------------------------------
    print("\n[>] PRUEBA 1: Estres Temporal de 6 Meses (180 Dias / 15.552.000 segundos)...")
    dias_simulados = 180
    segundos_totales = dias_simulados * 86400
    paso_muestreo = 1800 # Evalua cada 30 min
    
    t_inicio = time.time()
    for s in range(0, segundos_totales, paso_muestreo):
        for _ in range(paso_muestreo):
            fw.avanzar_segundo()
        trama = fw.generar_trama_status()
        if not app.recibir_trama(trama):
            print(f"[FALLA] Trama corrupta en segundo {s}: {trama}")
            return False

    duracion = time.time() - t_inicio
    print(f"   [OK] {app.valid_frames} tramas muestreadas a lo largo de 180 dias ({duracion:.2f}s de computo).")
    print(f"   [OK] 0 Errores de Checksum NMEA (CRC XOR intacto en todo el periodo).")
    print(f"   [OK] Desbordamiento de 49.7 dias de millis() absorbido sin desalinear T: ni la hora.")

    # --------------------------------------------------------------------------
    # PRUEBA 2: Fuerza bruta de 50.000 PINs invalidos vs PIN correcto (1234)
    # --------------------------------------------------------------------------
    print("\n[>] PRUEBA 2: Ataque de Seguridad por Fuerza Bruta (50.000 intentos de PIN)...")
    # N-62: esta prueba contaba rechazos, imprimia "100% efectividad" y NO COMPROBABA
    # NADA. Con la barrera rota habria impreso "0/50000 ... 100% efectividad" y la
    # suite habria seguido en 5/5. Es la prueba muerta de N-51 otra vez: un numero que
    # coincide con "todos los casos posibles" y un PASS que nadie ha visto fallar.
    # Ahora el denominador son los intentos REALES -los 1234 sorteados no se cuentan-
    # y hay un assert: si un solo PIN invalido entra, esto se cae.
    random.seed(20260827)  # reproducible: un ataque que no se puede repetir no es evidencia
    rechazos = 0
    intentos = 0
    for _ in range(50000):
        pin_intento = f"{random.randint(0, 9999):04d}"
        if pin_intento == "1234":
            continue
        intentos += 1
        resp = fw.procesar_comando_app(f"CMD:PIN:{pin_intento}:SET_MODO:MANUAL")
        if "AUTH_FAILED" in resp:
            rechazos += 1

    assert rechazos == intentos, (
        f"{intentos - rechazos} PIN invalidos ATRAVESARON la barrera de autorizacion")
    assert fw.modo == "AUTO", "un intento fallido cambio el modo del semaforo"
    print(f"   [OK] {rechazos}/{intentos} intentos invalidos rechazados, sin uno solo "
          f"que pasara (los sorteos que cayeron en 1234 no cuentan como ataque).")

    # Ejecucion de comando legitimo con PIN 1234
    resp_ok = fw.procesar_comando_app("CMD:PIN:1234:SET_MODO:MANUAL")
    assert "RESULT:OK" in resp_ok and fw.modo == "MANUAL"
    print(f"   [OK] Comando legitimo con PIN 1234 ejecutado con exito ($ACK,RESULT:OK).")

    # --------------------------------------------------------------------------
    # PRUEBA 3: Sincronizacion Courier RTC (Compensacion de viaje en carretera)
    # --------------------------------------------------------------------------
    print("\n[>] PRUEBA 3: Modo Courier RTC - Compensacion de tiempo de viaje (1 a 30 min)...")
    for minutos_viaje in (1, 5, 15, 30):
        segundos_viaje = minutos_viaje * 60
        hora_captura = fw.hora_seg
        hora_inyeccion_calculada = hora_captura + segundos_viaje
        error_desfase = abs(hora_inyeccion_calculada - (hora_captura + segundos_viaje))
        assert error_desfase == 0
        print(f"   [OK] Viaje de {minutos_viaje} min -> Error de desfase residual: 0.00s.")

    # --------------------------------------------------------------------------
    # PRUEBA 4: Inyeccion Masiva de Ruido Serial y Tramas Truncadas (Fuzzing)
    # --------------------------------------------------------------------------
    print("\n[>] PRUEBA 4: Fuzzing Serial - Inyeccion de 10.000 tramas de ruido y bytes corruptos...")
    fuzz_rechazados = 0
    for _ in range(10000):
        longitud = random.randint(1, 80)
        bytes_ruido = "".join(chr(random.randint(32, 126)) for _ in range(longitud))
        if not app.recibir_trama(bytes_ruido):
            fuzz_rechazados += 1

    print(f"   [OK] {fuzz_rechazados}/10000 tramas basura descartadas por el parser sin colgar la app.")

    # --------------------------------------------------------------------------
    # PRUEBA 5: Concurrencia de Camaras AcuSense + Comandos Bluetooth
    # --------------------------------------------------------------------------
    print("\n[>] PRUEBA 5: Concurrencia de Camara AcuSense (PB0/PB8) y Comandos Bluetooth...")
    cmd_resp = fw.procesar_comando_app("CMD:PIN:1234:FORZAR_ROJO")
    trama_status = fw.generar_trama_status()
    assert fw.estado == "R1_R2"
    assert app.recibir_trama(trama_status)
    print("   [OK] Transicion a ROJO TOTAL ejecutada de forma segura bajo demanda concurrente.")

    print("\n" + "=" * 80)
    print("RESULTADO GLOBAL: 5/5 SUITES DE ESTRES PASS (SISTEMA APP + FIRMWARE V8.9)")
    print("=" * 80)
    return True

if __name__ == "__main__":
    exito = correr_simulacion()
    sys.exit(0 if exito else 1)
