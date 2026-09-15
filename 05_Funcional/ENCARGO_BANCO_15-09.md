# ENCARGO DE BANCO del 15/09/2026 — firmware del commit `ca2de3d`

**Esto es una PETICION DE MEDIDA, no una version.** Pide que alguien ponga dos tarjetas delante y mire. No
autoriza instalar nada, no autoriza quitar el sufijo `SIN_BANCO` y no autoriza cargar el equipo del Sisga.

## 0. Antes de leer nada mas

- **Lo que corre en campo hoy** (lo dice `ESTADO.md`, no este fichero): la instalacion certificada es la
  **V8.4, commit `e303485`**. Ademas, el **Maestro del Sisga** (`SERIE:179DB0`) corrio el 10/09 con **V9
  `SIN_BANCO`**: se cargo `7ff7d12` y despues se probo `b354fe9`. **Cual quedo dentro no consta**: se lee
  antes de tocar ese equipo. El firmware del **Esclavo del Sisga** esta SIN VERIFICAR (su cinta no se trajo).
- **¿Ha pasado banco? NO.** Nada de lo construido desde el 05/09 ha visto una tarjeta.
- Es el **candidato a primer estable**: estable = este hash con banco pasado y cargado. Hoy no es ninguna.

## 1. ALTO — lo que se hace ANTES de enchufar nada

1. **Cargar por SWD con `mode=UR` y `-e all`. No se cambia el modo.** Si falla, se reintenta: `Unable to get
   core ID` no es un cable. El delator de un enganche malo es `NVM size: 128 KBytes (default)`.
2. **Verificar lo CARGADO, no lo compilado:** `sha256sum` del fichero contra §2 antes de cargar, y despues
   leer la flash de vuelta y sacar su `sha256sum`, que tiene que dar el mismo numero. Nunca por el tamano.
3. **`J16` p1 lleva 12 V crudos a un conector de senal directa al micro: se RETIRA EL PIN del conector
   volante** en las dos tarjetas, antes de cualquier otro cable en `J16`.
4. **`J14` es una ENTRADA del micro (3,3 V, sin opto ni diodo): en `J14` no se conecta NADA.** La
   talanquera va en `J15`. Un rele cableado a `J14` se desconecta antes de energizar.
5. **`J16` p5 y p8 no se cablean ni se puentean** (`SPEC_5` §3). Unica excepcion: la prueba C, en esta
   mesa, y **nunca en el equipo del Sisga**.
6. **El gesto de prueba de camara es `p10` contra `p9` y `p12` contra `p11`** (3,3 V contiguo). **Nunca
   contra masa.** En todo `J16` hay una sola masa, `p2`.
7. **Radios:** `2.4 kbps` de Air Data Rate, `M0`/`M1` en OFF, las dos iguales. **Nunca se energiza una
   radio sin la antena puesta.**

```
STM32_Programmer_CLI -c port=SWD mode=UR -e all -w Maestro_2026-09-15_ca2de3d_SIN_BANCO.bin 0x08000000 -v -rst
STM32_Programmer_CLI -c port=SWD mode=UR -u 0x08000000 42228 leido_maestro.bin
sha256sum leido_maestro.bin      (y lo mismo con el Esclavo: 36532 B)
```

- SIN VERIFICAR: la guia de banco escribe la carga sin la direccion `0x08000000`, y la orden de lectura
  (`-u`) no esta usada en este repositorio. Si la lectura no funciona: se pega la salida de `-v` y se anota
  **lectura NO HECHA**.
- sha256 leido del Maestro: ______
- sha256 leido del Esclavo: ______

## 2. Lo que se carga

- Maestro: `Maestro_2026-09-15_ca2de3d_SIN_BANCO.bin` · 42228 B · sha256
  `317f8e463cb461413594148102e2f670e12f922d029a23ce12de66f1593157f4`
- Esclavo: `Esclavo_2026-09-15_ca2de3d_SIN_BANCO.bin` · 36532 B · sha256
  `3884e6afaf75d38338f9be183332f1aa8673c24788928d22069e47afd3773434`
- App: `IOT_VIAL_Semaforos_2026-09-15_622a20b_SIN_BANCO.apk` · 4.020.773 B · sha256
  `811f6399555f02fa0e53fe8bd74038013be5a7d0fefbfff5fd58593eb3f03688`
  - La APK lleva el nombre del commit del que salio (`622a20b`): la app no cambio despues
    (`git diff --name-only 622a20b..ca2de3d -- 05_Funcional/App_Semaforo` sale vacio).
  - Se verifica ESTE fichero: recompilada en otro PC, la misma APK puede dar otro sha256 (finales de linea).
- Puente ESP32 (el mismo en los dos postes): `ESP32_Expansion_2026-09-15_ca2de3d_SIN_BANCO.bin` · 1130096 B · sha256
  `171ceb3cf5c1db897c34a9cb0c04456cd509e79a0cb0c1198e886b90d5534437`
  - Compilado desde cero dos veces con el mismo sha256. El puente cambio despues de `7ff7d12` (`D-26`): con uno
    viejo la hora no llega al STM32 y E2 y D no se pueden hacer.
- Paquete: `Paquete_Banco_2026-09-15_ca2de3d_SIN_BANCO.zip`.

**El sufijo `SIN_BANCO` no se quita al renombrar. Lo quita quien lo haya probado en un equipo.**

## 3. Lo que dice la compuerta, y por que no es un permiso

- Copiado de `evidencia/2026-09-15_compuerta.txt`: **18 PASS · 1 FALLA · 0 ABORTADO**; banco por packs
  **1416/1417**; arnes de las dos puntas **122/122**; Degradado a dos puntas **71/71**; flash Maestro
  **64.0 %**, Esclavo **55.3 %**.
- El FALLA es `decisiones_01_anclas` acusando a `D-22` (arrancar el cristal `Y1`), decidida y **sin
  construir**. Es correcto que acuse. **En esta sesion `Y1` no se toca.**
- El acta se tomo con el arreglo `1.49b3` integrado justo antes de comitearlo como `ca2de3d`: el fuente medido
  es el de `ca2de3d`. El tamano de un `.bin` no es la cifra de flash del acta; un binario se acredita por su sha256.
- **La compuerta dice que los modelos y arneses de PC no encuentran nada. No dice que funcione en la tarjeta.**

## 4. Como se trabaja la sesion

- **Dos telefonos, uno conectado a cada poste**, con la APK de §2 y rol administrador. Una `$ALARM` solo la
  ve el telefono que esta conectado a ESE poste en ese instante.
- **La cinta, toda la sesion y en las dos puntas** (pestana de depuracion: «Tramas en crudo» y «Diario de
  ordenes»), como la del Sisga en `evidencia/`. **Vale mas que la compuerta.** Tiene tope y tira lo mas
  antiguo: **se guarda al terminar CADA prueba**, en los dos telefonos, como `<prueba>_<poste>_<hora>`. Ve
  lo que el poste dice al telefono; **las tramas de radio entre postes no las graba nadie**.
- **Video** con las dos cabezas en el mismo plano, y el rele o LED de `J15` si lo hay: varias pruebas se
  deciden por un segundo o menos.
- **Modo Automatico** en el poste 1 (`SET_MODO:AUTO`). Tiempos configurados: verde ______ · despeje ______
- **Cortar la radio de un poste** = desconectar `485_A` y `485_B` en la radio de ese poste, con la radio
  energizada y con su antena. Es un corte TOTAL: ese poste ni oye ni habla.
- Cada resultado se anota **VISTO**, **NO VISTO** o **NO HECHO (motivo)**. Una casilla en blanco no se lee.
- **Si en cualquier momento hay verde en los dos postes a la vez: se para la sesion, se guardan las cintas
  y se avisa.** No se anota para seguir: se para.

## 5. Las pruebas, por dano

Las cifras de tiempo salen de las constantes de `ca2de3d` (silencio 25 s, margen 3,5 s, latido 3 s,
retardo de pluma 3 s, ambar de transicion 4 s). Valen para este hash y para ningun otro.

**Orden sugerido**, porque unas dejan preparada la siguiente: A1, A2, A3, B, C, E1, E3, E3b, E4, E1b, la
preparacion de D, E2, y el corte de D.

### A. `D-34` — ambar en un poste contra verde en el otro (`SPEC_2` §4, roadmap 1.39)

En un solo carril, un parpadeo en un poste con verde vivo en el otro es exposicion frontal.

**A1. Corte total con el verde en el poste 2**

- Prepara: ciclo normal, radio sana. Esperar a que el poste 2 este en verde.
- Manos: en el primer minuto de ese verde, cortar la radio del poste 2. Cronometro a cero en el corte.
- Debe verse:
  - El verde del poste 2 pasa a **ROJO FIJO** entre 18,5 y 21,5 s despues del corte.
  - Unos 3,5 s mas tarde, **los dos postes parpadean**, el poste 1 un instante despues que el 2.
  - **En ningun fotograma hay verde en el poste 2 con el poste 1 parpadeando.**
  - En cada telefono: `$ALARM` con `EVENTO:FALLO_RF`, `CAUSA:SILENCIO_25000ms`, `ACCION:CAMBIO_A_AMBAR`
    y la `HORA` al final. **No debe salir `REINTENTOS_AGOTADOS`.**
  - La pluma del poste 2 sigue arriba 3 s tras su rojo y baja. Con el parpadeo **las dos plumas suben**: es
    la politica decidida (`SPEC_8` §5), no un defecto.
  - Al reconectar: los dos a rojo, despeje, y el poste 1 abre su verde. Nunca dos verdes.
- Anotar: rojo del poste 2 a los ______ s · parpadeo poste 2 ______ s · poste 1 ______ s · alarmas ______
- Si NO se ve: si el verde del poste 2 pasa **directo a parpadeo sin rojo fijo**, `D-34` no esta dentro (hash
  equivocado) o no funciona. Si hay verde frente a parpadeo, aunque sea un fotograma, es el defecto que
  `D-34` cierra: se guarda el video.

**A2. Lo mismo con el verde en el poste 1** (`N-163`)

- Manos y debe verse: como A1, cortando la radio del poste 2 en el verde del poste 1. El verde del poste 1 pasa
  a rojo fijo entre 18,5 y 21,5 s y los dos parpadean unos 3,5 s despues. Si NO se ve: como A1.
- Anotar: rojo del poste 1 a los ______ s · parpadeos ______ s

**A3. Microcorte que vuelve despues de soltar el verde**

- Prepara: como A1.
- Manos: cortar la radio del poste 2 en su verde; **en cuanto su verde pase a rojo, reconectar en menos de
  2 s**. Con eso, en `ca2de3d`, un latido del poste 1 llega antes del silencio del poste 2. Entre 2 y 3,5 s
  puede no llegar, y entonces un parpadeo es lo esperado: se repite.
- Debe verse:
  - **Ningun poste parpadea.**
  - El poste 1 hace **un rojo y un despeje de mas**, y abre su verde (ambar fijo de 4 s y verde) como tarde
    **el despeje configurado mas unos 10 s** despues de reconectar.
  - El poste 2 se queda en rojo mientras tanto.
- Anotar: reconexion a los ______ s del rojo · el poste 1 abrio a los ______ s de reconectar · parpadeo si/no
- Si NO se ve: si hay parpadeo con la reconexion a menos de 2 s del rojo, es un ambar de mas en un
  microcorte. **Si el cruce se queda en todo-rojo minutos**, el poste 1 no oyo el aviso de verde soltado
  y no reanudo: es un modo de fallo que `D-34` crea.

**A4. Subida muerta con bajada viva — el caso exacto de `D-34`. NO SE PUEDE HACER con dos radios.**

- Nace cuando la orden de verde LLEGA al poste 2 y su acuse NO vuelve. El poste 2 acusa a los 200 ms (nadie
  corta un cable tan rapido) y en un RS485 de dos hilos no se corta un solo sentido. Lo limpio: `J12` de cada
  poste a un PC con dos USB-RS485 y un programa que copie bytes con un interruptor por sentido. **Ese programa
  no existe en el repositorio y no se improvisa.** Se anota **NO HECHO**; solo lo cubre el arnes de PC (`G12`-`G14`).

### B. `D-33` — la camara retiene la bajada de la pluma (`SPEC_8` §1 y §3)

Se hace en un poste, y si da tiempo en el otro. Prepara: rele o LED en `J15`; un cable para `p9`-`p10`.

**B1. Retardo sin camara**

- Manos: nada en `J16`. Mirar el paso de verde a rojo de ese poste.
- Debe verse: la pluma **sigue arriba unos 3 s despues del rojo** y luego baja.
- Anotar: segundos entre rojo y bajada ______
- Si NO se ve: si baja en el mismo instante del rojo, el retardo no esta dentro.

**B2. La camara no levanta la pluma**

- Manos: con el poste en rojo y la pluma ya abajo, cerrar `p9`-`p10` y mantener 10 s.
- Debe verse: **la pluma sigue ABAJO**. El campo `CAM:` pasa de `?` a `OK`.
- Anotar: pluma ______ · `CAM:` ______
- Si NO se ve: una camara que levanta una pluma en rojo es un defecto grave. Se para.

**B3. El veto**

- Manos: con el poste en verde, cerrar `p9`-`p10` y mantenerlo hasta despues del rojo.
- Debe verse:
  - Tras el rojo y sus 3 s, **la pluma sigue ARRIBA** mientras el puente siga puesto.
  - En el telefono de ese poste: `$EVENT` con `ORIGEN:CAMARA_PLUMA` y `DETALLE:VETO_ACTUADO_N:<n>`.
  - **El otro poste abre su verde igual** al acabar el despeje: el veto es local. Pluma arriba en rojo con
    verde enfrente es un estado DISENADO.
  - Al quitar el puente, la pluma baja en pocos segundos.
  - Repetir con `p11`-`p12`: lo mismo. Veta cualquiera de las dos.
- Anotar: `$EVENT` ______ · el otro poste abrio si/no ______ · bajada tras quitar ______ s · con p12 ______
- Si NO se ve: si la pluma baja con el puente puesto, el veto no existe en esta tarjeta.

**B4. Barrera retenida: el cartel de la app**

- Manos: como B3, pero mantener el puente mas de 2 minutos despues del rojo.
- Debe verse:
  - Pasados unos 90 s de veto: `$EVENT` `CAMARA_PLUMA` con `DETALLE:VETO_SOSTENIDO_S:<segundos>`, que se
    repite cada 90 s.
  - La app abre el cartel **«LA CÁMARA TIENE LA BARRERA RETENIDA»**: la barrera esta arriba y no va a bajar
    sola, mire debajo del brazo, y si no hay nada revise el apunte y la configuracion de la camara.
  - Al quitar el puente y llegar `PLUMA:ABAJO`: el cartel **se queda** y cambia a «LA BARRERA YA HA BAJADO».
- Anotar: segundos del primer aviso ______ · cartel si/no ______ · cambio al bajar si/no ______
- Si NO se ve: si llega el `$EVENT` y no hay cartel, el defecto es de la app. Si no llega, del firmware o del
  camino Bluetooth: se mira la cinta.

### C. Un puente en `J16` p5 o p8 no mueve nada (`SPEC_5` §3)

**Solo en esta mesa. NUNCA en el equipo del Sisga**: con el firmware viejo, un puente ahi compone
secuencias y una de ellas entra en Automatico sin ninguna guarda.

- Prepara: hash verificado (§1.2); `p1` retirado. Control: la prueba B2 en esa tarjeta, que demuestra que el
  cable y el gesto funcionan.
- Manos: cable desde `p4` (3,3 V) tocando `p5` tres veces (medio segundo cada una) = `A.A.A`. Luego desde
  `p7` a `p8` tres veces = `B.B.B`. Luego alternando `A.B.A.B`. Luego `p5` sostenido 10 s. Hacerlo con el
  cruce en Automatico y otra vez tras un reinicio, antes de dar ningun modo.
- Debe verse: **nada.** Ni luz, ni pluma, ni cambio de `MODO:` en el `$STATUS`, ni `$ACK`, ni `$EVENT`.
- Anotar: poste ______ · algo cambio si/no ______ · que ______
- Si NO se ve: cualquier cambio es un defecto grave. Se para la sesion y ese hash no sube a ningun sitio.

### D. El cristal que arranca y no cuenta (roadmap 1.22, marca `1.22` en `reloj.cpp`)

**No tiene alarma propia**: al medir que el contador no avanza, el firmware deja de creerse el reloj y la
sincronizacion guardada, y solo se ve por la consecuencia.

- Prepara: **solo discrimina en una tarjeta cuyo `Y2` arranca y no cuenta**. Se sabe antes:
  - Solo en el poste 1: «Reiniciar reloj y respaldo» (`REINICIAR_RELOJ`) desde la app (**borra el respaldo de
    la pila**: se hace antes de E2). La respuesta llega **unos segundos despues**, no en el acto: `SIGUE_PARADO_VEA_CONSULTA_RELOJ` = cristal
    muerto, esta prueba **NO APLICA**; `ARRANCA_Y_NO_CUENTA_VEA_CONSULTA_RELOJ` = **congelado**; `CRISTAL_OK_PONGA_LA_HORA`
    = cuenta (desde `ca2de3d` ya no sale con el cristal congelado). Si contesta `REPITA_EN_UNOS_SEGUNDOS`, se
    espera y se pide otra vez. El Esclavo no tiene esta orden.
  - Respuesta del poste 1: ______
  - Para distinguir congelado de vivo, en las dos tarjetas: leer por SWD el contador `RTC_CNTL`
    (`0x4000281C`) dos veces con 10 s de diferencia. Mismo valor = congelado; unos 10 mas = vivo. **SIN
    VERIFICAR** que se pueda leer asi. Si no se puede, la prueba D es **NO HECHO**: no se deduce.
  - Estado del cristal: poste 1 ______ · poste 2 ______
- Manos: al final de E2, con esa tarjeta en Degradado, desconectar su radio y quitarle la energia 10 s.
  Devolverla.
- Debe verse, con cristal **congelado**: **no vuelve sola al Degradado** (`MODO:` no es `DEGRADADO`), y un
  `SET_MODO:DEGRADADO` posterior (sin radio) se rechaza por hora o por sincronizacion; en el Maestro, en
  `ca2de3d`, `Falta: nunca hubo sincronizacion RF`. Si el Maestro llega a caer a ambar por esto, debe salir
  `$ALARM DEGRADADO` con causa `RELOJ_NO_CUENTA` (o `SYNC_SIN_FECHA`), nunca mudo. Con cristal **vivo** puede reanudar sola: no es defecto.
- Anotar: la hora que publica al arrancar ______ · reanudo si/no ______ · respuesta al pedir el modo ______
- Si NO se ve: una tarjeta con el cristal congelado que reanuda sola el Degradado se esta creyendo una
  sincronizacion de antiguedad desconocida. Se anota con la hora que publico al arrancar.

### E. Lo mas grave de lo que se arreglo y no ha visto una tarjeta

**E1. Ambar de emergencia del poste 2 con el poste 1 en verde** (`N-142`, `D-31`)

- Prepara: Automatico, radio sana, poste 1 en verde.
- Manos: desde el telefono del poste 2, **ÁMBAR EMERGENCIA**. Despues, pulsarlo otra vez.
- Debe verse:
  - El poste 2 parpadea al instante. `$ACK ... RESULT:OK`.
  - El poste 1 deja su verde y parpadea, y su `MODO:` pasa a `AMBAR`. Cuanto tarda: SIN VERIFICAR.
  - **En 20 s no sale ninguna `AVISO_RF` en el poste 2.** La segunda pulsacion: `RESULT:YA_EN_AMBAR_POSTE1_AVISADO`.
- Anotar: segundos hasta que el poste 1 dejo el verde ______ · `AVISO_RF` si/no ______
- Si NO se ve: si el poste 1 sigue en verde con el poste 2 parpadeando mas alla de unos segundos, es verde
  contra ambar en carril unico.

**E1b. El mismo ambar con el poste 2 sin radio** (hacer despues de E4)

- Manos: poste 1 en verde. Cortar la radio del poste 2 y, **antes de 10 s**, pulsar ÁMBAR EMERGENCIA.
- Debe verse: `$ACK ... RESULT:OK`; unos 14 s despues, `$ALARM` con `EVENTO:AVISO_RF`,
  `CAUSA:SIN_CONFIRMAR`, `ACCION:AVISE_POSTE_1`. El poste 1 sigue en verde hasta soltarlo por margen (18,5
  a 21,5 s del corte) y luego parpadea: es el residual conocido, y por eso la alarma manda avisar.
- Anotar: segundos hasta la `AVISO_RF` ______ · rojo del poste 1 a los ______ s del corte
- Si NO se ve: sin `AVISO_RF`, el tecnico se iria creyendo que el poste 1 lo sabe.

**E2. Modo Degradado en las dos puntas** (`D-18`, `D-20`, `D-26`; procedimiento en `SPEC_6` parte A)

- Prepara: los dos puentes ESP32 con el firmware de §2; hora puesta en los dos; radio sana unos minutos.
- Manos: `SET_MODO:DEGRADADO` (con PIN) al poste 1 y enseguida al poste 2. Mirar dos ciclos enteros.
  Cortar la radio del poste 2 y mirar otro ciclo. Reconectar y `SET_MODO:AUTO` al poste 1.
- Debe verse:
  - `RESULT:OK` en cada uno, o un `$ERR` con su motivo. **Un rechazo con motivo no es un defecto**: se anota.
  - Todo-rojo de entrada, y despues alternan. **Nunca verde en los dos.** Entre los dos verdes, un
    todo-rojo largo. Con la radio cortada no cambia nada.
  - Tras `SET_MODO:AUTO`, el poste 2 sale por todo-rojo cuando oye al poste 1, y el ciclo vuelve.
- Anotar: respuestas ______ · duracion medida de verde y todo-rojo ______ · salida ______
- Si NO se ve: verde en los dos = se para todo (§4). Si una punta entra y la otra no, se saca a la que entro.

**E3. Retirar el ambar desde el poste 2** (`N-152`, y de paso `N-147`)

- Prepara: el estado final de E1 (los dos parpadeando, el poste 1 en `MODO:AMBAR` por el aviso).
- Manos: `CANCELAR_AMBAR` (con PIN) desde el poste 2. Esperar 60 s sin tocar nada. Despues,
  `SET_MODO:AUTO` al poste 1.
- Debe verse: `RESULT:RETIRADO`; el poste 1 pasa a `MODO:MANUAL` y **los dos quedan en rojo fijo**. En los 60
  s **nadie da verde solo**. Tras `SET_MODO:AUTO`, despeje y ciclo.
- Anotar: segundos hasta el todo-rojo ______ · verde espontaneo si/no ______
- Si NO se ve: si el cruce sigue parpadeando, repetir `CANCELAR_AMBAR` (debe contestar
  `REENVIADO_AL_MAESTRO`) y anotar que el primer aviso se perdio.
- **E3b.** Poner el ambar desde el poste 1 (`SET_MODO:AMBAR`) y pulsar `CANCELAR_AMBAR` en el poste 2.
  Debe verse: el poste 2 contesta `REENVIADO_AL_MAESTRO` y **el cruce SIGUE en ambar**. Ese ambar lo pidio
  alguien en el poste 1 y el poste 2 no lo quita. Se sigue directamente con E4.

**E4. DAR PASO en modo Ambar** (`N-151`)

- Manos: con el poste 1 en `SET_MODO:AMBAR`, pulsar DAR PASO en el poste 1. Despues `SET_MODO:AUTO`, y
  cuando el ciclo este quieto, DAR PASO otra vez.
- Debe verse: primero `$ERR,CMD:CAMBIAR_TURNO,DESC:MODO_SIN_CICLO_SALGA_PRIMERO`; en Automatico el ciclo
  vuelve y DAR PASO contesta `RESULT:OK` o `EN_TRANSICION_REINTENTE`, nunca trabado.
- Anotar: respuestas ______ . Si la app no deja mandar la orden en Ambar, anotarlo: entonces se probo la app.
- Si NO se ve: si tras DAR PASO en Ambar el cruce no vuelve a ciclar en Automatico, el defecto sigue vivo.

### Lo que queda FUERA de este encargo, y por que

- `N-150` (ciclo tras aplicar tiempos): visto del lado del Maestro en el Sisga; se ejerce de paso en §4.
- `N-153` y `N-162` (`ESC:`, el puente): de pantalla, se ven en la cinta. Alarmas de hora de `D-26`
  (`J17_MUDO`): piden `J17` desenchufado mucho rato; si sobra tiempo.
- `CAM_CIEGA` (`D-24`): 24 h, no cabe en una sesion. `CAM_PEGADA`: 20 min de puente; si sobra, tras B4.
- `D-14` no existe en el firmware; `D-22` no esta construido. El «doble» del Modo Inteligente espera una
  firma sobre un manual, no una medida. Lo de la app sin firmware es de telefono, no de tarjeta.

## 6. Como se devuelve

- Una carpeta `Banco_ca2de3d_<fecha de la sesion>` con: las cintas y diarios de **los dos postes**, uno por
  prueba; los videos; la salida del programador de cada carga y lectura con el `sha256sum` leido, y **foto
  de esa pantalla junto a la tarjeta y su serie** (`SERIE:` del `$STATUS`); y **este documento relleno**.
- **Un paso no hecho se marca NO HECHO con su motivo. No se deja en blanco.**
- Que firmware queda dentro de cada tarjeta al acabar: poste 1 ______ · poste 2 ______
