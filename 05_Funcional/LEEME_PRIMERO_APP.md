# LEEME PRIMERO — App IOT-VIAL, paquete del 28/08/2026

> ## ⚠️ ESTE LÉEME DESCRIBE EL PAQUETE DEL 28/08. HAY DOS POSTERIORES
>
> 🔴 **La APK que hay que instalar hoy es la del 04/09:**
> **la APK del 04/09 que acompaña a este paquete** — su nombre exacto y su `md5` estan en `LEEME_PRIMERO.md`, en la raíz del `.zip`, que es el único sitio donde no caducan.
> Las anteriores **no pueden abrir el enlace Bluetooth**: `app.js` marcaba «Enlazado» y se
> suscribía sin haber llamado nunca a `connect()`, y además marcaba dos MAC escritos a mano que no
> son los del ESP32 (N-122 y N-124). Con cualquier APK anterior la app no conecta **por bien que
> funcione el módulo**. Y el orden en campo importa: **emparejar el ESP32 en Ajustes de Android
> primero**, y luego «Buscar Módulos Bluetooth» dentro de la app.
>
> ✏️ *Corregido el 04/09: esta línea citaba `…_2026-09-02_285b18d_…`, **un hash que no existe en
> ninguna parte**. La del 02/09 que sí está en disco es la `617bd00`. Se anota en vez de
> sustituirse en silencio: una cifra inventada que desaparece sin dejar rastro vuelve a escribirse.*
>
> Si va a probar con el `.apk` del **02/09** (`IOT_VIAL_Semaforos_2026-09-02_617bd00_SIN_BANCO.apk`),
> **la app trae cuatro cosas que este documento no menciona** y que cambian lo que hay que mirar:
>
> - **Comprueba el checksum y descarta las tramas corrompidas.** Antes las pintaba como buenas.
>   🔴 **Consecuencia: una pantalla quieta ya no significa «el equipo no contesta» — puede
>   significar «contesta y llega roto».** Son dos averías distintas.
> - **Pestaña `Tramas`** (🧪, sólo en modo técnico): las tramas en crudo, las rechazadas marcadas con
>   su motivo, y se puede guardar el registro en un fichero. **Es donde se mira cuando algo no
>   cuadra.**
> - **El PIN ya no se puede armar con el teclado cerrado.**
> - **El puente ESP32 dice por qué arrancó** al reconectar, con un `$EVENT,EVT:ARRANQUE`.
>
> El detalle está en `14_Manual_App_Movil_IOT_VIAL.md` §5.4. **Lo que no cambia: sigue sin pasar
> banco.**

## 1. Qué corre en la calle hoy, y esto no es eso

**En campo está la V8.4 del firmware** (commit `e303485`, 31/07/2026), validada por el funcional.
La app de este paquete habla con firmware **V8.5 a V9.0**, que **no está en campo**.

Esta app no reemplaza al panel del gabinete. Es una consola de diagnóstico y mando por Bluetooth.

---

## 2. Esta APK NO HA PASADO BANCO

Con esas palabras, y por eso el nombre del fichero lo lleva dentro:
`IOT_VIAL_Semaforos_2026-08-28_a8e1ceb_SIN_BANCO.apk`

Lo que sí tiene es la compuerta en verde: **15 PASS · 0 FALLA · 0 ABORTADO**, en
`ACTA_verificacion.txt`. **Eso no es un permiso.** Significa exactamente esto: los modelos y los
arneses que corren en un PC no encuentran nada. **No significa que la app funcione contra una
tarjeta**, porque ninguna de esas comprobaciones ha visto un STM32 ni un módulo HC-05.

Este proyecto ya pagó esa confusión: el 05/08 la compuerta salió en verde mientras había una
regresión abierta en la que el Modo Automático no encendía las luces en la tarjeta. Las dos cosas
eran ciertas a la vez.

**Instálela para probar contra el equipo. No la dé por buena hasta que alguien lo haga.**

---

## 3. Lo que hay que mirar primero cuando la conecte

Esta versión corrige cuatro defectos que la anterior tenía y **nadie había visto**, porque los dos
instrumentos que los habrían cazado no llegaban a ejecutarse. Los cuatro son de comportamiento
contra el equipo, así que son justo lo que hay que verificar con la tarjeta delante:

| qué comprobar | qué debe pasar |
|---|---|
| **La pantalla sigue al equipo** | Al conectar, el semáforo, el contador, la batería y el RF de la pantalla deben cambiar solos, sin tocar nada. Si se quedan quietos, la app no está recibiendo `$STATUS` |
| **El PIN se pide de verdad** | AUTOMÁTICO, DAR PASO y ÁMBAR deben abrir el teclado de PIN la primera vez. Si actúan sin pedirlo, avise |
| **ROJO TOTAL no pide PIN** | Es deliberado: la parada de emergencia la puede dar cualquiera que vea el accidente |
| **Un rechazo se ve** | Si el equipo contesta `$ERR`, tiene que aparecer en la pestaña Eventos. No debe tragárselo |

> ✅ **El simulador de demo ya no viene en el build de campo.** Hasta la versión anterior la
> pantalla principal tenía un panel *"SIMULADOR DE PRUEBAS"* cuyos botones pintaban un estado
> falso **en los mismos semáforos que el estado real**, y además un ciclo local que se animaba
> solo cuando no había equipo conectado. Los dos se retiraron: **todo lo que vea en la pantalla
> viene ahora del equipo.**
>
> Su reverso, que conviene conocer antes de reportarlo como fallo: **sin enlace la pantalla se
> queda quieta y lo dice** — el rótulo pasa a `SIN ENLACE - sin datos del equipo` y el contador
> a `--`. Eso no es que la app se haya colgado: es que no tiene nada que contar y no se lo
> inventa.

> 📱 **Y se arregló un corte de pantalla que solo se veía en el teléfono.** Medido con el
> navegador a cuatro anchos: a **412 px** —el de las capturas— no aparecía, pero a **360 px**, que
> es un Android normal, la página se salía **41 px** y cortaba todo lo de la derecha, incluidos
> **DAR PASO** y **ROJO TOTAL**. La causa no era la botonera sino la cabecera, que no encogía.
> Ya mide 0 px de desborde en los cuatro anchos. En pantallas estrechas las dos pastillas de
> arriba se quedan **en icono, sin rótulo**: se pierde el texto, no el mando.

---

## 4. Qué cambió, ya que ha llegado hasta aquí

- **La app volvió a oír al equipo.** La versión anterior había perdido el camino de telemetría
  entero: mandaba órdenes y pintaba un estado que se inventaba el propio teléfono.
- **La barrera de PIN es real.** Antes la app conocía el PIN y lo inyectaba en todos los comandos:
  la protección del firmware la abría ella misma. Ahora el PIN sale del selector del modal
  Bluetooth y hace falta teclearlo una vez por sesión.
- **Vuelven dos mandos** que el firmware atiende y la app había dejado sin botón: `SOLICITAR_PASO`
  y `Modo Manual`, los dos en Modo Técnico.
- **Se pueden renombrar y borrar cruces** otra vez desde el gestor.
- **Contraste medido, no elegido.** Cada color de texto se mide contra el fondo con la fórmula de
  WCAG y se exige 4,5:1. Cinco llegan a AAA. El rojo de la **lámpara** se dejó a propósito en 4,9:1:
  se probaron siete rojos y ninguno creíble de semáforo llega más arriba sobre fondo casi negro, y
  un rojo que no se lee como rojo es peor. El **rótulo** sí usa un rojo de texto más claro.

> **Sobre el sol directo:** la cuenta de contraste es necesaria pero **no suficiente**. Con luz
> fuerte, el reflejo del cristal sube el nivel de negro y comprime los ratios, y a un tema oscuro
> le comprime más que a uno claro. **Si la pantalla no se lee al mediodía, no es sorpresa y no es
> un fallo de la app: es que falta un modo día de fondo claro.** Dígalo si le pasa — es una medida
> de campo que desde aquí no se puede hacer.

---

## 5. Qué hay en este paquete

```
LEEME_PRIMERO_APP.md    esto
ACTA_verificacion.txt   la corrida de la compuerta: fecha, HEAD y toolchain
IOT_VIAL_...SIN_BANCO.apk   la app (Android, build debug)
02_Manuales/            manual de la app, protocolo Bluetooth y el documento de auditoria
```

La APK está **verificada por contenido**, no por confianza: sus nueve ficheros web son idénticos
byte a byte a los del repositorio en el commit `a8e1ceb`.

**Instalación:** es un *build debug* sin firmar para tienda. Hay que permitir *"instalar apps de
origen desconocido"* en el teléfono. Requiere emparejar antes el módulo por Bluetooth del sistema;
el PIN de emparejamiento del módulo y el PIN de autorización de la app (`1234`) son cosas distintas.

> 🛑 **El módulo es un `ESP32-WROOM-32` clásico, y este texto decía `HC-05/JDY-31`.** El **`JDY-31`
> está PROHIBIDO por su nombre** —es **BLE**, y la app conecta por **SPP** (Bluetooth clásico):
> con un `JDY-31` no empareja y habría que rehacer el puente nativo. Ver
> `04_Manuales/MANUAL_CONFIGURACION_BLUETOOTH.md` §1. **No lo compre ni lo pruebe.**

**Reproducir la verificación:**
`git checkout a8e1ceb && python 01_Firmware/compuerta.py`
