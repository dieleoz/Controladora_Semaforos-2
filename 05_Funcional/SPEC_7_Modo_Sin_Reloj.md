# SPEC 7 — MODO SIN RELOJ Y SIN RADIO: el veredicto, y lo que si hay que construir

> 🔴 **VEREDICTO, Y VA EN LA PRIMERA LINEA PORQUE ES LO QUE DECIDE SI SE SIGUE LEYENDO.**
> **El modo NO se construye, y por dos motivos opuestos que hay que separar:**
>
> 1. **«SIN RELOJ» NO SE SOSTIENE. Sin un `DS3231` vivo el cruce aguanta MINUTOS, no dias**
>    —el plazo exacto es `HORA_CADUCA_MS` y lo derivan cuatro `static_assert` de `reloj.h`—.
>    Un operador que tuviera que volver a esa cadencia no es un modo de operacion.
> 2. **«SIN RADIO» YA ESTA CONSTRUIDO Y EN `main`.** Es el Modo Degradado de hoy con
>    `D-26` (3): sin radio, cada punta corre con la hora de su propio `DS3231`, y el operador
>    se la pone con el telefono en el gabinete. No hace falta un modo nuevo: **ya existe, se
>    fusiono el 11/09 y no ha visto una tarjeta.**
>
> **Lo que SI falta es lo otro que el responsable pidio el mismo dia: QUE LA APP DIGA POR QUE el
> cruce esta en Degradado.** Hueco real, medido, y **empeoro esta manana** cuando `D-32` (1) saco
> el LCD. Es §6, y es lo unico de aqui que es teclado.
>
> **Y una TERCERA cosa que el encargo destapa y que nadie habia escrito: la visita del operador
> NO PUEDE ALARGAR EL MODO.** El tope son `LIMITE_DURO_MS` / `LIMITE_SIN_SYNC_MS` **desde la
> ultima sincronizacion POR RADIO**, y la hora del telefono **no la renueva**. No es un defecto:
> es una barrera deliberada. Pero significa que **visitar periodicamente para seguir operando hoy
> no funciona, y abrirlo es una FILA SUYA, no un modo** (§9).

**ESTADO: PROPUESTA con veredicto negativo.** Salvo lo marcado como construido —comprobable en
el fuente— **nada de aqui describe el equipo de hoy**; lo que no existe vive en §8, «HUECOS
MEDIDOS» (`CLAUDE.md` §1). ⚠️ **Ningun instrumento lee las `SPEC_*`** —censado el 13/09 sobre
`banco/packs/*.py` y `compuerta.py`, cero apariciones—: **esta spec no la vigila nadie.**

**Alcance.** Si el modo propuesto el 13/09 se sostiene; donde vive la hora entre visitas; su
camino real; que pasa si el operador no vuelve; y que tendria que publicar cada punta para que
la app diga POR QUE. **Fuera:** de donde sale la hora y como se siembra (**SPEC 3**, que manda en
todo lo que sea reloj y no se repite aqui) · el ciclo (SPEC 1) · la radio (SPEC 2) · la app por
dentro (SPEC 4) · el cobre (SPEC 5).

> 🔴 **NINGUNA CIFRA VIVE EN ESTE FICHERO:** se nombra la constante y la consecuencia en
> palabras. Fuentes: `{Maestro,Esclavo}/include/reloj.h`, los dos `src/modo_degradado.cpp` y
> `ESP32_Expansion/include/contrato.h`. Quien las recalcula en cada corrida: **`reloj_04`** (el
> plazo y el margen que queda) y **`esp32_13`** (la cadencia contra el aguante del cruce). Un
> numero copiado aqui naceria caducado (`CLAUDE.md` §14).

## 1. LA PREGUNTA QUE DECIDE: donde vive la hora entre una visita y la siguiente

Tres sitios posibles. **Dos no existen y el tercero es el de hoy.**

### (a) En el `DS3231` del ESP32 — **existe, y entonces el modo NO es «sin reloj»**

Es un TCXO con pila propia, uno por poste (`D-9`, `D-15`). Con el vivo, cada STM32 se resiembra
cada `SIEMBRA_INTERVALO_MS` **haya radio o no** —`siembra_revisar()` no pregunta por la radio—,
y el Esclavo la acepta en cuanto `reloj_radioManda()` es falso (`D-26` (3)). **Eso es
exactamente el modo que se pide, y es el Degradado de hoy.**

🔴 **Que averia lo deja inservible — medido, y el resultado es que NO HAY CASO INTERMEDIO.** El
unico contenido que sale hacia el STM32 lo compone `siembra_ahora()` a partir de `reloj_leer()`,
que **relee el chip en cada llamada y no tiene variante «damela igual»**. Si el bus no contesta
(modulo ausente, `SDA`/`SCL` cruzados), si el `OSF` esta puesto (oscilador parado, pila agotada)
o si los registros no cuadran al releerlos, **no sale NADA** — ni un hueco, ni un cero, ni la
ultima que se vio. Y **no hay reloj de software de respaldo**: el calendario de `siembra.cpp` es
solo cadencia y no sobrevive al reset, a proposito.

> **Consecuencia: «`DS3231` inservible» y «el ESP32 no tiene hora» son el MISMO estado.** El
> caso (b) no es una opcion peor: **no existe.**

### (b) En el ESP32 sin `DS3231` — **NO EXISTE en el firmware**

No hay cristal ni oscilador del ESP32 llevando la hora en ninguna linea de
`ESP32_Expansion/`: la hora se lee del chip o no se lee. **Medir su deriva seria medir un camino
que nadie escribio.** Construirlo seria darle al accesorio una hora **que ninguna barrera
valida**, que es justo lo que la barrera del contenido de `siembra.cpp` existe para impedir.

### (c) En el STM32 — **AQUI ESTA LA TRAMPA, y esta medida**

El STM32 **no tiene reloj que sirva**: `Y2` esta confirmado muerto (`N-17`, ⚠️ **medido en UNA
tarjeta**) y su calendario esta anclado por construccion (`reloj_fijarEnero()`). Lo que tiene es
una base de software extrapolada con `millis()`, y **`millis()` corre sobre el HSI**, cuyo peor
caso de ficha es `HSI_PPM_PEOR` —del orden del **2,5 %**, no de las decenas de ppm de un
cristal—.

⚠️ **La pila `CR2032` SI esta y midio 3 V con la tarjeta apagada (`N-37`): el que no oscila es el
cristal, no la pila.** Y `Y1` (8 MHz) **esta soldado y no se ha arrancado nunca** — `D-22`,
**OPCIONAL y la ULTIMA de la cola** desde el 07/09, con su riesgo escrito: si `Y1` no oscila y el
arranque lo exige, la tarjeta queda **a oscuras**. **Nada de esta spec supone que `Y1` exista.**

## 2. LA ARITMETICA, con los DOS extremos

**El aguante del cruce** —cuanto pueden separarse las dos puntas antes de que los verdes se
solapen— **no se escribe: se BARRE** con el modelo de costura (`_aguante()` de `esp32_13`) sobre
`DEG_VERDE_SEG`, `DEG_DESPEJE_SEG` y el ambar. **Las dos puntas pueden irse en sentidos
CONTRARIOS**, asi que lo que se compara contra el aguante es siempre la separacion RELATIVA:
`2 x deriva_de_una_punta + 2 x el residuo de segundo entero de cada siembra` (`_relativa_s()` de
`reloj_04`).

| si la hora vive en… | deriva relativa | plazo antes de solapar verdes |
|---|---|---|
| **el STM32 solo** (HSI, sin sembrar) | `2 x HSI_PPM_PEOR` = **~5 %** | **MINUTOS.** El aguante dividido por esa cifra da **menos de diez**, y el firmware corta antes y a proposito: `HORA_CADUCA_MS` |
| **el ESP32 sin `DS3231`** | — | **no existe** (§1.b): sin chip no hay siembra, luego es la fila de arriba |
| **los dos `DS3231`** (hoy) | lo que difieran dos TCXO de su ficha, uno contra otro | **SEMANAS.** Es el margen que `reloj_04` publica en cada corrida dividido por esa deriva |

> 🔴 **LA CUENTA DEL STM32 SOLO, QUE ES LA QUE MATA EL MODO: el aguante del cruce entre dos HSI
> corriendo libres se agota en menos de diez minutos.** El firmware no llega ni a eso: a
> `HORA_CADUCA_MS` de la ultima siembra buena, `reloj_horaFiable()` cae, la punta publica
> `$ALARM HORA_ESP32 / CADUCADA / CAMBIO_A_AMBAR` y **se va a ambar intermitente sin volver
> sola** (`D-21` (1)). **Un modo que pidiera al operador volver a esa cadencia no es un modo: es
> una averia con horario.**

> ⚠️ **LA CIFRA DE DIAS DEL CASO (c) NO SE ESCRIBE AQUI, Y NO ES PUDOR: ES §14.** Sale de dividir
> el margen que `reloj_04` publica por **la ficha de un componente**, y **ningun instrumento cruza
> esas dos cosas**: nace caducada. Ya paso —`18_Especificacion_Firmware_ESP32` tenia cuatro cifras
> de este mismo calculo y el 12/09 **se RETIRARON en vez de actualizarse**—. La consecuencia en
> palabras, que no envejece: **con el `DS3231` vivo el limite no lo pone la fisica, lo pone §5.**
> Y ese margen **se ESTRECHA con cada decision que compra robustez** —`D-28` (2) lo recorto a
> proposito—: **`reloj_04` falla el dia que deje de ser positivo.** Esa es la vigilancia real.

## 3. EL CAMINO REAL, y por que el que el responsable describio tiene un tramo que no existe

**Lo que se propuso:** telefono -> ESP32 Maestro -> STM32 Maestro -> **radio** -> STM32 Esclavo
-> **ESP32 Esclavo**.

**Medido sobre el fuente, esa cadena falla por los dos extremos:**

- **El ultimo salto NO EXISTE.** No hay camino `STM32 -> ESP32` que lleve la hora: `CMD:HORA_ESP32`
  va en el otro sentido. La «cadena completa» —que el STM32 Esclavo devuelva a su ESP32 la hora
  que recibio por radio— esta **aparcada por `D-26`** y anotada en `roadmap.md` §2.8.
  **Construirla es firmware nuevo en las dos puntas.**
- **El penultimo es la RADIO, que en este modo es justo lo que no hay.**
- **Y los dos ESP32 no se hablan:** censado sobre `ESP32_Expansion/` por radio, `esp_now` y red —
  **cero apariciones**. Lo unico que une los postes es la radio entre los STM32.

🟢 **PERO LOS DOS PRIMEROS SALTOS SI EXISTEN, Y ESO HACE INNECESARIO EL RESTO.** Un `SET_RTC`
del telefono lo **atiende el puente y ya no sigue viaje** (`D-26` (1)); el puente escribe el
`DS3231`, **lo relee**, y **en esa misma transaccion llama a `siembra_ahora()`**, que pone la
hora en su STM32.

> 🎯 **El camino real del modo que se pide es mucho mas corto que el propuesto, y ya esta
> construido:** `telefono --SET_RTC--> ESP32 del poste que se visita --CMD:HORA_ESP32--> su
> STM32`. En el Esclavo esa hora **entra** en cuanto `reloj_radioManda()` es falso (`D-26` (3)),
> que es la definicion de «sin radio» y **el mismo umbral** con el que sale `$ALARM FALLO_RF`. Y
> `D-21` (3) ya lo dice: *«la alarma se quita poniendole la hora al Esclavo desde el telefono»*,
> **porque es lo unico que funciona con la radio caida, que es cuando hay un tecnico delante**.

⚠️ **Si algun dia se quisiera el camino largo de verdad, lo que cuesta es una BARRERA, no
teclado:** un aviso nuevo `ESP32 -> STM32` seria **la TERCERA orden que el accesorio origina
hacia el micro que gobierna el cruce**, y `esp32_05_no_origina` la condiciona **por escrito** a
*«una decision escrita en `DECISIONES.md`, no un comentario»*. Es la misma puerta que `D-32` (2)
decidio **no cruzar** el 13/09.

## 4. Como se habilitaria — **PROPUESTA**, y es casi todo «no se toca»

| pieza | estado |
|---|---|
| pedir el modo en el poste 2 | 🟢 **existe**: `SET_MODO:DEGRADADO` por la app (`D-18`), con un `$ERR` por cada motivo de rechazo |
| la hora del propio poste sin radio | 🟢 **existe**: `D-26` (3), `reloj_radioManda()` |
| ponerle la hora con el telefono en el gabinete | 🟢 **existe**: `SET_RTC` -> `DS3231` -> `siembra_ahora()` |
| que la hora que miente no de verdes | 🟢 **existe**: `reloj_horaFiable()` en la PUERTA y en el BUCLE, las dos puntas |
| **que la visita ALARGUE el modo** | 🔴 **NO existe, y es deliberado** — §5 |
| **que la app diga POR QUE** | 🔴 **NO existe** — §6 |

> **O sea que «habilitar el modo» no es habilitar nada: es contestar las dos filas rojas.** Y
> ninguna de las dos se arregla con un modo nuevo (`CLAUDE.md` §8.1: un instrumento que
> certifica otra vez lo ya certificado sustituye).

## 5. Que pasa si el operador NO vuelve a tiempo — **la barrera ya esta, y avisa antes**

🟢 **EL TOPE EXISTE Y ES DURO.** Pasado `LIMITE_DURO_MS` (Maestro) / `LIMITE_SIN_SYNC_MS`
(Esclavo) **desde la ultima sincronizacion confirmada con la otra punta**, el Degradado termina:
el Maestro llama `irAAmbar("Limite 48h sin sync", "Revise el radio")` y el Esclavo se **rinde**
por todo-rojo hasta `DEG_RENDIDO`, que es ambar intermitente. **El motivo esta en el fuente y es
la frase que gobierna esta spec entera:** *«EL ESTADO SEGURO NO PUEDE DEPENDER DE QUE ALGUIEN SE
ACUERDE»*. Y la puerta de entrada del Maestro es aun mas estrecha: exige sincronizacion
**fresca** (`SYNC_FRESCA_MS`) y desfase dentro de `TOLERANCIA_DESFASE_S`, las dos **por radio**.

🔴 **LO QUE LA VISITA NO HACE, Y ES EL HALLAZGO DEL ENCARGO: la hora del telefono NO RENUEVA EL
TOPE.** Medido, y **SPEC 3 §3 lo decia sin sacar la consecuencia**: la hora del `DS3231` propio
no llama ni a `degradado_registrarSync()` ni a `respaldo_marcarSync()`, porque **una hora local
no demuestra que las dos puntas sigan en fase**. Es correcto — pero significa que **el operador
puede visitar el poste todos los dias y el cruce se ira a ambar igual al vencer el tope**, y lo
unico que lo levanta es que vuelva la radio.

🟢 **EL AVISO ANTICIPADO SE PUBLICA, EN LAS DOS PUNTAS.** Cada una emite un `$EVENT` de origen
`DEGRADADO` con **cuantas horas lleva sin sincronizar, si el aviso esta armado y si el plazo ya
vencio**: sale cuando cambia alguno de los tres y se repite con la cadencia del diagnostico
periodico mientras el aviso siga armado. El equipo **avisa antes de rendirse**, y la antiguedad de
la sincronizacion **si sale**.

> 🔴 **LO QUE SIGUE ABIERTO ES OTRA COSA, Y SON DOS:** que **los dos postes avisan con plazos
> DISTINTOS** —el 1 en las ultimas cuatro horas de las 48; el 2, antes— **y nadie cruza los dos
> numeros**; y que **la CUENTA ATRAS del Degradado no la publica ninguna de las dos puntas**: las
> funciones que la calculan se quedaron sin un solo llamador cuando se retiro la pantalla.

**Un modo que depende de una visita y no sabe que la visita no llego es peor que no tenerlo.** Con
el tope, el cruce **si lo sabe, lo dice antes y se degrada solo a ambar**; lo que no dice es **por
que** esta en Degradado, y eso es §6.

### 5.1 Y ese tope de 48 h no cae al segundo: puede cumplirse antes o despues, hasta por una hora

El equipo mide ese plazo con **dos relojes a la vez y se queda con el que diga mas tiempo**: un
contador que mantiene viva la pila de boton de la tarjeta —que sigue andando aunque se vaya la
luz— y el reloj de programa, que cuenta desde que la tarjeta arranco y **un corte lo pone a cero**.

🔴 **En las tarjetas que hay hoy en campo el cristal del contador de pila NO OSCILA**, asi que en
la practica queda solo el reloj de programa. Y ese lo gobierna el **oscilador interno de emergencia**
del microcontrolador, que es impreciso a proposito porque no es un cristal.

**Cuanto se desvia, y HACIA DONDE — las dos cosas, porque el sentido no es uno solo y no es
simetrico.** Ese oscilador puede correr **rapido o lento** segun la tarjeta y la temperatura, y **no
se sabe cual toca en cada equipo**. 🔴 **La cifra es de FICHA DEL FABRICANTE: nadie la ha medido en
una tarjeta de estas** *(y por eso se escribe asi: un numero de ficha no es una medida)*. De ficha,
sobre 48 horas el tope puede cumplirse **hasta ~1 h 10 min antes** —por el lado que se adelanta— o
**hasta ~1 h despues** —por el que se atrasa—.

- **Si se cumple antes:** el cruce se va a ambar intermitente **cuando todavia no hacia falta**.
  Molesta a quien lo opera; no hiere a nadie.
- **Si se cumple despues:** el cruce sigue repartiendo el paso **un rato mas de lo previsto** con los
  dos relojes ya separados. Es la direccion incomoda. ⚠️ **Y el plazo de 48 h NO se eligio con
  holgura, que es lo que parece:** esta medido con **8,8 segundos de margen** sobre los 29 s de
  desfase que el cruce aguanta, y esa cuenta se hizo con unos cristales que **en las tarjetas de hoy
  no oscilan**. Alargar el plazo obligaria a alargar el todo-rojo —una semana pediria ~90 s, que
  destroza la fluidez del paso—: **la salida real no es estirar el limite, es ir a arreglar la
  radio.**

⚠️ **Se acepta asi, y por que no se arregla:** la tarjeta lleva montado un segundo cristal que
quitaria la imprecision, pero **el firmware no lo ha encendido nunca**, y encenderlo tiene un modo de
fallo **peor que el problema que resuelve**: si ese cristal no llegara a oscilar, el arranque se
queda colgado **antes de encender una sola lampara** — tarjeta muerta, sin luces y sin reiniciarse
sola. Hacerlo bien obliga a escribir a mano una salida de emergencia, a **recalibrar todo lo que el
equipo mide por tiempo** —el micro pasaria a correr un 12 % mas rapido— y a probarlo **con una
tarjeta delante**.

🟡 **Que sitio ocupa ese trabajo en la cola lo decidio el responsable el 14/09: el ULTIMO, y solo,
y no se carga sin una tarjeta delante.** Lo que se acepta a cambio es el desvio de arriba.
*(Ficha interna: `D-22`, ordenada por `A-16`. El detalle tecnico del cristal esta en el apartado 3
de este mismo documento y no se repite aqui.)*

## 6. POR QUE esta el cruce en Degradado — **el censo, y que dato falta**

**La entrada es SIEMPRE de una persona** (`SFTY-21`, activacion manual): no hay ni un camino que
meta el cruce en Degradado solo. Asi que «por que esta en Degradado» son **tres preguntas**:

| pregunta | quien la sabe | sale hoy? |
|---|---|---|
| **por que lo pusieron** (la radio se cayo) | `$ALARM FALLO_RF` con su causa, y el `$EVENT ENLACE_RF` | 🟢 **sale**, pero como **evento pasado**: el tecnico que llega DESPUES depende de la bitacora |
| **por que NO puede entrar** | `MDG_FALTA_HORA`, `MDG_NUNCA_SYNC`, `MDG_SYNC_VIEJA`, `MDG_SIN_DESFASE`, `MDG_DESFASE_ALTO`, `MDG_SIN_CONFIG` (Maestro) · `DEG_RECHAZO_SIN_HORA`, `SIN_CONFIG`, `CICLO_NULO`, `SIN_SYNC`, `SYNC_VENCIDA`, `AMBAR_VIGENTE` (Esclavo) | 🟡 **solo como `$ERR` a quien mando la orden**. Quien se conecta despues no lo ve |
| **por que se SALIO** (ambar) | `irAAmbar("Reloj no fiable")` · `irAAmbar("Limite 48h sin sync")` · la rendicion del Esclavo | 🟡 el `$ALARM HORA_ESP32/CADUCADA` si sale; **el vencimiento del tope no publica causa propia** |

⚠️ **Y LO QUE LA APP HACE HOY ES PEOR QUE UN HUECO: AFIRMA LA CAUSA SIN QUE NADIE SE LA MANDE.**
Medido sobre `app.js`: el badge de `DEGRADADO` dice *«SIN ENLACE ENTRE POSTES»* y el de `RENDIDO`
*«DEGRADADO VENCIDO (48 h)»*. **Esos textos estan escritos en la app**, deducidos del nombre del
modo; el equipo publica `MODO:` y `ESTADO:`, **nunca la causa**. Si el operario lo puso en
Degradado por otro motivo, la app dice el motivo equivocado con cara de dato.

### Lo que tendria que publicar cada punta — **PROPUESTA**

**El mismo dato en las dos, leible conectandose a cualquiera** (el requisito del responsable):
**(1)** la CAUSA vigente del estado degradado —el motivo que el firmware ya calcula— y **(2)** la
ANTIGUEDAD de la ultima sincronizacion contra su tope, que es lo que dice cuanto queda. **De la
(2) ya salen la antiguedad y si el aviso esta armado** (§5); lo que no sale es cuanto queda (H-1).

🔴 **La via NO es un campo nuevo del `$STATUS`: esta medido que NO CABE.** `esp32_07` lo
recalcula en cada corrida —al peor `$STATUS` del **Maestro** le quedan menos caracteres libres de
los que ocupa un campo— y `documentos_03_trama_status` **prohibe** que el Esclavo emita un campo
que el Maestro no emita, asi que manda el margen del Maestro. **La via que queda es la que
`D-32` (2) eligio y que ya esta construida el 13/09: el `$EVENT` periodico**, que hoy lleva el
diagnostico del enlace (`ORIGEN:ENLACE_RF`) y el del Degradado (`ORIGEN:DEGRADADO`, §5). **Anadirle
la causa es extenderlo, no abrir camino nuevo**, y **no toca `esp32_05_no_origina`**.

⚠️ **Con su coste ya aceptado:** el tecnico lo ve **al cabo de la cadencia**, no al instante,
porque **el STM32 no puede saber que hay un telefono**. Y la cadencia **no baja**: cada `$EVENT`
entra en la bitacora de la app, que **recorta a un numero fijo de entradas**, y llenarla se
comeria los `$ALARM` que el tecnico vino a leer (`N-73` por inundacion).

## 7. Que se pierde sin radio — **dicho sin suavizarlo**

**Sin radio NADIE CONFIRMA NADA, y el Degradado es el unico modo que da verde SIN confirmacion
de la otra punta.** No es la coordinacion degradada: es la coordinacion **sustituida por dos
relojes que se creen el uno al otro sin hablarse.** Las tres perdidas:

1. **El acuerdo de fase deja de comprobarse.** El desfase solo se mide por radio
   (`coordinador_medirDesfase`). Sin ella, que las dos puntas sigan en fase es una
   **suposicion**, y lo unico que la acota es el tope de §5.
2. **La asimetria no tiene cura tecnica.** Si una punta pierde la hora fiable y la otra no, **una
   se va a ambar y la otra sigue dando verdes**: sin radio **no se puede ordenar «ambar en las
   dos»**. Es el `Riesgo 2`, **aceptado desde el 01/08** (`D-21`).
3. **Lo que difieran los dos `DS3231` no lo vigila nadie.** La alarma por discrepancia esta
   **propuesta y sin construir** (`roadmap.md` §2.8). **Hoy el cruce no sabe que sus dos relojes
   se han separado: solo lo sabe quien visite los dos postes y compare con `CMD:LEER_RTC`**
   (`D-17`).

## 8. HUECOS MEDIDOS — lo que NO existe, y no se escribe como si existiera

### H-1 🟡 El aviso previo YA se publica; lo que no casa son los dos plazos

⬇️ ~~El aviso anticipado esta declarado y no se ejerce~~ → **construido el 13/09 y remedido el
14/09** (§5). **Lo que queda abierto son dos cosas distintas, y ninguna tiene fila:**

1. **Los dos postes avisan con plazos DISTINTOS** —el 1 en las ultimas cuatro horas de las 48;
   el 2, antes— **y nadie cruza los dos numeros**, ni un `static_assert` ni un instrumento. El dia
   que alguien mueva uno, el otro no le sigue.
2. **La CUENTA ATRAS del Degradado no la publica ninguna punta.** Las dos funciones que la calculan
   se quedaron **sin un solo llamador** al retirarse la pantalla, y un comentario del firmware decia
   que en el poste 2 si salia —**tambien falso, corregido el 14/09**—.

### H-2 🔴 La causa del Degradado no se publica: la app la INVENTA

Los motivos de rechazo y de salida existen como enumerados en las dos puntas y **solo salen como
`$ERR` a quien mando la orden**. Los textos de causa que el operario lee estan escritos en
`app.js`. **Sin `D-x` que lo ordene, no se construye.**

### H-3 🔴 La visita no renueva el tope, y en ningun documento estaba escrita la consecuencia

Medido en §5. **No se propone cambiarlo aqui**: relajar el tope es retirar una barrera, y una
proteccion amputada no se nota hasta que alguien esta en la calzada (`CLAUDE.md` §8.3). Va a §9.

### H-4 El camino `STM32 -> ESP32` no existe, y su coste no es teclado

`roadmap.md` §2.8 lo tiene como mejora aparcada por `D-26`. Abrirlo es **la tercera orden del
accesorio hacia el micro** y `esp32_05_no_origina` lo condiciona a una fila de `DECISIONES.md`.

### H-5 Nada de esto ha visto una tarjeta

`D-26`, `D-28` y `D-21` (1) estan en `main` y **SIN BANCO**: lo que hay son packs y arneses de PC,
y **un verde de la compuerta no dice que el firmware funcione en la tarjeta** (`CLAUDE.md` §0.3).
La unica medida sobre el aparato real —la cinta del Sisga— es la que destapo el cristal que arranca
y no cuenta (`H-1` de **SPEC 3**: deteccion construida, sin banco), que no se repite aqui.

## 9. Lo que esta spec NO decide — **son DOS filas del responsable, no dos modos**

1. 🔴 **¿Puede una visita con el telefono renovar el tope del Degradado?** Hoy no: una hora local
   **no demuestra que las dos puntas sigan en fase**. Abrirlo cambiaria el sujeto del tope —de
   «hace cuanto que las dos puntas se hablaron» a «hace cuanto que alguien vino»—, o sea **que
   garantiza el estado seguro**. Si se decide que si, **hay que exigir que la visita compare los
   DOS relojes** (`CMD:LEER_RTC` en los dos postes, `D-17`): poner uno solo los separa mas.
2. 🟡 **¿La causa entra en el `$EVENT` periodico de `D-32` (2)?** La antiguedad ya va (§5). Es la
   via ya elegida, no toca ninguna barrera y el caudal lo aguanta. **Es teclado en cuanto haya fila.**

> ⚠️ **Y una que no es del responsable sino del banco:** al retirar el LCD el aviso previo se
> quedo sin lector y ningun instrumento lo acuso, porque `costura_10_funciones_muertas` aceptaba
> el huerfano con su motivo escrito. **Se sustituyo por el `$EVENT` de §5** y sus getters salieron
> de esa lista; lo que sigue sin lector es la cuenta atras (H-1).
