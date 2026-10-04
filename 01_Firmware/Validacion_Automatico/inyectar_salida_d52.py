"""Controles negativos de J (SPEC_2 7.quater (i), CLAUDE.md 6): cada defecto se inyecta en el .cpp REAL de la punta,
se corre compilar_degradado.ps1 y se restaura por HASH. Los TRES tienen que dar FALLA (codigo 1 y una linea J rota);
el cuarto de la SPEC (salidaS sin leer en la reanudacion) es de BANCO: el doble de flash del arnes no sobrevive al
corte. Los patrones salen de los nombres de la SPEC; si uno no casa EXACTAMENTE las veces esperadas, es ABORTADO (2).
Uso: python inyectar_salida_d52.py   (0 los tres dan FALLA - 1 alguno sigue verde - 2 no pudo correr)."""
import hashlib
import os
import re
import subprocess
import sys

AQUI = os.path.dirname(os.path.abspath(__file__))
MAESTRO = os.path.join(AQUI, "..", "Maestro", "src", "modo_degradado.cpp")


def sha(p):
    return hashlib.sha256(open(p, "rb").read()).hexdigest()


def cuerpo(txt, nombre):
    """(ini, fin) del cuerpo de la funcion `nombre` (de su '{' a la '}' que la cierra)."""
    m = re.search(r"\b" + re.escape(nombre) + r"\s*\([^)]*\)\s*\{", txt)
    if not m:
        raise SystemExit("ABORTADO: no existe la funcion %s" % nombre)
    n, i = 1, m.end()
    while n and i < len(txt):
        n += {"{": 1, "}": -1}.get(txt[i], 0)
        i += 1
    return m.end(), i


def en_funcion(nombre, patron, nuevo, veces=1):
    def f(txt):
        a, b = cuerpo(txt, nombre)
        t, k = re.subn(patron, nuevo, txt[a:b])
        if k != veces:
            raise SystemExit("ABORTADO: %s: '%s' casa %d veces (esperadas %d)" % (nombre, patron, k, veces))
        return txt[:a] + t + txt[b:]
    return f


def global_(patron, nuevo, veces):
    def f(txt):
        t, k = re.subn(patron, nuevo, txt)
        if k != veces:
            raise SystemExit("ABORTADO: '%s' casa %d veces (esperadas %d)" % (patron, k, veces))
        return t
    return f


DEFECTOS = [
    ("sin la baja del permiso en forzarRojo()",
     en_funcion("modo_degradado_forzarRojo", r"respaldo_guardarDegradado\(false\)\s*;", ";")),
    ("sin el disparo de la salida programada",
     global_(r"reloj_segundosDesde2000\(\)\s*>=\s*salidaS", "false", 1)),
    # salidaS en la reanudacion tras un corte: el doble de flash del arnes no sobrevive al corte;
    # ese control es de banco con dos tarjetas (SPEC_2 7.quater (i)).
    ("salidaS sin poner a cero al salir (las asignaciones salidaS = 0 fuera de la declaracion)",
     global_(r"(?<![\w ])\s*salidaS\s*=\s*0\s*;", ";", 1)),
]


def main():
    h = sha(MAESTRO)
    original = open(MAESTRO, "rb").read()
    sin_cabecera = subprocess.run(["powershell", "-NoProfile", "-Command",
                                   "Select-String -Path '%s' -Pattern modo_degradado_programarSalida -Quiet"
                                   % os.path.join(AQUI, "..", "Maestro", "include", "modo_degradado.h")],
                                  capture_output=True, text=True).stdout.strip() != "True"
    if sin_cabecera:
        print("ABORTADO: D-52 sin construir (modo_degradado.h no declara programarSalida)")
        return 2
    verdes = []
    try:
        for que, mutar in DEFECTOS:
            open(MAESTRO, "w", encoding="utf-8", newline="").write(mutar(original.decode("utf-8")))
            p = subprocess.run(["powershell", "-NoProfile", "-ExecutionPolicy", "Bypass", "-File",
                                os.path.join(AQUI, "compilar_degradado.ps1")], capture_output=True, text=True,
                               errors="replace")
            rotas = [l for l in p.stdout.splitlines() if l.startswith("   [FALLA] J")]
            print("%-80s -> codigo %d, J rotas: %d" % (que, p.returncode, len(rotas)))
            if p.returncode == 0 or not rotas:
                verdes.append(que)
            if p.returncode not in (0, 1):
                print("ABORTADO: no compilo con el defecto: " + (p.stdout + p.stderr)[-200:])
                return 2
    finally:
        open(MAESTRO, "wb").write(original)
        if sha(MAESTRO) != h:
            print("ABORTADO: la restauracion por hash FALLO: reponer con git checkout")
            return 2
    print("restaurado por hash OK; defectos que siguen en verde: %s" % (verdes or "ninguno"))
    return 1 if verdes else 0


if __name__ == "__main__":
    sys.exit(main())
