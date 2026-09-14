# SPEC 3 — LA HORA: de donde sale, y que pasa cuando se pierde

**Que autoriza esta spec.** El **Modo Degradado** da verdes **sin confirmar con la otra punta**: la fase
la calcula cada poste por su cuenta, a partir de su hora de pared. Si la hora miente, **los dos postes
pueden dar verde a la vez.** Todo lo que sigue existe por esa frase.

**Alcance.** De donde sale la hora, quien siembra a quien y cada cuanto, que pasa al perder la radio,
cuando la hora CADUCA, el ambar de la punta que la perdio, la reanudacion tras un corte de energia y
las alarmas de reloj. **Fuera:** el ciclo (SPEC 1) · la coordinacion de radio en si (SPEC 2) · la app
(SPEC 4) · el cobre (SPEC 5).

> 🔴 **NINGUNA CIFRA VIVE EN ESTE FICHERO.** Se nombra la constante y su consecuencia en palabras.
> La fuente de todo numero de plazo y cadencia es **`{Maestro,Esclavo}/include/reloj.h`** y
> **`ESP32_Expansion/include/contrato.h`**; los packs `reloj_04` y `esp32_13` los recalculan del C++
> en cada corrida. Un numero copiado aqui naceria caducado (`CLAUDE.md` §14).

## 1. Quien tiene el reloj

**Hay DOS relojes por cruce, y ninguno esta en el micro que gobierna las luces.**

| | |
|---|---|
| **ESP32 de cada poste** | lleva un **`DS3231` con pila propia**. Es **la autoridad de la hora, siempre y para todo** (`D-20`, que desarrolla `D-9`) |
| **STM32 de cada poste** | **no tiene reloj que sirva** (`D-9`). Su cristal `Y2` esta confirmado muerto en banco (`N-17`) y su calendario esta anclado a enero por construccion (`reloj_fijarEnero()`) |

**Al STM32 no se le pregunta la hora nunca.** Lo que tiene es una **base de tiempo de software**
—`segBaseDelDia` + `tBaseMillis`, extrapolada con `millis()`— que le **siembra** su propio ESP32. Ese
`millis()` corre sobre el **HSI**, el oscilador RC interno del micro, cuyo peor caso de ficha esta
escrito en `HSI_PPM_PEOR` (`reloj.h`): **es el que decide cuanto se separan las dos puntas entre
siembras, y es la razon de todo el resto de esta spec.** 🟡 **Y sobre ese HSI hay una decision VIGENTE
que ninguna spec nombraba: `D-22`** —montar `Y1` (8 MHz, ya soldado y sin usar) como reloj de sistema,
de 10.000-25.000 ppm a 20-50 ppm—, **OPCIONAL y la ULTIMA DE LA COLA** desde que el responsable la
degrado el 07/09. `Y1` **no lleva la hora: es el LATIDO**, y mejoraria todo lo que aqui cuelga de
`millis()`. **Precondicion medible y sin cumplir: nunca se ha arrancado**, y nada de esta spec supone que
exista. Y **el unico que contesta a `SET_RTC` es el
ESP32** (`D-15`): el puente se queda la orden del telefono y **ya no la pasa al STM32**; lo que el STM32
recibe es la linea de siembra, que compone el propio ESP32.

## 2. La siembra: quien siembra a quien, y cada cuanto — **la cadena, en este orden**

```
telefono --SET_RTC--> ESP32 (DS3231)          [el unico que acusa]
ESP32 --CMD:HORA_ESP32--> su STM32            [por el cable J17, no por radio]
STM32 Maestro --CMD_HORA_D/H/M/S--> STM32 Esclavo   [por radio, en cada siembra]
```

- **ESP32 → su STM32.** Lo hace `siembra_revisar()` (`ESP32_Expansion/src/siembra.cpp`) con la cadencia
  **`SIEMBRA_INTERVALO_MS`** (`contrato.h`), **que es la fuente y esta fijada AL SEGUNDO** —`D-28` (1)
  la cerro asi a proposito, para que nadie derive su propia cifra de un «~»—. El STM32 la espera con el
  nombre **`HORA_ESP32_CADENCIA_MS`** (`reloj.h` de las dos puntas), y **`esp32_13` exige que los dos
  binarios digan lo mismo.** Al arrancar hay ademas dos reintentos —`SIEMBRA_REINTENTO_1_MS` y
  `SIEMBRA_REINTENTO_2_MS`— porque el STM32 tarda en abrir su puerto de `J17` mas de lo que el ESP32
  tarda en llegar a su primer `loop()`: **la primera siembra pilla el puerto cerrado casi seguro.**
- **La barrera del CONTENIDO:** la linea sale **solo** de `reloj_leer()`, que relee el chip en cada
  llamada y **no tiene variante «damela igual»**. Un `DS3231` sin pila da una fecha perfectamente
  formada y falsa; ahi no llega, porque `reloj_leer()` dice que no **y entonces no sale nada** — ni un
  hueco, ni un cero, ni la ultima que se vio.
- **La barrera de la FORMA:** el STM32 solo lee la linea si tiene **exactamente** el patron que el ESP32
  compone (`isoBienFormado()`, `PATRON_ISO`). No hay checksum en ese cable: una linea truncada y pegada
  a la siguiente se rechaza por el patron, no por suerte. **Y el STM32 SOBREESCRIBE, no acumula**
  (`D-20`): repetir una siembra no cuesta nada y el orden en que el operario haga las cosas da igual.
- **Maestro → Esclavo por radio.** La rama `CMD:HORA_ESP32:` del Maestro llama a
  `coordinador_sincronizarHora()` **dentro del `if`** de la siembra, asi que la hora sale al Esclavo **en
  cada siembra buena**. El reenvio periodico del coordinador existe aparte (`INTERVALO_SYNC_MS`) y **no
  es el mismo numero ni el mismo mecanismo.**
- **La siembra ya NO escribe el RTC hardware del STM32** (`N-162`): bloqueaba segundos por siembra con
  el RTC parado, no tenia lector, y **rejuvenecia la marca de sync** que fecha el limite duro.

## 3. Cuando cae la radio

`D-26` (3) manda, y **deroga** la regla anterior de que el Esclavo solo hiciera caso al Maestro:

| | de quien acepta la hora |
|---|---|
| **Maestro** | de **su** ESP32, **siempre**. No tiene relevo: no lo siembra ninguna radio |
| **Esclavo, CON radio** | del **Maestro**. La de su propio ESP32 llega, se **ignora** y se anota en el diario |
| **Esclavo, SIN radio** | de **su propio ESP32** — su `DS3231`, y la que el usuario le ponga con el telefono en el gabinete |

- **«Sin radio» tiene UNA sola definicion en todo el equipo:** `SFTY6_SILENCIO_MS` (`protocolo.h`), el
  mismo silencio con el que se declara la orfandad y con el que sale `$ALARM FALLO_RF`. Lo pregunta
  `reloj_radioManda()`, que es **lectura pura**. **Importa que sea el mismo umbral:** esa alarma es la
  que manda al tecnico a este poste a ponerle la hora, y esa hora tiene que entrar.
- **La autoridad esta ordenada, no adivinada:** `FuenteHora` (`FH_NINGUNA < FH_RTC_HW < FH_ESP32 <
  FH_RADIO`) se marca **solo dentro del `if`** de la funcion que puso la hora, junto a `horaValida`. Una
  fuente que dijera RADIO sobre una hora que no entro seria otra vez el acuse que no depende de la
  llamada (`N-160`, `CLAUDE.md` §2).
- **Lo que la hora del propio ESP32 NO renueva:** ni `degradado_registrarSync()` ni
  `respaldo_marcarSync()`. El limite duro cuenta desde la ultima hora **del Maestro**; una del `DS3231`
  local no demuestra que las dos puntas sigan en fase.
- **El salto que deja el relevo esta acotado y aceptado:** lo que difieran los dos `DS3231` mas la deriva
  del HSI desde la ultima siembra. Lo tolera la regla del §5 —el salto pasa por rojo— y la alarma de radio.

## 4. El plazo de caducidad de la hora

> **Una hora que no es fiable NO es «sin hora»: es una hora que MIENTE** (`D-21`). Entre siembras la
> hora se extrapola sobre el HSI; pasado cierto plazo deja de poder decidir una luz.

**El plazo NO se escoge: se DERIVA, y la derivacion vive en `reloj.h`, identica en las dos puntas.**
Son **cuatro `static_assert` encadenados** y **`reloj_04` los recalcula del C++ en cada corrida**:

1. **El caso que MANDA (`D-28` (2)): DOS SIEMBRAS PERDIDAS SEGUIDAS** → `HORA_DOS_PERDIDAS_MS`. Dos
   perdidas no son dos cadencias sino **tres**: se pierde la 1 y la 2, y la que trae hora es la 3.
2. **Suelo conservado — una siembra NORMAL** debe llegar antes de caducar aun con el HSI en su extremo
   rapido, o el Degradado caeria a ambar con el `J17` sano en cada cadencia.
3. **Suelo conservado — el relevo de fuente del Esclavo** (`HORA_RELEVO_MS`): dos cadencias mas
   `SFTY6_SILENCIO_MS`. **Ya no manda**, y se queda porque es el unico que vigila el silencio de radio.
4. **Techo:** que el plazo sea el **MENOR** que cubre el caso peor. Sin el, alguien podria subirlo a
   mano para comprar holgura y el margen contra los dos `DS3231` menguaria sin decision de nadie.

De ahi salen `HORA_DERIVA_S` —la deriva concedida, cuantizada a segundos enteros— y **`HORA_CADUCA_MS`**.
**El plazo es un CONTRATO DEL CRUCE, no de una punta:** el Maestro lleva tambien el termino del relevo,
que es del Esclavo, porque con dos plazos distintos una punta se rendiria a ambar mientras la otra sigue
dando verdes con una hora de la misma edad. `reloj_04` exige que sean iguales. **Lo que cuesta, aceptado
por el responsable (`D-28`):** cada segundo de deriva concedido **recorta dos** del margen que le queda
a la discrepancia entre los dos `DS3231` — sigue siendo positivo, `reloj_04` lo recalcula en cada
corrida, **y el dia que deje de serlo el pack lo dice.**

**Quien pregunta que:**

| pregunta | funcion | quien la lee |
|---|---|---|
| ¿hay hora? | `reloj_enHora()` | la sincronizacion por radio, la medida de desfase, la telemetria — tienen que seguir funcionando con una hora vieja |
| **¿puede esta hora decidir una luz?** | **`reloj_horaFiable()`** | **solo la puerta y el bucle del Degradado** |

**El cerrojo:** una vez caducada, la hora **se queda** caducada (`siembraCaducada`) hasta la siguiente
siembra buena. Lo mantiene `reloj_actualizar()` en cada vuelta del `loop()`, **en todos los modos**,
para que la vuelta de `millis()` no rejuvenezca una siembra vieja.

**El BORDE, escrito** (`CLAUDE.md` §7): una hora que vino **solo del RTC de hardware**, sin ninguna
siembra en este arranque, **no caduca aqui** — esa no corre sobre el HSI, y la cubre el limite duro del
Degradado. **Ese borde es donde vive el defecto vivo H-1.**

## 5. El ambar de la punta cuya hora caduco

`D-21` (1): **la punta que pierde la hora fiable pasa a ambar intermitente. Cada punta decide por su
cuenta** — en Degradado no hay radio, asi que no se puede ordenar «ambar en las dos». Que las dos
coincidan solo pasa si las dos pierden la hora; la asimetria que queda es el `Riesgo 2` del Degradado,
aceptado desde el 01/08 y **sin solucion tecnica sin radio**.

**En la PUERTA de entrada** las dos puntas preguntan **`reloj_horaFiable()`**, no `reloj_enHora()`:
`MDG_FALTA_HORA` en el Maestro, `DEG_RECHAZO_SIN_HORA` en el Esclavo. Con la puerta mirando solo «¿hay
hora?», un equipo con la siembra caducada entraba, el telefono recibia su `$ACK` y la primera vuelta del
bucle lo mandaba a ambar: **un «si» a una orden que no se iba a cumplir** (`CLAUDE.md` §2).

**En el BUCLE las dos puntas NO hacen la misma linea** —el Maestro llama `irAAmbar(...)` y va rojo y
directo; el Esclavo llama `iniciarSalida(true)`, se RINDE por todo-rojo y entra en `DEG_RENDIDO`—, **y es
lo correcto. El motivo entero esta en SPEC 2 §7** y no se repite aqui.

**No se vuelve solo.** De ese ambar se sale **por una orden del operario**: una siembra fresca no
devuelve el modo (`D-21`).

**El salto de hora pasa por ROJO** (`D-26` (4)). Cada siembra mueve la fase de golpe lo que el HSI
derivo. Un salto mayor que **`SALTO_SIN_ROJO_MAX_S`** —**derivado del despeje, no escogido**: es el rojo
que separa los dos verdes, menos un segundo por el truncado— devuelve a la entrada en rojo **en la misma
vuelta**, antes de decidir ninguna luz, y publica `$EVENT DEGRADADO:SALTO_DE_HORA_POR_ROJO`. Un salto
menor se aplica directo: es la correccion normal de la deriva y el despeje la absorbe. **Que una siembra
NORMAL quede por debajo del umbral lo vigila un `static_assert`** — sin el, el cruce pararia en cada
cadencia.

## 6. Reanudacion tras un corte de energia

**El problema, medido el 12/09 (`D-29`):** desde que la siembra no escribe el RTC hardware,
`reloj_setup()` deja la hora invalida **tras cada corte**. La reanudacion cerraba por su primera puerta
**y en ese mismo arranque borraba el indicador de la pila**, de modo que la hora del ESP32 —que llega
por `J17` un segundo despues— **ya no tenia nada que reanudar**. No se decidio: fue un efecto colateral
de `D-20`/`D-26` que nadie vio. **La reconstruccion difiere el BORRADO del permiso, y nada mas:**

- `modo_degradado_reanudarTrasCorte()` la llama `setup()` **y, mientras la decision siga pendiente, el
  bucle** (`reanudacionPorDecidir`).
- **La ventana es `VENTANA_REANUDACION_MS`, y se DERIVA del simbolo, no se copia:** es exactamente
  `HORA_ESP32_ESPERA_MAX_MS`, el mismo instante en el que el firmware **da por muda la siembra y publica
  su alarma**. El permiso se conserva mientras el propio equipo cree que la siembra puede llegar; cuando
  se tira, el tecnico ya tiene la alarma que dice por que.
- **Se difiere SOLO por lo que la siembra puede arreglar:** falta la hora, **y** el ciclo acordado sigue
  en la pila, **y** el limite duro sigue abierto, **y** la ventana no ha vencido. Con cualquier otra cosa
  cerrada se borra hoy igual que antes.
- **La ventana se cierra antes si una persona elige un modo:** el Maestro mira `modoActual_get()`; el
  Esclavo mira que siga inactivo **y ademas consulta `mando_ambarLocal()`** (condicion anadida por el
  responsable el 12/09). **En el Maestro no hay llamada gemela y no es una omision:** esa funcion no
  existe en esa punta —el Maestro resuelve el ambar del mando con un **cambio de modo**, que la guarda
  de `modoActual_get()` ya ve; el Esclavo levanta un **cerrojo**.
- **Lo que NO se toca:** el **limite duro** (`LIMITE_DURO_MS` / `LIMITE_SIN_SYNC_MS`) sigue mandando —es
  la puerta que impide reanudar sobre una marca que ya no significa nada—, y **sigue sin haber entrada
  automatica al Degradado** (`SFTY-21`, activacion manual): esto **reanuda** un modo que ya estaba puesto.

## 7. Las alarmas de reloj — son DOS, porque son dos averias distintas

| alarma | que dice | a donde manda |
|---|---|---|
| **`$ALARM ... FALLO_RF`** (`main.cpp`, umbral `SFTY6_SILENCIO_MS`) | la radio se cayo | **ir al Esclavo y ponerle la hora con el telefono.** Desde `D-26` (3) esa hora entra |
| **`$ALARM ... EVENTO:HORA_ESP32`** (`bluetooth.cpp`, umbral `HORA_ESP32_ESPERA_MAX_MS`) | la hora del ESP32 no llega, o llega y no sirve | **revisar el circuito de la MISMA placa:** el ESP32, su `DS3231`, el cable `J17` |

**`HORA_ESP32` trae TRES causas, cada una con su literal en su propia rama** (`N-89`):
**`RECHAZADA_FORMATO`** —llego una linea que el sembrador tiro, y se dice en el acto—;
**`J17_MUDO`** —no llega **nada** por el cable, **ni el latido**—; y **`SIN_HORA_DEL_ESP32`** —el latido
si llega y la hora no: el `DS3231` no tiene hora fiable, o el ESP32 lleva un firmware sin siembra—.
**Las dos ultimas se distinguen con el registro de silencio de `J17`**: sin esa distincion mandarian a
mirar lo mismo, y son dos arreglos distintos.

**`ACCION` dice lo que el equipo HACE, no lo que deberia:** `SIGUE_SU_HORA`. Y **se repite** mientras
dure, porque quien la tiene que ver es el tecnico que se conecte **despues**.

**Y una tercera, consecuencia de la segunda:** al caducar la hora dentro del Degradado se publica
`HORA_ESP32 / CADUCADA / CAMBIO_A_AMBAR`. **Va detras de `reloj_enHora()` a proposito:** con la hora
borrada a mano por `REINICIAR_RELOJ`, quien la borro ya tiene su acuse, y decirle CADUCADA seria
mandarle a mirar el `J17` por algo que hizo el.

**El diario** (`$EVENT`) anota **solo el CAMBIO** de quien manda —la primera ignorada, la primera
sembrada, tras el arranque o tras una alarma—: una linea por siembra serian cientos al dia, y eso es
perdida silenciosa por inundacion (`N-73`). **Cada linea sale de lo que la llamada devolvio**, nunca de
un `true` fijo (`CLAUDE.md` §2).

## 8. Las decisiones que gobiernan, y cual deroga a cual

| decision | que aporta a esta spec | que deroga |
|---|---|---|
| **`D-20`** (07/09) | la autoridad es el ESP32, siempre y para todo; el Esclavo **sobrescribe** en vez de prohibir | «la hora la lleva el RTC del STM32 y viaja por radio»; y la propuesta de prohibir `SET_RTC` al Esclavo |
| **`D-26`** (11/09) | **desarrolla `D-20`**: la cadencia gana **nombre propio** (`SIEMBRA_INTERVALO_MS`, ya no reusa el del reenvio por radio), relevo de fuente del Esclavo, salto por rojo, dos alarmas | **el numero de `A-15`**, que se eligio por tener uno solo en el sistema y no por medida; y la regla intermedia «el Esclavo acepta su ESP32 si la radio lleva horas sin sembrar» |
| **`D-21`** (07/09) | la hora que miente se responde con **ambar intermitente en la punta que la tiene**, y se publica | «sin hora = no entra en Degradado» a secas, que no dice que hacer si la hora **ya estaba dentro** |
| **`D-28`** (12/09) | **la mas nueva, y manda sobre las otras cuatro en esto:** (1) la cadencia queda fijada **al segundo** en `SIEMBRA_INTERVALO_MS` —el responsable la cerro para que nadie derivase su propia cifra—; (2) el plazo pasa a derivarse de **dos siembras perdidas** | de `D-26` (2), el «~» que dejaba la cadencia aproximada; **de `D-21` (1), que el plazo se derive del relevo** |
| **`D-29`** (12/09) | el indicador de la pila no se borra hasta que la primera siembra del arranque haya podido llegar | que `N-20` hubiera muerto en el Esclavo — que era el estado de hecho, sin que nadie lo decidiera |

**No hay contradiccion viva entre las cinco.** Las que parecen chocar se resuelven por fecha, y lo dicen
en su propia fila: `D-26` desarrolla `D-20` y corrige el numero de `A-15`; `D-28` corrige el «~» de
`D-26` y cambia de sujeto la derivacion de `D-21`; y `D-21` lleva **su propia premisa tumbada**
—«tendrian que pasar MESES»— marcada como refutada al revisar `D-26`: con el `J17` mudo son minutos.

## 9. HUECOS MEDIDOS — lo que no cumple una decision vigente, o lo que una vigente no alcanza

### H-1 🔴 El cristal tiene TRES estados y el firmware distingue dos — **defecto VIVO** (`roadmap.md` 1.22)

`arrancarCristal()` da por bueno el cristal en cuanto sube **`LSERDY`**, y `reloj_setup()` pone
`rtcOperativo` justo despues. **Pero `LSERDY` dice que el oscilador ARRANCO, no que `CNT` INCREMENTE**,
y el tercer estado —`LSERDY` arriba con `CNT` quieto— **es el de la cinta de campo del Sisga**.

Reproducido en el fuente, y cada eslabon es una linea:

1. `reloj_contadorSegundos()` devuelve `CNT` en crudo... **y el centinela que deberia taparlo lo
   destapa**: `return v == 0 ? 1UL : v;` convierte un contador **parado en 0** en un **`1` no nulo**,
   que pasa los dos centinelas de `respaldo.cpp`.
2. `respaldo_horasDesdeSync()` resta dos lecturas **iguales** y devuelve **cero horas** — o sea «acabo
   de hablar con el otro poste», sobre un acuerdo que puede ser de meses.
3. Tras un reset no hay medida en RAM, asi que `msDesdeSyncEfectivo()` se queda con la de la pila y
   devuelve **cero**.
4. 🔴 **Y la puerta que se abre NO es la del limite duro: es la de `SYNC_FRESCA_MS`**, la frescura que
   exige la entrada del Maestro (`modo_degradado_evaluarEntrada()`, condicion 2). **Es mucho mas
   estrecha** —la razon se lee dividiendo las dos constantes de `Maestro/src/modo_degradado.cpp`—.

**Ningun instrumento puede cazarlo hoy, y no es un olvido:** el modelo de silicio del arnes
(`Validacion_Automatico/dos_puntas/reloj_real/stm32f1xx_hal.h`) **deriva `CNT` de `millis()`**, asi que
el contador congelado **no es un escenario que falte: es un estado que no se puede expresar**. El arnes
lleva su borde escrito y bien —declara «cristal vivo» y «Y2 muerto»— y el defecto vive en el tercero. La
perilla de congelacion que hay que anadirle **es a la vez el control negativo** que se exige antes de
conectarlo. **El arreglo barato no toca `respaldo.cpp`:** muestrear `CNT` en
`{Maestro,Esclavo}/src/reloj.cpp` y **bajar `rtcOperativo` si no cambia**. ⚠️ **Las otras formas medidas
chocan:** exigir RAM en la puerta **contradice `D-29` de frente**, que existe justo para apoyarse en la
marca de la pila. **Eso no es una orden: es una fila nueva del responsable.**

### H-2 🔴 La pieza (A) de `D-21` sigue sin construir: el `OSF` no llega al STM32

`D-21` cierra con tres piezas; la (A) es que el bit **`OSF`** del `DS3231` —el que dice que el oscilador
se paro— llegue a la decision del STM32. **Medido hoy:** `grep OSF` sobre
`{Maestro,Esclavo}/{src,include}`, filtrando `MOSFET`, da **dos lineas, las dos comentario** de
`Maestro/include/reloj.h`, que dicen exactamente que falta. La deteccion **existe y es correcta en el
ESP32** —el `OSF` se limpia solo tras releer y cuadrar la hora— pero **el STM32 no la ve**. Lo que hoy
tapa el hueco por otro camino es la **caducidad**: un `DS3231` sin hora fiable **no siembra**, y a las
tres cadencias la punta se declara caducada. **Es una red distinta, no la pieza (A).**

### H-3 `D-29` no alcanza a las tarjetas con `Y2` muerto — **y es correcto que no alcance**

Con el cristal muerto, `reloj_setup()` deja `rtcOperativo` en falso, `reloj_contadorSegundos()` devuelve
**cero a proposito** y `respaldo_horasDesdeSync()` responde **CADUCADA**: **cierra la SEGUNDA puerta y
el diferimiento ni llega a activarse.** `D-29` alcanza solo a las tarjetas cuyo `Y2` oscila; extenderlo
seria reanudar sobre una marca caducada, justo lo que esa puerta impide. **Se escribe porque se publico
una vez como «cierra la divergencia» y era falso:** la divergencia de las dos flotas **se estrecha, no
se cierra** — una tarjeta cuyo RTC escribio un firmware anterior al 11/09 reanuda y una recien grabada
no, **con el mismo binario y sin que nada en el `$STATUS` lo distinga**.

### H-4 `horaValida` es un trinquete de una sola direccion

**Medido:** los unicos `horaValida = false` viven en `reloj_setup()` de las dos puntas, mas
`reloj_reiniciarDominioRespaldo()` **solo en el Maestro** —esa funcion no existe en el Esclavo—. No hay
`reloj_invalidarHora()`: se retiro con el camino de escritura viejo y **no se restauro al abrirse el
nuevo** (`N-160`). **La guarda del Degradado no cuelga de esa bandera sino de `reloj_horaFiable()`**, con
su propio cerrojo; cerrarlo bien es la pieza (A) de H-2.

### H-5 Comentarios de cadencia caducados — el numero manda, el comentario no

**Seis lineas de comentario** recitan la cadencia **anterior a `D-26` (2)** en vez de nombrar la
constante, despues de que `D-28` (1) la fijara al segundo: `ESP32_Expansion/src/main.cpp` y
`siembra.cpp` (bloque `(3)`), `Maestro/src/modo_degradado.cpp` (dos, una de ellas con **dos** cadencias
que ya no existen) y `Esclavo/src/modo_degradado.cpp` (dos). **Las constantes son correctas y los packs
las releen**: no mueve ninguna luz, pero es la cifra que envejece en silencio con autoridad de dato
(`CLAUDE.md` §14). *(Las de `contrato.h` NO entran: narran por que se bajo, y eso sigue siendo cierto.)*

### H-6 Nada de esto ha visto una tarjeta

`D-26` y el plazo de `D-28` estan **fusionados en `main` y SIN BANCO**: lo que hay son packs y arneses de
PC, y **un verde de la compuerta no dice que el firmware funcione en la tarjeta** (`CLAUDE.md` §0.3). La
unica medida tomada sobre el aparato real —la cinta del Sisga— es justamente la que trae **H-1**.
