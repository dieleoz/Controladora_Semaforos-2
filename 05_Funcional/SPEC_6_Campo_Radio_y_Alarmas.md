# SPEC 6 — CAMPO: el Degradado paso a paso, la radio como aparato, y las alarmas

**Para el que esta subido al poste con el telefono en la mano**, y para el que compra antenas. Escrita el
12/09/2026. **Son los huecos que las otras cinco spec dejaron entre ellas**, y no repite lo que ya dicen: el
**comportamiento** del Degradado es SPEC 2 §7 y su **luz** es SPEC 1 · el **dialogo de radio** y los DOS RELOJES
del silencio son SPEC 2, §§4 y 9 · la **hora** es SPEC 3 §7 · el **formato de las tramas** y lo que la app pinta es
SPEC 4 · el **cobre**, `J12` y las camaras son SPEC 5. Aqui va **lo que se TECLEA, lo que se CONFIGURA, lo que se
HACE cuando salta una alarma — y el unico binario que hoy se puede cargar en el equipo montado (PARTE D).**

**De donde sale cada linea.** Lo decidido manda en `DECISIONES.md` y el fuente en lo que el equipo HACE. **Los
manuales anteriores NO son fuente**: se abrieron para censar y **ninguna frase entro sin remedirla**. Se cita el
**SIMBOLO**, nunca la linea, y **no se copia ninguna cifra que este fichero no pueda recalcular**. 🔴 **Un paso
inventado es peor que un paso ausente, porque el instalador lo ejecuta:** donde el fuente no dice como se hace algo,
aqui pone **HUECO** y no un procedimiento verosimil.

# PARTE A — PROCEDIMIENTO DE CAMPO DEL MODO DEGRADADO

**Cuando se usa:** la radio entre postes esta MUERTA y el cruce tiene que seguir alternando; que hace el equipo
dentro, SPEC 2 §7. **No se entra "por si acaso"**: con el Maestro vivo su latido saca al Esclavo del modo.

## A.1 La entrada — UNA orden, y en LAS DOS PUNTAS por separado

`SET_MODO:DEGRADADO`, **con PIN, a cada poste**; el Esclavo tambien la acepta (`D-18`). La app antepone el PIN ella
sola. **No hay mando, no hay pulsadores y no hay pantalla**, asi que **sin telefono no hay forma de pedirlo**
(`D-1`, `D-2`, `D-16`, `D-17.bis`). **Que contesta cada punta** (la tabla completa es SPEC 4 §3):

| respuesta | que hacer |
|---|---|
| `RESULT:OK` | entro. **Empieza el todo-rojo obligatorio de entrada** — la luz tarda, no repita |
| `RESULT:YA_ACTIVO` *(Esclavo)* | **ya estaba dentro**: esta pulsacion no encendio nada |
| `$ERR ... DESC:<motivo>` | **rechazado. Lea el motivo — dice cual falta.** Las dos tablas de A.2 |

## A.2 🔴 LAS DOS PUNTAS NO RECHAZAN POR LO MISMO, Y HAY QUE LEER CADA UNA

Medido sobre las funciones de entrada y rechazo de cada punta. **MAESTRO — seis motivos**, su `DESC:` las dos juntas:

| lo que se lee en el telefono | que falta, y que se hace |
|---|---|
| `Falta: reloj sin poner en hora` | la hora no puede decidir una luz: **fiable, no solo puesta**. Ponerle la hora a ESE poste con el telefono, o esperar la siembra de su ESP32; si no llega, mire la alarma de la hora (parte C) |
| `Falta: nunca hubo sincronizacion RF` | esta punta **jamas** hablo con la otra en esta sesion. **Arregle la radio primero** (parte B): sin una sincronizacion no hay modo |
| `Falta: la ultima sync es muy vieja` | la hubo y caduco. El plazo de frescura son **horas, no dias**, y **NO es el mismo que el del Esclavo** |
| `Falta: el esclavo no tiene el ciclo` | el otro poste no ha acusado la configuracion. **No se puede saltar**: sin ella el Maestro daria verde por reloj mientras el Esclavo cae a ambar por orfandad |
| `Falta: sin medida de desfase valida` | no hay medida de desfase utilizable (`SFTY-23`) |
| `Desfase fuera de tolerancia (+-3s)` | la hay y se sale de la tolerancia. **No se corrige a ojo** |

**ESCLAVO — seis motivos, y TRES no existen en el Maestro:**

| lo que se lee en el telefono | que falta, y que se hace |
|---|---|
| `SIN HORA VALIDA` | igual que la primera del Maestro |
| `FALTA CONFIG CICLO` | el Maestro nunca le mando la duracion del ciclo |
| `CICLO EN CERO` | 🔸 **solo aqui.** Se la mando con un verde o un despeje en cero |
| `NUNCA SINCRONIZADO` | no hubo ninguna sincronizacion en esta sesion |
| `SYNC CADUCADA >48h` | 🔸 **solo aqui, y el plazo es OTRO**: es el limite duro, mucho mas largo que el plazo de frescura del Maestro. **Obliga a arreglar la radio** |
| `AMBAR EMERG.PUESTO` | 🔸 **solo aqui, y es la que muerde** — ver el bucle de abajo |

> 🔴 **EL BUCLE DEL AMBAR ENCLAVADO, que se paga en el poste.** El plan de aborto (A.4) manda `AMBAR_EMERGENCIA` al
> Esclavo, y **ese ambar queda enclavado**: esa punta **rechaza volver a entrar en Degradado** hasta que alguien
> mande `CANCELAR_AMBAR`. **No es una averia**: la maquina no revoca sola lo que puso una persona, porque debajo
> de esa luz puede haber alguien trabajando. **El Maestro no tiene esa guarda**, asi que aceptara mientras el
> Esclavo rechaza. ⚠️ **Y de que las dos listas sean distintas sale lo peor: UNA PUNTA PUEDE ENTRAR Y LA OTRA NO**,
> que es peor que ninguna — **si la segunda rechaza, se saca a la primera** (A.4) antes de arreglar el motivo.

## A.3 Verificacion visual de LAS DOS puntas — obligatoria, y tambien al salir

Con las dos dentro, **quedese a ver un ciclo completo** y compruebe **con los ojos, no en la pantalla**:

- [ ] Maestro en verde ➜ Esclavo en rojo.
- [ ] Entre los dos verdes hay un **todo-rojo largo** con las dos puntas en rojo.
- [ ] Esclavo en verde ➜ Maestro en rojo.
- [ ] **En ningun momento hay verde en las dos a la vez.**

**Por que no vale la pantalla:** cada punta calcula SU fase, y si las dos calculan mal —relojes separados, una unidad
reiniciada— **las dos diran que todo va bien** mientras las luces cuentan otra cosa. El verde y el despeje de este
modo son **propios del modo: no heredan lo de Automatico.**

## A.4 Salida, y el plan de aborto

| | orden | detalle |
|---|---|---|
| **volver a Automatico** | `SET_MODO:AUTO` **solo al Maestro** | **el Esclavo no tiene salida a Automatico por app**: lo devuelve el propio Maestro **cuando vuelve la radio**. ⚠️ **Por app no hay todo-rojo de despedida** |
| **irse a ambar (Maestro)** | `SET_MODO:AMBAR` | |
| **irse a ambar (Esclavo)** | `AMBAR_EMERGENCIA` | **entra sin PIN a proposito**: quien ve el incidente tiene que poder pararlo |
| **retirar ese ambar** | `CANCELAR_AMBAR` | **pide PIN al reves que ponerlo**: quitarlo devuelve el cruce a dar verdes |

> 🔴 **`SALIENDO_TODO_ROJO` NO ES `OK`: el ambar todavia NO esta puesto.** Con el Degradado gobernando, el Esclavo
> sale **por todo-rojo** y eso tarda —saltar de verde a intermitente le dice al que viene lanzado que negocie el
> paso creyendo que aun tiene prioridad—. **No se vaya del cruce hasta verlo con los ojos.** Si contesta `$ERR ...
> SALIDA_A_ROJO_EN_CURSO_REPITA`, **repita la orden**. ⚠️ **Y `FORZAR_ROJO` NO sirve en el Esclavo:** esa punta lo
> **rechaza** nombrando el literal bueno (`RENOMBRADO_USE_AMBAR_EMERGENCIA`), no para nada, y el ciclo por reloj
> volvera a dar verde en la fase siguiente. **No es un plan de aborto.**

## A.5 Lo que pasa SIN que nadie lo pida — y como se ve

Tres cosas que el equipo hace solo, y solo se ven en **`MODO:`** del `$STATUS` (`DEGRADADO`·`RENDIDO`·`SUBORDINADO`):

1. **Vuelve la radio ➜ sale del modo.** Solo con tramas de gobierno; las de servicio no sacan.
2. **Se agota el limite duro ➜ se rinde sola a ambar**, y **el firmware no deja reentrar**: contesta el rechazo de
   sync caducada. **Hay que arreglar la radio, no reactivar el modo.** El Poste 1, ademas, lo dice en el instante
   con `$ALARM DEGRADADO` y su causa (PARTE C); el Poste 2, no.
3. **Corte de energia ➜ PUEDE REANUDAR SOLA**, en todo-rojo, si al volver siguen vigentes hora, ciclo en la pila y
   sincronizacion fechable (`D-29`). **Nadie se lo avisa al operario.** 🔴 **Tras cualquier corte, verifique A.3 en las
   dos puntas aunque el equipo no se lo pida**: si una reanudo y la otra no, puede dar verdes sin que nadie mire.

## A.6 🔴 LA COMPROBACION DE LA HORA QUE SIGUE HACIENDO FALTA, Y POR QUE

**Que una hora que deja de ser fiable en marcha mande la punta a ambar YA ESTA CONSTRUIDO** (`D-21` (1)), pero la pieza
que llevaria al STM32 el aviso de que el oscilador del reloj de calendario se paro **sigue sin construir** (la medida
es SPEC 3, H-2), asi que **el unico detector de una hora que MIENTE es el tecnico** y estos dos pasos siguen vivos:
**antes de autorizar un Degradado, lea la hora de las DOS puntas** con la consulta de reloj —`LEER_RTC`, que **lee y no
escribe** (`D-17`), y la contesta el ESP32— **y contrastela con un reloj de fuera**; y **si una punta devuelve una fecha
que no es la de hoy, no se entra en Degradado** — se pone en hora primero, y la pila de ese reloj es consumible.

# PARTE B — LA RADIO COMO APARATO: configuracion y antenas

**Que trata:** el modulo E90-DTU y su antena. **El protocolo que viaja por dentro es SPEC 2**, y el conector `J12` con
su cobre es SPEC 5. *(SPEC 2 §8 remitia «los parametros del modulo» a SPEC 5, que no los trae; corregido el 13/09.)*

## B.1 Lo que NO se negocia

1. **`2.4 kbps` de Air Data Rate.** La tasa vieja saturaba el canal y el cruce caia en fallo de comunicacion **justo
   al pasar el verde al otro poste**. Si el enlace sigue cayendo, **no se baja la velocidad otra vez**: se reporta.
2. **`M0` y `M1` los dos en `OFF` en operacion.** Los `ON` son **solo** para configurar, y ahi la buena es **`M0=ON` con
   `M1=OFF`**: la de `ON`/`ON` de un manual viejo deja la radio en un modo donde **la transmision puede quedar
   deshabilitada mientras la recepcion sigue activa** — radios que oyen y no contestan, un dia de campo.
3. **TODAS las radios del enlace, IGUALES.** Una punta con otra tasa o en otro canal **no enlaza**, y el sintoma no dice
   «mal configurada»: dice **«el equipo se callo»** y manda a mirar el cable.

## B.2 Los parametros, y cual de ellos puede este proyecto verificar

| parametro | valor | quien lo puede contradecir |
|---|---|---|
| **UART** | `9600`, `8N1` | 🟢 **el FIRMWARE**: abre el bus de radio a esa velocidad en **las dos puntas**, y el Repetidor abre sus dos radios igual, `8N1` explicito. **Si la radio no esta a esa velocidad, no hay enlace posible** |
| **Air Data Rate** · **Potencia** · **FEC** · **Transmission Mode** · **Canal** | `2.4 kbps` · la maxima del modulo · activado · `Transparent` *(fixed-point desactivado)* · **el MISMO en las dos puntas** | 🔴 **nadie**: viven en la NVM del modulo y ningun instrumento de aqui los lee |

> 🔴 **NINGUN INSTRUMENTO DE ESTE REPOSITORIO LEE LA CONFIGURACION DE UNA RADIO.** La compuerta no la ve, el firmware
> no la puede preguntar y el `$STATUS` no la publica. **La unica evidencia de que una radio quedo bien configurada es
> que alguien lo escribio en un papel** — por eso B.3 acaba en casillas y por eso la calidad de enlace de B.6 es la
> unica medida de verdad que queda. **El numero de canal no se publica aqui**: vive solo en la NVM y este fichero no
> puede recalcularlo. **Lo que se exige es la IGUALDAD**, y el numero se lee de la radio antes de tocar nada.

## B.3 El procedimiento, y el orden importa

1. **Radio sin energia.** Poner **`M0=ON`, `M1=OFF`**.
2. PC con convertidor **USB a RS485** (`A` con `485_A`, `B` con `485_B`) y 12 V a la bornera.
3. `04_Manuales/RF_Setting4.6.exe` ➜ COM ➜ **`9600`** ➜ `Open Port` ➜ **`Read Option`** *(leer ANTES de escribir: ahi
   se ve el canal que la instalacion tiene de verdad)*.
4. Poner los valores de B.2 y **`Write Option`**.
5. **Quitar energia. `M0` y `M1` a `OFF` los dos.** Reconectar `485_A`/`485_B` al semaforo.
6. **Repetir en la otra punta** y comprobar que las dos quedaron **iguales**.

> 🛑 **NUNCA se energiza una radio sin la antena conectada.** Transmitir sin carga —o contra una antena desadaptada—
> **daña el amplificador de salida**, y ese es el modo de averia que ya se pago una vez.

**Casillas antes de cerrar los gabinetes:** ⬜ misma Air Data Rate en las dos ⬜ mismo canal ⬜ `M0`/`M1` en `OFF` en
las dos ⬜ antena conectada en las dos ⬜ apuntado en el parte **que radio quedo con que canal**.

## B.4 Topologia: hoy son DOS radios en enlace directo

El repetidor **no esta montado** y es reinstalable. **El firmware no cambia** al pasar de dos a cuatro: las puntas son
agnosticas a la topologia y **lo unico que cambia es el CANAL**.

> 🔴 **LA TRAMPA ESTA EN RETIRARLO, NO EN PONERLO.** Con repetidor, las dos puntas quedan en canales DISTINTOS a
> proposito —si no, el repetidor se oye a si mismo—. **Quitar el repetidor sin devolver la radio del Esclavo al
> canal del Maestro deja las dos puntas en frecuencias distintas**: no se comunican y **las dos caen a ambar
> intermitente** al vencer el silencio de radio. Parece radio rota.

## B.5 Antenas

**La especificacion completa de fabricacion —frecuencia, ROE, plano de tierra, BOM y cable— vive en
`05_Funcional/7_Especificacion_Antenas.md`** y no se copia aqui. Lo unico que se repite, porque puede herir:

- **Una antena se pide por su FRECUENCIA REAL, no por su etiqueta comercial**, y fuera de banda **devuelve la potencia
  al amplificador**. ⚠️ **Las genericas de «LoRa» del 31/07 eran de otra banda y no se reinstalan en ningun radio.**
- **El entregable que decide la compra es el reporte de ROE** en las frecuencias de operacion, todo a **50 Ω**: sin
  el **no hay forma de distinguir una antena correcta de una que solo lo afirma**.
- 🔴 **ANTES DE EMITIR UN PEDIDO: el modelo real de las radios sigue SIN CONFIRMAR** y la antena esta especificada
  sobre ese supuesto de banda. **Se cierra leyendo la etiqueta de la caja metalica** (HUECO 4).

## B.6 Como se mide el alcance HOY

**La pantalla `PRUEBA ALCANCE` ya no existe** —se fue con el LCD (`D-17.bis`)— y el Modo Alcance publicaba **por ese
unico camino**: entrar en el desde la app deja el cruce parado en rojo y **no ensena nada a nadie** (SPEC 4 §7.5).
**En su lugar, sin cambiar de modo y sin parar el cruce:** se leen **`RF:`** y **`RTT:`** del `$STATUS` del Maestro,
**en el MISMO punto** con la antena vieja y con la nueva, anotando distancia y hora — una prueba de alcance sin punto
fijo no compara nada.

⚠️ **Y lo que aquel modo daba en crudo hoy sale por OTRO sitio, no por el `$STATUS` — remedido el 14/09.** Los acumulados
de **bytes recibidos, tramas validas y tramas descartadas** los publica el diario, en las **dos** puntas, como
`$EVENT ENLACE_RF RX:<n> OK:<n> RUIDO:<n>`, al cambiar el estado del enlace y ademas cada medio minuto (`D-32` (1)).
**Para una sesion de antenas se leen de ahi, no del `$STATUS`. Lo unico que sigue sin sustituto son los LATIDOS
PERDIDOS**, que no los cuenta nadie: si una sesion los necesita, **pregunta para el responsable.**

# PARTE C — LAS ALARMAS: censo completo, y que hace el tecnico con cada una

**`$ALARM` es la Caja Negra**: sale **en el instante** en que pasa algo y **solo lo ve quien esta conectado entonces**
(quien llegue despues lo lee en el `$STATUS` o en el Diario; formato en SPEC 4 §5). **Las emiten las DOS puntas y nadie
mas**: medido, el puente ESP32 **no origina ninguna** —solo retransmite— y el Repetidor no emite tramas. **El campo
`ACCION` dice lo que el equipo HIZO, no lo que deberia hacerse.**

| `EVENTO` | `CAUSA` | `ACCION` | quien | que la dispara | **que hace el tecnico** |
|---|---|---|---|---|---|
| `FALLO_RF` | `SILENCIO_<n>ms` | `CAMBIO_A_AMBAR` | **las dos** | no llega **nada** del otro poste en el plazo de silencio | **el enlace esta CORTADO.** Parte B por orden: energia ➜ `M0`/`M1` ➜ misma tasa y canal ➜ antena y su conector. **Y ponerle la hora a este poste**: sin radio, esa hora ya entra (`D-26` (5)) |
| `FALLO_RF` | `REINTENTOS_AGOTADOS` | `CAMBIO_A_ROJO` | **solo Maestro** | se agotaron los reintentos de un cambio de luz **con el enlace vivo**: la luz va a **rojo** por la vuelta del enlace, no a ambar (`D-34`, 15/09). ~~sale tambien con la radio MUERTA y antes que la de silencio~~ → refutado el 15/09 en el arnes de dos puntas: con la radio muerta el silencio vence antes y cambia el estado | **lo normal es que el enlace EXISTA y se degrade** —lluvia, distancia, interferencia—: eso es **cobertura y antena** (B.5), no configuracion. 🔴 **Si despues sale tambien `SILENCIO_<n>ms`, el enlace estaba CORTADO.** ~~entre las dos alarmas hubo un tramo con la otra punta pudiendo seguir en verde~~ → cerrado en el fuente por `D-34` (SPEC 2 §4), **sin banco**. **No se vaya sin mirar la otra punta** |
| `AVISO_RF` | `SIN_CONFIRMAR` | `AVISE_POSTE_1` | **solo Esclavo** | se puso el ambar de emergencia aqui y **el Poste 1 no acuso** el aviso **ni tras agotar los reintentos**. El acuse se construyo el 13/09 y su numero de reintentos lo fijo el responsable; la espera total es **(1 + reintentos) x el plazo de un viaje de radio** (`D-31`, `D-32` (3)). ⚠️ **NO ES INMEDIATA: espere a que salga antes de irse** | **haga lo que dice la accion**: cierre el paso aqui o **avise al Poste 1 antes de irse**. 🔴 **Dice «no he podido confirmarlo», NUNCA «el otro poste no se entero»**: una trama perdida por lluvia se ve igual que un transmisor muerto |
| `HORA_ESP32` | `J17_MUDO` · `SIN_HORA_DEL_ESP32` · `RECHAZADA_FORMATO` | `SIGUE_SU_HORA` | las dos | la hora del ESP32 **no llega, o llega y no sirve** | **es de la MISMA placa**: el ESP32, su reloj de calendario y el cable `J17`. Las tres causas mandan a sitios distintos: **SPEC 3 §7** |
| `HORA_ESP32` | `CADUCADA` | `CAMBIO_A_AMBAR` | las dos | la hora caduco **con el Degradado en marcha** (`D-21` (1)) | esa punta ya esta en ambar. Poner la hora antes de reintentar el modo (A.2, A.6) |
| `DEGRADADO` | `LIMITE_48H` | `CAMBIO_A_AMBAR` | **solo Maestro** | vencio el limite duro sin sincronizar con la otra punta, y el equipo SI sabia fecharlo —en RAM o en la pila— (1.49 (b)) | **es la radio**: sin ella nadie renueva el limite. Parte B por orden. **No reactive el Degradado**: rechaza por sincronizacion (A.5), y poner la hora con el telefono **no renueva el tope** (`SPEC_7` §5) |
| `DEGRADADO` | `SYNC_SIN_FECHA` | `CAMBIO_A_AMBAR` | **solo Maestro** | el contador del reloj cuenta, pero la marca de la ultima sincronizacion **no se puede fechar** —reloj movido hacia atras, dominio de respaldo borrado— y no hay medida en RAM | **no se sabe cuanto hace**, y el equipo lo trata como vencido. **Lo que lo levanta es una sincronizacion nueva entre las dos puntas, y esa solo viaja por radio**: la hora del telefono no la renueva (`SPEC_7` §5). Compruebe el enlace (Parte B) y que la puerta de A.2 acepte antes de reintentar el modo |
| `DEGRADADO` | `RELOJ_NO_CUENTA` | `CAMBIO_A_AMBAR` | **solo Maestro** | vencio el limite **y el contador del reloj de la tarjeta no cuenta** en ese instante. El rotulo dice «No es la radio» | 🔴 **NO SE FIE DEL ROTULO: mire LAS DOS COSAS.** Con el contador parado la marca de la pila no se puede fechar, y eso es del reloj (SPEC 3, H-1). Los bits los publica `REINICIAR_RELOJ` cuando rechaza (SPEC 4 §3.1), **que ademas BORRA la hora, el ciclo acordado y la autorizacion del Degradado**: no se pulsa para mirar; **pero en una tarjeta cuyo cristal no arranca esta causa sale SIEMPRE, tambien cuando el plazo lo cumplio de verdad una radio caida** —el hueco de SPEC 3 H-1—. Revise la radio (Parte B) igual que con `LIMITE_48H` |
| `CAM_PEGADA` | `CAM_C_CONTACTO_FIJO` · `CAM_D_CONTACTO_FIJO` | **`NINGUNA`** | las dos | el contacto de esa camara lleva cerrado mas del plazo de presencia legitima | **el borne lo dice el nombre**: son los dos de camara de `J16` (`D-25`, SPEC 5 §2.1). Revisar cableado y la configuracion de esa camara (`D-13`). **La causa dice `CONTACTO_FIJO` y NO «averia» a proposito**: no se puede distinguir de una presencia legitima larga |
| `CAM_CIEGA` | `CAM_C_SIN_FLANCO` · `CAM_D_SIN_FLANCO` | **`NINGUNA`** | las dos | esa camara **dio flancos antes** y lleva demasiado paso abierto sin dar ninguno | lo mismo. ⚠️ **Solo puede salir de una camara que YA vio algo alguna vez**: una muerta desde la instalacion, o un borne vacio, **no la dispara nunca** (SPEC 5 §3.2) |

> 🔴 **`ACCION:NINGUNA` NO ES RELLENO: significa «medida de seguridad vial ejecutada: ninguna».** Las dos alarmas de
> camara **no mueven nada por si mismas**: no bajan la pluma, no tocan una luz y no paran el ciclo.
> ⚠️ **PERO LA CAMARA QUE LAS DISPARA SI RETIENE LA BARRERA** (`SPEC_8` §1). ~~Ninguna camara protege la
> pluma~~ dejo de ser cierto el 14/09: **una camara pegada puede estar dejando el brazo ARRIBA ahora mismo**,
> y eso no se ve en esta alarma sino en el aviso de barrera retenida. **Atiendala mirando tambien si la
> pluma bajo.**

> ⚠️ **LAS TRES `DEGRADADO` SALEN SOLO DEL POSTE 1, Y UNA SOLA VEZ** —al contrario que las de hora, no se
> repiten—: quien se conecte despues ve el ambar en `MODO:` y no la causa. El Poste 2 se rinde por su limite **sin `$ALARM`**: lo unico que
> deja es el parte periodico de sincronizacion diciendo que vencio (HUECO 2). **Y la app todavia no las traduce**:
> llegan en crudo (SPEC 4 §7, hueco 11). Con cualquiera de las tres, **mire tambien el Poste 2**.

**Cuando se apaga una alarma.** Las de camara **se cierran solas por su prueba contraria** y lo dicen con un
`$EVENT` de camara recuperada: una que no se cierra nunca deja al operario sin saber si aquello se arreglo.
Las de hora **se repiten mientras duren**, porque quien las tiene que ver es el que se conecte **despues**.

# PARTE D — `T-2`: EL PARCHE DE PACIENCIA PARA LA VERSION QUE ESTA EN LA CALLE

> 🔴 **ESTO NO DESCRIBE `main`, DESCRIBE UN PARCHE SOBRE LA VERSION DE CAMPO, Y CONFUNDIRLAS SERIA PEOR QUE NO
> ESCRIBIRLO.** Todo lo anterior y SPEC 2 cuentan el firmware de `main`. `T-2` es **otro binario**: la instalacion
> certificada —cual es y con que hash lo dice `ESTADO.md`, no este fichero— con **un numero cambiado**, para que el
> cruce ya montado deje de irse a ambar con la lluvia antes de que se le pueda cargar el firmware entero.

## D.1 El problema de campo, y por que encaja

El reporte del 13/09 —*«si llueve, el cruce se iba a ambar antes de 15 segundos, a los 7»*— sale de tres cosas medidas
sobre el propio commit de esa version: (1) su techo de orfandad son **12 s**, y ⚠️ **no es una constante con nombre: es
el literal `12000` escrito a pelo**, dos veces, una por punta (`Esclavo/src/main.cpp`, junto a `tUltimoComando`;
`Maestro/src/coordinador.cpp`, junto a `tUltimaRxEsclavo`, nombre de esa version: en `main` el ancla es otra, SPEC 2
§4) — **se dice asi porque es parte del hallazgo**: no hay simbolo
que citar, y es la forma que `N-69` castiga; (2) el latido **se suprime mientras se espera un ACK** (`SFTY-13`,
SPEC 2 §2.3), asi que el contador de silencio **llega envejecido en uno o dos latidos** y **perder cuatro latidos
seguidos manda las dos puntas a ambar** — de ahi el «a los 7»; (3) ⚠️ **la SEGUNDA via al ambar, agotar los reintentos
del cambio, en esa version NO SE ALCANZA**: el silencio vence antes y **los ultimos reintentos nunca se han ejecutado en
la calle** — lo que un pack vigila en `main`, vivo aun en el poste.

## D.2 Que es `T-2`, y por que **17** y no el valor de `main`

`T-2` cambia **ese literal por `17000`**: dos lineas, una por punta, y **nada mas** —ni constante con nombre, ni ninguna
otra pieza de `main`—. 🔴 **Por que no se sube directamente al valor de `main`:** ese valor separa mucho los dos plazos
y abre una ventana con **una punta en VERDE y la otra en AMBAR INTERMITENTE** —el Maestro cae por reintentos agotados y
el Esclavo no suelta su verde hasta que vence SU silencio—, que **en un paso alternado de un solo carril es exposicion
frontal**. Con `17000` **las dos vias casi coinciden y la ventana vuelve a ~0**, y la paciencia sube igual: el plazo
crudo crece la diferencia entre los dos literales, **un latido entero mas de racha perdida**. El concepto —**los dos
relojes**— es **SPEC 2 §9**; `main` cierra esa ventana por otra via.

## D.3 ⚠️ EL LIMITE DECLARADO, QUE NO SE PUEDE OMITIR

**El commit de la version de campo NO SE PUEDE RECONSTRUIR DESDE GIT, y esta medido:** el `main.cpp` de su Maestro
incluye una cabecera que **no esta en ese commit** — un `.gitignore` en **UTF-16** ocultaba nueve ficheros y se arreglo
**tres horas mas tarde**. El binario de `T-2` es, literalmente, **«la version de campo + dos ficheros recuperados de
3 h despues + un numero»**, y asi hay que decirselo a quien lo cargue. **Y lo que sigue sin ser:** sus binarios llevan
**`_SIN_BANCO`**, que **no es decoracion** —nadie los ha pasado por banco ni por una tarjeta, y **su ausencia se leeria
como permiso**—. ⚠️ **En la carpeta de entrega conviven DOS parejas y un `.patch` que documenta el intento anterior,
no el bueno**: vale el que lleva el plazo en el nombre, y **antes de cargar se comprueba el `sha256sum`, no el tamano**.

# HUECOS MEDIDOS

**1. ⚠️ EL MODO DEGRADADO CASI NO DEJA RASTRO EN EL DIARIO — y el caso de uso es justo el que llega despues.**
Recensado el 14/09, esta vez sobre los cuatro ficheros que publican y no solo sobre el del modo: el diario trae **el
salto de hora que pasa por rojo** y **el parte periodico de sincronizacion** (HUECO 2). Sigue **sin haber traza de que
una punta ENTRO, SALIO ni REANUDO SOLA tras un corte** (A.5), y `MODO:` dice el AHORA, no el CUANDO; lo unico que se
infiere es la rendicion, porque el parte pasa a decir que vencio. **El molde existe; no hay fila que pida las otras.**
*(La salida del Poste 1 por su limite ya sale como `$ALARM DEGRADADO` —PARTE C—, pero una alarma es del instante y
no del diario: no cierra este hueco.)*

**2. ⚠️ EL AVISO PREVIO AL LIMITE DURO YA TIENE DONDE LEERSE — LO QUE FALTA AHORA ES OTRA COSA. Esta linea decia que no
salia por ningun sitio y ERA FALSA; remedido el 14/09 y corregido en vez de matizado.** Las dos puntas publican
`$EVENT DEGRADADO SYNC:<n>h AVISO:SI/NO VENCIDA:SI/NO` desde su modulo de Bluetooth, **por flanco y repetido cada medio
minuto mientras el aviso sigue armado**, y el propio fuente deja escrito que se reparo el 13/09, el mismo dia que se
rompio al retirar el LCD (`D-32` (1)). **Lo que queda de hueco, medido hoy:** (a) **los dos umbrales siguen sin ser el
mismo numero en las dos puntas** —el Maestro avisa en las ultimas 4 h de las 48, el Esclavo antes—; y (b) 🔴 **la CUENTA
ATRAS del Degradado no se publica en NINGUNA de las dos puntas**: las dos funciones que la calculan se quedaron **sin un
solo llamador** cuando se fue el LCD, y un comentario del firmware que dice *«en el Esclavo si»* **tambien es falso** —
verificado por `grep` en las dos puntas.

**3. 🔴 NADA DE ESTE REPOSITORIO VERIFICA LA CONFIGURACION DE UNA RADIO** (B.2): ni instrumento, ni lectura de
vuelta, ni acta — solo la casilla que marque una persona. · **4. 🔴 EL MODELO REAL DE LAS RADIOS SIGUE `SIN
VERIFICAR`, y la antena esta especificada sobre ese supuesto** (B.5), abierto desde el 01/08. · **5. 🔴 SIGUE SIN
CONSTRUIR que el aviso de oscilador parado llegue al STM32: el unico detector de una hora que miente es el tecnico**
(A.6). · **7. ⚠️ El plazo real del todo-rojo de salida del Esclavo no esta medido**: A.4 dice «tarda» y no cuanto.

**6. ⚠️ Que hacer EN LA CAMARA cuando salta `CAM_PEGADA` o `CAM_CIEGA` no esta escrito en ningun sitio.** Aqui se manda
a revisar el borne y la configuracion **porque eso es lo que fijan las dos decisiones de camara**; **el diagnostico de la
camara como aparato no lo cubre nadie**, y no se inventa. Ademas, **ninguna camara se ha conectado nunca a este equipo**
(SPEC 5 §6.1): **estas dos alarmas jamas se han visto salir**.

**8. ⚠️ Nada de este capitulo ha visto una tarjeta.** Ni el Degradado por app, ni las alarmas, ni una radio
reconfigurada con este firmware dentro — **ni `T-2`**. Que firmware corre en cada equipo lo dice `ESTADO.md`.

**9. 🔴 `T-2` NO TIENE FILA EN `DECISIONES.md`** (PARTE D). Recensado el 14/09: la tabla llega ya hasta `D-33` y **ninguna
de sus filas habla de subir el techo de orfandad de la version de campo** —buscado por el propio nombre del parche, cero
apariciones—. **Describe un binario construido, no una decision tomada**, y sin fila **no hay nada escrito que autorice
cargarlo** ni que fije cual plazo es el bueno. Se nombra como hueco; esta spec no abre filas.

*Las decisiones vigentes que esta spec recoge van citadas donde aplican; nombradas como hueco, la pieza del aviso de
oscilador parado y `T-2` sin fila. Remedido el 12/09/2026 sobre
`{Maestro,Esclavo}/src/{modo_degradado,bluetooth,botones,coordinador,main,protocolo,reloj}.cpp`, el 13/09 sobre
`coordinador.cpp`, `Esclavo/src/main.cpp`, `Esclavo/include/reloj.h`, `costura_09_presupuesto_radio.py` y el commit de
campo, y el 14/09 sobre los dos `bluetooth.cpp`, los dos `modo_degradado.cpp` y `DECISIONES.md`. Censados y NO copiados:
`8_Procedimiento_Modo_Degradado.md`, `4_Manual_Configuracion_Radios.md`, `7_Especificacion_Antenas.md` y
`MANUAL_EXACTO_RADIOS_E90_DTU.md`.*
