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
#
# ---------------------------------------------------------------------------
# LO QUE CAMBIO EL 08/09, y son tres defectos MEDIDOS de este mismo fichero:
#
#   A) EL CONTENIDO SALIA DEL DISCO Y EL NOMBRE LLEVABA UN COMMIT. `z.write(ruta)`
#      lee el ARBOL DE TRABAJO; el nombre del zip promete `HEAD`. Con otro agente
#      -u otra sesion de Claude en el mismo arbol- editando un manual, el paquete
#      sale con documentos a medias y con el hash de un commit que no los contiene.
#      Ya paso el 05/09 con 19 documentos (skill `entregar` 2.ter). Ahora TODO lo
#      versionado sale de `git show HEAD:<ruta>` y del disco salen solo las dos
#      cosas que no estan versionadas a proposito: la APK y el acta.
#
#   B) EL LEEME LLEVABA SU SECCION 3 -"que sigue abierto"- ESCRITA A MANO, y sus
#      cinco filas estaban caducadas: daba por abierta la regresion del Modo
#      Automatico (cerrada en cobre el 04/09), pedia no cablear camara a `J16` por
#      una polaridad "en contradiccion" (medida `M3`, CERRADA el 03/09) y dudaba de
#      si el ESP32 tiene Bluetooth Clasico (`BLQ-1`, cerrado el 31/08). O sea: el
#      unico documento que se lee ANTES de tocar nada BLOQUEABA trabajo ya
#      desbloqueado y CALLABA los bloqueantes de verdad -la tarjeta Maestro muerta,
#      las entradas de campo desnudas-. Es la misma forma que el punto 3 de 2026-08-31,
#      cometida otra vez en el mismo parrafo que la denuncia.
#      Ahora esa seccion se EXTRAE de la tabla `BLOQUEANTES` de `ESTADO.md` en HEAD,
#      y si no se puede leer, el paquete no sale. Una fila menos que envejece sola.
#
#   C) LA APK SE VERIFICABA CONTRA TRES FICHEROS DE LOS TRECE que tiene la app.
#      Es la quinta trampa de la skill `entregar` 2.bis, que documenta la receta de
#      compilacion copiando 3 de 13 -y deja fuera los SIETE `js/*.js` y `sw.js`-,
#      escrita al lado de un verificador que miraba esos mismos tres. Ahora se
#      comparan TODOS los ficheros de `www/` por CRC, entrada por entrada, y se
#      exige que los tres extras de Cordova -`bluetoothSerial.js` entre ellos, que
#      es por donde la app abre el socket SPP- esten dentro.
#
# Y SE ANADE EL `.htm`: el LEEME se entrega en HTML ademas de en Markdown, porque
# un `.md` no se abre con doble clic en el equipo del funcional -se abre en el
# Bloc de notas, con las tablas rotas- y el LEEME es la pieza que garantiza que se
# lea la mitad de arriba. Los dos salen de la MISMA cadena en la MISMA corrida, asi
# que no pueden divergir; y se cuentan las filas del uno contra el otro antes de
# cerrar el zip, que es el control que la skill exige para el otro conversor.

import hashlib
import os
import re
import subprocess
import sys
import zipfile

from entrega_leeme_html import _md_a_html
import entrega_apk  # la APK y su CRC contra www/

BASE_DIR = os.path.dirname(os.path.abspath(__file__))
FUNCIONAL = os.path.join(BASE_DIR, "05_Funcional")
EVIDENCIA = os.path.join(BASE_DIR, "evidencia")

# El documento de conexiones. Hasta el 31/08 no lo enlazaba nadie y este script no
# lo metia: 81 KB de guia de cableado que el funcional no recibia.
GUIA_HTML = "Guia_Cableado_y_Pruebas_Banco.html"
GUIA_RUTA = "05_Funcional/" + GUIA_HTML

# El documento que ORDENA a los demas (skill `entregar` seccion 3.bis). Nacio sin .docx,
# asi que un paquete que solo mire .docx lo deja fuera - que es justo lo que hacia
# la version anterior con el unico documento que corrige a todos los otros.
ARQUITECTURA_MD = "17_Arquitectura_28-08_y_Decisiones_Abiertas.md"


class Aborta(Exception):
    """Falta una pieza del paquete. No se empaqueta a medias y en silencio."""


def _git_bytes(*args):
    return subprocess.check_output(("git",) + args, cwd=BASE_DIR)


def _git(*args):
    return _git_bytes(*args).decode("utf-8", "replace").strip()


def _desde_head(ruta):
    """El contenido de un fichero VERSIONADO, tal y como esta en HEAD.

    No se lee del disco a proposito: el nombre del zip lleva un commit y tiene que
    cumplirlo. Con dos sesiones escribiendo en el mismo arbol, la diferencia entre
    `git show HEAD:x` y `open(x)` es el paquete entero (skill `entregar` 2.ter).
    """
    try:
        return _git_bytes("show", "HEAD:%s" % ruta)
    except subprocess.CalledProcessError:
        raise Aborta("`%s` no esta en HEAD. O no esta versionado, o se movio sin "
                     "actualizar este script (CLAUDE.md seccion 5)" % ruta)


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

    # Un acta de `--rapido` no trae las filas de compilacion, y es la que se cuela
    # sin que nadie lo note: el resumen de arriba tiene el mismo aspecto (CLAUDE.md
    # seccion 4). Un paquete cuya acta no compilo el firmware no acredita nada del firmware.
    if len([f for f in datos["filas"] if f[1].startswith("compila ")]) < 4:
        raise Aborta("el acta %s no trae las cuatro filas `compila *`: parece de un "
                     "`--rapido`.\n        Corre `python 01_Firmware/compuerta.py` "
                     "COMPLETO -y dos veces si la anterior fue rapida-" % nombre)
    return datos


def _bloqueantes_de_estado():
    """La tabla `BLOQUEANTES` de ESTADO.md, leida de HEAD y NO reescrita aqui.

    Es la seccion 3 del LEEME -"que sigue abierto, y no se puede saltar"-. Estuvo
    escrita a mano dentro de este fichero desde el 31/08 y para el 08/09 sus cinco
    filas estaban caducadas: bloqueaba el cableado de camara que `M3` desbloqueo el
    03/09 y no nombraba la tarjeta Maestro muerta. Un LEEME que bloquea lo que ya
    esta libre y calla lo que no, se deja de leer entero.
    """
    texto = _desde_head("ESTADO.md").decode("utf-8", "replace")
    m = re.search(r"^## [^\n]*BLOQUEANTES[^\n]*$(.*?)^---\s*$", texto, re.M | re.S)
    if not m:
        raise Aborta("ESTADO.md ya no tiene una seccion `BLOQUEANTES` que termine en "
                     "`---`.\n        La seccion 3 del LEEME sale de ahi: si se movio, "
                     "se actualiza este script en el MISMO commit (CLAUDE.md seccion 5)")
    filas = [l.rstrip() for l in m.group(1).splitlines() if l.startswith("|")]
    if len(filas) < 4:
        raise Aborta("la tabla `BLOQUEANTES` de ESTADO.md salio con %d lineas: no es "
                     "una tabla. El LEEME no se completa a mano" % len(filas))
    return filas


def _apk_verificada():
    """La APK, verificada contra www/. Vive en entrega_apk.py (corte del 28/09)."""
    return entrega_apk.apk_verificada(FUNCIONAL, Aborta)


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


def _leeme(acta, hash_head, arbol_limpio, nombre_apk, bloqueantes):
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

**Esta tabla NO se escribe aqui: se copia de `ESTADO.md` en el commit `{{HEAD}}`.** La que
habia escrita a mano llego al 08/09 con sus cinco filas caducadas -bloqueaba un cableado
ya desbloqueado y callaba la tarjeta averiada-, y ese es el unico documento que alguien
lee ANTES de tocar un equipo.

{{BLOQUEANTES}}

**El orden de trabajo NO es negociable, y es ASIMETRICO:** el firmware nuevo tiene que
estar **cargado y verificado en la tarjeta** antes de que nadie enchufe nada en `J16`.
Retirado el armador de un pin, la placa lo deja fijado por su pull-down y **un pin en 0 V
no ejecuta nada**; al reves no, porque con el firmware viejo dentro ese pin sigue siendo
el boton que EJECUTA, y lo que un instalador enchufe puede pulsarlo en un equipo que esta
en la calle. **No basta con que vayan en el mismo commit: un commit no protege de un
destornillador.**

🔴 **`J16` p1 lleva 12 V crudos a un conector de senal directa al micro. Se TAPA en cada
equipo que se monte**, antes de cablear nada. Y **`J14` es una ENTRADA del micro** (3,3 V,
sin opto ni diodo): la salida de talanquera es **`J15`**. Un rele cableado a `J14` se
**desconecta antes de energizar**.

## 4. Que dice la verificacion, y que NO dice

| comprobacion | estado | detalle |
|---|---|---|
{{TABLA}}

Acta: **`{{ACTA}}`** · HEAD `{{ACTA_HEAD}}` · arbol: {{ACTA_ARBOL}}

**Resumen del acta: {{RESUMEN}}**

**Lo que esa tabla significa exactamente:** los modelos y los arneses de PC no encuentran
nada. **No dice que el firmware funcione sobre la tarjeta** — la compuerta no carga
firmware ni mueve luces. El banco del 3-4/09 encontro **tres defectos que ninguna linea de
esa tabla podia ver**, porque ninguno es una propiedad del fuente: un chip que se calienta,
un permiso de Android y una resistencia del cobre.

**Y `ABORTADO` no es `PASS`:** una comprobacion que no pudo correr no dice *nada* del
firmware. Si en la columna del medio hay un `ABORTADO`, lo que vigilaba esa fila entro sin
mirar.

## 5. Contenido

| carpeta | que es |
|---|---|
| `01_Firmware_PlatformIO/` | **Fuente** para PlatformIO: Maestro, Esclavo, Repetidor. Sin `.bin`: se compila de aqui, y asi lo que se carga es lo que se revisa |
| `02_Manuales/` | Manuales en `.docx` y `.md`. **Se lee primero `{{ARQ}}`**, que corrige a los demas y ellos todavia no lo incorporan |
| `03_Cableado/` | `{{GUIA}}` — guia de conexiones: `J17` (ESP32), `J16` (camaras), `PB6`/`PB7`, `DS3231` y el conector SWD. **Es tambien el formulario de vuelta**: se rellena y se devuelve |
| `04_App/` | **Solo la APK** `{{APK}}`. Se instala, no se compila |
| `ACTA_verificacion.txt` | El acta de la corrida citada arriba |
| `LEEME_PRIMERO.htm` · `.md` | Este documento. El `.htm` se abre con doble clic; el `.md` es la misma cadena, generada en la misma corrida |

**Todo lo versionado sale de `git show HEAD:<ruta>` con `HEAD` = `{{HEAD}}`, no del disco.**
Del disco salen solo las dos cosas que no estan versionadas a proposito: la APK y el acta.
Asi el contenido del paquete es exactamente el del commit que lleva en el nombre.

## 6. Notas para quien instale esto

- **PIN `1234` de fabrica.** Es el unico control de acceso a los cambios de modo por
  Bluetooth. Cambiarlo antes de operar en via publica.
- **El `ROJO DE EMERGENCIA` no pide PIN**, y es deliberado: parar el trafico es la accion
  segura y no debe costar teclear una clave. Por el mismo criterio entran sin PIN
  `SET_MODO:MENU` y la consulta de estado.
- **La APK es una compilacion `debug`.** Sirve para probar; no es distribucion. Su nombre
  lleva fecha y commit, y su contenido se comprobo **entrada por entrada y por CRC contra
  los {{NASSETS}} ficheros del fuente web** de este paquete antes de meterla.
- **El sufijo `_SIN_BANCO` no se quita al renombrar.** Lo quita quien la haya probado en un
  equipo, no quien la copie.
"""

    for marca, valor in (("{{AVISO_ARBOL}}", aviso_arbol),
                         ("{{BLOQUEANTES}}", "\n".join(bloqueantes)),
                         ("{{TABLA}}", tabla),
                         ("{{ACTA}}", acta["nombre"]),
                         ("{{ACTA_HEAD}}", acta["head"]),
                         ("{{ACTA_ARBOL}}", acta["arbol"]),
                         ("{{RESUMEN}}", acta["resumen"]),
                         ("{{HEAD}}", hash_head),
                         ("{{NASSETS}}", str(acta["nassets"])),
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
    nombre_apk, ruta_apk, nassets = _apk_verificada()
    acta["nassets"] = nassets
    bloqueantes = _bloqueantes_de_estado()

    huecos = _documentos_sin_docx()
    if huecos:
        raise Aborta("estos manuales no tienen .docx y el paquete los dejaria fuera sin "
                     "avisar: %s.\n        Corre `python 05_Funcional/convertir_a_word.py`"
                     % ", ".join(huecos))

    leeme_md = _leeme(acta, hash_head, arbol_limpio, nombre_apk, bloqueantes)
    leeme_htm = _md_a_html(leeme_md, "V9.0 Controladora de Semaforos - LEEME PRIMERO")

    zip_name = "Paquete_Revision_V9.0_%s_%s_SIN_BANCO.zip" % (acta["fecha"], hash_head)
    destino = os.path.join(BASE_DIR, zip_name)

    # Lo versionado, con su ruta dentro del zip. El contenido se toma de HEAD.
    versionado = []
    for rel in _ficheros_versionados("01_Firmware",
                                     excluir=(r"01_Firmware/(Camara|Semaforos|Diagnostico_LCD)/",)):
        versionado.append((rel, "01_Firmware_PlatformIO/" + os.path.relpath(rel, "01_Firmware").replace(os.sep, "/")))

    # Manuales: .docx Y .md del PRIMER NIVEL de 05_Funcional/. El conversor solo mira
    # ese nivel, asi que un documento movido a `historico/` deja de entregarse - que es
    # como se retira un entregable sin borrarlo.
    for rel in _ficheros_versionados("05_Funcional"):
        resto = rel[len("05_Funcional/"):]
        if "/" in resto or resto == "README.md":
            continue
        if resto.endswith((".docx", ".md")):
            versionado.append((rel, "02_Manuales/" + resto))

    # La guia de conexiones. Va en su propia carpeta para que no se pierda entre 38
    # manuales: es el documento que se abre CON la tarjeta delante.
    versionado.append((GUIA_RUTA, "03_Cableado/" + GUIA_HTML))

    esperado = {}
    with zipfile.ZipFile(destino, "w", zipfile.ZIP_DEFLATED) as z:
        for rel, dentro in versionado:
            datos = _desde_head(rel)
            esperado[dentro] = hashlib.md5(datos).hexdigest()
            z.writestr(dentro, datos)

        # App: SOLO la APK. El fuente de la PWA estuvo aqui hasta el 31/08 y se
        # retiro: quien recibe esto la INSTALA, no la compila. Diez ficheros de
        # fuente al lado del .apk solo invitan a abrir el que no toca.
        z.write(ruta_apk, "04_App/" + nombre_apk)

        z.write(os.path.join(EVIDENCIA, acta["nombre"]), "ACTA_verificacion.txt")
        z.writestr("LEEME_PRIMERO.htm", leeme_htm.encode("utf-8"))
        z.writestr("LEEME_PRIMERO.md", leeme_md.encode("utf-8"))

    # ---- Comprobaciones SOBRE EL ZIP, no sobre la intencion ----
    with zipfile.ZipFile(destino) as z:
        nombres = z.namelist()

        basura = [n for n in nombres
                  if re.search(r"(^|/)(\.pio|build|__pycache__|node_modules|\.gradle)/", n)]
        if basura:
            raise Aborta("el zip lleva %d artefactos de compilacion, empezando por %s"
                         % (len(basura), basura[0]))

        difieren = [d for d, md5 in esperado.items()
                    if hashlib.md5(z.read(d)).hexdigest() != md5]
        if difieren:
            raise Aborta("%d entradas del zip no coinciden con HEAD: %s"
                         % (len(difieren), ", ".join(difieren[:5])))

        md5_apk_zip = hashlib.md5(z.read("04_App/" + nombre_apk)).hexdigest()
        md5_apk_disco = hashlib.md5(open(ruta_apk, "rb").read()).hexdigest()
        if md5_apk_zip != md5_apk_disco:
            raise Aborta("la APK del zip no es la del repositorio")

        if not any(n.endswith(GUIA_HTML) for n in nombres):
            raise Aborta("el zip no lleva la guia de conexiones")

        htm = z.read("LEEME_PRIMERO.htm").decode("utf-8")
        md = z.read("LEEME_PRIMERO.md").decode("utf-8")
        for cual, texto in (("htm", htm), ("md", md)):
            if nombre_apk not in texto:
                raise Aborta("el LEEME .%s no cita el nombre exacto de la APK que le "
                             "acompana: asi es como se instala la version equivocada" % cual)

        # Que el .htm no perdio filas por el camino. Es el control que la skill
        # `entregar` exige para el conversor a Word, aplicado al conversor de aqui:
        # se cuentan las celdas del renderizado contra las del origen.
        filas_md = len([l for l in md.splitlines()
                        if l.lstrip().startswith("|") and not re.fullmatch(r"\|[\s:|-]+\|", l.strip())])
        filas_htm = htm.count("<tr>")
        if filas_md != filas_htm:
            raise Aborta("el LEEME.htm tiene %d filas de tabla y el .md tiene %d: el "
                         "renderizado perdio contenido por el camino" % (filas_htm, filas_md))

    md5 = hashlib.md5(open(destino, "rb").read()).hexdigest()
    print("[OK] %s" % zip_name)
    print("     %d ficheros | %.2f MB | md5 %s"
          % (len(nombres), os.path.getsize(destino) / 1048576.0, md5))
    print("     acta %s (HEAD %s, arbol %s)" % (acta["nombre"], acta["head"], acta["arbol"]))
    print("     APK  %s | %d ficheros web verificados por CRC" % (nombre_apk, nassets))
    print("     LEEME_PRIMERO.htm + .md | %d filas de tabla, iguales en los dos" % filas_md)
    print("     contenido versionado tomado de HEAD %s: %d entradas, 0 difieren"
          % (hash_head, len(esperado)))
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
