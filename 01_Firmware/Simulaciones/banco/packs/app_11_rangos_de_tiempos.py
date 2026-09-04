# ===== banco/packs/app_11_rangos_de_tiempos.py =====
#
# LOS RANGOS DE TIEMPO VIVEN EN TRES SITIOS Y EN TRES LENGUAJES, Y NADIE LOS CRUZABA.
#
#   Maestro/src/modo_automatico.cpp:32-34   C++    la guarda de verdad
#   App_Semaforo/app.js  enRango(...)       JS     lo que la app deja teclear
#   App_Semaforo/index.html  min= / max=    HTML   lo que el teclado numerico ofrece
#
# El 04/09 el responsable subio el minimo de verde y rojo de 1 a 3 minutos -"tres minutos
# es la minima distancia de seguridad"-, y al ir a cambiarlo aparecio que los seis numeros
# estaban escritos a mano en los tres sitios, sin nada que los atara.
#
# Es exactamente lo que contrato.h llama R-9 y este repositorio ya se ha cobrado tres
# veces (N-36, N-39, cfgVerdeRecibido): "repetir los rangos en dos lados es una segunda
# copia que alguien tiene que sincronizar, y el dia que difieran una punta deja pasar lo
# que la otra rechaza".
#
# QUE PASA SI DIVERGEN, para que se entienda por que esto no es cosmetica:
#   - app MAS PERMISIVA que el firmware -> el operario teclea un valor, la app lo acepta,
#     el equipo lo rechaza con $ERR y el tecnico se queda mirando un boton que no hizo
#     nada. Es el "OK mudo" al reves.
#   - app MAS ESTRICTA que el firmware -> hay configuraciones legitimas que nadie puede
#     poner desde la unica interfaz que existe.
#   - HTML distinto del JS -> el teclado del movil ofrece un rango y la validacion rechaza
#     otro. El operario pelea con su propio telefono.
#
# LO QUE ESTE PACK NO PUEDE COMPROBAR, y va escrito para que no se lea como permiso: que
# 3 minutos sea el numero CORRECTO. Eso es una decision vial y la tomo el responsable con
# su motivo. Aqui solo se exige que las tres copias digan lo mismo que el firmware.

import re

NOMBRE = "app_11_rangos_de_tiempos"
DESCRIPCION = "los rangos de tiempos dicen lo mismo en el C++, en app.js y en el HTML"

CPP = ("Maestro", "src", "modo_automatico.cpp")
APP_JS = ("05_Funcional", "App_Semaforo", "app.js")
APP_HTML = ("05_Funcional", "App_Semaforo", "index.html")


def correr(b, fw):
    b.titulo("Los rangos de tiempos, releidos de los tres lenguajes")

    cpp = fw.codigo(*CPP)

    # SIN VALOR POR DEFECTO EN NINGUNO: si el lector no encuentra la constante, el pack
    # ABORTA. Suponer un rango seria aprobar sobre un numero inventado, que es justo lo
    # que este pack existe para impedir.
    def constante(nombre):
        m = re.search(r"%s\s*=\s*(\d+)" % nombre, cpp)
        if not m:
            raise fw.Abortado(
                "no se halla %s en Maestro/src/modo_automatico.cpp. Sin el rango del C++ "
                "no hay contra que comparar, y comparar contra un valor supuesto seria "
                "inventar la referencia" % nombre)
        return int(m.group(1))

    v_min, v_max = constante("VERDE_MIN_MIN"), constante("VERDE_MIN_MAX")
    r_min, r_max = constante("ROJO_MIN_MIN"), constante("ROJO_MIN_MAX")
    d_min, d_max = constante("DESPEJE_SEG_MIN"), constante("DESPEJE_SEG_MAX")

    b.reportar(
        "los rangos que manda el firmware",
        "verde %d-%d min · rojo %d-%d min · despeje %d-%d s"
        % (v_min, v_max, r_min, r_max, d_min, d_max))

    # ---- 1. La validacion de app.js -------------------------------------------
    js = fw.texto_repo(*APP_JS)
    m = re.search(r"enRango\(\s*verde\s*,\s*(\d+)\s*,\s*(\d+)\s*\)", js)
    m2 = re.search(r"enRango\(\s*rojo\s*,\s*(\d+)\s*,\s*(\d+)\s*\)", js)
    m3 = re.search(r"enRango\(\s*despeje\s*,\s*(\d+)\s*,\s*(\d+)\s*\)", js)
    if not (m and m2 and m3):
        raise fw.Abortado(
            "no se hallan las tres llamadas a enRango() en app.js. El lector se quedo "
            "ciego, y dar por buenos unos rangos que no se han leido es peor que no "
            "mirarlos")

    js_v = (int(m.group(1)), int(m.group(2)))
    js_r = (int(m2.group(1)), int(m2.group(2)))
    js_d = (int(m3.group(1)), int(m3.group(2)))

    b.verificar(
        js_v == (v_min, v_max) and js_r == (r_min, r_max) and js_d == (d_min, d_max),
        "app.js valida con los MISMOS rangos que el firmware: verde %s, rojo %s, "
        "despeje %s" % (js_v, js_r, js_d),
        "app.js y el firmware NO dicen lo mismo. C++: verde (%d,%d) rojo (%d,%d) despeje "
        "(%d,%d). app.js: verde %s rojo %s despeje %s. Si la app es mas permisiva, el "
        "operario teclea un valor que el equipo rechazara con $ERR y se queda sin saber "
        "por que; si es mas estricta, hay configuraciones legitimas que nadie puede poner"
        % (v_min, v_max, r_min, r_max, d_min, d_max, js_v, js_r, js_d))

    # ---- 2. Los min/max del HTML ----------------------------------------------
    html = fw.texto_repo(*APP_HTML)

    def limites(campo):
        m = re.search(r'id="num-tiempo-%s"[^>]*?min="(\d+)"[^>]*?max="(\d+)"' % campo, html)
        if not m:
            raise fw.Abortado(
                "no se hallan min/max del campo '%s' en index.html: el lector no puede "
                "decir que ofrece el teclado del movil" % campo)
        return (int(m.group(1)), int(m.group(2)))

    h_v, h_r, h_d = limites("verde"), limites("rojo"), limites("despeje")

    b.verificar(
        h_v == (v_min, v_max) and h_r == (r_min, r_max) and h_d == (d_min, d_max),
        "el HTML ofrece los MISMOS limites: verde %s, rojo %s, despeje %s"
        % (h_v, h_r, h_d),
        "los min/max del HTML no cuadran con el firmware. C++: verde (%d,%d) rojo (%d,%d) "
        "despeje (%d,%d). HTML: verde %s rojo %s despeje %s. El operario pelearia con su "
        "propio telefono: el teclado le ofrece un rango y la validacion le rechaza otro"
        % (v_min, v_max, r_min, r_max, d_min, d_max, h_v, h_r, h_d))

    # ---- 3. El valor POR DEFECTO tiene que ser valido --------------------------
    #
    # No es un detalle: al subir el minimo de 1 a 3, el value="2" que traia el formulario
    # se quedo fuera de rango. El operario abre la pantalla, no toca nada, pulsa guardar
    # y el equipo le rechaza SU PROPIO valor por defecto.
    for campo, (lo, hi) in (("verde", (v_min, v_max)), ("rojo", (r_min, r_max)),
                            ("despeje", (d_min, d_max))):
        m = re.search(r'id="num-tiempo-%s"[^>]*?value="(\d+)"' % campo, html)
        val = int(m.group(1)) if m else None
        b.verificar(
            val is not None and lo <= val <= hi,
            "el valor por defecto de '%s' (%s) esta dentro del rango %d-%d"
            % (campo, val, lo, hi),
            "el valor por defecto de '%s' es %s y el rango es %d-%d: el operario abre la "
            "pantalla, no toca nada, pulsa guardar y el equipo rechaza el valor que la "
            "propia app le puso delante" % (campo, val, lo, hi))

    # ---- CONTROLES NEGATIVOS ---------------------------------------------------
    b.control_negativo(
        re.search(r"enRango\(\s*verde\s*,\s*(\d+)", "if (!enRango(verde, 1, 15))")
        .group(1) == "1",
        "el lector de app.js extrae el numero real de la llamada, no uno supuesto")

    b.control_negativo(
        re.search(r'id="num-tiempo-verde"[^>]*?min="(\d+)"',
                  '<input id="num-tiempo-verde" min="9" max="15">') is not None,
        "el lector del HTML encuentra el min de un campo aunque valga otra cosa: "
        "compara, no da por bueno")

    b.control_negativo(
        re.search(r"VERDE_MIN_MIN\s*=\s*(\d+)", "static const uint8_t OTRA = 7;") is None,
        "si la constante del C++ cambiara de nombre, el lector NO la encuentra y el pack "
        "ABORTA en vez de aprobar sobre un rango que ya no existe")
