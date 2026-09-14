# SPEC 3 — LA HORA: de donde sale, y que pasa cuando se pierde

**Que autoriza esta spec.** El **Modo Degradado** da verdes **sin confirmar con la otra punta**: la fase
la calcula cada poste por su cuenta, a partir de su hora de pared. Si la hora miente, **los dos postes
pueden dar verde a la vez.** Todo lo que sigue existe por esa frase.

**Alcance.** De donde sale la hora, quien siembra a quien y cada cuanto, que pasa al perder la radio, cuando
la hora CADUCA, el ambar de la punta que la perdio, la reanudacion tras un corte de energia y las alarmas de
reloj. **Fuera:** el ciclo (SPEC 1) · la radio en si (SPEC 2) · la app (SPEC 4) · el cobre (SPEC 5).

> 🔴 **NINGUNA CIFRA VIVE EN ESTE FICHERO.** Se nombra la constante y su consecuencia en palabras.
> Todo plazo y toda cadencia salen del fichero de reloj de cada punta (`{Maestro,Esclavo}/include/reloj.h`)
> y del contrato del puente (`ESP32_Expansion/include/contrato.h`); dos pruebas de banco los recalculan
> del C++ en cada corrida. Un numero copiado aqui naceria caducado.

## 1. Quien tiene el reloj

**Hay DOS relojes por cruce, y ninguno esta en el micro que gobierna las luces.**

| | |
|---|---|
| **ESP32 de cada poste** | lleva un **reloj de calendario con su propia pila** (`DS3231`). Es **la autoridad de la hora, siempre y para todo** (`D-20`, que desarrolla `D-9`) |
| **STM32 de cada poste** | **no tiene reloj que sirva**: el cristal de reloj de la placa esta confirmado muerto en banco y su calendario queda anclado a enero por construccion (`Y2`, `N-17`) |

**Al STM32 no se le pregunta la hora nunca.** Lo que tiene es una **base de tiempo de software** —un
segundo del dia, mas los milisegundos transcurridos desde que se apunto— que le **siembra** su propio
ESP32. Esa cuenta de milisegundos corre sobre el **oscilador interno del micro**, cuyo peor caso de ficha
esta escrito como constante: **es el que decide cuanto se separan las dos puntas entre siembras, y es la
razon de todo el resto de esta spec.**

🟡 **Hay una mejora decidida y todavia sin hacer**, que ninguna spec nombraba: montar como reloj del sistema
el **cristal de 8 MHz que ya esta soldado en la placa y sin usar** bajaria el error de 10.000-25.000 ppm a
20-50 ppm. Ese cristal **no lleva la hora: es el LATIDO**, y mejoraria todo lo que en esta spec cuelga de la
cuenta de milisegundos. Es **OPCIONAL y la ULTIMA DE LA COLA** desde que el responsable la degrado el 07/09,
**nunca se ha arrancado** —precondicion medible y sin cumplir— y nada de esta spec supone que exista
(`Y1`, `D-22`).

**La orden de poner la hora la contesta el ESP32 y nadie mas** (`SET_RTC`, `D-15`): el puente se queda la
orden del telefono y **ya no la pasa al STM32**. Lo que el STM32 recibe es la linea de siembra, que compone
el propio ESP32.

## 2. La siembra: quien siembra a quien, y cada cuanto — **la cadena, en este orden**

```
telefono --SET_RTC--> ESP32 (DS3231)          [el unico que acusa]
ESP32 --CMD:HORA_ESP32--> su STM32            [por el cable J17, no por radio]
STM32 Maestro --CMD_HORA_D/H/M/S--> STM32 Esclavo   [por radio, en cada siembra]
```

- **ESP32 → su STM32.** El puente resiembra con una cadencia fija que es **la fuente del numero** y que
  el responsable cerro **AL SEGUNDO**, a proposito, para que nadie derive su propia cifra de un «~»
  (`SIEMBRA_INTERVALO_MS` en `contrato.h`; `D-28` (1)). El STM32 espera esa misma cadencia con otro
  nombre (`HORA_ESP32_CADENCIA_MS`), y **una prueba de banco exige que los dos binarios digan lo mismo.**
  Al arrancar hay ademas **dos reintentos**, porque el STM32 tarda en abrir su puerto del cable de
  siembra mas de lo que el ESP32 tarda en llegar a su primera vuelta: **la primera siembra pilla el
  puerto cerrado casi seguro.**
- **La barrera del CONTENIDO:** la linea sale **solo** de la lectura del chip, que **relee en cada llamada y
  no tiene variante «damela igual»**. Un reloj de calendario sin pila da una fecha perfectamente formada y
  falsa; ahi no llega, porque el lector dice que no **y entonces no sale nada** — ni un hueco, ni un cero,
  ni la ultima que se vio.
- **La barrera de la FORMA:** el STM32 solo lee la linea si tiene **exactamente** el patron que el ESP32
  compone. No hay checksum en ese cable: una linea truncada y pegada a la siguiente se rechaza por el patron,
  no por suerte. **Y el STM32 SOBREESCRIBE, no acumula**: repetir una siembra no cuesta nada y el orden en
  que el operario haga las cosas da igual.
- **Maestro → Esclavo por radio.** Al recibir la siembra, el Maestro sincroniza a la otra punta **dentro del
  `if`** que comprueba que la siembra fue buena, asi que la hora sale al Esclavo **en cada siembra buena**.
  El reenvio periodico del coordinador existe aparte y **no es el mismo numero ni el mismo mecanismo.**
- **La siembra ya NO escribe el reloj de hardware del STM32** (`N-162`): bloqueaba segundos por siembra
  con ese reloj parado, no tenia lector, y **rejuvenecia la marca de sync** que fecha el limite duro.

## 3. Cuando cae la radio

**El Esclavo ya no hace caso solo al Maestro**, y la regla anterior queda derogada (`D-26` (3)):

| | de quien acepta la hora |
|---|---|
| **Maestro** | de **su** ESP32, **siempre**. No tiene relevo: no lo siembra ninguna radio |
| **Esclavo, CON radio** | del **Maestro**. La de su propio ESP32 llega, se **ignora** y se anota en el diario |
| **Esclavo, SIN radio** | de **su propio ESP32** — su reloj de calendario, y la que el usuario le ponga con el telefono en el gabinete |

- **«Sin radio» tiene UNA sola definicion en todo el equipo:** el mismo silencio con el que se declara la
  orfandad y con el que sale la alarma de radio (`SFTY6_SILENCIO_MS`). La funcion que lo pregunta es
  **lectura pura**. **Importa que sea el mismo umbral:** esa alarma es la que manda al tecnico a este
  poste a ponerle la hora, y esa hora tiene que entrar.
- **La autoridad esta ordenada, no adivinada:** hay un escalafon de fuentes —ninguna, reloj de hardware,
  ESP32, radio— que se marca **solo dentro del `if`** de la funcion que puso la hora, junto a la bandera de
  hora valida. Una fuente que dijera RADIO sobre una hora que no entro seria otra vez el acuse que no depende
  de la llamada (`CLAUDE.md` §2).
- **Lo que la hora del propio ESP32 NO renueva:** ninguna de las dos marcas de sincronizacion. El limite duro
  cuenta desde la ultima hora **del Maestro**; una del reloj local no demuestra que sigan en fase.
- **El salto que deja el relevo esta acotado y aceptado:** lo que difieran los dos relojes de calendario, mas
  la deriva del oscilador interno desde la ultima siembra. Lo tolera la regla del §5 —el salto pasa por
  rojo— y la alarma de radio.

## 4. El plazo de caducidad de la hora

> **Una hora que no es fiable NO es «sin hora»: es una hora que MIENTE** (`D-21`). Entre siembras la
> hora se extrapola sobre el oscilador interno; pasado cierto plazo deja de poder decidir una luz.

**El plazo NO se escoge: se DERIVA, y la derivacion vive en el fichero de reloj, identica en las dos puntas.** Son
**cuatro condiciones encadenadas que el compilador comprueba**, y **el banco las recalcula del C++ en cada corrida**:

1. **El caso que MANDA: DOS SIEMBRAS PERDIDAS SEGUIDAS** (`D-28` (2)). Dos perdidas no son dos cadencias
   sino **tres**: se pierde la 1 y la 2, y la que trae hora es la 3.
2. **Suelo conservado — una siembra NORMAL** debe llegar antes de caducar aun con el oscilador en su
   extremo rapido, o el Degradado caeria a ambar con el cable de siembra sano en cada cadencia.
3. **Suelo conservado — el relevo de fuente del Esclavo**: dos cadencias mas el silencio de radio.
   **Ya no manda**, y se queda porque es el unico que vigila ese silencio.
4. **Techo:** que el plazo sea el **MENOR** que cubre el caso peor. Sin el, alguien podria subirlo a mano
   para comprar holgura y el margen contra los dos relojes menguaria sin decision de nadie.

De ahi salen la deriva concedida —cuantizada a segundos enteros— y **el plazo de caducidad**
(`HORA_DERIVA_S`, `HORA_CADUCA_MS`). **El plazo es un CONTRATO DEL CRUCE, no de una punta:** el Maestro
lleva tambien el termino del relevo, que es del Esclavo, porque con dos plazos distintos una punta se
rendiria a ambar mientras la otra sigue dando verdes con una hora de la misma edad; el banco exige que
sean iguales. **Lo que cuesta, aceptado por el responsable:** cada segundo de deriva concedido **recorta
dos** del margen que le queda a la discrepancia entre los dos relojes — sigue siendo positivo, el banco lo
recalcula en cada corrida, **y el dia que deje de serlo lo dice.**

**Quien pregunta que:**

| pregunta | funcion | quien la lee |
|---|---|---|
| ¿hay hora? | `reloj_enHora()` | la sincronizacion por radio, la medida de desfase, la telemetria — tienen que seguir funcionando con una hora vieja |
| **¿puede esta hora decidir una luz?** | **`reloj_horaFiable()`** | **solo la puerta y el bucle del Degradado** |

**El cerrojo:** una vez caducada, la hora **se queda** caducada hasta la siguiente siembra buena. Se
sostiene en cada vuelta del bucle, **en todos los modos**, para que la vuelta del contador de milisegundos
no rejuvenezca una siembra vieja.

**El BORDE, escrito** (`CLAUDE.md` §7): una hora que vino **solo del reloj de hardware**, sin ninguna siembra
en este arranque, **no caduca aqui** — esa no corre sobre el oscilador interno, y la cubre el limite duro del
Degradado. **Ese borde es donde vive el defecto vivo H-1.**

## 5. El ambar de la punta cuya hora caduco

**La punta que pierde la hora fiable pasa a ambar intermitente, y cada punta decide por su cuenta**
(`D-21` (1)) — en Degradado no hay radio, asi que no se puede ordenar «ambar en las dos». Que las dos
coincidan solo pasa si las dos pierden la hora; la asimetria que queda es el `Riesgo 2` del Degradado,
aceptado desde el 01/08 y **sin solucion tecnica sin radio**.

**En la PUERTA de entrada** las dos puntas preguntan si la hora **puede decidir una luz**, no si la hay
(`reloj_horaFiable()` y no `reloj_enHora()`), y cada una tiene su motivo de rechazo. Con la puerta mirando
solo «¿hay hora?», un equipo con la siembra caducada entraba, el telefono recibia su acuse y la primera
vuelta del bucle lo mandaba a ambar: **un «si» a una orden que no se iba a cumplir** (`CLAUDE.md` §2).

**En el BUCLE las dos puntas NO hacen la misma linea** —el Maestro va a ambar directo y en rojo; el Esclavo
se RINDE por todo-rojo y queda en su estado de rendido—, **y es lo correcto: el motivo entero esta en
SPEC 2 §7.**

**No se vuelve solo.** De ese ambar se sale **por una orden del operario**: una siembra fresca no devuelve el modo.

**El salto de hora pasa por ROJO** (`D-26` (4)). Cada siembra mueve la fase de golpe lo que el oscilador
derivo. El umbral **no se escogio: se deriva del despeje** —es el rojo que separa los dos verdes, menos un
segundo por el truncado (`SALTO_SIN_ROJO_MAX_S`)—. Un salto mayor devuelve a la entrada en rojo **en la
misma vuelta**, antes de decidir ninguna luz, y lo publica en el diario. Un salto menor se aplica directo:
es la correccion normal de la deriva y el despeje la absorbe. **Que una siembra NORMAL quede por debajo
del umbral lo comprueba el compilador** — sin eso, el cruce pararia en cada cadencia.

## 6. Reanudacion tras un corte de energia

> ⚠️ **Y con que PRECISION se cuentan los plazos largos —las 48 h sin sincronizar— no se decide aqui: esta en `SPEC_7` §5.1.** Resumen de una linea: en las tarjetas de campo el contador de pila no oscila, queda el reloj de programa, y **ese tope puede cumplirse hasta ~1 h 10 min antes o ~1 h despues** de las 48 h de reloj de pared —y es un numero de FICHA del fabricante, no una medida—.

**El problema, medido el 12/09 (`D-29`):** desde que la siembra no escribe el reloj de hardware, el
arranque deja la hora invalida **tras cada corte**. La reanudacion cerraba por su primera puerta **y en
ese mismo arranque borraba el indicador de la pila**, de modo que la hora del ESP32 —que llega por el
cable un segundo despues— **ya no tenia nada que reanudar**. No se decidio: fue un efecto colateral de las
dos decisiones anteriores que nadie vio. **La reconstruccion difiere el BORRADO del permiso, y nada mas:**

- La comprobacion corre al arrancar **y, mientras la decision siga pendiente, tambien en el bucle.**
- **La ventana se DERIVA del simbolo, no se copia:** es exactamente el instante en el que el firmware
  **da por muda la siembra y publica su alarma** (`VENTANA_REANUDACION_MS` = `HORA_ESP32_ESPERA_MAX_MS`).
  El permiso se conserva mientras el propio equipo cree que la siembra puede llegar; cuando se tira, el
  tecnico ya tiene la alarma que dice por que.
- **Se difiere SOLO por lo que la siembra puede arreglar:** falta la hora, **y** el ciclo acordado sigue
  en la pila, **y** el limite duro sigue abierto, **y** la ventana no ha vencido. Con cualquier otra cosa
  cerrada se borra hoy igual que antes.
- **La ventana se cierra antes si una persona elige un modo:** el Maestro mira el modo activo; el Esclavo
  mira que siga inactivo **y ademas consulta su cerrojo de ambar local** (condicion anadida por el
  responsable el 12/09). **En el Maestro no hay llamada gemela y no es una omision:** esa funcion no
  existe en esa punta —el Maestro resuelve el ambar del mando con un **cambio de modo**, que la guarda
  del modo activo ya ve; el Esclavo levanta un **cerrojo**.
- **Lo que NO se toca:** el **limite duro** sigue mandando —es la puerta que impide reanudar sobre una
  marca que ya no significa nada—, y **sigue sin haber entrada automatica al Degradado**: la activacion
  es manual (`SFTY-21`) y esto **reanuda** un modo que ya estaba puesto.

## 7. Las alarmas de reloj — son DOS, porque son dos averias distintas

| alarma | que dice | a donde manda |
|---|---|---|
| **`FALLO_RF`** (umbral: el silencio de radio) | la radio se cayo | **ir al Esclavo y ponerle la hora con el telefono.** Sin radio esa hora ya entra (§3) |
| **`EVENTO:HORA_ESP32`** (umbral: la espera maxima de siembra) | la hora del ESP32 no llega, o llega y no sirve | **revisar el circuito de la MISMA placa:** el ESP32, su reloj de calendario, el cable `J17` |

**La alarma de la hora trae TRES causas, cada una con su literal en su propia rama** (`N-89`):
**`RECHAZADA_FORMATO`** —llego una linea que el sembrador tiro, y se dice en el acto—;
**`J17_MUDO`** —no llega **nada** por el cable, **ni el latido**—; y **`SIN_HORA_DEL_ESP32`** —el latido
si llega y la hora no: el reloj de calendario no tiene hora fiable, o el ESP32 lleva un firmware sin
siembra—. **Las dos ultimas se distinguen con el registro de silencio del cable**: sin esa distincion
mandarian a mirar lo mismo, y son dos arreglos distintos.

**El campo de accion dice lo que el equipo HACE, no lo que deberia:** sigue su hora. Y **se repite** mientras
dure, porque quien la tiene que ver es el tecnico que se conecte **despues**.

**Y una tercera, consecuencia de la segunda:** al caducar la hora dentro del Degradado se publica la misma
alarma con causa `CADUCADA` y accion de cambio a ambar. **Va detras de la pregunta «¿hay hora?» a proposito:**
con la hora borrada a mano, quien la borro ya tiene su acuse, y decirle CADUCADA seria mandarle a mirar el
cable por algo que hizo el.

**El diario** anota **solo el CAMBIO** de quien manda —la primera ignorada, la primera sembrada, tras el
arranque o tras una alarma—: una linea por siembra serian cientos al dia, y eso es perdida silenciosa por
inundacion (`N-73`). **Cada linea sale de lo que la llamada devolvio**, nunca de un `true` fijo.

## 8. Las decisiones que gobiernan, y cual deroga a cual

| decision | que aporta a esta spec | que deroga |
|---|---|---|
| **`D-20`** (07/09) | la autoridad es el ESP32, siempre y para todo; el Esclavo **sobrescribe** en vez de prohibir | «la hora la lleva el reloj del STM32 y viaja por radio»; y la propuesta de prohibir poner la hora al Esclavo |
| **`D-26`** (11/09) | **desarrolla la anterior**: la cadencia de siembra gana **nombre propio** —ya no reusa el del reenvio por radio—, relevo de fuente del Esclavo, salto por rojo, dos alarmas | **el numero de `A-15`**, que se eligio por tener uno solo en el sistema y no por medida; y la regla intermedia «el Esclavo acepta su ESP32 si la radio lleva horas sin sembrar» |
| **`D-21`** (07/09) | la hora que miente se responde con **ambar intermitente en la punta que la tiene**, y se publica | «sin hora = no entra en Degradado» a secas, que no dice que hacer si la hora **ya estaba dentro** |
| **`D-28`** (12/09) | **la mas nueva, y manda sobre las otras cuatro en esto:** (1) la cadencia queda fijada **al segundo** —el responsable la cerro para que nadie derivase su propia cifra—; (2) el plazo pasa a derivarse de **dos siembras perdidas** | de `D-26` (2), el «~» que dejaba la cadencia aproximada; **de `D-21` (1), que el plazo se derive del relevo** |
| **`D-29`** (12/09) | el indicador de la pila no se borra hasta que la primera siembra del arranque haya podido llegar | que `N-20` hubiera muerto en el Esclavo — que era el estado de hecho, sin que nadie lo decidiera |

**No hay contradiccion viva entre las cinco.** Las que parecen chocar se resuelven por fecha, y lo dicen
en su propia fila. Y la del ambar por hora que miente lleva **su propia premisa tumbada** —«tendrian que
pasar MESES»— marcada como refutada al revisarla en noviembre: con el cable de siembra mudo son minutos.

## 9. HUECOS MEDIDOS — lo que no cumple una decision vigente, o lo que una vigente no alcanza

### H-1 🔴 El cristal tiene TRES estados y el firmware distingue dos — **defecto VIVO** (`roadmap.md` 1.22)

El arranque da por bueno el cristal en cuanto el chip dice **que el oscilador arranco**, y marca el reloj
como operativo justo despues. **Pero eso dice que ARRANCO, no que el contador INCREMENTE**, y el tercer
estado —oscilador listo con el contador quieto— **es el de la cinta de campo del Sisga**.

Reproducido en el fuente, y cada eslabon es una linea:

1. El lector del contador devuelve el valor en crudo... **y el centinela que deberia taparlo lo destapa**:
   `return v == 0 ? 1UL : v;` convierte un contador **parado en 0** en un **`1` no nulo**, que pasa los dos
   centinelas del respaldo.
2. La cuenta de horas desde la ultima sincronizacion resta dos lecturas **iguales** y devuelve **cero
   horas** — o sea «acabo de hablar con el otro poste», sobre un acuerdo que puede ser de meses.
3. Tras un reset no hay medida en RAM: la medida efectiva se queda con la de la pila y devuelve **cero**.
4. 🔴 **Y la puerta que se abre NO es la del limite duro: es la de la FRESCURA** que exige la entrada del
   Maestro (`SYNC_FRESCA_MS`, condicion 2). **Es mucho mas estrecha** —la razon se lee dividiendo las dos
   constantes de `Maestro/src/modo_degradado.cpp`—.

**Ningun instrumento puede cazarlo hoy, y no es un olvido:** el modelo de silicio del arnes
(`Validacion_Automatico/dos_puntas/reloj_real/stm32f1xx_hal.h`) **deriva el contador del reloj de programa**,
asi que el contador congelado **no es un escenario que falte: es un estado que no se puede expresar**. El arnes
lleva su borde escrito y bien —declara «cristal vivo» y «cristal muerto»— y el defecto vive en el tercero. La
perilla de congelacion que hay que anadirle **es a la vez el control negativo** que se exige antes de conectarlo.
**El arreglo barato no toca el respaldo:** muestrear el contador en el reloj de cada punta y **bajar la bandera de
reloj operativo si no cambia**. ⚠️ **Las otras formas medidas chocan:** exigir RAM en la puerta **contradice
`D-29` de frente**, que existe justo para apoyarse en la marca de la pila. **Eso no es una orden: es una fila
nueva del responsable.**

### H-2 🔴 La pieza (A) del ambar por hora que miente sigue sin construir: el aviso de oscilador parado no llega al STM32

Esa decision cierra con tres piezas; la (A) es que el bit que el reloj de calendario levanta cuando **su
oscilador se paro** llegue a la decision del STM32 (`OSF`, `D-21`). **Medido hoy:** buscado sobre
`{Maestro,Esclavo}/{src,include}` y filtrando los transistores, da **dos lineas, las dos comentario** de
`Maestro/include/reloj.h`, que dicen exactamente que falta. La deteccion **existe y es correcta en el
ESP32** —el bit se limpia solo tras releer y cuadrar la hora— pero **el STM32 no la ve**. Lo que hoy tapa
el hueco por otro camino es la **caducidad**: un reloj de calendario sin hora fiable **no siembra**, y a
las tres cadencias la punta se declara caducada. **Es una red distinta, no la pieza (A).**

### H-3 La reanudacion tras corte no alcanza a las tarjetas con el cristal muerto — **y es correcto que no alcance**

Con el cristal muerto el arranque deja el reloj como no operativo, el contador devuelve **cero a
proposito** y la cuenta de horas responde **CADUCADA**: **cierra la SEGUNDA puerta y el diferimiento ni
llega a activarse.** Esa decision (`D-29`) alcanza solo a las tarjetas cuyo cristal oscila; extenderla
seria reanudar sobre una marca caducada, justo lo que esa puerta impide. **Se escribe porque se publico
una vez como «cierra la divergencia» y era falso:** la divergencia de las dos flotas **se estrecha, no se
cierra** — una tarjeta cuyo reloj escribio un firmware anterior al 11/09 reanuda y una recien grabada no,
**con el mismo binario y sin que nada en el `$STATUS` lo distinga**.

### H-4 La bandera de hora valida es un trinquete de una sola direccion

**Medido:** los unicos sitios que la bajan son el arranque de las dos puntas, mas el reinicio del dominio
de respaldo **solo en el Maestro** —esa funcion no existe en el Esclavo—. No hay funcion que invalide la
hora a proposito: se retiro con el camino de escritura viejo y **no se restauro al abrirse el nuevo**
(`reloj_invalidarHora()`, `N-160`). **La guarda del Degradado no cuelga de esa bandera** sino de la
pregunta «¿puede esta hora decidir una luz?», con su propio cerrojo; cerrarlo bien es la pieza (A) de H-2.

### H-5 Comentarios de cadencia caducados — el numero manda, el comentario no

**Cuatro lineas de comentario** —remedidas el 14/09; eran seis y dos se corrigieron ya— recitan la
cadencia **anterior** a que se le diera nombre propio, en vez de nombrar la constante, y despues de que
se fijara al segundo: `ESP32_Expansion/src/main.cpp` y `siembra.cpp` (bloque `(3)`),
`Maestro/src/modo_degradado.cpp` y `Esclavo/src/modo_degradado.cpp`, una en cada uno, todas glosando un
«~» que ya no vale. **Las constantes son correctas y los packs las releen**: no mueve ninguna luz, pero es
la cifra que envejece en silencio con autoridad de dato (`CLAUDE.md` §14). *(Las de `contrato.h` NO
entran: narran por que se bajo, y eso sigue siendo cierto.)*

### H-6 Nada de esto ha visto una tarjeta

El relevo de fuente y el plazo nuevo estan **fusionados en `main` y SIN BANCO**: lo que hay son packs y
arneses de PC, y **un verde de la compuerta no dice que el firmware funcione en la tarjeta**
(`CLAUDE.md` §0.3). La unica medida tomada sobre el aparato real —la cinta del Sisga— es justamente la
que trae **H-1**.
