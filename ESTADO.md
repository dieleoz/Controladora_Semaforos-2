# ESTADO — donde esta parado el trabajo y por donde se retoma

Hace de RETOMAR: se reescribe entero en cada cierre. Lo anterior, en [`HISTORIA.md`](HISTORIA.md); el porque de cada
fila, en [`roadmap.md`](roadmap.md). La spec manda (`CLAUDE.md` §15). HEAD: `git rev-parse --short HEAD`.

## Por donde se retoma, en este orden

1. **Cerrar el paquete de `N-180`** (grupos via -> grupo -> pareja por MAC y simulador con estado, `D-117`..`D-126`;
   app en `ac844b9`, sin firmware: `D-118`). Falta, en orden: converger la compuerta (acta `2026-10-16`, se espera
   solo `D-22` y `D-94`..`D-97`); compilar la APK (`.claude/particularidades/entregar.md` §3) y poner su huella en
   `Pruebas_Funcional_2026-10-16.html`, que ya trae los casos 42-45 SIN comitear (lleva la huella vieja
   `a8214ed5...`); `.bin` con sello por `strings`; recorrer el menu en practica en el telefono del responsable a
   360 px (`D-125`); empaquetar con `herramientas/entrega/empaquetar_funcional.py`. Abierto, no bloquea: version
   siguiente de N-180 en `roadmap.md` (mutantes S3 y S4 de QA, `SET_TIEMPOS:259` aceptado por el `(uint8_t)`).
2. **Mandar a Marco** el paquete de `N-180`; sustituye a `ad74390` (16/10, `c68f11f0...`) si ese no salio. APK
   primero, despues firmware en los DOS postes y los DOS puentes (`D-102`).
3. **Radio con varios grupos (`D-124`, responsable):** un canal por pareja que no usa ningun otro grupo, prueba de
   puesta en marcha apagando cada punta (SPEC_6 B.3). El filtro de pareja en la trama queda como mejora: riesgo
   abierto. Falta el acta de banco con dos parejas (mismo canal, contiguos, separados) y cuantos canales caben en la
   banda de la antena (170/172 MHz).
4. **Cuando vuelvan sus cintas** (`N-177` V10): leerlas enteras y contando (`CLAUDE.md` §7), un `IOTVIAL_*` de CADA
   poste; si la hoja pasa, se quita `SIN_BANCO`. **A campo (El Sisga)** va lo que salga de ese banco; pendiente C3
   `Validacion_LCD`, C4 el grafo, `N-175`. **Sin explicar:** contadores de radio del 06/10 (`evidencia/061020261110`).

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
| paquete para Marco | `Paquete_Semaforos_2026-10-16_ad74390_SIN_BANCO` (zip `c68f11f0...`): `N-179` y `N-178` para los DOS postes y los DOS puentes, `.bin` con sello `FW:ad74390`; APK `a8214ed5...`; hoja del 16/10, 41 casos | `entregas/` |

## Verificacion en escritorio

Cifras copiadas del acta [`evidencia/2026-10-16_compuerta.txt`](evidencia/2026-10-16_compuerta.txt), que dice en su
cabecera con que HEAD y que arbol se midio; las comprueba `documentos_01` en cada corrida.

| | |
|---|---|
| Flash | Maestro **78.8 %** · Esclavo **65.3 %** · Repetidor **20.6 %** · ESP32 **35.8 %** |
| Banco por packs | **1492/1497 comprobaciones** en **70 packs**, 1 FALLA: `D-22` pide una tarjeta delante; `D-94`..`D-97` sin construir (`CLAUDE.md` §1) |
| Arneses de C++ real | 88/88 automatico · 78/78 ciclo · 175/175 dos puntas · 158/158 Degradado a dos puntas |
| App y puente | **802/802** jsdom · **70/70** funcional · **67/67** unitarios · 87/87 TDD · puente 159/159 |

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
