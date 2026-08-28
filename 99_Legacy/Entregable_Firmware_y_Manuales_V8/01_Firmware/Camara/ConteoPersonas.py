"""
Conteo de Personas y Vehículos por Zona - Cámara Hikvision AcuSense
Detecta cruces de línea y entradas a zonas configuradas en la cámara.

Requisitos:
    pip install requests

Uso:
    python hikvision_conteo.py
"""

import requests
import xml.etree.ElementTree as ET
from datetime import datetime
from requests.auth import HTTPDigestAuth

# ─── CONFIGURACIÓN ────────────────────────────────────────────────────────────
CAMERA_IP = "192.168.1.64"
USUARIO   = "admin"
PASSWORD  = "<RETIRADA - ver 01_Firmware/Camara: usar CAMARA_PASSWORD>"

# DEBUG = True  → muestra el XML crudo (útil para diagnóstico)
# DEBUG = False → solo muestra el panel de conteo limpio
DEBUG = True
# ──────────────────────────────────────────────────────────────────────────────

URL_EVENTOS = f"http://{CAMERA_IP}/ISAPI/Event/notification/alertStream"

conteo = {
    "personas":  0,
    "vehiculos": 0,
    "otros":     0,
    "inicio":    datetime.now(),
    "ultimo":    None,
}

# Tipos de eventos inteligentes que nos interesan
EVENTOS_SMART = (
    "fielddetection",       # Detección de entrada/intrusión en zona
    "linedetection",        # Cruce de línea
    "linecrossing",         # Cruce de línea (variante)
    "intrusion",            # Intrusión
    "loitering",            # Merodeo
    "vmd",                  # Detección de movimiento con AcuSense
    "targetdetection",      # Detección de objetivo
    "regionentrance",       # Entrada a región
    "regionexiting",        # Salida de región
)

# Palabras clave que indican vehículo en el XML
TIPOS_VEHICULO = ("vehicle", "car", "truck", "bus", "motorbike", "bicycle", "vehículo")

# Palabras clave que indican persona en el XML
TIPOS_PERSONA = ("human", "person", "pedestrian", "persona")


def find_tag(root, tag_name):
    """Busca un tag XML ignorando namespaces, retorna el primero encontrado."""
    for elem in root.iter():
        local = elem.tag.split("}")[-1] if "}" in elem.tag else elem.tag
        if local.lower() == tag_name.lower():
            return elem
    return None


def find_all_tags(root, tag_name):
    """Busca todos los tags XML con ese nombre ignorando namespaces."""
    result = []
    for elem in root.iter():
        local = elem.tag.split("}")[-1] if "}" in elem.tag else elem.tag
        if local.lower() == tag_name.lower():
            result.append(elem)
    return result


def clasificar_target(tipo_texto):
    """Clasifica un texto como vehicle, human u otro."""
    if not tipo_texto:
        return "otro"
    t = tipo_texto.strip().lower()
    if any(v in t for v in TIPOS_VEHICULO):
        return "vehicle"
    elif any(p in t for p in TIPOS_PERSONA):
        return "human"
    return "otro"


def parsear_evento(xml_texto):
    """
    Parsea el XML del evento ISAPI.
    Retorna: (categoria, tipo_evento, tipo_objeto)
      categoria: 'vehicle' | 'human' | 'otro' | None
      tipo_evento: string con el eventType
      tipo_objeto: string con el tipo de objeto detectado
    """
    try:
        xml_texto = xml_texto.strip()
        if not xml_texto or not xml_texto.startswith("<"):
            return None, None, None

        root = ET.fromstring(xml_texto)

        # Obtener eventType
        evento_elem = find_tag(root, "eventType")
        tipo_evento = evento_elem.text.strip().lower() if (evento_elem is not None and evento_elem.text) else "desconocido"

        # Filtrar solo eventos smart relevantes
        # (ignorar videoloss, illegalAccess, etc.)
        es_smart = any(s in tipo_evento for s in EVENTOS_SMART)
        if not es_smart:
            return "ignorar", tipo_evento, None

        # Buscar clasificación del objeto en múltiples campos posibles
        campos_target = [
            "targetType", "detectionTarget", "DetectionTarget",
            "targetDetection", "objectType", "type"
        ]

        for campo in campos_target:
            elems = find_all_tags(root, campo)
            for elem in elems:
                if elem.text:
                    cat = clasificar_target(elem.text)
                    if cat != "otro":
                        return cat, tipo_evento, elem.text.strip()

        # Buscar dentro de <target> o <targets>
        for contenedor in ["targets", "target", "detectionRegionList"]:
            cont_elem = find_tag(root, contenedor)
            if cont_elem is not None:
                for campo in ["type", "objectType", "targetType"]:
                    elems = find_all_tags(cont_elem, campo)
                    for elem in elems:
                        if elem.text:
                            cat = clasificar_target(elem.text)
                            return cat, tipo_evento, elem.text.strip()

        # Si es evento smart pero no tiene clasificación específica
        return "otro", tipo_evento, "sin_clasificar"

    except ET.ParseError as e:
        if DEBUG:
            print(f"[XML Parse Error] {e}")
        return None, None, None


def imprimir_panel(ultimo_evento=None):
    """Imprime el panel de conteo en consola."""
    duracion = datetime.now() - conteo["inicio"]
    h = int(duracion.total_seconds() // 3600)
    m = int((duracion.total_seconds() % 3600) // 60)
    s = int(duracion.total_seconds() % 60)

    print("=" * 52)
    print("    CONTEO POR ZONA - Hikvision AcuSense")
    print("=" * 52)
    print(f"  Camara  : {CAMERA_IP}")
    print(f"  Tiempo  : {h:02d}:{m:02d}:{s:02d}")
    if ultimo_evento:
        print(f"  Ultimo  : {ultimo_evento}")
    print("-" * 52)
    print(f"  🚶 Personas detectadas  : {conteo['personas']:>6}")
    print(f"  🚗 Vehiculos detectados : {conteo['vehiculos']:>6}")
    print(f"  📊 Total                : {conteo['personas'] + conteo['vehiculos']:>6}")
    print("=" * 52)
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
                return
            elif resp.status_code != 200:
                print(f"ERROR de conexion: HTTP {resp.status_code}")
                return

            print("Conectado OK. Escuchando eventos en la zona configurada...\n")
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

                    categoria, tipo_evento, tipo_objeto = parsear_evento(xml_bloque)

                    if categoria == "ignorar" or categoria is None:
                        if DEBUG:
                            print(f"[ignorado] eventType: {tipo_evento}\n")
                        continue

                    ts = datetime.now().strftime("%H:%M:%S")

                    if categoria == "human":
                        conteo["personas"] += 1
                        ultimo = f"[{ts}] PERSONA ({tipo_evento})"
                        print(f">>> PERSONA detectada en zona | evento: {tipo_evento} | objeto: {tipo_objeto}")
                        imprimir_panel(ultimo)

                    elif categoria == "vehicle":
                        conteo["vehiculos"] += 1
                        ultimo = f"[{ts}] VEHICULO ({tipo_evento})"
                        print(f">>> VEHICULO detectado en zona | evento: {tipo_evento} | objeto: {tipo_objeto}")
                        imprimir_panel(ultimo)

                    else:
                        conteo["otros"] += 1
                        if DEBUG:
                            print(f"[OTRO] evento: {tipo_evento} | objeto: {tipo_objeto}\n")

    except requests.exceptions.ConnectionError:
        print(f"\nERROR: No se pudo conectar a {CAMERA_IP}.")
        print("Verifica que la camara este encendida y en la misma red.")
    except requests.exceptions.Timeout:
        print("\nTimeout. La camara no respondio a tiempo.")
    except KeyboardInterrupt:
        print("\n\nDetenido por el usuario.")
        print("\nRESUMEN FINAL:")
        print(f"  Personas  : {conteo['personas']}")
        print(f"  Vehiculos : {conteo['vehiculos']}")
        print(f"  Total     : {conteo['personas'] + conteo['vehiculos']}")


if __name__ == "__main__":
    escuchar_eventos()