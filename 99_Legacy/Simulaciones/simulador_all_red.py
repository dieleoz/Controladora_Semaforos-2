import time

class TunelFisico:
    def __init__(self, longitud_metros=100, velocidad_auto_ms=10):
        self.autos_en_tunel = []
        self.longitud = longitud_metros
        self.velocidad = velocidad_auto_ms
        self.colisiones = 0
        
    def entrar_auto(self, id_auto, direccion):
        self.autos_en_tunel.append({"id": id_auto, "dir": direccion, "pos": 0})
        # Check colisiones frontales inmediatas
        direcciones = {a["dir"] for a in self.autos_en_tunel}
        if len(direcciones) > 1:
            print(f"FATAL: COLISION FRONTAL DETECTADA! Autos viajando en sentidos opuestos: {direcciones}")
            self.colisiones += 1

    def actualizar_fisica(self, delta_sec):
        vivos = []
        for a in self.autos_en_tunel:
            a["pos"] += self.velocidad * delta_sec
            if a["pos"] < self.longitud:
                vivos.append(a)
            else:
                pass # print(f"Auto {a['id']} salió del túnel con éxito.")
        self.autos_en_tunel = vivos

class CoordinadorVulnerable:
    def __init__(self):
        self.estado = "VERDE_A"
        
    def cambiar_via(self):
        # Vulnerabilidad: Cambio casi instantáneo o con tiempo de despeje insuficiente
        print("\n[Coord Actual] Cambiando Via...")
        self.estado = "ROJO_A_AMBOS"
        # Simula el "tiempoEstatico" actual (e.g. 1 segundo, no alcanza a despejar el túnel)
        return 1.0 

class CoordinadorSeguroSFTY4:
    def __init__(self, longitud_tunel, vel_auto):
        self.estado = "VERDE_A"
        self.tiempo_all_red_seguro = (longitud_tunel / vel_auto) + 2.0 # Tiempo de cruce + 2s de margen
        
    def cambiar_via(self):
        print(f"\n[Coord SFTY-4] Cambiando Via... Forzando ALL-RED (Ambos en Rojo) por {self.tiempo_all_red_seguro}s")
        self.estado = "ALL_RED"
        return self.tiempo_all_red_seguro

def simular_trafico(coordinador, tunel, nombre_prueba):
    print(f"\n======================================")
    print(f"Iniciando Prueba: {nombre_prueba}")
    print(f"======================================")
    
    # Via A en Verde. Entran 2 autos.
    tunel.entrar_auto("Auto_1_ViaA", "Norte-Sur")
    tunel.actualizar_fisica(2) # Pasan 2 segundos
    tunel.entrar_auto("Auto_2_ViaA", "Norte-Sur")
    
    # Se pide cambio de vía (Ej. llega camión por Vía B)
    tiempo_espera_rojo = coordinador.cambiar_via()
    
    # Pasa el tiempo de espera programado en la controladora (All-Red)
    tunel.actualizar_fisica(tiempo_espera_rojo)
    
    # Se da Verde a la Vía B
    print("-> Se encendio el VERDE en la Via B.")
    coordinador.estado = "VERDE_B"
    tunel.entrar_auto("Camion_ViaB", "Sur-Norte")
    
    if tunel.colisiones == 0:
        print(f"EXITO SFTY-4: Ninguna colision. Tunel despejado correctamente.")
    else:
        print(f"FALLA DE SEGURIDAD: Colision registrada por falta de tiempo de despeje.")

if __name__ == "__main__":
    # Parámetros del mundo real
    LONGITUD_TUNEL_M = 100
    VELOCIDAD_MS = 10 # 36 km/h (velocidad típica en túnel de obra)
    
    # Prueba 1: Coordinador actual (tiempoEstatico insuficiente o mal calculado)
    tunel_1 = TunelFisico(LONGITUD_TUNEL_M, VELOCIDAD_MS)
    coord_vuln = CoordinadorVulnerable()
    simular_trafico(coord_vuln, tunel_1, "Coordinador Actual (Sin All-Red dinámico)")
    
    # Prueba 2: Coordinador SFTY-4 (All-Red calculado matemáticamente)
    tunel_2 = TunelFisico(LONGITUD_TUNEL_M, VELOCIDAD_MS)
    coord_seguro = CoordinadorSeguroSFTY4(LONGITUD_TUNEL_M, VELOCIDAD_MS)
    simular_trafico(coord_seguro, tunel_2, "Coordinador Seguro SFTY-4 (Despeje Total)")
