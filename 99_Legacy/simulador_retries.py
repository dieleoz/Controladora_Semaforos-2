class Simulador:
    def __init__(self):
        self.t = 0
        self.estadoC = 'C_ESPERA_ESTATICO_TRAS_MASTER'
        self.tRef = 0
        self.tiempoDespejeMs = 15000
        self.TIMEOUT_ACK_MS = 1500
        self.tEsperandoAck = 0
        self.retryCount = 0
        self.logs = []

    def log(self, msg):
        self.logs.append(f"[T={self.t}ms] {msg}")

    def loop(self):
        if self.estadoC == 'C_ESPERA_ESTATICO_TRAS_MASTER':
            if self.t - self.tRef >= self.tiempoDespejeMs:
                self.log(">> Enviando Paquete: GO_GREEN (INTENTO 1)")
                self.tEsperandoAck = self.t
                self.retryCount = 0
                self.estadoC = 'C_ESPERANDO_ACK_GREEN'
        elif self.estadoC == 'C_ESPERANDO_ACK_GREEN':
            # Simular perdida de paquetes (nunca llega ACK)
            if self.t - self.tEsperandoAck > self.TIMEOUT_ACK_MS:
                self.retryCount += 1
                if self.retryCount >= 3:
                    self.log(f"!! TIMEOUT FINAL TRAS {self.retryCount} REINTENTOS. Declarando C_FALLO.")
                    self.estadoC = 'C_FALLO'
                else:
                    self.log(f">> Enviando Paquete de Respaldo: GO_GREEN (INTENTO {self.retryCount + 1})")
                    self.tEsperandoAck = self.t

    def run(self):
        for i in range(25000):
            self.t = i
            self.loop()
            if self.estadoC == 'C_FALLO':
                break
        
        for l in self.logs:
            print(l)

sim = Simulador()
sim.run()
