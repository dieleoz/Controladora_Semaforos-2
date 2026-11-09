# ESTADO — donde esta parado el trabajo y por donde se retoma

Hace de RETOMAR: se reescribe entero en cada cierre. Lo anterior, en [`HISTORIA.md`](HISTORIA.md); el porque de cada
fila, en [`roadmap.md`](roadmap.md). La spec manda (`CLAUDE.md` §15). HEAD: `git rev-parse --short HEAD`.

## Por donde se retoma, en este orden

1. **Paquete para Marco (`N-182`, 09/11):** firmware con los interruptores de camara y barrera (`D-94`..`D-97`,
   `D-148`..`D-152`), app simplificada (bloque U, `D-141`..`D-145`), textos llanos de `N-183` y simulador de Practica
   con los interruptores; hoja de 24 pruebas (barrera con puente H, 4 camaras, interruptores, radios, El Sisga y calle
   con mensula). El vigente esta en `entregas/` y en la fila de abajo; los anteriores, en `RETIRADOS/`. Sin banco.
2. **Lo que mide Marco en esa visita:** recorrido real de la pluma (dice ~60 s con final de carrera) y si la camara
   frena una bajada ya ordenada (hoy no, C5: decision del responsable tras medir); banderas tras un corte con pila;
   `D-22` una tarjeta; `D-124` dos parejas. El Sisga: `VERSION` de cada poste y cinta de viernes a lunes (hipotesis
   no medida: tope de 48 h sin sincronizar del firmware anterior al 29/09).
3. **Radio con varios grupos (`D-124`):** un canal por pareja (SPEC_6 B.3); falta el acta con dos parejas (mismo
   canal, contiguos, separados) y cuantos canales caben en 170/172 MHz. Filtro de pareja en la trama: riesgo abierto.
4. **Cuando vuelvan sus cintas** (`N-177` V10): leerlas enteras y contando (`CLAUDE.md` §7), un `IOTVIAL_*` de CADA
   poste; si la hoja pasa, se quita `SIN_BANCO`. **A campo (El Sisga)** va lo que salga de ese banco; pendiente C3
   `Validacion_LCD`, C4 el grafo, `N-175`. **Sin explicar:** contadores de radio del 06/10 (`evidencia/061020261110`).

## Frentes abiertos (tres como maximo)

- **Banco de Marco** con el paquete de `N-182` (punto 1): barrera, camaras e interruptores; radios a 9600 transparente
  con M0/M1 en ON (`D-160`, 100 % de
  latidos el 09/11), falta el alcance en metros.
- **Campo** El Sisga: firmware por decidir; `N-172` sin medir en tarjeta.
- **Orden del repo** (fase F). Fases 0-4 hechas con el arquitecto de plataforma; la 5 (`lib/`), tras banco.

## Que firmware hay en cada equipo (`CLAUDE.md` §0.2)

| equipo | firmware | como se sabe |
|---|---|---|
| instalacion certificada | V8.4, `e303485` | la ultima que paso banco |
| Maestro `179DB0` (El Sisga) | `7ff7d12` `SIN_BANCO`, probado despues `b354fe9`. Ninguno lleva el veto de camara a la pluma (`363e375`, `D-33`): `Camaras_Sisga_4x.html` se corrige en la MISMA visita en que se cargue un firmware posterior | cinta y diario en `evidencia/` |
| Esclavo del Sisga | sin medir | su cinta no se ha traido |
| Maestro `4D2007` y Esclavo `38EB53` (Marco, banco) | `6bd1e4f` en controlador y puente, medido: `$ACK,CMD:VERSION` el 01/10 | `evidencia/011020260934/` |
| paquete para Marco | `Paquete_Semaforos_2026-11-09_277fb72_SIN_BANCO` (zip `90063c81...`, APK `cd1d8834...`, hoja de 24 pruebas con M0/M1 en ON); en su banco hay `8f8dd7e` (mismo codigo de firmware), medido el 09/11 15:42; los anteriores en `RETIRADOS/` | `entregas/`, `evidencia/091120261545` |

## Verificacion en escritorio

Cifras copiadas del acta [`evidencia/2026-11-09_compuerta.txt`](evidencia/2026-11-09_compuerta.txt), que dice en su
cabecera con que HEAD y que arbol se midio; las comprueba `documentos_01` en cada corrida.

| | |
|---|---|
| Flash | Maestro **80.2 %** · Esclavo **66.8 %** · ESP32 **35.8 %** |
| Banco por packs | **1518/1519 comprobaciones** en **70 packs**, 1 FALLA: `D-22` pide una tarjeta delante (`CLAUDE.md` §1) |
| Arneses de C++ real | 99/99 automatico · 78/78 ciclo · 184/184 dos puntas · 161/161 Degradado a dos puntas |
| App y puente | **922/922** jsdom · **71/71** funcional · **70/70** unitarios · 89/89 TDD · puente 159/159 |
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
