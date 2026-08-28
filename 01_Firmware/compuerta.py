#!/usr/bin/env python3
# ===== 01_Firmware/compuerta.py =====
#
# COMPUERTA UNICA DE VERIFICACION — N-28.
#
# Compila los firmwares y corre TODAS las comprobaciones de una vez, con un unico
# codigo de salida. Existe por un fallo concreto, no por prolijidad:
#
#   validador_maestro.py llevaba dias ABORTANDO en silencio -no encontraba una
#   constante en respaldo.cpp tras reescribirse el checksum- y nadie lo noto,
#   porque cada validador se lanzaba a mano y por separado. Desde fuera parecia que
#   habia corrido. El Maestro estuvo sin cobertura de validacion y las cifras que el
#   README publicaba se habian medido contra un fuente que ya no existia.
#
# LA DISTINCION QUE JUSTIFICA ESTE SCRIPT: PASS / FALLA / ABORTADO.
#
#   PASS      la comprobacion corrio y el firmware cumple.
#   FALLA     la comprobacion corrio y el firmware NO cumple. Hay que arreglarlo.
#   ABORTADO  la comprobacion NO PUDO correr. No dice nada del firmware.
#
# Confundir ABORTADO con PASS es como se pierde la cobertura sin enterarse, y es
# justo lo que paso. Aqui ABORTADO nunca cuenta como exito: si algo no pudo medirse,
# la compuerta se cierra igual que si hubiera fallado, y el resumen lo dice con esa
# palabra para que la diferencia no se diluya.
#
# La convencion de codigos de salida es la que ya usaban los validadores:
#   0 = PASS, 1 = FALLA (propiedad rota), 2 = ABORTADO (no se pudo medir).
#
# USO:  python 01_Firmware/compuerta.py
#       python 01_Firmware/compuerta.py --rapido    (sin compilar, solo modelos)

import os
import re
import glob
import subprocess
import sys
import shutil

RAIZ = os.path.dirname(os.path.abspath(__file__))
PIO = shutil.which("pio") or shutil.which("platformio") or r"C:\.platformio\penv\Scripts\platformio.exe"

PASS, FALLA, ABORTADO = "PASS", "FALLA", "ABORTADO"

resultados = []  # (nombre, estado, detalle)

# La consola de Windows suele venir en cp1252 y la salida de los validadores trae
# acentos, flechas y emojis. Sin esto, la compuerta REVIENTA al imprimir el resumen
# -que es lo unico que de verdad importa- por un caracter de adorno. Un instrumento
# que se cae al dar el resultado no sirve, asi que se fuerza UTF-8 y, ademas, cada
# detalle se reduce a ASCII antes de imprimirlo.
try:
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")
except Exception:
    pass


def _ascii(t):
    return "".join(c if 32 <= ord(c) < 127 else " " for c in t).strip()


def anotar(nombre, estado, detalle=""):
    detalle = _ascii(detalle)
    resultados.append((nombre, estado, detalle))
    marca = {PASS: "  OK  ", FALLA: " FALLA", ABORTADO: " ABORT"}[estado]
    print(f"  [{marca}] {nombre}" + (f"  - {detalle}" if detalle else ""))


# ---------------------------------------------------------------------------
# GUARDA DE RUTAS — la trampa de la migracion a lib/Common.
#
# Los validadores no incluyen el firmware: lo PARSEAN, y direccionan cada fuente por
# tuplas -("Maestro","src","mando.cpp")-. Eso significa que mover un archivo de sitio
# los deja midiendo un fuente que ya no existe. Es N-36 otra vez, y la primera vez
# costo dias de cobertura fantasma.
#
# La guarda NO lleva la lista de rutas escrita a mano: eso seria una segunda copia que
# se desincroniza igual. Lee las tuplas de los propios validadores, que es la unica
# fuente que no puede mentir sobre lo que ellos abren.
#
# Y lleva un SUELO: si el censo encuentra sospechosamente pocas rutas, aborta en vez
# de aprobar. Un buscador que no encuentra nada no demuestra que no haya nada -esa
# leccion la pago este proyecto dos veces, con gcc y con el recuento a mano-, asi que
# una guarda que se queda ciega tiene que decirlo, no dar verde.
# ---------------------------------------------------------------------------

RUTAS_MINIMAS_ESPERADAS = 20

_ROLES = ("Maestro", "Esclavo", "Repetidor")

# ("Maestro", "src", "mando.cpp") — admite saltos de linea dentro de la llamada.
_RE_TRIPLE = re.compile(
    r'["\'](Maestro|Esclavo|Repetidor)["\']\s*,\s*'
    r'["\'](src|include)["\']\s*,\s*'
    r'["\']([A-Za-z0-9_]+\.(?:h|cpp|ini))["\']', re.S)

# ("src", "respaldo.cpp") sueltas, que el validador combina con ambos roles.
_RE_PAR = re.compile(
    r'["\'](src|include)["\']\s*,\s*["\']([A-Za-z0-9_]+\.(?:h|cpp|ini))["\']')


def censo_de_rutas():
    """Comprueba que cada fuente que los instrumentos dicen abrir existe de verdad."""
    # Se censa TODO el arbol de instrumentos, no solo el nivel de arriba: al retirar
    # los tres monoliticos las rutas se quedaron viviendo en banco/packs y en
    # banco/modelos, y un censo que mirase solo Simulaciones/*.py caeria a 4 rutas.
    # El suelo de abajo lo detecto y aborto, que es su trabajo; pero la guarda tiene
    # que mirar donde estan los instrumentos de hoy, no donde estaban ayer.
    fuentes = sorted(glob.glob(os.path.join(RAIZ, "Simulaciones", "*.py"))
                     + glob.glob(os.path.join(RAIZ, "Simulaciones", "banco", "**", "*.py"),
                                 recursive=True))
    if not fuentes:
        anotar("guarda de rutas", ABORTADO, "no se encontro ningun validador que censar")
        return

    rutas = set()
    for f in fuentes:
        try:
            with open(f, "r", encoding="utf-8", errors="replace") as fh:
                txt = fh.read()
        except OSError as e:
            anotar("guarda de rutas", ABORTADO, f"no se pudo leer {os.path.basename(f)}: {e}")
            return
        for rol, carpeta, fichero in _RE_TRIPLE.findall(txt):
            rutas.add((rol, carpeta, fichero))
        # Las tuplas de dos elementos no dicen el rol; se exigen en ambas puntas,
        # que es como las usa el validador de costura.
        for carpeta, fichero in _RE_PAR.findall(txt):
            if not any((r, carpeta, fichero) in rutas for r in _ROLES):
                for r in ("Maestro", "Esclavo"):
                    rutas.add((r, carpeta, fichero))

    if len(rutas) < RUTAS_MINIMAS_ESPERADAS:
        anotar("guarda de rutas", ABORTADO,
               f"el censo solo hallo {len(rutas)} rutas (esperadas >= "
               f"{RUTAS_MINIMAS_ESPERADAS}): fallo el buscador, no el arbol")
        return

    faltan = [os.path.join(*r) for r in sorted(rutas)
              if not os.path.isfile(os.path.join(RAIZ, *r))]
    if faltan:
        anotar("guarda de rutas", ABORTADO,
               f"{len(faltan)} fuente(s) que los validadores parsean NO existen: "
               + ", ".join(faltan[:4]) + (" ..." if len(faltan) > 4 else ""))
        return
    anotar("guarda de rutas", PASS, f"{len(rutas)} rutas parseadas, todas existen")


def _es_ascii(t):
    return all(ord(c) < 128 for c in t)


def _base_ascii():
    """Un directorio de trabajo cuya ruta sea ASCII, o None si no hay ninguno.

    No es un capricho: el propio TEMP de este usuario vive bajo una carpeta con
    una 'n' con tilde, y ahi el toolchain roto que motiva N-44 falla SIEMPRE. Un
    test de enlazado montado en TEMP daria negativo hasta con un gcc sano, y
    rechazaria el bueno. El buscador tiene que saber encontrar."""
    for base in (RAIZ, os.environ.get("TEMP", ""), os.getcwd()):
        if base and _es_ascii(base) and os.path.isdir(base):
            return base
    return None


def _enlaza_de_verdad(gcc):
    """Comprueba que ese gcc puede ENLAZAR, no solo que existe y responde.

    N-44: gcc.exe estaba, contestaba --version y compilaba a .o, pero su ld no
    encontraba ni crt2.o ni libgcc.a -que existen, miden lo que deben y se leen
    sin problema- porque el toolchain vivia bajo una ruta con 'n' con tilde. Los
    DOS arneses que compilan C++ real pasaron de PASS a ABORTADO de un dia para
    otro sin que nadie tocara el compilador, y el acta registraba la misma version
    de gcc en las dos corridas.

    Preguntar "hay gcc?" no distingue ese caso. Enlazar un main() vacio si. Es la
    diferencia entre censar el instrumento y comprobar que mide."""
    base = _base_ascii()
    if base is None:
        return True  # sin sitio donde probar, no se puede afirmar que este roto
    d = os.path.join(base, ".compuerta_prueba_gcc")
    try:
        os.makedirs(d, exist_ok=True)
        c = os.path.join(d, "prueba.c")
        exe = os.path.join(d, "prueba.exe")
        with open(c, "w", encoding="ascii") as f:
            f.write("int main(void){return 0;}\n")
        p = subprocess.run([gcc, c, "-o", exe], capture_output=True, text=True,
                           errors="replace", timeout=120)
        return p.returncode == 0 and os.path.isfile(exe)
    except (OSError, subprocess.SubprocessError):
        return False
    finally:
        shutil.rmtree(d, ignore_errors=True)


_GCC = []        # cache: vacio = sin resolver; [ruta] o [None] = ya resuelto
MOTIVO_GCC = ""  # por que no hay gcc utilizable, para que el ABORTADO lo diga


def _asegurar_gcc():
    """Devuelve la ruta de un gcc QUE ENLAZA, y lo mete en el PATH del entorno.

    N-38: gcc ESTABA instalado desde el principio -winget lo dejo en su carpeta de
    paquetes, fuera del PATH- y shutil.which() no lo veia. El ABORTADO que decia "las
    209 pantallas NO se validaron" era un falso negativo del instrumento. Que hoy este
    en el PATH de una consola no garantiza que lo este en la de manana, asi que la
    compuerta lo busca donde los instaladores lo dejan.

    N-44 anade la otra mitad: encontrarlo no basta. Un candidato solo vale si
    enlaza, y el primero que enlaza es el que se usa -aunque haya otro antes en el
    PATH-. Asi un toolchain instalado en ruta no-ASCII deja de ser un ABORTADO
    misterioso y pasa a ser un candidato descartado con motivo."""
    global MOTIVO_GCC
    if _GCC:
        return _GCC[0]

    candidatos = []
    g = shutil.which("gcc")
    if g:
        candidatos.append(g)
    patrones = [
        r"D:\toolchain\mingw64\bin\gcc.exe",
        r"C:\mingw64\bin\gcc.exe",
        r"C:\msys64\mingw64\bin\gcc.exe",
        r"C:\msys64\ucrt64\bin\gcc.exe",
        r"C:\MinGW\bin\gcc.exe",
        os.path.join(os.environ.get("ProgramFiles", ""), "mingw64", "bin", "gcc.exe"),
        os.path.join(os.environ.get("LOCALAPPDATA", ""), "Microsoft", "WinGet",
                     "Packages", "*", "mingw64", "bin", "gcc.exe"),
    ]
    for p in patrones:
        candidatos += sorted(x for x in glob.glob(p) if x not in candidatos)

    rotos = []
    for c in candidatos:
        if _enlaza_de_verdad(c):
            os.environ["PATH"] = os.path.dirname(c) + os.pathsep + os.environ.get("PATH", "")
            _GCC.append(c)
            return c
        rotos.append(c)

    if rotos:
        MOTIVO_GCC = (f"hay {len(rotos)} gcc instalado(s) pero NINGUNO enlaza; "
                      f"el primero, en {os.path.dirname(rotos[0])}")
        if not _es_ascii(rotos[0]):
            MOTIVO_GCC += " (ruta con caracteres no ASCII: su ld no encuentra crt2.o)"
    else:
        MOTIVO_GCC = "no hay gcc de host (MinGW-w64) instalado"
    _GCC.append(None)
    return None


def compilar(entorno, carpeta):
    """Compila un proyecto de PlatformIO. Sin PlatformIO no es un fallo del
    firmware: es que aqui no se puede compilar. Eso es ABORTADO."""
    if not (PIO and (os.path.isfile(PIO) or shutil.which(PIO))):
        anotar(f"compila {entorno}", ABORTADO, "no se encuentra platformio.exe o pio en PATH")
        return
    d = os.path.join(RAIZ, carpeta)
    if not os.path.isdir(d):
        anotar(f"compila {entorno}", ABORTADO, f"no existe {carpeta}")
        return
    p = subprocess.run([PIO, "run"], cwd=d, capture_output=True, text=True,
                       errors="replace")
    if p.returncode != 0:
        cola = [l for l in p.stdout.splitlines() if "error" in l.lower()][-1:]
        anotar(f"compila {entorno}", FALLA, cola[0].strip() if cola else "ver salida")
        return
    uso = [l.strip() for l in p.stdout.splitlines() if l.startswith("Flash:")]
    anotar(f"compila {entorno}", PASS, uso[0] if uso else "")


def correr_python(nombre, script, base=None):
    """Los validadores ya distinguen los tres estados por codigo de salida: 0 PASS,
    1 propiedad rota, 2 no se pudo medir. Aqui solo se respeta esa convencion en vez
    de reinterpretarla.

    N-46: SALVO QUE LA SALIDA DIGA OTRA COSA. validador_maestro.py imprime 7 lineas
    [FALLA] -propiedades que probo contra el C++ real y que el firmware NO cumple, una
    de ellas vial- y despues sale con codigo 0, porque alguien las declaro "residuales
    aceptados". La compuerta se fiaba del numero y escribia [OK] 60/67 en el acta.

    O falla o no falla. Una comprobacion que corrio y dio FALLA es FALLA, y ninguna
    etiqueta de "residual" la convierte en PASS: si se acepta convivir con ella, se
    acepta con el acta en rojo delante, no escondiendola detras de un codigo de salida.
    El codigo 0 se sigue respetando para todo lo demas; lo que ya no puede es
    CONTRADECIR a la propia salida del validador."""
    # base: los instrumentos viven casi todos en Simulaciones/, pero no todos. El de
    # la app esta junto a la app que mide, y eso no es motivo para dejarlo fuera del
    # acta -que es exactamente como se perdio de vista simulador_app_bluetooth.py-.
    ruta = os.path.join(base or os.path.join(RAIZ, "Simulaciones"), script)
    if not os.path.isfile(ruta):
        anotar(nombre, ABORTADO, f"no existe {script}")
        return
    p = subprocess.run([sys.executable, ruta], capture_output=True, text=True,
                       errors="replace")
    salida = (p.stdout or "") + (p.stderr or "")

    if "[ABORTADO]" in salida or p.returncode == 2:
        motivo = next((l.strip() for l in salida.splitlines() if "[ABORTADO]" in l), "")
        anotar(nombre, ABORTADO, motivo[:110])
        return
    if "Traceback" in salida:
        anotar(nombre, ABORTADO, "excepcion de Python, no llego a medir")
        return

    cuenta = ""
    for l in reversed(salida.splitlines()):
        if "/" in l and any(c.isdigit() for c in l) and \
           ("PASS" in l.upper() or "comprobacion" in l.lower()):
            cuenta = l.strip()[:110]
            break

    # N-46. Se cuentan las lineas [FALLA] de la propia salida. No es una heuristica
    # sobre texto libre: es la marca exacta que los validadores imprimen al romper una
    # propiedad, la misma que este script usa para [ABORTADO].
    impresos = sum(1 for l in salida.splitlines() if "[FALLA]" in l)
    if impresos and p.returncode == 0:
        anotar(nombre, FALLA,
               f"{impresos} FALLA impresos y exit 0 (N-46) | {cuenta}"[:110])
        return

    # N-71: LA CUARTA CARA DE N-46, Y LA QUE ESTUVO ABIERTA MAS TIEMPO.
    #
    # El detector de arriba busca la marca literal "[FALLA]". Es la que imprimen los
    # validadores del banco... y NINGUNO de los dos simuladores mas viejos. Aquellos
    # escriben "X FAIL:" y cierran con "VEREDICTO FINAL: 17/20 PASS - HAY FALLOS
    # PENDIENTES", saliendo con codigo 0. Resultado: el simulador funcional podia caer
    # de 20/20 a 17/20 y el acta seguia diciendo [OK] con la cuenta mala al lado, que
    # nadie lee cuando el semaforo de la izquierda esta en verde.
    #
    # Se descubrio al subir el techo de SFTY-6 a 25 s: tres pruebas que daban por
    # sentado el ambar a los 12 s empezaron a fallar y la compuerta no se inmuto.
    #
    # LA REGLA NUEVA NO DEPENDE DEL MARCADOR, que es justo lo que fallaba: si el
    # instrumento publica una cuenta "x/y", se exige x == y. Un instrumento que anuncia
    # 17 de 20 esta diciendo que tres comprobaciones no cumplen, lo escriba como lo
    # escriba y salga con el codigo que salga.
    ratios = re.findall(r"(\d+)\s*/\s*(\d+)", cuenta)
    incompletas = [(a, b) for a, b in ratios if int(a) != int(b)]
    if incompletas and p.returncode == 0:
        detalle = ", ".join("%s/%s" % r for r in incompletas)
        anotar(nombre, FALLA,
               f"cuenta incompleta {detalle} con exit 0 (N-71) | {cuenta}"[:110])
        return

    anotar(nombre, PASS if p.returncode == 0 else FALLA, cuenta)


def arnes_lcd():
    """El arnes compila los lcd.cpp REALES contra un framebuffer en el PC. Necesita
    gcc de host: sin el no puede correr, y eso es ABORTADO, no PASS. Ese matiz
    importa mas de lo que parece -las 209 pantallas dejan de estar cubiertas- y sin
    esta compuerta se pasaba por alto."""
    d = os.path.join(RAIZ, "Validacion_LCD")
    if not os.path.isdir(d):
        anotar("arnes de pantalla", ABORTADO, "no existe Validacion_LCD")
        return
    if _asegurar_gcc() is None:
        anotar("arnes de pantalla", ABORTADO,
               f"{MOTIVO_GCC}: las pantallas NO se validaron")
        return
    # -ExecutionPolicy Bypass NO es un atajo de comodidad: sin el, la politica del
    # sistema rechaza compilar.ps1 y la compuerta se comia el error como si el arnes
    # hubiera corrido y fallado. Se descubrio porque el acta salia SIN CIFRA: 433
    # caracteres de salida donde el arnes escribe 239 comprobaciones.
    p = subprocess.run(["powershell", "-NoProfile", "-ExecutionPolicy", "Bypass",
                        "-File", os.path.join(d, "compilar.ps1")],
                       cwd=d, capture_output=True, text=True, errors="replace")
    salida = (p.stdout or "") + (p.stderr or "")

    # UN ARNES QUE NO ARRANCA ES ABORTADO, NO FALLA. La distincion es la razon de ser
    # de este script y aqui misma se estaba incumpliendo: FALLA afirma "el firmware no
    # cumple", y un arnes bloqueado por la politica de ejecucion no midio el firmware.
    if ("ejecuci" in salida and "deshabilitada" in salida) or \
       "UnauthorizedAccess" in salida or "cannot be loaded" in salida:
        anotar("arnes de pantalla", ABORTADO,
               "la politica de ejecucion de PowerShell bloqueo compilar.ps1")
        return
    if "comprobaciones" not in salida:
        anotar("arnes de pantalla", ABORTADO,
               f"el arnes no llego a medir (solo {len(salida)} caracteres de salida)")
        return
    # El arnes resume en "MAESTRO 110/113 comprobaciones OK" y "TOTAL 3 de 239 ...
    # FALLARON": dice OK y FALLARON, no PASS. Buscar "PASS" aqui dejaba el acta sin
    # cifra -y un acta sin cifra no certifica nada-.
    partes = [l.strip() for l in salida.splitlines()
              if re.search(r"\b(MAESTRO|ESCLAVO)\b\s+\d+/\d+", l)]
    total = next((l.strip() for l in reversed(salida.splitlines())
                  if "TOTAL" in l.upper() and any(c.isdigit() for c in l)), "")
    cuenta = " | ".join(partes + ([total] if total and not partes else []))
    if partes and total:
        cuenta += " | " + total
    anotar("arnes de pantalla", PASS if p.returncode == 0 else FALLA, cuenta[:150])


# ---------------------------------------------------------------------------
# ACTA DE EVIDENCIA.
#
# "El simulador da 20/20" en un README es una frase: envejece sin avisar y ya envejecio
# una vez -las cifras publicadas se habian medido contra un fuente que ya no existia-.
# Un acta con FECHA y HASH DE HEAD es un artefacto: el auditor la re-corre sobre ese
# mismo commit y compara. Las cifras del README se COPIAN de la ultima acta; no se
# escriben a mano nunca mas.
# ---------------------------------------------------------------------------

def _cmd(args, cwd=None):
    try:
        p = subprocess.run(args, cwd=cwd, capture_output=True, text=True,
                           errors="replace", timeout=30)
        return p.stdout.strip() if p.returncode == 0 else ""
    except Exception:
        return ""


def escribir_acta(n_pass, n_falla, n_abort, rapido):
    import datetime
    repo = os.path.dirname(RAIZ)
    head = _cmd(["git", "rev-parse", "--short", "HEAD"], cwd=repo) or "SIN-GIT"
    rama = _cmd(["git", "rev-parse", "--abbrev-ref", "HEAD"], cwd=repo) or "?"
    # La propia acta ensucia el arbol al escribirse, asi que evidencia/ no cuenta: si
    # contara, TODA acta nacería avisando de que no corresponde a su hash.
    sucio = "\n".join(l for l in _cmd(["git", "status", "--porcelain"], cwd=repo).splitlines()
                      if "evidencia/" not in l.replace("\\", "/"))
    gcc = _asegurar_gcc()
    ver_gcc = (_cmd([gcc, "--version"]).splitlines() or [""])[0] if gcc else "AUSENTE"
    ver_pio = (_cmd([PIO, "--version"]) or "AUSENTE") if os.path.isfile(PIO) else "AUSENTE"

    hoy = datetime.date.today().isoformat()
    d = os.path.join(repo, "evidencia")
    os.makedirs(d, exist_ok=True)
    ruta = os.path.join(d, f"{hoy}_compuerta.txt")

    lineas = [
        "ACTA DE VERIFICACION - Controladora de Semaforos",
        "=" * 78,
        f"Fecha        : {hoy}",
        f"HEAD         : {head}   rama: {rama}",
        f"Arbol        : {'LIMPIO' if not sucio else 'CON CAMBIOS SIN COMMITEAR'}",
        f"Modo         : {'--rapido (SIN compilar)' if rapido else 'completo'}",
        f"PlatformIO   : {ver_pio}",
        f"GCC de host  : {ver_gcc}",
        "",
        "RESULTADOS",
        "-" * 78,
    ]
    # El nombre y el detalle van en columnas, pero la SEPARACION entre ellos no
    # puede depender del relleno. Con un ancho fijo de 26, "simulador de app y
    # bluetooth" -28 caracteres- desbordo la columna, el hueco cayo a UN espacio, y
    # el pack que lee esta acta (documentos_01_cifras_del_acta) dejo de poder partir
    # la linea en nombre y detalle. No dio error: leyo 14 resultados de 15 y siguio.
    # Por eso el ancho se mide del nombre mas largo en vez de fijarse a mano, y
    # ademas se emiten DOS espacios siempre: aunque manana un nombre vuelva a
    # desbordar, la linea se seguira pudiendo partir.
    ancho = max([26] + [len(n) for n, _, _ in resultados])
    for n, e, det in resultados:
        lineas.append(f"  {e:<9} {n:<{ancho}}  {det}")
    lineas += [
        "-" * 78,
        f"  RESUMEN: {n_pass} PASS | {n_falla} FALLA | {n_abort} ABORTADO",
        "",
        "ABORTADO no es PASS: una comprobacion que no pudo correr no dice nada del",
        "firmware. Esta acta se verifica volviendo a " + head + " y re-corriendo",
        "python 01_Firmware/compuerta.py.",
    ]
    if sucio:
        lineas += ["", "AVISO: el arbol tenia cambios sin commitear al medir. Estas",
                   "cifras NO corresponden exactamente a " + head + "."]
    try:
        with open(ruta, "w", encoding="utf-8") as f:
            f.write("\n".join(lineas) + "\n")
        print(f"\n Acta escrita: evidencia/{os.path.basename(ruta)}")
    except OSError as e:
        print(f"\n No se pudo escribir el acta: {e}")


def arnes_ciclo():
    """FASE 6: el primer trozo de firmware que se MIDE en vez de duplicarse.

    Compila el ciclo_degradado.h REAL y barre las 86.400 posiciones del dia sobre EL.
    Hasta ahora ese barrido corria contra un espejo en Python reescrito a mano, y el
    validador de costura llego a comprobar el espejo LINEA POR LINEA contra el C++ con
    expresiones regulares: una prueba para vigilar a la prueba. Aqui no hay espejo que
    envejecer, asi que no hay N-36 posible en este camino."""
    d = os.path.join(RAIZ, "Validacion_Ciclo")
    if not os.path.isdir(d):
        anotar("arnes del ciclo", ABORTADO, "no existe Validacion_Ciclo")
        return
    if _asegurar_gcc() is None:
        anotar("arnes del ciclo", ABORTADO, MOTIVO_GCC)
        return
    p = subprocess.run(["powershell", "-NoProfile", "-ExecutionPolicy", "Bypass",
                        "-File", os.path.join(d, "compilar.ps1")],
                       cwd=d, capture_output=True, text=True, errors="replace")
    salida = (p.stdout or "") + (p.stderr or "")
    if "comprobaciones" not in salida:
        anotar("arnes del ciclo", ABORTADO,
               f"no llego a medir (solo {len(salida)} caracteres de salida)")
        return
    cuenta = next((l.strip() for l in reversed(salida.splitlines())
                   if "RESULTADO" in l), "")
    anotar("arnes del ciclo", PASS if p.returncode == 0 else FALLA, cuenta[:110])


def arnes_dom():
    """La app EJECUTADA, no leida. Es el escalon que faltaba (N-66).

    Todo lo que el banco sabia de la app comparaba TEXTO: que los strings casaran
    entre el .js, el .cpp y el manual. Eso no ve un TypeError, ni un getElementById
    que devuelve null, ni un boton que no dispara. Esta suite carga index.html y
    app.js en un DOM de verdad (jsdom) y los ejercita: pestanas, modales, ingesta de
    telemetria, fuzzing de tramas corruptas y los botones que mandan comandos.

    SI FALTA NODE O JSDOM, ESTO ES ABORTADO, NO PASS. La suite vive fuera de Python y
    necesita `npm install` en 05_Funcional/App_Semaforo; node_modules no se versiona
    -50 MB- asi que en una maquina recien clonada esta comprobacion no puede correr.
    Decirlo es justo su trabajo: un hueco silencioso es lo que este proyecto lleva un
    mes cerrando."""
    d = os.path.join(os.path.dirname(RAIZ), "05_Funcional", "App_Semaforo")
    script = os.path.join(d, "test_dom_execution.js")
    if not os.path.isfile(script):
        anotar("app ejecutada en DOM", ABORTADO, "no existe test_dom_execution.js")
        return
    if not os.path.isdir(os.path.join(d, "node_modules", "jsdom")):
        anotar("app ejecutada en DOM", ABORTADO,
               "falta jsdom: correr 'npm install' en 05_Funcional/App_Semaforo")
        return
    node = shutil.which("node")
    if node is None:
        anotar("app ejecutada en DOM", ABORTADO, "no hay node en el PATH")
        return
    p = subprocess.run([node, script], cwd=d, capture_output=True, text=True,
                       errors="replace")
    salida = (p.stdout or "") + (p.stderr or "")
    if "RESULTADO JSDOM" not in salida:
        anotar("app ejecutada en DOM", ABORTADO,
               f"no llego a medir (solo {len(salida)} caracteres de salida)")
        return
    cuenta = next((l.strip() for l in reversed(salida.splitlines())
                   if "RESULTADO JSDOM" in l), "")
    anotar("app ejecutada en DOM", PASS if p.returncode == 0 else FALLA, cuenta[:110])


def arnes_unitarios_app():
    """Ejecuta los tests unitarios puros de JavaScript (test_unitarios_app.js).
    Cubre NMEA XOR checksums, parsing $STATUS/$ALARM/$ERR, barrera PIN, Courier RTC,
    y CRUD de cruces viales."""
    d = os.path.join(os.path.dirname(RAIZ), "05_Funcional", "App_Semaforo")
    script = os.path.join(d, "test_unitarios_app.js")
    if not os.path.isfile(script):
        anotar("test unitarios de la app", ABORTADO, "no existe test_unitarios_app.js")
        return
    node = shutil.which("node")
    if node is None:
        anotar("test unitarios de la app", ABORTADO, "no hay node en el PATH")
        return
    p = subprocess.run([node, script], cwd=d, capture_output=True, text=True,
                       errors="replace")
    salida = (p.stdout or "") + (p.stderr or "")
    if "RESUMEN DE PRUEBAS" not in salida:
        anotar("test unitarios de la app", ABORTADO,
               f"no llego a medir (solo {len(salida)} caracteres de salida)")
        return
    cuenta = next((l.strip() for l in reversed(salida.splitlines())
                   if "RESUMEN DE PRUEBAS" in l), "")
    anotar("test unitarios de la app", PASS if p.returncode == 0 else FALLA, cuenta[:110])


def arnes_respaldo():
    """N-43 / N-29: compila el calcularSuma() REAL, y hasta hoy no estaba aqui.

    Existia desde N-31, compilaba el fuente que va a la tarjeta y llevaba dias
    roto -- y el acta no lo echaba de menos, porque eran 12 suites y ninguna era
    esta. Un ABORTADO al menos grita; un hueco no. Por eso conectarlo forma parte
    de escribir un arnes, no es un paso posterior.

    Comprueba dos cosas distintas, y se clasifican distinto:
      - respaldo.cpp/.h IDENTICOS entre Maestro y Esclavo -> si difieren, las dos
        puntas fechan la misma sincronizacion con aritmetica distinta. Eso es el
        firmware incumpliendo: FALLA.
      - el binario contesta al PING -> si no arranca, no se midio nada: ABORTADO.
    """
    d = os.path.join(RAIZ, "Validacion_Respaldo")
    if not os.path.isdir(d):
        anotar("arnes del respaldo", ABORTADO, "no existe Validacion_Respaldo")
        return
    if _asegurar_gcc() is None:
        anotar("arnes del respaldo", ABORTADO, MOTIVO_GCC)
        return
    p = subprocess.run(["powershell", "-NoProfile", "-ExecutionPolicy", "Bypass",
                        "-File", os.path.join(d, "compilar.ps1")],
                       cwd=d, capture_output=True, text=True, errors="replace")
    salida = (p.stdout or "") + (p.stderr or "")

    # DIFIERE es la unica salida de este arnes que habla del firmware. Todo lo
    # demas que impida llegar hasta ahi es ABORTADO, nunca FALLA: acusar al
    # firmware de un defecto que no se ha medido es el fallo de N-27, el de la
    # propia compuerta y el que casi se comete aqui.
    if "DIFIERE" in salida:
        motivo = next((l.strip() for l in salida.splitlines() if "DIFIERE" in l), "")
        anotar("arnes del respaldo", FALLA, _ascii(motivo)[:130])
        return
    if p.returncode != 0 or "OK:" not in salida:
        anotar("arnes del respaldo", ABORTADO,
               f"el arnes no llego a medir ({len(salida)} caracteres de salida)")
        return
    anotar("arnes del respaldo", PASS,
           "respaldo.cpp y respaldo.h identicos entre puntas | arnes vivo (PING/PONG)")


def arnes_automatico():
    """El arnes que faltaba: ejerce el CICLO AUTOMATICO sobre el C++ real.

    La regresion del Modo Automatico paso con la compuerta en verde y el arnes de
    pantalla en 241/241, y no fue mala suerte: ningun instrumento ejercia
    coordinador.cpp + semaforo.cpp + modo_automatico.cpp. Los simuladores son
    Python escrito a mano que REIMPLEMENTA lo que hace el C++, asi que su PASS
    hablaba del modelo, no del codigo que se carga en la tarjeta.

    Comprobado que sabe fallar antes de conectarlo: con VERDE1 forzado a HIGH por
    debajo del enclavamiento de semaforo.cpp, baja a 25/26 y sale con 1, y la que
    cae es la de SFTY-2 medida sobre las escrituras de pin reales. Un arnes que no
    se ha visto fallar no es una prueba, es un adorno que da verde."""
    d = os.path.join(RAIZ, "Validacion_Automatico")
    if not os.path.isdir(d):
        anotar("arnes del automatico", ABORTADO, "no existe Validacion_Automatico")
        return
    if _asegurar_gcc() is None:
        anotar("arnes del automatico", ABORTADO, MOTIVO_GCC)
        return
    p = subprocess.run(["powershell", "-NoProfile", "-ExecutionPolicy", "Bypass",
                        "-File", os.path.join(d, "compilar.ps1")],
                       cwd=d, capture_output=True, text=True, errors="replace")
    salida = (p.stdout or "") + (p.stderr or "")
    if "comprobaciones" not in salida:
        anotar("arnes del automatico", ABORTADO,
               f"no llego a medir (solo {len(salida)} caracteres de salida)")
        return
    cuenta = next((l.strip() for l in reversed(salida.splitlines())
                   if "RESULTADO" in l), "")
    anotar("arnes del automatico", PASS if p.returncode == 0 else FALLA, cuenta[:110])


def main():
    rapido = "--rapido" in sys.argv
    print("=" * 78)
    print(" COMPUERTA DE VERIFICACION - Controladora de Semaforos")
    print("=" * 78)

    print("\n-- Instrumentos --")
    censo_de_rutas()

    if not rapido:
        print("\n-- Compilacion --")
        compilar("maestro", "Maestro")
        compilar("esclavo", "Esclavo")
        compilar("repetidor", "Repetidor")

    print("\n-- Modelos de comportamiento --")
    correr_python("simulador funcional", "simulador_sistema_v7_6.py")
    correr_python("simulador de repetidor", "simulador_repetidor.py")
    # N-62: existia desde el 26/08 y NO estaba aqui. "Un instrumento que no esta en la
    # compuerta no mide nada, y no deja rastro de que falta": la lista de la sesion de
    # banco lo daba por hecho -"simulador_app_bluetooth.py en 5/5 PASS"- y el acta no
    # lo echaba de menos porque no lo conocia. Se conecta despues de arreglarle la
    # prueba 2, que contaba rechazos sin comprobar ninguno; con la barrera de PIN rota
    # a proposito cae con 49.996 intentos colados, que es como se demuestra que mide.
    correr_python("simulador de app y bluetooth", "simulador_app_bluetooth.py")
    # N-62: el test funcional de la app tampoco estaba en la compuerta, y su resumen
    # decia "22/22" a mano cuando ejecuta 34 comprobaciones. Ademas su prueba de
    # Courier RTC era una tautologia -t1 - (t0 + viaje) con t1 = t0 + viaje-, que no
    # puede fallar. Corregida y conectada, se vio caer al romper la compensacion en el
    # app.js real.
    correr_python("test funcional de la app", os.path.join("App_Semaforo", "test_funcional_app.py"),
                  base=os.path.join(os.path.dirname(RAIZ), "05_Funcional"))
    arnes_unitarios_app()
    arnes_dom()

    print("\n-- Validadores de firmware --")
    # Banco por packs. La migracion termino: los tres monoliticos se retiraron tras
    # demostrar, uno a uno, que los packs sumaban EXACTAMENTE sus comprobaciones y
    # que el texto de cada una coincidia literalmente. Costura 41 = 41, Maestro
    # 64/67 = 64/67, Esclavo 31 = 31.
    #
    # Retirarlos no fue solo limpieza: los monolitos imprimian FALLA y salian con 0,
    # asi que un fallo real se pintaba en verde (N-46). Los packs si lo cuentan.
    correr_python("banco por packs", os.path.join("banco", "correr.py"))

    # FASE 6: lo que se compila y se ejecuta DE VERDAD, frente a los modelos de arriba.
    # Es la unica seccion cuyo PASS habla del codigo y no de una copia suya.
    print("\n-- Firmware compilado y ejecutado en el PC --")
    arnes_lcd()
    arnes_ciclo()
    arnes_respaldo()
    arnes_automatico()

    n_pass = sum(1 for _, e, _ in resultados if e == PASS)
    n_falla = sum(1 for _, e, _ in resultados if e == FALLA)
    n_abort = sum(1 for _, e, _ in resultados if e == ABORTADO)

    print("\n" + "=" * 78)
    print(f" RESUMEN: {n_pass} PASS | {n_falla} FALLA | {n_abort} ABORTADO"
          f"  (de {len(resultados)})")
    print("=" * 78)

    if n_abort:
        print("\n ABORTADO NO ES PASS. Estas comprobaciones no llegaron a medir nada,")
        print(" asi que no dicen NADA del firmware. Tratarlas como aprobadas es como")
        print(" se perdio la cobertura del Maestro sin que nadie se enterara:")
        for n, e, d in resultados:
            if e == ABORTADO:
                print(f"   - {n}: {d}")
    if n_falla:
        print("\n FALLAN (el firmware no cumple):")
        for n, e, d in resultados:
            if e == FALLA:
                print(f"   - {n}: {d}")

    escribir_acta(n_pass, n_falla, n_abort, rapido)

    if n_falla:
        return 1
    if n_abort:
        return 2
    print("\n Todo verde. Estas cifras son las que puede publicar el README:")
    for n, e, d in resultados:
        if d:
            print(f"   - {n}: {d}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
