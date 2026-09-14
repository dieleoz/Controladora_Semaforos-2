# SPEC 6 — CAMPO: el Degradado paso a paso, la radio como aparato, y las alarmas

**Para el que esta subido al poste con el telefono en la mano**, y para el que compra antenas. Escrita el
12/09/2026. **Son los huecos que las otras cinco spec dejaron entre ellas**, y no repite lo que ya dicen: el
**comportamiento** del Degradado es SPEC 2 §7 y su **luz** es SPEC 1 · el **dialogo de radio** y los DOS RELOJES
del silencio son SPEC 2, §§4 y 9 · la **hora** es SPEC 3 §7 · el **formato de las tramas** y lo que la app pinta es
SPEC 4 · el **cobre**, `J12` y las camaras son SPEC 5. Aqui va **lo que se TECLEA, lo que se CONFIGURA, lo que se
HACE cuando salta una alarma — y el unico binario que hoy se puede cargar en el equipo montado (PARTE D).**

**De donde sale cada linea.** `DECISIONES.md` manda en lo decidido y el fuente en lo que el equipo HACE. **Los
manuales anteriores NO son fuente**: se abrieron para censar y **ninguna frase entro sin remedirla**. Se cita el
**SIMBOLO**, nunca la linea (`CLAUDE.md` §7.3), y **no se copia ninguna cifra que este fichero no pueda
recalcular** (§14). 🔴 **Un paso inventado es peor que un paso ausente, porque el instalador lo ejecuta:** donde el
fuente no dice como se hace algo, aqui pone **HUECO** y no un procedimiento verosimil.

# PARTE A — PROCEDIMIENTO DE CAMPO DEL MODO DEGRADADO

**Cuando se usa:** la radio entre postes esta MUERTA y el cruce tiene que seguir alternando; que hace el equipo
dentro, SPEC 2 §7. **No se entra "por si acaso"**: con el Maestro vivo su latido saca al Esclavo del modo.

## A.1 La entrada — UNA orden, y en LAS DOS PUNTAS por separado

`SET_MODO:DEGRADADO`, **con PIN, a cada poste** (`D-18` se la dio tambien al Esclavo). La app antepone el PIN ella
sola. **No hay mando, no hay pulsadores y no hay pantalla** (`D-1`, `D-2`, `D-17.bis`): **sin telefono no hay forma
de pedirlo** (`D-16`). **Que contesta cada punta** (la tabla completa es SPEC 4 §3):

| respuesta | que hacer |
|---|---|
| `RESULT:OK` | entro. **Empieza el todo-rojo obligatorio de entrada** — la luz tarda, no repita |
| `RESULT:YA_ACTIVO` *(Esclavo)* | **ya estaba dentro**: esta pulsacion no encendio nada |
| `$ERR ... DESC:<motivo>` | **rechazado. Lea el motivo — dice cual falta.** Las dos tablas de A.2 |

## A.2 🔴 LAS DOS PUNTAS NO RECHAZAN POR LO MISMO, Y HAY QUE LEER CADA UNA

Medido sobre `modo_degradado_evaluarEntrada()` + `modo_degradado_motivoL1()`/`motivoL2()` (Maestro) y sobre
`degradado_entrar()` + `degradado_textoRechazo()` (Esclavo). **MAESTRO — seis motivos**, su `DESC:` las dos juntas:

| lo que se lee en el telefono | que falta, y que se hace |
|---|---|
| `Falta: reloj sin poner en hora` | `reloj_horaFiable()` es falso: **fiable, no solo puesta**. Ponerle la hora a ESE poste con el telefono, o esperar la siembra de su ESP32; si no llega, mire la alarma `HORA_ESP32` (parte C) |
| `Falta: nunca hubo sincronizacion RF` | esta punta **jamas** hablo con la otra en esta sesion. **Arregle la radio primero** (parte B): sin una sincronizacion no hay modo |
| `Falta: la ultima sync es muy vieja` | la hubo y caduco. El plazo es `SYNC_FRESCA_MS` — **horas, no dias**, y **NO es el mismo que el del Esclavo** |
| `Falta: el esclavo no tiene el ciclo` | el otro poste no ha acusado la configuracion. **No se puede saltar**: sin ella el Maestro daria verde por reloj mientras el Esclavo cae a ambar por orfandad |
| `Falta: sin medida de desfase valida` | no hay medida de desfase utilizable (SFTY-23) |
| `Desfase fuera de tolerancia (+-3s)` | la hay y se sale de `TOLERANCIA_DESFASE_S`. **No se corrige a ojo** |

**ESCLAVO — seis motivos, y TRES no existen en el Maestro:**

| lo que se lee en el telefono | que falta, y que se hace |
|---|---|
| `SIN HORA VALIDA` | igual que la primera del Maestro |
| `FALTA CONFIG CICLO` | el Maestro nunca le mando la duracion del ciclo |
| `CICLO EN CERO` | 🔸 **solo aqui.** Se la mando con un verde o un despeje en cero |
| `NUNCA SINCRONIZADO` | no hubo ninguna sincronizacion en esta sesion |
| `SYNC CADUCADA >48h` | 🔸 **solo aqui, y el plazo es OTRO**: `LIMITE_SIN_SYNC_MS`, el limite duro, mucho mas largo que el `SYNC_FRESCA_MS` del Maestro. **Obliga a arreglar la radio** |
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

**Por que no vale la pantalla:** cada punta calcula SU fase, y si las dos calculan mal —relojes separados, una
unidad reiniciada— **las dos diran que todo va bien** mientras las luces cuentan otra cosa. El verde y el despeje
de este modo son propios (`DEG_VERDE_SEG` / `DEG_DESPEJE_SEG`): **no heredan lo de Automatico**.

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

Tres cosas que el equipo hace solo, y la unica superficie donde se notan es **`MODO:`** del `$STATUS`
(`DEGRADADO` · `RENDIDO` · `SUBORDINADO`):

1. **Vuelve la radio ➜ sale del modo.** Solo con tramas de gobierno; las de servicio no sacan.
2. **Se agota el limite duro ➜ se rinde sola a ambar**, y **el firmware no deja reentrar**: contesta el rechazo de
   sync caducada. **Hay que arreglar la radio, no reactivar el modo.**
3. **Corte de energia ➜ PUEDE REANUDAR SOLA** (`D-29`), en todo-rojo, si al volver siguen vigentes hora, ciclo en
   la pila y sincronizacion fechable. **Nadie se lo avisa al operario.** 🔴 **Tras cualquier corte, verifique A.3 en
   las dos puntas aunque el equipo no se lo pida**: si una reanudo y la otra no, el cruce puede dar verdes por
   reloj sin que nadie haya mirado.

## A.6 🔴 LA COMPROBACION DE LA HORA QUE SIGUE HACIENDO FALTA, Y POR QUE

**`D-21` (1) esta CONSTRUIDA** —una hora que deja de ser fiable en marcha manda la punta a ambar—, pero la pieza
**(A)** —que el bit de parada del `DS3231` llegue al STM32— **sigue sin construir** (la medida es SPEC 3, H-2), asi
que **el unico detector de una hora que MIENTE es el tecnico** y estos dos pasos siguen vivos: **antes de autorizar
un Degradado, lea la hora de las DOS puntas** con la consulta de reloj (`LEER_RTC`, `D-17`: **lee y no escribe**)
**y contrastela con un reloj de fuera**; y **si una punta devuelve una fecha que no es la de hoy, no se entra en
Degradado en ese cruce** — se pone en hora primero, y la pila del `DS3231` es **consumible con fecha**, no
mantenimiento eventual.

# PARTE B — LA RADIO COMO APARATO: configuracion y antenas

**Que trata:** el modulo E90-DTU y su antena. **El protocolo que viaja por dentro es SPEC 2**, y el conector `J12`
con su cobre es SPEC 5. *(SPEC 2 §8 remitia «los parametros del modulo» a SPEC 5, que no los trae; corregido el
13/09: apunta aqui.)*

## B.1 Lo que NO se negocia

1. **`2.4 kbps` de Air Data Rate** (`CLAUDE.md` §3). La tasa vieja saturaba el canal y el cruce caia en fallo de
   comunicacion **justo al pasar el verde al otro poste**. Si el enlace sigue cayendo, **no se baja la velocidad
   otra vez**: se anota y se reporta.
2. **`M0` y `M1` los dos en `OFF` en operacion.** Los `ON` son **solo** para configurar, y ahi la buena es **`M0=ON`
   con `M1=OFF`**: la de `ON`/`ON` que un manual viejo publicaba deja la radio en un modo donde **la transmision
   puede quedar deshabilitada mientras la recepcion sigue activa** — radios que oyen y no contestan, un dia de campo.
3. **TODAS las radios del enlace, IGUALES.** Una punta con otra tasa o en otro canal **no enlaza**, y el sintoma no
   dice «mal configurada»: dice **«el equipo se callo»** y manda a mirar el cable.

## B.2 Los parametros, y cual de ellos puede este proyecto verificar

| parametro | valor | quien lo puede contradecir |
|---|---|---|
| **UART** | `9600`, `8N1` | 🟢 **el FIRMWARE**: `protocolo_setup()` abre el bus de radio a esa velocidad en **las dos puntas** (`Bus.begin()` en `protocolo.cpp`), y el Repetidor abre sus dos radios igual, `8N1` explicito. **Si la radio no esta a esa velocidad, no hay enlace posible** |
| **Air Data Rate** · **Potencia** · **FEC** · **Transmission Mode** · **Canal** | `2.4 kbps` · la maxima del modulo · activado · `Transparent` *(fixed-point desactivado)* · **el MISMO en las dos puntas** | 🔴 **nadie**: viven en la NVM del modulo y ningun instrumento de aqui los lee |

> 🔴 **NINGUN INSTRUMENTO DE ESTE REPOSITORIO LEE LA CONFIGURACION DE UNA RADIO.** La compuerta no la ve, el
> firmware no la puede preguntar y el `$STATUS` no la publica. **La unica evidencia de que una radio quedo bien
> configurada es que alguien lo escribio en un papel** — por eso B.3 acaba en una lista de casillas y por eso `RF:`
> de B.6 es la unica medida de verdad que queda. **El numero de canal no se publica aqui**: vive solo en la NVM y
> este fichero no puede recalcularlo (`CLAUDE.md` §14). **Lo que se exige es la IGUALDAD**, y el numero concreto se
> lee de la radio con la herramienta antes de tocar nada.

## B.3 El procedimiento, y el orden importa

1. **Radio sin energia.** Poner **`M0=ON`, `M1=OFF`**.
2. PC con convertidor **USB a RS485** (`A` con `485_A`, `B` con `485_B`) y 12 V a la bornera.
3. `04_Manuales/RF_Setting4.6.exe` ➜ puerto COM ➜ **`9600`** ➜ `Open Port` ➜ **`Read Option`** *(leer ANTES de
   escribir: ahi se ve el canal que la instalacion tiene de verdad)*.
4. Poner los valores de B.2 y **`Write Option`**.
5. **Quitar energia. `M0` y `M1` a `OFF` los dos.** Reconectar `485_A`/`485_B` al semaforo.
6. **Repetir en la otra punta** y comprobar que las dos quedaron **iguales**.

> 🛑 **NUNCA se energiza una radio sin la antena conectada.** Transmitir sin carga —o contra una antena desadaptada—
> **daña el amplificador de salida**, y ese es el modo de averia que ya se pago una vez.

**Casillas antes de cerrar los gabinetes:** ⬜ misma Air Data Rate en las dos ⬜ mismo canal ⬜ `M0`/`M1` en `OFF` en
las dos ⬜ antena conectada en las dos ⬜ apuntado en el parte **que radio quedo con que canal**.

## B.4 Topologia: hoy son DOS radios en enlace directo

El repetidor **no esta montado** y es reinstalable. **El firmware no cambia** al pasar de dos a cuatro: las puntas
son agnosticas a la topologia y **lo unico que cambia es el CANAL**.

> 🔴 **LA TRAMPA ESTA EN RETIRARLO, NO EN PONERLO.** Con repetidor, las dos puntas quedan en canales DISTINTOS a
> proposito —si no, el repetidor se oye a si mismo—. **Quitar el repetidor sin devolver la radio del Esclavo al
> canal del Maestro deja las dos puntas en frecuencias distintas**: no se comunican y **las dos caen a ambar
> intermitente** al vencer `SFTY6_SILENCIO_MS`. Parece radio rota.

## B.5 Antenas

**La especificacion completa de fabricacion —frecuencia, ROE, plano de tierra, BOM y cable— vive en
`05_Funcional/7_Especificacion_Antenas.md`** y no se copia aqui. Lo unico que se repite, porque puede herir:

- **Una antena se pide por su FRECUENCIA REAL, no por su etiqueta comercial**, y fuera de banda **devuelve la
  potencia al amplificador** — la explicacion mas probable de la averia de un transmisor. ⚠️ **Las genericas de
  «LoRa» del 31/07 eran de otra banda y no se reinstalan en ningun radio.**
- **El entregable que decide la compra es el reporte de ROE** en las frecuencias de operacion, todo a **50 Ω**: sin
  el **no hay forma de distinguir una antena correcta de una que solo lo afirma**.
- 🔴 **ANTES DE EMITIR UN PEDIDO: el modelo real de las radios sigue SIN CONFIRMAR** y la antena esta especificada
  sobre ese supuesto de banda. **Se cierra leyendo la etiqueta de la caja metalica** (HUECO 4).

## B.6 Como se mide el alcance HOY

**La pantalla `PRUEBA ALCANCE` ya no existe** (`D-17.bis`) y el Modo Alcance publicaba **por ese unico camino**:
entrar en el desde la app deja el cruce parado en rojo y **no ensena nada a nadie** (SPEC 4 §7.5). **En su lugar,
sin cambiar de modo y sin parar el cruce:** se leen **`RF:`** y **`RTT:`** del `$STATUS` del Maestro, **en el MISMO
punto** con la antena vieja y con la nueva, anotando distancia y hora — una prueba de alcance sin punto fijo no
compara nada. ⚠️ **Lo que se perdio y no tiene sustituto:** los contadores acumulados de latidos perdidos, bytes
recibidos y tramas validas **no salen en el `$STATUS`**; si una sesion de antenas los necesita, **pregunta para el
responsable.**

# PARTE C — LAS ALARMAS: censo completo, y que hace el tecnico con cada una

**`$ALARM` es la Caja Negra**: sale **en el instante** en que pasa algo y **solo lo ve quien esta conectado
entonces** (quien llegue despues lo lee en el `$STATUS` o en el Diario; formato en SPEC 4 §5). **Las emiten las DOS
puntas y nadie mas**: medido, el puente ESP32 **no origina ninguna** —solo retransmite— y el Repetidor no emite
tramas. **El campo `ACCION` dice lo que el equipo HIZO, no lo que deberia hacerse.**

| `EVENTO` | `CAUSA` | `ACCION` | quien | que la dispara | **que hace el tecnico** |
|---|---|---|---|---|---|
| `FALLO_RF` | `SILENCIO_<n>ms` | `CAMBIO_A_AMBAR` | **las dos** | no llega **nada** del otro poste en `SFTY6_SILENCIO_MS` | **el enlace esta CORTADO.** Parte B por orden: energia ➜ `M0`/`M1` ➜ misma tasa y canal ➜ antena y su conector. **Y ponerle la hora a este poste** (`D-26` (5)): sin radio, esa hora ya entra |
| `FALLO_RF` | `REINTENTOS_AGOTADOS` | `CAMBIO_A_AMBAR` | **solo Maestro** | se agotaron los reintentos de un cambio de luz. ⚠️ **Remedido el 13/09: la salida desde `C_ESPERANDO_ACK_GREEN` NO comprueba el enlace** —la de `C_ESPERANDO_ACK_RED` si—, asi que esta causa tambien sale con la radio MUERTA, y sale **antes** que la de silencio | **lo normal es que el enlace EXISTA y se degrade** —lluvia, distancia, interferencia—: eso es **cobertura y antena** (B.5), no configuracion. 🔴 **Pero si despues sale tambien `SILENCIO_<n>ms`, el enlace estaba CORTADO y entre las dos alarmas hubo un tramo con la otra punta pudiendo seguir en verde** (SPEC 2 §9). **No se vaya sin mirar la otra punta** |
| `AVISO_RF` | `SIN_CONFIRMAR` | `AVISE_POSTE_1` | **solo Esclavo** | se puso el ambar de emergencia aqui y **el Poste 1 no acuso** el aviso **ni tras agotar los reintentos** — `D-32` (3), 13/09: son `AVISO_AMBAR_REINTENTOS`, y la espera total es `(1 + N) x AVISO_AMBAR_TIMEOUT_MS`. ⚠️ **NO ES INMEDIATA: espere a que salga antes de irse** | **haga lo que dice la accion**: cierre el paso aqui o **avise al Poste 1 antes de irse**. 🔴 **Dice «no he podido confirmarlo», NUNCA «el otro poste no se entero»**: una trama perdida por lluvia se ve igual que un transmisor muerto |
| `HORA_ESP32` | `J17_MUDO` · `SIN_HORA_DEL_ESP32` · `RECHAZADA_FORMATO` | `SIGUE_SU_HORA` | las dos | la hora del ESP32 **no llega, o llega y no sirve** | **es de la MISMA placa**: el ESP32, su `DS3231` y el cable `J17`. Las tres causas mandan a sitios distintos: **SPEC 3 §7** |
| `HORA_ESP32` | `CADUCADA` | `CAMBIO_A_AMBAR` | las dos | la hora caduco **con el Degradado en marcha** (`D-21` (1)) | esa punta ya esta en ambar. Poner la hora antes de reintentar el modo (A.2, A.6) |
| `CAM_PEGADA` | `CAM_C_CONTACTO_FIJO` · `CAM_D_CONTACTO_FIJO` | **`NINGUNA`** | las dos | el contacto de esa camara lleva cerrado mas de `CAM_PEGADA_MS` | **el borne lo dice el nombre**: `CAM_C` y `CAM_D` son los dos de camara de `J16` (`D-25`, SPEC 5 §2.1). Revisar cableado y la configuracion de esa camara (`D-13`). **La causa dice `CONTACTO_FIJO` y NO «averia» a proposito**: no se puede distinguir de una presencia legitima larga |
| `CAM_CIEGA` | `CAM_C_SIN_FLANCO` · `CAM_D_SIN_FLANCO` | **`NINGUNA`** | las dos | esa camara **dio flancos antes** y lleva `CAM_CIEGA_MS` de paso abierto sin dar ninguno | lo mismo. ⚠️ **Solo puede salir de una camara que YA vio algo alguna vez**: una muerta desde la instalacion, o un borne vacio, **no la dispara nunca** (SPEC 5 §3.2) |

> 🔴 **`ACCION:NINGUNA` NO ES RELLENO: significa «medida de seguridad vial ejecutada: ninguna».** Las dos alarmas de
> camara **observan y no vetan** — no bajan la pluma, no tocan una luz y no paran el ciclo (`D-13` fase 1;
> **ninguna camara protege la pluma**, SPEC 5, aviso 4), asi que **no son una urgencia del cruce**.

**Cuando se apaga una alarma.** Las de camara **se cierran solas por su prueba contraria** y lo dicen con un
`$EVENT` `CAMARA / CAM_x_RECUPERADA`: una que no se cierra nunca deja al operario sin saber si aquello se arreglo.
Las de hora **se repiten mientras duren**, porque quien las tiene que ver es el que se conecte **despues**.

# PARTE D — `T-2`: EL PARCHE DE PACIENCIA PARA LA VERSION QUE ESTA EN LA CALLE

> 🔴 **ESTO NO DESCRIBE `main`, DESCRIBE UN PARCHE SOBRE LA VERSION DE CAMPO, Y CONFUNDIRLAS SERIA PEOR QUE NO
> ESCRIBIRLO.** Todo lo anterior y SPEC 2 cuentan el firmware de `main`. `T-2` es **otro binario**: la instalacion
> certificada —cual es y con que hash lo dice `ESTADO.md`, no este fichero— con **un numero cambiado**, para que el
> cruce ya montado deje de irse a ambar con la lluvia antes de que se le pueda cargar el firmware entero.

## D.1 El problema de campo, y por que encaja

El reporte del 13/09 —*«si llueve, el cruce se iba a ambar antes de 15 segundos, a los 7»*— sale de tres cosas
medidas sobre el propio commit de esa version: (1) su techo de orfandad son **12 s**, y ⚠️ **no es una constante
con nombre: es el literal `12000` escrito a pelo**, dos veces, una por punta (`Esclavo/src/main.cpp`, junto a
`tUltimoComando`; `Maestro/src/coordinador.cpp`, junto a `tUltimaRxEsclavo`) — **se dice asi porque es parte del
hallazgo**: no hay simbolo que citar, y es la forma que `N-69` castiga; (2) el latido **se suprime mientras se
espera un ACK** (`SFTY-13`, SPEC 2 §2.3), asi que el contador de silencio **llega envejecido en uno o dos latidos**
y **perder cuatro latidos seguidos manda las dos puntas a ambar** — de ahi el «a los 7»; (3) ⚠️ **la SEGUNDA via al
ambar, agotar los reintentos del cambio, en esa version NO SE ALCANZA**: el silencio vence antes y **los ultimos
reintentos nunca se han ejecutado en la calle** — lo que `costura_09` vigila en `main`, vivo aun en el poste.

## D.2 Que es `T-2`, y por que **17** y no el valor de `main`

`T-2` cambia **ese literal por `17000`**: dos lineas, una por punta, y **nada mas** —ni constante con nombre, ni
`N-163`, ni ninguna otra pieza de `main`—. 🔴 **Por que no se sube directamente al valor de `main`:** ese valor
separa mucho los dos plazos y abre una ventana con **una punta en VERDE y la otra en AMBAR INTERMITENTE** —el
Maestro cae por reintentos agotados y el Esclavo no suelta su verde hasta que vence SU silencio—, que **en un paso
alternado de un solo carril es exposicion frontal**. Con `17000` **las dos vias casi coinciden y la ventana vuelve
a ~0**, y la paciencia sube igual: el plazo crudo crece la diferencia entre los dos literales, **un latido entero
mas de racha perdida**. El concepto —**los dos relojes**— es **SPEC 2 §9**; `main` cierra esa ventana por otra via.

## D.3 ⚠️ EL LIMITE DECLARADO, QUE NO SE PUEDE OMITIR

**El commit de la version de campo NO SE PUEDE RECONSTRUIR DESDE GIT, y esta medido:** el `main.cpp` de su Maestro
hace `#include "modo_alcance.h"` y **ese fichero no esta en ese commit** — un `.gitignore` en **UTF-16** ocultaba
nueve ficheros y se arreglo **tres horas mas tarde**. El binario de `T-2` es, literalmente, **«la version de campo
+ dos ficheros recuperados de 3 h despues + un numero»**, y asi hay que decirselo a quien lo cargue. **Y lo que
sigue sin ser:** sus binarios llevan **`_SIN_BANCO`**, que **no es decoracion** (`CLAUDE.md` §13) —nadie los ha
pasado por banco ni por una tarjeta, y **su ausencia se leeria como permiso**—. ⚠️ **En la carpeta de entrega
conviven DOS parejas y un `.patch` que documenta el intento anterior, no el bueno**: vale el que lleva el plazo en
el nombre, y **antes de cargar nada se comprueba el `sha256sum`, nunca el tamano**.

# HUECOS MEDIDOS

**1. 🔴 EL MODO DEGRADADO NO DEJA RASTRO EN EL DIARIO — y el caso de uso es justo el que llega despues.** Censado
sobre `{Maestro,Esclavo}/src/modo_degradado.cpp`: **el unico `$EVENT` del modo es `DEGRADADO /
SALTO_DE_HORA_POR_ROJO`**. No hay traza de que una punta **entro**, **salio**, **se rindio** ni **reanudo sola tras
un corte** (A.5), y `MODO:` dice el AHORA, no el CUANDO. El molde existe —`D-32` (2)— pero **no hay fila que lo
pida.**

**2. 🔴 EL AVISO PREVIO AL LIMITE DURO NO TIENE DONDE LEERSE, EN NINGUNA DE LAS DOS PUNTAS.** Medido: el del Maestro
se compone dentro de la funcion que **dibuja la pantalla** (`AVISO_LIMITE_MS` ➜ `lcd_dibujarDegradado()`) y el del
Esclavo (`AVISO_SIN_SYNC_MS` ➜ `degradado_avisoLimite()`) **solo lo llama `menu.cpp`** — **y el LCD se retiro**
(`D-17.bis`). **Ni `$ALARM`, ni `$EVENT`, ni campo del `$STATUS`:** el operario se entera cuando ya es
`MODO:RENDIDO`. **Y los dos umbrales tampoco son el mismo numero en las dos puntas.**

**3. 🔴 NADA DE ESTE REPOSITORIO VERIFICA LA CONFIGURACION DE UNA RADIO** (B.2): ni instrumento, ni lectura de
vuelta, ni acta — solo la casilla que marque una persona. · **4. 🔴 EL MODELO REAL DE LAS RADIOS SIGUE `SIN
VERIFICAR`, y la antena esta especificada sobre ese supuesto** (B.5), abierto desde el 01/08. · **5. 🔴 `D-21` PIEZA
(A) SIN CONSTRUIR: el unico detector de una hora que miente es el tecnico** (A.6). · **7. ⚠️ El plazo real del
todo-rojo de salida del Esclavo no esta medido**: A.4 dice «tarda» y no cuanto, porque nadie lo ha cronometrado.

**6. ⚠️ Que hacer EN LA CAMARA cuando salta `CAM_PEGADA` o `CAM_CIEGA` no esta escrito en ningun sitio.** Aqui se
manda a revisar el borne y la configuracion **porque eso es lo que fijan `D-25` y `D-13`**; **el diagnostico de la
camara como aparato no lo cubre nadie**, y no se inventa. Ademas, **ninguna camara se ha conectado nunca a este
equipo** (SPEC 5 §6.1): **estas dos alarmas jamas se han visto salir**.

**8. ⚠️ Nada de este capitulo ha visto una tarjeta.** Ni el Degradado por app, ni las alarmas, ni una radio
reconfigurada con este firmware dentro — **ni `T-2`**. Que firmware corre en cada equipo lo dice `ESTADO.md`.

**9. 🔴 `T-2` NO TIENE FILA EN `DECISIONES.md`** (PARTE D). Censado el 13/09: la tabla llega hasta `D-32` y ninguna
fila habla de subir el techo de orfandad de la version de campo. **Describe un binario construido, no una decision
tomada**, y sin fila **no hay nada escrito que autorice cargarlo** ni que fije cual plazo es el bueno. Se nombra
como hueco; esta spec no abre filas.

*Vigentes recogidas: `D-1`, `D-2`, `D-13`, `D-16`, `D-17`, `D-17.bis`, `D-18`, `D-21` (1), `D-25`, `D-26` (3) y
(5), `D-29`, `D-31`, `D-32` (2) y (3). Nombradas como hueco: `D-21` (A) y `T-2` sin fila. Remedido el 12/09/2026
sobre `{Maestro,Esclavo}/src/{modo_degradado,bluetooth,botones,coordinador,main,protocolo,reloj}.cpp`, y el 13/09
sobre `coordinador.cpp`, `Esclavo/src/main.cpp`, `Esclavo/include/reloj.h`, `costura_09_presupuesto_radio.py` y el
commit de campo. Censados y NO copiados: `8_Procedimiento_Modo_Degradado.md`, `4_Manual_Configuracion_Radios.md`,
`7_Especificacion_Antenas.md`, `MANUAL_EXACTO_RADIOS_E90_DTU.md`.*
