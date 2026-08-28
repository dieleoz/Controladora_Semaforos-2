# ===== 01_Firmware/Simulaciones/banco/contador.py =====
#
# EL VEREDICTO — una sola definicion de que cuenta como comprobacion.
#
# Los tres validadores llevaban su propio contador, y no contaban igual: el de
# costura suma los hallazgos confirmados como comprobaciones ejecutadas, el del
# Maestro no. Por eso "37/41" y "60/67" no significan lo mismo, y nadie podia
# sumarlos sin equivocarse. Aqui la regla se escribe una vez.
#
# LOS TRES ESTADOS, QUE NO SE MEZCLAN NUNCA:
#
#   PASS      corrio y la propiedad se cumple.
#   FALLA     corrio y la propiedad NO se cumple. Hay que arreglar el firmware.
#   ABORTADO  NO PUDO correr. No dice NADA del firmware.
#
# Codigos de salida: 0 PASS, 1 FALLA, 2 ABORTADO. Es la convencion que ya usaba
# compuerta.py y no se cambia.

import sys


class Banco:
    """Contador de un pack. Uno por pack, para que cada uno rinda cuentas solo."""

    def __init__(self, nombre):
        self.nombre = nombre
        self.total = 0
        self.pasadas = 0
        self.fallos = []       # (mensaje,)
        self.rotas = []        # propiedades de seguridad que el banco logro romper
        self.hallazgos = []    # (titulo, detalle)
        self.abortado = None   # motivo, si no pudo medir

    # -- comprobaciones -----------------------------------------------------

    def verificar(self, condicion, msg_pass, msg_fail):
        """Comprobacion normal: se espera que la propiedad se cumpla."""
        self.total += 1
        if condicion:
            self.pasadas += 1
            print(f"   OK    {msg_pass}")
        else:
            self.fallos.append(msg_fail)
            print(f"   FALLA {msg_fail}")
        return bool(condicion)

    def control_negativo(self, detecta, que):
        """Exige que la comprobacion SEPA FALLAR.

        Una prueba que aprueba todo no comprueba nada. Cada propiedad importante
        lleva su version defectuosa al lado para demostrar que el detector detecta;
        sin esto, el dia que las rutas dejaran de resolver la prueba compararia nada
        contra nada y saldria verde."""
        return self.verificar(
            detecta,
            f"control negativo: {que} — la prueba sabe distinguir el caso malo",
            f"CONTROL NEGATIVO ROTO: {que}. La comprobacion aprueba incluso el caso "
            f"defectuoso, asi que su PASS no vale nada")

    def propiedad(self, condicion, msg_ok, msg_roto):
        """Propiedad de seguridad que el banco INTENTA ROMPER — y a veces rompe.

        Se marca ROTA y no FALLA a proposito, porque no significan lo mismo. FALLA se
        lee como "el banco esta mal"; ROTA dice lo que de verdad ocurre: el escenario
        existe, se reprodujo, y el firmware no lo resiste. El banco no lo arregla -no
        es su trabajo- pero tampoco lo disimula dando PASS."""
        self.total += 1
        if condicion:
            self.pasadas += 1
            print(f"   OK    {msg_ok}")
        else:
            self.rotas.append(msg_roto)
            print(f"   ROTA  {msg_roto}")
        return bool(condicion)

    def reportar(self, titulo, detalle):
        """Anota un hallazgo SIN contarlo como comprobacion.

        Se distingue de hallazgo() a proposito, y la diferencia no es cosmetica: era
        justo el motivo de que "37/41" y "30/31" no se pudieran sumar. Aqui el
        hallazgo acompana a una propiedad() que ya cuenta por su cuenta; contarlo otra
        vez inflaria el total con la misma comprobacion dos veces."""
        self.hallazgos.append((titulo, detalle))
        print(f"   >>>   HALLAZGO: {titulo}")
        for l in detalle:
            print(f"         {l}")

    def hallazgo(self, reproducido, titulo, detalle):
        """Desajuste CONFIRMADO reproduciendolo.

        Cuenta como comprobacion ejecutada Y como hallazgo: las dos cosas son
        ciertas -la prueba corrio bien, y encontro algo-. Si NO se reproduce, el
        que esta mal es el modelo, y eso es un FALLA del instrumento."""
        self.total += 1
        if reproducido:
            self.pasadas += 1
            self.hallazgos.append((titulo, detalle))
            print(f"   OK    HALLAZGO CONFIRMADO -> {titulo}")
            for l in detalle:
                print(f"         {l}")
        else:
            self.fallos.append(f"el modelo no reproduce {titulo!r}: revisar el MODELO")
            print(f"   FALLA el modelo no reproduce {titulo!r}: revisar el modelo")

    def titulo(self, t):
        print(f"\n-- {t} " + "-" * max(0, 72 - len(t)))

    # -- veredicto ----------------------------------------------------------

    @property
    def estado(self):
        if self.abortado:
            return "ABORTADO"
        # Una propiedad ROTA es un fallo del firmware, no del banco: cierra la
        # compuerta igual que un FALLA. Se distingue en el texto, no en el veredicto.
        return "FALLA" if (self.fallos or self.rotas) else "PASS"

    @property
    def resumen(self):
        if self.abortado:
            return f"ABORTADO: {self.abortado}"
        return f"{self.pasadas}/{self.total}"


def salir(estado):
    sys.exit({"PASS": 0, "FALLA": 1, "ABORTADO": 2}[estado])
