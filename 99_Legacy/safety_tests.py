class StateMachine:
    def __init__(self):
        self.estadoC = 'C_IDLE'
        self.quienVerde = 'QV_NINGUNO'
        self.tRef = 0
        self.tEsperandoAck = 0
        self.retryCount = 0
        self.tiempoDespejeMs = 15000
        self.TIMEOUT_ACK_MS = 1500
        self.tUltimoPing = 0
        self.handshakeOk = True
        self.tFalloCom = 0
        self.slave_estado = 'ROJO'
        self.master_estado = 'ROJO'
        self.t = 0
        self.logs = []

    def log(self, msg):
        self.logs.append(f"[T={self.t}ms] {msg}")

    def update(self, rx_packet=None):
        if self.t - self.tUltimoPing > 3000:
            self.tUltimoPing = self.t
            if not self.handshakeOk:
                if self.tFalloCom == 0:
                    self.tFalloCom = self.t
                elif self.t - self.tFalloCom > 9000:
                    if self.estadoC != 'C_FALLO':
                        self.log("!! HEARTBEAT TIMEOUT -> ENTRANDO A C_FALLO (ROJO INTERMITENTE) !!")
                    self.estadoC = 'C_FALLO'
                    self.master_estado = 'FALLO'
            else:
                self.handshakeOk = False
                self.tFalloCom = 0
                if self.estadoC == 'C_FALLO':
                    self.log("++ SENAL RECUPERADA (PONG RECIBIDO) -> SALIENDO DE FALLO, FORZANDO ROJOS FIJOS ++")
                    self.estadoC = 'C_IDLE'
                    self.quienVerde = 'QV_NINGUNO'
                    self.master_estado = 'ROJO'

        if rx_packet == 'PONG':
            self.handshakeOk = True

        if self.estadoC == 'C_IDLE':
            pass
        elif self.estadoC == 'C_INICIAL_ESPERA_ESTATICO':
            if self.t - self.tRef >= self.tiempoDespejeMs:
                self.master_estado = 'VERDE'
                self.estadoC = 'C_INICIAL_MASTER_A_VERDE'
                self.log(">> TIEMPO DESPEJE FINALIZADO -> MAESTRO A VERDE")
        elif self.estadoC == 'C_INICIAL_MASTER_A_VERDE':
            self.quienVerde = 'QV_MASTER'
            self.estadoC = 'C_IDLE'
        elif self.estadoC == 'C_MASTER_A_ROJO':
            self.master_estado = 'ROJO'
            self.tRef = self.t
            self.estadoC = 'C_ESPERA_ESTATICO_TRAS_MASTER'
            self.log(">> MAESTRO EN ROJO -> INICIA TIEMPO DE DESPEJE (ALL-RED)")
        elif self.estadoC == 'C_ESPERA_ESTATICO_TRAS_MASTER':
            if self.t - self.tRef >= self.tiempoDespejeMs:
                self.log(">> TIEMPO DESPEJE FINALIZADO -> ENVIANDO GO_GREEN A ESCLAVO")
                self.tEsperandoAck = self.t
                self.retryCount = 0
                self.estadoC = 'C_ESPERANDO_ACK_GREEN'
        elif self.estadoC == 'C_ESPERANDO_ACK_GREEN':
            if rx_packet == 'ACK_GREEN':
                self.slave_estado = 'VERDE'
                self.quienVerde = 'QV_ESCLAVO'
                self.estadoC = 'C_IDLE'
                self.log(">> ESCLAVO RECIBIO VERDE CON EXITO")
            elif self.t - self.tEsperandoAck > self.TIMEOUT_ACK_MS:
                self.retryCount += 1
                if self.retryCount >= 3:
                    self.estadoC = 'C_FALLO'
                    self.master_estado = 'FALLO'
                    self.log("!! FALLO AL ENVIAR VERDE AL ESCLAVO -> C_FALLO !!")
                else:
                    self.tEsperandoAck = self.t
        elif self.estadoC == 'C_ESPERANDO_ACK_RED':
            if rx_packet == 'ACK_RED':
                self.slave_estado = 'ROJO'
                self.tRef = self.t
                self.estadoC = 'C_ESPERA_ESTATICO_TRAS_ESCLAVO'
                self.log(">> ESCLAVO RECIBIO ROJO -> INICIA TIEMPO DE DESPEJE (ALL-RED)")
            elif self.t - self.tEsperandoAck > self.TIMEOUT_ACK_MS:
                self.retryCount += 1
                if self.retryCount >= 3:
                    self.estadoC = 'C_FALLO'
                    self.master_estado = 'FALLO'
                    self.log("!! FALLO AL ENVIAR ROJO AL ESCLAVO -> C_FALLO !!")
                else:
                    self.tEsperandoAck = self.t
        elif self.estadoC == 'C_ESPERA_ESTATICO_TRAS_ESCLAVO':
            if self.t - self.tRef >= self.tiempoDespejeMs:
                self.master_estado = 'VERDE'
                self.estadoC = 'C_MASTER_A_VERDE'
                self.log(">> TIEMPO DESPEJE FINALIZADO -> MAESTRO A VERDE")
        elif self.estadoC == 'C_MASTER_A_VERDE':
            self.quienVerde = 'QV_MASTER'
            self.estadoC = 'C_IDLE'

    def pedirCambio(self):
        if self.estadoC != 'C_IDLE': return
        if self.quienVerde == 'QV_NINGUNO':
            self.tRef = self.t
            self.estadoC = 'C_INICIAL_ESPERA_ESTATICO'
            self.log(">> INICIA ARRANQUE: ESPERANDO TIEMPO DE DESPEJE INICIAL")
        elif self.quienVerde == 'QV_MASTER':
            self.estadoC = 'C_MASTER_A_ROJO'
            self.log(">> PIDIENDO ROJO AL MAESTRO")
        elif self.quienVerde == 'QV_ESCLAVO':
            self.log(">> PIDIENDO ROJO AL ESCLAVO")
            self.tEsperandoAck = self.t
            self.retryCount = 0
            self.estadoC = 'C_ESPERANDO_ACK_RED'

def test_recovery_car_in_middle():
    print("--- TEST 1: AUTO EN EL MEDIO Y CORTE DE SENAL ---")
    sm = StateMachine()
    sm.tiempoDespejeMs = 15000
    
    # Arrancar sistema 
    sm.pedirCambio()
    for i in range(16000):
        sm.t = i
        sm.update('PONG' if i % 3000 == 100 else None)
        
    print(f"T=16000 Estado tras arranque: Maestro={sm.master_estado}")
    
    # Corte de senal abrupto (60 segundos)
    print(">>> SIMULANDO CORTE DE SENAL... (Cables Cortados/Lluvia)")
    for i in range(16000, 76000):
        sm.t = i
        sm.update(None)
        if sm.estadoC == 'C_IDLE': # Simulamos que durante el corte, el automovil pide cambio
           if sm.master_estado == 'VERDE':
               sm.pedirCambio()
        
    print(f"T=76000 Estado tras corte prolongado (Debe ser FALLO): Maestro={sm.master_estado}")
    
    # Restaurar senal 
    print(">>> SENAL RESTAURADA...")
    for i in range(76000, 80000):
        sm.t = i
        sm.update('PONG')
        
    print(f"T=80000 Estado tras restaurar (Debe ser ROJO): Maestro={sm.master_estado}")
    
    # Simulamos que el Modo Automatico pide cambio para iniciar ciclo tras el rojo de reseteo
    sm.pedirCambio()
    
    # Avanzar tiempo durante el despeje
    for i in range(80000, 96000):
        sm.t = i
        sm.update('PONG')
        if i == 89000:
            print(f"T=89000ms (Llevamos 9s de Despeje). Maestro={sm.master_estado}")
            
    print(f"T=96000 Estado tras 15s de Despeje: Maestro={sm.master_estado}")
    print("\nLogs:")
    for l in sm.logs: print(l)

test_recovery_car_in_middle()
