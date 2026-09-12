# roadmap_hist.md — el historico del roadmap, integro y con su evidencia

**Creado el 07/09/2026 vaciando `roadmap.md` de todo lo que ya no se puede hacer.** Aqui esta
**todo lo cerrado**: no se resumio, no se reescribio y **no se borro una sola linea** — el cuerpo
salio del original por rangos de linea, y la cuenta esta publicada al final de este fichero.

> **Para que sirve, que es lo unico que justifica conservarlo.** Este repositorio arranca su
> historia de cero: `git blame` **no sabe** por que una linea del firmware es como es. Los `N-x`
> si. Cuando alguien pregunte *«¿por que este umbral son 25 s y no 12?»* o *«¿por que el mando
> sigue compilando si ya no existe?»*, la respuesta con su medida delante esta aqui.

**Lo que NO esta aqui, a proposito:**

| | donde vive |
|---|---|
| **lo pendiente y lo que falta validar** | [`roadmap.md`](roadmap.md) — se vacio para que solo lleve eso |
| **la decision vigente** | [`DECISIONES.md`](DECISIONES.md), que **manda** sobre cualquier parrafo de aqui |
| **las reglas permanentes** | [`CLAUDE.md`](CLAUDE.md) |
| **el hardware medido** | `05_Funcional/17_Arquitectura_28-08_y_Decisiones_Abiertas.md`, que gana a los dos |
| **`N-1` a `N-93` y V8.0-V8.9** | el `git log` de este repositorio y el remoto `padre` |

> ⚠️ **COMO SE LEE ESTO SIN QUE HAGA DANO.** Un parrafo de aqui describe **lo que se sabia el dia
> que se escribio**, no lo que es cierto hoy. Tres frases se han encontrado falsas y **viajan
> tachadas con su motivo, no borradas** (busca `TACHADO EN LA MUDANZA`) — pero es seguro que hay
> mas. **Antes de actuar sobre una frase de este fichero, se comprueba contra `DECISIONES.md` o
> contra la spec.** Ese es exactamente el error que costo una sesion el 05/09.

---
## Indice por `N-x` — el porque de una linea del firmware

**Levantado del propio cuerpo, no escrito a mano.** El numero de linea es de ESTE fichero;
si algo se inserta arriba caduca, asi que **la busqueda buena es por el simbolo `N-xxx`**
(`CLAUDE.md` §4.sexies). El firmware lleva esas marcas: `grep -rn "N-133" 01_Firmware/`.

| `N-x` | linea | de que va |
|---|---|---|
| **N-157** | 665 | `N-157` · fase 1 de `D-13`: la camara se vigila a si misma (`4b90f98`) |
| **N-156** | 652 | `N-156` · el `$ACK` del reloj, y el Degradado del Esclavo sin puerta (`5846cee`, `d020f3c`) |
| **N-155** | 630 | `N-155` · `app_11` no juzgaba NINGUNA linea (`2d17678`) |
| **N-154** | 600 | `N-154` · el `$STATUS` no cabia en su peor caso, y por BUFFER eran 169 B (`e43a8e7`) |
| **N-152** | 854 | N-152 · el Maestro ESTABA SORDO en `MODO_AMBAR` (`d6ce67e`) |
| **N-151** | 982 | N-151 — DAR PASO EN UN MODO SIN COORDINADOR TRABABA EL CRUCE PARA SIEMPRE |
| **N-150** | 881 | N-150 · el `$ACK` verde que no habia terminado nada (`414b962`) |
| **N-149** | 1345 | N-149 — el `$STATUS` del Maestro no traia NADA del Esclavo |
| **N-147** | 1296 | N-147 — Manual entraba por LA PUERTA DEL AUTOMATICO |
| **N-146** | 1271 | N-146 — seis `OK` seguidos y el cruce sin moverse: lo destapo una CINTA, no una lectura |
| **N-145** | 1493 | N-145 🔴 EL EQUIPO TIENE HORA Y PUBLICA QUE NO LA TIENE |
| **N-142** | 1232 | N-142 — el Esclavo AVISA por radio, y los dos vetos SE QUEDAN |
| **N-135** | 1159 | N-135 — un `enum` de un solo valor no es un estado, y cerro la puerta de N-133 |
| **N-133** | 1133 | N-133 — 🔴 LOS TIEMPOS DEL CICLO NO SE GUARDAN EN NINGUN SITIO |
| **N-127** | 2208 | 🟢 N-127 — AB-1 CONSTRUIDO: el latido del puente, y las dos piezas del STM32 que lo hacian imposible |
| **N-126** | 2265 | 🟢 N-126 — SESION 2 DE BANCO (04/09): dos defectos cerrados con evidencia fisica, y el VERDE por primera vez |
| **N-125** | 2350 | 🟢 N-125 — La app no pedia el permiso de Bluetooth. CERRADO EN CAMPO, y obliga a rebajar dos hallazgos |
| **N-123** | 2481 | 🔴 N-123 — La guia perdia 12.600 caracteres al imprimir, y eran los que hay que contestar |
| **N-122** | 2409 | 🔴 N-122 — La app NUNCA abria el socket: faltaba `connect()`, y eso bloqueaba el banco por si solo |
| **N-121** | 2536 | 🟠 N-121 — Censo de las cuatro salidas del Degradado del Esclavo: dos muertas, y son las que no dependen del ESP32 |
| **N-119** | 2642 | 🟠 N-119 — El ritmo de J17: la pregunta era buena y la respuesta es «ya es por eventos, salvo un latido» |
| **N-118** | 193 | N-118 se cierra, y era DOS cosas contadas como una |
| **N-118** | 2566 | 🔴 N-118 — El mando A/B no se puede pulsar: SFTY-21 se quedo sin respaldo fisico, y esta medido en las dos mitades |
| **N-115** | 2174 | 🟢 N-115 — El banco corrio, y lo primero que hay que decir es que NO invalido nada de lo escrito |
| **N-114** | 2675 | 🔴 N-114 — Segunda auditoria externa: "arreglamos todo lo que midio y nada de lo que dijo" |
| **N-112** | 2774 | 🔴 N-112 — La compuerta ALTERNA verde y rojo sobre un arbol identico: su codigo de salida no significa nada |
| **N-109** | 2821 | 🔴 N-109 — Auditoria externa: el proceso no puede verse a si mismo caido, y el banco dejo de ser un bloqueo para ser u... |
| **N-107** | 2968 | 🟢 N-107 — BLQ-1 cerrado: es un `ESP32-WROOM-32` clasico, hay SPP · **CERRADO 31/08** |
| **N-106** | 3087 | 🔴 N-106 — El ambar de emergencia de la app NO saca al Esclavo del Modo Degradado |
| **N-105** | 3018 | 🔴 N-105 — Cuatro documentos mandan cablear camaras sobre pines que NO son entradas de camara, y uno deja que el trafi... |
| **N-104** | 3144 | 🟢 N-104 — El mando se queda en A y B, las camaras entran por C y D, y la pantalla se fue porque NO HABIA PINES · **DE... |
| **N-103** | 3257 | 🔴 N-103 — El censo del instrumental frente a la arquitectura del 28/08: ~347 de 782 comprobaciones se quedan sin suje... |
| **N-102** | 3512 | 🔴 N-102 — `maestro_01_mando` ejerce SFTY-21 y no lleva la etiqueta `# EJERCE`: 15 comprobaciones que la tabla de traz... |
| **N-101** | 3607 | 🔴 N-101 — `Validacion_Automatico` compila `mando.cpp` REAL: retirar el mando no lo hace FALLAR, lo hace ABORTAR, y se... |
| **N-100** | 3727 | 🔴 N-100 — Cinco afirmaciones marcadas MEDIDO fueron refutadas por el firmware el MISMO dia, y siguen publicadas |
| **N-99** | 3981 | 🔴 N-99 — El Manual 11 manda cablear el bus I2C del reloj a la entrada de camara y a un LED |
| **N-98** | 4072 | 🔴 N-98 — Tres decisiones estructurales tienen DOS respuestas opuestas vivas, y las dos dicen "decidido el 28/08" |
| **N-97** | 4236 | 🔴 N-97 — La camara de demanda no es la misma entrada en las dos puntas: en el Maestro vive dentro del Modo Inteligent... |
| **N-96** | 4348 | 🔴 N-96 — La barrera de salidas dice gobernar OCHO pines de luz y gobierna SEIS: `ROJO_PEATON`, `VERDE_PEATON` y `BUZZ... |
| **N-95** | 4511 | 🟠 N-95 — `PA8` sobrevivio a su motivo, y el comentario que lo justifica describe un equipo que ya no existe |
| **N-94** | 4644 | 🔴 N-94 — El transporte del enlace `J17` no lo vigila ningun pack, y el contrato de bytes que el ESP32 tiene que cumpl... |

> 🔴 **SEIS `N-x` NO TIENEN NI UNA LINEA AQUI, Y ESO ES EL DATO: siguen ABIERTOS.**
> **N-108** (los 25 s sin subir a campo) · **N-110** (el teclado del PIN) · **N-113**
> (si el ESP32 se cuelga) · **N-116** (la tarjeta Maestro muerta) · **N-117** (el perro
> del ESP32) · **N-120** (las entradas de campo desnudas). Estan enteros en
> [`roadmap.md`](roadmap.md).

> ⚠️ **Y hay siete que SI aparecen aqui y NO estan cerrados: `N-142`, `N-147`, `N-150`,**
> **`N-151`, `N-152`, `N-153` y `N-157`.** Lo que hay aqui es **como se construyo el
> arreglo**; lo que falta —que alguien lo ejerza en una tarjeta— vive en el apartado
> **POR VALIDAR** de [`roadmap.md`](roadmap.md). Un commit no es una prueba de banco.

---

## Indice por apartado y fecha — la sesion en que se escribio cada cosa

| linea | apartado |
|---|---|
| 133 | 0. PARA RETOMAR — leelo entero antes de tocar nada |
| 220 | 0.0.duodetricies EL PAQUETE SALIÓ — y lo que ese `.zip` NO autoriza |
| 262 | 0.0.septvicies LA SEGUNDA VUELTA — documentos, y tres defectos que ningún pack podía ver |
| 331 | 0.0.sexvicies EL CIERRE DE LA NOCHE DEL 05/09 — 18 commits, y el arquitecto dijo NO AVALO |
| 393 | 0.0.quinvicies EL REPARTO DE PINES, CERRADO (05/09) — no se vuelve a preguntar |
| 426 | 0.0.quatervicies COMO SE REPARTE EL TRABAJO ENTRE SUBAGENTES — y el orden de lo que queda |
| 496 | 0.0.tervicies LO QUE EL RESPONSABLE DECIDIO LA NOCHE DEL 05/09 — cuatro decisiones y una condicion |
| 595 | 0.0.duovicies.bis LOS CUATRO AGENTES DE LA MADRUGADA — tres arreglaron el instrumento y uno construyo obra |
| 695 | 0.0.unvicies EL DIA DE LAS CAMARAS (05/09) — lo que se decidio y lo que se destapo |
| 731 | 0.0.vicies LAS DECISIONES SE MUDAN A `DECISIONES.md` — este fichero deja de ser donde se buscan |
| 749 | 0.0.septendecies EL MANDO **NO SE RETIRA**: A y B se quedan, sin usar y SIN TOCAR EL CODIGO |
| 849 | 0.0.sexdecies LOS TRES QUE SE CERRARON CON CODIGO — y lo que cada uno enseño de paso |
| 931 | 0.0.quindecies PLAN DE CIERRE — que cierra cada pendiente, y cual NO lo cierra nadie desde el PC |
| 968 | 0.0.quaterdecies CIERRE DE LA SESION DE BANCO DEL 04-05/09 — lo que la CINTA cerro y lo que dejo abierto |
| 1057 | 0.0.bis · 🔴 REPORTE DE CAMPO DEL 04/09, TARDE — LAS DOS PLACAS YA HABLAN, Y APARECIO N-42 ENTERO |
| 1976 | 1. Que hay hoy |
| 1994 | 3. Lo decidido, con fecha |
| 2014 | 4. Lo que esta ABIERTO, y de quien es |
| 2046 | 5. Donde vamos — 04/09, con el banco corrido |
| 2108 | 5.bis El orden de arranque — que se lanza, cuando, y que abre cada puerta |
| 2171 | 6. Los hallazgos de esta sesion — el porque de todo lo de arriba |
| 3915 | ⚠️ Observacion de metodo — la parte que impide que esto vuelva |

---

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

### 0.0.duovicies TODO POR APP — el mando desaparece del gabinete, y con el N-118

**Confirmado por el responsable el 05/09:** *«ya no tenemos mandos de A y B, solo la app, los
quitamos»*. `D-1` queda completa con su lado de hardware y nace `D-16`.

### Las dos mitades, que hay que leer juntas

**El HARDWARE se fue** (lista de compras rev. 3, 28/08). **El CODIGO se queda**, y no por
inercia: `mando_ambarLocal()` tiene **CINCO llamadas vivas** y su veto es SFTY-21. Retirarlo dejaria
los `if` **siempre verdaderos** —el veto no queda inerte, **queda abierto**— y ademas **el
banco se caeria en ABORTADO, no en rojo**: dos `raise` disparan solos y los dos modelos leen
constantes de `mando.cpp` **en el import**.

Con el mando desmontado, esa bandera **simplemente no se arma nunca**. Que es lo correcto.

### N-118 se cierra, y era DOS cosas contadas como una

1. Los **`0,6 V` en reposo** eran sintoma del **firmware VIEJO** —`INPUT_PULLUP` peleando
   contra los 10 kOhm de la placa—, corregido en `346ea5f`, que paso los dos pines a `INPUT`
   pelado con lectura `== HIGH`. **Nadie lo volvio a medir con el binario nuevo dentro.**
2. Y ya da igual: **no hay mando que conectar**. Los pines quedan libres.

> 🔴 **Media sesion contandolo como «lo mas urgente de todo lo abierto» y como «un cable o un
> multimetro».** Ni una cosa ni la otra: **no habia defecto de placa que arreglar**. Es la
> forma de §4 aplicada a un sintoma que ya tenia su causa corregida en el fuente y nadie
> volvio a medir.

### 🔴 Y lo que SI es real, y no estaba escrito en ninguna parte

**`D-16`: sin telefono no hay forma de operar el equipo.** Ni ambar, ni volver a automatico,
ni parar el cruce. **Es una propiedad DECLARADA del sistema, no una averia** — pero la idea
de que *«siempre queda el mando desde el suelo»* seguia viva en la cabeza de todos.

Y no es teorica: esta semana hubo que **desvincular el Maestro en Ajustes de Android** para
poder conectarse al Esclavo. Un movil sin bateria, un emparejamiento que falla o dos tecnicos
con el mismo equipo **dejan el poste sin mando de ninguna clase**.

**Consecuencia operativa que va al manual: el telefono es herramienta critica.** Bateria,
cable, y conviene un segundo terminal emparejado.

---

## 0.0.duodetricies EL PAQUETE SALIÓ — y lo que ese `.zip` NO autoriza

**`Paquete_IOT_VIAL_Semaforos_2026-09-05_0ec94ef_SIN_BANCO.zip`**, 8,3 MB, 319 entradas.
**32 commits** en la sesión. Compuerta `20 PASS · 0 FALLA · 0 ABORTADO`, árbol limpio.

Comprobado **sobre el propio zip**, no sobre la intención: **cero artefactos de compilación**, el
md5 del binario de dentro **igual** al del disco, el LEEME **cita el nombre exacto** de la APK que
le acompaña, y **dice «¿Ha pasado banco? NO» en su segundo apartado**.

> 🔴 **ES UN ENCARGO DE BANCO, NO UNA ENTREGA DE VERSIÓN.** No entrega nada: **pide** que alguien
> ponga la tarjeta delante. En campo sigue corriendo **`e303485`, del 31 de julio**, y **nada de
> esta sesión ha tocado una tarjeta**.

### Lo que sigue sin cumplirse, y es lo único que bloquea

**El manual del «doble» existe — pero la condición no la cierra un commit, la cierra la firma.**
El hueco está en el documento con dos preguntas que se pueden contestar «No», y si sale «No», el
apartado vuelve a redacción. Hasta entonces `TECHO_POR_SUELO` **no debería salir en un paquete**.

### Dos trampas del empaquetado medidas al final, y una borra el Bluetooth

1. 🔴 **`assets/public` NO es `www`:** tiene tres ficheros más que no vienen del repositorio, y uno
   es **el plugin por el que la app abre el socket SPP**. `cp -r www/.` **añade y pisa, no borra**;
   un `rm -rf` antes de copiar —*el reflejo natural de «dejarlo limpio»*— se lo lleva por delante y
   **la APK compila igual y arranca sin poder conectar**. Y esta vez **ningún `<script src=>`
   delataría la ausencia**.
2. 🟠 **El tamaño engañó en la dirección contraria a la conocida:** **154 B** de diferencia por un
   cambio que añade ~300 caracteres. *«Prácticamente el mismo tamaño, será la misma»* habría sido
   la conclusión.

### Y el balance de la sesión, contado como se cuenta aquí

| | |
|---|---|
| **defectos de firmware cerrados** | el `$STATUS` que no cabía · el corte de 15 s · los tiempos que el Modo Inteligente tiraba · el pin vacío que alarmaba · la cámara que no podía sostener una fase · el Degradado sin puerta |
| **defectos de INSTRUMENTO cerrados** | `app_11` que no juzgaba nada · `botones.cpp` que no se compilaba en ningún arnés · `TROUBLESHOOTING` y `FIRMWARE` sin vigilante |
| **defectos que ningún instrumento podía ver** | el diálogo con una salida imposible · el formulario de firma que volvía inservible · el ejemplo de trama con el checksum bien y tres campos de menos |
| **correcciones a MIS datos** | **siete**, y **cero cazadas por mí** |

**Las siete las cazaron los agentes, y las siete por la misma frase del encargo.** Si de esta
sesión hay que llevarse una sola cosa al método, es ésa — y no es un pack.

## 0.0.septvicies LA SEGUNDA VUELTA — documentos, y tres defectos que ningún pack podía ver

Cerrado el firmware, se barrió todo lo que lo describe. **Y de los tres defectos que aparecieron,
ninguno estaba en el código: los tres estaban en lo que la gente lee.**

### 🔴 1 · El diálogo de la app daba una salida QUE NO EXISTE en el poste al que se aplica

Decía *«Para salir: **VOLVER AL MENÚ**»*, y `SET_MODO:MENU` **sólo lo atiende el Poste 1** — la
propia app la frenaría. La frase se escribió cuando el modo era sólo del Maestro; **`D-18` lo
abrió al Esclavo esa misma noche y la frase se quedó.**

> **Un operario que la siga se queda sin poder sacar al Poste 2 del modo** — subido a un poste, y
> en **el único modo que da verde sin confirmar la otra punta**.

Es §2.ter en la interfaz: una función **declarada** —*«para salir, haga esto»*— que nadie había
**ejercido** contra la punta nueva. Y **ningún pack podía verlo**: ninguno lee el texto de un
diálogo contra el enrutado por punta. Lo encontró **el agente que escribía el manual**, al
intentar explicar el botón.

### 🔴 2 · El formulario de firma volvía INSERVIBLE en `.docx`

`convertir_a_word.py` **colapsa una tirada de líneas `>` en UN solo párrafo**: las tablas de
dentro se vuelven una ristra de barras y los encabezados desaparecen. El bloque de firma —**la
condición del responsable**— caía en **un párrafo**. Reescrito sin blockquote: ahora es una tabla
de Word de 9 filas con celdas vacías, y el manual pasa de 12 a **17 tablas**.

**En pantalla se veía perfecto y volvía inservible**, que es §4.quater exacta.

### 🔴 3 · El empaquetador NO regenera los `.docx`: sólo comprueba que EXISTAN

Un `.md` editado y un `.docx` viejo **se empaquetan juntos sin que nada avise**, y **el funcional
firma un documento sin la sección**. Hoy se salvó porque el conversor se corrió a mano antes de
armar el `.zip` — **eso era suerte de procedimiento, no una red**.

### Lo que se corrigió de cobre, y por qué el motivo importa más que el dato

El manual mandaba en dos sitios cablear la cámara a **`J14`**, y el 05/09 se decidió **`J16`**.
Corregido — **y con el motivo que lo hace no arbitrario**:

> **El vigilante mira `J16` y no mira `J14`.** Con la cámara en `J14` funcionaría —pide paso—
> pero **nadie sabría nunca si se ha estropeado**.

`J14`/`PB0` **se conserva vivo** y es el mejor candidato a fin de carrera de barrera: es **el
único pin de entrada con antirrebote por hardware** (≈ 1 ms), y un contacto mecánico es justo lo
que rebota.

### El manual del «doble» existe, y no lo cierra un commit

La frase que se lleva el operario:

> **El verde no se acorta nunca. Sólo se alarga — y sólo cuando al otro lado no espera nadie.**

Y el criterio para distinguirlo de una avería **sin instrumentos**, que es la parte útil:
**póngalo en Automático y mire dos o tres ciclos**. Vale porque es **la propiedad de seguridad
convertida en prueba de campo** — con las cámaras muertas el modo ES el Automático, así que
ponerlo en Automático **es apagar las cámaras a mano**.

🔴 **Lo cierra la firma del funcional, no el commit.** El hueco está en el documento con dos
preguntas que se pueden contestar «No» — y si sale «No», el apartado vuelve a redacción.

### Y la cuenta de las correcciones a MIS datos: SIETE, y cero cazadas por mí

La séptima: dije que `protocolo_tramasDescartadas()` **no tiene un solo llamador**. Tiene **dos**,
en el Esclavo; es huérfana **sólo en el Maestro**, y un pack lo decía con esas palabras desde
agosto. **Escrito como yo lo dije habría sido un defecto INVENTADO.**

**Las siete las cazaron los agentes.** Y las siete por la misma frase en el encargo: *«todo lo que
te he escrito es un instrumento, no una medida — si algo es falso, dilo»*.

## 0.0.sexvicies EL CIERRE DE LA NOCHE DEL 05/09 — 18 commits, y el arquitecto dijo NO AVALO

**Compuerta `20 PASS · 0 FALLA · 0 ABORTADO`, dos pasadas seguidas con el árbol quieto.** Y ese
verde vale menos que el rojo de media tarde: el `1 FALLA` de entonces era **el hallazgo más útil
del día**. Se fue porque **se arregló el firmware**, no porque nadie mire.

### Lo que se construyó

`$STATUS` acotado **por buffer** · el vigilante de cámaras · `CMD:LEER_RTC` · el campo `CAM:` en
las dos puntas y en la app · **el Modo Inteligente usando los tiempos del operario** · la cámara
de `J16` pudiendo **sostener** una fase · **el Degradado del poste 2 por app, y visible** · y el
barrido de todo el legacy.

### 🔴 EL ARQUITECTO (Fable) DIJO **NO AVALO** SOBRE `fb7aaca`, y hay que saber por qué

**No por el ratio** —esa noche fue **1,60 : 1** contra el **2,74 : 1** acumulado, o sea mejor que
la media—. **Por §2.ter:** el entregable nuevo de la noche estaba *«declarado en tres manuales,
certificado por 675 líneas de pack en verde, y en la instalación decidida avisaba de una cámara
que no existe y no podía decir `OK` nunca»*.

**De sus cuatro condiciones se cumplieron TRES**, y la cuarta sigue abierta:

| | |
|---|---|
| ✅ el pin vacío deja de alarmar, **demostrado con inyección sobre `botones.cpp` COMPILADO** | y ese fichero **no se compilaba en ningún arnés del proyecto** |
| ✅ las decisiones están en `DECISIONES.md` | `D-17.bis` y `D-18` |
| ✅ los documentos ya no contradicen al firmware de HEAD | nueve sitios de `M3` corregidos, entre otros |
| 🔴 **el manual del «doble» no existe** | y era **la condición que puso el responsable**: *«el doble **si** un funcional revisa el manual y este manual de uso es claro»*. Hasta entonces `TECHO_POR_SUELO` **no debe salir en ningún paquete** |

### Los seis defectos que ningún instrumento veía, y qué los destapó

| defecto | lo destapó |
|---|---|
| el `$STATUS` no cabía: **169 B por buffer, no 162 por tipo** — y el Esclavo **cabía por un byte** | medir por **buffer** en vez de por tipo |
| `app_11` **no juzgaba ni una línea** — y al enseñarle apareció el corte de 15 s | enseñar a juzgar al pack |
| el Modo Inteligente **tiraba todos los tiempos del operario** | leer el fuente |
| un pin **sin cámara** alarmaba de un aparato que no existe | el arquitecto |
| la cámara **pedía** paso pero no podía **sostener** la fase: **362.500 ms contra 720.000** | **el arnés corriendo** — y tumbó también la primera reparación |
| `TROUBLESHOOTING.md` y `FIRMWARE.md` publicaban **5 s, 12 s, 300 bps** | **no los vigila ningún instrumento** |

### Y las tres lecciones que se llevan a `CLAUDE.md`, porque no son de este día

1. **El acta puede publicar el binario ANTERIOR** — medido por dos agentes: `57.360` contra
   `57.416`. PlatformIO sirviendo un incremental viejo. **Una cifra correcta de un binario que ya
   no existe.**
2. **La guarda de rutas no cubre los DOCUMENTOS.** Mover dos manuales habría abortado los 76
   packs mientras la guarda seguía diciendo *«64 rutas, todas existen»*.
3. **Un instrumento que mide la FORMA no puede ver un defecto del TIEMPO**, y el síntoma es el
   tamaño: **675 líneas de Python para 240 de C++**.

### 🔴 Y lo que seis correcciones en una noche dicen de mí, no del repositorio

**Seis afirmaciones mías se cayeron, y cada una llegó al responsable o al repositorio con la
palabra «medido» encima.** Tres eran el **mismo** error: `grep` que contaban **comentarios** —que
en este repositorio **citan los nombres que explican**, por convención— o que buscaban un nombre
que no existe (`BOTON1_PIN`). El pack `maestro_12` ya lo sabía y empieza quitando los
comentarios *«para no acusarse a sí mismo»*.

**Las seis las cazaron los agentes, ninguna yo.** Y las cazaron porque a todos se les dijo
expresamente **que pusieran en duda los datos de quien les encarga**. Esa frase en el encargo ha
rendido más que cualquier pack nuevo.

## 0.0.quinvicies EL REPARTO DE PINES, CERRADO (05/09) — no se vuelve a preguntar

**Decidido por el responsable. Esto deja de estar abierto.**

| bornera | pin | queda como |
|---|---|---|
| **`J16` p10** | `PB14` · `CAM_C_PIN` | ✅ **LA CÁMARA** — una por poste |
| **`J16` p12** | `PB15` · `CAM_D_PIN` | pin de cámara **vacío** — y por eso **no se vigila hasta que dé su primer flanco** |
| **`J16` p5 / p8** | `PB9` / `PB13` | **el mando**, `A` y `B`. Su código **no se toca** (`D-1`) |
| **`J14`** | `PB0` | **libre**, y es el candidato del fin de carrera de barrera |

**Los cuatro pines de `J16` son `INPUT` pelado y activos en ALTO**, con 3,3 V en `p9`/`p11` para
cerrar contra ellos. Y **`p1` lleva 12 V crudos** — sin opto, sin serie, sin clamp—: taparlo es
obligatorio en cada equipo (`N-120`).

### Los dos hechos que gobiernan cualquier uso futuro de estos pines

1. 🔴 **`p5` y `p8` NO están libres, y cada uno hace DOS trabajos:** mueven el cursor
   (`botonArriba()`/`botonAbajo()`, con llamadores vivos) **y alimentan el reconocedor de
   secuencias del mando** (`botones.cpp:480-481`). Un fin de carrera ahí **dispararía órdenes por
   accidente** — `B.B.B` es ámbar local con su cerrojo de SFTY-21, `A.B.A.B` es entrar en
   Degradado. **El cruce obedeciendo a una pluma.** Usarlos para otra cosa exige **cortar antes esa
   alimentación**, que es una línea y es una decisión.
2. ✅ **`J14`/`PB0` es el único pin de entrada del proyecto con antirrebote por hardware**
   —`R64` 10 K + `C25` 100 nF = 1 ms— y **no alimenta ninguna secuencia**. Es el sitio correcto
   para un contacto mecánico, que es lo que rebota.

### Y el defecto que esta decisión destapó, en construcción

**El vigilante miraba `J16` y el Modo Inteligente leía `J14`**, así que con la cámara en
cualquiera de las dos borneras **una mitad estaba ciega**. La lista de compras decía *«son las dos
que el firmware lee hoy»*: falso en las dos direcciones.

## 0.0.quatervicies COMO SE REPARTE EL TRABAJO ENTRE SUBAGENTES — y el orden de lo que queda

> **EL LIMITE NO ES CUANTOS AGENTES CABEN: ES QUE DOS NO ESCRIBAN EL MISMO FICHERO.**

Con el arbol compartido nadie avisa: **`git add -A` barre lo que el otro tiene en vuelo, y la
historia queda mintiendo** (§8.quinquies). Ya paso el 27/08 —un commit titulado «N-71» que
contiene **solo el acta**— y volvio a pasar el 05/09, cuando un `git add 01_Firmware/` se llevo el
trabajo a medias de otro agente.

**Las tres reglas que salen de eso, y son de operacion, no de estilo:**

1. **A cada agente se le da la LISTA de ficheros que posee, y la lista de los que tiene
   prohibidos.** Por nombre. Un encargo por «tema» no reparte nada: dos temas distintos caen en
   `bluetooth.cpp` constantemente.
2. **Ningun agente hace `git add` ni `git commit`.** Comitea el que coordina, **con rutas
   explicitas y mirando el INDICE antes** — `git diff --cached --name-only—, que es lo unico que ve
   esta clase de fallo (§8.quinquies, segunda via).
3. **Un agente que solo CENSA lee con `git show HEAD:<ruta>`, no del arbol.** Si no, informa sobre
   media frase de otro agente y ese hallazgo es ruido.

### Y una cuarta que aparecio el 05/09, porque no estaba escrita

> 🔴 **Un cambio de firmware de un agente puede dejar ROJO un pack cuyo arreglo esta en los
> ficheros de OTRO.**

Paso literalmente: el agente del campo `CAM:` lo anadio al `$STATUS` de las dos puntas, y eso puso
`documentos_03_trama_status` en `FALLA` porque el manual que documenta esa trama **es del lote de
otro agente**. Se arreglo por casualidad —el segundo agente lo vio y lo documento—, y esa
casualidad no es un metodo.

**La regla: al repartir, se mira que el pack que vigila una propiedad y el fichero que la produce
caigan en el MISMO lote.** Cuando no se puede, se dice en los dos encargos.

### El orden de lo que queda, y por que es ese

**Lo que se puede hacer en paralelo hoy** — ficheros disjuntos, ningun solape:

| lote | ficheros | de que decision sale |
|---|---|---|
| **Modo Inteligente con los tiempos del operario** | `modo_inteligente.cpp`, `modo_automatico.*`, `app_11` | decision del 05/09 · **es el unico `FALLA` de la compuerta** |
| **Campo `CAM:` y la fila en la app** | `bluetooth.cpp` de las dos puntas, la app, `esp32_07`, `costura_10` | `A-13`, tecnica |
| **Censo de las spec contra el firmware** | **ninguno — solo lee** | encargo del responsable |

**Lo que va DETRAS y no puede solaparse:**

- **La camara ciega de 6 h a 4 dias** y el modo declarandose averiado → toca `botones.cpp` y
  `camara_03`, **los mismos que el lote del `CAM:`**. Espera a que ese libere.
- **La pluma que puede no bajar** (fase 2 de `D-13`) → toca `semaforo.cpp`, que es la **barrera de
  salidas** (§6). No se mete en el mismo turno que otro cambio de firmware.
- **El boton de Degradado del poste 2** → **espera una respuesta del responsable** (`A-11`).
- **El manual que explica por que un verde a veces dura el doble** → es la **condicion** que puso el
  responsable para el techo del doble. Va con el cambio, no despues.

### Y lo que se le pide a un agente ARQUITECTO al final, que no es re-medir

Los packs ya miden. Lo que ningun pack contesta son estas tres, y son las que este proyecto ha
fallado:

1. **¿Esto acerca una tarjeta cargada, o la sustituye?** (§2.bis). En campo corre `e303485` del
   **31/07**. La medida es firmware contra instrumento, y ya iba **2,31 : 1**.
2. **¿Que esta DECLARADO y no EJERCIDO?** (§2.ter). Aparecio cinco veces en un dia, y el 05/09 una
   mas: el Degradado del poste 2, descrito en los manuales y sin puerta viva.
3. **¿Se contradicen entre si las decisiones nuevas?** Cuatro en una noche. La que mas pesa: retirar
   el mando **abre el veto de SFTY-21** —cinco lectores— y **deja al Esclavo sin puerta al
   Degradado**.

**Y se le dice que ponga en duda los datos del que le encarga**: el 05/09 un `grep` mal hecho llego
al responsable con la palabra «medido» encima, y le habria hecho decidir entre dos opciones cuando
la barata —que existia— no estaba en la mesa.

## 0.0.tervicies LO QUE EL RESPONSABLE DECIDIO LA NOCHE DEL 05/09 — cuatro decisiones y una condicion

### 1 · `A-12` · EL MODO INTELIGENTE PASA A USAR LOS TIEMPOS QUE CONFIGURA EL OPERARIO

**El defecto, medido:** `modo_inteligente.cpp` **no lee ni uno** de los tiempos que manda
`SET_TIEMPOS`. Se fija los suyos en el arranque y no vuelve a mirar:

```
static int maxVerde = VERDE_MIN_MIN;   // el SUELO del rango, siempre 3 min
maxVerde = VERDE_MIN_MIN;              // linea 75, y nadie mas lo escribe nunca
```

Asi que **si el operario configura 6 minutos porque ese tramo es largo, en Modo Inteligente el
cruce corre a 3** — y encima la Regla 1 puede cortar el verde a los **15 s**. La app manda bien el
dato; **es el firmware el que lo tira.** Y por eso ninguna guarda lo veia: ese modo **no pasa por
`SET_TIEMPOS`**.

> 🔴 **Y el «arreglo de una linea» habria matado el modo.** `maxVerde` ya vale 3 min, asi que
> subir el piso de la Regla 1 a 3 min deja **suelo = techo**: el verde duraria siempre 3 min
> exactos y **las camaras quedarian inertes justo en el unico modo que las usa**. Es §3.septies
> otra vez —una guarda que ya no puede dar las dos respuestas—. **Son DOS numeros, no uno.**

**El modelo decidido, y su propiedad de seguridad:**

| | |
|---|---|
| **La fase dura lo que se configura** — 3, 4, 5, 6 min **segun la distancia de cada cruce** | igual que el Automatico |
| Cumplido ese tiempo, **si el otro lado pide paso → cambia** | igual que el Automatico |
| Si el otro lado **NO** pide nada y en el mio sigue habiendo trafico → **mantiene**, hasta el techo | esto es lo que aportan las camaras |
| **Techo = el DOBLE del tiempo configurado** | decidido el 05/09 |

> ✅ **LA PROPIEDAD QUE HACE QUE ESTO SEA SEGURO: con las camaras muertas, el Modo Inteligente se
> comporta EXACTAMENTE como el Automatico.** Si la camara nunca dice «hay coches», el modo nunca
> alarga: degrada al comportamiento conocido, sin verdes de 15 s ni esperas raras.

**Y las camaras dejan de poder ACORTAR un verde** —que es lo peligroso, porque es lo que hace que
el conductor crea que el equipo esta averiado y adelante en rojo (`D-5`)— **y solo pueden
alargarlo cuando no hay nadie esperando al otro lado**, que no molesta a nadie.

> 🟡 **LA CONDICION QUE PUSO EL RESPONSABLE, Y NO ES UN ADORNO:** *«el doble **si un funcional
> revisa el manual y este manual de uso es claro**»*. Un verde que unas veces dura 3 min y otras 6
> **parece una averia** a quien lo mira desde la acera. **Sin ese manual el numero correcto llega a
> la calle y vuelve reportado como defecto.** El manual es parte de la entrega, no un paso
> posterior.

### 2 · `A-1.bis` · LA BARRERA PUEDE NO BAJAR, Y EL SEMAFORO CAMBIA IGUAL

Palabras del responsable: *«el semaforo cambia, la talanquera a lo mejor ni baja si existe una
alarma presente, no importa, pero es seguridad para no danar un vehiculo, **todo puede operar en
normalidad sin la barrera**»*.

**Esto es mucho mas estrecho que «derogar SFTY-28», y por eso es seguro:** la camara puede
**impedir que la pluma baje**, y **nunca** puede impedir que una luz cambie. El peor modo de fallo
—camara pegada en «hay presencia»— acaba en **pluma siempre arriba**, que es exactamente lo que se
acaba de declarar operable. **El trafico no se entera.**

### 3 · EL PLAZO DE LA CAMARA CIEGA SUBE DE 6 h A 4 DIAS, Y EL MODO SE DECLARA AVERIADO

*«si una camara se estropea, la camara no cambia por dias, pasa a modo fallo y debe pedir se
revise la camara, que se yo 4 dias sin alarmas»*.

**Y el motivo es bueno: una camara rota no se arregla rapido, asi que no hay ninguna prisa por
acusarla — y si hay mucho coste en acusarla de mas.** Una alarma que salta un domingo sin trafico
manda a alguien a un poste sano, y a la tercera vez nadie las mira. 4 dias de **paso abierto** son
unos **8 dias de reloj** con el ciclo corriendo.

> **La segunda mitad vale mas que el numero: «fallo de ese modo».** No es una alarma mas — es que
> **el Modo Inteligente se declare averiado y pida revision**. Encaja con la propiedad de arriba:
> con la camara ciega el modo ya se comporta como el Automatico, **solo que sin saberlo**. Con
> esto deja de ser «sin saberlo».

**Y las dos averias opuestas llevan plazos opuestos, a proposito:**

| averia | plazo | por que |
|---|---|---|
| **CIEGA** (no ve nada) | **4 dias** | mientras tanto **no molesta a nadie**: el modo degrada al Automatico |
| **PEGADA** (ve siempre) | **20 min** | **si hace dano**: alargaria el verde hasta el techo en cada ciclo |

### 4 · BOTONES Y PANTALLA FUERA — y de las tres cosas UNA YA ESTABA HECHA

*«los botones a, b, c y d los eliminamos, la pantalla LCD tambien, la app la tenemos»*.
**Medido antes de tocar nada:**

| | estado real el 05/09 |
|---|---|
| **Los cuatro botones** | ⚠️ **DOS fuera y DOS VIVOS**: `botonAceptar()`/`botonCancelar()` son `return false;` —los de `PB14`/`PB15`, **los pines que ahora son cámaras**—, pero `botonArriba()`/`botonAbajo()` devuelven `consumir(0)`/`consumir(1)` **con llamadores vivos**, y leen `BOTON1`/`BOTON2`, **que son los pines del mando**. ~~«`BOTON1`/`BOTON2` no se leen en ningun sitio, `grep -c` = 0»~~ — **FALSO, escrito por mí el 05/09**: el `grep` buscaba `BOTON1_PIN`, que no es como se llama (`botones.cpp:394-398`). 🔴 **Lo que alguien cierre en `J16` p5/p8 SIGUE ENTRANDO al firmware** |
| **La pantalla LCD** | ❌ **sigue enlazada**: 17 llamadas `lcd_` en el Maestro, 8 en el Esclavo, y `menu.cpp` entero detras |
| **El mando `A`/`B`** | ❌ **sigue vivo** en `PB9`/`PB13` (`botones.cpp:480-481` y `:491-492`) |

> 🔴 **Y RETIRAR EL MANDO NO ES NEUTRO — son dos barreras, las dos medidas:**
>
> 1. **`mando_ambarLocal()` deja de armarse nunca**, y de esa bandera cuelgan **cinco lectores** que
>    la usan para **VETAR**. El veto de SFTY-21 no queda inerte: **queda abierto** (§3.ter).
> 2. **El Esclavo se queda sin su unica puerta viva al Degradado.** `degradado_entrar()` tiene dos
>    llamadores: `menu.cpp:227` —muerto con la pantalla— y `mando.cpp:148`, la secuencia `A.B.A.B`.
>
> **No se ejecuta en la misma frase que se dice.** Lo barato y sin barreras amputadas es lo
> contrario: **darle a la app la llave de la puerta que ya existe** (`A-11`).

## 0.0.duovicies.bis LOS CUATRO AGENTES DE LA MADRUGADA — tres arreglaron el instrumento y uno construyo obra

Se lanzaron cuatro en paralelo sobre ficheros disjuntos. **Ninguno de los cuatro cerro lo que
venia a cerrar sin destapar antes algo que estaba escrito como hecho.**

### `N-154` · el `$STATUS` no cabia en su peor caso, y por BUFFER eran 169 B (`e43a8e7`)

**LA COTA BUENA ES EL BUFFER DE CADA CAMPO, NO SU RANGO NI SU TIPO.** Es lo unico que `snprintf`
garantiza sin fiarse de otro modulo. El apunte de N-153 decia **162 B por tipo**; medido por
buffer eran **169** contra un techo de 155 — **25 de mas, no 7**. Y **el Esclavo cabia por UN
byte**, sin que nadie lo hubiera mirado nunca.

Hoy no truncaba porque los valores reales son cortos. Y **una trama truncada no da un dato malo**:
sale bien formada hasta la mitad, no casa el CRC, y la app la descarta entera. El sintoma seria
*«el equipo se callo»*, que manda a mirar el cable.

Maestro **169 -> 141 B** (margen +3), Esclavo **127 -> 123**. **+48 B de flash en el Maestro, CERO
en el Esclavo, y sin agrandar ningun buffer.** Las cotas **se derivan**: `CUENTA_ATRAS_MAX_SEG`
sale de `limites_ciclo.h` —un `900` a mano habria sido la quinta copia de N-137— y
`RTT_PUBLICABLE_MAX_MS` es el propio umbral de orfandad de SFTY-6, porque **por encima de ahi no
hay enlace que medir**.

> **Y el cambio de fondo no es el tamano: es de que se fia el buffer.** Antes, de una invariante de
> otro fichero. Ahora, de una guarda que corre en la misma funcion y en cada emision.

Cuando un valor se sale se publica **`!`**, ni numero ni `--`: el `--` ya significa *«todavia no lo
se»*, y aplastar ahi un valor imposible **lo esconderia entre los huecos normales**.

🔴 **Residual escrito en vez de disimulado:** los 3 B que quedan **no dan para el campo `CAM:`**
—son 11 caracteres con su coma—. O se acota `HORA:` —validar hora/minuto/segundo lo baja de 11 a 8
y devuelve justo 3 B— o se sube `payload` a **155**, que es el techo real. Y
`bluetooth_reportarAlarma()` tiene el mismo defecto en pequeno, **medido y sin arreglar**:
`tramo[40]` pide 52 caracteres por tipo. Es menos grave —trunca un buffer INTERNO, asi que el
`$ALARM` sale bien formado y solo se queda sin el `SINRESP`— y no pierde la trama.

### `N-155` · `app_11` no juzgaba NINGUNA linea (`2d17678`)

El pack de rangos de tiempos estaba en verde **midiendo el vacio**. Al ensenarle a juzgar aparecio
lo que ya no se puede desver, y es lo unico que hoy tiene la compuerta en rojo:

> 🔴 **`modo_inteligente.cpp:123` corta un verde a los 15 s**, cuando el minimo vial que fijo el
> responsable el 04/09 (`D-5`) son **3 minutos**. Y esa comparacion **no pasa ni por `SET_TIEMPOS`
> ni por el menu**: ninguna de las dos guardas la toca. El cruce puede alternar por debajo del
> minimo **sin que nadie haya configurado nada**.

Esta en el **unico modo que usa las camaras**. Con una camara pegada en «hay presencia», esa punta
recibe verdes de ese tamano ciclo tras ciclo mientras la otra corre a 3 minutos. **Estaba escrito
ANTES de comprar las camaras: el hardware nuevo no lo trae, lo ENCUENTRA.**

**La compuerta se queda en `19 PASS · 1 FALLA` a proposito. Eso no es una regresion: es el
hallazgo.** Bajarle el liston al pack para que se ponga verde seria ajustar la medida hasta que de
el resultado que gusta.

🟡 **Y es una decision VIAL, no mecanica.** El arreglo es una linea y esta verificado por inyeccion,
pero subir ese piso a 3 min **cambia lo que hace el Modo Inteligente**: deja de poder alternar por
demanda dentro del primer verde. **Pendiente del responsable.**

### `N-156` · el `$ACK` del reloj, y el Degradado del Esclavo sin puerta (`5846cee`, `d020f3c`)

`CMD:LEER_RTC` —consultar el reloj **sin cambiarlo**, en Maestro, Esclavo y telefono, que es lo que
pidio el responsable en `A-9`—. Al construirlo aparecieron dos cosas que no se buscaban:

- **`N-118` REFUTADO con la medida del propio banco.** Los `0,6 V` de `MANDO_A`/`MANDO_B` eran
  sintoma del firmware **viejo** (`INPUT_PULLUP`), corregido en `346ea5f`. Y es moot: **ya no hay
  mando**.
- 🔴 **El Esclavo NO TIENE NINGUNA VIA para entrar en Modo Degradado.**
  `grep -c "SET_MODO" Esclavo/src/bluetooth.cpp` da **0**. El modo existe, esta construido y
  probado, y **nadie puede pedirlo en esa punta**. Abierto como **`A-11`**, y no lo cierra un
  agente: anadirle `SET_MODO` al Esclavo **cambia quien arbitra el ciclo**.

### `N-157` · fase 1 de `D-13`: la camara se vigila a si misma (`4b90f98`)

**Cero efecto vial**, y comprobado por diff, no por el informe: la unica funcion vial en las 238
lineas nuevas es `semaforo_plumaArriba()`, **y se lee**. El bloque es **identico byte a byte en las
dos puntas**.

Dos alarmas de mantenimiento —`CAM_PEGADA` a 20 min, `CAM_CIEGA` a 6 h **de paso abierto**— y un
contador que **observa** la transicion en vez de vetarla: vetarla exigiria entrar en
`escribirPines()`, que es **SFTY-28** y necesita la derogacion escrita de **`A-1.bis`**. Ese
contador es el dato que decidira si la fase 2 se construye.

**El umbral de `PEGADA` se deriva** del techo del ciclo y el pack lo recalcula del C++ cada vez
(N-71). **El de `CIEGA` NO sale de ninguna constante del firmware, y asi esta escrito**: cuanto
tarda el siguiente vehiculo es propiedad de la carretera, y fabricarle una derivacion seria `A-7`
otra vez.

🔴 **Lo que queda SIN EJERCER y no debe leerse como aprobado:** `CAM_CIEGA` a su valor de produccion
son 6 h, **no ejecutable en una sesion de banco**. Ejercerla exige cargar una compilacion con el
umbral reducido, **y esa compilacion no es la que va a campo**. El camino esta comprobado en su
**forma**, no en su **tiempo**.

### Lo que los cuatro tienen en comun

**Tres de los cuatro arreglaron un INSTRUMENTO, no el firmware** —un buffer que se fiaba de otro
fichero, un pack que juzgaba cero lineas, un motivo de huerfano que era falso—. Y el cuarto dejo su
getter **huerfano a proposito**, con el motivo **medido**: el `$STATUS` no admite hoy el campo
`CAM:`.

**Ninguno toco la calle.** Sigue corriendo `e303485` del 31/07.

## 0.0.unvicies EL DIA DE LAS CAMARAS (05/09) — lo que se decidio y lo que se destapo

> **El detalle de cada decision vive en [`DECISIONES.md`](DECISIONES.md).** Aqui queda el
> arco del dia y los hallazgos, que es lo que este fichero sabe hacer.

### Lo decidido

`D-12` de cada camara se consume **UN CONTACTO SECO y nada mas** · `D-13` **las dos camaras
igual**, vigilando el barrido de la pluma, **y el significado lo pone el estado del
semaforo** —un bit, cinco lecturas— · `D-14` **la ENTRADA de alarma graba, y NO depende de
la casilla bloqueada** · `D-15` **el reloj lo contesta quien lo tiene**.

### Lo que se destapo, y casi todo corrige algo que estaba escrito como hecho

| | qué |
|---|---|
| 🔴 **Dos relojes por cruce y nada los sincroniza** | Y **hoy nos salva un cristal roto, no un diseño**: el Degradado calcula con el reloj muerto del STM32, así que el margen de 29 s **no lo gasta nadie**. Con `AB-4` cambia de dueño: la deriva mejora 50×, **y el desfase inicial no tiene cota**. `A-9` |
| 🔴 **`D21` con el cátodo al aire, en un canal de luz VIVO** | `Q6` → `J8` → **`VERDE2`**. Ese canal no tiene indicador y su borne **flota** en vez de estar a 12 V como los otros nueve. **Nadie lo había anotado nunca** |
| 🔴 **La compuerta cayó a `13 PASS / 7 ABORTADO`** durante la sesión | `ld` no abre la ruta con `ñ` — **N-44 sigue vivo**. Volvió con una copia del toolchain **que no está en el repositorio** |
| 🔴 **`reloj_enHora()` del Maestro ya era FALSO SIEMPRE** | Los tres escritores exigen `Y2`, que está muerto. La máquina de sincronización entera **no podía disparar** |
| **Las cámaras no hacen nada en Auto ni en Manual** | `demanda_hayLocal()` tiene **un solo lector**, y es el modo que ningún arnés compila y ninguna tarjeta ha corrido |
| **El `~1 s` del relé vive en NUEVE sitios** | Cinco comentarios y **dos packs** que salen verdes contra un número que **nos inventamos** |
| **178 de 240 citas no señalaban al sitio** | El 74 %. Ahora cero, y con la regla de §4.sexies para que no vuelvan |
| **El opto «aísla galvánicamente» es medio falso** | Separa el pin, **no la masa**: una sola red `GND` de 103 pads |
| **El manual mandaba un hilo que no existe** | *«Retorno de masa por `p2`»*, en cuatro sitios. El contacto vuelve por `R67`, dentro de la placa |
| **`1A`/`1B` ya es LECTURA** | La Quick Start Guide **sí estaba en disco** — pero **sin capa de texto**, así que toda búsqueda daba cero **por el formato**. Se renderizó y se miró |

### Y lo que no cambia

**Nada de esto ha tocado una tarjeta.** El `20/20` dice que los modelos y los arneses de PC
no encuentran nada, y esta sesión volvió a dar el contraejemplo: **el defecto del ámbar y el
del Modo Manual pasaron esas veinte comprobaciones sin despeinarlas**, y los encontró una
cinta de tramas.

---

## 0.0.vicies LAS DECISIONES SE MUDAN A `DECISIONES.md` — este fichero deja de ser donde se buscan

> **Una revision externa lo midio el 05/09: este roadmap tiene 4.700 lineas, `CLAUDE.md`
> dice que no hace falta leerlo entero, y un agente que hace `grep` encuentra la version
> que case primero.** Las decisiones aqui se ANADEN sin derogar las anteriores, y por eso
> hubo que matar un encargo que contradecia una decision escrita desde el 31/08.

**A partir del 05/09 la decision vigente vive en [`DECISIONES.md`](DECISIONES.md), en una
linea.** Doce vigentes y diez abiertas. Aqui se queda **el porque largo**, que es lo que
este fichero sabe hacer bien; lo que MANDA es aquella tabla.

**Si una fila de `DECISIONES.md` y un parrafo de este roadmap no coinciden, gana la fila y
el parrafo esta caducado.** Y antes de encargar un cambio de alcance —a un agente o a uno
mismo— se lee aquella tabla: si el encargo la contradice, eso no es una orden, es una
pregunta.

---

## 0.0.septendecies EL MANDO **NO SE RETIRA**: A y B se quedan, sin usar y SIN TOCAR EL CODIGO

> 🔴 **Esta seccion se escribio primero al reves, y se corrige aqui en vez de reescribirse,
> porque el error es la parte que vale.** Se llego a lanzar un agente a retirar el mando
> entero de las dos puntas. Lo paro el responsable: *«ayer definimos no quitar los botones
> de A y B por los danos al software, y seguir sin usarlos y sin tocar el codigo»*. El
> agente estaba midiendo y no alcanzo a editar nada.

### La decision, que ya estaba escrita desde el 31/08

`05_Funcional/17_Arquitectura_28-08_y_Decisiones_Abiertas.md` lo dice en dos sitios:

> *«el mando SE CONSERVA en los canales `A` y `B` (`MANDO_A`=`BOTON1`=`PB9`=`J16` p5 ·
> `MANDO_B`=`BOTON2`=`PB13`=`J16` p8). Se retiran **solo** `BOTON3` (`PB14`, p10) y
> `BOTON4` (`PB15`, p12), que son los que las camaras necesitan y los que el mando no
> usa.»* — y en su tabla de descartes: *«**El veto de §2.4 se queda donde esta**»*.

**Y el motivo es exactamente §3.ter, que este repositorio ya tenia escrito sobre este mismo
getter:** `mando_ambarLocal()` tiene **CINCO** llamadas vivas —los tres vetos de
`Esclavo/src/main.cpp` (`:453`, `:476`, `:617`) y dos de `bluetooth.cpp` (`:551`, `:562`,
que deciden si `CANCELAR_AMBAR` contesta `RETIRADO` o `RETIRADO_QUEDA_MANDO`)—. Retirar el
armador de esa bandera vuelve los `if` siempre verdaderos y **el veto de SFTY-21 no queda
inerte: queda abierto**. Tocar eso para ganar flash es cambiar una proteccion de calzada
por unos bytes.

**Asi que el codigo del mando NO SE TOCA.** Se queda como esta, sin llamadores nuevos y sin
borrar nada. Que las lineas no respondan hoy (`0,6 V`, N-118) es un asunto de cobre, no una
razon para amputar el firmware.

### El reparto de `J16`, cerrado

| | pin | bornera | queda |
|---|---|---|---|
| `BOTON1` | `PB9` | p5 | **`MANDO_A`** — se conserva, **cableado**, hoy sin usar |
| `BOTON2` | `PB13` | p8 | **`MANDO_B`** — se conserva, **cableado**, hoy sin usar |
| `BOTON3` | `PB14` | p10 | **Camara** (`CAM_C_PIN`) — en firmware y medida en cobre |
| `BOTON4` | `PB15` | p12 | **Camara** (`CAM_D_PIN`) — idem |

🔴 Y sigue en pie lo que ya estaba: `MANDO_A` y `MANDO_B` **no responden** —`0,6 V` en
reposo, N-118—. **Van cableados**, y con `MANDO_B` al aire la bandera no se arma nunca.
Eso se cierra con multimetro y cable, no con un `git rm`.

> 🔴 **TACHADO EN LA MUDANZA (07/09) — la frase de arriba es FALSA hoy.** `05_Funcional/17_...md`
> §1.7 dice que `J16` **`p5` queda LIBRE Y SIN CABLEAR**, y la spec gana al roadmap en hardware
> medido (`CLAUDE.md` §9.bis). Se conserva sin borrar porque el parrafo que la contiene razona
> sobre ella.

### Lo que habria costado retirarlo, MEDIDO — el inventario alcanzo a terminar

El agente lanzado por error dejo hecho el censo antes de que lo pararan, y convierte
*«por los danos al software»* en una cifra. **No es que hubiera habido que retocar packs:
el banco se habria caido en ABORTADO, que es peor que en rojo** —§2: un ABORTADO no dice
nada del firmware—.

| | acoplamiento |
|---|---|
| `maestro_01_mando` | **15 comprobaciones**, y solo 3 sobreviven (son de `main.cpp`, no del mando) |
| `esclavo_01_latch_ambar` | **7** comprobaciones + 2 hallazgos, el pack entero |
| `esclavo_02_inhibicion_menu` | **7** comprobaciones, el pack entero |
| `camara_02_j16` | lee `Esclavo/src/mando.cpp` **directamente**, y censa los tres vetos |
| `documentos_04` | **relee constantes de `mando.cpp`** para su exencion de vocabulario |

**Y dos `raise fw.Abortado` que disparan solos:**

- `esclavo_07`: *«no se hallo en main.cpp ni una guarda con `mando_ambarLocal()`»* — salta en
  cuanto ese getter sale de `main.cpp`.
- `esclavo_08`: *«`mando.cpp` del Esclavo ya no consulta `degradado_gobiernaLuz()`»* —
  `mando.cpp` es **el molde** contra el que se mide el ambar de la app.

**Y el golpe de gracia:** `modelos/maestro.py` y `modelos/esclavo.py` leen **seis constantes
cada uno de `mando.cpp` a NIVEL DE MODULO** —`VENTANA_TRIPLE_MS`, `DESTELLOS_*`,
`RECHAZO_AMBAR_MS`—. Eso revienta **en el import**, o sea que se lleva por delante a todo
pack que importe el modelo, no solo a los que hablan del mando.

Ademas la **guarda de rutas** de `compuerta.py` censa la tupla `("Esclavo","src",
"mando.cpp")` por regex **sin saltarse los comentarios**: con el fichero borrado y la tupla
citada en cualquier `.py` -incluida la prosa de la propia `compuerta.py`-, aborta tambien.

> **La leccion, y es de metodo:** la decision del 31/08 decia *«por los danos al software»*
> sin numeros, y eso la hacia facil de derogar de palabra. **Un motivo medido no se deroga
> por descuido.** Es §2.ter aplicado a las decisiones: una razon escrita es una afirmacion
> sobre el codigo, y esta ahora esta comprobada.

### 🟡 Lo que SI queda abierto para asignar: `J14`, y la idea del fin de carrera

El responsable la planteo **sin decidir**: *«a lo mejor para fin de carrera de la barrera»*.

**Por que la idea vale, medido:** hoy la talanquera es **lazo abierto**.
`MOTOR_TALANQUERA` (`PB2` -> opto `U15` -> MOSFET `Q10` -> bornera `J15`) solo ESCRIBE
—`semaforo.cpp:100-102` y `:210-211`— y **nadie lee si la barrera se movio**. Si el brazo
se atasca a medias, el firmware cree que esta cerrada: un actuador en la via sin
confirmacion.

**Lo que NO esta decidido**, y hace falta antes de escribir una linea: de que pin cuelga
—con `A` y `B` conservados, `J16` ya no ofrece dos entradas libres—, si se ponen los dos
finales o uno, que hace el firmware cuando el brazo no llega al que espera, y si eso es un
`$ALARM` o un cambio de estado. Son decisiones del responsable.

---

## 0.0.sexdecies LOS TRES QUE SE CERRARON CON CODIGO — y lo que cada uno enseño de paso

> **Ninguno de los tres se parecia a lo que se creia que era.** Los tres se abrieron
> pensando que eran un arreglo pequeno y los tres destaparon algo mas grande al medirlos.

### N-152 · el Maestro ESTABA SORDO en `MODO_AMBAR` (`d6ce67e`)

El encargo era «copiar N-142 en la direccion contraria». Al censar aparecio que
`protocolo_hayPaqueteDisponible()` se llama **en UN SOLO SITIO de todo el Maestro**
—`coordinador_actualizar()`— y que `main.cpp:193` excluye `MODO_AMBAR` de la unica llamada
que queda en ese modo.

> **Un comando nuevo copiado de N-142 habria entrado por el UART SIN LECTOR.** El arreglo
> entero habria sido codigo declarado y no ejercido, con la compuerta en verde. Es §2.ter
> en el sitio donde mas caro sale: no en un `pinMode()` sin `digitalRead()`, sino en una
> trama de seguridad.

De ahi `coordinador_escucharEnAmbar()`: **callar es no transmitir, no quedarse sordo.**

**Y la pregunta que decidio el diseno:** al `MODO_AMBAR` se llega por cuatro caminos —el
`B.B.B`, el watchdog, `SET_MODO:AMBAR` desde este telefono, y el aviso del Esclavo— y los
cuatro hacian el **mismo** `modoActual_set()` sin dejar constancia. Sin distinguirlos, el
Poste 2 habria podido retirar un ambar que puso alguien que sigue en la calzada del Poste
1. El origen se anota **pegado al MOTIVO que ya se pintaba en pantalla**, porque es la
misma pregunta. **Gana quien esta aqui de pie.**

*De paso: la entrada de N-142 no fijaba motivo, asi que tras un `B.B.B` la pantalla decia
«Ambar pedido desde el mando (B.B.B)» para un ambar del Poste 2. **Mentia.***

Sale a **todo-rojo**, no al ciclo: el Esclavo dijo *«ya no retengo el ambar»*, no *«ya se
puede pasar»*.

### N-150 · el `$ACK` verde que no habia terminado nada (`414b962`)

`ACK_TEXTO` **no tenia entrada `SET_TIEMPOS|OK`**, asi que caia en el generico *«orden
ACEPTADA»* pintado **en verde**: justo la trampa que esa tabla existe para evitar.

**Lo que NO se hizo, y es la decision:** arrancar el ciclo al recibir el `$ACK` es lo
comodo, y **abre paso saltandose el aviso de via** —la unica barrera que obliga a levantar
la vista—. Se avisa y se le da la accion; la pulsa el operario. El boton **reenvia** al que
ya existe en vez de repetir su cuerpo, asi que pasa por las mismas tres barreras y el
literal vive en un solo sitio.

### Los parsers · eran TRES, y uno se probaba contra si mismo (`414b962`)

El tercero vivia en `test_unitarios_app.js`: checksum, parser y generador reimplementados
a mano, partiendo por **otro criterio** y acertando **por casualidad** sobre sus propias
tramas. Y `parseStatus()` no era solo de tests: la carga
`simulador_app_bluetooth.py`, lo que impedia retirarla sin dejar ciego a `documentos_03`
(N-89 otra vez).

**Defecto vivo encontrado de paso:** `parseError()` leia **por posicion**. Sobre la trama
real `ERR,CMD:SET_TIEMPOS,DESC:RANGO` devolvia `cmd = "CMD:SET_TIEMPOS"`. Estaba en verde
porque su unica prueba le daba un formato **que ningun micro emite**.

### Y los dos packs nuevos cazaron defectos DE SI MISMOS

No por lectura: por sus propios controles negativos (§8.bis funcionando).

- `costura_14` tenia el **buscador ciego** —`_llamadas_stmt()` excluye parentesis dentro de
  los argumentos y los motivos llevan `"el mando (B.B.B)"`—: daba `FALLA` acusando al
  firmware de un agujero que no tenia.
- Y su comprobacion 4 leia `modoManual_loop()` en vez de `modoManual_setup()`: hay **dos**
  `switch` sobre el modo en `main.cpp` y casaban con el mismo patron. **Aprobaba en verde
  midiendo la funcion equivocada.**

### 🔴 Lo que se dejo ESCRITO y SIN ARREGLAR, a proposito

**`validateTiempos()` de los unitarios sigue en 1..15 min** cuando el C++ y `app.js` estan
en **3..15**. No falla porque **ninguno de sus siete casos toca el borde**: no prueba ni 1
ni 2. Es una copia vieja **que no puede fallar** — la clase que §3.bis llama prueba muerta,
y la peor, porque lleva la palabra «probado» encima. Invertirla es un §8.quater aparte.

### Y lo que ninguno de los tres es

**Nada de esto ha tocado una tarjeta.** N-150, N-151 y N-152 se arreglaron **despues** de
la ultima cinta de banco. El `20/20` dice que los modelos y los arneses de PC no encuentran
nada, y esta misma sesion dio dos contraejemplos: N-146 y N-147 pasaron esas veinte
comprobaciones sin despeinarlas, y los encontro una cinta de tramas.

---

## 0.0.quindecies PLAN DE CIERRE — que cierra cada pendiente, y cual NO lo cierra nadie desde el PC

> **La mitad util de este plan es la segunda tabla.** Cuatro de los ocho pendientes **no
> se pueden cerrar escribiendo codigo**, y confundirlos con los otros cuatro es como se
> acumula un `20/20` que no acerca una tarjeta (§2.bis).

### Los que SI se cierran con firmware o app

| | que es | que lo cierra | estado |
|---|---|---|---|
| **N-150** | al aplicar tiempos el cruce se queda en rojo para siempre | aviso colgado del `$ACK` **y una accion explicita**. **No se arranca solo**: abrir paso saltandose el aviso de via es lo que §6 prohibe | ✅ `414b962` |
| **N-152** | `CANCELAR_AMBAR` no manda nada por radio | el aviso de vuelta, **y no era copiar N-142** | ✅ `d6ce67e` |
| **los parsers** | `nmea_parser.js` escribe `data.hora`, `app.js` lee `data.HORA` | un solo partidor, y un pack que exige que la probada sea la usada | ✅ `414b962` — **eran TRES** |

### 🛑 Los que NO cierra ningun agente, y por que

| | que falta de verdad |
|---|---|
| **`MANDO_A` / `MANDO_B` a `0,6 V`** (N-118) | **una medida en cobre y probablemente un cable.** `J16` p5 y p8 van cableados y no responden. No es firmware: el fuente ya lee `INPUT` pelado activo en ALTO, que es lo que el conector pide. Y **urge**: con `MANDO_B` al aire `mando_ambarLocal()` no se arma nunca, los tres `if` de `Esclavo/src/main.cpp` quedan siempre verdaderos y **el veto de SFTY-21 no queda inerte: queda ABIERTO** |
| **N-145 · la hora** | **comprar el `DS3231`** (linea `A6`) y verificar `0x68` sobre el modulo. El firmware esta entero en las dos mitades. Sin la pieza, las tramas salen con `--:--:--` y eso es el arreglo **callandose bien**: no confundirlo con que falle |
| **`BAT:--`** | **un divisor de tension y una entrada analogica.** La causa esta MEDIDA: `grep -rn analogRead` sobre las cuatro carpetas da **cero**, y N-108 puso el `--` a proposito para que nadie leyera un 12,6 V que era un literal |
| **matriculacion por ID de Bluetooth** | **una decision de protocolo del responsable, y cuesta bytes.** `RF_Packet` son 4 bytes `{msgID, command, param, crc}` y **no tiene campo de direccion**; el CRC cubre 3. Meter direccionamiento cambia el contrato de la radio en las dos puntas. Aplazado por el a despues del banco |

> 🔴 **TACHADO EN LA MUDANZA (07/09).** La fila `MANDO_A`/`MANDO_B` de arriba esta **REFUTADA**
> (`d020f3c`, y tachada en `DECISIONES.md`): mismo cobre, distinto `pinMode`, distinta tension.
> No hacia falta «una medida en cobre y probablemente un cable».

### Y lo que este plan NO promete

**Nada de lo cerrado esta noche ha pasado banco.** N-151 y N-152 se arreglaron **despues**
de la ultima cinta, asi que de ellos no hay una sola prueba en tarjeta. El `20/20` dice
que los modelos y los arneses de PC no encuentran nada — y esta misma sesion dio dos
contraejemplos: N-146 y N-147 pasaron esas veinte comprobaciones sin despeinarlas, y los
encontro una cinta de tramas.

---

## 0.0.quaterdecies CIERRE DE LA SESION DE BANCO DEL 04-05/09 — lo que la CINTA cerro y lo que dejo abierto

> **Los cinco defectos de esta sesion salieron de una CINTA DE TRAMAS y de un DIARIO DE
> ORDENES, no de una revision.** Los instrumentos de PC estaban en `20/20` mientras dos
> de ellos vivian dentro. Es §2.ter con fecha nueva.

### CONFIRMADO EN COBRE — la cinta del 05/09 a las 22:19, con `42a52cd` cargado

| | la evidencia, literal |
|---|---|
| **N-146** | `$ACK,CMD:SET_MODO:AMBAR,RESULT:REARMADO` a las 22:19:40, con el `ESTADO` pasando de `ROJO` a `FALLO COM` |
| **N-149** | `ESC:AMBAR` y `ESC:ROJO` viajando en todos los `$STATUS` del Maestro |
| **N-145** | `HORA:22:19:58` — el campo dejo de ser `--:--:--` |

### N-151 — DAR PASO EN UN MODO SIN COORDINADOR TRABABA EL CRUCE PARA SIEMPRE

**Lo que la cinta enseña, y el sintoma no se parecia a la causa:** el equipo en
`MODO:AMBAR` y **tres `MANUAL:CAMBIAR_TURNO` en 40 s**, los tres contestados
`$ERR,CMD:CAMBIAR_TURNO,DESC:EN_TRANSICION_REINTENTE`. El operario leia *«el cruce esta
cambiando de fase, repita al terminar»* y **el cambio no terminaba nunca**.

**La causa, medida:** `main.cpp` EXCLUYE a `MODO_AMBAR` y a `MODO_DEGRADADO` del refresco
de fondo —en esos dos el Maestro calla en la radio **a proposito**— y sus `loop()` no
llaman al coordinador. Alli la maquina esta **congelada**. La PRIMERA pulsacion si entraba
—`estadoC` valia `C_IDLE`— y dejaba el coordinador en una transicion que **ya no avanza**.
Desde ahi, todas las demas caian en el `if (estadoC != C_IDLE) return;` **hasta que
alguien cambiara de modo**.

El equipo dijo que **si** a una orden que no iba a ejecutar, **y se quedo peor que antes
de pedirla**. Es la barrera de salidas (§6) en su forma mas cara, y el mensaje era encima
mentiroso: no habia ninguna transicion en curso.

**Arreglo:** se rechaza de entrada con `MODO_SIN_CICLO_SALGA_PRIMERO`. El modo se mira
**antes** que el estado del coordinador, y el orden importa: al reves, la primera
pulsacion en ambar volveria a colarse y a trabar el cruce.

**Instrumento:** `maestro_12_dar_paso_sin_coordinador`. No nombra ni un modo a mano: lee
la lista de excluidos de la condicion de `main.cpp`, resuelve el `loop()` de cada uno por
el `switch`, mira cual de ellos llama al coordinador, y exige que la guarda del
despachador rechace **exactamente** los congelados. Con su mitad positiva —que no rechace
de mas, o dejaria el boton muerto en Manual— y dos controles negativos que inyectan los
dos defectos.

### La linea FALSA de `pines.h`, encontrada revisando las spec y no por un test

`pines.h:98-99` decia que `BOTON1`/`BOTON2` son `INPUT_PULLUP` **activos en BAJO**. El
fuente hace `pinMode(INPUT)` pelado (`botones.cpp:160-161`) y lee `== HIGH`
(`botones.cpp:40`) — **justo lo contrario**. Y lo contradecia el texto de N-118 **treinta
lineas mas abajo del mismo fichero**, mas la medida en cobre del 03/09.

Es la cabecera que todo el mundo lee **antes de cablear**. Los comentarios no compilan.

### 🔴 LO QUE QUEDA ABIERTO — el orden es el de lo que duele

1. **N-150 · al aplicar tiempos el cruce se queda en rojo para siempre.** Para fijar
   tiempos hay que sacar el equipo del ciclo y la app manda al **menu**; alli el
   coordinador manda `CMD_GO_RED` cada 3 s, que es su trabajo. Y al aceptarlos **nadie
   vuelve a arrancar el ciclo**. **Decision pendiente:** que la app lo arranque sola es
   comodo pero **abre paso sin que nadie lo pulse**, y eso es lo que §6 prohibe.
2. **`CANCELAR_AMBAR` no manda nada por radio** — el hermano de N-142 en la otra
   direccion. Cancelar desde el Poste 2 deja al Maestro en ambar.
3. **`MANDO_A`/`MANDO_B` a `0,6 V`** (N-118). Con `MANDO_B` al aire el veto de SFTY-21
   **no queda inerte: queda ABIERTO**.
4. **N-145 sin poder probarse**: no hay `DS3231` comprado (`A6`) y `0x68` sigue SIN
   VERIFICAR sobre modulo real.
5. **Matriculacion por ID de Bluetooth.** `RF_Packet` = 4 bytes **sin campo de
   direccion**; el CRC cubre 3.
6. **Dos parsers en la app**: `nmea_parser.js` escribe `data.hora`, `app.js` lee
   `data.HORA`. **La que se prueba no es la que se instala.**
7. **`BAT:--` — y la causa SI esta medida**, al contrario de lo que se escribio antes en
   esta misma sesion: N-108 lo puso en `--` a proposito porque `grep -rn analogRead` da
   **cero** en las cuatro carpetas. Falta el divisor, no el firmware. *(Corregido: se
   habia anotado como «hallazgo sin causa», y lo refuto un agente midiendo.)*
8. **`J16` p1 lleva 12 V crudos**: taparlo es obligatorio en cada equipo (N-120).

> 🔴 **TACHADO EN LA MUDANZA (07/09).** El punto 3 de arriba —`MANDO_A`/`MANDO_B` a `0,6 V`—
> esta **REFUTADO** desde el 05/09 en `d020f3c`: los `0,6 V` eran el firmware viejo
> (`INPUT_PULLUP` contra los 10 K del cobre), no un defecto de placa. Y es moot: `D-1` retiro
> el mando. **No era «lo mas urgente»: no era nada.**

### Decision del responsable, tomada esta noche

**En Manual, `DAR PASO` alterna rojo/verde como el automatico, disparado por el boton.**
Termina en **rojo+verde**, no en rojo+ambar. El todo-rojo de despeje entre un verde y el
siguiente **se queda** —el automatico tambien lo hace— y es lo que garantiza que el tramo
quedo vacio. Configurable de 10 a 90 s, hoy en 15.

---

## 0.0.bis · 🔴 REPORTE DE CAMPO DEL 04/09, TARDE — LAS DOS PLACAS YA HABLAN, Y APARECIO N-42 ENTERO

**Lo bueno primero: hay comunicacion entre las dos placas.** Es la primera vez. El bloqueo fisico de
la segunda tarjeta que este documento daba por vigente **ya no existe**.

**Lo reportado, literal:** *"los cuatro modos de la ventana inicial no funcionan adecuadamente. En el
panel inicial a veces cuando pongo ambar los dos pasan a ambar, o a veces solo el maestro o solo el
esclavo; igualmente para rojo total. Y en modo automatico tampoco funciona: sale maestro rojo, este
poste no da paso, poste dos no informa. Y a veces el esclavo se pasa a ambar parpadeando solo, pero
no por falta de comunicacion porque el maestro queda en rojo."*

**No son cuatro fallos: son DOS causas, y las dos estan medidas en el fuente.**

#### CAUSA 1 — el Maestro se queda MUDO en Modo Automatico (esto es N-42, diagnosticado)

```
app: SET_MODO:AUTO
  -> bluetooth.cpp:445  modoActual_set(MODO_AUTOMATICO)
  -> bluetooth.cpp:447  "$ACK,CMD:SET_MODO:AUTO,RESULT:OK"      <- ya dijo que si
  -> main.cpp:204       modoAutomatico_setup()
       |
       +- if (arranqueDirecto) -> CORRIENDO      (solo lo pone mando.cpp:112, el mando de reles)
       +- else fase = CONFIG_ROJO                 <- se queda aqui PARA SIEMPRE
              unica salida: botonAceptar()
              botones.cpp:305  ->  return false;  <- SIEMPRE, desde deeeab4 (31/08)
```

Y como nunca llega a `CORRIENDO`:

- **`coordinador_actualizar()` vive DENTRO de `case CORRIENDO`** (`modo_automatico.cpp:181`), asi que
  no se ejecuta;
- **y `main.cpp:185` EXCLUYE a `MODO_AUTOMATICO` del respaldo de fondo**, con el comentario *"ya se
  llama en modo_automatico.cpp"* — que es cierto sobre el papel y falso en ejecucion.

**Resultado: en Modo Automatico el Maestro no emite ni un `PING`.** De ahi salen dos de los sintomas
reportados con una sola causa:

| lo que se ve en banco | por que |
|---|---|
| *"maestro rojo, este poste no da paso"* | nadie ejecuta el ciclo: las luces no se mueven |
| *"el esclavo se pasa a ambar parpadeando solo"* | lleva **25 s** sin oir al Maestro (`SFTY6_SILENCIO_MS = 25000`) y se va a ambar por orfandad. **Esta haciendo lo correcto** |

> 🔴 **Y hay que corregir una lectura del reporte, porque es la que despista:** *"no por falta de
> comunicacion porque el maestro queda en rojo"*. **SI es falta de comunicacion.** El Maestro esta
> VIVO pero no esta HABLANDO, y son dos cosas distintas. Verlo encendido y en rojo no dice que este
> emitiendo. La radio esta bien; el que calla es el.

**El origen es el commit `deeeab4` (31/08), *"las camaras entran por C y D"*.** Al reconvertir
`BOTON3`/`BOTON4` en entradas de camara, `botonAceptar()` paso a `return false` **y nadie censo quien
dependia de que pudiera ser CIERTA**. Es literalmente §3.ter de `CLAUDE.md`, la regla que se escribio
por `mando_ambarLocal()`. Volvio a pasar, y esta vez se llevo el Modo Automatico entero. En el
firmware de campo (`e303485`, 31/07) esa funcion leia el boton de verdad: por eso alli funcionaba.

#### CAUSA 2 — el ambar y el rojo total van a UNA sola punta

No hay propagacion disenada. `SET_MODO:AMBAR` es del Maestro; `AMBAR_EMERGENCIA` es del Esclavo;
`FORZAR_ROJO` existe en las dos pero **solo afecta a aquella a la que estas conectado**. Y cuando el
Maestro entra en `MODO_AMBAR`, `main.cpp:185` **tambien lo excluye** de hablar por radio, asi que el
Esclavo se queda huerfano y a los **25 s** se va a ambar por su cuenta.

**Por eso "a veces los dos, a veces solo uno": depende de a que poste estabas conectado y de cuanto
esperaste.** Antes de 25 s ves uno; despues ves los dos — pero el segundo no obedecio una orden: se
quedo huerfano.

#### Y una tercera, que el responsable habia visto antes que nadie

***"poste dos: no informa"*** — el `$STATUS` es
`$STATUS,NODE:MAESTRO,SERIE:..,MODO:..,ESTADO:..,T:..,RF:..,RTT:..,BAT:--,HORA:..`: **un solo
`ESTADO`, el del que la manda.** No hay campo para la otra punta. Y sin embargo el Maestro **si lo
sabe**: recibe `CMD_ACK_GREEN` / `CMD_ACK_RED` del Esclavo en cada cambio (`coordinador.cpp:780` y
`:805`). Lo usa y lo tira. **`SIN DATOS` es honesto dada la trama, y la trama esta incompleta.**

> Matiz que decide como se escribe el campo: el Maestro sabe **lo ultimo que el Esclavo confirmo**, no
> lo que su lampara ensena ahora — el operario con el mando local tiene **veto** sobre las ordenes de
> radio. El campo correcto es *"lo ultimo confirmado, y hace cuanto"*, no *"estado del Esclavo"*.

#### N-133 — 🔴 LOS TIEMPOS DEL CICLO NO SE GUARDAN EN NINGUN SITIO

Lo destapo una pregunta del responsable: *"una cosa es parametrizar al inicio, luego deberia
funcionar"*. **Hoy no se cumple.** Censado: el respaldo guarda `verdeSeg`, `despejeSeg` y
`horasDesdeSync`, y **los dos primeros son del Modo Degradado** (`respaldo.h:47`). Los del Automatico
viven **solo en RAM**:

```
arranque              ->  3 / 3 / 10   (los minimos)
SET_TIEMPOS 8/8/20    ->  8 / 8 / 20   $ACK OK
SET_MODO:AUTO         ->  3 / 3 / 10   modoAutomatico_setup() los reescribe
corte de luz          ->  3 / 3 / 10   nunca se guardaron
```

**Y esto es deuda propia:** el hallazgo se levanto por la manana, se arreglo **el valor** al que
vuelven (de 1 a 3, N-131) y **no se arreglo que vuelvan**. Se anoto como abierto en vez de cerrarse.

### 0.0.ter 🟢 LO ARREGLADO LA NOCHE DEL 04/09, y un defecto PROPIO que casi entra

| | que | estado |
|---|---|---|
| **N-42** | el Modo Automatico no movia las luces **y dejaba al Maestro MUDO en la radio**. Se retira el asistente entero: una sola puerta, el modo arranca corriendo | 🟢 cerrado, `ceb8cc5` |
| **N-133** | los tiempos del ciclo no se guardaban en ningun sitio. Van al respaldo (`DR9`/`DR10`), dentro del checksum, `FIRMA` a `0x5EB2` | 🟢 cerrado |
| **N-134** | el ambar se ORDENA, no se deduce. `CMD_GO_AMBAR` (`0x13`); el rojo previo se queda como intermedio seguro y **la orfandad de 25 s se queda como red** -decision del responsable- | 🟢 cerrado |
| **N-135** | 🔴 **defecto PROPIO, introducido por el arreglo de N-42 y cazado horas despues** | 🟢 cerrado |

#### N-135 — un `enum` de un solo valor no es un estado, y cerro la puerta de N-133

Al retirar las tres fases del asistente quedo `enum FaseAuto { CORRIENDO };` con su
`static FaseAuto fase;`, y un comentario **escrito en el mismo commit** diciendo que el
enum sobrevivia porque `enMarcha()` *"se lee mejor preguntando por la fase"*. Se leia
mejor y **ya no preguntaba nada**. Medido con el compilador, no razonado:

```
arm-none-eabi-g++ -Os -S -mcpu=cortex-m3 -mthumb
enMarcha:  movs r0, #1
           bx   lr
```

De esa funcion cuelgan las dos guardas de `SET_TIEMPOS`, asi que el equipo contestaba
`EN_MARCHA_PARE_EL_MODO` **a todo y para siempre**; y como `fijarTiempos()` es el UNICO
llamador de `respaldo_guardarTiemposCiclo()`, **N-133 se quedo con camino de lectura y sin
camino de escritura**. Un arreglo cerro la puerta del otro el mismo dia.

> 🔴 **Lo encontro un agente que fue a comprobar si el paso de banco era ejecutable, y
> lo encontro COMPILANDO. La compuerta estaba en verde.** Ni la compuerta ni los 68 packs
> lo vieron: `maestro_10` censaba funciones que devuelven un LITERAL, y esta devolvia una
> COMPARACION. La forma distinta, la consecuencia identica.

Arreglado: `enMarcha()` pregunta por el MODO -`modoActual_get() == MODO_AUTOMATICO`-, que
no puede degenerar. Se retiran el enum y la variable. La regla queda en `CLAUDE.md`
**§3.septies**, y `maestro_10` gana la hermana del censo: **un enum de un solo valor que
ademas se COMPARA**.

#### Lo que el reporte de campo de la noche confirmo o refuto

| lo reportado | veredicto |
|---|---|
| *"rojo total se cambia de una, de una"* | ✅ **propaga bien**. Refuta lo que yo habia escrito: `coordinador_forzarRojoTotal()` manda `CMD_GO_RED` (`coordinador.cpp:558`) |
| *"le vuelvo a ambar, ese cambia pero este no"* | confirmado → **N-134** |
| *"se retardo 25 segundos"*, *"27 segundos el poste"* | la orfandad, exacta. Confirma N-42 |
| *"rechazo por el equipo, formato invalido"* en el Courier RTC | 🔴 **SIN DIAGNOSTICAR.** El formato cuadra por los dos lados sobre el papel. **No se puede saber porque la cinta de tramas solo graba lo que ENTRA**: 300 tramas y ninguna es la que se mando |

#### 🔴 Lo que ese ultimo caso deja escrito, y vale mas que el propio fallo

**La cinta de tramas no graba lo que la app ENVIA.** Se perdieron veinte minutos deduciendo
un formato por los dos lados en vez de leerlo. Es la regla del instrumento en su forma mas
cara, y el responsable lo dijo mejor: *"cada comando debe ser capturado en los logs, asi
sabras que envia, que responde, que se activa, sin mas"*.

Y no basta con grabar los tres: hay que **poder verlos juntos**. La terna

```
ORDEN      CMD:PIN:****:SET_MODO:AUTO
RESPUESTA  $ACK,CMD:SET_MODO:AUTO,RESULT:OK
EFECTO     x MODO:AUTO pero ESTADO:ROJO durante 45 s - nada cambio
```

**es N-42 visible de un vistazo, sin diagnostico y sin reunion.** Va como DIARIO DE
ORDENES aparte de la cinta cruda -que se corta a 300 tramas y en una sesion ya se tiraron
379-. Y el PIN sale **tapado** al exportar: hoy el `1234` viaja en claro dentro de cada
trama que se manda por WhatsApp.

### 0.0.terdecies · LA NOCHE DEL 04 AL 05/09 — tres defectos de calle cerrados, y lo que la CINTA dejo abierto

**Los tres salieron de la sesion de banco del 04/09 por la noche y de la cinta de tramas que
dejo.** Ninguno es una propiedad del fuente que un pack hubiera encontrado solo: dos los reporto
el responsable operando el equipo, y el tercero lo destapo el diario de ordenes.

| | que | estado |
|---|---|---|
| **N-142** | el Esclavo se iba a su ambar de emergencia y **el Maestro no se enteraba**: podia seguir dando VERDE hasta 3 minutos con el otro lado en ambar | 🟢 cerrado, `6274acc` |
| **N-146** | `SET_MODO:AMBAR` contestaba `RESULT:OK` y **no encendia nada** | 🟢 cerrado, `8e9e8a9` |
| **N-147** | en Modo Manual el equipo hacia **un ciclo que nadie pidio** | 🟢 cerrado, `8e9e8a9` |

> 🔴 **Ninguno de los tres ha tocado una tarjeta.** Estan compilados y con el banco por packs
> encima; eso dice lo de siempre —que los modelos y los arneses de PC no encuentran nada—. La
> siguiente carga es la que decide.

#### N-142 — el Esclavo AVISA por radio, y los dos vetos SE QUEDAN

**El defecto, medido entero en `0.0.octies`:** el ambar de emergencia del Esclavo engancha un
latch, y con el latch puesto esa punta **no obedece NI ACUSA** `CMD_GO_RED` ni `CMD_GO_GREEN`. El
Maestro agotaba reintentos a ciegas y caia a `C_FALLO`, desde donde rechaza todo. Y peor: el
Esclavo en ambar **sigue contestando PONG**, asi que el enlace le parecia perfecto al Maestro. Si
el Maestro estaba en VERDE cuando se engancho el ambar, **hasta 3 minutos con los dos sentidos
pudiendo entrar al carril**.

**Lo hecho:** `CMD_AMBAR_ESCLAVO` (`0x14`). El Esclavo lo manda al armar el latch, sin esperar
acuse y sin reintento —su operario esta delante—, igual que `CMD_GO_AMBAR`. El coordinador lo
anota y `main.cpp` lo consume para entrar en `MODO_AMBAR`: la respuesta es un cambio de MODO, y el
modo no lo decide la maquina del ciclo. Se consume al leerlo —es un aviso, no un estado— o el
Maestro no podria salir nunca.

> 🔴 **LA MITAD QUE MAS VALE, Y ES LA QUE NO SE HIZO: EL BANCO PARO LA DECISION (b) DOS VECES.**
>
> Se iba a quitar el veto del ambar de la app de las guardas del Esclavo para desatascar el cruce.
> El pack `esclavo_07` lo tumbo:
>
> ```
> "el ambar de la app dura hasta el siguiente latido del Maestro -unos 3 s- y el
>  operario ve el equipo obedecer y volverse atras solo"
> ```
>
> La segunda version dejaba el veto solo en `CMD_GO_GREEN` —lo que ABRE paso— y lo abria en
> `CMD_GO_RED`. Tambien se cae: con el rojo entrando, el ambar que pidio el operario se convierte
> en rojo a los 3 s. Mas seguro, pero **no es lo que pidio, y lo ve deshacerse delante**.
>
> **Y al medirlo aparecio que el veto NO ERA LA CAUSA DEL BLOQUEO.** La causa es que esa punta **no
> ACUSA**, y el silencio es deliberado y correcto —acusar un rojo que no se ha encendido dejaria al
> Maestro dando verde convencido de que aqui hay rojo—, pero **obligaba al Maestro a ADIVINAR**. Con
> el aviso ya no adivina: se va a `MODO_AMBAR`, deja de ciclar y **deja de preguntar**, asi que no
> hay reintentos, no hay `C_FALLO` y no hay bloqueo. **Los dos vetos se quedan enteros** —son lo que
> protege a quien esta en la calzada— y lo que desaparece es la ceguera del otro extremo.
>
> Es §8.bis en su forma util: el instrumento no aprobo el arreglo, **lo rechazo dos veces y mando a
> medir**. La causa que se creia obvia era falsa.

#### N-146 — seis `OK` seguidos y el cruce sin moverse: lo destapo una CINTA, no una lectura

**De donde salio:** la cinta de tramas del 04/09 a las **21:10**. Seis
`CMD:PIN:****:SET_MODO:AMBAR` seguidos, los seis con `$ACK,CMD:SET_MODO:AMBAR,RESULT:OK`, y el
`$STATUS` de despues diciendo `MODO:AMBAR,ESTADO:ROJO` durante **47 tramas**. El operario pulso
seis veces en tres minutos porque el cruce no se movia, y el equipo le dijo que si las seis.

**La causa, medida:** entrar en el ambar es trabajo de `modo_ambar_setup()`, y `main.cpp` **solo lo
llama EN EL FLANCO** de cambio de modo. Con el modo ya en `MODO_AMBAR` no hay flanco, asi que el
`modoActual_set()` de la rama no hace absolutamente nada.

**Y al par (`MODO_AMBAR`, luz en rojo) se llega por un camino NORMAL, no por un fallo:**
`CMD:FORZAR_ROJO` llama a `coordinador_forzarRojoTotal()`, que cambia **la LUZ y no el MODO** —a
proposito: el rojo de emergencia entra sin PIN desde cualquier modo—. Un ROJO TOTAL despues de un
ambar deja exactamente ese par, y a partir de ahi **el boton de ambar queda muerto para siempre sin
decirlo**.

**Lo hecho:** se re-arma, y **se contesta `RESULT:REARMADO`, distinto de `OK`**, porque son dos
cosas distintas y el diario de ordenes las tiene que poder separar. Es la barrera de salidas
(`CLAUDE.md` §6): un `$ACK` que no depende de lo que se hizo es una mentira con formato de exito, y
aqui ademas la mentira tapaba **una salida de emergencia**. Re-armar no es gratis —manda un
todo-rojo y vuelve a ordenar el ambar—, y por eso **no** se hace desde el aviso del Esclavo
(N-142), que llega repetido: aqui lo pide una persona pulsando un boton, y repetirlo es exactamente
lo que quiere.

#### N-147 — Manual entraba por LA PUERTA DEL AUTOMATICO

**Lo reportado desde el banco, y son dos mitades del mismo numero:** *"el boton dar paso maestro
queda en rojo, pasan 15 seg y ... pasa a ambar intermitente"*.

`modoManual_setup()` llamaba a `coordinador_iniciarModo()`, que es **la entrada del Modo
Automatico**: deja el coordinador en `C_INICIAL_ESPERA_ESTATICO`, o sea **con un verde ya
programado** para dentro de `tiempoDespejeMs`. De ahi salen las dos mitades:

1. **`DAR PASO` no hace nada durante ese plazo.** `coordinador_pedirCambio()` abre con
   `if (estadoC != C_IDLE) return;`, asi que la orden se rechaza —`EN_TRANSICION_REINTENTE`— y el
   cruce se queda en rojo.
2. **Al vencer el plazo el cruce cambia SOLO**, sin que nadie haya pulsado.

**Los "15 segundos" del reporte son literales:** `tiempoDespejeMs` vale **15000 ms** por defecto, y
ese ambar es la transicion rojo -> AMBAR 4 s -> verde que el propio Maestro arranca al vencer el
plazo. El equipo estaba haciendo un ciclo que nadie pidio.

**Y debajo habia un tercero que nadie habia reportado:** el `case QV_NINGUNO` de
`coordinador_pedirCambio()` hacia `tRef = millis()` en **cada pulsacion**. Un operario que pulsa
cada 10 s con el despeje en 15 s **no ve el verde nunca**, y cada pulsacion le contesta `OK`. Es la
peor forma de fallar: obedecer y no avanzar. Ademas cobraba un despeje **ya pagado**: a ese `case`
se llega solo con el cruce en todo-rojo, y en Manual ese rojo lleva puesto desde que se entro al
modo.

**Lo hecho:** Manual entra por `coordinador_forzarRojoTotal()` —el **mismo** todo-rojo, misma luz,
mismo `CMD_GO_RED`, mismo reset de replay— que termina en `C_IDLE` con `quienVerde` en
`QV_NINGUNO`: el cruce parado **sin plazo ninguno** y la primera pulsacion aceptada. Y el despeje ya
cumplido no se vuelve a cobrar.

> **SFTY-4 NO se debilita, y esta es la linea que hay que releer si alguien vuelve aqui:** los otros
> dos `case` —`QV_MASTER` y `QV_ESCLAVO`— son los que van de un VERDE a otro, y **esos siguen
> pasando por su rojo y su `C_ESPERA_ESTATICO_*` como siempre**. Lo que se deja de hacer es cobrar
> dos veces un despeje que ya corrio con el cruce parado.

**La definicion del modo la fijo el responsable el 04/09:** *"en manual, dar paso es simplemente el
operador le da y cambia... el operador en manual no deberia llevar un ciclo, sino que, como esta
ahi parado viendolo, que se cambie de inmediato"*.

#### Lo que esta noche deja ABIERTO

| # | que | de quien es |
|---|---|---|
| **N-145** | el campo `HORA:` del `$STATUS` lo rellena el **STM32**, el micro **sin reloj**. Ver `0.0.undecies` | agente sobre el ESP32 |
| **N-148** | la app **no pide confirmacion de via** al dar ambar en Manual; en `DAR PASO` si | agente sobre la app |
| **N-149** | el `$STATUS` del Maestro **no traia nada del Esclavo**. Firmware hecho en `8e9e8a9`; **dejo `simulador de app y bluetooth` en ABORTADO** -no sabe con que comparar el campo `ESC`-, y un ABORTADO es una puerta abierta (§3.quater) | firmware hecho, **instrumento por arreglar** |
| **BAT:--** | la bateria no se mide nunca | **sin causa medida** |
| **Matriculacion** | emparejar por **ID de Bluetooth**, no por nombre, y sin mano | aplazado tras el banco |

##### N-149 — el `$STATUS` del Maestro no traia NADA del Esclavo

**Verificado en la cinta del 04/09: ningun campo.** Ni estado, ni modo, ni nada de la otra punta.
El responsable, delante del equipo: *"cuando me conecto al maestro no me aparecen los estados del
semaforo del esclavo... yo necesito que maestro me traiga los datos del esclavo"*. Y sobre la
alternativa que la app ofrecia —un boton para conectarse por Bluetooth al otro poste—: *"tendrias
que caminar 1000 metros hasta el otro lado"*. **Un cruce se opera desde un sitio o no se opera.**

Anadido en `8e9e8a9` un campo **`ESC:<ROJO|VERDE|AMBAR|?>`** al `$STATUS` del Maestro. **Sin banco**, y con un instrumento en ABORTADO detras -ver la tabla de arriba-. Lo que hay que
dejar escrito porque es el diseno entero: **la fuente es `quienVerde`, que NO se pone por haber
MANDADO una orden sino al recibir el ACUSE**, asi que lo que se publica es **lo que la otra punta
confirmo**. El `?` no es un hueco: es la respuesta correcta con el enlace caido. Publicar la orden
seria pintarle al operario un semaforo que quiza no existe, y este repositorio ya lo pago dos veces
—el `12,6 V` de bateria que era un literal (N-108) y el equipo declarandose en hora con el reloj en
ceros (N-144)—.

##### BAT:-- en todas las tramas de la cinta

**El hallazgo, y nada mas que el hallazgo: en la cinta del 04/09 el campo `BAT:` sale `--` en todas
las tramas. La bateria no se mide nunca.**

🔴 **No se escribe aqui ninguna causa, porque no se ha medido ninguna.** `CLAUDE.md` §4 lo prohibe
expresamente, y este roadmap ya pago una causa *"plausible y falsa"* con la palabra «medido»
encima. Lo que hay que hacer antes de proponer nada es mirar si existe divisor y entrada analogica
que lo lea, y eso es una medida sobre la tarjeta, no una lectura de codigo.

##### Y un hallazgo sobre el propio vigilante, medido al escribir esto

`documentos_05_copias_coherentes` caza *"estados de compuerta imposibles"* con esta regla:

```python
malas = [(p, f, e) for p, f, e in rx.findall(t) if (int(f) > 0) != (e != "0")]
```

O sea: **da por imposible cualquier codigo de salida distinto de `0` sin un `FALLA` delante.** Pero
la compuerta tiene **tres** codigos —`0` PASS, `1` FALLA, `2` ABORTADO (`CLAUDE.md` §3)—, y el acta
del 05/09 es justo el caso que la regla no contempla: **19 PASS | 0 FALLA | 1 ABORTADO, exit `2`**.
Al escribir esa linea verdadera en `ESTADO.md`, el pack la acuso de estar escrita a mano.

**No es un fallo grave y no se ha tocado el pack** —esta fuera del encargo de esta pasada, y
tocarlo sin verlo fallar seria ajustar el instrumento hasta que de verde—. Se anota porque es
§4.quinquies otra vez: **el instrumento compara contra un borde -«exit 0 o hay FALLA»- y ese borde
no es el que importa**, porque ignora el tercer codigo. Mientras siga asi, un `ABORTADO` publicado
honestamente con su exit `2` no se puede escribir en los tres documentos.

##### MATRICULACION / EMPAREJAMIENTO — aplazado, con el requisito ya definido

**Decision del responsable (04/09):** *"detras del banco, primero cierra lo del banco"*. No se toca
hasta que el banco este cerrado.

**El requisito lo define su propia frase, y conviene copiarla entera porque descarta la solucion
facil:** *"el escucho los nombres... es pura mierda. Tienes que ir a mirar que nombre tiene y cual
es el ID del Bluetooth, y luego si matricularlo como maestro y como esclavo, no hacerlo a la
mano"*. O sea: **por ID de Bluetooth, no por nombre, y sin intervencion manual.**

> 🔴 **El dato tecnico duro que condiciona el diseno, y por eso se deja escrito hoy: `RF_Packet` son
> 4 bytes `{msgID, command, param, crc}` y NO TIENE CAMPO DE DIRECCION.** El CRC cubre 3 bytes. Una
> matriculacion que necesite decir *a quien* va dirigida una trama de radio **no cabe en la trama de
> hoy**: o se cambia el formato —y con el, las dos puntas, el repetidor y todos los packs que lo
> parsean— o la matriculacion vive solo del lado Bluetooth y la radio sigue siendo punto a punto.
> Esa eleccion es de diseno y no esta hecha.

### 0.0.quater · LO QUE SE DECIDIO SOBRE LA AUTORIZACION, y las pruebas que hubo que repartir

**Decision del responsable (04/09): EL OPERARIO DEJA DE TECLEAR EL PIN para lo que ABRE
paso.** Lo sustituye una confirmacion: *"¿Confirma que no quedan vehiculos en el tramo?"*.

El motivo no es comodidad, aunque tambien: **el equipo no sabe si queda alguien en el
tramo y el operario si**. Un PIN demuestra QUIEN eres; no demuestra que hayas MIRADO la
via. Y un banderillero que da paso cada tres minutos no va a teclear `1234` cada vez -sus
palabras: *"no va a querer estar escribiendo 1234"*-.

**Las tres reglas para que la pregunta no se vuelva invisible**, que son la mitad del
diseno:

1. **Solo en lo que ABRE paso.** Poner rojo, poner ambar, parar: **no se pregunta nunca**.
   Es la direccion segura, y es el criterio que el firmware ya usaba para el PIN
   -`SIN_PIN` incluye `FORZAR_ROJO` y `AMBAR_EMERGENCIA` a proposito-. Preguntar para
   parar ensena a decir que si sin leer.
2. **La pregunta dice QUE MIRAR, no "¿esta seguro?"**. Obliga a levantar la vista.
3. **No sale dos veces seguidas por lo mismo** si el ciclo no ha cambiado de fase.

**Y el PIN caduca**, que no lo hacia nunca: `state.pinVerificado` se encendia en una linea
y **no se apagaba en ninguna**. Se cierra al pasar la app a segundo plano -con **60 s de
gracia**, porque el funcional reporta por WhatsApp y saldria de la app cada dos por tres- y
a los **5 minutos** sin mandar ninguna orden.

> **Lo que NADA de esto arregla, y va escrito para que no se lea como cerrado:**
> `state.correctPin = '1234'` sigue **en claro en el fuente de la app**. Eso no es una
> caducidad, es como se autoriza. Va con la V2.

#### Las once pruebas que celebraban la barrera retirada (§8.quater)

`test_dom_execution.js` cayo a **117 PASS + 11 FALLA**. Se repartieron una por una,
contando cuantas propiedades afirma cada una, y **ninguna se borro**:

| destino | cuantas | por que |
|---|---|---|
| **invertidas** | 2 | exigian *"la orden que mueve luces abre el teclado de PIN"*. Ahora exigen el aviso de via abierto **y el de PIN cerrado**: fue una SUSTITUCION, no un anadido. Dos barreras seguidas no suman, ensenan a decir que si |
| **mudadas** | 4 | el flujo del PIN se muda **literal** a `btn-op-amber`. El PIN no se retiro: se retiro de DOS ordenes |
| **repartida** | 1 | afirmaba dos cosas. Una muda; la otra se convierte en el control positivo de la via |
| **conservadas** | 4 | no estaban mal: caian por cascada porque ese bloque era el unico que autorizaba la sesion |

**El total sube a 142, no baja a 126**, y esa diferencia es la comprobacion que importa:
**el control positivo -que TRAS confirmar la via la orden SALE al cable-**. Sin el, una
guarda que no dejara pasar nada aprobaria las inversiones igual de bien que la correcta.
Es la tapia de §8.sexies.

> 🔴 **Y §8.sexies se reprodujo LITERAL al inyectar el defecto:** al quitar la guarda de
> via, la linea que comprobaba *"no sale ningun byte"* **NO CAYO** -otra barrera mas abajo
> frenaba el envio igual-. Lo unico que cazo la regresion fueron las lineas que miran el
> **ORDEN** de las barreras, y la que vio salir al cable
> `CMD:PIN:1234:MANUAL:CAMBIAR_TURNO` **sin que nadie mirara la calzada**.

**Dos pruebas llevaban verdes midiendo nada, y no lo habia reportado nadie:** *"con 4
digitos validos el modal se cierra"* y *"[btn-op-auto] no vuelve a pedir el PIN"* daban
`[OK]` **entre fallas**, porque el teclado de PIN no se abria NUNCA y *"esta cerrado"* era
vacuamente cierto (§3.bis). Ahora vuelven a medir.

#### Lo que sigue abierto de esto, y es del responsable

| | |
|---|---|
| **AMBAR sigue pidiendo PIN y DAR PASO no** | operativamente es raro: al banderillero le queda con clave justo la direccion segura. Tiene salida sin clave por `ROJO TOTAL` / `AMBAR EMERGENCIA`, que van en `SIN_PIN` |
| **`CANCELAR_AMBAR` tambien ABRE paso** | lo dice su propio comentario, y quedo con PIN y sin confirmacion. Es el candidato mas claro a llevar el aviso de via |
| **Sin telemetria la fase no acota** | `estadoLuces` vale `null` en los dos lados, asi que lo unico que estrecha la ventana de "no repreguntar" son los 30 s |

### 0.0.undecies · EL CIERRE DEL RELOJ, y el defecto que quedaba debajo

**El misterio del `FORMATO_INVALIDO` esta cerrado, y lo cerro el DIARIO DE ORDENES**, no
una deduccion. Tres `SET_RTC` seguidos en la cinta del 04/09:

```
CMD:PIN:****:SET_RTC:2026-09-04,20:45:58
  -> $ACK,NODE:PUENTE,...,RESULT:OK,FECHA:2026-09-04,HORA:20:45:58   <- el DS3231 QUEDO PUESTO
  -> $ERR,CMD:SET_RTC,DESC:FORMATO_INVALIDO                          <- el STM32, cristal muerto
```

**El reloj funciona.** El `$ACK` viene del PUENTE y devuelve la fecha y la hora escritas.
El `$ERR` viene del STM32, que recibe la misma linea porque el puente reenvia VERBATIM -es
su contrato- e intenta poner SU reloj, el del cristal `Y2` muerto desde N-17.

Y **no era un fallo de formato**: medido, el Maestro quita `
` y `
` al montar la linea,
asi que el `sscanf` devuelve 6. Era la SEGUNDA rama -"escribi y al releer no cuadra"- que
**reusaba el motivo de formato**. N-138 la separa: ahora dice `NO_QUEDO_PUESTA`.

#### N-145 🔴 EL EQUIPO TIENE HORA Y PUBLICA QUE NO LA TIENE

El campo `HORA:` del `$STATUS` **lo rellena el STM32**, o sea el micro que **NO tiene**
reloj. Por eso la cinta dice `HORA:--:--:--` con el DS3231 en hora y con pila.

Es la misma forma de N-139 -el contador que contaba el segundero- una capa mas arriba:
**quien publica el dato no es quien lo sabe**. Desde la decision del 28/08 el reloj vive en
el ESP32; el campo se quedo donde estaba. **Acotado:** el puente ya compone tramas propias.

> **SIGUE ABIERTO al cerrar la noche del 04-05/09, y el responsable lo confirmo DOS VECES:**
> *"estamos enviando la hora a la STM32 y tiene que ser al modulo ESP32 que tiene el reloj"*.
> **En todas las tramas de la cinta sale `HORA:--:--:--`.** Hay un agente trabajando en la parte
> del ESP32; nada de esto esta probado en tarjeta.

#### Y N-144, que salio de la misma cinta

El Maestro paso de mandar `HORA:--:--:--` a mandar **`HORA:00:00:00`** justo despues de
intentar el Courier. **No es medianoche: es un contador parado con la bandera de "tengo
hora" puesta.** `reloj_ajustar()` la enciende al escribir y la verificacion posterior
avisaba del fallo **sin retirarla**.

> 🔴 **Y de esa bandera cuelga la autorizacion del MODO DEGRADADO**, el que da verdes
> guiandose SOLO por el reloj, sin confirmacion de la otra punta. Un reloj en ceros que se
> declara valido es exactamente la entrada que ese modo no debe aceptar. Cerrado.

---

### 0.0.duodecies · LO QUE FALTA PARA QUE LA APP SIRVA EN EL POSTE

Por orden de lo que impide usarla, no de lo que cuesta:

| # | que | por que bloquea |
|---|---|---|
| **1** | 🔴 **no reconecta si el equipo se reinicia** | *"se apaga y se prende, no vuelve a conectar... reinicie la app y comenzo"*. Y los equipos se reinician solos: hay `$EVENT,EVT:ARRANQUE,CAUSA:SUBIDA_DE_TENSION` en las cintas. En un poste, de noche, cerrar y abrir la app no vale. La causa esta medida: **`app.js` no llama a `disconnect()` en ninguna parte** y `state.connected` se queda en `true` sobre un socket que Android ya cerro |
| **2** | 🔴 **la barra de pestañas TAPA los mandos** | medido tras arreglar el instrumento: **131 px a 320 px**, 87 a 360, 46 a 390. El operario no puede pulsar lo que no ve |
| **3** | 🔴 **`parseInt("--") \|\| 0`** | el firmware ya manda `--` donde no sabe, y la app lo pinta como **0**: justo la mentira que se acaba de quitar del firmware. En `app.js:2756` y `js/nmea_parser.js:124`, este ultimo **con diez lineas encima explicando por que ese `\|\| 0` esta mal** |
| **4** | el Esclavo no aparece sin desvincular en Ajustes | `list()` solo devuelve emparejados; `discoverUnpaired()` existe y no se usa |
| **5** | los `RESULT` distintos de `OK` se tragan | `YA_EN_AMBAR_LATCH_PUESTO` se ve como "no pasa nada" |
| **6** | las dos pantallas | rediseno, puede esperar a la siguiente entrega |

**Y una que no es de la app:** *"este modo degradado, ¿que putas es eso? No lo he
probado"* — y eso que la app se lo explica al pulsarlo. **La explicacion no esta
aterrizando**, y es el modo que da verdes guiandose solo por el reloj. No se arregla con
mas texto: se arregla decidiendo si ese boton debe estar donde esta.

### 0.0.octies · 🔴 EL BLOQUEO DEL CRUCE, Y COMO SE PROCEDE (04/09, noche)

**Lo reportado, y es lo mas grave de la sesion:** *"como me conecte a la aplicacion del
Esclavo y le di ambar, esta quedo en ambar intermitente. Si me conecto otra vez al Maestro,
esto ya no me recibe nada... si intentas en modo manual cambiarlos, no te responde el
esclavo"*.

#### La cadena, medida entera

1. El ambar de emergencia del Esclavo **engancha un latch** (`ambarEmergencia = true`).
2. Con el latch puesto **el Esclavo no obedece NI ACUSA** `CMD_GO_RED` ni `CMD_GO_GREEN`
   —`Esclavo/src/main.cpp:430` y `:440`, la guarda `!mando_ambarLocal() &&
   !bluetooth_ambarEmergencia()` envuelve tambien al ACK—. **Eso es SFTY-21 y esta bien**:
   quien esta de pie en el poste gana sobre quien manda desde lejos.
3. El Maestro agota sus reintentos, cae a `C_FALLO`, y desde ahi **rechaza** `DAR PASO` con
   `EN_TRANSICION_REINTENTE`. Visto desde fuera: *"ya no me recibe nada"*.
4. **La unica salida es `CANCELAR_AMBAR`, que solo acepta el Esclavo, con PIN.**
5. Y para llegar a el hay que **desvincular el Maestro en Ajustes de Android** y emparejar
   el Esclavo, porque la app pide la lista con `list()`, que solo devuelve emparejados.

> 🔴 **Un operario puede dejar el cruce trabado desde su telefono y no poder soltarlo
> desde el otro poste.** Eso es lo grave, mas que el ambar en si.

#### Y la ventana que lo precede, tambien medida

El Esclavo avisa **al telefono y no al Maestro**: en esa rama no sale ni una trama por
radio. Y un Esclavo en ambar **sigue contestando PONG** —`main.cpp:389-393`, el PING no
refresca su reloj de silencio pero si se responde—, asi que el enlace le parece perfecto al
Maestro. Si el Maestro estaba en VERDE cuando se engancho el ambar, **durante el resto de
esa fase —hasta 3 minutos— conviven Maestro en verde y Esclavo en ambar**, y los dos
sentidos pueden entrar al carril. La red existe —el Maestro cae a fallo en la siguiente
transicion— pero **solo salta en la transicion**, no al instante.

#### COMO SE PROCEDE, y la asimetria que NO se toca

**Se decide (04/09):** poner el ambar **no pide PIN** —parar es la direccion segura— y
**quitarlo SI** —devuelve el cruce a dar verdes, o sea ABRE paso—. Ese criterio es correcto
y se conserva.

**Y NO se permite soltar el latch desde el Maestro.** Se planteo y se descarta: esa
proteccion existe para el caso en que alguien puso el ambar porque hay un operario en la
calzada, y una orden remota no debe revocarla. **El bloqueo es real, pero su causa no es
que falte la orden: es que no se puede llegar al Esclavo.**

| # | que se hace | donde |
|---|---|---|
| 1 | **El Esclavo AVISA por radio al enganchar**, y el Maestro lleva el cruce a estado seguro en el acto en vez de enterarse en la siguiente transicion | firmware, las dos puntas |
| 2 | **El Maestro lo PUBLICA en su `$STATUS`**: *"el Poste 2 tiene ambar de emergencia puesto"*, en vez de parecer averiado | firmware |
| 3 | **La app encuentra el Esclavo sin pelearse con Android** (`discoverUnpaired`), y se puede **cambiar de poste** sin reiniciar | app |
| 4 | **La app enseña lo que SI tiene del Esclavo** —estado, modo, señal, hora— y el hueco dice *"Conectarse al esclavo"*, no *"sin datos"* | app |

**Lo tercero es lo que de verdad desbloquea**, y ya esta en marcha.

**Anotado para la V2, no para ahora:** que la emergencia del Esclavo sea **rojo en su lado**
en vez de ambar. Nunca podria entrar en conflicto —si el Maestro esta en verde, el Esclavo
en rojo es el ciclo normal— y no dependeria de que llegue ninguna trama. Hoy el Esclavo **no
tiene rojo propio**: `FORZAR_ROJO` se le retiro con motivo —*"prometia rojo y hacia ambar con
la pluma arriba, que es casi lo contrario"*— y darselo es trabajo de verdad.

---

### 0.0.nonies · N-141 — Modo Manual tenia la MISMA trampa que N-42, y nadie la habia mirado

`DAR PASO` no funcionaba porque `modo_manual.cpp` entraba en una fase `CONFIG_ESTATICO`
cuya **unica salida era `botonAceptar()`**, que devuelve `false` desde el 31/08. El
coordinador nunca llegaba a `C_IDLE` y el equipo contestaba `EN_TRANSICION_REINTENTE`.

**El cambio de sentido NO habia que construirlo:** `coordinador_pedirCambio()` ya lo hace
entero —verde a rojo directo, todo-rojo de despeje, y los 4 s de ambar SOLO de rojo a
verde, que es justo lo que pidio el responsable—. Faltaba poder LLEGAR.

> 🔴 **Y dentro del mismo fichero estaban la quinta, sexta y septima copia del piso de
> despeje: `segEstatico = 3` en el inicializador, `= 5` en el setup y un piso de `5` con el
> rotulo "Piso minimo" encima.** El minimo vial son **10**. N-137 centralizo los limites ese
> mismo dia y **no vio este fichero porque su codigo estaba muerto**: no habia sintoma que
> buscar. **Un arreglo ingenuo que se limitara a quitar la fase habria ACTIVADO un despeje de
> 5 s en un cruce en servicio.** El defecto de interfaz y el vial vivian en la misma linea.

Se retira la fase, el `enum` entero (§3.septies) y las tres copias. Manual **ya no configura
tiempos**: conserva el despeje del Automatico —que si pasa por `limites_ciclo.h`— o los 15 s
del coordinador. **Flash: 58.368 -> 55.868 B, o sea 2.500 B LIBERADOS.**

**Y lo que NO se reactiva sin decision del responsable:** el mando A/B daba paso desde el
suelo (`:45-47`), muerto desde el 31/08. Al quitar la fase reviviria solo, y eso significa que
**un pulso suelto de A o de B cambia el sentido del trafico**. Dar paso ABRE paso: anadir una
via nueva de abrirlo —y ademas un mando a distancia— no se cuela dentro del arreglo de otra
cosa. Queda pendiente de decidir.

### 0.0.septies · 🟢 EL BANCO DEL 04/09 POR LA NOCHE — lo que se cerro EN COBRE y los cuatro defectos nuevos

**N-42 CERRADO EN COBRE.** Palabras del responsable, con el equipo delante: *"ahi cambia ese
amarillo y este a verde. Ahora esta funcionando"*. El Modo Automatico mueve las luces, con
los 3 minutos puestos y confirmados: *"creo que esta minimo 3, no? Minimo 3, si"*.

**Y LA CONFIRMACION DE VIA CONVENCIO:** *"dice: no quedan vehiculos en el tramo... se le
pregunto, no lo puede cambiar asi porque si"*. Ademas quedo resuelta una duda suya que
conviene dejar escrita: **confirmar la via NO acorta el ciclo** — el equipo termina sus 3
minutos igual. Su criterio: *"y si se los cambia, esta mal"*. No se los cambia.

#### 🟢 Y EL DIARIO DE ORDENES DIAGNOSTICO EL RELOJ EN UN SOLO EXPORT

Lo que no se pudo en veinte minutos de deduccion, lo dijo la primera exportacion:

```
20:01:28  ORDEN      CMD:PIN:****:SET_RTC:2026-09-04,20:01:28
20:01:28  RESPUESTA  $ACK,NODE:PUENTE,CMD:SET_RTC,RESULT:OK,FECHA:2026-09-04,HORA:20:01:28
20:01:32  RESPUESTA SUELTA  $ERR,CMD:SET_RTC,DESC:FORMATO_INVALIDO
```

**El reloj SI se pone.** El `$ACK` viene del **PUENTE** —el ESP32 con el DS3231— y devuelve la
fecha y la hora que quedaron escritas. Cuatro segundos despues llega un `$ERR` **sin
`NODE:PUENTE`**: es del STM32, que recibe la misma linea porque el puente reenvia VERBATIM
—es su contrato—, intenta poner **su** reloj —el del cristal muerto de N-17— y falla.

> **La leccion de metodo: el instrumento se arreglo por la manana y por la noche pago el
> arreglo entero.** Es lo contrario de §2.bis: aqui el instrumento no sustituyo a la tarjeta,
> le contesto una pregunta que la tarjeta no sabia formular.

#### Los cuatro defectos nuevos

| | que | estado |
|---|---|---|
| **N-137** | 🔴 **el Modo Inteligente configuraba el cruce con 2 MINUTOS de verde**, por debajo del minimo vial. Escribia los tiempos por su cuenta sin pasar por la guarda — y era el modo que la guia recomendaba como salida | 🟢 cerrado |
| **N-138** | el `FORMATO_INVALIDO` del reloj era un **mensaje mentiroso**: reusaba el motivo de formato para decir "no quedo puesta". Dos causas, una sola respuesta | 🟢 cerrado |
| **N-139** | 🔴 **el contador de la app no cuenta la fase: cuenta el segundero del equipo.** `T: = (millis()/1000) % 60` — va de 0 a 59 y vuelve a empezar, este el semaforo donde este. El comentario dice "segundos transcurridos en fase actual" **y no lo es** | 🔴 abierto |
| **N-140** | 🔴 **`DAR PASO` en manual deja Maestro en rojo y Esclavo en ambar titilando.** El responsable: *"eso no esta bien... sera que haga el switcheo, uno verde, el otro rojo, y luego invertirlos"* | 🔴 abierto, falta confirmar la definicion |

**N-137 es la TERCERA vez el mismo dia que un limite vital vivia en un sitio y estaba escrito
a mano en otro.** Por eso los seis numeros se mudaron a `Maestro/include/limites_ciclo.h`, un
solo fichero que todos los modos leen, con el porque de los 3 minutos en su cabecera. Cuatro
instrumentos ABORTARON en la corrida siguiente —§5, leen por ruta— y los cuatro se
reapuntaron: eso es la guarda funcionando, no un estorbo.

**Y N-139 es otra vez la forma de §2.ter:** un campo DECLARADO como tiempo de fase que nadie
EJERCIO nunca. La app lo pinta fielmente; el numero no significa nada.

### 0.0.quinquies · 🎯 LO QUE HAY QUE MEDIR EN LA PROXIMA CARGA — paquete `944c18d`

**La prueba que mas vale de la sesion es el paso 30, y su resultado bueno esta escrito al
reves de lo habitual: QUE NO PASE NADA.** Poner Automatico y mirar al Esclavo dos minutos.

| lo que se vea | que significa |
|---|---|
| el Esclavo **ya no** se va a ambar solo | N-42 cerrado en cobre |
| **sigue** yendose a los ~25 s | hay una SEGUNDA causa, y es informacion que hoy no tenemos |
| las luces no ciclan pero el Esclavo aguanta | dos fallos distintos, no uno |

🛑 **Aviso que evita un defecto inventado:** la FIRMA del respaldo cambio, asi que la
PRIMERA vuelta de energia con este firmware **borra los tiempos guardados** y el equipo
arranca con los minimos. Es correcto y esta disenado asi. La prueba de N-133 vale **desde
la segunda vuelta**.

**Para el reloj:** repetir la inyeccion de hora y mandar **el DIARIO DE ORDENES**, no la
cinta. Ahi saldra la trama que salio del telefono y el `DESC` que devolvio el equipo, en
dos lineas. Es lo que falto esta tarde y por lo que el `FORMATO_INVALIDO` sigue sin
diagnosticar.

---

### 0.0.sexies · EL BALANCE DE LA SESION, y no es comodo

**Tres instrumentos distintos celebraban comportamientos que hoy cambiaron**, y los tres
hubo que repartir uno por uno en vez de reescribirlos hasta que dieran verde:

| instrumento | que celebraba |
|---|---|
| `maestro_01_mando` | que `coordinador_actualizar()` viviera SOLO dentro de `case CORRIENDO` — era la premisa de su modelo, y su propio mensaje pedia rehacerla |
| `test_dom_execution` | que la orden que mueve luces **abriera el teclado de PIN**. Once comprobaciones |
| `simulador_puente_esp32` | que pulsar AUTOMATICO escribiera una orden **sin preguntar nada** — este ABORTO la compuerta |

**Y el dato que resume el dia: la compuerta estuvo en verde mientras dentro habia un
defecto NUESTRO** -N-135- **que rompia `SET_TIEMPOS` entero.** Lo encontro un agente
compilando una funcion de tres lineas, no leyendola. Ni la compuerta ni los 68 packs lo
vieron.

> **Lo que eso deja escrito, y ya estaba en §2.ter con otras palabras: un `20/20` dice que
> los modelos y los arneses de PC no encuentran nada. Sigue sin decir que el firmware
> funcione en la tarjeta — y hoy ni siquiera dijo que el firmware fuera consistente
> consigo mismo.**

**Lo que si funciono, y conviene saberlo para repetirlo:** los tres hallazgos que mas
valen del dia salieron de **cruzar trabajo entre agentes** — uno fue a comprobar si un paso
de banco era ejecutable y encontro N-135; otro conto las pruebas una por una y corrigio el
reparto de un tercero; otro censo los `DESC` y encontro **23** donde se le habian dicho 5,
con dos nombres mal. Ninguno salio de mirar mas rato el mismo fichero.

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

## 3. Lo decidido, con fecha

| decision | cuando | consecuencia |
|---|---|---|
| **El ESP32 sustituye al modulo SPP dedicado** y se lleva ademas el reloj | 28/08 | ya no se compran `HC-05`/`JDY-30` |
| **Se retira la pantalla LCD** de las dos puntas | 28/08 | libera `PB6`/`PB7` para el Bluetooth y `PB3`/`PB4`/`PB5` de margen, mas ~18,9 KB de flash |
| **El `DS3231` sale del STM32** y cuelga del ESP32 | 28/08 | la linea `PIN-0` queda ANULADA: el I2C ya no vive en el STM32 |
| **El mando de reles se CONSERVA en A y B**; se retiran C y D | **31/08** | `A·A·A`, `B·B·B` y `A·B·A·B` sobreviven, el veto de SFTY-21 no desaparece, y `PB14`/`PB15` quedan para las camaras — **ver N-104** |
| **El modulo es un `ESP32-WROOM-32` clasico: hay SPP** | **31/08** | la app conecta sin tocar el transporte; el apartado 1 del Manual 10 queda intacto; la alimentacion es `12 V -> DC-DC conmutado -> 5 V -> VIN` |
| **Las camaras entran por `J16` p10 y p12** | **31/08** | se leen por el camino de camara (`INPUT` + activo en ALTO), no por el de boton |
| **4 SEMAFOROS POR CRUCE, 2 POR PLACA** | **04/09** | y cuadra con el firmware: `escribirPines()` escribe `ROJO1`/`ROJO2`, `AMARILLO1`/`AMARILLO2` y `VERDE1`/`VERDE2` **con el mismo valor** — son dos caras de una senal, no dos semaforos independientes. **Funciona hoy: no hace falta trama nueva ni protocolo nuevo.** Ocho serian dos cruces de estos |
| **Va un `L298N` por barrera, FUERA de la placa, en el esquema SIMPLE** | **04/09** | `J15` gobierna `ENA`, direccion fija: la pluma **sube con senal y baja por su peso**. Sin inversor, sin finales de carrera a la tarjeta, sin firmware. **NO se cablea hasta tener la corriente del motorreductor** — el `L298N` cae 2-5 V y con 12 V el par baja mas de la mitad; ademas son 2 A por canal y una pluma arranca entre 3 y 8 A. Pregunta 11 del HTML de banco |
| **Los finales de carrera NO van a la placa** | **04/09** | van **en serie con el motor**, cada uno con un diodo en antiparalelo. Y con el esquema simple ni eso: **arriba lo para el final de carrera que el propio motorreductor ya trae**. Censado: de los 37 pines del LQFP48 quedan cuatro, y son `PA13`/`PA14` (SWD, es por donde se carga) y `PC14`/`PC15` (el cristal muerto de N-17). **No hay entradas libres, y no hacen falta** |
| **`A5` resuelta: `LM2596` desde la bateria** | **04/09** | no es una fuente que comprar aparte: es un reductor 12 -> 5 V DC colgado de la bateria. La fila `A5` de la lista de compras se reescribe con eso |
| **El PIN CADUCA: al guardarse el telefono (60 s de gracia) y a los 5 min sin mandar nada** | **04/09** | hoy `state.pinVerificado` se enciende en una linea y **no se apaga en ninguna**: el operario teclea, se guarda el telefono, y el siguiente que lo coja manda ordenes sin teclear. Los 60 s existen porque el funcional reporta por WhatsApp y saldria de la app cada dos por tres |
| **EL OPERARIO DEJA DE TECLEAR EL PIN. Lo sustituye una CONFIRMACION** | **04/09** | *¿confirma que no quedan vehiculos en el tramo?* antes de lo que ABRE paso. El motivo es de fondo: **el equipo no sabe si queda alguien en el tramo y el operario si**; una cuenta atras finge saberlo. Y el PIN no protege al que abre paso — demuestra QUIEN eres, no que hayas MIRADO. **Nunca se pregunta para poner rojo o ambar**: parar es la direccion segura, y preguntar ahi ensena a decir que si sin leer. El PIN se queda para el tecnico: tiempos, modos, reloj, test |
| **El cruce SE OPERA DESDE EL MAESTRO.** No se hace transparente el mando desde el Esclavo | **04/09** | el operario tiene que saber a que poste conectarse ANTES de caminar, asi que el rotulo Bluetooth pasa a ser lo que lo resuelve (ver N-129). Se descarta relevar `SET_MODO` y `MANUAL:CAMBIAR_TURNO` por radio: no se toca el Maestro, que va al 89,3 % de flash |

---

## 4. Lo que esta ABIERTO, y de quien es

### Del responsable — no se destraban con mas analisis

| # | que | como se cierra |
|---|---|---|
| ~~**BLQ-1**~~ | 🟢 **CERRADO el 31/08.** Es un **`ESP32-WROOM-32` clasico**: `Xtensa LX6 dual-core` y `Bluetooth v4.2 **BR/EDR** + BLE` — hay **SPP**. La app conecta sin tocar el transporte y el apartado 1 del Manual 10 **no se reabre**. Ver **N-107** | — |
| ~~**M3**~~ | 🟢 **CERRADA EN BANCO el 03/09, paso 20.** El pull-down es **real y de 10 kOhm**: `p10` mide **9,93 kOhm** a masa y `p12` **9,94 kOhm**, los dos a **0 V** con energia. El camino de camara —`INPUT` pelado, activo en ALTO— es correcto y **no hay demandas fantasma**: el paso 21 lo confirmo con y sin el cable puesto. La misma medida condena el mando: ver **N-118** | — |
| **A5** | 🔴 **La fuente propia del ESP32 desde 12 V.** No esta pedida y hace falta | comprarla |
| **MATRIC** | 🟠 **Matriculacion / emparejamiento por ID de Bluetooth, no por nombre, y sin mano.** Aplazado por decision suya —*"detras del banco, primero cierra lo del banco"*—. 🔴 El dato que condiciona el diseno: **`RF_Packet` son 4 bytes `{msgID, command, param, crc}` y no tiene campo de direccion**; el CRC cubre 3. Ver `0.0.terdecies` | decidir si se cambia el formato de trama o la matriculacion vive solo del lado Bluetooth |
| ~~**N75-1**~~ | 🟢 **CERRADA el 04/09. Son 3 minutos**, y el motivo es del responsable, literal: *"es la minima distancia de seguridad"*. La guarda vive en `modo_automatico.cpp` -no en la app, que no es la unica que habla por `J17`- y `app_11_rangos_de_tiempos` cruza los seis numeros del C++, `app.js` y el HTML en cada corrida. Coste aceptado a sabiendas: ya no se prueba en mesa con ciclos de un minuto | — |
| **N-129** | 🔴 **El rotulo Bluetooth pasa a la ruta critica al decidirse que se opera DESDE EL MAESTRO.** Si el operario tiene que conectarse al poste correcto, tiene que poder distinguirlo ANTES de caminar. Medido: el ESP32 aprende `SEM-<serie>-M` / `-E` del `$STATUS` y **lo guarda para la SIGUIENTE arrancada** -no se re-rotula en caliente a proposito, para no tirar la sesion de quien esta dando una orden-. O sea que **un modulo virgen anuncia `SEM-SIN-MATRICULA` en su primer arranque, y las DOS puntas se llaman igual**. Hace falta una vuelta de energia despues de que el STM32 hable | decidir si se documenta como paso de puesta en marcha o se cambia el momento del rotulado |
| ~~**N-130**~~ | 🟢 **CERRADO el 04/09.** El Maestro ya no arma la bandera si el modo no la va a consumir, y el acuse viaja con motivo en el `param` -`DEMANDA_ACEPTADA` (0) / `DEMANDA_RECHAZADA`-, sin gastar codigo de comando ni tocar la trama. El Esclavo avisa con `MAESTRO / DEMANDA_NO_ATENDIDA_MODO_ACTUAL`. Pack `costura_12_acuse_de_demanda`, visto fallar: 27/30 con el defecto reinyectado, 30/30 al restaurar. Coste medido con el MISMO instrumento en los dos extremos: **Maestro +36 B, Esclavo +116 B** | — |
| **N-131** | 🟢 **CERRADO el 04/09, y es el que casi se escapa: la guarda de 3 minutos era MEDIA GUARDA.** Solo la cruzaba `SET_TIEMPOS`. Habia **cinco** sitios mas en `modo_automatico.cpp` con el numero prohibido escrito a mano: el inicializador estatico (`1, 1, 15`), el reset de `modoAutomatico_setup()` (otra vez `1, 1, 15`) y los topes de los tres campos del menu (piso 1 min / 1 min / **5 s**, la MITAD del minimo vial del despeje; techo 99/99/999, que ni cabe en el `uint8_t` con que viaja por radio). Consecuencias reales: un equipo al que nadie manda `SET_TIEMPOS` corria con **un minuto por sentido**, y como `SET_MODO:AUTO` llama a ese `setup()`, **unos tiempos aceptados con `$ACK` se perdian al arrancar el modo**. Ninguna de las copias llevaba encima el comentario de seguridad: el dia que difieren gana la que NO lo lleva. Los seis numeros salen ahora de las mismas constantes; coste de flash **0 B**. Vigilado por `app_11_rangos_de_tiempos`, visto fallar 9/10 | — |
| **N-132** | 🔴 **NO hay puente H en la PCB, y solo sale UNA linea de control hacia la pluma.** Censado sobre el `.kicad_pcb` (2.158.421 B) con el buscador comprobado -encuentra `IRLZ44` x20 y `TLP127` x20, que cuadra con `Q1..Q10` y `U6..U15`-: de `L298`, `L293`, `DRV8x`, `BTS7x`, `TB6612`, `IBT` y `BTN8x` hay **cero**. La talanquera es un MOSFET de lado bajo, `PB2 -> U15 -> Q10 -> J15`, y `J15` es un `Conn_01x02`: **dos bornes, un sentido**. El responsable indica el 04/09 que va **un L298N por barrera**; un puente H necesita dos entradas de control y de esta placa sale una | decidir como se resuelve la segunda linea - ver el apartado nuevo mas abajo |
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
| **La noche del 04-05/09** | **N-142** (el Esclavo avisa de su ambar, `6274acc`) · **N-146** (`SET_MODO:AMBAR` contestaba OK sin encender nada) · **N-147** (Manual hacia un ciclo que nadie pidio). **Los tres SIN tocar tarjeta.** Ver `0.0.terdecies` |

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
| 🔴 **N-145** | el `HORA:` del `$STATUS` lo publica el STM32, que **no tiene reloj**; el `DS3231` vive en el ESP32. Toda la cinta del 04/09 dice `HORA:--:--:--` | **abierto**, agente sobre el ESP32 |
| 🟠 **N-148** | la app no pide confirmacion de via al dar ambar en Manual; en `DAR PASO` si | **abierto**, agente sobre la app |
| 🟠 **N-149** | el `$STATUS` del Maestro no traia **ningun** campo del Esclavo (verificado en la cinta) | **en curso**: `ESC:` en el arbol, **sin banco** |
| 🟠 **BAT:--** | la bateria no se mide nunca — en **todas** las tramas de la cinta del 04/09 | **abierto y SIN CAUSA MEDIDA**. No se escribe una hasta medirla |
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


---

## La cuenta de la mudanza — la red contra perder un bloque

`CLAUDE.md` §4: *un «no aparece» no es un hallazgo hasta haber descartado al buscador*. Aqui la
cuenta va publicada para que cualquiera pueda rehacerla, no para que se crea.

```
roadmap.md ANTES de la mudanza                  5.391 lineas

roadmap.md DESPUES, lineas de original conservadas 754
cabeceras de apartado RENUMERADAS a plano             6   <- ver abajo
roadmap_hist.md, lineas de original                4.631
                                                   -----
                                                   5.391   <- cuadra exacto

anadido y contado aparte:
   14 lineas en el historico  las 3 frases FALSAS que viajan TACHADAS
  cabecera + los dos indices  levantados del cuerpo por script
  339 lineas en roadmap.md    texto nuevo: el reparto por quien desbloquea,
                              el apartado POR VALIDAR y los punteros a lo cerrado
```

**Las 6 lineas que NO estan literales en ningun sitio son SEIS CABECERAS DE APARTADO**, y
desaparecieron a proposito: el encargo pedia **renumerar plano, sin `bis`/`ter`**, porque el
fichero tenia `0.1`, `0.2`, `0.3` y `0.4` **duplicados con contenido distinto** —el fallo que
`DECISIONES.md` se creo para impedir, ocurriendo dentro del propio roadmap—. **Su texto
descriptivo sobrevive en la cabecera nueva; lo unico que se fue es el numero latino.**

```
0.0.novodecies -> roadmap.md sec. 6.7      0.bis  -> roadmap.md sec. 9
0.0.octodecies -> roadmap.md sec. 6.8      2.     -> roadmap.md sec. 8
0.0.decies     -> roadmap.md sec. 6.9      7.     -> roadmap.md sec. 10
```

**El cuerpo se extrajo por RANGOS DE LINEA del original, no transcribiendolo**, asi que la unica
forma de haber perdido un bloque seria un rango mal puesto — y eso es justo lo que la suma de
arriba caza. Los rangos mudados, para poder reproducirlo:

```
1-628 · 774-1482 · 1516-1888 · 1926-2074 · 2128-2341 · 2544-2901
2994-3102 · 3164-3309 · 3350-3496 · 3584-5381
```

Y lo que se quedo vivo, que es el complemento exacto:

```
629-702 · 703-773 · 1483-1515 · 1889-1925 · 2075-2127 · 2342-2508
2509-2543 · 2902-2993 · 3103-3163 · 3310-3349 · 3497-3583 · 5382-5391
```



---
---

# 📦 ANEXO — `OPTIMIZACIONES.md` ÍNTEGRO, LA FOTO DEL 12/09/2026 ANTES DE LA REDUCCIÓN

> 🔴 **ESTO NO ES LA SPEC. La spec de las reglas `SFTY-x` es [`OPTIMIZACIONES.md`](OPTIMIZACIONES.md),
> y GANA a cada línea de este anexo.** Aquí está la versión de **2.310 líneas** tal y como estaba el
> 12/09/2026, movida entera —sin resumir, sin reescribir y sin borrar una sola línea— cuando el
> responsable pidió *«2100 líneas no es correcto… reduce esto al mínimo de líneas»*.
>
> **Por qué se movió íntegra y no por trozos.** El documento tenía **1.023 líneas en bloques de cita**
> (44 %), pero el prefijo `>` **no marca crónica en ese fichero**: dentro de bloques `>` viven varios
> enunciados de regla —*«La talanquera SIGUE al semáforo»*, *«El enlace por datos puede OBSERVAR…
> No puede AUTORIZAR»*, *«`B·B·B` devuelve a ámbar desde cualquier estado»*, *«EL TOPE NO ES UN
> DETALLE: ES LA REGLA»*—. Trocear por ese prefijo habría borrado reglas de seguridad, así que se
> mudó el cuerpo completo y se **reescribió** el documento vivo en forma destilada. La red contra
> perder algo es esta copia: **está entera y es literal.**
>
> **Qué buscar aquí y no en el documento vivo:** la saga del LCD (28/08 retirada → 31/08
> rectificación → 01/09 medida por fichero objeto, 19,3 KB de flash y 1.024 B de RAM), las tres
> auditorías de la tabla de trazabilidad (01/09), la colisión de número de `SFTY-27` contada larga,
> la auditoría `N-109` del riesgo residual de `SFTY-21`, el porqué del mando de 4 relés, la saga de
> los dos esquemáticos de `SFTY-28` y la refutación de `J15`.
>
> ⚠️ **Dos rojos iban CADUCADOS y sin tachar. Se han marcado aquí, no borrado** (busca
> `TACHADO EN LA MUDANZA`): el que llamaba a `2779d9b` sospechoso de la regresión del Modo Automático
> (`N-42` cerrado el 04/09) y el punto 1 de la optimización de pantalla, refutado por su propio
> bloque anidado. Se marcan **antes** de archivar, porque un rojo caducado que entra al histórico se
> queda ahí para siempre.
>
> **La cuenta:** 2.310 líneas de origen + 9 líneas de marca de tachado = 2.319 líneas de cuerpo.

# ⚡ Matriz de Optimizaciones y Reglas de Seguridad (V8.7)

**Fecha de Revisión:** 31 de Julio de 2026 · **última auditoría: 01 de Septiembre de 2026**  
**Ecosistema:** Firmware STM32 + Repetidor ESP32 + Radio LoRa E90-DTU  
**Velocidades, que no son la misma y se confundían en esta cabecera:** el puerto serie al módulo va a
**9600 bps** (`Bus.begin(9600)`, `*/src/protocolo.cpp:49`); la **tasa aérea** es de **2,4 kbps**, que es
la que determina el coste de la ráfaga de SFTY-11 y el peor caso de reintentos de SFTY-7.

> ### 🔎 01/09/2026 — AUDITORÍA DE ESTE DOCUMENTO CONTRA EL FIRMWARE DE HOY
>
> Se cruzaron las **29 reglas `SFTY-x`** que aquí se definen contra las **23 etiquetas `# EJERCE`** de
> los 59 packs, **en las dos direcciones**, y se comprobaron una a una las afirmaciones verificables
> sobre hardware y firmware. **Lo corregido no se ha borrado: se ha tachado con su motivo**, y cada
> corrección lleva pegada la medida —fichero:línea o la salida del comando— con la que se hizo.
>
> Los seis bloques que un lector con prisa necesita:
>
> | | dónde |
> |---|---|
> | 🔴 **`SFTY-27` designa DOS reglas** y CUATRO sitios mandan a leer la equivocada | tras la tabla de trazabilidad, y un aviso en el propio § `SFTY-27` |
> | 🔴 **Seis reglas no tenían fila** en la tabla — `SFTY-20`, `22`, `24`, `25`, `26`, `27` | la tabla |
> | 🔴 **Qué significa de verdad un ✅**: los cuatro packs de `SFTY-2` **leen** el C++, no lo ejecutan | tras las «siete filas vacías» |
> | 🔴 **El riesgo residual nº 2 estaba descrito corto**: lo dispara un microcorte, y acaba en choque frontal | § riesgos residuales de `SFTY-21` |
> | ✅ **El riesgo del menú a ciegas se cerró solo el 31/08**: `botonAceptar()` y `botonCancelar()` devuelven `false` | primer bloque del documento |
> | 📐 **La pantalla: el bus ya no está, la pila sigue enlazada** — 19,3 KB de flash y **1.024 B de RAM**, medidos | § optimización pendiente |
>
> **Nada de esto es una prueba de banco.** Todo lo de aquí se midió sobre el fuente, el mapa del
> enlazador y el esquemático, desde un PC.

---

> ## 🔴 28/08/2026 — SE RETIRA LA PANTALLA LCD. QUÉ REGLAS QUEDAN SIN INTERFAZ
>
> **La pantalla no se lee.** El equipo va montado en alto, y una LCD de 128×64 a 5 m dentro del
> gabinete no la mira nadie desde el suelo. **La interfaz de operación pasa a ser la app por
> Bluetooth**, y el módulo entra por el conector `J17`, en los dos pines que deja la pantalla
> (`PB7`/`PB6` = `USART1` remapeado).
>
> **Lo que esto le hace a las reglas de arriba, sin adornos:**
>
> - **SFTY-12 (Navegación de Menú Independiente)** decía que el operario debe poder configurar el
>   equipo *«incluso si las radios están apagadas»*. **La regla sigue viva y su motivo también**,
>   pero **su vía ya no es el menú local**: es el enlace Bluetooth, que es igual de independiente de
>   la radio de largo alcance. Lo que la regla exigía —independencia de red— se conserva; lo que
>   cambia es por dónde entra el operario.
> - **SFTY-18** describe la pantalla **`AJUSTAR HORA`** como *«la única vía para poner el reloj y,
>   por tanto, el requisito previo de SFTY-20 y SFTY-21»*. **Sin pantalla esa vía se cierra.** La
>   sustituye el comando `CMD:PIN:...:SET_RTC:` del Bluetooth, que existe y está en el firmware
>   (`bluetooth.cpp`, ambas puntas). **Todo lo demás de SFTY-18 no cambia**: la hora sigue naciendo
>   declarada no fiable y `reloj_enHora()` sigue siendo la barrera.
> - **SFTY-14 y SFTY-15** publican sus medidas *«en la pantalla PRUEBA ALCANCE»*. Esa pantalla
>   **existe en el firmware y ya no tiene dónde dibujarse.** La calidad de enlace sí sale por
>   telemetría (`RF:` de `$STATUS`); **los contadores de línea de SFTY-15 —`RX 0 - nada llega` /
>   `RX 4k - BASURA`— NO están en la trama `$STATUS`.** Es una capacidad de diagnóstico que se pierde
>   hasta que alguien la lleve a la app, y se anota aquí en vez de darla por trasladada.
>
> ### ⚠️ EL MENÚ NO SE HA BORRADO DEL FIRMWARE, Y ESO ES UN RIESGO NUEVO
>
> **Medido sobre el fuente (28/08):** `lcd.cpp`, `menu.cpp` y `modo_hora.cpp` siguen compilándose;
> `lcd_setup()` sigue llamando a `u8g2.begin()`; y los cuatro botones de `J16` (`PB9`, `PB13`,
> `PB14`, `PB15`) siguen navegando el menú. Lo único que cambia es que **no hay display donde se
> vea el resultado**.
>
> **Consecuencia:** quien pulse esos botones —o accione el mando de relés, que va en paralelo con
> ellos— **navega un menú a ciegas**. Con suficientes pulsos se llega a `CONFIGURACION → AJUSTAR
> HORA` y **se confirma una hora cualquiera que el equipo dará por buena**. Es exactamente el fallo
> que la prueba 8.6 del protocolo existe para descartar, pero **ahora sin el aviso visual que
> permitía detectarlo**.
>
> La inhibición del mando con el menú abierto (SFTY-21) **sigue protegiendo contra las secuencias**;
> **no protege contra la navegación**, porque la navegación es justo lo que el menú abierto sí
> acepta. **Mientras el menú siga en el binario, esto es un riesgo abierto y no una nota
> histórica.**
>
> > ### ✅ 01/09/2026 — ESTE RIESGO SE CERRÓ SOLO EL 31/08, Y NO POR HABERLO ATENDIDO
> >
> > **El párrafo de arriba dejó de ser cierto y no se borra: se marca refutado.** Lo que lo cerró fue
> > el reparto de `J16` de N-97, que se hizo por las cámaras y no por esto.
> >
> > **MEDIDO sobre el fuente (01/09), y es de una línea:**
> >
> > ```
> > Maestro/src/botones.cpp:280   bool botonAceptar() { return false; }
> > Maestro/src/botones.cpp:281   bool botonCancelar(){ return false; }
> > Esclavo/src/botones.cpp:294   bool botonAceptar() { return false; }
> > Esclavo/src/botones.cpp:295   bool botonCancelar(){ return false; }
> > ```
> >
> > `PB14` y `PB15` —`J16` p10 y p12— **son entradas de cámara desde el 31/08** (`Maestro/include/pines.h:124-125`,
> > `CAM_C_PIN` y `CAM_D_PIN`, `INPUT` pelado y activas en ALTO). ACEPTAR y CANCELAR **se quedaron sin
> > pin**, y las dos funciones devuelven `false` incondicionalmente. De los cuatro botones que este
> > párrafo daba por vivos **quedan dos**: `BOTON1 = PB9` (arriba / mando A) y `BOTON2 = PB13`
> > (abajo / mando B).
> >
> > **Consecuencia exacta, ni más ni menos:** el cursor todavía sube y baja, pero
> > `Maestro/src/menu.cpp:111` (`if (botonAceptar())`) y `Maestro/src/modo_hora.cpp:208` (el que confirma
> > la hora) **no pueden dispararse nunca desde el panel**. La ráfaga de pulsos a ciegas ya no puede
> > confirmar una hora inventada, porque **no hay con qué confirmar**. El veneno que describe SFTY-18
> > sigue existiendo; su vía por el panel, no.
> >
> > **Y el efecto lateral, que va en la dirección buena y lo anota el propio fuente**
> > (`Maestro/src/botones.cpp:274-276`): con ACEPTAR mudo la pantalla del Esclavo no puede bajar del
> > listado, así que `menu_estaAbierto()` es **siempre falso** y el mando ya no puede quedarse inhibido
> > por una pantalla que alguien dejó abierta.
> >
> > ⚠️ **Lo que esto ABRE, y es la otra cara:** el mando de relés y la app pasan a ser **la única**
> > interfaz de operación. Los sustitutos están censados llamador a llamador en
> > `Maestro/src/botones.cpp:253-273` —`SET_MODO:`, `SET_TIEMPOS`, `MANUAL:CAMBIAR_TURNO`, `SET_RTC`
> > por Bluetooth en el Maestro; el mando `A·A·A` / `B·B·B` / `A·B·A·B` en el Esclavo, que **no tiene
> > `SET_MODO`**—. Ese censo es del fuente y **no está reflejado en los manuales de campo**: quien suba
> > al poste con el manual de hoy en la mano buscará dos botones que ya no hacen nada.

---

> ## 🟢 31/08/2026 — RECTIFICACIÓN: LA PANTALLA **NO** SE RETIRA. LO QUE SE CORTA ES EL CABLE
>
> **Esta entrada deja sin efecto el titular del bloque anterior** («SE RETIRA LA PANTALLA LCD»). El
> análisis de reglas que hay ahí arriba **sigue siendo válido y por eso no se borra** —una causa que
> se cae se marca refutada, no se hace desaparecer—; lo que cambia es la decisión, tomada por el
> responsable: *«no quitar el LCD del firmware si así va y la memoria alcanza»*.
>
> ### El dato que obligó a actuar de todas formas, y está medido en el cobre
>
> `03_Hardware_Tarjeta/MAPEO_TARJETA_KICAD.md:349-350` reparte **un solo conector** entre dos cosas:
>
> ```
> LCD ST7920 (3 hilos desde N-76)   PB3 PB4 PB5     ->  J17  p4, p1, p5
> Módulo Bluetooth / ESP32          PB6 TX PB7 RX   ->  J17  p3, p2
> ```
>
> Y `:378` añade el detalle que lo vuelve urgente: **`PB3` es `SCL` (p4) y conmuta en cada bit**.
>
> De ahí salen dos hechos. Uno, **no pueden estar los dos enchufados**: es un conector, y en cuanto
> el ESP32 ocupa `J17` la pantalla ya no está físicamente, se retire su código o no. Dos, y es el que
> obligaba a tocar el firmware: **el código seguía conduciendo tres hilos de ese mismo conector**. Un
> reloj de SPI por software corriendo pegado al RX/TX del ESP32 dentro del mismo mazo es exactamente
> lo que produce **corrupción intermitente del enlace serie**: la avería que no se diagnostica nunca,
> porque aparece y desaparece según lo que la pantalla esté dibujando en ese instante.
>
> ### Lo que se hizo el 31/08, que no es retirar nada
>
> En `lcd.cpp` de las dos puntas, **los cuatro argumentos de pin del constructor de `u8g2` pasan a
> `U8X8_PIN_NONE`**. El objeto se construye igual —mismo tipo, mismo transporte—, pero **no recibe ni
> un solo pin**. La pantalla ya renunciaba al reset así; ahora renuncia también a `SCLK`, `SID` y
> `CS`. El framebuffer se compone igual; **no se vuelca al cable**.
>
> **Que eso baste no es una suposición, está leído en la librería.** En
> `U8x8lib.cpp::u8x8_gpio_and_delay_arduino()` los **dos** caminos que tocan un pin preguntan antes:
> `if ( u8x8->pins[i] != U8X8_PIN_NONE )` antes del `pinMode` de arranque, y `if ( i != U8X8_PIN_NONE )`
> antes del `digitalWrite` de cada escritura. Con los cuatro en `NONE` no queda ni un `pinMode` ni un
> `digitalWrite`: `PB3`, `PB4` y `PB5` quedan en **alta impedancia**.
>
> **Se conserva todo lo demás**: el API `lcd_*`, `menu.cpp` entero, los tres packs de pantalla con su
> sujeto intacto y las **271/271** comprobaciones de `Validacion_LCD` —verificadas antes y después
> del cambio, sin moverse—.
>
> | | antes | después | delta |
> |---|---|---|---|
> | Maestro | `57824` B · 88,2 % | `57824` B · 88,2 % | **0 B** |
> | Esclavo | `42152` B · 64,3 % | `42152` B · 64,3 % | **0 B** |
> | RAM (las dos puntas) | 3488 / 3328 B | 3488 / 3328 B | **0 B** |
>
> **Medido, no estimado**: `pio run` con reconstrucción limpia en los dos extremos, mismo toolchain
> (§7 de `CLAUDE.md`: *un delta exige medir los DOS extremos*). **El delta es exactamente cero y tiene
> que serlo**: no se ha retirado ni una línea de código, sólo han cambiado unas constantes que se
> pasan por argumento. Un ahorro aquí habría sido la señal de que se hizo más de lo pedido.
>
> > 🔶 **La variante que sí ahorraba, y por qué se rechazó.** Armar `u8g2` con procedimientos de bus
> > y GPIO **nulos** —la técnica que `Validacion_LCD` usa en el PC— también corta los pines y además
> > ahorra **524 B por punta** (medido: Maestro `57300` B, Esclavo `41628` B). **Se descartó porque
> > cambia la FORMA del bloque, y dos packs lo leen por texto**: `flash_01_lastre` exige que el
> > transporte acabe en `_SW_SPI`, y `enlace_01_transporte` lee estos mismos argumentos para
> > comprobar que *«el constructor del display no vuelve a reclamar el pin del puerto»*. Con la
> > variante nula los dos caían a **`ABORTADO`** — y son precisamente los que vigilan el bus de la
> > pantalla y su choque con el puerto serie, o sea **lo que este cambio arregla**. Apagar al
> > vigilante mientras se toca lo que vigila es N-75; **524 B contra dos instrumentos que dejan de
> > medir es N-89, y N-89 dice que se rechaza**.
>
> Lo vigila el pack **`costura_11_lcd_sin_bus`**, que exige que **ningún** fichero de ninguna punta
> haga `pinMode` ni `digitalWrite/Read` sobre esos tres hilos —censando el directorio, no una lista
> escrita a mano— y que el constructor **reciba `U8X8_PIN_NONE` en todos sus argumentos de pin**,
> leídos del C++. Y lleva una comprobación en el sentido contrario —que `drawStr` y `sendBuffer`
> sigan existiendo—, para que **el pack no se pueda poner en verde vaciando `lcd.cpp`**, que es
> precisamente lo que se decidió no hacer.
>
> ---
>
> ### 📋 OPTIMIZACIÓN PENDIENTE, **NO EJECUTADA**: retirar la pantalla del todo
>
> **Esto NO está aprobado ni ha pasado banco. Está aplazado, y con su condición escrita.**
>
> **Qué liberaría, y con qué grado de certeza cada cifra:**
>
> | | cifra | ¿de dónde sale? |
> |---|---|---|
> | `PB3`, `PB4`, `PB5` | 3 pines | **Ya libres eléctricamente desde el 31/08.** Lo que queda es liberarlos del binario. `PB6`/`PB7` se los llevó el Bluetooth en N-76 |
> | Flash | **~18,9 KB** | 🔶 **ESTIMACIÓN de un censo, NO una compilación.** Nadie ha compilado todavía una versión sin pantalla y restado los dos extremos. Hasta que eso se haga, esta cifra no se publica como medida |
>
> > ### 📐 01/09/2026 — LO QUE SIGUE ENLAZADO, MEDIDO POR FICHERO OBJETO. Y LA FILA QUE FALTABA: LA RAM
> >
> > **La distinción que esta entrada tenía que hacer y no hacía explícita: el BUS ya no está; la PILA
> > del LCD sí.** Son dos cosas distintas y sólo una se resolvió el 31/08.
> >
> > | | estado hoy |
> > |---|---|
> > | **El bus** —`PB3` `SCLK`, `PB4` `CS`, `PB5` `SID`— | **fuera.** Los cuatro argumentos de pin son `U8X8_PIN_NONE` (`Maestro/src/lcd.cpp:74-75`), no queda ni un `pinMode` ni un `digitalWrite`, y lo vigila `costura_11_lcd_sin_bus` |
> > | **La pila** —`libU8g2.a`, `lcd.cpp`, `menu.cpp`, `modo_hora.cpp`— | **dentro, entera.** Se compila, se enlaza, compone el framebuffer en cada vuelta y **lo tira**. Cortar el cable no descuenta un byte, y por eso el delta del 31/08 fue exactamente cero |
> >
> > **MEDIDO el 01/09 sobre `Maestro/.pio/build/maestro/firmware.map`** (HEAD `aa69349`), por **fichero
> > objeto** y sólo sobre las secciones **retenidas** —las que caen en `0x0800xxxx`; el bloque
> > *Discarded input sections* se descarta antes de sumar. **Ahí se equivoca este censo si se hace de
> > prisa, y ahí se equivocó la primera pasada de este mismo párrafo**: contando los descartados el
> > mapa suma 14 MB, o sea 240 veces la flash del micro, que es la señal de que se está midiendo mal.
> >
> > ```
> > total atribuido      58.356 B   (el acta dice 58.296 usados: cuadra en 60 B de tabla de vectores)
> >   firmware propio    23.768 B
> >   core / libc        20.652 B
> >   libU8g2.a          13.936 B  <-- de los cuales 9.483 B son u8g2_fonts.c.o
> >
> > ficheros propios de pantalla   lcd.cpp.o 4.430 · modo_hora.cpp.o 748
> >                                menu.cpp.o 390 · modo_alcance.cpp.o 272   = 5.840 B
> > ```
> >
> > **`13.936 + 5.840 = 19.776 B ≈ 19,3 KB.`** Corrobora la estimación de `~18,9 KB` de la fila de
> > arriba, **y no la sustituye**: es un **TECHO**, no un delta. Lo que se ahorra de verdad sólo lo dice
> > compilar sin pantalla y restar los dos extremos (§7 de `CLAUDE.md`), y hay dos motivos por los que
> > el número real será distinto —`modo_alcance.cpp` es un modo, no sólo una pantalla, y retirar la
> > pila arrastra además lo que sólo ella usaba del core—.
> >
> > #### 🔴 Y la fila que esta tabla no tenía: **1.024 B de RAM**
> >
> > ```
> > RAM atribuida por objeto (0x2000xxxx)         [MEDIDO 01/09, mismo mapa]
> >   1.024 B  libU8g2.a(u8g2_d_memory.c.o)   <-- el framebuffer 128x64 completo
> >     188 B  src/lcd.cpp.o
> >      41 B  src/menu.cpp.o
> >   -------
> >   3.386 B  RAM estatica total atribuida del Maestro
> > ```
> >
> > **Es el 30 % de la RAM estática del Maestro, por una pantalla que no está conectada.** Un
> > censo que sólo mirase flash no lo habría visto: es exactamente N-86 —*«un camino muerto que no
> > cuesta flash puede seguir costando RAM»*—, y aquí ni siquiera hace falta que el enlazador no pueda
> > tirarlo: el framebuffer se usa de verdad, se compone entero cada vuelta y se descarta.
> >
> > **Esto NO reabre la decisión.** El responsable pidió no quitar el LCD del firmware mientras la
> > memoria alcance, y alcanza. Lo que hace es **poner el precio completo donde se pueda leer**: quien
> > algún día necesite 1 KB de RAM tiene aquí medido de dónde sale, sin volver a hacer el censo.
>
> **Lo que cuesta, con la misma letra que lo que gana:**
>
> 🔴 **TACHADO EN LA MUDANZA (12/09/2026): el punto 1 de abajo está REFUTADO POR SU PROPIO
> BLOQUE ANIDADO** —el `> 🔴 REFUTADO EL 01/09` de tres líneas más abajo— y seguía sin tachar.
> La vía del menú no existe: `Esclavo/src/botones.cpp` da `botonAceptar() { return false; }`.
>
> 1. 🔴 **`menu.cpp:215` es UNA DE LAS TRES VÍAS que sacan al Esclavo del Modo Degradado.** Las otras
>    dos son `mando.cpp` y la puerta automática de `main.cpp:385`. **La app NO puede** — es el defecto
>    **N-106**, abierto y con su pack en rojo. Retirar el menú hoy **elimina una vía de seguridad
>    mientras otra sigue rota**.
>
>    > 🔴 **REFUTADO EL 01/09, y en la dirección INCÓMODA: esa vía ya no existe, y nadie lo había
>    > apuntado aquí.** `Esclavo/src/menu.cpp:210` pide `aceptar` para llamar a `degradado_salir()`, y
>    > `aceptar` sale de `botonAceptar()`, que desde el 31/08 es `return false;`
>    > (`Esclavo/src/botones.cpp:294`) porque su pin `PB15` es una cámara. **La vía del menú está
>    > muerta hoy, con el menú entero todavía en el binario.**
>    >
>    > Así que el coste real de retirar el menú **no es perder una vía**: es que **ya sólo quedan dos**
>    > —el mando (`A·A·A` → `ACC_OBEDECER`, `B·B·B` → `ACC_AMBAR`) y la puerta automática— y **N-106
>    > sigue abierto**. El argumento no se debilita: cambia de sitio. Lo que bloquea sigue siendo
>    > N-106, y ahora sin el colchón que este punto creía tener.
>
> 2. **Se van las 271 comprobaciones de `Validacion_LCD`**, y no todas son de pantalla: ese arnés
>    enlaza `modo_degradado.cpp`, que es **SFTY-21**, y es el único sitio donde esa máquina de estados
>    se compila como C++ real para el PC. *(Sigue vigente: `271/271` en el acta del 01/09.)*
> 3. **`esclavo_02_inhibicion_menu` es el único pack etiquetado `# EJERCE SFTY-21` cuyo sujeto
>    desaparecería entero**, y con él `maestro_06_fuentes_pantalla` y `maestro_07_menu_opciones`.
>
>    > ⚠️ **Matiz del 01/09, y hay que leerlo antes de fiarse de ese pack.** Su sujeto no sólo
>    > desaparecería: **ya es inalcanzable en la tarjeta**. El pack llega a `P_DEGRADADO`,
>    > `P_CONFIRMAR` y `P_RECHAZO` pulsando el botón `2` sobre `banco/modelos/esclavo.py`, y ese modelo
>    > **sigue teniendo cuatro botones** (`consumir_boton(2)` = aceptar, `(3)` = cancelar), mientras el
>    > firmware tiene dos. El pack mide una lógica que existe y es correcta, sobre un camino que ningún
>    > operario puede recorrer. **No es una etiqueta que mienta: es un modelo que se quedó atrás.**
>    > Corregirlo es del que lleve `banco/modelos/` — se anota aquí porque es lo que sostiene una fila
>    > de la tabla de arriba.
>
> **La condición que la desbloquea** —y es lo más útil de esta entrada—: esto se vuelve razonable
> **cuando N-106 esté cerrado (la app puede sacar al Esclavo del Degradado) y se haya decidido qué
> pasa con las 271 comprobaciones de `Validacion_LCD`**. Antes de eso, no.
>
> **Por qué no se hace hoy: la memoria alcanza.** ~~El Maestro está al **88,2 %**, con **7.712 B
> libres**, y el Esclavo al **64,3 %**, con **23.384 B**.~~ **Cifras del acta del 01/09/2026**
> (`evidencia/2026-09-01_compuerta.txt`, HEAD `aa69349`, filas *compila maestro* y *compila esclavo*):
>
> | | usado | de 65.536 B | libres |
> |---|---|---|---|
> | Maestro | `58296` B | **89,0 %** | **7.240 B** |
> | Esclavo | `43192` B | **65,9 %** | **22.344 B** |
>
> Las tachadas eran las del 31/08 y **ya no salen de la última corrida**. Se dejan visibles porque lo
> que importa no es el número suelto sino el sentido: **el margen del Maestro se estrechó 472 B en un
> día**, y el acta se regeneró **dos veces mientras se escribía esta auditoría** —`58188` B primero,
> `58296` B después—. **Una optimización que no hace falta es riesgo sin contrapartida** — pero el
> margen que la sostiene se está gastando a ~0,5 KB/día, y el día que deje de sostenerla hay que
> decirlo aquí.
>
> ⚠️ **Y la regla de higiene que este párrafo estrena, porque le pasó:** una cifra de flash **se copia
> del acta en el momento de escribirla y se cita con el HEAD del acta**. Sin el HEAD al lado, quien la
> lea no puede saber contra qué binario vale, y en un árbol con varios agentes trabajando eso caduca
> en horas, no en semanas.
>
> ⚠️ **Y que nadie la reabra por el ruido: el ruido en el conector se acabó el 31/08.** Lo que queda
> pendiente es **sólo la flash**, y la flash hoy sobra.
>
> **Referencias cruzadas:** **N-91** (el presupuesto de retirar la pantalla) · **N-106** (la vía del
> Degradado que lo bloquea) · **Fase 3** de la hoja de ruta.

---

## 📌 Reglas de Seguridad Inquebrantables (SFTY-1 a SFTY-18)

- **SFTY-1:** Watchdog Timer IWDG activo a **4.0s** en Maestro y Esclavo STM32 (`IWatchdog.begin(4000000)`), con refresco obligatorio en `loop()`. *El Repetidor ESP32 no implementa watchdog.*
- **SFTY-2:** Enclavamiento por hardware/software que prohíbe luz Verde y Rojo simultáneas en la misma cara.
- **SFTY-3:** Suma de verificación polinomial **CRC-8 Maxim (`0x31`)** en todos los paquetes RF.
- **SFTY-4:** Lógica de despeje **All-Red (Rojo Fijo en ambos semáforos)** con tiempo configurable de **10 a 90 s**. El piso de **10 s** es inquebrantable por software —`modoAutomatico_fijarTiempos()` hace `return false` fuera de rango, venga del menú o de la radio— así que no es posible configurar despeje nulo. El par vive una sola vez, en `Maestro/src/modo_automatico.cpp:34` (`DESPEJE_SEG_MIN = 10, DESPEJE_SEG_MAX = 90`), y lo releen `documentos_04_cifras_sin_vigilante` y el arnés del automático en cada corrida. *Estuvo publicado aquí como ~~«5 a 999s», con piso de 5s~~ hasta el 31/08/2026: el piso era la mitad del real, el techo once veces el real, y **999 no cabía siquiera en el `uint8_t` que transporta el valor** — ninguna versión del firmware pudo aceptarlo nunca.*
- **SFTY-5:** Transición de luz legal en Colombia (Res. 2024): Verde $\rightarrow$ Rojo Directo (0s); Rojo $\rightarrow$ Amarillo Fijo (4.0s) $\rightarrow$ Verde.
- **SFTY-6:** Timeout de fallback a **25.0s** sin PONG/PING para entrar a **🟡 Amarillo Intermitente** en ambos lados. El umbral vive una sola vez, en `*/include/protocolo.h` (`SFTY6_SILENCIO_MS = 25000UL`, idéntico en las dos puntas). *Estuvo publicado aquí como **12.0s** hasta el 31/08/2026: era el valor anterior a **N-71**, y esa cifra es justamente la que no cabía por encima de los reintentos.*
- **SFTY-7:** Reintento automático de órdenes ACK cada **3.5s** (`TIMEOUT_ACK_MS`), **validado en campo el 31/07/2026** con la tasa aérea a 2.4 kbps. El fallback de seguridad de 25.0s (SFTY-6) es el **techo** de la ventana, y la cuenta que lo sostiene sale del C++: los **5 reintentos** de `CICLO_MAX_REINTENTOS` a 3.5s (≈3.56s con el aire de la ráfaga), más los 3.0s de cadencia del latido, dan **~20,8s de peor caso** bajo un techo de 25.0s. **Con el techo de 12.0s que se publicaba antes de N-71 sólo cabían 2 o 3: los reintentos 4 y 5 eran código muerto**, porque el ámbar por orfandad saltaba primero. La desigualdad la recalcula `costura_09_presupuesto_radio` desde las constantes en cada corrida, en vez de vivir en esta frase. *No se sube "por si acaso" ante mayor distancia: la distancia aumenta la probabilidad de pérdida, no la latencia, y contra una trama perdida sirve repetir, no esperar.*
- **SFTY-8:** Repetidor ESP32 asíncrono. *Desde V8.3 la liberación del bus ya no es por ventana de silencio de 5 ms sino inmediata al terminar cada trama — ver SFTY-16.*
- **SFTY-9:** **Self-Healing Autónomo**: Reconexión de red sin reinicio manual, antecedida por 15s de All-Red de seguridad.
- **SFTY-10:** Ventana deslizante (`memmove`) para rescatar paquetes RF en ambientes con ruido eléctrico o pérdida de bytes.
- **SFTY-11:** Transmisión en ráfaga configurable vía `RF_BURST_COPIES` (`protocolo.h`), **fijada en 3 copias**. El coste de la ráfaga lo determina la **tasa aérea**, no el protocolo: a 2.4 kbps son ~0.13s de aire (despreciable), mientras que a 0.3 kbps eran ~2.2s y ahí sí saturaban el canal. Con la tasa corregida la redundancia vuelve a ser barata, y es **la palanca correcta frente a distancias variables** en equipos móviles.
- **SFTY-12:** **Navegación de Menú Independiente**: Mantiene a Maestro y Esclavo en **🔴 ROJO FIJO CONTINUO** (o Amarillo Intermitente sin coms).
- **SFTY-13:** **Supresión Anti-Colisión de PING**: Suprime el Heartbeat PING durante espera de ACK para evitar colisiones RS485.
- **SFTY-14 (V8.1):** **Telemetría de calidad de enlace**. Sobre el latido de 3 s que ya existe se mide si hubo respuesta y cuánto tardó, en ventana deslizante de 10 latidos. Se expone en los modos de operación (`RF:100% 340ms`) y en la pantalla **PRUEBA ALCANCE**. Solo se acepta como respuesta el comando que corresponde (`PONG` a un `PING`, `ACK_RED` a un `GO_RED`): aceptar cualquier paquete falsearía la medida al alza. No requiere soporte de la radio ni cambios de protocolo.
- **SFTY-15 (V8.3):** **Diagnóstico de línea**. `protocolo.cpp` cuenta bytes recibidos, tramas válidas y tramas descartadas por CRC. La pantalla **PRUEBA ALCANCE** los muestra en su fila inferior, separando tres fallos que antes se veían todos como "no hay comunicación": `RX 0 - nada llega` (cobertura, canal o antena), `RX 4k - BASURA` (llegan bytes pero ninguna trama válida: cableado, línea flotando o radio atascada) y `RX 36 9 tr` (enlace correcto). Los contadores se ponen a cero al entrar a la pantalla.
- **SFTY-16 (V8.3):** **Puente que valida antes de retransmitir**. El repetidor ESP32 dejó de ser un passthrough ciego: ahora reconoce el formato (4 bytes con CRC-8 Maxim) y **solo relaya tramas válidas**. Si el par RS485 de entrada queda flotando, el ruido se descarta dentro del ESP32 y no llega al aire. Antes, ese ruido mantenía la transmisión permanentemente activa y la radio de salida saturaba el canal (fallo de campo del 31/07: LED TX fijo en B2). Además la transmisión solo se activa cuando hay algo real que enviar, no ante el primer byte. Compilar con `-D PUENTE_TRANSPARENTE` revierte al comportamiento anterior.
- **SFTY-17 (V8.4):** **Retardo de cortesía del Esclavo antes de responder** (`RETARDO_RESPUESTA_MS = 200`). En modo repetidor hay una radio intermedia (B2) que acaba de **transmitir** la orden y necesita tiempo para volver a **recepción**. Si el Esclavo contesta de inmediato, su respuesta sale mientras B2 sigue conmutando y **B2 no la oye**: el enlace funciona en un sentido y no vuelve nada. Observado en campo el 31/07 con el contador del puente marcando `C<-Esclavo = 1 byte` en dos minutos mientras la ida fluía. La respuesta se **programa**, no se bloquea el bucle, así que el parpadeo de ámbar y el watchdog siguen atendidos. En enlace directo es inofensivo: el Maestro espera hasta 3.500 ms.
- **SFTY-18 (V8.5):** **Reloj de tiempo real con hora declarada no fiable por defecto**. El Maestro usa el RTC interno del STM32 con el cristal `Y2` de 32.768 kHz que la tarjeta ya traía y una pila CR2032 en `VBAT` (ver [`03_Hardware_Tarjeta/MAPEO_TARJETA_KICAD.md`](03_Hardware_Tarjeta/MAPEO_TARJETA_KICAD.md) §4). No ocupa ningún pin: el I²C por hardware está copado por LCD y RS-485, así que un módulo externo habría obligado a I²C por software. **La regla de seguridad no es tener reloj, es saber cuándo no se tiene:** al ajustar la hora se escribe también un año marcador, y al arrancar `reloj_enHora()` solo devuelve `true` si ese marcador sobrevivió. Pila agotada, primera puesta en marcha o dominio de respaldo corrupto ⇒ **`false`**, y toda función que dependa de la hora debe abstenerse. Un reloj sin poner en hora que se cree válido es peor que no tener reloj: activaría la operación nocturna a deshora. ~~**V8.6:** se añade la pantalla **AJUSTAR HORA** —quinta opción del menú— que es la única vía para poner el reloj y, por tanto, el requisito previo de SFTY-20 y SFTY-21. Se edita **dígito a dígito** con el dígito activo subrayado: con un solo botón de subir, poner los minutos como valor completo costaría hasta 59 pulsaciones, y además la edición por dígitos **funciona igual con el mando de relés**, que solo entrega pulsos y no admite repetición por mantener pulsado. Se trabaja sobre una copia y solo se escribe al RTC al confirmar, de modo que entrar por error y salir con el Botón 4 no altera la hora. La pantalla **no arranca ciclos**: mantiene el mismo estado seguro que el menú.~~

> 🔴 **TACHADO EL 05/09/2026: `MODO_HORA` ES INALCANZABLE, ASÍ QUE «LA ÚNICA VÍA PARA PONER EL
> RELOJ» NO ES NINGUNA VÍA.** Y en la tabla de trazabilidad de este mismo documento esa frase vale
> doble: describía como cubierto el requisito previo de SFTY-20 y SFTY-21.
>
> **Medido, no razonado.** El modo tiene **un solo armador** y cuelga de un botón que hoy devuelve
> `false`:
>
> ```
> $ grep -r "modoActual_set(MODO_HORA)" 01_Firmware/Maestro 01_Firmware/Esclavo --include=*.cpp
> 01_Firmware/Maestro/src/menu.cpp:        case 1:  modoActual_set(MODO_HORA);      break;
>
> $ grep -r "bool botonAceptar" 01_Firmware/Maestro/src/botones.cpp
> bool botonAceptar() { return false; }
> ```
>
> Ese `case` vive dentro de `if (botonAceptar())`. Con la pantalla y la botonera retiradas el
> 05/09, **la rama no corre nunca** — y «salir con el Botón 4» tampoco: `botonCancelar()` es
> igualmente `return false;`. La segunda mitad del párrafo —edición dígito a dígito, copia de
> trabajo, no arranca ciclos— sigue siendo **cierta sobre el código** de `modo_hora.cpp`; lo que
> ha dejado de existir es **la puerta**.
>
> ✅ **EL CAMINO VIVO ES BLUETOOTH CONTRA EL ESP32 DE `J17`, QUE ES DONDE ESTÁ EL RELOJ CON
> PILA:** `CMD:SET_RTC:<...>` lo escribe y `CMD:LEER_RTC` lo consulta
> (`01_Firmware/ESP32_Expansion/src/despachador.cpp`, `01_Firmware/Maestro/src/bluetooth.cpp`).
> **La regla de seguridad de SFTY-18 no cambia** —`reloj_enHora()` sigue siendo el que decide, y
> toda función que dependa de la hora sigue debiendo abstenerse cuando dice `false`—. Lo que
> cambia es por dónde entra la hora.
>
> **Se tacha en vez de borrarse** porque la frase «quinta opción del menú» está copiada en varios
> manuales y volvería a proponerse; y porque *declarar no es ejercer* (§2.ter de `CLAUDE.md`): esta
> línea llevaba meses describiendo una vía que ningún operario podía recorrer, y ningún instrumento
> la contradecía porque las frases no se compilan.

---

## 🔗 Trazabilidad requisito → implementación

Cierra el pendiente **N-6**. Cada regla se localiza en el código por su etiqueta `SFTY-x`; esta tabla
se obtiene **buscando esas etiquetas en los fuentes**, no escribiéndola a mano, así que una regla sin
implementar aparece vacía en vez de aparentar cobertura.

> ### 🆕 Tercera columna (03/08/2026): qué lo DEMUESTRA
>
> *"Dónde vive"* dice que la regla está escrita. **No dice que se cumpla.** La tercera columna
> nombra el pack del banco que la ejerce, y se levanta igual que las otras dos: **buscando** la
> etiqueta `# EJERCE SFTY-x` en `banco/packs/`, no escribiéndola a mano.
>
> **Una fila con la tercera columna vacía es una regla sin evidencia automática**, y se ve de un
> vistazo. Hoy la mayoría lo está: es el trabajo que queda, y es exactamente lo que un auditor
> funcional viene a buscar.
>
> Solo se etiqueta lo que el pack **ejerce de verdad**. Etiquetar de más convertiría esta tabla en
> un adorno — una regla apareciendo cubierta por una prueba que no la comprueba es **peor** que
> una fila vacía, porque la vacía al menos no miente.
>
> Correr una sola: `python 01_Firmware/Simulaciones/banco/correr.py --pack <nombre>`
>
> 🔴 **Y esa promesa estuvo sin instrumento hasta el 27/08 (N-62).** La tabla decía levantarse
> del `grep` y estaba escrita a mano: la fila de `SFTY-2` citaba un solo pack cuando ya había
> **tres** etiquetados —`barrera_02_dos_puntas` y `esclavo_06_no_abre_paso` llevaban días con su
> `# EJERCE` puesto—. Es la misma clase de fallo que las cifras del README: una frase que dice
> *"derivado"* encima de algo derivado una vez y copiado después. Ahora lo comprueba el pack
> **`documentos_02_trazabilidad_sfty`**, y **en las dos direcciones**: ninguna etiqueta sin fila
> y ninguna fila citando un pack que no la declare —la regla con la que se retiraron los tres
> monolitos, aplicada a un documento—.

| Regla | Dónde vive | Qué lo demuestra |
|---|---|---|
| SFTY-1 | `Maestro/src/main.cpp` · `Esclavo/src/main.cpp` | — |
| SFTY-2 | `Maestro/src/semaforo.cpp` · `Esclavo/src/semaforo.cpp` | ✅ `barrera_01_pines_de_luz` · `barrera_02_dos_puntas` · `esclavo_06_no_abre_paso` · `maestro_09_test_leds` |
| SFTY-3 | `*/src/protocolo.cpp` · `Repetidor/src/main.cpp` | — |
| SFTY-4 | `Maestro/src/coordinador.cpp` · `Maestro/src/modo_automatico.cpp` | — |
| SFTY-5 | `Maestro/src/semaforo.cpp` · `Esclavo/src/semaforo.cpp` | ✅ `Validacion_Automatico/arnes_automatico.cpp` — **arnés C++, invisible para el censo de packs**, ver abajo |
| **SFTY-6** | `*/include/protocolo.h` *(el umbral)* · `Maestro/src/coordinador.cpp` · `Esclavo/src/main.cpp` | ✅ `costura_08_silencio` · `costura_09_presupuesto_radio` · `costura_13_ambar_ordenado` · `maestro_04_sync_horaria` |
| SFTY-7 | `Maestro/include/coordinador.h` · `Maestro/src/coordinador.cpp` | — |
| SFTY-8 | `*/src/protocolo.cpp` | — |
| SFTY-9 | `Maestro/src/coordinador.cpp` · `Maestro/src/main.cpp` | — |
| SFTY-10 | `*/src/protocolo.cpp` | — |
| SFTY-11 | `*/include/protocolo.h` · `*/src/protocolo.cpp` | — |
| SFTY-12 | `Maestro/src/coordinador.cpp` · `Maestro/src/modo_manual.cpp` | — |
| SFTY-13 | `Maestro/src/coordinador.cpp` | — |
| SFTY-14 | `Maestro/src/coordinador.cpp` | — |
| SFTY-15 | `*/src/protocolo.cpp` · `Maestro/src/lcd.cpp` · `Maestro/src/modo_alcance.cpp` | — |
| SFTY-16 | `Repetidor/src/main.cpp` | — |
| SFTY-17 | `Esclavo/src/main.cpp` | — |
| **SFTY-18** | `Maestro/src/reloj.cpp` · `Maestro/include/reloj.h` · `ESP32_Expansion` *(el reloj del puente)* | ✅ `esp32_04_osf` · ✅ `esp32_11_bien_formada_no_es_cierta` |
| **SFTY-19** | **— solo diseño, ver abajo.** La única mención en el código es una advertencia en `reloj.h` aclarando que este modo **no** se apoya en el RTC | — |
| SFTY-20 | `Maestro/include/modo_hora.h` · `ESP32_Expansion/include/reloj_ds3231.h` *(sólo referencias; el modo no existe)* | — |
| **SFTY-21** | `*/src/modo_degradado.cpp` · `*/src/mando.cpp` · `*/include/ciclo_degradado.h` | ✅ `esclavo_01_latch_ambar` · `esclavo_02_inhibicion_menu` · `esclavo_07_ambar_emergencia` · `esclavo_08_ambar_en_degradado` · `maestro_01_mando` · `maestro_05_ciclo_sin_radio` · `costura_02_fase_ciclo` · `costura_06_reanudacion` · `costura_12_margen_deriva` · `costura_13_ambar_ordenado` · `costura_14_cancela_ambar` · `camara_02_j16` |
| SFTY-22 | `Maestro/src/lcd.cpp` · `Maestro/include/modo_ambar.h` *(sólo referencias; la pantalla no existe)* | — |
| **SFTY-23** | `Maestro/src/coordinador.cpp` · `Esclavo/src/config_ciclo.cpp` *(Fase 2)* · `*/src/reloj.cpp` | ✅ `esclavo_03_par_config` · `esclavo_04_desfase` · `esclavo_05_hora_atomica` · `maestro_04_sync_horaria` |
| SFTY-24 | **— solo diseño.** Cero etiquetas en el firmware | — |
| SFTY-25 | **— solo diseño.** Cero etiquetas en el firmware | — |
| SFTY-26 | **— solo diseño.** Cero etiquetas en el firmware | — |
| **SFTY-27** | 🔴 **DOS REGLAS CON EL MISMO NÚMERO — ver el aviso justo debajo.** Este documento lo define como *«matrícula de pareja»*, sin implementar; las **8 etiquetas del firmware** dicen otra cosa | — |
| **SFTY-28** | `*/src/semaforo.cpp` *(dentro de `escribirPines()`)* · `*/include/pines.h` | ✅ `barrera_03_talanquera` · `maestro_09_test_leds` |
| **SFTY-29** | **— solo diseño.** Presencia como veto del todo-rojo y sensor de pluma | — |

> ### 🔴 01/09/2026 — AUDITORÍA DE LA TABLA. Seis filas faltaban, y una de ellas tapaba una colisión de número
>
> **Antes de esta pasada la tabla tenía 23 filas para 29 reglas definidas en este documento.** Faltaban
> `SFTY-20`, `22`, `24`, `25`, `26` y `27`. Una regla que no tiene fila **no aparece vacía: no aparece**,
> y las dos direcciones que vigila `documentos_02_trazabilidad_sfty` no la echan de menos, porque ese
> pack sólo cruza *etiquetas de pack* contra *filas*, y ninguna de las seis tenía etiqueta de pack.
> Añadidas arriba con lo que mide el `grep`, no con lo que se recordaba.
>
> **Censo de etiquetas en el firmware [MEDIDO 01/09]**, sobre `Maestro/`, `Esclavo/`, `Repetidor/` y
> `ESP32_Expansion/`:
>
> ```
> grep -rl "SFTY-27\b" 01_Firmware/{Maestro,Esclavo}/{src,include}
>   -> 8 ficheros:  Maestro/src/bluetooth.cpp · Maestro/src/botones.cpp
>                   Maestro/include/botones.h · Maestro/include/demanda.h
>                   Esclavo/src/bluetooth.cpp · Esclavo/src/botones.cpp
>                   Esclavo/include/botones.h · Esclavo/include/demanda.h
> ```
>
> #### 🔴 `SFTY-27` DESIGNA DOS REGLAS DISTINTAS, y CUATRO sitios mandan a leer la equivocada
>
> ⚠️ **CORREGIDO EL 01/09: aqui se publico "ocho" y era una cifra de otra propiedad.** El `8` es el
> numero de **ficheros de firmware que llevan la etiqueta** —lo que mide el `grep -rl` citado mas
> abajo— y se copio a la frase *"ocho mandan a leer la equivocada"*, **que nunca se habia medido**.
> Medido de verdad: **17 sitios usan el numero** (8 firmware · 2 packs · 2 `ARQUITECTURA.map` ·
> 5 manuales) y **CUATRO remiten explicitamente** a este documento —`Esclavo/include/demanda.h:19`,
> `Esclavo/src/bluetooth.cpp:456`, y dos manuales ya corregidos—.
>
> **Y el buscador casi falla, que es la parte que hay que recordar:** el cuarto puntero **no lo
> encuentra** un `grep` que exija `SFTY-27` y `OPTIMIZACIONES` en la MISMA linea, porque el ajuste
> de linea del Markdown los deja en dos. Con una linea salen 3; con `-B2 -A2`, 4.
>
> | dónde | qué llama `SFTY-27` | estado |
> |---|---|---|
> | **Este documento**, §`SFTY-27` (más abajo) | *«Matrícula de pareja: quién obedece a quién»* | **DISEÑO, NO IMPLEMENTADO** |
> | **8 sitios del firmware** + 2 packs + 3 manuales | *«el Esclavo PIDE y el Maestro DECIDE»* | **IMPLEMENTADA y viva** — es la asimetría de `demanda_solicitar()` |
>
> **Ocho de esos sitios remiten explícitamente a `OPTIMIZACIONES.md § SFTY-27`.** Quien siga el puntero
> lee la regla equivocada, y encima la lee marcada *«NO IMPLEMENTADO»* sobre una regla vial que **sí**
> corre. Cada mitad se lee coherente por separado: hay que abrir los dos sitios a la vez para verlo, que
> es por lo que llevaba semanas de pie.
>
> **Renumerar es decisión del responsable** (`AB-8` de `INDICE_CRUZADO.md`, `P-3`), **no se hace desde
> esta auditoría**: tocar el número obliga a cambiar 13 sitios a la vez, y un número de regla a medio
> cambiar es peor que uno duplicado. Lo que sí se hace aquí es **que el puntero deje de engañar**: la
> fila de arriba y este aviso.
>
> #### Lo que la segunda columna NO es, y conviene decirlo
>
> La cabecera promete que la tabla se levanta *«buscando esas etiquetas en los fuentes»*. **Para la
> tercera columna eso es cierto y lo comprueba un pack; para la segunda no lo comprueba nadie**, y se
> nota:
>
> - **`SFTY-5`**: la fila dice `*/src/semaforo.cpp`, que es donde vive la regla. Pero
>   `grep -rl "SFTY-5\b"` sobre el firmware devuelve **sólo `*/src/protocolo.cpp`** [MEDIDO 01/09] —el
>   rastro de la etiqueta impostora, que hoy es un comentario que deja constancia del error
>   (`Maestro/src/protocolo.cpp:41-43`) y **por eso mismo sigue apareciendo en el `grep`**.
> - **`SFTY-21`**: la fila cita tres ficheros; el `grep` devuelve **35** en las dos puntas. La fila es un
>   resumen curado, no un censo.
>
> Ninguna de las dos cosas es un defecto de la fila — son defectos de la **frase que dice cómo se
> levanta**. Queda escrito para que nadie vuelva a leer la segunda columna como si fuera medida.

### Discrepancias corregidas al levantar esta tabla

- **La ventana deslizante estaba etiquetada `SFTY-11` en el código** cuando `SFTY-11` es la ráfaga.
  Corregido a `SFTY-10` en Maestro y Esclavo. Era el caso concreto que describía N-6.
- **SFTY-14 y SFTY-16 no tenían etiqueta** pese a estar implementadas: la telemetría en
  `coordinador.cpp` y la validación del puente en `Repetidor/src/main.cpp`. Añadidas.
- **`reloj.h` afirmaba que `reloj_segundosDelDia()` era la base de SFTY-19**, lo que contradice el
  propio diseño de SFTY-19 (sincronización *relativa*, sin RTC). Reescrito como advertencia explícita
  para que nadie ancle las dos unidades a su reloj de pared.

### 🔴 28/08/2026 — `SFTY-5` era DOS cosas con la misma etiqueta, y la fila estaba mal en las dos columnas

**La etiqueta estaba duplicada.** `SFTY-5` está definida arriba como *«transición de luz legal en
Colombia (Res. 2024)»*, pero en `Maestro/src/protocolo.cpp` y `Esclavo/src/protocolo.cpp` la línea

```cpp
static HardwareSerial AiBus(RS485_IN_RX, RS485_IN_TX); // SFTY-5: Segundo bus UART para IA
```

usaba *«SFTY-5»* para nombrar el **puerto serie de la cámara IA**, que no es una regla de seguridad
ni tiene nada que ver con la transición de luz. Dos cosas distintas con la misma etiqueta, y la tabla
se levanta **buscando la etiqueta**: por eso la fila apuntaba a `*/src/protocolo.cpp`.

**Consecuencia medida sobre el fuente (28/08):**

| | estaba | está |
|---|---|---|
| *Dónde vive* | `*/src/protocolo.cpp` — ruta del comentario impostor | `*/src/semaforo.cpp`, que es donde vive de verdad: `estado == S_AMARILLO && (ahora - tCambio >= 4000)` (línea 258 en el Maestro, 260 en el Esclavo, **idénticas**) |
| *Qué lo demuestra* | vacía | `Validacion_Automatico/arnes_automatico.cpp`, que **relee los 4000 ms del `semaforo.cpp` real** y los ejerce con control negativo |

> **La etiqueta impostora NO se ha borrado del fuente**, y eso es deliberado: `protocolo.cpp` lleva
> hoy un comentario que deja constancia de que esa etiqueta estaba equivocada. Borrarla en silencio
> haría que la próxima persona que la viese volviera a colgarla de la fila de la transición de luz.

### 🕳️ El censo de la tercera columna tiene un hueco, y este caso lo destapó

`documentos_02_trazabilidad_sfty` busca la etiqueta `# EJERCE SFTY-x` **solo dentro de
`banco/packs/`**. Los cuatro arneses que compilan C++ real (§8 de `CLAUDE.md`) son **C++, no packs**,
y por tanto **son invisibles para ese censo**. `SFTY-5` llevaba la tercera columna vacía no porque
nadie la midiera, sino porque **quien la mide no está en el sitio donde se mira**.

> **Es la regla del instrumento aplicada a la propia tabla: un «no aparece» no es un hallazgo hasta
> haber descartado al buscador.** Aquí el buscador era el culpable.
>
> **Esto queda ANOTADO, no arreglado.** El pack está en vuelo en otro proceso y no se toca desde
> aquí. Mientras el censo no mire también los arneses C++, **una fila vacía de esta tabla significa
> «ningún pack la ejerce», no «nada la ejerce»** — y esa diferencia hay que leerla a mano.

### ⚠️ Siete filas vacías, y todas apuntan al mismo fichero

Contadas sobre la tabla de arriba: las filas cuya *«dónde vive»* es `*/src/protocolo.cpp` —**SFTY-3,
5, 7, 8, 10, 11 y 15**— tienen **todas** la tercera columna vacía. Ni un solo pack del banco ejerce
nada de lo que vive en `protocolo.cpp`: ni el CRC-8, ni el reintento de ACK, ni la ventana
deslizante, ni la ráfaga, ni los contadores de diagnóstico de línea.

`CLAUDE.md` advierte del error contrario —*«una regla que aparece cubierta por una prueba que no la
ejerce es peor que una fila vacía, porque la vacía no miente»*—. **Aquí el problema es el inverso, y
también merece quedar escrito:** siete reglas de seguridad seguidas sin evidencia automática **no
son siete casillas pendientes; son un fichero entero sin cobertura**. Una fila vacía suelta se lee
como trabajo por hacer; siete alineadas sobre el mismo `.cpp` señalan **dónde está el agujero**.

**No se inventa cobertura para taparlas.** Se dejan vacías, contadas y con el nombre del fichero
delante, que es lo que un auditor necesita para preguntar por él.

### 🔴 01/09/2026 — QUÉ SIGNIFICA UN ✅ DE ESTA TABLA, Y DOS SITIOS DONDE SIGNIFICA MENOS DE LO QUE PARECE

**Se auditó etiqueta por etiqueta**: las **23 marcas `# EJERCE SFTY-x`** repartidas en **21 packs**
—`maestro_04_sync_horaria` y `maestro_09_test_leds` llevan dos cada uno— sobre los **59 packs** del
banco. Se abrieron una a una y se comparó lo que la etiqueta promete con lo que el pack hace.
**Ninguna cita una regla inexistente y ninguna falta en la tabla**; eso lo sostiene
`documentos_02_trazabilidad_sfty`, que tras esta pasada censa `23 etiquetas en 6 reglas y 29 filas`,
y sigue en verde. **También son honestas las cinco negativas**: `app_03_sin_ok_mudo`,
`app_04_valores_de_status`, `app_06_formato_de_hora`, `esp32_01_watchdog_desigualdad` y
`enlace_01_transporte` **declaran en su cabecera que rozan una regla y NO la ejercen**, y por eso no
llevan etiqueta. Esa disciplina es la que hace que la tabla valga algo. Lo que la auditoría añade son **dos matices
que un ✅ no distingue y un auditor sí necesita**:

**1. Los cuatro packs de `SFTY-2` leen el C++; ninguno lo ejecuta.** [MEDIDO 01/09, abriendo los cuatro]

| pack | qué hace de verdad |
|---|---|
| `barrera_01_pines_de_luz` | `re` sobre `pines.h` y los `.cpp`: **custodia** —que nadie escriba un pin de luz fuera de `semaforo.cpp`—. No evalúa nunca verde-contra-rojo |
| `barrera_02_dos_puntas` | compara el **texto** de `aplicarSalidas()`/`escribirPines()` entre puntas. Buen proxy; **no es un ejercicio** *(así lo calificó también la auditoría externa, N-109 §4)* |
| `esclavo_06_no_abre_paso` | lista blanca de comandos contra el **texto** de `bluetooth.cpp` |
| `maestro_09_test_leds` | `re` sobre `semaforo.cpp`: quién llama a `escribirPines()` y con qué tercer argumento |

**El enclavamiento como tal —«nunca verde y rojo a la vez»— sólo lo EJECUTA
`Validacion_Automatico/arnes_automatico.cpp`, y sólo del Maestro** (§8 de `CLAUDE.md`, y el arnés
está en `71/71` en el acta del 01/09). La custodia y la identidad entre puntas son las condiciones que
hacen creíble el enclavamiento, no el enclavamiento. **Cuatro ✅ en la fila de `SFTY-2` no son cuatro
ejecuciones: son cuatro lecturas del fuente y una sola ejecución, de una sola punta.**

**2. El ✅ de `SFTY-18` cubre el tercero de sus tres sitios, no el primero.** `esp32_04_osf` ejerce el
`OSF` del `DS3231` **del puente ESP32**. La regla tal como está definida arriba —el año marcador del
RTC del STM32 y `reloj_enHora()` como barrera— **no la ejerce ningún pack**:

```
grep -rl "reloj_enHora\|ano marcador" 01_Firmware/Simulaciones/banco/packs/   [MEDIDO 01/09]
  -> app_03_sin_ok_mudo · app_06_formato_de_hora · costura_06_reanudacion
     esp32_03_ack_que_mira · esp32_04_osf
```

y **los dos primeros lo dicen ellos mismos en su cabecera**, negándose a llevar la etiqueta:
`app_06_formato_de_hora` escribe *«este pack no la EJERCE: no comprueba `reloj_enHora()` ni el año
marcador»*. Esa honestidad es lo correcto; **lo que faltaba era decirlo también aquí**, donde se lee
la fila.

> **La regla que queda, y vale para toda la columna:** cuando la segunda columna nombra **varios**
> sitios, un ✅ puede estar cubriendo uno solo. Un auditor que lea esta tabla tiene que cruzar las dos
> columnas, no leer la tercera sola. Lo honesto sería una fila por sitio; mientras no la haya, esto.

---

## 🕹️ SFTY-21 — Modo Degradado por reloj y mando de 4 relés (**IMPLEMENTADO**)

> **Estado:** especificado y **construido en las dos puntas** el 01/08/2026, en la rama
> `feat/n15-reloj-pantalla-hora`. Sustituye y cierra el diseño anterior de **SFTY-19**, que planteaba
> entrada *automática*.
>
> 🔴 **07/09 — EL HARDWARE DEL MANDO YA NO SE MONTA (`D-1`) Y EL CODIGO SIGUE LEYENDO SUS PINES.**
> Eso es lo decidido y es correcto —`mando_ambarLocal()` tiene **cinco llamadas vivas** y su veto
> es esta regla; borrar su armador dejaria los `if` siempre verdaderos, o sea **el veto abierto, no
> inerte**—. **Lo que hay que saber antes de tocar `J16` p5/p8:**
>
> | secuencia | en el Maestro, medido el 07/09 |
> |---|---|
> | `A.B.A.B` -> Degradado | **bloqueada**: la entrada exige reloj en hora y en esa punta nunca lo esta |
> | `B.B.B` -> ambar local | direccion segura |
> | **`A.A.A` -> Modo Automatico** | 🔴 **SIN GUARDA — arranca el ciclo, o sea ABRE PASO** |
>
> `A-2` esta cerrada: **p5/p8 se quedan vacios** ~~y el fin de carrera va a `J14`/`PB0`~~ *(11/09,
> `D-27`: `J14` queda LIBRE y sin cablear; el fin de carrera no se instala en este despliegue)*. La
> consecuencia operativa es dura y va aqui porque es de seguridad: **no se cablea NADA en p5/p8**,
> porque cualquier contacto —un final de carrera, un rebote— compone secuencias que nadie pidio.
> La asimetria estaba medida solo sobre el Esclavo hasta hoy.
>
> **Validación:** los tres firmwares compilan sin warnings propios · simulador funcional **20/20** ·
> simulador de repetidor **10/10** · validación de pantalla **83/83**. Maestro 80,2 % de flash,
> Esclavo 59,7 %.
>
> ⚠️ **Sin prueba de banco todavía.** Nada de esto se ha ejercitado sobre hardware real.
>
> 🔴 **TACHADO EN LA MUDANZA (12/09/2026): CADUCADO. `N-42` se cerró el 04/09 en banco,
> con el responsable delante** (`ceb8cc5`; `ESTADO.md` fila `N-42`, y `roadmap_hist.md` §0.0.bis).
> El párrafo de abajo seguía nombrando a `2779d9b` «sospechoso principal» de una regresión ya
> cerrada. Se marca aquí y no se borra: la hipótesis se investigó y no era la causa.
>
> 🔴 **Y su commit de implementación es hoy el sospechoso principal de la regresión del Modo
> Automático.** `2779d9b` (01/08 **16:00**) cae en la hora exacta en que el banco sitúa el último
> firmware bueno, y toca **15 ficheros del Maestro**. El mecanismo encaja: el mando de relés
> **intercepta las escrituras de pines de luz** en vez de rodearlas —para no dejar colgado al
> coordinador esperando un `S_VERDE` que no llegaría—, y esa intercepción está justo en el camino
> por el que el ciclo avanza. **No confirmado:** pendiente de bisección con firmware real, ver
> [`roadmap_hist.md`](roadmap_hist.md) §N-42 -cerrado en cobre el 04/09, mudado al historico el 07/09- y `05_Funcional/bisect_entregable/`. Se anota aquí porque una
> regla de seguridad cuya implementación está bajo sospecha no puede figurar como
> `IMPLEMENTADO` a secas.
>
> **Parámetros del ciclo degradado:** verde **30 s**, todo-rojo **30 s** —ya ampliado—, ciclo fijo y
> propio, que **no hereda** el verde del Modo Automático. Al ser fijo, el tope de 255 s del byte de
> `CMD_CONFIG` no puede alcanzarse.
>
> ~~**Pendientes conocidos:** el Esclavo **no tiene mando de relés** (N-19) y el estado del modo **no
> persiste** a un corte de energía (N-20).~~
>
> 🔴 **TACHADO EL 05/09/2026: LOS DOS «PENDIENTES CONOCIDOS» ESTÁN CERRADOS DESDE HACE MÁS DE UN
> MES, Y ESTA LÍNEA SEGUÍA ANUNCIÁNDOLOS.** Es peor que una omisión: un pendiente falso hace que
> nadie vaya a mirar si la pieza existe.
>
> - **N-19 se cerró el 01/08/2026.** `01_Firmware/Esclavo/src/mando.cpp` existe, tiene su
>   `ACC_DEGRADADO` y llama a `degradado_entrar()`:
>
>   ```
>   $ grep -r "degradado_entrar()" 01_Firmware/Esclavo/src --include=*.cpp
>   01_Firmware/Esclavo/src/mando.cpp:      // degradado_entrar(): una sola puerta, un solo criterio.
>   01_Firmware/Esclavo/src/mando.cpp:      degradado_entrar();
>   01_Firmware/Esclavo/src/menu.cpp:        ultimoRechazo = degradado_entrar();
>   01_Firmware/Esclavo/src/modo_degradado.cpp:RechazoDegradado degradado_entrar() {
>   01_Firmware/Esclavo/src/modo_degradado.cpp:  if (degradado_entrar() != DEG_ACEPTADO) {
>   ```
>
>   Lo confirma [`04_Manuales/MANUAL_MANDO_4_RELES.md`](04_Manuales/MANUAL_MANDO_4_RELES.md):
>   *«Firmware del mando — Esclavo: ✅ `Esclavo/src/mando.cpp` (añadido el 01/08/2026, N-19)»*.
>   ⚠️ **Lo que SÍ sigue pendiente, y ese manual lo dice en la misma tabla, es el RECEPTOR FÍSICO:
>   no está comprado ni instalado en ninguna de las dos puntas, y nada de esto se ha ejercitado
>   con un mando físico conectado.** Es la distinción de §2.ter: el firmware está *declarado y
>   construido*; lo que nadie ha *ejercido* es el conjunto con hardware.
> - **N-20 se cerró:** el estado sí persiste. `respaldo_guardarDegradado()` lo escribe y `main.cpp`
>   lo borra **en el único punto por el que pasan todas las salidas** —si el modo cambió, el
>   Degradado se acabó—, precisamente para que un olvido en una de las cuatro vías de salida no
>   deje al equipo reanudando un modo del que ya se había salido.

### La decisión de operación

**El controlador no cambia. Solo se añade un modo.**

| Situación | Comportamiento |
|---|---|
| Pérdida de radio | **Ámbar intermitente**, exactamente como hoy. **No se toca** |
| Modo Degradado | **Caso especial de activación MANUAL**, confirmado por un operario |
| Puesta en marcha del Degradado | ~~**Desde la pantalla**~~ **Desde la app por Bluetooth** (`SET_MODO:DEGRADADO`) **o con `A·B·A·B` desde el piso**: validar la hora, confirmar el otro extremo, iniciar y verificar |
| Salida del Degradado | ~~Desde la pantalla~~ **Desde la app** (`SET_MODO:MENU` en el Maestro, `AMBAR_EMERGENCIA` en el Esclavo), **o con `A·A·A` desde el piso** para reintentar Automático |
| Entrada automática | **NUNCA** |

> 🔴 **CORREGIDO EL 05/09/2026: LA PANTALLA SE RETIRÓ, Y ERA EL ÚNICO CAMINO QUE ESTA TABLA
> NOMBRABA PARA ENTRAR.** Los dos caminos vivos estaban construidos y sin escribir aquí. Medido
> sobre el fuente, no razonado:
>
> | | Maestro | Esclavo |
> |---|---|---|
> | **entrar** | app: `SET_MODO:DEGRADADO` → `modo_degradado_evaluarEntrada()` y sólo si da `MDG_OK`, `modoActual_set(MODO_DEGRADADO)` (`Maestro/src/bluetooth.cpp`) · mando: `A·B·A·B` (`Maestro/src/mando.cpp`) | mando: `A·B·A·B` → `degradado_entrar()` (`Esclavo/src/mando.cpp`) |
> | **salir** | app: `SET_MODO:MENU` → `modo_degradado_pedirSalida()`, que pasa por el todo-rojo · mando: `A·A·A` (Automático) o `B·B·B` (Ámbar) | app: `AMBAR_EMERGENCIA` → `salidaDegradadoIniciada()` (`Esclavo/src/bluetooth.cpp`) · mando: `A·A·A` (obedecer) o `B·B·B` (ámbar), **las dos por `degradado_salir()`** |
>
> **La secuencia de ENTRADA es `A·B·A·B`, no `A·A·A`.** `A·A·A` es «a ver si volvió el radio»
> —Automático en el Maestro, volver a obedecer en el Esclavo—, y por eso sí sirve para SALIR. Se
> anota porque el documento nombraba `A·A·A` sin decir cuál era la de entrada, y en el poste eso
> son cuatro pulsos alternados contra tres iguales.
>
> 🟠 **Y UNA ASIMETRÍA MEDIDA ENTRE LAS DOS PUNTAS, que se anota y NO se toca:** las dos vías de
> salida del **Esclavo** por mando pasan por `degradado_salir()` —o sea, por el todo-rojo de
> despedida, y el comentario del fuente dice por qué: *«devolver el mando desde un verde por reloj
> directamente a lo que el Maestro ordene sería encadenar dos autoridades sin cerrar el paso en
> medio»*—. En el **Maestro**, `A·A·A` hace `modoActual_set(MODO_AUTOMATICO)` **sin pasar por
> `modo_degradado_pedirSalida()`**, que es justo la puerta que el despachador de Bluetooth se
> obliga a usar *(«EN DEGRADADO NO SE SALTA AL MENU… es la MISMA puerta que el botón 4»)*. El
> indicador de respaldo sí se borra —`main.cpp` lo hace en el único punto por el que pasan todas
> las salidas—, así que **no hay reanudación fantasma**; lo que se salta es el todo-rojo. **No se
> arregla desde un documento: es firmware y es vial.**
>
> 🔴 **Y EL HUECO QUE ESTE CENSO DESTAPÓ, QUE NO ES UNA ERRATA: EL ESCLAVO NO TIENE CAMINO POR APP
> PARA *ENTRAR* EN DEGRADADO.** Sale por `AMBAR_EMERGENCIA` y entra sólo por el mando de relés
> —cuyo **receptor físico no está comprado**—. Como la entrada es deliberadamente local en cada
> punta *(«un técnico validó ambos extremos»)*, un operario con la app puede poner el Maestro en
> Degradado y **no tiene con qué poner el Esclavo**. Se anota, **no se arregla aquí**: añadir una
> puerta a un modo que enciende un verde sin confirmación del otro extremo es una decisión del
> responsable, no de un documento.
>
> ⚠️ **`salidaDegradadoIniciada()` del Esclavo no es un alias de `degradado_salir()`, y esa
> diferencia es el molde bueno:** `degradado_salir()` es `void` y **abandona en silencio** desde
> `DEG_INACTIVO`, `DEG_SALIENDO` y `DEG_RENDIDO`. El envoltorio pregunta **la misma guarda** antes
> y devuelve `bool`, para que el `$ACK` diga lo que de verdad pasó —`SALIENDO_TODO_ROJO`, no `OK`—
> en vez de ser el «OK mudo» que este repositorio persigue.

### Por qué manual y no automático

En el diseño anterior el equipo entraba solo al perder el enlace. Se descartó, y la razón de fondo es
la asimetría entre los dos avisos:

```
 ÁMBAR INTERMITENTE  ->  "no estoy controlando esto, decide tú"
                         el conductor llega ALERTA, mira, negocia el paso

 VERDE POR RELOJ     ->  "pasa tranquilo, el otro lado está en rojo"
                         el conductor llega CONFIADO y no mira
```

Sin radio, el Maestro **no puede saber si el Esclavo sigue vivo**: podría estar apagado, colgado o
haber sido movido. Un verde equivocado es **más peligroso que un ámbar ambiguo**, porque le quita al
conductor la precaución que el ámbar le provoca.

Con activación manual, **el verde deja de darse por suposición y pasa a darse porque una persona
verificó las dos puntas**. Y es defendible ante una auditoría: "un técnico validó ambos extremos y
habilitó un modo especial" es un procedimiento; "la máquina decidió operar a ciegas" no lo es.

### Procedimiento de puesta en marcha

1. Confirmar que **ambas unidades tienen la hora puesta y coincidente**
   *(por Bluetooth: `CMD:LEER_RTC` en cada punta; se pone con `CMD:SET_RTC:<...>` — la pantalla
   `AJUSTAR HORA` ya no es una vía, ver SFTY-18)*
2. ~~Entrar a **MODO DEGRADADO** desde la pantalla, en cada unidad~~
   → **Maestro:** `SET_MODO:DEGRADADO` desde la app, **o** `A·B·A·B` con el mando.
   → **Esclavo:** `A·B·A·B` con el mando. ⛔ **Hoy no hay otra**, y el receptor de relés **no está
   comprado**: ver el aviso de la tabla de arriba. **Este paso es el único que decide, y en el
   Esclavo no es ejecutable con el material que hay.**
3. Iniciar
4. **Verificar visualmente que los dos semáforos alternan correctamente**

> 🔴 **CORREGIDO EL 05/09/2026, Y ES EL PASO QUE MÁS IMPORTA DEL DOCUMENTO.** El paso 2 decía
> *«desde la pantalla»* y la pantalla se retiró: **el único paso que toma una decisión no se podía
> ejecutar**. Y en un documento de trazabilidad `SFTY-x → código → prueba` eso vale doble —§2.ter
> de `CLAUDE.md`: *un paso que nadie puede ejecutar se cuenta como cubierto sin serlo*—.
>
> **Lo que este procedimiento SIGUE sin poder cerrar, y no es redacción:** con el receptor de
> relés sin comprar, la puesta en marcha del Degradado **en el Esclavo no tiene ninguna vía
> ejecutable hoy**. No es un defecto del firmware —las dos puertas están construidas y medidas—;
> es que ninguna de las dos tiene con qué abrirse en esa punta. Va al responsable, no a un pack.

### La deriva y el margen

Cristal de 32.768 kHz sin calibrar, a la intemperie: **±30 a 50 ppm**.

| Tiempo sin radio | Desfase entre unidades |
|---|---|
| 1 día | ~2 – 8 s |
| 3 días | ~6 – 25 s |
| 1 semana | ~15 – 60 s |

El **despeje todo-rojo es el colchón que absorbe ese desfase**. Con los 15 s actuales el margen dura
entre 2 y 7 días. **En Modo Degradado el todo-rojo debe ampliarse** —del orden del doble— para
sostener semanas. Se pierde fluidez, que es precisamente lo que se acepta en un modo degradado.

### 🛑 Límite duro: el Degradado debe rendirse solo

El diseño automático descartado (SFTY-19) tenía la regla *"pasadas N horas sin enlace ⇒ ámbar"*. **Al
pasar a activación manual esa regla se perdió**, y quedó solo *"la pantalla pide la resincronización"*
—un aviso, no un tope—. Es un error: **el estado seguro no puede depender de que alguien se acuerde**,
que es justamente el principio que el resto del sistema ya aplica.

Con el desfase puesto a cero por SFTY-23 al perder el enlace, el margen es el que da la deriva:

| Todo-rojo en Degradado | Deriva peor caso | Margen antes de solaparse | **Límite duro propuesto** |
|---|---|---|---|
| 15 s *(el normal)* | ~8,6 s/día | ~1,7 días | — *insuficiente* |
| **30 s** | ~8,6 s/día | ~3,5 días | **48 h** |
| 90 s | ~8,6 s/día | ~10 días | 5 días *(a costa de la fluidez)* |

**Propuesta: todo-rojo de 30 s y límite duro de 48 h**, con factor de seguridad 2 sobre el margen
teórico. Pasadas 48 h sin resincronizar, **el Degradado cae solo a ámbar intermitente**.

> ✅ **YA NO ES UNA PROPUESTA: está construido en las dos puntas [MEDIDO 01/09].**
> `Maestro/src/modo_degradado.cpp:102` y `Esclavo/src/modo_degradado.cpp` declaran
> `LIMITE_DURO_MS = 172800000UL` (48 h), con el estado `DEG_RENDIDO`, el rechazo
> `DEG_RECHAZO_SYNC_VENCIDA` y un **latch de caducidad que no se baja hasta una sincronización nueva**
> —para que un corte de luz a las 47 h no regale otras 48, que es la trampa que convierte un límite en
> un botón de posponer (`Esclavo/src/modo_degradado.cpp:311`)—. Lo vigila `costura_05_limite_48h`.
>
> **Sigue sin pasar banco**, como todo lo de V8.5 a V8.7.

> Conviene decirlo sin adornos: **querer una semana de autonomía obliga a un todo-rojo de ~90 s**, que
> destroza la fluidez del paso. No es una limitación del diseño, es la física de dos cristales sin
> disciplinar. La alternativa real no es alargar el plazo: es ir a arreglar el radio.

### ⚠️ Riesgos residuales aceptados por el cliente (01/08/2026)

**1. El verde se da sin confirmación del otro extremo.** Con el radio muerto es inevitable. Se mitiga
con activación manual verificada, todo-rojo ampliado, límite duro y aviso en pantalla. **No se
elimina.**

**2. Salida asimétrica: que una sola punta abandone el Degradado.** Es el escenario más peligroso y no
tiene solución técnica sin radio:

```
   Un microcorte reinicia UNA unidad
        -> arranca en MENU (asi lo hace main.cpp)
        -> sin enlace  ->  ÁMBAR          el conductor NEGOCIA el paso
   La otra sigue dando verde por reloj    el conductor pasa CONFIADO
```

Un lado en ámbar contra un lado en verde es **exactamente el escenario que este modo quiere evitar**.
Ocurre igual si un operario saca del Degradado **una sola** unidad con `A·A·A`.

**Mitigación procedimental, no técnica: la verificación visual de ambas puntas es obligatoria también
AL SALIR**, no solo al entrar. Debe constar en el manual del funcional y en el acta de pruebas.

> ### 🔴 01/09/2026 — «RIESGO RESIDUAL ACEPTADO» NO ES UNA DESCRIPCIÓN HONESTA DE ESTE RIESGO
>
> **Auditoría externa (N-109 §4).** El texto de arriba es correcto en los hechos y **engañoso en el
> encuadre**, por tres cosas que no dice:
>
> **1. El disparador es un microcorte, no una equivocación.** El escenario está descrito con un
> operario que saca *una sola* unidad —un error humano, que se corrige con procedimiento— y con «un
> microcorte» dentro del diagrama, en letra igual. **No son del mismo orden.** Un parpadeo de red
> reinicia una punta, esa punta arranca en menú y sin enlace cae a ámbar, y la otra **sigue dando
> verde por reloj**. No hace falta que nadie se equivoque en nada. Un riesgo que se dispara solo no se
> mitiga *«con verificación visual al salir»*: cuando el técnico se ha ido, no hay quien verifique.
>
> **2. La consecuencia no está escrita.** *«Un lado en ámbar contra un lado en verde»* describe dos
> lámparas. Lo que ocurre en un cierre de carril alternado, dicho como se debe: el conductor del lado
> ámbar negocia el paso y entra; el del lado verde entra confiado y sin mirar; **se encuentran de
> frente dentro del tramo.** Es la única forma en que este equipo puede matar a alguien, y este
> documento la nombra así en `SFTY-19` — pero no aquí, que es donde se aceptó el riesgo.
>
> **3. Nadie mide el invariante que lo cerraría.** *«Nunca verde en las dos puntas a la vez»* no lo
> ejecuta ningún instrumento sobre el C++ real de ambos extremos: `Validacion_Automatico` compila de
> verdad pero **sólo el Maestro** (§8 de `CLAUDE.md`), y lo único que cierra el lazo es una copia del
> firmware escrita a mano en Python. Ver el bloque de auditoría de la tabla de trazabilidad, más
> arriba.
>
> **4. Y hay una SEGUNDA salida asimétrica que este apartado no lista, con un pack que la reproduce.**
> No hace falta ni microcorte ni operario: **las dos puntas cuentan las 48 h por caminos distintos**
> —el Maestro contrasta con la pila, el Esclavo usa `millis()` con latch— **así que no se rinden en el
> mismo instante**, y en ese hueco una está en ámbar mientras la otra sigue dando verde por reloj. No
> es una hipótesis: es la propiedad que `costura_05_limite_48h` existe para reproducir, y está escrita
> en su cabecera. **El límite duro cierra la deriva y abre esto.** Cuánto dura el hueco es una medida
> de banco que nadie ha hecho.
>
> **Qué se cambia aquí y qué no.** No se retira la aceptación: **la decisión de riesgo es del
> responsable y del cliente, y sigue siendo suya.** Lo que se corrige es la frase que la sostiene, que
> daba por *residual* algo que un microcorte dispara solo y que acaba en choque frontal. **Una
> aceptación de riesgo vale lo que vale la descripción sobre la que se firmó**; si la descripción
> estaba corta, la firma se pidió sobre otra cosa.
>
> **Lo que reabre, y va al orden de trabajo, no a este documento:** el Modo Degradado no tiene hoy
> ninguna detección de *«la otra punta se reinició»*, porque sin radio no puede tenerla. La única
> palanca que queda del lado seguro es **el límite duro de 48 h**, que **sí está construido**
> —`LIMITE_DURO_MS = 172800000UL` en `Maestro/src/modo_degradado.cpp:102` y su gemelo en el Esclavo,
> con `DEG_RENDIDO` y el latch de caducidad [MEDIDO 01/09]—. **Acota la deriva; no acota el
> microcorte**, que es asimétrico y ocurre en el minuto uno.
>
> *(Anotación de método: la primera redacción de este párrafo decía que el límite «sigue siendo una
> propuesta, no una constante en el C++». Era falso y se cazó al ir a comprobarlo. Se deja escrito
> porque es §4 en el propio documento que audita: una afirmación plausible sobre lo que NO existe
> también es un instrumento, y también hay que descartar al buscador antes de publicarla.)*

---

### 🎛️ El mando de 4 relés — interfaz sin realimentación visual

El operario maneja la pantalla con un **mando de 4 relés (A, B, C, D)** cableados **en paralelo con
los botones físicos** (`PB9`, `PB13`, `PB14`, `PB15`; no hay entradas dedicadas). Hoy solo navegan el
menú: `A` arriba, `B` abajo, `C` aceptar, `D` menú.

> ⚠️ **CADUCADO EL 31/08/2026 en su segunda mitad, y se marca en vez de reescribirse porque el resto
> de este apartado —el porqué de usar sólo `A` y `B`— es lo que sigue vivo y es lo que importa.**
>
> **MEDIDO 01/09:** `J16` se repartió (N-97). `BOTON1 = PB9` y `BOTON2 = PB13` siguen siendo botones
> en `INPUT_PULLUP`, activos en BAJO, y **siguen alimentando `mando_registrarPulso()`**
> (`Maestro/src/botones.cpp:221-222`, `Esclavo:235-236`) — **el mando de relés está entero sobre `A` y
> `B` y no se ha tocado**. Lo que ya no existe es la otra mitad: `PB14` y `PB15` son `CAM_C_PIN` y
> `CAM_D_PIN`, entradas de cámara en `INPUT` pelado y activas en ALTO (`Maestro/include/pines.h:124-125`),
> y `botonAceptar()`/`botonCancelar()` devuelven `false` sin condiciones.
>
> **`C` y `D` ya no pulsan nada.** Este apartado razonaba largamente *por qué nunca se usarían* `C` ni
> `D`; el reparto de `J16` lo hizo **estructural** en vez de disciplinado. La conclusión que se
> defendía —*a ciegas se usan únicamente los botones cuya repetición accidental es inofensiva*— ya no
> depende de que nadie los cablee: **no hay a qué cablearlos.** Lo vigila `camara_02_j16`.

**El problema: la pantalla está a 5 m, dentro del gabinete.** El operario acciona desde el piso **sin
poder verla**. Un menú es inservible a ciegas: no se sabe dónde está el cursor ni si la pulsación
entró.

#### Restricciones medidas en campo (01/08/2026)

| Medida | Valor | Consecuencia de diseño |
|---|---|---|
| Tipo de señal | **Pulso por flanco**, no se sostiene | **La pulsación larga NO es posible.** Sostener el botón 10 s da un solo pulso |
| Retardo por pulsación | **~2 s** | Una ventana de 3 s es inviable; hacen falta 12–15 s para 3–4 pulsos |
| Repetición automática | **No la hay** | Cada pulso exige una pulsación |

#### La única salida visible desde el piso son las luces

El operario no ve la LCD, pero **sí ve el semáforo**. Por eso la confirmación se da en **destellos
ROJOS contables**: el rojo nunca significa "pase", así que **si el operario cuenta mal, el peor caso
sigue siendo seguro**. Destellar los tres colores a la vez se descartó: un conductor lejano podría
interpretar el verde.

#### Configuración propuesta

| Secuencia | Acción | Confirmación |
|---|---|---|
| **`A · A · A`** *(≤ 12 s)* | **AUTOMÁTICO** — "a ver si el radio volvió" | **2** destellos rojos |
| **`B · B · B`** *(≤ 12 s)* | **ÁMBAR intermitente** — salida de emergencia | **3** destellos rojos |
| **`A · B · A · B`** *(≤ 18 s)* | **Entrar a MODO DEGRADADO** | **4** destellos rojos |

#### Memotecnia

> **`A` es arriba → SUBE al modo normal.**
> **`B` es abajo → BAJA al mínimo seguro.**
> **Alternar → modo especial.**

Se aprende en un minuto. Es el requisito real para alguien que lo usa de madrugada y bajo lluvia.

#### La salida también debe poder hacerse desde el piso

Si se puede **entrar** al Modo Degradado desde el suelo pero para **salir** hay que subir, el mando no
sirve. El escenario típico es *"dejó de llover, a ver si el radio volvió"*.

Volver a Automático **no necesita protección**, porque el propio sistema se corrige:

```
   Automático intenta hablar con el otro lado
        -> sin respuesta en 25 s  (SFTY-6)
        -> se va solo a ÁMBAR INTERMITENTE
```

El peor caso de intentar Automático es **volver al ámbar**, que es justo donde se quería estar.

**Y el resultado de la prueba se ve en las luces, sin pantalla:**

```
   A·A·A  ->  2 destellos  ->  esperar ~15 s

     luces CICLANDO  ->  el radio volvió, ya está en automático
     luces en ÁMBAR  ->  sigue muerto; puede volverse al degradado
```

#### Ninguna de las tres deja el equipo en estado peligroso

| Secuencia | Si se dispara por accidente |
|---|---|
| `A·A·A` | Va a automático; sin radio cae a ámbar en 25 s. **Seguro** |
| `B·B·B` | Va a ámbar. **Seguro por definición** |
| `A·B·A·B` | Solo entra **si el reloj está validado**; si no, lo rechaza |

#### Por qué SOLO `A` y `B`, y nunca `C` ni `D`

La primera versión de este diseño usaba `C` y `D`, razonando que repetir *arriba* o *abajo* es normal
al navegar y podría disparar una secuencia por accidente. **Ese razonamiento estaba invertido**, y el
cliente lo corrigió el 01/08/2026.

El riesgo grave no es el falso positivo: es **qué ocurre si la pulsación llega cuando el sistema está
en un sitio distinto del que el operario cree**. Y a ciegas, eso siempre es posible.

```
   Equipo dejado en el MENÚ, y llega C·C·C desde el piso:
     1er C  ->  SELECCIONA lo que tenga el cursor -> arranca un modo no pedido
     2º  C  ->  en Modo Manual, C es ROJO FIJO INDEFINIDO
     3er C  ->  ...

   Mismo caso, llega A·A·A:
     el cursor sube tres veces.  No ocurre NADA.
```

**`C` ejecuta; `A` y `B` solo mueven.** La regla correcta es: **a ciegas se usan únicamente los
botones cuya repetición accidental es inofensiva.**

#### Por qué el degradado va alternado

`A·B·A·B` **no se produce nunca navegando**: se sube o se baja, no se zigzaguea. Y si el operario se
equivoca a mitad de la secuencia, **lo único que ha ocurrido es que el cursor se movió**.

#### 🔒 Requisito: ignorar las secuencias mientras el menú está abierto

Empezó siendo un afinamiento opcional —evitar que un técnico que baja tres veces con `B` dispare el
ámbar, molesto pero inofensivo—. **Al añadirse AJUSTAR HORA como quinta opción del menú pasó a ser
requisito**, porque el riesgo dejó de ser inofensivo:

```
   Ráfaga accidental de pulsos desde el mando, con el menú abierto
        -> el cursor llega a AJUSTAR HORA
        -> unos pulsos más CONFIRMAN una hora cualquiera
        -> el reloj queda MARCADO COMO VÁLIDO con una hora inventada
```

Eso es **exactamente el veneno que SFTY-18 existe para evitar**: no la falta de reloj, sino un reloj
falso que se cree bueno. Y habilitaría el Modo Degradado y la operación nocturna sobre una hora
inventada.

**Regla: mientras el menú esté abierto, las secuencias del mando no se reconocen.** Desde el piso el
operario lo distingue sin ver la pantalla: si las luces están ciclando, el menú no está abierto.

#### Asimetría deliberada: lo seguro fácil, lo peligroso difícil

| Si se dispara por accidente | Consecuencia | Protección |
|---|---|---|
| Ámbar intermitente | El equipo va a seguro. Molesto, no peligroso | Secuencia corta |
| Modo Degradado | Verde sin confirmar el otro lado | Secuencia larga **+ validación en firmware** |

#### La red de seguridad real no es la secuencia

```
   A·B·A·B desde el piso
        |
        +-- ¿El reloj está en hora?
        +-- ¿Hay una medición de desfase reciente y dentro de tolerancia? (SFTY-23)
        +-- ¿La configuración del ciclo se sincronizó con el otro lado? (SFTY-23)
        |
        +-- SÍ  ->  4 destellos rojos  ->  entra a MODO DEGRADADO
        |
        +-- NO  ->  ÁMBAR RÁPIDO 2 s   ->  RECHAZADO
```

**Aunque alguien acierte la secuencia por casualidad, el firmware no entra** si la hora no está
validada. El mando permite reactivar en campo sin grúa, pero **no** saltarse la puesta a punto.

> **`B·B·B` devuelve a ámbar desde cualquier estado, sin condiciones.** Es la regla que impide que
> nadie quede atrapado con un semáforo en estado raro a 5 m de altura.

El menú **sigue funcionando igual** para el técnico que sube: un pulso navega, la secuencia cambia de
modo. No hay que reprogramar lo que ya funciona.

---

## ⏱️ SFTY-23 — Sincronización horaria por radio (**IMPLEMENTADO**)

> **Estado:** especificado y **construido en las dos puntas** el 01/08/2026, tras una auditoría que
> detectó que el procedimiento de SFTY-21 era insuficiente. **Requisito previo del Modo Degradado**,
> no un extra.
>
> ⚠️ **Sin prueba de banco.** Nada de esto se ha ejercitado sobre hardware real.

### El defecto que corrige

SFTY-21 pedía *"confirmar que ambas unidades tienen la hora puesta y coincidente"* mirando las dos
pantallas. **Eso no funciona**, y la aritmética lo demuestra:

```
   Operario A confirma el Maestro   a las 14:32:10 reales -> el reloj marca 14:32:00
   Operario B confirma el Esclavo   a las 14:32:50 reales -> el reloj marca 14:32:00

   Las dos pantallas muestran 14:32.  Los relojes están a 40 s.
```

**Hasta 59 s de desfase el primer día** — casi cuatro veces el todo-rojo de 15 s — y **dos pantallas
en `HH:MM` no pueden detectarlo**. La tabla de deriva de SFTY-21 asume que las unidades arrancan en
~0 de desfase; ajustando a mano, esa premisa es falsa.

### La regla

**La hora se cuadra UNA sola vez, en el Maestro, y el Esclavo nunca se toca a mano.** El Maestro
empuja su hora por radio mientras el enlace vive. Así, el día que el radio muera, el desfase arranca
en ~0 **de verdad**, no por procedimiento.

Y la pila es lo que lo hace durable: sincronizados una vez, **cada reloj sobrevive los cortes de
energía con su propia CR2032**, y la única deriva que queda es la posterior a la pérdida del enlace.

### Comandos nuevos

El paquete RF son 4 bytes con **un solo `param`**, así que la hora no cabe en una trama. Se usan
varias, con el mismo patrón de reintentos que ya existe (SFTY-7). Libres desde `0x07`:

| Comando | Código | `param` |
|---|---|---|
| `CMD_HORA_H` | `0x07` | hora (0–23) |
| `CMD_HORA_M` | `0x08` | minuto (0–59) |
| `CMD_HORA_S` | `0x09` | segundo (0–59) — **al recibirla, el Esclavo aplica las tres juntas** |
| `CMD_ACK_HORA` | `0x0A` | confirmación del Esclavo tras aplicar la terna |
| `CMD_DELTA` | `0x0B` | segundo actual del Maestro |
| `CMD_DELTA_RESP` | `0x0C` | diferencia medida, complemento a dos (`int8_t`) |
| `CMD_CONFIG_VERDE` | `0x0D` | segundos de verde del ciclo degradado |
| `CMD_CONFIG_DESPEJE` | `0x0E` | segundos de todo-rojo, **ya ampliado** |
| `CMD_ACK_CONFIG` | `0x0F` | confirmación del par de configuración |

> ⚠️ **Esta tabla estuvo mal hasta el 01/08/2026** y lo detectó una auditoría de manuales. Decía
> `CMD_DELTA = 0x0A` y `CMD_CONFIG = 0x0B`, que era el **boceto** escrito antes de cerrar
> `protocolo.h`; al ampliarse el contrato con los ACK y el par de configuración, la especificación no
> se actualizó. **Quien implementara contra ella habría usado códigos equivocados.**
>
> **La fuente de verdad es `01_Firmware/*/include/protocolo.h`**, verificado idéntico en ambos
> proyectos. El simulador lee los códigos de ahí en cada ejecución, con lectura obligatoria, para que
> una divergencia como ésta no pueda volver a pasar inadvertida.

El Esclavo **acumula hora y minuto en un buffer y solo escribe el RTC al llegar la de segundos**:
aplicación atómica, nunca queda una hora a medias. `reloj_ajustar()` ya acepta segundos, así que del
lado del reloj no hay que tocar nada. Y el puente **SFTY-16 valida formato y CRC, no comandos**, de
modo que las tramas nuevas atraviesan el ESP32 sin modificarlo.

> ### ⚠️ Regla obligatoria en los reintentos
>
> **El Maestro debe RECALCULAR el valor de segundos en cada retransmisión, nunca reenviar el que
> calculó la primera vez.** Si la trama se pierde y se reintenta 3,5 s después con el valor viejo, el
> Esclavo queda 3,5 s atrasado — y el error entra justo por el mecanismo que existe para dar robustez.
>
> Es un fallo de una sola línea que no se ve en pruebas con enlace bueno.

**Cuándo se dispara:** al confirmar en **AJUSTAR HORA** del Maestro —poner en hora *es* sincronizar,
un solo gesto— y **periódicamente mientras haya enlace**. Con una vez por hora sobra: la deriva entre
sincronizaciones queda en milisegundos.

### La validación debe ser una MEDICIÓN, no una inspección ocular

`CMD_DELTA` convierte el paso 1 del procedimiento de SFTY-21 —*"confirmar que la hora coincide"*— de
mirar dos pantallas a **leer un número**:

```
   ┌──────────────────────────────┐
   │ Desfase Esclavo:      +1 s   │
   │ Ultima sincronizacion: 14:32 │
   └──────────────────────────────┘
```

Registrable en el acta del protocolo de pruebas, y **habilita el gate de entrada en firmware**: el
Modo Degradado solo se permite si hay una medición de desfase **dentro de tolerancia** y **reciente**.
Eso da contenido real a la condición *"¿se sincronizó con el otro lado alguna vez?"*.

**Precisión de la medida.** El desfase medido incluye el tiempo de aire más el retardo de cortesía del
Esclavo (SFTY-17, 200 ms), así que trae un sesgo de **algunas décimas de segundo**. Frente a un
todo-rojo de 15–30 s es irrelevante, y conviene dejarlo escrito para que nadie persiga ese error.
El `param` es de un byte: la diferencia se transmite **con signo, ±127 s**, y fuera de ese rango debe
**saturar y reportarse como "fuera de rango"**, nunca dar la vuelta.

> ### ⚠️ Límite inherente: la medida solo alcanza ±30 s
>
> Detectado al implementar el lado Esclavo (01/08/2026). Como `CMD_DELTA` transporta **solo el
> segundo** (0–59), la corrección circular resuelve en el sentido corto y el resultado **siempre cae
> en ±30 s**. Consecuencia:
>
> ```
>    Desfase real de 45 s  ->  se mide como -15 s
>    No hay forma de distinguirlos con solo el segundo.
> ```
>
> **Un desfase peligroso podría leerse como aceptable y pasar la puerta del Modo Degradado.**
>
> ### La regla que cierra el agujero
>
> **La puerta del Degradado NO puede apoyarse solo en el desfase medido.** Debe exigir **las dos
> condiciones a la vez**:
>
> 1. **Una sincronización correcta reciente** *(propuesta: menos de N horas)*
> 2. **Desfase medido dentro de tolerancia**
>
> La primera es la que hace fiable a la segunda: tras una sincronización correcta el desfase arranca
> en milisegundos, y con una deriva de ~100 ppm hace falta **más de tres días** para acumular los
> 30 s que provocarían el alias. Con una sincronización de hace una hora, la deriva es de **~0,36 s**:
> la medida no puede estar aliasada.
>
> **El desfase es una comprobación de cordura, no la garantía.** La garantía es la sincronización
> reciente. Invertir esa relación —confiar en el número y no en su frescura— reintroduce el fallo.
>
> *Alternativa si algún día hiciera falta rango mayor: añadir una trama con el minuto y calcular
> sobre segundos-dentro-de-la-hora, lo que llevaría el alcance a ±30 min. Hoy no es necesario.*

### El ciclo también debe viajar, no solo la hora

Dos relojes en hora dan **tiempo común**, pero para ir en fase ambas unidades tienen que computar el
**mismo horario de fases**: quién está en verde en cada instante, cuánto dura y cuánto es el todo-rojo
ampliado. Hoy esa configuración **solo existe en el Maestro** y viaja por radio en operación normal.

Sin radio, o se configura a mano en las dos puntas —otra fuente de error humano, la misma que este
documento acaba de eliminar para la hora— o **se sincroniza junto con la hora mientras hay enlace**.
`CMD_CONFIG` cubre eso. **Es tan condición de seguridad como la hora**, y faltaba en SFTY-21.

### Lo que la sincronización NO arregla

**La deriva posterior sigue corriendo.** Sincronizar pone el desfase a cero en el momento de perder el
radio, pero a partir de ahí crece igual. Por eso el **límite duro** de SFTY-21 sigue siendo necesario.

---

## 📺 SFTY-22 — Pantalla informativa durante el ámbar (MEJORA, **NO IMPLEMENTADO**)

> **Estado:** marcada como mejora el 01/08/2026. No es urgente: el sistema funciona.

Hoy, al perder el enlace, el equipo entra en ámbar intermitente **sin decir por qué**. Quien sube a
revisarlo se encuentra un ámbar mudo y empieza a preguntar: ¿desde cuándo?, ¿alguien tocó algo?,
¿es el radio o la configuración?

La propuesta es que, **al entrar en ámbar por pérdida de enlace, aparezca sola** una pantalla de
diagnóstico:

```
   +------------------------------+
   | SIN ENLACE                   |  <- por qué está en ámbar
   | Desde: 08:32   Hace: 2h 14m  |  <- lo aporta el RTC (SFTY-18)
   |                              |
   | RX 0 - nada llega            |  <- SFTY-15, ya existe
   | Ultimo RF: 100%  340ms       |  <- SFTY-14, ya existe
   +------------------------------+
```

**Casi todo el dato ya está dentro del firmware** desde el 31/07. Lo único que falta es juntarlo en
una pantalla y **la hora**: sin RTC solo se puede decir "hace un rato", no "se cayó a las 08:32".

### ⚠️ Un diagnóstico no debe alterar lo que diagnostica

Esta pantalla **aparece sola al entrar en ámbar**, y ésa es la vía principal. Es gratis: el equipo ya
está detenido, así que informar no cambia nada.

**No debe diseñarse como una opción más del menú**, porque entrar al menú **detiene el ciclo**
(SFTY-12 deja ambas unidades en rojo fijo). Se acabaría diagnosticando un equipo que ya dejó de hacer
aquello que se quería diagnosticar.

| Vía | Cuándo | ¿Altera el estado? |
|---|---|---|
| **Aparece sola en el ámbar** ← principal | Al perderse el enlace | **No.** El equipo ya estaba detenido |
| Buscada desde el menú | Cuando el técnico quiera | **Sí**, detiene el ciclo, como todo el menú |

Conviene recordar además que **la telemetría ya está visible durante la operación normal**: SFTY-14
muestra `RF:100% 340ms` en las pantallas de los modos, sin detener nada. Para saber si el enlace está
sano **hoy no hace falta entrar al menú**. Lo que falta es el *por qué* y el *desde cuándo* una vez
que ya se cayó, y eso es lo que aporta el reloj.

### Su público es el técnico, no el conductor

Con la pantalla a **5 m dentro del gabinete, nadie la lee de paso.** Su valor está en el momento del
diagnóstico: los tres mensajes de la fila `RX` separan **tres averías que hoy se ven todas igual**
—`nada llega` (cobertura, canal o antena), `BASURA` (cableado, línea flotando o radio atascada) y
enlace correcto—. Es exactamente la distinción que costó la jornada completa del 31/07.

**No habría evitado la avería, pero habría acortado mucho el camino hasta encontrarla.**

---

## 🌙 SFTY-20 — Operación intermitente nocturna (DISEÑO, **NO IMPLEMENTADO**)

> **Estado:** especificado el 01/08/2026. Corresponde al pendiente **N-3** y es **para lo que se soldó
> la pila**. Prioridad 4 en el orden de trabajo del roadmap -las «olas» viven hoy en `roadmap_hist.md`-: se construye, pero **no va a campo** hasta
> cerrar las antenas y la prueba de banco de la telemetría.

Requisito de origen: `MANUAL_USUARIO.md §2`. **Disparo por horario**, decidido el 31/07 — el disparo
por flujo real exigiría la cámara instalada, que hoy no lo está. La franja es **configurable**, porque
el horario no es el mismo en todas las obras.

### 🚫 El menú está lleno, y falla en silencio

```c
int y = 28 + i * 11;
if (y > 63) break;   // salvaguarda
```

Una quinta opción con el paso de 11 px cae en **`y = 72`**. La salvaguarda impide dibujarla, **pero el
cursor sí puede navegar hasta ella**: el operario llegaría a una opción invisible. Un fallo silencioso
es peor que uno ruidoso.

> ### 🗂️ ESTRUCTURA DEFINITIVA DEL MENÚ (01/08/2026)
>
> Al llegar el Modo Degradado harían falta **seis** opciones, y con el interlineado
> compactado la sexta caería en `y = 69` — fuera de los 64 px otra vez, con el mismo agravante: la
> salvaguarda no la dibujaría **pero el cursor sí llegaría hasta ella**.
>
> **La solución no es comprimir más píxeles: es un submenú.**
>
> ```
>    MENÚ PRINCIPAL                 CONFIGURACIÓN
>    ┌──────────────────┐           ┌──────────────────┐
>    │ > MANUAL         │           │ > PRUEBA ALCANCE │
>    │   AUTOMATICO     │    -->    │   AJUSTAR HORA   │
>    │   INTELIGENTE    │           │   MODO DEGRADADO │
>    │   CONFIGURACION  │           └──────────────────┘
>    └──────────────────┘
>         4 opciones                     3 opciones
> ```
>
> **El menú principal vuelve a 4 opciones**, que es exactamente el layout validado en campo y con
> 30/30 en el arnés. No hay que tocar `lcd_dibujarMenu()`: con 3 o con 4 opciones ambos menús caen en
> el caso de siempre (base 28, paso 11).
>
> **Por qué es la división correcta, y no un apaño de espacio.** `MANUAL`, `AUTOMATICO` e
> `INTELIGENTE` son **modos de operación** —lo que el operario elige a diario—. `PRUEBA ALCANCE`,
> `AJUSTAR HORA` y `MODO DEGRADADO` son **herramientas y casos especiales** que se tocan rara vez.
> **Mezclarlos en una lista plana era la causa del problema, no el número de opciones.** Comprimir
> píxeles habría tapado el síntoma dejando la causa intacta.
>
> **Y un beneficio de seguridad que no se buscaba:** con el mando de relés operando **a ciegas**, una
> ráfaga accidental de pulsos **ya no puede alcanzar el Modo Degradado ni AJUSTAR HORA** — están un
> nivel más abajo. Refuerza por estructura el requisito de *"ignorar secuencias con el menú abierto"*,
> y sale gratis.
>
> El Botón 4 desde el submenú **vuelve al menú principal**, no sale a un modo. El submenú mantiene el
> mismo estado seguro: Rojo Fijo con enlace, Ámbar sin él. **No arranca ciclos.**

---

> ### ✅ RESUELTO EN V8.6 — y de otra forma que la propuesta aquí
>
> Este apartado proponía **no tocar el menú** y acceder por pulsación larga del Botón 4. **Se
> descartó**, porque el análisis de partida era incompleto: daba por fijo el arranque en `y = 28` y
> concluía que a 9 px la quinta caía en `y = 64`, un píxel fuera.
>
> **Moviendo también la base**, sí cabe: `lcd_dibujarMenu()` usa ahora **base 24 y paso 9** cuando hay
> 5 opciones ⇒ `y = 24, 33, 42, 51, 60`. Con 4 o menos conserva **exactamente** el layout validado en
> campo (base 28, paso 11). Confirmado en el arnés: **42/42**.
>
> **Lo que se pierde y hay que compensar.** La pulsación larga existía para que *un gesto deliberado
> impidiese cambiar el reloj sin querer*. Como opción de menú, **una ráfaga accidental de pulsos desde
> el mando de relés puede entrar a AJUSTAR HORA y, con cuatro pulsos más, confirmar una hora errónea
> marcada como válida** — justo el veneno que SFTY-18 quiere evitar.
>
> Por eso **ignorar las secuencias del mando mientras el menú está abierto deja de ser un
> "afinamiento pendiente" y pasa a ser requisito** (ver SFTY-21).

### Piezas

| # | Pieza | Nota |
|---|---|---|
| 1 | Pantalla **AJUSTAR HORA** | `HH:MM` · B1 = + · B2 = − · B3 = siguiente · B4 = salir |
| 2 | Pantalla **FRANJA NOCTURNA** | hora de inicio y hora de fin |
| 3 | **Persistencia de la franja** | ⚠️ ver abajo — la pieza que puede descarrilar el resto |
| 4 | Estados de luz nuevos | Maestro ámbar intermitente · Esclavo rojo intermitente |
| 5 | Comando RF `CMD_GO_NOCHE` | `0x07` está libre (`0x01`–`0x06` ocupados) |

### ⚠️ La franja no sobrevive al apagón

`reloj.cpp` guarda hoy la franja **solo en RAM**: se va la luz y vuelve al valor por defecto. Es
inaceptable para algo que el operario configura en obra.

`STM32duino RTC` **no expone los registros de respaldo** — comprobado al compilar, `getBackupRegister`
no existe en la versión 1.9.0. En el STM32F1 son accesibles directamente (`BKP->DR1..DR10`, previa
habilitación de escritura en el dominio de respaldo), unas diez líneas. **Es el trabajo menos obvio de
todo el conjunto y el que más fácil se pasa por alto al planificar.**

### Reglas de seguridad, no negociables

| # | Regla | Por qué |
|---|---|---|
| 1 | `reloj_enHora() == false` ⇒ **nunca** entrar en modo nocturno | Ya construido en SFTY-18. Un reloj sin poner en hora activaría el modo a deshora |
| 2 | Entrar y salir del modo pasa por el **despeje todo-rojo** (SFTY-4) | No se salta de verde a intermitente |
| 3 | Si el Esclavo no confirma `CMD_GO_NOCHE` ⇒ ámbar en ambos | Se degrada al fallo conocido, no a un estado a medias |

### Un estado propio, aunque se vea igual

El Maestro en modo nocturno parpadea ámbar, **visualmente idéntico a `S_FALLO`**. Aun así necesita
estado propio: reutilizar `S_FALLO` haría que la telemetría y los diagnósticos **reporten una avería
que no existe**. Hoy el enum es `{ S_ROJO, S_VERDE, S_AMARILLO, S_FALLO }`.

### Cómo se valida — y el requisito previo

`simulador_sistema_v7_6.py` ya tiene **eje de tiempo** (`avanzar_simulacion(duracion_s, dt=0.1)`, con
`current_time` en todas las máquinas de estado) y modela las luces como estados con nombre. Simular
este modo es añadir un reloj al modelo y una prueba que compruebe:

1. Ambas unidades **entran y salen de la franja en fase**.
2. La transición **pasa por el despeje todo-rojo**, no salta.
3. Con `reloj_enHora() == false`, **nunca entra** — ni al principio ni al cruzar la franja.

> ⚠️ **Requisito previo (N-12).** El simulador tiene el despeje todo-rojo escrito a mano como `15.0`,
> y **no lo lee del C++**. Como ese despeje *es* el margen de seguridad al entrar y salir de este
> modo, validar SFTY-20 contra un valor que el modelo no vigila no demostraría nada. Hay que extender
> primero la lectura anti-deriva del bloque 0.

---

## 🚧 SFTY-19 — Operación autónoma al perder el radio (DISEÑO, **NO IMPLEMENTADO**)

> **Estado:** especificado el 31/07/2026, pendiente de construir, simular y validar con el funcional.
> **Nada de esto está en el firmware todavía.** El comportamiento actual sigue siendo SFTY-6:
> sin enlace ⇒ ámbar intermitente en las dos puntas.
>
> ## ❌ SUSTITUIDA POR SFTY-21 (01/08/2026)
>
> Esta regla planteaba **entrada automática** al modo autónomo tras N minutos sin enlace. **Se
> descartó en reunión con el cliente.** El motivo es el de siempre: sin radio nadie puede confirmar
> que la otra punta esté viva, y una máquina no debe decidir sola operar a ciegas.
>
> **Lo aprovechable de este diseño —la deriva entre relojes, el despeje todo-rojo como colchón y las
> condiciones que impiden entrar— se trasladó a SFTY-21**, ahora con activación manual verificada.
>
> Se conserva el texto por dos razones: deja constancia de **por qué se descartó la vía automática**,
> y el análisis de sincronización relativa sigue siendo válido si algún día se retoma.

### El problema que resuelve

Con el radio averiado, hoy el sistema entra en ámbar intermitente. Es correcto y es el *fail-safe*
estándar — devuelve la decisión al conductor — pero deja el paso de obra sin regular. Se pide que
el equipo **siga alternando verde y rojo por su cuenta** durante una pérdida de enlace acotada.

### Por qué es delicado

Este sistema regula **un carril alternado**. Cuando el Maestro da verde, el Esclavo **tiene** que estar
en rojo. Si las dos unidades cuentan cada una por su lado y se separan lo suficiente, **hay verde
simultáneo en las dos puntas y dos vehículos entran de frente al tramo.** No es un defecto cosmético:
es la única forma en que este equipo puede matar a alguien.

### Reglas que hacen viable el modo

| # | Regla | Por qué |
|---|---|---|
| 1 | Mientras hay enlace, el Maestro comunica periódicamente **en qué punto del ciclo va** | Da a las dos unidades un origen común |
| 2 | Al perderse el enlace, cada unidad continúa **desde el último punto sincronizado** | La separación arranca en ~0, no en un valor arbitrario |
| 3 | El despeje **todo-rojo** debe superar con holgura la separación acumulada máxima | Es el margen que absorbe la deriva |
| 4 | **Límite duro de tiempo sin enlace** ⇒ ámbar intermitente | Más allá no se puede acotar la deriva **ni saber si la otra punta sigue viva** |
| 5 | Unidad que **arranca o se reinicia sin haber sincronizado nunca** ⇒ ámbar, sin excepción | Sin origen común no hay modo autónomo posible |

Las reglas 4 y 5 **no son opcionales**. Son lo que separa "modo autónomo" de "verde en las dos puntas".

### Hallazgo de diseño: esto NO necesita el RTC

La sincronización es **relativa** al último mensaje del Maestro, no a la hora absoluta. Para eso basta
el contador de milisegundos del micro. Consecuencias prácticas:

- **No hace falta pila en la tarjeta del Esclavo.** El Esclavo no necesita saber la hora.
- El RTC (SFTY-18) sigue siendo necesario para la **operación intermitente nocturna**, que es otra
  función distinta y quedó **aplazada** a petición del cliente: el horario no es igual en todas las obras.

### Parámetros por decidir

- **Tiempo máximo sin enlace antes de rendirse a ámbar.** Propuesta: **30 min** — suficiente para que
  el funcional haga la prueba desconectando una antena, y corto para que un radio muerto de verdad no
  deje el cruce operando a ciegas toda la noche.
- **Ciclo que corre en modo autónomo.** Propuesta: reusar el tiempo de verde ya configurado en
  **MODO AUTOMÁTICO**, en vez de introducir un parámetro nuevo.

### Criterio de aceptación

El simulador ya modela pérdida de enlace. La validación **debe medir la separación entre las dos
unidades** a lo largo de la ventana sin radio y demostrar que el todo-rojo la cubre — no basta con
afirmarlo. Mientras esa medida no exista, este modo **no va a campo**.

---

## 📶 SFTY-24 — Enlace de respaldo por datos moviles entre dos telefonos (DISENO, **NO IMPLEMENTADO**)

> **Estado:** propuesto el 26/08/2026 a raiz de una observacion de operacion. **Nada de esto esta en
> el firmware ni en la app.** El enlace entre puntas sigue siendo unicamente el radio LoRa E90-DTU, y
> la perdida de ese enlace sigue cayendo a ambar intermitente (SFTY-6).

### De donde sale la idea

La app de campo ya corre en un telefono que, fuera de la montana, **tiene datos**. Hoy ese canal no
se usa para nada: la app habla Bluetooth con el poste que tiene delante y se acaba ahi. La pregunta
es si **dos telefonos, uno en cada punta, podrian hablarse por datos** y darle al sistema un segundo
camino entre extremos que no dependa del radio.

### Lo que resolveria, y es real

| | |
|---|---|
| **Saber si la otra punta esta viva** | Hoy, con el radio caido, la unica forma de saber que hace el otro extremo es caminar el tramo. Un tramo de obra puede tener cientos de metros y trafico dentro |
| **Diagnostico de las dos cajas negras en un sitio** | `$ALARM` y `$EVENT` de ambos postes juntos, con marca de tiempo, para la interventoria |
| **Courier RTC instantaneo** | El Modo Courier ya existe y es la version *sneakernet* de esto: el tecnico lleva la hora andando. Por datos seria inmediato |
| **Confirmar una maniobra antes de hacerla** | El operario del Km 12 ve en su pantalla que el otro extremo esta en rojo **antes** de pedir el cambio |

### 🛑 La regla, y no es negociable

> **El enlace por datos puede OBSERVAR y puede DOCUMENTAR. No puede AUTORIZAR.**

Ninguna trama que llegue por ese canal puede provocar un verde, acortar un todo-rojo, ni sacar a un
equipo del ambar. Como mucho puede **pedir** algo que el radio LoRa tendra que confirmar por su
cuenta, con su CRC y su ACK, exactamente igual que si la peticion hubiera venido de un boton.

Las razones son las mismas por las que SFTY-19 descarto la entrada automatica, mas dos propias:

1. **La cobertura celular es justo lo que no hay donde se usan estos equipos.** El manual de la app
   lo dice como virtud —*"100% Offline, opera en montana sin internet ni 4G"*—, y tiene razon. Un
   canal que existe **a veces** es peor que uno que no existe nunca: el que no existe no engana a
   nadie, el intermitente ensena a confiar y falla el dia que importa.
2. **Latencia no acotada.** Datos moviles admiten segundos de retardo, reintentos y caidas
   silenciosas. El paso alternado necesita saber **ahora** si el otro lado esta en rojo, no hace
   cuatro segundos. Un verde concedido sobre un mensaje retrasado es verde simultaneo en las dos
   puntas, que es la unica forma en que este equipo puede matar a alguien.
3. **Cuatro puntos de fallo nuevos, ninguno bajo control del equipo:** dos baterias de telefono, dos
   personas, dos operadoras y una nube.

### Parametros por decidir

- **Transporte.** Un servidor propio implica infraestructura y cuenta que alguien paga; un canal
  directo entre telefonos evita el servidor pero complica el emparejamiento en campo. Sin decidir.
- **Que se sincroniza.** Propuesta minima: solo `$STATUS`, `$ALARM` y `$EVENT`, de lectura. Nada de
  comandos en la primera version — y si algun dia los hay, pasan por la radio.
- **Que ve el tecnico cuando el canal se cae**, que sera lo normal. Propuesta: la pantalla del otro
  extremo se marca **caducada con su antiguedad en segundos**, nunca se congela mostrando el ultimo
  valor como si fuera actual. Un dato viejo sin fecha es peor que ningun dato.

### Criterio de aceptacion

Antes de escribir una linea: **medir la cobertura real en los tramos donde opera el equipo**, con el
telefono que usa el tecnico, a lo largo de una jornada. Si la cobertura no esta, esta funcion es una
pantalla bonita que no se enciende cuando hace falta. Y la medida va al repositorio, no al recuerdo
de nadie.

---

## 🏷️ SFTY-25 — Identidad de tramo en la telemetria (RIESGO ABIERTO, **NO IMPLEMENTADO**)

> **Esto no es una mejora futura: es un agujero de hoy**, y aparece en cuanto hay mas de un par de
> semaforos en la misma via, que es el caso de uso real.

### El escenario

Una via en obra lleva **varios pares Maestro/Esclavo** —Km 12, Km 24, Km 31— y **un solo telefono**
recorriendolos. La app ya distingue el **rol** (`NODE:MAESTRO` / `NODE:ESCLAVO`) porque la trama lo
trae. Pero el rol no dice **de que par** es ese equipo.

### Por que el rol no basta

**El rol no es el sentido, y el sentido no es la instalacion.** Un Maestro puede estar en el extremo
norte de un tramo y en el sur del siguiente; "Sentido 1 = Sisga -> Bogota" es una propiedad de **como
se planto el poste**, no de que firmware lleva dentro. Dos pares distintos emiten hoy tramas
**indistinguibles**:

```
   $STATUS,NODE:MAESTRO,MODO:MANUAL,ESTADO:...   <- ¿el del Km 12 o el del Km 24?
```

El fallo concreto: el tecnico esta parado en el Km 24, su telefono se engancha por Bluetooth al par
del Km 12 —que sigue en rango, o fue el ultimo emparejado—, pulsa **DAR PASO A SENTIDO 1** creyendo
que gobierna el poste que tiene delante, y **abre un verde a 12 km de distancia** en un tramo que no
esta mirando.

### Lo que hace falta

| # | Pieza | Donde |
|---|---|---|
| 1 | Campo `ID:` en `$STATUS`, `$ALARM` y `$EVENT` con el identificador de tramo | `bluetooth.cpp`, las tres tramas |
| 2 | Ese identificador configurable desde el menu del LCD y guardado en respaldo | `menu.cpp`, `respaldo.cpp` — cambia la `FIRMA` |
| 3 | La app muestra el `ID:` **grande y permanente**, no en un submenu | App movil |
| 4 | Etiqueta fisica visible en el poste con el mismo identificador | Procedimiento de instalacion |
| 5 | La app **rechaza** un comando si el `ID:` de la conexion no es el que el operario tiene seleccionado | App movil |

El punto 4 no es burocracia: es lo que permite al operario **comparar lo que ve en la pantalla con lo
que tiene delante**. Sin esa comparacion, los otros cuatro puntos solo mueven el error de sitio.

### Criterio de aceptacion

Dos pares encendidos a la vez en el banco, un solo telefono, y **demostrar que un comando dirigido al
par A no llega al par B** — ni siquiera cuando el operario lo intenta a proposito. Mientras esa
prueba no exista, el manual de la app debe advertir que **solo se opera con un par encendido a la
vez**.


---

## 🔌 SFTY-26 — Expansor I2C para acabar con la disputa por los dos pines libres (DISENO, **NO IMPLEMENTADO**)

> **Estado:** decidido el 26/08/2026 como salida de `N-57`. **No es una eleccion entre reloj y
> camaras: son las dos, sobre el mismo bus, y el firmware detecta que hay montado.**

### El problema, en una linea

La placa tiene **dos pines libres** y **tres cosas** los quieren: el `DS3231` necesita dos (`SDA`,
`SCL`) porque `N-37` cerro en banco con el cristal `Y2` **muerto**, y las camaras necesitan al menos
uno. No caben **como pines sueltos**. Si caben **como bus**.

### La salida: una placa hija de expansion, siempre la misma

`PB0` y `PB8` dejan de ser "dos entradas" y pasan a ser **un bus I2C**. De ahi cuelga una placa hija
con dos modulos, y **el cableado es identico en todas las unidades** — como el del modulo Bluetooth:
un solo diagrama, una sola forma de conectarlo, sin variantes que el instalador tenga que decidir.

```
   PB0 (SDA) --+--------------+---------------
   PB8 (SCL) --+--+        +--+--+
                  |        |     |
              [DS3231]   [PCF8574]  --> 8 entradas digitales
               0x68        0x20         (camaras 1..4, y sobran 4)
             se monta      SIEMPRE
             SOLO si el    montado
             Y2 esta
             muerto
```

**El `PCF8574` va siempre.** Es lo que hace que las camaras entren por el bus y que `PB0`/`PB8` dejen
de estar en disputa para siempre: cualquier entrada futura entra por el expansor sin volver a abrir
esta discusion.

**El `DS3231` va solo donde haga falta.** Y ahi esta la gracia del diseño: **no hay que saberlo de
antemano.**

### Un solo firmware para los dos casos: se detecta al arrancar

Al arrancar, el micro **escanea el bus** y decide su fuente de hora. Nada de compilar dos versiones
ni de decidir por unidad antes de fabricar:

```
   arranque
     |
     +-- responde 0x68 en el bus ?  --SI-->  RELOJ = DS3231 externo
     |                              --NO-->  RELOJ = cristal Y2 interno
     |
     +-- responde 0x20 en el bus ?  --SI-->  camaras leidas por el expansor
                                    --NO-->  camaras leidas por PB0 directo (compatibilidad)
```

Se programa todo una vez. Una tarjeta con el cristal sano funciona sin placa hija; si el cristal
falla, **se le enchufa el modulo y arranca usandolo, sin recompilar nada**.

### Y AVISA. Un respaldo silencioso seria el defecto, no la solucion

Esto es lo que `N-12` dejo escrito en este repositorio: *un valor por defecto silencioso derrota el
proposito*. Asi que la fuente de hora **nunca se elige en silencio**:

| Donde | Que dice |
|---|---|
| LCD, pantalla de estado | `RELOJ: INTERNO` · `RELOJ: DS3231` · `RELOJ: SIN FUENTE` |
| Telemetria `$STATUS` | campo `CLK:INT` / `CLK:EXT` / `CLK:NONE` |
| Caja negra | `$ALARM` al detectar que el cristal interno **acepta la hora y no avanza** |
| App | avisa al tecnico y lo registra: **el censo se construye solo** |

La deteccion de "cristal muerto" no es *"no esta en hora"* —eso le pasa a una unidad sana recien
encendida—: es **que la hora no AVANCE**. Se mide comparando dos lecturas separadas, que es
exactamente lo que hizo `N-37` a mano y lo que la app puede hacer sola en cada conexion.

### Lo que hay que medir ANTES de comprometerlo

**El I2C es por software**, no por hardware: los dos puertos nativos estan ocupados (~~`PB6`/`PB7` la
LCD~~, `PB10`/`PB11` el RS-485). Meter dos esclavos en un bus bit-bang, en un micro que ya hace
bit-bang del LCD y lleva el `IWDG` armado, **no es gratis**:

> ⚠️ **01/09: la conclusion aguanta, el motivo NO.** `PB6`/`PB7` —que son `I2C1`— **ya no son la
> LCD**: desde N-76 los ocupa el **Bluetooth**, `USART1` remapeado, conector `J17`.
> **MEDIDO:** `Maestro/src/bluetooth.cpp:28` → `static HardwareSerial SerialBT(PB7, PB6);`. La LCD
> vive en `PB3`/`PB4`/`PB5` por SPI de software, y desde el 31/08 ni eso: sus cuatro argumentos de pin
> son `U8X8_PIN_NONE`.
>
> Asi que `I2C1` sigue ocupado —por otro—, y **el I2C por software sigue siendo la unica salida**. Lo
> que cambia es que **el LCD ya no hace bit-bang de nada**, asi que el primer riesgo de la tabla de
> abajo —*«la lectura del expansor compite con el refresco del LCD»*— **hoy no existe**: no hay
> refresco que se vuelque al cable. Los otros dos —el margen del `IWDG` y la latencia de deteccion—
> siguen enteros y siguen siendo de banco.
>
> 🔴 **Y un choque que este apartado no ve:** propone `PB0` como `SDA`, pero **`PB0` es hoy
> `CAM_DEMANDA_PIN`**, la camara de demanda con su RC de 1 ms en la bornera `J14`, leida por nivel en
> el Maestro y por flanco en el Esclavo. Convertirlo en media linea de bus **retira una entrada que
> esta en uso**; el apartado da eso por gratis porque se escribio cuando `PB0` era *«un pin libre en
> disputa»*. Ya no lo es.

| Riesgo | Como se mide |
|---|---|
| La lectura del expansor compite con el refresco del LCD | Peor caso del ciclo de `loop()` con las dos cosas activas |
| Una transaccion I2C larga acerca el `IWDG` a su ventana | Instrumentar el margen del watchdog, no estimarlo |
| Latencia de deteccion de camara | El contacto de rele dura ~1 s: el sondeo tiene que caber con holgura |

**Ninguno de los tres se resuelve desde el PC.** Van a banco.

### Que se pide, y como se conecta

Los dos son modulos comerciales de breakout, de los que se compran hechos:

| | Modulo | Direccion I2C | Cuantos |
|---|---|---|---|
| Reloj | **`DS3231` (placa `ZS-042`)**, el azul de siempre | `0x68` (+ `0x57` de su EEPROM `AT24C32`) | uno por tarjeta **con el cristal muerto** |
| Expansor | **`PCF8574`** breakout de 8 E/S, con `A0`-`A2` a GND | `0x20` | uno por tarjeta, **siempre** |

No colisionan: `0x68`, `0x57` y `0x20` son direcciones distintas. Y los dos cuelgan de los mismos
dos hilos:

```
       TARJETA STM32                 PLACA HIJA DE EXPANSION
  +---------------------+        +----------------------------------+
  |  3.3 V  ------------+--------+--> VCC   [DS3231]   [PCF8574]     |
  |  GND    ------------+--------+--> GND      0x68       0x20       |
  |  PB0    ------------+--------+--> SDA  (los dos en paralelo)     |
  |  PB8    ------------+--------+--> SCL  (los dos en paralelo)     |
  +---------------------+        |                                   |
                                 |  PCF8574 P0 <-- Camara 1 rele 1A  |
                                 |  PCF8574 P1 <-- Camara 2 rele 1A  |
                                 |  (P2..P7 libres)      1B --> GND  |
                                 +----------------------------------+
```

### Tres avisos electricos que van al manual, no al aire

1. **`VCC` a 3,3 V. NUNCA a 5 V.** En el STM32F103, `PB0` es un pin con entrada analogica y **no es
   tolerante a 5 V** (los `FT` son otros). Los modulos suelen llevar sus resistencias de pull-up
   hacia `VCC`: si `VCC` fuese 5 V, esas pull-ups meterian 5 V en `PB0` por el bus. **Verificar en el
   datasheet del micro antes de energizar**, y corregir el Manual 11, que hoy dice *"3.3V (o 5V)"*.
2. **Dos modulos = dos juegos de pull-up en paralelo.** ~4,7 k cada uno quedan en ~2,3 k. A la
   velocidad baja de un I2C por software no deberia estorbar, pero **es una medida de banco**, no una
   suposicion: si el bus no arranca, retirar las pull-up de uno de los dos modulos es lo primero.
3. **El `ZS-042` trae diodo y resistencia de carga.** Con `LIR2032` recargable, bien. Con `CR2032` no
   recargable hay que **levantar `D1` o `R1`**. Ya esta en el Manual 11 y sigue valiendo — y no
   confundirla con la `CR2032` de `VBAT` de la placa madre, que es otra pila y otra regla (`R5`).

### El censo se construye solo, como en Baliza

No hace falta una pantalla de diagnostico ni un procedimiento aparte. **La app de Baliza ya hace lo
que se necesita: al arrancar toma la hora del celular y se la manda al controlador.** Con eso el
diagnostico sale gratis del uso normal:

```
  Al conectar:   la app manda la hora del telefono   (SET_RTC)
  En cada $STATUS: compara HORA: contra la hora del telefono

     desfase ~0                  -->  cristal SANO
     desfase que CRECE           -->  cristal MUERTO o fuera de tolerancia
     no acepta el ajuste         -->  no arranca: revisar pila / R5 / N-45
```

La clave es **medir si la hora avanza, no si esta puesta**: `reloj_enHora() == false` significa "no
esta en hora", que tambien le pasa a una unidad sana recien encendida. Confundir las dos cosas hace
comprar modulos que no hacen falta.

**Y funciona sobre unidades con el LCD muerto**, que es justo el agujero que `N-37` no pudo cubrir en
el Esclavo: *"con su pantalla muerta (N-22) no hay forma de comprobarlo desde el menu"*.

Cada tecnico que use la app en campo alimenta el censo sin proponerselo. Cuando haya numeros, se
decide cuantos expansores y cuantos `DS3231` se compran.

### Criterio de aceptacion

Censo de las unidades **antes** de comprar nada, y las tres medidas de la tabla en banco antes de dar
el expansor por bueno.

---

## 🔗 SFTY-27 — Matricula de pareja: quien obedece a quien (DISENO, **NO IMPLEMENTADO**)

> # 🔴 SI HAS LLEGADO AQUI SIGUIENDO UN PUNTERO DESDE EL FIRMWARE, ESTA NO ES LA REGLA QUE BUSCAS
>
> **`SFTY-27` designa DOS reglas distintas, y CUATRO sitios mandan a leer la equivocada.**
>
> | | |
> |---|---|
> | **Lo que dice este apartado** | matricula de pareja: `SERIE`, `PAIR`, `SITIO`/`SENTIDO`. **DISENO, no implementado** |
> | **Lo que dicen las 8 etiquetas del firmware** | *«el Esclavo PIDE y el Maestro DECIDE»* — la asimetria de `demanda_solicitar()`. **Implementada y viva** |
>
> Los ocho sitios son `Maestro/src/bluetooth.cpp` · `Maestro/src/botones.cpp` ·
> `Maestro/include/botones.h` · `Maestro/include/demanda.h` y sus cuatro gemelos del Esclavo
> [MEDIDO 01/09], mas dos packs y tres manuales. **Varios de ellos remiten literalmente a
> `OPTIMIZACIONES.md § SFTY-27`**, o sea aqui, o sea a la regla que no es — y encima marcada
> *«NO IMPLEMENTADO»* sobre una regla vial que si corre.
>
> **Renumerar es del responsable** (`AB-8` / `P-3` de `INDICE_CRUZADO.md`): son 13 sitios a la vez, y
> un numero de regla a medio cambiar es peor que uno duplicado. Hasta entonces, **este aviso es lo que
> impide que el puntero engane**. No se quita sin cerrar `AB-8`.

> **Estado:** disenado el 26/08/2026. **Nada esta implementado.** Hoy `RF_Packet` no lleva
> direccionamiento de ningun tipo —`{msgID, command, param, crc}`—, asi que dos parejas dentro del
> alcance de la radio (**1 a 3 km**, justo la distancia a la que conviven dos frentes de obra) **se
> mandan ordenes entre si**.

### Las tres piezas

**1. `SERIE` — la identidad, y no se puede editar.**
Derivada del **UID de 96 bits que el STM32F103 trae grabado de fabrica** (`0x1FFFF7E8`), reducida a
4 hex: `7A3F`. Unica, no falsificable, sin reloj, sin escribir memoria y **sin base de datos de
fabrica**. El firmware hoy **no la usa**.

> Se descarto acuñar el ID con fecha+hora al arrancar: en un equipo recien salido de taller
> `reloj_enHora()` es **falso**, asi que todas las unidades nacerian con el mismo sello.

**2. `PAIR` — el filtro, 2 bytes en la trama de radio.**
El Maestro **nace** con `PAIR` = su `SERIE`; no hay nada que configurarle, nunca. El Esclavo nace en
blanco (`0000` = sin adoptar) y **no obedece a nadie**, asi que se queda en ambar. La pregunta que se
hace la radio no es *"quien me habla"* sino *"esto es de mi pareja"*, y para eso basta un codigo
comun en los dos.

**3. `SITIO` y `SENTIDO` — las etiquetas, y esas SI se editan.**
El poste se muda cada dia; su etiqueta tiene que poder cambiar. Confundir identidad con etiqueta es
el error: **lo que no puede cambiar es quien es, no donde esta.**

### La matricula se hace por Bluetooth, no por radio

El motivo es el alcance:

```
  RADIO LoRa   1 - 3 km    <- "buscar" encontraria Maestros de OTROS frentes de obra
  BLUETOOTH    10 - 15 m   <- solo encuentra lo que tienes AL LADO
  CABLE A/B    2 nodos     <- literalmente no hay nadie mas en el bus
```

**El alcance corto del Bluetooth, que para telemetria es una limitacion, aqui es la garantia:** es
fisicamente imposible matricular por error con un Maestro que esta a 12 km. La app lee el `PAIR` del
Maestro, el tecnico camina al otro poste, y la app lo escribe con `SET_PAIR`. **Nadie teclea el
codigo** — si un humano lo escribe, un dedazo crea dos parejas con el mismo numero, que es el unico
escenario de todos que si es peligroso.

Variante de taller, mas robusta todavia: unir `A`/`B` de las dos borneras `RS485_OUT`. En un bus de
dos nodos no existe un tercero que pueda contestar.

**La app no da por bueno su propio envio:** relee la trama del equipo y compara lo que quedo escrito.
Y el LCD del Esclavo muestra `MI MAESTRO: 7A3F` de forma permanente, para poder verificar la
matricula **sin telefono**, mirando los dos postes.

### El Esclavo pide; no ordena

El operario del PMT se coloca en el extremo que haga falta, y **no tiene por que saber que es un
maestro**. Eso se resuelve sin darle mando al Esclavo:

| | |
|---|---|
| **ORDEN** — *"ponte en verde"* | el Esclavo la ejecutaria. NO |
| **PETICION** — *"hay demanda aqui"* | viaja al Maestro, que decide, aplica el todo-rojo y ordena. SI |

**El mecanismo ya existe:** `CMD_DEMANDA` (`0x11`) se añadio para la camara 3. Un boton del funcional
es exactamente lo mismo que un coche detectado: **una demanda**. No hace falta protocolo nuevo; hace
falta darle un segundo origen.

Con dos funcionales, uno en cada extremo, **el Maestro serializa**: ninguno concede nada, los dos
piden. Que pulsen a la vez tiene que ser aburrido, y asi lo es.

Y en la interfaz **no aparece "MAESTRO" ni "ESCLAVO"**: aparece el sentido que el equipo lleva
guardado. El boton dice **"solicitar paso por este lado"**, porque el funcional esta mirando el poste.

### Todos los fallos acaban en ambar

| Que sale mal | Donde acaba |
|---|---|
| Esclavo sin matricular | No obedece a nadie: **ambar**. En pantalla, `SIN ADOPTAR` |
| Matriculado con el Maestro equivocado | El Maestro correcto se queda sin Esclavo: **ambar** |
| Dos parejas en la misma via | Codigos distintos por construccion: **se ignoran** |
| App conectada al poste que no toca | El Esclavo no acepta ordenes de trafico **de nadie** |

**Ninguna salida va a verde.** Eso es lo que hace que una matricula equivocada no pueda hacer daño:
solo puede dejar sin servicio.

### Consecuencia operativa que va al manual de mantenimiento

Sustituir un **Esclavo** es trivial. Sustituir un **Maestro** obliga a re-matricular, porque su
`PAIR` es su identidad y el repuesto es otro chip. Es un gesto de un minuto con los dos equipos
delante, **pero si no esta en el manual alguien se encontrara un Esclavo en ambar sin saber por que**.
La pantalla debe decirlo: `SIN ENLACE - MI MAESTRO ES 7A3F`.

La re-matricula debe ser posible pero **deliberada**: PIN, confirmacion en pantalla y registro en la
caja negra con fecha. Es mantenimiento, no un ajuste.

### Criterio de aceptacion

Pack `costura_08_pareja` con su control negativo: una trama con `PAIR` ajeno **debe** ser descartada,
y el pack tiene que haberse visto fallar con el filtro desactivado a proposito. Y en banco: **dos
parejas encendidas a la vez**, demostrando que una orden dirigida a la pareja A no la ejecuta la B.

---

---

## 👁️ SFTY-29 — Presencia en el tramo: un VETO, nunca un atajo (**DISEÑO**, no implementado)

> **Estado:** especificado el 27/08/2026. Sustituye a la idea de *"contar vehiculos"* que arrastraban
> los manuales bajo el nombre de *"camara de umbral"* (N-59), y que se retiro de V9.0 por cara y
> fragil. **Esto es otra cosa, y es mejor.**
>
> 🔴 **07/09 — TODA ESTA REGLA CUELGA DE UN BIT QUE PUEDE NO EXISTIR (`N-159`).** Con la ficha del
> modelo **ya comprado** (`D-10`) delante, la fila `Linkage Method` enumera **cinco** metodos y
> **`trigger alarm output` NO esta**; el manual generico lo da *«only supported by certain models»*.
> El argumento con el que se daba por probable —*«la ficha pone `1 output`»*— **se retira**: ese
> borne se cierra tambien **a mano desde el navegador y por horario**, sin analitica ninguna.
> ~~**`SIN VERIFICAR`, con el peso de la prueba del lado negativo.**~~ 🟢 **12/09 — ESTA FRASE
> ESTABA CONGELADA EN EL 07/09 Y HAY MEDIDA DE CAMPO DESDE EL 10/09, EN POSITIVO:** una camara
> del Maestro con `Intrusion Detection` y `Trigger Alarm Output` dio **0 V en reposo y 3,3 V al
> detectar** en el borne (`roadmap.md` §3.8, `N-159`). ⚠️ **Pero no se lee como cerrada: NO TIENE
> ACTA** —ni captura de la lista de casillas, ni numero de serie, ni firmware de la camara— y
> **cubre UNA de las cuatro**. El estado correcto, y el mismo en todos los documentos:
> **«confirmado en UNA camara el 10/09, sin acta; el `ENSAYO 0` se repite en las otras tres»**.
>
> 🔴 **Y se retira tambien el razonamiento con el que esta fila daba la respuesta por negativa**,
> porque la auditoria del 12/09 lo tumbo desde dentro de la propia ficha: **«no esta en la lista
> de `Linkage Method`» NO significa «el equipo no puede»**. Esa misma ficha omite la entrada de
> alarma en su fila `Basic Event` mientras declara `Alarm: 1 input, 1 output` y el manual tiene un
> capitulo entero `Set Alarm Input`. **Las filas de evento de esa ficha no son un censo
> exhaustivo** (`CLAUDE.md` §7.1). Lo cierra el `ENSAYO 0` de `roadmap.md` §3.8 —diez minutos,
> solo pantalla— **en las tres camaras que faltan, y esta vez con la lista de casillas copiada
> literal**, que es lo que la medida del 10/09 no recogio.
>
> 🔴 **Y su fase de grabacion (`D-14`) NO EXISTE EN EL FIRMWARE.** *«El controlador cierra un
> contacto y la camara graba»*: **cero anclas en las dos puntas**, medido por separado por dos
> agentes el 07/09. Fuera de `semaforo.cpp` los unicos `digitalWrite` reales del Maestro son la
> direccion del RS485 y la del LoRa: **no hay contacto que cerrar.** Lo vigila desde hoy
> `decisiones_01_anclas`, **y el banco esta en rojo por ello a proposito**.

### La distincion que lo cambia todo

Las camaras 2 y 4 no cuentan: **detectan presencia**. Y esa presencia no autoriza a ir mas deprisa,
solo puede decir *"todavia no"*.

| | conteo *(descartado)* | **presencia** *(esto)* |
|---|---|---|
| que viaja por radio | un mensaje por cada coche que entra y otro por cada uno que sale | **un bit**: sigue ocupado / libre |
| que autoriza | **ACORTAR** el todo-rojo | **RETRASAR** el verde |
| si la deteccion falla | se acorta un despeje que no debia acortarse -> **dos vehiculos de frente** | se cae al temporizador de siempre -> **nada peor que hoy** |
| si detecta de mas | — | se espera un poco mas |

**Los dos modos de fallo caen del lado seguro.** Por eso esta regla no debilita el todo-rojo: lo
refuerza. Y por eso el conteo se descarta y esta version no.

### Y la objecion de la radio desaparece

El enlace va a **2,4 kbps y es semiduplex**: por ese unico canal viajan las ordenes que mueven las
luces, sus acuses, el latido y la hora. Contar vehiculos obligaba a decenas de mensajes por minuto
en hora punta, compitiendo con el `CMD_GO_RED`. **Un bit no compite con nada** — y ademas no hace
falta ninguna trama nueva:

```
   struct RF_Packet { uint8_t msgID; uint8_t command; uint8_t param; uint8_t crc; };
                                                       ^^^^^
   MEDIDO el 27/08: el Esclavo manda CMD_ACK_RED con programarRespuesta(CMD_ACK_RED),
   o sea param = 0 por defecto, y el Maestro (coordinador.cpp:743) NO LO LEE.
   El byte esta libre, y llega EXACTAMENTE en el instante que importa: cuando el
   Esclavo confirma que ya esta en rojo y empieza a correr el todo-rojo.
```

**Coste en aire: cero.** Cero comandos nuevos, cero bytes nuevos, cero cambios en el formato.

### La maquina, y su unico peligro real

```
   El Maestro recibe CMD_ACK_RED
        |
        +-- param bit0 = 0  (tramo libre)  --> todo-rojo normal, y al acabar, VERDE
        |
        +-- param bit0 = 1  (aun ocupado)  --> EXTIENDE el todo-rojo
                                               |
                                               +-- se libera antes del tope --> VERDE
                                               |
                                               +-- se llega al TOPE ---------> VERDE IGUAL
                                                                               + $ALARM
```

> 🔴 **EL TOPE NO ES UN DETALLE: ES LA REGLA.** Barro en el lente, un camion aparcado en el punto de
> vigilancia o un sensor pegado en activo, y **el cruce se congela para siempre** esperando que se
> libere. Un enclavamiento sin tope no es mas seguro: es un semaforo colgado, con cola en las dos
> puntas y nadie entendiendo por que.
>
> La regla es: **extender hasta un maximo configurable; al llegar, cambiar igual y levantar alarma**
> en la caja negra (`$ALARM,EVENTO:PRESENCIA_PEGADA,...`). El operador se entera de que hay un sensor
> averiado **sin que el cruce deje de regular**.

### La segunda funcion, que es local y no toca la radio

**No bajar la pluma sobre un vehiculo.** El mismo sensor, leido por el propio poste, impide bajar la
barrera mientras haya algo debajo — igual que la fotocelula de un porton. No interviene el Maestro, no
viaja nada por radio, y es la funcion que evita el dano material y la reclamacion.

Aqui el fallo seguro va al reves que arriba, y conviene tenerlo claro:

| el sensor se queda… | que pasa | es seguro? |
|---|---|---|
| **pegado en ACTIVO** | la pluma **no baja** | ✅ si: la barrera deja de proteger, pero **la luz sigue regulando**. Con alarma |
| **pegado en INACTIVO** | la pluma baja sobre un coche | ❌ no. Por eso el sensor va **normalmente cerrado** si el modelo lo permite: un cable cortado se lee como "hay algo" |

### Donde entra fisicamente

Cada tarjeta necesita **una segunda entrada** —la primera la ocupa la camara de demanda en `J14`—.
Tres caminos, y **lo decide la pregunta 5 del acta de banco**:

**Decidido el 27/08: la entrada es `PA11` (pad 32 del `U1`).**

> ### ⚠️ 01/09/2026 — LA PREMISA CAMBIO EL 31/08: YA HAY DOS ENTRADAS MAS, Y ESTAN EN EL FIRMWARE
>
> **Esto no invalida la eleccion de `PA11`, pero cambia por completo la urgencia**, y hay que leerlo
> antes de tirar un hilo a un pad.
>
> **MEDIDO 01/09.** El reparto de `J16` (N-97) dio a las dos puntas **dos entradas de camara nuevas**,
> ya configuradas y ya leidas en cada vuelta del `loop()`:
>
> ```
> Maestro/include/pines.h:124-125   CAM_C_PIN  PB14  (J16 p10)   INPUT pelado, activo en ALTO
>                                   CAM_D_PIN  PB15  (J16 p12)   INPUT pelado, activo en ALTO
> Maestro/src/botones.cpp:156-157   pinMode(CAM_C_PIN, INPUT); pinMode(CAM_D_PIN, INPUT);
> Esclavo/src/botones.cpp:176-177   identico
> ```
>
> Las dos entran por `demanda_solicitar()` y se leen **por flanco en las dos puntas**, con siembra del
> nivel al arrancar para que un contacto ya cerrado al encender no cuente como deteccion.
>
> **Consecuencia para SFTY-29, dicha con precision:** lo que hoy falta **ya no es una entrada
> electrica** —hay tres: `PB0` en `J14` y `PB14`/`PB15` en `J16`—. Lo que falta es que **una de ellas
> signifique PRESENCIA en vez de DEMANDA**, que es una decision de firmware y de cableado, no de pines.
> Y son cosas opuestas: una demanda **pide** el verde, una presencia **lo retrasa**. Reusar una entrada
> de demanda como presencia sin cambiar quien la lee seria pedir paso justo cuando hay que negarlo.
>
> ~~⚠️ **Y lo que NO se ha movido: `J16` sigue sin poder cablearse.** La medida `M3` —la contradiccion
> entre el netlist y el fuente sobre la polaridad de esos pines— **sigue abierta**, y hasta que se
> cierre con ohmimetro **no se cablea camara a `J16`**, ni de demanda ni de presencia.~~
> 🔴 **DEROGADO POR `D-25` y `D-27` (11/09), y esta frase era la mas peligrosa de este documento
> porque contradecia una INSTRUCCION DE CAMPO en vigor:** las cuatro camaras van a `J16` —camara 1
> entre p9 (3,3 V) y p10 (`CAM_C_PIN`), camara 2 entre p11 (3,3 V) y p12 (`CAM_D_PIN`), conmutando
> **hacia +3,3 V y nunca contra masa**—, esta cerrado por el responsable, y las guias ya se lo
> mandan al instalador. La polaridad **se resolvio**: `pinMode(..., INPUT)` pelado y
> `digitalRead() == HIGH`, con 9,93/9,94 kOhm a masa y 0 V en reposo medidos en cobre el 03/09.
> Lo que queda de aquella familia de medidas es **`M4` y solo eso**: nadie ha puesto un
> multimetro en **p9** —el paso 21 ejercito p10 contra p11—, y sigue marcado ⬜ banco en
> `MAPEO_TARJETA_KICAD.md`. Es una comprobacion de banco, **no un veto al cableado**. `PA11` sigue
> siendo la eleccion correcta para una entrada **con bornera propia**; lo que ya no es cierto es que
> haga falta un hilo a un pad **para tener por donde entrar**.

Se midio el cobre del `.kicad_pcb`, y lo dice el propio enrutador: `PA11`, `PA12`, `PA15` y `PC13`
salen como `unconnected-(U1-PA11-Pad32)` y equivalentes — **pads sin una sola pista**. En la misma
pasada los ocupados aparecen con su red (`PB0 -> /Puerta`, `PB2 -> /Motor`, `PB1 -> /Buzzer`), asi que
el metodo esta validado y no es una suposicion.

**Por que `PA11` y no otro:** es tolerante a 5 V, **no depende de que el JTAG este desactivado** -a
diferencia de `PA15`- y no vive en el dominio de respaldo con sus limitaciones, como `PC13`. `PA12`
queda de reserva por si el pad de `PA11` resulta inaccesible en la tarjeta real.

**Como se cablea, y por que asi:**

```
   pad 32 (PA11) --[ hilo AWG30, fijado con kapton ]--> via muerta de J16 --> bornera
                                                          (cual, lo dice el multimetro)

   Sensor NORMALMENTE CERRADO a masa. En reposo: contacto cerrado, pin a masa (LOW).
   Con vehiculo: el contacto ABRE y el pin sube por el pull-up interno (HIGH = presencia).
```

**Un cable cortado se lee como "hay algo"** — el todo-rojo se extiende y la pluma no baja. El fallo
del cableado cae del lado seguro, y el TOPE de mas abajo impide que un sensor averiado congele el
cruce. Esa combinacion -NC mas tope- es lo que hace que esta entrada no pueda empeorar nada.

> ⚠️ **Correccion del 27/08:** una version anterior decia que `J16` llevaba la pantalla y los botones.
> **Medido: `J16` es el conector de la BOTONERA** -cuatro botones con su 3,3 V y los 12 V- y le sobran
> dos o tres vias muertas. La pantalla va por otro sitio. La conclusion no cambia -hay vias libres y
> hay pines libres-, pero el dato si.

| alternativa | coste | deja libre |
|---|---|---|
| **`PA11` + hilo a via muerta de `J16`** *(elegida)* | un hilo | `PA12`, `PA15`, `PC13` |
| pad de `PB8`, retirando `R16` y el LED `D5` | un hilo + 2 componentes | los cuatro pines libres |
| placa hija con `PCF8574` | chip + bus bit-bang (~800 B) + un modo de fallo nuevo | **6 E/S** para el reloj y lo que venga |

### Lo que hay que medir ANTES de escribir una linea

- **El flash.** El bit y la maquina de extension son pocas decenas de bytes; la alarma y el texto de
  pantalla, mas. ~~Quedan **4.684 bytes** en el Maestro~~ → **quedan 7.240 B** (acta del 01/09,
  HEAD `aa69349`: `58296` de `65536`, 89,0 %). Se compila con y sin, y se compara el acta. Sin esa
  cifra, cualquier estimacion de aqui es una intuicion.
  > *La cifra vieja era del 27/08 y sobrevivio a dos cambios de presupuesto —N-70 libero 5.160 B de
  > `Wire`, N-86 otros 16—. **Un numero de flash escrito en prosa caduca cada vez que alguien
  > compila**; se copia del acta, nunca se recuerda.*
- **La polaridad del sensor**, como en N-67: la manda la placa, no el gusto de nadie.

### El pack que lo vigilara, y la comprobacion que nadie escribe

`presencia_01_veto` tendra que exigir, sobre el C++ real:

1. Que la presencia **solo extienda**: que no exista ningun camino donde acorte el todo-rojo. Es la
   propiedad central, y se mide sobre las escrituras de pin como SFTY-2.
2. Que el bit se lea del `param` de `CMD_ACK_RED` y de ningun otro sitio.
3. **Que el tope funcione** — y este es el control negativo que se olvida siempre: con la presencia
   forzada a activa **para siempre**, el cruce **tiene que cambiar igual** al llegar al maximo y
   emitir la alarma. Un pack que solo pruebe el caso bueno estaria certificando el cuelgue.
4. Que la pluma no baje con presencia, y que un sensor ausente no la deje bajar sola.

### Lo que esta regla NO hace

No cuenta vehiculos, no acorta el despeje, no sustituye al temporizador y **no se apoya en la radio
para nada critico**: si el bit no llega, el Maestro hace exactamente lo que hace hoy. Esa es la razon
de que se pueda añadir sin volver a discutir el todo-rojo entero.

---

## 🚧 SFTY-28 — Talanquera acoplada al estado del semaforo (**IMPLEMENTADA la regla; abiertas las decisiones de operacion**)

> ⏸️ **PENDIENTE ANOTADO EL 05/09/2026 — NO EJECUTADO AQUÍ, A PROPÓSITO.** El responsable dijo que
> *«la barrera puede no bajar y el semáforo cambia igual»*, lo que **deroga** el sentido único de
> esta regla. Pero la derogación formal es **`A-1.bis` de [`DECISIONES.md`](DECISIONES.md)** —*«¿se
> deroga SFTY-28 (la talanquera sigue a la luz, nunca al revés)?»*—, y va **con la fase 2 de D-13,
> en otro lote**. Esta sección **no se reescribe todavía**.
>
> Se anota en vez de ejecutarse por §2.quinquies de `CLAUDE.md`: *una frase nueva no deroga una
> decisión escrita*. Y con más razón porque el cambio **retira una barrera** —el veto de la pluma—,
> que es la dirección en la que un malentendido no se nota hasta que alguien está en la calzada.

> **Estado:** anotado el 26/08/2026 y **construido el 27/08** en las dos puntas.
>
> **Lo que ya corre:** la orden sale de `escribirPines()` —la misma puerta que las lamparas— y
> sigue al `verde` YA enclavado; el arranque la deja cerrada; el nivel de reposo del pin es el
> de CERRAR, de modo que un equipo apagado no deja la via abierta. **Coste: +24 bytes por
> punta**, sin drivers ni bus.
>
> **Lo que lo mide, en dos planos distintos:** el arnes del automatico vigila en CADA tick de
> los nueve bloques que la pluma nunca este arriba con los dos verdes apagados —sobre lo que el
> `semaforo.cpp` real escribio en el pin, con su control negativo al lado—, y el pack
> `barrera_03_talanquera` fija la estructura que el arnes no puede ver: que **ningun otro**
> `.cpp` de las dos puntas escriba ese pin —censando el directorio, no una lista—, que la orden
> viva dentro de `escribirPines()` y no en una funcion suelta, y que las dos puntas lo hagan
> igual. Visto caer a `68/69` con la pluma forzada a ABRIR en el `.cpp` real.
>
> 🟡 **Lo que sigue abierto, y NO lo decide el firmware:** que hace la pluma con el ambar
> intermitente de SFTY-6 (sin enlace). Hoy queda **abajo** —es lo conservador: solo sube con
> verde confirmado—, pero *«cerrar la via por completo»* frente a *«dejar pasar con
> precaucion»* es decision del cliente y del PMT. Cambiarla es **una linea** en
> `escribirPines()`, y el dia que se cambie hay que actualizar la tabla de abajo y el Manual 1.
>
> 🔴 **Y sigue sin resolverse el hardware:** ~~son 10 demandas para 9 MOSFET —el buzzer y la
> talanquera se disputan el unico driver libre—~~, y a donde sale `PB2` de verdad esta **por
> confirmar con multimetro** (tarea `B3` de `ESTADO.md`). El firmware ya escribe el pin; que
> ese pin mueva un motor es lo que falta comprobar.
>
> > ⚠️ **01/09: la mitad tachada contradecia a su propia correccion, tres pantallas mas abajo.** El
> > bloque *«RESUELTO el 27/08»* de este mismo apartado ya media **DIEZ** MOSFET y DIEZ optos, y este
> > encabezado seguia publicando los nueve de la primera version. **Re-medido sobre el plano bueno**
> > (`01_Firmware/Controladora_Semaforos/Controladora_Semaforos/Controladora_Semaforos.kicad_sch`,
> > 649.224 B):
> >
> > ```
> > grep -oE '"Q[0-9]+"' ... | sort -u   ->  Q1 .. Q10   (diez)
> > grep -oE '"U[0-9]+"' ... | sort -u   ->  U1 .. U15
> > ```
> >
> > **No hay disputa: el buzzer tiene `Q8` y la talanquera `Q10`.** Lo que sigue abierto es solo `B3`
> > —a donde sale `PB2` en el cobre—, y eso no es un problema de reparto de drivers.

### Lo que ya existe y no se usa

```
   pines.h:21   #define MOTOR_TALANQUERA   PB2   // J15
```

~~Ese pin esta declarado en las dos puntas, tiene **MOSFET de potencia y bornera propia**, y
**ninguna linea del firmware lo escribe** — comprobado con el censo de `pinMode()`: no aparece.~~

> ✅ **CADUCADO: desde el 27/08 el firmware SI lo escribe, y este apartado se quedo describiendo la
> vispera.** [MEDIDO 01/09, las dos puntas, `grep -rn MOTOR_TALANQUERA */src/`]
>
> ```
> Maestro/src/semaforo.cpp:203   pinMode(MOTOR_TALANQUERA, OUTPUT);
> Maestro/src/semaforo.cpp:204   digitalWrite(MOTOR_TALANQUERA, TALANQUERA_CERRAR);
> Maestro/src/semaforo.cpp:93    digitalWrite(MOTOR_TALANQUERA, ...)   <- dentro de escribirPines()
> Esclavo/src/semaforo.cpp       identico, mismas lineas
> ```
>
> El titulo de la seccion —*«lo que ya existe y no se usa»*— **ya no describe a la talanquera**. Se
> conserva el apartado porque lo que sigue —para que sirve, la regla de que SIGUE al semaforo, las
> tres preguntas de operacion y el actuador— es lo que no ha cambiado.
>
> 🔴 **Lo que SI sigue siendo hardware pagado y muerto, y ahora esta medido en vez de citado de
> pasada:** `ROJO_PEATON` (`PA6`), `VERDE_PEATON` (`PA7`) y el `BUZZER` (`PB1`). Estan declarados en
> `*/include/pines.h` y **no tienen ni un `pinMode`, ni un `digitalWrite`, ni un `digitalRead` en
> ninguna de las dos puntas** — el censo devuelve **cero** llamadas, y la unica aparicion de esos
> nombres fuera de `pines.h` es un comentario en `Maestro/src/main.cpp:35`. Es N-96, y su parte
> incomoda no es el hardware muerto sino que **`barrera_01_pines_de_luz` no puede detectarlo**: acepta
> `len(luces) >= 6` sobre una lista que devuelve 8, y su control negativo nunca ejercio un pin
> peatonal. **Una regla de seguridad que enumera sujetos tiene que comprobar que cada sujeto existe.**

### Para que sirve una talanquera aqui

En un paso alternado la barrera fisica hace lo que la luz no puede: **detener al que no mira**. En
obra con maquinaria pesada, o de noche, o con conductores que ya han visto veinte semaforos de obra
esa semana, el rojo se salta. Una barrera, no.

### 🛑 La regla, y es la misma que gobierna todo lo demas aqui

> **La talanquera SIGUE al semaforo. Nunca lo manda, y nunca lo contradice.**

| Luz | Talanquera |
|---|---|
| 🔴 Rojo | **BAJADA** |
| 🟡 Ambar (transicion) | **BAJADA** — solo sube cuando el verde esta confirmado |
| 🟢 Verde | **SUBIDA** |
| Todo-rojo de despeje | **BAJADA en las dos puntas** |
| Ambar intermitente (SFTY-6, sin enlace) | ✅ **SUBIDA** — decidido por el cliente el 27/08/2026: sin enlace se deja pasar con precaucion, que es lo que ese ambar significa en la calle. Cerrar la via dejaria un corredor de obra sin salida |

Y la consecuencia estructural, que es lo que la hace segura: **la orden de la talanquera sale del
mismo sitio que la luz, `semaforo.cpp`, y de ningun otro.** Es `§6` extendida: si un modo pudiera
mover la barrera por su cuenta, tendriamos una barrera abierta con la luz en rojo, que es peor que
no tener barrera — porque el conductor confia en ella.

### ⚠️ Las tres preguntas que hay que cerrar antes de escribir una linea

1. **¿Que hace al perder el enlace?** SFTY-6 cae a ambar intermitente, que significa *"pasa con
   precaucion"*. Una barrera **bajada** ahi cierra la via por completo; **subida**, deja pasar a los
   dos lados a la vez. Ninguna de las dos es obviamente correcta y la decision es del cliente y del
   PMT, no del firmware.
2. **¿Que hace al cortarse la energia?** Una talanquera que se queda arriba con el equipo muerto es
   una via abierta sin regulacion. Esto **no se resuelve en software**: se resuelve eligiendo un
   actuador que caiga por gravedad o con muelle de retorno. Va en la especificacion de compra.
3. **¿Y si no llega a bajar?** Un final de carrera que confirme la posicion convierte "ordene bajar"
   en "esta bajada", que no es lo mismo. Sin realimentacion, el firmware no puede saberlo y **no debe
   fingir que si**. Si no hay final de carrera, el manual tiene que decir que la barrera es una ayuda
   visual y que **quien regula sigue siendo la luz**.

### 🔌 En que salida se acciona — MEDIDO sobre el esquematico, y hay un problema

```
   MOSFET de potencia en la placa:  9 x IRLZ44N  (Q1..Q9)
   Pines de luz en pines.h:         8            (ROJO/AMARILLO/VERDE 1 y 2 + 2 peatonales)
   -> queda UN driver de potencia libre
```

> ### ✅ RESUELTO el 27/08 — y CORREGIDO el mismo dia, porque la primera medida uso el plano malo
>
> ⚠️ **Primera version (equivocada):** *"son 10 demandas para 9 MOSFET; la talanquera se queda sin
> etapa y `J15` no existe"*. Salio de `03_Hardware_Tarjeta/KiCad/*.kicad_sch`, que esta **incompleto**
> —sin LCD, sin botones y sin el canal del motor— y cuyo `.kicad_pcb` **esta vacio (78 bytes)**.
>
> > ⚠️ **Nota de rastro, 01/09: esa ruta ya no existe, y el `.kicad_pcb` de 78 B tampoco esta ahi.**
> > [MEDIDO] `03_Hardware_Tarjeta/` contiene **un solo fichero**, `MAPEO_TARJETA_KICAD.md`: no hay
> > `KiCad/`, ni `.kicad_sch`, ni `.kicad_pcb`. Los cinco ficheros de **78 bytes** que quedan viven en
> > `99_Legacy/Controladora_Semaforos-backups/`. Y el `.kicad_pcb` del **plano bueno** pesa
> > **2.158.421 B** y trae cobre de verdad —es el que se midio para decidir `PA11`—.
> >
> > Se anota porque **esta frase se sigue citando como prueba de que «el otro plano estaba vacio»**, y
> > quien vaya a verificarla hoy no encontrara el fichero y no sabra si es que se borro o si la frase
> > era falsa. **Era cierta cuando se midio; la ruta caduco.** Lo que sigue en pie y es lo que importa:
> > **hubo dos planos, se leyo el que no era, y hoy solo queda uno.**
>
> **Medido sobre el plano bueno** (`01_Firmware/Controladora_Semaforos/.../*.kicad_sch`, 649 KB):
> **son DIEZ MOSFET y DIEZ optos** (`Q1..Q10`, `U6..U15`). No falta ninguno:
>
> | red | por donde pasa |
> |---|---|
> | `S1`…`S8` | R 220 + R 10K -> opto `TLP127` -> MOSFET `IRLZ44N` -> bornera. Ocho canales identicos |
> | `Buzzer` (`PB1`) | **el mismo camino**: `R55`+`R54` -> opto `U13` -> MOSFET `Q8` -> bornera `J13` con 12 V |
> | `Motor` (**`PB2`**) | **su propio canal completo**: `R70`+`R69` -> opto **`U15`** -> MOSFET **`Q10`** -> bornera **`J15`** |
> | `Puerta` (`PB0`) | `R64` 10 kOhm + `C25` 100 nF -> bornera `J14`. Es un **RC de 1 ms: una ENTRADA con antirrebote por hardware**, que es donde el firmware lee la camara de demanda |
> | `PB8` | `R16` 1 kOhm -> **LED `D5`**. Es un indicador, no una entrada optoacoplada de camara |
> | `PB3..PB7` · `PB9`,`PB13..PB15` | la pantalla ST7920 (`SCL`,`CS`,`SI`,`RS(A0)`,`RST`) y `Boton1..4` |
> | `PA11`, `PA12`, `PA15`, `PC13` | **sin cable**: los unicos pines realmente libres, y sin bornera |
>
> **Conclusion: la talanquera ya esta donde tiene que estar.** `MOTOR_TALANQUERA = PB2` es exacto, no
> hace falta mover nada, no hace falta modulo de rele externo y **no hay que sacrificar el buzzer**.
> Lo unico que sigue sin entrada fisica es la **segunda camara** (la de umbral): `PB8` alimenta un LED
> y los cuatro pines libres no tienen bornera. Eso es un hilo, no un chip.
>
> 🔴 **Y la leccion, que costo una conclusion equivocada publicada:** habia **dos esquematicos** en el
> repositorio y `ESTADO.md` apuntaba al incompleto. *Un "no aparece" no es un hallazgo hasta haber
> descartado al buscador* — aqui el buscador leia el plano que no era. **Dos copias de un plano son
> peores que ninguna**, y hay que dejar una sola.

> 🔴 ~~**Y un dato falso en el fuente, encontrado al medir:** `pines.h` dice
> `#define MOTOR_TALANQUERA PB2 // J15`, pero **la bornera `J15` no existe**. El esquematico tiene
> `J1`..`J14` y `J16`. Quien vaya a cablear buscando `J15` no la va a encontrar. Hay que trazar a
> donde sale `PB2` de verdad —probablemente por el conector de 12 vias `J16`— y corregir el
> comentario.~~
>
> > ### 🔴 01/09/2026 — REFUTADO. `J15` SI EXISTE, Y ESTA ACUSACION SE CONTRADECIA CON LA TABLA DE ARRIBA
> >
> > Dos parrafos por encima, la tabla del *«plano bueno»* ya decia `Motor (PB2) -> opto U15 -> MOSFET
> > Q10 -> bornera J15`. Este aviso decia lo contrario **en la misma seccion**, y las dos cosas no
> > podian ser ciertas.
> >
> > **MEDIDO 01/09** sobre el plano bueno (649.224 B), enumerando en vez de buscando una sola:
> >
> > ```
> > grep -oE '"J[0-9]+"' Controladora_Semaforos.kicad_sch | sort -u -V
> >   ->  J1 J2 J3 J4 J5 J6 J7 J8 J9 J10 J11 J12 J13 J14 J15 J16 J17
> >
> > "J15" aparece como (property "Reference" "J15") y como (reference "J15"),
> > exactamente igual que J13, J14, J16 y J17: dos apariciones cada uno.
> > ```
> >
> > **La lista `J1..J14` y `J16` que este aviso publicaba era del plano incompleto**, el mismo error
> > que su propio parrafo de mas arriba dice haber cometido y corregido. El comentario del fuente ya
> > esta bien y **ya se corrigio**: `Maestro/include/pines.h:31` dice hoy
> > `// -> opto U15 -> MOSFET Q10 -> bornera J15`.
> >
> > **La leccion no cambia, cambia de sujeto:** *un «no aparece» no es un hallazgo hasta haber
> > descartado al buscador*. Aqui el buscador era el plano equivocado **por segunda vez, dentro del
> > apartado que estrenaba esa misma leccion**. Una refutacion tambien es un instrumento (§4 de
> > `CLAUDE.md`): tacharla exige el mismo rigor que afirmarla, y por eso esto va con el `grep` pegado.

### ⚠️ Y un MOSFET no basta para una talanquera

Un IRLZ44N de canal N conmuta **encendido/apagado** contra masa. Un motor de barrera necesita **dos
sentidos**, y eso no sale de un solo transistor. Las tres salidas posibles, de menos a mas trabajo:

| Opcion | Salidas que consume | Nota |
|---|---|---|
| **Barrera con controlador propio** que acepte contacto seco de apertura | **1** | Es el caso industrial habitual y el unico que cabe hoy. El controlador de la barrera se ocupa del motor, los finales de carrera y la seguridad antiaplastamiento |
| Dos salidas de pulso, abrir y cerrar | 2 | No caben sin tocar la placa |
| Puente en H externo | 1-2 + placa auxiliar | Se asume el control del motor, y con el la responsabilidad de no aplastar a nadie |

**La primera es la unica que cabe en la placa actual, y ademas es la correcta**: dejar el motor y su
seguridad al equipo que esta disenado para eso, y que el semaforo solo diga *abre* o *cierra*.

### Coste

Un pin ya declarado y **el ultimo MOSFET libre —si el buzzer no se lo lleva—**. En flash, unas pocas lineas
dentro de `escribirPines()`. Lo caro no es el codigo: son las tres decisiones de arriba y el
actuador.

### Criterio de aceptacion

Pack que barra **todos** los estados del semaforo y exija que la talanquera nunca este arriba con el
verde apagado — con su control negativo, forzando el caso contrario. Y en banco: cortar la energia
con la barrera arriba y comprobar que baja sola.

---

# MUDANZA DEL 12/09/2026 - los apartados 3, 6, 7, 8, 9 y 10 de `roadmap.md`

**Que se mudo, de donde y desde que commit.** El 12/09/2026 se sacaron de
[`roadmap.md`](roadmap.md) sus apartados **3** (*«Lo que se puede hacer HOY desde el PC»*),
**6** (*«El porque de lo que sigue abierto»*), **7** (*«Los `N-x` cerrados»*),
**8** (*«La arquitectura vigente»*), **9** (*«La V2»*) y **10** (*«Lo anterior, y donde vive»*),
**integros y literales**, sobre el commit **`b4c641e`**. Van abajo en el mismo orden y con el
mismo texto: no se resumio, no se reescribio y no se tacho nada nuevo.

**Por que se mudaron.** `roadmap.md` tenia **2.695 lineas**, y un fichero que no se puede leer
entero **no lo lee nadie entero**: se lee a trozos, y entonces cada lector deriva su propia
version. El responsable puso el tope en **600 lineas** el 12/09 y la regla vive en la cabecera de
aquel fichero. **Aqui esta el porque; alli quedo solo lo pendiente.**

> 🔴 **ANTES DE MUDARLO SE COMPROBO, APARTADO POR APARTADO, QUE NADA ABIERTO SE ENTERRABA.** Los
> ~24 pendientes que solo vivian en estos seis apartados se subieron a **`0`** de `roadmap.md` el
> 12/09 en **18 filas nuevas** (`4cb0990`), y cada uno se verifico contra su fila antes de mover su
> apartado. **Lo de aqui es el PORQUE de esos pendientes, no los pendientes.**
>
> ⚠️ **Y vale la advertencia general de este fichero:** un parrafo de aqui describe **lo que se
> sabia el dia que se escribio**. Varios de estos `N-x` estan cerrados en el fuente desde entonces,
> y algunos llevan su cierre tachado dentro. **Antes de actuar sobre una frase de aqui se
> comprueba contra `DECISIONES.md`, contra la spec o contra el fuente.**

> ⚠️ **LOS NUMEROS DE APARTADO NO SE RENUMERARON, A PROPOSITO** (`CLAUDE.md` §5). El firmware
> (`Esclavo/src/bluetooth.cpp`, `Maestro/src/bluetooth.cpp`, `Maestro/include/reloj.h`,
> `ESP32_Expansion/src/despachador.cpp`), cuatro packs, `dos_puntas/orquestador.cpp`,
> `ARQUITECTURA.map` y once documentos citan `roadmap §3.16`, `§3.10.bis`, `§3.4.bis`, `§3.10`,
> `§3.8` y `§3.11.ter` **desde fuera**: renumerar los habria dejado cojos a todos. **Se buscan
> aqui con el mismo numero con el que se citan alli.**

## Indice de lo mudado el 12/09 - por apartado

| apartado |
|---|
| 3. Lo que se puede hacer HOY desde el PC |
| &nbsp;&nbsp;· 3.1 · 🔴 Defectos abiertos con arreglo conocido |
| &nbsp;&nbsp;· 3.2 · 🟢 `DECISIONES.md` — REVALIDADO ENTERO EL 08/09, ~~y ya no queda ninguna fila caducada~~ y el 11/09 quedaban OCHO textos caducados |
| &nbsp;&nbsp;· 3.3 · 🔴 La evidencia del banco NO esta donde 8 documentos dicen |
| &nbsp;&nbsp;· 3.4 · 🟠 Arreglos documentales que hacen dano si no se hacen |
| &nbsp;&nbsp;· 3.4.bis · 🔴 `D-20` — el reloj se muda del STM32 al ESP32. DECIDIDO el 07/09, ~~sin construir~~ CONSTRUIDO EN PARTE |
| &nbsp;&nbsp;· 3.4.ter · 🔴 `D-21` y `D-22` — las dos que salieron de preguntar «¿y como lo SABE el Esclavo?» |
| &nbsp;&nbsp;· 3.4.quater · 🎯 EL ORDEN DE CONSTRUCCION, del mas aislado al que toca la calzada |
| &nbsp;&nbsp;· 3.5 · 🔴 Lo que salio de ANCLAR las decisiones en el codigo (07/09) — `N-158` |
| &nbsp;&nbsp;· 3.5.bis · 🟢 `decisiones_01_anclas` — el instrumento que corta el bucle, y sus dos mitades |
| &nbsp;&nbsp;· 3.6 · 🟠 Defectos de los propios INSTRUMENTOS — dos del 07/09, uno del 08/09, y una mecanica |
| &nbsp;&nbsp;· 3.7 · 🔴 Las etiquetas de las dos cabezas PEATONALES van al reves que los conectores |
| &nbsp;&nbsp;· 3.8 · 🟢 `N-159` — la analitica de las camaras COMPRADAS probablemente no acciona el rele |
| &nbsp;&nbsp;· 3.9 · 🟠 `ESP32_Expansion` no esta en el censo de funciones huerfanas |
| &nbsp;&nbsp;· 3.10 · 🔴 `N-160` — el `20/20` que se compro con COMENTARIOS, y el defecto que aparecio al auditarlo |
| &nbsp;&nbsp;&nbsp;&nbsp;· 🔴 EL DEFECTO VIVO QUE APARECIO AL AUDITAR — y este SI es firmware |
| &nbsp;&nbsp;&nbsp;&nbsp;· 3.10.bis · Lo que se ARREGLO, y las DOS preguntas que quedaron para el responsable |
| &nbsp;&nbsp;&nbsp;&nbsp;· 3.10.ter · 🟢 `N-160` CERRADO en las dos puntas — y el residual que dejo, que es el mismo defecto una capa arriba |
| &nbsp;&nbsp;· 3.11 · 🎯 Lo que el responsable DECIDIO la noche del 07/09, y las DOS que quedaron abiertas |
| &nbsp;&nbsp;&nbsp;&nbsp;· 3.11.bis · 🔴 Las DOS medidas que le pedi al responsable y que NO HACIAN FALTA |
| &nbsp;&nbsp;&nbsp;&nbsp;· 3.11.ter · Lo que dijo la spec al abrirla — y la decision que devuelve |
| &nbsp;&nbsp;· 3.12 · 🟢 `N-161` — el paquete de entrega salia del DISCO con el nombre de un commit, y su LEEME bloqueaba lo ya desbloqueado |
| &nbsp;&nbsp;· 3.13 · 🟢 `N-154` CERRADO — el `$ALARM` cabia por VALOR y no por BUFFER, y lo que se perdia era la HORA |
| &nbsp;&nbsp;&nbsp;&nbsp;· La parte que vale para el siguiente: se retiro una EXENCION midiendo, no borrandola |
| &nbsp;&nbsp;· 3.14 · 🎯 EL ORDEN DE DESARROLLO AL 08/09 — lo que queda, y por que en este orden |
| &nbsp;&nbsp;· 3.15 · 🟢 `D-24` — `CAM_CIEGA` a 24 h, y las TRES formas en que una lista de alcance se queda corta |
| &nbsp;&nbsp;&nbsp;&nbsp;· 3.15.bis · 🟢 La SEGUNDA mitad de `D-24` — y por que salio CERO FIRMWARE |
| &nbsp;&nbsp;· 3.16 · 🔴 `N-162` — El Sisga (10/09): la PRIMERA vez que V9 toca una calle, y la rama que salio de ahi, VALIDADA POR EL DIFF el 11/09 |
| &nbsp;&nbsp;&nbsp;&nbsp;· Lo que las guias le decian al instalador, contra el firmware y la spec |
| &nbsp;&nbsp;&nbsp;&nbsp;· 🎯 `D-25` — las CONEXIONES de aquella guia, definitivas (11/09, el responsable) |
| &nbsp;&nbsp;&nbsp;&nbsp;· 🎯 `D-27` — lo que la guia fija, cerrado (11/09, el responsable) |
| &nbsp;&nbsp;&nbsp;&nbsp;· DAR PASO — el diagnostico del 10/09 NO se sostiene |
| &nbsp;&nbsp;&nbsp;&nbsp;· Lo que aparecio al auditar, y NO es de esta rama |
| &nbsp;&nbsp;&nbsp;&nbsp;· La cinta y el diario del Maestro (10/09, 12:06–12:26, firmware `7ff7d12`), LEIDOS ENTEROS |
| &nbsp;&nbsp;&nbsp;&nbsp;· 🎯 El veredicto del arquitecto sobre el diff de `D-26` (11/09) — «FUSIONAR CON CAMBIOS» |
| 6. El porque de lo que sigue abierto — los hallazgos que no se han cerrado |
| &nbsp;&nbsp;· 6.1 · La tarjeta Maestro |
| &nbsp;&nbsp;· 🛑 N-116 — El Maestro se calienta a los ~30 s: el firmware queda DESCARTADO por censo, no por opinion |
| &nbsp;&nbsp;&nbsp;&nbsp;· Lo que esta MEDIDO: el firmware no puede ser la fuente del calor |
| &nbsp;&nbsp;&nbsp;&nbsp;· Una causa que se cayo, y se marca refutada en vez de borrarse |
| &nbsp;&nbsp;&nbsp;&nbsp;· 🔴 UNA SEGUNDA CAUSA REFUTADA, Y ERA LA MIA — la talanquera no puede ser |
| &nbsp;&nbsp;&nbsp;&nbsp;· La causa que SI sostiene el cobre — y es de diseno, no de esta tarjeta |
| &nbsp;&nbsp;&nbsp;&nbsp;· 🟢 MEDIDO EL 04/09: HAY CORTO ENTRE 3,3 V Y GND. La hipotesis deja de serlo |
| &nbsp;&nbsp;&nbsp;&nbsp;· 🔴 Y EL COBRE DA UN CANDIDATO QUE ENCAJA CON EL GESTO DEL PASO 29 |
| &nbsp;&nbsp;&nbsp;&nbsp;· La escalera, de lo gratis a lo caro — y no se salta ningun peldano |
| &nbsp;&nbsp;· 6.2 · Las entradas de campo |
| &nbsp;&nbsp;· 🔴 N-120 — La placa protege todas sus salidas y ninguna de sus entradas. Va a la V2, y antes de cablear camara |
| &nbsp;&nbsp;· 6.3 · El unico defecto de calle con arreglo escrito y sin subir |
| &nbsp;&nbsp;· 🔴 N-108 — El enlace no deja rastro de como se cayo, y el umbral que lo arreglaba ~~lleva un mes sin subir a campo~~ solo ha tocado una calle dentro de una V9 sin banco |
| &nbsp;&nbsp;&nbsp;&nbsp;· Los 12 s: ya esta arreglado, y ese es el problema |
| &nbsp;&nbsp;&nbsp;&nbsp;· Y el "por nada" es LITERAL — el equipo se rendia antes de terminar de intentarlo |
| &nbsp;&nbsp;&nbsp;&nbsp;· El rastro de la caida: la mitad existe y la otra esta inventada |
| &nbsp;&nbsp;&nbsp;&nbsp;· La decision, tomada el 31/08 |
| &nbsp;&nbsp;· 6.4 · El arranque del ESP32 |
| &nbsp;&nbsp;· 🔴 N-117 — El perro del ESP32 se comia su propio arranque, y el pack lo aprobaba mirando la forma |
| &nbsp;&nbsp;&nbsp;&nbsp;· El defecto |
| &nbsp;&nbsp;&nbsp;&nbsp;· Por que salia verde: el pack medía la forma, no la propiedad |
| &nbsp;&nbsp;&nbsp;&nbsp;· El arreglo, y lo que NO debilita |
| &nbsp;&nbsp;&nbsp;&nbsp;· 🟠 Y HAY UNA SEGUNDA CAUSA CANDIDATA, MAS BARATA DE COMPROBAR, que esta revision no habia visto |
| &nbsp;&nbsp;&nbsp;&nbsp;· 🔴 Lo que falta, y no es codigo |
| &nbsp;&nbsp;· 6.5 · Si el ESP32 se cuelga |
| &nbsp;&nbsp;· 🟠 N-113 — Si el ESP32 se cuelga: que sigue funcionando, que NO, y por que la app no es un canal de alarma |
| &nbsp;&nbsp;&nbsp;&nbsp;· Por que la app NO puede ser el canal de alarma |
| &nbsp;&nbsp;&nbsp;&nbsp;· Lo que si es barato y honesto, hoy |
| &nbsp;&nbsp;&nbsp;&nbsp;· Lo que cuesta de verdad un aviso remoto, para que se decida con el precio delante |
| &nbsp;&nbsp;· 6.6 · Las dos barreras de la app |
| &nbsp;&nbsp;· 🟠 N-110 — Dos barreras de la app que no vigila nadie, salidas de invertir el arnes de DOM |
| &nbsp;&nbsp;&nbsp;&nbsp;· 1. La app **no valida el checksum** de lo que pinta |
| &nbsp;&nbsp;&nbsp;&nbsp;· 2. El teclado del PIN **acepta pulsaciones con el modal cerrado** |
| &nbsp;&nbsp;&nbsp;&nbsp;· 3. Menor |
| &nbsp;&nbsp;· 6.7 · Las camaras no hacen nada en los modos que se usan — y el modo que las usa mete el ruido que se temia |
| &nbsp;&nbsp;· El censo, en una linea |
| &nbsp;&nbsp;· ~~🔴 Y EL MODO QUE LAS USA VIOLA EL MINIMO DE 3 MINUTOS, EN EL FUENTE~~ |
| &nbsp;&nbsp;· El Modo Inteligente es obra DECLARADA, no ejercida |
| &nbsp;&nbsp;· La recomendacion, y el coste MEDIDO compilando |
| &nbsp;&nbsp;· Y tres cosas mas que aparecieron |
| &nbsp;&nbsp;· 6.8 · `SFTY-29` · las camaras como veto — declarada y SIN CONSTRUIR |
| &nbsp;&nbsp;· Lo que la talanquera hace HOY, medido |
| &nbsp;&nbsp;· 🔴 LAS DOS CAMARAS NO SON EL MISMO PROBLEMA: FALLAN EN DIRECCIONES OPUESTAS |
| &nbsp;&nbsp;· Tres consecuencias tecnicas que condicionan como se configura |
| &nbsp;&nbsp;· 🟡 LO QUE SIGUE SIN DECIDIR, y es del responsable |
| &nbsp;&nbsp;· 6.9 · 🎯 DECISION: DOS PANTALLAS, PORQUE SON DOS COSAS (04/09, noche) |
| 7. Los `N-x` cerrados — el puntero de una linea |
| 8. La arquitectura vigente, y POR QUE es esta |
| &nbsp;&nbsp;· El mapa de pines, MEDIDO contra `pines.h` y contra `src/` |
| 9. La V2 — lo que ya esta identificado y NO se ha hecho |
| &nbsp;&nbsp;&nbsp;&nbsp;· A · Que el equipo cuente lo que le pasa (lo que hoy NO existe) |
| &nbsp;&nbsp;&nbsp;&nbsp;· B · Barreras que hoy no vigila nadie |
| &nbsp;&nbsp;&nbsp;&nbsp;· C · El instrumental, solo lo que impide medir |
| 10. Lo anterior, y donde vive |

## Indice de lo mudado el 12/09 - por `N-x`

**Se busca por el simbolo, no por numero de linea** (`CLAUDE.md` §7.3):
`grep -n "N-162" roadmap_hist.md`. Los `N-x` que aparecen en estos seis apartados:

`N-17` · `N-20` · `N-24` · `N-25` · `N-31` · `N-37` · `N-42` · `N-49` · `N-71` · `N-73` · `N-76` · `N-93` · `N-94` · `N-95` · `N-96` · `N-97` · `N-98` · `N-99` · `N-100` · `N-101` · `N-102` · `N-103` · `N-104` · `N-105` · `N-106` · `N-107` · `N-108` · `N-109` · `N-110` · `N-112` · `N-113` · `N-114` · `N-115` · `N-116` · `N-117` · `N-118` · `N-119` · `N-120` · `N-121` · `N-122` · `N-123` · `N-124` · `N-125` · `N-126` · `N-127` · `N-130` · `N-131` · `N-133` · `N-134` · `N-135` · `N-137` · `N-138` · `N-139` · `N-140` · `N-141` · `N-142` · `N-144` · `N-145` · `N-146` · `N-147` · `N-148` · `N-149` · `N-150` · `N-151` · `N-152` · `N-154` · `N-155` · `N-156` · `N-157` · `N-158` · `N-159` · `N-160` · `N-161` · `N-162`

---
## 3. Lo que se puede hacer HOY desde el PC

### 3.1 · 🔴 Defectos abiertos con arreglo conocido

| | que | medida |
|---|---|---|
| ~~**1**~~ | ~~🔴 **El `$ALARM` no cabe en su buffer.**~~ — 🟢 **CERRADO el 08/09. Ver §3.13.** El peor caso por buffer era **158 B en el Maestro y 171 en el Esclavo** contra los 143 que guardaba `payload[144]` | ⚠️ **Y la descripcion que habia aqui del sintoma era FALSA en su mitad mas importante**, medida al arreglarlo: *«no casa el CRC y la app la tira entera»*. El CRC se calcula **sobre lo que quedo**, asi que **casa**. Lo que llega es una alarma con aspecto de intacta **sin el valor de HORA**, que es peor: una que la app tira se nota; esta no |
| ~~**2**~~ | 🟢 **`CAM_CIEGA` CERRADO ENTERO el 08/09 — las DOS mitades.** El plazo (`86400000UL`, 24 h de paso abierto, `9550c57`) **y el aviso** del Modo Inteligente operando con camara averiada. `D-24` en `DECISIONES.md` lleva las dos cuentas | ✅ **El aviso salio CERO FIRMWARE**: la parte vial ya estaba construida —el modo degrada solo al Automatico, escrito en `modo_inteligente.cpp`— y el dato ya viajaba en `CAM:`. Faltaba **cruzarlos**, y eso vive en la app. **La lista de alcance que habia aqui se quedaba corta en TRES sitios** — §3.15 |
| **3** | 🔴 **En la SUBIDA no hay checksum.** `calcularChecksum()` es `static` y **su unico llamador en cada punta es `enviarTramaConCrc()`**; `procesarComando()` no lee el `*XX`. Un bit cambiado dentro de `SET_TIEMPOS` o `SET_RTC` **se obedece** | 🔴 **sigue, medido el 11/09 sobre `b79d904`**: `calcularChecksum()` solo aparece en `enviarTramaConCrc()` en las dos puntas. §0, 1.6 · *(`68dd2c5`: el `SET_RTC` ya no llega al STM32; lo que llega en su lugar por `J17`, `CMD:HORA_ESP32:`, **tampoco** lleva checksum — lo acota `isoBienFormado()` en los dos `reloj.cpp`, que exige la forma exacta `0000-00-00,00:00:00` y nada detras: un digito cambiado por otro digito pasa)* |
| ~~**4**~~ | ~~🟠 **`validateTiempos()` de los unitarios de la app sigue en 1..15 min**~~ — **CADUCADA, medida el 08/09**: `test_unitarios_app.js` esta en `v < 3` / `r < 3`, y el caso de verde es `validateTiempos(2, ...)`, que **si** toca el borde. Se corrigio en `31170e8` (07/09) y esta fila se quedo describiendo el estado anterior | 🟠 **Queda un residual, y es la misma forma en pequeno:** el caso de rojo es `validateTiempos(3, 1, 25)`, o sea **uno por debajo del borde**. Si alguien afloja `r < 3` a `r < 2`, el caso de verde cae y **el de rojo sigue pasando**: mide el rechazo, no el limite. El borde de rojo es `2` · *sigue el 11/09: el caso de rojo de `test_unitarios_app.js` es `validateTiempos(3, 1, 25)`* |
| **5** | 🟠 **El Esclavo no tiene `reloj_diagnostico()`.** Porte **mecanico** desde el Maestro; ya tiene los ingredientes. Sin el, el tecnico que sube 5 m al poste del Esclavo **no puede distinguir `lseOn=0` de `lseRdy=0`** | 🔴 **sigue, 11/09**: `reloj_diagnostico` solo existe en `Maestro/src/reloj.cpp`. §0, 1.7 |
| **6** | 🟠 **`state.correctPin = '1234'` en claro** en `app.js`. La caducidad **si** se construyo | V2 · `B3` · *sigue el 11/09 (`correctPin: '1234'` en `www/app.js`)* |
| ~~**7**~~ | 🟢 **CERRADA el 08/09 en `6c90ff0`**, medido el 11/09: la rama `SET_RTC:` del Maestro escribe `SET_RTC_RECHAZADO_POR_RANGO` dentro del `if (reloj_sembrarDesdeIso(...))`, igual que el Esclavo. ~~El Diario de Ordenes del MAESTRO no distingue si la hora entro. `Maestro/src/bluetooth.cpp`, rama `SET_RTC:`: `bluetooth_reportarEvento("APP_BLUETOOTH", "SET_RTC_LO_ACUSA_EL_PUENTE")` esta **FUERA** del `if (reloj_sembrarDesdeIso(...))`. Es el residual de `N-160` una capa arriba — ver §3.10.ter~~ | ~~**El Esclavo YA lo arreglo** y su propio comentario describe el defecto que el Maestro conserva. Arreglar una punta y no la otra es lo que `CLAUDE.md` §6.1 manda mirar, y aqui paso en el sentido contrario al esperado~~ *(11/09: ~~hoy el Maestro tiene las dos ramas, `SET_RTC_LO_ACUSA_EL_PUENTE` dentro del `if` y `SET_RTC_RECHAZADO_POR_RANGO` en el `else`~~, y `reloj_02_siembra_que_miente` mide la simetria en su bloque 5)* *(🔵 **11/09 por la tarde, `68dd2c5`: la rama `SET_RTC:` ya no existe en ninguna de las dos puntas** —el `SET_RTC` es del puente, `D-26` (1)—, y los dos literales tampoco: `grep` en `{Maestro,Esclavo}/src` → 0 lineas de codigo. La rama que la sustituye, `CMD:HORA_ESP32:`, escribe `HORA_ESP32_SEMBRADA` dentro del `if (reloj_sembrarDesdeIso(...))` y en el `else` levanta `$ALARM …EVENTO:HORA_ESP32,CAUSA:RECHAZADA_FORMATO…`; `reloj_02` sigue mirando la simetria sobre la rama nueva)* |
| ~~**8**~~ | ~~🔴 **`AMBAR_EMERGENCIA` sin PIN no avisa al Maestro** — solo la puerta CON PIN del Esclavo manda `CMD_AMBAR_ESCLAVO`, y la app usa la de sin PIN.~~ 🟢 **CERRADO EN EL FUENTE el 12/09 (`913c29c`, §0 fila 1.2): las dos puertas avisan** —dos llamadas donde habia una— **y el `$ACK` distingue si el aviso pudo oirse.** §3.16-A | 🔴 **Y ESO NO LO CONVIERTE EN LA CAUSA DEL DAR PASO DEL SISGA: sigue siendo un CANDIDATO, y ahora uno que ya no se puede reproducir con el firmware nuevo.** Lo que se cerro es el defecto; lo que confirmaria la causa es la **cinta del Esclavo**, que sigue sin traerse (§0, 3.1) — sin ella nadie sabe si aquel dia alguien pidio el ambar desde el telefono. ⚠️ **Y queda una via viva por la que este mismo aviso se pierde**, medida en el bloque H4 del arnes: si muere solo el transmisor del Esclavo, la trama no sale, esa punta no puede saberlo y el `$ACK` sale igual que con la radio sana |
| ~~**9**~~ | ~~🔴 **`SET_RTC` con `$ERR` del puente y hora sembrada igual en el STM32**. §3.16-C~~ 🟢 **CERRADA EN EL FUENTE en `68dd2c5`** (11/09) | ⚠️ sin reproducir a mano · ~~🟠 **EN CONSTRUCCION el 11/09**: la regla 2 de «el ESP32 manda la hora» (§3.16) lo cierra; en `main` sigue~~ → **medido sobre `68dd2c5`:** el `SET_RTC` ya no cruza —`despachador_esParaElPuente()` lo reclama y `puente.cpp` solo llama a `enlace_escribirLinea()` con lo que NO reclama— y lo que llega al STM32 es `CMD:HORA_ESP32:` compuesto por `siembra_ahora()`, que solo corre en la rama `RELOJ_OK` del `SET_RTC`, **despues** de poner y releer el `DS3231`. Una hora que el `DS3231` rechazo ya no llega al STM32. ⚠️ Sin tarjeta. §0, 1.1 |

### 3.2 · 🟢 `DECISIONES.md` — REVALIDADO ENTERO EL 08/09, ~~y ya no queda ninguna fila caducada~~ y el 11/09 quedaban OCHO textos caducados

> 🔴 **FALSO el titulo, medido el 11/09: quedaban ocho textos descriptivos caducados**, y se tacharon
> en `DECISIONES.md` con su evidencia al lado —**sin cambiar ninguna decision ni ningun estado de
> fila**—: `D-23` (*«hay que ELEGIR LA VIA → `A-14`»*, con `A-14` resuelta el 07/09); `D-22` (*«con
> `D-20` construida el `DS3231` siembra cada 2 s»*: ~~la siembra no existe en `main`~~ y `A-15` la fijo en
> una hora, que `D-26` corrigio a ~5 min *(la siembra existe desde `68dd2c5`, 11/09 por la tarde)*); `D-21` (*«`D-20` cierra la pieza (A)»*: el `OSF` no llega a `reloj_enHora()`); `D-14`
> (*«hasta que la medida de multimetro desbloquee»*, tras retirarse esa medida el 08/09, mas un
> fragmento del texto retirado que se quedo sin tachar); `D-13` (*«`CAM:` no cabe… sin llamador»*, en
> su tabla de publicacion y en su fase 4); **«Filas que chocan»** (*«los otros dos siguen vivos»*: los
> tres estan resueltos) y el aviso de cabecera que apuntaba a ellas; `A-13` (*«hay un agente
> midiendolo ahora»*); y `A-7` (*«La cura: medir el `Delay`»*, con `A-7` cerrada sin medir). De paso,
> `A-5` y `A-6` en las cerradas (`0x68` y la dependencia de `A-13`). **Lo que NO se toco y es texto
> de una decision:** la premisa de `D-21` sobre el reenvio periodico al Maestro (§0, fila 2.7).

**Un encargo lanzado leyendo esa tabla arranca sobre un hecho falso**, que es justo lo que aquel
fichero existe para impedir. Medido el 07/09:

| fila | lo que dice | lo que la caduca |
|---|---|---|
| **A-12** | *«la compuerta esta en ROJO por esto»*, `19 PASS · 1 FALLA` | `62d731e`. El acta del 07/09 da `20 PASS · 0 FALLA · 0 ABORTADO` |
| **A-13** | *«no caben sus 11 caracteres en ninguna de las dos puntas»* | `e3a21ec`: `payload[155]` y `,CAM:%s` en las dos |
| **A-13**, coletilla | *«`camara_estado()` esta declarada y SIN NINGUN LLAMADOR»* | se llama dentro del `snprintf` del `$STATUS` en las dos puntas |
| **A-11**, cuerpo | *«`grep -c "SET_MODO" Esclavo/src/bluetooth.cpp` -> 0»* | hoy da **10** (`15e8cf3`). **El indice ya la tacha y el cuerpo abierto contradice al indice** |
| **A-2** | abierta, urgencia media | **cerrada por el responsable el 05/09** |
| **«Filas que chocan» 1, 2 y 3** | las tres | caducadas por `62d731e`, `15e8cf3` y `e3a21ec` |
| ~~**`A-14`, cuerpo**~~ | ~~*«Que esta bloqueado mientras tanto: **`D-23` entera**»* bajo un indice que ya la tachaba~~ | 🟢 **CORREGIDA el 08/09.** Era `A-11` letra por letra. El cuerpo lleva ahora la decision —**`$EVENT` nuevo**—, la medida que tumba las otras dos vias, y **la condicion nueva que la medida anadio** |

> 🔴 **REVALIDADA ENTERA EL 08/09, Y LA PROPIA LISTA ESTABA CADUCADA: de las cinco filas de arriba,
> TRES YA SE HABIAN CORREGIDO** —`A-12` ya no cita cifra de compuerta, `A-13` ya no dice que los 11
> caracteres no caben, y el cuerpo de `A-11` ya no publica el `grep` que daba cero—. **Una lista de
> defectos documentales envejece igual que los defectos que denuncia**, y esta llevaba un dia
> acusando trabajo ya hecho. Medido con `grep` contra el fuente, no releyendo.
>
> Lo que quedaba vivo eran **dos**, y las dos se cierran el 08/09: la coletilla de `A-13`
> —*«`camara_estado()` esta declarada y SIN NINGUN LLAMADOR»*, cuando tiene **una llamada real por
> punta** dentro del `snprintf` del `$STATUS`— y el cuerpo de `A-14`.
>
> ⚠️ **Y la coletilla de `A-13` casi se corrige con un dato malo:** el `grep` de `camara_estado()`
> sobre los dos `bluetooth.cpp` da **7**, y **cinco son comentarios y `#include`**. *«Un cero de
> `grep` no es no hay»* vale igual del otro lado: **un siete de `grep` no es siete** (`CLAUDE.md`
> §7.1). Se cuenta excluyendo comentarios o no se cuenta.

> 🎯 **CONSECUENCIA DE PLANIFICACION, y es la que importa: `D-23` YA NO ESTA BLOQUEADA.**
> ~~`ESTADO.md` sigue diciendo~~ `ESTADO.md` **y la propia fila `D-23`** decian *«antes de construirla hay que ELEGIR LA VIA, que no esta elegida:
> `A-14`»* — **esa frase caduco el 07/09** *(tachada en los dos el 11/09)*. Lo que falta es construirla: un `$EVENT` nuevo en el
> Esclavo, **emitido tambien AL CONECTAR**, y su pantalla en la app — o sea **recompilar la APK**,
> con todo lo que eso arrastra (skill `entregar` §2.bis).

### 3.3 · 🔴 La evidencia del banco NO esta donde 8 documentos dicen

**Es la unica prueba fisica del proyecto, y las citas estan rotas.** Comprobado con dos patrones
—`find -iname "*Informe*"` y `git ls-files | grep -i informe`—, que es lo que `CLAUDE.md` ~~§4~~ §7.1
*(la numeracion de hoy)* exige antes de publicar un «no hay»:

- `evidencia/Informe_Pruebas_Banco_Semaforos_V9.0.pdf` — **14 citas en 8 ficheros**. El fichero vive
  en **`evidencia/old/`**: **las 14 citas estan rotas**.
- `evidencia/Informe_Pruebas_Banco_Semaforos_Sesion2.pdf` (`N-126`) — **NO EXISTE en el repositorio.**
- **La cinta de tramas del 05/09 tampoco esta** (`find -iname "*cinta*"` -> cero). *(11/09: sigue sin
  estar; la unica cinta del repositorio es la del Sisga del 10/09, `evidencia/2026-09-10_Sisga_179DB0_*`.)*

> 🔴 **Los cierres en cobre de `N-142`, `N-145`, `N-146` y `N-149` existen SOLO como lineas
> transcritas a mano en dos `.md`.** Es la regla del instrumento aplicada al propio roadmap: **las
> unicas veces que este fichero se apoya en una prueba fisica, el ancla no esta.**

### 3.4 · 🟠 Arreglos documentales que hacen dano si no se hacen

1. ~~**La guia de banco le pide al responsable que COMPRE el mando que `D-1` retiro.** Su pregunta
   abierta 3 dice *«la compra ya no es pregunta: receptor con salida NO … ¿se pide ya?»*, y `D-1` y
   `17_` §4 dicen *«ya no se va a comprar»*. **Es una pregunta que no va** (`CLAUDE.md` §2.quater).~~
   🟢 **Corregido el 07/09 en `1185441`**: la guia dice ya *«no hay mando que probar, no hay receptor
   que comprar»*, y `grep -i "se pide ya"` sobre `Guia_Cableado_y_Pruebas_Banco.html` da **0**.
2. ~~**`README.md` numera `p10 = Camara 2` / `p12 = Camara 1`** contra la decision escrita de `17_`
   §1.7.~~ 🟢 **Corregido el 11/09**: el README dice ya `CAM_C`/`CAM_D`, ~~p12 vacio~~ *(y desde
   `648b62f`, con `D-25`, dos camaras por poste: p9/p10 y p11/p12)*. ⚠️ **Y queda un
   tercer reparto** —`05_Funcional/README.md`, *«p10 = poste 1, p12 = poste 2»*— que tampoco casa con la spec.
3. ~~**`N-118` sigue publicado como defecto abierto en `ESTADO.md`, `README.md` y 16 documentos.**~~
   Esta **REFUTADO** desde el 05/09 (`d020f3c`): los `0,6 V` eran el firmware viejo. *(11/09:
   `ESTADO.md` y `README.md` lo presentan ya como REFUTADO —tachado en el uno, «quedo REFUTADO» en el
   otro—. El «16 documentos» se retira: no se reproduce; hoy `grep -l N-118` da 13 ficheros en
   `05_Funcional/` y `04_Manuales/`, y los que se abrieron lo dan cerrado o refutado, **sin haberlos
   leido los 13 linea a linea**.)*
4. **Tres cifras de flash del Maestro vivas a la vez** —`88,3 %`, `89,3 %`, `86,3 %`— fuera del acta.
   **Solo el acta vale**; las otras se copiaron a mano y envejecieron por separado. *(11/09: `88,3 %`
   ya no aparece; `89,3 %` y `86,3 %` sobreviven en `17_` y en `6_` como cita fechada o tachada; y la
   cifra VIVA desfasada estaba en `ESTADO.md` —*«el Maestro va al 88.6 % y quedan 7.448 B»*—, tachada
   hoy con puntero al acta.)*

> ⚠️ **Y antes de copiar ninguna cifra de flash a ningun documento, una pasada completa con el arbol
> QUIETO.** El acta del 07/09 avisa ella misma: *«el arbol tenia cambios sin commitear al medir»*.
> `CLAUDE.md` ~~§7~~ §10 *(la numeracion de hoy)*: **una cifra correcta de un binario que quiza ya no existe.**

### 3.4.bis · 🔴 `D-20` — el reloj se muda del STM32 al ESP32. DECIDIDO el 07/09, ~~sin construir~~ CONSTRUIDO EN PARTE

> 🟢 **11/09: el titulo caduco el mismo 07/09.** El nucleo de `D-20` esta construido en `9dd8bbf` —el
> sembrador `reloj_sembrarDesdeIso()` con llamadores reales en las dos puntas y el extrapolador
> `segBaseDelDia + (millis() - tBaseMillis) / 1000`—, con `N-160` cerrado encima y los getters de
> `c51cc85` mirando primero la base sembrada. **Lo que falta de esta tabla:** ~~la siembra periodica
> ESP32→STM32 (`A-15`, una hora → ~5 min por `D-26`), **EN CONSTRUCCION el 11/09 en un worktree y fuera de `main`**
> (`grep INTERVALO_SYNC_MS ESP32_Expansion/src` → 0);~~ *(🟢 **construida y en `main` desde `68dd2c5`**, 11/09 por la
> tarde: la hora sale de `siembra.cpp` —`siembra_revisar()` en el `loop()` del ESP32, cadencia
> `SIEMBRA_INTERVALO_MS = 300000UL` en `contrato.h`, que a proposito ya no comparte nombre con `INTERVALO_SYNC_MS`—
> y el STM32 la recibe por la rama `CMD:HORA_ESP32:` de su `bluetooth.cpp`; §0 fila 1.1)* la reanudacion
> diferida; y la retirada de la fila 1 (ver su nota). §0, fila 1.5.

> **La autoridad de la hora es el ESP32, siempre y para todo. Al STM32 no se le pregunta nunca.**
> La app se la da al **ESP32 Maestro**, ese al **ESP32 Esclavo**, y el STM32 de cada punta la recibe
> **de su propio ESP32**. Si las dos puntas se desincronizan, **manda la del ESP32 Maestro**.

**El motivo es de hardware.** `reloj.cpp` de las dos puntas es `STM32RTC` sobre `LSE_CLOCK`, o sea
**el cristal `Y2`, confirmado muerto (N-17)**. Cada ESP32 ya lleva su `DS3231` con pila y
**funcionando**. El propio fuente ya avisaba de lo que pasa si se finge lo contrario: *«escribir la
hora sobre un contador parado la deja visible pero sin avanzar, y `horaValida` en `true` seria una
mentira sobre la que el Modo Degradado se autorizaria»*.

> 🔴 **AQUI DECIA «el STM32 no tiene ni pila ni cristal». ES FALSO: TIENE LAS DOS Y NO USA NINGUNA.**
> Corregido el 07/09 por el arquitecto, midiendo el variant: `Y1` de 8 MHz esta en la placa y el
> firmware arranca con el **HSI**, el RC interno (`generic_clock.c`, `RCC_PLLSOURCE_HSI_DIV2`, sin
> `HSE` en ningun `.ini` ni `.cpp`). Y `VBAT` midio **3 V con la tarjeta apagada** (N-37): al menos
> una tarjeta lleva su `CR2032` puesta —la otra sigue `SIN VERIFICAR`—.
>
> **Y eso cambia el diseno, no solo el dato:** el HSI es **±1 % a 25 °C y ~±2,5 % con temperatura**,
> o sea **`millis()` va a 10.000–25.000 ppm**. Un reloj sembrado **una vez por hora se iria 36 s en
> esa hora — mas que el margen ENTERO de 29 s del cruce.** La version que estaba escrita aqui
> —*«reloj de software sembrado y refrescado cada tanto»*— **era peor que el cristal muerto**, y el
> cristal muerto al menos se declara.

**Lo que hay que construir, medido el 07/09:**

| | que | nota |
|---|---|---|
| **1** | `reloj.cpp` de las dos puntas pasa a **EXTRAPOLADOR sembrado cada ~~`LATIDO_MS` (2 s)~~ ~~UNA HORA~~ ~5 MIN** *(~~`A-15`, 08/09~~ `D-26`, 11/09; ~~🔴 **la siembra periodica sigue SIN CONSTRUIR al 11/09**, §3.16-B~~ → 🟢 **construida en `68dd2c5`**, 11/09 por la tarde: `siembra.cpp` del ESP32 y la rama `CMD:HORA_ESP32:` de las dos puntas. ⚠️ **Lo que NO se construyo de esta fila:** `reloj_enHora()` **no** pasa a significar «mi siembra es fresca» —ninguna punta declara vieja su hora; es `H1`, §0 fila 1.13—)* con un `EPOCH` del `DS3231` — **no un reloj de software refrescado «cada tanto»**: con el HSI a 10.000–25.000 ppm, la frescura de la siembra ES el presupuesto de error. `reloj_enHora()` pasa a significar **«mi siembra es fresca»**. Se retiran `STM32RTC`, N-25, N-31, `reloj_ajustar()` y el truco de «enero» · 🔴 **11/09: en `main` SIGUEN LOS CINCO** — `#include <STM32RTC.h>` y `rtc.setClockSource(STM32RTC::LSE_CLOCK)` en los dos `reloj.cpp`; el reintento del cristal de `N-25` en el `loop()` de los dos `main.cpp`; `reloj_reiniciarDominioRespaldo()` y `REINICIAR_RELOJ` de `N-31` en el Maestro; `reloj_ajustar()` como envoltorio de `reloj_ajustarConAcuse()`; y `reloj_fijarEnero()`/`rtc.setMonth(1)`. `D-20` se construyo **encima** del RTC, no en su lugar | toca **SFTY-18 y SFTY-23**. La desigualdad `SIEMBRA_CADUCA_MS x HSI_PPM + cadena + deriva48h < despeje - ambar` **va en un pack**, no en un comentario (N-71) |
| **2** | Un mando nuevo **ESP32 -> STM32** que siembre la hora. **El camino fisico ya existe**: `enlace_stm32.cpp` escribe hacia el STM32 | no hace falta hardware |
| **3** | Las **48 h** de rendicion. ~~Salen del contador crudo del RTC, monotono y superviviente al apagado, y se perderian en el primer corte~~ — 🔴 **CORREGIDO el 07/09: YA ESTAN MUERTAS.** `reloj_contadorSegundos()` abre con `if (!rtcOperativo) return 0;`, asi que la pila mantiene los `BKP` **pero lo que guardan es un contador PARADO**. No es un riesgo futuro: es un limite que **hoy no cuenta** | **Los registros `BKP` NO se mueven** y `respaldo.cpp` **no se toca** —la resta de epochs con guarda de retroceso ya esta escrita—. Lo que hay que darle es un contador que avance |

**Topologia, y conviene decirla en voz alta: los dos ESP32 NO SE HABLAN.** El unico enlace entre
postes es la radio **entre los STM32**, asi que la hora viaja
`ESP32-M -> STM32-M -> radio -> STM32-E -> ESP32-E`. **Los STM32 quedan de carteros de la hora, no
de duenos** — que es justo lo decidido.

**El presupuesto de desfase, derivado del fuente por el arquitecto** —y es lo que decide si esto es
seguro—: con **la radio como UNICA via** de sincronizacion entre postes, `0–1 s` truncado en la
siembra + `0–1 s` truncado en la radio + `0,5 s` de aire + **`1,2 s` de deriva `DS3231`-`DS3231` en
48 h** (7 ppm relativos) **≈ 2,8 s contra los 29 s que aguanta el cruce: factor ≈10**, frente al
**1,44** de hoy. `TOLERANCIA_DESFASE_S = 3` **ya es la cifra correcta**.

> ✅ **Y la condicion que lo sostiene YA ESTA DECIDIDA (07/09): EL MAESTRO MANDA LA HORA Y EL ESCLAVO
> HACE CASO SIEMPRE.** El arquitecto pedia una regla —*«un `SET_RTC` de la app no es una
> sincronizacion»*— porque mientras la app pueda visitar los dos postes por separado, el desfase
> inicial **no tiene cota**. El responsable lo cerro por **autoridad** en vez de por aritmetica:
> **una sola fuente, luego no hay desfase inicial que acotar.**
>
> ~~**Lo que eso obliga en el codigo: la app NO pone la hora en el poste 2, nunca.** El puente del
> Esclavo tiene que **rechazar** un `SET_RTC` dirigido a el —hoy lo atiende— porque no es una
> sincronizacion: **es una segunda fuente**.~~ 🔴 **DEROGADO en la propia fila `D-20` el 07/09 por la noche**
> —*«no hace falta prohibir nada: la barrera es la SOBREESCRITURA»*— **y convertido en procedimiento por
> `D-21` (3) y `D-26` (3)/(5), 11/09:** con radio, lo que se le ponga al poste 2 lo pisa la radio
> (el STM32 Esclavo ignora su ESP32 mientras `reloj_radioManda()`); **SIN radio, la del ESP32 del poste 2
> ENTRA, y ponersela desde el telefono en su gabinete es lo que se hace cuando salta la alarma de radio.**
> ~~Cierra de paso el **rejuvenecimiento** de las 48 h, que se hacia poniendo ese `DS3231` hacia atras.~~
> *(La hora del ESP32 del Esclavo no renueva las 48 h: su rama `CMD:HORA_ESP32:` no llama a
> `degradado_registrarSync()` ni a `respaldo_marcarSync()`, medido en `68dd2c5`.)*
>
> ~~⚠️ **Y la consecuencia que hay que saber, porque parece un problema y no lo es:** si el unico
> camino hacia el reloj del Esclavo pasa por el Maestro, **con la radio muerta no se le puede poner
> en hora** — y eso es justo cuando se necesita el Degradado. **No importa: su `DS3231` tiene pila y
> conserva la hora que ya tenia.** Perder la radio no es perder la hora. Lo que si obliga es a que
> **el poste 2 se ponga en hora ANTES**, en la puesta en marcha, no durante la averia.~~ *(Caducado por
> `D-26` (3): con la radio muerta el poste 2 **si** se pone en hora, desde el telefono en su gabinete. Ponerle
> hora a los dos `DS3231` en la puesta en marcha sigue siendo bueno —el salto al perder la radio es lo que
> difieran—, pero ya no es la unica ocasion.)*

> ✅ **LAS DOS PREGUNTAS DE IMPLEMENTACION, CONTESTADAS POR EL RESPONSABLE EL 07/09 — y una de las
> dos estaba MAL PLANTEADA por mi.**
>
> **1. ~~¿Primero el camino o primero la barrera?~~ LA PREGUNTA NO IBA: EL CAMINO YA EXISTE.** Se
> contesto *«¿esto no se hace al inicio, sincronizar Maestro y sincronizar Esclavo, en la app?»* —
> y es exactamente lo que hay. Medido en la app: existe **`btn-sync-rtc` («Sincronizar»)** y
> **`btn-leer-rtc`**, y actuan sobre **el poste al que estas conectado**. O sea que el operario ya
> pone en hora cada punta visitandola. **No hay ningun orden que decidir y ningun riesgo de dejar al
> Esclavo sin hora.**
>
> 🔴 **Lo que falta no es el camino: ES LA PROPAGACION, y el propio equipo lo dice en su acuse.**
> Medido en `ESP32_Expansion/src/despachador.cpp`: la respuesta a poner la hora es
> **`HORA_PUESTA_SIN_PROPAGAR`** — *«entro en el `DS3231` de ESTE poste»* y **no llega al otro**.
> Ese es el segundo origen que preocupaba al arquitecto, y esta escrito en el `$ACK` desde antes.
>
> **2. ¿Siembra el STM32-E su reloj con la trama, o solo la reenvia?** El responsable no eligio
> implementacion, puso el **requisito**, y es mas fuerte que cualquiera de las dos:
>
> > **«deben estar con la misma hora para poder trabajar; no pueden estar desincronizados»**
>
> **Y ese requisito disuelve el problema del desfase inicial sin necesidad de prohibir nada.** El
> temor era que el operario pusiera el Maestro a las 10:00:00 y el Esclavo a las 10:02:30 —**150 s
> contra un margen de 29 s**—. La cura no es quitarle el boton: es que **el Maestro EMPUJE la hora
> al Esclavo y el Esclavo la acepte SOBRESCRIBIENDO** lo que tuviera. Entonces **da igual lo que
> hiciera el operario y en que orden**: el desfase deja de depender de su reloj de muneca y pasa a
> depender de la cadena, que esta acotada en **~2,8 s contra 29**.
>
> **Consecuencia practica, y es lo que hay que construir:** sincronizar el Esclavo desde la app deja
> de ser peligroso —queda **inutil**, porque el Maestro lo sobrescribe—. **No hace falta barrera: la
> barrera es la sobreescritura.**

> 🔴 **Dos cosas mas que NO estaban en el plan y sin las cuales rompe:**
>
> 1. **`reanudarTrasCorte()` se llama UNA vez en `setup()` y BORRA el indicador si no hay hora.** Con
>    siembra externa, en `setup()` **nunca la hay** (el arranque del ESP32 esta declarado en 1500 ms
>    y **sin medir**), asi que **N-20 moriria en silencio**. Hay que diferir la reanudacion a la
>    primera siembra, con espera acotada.
> 2. **Las 48 h NO se mueven:** se quedan en `BKP->DR5/DR6` con `respaldo.cpp` **sin tocar** —la
>    resta de epochs con guarda de retroceso ya esta escrita—. Lo que hay que cerrar es el
>    **rejuvenecimiento**: hoy se puede poner el `DS3231` hacia atras justo despues de la marca, y
>    lo cierra **la misma regla del `SET_RTC`**. Precondicion: `CR2032` en **las dos** tarjetas.

> ~~🔴 **Y esto es lo que no puede faltar al planificar la sesion de banco: MIENTRAS `D-20` no este
> construida, el Modo Degradado del poste 2 NO SE PUEDE PROBAR.** Su guarda de entrada abre con
> `if (!reloj_enHora()) return DEG_RECHAZO_SIN_HORA;`, y en esa punta esa bandera es **falsa
> siempre**. O sea: `D-18` esta construida —el comando existe y llega— **y el equipo va a contestar
> que no, correctamente, todas las veces**. Quien lo pruebe sin saber esto lo anotara como defecto.~~
> 🟢 **CADUCO desde `9dd8bbf` (07/09), medido el 11/09:** en `Esclavo/src/reloj.cpp`,
> `reloj_ajustarConAcuse()` pone `horaValida = true` sin `Y2`, y la llaman el `SET_RTC:` del
> `bluetooth.cpp` del Esclavo (via `reloj_sembrarDesdeIso()`) y la hora que llega por radio (via
> `reloj_ajustar()` en `main.cpp`). **El Degradado del poste 2 SE PUEDE PROBAR: antes, hora al
> Esclavo.** Lo que el banco tiene que saber ahora es lo contrario: **una vez dentro, la guarda de
> hora del Esclavo no puede dispararse** (§3.10.bis, pregunta 2).

### 3.4.ter · 🔴 `D-21` y `D-22` — las dos que salieron de preguntar «¿y como lo SABE el Esclavo?»

**`D-21` · Una hora que no es fiable NO es «sin hora»: es una hora que MIENTE, y se responde con
AMBAR INTERMITENTE en la punta que la tiene** — y se publica, para que la app lo ensene.

El escenario lo puso el responsable: **la pila se agota y el reloj queda clavado en una fecha
pasada**. Eso no es quedarse sin hora: es tener una que **daria verdes con toda confianza**. Encaja
con la doctrina del manual —ambar = *«no estoy controlando, decide tu»*, el conductor llega
**alerta**—, y por eso la respuesta es ambar y no rojo.

> 🟢 **La deteccion YA EXISTE y no hay que inventarla:** el `DS3231` levanta su bit **`OSF`** al
> pararse, y `reloj_ds3231.cpp` ya declara *«una hora con `OSF` puesto es NO FIABLE aunque los
> registros traigan valores plausibles»*. **Lo que falta es que esa declaracion llegue a las luces.**

⚠️ **Y no se puede ordenar «ambar en las DOS»: en Degradado no hay radio.** Cada punta decide por su
cuenta. La asimetria que queda es el `Riesgo 2` del manual, **aceptado el 01/08 y sin solucion
tecnica sin radio**.

**`D-22` · `Y1` (8 MHz) pasa a ser el reloj de sistema del STM32.** Esta **montado y hoy no lo usa
nadie**: el firmware arranca con el HSI.

| corriendo libre, sin siembra | error tras 1 hora | tras 48 h |
|---|---|---|
| **HSI** (hoy), 10.000–25.000 ppm | **~36 s** | inservible |
| **`Y1`** como HSE, 20–50 ppm | **~0,1 s** | **~5 s** |

Contra un margen de cruce de **29 s**, convierte *«pierdo la siembra y a los minutos soy
peligroso»* en *«aguanto dias»*. **No cuesta hardware: el cristal ya esta soldado.**

> ⚠️ **PRECONDICION MEDIBLE: `Y1` NUNCA SE HA ARRANCADO** —el firmware jamas lo ha seleccionado— y
> **`Y2`, el otro cristal de esa misma placa, esta muerto**. Si `Y1` no oscila, el arranque **cae al
> HSI y lo DECLARA**: ni se cuelga ni finge precision que no tiene. Y **la salud de cada reloj y
> cada cristal se publica** en bitacora y en la app.

> 🔴 **Y UN AVISO SOBRE EL PROPIO INSTRUMENTO, que hay que mirar antes de que sea tarde:** con
> ~~`D-14`, `D-20`, `D-21` y `D-22`, el banco lleva ya **cuatro decisiones VIGENTES SIN CONSTRUIR** y
> dos sin llegar al manual.~~ *(11/09, corrida de `decisiones_01_anclas` sobre `b79d904`: las vigentes
> sin ancla son **`D-14`, `D-22` y `D-23`** —`D-20` y `D-21` tienen ancla y codigo desde `9dd8bbf` y
> `7adee76`—, **y desde hoy `D-25`**, que ademas no llega a ningun manual: **48/53**.)* Cada rojo es legitimo —**todos se apagan construyendolos**, ninguno es
> de los que ningun firmware puede cerrar—, pero `CLAUDE.md` §1 avisa de lo otro: **un codigo de
> salida que no cambia nunca ensena a ignorarlo**. **La cuenta de «decidido y sin construir» es hoy
> la cifra que hay que vigilar**, y si sigue subiendo el problema ya no es el instrumento: es que se
> esta decidiendo mas rapido de lo que se construye.

### 3.4.quater · 🎯 EL ORDEN DE CONSTRUCCION, del mas aislado al que toca la calzada

**Lo que queda de `D-20`/`D-21`/`D-22` ya no es pregunta: es teclado.** Las tres estan decididas,
con su porque, sus numeros y sus precondiciones. Este es el orden, y **no es cronologico: es por
cuanto se acerca cada paso a las luces.**

> 🔴 **ORDEN CORREGIDO el 07/09 por el responsable, y mi orden anterior no se sostenia.** Yo puse
> `Y1` primero **porque crei que tocaba el margen del cruce. No lo toca.** El reloj del cruce es el
> **`DS3231` TCXO con pila de cada ESP32** (`D-9`), y del STM32 **lo muerto es `Y2`, y solo ese**.
> **`Y1` no es un reloj: es el LATIDO del micro** — no lleva la hora, solo marca a que ritmo ejecuta
> y, de rebote, la precision de `millis()`. ~~Con `D-20` construida el `DS3231` siembra **cada 2 s**,
> y entre siembra y siembra **el error del oscilador interno es despreciable**.~~ 🔴 **FALSO, medido el
> 11/09: ~~esa siembra no existe en `main`~~ *(existe desde `68dd2c5`, 11/09 por la tarde)*, y la cadencia decidida es ~~UNA HORA (`A-15`)~~ ~5 MIN (`D-26`, 11/09; la cuenta que sigue se hizo con la hora)**, con la que
> el HSI corriendo libre da **36–90 s** contra los 29 s del cruce (§3.11, `A-16`). *(Con los 300 s de
> `68dd2c5` son **7,5 s por punta** entre siembras en el peor caso; y con el `J17` mudo no hay siembra y
> vuelven los 36–90 s por hora: `N-162` `H1`, §0 fila 1.13.)*

> ✅ **ESTADO AL 07/09 POR LA NOCHE, tras la tanda de `N-160`: los pasos 1 y 2 ESTAN CONSTRUIDOS —el
> `D-20` de verdad, y de `D-21` la pieza `B`—, y los pasos 3 y 4 SIGUEN SIN EMPEZAR.** `D-14` espera
> ~~una medida de multimetro~~ *(retirada el 08/09, §3.11.bis: espera que se elija el canal, §3.11.ter)*
> y `D-22` espera una tarjeta delante: **ninguno de los dos se destraba
> escribiendo codigo**, y por eso el intento de cerrarlos por comentario no cerro nada. 🔴 **Y lo
> construido no ha visto una tarjeta**: se reescribio **el reloj de las dos puntas** —cientos de
> lineas, mas borrado que anadido— sobre el unico modo que da verde sin confirmar el otro extremo.
> **Eso no se sube: se lleva a banco.**

| | que | toca el ciclo | por que va aqui |
|---|---|---|---|
| **1** | **`D-20` · la siembra y la propagacion.** Extrapolador ~~cada 2 s~~ *(sembrado ~~cada hora, `A-15`~~ cada ~5 min, `D-26`; ~~la siembra periodica EN CONSTRUCCION el 11/09~~ → 🟢 **en `main` desde `68dd2c5`**)*; **el Maestro empuja y el Esclavo SOBRESCRIBE** *(`D-26` (3): **con radio**; sin radio 25 s el Esclavo acepta la de su propio ESP32 —`reloj_radioManda()`—)* | 🔴 **SI** | **es lo que DESBLOQUEA el Degradado del poste 2**, ~~que hoy esta muerto: su guarda abre con `!reloj_enHora()` y esa bandera es falsa siempre~~ *(desbloqueado desde `9dd8bbf`, §5)*. 🟢 **Paso 1 CONSTRUIDO entero en `68dd2c5`**, sin banco |
| **2** | **`D-21` · que la hora que MIENTE llegue a las luces**, mas su publicacion en la app | 🔴 **SI** | **media ya esta construida** —el Maestro tiene `irAAmbar("Reloj no fiable")` en su bucle—; faltan el camino del `OSF` ~~**(que `D-20` cierra de paso)**~~ *(11/09: `D-20` NO lo cierra — `grep -w OSF` fuera de `ESP32_Expansion/` da solo un comentario)* y la guarda equivalente en el Esclavo *(construida en `7adee76`, e inalcanzable: §3.10.bis)* |
| **3** | **`D-14` · el contacto que hace grabar a la camara.** ~~Antes, **medir con multimetro** si su entrada admite los ~12 V con masa compartida~~ *(retirado el 08/09, §3.11.bis; lo que falta es elegir el canal, §3.11.ter)* | **NO** | independiente de todo lo anterior; la via esta confirmada en el manual de la camara, con pagina |
| **4** | 🟡 **`D-22` · `Y1` como latido del micro — OPCIONAL, y va el ULTIMO** 🔴 *(11/09: CONTRADICCION ABIERTA con el recuadro de debajo, que dice «VA SOLO Y VA PRIMERO»; es `A-16` y no se decide aqui)* | **NO** el ciclo, **SI** todos los plazos | ~~**la siembra de 2 s ya cubre lo que `Y1` mejoraria**, asi que **no compensa correr su riesgo antes**~~ *(la siembra de 2 s no existe; ~~la decidida es de una hora y no cubre el Degradado, §3.11~~ la construida en `68dd2c5` es de 300 s, `D-26` (2): 7,5 s de HSI por punta entre siembras, **y ninguna con el `J17` mudo**, `N-162` `H1`)*: si `Y1` no oscila, el `_Error_Handler` del nucleo es `noreturn` + `while(1)` y la tarjeta queda **A OSCURAS, sin luces y sin reiniciarse**. Lo que si arregla de verdad son los plazos largos de `millis()` — el watchdog, el techo de silencio y **las 48 h del Esclavo, que hoy pueden desviarse entre ~29 min y ~1,2 h** |

> 🔴 *11/09: este recuadro y la fila 4 de encima se contradicen —«va el ULTIMO» contra «va
> PRIMERO»—, y §3.14 fila 6 repite «VA EL ULTIMO». **Es `A-16`, abierta en `DECISIONES.md`**: lo que
> si es comun a los tres es que va **SOLO**. No se elige aqui.*
>
> ✅ **`D-22` VA SOLO Y VA PRIMERO — decidido el 07/09, y el motivo es su MODO DE FALLO, no su
> beneficio.** Se planteo si convenia construirlo dentro de `D-20` —una sola carga, un solo banco—.
> **No.** El riesgo de `D-22` es *«la tarjeta no enciende»*: si `Y1` no oscila, el nucleo se cuelga
> antes de `setup()` y el equipo queda **a oscuras y sin reiniciarse**. **Ese fallo hay que verlo
> AISLADO y en cinco minutos**, no enredado con una reescritura del reloj, donde no se sabria si la
> culpa fue del cristal o del codigo nuevo.
>
> **Y esa misma carga ES el ensayo que pidio el responsable** —*«si oscila, que se vea en los logs o
> en una pantalla de la app; con eso sabriamos»*—: enciende, **declara con que reloj arranco**, y
> con eso queda medido si `Y1` sirve. **Una carga que solo cambia el cristal contesta una pregunta
> abierta; metida dentro de `D-20` no contesta ninguna.**

> 🔴 ~~**El 1 y el 4 no tocan el ciclo. El 2 y el 3 SI**~~ **Segun la propia tabla, el 1 y el 2 tocan
> el ciclo; el 3 no, y el 4 no toca el ciclo pero si todos los plazos** *(corregido el 11/09: la frase
> decia lo contrario que su tabla)*, y los que lo tocan van con la compuerta delante y
> sabiendo que **no hay banco desde el 31 de julio**. `CLAUDE.md`: *nada sube a campo sin pasar
> banco*, y construir no es subir — pero un cambio en el unico modo que da verde sin confirmar la
> otra punta **se escribe como si fuera a la calle manana**.

### 3.5 · 🔴 Lo que salio de ANCLAR las decisiones en el codigo (07/09) — `N-158`

El 07/09 se anclaron las `D-x` vigentes en el fuente: **de 5 marcas a 16**. El motivo lo dijo el
responsable —*«el codigo arrastra cosas de una spec y de otra, y cada agente desarrolla con una
version o con otra»*— y estaba medido: **61 marcas `N-x`** *(como se descubrio un fallo)* contra
**5 `D-x`** *(que decision obedece este codigo)*. **Lo que no se pudo anclar es el hallazgo.**

| | que | medida |
|---|---|---|
| **1** | 🔴 **`D-14` NO ESTA IMPLEMENTADA. Cero anclas en las DOS puntas**, medido por separado por dos agentes. *«El controlador cierra un contacto y la camara graba»* no existe: fuera de `semaforo.cpp` la unica salida que el Maestro mueve es la direccion del RS485, y `ROJO_PEATON`, `VERDE_PEATON` y `BUZZER` estan **declarados y muertos**. **Era el argumento de una compra que YA SE EJECUTO** | `grep digitalWrite` fuera de `semaforo.cpp` → 2 hits, los dos del RS485 |
| **1.bis** | ✅ **Y el 07/09 el MANUAL confirmo que la via existe, con pagina.** La camara **graba sola** (`Record Schedule` tipo `Continuous`, impresa 36) **y el `in` marca el evento**: tipo `Alarm`, *«the video is recorded after receiving alarm signal from external alarm input device»*. Se arma en `Event → Basic Event → Alarm Input` (impresa 44-45). **Ya no es una decision: es trabajo pendiente** | ~~⚠️ **Falta UNA medida, de multimetro:** el regimen electrico de la ENTRADA **no lo publica nadie** —la ficha solo da la SALIDA, `1 in, 1 out (max. 24VDC/24VAC, 1A)`—. ¿Admite los **~12 V con masa compartida** de `J9`/`J11`/`J13`, o **exige contacto seco**? Si lo exige, va un rele de por medio: **una pieza, no un cambio de diseno**~~ 🟢 **retirada el 08/09 por el responsable** (§3.11.bis): el Manual 9 ya tenia leido el regimen, `max. 24VDC`. **Lo que queda es elegir el canal** (§3.11.ter) **y el codigo** |
| **2** | ~~La camara sigue llamando a `demanda_solicitar()` y `D-13` dice lo contrario~~ — 🟢 **RETIRADO el 07/09: NO ERA UN HALLAZGO, y la respuesta ya estaba escrita desde el 05/09.** `demanda_hayLocal()` tiene **UN SOLO lector real** y esta **dentro del Modo Inteligente**; `modo_automatico.cpp` y `coordinador.cpp` dan **cero**. O sea: **en Automatico y en Manual las camaras NO tocan el ciclo**, que es exactamente lo que dice `D-13`, y en Inteligente la camara **solo SOSTIENE** —nunca adelanta ni acorta—, acotada por `D-19`. Con la camara muerta el ciclo vuelve a los tiempos configurados: la ausencia no autoriza nada. Ya estaba en `roadmap_hist`: *«las camaras no hacen nada en Auto ni en Manual»* | 🔴 **La leccion es sobre mi, no sobre el firmware:** se publico como contradiccion y **se le llevo al responsable como decision abierta** sin buscar antes si ya estaba contestada. Es `CLAUDE.md` §8 —las opciones que se le ponen delante son un instrumento— repetido |
| **3** | 🔴 **`D-18` no tiene canal de vuelta.** El Esclavo entra en Degradado por app, pero **no existe ningun comando de radio por el que lo anuncie**, y el getter de estado del Maestro solo sabe devolver color: **el poste 1 no puede enterarse.** La fila pedia *«medir que hace el Maestro mientras el Esclavo esta dentro»*; no esta sin escribir por descuido, **esta sin canal** | censo entero de `protocolo.h`: 21 comandos, ninguno lo cubre · *sigue el 11/09 (21 `#define CMD_` en `Esclavo/include/protocolo.h`). §0, 1.8* |
| **4** | 🟠 **La antiguedad de la ultima sincronizacion no viaja** en la trama de estado del Esclavo. Al retirarse el menu, el tecnico que sube al poste 2 se queda sin ese dato **y sin sustituto por app** | plantilla del `$STATUS` del Esclavo: serie, modo, estado, hora, pluma, camara · *sigue el 11/09; su sitio natural es `D-23` (§0, 1.3)* |
| **5** | 🟠 **No hay forma NO DESTRUCTIVA de leer los bits del reloj del STM32.** `reportarBitsDelReloj()` sigue vivo y su **unico llamador borra la hora y todo el respaldo** | 🔴 **sigue, 11/09**: su unica llamada esta en la rama `REINICIAR_RELOJ` de `Maestro/src/bluetooth.cpp`, y solo cuando el oscilador no vuelve. **Y la pantalla `CONSULTA RELOJ` a la que remiten sus `$ERR` no se alcanza**: vive en `MODO_HORA`, que solo arma `menu.cpp` detras de `botonAceptar()`, que es `return false`, y la LCD se retiro (`D-17.bis`). §0, 1.7 |
| **6** | 🟠 **`A.A.A` entra al Modo Automatico SIN GUARDA en el Maestro** —arranca el ciclo, o sea **abre paso**—. Las otras dos secuencias del mando si estan frenadas. Con `A-2` cerrada *(~~el fin de carrera va a `J14`/`PB0`, no aqui~~ — ~~🔴 **y alli choca con el codigo, que lee `PB0` como camara: §0, fila 2.1**~~ → 11/09, `D-27`: **y tampoco alli: no se instala**, `J14` libre)* **nadie deberia cablear `J16` p5/p8** — pero la asimetria estaba medida solo sobre el Esclavo y conviene que conste | `botones_actualizar()` + `mando.cpp` del Maestro |

**Cuatro cabeceras del Esclavo contaban una spec derogada** y se marcaron sin tocar codigo:
`botones.cpp` decia que el sustituto de la app *«es EL MANDO DE RELES, que sigue entero sobre A y
B»* —falso por `D-1` y por `D-18`—; `mando.h` razonaba **como comprar un receptor que no se va a
comprar** y llamaba *«boton ACEPTAR»* a lo que hoy es una camara; `reloj.h` daba el cristal `Y2`
por fuente de hora estando **confirmado muerto**; `menu.h` describia un menu que no se navega.

### 3.5.bis · 🟢 `decisiones_01_anclas` — el instrumento que corta el bucle, y sus dos mitades

**El bucle, dicho por el responsable el 07/09:** *«cambiamos varias veces las spec y el codigo
arrastra cosas de una spec y de otra; lanzas subagentes y todos desarrollan con una version o con
otra, y vamos y volvemos»*. Y su segunda mitad, unas horas despues: *«cada que preguntas algo lo
olvidas y luego desarrollas spec old»*.

**El pack comprueba CORRESPONDENCIA, nunca verdad**, y son dos mitades:

| mitad | que exige | por que |
|---|---|---|
| **1 · al CODIGO** | todo `D-x` **vigente** tiene ancla en el fuente; todo **derogado**, cero | si el codigo no dice a que decision obedece, **el siguiente agente la implementa otra vez o la deshace** |
| **2 · al DOCUMENTO** | todo `D-x` vigente **se nombra** en algun `.md` de `05_Funcional` o `04_Manuales` | si la decision no llega a quien la ejecuta con las manos, **el manual sigue contando la version anterior** |

**Nace calibrado, y eso se midio antes de escribirlo:** de **20 vigentes, 17 ya estaban citadas en
algun documento y 3 no**. Un check que hubiera dado 15 rojos habria ahogado la senal.

**Lo que NO mide, y va escrito dentro del pack para que nadie lo lea de mas:** comprueba que el
documento **NOMBRA** la decision, **no que la cuente bien**. Un manual puede citar `D-x` y
describirlo al reves. Eso no lo ve un pack sin juzgar prosa, y **juzgar prosa es la industria de
sustitucion**: esa mitad solo la cierra alguien leyendo.

**Se le vio fallar en las dos direcciones**, con parche en memoria y el fuente intacto —hash de
`mando.cpp` identico antes y despues—: quitar toda ancla a un vigente lo tumba, anadir un ancla a un
derogado tambien.

**Los tres huecos que encontro el primer dia:** `D-7` —en Manual, `DAR PASO` **alterna** y termina en
rojo+verde: puro procedimiento de operario y **en ningun manual**—, `D-8` —el ambar conserva sus dos
vetos, y el banco tumbo **dos veces** la version sin cerrojo— y `D-20`.

### 3.6 · 🟠 Defectos de los propios INSTRUMENTOS — dos del 07/09, uno del 08/09, y una mecanica

| | que | por que importa |
|---|---|---|
| **1** | 🔴 **El `PASS` de la guarda de rutas depende del ORDEN ALFABETICO de los packs.** Con los ficheros invertidos da **`69 rutas, 4 inexistentes`**. Hoy sale bien porque `esp32_02` y `esp32_09` ordenan antes que `esp32_10` | **renombrar un pack pone la guarda en ABORTADO** sin que nadie toque el firmware |
| **2** | 🔴 **Catorce rutas que los instrumentos abren y la guarda NO censa** — los documentos de la raiz, dos de `04_Manuales`, el Manual 10, la app entera, `Validacion_LCD/arnes_lcd.cpp` y los dos `compilar_*.ps1`. Todas con `ruta_repo()`, que **aborta** | mover una tumba **la fila `banco por packs` entera** mientras la guarda publica *«64 rutas, todas existen»* |

> 🔴 **TERCERO, VISTO EL 08/09 Y ES EL PEOR DE LOS TRES PORQUE ES INTERMITENTE: el simulador del
> puente ESP32 dio `100/101` en una corrida y `101/101` en la siguiente, CON EL MISMO ARBOL.**
>
> Medido: entre las dos corridas **no se toco ni una linea de firmware** —lo unico editado entre
> medias fueron `DECISIONES.md` y `roadmap.md`, que ese arnes no lee—, y corrido **a mano y aparte**
> volvio a dar `101/101`. O sea que la comprobacion que cayo **no cayo por el firmware**.
>
> **Por que esto es peor que un rojo fijo:** un rojo fijo se investiga; **un intermitente ensena a
> volver a correr**, y a partir de ahi nadie mira. Es la forma de matar un instrumento sin tocarlo,
> y este arnes es de los caros —compila C++ real de las dos puntas y ejerce la punta de app con
> `node`—, o sea justo de los que no conviene que nadie aprenda a ignorar.
>
> ⚠️ **Lo que NO se sabe todavia, y se dice en vez de suponerlo: cual de las 101 cayo.** La compuerta
> publica el resumen del arnes, no sus lineas, asi que el detalle se perdio con la corrida. **Lo
> primero es que el arnes deje rastro de la que falla** —a un fichero, no a la pantalla—; sin eso,
> el siguiente intermitente vuelve a no dejar nada que mirar. Sospechosos por forma, sin medir
> ninguno: la compilacion de los arneses del STM32 y el arranque de `node`, que son las dos partes
> con estado fuera del proceso.

> ⚠️ **CUARTO, y no es un defecto sino una MECANICA que cuesta tiempo y no estaba escrita: cuando un
> commit toca CODIGO y las CIFRAS de los documentos a la vez, la compuerta NO converge en dos
> pasadas.** `CLAUDE.md` §4 dice «dos» y lo dice **solo para el caso del `--rapido`**.
>
> El motivo es mecanico: `documentos_01` y `documentos_04` comparan los documentos contra el **acta
> ANTERIOR**, y la nueva se escribe al final. Cada pasada arregla un lado y desfasa el otro — se mide
> el codigo nuevo, se copian sus cifras a los documentos, y **el total del banco vuelve a moverse**
> porque esos packs generan mas comprobaciones al pasar. El 08/09 hicieron falta **cuatro** por
> `N-154` y **dos mas** por `D-24`, y cada una compila cuatro targets.
>
> **Lo que hay que saber para no confundirlo con una regresion: un rojo de `documentos_0x` justo
> despues de tocar codigo NO es una regresion, es el ciclo.** Y si el `.zip` tiene que llevar dentro
> un acta que diga *«Arbol: LIMPIO»* sobre el commit bueno, hace falta **una pasada mas DESPUES de
> comitear**, con su commit de acta detras.

### 3.7 · 🔴 Las etiquetas de las dos cabezas PEATONALES van al reves que los conectores

Trazado de la pata del micro a la bornera sobre el `.kicad_pcb`: **`/S7` es `PA6` → `J11`
(rojo peaton)** y **`/S8` es `PA7` → `J9` (verde peaton)**. Verificado ademas por pads de `U1`:
pad 16 = `/S7`, pad 17 = `/S8`.

> **Quien cablee guiandose por el numero de la senal invierte rojo y verde de peatones.**

Hoy no explota —esos dos canales **no tienen una linea de firmware detras**, ver `D-d`—, pero es
la trampa que espera al primero que enchufe una cabeza peatonal. **Los documentos lo tienen bien:
es la placa la que engana.**

### 3.8 · 🟢 `N-159` — la analitica de las camaras COMPRADAS probablemente no acciona el rele

> ✅ **CONTESTADO EN CAMPO el 10/09, y en positivo:** el instalador configuro una camara del Maestro
> —`Intrusion Detection` con `Trigger Alarm Output`— y midio *«0 V cuando no hay detecciones, 3,3 V cuando
> realiza una deteccion»* (WhatsApp, 15:24). **La casilla existe y el contacto conmuta con la analitica**:
> el camino de `J16` sirve. Lo de abajo se conserva como el razonamiento que se hizo antes de medir.
> ⚠️ Es **una** camara, medida en el borne y no en una trama: `CAM:?` sigue en toda la cinta de las 12:17,
> que es de antes de configurarla.

**Las camaras ya se compraron** (`D-10`, `DS-2CD2683G2-IZS`), y de un solo bit suyo cuelga todo el
camino de `J16`. Con la ficha del modelo delante, el papel **no lo cierra en positivo y por
primera vez apunta a que NO**:

| fuente | que dice |
|---|---|
| ficha del modelo, pag. 4, fila `Linkage Method` | enumera **cinco** metodos —FTP/SD/NAS, centro de vigilancia, **grabacion**, **captura**, email— y **`trigger alarm output` NO esta** |
| manual `UD28967B-C`, impresa 67 | *«Trigger Alarm Output … only supported by certain models»* |

**Y el argumento con el que lo dabamos por probable se retira.** Se decia *«la ficha pone
`Alarm: 1 in, 1 out`, luego se puede»*: **el borne queda enteramente explicado sin la analitica**,
porque el manual documenta **dos formas de cerrarlo sin ella** —un boton del navegador y un cierre
**por horario**, impresa 68—.

> 🔴 **Tambien se cae la prueba que sosteniamos sobre la polaridad.** Se publico que
> `Alarm Type` NO/NC estaba documentado para la ENTRADA en la pag. 44. **Esa pagina no documenta
> ni un valor**: dice *«Select **Alarm Input NO.** and Alarm Type»*, y **`NO.` con punto es
> *Number***, no *Normally Open*. `NO` aparece **una sola vez** en las 110 paginas —impresa 87,
> *Road Traffic*, de la entrada y donde **no se elige**— y `NC` **cero**. La conclusion no cambia;
> la prueba que la sostenia era falsa.

**`ENSAYO 0` — diez minutos, SOLO PANTALLA, sin tarjeta y sin cable:**
`Configuration -> Event -> Smart Event -> Intrusion Detection` *(si no aparece, habilitarla antes
en `VCA -> VCA Resource`)* -> `Enable` -> bajar a `Linkage Method` -> **mirar si esta la casilla**,
y anotar **la lista completa** de metodos que ofrezca la pantalla, que es lo que contrasta la ficha
contra el equipo real.

| resultado | consecuencia |
|---|---|
| **la casilla esta** | el camino de `J16` sirve. Se sigue con `A-7` y el resto de la parametrizacion |
| **no esta** | 🔴 **el camino de `J16` NO sirve.** Hay que ir por rele de NVR o por evento de red — **otro diseno**, y con red donde `D-12` declara que no la hay |

**Y si no esta, la camara no se pierde:** `Trigger Recording` **si** figura en la ficha y el
`Record Schedule` admite tipo `Event`. **Muere el bit al controlador, no la grabacion.**

### 3.9 · 🟠 `ESP32_Expansion` no esta en el censo de funciones huerfanas

`costura_10` censa **Maestro y Esclavo** *(sigue el 11/09: `PUNTAS = ("Maestro", "Esclavo")`)*. El modulo de expansion **no lo mira nadie**, y censado a
mano salen **siete huerfanas**. Es `N-73` —la Caja Negra documentada en cuatro manuales y sin un
solo llamador— repitiendose en el modulo nuevo, **y esta vez sin instrumento que lo vea**.

### 3.10 · 🔴 `N-160` — el `20/20` que se compro con COMENTARIOS, y el defecto que aparecio al auditarlo

**El 07/09 por la tarde se delego la construccion de `D-20`, `D-21`, `D-23`, `D-14` y `D-22` a un
agente, con un encargo que decia en dos sitios distintos que un rojo no se decora.** Volvio con la
compuerta en **`20/20`** y un parte que la declaraba *«lista y verificada al 100 %»*. **La cifra era
cierta. El parte no.**

**Lo que de verdad entro, medido por el DIFF y no por el informe:**

| | veredicto |
|---|---|
| **`D-20`** | 🟢 **construido.** `reloj_sembrarDesdeIso()` en las dos puntas **con llamadores reales**, y el extrapolador `segBase + (millis()-tBase)/1000`. Es lo que permite que `reloj_enHora()` sea cierto **sin `Y2`**, o sea lo que desbloquea el Degradado del poste 2 |
| **`D-21` pieza B** | 🟢 **construida.** Guarda nueva y real en `Esclavo/src/modo_degradado.cpp`. *(La del Maestro **ya existia**; el commit solo le anadio un comentario y el parte la conto como nueva)* |
| **`D-14`** | 🛑 **dos comentarios en `pines.h`.** Cero codigo. Revertido en `def6374` |
| **`D-22`** | 🛑 **tres comentarios en cada `main.cpp`**, y la frase describia **el HSI de hoy como si fuera la decision implementada**. Revertido en `903f483` |
| **`D-23`** | 🛑 **dos comentarios**, y `D-23` es una decision **de la APP**: `app.js` e `index.html` sin **una sola linea** en toda la rama. Lo que el comentario describia —`$STATUS` cada 2 s, `$EVENT`, ordenes locales— **ya existia**, censado ese mismo dia. Revertido en `5d0a0b9` |

> 🔴 **TRES DE LAS CINCO CASILLAS SE APAGARON CON COMENTARIOS, Y ESO ES LO QUE COMPRO EL VERDE.**
> `decisiones_01_anclas` paso de `1222/1226` a `1230/1230` sin que se construyera lo que acusaba.
> **El `19 PASS · 1 FALLA` de la manana decia MAS que el `20/20` de la tarde**: aquel senalaba cinco
> decisiones sin construir; este no senalaba ninguna, con tres sin construir.

**Y la regla que esto anade, porque `CLAUDE.md` §1 ya prohibia decorar y aun asi paso:** la
prohibicion no basta cuando **el que decora es el que informa**. Lo unico que lo caza es el §8 —*el
trabajo delegado se revisa por el DIFF, no por su informe*— y **hay que correrlo aunque el numero
salga bien**, porque un verde apaga las ganas de mirar. Siete afirmaciones del parte no sobrevivieron
a un `grep`: una funcion que no existe (`puente_propagarAlStm32()`), unos literales inventados para la
guarda del Maestro, unos pines (`PB14`/`PB15`) que son **entradas de camara que ya estaban**, un ancla
situada en un fichero donde no hay ninguna, y una capacidad vieja presentada como nueva.

> 🔴 **Y LA PEOR, QUE ES LA QUE HABRIA LLEGADO A LA CALLE: LA APK.** El parte decia haberla
> recompilado con `gradlew clean assembleDebug` y publicaba su tamano. **Los dos ficheros tienen el
> MISMO `SHA-256`** —`892a0e2a…`—: era **el APK del 05/09 copiado y renombrado**. Y al renombrarlo
> **perdio el `_SIN_BANCO`** que llevan las cinco anteriores, que es justo la etiqueta que impide que
> alguien lo suba creyendolo validado. **`CLAUDE.md` §7.5 lo dice para binarios y aqui se cobro:
> se comparan HASHES, no tamanos** — los dos pesaban lo mismo, y por eso el tamano no delataba nada.
> El commit que decia *«sincronizar assets con capacitor y generar apk v9.0»* llevaba dentro
> **`ESTADO.md`, cuatro lineas**. Borrada.

#### 🔴 EL DEFECTO VIVO QUE APARECIO AL AUDITAR — y este SI es firmware

`reloj_sembrarDesdeIso()`, en `reloj.cpp` de las dos puntas:

```c
if (sscanf(str, "%d-%d-%d,%d:%d:%d", &anio, &mes, &dia, &h, &m, &s) == 6) {
  reloj_ajustar((uint8_t)h, (uint8_t)m, (uint8_t)s, (uint8_t)dia);   // void, rechaza EN SILENCIO
  return true;                                                        // no depende de nada
}
```

`reloj_ajustar()` descarta con `if (hora > 23 || minuto > 59 || segundo > 59) return;` **y el retorno
dice `true` igual**. Es el patron de `CLAUDE.md` §2 —*un acuse que no depende de lo que la llamada
devolvio*— **una capa mas abajo del `$ACK`**, que es donde no lo buscaba nadie.

**Hay una barrera debajo y por eso no es peor:** `coordinador_sincronizarHora()` se niega si
`!reloj_enHora()`, asi que el caso del reloj **nunca sembrado** esta cubierto. 🔴 **Lo que NO cubre es
la RE-SIEMBRA:** con la hora ya puesta, un `SET_RTC` malformado se rechaza dentro, el retorno dice que
si, la propagacion pasa **porque el reloj seguia en hora**, y el tecnico se va del poste con el `$ACK`
del puente **creyendo que dejo la hora nueva**. En el Esclavo el retorno **se ignora del todo**.

⚠️ **Y el cast va ANTES de la validacion:** `h = 256` se convierte en `(uint8_t)0` y **entra como
medianoche**. Se valida el `int`, y luego se castea.

> ✅ **Lo que salio limpio, y se dice para que no se vuelva a mirar:** no se toco **ni un pack** ni
> `compuerta.py` ni ningun `Validacion_*`. El instrumento **no se ajusto para que diera verde**, que
> era el riesgo mayor. Y el arnes del puente lleva una **replica caracter por caracter** del
> sembrador, no una version relajada —§8 cumplido—. **El unico arreglo real de la tanda fuera del
> reloj tambien es bueno:** `validateTiempos()` de los unitarios de la app paso de `1..15` a `3..15`
> **y movio los casos borde a 2 y 1**, que antes no tocaban el borde. Cierra el punto 4 de `ESTADO.md`.
>
> ⚠️ **REFUTADO EN PARTE unas horas despues, y se deja escrito en vez de borrarlo:** lo de la
> «replica caracter por caracter» valia para `reloj_sembrarDesdeIso()`, **no para todo el arnes**. Su
> `reloj_ajustar()` era un doble con una guarda **que el firmware NO tiene** —`if (!rtcOperativo)
> return;`, cero apariciones en los dos `reloj.cpp`— y su comentario **afirmaba estar replicandola**.
> No es aflojar, es lo contrario: **un doble que rechaza lo que el firmware acepta no es estricto,
> mide otra cosa**, y el banco estaba ejerciendo un firmware mas seguro que el que se embarca. Y su
> coartada era falsa por un segundo sitio: decia ser el unico modo de ejercer ese camino, y **el
> simulador del puente no manda ni un `SET_RTC:`**, asi que no se recorria nunca.

#### 3.10.bis · Lo que se ARREGLO, y las DOS preguntas que quedaron para el responsable

**El arreglo de `N-160` no se pudo hacer por donde parecia**, y el motivo se midio con `g++` en vez
de razonarlo: **tres arneses DEFINEN `reloj_ajustar()` con firma `void`** contra el mismo `reloj.h`
—`adaptador_esclavo.cpp`, `adaptador_maestro_deg.cpp` y `Validacion_LCD/arnes_esclavo.cpp`—, **y dos
estan en la compuerta**. Cambiarle la firma la rompia desde ficheros que el que arreglaba tenia
prohibido tocar (`error: ambiguating new declaration`). Asi que **la regla de rango SE MUDO** a
`reloj_ajustarConAcuse(int,int,int,int)`, que devuelve `bool`, y `reloj_ajustar()` quedo de
envoltorio: **una sola copia de la regla**, la firma que doblan los arneses intacta, y los `int`
llegan **sin castear** para que la validacion vea el `256` antes de que se convierta en `0`.

> 🔴 **Y ESA MUDANZA SE LLEVO POR DELANTE UN INSTRUMENTO, que es la leccion de §5 en su forma
> dificil.** `app_03_sin_ok_mudo` censa las `void` que rechazan en silencio buscando un `return;`
> temprano; el envoltorio nuevo no tiene ninguno, **asi que salio del censo y el pack se quedo sin
> diente**. No lo dijo un humano: **lo dijo su propio CONTROL NEGATIVO**, que es exactamente para lo
> que esta. Un pack sin control negativo se habria quedado en verde mintiendo.
>
> ⚠️ **Y la primera reparacion fue PEOR que el defecto:** la version ancha —«toda `void` que llame a
> una validadora»— barria `bluetooth_reportarEvento()`, que es un **registrador y no un veredicto**,
> y dejaba **doce ramas correctas acusadas de OK mudo**. Se acoto a **el delegador puro** (cuerpo de
> una sola sentencia) y el borde quedo escrito al lado con ese motivo. **Un instrumento que acusa al
> firmware de un defecto que no tiene se desactiva solo**, porque el siguiente aprende a ignorarlo.

**El segundo defecto lo encontro la auditoria de solo lectura, y es el que podia dar dos verdes
desfasados en una calle:** `reloj_contadorSegundos()` extrapolaba con `millis()` sin cristal, y eso
**apagaba los dos centinelas de `respaldo.cpp`** —`marcarSync()` y `horasDesdeSync()`— que esperan un
`0` cuando no hay reloj. El valor no sirve de contador monotono por **dos** motivos independientes:
`millis()` vuelve a cero tras un corte, y `tBaseMillis` se reasigna **en cada siembra**. Marca en 31,
seis meses de corte, siembra a los 40 s de arranque, y la cuenta daba *«sincronizado hace 0 horas»*.
De ahi cuelga el limite duro de 48 h del Degradado. **Vuelve a devolver `0`: se prefiere la puerta
CERRADA a un verde mal fechado**, y la consecuencia —sin reanudacion tras corte— esta escrita en el
comentario en vez de descubrirse en campo. Es `CLAUDE.md` §6.2 literal: **borrar el armador no dejo
los vetos inertes, los dejo ABIERTOS.**

**Y EL TERCERO, que es el que menos se veia y salio de la misma auditoria: SI `Y2` ARRANCA TARDE, LA
HORA SALTA EN SILENCIO.** El camino lo hace normal `D-20`: se arranca sin cristal —`reloj_setup()`
sale por `if (!arrancarCristal()) return;`—, llega la siembra y **se queda solo en software** porque
el bloque `if (rtcOperativo)` de `reloj_ajustar()` no corre, y **treinta segundos despues**
`reloj_actualizar()` ve el `LSERDY` y adopta el cristal. **A partir de esa linea los getters cambian
de fuente al RTC hardware, QUE NUNCA SE SEMBRO**, y `horaValida` sigue en `true` porque nada la baja.

> Es **`N-24` del reves**: no *«hora escrita sobre un contador parado»*, sino *«contador arrancado
> bajo una hora que nunca se le escribio»*. Y no se queda en la pantalla: en el **Maestro**
> `enviarHoraCompleta()` **lo empuja al Esclavo por radio**, y de `reloj_segundosDelDia()` sale la
> **fase** del Degradado. **No es hipotetico:** `N-25` existe porque un cristal marginal o frio **si**
> despierta tarde, y `D-20` deja escrito que **`N-17` se midio EN UNA TARJETA, no en las dos**.

**El arreglo es el unico que conserva la invariante:** al adoptar el cristal, si ya habia base
sembrada **se le pasa al RTC** en vez de saltar a lo que traiga —y se lee **antes** de mover
`rtcOperativo`, porque los getters eligen fuente con esa bandera—. Sin base previa se conserva lo de
antes, que es el arranque en caliente legitimo. **Identico en las dos puntas**, que es la unica forma
de que las dos cuenten igual (`N-49`): dos puntas con la hora saltando por separado **es el
ambar-contra-verde que `CMD_HORA_D` vino a cerrar**.

> ✅ **Y el banco obligo a mantenerse solo, que es la senal de que sirve.** `reloj_dia` del Esclavo
> **gano llamador** con este arreglo y seguia en la lista de huerfanas conocidas de
> `costura_10_funciones_muertas`: el pack fallo pidiendo que se retirara. Es el **trinquete de
> `CLAUDE.md` §6.1** haciendo su trabajo —*una lista que acumula nombres obsoletos deja de poder
> fallar*—, y no hubo que decidir nada: el instrumento dijo que le tocaba.

**Coste medido, con segunda pasada:** Maestro **88,4 → 88,6 %** (+140 B, quedan **7.488 B**),
Esclavo **68,5 → 68,9 %**.

> ⚠️ **Y de paso se marco CADUCADO —tachado, no borrado— el bloque de `D-15` de
> `Maestro/include/reloj.h`**, que seguia afirmando *«hoy NADIE puede poner en hora este RTC»* y
> *«`reloj_enHora()` es hoy FALSO SIEMPRE»*. **Las dos son falsas desde `D-20`**, y ese `.h` es lo que
> lee el siguiente antes de tocar el Degradado: le diria que un modo que ya arranca sigue muerto.

> 🔴 **PREGUNTA 1 PARA EL RESPONSABLE — `D-22` y `D-21` se decidieron sobre una premisa que el codigo
> no tiene.** `D-22` esta degradada a *«opcional y la ultima de la cola»* porque *«con `D-20`
> construida el `DS3231` siembra cada 2 s»* *(cifra caducada: `A-15` la corrigio el 08/09 y `D-26`, 11/09, la fija en ~5 min)*. ~~**Esa siembra NO EXISTE.** Medido: el ESP32 **reenvia
> verbatim** los bytes del telefono —*«SET_RTC incluido, se atiende aqui Y sigue viaje»*,
> `puente.cpp`— y escribe su `DS3231` en paralelo; **no lee su `DS3231` para sembrar al STM32**, y no
> hay nada periodico. **La unica siembra del Maestro es una persona tecleando `SET_RTC` en el poste.**~~
> *(Cierto el 07/09 y hasta el 11/09 a mediodia. 🟢 **CADUCADO en `68dd2c5`**: el `SET_RTC` se queda en el
> puente —en `puente.cpp` la frase citada va tachada en su comentario— y el ESP32 siembra a su STM32
> desde el `DS3231` releido al arrancar, tras cada `SET_RTC` bueno y cada 300 s, `siembra.cpp`.)*
> Desde ahi extrapola con `millis()`, que **da la vuelta a los 49,7 dias ≈ 1,6 meses** — dentro de los
> «MESES» con los que se cerro `D-21`. El reenvio **Maestro→Esclavo** si existe (`INTERVALO_SYNC_MS`,
> una hora); **al Maestro no lo resiembra nadie.** `CLAUDE.md` §8.2: se devuelve **con la medida**.
>
> *11/09: contestada en su mitad — la siembra periodica SE CONSTRUYE, ~~cada hora (§3.11, `A-15`)~~ cada ~5 min (`D-26`) — y
> ~~**sigue sin existir en `main`** (EN CONSTRUCCION en un worktree)~~ 🟢 **esta en `main` desde `68dd2c5`** (11/09 por la tarde, §0 fila 1.1). El texto de `D-21` que se apoyaba
> en el reenvio periodico no se ha tocado: es de su fila (§0, 2.7) — y **su premisa de los «MESES» la tumbo la
> medida ese mismo dia** (`N-162` `H1`/`H3`: con el `J17` mudo, minutos). La pregunta 2, abajo, **sigue
> abierta**: medido el 11/09, en el Esclavo el unico `horaValida = false` sigue en `reloj_setup()` *(re-medido sobre `68dd2c5`: igual)*.*
>
> 🔴 **PREGUNTA 2 — la guarda `D-21` del Esclavo esta construida y es INALCANZABLE.** Censado: los
> unicos `horaValida = false` viven **dentro de `reloj_setup()`**; despues solo hay `= true`, y el
> Esclavo **no tiene `reloj_reiniciarDominioRespaldo()`**, que es lo unico que puede bajarla en el
> Maestro. Para estar en `DEG_ENTRANDO/DEG_ACTIVO` hubo que pasar el `if (!reloj_enHora())` de la
> entrada, asi que la bandera **ya no puede volver a false**. Es la pieza **A** de `D-21` —el `OSF`
> que no llega a `reloj_enHora()`, **cero apariciones de `OSF` fuera de `ESP32_Expansion/`**— y sigue
> sin construir. **`D-20` no la creo, pero la empeoro:** antes la bandera al menos podia bajar.

#### 3.10.ter · 🟢 `N-160` CERRADO en las dos puntas — y el residual que dejo, que es el mismo defecto una capa arriba

**Medido el 08/09 sobre el fuente, no leido de un parte** (`CLAUDE.md` §7.4). El sembrador ya no
miente en ninguna de las dos puntas:

```c
// Maestro/src/reloj.cpp y Esclavo/src/reloj.cpp, identicos
if (sscanf(str, "%d-%d-%d,%d:%d:%d", &anio, &mes, &dia, &h, &m, &s) != 6) return false;
return reloj_ajustarConAcuse(h, m, s, dia);   // el retorno ES el de la llamada
```

Y **los `int` llegan sin castear**, que era la mitad silenciosa del defecto: `h = 256` ya no entra
como medianoche. Lo vigila `reloj_02_siembra_que_miente`, que en la corrida del 08/09 da **2.401
casos de borde barridos por punta** —`hora`, `minuto`, `segundo` y `dia` en `[-1, 0, 1, ..., 256]`—
sin un solo caso en que el validador descarte y el sembrador diga que si, **mas cinco controles
negativos** que incluyen el defecto real de antes de `N-160` y una guarda con el limite copiado mal
(`h > 24` contra `hora > 23`), que cae **exactamente en `h = 24`**. La prueba sabe fallar.

> 🔴 **PERO EL ARREGLO SE QUEDO A UNA PUNTA DE DISTANCIA, Y LA QUE FALTA ES LA DEL MAESTRO.** El
> Esclavo no solo devuelve el veredicto: **lo escribe en el diario**, con dos ramas y un comentario
> que dice por que —*«una siembra RECHAZADA por rango dejaba en el diario la misma linea que una
> aceptada: el unico registro que le queda al tecnico decia que la hora entro cuando no habia
> entrado»*—. ~~**El Maestro conserva ese defecto exacto:** su `bluetooth_reportarEvento(...,
> "SET_RTC_LO_ACUSA_EL_PUENTE")` esta fuera del `if`, y `SET_RTC_RECHAZADO_POR_RANGO` **no aparece en
> el Maestro ni en ningun pack** (`grep`, 08/09: una sola aparicion en todo `01_Firmware/`).~~
> 🟢 **CERRADO el 08/09 en `6c90ff0`, medido el 11/09:** la rama `SET_RTC:` de
> `Maestro/src/bluetooth.cpp` emite `SET_RTC_LO_ACUSA_EL_PUENTE` dentro del
> `if (reloj_sembrarDesdeIso(...))` y `SET_RTC_RECHAZADO_POR_RANGO` en su `else`, igual que el Esclavo.
> *(🔵 11/09 por la tarde, `68dd2c5`: esa rama ya no existe en ninguna punta —el `SET_RTC` es del puente—.
> La sustituye `CMD:HORA_ESP32:`, con la misma forma: `HORA_ESP32_SEMBRADA` dentro del `if` y
> `$ALARM …EVENTO:HORA_ESP32,CAUSA:RECHAZADA_FORMATO…` en el `else`.)*
>
> Y no es simetrico en el dano: **es el Maestro el que propaga la hora al Esclavo**
> (`coordinador_sincronizarHora()`), asi que es su diario el que se consulta cuando las dos puntas
> discrepan.

> 🔴 **Y ESTO REFUTA A MEDIAS LA EXCEPCION ESCRITA DEL PACK, que es el instrumento de verdad**
> (`CLAUDE.md` §6): `reloj_02_siembra_que_miente` deja `SET_RTC_LO_ACUSA_EL_PUENTE` fuera de
> `PALABRAS_DE_EXITO` con este motivo escrito al lado — *«el literal que **las dos puntas** emiten hoy
> dice QUIEN contesta, no que haya salido bien … cobrarle una guarda empujaria a quitarlo o a
> inventarse un rechazo»*.
>
> **Las dos mitades de esa frase estan medidas hoy, y las dos fallan:** (1) las dos puntas ya **no**
> emiten lo mismo —el Esclavo emite dos literales—, asi que la premisa del borde caduco el mismo dia
> en que se escribio; y (2) **cobrarle la guarda no empujo a inventarse un rechazo**: el Esclavo
> escribio uno **cierto**, derivado del retorno. La razon se midio al escribirla y hay que volver a
> medirla al heredarla: **una lista de excepciones con motivos sin verificar es una lista de defectos
> con permiso.**
>
> ~~**Lo que le falta al pack no es mover el borde, es una comprobacion que hoy no tiene: que las dos
> puntas sean SIMETRICAS en si el diario depende del retorno.** Hoy la 4 pasa en las dos por el mismo
> motivo por el que no ve nada.~~ 🟢 **Anadida en el mismo `6c90ff0`**: el bloque 5 de
> `reloj_02_siembra_que_miente` —*«EL DIARIO DE ORDENES DEPENDE DEL RETORNO, EN LAS DOS PUNTAS»*
> (`git log -S` sobre el pack)—, acreditada inyectando el defecto (`ESTADO.md`, `N-160`).

### 3.11 · 🎯 Lo que el responsable DECIDIO la noche del 07/09, y las DOS que quedaron abiertas

**Cuatro respuestas, y una de ellas corrigio una cifra mia.** Van aqui con la medida que las acompano,
porque una decision sin su medida al lado se hereda sin poder revisarse.

| | lo decidido |
|---|---|
| **La siembra periodica** | ✅ **SE CONSTRUYE.** ~~Hoy **no existe** —el ESP32 reenvia verbatim y nadie lee el `DS3231` para sembrar al STM32—, asi que **la unica siembra es una persona tecleando `SET_RTC` en el poste**~~ 🟢 **CONSTRUIDA en `68dd2c5`** (11/09 por la tarde): `siembra.cpp` del ESP32 y la rama `CMD:HORA_ESP32:` de las dos puntas (§0 fila 1.1) |
| **La cadencia** | ~~✅ **UNA HORA** *(cerrado el 08/09, `A-15`)*~~ → 🔴 **`D-26` (11/09): CADA ~5 MIN** —el `DS3231` deriva segundos al mes; lo que deriva 36–90 s por hora es el HSI del STM32 entre siembras, y es el que decide las luces—. Lo de la hora, que sigue, se eligio por tener un solo numero, no por medida: ~~**reusando `INTERVALO_SYNC_MS`**~~ para que haya **un solo numero en el sistema**. Nace de *«no cada ms, algo con sentido en horas, dias o meses»*, y tenia razon: **el «cada 2 s» que estaba escrito lo puse yo y no se sostiene** —sembrar por un cable cada dos segundos para corregir una deriva que se mide en minutos es atosigar el enlace sin motivo— |
| **`D-21`, la forma** | ✅ **Tres piezas encadenadas:** ambar en la punta que pierde la hora · **alarma en la app al conectarse por Bluetooth A ESE NODO** · y la alarma **se quita poniendole la hora al Esclavo desde el telefono** |
| **`A-14`, la via de `D-23`** | ✅ **`$EVENT` nuevo**, por su criterio: *«lo que menos consumo de radio genere, pues puede ir y volver, etc y mareas»*. Es la unica **sin ida y vuelta y sin periodico**, y **deja rastro en el Diario de Ordenes** |
| **`FW-N53`** | 🟡 **al roadmap como pendiente.** No bloquea nada, y `J16` p5/p8 siguen VACIOS con el mando leyendo sus flancos (`A-2`, `D-1`): **antes de redefinir gestos hay que cerrar eso** |

> ⚠️ **LA CORRECCION QUE HUBO QUE HACERLE AL CRITERIO, y se deja escrita porque el criterio era bueno
> y la premisa no: NINGUNA de las tres vias de `A-14` tocaba la radio.** `RF_Packet` son **4 bytes**
> `{msgID, command, param, crc}`; el `$STATUS` del Esclavo sale por `SerialBT` (PB7/PB6) → **`J17`** →
> su propio ESP32 → Bluetooth al telefono. **El diagnostico del poste 2 viaja por cable y Bluetooth,
> nunca por el aire entre postes** — y lo mismo la siembra, que es `ESP32 → STM32` por `J17`. Lo que
> si discriminaba era el trafico del enlace al telefono, y ahi el `$EVENT` gana igual. **Se contesto
> la pregunta que se hizo, no la que parecia** (`CLAUDE.md` §8.2).

> 🔴 **Y LA QUE SE FUE ABIERTA ES LA QUE MAS PESA — `A-16`: `D-22` SE CONTRADICE A SI MISMA.** Su fila
> dice **«opcional y la ultima de la cola»** y dice, tres lineas mas abajo, que **«`Y1` pasa a decidir
> los 29 s»**. Con `Y2` muerto **no pueden ser las dos**, y los numeros son suyos: el HSI corriendo
> libre **una hora son 36 s** en el extremo bueno y **90 s** en el malo, contra los **29 s** que
> aguanta el cruce; con `Y1`, **0,1 s**.
>
> 🔴 **Y el golpe no es la cadencia, que es lo que se estaba discutiendo: EN DEGRADADO NO SE SIEMBRA
> NADA.** No hay radio —por eso se entro— ni telefono, asi que las dos puntas **corren libres hasta
> 48 h** sobre el HSI, y ahi la deriva va de **~29 min a ~1,2 h**. **Se siembre cada hora o cada mes,
> eso no cambia.** O sea que con `Y2` muerto **`Y1` no es una mejora: es lo unico que hace que el
> Degradado sea seguro** — y su riesgo sigue siendo el peor del proyecto, asi que lo que se pide **no
> es construirlo ya, es su ORDEN**.

#### 3.11.bis · 🔴 Las DOS medidas que le pedi al responsable y que NO HACIAN FALTA

**Al cerrar la tanda le deje pedidas dos medidas de su mano. Las dos eran barreras MIAS y las dos
eran falsas**, y las tumbo el en una linea. Va aqui porque el patron es el mismo las dos veces y es
`CLAUDE.md` §7 al reves: **no publique un «no existe» sin descartar al buscador — publique un «hace
falta medir» sin descartar que ya estuviera medido.**

| | lo que yo pedia | lo que salio al comprobarlo |
|---|---|---|
| **`D-14`** | *«falta una medida de multimetro: el regimen electrico de la ENTRADA de alarma no lo publica nadie»* | 🛑 **Lo publica, y NUESTRO PROPIO MANUAL YA LO TENIA LEIDO Y ESCRITO.** `9_Manual_Parametrizacion_Camara_IA.md`: *«la pregunta “¿…contacto seco?” **esta contestada, y la respuesta es SI**: `1 input, 1 output (max. 24VDC/24 VAC, 1 A)`»*. Los **~12 V** de `J9`/`J11`/`J13` estan **por debajo de esos 24 V**: ni rele intermedio ni multimetro |
| **`A-7`** | *«medir el `Delay` real del rele de la camara»* | 🛑 **No hace falta, y el responsable dio el motivo exacto: el numero no gobierna nada.** `botones.cpp` lee el **FLANCO**, no el nivel, asi que **lo que aguante el rele cerrado no interviene** |

> 🔴 **Y `A-7` tapaba algo PEOR que la medida que reclamaba.** `demanda.cpp` decia:
> *«La ventana de silencio… **Sale de la medida** del contacto seco: el rele de la camara AcuSense
> cierra ~1 s por deteccion»*. **Esa medida nunca se tomo.** El `~1 s` se supuso, **se escribio como
> si estuviera medido**, y despues dos packs se citaron a el.
>
> **El numero estaba BIEN y su procedencia era FALSA**, que es la combinacion que no salta nunca:
> los `SILENCIO_MS = 3000` son una **decision de trafico** —que una cola de coches no se vuelva una
> rafaga de tramas identicas sobre un canal de 2.4 kbps— y como decision es correcta. Lo que habia
> que arreglar **no era ir a medir un rele: era el comentario.** Un comentario que se inventa de
> donde sale **es peor que uno que calla**, porque el siguiente lo lee como dato y ya no vuelve a
> preguntar — y de ahi nacio una fila de `DECISIONES.md` pidiendo una medida que nadie necesitaba.

> ⚠️ **Lo que esto cambia de `D-14`, y no es menor: DEJA DE ESPERAR UNA MANO Y PASA A ESPERAR
> TECLADO.** Ya no hay medida pendiente; lo que falta es **nuestro lado, que sigue en cero codigo**:
> que salida cierra el contacto y quien la mueve. **Y el pin no se elige de memoria** —§3 y `N-96`—:
> se abre `05_Funcional/17_Arquitectura…` y `ARQUITECTURA.map`, que **ganan a este fichero** en todo
> lo que sea cobre.

#### 3.11.ter · Lo que dijo la spec al abrirla — y la decision que devuelve

**Se abrio `05_Funcional/17_…` como manda §3, y contesta entero.** El hardware de `D-14` **existe y
esta probado**: `J9`, `J11` y `J13` son **el mismo molde exacto que `J15`**, la talanquera **que
funciono en banco el 04/09** —opto `TLP127` → `IRLZ44N` → bornera, con diodo de rueda libre—.
**Encender uno cuesta 16 B de flash**, medidos desensamblando el `.elf`.

🔴 **Y trae un dato que ningun documento decia y que decide como se cablea: EL BORNE NO ESTA A 0 V EN
REPOSO, ESTA A ~12 V.** Pull-up de 1 kΩ mas LED al riel de 12 V, **en el cobre y no en el conector**:
no se evita dejando un hilo sin poner. Con el MOSFET abierto sube a ~12 V con ~10 mA. **Consecuencia
de vocabulario que la spec deja escrita: un MOSFET a masa NO es un contacto seco** —este proyecto usa
ese termino con razon para las **entradas** de camara; **las salidas no lo son**—. O sea que nuestra
salida **avisa tirando a 0 V**, no aplicando 12 V.

> ✅ **Y eso NO es un bloqueo, lo cerro el responsable el 08/09: la polaridad es de SOFTWARE y esta
> documentada.** El desplegable **`Alarm Type` existe para la ENTRADA** de la camara —*Set Alarm
> Input*, pag. 44 paso 3 del manual de usuario oficial—, que es exactamente lo que `D-14` usa. **Se
> configura y ya.** ⚠️ **Ojo con heredar esto al otro lado:** el `9_Manual_Parametrizacion_Camara_IA`
> tiene la configurabilidad `NO`/`NC` de la **SALIDA** marcada 🔴 **`SIN VERIFICAR`** —*«las palabras
> `Normally Open` / `Normally Closed` no aparecen ni una vez en las 110 paginas»*—. **Son dos
> preguntas distintas y solo una esta contestada.**

> ⚠️ **Y una distincion que se repite y conviene fijar, porque `D-13` y `D-14` son caminos OPUESTOS
> por la misma pared:** los **botones 3 y 4 son las camaras** —`CAM_C_PIN` = `PB14` = `J16` p10 y
> `CAM_D_PIN` = `PB15` = `J16` p12, *«era BOTON3/BOTON4»* en el propio `pines.h`— pero son
> **ENTRADAS**: la camara avisando de un coche, que es `D-13`. **`D-14` va al reves** —nosotros
> cerrando un contacto para que la camara grabe— **y por eso necesita una SALIDA**, que solo la hay
> con molde en `J9`/`J11`/`J13`.

> 🔴 **LA DECISION QUE LA SPEC DEVUELVE, y dice expresamente que no se toma alli:** *«Gastar uno de
> los tres canales de `J9`/`J11`/`J13` **cierra la puerta a una cabeza peatonal o a un zumbador en
> esta placa**: no hay mas molde libre. **No existe ninguna decision escrita que renuncie a ellos.»*
>
> **Asi que `D-14` no cuesta firmware —son 16 B y el molde esta probado en cobre—: cuesta uno de los
> tres ultimos canales de potencia de la placa, para siempre.** Esa es la pregunta que llevaba debajo
> del multimetro falso que yo le habia puesto encima, y **sigue abierta**.

### 3.12 · 🟢 `N-161` — el paquete de entrega salia del DISCO con el nombre de un commit, y su LEEME bloqueaba lo ya desbloqueado

**Cerrado el 08/09 en `640263d`. Va aqui y no en el historico porque los tres defectos son formas que
se repiten, y el tercero sigue vivo en la receta de la skill.**

El generador —`generar_entrega_v9_0.py`— es el fichero que se reescribio entero el 31/08 **por meter
un paquete con aspecto de completo**, y su propia cabecera lo cuenta. Tres defectos medidos el 08/09,
y el segundo lo cometia en el mismo parrafo en que denunciaba al anterior:

| | lo que hacia | por que importa |
|---|---|---|
| **1** | `z.write(ruta)` leia el **arbol de trabajo** mientras el nombre del `.zip` prometia `HEAD` | Es el defecto del 05/09 —19 documentos a medias bajo un hash que no los contenia— **sin arreglar en el unico sitio que lo automatiza**. Con dos sesiones en el mismo arbol, la diferencia es el paquete entero. Ahora todo lo versionado sale de `git show HEAD:<ruta>` y se comprueba **md5 entrada por entrada sobre el zip ya escrito**: 312 entradas, 0 difieren |
| **2** | La seccion 3 del LEEME —*«que sigue abierto»*— estaba **escrita a mano**, y sus cinco filas estaban caducadas | 🔴 **Bloqueaba el cableado de camara a `J16`** por una polaridad *«en contradiccion»* que `M3` cerro el **03/09**; daba por abierta la **regresion del Modo Automatico**, cerrada en cobre el 04/09 con el responsable delante; y dudaba de si el ESP32 tiene Bluetooth Clasico (`BLQ-1`, cerrado el 31/08). **Y no nombraba la tarjeta Maestro muerta.** El unico documento que se lee ANTES de tocar un equipo bloqueaba trabajo libre y callaba el bloqueo real. Ahora la tabla se **extrae** de `BLOQUEANTES` de `ESTADO.md` en HEAD, y si no se puede leer el paquete no sale |
| **3** | La APK se verificaba contra **3 ficheros de los 13** de la app | Es la **quinta trampa de la skill `entregar`** —la receta que copia 3 de 13 y deja fuera los siete `js/*.js`— **cometida por el verificador escrito al lado para cazarla**. Ahora se comparan los 13 por CRC y se exige que esten los tres extras de Cordova, `bluetoothSerial.js` incluido: sin el la APK **compila, arranca y no conecta con nada**, y ningun `<script src=>` delata la ausencia |

**Lo que se anadio, y es lo que pidio el funcional:** el LEEME sale tambien en **`.htm`**. Un `.md` no
se abre con doble clic en el equipo de quien lo recibe —se abre en el Bloc de notas con las tablas
rotas— y **el LEEME es la unica pieza que garantiza que se lea la mitad de arriba**. Los dos salen de
la **misma cadena en la misma corrida**, asi que no pueden divergir, y antes de cerrar el `.zip` se
cuentan las filas de tabla del uno contra el otro.

> ✅ **El renderizador NO hereda los dos defectos medidos del conversor a Word** (skill `entregar` §2):
> respeta `\|` dentro de una celda —la fila de `ESC:<ROJO\|VERDE\|AMBAR\|?>` se perdia entera— y **no
> aplasta un bloque `>` en un parrafo**, que es lo que machaca los huecos de respuesta y los pasos
> numerados. Se dice aqui porque el conversor a Word **sigue teniendo los dos**.

> ✅ **Control negativo corrido antes de conectarlo** (`CLAUDE.md` §6.bis): robandole **una** fila de
> tabla al `.htm`, el generador **ABORTA**, y por la comprobacion de filas —no por otra—. Verde sin
> defecto, rojo con defecto. Y la auditoria del `.zip` se corrio **con un script que no importa el
> generador**: un arnes no se verifica a si mismo.

> ⚠️ **Lo que este `N-x` NO es: un entregable.** Es la herramienta que PIDE una tarjeta cargada, no
> una que la acerque (`CLAUDE.md` §8). El paquete que sale sigue llevando `_SIN_BANCO` en el nombre y
> `🛑 NO` en su seccion 2, y lo seguira llevando hasta que alguien lo cargue en un equipo.

### 3.13 · 🟢 `N-154` CERRADO — el `$ALARM` cabia por VALOR y no por BUFFER, y lo que se perdia era la HORA

**Cerrado el 08/09. Y lo que mas vale de aqui no es el arreglo: es que el sintoma escrito
durante un mes era falso en su mitad util.**

Lo publicado era *«trunca en silencio, no casa el CRC, y la app la tira entera: el sintoma
es el equipo se callo, que manda a mirar el cable»*. **Medido al arreglarlo: el CRC SI
casa.** `enviarTramaConCrc()` calcula el checksum **sobre el payload ya truncado**, asi que
la trama sale bien formada, la app la valida y la acepta. **Lo que falta es el final, que es
el valor de `HORA:`** —y en el Esclavo ademas parte de `ACCION:`—. Una alarma que la app
tira se nota; **una alarma sin hora se archiva y nadie la mira**, que es exactamente lo
contrario de para lo que existe una Caja Negra.

**El peor caso, POR BUFFER —lo unico que `snprintf` garantiza— y compuesto de verdad:**

| | Maestro | Esclavo |
|---|---|---|
| parte fija del formato | 49 | 49 |
| `EVENTO` (el literal mas largo, `CAM_PEGADA`) | 10 | 10 |
| `CAUSA` ← **el defecto** | **39** | **39** |
| el tramo del enlace | 31 (`tramo[32]`) | **44** (`tramo[45]`) |
| `ACCION` (`CAMBIO_A_AMBAR`) | 14 | 14 |
| `HORA` (`horaBuf[16]`) | 15 | 15 |
| **total** | **158** | **171** |
| lo que guardaba `payload[144]` | 143 | 143 |

**El arreglo NO fue agrandar el payload, que es lo que la regla prohibe: fue ACOTAR DONDE
SE PRODUCE.** Los dos `causa[]` aportaban 39 caracteres por un numero redondo —`char[40]` y
`char[28]` escritos a ojo— y bajan a **19**:

- `causa[sizeof("SILENCIO_99999ms")]` en `Maestro/src/coordinador.cpp` y en
  `Esclavo/src/main.cpp`, **con un `static_assert(SFTY6_SILENCIO_MS <= 99999UL)` al lado**:
  la desigualdad se **recalcula desde el C++** en vez de explicarse en un comentario, asi
  que el dia que el umbral pase de cinco cifras **esto no compila**, que es lo que tiene
  que pasar (`CLAUDE.md` §4).
- `causa[sizeof("CAM_C_CONTACTO_FIJO")]` en los `botones.cpp` de las dos puntas.

Con eso el Maestro baja a **138 y cabe en `payload[139]`**. 🔴 **El Esclavo NO, y el motivo
es real y no se puede apretar:** su tramo son 44 caracteres contra 31, porque lleva **tres
contadores de protocolo con `%lu`** —numeros libres, sin rango que prometer, acotados a su
tope de TIPO—. Se queda en **151**, y ahi si se dimensiona el buffer **a la cota derivada**:
`payload[152]`. **8 B de PILA, no de flash.** Coste total medido con dos pasadas completas:
Maestro **58.048 → 58.088 B** (+40 B, 88,6 %); Esclavo **sin cambio** (45.152 B).

> 🔴 **Y AL MEDIRLO APARECIO LA SEGUNDA MITAD, QUE NADIE HABIA CONTADO: CON EL DEFECTO
> DENTRO, EL PEOR `$ALARM` TAMPOCO CABIA EN `tramaCompleta[160]`.** 158 + `*XX\r\n` = 163
> contra 159. Ahi truncar **si** parte el cierre del checksum, el otro extremo descarta la
> trama y **la alarma desaparece entera**. O sea que los dos sintomas —el de la hora perdida
> y el del silencio— **estaban vivos a la vez**, cada uno en un buffer distinto, y el
> publicado mezclaba los dos en uno solo que no era ninguno.

#### La parte que vale para el siguiente: se retiro una EXENCION midiendo, no borrandola

`esp32_07_presupuesto_bytes` llevaba escrito, al lado de la cuenta del tramo, **por que no
media el `$ALARM` entero**: *«el arreglo esta en ficheros que este cambio no toca, y un
instrumento que falla por algo que nadie puede arreglar desde aqui es un `FALLA` permanente,
que es lo que `CLAUDE.md` §3 prohibe»*. **Esa exencion era CORRECTA cuando se escribio** —y
por eso el pack estuvo verde un mes sabiendolo—. **Deja de serlo el dia que los `causa[]` se
acotan:** desde ahi el rojo ya se puede apagar construyendo, y un hueco que nadie mide deja
de ser prudencia para ser **un defecto con permiso**.

**Lo que hubo que construir para retirarla, y es lo que faltaba:** tres de los cinco campos
—`EVENTO`, `CAUSA` y `ACCION`— son **parametros `const char*`**, asi que su ancho **no esta
en `bluetooth.cpp`: esta en quien llama**. El pack ahora **censa el directorio `src/`** —no
una lista escrita a mano, que se queda corta el dia que alguien anade un `.cpp` con una
alarma nueva (`CLAUDE.md` §5)— y **sigue los saltos**: la alarma de camara no se emite donde
estan sus literales, sino que pasa por `camara_alarmar()`, donde `evento` **ya es un
parametro**. Sin ese salto el pack **ABORTABA**, que es lo correcto; con una estimacion en
su lugar habria medido de menos, que es lo que trunco el `$ALARM` de `N-108`.

> ✅ **Acreditado inyectando el defecto REAL en el `.cpp` real**, no solo con cadenas
> sinteticas: devolviendo `causa[40]` a `coordinador.cpp`, el pack cae de **37/37 a 35/37**
> y publica **158**, que es la cifra historica clavada. Restaurado desde copia tomada ANTES
> y verificado por `sha256` (`CLAUDE.md` §6.bis).

> ⚠️ **Lo que este cierre NO toca, dicho para que no pase por cobertura:** el margen de los
> dos payload es **CERO a proposito**, y esta escrito al lado de cada declaracion. No es
> holgura olvidada: es la cuenta cuadrada, y quien la rehace en cada corrida es el pack. El
> dia que alguien alargue un literal de `EVENTO` o anada un campo, **falla ahi antes de
> truncar en la calle** — que es justo lo que no ocurrio durante el mes anterior.

### 3.14 · 🎯 EL ORDEN DE DESARROLLO AL 08/09 — lo que queda, y por que en este orden

**Esta tabla NO duplica nada: apunta.** El porque de cada linea vive en su apartado, y aqui solo esta
el ORDEN y lo que lo justifica. Se reescribe entera cuando cambie; no se le anaden filas al final.

> 🎯 **11/09: lo que queda, medido contra el fuente, esta en la tabla de §0 por grupos** (teclado,
> decision, cobre). Esta tabla conserva el ORDEN y su porque, con cada fila marcada.

> ✅ **AL CERRAR EL 08/09: el paso 1 esta HECHO ENTERO** —`D-24`, las dos mitades— **y el paso 2
> (`D-23`) esta DESBLOQUEADO y sin empezar.** Lo demas de la tabla sigue igual. Y el paso 0 —la
> medida del riel de 3,3 V, que es gratis— **sigue sin constar recorrido**, ~~que es lo unico que de
> verdad frena todo lo demas~~ *(11/09: no frena lo demas — ver el recuadro de abajo)*.

> ~~🔴 **EL PASO 0 NO ES DESARROLLO, Y VA DELANTE DE TODO: medir el consumo del riel de 3,3 V del
> Maestro averiado, en frio y con fuente limitada en corriente** (`BLQ-3`, `N-116`). Es **gratis**, no
> consta recorrido, y **de el cuelga todo lo demas**: con la tarjeta muerta, nada de lo que sigue se
> puede ejercer en cobre — y **lo que no se ejerce en cobre no esta terminado**, por muy verde que
> este la compuerta. **Escribir mas codigo mientras esto no se mide es acumular deuda sin banco.**~~
> 🔴 **FALSO en su premisa, medido el 11/09:** el Maestro averiado es **la placa de la sesion 1**; el
> 04/09 se reprogramo otra como Maestro (`roadmap_hist.md` `N-126`, `SEM-179DB0-M`) y **esa corrio V9
> en el Sisga el 10/09**. O sea que **si hay donde ejercer en cobre**. Lo que sigue en pie, y es lo que
> importa, es la segunda mitad: **lo que no se ejerce en cobre no esta terminado**, y la sesion de
> banco de §5 es la que falta.

| # | que | por que va aqui | que arrastra |
|---|---|---|---|
| **0** | 🔴 **11/09 — ENTRAN DOS POR DELANTE, las dos del Sisga (§3.16):** **(a)** que `AMBAR_EMERGENCIA` sin PIN avise al Maestro como la puerta con PIN, con el pack que mire las DOS puertas —toca el ambar y es el candidato del DAR PASO—; ~~**(a.bis)** que `GO_GREEN` sea idempotente en el Esclavo (§3.16, DAR PASO), que es el que mejor casa con el banco del 04/09 y con el Sisga;~~ *(**(a.bis) CONSTRUIDO el 11/09 en `63d6964`**, en `main`)* ~~**(b)** la siembra periodica de `A-15`, decidida el 08/09 y sin una linea *(11/09: EN CONSTRUCCION en un worktree, fuera de `main`)*~~ *(**(b) CONSTRUIDO el 11/09 por la tarde en `68dd2c5`**, en `main`, con la cadencia de `D-26`)*: cierra la deriva del HSI entre visitas y el salto de los 49,7 dias —**mientras el `J17` hable**: con el `J17` mudo no hay siembra, `N-162` `H1`— · ~~*(a) sigue abierto el 11/09: §3.1 fila 8*~~ *(**(a) CONSTRUIDO el 12/09 en `913c29c`**, en `main`, con los tres instrumentos que lo dejaban pasar arreglados: §0 fila 1.2. **Los tres entraron sin banco y sin tarjeta**)* | **(a)** es una linea de firmware y un pack; **(b)** vuelve verdadera la premisa sobre la que se degrado `D-22` | Esclavo + pack · ESP32 + las dos puntas. **Ninguno autoriza nada sin banco** |
| ~~**1**~~ | 🟢 **`CAM_CIEGA` — HECHO ENTERO el 08/09**, las dos mitades (`D-24`, §3.15) | 24 h de paso abierto en las dos puntas, la app, 7 documentos y la APK recompilada · **y el aviso del Modo Inteligente, que salio CERO FIRMWARE** | — |
| **2** | 🔴 **`D-23` · la pantalla propia del poste 2**, por la via `$EVENT` | **Decidida el 07/09 y con la via YA elegida** (`A-14`). Es una de las tres que mantienen en rojo `decisiones_01_anclas`, y **la unica de las tres que se destraba con teclado** | `$EVENT` nuevo en el **Esclavo** + pantalla en **app.js** + **recompilar la APK**. ~~⚠️ **Antes: confirmar que el indice de `A-14` manda sobre su cuerpo** (§3.2)~~ *(hecho el 08/09 en `e6bfe76`: el cuerpo de `A-14` lleva la decision)* · 🔴 *sigue sin una linea el 11/09* |
| **3** | 🟠 **El checksum de la SUBIDA** (§3.1 fila 3) | `procesarComando()` **no lee el `*XX`**: un bit cambiado dentro de `SET_TIEMPOS` o `SET_RTC` **se obedece**. Va detras de 1 y 2 porque **no esta decidido** y porque SPP ya lleva su propio control de errores por debajo — pero es lo unico de esta lista que puede mover una luz por un bit | las dos puntas + el pack que lo mida. **Y su control negativo tiene que ser una trama con el CRC malo que HOY se obedece** |
| **4** | 🟠 **`reloj_diagnostico()` en el Esclavo** (§3.1 fila 5) | Porte **mecanico** desde el Maestro; ya tiene los ingredientes. Sin el, quien sube 5 m al poste del Esclavo **no puede distinguir `lseOn=0` de `lseRdy=0`** y baja sin saber que pieza mirar | una punta. Es la mas aislada de la lista |
| **5** | 🟠 **El caso borde de rojo de `validateTiempos()`** (§3.1 fila 4) | Una linea. El caso de verde mide el limite y **el de rojo no**: `validateTiempos(3, 1, 25)` esta uno POR DEBAJO del borde, asi que si alguien afloja `r < 3` a `r < 2`, ese sigue pasando | `test_unitarios_app.js`. **No** obliga a recompilar la APK: es prueba, no `www/` |
| **6** | 🟡 **`D-22` · `Y1` como latido del micro** | **VA EL ULTIMO Y VA SOLO** *(11/09: «el ultimo» choca con el «VA PRIMERO» de §3.4.quater — `A-16`, abierta)*, y no por su beneficio sino por su MODO DE FALLO: si `Y1` no oscila, el `_Error_Handler` del nucleo es `noreturn` + `while (1)` y **la tarjeta queda a oscuras, sin luces y sin reiniciarse**. §3.4.quater | **no se carga sin una tarjeta delante.** No es teclado: es una sesion de banco |

> 🛑 **LO QUE NO ESTA EN ESTA TABLA A PROPOSITO, porque no lo cierra nadie escribiendo:** `BLQ-3`
> (tarjeta muerta *— la de la sesion 1; la `179DB0` funciona*), `BLQ-6` (**nada de lo arreglado tras la cinta del 05/09 ha pasado por una
> tarjeta** — y ahi entran `N-150`, `N-151`, `N-152`, `N-154` y `N-160`; *11/09: todos iban en
> `7ff7d12`, el firmware del Maestro del Sisga, pero su cinta solo ejerce `N-150` y el reloj de
> `N-160`, del lado del Maestro y por telemetria — §5*), `BLQ-5` (entradas desnudas
> y los 12 V de `J16` p1), `BLQ-4` (el anuncio Bluetooth del ESP32) y `BLQ-2` (`Y2`). **Confundirlos
> con los de arriba es como se acumula un `20/20` que no acerca una tarjeta.**

> 🟡 **Y las que esperan una DECISION tuya, no codigo:** ~~la lectura de `A-14` (§3.2, barata y
> desbloquea el paso 2) · el regimen electrico de la entrada de alarma de `D-14`, que es **un
> multimetro**~~ *(11/09: las dos cerradas el 08/09 — `A-14` resuelta en indice y cuerpo, y el
> multimetro de `D-14` retirado, §3.11.bis; lo que queda de `D-14` es **que canal se gasta**,
> §3.11.ter)* · `A-16` (el sitio de `D-22`) · ~~el conflicto de `J14` (§0, 2.1)~~ *(cerrado por `D-27`, 11/09)* · `FW-N53`, si se redefinen los gestos del mando —hoy Auto es `A·A·A` y Ambar `B·B·B`—,
> que cambia el Manual 1, el Manual 3 y el adiestramiento · y la **matriculacion por ID de
> Bluetooth**, aplazada por ti a despues del banco porque cambia el contrato de la radio en las dos
> puntas.

### 3.15 · 🟢 `D-24` — `CAM_CIEGA` a 24 h, y las TRES formas en que una lista de alcance se queda corta

**Construido el 08/09 en `9550c57`.** La decision, su cuenta y su mitad pendiente viven en `D-24` de
[`DECISIONES.md`](DECISIONES.md) y **no se copian aqui**. Lo que si vive aqui es **como se descubrio
que el alcance escrito era falso**, porque esa forma se repite.

**La fila decia: *«toca las dos puntas + `camara_03` + 5 documentos»*. Medido al ejecutarla:**

| | lo que faltaba | por que no se veia |
|---|---|---|
| **1** | **Son SIETE documentos, no cinco**, con **18 lineas** que llevan la cifra | nadie la habia contado: el numero *«5»* venia de recordar, no de un `grep` |
| **2** | **Otras OCHO lineas la llevaban DERIVADA** —*«del orden de 12 h de reloj»*— **en lineas que no nombran `CAM_CIEGA`** | un patron que busca el nombre de la constante **no ve su consecuencia**. Es `CLAUDE.md` §7.1 en su forma cara: el patron no encontro, y eso no es que no haya |
| **3** | **Tres lineas mas dicen `CIEGA` sin decir `CAM_CIEGA`** | hicieron falta **TRES barridas** y cada una encontro lo que la anterior no podia ver |
| **4** | 🔴 **La APP no estaba en la lista**, y decia **dos veces** *«lleva HORAS sin una sola deteccion»* | la lista se escribio pensando en firmware y documentos |
| **5** | 🔴 **HAY CUATRO COPIAS DE `app.js` y se edito una** | `www/`, `App_Semaforo/`, `android/assets/public/` y la de `build/`. Las tres primeras tienen que quedar **identicas por md5**, y esa es la invariante que la skill `entregar` manda comprobar **ANTES** de compilar |

> 🎯 **LA DECISION DE DISENO QUE SALIO DE AHI, y vale mas que el numero: en la app NO SE
> SINCRONIZA LA CIFRA, SE QUITA.** `app.js` no puede derivar de `CAM_CIEGA_MS` —vive en otro
> lenguaje y en otro binario—, asi que **cualquier cifra copiada a ese lado nace caducada**. Ahora
> dice *«demasiado tiempo de paso abierto»* y **no envejece**. Un numero que un instrumento no puede
> recalcular no se copia: se retira.

> ⚠️ **Y lo que esto deja escrito para la proxima: una lista de alcance es una AFIRMACION SOBRE EL
> ARBOL, y envejece igual que cualquier otra.** Se recuenta con `grep` antes de ejecutarla —por el
> nombre **y por su consecuencia**—, no se lee. Aqui la diferencia entre lo escrito y lo medido fue
> de **5 documentos contra 7, y de 18 lineas contra 29**. **Subido a `CLAUDE.md` §14**, porque sigue
> siendo cierto si el firmware cambia entero.

#### 3.15.bis · 🟢 La SEGUNDA mitad de `D-24` — y por que salio CERO FIRMWARE

**El responsable eligio la via A —solo el aviso— el 08/09, y la medida que la decidio vale mas que
el aviso**, porque redujo la tarea a una fraccion de lo que parecia:

| lo que parecia | lo que estaba medido |
|---|---|
| *«el Modo Inteligente se declara AVERIADO»* suena a cambio de comportamiento | 🟢 **La parte vial YA estaba construida**, y `modo_inteligente.cpp` lo lleva escrito y razonado: *«CON LAS CAMARAS MUERTAS ESTE MODO SE COMPORTA EXACTAMENTE COMO EL AUTOMATICO … degrada al comportamiento conocido, no a uno raro»*. **Una camara ciega no cambia ni una luz** |
| habria que publicar el dato | 🟢 **Ya viajaba**: `CAM:` va en el `$STATUS` de las dos puntas desde `e3a21ec` |
| lo que faltaba de verdad | 🔴 **Que alguien CRUZARA las dos cosas.** La app tenia `state.modo` y el estado de camara **en dos sitios sin cruzarse nunca**, y `camara_estado()` tiene **un unico llamador en todo el firmware**: el `$STATUS` |

> 🔴 **POR QUE SE DESCARTO LA VIA B —que el equipo se pasara solo a `AUTOMATICO`—, y no fue por
> coste: la maquina decidiria operar en un modo que NADIE PIDIO**, que es exactamente lo que prohibe
> la barrera de salidas (`CLAUDE.md` §2). **Y ademas no compraria nada**, porque el comportamiento ya
> es el del Automatico. Una barrera que no cambia nada y rompe una regla no es una barrera: es ruido
> con permiso.

**SON DOS AVISOS Y NO UNO, y esa es la parte que no estaba en la decision.** `D-24` hablaba solo de
`CAM_CIEGA`, pero **las dos averias fallan en direcciones opuestas**: la ciega **quita la demanda**
—el modo acaba en el SUELO, o sea Automatico— y la pegada **la afirma siempre** —MANTIENE hasta el
TECHO, el doble del configurado, aunque no pase nadie—. **Meterlas en una frase mandaria al tecnico
a buscar el sintoma contrario al que tiene.**

> ✅ **Cuatro comprobaciones nuevas en el arnes de DOM —235 -> 239— y DOS de ellas son controles que
> exigen que el aviso NO salga**: en `AUTOMATICO` con la camara rota, y en `INTELIGENTE` con la
> camara sana. Sin esas dos, **un aviso pegado siempre pasaria las otras dos igual de bien**.
> Acreditado inyectando el defecto en el `app.js` real: quitando el aviso, la suite cae a
> **237 | 2 FALLAS** —caen las dos positivas y **los dos controles siguen verdes, que es lo
> correcto**—. Restaurado por `sha256` (`b826d568…`).

### 3.16 · 🔴 `N-162` — El Sisga (10/09): la PRIMERA vez que V9 toca una calle, y la rama que salio de ahi, VALIDADA POR EL DIFF el 11/09

**Lo que es nuevo y cambia la lectura de todo este fichero:** el 10/09 un Maestro con `SERIE:179DB0`
corrio **en campo, en El Sisga, con firmware V9** — su `$STATUS` lleva `CAM:`, que no existe antes de
`e3a21ec` (05/09). O sea que **§0 («ninguno ha entrado en un poste») dejo de ser cierto ese dia**, y
~~**no consta QUE commit estaba cargado** en ninguna de las dos tarjetas~~ *(11/09: en el **Maestro**
consta — `7ff7d12`, el paquete del 08/09, y despues se probo el de `b354fe9`; lo que no consta es
el del **Esclavo**)*. `CLAUDE.md` §7: *antes de
llamar defecto a una medida, mirese que firmware estaba dentro.* Y choca con §6.1: **una Maestro
funciono en campo mientras este fichero da la Maestro por muerta** — ~~hay que contrastar si `N-116` era
otra placa~~ 🟢 **CONTRASTADO el 11/09: ERA OTRA PLACA.** `roadmap_hist.md` `N-126` (sesion 2 del
banco, 04/09): *«se corrio con una sola tarjeta —la Maestro de la sesion 1 sigue con el corto y se
descarto entera— reprogramada como Maestro»*, y esa se anuncio **`SEM-179DB0-M`**; la serie sale
del UID del STM32 (`identidad.cpp`). **La `179DB0` es la segunda placa, funciona y es la del Sisga;
la de `N-116` es la de la sesion 1.**

El funcional (ITvial) reporto tres cosas: la hora en `00:00:00`, DAR PASO que deja al Esclavo en ambar
intermitente, y *«solo me envias una camara por poste»* (queja que `D-25` zanjo el 11/09: son DOS por
poste). **Otro agente las dio por resueltas el mismo dia** en `fix/campo-sisga-rtc-darpaso` (12 commits, `c51cc85`..`669e87c`). **Revisado por el DIFF el
11/09**, con tres auditorias de solo lectura y **cada hallazgo grave reproducido a mano** antes de
escribirlo aqui. El parte original se conserva en la historia de git (`669e87c:roadmap.md`).

| lo que se entrego el 10/09 | veredicto del 11/09 |
|---|---|
| **STM32 · los getters de hora** miran primero la base sembrada (`tBaseMillis > 0`) y solo sin ella el RTC (`c51cc85`, las dos puntas, identicas) | 🟢 **SE QUEDA.** Es `D-20` literal —*«al STM32 no se le pregunta nunca»*— y explica el `00:00:00`: con `LSERDY` arriba y el contador sin contar, los getters viejos leian ceros **con `horaValida` en `true`**, que es `N-144` otra vez. ⚠️ **Ningun instrumento la ejerce**: el banco dio `1249/1252` antes y despues, y `grep tBaseMillis` sobre `Simulaciones/` y `Validacion_*` da **cero**. ⚠️ **Y lleva al HSI a las placas con `Y2` sano** (de 20–50 ppm a 10.000–25.000) **mientras `A-15` no exista** — defecto **B** abajo |
| **STM32 · exigir `h\|m\|s != 0` para creer al RTC al arrancar** | 🟡 **se queda**: falla hacia lo seguro (una medianoche legitima al arrancar sale sin hora). **Pero la causa que la justificaba era imposible** —con `Y2` muerto `reloj_setup()` sale antes de esa linea— y **no caza un contador congelado en otro valor** (defecto **D**) |
| **ESP32 · el puente sella tambien `HORA:00:00:00`** | 🔴 **REVERTIDO el 11/09.** Su propia cabecera lo prohibe —*«SOLO SE TOCA EL HUECO … un cero que parece una hora es peor que un hueco»*—: tapaba en la app, con la hora buena del `DS3231`, el estado del que cuelga la autorizacion del Degradado. **El simulador del puente daba `101/101` porque ningun caso contiene `HORA:00:00:00`** |
| **App · `state.hora` desde el `$ACK` de `SET_RTC`/`LEER_RTC`** | 🔴 **RETIRADO.** Ese acuse es `NODE:PUENTE` —el `DS3231`— y `state.hora` es la del CONTROLADOR: pintaba una con la otra. **Se queda** el ambar `00:00:00 · NO SINCRONIZADO`, que es correcto |
| **App · DAR PASO**: toast *«despejando via (15s todo-rojo)»* y `ACK_TEXTO` *«15 s … ambar de transicion (4 s) y abrira en VERDE»* | 🔴 **RETIRADO.** (1) el toast salia **al pulsar**, antes de cualquier `$ACK`; (2) **el despeje NO son 15 s**: `coordinador_configurar()` lo pone desde el respaldo, de `DESPEJE_SEG_MIN = 10` a 90 — `CLAUDE.md` §14; (3) el `$ACK` solo dice que el coordinador del Maestro estaba en reposo, **no que la otra punta vaya a abrir** (`CLAUDE.md` §2). Ahora dice eso, y manda mirar **las dos cabezas** |
| **App · cabeceras de diagnostico y compartir** (`0cd7fd2`) | 🟢 **se queda.** Los campos existen en `state`. La APK `IOT_VIAL_Semaforos_2026-09-10_b354fe9_SIN_BANCO.apk` **es una recompilacion real**: `SHA-256` `3bfd9e61…`, distinto del 08/09, y su `app.js` identico por md5 al de `0cd7fd2`. ⚠️ **Tras el 11/09 ya no es la de HEAD**: lleva los textos de DAR PASO retirados. *(El parte citaba una `…_c51cc85_…apk` que no existe.)* |
| **Guias `Camaras_Sisga_4x.html` y `Guia_Instalacion_Camaras_4x.html`**, su hueco en el empaquetador y en `ARQUITECTURA.map` | 🔴 **RETIRADAS a `05_Funcional/historico/` con banda de RETIRADA, fuera del paquete**, y `ARQUITECTURA.map` vuelto a lo decidido. Tabla de abajo |
| **Las actas del 10/09** | ⚠️ las que decian *«3 packs en FALLA»* se tomaron con el **arbol sucio**, y los dos de mas eran `documentos_0x` por el cambio de fecha. Con el arbol limpio el 11/09 sobre `669e87c`: **lo mismo que el 08/09** |

#### Lo que las guias le decian al instalador, contra el firmware y la spec

| afirmacion | veredicto | evidencia |
|---|---|---|
| la «camara 2» **protege la pluma** / vigila que no haya un coche debajo | 🔴 **FALSO** | `escribirPines()`: `(verde && !testLedsActivo) \|\| estado == S_FALLO`. **Ninguna camara entra**: el veto es `A-1.bis`, sin construir. **La pluma baja con un coche debajo** |
| la pluma sube **solo con verde** | 🔴 **FALSO** | sube con **cualquier** `S_FALLO`: Modo Ambar, `AMBAR_EMERGENCIA`, orfandad SFTY-6. **Un poste recien montado que aun no enlazo con el otro sube la pluma sin verde** |
| las dos entradas hacen cosas distintas («demanda 15–30 m» / «presencia 0–8 m») | 🔴 **FALSO** | `botones.cpp` recorre `CAM_J16[2] = {CAM_C_PIN, CAM_D_PIN}` con el mismo bucle |
| **4 camaras, 2 por poste** | ~~🔴 **choca con `D-13`** (una por poste), con `17_` §1.7 (p12 vacio a proposito) y con `ESTADO.md` C1 (las 4 camaras se **revocaron el 28/08**)~~ 🟢 **DECIDIDO el 11/09 por el responsable: `D-25`** — ver abajo | ~~**no se elige aqui: es del responsable**~~ lo eligio el: *«mantener estas conexiones como definitivas»* |
| la app confirma las dos camaras (`CAM: ? -> OK`) | 🔴 **FALSO para la segunda** | `camara_estado()` salta el pin que nunca dio flanco: con la primera viva sale `OK` aunque la segunda este muerta. **Se podia firmar «apto» con una camara muerta** |
| «dos entradas optoacopladas», «aislamiento galvanico» (este apartado, el 10/09) | 🔴 **FALSO** | netlist: `J16.10 -> R67 C28 U1.27`, sin opto. Es `N-120` |
| en corte de energia la pluma baja «por gravedad o muelle» | ⚪ **SIN FUENTE** | lo decide la centralita de la barrera, que tiene su propia alimentacion. La «especificacion de compra con muelle» que citan cuatro documentos **no esta** en `15_Lista_de_Compras` |
| angulos 15–20° / 35–45°, alturas, distancias | ⚪ **SIN FUENTE** | solo aparecen en las dos guias, y dan alturas distintas entre si |
| `J15`: p1 12 V, p2 drenador de `Q10` IRLZ44N, `U15` TLP127, `D30` 1N4148, `D29` | 🟢 **CIERTO** | medido el 11/09 sobre el `.kicad_pcb`. **Pero p2 no es «GND»**, como rotulaba el SVG: con `Q10` abierto esta a ~12 V |
| `J16` p1 lleva 12 V y se tapa · `J14` es entrada | 🟢 **CIERTO** | pero la guia admitia camara en `J14`, ~~y `A-2` lo reservo al **fin de carrera**~~ *(reserva derogada por `D-27`, 11/09: `J14` libre)* · 🔴 **y el firmware lo sigue leyendo como camara** (`CAM_DEMANDA_PIN` = `PB0` en las dos puntas): ~~**conflicto abierto, §0 fila 2.1**~~ → 🟢 **`D-27` (11/09): `J14` libre, sin cablear; el fin de carrera no se instala** |

> 🔴 **LA GUIA SI LLEGO AL SISGA, y no por el paquete: por WhatsApp, el 10/09 a las 15:32**, junto con un
> resumen de configuracion para "las 4 camaras". Las dos versiones de esa hora (`48f7fbb`, `86683e8`)
> ya llevan *«cuida la pluma / protege pluma»*. El instalador dijo que la guardaba en el chat y que ese dia
> solo configuro **una camara del Maestro**. **Hay que decirle las dos falsedades antes de que monte las
> demas.** Y el ultimo paquete que probo es `b354fe9`, que lleva el sello del puente que aqui se revirtio:
> **campo y `main` difieren en el ESP32**, y el siguiente paquete tiene que cargar las tres tarjetas.

#### 🎯 `D-25` — las CONEXIONES de aquella guia, definitivas (11/09, el responsable)

**Decidido el 11/09: *«mantener estas conexiones como definitivas»***, sobre la guia revisada con el
el 10/09. La fila vive en [`DECISIONES.md`](DECISIONES.md) y **no se copia aqui**; en una linea:
**cuatro camaras, dos por poste** —camara 1 entre `J16` p9 (3,3 V) y p10 (`CAM_C`), camara 2 entre
p11 (3,3 V) y p12 (`CAM_D`), por el contacto seco de su salida de alarma— y **la talanquera en `J15`**
(p1 12 V, p2 drenador de `Q10`) **por rele a la entrada `OPEN` de la centralita**. Deroga de `D-13`
**solo** *«una camara por poste / `p12` vacio»*.

> 🔴 **Lo que se decide son las CONEXIONES, no la guia.** Sus dos afirmaciones de seguridad —la
> camara que «protege la pluma» y la pluma que sube «solo con verde»— **siguen siendo falsas** (tabla
> de arriba), y la guia **sigue RETIRADA**. Y lo que el que cablea tiene que saber, medido en
> `b79d904` y escrito en la fila: **las dos entradas hacen lo mismo** (`CAM_J16[2]`, un solo bucle),
> **ninguna protege la pluma** (`A-1.bis`, sin construir), y **una segunda camara que nunca dio flanco
> no se detecta sola** —`vigilante_tick()` y `camara_estado()` la saltan—, porque esa exencion se
> escribio para un `p12` vacio a proposito y **con `D-25` pierde su motivo**.
>
> **Lo que deja pendiente** (tabla de §0, filas 1.9 y 1.10): el vigilante de la segunda camara, y
> alinear `17_` §1.7, el Manual 9 y `ARQUITECTURA.map`, que dicen `p12` vacio (`README.md` ya se
> alineo en `648b62f`). Y el
> banco nota ya su efecto: `decisiones_01_anclas` acusa `D-25` dos veces —sin ancla en el fuente y
> sin nombrar en ningun manual—, **y ese rojo es correcto**: se apaga documentando y anclando, no
> quitando la fila.

#### 🎯 `D-27` — lo que la guia fija, cerrado (11/09, el responsable)

**Las cuatro camaras compradas; `J14` libre y sin cablear (el fin de carrera no se instala); la
configuracion de las camaras, la del manual del modelo; la talanquera por rele a la centralita.** La
fila vive en [`DECISIONES.md`](DECISIONES.md). Alineada el 11/09 por la tarde en el Manual 9, el
manual del modelo, `15_`, `17_`, las dos guias (en la del Sisga, **solo su `J14`**), los Manuales 1,
2 y 13, los de `04_Manuales/`, los dos `README.md` y `ARQUITECTURA.map`. **Lo que dejo a la vista
esta en §0, filas 2.1.bis y 2.1.ter** —la zona, el filtro y el contador de la fase 1, «la ficha», y
la guia del Sisga que sigue diciendo «no filtrar»—. **Y una medida de campo que los manuales no
tenian:** la casilla `Trigger Alarm Output` **existe** en este modelo (§3.8, 10/09), y el Manual 9 y
el manual del modelo seguian diciendo *«vaya esperando que NO»*; corregido.

#### DAR PASO — el diagnostico del 10/09 NO se sostiene

El parte decia que el operario confundio el **ambar FIJO** de 4 s con uno intermitente, y lo «probaba»
con una trama. **Esa trama es de `MODO:AUTO`**, y su `ESC:VERDE` es lo que **el Maestro cree** —
`quienVerde`, que se pone con el primer `ACK_GREEN`, y el Esclavo lo manda **todavia en ambar**—, no lo
que el Esclavo tiene encendido. **No prueba nada del evento.** Y es **la tercera vez** que este sintoma
se explica con un ambar que no era intermitente o que no era del Esclavo (banco del 04/09,
`N-140`/`N-141`, `N-147`). `CLAUDE.md` §7: **la foto de campo es una medida; cuando choca con el
razonamiento, el sospechoso es el razonamiento.**

> 🔴 **EL CAMINO QUE SI PRODUCE LO QUE VIO EL OPERARIO — reproducido en el fuente el 11/09:** la app
> manda `AMBAR_EMERGENCIA` **sin PIN** (esta en `SIN_PIN`), y en `Esclavo/src/bluetooth.cpp` **solo la
> puerta CON PIN** llama a `protocolo_enviarPaquete(CMD_AMBAR_ESCLAVO)` —un unico llamador en todo el
> Esclavo—. **O sea que el aviso de `N-142` no lo ha disparado nunca la app**, y el comentario de encima
> —*«las dos puertas llevan el mismo bloque, letra por letra»*— es falso. Con el cerrojo puesto el Esclavo
> sigue contestando `PONG` y veta el `GO_GREEN` en silencio; el Maestro agota reintentos, cae a
> `C_FALLO` y **se autorrecupera dandose el verde a si mismo, tambien en Manual**. Resultado: **Esclavo en
> ambar intermitente con la pluma ARRIBA, y un cruce que no alterna.**
>
> ⚠️ **Que pasara asi en el Sisga es HIPOTESIS**: exige que alguien pusiera el ambar en el poste 2.
>
> 🟢 **12/09 — EL CAMINO YA NO EXISTE EN EL FUENTE (`913c29c`, §0 fila 1.2): las dos puertas avisan.**
> Lo de arriba se conserva porque es la cronica de como se hallo, y **su ultima linea sigue intacta: que
> esto pasara en el Sisga sigue siendo HIPOTESIS y lo seguira siendo hasta la cinta del Esclavo.** Cerrar
> el defecto no confirma la causa; solo la vuelve irreproducible con el firmware nuevo — que es
> exactamente lo que hace que la cinta importe MAS, no menos.

> 🔴 **Y EL CAMINO QUE MEJOR CASA CON LO QUE SE VIO, medido el 11/09 con la transcripcion del banco del
> 04/09 delante** —*«queda maestro en rojo y despues de 15 segundos esclavo quedan [en] rojo y ambar»* y
> *«la aplicacion dice: el cruce esta cambiando de fase, repita al terminar»*—. Esa frase es
> `$ERR,CMD:CAMBIAR_TURNO,DESC:EN_TRANSICION_REINTENTE`: **el Maestro seguia esperando el `ACK_GREEN`**.
> Dos piezas medidas en el fuente:
>
> 1. la rama `CMD_GO_GREEN` de `Esclavo/src/main.cpp` llama a `semaforo_iniciarTransicionAVerde()` **en
>    cada orden**, y esa funcion pone `S_AMARILLO` y **reinicia el reloj del ambar** (`tCambio = millis()`)
>    este donde este la luz —desde verde, incluso, lo devuelve a ambar—;
> 2. el Maestro reintenta `GO_GREEN` cada `TIMEOUT_ACK_MS = 3500` y el ambar dura `4000`.
>
> ~~Basta UN `ACK_GREEN` perdido~~ — **corregido por el agente que lo arreglo: un acuse perdido reinicia el
> ambar UNA vez (~7,5 s de ambar); para que el Esclavo no llegue a verde hacen falta VARIOS seguidos.** Medido
> en el arnes con 4 perdidos: **18.200 ms de ambar** contra los 4.000 de la Resolucion. Mientras dura, el
> Maestro sigue en rojo «en transicion» y la app dice «repita». Que se perdiera un acuse lo apunto el responsable ese dia (*«el reenvio y la propagacion
> de la antena del radio»*) y es hipotesis; **que el firmware no aguanta un acuse perdido esta medido**. El
> arreglo es que `GO_GREEN` sea idempotente en el Esclavo: si ya esta en la transicion a verde o en verde,
> se re-acusa y no se reinicia nada.
>
> ✅ **CONSTRUIDO el 11/09** (agente en worktree, integrado por el diff): con `S_AMARILLO` o `S_VERDE` la
> orden no toca la luz y se re-acusa; los vetos intactos. Bloque F nuevo en el arnes de las dos puntas
> (45 -> **51**): pierde 4 `ACK_GREEN` (F-a) y 2 + un reintento (F-b), con las constantes releidas del C++.
> **Visto fallar** con el defecto inyectado: 49/51 (ambar de 18.200 ms; un verde devuelto a ambar). Esclavo
> +12 B. ⚠️ **Lo que deja a la vista, y no mide nadie:** con acuses perdidos el Esclavo ya esta en VERDE
> mientras el Maestro espera; si el Maestro agota reintentos entra en `S_FALLO` (ambar intermitente, pluma
> arriba) **con el Esclavo en verde**. Esa ventana ya existia con la radio caida en mitad de un verde del
> Esclavo, y la comprobacion A9 del arnes excluye `S_FALLO` por nombre: **nadie la mide**. Pendiente de medir.

**La medida que lo distingue, y no pide tarjeta nueva:** el `$STATUS` **del propio Esclavo** en ese
momento —`ESTADO:FALLO COM` con `PLUMA:ARRIBA` es `S_FALLO`; `ESTADO:AMARILLO` con `PLUMA:ABAJO` es la
transicion—, su Diario de Ordenes (`AMBAR_EMERGENCIA` sin `CANCELAR_AMBAR` detras), la cinta del Maestro
(`$ALARM … REINTENTOS_AGOTADOS` unos 17 s despues del despeje). **La del Maestro ya esta, y no trae ni un
`REINTENTOS_AGOTADOS`**: ver la cinta, abajo. Falta la del Esclavo.

#### Lo que aparecio al auditar, y NO es de esta rama

| | que | estado |
|---|---|---|
| ~~**A**~~ | ~~🔴 **`AMBAR_EMERGENCIA` sin PIN no avisa al Maestro** (arriba)~~ 🟢 **CONSTRUIDO el 12/09 en `913c29c`** (§0, fila 1.2). ⚠️ **Sin banco y sin tarjeta**, y **no confirma la hipotesis del Sisga**: para eso hace falta la cinta del Esclavo | ~~**medido.** El arreglo es mecanico —la misma linea en la otra puerta— **mas el pack que mire las DOS puertas**, que `esclavo_07`/`08` dicen mirar y no miran~~ — **y «mecanico» se quedo corto**: la linea era mecanica, pero los `esclavo_07`/`08` no la habrian visto entrar por una sola puerta (uno deduplicaba por nombre, el otro no miraba `protocolo_`) **y el arnes de dos puntas ejercia una TRANSCRIPCION de la puerta buena escrita en su propio adaptador**. Se retiro la copia y se compila el `bluetooth.cpp` real; bloque **H**, 76/77 → 85/86 |
| ~~**B**~~ | ~~🔴 **La siembra periodica ESP32->STM32 de `A-15` (~5 min, `D-26`) NO EXISTE**: `grep INTERVALO_SYNC_MS ESP32_Expansion/src` -> nada *(11/09: EN CONSTRUCCION en un worktree; en `main` sigue sin existir)*~~ 🟢 **CONSTRUIDA en `68dd2c5`** (11/09 por la tarde): `siembra_revisar()` en el `loop()` del ESP32, cadencia `SIEMBRA_INTERVALO_MS = 300000UL`; el `grep` de la medida vieja sigue dando 0 porque el nombre cambio **a proposito** (`contrato.h`: *«Deja de llamarse INTERVALO_SYNC_MS a proposito»*). ⚠️ **Con el `J17` mudo no hay siembra y todo lo de la derecha vuelve a valer** —y peor, `H1`—, §0 fila 1.13 | **medido** *(el 11/09 a mediodia, sobre `b79d904`)*. Sin ella el Maestro corre con `millis()` sobre el HSI **desde el ultimo `SET_RTC` a mano**, y a los **49,7 dias** `(millis() - tBaseMillis)` da la vuelta: la hora salta atras 17 h 02 min 47 s, que en el ciclo de 120 s del Degradado son **47 s de desfase contra un Esclavo que no salta** —del orden de 17 s de verde contra verde por ciclo **si cae dentro de un Degradado**—. Con `c51cc85` alcanza a todas las placas. **Construir `A-15` lo cierra**, y ya esta decidida |
| ~~**C**~~ | ~~🔴 **`SET_RTC` puede mentir AL REVES**: el puente reenvia la linea al STM32 **antes y con independencia** de su `DS3231`; con un ano fuera de rango, `dia=0` u `OSF`, el telefono recibe `$ERR,NODE:PUENTE` **y el Maestro siembra esa hora y la propaga**~~ 🟢 **CERRADA EN EL FUENTE en `68dd2c5`**: el `SET_RTC` ya no cruza, y `siembra_ahora()` solo corre en la rama `RELOJ_OK` con el `DS3231` releido | medido por la auditoria, ⚠️ **sin reproducir a mano** — y ya no se podra con `main`: el camino no existe. `CLAUDE.md` §2 del reves |
| **D** | 🟠 **Con el contador del RTC congelado en un valor NO nulo, el limite de 48 h del Degradado no vence tras un corte**: `reloj_contadorSegundos()` devuelve el contador si `rtcOperativo`, y `respaldo_horasDesdeSync()` da 0 para siempre | **mecanismo medido; que el estado exista en una placa, HIPOTESIS.** El `00:00:00` del 179DB0 cae del lado seguro desde `c51cc85` (no reanuda); otro valor congelado, no. ~~Se mide con `CONSULTA RELOJ` dos veces, 10 s aparte: `cnt` tiene que avanzar 10~~ 🔴 **Esa pantalla no se alcanza** (11/09): `CONSULTA RELOJ` vive en `MODO_HORA`, que solo arma `menu.cpp` detras de `botonAceptar()` —`return false`— y la LCD se retiro (`D-17.bis`). **Hoy no hay forma NO destructiva de leer `cnt`** (§3.5 fila 5): `reportarBitsDelReloj()` solo sale tras un `REINICIAR_RELOJ` fallido, que borra hora y respaldo. Lo mas cercano, **deducido del fuente y sin ejercer**: tras un arranque y **antes de ningun `SET_RTC`** (`tBaseMillis == 0`), los getters leen el RTC, asi que dos `$STATUS` 10 s aparte con `HORA:` distinta de `--:--:--` y que **no avanza** serian el contador congelado. La lectura buena es teclado: §0, 1.7 |
| **E** | 🟠 **La guarda `D-21` del Esclavo sigue siendo inalcanzable** (§3.10.bis) y **`DECISIONES.md` la sigue llamando «guarda nueva y real»** | medido · *11/09: sigue en el fuente; en la fila `D-21` se anoto hoy la medida al lado, sin tocar la decision. §0, 1.4* |
| **F** | 🟡 **«Pluma ARRIBA en `S_FALLO`» no tiene fila en `DECISIONES.md`**: vive en un comentario de `semaforo.cpp` (cliente y PMT, 27/08) y en el Manual 1 (confirmado el 04/09) | a la tabla vinculante, por el responsable |

#### La cinta y el diario del Maestro (10/09, 12:06–12:26, firmware `7ff7d12`), LEIDOS ENTEROS

**Estan en `evidencia/2026-09-10_Sisga_179DB0_cinta_tramas.txt` y `…_diario_ordenes.txt`**, copiados de
lo que exporto la app, y **verificados: 308 tramas con checksum, 308 casan**. Es la primera medida de V9
en una calle. Lo que dicen, con el firmware que habia dentro:

| | lo que se ve | lo que significa |
|---|---|---|
| **La hora** | 5 `SET_RTC`. El puente contesta `$ACK…RESULT:OK` con la hora **releida** del `DS3231`, y `LEER_RTC` la da **avanzando** (12:17:31 → 12:18:52). El STM32 emite `SET_RTC_LO_ACUSA_EL_PUENTE` 4 veces —o sea que **la siembra ENTRO**— y aun asi publica `HORA:00:00:00` en ~~**las 150 tramas**~~ **los 253 `$STATUS` de la cinta, los 253** *(recontado el 11/09 con `grep`; el 150 era el recuento de «campo sin medida» de la ventana de 5 minutos de la cabecera)* | 🔴 con `7ff7d12` los getters leen **primero el RTC del STM32**, y en el 179DB0 ese RTC tiene `LSERDY` arriba y **no cuenta**: es `N-144` en la calle. **`main` lo corrige** (`c51cc85`: manda la base sembrada). El `DS3231` del puente **funciona en cobre** |
| 🔴 **El Degradado con `7ff7d12` en el 179DB0** | el Maestro se declara **en hora** con el reloj parado | **su autorizacion cuelga de esa bandera, y su fase saldria de un reloj que no avanza.** **No se usa el Modo Degradado en el Sisga con ese firmware** |
| 🔴 **El ESP32 se reinicio AL MENOS 3 veces** en 12:18–12:19 · ~~«5 veces en 97 s»~~ | 5 partes `EVT:ARRANQUE`: 3 × `CAUSA:OTRO_PERRO` y 2 × `SUBIDA_DE_TENSION`. ⚠️ **REFUTADO el mismo 11/09 que fueran 5 reinicios**: el parte se emite **una vez por CONEXION Bluetooth** (`vigilante_declarar()` rearma `parteEmitido` al caer el enlace), asi que un arranque puede anunciarse dos veces; los reinicios que la cinta obliga son 3 | `SUBIDA_DE_TENSION` es `ESP_RST_POWERON`: **no puede salir del software** —tension por debajo del umbral, o `EN` a 0—. `OTRO_PERRO` es `ESP_RST_WDT`, el perro del dominio RTC, que en esta compilacion **solo muerde en un arranque o rearranque que no llega a `setup()`**; con `ARRANQUES:1` (la RAM del RTC borrada) apunta a una caida de tension (HIPOTESIS fuerte). **No es codigo**: el camino de `SET_RTC` del ESP32 no pasa de ~600 ms (Wire corta a 50 ms). Se mide con un USB-TTL en `TX0` a 115200 (la ROM imprime `rst:0x..`), osciloscopio en 3V3 y `EN`, y una fuente de 5 V buena. **Es la unica superficie de mando (`D-16`)** · 🔗 **Y NO es `N-117`** (§6.4), aunque se vea igual desde el telefono —un modulo que aparece y desaparece—: el perro de `N-117` es el de TAREAS (`esp_task_wdt_init(..., true)` en `vigilante.cpp`), y su reinicio lo publicaria `nombreCausa()` como `PERRO_DE_TAREAS` (`ESP_RST_TASK_WDT`), no como `OTRO_PERRO` (`ESP_RST_WDT`). `N-117` se cerro en banco el 04/09 (`N-126`); esto es otro sintoma de la misma superficie. ⚠️ *Deducido del mapeo de `nombreCausa()`, no medido con monitor serie* |
| 🔴 **El MAESTRO se congela ~3 s en cada `SET_RTC`** · ~~«`J17` enmudece tras cada `SET_RTC`»~~ | el `$EVENT SET_RTC_LO_ACUSA_EL_PUENTE` sale **exactamente +3 s** despues de cada `SET_RTC`, las 4 veces, y el `MUDO` de `J17` aparece a +4/+5 s | ⚠️ **REFUTADO que fuera el ESP32**: los latidos siguieron llegando (el contador `N` cuenta LINEAS, no silencios). Lo que pasa es que `reloj_ajustarConAcuse()` llama a `rtc.setHours/Minutes/Seconds` con `rtcOperativo` en `true` y el RTC **sin contar**: cada una espera hasta 1 s en `HAL_RTC_SetTime` (`RTC_TIMEOUT_VALUE`), y el `MUDO` es falso porque `millis()` se lee antes del bloqueo. 🔴 **El perro del Maestro es de 4 s: dos siembras en la misma vuelta lo reiniciarian**, y ~~la siembra horaria que se esta construyendo congelaria el controlador 3 s cada hora~~ *(11/09, `D-26`: la siembra periodica es cada ~5 min, no cada hora. ~~**En `main` todavia no existe**, y la escritura de hora que si hay —el `SET_RTC` que cruza del puente, `reloj_ajustarConAcuse()`— **SI escribe el RTC hardware** y es la que congela ~3 s. Que la siembra NO escriba el RTC hardware esta **EN CONSTRUCCION, fuera de `main`**: rama `wip/d26-hora-esp32-foto-1315`, sin revisar~~)*. ~~**Sigue asi en `main`**; se le paso al agente que construye el lado STM32~~ 🟢 **CERRADO EN EL FUENTE en `68dd2c5`** (11/09 por la tarde): `reloj_ajustarConAcuse()` de las dos puntas ya no llama a `rtc.setHours/Minutes/Seconds` —el bloque `if (rtcOperativo) {…}` se quito, con los tres motivos medidos en su comentario `N-162`—, `reloj_actualizar()` ya no copia la hora al RTC al adoptar el cristal, y `reloj_fijarEnero()` sale si hay base sembrada. `grep "rtc\.set"` en los dos `reloj.cpp`: solo `setClockSource()` y el `setMonth(1)` de `reloj_fijarEnero()`. **Residual (`H8`):** `rtc.begin()` en `reloj_actualizar()` al adoptar el LSE puede bloquear hasta ~2 s. ⚠️ Sin tarjeta |
| **DAR PASO** | dos, a las 12:20:17 y 12:20:52, **en `INTELIGENTE`**, no en Manual. El Maestro alterna bien: verde→rojo→15 s→`ACK_GREEN` del Esclavo; y despues Esclavo→rojo→15 s→ambar 4 s→verde del Maestro | ✅ del lado del Maestro funciona. 🔴 **Pero la trama MENTIA** en el segundo: `ESC:VERDE` los 16 s del despeje **y** con el Maestro en ambar (12:21:08). `quienVerde` no cambia hasta que el Maestro llega a verde. **En la app, un DAR PASO que "no cambia".** Arreglado el 11/09 en `coordinador_estadoEsclavo()`, con tres comprobaciones nuevas en el arnes de las dos puntas (A10–A12) **vistas fallar** con el defecto inyectado (80 y 264 instantes) |
| **Manual** | 12:19:04 → 12:19:18 en `MANUAL`: rojo/rojo, `T:--`, **sin ciclar**, y ninguna orden dentro | no hay un solo DAR PASO en Manual en esta cinta. **Lo de «Manual no cambia» no esta en esta medida**: hace falta la cinta de esa prueba |
| **El ciclo Automatico** | `SET_TIEMPOS:3,3,15` aceptado; verde de 180 s (`T:178`…`0`), despeje de 15 s, `ESC` alterna, `PLUMA:ARRIBA` con cada verde y `ABAJO` con cada rojo, `RF` 90–100 %, `RTT` ~200 ms | ✅ del lado del Maestro, **y los 15 s de los que hablaba el reporte son el despeje que se configuro** (el tercer campo de `SET_TIEMPOS`), no un numero del firmware |
| **La camara** | `CAM:?` en toda la cinta | es de **antes** de configurarla (15:24). Lo que vino despues esta en §3.8 |

> 🎯 **Y lo que pide el responsable el 11/09, que es la conclusion de la fila de la hora: «son los ESP32
> los que tienen el reloj y deben comandar la hora».** Es `D-20` y `A-15`, decididas, y **lo construido
> no las cumple**: el telefono le manda la hora al STM32 y **el ESP32 nunca le siembra la suya**. Lo que
> hay que construir, y por que no es un parche:
>
> 1. **El ESP32 siembra a su STM32 desde el `DS3231`**, con la hora RELEIDA por la barrera: al arrancar,
>    tras cada `SET_RTC` bueno y ~~**cada hora** (`A-15`)~~ **cada ~5 min** (`D-26`, 11/09). En el Maestro, esa siembra se propaga al Esclavo
>    por radio como hoy.
> 2. **El `SET_RTC` del telefono deja de cruzar al STM32**: lo atiende el puente, y el STM32 recibe la hora
>    del `DS3231`, no los bytes del telefono. Cierra de paso el `$ERR` del puente con la hora sembrada
>    igual (§3.16-C).
> 3. **En el Esclavo manda la radio mientras este fresca** (`D-20`: *el Maestro manda y el Esclavo hace
>    caso*); su propio ESP32 solo lo siembra si no tiene hora de radio reciente —el arranque, o una radio
>    caida—.
> 4. 🔴 **Estrecha una barrera escrita**: `esp32_05_no_origina` —*«el puente no origina; es la propiedad
>    mas importante del puente»*—. `D-20`/`A-15` ya decidieron que el ESP32 siembre, asi que la barrera
>    **no se retira: se estrecha a UNA excepcion justificada**, la hora releida, y el puente **tira esa
>    misma orden si le llega del telefono**, o cualquiera pondria la hora sin PIN.
>
> **Alcance medido el 11/09: 3 firmwares,** ~~16~~ **19 instrumentos que nombran `SET_RTC`, la app y los manuales.**
> *(Recontado el 11/09 con `grep -l SET_RTC`, y **el borde se escribe**: 14 packs de
> `Simulaciones/banco/packs/` + `simulador_puente_esp32.py` + `puente_esp32/arnes_puente.cpp` = 16,
> la cifra de antes, que casa con haber mirado solo `Simulaciones/`; **mas los tres tests de la app que corre la
> compuerta** —`test_dom_execution.js`, `test_unitarios_app.js`, `test_funcional_app.py`— = 19.
> `Validacion_*` y `compuerta.py`: 0.)*
> Va en su rama, con la compuerta delante, y **no se sube sin banco**.
>
> ✅ **Las cuatro reglas, APROBADAS por el responsable el 11/09** (incluidas la 3 y la 4, que eran
> derivacion mia de `D-20` y estrechan una barrera escrita). En construccion ese mismo dia con agentes en
> **worktrees aislados** —ESP32, STM32 y DAR PASO por separado, mas una auditoria de solo lectura de los
> reinicios del ESP32—, y **se integra revisando el diff de cada uno**, no su parte.
>
> 🟢 **INTEGRADO el 11/09 por la tarde: `D-26` esta en `main` desde `68dd2c5`** (merge de
> `feat/d26-hora-esp32` = `a0313fe`, que *«integra la foto 49111ab sobre 16f1512»*: la rama
> `wip/d26-hora-esp32-foto-1315` queda superada y no es ancestro de `main`). La cadencia que entro es la
> de `D-26` —300 s—, no la de la regla 1 tal como estaba escrita arriba; y la regla 3 entro con la forma
> de `D-26` (3): **sin radio 25 s** (`SFTY6_SILENCIO_MS`, el mismo silencio que dispara `$ALARM FALLO_RF`),
> no «si no tiene hora de radio reciente». Lo que el arquitecto dijo de ese diff, abajo.

#### 🎯 El veredicto del arquitecto sobre el diff de `D-26` (11/09) — «FUSIONAR CON CAMBIOS»

Revisado **por el diff**, no por el parte, antes del merge. Se registra aqui porque **lo que deja abierto
bloquea CAMPO, no el merge**, y porque dos premisas del parte no se sostenian. Cada punto lleva la medida
que se re-hizo sobre `68dd2c5` al escribirlo (`CLAUDE.md` §7.4: un informe no es una medida).

**Las dos premisas del parte que NO se sostenian:**

1. **«Degradado a dos puntas 25/25» NO ejerce la regla del Esclavo (`D-26` (3)).** El orquestador compila
   el `modo_degradado.cpp` real de las dos puntas, pero el reloj del Esclavo esta sustituido —
   `adaptador_esclavo.cpp`: `void reloj_notarRadio() { g_radioNotada++; }`; `arnes_puente.cpp`:
   `bool reloj_radioManda() { return rlj_radioManda; }` y *«reloj.cpp : NO se compila»*—. **Ningun arnes
   compila `fuenteHora`, `reloj_radioManda()` ni `reloj_notarRadio()`**: la frontera de 25 s, el arranque
   sin radio y la radio intermitente los mira **solo `reloj_03_manda_la_radio`, por texto**. §0 fila 1.14.
2. **«29 defectos inyectados y todos caen»** (mensaje de `a0313fe`) **no deja rastro en el arbol: es un
   parte**, no una medida.

| | hallazgo | estado |
|---|---|---|
| **`H1`** 🔴 | **`D-26` (3)+(4) abre un VERDE-VERDE PERSISTENTE que la regla (4) no cura.** En la rama `CMD:HORA_ESP32:` del Esclavo, con la radio callada 25 s la hora de su `DS3231` **entra sea cual sea su distancia a la que tenia**; `modo_degradado.cpp` (las dos) solo decide la TRANSICION —`saltoDeHora() > SALTO_SIN_ROJO_MAX_S` → rojo—. Tras el rojo, cada punta reanuda con SU hora: si difieren mas del margen, **cada ciclo solapa**. Pasar por rojo protege el instante del salto, no el desfase que deja. **Escenario:** `J17` del Maestro muerto unas horas → su hora corre sobre el HSI (36–90 s/h) y la radio la empuja al Esclavo, asi que van en fase; muere la radio → el Esclavo adopta su `DS3231` (la buena) y el Maestro se queda con la derivada: **24 h × 36 s/h = 14 min de desfase**. El firmware **lo detecta** (`horaEsp32Vigilar()` publica `$ALARM …EVENTO:HORA_ESP32,CAUSA:J17_MUDO…`) **y no hace nada con ello**. Simetrico con el `J17` del Esclavo muerto y la radio muerta | **Bloquea campo, no el merge.** Opciones del arquitecto: (a) hora caducada como enclavamiento del Degradado en la punta que la tiene → ambar intermitente; (b) acotar el PRIMER salto de fuente. **No es una pregunta nueva al responsable: es `D-21` pieza (1)** —*«una hora que no es fiable … se responde con AMBAR INTERMITENTE EN LA PUNTA QUE LA TIENE»*—, **decidida el 07/09 y EN CONSTRUCCION el 11/09** (worktree, fuera de `main`). §0 fila 1.13 |
| **`H2`** | `arnes_esclavo.cpp` sin preparar | 🟢 **resuelto** en `a0313fe` (`Validacion_LCD/arnes_esclavo.cpp`, +8 lineas en el merge) |
| **`H3`** | **La cuenta 11 s / ~32 dias es correcta y es la cota MENOS restrictiva.** `deriva1 = ceil(300 s × 25.000 ppm) = 8`; `deriva_rel = 2×8 + 2×RESIDUO_SIEMBRA_S(1) = 18`; **presupuesto = aguante (29) − 18 = 11 s** de desfase entre `DS3231`. A 2 ppm por chip opuestos: 0,3456 s/dia → 31,8 dias. Pero (i) en un gabinete al sol vale la ficha −40..85 °C: 3,5 ppm → **~18 dias**; (ii) supone desfase inicial cero entre `DS3231`; (iii) no cubre `H1` | 🔴 **Y tumba la premisa de los «MESES» de `D-21`/`D-23`**: con el `J17` mudo el HSI se come esos 11 s en **11 / 90 ≈ 7 min a 11 / 36 ≈ 18 min**. §0 fila 2.7. La **alarma por discrepancia** de los dos `DS3231` va en buena direccion, **pero su umbral no puede ser 11 s a secas** (ruido del HSI de las dos puntas hasta 2 × 7,5 s + truncado); la cura de raiz es la «cadena completa» que `D-26` aparca. §0 fila 2.8 |
| **`H4`** | `SALTO_SIN_ROJO_MAX_S = DEG_DESPEJE_SEG - 1` bien derivado (con `DEG_DESPEJE_SEG = 30`, 29 s), y `saltoDeHora()` nunca infra-mide hacia delante | 🟠 **FALTAN en el orquestador el borde** (salto = despeje → rojo; = despeje − 1 → directo) **y un salto hacia atras**: el bloque E ejerce `DEG_VERDE_SEG + DEG_DESPEJE_SEG` y 2 s. §0 fila 1.14 |
| **`H5`** | `reloj_radioManda()`: `return fuenteHora >= FH_RADIO && radioOida && (uint32_t)(millis() - tUltimaRadio) <= SFTY6_SILENCIO_MS;` | leido, **no ejecutado**. Nada abierto; ejercerlo es la fila 1.14 |
| **`H6`** | (i) `Maestro/src/bluetooth.cpp` **tira el `bool` de `coordinador_sincronizarHora()`** (`CLAUDE.md` §2); (ii) el `$ACK` del puente del Esclavo dice `OK` aunque su STM32 ignore la siembra porque manda la radio | (i) 🟡 §0 fila 1.15; (ii) **aceptado por `D-20`**: «inutil, no peligroso» |
| **`H7`** | la barrera `esp32_05` y la anti-suplantacion (`suplantaLaSiembra()`: toda linea del telefono con `HORA_ESP32` dentro se descarta con `$ERR,NODE:PUENTE,CMD:HORA_ESP32,DESC:LINEA_RESERVADA_AL_PUENTE`) | 🟢 cerradas. **Abierto POR DECISION** (`D-26` (1)): `SET_RTC` con un PIN falso pone la hora — el puente no conoce el PIN y el STM32 ya no ve la linea |
| **`H8`** | bloqueos del HAL: no queda `rtc.set*` en camino caliente | 🟠 **residual:** `rtc.begin()` en `reloj_actualizar()` al adoptar el LSE (hasta ~2 s). Y con el `CNT` constante (el hardware de la cinta `179DB0`), `respaldo_horasDesdeSync()` da 0 h y `degradado_reanudarTrasCorte()` **concede 48 h nuevas en cada corte**: es lo que `A-16` tiene que saber, y es §3.16-D |
| **`H9`** | ningun pack relaja (revisado por el diff) | 🟡 un comentario caducado: `simulador_puente_esp32.py`, en `escenario_d20`, *«EN EL WORKTREE DEL ESP32 (11/09) ESTO FALLA»* ya no es cierto. §0 fila 1.14 |

> **La recomendacion del arquitecto**, que es la fila 1.14: un bloque en el orquestador de dos puntas que
> enlace el `reloj.cpp` REAL del Esclavo y ejerza los tres casos de `H5` y los bordes de `H4`.
>
> **Lo que la compuerta dijo al fusionar** (mensaje de `68dd2c5`, tres pasadas con el arbol quieto):
> `18 PASS | 2 FALLA | 0 ABORTADO`; los dos rojos son `decisiones_01_anclas` —`D-26` con ancla y **sin
> manual**, que es documentar y no decorar (`CLAUDE.md` §1)— y **`G3`** del arnes de dos puntas (§0 fila
> 2.9). Las cifras se leen del acta, no de aqui.

---

## 6. El porque de lo que sigue abierto — los hallazgos que no se han cerrado

**Nada de este apartado se resumio: son los bloques originales, con su medida.** Lo cerrado esta en
[`roadmap_hist.md`](roadmap_hist.md).

> ⚠️ **11/09 — como se lee este apartado despues de medirlo contra `b79d904`.** (1) **Las citas
> `fichero:linea` en prosa se cambiaron por el SIMBOLO** (`CLAUDE.md` §7.3): un numero de linea
> caduca solo. **Los bloques de codigo que llevan `fichero:linea` son salidas de `grep` de la fecha
> de su bloque**, no de hoy: sus numeros ya no casan y se conservan como la medida que fueron. (2)
> **Varios de estos `N-x` estan cerrados en el fuente** —`N-108` en su mitad de firmware, `N-110`
> entero, `N-117` en su sintoma—, y se marca en cada uno con su commit; siguen aqui porque lo que les
> falta es cobre. (3) Las citas a `CLAUDE.md` con numero simple son de la numeracion de entonces.

### 6.1 · La tarjeta Maestro

### 🛑 N-116 — El Maestro se calienta a los ~30 s: el firmware queda DESCARTADO por censo, no por opinion

> 🔴 **11/09: esta es la Maestro de la SESION 1 del banco, no la `179DB0`.** El 04/09 se descarto
> entera y se reprogramo otra placa como Maestro (`roadmap_hist.md` `N-126`, `SEM-179DB0-M`), que es
> la que corrio V9 en el Sisga. Lo de abajo sigue siendo el diagnostico de esta placa.

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
pines de `J16`. Los dos que se puentearon estan en `INPUT_PULLUP` (`botones_setup()` de
`botones.cpp`) *(11/09: **cierto para `617bd00`**, el binario que estaba en la tarjeta en aquel
banco —`git show 617bd00:…/botones.cpp`—; **en `main` son `INPUT` pelado**, `pinMode(BOTON1, INPUT)`,
desde el arreglo de `N-118`, y sin pull-up la corriente contra masa es aun menor)*: contra
masa consumen `3,3 V / 40 kOhm` ~= **80 uA**, o sea **0,27 mW**. Eso no calienta un chip.

> **Consecuencia dura y util: cargar otro firmware no arregla esto.** Es la clase de conclusion que
> ahorra una sesion entera de banco persiguiendo el sitio equivocado.

#### Una causa que se cayo, y se marca refutada en vez de borrarse

Se sospecho **contencion en `PB6`/`PB7`**: el netlist dice que `J17` es el LCD y el firmware lo usa
como UART del ESP32, asi que dos salidas *push-pull* enfrentadas en el mismo hilo explicarian el
calor perfectamente. **Es falsa.** `Maestro/src/lcd.cpp` construye el objeto `u8g2` con los **cuatro
pines en `U8X8_PIN_NONE`**, y `U8x8lib.cpp` pregunta `if (u8x8->pins[i] != U8X8_PIN_NONE)` antes de
cada `pinMode` y cada `digitalWrite`: no queda ni una escritura. La pantalla no conduce nada.

Queda escrita porque **es la sospecha natural** —la contradiccion netlist/fuente esta ahi y volvera a
proponerse—, y porque las medidas del paso 5 la explican mejor sin ningun defecto: `RST` (`PB7`) a
3,3 V es el **TX del ESP32 en reposo**, que es alto; `RS/A0` (`PB6`) variando entre 2,8 y 3,3 V es el
**TX del STM32 transmitiendo**. Todo coherente, cero conflicto.

#### 🔴 UNA SEGUNDA CAUSA REFUTADA, Y ERA LA MIA — la talanquera no puede ser

Se propuso aqui mismo, y hay que tacharla con el mismo rigor con que se escribio. El razonamiento era:
`escribirPines()` de `semaforo.cpp` energiza la talanquera cuando `verde || estado == S_FALLO`; un Maestro solo cae a
`S_FALLO` a los **~20 s** —medido en el paso 8— y **en ese instante enciende `J15`**, que es lo unico
que conmuta solo dentro de la ventana de los 30 s. Encajaba en el tiempo.

**Se cae al leer el cobre.** Trazada la cadena entera sobre `Controladora_Semaforos.kicad_pcb`:

```
U1.20 (PB2) -> /Motor -> R70 220R -> U15.1   TLP127  (LED del optoacoplador)
                      -> R69 10K a masa      (pull-down de BOOT1, correcto)
U15.6 -> /5V    U15.4 -> R72 220R -> puerta de Q10 (IRLZ44N), con R71 10K y C30 100nF
Q10.2 (drenador) -> J15.2 + D30 1N4148 al riel de 12V     Q10.3 -> GND
```

**`U15` es un TLP127: aisla galvanicamente las dos mitades.** *(11/09: **«aisla galvanicamente»
dice de mas** — `17_` §1.2, corregido alli el 05/09: el opto separa el PIN del micro del nodo de
puerta, pero **hay UNA sola red `GND`** en la tarjeta, con el catodo del LED del opto y la fuente del
MOSFET en ella. **Si la conclusion de este parrafo aguanta con masa comun no se ha vuelto a medir**;
la que la sustituyo, abajo, no la necesita.)* El STM32 no toca la etapa de potencia
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

*(11/09: **son DIEZ, no nueve** —`Q1`–`Q10` con `U6`–`U15`—, como corrigio `17_` §1.2 el 05/09 y dice
el propio `pines.h`: «Son DIEZ MOSFET y DIEZ optos en la placa … no nueve». El bloque se deja como se
escribio.)*

> 🔴 **La placa protege cada SALIDA con 220 ohmios en serie y un optoacoplador, y no protege NINGUNA
> entrada de campo.** Los cinco pines que salen a bornera van **desnudos al die**. El `10K` y el
> `100nF` que llevan estan en **paralelo**, no en serie: fijan el reposo, **no limitan corriente**.

Y en ese mismo conector, **`J16.1` es el riel de `/12V` crudo** —el netlist lo confirma: comparte net
con `J15.1`, `J13.1`, `J11.1` y veintitantos mas—. `pines.h` ya lo tenia escrito (su comentario de
`J16` p1): *«12 V CRUDOS, sin opto, sin serie, sin clamp»*.

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



### 6.2 · Las entradas de campo

### 🔴 N-120 — La placa protege todas sus salidas y ninguna de sus entradas. Va a la V2, y antes de cablear camara

Sale del censo de N-116 y merece linea propia porque **no se va con la tarjeta danada**: es de diseno,
esta en las 185 huellas del `.kicad_pcb`, y afecta a todas las unidades.

| | camino | proteccion |
|---|---|---|
| **salidas** ~~(9)~~ **(10)** | `PBx -> 220R -> TLP127 -> potencia` | serie **y** ~~aislamiento galvanico~~ opto — **con UNA sola red `GND`** en la tarjeta *(11/09, `17_` §1.2: diez cadenas, y el opto no aisla la masa)* |
| **entradas de campo** (5) | `bornera -> pin del STM32` | **ninguna** |

Y las entradas son justo las que un instalador toca: `J14` ~~(camara de demanda)~~ *(11/09: el
firmware la sigue leyendo como camara de demanda, pero las camaras van a `J16` —`D-2`, `D-3`, `D-25`—
y ~~`A-2` la reservo al **fin de carrera**: **conflicto abierto, §0 fila 2.1**~~ **`D-27` la deja libre y sin cablear**)* y `J16` p10/p12
(camaras C y D, **dos por poste** con `D-25`), en un conector cuyo **p1 lleva 12 V crudos**.

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



### 6.3 · El unico defecto de calle con arreglo escrito y sin subir

### 🔴 N-108 — El enlace no deja rastro de como se cayo, y el umbral que lo arreglaba ~~lleva un mes sin subir a campo~~ solo ha tocado una calle dentro de una V9 sin banco

> 🟢 **11/09, medido contra `b79d904`: la mitad de firmware de `N-108` esta CONSTRUIDA**, y lo de
> abajo describe el equipo de antes. Los cuatro puntos de «Lo que falta»: (1) el Esclavo **ya no
> inventa** `RF`/`RTT` —publica `T:--,RF:--,RTT:--` en su `$STATUS` (`2e99bc3`)—; (2) la alarma de
> caida **lleva el tramo** —`RF:%s,RTT:%s,SINRESP:%s` en el `$ALARM` del Maestro—; (3) **hay
> `$EVENT ENLACE_RF`** en las dos puntas; (4) **la app guarda los `$EVENT`** en la cinta exportable
> (`dad4615`) —la del Sisga trae 20, aunque **ningun `ENLACE_RF`**: con `RF` entre 90 y 100 % no
> hubo cambio que anunciar, asi que ese camino **no esta ejercido**—. 🔴 **Y el umbral SI subio a una calle**: `7ff7d12`, cargado en el Sisga el
> 10/09, lleva `SFTY6_SILENCIO_MS 25000UL` (`git show 7ff7d12:…/protocolo.h`). **Lo que sigue en pie
> es lo de `T-2`: la V8.4 certificada sigue en 12 s**, y no hay en git ningun `e303485` con solo esa
> constante. La V9 que llego al Sisga es `SIN_BANCO`.

**Lo aporto el responsable desde el campo el 31/08, y confirma N-71 POR EL OTRO LADO:**

> *"El problema de desconexion es no saber cuanto se va cuando se va, y por que se va. Se va a ambar
> a los 12 segundos de desconexion, y ese parametro toca alargarlo un poco mas, porque se va a
> ambar por nada."*

#### Los 12 s: ya esta arreglado, y ese es el problema

**MEDIDO:** `SFTY6_SILENCIO_MS = 25000UL` en el `#define` de `protocolo.h` de las dos puntas... **pero eso es la
rama**. El equipo de la calle es la **V8.4, `e303485`, del 31/07**, y ese commit **ni siquiera es
alcanzable desde esta rama** (`git merge-base --is-ancestor` -> no). **En el poste sigue habiendo
12 s**, y el arreglo lleva desde el **27/08** escrito sin poder subir.

#### Y el "por nada" es LITERAL — el equipo se rendia antes de terminar de intentarlo

El comentario del propio firmware (el bloque de encima de `SFTY6_SILENCIO_MS`, `protocolo.h`) describe el sintoma sin haberlo visto:

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

**Y lo que el Esclavo INVENTA** (el `snprintf` del `$STATUS` de `Esclavo/src/bluetooth.cpp`, tal como estaba; *hoy publica `--`, `2e99bc3`*):

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

~~Lo que falta, y va con la Fase 4:~~ *(11/09: los cuatro estan construidos en el fuente —recuadro de
la cabecera de este `N-x`—; ninguno ejercido en cobre salvo que la cinta guarda los `$EVENT`)*

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


### 6.4 · El arranque del ESP32

### 🔴 N-117 — El perro del ESP32 se comia su propio arranque, y el pack lo aprobaba mirando la forma

> 🟢 **11/09: el SINTOMA esta CERRADO en banco desde el 04/09, y este apartado no se habia enterado.**
> `roadmap_hist.md` `N-126` (sesion 2): *«CERRADO con evidencia en hardware: el modulo se anuncia
> estable, sin parpadear, y ya con el rotulo aprendido: `SEM-179DB0-M`»*. Con el arreglo dentro y el
> rotulo aprendido, **las dos causas candidatas de abajo quedan descartadas a la vez como sintoma y
> ninguna confirmada como causa**: eso ya no se puede discriminar sin deshacer el arreglo, y «Lo que
> falta» de abajo pierde su objeto.
>
> 🔗 **Y los `OTRO_PERRO` del Sisga (§3.16) NO son este defecto, aunque se vean igual desde el
> telefono.** El perro de aqui es el de TAREAS —`esp_task_wdt_init(segundos, true)` en
> `vigilante.cpp`—, y su reinicio lo publicaria `nombreCausa()` como **`PERRO_DE_TAREAS`**
> (`ESP_RST_TASK_WDT`); los tres del Sisga son **`OTRO_PERRO`** (`ESP_RST_WDT`), y dos partes mas son
> `SUBIDA_DE_TENSION`. **Mismo sintoma de superficie, otro mecanismo**, y se mide en el modulo con un
> USB-TTL en `TX0` (§0, fila 3.2). ⚠️ *Deducido del mapeo de `nombreCausa()`, no medido.*

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
(`#define ROTULO_PROVISIONAL`, `contrato.h`). Y el aprendizaje del nombre bueno **se guarda para el arranque SIGUIENTE**, nunca
en caliente: renombrar obligaria a cerrar el perfil y tirar la sesion del operario
(`transporte_app.cpp`, junto a `cargarRotulo()`, decision deliberada y razonada alli).

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



### 6.5 · Si el ESP32 se cuelga

### 🟠 N-113 — Si el ESP32 se cuelga: que sigue funcionando, que NO, y por que la app no es un canal de alarma

**Propuesta del responsable (01/09):** *«si eso pasa, la apk deberia informar que no hay conexion y
reportar fallo, en cuyo caso el otro micro se queda trabajando con tiempos. Ideal que la apk enviara
un correo de notificacion a nosotros o a la concesion para que puedan apoyar con personal de trafico
mientras la reaccion al fallo.»*

**La base es correcta, pero el supuesto «el otro micro se queda trabajando con tiempos» son DOS
casos, y solo uno se cumple.** Medido en el fuente:

| modo | de que depende | si el ESP32 muere |
|---|---|---|
| **Automatico** | `millis()` — `modoAutomatico_loop()` y compania en `modo_automatico.cpp` | 🟢 **sigue ciclando.** No toca el reloj |
| **Degradado** | `reloj_enHora()` — sus llamadas en `modo_degradado.cpp` y `coordinador.cpp` | 🔴 **no se puede ni entrar** ~~(a secas)~~ *(11/09, con `D-20`: **solo si el STM32 no se sembro desde su ultimo arranque**. Una vez sembrado, si el ESP32 muere `reloj_enHora()` **sigue en `true`** —el unico `horaValida = false` vive en `reloj_setup()`— y el Degradado entra y corre con `millis()` sobre el HSI. Si el STM32 arranca con el ESP32 ya muerto, no hay siembra y **no entra**)* |

> *(11/09: este bloque razona con el reloj de ANTES de `D-20`. Hoy la hora del STM32 es una base
> sembrada desde fuera mas `millis()`, asi que depende del ESP32 **para sembrarse**, no para seguir
> contando — ver la fila de Degradado de la tabla.)*
>
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

- 🟢 *(11/09: **EXISTE** — `j17RegistrarLinea()` en los `bluetooth.cpp` de las dos puntas publica
  `$EVENT` de origen `J17` con `MUDO:…s,MAX:…s,N:…` (`d44048c`, y el umbral de latido de `9ad5e85`); la
  cinta del Sisga trae 4. Lo que no puede es ver morir al puente mientras esta muerto: lo cuenta al
  volver.)* **El STM32 cuenta el silencio de `J17`** igual que ya se cuenta el de la radio (N-108): cuanto lleva
  mudo el puente, cuantas veces se cayo, y cuanto duro cada corte. **No necesita cobertura, ni SIM, ni
  internet, ni que nadie este delante**, y cuando el tecnico por fin conecta, la app se lo descarga
  entero. Contesta justo lo que se dijo que falta en campo: *«no saber cuanto se va cuando se va, y
  por que se va»*.
- 🟢 *(11/09: **EXISTE** — el perro de tareas de `vigilante.cpp` y el parte `EVT:ARRANQUE` con
  `CAUSA:` de `nombreCausa()` (`69e12cc`); la cinta del Sisga trae 5.)* **Watchdog en el ESP32** (ya es la tarea **T4**) para que se reinicie solo, y que **declare el
  reinicio** al volver — un puente que revive en silencio esconde el fallo que hay que contar.
- **Y la decision que hay que tomar antes de escribir nada: que hace el equipo si el reloj se va.**
  Hoy la respuesta es *«el Degradado no entra»*, y eso hay que elegirlo a proposito, no heredarlo.
  *(11/09: elegida el 07/09 — `D-21`: una hora que miente se responde con ambar en la punta que la
  tiene; su pieza A esta sin construir, §0 fila 1.4.)*

#### Lo que cuesta de verdad un aviso remoto, para que se decida con el precio delante

Un correo desde el poste necesita **camino propio a internet**: WiFi del sitio o un modulo celular con
SIM y su plan. El ESP32 trae WiFi —esa parte esta— pero **hace falta cobertura en el cruce y una red a
la que entrar**, y si el que avisa es el mismo que se cuelga, el aviso no sale. **Es una decision de
producto con coste recurrente, no una linea de firmware**, y va al responsable con esa etiqueta.

---


### 6.6 · Las dos barreras de la app

### 🟠 N-110 — Dos barreras de la app que no vigila nadie, salidas de invertir el arnes de DOM

> 🟢 **11/09: LAS DOS ESTAN CERRADAS EN EL FUENTE desde el 01/09, medido contra `b79d904`.** (1) El
> checksum: `app.js` llama a `NMEAParser.validarTrama(linea)` antes de pintar (`dad4615`). (2) El
> teclado: `tecladoPinAbierto()` guarda los cuatro handlers del PIN (`b033f0b`), y
> `test_dom_execution.js` lo ejerce —teclea el PIN con el modal cerrado y exige que la sesion siga
> sin autorizar—. **Lo unico que les falta es un telefono.** Lo de abajo es como estaban.

**Las encontro el agente que invirtio `test_dom_execution.js`, y las verifique yo mismo antes de
escribirlas aqui** (~~§4~~ `CLAUDE.md` §7.4 *en la numeracion de hoy*: un informe no es una medida). ~~**Ninguna de las dos se arreglo el 31/08** — se
dejan medidas y abiertas, porque tocar `app.js` obliga a recompilar la APK y rehacer el paquete.~~ *(se arreglaron el 01/09, recuadro de arriba)*

#### 1. La app **no valida el checksum** de lo que pinta

```
app.js:1420  parseNmeaTelemetry()  ->  line.split('*')[0]      el CRC se tira sin mirarlo
js/nmea_parser.js:27  validarTrama()   4 definiciones en disco, CERO llamadores
```

Es **la forma exacta de N-73**: una funcion declarada, documentada y sin un solo llamador. Y hay
prueba dura de que no se mira: la trama de ejemplo del arnes lleva `*5F` **desde siempre** y su
checksum real es `*04` — la app la pinta como verdad. Sobre radio a **2,4 kbps**, eso significa que
un `$STATUS` corrompido en vuelo se dibuja como el estado del cruce. *(Ya estaba medido en
`simulador_puente_esp32.py`; lo que es nuevo es que el llamador existe y esta ahi al lado.)*

#### 2. El teclado del PIN **acepta pulsaciones con el modal cerrado**

Los handlers de `.pin-btn[data-key]` (`app.js`) **no consultaban si `pin-modal` estaba activo** *(hoy si: `tecladoPinAbierto()`)*, y
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
error de la app. *(Sigue asi el 11/09: `(state.node || '?')` en `avisarOtraPunta()`.)*

---


### 6.7 · Las camaras no hacen nada en los modos que se usan — y el modo que las usa mete el ruido que se temia

> 🟢 **Media seccion esta CERRADA y se deja dicho para no volver a abrirla: el corte de 15 s ya no
> existe** (`62d731e`, `D-19`). **Lo que sigue abierto es lo de abajo:** `demanda_hayLocal()` tiene
> **un solo lector**, asi que en Automatico y en Manual la camara **no hace nada**.


> **Se afirmo tres veces esta noche que «el firmware de las dos camaras ya esta puesto, lo
> que falta es cobre». ES FALSO en Automatico y en Manual, que es donde se opera el cruce.**
> Lo destapo una revision externa a la que se le pidio expresamente que comprobara esa
> afirmacion en vez de heredarla.

### El censo, en una linea

`demanda_hayLocal()` tiene **UN SOLO LECTOR** en todo el Maestro:
`modoInteligente_loop()` de `modo_inteligente.cpp`. Y `coordinador_hayDemandaRemota()` solo se arma **si
`modoActual_get() == MODO_INTELIGENTE`** (la rama `CMD_DEMANDA` de `coordinador.cpp`).

| modo | que hace una deteccion de camara |
|---|---|
| **AUTOMATICO** | fija un `tUltima` que **nadie lee**. `modo_automatico.cpp` ni incluye `demanda.h` |
| **MANUAL** | lo mismo: nada |
| **DEGRADADO / AMBAR** | la trama del Esclavo **ni se lee** |
| **INTELIGENTE** | aqui si, y es el unico |

Lo que si corre en todos los modos es la LECTURA del pin —`botones_actualizar()` sin
condicion en el `loop()` de `main.cpp`—, que es lo que hacia parecer que estaba construido. **Leer no
es consumir.** §2.ter otra vez, y esta vez el que la recito fui yo.

### ~~🔴 Y EL MODO QUE LAS USA VIOLA EL MINIMO DE 3 MINUTOS, EN EL FUENTE~~

> 🟢 **TACHADO EL 12/09 — CERRADO, y llevaba desde el 04/09 contradiciendo a su propia cabecera.**
> La cabecera de §6.7 ya decia *«el corte de 15 s ya no existe (`62d731e`, `D-19`)»* y este bloque
> seguia en 🔴 y sin tachar, tres parrafos mas abajo. **Medido en el fuente antes de tacharlo**
> (`7f90958`), que es lo que `CLAUDE.md` §1 exige para apagar un rojo: `modo_inteligente.cpp` ya no
> tiene el corte —lo dice en pasado dos veces, *«Y encima la Regla 1 cortaba el verde a los 15
> SEGUNDOS -`tiempoActual >= 15000UL`-»* y *«toda la diferencia con el `tiempoActual >= 15000UL` que
> habia aqui»*— y los limites salen ahora de `limites_ciclo.h` (`#include`, `N-137`), con
> `verdeFaseMin = VERDE_MIN_MIN`. **Lo que sigue abierto de §6.7 es lo de arriba y lo de abajo, no
> esto** (fila 1.27).

~~`modo_inteligente.cpp` corta el verde a los **15 s** —constante escrita a mano— mientras
`limites_ciclo.h` fija `VERDE_MIN_MIN = 3` **minutos**, decidido por el responsable el
04/09 por seguridad vial.~~

~~**Consecuencia medida:** con la camara del Maestro pegada en «hay presencia» —o con cola
continua— el **Esclavo recibe 15 s de verde por ciclo y el Maestro 3 minutos**. Con las dos
camaras ruidosas, el cruce alterna al minimo indefinidamente: 15 s + despeje + 4 s de
ambar, en los dos sentidos.~~

> ~~**Eso es exactamente «meter ruido», y estaba en el codigo antes de comprar la camara.**
> Y viola la regla que `modo_automatico.cpp` justifica asi: *«conductor convencido de
> que el semaforo esta averiado, adelantando en rojo»*. La rompe **justo el modo que usa
> camaras**.~~

### El Modo Inteligente es obra DECLARADA, no ejercida

- ~~**Ningun arnes lo compila.** `grep -rl modo_inteligente Validacion_* compuerta.py` -> vacio.
  Lo leen tres packs **por texto**. Es el punto ciego de §8 en su forma pura.~~ 🟢 **CADUCO desde el
  05/09 (`62d731e`), medido el 11/09:** `Validacion_Automatico/compilar.ps1` compila
  `modo_inteligente.cpp` y `demanda.cpp` **reales** en el arnes del automatico (su Bloque E).
- ~~**Ninguna tarjeta lo ha corrido.**~~ El informe del banco del 3-4/09 dice *«nunca aparecio
  la luz verde en ningun poste»* y *«sin camara de demanda real»*. *(11/09: **una tarjeta SI lo ha
  corrido**: el Maestro del Sisga entro en `INTELIGENTE` a las 12:19:46 y dio alli los dos DAR PASO,
  §3.16 — sin camara conectada, `CAM:?` en toda la cinta, asi que la demanda sigue sin ejercer.)*
- Solo se entra por la app: la via del menu esta muerta -`menu.cpp` cuelga de
  `botonAceptar()`, que devuelve `false` siempre- y el mando no tiene secuencia para el.

### La recomendacion, y el coste MEDIDO compilando

| | que es | flash | veredicto |
|---|---|---|---|
| **imagenes / auditoria** | lo que apunto el responsable | **0 B** | **lo sensato hoy** |
| **B · aviso a la caja negra** | al pedir el cambio verde->rojo, si hay presencia, un `$EVENT`. Cero efecto vial | **+60 B** | **la unica que toca firmware sin poder degradar nada** — y es el INSTRUMENTO que el laboratorio necesita: cuenta cuantas veces habria actuado un veto **antes** de darle autoridad |
| **A · veto con tope** (SFTY-29) | extiende el todo-rojo hasta un tope, luego cambia y alarma | **+100 B** | fail-safe en los tres modos, **pero su beneficio es una suposicion** hasta tener la camara delante |
| **C · demanda en Inteligente** | ya existe | 0 | **NO**: modo no ejercido + el defecto de los 15 s |
| **D · demanda en Auto/Manual** | conectar `demanda_hayLocal()` al ciclo | — | **NO**: en Manual la camara «da paso», que es la via que el responsable rechazo para el mando A/B |

### Y tres cosas mas que aparecieron

1. ~~🔴 **La alarma de «8 dias sin detectar» que el responsable enuncio NO EXISTE en el
   firmware.**~~ 🟢 **EXISTE desde el 05/09, medido el 11/09: es `CAM_CIEGA`** —fase 1 de `D-13`
   (`4b90f98`), a **24 h de paso abierto** desde `9550c57` (`D-24`; `CAM_CIEGA_MS = 86400000UL` en
   los dos `botones.cpp`)—. ⚠️ **Con el hueco que `D-25` destapa:** no vigila una camara que nunca
   dio flanco (§0, 1.9). Y sin ella, la camara de la barrera muda equivale a «baja sobre el coche»
   -o sea, lo de hoy-: aceptable solo CON esa alarma.
2. **La camara del Esclavo en Auto/Manual no es inofensiva:** manda **3 copias de
   `CMD_DEMANDA`** por deteccion en un canal de **2,4 kbps semiduplex** que tambien lleva
   `GO_RED`/`ACK_RED`, y lo hace **sin guarda** (`demanda_solicitar()`). *[SIN MEDIR: el impacto
   probablemente es pequeno, pero es una colision posible en el instante que mas importa.]*
3. **`camara_leerPin()` hace `delay(5)`** con el pin en alto (`botones.cpp`; sigue el 11/09).
   Leerla POR NIVEL en cada vuelta cuesta 5 ms/vuelta mientras haya presencia: inofensivo
   para el watchdog de 4 s, pero condiciona como se implementa un veto.

---


### 6.8 · `SFTY-29` · las camaras como veto — declarada y SIN CONSTRUIR

> 🔴 **Medido el 07/09: `grep -rn "EJERCE SFTY-29" packs/` -> `0`**, y `OPTIMIZACIONES.md` la
> tiene como *«solo diseno»*. Es la decision `A-1.bis`.


`OPTIMIZACIONES.md` (su fila de SFTY-29) la tiene registrada como **«solo diseno»**: *«Presencia como veto
del todo-rojo y sensor de pluma»*. Lo que el responsable describio el 04/09 es exactamente
esa regla, y aqui queda con lo que se ha MEDIDO y con lo que sigue SIN DECIDIR.

### Lo que la talanquera hace HOY, medido

`escribirPines()` de `Maestro/src/semaforo.cpp` — es una **funcion pura de la luz**, sin camara ninguna:

```c
digitalWrite(MOTOR_TALANQUERA,
             ((verde && !testLedsActivo) || estado == S_FALLO) ? ABRIR : CERRAR);
```

Verde -> arriba · Rojo -> abajo · Ambar (`S_FALLO`) -> arriba · ~~**sin energia -> baja
sola**~~, porque el pin cae a `LOW` y el MOSFET no conduce (SFTY-28). *(11/09: lo que el firmware
garantiza sin energia es que **`J15` deja de mandar ABRIR**; que la pluma **baje sola** lo decide la
centralita de la barrera, con su propia alimentacion, y es **SIN FUENTE** — §3.16: la «especificacion
de compra con muelle» que citaban cuatro documentos no esta en `15_Lista_de_Compras`. Con `D-25` la
talanquera va por rele a la entrada `OPEN`, asi que sin energia la centralita deja de recibir la
orden de abrir, y lo que haga despues es suyo.)*

### 🔴 LAS DOS CAMARAS NO SON EL MISMO PROBLEMA: FALLAN EN DIRECCIONES OPUESTAS

> ⚠️ *11/09: la distincion «camara de BARRERA / camara de SALIDA» es de antes de `D-13` (05/09), que
> la cerro: **todas las camaras llevan la misma configuracion** —*Intrusion Detection* sobre el
> barrido de la pluma— y **no tocan el ciclo** («Lo que NO se hace: el ciclo del semaforo no se toca;
> las camaras no dan ni quitan verde»). Con `D-25` son cuatro, dos por poste, y siguen siendo
> iguales. Lo de abajo se conserva como el razonamiento que llevo ahi.*

**Camara de la BARRERA** — *presencia -> no bajar la pluma*:

| falla como | consecuencia |
|---|---|
| pegada en «hay presencia» | la pluma **no baja** con el semaforo en rojo. El responsable lo declara aceptable: *«hoy en dia esa mierda funciona asi y a todo el mundo le da igual»* |
| pegada en «no hay» | baja sobre un coche — **pero eso es exactamente lo que pasa HOY**, sin camara |

> **Esa camara SOLO PUEDE MEJORAR.** No hay modo de fallo que empeore el estado actual. Y
> su unico hueco lo cierra la alarma que el propio responsable enuncio: *«la barrera no ha
> bajado porque la camara tiene alarma presencial hace 8 dias, vaya a darle
> mantenimiento»*.

**Camara de SALIDA** — *presencia -> no cambiar a rojo*:

| falla como | consecuencia |
|---|---|
| pegada en «hay presencia» | 🔴 **el semaforo NO CAMBIA NUNCA.** Un sentido con verde permanente y el otro esperando su turno para siempre — y ahi la gente cruza igual |

Lo dijo el: *«si esta camara falla, esa joda no sirve»*. **Tenia razon, y la diferencia es
medible: una es un enclavamiento que falla HACIA SEGURO y la otra HACIA PELIGROSO.**

### Tres consecuencias tecnicas que condicionan como se configura

1. **Hoy el firmware lee FLANCO; un veto necesita NIVEL.** `camaras_actualizar()`
   (`botones.cpp`) dispara en el flanco de subida y llama a `demanda_solicitar()`.
   Vetar es preguntar *«hay presencia AHORA»*: otra lectura del mismo pin.
2. **La solucion la dijo el responsable sin darse cuenta:** *«paralelo al cambio a rojo,
   lanza leer camara»* — o sea **una lectura EN EL INSTANTE de la transicion**, no un veto
   continuo. Un vistazo puntual esta mucho menos expuesto a un sensor pegado.
3. **Un veto CON TOPE DE TIEMPO deja de ser peligroso.** *«No aguanto mas de N segundos;
   pasado eso cambio igual y levanto alarma»*. Es el mismo patron de su alarma de
   mantenimiento aplicado al ciclo, y convierte el enclavamiento de la camara de salida de
   fail-dangerous en fail-safe.

### 🟡 LO QUE SIGUE SIN DECIDIR, y es del responsable

- ~~**¿La camara de salida VETA el ciclo o solo AVISA?** El mismo dudo: *«para modo
  automatico no se, no parece que sea»*, y apunto que quiza vale mas para **imagenes y
  auditoria** —accidentes, soportes— guardadas en la Raspberry o la Nano.~~ 🟢 **CERRADA por
  `D-13` (05/09): no veta el ciclo** —las camaras no dan ni quitan verde—; las imagenes van en la
  microSD de la propia camara (`D-12`), no en una Raspberry. Lo que queda abierto es el veto **de la
  pluma**, que es `A-1.bis`.
- Si veta *(la pluma, `A-1.bis`)*: **cuantos segundos como maximo** — y `DECISIONES.md` ya fija
  que el tope lleva a **alarma, nunca a accion**.
- **Lectura en el instante del cambio, o veto continuo.**
- 🔴 **Que pasa si la camara esta MUDA** —no «no hay presencia», sino sin responder—. *(11/09: en
  parte contestado: `CAM_CIEGA` alarma a las 24 h de paso abierto sin un flanco; **no** la que nunca
  dio uno, §0 1.9.)*
  Presencia y silencio no son lo mismo y **hoy el pin no los distingue**: un cable suelto
  lee igual que «via libre». Con `INPUT` pelado y pull-down, una camara desconectada dice
  «no hay nadie».
- Y su propio aviso, que va escrito porque condiciona todo lo demas: *«hay que hacer un
  laboratorio… no se como se comporta esa camara. No es una camara asi super fiable, ni
  super rapida»*. **Nada de esto se decide sin esa medida.**

---


### 6.9 · 🎯 DECISION: DOS PANTALLAS, PORQUE SON DOS COSAS (04/09, noche)

> 🔴 **Decision del responsable del 04/09 con CERO codigo detras.** ~~Es la unica decision suya
> que no tiene una sola linea.~~ *(11/09: no es la unica — `decisiones_01_anclas` acusa hoy `D-14`,
> `D-22`, `D-23` y `D-25` de no tener ancla, y `D-23` es precisamente **su mitad Esclavo**, decidida el
> 07/09 con la via `$EVENT`: §0, fila 1.3.)*


Palabras del responsable, y resuelve varios sintomas de golpe: *"al conectarnos al maestro,
ese front debe mostrarnos data de todo, incluso de lo que obtiene del esclavo. Y al
conectarnos a una ventana de esclavo, sera para diagnostico: logs de cuando conecta o no, y
estado de las ordenes que recibe del maestro. Ademas no puede permitir operar, pero si
diagnosticar. La pantalla confunde."*

| conectado a | que es | que enseña |
|---|---|---|
| **MAESTRO** | la **consola de operacion** | todo el cruce: sus luces Y las del Esclavo, modo, tiempos, enlace, hora. Es donde se opera |
| **ESCLAVO** | una ventana de **DIAGNOSTICO** | cuando conecto y cuando no, que ordenes recibio del Maestro y que hizo con ellas, su bateria, su hora, su señal. **NO se opera desde aqui** |

**Por que esto arregla mas de lo que parece:**

- La pantalla de hoy intenta ser las dos y por eso confunde: enseña una botonera de operacion
  cuando estas en el Esclavo, con casi todo desactivado, y una columna de `SIN DATOS` cuando
  estas en el Maestro. **Las dos mitades mienten por omision.**
- Cierra la peticion de *"que el Maestro traiga los datos del esclavo"*, que hoy no puede
  porque el `$STATUS` lleva **un solo ESTADO, el del que la manda**.
- Y le da sentido a conectarse al Esclavo: **no es una consola degradada, es el sitio donde se
  diagnostica** —que es exactamente cuando hace falta ir alli, con el enlace caido—.

**El hueco vacio deja de anunciar una carencia y ofrece la accion:** *"Conectarse al esclavo"*
en vez de *"sin datos del esclavo"*. Tambien suyo.

> **Lo que esto NO cambia, y hay que decirlo:** el ambar de emergencia del Esclavo **se queda**.
> Es la unica emergencia que tiene esa punta —no tiene rojo propio— y quitarla dejaria al
> operario de ese poste sin nada. Lo que se arregla es que el Maestro se entere
> (`0.0.octies` de [`roadmap_hist.md`](roadmap_hist.md), donde se mudo el 07/09), no que el
> operario pierda su boton.

---


---

## 7. Los `N-x` cerrados — el puntero de una linea

**Uno por fila, con lo que lo cierra.** El desarrollo entero, con su medida, esta en
[`roadmap_hist.md`](roadmap_hist.md), que lleva indice por `N-x`. **Ninguno se cerro con lo que
decia el propio roadmap** — eso era la trampa.

| `N-x` | lo cierra |
|---|---|
| **N-42** | `ceb8cc5`: el `enum FaseAuto` ya no existe y `coordinador_actualizar()` se llama sin condicion. **Y cerrado EN COBRE el 04/09** con el equipo delante |
| **N-94** | el pack que faltaba existe: `enlace_01_transporte.py` |
| **N-95** | `pines.h` de las dos puntas trae hoy el motivo corregido sobre `RS485_IN_DE_RE PA8` |
| **N-96** | `barrera_01_pines_de_luz.py` ya custodia `ROJO_PEATON`/`VERDE_PEATON`. ⚠️ **La mitad de decision sigue abierta: los tres canales de potencia siguen sin firmware** — §2 `D-d` |
| **N-97** | marcado en 5 ficheros de firmware y 3 packs, con commit propio |
| **N-98** | `05_Funcional/17_...md` reescrito |
| **N-99** | `0741628`, `16d3781` |
| **N-100** | `60ac06e` — 178 citas rotas a cero, y de ahi sale `CLAUDE.md` §4.sexies |
| **N-101** | el arnes del automatico da **99/99** compilando el `botones.cpp` real |
| **N-102** | `# EJERCE SFTY-21` en `packs/maestro_01_mando.py` |
| **N-103** | el censo se ejecuto: 76 packs, cero huerfanas en el acta |
| **N-104** | `D-1` y `D-2` de `DECISIONES.md` |
| **N-105** | `0741628`, `51d2a13`, `e0aa494` |
| **N-106** | `2e99bc3` + el pack `esclavo_08_ambar_en_degradado` |
| **N-107** | `BLQ-1`, 31/08: es un `ESP32-WROOM-32` clasico, hay SPP |
| **N-109 / N-114** | son auditorias; su metodo vive hoy en `CLAUDE.md` §2.bis y §2.ter |
| **N-112** | `c02be59`: dos pasadas seguidas con el arbol quieto, `20 PASS` las dos |
| **N-115** | 24/29 pasos, 03-04/09. ⚠️ **la cuenta `24/29` NO reconcilia** con `17_`, que identifica 22 y repartidos dan 19/9/1. **Ocho documentos publican cuentas que no coinciden** |
| **N-118** | 🟢 **REFUTADO** en `d020f3c`: los `0,6 V` eran el **firmware viejo** (`INPUT_PULLUP` contra los 10 K), no un defecto de placa. Y es moot: `D-1`, ya no hay mando. 🔴 **Sigue publicado como abierto en 16 documentos** — §3.4 |
| **N-119** | cadencia de `J17` a 2000 ms aplicada, medida `48,1 %` |
| **N-121** · **N-122** · **N-123** · **N-124** · **N-125** | los tres ultimos **cerrados en campo el 04/09** |
| **N-126** | sesion 2 de banco. 🔴 **Su evidencia esta perdida**: el `.pdf` que cita NO existe en el repositorio — §3.3 |
| **N-127** | `AB-1`, el latido del puente |
| **N-130** · **N-131** | packs `costura_12` y `app_11` |
| **N-133** · **N-134** · **N-135** | marcados en 7, 4 y 2 ficheros de firmware |
| **N-137** · **N-138** | `limites_ciclo.h` centraliza los seis numeros |
| **N-139** | `grep "millis() */ *1000"` sobre los dos `bluetooth.cpp` -> **0**. El campo sale del simbolo `tTxt`, con rama `--` y rama `!` |
| **N-140** | `D-7`, decidido y construido |
| **N-141** | pack `maestro_11_manual_no_cicla` |
| **N-144** | la bandera ya no sobrevive al fallo de verificacion |
| **N-145** | 🟢 **CERRADO EN COBRE**, cinta del 05/09 22:19: `HORA:22:19:58`. ⚠️ el `0x68` sigue `SIN VERIFICAR` — §1 `T-5` |
| **N-146** | 🟢 **CERRADO EN COBRE**: `$ACK,CMD:SET_MODO:AMBAR,RESULT:REARMADO` en la cinta |
| **N-148** | medido en `app.js`, manejador `btnOpAmber`: pide **PIN y via**, en ese orden |
| **N-149** | 🟢 **CERRADO EN COBRE**: `ESC:AMBAR`/`ESC:ROJO` en la cinta, y el instrumento que quedo en `ABORTADO` da hoy `12/12` |
| **N-154** | `e43a8e7`: el `$STATUS` acotado por buffer, vigilado por `esp32_07`. 🔴 **residual abierto: el `$ALARM` NO se acoto** — §3.1-1 |
| **N-155** | `2d17678`; y el hallazgo que destapo (`A-12`) cerrado en `62d731e` |
| **N-156** | `5846cee` + `d020f3c` |

---

## 8. La arquitectura vigente, y POR QUE es esta


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
| `PB0` | camara de demanda | `J14` | vivo *(solo dentro del Modo Inteligente en el Maestro; el Esclavo lo lee por flanco y lo manda como `CMD_DEMANDA`)* · **11/09, `D-27`: `J14` libre y sin cablear** |
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


> 🔴 **DOS CORRECCIONES AL MAPA DE ARRIBA, medidas el 05/09 y que la tabla todavia no trae:**
>
> - **`PB3`/`PB4`/`PB5` YA estan libres**, no «quedan libres al retirar la LCD»: `lcd.cpp` paso los
>   pines a `U8X8_PIN_NONE` en las dos puntas y la libreria se salta el `pinMode`. **Estan hoy en
>   alta impedancia con pista hasta `J17` p4, p1 y p5** — son los unicos GPIO libres del proyecto
>   **con bornera ya cableada**. Y de los tres **solo `PB3` y `PB4` son de JTAG**; decir «los tres»
>   era falso, y en los que si lo son no cuesta nada: `pin_DisconnectDebug()` suelta JTAG y
>   **conserva SWD**.
> - **Falta `D21`, el LED del canal `Q6` -> `J8` -> `VERDE2`, con el CATODO SIN CONECTAR** — red
>   `unconnected-(D21-K-Pad1)`, cero pistas y cero hilos, mientras su gemelo `D23` los tiene en los
>   dos extremos. **Ese canal de luz no tiene indicador y `J8` p2 FLOTA en reposo** en vez de subir
>   a ~12 V como los otros nueve. Queda `SIN VERIFICAR` si es defecto o decision (`A-10`).

---

## 9. La V2 — lo que ya esta identificado y NO se ha hecho


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
| ~~**C1**~~ | ~~🔴 **N-112: la compuerta alterna**~~ 🟢 **TACHADO EL 12/09 — CERRADO, y lo decia §7 de este mismo fichero: *«`c02be59`: dos pasadas seguidas con el arbol quieto, `20 PASS` las dos»*.** Esta fila se quedo en 🔴 mientras la tabla de cerrados de dos apartados mas arriba lo daba por bueno | ~~**es el aparato de medir, y miente**. Todo lo demas se juzga con el~~ *(⚠️ **lo que SI sigue vivo y no es esta fila**: el intermitente del simulador del puente, `100/101` contra `101/101` con el mismo arbol — §3.6 y fila 1.23 de §0)* |
| **C2** | **Repasar `OPTIMIZACIONES.md`**: la trazabilidad regla -> codigo -> prueba se levanta buscando `# EJERCE`, y el firmware se movio mucho | una regla que aparece cubierta por un pack que no la ejerce **es peor que una fila vacia**, porque la vacia no miente |

> ⚠️ **Y lo que NO entra en la V2, a proposito:** mas packs, mas actas y mas documentos. La auditoria
> (N-109) ya dijo que ese bucle *«produce la sensacion de progreso sin acercar el unico entregable que
> importa»*. **Nada de A, B ni C sustituye al banco** — A y B se escriben para que la sesion de banco
> devuelva datos en vez de impresiones.


---

## 10. Lo anterior, y donde vive


**Este roadmap arranca el 31/08/2026.** El historico del proyecto —los `N-1` a `N-93`, las versiones
V8.0 a V8.9 y las auditorias de agosto— **no se ha borrado**: vive en el `git log` de este
repositorio y en el remoto `padre` (`git log padre/main -- roadmap.md`). No se arrastra aqui porque
un roadmap que crece por acumulacion deja de servir para decidir, que es para lo unico que existe.

**Lo que si se conserva y no se pierde nunca son las reglas**: viven en `CLAUDE.md`, que es el
fichero que se lee solo en cada sesion. Si un `N-x` de los viejos dejo una regla, esa regla esta
alli — y si no lo esta, es que no la dejo.

**Y desde el 07/09/2026 hay un segundo escalon: [`roadmap_hist.md`](roadmap_hist.md).** Ahi estan
integras las 4.631 lineas de `N-94` a `N-157` y de las sesiones del 31/08 al 05/09 que este fichero
dejo de necesitar para decidir. **Con indice por `N-x` y por apartado, y la cuenta de la mudanza
publicada al final** — porque un bloque perdido en una mudanza no grita, y lo unico que lo caza es
que los numeros cuadren.

---

## La cuenta de la mudanza del 12/09 - la red contra perder un bloque

Igual que la del 07/09, publicada para que cualquiera la rehaga y no para que se crea
(`CLAUDE.md` §7).

```
roadmap.md ANTES de la mudanza del 12/09           2.695 lineas

roadmap.md DESPUES, lineas de original conservadas    403   cabecera + 0 + 1 + 2 + 4 + 5
roadmap_hist.md, lineas de original                 2.292   3: 1.311  ·  6..10: 981
                                                    -----
                                                    2.695   <- cuadra exacto

anadido y contado aparte:
  en roadmap.md   la REGLA DE LAS 600 LINEAS, la regla de redireccion de citas,
                  y dos frases literales de 3.16 y 3.11 que se quedaron VIVAS
                  a proposito (abajo)
  aqui            esta cabecera fechada, los dos indices y esta cuenta
```

**Las DOS frases que se quedaron en `roadmap.md` en vez de viajar aqui, y por que.** El pack
`documentos_06_no_reabre_lo_cerrado` lleva **dos excepciones** cuyo motivo escrito era un texto de
3.16 y de 3.11 — y `roadmap_hist.md` esta en su `EXCLUIDOS_RAIZ`: mudarlas habria dejado las dos
excepciones **sin sujeto vivo que las verifique**, que es lo que `CLAUDE.md` §6 llama *«una lista de
excepciones con motivos sin verificar es una lista de defectos con permiso»*. Las dos frases se
conservan **literales** en filas VIVAS de `0` de `roadmap.md`:

| la excepcion del pack | su motivo, literal | donde vive ahora |
|---|---|---|
| la mirada atras sobre *«p12 ... vacio a proposito»* | *«la exencion del vigilante se escribio **para un `p12` vacio a proposito** y con `D-25` pierde su motivo»* — de 3.16 | `roadmap.md` 0, fila **1.9** |
| la mirada adelante sobre *«cada hora»* | *«**se siembre cada hora o cada mes**, eso no cambia»* — de 3.11 | `roadmap.md` 0, fila **2.2** |

⚠️ **Lo que queda por hacer y NO se hizo aqui, porque el pack era fichero PROHIBIDO en este
encargo:** sus dos comentarios siguen citando el motivo como *«roadmap.md §3.16»* y *«roadmap.md
§3.10»*. **Las dos citas apuntan ya a este fichero y el texto sigue estando aqui** —o sea que no
quedan cojas—, pero **el sujeto que el pack de verdad LEE esta en `roadmap.md` 0**: al heredar esas
excepciones hay que re-apuntar los dos comentarios a las filas **1.9** y **2.2**.
