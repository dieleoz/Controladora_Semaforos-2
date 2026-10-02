# DECISIONES - indice de lo ya decidido

Indice de lo que el responsable ya contesto, para no volver a preguntarlo. No es la spec: **manda la SPEC**
(`SPEC_0..SPEC_8`, y `05_Funcional/17_...` en cobre medido; `CLAUDE.md` §15). Cada fila da la respuesta en dos lineas
y la seccion de la SPEC que la ejecuta; si la fila y la SPEC no coinciden, se corrige una de las dos en el mismo commit.
Una decision nueva se escribe en el MISMO commit que su SPEC. Derogar es BORRAR la fila: su Id pasa a «Derogadas».
Antes de encargar un cambio de alcance se lee esta tabla: si el encargo la contradice, es una pregunta, no una orden.
Lo historico (motivos, fechas, cronica y A-x cerradas) esta en `05_Funcional/historico/DECISIONES_hist.md`.

Superficie: `firmware` exige ancla en el fuente (`decisiones_01_anclas`); `camara`, `campo`, `compra` y `app`, no.

## Vigentes

| Id | Respuesta | Vive en | Superficie | Deroga |
|---|---|---|---|---|
| **D-1** | El equipo se opera SOLO por la app; el mando de reles no existe. Su codigo salio del firmware (`D-30`, `D-44`). | SPEC_0 §1, SPEC_4 §3.bis | firmware | «se retira el mando» leido como hardware solo |
| **D-2** | `BOTON3` (`PB14`, `J16` p10) y `BOTON4` (`PB15`, `J16` p12) son las entradas de las camaras. | SPEC_5 §2.1 | firmware | los cuatro pulsadores |
| **D-3** | Las camaras se cablean a `J16` (medida M3 cerrada: pull-down de 10 kOhm, p10 y p12 a 0 V en reposo). | SPEC_5 §2.1 | firmware | «no se cablea camara a `J16` hasta M3» |
| **D-4** | `J16` p1 lleva 12 V crudos a un conector de senal directa al micro: se TAPA en cada equipo que se monte. | SPEC_5 §1 | campo | «cautela de banco» |
| **D-5** | Minimo por sentido 3 min (`VERDE_MIN_MIN = 3`). | SPEC_1 §5 | firmware | `VERDE_MIN_MIN = 1` |
| **D-7** | En Manual, DAR PASO alterna rojo/verde como el automatico y conserva el todo-rojo de despeje (10-90 s). | SPEC_1 §8 | firmware | «Manual lleva su propio ciclo» |
| **D-8** | El ambar de emergencia del Poste 2 conserva su veto: no lo pisan la orden de rojo, la de verde ni la recuperacion tras fallo. Hoy lo pone un solo sujeto, la app. | SPEC_1 §4, SPEC_2 §2.3 | firmware | «sin cerrojo cuando viene de la app» |
| **D-9** | La hora la da el `DS3231` del ESP32; el STM32 no tiene reloj propio (`Y2` muerto). | SPEC_3 §1 | firmware | «la hora la lleva el RTC interno del STM32» |
| **D-10** | Camara comprada: Hikvision `DS-2CD2683G2-IZS` (1 in, 1 out, 24 V / 1 A). | SPEC_5 §3 | compra | los modelos anteriores de la lista |
| **D-11** | Al aplicar tiempos la app avisa y ofrece el boton: fijar tiempos NO arranca el ciclo. | SPEC_1 §5 | firmware, app | - |
| **D-12** | De cada camara el controlador consume UN contacto seco: sin red, sin imagen, sin analitica. La grabacion vive en la microSD de la camara. | SPEC_5 §3, SPEC_8 | firmware | «imagenes y auditoria en Raspberry o Nano» |
| **D-13** | Las camaras llevan la misma configuracion: Intrusion Detection sobre el barrido de la pluma. El significado lo pone el estado del semaforo; no tocan el ciclo. | SPEC_8 §1-§2 | firmware, camara | «un significado por camara» |
| **D-14** | La entrada de alarma de la camara (grabar cuando el controlador cierra un contacto) NO se instala en este despliegue (`D-32` (4)); la capacidad queda. | SPEC_5 §2, §6 | campo | «solo se mira la salida de la camara» |
| **D-15** | El reloj lo lleva el ESP32 de cada punta y es el UNICO que contesta a `SET_RTC`; la app valida la hora en las dos. | SPEC_3 §1, SPEC_4 §1 | firmware | «el STM32 contesta a `SET_RTC`» |
| **D-16** | Sin telefono no hay forma de operar el equipo: es una propiedad declarada del sistema, no una averia, y va en el manual del operario. | SPEC_0 §1, SPEC_4 | firmware, app | «siempre queda el mando desde el suelo» |
| **D-17** | `CMD:LEER_RTC` consulta el reloj sin cambiarlo en Maestro, Esclavo y telefono; la app muestra el desfase entre postes. | SPEC_6 A.6 | firmware, app | «hay que sincronizar los dos relojes entre si» |
| **D-17.bis** | La pantalla LCD y el menu se retiran del equipo: todo se opera por la app. Su codigo salio con `D-30` y `D-44`. | SPEC_0 §1, SPEC_4 §3.bis | firmware | `D-6` |
| **D-18** | El Degradado sin testigo del Poste 2 entra por su puerta unica `degradado_entrar()`, que rechaza con motivo. La app ya no lo ofrece (`D-37`) y su orden sale (`D-46`). | SPEC_2 §7 | firmware | dos pulsadores fisicos en el Esclavo |
| **D-19** | El Inteligente usa los tiempos del operario: suelo = tiempo de la fase, techo = el doble (`TECHO_POR_SUELO = 2`). POR VALIDAR hasta la firma del funcional. | SPEC_1 §7 | firmware | piso de 15 s; techo fijo de 3 min |
| **D-20** | La autoridad de la hora es el ESP32: la app la da al ESP32 Maestro, este al Esclavo, y cada STM32 la recibe del suyo. El Esclavo sobrescribe: una sola fuente. | SPEC_3 §1 | firmware | «la hora la lleva el RTC del STM32 y viaja por radio» |
| **D-21** | La hora no fiable se publica; la app alarma al conectarse a ESE poste y la alarma se quita poniendole la hora por Bluetooth. En Degradado, rojo fijo (`D-38`). | SPEC_3 §4-§5, §7 | firmware, app | «sin hora = no entra en Degradado» a secas |
| **D-22** | `Y1` (8 MHz) como latido del STM32: opcional, la ULTIMA de la cola, sola y cargada con una tarjeta delante; si no oscila, cae al HSI y lo declara. Sin construir. | SPEC_7 §5.1, SPEC_3 §1 | firmware | «el `millis()` del STM32 sirve para tiempo largo» |
| **D-23** | La app tiene una pantalla propia del Poste 2 al conectarse a el por Bluetooth; el diagnostico llega por un `$EVENT` periodico (`D-32` (2)). | SPEC_4 §6 | firmware, app | «la app ensena el cruce y con eso basta» |
| **D-24** | `CAM_CIEGA` salta a las 24 h de paso abierto sin flanco (`CAM_CIEGA_MS`). Construido; no ejercido en su tiempo real. | SPEC_8 §4 (sin nombrarla) | firmware | los 4 dias del 05/09 |
| **D-25** | Cuatro camaras, dos por poste: `J16` p9-p10 (`PB14`) y p11-p12 (`PB15`), por su contacto seco `1A`/`1B`. Talanquera en `J15` (p2 es el drenador de `Q10`, no masa). | SPEC_5 §2.1, §3, §4 | firmware, campo | de `D-13`, «una camara por poste» y «p12 vacio» |
| **D-26** | Cada ESP32 siembra a su STM32 (`CMD:HORA_ESP32`); `SET_RTC` sin PIN en el ESP32. Esclavo: con radio, la hora del Maestro; sin radio, la de su ESP32. En Degradado un salto pasa por rojo; dos alarmas. | SPEC_3 §2-§3, §7 | firmware | el numero de la siembra de hora de 08/09 (una hora) |
| **D-27** | Guia del Sisga: cuatro camaras compradas; `J14` libre y sin cablear (el fin de carrera no se instala); camara configurada segun el manual del modelo; talanquera con rele y centralita. | SPEC_5 §2-§4 | campo | de `A-2`, «el fin de carrera va a `J14`» |
| **D-28** | Cadencia de siembra 120000 ms exactos; `HORA_CADUCA_MS` = 400 s, para aguantar dos siembras perdidas seguidas. | SPEC_3 §2, §4 | firmware | de `D-26` el «~2 min»; de `D-21`, el plazo derivado del relevo |
| **D-29** | Un corte de luz no mata la reanudacion del Degradado: el indicador de la pila no se borra hasta la primera siembra del arranque. El limite de 48 h sigue mandando. | SPEC_3 §6, SPEC_2 §7 | firmware | que `N-20` hubiera muerto en el Esclavo |
| **D-30** | El LCD y el mando A/B/C/D salen del firmware (reafirmada el 14/09); lo que quedaba de la interfaz lo saca `D-44`. | SPEC_1 §4, §11, SPEC_4 §3.bis | firmware | de `D-1`, «el codigo se queda»; de `D-17.bis`, el software de la pantalla |
| **D-31** | El aviso de ambar del Poste 2 al Poste 1 se acusa (solo el primero; el cancelar tambien); 3 reintentos, 14 s. Si no llega, la app dice «no he podido confirmarlo». | SPEC_2 §2.1, §6, SPEC_6 C | firmware, app | el aviso sin acuse ni reintento |
| **D-32** | Del 13/09 siguen: `D-23` por `$EVENT` periodico; `AVISO_AMBAR_REINTENTOS = 3` (14 s); `D-14` no se instala en este despliegue. Su (1), «el mando se queda», cayo el 14/09. | SPEC_2 §6, SPEC_4 §6, SPEC_5 §2 | firmware | de `D-31` el reintento unico; de `D-23` el mecanismo sin elegir |
| **D-33** | La camara veta la bajada de la pluma (cualquiera de las dos, veto local); la pluma baja 3 s despues del rojo; ante error no baja y avisa. Sin umbral de falsas alarmas. | SPEC_8 §1, §6 | firmware, app | `SFTY-28` en su «nunca al reves» |
| **D-34** | La ventana ambar contra verde se cierra en las dos puntas: el Esclavo suelta su verde antes de su silencio y el Maestro no entrega `GO_GREEN` sin un `PONG` reciente. | SPEC_2 §3-§4 | firmware | «lo que se alarga es una espera, no un riesgo» |
| **D-35** | Degradado con testigo: `SET_MODO:DEG_T:ahora,inicio,verde,despeje`; rechaza si la hora difiere; rojo fijo hasta `inicio` y luego alterna sin radio. No vence; alarma a los 28 dias. | SPEC_2 §7.bis, SPEC_4 §3.ter | firmware, app | el vencimiento a 31 dias |
| **D-36** | La app no ofrece «Solicitar Paso»: en campo el paso se da con Modo Manual y DAR PASO. La orden sale del firmware (`D-46`). | SPEC_4 §3.2 | app | - |
| **D-37** | La app no ofrece el Degradado sin testigo; queda el testigo (`D-35`). La orden `SET_MODO:DEGRADADO` sale del firmware (`D-46`). | SPEC_4 §3.1-§3.2 | app | el boton de `D-18` |
| **D-38** | En Degradado (testigo o automatico) la punta que pierde la hora fiable pasa a ROJO FIJO, no a ambar. Construido; sin banco. | SPEC_3 §5 | firmware | de `D-21` el ambar, solo dentro del Degradado |
| **D-39** | Las cuatro camaras detectan Vehiculo y Humano; sensibilidad 50 %, se reajusta en campo con el enfoque. Precio: alguien parado en el barrido deja la pluma arriba. | SPEC_8 §1 (sin nombrarla) | camara | de `D-27` (3), el filtro «solo Vehiculo» |
| **D-40** | Tras corte o watchdog el Poste 1 arranca en AMBAR INTERMITENTE y sale con una orden; si estaba en Degradado, reanuda (`D-29`). El Poste 2 arranca en rojo. | SPEC_1 §4.1 | firmware | el arranque del Poste 1 en Menu |
| **D-41** | Se acepta la ventana del testigo: si el operario no llega al Esclavo antes de `inicio`, el Maestro da verde contra su ambar. La acota el traslado. | SPEC_2 §7.bis | campo | - |
| **D-42** | La pluma retenida baja cuando la camara deja de ver: su salida de alarma queda activa 5 s tras el evento (de fabrica). El firmware no cambia: retardo del rojo, 3 s. | SPEC_8 §1 | camara | - |
| **D-43** | En carretera el Degradado automatico va APAGADO (como sale de fabrica); sin radio se usa el testigo, con el traslado real (15-20 min) en su campo de la app. | SPEC_2 §7.ter (sin nombrarla) | campo | - |
| **D-44** | Sale del firmware la interfaz local que quedaba: lectura de `J16` p5/p8, `botonAceptar`/`Cancelar`, navegacion del menu, Modo Hora, `semaforo_toggle()`. `menu_setup()` se queda. | SPEC_1 §12 (sin nombrarla) | firmware | de `D-1` y `D-32` (1), «el codigo se queda»; de `D-17.bis`, Modo Hora |
| **D-45** | Secuencia de la norma ROJO - VERDE - AMARILLO 3 s - ROJO en todos los caminos (tambien `FORZAR_ROJO`); sale el ambar previo al verde. Construido en `cda33df` (`N-174`), sin banco. | SPEC_1 §3.2, SPEC_2 §2.2.bis | firmware | SPEC_1 §3.1 (verde a rojo directo) |
| **D-46** | Salen las ordenes `SOLICITAR_PASO` (Esclavo) y `SET_MODO:DEGRADADO`. Se quedan `J14`/`PB0` (reservada al fin de carrera), el andamio `SFTY-20` y los pines de peaton y zumbador. | SPEC_4 §3.1-§3.2 (sin nombrarla) | firmware | de `D-36` y `D-37`, «la orden se queda» |
| **D-47** | El rojo fijo por falta de hora (`D-38`) sobrevive a un corte: ese poste arranca en rojo fijo, no en ambar; si estaba en verde, antes 3 s de amarillo. Construido en `cda33df` (`N-174`), sin banco. | SPEC_3 §5, SPEC_1 §4.1 (sin nombrarla) | firmware | - |
| **D-48** | El umbral de silencio de la radio (`SFTY6_SILENCIO_MS`) sube de 25 a 28 s para que quepa el amarillo del Esclavo (`D-45`) con los reintentos del ciclo. Construido en `cda33df` (`N-174`), sin banco. | SPEC_2 §4, §9 (sin nombrarla) | firmware | «el umbral no se toca», que solo prohibia bajarlo |
| **D-49** | En Degradado, el reloj que se congela en marcha (cristal que deja de oscilar) va a ROJO FIJO como `D-38`, no a ambar; antes 3 s de amarillo si estaba en verde. Construido en `cda33df` (`N-174`), sin banco. | SPEC_3 §5 (sin nombrarla) | firmware | de `D-38`, la excepcion del reloj que no cuenta |
| **A-15** | Degradado automatico (29/09): activado con PIN en los dos postes, tras 5 min sin radio entra por la puerta del testigo. Construido; sin banco. En carretera va apagado (`D-43`). | SPEC_2 §7.ter, SPEC_4 §3.ter.bis | firmware, app | SPEC_0 §5.7 «no entra solo»; de `D-18` y `D-21`, la llave solo en la app |

Derogadas: D-6

---

## Abiertas

| Id | Pregunta | Que bloquea |
|---|---|---|
| **A-0** | Grabacion en las microSD (cuatro de 64 GB, compradas): dias de retencion, y continua o por evento. La guia del poste no configura `Trigger Recording` ni el `Record Schedule`. | la parametrizacion de la camara; no toca firmware |
| **A-4** | Que pasa con `MENU` si se replanteara la interfaz: hoy solo se alcanza con `SET_MODO:MENU` desde la app y es el todo-rojo de las dos puntas. | nada hoy |
| **A-10** | Si el LED `D21` de `VERDE2` enciende con la salida activa: comprobacion de banco (paso 7.bis de la guia), no de firmware. | fabricar mas placas sin saberlo |
