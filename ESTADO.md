# ESTADO — donde esta parado el trabajo y por donde se retoma

Hace de RETOMAR: se reescribe entero en cada cierre. Lo anterior, en [`HISTORIA.md`](HISTORIA.md); el porque de cada
fila, en [`roadmap.md`](roadmap.md). La spec manda (`CLAUDE.md` §15). HEAD: `git rev-parse --short HEAD`.

## Por donde se retoma, en este orden

**Orden del responsable (09/11, 19:00): «construir todo, que no falte nada», hasta el `.zip` final; con subagentes y
workflows en paralelo (ficheros disjuntos). Decisiones ya tomadas: no se vuelven a preguntar.**

1. **ENTREGABLE, con `N-187`:** `entregas/Paquete_Semaforos_2026-11-10_c10e60f_SIN_BANCO.zip` (zip `a9e98dd9...`,
   APK `921e0ab4...`); arquitecto y QA APTO CON CONDICIONES, compuerta 16/17 (solo `D-22`), hoja del 10/11 con la
   Parte 9 (pruebas 26-34). Radio resuelta (`D-160`: RS485 a 9600 transparente, M0/M1 en ON; 100 % de latidos).
2. **`N-187` CONSTRUIDO sin banco (09/11); arquitecto y QA APTO CON CONDICIONES (cerradas: D1 de QA).** Parametros
   de camaras y barrera desde la app, en Ajustes con clave de tecnico y PIN: SPEC_8 §8 y `D-161`..`D-165`.
   Decidido con la simulacion de dos modelos + arbitro (scratchpad `pluma/`, roadmap `N-187`):
   - parametros: contacto **NA/NC** por camara (C y D), de fabrica NA; **retardo de bajada tras el rojo 3..5 s**, de
     fabrica 3 (recomendado: con 2 s el brazo cae ~7 veces mas sobre quien cruza al final del amarillo). Si el
     responsable dice 2..5, se cambia solo el rango y su prueba;
   - fijos (no parametro): sube con el verde; nunca baja antes del rojo; arriba en ambar intermitente (cliente/PMT
     27/08); J15 activa = subir y de NIVEL (sin pulso: J15 no tiene «parado» ni final de carrera leido);
   - flash propia `0x0800F400` (molde `testigo_flash`), ordenes `SET_CAMARA`, `SET_PLUMA_RETARDO`,
     `CONSULTA_INSTALACION` con molde `SET_TIEMPOS`; `$STATUS` no crece; simulador de Practica las contesta
     (memoria: una orden nueva va al simulador); app en Ajustes > Averias / Instalacion.
   - Workflow: arquitecto revisa SPEC_8 §8 ANTES (§11.7) -> firmware (worktree) + app y simulador (worktree) en
     paralelo, prueba vista en rojo, arneses existentes (simulador congelado) -> integrar por el diff -> spec en el
     MISMO commit (regla del 09/11).
3. **Runbook completo:** compuerta en el arbol principal hasta dos pasadas iguales (17 filas; cifras a README y
   ESTADO), arquitecto y QA una vez en contexto limpio, APK (JDK 21: `java-21-openjdk...` en Baliza), `npx cap copy
   android` antes, telefono si esta conectado (adb + CDP), y UN `.zip` con la hoja del 10/11 + seccion de los
   parametros nuevos (como se ponen y que se espera). El anterior a `RETIRADOS/`.
4. **Cuando vuelvan los registros de Marco (10/11):** leerlos enteros y contando (`CLAUDE.md` §7): `RTT:` con LoRa,
   cobertura en metros, tiempo real de subida y bajada del brazo (el que decide: con >10 s ningun retardo protege),
   camaras y C5. Con eso se ajustan rangos de `N-187` desde la app, sin firmware.
5. **Frentes:** banco de Marco (10/11), `N-187`, El Sisga tras banco. Version siguiente: `roadmap.md` orden, fila 5.

## Que firmware hay en cada equipo (`CLAUDE.md` §0.2)

| equipo | firmware | como se sabe |
|---|---|---|
| instalacion certificada | V8.4, `e303485` | la ultima que paso banco |
| Maestro `179DB0` (El Sisga) | `7ff7d12` `SIN_BANCO`, probado despues `b354fe9`. Ninguno lleva el veto de camara a la pluma (`363e375`, `D-33`): `Camaras_Sisga_4x.html` se corrige en la MISMA visita en que se cargue un firmware posterior | cinta y diario en `evidencia/` |
| Esclavo del Sisga | sin medir | su cinta no se ha traido |
| Maestro `4D2007` y Esclavo `38EB53` (Marco, banco) | `6bd1e4f` en controlador y puente, medido: `$ACK,CMD:VERSION` el 01/10 | `evidencia/011020260934/` |
| paquete para Marco | `Paquete_Semaforos_2026-11-10_c10e60f_SIN_BANCO` (zip `a9e98dd9...`, APK `921e0ab4...`), sin enviar; en su banco, `8f8dd7e` (mismo codigo de firmware), medido el 09/11 15:42 | `entregas/`, `evidencia/091120261545` |

## Verificacion en escritorio

Cifras copiadas del acta [`evidencia/2026-11-09_compuerta.txt`](evidencia/2026-11-09_compuerta.txt), que dice en su
cabecera con que HEAD y que arbol se midio; las comprueba `documentos_01` en cada corrida.

| | |
|---|---|
| Flash | Maestro **83.9 %** · Esclavo **70.4 %** · ESP32 **35.8 %** |
| Banco por packs | **1520/1521 comprobaciones** en **70 packs**, 1 FALLA: `D-22` pide una tarjeta delante (`CLAUDE.md` §1) |
| Arneses de C++ real | 109/109 automatico · 78/78 ciclo · 184/184 dos puntas · 161/161 Degradado a dos puntas |
| App y puente | **949/949** jsdom · **71/71** funcional · **70/70** unitarios · 91/91 TDD · puente 164/164 |
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
