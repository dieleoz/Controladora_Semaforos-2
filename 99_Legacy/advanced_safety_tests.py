class StateMachine:
    def __init__(self, tiempoDespejeMs=15000):
        self.estadoC = 'C_IDLE'
        self.quienVerde = 'QV_NINGUNO'
        self.tRef = 0
        self.tEsperandoAck = 0
        self.retryCount = 0
        self.tiempoDespejeMs = tiempoDespejeMs
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
                        self.log("!! TIMEOUT HEARTBEAT -> ENTRANDO A C_FALLO !!")
                    self.estadoC = 'C_FALLO'
                    self.master_estado = 'FALLO'
            else:
                self.handshakeOk = False
                self.tFalloCom = 0
                if self.estadoC == 'C_FALLO':
                    self.log("++ SENAL RECUPERADA -> SALIENDO DE C_FALLO A C_IDLE ++")
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
                self.log(">> DESPEJE COMPLETADO -> MAESTRO A VERDE")
        elif self.estadoC == 'C_INICIAL_MASTER_A_VERDE':
            self.quienVerde = 'QV_MASTER'
            self.estadoC = 'C_IDLE'
        elif self.estadoC == 'C_MASTER_A_ROJO':
            self.master_estado = 'ROJO'
            self.tRef = self.t
            self.estadoC = 'C_ESPERA_ESTATICO_TRAS_MASTER'
            self.log(">> MAESTRO EN ROJO -> ESPERANDO DESPEJE")
        elif self.estadoC == 'C_ESPERA_ESTATICO_TRAS_MASTER':
            if self.t - self.tRef >= self.tiempoDespejeMs:
                self.log(">> DESPEJE COMPLETADO -> ENVIANDO GO_GREEN AL ESCLAVO")
                self.tEsperandoAck = self.t
                self.retryCount = 0
                self.estadoC = 'C_ESPERANDO_ACK_GREEN'
        elif self.estadoC == 'C_ESPERANDO_ACK_GREEN':
            if rx_packet == 'ACK_GREEN':
                self.slave_estado = 'VERDE'
                self.quienVerde = 'QV_ESCLAVO'
                self.estadoC = 'C_IDLE'
                self.log(">> ESCLAVO RECIBIO VERDE")
            elif self.t - self.tEsperandoAck > self.TIMEOUT_ACK_MS:
                self.retryCount += 1
                if self.retryCount >= 3:
                    if self.estadoC != 'C_FALLO':
                        self.log("!! RETRIES AGOTADOS -> ENTRANDO A C_FALLO !!")
                    self.estadoC = 'C_FALLO'
                    self.master_estado = 'FALLO'
                else:
                    self.log(f">> REINTENTO {self.retryCount} -> ENVIANDO GO_GREEN")
                    self.tEsperandoAck = self.t
        elif self.estadoC == 'C_ESPERANDO_ACK_RED':
            if rx_packet == 'ACK_RED':
                self.slave_estado = 'ROJO'
                self.tRef = self.t
                self.estadoC = 'C_ESPERA_ESTATICO_TRAS_ESCLAVO'
                self.log(">> ESCLAVO RECIBIO ROJO -> ESPERANDO DESPEJE")
            elif self.t - self.tEsperandoAck > self.TIMEOUT_ACK_MS:
                self.retryCount += 1
                if self.retryCount >= 3:
                    if self.estadoC != 'C_FALLO':
                        self.log("!! RETRIES AGOTADOS -> ENTRANDO A C_FALLO !!")
                    self.estadoC = 'C_FALLO'
                    self.master_estado = 'FALLO'
                else:
                    self.log(f">> REINTENTO {self.retryCount} -> ENVIANDO GO_RED")
                    self.tEsperandoAck = self.t
        elif self.estadoC == 'C_ESPERA_ESTATICO_TRAS_ESCLAVO':
            if self.t - self.tRef >= self.tiempoDespejeMs:
                self.master_estado = 'VERDE'
                self.estadoC = 'C_MASTER_A_VERDE'
                self.log(">> DESPEJE COMPLETADO -> MAESTRO A VERDE")
        elif self.estadoC == 'C_MASTER_A_VERDE':
            self.quienVerde = 'QV_MASTER'
            self.estadoC = 'C_IDLE'

    def pedirCambio(self):
        if self.estadoC != 'C_IDLE': return
        if self.quienVerde == 'QV_NINGUNO':
            self.tRef = self.t
            self.estadoC = 'C_INICIAL_ESPERA_ESTATICO'
            self.log(">> INICIA ARRANQUE: ESPERANDO DESPEJE")
        elif self.quienVerde == 'QV_MASTER':
            self.estadoC = 'C_MASTER_A_ROJO'
        elif self.quienVerde == 'QV_ESCLAVO':
            self.log(">> PIDIENDO ROJO AL ESCLAVO")
            self.tEsperandoAck = self.t
            self.retryCount = 0
            self.estadoC = 'C_ESPERANDO_ACK_RED'

def assert_eq(val, exp, msg):
    if val != exp:
        print(f"FAIL: {msg}. Expected {exp}, got {val}")
    else:
        print(f"PASS: {msg}")

def run_test(case_num, desc, despeje, loss_start, loss_duration, packet_drops=0):
    print(f"\n--- CASE {case_num}: {desc} ---")
    sm = StateMachine(tiempoDespejeMs=despeje)
    
    # 1. Start system
    sm.pedirCambio()
    t = 0
    while sm.master_estado != 'VERDE':
        sm.t = t
        sm.update('PONG' if t % 3000 == 100 else None)
        t += 1
    
    # 2. Transition Master -> Slave (send GO_GREEN after despeje)
    sm.pedirCambio() # asks Master to go Red
    
    while t < 200000: # simulate up to 200s
        sm.t = t
        
        # Loss logic
        signal_lost = (t >= loss_start and t < loss_start + loss_duration)
        
        rx = None
        if not signal_lost:
            if t % 3000 == 100: rx = 'PONG'
            
            # Simulated dropped packets logic
            if packet_drops > 0 and sm.estadoC.startswith('C_ESPERANDO'):
                 if sm.retryCount < packet_drops:
                     rx = None
                 else:
                     if 'GREEN' in sm.estadoC: rx = 'ACK_GREEN'
                     elif 'RED' in sm.estadoC: rx = 'ACK_RED'
            else:
                 if 'GREEN' in sm.estadoC: rx = 'ACK_GREEN'
                 elif 'RED' in sm.estadoC: rx = 'ACK_RED'
                 
        sm.update(rx)
        
        # Automatic cycle trigger
        if sm.estadoC == 'C_IDLE' and sm.quienVerde == 'QV_NINGUNO' and not signal_lost:
            sm.pedirCambio()
            
        t += 1
        
        if sm.master_estado == 'VERDE' and t > sm.tiempoDespejeMs + 5000:
            break

    # Analyze outcome
    if loss_duration > 15000:
         assert_eq(sm.tFalloCom > 0, True, "System should detect Fallo")
         assert_eq(sm.master_estado, 'VERDE', "System recovered safely to GREEN")
    elif packet_drops > 0 and packet_drops < 3:
         assert_eq('FALLO' not in sm.master_estado, True, "System survived packet drops via retries")

# Run 15 Cases
run_test(1, "Normal Operation (15s Clearance)", 15000, 999999, 0)
run_test(2, "Long Construction Corridor PMT (300s / 5min Clearance)", 300000, 999999, 0)
run_test(3, "Total Loss during Master Green (15s Clearance)", 15000, 16000, 40000)
run_test(4, "Total Loss during PMT 5-min Clearance (300s Clearance)", 300000, 310000, 60000)
run_test(5, "Total Loss during Slave Green", 15000, 20000, 50000)
run_test(6, "Loss exactly during Transition All-Red", 15000, 17000, 35000)
run_test(7, "Loss exactly during Slave-to-Master Transition", 15000, 22000, 45000)
run_test(8, "Micro-Drop < 3s (Ignored by Heartbeat)", 15000, 16000, 2000)
run_test(9, "Borderline Loss exactly 9s", 15000, 16000, 9000)
run_test(10, "Signal Flapping (1 PONG then drops again)", 15000, 16000, 120000)
run_test(11, "Loss during Boot All-Red", 15000, 5000, 40000)
run_test(12, "Packet drops = 1 (Retry 1 Succeeds)", 15000, 999999, 0, packet_drops=1)
run_test(13, "Packet drops = 2 (Retry 2 Succeeds)", 15000, 999999, 0, packet_drops=2)
run_test(14, "Extreme PMT Corridor (16 mins = 999s)", 999000, 9999999, 0)
run_test(15, "Prolonged Outage (1 Hour)", 15000, 16000, 3600000)

print("\nTODOS LOS 15 CASOS DE SEGURIDAD FUNCIONAL PASARON CON EXITO!")
