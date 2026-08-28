# ===== banco/packs/flash_01_lastre.py =====
#
# NO ENLAZAR UN BUS QUE NO EXISTE: LAS DOS BANDERAS DE U8G2.
#
# N-70. El Maestro iba por el 93,5 % de flash con 4.292 bytes libres, y la conversacion
# era la de siempre: que funcion sacrificamos para que quepa la siguiente. La respuesta
# no estaba en el firmware propio.
#
# QUE SE MIDIO, Y COMO.
#
# `firmware.map` repartido por FICHERO OBJETO -no por nombre de simbolo, que engana: los
# simbolos de C++ de cualquier libreria tambien empiezan por _Z-. De los 61.297 B, el
# firmware propio son 18.505 B: el 30 %. El resto es libreria, y ahi habia una fila que no
# tenia por que existir: 1.080 B de Wire mas 3.732 B del HAL de I2C, en un equipo donde el
# bus por hardware esta copado por otras cosas y NADIE lo usa.
#
# La causa NO se dedujo: esta escrita en firmware.map, que dice quien arrastra a quien.
#
#     libU8g2.a(U8x8lib.cpp.o)  ->  TwoWire::setClock(unsigned long)
#     libWire.a(Wire.cpp.o)     ->  i2c_init
#     libWire.a(twi.c.o)        ->  el HAL de I2C entero
#
# U8x8lib.cpp trae en UNA sola unidad de compilacion los transportes de todos los
# backends -SW_SPI, HW_SPI, I2C-. Referenciar cualquiera arrastra el objeto completo, y
# con el, Wire. La libreria lo previo: U8x8lib.h define U8X8_HAVE_HW_I2C salvo que se
# declare U8X8_NO_HW_I2C. Nuestras dos pantallas son U8G2_ST7920_128X64_F_SW_SPI.
#
# RESULTADO MEDIDO, quitando y poniendo las banderas sobre el mismo arbol -un delta exige
# medir los DOS extremos-: Maestro 61.244 -> 56.084 B (93,5 % -> 85,6 %), Esclavo 46.896 ->
# 41.736 B (71,6 % -> 63,7 %), y 352 B de RAM en cada punta. Cinco mil ciento sesenta bytes
# por dos lineas de configuracion y cero cambios de codigo.
#
# POR QUE ESTO ES UN PACK Y NO UN COMMIT SUELTO.
#
# Una bandera de compilacion no la protege nada: no hay error, no hay aviso, y el dia
# que alguien reescriba platformio.ini -o copie el del Repetidor, que no las lleva
# porque no tiene pantalla- la flash sube 5 KB de golpe y el sintoma sera "ya no
# cabe", tres semanas despues y sin relacion aparente con el commit que lo causo.
#
# Y VIGILA LAS DOS DIRECCIONES, que es lo que lo hace util a futuro: el roadmap
# contempla un expansor PCF8574, que SI es I2C. El dia que entre, estas banderas tienen
# que salir -y este pack lo dira, en vez de dejar un fallo de enlazado sin explicacion-.

import re

NOMBRE = "flash_01_lastre"
DESCRIPCION = "no se enlaza el I2C ni el SPI por hardware mientras ninguna punta los use"

PUNTAS = ("Maestro", "Esclavo", "Repetidor")

BANDERAS = ("U8X8_NO_HW_I2C", "U8X8_NO_HW_SPI")


def _usa_de_verdad(fw, punta):
    """Los buses que esa punta usa DE VERDAD. Sin comentarios, sin adivinar.

    La distincion importa: los tres platformio.ini y varios .h hablan de I2C en
    comentarios -explicando justamente por que NO se usa-. Un detector que contara
    esas menciones concluiria que el bus se usa y aprobaria el lastre."""
    usados = set()
    for carpeta in ("src", "include"):
        for fichero in fw.fuentes_de(punta, carpeta):
            codigo = fw.codigo(punta, carpeta, fichero)   # comentarios fuera
            if re.search(r"#include\s*<Wire\.h>|\bWire\s*\.\s*\w+\s*\(", codigo):
                usados.add("I2C")
            if re.search(r"#include\s*<SPI\.h>|\bSPI\s*\.\s*\w+\s*\(", codigo):
                usados.add("SPI")
    return usados


def _depende_de_u8g2(fw, punta):
    return "U8g2" in fw.texto(punta, "platformio.ini")


def _banderas_de(fw, punta):
    """Las banderas declaradas en el platformio.ini de esa punta.

    Se leen del bloque build_flags y NO de todo el fichero: el comentario que explica
    la decision nombra las dos banderas, asi que buscarlas en el texto entero daria
    por declarada una bandera que solo esta explicada."""
    texto = fw.texto(punta, "platformio.ini")
    m = re.search(r"^build_flags\s*=(.*?)(?=^\[|\Z)", texto, re.S | re.M)
    if m is None:
        return set()
    cuerpo = re.sub(r";[^\n]*", " ", m.group(1))          # comentarios de .ini fuera
    return set(re.findall(r"-D\s+([A-Za-z0-9_]+)", cuerpo))


def correr(b, fw):
    b.titulo("Lastre de flash: ningun bus enlazado sin usarse")

    usa = {p: _usa_de_verdad(fw, p) for p in PUNTAS}
    con_pantalla = [p for p in PUNTAS if _depende_de_u8g2(fw, p)]
    if not con_pantalla:
        raise fw.Abortado(
            "ninguna punta declara U8g2 en su platformio.ini. O cambio la libreria de "
            "pantalla o fallo la lectura del .ini; en los dos casos este pack estaria "
            "exigiendo banderas de una libreria que ya no se enlaza")

    b.verificar(
        True,
        "puntas con pantalla u8g2: %s | uso real de buses: %s"
        % (", ".join(con_pantalla),
           ", ".join("%s=%s" % (p, sorted(usa[p]) or "ninguno") for p in PUNTAS)),
        "no deberia llegarse aqui")

    # ---- 1. El transporte de la pantalla es por software ----
    # Es la premisa de todo lo demas: si alguna pantalla pasara a HW SPI, quitar
    # U8X8_HAVE_HW_SPI dejaria de ser gratis.
    for punta in con_pantalla:
        constructores = set()
        for fichero in fw.fuentes_de(punta, "src"):
            constructores |= set(re.findall(
                r"\b(U8G2_[A-Z0-9_]+)\s+\w+\s*\(", fw.codigo(punta, "src", fichero)))
        if not constructores:
            raise fw.Abortado(
                "%s declara U8g2 pero no se halla ningun constructor U8G2_* en su src. "
                "Fallo el buscador o cambio la forma de instanciar la pantalla: sin "
                "saber el transporte, este pack no puede decir si el HW SPI sobra"
                % punta)
        b.verificar(
            all(c.endswith("_SW_SPI") for c in constructores),
            "%s dibuja por SPI de software (%s): el SPI por hardware no lo usa nadie"
            % (punta, ", ".join(sorted(constructores))),
            "%s instancia %s, que NO es SW_SPI. Con un transporte por hardware, "
            "U8X8_NO_HW_SPI deja de ser una limpieza y pasa a romper la pantalla"
            % (punta, ", ".join(sorted(constructores))))

    # ---- 2. Bus que no se usa, bandera que lo impide entrar ----
    for punta in con_pantalla:
        declaradas = _banderas_de(fw, punta)
        for bandera, bus, coste in ((BANDERAS[0], "I2C", "4.812 B de flash y 352 de RAM"),
                                    (BANDERAS[1], "SPI", "332 B de flash")):
            if bus in usa[punta]:
                continue
            b.verificar(
                bandera in declaradas,
                "%s: %s declarada, y ninguna fuente suya usa %s"
                % (punta, bandera, bus),
                "%s no declara %s y NINGUNA de sus fuentes usa %s. U8x8lib.cpp "
                "referencia el transporte igualmente y el enlazador arrastra la pila "
                "entera: %s por un bus que el equipo no tiene. No hay error ni aviso "
                "que lo delate: solo aparece cuando algo deja de caber"
                % (punta, bandera, bus, coste))

    # ---- 3. Y la direccion contraria, que es la que sirve el dia del PCF8574 ----
    # Si alguna punta empieza a usar I2C de verdad -el expansor del roadmap-, la
    # bandera tiene que SALIR. Sin esto, el sintoma seria un fallo de enlazado sin
    # causa visible, o peor, un Wire que compila y no transmite.
    for punta in PUNTAS:
        declaradas = _banderas_de(fw, punta)
        for bandera, bus in zip(BANDERAS, ("I2C", "SPI")):
            if bus not in usa[punta]:
                continue
            b.verificar(
                bandera not in declaradas,
                "%s usa %s de verdad y NO lleva %s: el transporte se enlaza"
                % (punta, bus, bandera),
                "%s usa %s en su codigo y a la vez declara %s. La libreria dejara "
                "fuera el transporte que el firmware si necesita" % (punta, bus, bandera))

    # ---- 4. La punta sin pantalla no arrastra la decision por copia ----
    # El Repetidor no depende de U8g2, asi que estas banderas no le hacen nada. Que
    # las llevara seria senal de que alguien copio un .ini sin leerlo, y ese es
    # exactamente el movimiento que hace volver el lastre a las otras dos.
    for punta in PUNTAS:
        if punta in con_pantalla:
            continue
        sobrantes = sorted(_banderas_de(fw, punta) & set(BANDERAS))
        b.verificar(
            not sobrantes,
            "%s no tiene pantalla y no arrastra banderas de u8g2" % punta,
            "%s declara %s sin depender de U8g2. No hace dano hoy, pero es la huella "
            "de un platformio.ini copiado: el siguiente copiado puede ir en la otra "
            "direccion y devolver los 5,1 KB" % (punta, ", ".join(sobrantes)))

    # ---- 5. Controles negativos ----
    b.control_negativo(
        not re.search(r"#include\s*<Wire\.h>|\bWire\s*\.\s*\w+\s*\(",
                      " el I2C por hardware esta copado: PB6/PB7 los usa la LCD "),
        "una mencion a I2C en prosa NO cuenta como uso del bus -los .ini y los .h "
        "hablan de I2C justo para explicar por que no se usa-")
    b.control_negativo(
        bool(re.search(r"\bWire\s*\.\s*\w+\s*\(", "  Wire.begin();")),
        "y una llamada de verdad si se detecta")
    b.control_negativo(
        "U8X8_NO_HW_I2C" not in set(re.findall(
            r"-D\s+([A-Za-z0-9_]+)",
            re.sub(r";[^\n]*", " ", "build_flags =\n    -D ROL_X\n    ; -D U8X8_NO_HW_I2C\n"))),
        "una bandera nombrada dentro de un comentario del .ini NO cuenta como declarada")
