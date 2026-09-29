# ESTADO — donde esta parado el trabajo y por donde se retoma

Hace de RETOMAR: se reescribe entero en cada cierre. Lo anterior, en [`HISTORIA.md`](HISTORIA.md); el porque de cada
fila, en [`roadmap.md`](roadmap.md). La spec manda (`CLAUDE.md` §15). HEAD: `git rev-parse --short HEAD`.

## Por donde se retoma, en este orden

1. **Marco prueba en banco el paquete de `D-35`** con `Pruebas_Funcional_2026-09-29.html`: version, perdida de
   enlace, testigo sin cable, corte de luz. Informes del 28 y 29/09 en `evidencia/`: camaras OK en las cuatro.
2. **Fase 5, lo duplicado Maestro/Esclavo a `lib/`**: solo despues de que la candidata pase banco.

## Frentes abiertos (tres como maximo)

- **Degradado con testigo (`D-35`)**: construido en firmware y app, `SIN_BANCO`. Sin medir: borrado de flash
  frente al watchdog (`$EVENT` TESTIGO), conexion desde el PC (`Guia_Conectar_PC_Bluetooth.html`).
- **Sin medir en tarjeta**: DR6/DR7 de la pila los escribe tambien la libreria RTC; CNT podria no ser monotono.

## Organizacion del repo por fases (acordada con el arquitecto de plataforma)

| fase | que | estado |
|---|---|---|
| 0 | paquetes a `entregas/` del repo, ignorada por git (viejos en `entregas/RETIRADOS/`); nada fuera | hecha; lo externo, el responsable |
| 1 | inventario de instrumentos por requisito de SPEC | hecha; opcion B |
| 2 | `CLAUDE.md` a 200 lineas sin renumerar; cronica a `HISTORIA.md` | hecha |
| 3 | plugin `orquestador@diego`, pre-commit del metodo, skills a `.claude/particularidades/` | hecha |
| 4 | poda B (70 packs), trinquete 1,665 en el pre-commit, `CLAUDE.md` §16.3 retirada | hecha |
| 5 | lo duplicado Maestro/Esclavo a `lib/` | despues de banco |

## Que firmware hay en cada equipo (`CLAUDE.md` §0.2)

| equipo | firmware | como se sabe |
|---|---|---|
| instalacion certificada | V8.4, `e303485` | la ultima que paso banco |
| Maestro `179DB0` (El Sisga) | `7ff7d12` `SIN_BANCO`, probado despues `b354fe9` | cinta y diario en `evidencia/` |
| Esclavo del Sisga | sin medir | su cinta no se ha traido |
| Maestro `4D2007` (Marco) | `1889631` por declaracion: el puente contesto `FW:--` | su cinta |
| paquete enviado a Marco | `8974932` `SIN_BANCO` (Paquete_Funcional), zip sha256 `6db7024c`, el 29/09 | Marco dice que la web conecta desde el PC; version sin ver |

## Verificacion en escritorio

Cifras copiadas del acta [`evidencia/2026-09-29_compuerta.txt`](evidencia/2026-09-29_compuerta.txt), que dice en su
cabecera con que HEAD y que arbol se midio; las comprueba `documentos_01` en cada corrida.

| | |
|---|---|
| Flash | Maestro **70.9 %** · Esclavo **61.4 %** · Repetidor **20.6 %** · ESP32 **35.7 %** |
| Banco por packs | **1325/1326 comprobaciones** en **70 packs**; el unico FALLA, `D-22`, es correcto: pide una tarjeta delante (`CLAUDE.md` §1) |
| Arneses de C++ real | 75/75 automatico · 22/22 ciclo · 122/122 dos puntas · 71/71 Degradado a dos puntas |
| App y puente | **341/341** jsdom · **70/70** funcional · **63/63** unitarios · 85/85 TDD · puente 123/123 |

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
