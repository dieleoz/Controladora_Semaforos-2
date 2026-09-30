# ESTADO — donde esta parado el trabajo y por donde se retoma

Hace de RETOMAR: se reescribe entero en cada cierre. Lo anterior, en [`HISTORIA.md`](HISTORIA.md); el porque de cada
fila, en [`roadmap.md`](roadmap.md). La spec manda (`CLAUDE.md` §15). HEAD: `git rev-parse --short HEAD`.

## Por donde se retoma, en este orden

1. **Marco prueba en banco `a505fa2`** (Degradado automatico, `N-167`) con `Pruebas_Funcional_2026-09-29.html`, casos
   0-8, y exporta los dos postes por caso. Se leen ENTEROS y contando (`CLAUDE.md` §7) antes de tocar nada.
2. **`N-168` (`D-38`, decidido 30/09):** en Degradado, la punta que pierde la hora pasa a ROJO fijo. Sin construir;
   antes, cargar la norma de senalizacion con la skill `norma`.
3. **Fase 5, lo duplicado Maestro/Esclavo a `lib/`**: solo despues de que la candidata pase banco.

## Frentes abiertos (tres como maximo)

- **Degradado automatico (`A-15`, `SPEC_2` §7.ter)**: `a505fa2` con arquitecto y QA APTO con condiciones atendidas,
  compuerta 2x solo `D-22`. Riesgos residuales (f).1 (Manual durante el corte: 528 s de verde contra ambar en el arnes,
  lo cubren paleteros y el aviso) y (f).6 sin decidir. Nada medido en tarjeta.
- **Degradado con testigo (`D-35`)**: probado en banco el 29/09 (rojo hasta inicio, verdes alternos); ya no vence.
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
| Maestro `4D2007` y Esclavo `38EB53` (Marco, banco) | `8974932` en controlador y puente, medido: `$ACK,CMD:VERSION` el 29/09 | `evidencia/2026-09-29_campo_testigo/` |
| paquete para Marco | `a505fa2` `SIN_BANCO` (Paquete_Funcional): Degradado automatico con runbook N-167 completo (arquitecto y QA con condiciones atendidas, compuerta 2x solo D-22); el enviado antes, `a0d605b` sin pruebas | falta banco de dos tarjetas con la guia y los exportes |

## Verificacion en escritorio

Cifras copiadas del acta [`evidencia/2026-09-30_compuerta.txt`](evidencia/2026-09-30_compuerta.txt), que dice en su
cabecera con que HEAD y que arbol se midio; las comprueba `documentos_01` en cada corrida.

| | |
|---|---|
| Flash | Maestro **74.4 %** · Esclavo **66.5 %** · Repetidor **20.6 %** · ESP32 **35.7 %** |
| Banco por packs | **1337/1338 comprobaciones** en **70 packs**; el unico FALLA, `D-22`, es correcto: pide una tarjeta delante (`CLAUDE.md` §1) |
| Arneses de C++ real | 75/75 automatico · 22/22 ciclo · 122/122 dos puntas · 117/117 Degradado a dos puntas |
| App y puente | **401/401** jsdom · **70/70** funcional · **63/63** unitarios · 85/85 TDD · puente 126/126 |

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
