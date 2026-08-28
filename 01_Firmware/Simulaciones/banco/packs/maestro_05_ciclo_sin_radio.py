# ===== banco/packs/maestro_05_ciclo_sin_radio.py =====
#
# EL CICLO QUE EL MAESTRO CORRE SIN RADIO
#
# Barrido de las 86.400 posiciones del dia: en Modo Degradado NUNCA se pasa de verde
# a verde sin todo-rojo por medio. Se barre el dia entero y no una muestra porque los
# fallos de aritmetica circular viven justo en los bordes que un muestreo se salta.

from banco.modelos.maestro import *          # noqa: F401,F403
from banco.modelos.maestro import (          # los guiones bajos no
    _codigo, _fuente, _main, _ruta,          # los exporta import *
)

# EJERCE SFTY-21: el ciclo por reloj: nunca verde a verde sin todo-rojo.

NOMBRE = "maestro_05_ciclo_sin_radio"
DESCRIPCION = "el ciclo que el Maestro corre sin radio"


def correr(b, fw):
    # Bloque traido LITERAL del validador monolitico, solo reindentado. Reescribir
    # logica ya probada para renombrar las llamadas es como se cuelan los errores en
    # una migracion que se supone que no cambia comportamiento.
    verificar = b.verificar
    titulo = b.titulo


    def fase(seg_dia, verde, despeje):
        """Port de ciclo_degradado_fase(). Se reproduce aqui, y no se importa del otro
        simulador, para que este banco sea autonomo y para poder atacarlo con el ciclo
        REAL del Maestro leido de modo_degradado.cpp."""
        if verde == 0 or despeje == 0:
            return "FD_DESPEJE_A"
        ciclo = 2 * (verde + despeje)
        if seg_dia < despeje:
            return "FD_DESPEJE_B"
        if SEGUNDOS_DEL_DIA - seg_dia <= despeje:
            return "FD_DESPEJE_B"
        pos = seg_dia % ciclo
        if pos < verde:
            return "FD_VERDE_MAESTRO"
        if pos < verde + despeje:
            return "FD_DESPEJE_A"
        if pos < 2 * verde + despeje:
            return "FD_VERDE_ESCLAVO"
        return "FD_DESPEJE_B"


    # --- 5.1 -------------------------------------------------------------------
    # Con el ciclo REAL del firmware, barrido de las 86400 posiciones del dia: nunca
    # se pasa de un verde a otro sin todo-rojo de por medio. Es la unica linea del
    # firmware que enciende un verde sin confirmacion del otro extremo.
    anterior, saltos = None, []
    for s in range(SEGUNDOS_DEL_DIA):
        f = fase(s, DEG_VERDE_SEG, DEG_DESPEJE_SEG)
        if anterior and anterior != f and "VERDE" in anterior and "VERDE" in f:
            saltos.append(s)
        anterior = f
    verificar(not saltos,
              f"Con el ciclo real del Maestro ({DEG_VERDE_SEG}/{DEG_DESPEJE_SEG} s) y las "
              f"{SEGUNDOS_DEL_DIA} posiciones del dia, JAMAS se pasa de verde a verde sin "
              "todo-rojo, medianoche incluida.",
              f"Transicion verde->verde sin despeje en los segundos {saltos[:5]}")

    # --- 5.2 -------------------------------------------------------------------
    # El despeje tiene que cubrir la deriva acumulada durante todo el plazo que el
    # limite duro autoriza. Es la cuenta que justifica los 30 s, rehecha con los
    # numeros de hoy en lugar de darla por buena.
    deriva_en_limite = DERIVA_PEOR_S_DIA * (LIMITE_DURO_MS / 86400000.0)
    dias_hasta_solape = DEG_DESPEJE_SEG / DERIVA_PEOR_S_DIA
    verificar(deriva_en_limite < DEG_DESPEJE_SEG and
              dias_hasta_solape > 1.5 * (LIMITE_DURO_H / 24.0),
              f"El despeje de {DEG_DESPEJE_SEG} s absorbe la deriva de "
              f"{deriva_en_limite:.1f} s que dos cristales acumulan en las {LIMITE_DURO_H} h "
              f"del limite duro (factor {DEG_DESPEJE_SEG/deriva_en_limite:.2f}). Los verdes "
              f"tardarian {dias_hasta_solape:.2f} dias en solaparse y el limite corta a los "
              f"{LIMITE_DURO_H/24:.0f}: factor {dias_hasta_solape/(LIMITE_DURO_H/24):.2f} "
              "sobre el plazo.",
              f"El despeje de {DEG_DESPEJE_SEG} s no cubre la deriva de "
              f"{deriva_en_limite:.1f} s del plazo autorizado: los verdes se solaparian a los "
              f"{dias_hasta_solape:.2f} dias y el limite duro no corta hasta los "
              f"{LIMITE_DURO_H/24:.0f}")

    # --- 5.3 -------------------------------------------------------------------
    # El aviso de proximidad al limite tiene que llegar ANTES del limite y con margen
    # para programar una visita, no cuando ya no sirve de nada.
    verificar(AVISO_LIMITE_MS < LIMITE_DURO_MS and
              (LIMITE_DURO_MS - AVISO_LIMITE_MS) >= 4 * 3600000,
              f"El aviso salta {(LIMITE_DURO_MS - AVISO_LIMITE_MS)/3600000:.0f} h antes del "
              "limite duro: tiempo de sobra para programar una visita.",
              f"El aviso salta solo {(LIMITE_DURO_MS - AVISO_LIMITE_MS)/3600000:.1f} h antes "
              "del limite (o despues): no da margen para reaccionar")

    # --- 5.4 -------------------------------------------------------------------
    # Coherencia del contrato de las dos puntas: el despeje que se guarda en la pila
    # y el que se envia por radio tienen que ser el MISMO simbolo. Si una punta
    # ampliara por su cuenta, las dos calcularian ciclos de distinta duracion sobre
    # la misma hora y los verdes se solaparian durante minutos.
    _pub = re.search(r"coordinador_enviarConfigCiclo\(\(uint8_t\)(\w+),\s*\(uint8_t\)(\w+)\)",
                     _fuente("Maestro", "src", "modo_degradado.cpp"))
    _guard = re.search(r"respaldo_guardarCiclo\(\(uint8_t\)(\w+),\s*\(uint8_t\)(\w+)\)",
                       _fuente("Maestro", "src", "modo_degradado.cpp"))
    _fase_llamada = re.search(r"ciclo_degradado_fase\(reloj_segundosDelDia\(\),\s*(\w+),\s*(\w+)\)",
                              _fuente("Maestro", "src", "modo_degradado.cpp"))
    mismos_simbolos = (_pub and _guard and _fase_llamada and
                       _pub.groups() == _guard.groups() == _fase_llamada.groups())
    verificar(mismos_simbolos,
              "El ciclo que se envia al Esclavo, el que se guarda en la pila y el que alimenta "
              f"el calculo de fase son EL MISMO simbolo ({_pub.group(1)}/{_pub.group(2)} si se "
              "leyo): no existe sitio donde alguien pueda volver a ampliar el despeje.",
              "El ciclo enviado, el guardado y el calculado NO salen de los mismos simbolos: "
              "las dos puntas podrian computar ciclos de distinta duracion")
