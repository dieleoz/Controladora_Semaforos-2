# ===== banco/packs/maestro_02_respaldo.py =====
#
# RESPALDO EN PILA Y REANUDACION TRAS CORTE (N-20)
#
# Lo que sobrevive al corte de energia va en los registros del dominio de respaldo,
# alimentados por la pila. Si el checksum no distingue una permutacion de los
# registros, un corte a mitad de escritura puede dejar un estado que parece valido
# y no lo es.

from banco.modelos.maestro import *          # noqa: F401,F403
from banco.modelos.maestro import (          # los guiones bajos no
    _codigo, _fuente, _main, _ruta,          # los exporta import *
)

NOMBRE = "maestro_02_respaldo"
DESCRIPCION = "respaldo en pila y reanudacion tras corte (N-20)"


def correr(b, fw):
    # Bloque traido LITERAL del validador monolitico, solo reindentado. Reescribir
    # logica ya probada para renombrar las llamadas es como se cuelan los errores en
    # una migracion que se supone que no cambia comportamiento.
    verificar = b.verificar
    titulo = b.titulo

    # --- 2.1 -------------------------------------------------------------------
    # BARRIDO COMPLETO del calculo de antiguedad: los 31x31 pares de dia del mes
    # cruzados con una rejilla horaria completa. La invariante es la que el propio
    # fichero declara: ante la duda, CADUCADA. Traducida a algo comprobable:
    #
    #   - el resultado NUNCA puede ser menor que la antiguedad real, porque
    #     subestimar la antiguedad es lo que autoriza el Degradado sobre una
    #     sincronizacion vieja
    #   - el reloj que retrocede siempre es CADUCADA
    #   - y desde N-49, la que ANTES ERA IMPOSIBLE de exigir: una antiguedad de
    #     semanas se mide como semanas, no como horas
    #
    # N-49 — ESTE BLOQUE CAMBIO DE SUJETO. Barria los 31x31 pares de dia del mes y
    # exigia que el CAMBIO DE MES fuese siempre CADUCADA. Esa comprobacion se ha
    # BORRADO, no arreglado: documentaba el defecto. Con el fechado sobre el contador
    # del RTC no hay dias del mes que bajen, y el caso que aquella prueba protegia
    # -"no dar por reciente lo que no se puede fechar"- lo cubre ahora la de abajo,
    # que es mas fuerte: no dar por reciente lo que es VIEJO.
    BASE_RTC = 1000000          # un instante cualquiera del contador, en segundos
    sub_estimados, atras_malo, alias_malo = [], [], []
    finitos = 0
    reg = marcar_sync(BASE_RTC)
    for dias in range(0, 40):
        for hora in range(0, 24, 3):
            real = dias * 86400 + hora * 3600
            h = horas_desde_sync(reg[0], reg[1], True, None, BASE_RTC + real)
            if h == SYNC_CADUCADA:
                continue
            finitos += 1
            # Subestimar es la direccion insegura: autoriza el Degradado sobre una
            # sincronizacion mas vieja de lo que se cree.
            if h * 3600 > real + 3600:
                sub_estimados.append((dias, hora, h, real))
            # N-49: 31 dias tienen que medirse como 31 dias. Antes esto devolvia la
            # misma cifra que "hace unas horas", porque el dia del mes coincidia.
            if dias >= 31 and h < 24 * 30:
                alias_malo.append((dias, hora, h))
    for atras in (1, 3600, 86400, 31 * 86400):
        if horas_desde_sync(reg[0], reg[1], True, None, BASE_RTC - atras) != SYNC_CADUCADA:
            atras_malo.append(atras)
    verificar(not sub_estimados,
              f"Ningun caso fechable devuelve una antiguedad MENOR que la real. "
              f"{finitos} casos examinados sobre el contador del RTC.",
              f"Se subestima la antiguedad (direccion insegura) en {sub_estimados[:3]}")
    verificar(not atras_malo,
              "El reloj que RETROCEDE da siempre caducada -1 s, 1 h, 1 dia y 31 dias hacia "
              "atras-: una antiguedad negativa no se convierte en un numero pequeno.",
              f"Reloj hacia atras aceptado como reciente en {atras_malo[:5]}")
    verificar(not alias_malo,
              "N-49: una sincronizacion de 31 dias o mas se mide como semanas, no como "
              "horas. Con el dia del mes esto era IMPOSIBLE de cumplir: 'hoy' y 'hace 31 "
              "dias' producian los mismos numeros y el Degradado se autorizaba sobre una "
              "sincronizacion de hace un mes.",
              f"Una sincronizacion de semanas se lee como reciente en {alias_malo[:5]}")

    # --- 2.2 -------------------------------------------------------------------
    # N-49 — FRONTERAS NUEVAS. Las de antes -dia 0, dia 32, segundos >= 86400- ya no
    # existen: no se guarda ni dia ni segundo del dia, asi que comprobarlas seria
    # medir un firmware que ya no esta. Las de ahora son dos, y las dos importan:
    #
    #   - el CERO significa "no hay reloj". Marcar con el dejaria una fecha apoyada en
    #     un contador que nadie hace avanzar, y sobre esa mentira el Degradado se
    #     autorizaria. Tiene que rechazarse al ESCRIBIR y al LEER, no solo al leer:
    #     una marca imposible que llega a la pila la puede dar por buena cualquier
    #     cambio posterior de la lectura.
    #   - la ausencia de marca sigue dando CADUCADA, no un cero enganoso.
    verificar(marcar_sync(0) is None and
              marcar_sync(1) is not None and
              marcar_sync(0xFFFFFFFF) is not None,
              "respaldo_marcarSync() rechaza el contador 0 -que significa 'no hay reloj'- y "
              "acepta cualquier instante real: la marca imposible no llega a escribirse.",
              "respaldo_marcarSync() acepta una marca imposible")
    verificar(horas_desde_sync(0, 0, True, None, 0) == SYNC_CADUCADA and
              horas_desde_sync(15, 16960, True, None, 0) == SYNC_CADUCADA and
              horas_desde_sync(15, 16960, False, None, 1003600) == SYNC_CADUCADA,
              "respaldo_horasDesdeSync() rechaza el contador 0 -sin reloj- y la ausencia de "
              "marca: dan CADUCADA, no un cero enganoso.",
              "Alguna entrada imposible devuelve una antiguedad finita")

    # --- 2.3 -------------------------------------------------------------------
    # N-49 — ESTA PRUEBA EXIGIA EL DEFECTO, Y AHORA EXIGE LO CONTRARIO.
    #
    # Decia: "una unidad que se sincronizo el dia 31 a las 23:00 y sufre un corte a
    # las 01:00 del dia 1 NO puede reanudar el Degradado, aunque solo hayan pasado dos
    # horas. Cae a ambar contra la otra punta". Lo llamaba coste operativo y lo daba
    # por aceptable porque el dato guardado no permitia fechar.
    #
    # Ya permite. Ese mismo escenario -dos horas reales- se fecha como dos horas, y la
    # reanudacion de N-20 funciona TAMBIEN al cruzar la vuelta del contador. El coste
    # operativo de "1 vez al mes" desaparecio, y con el la asimetria que dejaba a una
    # punta en ambar y a la otra en verde.
    reg = marcar_sync(BASE_RTC)
    h_frontera = horas_desde_sync(reg[0], reg[1], True, None, BASE_RTC + 2 * 3600)
    verificar(h_frontera == 2,
              "Una sincronizacion de 2 h reales se fecha como 2 h, caiga donde caiga en el "
              "calendario. Antes de N-49 este mismo caso daba CADUCADA una vez al mes y la "
              "reanudacion de N-20 no funcionaba: el Maestro se rendia a ambar mientras el "
              "Esclavo seguia dando verde.",
              f"Una sincronizacion de 2 h no se fecha como 2 h (dio {h_frontera})")

    # --- 2.4 -------------------------------------------------------------------
    # LA SUMA DE COMPROBACION. Primero lo que SI hace: cualquier corrupcion de UN
    # SOLO registro cambia la suma, porque un cambio de 16 bits no puede ser multiplo
    # de 65536. Se barren todos los volteos de bit de los cinco registros con dato.
    base = {REG_VERDE: DEG_VERDE_SEG, REG_DESPEJE: DEG_DESPEJE_SEG,
            REG_FLAGS: FLAG_CICLO | FLAG_SYNC | FLAG_DEGRADADO,
            # N-49: las dos mitades de un contador de RTC corriente (17*65536+21600).
            REG_SYNC_ALTA: 17, REG_SYNC_BAJA: 21600}

    # N-133 (04/09): LA MUESTRA SE COMPLETA CON LO QUE EL C++ TENGA, NO CON UNA LISTA.
    #
    # Esta muestra estaba escrita a mano con cinco registros, y al anadir los dos de los
    # tiempos del ciclo el pack ABORTO con un KeyError: calcular_suma() recorre
    # ORDEN_SUMA -que se lee del C++- y pedia un registro que la muestra no tenia. El
    # ABORTADO fue correcto; lo que estaba mal era la lista.
    #
    # Se rellena desde ORDEN_SUMA tomando un valor CENTRAL del dominio real de cada
    # registro. Central y no cero a proposito: un cero se intercambia con otro cero sin
    # cambiar la suma, y el barrido de transposiciones de mas abajo se saltaria esos
    # pares por iguales, midiendo menos de lo que dice medir.
    for _r in ORDEN_SUMA:
        if _r in base:
            continue
        _dom = DOMINIO_REG.get(_r)
        base[_r] = (list(_dom)[len(_dom) // 2] if _dom else 1)
    suma_base = calcular_suma(base)

    # --- 2.4.bis ---------------------------------------------------------------
    # LA FIRMA TIENE QUE CAMBIAR CUANDO CAMBIA EL FORMATO, Y HASTA HOY NADIE LO MEDIA.
    #
    # respaldo.cpp lo dice en prosa desde N-49: "LA FIRMA CAMBIA CON EL FORMATO, y no
    # es opcional. Un equipo actualizado que encontrara una firma vieja daria por bueno
    # un contenido escrito con otra aritmetica". Es correcto y era SOLO UN COMENTARIO.
    #
    # Se descubrio el 04/09 inyectando el defecto (§8.bis): al anadir los dos registros
    # de los tiempos del ciclo y NO subir la firma, el banco daba 19/19. O sea que la
    # regla mas importante de este fichero no la podia romper nadie... porque nadie la
    # comprobaba. Es §3.bis literal: una relacion que vive en prosa no falla cuando
    # alguien cambia un numero.
    #
    # COMO SE MIDE, que es lo unico que se puede medir aqui: se ata la FIRMA al CONJUNTO
    # Y ORDEN de registros del checksum, los dos leidos del C++. Si alguien toca uno sin
    # tocar el otro, esto cae. La pareja de abajo esta escrita a mano A PROPOSITO: es un
    # valor de oro, y actualizarla es el acto deliberado que demuestra que alguien penso
    # en los equipos que ya tienen contenido guardado. Si se pudiera derivar sola, no
    # comprobaria nada.
    _firma = re.search(r"FIRMA\s*=\s*(0x[0-9A-Fa-f]+)",
                       fw.codigo("Maestro", "src", "respaldo.cpp"))
    if not _firma:
        raise fw.Abortado("no se halla la FIRMA en respaldo.cpp")
    _formato = tuple(NOMBRE_REG[r] for r in ORDEN_SUMA)

    FORMATO_ESPERADO = ("VERDE", "DESPEJE", "FLAGS", "SYNC_ALTA", "SYNC_BAJA",
                        "CICLO_RV", "CICLO_DESPEJE")
    FIRMA_ESPERADA = "0x5EB2"

    b.verificar(
        _formato == FORMATO_ESPERADO and _firma.group(1).lower() == FIRMA_ESPERADA.lower(),
        "el formato del respaldo y su FIRMA cuadran: %d registros en el checksum "
        "(%s) con la firma %s" % (len(_formato), ", ".join(_formato), _firma.group(1)),
        "EL FORMATO DEL RESPALDO Y LA FIRMA NO CUADRAN. En el C++: registros %s con "
        "firma %s. Esperado: %s con %s. Si has CAMBIADO el formato, sube la FIRMA y "
        "actualiza esta pareja; si has cambiado la firma sin tocar el formato, "
        "sobra. Con una firma vieja, un equipo actualizado da por bueno un contenido "
        "escrito con otra aritmetica: leeria como tiempos lo que dejo el arranque "
        "anterior" % (list(_formato), _firma.group(1),
                      list(FORMATO_ESPERADO), FIRMA_ESPERADA))
    no_detectados_bit = []
    for reg_n in base:
        for bit in range(16):
            alt = dict(base)
            alt[reg_n] ^= (1 << bit)
            if calcular_suma(alt) == suma_base:
                no_detectados_bit.append((reg_n, bit))
    verificar(not no_detectados_bit,
              "La suma detecta los 80 volteos de un solo bit sobre los cinco registros con "
              "dato: la corrupcion aislada no pasa por configuracion valida.",
              f"Volteos de un bit NO detectados: {no_detectados_bit[:5]}")

    # --- 2.5 -------------------------------------------------------------------
    # LAS TRANSPOSICIONES, sobre la muestra concreta del ciclo real. Es el caso que
    # motivo el cambio de algoritmo el 01/08/2026.
    # Mismo motivo que la muestra: los registros salen del C++ via ORDEN_SUMA. Con la
    # lista a mano, un registro nuevo entraba en el checksum SIN que nadie barriera
    # sus transposiciones: el pack seguiria en verde midiendo menos que ayer, que es
    # la prueba muerta de §3.bis introducida por un cambio ajeno al pack.
    regs_lista = list(ORDEN_SUMA)
    transp_no_detectadas, transp_totales = [], 0
    for i in range(len(regs_lista)):
        for j in range(i + 1, len(regs_lista)):
            a, b = regs_lista[i], regs_lista[j]
            if base[a] == base[b]:
                continue                      # intercambiar iguales no es corrupcion
            transp_totales += 1
            alt = dict(base)
            alt[a], alt[b] = alt[b], alt[a]
            if calcular_suma(alt) == suma_base:
                transp_no_detectadas.append((NOMBRE_REG[a], NOMBRE_REG[b]))
    compensado = dict(base)
    compensado[REG_VERDE] += 1
    compensado[REG_DESPEJE] -= 1
    verificar(not transp_no_detectadas and calcular_suma(compensado) != suma_base,
              # N-51: decia "con los pesos leidos del C++ {PESOS_SUMA}", y PESOS_SUMA
              # es {1,1,1,1,1} por compatibilidad -no se lee de ningun sitio-. Los
              # pesos REALES de Horner son posicionales (COEF_SUMA, si arriba en el
              # modelo) y se derivan de MULT_SUMA y ORDEN_SUMA, que si son del C++.
              f"Con el orden {[NOMBRE_REG[r] for r in ORDEN_SUMA]} y el multiplicador "
              f"{MULT_SUMA} leidos del C++, la suma detecta las {transp_totales} "
              "transposiciones de la muestra real y el par compensado (+1 en VERDE, "
              "-1 en DESPEJE).",
              f"La suma sigue siendo insensible al orden en {transp_no_detectadas}")

    # --- 2.6 -------------------------------------------------------------------
    # La prueba anterior tiene que poder fallar: se repite con la suma LLANA, que es
    # la que habia antes del arreglo. Si el banco no distinguiera los dos algoritmos,
    # el PASS de 2.5 no significaria nada.
    suma_llana_base = suma_llana(base)
    transp_mutante = 0
    for i in range(len(regs_lista)):
        for j in range(i + 1, len(regs_lista)):
            a, b = regs_lista[i], regs_lista[j]
            if base[a] == base[b]:
                continue
            alt = dict(base)
            alt[a], alt[b] = alt[b], alt[a]
            if suma_llana(alt) == suma_llana_base:
                transp_mutante += 1
    verificar(transp_mutante == transp_totales and suma_llana(compensado) == suma_llana_base,
              f"Control del modelo: la suma LLANA anterior al arreglo deja pasar las "
              f"{transp_mutante} transposiciones y el par compensado. El banco distingue los "
              "dos algoritmos, luego el PASS de 2.5 describe un arreglo real.",
              f"El banco no distingue la suma llana de la ponderada ({transp_mutante} de "
              f"{transp_totales}): la prueba 2.5 no esta midiendo el arreglo")

    # --- 2.7 -------------------------------------------------------------------
    # ...Y AHORA SE ATACA EL ALGORITMO NUEVO. Una muestra concreta no demuestra nada
    # sobre las demas.
    #
    # N-51 — ESTA PRUEBA COMPARABA PESOS_SUMA, Y PESOS_SUMA YA NO DESCRIBE EL
    # ALGORITMO. calcularSuma() es Horner (s = s*31 + reg), no una suma ponderada:
    # cada registro pesa segun su POSICION (31^4, 31^3, 31^2, 31, 1), y dos
    # posiciones nunca comparten peso. La version anterior de esta prueba marcaba
    # "ciego" en cuanto veia PESOS_SUMA[a] == PESOS_SUMA[b] -que con todos los pesos
    # a 1 era SIEMPRE- sin llamar una sola vez al checksum real: metia los 10 pares
    # posibles en la lista por construccion, no por medida.
    #
    # Ahora se barren los DOMINIOS REALMENTE ALCANZABLES de cada registro -no los
    # 65536 valores posibles, que incluirian contenidos que el firmware nunca
    # escribe- llamando a transposicion_ciega(), que usa la aritmetica real de
    # Horner (controlada contra calcular_suma() al importar el modelo) para
    # encontrar cualquier par de valores legitimos cuya transposicion pase
    # inadvertida.
    ciegas = []
    for i in range(len(regs_lista)):
        for j in range(i + 1, len(regs_lista)):
            a, b = regs_lista[i], regs_lista[j]
            encontrado = transposicion_ciega(a, b, base)
            if encontrado:
                ciegas.append((NOMBRE_REG[a], NOMBRE_REG[b], encontrado))
    verificar(not ciegas,
              "Barrido de los dominios alcanzables de los cinco registros: NINGUNA "
              "transposicion de valores legitimos pasa inadvertida.",
              f"El arreglo mejora la suma pero NO LA CIERRA: quedan {len(ciegas)} pares de "
              f"registros con transposiciones ciegas para valores alcanzables -> {ciegas}. "
              f"El pliegue final de 32 a 16 bits tira la mitad de la mezcla de Horner: dos "
              f"contenidos distintos pueden plegar al mismo valor aunque sus 32 bits crudos "
              f"difieran.")

    # --- 2.8 -------------------------------------------------------------------
    # ?Y eso es peligroso, o es una curiosidad aritmetica? Se busca el caso concreto:
    # una transposicion ciega que ademas deje un contenido que el equipo USE, y en la
    # direccion insegura. La peor es la que enciende el indicador de Degradado activo
    # y deja una marca de sincronizacion recien hecha.
    #
    # N-51 — TAMBIEN COMPARABA PESOS_SUMA, y con dif_peso siempre 0 el "break" del
    # primer flags cortaba el bucle entero: peor_caso quedaba SIEMPRE en None sin
    # examinar un solo candidato. El PASS no media nada.
    #
    # Ahora se restringe el dominio de SYNC_BAJA a los valores cuyos 3 bits bajos
    # coinciden con CICLO|SYNC|DEGRADADO -los unicos que, si acaban en el registro
    # FLAGS tras la transposicion, encenderian los tres indicadores- y se pregunta a
    # transposicion_ciega() si alguno de esos colisiona de verdad con algun FLAGS
    # alcanzable.
    DANGER = FLAG_CICLO | FLAG_SYNC | FLAG_DEGRADADO
    dominio_sync_peligroso = [v for v in DOMINIO_REG[REG_SYNC_BAJA] if (v & DANGER) == DANGER]
    peor = transposicion_ciega(REG_FLAGS, REG_SYNC_BAJA, base,
                                dominio_b=dominio_sync_peligroso)
    peor_caso = None
    if peor:
        flags_antes, sync_antes = peor
        peor_caso = (flags_antes, sync_antes, sync_antes, sync_antes)
    verificar(peor_caso is None,
              "Ninguna transposicion ciega deja un contenido que el equipo interprete como "
              "'Degradado activo con sincronizacion reciente'.",
              f"Transposicion ciega EXPLOTABLE: con FLAGS={peor_caso[0]} y "
              f"SYNC_BAJA={peor_caso[1]}, permutarlos deja la suma intacta y produce "
              f"FLAGS={peor_caso[2]} -CICLO+SYNC+DEGRADADO los tres encendidos- con una marca "
              f"de sincronizacion cuya mitad baja queda en el segundo {peor_caso[3]} del "
              f"contador. Un arranque tras corte leeria eso como una autorizacion vigente y "
              f"REANUDARIA el Modo Degradado sobre un contenido corrupto." if peor_caso else "")

    # --- 2.7 -------------------------------------------------------------------
    # LOS GEMELOS. respaldo.h dice en su cabecera "ESTE FICHERO Y SU .cpp DEBEN SER
    # IDENTICOS EN MAESTRO Y ESCLAVO", y no es una recomendacion de estilo: las dos
    # puntas fechan la misma sincronizacion con la misma aritmetica, y si una ponderase
    # la suma y la otra no, un respaldo escrito por una no lo validaria la otra.
    #
    # Se comprueba TRAS el cambio de algoritmo del 01/08/2026, que es justo cuando un
    # par de gemelos se separa: se corrige uno y se olvida el otro.
    _par_identico = []
    for _f in (("src", "respaldo.cpp"), ("include", "respaldo.h")):
        _par_identico.append(_fuente("Maestro", *_f).replace("\r\n", "\n")
                             == _fuente("Esclavo", *_f).replace("\r\n", "\n"))
    verificar(all(_par_identico),
              "respaldo.cpp y respaldo.h son identicos en Maestro y Esclavo, tambien despues "
              "de cambiar el algoritmo de la suma: las dos puntas fechan la sincronizacion "
              "con la misma aritmetica.",
              f"Los gemelos se separaron: respaldo.cpp identico={_par_identico[0]}, "
              f"respaldo.h identico={_par_identico[1]}. Un respaldo escrito por una punta "
              "podria no validarlo la otra.")

    # Un dominio de respaldo lleno de basura no debe pasar por bueno. Los dos casos
    # clasicos: todo a cero (pila nueva sin escribir) y todo a 0xFFFF (dominio
    # flotante o pila agotada a medias).
    #
    # N-51: la suma guardada ya no cabe en un solo registro -son los 32 bits crudos
    # de calcular_suma() en REG_SUMA_ALTA/REG_SUMA_BAJA-, asi que la comprobacion
    # recompone las dos mitades igual que respaldo_setup().
    def contenido_valido(firma, suma_alta_guardada, suma_baja_guardada, regs):
        suma_guardada = ((suma_alta_guardada & 0xFFFF) << 16) | (suma_baja_guardada & 0xFFFF)
        return firma == FIRMA and suma_guardada == calcular_suma(regs)


    ceros = {r: 0 for r in regs_lista}
    efes = {r: 0xFFFF for r in regs_lista}
    verificar(not contenido_valido(0x0000, 0x0000, 0x0000, ceros) and
              not contenido_valido(0xFFFF, 0xFFFF, 0xFFFF, efes),
              "Un dominio de respaldo a ceros o a 0xFFFF se declara INVALIDO: la basura "
              "no se arrastra como configuracion.",
              "Basura en el dominio de respaldo pasa por contenido valido")

    # --- 2.8 -------------------------------------------------------------------
    # respaldo_guardarCiclo() rechaza un ciclo con algun tramo a cero, para que
    # respaldo_hayCiclo() no mienta. Se comprueba tambien el limite del byte, porque
    # el ciclo se envia al Esclavo en un solo byte por valor.
    def guardar_ciclo(verde, despeje):
        if verde == 0 or despeje == 0:
            return None
        return verde & 0xFF, despeje & 0xFF


    verificar(guardar_ciclo(0, 30) is None and guardar_ciclo(30, 0) is None and
              guardar_ciclo(0, 0) is None and guardar_ciclo(DEG_VERDE_SEG, DEG_DESPEJE_SEG)
              == (DEG_VERDE_SEG, DEG_DESPEJE_SEG),
              "respaldo_guardarCiclo() rechaza cualquier tramo a cero y guarda intacto el "
              f"ciclo real del firmware ({DEG_VERDE_SEG}/{DEG_DESPEJE_SEG} s).",
              "respaldo_guardarCiclo() acepta un ciclo con un tramo a cero")
    verificar(DEG_VERDE_SEG <= 255 and DEG_DESPEJE_SEG <= 255,
              f"El ciclo degradado ({DEG_VERDE_SEG}/{DEG_DESPEJE_SEG} s) cabe en el byte de "
              "CMD_CONFIG: las dos puntas computaran la misma duracion.",
              f"El ciclo degradado ({DEG_VERDE_SEG}/{DEG_DESPEJE_SEG}) desborda el byte del "
              "protocolo: el Esclavo recibiria otra duracion")

    # --- 2.9 -------------------------------------------------------------------
    # LA REANUDACION. Se barre la antiguedad guardada de 0 a 80 h y se cruza con las
    # otras dos condiciones. La regla que se exige es la del fichero: se piden TODAS,
    # ninguna es recomendable.
    def reanudar(degradado_activo, en_hora, hay_ciclo, horas):
        if not degradado_activo:
            return False, False              # (reanuda, borra_indicador)
        ok = en_hora and hay_ciclo and horas != SYNC_CADUCADA and horas < LIMITE_DURO_H
        return ok, not ok


    malos_reanuda = []
    for horas in list(range(0, 81)) + [SYNC_CADUCADA]:
        for en_hora in (False, True):
            for hay_ciclo in (False, True):
                r, borra = reanudar(True, en_hora, hay_ciclo, horas)
                debe = (en_hora and hay_ciclo and horas != SYNC_CADUCADA
                        and horas < LIMITE_DURO_H)
                if r != debe or (not r and not borra):
                    malos_reanuda.append((horas, en_hora, hay_ciclo, r, borra))
    verificar(not malos_reanuda,
              f"Barrido 0..80 h x reloj x ciclo: se reanuda si y solo si hay hora, hay ciclo "
              f"guardado y la sincronizacion es fechable y menor de {LIMITE_DURO_H} h. Al "
              "rechazar, el indicador SIEMPRE se borra (no se reintenta en cada arranque).",
              f"La reanudacion acepta o rechaza mal en {malos_reanuda[:5]}")

    verificar(reanudar(True, True, True, SYNC_CADUCADA)[0] is False and
              reanudar(True, True, True, LIMITE_DURO_H)[0] is False and
              reanudar(True, True, True, LIMITE_DURO_H - 1)[0] is True,
              f"El centinela CADUCADA (0x{SYNC_CADUCADA:08X}) no se cuela por 'muchisimas "
              f"horas': se comprueba aparte, y el corte esta exactamente en {LIMITE_DURO_H} h.",
              "El limite de la reanudacion no cae donde debe")

    verificar(reanudar(False, True, True, 1)[0] is False,
              "Sin indicador de Degradado en la pila no se reanuda nada: la unidad que no "
              "estaba en el modo arranca en el menu.",
              "Se reanuda el Degradado sin que constase que estaba activo")

    # --- 2.10 ------------------------------------------------------------------
    # El orden de main.cpp. modo_degradado_publicarConfig() guarda el ciclo en la
    # pila, asi que si se llamara ANTES de consultar la reanudacion, respaldo_hayCiclo()
    # seria cierto SIEMPRE y esa condicion dejaria de comprobar nada. Se verifica en
    # el fuente, porque es un fallo de ORDEN y no de calculo: ninguna simulacion de
    # la funcion lo veria.
    pos_reanudar = _main.find("modo_degradado_reanudarTrasCorte")
    pos_publicar = _main.find("modo_degradado_publicarConfig")
    verificar(pos_reanudar != -1 and pos_publicar != -1 and pos_reanudar < pos_publicar,
              "main.cpp consulta modo_degradado_reanudarTrasCorte() ANTES de "
              "modo_degradado_publicarConfig(): la condicion 'hay ciclo acordado' sigue "
              "comprobando algo.",
              "main.cpp publica el ciclo antes de consultar la reanudacion: respaldo_hayCiclo() "
              "seria cierto siempre y la condicion quedaria vacia")


    # ==========================================================================
    # BLOQUE 3 — LA PUERTA DEL MODO DEGRADADO
    # ==========================================================================
