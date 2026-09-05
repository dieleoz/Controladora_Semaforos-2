# LÉEME PRIMERO — paquete del 04/09/2026, noche

## 1. Qué corre hoy en la calle, y que esto NO es eso

**En campo corre `V8.4`, commit `e303485`, del 31/07/2026.** Este paquete es `944c18d`.

## 2. ¿Ha pasado banco? **NO.**

Con esas palabras, porque es lo único que importa antes de instalar nada.

La compuerta sale **20 PASS · 0 FALLA · 0 ABORTADO** y el banco **998/998 en 69 packs**.
Eso dice exactamente esto: *los modelos y los arneses de PC no encuentran nada*. **No dice
que el firmware funcione en la tarjeta.**

Y hoy hay dos pruebas de ello, con fecha:

- Los cuatro fallos que reportasteis esta tarde **pasaron esas mismas comprobaciones sin
  despeinarlas**.
- Y un defecto que introdujimos nosotros a media tarde —`N-135`— **estuvo en verde en la
  compuerta y en los 68 packs**. Lo encontró un agente **compilando**, no leyendo.

## 3. 🎯 Lo que hay que probar, y es lo que desbloquea

**Cargad las dos puntas y probad el Modo Automático.** Es lo que estaba roto.

**El paso 30 de la guía es el más valioso de la sesión:** poned Automático y **mirad al
Esclavo dos minutos**. El resultado bueno es **que no pase nada**.

- Si el Esclavo **ya no** se va a ámbar solo → el arreglo funcionó.
- Si **sigue** yéndose a ámbar a los ~25 s → hay una segunda causa, y eso es información
  que hoy no tenemos. Anotadlo, no lo deis por sabido.

🛑 **Un aviso para que nadie reporte un defecto que no existe:** la firma del respaldo
cambió, así que **la PRIMERA vuelta de energía con este firmware BORRA los tiempos
guardados** y el equipo arranca con los mínimos (3 / 3 / 10). Es correcto y está
diseñado así. La prueba de que los tiempos sobreviven al corte **vale desde la segunda
vuelta**.

## 4. Qué se arregló

| | |
|---|---|
| **`N-42`** | el Automático no movía las luces **y dejaba al Maestro mudo en la radio**. Por eso el Esclavo se iba a ámbar solo: era orfandad, no un fallo de radio |
| **`N-133`** | los tiempos del ciclo **no se guardaban en ningún sitio**. Ahora sobreviven al corte |
| **`N-134`** | el ámbar **se ordena** en vez de que el Esclavo lo dedujera 25 s después |
| **`N-135`** | 🔴 defecto **nuestro**, introducido esta tarde por el arreglo de `N-42` y cazado horas después |

**Y la app:** todo lo que se pulsa llega a 44×44 px; el PIN caduca; el operario **deja de
teclear `1234`** para dar paso y confirma en su lugar que ha mirado el tramo; y los
errores del equipo **se traducen** a lo que hay que hacer en el poste.

## 5. 🔴 Lo que sigue roto o abierto

### 5.1 · El reloj: `FORMATO_INVALIDO` — SIN DIAGNOSTICAR

El formato cuadra por los dos lados sobre el papel. **No se ha podido saber más porque la
cinta de tramas sólo grababa lo que ENTRA**: 300 tramas y ninguna era la que se mandó.

**Este paquete trae el arreglo del instrumento, no del fallo:** la app ahora graba también
lo que **sale**, y lleva un **diario de órdenes** con la terna *orden / respuesta / efecto*.
Repetid la inyección de hora y **mandad el diario**: dirá en una línea qué está mal.

### 5.2 · El puente H no se cablea todavía

Está decidido un `L298N` por barrera, **fuera de la placa**, en el esquema simple: `J15`
gobierna `ENA`, la pluma sube con señal y baja por su peso. **Faltan dos números de la
placa del motorreductor: corriente nominal y de arranque.** Con 12 V el `L298N` cae entre
2 y 5 V, y el par baja más de la mitad. Pregunta 11 de la guía.

### 5.3 · El ESP32 se reinicia por tensión

En vuestra cinta hay **dos arranques en siete minutos** (`SUBIDA_DE_TENSION`,
`TENSION_BAJA`). Es la alimentación, no el firmware: falta el `LM2596` desde la batería.

### 5.4 · Cámaras: son **DOS**, una por poste

El manual anterior listaba tres entradas y se podía leer «tres cámaras». **La cámara va a
`J14`**, que es el único borne con antirrebote por hardware.

## 6. Qué hay dentro

| | |
|---|---|
| `IOT_VIAL_Semaforos_2026-09-04_944c18d_SIN_BANCO.apk` | verificada entrada por entrada contra el fuente |
| `ACTA_verificacion.txt` | la corrida: fecha, `HEAD`, toolchain |
| `01_Firmware/` | **fuente** para PlatformIO. Sin binarios: se compilan del código que se revisa |
| `02_Manuales/` | manuales y la **guía de banco de 36 pasos** |

**El orden es: firmware primero, cargado y verificado en la tarjeta; el cableado después.**
Nunca al revés. Un commit no protege de un destornillador.

## 7. Carga por SWD

`mode=UR` con `-e all`, y no se cambia. Si falla, **reintenta** — enganchar es cuestión de
*timing*. `HOTPLUG` con un firmware que se cuelga al arrancar deja `failed to erase memory`.
