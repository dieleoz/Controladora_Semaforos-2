# Auditoria N-182 - Interruptores de camara y barrera (`D-94..D-97`, `D-148..D-152`)

Rama: `auditoria/n182` sobre `main` actual (`edb2462`, incluyendo integracion previa de los 7
hallazgos iniciales en `9b52f11..e990789` + `d77695a`, y mejoras posteriores en `SPEC_8`,
`costura_01_contratos.py` y `dom_interruptores.js`).

## 1. Hallazgos resueltos (N-182 base + mejoras en `main` y `auditoria/n182`)

1. **`NO_QUEDO_PUESTO` sin traducir (`js/interruptores.js`, `textos_err.js` tenia `NO_QUEDO_PUESTA`)**
   - Propuesta/cambio: traduccion en `Interruptores.RECHAZOS` y caso `I7` (`main`: `4c709fd`).
   - Prueba: `I7` en `test_dom_execution.js`, verde dentro de `949/949 PASS`.
2. **Cabecera falsa y `$ERR` sin argumento en `tests/dom_interruptores.js`**
   - Propuesta/cambio: cabecera y `I7` con literal del firmware (`4c709fd`); refuerzo `I6`/`I9` (`1640ed0`).
   - Prueba (rojo y verde): `I6` antes usaba `/veto/i` (aprobaba `OFF`: `true` vs `/DEVUELTO/i: false`);
     verde en `test_dom_execution.js`: `949/949 PASS`.
3. **Despachador del Maestro sin ejercicio C++ real para interruptores (`bluetooth.cpp`)**
   - Propuesta/cambio: `P6` en `escenario_p_salida.py` + memoria en RAM en `arnes_puente_testigo2.inc` +
     `arnes_app.js` con `NODE_PATH` (`main`: `a6996bc` + `d77695a`).
   - Prueba (rojo y verde): rojo sin estado en RAM `159/161` (`NO_QUEDO_PUESTO`); verde `164/164 PASS`.
4. **`barrera_03_talanquera.py` permisivo en `luzPideArriba` del Esclavo**
   - Propuesta/cambio: lista cerrada `{verde, testLedsActivo, estado, S_FALLO, S_AMARILLO, S_AMBAR_INT,
     S_ROJO_INT}` + `mF` (`main`: `3c29e44`).
   - Prueba (rojo y verde): rojo inyectando `camara_presenciaJ16()` en `Esclavo/semaforo.cpp`: `17/19`;
     verde tras restaurar por SHA-256: `19/19 PASS`.
5. **`Validacion_Respaldo/compilar.ps1` usaba gcc de ruta no-ASCII y salia `1` en error de entorno**
   - Propuesta/cambio: anteponer `D:\toolchain\mingw64\bin` y `exit 2` con `[ABORTADO]` (`main`: `47b4060`).
   - Prueba (rojo y verde): rojo en `bash.exe -lc` salida `1` sin `crt2.o`; verde salida `0`, `D-62 5/5`.
6. **`.claude/rondas.json` sin ignorar ensucia el sello (`FW:<hash>+SUCIO`)**
   - Propuesta/cambio: entrada en `.gitignore` (`main`: `9b52f11`).
   - Prueba: rojo `?? .claude/rondas.json`; verde arbol limpio.
7. **Fallo intermitente `[FAIL] 15: D-78 ... toast false` (`tests/dom_textos_spec4.js`)**
   - Propuesta/cambio: `setTimeout(r, 5)` -> `setTimeout(r, 0)` en D-78 (`main`: `ab76cca`), mismo bucket
     FIFO `timerListMap[1]` de Node que `Testigo.anotarAcuse()`.
   - Prueba: rojo con timer previo de 5 ms tras tramo sincrono: `[FAIL] 15`; verde: `125/125 PASS`.
8. **Items 10-11 y Modo Practica (`js/simulador.js`, `tests/dom_simulador.js`)**
   - Propuesta/cambio: `ordenes()`/`vocabulario()` leen `interruptores.cpp` (`%s,CMD:%s%s%s,%s`) y el
     simulador contesta `D-94` (`main`: `666d904`, `6242f8f`, `e990789`).
   - Prueba: rojo antes `910/912` (`2` fallas); verde `949/949 PASS` y `app_01_comandos`: `12/12 PASS`.
9. **Condicion `C1` (Arquitecto): `costura_01_contratos.py` ciego ante `interruptores` e `instalacion`**
   - Propuesta/cambio: incluidos `interruptores.{h,cpp}` e `instalacion.{h,cpp}` en `COMPARTIDOS`
     (`208ed6c`, `-17` lineas netas).
   - Prueba: rojo mutando `Esclavo/src/interruptores.cpp`: antes `7/7`, luego `6/7 FALLA DIVERGEN`;
     verde tras restaurar SHA-256: `7/7 PASS`.
10. **Condicion `C2` y erratas de `SPEC_8` (Arquitecto + QA `D4`)**
    - Propuesta/cambio: corregida formula de `luzPideArriba` (`SPEC_8:42`), `:<valor>` en `$ACK` (`:220`) y
      regla real de apertura de «Averias» bajo `D-151`/`D-60` (`:298`, commit `63a45fb`); anadido control
      `CAM:PEGADA` en `dom_interruptores.js` (`1640ed0`).
    - Prueba: `spec_01_no_promete`: `11/11 PASS`; `test_dom_execution.js`: `949/949 PASS`.

## 2. Propuestas pendientes para el responsable (sin decidir conducta de luces/pluma)

### Punto 8 - Decisiones abiertas de conducta y hardware
1. **8a - Arranque del Esclavo tras corte con `FLAG_BARRERA_FUERA`**
   (`orquestador_interruptores.inc` `S3`, `Esclavo/src/main.cpp`):
   En el Esclavo, `semaforo_setup()` y `semaforo_forzarRojo()` corren en `main.cpp:173-175` antes de
   `respaldo_setup()` (`main.cpp:250`). Si vuelve la energia con `FLAG_BARRERA_FUERA` activa y no cambia
   la luz (en `S_ROJO` estable antes de recibir `CMD_GREEN` o llegar al ambar `SFTY-6` a los 12 s), la
   pluma queda ABAJO y `$STATUS` publica `PLUMA:ABAJO` (`S3`), hasta un cambio de luz o reenvio de
   `SET_BARRERA_SERVICIO:OFF` (`S3b`, `D-149`). Pregunta al responsable: reentrar tras
   `respaldo_setup()` para subir la barrera al arrancar, o dejarlo como esta hasta el nuevo `J15`
   (`D-168`: reposo/sin energia = ARRIBA en `SPEC_8` §8.2, hoy `HUECO`). Modo de fallo mientras siga
   como hoy: contradice el texto de arranque en la app (`js/interruptores.js:72-73`: «el brazo esta
   arriba tambien en rojo»).
2. **8b - `C5`: ventana de bajada lenta (~60 s) sin freno de camara**
   (`semaforo.cpp` `escribirPines()`, `SPEC_8` §1):
   Una vez que `escribirPines()` entra a `!plumaAbierta` (`Maestro/src/semaforo.cpp:250`,
   `Esclavo/src/semaforo.cpp:224`), cierra la pluma en seco y deja de consultar `camara_presenciaJ16()`.
   Con un brazo de ~60 s de recorrido por final de carrera interno de la centralita (`SPEC_8` §1 y §7
   fila 7, sin medir), una moto o coche que entre bajo el brazo ya bajando no activa ningun freno ni
   reapertura. Pregunta abierta (`roadmap.md` fila 6): esperar medicion real del recorrido (hoja 10/11
   prueba 4) y definir politicas / anticipos / sensor de fin de carrera (requiere hardware).
3. **8c - Granularidad de camaras (`D-166`) y «barrera ABAJO» desde la app**:
   `N-182` implementa un unico interruptor `SET_VETO_CAMARA` para ambas camaras (`SPEC_8` §2). El
   requisito posterior `D-166` (inhibir UNA camara C/D sin anular la otra) ya esta en `roadmap.md`
   en construccion tras `D-167`. Sobre «barrera ABAJO fija» desde la app, `D-96` solo da ARRIBA y
   `roadmap.md:129-130` recomienda no crearla porque el rojo fijo ya la baja.

### Punto 9 - El Sisga (`179DB0`): limite duro de 48 h (`LIMITE_DURO_MS`)
- **Hipotesis NO medida (`ESTADO.md:42-43`)**: el ambar del fin de semana en El Sisga corresponde al
  tope de 48 h sin sincronizar por radio (`LIMITE_DURO_MS = 172800000UL` en
  `Maestro/src/modo_degradado.cpp` y `LIMITE_SIN_SYNC_MS = 48 * 3600 * 1000UL` en Esclavo). En el
  binario instalado (`7ff7d12` / `b354fe9`, anterior al testigo de `990c278` y a los `$ALARM`
  repetidos de `D-67`/`D-68`) existia ese tope duro sin testigo: al cumplirse 48 h desde
  `msDesdeSyncEfectivo()`, el Maestro pasaba a `DEG_AMBAR` (borrando `respaldo_guardarDegradado(false)`)
  y el Esclavo pasaba a `DEG_RENDIDO`.
- **Medida que lo confirma o refuta (no existe captura de campo de ese paso en `evidencia/`)**:
  1. En **Esclavo** (`$STATUS`): `MODO:RENDIDO,ESTADO:FALLO COM` (`obtenerNombreModo()` para
     `DEG_RENDIDO`) **confirma** rendicion por limite de 48 h; `MODO:DEGRADADO` o `MODO:SUBORDINADO`
     con `FALLO COM` la **refuta**.
  2. En **Maestro** (`$STATUS` en `7ff7d12`): `MODO:DEGRADADO,ESTADO:FALLO COM,HORA:<HH:MM:SS>` con
     `HORA` valida y diferencia >= 48 h respecto a la ultima sync RF confirma `DEG_AMBAR` (mientras
     `MODO:AMBAR` indica ambar de arranque `D-40` o pedido por app; `MODO:AUTO,ESTADO:FALLO COM` indica
     perdida de radio sin salir de ciclo).
  3. **Firmware actual (`edb2462`)**: sin testigo (`!testigo`) repite cada minuto `$ALARM,NODE:<punta>,
     ORIGEN:DEGRADADO,CAUSA:LIMITE_48H,ACCION:CAMBIO_A_AMBAR` (o `SYNC_SIN_FECHA` / `RELOJ_NO_CUENTA`)
     y traba `FLAG_RENDIDO`; **con testigo** (`SET_MODO:DEG_T:...`, `990c278` + H9), `!testigo` es falso
     y el tope de 48 h **no actua** (aviso diario tras 28 dias, manteniendo `HORA_ESP32 CADUCADA -> ROJO`).

## 3. Pendiente de instrumentacion y tarjeta (condiciones de Arquitecto y QA)
- **Packs con puntos ciegos fuera de `bluetooth.cpp` (`C1` / `D3` / `D5`)**: tras delegar en
  `interruptores_orden()` e `instalacion_orden()`, `app_08_enrutado_por_punta.py`,
  `app_03_sin_ok_mudo.py` y `app_10_ack_con_varios_sies.py` siguen leyendo solo `bluetooth.cpp` /
  `procesarComando()`, y `app_01_comandos.py` valida por prefijo sin `js/interruptores.js`. Queda
  propuesto ampliarlos cuando se toque la arquitectura comun (`lib/Common`).
- **Ramas C++ sin ejercicio dinamico propio (`D2`)**: `$ERR ... DESC:NO_QUEDO_PUESTO` y los
  `$EVENT` de arranque `ARRANQUE_CAM_NOVETO` y `ARRANQUE_CAM_NOVETO_Y_BARRERA_FUERA` solo se prueban
  hoy por unidad de bits en `arnes_respaldo` y en JS (`dom_interruptores.js`);
  `orquestador_interruptores.inc` solo ejerce `ARRANQUE_BARRERA_FUERA` (`S3`).
- **Pendiente de hardware/banco real (`SPEC_8` §7 filas 1, 2, 4, 5)**: persistencia de `DR4` con pila
  CR2032 tras corte fisico >= 30 s, rechazo real `RESPALDO_INVALIDO` sin pila y flujo BLE completo en
  tarjeta con ESP32.

## 4. Tabla del Runbook (sobre `auditoria/n182`, tras `1640ed0`)

- `python 01_Firmware/compuerta.py` (**Pasada 1**): `15 PASS | 1 FALLA | 1 ABORTADO`
  (`banco`: `1519/1520`, solo `D-22`; `unitarios`: `70/70` y `puente ESP32`: `164/164` con `NODE_PATH`;
  `app DOM`: `ABORT` sin `node_modules` en worktree, `CLAUDE.md` §17).
- `python 01_Firmware/compuerta.py` (**Pasada 2**): `15 PASS | 1 FALLA | 1 ABORTADO`
  (converge a `15 PASS | 1 FALLA | 1 ABORTADO`; `banco`: `1518/1520` porque el acta de P1 en worktree
  marco `ABORT` en `app DOM`; `evidencia/2026-11-10_compuerta.txt` restaurada a `edb2462` antes del commit).
- `test_dom_execution.js` (`NODE_PATH` a `main`): **`949 PASS | 0 FALLAS`** (solo lectura, `I6`/`I9` ok).
- `simulador_puente_esp32.py` (`NODE_PATH` a `main`): **`164/164 PASS`** (incluye `P6` Maestro y Esclavo).
- `arquitecto-iot` (contexto limpio): **`APTO CON CONDICIONES`** (`C1` parcial cerrada en `208ed6c`, `C2`
  y erratas de `SPEC_8` cerradas en `63a45fb` + `1640ed0`, `C3` en tarjeta fisica).
- `qa-istqb` (contexto limpio): **`APTO CON CONDICIONES`** (`D4` cerrada en `1640ed0`; quedan `D1` en
  `dom_simulador.js`, `D2`/`D3`/`D5` y pruebas fisicas con CR2032).
