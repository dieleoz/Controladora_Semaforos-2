import time
import random

class RepetidorVulnerable:
    """Emula la lógica actual (peligrosa) del ESP32 RepetidorB.ino"""
    def __init__(self):
        self.buf = ""
    
    def recibir_caracter(self, c):
        # Vulnerabilidad: Solo se vacía si llega un \n
        if c == '\n':
            self.buf = ""
        elif c != '\r':
            self.buf += c

class RepetidorSeguro:
    """Emula la lógica propuesta para solucionar la falla del ESP32"""
    def __init__(self, max_buffer=64):
        self.buf = ""
        self.max_buffer = max_buffer
        self.last_char_time = time.time()
        
    def recibir_caracter(self, c, current_time):
        # 1. Protección por Timeout (ej. > 500ms sin recibir nada asume corte de señal)
        if current_time - self.last_char_time > 0.5:
            if len(self.buf) > 0:
                self.buf = "" # Vaciamos la basura anterior
        
        self.last_char_time = current_time

        if c == '\n':
            self.buf = ""
        elif c != '\r':
            self.buf += c
            # 2. Protección por Límite Físico (Previene Heap Overflow)
            if len(self.buf) >= self.max_buffer:
                self.buf = "" # Vaciar basura acumulada

def inyectar_ruido(repetidor, nombre, limite_memoria=100000):
    start = time.time()
    bytes_enviados = 0
    print(f"\n--- Iniciando Ataque de Ruido a: {nombre} ---")
    
    while True:
        # Inyectar ruido al azar simulando estática de radio (sin \n)
        c = chr(random.randint(32, 126)) 
        
        # Pasar el tiempo actual para la lógica de timeout (aunque aquí bombardeamos rápido)
        current_time = time.time()
        if hasattr(repetidor, 'last_char_time'):
            repetidor.recibir_caracter(c, current_time)
        else:
            repetidor.recibir_caracter(c)
            
        bytes_enviados += 1
        
        # Simular que la memoria RAM colapsa si el buffer crece demasiado
        if len(repetidor.buf) > limite_memoria:
            print(f"FATAL ERROR: Heap Overflow detectado en {nombre}.")
            print(f"El microcontrolador se colgo tras acumular {bytes_enviados} bytes de pura basura.")
            return False
            
        # Detener la prueba si sobrevivió la inyección de 300,000 bytes
        if bytes_enviados >= 300000:
            print(f"EXITO: {nombre} sobrevivio a {bytes_enviados} bytes de ruido. Tamano final del buffer: {len(repetidor.buf)} bytes.")
            return True

if __name__ == "__main__":
    print("Iniciando Fase de Simulación (Step 3) - Prueba de Resiliencia LoRa")
    
    # 1. Probar el código actual (que está fallando en terreno)
    esp32_actual = RepetidorVulnerable()
    inyectar_ruido(esp32_actual, "ESP32 (Lógica Actual Vulnerable)")
    
    time.sleep(1)
    
    # 2. Probar la propuesta de optimización
    esp32_optimizado = RepetidorSeguro(max_buffer=64)
    inyectar_ruido(esp32_optimizado, "ESP32 (Lógica Optimizada con Limites)")
