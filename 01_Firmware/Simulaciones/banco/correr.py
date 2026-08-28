#!/usr/bin/env python3
# ===== 01_Firmware/Simulaciones/banco/correr.py =====
#
# CORREDOR DE PACKS.
#
# POR QUE SE PARTE EL BANCO EN PACKS.
#
# Los instrumentos crecieron hasta pesar lo mismo que el firmware que miden: 8.898
# lineas de banco para 8.895 de firmware, uno a uno. Y no son pruebas: son una
# SEGUNDA COPIA del firmware escrita en Python, que alguien tiene que mantener
# sincronizada a mano.
#
# Sincronizar a mano no escala, y ya fallo tres veces en una semana: N-36 (el
# validador leia un fichero que ya no existia), N-39 (el arnes mide ncenB10 cuando
# el codigo dibuja en 7x14B) y la propia compuerta, que daba FALLA de un arnes que
# ni arrancaba. Siempre el mismo fallo: la copia se queda atras.
#
# Un fichero de 2.106 lineas obliga a leerlo entero para cambiar una prueba. Un
# pack de 150 se lee de una sentada, se corre solo en un segundo, y se puede
# revisar -o reescribir- sin tocar los otros diecinueve.
#
# USO:
#   python banco/correr.py                    todos los packs
#   python banco/correr.py --pack costura_01  uno solo, por prefijo
#   python banco/correr.py --listar           que packs hay
#
# Codigos de salida: 0 PASS, 1 FALLA, 2 ABORTADO. ABORTADO NO ES PASS.

import argparse
import importlib
import os
import pkgutil
import sys

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from banco import fuente as fw                      # noqa: E402
from banco.contador import Banco, salir             # noqa: E402

try:
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")
except Exception:
    pass


def descubrir():
    import banco.packs
    encontrados = []
    for m in pkgutil.iter_modules(banco.packs.__path__):
        if not m.name.startswith("_"):
            encontrados.append(m.name)
    return sorted(encontrados)


def main():
    ap = argparse.ArgumentParser(description="Banco de validacion por packs")
    ap.add_argument("--pack", help="corre solo los packs cuyo nombre empiece asi")
    ap.add_argument("--listar", action="store_true")
    args = ap.parse_args()

    nombres = descubrir()
    if args.listar:
        for n in nombres:
            mod = importlib.import_module(f"banco.packs.{n}")
            print(f"  {n:<34} {getattr(mod, 'DESCRIPCION', '')}")
        return 0

    if args.pack:
        nombres = [n for n in nombres if n.startswith(args.pack)]
        if not nombres:
            print(f"[ABORTADO] ningun pack empieza por {args.pack!r}")
            return 2

    print("=" * 78)
    print(f" BANCO POR PACKS — {len(nombres)} pack(s)")
    print("=" * 78)

    bancos = []
    for n in nombres:
        b = Banco(n)
        bancos.append(b)
        print(f"\n### {n}")
        try:
            mod = importlib.import_module(f"banco.packs.{n}")
            mod.correr(b, fw)
        except fw.Abortado as e:
            # Un pack que no puede medir NO se lleva por delante a los demas: se
            # marca ABORTADO y el corredor sigue. Con sys.exit(), un pack roto
            # ocultaba el resultado de todos los siguientes.
            b.abortado = str(e)
            print(f"   ABORT {e}")
        except Exception as e:  # noqa: BLE001
            b.abortado = f"excepcion en el pack: {type(e).__name__}: {e}"
            print(f"   ABORT {b.abortado}")

    print("\n" + "=" * 78)
    n_pass = sum(1 for b in bancos if b.estado == "PASS")
    n_falla = sum(1 for b in bancos if b.estado == "FALLA")
    n_abort = sum(1 for b in bancos if b.estado == "ABORTADO")
    comprob = sum(b.total for b in bancos)
    ok = sum(b.pasadas for b in bancos)
    print(f" RESUMEN: {ok}/{comprob} comprobaciones  |  packs: {n_pass} PASS, "
          f"{n_falla} FALLA, {n_abort} ABORTADO")
    print("=" * 78)

    for b in bancos:
        if b.estado != "PASS":
            print(f"  [{b.estado}] {b.nombre}: {b.resumen}")
            for f in b.fallos:
                print(f"      - FALLA {f}")
            for r in b.rotas:
                print(f"      - ROTA  {r}")

    if n_abort:
        print("\n ABORTADO NO ES PASS: esos packs no midieron nada del firmware.")

    if n_falla:
        return 1
    if n_abort:
        return 2
    return 0


if __name__ == "__main__":
    sys.exit(main())
