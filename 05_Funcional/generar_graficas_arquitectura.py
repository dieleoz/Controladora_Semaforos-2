#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
generar_graficas_arquitectura.py
Genera diagramas e infografías de alta resolución (PNG) para el manual de la App Semafórica V9.0.
"""

import os
from PIL import Image, ImageDraw, ImageFont

OUTPUT_DIR = os.path.join(os.path.dirname(__file__), "graficas")
EVIDENCIA_DIR = os.path.join(os.path.dirname(__file__), "..", "evidencia")

os.makedirs(OUTPUT_DIR, exist_ok=True)
os.makedirs(EVIDENCIA_DIR, exist_ok=True)

# Paleta de Colores
BG_COLOR = (11, 17, 30)        # #0B111E Deep Slate
CARD_BG = (20, 29, 48)         # #141D30
TEXT_WHITE = (248, 250, 252)   # #F8FAFC
TEXT_MUTED = (148, 163, 184)   # #94A3B8
GREEN = (0, 230, 118)          # #00E676
AMBER = (255, 179, 0)          # #FFB300
RED = (255, 30, 68)            # #FF1E44
CYAN = (0, 240, 255)           # #00F0FF

def get_font(size, bold=False):
    font_paths = [
        r"C:\Windows\Fonts\segoeuib.ttf" if bold else r"C:\Windows\Fonts\segoeui.ttf",
        r"C:\Windows\Fonts\arialbd.ttf" if bold else r"C:\Windows\Fonts\arial.ttf",
    ]
    for p in font_paths:
        if os.path.exists(p):
            try:
                return ImageFont.truetype(p, size)
            except Exception:
                pass
    return ImageFont.load_default()

# ==============================================================================
# 1. DIAGRAMA DE ARQUITECTURA DE 2 ROLES (OPERARIO VS TECNICO)
# ==============================================================================
def generar_grafica_roles():
    width, height = 1200, 680
    img = Image.new("RGB", (width, height), BG_COLOR)
    draw = ImageDraw.Draw(img)

    font_title = get_font(32, bold=True)
    font_subtitle = get_font(18, bold=False)
    font_h2 = get_font(22, bold=True)
    font_body = get_font(15, bold=False)
    font_bold = get_font(16, bold=True)

    draw.text((40, 30), "ARQUITECTURA DE 2 ROLES - APP IOT-VIAL V9.0", font=font_title, fill=CYAN)
    draw.text((40, 75), "Separacion clara entre Operacion Tactica en Campo (1 toque) y Ajustes Criticos Protegidos por PIN", font=font_subtitle, fill=TEXT_MUTED)

    # Card Izquierda: MODO OPERARIO
    draw.rounded_rectangle([(40, 130), (580, 630)], radius=16, fill=CARD_BG, outline=GREEN, width=2)
    draw.text((70, 155), "MODO OPERARIO (POR DEFECTO)", font=font_h2, fill=GREEN)
    draw.text((70, 190), "Disenado para el paletero / operario de obra en campo.", font=font_body, fill=TEXT_MUTED)
    draw.text((70, 215), "- Cero contrasenas para gobernar el trafico.", font=font_body, fill=TEXT_WHITE)
    draw.text((70, 240), "- Reemplaza el mando fisico de reles con 4 botones grandes:", font=font_body, fill=TEXT_WHITE)

    # 4 Botones del Operario
    btns = [
        ("1. AUTOMATICO", "Ciclo estandar autonomo programado", GREEN, (70, 280, 550, 345)),
        ("2. DAR PASO", "Alterna sentido con despeje de seguridad", CYAN, (70, 360, 550, 425)),
        ("3. AMBAR PRECAUCION", "Intermitencia vial 1Hz (lluvia / noche)", AMBER, (70, 440, 550, 505)),
        ("4. ROJO TOTAL", "Detiene el trafico en ambos sentidos", RED, (70, 520, 550, 585)),
    ]
    for title, desc, color, coords in btns:
        draw.rounded_rectangle(coords, radius=10, fill=(15, 23, 42), outline=color, width=2)
        draw.text((coords[0] + 15, coords[1] + 10), title, font=font_bold, fill=color)
        draw.text((coords[0] + 15, coords[1] + 35), desc, font=font_body, fill=TEXT_MUTED)

    # Card Derecha: MODO TECNICO
    draw.rounded_rectangle([(620, 130), (1160, 630)], radius=16, fill=CARD_BG, outline=CYAN, width=2)
    draw.text((650, 155), "MODO TECNICO / ADMINISTRADOR", font=font_h2, fill=CYAN)
    draw.text((650, 190), "Desbloqueado con PIN '1234' para ingenieros y cuadrilla.", font=font_body, fill=TEXT_MUTED)
    draw.text((650, 215), "- Protege la memoria EEPROM y parametros de seguridad vial.", font=font_body, fill=TEXT_WHITE)
    draw.text((650, 240), "- Habilita pestanas avanzadas en la barra de navegacion:", font=font_body, fill=TEXT_WHITE)

    # Opciones Modo Técnico
    admin_ops = [
        ("AJUSTES DE TIEMPOS DE CICLO", "Programar minutos de Verde/Rojo (1-15m) y Despeje (10-90s)", CYAN, (650, 280, 1130, 345)),
        ("ASISTENTE COURIER RTC", "Traslado de hora Maestro -> Esclavo con compensacion", CYAN, (650, 360, 1130, 425)),
        ("TEST DE MOSFETS Y LUCES", "Validacion secuencial de potencia (6 segundos)", AMBER, (650, 440, 1130, 505)),
        ("GESTOR CRUD DE CRUCES", "Catalogo de frentes de obra guardados en LocalStorage", GREEN, (650, 520, 1130, 585)),
    ]
    for title, desc, color, coords in admin_ops:
        draw.rounded_rectangle(coords, radius=10, fill=(15, 23, 42), outline=color, width=2)
        draw.text((coords[0] + 15, coords[1] + 10), title, font=font_bold, fill=color)
        draw.text((coords[0] + 15, coords[1] + 35), desc, font=font_body, fill=TEXT_MUTED)

    path1 = os.path.join(OUTPUT_DIR, "grafica_01_arquitectura_roles.png")
    path2 = os.path.join(EVIDENCIA_DIR, "grafica_01_arquitectura_roles.png")
    img.save(path1)
    img.save(path2)
    print(f"[OK] Grafica 1 guardada en: {path1}")

# ==============================================================================
# 2. DIAGRAMA DE FLUJO COURIER RTC
# ==============================================================================
def generar_grafica_courier():
    width, height = 1200, 600
    img = Image.new("RGB", (width, height), BG_COLOR)
    draw = ImageDraw.Draw(img)

    font_title = get_font(30, bold=True)
    font_subtitle = get_font(17, bold=False)
    font_h2 = get_font(20, bold=True)
    font_body = get_font(14, bold=False)
    font_mono = get_font(15, bold=True)

    draw.text((40, 30), "ASISTENTE COURIER RTC - SINCRONIZACION SIN RADIO (SFTY-18)", font=font_title, fill=CYAN)
    draw.text((40, 75), "Mecanismo de transporte de tiempo y ciclo con compensacion de tiempo de viaje", font=font_subtitle, fill=TEXT_MUTED)

    # Paso 1: Poste 1 (Maestro)
    draw.rounded_rectangle([(40, 140), (380, 530)], radius=14, fill=CARD_BG, outline=GREEN, width=2)
    draw.text((60, 165), "PASO 1 - EN POSTE 1", font=font_h2, fill=GREEN)
    draw.text((60, 205), "1. Conectar al Maestro (P1).", font=font_body, fill=TEXT_WHITE)
    draw.text((60, 235), "2. Pulsar 'Capturar Maestro'.", font=font_body, fill=TEXT_WHITE)
    draw.text((60, 265), "El celular memoriza:", font=font_body, fill=TEXT_MUTED)
    draw.text((60, 300), "- Hora RTC: 18:25:00", font=font_mono, fill=CYAN)
    draw.text((60, 330), "- Fase Activa: VERDE_P1", font=font_mono, fill=CYAN)
    draw.text((60, 360), "- Tiempo Restante: 38s", font=font_mono, fill=CYAN)
    draw.text((60, 410), "Inicia cronometro de viaje:", font=font_body, fill=TEXT_MUTED)
    draw.text((60, 445), "Reloj: 00:00 -> 02:00", font=font_mono, fill=AMBER)

    # Flecha Central
    draw.rounded_rectangle([(410, 270), (790, 400)], radius=12, fill=(15, 23, 42), outline=AMBER, width=2)
    draw.text((430, 290), "TRASLADO DEL OPERARIO", font=font_h2, fill=AMBER)
    draw.text((430, 325), "El operario camina o viaja hacia el Poste 2.", font=font_body, fill=TEXT_WHITE)
    draw.text((430, 355), "La App suma cada segundo transcurrido (dt).", font=font_body, fill=TEXT_MUTED)

    # Paso 2: Poste 2 (Esclavo)
    draw.rounded_rectangle([(820, 140), (1160, 530)], radius=14, fill=CARD_BG, outline=CYAN, width=2)
    draw.text((840, 165), "PASO 2 - EN POSTE 2", font=font_h2, fill=CYAN)
    draw.text((840, 205), "1. Conectar al Esclavo (P2).", font=font_body, fill=TEXT_WHITE)
    draw.text((840, 235), "2. Pulsar 'Inyectar en Esclavo'.", font=font_body, fill=TEXT_WHITE)
    draw.text((840, 265), "Compensacion automatica:", font=font_body, fill=TEXT_MUTED)
    draw.text((840, 300), "- Hora = 18:25:00 + 2 min", font=font_mono, fill=GREEN)
    draw.text((840, 330), "- Hora Compensada = 18:27:00", font=font_mono, fill=GREEN)
    draw.text((840, 360), "- Estado = R1_V2 (Sincronizado)", font=font_mono, fill=GREEN)
    draw.text((840, 410), "Resultado:", font=font_body, fill=TEXT_MUTED)
    draw.text((840, 445), "[OK] Sincronismo perfecto sin radio.", font=font_mono, fill=CYAN)

    path1 = os.path.join(OUTPUT_DIR, "grafica_02_courier_rtc_flujo.png")
    path2 = os.path.join(EVIDENCIA_DIR, "grafica_02_courier_rtc_flujo.png")
    img.save(path1)
    img.save(path2)
    print(f"[OK] Grafica 2 guardada en: {path1}")

# ==============================================================================
# 3. DIAGRAMA DE STACK TECNOLOGICO
# ==============================================================================
def generar_grafica_stack():
    width, height = 1200, 650
    img = Image.new("RGB", (width, height), BG_COLOR)
    draw = ImageDraw.Draw(img)

    font_title = get_font(30, bold=True)
    font_subtitle = get_font(17, bold=False)
    font_h2 = get_font(20, bold=True)
    font_body = get_font(14, bold=False)
    font_mono = get_font(14, bold=True)

    draw.text((40, 30), "STACK TECNOLOGICO Y PIPELINE DE COMPILACION - IOT-VIAL V9.0", font=font_title, fill=CYAN)
    draw.text((40, 75), "Arquitectura moderna, pruebas automatizadas TDD/E2E y compilacion a APK nativa", font=font_subtitle, fill=TEXT_MUTED)

    # 4 Columnas
    cols = [
        ("FRONTEND WEB", [
            ("HTML5 + Vanilla JS ES6", "Sin frameworks pesados"),
            ("CSS3 Cyber-Industrial", "Optica LED 3D + Glassmorphism"),
            ("Ergonomia Tactica", "Botonera de 4 toques"),
            ("Simulador en Vivo", "Pruebas directas en browser")
        ], GREEN, 40),
        ("TDD & E2E VISUAL", [
            ("29 Test Unitarios (100%)", "Checksum, NMEA, PIN, RTC"),
            ("Puppeteer E2E", "Validacion en Google Chrome"),
            ("Capturas Automaticas", "Evidencia visual en evidencia/"),
            ("Node.js Test Runner", "Ejecucion rapida y modular")
        ], CYAN, 320),
        ("MOTOR HIBRIDO", [
            ("Capacitor 6.x", "Puente nativo WebView"),
            ("Cordova BT Serial", "Sockets RFCOMM SPP (HC-05)"),
            ("Web Bluetooth BLE", "Soporte HM-10 / GATT"),
            ("LocalStorage", "Persistencia offline de cruces")
        ], AMBER, 600),
        ("COMPILACION APK", [
            ("JDK 17 / JDK 21", "Java runtime moderno"),
            ("Android SDK (API 34)", "Compatibilidad Android 7 a 14"),
            ("Gradle Wrapper", "assembleDebug (21 segundos)"),
            ("Salida: APK V9.0", "IOT_VIAL_Semaforos_v9.0.apk")
        ], RED, 880)
    ]

    for title, items, color, x in cols:
        draw.rounded_rectangle([(x, 130), (x + 260, 590)], radius=14, fill=CARD_BG, outline=color, width=2)
        draw.text((x + 15, 155), title, font=font_h2, fill=color)

        y = 210
        for name, desc in items:
            draw.rounded_rectangle([(x + 12, y), (x + 248, y + 70)], radius=8, fill=(15, 23, 42), outline=(255,255,255,30), width=1)
            draw.text((x + 22, y + 10), name, font=font_mono, fill=TEXT_WHITE)
            draw.text((x + 22, y + 36), desc, font=font_body, fill=TEXT_MUTED)
            y += 85

    path1 = os.path.join(OUTPUT_DIR, "grafica_03_stack_tecnologico_compilacion.png")
    path2 = os.path.join(EVIDENCIA_DIR, "grafica_03_stack_tecnologico_compilacion.png")
    img.save(path1)
    img.save(path2)
    print(f"[OK] Grafica 3 guardada en: {path1}")

if __name__ == "__main__":
    print("Iniciando generacion de infografias de arquitectura...")
    generar_grafica_roles()
    generar_grafica_courier()
    generar_grafica_stack()
    print("[OK] Todas las graficas han sido generadas con exito.")
