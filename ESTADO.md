# ESTADO — donde esta parado el trabajo y por donde se retoma

Hace de RETOMAR: se reescribe entero en cada cierre. Lo anterior, en [`HISTORIA.md`](HISTORIA.md); el porque de cada
fila, en [`roadmap.md`](roadmap.md). La spec manda (`CLAUDE.md` §15). HEAD: `git rev-parse --short HEAD`.

## Por donde se retoma, en este orden

1. **Retomar la rama `wip/correccion-no-apto`** (`0d35f21`, INTERRUMPIDO): puntos 1-4 del NO APTO hechos segun el
   agente (suelta del Poste 1 a 21,5 s; `FORZAR_ROJO` en Degradado = salida por todo-rojo; pruebas de `D-47`;
   `N-172` de raiz quitando la libreria STM32duino RTC). Falta: re-correr arneses y `--rapido`, aplicar
   `wip/app_js_correccion_no_apto.patch` sin que `app.js` crezca, filas `D-50`/`D-51` (texto en el informe del
   agente), fusionar a `main`, compuerta hasta converger, arquitecto, QA y paquete validado.
2. **Fase F** hecha en su primera parte (`1617042`). Queda: partir
   `17_...`, rehacer `ARQUITECTURA.map`, planos KiCad a `03_Hardware_Tarjeta/`, worktrees viejos.
3. **Marco prueba `Paquete_Funcional_2026-10-02_851805c_SIN_BANCO`** con `Pruebas_Funcional_2026-10-02.html`; sus
   cintas se leen enteras y contando. Lunes 05/10, primera sesion en campo con `Montaje_Campo_2026-10-05.html`.
4. **Pendiente del responsable:** `N-175` modo nocturno (que hace en un paso alterno de un carril); C3 renombrar
   `Validacion_LCD`; C4 donde vive el grafo. Fase 5 (`lib/`), despues de banco.

## Frentes abiertos (tres como maximo)

- **Candidata con I+G** (`851805c`): NO APTO en correccion. Lo nuevo: `D-44..D-49`.
- **Orden del repo** (fase F, `evidencia/2026-10-02_inventario_repo.txt`).
- **Campo**: lunes 05/10; `N-172` sin medir en tarjeta.

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
| paquete para Marco | `Paquete_Funcional_2026-10-02_851805c_SIN_BANCO` (amarillo de la norma, legacy fuera; arquitecto NO APTO, dos defectos avisados en LEEME y hoja). En banco desde el 02/10: `d3606be` | banco de Marco |

## Verificacion en escritorio

Cifras copiadas del acta [`evidencia/2026-10-04_compuerta.txt`](evidencia/2026-10-04_compuerta.txt), que dice en su
cabecera con que HEAD y que arbol se midio; las comprueba `documentos_01` en cada corrida.

| | |
|---|---|
| Flash | Maestro **69.7 %** · Esclavo **61.9 %** · Repetidor **20.6 %** · ESP32 **35.7 %** |
| Banco por packs | **1368/1369 comprobaciones** en **70 packs**; el unico FALLA, `D-22`, es correcto: pide una tarjeta delante (`CLAUDE.md` §1) |
| Arneses de C++ real | 76/76 automatico · 22/22 ciclo · 122/122 dos puntas · 139/139 Degradado a dos puntas |
| App y puente | **457/457** jsdom · **70/70** funcional · **63/63** unitarios · 85/85 TDD · puente 150/150 |

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
