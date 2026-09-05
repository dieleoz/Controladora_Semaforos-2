# LÉEME PRIMERO — paquete del 05/09/2026, madrugada

## 1. Qué es esto

**Firmware Y APK.** En campo corre `V8.4`, commit `e303485`, del 31/07/2026. Este paquete
es `42a52cd`.

Sale de la sesión de banco de anoche, y **casi todo lo de aquí lo destapasteis vosotros**:
tres de los cinco defectos salieron de una **cinta de tramas** y de un **diario de
órdenes**, no de una revisión nuestra.

## 2. ¿Ha pasado banco? **NO.**

Con esas palabras. La compuerta sale **20 PASS · 0 FALLA · 0 ABORTADO** y el banco por
packs **1025/1025 en 71 packs**. Eso dice que *los modelos y los arneses de PC no
encuentran nada*. **No dice que el firmware funcione en la tarjeta.**

Y anoche hubo dos pruebas de ello: **el defecto del ámbar pasó las 20 comprobaciones sin
despeinarlas** —lo encontró vuestra cinta— y **el del Modo Manual también**.

## 3. 🎯 Qué probar con esta carga

| | qué hacer | resultado bueno |
|---|---|---|
| **El ámbar que no entraba** | ÁMBAR → **ROJO TOTAL** → **ÁMBAR otra vez** | la segunda vez **entra**, y la app dice **«ya estaba y se ha vuelto a encender»**, no un OK igual que el primero |
| **DAR PASO en Manual** | entrar en Manual y pulsar **enseguida**, sin esperar | **cambia a la primera**. Antes se rechazaba durante 15 s |
| **Manual quieto** | entrar en Manual y **no tocar nada** un minuto | **no se mueve solo**. Antes cambiaba a los 15 s |
| **El ámbar del Poste 2** | pedir ámbar desde el teléfono **junto al Esclavo** | el **Maestro se va a ámbar también**, en el acto y no a los 25 s |
| **El Esclavo en pantalla** | conectarse al **Maestro** | se ven **los dos postes**. Si el enlace cae, dice **«SIN DATO DEL POSTE 2»** — no se inventa un color |
| **La hora** | mirar la tarjeta de sincronizar reloj | dice lo que el equipo tiene: una hora, o **«SIN HORA PUESTA»** |

## 4. 🛑 Dos avisos, para que nadie reporte un defecto que no existe

**LA HORA SEGUIRÁ SALIENDO `--:--:--` SI NO HAY UN DS3231 CONECTADO.** Eso es el arreglo
**callándose bien**, no fallando. El reloj del STM32 está muerto (`Y2`, N-17), así que
ahora la hora la pone el **DS3231 del ESP32** al pasar la trama. Pero la dirección I2C
`0x68` está **SIN VERIFICAR sobre el módulo real** y **el DS3231 no está comprado**
(línea A6 de la lista de compras). **Sin módulo en el bus, esta parte no se puede dar por
probada.**

**`BAT:--` en todas las tramas es correcto de momento.** La batería no se mide: no hay
divisor ni entrada analógica que la lea. Aparece como pregunta abierta, **sin causa**,
porque no la hemos medido.

## 5. Qué se arregló, y de dónde salió cada cosa

| | qué era | de dónde salió |
|---|---|---|
| **`N-146`** | 🔴 **el ámbar contestaba `OK` y no encendía nada.** Seis órdenes seguidas, seis OK, y el equipo en rojo durante 47 tramas. Entrar en ámbar sólo pasaba al **cambiar de modo**, y `ROJO TOTAL` cambia la luz sin cambiar el modo: después de un rojo total, **el botón de ámbar quedaba muerto para siempre** | **vuestra cinta de tramas** |
| **`N-147`** | 🔴 **el Modo Manual hacía un ciclo que nadie pidió.** Entraba por la puerta del Automático. `DAR PASO` se rechazaba 15 s, y a los 15 s **el cruce cambiaba solo**. Los «15 segundos» que reportasteis son literales: `tiempoDespejeMs = 15000` | **vuestro reporte** |
| **`N-147.b`** | y uno que nadie había visto: pulsar `DAR PASO` **reiniciaba** el contador de despeje. Pulsando cada 10 s **no se veía el verde nunca**, y cada pulsación contestaba OK | salió al medir |
| **`N-142`** | 🔴 el Esclavo se ponía en ámbar y **el Maestro no se enteraba**: podía seguir dando **VERDE hasta 3 minutos** con el otro lado en ámbar | vuestro reporte |
| **`N-149`** | el `$STATUS` del Maestro **no traía nada del Esclavo** | *«necesito que maestro me traiga los datos del esclavo»* |
| **`N-148`** | el ámbar **no pedía confirmación de vía**. Y al medirlo: es la orden que **más** abre paso —pone intermitente en **los dos** extremos— | vuestro reporte |
| **`N-145`** | la hora se mandaba al micro **sin reloj** | vuestra cinta |

**El despeje entre verde y verde NO se ha tocado** (SFTY-4). Lo que se quitó es cobrarlo
dos veces.

## 6. 🔴 Lo que sigue abierto

**La matriculación de las dos puntas.** Hoy se hace mirando los **nombres** de los
Bluetooth. Tiene que ser por **ID**, y sin hacerlo a mano. Está aplazado a después del
banco, por decisión vuestra.

**`MANDO_A` y `MANDO_B` no responden** — `0,6 V` en reposo (`N-118`), `J16` p5 y p8.
**Van cableados**: con `MANDO_B` al aire, el ámbar local no se arma nunca y **se pierde
el veto de SFTY-21** sin que ningún test lo diga.

**`J16` p1 lleva 12 V crudos.** Taparlo es **obligatorio en cada equipo que se monte**
(`N-120`), no una cautela de banco.

**El ESP32 se reinicia por tensión** — hay `EVT:ARRANQUE,CAUSA:SUBIDA_DE_TENSION` en
vuestras cintas. Es el `LM2596`, no el firmware.

**El puente H no se cablea todavía:** faltan la corriente nominal y de arranque del
motorreductor.

## 7. Qué hay dentro

| | |
|---|---|
| `ACTA_verificacion.txt` | la corrida: fecha, `HEAD`, toolchain |
| `IOT_VIAL_Semaforos_2026-09-05_42a52cd_SIN_BANCO.apk` | la app, verificada entrada por entrada contra el fuente (501 entradas, 13 ficheros web, cero diferencias) |
| `01_Firmware/` | **fuente** para PlatformIO. Sin binarios: se compilan del código que se revisa |
| `02_Manuales/` | manuales y la guía de banco — **la guía y las specs se están actualizando ahora mismo con lo de anoche; van en el siguiente** |

**Firmware primero, cargado y verificado en la tarjeta; el cableado después.** Nunca al
revés.

## 8. Carga por SWD

`mode=UR` con `-e all`, y no se cambia. Si falla, **reintenta**: enganchar es cuestión de
*timing*. `HOTPLUG` con un firmware que se cuelga al arrancar deja `failed to erase memory`.
