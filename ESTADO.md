# ESTADO — donde esta parado el trabajo y por donde se retoma

Hace de RETOMAR: se reescribe entero en cada cierre. Lo anterior, en [`HISTORIA.md`](HISTORIA.md); el porque de cada
fila, en [`roadmap.md`](roadmap.md). La spec manda (`CLAUDE.md` §15). HEAD: `git rev-parse --short HEAD`.

## Por donde se retoma, en este orden

1. **Paquete a Marco** `Paquete_Funcional_2026-09-28_bd2780e_SIN_BANCO.zip` (sha256 `29023007...`): sello, version
   en logs, export adjunto, usabilidad APK. Al dia con HEAD; el responsable lo pasa a `entregas/` y lo envia.
2. **Tramas de Marco** (funcional, Maestro `4D2007`): los 17 s del Esclavo (`grep 17000` cero hoy: 21,5 s para
   verde, 25 s para ambar; pista: un arbol V8.4 del 31/07 borrado subia ese silencio a 17.000 ms — puede ser
   firmware viejo), focos solos, y camara con alguien delante (1.54). Pedir version antes de leer nada.
3. **Fase 5, lo duplicado Maestro/Esclavo a `lib/`**: solo despues de que la candidata pase banco.

## Frentes abiertos (tres como maximo)

- **Camaras: `CAM:?` en toda la cinta, pero Marco midio 3,3 V con deteccion y 0 V sin ella** (roadmap 1.54). Falta la
  cinta con alguien delante en modo Inteligente para separar camara de equipo.
- **APK `SIN_BANCO` probada en telefono sin equipo** (28/09): export como adjunto, cinta sin ENVIADA falso, atras.
  Sin medir con enlace: la linea `Firmware:` real y el `CMD:VERSION` automatico. Pendiente menor en `APP-1`
  (titulo «BOTONERA DE C...» cortado). El doble ACK, sin vigilante ni correlacion: roadmap 1.56.

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
| paquete enviado a Marco | `1e56d83` `SIN_BANCO`, zip sha256 `a45e6eeb` | nadie ha informado de que este cargado |

## Verificacion en escritorio

Cifras copiadas del acta [`evidencia/2026-09-28_compuerta.txt`](evidencia/2026-09-28_compuerta.txt), que dice en su
cabecera con que HEAD y que arbol se midio; las comprueba `documentos_01` en cada corrida.

| | |
|---|---|
| Flash | Maestro **64.7 %** · Esclavo **55.4 %** · Repetidor **20.6 %** · ESP32 **35.7 %** |
| Banco por packs | **1318/1319 comprobaciones** en **70 packs**; el unico FALLA, `D-22`, es correcto: pide una tarjeta delante (`CLAUDE.md` §1) |
| Arneses de C++ real | 75/75 automatico · 22/22 ciclo · 122/122 dos puntas · 71/71 Degradado a dos puntas |
| App y puente | **340/340** jsdom · **70/70** funcional · **63/63** unitarios · 85/85 TDD · puente 123/123 |

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
