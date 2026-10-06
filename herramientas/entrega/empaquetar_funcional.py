# ===== empaquetar_funcional.py =====
#
# Arma el .zip que va al funcional (receta: .claude/particularidades/entregar.md):
# 01_Firmware/ (fuente PlatformIO de las cuatro puntas, con FW_HASH.txt = HEAD), la APK,
# App_Web/ (www/) y la hoja de pruebas .html con el codigo de version puesto. Sin LEEME.
# Todo sale de `git show HEAD:`; del disco solo la APK, verificada por CRC contra www/.
# Aborta si el firmware cambio desde el commit del acta de la compuerta.
#
# USO: python herramientas/entrega/empaquetar_funcional.py <fecha> <hoja.html> <acta>
#   p.ej. AAAA-MM-DD 05_Funcional/Pruebas_Funcional_<fecha>.html evidencia/<fecha>_compuerta.txt
# Antes: npx cap copy android y gradlew assembleDebug (entregar.md §3).

import hashlib, os, re, shutil, subprocess, sys, zipfile, zlib

REPO = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
if len(sys.argv) != 4:
    sys.exit(__doc__ or "uso: <fecha> <hoja.html> <acta>")
FECHA = sys.argv[1]
HTML = sys.argv[2]
APK_SRC = os.path.join(REPO, r"05_Funcional\App_Semaforo\android\app\build\outputs\apk\debug\app-debug.apk")
ACTA = sys.argv[3]


def git(*a):
    return subprocess.run(["git"] + list(a), cwd=REPO, capture_output=True, check=True).stdout


def head(ruta):
    return git("show", "HEAD:" + ruta)


h = git("rev-parse", "--short=7", "HEAD").decode().strip()
acta_head = head(ACTA).decode("utf-8", "replace")
hash_acta = [l.split(":")[1].split()[0] for l in acta_head.splitlines() if l.startswith("HEAD")][0]
# El firmware no puede haber cambiado entre el commit del acta y HEAD.
dif = git("diff", "--name-only", hash_acta, "HEAD", "--", "01_Firmware/Maestro", "01_Firmware/Esclavo",
          "01_Firmware/Repetidor", "01_Firmware/ESP32_Expansion").decode().split()
if dif:
    sys.exit("ABORTA: firmware cambio desde el acta %s: %s" % (hash_acta, dif))

nombre = "Paquete_Semaforos_%s_%s_SIN_BANCO" % (FECHA, h)
apk = "IOT_VIAL_Semaforos_%s_%s_SIN_BANCO.apk" % (FECHA, h)
ENT = os.path.join(REPO, "entregas")
apk_dst = os.path.join(ENT, apk)
shutil.copyfile(APK_SRC, apk_dst)

# APK contra www/ de HEAD, entrada por entrada por CRC.
www = [r for r in git("ls-files", "05_Funcional/App_Semaforo/www").decode().split("\n") if r]
with zipfile.ZipFile(apk_dst) as za:
    crc = {i.filename: i.CRC for i in za.infolist()}
    for r in www:
        dentro = "assets/public/" + r[len("05_Funcional/App_Semaforo/www/"):]
        # El disco es HEAD salvo CRLF (mismo blob); la APK se compila del disco.
        if git("hash-object", r).strip() != git("rev-parse", "HEAD:" + r).strip():
            sys.exit("ABORTA: el disco no es HEAD en " + r)
        if crc.get(dentro) != (zlib.crc32(open(os.path.join(REPO, r), "rb").read()) & 0xffffffff):
            sys.exit("ABORTA: APK no coincide con HEAD en " + dentro)
    for extra in ("assets/public/cordova.js", "assets/public/cordova_plugins.js"):
        if extra not in crc:
            sys.exit("ABORTA: falta " + extra)
    if not any(n.endswith("bluetoothSerial.js") for n in crc):
        sys.exit("ABORTA: falta bluetoothSerial.js")
sha_apk = hashlib.sha256(open(apk_dst, "rb").read()).hexdigest()

fw = [r for r in git("ls-files", "01_Firmware/Maestro", "01_Firmware/Esclavo", "01_Firmware/Repetidor",
                      "01_Firmware/ESP32_Expansion").decode().split("\n") if r and not r.endswith("README.md")]
html = head(HTML).decode("utf-8")
n_x = html.count("XXXXXXX")
html = re.sub(r"Paquete_Semaforos_\d{4}-\d\d-\d\d_XXXXXXX", "Paquete_Semaforos_%s_XXXXXXX" % FECHA, html)
html = html.replace("XXXXXXX", h)
if n_x == 0 or "XXXXXXX" in html:
    sys.exit("ABORTA: el html no tenia el marcador del hash")


zpath = os.path.join(ENT, nombre + ".zip")
with zipfile.ZipFile(zpath, "w", zipfile.ZIP_DEFLATED) as z:
    for r in fw:
        z.writestr(r, head(r))
    for p in ("Maestro", "Esclavo", "Repetidor", "ESP32_Expansion"):
        if "01_Firmware/%s/platformio.ini" % p in fw:
            z.writestr("01_Firmware/%s/FW_HASH.txt" % p, h + "\n")
    for r in www:
        z.writestr("App_Web/" + r[len("05_Funcional/App_Semaforo/www/"):], head(r))
    z.write(apk_dst, apk)
    z.writestr(HTML.split("/")[-1], html.encode("utf-8"))

with zipfile.ZipFile(zpath) as z:
    names = z.namelist()
    malos = [n for n in names if any(x in n for x in (".pio/", "/build/", "__pycache__", "node_modules"))
             or n.endswith(".md") or n.endswith(".bin")]
    if malos:
        sys.exit("ABORTA: artefactos o .md en el zip: %s" % malos[:5])
    for r in fw:
        if z.read(r) != head(r):
            sys.exit("ABORTA: difiere de HEAD " + r)
    if hashlib.sha256(z.read(apk)).hexdigest() != sha_apk:
        sys.exit("ABORTA: APK del zip no coincide")
print("OK", zpath)
print("ficheros", len(names), "firmware", len(fw), "web", len(www), "marcadores html", n_x)
print("APK", apk, sha_apk)
print("ZIP sha256", hashlib.sha256(open(zpath, "rb").read()).hexdigest())
