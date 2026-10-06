# ESTADO — donde esta parado el trabajo y por donde se retoma

Hace de RETOMAR: se reescribe entero en cada cierre. Lo anterior, en [`HISTORIA.md`](HISTORIA.md); el porque de cada
fila, en [`roadmap.md`](roadmap.md). La spec manda (`CLAUDE.md` §15). HEAD: `git rev-parse --short HEAD`.

## Por donde se retoma, en este orden

1. **Logs de Marco con `Paquete_Semaforos_2026-10-06_16c97e6_SIN_BANCO`** (`entregas/`, zip `ba1e0875...`; solo APK
   nueva: «Que falta» del Degradado automatico; firmware igual a `d810f81`). Esperado: la (b) probada con el POSTE 1
   en AUTO/INTELIGENTE, los dos «Listo: SI», radio cortada 5,5 min, y un `IOTVIAL_*` de CADA poste. Leerlos enteros
   y contando (`CLAUDE.md` §7). Lo del 06/10 (`evidencia/0610*`) ya esta leido: ningun defecto de firmware; la (b)
   se activo en MENU o sin el POSTE 1 ciclando, y la app no decia que hacer (arreglado, `16c97e6`). 15:25: «ya
   entro, pero queda en rojo»: esperado 7-12 min (`DEG_AUTO_ROJO_MIN_S`); si sigue pasada la hora, defecto.
2. **A campo (El Sisga) va el firmware que salga de este banco** (responsable, 06/10), solo tras el banco de Marco
   (`CLAUDE.md` §0.2). Pendiente: C3 `Validacion_LCD`; C4 el grafo; `N-175`. El Degradado automatico se queda.
3. **Pendientes de escribir** (no se pudieron leer `OPTIMIZACIONES.md` ni `roadmap.md` el 06/10: el clasificador
   lo bloqueo): fila de version siguiente con las cuatro mejoras del registro de la app que aprobo el responsable
   (cinta con mas tope o registro aparte de $STATUS de cambio; copia persistente fuera de CACHE para adb; el
   registro de enlace dentro del mismo .txt; en nativo, a fichero desde el principio). Y `N-176` (cabecera).
4. **Sin explicar:** a las 10:43:30 del 06/10 los contadores de radio del Maestro bajan de 1789 a 91 sin reinicio
   que conste (`evidencia/061020261110`, Enlace csv). **Fase F** (partir `17_...`, `ARQUITECTURA.map`). Pregunta
   abierta al responsable: construir los dos interruptores de SPEC_8 §6 (Marco cree que existen; no existen).

## Frentes abiertos (tres como maximo)

- **Banco de Marco** con el paquete `16c97e6`: la (b) bien probada.
- **Campo** El Sisga: firmware por decidir; `N-172` sin medir en tarjeta.
- **Orden del repo** (fase F). Fases 0-4 hechas con el arquitecto de plataforma; la 5 (`lib/`), tras banco.

## Que firmware hay en cada equipo (`CLAUDE.md` §0.2)

| equipo | firmware | como se sabe |
|---|---|---|
| instalacion certificada | V8.4, `e303485` | la ultima que paso banco |
| Maestro `179DB0` (El Sisga) | `7ff7d12` `SIN_BANCO`, probado despues `b354fe9`. Ninguno lleva el veto de camara a la pluma (`363e375`, `D-33`): `Camaras_Sisga_4x.html` se corrige en la MISMA visita en que se cargue un firmware posterior | cinta y diario en `evidencia/` |
| Esclavo del Sisga | sin medir | su cinta no se ha traido |
| Maestro `4D2007` y Esclavo `38EB53` (Marco, banco) | `6bd1e4f` en controlador y puente, medido: `$ACK,CMD:VERSION` el 01/10 | `evidencia/011020260934/` |
| paquete para Marco | `Paquete_Semaforos_2026-10-06_16c97e6_SIN_BANCO` (zip sha256 `ba1e0875...`): solo cambia la APK («Que falta» del Degradado automatico); fuente del firmware igual a `d810f81`, no hay que recargar. Los anteriores, a `entregas/RETIRADOS/` | `entregas/` |

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
