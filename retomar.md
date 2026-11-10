# Retomar auditoria N-182 (`auditoria/n182`)

Worktree: `D:\@Proyect\Controladora_Semaforos 2\.claude\worktrees\auditoria-n182`
Rama local: `auditoria/n182` (base `7f4a502` + 7 commits `0a81b54..59c2271`).

## Estado frente a `main` (`edb2462` en `origin/main`)
Los 7 commits producidos en `auditoria/n182` ya fueron integrados en `main` junto con la
condicion `d77695a`, y `main` siguio avanzando hasta `edb2462` (`[origin/main]`):

1. `0a81b54` -> en `main` `9b52f11` (`chore(git): ignorar .claude/rondas.json`)
2. `b8b7cc7` -> en `main` `3c29e44` (`barrera_03_talanquera.py` lista cerrada en `luzPideArriba`)
3. `3ed24c7` -> en `main` `47b4060` (`Validacion_Respaldo/compilar.ps1` ASCII + exit 2)
4. `e5089b9` -> en `main` `a6996bc` (`escenario_p_salida.py` P6 Maestro/Esclavo + `arnes_app.js`)
   - En `main` se anadio `d77695a` (devolver el `NO CUBRE` a cabecera de `escenario_p_salida.py`).
5. `799d877` -> en `main` `ab76cca` (`dom_textos_spec4.js` esperar 0 ms en D-78)
6. `609d4e4` -> en `main` `223e6d9` (`app_01_comandos.py` leer `interruptores.cpp` y `data-interruptor`)
7. `59c2271` -> en `main` `e990789` (`dom_simulador.js` leer `interruptores.cpp`)

Ademas en `main` ya estaban:
- `4c709fd` (puntos 1 y 2: `NO_QUEDO_PUESTO` en `js/interruptores.js` y `tests/dom_interruptores.js`)
- `666d904` y `6242f8f` (hallazgo del telefono: `SET_VETO_CAMARA` / `SET_BARRERA_SERVICIO` en
  `js/simulador.js`, `www/js/simulador.js` y `tests/dom_simulador.js`)
- `8867292`, `3ea00d4`, `6ca489f`, `52b4acc`, `edb2462` (N-187, refactor de ficheros < 500
  lineas, paquete `6ca489f` del 10/11, D-168/D-169).

## Pendiente en la rama si se continua la auditoria N-182
1. Rebasar `auditoria/n182` sobre `main` (`edb2462`) si se quiere redactar el cierre alli.
2. Las revisiones de `arquitecto-iot` y `qa-istqb` fallaron al lanzarse como subagentes por
   cuota (`429 RESOURCE_EXHAUSTED`); quedan pendientes sobre `main` / rama rebasada.
3. Redactar `05_Funcional/historico/auditoria_n182.md` si aun se requiere el acta formal
   con:
   - preguntas abiertas para el responsable (punto 8: arranque S3 del Esclavo, C5 ventana de
     bajada ~60 s, camara unica / barrera ABAJO — notar que D-168/D-169 ya entraron en `main`);
   - analisis de El Sisga 48 h (punto 9: `LIMITE_DURO_MS` y trama/alarma/hora).
