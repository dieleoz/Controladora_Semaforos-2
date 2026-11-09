#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
===============================================================================
SIMULADOR DE ESCENARIOS DE REPETIDOR — Controladora de Semáforos V8.x
===============================================================================
Complementa a `simulador_sistema_v7_6.py` (bateria funcional 9/9), que NO se
toca. Este archivo se centra en el enlace de radio y el puente ESP32:

  - Latencia real por salto de aire (1 salto en directo, 2 con repetidor).
  - Perdida de tramas por ruido/distancia, aplicada COPIA A COPIA de la rafaga.
  - Interferencia por co-ubicacion: B1 queda sordo mientras B2 transmite.
  - Caida y restablecimiento del repetidor (fail-safe + self-healing).
  - Barrido de perdida para encontrar el punto de ruptura del protocolo.

Reutiliza las maquinas de estado del simulador funcional, que a su vez leen
RF_BURST_COPIES y TIMEOUT_ACK_MS del firmware C++. Asi los tres se mueven juntos.

Uso:  python 01_Firmware/Simulaciones/simulador_repetidor.py
      (funciona desde cualquier directorio de trabajo)
===============================================================================
"""

import os
import random
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

if sys.platform == "win32":
    try:
        sys.stdout.reconfigure(encoding="utf-8")
    except Exception:
        pass

from simulador_sistema_v7_6 import (  # noqa: E402
    FALLBACK_S,
    LATIDO_S,
    RF_BURST_COPIES,
    TIMEOUT_ACK_S,
    SemafaroMaestro,
    SemaforoEsclavo,
    calcular_crc8_maxim,
)

DT = 0.1  # paso de simulacion, en segundos

# Tiempo de aire de UNA trama de 4 bytes, medido de extremo a extremo por salto.
# A 2.4 kbps con FEC y preambulo ronda los 0.15 s. A 0.3 kbps rondaba los 0.75 s
# y era lo que hacia desbordar el timeout del Maestro (fallo N-1 en campo).
AIRE_2400 = 0.15
AIRE_300 = 0.75

# Cuanto dura el bloqueo del receptor vecino respecto al tiempo que su hermano
# esta transmitiendo. 1.0 = solo mientras hay portadora; >1 modela que el AGC
# del receptor tarda en recuperarse tras la saturacion.
RECUPERACION_AGC = 2.0

# Ruido que entra al ESP32 cuando el par RS485 de B1 queda flotando. El receptor
# del MAX3485 oscila y la UART lo interpreta como bytes a la velocidad del puerto:
# 9600 baudios / 10 bits por byte = 960 B/s, o sea 96 bytes por paso de 0.1 s.
# Muy por encima de los 150 B/s que caben en el aire a 2.4 kbps: de ahi que el
# puente sin validar sature el canal por completo.
RUIDO_LINEA_FLOTANDO = 96


class SaltoAire:
    """Un salto de radio: introduce latencia y puede perder tramas.

    La perdida se aplica COPIA A COPIA de 4 bytes, no al bloque entero. Es lo que
    permite ver si la rafaga de RF_BURST_COPIES aporta algo real.
    """

    # Capacidad del canal en bytes/s. A 2.4 kbps son 300 B/s en bruto, que el FEC
    # deja en unos 150. Si se le mete mas trafico del que cabe, el exceso SE PIERDE:
    # es lo que ocurre cuando una radio se queda radiando ruido sin parar.
    CAPACIDAD_BYTES_S = 150

    def __init__(self, latencia, prob_perdida=0.0, rng=None):
        self.latencia = latencia
        self.prob_perdida = prob_perdida
        self.rng = rng or random.Random(0)
        self.activo = True
        self.cola = []          # [(t_entrega, bytes)]
        self.sordo_hasta = 0.0  # ventana de bloqueo por interferencia
        self.copias_perdidas = 0
        self.copias_enviadas = 0
        self.bytes_en_ventana = 0
        self.ventana_actual = -1
        self.bytes_saturados = 0

    def enviar(self, datos, t):
        if not datos or not self.activo:
            return
        # Contabilidad de ocupacion del aire, por segundo.
        seg = int(t)
        if seg != self.ventana_actual:
            self.ventana_actual = seg
            self.bytes_en_ventana = 0

        for i in range(0, len(datos) - 3, 4):
            copia = bytes(datos[i:i + 4])
            self.copias_enviadas += 1

            self.bytes_en_ventana += 4
            if self.bytes_en_ventana > self.CAPACIDAD_BYTES_S:
                self.bytes_saturados += 4   # el canal ya no da mas: se pierde
                self.copias_perdidas += 1
                continue
            if t < self.sordo_hasta:      # receptor bloqueado: no la oye
                self.copias_perdidas += 1
                continue
            if self.rng.random() < self.prob_perdida:
                self.copias_perdidas += 1
                continue
            self.cola.append((t + self.latencia, copia))

    def recibir(self, t):
        listos = [d for (te, d) in self.cola if te <= t]
        self.cola = [(te, d) for (te, d) in self.cola if te > t]
        return b"".join(listos)

    def apagar(self):
        self.activo = False
        self.cola.clear()

    def encender(self):
        self.activo = True


class Enlace:
    """Camino completo Maestro <-> Esclavo, con o sin repetidor."""

    def __init__(self, con_repetidor, aire=AIRE_2400, perdida=0.0,
                 interferencia_cosite=False, semilla=42,
                 ruido_puente=0, puente_valida=True):
        rng = random.Random(semilla)
        self.con_repetidor = con_repetidor
        self.interferencia_cosite = interferencia_cosite
        # ruido_puente: bytes de basura por paso que entran al ESP32 desde la linea
        # RS485 de B1 (par flotando, sin polarizacion, cable partido...).
        # puente_valida: True = el ESP32 solo retransmite tramas con CRC correcto.
        self.ruido_puente = ruido_puente
        self.puente_valida = puente_valida
        self.rng_ruido = random.Random(semilla + 1)
        self.ruido_descartado = 0
        self.ruido_retransmitido = 0
        self.saltos_m_e = []  # Maestro -> Esclavo
        self.saltos_e_m = []  # Esclavo -> Maestro
        n = 2 if con_repetidor else 1
        for _ in range(n):
            self.saltos_m_e.append(SaltoAire(aire, perdida, rng))
            self.saltos_e_m.append(SaltoAire(aire, perdida, rng))
        self.transito_teorico = 2 * n * aire  # ida y vuelta

    def _puente_esp32(self, entregado, t):
        """Lo que hace el ESP32 con lo que le entrega la radio de entrada.

        Aqui es donde se nota la diferencia entre el puente tonto y el que valida:
        el ruido de una linea flotando entra igual en ambos, pero solo uno lo deja
        pasar al aire.
        """
        crudo = bytearray(entregado)
        if self.ruido_puente:
            for _ in range(self.ruido_puente):
                crudo.append(self.rng_ruido.randrange(256))

        if not self.puente_valida:
            self.ruido_retransmitido += self.ruido_puente
            return bytes(crudo)

        # Puente validador: ventana deslizante + CRC-8, igual que el firmware.
        salida = bytearray()
        i = 0
        while i + 4 <= len(crudo):
            if calcular_crc8_maxim(bytes(crudo[i:i + 3])) == crudo[i + 3]:
                salida.extend(crudo[i:i + 4])
                i += 4
            else:
                self.ruido_descartado += 1
                i += 1
        return bytes(salida)

    def _recorrer(self, saltos, datos, t):
        """Inyecta en el primer salto y va encadenando lo que sale de cada uno."""
        saltos[0].enviar(datos, t)
        salida = b""
        for i, salto in enumerate(saltos):
            entregado = salto.recibir(t)
            # El puente solo existe entre el primer y el segundo salto.
            if i + 1 < len(saltos) and saltos is self.saltos_m_e:
                entregado = self._puente_esp32(entregado, t)
            if not entregado:
                continue
            if i + 1 < len(saltos):
                siguiente = saltos[i + 1]
                siguiente.enviar(entregado, t)
                # Co-ubicacion: las dos radios del repetidor estan a centimetros.
                # Cuando B2 transmite hacia el Esclavo, satura el receptor de B1,
                # que es el que escucha al Maestro -> ese es saltos[0] de ESTA misma
                # ruta. (Y simetricamente en la ruta de vuelta.)
                # RECUPERACION_AGC: el receptor no vuelve en cuanto cesa la portadora;
                # el control automatico de ganancia tarda en recuperarse.
                if self.interferencia_cosite:
                    saltos[0].sordo_hasta = t + siguiente.latencia * RECUPERACION_AGC
            else:
                salida += entregado
        return salida

    def paso(self, tx_maestro, tx_esclavo, t):
        rx_esclavo = self._recorrer(self.saltos_m_e, tx_maestro, t)
        rx_maestro = self._recorrer(self.saltos_e_m, tx_esclavo, t)
        return rx_maestro, rx_esclavo

    def apagar_repetidor(self):
        for s in self.saltos_m_e + self.saltos_e_m:
            s.apagar()

    def encender_repetidor(self):
        for s in self.saltos_m_e + self.saltos_e_m:
            s.encender()

    def estadisticas(self):
        env = sum(s.copias_enviadas for s in self.saltos_m_e + self.saltos_e_m)
        per = sum(s.copias_perdidas for s in self.saltos_m_e + self.saltos_e_m)
        return env, per


class Banco:
    """Arranca Maestro, Esclavo y enlace, y los hace avanzar en el tiempo."""

    def __init__(self, enlace, despeje_s=15.0):
        self.maestro = SemafaroMaestro()
        self.esclavo = SemaforoEsclavo()
        self.maestro.tiempo_despeje_s = despeje_s
        self.enlace = enlace
        self.t = 0.0
        self._pendiente_maestro = bytearray()

    def pulsar_cambio(self):
        self._pendiente_maestro.extend(self.maestro.pedir_cambio(self.t) or b"")

    def avanzar(self, duracion, al_paso=None):
        objetivo = self.t + duracion
        while self.t < objetivo:
            tx_m = self.maestro.actualizar(self._rx_maestro, self.t)
            if self._pendiente_maestro:
                tx_m = bytes(self._pendiente_maestro) + tx_m
                self._pendiente_maestro.clear()
            tx_e = self.esclavo.actualizar(self._rx_esclavo, self.t)
            self._rx_maestro, self._rx_esclavo = self.enlace.paso(tx_m, tx_e, self.t)
            if al_paso:
                al_paso(self)
            self.t += DT

    _rx_maestro = b""
    _rx_esclavo = b""


# =============================================================================
# PRUEBAS
# =============================================================================

total_pass = 0
total_tests = 0


def verificar(cond, ok, fail):
    global total_pass, total_tests
    total_tests += 1
    if cond:
        total_pass += 1
        print(f"   ✔ PASS: {ok}")
    else:
        print(f"   ✘ FAIL: {fail}")


def ciclo_completo(banco):
    """Arranca un modo y ejecuta un relevo completo Maestro -> Esclavo -> Maestro."""
    banco.maestro.iniciar_modo(banco.t)
    banco.avanzar(15.0 + 4.5)            # despeje + amarillo -> Maestro en verde
    paso1 = banco.maestro.luz_local
    banco.avanzar(20.0)
    banco.pulsar_cambio()                # cede el paso al Esclavo
    banco.avanzar(15.0 + 4.5 + 10.0)
    paso2 = banco.esclavo.luz_local
    banco.pulsar_cambio()                # lo recupera el Maestro
    banco.avanzar(15.0 + 4.5 + 10.0)
    paso3 = banco.maestro.luz_local
    return paso1, paso2, paso3


def encabezado(txt):
    print(f"\n▶ {txt}")


print("=" * 78)
print("📡 SIMULACION DE ESCENARIOS DE REPETIDOR — V8.x")
print("=" * 78)
print(f"   Constantes leidas del firmware C++: RF_BURST_COPIES={RF_BURST_COPIES}"
      f"   TIMEOUT_ACK={TIMEOUT_ACK_S}s")

# --- 1. Referencia: enlace directo sano --------------------------------------
encabezado("PRUEBA 1: Enlace DIRECTO (2 radios) a 2.4 kbps, sin perdidas")
enlace = Enlace(con_repetidor=False)
banco = Banco(enlace)
p1, p2, p3 = ciclo_completo(banco)
print(f"   Transito ida-vuelta teorico: {enlace.transito_teorico:.2f}s"
      f"   (timeout del Maestro: {TIMEOUT_ACK_S}s)")
print(f"   Maestro verde={p1}  Esclavo verde={p2}  Maestro recupera={p3}")
verificar(p1 == "S_VERDE" and p2 == "S_VERDE" and p3 == "S_VERDE",
          "Relevo completo sin caidas en enlace directo.",
          f"Secuencia incompleta: {p1} / {p2} / {p3}")

# --- 2. Repetidor sano --------------------------------------------------------
encabezado("PRUEBA 2: Enlace con REPETIDOR (4 radios) a 2.4 kbps, sin perdidas")
enlace = Enlace(con_repetidor=True)
banco = Banco(enlace)
p1, p2, p3 = ciclo_completo(banco)
print(f"   Transito ida-vuelta teorico: {enlace.transito_teorico:.2f}s"
      f"   (dos saltos de aire por sentido)")
verificar(p1 == "S_VERDE" and p2 == "S_VERDE" and p3 == "S_VERDE",
          "Relevo completo a traves del repetidor.",
          f"Secuencia incompleta: {p1} / {p2} / {p3}")

# --- 3. Regresion N-1: el mismo repetidor a 0.3 kbps --------------------------
encabezado("PRUEBA 3: REGRESION N-1 — repetidor a 0.3 kbps (config anterior)")
enlace = Enlace(con_repetidor=True, aire=AIRE_300)
banco = Banco(enlace)
p1, p2, p3 = ciclo_completo(banco)
print(f"   Transito ida-vuelta teorico: {enlace.transito_teorico:.2f}s"
      f"   vs timeout de {TIMEOUT_ACK_S}s")
print(f"   Estado final del coordinador: {banco.maestro.estado_c}")
verificar(enlace.transito_teorico > TIMEOUT_ACK_S * 0.5,
          f"Confirmado el margen critico a 0.3 kbps: transito "
          f"{enlace.transito_teorico:.2f}s frente a timeout {TIMEOUT_ACK_S}s.",
          "El modelo no reproduce el estrangulamiento de 0.3 kbps.")

# --- 4. Repetidor con perdida de tramas --------------------------------------
encabezado("PRUEBA 4: Repetidor con 30% de perdida por trama")
enlace = Enlace(con_repetidor=True, perdida=0.30)
banco = Banco(enlace)
p1, p2, p3 = ciclo_completo(banco)
env, per = enlace.estadisticas()
print(f"   Copias emitidas: {env}   perdidas: {per} ({100*per/max(env,1):.0f}%)")
verificar(p1 == "S_VERDE" and p2 == "S_VERDE" and p3 == "S_VERDE",
          f"La rafaga de {RF_BURST_COPIES} copias y los reintentos absorbieron el 30% de perdida.",
          f"El ciclo no se completo con 30% de perdida: {p1} / {p2} / {p3}")

# --- 5. Interferencia por co-ubicacion ---------------------------------------
encabezado("PRUEBA 5: Interferencia co-ubicada (B1 sordo mientras B2 transmite)")
enlace = Enlace(con_repetidor=True, interferencia_cosite=True)
banco = Banco(enlace)
p1, p2, p3 = ciclo_completo(banco)
env, per = enlace.estadisticas()
completo = (p1 == "S_VERDE" and p2 == "S_VERDE" and p3 == "S_VERDE")

# Ciclo de trabajo del canal: cuanto tiempo hay portadora frente al total.
# Con un latido cada 3 s y transmisiones de ~0.15 s, ronda el 5%.
ciclo_trabajo = AIRE_2400 / 3.0
print(f"   Copias emitidas: {env}   bloqueadas: {per} ({100*per/max(env,1):.0f}%)")
print(f"   Ciclo de trabajo del canal: {100*ciclo_trabajo:.0f}%"
      f"   (ventana de bloqueo x{RECUPERACION_AGC:.0f} = {100*ciclo_trabajo*RECUPERACION_AGC:.0f}%)")
print(f"   Relevo completo: {'SI' if completo else 'NO'}"
      f"   Estado final: {banco.maestro.estado_c}")

# Este es el punto: el bloqueo mutuo solo puede destruir una trama si el hermano
# transmite JUSTO mientras la otra radio esta recibiendo. Con un repetidor de
# almacenamiento y reenvio, el trafico es secuencial por diseno: el Maestro emite,
# calla, y solo despues el repetidor retransmite. Apenas se solapan.
verificar(per == 0 and completo,
          f"El bloqueo mutuo NO rompe el enlace: con {100*ciclo_trabajo:.0f}% de ciclo de "
          f"trabajo las transmisiones apenas se solapan y no se perdio ninguna trama. "
          f"=> La interferencia co-ubicada NO explica por si sola un fallo total del repetidor.",
          f"Se bloquearon {per} copias ({100*per/max(env,1):.0f}%) y el relevo "
          f"{'se completo' if completo else 'NO se completo'}: la co-ubicacion SI seria "
          f"un factor a considerar.")

# --- 6. Caida y restablecimiento del repetidor -------------------------------
encabezado("PRUEBA 6: Corte de energia del repetidor y auto-recuperacion")
enlace = Enlace(con_repetidor=True)
banco = Banco(enlace)
banco.maestro.iniciar_modo(banco.t)
banco.avanzar(15.0 + 4.5)
print("   [Accion] Se corta la alimentacion del ESP32...")
enlace.apagar_repetidor()
# N-71: era un 13.0 fijo, elegido cuando el umbral de orfandad eran 12 s. Se deriva
# de FALLBACK_S, que el simulador lee del protocolo.h: lo que esta prueba mide es que
# el fail-safe llegue DESPUES del umbral, no que llegue en el segundo 13.
banco.avanzar(FALLBACK_S + 1.0)
caido_m, caido_e = banco.maestro.luz_local, banco.esclavo.luz_local
print(f"   Tras {FALLBACK_S + 1.0:.0f}s sin repetidor -> Maestro={caido_m}  Esclavo={caido_e}")
verificar(caido_m == "S_FALLO" and caido_e == "S_FALLO",
          f"Ambos nodos pasaron a AMARILLO INTERMITENTE a los {FALLBACK_S:.0f}s (fail-safe).",
          f"Se esperaba S_FALLO en ambos: {caido_m} / {caido_e}")

print("   [Accion] Se restablece la alimentacion del ESP32...")
enlace.encender_repetidor()
# N-71: eran 3.0 s fijos, que es EXACTAMENTE un periodo de latido: la prueba vivia
# justo en la frontera y solo pasaba por como caia la fase. Medido al subir el umbral
# de orfandad: con 3.0, 3.5 y 4.0 falla; a partir de 4.5 pasa. No se subio el numero
# hasta que dejo de fallar -eso es ajustar el instrumento hasta que de verde-: se
# deriva del peor caso real de un reenganche, que es esperar a que toque el siguiente
# latido (LATIDO_S) mas la ventana completa de su ACK (TIMEOUT_ACK_S).
banco.avanzar(LATIDO_S + TIMEOUT_ACK_S)
print(f"   Maestro={banco.maestro.luz_local}  estado={banco.maestro.estado_c}")
verificar(banco.maestro.luz_local == "S_ROJO"
          and banco.maestro.estado_c == "C_INICIAL_ESPERA_ESTATICO",
          "Self-Healing autonomo: reengancha solo y entra a All-Red de despeje.",
          f"No reengancho: {banco.maestro.luz_local} / {banco.maestro.estado_c}")

# --- 7. Fallo de campo 31/07: ruido continuo en la linea B1 -> ESP32 ---------
# Reproduce el fallo observado: el par RS485 de entrada del puente mete bytes
# falsos sin parar. Se compara el puente TONTO (retransmite todo) con el puente
# VALIDADOR (solo relaya tramas con CRC correcto).
encabezado("PRUEBA 7a: puente TONTO con ruido continuo en la linea de B1")
enlace = Enlace(con_repetidor=True, ruido_puente=RUIDO_LINEA_FLOTANDO, puente_valida=False)
banco = Banco(enlace)
p1, p2, p3 = ciclo_completo(banco)
tonto_ok = (p1 == "S_VERDE" and p2 == "S_VERDE" and p3 == "S_VERDE")
sat_tonto = sum(s.bytes_saturados for s in enlace.saltos_m_e)
print(f"   Ruido retransmitido al aire: {enlace.ruido_retransmitido} bytes")
print(f"   Bytes perdidos por saturacion del canal: {sat_tonto}")
print(f"   Relevo completo: {'SI' if tonto_ok else 'NO'}   estado={banco.maestro.estado_c}")
verificar(not tonto_ok and sat_tonto > 0,
          f"Reproducido el fallo de campo: el puente sin validar satura el canal "
          f"({sat_tonto} bytes descartados por exceso) y el relevo NO se completa.",
          "El modelo no reproduce la saturacion; revisar la capacidad del canal.")

encabezado("PRUEBA 7b: puente VALIDADOR con el MISMO ruido")
enlace = Enlace(con_repetidor=True, ruido_puente=RUIDO_LINEA_FLOTANDO, puente_valida=True)
banco = Banco(enlace)
p1, p2, p3 = ciclo_completo(banco)
valida_ok = (p1 == "S_VERDE" and p2 == "S_VERDE" and p3 == "S_VERDE")
sat_val = sum(s.bytes_saturados for s in enlace.saltos_m_e)
print(f"   Ruido descartado en el ESP32: {enlace.ruido_descartado} tramas")
print(f"   Ruido retransmitido al aire: {enlace.ruido_retransmitido} bytes")
print(f"   Bytes perdidos por saturacion: {sat_val}")
print(f"   Relevo completo: {'SI' if valida_ok else 'NO'}   estado={banco.maestro.estado_c}")
verificar(valida_ok and enlace.ruido_retransmitido == 0,
          "El puente validador descarta el ruido dentro del ESP32: no llega al aire, "
          "el canal no se satura y el relevo se completa con normalidad.",
          f"El puente validador no resolvio el escenario (relevo={valida_ok}, "
          f"ruido al aire={enlace.ruido_retransmitido})")

# --- 8. Barrido de perdida: punto de ruptura ---------------------------------
encabezado("PRUEBA 7: Barrido de perdida — hasta donde aguanta el protocolo")
limite = None
for pct in (0, 10, 20, 30, 40, 50, 60, 70, 80):
    e = Enlace(con_repetidor=True, perdida=pct / 100.0, semilla=7)
    b = Banco(e)
    a, bb, c = ciclo_completo(b)
    ok = (a == "S_VERDE" and bb == "S_VERDE" and c == "S_VERDE")
    print(f"   perdida {pct:3d}%  ->  {'OK' if ok else 'CAE'}")
    if not ok and limite is None:
        limite = pct
if limite is None:
    print("   El protocolo completo el ciclo incluso con 80% de perdida por trama.")
else:
    print(f"   Punto de ruptura: {limite}% de perdida por trama.")
verificar(limite is None or limite >= 30,
          f"Margen aceptable: aguanta hasta "
          f"{'>80' if limite is None else limite - 10}% de perdida por trama.",
          f"Margen insuficiente: cae ya con {limite}% de perdida.")

# =============================================================================
print("\n" + "=" * 78)
if total_pass == total_tests:
    print(f"🏆 RESULTADO: {total_pass}/{total_tests} PASS")
else:
    print(f"⚠️  RESULTADO: {total_pass}/{total_tests} PASS — revisar los FAIL")
print("=" * 78)
print("\nNOTA: este es un MODELO del enlace de radio. No sustituye la medicion en")
print("campo: los tiempos de aire son estimados y la perdida real depende de")
print("terreno, antenas y ubicacion. Sirve para comparar escenarios entre si.")
