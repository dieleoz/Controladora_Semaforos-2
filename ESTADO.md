# ESTADO — donde esta parado el trabajo y por donde se retoma

Hace de RETOMAR: se reescribe entero en cada cierre. Lo anterior, en [`HISTORIA.md`](HISTORIA.md); el porque de cada
fila, en [`roadmap.md`](roadmap.md). La spec manda (`CLAUDE.md` §15). HEAD: `git rev-parse --short HEAD`.

## Por donde se retoma, en este orden

1. **UN SOLO paquete que no deje nada de teclado pendiente (orden del responsable, 16/10).** El de `0fe782f`
   (`entregas/Paquete_Semaforos_2026-10-16_0fe782f_SIN_BANCO.zip`, `b5018718...`, APK `df02795d...`) esta hecho y
   verificado pero RETENIDO: no se manda. Antes, en una tanda (`N-181` de `roadmap.md`: agentes, ficheros y
   tiempos) se cierra TODO lo que solo pide teclado: (a) la version siguiente de `N-180` entera de `roadmap.md` (MAC en
   minusculas, `grupos:[null]`, orden numerico de grupos, «Quitar del grupo» y cambio de hueco sin confirmar, ACK
   diferido entre sesiones, PR sin tope, avisos por MAC al reenlazar, `anotarFw`, `preguntarNuevo`, mutantes de QA
   `SET_TIEMPOS` sin rango y `parar()` vacio, reloj inyectable en `dom_claves.js` «Practica» (4)); (b) la maquetacion
   medida a 320 px CSS en su telefono (franja sobre el modal, «emparejado», «(radio N %)» bajo 🛡️, «MANDOS DE CAMPO»,
   `.card-sub` sin regla, buscador cortado); (c) firmware `SET_TIEMPOS:259` aceptado como 3 por el `(uint8_t)`.
   Despues: compuerta completa, QA una vez, APK, `.bin` con sello, telefono (CDP 9333 + capturas) y UN paquete.
2. **Lo que no cierra el teclado va en la MISMA visita de Marco, pedido en la hoja de ese paquete** (que mida, no
   que pruebe otra version): `D-94`..`D-97` (veto de camara y barrera fuera de servicio) piden tarjeta con barrera y
   pila medidas; `D-22` una tarjeta delante; `D-124` el acta con dos parejas. El responsable dijo SI a
   `D-94`..`D-97` el 27/10 (fila X de `N-181`): se construyen en la tanda; sin pila valida el equipo rechaza la orden.
3. **Radio con varios grupos (`D-124`):** un canal por pareja (SPEC_6 B.3); falta el acta con dos parejas (mismo
   canal, contiguos, separados) y cuantos canales caben en 170/172 MHz. Filtro de pareja en la trama: riesgo abierto.
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
| paquete para Marco | RETENIDO: `Paquete_Semaforos_2026-10-16_0fe782f_SIN_BANCO` (zip `b5018718...`, APK `df02795d...`, `.bin` con `FW:0fe782f`, hoja de 45 casos); el de `ad74390` (`c68f11f0...`) queda sustituido. Sale uno solo, tras el punto 1 | `entregas/` |

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
