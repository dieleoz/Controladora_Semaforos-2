# Roadmap — Controladora de Semaforos Moviles de 3 Estados (V9.0)

**Arranca el 31 de Agosto de 2026.** Este fichero lleva **el estado de lo que tenemos**, no una
bitacora. Lo anterior no se pierde —vive en el `git log` de este repositorio y en el remoto
`padre`—, pero no se arrastra aqui: lo que no sirve para decidir hoy, no esta.

> **Como se lee.** Arriba, **lo que hay y lo que esta decidido**. Abajo, los `N-x` de esta sesion,
> que son **el porque** de cada decision con su medida al lado. Un roadmap sin el porque obliga a
> volver a descubrirlo, y en este proyecto eso ya se pago tres veces.

---

## 0. PARA RETOMAR — leelo entero antes de tocar nada

### 0.0 · 🟢 EL BANCO CORRIO. Lo que este roadmap daba por bloqueante ya no lo es

**El 03 y 04/09 el funcional ejecutó la guía de 29 pasos sobre el paquete V9.0 (`617bd00`), con dos
tarjetas cargadas.** Informe en `evidencia/Informe_Pruebas_Banco_Semaforos_V9.0.pdf`, preparado por
Sebastian, equipo `nitro5-marco`. **24 de 29 pasos completados y verificados en hardware.**

Durante 34 dias este fichero repitio *«ninguna linea ha tocado una tarjeta»* y *«BANCO sigue siendo
EL bloqueante»*. **Eso dejo de ser cierto el 03/09 y el resto del documento se lee con eso delante.**

Lo que el banco confirmo funcionando en cobre, que no es poco y no lo decia nadie hasta ahora:

```
carga de firmware por SWD/ST-LINK, las dos puntas, al primer intento y sin BOOT0
radio Maestro <-> Esclavo, con caida a ambar intermitente en ~20 s y vuelta sola en ~3 s
talanquera J15: sube en ambar, baja al recuperar enlace
camara de demanda J14: 3,3 V / 0 V y conmutacion correcta
camara cableada en J16 p10: sin falsa activacion, con y sin el cable puesto
masa comun del modulo definitivo contra la STM32: 0 V (umbral exigido 50 mV)
identidad real de J17 -RESUELTA-: es el UART del ESP32, no el LCD del netlist
```

**Y lo que el banco NO pudo probar, que es lo que manda ahora:** 4 pasos bloqueados en cascada por
el enlace Bluetooth, 1 abortado por un incidente de seguridad, y **la tarjeta Maestro fuera de
servicio**. Ver **N-115**, **N-116** y **N-117**.

> ⚠️ **La regla §2.bis no se relaja por esto — se refuerza.** El banco no invalido ni un instrumento:
> **encontro tres cosas que ninguno de los 34.532 renglones podia ver**, porque ninguna es una
> propiedad del fuente. Un chip que se calienta, un modulo que no se anuncia y una resistencia de
> 10 kOhm en el cobre no salen de leer C++. Esa es exactamente la diferencia que §2.bis nombra.

| | firmware | instrumento | ratio |
|---|---|---|---|
| 28/08 | 8.895 | 8.898 | 1,00 : 1 |
| 02/09 | **14.976** | **34.532** | **2,31 : 1** |

**Tres auditorias externas independientes dijeron lo mismo**, y la tercera lo dijo de la respuesta
a la segunda: *«arreglamos todo lo que midio y nada de lo que dijo»*. Ver **N-109**, **N-114** y la
regla **§2.bis de `CLAUDE.md`**, que existe por esto.

> **La pregunta antes de escribir cualquier cosa: ¿esto acerca una tarjeta cargada, o la sustituye?**
> Desde el 03/09 hay una segunda, y es mejor: **¿esto desatasca uno de los 5 pasos que el banco no
> pudo correr?** Lo que no conteste a ninguna de las dos, no se escribe.

### 0.1 · Lo unico que hay que hacer, en orden — tras la SESION 2 de banco (04/09)

> 🟢 **EL BLUETOOTH ESTA CERRADO CON EVIDENCIA FISICA.** La sesion 2 confirmo **N-117** y **N-122** en
> hardware, con la app operando el equipo de punta a punta, y encendio **VERDE por primera vez en dos
> sesiones**. Ver **N-126**.
>
> 🔴 **Y ahora hay UN solo bloqueo, y es fisico: no hay segunda tarjeta.** De ahi cuelgan los tres
> asuntos que la sesion 2 dejo abiertos —N-42, la verificacion del mando, y los pasos 25/26/28—. **No
> lo destraba nadie escribiendo.**

| | que | por que |
|---|---|---|
| **0** | 🎯 **El paso 29, otra vez — pero con el gesto BUENO y contando los DESTELLOS** | **no necesita segunda tarjeta, ni app, ni cable.** Salio «inconcluso» porque nuestra guia mandaba el gesto viejo (a masa). Con `p5` contra `p4`: **2 destellos rojos** y N-118 queda verificado. Es lo mas barato que queda por hacer |
| **0.bis** | 🎯 **Cronometrar el arranque del ESP32 con el modulo delante** | cierra la desigualdad que la respuesta 8 dejo rota. Lo medido -2 a 3 s- incluye emparejamiento y app; hace falta *reset -> primer byte*, que es menor |

| | que | por que |
|---|---|---|
| **1** | 🛑 **La tarjeta Maestro: STM32 confirmado muerto y los pads con cortos.** Hay que montar una **nueva desde cero** | sin Maestro no hay ciclo, no hay paso 7 y **no se cierra N-42**. Es lo unico que bloquea de verdad. 🛑 **Y la que esta no se reenergiza** |
| **2** | 🟢 **Al soldar la placa nueva: 2K2 en serie en las 5 entradas de campo, y no poblar los 12 V de `J16` p1** | **es el momento y no vuelve.** N-120: hoy las entradas van desnudas al die mientras las 9 salidas llevan 220 Ohm y opto. Si se suelda igual, la placa nueva nace con la misma averia dentro |
| **3** | 🟢 **Cargar el firmware ANTES de enchufar nada**, y comprobarlo con el **paso 2.bis** de la guia | `J16` vacio, medir contra masa: **0 V en las cuatro posiciones**. Si p5/p8 dan 0,6 V, entro el binario viejo. Un multimetro y diez segundos |
| **4** | 🔴 **Con el Esclavo solo, ya se puede: pasos 11-14 y 25-28** | telemetria viva, `AMBAR_EMERGENCIA`, `FORZAR_ROJO`, la barrera de PIN y `SET_RTC`. **No hace falta el Maestro para nada de eso** |
| **5** | 🔴 **Repetir los pasos 7, 19 y 21** cuando haya Maestro sano | son los que deciden si **N-42** sigue viva. El banco **no la confirmo ni la descarto** |
| **6** | 🔴 **Cargar `SFTY6_SILENCIO_MS = 25000UL` sobre `e303485`** — solo esa constante, sobre la V8.4 que **ya esta probada en la calle** | sigue siendo lo unico que llega al conductor esta semana, y **no depende de nada de la V9.0** |

#### Lo que sigue SIN medir, y son medidas, no decisiones

| | quien |
|---|---|
| **Que la app recibe TELEMETRIA VIVA**, no solo que conecta. *«Ya conecta seguramente»* es una suposicion; lo constatado es que dejo de fallar el escaneo. La prueba que lo cierra: dejar el ESP32 hablando con el Esclavo, **reiniciarlo**, y ver si pasa de `SEM-SIN-MATRICULA` a `SEM-<serie>-E` | Marco |
| **AB-3: cuanto tarda el ESP32 desde tension hasta pasar bytes.** Hoy esta puesto a ojo y gobierna su watchdog. Con el movil basta | Marco |
| **La tension de `J16` p5/p8 con el puente a 3,3 V y el firmware nuevo** — el dato que el paso 29 nunca tomo | Marco, con Maestro sano |
| **La fuente 12 V -> 5 V con carga real.** El banco se alimento por USB | Marco |

**Lo que NO hay que hacer:** ni un pack, ni un arnes, ni un documento — salvo que **conteste una
pregunta abierta**, que es la excepcion escrita en §2.bis.

> 🔴 **Y el 04/09 lo dejo demostrado, no advertido: `CLAUDE.md` §2.ter.** Los cinco defectos que
> pararon el banco vivian entre algo **declarado** y ese algo **ejercido** —un permiso declarado sin
> pedir, una funcion definida sin llamador, un MAC escrito sin equipo, una polaridad sin cobre, una
> constante sin cronometro—, y **cuatro de los cinco no se pueden ver desde el PC**. Un pack mas
> habria dado verde igual.

### 0.2 · Donde esta todo, medido el 04/09 tras el arreglo de N-117

```
compuerta      20 PASS | 0 FALLA | 0 ABORTADO   (acta evidencia/2026-09-04_compuerta.txt)
banco          964/964 en 66 packs
firmwares      Maestro 89,3 % (7.040 B libres) · Esclavo 65,9 % · Repetidor 20,6 % · ESP32 35,7 %
simuladores    9/9 · 10/10 · 12/12 · 85/85
arneses C++    pantalla 271/271 · ciclo 22/22 · automatico 71/71 · dos puntas 42/42 · Degradado 18/18
app            jsdom 128 · unitarios 32 + 55 · funcional 58/58
```

**Y ahora, al lado, la cifra que hasta el 03/09 no existia:**

```
BANCO          24/29 pasos COMPLETOS  ·  4 BLOQUEADOS (Bluetooth)  ·  1 ABORTADO (seguridad)
               informe: evidencia/Informe_Pruebas_Banco_Semaforos_V9.0.pdf
```

> ⚠️ **El 20/20 sigue sin ser un entregable, y ahora hay prueba de por que.** Las tres cosas que
> pararon el banco —el chip que se calienta, el modulo que no se anuncia, los 10 kOhm del cobre—
> pasaron **las 20 comprobaciones sin despeinarlas**. Verde no es entregable: esta vez con el
> contraejemplo delante en vez de como advertencia.

### 0.3 · Lo que espera DECISION del responsable, no trabajo

| | |
|---|---|
| 🛑 **Que se hace con la tarjeta Maestro danada** | reparar, sustituir o diagnosticar. **Bloquea todo lo demas**: sin Maestro no hay banco. La causa que sostiene el cobre es **latch-up por 12 V en una entrada sin proteger** — ver **N-116** |
| 🔴 **Proteger las entradas de campo** —hoy los 5 pines de bornera van **desnudos al die** mientras las 9 salidas llevan 220R y opto | **N-120.** Es de diseño y afecta a todas las unidades, no solo a la danada. Cuenta hecha: **2K2 en serie** cumple las dos desigualdades. **Y mientras no exista, tapar el pin de 12 V de `J16` pasa a ser obligatorio en cada equipo** |
| 🟠 **Con que salida se compra el receptor de mando (NO/NC)** | **N-118**. La polaridad ya no se pregunta: el cobre la decide —los cuatro pines de `J16` tienen 10K a masa y 3,3 V al lado, o sea **activo en ALTO**—. Lo que queda es la compra, y quien valida que un cambio en un camino de seguridad entra sin banco |
| 🟢 **La cadencia del `$STATUS` por J17** | ✅ **DECIDIDA Y APLICADA el 04/09: baja a 2000 ms** en las dos puntas. **MEDIDO tras el cambio: 462 B de 960 B/s = 48,1 %**, no «bajo el 30 %» como se publico aqui. Solo el `$STATUS` periodico se parte por dos; el `$EVENT`, el `$ALARM` y el `$ACK` que coinciden en el peor segundo **no escalan con la cadencia**. Era una cuenta hecha a ojo con autoridad de dato. El coste declarado: el tablero refresca la mitad de rapido. **Cifras en N-119** |
| 🔴 **Quien disena y quien fabrica** la placa portadora | bloquea el montaje permanente, **no la prueba** |
| 🔴 **Pedir la fuente `A5`** — conmutada 12->5 V, >= 1 A | |
| 🔴 **`AB-9`: el PIN no caduca NUNCA** | se teclea, se guarda el telefono, y el siguiente manda ordenes sin teclear. **Cinco opciones con su coste en §0.quinquies** |
| 🔴 **Por que los documentos COPIAN cifras en vez de citarlas** | toda la familia N-62 -> N-93 -> N-112 sale de esa duplicacion, y hay **1.120 lineas de Python vigilando copias**. Si dijeran *«ver la ultima acta»*, dos packs enteros sobran |
| 🟠 **`AB-1`: el latido del ESP32** | sin el, el contador de `J17` **no distingue** un puente muerto de un telefono apagado |
| 🟠 **Los codigos de los mandos** · **`M3` en cobre** | compras y una medida de multimetro |

### 0.4 · Deuda de firmware, si se decide seguir por ahi

1. **El Esclavo no tiene `reloj_diagnostico()`** — porte **mecanico** desde el Maestro; ya tiene todos los ingredientes. Sin el, la mitad del diagnostico de `Y2` sigue tapiada.
2. 🔴 **En la SUBIDA no hay checksum en ningun sitio.** Ninguna punta llama a `calcularChecksum()` **en recepcion**: un bit cambiado dentro del parametro de `SET_TIEMPOS` o `SET_RTC` casa con el `strncmp` del prefijo y **el equipo obedece valores mutilados**.
3. **N-42** — el Modo Automatico no mueve las luces en banco. Abierta desde antes de toda esta arquitectura.

### 0.5 · El paquete que esta en la mano del funcional

```
Encargo_Banco_2026-09-04_6126bfa_SIN_BANCO.zip     5.419.031 B · 152 entradas
APK  IOT_VIAL_Semaforos_2026-09-04_6126bfa_SIN_BANCO.apk
     md5 3199cab8dde4679eaab1c742c915ccc5
     lleva N-122 (connect), N-124 (la lista sale del escaneo) y N-125 (pide el permiso)

01_Firmware/   COMO_CARGAR.md  ·  TROUBLESHOOTING.md
               Maestro/ 50  ·  Esclavo/ 36  ·  ESP32_Expansion/ 17
02_Manuales/   43
raiz           LEEME_PRIMERO.md · Guia_Cableado_y_Pruebas_Banco.html · ACTA
```

**SE RECORTO A PROPOSITO EL 04/09, y el motivo es del que lo abre:** antes iban los **257** ficheros
versionados de `01_Firmware`, o sea 88 de simuladores, 37 de arneses, 16 de KiCad —4,8 MB— y el
Repetidor, que hoy no se carga en ningun sitio. **Quien abre el paquete para CARGAR no sabe cual de
las trece carpetas mirar, y el ruido esconde lo que importa.** Quedan **tres carpetas, tres cosas que
se cargan**, y una hoja —`COMO_CARGAR.md`— que dice cual va en cada tarjeta, con que se carga, en que
orden, y **que NO esta y por que**, para que nadie lo busque.

**Y el armador lo comprueba sobre el propio zip, no sobre la intencion:** artefactos de compilacion
= 0 (se parte de `git ls-files`, no de una lista de exclusiones), md5 de la APK de dentro igual al
del repositorio, el LEEME citando su nombre exacto, el LEEME diciendo **NO** al banco en la primera
pantalla, que en `01_Firmware` solo va lo que se carga, y que la guia viaja dentro.

> 🔴 **Sigue siendo un ENCARGO, no una entrega de version** (skill `entregar` §1): una entrega solo
> sale con **banco pasado**, y el banco sigue abierto. **Pide medidas; no autoriza a instalar nada
> en la calle.**

**Lo entregado el 02/09**, que es contra lo que el banco corrio:
`Paquete_Revision_V9.0_2026-09-02_617bd00_SIN_BANCO.zip` con su APK `617bd00`.

---

### 0.bis · La V2 — lo que ya esta identificado y NO se ha hecho

**No es una lista de deseos: cada linea salio de un defecto medido en esta sesion o de una pregunta
del responsable.** Se agrupa por lo que compra, no por lo que cuesta.

#### A · Que el equipo cuente lo que le pasa (lo que hoy NO existe)

> El sintoma que se sufre en la calle es *«no saber cuanto se va cuando se va, y por que se va»*. Hoy
> el equipo **no guarda nada**: cuando el tecnico llega, lo que paso ya no esta.

| | que | por que |
|---|---|---|
| **A1** | **Pantalla de modo depuracion en la app** — las tramas **en crudo**, las rechazadas **con su motivo**, y los contadores de la ultima ventana | hoy la app pinta el estado y tira todo lo demas. Un fallo intermitente no se diagnostica con un semaforo dibujado: se diagnostica con la trama que no cuadro |
| **A2** | **Registro descargable** de esas tramas | para que el dato salga del poste. `registro_enlace.js` (N-108) ya lleva la mitad: guarda los cortes del enlace. Falta el **contenido**, no solo el hueco |
| **A3** | **El STM32 cuenta el silencio de `J17`** (N-113) | el ESP32 no puede reportar su propia muerte. El que sobrevive es el STM32 |
| **A4** | **El ESP32 declara su reinicio** al volver del watchdog | un puente que revive en silencio **esconde** el fallo que hay que contar |

#### B · Barreras que hoy no vigila nadie

| | que | estado |
|---|---|---|
| **B1** | **La app no valida el checksum de lo que pinta.** `validarTrama()` existe, esta en 4 copias y **no tiene un solo llamador** | medido, N-110 |
| **B2** | **El teclado del PIN acepta pulsaciones con el modal cerrado**: una barrera cuyo estado se arma sin abrir la barrera | medido, N-110 |
| **B3** | **El PIN es `1234` literal en claro** sobre SPP sin cifrar, y **`FORZAR_ROJO`, `SET_MODO:MENU`, `SET_MODO:ALCANCE` y `AMBAR_EMERGENCIA` no lo piden** | N-109 §5. **No esta clasificado como riesgo de seguridad, y lo es** |

#### C · El instrumental, solo lo que impide medir

| | que | por que ahora |
|---|---|---|
| **C1** | 🔴 **N-112: la compuerta alterna** | **es el aparato de medir, y miente**. Todo lo demas se juzga con el |
| **C2** | **Repasar `OPTIMIZACIONES.md`**: la trazabilidad regla -> codigo -> prueba se levanta buscando `# EJERCE`, y el firmware se movio mucho | una regla que aparece cubierta por un pack que no la ejerce **es peor que una fila vacia**, porque la vacia no miente |

> ⚠️ **Y lo que NO entra en la V2, a proposito:** mas packs, mas actas y mas documentos. La auditoria
> (N-109) ya dijo que ese bucle *«produce la sensacion de progreso sin acercar el unico entregable que
> importa»*. **Nada de A, B ni C sustituye al banco** — A y B se escriben para que la sesion de banco
> devuelva datos en vez de impresiones.

### 0.quater · Lo que cayo el 01/09 — siete encargos en paralelo

**Nada de esto ha visto una tarjeta**, y conviene leerlo con eso delante.

| | que se gano | lo que aparecio al MEDIR, que no estaba en el encargo |
|---|---|---|
| **Verde simultaneo** | un instrumento ejecuta el C++ **real de las dos puntas a la vez** (`Validacion_Automatico/dos_puntas`). `42/42`, verde de los dos en **0 de 53.236 instantes** | el choque de simbolos se resuelve con **una DLL por punta en el mismo proceso**: un tick pone el mismo `millis()` en las dos y **solo entonces** lee los doce pines. Y un microcorte es descargar y recargar la DLL, asi que vuelven al arranque **todas** las estaticas — lo que un `reset()` escrito a mano no garantiza |
| **App: depuracion + checksum** | pestana aparte con las tramas en crudo y las rechazadas **con su motivo**; DOM `77 -> 120`, unitarios `32 -> 55` | 🔴 **52 tramas que se pintaban ahora se rechazan**, y **50 de ellas marcaban el enlace como VIVO**: la app decia que habia equipo al otro lado **porque le llegaba basura** |
| **Reloj `DS3231`** | tres puertas nuevas cerradas | 🔴 **el bit 12/24**: un modulo en 12 h con el oscilador sano devuelve numeros bien formados con **hasta DOCE HORAS de error**, y el `OSF` a cero con razon |
| **Watchdog del ESP32** | el puente **declara por que arranco** y cuantas veces lleva | `RTC_DATA_ATTR` promete sobrevivir a un *deep sleep* y **no menciona reinicio**; `RTC_NOINIT_ATTR` si. Verificado **sobre el binario** con `nm`, no leyendo el header |
| **Silencio de `J17`** | el STM32 cuenta los silencios del puerto | 🔴 **por `J17` solo entra lo que un dedo pulsa en el telefono**: `enlace_escribirLinea()` tiene un solo llamador. *«El puente no dice nada»* y *«el puente no esta»* **siguen sin distinguirse** hasta que el ESP32 emita un latido propio (`AB-1`) |
| **`SFTY-27`** | referencias corregidas | 🔴 **`SFTY-3` y `SFTY-7` estaban INTERCAMBIADAS** en las tres puntas, y **la tabla de trazabilidad heredo el error** — se levanta del label, asi que un label malo se lee como medida |
| **N-112** | — | **sin cerrar**. Es el paso 1 |

**Y tres huecos del propio banco que se cerraron de paso**, todos de la misma familia —el instrumento que no puede ver lo que vino a vigilar—:

- `documentos_03` comparaba **solo `app.js`** entre las copias. Con `js/depuracion.js` nuevo, quitarle el `<script>` **revienta la app al cargar** y ningun pack lo veia. Ahora **censa lo que `index.html` carga**.
- El mismo pack comparaba `android/.../public`, que **no esta en git** —lo genera el build—: en un clon limpio habria dado **FALLA por algo que no es un defecto**, y un falso rojo ensena a ignorar el pack.
- `esp32_05` dejo el literal nuevo del watchdog en `9/10` hasta que **un humano lo mirara**. Eso no es un fallo: es la lista blanca escrita a mano haciendo su trabajo.

> ⚠️ **Lo que NO se puede concluir de esta tabla.** Son siete instrumentos y dos defectos de
> firmware reales; el resto es **cobertura**. La compuerta sigue alternando, asi que su codigo de
> salida no acredita nada, y **en campo sigue corriendo `e303485`** con el arreglo del ambar escrito
> desde el 27/08 y sin subir. Mas medida no es mas entregado.

### 0.quinquies · 🔴 `AB-9` — El PIN no caduca NUNCA. Decision del responsable

**Medido el 01/09:** `state.pinVerificado` se pone a `true` en **una linea** (`app.js:2549`) y **no se
apaga en ninguna**. Ni al cerrar el modal, ni al cambiar de punta, ni al caerse el enlace, ni con el
tiempo. Dura lo que dure el proceso del navegador.

> **El operario teclea la clave, se guarda el telefono, y el siguiente que lo coja manda ordenes sin
> teclear nada.**

La demostracion de que no caduca esta en el propio arnes: para probar una sesion sin autorizar **hay
que montar un navegador nuevo**.

**Las opciones, con su coste. No se elige aqui porque decide quien puede parar un cruce:**

| | criterio | coste operativo | cubre el telefono olvidado |
|---|---|---|---|
| **A** | **Inactividad** — sin orden enviada durante X | el tecnico que mira telemetria 20 min re-teclea al actuar | si |
| **B** | **Sesion absoluta** — X desde el desbloqueo | **corta faenas largas en mitad de un cruce parado** | si |
| **C** | **Al perder el enlace** | reconectar con radio floja re-pide clave cada vez | **parcial**: el telefono guardado con enlace vivo sigue autorizado |
| **D** | **Al pasar la app a segundo plano** | ninguno perceptible; **guardarse el telefono = bloquear** | si, y es el que mas se parece al gesto real |
| **E** | Boton *«bloquear»* explicito | depende de que alguien lo pulse | **no** |

**Aporte tecnico, sin elegir:** **A y D son complementarias y baratas**; **C es la que mas friccion
crea por menos cobertura**. Y sea cual sea la que se elija, **ya funcionara**: al vaciar el buffer del
teclado en el cierre se quito el residuo que habria convertido cualquier caducidad en un adorno —un
`OK` suelto la habria re-armado con el PIN bueno todavia en memoria—.

**Relacionado y tambien abierto:** `state.correctPin = '1234'` **sigue en el fuente de la app, en
claro**. La app conoce el PIN y lo inyecta en cada trama. Va con `B3` de la V2.

### 0.ter · Como se reparte el trabajo entre agentes, y por que asi

> **El cuello de botella NO es cuantos agentes se coordinan: es que dos sobre el mismo fichero se
> pisan sin avisar.** El limite real es **cuantos ficheros disjuntos quedan por repartir.**

Costo una historia mintiendo (27/08): `ff6bd19 fix(N-71): el techo de silencio...` contiene **un solo
fichero, el acta**, y todo el firmware de N-71 acabo dentro de otro commit que habla de otra cosa. El
contenido era correcto y la compuerta verde; **lo roto era la historia**, y un `revert` de aquel commit
no deshace nada.

**Las reglas, que no son negociables:**

| | |
|---|---|
| **Un agente = sus ficheros en exclusiva**, listados dentro del encargo | si necesita algo fuera, **para y lo dice**; no lo coge |
| **Ningun agente hace `git add` ni commit** | comiteo yo, **por rutas explicitas**. Nunca `git add -A`, y nunca un directorio que pueda contener artefactos de compilacion — eso ya metio 408 KB de `.exe` y `.o` dos veces |
| **Se revisa por el DIFF, no por su informe** | un agente entrego *«31/31 verificado»* habiendo cambiado a la vez el firmware **y las dos copias del modelo que debian vigilarlo**. Las tres decian lo mismo, asi que el banco no podia verlo |
| **`compuerta.py` no lo toca nadie** | lo quieren todos y es donde chocarian. Los agentes crean packs con **nombre nuevo** y dicen cual conectar |
| **Cada agente tiene que VER CAER su instrumento** | defecto inyectado en el `.cpp` real, la cuenta baja, se restaura, y se verifica con `git diff` **vacio** — no con la impresion de haberlo restaurado |
| **Si cambia una decision a mitad, se redirige al agente vivo** | funciona, y evita tirar trabajo hecho |

**Y el reparto se hace por FICHEROS, no por temas.** Dos encargos que suenan distintos —"el registro del
enlace" y "la seguridad del PIN"— caen los dos en `bluetooth.cpp` y **no pueden ir a la vez**. Elegir
sospechoso por el titulo en vez de por los ficheros ya mando una vez a acusar al commit equivocado.

### 0.1 · Que se entrego el 31/08, y con que salvedad

```
Paquete_Revision_V9.0_2026-08-31_59c5263_SIN_BANCO.zip
365 ficheros | 6,95 MB | md5 c84a5d58366afda94883cfd8d2e77e8c
APK  IOT_VIAL_Semaforos_2026-08-31_59c5263_SIN_BANCO.apk   recompilada, al dia
acta 2026-08-31_compuerta.txt   ->  de fa66710, ARBOL SUCIO, NO corresponde
```

Los cuatro firmwares compilan: **Maestro 88,8 % · Esclavo 65,7 % · Repetidor 20,6 % · ESP32 35,6 %**.
El `04_App/` del paquete lleva **solo la APK** desde el 31/08 — quien lo recibe la instala, no la compila.

### 0.2 · Lo que de verdad mueve el proyecto

> **Tres cables, un cargador USB y la guia impresa.** La placa portadora bloquea **desplegar**, no
> **probar**. El montaje esta paso a paso en el apartado 04 de la guia.

Y el funcional **tiene el banco**: conecta el ESP32, las camaras y las talanqueras, instala la app,
**simula las entradas** —el mando se ejerce puenteando `J16` p5/p8 contra masa, sin el receptor que
nunca se compro— y **mide las salidas**.

Contesta cinco cosas que ningun PC puede, y **dos dan miedo**:

- 🔴 **si el Modo Degradado se puede entrar siquiera** — toda su fase sale de `reloj_segundosDelDia()`,
  y el cristal `Y2` no oscila en las tarjetas reales (N-17 / N-37)
- 🔴 **si las dos puntas pueden dar verde a la vez** — hoy lo sostiene **una copia del firmware escrita
  a mano en Python**; ningun instrumento ejecuta el C++ real de las dos puntas a la vez
- y de paso valida **los 25 s** que arreglan el *«se va a ambar por nada»* que se sufre hoy en la calle

### 0.3 · Lo que espera decision del responsable

| | |
|---|---|
| 🔴 **Quien disena y quien fabrica** la placa portadora | bloquea todo lo demas de la placa |
| 🔴 **Pedir la fuente `A5`** — conmutada 12->5 V, >= 1 A | sin ella no hay montaje permanente |
| 🟠 **Los codigos de los mandos** | bloquean comprar los receptores `A9` |
| 🟠 **`SFTY-27` designa DOS reglas distintas** | y ocho documentos mandan a leer la equivocada |
| 🟠 **Contar los pines del ESP32** (30 o 38) y su ancho | bloquea el taladro, **no** el firmware. La guia ya lo pide en el paso 1 |
| 🟠 **Si el reloj se va con el ESP32, el Modo Degradado no entra** (N-113) | hay que elegirlo a proposito. Y si se quiere aviso remoto, es **coste recurrente** —SIM o WiFi en el cruce—, no una linea de firmware |

### 0.4 · Y la frase que no conviene olvidar

> *"El banco lleva siendo EL bloqueante desde el 31/07 sin moverse, y eso ha dejado de ser un bloqueo
> para convertirse en una condicion permanente alrededor de la cual se ha construido una industria de
> sustitucion."*

**N-42 —que el Modo Automatico no mueve las luces en banco— sigue sin tocarse**, y es el bloque 1 del
encargo de banco. Ver **N-109** entero.

---

## 1. Que hay hoy

| | |
|---|---|
| **En campo** | **V8.4**, commit `e303485` (31/07/2026), validada por el funcional |
| **En el repositorio** | **V9.0** — implementada y compilando. **NO probada en banco** |
| **Compuerta** | ✅ 15 PASS · 0 FALLA · 0 ABORTADO — acta `evidencia/2026-08-31_compuerta.txt` |
| **Flash** | Maestro **88,3 %** (57.880 de 65.536 B, **7.656 B libres**) · Esclavo 64,4 % · Repetidor 20,6 % |
| **Banco** | 445/445 en 39 packs · 271/271 pantalla · 71/71 automatico · 29/29 ciclo · app 32/32 + 61/61 + 58/58 |

> 🛑 **Verde no es entregable.** Ese `0` significa que los modelos y los arneses de PC no encuentran
> nada. **Ninguno toca la tarjeta.** Nada sube a campo sin pasar banco.

> 🔴 **Regresion abierta:** el Modo Automatico no mueve las luces en banco. Es anterior a esta
> arquitectura y no se cierra con ella.

---

## 2. La arquitectura vigente, y POR QUE es esta

**La razon de fondo no es de firmware: este PCB no permite ampliacion.** A diferencia del proyecto
anterior, lo que se desarrollo para ampliarlo **no era fisicamente realizable** —exigia soldar sobre
una placa que no lo admite bien—. No habia de donde sacar pines, y la unica fuente disponible era
retirar funciones. Todo lo que sigue cuelga de ahi.

```
                       fuente propia 12 V (NO sale de la tarjeta)
                                    |
   +--------------------+     +-----v--------------------+
   |   STM32F103C8      |     |         ESP32            |
   |   (controlador)    |     |   (modulo de expansion)  |
   |                    |     |                          |
   |  6 luces  J3-J8    |     |  DS3231  GPIO21 SDA      |
   |  barrera  J15  PB2 |     |          GPIO22 SCL      |
   |  camaras  J16      |     |          (pila propia)   |
   |  LoRa     J12      |     |                          |
   |  mando A/B J16     |     |  Bluetooth (sustituye    |
   |                    |     |   al modulo SPP)         |
   |  PB6 TX == J17 p3 <------ GPIO16 (RX2)              |
   |  PB7 RX == J17 p2 ------> GPIO17 (TX2)              |
   +---------|----------+     +-----------|--------------+
             +-------- masa comun --------+
                       9600 8N1
```

**El STM32 manda sobre las luces; el ESP32 no.** La barrera de salidas de `CLAUDE.md` §6 no cambia.

### El mapa de pines, MEDIDO contra `pines.h` y contra `src/`

| pin | funcion | bornera | estado |
|---|---|---|---|
| `PA0`-`PA5` | 6 luces | `J3`-`J8` | vivo |
| `PA6` `PA7` | peatonal rojo/verde | `J11` `J9` | 🔴 **declarado y MUERTO** en las dos puntas |
| `PB1` | buzzer | `J13` | 🔴 **declarado y MUERTO** en las dos puntas |
| `PB2` | barrera | `J15` | vivo |
| `PB0` | camara de demanda | `J14` | vivo *(solo dentro del Modo Inteligente en el Maestro)* |
| `PB8` | `LED_TESTIGO` | LED `D5` | `INPUT` sin lectura — **deliberado y documentado** |
| `PB9` | **MANDO A** (`BOTON1`) | `J16` p5 | ✅ **SE QUEDA** |
| `PB13` | **MANDO B** (`BOTON2`) | `J16` p8 | ✅ **SE QUEDA** — arma el veto de SFTY-21 |
| `PB14` | `BOTON3` hoy | `J16` p10 | → **camara**, tras retirar C |
| `PB15` | `BOTON4` hoy | `J16` p12 | → **camara**, tras retirar D |
| `PB6` `PB7` | enlace ESP32 | `J17` p3/p2 | vivo — `SerialBT(PB7, PB6)` |
| `PB3` `PB4` `PB5` | SPI de la pantalla | — | **quedan LIBRES al retirar la LCD** |
| `PB10`-`PB12` | radio LoRa | `J12` | vivo |
| `PA8` | desacoplo `U2` | — | vivo, con justificacion caducada (N-95) |
| `PA9` `PA10` | `RS485_IN` | — | declarados y muertos desde N-76 |

**El reloj no cuesta ni un pin del STM32:** el `DS3231` cuelga del ESP32.

---

## 3. Lo decidido, con fecha

| decision | cuando | consecuencia |
|---|---|---|
| **El ESP32 sustituye al modulo SPP dedicado** y se lleva ademas el reloj | 28/08 | ya no se compran `HC-05`/`JDY-30` |
| **Se retira la pantalla LCD** de las dos puntas | 28/08 | libera `PB6`/`PB7` para el Bluetooth y `PB3`/`PB4`/`PB5` de margen, mas ~18,9 KB de flash |
| **El `DS3231` sale del STM32** y cuelga del ESP32 | 28/08 | la linea `PIN-0` queda ANULADA: el I2C ya no vive en el STM32 |
| **El mando de reles se CONSERVA en A y B**; se retiran C y D | **31/08** | `A·A·A`, `B·B·B` y `A·B·A·B` sobreviven, el veto de SFTY-21 no desaparece, y `PB14`/`PB15` quedan para las camaras — **ver N-104** |
| **El modulo es un `ESP32-WROOM-32` clasico: hay SPP** | **31/08** | la app conecta sin tocar el transporte; el apartado 1 del Manual 10 queda intacto; la alimentacion es `12 V -> DC-DC conmutado -> 5 V -> VIN` |
| **Las camaras entran por `J16` p10 y p12** | **31/08** | se leen por el camino de camara (`INPUT` + activo en ALTO), no por el de boton |

---

## 4. Lo que esta ABIERTO, y de quien es

### Del responsable — no se destraban con mas analisis

| # | que | como se cierra |
|---|---|---|
| ~~**BLQ-1**~~ | 🟢 **CERRADO el 31/08.** Es un **`ESP32-WROOM-32` clasico**: `Xtensa LX6 dual-core` y `Bluetooth v4.2 **BR/EDR** + BLE` — hay **SPP**. La app conecta sin tocar el transporte y el apartado 1 del Manual 10 **no se reabre**. Ver **N-107** | — |
| ~~**M3**~~ | 🟢 **CERRADA EN BANCO el 03/09, paso 20.** El pull-down es **real y de 10 kOhm**: `p10` mide **9,93 kOhm** a masa y `p12` **9,94 kOhm**, los dos a **0 V** con energia. El camino de camara —`INPUT` pelado, activo en ALTO— es correcto y **no hay demandas fantasma**: el paso 21 lo confirmo con y sin el cable puesto. La misma medida condena el mando: ver **N-118** | — |
| **A5** | 🔴 **La fuente propia del ESP32 desde 12 V.** No esta pedida y hace falta | comprarla |
| **N75-1** | 🟠 El minimo de tiempo por sentido | es una cifra, y hace falta |
| ~~**APK**~~ | 🟢 **CERRADA el 31/08.** Recompilada contra el fuente al dia: `IOT_VIAL_Semaforos_2026-08-31_59c5263_SIN_BANCO.apk`, 3.908.591 B. El paquete ya no aborta | |

### Tecnico — se puede hacer ya

| # | que | notas |
|---|---|---|
| **T1** | 🔴 **Los documentos peligrosos**: Manual 11 (manda cablear I2C contra la camara y un LED), Manual 10 (manda el modulo equivocado) y el HTML de cableado (viaja en el paquete de entrega) | **daño fisico y dinero.** Va primero |
| **T2** | 🔴 **Blindar los instrumentos antes de tocar firmware**: la etiqueta `# EJERCE` que falta, el `TOTAL_PACKS` que no sabe fallar, el pack del transporte, y el rol del ESP32 en la guarda de rutas | **la compuerta ve lo que se BORRA y no lo que se queda sin sujeto** (N-103) |
| **T3** | 🔴 **Los cinco `MEDIDO` caducados** | hacen reimplementar trabajo ya hecho (N-100) |
| **T4** | 🟠 **Firmware del ESP32**: watchdog primero, luego `DS3231`. No dependen de BLQ-1 | el watchdog con su desigualdad en un pack: periodo **<** `SFTY6_SILENCIO_MS = 25000UL` |
| **T5** | 🟠 **Fases 2 y 3** del firmware STM32 | 🔴 `compilar.ps1` y los stubs de `Validacion_Automatico` **en el mismo commit** que toque `mando.cpp` (N-101) |
| **T7** | 🟠 **Las dos barreras de la app de N-110**: validar el checksum de la telemetria (el llamador ya existe y esta sin usar) y que el teclado del PIN no acepte pulsaciones con el modal cerrado | tocar `app.js` obliga a **recompilar la APK y rehacer el paquete**: van juntas, no sueltas |
| **T8** | 🟢 **N-117 ARREGLADO el 04/09**: el perro del ESP32 ya no muerde su propio arranque | queda **confirmarlo en el modulo** con el monitor serie. El arreglo esta en el arbol; lo que falta no es codigo |
| **T6** | 🟠 **BANCO — corrio el 03-04/09.** Ya no es EL bloqueante entero: quedan **5 pasos de 29** | los desatasca el ESP32 (4) y la reparacion del Maestro (1). **Sigue siendo cierto que nada de lo escrito lo sustituye** |

---

## 5. Donde vamos — 04/09, con el banco corrido

**Rama `main-nuevo`. `origin/main` intacto en `f25fa57`.**

> 🟢 **El eje del proyecto cambio el 03/09.** Durante 34 dias la pregunta fue *«como llegamos al
> banco»*. Ya se llego: **24/29**. La pregunta de hoy es **«como se destraban los 5 que faltan»**, y
> los cinco tienen nombre — 4 el ESP32 (**N-117**), 1 la tarjeta Maestro (**N-116**). Ninguno se
> destraba escribiendo.

### 🔴 Lo que la auditoria externa dejo claro (N-109)

> *El banco lleva siendo EL bloqueante desde el 31/07 sin moverse, y eso ha dejado de ser un bloqueo
> para convertirse en una condicion permanente alrededor de la cual se ha construido una industria de
> sustitucion.*

**Y lo que lo desatasca no es otro pack: son TRES CABLES Y UN USB.** La placa portadora bloquea
**desplegar**, no **probar**. Ese montaje ya esta escrito paso a paso en la guia de cableado
(apartado 04, pasos 9 a 14) y **se puede hacer hoy, sin comprar nada**.

### Cerrado hoy

| | |
|---|---|
| **Firmware** | camaras en C y D (N-97 cerrado) · **el firmware del ESP32, que no existia** · la app: tablero, reloj y cinco exitos mudos · la pantalla deja de conducir `PB3/PB4/PB5` |
| **Instrumentos** | `enlace_01` transporte · 9 packs `esp32_*` · 5 packs `app_*` · `costura_11` · **el simulador de la interaccion con las dos puntas REALES** · la tautologia de `arnes_ciclo` · el `TOTAL_PACKS` que no sabia fallar |
| **Documentos** | roadmap · README · `.map` · **`INDICE_CRUZADO.md`** · **la especificacion de la placa portadora** · manuales 1, 2, 3, 5, 9, 10, 11, 12, 14, 15, 17, 18 · `MANUAL_MANDO`, `MANUAL_CONFIGURACION_BLUETOOTH` · `CERTIFICACION_SW` · `OPTIMIZACIONES` · **el encargo de banco** |
| **La guia de campo** | reescrita **para el tecnico**: 29 pasos `HAZ / COMPRUEBA / TIENES QUE VER / ANOTA`, con el montaje de mesa dentro |
| **Decisiones** | BLQ-1 cerrado · el mando se conserva en A y B · la pantalla no se retira · R-1 a R-4 del ambar · el `sscanf` se arregla |

### Defectos ABIERTOS, con instrumento que los ve

| | que | estado |
|---|---|---|
| 🛑 **N-116** | **la tarjeta Maestro se calienta y se para a los ~30 s** | **fuera de servicio.** El firmware queda descartado por censo; es hardware |
| 🟢 **N-125** | **la app no pedia el permiso de Bluetooth de Android** | ✅ **CERRADO EN CAMPO el 04/09 a las 13:32**: *«ya funciona la app»*. Era la causa real del *«el escaneo fallo»* del banco |
| 🟠 **N-117** | el ESP32 no se anuncia por Bluetooth de forma fiable | **arreglado en el arbol el 04/09**, y ahora **probablemente NO era la causa** — ver el aviso de abajo |
| 🟠 **N-122 / N-124** | la app no abria el socket, y marcaba dos MAC escritos a mano | arregladas y en la APK. **Ejercidas en campo solo hasta donde llego la observacion**: ver el aviso de abajo |
| 🔴 **N-118** | **el mando A/B no puede pulsarse**: SFTY-21 no tiene respaldo fisico | medido en cobre y en el fuente. **Espera decision de polaridad** |
| ~~🟢 **N-106**~~ | el ambar de la app no saca al Esclavo del Degradado | ✅ **CERRADO, y esta fila llevaba dias mintiendo.** Lo cerro `2e99bc3` con el molde de cuatro filas de `Esclavo/src/bluetooth.cpp`, y `esclavo_08_ambar_en_degradado` lo vigila **8/8 con cinco controles negativos**. Se descubrio auditando, no trabajando: ver **N-121** |
| 🔴 **el `` cruzado** | 32 de 289 pares confirman OTRA orden | en curso |
| 🔴 **N-42** | el Modo Automatico no mueve las luces en banco | **el banco del 03/09 NO la confirmo ni la descarto**: el equipo nunca llego a Modo Automatico porque falto la app. Se decide repitiendo el paso 7 |
| 🔴 **el verde simultaneo** | lo sostiene un modelo de Python, no el codigo | **solo se cierra en banco**, y el banco no llego a ejercerlo |
| 🟠 **el PIN 1234 en claro** | y las ordenes mas peligrosas no lo piden | sin elevar a riesgo de seguridad. **El banco aclaro un punto de proceso**: el ESP32 empareja por *Just Works*, asi que el 1234 es PIN de comando de la app, **no** de emparejamiento |

### Lo que falta para cerrar — al dia el 04/09

| | | estado |
|---|---|---|
| 1 | Sincronizar `www/` y los assets de Android, y **recompilar la APK** | 🟢 **HECHO el 04/09.** Las TRES copias identicas —`app.js`, `www/app.js` y `android/.../assets/public/app.js`, que es de donde se construye la APK— y la APK recompilada con JDK 17 y verificada por CRC |
| 2 | **Compuerta completa, DOS pasadas**, y acta sobre arbol limpio | 🟢 **HECHO.** `20/20`, exit 0, `HEAD edf4783`, arbol LIMPIO |
| 3 | **Cuadrar las cifras** en README, `ESTADO.md`, `CERTIFICACION_SW.md` y este roadmap | 🟢 **HECHO.** Las tres citaban el acta del 02/09 con la del 04/09 escrita; lo cazaron `documentos_01` y `documentos_04` |
| 4 | La contradiccion de la guia: *«los cuatro pines tienen que dar lo mismo»* contra el reparto del 31/08 | 🟠 **abierta, y ahora medida.** El cobre dice que los **cuatro** son identicos —10K a masa y 3,3 V al lado—, o sea que la guia tenia razon y el reparto no: **los cuatro son activos en ALTO**. Es N-118, y lo que falta es la decision de compra, no la medida |
| 5 | 🛑 **BANCO: los 5 pasos que faltan** | **es lo unico que queda de verdad.** 4 los abren el ESP32 (N-117) y la APK nueva (N-122); 1, la reparacion del Maestro (N-116) |
| 6 | 🟢 **Los manuales, HECHO el 04/09.** Nueve reescritos por cinco agentes en ficheros disjuntos, mas la guia | y despues **auditados en cruce**, que es lo que ninguno de los cinco podia hacer: salieron **7 fallos, 3 graves** —una APK que no existe, un hash inventado, y que ninguno sabia que el mando ya estaba arreglado, con la guia mandando simularlo **con un cable a masa**—. Corregidos los siete |

---

## 5.bis El orden de arranque — que se lanza, cuando, y que abre cada puerta

**La regla que fija el orden, y no es de gusto:** hoy la compuerta **ve lo que se BORRA y no ve lo
que se queda sin sujeto** (N-103). Si el firmware se mueve antes que los instrumentos, hay una
ventana en la que decenas de comprobaciones dan verde midiendo codigo que ya no corre. Por eso los
instrumentos van **antes**, no despues.

### Ola A — lo que puede hacer daño hoy 🔴

Va primero porque **no depende de nada** y es lo unico con daño fisico o dinero detras.

| | que | por que aqui |
|---|---|---|
| **A1** | Los cuatro documentos de **N-105**: `MANUAL_USUARIO.md:66-70`, `04_Manuales/MANUAL_INSTALACION_RELOJ_DS3231.md`, `MANUAL_HARDWARE.md:63,66`, `9_Manual_Parametrizacion_Camara_IA.md:64,168` | mandan cablear camaras sobre `PB9`/`PB13` —el mando— y el I2C sobre la entrada de camara y un LED |
| **A2** | El **Manual 17** y **`ESTADO.md`**: llevan la decision anterior *(«se retiran los cuatro pulsadores»)*, y `17_:152-153` manda dejar `p5`/`p8` vacios, que es justo el mando que se conserva | `ESTADO.md:104` ejecutado literal **borra `ambarLocal` y el veto de SFTY-21** |
| **A3** | Los documentos que dan **`FORZAR_ROJO` por valido en el Esclavo** | es el boton de panico: el operario cree que paro el trafico y no paro nada |
| **A4** | Las **cifras sin vigilante**: `MANUAL_USUARIO.md:21` publica despeje de *5 a 999 s* cuando son `DESPEJE_SEG_MIN=10, MAX=90` en un `uint8_t` — 999 nunca fue representable. Y `CERTIFICACION_SW.md` publica 65,0 % de flash cuando son 88,3 % | quien planifique cree tener 23 KB y quedan 7.656 B |

### Ola B — la red, antes de tocar firmware 🔴

| | que | por que aqui |
|---|---|---|
| **B1** | El **`TOTAL_PACKS`** de `documentos_01`: se comprueba como numero suelto y **casa por accidente con el hash `50a5380` del README** | esta demostrado que da falso verde. Se arregla anclandolo a la frase, y se ve caer |
| **B2** | El **rol del ESP32** en `_ROLES` de `compuerta.py`, y su compilacion | sin el, el fuente del ESP32 es **invisible** para la guarda y el acta no tiene una fila donde echarlo de menos |
| **B3** | El **pack de N-106**: que el ambar de la app saque al Esclavo del Degradado | tiene que **fallar** sobre el firmware de hoy antes de que nadie lo arregle |
| **B4** | Un **`documentos_04`** que vigile los manuales que hoy no parsea nadie | `documentos_01` solo mira `README.md` y `ESTADO.md`, y ahi no estan las cifras malas |

### Ola C — firmware

| | que | notas |
|---|---|---|
| **C1** | **Camaras en C y D, LAS DOS PUNTAS EN UN SOLO AGENTE** | dos agentes en paralelo sobre la misma regla es como divergen: es SFTY-2 con el `amarillo = false` de mas, y es N-97. Cierra N-97 de paso unificando como se lee la camara |
| **C2** | **N-106**: la llamada que falta en `Esclavo/src/bluetooth.cpp` | despues de B3, no antes |
| **C3** | **ESP32: watchdog primero, luego `DS3231`** | no dependen de BLQ-1. El watchdog con su desigualdad en un pack: periodo **<** `SFTY6_SILENCIO_MS = 25000UL`, recalculada del C++ |

### Ola D — la pantalla

Sola, con el arbol quieto. Toca `lcd.cpp`, `menu.cpp`, `Validacion_LCD` (271 comprobaciones) y tres
packs, y cada prueba afectada va a **se borra / se invierte / se conserva**, una por una y anotada.
Libera ~18,9 KB y `PB3`/`PB4`/`PB5`.

### Ola E — BANCO

🟢 **CORRIO EL 03-04/09.** 24/29 pasos verificados en hardware. Lo que sigue abierto son **5 pasos**,
y ninguno se destraba escribiendo: **4 los abre el ESP32** (N-117) y **1 la reparacion del Maestro**
(N-116).

**La frase de este apartado se mantiene entera, y ahora con prueba:** ni la compuerta en verde, ni
los arneses que compilan C++ real, ni este roadmap vieron venir ninguno de los tres hallazgos del
banco. **Los tres pasaron el 20/20 sin despeinarlo.**

### Lo que NO desbloquea ningun agente

| | quien | coste |
|---|---|---|
| ~~**BLQ-1** la serigrafia del ESP32~~ | ✅ **cerrado el 31/08** | era `WROOM-32` clasico. Queda una pregunta mucho menor: **30 o 38 pines** de la NodeMCU, para las hembrillas de la placa — pie de rey, y no bloquea firmware |
| **M3** el pull-down real de `PB14`/`PB15` en cobre | funcional | multimetro. Decide **como se configura la camara** y en que pin va cada una: `p10` tiene 4,27 mm contra los 12 V y `p12` solo **1,36 mm** |
| **A5** la fuente propia del ESP32 | responsable | no esta pedida |
| el **receptor del mando** | responsable | nunca se compro: hoy hay firmware y veto, no equipo |
| **recompilar la APK** | responsable | el paquete de entrega aborta con exit 2 |

---

## 6. Los hallazgos de esta sesion — el porque de todo lo de arriba


### 🟢 N-115 — El banco corrio, y lo primero que hay que decir es que NO invalido nada de lo escrito

**03-04/09, informe en `evidencia/Informe_Pruebas_Banco_Semaforos_V9.0.pdf`.** 24 de 29 pasos
completos sobre `617bd00`, con dos tarjetas cargadas por SWD al primer intento y sin BOOT0.

**Lo que conviene registrar con cuidado, porque es la respuesta a tres auditorias:** el banco **no
encontro ni un defecto de logica**. El ciclo, la radio, la caida a ambar, la talanquera, la camara de
J14 y el enclavamiento se comportaron como los modelos decian. Los 34.532 renglones de instrumento
**acertaron en todo lo que sabian mirar**.

Y aun asi el banco paro. Los tres hallazgos que lo pararon comparten una propiedad:

| hallazgo | por que ningun instrumento podia verlo |
|---|---|
| **N-116** el chip se calienta | es corriente y temperatura, no es una propiedad del fuente |
| **N-117** el modulo no se anuncia | es **cuanto TARDA** un arranque, y eso no se lee del C++ |
| **N-118** los 10 kOhm del cobre | esta en la placa, no en el repositorio |

> **Eso no es una absolucion del §2.bis: es su enunciado exacto.** La critica nunca fue *«los
> instrumentos estan mal»* —estan bien—, sino que **certifican otra vez lo ya certificado mientras
> nadie mide lo que solo se mide con la tarjeta en la mano**. El banco acaba de mostrar cual era la
> mitad que faltaba, y no era Python.

**Correccion de una cifra de este roadmap, para que no se arrastre:** aqui se ha repetido 34 dias que
el montaje de mesa *«se puede hacer HOY, sin comprar nada»*. Era cierto, y cuando se hizo salieron
**dos problemas de hardware que ningun analisis de escritorio habria encontrado**. La conclusion no
es que el analisis sobrara: es que **el orden estaba invertido desde el 31/07**.

**Y un aviso de proceso que el propio informe merece:** esta redactado con las tres categorias
separadas —*completo*, *bloqueado*, *abortado*— y **no cuenta un `BLOQUEADO` como aprobado en ninguna
linea**. Es §2 de `CLAUDE.md` aplicado por alguien que no lo ha leido, lo cual dice que la distincion
es natural y no una mania de este repositorio.


### 🛑 N-116 — El Maestro se calienta a los ~30 s: el firmware queda DESCARTADO por censo, no por opinion

**Sintoma, del funcional el 04/09:** *«al iniciarse o alimentar la placa funciona adecuadamente
durante aproximadamente 30 segundos, se calienta de mas el microcontrolador y deja de funcionar»*.
Aparecio durante el **paso 29**, puenteando `J16` p5/p8 contra masa, y **ahora se repite sin puente**.

#### Lo que esta MEDIDO: el firmware no puede ser la fuente del calor

Censo de **todas** las salidas del Maestro —`grep` de `pinMode(..., OUTPUT)` sobre `Maestro/src/`,
no lectura—:

```
semaforo.cpp:193-198   ROJO1/2, AMARILLO1/2, VERDE1/2
semaforo.cpp:203       MOTOR_TALANQUERA (PB2)
bluetooth.cpp:133      RS485_IN_DE_RE (PA8)
protocolo.cpp:14       LORA_DE_RE
```

**Nueve salidas en todo el firmware, y ninguna es `PB9`, `PB13`, `PB14` ni `PB15`** — los cuatro
pines de `J16`. Los dos que se puentearon estan en `INPUT_PULLUP` (`botones.cpp:139-140`): contra
masa consumen `3,3 V / 40 kOhm` ~= **80 uA**, o sea **0,27 mW**. Eso no calienta un chip.

> **Consecuencia dura y util: cargar otro firmware no arregla esto.** Es la clase de conclusion que
> ahorra una sesion entera de banco persiguiendo el sitio equivocado.

#### Una causa que se cayo, y se marca refutada en vez de borrarse

Se sospecho **contencion en `PB6`/`PB7`**: el netlist dice que `J17` es el LCD y el firmware lo usa
como UART del ESP32, asi que dos salidas *push-pull* enfrentadas en el mismo hilo explicarian el
calor perfectamente. **Es falsa.** `Maestro/src/lcd.cpp:74-75` construye el U8g2 con los **cuatro
pines en `U8X8_PIN_NONE`**, y `U8x8lib.cpp` pregunta `if (u8x8->pins[i] != U8X8_PIN_NONE)` antes de
cada `pinMode` y cada `digitalWrite`: no queda ni una escritura. La pantalla no conduce nada.

Queda escrita porque **es la sospecha natural** —la contradiccion netlist/fuente esta ahi y volvera a
proponerse—, y porque las medidas del paso 5 la explican mejor sin ningun defecto: `RST` (`PB7`) a
3,3 V es el **TX del ESP32 en reposo**, que es alto; `RS/A0` (`PB6`) variando entre 2,8 y 3,3 V es el
**TX del STM32 transmitiendo**. Todo coherente, cero conflicto.

#### 🔴 UNA SEGUNDA CAUSA REFUTADA, Y ERA LA MIA — la talanquera no puede ser

Se propuso aqui mismo, y hay que tacharla con el mismo rigor con que se escribio. El razonamiento era:
`semaforo.cpp:93` energiza la talanquera cuando `verde || estado == S_FALLO`; un Maestro solo cae a
`S_FALLO` a los **~20 s** —medido en el paso 8— y **en ese instante enciende `J15`**, que es lo unico
que conmuta solo dentro de la ventana de los 30 s. Encajaba en el tiempo.

**Se cae al leer el cobre.** Trazada la cadena entera sobre `Controladora_Semaforos.kicad_pcb`:

```
U1.20 (PB2) -> /Motor -> R70 220R -> U15.1   TLP127  (LED del optoacoplador)
                      -> R69 10K a masa      (pull-down de BOOT1, correcto)
U15.6 -> /5V    U15.4 -> R72 220R -> puerta de Q10 (IRLZ44N), con R71 10K y C30 100nF
Q10.2 (drenador) -> J15.2 + D30 1N4148 al riel de 12V     Q10.3 -> GND
```

**`U15` es un TLP127: aisla galvanicamente las dos mitades.** El STM32 no toca la etapa de potencia
por ningun camino — lo unico que ve desde ese lado es el LED del opto detras de **220 ohmios**, o sea
`(3,3 - 1,2) / 220 = 9,5 mA`, dentro de los 20 mA que el pin admite. **Aunque `J15`, `Q10` y `D30`
ardieran enteros, no hay por donde inyectar corriente al silicio.** La hipotesis era plausible y es
falsa, que es exactamente lo que §4 castiga cuando llega con la palabra *«medido»* encima.

*(De paso, un hallazgo real que salio de mirar ahi y que **no** es la causa de esto:* **`D30` es un
`1N4148`** *—200 mA— haciendo de diodo de rueda libre de una salida de motor gobernada por un*
*`IRLZ44N`. Esta infradimensionado en dos ordenes de magnitud. No mata al STM32, que esta aislado,*
*pero se lleva por delante `D30` y despues `Q10` en cuanto un motor real de pluma haga su retorno*
*inductivo. Va a la V2.)*

#### La causa que SI sostiene el cobre — y es de diseno, no de esta tarjeta

El censo de que hay **entre el borne de campo y el silicio**, leido del `.kicad_pcb`:

```
PB0   pad 18   /Puerta   <- J14.1     serie: NADA
PB9   pad 46   /Boton1   <- J16.5     serie: NADA
PB13  pad 26   /Boton2   <- J16.8     serie: NADA
PB14  pad 27   /Boton3   <- J16.10    serie: NADA
PB15  pad 28   /Boton4   <- J16.12    serie: NADA
```

Contra lo que hace la placa con **todas** sus salidas, sin una sola excepcion:

```
PA0 (/S1) -> R19 220R -> U6  TLP127 -> lado de potencia     ... y asi las nueve
```

> 🔴 **La placa protege cada SALIDA con 220 ohmios en serie y un optoacoplador, y no protege NINGUNA
> entrada de campo.** Los cinco pines que salen a bornera van **desnudos al die**. El `10K` y el
> `100nF` que llevan estan en **paralelo**, no en serie: fijan el reposo, **no limitan corriente**.

Y en ese mismo conector, **`J16.1` es el riel de `/12V` crudo** —el netlist lo confirma: comparte net
con `J15.1`, `J13.1`, `J11.1` y veintitantos mas—. `pines.h:120-121` ya lo tenia escrito: *«12 V
CRUDOS, sin opto, sin serie, sin clamp»*.

**El mecanismo, que es estandar y encaja con todo lo observado:** 12 V tocando cualquiera de esos
cinco pines hace conducir el diodo de sujecion de ESD del STM32 **hacia el riel de 3,3 V**. Sin nada
en serie que limite, la corriente la fija solo la impedancia de la fuente. Eso dispara el **latch-up**
—el tiristor parasito del CMOS— y el chip pasa a consumir corriente sostenida de `VDD` a `VSS`:
**calienta, y sigue calentando hasta que se le quita la alimentacion**. El dano suele ser permanente.

**Y explica los 30 segundos sin necesidad de la talanquera:** una pastilla ya danada arranca, funciona,
y su propia corriente de fuga la calienta; al calentarse la fuga sube, y eso realimenta. La fuga
termica de un encapsulado asi tarda **decenas de segundos** en hacerse notar. Por eso se manifiesta
igual con el conector vacio, que es lo que el funcional describe hoy.

**Lo que esto NO dice:** cual fue el contacto concreto. El informe afirma que el puente solo toco
p5/p8 y masa, y no hay motivo para dudarlo — pero el dano pudo entrar en cualquiera de los pasos 15
a 29, en los que se manipulo `J16` y `J15` repetidamente con los 12 V presentes, o con el conector
volante insertado una posicion corrido. **Eso solo lo dice la inspeccion.**

#### 🟢 MEDIDO EL 04/09: HAY CORTO ENTRE 3,3 V Y GND. La hipotesis deja de serlo

El responsable lo midio en la tarjeta: **corto franco entre el riel de 3,3 V y masa**. Eso era
exactamente la comprobacion que este apartado pedia, y **explica los ~30 s enteros sin necesitar
ninguna otra causa**: el regulador entra en limitacion de corriente, disipa toda la diferencia
`12 V -> 3,3 V` contra el corto, y se va a proteccion termica. Puede calentar `U5` **o** `U1`, y
desde fuera se sienten igual.

#### 🔴 Y EL COBRE DA UN CANDIDATO QUE ENCAJA CON EL GESTO DEL PASO 29

```
J16   3,3 V en pines 4, 7, 9, 11     GND en pin 2
J17   3,3 V en pines 6, 8            GND en pines 7, 9
```

En el paso 29 se estaba puenteando **p5 y p8 contra masa**, y la masa de ese conector es **p2**.
**`p4` es adyacente a `p5`, y `p7` es adyacente a `p8`** — y las dos llevan 3,3 V. Un puente que
resbale **una sola posicion** pone el riel de 3,3 V directamente contra masa. Es el mismo gesto que se
estaba haciendo, corrido un pin.

Y `J17` es peor todavia: **3,3 V en 6 y 8 con masa en 7 y 9**, o sea alternados. Un conector insertado
una posicion corrida cortocircuita el riel sin que nada lo delate — y en el paso 24 se enchufo ahi el
modulo definitivo.

> **Lo que esto ABRE, y es la buena noticia: el STM32 puede estar sano.** Un corto de 3,3 V a masa
> hecho con un puente castiga al **regulador**, que tiene limitacion de corriente y proteccion
> termica. Que la pastilla este muerta es **una** de las salidas, no la unica ni la mas barata. **No
> se da por muerto el micro hasta haber recorrido la escalera de abajo.**

#### La escalera, de lo gratis a lo caro — y no se salta ningun peldano

**1. DESENCHUFAR TODO** —`J14`, `J15`, `J16`, `J17`, `J2`— y volver a medir 3,3 V contra masa.
Los cuatro conectores sacan el riel fuera de la placa. **Si el corto desaparece, la placa esta bien**
y el problema esta en el cableado volante o en el modulo ESP32. Es gratis y puede cerrar el caso.

**2. Si el corto sigue, esta en la placa.** Lo que cuelga del riel, en orden de coste:

| | | por que en este orden |
|---|---|---|
| `C1` `C2` `C3` `C4` `C10` `C11` (100 nF) · `C15` (10 uF) | condensadores de desacoplo | **un ceramico en corto es el fallo mas frecuente y el mas barato.** Se levanta uno y se remide |
| `U5` | LM1117DT-3.3 | es quien mas ha sufrido: el corto lo castiga a el |
| `U2` `U3` (pin 8) | los dos MAX3485 | alimentados del mismo riel |
| `U1` (pines 9, 24, 36, 48) | el STM32 | **el ultimo, no el primero** |

**3. Discriminar SIN desoldar:** inyectar 3,3 V en el riel con **fuente limitada a ~200 mA** y buscar
que componente calienta. Con alcohol isopropilico sobre la zona, el que primero seca es el que
disipa. Es el metodo estandar y no arriesga nada mas.

**4. Inspeccionar `J16` y `J17`** buscando el rastro: en `J16`, p2 contra p4 o p7; en `J17`, cualquier
pareja 6-7 u 8-9.

> 🛑 **Mientras tanto sigue en pie: no reenergizar «a ver si pasa».**

> ⚠️ **Y lo que de verdad importa, porque no se va con la tarjeta rota: esto le va a pasar a la
> siguiente.** Las camaras van a `J16` p10/p12 **en campo, con instaladores**, y el conector lleva 12 V
> en p1 y silicio desnudo en p5, p8, p10 y p12. **Un commit no protege de un destornillador**
> (§9.bis), y aqui tampoco protege un manual. Ver **N-120**.


### 🔴 N-120 — La placa protege todas sus salidas y ninguna de sus entradas. Va a la V2, y antes de cablear camara

Sale del censo de N-116 y merece linea propia porque **no se va con la tarjeta danada**: es de diseno,
esta en las 185 huellas del `.kicad_pcb`, y afecta a todas las unidades.

| | camino | proteccion |
|---|---|---|
| **salidas** (9) | `PBx -> 220R -> TLP127 -> potencia` | serie **y** aislamiento galvanico |
| **entradas de campo** (5) | `bornera -> pin del STM32` | **ninguna** |

Y las entradas son justo las que un instalador toca: `J14` (camara de demanda) y `J16` p10/p12
(camaras C y D), en un conector cuyo **p1 lleva 12 V crudos**.

**La cuenta de lo que costaria cerrarlo, para que se decida con el numero delante.** Una resistencia
en serie por entrada:

```
con 2K2 en serie:   12 V en el pin ->  (12 - 4,0) / 2200  =  3,6 mA   <  los 5 mA que el
                                                                          datasheet admite de
                                                                          inyeccion por pin
y el contacto cerrado sigue leyendose:  3,3 x 10 / (10 + 2,2)  =  2,70 V   >  2,31 V de VIH
```

**2K2 es el punto donde las dos desigualdades se cumplen a la vez** —4K7 ya deja el nivel alto en
2,24 V, por debajo de `VIH`, y dejaria de leer la camara—. **Son cuentas de sobremesa, no una
decision:** quien firme el diseno de la placa las rehace y elige. Lo que no es opinable es que hoy no
hay **nada**.

> 🛑 **Consecuencia inmediata, que no espera a la V2:** mientras las entradas sigan desnudas, cablear
> camara a `J16` es exponer el micro a los 12 V de p1 con la mano de un instalador de por medio. El
> paso 4 de la guia —**tapar fisicamente el pin de 12 V**— deja de ser una precaucion de banco y pasa
> a ser **obligatorio en cada equipo, escrito en la guia de instalacion**. Es lo unico que hay hoy
> entre el instalador y esta averia.


### 🟢 N-127 — AB-1 CONSTRUIDO: el latido del puente, y las dos piezas del STM32 que lo hacian imposible

**Decision del responsable, 04/09.** El ESP32 emite ahora un latido propio para que los tres
contadores de silencio de `J17` **signifiquen algo**. Hasta hoy no servian, y lo decia su propio
comentario: por `J17` solo entra lo que un dedo pulsa en la app, asi que **un puente vivo y uno
muerto eran indistinguibles** desde el STM32.

#### Un primer intento se PARO, y esa parada es la mitad del valor

El agente que lo iba a escribir midio antes y no lo escribio: `j17RegistrarLinea()` vive **despues**
de `procesarComando()`, y el despachador **contesta a todo** —lo que no case cae en
`$ERR,CMD:AUTH_FAILED,DESC:PIN_INVALIDO`—. Un latido a secas habria sido **un aviso rojo cada dos
segundos acusando al operario de una clave que nadie tecleo**: el *falla permanente* de §2, que
enseña a ignorar los rechazos de verdad.

**Y de paso refuto una frase escrita en el propio `bluetooth.cpp`:** *«estos mismos tres numeros
pasan a ser el registro de cortes de verdad SIN TOCAR UNA LINEA de aqui»*. Era falsa —hacen falta
**dos** cambios en las **dos** puntas— y era una cuenta hecha dentro de un comentario, con la
autoridad de un dato.

#### Las tres piezas, y ninguna sirve sin las otras dos

| | |
|---|---|
| **ESP32** | `LATIDO_LINEA = "$LATIDO"`, `LATIDO_MS = 2000`. El literal empieza por `$` a proposito: las ordenes son `CMD:...`, y lo que empieza por `$` son las tramas que el **equipo emite**. No hay ninguna orden a un byte de distancia. Vive en `vigilante.cpp` porque `puente.cpp` y `enlace_stm32.cpp` son el camino de datos y `P-1/P-4` les prohiben tener reloj |
| **STM32, las dos puntas** | la **linea reservada**, la primera de todas en `procesarComando()`: devuelve **sin actuar y SIN CONTESTAR**. No rompe 6.4 —la regla prohibe originar **ordenes**, y esto no ejecuta nada, no mueve una luz y no contesta—. Su unico efecto es que se cierre un silencio. **No se manda: se respira** |
| **STM32, las dos puntas** | el **umbral de publicacion**. Antes se publicaba un `$EVENT` por CADA linea; con latido cada 2 s serian **1.800 lineas identicas por hora** en la bitacora donde hay que encontrar el fallo de campo. El umbral **sale del periodo del latido** —`LATIDO_MS x 1,5 = 3000 ms`—, no de un numero elegido. **Los contadores siguen contando todo: lo que se acota es lo que se publica** |

**El periodo sale de una ventana medida:** `1000 ms < T < 3500 ms`. Por abajo, el STM32 publica el
silencio en segundos enteros y por debajo de 1000 ms `MUDO` sale siempre `0 s`. Por arriba, el corte
mas corto que hay que ver es un ciclo de perro entero. ⚠️ **Ese techo descansa sobre
`ESP32_ARRANQUE_MS`, que sigue con `MEDIDO = 0` (AB-3)** — el dia que se mida, la ventana se
recalcula.

#### Cinco instrumentos se movieron, y el mas interesante era un PROXY

`esp32_10` comprobaba `B-1` —*el accesorio no origina trafico hacia el micro que gobierna el cruce*—
**prohibiendo que `vigilante.cpp` NOMBRARA `enlace_escribirLinea()`**. Medir el vocabulario del
fichero en vez de a donde va el parte. Funcionaba solo mientras no hubiera otra razon para hablar con
el STM32. **Se reparte en dos**, y ahora `B-1` se comprueba de verdad: que lo unico que sale de ahi
hacia el equipo sea el latido, **comparando el literal**.

Los otros cuatro —`app_07`, `esp32_09` y dos censos del simulador— tenian el mismo falso positivo:
**buscan literales `$` y no distinguen lo EMITIDO de lo COMPARADO**. Se acotan por el literal exacto,
nunca por una regla general: una trama de salida nueva tiene que seguir rompiendolos.

> 🔴 **Y la mitad que la inversion se habria llevado por delante (§8.sexies).** Al invertir la
> comprobacion del registro `J17` de «uno por linea» a «solo lo que pase del umbral», bastaba cambiar
> `==` por `<=`… y entonces **un firmware que no publicara NINGUN evento pasaria igual de bien**. Se
> anade el control que faltaba: el umbral se lee del C++ de las **dos** puntas y el periodo del
> `contrato.h` del ESP32, y se exige que **el umbral este entre un latido y dos y sea el mismo en las
> dos puntas**. Sin eso, *«el umbral sale del latido»* seria una frase, no una comprobacion.

**Flash del Maestro: 89,2 % -> 89,3 %** (58.456 -> 58.496 B). El banco pasa de **963 a 964**.
**La APK NO cambia**: `$LATIDO` muere en el despachador del STM32 y nunca llega a la app.


### 🟢 N-126 — SESION 2 DE BANCO (04/09): dos defectos cerrados con evidencia fisica, y el VERDE por primera vez

**Informe en `evidencia/Informe_Pruebas_Banco_Semaforos_Sesion2.pdf`.** Se corrio con **una sola
tarjeta** —la Maestro de la sesion 1 sigue con el corto y se descarto entera— reprogramada como
Maestro.

#### Lo que quedo CERRADO con evidencia en hardware

| | |
|---|---|
| **N-117** | el modulo se anuncia **estable, sin parpadear**, y ya con el rotulo aprendido: **`SEM-179DB0-M`**. Eso prueba de paso la cadena entera del `$STATUS`: el nombre solo se aprende de ahi |
| **N-122** | la app conecta y **opera de punta a punta**: `FORZAR_ROJO` -> rojo fisico, `SET_MODO:AMBAR` -> vuelve. Reversible |
| **N-125** | confirmado por su ausencia: ya no aparece el *«el escaneo fallo»* |
| 🟢 **VERDE FISICO** | **primera vez en dos sesiones de banco.** La prueba de focos encendio rojo, ambar y **verde** |
| **paso 2.bis** | el que se anadio esa manana **se ejecuto y salio completo**: las cuatro senales a 0 V. El binario nuevo entro |
| **paso 14** | al perder el cable la app **se congela y lo declara**; recupera sola en 1-2 s. No finge datos |
| **paso 12** | **ya no inventa la bateria**: declara que el equipo no la mide |
| **paso 27** | el reloj corre y `SET_RTC` responde *«hora puesta y aprobada»* |

> **Y el verde acota N-42 sin cerrarlo:** con la cadena de comandos funcionando de punta a punta y la
> salida de verde confirmada, lo que falla —si falla— esta en **el arranque del ciclo coordinado**, no
> en el hardware de salida ni en la cadena de mando. Sigue **sin confirmar ni descartar**: hace falta
> la segunda tarjeta.

#### 🔴 Y lo que la sesion destapo de NUESTROS documentos

**El paso 29 volvio a salir *«inconcluso»*, y no fue por falta de instrumento: fue porque la guia
mandaba el gesto equivocado.** Se corrigio el gesto en el aviso de cabecera y en la tabla de
conectores… **y NO en el paso 29 mismo**, que es donde se lee al hacerlo. Seguia diciendo *«p5 a masa
= A»* y *«un cable del pin a masa es exactamente lo que hace el relé»*. Con el firmware nuevo eso
**no produce nada**, y ademas es **el gesto que sobrecalento la tarjeta el 03/09**.

**El informe recomendo entonces un USB-TTL como Prioridad 1 para verificar N-118. Sobra, y la
propuesta salio de nuestra propia guia**, que apoya el USB-TTL en seis sitios como *«sin app, igual
con el USB-TTL»*. Dos cosas lo hacen innecesario, y las dos estan verificadas en el fuente:

1. **El mando ya confirma solo, con destellos de las propias luces** (`Maestro/src/mando.cpp:45-47`):
   `A·A·A` -> **2 destellos rojos**, `B·B·B` -> **3**, `A·B·A·B` -> **4**, rechazo -> ambar rapido de
   2 s. **Se ve desde el suelo**, sin app, sin cable y sin segunda tarjeta. Esta disenado asi porque
   quien acciona el mando esta a 5 m sin pantalla.
2. **Y la afirmacion del informe de que *«la app no expone un campo `MODO:`/`ESTADO:`»* es FALSA**:
   el STM32 los manda (`bluetooth.cpp:699`), el parser los lee (`nmea_parser.js:113-114`) y la app
   los pinta (`app.js:1180` y `:1235`).

> ⚠️ **Y una trampa que habria dejado el cruce con la app igual de inconcluso:** `A·A·A` hace
> `modoActual_set(MODO_AUTOMATICO)`, pero si el equipo **ya estaba** en automatico el firmware entra
> por `if (modoActual_get() == MODO_AUTOMATICO) modoAutomatico_setup();` y **`MODO:` no cambia**. Se
> prueba **desde otro modo**, o se cuentan los destellos, que se ven siempre.

**La leccion, y es de §2.ter: propusimos un instrumento para un problema que el firmware ya tenia
resuelto.** El dato estaba en las luces desde el primer dia.

#### 🔴 La respuesta 8 cierra AB-3 y ROMPE una desigualdad

El responsable midio el arranque del ESP32: **2 a 3 segundos**. Eso cierra `AB-3`, que llevaba
abierto desde que se escribio. Pero en `contrato.h` pone `ESP32_ARRANQUE_MS = 1500UL`.

Escrito el peor caso medido, el pack cae al instante:

```
FALLA  NO CABE: el watchdog (2000 ms) mas su arranque (3000 ms) suman 5000 ms
       y la cota es 5000 ms
```

**No se escribe `2000` porque cuadre** —coger el extremo favorable de un rango medido es exactamente
lo que costo el margen «2» que era **1,44**—. **Decision del responsable: medir fino primero.** Lo
medido es *energizado -> primer dato en la app*, que **incluye el emparejamiento y la app**; la
desigualdad solo necesita *reset del modulo -> volver a pasar bytes*, que sera bastante menor.
Mientras tanto la constante se queda en 1500 **con su bandera `MEDIDO = 0`**, que al menos es honesta
sobre no saberse.

#### Las 8 decisiones del responsable, tomadas

| | |
|---|---|
| 1 · tarjeta Maestro | **se repara** |
| 2 · proteccion de entradas (N-120) | 🟢 **2K2 APROBADO** para diseno |
| 3 · mando A/B | falta comprobar en tarjeta |
| 4 · refresco del tablero | 🟢 **baja a 2000 ms** |
| 5 · caducidad del PIN | **no**, queda como esta. `AB-9` cerrado por decision |
| 6 · placa portadora | aun no definitiva |
| 7 · si el ESP32 se cuelga | **sigue en el ultimo modo Y avisa.** `AB-1` decidido: **latido propio del ESP32** |
| 8 · arranque del ESP32 | **2 a 3 s** — cierra `AB-3` y abre lo de arriba |


### 🟢 N-125 — La app no pedia el permiso de Bluetooth. CERRADO EN CAMPO, y obliga a rebajar dos hallazgos

**04/09, 13:32, del funcional: *«ya funciona la app»*.** Es el primer bloqueo del banco que se
cierra, y desatasca los pasos 11-14 y 25-28.

#### El defecto

```
BluetoothSerial.java:107-109   list -> listBondedDevices()  SIN comprobar permiso
BluetoothSerial.java:220       el UNICO requestPermission del plugin: ACCESS_COARSE_LOCATION
                               -permiso de Android 6-11, y cuelga de DESCUBRIR, no de listar-
MainActivity.java              vacio: no pedia nada
variables.gradle               targetSdk 34
```

Desde Android 12 (API 31), `getBondedDevices()` exige **`BLUETOOTH_CONNECT` concedido EN RUNTIME** y
lanza `SecurityException` si no. Esa excepcion caia en el callback de error del plugin y salia como
*«el escaneo fallo»*, que fue literalmente lo que el funcional reporto.

> **Los permisos SI estaban declarados en el manifest desde siempre. Declarar no es pedir:** en
> runtime, un permiso peligroso no concedido se comporta **igual que uno que no existe**. Por eso
> costo verlo — el manifest se lee y parece completo. Es un `pinMode()` sin `digitalRead()` con otra
> ropa, y es la misma familia que N-73.

#### 🔴 LO QUE ESTE CIERRE OBLIGA A REBAJAR, Y ES LA PARTE QUE IMPORTA

**Si funciono SIN reflashear el ESP32 —que es lo que parece—, entonces N-117 NO era la causa.** El
watchdog seguia siendo un defecto real —un techo de 2 s sobre un arranque que `contrato.h` declara
sin medir es una apuesta, no una proteccion— y su arreglo se queda. **Pero no era lo que bloqueaba
el banco**, y apuntarselo seria fabricar un acierto.

Es §4 otra vez, y esta vez en nuestra contra: **una causa plausible que resulto no ser la causa.** Se
degrada de 🔴 a 🟠 y se deja escrito, en vez de quedarse como "arreglado" al lado del sintoma que no
arreglo.

#### ⚠️ Y LO QUE TODAVIA NO ESTA MEDIDO, dicho como tal

*«Ya conecta seguramente»* es una suposicion del responsable, no una observacion del banco. Lo unico
CONSTATADO es que **la app dejo de dar el error de escaneo**. Faltan tres cosas, y cada una cierra un
hallazgo distinto:

| lo que hay que VER | que cierra |
|---|---|
| sale la lista con el ESP32 | **N-125** — esto ya esta |
| conecta y el boton queda en «Enlazado» | **N-122**: el socket abre de verdad |
| **llega telemetria viva** —el contador se mueve solo— | la cadena entera: STM32 -> J17 -> ESP32 -> app |

**Y la prueba que las cierra las tres de golpe, sin depender de lo que pinte el tablero:** dejar el
ESP32 hablando un rato con el Esclavo, **reiniciarlo**, y mirar el nombre. Si paso de
`SEM-SIN-MATRICULA` a `SEM-<serie>-E`, el `$STATUS` salio del STM32, cruzo J17, se parseo y se
guardo. El rotulo solo se aprende de ahi.

#### La solucion inmediata que quedo escrita, porque servira otra vez

Conceder **«Dispositivos cercanos»** a mano en Ajustes de Android desbloquea la app **sin instalar
nada**. Sirve con cualquier APK anterior, y es la primera comprobacion cuando alguien diga que el
escaneo falla.


### 🔴 N-122 — La app NUNCA abria el socket: faltaba `connect()`, y eso bloqueaba el banco por si solo

**La pregunta que lo destapo fue del responsable el 04/09: *«si pasan los simuladores, ¿la apk no
necesita cambios?»***. La APK del 02/09 esta al dia con el fuente —ningun commit ha tocado
`App_Semaforo/` desde entonces—, asi que por esa via la respuesta era *no*. Mirando **que hace** el
fuente, la respuesta es que si, y era lo que tenia parado el banco.

#### El defecto

Al pulsar una fila de la lista de equipos, `app.js` hacia esto:

```js
state.connected = true;               // <- sin haber abierto nada
state.deviceMac = mac;                // <- y esta variable no se leia NUNCA mas
...
window.bluetoothSerial.subscribe('\n', ...);   // <- sobre un socket inexistente
```

**`connect(mac)` no se llamaba en ningun sitio.** En `cordova-plugin-bluetooth-serial`, `subscribe()`
y `write()` operan sobre la conexion que abre `connect()`: sin ella no hay `BluetoothSerialService`,
la suscripcion no engancha, y el `write()` de `enviarComandoFirmware()` se va al vacio. La app se
pintaba **«Enlazado»** por haber pulsado una fila.

> **Esto bloquea los pasos 11 a 14 y 25 a 28 POR SI SOLO, con independencia de N-117.** Un ESP32
> perfecto, anunciandose con su nombre correcto y sin un solo reinicio, tampoco habria conectado.
> Son dos defectos en serie sobre el mismo camino, y el banco solo podia ver el sintoma del final.

#### Por que ningun instrumento lo cazo, que es la parte que hay que entender

**No es que nadie lo supiera: estaba anotado, y aprobado.** `app_07_generadores_de_trama` lleva una
lista **congelada** de huerfanos conocidos, y ahi dentro esta:

```python
"BluetoothDriver": "js/bluetooth_driver.js - transporte alternativo sin conectar",
# Capa de transporte SPP/BLE/Serial escrita entera. app.js habla por window.
# bluetoothSerial y por fetch() al puente, sin pasar por aqui.
```

`js/bluetooth_driver.js` tiene la llamada escrita —`conectarNativoSPP()`, con su `connect()` y su
`subscribe()` dentro del callback de exito— y **cero llamadores**. El pack lo acepta porque su regla
es un **trinquete**: falla si aparece un huerfano NUEVO o si uno de la lista GANA llamador y no sale.
Eso es correcto y es N-73 bien aplicado.

**Lo que fallo fue el MOTIVO con el que se acepto.** *«app.js habla por `window.bluetoothSerial`»* es
**medio cierto**: app.js usa `write`, `subscribe` y `list`… y **no usa `connect`**, que es justo la
que hace funcionar a las otras tres. Nadie comprobo esa frase entera, y la mitad que faltaba era la
unica que importaba.

> **La regla que deja: un huerfano se acepta por una razon, y la razon es una afirmacion sobre el
> codigo — o sea, algo que se comprueba, no que se escribe.** Una lista de excepciones con motivos
> sin verificar es una lista de defectos con permiso.

#### El arreglo, y el orden que es la mitad de el

Se llama a `connect(mac)` y **`state.connected` solo se pone a `true` en su callback de exito**, junto
con el `.connected` del boton y la suscripcion. El fallo se dice —`$ERR` visible, estado en falso— en
vez de pintar un enlace que no existe.

**El orden no es cosmetico:** `enviarComandoFirmware()` guarda con `&& state.connected`. Con la
bandera puesta al pulsar, esa guarda **daba paso a ordenes que no tenian por donde salir**, y el
operario las veia aceptadas. Es §8.sexies otra vez: lo que se siente en la calle es el orden.

**Y hay TRES copias del fuente, no dos.** `documentos_03_trama_status` lo caza y por eso el primer
intento salio en `FALLA`: `app.js`, `www/app.js` y **`android/app/src/main/assets/public/app.js`**,
que es desde donde se construye la APK. *«Lo que se prueba en el navegador y lo que se instala en el
celular del tecnico son dos programas distintos»* — las tres quedan identicas.

**Compuerta: `20 PASS | 0 FALLA | 0 ABORTADO`.** 🔴 **Pero esto NO llega al telefono hasta recompilar
la APK** (`APP-APK`, JDK 17, verificacion por CRC entrada por entrada). El fuente arreglado en el
repositorio no es una app arreglada en la mano del tecnico.


### 🔴 N-123 — La guia perdia 12.600 caracteres al imprimir, y eran los que hay que contestar

**El PDF que devuelve el funcional ES el canal de vuelta** —asi llego el informe del 3-4/09—, o sea
que lo que no se imprime no se contesta. Y la hoja de impresion llevaba esto:

```css
.barra, .noimprimir, .detalle-cab, details { display: none !important; }
```

**Medido: 12.600 caracteres en ocho bloques plegados que NO salian en el PDF.** Entre ellos:

| | |
|---|---|
| 1.820 | *«Consulta · que va en cada bornera»* — **las conexiones** |
| 2.241 | *«Paso 22 · la placa del modulo»* — **la arquitectura** |
| 1.008 | *«Lo que esta visita NO decide»* — **las preguntas abiertas** |
| 1.478 | *«Paso 9 · el montaje de mesa, cable a cable»* |
| 2.384 | *«Paso 29 · el mando sin receptor»* · y tres mas |

**Nadie lo noto porque en pantalla estaba todo.** Se veia bien y volvia incompleto, y quien rellena
no puede echar de menos lo que no sabe que existe.

**Las dos mitades del arreglo, porque la primera sola no basta:** un `<details>` cerrado **no se abre
desde CSS** —el navegador oculta sus hijos por el mecanismo del elemento—, asi que hace falta poner
`open`; y eso va en **`beforeprint`, no dentro del boton**, porque el PDF sale tambien con `Ctrl+P` y
un arreglo que viva solo en el boton seguiria perdiendo lo mismo en silencio. Se restaura en
`afterprint`. **La regla queda en `CLAUDE.md` §4.quater.**

**Seccion nueva `9.bis · Preguntas abiertas`, con hueco de respuesta.** El bloque que habia
**listaba** lo no decidido y no dejaba contestarlo — *«una pregunta sin hueco para contestar no es
una pregunta: es una nota»*. Son ocho, cada una diciendo por que no la desatasca una medida, y con
el aviso de que *«no se»* o *«lo decide Julio»* valen: una en blanco no se distingue de una que se
paso por alto, y vuelve dentro de un mes.

#### Y los acentos NO eran el problema — se deja escrito para no volver a buscarlo ahi

Se reporto *«que salga en español, no caracteres raros»*. Medido sobre el fichero: **decodifica como
UTF-8 limpio, sin BOM, 662 acentos bien formados, CERO mojibake**, y el `<meta charset>` puesto desde
siempre en la linea 4. **Lo que si podia salir raro son los 25 emoji**, porque ninguna de las **33**
pilas de fuentes declaraba una fuente de emoji; se anade como **ultimo recurso** en todas —no cambia
una sola letra de texto normal, solo entra cuando el glifo no existe en las de delante—.

> **Y una que casi se «arregla» rompiendo algo que funciona:** el fichero tiene dos-puntos de ancho
> completo (`U+FF1A`), que parecen un caracter CJK colado. **Son intencionales**: viven dentro de la
> regex `/[:：]$/` que limpia los dos. Se miro el contexto antes de tocarlos. Es §4 aplicada a una
> «corrección».

#### El defecto del paquete que esto destapo

**La guia no entraba en el `.zip`.** El filtro de manuales era `.md`/`.docx`, y `CLAUDE.md` dice que
el HTML de cableado **viaja en el paquete de entrega**. Corregido, y ademas va **en la raiz** —es lo
que se abre y se devuelve, no un manual de consulta— con una comprobacion nueva que lo exige. El
paquete pasa de 303 a **304 entradas**.


### 🟠 N-121 — Censo de las cuatro salidas del Degradado del Esclavo: dos muertas, y son las que no dependen del ESP32

Sale de auditar una revision externa, no de trabajar. Dos cosas, y la primera corrige este roadmap.

**1. `N-106` esta CERRADO y esta tabla lo daba por abierto.** Lo cerro `2e99bc3` con el molde de
cuatro filas de `Esclavo/src/bluetooth.cpp`, y `esclavo_08_ambar_en_degradado` lo vigila **8/8 con
cinco controles negativos** que saben distinguir el arreglo del defecto. Llevaba dias escrito como
*«en curso»*: **un defecto cerrado que el roadmap sigue publicando como abierto cuesta la misma sesion
que uno real**, y es el error de N-100 -los `MEDIDO` caducados- por el otro lado.

**2. El censo, que el propio pack imprime:** `degradado_salir()` tiene **cuatro** llamadores.

| via | estado |
|---|---|
| `main.cpp:385` — vuelve el Maestro por radio (`CMD_PING`/`GO_RED`/`GO_GREEN`) | 🟢 **viva** |
| `bluetooth.cpp:273` — `AMBAR_EMERGENCIA` desde la app | 🟢 **viva** (N-106) |
| `menu.cpp:215` — el menu del Esclavo | 🔴 **muerta**: `botonAceptar()` devuelve `false` desde el 31/08 |
| `mando.cpp:121` y `:138` — `A.A.A` y `B.B.B` | 🔴 **muerta**: N-118, el pin no puede dar un flanco |

**Ninguna esta rota por descuido:** la del menu se retiro con un censo que nombra su sustituto, y ese
sustituto **era el mando**. Lo que nadie podia saber al escribirlo es que el mando tampoco iba a
poder pulsarse — eso se midio en cobre el 03/09.

> **Lo que el censo dice, y es lo unico que hay que retener: las dos vias muertas son exactamente las
> dos que NO necesitan el ESP32.** Con el Bluetooth caido —que es la situacion del banco— al operario
> de pie junto al cruce **no le queda ninguna**: solo que vuelva el Maestro por radio. Es la frase que
> el propio `ESP32_Expansion/src/main.cpp` tiene escrita en su cabecera —*«un ESP32 colgado deja el
> equipo seguro pero NO OPERABLE»*— ocurriendo de verdad, y con una punta menos de las que se creia.


### 🔴 N-117 — El perro del ESP32 se comia su propio arranque, y el pack lo aprobaba mirando la forma

**Sintoma en banco, paso 10:** el modulo *«no aparecio de forma confiable»* en la lista del telefono,
con el firmware cargado sin errores y el hardware confirmado compatible (BR/EDR, o sea SPP).

#### El defecto

`ESP32_Expansion/src/main.cpp` armaba el watchdog —**2 s, `panic = true`**— y **no lo alimentaba
hasta el primer `vigilante_alimentar()` de `loop()`**. Entre medias, cuatro etapas compartiendo **un
solo presupuesto de 2000 ms**:

```
enlace_setup()        Serial2, barato
reloj_setup()         Wire.begin() + lectura del OSF del DS3231
transporte_abrir()    Preferences/NVS  +  pila Bluedroid CLASICA entera
puente_setup()        trivial
```

La cara es la tercera. **Y cuanto tarda no lo ha medido nadie: lo declara el propio `contrato.h` en
`AB-3`**, con todas las letras —*«Nadie ha medido cuanto tarda este modulo desde el reset hasta
volver a pasar bytes»*— y con `ESP32_ARRANQUE_MEDIDO = 0` para que no se lea como cifra.

**Un techo duro de 2 s sobre un arranque de duracion desconocida no es una proteccion: es una
apuesta.** Si se pierde, `panic` reinicia, y vuelta a empezar — para siempre.

> **Y asi es como se ve desde fuera, que es lo que ata el defecto al sintoma:** un modulo que
> rearranca cada 2 s **no parece averiado desde el telefono. Parece que APARECE Y DESAPARECE de la
> lista**, porque el descubrimiento de Android necesita que el equipo se quede en *inquiry scan*
> varios segundos seguidos. Es literalmente la frase del paso 10.

#### Por que salia verde: el pack medía la forma, no la propiedad

`esp32_02_watchdog_alimentado` comprueba **11 cosas** y todas pasan: que se arma (`W-1`), que se
registra la tarea correcta, que `loop()` alimenta **despues** de bombear (`W-2`), que ningun `while`
esconde el reset (`W-3`), que los bucles llevan tope (`W-4`) y que el perro se arma **antes** del I2C
y del SPP (`W-5`).

**Ni una sola acota la DURACION de la ventana contra `ESP32_WDT_MS`.** Comprueba el **orden** del
armado y la **presencia** del reset; nunca pregunta si lo de en medio **cabe**. Es la forma exacta de
la prueba muerta de §3.bis: verde perfecto sobre la propiedad de al lado.

#### El arreglo, y lo que NO debilita

Se alimenta **entre etapa y etapa**, en linea recta. `W-5` queda intacto —cada etapa se alimenta
*antes* de entrar y la siguiente solo alimenta si la anterior **volvio**, asi que un `DS3231` que
cuelgue el bus sigue sin llegar a su reset y el perro muerde a los 2 s—; `W-3` tambien, porque no hay
ningun `while` de por medio. Lo unico que cambia es que **cada etapa tiene sus propios 2 s en vez de
repartirse unos solos entre las cuatro**.

**Compuerta tras el cambio: `20 PASS | 0 FALLA | 0 ABORTADO`, codigo 0**, acta
`evidencia/2026-09-04_compuerta.txt`. `esp32_02` sigue en 11/11 y `esp32_07` en 11/11.

#### 🟠 Y HAY UNA SEGUNDA CAUSA CANDIDATA, MAS BARATA DE COMPROBAR, que esta revision no habia visto

No la encontro este analisis: la aporto una revision paralela el 04/09, y hay que apuntarla porque
**explica el mismo sintoma sin ningun defecto** y se descarta en treinta segundos.

`transporte_abrir()` llama a `cargarRotulo()`, que lee el nombre SPP de la NVS. Si no hay nada
guardado —modulo virgen— usa `ROTULO_PROVISIONAL`, que vale **`SEM-SIN-MATRICULA`**
(`contrato.h:212`). Y el aprendizaje del nombre bueno **se guarda para el arranque SIGUIENTE**, nunca
en caliente: renombrar obligaria a cerrar el perfil y tirar la sesion del operario
(`transporte_app.cpp:109-114`, decision deliberada y razonada alli).

**Consecuencia: durante TODA la sesion de banco el modulo se anuncio como `SEM-SIN-MATRICULA`**, hable
o no el STM32 con el. Quien buscara `IOT_VIAL`, el nombre del cruce o algo con la serie **no lo
reconocio en la lista**, y eso se reporta exactamente igual que un modulo que no aparece.

> **Las dos causas no compiten, y por eso el orden de comprobacion importa:** la del rotulo cuesta
> mirar una lista de Bluetooth; la del perro cuesta un monitor serie. **Se mira el nombre primero.**
> Un `SEM-SIN-MATRICULA` presente y estable en la lista **descarta el bucle de reinicio de golpe** —un
> modulo que rearranca cada 2 s no se queda quieto en un escaneo—, y entonces el arreglo del perro
> sigue siendo correcto pero no era esto.

#### 🔴 Lo que falta, y no es codigo

**Esto es una hipotesis con arreglo aplicado, no una causa demostrada.** Se confirma en dos minutos y
**antes** de reflashear: monitor serie a **115200** sobre el CP2102. El firmware no imprime nada
propio —no hay un solo `Serial.begin()` en el proyecto, comprobado—, pero **la ROM del ESP32 si saca
su banner** (`rst:0x...`, `ets ...`) en cada arranque.

| lo que se ve | lo que significa |
|---|---|
| banner repitiendose cada ~2 s | **bucle de reinicio: N-117 confirmado** |
| banner una vez y despues silencio | es **otra cosa** — antena, *advertising* o interferencia — y hay que mirar ahi |

> **Y la regla que deja: no se escribe un pack para esto.** La propiedad que fallo es un **tiempo**, y
> un tiempo no se lee del fuente. Un pack que exigiera *«hay un `vigilante_alimentar()` entre etapa y
> etapa»* estaria midiendo la forma otra vez — el mismo error que dejo pasar esto. **La medida vive
> en el modulo; lo que va al repositorio es el numero medido en `ESP32_ARRANQUE_MS` y su bandera
> `ESP32_ARRANQUE_MEDIDO` a 1.** Ese es el cierre de `AB-3`, y sigue abierto.


### 🔴 N-118 — El mando A/B no se puede pulsar: SFTY-21 se quedo sin respaldo fisico, y esta medido en las dos mitades

El informe lo reporta como `H2`, *«riesgo sobre el respaldo de seguridad del mando de reles»*.
**Cruzando su medida con el fuente, no es un riesgo: ya pasó.**

#### La mitad del cobre, del paso 20

`J16` p5 (`PB9`, mando A) y p8 (`PB13`, mando B) miden **9,92 kOhm a masa** y **0,6 V** con energia.

#### La mitad del fuente, que estaba escrita desde el 31/08

`pines.h:107-110` ya traia la cuenta hecha, para las camaras: el pull-up interno (~40 kOhm) contra un
pull-down de 10 kOhm deja el pin en `3,3 x 10/50 = 0,66 V`, **que el micro lee LOW**. El funcional
midio **0,6 V**. La prediccion y la medida coinciden.

Lo que nadie habia cruzado es que **`R65`/`R66` hacen lo mismo en p5/p8**, donde el firmware **sigue
leyendo activo en BAJO**. Y entonces `botones.cpp` cierra el circulo solo:

- `botones_setup()` siembra `disparadoAnt[i] = pulsado` — un pin que ya viene en LOW al arrancar
  queda marcado como **«flanco ya consumido»** (es N-26, y es la decision correcta).
- `botones_actualizar()` solo llama a `mando_registrarPulso()` **con un flanco**.
- Con el pin clavado en 0,6 V **nunca sube**, luego **nunca hay flanco**, luego **el mando no
  registra un solo pulso en toda la vida del equipo**.

**Ninguna de las tres secuencias —`A·A·A`, `B·B·B`, `A·B·A·B`— es alcanzable.** Y el propio comentario
de `botones.cpp:186-189` lo habia anticipado sin saber que ya era el caso: *«si esta trabado de
verdad, el equipo arranca en el menu y ese boton no responde»*.

> **Confirmacion independiente, del paso 29:** al puentear p5/p8 contra masa *«no se observo ningun
> cambio de comportamiento»*. Claro: **el pin ya estaba en LOW**. El puente no cambiaba nada.

#### Por que importa mas de lo que parece

El mando de reles es **el respaldo fisico de seguridad deliberadamente conservado el 31/08**, el unico
camino de mando que **no depende de la app ni del ESP32**. Con la pantalla y los pulsadores retirados,
era lo unico que quedaba cuando el Bluetooth falla — que es exactamente lo que acaba de pasar en banco.

**Hoy el equipo no tiene ninguna via de mando local.** Y el veto de `mando_ambarLocal()` que
documenta `CLAUDE.md` §3.ter —los tres `if` de `Esclavo/src/main.cpp` que impiden que una orden de
radio saque del ambar a un operario— **no puede armarse nunca**, porque su bandera cuelga de un pulso
que no llega. Es el caso que aquella regla describe, ocurriendo por hardware en vez de por un borrado.

#### 🟢 La tercera mitad, del 04/09: el COBRE decide la polaridad, y ya no hay ambiguedad

`CLAUDE.md` §9.bis llevaba abierta *«la contradiccion entre el netlist y el fuente»* sobre la
polaridad de estos pines. Leido el `.kicad_pcb` con un parser de parentesis balanceados **no hay
contradiccion: hay un fuente equivocado**.

```
J16.4  /3.3V     J16.5   /Boton1  -> R65 10K a masa + C26 100nF + U1.46 (PB9)
J16.7  /3.3V     J16.8   /Boton2  -> R66 10K a masa + C27 100nF + U1.26 (PB13)
J16.9  /3.3V     J16.10  /Boton3  -> R67 10K a masa + C28 100nF + U1.27 (PB14)
J16.11 /3.3V     J16.12  /Boton4  -> R68 10K a masa + C29 100nF + U1.28 (PB15)
```

**Los CUATRO son identicos, y los cuatro tienen 3,3 V en la posicion de al lado.** El conector esta
diseñado, sin lugar a duda, para que **un contacto seco cierre el pin contra los 3,3 V vecinos**, con
el 10K sujetando el reposo en bajo. Eso es **activo en ALTO para los cuatro**.

*(Y de paso queda verificado que la asignacion de pines del netlist y la de `pines.h` coinciden pin a
pin —`U1.46 = PB9`, `U1.26 = PB13`, `U1.27 = PB14`, `U1.28 = PB15` sobre el LQFP48—. Lo que difiere no
es el mapa: es solo como los lee `botones.cpp`.)*

Asi que la opcion **1 no es una preferencia: es lo que la placa pide**, y la 2 seria pelear contra el
diseño.

| | que | coste |
|---|---|---|
| **1** ✅ | **Leer A/B activo en ALTO**, exactamente como las camaras desde el 31/08 — `INPUT` pelado y contacto contra los 3,3 V de p4/p7 | firmware. Es el mismo bloque que ya corre en C y D |
| **2** ❌ | Retirar `R65`/`R66` para que gane el pull-up interno | tocar cuatro placas y quedarse con el pin flotando si el receptor se desconecta |

**Lo tecnico queda decidido por el cobre. Lo que sigue siendo del responsable es lo de fuera:** con que
salida se compra el receptor de mando —NO o NC— y quien valida que un cambio en un camino de seguridad
entra sin banco. **No se implementa de oficio.**


### 🟠 N-119 — El ritmo de J17: la pregunta era buena y la respuesta es «ya es por eventos, salvo un latido»

**Pregunta del responsable (04/09):** *«no debe ser tan continua sino por eventos entre el ESP y el
STM, no son un computador y no aguantan esos ciclos tan rapidos»*.

Medido con `esp32_07_presupuesto_bytes`, que recalcula esto del C++ en cada corrida:

| direccion | como es hoy |
|---|---|
| **app -> STM32** | **ya es puramente por eventos.** Cero envios periodicos, y hay pack que lo exige (`P-1`/`P-4`: ni `puente.cpp` ni `enlace_stm32.cpp` tienen reloj). Por J17 entra **exactamente lo que un dedo pulsa** |
| **STM32 -> app** | eventos **+ un `$STATUS` cada 2000 ms** desde el 04/09 (antes 1000) |

```
peor segundo    528 B de 960 B/s   =  55,0 %   ($STATUS + $EVENT + $ALARM + $ACK)
reposo          ~130 B             =  ~13,5 %
```

**Lo que la medida descarta:** el ritmo **no** es la causa del calentamiento de N-116. 9600 baudios es
un periferico UART al 13,5 % de uso; no hay ciclo rapido que quemar. No se persigue por ahi.

**Lo que la medida confirma del instinto:** el unico consumidor del `$STATUS` es `vigilarEnlace()` de
la app, y **su cota son 5 s**. El latido va **cinco veces mas rapido de lo que nadie necesita**, y el
peor segundo se come mas de la mitad del cable.

> 🔴 **Y aqui se publico una cifra que la medida refuto.** Ponia que pasarlo a 2000 ms lo dejaria
> «bajo el 30 %». **MEDIDO tras el cambio: 462 B de 960 B/s = 48,1 %**, no «bajo el 30 %» como se publico aqui. Solo el `$STATUS` periodico se parte por dos; el `$EVENT`, el `$ALARM` y el `$ACK` que coinciden en el peor segundo **no escalan con la cadencia**. Era una cuenta hecha a ojo con autoridad de dato, y el cambio se aplico igual porque 48,1 % sigue siendo mejor que 55 %
> — pero el numero que lo justificaba estaba mal.

**El coste, declarado:** el tablero del operario refresca la mitad de rapido. **Es decision del
responsable** —afecta a lo que ve quien decide sobre el trafico—, esta en §0.3 y **no se toca de
oficio**.


### 🟠 N-113 — Si el ESP32 se cuelga: que sigue funcionando, que NO, y por que la app no es un canal de alarma

**Propuesta del responsable (01/09):** *«si eso pasa, la apk deberia informar que no hay conexion y
reportar fallo, en cuyo caso el otro micro se queda trabajando con tiempos. Ideal que la apk enviara
un correo de notificacion a nosotros o a la concesion para que puedan apoyar con personal de trafico
mientras la reaccion al fallo.»*

**La base es correcta, pero el supuesto «el otro micro se queda trabajando con tiempos» son DOS
casos, y solo uno se cumple.** Medido en el fuente:

| modo | de que depende | si el ESP32 muere |
|---|---|---|
| **Automatico** | `millis()` — `modo_automatico.cpp:67,126,141` | 🟢 **sigue ciclando.** No toca el reloj |
| **Degradado** | `reloj_enHora()` — `modo_degradado.cpp:155,329,349,515` y `coordinador.cpp:335,476,498` | 🔴 **no se puede ni entrar** |

> 🔴 **Y ahi esta el problema de la arquitectura que se decidio, escrito sin rodeos: se colgo el reloj
> del ESP32 (DS3231 por `J17`), y el Modo Degradado sale ENTERO de ese reloj.** O sea que el accesorio
> que se anadio para ganar funciones es tambien el que puede llevarse por delante la funcion de vida
> —el todo-rojo coordinado que evita verde-contra-verde—. Es exactamente lo que la auditoria de N-109
> senalo en su punto 3, ahora con las lineas delante.

#### Por que la app NO puede ser el canal de alarma

**No es que sea mala idea: es que no llega a tiempo por construccion.** El unico enlace de la app es
**Bluetooth SPP a traves del propio ESP32**, y eso impone tres cosas a la vez:

1. **El alcance es de metros.** La app solo se entera cuando alguien ya esta en el poste — el momento
   en que el correo sobra, porque el tecnico ya lo esta viendo.
2. **Si el ESP32 es lo que se colgo, el enlace de la app es justo lo que ha desaparecido.** La app no
   puede distinguir *«el puente esta muerto»* de *«estoy fuera de alcance»* o *«tengo el Bluetooth
   apagado»*, y las tres se ven igual desde el telefono.
3. **Nadie tiene la app abierta a las 3 de la manana.** Una alarma que depende de que un humano este
   mirando no es una alarma; es un aviso al que ya estaba mirando.

**La app SI debe declarar la ausencia de enlace** —y hoy ya lo hace, sin inventarse el estado
(§3.quinquies)—. Lo que no puede es ser quien avisa a la concesion.

#### Lo que si es barato y honesto, hoy

> **El equipo que sobrevive al fallo es el STM32, asi que es el STM32 quien tiene que llevar el
> registro.** El ESP32 no puede reportar su propia muerte.

- **El STM32 cuenta el silencio de `J17`** igual que ya se cuenta el de la radio (N-108): cuanto lleva
  mudo el puente, cuantas veces se cayo, y cuanto duro cada corte. **No necesita cobertura, ni SIM, ni
  internet, ni que nadie este delante**, y cuando el tecnico por fin conecta, la app se lo descarga
  entero. Contesta justo lo que se dijo que falta en campo: *«no saber cuanto se va cuando se va, y
  por que se va»*.
- **Watchdog en el ESP32** (ya es la tarea **T4**) para que se reinicie solo, y que **declare el
  reinicio** al volver — un puente que revive en silencio esconde el fallo que hay que contar.
- **Y la decision que hay que tomar antes de escribir nada: que hace el equipo si el reloj se va.**
  Hoy la respuesta es *«el Degradado no entra»*, y eso hay que elegirlo a proposito, no heredarlo.

#### Lo que cuesta de verdad un aviso remoto, para que se decida con el precio delante

Un correo desde el poste necesita **camino propio a internet**: WiFi del sitio o un modulo celular con
SIM y su plan. El ESP32 trae WiFi —esa parte esta— pero **hace falta cobertura en el cruce y una red a
la que entrar**, y si el que avisa es el mismo que se cuelga, el aviso no sale. **Es una decision de
producto con coste recurrente, no una linea de firmware**, y va al responsable con esa etiqueta.

---

### 🔴 N-114 — Segunda auditoria externa: "arreglamos todo lo que midio y nada de lo que dijo"

**Se pidio que NO repitiera N-109 y que dijera que ha cambiado, que no, y que se ha vuelto peor
precisamente por lo que se hizo bien.** Todo lo que sigue esta verificado por el orquestador.

#### 1. Lo que reconoce como bueno, para saber que conservar

`Validacion_Automatico/dos_puntas` *«es un salto de clase, no de cantidad»*: lo ejecuto el mismo y da
`42/42`, verde simultaneo en 0 de 53.236 instantes, compilando **siete ficheros reales del Esclavo,
`src/main.cpp` incluido**. Sus puntos ciegos estan **declarados en su propia cabecera**, y eso es lo
que hay que conservar. Igual de reales: el checksum conectado, el bit 12/24 del `DS3231`, el parte de
arranque y el contador de `J17` — **cinco propiedades nuevas de verdad**.

#### 2. 🔴 El README publicaba un verde que su propia acta desmentia — otra vez, y a las 24 horas

```
README.md:58   banco por packs  OK 829/829 - los 59 packs en PASS
acta CITADA    FALLA banco por packs - 824/829 | packs: 57 PASS, 2 FALLA
```

**Verificado.** Y no es el mismo defecto de N-109: aquel fue un descuido. Este salio de **repetir la
corrida hasta que una dio verde**, con la alternancia de N-112 como excusa disponible. *«La
alternancia se ha convertido en licencia para publicar el numero de la pasada que salio verde»* — el
habito que N-112 describe como destructor de la compuerta, ejercido **en el mismo commit que lo
documenta**.

#### 3. Lo que empeoro por lo que se hizo bien

| | |
|---|---|
| **La sofisticacion de `documentos_01` es lo que ROMPIO la compuerta** | anclar el recuento a la linea del acta era rigor bien intencionado; su rama `else` hizo que el total dependiera del veredicto. **Un defecto creado por el instrumental, no encontrado por el** |
| **La observabilidad se pago en la flash del micro mas lleno** | Maestro **+472 B en una tarde**, y el contador de `J17` se grabo en las DOS puntas **despues** de que la propia medicion demostrara que hoy no distingue nada. Firmware pagado por adelantado de una decision (`AB-1`) que sigue abierta |
| **El checksum cambia comportamiento de campo y solo se valido en PC** | el modo de fallo nuevo es **asimetrico**: antes la basura mantenia la interfaz *«viva»*; ahora una divergencia real deja a la app **sorda diciendo que no hay equipo, junto a un equipo que funciona** |
| **El numero estrella ya viaja sin sus salvedades** | el roadmap cita *«42/42, 0 de 53.236»*; la letra pequena vive en la cabecera de un `.cpp` que quien lea esto no va a abrir. **El instrumento es honesto; el titular le va a sobrevivir** |

Y la relacion instrumento/firmware **se ha duplicado en cuatro dias**: 14.829 lineas de firmware
contra **30.740 de instrumentacion**, o sea **2,07:1** — el 28/08 era 1:1.

#### 4. 🔴 El invariante de vida: lo que SIGUE sin cubrir

> **Las DOS puntas en Degradado a la vez** — el modo disenado para cuando la radio muere, donde el
> verde de cada punta sale de **su propio reloj**.

`Maestro/src/modo_degradado.cpp` esta **excluido** del arnes nuevo, y el reloj del Esclavo esta anclado
al mismo `arnes_millis`, asi que **la deriva entre relojes no es representable**. La desigualdad que
decide un choque frontal —despeje ampliado contra deriva acumulada— hoy la recalcula **solo
`costura_02_fase_ciclo.py`, el modelo de Python escrito a mano**: el invariante del modo mas critico
sigue sostenido por exactamente la clase de instrumento que la auditoria anterior senalo.

**Y es alcanzable sin banco, con piezas que ya existen aqui:** `Validacion_LCD/compilar.ps1` **ya
compila el `modo_degradado.cpp` real con las fuentes de U8g2 en el host**. La exclusion *«arrastra
u8g2»* es un problema **ya resuelto a diez directorios de distancia**.

Segundo hueco alcanzable: **`protocolo.cpp` no entra** — una trama corrompida, repetida (*replay*) o
de otra pareja no se ejerce de punta a punta.

#### 5. La pregunta del 31/07: **sigue siendo evitacion**, con esas palabras

- 19 commits, **~7.400 lineas insertadas, cero contacto con una tarjeta**.
- La V2 escribio *«lo que NO entra: mas packs»* y **esa misma noche el banco crecio de 445 a 859**.
- **El montaje de mesa con tres cables —que se puede hacer HOY, sin comprar nada, con la guia ya
  escrita— no se hizo**, mientras si hubo tiempo para 42 comprobaciones nuevas del arnes que lo
  sustituye en PC.

> *«El proyecto ha perfeccionado la descripcion de su evitacion hasta que la descripcion funciona como
> entregable. N-109 dijo "industria de sustitucion"; la respuesta fue anadirle a la industria un
> departamento de autocritica.»*

#### 6. 🔴 Las tres preguntas que nadie estaba haciendo

**(a) Por que los documentos COPIAN cifras en vez de citarlas.** Toda la familia N-62 -> N-93 -> N-112
existe porque README, ESTADO y CERTIFICACION **duplican** numeros que el acta ya publica, y hay
**1.120 lineas de Python vigilando las copias**. Si los documentos dijeran *«banco: ver la ultima
acta»* y publicaran solo lo que el acta **no** mide, `documentos_01`, `documentos_04` y N-112 entero
**desaparecen sin arreglar nada**. *«El instinto del proyecto ante cada defecto es anadir un
instrumento»*, y nadie ha preguntado **cuales de sus defectos los fabrica la propia duplicacion**.
**Es una decision del responsable y esta abierta.**

**(b) Cual es el incremento MINIMO cargable.** Todos los documentos tratan el banco como **un
acontecimiento unico** que valida V9.0 entera de un salto desde V8.4. Ninguno pregunta que pasa **la
manana siguiente cuando falle** —y va a fallar en algo: `Y2` esta medido muerto y N-42 es una
regresion abierta *en banco*—. Ni cual es el delta minimo que paga la deuda de calle: **el arreglo de
los 25 s podria ser una carga de una constante sobre la V8.4 que ya esta probada**, sin esperar a
certificar 14.000 lineas. *«El proyecto sabe hacer trinquetes en sus packs; no ha pensado su despliegue
como trinquete.»*

**(c)** Cuando haya banco, **la primera comprobacion del protocolo debe ser el checksum sobre bytes
reales** app <-> ESP32 <-> STM32: es la unica pieza nueva capaz de dejar al tecnico **sin herramienta
de diagnostico justo el dia que la estrena**.

---

> **La frase, copiada entera:** *«El proyecto respondio a la auditoria arreglando todo lo que ella
> midio y nada de lo que ella dijo — el instrumento que faltaba ya ejecuta las dos puntas de verdad, y
> la tarjeta sigue sin tocarse mientras el README vuelve a publicar, veinticuatro horas despues de
> N-109 y con su acta en rojo al lado, un verde elegido a base de repetir.»*

---

### 🔴 N-112 — La compuerta ALTERNA verde y rojo sobre un arbol identico: su codigo de salida no significa nada

**Medido el 01/09, tres corridas completas seguidas sin tocar UN SOLO fichero entre ellas:**

```
pasada 1:  RESUMEN: 16 PASS | 1 FALLA | 0 ABORTADO   ->  exit 1
pasada 2:  RESUMEN: 17 PASS | 0 FALLA | 0 ABORTADO   ->  exit 0
pasada 3:  RESUMEN: 16 PASS | 1 FALLA | 0 ABORTADO   ->  exit 1
```

**El mecanismo, observado directamente en el acta.** `documentos_01_cifras_del_acta` compara lo que
publican README y ESTADO contra la fila `banco por packs` del acta **ANTERIOR**. Cuando el banco sale
en verde esa fila trae sus cifras —`RESUMEN: 829/829 comprobaciones | packs: 59 PASS...`— y el pack
puede anclar el recuento: son **dos comprobaciones mas**. Cuando el banco sale en rojo, la fila del
acta guarda **el texto del fallo en vez de las cifras**, `_cifra()` no encuentra el patron
`x/y comprobaciones`, y esas dos comprobaciones **desaparecen**.

Total en verde **829**, total en rojo **827**. Publicar cualquiera de los dos hace fallar la corrida
siguiente, que restaura el otro. **No hay ninguna cifra publicable que estabilice esto**: la busque
iterando, y el punto fijo del banco a solas (827) no es el punto fijo bajo la compuerta (829).

> 🔴 **Por que esto es peor que un FALLA:** el proyecto ya tiene escrito que *«un `FALLA` permanente
> tampoco es un aviso»* y que *«un codigo de salida que jamas cambia ensena a ignorarlo»*. Esto es la
> version siguiente y mas danina: **un codigo de salida que cambia solo ensena a re-correr hasta que
> salga verde**, y ese habito destruye la compuerta entera — porque el `0` que se acaba publicando es,
> literalmente, el que se ha elegido a base de repetir.
>
> Y esta noche pasó exactamente eso: estuve a punto de correr una vez mas «para dejar el acta en
> verde». Eso habria sido el error, no el arreglo.

**La regla que queda, y generaliza a cualquier instrumento del banco:**

> **El NUMERO de comprobaciones que emite un pack no puede depender de su propio veredicto.** Si una
> rama de `if` llama a `verificar()` dos veces y la otra a `reportar()`, el total se mueve con el
> resultado, y un total que se mueve solo no se puede publicar — que es justo lo que §3 exige.

**Como se arregla** (no se hizo el 01/09: son las 2 de la manana y tocar el pack de cifras a esta hora
es como se meten los defectos que este fichero documenta): las dos comprobaciones ancladas se emiten
**siempre**, y cuando el acta no trae el par se emiten en **FALLA** diciendo que el acta no lo trae —
que es una afirmacion verdadera y util— en vez de no emitirse. Y su `control_negativo` tiene que
ejercer las dos ramas, porque es justo lo que hoy no hace.

**Mientras tanto: las cifras de la tabla de §1 SI estan medidas y no oscilan** —flash, packs,
arneses, app—. Lo que no vale nada hasta arreglar esto es **el codigo de salida**.

---

### 🟠 N-110 — Dos barreras de la app que no vigila nadie, salidas de invertir el arnes de DOM

**Las encontro el agente que invirtio `test_dom_execution.js`, y las verifique yo mismo antes de
escribirlas aqui** (§4: un informe no es una medida). **Ninguna de las dos se arreglo el 31/08** — se
dejan medidas y abiertas, porque tocar `app.js` obliga a recompilar la APK y rehacer el paquete.

#### 1. La app **no valida el checksum** de lo que pinta

```
app.js:1420  parseNmeaTelemetry()  ->  line.split('*')[0]      el CRC se tira sin mirarlo
js/nmea_parser.js:27  validarTrama()   4 definiciones en disco, CERO llamadores
```

Es **la forma exacta de N-73**: una funcion declarada, documentada y sin un solo llamador. Y hay
prueba dura de que no se mira: la trama de ejemplo del arnes lleva `*5F` **desde siempre** y su
checksum real es `*04` — la app la pinta como verdad. Sobre radio a **2,4 kbps**, eso significa que
un `$STATUS` corrompido en vuelo se dibuja como el estado del cruce. *(Ya estaba medido en
`simulador_puente_esp32.py:1295`; lo que es nuevo es que el llamador existe y esta ahi al lado.)*

#### 2. El teclado del PIN **acepta pulsaciones con el modal cerrado**

Los handlers de `.pin-btn[data-key]` (`app.js:2099`) **no consultan si `pin-modal` esta activo**, y
`validatePin()` pone `state.pinVerificado = true` igual. Hoy nadie llega —el modal esta oculto en el
navegador—, pero **es una barrera cuyo estado se puede armar sin abrir la barrera**.

> **Y no es teorico: esto enmascaraba parte del fallo que se acaba de arreglar.** El arnes tecleaba
> `1234` sobre un modal que la guarda de punta nunca abrio, **se autorizaba solo**, y por eso
> `SOLICITAR_PASO` seguia dando `[OK]` mientras seis lineas de al lado caian.

El arreglo es una linea al principio del handler:
`if (!pinModal.classList.contains('active')) return;`

#### 3. Menor

`avisarOtraPunta()` escribe el evento con `state.node || '?'`. Si la orden se pulsa **antes del primer
`$STATUS`**, el operario lee *"ahora mismo hay un ? al otro lado"*. Es honesto, pero se lee como un
error de la app.

---

### 🔴 N-109 — Auditoria externa: el proceso no puede verse a si mismo caido, y el banco dejo de ser un bloqueo para ser una coartada

**El responsable pidio una auditoria de fuera del marco.** Todo lo de esta sesion se habia medido
**contra las reglas del propio proyecto**; nadie habia preguntado si las reglas eran las correctas.
Lo que sigue lo verifico punto por punto el orquestador antes de escribirlo.

#### 1. El repositorio publicaba un verde que su propia acta desmentia

```
evidencia/2026-08-31_compuerta.txt   (HEAD fa66710)
    RESUMEN: 15 PASS | 1 FALLA | 1 ABORTADO
    ABORTADO  banco por packs

ESTADO.md:11 publica, CITANDO ESA MISMA ACTA
    "15 PASS | 0 FALLA | 0 ABORTADO (Exit code: 0) ... 445/445 en 39 packs"

packs en disco: 57
```

Tres incumplimientos a la vez: **la cifra no se copio del acta** (§3 literal); **el instrumento
central estaba ABORTADO** —detras de un `ABORTADO` en *banco por packs* no corre **ninguno** de los
57—; y 🔴 **`documentos_01_cifras_del_acta`, el pack que caza justo esta deriva, VIVE DENTRO del
banco abortado**: el unico guardian de las cifras estaba apagado **el dia que las cifras mentian**.

> **LA LECCION, y es la respuesta a "que no puede ver este proceso por construccion":
> EL PROCESO NO PUEDE VERSE A SI MISMO CAIDO.** Cuando el instrumento que valida las cifras es parte
> de lo que aborta, el `0` publicado deja de tener nada detras y **no hay meta-instrumento**. Un
> `ABORTADO` en *banco por packs* no es una fila mas del acta: **es todas las filas**.

**Verificado despues:** aquel `ABORTADO` era transitorio —un agente corrio la compuerta con
`lcd.cpp` a medio escribir— y hoy `enlace_01` da `34/34`. **El hallazgo no se retira por eso.** Era
cierto cuando se midio, y la discrepancia siguio en pie horas. Se deja escrito **precisamente porque
la tentacion era regenerar el acta a verde y no contarlo**, y eso habria sido pasar por encima de la
regla propia sin dejar rastro.

#### 2. "Medir en vez de entregar" — la respuesta es SI

```
37 commits en 48 h · 57 packs · 4 arneses C++ · 3 simuladores.  NINGUNO toca una tarjeta.
En campo corre e303485 (V8.4, 31/07), que NO ES ALCANZABLE desde esta rama.
El arreglo del "se va a ambar a los 12 s" esta escrito desde el 27/08.
```

> *"El banco lleva siendo EL bloqueante desde el 31/07 sin moverse, y eso ha dejado de ser un bloqueo
> para convertirse en una condicion permanente alrededor de la cual se ha construido una industria de
> sustitucion. Cada auditoria interna encuentra defectos reales, pero todos son de escritorio, y cada
> uno genera packs, que generan actas, que generan cifras que cuadrar. Es un bucle que se alimenta
> solo y produce la sensacion de progreso sin acercar el unico entregable que importa: una tarjeta
> cargada y ciclando."*

**El indicador que lo delata:** **N-42 —que el Modo Automatico no mueve las luces en banco— lleva
abierta desde ANTES de toda esta arquitectura y sigue sin tocarse**, mientras el instrumental se
refactoriza una y otra vez.

> **El rigor se ha aplicado solo donde no cuesta una sesion de banco.** Un mes sin cargar una tarjeta,
> con un defecto de campo conocido y arreglado en el disco, **no es rigor: es evitacion.**

#### 3. El fallo del que nadie hablaba: el Degradado da VERDE por un reloj que esta muerto

Encadena hechos que el proyecto tenia medidos **por separado** y que nadie habia juntado:

```
modo_degradado.cpp:155   exige reloj_enHora(); sin el -> MDG_FALTA_HORA.
                         Toda la fase sale de reloj_segundosDelDia()
N-17 / N-37              el cristal Y2 NO OSCILA en las tarjetas reales (banco, 01/08)
```

**Sobre el hardware de campo, el Modo Degradado —la funcionalidad estrella de V8.5 a V8.7, con sus
packs y sus arneses— podria no poder ni entrarse.** Nadie lo ha comprobado porque nadie ha llevado el
banco.

Y el plan lo resuelve **colgando el reloj del ESP32**: se hace depender una **funcion de vida** —el
todo-rojo que evita verde-contra-verde— de un accesorio que **no tenia watchdog**, cuyo firmware **se
escribio hoy y jamas se ha probado**, y que cuelga de **un unico cable serie que ningun pack vigilaba
hasta ayer**.

#### 4. Y el invariante que evitaria un choque frontal lo sostiene una copia a mano

**Verificado por el orquestador, con el matiz que la auditoria no tenia:**

| instrumento | que hace de verdad |
|---|---|
| `barrera_02_dos_puntas` | comprueba que el enclavamiento sea **textualmente el mismo** en las dos puntas. Buen proxy, **no un ejercicio** |
| `Validacion_Automatico` | compila C++ **real**... **solo del Maestro**. `CLAUDE.md` §8 lo dice literal |
| `simulador_sistema_v7_6` prueba 5 | **si ejercita las dos puntas** — en un **modelo de Python escrito a mano** |

> **La conclusion aguanta: NINGUN instrumento ejecuta el C++ real de las dos puntas a la vez y
> comprueba que nunca dan verde las dos.** Lo unico que cierra ese lazo es una copia del firmware
> escrita a mano, que es justo lo que §8 avisa que no prueba el codigo.
>
> Y el escenario de salida asimetrica —una punta en verde, otra en ambar—, que `OPTIMIZACIONES.md:422`
> da por **riesgo residual aceptado**, ocurre con **un solo microcorte** que reinicie una unidad. En un
> cierre de carril, eso es un choque frontal.

#### 5. La seguridad no esta clasificada como seguridad

**MEDIDO:** el PIN es `1234` **literal en claro** (`Maestro/src/bluetooth.cpp:166`, `Esclavo:164`)
sobre Bluetooth SPP sin cifrar. Y **las ordenes mas peligrosas no lo piden**: `FORZAR_ROJO`,
`SET_MODO:MENU` y `SET_MODO:ALCANCE` en el Maestro; `AMBAR_EMERGENCIA` y `FORZAR_ROJO` en el Esclavo.

**Cualquiera con la app —o un terminal serie— a distancia de radio puede parar el trafico o cambiar el
modo.** Esta como fila del contrato de bytes; **no esta elevado a riesgo de seguridad, y lo es.**

#### 6. La arquitectura: una cadena de apanos racional, con una cuenta sin hacer

La restriccion es real y, dado eso, retirar funciones y colgar un ESP32 **es defendible sin
presupuesto para respin**. **Lo que no esta en la cuenta:** cada apano anade un eslabon —STM32 ->
serie -> ESP32 -> telefono -> BT— y cada eslabon es un punto de fallo nuevo. Seguir por este camino
son N funciones nuevas de firmware mas **16 instrumentos que hacen falta antes de la primera linea**,
para recuperar lo que un pin de sobra habria dado gratis.

> **La comparacion honesta —"apano barato ahora" contra "respin caro una vez"— NO ESTA HECHA EN NINGUN
> DOCUMENTO**, y deberia: el apano ya lleva un mes y sigue sin tocar banco.

#### 7. Lo que la auditoria dice que esta BIEN

La disciplina de **ver caer el instrumento con el defecto inyectado** (§8.bis) es *"genuinamente buena
ingenieria de test y rara de ver"*. El razonamiento de por que el Degradado es manual, *"correcto y
bien argumentado"*. Los despachadores `$ACK`/`$ERR` del Maestro, *"un molde honesto"*. Y la autocritica
N-94 a N-108, *"de un nivel poco comun"* — con el pero: *"ven de escritorio y no cargan la tarjeta"*.

#### 8. Donde el rigor cuesta mas de lo que compra

**767 comprobaciones que reimplementan el firmware en Python** son, como §8 admite, *"una segunda copia
del firmware escrita a mano"*. Ver caer cada una es correcto; **la CANTIDAD es donde el coste supera la
compra**: cada pack es superficie que mantener sincronizada, **y ya rompio el arranque del banco**. El
exceso concreto: *"escribir 9 packs del ESP32 y refactorizar el instrumental ANTES de haber cargado
nunca la tarjeta que valida la premisa de todo."*

---

> **LA FRASE QUE RESUME LA AUDITORIA, copiada entera porque suavizarla seria el error que denuncia:**
>
> *"El proyecto ha perfeccionado el arte de medir en PC hasta el punto de que la medicion se ha vuelto
> el trabajo, mientras el firmware de la calle lleva un mes con un defecto conocido cuyo arreglo esta
> escrito y sin subir — y en este preciso momento el acta de record esta en rojo con el banco ABORTADO
> mientras README y ESTADO publican exit-0 verde. **Verde no es entregable; hoy, ademas, el verde no es
> ni verdadero.**"*

**LO QUE CAMBIA A PARTIR DE AQUI**, y lo aporto el responsable en la misma sesion: con **tres cables y
un USB** se monta el enlace ESP32 <-> STM32 **en una mesa, hoy, sin comprar nada** — sin la placa, sin
la fuente, sin el reloj. **La placa bloquea DESPLEGAR, no PROBAR.** Y esa mesa contesta de una vez lo
que ningun pack puede: que el enlace existe, que el SPP empareja, cuanto tarda el ESP32 en arrancar, y
**de paso valida los 25 s que arreglan el sintoma que hoy se sufre en la calle**.

---

### 🔴 N-108 — El enlace no deja rastro de como se cayo, y el umbral que lo arreglaba lleva un mes sin subir a campo

**Lo aporto el responsable desde el campo el 31/08, y confirma N-71 POR EL OTRO LADO:**

> *"El problema de desconexion es no saber cuanto se va cuando se va, y por que se va. Se va a ambar
> a los 12 segundos de desconexion, y ese parametro toca alargarlo un poco mas, porque se va a
> ambar por nada."*

#### Los 12 s: ya esta arreglado, y ese es el problema

**MEDIDO:** `SFTY6_SILENCIO_MS = 25000UL` en `protocolo.h:149` de las dos puntas... **pero eso es la
rama**. El equipo de la calle es la **V8.4, `e303485`, del 31/07**, y ese commit **ni siquiera es
alcanzable desde esta rama** (`git merge-base --is-ancestor` -> no). **En el poste sigue habiendo
12 s**, y el arreglo lleva desde el **27/08** escrito sin poder subir.

#### Y el "por nada" es LITERAL — el equipo se rendia antes de terminar de intentarlo

El comentario del propio firmware (`protocolo.h:120-135`) describe el sintoma sin haberlo visto:

```
coordinador.cpp reintenta 5 veces con TIMEOUT_ACK_MS = 3500 ms. El peor caso
son 3 + 5 x 3,56 = 20,8 s.

Con el techo en 12 s, el ambar por orfandad saltaba sobre el segundo o tercer
reintento. Los reintentos 4 y 5 NO PODIAN EJECUTARSE NUNCA.
```

Un enlace que se habria recuperado en el reintento 4 **nunca llegaba ahi**. Y el detalle de como se
colo: el comentario viejo decia *"fallo tras 5 reintentos (12.5 s)"*, y esa cuenta venia de un
`TIMEOUT_ACK_MS` de 2500 ms **que dejo de existir el 31/07**. Alguien cambio el numero y el
comentario se quedo describiendo un equipo que ya no existia.

> **Esto le cambia el sentido a la sesion de banco.** No es solo validar lo nuevo: **el sintoma que
> el responsable sufre hoy se arregla con lo que ya esta escrito**, y lleva un mes sin poder subir.

#### El rastro de la caida: la mitad existe y la otra esta inventada

**MEDIDO, lo que el Maestro SI mide de verdad:**

```
coordinador.cpp:845   coordinador_calidadEnlace()   ventana deslizante de los ULTIMOS
                      10 LATIDOS, uno cada 3 s -> "% de los ultimos 30 segundos"
coordinador.cpp:857   coordinador_tiempoRespuestaMs()   media exponencial real del RTT
coordinador.cpp:861   coordinador_latidosSinRespuesta()  cuantos seguidos sin contestar
```

**Y lo que el Esclavo INVENTA** (`Esclavo/src/bluetooth.cpp:328`):

```c
"...,MODO:SUBORDINADO,ESTADO:%s,T:%lu,RF:98%%,RTT:85ms,BAT:12.6,HORA:%s"
                                       ^^^^^^^^^^^^^^^^^^^^^^^  literales
```

**De las dos puntas del enlace, solo una tiene dato — y la que se queda sin radio a 5 m de altura es
justo la otra.**

**Y nadie lo guarda.** El `RF:` solo viaja en el `$STATUS` mientras alguien mira el telefono. En el
momento de la caida el tecnico no esta delante, y el numero que le habria dicho *"venia bajando
desde el 60 %"* ya no existe. El `$ALARM,...,CAUSA:SILENCIO_25000ms` dice **que** se cayo, no
**desde donde**.

#### La decision, tomada el 31/08

**No se mide potencia.** Nada de RSSI: pedirselo al modulo de radio es otro proyecto y hoy no lo hace
nadie. **Se llega a latidos y se indica visualmente** — que es lo que ya existe en el Maestro.

Lo que falta, y va con la Fase 4:

1. **El Esclavo deja de inventar `RF`/`RTT`**: o los mide, o el campo se retira. *Un campo que no se
   mide se retira o se marca; no se deja con aspecto de medida.*
2. **La alarma de caida lleva el ultimo tramo**: `RF` de la ventana, `RTT` medio y latidos seguidos
   sin respuesta. **Eso es la pregunta del responsable contestada.**
3. **Un `$EVENT` en cada cambio de estado del enlace**, con su valor.
4. **Que la app lo persista** — ya recibe `$EVENT` y hoy no lo guarda.

**Limitacion escrita, para que nadie espere de mas:** esto mide **latidos contestados**, no potencia.
Distingue *"el enlace se degrada"* de *"el enlace va bien"*, y **no** dice dBm ni si la culpa es de la
antena, del cable o de un obstaculo nuevo.

> **LA LECCION: un sintoma de campo y una medida de escritorio pueden ser el mismo defecto visto por
> dos lados, y ninguno de los dos lo demuestra solo.** N-71 salio de cruzar tres constantes en un
> fichero; esto salio de que a alguien se le fuera el cruce a ambar sin motivo aparente. **Los dos
> decian lo mismo y ninguno lo sabia.** Cuando el campo reporta un sintoma, el primer sitio donde
> mirar es si alguna medida vieja ya lo predijo — y al reves.

---

### 🟢 N-107 — BLQ-1 cerrado: es un `ESP32-WROOM-32` clasico, hay SPP · **CERRADO 31/08**

**La ficha del modulo comprado**, aportada por el responsable, cierra la fila mas bloqueante del
proyecto con **tres confirmaciones independientes**:

```
Microcontrolador ...  ESP32-WROOM-32
CPU ................  Tensilica Xtensa 32-bit LX6, DUAL-CORE
                      el S3 es LX7 y el C3 es RISC-V -> no es ninguno de los dos
Bluetooth ..........  v4.2 BR/EDR and Bluetooth Low Energy (BLE)
                             ^^^^^^ Bluetooth Clasico
```

`BR/EDR` es exactamente el perfil que necesita `createRfcommSocketToServiceRecord`. **La app conecta
sin tocar el transporte**, y el apartado 1 del Manual 10 —congelado por escrito— **no se reabre**.

Y la ficha resolvio de paso dos cosas abiertas: la alimentacion queda en
**`12 V -> DC-DC conmutado -> 5 V -> VIN`** (entrada recomendada 5 V, limite 5,5, con regulador a
bordo), y las E/S a **3,3 V** confirman que el enlace con el STM32 va directo, sin adaptar niveles.

#### Lo que se hizo mal por el camino, que es lo que hay que guardar

**El responsable dijo dos veces que el modulo "ya tiene Bluetooth integrado", y las dos veces se le
contesto con la misma explicacion en vez de con una medida.** La afirmacion era **cierta**; lo que
faltaba era distinguir `BR/EDR` de `BLE`. Pero la forma de resolverlo no era repetir la distincion:
era **buscar el dato**.

Y el dato **no exigia el modulo en la mano**. Se exigio durante toda la sesion *"la serigrafia del
blindaje, 30 segundos"*, cuando **la ficha tecnica del articulo comprado ya lo declaraba**. El
bloqueo se mantuvo mas tiempo del necesario **por no haber preguntado por la referencia de compra**,
que es un dato que el responsable tenia a mano desde el principio.

> **LA LECCION: antes de declarar algo bloqueado por una medida fisica, censa que fuentes escritas
> pueden responderlo ya.** Una serigrafia, una ficha de compra, una factura y un `esptool chip_id`
> contestan la misma pregunta con costes muy distintos, y **la mas cara no es la primera que hay que
> pedir**. Un bloqueo que se puede levantar leyendo no es un bloqueo: es una consulta pendiente.
>
> Y su corolario, que es de trato: **cuando alguien insiste en un hecho que resulta ser cierto,
> repetir la objecion no lo convierte en falso.** La segunda vez que se oye la misma afirmacion es
> la senal de ir a medir, no de explicar mejor.

#### Lo que sigue abierto, y es mucho menor

**Estas NodeMCU vienen en 30 y en 38 pines, con anchos distintos**, y la placa portadora lleva
hembrillas, no la huella del `WROOM-32` —el modulo es de formato protoboard, asi que va enchufado y
es reemplazable sin soldador—. Contar pines y medir el ancho con pie de rey **antes de fabricar**.
No bloquea firmware.

---

### 🔴 N-105 — Cuatro documentos mandan cablear camaras sobre pines que NO son entradas de camara, y uno deja que el trafico cambie el modo del semaforo solo

**Lo encontro la pasada de coherencia del 31/08, y lo verifico el orquestador contra el fuente.**

#### El peor: las camaras sobre los pines del mando

**MEDIDO** — `MANUAL_USUARIO.md:66-70`, cuatro lineas, las dos puntas:

```
* Camara 1 (Aproximacion Sentido 1): Contacto seco 1A/1B en **PB9**  y GND -> Demanda Verde Maestro.
* Camara 2 (Monitoreo Obra Sentido 1): Contacto seco 1A/1B en **PB13** y GND -> Confirma flujo interno.
* Camara 3 (Aproximacion Sentido 2): Contacto seco 1A/1B en **PB9**  y GND -> Demanda Verde Esclavo.
* Camara 4 (Monitoreo Obra Sentido 2): Contacto seco 1A/1B en **PB13** y GND -> Confirma flujo interno.
```

**Contra el fuente, MEDIDO:**

```
botones.cpp:119   if (flanco[0]) mando_registrarPulso(MANDO_A);   // BOTON1 = PB9
botones.cpp:120   if (flanco[1]) mando_registrarPulso(MANDO_B);   // BOTON2 = PB13
mando.cpp:38      static const unsigned long VENTANA_TRIPLE_MS = 12000;
mando.cpp:241-248 A.A.A -> ACC_AUTOMATICO   ·   B.B.B -> ACC_AMBAR
mando.cpp:129-132 case ACC_AMBAR: ambarLocal = true;
```

> **`PB9` y `PB13` son `MANDO_A` y `MANDO_B`.** Una camara enchufada ahi entrega pulsos, y **tres
> pulsos dentro de la ventana de 12 s componen una secuencia del mando**: en `PB9`, `A·A·A` mete el
> equipo en Automatico; en `PB13`, `B·B·B` lo manda a ambar **y arma `ambarLocal`**, que ademas veta
> las ordenes de radio. **El trafico cambiaria el modo del semaforo solo**, sin que nadie lo pida y
> sin que nada lo registre como orden.

**Y hay un segundo error encima del primero, que lo tapa:** el manual dice *"y `GND`"*. El camino de
camara es `pinMode(INPUT)` pelado y **activo en ALTO** (`modo_inteligente.cpp:46`, `:25`), con la
bornera sacando el pin junto a 3,3 V. Cableado a masa, **la camara no dispara nunca** — asi que un
ensayo de taller la aprobaria sin ver el defecto de arriba, y este aparece el dia que alguien
"arregla" el cableado.

#### Los otros tres

| `fichero:linea` | manda cablear | lo que hay |
|---|---|---|
| `04_Manuales/MANUAL_INSTALACION_RELOJ_DS3231.md:43-44`, `:56-57`, `:89`, `:121` | I2C del `DS3231` a `PB0` (SDA) y `PB8` (SCL) | **es la SEGUNDA COPIA del defecto del Manual 11.** `PB0` es `CAM_DEMANDA_PIN` y `PB8` es `LED_TESTIGO` |
| `MANUAL_HARDWARE.md:63`, `:66` | Camara 2 a `PB8`, *"Entrada Libre"* | `PB8` no es entrada: es salida a LED por `R16` 1 K |
| `05_Funcional/9_Manual_Parametrizacion_Camara_IA.md:64`, `:168` | bornera `1A`/`1B` a `PB0` **y `GND`** | mismo error de polaridad: `R64` es pull-**down**, la camara no disparara |

#### Por que aparecio ahora, que es la parte reutilizable

**El Manual 11 se arreglo esta misma manana** (`e1d3720`) y esta copia de `04_Manuales/` **siguio
intacta**. Se arreglo el fichero que alguien nombro, no la propiedad. El censo que habria encontrado
las dos es `grep` del pin, no del nombre del documento.

Y la segunda mitad, que es mas incomoda:

> **La decision de conservar el mando (N-104) convirtio cuatro documentos viejos en un peligro
> vial.** Mientras el plan era retirar los cuatro pulsadores, `PB9`/`PB13` iban a quedar sin dueño y
> un manual que mandara camaras ahi era solo un error de papel. Al conservar A y B, ese mismo texto
> pasa a describir un cableado que **compone ordenes de mando con el trafico**.
>
> **LA LECCION: una decision no solo cambia lo que se construye, cambia lo que SIGNIFICAN los
> documentos que ya estaban escritos.** Al cerrar una decision hay que censar que documentos hablan
> de los pines que toca — y el censo es `grep` del pin, no lectura del indice.

Y el hueco que lo dejo llegar hasta aqui: `documentos_01_cifras_del_acta` **solo vigila `README.md` y
`ESTADO.md`** (MEDIDO, `:224-225`). `MANUAL_HARDWARE.md`, `MANUAL_USUARIO.md` y `CERTIFICACION_SW.md`
**no los parsea nadie**, y es justo donde estan todas las cifras malas. Un `ABORTADO` grita; un hueco
no.

---

### 🔴 N-106 — El ambar de emergencia de la app NO saca al Esclavo del Modo Degradado

**MEDIDO por lectura del fuente. La consecuencia exacta esta razonada, no ejecutada: se marca como
tal y se cierra con el arnes, no con esta nota.**

```
grep -c "degradado" Esclavo/src/bluetooth.cpp   ->   0

Esclavo/src/bluetooth.cpp:130-136
    if (strcmp(cmd, "CMD:AMBAR_EMERGENCIA") == 0) {
      semaforo_iniciarFallo();
      ambarEmergencia = true;
      enviarTramaConCrc("$ACK,CMD:AMBAR_EMERGENCIA,RESULT:OK");
      ...
    }
```

**Quien SI sabe salir del Degradado, censado:**

```
degradado_salir()  <-  Esclavo/src/main.cpp:385   (la puerta automatica)
                   <-  Esclavo/src/mando.cpp:121  (el mando)
                   <-  Esclavo/src/mando.cpp:138  (el mando)
                   <-  Esclavo/src/menu.cpp:215   (la pantalla)
```

**`bluetooth.cpp` no esta en esa lista.** Y `degradado_actualizar()` corre en cada vuelta
(`main.cpp:363`), con `degradado_gobiernaLuz()` decidiendo quien manda sobre la luz (`:383`, `:555`,
`:619`).

> **Es decir: el mando puede sacar al Esclavo del Degradado, la pantalla puede, y la app NO.** El
> `$ACK,RESULT:OK` se manda igual. Es el patron de §6 —un `ACK` que no depende de lo que la llamada
> consiguio— pero esta vez el defecto no esta en el que contesta: **esta en que falta la llamada**.

**Y lo que lo vuelve grave es la conjuncion con las fases:**

- La **pantalla se retira** en la Fase 3, y con ella `menu.cpp:215`.
- Si el mando se hubiera retirado tambien —que era el plan hasta el 31/08— **no habria quedado
  NINGUNA via externa para sacar al Esclavo del Degradado**. Solo la puerta automatica de
  `main.cpp:385`.
- La decision de N-104 lo evito **por accidente**, no porque nadie lo hubiera visto.

**Lo que hay que hacer, y en este orden:** primero el pack que lo ejerza —que hoy no existe: ningun
instrumento comprueba que el ambar de la app saque del Degradado—, verlo **fallar** sobre el firmware
de hoy (§8.bis), y solo entonces añadir la llamada. Al reves, el arreglo entra sin testigo.

Y `05_Funcional/8_Procedimiento_Modo_Degradado.md:474` **llego a la conclusion correcta citando un
comando que no existe**: acerto por el camino equivocado. Se reescribe, no se borra.

> **LA LECCION: un censo de llamadores tiene dos direcciones, y la segunda casi nunca se hace.**
> Preguntar *"¿quien llama a esta funcion?"* encuentra codigo muerto. Preguntar *"¿quien DEBERIA
> llamarla y no lo hace?"* encuentra agujeros — y esa pregunta solo se puede hacer con la lista de
> los que si llaman delante. `mando.cpp` y `menu.cpp` la llaman; `bluetooth.cpp`, que es la unica
> interfaz que va a quedar, no.

---

### 🟢 N-104 — El mando se queda en A y B, las camaras entran por C y D, y la pantalla se fue porque NO HABIA PINES · **DECIDIDO 31/08**

**El porqué de toda la arquitectura del 28/08 no estaba escrito en ningun documento del repositorio.**
Los manuales explican *que* se retira; ninguno decia *por que no habia alternativa*. Lo aporto el
responsable el 31/08 y va aqui, porque sin ello dentro de tres meses alguien vuelve a proponer la
placa de expansion y nadie recuerda que se probo y no entraba.

**La razon de fondo, que no es de firmware:** a diferencia del proyecto anterior, **este PCB no
permite ampliacion**. Lo que se desarrollo para ampliarlo **no era fisicamente realizable**: exigia
soldar sobre una placa que no lo admite bien. Asi que no habia de donde sacar pines, y la unica
fuente disponible era retirar funciones.

#### La cuenta, MEDIDA

```
La pantalla ocupaba CINCO pines   (Maestro/include/pines.h:85-89)
    LCD_SCLK PB3 · LCD_CS PB4 · LCD_SID PB5 · LCD_PSB PB6 · LCD_RST PB7

Los cuatro pulsadores ocupan CUATRO (pines.h:92-95)
    BOTON1 PB9 · BOTON2 PB13 · BOTON3 PB14 · BOTON4 PB15

Lo que hacia falta y no cabia:
    Bluetooth por USART1 remapeado ...  PB6  PB7    <- los suelta la pantalla
    Reloj (en el plan de entonces) ...  PB3  PB4    <- los suelta la pantalla
    Segunda camara ..................   PB14 PB15   <- los sueltan los botones C y D
```

> **La pantalla no se retiro por la pantalla: se retiro porque el Bluetooth necesitaba exactamente
> dos de los cinco pines que ella tenia, y no habia otros.** Por eso N-76 fue lo primero de aquella
> sesion y todo lo demas cayo detras: con la LCD puesta, `PB6`/`PB7` estaban ocupados, no habia
> `USART1`, y sin `USART1` no hay Bluetooth.

**Y una consecuencia que ABARATA la cuenta, posterior a esa decision:** al mudarse el `DS3231` al
ESP32 (`GPIO21`/`GPIO22`, con pila propia), **el reloj dejo de costar pines del STM32**. `PB3`, `PB4`
y `PB5` quedan LIBRES al ejecutar la Fase 3 — margen que el equipo no tenia. Hoy siguen ocupados:
`lcd_setup()` se llama todavia (`Maestro/src/main.cpp:46`, `Esclavo/src/main.cpp:212`) y u8g2 retiene
los tres (`lcd.cpp:29`). `PB6` ya esta suelto desde N-76, que le paso `U8X8_PIN_NONE`.

#### El reparto de los cuatro pulsadores: el mando NO los usa todos

Esto estaba sin medir, y decidia si el mando podia convivir con las camaras.

```
botones.cpp:119   if (flanco[0]) mando_registrarPulso(MANDO_A);   // BOTON1 = PB9  = J16 p5
botones.cpp:120   if (flanco[1]) mando_registrarPulso(MANDO_B);   // BOTON2 = PB13 = J16 p8
botones.cpp:131   bool botonAceptar()  { return consumir(2); }    // BOTON3 = PB14 = J16 p10
botones.cpp:132   bool botonCancelar() { return consumir(3); }    // BOTON4 = PB15 = J16 p12

grep -n "BOTON[1-4]" Maestro/src/mando.cpp   ->   CERO coincidencias
```

**MEDIDO: el mando de reles vive entero en A y B.** No conoce pines —trabaja sobre `MANDO_A` /
`MANDO_B`, que solo alimentan los botones 1 y 2—, y **no toca `PB14` ni `PB15`**, que son Aceptar y
Cancelar del menu de la pantalla que se retira.

#### Por que A y B, y no solo A

Se evaluo dejar cableado un solo canal. **Se rechaza, y la razon es una medida, no una preferencia:**

```
Esclavo/src/mando.cpp:241-242   A·A·A     -> ACC_AUTOMATICO
Esclavo/src/mando.cpp:246-248   B·B·B     -> ACC_AMBAR
Esclavo/src/mando.cpp:219-220   A·B·A·B   -> ACC_DEGRADADO

Esclavo/src/mando.cpp:129-132
    case ACC_AMBAR:
      // Sin condiciones y desde cualquier estado. Es la regla que impide que nadie
      // quede atrapado con un semaforo en estado raro a 5 m de altura.
      ambarLocal = true;      <- EL UNICO sitio donde se pone a true
```

`ambarLocal` es lo que devuelve `mando_ambarLocal()` (`Esclavo/src/mando.cpp:103`), y de el cuelgan
los tres vetos negados del Esclavo (`Esclavo/src/main.cpp:406`, `:416`, `:540`).

> **Sin el canal `B`, `ambarLocal` no se arma jamas**, los tres `if` se vuelven siempre-verdaderos y
> una orden de radio puede sacar al Esclavo de un ambar que un operario dejo puesto a proposito. Es
> **N-79 exacto** —SFTY-21 desapareciendo por sustraccion— solo que por dejar sin cablear un canal en
> vez de por borrar `mando.cpp`.

Y `A`-solo no compra nada: liberaria **tres** entradas cuando solo hacen falta **dos**.

#### La decision, y lo que cierra

**DECIDIDO el 31/08 por el responsable: se conservan `A` (`PB9`) y `B` (`PB13`); se retiran `C`
(`PB14`) y `D` (`PB15`) y esos dos pines pasan a las camaras.**

| queda cerrado | por que |
|---|---|
| **N-79** · el veto que se borraba por sustraccion | `mando_ambarLocal()` se sigue armando por `B·B·B`. SFTY-21 no desaparece |
| **§3.3** del Manual 17 · sin superficie de mando si el ESP32 se cuelga | es la opcion 3 de su tabla —*dejar el mando*— y sale gratis |
| **N-101** · `Validacion_Automatico` abortando | `mando.cpp` **no se borra**, asi que `compilar.ps1:64` sigue enlazando y SFTY-5 conserva su unico instrumento |
| parte de **N-103** | `maestro_01_mando` conserva su sujeto: lo que se pierde baja de ~32 comprobaciones a ~17 —solo lo que muere con la pantalla— |

**Lo que NO cierra, y sigue igual de vivo:**

- 🔴 **La polaridad de `PB14`/`PB15` sigue en contradiccion medida** (N-84). El netlist tiene
  pull-**down** de 10 k con 3,3 V al lado —activo en ALTO— y `botones.cpp:19` lee `== LOW`. **No se
  cablea camara hasta la medida M3**, con multimetro.
- 🔴 **El orden sigue siendo asimetrico** (CLAUDE.md §9.bis): `PB14` es `botonAceptar()`, el que
  EJECUTA. El firmware nuevo tiene que estar **CARGADO EN LA TARJETA** antes de que nadie enchufe
  nada en `J16`.
- 🟠 **El receptor del mando nunca se compro** (Manual 17 §2.7). Lo que se conserva hoy es el
  **firmware y el veto**; para tener mando fisico hay que comprarlo. No contarlo como red de §3.3
  hasta entonces.

> **LA LECCION: un reparto de pines no se decide leyendo los nombres de los `#define`, se decide
> midiendo quien los consume.** Cuatro pulsadores parecian cuatro entradas del mando, y el mando solo
> usaba dos: la diferencia entre creer eso y medirlo son las dos entradas de camara que hacian falta
> y el veto de una regla de seguridad. Y su corolario: **antes de retirar un canal, busca que bandera
> deja de armarse** —el `grep` que importa no es el del pin, es el del `= true`—.

---

### 🔴 N-103 — El censo del instrumental frente a la arquitectura del 28/08: ~347 de 782 comprobaciones se quedan sin sujeto, y la compuerta sabe ver lo que se BORRA pero no lo que se queda sin sujeto

**De donde sale:** del encargo del `05_Funcional/17_...md`, Anexo, punto 8 —*"los packs que ejercen
pantalla, botones y mando quedan sin sujeto. Hay que decidir uno por uno si se borran, se invierten o
se conservan (`CLAUDE.md` §8.quater), con la cuenta comparada antes y despues"*—. Esta es esa decision,
pack a pack, hecha ANTES de tocar el firmware, que es el unico momento en que sirve.

**MEDIDO, corriendo los 38 packs uno a uno** con `python 01_Firmware/Simulaciones/banco/correr.py
--pack <nombre>` y sumando: **411**, la misma cifra del acta. La suma cuadra, asi que las cifras por
pack de abajo no son estimaciones.

---

#### A · Los cuatro packs que se quedan sin sujeto ENTERO — 32 comprobaciones

| pack | hoy | que vigila | destino | por que |
|---|---|---|---|---|
| `maestro_01_mando` | **15** | las tres secuencias `A.B.A.B` / `B.B.B` / `A.A.A`, el barrido de los 254 trenes de 1 a 7 pulsos, el barrido de cadencia 100..10.000 ms, la ventana deslizante y `purgarViejos()` | **SE BORRA — menos dos** | 13 de las 15 no las puede aprobar **ni suspender** ningun firmware sin reles: no hay quien genere un pulso. Es exactamente el residual del alias de `CMD_DELTA` que `CLAUDE.md` §2 mando a `reportar()`. **Pero DOS se conservan mudandolas de pack**, porque no hablan del mando: *"main.cpp llama a `semaforo_actualizar()` SIN CONDICION en el `loop()`"* y *"`modo_automatico.cpp` llama a `coordinador_actualizar()` unicamente dentro de `case CORRIENDO`"*. Esas dos cierran el fallo del cabezal a oscuras y su sitio es `maestro_05` o el arnes del automatico. **Y antes de borrar nada, N-102: ponerle la etiqueta `# EJERCE`** |
| `maestro_06_fuentes_pantalla` | **4** | que `lcd.cpp` dibuje los titulos en `u8g2_font_7x14B_tr` y que el arnes mida sus anchos con **esa misma** fuente | **SE BORRA** | existe por N-39: que el arnes y el firmware no divergan. Sin arnes de pantalla y sin `lcd.cpp` **no hay dos puntas que comparar**; la prueba aprobaria vacia, comparando nada contra nada, que es justo lo que `costura_01` avisa de si mismo para el dia de `lib/Common`. **Dejarlo es peor que borrarlo**: su `control_negativo` seguiria dando verde sobre un fosil |
| `maestro_07_menu_opciones` | **6** | que el array, los textos y la constante de cada menu digan lo mismo, y que la ultima opcion caiga en `y=61`, dentro de los 63 px | **SE BORRA** | los 64 px de alto se van con la pantalla. **La leccion no se pierde** —*"mientras el numero viva en dos sitios, alguien actualizara uno y no el otro"*— : la heredan `app_02_modos_simetricos` y el pack nuevo que ate `VERDE_MIN_MIN` a los limites de `app.js` (N75-2) |
| `esclavo_02_inhibicion_menu` | **7** | con el menu abierto, `B.B.B` no se reconoce; barrido de 85 a 95 s del regreso automatico; el cartel de rechazo caduca a los 6 s sin parar la cuenta | **SE INVIERTE** | 🔴 **es el UNICO pack etiquetado `# EJERCE SFTY-21` cuyo sujeto desaparece entero** (`:14`). La regla que protege —*"dos personas dando ordenes contrarias a la vez es peor que cualquiera de las dos ordenes"*— **no muere con el menu: se muda a dos telefonos sobre el mismo poste**, y hoy eso no lo mide nadie. Se invierte a: mientras un `AMBAR_EMERGENCIA` este puesto, una segunda sesion Bluetooth no lo revoca sin decirlo. Y su fila de `OPTIMIZACIONES.md:128` se toca **en el mismo commit**, o `documentos_02` falla |

---

#### B · Los packs con parte del sujeto fuera — el destino va por COMPROBACION, no por pack

| pack | hoy | caen | destino y por que |
|---|---|---|---|
| `esclavo_01_latch_ambar` | 7 | ~5 | **SE INVIERTE.** El barrido de 15 comandos x 4 estados de partida es de lo mejor que tiene el banco y **sigue valiendo entero**: lo que cambia es **quien arma el latch**, de `mando_ambarLocal()` a `bluetooth_ambarEmergencia()`. Las dos del Maestro cayendo a `C_FALLO` en 17,6 s no tocan el mando: **se conservan tal cual**. 🔴 Y aqui vive N-79: el veto de `Esclavo/src/main.cpp:406`, `:416`, `:540` |
| `esclavo_07_ambar_emergencia` | 16 | ~3 | **SE CONSERVA, con dos filas reescritas.** 13 de 16 son sobre nombres de comando y coherencia `$ACK`/rama —N-83— y no las toca nada. La que dice *"las 3 guardas de main.cpp que respetan el ambar del mando respetan TAMBIEN el pedido por Bluetooth"* **se invierte y sube de rango**: pasa de comparar dos vetos a ser **el unico** veto que queda, y ahi es donde hay que poner el `control_negativo` nuevo |
| `maestro_09_test_leds` | 18 | ~4 | **SE CONSERVA, rebaselinado.** Tres citan *"los cuatro caminos de la senal SFTY-21"* y `senalActiva`, el `static` que **solo pone `mando.cpp`**. No se relajan: se recuentan contra el censo nuevo. 🔴 **Relajar *"las 5 funciones que llaman a `escribirPines()` son las conocidas"* a *"las que haya"* mata el pack**, y es la tentacion exacta que §8.quater castiga |
| `maestro_03_puerta_degradado` | 19 | 1 | **SE BORRA esa una.** La ultima —*"las dos pantallas reciben un indicador de vencimiento y no solo un numero"*— lee `lcd.cpp` de las dos puntas (`packs/maestro_03_puerta_degradado.py:549-550`). Su equivalente honesto es un campo de `$STATUS` que lo diga, y eso ya es Fase 4. **Las otras 18 no se tocan** |
| `costura_06_reanudacion` | 6 | 0 o 1 | **SE CONSERVA.** Solo menciona el menu como *estado de partida* de la punta que arranca. Si `MODO_MENU` sobrevive como estado seguro sin display —que es lo que hace falta para SFTY-12— no cambia nada |
| `app_02_modos_simetricos` | 8 | 0 o 3 | **SE CONSERVA.** Tres listas incluyen el literal `MENU` de `obtenerNombreModo()`. **El pack las relee del C++ en cada corrida**, que es exactamente para lo que se escribio: si `MODO_MENU` desaparece de `ModoSistema`, se ajustan solas |
| `barrera_01_pines_de_luz` | 5 | 0 | **SE CONSERVA ENTERA.** Solo queda historico el comentario sobre los destellos del mando que *interceptan* en vez de rodear |
| `app_01_comandos` | 8 | 0 | **SE CONSERVA.** Los seis comandos de la Fase 1 los vera aparecer solo, porque lee los despachadores del C++ |
| `flash_01_lastre` | 11 | 11 | **SE INVIERTE, y es el mejor negocio de la lista.** Sus 11 comprobaciones son *"u8g2 declarado sin arrastrar I2C ni SPI"*. Retirada la pantalla, **u8g2 no debe estar**, y el pack **ya sabe expresar la forma invertida**: la fila *"Repetidor no tiene pantalla y no arrastra banderas de u8g2"* es literalmente eso. Y vigila los **~18,9 KB estimados** que la Fase 3 promete liberar en el Maestro: sin el, ese numero es una estimacion de nadie (`CLAUDE.md` §7: un delta exige medir los DOS extremos) |

---

#### C · Los tres detectores que se disparan solos — se atienden, NO se silencian

| pack | hoy | que hara | destino |
|---|---|---|---|
| `costura_10_funciones_muertas` | 13 | **FALLA.** Lleva las huerfanas conocidas escritas a mano (`packs/costura_10_funciones_muertas.py:44-68`): **15 en el Maestro, 8 en el Esclavo**. En cuanto `botones_setup()`, `lcd_*` o `menu_*` pierdan su llamador, el censo vera huerfanas **nuevas** fuera de lista | **SE CONSERVA.** 🟢 **Es el UNICO instrumento del banco que caza la Fase 2**, donde el fichero sigue en disco y solo deja de llamarse. La lista se **amplia una por una con su motivo escrito**, jamas se relaja: es un trinquete, y un trinquete que se afloja no es nada |
| `documentos_01_cifras_del_acta` | 51 | **FALLA** el dia que `411`, los `38` packs, el `271/271` de pantalla o **las 15 filas** del acta dejen de coincidir con README y `ESTADO.md` | **SE CONSERVA.** Es la comparacion de totales de `CLAUDE.md` §5, **la unica red para esta clase de deriva**, y ya ha salvado la migracion dos veces. Sus cifras se copian del acta nueva, nunca se escriben a mano |
| `documentos_02_trazabilidad_sfty` | 10 | **FALLA** si desaparece la etiqueta `# EJERCE SFTY-21` de `esclavo_02:14` sin tocar `OPTIMIZACIONES.md:128`; exige coincidencia **exacta en las dos direcciones** | **SE CONSERVA.** Etiqueta y fila van en el **mismo commit**. Ver N-102: su punto ciego es lo que nunca se etiqueto |

**Los 21 restantes no se tocan** —verificado uno a uno—: `app_03`, `barrera_02`, `barrera_03`,
`camara_01`, `costura_01`..`costura_05`, `costura_07`, `costura_08`, `costura_09`, `documentos_03`,
`esclavo_03`..`esclavo_06`, `identidad_01`, `maestro_02`, `maestro_04`, `maestro_05`, `maestro_08`.
`costura_01_contratos` se comprobo expresamente: sus 7 ficheros compartidos son `protocolo`,
`ciclo_degradado`, `respaldo` e `identidad` — **ni `lcd`, ni `menu`, ni `mando`**.

---

#### D · Los cuatro arneses de C++ real y los simuladores

| instrumento | acta | que le pasa |
|---|---|---|
| **`Validacion_LCD`** | **271/271** | **MUERE ENTERO.** `compilar.ps1` compila `lcd.cpp`, `menu.cpp`, `modo_degradado.cpp` y `modos.cpp`; el arnes incluye `lcd.h`, `menu.h` y `botones.h`. Sin `lcd.cpp` no enlaza y `compuerta.py:357` lo marca **ABORTADO**. 🔴 **No se deja abortado ni un dia** (§3.quater: N-75 entro exactamente asi, con dos instrumentos en ABORTADO y cuatro defectos detras). **Y no todo lo suyo es pantalla:** enlaza `modo_degradado.cpp`, que es SFTY-21. Si algo se rescata es un arnes nuevo que compile `modo_degradado.cpp` sin display, no las 271 |
| **`Validacion_Automatico`** | **71/71** | **ABORTA con `mando.cpp`, y se lleva el unico instrumento de SFTY-5.** Es N-101 entero |
| `Validacion_Ciclo` | 29/29 | **INTACTO.** Solo `ciclo_degradado.h`, funcion pura |
| `Validacion_Respaldo` | vivo | **INTACTO.** `respaldo.cpp` + `calcularSuma()` |
| `simulador_sistema_v7_6.py` | 20/20 | **3 de 20 se quedan sin sujeto:** PRUEBA 1 *"Menu con comunicacion -> ambos en ROJO FIJO"*, PRUEBA 2 *"Menu SIN comunicacion -> ambos en AMARILLO PARPADEO"* y PRUEBA 6 *"Modo Manual - Boton 3"*. **Se reescriben por la via Bluetooth**, que es lo que SFTY-12 dice que se conserva: la independencia de la radio, no el menu. 🔴 Y recordar la cuarta cara de N-46: **este simulador escribe `✘ FAIL` y sale con codigo `0`**; lo unico que lo caza es la regla `x == y` sobre su `20/20` |
| `simulador_app_bluetooth.py` | 5/5 | no se toca |
| `simulador_repetidor.py` | 10/10 | **INTACTO** — cero menciones de pantalla, boton o mando |

---

#### E · La cuenta

| | comprobaciones |
|---|---|
| Banco: sujeto que desaparece entero | **32** (15 + 4 + 6 + 7) |
| Banco: se invierten o se rebaselinan | **~46** (`flash_01` 11, `esclavo_01` ~5, `esclavo_07` ~3, `maestro_09` ~4, `maestro_03` 1, `costura_10` 13 en revision) |
| `Validacion_LCD` | **271** |
| `Validacion_Automatico`, BLOQUE D | **44 de 72** llamadas a `comprobar()` |
| Simulador funcional | **3 de 20** |
| **EXPUESTO** | **~347 de las 782** comprobaciones de PC del acta — **~44 %** |

El banco pasaria de `411/411` a **~379**, y el acta de **15 filas a 14** si `Validacion_LCD` se retira.

**Cobertura SFTY-x despues, cruzando las 18 etiquetas `# EJERCE` con `OPTIMIZACIONES.md:109-131`:**

- **SFTY-21:** de 6 packs etiquetados a 5 (cae `esclavo_02`). Pero la mitad que se queda **de verdad**
  descubierta es la del mando, y esa la media `maestro_01_mando` **sin etiqueta** — N-102: **la tabla
  no lo reflejara**.
- **SFTY-2 y SFTY-28:** siguen cubiertas (`barrera_01/02/03`, `esclavo_06`, `maestro_09`), con las
  cuatro filas de `maestro_09` rebaselinadas.
- **SFTY-6 y SFTY-23:** intactas.
- **SFTY-5:** **cero** si `Validacion_Automatico` aborta. N-101.
- **SFTY-12, 14, 15, 18:** tercera columna **ya vacia hoy**, asi que no se pierde cobertura porque no
  la hay — pero su via se muda al Bluetooth y **nada la medira despues tampoco**. La peor es
  **SFTY-15**: sus contadores de linea (`RX 0 - nada llega` / `RX 4k - BASURA`) **no estan en
  `$STATUS`**, asi que esa capacidad de diagnostico **se pierde de verdad, no se traslada**.

---

#### F · El ESP32: dieciseis instrumentos que hacen falta ANTES de la primera linea

**MEDIDO:** cero rutas declaradas, cero packs, cero pasos en `compuerta.py` para el ESP32 de
expansion. Y `compuerta.py:88`:

```
_ROLES = ("Maestro", "Esclavo", "Repetidor")
```

**Tres papeles.** La guarda de rutas censa tuplas `(rol, carpeta, fichero)` y en `:128` completa las
que vienen sin rol probando **solo esos tres**. **Un proyecto nuevo del ESP32 de expansion es
invisible para ella**, y con `RUTAS_MINIMAS_ESPERADAS = 20` (`compuerta.py:86`) el suelo tampoco lo
nota, porque las 43 rutas de hoy siguen ahi.

> 🔴 **Es N-75 con un agravante.** Alli entraron cuatro defectos detras de **dos instrumentos que
> ABORTARON** —o sea, que gritaron y nadie escucho—. Aqui **no hay ninguno que pueda abortar**:
> `CLAUDE.md` §3, literal, *"un `ABORTADO` al menos grita; un hueco no"*.

**Puente serie (`J17` p2/p3, `PB7`/`PB6`, 9600 8N1):**

1. **Costura de tres tramos.** `app_01_comandos` cruza hoy el `.js` contra el C++ de las dos puntas.
   Con el ESP32 en medio hay **una tercera tabla**: todo comando que la app emite el puente lo pasa
   integro, y todo `$ACK`/`$ERR`/`$STATUS` del STM32 llega a la app. Un puente que filtra en silencio
   es N-58 otra vez.
2. **El puente NO origina.** SFTY-2 extendido: censo `grep` de literales de comando en el fuente del
   ESP32 que no procedan del buffer de entrada. El molde existe y es `esclavo_06_no_abre_paso`.
3. **Valida antes de retransmitir**, como SFTY-16 ya obliga al repetidor: ni relaya basura, ni
   **parte ni une** una trama —`$STATUS` lleva su `*XX`—. `documentos_03_trama_status` vigila hoy
   tres copias del contrato (C++, Manual 10, `app.js`); **el ESP32 es la cuarta**.
4. **Presupuesto del enlace.** `costura_09_presupuesto_radio` aplicado al serie: peor caso de
   bytes/segundo contra los 9600 bps, y el buffer del puente por encima de la rafaga. **Recalculado
   del fuente en cada corrida, no escrito en prosa** — N-71.
5. **Silencio no es orden.** Con el TX del ESP32 mudo, ausente o en reposo (medida **M5** del
   Manual 17), ninguna accion. Con `control_negativo`.

**Watchdog — va PRIMERO, como dice la Fase 5:**

6. **Existe y se alimenta.** Un `esp_task_wdt_init()` sin `esp_task_wdt_reset()` es `CAM_UMBRAL_PIN`
   con otro nombre. El censo es `grep` de la declaracion contra las llamadas: `costura_10`.
7. **Y se alimenta desde la tarea que se cuelga**, no desde otra que sigue viva cuando la primera
   muere. Un watchdog alimentado por el vecino no vigila a nadie.
8. **La desigualdad, en un pack.** Periodo del watchdog del ESP32 **<** `SFTY6_SILENCIO_MS = 25000UL`
   (`*/include/protocolo.h:149` en las dos puntas). Si no, el STM32 ya se fue a ambar antes de que el
   puente se recupere solo, y el watchdog no sirve para lo que se puso. **Es N-71 exacto**: un techo
   que hoy vive en prosa.
9. **El STM32 sigue operable sin ESP32.** Ningun camino del STM32 espera bloqueado una respuesta del
   puente. Hoy eso lo garantiza el diseno; **el Manual 17 §3.3 lo deja abierto y ningun instrumento
   lo mide**.

**`DS3231`:**

10. **La hora nace no fiable** — SFTY-18 trasladado: existe una funcion *"tengo hora?"* y **toda**
    ruta que use la hora la consulta **antes**. El molde es la prueba 5 de `maestro_03_puerta_degradado`
    (*"sin reloj en hora la puerta se cierra antes de mirar nada mas"*).
11. **El bit `OSF`.** El `DS3231` trae el *oscillator-stop flag*: se lee al arrancar y una hora con
    `OSF` puesto se declara **no fiable**. Es la leccion del ano marcador de SFTY-18, con el bit que
    el chip ya regala.
12. 🔴 **`$ACK` que mira** — `app_03_sin_ok_mudo` extendido al ESP32. **La rama `SET_RTC` del puente
    no puede contestar `RESULT:OK` sin mirar el retorno de la escritura I2C.** Es literalmente el
    defecto del 28/08 (N-80) **mudandose de micro**: sin este pack entra otra vez, y esta vez sin
    instrumento que lo cace.
13. **Atomicidad**: la hora entra entera o no entra. `esclavo_05_hora_atomica` trasladado a la
    escritura multi-registro por I2C.
14. **Rango y BCD por BARRIDO, no por muestra.** El `DS3231` guarda BCD; ninguna hora invalida
    (mes 13, 31/02) se escribe, y el rechazo es explicito. `esclavo_04_desfase` barre las 3.600
    combinaciones precisamente porque *"los fallos de aritmetica circular viven en el salto de 59 a 0,
    que un muestreo se salta"*.

**Estructural, sin lo cual nada de lo anterior mide:**

15. **Un rol nuevo en `compuerta.py:88`** para el proyecto del ESP32, o la guarda de rutas no ve su
    fuente. Sin esto, mover o renombrar un `.cpp` del ESP32 rompe instrumentos en silencio.
16. **`compuerta.py` tiene que COMPILAR el ESP32 de expansion**, como ya compila el Repetidor
    (`20.6% de 1310720 B` en el acta). `CLAUDE.md` §3, literal: *"un instrumento que no esta en la
    compuerta no mide nada — y no deja rastro de que falta"*.

**Y §8.bis para los dieciseis**: se inyecta el defecto en el fuente real, se exige que **baje la
cuenta y cambie el codigo de salida**, y se restaura verificando con `git diff HEAD` **vacio** — no
con la impresion de haberlo restaurado.

---

#### G · Veredicto sobre la compuerta

**SI se daria cuenta de cuatro cosas** —y conviene decirlo, porque son mejores de lo esperado:

1. **El borrado de ficheros.** `banco/modelos/maestro.py:31` declara `MANDO`, `:48` declara `BOT =
   ("Maestro","src","botones.cpp")`; `banco/modelos/esclavo.py:28-29` declaran `_ESC_MANDO` y
   `_ESC_MENU`; `packs/maestro_06_fuentes_pantalla.py:50` y `packs/maestro_07_menu_opciones.py:46`
   declaran `lcd.cpp` y `menu.cpp`. Borrar cualquiera -> **guarda de rutas ABORTADO, exit `2`, la
   compuerta se para antes de compilar nada**.
2. **Los arneses ausentes.** `compuerta.py:357` y `:626` marcan `ABORTADO` con motivo si falta el
   directorio.
3. **La deriva de cifras.** `documentos_01_cifras_del_acta`, 51 comprobaciones, compara README y
   `ESTADO.md` contra el acta **mas reciente**: `411`, `38` packs, `271/271`, y **las 15 filas**.
   Retirar `Validacion_LCD` de la compuerta baja el acta a 14 y esa comprobacion falla.
4. **La Fase 2.** `costura_10_funciones_muertas` ve a `botones_setup()` perder su llamador **aunque
   `botones.cpp` siga en disco**. Es el unico que lo ve.

**NO se daria cuenta de cinco, y estas son las que importan:**

1. 🔴 **El ESP32 entero.** Sin rol en `_ROLES`, sin pack, sin compilacion. La compuerta saldria con
   **15 PASS y exit `0`** con el firmware del ESP32 sin una sola comprobacion detras, **y el acta no
   tendria una fila donde echarlo de menos**.
2. 🔴 **Un pack en verde midiendo hardware que ya no existe.** Si `mando.cpp` se queda en disco y solo
   se retiran los reles, las **15 comprobaciones** de `maestro_01_mando` siguen en `PASS` sobre
   secuencias que **ningun dedo puede generar**. Es la prueba muerta de N-51 introducida **sin tocar
   el fuente**, y la forma de N-89: un cambio que ningun test delata porque el firmware sigue siendo
   correcto.
3. 🔴 **El veto de `mando_ambarLocal()` (N-79).** `esclavo_07` comprueba que las tres guardas
   **consultan** el latch de Bluetooth; **no** comprueba que el latch del mando pueda **armarse**.
   Retirado el armador, `Esclavo/src/main.cpp:406`, `:416` y `:540` se vuelven siempre-verdaderos, la
   compuerta sigue en verde, y SFTY-21 **desaparece por sustraccion**.
4. 🔴 **La perdida de cobertura de SFTY-21 por el mando.** N-102: `maestro_01_mando` no lleva
   `# EJERCE`, asi que `documentos_02` seguira en `10/10` y la tabla no cambiara ni un caracter.
5. 🔴 **`Validacion_Automatico` llevandose SFTY-5.** La compuerta *diria* `ABORTADO` —bien—, pero
   **nada en el repositorio dice que ese arnes es el unico instrumento de SFTY-5** salvo una celda de
   `OPTIMIZACIONES.md:113` que ningun pack cruza, porque la propia tabla admite que los arneses C++
   son *"invisibles para ese censo"*.

> 🔴 **En una linea: LA COMPUERTA SABE VER LO QUE SE BORRA Y NO SABE VER LO QUE SE QUEDA SIN SUJETO.**
>
> Y el orden de las seis fases empuja hacia el segundo caso: **la Fase 2 *ignora* los pulsadores y la
> Fase 3 *retira* la pantalla**. Entre una y otra hay una ventana —dias, quiza semanas— en la que
> **32 comprobaciones estarian en verde midiendo codigo que ya no corre**, con la compuerta en `15
> PASS | 0 FALLA | 0 ABORTADO` y exit `0`. Ese `0` no diria *"el firmware cumple"*: diria *"nadie ha
> preguntado"*.

**Lo que hay que hacer antes de tocar firmware, en este orden:**

| # | que | por que va ahi |
|---|---|---|
| 1 | **Etiquetar `maestro_01_mando`** con `# EJERCE SFTY-21` y actualizar `OPTIMIZACIONES.md:128` a siete packs | N-102: sin esto, el borrado del pack es **silencioso** para el instrumento de trazabilidad. Va primero porque hace visible una cobertura que hoy no se cuenta |
| 2 | **Actualizar `compilar.ps1:64` y los stubs de `arnes_automatico.cpp`** en el **mismo commit** que retire `mando.cpp` | N-101: o SFTY-5 se queda con cobertura **cero**, en ABORTADO |
| 3 | **El pack que herede el veto de `mando_ambarLocal()`**, ANTES de borrar el armador | N-79. Retirar codigo **no es neutro** cuando otros dependen de que una bandera pueda ser CIERTA |
| 4 | **El rol del ESP32 en `_ROLES` y su compilacion en la compuerta**, aunque el pack aun este vacio | para que el hueco **grite** en vez de no dejar rastro |
| 5 | **Anotar el total `411` esperado antes y despues de CADA fase** | es la comparacion de totales de §5, la unica red para esta clase de deriva, y ya ha salvado la migracion dos veces |

**LECCION REUTILIZABLE: retirar hardware es la operacion mas peligrosa que puede sufrir un banco de
pruebas, porque no rompe ningun instrumento — los deja midiendo. La guarda de rutas vigila ficheros
que desaparecen; el compilador vigila simbolos que faltan; ninguno de los dos vigila una comprobacion
que sigue corriendo, sigue pasando y ya no habla de nada. Antes de retirar una pieza, el censo no es
"que se rompe" sino "que se queda sin sujeto", y ese censo hay que hacerlo pack a pack Y arnes a
arnes, con la cuenta anotada antes y despues, porque el unico sintoma que este banco emite ante un
instrumento sin sujeto es un numero que no baja.**

---

*Borrador escrito el 31/08/2026 sobre HEAD `8d76f1e`, arbol LIMPIO, sin modificar ni un fichero del
repositorio. Los 38 packs se corrieron uno a uno y la suma da `411`, la del acta. Lo marcado MEDIDO
se puede repetir abriendo el fichero y la linea que se cita, o corriendo el comando que se pega. Lo
marcado LEIDO viene de un documento y no se ha verificado contra el fuente. Nada de esto autoriza
nada: la sesion de banco sigue siendo el bloqueante.*

---

### 🔴 N-102 — `maestro_01_mando` ejerce SFTY-21 y no lleva la etiqueta `# EJERCE`: 15 comprobaciones que la tabla de trazabilidad nunca conto, y no vera desaparecer

**De donde sale:** de cruzar los packs que se quedan sin sujeto contra la tercera columna de
`OPTIMIZACIONES.md`, para saber que reglas `SFTY-x` pierden cobertura. Uno de los packs afectados no
aparecia en la cuenta, y la razon no era que no ejerciera nada.

**MEDIDO:**

```
grep -c "EJERCE" 01_Firmware/Simulaciones/banco/packs/maestro_01_mando.py    ->  0

01_Firmware/Simulaciones/banco/packs/maestro_01_mando.py:3
    # SECUENCIAS DEL MANDO DE RELES (SFTY-21, mando.cpp)

01_Firmware/Simulaciones/banco/packs/esclavo_02_inhibicion_menu.py:14
    # EJERCE SFTY-21: el mando queda inhibido con el menu abierto.
```

**El pack se titula "SFTY-21" en su primera linea util y no lleva la etiqueta.** Su hermano
`esclavo_02`, que vigila la otra mitad de lo mismo, si la lleva. Las 18 etiquetas `# EJERCE SFTY-x`
que hay hoy en `banco/packs/` son estas —censo completo, `grep -n "EJERCE SFTY" *.py`—:

```
barrera_01_pines_de_luz:34        SFTY-2      esclavo_04_desfase:14             SFTY-23
barrera_02_dos_puntas:50          SFTY-2      esclavo_06_no_abre_paso:36        SFTY-2
barrera_03_talanquera:3           SFTY-28     esclavo_07_ambar_emergencia:34    SFTY-21
costura_02_fase_ciclo:11          SFTY-21     maestro_05_ciclo_sin_radio:14     SFTY-21
costura_06_reanudacion:11         SFTY-21     maestro_09_test_leds:53           SFTY-2
costura_08_silencio:3             SFTY-6      maestro_09_test_leds:54           SFTY-28
costura_09_presupuesto_radio:3    SFTY-6
esclavo_01_latch_ambar:14         SFTY-21
esclavo_02_inhibicion_menu:14     SFTY-21
esclavo_03_par_config:27          SFTY-23
```

Y lo que publica la tabla, `OPTIMIZACIONES.md:128`:

```
| SFTY-21 | ✅ esclavo_01_latch_ambar · esclavo_02_inhibicion_menu · esclavo_07_ambar_emergencia
           · maestro_05_ciclo_sin_radio · costura_02_fase_ciclo · costura_06_reanudacion |
```

Seis packs. **`maestro_01_mando` no esta, y son 15 comprobaciones** —el barrido de los 254 trenes de
1 a 7 pulsos, el barrido de cadencia de 100 a 10.000 ms, la ventana deslizante, la purga de gestos
viejos—: la mitad **del Maestro** de SFTY-21, que es literalmente *"Modo Degradado por reloj y mando
de 4 reles"* (`OPTIMIZACIONES.md:199`).

**Por que esto no es una fila que falta, sino un fallo del instrumento.** `documentos_02_trazabilidad_sfty`
existe porque el 27/08 se descubrio que esa columna estaba **escrita a mano**, y su promesa es que se
levanta *"BUSCANDO la etiqueta `# EJERCE SFTY-x` en `banco/packs/`, no escribiendola a mano"*. Lo
cumple, y lo cumple en las dos direcciones: sus 10 comprobaciones exigen que cada fila cite
**exactamente** los packs etiquetados, que ningun pack citado carezca de etiqueta y que todos existan.
Corrido hoy sale en `10/10`.

> 🔴 **Y aun asi tiene un punto ciego estructural: solo puede ver lo que lleva etiqueta.** Un pack
> que ejerce una regla y no se etiqueta es invisible para el censo, **y por tanto tambien lo es su
> desaparicion**. Cuando `maestro_01_mando` se borre, `documentos_02` seguira en `10/10` y la fila de
> SFTY-21 no cambiara **ni un caracter**. La tabla dira lo mismo el dia antes y el dia despues de
> perder 15 comprobaciones de una regla de seguridad.
>
> Es la forma exacta de N-73 y de `CAM_UMBRAL_PIN` trasladada al instrumental: **no una prueba que no
> mide, sino una medida que no se cuenta**. Y es peor de lo que parece, porque el documento advierte
> encima de si mismo que *"una fila que cita menos packs de los que hay no se ve"*. Se escribio la
> advertencia y el caso estaba debajo.

**Lo que hay que hacer, y en que orden:**

1. **Poner la etiqueta AHORA, antes de borrar nada.** `# EJERCE SFTY-21: las tres secuencias del
   mando de reles y su ventana deslizante.` Con la etiqueta puesta y la fila de `OPTIMIZACIONES.md:128`
   actualizada a **siete** packs, el borrado del pack **si** hara fallar a `documentos_02` y obligara
   a tocar la tabla de forma consciente. Sin ella, el borrado es silencioso.
2. **Es un commit de un solo cambio con sentido propio**, y no va mezclado con la retirada del mando:
   primero se hace visible la cobertura, luego se retira. Al reves no sirve para nada.
3. **Censar si hay mas casos.** El metodo es `grep` de `SFTY-` en las cabeceras de los 38 packs
   contra `grep` de `EJERCE SFTY-`, y comparar. `esclavo_05_hora_atomica` se titula *"APLICACION
   ATOMICA DE LA HORA (SFTY-23)"* y **tampoco lleva etiqueta** —7 comprobaciones mas que la tabla no
   cuenta—. Y `maestro_04_sync_horaria` se titula *"SINCRONIZACION HORARIA POR RADIO (SFTY-23)"*, sin
   etiqueta, 11 comprobaciones. **Son tres, no uno**, y los tres se han encontrado con el mismo
   `grep` de dos lineas. *(MEDIDO sobre las cabeceras; **no** he verificado prueba por prueba que las
   18 comprobaciones de `esclavo_05` y `maestro_04` ejerzan SFTY-23 de verdad, y esa verificacion es
   obligatoria antes de etiquetar: `CLAUDE.md` dice que **solo se etiqueta lo que el pack comprueba
   de verdad**, porque una fila que miente es peor que una vacia.)*

**LECCION REUTILIZABLE: un instrumento que se levanta buscando una etiqueta solo puede ver lo
etiquetado, asi que su punto ciego no son los falsos positivos —esos los caza, y en las dos
direcciones— sino las OMISIONES, que son invisibles por construccion. Y la omision se cobra dos
veces: primero como cobertura que no aparece, y despues, el dia del borrado, como perdida de
cobertura que el instrumento no puede senalar porque nunca la tuvo apuntada. El titulo de un pack no
es su etiqueta: si la cabecera nombra una SFTY-x y la etiqueta no esta, o el titulo miente o falta la
etiqueta, y hay que decidir cual de las dos.**

---

---

### 🔴 N-101 — `Validacion_Automatico` compila `mando.cpp` REAL: retirar el mando no lo hace FALLAR, lo hace ABORTAR, y se lleva el unico instrumento de SFTY-5

**De donde sale:** de preguntarse que arnes queda sin sujeto al retirar el mando de 4 reles, dando
por hecho —como dice `CLAUDE.md` §8— que `Validacion_Automatico` es *"el arnes del ciclo"*. Lo es, y
ademas es otra cosa que no estaba escrita en ningun sitio.

**MEDIDO, abriendo los dos ficheros:**

```
01_Firmware/Validacion_Automatico/compilar.ps1:64
    Compilar-Fuente (Join-Path $MAESTRO 'src\mando.cpp')           'mando.o'

01_Firmware/Validacion_Automatico/arnes_automatico.cpp:446
    mando_setup();                   // N-52: limpia secBoton/pendiente del mando
01_Firmware/Validacion_Automatico/arnes_automatico.cpp:485-486
    if (pulsarA) mando_registrarPulso(MANDO_A);
    if (pulsarB) mando_registrarPulso(MANDO_B);
01_Firmware/Validacion_Automatico/arnes_automatico.cpp:494
    mando_actualizar();
```

Y el reparto de sus comprobaciones, contado sobre las llamadas a `comprobar()`:

```
total de llamadas a comprobar() en arnes_automatico.cpp .......  72
    lineas 1..948   (BLOQUE A + B + C) ........................  28
    lineas 949..fin (BLOQUE D: EL MANDO DE RELES) .............  44
```

`arnes_automatico.cpp:949` abre el bloque asi, literal: `// BLOQUE D: EL MANDO DE RELES (SFTY-21) —
mando.cpp REAL, sobre los PINES`.

*(El acta publica `71/71` y las llamadas a `comprobar()` son 72: una vive en un ayudante o en una
rama que no se ejerce. **La cifra de 44/72 es de sitios de llamada, no de comprobaciones ejecutadas**
— se dice asi a proposito, porque contar lo uno y publicar lo otro es como se cuelan las cifras que
nadie midio.)*

**El propio `compilar.ps1` explica por que esta ahi**, y el motivo es bueno (`compilar.ps1:7-9`):

```
# N-52: mando.cpp se suma aqui. Antes el arnes media los pines de verdad pero
# senalActiva -el static de semaforo.cpp que SOLO pone mando.cpp- nunca se ponia a
# true en este binario, porque mando.cpp no se compilaba.
```

Es decir: **`mando.cpp` se anadio al arnes precisamente porque sin el una rama de `semaforo.cpp`
nunca se ejercia.** La solucion de N-52 es correcta y crea la dependencia que hoy muerde.

**LO QUE NADIE HABIA ESCRITO, y es lo grave.** `OPTIMIZACIONES.md:113`:

```
| SFTY-5 | Maestro/src/semaforo.cpp · Esclavo/src/semaforo.cpp | ✅ Validacion_Automatico/arnes_automatico.cpp
          — **arnes C++, invisible para el censo de packs**, ver abajo |
```

**`Validacion_Automatico` es el UNICO instrumento de SFTY-5** —la transicion de luz legal en
Colombia, Res. 2024: Verde -> Rojo directo, Rojo -> Amarillo fijo 4,0 s -> Verde—. Ningun pack la
ejerce; la propia tabla admite que los arneses C++ son invisibles para el censo de
`documentos_02_trazabilidad_sfty`, que solo busca `# EJERCE SFTY-x` **dentro de `banco/packs/`**.

**La consecuencia, y es la diferencia entre FALLA y ABORTADO otra vez:**

| si se borra `mando.cpp` y no se toca nada mas | que pasa |
|---|---|
| `compilar.ps1:64` no encuentra el fichero | el arnes **no enlaza** |
| `compuerta.py:626` | `anotar("arnes del automatico", ABORTADO, ...)` |
| las 28 comprobaciones de los BLOQUES A/B/C, incluida SFTY-5 al milisegundo | **no corren** |
| el acta | `14 PASS | 0 FALLA | 1 ABORTADO`, exit **2** |

Y antes que eso, `compuerta.py:101` (guarda de rutas) ya habria abortado, porque
`banco/modelos/maestro.py:31` declara `MANDO = ("Maestro", "src", "mando.cpp")` y
`banco/modelos/esclavo.py:28` declara `_ESC_MANDO`. **La compuerta se para en la primera fila y ni
siquiera llega a compilar.** Eso es bueno —grita— pero deja el trabajo a medias con el arbol en
rojo, que es justo la situacion que `CLAUDE.md` §3.quater prohibe apuntar para luego.

> 🔴 **Y hay un modo de romperlo SIN que grite, que es el peligroso.** Si en vez de borrar
> `mando.cpp` se le retiran los llamadores —Fase 2, *"ignorar los pulsadores"*—, el fichero sigue en
> disco, la guarda de rutas no ve nada, el arnes enlaza igual y **sus 44 comprobaciones del BLOQUE D
> siguen en verde ejerciendo un mando que ningun rele puede accionar**. El arnes seguiria midiendo
> de verdad; lo que ya no existiria es el sujeto.

**Lo que este N-x fija, y no es una recomendacion:**

1. **`compilar.ps1:64` y los stubs de `arnes_automatico.cpp` se actualizan EN EL MISMO COMMIT que
   retire `mando.cpp`.** No en el siguiente. `CLAUDE.md` §5 lo dice para los packs que leen por ruta;
   esto es lo mismo para un arnes que **compila** por ruta.
2. **Los BLOQUES A/B/C se preservan enteros.** Son SFTY-5, el ciclo completo y la orfandad SFTY-6, y
   **ninguno de los tres depende del mando**. Si el arnes se toca, se toca el bloque D y solo el.
3. **Antes de dar el arnes por bueno tras el cambio, §8.bis:** se inyecta el defecto que ya se sabe
   que caza —`VERDE1` forzado a `HIGH` por debajo del enclavamiento de `aplicarSalidas()`, que es
   como se conecto: cayo a `25/26`— y se exige que la cuenta baje y el codigo de salida cambie. Un
   arnes recortado que no se ha visto fallar despues del recorte es un arnes nuevo.

**Y una nota de citas que aparecio de paso, MEDIDA:** `05_Funcional/17_...md` §2.4 y `ESTADO.md` §4b
citan los tres consumidores del veto como `Esclavo/src/main.cpp:401`, `:408`, `:526`. **Estan
caducadas.** Medido hoy:

```
Esclavo/src/main.cpp:406   if (!mando_ambarLocal() && !bluetooth_ambarEmergencia()) {
Esclavo/src/main.cpp:416   if (!mando_ambarLocal() && !bluetooth_ambarEmergencia()) {
Esclavo/src/main.cpp:540   if (!mando_ambarLocal() && !bluetooth_ambarEmergencia() &&
Esclavo/src/mando.cpp:103  bool mando_ambarLocal() { return ambarLocal; }
Esclavo/include/mando.h:76 bool mando_ambarLocal();
```

`CLAUDE.md` §3.ter tiene las buenas (`:406`, `:416`, `:540`); los dos documentos de trabajo van tres
commits por detras. Una cita `fichero:linea` que apunta al sitio equivocado no es un error
cosmetico: manda a quien va a ejecutar la Fase 2 a mirar donde no esta.

**LECCION REUTILIZABLE: un arnes que COMPILA un `.cpp` depende de el con mas fuerza que un pack que
lo parsea, y esa dependencia no esta en ninguna tabla — hay que leerla del `compilar.ps1`. Antes de
retirar un modulo, el censo no es solo `grep` de sus llamadores en el firmware: es tambien `grep` de
su nombre en los scripts de compilacion del instrumental. Y cuando el modulo que se retira es la
unica pata de un arnes que resulta ser el unico instrumento de una regla SFTY-x, retirarlo no baja
la cobertura: la pone a cero, en ABORTADO, y con la compuerta parada en la primera fila.**

---

---

### 🔴 N-100 — Cinco afirmaciones marcadas MEDIDO fueron refutadas por el firmware el MISMO dia, y siguen publicadas

**De donde sale:** de cruzar cada censo de comandos de los documentos vigentes contra
`bluetooth.cpp` de las dos puntas, en vez de fiarse de su marca `MEDIDO`.

Este no es el fallo de N-98 —dos documentos que se contradicen— ni el de N-99 —un documento que
nadie toco—. Es el tercero y el mas caro de detectar: **documentos que se midieron bien, se
marcaron `MEDIDO` con razon, y quedaron falsos horas despues porque el firmware avanzo.**

**MEDIDO — lo que el firmware hace hoy** (`d34cfe2` N-78 y `caef8a1` N-82/N-83, los dos del 28/08):

```
01_Firmware/Maestro/src/bluetooth.cpp:191   SET_MODO:MENU          ($ACK :209, y rama propia
                                                                    para DEGRADADO en :196-204)
01_Firmware/Maestro/src/bluetooth.cpp:212   SET_MODO:ALCANCE
01_Firmware/Maestro/src/bluetooth.cpp:223   SET_MODO:INTELIGENTE
01_Firmware/Maestro/src/bluetooth.cpp:234   SET_MODO:DEGRADADO     ($ERR motivado :245, $ACK :250)
01_Firmware/Maestro/src/bluetooth.cpp:330   REINICIAR_RELOJ
01_Firmware/Maestro/src/bluetooth.cpp:295-325  SET_RTC con CINCO ramas distintas
01_Firmware/Esclavo/src/bluetooth.cpp:130   CMD:AMBAR_EMERGENCIA   (y :171 con PIN)
01_Firmware/Esclavo/src/bluetooth.cpp:157-158  FORZAR_ROJO -> $ERR,DESC:RENOMBRADO_USE_AMBAR_EMERGENCIA
```

Y la app los manda: `05_Funcional/App_Semaforo/.../app.js:537` (`AMBAR_EMERGENCIA`), `:602`
(`SET_MODO:MENU`), `:668` (`SET_MODO:DEGRADADO`), con la lista sin PIN en `:189`.

**Las cinco afirmaciones refutadas — se marcan REFUTADAS, no se borran:**

**1. REFUTADA** — *"No hay forma de entrar al Degradado por Bluetooth"*

> `05_Funcional/8_Procedimiento_Modo_Degradado.md:30-31` — *"Y hoy ni siquiera es una puerta:
> **tampoco existe el comando de ida.** No hay forma de entrar al Degradado por Bluetooth"*
>
> `:41` — *"**No existe comando Bluetooth para ENTRAR** en Degradado | `grep DEGRADADO` sobre los
> dos `bluetooth.cpp` devuelve **una sola linea** ... la cadena de estado de `$STATUS`, no un
> comando"*
>
> **REFUTADO por `Maestro/src/bluetooth.cpp:234`.** El `grep` era correcto **cuando se corrio**
> —commit `bdcf03d`, 19:00— y dejo de serlo con `d34cfe2`, el mismo dia.

**2. REFUTADA** — *"no hay `SET_MODO:MENU`"*

> `05_Funcional/17_Arquitectura_28-08_y_Decisiones_Abiertas.md:296` — *"Desde Bluetooth se alcanzan
> **tres** [de ocho modos]. Y **no hay `SET_MODO:MENU`**"*, con la consecuencia de `:300-308`:
> *"cada modo se convierte en una puerta de un solo sentido"*, y el Anexo `:880-881` pidiendolo como
> trabajo pendiente numero 1.
>
> **REFUTADO por `Maestro/src/bluetooth.cpp:191`**, y con mas cuidado del que el Manual 17 pedia:
> la rama trata aparte el caso `MODO_DEGRADADO` (`:196-204`) para no saltarse el todo-rojo de
> despedida.

**3. REFUTADA** — *"ninguno pone al Esclavo en ambar"*

> `04_Manuales/MANUAL_MANDO_4_RELES.md:439` — *"| **Sin sustituto** | **MEDIDO:**
> `Esclavo/src/bluetooth.cpp` acepta `FORZAR_ROJO` (`:109`, `:124`), `SOLICITAR_PASO` (`:128`),
> `TEST_LEDS` (`:146`) y `SET_RTC:` (`:159`). **Ninguno pone al Esclavo en ambar.** `FORZAR_ROJO` es
> rojo, no ambar, y no revoca nada |"*
>
> `05_Funcional/8_Procedimiento_Modo_Degradado.md:473` — *"el Esclavo **no tiene comando**"*
>
> `05_Funcional/17_...md:291-292` — *"Y el del Esclavo (`Esclavo/src/bluetooth.cpp:124-168`):
> `FORZAR_ROJO`, `SOLICITAR_PASO`, `TEST_LEDS`, `SET_RTC`"*
>
> **REFUTADO por `Esclavo/src/bluetooth.cpp:130` y `:171`** (`CMD:AMBAR_EMERGENCIA`), y por
> `:157-158`, donde `FORZAR_ROJO` **ya no se acepta**: contesta
> `$ERR,DESC:RENOMBRADO_USE_AMBAR_EMERGENCIA`. Es N-83, cerrado en `caef8a1` (`roadmap.md:47`).

**Esta tercera es la que mas cuesta**, y conviene decir por que: la fila `:439` del Manual del mando
es **la que sostiene la decision abierta §3.3 del Manual 17** —*"como se opera el equipo si el ESP32
se cuelga"*—. Publica que **no hay sustituto** del `B·B·B` justo cuando el sustituto ya existe y la
app ya tiene su boton. Un responsable que lea esa fila esta decidiendo entre alternativas con una de
ellas tachada por error.

**4. REFUTADA** — *"`SET_RTC` puede rechazar en silencio y contestar `RESULT:OK`"*

> `05_Funcional/17_...md:334` — *"### 2.5 🔴 `SET_RTC` puede rechazar en silencio y contestar
> `RESULT:OK`"*, con `bluetooth.cpp:173-175` citado, y el Anexo `:884-885` pidiendo el arreglo.
>
> **REFUTADO por `Maestro/src/bluetooth.cpp:295-325`**, que tiene hoy **cinco ramas**:
> `FORMATO_INVALIDO`, `SIN_CRISTAL_VEA_CONSULTA_RELOJ` via `reloj_hayCristal()`, rango fuera de
> calendario, `HORA_PUESTA_SIN_PROPAGAR` y `OK`. Es N-80, y **`ESTADO.md` BLQ-2 ya lo declara
> cerrado**, con una frase que vale para toda esta entrada: *"Un bloqueante que ya no bloquea,
> escrito como si bloqueara, cuesta la misma sesion que uno real"*.

**5. REFUTADA** — la cita del acta de referencia del propio Manual 17

> `05_Funcional/17_...md:5-6` — *"Acta de compuerta de referencia: `evidencia/2026-08-28_compuerta.txt`
> — `15 PASS | 0 FALLA | 0 ABORTADO`, HEAD **`3733544`**, arbol LIMPIO (lo dice la propia acta)"*
>
> **REFUTADO:** ese fichero hoy dice `HEAD : 043860a` (lo reescribio `f25fa57`, el mismo dia). La
> cifra sigue siendo correcta; **la cita ya no reproduce**, que es lo unico que este repositorio
> exige de una cita.

---

#### El HTML huerfano: la misma enfermedad, en el documento que se entrega

`05_Funcional/Guia_Cableado_y_Pruebas_Banco.html` — **81.093 bytes, 1.350 lineas, un solo commit
(`24276ab`)**. Nunca se ha tocado. Ademas de llevar la arquitectura anterior entera (N-98: `HC-05`
vigente en `:435`, ESP32 alternativa en `:439`, `DS3231` en `PB3`/`PB4` en `:654`, alimentacion
desde el riel de `J17` en `:435`), **publica cifras de verificacion que no salen de ninguna acta**.

**MEDIDO — su cabecera** (`:279-284`):

```
commit 614065d
compuerta 14 PASS · 1 FALLA · 0 ABORTADO
Maestro 85,8 % · 56.260 B
Esclavo 63,9 % · 41.872 B
arnes de pantalla 271/271
```

**MEDIDO — contra `evidencia/`:**

| cifra del HTML | acta del 31/08 (`8d76f1e`) | acta del 28/08 que el propio HTML dice usar |
|---|---|---|
| `14 PASS · 1 FALLA` | **`15 PASS \| 0 FALLA \| 0 ABORTADO`** | **`15 PASS \| 0 FALLA \| 0 ABORTADO`** |
| Maestro 85,8 % / 56.260 B | **88,3 % / 57.880 B** | 88,3 % / 57.880 B |
| Esclavo 63,9 % / 41.872 B | **64,4 % / 42.176 B** | 64,4 % / 42.176 B |
| 271/271 pantalla | 271/271 ✅ | 271/271 ✅ |

El HTML afirma en `:1282-1283` que sus cifras *"salen del acta `evidencia/2026-08-28_compuerta.txt`"*.
**No salen de ahi.** Esa acta, en cualquier version de su historia —incluida
`git show 24276ab:evidencia/2026-08-28_compuerta.txt`—, dice `15 PASS | 0 FALLA | 0 ABORTADO`. Y el
buscador se descarto antes de reportar (`CLAUDE.md` §4):

```
grep -l "14 PASS" evidencia/*.txt   ->   evidencia/2026-08-03_compuerta.txt   (unico)
                                          y ese acta dice  10 PASS | 2 FALLA | 0 ABORTADO
```

**Ninguna acta del repositorio ha dicho nunca `14 PASS | 1 FALLA`.** La cifra de la cabecera del
entregable es irreproducible.

**Y el commit al que se ancla no existe para quien lo reciba. MEDIDO:**

```
git merge-base --is-ancestor 614065d HEAD   ->   NO
```

`614065d` esta en el object store —viene del repositorio padre— pero **no es alcanzable desde
`main-nuevo`**. El HTML se describe a la vez como *"acta sobre `614065d`"* (`:471`) y como *"El
firmware de esta entrega es POSTERIOR a `614065d`, y todavia sin commitear"* (`:708`).

**Por que esto no es un detalle de un fichero olvidado:** es **el documento de conexiones que se
entrega**. `generar_entrega_v9_0.py:39` lo mete en el paquete:

```
GUIA_HTML = "Guia_Cableado_y_Pruebas_Banco.html"
```

Lo enlaza `ESTADO.md:41` **desde el 31/08** (`8d76f1e`) —antes no lo enlazaba nadie, y el propio
`ESTADO.md` lo dice—. **`README.md` y `roadmap.md` siguen sin enlazarlo**: `grep "Guia_Cableado"`
sobre los dos da **cero**.

🔴 **Y la ficha del indice describe mal el fichero indexado.** `ESTADO.md:41` dice que el HTML
*"Cubre `J17` (ESP32)"*. El HTML cubre `J17` **con un `HC-05`** y llama al ESP32 alternativa
(`:435`, `:439`). El indice y lo indexado se contradicen, asi que el enlace nuevo **no arregla el
problema: lo publica**.

**Que haria falta para cerrarlo.** Tres cosas, y ninguna es reescribir prosa:

1. Que las cifras de la cabecera del HTML **salgan del acta**, como ya exige la regla de
   `CLAUDE.md` §3 para el README —*"las cifras del README se copian del acta, nunca se escriben a
   mano"*—. Hoy el HTML esta fuera de esa regla porque **ningun pack lo parsea**.
2. Que su commit de referencia sea **alcanzable desde la rama que se publica**.
3. Que los censos de comandos de los tres documentos (`MANUAL_MANDO_4_RELES.md:439`,
   `8_Procedimiento...:30-41` y `:473`, `17_...md:276-292`, `:334`, `:462`, Anexo `:880-885`) se
   **releen del `.cpp`**, o se marquen con la fecha y el commit en que se midieron — que es lo
   minimo para que un lector sepa cuanto vale la marca.

> **LECCION REUTILIZABLE: `MEDIDO` es la marca que este repositorio usa para SALTARSE la
> verificacion, asi que un `MEDIDO` caducado cuesta exactamente la misma sesion que un defecto real
> — y ademas llega blindado.** Es la segunda cara de `CLAUDE.md` §4 —*"lo que TU reportas tambien es
> un instrumento"*— con un agravante que no estaba escrito: alli la causa era **plausible y falsa**;
> aqui era **verdadera y se murio de vieja**, en horas, y ninguna revision la habria pillado leyendo
> el documento, porque el documento es correcto en cada linea. **Lo que distingue una medida viva de
> una muerta no es su contenido: es su FECHA junto a la del fichero medido.** Por eso una cita de
> firmware en un documento lleva `fichero:linea` **y el commit en que se leyo**; sin el, no es una
> medida, es una foto sin fecha.
>
> **Corolario, y es la mitad que se olvida: una medida que se cae se marca REFUTADA, no se borra ni
> se "actualiza en silencio".** Las cinco de arriba nacieron bien y describen decisiones que se
> tomaron por ellas —§3.3 del Manual 17 elige entre alternativas contando con que no hay sustituto
> del `B·B·B`—. Si se corrigen sin dejar rastro, la decision queda en pie sin su premisa.

---

## ⚠️ Observacion de metodo — la parte que impide que esto vuelva

**Nueve de los diez puntos de esta auditoria viven en documentos que NINGUN pack parsea.**

**MEDIDO:** el banco son **38 packs** (`ls 01_Firmware/Simulaciones/banco/packs/*.py` -> 39
ficheros, 38 packs mas `__init__.py`), y los unicos que leen documentacion son tres:

```
banco/packs/documentos_01_cifras_del_acta.py
banco/packs/documentos_02_trazabilidad_sfty.py
banco/packs/documentos_03_trama_status.py
```

Entre los tres vigilan `README.md`, `ESTADO.md`, `OPTIMIZACIONES.md` y el Manual 10 — **y nada mas**.
Fuera del alcance de la compuerta quedan: el Manual 11, el Manual 14, el Manual 3, el
`MANUAL_CONFIGURACION_BLUETOOTH.md`, el `MANUAL_MANDO_4_RELES.md`, el
`8_Procedimiento_Modo_Degradado.md`, el `2_Manual_Hardware_y_Pruebas.md`, el propio Manual 17 y **el
HTML de 81 KB que se entrega**. El commit `2e6baf4` lo dejo escrito de su propio documento sin que
nadie sacara la consecuencia: *"Ningun pack parsea este manual: la compuerta no lo ve."*

**Y el dato duro que lo cierra: la compuerta salio `15 PASS | 0 FALLA | 0 ABORTADO` el 31/08
—`evidencia/2026-08-31_compuerta.txt`, HEAD `8d76f1e`, arbol LIMPIO— con las diez contradicciones
dentro del arbol.** Tres decisiones estructurales con dos respuestas opuestas, un diagrama que
manda cablear un bus I2C contra un LED, cinco `MEDIDO` refutados y un entregable con cifras que no
existen: **cero FALLA, exit code `0`.**

No es un defecto de la compuerta. Es `CLAUDE.md` §3 en su forma mas literal, la que ya se pago con
`Validacion_Respaldo` en N-43:

> **"Un instrumento que no esta en la compuerta no mide nada — y no deja rastro de que falta. Un
> `ABORTADO` al menos grita; un hueco no."**

Aqui el hueco son **nueve documentos**, y entre ellos el que lleva el dibujo que alguien sigue con
el destornillador. Un `0` de la compuerta sobre este arbol significa exactamente lo que `CLAUDE.md`
dice que significa —*los modelos y los arneses de PC no encuentran nada*— y ni una palabra sobre si
los papeles que van al funcional, al auditor y al instalador se contradicen entre si.

**La direccion del arreglo, y es la unica que impide que vuelva:** no son diez correcciones de
prosa. Es **un cuarto pack `documentos_04_*` que censa** —no que lea— tres propiedades comprobables
por texto sobre **todo** `05_Funcional/` y `04_Manuales/`:

1. **Unicidad de decision.** La cadena `"Decision de obra del 28/08"` (y sus variantes) no puede
   aparecer con dos direcciones contrarias. Hoy aparece en tres ficheros y significa dos cosas.
2. **Pines citados contra `pines.h`.** Cualquier documento que escriba `PB0`, `PB8`, `PB14`,
   `PB15`, `GPIO21`... junto a una funcion, tiene que coincidir con el `#define` real. Esto solo
   habria cazado N-99 entero, y N-59 y N-64 antes que el.
3. **Censos de comandos contra `bluetooth.cpp`.** Todo documento que publique una lista de comandos
   marcada `MEDIDO` se compara con los `strcmp(accion, ...)` de las dos puntas. Esto caza los cinco
   `MEDIDO` de N-100 el mismo dia en que caducan, que es cuando cuesta un minuto arreglarlos.

Y, como siempre en este repositorio, **el pack no se da por bueno hasta verlo caer** con el defecto
inyectado en el documento real (`CLAUDE.md` §8.bis): se invierte una frase de decision, se cambia un
pin en un manual, se borra un comando de un censo, y se exige que **baje la cuenta y cambie el
codigo de salida**. Un pack de documentos que nadie ha visto fallar es exactamente el adorno que da
verde del que avisa §8.bis — y esta auditoria es la prueba de que hoy hay diez cosas rojas debajo de
un `0`.

---

*Borrador escrito el 31/08/2026 sobre `main-nuevo` @ `8d76f1e`. Todo lo marcado MEDIDO se repite
abriendo el fichero y la linea que se cita, o volviendo a correr el `git` que se transcribe. Nada de
lo marcado LEIDO se ha comprobado contra hardware — y en este documento no hay ninguna afirmacion
sobre el cobre.*

---

### 🔴 N-99 — El Manual 11 manda cablear el bus I2C del reloj a la entrada de camara y a un LED

**De donde sale:** de bajar al detalle del segundo pinout de `DS3231` de N-98, en vez de anotarlo
como *"otro documento desfasado"*.

`05_Funcional/11_Manual_Instalacion_RTC_DS3231_Bateria.md` es **el unico documento del censo cuyo
seguimiento hace dano fisico**, y es tambien uno de los que **nadie ha tocado**: `git log` sobre el
fichero devuelve **un solo commit, `24276ab`**, y `grep "28/08"` sobre el da **cero coincidencias**.
No lleva aviso, no lleva tachado, no lleva fe de erratas. Se lee como vigente porque no hay nada
que diga que no lo es.

**MEDIDO — lo que el manual manda:**

```
05_Funcional/11_Manual_Instalacion_RTC_DS3231_Bateria.md:5
  "Plan de Contingencia: Modulo Externo DS3231 en pines libres PB0 (SDA) y PB8 (SCL)
   por I2C Software"

05_Funcional/11_Manual_Instalacion_RTC_DS3231_Bateria.md:111
  "│  [ SDA ]  (Datos I2C)   ──────┼─────────┼──► Pin PB0 (I2C Soft SDA)     │"

05_Funcional/11_Manual_Instalacion_RTC_DS3231_Bateria.md:112
  "│  [ SCL ]  (Reloj I2C)   ──────┼─────────┼──► Pin PB8 (I2C Soft SCL)     │"
```

**No es una frase suelta en una cabecera: es un diagrama ASCII de conexion**, del tipo que alguien
sigue con el destornillador en la mano. Y la linea `:5` lo llama *"pines libres"*.

**MEDIDO — lo que esos dos pines son de verdad:**

```
01_Firmware/Maestro/include/pines.h:46
  #define CAM_DEMANDA_PIN    PB0   // -> R64 10K + C25 100nF -> bornera J14 (antirrebote 1 ms)

01_Firmware/Maestro/include/pines.h:63
  #define LED_TESTIGO        PB8   // -> R16 1K -> LED D5. NO es entrada de camara
```

Ninguno de los dos esta libre:

- **`PB0` es la entrada de camara de demanda**, con `R64` 10 kOhm de **pull-DOWN** y `C25` 100 nF
  hasta la bornera `J14`. Un SDA bit-bang sobre ese pin no solo no va a hablar I2C contra un RC de
  1 ms: **mueve la linea que el firmware lee como demanda vehicular**. El efecto no es un reloj que
  no da la hora — es un semaforo que pide paso solo, o que deja de pedirlo.
- **`PB8` es un LED testigo** (`D5` a traves de `R16` 1 kOhm). Es una **salida de aviso, no una
  bornera**, y el propio `pines.h:50-62` explica que se deja en alta impedancia **a proposito**
  porque el sentido del LED no esta trazado. Un SCL contra ese nudo es sacar corriente a un diodo.

**Y ya esta pagado antes en este proyecto, dos veces, con estos mismos dos pines:**

- **N-59 / N-64:** `PB8` estuvo en **cuatro manuales** como *"umbral de tramo"* con el pin **sin
  leer**. `pines.h:53` lo deja escrito: *"Durante meses cuatro manuales lo describieron como
  'umbral de tramo' (N-59) y el firmware le hacia un pinMode que no servia para nada"*.
- **N-67:** la contradiccion de polaridad sobre la linea de camara —el pull-down de 10 kOhm de la
  placa contra el pull-up interno— dejaba el pin en `0,66 V`, que el micro lee **LOW**, o sea
  **demanda permanente desde el arranque sin ninguna camara conectada**. `PB0` es exactamente esa
  linea.

**El resto del repositorio ya lo sabe, y ahi esta la señal:** `13_Manual_Modulo_Expansion_I2C_y_Compras.md`
—el manual hermano, del mismo tema— lleva **fe de erratas fechada el 28/08** justo sobre esto
(`:69-90`: *"LA FILA DE `PB8` DE LA SECCION 4 ERA FALSA"*), y en `:145-146` descarta `PB0` y `PB8`
uno por uno con su motivo. Los dos documentos hablan del mismo bus, del mismo chip y de los mismos
dos pines; **uno se corrigio y el otro no**, y el que no se corrigio es el que trae el dibujo.

**Refutada, no borrada, la premisa que sostenia el diagrama:** *"`PB0`/`PB8` son los unicos pines
libres"* es falsa, y lo dice el propio Manual 13 en `:89` —*"con esa fila cayo tambien su premisa:
`PB0`/`PB8` no eran «los unicos pines libres»"*—, con el censo en su §4.1 y las rutas B y D vivas
en §4.2. Se anota aqui para que no vuelva a proponerse.

**Que haria falta para cerrarlo.** Con la arquitectura del 28/08 el bus I2C **ya no vive en el
STM32**: el `DS3231` cuelga del ESP32 por `GPIO21`/`GPIO22` con pila propia
(`17_...md:89`, `15_Lista...:203`). Asi que el arreglo **no es reelegir ruta**: es marcar el §
entero del Manual 11 como **plan de contingencia retirado**, con su motivo, y dejar en pie lo que
sigue siendo cierto de ese manual —la pila `CR2032` en `VBAT` con `R5` desoldada, `:36`-`:67`, y el
aviso de la `CR2032` sobre circuito de carga, `:114`—. Y **no se borra el diagrama: se tacha**, o
alguien volvera a proponer `PB0`/`PB8` dentro de un mes.

> **LECCION REUTILIZABLE: "pin libre" es una afirmacion sobre el firmware Y sobre el cobre, y un
> manual no la puede sostener sola.** Los dos pines de este diagrama estaban tomados —uno por una
> entrada con RC de placa, otro por un LED con su resistencia—, y las dos cosas se comprueban en
> **una linea de `pines.h` y una del esquematico**. **Corolario de censo: cuando dos manuales
> describen el mismo bus, el mismo chip y los mismos pines, la fe de erratas de uno es una alarma
> sobre el otro.** El Manual 13 se corrigio el 28/08 y el 11 no; nadie cruzo los dos, y el que se
> quedo sin corregir es precisamente el que lleva el dibujo que alguien sigue con el
> destornillador. **Un aviso que solo se pone en el documento que se estaba mirando no es un
> aviso: es un parche.**

---

---

### 🔴 N-98 — Tres decisiones estructurales tienen DOS respuestas opuestas vivas, y las dos dicen "decidido el 28/08"

**De donde sale:** de auditar los cinco "Ordenes" de la seccion B de
`05_Funcional/17_Arquitectura_28-08_y_Decisiones_Abiertas.md` documento por documento, en vez de
darla por vigente.

El repositorio ya tuvo esta clase de fallo una vez y esta escrito en el propio Manual 17
(`:725`): *"es la decision contraria a la del 28/08. El mismo dia, el mismo documento"*. Se
arreglo **una** copia. **Quedaron dos, y aparecieron dos contradicciones mas de la misma familia.**

---

#### 1. Que se enchufa en `J17` — dos contra uno, y el que pierde lleva el dibujo

**MEDIDO**, tres ficheros abiertos:

```
05_Funcional/15_Lista_de_Compras_Hardware.md:159
    "### 🔄 Decision de obra del 28/08 - VIGENTE: el ESP32 SUSTITUYE al modulo SPP"

05_Funcional/10_Manual_Modulo_Bluetooth_Telemetria.md:132-134
    "### ✅ Decision de obra del 28/08: se sigue con el modulo SPP dedicado"
    "Se instala HC-05 / JDY-30, no ESP32."

05_Funcional/Guia_Cableado_y_Pruebas_Banco.html:435,439
    "Ese es el modulo vigente [HC-05 / JDY-30], y es lo que pide la lista de compras"
    "El ESP32 queda como ALTERNATIVA, no como sustituto, y solo entra si no llegan los HC-05"
```

**La frase del `:439` del HTML es literalmente la que el commit `2e6baf4` califico de *"lo contrario
de lo decidido"* al arreglar la lista de compras.** Sigue viva, palabra por palabra, en otro fichero.

**Por que el Manual 10 es el peor sitio donde dejarlo, y no es cuestion de gusto:**

- Es el unico documento de la entrega **con dibujo de conexion del modulo** (`:286`:
  `MODULO BLUETOOTH (HC-05 / JDY-30)   TARJETA -- CONECTOR J17`). El pinout es correcto; el modulo
  que manda enchufar, no.
- Su apartado 1 esta **congelado por escrito** (`:26`, `:148`), y una decision congelada solo vale
  si reabrirla cuesta un documento. Hoy la reabre otro fichero, en silencio.
- Su tabla de tres caminos (`:144-146`) sigue **sin fila decidida**.
- `git log -- 05_Funcional/10_Manual_Modulo_Bluetooth_Telemetria.md` devuelve **un solo commit,
  `24276ab`, el raiz**. Nadie lo ha tocado desde que se creo el repositorio nuevo.

#### 2. Donde va el `DS3231` — tres documentos, tres pinouts distintos

**MEDIDO:**

```
05_Funcional/17_Arquitectura_28-08_y_Decisiones_Abiertas.md:89
    DS3231 sobre el ESP32:  GPIO21 = SDA, GPIO22 = SCL, pila propia   <- replicado en
    05_Funcional/15_Lista_de_Compras_Hardware.md:203 y en ESTADO.md seccion 4

05_Funcional/11_Manual_Instalacion_RTC_DS3231_Bateria.md:5
    "Modulo Externo DS3231 en pines libres PB0 (SDA) y PB8 (SCL) por I2C Software"

05_Funcional/Guia_Cableado_y_Pruebas_Banco.html:654
    "🕐 Reloj DS3231 - pines 40 y 39 - PB4  PB3 - SDA y SCL si se monta el reloj - J17 p1 y p4"
```

Tres destinos distintos para la misma pieza, ninguno marcado como caducado. El del Manual 11 ademas
hace dano fisico y tiene entrada propia: **N-99**.

#### 3. Los cuatro pulsadores: se retiran o se quedan

**MEDIDO:**

```
05_Funcional/17_...md:140
    "Los cuatro pulsadores (PB9, PB13, PB14, PB15) | libera J16"     <- se retiran

ESTADO.md seccion 4
    "Se retiran: la pantalla LCD de las dos puntas, los cuatro pulsadores ... y el mando"

05_Funcional/2_Manual_Hardware_y_Pruebas.md:11
    "| Botonera de 4 pulsadores | ✅ Se queda en AMBOS | Botones 1 a 4 (PB9, PB13, PB14,
     PB15) por el conector J16 |"
```

El `2_Manual` **si** recogio la retirada de la pantalla en la fila de al lado (`:10`), asi que no es
un documento sin tocar: es una tabla de estado **medio actualizada**, que es peor, porque su fila
buena avala a la mala.

---

#### El hallazgo estructural que lo explica: **el censo de la seccion B nacio desfasado**

Esto es lo que convierte tres erratas en un problema de metodo. **MEDIDO** con `git log`:

```
bdcf03d  28/08 19:00  docs: aviso en las dos maniobras que se quedan sin actuador   (Ordenes 3 y 4)
2e6baf4  28/08 19:03  docs: la lista de compras pedia lo contrario de lo decidido   (Orden 1)
3733544  28/08  --    docs: el documento del funcional para la arquitectura del 28/08 (Manual 17)
```

Los dos commits que reparan tres de los cinco Ordenes son **anteriores** al commit del propio
Manual 17. Y sin embargo el Manual 17 dice, en `:716`:

> **"Ninguno de estos ficheros se ha tocado al escribir este documento. Lo que sigue es el censo, no
> el arreglo."**

**Esa frase ya era falsa en el instante en que se escribio.** El propio documento lo avisa a medias
en `:11-13` —*"Habia otros trabajos en vuelo sobre el mismo arbol el dia que se escribio"*—, pero
avisar de que puede haber deriva no es lo mismo que medirla: la lista se publico como censo y se
lee como censo.

**Es `CLAUDE.md` §8.quinquies en directo, en su segunda forma.** Alli el dano de dos agentes sobre
el mismo arbol fue **la historia** (`ff6bd19` con un solo fichero, el acta). Aqui el dano es
distinto y peor: **un documento de arquitectura publica una lista de trabajo pendiente que ya
estaba hecha en parte**. Nadie escribio nada falso a proposito; simplemente ninguno de los dos
agentes miro el arbol del otro antes de publicar un censo.

**Consecuencia practica, y es la que cuesta la sesion:** hoy la seccion B **no se puede usar como
lista de trabajo**. Dos de sus cinco Ordenes (1 y 4) estan hechos, uno (3) esta hecho a medias y
ademas caducado, y solo dos (2 y 5) siguen enteros. Quien la coja de arriba abajo empieza por
reabrir la lista de compras, que es justo el documento que ya esta bien.

**Estado real de los cinco Ordenes, verificado fichero por fichero:**

| Orden | Documento | Estado | Evidencia |
|---|---|---|---|
| 1 | `15_Lista_de_Compras_Hardware.md` | ✅ **YA REPARADO** | `:85` A1 anulada, `:159` decision invertida, `:91` A6, `:361` B1 movido, `:413` repetidor separado |
| 2 | `10_Manual_Modulo_Bluetooth_Telemetria.md` | 🔴 **SIGUE FALSO, intacto** | un solo commit (`24276ab`); `:132`, `:134`, `:139`, `:144-146`, `:286` |
| 3 | `8_Procedimiento_Modo_Degradado.md` | 🟠 **PARCIAL y caducado** | avisos en `:8`, `:187`, `:200`, `:217`, `:276`, `:342`, `:369`, `:452`, `:464`; pero `:30-31`, `:41` y `:473` refutados por firmware — ver **N-100** |
| 4 | `04_Manuales/MANUAL_MANDO_4_RELES.md` | 🟠 **PARCIAL**, una fila hoy falsa | cabecera y tachado de `B·B·B` en `:425` correctos; `:439` refutado — ver **N-100** |
| 5 | `3_Protocolo_Pruebas_Rigurosas.md` | 🔴 **SIGUE FALSO, intacto** | un solo commit; `grep "28/08"` da **cero**; sigue emitido como V8.7 del 01/08 (`:3`) con el Repetidor en el entorno auditable (`:6`) |

**Y el segundo bloque, tambien verificado:** reparados `MAPEO_TARJETA_KICAD.md` (`02d913d`, y
`grep "vacio"` da cero), `1_Manual_Usuario.md` (`:8`, `:85`, `:153`) y `ESTADO.md` (`:37`, `:56`,
`:59`). Parciales `2_Manual_Hardware_y_Pruebas.md`, `9_Manual_Parametrizacion_Camara_IA.md`,
`04_Manuales/MANUAL_CONFIGURACION_CAMARAS_IA.md` y `13_Manual_Modulo_Expansion_I2C_y_Compras.md`
—cuya §4 sigue eligiendo rutas de I2C **sobre el STM32** (`:142-159`), que es lo que la arquitectura
del ESP32 deja sin objeto—. Intactos y falsos `11_Manual_Instalacion_RTC_DS3231_Bateria.md`
(**N-99**), `04_Manuales/MANUAL_CONFIGURACION_BLUETOOTH.md` y
`14_Manual_App_Movil_IOT_VIAL.md` —este ultimo manda `HC-05 / JDY-31` en `:6` y `:45`, y el
`JDY-31` esta **prohibido por nombre** en el Manual 10 §1 por ser BLE (`10_Manual...:39`)—.

**Y una fila que quedo mal en el unico documento que si vigila la compuerta:** `OPTIMIZACIONES.md:60`
publica *"SFTY-6: Timeout de fallback a **12.0s**"* y `:61` deriva de ahi la cuenta de reintentos de
SFTY-7. **MEDIDO:** `Maestro/include/protocolo.h:149` y `Esclavo/include/protocolo.h:149` =
`#define SFTY6_SILENCIO_MS 25000UL`. `ESTADO.md:59` ya lo corrigio con tachado; `OPTIMIZACIONES.md`,
que es **la tabla de trazabilidad `SFTY-x -> codigo -> prueba`**, no. Es N-71 volviendo al sitio
donde mas duele.

**Que haria falta para cerrarlo.** No es reescribir los documentos: es **decidir y anotar la fila**
del apartado 1 del Manual 10 —que su propia tabla `:144-146` deja abierta—, y despues propagar esa
unica decision a los otros dos ficheros con tachado y motivo, como ya se hizo bien en la lista de
compras. Y un pack que ate la cadena literal *"Decision de obra del 28/08"* a **una sola
direccion** en todo `05_Funcional/`: hoy la cadena aparece en tres ficheros y significa dos cosas
contrarias.

> **LECCION REUTILIZABLE: una decision no esta tomada mientras exista una copia que diga lo
> contrario, y arreglar UNA copia es lo que hace que el error se vuelva invisible.** Mientras los
> tres documentos estaban desfasados, cualquiera que abriera dos notaba el desfase. Arreglado uno,
> los otros dos quedan **avalados por su propia coherencia entre si** —y ademas fechados el mismo
> dia y marcados "decidido"—, asi que quien abra el equivocado no comete un error detectable:
> implementa fielmente la decision contraria con un documento que le da la razon. **Corolario de
> proceso: un censo de "que documentos quedan falsos" caduca en horas si otro agente esta tocando
> el arbol, asi que se levanta con `git log` sobre cada fichero en el momento de usarlo, no en el
> momento de escribirlo.**

---

---

### 🔴 N-97 — La camara de demanda no es la misma entrada en las dos puntas: en el Maestro vive dentro del Modo Inteligente y en el Esclavo vive siempre

**De donde sale:** del mismo censo de N-96, al comparar punta contra punta en vez de leer cada una
por separado. Es la familia del `amarillo = false` de mas de SFTY-2 (CLAUDE.md §3.ter): dos puntas
que dicen implementar lo mismo y no lo implementan igual.

---

#### 1. Lo MEDIDO — el mismo `#define`, dos entradas distintas

`pines.h` es identico en las dos puntas (`md5 8791a4c1f9afbe5e0e55adad2959b3eb`, ver N-96), asi que
la diferencia no esta en la declaracion:

```
Maestro/include/pines.h:46   #define CAM_DEMANDA_PIN  PB0   // R64 10K + C25 100nF -> J14
Esclavo/include/pines.h:46   #define CAM_DEMANDA_PIN  PB0   // identico
```

**Donde se configura y donde se lee, MEDIDO:**

| | Maestro | Esclavo |
|---|---|---|
| `pinMode(CAM_DEMANDA_PIN, INPUT)` | `modo_inteligente.cpp:46` — **dentro de `modoInteligente_setup()`** | `main.cpp:288` — **en `setup()`** |
| quien llama a eso | `main.cpp:205`, `case MODO_INTELIGENTE:` del `switch` de cambio de modo. **Unico llamador** (censado con `grep`, no leyendo) | nadie: es el `setup()` del arranque |
| lecturas | `modo_inteligente.cpp:98` y `:136`, las dos dentro del `loop` de ese modo | `main.cpp:350`, en el `loop()` principal |
| como se lee | **nivel**, con antirrebote software de 5 ms: `leerPinCamara()`, `modo_inteligente.cpp:21-30` | **flanco**, sin antirrebote software: `main.cpp:347-354`, `demandaCamaraActual && !demandaCamaraAnt` |

**Consecuencia:** en el Maestro, en Modo Manual, Automatico, Alcance, Hora, Degradado, Ambar o en el
Menu, **el pin `PB0` ni se configura ni se lee**. La camara del Maestro no existe fuera de un modo.
En el Esclavo la camara vive en todos los modos, porque su lectura esta en el `loop()` principal.

Y ni siquiera filtran igual: el Maestro exige nivel alto estable 5 ms (mas el RC de 1 ms de la
placa); el Esclavo se fia **solo** del RC de 1 ms y cuenta flancos. Con un rele de camara que rebote
mas de 1 ms —que es justo lo que el comentario del Maestro dice que puede pasar,
`modo_inteligente.cpp:22-24`— las dos puntas cuentan distinto el mismo gesto.

---

#### 2. Las citas enfrentadas: los documentos hablan de "las camaras" como una sola cosa

> `ESTADO.md:79` — *"…la **radio LoRa** (`USART3`, `J12`) **y las camaras**."*

> `ESTADO.md:117`, fila `FW-CAM` — *"`PB0`/`J14` es hoy **el unico camino de camara con firmware
> probado** (N-67 corregido, `pinMode(INPUT)` y `== HIGH` en **las dos puntas**, pack
> `camara_01_demanda`)."*

> `05_Funcional/17_...md:245-246` — *"La camara se arreglo: `pinMode(CAM_DEMANDA_PIN, INPUT)` y
> deteccion contra `HIGH` (`Maestro/src/modo_inteligente.cpp:19-25`, `:44`; `Esclavo/src/main.cpp:288`,
> `:350`)."*

**Contra lo MEDIDO:** las dos citas son **exactas en lo que afirman** —la polaridad si es `INPUT` y
`== HIGH` en las dos puntas, N-67 se cerro bien— y **el propio Manual 17 imprime la asimetria sin
verla**: cita `modo_inteligente.cpp` para una punta y `main.cpp` para la otra, en la misma linea, sin
que a nadie le llame la atencion que la misma entrada viva en un modo en un lado y en el arranque en
el otro.

Lo que ningun documento dice, y es lo que importa: **que en el Maestro esa entrada esta apagada en
siete de los ocho modos.** Un documento que dice *"las camaras"* describe un sistema simetrico que no
existe.

---

#### 3. Por que esto no lo caza el pack de camara

`banco/packs/camara_01_demanda.py` vigila lo que N-64 y N-67 dejaron: que `PB8` no vuelva a llamarse
`CAM_UMBRAL_PIN` (`:49-68`), que nadie lea `PB8` (`:84-86`), y la polaridad del `pinMode` de
`CAM_DEMANDA_PIN` (`:106`, `re.findall(r"pinMode\s*\(\s*CAM_DEMANDA_PIN\s*,\s*(\w+)\s*\)", codigo)`).

**Busca el `pinMode` por texto, en el codigo de la punta.** Lo encuentra igual este dentro de
`setup()` o dentro de `modoInteligente_setup()`: **el pack no tiene forma de saber quien llama a la
funcion que lo contiene.** Vigila la polaridad, que es lo que le pidieron; no vigila el alcance.

Es exactamente el punto ciego que `barrera_02_dos_puntas.py:5-6` describe para la otra regla —*"este
vigila lo otro, que faltaba: que `semaforo.cpp` DIGA LO MISMO en las dos puntas"*—. Para la barrera
de luz existe ese segundo pack. **Para la camara no existe.**

---

#### 4. Y de paso: hay un CUARTO `pines.h` que no compila ni censa nadie

MEDIDO. `01_Firmware/Semaforos/` es un proyecto completo en el arbol activo —`platformio.ini`,
`src/`, `include/`, `test/`, `compile_commands.json`— con su propio `include/pines.h`, que tambien
declara `BUZZER`. Y:

```
01_Firmware/compuerta.py:88    _ROLES = ("Maestro", "Esclavo", "Repetidor")
01_Firmware/compuerta.py:655   compilar("maestro",   "Maestro")
01_Firmware/compuerta.py:656   compilar("esclavo",   "Esclavo")
01_Firmware/compuerta.py:657   compilar("repetidor", "Repetidor")
```

**`Semaforos` no esta.** No se compila, la guarda de rutas no lo censa, y ningun pack lo lee. No es
un `ABORTADO` —nadie intento medirlo y fallo—: es un **hueco**, la clase que no deja rastro
(CLAUDE.md §3). Si es legado, su sitio es `99_Legacy/`, donde ya viven tres copias suyas; si esta
vivo, le falta un papel en `_ROLES`. Hoy no es ninguna de las dos cosas, y un `pines.h` sin vigilar
en el arbol activo es la clase de fichero que alguien acaba editando por error creyendo que es el
bueno.

*(No se ha tocado nada: solo se mide y se anota.)*

---

> **LECCION REUTILIZABLE: dos puntas pueden pasar el mismo pack con el mismo texto y no tener la
> misma entrada, porque un pack que busca una llamada por texto no sabe QUIEN llama a la funcion que
> la contiene. El alcance de un `pinMode` —arranque incondicional, o dentro del `setup()` de un modo—
> no se ve leyendo la linea: se ve censando los llamadores con `grep`, que es un segundo censo y hay
> que hacerlo aparte. Y su corolario para los documentos: cuando una nota cita `fichero_A.cpp` para
> una punta y `fichero_B.cpp` para la otra en la misma frase, esa asimetria de rutas ya esta escrita
> delante de quien la publica — se comprueba antes de resumir las dos como "las camaras".**

---

### 🔴 N-96 — La barrera de salidas dice gobernar OCHO pines de luz y gobierna SEIS: `ROJO_PEATON`, `VERDE_PEATON` y `BUZZER` estan declarados y muertos, y el pack que respalda la regla es vacuamente cierto sobre ellos

**De donde sale:** de censar la superficie de entrada y salida del firmware pin a pin y cruzarla con
lo que los documentos prometen (CLAUDE.md §3.ter). No lo pregunto nadie.

---

#### 1. El descarte del buscador, primero — porque sin el esto no es un hallazgo

Un *"no aparece"* no vale en este repositorio hasta haber descartado al buscador (CLAUDE.md §4).
Tres controles, y los tres son la parte reutilizable de este N-x:

**Control 1 — los dos `pines.h` son el MISMO fichero, asi que basta censar una lista.** MEDIDO:

```
md5sum 01_Firmware/Maestro/include/pines.h  ->  8791a4c1f9afbe5e0e55adad2959b3eb
md5sum 01_Firmware/Esclavo/include/pines.h  ->  8791a4c1f9afbe5e0e55adad2959b3eb
```

**27 `#define` con pin fisico en cada punta**, identicos. Cualquier asimetria que aparezca despues
no puede venir de la declaracion: viene del uso.

**Control 2 — se busco por macro Y por literal de pin crudo.** Buscar solo `BUZZER` seria confiar
en que nadie escribio `digitalWrite(PB1, ...)` a pelo. Se hicieron las dos pasadas sobre
`src/` + `include/` de las dos puntas, recursivo, `.cpp` y `.h`, excluyendo `.pio/` (libreria
ajena) y `.cache/`:

```
grep -rn "\bBUZZER\b"  Maestro/src Maestro/include Esclavo/src Esclavo/include   ->  0
grep -rn "\bPB1\b"     (los mismos directorios)                                 ->  solo pines.h:20
grep -rn "\bROJO_PEATON\b|\bVERDE_PEATON\b"                                     ->  solo un COMENTARIO
grep -rn "\bPA6\b|\bPA7\b"                                                      ->  solo pines.h:15-16
```

**Control 3 — el buscador SI sabe encontrar.** El mismo patron sobre un pin vivo:

```
grep -rc "\bROJO1\b" Maestro/src/semaforo.cpp   ->  2   (pinMode :193 y digitalWrite :49)
```

Y ampliado al repositorio entero —sin `.pio/`, sin `.git/`— `BUZZER` aparece en **6 `pines.h`,
5 `.kicad_pcb` y `roadmap.md`**, y en **ningun `.cpp` de ningun proyecto**. La palabra existe, el
grep la encuentra donde esta, y no esta en el firmware.

---

#### 2. Lo MEDIDO: `escribirPines()` escribe seis, no ocho

`Maestro/src/semaforo.cpp:48-54` y `Esclavo/src/semaforo.cpp:48-54`, la funcion entera:

```c
digitalWrite(ROJO1, rojo);          digitalWrite(ROJO2, rojo);
digitalWrite(AMARILLO1, amarillo);  digitalWrite(AMARILLO2, amarillo);
digitalWrite(VERDE1, verde);        digitalWrite(VERDE2, verde);
```

**Seis.** Y `semaforo_setup()` (`:193-198` en las dos puntas) hace `pinMode` a esos mismos seis.

| pin | nombre | bornera | `pinMode` | leido | escrito | veredicto |
|---|---|---|---|---|---|---|
| `PA6` | `ROJO_PEATON` | `J11` | **NO** | **NO** | **NO** | 🔴 declarado y muerto |
| `PA7` | `VERDE_PEATON` | `J9` | **NO** | **NO** | **NO** | 🔴 declarado y muerto |
| `PB1` | `BUZZER` | `J13` | **NO** | **NO** | **NO** | 🔴 declarado y muerto |

Los tres, iguales en las dos puntas. `ROJO_PEATON` y `VERDE_PEATON` aparecen **una sola vez** fuera
de `pines.h` en todo el firmware, y es dentro de un comentario: `Maestro/src/main.cpp:35`. `BUZZER`
no aparece ni ahi.

---

#### 3. Las citas enfrentadas — tres documentos, y no pueden ser ciertos a la vez

**Lo que dice la regla permanente:**

> `CLAUDE.md` §6 — *"**Solo `semaforo.cpp` escribe pines de luz.** Los ocho: `ROJO1/2`,
> `AMARILLO1/2`, `VERDE1/2`, `ROJO_PEATON`, `VERDE_PEATON`. Todo pasa por su `escribirPines()`
> estatico"*

**Lo que dice el propio fuente, con la palabra CUSTODIA dentro:**

> `Maestro/src/main.cpp:34-35` — *"NINGUN pin de luz se escribe fuera de semaforo.cpp. Los OCHO,
> incluidos ROJO_PEATON y VERDE_PEATON, que estaban sin custodia."*

**Contra lo MEDIDO:** `semaforo.cpp:48-54` escribe seis. `PA6` y `PA7` no pasan por
`escribirPines()` porque **no pasan por ningun sitio**.

La frase de §6 es **literalmente cierta y vacuamente cierta a la vez**: ningun pin de luz se escribe
fuera de `semaforo.cpp`, en efecto — dos de ellos tampoco dentro. La palabra *"custodia"* sugiere
que hay algo vigilado; lo que hay es un pin que nadie toca.

**Y sobre el buzzer, dos documentos lo dan por vivo y uno lo declara muerto:**

> `ESTADO.md:79` — *"El STM32 sigue siendo el controlador del semaforo. Conserva las **8 luces**
> (`J3`-`J9`, `J11`), la **barrera** (`PB2`, `J15`), el **buzzer** (`PB1`, `J13`), la **radio LoRa**
> (`USART3`, `J12`) y las camaras."*

> `05_Funcional/17_Arquitectura_28-08_y_Decisiones_Abiertas.md:68-78`, seccion **1.2 Que se queda en
> el STM32**, encabezada en `:70` por *"Todo esta MEDIDO en `01_Firmware/Maestro/include/pines.h`"* —
> `:76` *"| Rojo peaton / Verde peaton | `PA6` `PA7` | `J11` `J9` | `pines.h:15-16` |"* · `:78`
> *"| Buzzer | `PB1` | `J13` | `pines.h:20` |"*

> `OPTIMIZACIONES.md:1427` — *"Es hardware pagado y muerto, igual que el semaforo peatonal
> (`PA6`/`PA7`) y el buzzer (`PB1`)."*

**El que coincide con la medida es `OPTIMIZACIONES.md`.** El hardware muerto no es el hallazgo: ya
estaba escrito bien en un sitio. El hallazgo es que otros dos documentos lo publican como funcion
conservada.

> ⚠️ **Y el "MEDIDO" del Manual 17 §1.2 es lo que lo hace peor, no mejor.** Lo medido es que la
> linea existe en `pines.h` — cierto, repetible, y **no es lo que el titulo de la tabla promete**.
> La tabla se llama *"Que se queda en el STM32"*, que un lector entiende como *funcion que el equipo
> conserva*. Tres de sus siete filas —peatonal, buzzer, y la camara del Maestro de N-97— son
> declaraciones sin firmware detras. **Una marca MEDIDO responde a la pregunta que se le hizo, no a
> la que el lector cree que se le hizo.**

---

#### 4. Lo grave: el pack que respalda la regla NO PUEDE detectarlo

`01_Firmware/Simulaciones/banco/packs/barrera_01_pines_de_luz.py`, MEDIDO linea a linea:

- **Solo mide propiedades negativas.** `:81-97` recorre los `.cpp` de cada punta saltandose
  `PERMITIDOS = ("semaforo.cpp",)` (`:52`) y busca *fugas* hacia fuera. **En ningun sitio comprueba
  que un pin de luz declarado sea escrito DENTRO.** Un pin que nadie escribe pasa la barrera por
  definicion: no puede fugarse lo que no se mueve.
- **La guarda de recuento acepta la perdida.** `:74` — `len(luces) >= 6`. Su propio regex (`:44`,
  `^(ROJO|AMARILLO|VERDE)`) devuelve **8** al correrlo sobre `pines.h` — MEDIDO, ejecutado. **Si
  alguien borrara manana los dos `#define` peatonales, el pack seguiria en verde**, y el mensaje de
  exito (`:75-76`) seguiria diciendo *"todos entran bajo custodia"*.
- **El control negativo nunca ha ejercido un peatonal.** `:102-108` inyecta la fuga sobre
  `luces_m[0]`, que es `ROJO1`: un pin **vivo**. El control demuestra que el regex casa; no demuestra
  que la regla sepa distinguir un pin gobernado de uno abandonado.

El pack cumple exactamente lo que promete su `DESCRIPCION` (`:37`, *"ningun pin de luz se escribe
fuera de semaforo.cpp"*). Lo que sobra es la lectura de `CLAUDE.md` §6 y de `main.cpp:34-35`, que
convierten esa propiedad negativa en *"los ocho estan gobernados"*.

Es el patron de N-51 con otra cara: **un `PASS` de algo que nadie ha visto fallar nunca**, y un
numero —`>= 6`— que no coincide con el que el fichero declara.

---

#### 5. Lo que hay que decidir, y no lo decide el firmware

Las tres salidas tienen canal de potencia completo en la placa (`OPTIMIZACIONES.md:1486` para el
buzzer: `R55`+`R54` -> opto `U13` -> MOSFET `Q8` -> bornera `J13`). Que se implementen o no es
decision de operacion. **Lo que no es opcional es que los documentos digan cual de las dos cosas
es.** Hoy hay tres respuestas publicadas y solo una coincide con el codigo.

---

> **LECCION REUTILIZABLE: una regla de seguridad enunciada en NEGATIVO —"nadie escribe X fuera de
> aqui"— es vacuamente cierta sobre todo sujeto que nadie escribe, y su pack no puede notar la
> diferencia entre un pin gobernado y uno abandonado. Cuando la regla se resume como "los N estan
> bajo custodia", hace falta la mitad POSITIVA: que cada sujeto declarado aparezca dentro de la
> puerta unica. Y el sintoma que lo delata sin leer el pack es una comparacion de recuento con
> holgura —`>= 6` sobre una lista que declara 8—: una guarda que acepta menos sujetos de los que
> existe no esta contando, esta permitiendo.**

---

---

### 🟠 N-95 — `PA8` sobrevivio a su motivo, y el comentario que lo justifica describe un equipo que ya no existe

**De donde sale:** de la pregunta *"el desacoplo de `PA8`, ¿sigue siendo una barrera viva o quedo
como resto de la epoca de `PA9`/`PA10`?"*. Se midio antes de leer lo que dijera ningun documento, y
salieron **dos** afirmaciones del fuente que el propio fuente contradice.

**MEDIDO — netlist del `.kicad_pcb`, extraido por huella:**

```
   U2 pad1 (RO)   -> red /PA10   ->  U1 pad 31  (PA10)
   U2 pad2 (~RE)  -> red /PA8   \
   U2 pad3 (DE)   -> red /PA8   /->  U1 pad 29  (PA8)    <- un solo nivel manda sobre las dos mitades
   U2 pad4 (DI)   -> red /PA9    ->  U1 pad 30  (PA9)
   U2 pad6/pad7   -> J10 pin1 / pin2

   U3 pad1 -> /PB11 · pad2,pad3 -> /PB12 · pad4 -> /PB10 · pad6,pad7 -> J12
```

Confirmado de paso que la correccion `U3` -> `U2` que arrastran los comentarios **es la buena**: `U3`
es el de la radio LoRa y no toca el `USART1`. Eso ya no esta en disputa.

**MEDIDO — firmware, identico en las dos puntas** (`Maestro/src/bluetooth.cpp:68-69`,
`Esclavo/src/bluetooth.cpp:76-77`):

```cpp
pinMode(RS485_IN_DE_RE, OUTPUT);
digitalWrite(RS485_IN_DE_RE, HIGH); // Apaga el receptor RO de U2 y libera PA10 al modulo Bluetooth
```

#### Refutacion 1 — el motivo escrito encima de la linea es falso

> **REFUTADO.** `Maestro/src/bluetooth.cpp:69` afirma:
> *"Apaga el receptor RO de U2 y **libera PA10 al modulo Bluetooth**"*.
> Y `Maestro/include/pines.h:109` lo repite en el `#define`:
> *"HIGH: apaga el receptor de U2 y **libera PA10** (el TX de U2 queda activo)"*.
>
> **Contra:** `Maestro/src/bluetooth.cpp:28` — `static HardwareSerial SerialBT(PB7, PB6);`
>
> **El modulo Bluetooth no esta en `PA10` desde N-76. No hay nada que liberar.** El beneficio se fue
> con el remapeo a `J17`; el coste se quedo. La linea sobrevivio a su numero y el comentario le
> quedo encima, **con la autoridad de una cuenta hecha**.

#### Refutacion 2 — el efecto que el fuente describe ya no puede ocurrir

Esta es la que no estaba vista, y es la peor de las dos porque **es la que alguien usaria para
decidir si toca `J10`**.

> **REFUTADO.** `Maestro/src/bluetooth.cpp:60-61` y `Maestro/include/pines.h:105-106` afirman:
> *"U2 vuelca **la telemetria** por J10 de forma permanente"* / *"con PA8 en HIGH ... **J10 emite la
> telemetria** de forma permanente y no puede recibir nunca"*.
>
> **Contra, MEDIDO:** el `DI` de `U2` es `PA9`, y sobre todo `01_Firmware/` (excluyendo
> `Simulaciones/`) **no hay un solo `pinMode` ni `digitalWrite` sobre `PA9` ni `PA10`**:
>
> ```
> $ grep -rn "RS485_IN_TX\|RS485_IN_RX" --include=*.cpp --include=*.h 01_Firmware/
> Maestro/include/pines.h:98   #define RS485_IN_RX     PA10      <- solo el #define
> Maestro/include/pines.h:99   #define RS485_IN_TX     PA9       <- solo el #define
> Maestro/src/protocolo.cpp:20 // (RS485_IN_RX, RS485_IN_TX) = ...  <- solo un comentario
> Esclavo/include/pines.h:98   ...  (identico)
> Esclavo/src/protocolo.cpp:20 ...  (identico)
> ```
>
> `protocolo_setup()` retiro la apertura de `AiBus` (N-76) y N-86 retiro el objeto entero
> (`Maestro/src/protocolo.cpp:16-46`), y el `USART1` se fue a `PB6`/`PB7`. **El `DI` de `U2` es una
> entrada flotante.** Por `J10` no sale telemetria: sale **un nivel indeterminado**. La frase describe
> el equipo de antes de N-76 y **se ha copiado a dos ficheros**.

*(El Manual 10 §2.5 —`05_Funcional/10_Manual_Modulo_Bluetooth_Telemetria.md:361-430`— ya reclasifico
`PA8` como **"RESIDUO PENDIENTE DE REVISAR"** el 28/08 y observa lo del `DI` sin gobierno en `:420-427`.
**LEIDO**, y coincide con lo medido aqui de forma independiente. Lo que ese manual no cubre, y es lo
que este N-x anade, es que **los comentarios del fuente siguen diciendo lo contrario** en cuatro
sitios: `bluetooth.cpp:60-61` y `:69` y `pines.h:105-106` y `:109`, por duplicado en las dos puntas.
Un manual corregido y un fuente sin corregir es peor que los dos mal: quien lee el `.cpp` no sabe que
hay un manual que lo desmiente.)*

#### El matiz que impide el arreglo obvio

La tentacion, leidas las dos refutaciones, es borrar el `digitalWrite`. **Es la respuesta
equivocada.**

| `PA8` | receptor (`RO` -> `PA10`) | transmisor (`DI` <- `PA9`, salida a `J10`) |
|:---:|---|---|
| `LOW` | escuchando: `U2` **conduce** `PA10` desde fuera | apagado |
| **`HIGH`** *(hoy)* | Hi-Z — `PA10` libre, **que ya no le hace falta a nadie** | **encendido permanente sobre el par A/B de `J10`, con el `DI` flotando** |
| **sin configurar** *(si se borra la linea)* | `~RE` y `DE` **flotan**: el `MAX3485` queda en estado indefinido | idem |

`PA8` **tiene que quedar en un nivel determinista**; lo que hay que decidir es **cual**. Borrar la
linea deja las dos mitades del transceptor al aire, que es peor que cualquiera de los dos niveles.

> **La pregunta abierta no es "se borra o se queda": es "HIGH o LOW", y hoy `HIGH` no tiene ningun
> argumento a favor.** Con `HIGH`, el transmisor de `U2` esta permanentemente tomado sobre `J10` con
> una entrada flotante detras — que es literalmente el fallo del repetidor del 31/07/2026 ya escrito
> en `01_Firmware/TROUBLESHOOTING.md:48` y `:55`: *"si un DE/RE se queda permanentemente en alto, esa
> linea queda bloqueada en ambos sentidos"*. Alli fue una averia; aqui dejo de ser una decision y
> volvio a ser lo que era alli.

**A quien bloquea y a quien no:**

- **Al enlace del ESP32: a NADIE.** `U2` no toca `PB6` ni `PB7` en ninguna de sus cuatro patas
  (MEDIDO arriba). Esto **no** es un bloqueante de N-94 ni del firmware del ESP32.
- **A `J10`: si.** El puerto RS-485 de `J10` no esta libre, esta **tomado y sordo**. Y `J10` es
  precisamente el segundo puerto del que depende la idea de portar el repetidor a esta misma placa
  (`03_Hardware_Tarjeta/MAPEO_TARJETA_KICAD.md` §5, que lo lista como *"hoy esta vacio, asi que el
  segundo puerto no necesita ni un hilo nuevo"* — **LEIDO, y es optimista**: el hilo no hace falta, el
  cambio de firmware si).
- **Que nivel presenta hoy `J10` en la placa: SIN VERIFICAR.** No se ha medido nunca, y no se afirma
  aqui. Es una de las cosas que la sesion de banco tiene que mirar.

**Como se cierra:** su propio `N-x`, con su pack, no colado dentro de otro cambio. Y la parte que
**si se puede hacer ya y no toca comportamiento**: corregir los cuatro comentarios refutados
—`bluetooth.cpp:60-61`, `bluetooth.cpp:69`, `pines.h:105-106`, `pines.h:109`, en las dos puntas—,
porque hoy apuntan a quien venga a medir en la direccion equivocada.

> ⚠️ **Y no viajan solos: hay una tercera afirmacion caduca del mismo remapeo.**
> `Maestro/include/reloj.h:10` sigue diciendo *"el I2C por hardware esta copado: **PB6/PB7 los usa la
> LCD**"*, y `pines.h:88-89` sigue definiendo `LCD_PSB PB6` / `LCD_RST PB7`. **MEDIDO:** el consumidor
> real de esos dos pines es `SerialBT` desde N-76, y `lcd.cpp:29` construye el `U8G2` con
> `U8X8_PIN_NONE` y solo `SCLK`/`SID`/`CS` — **cero llamadas a `LCD_PSB` y `LCD_RST`**. Los `#define`
> no rompen nada porque nadie los usa; el comentario de `reloj.h` si engana, porque **razona una
> decision de arquitectura** —por que el RTC no va por I2C— sobre un hecho que dejo de ser cierto.

> **La regla que queda: cuando un cambio mueve un periferico de pines, lo que hay que censar no son
> los `#define` que se quedan huerfanos —esos son inertes— sino los COMENTARIOS QUE EXPLICAN POR QUE.**
> Un `#define` sin llamadores no hace dano; una frase que justifica una decision con un hecho caducado
> **sigue tomando decisiones**, porque el siguiente la lee y no vuelve a medir. Aqui fueron cinco
> frases en cuatro ficheros y las dos puntas, todas correctas el dia que se escribieron, todas falsas
> hoy, y **todas con la palabra que las blinda: `MEDIDO`, `en la PCB`, `trazado red por red`**. El
> censo de un remapeo es `grep` del nombre del pin viejo —`PA9`, `PA10`— **en los comentarios**, no
> solo en el codigo.

---

### 🔴 N-94 — El transporte del enlace `J17` no lo vigila ningun pack, y el contrato de bytes que el ESP32 tiene que cumplir no esta escrito en ningun sitio

**De donde sale:** de validar si §1.4 del Manual 17 —la tabla pin a pin del enlace ESP32 <-> STM32—
es implementable exactamente como esta escrita. **Lo es: las siete filas coinciden.** Lo que aparecio
al comprobarlo es lo de siempre en este repositorio: la cifra estaba bien y **nadie la vigilaba**.

**Primero, lo que SI esta bien, porque un hallazgo empieza descartando al buscador (CLAUDE.md §4).**
`§1.4` se verifico contra tres fuentes independientes y las tres casan:

```
MEDIDO — fuente:
  Maestro/src/bluetooth.cpp:28   static HardwareSerial SerialBT(PB7, PB6);
  Esclavo/src/bluetooth.cpp:26   static HardwareSerial SerialBT(PB7, PB6);
  Maestro/src/bluetooth.cpp:70   SerialBT.begin(9600);
  Esclavo/src/bluetooth.cpp:78   SerialBT.begin(9600);

MEDIDO — framework, NO el comentario del .cpp:
  C:/.platformio/.../cores/arduino/HardwareSerial.h:111
      HardwareSerial(uint32_t _rx, uint32_t _tx, ...)      <- el PRIMER argumento es RX
  C:/.platformio/.../cores/arduino/HardwareSerial.h:116-119
      void begin(unsigned long baud) { begin(baud, SERIAL_8N1); }

MEDIDO — netlist del .kicad_pcb (185 huellas, extraido balanceando parentesis, no con grep de token):
  J17 pad 2 -> /RST        U1 pad 43 -> /RST        (LQFP48 pad 43 = PB7)
  J17 pad 3 -> /RS(A0)     U1 pad 42 -> /RS(A0)     (LQFP48 pad 42 = PB6)
  J17 pad 7 -> GND         J17 pad 9 -> GND
  J17 pad 1 = /CS · 4 = /SCL · 5 = /SI · 6 = /3.3V · 8 = /3.3V · 10-13 sin red
```

`GPIO17` (TX2) -> `J17` p2 -> `PB7` **RX** del micro; `GPIO16` (RX2) <- `J17` p3 -> `PB6` **TX**.
**El cruce esta bien puesto en el documento y bien puesto en el cobre.** Una sola cautela de
etiqueta: **`8N1` no lo elige nadie en este repositorio** — sale del valor por defecto de
`HardwareSerial::begin(baud)`. Es cierto, pero es una herencia de libreria, no una decision escrita.

> 🔴 **Y aqui esta el hallazgo: NADA de lo anterior lo comprueba el banco.** Censo sobre los **38
> packs** de `01_Firmware/Simulaciones/banco/packs/`:
>
> ```
> $ ls packs/*.py | grep -v __init__ | wc -l
> 38
> $ grep -rln "SerialBT\|HardwareSerial\|PB6\|PB7\|J17\|begin(9600)\|USART1" packs/*.py modelos/*.py
> packs/flash_01_lastre.py
> ```
>
> **Un solo fichero, y no cuenta.** `flash_01_lastre.py:180` contiene la cadena
> `" el I2C por hardware esta copado: PB6/PB7 los usa la LCD "` **dentro de un
> `control_negativo`**, como texto sintetico para demostrar que una mencion en prosa no es un uso del
> bus. No lee el firmware: **es una cadena literal escrita dentro del pack**.
>
> **Siete packs SI abren `bluetooth.cpp`** —`app_01_comandos`, `app_02_modos_simetricos`,
> `app_03_sin_ok_mudo`, `documentos_03_trama_status`, `esclavo_06_no_abre_paso`,
> `esclavo_07_ambar_emergencia`, `maestro_08_set_tiempos`— y **ninguno de los siete mira el
> transporte**: leen comandos del despachador, la cadena de formato de `$STATUS`, las ramas de
> `$ACK`/`$ERR` y los limites de `SET_TIEMPOS`. Todos miran **lo que se dice**; ninguno mira **por
> donde y a que velocidad se dice**.

**Que significa en la practica:** hoy alguien puede escribir `SerialBT(PA10, PA9)` o
`SerialBT.begin(115200)` en las dos puntas y **la compuerta sale en verde, el banco da `411/411` y el
acta lo firma**. El equipo se queda mudo en la calle y ningun instrumento lo dijo. Es exactamente el
hueco de CLAUDE.md §3: *un instrumento que no esta en la compuerta no mide nada — y no deja rastro de
que falta*. **Un `ABORTADO` grita; un hueco no.** Y este hueco es especialmente caro ahora, porque
N-76 acaba de mover ese puerto y el 28/08 acaba de decidir que por ahi entra el ESP32: **es el unico
cable entre el controlador del semaforo y su unica superficie de mando futura.**

#### El contrato de bytes, que tampoco esta escrito en ningun sitio

`§1.4` da los pines y la velocidad, y ahi se acaba. **Todo lo demas que el firmware del ESP32 tiene
que cumplir esta implicito en el `.cpp` y no aparece en ningun documento.** Deducido del fuente,
MEDIDO linea por linea, identico en las dos puntas:

| regla | de donde sale | consecuencia si el ESP32 no la respeta |
|---|---|---|
| **Terminador `\r` o `\n` obligatorio** | `Maestro/src/bluetooth.cpp:391` (`if (c == '\n' \|\| c == '\r')`) · `Esclavo/src/bluetooth.cpp:297` | sin terminador **`procesarComando()` no se llama nunca**. El equipo no contesta y no hay error: el comando se queda en el buffer |
| **Maximo 63 caracteres utiles antes del terminador** | `btBufIn[64]` (`Maestro:31`, `Esclavo:29`) con la guarda `btIdxIn < sizeof(btBufIn) - 1` (`Maestro:397`, `Esclavo:303`) | **descarte SILENCIOSO del caracter 64 en adelante**, y el despachador recibe una linea truncada que casara con un `strcmp` equivocado o con ninguno |
| **El STM32 NO valida el checksum de entrada** | `procesarComando()` empieza en `Maestro/src/bluetooth.cpp:135` con `strcmp(cmd, "CMD:FORZAR_ROJO")` — no hay lectura de `*XX` en ninguna rama | el ESP32 **puede** mandar comandos sin checksum, pero tambien significa que **el enlace no tiene deteccion de error en el sentido ESP32 -> STM32**. Un byte corrompido no se rechaza: se compara |
| **El STM32 SI lo emite, siempre** | `enviarTramaConCrc()`, `Maestro/src/bluetooth.cpp:42-48`: `snprintf(tramaCompleta, ..., "%s*%02X\r\n", payload, crc)` | el ESP32 **tiene que verificarlo** en el sentido STM32 -> ESP32, o se estara fiando de tramas sin comprobar |
| **XOR-8 saltando el `$` inicial** | `calcularChecksum()`, `Maestro/src/bluetooth.cpp:33-40`, invocado como `calcularChecksum(payload + 1)` en `:43`. Recorre hasta `'\0'` **o hasta `'*'`** | un ESP32 que calcule el XOR incluyendo el `$` rechazara **todas** las tramas buenas |
| **Telemetria no solicitada cada 2000 ms** (1000 hasta el 04/09) | `Maestro/src/bluetooth.cpp:403` (`if (ahora - tUltimaTelemetria >= 1000)`) | el enlace **no es pregunta-respuesta**: llegan `$STATUS` sin pedirlos, mas `$ALARM` y `$EVENT` asincronos. Un parser que espere respuesta a su comando leera un `$STATUS` como respuesta |
| **La autenticacion es un literal en claro** | `strncmp(cmd, "CMD:PIN:1234:", 13)`, `Maestro/src/bluetooth.cpp:166` | no cambia por el ESP32, pero el ESP32 pasa a ser **quien lo transporta**, y eso es una decision de seguridad que hereda |

> ⚠️ **Correccion de una cifra propia, dentro de este mismo N-x.** El primer informe de esta
> validacion publico *"maximo 62 caracteres"*. Es **falso**: la guarda es `btIdxIn < 63`, asi que se
> escriben los indices `0..62` —**63 caracteres**— y el `'\0'` cae en el `63`. Se corrige aqui en vez
> de en silencio, porque **una cifra que desaparece vuelve a proponerse** (CLAUDE.md §4). Y la
> leccion de fondo es la de siempre: la cuenta se hace sobre la condicion del `if`, no sobre el
> tamano del array.

**Que hace falta para cerrarlo.** Tres cosas, y las tres antes de soldar el primer hilo:

1. **Un pack que fije el transporte** — que relea del C++ los pines de la declaracion de `SerialBT`,
   la velocidad de `begin()` y que las **dos puntas** sean identicas (`bluetooth.cpp:24` del Esclavo
   ya declara esa simetria como intencion: *"IDENTICO AL MAESTRO A PROPOSITO"*, y hoy nada la
   comprueba). **Sin valor por defecto**, releido en cada corrida.
2. **Y verlo fallar antes de conectarlo** (CLAUDE.md §8.bis): se inyecta `SerialBT(PA10, PA9)` en el
   `.cpp` real, se exige que **baje la cuenta y cambie el codigo de salida**, y se restaura
   verificando con `git diff HEAD` vacio.
3. **Escribir el contrato de bytes** —las siete filas de arriba— en `§1.4` del Manual 17 o en el
   Manual 10, porque hoy quien escriba el firmware del ESP32 tiene que **deducirlo leyendo
   `bluetooth.cpp`**, y las dos reglas que muerden —el terminador y el descarte silencioso a los 63
   caracteres— fallan **sin sintoma**: no hay error, no hay `$ERR`, no hay nada. El equipo
   simplemente no contesta.

**Y un dato que NO es un bloqueante pero conviene tener escrito:** al STM32 **le da exactamente igual
que hay al otro lado**. Se grepearon `AT+`, `HC-05`, `HC05`, `JDY`, `SPP`, `pairing` y `emparej` sobre
`Maestro/src`, `Maestro/include`, `Esclavo/src` y `Esclavo/include` —y el buscador **si encuentra
cosas**, cinco aciertos de `emparej` y tres de `115200`, asi que sabia buscar—: **cero comandos AT,
cero secuencia de configuracion, cero pin de KEY/EN**. La unica mencion a `AT+NAME` esta en un
comentario de documentacion, `Maestro/include/identidad.h:42`. `bluetooth_setup()`
(`Maestro/src/bluetooth.cpp:66-71`) hace tres cosas y ninguna es del modulo. **Sustituir el `HC-05`
por un ESP32 no cuesta ni una linea de firmware del STM32** — cuesta exactamente el pack que no
existe.

> **La regla que queda: un contrato que solo vive dentro de un `.cpp` no es un contrato — es una
> arqueologia que el siguiente tiene que hacer.** Y su otra mitad, que es la que este repositorio ya
> conoce con otro disfraz: **el transporte de un enlace se vigila con un pack, igual que su
> contenido.** Siete packs leen lo que dice `bluetooth.cpp` y ninguno por donde lo dice; el dia que
> alguien mueva el puerto, los siete seguiran en verde midiendo el mensaje de un equipo mudo.

---

---

## 7. Lo anterior

**Este roadmap arranca el 31/08/2026.** El historico del proyecto —los `N-1` a `N-93`, las versiones
V8.0 a V8.9 y las auditorias de agosto— **no se ha borrado**: vive en el `git log` de este
repositorio y en el remoto `padre` (`git log padre/main -- roadmap.md`). No se arrastra aqui porque
un roadmap que crece por acumulacion deja de servir para decidir, que es para lo unico que existe.

**Lo que si se conserva y no se pierde nunca son las reglas**: viven en `CLAUDE.md`, que es el
fichero que se lee solo en cada sesion. Si un `N-x` de los viejos dejo una regla, esa regla esta
alli — y si no lo esta, es que no la dejo.
