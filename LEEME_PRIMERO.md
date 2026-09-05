# LÉEME PRIMERO — paquete del 04/09/2026, noche

## 1. Qué es esto

**Firmware Y APK.** En campo corre `V8.4`, commit `e303485`, del 31/07/2026. Este paquete
es `7dfb8c7`.

**La APK trae un arreglo que se ve nada más abrirla:** la barra de pestañas tapaba hasta
**131 px** de la botonera —un botón entero a 320 px— y ya no tapa nada. Lo destapó una
foto vuestra, no un instrumento: el que debía medirlo daba «sin hallazgos» porque medía
contra el borde de la pantalla y la barra **flota encima**.

## 2. ¿Ha pasado banco? **NO — pero esta noche se probó mucho, y funcionó.**

Con esas palabras. Lo que sí ocurrió el 04/09 por la noche:

🟢 **El Modo Automático quedó cerrado EN COBRE.** Con el equipo delante: *«ahí cambia ese
ámbar y este a verde. Ahora está funcionando»*, con los 3 minutos puestos.

🟢 **La confirmación de vía funciona y convenció:** *«se le preguntó, no lo puede cambiar
así porque sí»*.

🟢 **El corte de comunicación se recupera solo**, y el modo elegido sobrevive al corte.

La compuerta sale **1008/1008 en 70 packs** y los cuatro binarios compilan. Eso dice que
*los modelos y los arneses de PC no encuentran nada*. **No dice que el firmware funcione en
la tarjeta** — y esta misma noche hay dos pruebas de ello: un defecto nuestro (`N-135`)
estuvo en verde en la compuerta y en los 68 packs hasta que un agente lo encontró
**compilando** una función de tres líneas.

## 3. 🎯 Qué probar con esta carga

**Lo que más vale es comprobar que se arregló lo que reportasteis anoche:**

| | qué mirar | resultado bueno |
|---|---|---|
| **`DAR PASO` en Manual** | pulsar y ver el cruce | **cambia el sentido**: el verde pasa a rojo y el otro a verde, con 4 s de ámbar sólo al pasar de rojo a verde |
| **El contador** | mirarlo en Automático | **baja**, y baja durante los 3 minutos — no sólo en el todo-rojo |
| **El ámbar** | ponerlo desde el Maestro | el Esclavo lo sigue **al instante**, no a los 25 s |
| **El reloj** | inyectar hora y mirar el diario | el `$ERR` del STM32 ya no dice «formato inválido»: dice **`NO_QUEDO_PUESTA`**, que es la verdad |

🛑 **Y un aviso para que nadie reporte un defecto que no existe:** la firma del respaldo
cambió, así que **la primera vuelta de energía con este firmware borra los tiempos
guardados** y el equipo arranca con los mínimos (3 / 3 / 10). Es correcto y está diseñado
así. La prueba de que sobreviven al corte **vale desde la segunda vuelta**.

## 4. Qué se arregló, y de dónde salió cada cosa

| | qué era | de dónde salió |
|---|---|---|
| **`N-141`** | **`DAR PASO` no funcionaba**: Modo Manual tenía la misma trampa que `N-42` — un asistente cuya única salida es un botón que ya no existe | vuestro reporte |
| **`N-143`** | **el contador de la fase larga**. `N-139` lo arregló a medias: la cuenta la hacía el coordinador y el coordinador **no sabe** cuánto dura esa fase | vuestro reporte, dos veces |
| **`N-139`** | el contador **no contaba la fase: contaba el segundero del equipo** (`millis()/1000 % 60`) | vuestro reporte |
| **`N-144`** | 🔴 el equipo se declaraba **en hora con el reloj parado en ceros** — y de esa bandera cuelga la autorización del Modo Degradado | vuestra cinta |
| **`N-137`** | 🔴 el **Modo Inteligente corría a 2 minutos**, por debajo del mínimo vial | revisión cruzada |
| **`N-138`** | el `FORMATO_INVALIDO` del reloj era un mensaje **mentiroso** | el diario de órdenes |
| **`N-134`** | el ámbar **se ordena** en vez de que el Esclavo lo dedujera 25 s después | vuestro reporte |

**Casi todo salió del banco.** Los instrumentos de PC vieron dos de los siete.

## 5. 🔴 Lo que sigue roto, y no lo arregla esta carga

**El bloqueo del cruce.** El ámbar de emergencia del Esclavo engancha un cerrojo que sólo
se suelta desde el propio Esclavo, con PIN — y para llegar ahí hay que desvincular el
Maestro en Ajustes de Android. **Un operario puede dejar el cruce trabado y no poder
soltarlo desde el otro poste.** Está pendiente de decidir cómo se cierra.

**El equipo tiene hora y publica que no la tiene** (`N-145`). El campo `HORA:` lo rellena el
STM32, que es el micro **sin** reloj; el DS3231 vive en el ESP32. Acotado, va en la
siguiente.

**La app:** 🟢 la barra ya no tapa los mandos. Sigue abierto que **no reconecta si el
equipo se reinicia** —hay que cerrarla y abrirla, y los equipos se reinician solos: hay
`EVT:ARRANQUE,CAUSA:SUBIDA_DE_TENSION` en vuestras cintas— y que el `--` que el firmware
manda ahora se pinte como **0** en el anillo. Las dos en arreglo.

**El puente H no se cablea todavía:** faltan la corriente nominal y de arranque del
motorreductor.

**El ESP32 se reinicia por tensión** — hay `EVT:ARRANQUE,CAUSA:SUBIDA_DE_TENSION` en
vuestras cintas. Es el `LM2596`, no el firmware.

## 6. Qué hay dentro

| | |
|---|---|
| `ACTA_verificacion.txt` | la corrida: fecha, `HEAD`, toolchain |
| `IOT_VIAL_Semaforos_2026-09-04_7dfb8c7_SIN_BANCO.apk` | la app, verificada entrada por entrada contra el fuente |
| `01_Firmware/` | **fuente** para PlatformIO. Sin binarios: se compilan del código que se revisa |
| `02_Manuales/` | manuales y la guía de banco — **todavía sin lo de esta noche** |

**Firmware primero, cargado y verificado en la tarjeta; el cableado después.** Nunca al
revés.

## 7. Carga por SWD

`mode=UR` con `-e all`, y no se cambia. Si falla, **reintenta**: enganchar es cuestión de
*timing*. `HOTPLUG` con un firmware que se cuelga al arrancar deja `failed to erase memory`.
