# ===== generar_entrega_v9_0.py =====
#
# Arma el paquete que revisa el funcional. NO produce una entrega de campo: eso
# exige banco pasado, y no lo esta (skill `entregar` seccion 1).
#
# POR QUE ESTE FICHERO SE REESCRIBIO ENTERO EL 31/08.
#
# La version anterior tenia TRES saltos silenciosos, todos con la misma forma
# `if os.path.exists(x): meter(x)` -y sin `else`-:
#
#   1. el acta estaba fijada a `2026-08-26_compuerta.txt`, cinco corridas atras;
#   2. la APK, a `IOT_VIAL_Semaforos_v8.9.apk`, que NO EXISTE en el disco desde
#      que la skill impuso poner fecha y commit en el nombre;
#   3. y el LEEME afirmaba a mano "11/11 PASS", "12 manuales", "camaras PB0/PB8",
#      "Bluetooth PA9/PA10" y "el codigo de la app no ha cambiado en V9.0".
#
# Ninguna de esas cinco frases era cierta el 31/08, y la ultima es falsa por 629
# lineas de `app.js`. Un fichero que falta se saltaba sin decir nada y el paquete
# salia con aspecto de completo: es CLAUDE.md seccion 3 -un hueco no grita- dentro del
# unico artefacto que alguien lee ANTES de instalar un semaforo.
#
# LA REGLA QUE SIGUE ESTE SCRIPT: cada entrada del paquete o se mete, o ABORTA
# diciendo por que. No hay tercera opcion. Y ninguna cifra del LEEME se escribe
# aqui: se lee del acta, como manda CLAUDE.md.

import hashlib
import os
import re
import subprocess
import sys
import zipfile

BASE_DIR = os.path.dirname(os.path.abspath(__file__))
FUNCIONAL = os.path.join(BASE_DIR, "05_Funcional")
EVIDENCIA = os.path.join(BASE_DIR, "evidencia")

# El documento de conexiones. Hasta el 31/08 no lo enlazaba nadie y este script no
# lo metia: 81 KB de guia de cableado que el funcional no recibia.
GUIA_HTML = "Guia_Cableado_y_Pruebas_Banco.html"

# El documento que ORDENA a los demas (skill `entregar` seccion 3.bis). Nacio sin .docx,
# asi que un paquete que solo mire .docx lo deja fuera - que es justo lo que hacia
# la version anterior con el unico documento que corrige a todos los otros.
ARQUITECTURA_MD = "17_Arquitectura_28-08_y_Decisiones_Abiertas.md"

# Las tres copias del fuente web que tienen que ir dentro de la APK.
ASSETS_APP = ("app.js", "index.html", "style.css")


class Aborta(Exception):
    """Falta una pieza del paquete. No se empaqueta a medias y en silencio."""


def _git(*args):
    return subprocess.check_output(("git",) + args, cwd=BASE_DIR).decode("utf-8", "replace").strip()


def _cabecera_del_arbol():
    """Hash corto de HEAD y si el arbol esta limpio. Va en el nombre del zip.

    Un paquete cuyo nombre no dice de que commit sale obliga a fiarse de quien lo
    mando; el 27/08 hubo tres ficheros con dos binarios y nadie sabia cual era cual.
    """
    hash_head = _git("rev-parse", "--short", "HEAD")
    limpio = _git("status", "--porcelain") == ""
    return hash_head, limpio


def _acta_mas_reciente():
    """La ultima acta de evidencia/. ABORTA si no hay ninguna."""
    actas = sorted(f for f in os.listdir(EVIDENCIA) if re.fullmatch(r"\d{4}-\d{2}-\d{2}_compuerta\.txt", f))
    if not actas:
        raise Aborta("no hay ninguna acta en evidencia/. Un paquete sin acta no se "
                     "puede verificar: corre `python 01_Firmware/compuerta.py` antes")
    nombre = actas[-1]
    texto = open(os.path.join(EVIDENCIA, nombre), encoding="utf-8", errors="replace").read()
    datos = {"nombre": nombre, "texto": texto}

    for clave, patron in (("head", r"^HEAD\s*:\s*(\S+)"),
                          ("arbol", r"^Arbol\s*:\s*(.+)$"),
                          ("fecha", r"^Fecha\s*:\s*(\S+)"),
                          ("resumen", r"^\s*RESUMEN:\s*(.+)$")):
        m = re.search(patron, texto, re.M)
        if not m:
            raise Aborta("el acta %s no deja leer su %s. Copiar sus cifras a mano es "
                         "justo lo que este script vino a impedir" % (nombre, clave))
        datos[clave] = m.group(1).strip()

    # Las lineas de resultado, para que el LEEME las cite sin reescribirlas.
    datos["filas"] = re.findall(r"^ {2}(PASS|FALLA|ABORTADO)\s+(\S.*?)\s{2,}(\S.*)$", texto, re.M)
    if not datos["filas"]:
        raise Aborta("el acta %s no trae lineas de resultado legibles" % nombre)
    return datos


def _crc_de(ruta):
    datos = open(ruta, "rb").read()
    return zipfile.crc32(datos) & 0xFFFFFFFF if hasattr(zipfile, "crc32") else None


def _apk_verificada():
    """La APK mas reciente, Y con su contenido comprobado contra el repositorio.

    No basta con encontrar un .apk: la skill `entregar` seccion 2.bis lo deja medido -dos
    APK del mismo contenido NO tienen el mismo md5, asi que la comparacion util es
    por CRC de las entradas-. Si el fuente web del repositorio ha cambiado despues
    de compilarla, la APK del disco NO lleva lo que su nombre promete y el paquete
    NO sale: se recompila. Es la diferencia entre entregar una version y entregar
    un binario parecido.
    """
    candidatas = sorted(f for f in os.listdir(FUNCIONAL)
                        if f.startswith("IOT_VIAL_Semaforos_") and f.endswith(".apk"))
    if not candidatas:
        raise Aborta("no hay ninguna APK en 05_Funcional/. Compilala con la receta de "
                     "la skill `entregar` seccion 2.bis; el paquete no sale sin ella")
    nombre = candidatas[-1]
    ruta = os.path.join(FUNCIONAL, nombre)

    dentro = {}
    with zipfile.ZipFile(ruta) as z:
        for info in z.infolist():
            base = os.path.basename(info.filename)
            if info.filename.startswith("assets/public/") and base in ASSETS_APP:
                dentro[base] = info.CRC

    faltan = [a for a in ASSETS_APP if a not in dentro]
    if faltan:
        raise Aborta("la APK %s no trae %s en assets/public/: no es una APK de esta app"
                     % (nombre, ", ".join(faltan)))

    desfasados = []
    for asset in ASSETS_APP:
        repo = os.path.join(FUNCIONAL, "App_Semaforo", "www", asset)
        if _crc_de(repo) != dentro[asset]:
            desfasados.append(asset)
    if desfasados:
        raise Aborta(
            "la APK %s NO lleva el fuente que hay hoy en el repositorio: difieren %s.\n"
            "        Recompilala (skill `entregar` seccion 2.bis) y renombrala con la fecha y el\n"
            "        commit. Meterla asi entregaria una app sin los botones que el firmware\n"
            "        ya atiende, con un nombre que promete lo contrario."
            % (nombre, ", ".join(desfasados)))
    return nombre, ruta


def _ficheros_versionados(prefijo, excluir=()):
    """Solo lo que git conoce: asi no se cuela .pio/, build/ ni __pycache__.

    Una lista de exclusiones siempre se escapa algo; partir de `git ls-files` no.
    """
    salida = _git("ls-files", prefijo).splitlines()
    return [f for f in salida if not any(re.match(pat, f) for pat in excluir)]


def _documentos_sin_docx():
    """Los .md numerados de 05_Funcional/ que no tienen su .docx al lado.

    El conversor no es opcional: el paquete lleva .docx, y un manual sin el sale
    del paquete sin que nadie lo eche de menos. Le paso al 17, que es el documento
    que corrige a todos los demas.
    """
    huecos = []
    for f in sorted(os.listdir(FUNCIONAL)):
        if f.endswith(".md") and re.match(r"\d", f):
            if not os.path.exists(os.path.join(FUNCIONAL, f[:-3] + ".docx")):
                huecos.append(f)
    return huecos


def _leeme(acta, hash_head, arbol_limpio, nombre_apk):
    """El LEEME, con las cifras LEIDAS del acta.

    Orden obligatorio (skill `entregar` seccion 4): que corre en campo, si ha pasado banco,
    que sigue roto, y solo despues las novedades. No abre con la cifra en verde: un
    LEEME que empieza en "100% PASS" se lee como un permiso y nadie llega a la linea
    que dice que no ha pasado banco.
    """
    # La barra se escapa: varios detalles del acta la llevan dentro -"32 PASS | 0
    # FALLAS"- y sin escapar parten las columnas de la tabla. Un LEEME que se
    # renderiza mal es un LEEME que no se lee entero.
    tabla = "\n".join("| %s | %s | %s |" % (n, e, d.replace("|", "\\|"))
                      for e, n, d in acta["filas"])
    aviso_arbol = ("" if arbol_limpio else
                   "\n> ⚠️ **El arbol de trabajo tenia cambios sin commitear al generar este "
                   "paquete.** El hash de abajo NO describe exactamente lo que va dentro.\n")
    # Marcas NOMBRADAS, no %s posicionales. La primera version de esta funcion
    # llevaba 9 argumentos para 8 huecos y reventaba al generar: un texto largo que
    # alguien va a editar no puede depender de contar posiciones, y el LEEME es el
    # unico artefacto del paquete que se lee ANTES de instalar nada.
    plantilla = """# 📦 V9.0 — Controladora de Semaforos Moviles · PAQUETE DE REVISION

## 1. Que corre en campo hoy

**La V8.4 (`e303485`), certificada el 31/07/2026. Este paquete NO es eso y no la sustituye.**

## 2. ¿Ha pasado banco?

# 🛑 NO.

Nada de lo que va aqui se ha cargado en una tarjeta y visto mover luces. Todo lo que
sigue se midio **sobre ficheros** -el `.cpp`, el `.h`, el `.kicad_pcb`, el `.elf`- y un
fichero dice lo que alguien escribio, no lo que se fabrico.

**Cargar esto en un equipo de calle exige antes una sesion de banco, con acta.**
{{AVISO_ARBOL}}
## 3. Que sigue abierto, y no se puede saltar

| | |
|---|---|
| **Regresion del Modo Automatico** | En banco no arranca el ciclo. No es radio ni boton |
| **Cristal `Y2`** | No oscila en la tarjeta medida. Falta diagnosticar el de la segunda |
| **Polaridad de `J16`** | En contradiccion medida. 🛑 **NO se cablea camara a `J16`** hasta medir con multimetro. Al reves da demanda permanente o demanda que nunca llega: las dos son de calle |
| **`J16` p1 lleva 12 V crudos** | Unico conector de senal con 12 V sin opto ni clamp. Se tapa antes de cablear |
| **Modulo ESP32 sin identificar** | Si no es Bluetooth Clasico no hay SPP y la app no conecta |

**El orden de trabajo NO es negociable:** el firmware nuevo tiene que estar **cargado y
verificado en la tarjeta** antes de que nadie enchufe nada en `J16`. Con el firmware
viejo dentro, `PB14` sigue siendo *Aceptar* leido activo en BAJO, y cualquier cosa que se
enchufe ahi puede ejecutar un comando en un equipo que esta en la calle.

## 4. Que dice la verificacion, y que NO dice

| comprobacion | estado | detalle |
|---|---|---|
{{TABLA}}

Acta: **`{{ACTA}}`** · HEAD `{{ACTA_HEAD}}` · arbol: {{ACTA_ARBOL}}

**Resumen del acta: {{RESUMEN}}**

**Lo que esa tabla significa exactamente:** los modelos y los arneses de PC no encuentran
nada. **No dice que el firmware funcione sobre la tarjeta** — la compuerta no carga
firmware ni mueve luces.

## 5. Contenido

| carpeta | que es |
|---|---|
| `01_Firmware_PlatformIO/` | **Fuente** para PlatformIO: Maestro, Esclavo, Repetidor. Sin `.bin`: se compila de aqui, y asi lo que se carga es lo que se revisa |
| `02_Manuales/` | Manuales en `.docx` y `.md`. **Se lee primero `{{ARQ}}`**, que corrige a los demas y ellos todavia no lo incorporan |
| `03_Cableado/` | `{{GUIA}}` — guia de conexiones: `J17` (ESP32), `J16` (camaras), `PB6`/`PB7`, `DS3231` y el conector SWD |
| `04_App/` | **Solo la APK** `{{APK}}`. Se instala, no se compila |
| `ACTA_verificacion.txt` | El acta de la corrida citada arriba |

## 6. Notas para quien instale esto

- **PIN `1234` de fabrica.** Es el unico control de acceso a los cambios de modo por
  Bluetooth. Cambiarlo antes de operar en via publica.
- **El `ROJO DE EMERGENCIA` no pide PIN**, y es deliberado: parar el trafico es la accion
  segura y no debe costar teclear una clave.
- **La APK es una compilacion `debug`.** Sirve para probar; no es distribucion. Su nombre
  lleva fecha y commit, y su contenido se comprobo entrada por entrada contra el fuente
  de este paquete antes de meterla.
"""

    for marca, valor in (("{{AVISO_ARBOL}}", aviso_arbol),
                         ("{{TABLA}}", tabla),
                         ("{{ACTA}}", acta["nombre"]),
                         ("{{ACTA_HEAD}}", acta["head"]),
                         ("{{ACTA_ARBOL}}", acta["arbol"]),
                         ("{{RESUMEN}}", acta["resumen"]),
                         ("{{ARQ}}", ARQUITECTURA_MD),
                         ("{{GUIA}}", GUIA_HTML),
                         ("{{APK}}", nombre_apk)):
        plantilla = plantilla.replace(marca, valor)

    # Una marca sin rellenar es un LEEME con un hueco donde iba una cifra medida.
    # Se aborta: es preferible no tener paquete a tener uno que no dice de que acta sale.
    sobrantes = re.findall(r"\{\{[A-Z_]+\}\}", plantilla)
    if sobrantes:
        raise Aborta("el LEEME quedo con marcas sin rellenar: %s" % ", ".join(sobrantes))
    return plantilla


def crear_paquete():
    hash_head, arbol_limpio = _cabecera_del_arbol()
    acta = _acta_mas_reciente()
    nombre_apk, ruta_apk = _apk_verificada()

    huecos = _documentos_sin_docx()
    if huecos:
        raise Aborta("estos manuales no tienen .docx y el paquete los dejaria fuera sin "
                     "avisar: %s.\n        Corre `python 05_Funcional/convertir_a_word.py`"
                     % ", ".join(huecos))

    ruta_guia = os.path.join(FUNCIONAL, GUIA_HTML)
    if not os.path.exists(ruta_guia):
        raise Aborta("falta %s, la guia de conexiones que el funcional usa en el banco" % GUIA_HTML)

    zip_name = "Paquete_Revision_V9.0_%s_%s_SIN_BANCO.zip" % (acta["fecha"], hash_head)
    destino = os.path.join(BASE_DIR, zip_name)
    metidos = 0

    with zipfile.ZipFile(destino, "w", zipfile.ZIP_DEFLATED) as z:
        # 1. Firmware: FUENTE, y solo lo versionado. Fuera las carpetas de ruido.
        for rel in _ficheros_versionados("01_Firmware",
                                         excluir=(r"01_Firmware/(Camara|Semaforos|Diagnostico_LCD)/",)):
            z.write(os.path.join(BASE_DIR, rel),
                    os.path.join("01_Firmware_PlatformIO", os.path.relpath(rel, "01_Firmware")))
            metidos += 1

        # 2. Manuales: .docx Y .md. El 17 va primero por nombre y lo dice el LEEME.
        for f in sorted(os.listdir(FUNCIONAL)):
            if f.endswith((".docx", ".md")) and f != "README.md":
                z.write(os.path.join(FUNCIONAL, f), os.path.join("02_Manuales", f))
                metidos += 1

        # 3. La guia de conexiones. Va en su propia carpeta para que no se pierda
        #    entre 38 manuales: es el documento que se abre CON la tarjeta delante.
        z.write(ruta_guia, os.path.join("03_Cableado", GUIA_HTML))
        metidos += 1

        # 4. App: SOLO la APK. El fuente de la PWA estuvo aqui hasta el 31/08 y se
        #    retiro: quien recibe esto la INSTALA, no la compila. Diez ficheros de
        #    fuente al lado del .apk solo invitan a abrir el que no toca.
        z.write(ruta_apk, os.path.join("04_App", nombre_apk))
        metidos += 1

        # 5. El acta y el LEEME.
        z.write(os.path.join(EVIDENCIA, acta["nombre"]), "ACTA_verificacion.txt")
        z.writestr("LEEME_PRIMERO.md", _leeme(acta, hash_head, arbol_limpio, nombre_apk))
        metidos += 2

    # ---- Comprobaciones SOBRE EL ZIP, no sobre la intencion ----
    with zipfile.ZipFile(destino) as z:
        nombres = z.namelist()
    basura = [n for n in nombres if re.search(r"(^|/)(\.pio|build|__pycache__|node_modules|\.gradle)/", n)]
    if basura:
        raise Aborta("el zip lleva %d artefactos de compilacion, empezando por %s"
                     % (len(basura), basura[0]))
    if not any(n.endswith(GUIA_HTML) for n in nombres):
        raise Aborta("el zip no lleva la guia de conexiones")
    leeme = zipfile.ZipFile(destino).read("LEEME_PRIMERO.md").decode("utf-8")
    if nombre_apk not in leeme:
        raise Aborta("el LEEME no cita el nombre exacto de la APK que le acompana: asi "
                     "es como se instala la version equivocada")

    md5 = hashlib.md5(open(destino, "rb").read()).hexdigest()
    print("[OK] %s" % zip_name)
    print("     %d ficheros | %.2f MB | md5 %s" % (metidos, os.path.getsize(destino) / 1048576.0, md5))
    print("     acta %s (HEAD %s, arbol %s) | APK %s"
          % (acta["nombre"], acta["head"], acta["arbol"], nombre_apk))
    if not arbol_limpio:
        print("     AVISO: arbol con cambios sin commitear; el LEEME lo dice dentro")
    print("\n     Esto NO ha pasado banco. No es una entrega de campo.")


if __name__ == "__main__":
    try:
        crear_paquete()
    except Aborta as e:
        print("\n[ABORTADO] %s\n" % e)
        print("  ABORTADO no es PASS: el paquete NO se ha creado. Un fichero que falta")
        print("  se saltaba en silencio en la version anterior de este script, y el")
        print("  paquete salia con aspecto de completo.")
        sys.exit(2)
