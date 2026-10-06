# ESTADO — donde esta parado el trabajo y por donde se retoma

Hace de RETOMAR: se reescribe entero en cada cierre. Lo anterior, en [`HISTORIA.md`](HISTORIA.md); el porque de cada
fila, en [`roadmap.md`](roadmap.md). La spec manda (`CLAUDE.md` §15). HEAD: `git rev-parse --short HEAD`.

## Por donde se retoma, en este orden

1. **Leer los logs de Marco con `Paquete_Semaforos_2026-10-06_d810f81_SIN_BANCO`** (`entregas/`, sha256
   `014ea336...`): llegan a `evidencia/<ddmmaaaahhmm>/` (cinta, diario de ordenes `IOTVIAL_*`, enlace, log) y el
   PDF de `Pruebas_Funcional_2026-10-05.html` (8 casos). Se leen enteros y contando (`CLAUDE.md` §7): primero que
   los cuatro equipos digan `d810f81`, despues caso por caso contra SPEC_1 §3.3 (`D-53`) y SPEC_4 §3.ter.bis.
2. **Pendiente del responsable (06/10):** si a las cuatro tareas del orquestador (empaquetador del funcional a
   `herramientas/entrega/` retirando `generar_entrega_v9_0.py`; regla del telefono «solo `am start -n`, nunca
   `monkey`» en `particularidades/`; `compuerta.py --help` sin correr nada; `PYTHONIOENCODING`/`PYTHONUTF8` en
   `.claude/settings.json`). Que firmware va a campo en El Sisga (`d810f81` no tiene banco). Retirar o no el
   Degradado automatico. `N-175` modo nocturno; C3 `Validacion_LCD`; C4 el grafo.
3. **Version siguiente de la app:** el rotulo del enlace en la cabecera pisa los iconos (roadmap, `N-176`).
4. **Ramas de revision por leer** del 04/10 (`revision/*`) y **fase F** (partir `17_...`, `ARQUITECTURA.map`).

## Frentes abiertos (tres como maximo)

- **Banco de Marco** con el paquete del 06/10 (`d810f81`).
- **Campo** El Sisga: firmware por decidir; `N-172` sin medir en tarjeta.
- **Orden del repo** (fase F).

## Organizacion del repo por fases (con el arquitecto de plataforma)

Fases 0-4 hechas (paquetes a `entregas/`, inventario, `CLAUDE.md` a 200, plugin y pre-commit, poda B con
trinquete 1,665); la 5, lo duplicado a `lib/`, despues de banco.

## Que firmware hay en cada equipo (`CLAUDE.md` §0.2)

| equipo | firmware | como se sabe |
|---|---|---|
| instalacion certificada | V8.4, `e303485` | la ultima que paso banco |
| Maestro `179DB0` (El Sisga) | `7ff7d12` `SIN_BANCO`, probado despues `b354fe9` | cinta y diario en `evidencia/` |
| Esclavo del Sisga | sin medir | su cinta no se ha traido |
| Maestro `4D2007` y Esclavo `38EB53` (Marco, banco) | `6bd1e4f` en controlador y puente, medido: `$ACK,CMD:VERSION` el 01/10 | `evidencia/011020260934/` |
| paquete para Marco | `Paquete_Semaforos_2026-10-06_d810f81_SIN_BANCO` (D-53 y eleccion del Degradado automatico; firmware igual a `5f1d37f`, el del acta). Los anteriores, a `entregas/RETIRADOS/` | `entregas/` |

## Verificacion en escritorio

Cifras copiadas del acta [`evidencia/2026-10-05_compuerta.txt`](evidencia/2026-10-05_compuerta.txt), que dice en su
cabecera con que HEAD y que arbol se midio; las comprueba `documentos_01` en cada corrida.

| | |
|---|---|
| Flash | Maestro **70.4 %** · Esclavo **62.6 %** · Repetidor **20.6 %** · ESP32 **35.7 %** |
| Banco por packs | **1372/1373 comprobaciones** en **70 packs**; el unico FALLA, `D-22`, es correcto: pide una tarjeta delante (`CLAUDE.md` §1) |
| Arneses de C++ real | 88/88 automatico · 55/55 ciclo · 127/127 dos puntas · 143/143 Degradado a dos puntas |
| App y puente | **540/540** jsdom · **70/70** funcional · **63/63** unitarios · 85/85 TDD · puente 150/150 |

## BLOQUEANTES: lo que no se cierra con teclado (roadmap §1 y §4)

| bloqueo | referencia |
|---|---|
| `BAT:--` sin divisor ni entrada analogica | `N-108` |
| `J16` p1 con 12 V crudos: se tapa en cada equipo | `N-120` |
| entradas de campo sin proteger; `Y2` del Maestro `179DB0` no oscila | `C-6` |
| la Maestro de sesion 1 muere a los ~30 s; reinicios del ESP32 del Sisga | `N-116` |
| nada posterior a `7ff7d12` ha visto cobre con cinta | `CLAUDE.md` §0.2 |

---

## Dependencias de esta maquina

- `D:\toolchain\mingw64`: el `gcc` de host fuera de la ruta con `ñ` (`N-44`); sin el, los arneses dan `ABORTADO`.
- Sin `java` en el `PATH`: `JAVA_HOME` a un JDK de `D:\@Proyect\Baliza\7 sw apk\`; `sdk.dir = C:/android-sdk`.
