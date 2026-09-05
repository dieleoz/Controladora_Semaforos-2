# DECISIONES — una fila por decisión, y sólo la VIGENTE

> **Por qué existe este fichero.** El 05/09 se lanzó un agente a retirar el mando de relés
> de las dos puntas sobre una frase dicha de viva voz. La decisión contraria estaba escrita
> desde el 31/08 en `05_Funcional/17_...`, y no se encontró porque **las decisiones de este
> repositorio se añaden sin derogar las anteriores**: `roadmap.md` tiene 4.700 líneas y un
> `grep` devuelve la versión que case primero. Hubo que matar al agente.
>
> **Regla: una decisión vigente vive AQUÍ, en una línea. Todo lo demás apunta a este
> fichero en vez de repetirla.** El porqué largo se queda en el roadmap; lo que manda es
> esta tabla. Si una fila de aquí y un párrafo de allí no coinciden, **gana ésta**, y el
> párrafo está caducado.

**Antes de encargar un cambio de alcance —a un agente o a ti mismo— se lee esta tabla.**
Si el encargo contradice una fila, eso no es una orden: **es una pregunta.**

---

## Vigentes

| # | decisión | fecha | motivo | deroga |
|---|---|---|---|---|
| **D-1** | **El mando de relés SE CONSERVA en los canales `A` y `B`** (`J16` p5 `PB9`, p8 `PB13`). Van cableados, hoy sin usar, **y su código NO se toca** | 31/08, reconfirmada 05/09 | `mando_ambarLocal()` tiene **seis lectores** y su veto es SFTY-21. Retirar el armador de la bandera deja los `if` siempre verdaderos: **el veto no queda inerte, queda abierto**. Y medido: el banco se caería en **ABORTADO**, no en rojo — dos `raise` disparan solos y los dos modelos leen constantes de `mando.cpp` **en el import** | «se retira el mando de 4 relés» (28/08) |
| **D-2** | **`BOTON3` (`PB14`, p10) y `BOTON4` (`PB15`, p12) son las DOS CÁMARAS** | 28/08 | son los pines que las cámaras necesitan y los que el mando no usa | los cuatro pulsadores |
| **D-3** | **`M3` CERRADA: las cámaras se cablean a `J16`** | 03/09 | medido en cobre: pull-down real de 10 kΩ en las cuatro posiciones (`R65`–`R68`), p10 y p12 a **0 V** en reposo, y el paso 21 cableó p10 contra p11 **sin demandas fantasma** | «no se cablea cámara a `J16` hasta M3» |
| **D-4** | **`J16` p1 se TAPA en cada equipo que se monte** | — | lleva **12 V crudos** a un conector de señal directa al micro (N-120) | «cautela de banco» |
| **D-5** | **Mínimo por sentido = 3 minutos** (`VERDE_MIN_MIN = 3`) | 04/09 | por debajo, el conductor se convence de que el semáforo está averiado y adelanta en rojo | `VERDE_MIN_MIN = 1` |
| **D-6** | **La pantalla LCD NO se retira** | 04/09 | 271 comprobaciones cuelgan de ella | «se retira la pantalla» (28/08) y la «Ola D» del roadmap |
| **D-7** | **En Manual, `DAR PASO` alterna rojo/verde como el automático**, disparado por el botón. Termina en **rojo+verde**, no rojo+ámbar. El todo-rojo de despeje **se queda** | 04/09 | el automático también lo hace, y es lo que garantiza que el tramo quedó vacío. Configurable 10–90 s, hoy 15 | «Manual lleva su propio ciclo» |
| **D-8** | **El ámbar de emergencia conserva SUS DOS VETOS** (mando y app) | 04/09 | el banco tumbó **dos veces** la versión sin cerrojo. Al medirlo, el veto **no era** la causa del bloqueo: lo era que esa punta no acusaba | la decisión «(b), sin cerrojo cuando viene de la app» |
| **D-9** | **La hora la pone el DS3231 del ESP32**; el STM32 no tiene reloj (`Y2` muerto, N-17) | 04/09 | el puente rellena el hueco `HORA:--:--:--` al pasar la trama y recalcula el CRC | «la hora la lleva el RTC interno del STM32» |
| **D-10** | **Cámara comprada: Hikvision `DS-2CD2683G2-IZS`** | 05/09 | tiene salida de alarma (`1 in, 1 out, 24 V/1 A`, ficha oficial) | los modelos anteriores de la lista |
| **D-11** | **Al aplicar tiempos, la app AVISA y da el botón: NO arranca el ciclo sola** | 05/09 | arrancar el ciclo abre paso, y hacerlo automáticamente se salta la confirmación de vía (§6) | — |
| **D-12** | **De cada cámara el sistema consume UN CONTACTO SECO. No hay red, no hay imagen, no hay vídeo, y no hay analítica en el controlador** | 05/09 | medido: cero `WiFi`, `HTTPClient`, servidor u ONVIF en todo el ESP32; el STM32 sólo lee un pin. **Consecuencia: toda la inteligencia vive en la CONFIGURACIÓN de la cámara**, y el manual de parametrización pasa de documento de apoyo a entregable principal. **Y lo que se pierde: sin imágenes NO hay soporte de accidentes ni auditoría** — que era el uso que el responsable les había encontrado | «imágenes y auditoría en la Raspberry o la Nano», propuesto el 04/09 y recomendado por dos revisiones el 05/09 **sin comprobar que hubiera camino** |

---

## 🟡 Abiertas — y aquí NO se decide por descarte

| # | pregunta | qué falta para poder decidirla |
|---|---|---|
| **A-1** | **¿Qué significa cada uno de los DOS bits que tenemos?** | Ver **D-12**: no hay red ni imágenes, así que **una cámara = una salida = UN significado**, y con dos cámaras hay **dos significados para todo el cruce**. Uno está tomado (presencia antes de bajar la pluma). El otro está por decidir entre: **vehículo detenido en el tramo**, **invasión en rojo**, **espera prolongada**, o **vigilancia de la propia cámara**. Todos como EVENTO, no como control |
| **A-2** | **¿Qué se pone en `J16` p5 y p8?** | Quedan libres si algún día se retira el mando — hoy **no se retira** (D-1). Idea sin decidir: **fin de carrera de la talanquera**, que hoy es **lazo abierto** |
| **A-3** | **¿A quién le habla el operario al poner la hora?** | El ESP32 contesta `$ACK` y el STM32 `$ERR NO_QUEDO_PUESTA`. **Las dos son ciertas** |
| **A-4** | **¿Qué pasa con `MENU` si se replantea la interfaz?** | Es el estado «parado» del que depende fijar tiempos: `C_MENU_IDLE` fuerza rojo a las dos puntas |
| **A-5** | 🔴 **¿Había un DS3231 conectado en el banco del 04/09?** | `ESTADO.md` dice a la vez «N-145 confirmado en cobre» (`HORA:22:19:58`) y «falta comprar el DS3231». **No pueden ser las dos.** El ESP32 no tiene otra fuente de hora — medido: `getLocalTime`, `settimeofday`, `configTime`, `time()` dan **cero** en su fuente. Sólo lo contesta quien estaba delante |
| **A-6** | **¿Se implementa la alarma de «la barrera lleva N días sin bajar»?** | La enunció el responsable y **no existe en el firmware** |

---

## Cómo se cambia una fila

1. Se escribe la nueva, con **fecha y motivo medido**. Un motivo sin números se deroga de
   palabra: eso es lo que pasó con D-1.
2. La anterior se **tacha aquí y se deja**, no se borra — una decisión que desaparece en
   silencio vuelve a proponerse dentro de un mes.
3. Si la decisión **retira una barrera**, además se censa quién depende de ella antes de
   tocar nada (`CLAUDE.md` §3.ter).
