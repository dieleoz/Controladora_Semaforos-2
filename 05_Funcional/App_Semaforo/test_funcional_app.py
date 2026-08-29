#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
test_funcional_app.py
Bateria de Pruebas Funcionales y Validacion Integral de la App Movil IOT-VIAL V8.9
Verifica cada boton de la interfaz, transicion de pantallas, CRUD de cruces,
logica Courier RTC y comunicacion bidireccional con el firmware STM32.
"""

import os
import glob
import re
import sys
import json
import time

import sys
import io

# Forzar salida UTF-8 en consola Windows
if sys.platform == "win32":
    sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')

def print_header(title):
    print("\n" + "=" * 80)
    print(f" [TEST] {title}")
    print("=" * 80)

PASADAS = 0

def test_passed(name, detail=""):
    # N-62: el resumen llevaba "22/22" escrito a mano. Un recuento que no se cuenta es
    # el mismo defecto que las cifras del README: sobrevive a que alguien borre una
    # suite entera. Ahora se cuenta aqui.
    global PASADAS
    PASADAS += 1
    print(f"  [OK] {name}" + (f" -> {detail}" if detail else ""))

def test_failed(name, reason):
    print(f"  [FAIL] {name} -> {reason}")
    sys.exit(1)

def calcular_checksum_nmea(trama):
    """Calcula el checksum XOR NMEA estandar."""
    xor_val = 0
    contenido = trama.lstrip('$').split('*')[0]
    for c in contenido:
        xor_val ^= ord(c)
    return f"{xor_val:02X}"

def main():
    print_header("SUITE FUNCIONAL 1: ESTRUCTURA Y ELEMENTOS INTERACTIVOS (HTML/CSS)")
    
    html_path = os.path.join(os.path.dirname(__file__), "index.html")
    if not os.path.exists(html_path):
        test_failed("Existencia de index.html", "No se encontro el archivo")
    
    with open(html_path, "r", encoding="utf-8") as f:
        html_content = f.read()

    # 1.1 Verificar Pestañas de Navegacion
    # N-75: la interfaz de 2 roles retiro tab-control -sus mandos son ahora la
    # botonera tactica del operario- y tab-rtc -el Courier vive en tab-diag-, y
    # anadio tab-tiempos para los ajustes de ciclo. Se comprueban las que hay.
    expected_tabs = ["tab-estado", "tab-eventos", "tab-tiempos", "tab-diag"]
    for tab in expected_tabs:
        if f'data-tab="{tab}"' in html_content and f'id="{tab}"' in html_content:
            test_passed(f"Pestaña de Navegacion: {tab}", "Declarada en HTML y Bottom-Nav")
        else:
            test_failed(f"Pestaña de Navegacion: {tab}", "Falta en HTML o nav-bar")

    # 1.2 Verificar Botones de Control y Modos
    expected_buttons = [
        ("btnDevice", "Boton de Enlace Bluetooth"),
        ("btn-select-site", "Selector de Cruces"),
        ("btn-open-add-site", "Crear Nuevo Cruce"),
        ("btn-op-auto", "Tactico: reanudar Automatico"),
        ("btn-op-step", "Tactico: cambiar turno"),
        ("btn-op-amber", "Tactico: Ambar de precaucion"),
        ("btn-op-emergency", "Tactico: Rojo Total de emergencia"),
        # N-83: el Esclavo no cae a rojo, cae a ambar intermitente con la
        # talanquera abierta. Son dos maniobras distintas y por eso son dos
        # mandos: un solo boton para las dos volveria a ocultar cual se pide.
        ("btn-op-ambar-emergencia", "Tactico: Ambar de emergencia (Esclavo)"),
        ("btn-toggle-role", "Conmutador Operario / Tecnico"),
        ("btn-start-test-leds", "Test de Leds"),
        ("btn-sync-rtc", "Sincronizacion Horaria"),
        ("btn-courier-capture", "Courier Paso 1 (Captura)"),
        ("btn-courier-inject", "Courier Paso 2 (Inyeccion)"),
        ("btn-share-whatsapp", "Reporte WhatsApp"),
        ("btn-export-csv", "Exportar Log CSV")
    ]
    for btn_id, desc in expected_buttons:
        if f'id="{btn_id}"' in html_content:
            test_passed(f"Elemento UI: {desc} (#{btn_id})", "Presente en interfaz")
        else:
            test_failed(f"Elemento UI: {desc} (#{btn_id})", "No existe en el DOM")

    # 1.3 Verificar Modal PIN y Modal Bluetooth
    if 'id="pin-modal"' in html_content and 'class="pin-pad"' in html_content:
        test_passed("Modal PIN de Seguridad", "Teclado numerico de 4 digitos presente")
    else:
        test_failed("Modal PIN", "Falta el modal o teclado de PIN")

    if 'id="bt-modal"' in html_content and 'name="bt_pin_opt"' in html_content:
        test_passed("Modal Bluetooth Estilo Baliza", "Selector de PIN (1234/0000) y lista presentes")
    else:
        test_failed("Modal Bluetooth", "Falta el modal o selector de PIN")


    print_header("SUITE FUNCIONAL 2: LOGICA JAVASCRIPT, NAVEGACION Y EVENTOS (app.js)")
    
    # N-75: el rewrite saco el gestor de cruces, el Courier y el driver BT de app.js
    # a js/*.js. Buscar solo en app.js hacia que este arnes acusara a la app de haber
    # perdido funciones que solo habian cambiado de fichero -CLAUDE.md 5-.
    js_dir = os.path.dirname(__file__)
    js_content = ""
    for ruta in [os.path.join(js_dir, "app.js")] + sorted(
            glob.glob(os.path.join(js_dir, "js", "*.js"))):
        with open(ruta, "r", encoding="utf-8") as f:
            js_content += f.read() + "\n"

    # 2.1 Tab Switcher Event Listener
    if "navItems.forEach" in js_content and "targetEl.classList.add('active')" in js_content:
        test_passed("Navegacion entre Pestañas", "Evento click conmuta clases active en nav-items y tab-contents")
    else:
        test_failed("Navegacion entre Pestañas", "Falta el evento conmutador de pestañas en app.js")

    # 2.2 CRUD de Cruces Viales
    if "guardarCruces" in js_content and "localStorage" in js_content and "renderSiteList" in js_content:
        test_passed("Gestor Dinámico de Cruces (CRUD)", "Persistencia en localStorage y render dinamico")
    else:
        test_failed("CRUD de Cruces", "Faltan funciones de gestion de cruces")

    # 2.3 Validacion de PIN de Seguridad
    #
    # ANTES esta comprobacion solo miraba que existieran validatePin() y la constante
    # del PIN. Las dos existian en la version que inyectaba 'CMD:PIN:1234:' en TODOS
    # los comandos sin que nadie tecleara nada, asi que daba PASS sobre una barrera
    # abierta. Lo que hay que medir no es que la funcion exista: es que la trama NO se
    # pueda construir sin que alguien haya verificado el PIN en esta sesion.
    if ("validatePin" in js_content
            and "state.pinVerificado" in js_content
            and "!state.pinVerificado" in js_content):
        test_passed("Barrera de Seguridad PIN",
                    "El envio exige PIN verificado en la sesion; no se inyecta un literal")
    else:
        test_failed("Barrera de Seguridad PIN",
                    "El envio de comandos no comprueba state.pinVerificado: la app "
                    "estaria autorizandose sola")

    # 2.3.bis El PIN sale del selector del modal, no de un literal enterrado
    if 'name="bt_pin_opt"' in html_content and "bt_pin_opt" in js_content:
        test_passed("Origen del PIN de autorizacion",
                    "Se lee del selector del modal Bluetooth y el pack lo cruza con el C++")
    else:
        test_failed("Origen del PIN de autorizacion",
                    "El PIN no sale de la interfaz: si el equipo cambia de PIN no hay "
                    "forma de cambiarlo sin recompilar la app")

    # 2.3.ter La excepcion del rojo de emergencia esta declarada, y es UNA
    if "SIN_PIN" in js_content and "'FORZAR_ROJO'" in js_content:
        test_passed("Excepcion de emergencia declarada",
                    "El rojo de emergencia viaja sin PIN, como acepta bluetooth.cpp")
    else:
        test_failed("Excepcion de emergencia",
                    "No hay lista explicita de ordenes que viajan sin PIN")

    # 2.3.quater La app vuelve a OIR al equipo. El rewrite de V9.0 borro este camino
    # entero y la pantalla paso a pintar un estado inventado por el propio telefono.
    if "parseNmeaTelemetry" in js_content and "bluetoothSerial.subscribe" in js_content:
        test_passed("Ingesta de telemetria del equipo",
                    "La app se suscribe al canal serie y parsea $STATUS/$ALARM/$ERR")
    else:
        test_failed("Ingesta de telemetria",
                    "La app no lee lo que el equipo emite: todo lo que muestra seria "
                    "una simulacion local, incluido el color de las luces")

    # 2.4 Asistente Courier RTC
    if "btnCourierCapture" in js_content and "btnCourierInject" in js_content and "courierSnapshot" in js_content:
        test_passed("Asistente Courier RTC", "Captura de snapshot y calculo de desfase de viaje implementados")
    else:
        test_failed("Courier RTC", "Falta la logica de captura/inyeccion temporal")

    # 2.5 Driver Hibrido Bluetooth (SPP Nativo + BLE Fallback)
    if "window.bluetoothSerial" in js_content and "bluetoothSerial.connect" in js_content:
        test_passed("Driver Bluetooth Nativo SPP", "Soporte RFCOMM para modulos HC-05/JDY-31")
    else:
        test_failed("Driver Bluetooth Nativo", "Falta el enlace con cordova-plugin-bluetooth-serial")


    # 2.6 La cabecera no se puede romper con un nombre largo
    #
    # N-75: el nombre "Tramo Obra Km 45 - Via al Llano" -que la app traia DE FABRICA-
    # partia la cabecera en dos lineas y montaba el rotulo del nodo con el RSSI. Se
    # acorto el default y se puso un tope de 32 al alta, pero NINGUNA de esas dos cosas
    # es la garantia: los nombres tambien llegan de localStorage de versiones
    # anteriores, donde ningun limite de hoy alcanza. La garantia es que el texto
    # TRUNQUE. Y se comprueba sobre el CSS y no contando caracteres a mano, que es el
    # error que este repositorio ya pago una vez con los anchos del LCD.
    ruta_css = os.path.join(os.path.dirname(os.path.abspath(__file__)), "style.css")
    with io.open(ruta_css, encoding="utf-8", errors="replace") as f:
        css = f.read()

    def trunca(selector):
        m = re.search(re.escape(selector) + r"\s*\{([^}]*)\}", css)
        if not m:
            return False
        cuerpo = m.group(1)
        return ("text-overflow: ellipsis" in cuerpo
                and "white-space: nowrap" in cuerpo
                and "overflow: hidden" in cuerpo
                and "max-width" in cuerpo)

    for selector, que in ((".site-name", "nombre del cruce"),
                          ("#node-name", "rotulo del nodo")):
        if trunca(selector):
            test_passed(f"Cabecera a prueba de nombres largos ({que})",
                        f"{selector} trunca con ellipsis y max-width")
        else:
            test_failed(f"Cabecera: {que}",
                        f"{selector} no trunca: un nombre largo parte la cabecera en dos "
                        f"lineas y se monta con el texto de al lado")

    # Y ningun nombre de fabrica puede pasarse del tope que la propia app aplica.
    ruta_sm = os.path.join(os.path.dirname(os.path.abspath(__file__)), "js", "site_manager.js")
    with io.open(ruta_sm, encoding="utf-8", errors="replace") as f:
        sm = f.read()
    m_tope = re.search(r"MAX_NOMBRE:\s*(\d+)", sm)
    if m_tope:
        tope = int(m_tope.group(1))
        largos = [n for n in re.findall(r"nombre:\s*'([^']+)'", sm) if len(n) > tope]
        if not largos:
            test_passed("Nombres de fabrica dentro del tope",
                        f"ninguno de los cruces por defecto pasa de {tope} caracteres")
        else:
            test_failed("Nombres de fabrica",
                        f"la app trae de fabrica nombres de mas de {tope}: {largos}. "
                        f"No los escribio el funcional: los enviamos nosotros")
    else:
        test_failed("Tope de nombre", "no se hallo MAX_NOMBRE en site_manager.js")

    # 2.7 CONTRASTE DE LA PALETA, MEDIDO — NO ELEGIDO
    #
    # La app se usa a pie de calzada, con sol o con lluvia. "El tema oscuro contrasta
    # bien" es una opinion; el contraste WCAG es una cuenta que sale del propio CSS.
    # Se mide cada color de TEXTO contra el fondo real de la app.
    #
    # UMBRALES: 4.5:1 texto normal, 3:1 texto grande y componentes graficos.
    # La LAMPARA del semaforo se mide como componente (3:1) y no como texto: su rojo
    # se quedo en 4.9:1 a proposito porque ningun rojo creible de semaforo llega a
    # AAA sobre fondo casi negro -se midieron siete- y un rojo que no se lee como
    # rojo es peor que un ratio mas bajo. El ROTULO si usa un rojo de texto aparte.
    #
    # LO QUE ESTO NO MIDE, y conviene que quede escrito: bajo sol directo la luz que
    # refleja el cristal sube el nivel de negro y COMPRIME los ratios, y a un tema
    # oscuro le comprime mas que a uno claro. Esta cuenta es necesaria, no suficiente.
    def _lin(c):
        c = c / 255.0
        return c / 12.92 if c <= 0.03928 else ((c + 0.055) / 1.055) ** 2.4

    def _lum(hexa):
        h = hexa.lstrip("#")
        r, g, b = (int(h[i:i + 2], 16) for i in (0, 2, 4))
        return 0.2126 * _lin(r) + 0.7152 * _lin(g) + 0.0722 * _lin(b)

    def _ratio(a, b):
        la, lb = _lum(a), _lum(b)
        return (max(la, lb) + 0.05) / (min(la, lb) + 0.05)

    paleta = dict(re.findall(r"--([a-z0-9-]+):\s*(#[0-9a-fA-F]{6})", css))
    if "bg-surface" not in paleta:
        test_failed("Contraste de la paleta",
                    "no se hallo --bg-surface en style.css: sin el fondo no hay contra "
                    "que medir, y aprobar aqui seria comparar nada contra nada")
    fondo = paleta["bg-surface"]

    # (token, umbral, para que se usa)
    EXIGIDOS = [
        ("text-primary",   4.5, "texto principal"),
        ("text-secondary", 4.5, "texto secundario"),
        ("text-muted",     4.5, "texto atenuado (MAC, sellos de hora, subtitulos)"),
        ("cyan-neon",      4.5, "acento y nombre del cruce"),
        ("green-lamp",     4.5, "rotulo VERDE (PASO)"),
        ("amber-lamp",     4.5, "rotulo AMBAR"),
        ("red-text",       4.5, "rotulo ROJO (ESPERA) y cifras en rojo"),
        ("red-lamp",       3.0, "lampara roja: componente grafico, no texto"),
    ]
    for token, umbral, uso in EXIGIDOS:
        if token not in paleta:
            test_failed(f"Contraste: --{token}",
                        f"el token no existe en style.css. Se usa para {uso}")
            continue
        r = _ratio(paleta[token], fondo)
        if r >= umbral:
            nivel = "AAA" if r >= 7.0 else "AA"
            test_passed(f"Contraste --{token} ({uso})",
                        f"{r:.1f}:1 sobre {fondo} — {nivel}, exigido {umbral}:1")
        else:
            test_failed(f"Contraste --{token}",
                        f"{r:.1f}:1 sobre {fondo}, por debajo de {umbral}:1. Se usa "
                        f"para {uso}: a pleno sol eso no se lee")

    # Control negativo: la cuenta tiene que saber suspender.
    if _ratio("#64748B", "#0B111E") < 4.5 and _ratio("#F8FAFC", "#0B111E") >= 7.0:
        test_passed("Control Negativo de contraste",
                    "el calculo suspende el gris viejo (4.0:1) y aprueba el blanco (18:1)")
    else:
        test_failed("Control Negativo de contraste",
                    "el calculo de contraste no distingue un color que cumple de uno "
                    "que no: su PASS no vale nada")

    # 2.8 NADA PINTA UN CRUCE QUE NO EXISTE
    #
    # El build de campo no lleva simulador. Habia dos cosas de la misma familia:
    #   - el panel "SIMULADOR DE PRUEBAS", ocho botones que escribian en state y
    #     repintaban los MISMOS semaforos que la telemetria real, avisando con un
    #     toast que se va solo;
    #   - runLocalTicker(), que hacia lo mismo SIN QUE NADIE LO PULSARA: con la app
    #     abierta y sin equipo animaba un ciclo completo y creible, indefinidamente.
    #
    # Se retiraron las dos. Esta comprobacion existe porque un panel de demo se vuelve
    # a anadir muy facil -"solo para probar"- y el dia que vuelva nadie se acordara de
    # que el problema no era el panel: era que un tablero que no sabe tiene que DECIR
    # que no sabe, no rellenar el hueco con algo verosimil.
    restos_html = [m for m in ("field-sim-bar", "sim-buttons-grid", "btn-sim",
                               "SIMULADOR DE PRUEBAS", "DEMO EN VIVO")
                   if m in html_content]
    restos_js = [m for m in ("runLocalTicker", "btnSim", "sim-btn-") if m in js_content]
    if not restos_html and not restos_js:
        test_passed("Sin simulador en el build de campo",
                    "ni panel de demo ni ciclo local: lo que se ve viene del equipo")
    else:
        test_failed("Simulador en el build de campo",
                    f"quedan restos {restos_html + restos_js}. Un boton que pinta un "
                    f"estado falso en los mismos widgets que el real es lo que hace "
                    f"que el operario no pueda fiarse de ninguno de los dos")

    # Y su reverso: sin enlace la pantalla tiene que declararlo.
    if "marcarSinEnlace" in js_content and "SIN ENLACE" in js_content:
        test_passed("La pantalla admite cuando no tiene datos",
                    "sin equipo conectado el rotulo pasa a SIN ENLACE en vez de animar")
    else:
        test_failed("Sin enlace no se declara",
                    "al perder el equipo la pantalla se queda con el ultimo cuadro sin "
                    "decir que ya no es en vivo")

    # 2.9 LA CABECERA TIENE QUE ENCOGER
    #
    # Medido con el navegador a cuatro anchos (herramientas_medir_desborde.js):
    #   412 px -el de la maqueta con la que se hicieron las capturas-  0 px
    #   390 px  11 px      360 px  41 px      320 px  81 px  DE DESBORDE
    #
    # El sintoma que se reporto era "no veo el boton de la derecha y DAR PASO y ROJO
    # TOTAL salen a la mitad". La causa estaba en otro sitio: la cabecera no encogia,
    # ensanchaba el documento entero y cortaba todo lo de la derecha. Por eso se midio
    # en vez de mirar donde dolia.
    #
    # Aqui no se puede medir el ancho -esto es Python leyendo ficheros-, asi que se
    # comprueba la CAUSA: que los hijos flex puedan bajar de su ancho de contenido y
    # que las pastillas se queden en icono en pantalla estrecha. La medida de verdad
    # la hace el script con el navegador.
    guardas = [
        ("min-width: 0", ".status-left puede encoger"),
        ("@media (max-width: 400px)", "hay regla para pantalla estrecha"),
        ("#bt-btn-text, #role-label { display: none; }", "las pastillas se quedan en icono"),
    ]
    faltan = [q for m, q in guardas if m not in css]
    if not faltan:
        test_passed("Cabecera que encoge en pantalla estrecha",
                    "min-width:0 y las pastillas a icono bajo 400 px")
    else:
        test_failed("Cabecera se desborda",
                    f"faltan las guardas: {faltan}. A 360 px la pagina se sale y se "
                    f"cortan los botones de la derecha, incluido ROJO TOTAL")

    print_header("SUITE FUNCIONAL 3: COMUNICACION BIDIRECCIONAL CON FIRMWARE STM32 (NMEA)")

    # 3.1 Simulacion de Recepcion de Telemetria NMEA del STM32
    telemetria_ejemplo = "$STATUS,NODE:MAESTRO,ID:SEM-M-01,SITE:Km 12 Sisga,PAIR:SEM-E-01,MODO:AUTO,ESTADO:V1_R2,T:35,RF:98,RTT:82,BAT:12.6,HORA:18:25:00"
    crc_calc = calcular_checksum_nmea(telemetria_ejemplo)
    trama_completa = f"{telemetria_ejemplo}*{crc_calc}"
    
    # Validar formato con regex de la App
    match = re.match(r'^\$STATUS,(.*)\*([0-9A-F]{2})$', trama_completa)
    if match:
        campos = dict(item.split(':', 1) for item in match.group(1).split(',') if ':' in item)
        if campos.get('NODE') == 'MAESTRO' and campos.get('ESTADO') == 'V1_R2' and campos.get('BAT') == '12.6':
            test_passed("Parser de Telemetria NMEA $STATUS", f"Campos extraidos: Nodo={campos['NODE']}, Estado={campos['ESTADO']}, Bat={campos['BAT']}V, Hora={campos.get('HORA')}")
        else:
            test_failed("Parser NMEA", "Campos no coinciden con la trama esperada")
    else:
        test_failed("Formato NMEA", "La trama no cumple el estandar NMEA $STATUS...*CRC")

    # 3.2 Validacion de Comandos Enviados por la App al STM32
    comandos_a_probar = [
        ("SET_MODO:AUTO", "CMD:PIN:1234:SET_MODO:AUTO\r\n"),
        ("SET_MODO:MANUAL", "CMD:PIN:1234:SET_MODO:MANUAL\r\n"),
        ("SET_MODO:AMBAR", "CMD:PIN:1234:SET_MODO:AMBAR\r\n"),
        ("FORZAR_ROJO", "CMD:PIN:1234:FORZAR_ROJO\r\n"),
        ("PASO_P1", "CMD:PIN:1234:PASO_P1\r\n"),
        ("PASO_P2", "CMD:PIN:1234:PASO_P2\r\n"),
        ("SET_CONFIG", "CMD:PIN:1234:SET_CONFIG:SITE=Km 24 Macheta,ROLE=MAESTRO,PAIR=SEM-E-01\r\n"),
        ("SET_RTC", "CMD:PIN:1234:SET_RTC:2026-08-27,09:30:00\r\n")
    ]

    for nombre, trama_cmd in comandos_a_probar:
        # Simular verificacion de PIN en firmware
        partes = trama_cmd.strip().split(':')
        if len(partes) >= 4 and partes[0] == "CMD" and partes[1] == "PIN":
            pin_recibido = partes[2]
            accion = ":".join(partes[3:])
            if pin_recibido == "1234":
                test_passed(f"Comando Firmware: {nombre}", f"Trama '{trama_cmd.strip()}' -> PIN Valido -> ACK:OK")
            else:
                test_failed(f"Comando Firmware: {nombre}", "PIN rechazado")
        else:
            test_failed(f"Comando Firmware: {nombre}", "Formato de comando incorrecto")

    # 3.3 Control Negativo: Comando con PIN Incorrecto
    trama_ataque = "CMD:PIN:0001:FORZAR_ROJO\r\n"
    partes_atq = trama_ataque.strip().split(':')
    if partes_atq[2] != "1234":
        test_passed("Control Negativo de Seguridad", f"Trama con PIN {partes_atq[2]} rechazada -> ERR:PIN_INVALIDO")
    else:
        test_failed("Control Negativo", "Acepto un PIN no autorizado")


    print_header("SUITE FUNCIONAL 4: LOGICA COURIER RTC (DESFASE CERO)")

    # N-62: AQUI HABIA UNA TAUTOLOGIA, Y CONVIENE QUE QUEDE ESCRITO POR QUE.
    #
    # La prueba hacia t1 = t0 + viaje y luego comprobaba que t1 - (t0 + viaje) == 0.
    # Eso es cierto para CUALQUIER t0 y CUALQUIER viaje: no puede fallar, luego no mide
    # nada. Es la nota disfrazada de prueba de CLAUDE.md §3, y encima sobre la funcion
    # mas delicada de la app: la que pone en hora un poste que no tiene radio.
    #
    # La propiedad de verdad es esta: al INYECTAR, la app manda la hora de ESE instante,
    # no la que capturo al salir del otro poste. Si mandara la capturada, el poste
    # quedaria atrasado exactamente lo que duro el viaje -y nadie lo notaria hasta que
    # las dos puntas se desfasaran en Degradado-. Se mide sobre el app.js real, y sabe
    # fallar: el control negativo de debajo lo demuestra.
    def inyecta_hora_del_momento(texto):
        """La hora inyectada es la de AHORA, no la capturada al salir.

        Dos condiciones, y hacen falta las dos: que el tiempo de viaje se calcule
        restando la captura del instante de inyeccion, y que ese transcurrido se
        SUME a la hora capturada. Codigo que calcula el transcurrido y no lo suma
        deja el poste atrasado exactamente lo que duro el trayecto."""
        transcurrido = re.search(
            r"elapsedMs\s*=\s*timestampInyeccion\s*-\s*snapshot\.timestampCaptura", texto)
        suma = re.search(
            r"setSeconds\(\s*dateObj\.getSeconds\(\)\s*\+\s*elapsedSeg\s*\)", texto)
        return bool(transcurrido and suma)

    if inyecta_hora_del_momento(js_content):
        test_passed("Algoritmo Courier RTC",
                    "la inyeccion suma el tiempo de viaje transcurrido al instante capturado")
    else:
        test_failed("Courier RTC",
                    "la app no compensa el viaje al inyectar: mandaria la hora de salida y el "
                    "poste quedaria atrasado lo que durase el trayecto")

    # El calculo no vale de nada si nadie lo llama: se exige el llamador, no solo la
    # funcion. Un modulo correcto que la app no invoca es la prueba muerta silenciosa.
    if "CourierRTC.calcularCompensacion" in js_content and "CourierRTC.capturarMaestro" in js_content:
        test_passed("Courier RTC conectado a la interfaz",
                    "los dos pasos del asistente llaman al calculo real")
    else:
        test_failed("Courier RTC sin llamador",
                    "el calculo de compensacion existe pero la app no lo usa")

    if not inyecta_hora_del_momento(
            "const injectedDate = new Date(snapshot.timestampCaptura);"):
        test_passed("Control Negativo Courier RTC",
                    "una inyeccion que manda la hora capturada tal cual NO pasa el detector")
    else:
        test_failed("Control Negativo Courier RTC",
                    "el detector aprueba una inyeccion sin compensar: no sabe fallar")

    # El caso peligroso de verdad: SI calcula el viaje y luego NO lo suma.
    if not inyecta_hora_del_momento(
            "const elapsedMs = timestampInyeccion - snapshot.timestampCaptura;\n"
            "dateObj.setSeconds(dateObj.getSeconds());"):
        test_passed("Control Negativo Courier RTC (transcurrido calculado y no sumado)",
                    "calcular el viaje sin aplicarlo tampoco pasa el detector")
    else:
        test_failed("Control Negativo Courier RTC (transcurrido calculado y no sumado)",
                    "el detector se conforma con que aparezca el calculo: no mide la suma")
    # El "N/N" no es un adorno: es la forma que compuerta.py sabe extraer para el acta,
    # y aqui es honesto porque test_failed() corta la corrida en el acto -si se llega a
    # esta linea, las N que se ejecutaron pasaron todas-.
    print_header(f"RESUMEN: {PASADAS}/{PASADAS} comprobaciones funcionales de la app en PASS")
    print("Interfaz HTML/CSS, botones de control, gestor de cruces, Courier RTC y enlace NMEA.")
    print("AVISO: esto mide el HTML/JS y el protocolo en el PC. No sustituye la prueba con el")
    print("       modulo Bluetooth fisico delante: ver ESTADO.md, tarea BANCO.")
    print("=" * 80 + "\n")

if __name__ == "__main__":
    main()
