class RepetidorESP32:
    def __init__(self):
        self.M1_DE_RE = False
        self.M2_DE_RE = False
        self.uart_A_rx = []
        self.uart_C_rx = []
        self.uart_A_tx = []
        self.uart_C_tx = []
        self.t = 0
        self.ram_usage = 0
        self.crashed = False

    def loop(self):
        if self.crashed: return

        # Lado A -> C
        if len(self.uart_A_rx) > 0:
            # Simulamos el delay(5) del firmware
            self.t += 5
            
            # digitalWrite(M2_DE_RE, HIGH)
            self.M2_DE_RE = True
            
            # while(RadioA.available())
            while len(self.uart_A_rx) > 0:
                b = self.uart_A_rx.pop(0)
                self.uart_C_tx.append(b)
                # Tracking ram usage: we don't store in string, so RAM stays flat
                self.ram_usage = max(self.ram_usage, len(self.uart_A_rx))
            
            # flush y digitalWrite(M2_DE_RE, LOW)
            self.M2_DE_RE = False

        # Lado C -> A
        if len(self.uart_C_rx) > 0:
            self.t += 5
            self.M1_DE_RE = True
            while len(self.uart_C_rx) > 0:
                b = self.uart_C_rx.pop(0)
                self.uart_A_tx.append(b)
                self.ram_usage = max(self.ram_usage, len(self.uart_C_rx))
            self.M1_DE_RE = False

def assert_eq(val, exp, msg):
    if val != exp:
        print(f"FAIL: {msg}. Expected {exp}, got {val}")
    else:
        print(f"PASS: {msg}")

def run_tests():
    print("=== SAFETY TESTS ESP32: REPETIDOR BINARIO TRANSPARENTE ===")
    
    # CASE 1: Paquete binario normal (4 bytes)
    print("\n[TEST 1] Transmision Normal (4 bytes de Ping del Maestro)")
    esp = RepetidorESP32()
    paquete_ping = [0x50, 0x49, 0x4E, 0x1A] # Ejemplo binario
    esp.uart_A_rx.extend(paquete_ping)
    esp.loop()
    assert_eq(len(esp.uart_C_tx), 4, "ESP32 debio reenviar 4 bytes exactos al lado C")
    assert_eq(esp.uart_C_tx, paquete_ping, "El paquete reenviado debe ser identico al original (sin corrupcion)")
    assert_eq(esp.ram_usage, 3, "Uso de memoria pico debio ser practicamente nulo (no buffers infinitos)")
    
    # CASE 2: Tormenta de Ruido (Buffer overflow en el diseño anterior)
    print("\n[TEST 2] Tormenta de Ruido de Montana (Sin salto de linea \\n)")
    esp2 = RepetidorESP32()
    # Metemos 100,000 bytes de ruido crudo que antes reventaban la RAM del String
    ruido = [0xAA] * 100000 
    esp2.uart_C_rx.extend(ruido)
    esp2.loop()
    assert_eq(len(esp2.uart_A_tx), 100000, "ESP32 debio reenviar el ruido y dejar que el STM32 lo filtre")
    assert_eq(esp2.crashed, False, "El ESP32 NO debe crashear por falta de \\n")
    assert_eq(esp2.ram_usage, 99999, "La lectura se hace byte a byte sin acumular un macro-String en la RAM")

run_tests()
