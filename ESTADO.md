# ESTADO — donde esta parado el trabajo y por donde se retoma

Hace de RETOMAR: se reescribe entero en cada cierre. Lo anterior, en [`HISTORIA.md`](HISTORIA.md); el porque de cada
fila, en [`roadmap.md`](roadmap.md). La spec manda (`CLAUDE.md` §15). HEAD: `git rev-parse --short HEAD`.

## Por donde se retoma, en este orden

1. **Marco prueba `Paquete_Funcional_2026-10-01_d3606be_SIN_BANCO`** con `Pruebas_Funcional_2026-10-01.html`
   (casos 0-7) y manda PDF, diarios por prueba y log. Se leen ENTEROS y contando (`CLAUDE.md` §7). La prueba 1
   (corte del Maestro en Degradado) dice si la reposicion de DR6/DR7 cierra `N-172`; la 4, si `D-38` da rojo fijo.
2. **Lunes 05/10, primera sesion en campo** (semaforos en todos los modos; talanqueras despues). Antes: respaldar
   con md5 el firmware del Sisga, tapar `J16` p1. Va `SIN_BANCO` (decision del responsable): sin Degradado
   desatendido hasta medir `N-172`.
3. **Deuda del paquete sin runbook:** invertir los arneses que celebran el ambar por hora (`orquestador_degradado`
   F2.2-F2.4/F3.2-F3.3, `reloj_04`, revisar H8 de `orquestador_deg_auto`), compuerta completa 2x, arquitecto y QA
   sobre `d3606be`; SPEC_2 §7.ter (c) y (f).6 al comportamiento nuevo.
4. **Decisiones abiertas del responsable:** un poste en rojo fijo sin hora que sufre un corte arranca en ambar
   (`D-40`) contra el otro alternando; el Degradado `D-18` sin testigo sigue sin reanudar tras >24 h (CNT plegado).
5. **Fase 5, lo duplicado Maestro/Esclavo a `lib/`**: solo despues de que la candidata pase banco.

## Frentes abiertos (tres como maximo)

- **Degradado tras un corte (`N-172`)**: el Maestro no reanudo el 30/09; causa candidata H-D (la libreria RTC pisa
  DR6/DR7) arreglada sin medir en `d3606be`; H-A pila VBAT, H-B DS3231, H-C orden en la ventana siguen abiertas.
- **Paquete `d3606be` sin runbook**: `D-38`, `D-40`, aviso de radio cada 60 s, carteles en la app.
- **Campo del lunes**: camaras con peatones (`D-39`, sensibilidad 50 %), centralita de talanquera (pulso o contacto).

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
| paquete para Marco | `Paquete_Funcional_2026-10-01` `SIN_BANCO`, sin runbook por decision del responsable: `D-40` (Maestro), `D-38` rojo fijo sin hora (`N-168`), aviso de radio cada 60 s, reposicion de DR6/DR7 (`N-172` H-D), app con carteles de corte y radio; hoja `Pruebas_Funcional_2026-10-01.html`. En banco desde el 01/10: `6bd1e4f` | banco de Marco con la hoja y los exportes |

## Verificacion en escritorio

Cifras copiadas del acta [`evidencia/2026-10-02_compuerta.txt`](evidencia/2026-10-02_compuerta.txt), que dice en su
cabecera con que HEAD y que arbol se midio; las comprueba `documentos_01` en cada corrida.

| | |
|---|---|
| Flash | Maestro **73.2 %** · Esclavo **66.1 %** · Repetidor **20.6 %** · ESP32 **35.7 %** |
| Banco por packs | **1353/1354 comprobaciones** en **70 packs**; el unico FALLA, `D-22`, es correcto: pide una tarjeta delante (`CLAUDE.md` §1) |
| Arneses de C++ real | 76/76 automatico · 22/22 ciclo · 122/122 dos puntas · 117/117 Degradado a dos puntas |
| App y puente | **426/426** jsdom · **70/70** funcional · **63/63** unitarios · 85/85 TDD · puente 126/126 |

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
