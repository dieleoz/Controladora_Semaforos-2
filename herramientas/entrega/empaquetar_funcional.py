# ===== empaquetar_funcional.py =====
#
# Arma el .zip que va al funcional (receta: .claude/particularidades/entregar.md §4):
# 01_Firmware/ (fuente PlatformIO de las cuatro puntas, con FW_HASH.txt = <commit>), la APK,
# App_Web/ (www/) y la hoja de pruebas .html con el codigo de version puesto. Sin LEEME ni guia del PC.
# Un paquete, un commit (D-84): todo sale de `git show <commit>:`; sin [commit], HEAD. El commit es
# ancestro de HEAD, su firmware es el del acta (leida de HEAD:<acta>) y la hoja esta en el.
# Del disco solo la APK, comprobada entrada a entrada contra www/ de <commit> (D-85).
#
# USO: python herramientas/entrega/empaquetar_funcional.py <fecha> <hoja.html> <acta> [commit]
#   p.ej. AAAA-MM-DD 05_Funcional/Pruebas_Funcional_<fecha>.html evidencia/<fecha>_compuerta.txt b220f4d
# Antes: npx cap copy android y gradlew assembleDebug (entregar.md §3).
"""uso: empaquetar_funcional.py <fecha> <hoja.html> <acta> [commit]"""

import hashlib, os, re, shutil, subprocess, sys, zipfile

REPO = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
if len(sys.argv) not in (4, 5):
    sys.exit(__doc__)
FECHA, HTML, ACTA = sys.argv[1], sys.argv[2], sys.argv[3]
C = sys.argv[4] if len(sys.argv) == 5 else "HEAD"
APK_SRC = os.path.join(REPO, r"05_Funcional\App_Semaforo\android\app\build\outputs\apk\debug\app-debug.apk")
WWW = "05_Funcional/App_Semaforo/www/"
FW4 = ["01_Firmware/Maestro", "01_Firmware/Esclavo", "01_Firmware/Repetidor", "01_Firmware/ESP32_Expansion"]


def git(*a):
    return subprocess.run(["git"] + list(a), cwd=REPO, capture_output=True, check=True).stdout


def rc(*a):
    return subprocess.run(["git"] + list(a), cwd=REPO, capture_output=True).returncode


def show(ruta):
    return git("show", C + ":" + ruta)


def lista(*rutas):
    return [r for r in git("ls-tree", "-r", "--name-only", C, "--", *rutas).decode().split("\n") if r]


def lf(b):
    return b.replace(b"\r\n", b"\n")


if rc("rev-parse", "--verify", "--quiet", C + "^{commit}") != 0:
    sys.exit("ABORTA: el commit %s no existe" % C)
h = git("rev-parse", "--short=7", C + "^{commit}").decode().strip()
h_head = git("rev-parse", "--short=7", "HEAD").decode().strip()
if rc("merge-base", "--is-ancestor", h, "HEAD") != 0:
    sys.exit("ABORTA: %s no es ancestro de HEAD %s" % (h, h_head))
acta = git("show", "HEAD:" + ACTA).decode("utf-8", "replace")
hash_acta = [l.split(":")[1].split()[0] for l in acta.splitlines() if l.startswith("HEAD")][0]
# El firmware de <commit> es el que midio el acta.
dif = git("diff", "--name-only", hash_acta, h, "--", *FW4).decode().split()
if dif:
    sys.exit("ABORTA: firmware cambio desde el acta %s: %s" % (hash_acta, dif))
if HTML not in lista(HTML):
    sys.exit("ABORTA: la hoja no esta en %s: %s" % (h, HTML))

nombre = "Paquete_Semaforos_%s_%s_SIN_BANCO" % (FECHA, h)
apk = "IOT_VIAL_Semaforos_%s_%s_SIN_BANCO.apk" % (FECHA, h)
ENT = os.path.join(REPO, "entregas")
apk_dst = os.path.join(ENT, apk)
shutil.copyfile(APK_SRC, apk_dst)

# APK contra www/ de <commit>, entrada por entrada por bytes con CRLF normalizado (D-85).
www = lista(WWW)
with zipfile.ZipFile(apk_dst) as za:
    dentro = set(za.namelist())
    for r in www:
        e = "assets/public/" + r[len(WWW):]
        if e not in dentro:
            sys.exit("ABORTA: falta en la APK " + e)
        if lf(za.read(e)) != lf(show(r)):
            sys.exit("ABORTA: APK no coincide con %s en %s" % (h, e))
    for extra in ("assets/public/cordova.js", "assets/public/cordova_plugins.js"):
        if extra not in dentro:
            sys.exit("ABORTA: falta " + extra)
    if not any(n.endswith("bluetoothSerial.js") for n in dentro):
        sys.exit("ABORTA: falta bluetoothSerial.js")
sha_apk = hashlib.sha256(open(apk_dst, "rb").read()).hexdigest()

fw = [r for r in lista(*FW4) if not r.endswith("README.md")]
html = show(HTML).decode("utf-8")
n_x = html.count("XXXXXXX")
html = re.sub(r"Paquete_Semaforos_\d{4}-\d\d-\d\d_XXXXXXX", "Paquete_Semaforos_%s_XXXXXXX" % FECHA, html)
html = html.replace("XXXXXXX", h)
if n_x == 0 or "XXXXXXX" in html:
    sys.exit("ABORTA: el html no tenia el marcador del hash")

zpath = os.path.join(ENT, nombre + ".zip")
with zipfile.ZipFile(zpath, "w", zipfile.ZIP_DEFLATED) as z:
    for r in fw:
        z.writestr(r, show(r))
    for p in FW4:
        if p + "/platformio.ini" in fw:
            z.writestr(p + "/FW_HASH.txt", h + "\n")
    for r in www:
        z.writestr("App_Web/" + r[len(WWW):], show(r))
    z.write(apk_dst, apk)
    z.writestr(HTML.split("/")[-1], html.encode("utf-8"))

with zipfile.ZipFile(zpath) as z:
    names = z.namelist()
    malos = [n for n in names if any(x in n for x in (".pio/", "/build/", "__pycache__", "node_modules"))
             or n.endswith(".md") or n.endswith(".bin")]
    if malos:
        sys.exit("ABORTA: artefactos o .md en el zip: %s" % malos[:5])
    for r in fw:
        if z.read(r) != show(r):
            sys.exit("ABORTA: difiere de %s %s" % (h, r))
    for r in www:
        if z.read("App_Web/" + r[len(WWW):]) != show(r):
            sys.exit("ABORTA: difiere de %s %s" % (h, r))
    sueltos = [n for n in names if n.endswith(".html") and not n.startswith("App_Web/")]
    if sueltos != [HTML.split("/")[-1]] or b"XXXXXXX" in z.read(sueltos[0]):
        sys.exit("ABORTA: tiene que haber un solo .html fuera de App_Web/, sin marcador: %s" % sueltos)
    if "App_Web/index.html" not in names:
        sys.exit("ABORTA: falta App_Web/index.html en el zip")
    if hashlib.sha256(z.read(apk)).hexdigest() != sha_apk:
        sys.exit("ABORTA: APK del zip no coincide")
print("OK", zpath)
print("commit", h, "" if h == h_head else "(HEAD %s)" % h_head)
print("ficheros", len(names), "firmware", len(fw), "web", len(www), "marcadores html", n_x)
print("APK", apk, sha_apk)
print("ZIP sha256", hashlib.sha256(open(zpath, "rb").read()).hexdigest())
