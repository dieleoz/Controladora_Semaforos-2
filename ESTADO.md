# ESTADO — donde esta parado el trabajo y por donde se retoma

Hace de RETOMAR: se reescribe entero en cada cierre. Lo anterior, en [`HISTORIA.md`](HISTORIA.md); el porque de cada
fila, en [`roadmap.md`](roadmap.md). La spec manda (`CLAUDE.md` §15). HEAD: `git rev-parse --short HEAD`.

## Por donde se retoma, en este orden

1. **Leer los logs de Marco con `Paquete_Semaforos_2026-10-06_d810f81_SIN_BANCO`** (`entregas/`, sha256
   `014ea336...`): llegan a `evidencia/<ddmmaaaahhmm>/` (cinta, diario de ordenes `IOTVIAL_*`, enlace, log) y el
   PDF de `Pruebas_Funcional_2026-10-05.html` (8 casos). Se leen enteros y contando (`CLAUDE.md` §7): primero que
   los cuatro equipos digan `d810f81`, despues caso por caso contra SPEC_1 §3.3 (`D-53`) y SPEC_4 §3.ter.bis.
2. **A campo (El Sisga) va el firmware que salga de este banco** (responsable, 06/10): `d810f81` o su
   correccion, y solo despues de pasar el banco de Marco (`CLAUDE.md` §0.2). Pendiente: C3 `Validacion_LCD`;
   C4 el grafo; `N-175` bajo trafico por dias de la semana, en roadmap. El Degradado automatico se queda.
3. **Version siguiente de la app:** el rotulo del enlace en la cabecera pisa los iconos (roadmap, `N-176`).
   Marco (06/10, `evidencia/061020261035` y `061020261110`): «la (b) no entra a los 5 min». El banco de `d810f81` no
   da defecto de firmware: activo la (b) con el Maestro en MENU, donde no hay acuse (SPEC_2 §7.ter), y OTRO es el
   ultimo APTO oido, no la opcion. App corregida (`394678a`, `b4894d8`, `6fc5ffa`); falta APK y repetir la prueba.
4. **Fase F** (partir `17_...`, `ARQUITECTURA.map`). Las ramas `revision/*` del 04/10 se leyeron el 06/10 contra
   `aec07e7`: casi todo cerrado o falso; lo vigente son huecos ya declarados en la spec.

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
| Maestro `179DB0` (El Sisga) | `7ff7d12` `SIN_BANCO`, probado despues `b354fe9`. Ninguno lleva el veto de camara a la pluma (`363e375`, `D-33`): `Camaras_Sisga_4x.html` se corrige en la MISMA visita en que se cargue un firmware posterior | cinta y diario en `evidencia/` |
| Esclavo del Sisga | sin medir | su cinta no se ha traido |
| Maestro `4D2007` y Esclavo `38EB53` (Marco, banco) | `6bd1e4f` en controlador y puente, medido: `$ACK,CMD:VERSION` el 01/10 | `evidencia/011020260934/` |
| paquete para Marco | `Paquete_Semaforos_2026-10-06_6078583_SIN_BANCO` (zip sha256 `d5cc7fa9...`): solo cambia la APK (textos del Degradado automatico); fuente del firmware igual a `d810f81`, no hay que recargar. Los anteriores, a `entregas/RETIRADOS/` | `entregas/` |

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
