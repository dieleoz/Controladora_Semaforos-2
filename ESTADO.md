# ESTADO — donde esta parado el trabajo y por donde se retoma

Hace de RETOMAR: se reescribe entero en cada cierre. Lo anterior, en [`HISTORIA.md`](HISTORIA.md); el porque de cada
fila, en [`roadmap.md`](roadmap.md). La spec manda (`CLAUDE.md` §15). HEAD: `git rev-parse --short HEAD`.

## Por donde se retoma, en este orden

1. **Mandar a Marco `Paquete_Semaforos_2026-10-14_96c3d32_SIN_BANCO.zip`** (`entregas/`, sha256 `b4390a0d...`):
   fuente PlatformIO de los cuatro firmwares, APK `ba1d595c...`, app web y la hoja unica del 14/10 (24 casos: lo
   nuevo de `D-59`/`D-55` y lo no reportado desde el 04/10). Pedido de Marco del 13/10: verde del Degradado con
   testigo de 3 a 15 min (`D-59`); firmware nuevo en los DOS postes y la APK en TODOS los telefonos del cruce.
   Arquitecto APTO, QA APTO CON CONDICIONES (cerradas), acta `2026-10-14` convergida (solo `D-22`). APK probada en el
   telefono del responsable (`evidencia/2026-10-14_telefono_d55.txt`). Texto para Marco: el de la sesion del 14/10.
2. **Cuando vuelvan sus cintas** (`N-177` V10): leerlas enteras y contando (`CLAUDE.md` §7), un `IOTVIAL_*` de CADA
   poste; si la hoja pasa, se quita `SIN_BANCO`. Si el responsable tiene el PDF de la hoja del 04/10, los casos 16-24
   que ya se hicieron salen de la hoja.
3. **A campo (El Sisga) va el firmware que salga de este banco** (responsable, 06/10), solo tras el banco de Marco
   (`CLAUDE.md` §0.2). Pendiente: C3 `Validacion_LCD`; C4 el grafo; `N-175`. El Degradado automatico se queda.
4. **`N-178` comiteado, sin banco**: M1-M13 y ronda 2, `D-60`..`D-102`
   (lo abierto lo decidio Fable). Acta `2026-10-15` convergida, 1 FALLA: `D-22` y `D-94`..`D-97` sin construir (M12,
   solo SPEC): piden placa, como `D-22`. Arquitecto y QA: condiciones cerradas. Parches en el scratchpad
   `n178`/`n178r2`. La hoja del proximo paquete dice «APK primero, luego firmware» (`D-102`). **Sin explicar:** los
   contadores de radio del 06/10 de 1789 a 91 (`evidencia/061020261110`).

## Frentes abiertos (tres como maximo)

- **Banco de Marco** con el paquete `96c3d32` (`N-177`): verde del Degradado y lo pendiente desde el 04/10.
- **Campo** El Sisga: firmware por decidir; `N-172` sin medir en tarjeta.
- **Orden del repo** (fase F). Fases 0-4 hechas con el arquitecto de plataforma; la 5 (`lib/`), tras banco.

## Que firmware hay en cada equipo (`CLAUDE.md` §0.2)

| equipo | firmware | como se sabe |
|---|---|---|
| instalacion certificada | V8.4, `e303485` | la ultima que paso banco |
| Maestro `179DB0` (El Sisga) | `7ff7d12` `SIN_BANCO`, probado despues `b354fe9`. Ninguno lleva el veto de camara a la pluma (`363e375`, `D-33`): `Camaras_Sisga_4x.html` se corrige en la MISMA visita en que se cargue un firmware posterior | cinta y diario en `evidencia/` |
| Esclavo del Sisga | sin medir | su cinta no se ha traido |
| Maestro `4D2007` y Esclavo `38EB53` (Marco, banco) | `6bd1e4f` en controlador y puente, medido: `$ACK,CMD:VERSION` el 01/10 | `evidencia/011020260934/` |
| paquete para Marco | `Paquete_Semaforos_2026-10-14_96c3d32_SIN_BANCO` (zip sha256 `b4390a0d...`): firmware `D-59` (verde del Degradado 3-15 min) para los DOS postes, `.bin` sueltos al lado; APK `ba1d595c...` con `D-55`, `D-56` y `D-59`; hoja unica del 14/10, 24 casos (lo nuevo y lo no reportado desde el 04/10). Arquitecto APTO, QA APTO CON CONDICIONES (cerradas). Los anteriores, a `entregas/RETIRADOS/` | `entregas/` |

## Verificacion en escritorio

Cifras copiadas del acta [`evidencia/2026-10-15_compuerta.txt`](evidencia/2026-10-15_compuerta.txt), que dice en su
cabecera con que HEAD y que arbol se midio; las comprueba `documentos_01` en cada corrida.

| | |
|---|---|
| Flash | Maestro **72.2 %** · Esclavo **64.0 %** · Repetidor **20.6 %** · ESP32 **35.8 %** |
| Banco por packs | **1453/1458 comprobaciones** en **70 packs**, 1 FALLA: `D-22` pide una tarjeta delante; `D-94`..`D-97` sin construir (`CLAUDE.md` §1) |
| Arneses de C++ real | 88/88 automatico · 78/78 ciclo · 127/127 dos puntas · 158/158 Degradado a dos puntas |
| App y puente | **721/721** jsdom · **70/70** funcional · **67/67** unitarios · 90/90 TDD · puente 156/156 |

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
