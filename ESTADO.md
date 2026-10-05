# ESTADO — donde esta parado el trabajo y por donde se retoma

Hace de RETOMAR: se reescribe entero en cada cierre. Lo anterior, en [`HISTORIA.md`](HISTORIA.md); el porque de cada
fila, en [`roadmap.md`](roadmap.md). La spec manda (`CLAUDE.md` §15). HEAD: `git rev-parse --short HEAD`.

## Por donde se retoma, en este orden

1. **Marco prueba `Paquete_Semaforos_2026-10-05_20739d9_SIN_BANCO`** (`entregas/`, sha256 `563a3eb3...`):
   rojo+amarillo antes del verde (`D-53`) y Degradado automatico como eleccion de montaje (`N-176`). Trae firmware,
   APK, `App_Web/` y `Pruebas_Funcional_2026-10-05.html` (8 casos, solo lo nuevo). Los DOS postes se cargan a la vez.
   Arquitecto: APTO CON CONDICIONES, cerradas; QA en curso. Origen: su banco del 05/10 (`evidencia/051020261239/`).
2. **Lunes 05/10, montaje en campo** con `Montaje_Campo_2026-10-05.html` (sin Degradado desatendido).
   **Falta que el responsable diga que firmware se graba**: el paquete del 04/10 no va a campo sin banco, y
   `Camaras_Sisga_4x.html` (pluma sin veto) solo es verdad con `7ff7d12`/V8.4 (roadmap B7).
3. **Ramas de revision por leer** (dictamenes, sin merge): `revision/spec-manuales-2026-10-04`,
   `revision/opinion-spec-manuales-...`, `revision/spec-vs-firmware-...`, `revision/app-por-spec-...` (2026-10-04);
   lo que aplicaba ya esta en main (roadmap B6-B8).
4. **Fase F** (partir `17_...`, `ARQUITECTURA.map`, planos KiCad, worktrees viejos) y `N-166` (renombrar `A-15`).
5. **Pendiente del responsable:** `N-175` modo nocturno; C3 renombrar `Validacion_LCD`; C4 donde vive el grafo;
   B3 actualizar el plugin del metodo (fuera del repo).

## Frentes abiertos (tres como maximo)

- **Banco de Marco** con el paquete del 05/10 (`20739d9`).
- **Campo**: lunes 05/10; `N-172` sin medir en tarjeta.
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
| paquete para Marco | `Paquete_Semaforos_2026-10-05_20739d9_SIN_BANCO` (D-53 y eleccion del Degradado automatico; firmware igual a `5f1d37f`, el del acta). Los anteriores, a `entregas/RETIRADOS/` | `entregas/` |

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
