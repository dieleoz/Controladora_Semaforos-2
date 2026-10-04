# Escenario P del simulador del puente: D-51 corregida y D-52 (SPEC_2 7.quater (i)). Importado por main() de
# simulador_puente_esp32.py. Valores de SPEC_2 7.quater (b), (c) y las "Firmas" (g), nunca del firmware.
# Maestro: los valores los manda el arnes (arnes_puente_salida.inc: FZR, REV, SAL). Esclavo: modo_degradado.cpp REAL.
# NO CUBRE: DEG_T con salida en el Esclavo ni sus motivos de estado (hacen falta reloj y Degradado reales: Validacion_
# Automatico/J4-J6). La SPEC dice RechazoSalida en (c) y MotivoSalida en las Firmas: aqui manda la lista de (c).

RRT = ["OK", "ROJO_FIJO_EN_ESTE_POSTE", "YA_EN_ROJO_FIJO", "SALIDA_YA_EN_CURSO",
       "OK_REANUDACION_CANCELADA"]   # RRT_* 0..4
MOTIVOS = ["PROGRAMADA", "REPROGRAMADA", "PROGRAMADA_SIN_RESPALDO", "CANCELADA", "No esta en Degradado",
           "Ya esta saliendo", "Falta: reloj sin poner en hora", "Ahora no coincide", "Salida ya vencida",
           "Salida antes del inicio", "En verde: repita en rojo", "No se pudo guardar la salida",
           "No hay salida programada"]   # MDF_* en el orden de (c)
FORMATO_MALO = ("SET_MODO:DEG_FIN:25:00:00,23:00:00", "SET_MODO:DEG_FIN:10:00:00",
                "SET_MODO:DEG_FIN:10:00:00,11:00:00x", "SET_MODO:DEG_FIN:CANCELARx")
DEG_FIN = "SET_MODO:DEG_FIN:18:25:00,18:27:00"   # 18:25:00 = el reloj del arnes, 66300 s


def _acuse(pr, linea):
    pr.rx("\n")
    return [s.strip().split("*")[0] for s in pr.rx(linea + "\n") if s.startswith(("$ACK", "$ERR"))]


def _una(pr, linea):
    r = _acuse(pr, linea)
    return r[0] if r else None


def _p1_cumple(esperado, visto):
    return esperado == visto and all("SALIENDO_TODO_ROJO" not in (v or "") for v in visto)


def _comunes(t, c, punta, pr, pin):
    """P2/P3 en las dos puntas, sin doble: formato malo, sin PIN, CONSULTA sin salida, motivo sin Degradado."""
    malas = [_una(pr, pin + o) for o in FORMATO_MALO]
    t.verificar(all(m and m.startswith("$ERR,CMD:SET_MODO:DEG_FIN") and "DESC:FORMATO_INVALIDO" in m for m in malas),
                "P2 %s: las 4 entradas de formato malo dan $ERR DEG_FIN FORMATO_INVALIDO" % punta,
                "P2 %s: DEG_FIN con formato malo contesta %s (esperado $ERR,CMD:SET_MODO:DEG_FIN,"
                "DESC:FORMATO_INVALIDO)" % (punta, malas))
    sp = _una(pr, "CMD:" + DEG_FIN)
    t.verificar(bool(sp) and "AUTH_FAILED" in sp, "P2 %s: DEG_FIN sin PIN -> AUTH_FAILED" % punta,
                "P2 %s: DEG_FIN sin PIN contesta %s" % (punta, sp))
    q = _una(pr, "CMD:CONSULTA_DEG_FIN")
    t.verificar(q == "$ACK,CMD:CONSULTA_DEG_FIN,RESULT:NINGUNA",
                "P2 %s: CONSULTA_DEG_FIN sin PIN y sin salida -> RESULT:NINGUNA" % punta,
                "P2 %s: CONSULTA_DEG_FIN contesta %s (esperado RESULT:NINGUNA)" % (punta, q))


def escenario_p(t, c, maestro, esclavo, util_max):
    t.titulo("P - D-51/D-52: respuestas de FORZAR_ROJO y de DEG_FIN (SPEC_2 7.quater (i))")
    pm, pe = c.prefijo_pin["Maestro"], c.prefijo_pin["Esclavo"]

    # P1: cada RRT_* -> su RESULT, con y sin PIN, ninguno SALIENDO_TODO_ROJO; MENU en la ventana de D-29.
    for etiqueta, linea in (("sin PIN", "CMD:FORZAR_ROJO"), ("con PIN", pm + "FORZAR_ROJO")):
        visto = []
        for cod in range(len(RRT)):
            maestro.pr.pedir("FZR %d" % cod)
            a = _una(maestro, linea)
            visto.append(a.split("RESULT:")[-1] if a else None)
        t.verificar(_p1_cumple(RRT, visto), "P1 FORZAR_ROJO %s: RRT_* 0..4 -> %s" % (etiqueta, RRT),
                    "P1 FORZAR_ROJO %s: los codigos 0..4 contestan %s (esperado %s, ninguno SALIENDO_TODO_ROJO)"
                    % (etiqueta, visto, RRT))
    maestro.pr.pedir("FZR -1")
    t.control_negativo(not _p1_cumple(RRT, ["OK", "SALIENDO_TODO_ROJO", "SALIDA_YA_EN_CURSO", "OK", "OK"]),
                       "P1 rechaza el conjunto de respuestas de hoy (con SALIENDO_TODO_ROJO)")
    for rev, esperado in ((1, "OK_REANUDACION_CANCELADA"), (0, "OK")):
        maestro.pr.pedir("REV %d" % rev)
        a = _una(maestro, "CMD:SET_MODO:MENU")
        t.verificar(a == "$ACK,CMD:SET_MODO:MENU,RESULT:" + esperado,
                    "P1 SET_MODO:MENU con revocarEsperaSiembra()=%d -> RESULT:%s" % (rev, esperado),
                    "P1 SET_MODO:MENU con revocarEsperaSiembra()=%d contesta %s (esperado %s)" % (rev, a, esperado))
    maestro.pr.pedir("REV 0")

    # P2 (Maestro): cada motivo devuelto por el doble -> su $ACK/$ERR; el $ACK solo con lo que devolvio la llamada.
    errores, acuses = {}, {}
    for m, txt in enumerate(MOTIVOS):
        if m == 3 or m == 12:
            continue
        maestro.pr.pedir("SAL %d 3 0 0" % m)
        errores[m] = _una(maestro, pm + DEG_FIN)
    ok = all(errores[m] == "$ACK,CMD:SET_MODO:DEG_FIN,RESULT:" + MOTIVOS[m] for m in (0, 1, 2)) and \
        all(errores[m] == "$ERR,CMD:SET_MODO:DEG_FIN,DESC:" + MOTIVOS[m] for m in range(4, 12))
    t.verificar(ok, "P2 Maestro: cada MotivoSalida de programar -> su $ACK (3 primeros) o su $ERR con el texto de (c)",
                "P2 Maestro: DEG_FIN contesta %s (esperado $ACK RESULT:<PROGRAMADA|REPROGRAMADA|PROGRAMADA_SIN_"
                "RESPALDO> o $ERR DESC:<texto de (c)>)" % errores)
    for m, esperado in ((3, "$ACK,CMD:SET_MODO:DEG_FIN,RESULT:CANCELADA"),
                        (12, "$ERR,CMD:SET_MODO:DEG_FIN,DESC:No hay salida programada"),
                        (10, "$ERR,CMD:SET_MODO:DEG_FIN,DESC:En verde: repita en rojo")):
        maestro.pr.pedir("SAL 0 %d 0 0" % m)
        a = _una(maestro, pm + "SET_MODO:DEG_FIN:CANCELAR")
        t.verificar(a == esperado, "P2 Maestro: CANCELAR con motivo %d -> %s" % (m, esperado),
                    "P2 Maestro: CANCELAR con motivo %d contesta %s (esperado %s)" % (m, a, esperado))
    _acuse(maestro, "CMD:HORA_ESP32:%04d-%02d-%02d,18:25:00" % (2026, 10, 4))   # otros escenarios mueven el reloj
    for z, r, esperado in ((0, 0, "NINGUNA"), (66420, 1, "SALE_182700_FALTAN_120S_RESPALDADA"),
                           (66420, 0, "SALE_182700_FALTAN_120S_SIN_RESPALDO")):
        maestro.pr.pedir("SAL 0 3 %d %d" % (z, r))
        a = _una(maestro, "CMD:CONSULTA_DEG_FIN")
        t.verificar(a == "$ACK,CMD:CONSULTA_DEG_FIN,RESULT:" + esperado,
                    "P2 Maestro: CONSULTA_DEG_FIN (salidaS %d, respaldada %d) -> %s" % (z, r, esperado),
                    "P2 Maestro: CONSULTA_DEG_FIN contesta %s (esperado RESULT:%s)" % (a, esperado))
    # P4: DEG_T con salida programada -> «Cancele antes la salida programada» (el doble de DEG_T diria FALTA_HORA).
    maestro.pr.pedir("SAL 0 3 66420 0")
    a = _una(maestro, pm + "SET_MODO:DEG_T:18:25:00,18:30:00,180,30")
    t.verificar(a == "$ERR,CMD:SET_MODO:DEG_T,DESC:Cancele antes la salida programada",
                "P4 Maestro: DEG_T con salida programada -> Cancele antes la salida programada",
                "P4 Maestro: DEG_T con salida contesta %s" % a)
    maestro.pr.pedir("SAL 0 3 0 0")
    _comunes(t, c, "Maestro", maestro, pm)

    # P3 (Esclavo, modo_degradado.cpp REAL): lo mismo que se pueda sin estado, y FORZAR_ROJO sigue renombrado.
    _comunes(t, c, "Esclavo", esclavo, pe)
    a = _una(esclavo, pe + DEG_FIN)
    t.verificar(a == "$ERR,CMD:SET_MODO:DEG_FIN,DESC:No esta en Degradado",
                "P3 Esclavo: DEG_FIN fuera del Degradado -> No esta en Degradado",
                "P3 Esclavo: DEG_FIN fuera del Degradado contesta %s" % a)
    fr = [_una(esclavo, x) for x in ("CMD:FORZAR_ROJO", pe + "FORZAR_ROJO")]
    t.verificar(all(x and x.startswith("$ERR") and "RENOMBRADO_USE_AMBAR_EMERGENCIA" in x for x in fr),
                "P3 Esclavo: FORZAR_ROJO, con y sin PIN, sigue en RENOMBRADO_USE_AMBAR_EMERGENCIA",
                "P3 Esclavo: FORZAR_ROJO contesta %s" % fr)
