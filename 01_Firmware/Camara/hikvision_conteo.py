"""
Conteo de Vehículos y Personas - Cámara Hikvision AcuSense
Conecta via ISAPI HTTP y escucha eventos de detección en tiempo real.

Requisitos:
    pip install requests

Uso:
    python hikvision_conteo.py
"""

import os
import sys
import requests
import xml.etree.ElementTree as ET
from datetime import datetime
from requests.auth import HTTPDigestAuth

# ─── CONFIGURACIÓN ────────────────────────────────────────────────────────────
# La contraseña NO va escrita aquí. Este archivo está en el repositorio, y una
# credencial en el repositorio es una credencial publicada: el 28/07/2026 se
# subió la de la cámara y quedó expuesta hasta el 01/08/2026.
#
# Definir antes de ejecutar (PowerShell):
#     $env:CAMARA_IP = "192.168.1.64"
#     $env:CAMARA_USUARIO = "admin"
#     $env:CAMARA_PASSWORD = "la-contrasena-nueva"
CAMERA_IP = os.environ.get("CAMARA_IP", "192.168.1.64")
USUARIO   = os.environ.get("CAMARA_USUARIO", "admin")
PASSWORD  = os.environ.get("CAMARA_PASSWORD")

if not PASSWORD:
    sys.exit(
        "Falta la variable de entorno CAMARA_PASSWORD.\n"
        "  PowerShell:  $env:CAMARA_PASSWORD = \"...\"\n"
        "  Linux/macOS: export CAMARA_PASSWORD=\"...\""
    )

# DEBUG = True  → muestra el XML crudo que manda la cámara (útil para diagnóstico)
# DEBUG = False → muestra solo el panel de conteo limpio
DEBUG = True
# ──────────────────────────────────────────────────────────────────────────────

URL_EVENTOS = f"http://{CAMERA_IP}/ISAPI/Event/notification/alertStream"

conteo = {
    "vehiculos": 0,
    "personas":  0,
    "otros":     0,
    "inicio":    datetime.now()
}


def find_tag(root, tag_name):
    """Busca un tag XML ignorando namespaces."""
    for elem in root.iter():
        local = elem.tag.split("}")[-1] if "}" in elem.tag else elem.tag
        if local.lower() == tag_name.lower():
            return elem
    return None


def parsear_evento(xml_texto):
    """
    Parsea el XML del evento ISAPI.
    Retorna: ('vehicle' | 'human' | 'otro' | None, tipo_raw)
    """
    try:
        xml_texto = xml_texto.strip()
        if not xml_texto or not xml_texto.startswith("<"):
            return None, None

        root = ET.fromstring(xml_texto)

        # 1. Clasificación AcuSense: targetType / detectionTarget
        for campo in ["targetType", "detectionTarget", "DetectionTarget", "targetDetection"]:
            elem = find_tag(root, campo)
            if elem is not None and elem.text:
                tipo = elem.text.strip().lower()
                if tipo in ("vehicle", "car", "truck", "bus", "motorbike", "bicycle"):
                    return "vehicle", tipo
                elif tipo in ("human", "person", "pedestrian"):
                    return "human", tipo
                else:
                    return "otro", tipo

        # 2. Dentro de <target><type>
        target_elem = find_tag(root, "target")
        if target_elem is not None:
            type_elem = find_tag(target_elem, "type")
            if type_elem is not None and type_elem.text:
                tipo = type_elem.text.strip().lower()
                if tipo in ("vehicle", "car", "truck", "bus", "motorbike"):
                    return "vehicle", tipo
                elif tipo in ("human", "person", "pedestrian"):
                    return "human", tipo
                return "otro", tipo

        # 3. Fallback: eventType
        evento_elem = find_tag(root, "eventType")
        if evento_elem is not None and evento_elem.text:
            return "otro", evento_elem.text.strip()

        return "otro", "desconocido"

    except ET.ParseError as e:
        if DEBUG:
            print(f"[XML Parse Error] {e}")
        return None, None


def imprimir_panel():
    """Imprime el panel de conteo en consola."""
    duracion = datetime.now() - conteo["inicio"]
    h = int(duracion.total_seconds() // 3600)
    m = int((duracion.total_seconds() % 3600) // 60)
    s = int(duracion.total_seconds() % 60)

    print("=" * 50)
    print("   CONTEO EN TIEMPO REAL - Hikvision AcuSense")
    print("=" * 50)
    print(f"  Camara : {CAMERA_IP}")
    print(f"  Tiempo : {h:02d}:{m:02d}:{s:02d}")
    print("-" * 50)
    print(f"  Vehiculos detectados : {conteo['vehiculos']:>6}")
    print(f"  Personas detectadas  : {conteo['personas']:>6}")
    print(f"  Otros eventos        : {conteo['otros']:>6}")
    print(f"  Total                : {conteo['vehiculos'] + conteo['personas'] + conteo['otros']:>6}")
    print("=" * 50)
    print("  Presiona Ctrl+C para detener\n")


def escuchar_eventos():
    """Conecta al stream ISAPI y procesa eventos en tiempo real."""
    print(f"\nConectando a {CAMERA_IP}...")

    auth    = HTTPDigestAuth(USUARIO, PASSWORD)
    headers = {"Accept": "multipart/x-mixed-replace"}

    try:
        with requests.get(
            URL_EVENTOS,
            auth=auth,
            headers=headers,
            stream=True,
            timeout=60
        ) as resp:

            if resp.status_code == 401:
                print("ERROR: Usuario o contrasena incorrectos.")
                print("Si la camara esta bloqueada, espera unos minutos e intenta de nuevo.")
                return
            elif resp.status_code != 200:
                print(f"ERROR de conexion: HTTP {resp.status_code}")
                return

            print("Conectado OK. Escuchando eventos...\n")
            imprimir_panel()

            buffer = ""

            for chunk in resp.iter_content(chunk_size=1024, decode_unicode=True):
                if not chunk:
                    continue

                if isinstance(chunk, bytes):
                    chunk = chunk.decode("utf-8", errors="ignore")

                buffer += chunk

                while "<EventNotificationAlert" in buffer:
                    inicio = buffer.find("<EventNotificationAlert")
                    fin    = buffer.find("</EventNotificationAlert>", inicio)

                    if fin == -1:
                        break

                    xml_bloque = buffer[inicio: fin + len("</EventNotificationAlert>")]
                    buffer     = buffer[fin + len("</EventNotificationAlert>"):]

                    if DEBUG:
                        print("\n--- XML RECIBIDO ---")
                        print(xml_bloque)
                        print("--------------------\n")

                    categoria, tipo_raw = parsear_evento(xml_bloque)

                    if categoria == "vehicle":
                        conteo["vehiculos"] += 1
                        print(f">>> VEHICULO detectado (tipo: {tipo_raw})")
                        imprimir_panel()

                    elif categoria == "human":
                        conteo["personas"] += 1
                        print(f">>> PERSONA detectada (tipo: {tipo_raw})")
                        imprimir_panel()

                    elif categoria == "otro":
                        conteo["otros"] += 1
                        if DEBUG:
                            print(f">>> OTRO evento (tipo: {tipo_raw})")
                        imprimir_panel()

    except requests.exceptions.ConnectionError:
        print(f"\nERROR: No se pudo conectar a {CAMERA_IP}.")
        print("Verifica que la camara este encendida y en la misma red.")
    except requests.exceptions.Timeout:
        print("\nTimeout. La camara no respondio a tiempo.")
    except KeyboardInterrupt:
        print("\n\nDetenido por el usuario.")
        print("\nRESUMEN FINAL:")
        print(f"  Vehiculos : {conteo['vehiculos']}")
        print(f"  Personas  : {conteo['personas']}")
        print(f"  Otros     : {conteo['otros']}")
        print(f"  Total     : {conteo['vehiculos'] + conteo['personas'] + conteo['otros']}")


if __name__ == "__main__":
    escuchar_eventos()