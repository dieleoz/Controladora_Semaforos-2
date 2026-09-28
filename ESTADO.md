# ESTADO — dónde está parado el trabajo HOY (28/09/2026)

> ## ▶️ PUNTO DE CONTINUACION — 28/09/2026, Y ES POR AQUI POR DONDE SE RETOMA
>
> 🚦 **LLEGARON LAS TRAMAS DEL FUNCIONAL (Marco, Maestro `4D2007`, firmware `1889631` por declaracion:
> el puente contesto `FW:--`).** Lo medido en la cinta y lo hecho:
>
> - 🔴 **La prueba de focos daba VERDE CONTRA VERDE en servicio** (medido con el C++ real: 2 s con el
>   Esclavo en verde). **Ahora se rechaza en servicio** (`EN_SERVICIO_PASE_A_MENU`), solo corre en
>   MENU/HORA/ALCANCE **con el rojo del Esclavo acusado** (`ESPERANDO_ROJO_DEL_ESCLAVO`) y al acabar
>   devuelve la luz del estado. Decision del responsable: *«es lo esperado, asi no le guste al funcional»*.
> - La app ya no rotula `FALLO COM` el ambar PEDIDO, y la Prueba de Alcance pinta el enlace donde se pulsa.
> - **Camaras: `CAM:?` toda la sesion** — ninguna cerro su contacto; el cableado que paso Marco casa con
>   `SPEC_5` §3. Sin medir si es la camara o el equipo: lo separa el puente p10-p9 del `.html` nuevo.
> - **Hora, degradado:** sin evidencia en la cinta. Van en `05_Funcional/Pruebas_Funcional_2026-09-28.html`,
>   que el funcional rellena y devuelve en PDF.
>
> 🧭 **ORGANIZAR EL REPO POR FASES (acordado con el responsable y el arquitecto de plataforma, 28/09).
> Es ordenar, no romper: cada fase con su commit y la compuerta en verde.**
>
> | fase | que | estado |
> |---|---|---|
> | 0 | simulador congelado (ningun pack nuevo); QA solo sobre la candidata; paquetes FUERA del repo, en `D:\@Proyect\Entregas_Semaforos\` (lo viejo en `RETIRADOS\`) | ✅ paquetes movidos · la cadencia entra en `CLAUDE.md` §4 con la fase 2 |
> | 1 | inventario de instrumentos por requisito de SPEC, y borrador de `particularidades` de las skills | ✅ hecho: 1,76 a 1 medido; opcion B elegida |
> | 2 | `CLAUDE.md` a 200 lineas sin renumerar; `ESTADO.md` sin cronica (a `HISTORIA.md`, citado en `ARQUITECTURA.map`) | ✅ hecha: `CLAUDE.md` 200, `ESTADO.md` 109, cronica en `HISTORIA.md` |
> | 3 | plugin `orquestador@diego` en el proyecto; skills `entregar`/`verificar` a `.claude/particularidades/` | 🔄 plugin instalado (`6651752`); faltan pre-commit con trinquete y mudar las skills · el `/plugin install` lo hace el responsable |
> | 4 | poda de instrumentos segun el inventario; lo archivado a historico y citado en `ARQUITECTURA.map` | 🔄 opcion B en una rama, midiendo mutantes; despues, goteo con acta de campo |
> | 5 | lo duplicado Maestro/Esclavo a `lib/` | **despues** de que la candidata pase banco |
>
> 📦 **Enviado: `Paquete_Banco_2026-09-28_1e56d83_SIN_BANCO.zip`** (sha256 `5edc8055`) con la APK
> `IOT_VIAL_Semaforos_2026-09-28_1e56d83_SIN_BANCO.apk` (`a249e42a`) y el `.html` de pruebas.
>
> **Lo siguiente: el PDF de Marco con las cintas de LAS DOS tarjetas.** Pedir la version antes de leer nada.

> **Este fichero es el estado VIVO:** lo abierto, lo que bloquea y lo que falta medir. La cronica de los puntos de
> continuacion anteriores (16/09, 14/09, 12/09 y la V9.0) esta literal en [`HISTORIA.md`](HISTORIA.md); el porque de
> cada `N-x`, en [`roadmap.md`](roadmap.md). La spec manda (`CLAUDE.md` §15). El HEAD se mide: `git rev-parse --short HEAD`.

## Que firmware hay en cada equipo (`CLAUDE.md` §0.2)

| equipo | firmware | como se sabe |
|---|---|---|
| instalacion certificada | V8.4, `e303485` (31/07) | la ultima que paso banco |
| Maestro `SERIE:179DB0` (El Sisga) | V9 `SIN_BANCO`: cargado `7ff7d12` (paquete del 08/09), probado despues `b354fe9` (10/09) | cinta y diario del Maestro en `evidencia/` |
| Esclavo del Sisga | sin medir | su cinta sigue sin traerse |
| Maestro `4D2007` (funcional, Marco) | `1889631` **por declaracion**: el puente contesto `FW:--` | cinta del 28/09 |
| paquete enviado el 28/09 | `1e56d83` `SIN_BANCO` | nadie ha informado aun de que este cargado |

- El `Y2` de 32,768 kHz del Maestro `179DB0` NO oscila (`ON:1 RDY:0 BYP:0 SEL:0 EN:0 CNT:--`): es soldadura (`C-6`).
  Un fallo de hora en ESA tarjeta es esto.
- Los «17 segundos del Esclavo» no existen en el firmware (`grep 17000` = cero): son 21,5 s para soltar el verde
  (`SFTY6_SILENCIO_MS - AVISO_AMBAR_TIMEOUT_MS`) y 25 s para el ambar. Pendiente de decision del responsable.

## Dependencias de esta maquina que no estan en el repositorio

- **`D:\toolchain\mingw64`**: el `gcc` de host fuera de la ruta con `ñ` (alli su `ld` no abre `crt2.o`, `N-44`). Si
  desaparece, los arneses que compilan C++ real caen a `ABORTADO` a la vez. Un `ABORTADO` se lee siempre.
- **No hay `java` en el `PATH`.** `gradlew` necesita `JAVA_HOME` apuntando a un JDK de `D:\@Proyect\Baliza\7 sw apk\`;
  `sdk.dir = C:/android-sdk` (lo dice `android/local.properties`).
- **Los paquetes salen a `D:\@Proyect\Entregas_Semaforos\`**, fuera del repo; los viejos, en su `RETIRADOS\`.

## ABIERTO, por orden de lo que duele

Los tres primeros no los cierra nadie escribiendo codigo.

1. **`BAT:--`**: falta un divisor de tension y una entrada analogica (`grep -rn analogRead` da cero; `N-108`).
2. **`J16` p1 lleva 12 V crudos:** taparlo es obligatorio en cada equipo que se monte (`N-120`).
3. **Matriculacion por ID de Bluetooth:** `RF_Packet` son 4 bytes sin campo de direccion. Decision de protocolo del
   responsable, aplazada a despues del banco.
4. `buildCommand()` sigue siendo copia a mano de `generarComando()`.
5. Retirar `parseStatus()` de verdad exige tocar `simulador_app_bluetooth.py` y `documentos_03`.
6. `FW-N53`: decidir si se redefinen los gestos (hoy Auto `A·A·A`, Ambar `B·B·B`). Es decision de spec.

## BLOQUEANTES

| # | Que esta bloqueado | Que lo desbloquea | De quien |
|---|---|---|---|
| BLQ-3 | La Maestro de la sesion 1 del banco (`N-116`) se calienta y muere a los ~30 s; causa que sostiene el cobre: latch-up | Medir el consumo del riel de 3,3 V en frio con fuente limitada. No reenergizar «a ver si pasa» | Responsable |
| BLQ-6 | Luces, Esclavo, `N-151` y `N-152` no han visto cobre; lo posterior a `7ff7d12` no ha tocado una tarjeta con cinta | Una carga y una pasada de ambar, rojo total y `DAR PASO` | Banco |
| BLQ-5 | Ninguna de las 5 entradas de campo esta protegida (`N-120`) | Revision de diseno (2K2 en serie). Mientras, tapar `J16` p1 | Responsable |
| BLQ-4 | Reinicios del ESP32 del Sisga (`OTRO_PERRO`, `SUBIDA_DE_TENSION`) | USB-TTL en `TX0` a 115200, osciloscopio en 3V3 y `EN`, fuente de 5 V buena | Tecnico |
| BLQ-2 | El cristal `Y2` no oscila en la tarjeta medida | Diagnosticar el `Y2` de la segunda tarjeta | Responsable |

## VERIFICACION EN ESCRITORIO — lo que dice la ultima acta

Cifras **copiadas del acta
[`evidencia/2026-09-28_compuerta.txt`](evidencia/2026-09-28_compuerta.txt)**, no escritas a mano —
lo comprueban `documentos_01`, `documentos_04` y `documentos_05` en cada corrida.

| | |
|---|---|
| Flash | Maestro **64.7 %** (**42380** de 65536 B → **23.156 B libres**) · Esclavo **55.4 %** (36336 B) · Repetidor **20.6 %** · ESP32 **35.7 %** |
| Banco por packs | 🔴 **1318/1319 comprobaciones** en **70 packs** — 69 PASS, **1 FALLA**. El rojo es CORRECTO: `decisiones_01_anclas` acusa a **`D-22`**, la única decisión vigente sin construir, y **necesita una tarjeta delante** (`CLAUDE.md` §1: no se decora) |
| Arneses que compilan C++ real | ~~287/287 pantalla~~ *(retirado con el LCD, `D-32` (1))* · **75/75** automático · 22/22 ciclo · **122/122 dos puntas** · **71/71 Degradado a dos puntas** |
| Puente ESP32 | **101/101** |
| App | **310/310** jsdom · **70/70** funcional · **63/63** unitarios · **75/75** TDD |

> 🔴 **Qué HEAD y con qué árbol se midió lo dice el acta en su cabecera, y no se copia aquí**: aquí
> ponía `f27f1a0` cuando el acta citada decía otro. Si dice `CON CAMBIOS SIN COMMITEAR`, sus cifras
> **no corresponden exactamente** a ningún commit, y para que sean reproducibles hay que volver a
> correr la compuerta con el árbol limpio.

## Donde esta cada artefacto

- App: [`05_Funcional/App_Semaforo/`](05_Funcional/App_Semaforo/). APK y paquetes: fuera del repo (arriba).
- Guia de cableado y formulario de vuelta: `05_Funcional/Guia_Cableado_y_Pruebas_Banco.html`, devuelta en PDF.
- Pruebas del funcional del 28/09: `05_Funcional/Pruebas_Funcional_2026-09-28.html`, devuelta en PDF.
- Esquematico KiCad: [`01_Firmware/Controladora_Semaforos/`](01_Firmware/Controladora_Semaforos/).
- Informe de banco 3-4/09: `evidencia/Informe_Pruebas_Banco_Semaforos_V9.0.pdf` (24 de 29 pasos, sobre `617bd00`).
