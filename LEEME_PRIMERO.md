# LÉEME PRIMERO — paquete del 05/09/2026, noche

## 1. Qué es esto

**Firmware Y APK.** En campo corre `V8.4`, commit `e303485`, **del 31/07/2026**. Este paquete
es **`517531d`**, que es el commit del que sale **todo el contenido de este `.zip`**.

> *(Nota mecánica, por si alguien la busca: la línea que acaba de leer se comiteó en el commit
> siguiente —un fichero no puede contener el hash del commit que lo contiene—. El firmware, los
> manuales y el acta de este paquete son los de `517531d` exactamente.)*

> ⚠️ **La APK dice `7586c46` y el paquete `bfc8800`. No es un descuido:** se compiló sobre
> `7586c46` y los commits siguientes **sólo tocan documentos**, verificado con
> `git diff --name-only 7586c46..bfc8800 -- 05_Funcional/App_Semaforo/`, que sale **vacío** (corrido ahora). **El
> nombre lleva el commit del que salió el binario, no el de cuando se empaquetó**: al revés
> sería decir que se midió algo que no se midió.

## 2. ¿Ha pasado banco? **NO.**

Con esas palabras. La compuerta sale **20 PASS · 0 FALLA · 0 ABORTADO** y el banco por packs
**1161/1180 en 76 packs**. Eso dice que *los modelos y los arneses de PC no encuentran nada*.
**No dice que el firmware funcione en la tarjeta.**

> 🔴 **Y esta noche hay una prueba concreta de ello, que conviene leer entera:** durante horas la
> compuerta estuvo en `19 PASS · 1 FALLA`, **y ese `FALLA` era el hallazgo más útil del día** — el
> Modo Inteligente podía cortar un verde a los 15 s, por debajo del mínimo vial de 3 minutos. Se
> ha ido porque **se arregló el firmware**, no porque nadie mire.

**Nada de este paquete ha tocado una tarjeta.** En concreto: `camara_estado()` **nunca se ha
ejercido con cobre en `J16`**, y ningún banco ha visto una alarma `CIEGA` ni `PEGADA` de verdad.

## 3. 🛑 Lo que hay que hacer ANTES de enchufar nada

**El firmware nuevo tiene que estar CARGADO Y VERIFICADO en la tarjeta antes de que nadie
conecte una cámara a `J16`.** No es «el mismo commit»: **un commit no protege de un
destornillador**. Con el firmware viejo dentro, lo que se enchufe en `J16` p10 puede pulsar
*Aceptar* en un equipo que está en la calle.

**Y `J16` p1 lleva 12 V CRUDOS** — sin opto, sin resistencia, sin clamp, directo al micro.
**Taparlo es obligatorio en cada equipo que se monte**, no una cautela de banco.

## 4. 🎯 Qué probar con esta carga

| | qué hacer | resultado bueno |
|---|---|---|
| **La cámara mueve el cruce** | cerrar `J16` p10 contra p9 (3,3 V) con el equipo en **Modo Inteligente** | el verde de ese lado **se mantiene** mientras haya detecciones, hasta el doble del tiempo configurado |
| **Y NO lo acorta** | configurar 3 min y pedir paso desde la cámara enseguida | **no cambia antes de los 3 min**. Antes cortaba a los **15 s** |
| **Sin cámaras se porta como el Automático** | dejar `J16` desconectado y correr en Inteligente | ciclo normal con los tiempos configurados. **Si esto falla, para y avisa** |
| **El pin vacío NO alarma** | con **una sola** cámara conectada, esperar | **no puede salir `CAM_CIEGA` del pin vacío**. Si sale, es un defecto |
| **La cámara pegada sí alarma** | puentear p10 y dejarlo **20 min** | llega `$ALARM … CAM_PEGADA`. Quitar el puente → `CAM_C_RECUPERADA` |
| **El Degradado del poste 2** | desde la app, pedir Modo Degradado **en el Esclavo** | entra, **o dice por qué no** con uno de sus seis motivos. Y **se ve en la pantalla de la app** |
| **La hora** | consultar reloj de maestro, esclavo y celular | los tres, y el desfase entre ellos |

**Los pasos con sus casillas están en la Guía de Cableado y Pruebas de Banco**, que se rellena y
**se devuelve en PDF**. Ese PDF es el formulario de vuelta: lo que no salga en él, no se contesta.

## 5. 🛑 Avisos, para que nadie reporte un defecto que no existe

**`CAM: ?` («SIN COMPROBAR») es lo normal hasta la primera detección.** No es un fallo: **un pin
que nunca ha dado una señal no se vigila**, a propósito. Por eso **el paso de instalación en que
el instalador provoca una detección DEJA DE SER OPCIONAL** — es lo que arma el vigilante y lo que
hace que `CAM:` pueda decir `OK`.

**Las alarmas de cámara llevan `ACCION:NINGUNA`, y eso es un dato, no un hueco.** El cruce
funciona **exactamente igual** con las dos alarmas puestas: son avisos de mantenimiento, no
fallos de seguridad. **No se para un cruce por esto.**

**`CAM_PEGADA` no sabe distinguir un relé trabado de un vehículo parado veinte minutos debajo de
la pluma.** Las dos cosas piden que alguien vaya a mirar; por eso la causa dice `CONTACTO_FIJO`
y no «avería».

**`BAT:--` sigue siendo correcto.** La batería no se mide: no hay divisor ni entrada analógica.

## 5.bis 🔴 Un arreglo de esta última hora, y hay que saberlo

**El diálogo del Modo Degradado decía *«Para salir: VOLVER AL MENÚ»*, y esa orden SÓLO LA
ATIENDE EL POSTE 1.** La frase se escribió cuando ese modo era sólo del Maestro; al abrirlo al
Poste 2 se quedó. **Un operario que la siguiera no podía sacar al Poste 2 del modo.**

Corregido. **Las tres salidas reales del Poste 2 son:** que vuelva el enlace con el Poste 1 ·
**ÁMBAR EMERGENCIA**, la única que el operario tiene en la mano · o que venzan las **48 h**.

> **Y esto no lo encontró ningún instrumento**: lo encontró quien fue a escribir el manual, al
> intentar explicar el botón. Ningún pack lee el texto de un diálogo contra el enrutado por
> punta.

## 6. Qué cambió en la app, que es mucho

**223 líneas.** Tarjeta nueva de **cámaras** (`OK` / `SIN COMPROBAR` / `CIEGA` / `PEGADA`),
**Modo Degradado del poste 2** con su botón y sus seis motivos de rechazo, **consulta de reloj**
de las dos puntas y del teléfono, y el **modo real del poste 2** en la trama, que antes era un
texto fijo.

⚠️ **La APK anterior queda obsoleta.** La de este paquete es
`IOT_VIAL_Semaforos_2026-09-05_7586c46_SIN_BANCO.apk`, y su contenido está verificado **entrada
por entrada y por CRC** contra el repositorio: **0 nombres distintos, 0 CRC distintos**.

## 7. 🔴 Lo que sigue abierto

**El manual del «doble» NO EXISTE, y es una condición del responsable.** El Modo Inteligente
puede alargar una fase hasta **el doble** del tiempo configurado, y eso se aprobó *«**si** un
funcional revisa el manual y este manual de uso es claro»*. **Ese manual está sin escribir.**
Un verde que unas veces dura 3 minutos y otras 6 **parece una avería desde la acera**.

**El `$ALARM` no cabe en su peor caso** — medido: 158 caracteres contra 143 en el Maestro, 171 en
el Esclavo. Es **anterior** a este paquete y no se ha arreglado. Una trama truncada **no da un
dato malo**: no casa el CRC y la app la descarta entera.

**Tras un reinicio, la vigilancia de silencio queda desarmada** hasta la primera detección: una
cámara que muera en el mismo corte que reinicia al equipo **no se anuncia**.

**El `botones.cpp` del Esclavo no se compila en ningún arnés.** Lo que sostiene su corrección es
que su bloque es **idéntico byte a byte** al del Maestro — y eso **no es lo mismo que ejecutarlo**.

**El ESP32 se reinicia por tensión** (`EVT:ARRANQUE,CAUSA:SUBIDA_DE_TENSION`). Es el `LM2596`,
no el firmware.

## 8. Qué hay dentro

| | |
|---|---|
| `ACTA_verificacion.txt` | la corrida: fecha, `HEAD`, toolchain |
| `IOT_VIAL_Semaforos_2026-09-05_7586c46_SIN_BANCO.apk` | la app, verificada por CRC contra el fuente |
| `01_Firmware/` | **fuente** para PlatformIO. Sin binarios: se compilan del código que se revisa |
| `02_Manuales/` | manuales y la guía de banco, **con sus cabeceras de estado** |

**Los manuales que llevan aviso salen CON el aviso, o no salen.** Varios describen cosas que
**no se deben hacer todavía**, y ese aviso es la parte útil del documento.

## 9. Carga por SWD

`mode=UR` con `-e all`, y no se cambia. Si falla, **reintenta**: enganchar es cuestión de
*timing*. `HOTPLUG` con un firmware que se cuelga al arrancar deja `failed to erase memory`.
