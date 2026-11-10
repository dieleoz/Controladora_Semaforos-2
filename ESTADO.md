# ESTADO — donde esta parado el trabajo y por donde se retoma

Hace de RETOMAR: se reescribe entero en cada cierre. Lo anterior, en [`HISTORIA.md`](HISTORIA.md); el porque de cada
fila, en [`roadmap.md`](roadmap.md). La spec manda (`CLAUDE.md` §15). HEAD: `git rev-parse --short HEAD`.

## Por donde se retoma, en este orden

1. **Paquete `5b6e8f9` en el banco de Marco** (hoja `05_Funcional/Novedades_y_Pruebas_2026-11-11.html`, 12 pruebas): se
   esperan sus registros, que se leen enteros (`CLAUDE.md` §7), y la cinta de Averias (prueba 30) que se pidio.
2. **Al cargar `J15:BAJAR`:** central a «sin senal = subir» en la MISMA visita, `J15` suelta entre medias; se anota
   aqui la polaridad por poste. Todos los equipos de la tabla llevan firmware anterior: `J15` conduce = SUBIR.
3. **Candidata 3 sobre `5b6e8f9`** (`roadmap.md` fila 5d): arquitecto y QA una vez; compuerta hasta dos pasadas iguales;
   `8a81f1e` y `416766a` a `RETIRADOS/` solo con permiso del responsable.
4. **`D-175`, casos del responsable sin construir:** F2 (Esclavo reiniciado, pluma arriba hasta 28 s) y la pluma del
   Maestro en la ventana de `D-29` (`roadmap.md` fila 5g).
5. **Frentes:** El Sisga tras banco, `D-158` tras medir telefonos. Version siguiente: `roadmap.md`.

## Que firmware hay en cada equipo (`CLAUDE.md` §0.2)

| equipo | firmware | como se sabe |
|---|---|---|
| instalacion certificada | V8.4, `e303485` | la ultima que paso banco |
| Maestro `179DB0` (El Sisga) | `7ff7d12` `SIN_BANCO`, probado despues `b354fe9`. Ninguno lleva el veto de camara a la pluma (`363e375`, `D-33`): `Camaras_Sisga_4x.html` se corrige en la MISMA visita en que se cargue un firmware posterior | cinta y diario en `evidencia/` |
| Esclavo del Sisga | sin medir | su cinta no se ha traido |
| Maestro `4D2007` y Esclavo `38EB53` (Marco, banco) | `6bd1e4f` en controlador y puente, medido: `$ACK,CMD:VERSION` el 01/10 | `evidencia/011020260934/` |
| paquete para Marco | `Paquete_Semaforos_2026-11-11_5b6e8f9_SIN_BANCO` (zip `386ab58b...`, APK `802f419a...`): `D-170` a `D-176`; en su banco, `8f8dd7e`, medido el 09/11 15:42 | `entregas/`, `evidencia/091120261545` |

## Verificacion en escritorio

Cifras copiadas del acta [`evidencia/2026-11-10_compuerta.txt`](evidencia/2026-11-10_compuerta.txt), que dice en su
cabecera con que HEAD y que arbol se midio; las comprueba `documentos_01` en cada corrida.

| | |
|---|---|
| Flash | Maestro **85.5 %** · Esclavo **72.3 %** · ESP32 **35.8 %** |
| Banco por packs | **1524/1525 comprobaciones** en **70 packs**, 1 FALLA: `D-22` pide una tarjeta delante (`CLAUDE.md` §1) |
| Arneses de C++ real | 120/120 automatico · 78/78 ciclo · 204/204 dos puntas · 184/184 Degradado a dos puntas |
| App y puente | **971/971** jsdom · **71/71** funcional · **70/70** unitarios · 92/92 TDD · puente 167/167 |
## BLOQUEANTES (lo que no cierra el teclado): `roadmap.md` §1 y §4 (`N-108`, `N-120`, `C-6`, `N-116`, §0.2).

---|---|
| `BAT:--` sin divisor ni entrada analogica | `N-108` |
| `J16` p1 con 12 V crudos: se tapa en cada equipo | `N-120` |
| entradas de campo sin proteger; `Y2` del Maestro `179DB0` no oscila | `C-6` |
| la Maestro de sesion 1 muere a los ~30 s; reinicios del ESP32 del Sisga | `N-116` |
| nada posterior a `7ff7d12` ha visto cobre con cinta | `CLAUDE.md` §0.2 |

## Dependencias de esta maquina

- `D:\toolchain\mingw64`: el `gcc` de host fuera de la ruta con `ñ` (`N-44`); sin el, los arneses dan `ABORTADO`.
- Sin `java` en el `PATH`: `JAVA_HOME` a un JDK de `D:\@Proyect\Baliza\7 sw apk\`; `sdk.dir = C:/android-sdk`.
