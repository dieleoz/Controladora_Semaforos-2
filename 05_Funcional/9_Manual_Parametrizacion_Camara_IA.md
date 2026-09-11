# 📷 MANUAL DE CONFIGURACIÓN Y PARAMETRIZACIÓN — CÁMARAS IA ACUSENSE PARA SEMÁFOROS MÓVILES (V9.0)

**Sistema:** Controladora de Semáforos Móviles de 3 Estados (Maestro y Esclavo V9.0)  
**Cámara Certificada:** ~~Hikvision AcuSense Varifocal Motorizada (DS-2CD3643G2-LIZSU o equivalente)~~ ⛔ **RETIRADO el 05/09: era un modelo de REFERENCIA con «o equivalente» detrás, no el comprado.** → **Hikvision `DS-2CD2683G2-IZS` (2.8 – 12 mm)**, bullet AcuSense varifocal motorizada de 8 MP. **Es la cámara que el responsable ha comprado**, y desde hoy toda cifra de óptica, alarma o consumo de este manual sale de **su** ficha oficial, con la cita al lado — ver **§1.1**.  
**Topología del Sistema:** Analítica Deep Learning Embebida (sin PC externo) + contacto seco a la tarjeta. 🔴 **LAS CÁMARAS SE CABLEAN A `J16` p10 (`PB14`) y p12 (`PB15`)**, ~~**UNA POR POSTE**~~ → 🎯 **11/09, `D-25`: CUATRO CÁMARAS, DOS POR POSTE** — en cada poste, **cámara 1** entre `p9` (3,3 V) y `p10` (`CAM_C_PIN`) y **cámara 2** entre `p11` (3,3 V) y `p12` (`CAM_D_PIN`) — `DECISIONES.md` filas **`D-2`**, **`D-3`** y **`D-25`**, con `M3` cerrada en cobre el 03/09. Contacto seco contra los **3,3 V contiguos** (`p9` para `p10`, `p11` para `p12`), **activo en ALTO, dos hilos, nada a masa**. **Las dos entradas hacen exactamente lo mismo** (ver la revisión del 11/09 aquí abajo). El firmware lee **tres** entradas por punta —~~la bornera `J14` (`PB0`, con antirrebote RC de 1 ms) es la tercera~~— pero **`J14` NO lleva cámara**: se conserva vivo como candidato a fin de carrera de barrera. 🔴 **CONFLICTO ABIERTO (11/09), no resuelto aquí:** `A-2` manda el fin de carrera a `J14`/`PB0`, y el firmware **sigue leyendo `PB0` como cámara** (`CAM_DEMANDA_PIN`) — un fin de carrera ahí se leería como demandas de paso. Lo decide el responsable. La **talanquera** va en **`J15`** (`D-25`), no en `J14` ni en `J16`.  
**Verificación Hardware:** Esquemáticos KiCad `Controladora_Semaforos.kicad_sch`, `pines.h` y `03_Hardware_Tarjeta/MAPEO_TARJETA_KICAD.md`  
**Normativa Aplicable:** Manual de Señalización Vial de Colombia (Resolución 2024 - MinTransporte)  
**Fecha de Emisión:** 26 de Agosto de 2026  
**Última revisión:** 11 de septiembre de 2026 — 🎯 **`D-25`: CUATRO CÁMARAS, DOS POR POSTE, Y LAS CONEXIONES SON DEFINITIVAS.** El responsable, 11/09: *«mantener estas conexiones como definitivas»*. En cada poste, **cámara 1** entre `J16` p9 (3,3 V) y **p10** (`PB14`, `CAM_C_PIN`) y **cámara 2** entre `J16` p11 (3,3 V) y **p12** (`PB15`, `CAM_D_PIN`), cada una por el contacto seco de su salida de alarma (`1A`/`1B`). La **talanquera** va en **`J15`** (p1 = 12 V, p2 = drenador de `Q10`, que **no es masa**) a la bobina de un relé cuyo contacto va a la entrada `OPEN` de la centralita de la barrera. **Deroga de `D-13` SÓLO «una cámara por poste» y «`p12` vacío a propósito»**: el resto de `D-13` sigue — **las cuatro llevan la misma configuración** (§4). Lo que tachan estas líneas donde este manual decía «una por poste» está fechado *11/09, D-25*. **Lo que el que cablea TIENE que saber, medido el 11/09 en el firmware de las dos puntas:** (1) **las dos cámaras de un poste hacen LO MISMO** — `botones.cpp` recorre `CAM_J16[2] = {CAM_C_PIN, CAM_D_PIN}` en el mismo bucle de `camaras_actualizar()`: pedir paso (`demanda_solicitar()`) y vigilante. **No hay una «de demanda» y otra «de pluma»**; (2) 🔴 **NINGUNA CÁMARA PROTEGE LA PLUMA** — `escribirPines()` de `semaforo.cpp` la sube con `(verde && !testLedsActivo) || estado == S_FALLO` y **no lee ninguna cámara**: **la pluma baja aunque haya un coche debajo**. El veto es `A-1.bis` y **no está construido**; lo que existe es un **contador** que observa la bajada ya hecha; (3) 🔴 **la pluma SUBE con verde y también con el ámbar intermitente** (`S_FALLO`): Modo Ámbar, ámbar de emergencia, Degradado en ámbar y **falta de enlace — incluido un poste recién encendido que aún no habla con el otro**. **No se ponga debajo del brazo mientras trabaja**; (4) **en Automático y en Manual las cámaras no cambian ninguna luz**; sólo en **Inteligente**, cumplido el tiempo configurado, **pueden alargar** una fase con un tope — **nunca la acortan**; (5) 🔴 **la app pinta `CAM: OK` —*«las dos ven y ninguna está pegada»*— en cuanto CUALQUIERA de las dos detecta algo, y SIGUE EN OK AUNQUE LA OTRA NO HAYA DETECTADO NUNCA** (`camara_estado()` salta el pin que nunca dio un flanco, y `vigilante_tick()` no lo vigila): **una segunda cámara muerta desde el día de la instalación no la detecta nadie**. Por eso **cada cámara se comprueba con el multímetro en su propio borne** (§4.bis.7). Esa exención del firmware se escribió porque `p12` iba vacío; con `D-25` pierde su motivo y **queda pendiente de rehacer en el firmware** (no se toca desde este manual); (6) **con un firmware anterior a `deeeab4`** —incluida la V8.4 que está en campo— **`PB14` es *Aceptar* y `PB15` es *Cancelar***: lo que se enchufe en `p10` **o en `p12`** puede pulsar un botón en un equipo en la calle. **Carga verificada antes del primer hilo**; (7) **`p12` es el borne de `J16` más cercano a la red de 12 V** (`1,359 mm` de cobre): con `p1` tapado (`D-4`) y trabajo limpio. **Y un conflicto que NO se resuelve aquí:** `A-2` manda el fin de carrera a `J14`, y el firmware lee `PB0` como cámara.

*Revisión anterior:* 7 de septiembre de 2026 — 🔴 **Se volvió a abrir la ficha del modelo comprado y la respuesta a la pregunta del relé CAMBIÓ DE SIGNO**: la fila *Linkage Method* (ficha pág. 4) **enumera** cinco vinculaciones y `trigger alarm output` **no está**, y el argumento con que este manual lo daba por probable —*«tiene `1 output`»*— **se retira**, porque `Manual Alarm` y `Automatic Alarm` cierran esa salida sin analítica ninguna (PDF 80 · impresa 68). Ver el bloque 🔴 de aquí abajo y el `Paso 0`. Además: **se corrige una lectura equivocada de la fuente** —`Alarm Input **NO.**` de la pág. 44 es *Number*, no *Normally Open*—, se localiza **la única aparición real de `NO` como valor** en las 110 páginas (PDF 99 · impresa 87), se deja escrito que **el lado del controlador de `D-14` NO está construido** (medido), se pone delante el **conflicto vivo `D-13` contra el firmware** sobre qué zona se dibuja, y se sustituyen **las citas por número de línea que estaban caducadas** por el símbolo y su `grep`.

*Revisión anterior:* 5 de septiembre de 2026 (3.ª del día) — 🔧 **YA HAY PASO A PASO DE CÓMO SE CABLEA A LA PLACA Y DÓNDE: el nuevo §4.bis.** Es lo que faltaba: el manual sabía entrar en la cámara y configurar la analítica, pero **no decía con qué hilos se une a la tarjeta**. Trae alimentación (PoE `802.3at` Clase 4 o `12 V` directos a batería, con los ~~`26 Ah/día`~~ *(11/09, `D-25`: `52 Ah/día` por poste, dos cámaras)* que eso cuesta), los **dos** hilos del contacto seco a `J16`, por qué la entrada `IN1`/`GND1` **hoy no se cablea**, la comprobación con multímetro y una **ficha que se rellena y se devuelve**. 🔴 **Y cierra una lectura que podía cablearse mal: `1A` y `1B` son UNA PAREJA —los dos bornes de UN contacto—, no dos salidas**, leído del diagrama de la Guía rápida que llegó hoy a `04_Manuales/`.

*Revisión anterior (05/09/2026, 2.ª del día): 🔑 **AHORA EMPIEZA POR ENTRAR EN LA CÁMARA, Y ESE ES EL CAMBIO IMPORTANTE.***
Con la cámara delante, el reporte del funcional fue **«no encuentro ni la IP»**: un manual de
parametrización que arranca en la analítica no le sirve a quien todavía no ha visto el login. El
bloque 🔑 de la cabecera lleva ahora el descubrimiento con **SADP —que ya está en el repositorio,
en `04_Manuales/SADP＿EN/SADP.exe`—**, la **activación** (la cámara sale **inactiva**, y eso es lo
que más despista), la IP y el acceso web, con **qué hacer cuando la lista sale vacía**.
Se añade además **📜 EL CONTRATO** *(D-12)*: **de cada cámara el sistema consume UN CONTACTO SECO**
—sin red, sin imagen, sin analítica en el controlador—, lo que convierte **este documento en el
entregable principal** de la parte de cámaras. 🟢 **Y se CORRIGE una afirmación de este mismo día:**
decía que *«sin imágenes no hay soporte de accidentes ni auditoría»* y **es falso** — la cámara
graba por su cuenta en **microSD de hasta 512 GB** *(ficha, pág. 3)*. Está tachado con su motivo.
🔴 **Aviso que vale para todo el documento: el manual de usuario que citamos es de la `5.7.20` y el
firmware vigente de la cámara es la `5.7.23`** — las rutas de menú pueden no coincidir con la
pantalla.

*Revisión anterior (05/09/2026): 🎥 **LA CÁMARA YA TIENE NOMBRE Y FICHA: `DS-2CD2683G2-IZS`.**
La pregunta que podía tirar el diseño —*¿este modelo concreto tiene salida de alarma por contacto
seco?*— **está contestada, y la respuesta es SÍ**: `1 input, 1 output (max. 24VDC/24 VAC, 1 A)`, con
los bornes `1A`/`1B` que este manual ya citaba. **El camino de `J16` sigue en pie y no hay que
rediseñar nada.** Lo que **sí** se ha caído es una frase que llevaba meses sosteniendo decisiones sin
que nadie la comprobara: *«la salida de la AcuSense es configurable NO/NC»* — **eso no lo dice
ninguna fuente oficial de este modelo**. Ver el bloque 🎥 de la cabecera y **§1.1**, **§4 Paso 4** y
**§7**.*

*Revisión anterior (04/09/2026): 🟢 **`M3` ESTÁ CERRADA EN BANCO (paso 20): el
pull-down de `J16` p10 y p12 es REAL y de 10 kΩ.** Con eso **desaparece el único bloqueo que quedaba
para cablear cámara a `J16`**, y la **polaridad la decide el cobre: ACTIVO EN ALTO**. Y con el mismo
banco entra un aviso que **no se puede separar de este manual**: `J16` p1 lleva **12 V crudos** y las
cinco entradas de campo van **desnudas al pin del STM32** — ver el bloque 🛑 de la cabecera, que es
lo primero que se lee. Ver apartados 2.1, 4, 6 y 7.*

*Revisión anterior (02/09/2026): **`J16` p10 y p12 ya son entradas de cámara en el firmware.** Este
manual las describía como los pulsadores* Aceptar *y* Cancelar *y decía que «ningún firmware los lee
todavía como cámara»; las dos cosas son falsas desde el 31/08. Y se corrigió un segundo punto:
**ninguna cámara de este equipo se configura con Detección de Cruce de Línea** — las tres entradas
son de demanda.*

*Revisión anterior (31/08/2026): corregida la POLARIDAD del cableado. Este manual mandaba llevar el
contacto seco a «`PB0` y `GND`». **La entrada es activa en ALTO**: contra masa la cámara no dispara
nunca. Nada se borra: el texto viejo queda tachado en su sitio con el motivo.*

---

> ## 🛑 ANTES DE ENCHUFAR NADA EN `J16`: TAPAR EL PIN 1, QUE LLEVA 12 V CRUDOS (04/09/2026)
>
> **Este aviso no se separa de este manual y no se resume.** Es lo único que hay entre un instalador
> y una avería que **ya ocurrió**: el 04/09 una tarjeta **Maestro quedó con un cortocircuito de
> 3,3 V a masa** durante la sesión de banco.
>
> ```text
>   J16 p1  ------------------------------->  12 V CRUDOS
>           el unico conector de senal de toda la tarjeta que los trae
>
>   Las 5 ENTRADAS DE CAMPO van DESNUDAS al pin del STM32:
>           sin resistencia en serie, sin optoacoplador, sin clamp
>
>   Las 9 SALIDAS de la placa si van protegidas:
>           220 Ohm + optoacoplador
> ```
>
> **La asimetría es el dato:** lo que sale de la tarjeta está protegido; **lo que entra, no**. Un
> roce de 12 V contra cualquiera de las cinco entradas llega directo a la patilla del micro que
> gobierna el semáforo.
>
> ### 🔧 Lo que se hace, en cada equipo, ANTES de cablear cámara
>
> 1. **Tapar físicamente el pin 1 de `J16`** —tapón, funda termorretráctil o el propio conector con
>    la posición 1 sin terminal—. **Es obligatorio**, no recomendado, y se hace **equipo por equipo**.
> 2. Sólo entonces se llevan los hilos a **p10 / p12**, ~~con retorno por **p2 (`GND`)**~~
>    ⛔ **CORREGIDO el 05/09: son DOS hilos y no hay tercero a masa** — el contacto cierra
>    contra los **3,3 V** del borne contiguo (`p9` para `p10`, `p11` para `p12`) y la corriente
>    vuelve a masa **por `R67`/`R68`, dentro de la propia tarjeta**. Ver **§4.bis.5**.
> 3. **En el p1 no se conecta nada, nunca.**
>
> ⚠️ **Y sigue en pie la confusión que quema módulos:** `J16` y `J17` **comparten footprint y son
> idénticos a la vista**. Multímetro en la posición 1 contra masa **antes** de enchufar: **si da
> ≈ 12 V es `J16`** —ahí va la cámara, con el p1 tapado—; **si no, es `J17`** y ahí va el módulo.
>
> 🔴 **Lo que este aviso NO arregla:** tapar el pin protege del error de cableado, **no** de una
> sobretensión que entre por el hilo de campo. La protección de verdad —**2K2 en serie en las cinco
> entradas**— es una modificación de la **revisión V2 de la placa**, y está anotada como línea de
> compra en `15_Lista_de_Compras_Hardware.md` (bloque **E**). **Hoy no existe en el cobre.**

> # 🔴 07/09 — LA RESPUESTA A LA PREGUNTA MÁS CARA DEL PROYECTO, Y ES PEOR DE LO QUE ESTE MANUAL DECÍA
>
> **La pregunta:** *¿puede la **analítica** de esta cámara accionar su relé de salida?* De ella
> depende que el camino de `J16` —dos hilos, un contacto seco, el único que el firmware sabe leer—
> exista o no. Con las cámaras **ya compradas**, es la pregunta que puede tirar el diseño entero.
>
> **La respuesta del papel, con la ficha del modelo comprado delante:**
>
> | | |
> |---|---|
> | **Ninguna fuente oficial dice que sí** | — |
> | **La ficha del modelo lo OMITE de una lista que ENUMERA** | `DS-2CD2683G2-IZS_Datasheet_V5.5.113_20230303.pdf`, **pág. 4**, fila *Linkage Method*: *«Upload to FTP/memory card/NAS, notify surveillance center, trigger recording, trigger capture, send email»*. **Cinco enlaces, y `trigger alarm output` no está entre ellos** |
> | **El manual de usuario pone la condición de modelo** | *«Trigger Alarm Output … **This function is only supported by certain models**»* — **PDF pág. 79 · impresa 67** |
>
> 🛑 **Este manual venía diciendo que el hardware `1 output` lo hacía *«probable»*. Eso se retira
> hoy:** el mismo manual documenta **`Manual Alarm`** (un botón en el navegador) y **`Automatic
> Alarm`** (cierre **por horario**, no por detección) — **PDF pág. 80 · impresa 68** —, que cierran
> esa salida **sin analítica ninguna**. **El borne existe y eso no dice nada sobre el enlace.**
>
> ✅ **Sigue siendo 🔴 `SIN VERIFICAR` —el papel no lo cierra en positivo— pero el peso de la prueba
> está del otro lado: lo esperable es que la casilla NO esté.** Lo cierra **el `Paso 0` del §4, diez
> minutos con la cámara delante**, y ese paso ya no es un trámite: es lo primero que se hace.
>
> 🔻 **Y si no está, lea el bloque 🔻 del `Paso 0` antes de dar nada por perdido:** la grabación por
> analítica en la microSD **sobrevive** (`Trigger Recording` **sí** está en la lista de la ficha), y
> lo que muere es el bit al controlador. **La vía de `D-14` NO es el relevo**: su lado del
> controlador **no está construido** — medido el 07/09, ver §4.bis.6.

### 🔧 ¿VIENE USTED A CABLEAR? VAYA DIRECTO AL **§4.bis**

**El paso a paso del cableado —qué hilo va a qué borne, en qué orden, y cómo se comprueba que
quedó bien— está en el apartado §4.bis, y está escrito para ejecutarse de pie delante del
gabinete, sin leer el resto del manual.** Lo demás de este documento explica **por qué**; §4.bis
dice **cómo**.

| si viene a… | vaya a |
|---|---|
| **cablear la cámara a la placa** | **§4.bis** — alimentación, los dos hilos del contacto, la comprobación con multímetro y la ficha que se rellena |
| **entrar en la cámara** *(no encuentra la IP)* | el bloque 🔑 de aquí abajo |
| **parametrizar la analítica** | **§4**, Pasos 0 a 4 |
| **saber qué está medido y qué no** | **§7** |

🛑 **Y las dos cosas que no se saltan por prisa, las dos en §4.bis:** el **firmware nuevo tiene que
estar ya cargado y verificado en la tarjeta** antes de enchufar un hilo, y **`J16` p1 se tapa** en
cada equipo. **Un commit no protege de un destornillador.**

> ## 🔑 EMPIEZA AQUÍ: **ENTRAR EN LA CÁMARA.** No se parametriza lo que no se ve (05/09/2026)
>
> **Este bloque está delante de todo lo demás porque es donde el trabajo se paró de verdad.** El
> 05/09, con la cámara delante, el reporte fue literal: **«el funcional no encuentra ni la IP»**. Un
> manual que arranca en la analítica no le sirve a quien todavía no ha visto la pantalla de login.
>
> ### 🟢 La herramienta YA ESTÁ EN EL REPOSITORIO. No hay que descargar nada
>
> ```text
>   04_Manuales/SADP＿EN/SADP.exe          <-- 72 MB, verificado en disco el 05/09
> ```
>
> > 🛑 **COPIA ESA RUTA TAL CUAL.** El carácter entre `SADP` y `EN` **no es un guion bajo normal**: es
> > `＿` (*fullwidth low line*, `U+FF3F`). Si se teclea un `_` corriente **la carpeta no aparece**, y
> > el técnico concluye que la herramienta no está cuando sí está. Es exactamente la clase de
> > «no aparece» que este proyecto obliga a descartar antes de reportarlo.
>
> ### Los cuatro pasos, en orden, y lo que atasca en cada uno
>
> | | qué se hace | lo que despista |
> |---|---|---|
> | **1** | **Encontrar la cámara.** Ejecutar `SADP.exe` y buscar dispositivos en línea | 🔵 **SADP la ve AUNQUE su IP no esté en tu rango** — usa descubrimiento por difusión, no una petición a su IP. Por eso es la herramienta correcta justo cuando «no aparece la IP» |
> | **2** | **Activarla.** Sale **INACTIVA de fábrica**: usuario `admin` y **sin contraseña ninguna** | 🔴 **ESTO ES LO QUE MÁS DESPISTA.** No es que la contraseña esté mal: **es que todavía no hay contraseña, y la cámara no contesta hasta que se le pone una.** La columna `Status` de SADP dirá `Inactive` |
> | **3** | **Ponerle IP** del rango de ustedes, desde SADP, con la contraseña recién creada | Hay que **volver a escribir la contraseña** para que acepte el cambio de IP (`Modify`) |
> | **4** | **Entrar por navegador** a la IP nueva | Ver el aviso de `http` / `https` de abajo |
>
> **Fuente de los pasos 1-3:** manual de usuario, cap. 1 *«Device Activation and Accessing»*, §1.1.1
> *Activate via SADP* — **PDF pág. 13, impresa 1**. Leído del PDF que está en `04_Manuales/`.
>
> ### 🆘 Si la lista de SADP sale VACÍA
>
> Antes de dar por muerta la cámara, se descarta al buscador (`CLAUDE.md` §4). Por orden de
> probabilidad:
>
> 1. **El firewall de Windows** está bloqueando el descubrimiento. Es la causa más común: SADP
>    escucha por **UDP** y Windows lo bloquea de serie en redes «públicas». Permitir `SADP.exe` en el
>    firewall, o marcar la red como «privada».
> 2. **El PC está en otra VLAN o en otro switch.** El descubrimiento **no cruza routers**: el
>    portátil y la cámara tienen que estar en el **mismo dominio de difusión**. Lo más seguro es un
>    cable directo entre portátil y cámara.
> 3. **La cámara no tiene alimentación.** Si se alimenta por **PoE**, el puerto del switch tiene que
>    dar PoE de verdad (**802.3at, Clase 4** — ficha, pág. 4); un puerto sin PoE deja la cámara
>    muerta y **no enciende ningún testigo visible desde abajo**. Alternativa: **12 VDC** por el
>    conector coaxial de 5,5 mm.
> 4. **Hay más de una tarjeta de red** en el portátil (Wi-Fi + cable, o una VPN levantada) y SADP
>    está escuchando por la que no es. Desactivar el Wi-Fi y las VPN mientras se busca.
>
> > 🔴 **Y una trampa que deja fuera a quien ya lleva un rato intentándolo:** el manual documenta un
> > **bloqueo por intentos fallidos activado de fábrica** — *«If admin user performs seven failed
> > password attempts (five attempts for user/operator), the IP address is blocked for 30 minutes»*
> > (**PDF pág. 16, impresa 4**). Si se han hecho varios intentos a ciegas, **puede que la cámara
> > esté bloqueando el PC durante media hora** y no haya nada que arreglar salvo esperar. Se apaga en
> > `Configuration → System → Security → Security Service`.
>
> ### 🌐 `http://` o `https://` — y aquí NO vale la respuesta corta
>
> > ⚠️ **Circula la idea de que «es `https://`, no `http://`». Las fuentes NO dicen eso**, y se
> > comprueba antes de escribirlo:
> > * El manual (**PDF pág. 91, impresa 79**) presenta HTTPS como algo que **se habilita**: *«Check
> >   Enable to access the camera via HTTP **or** HTTPS protocol»*, y sólo si además se marca
> >   **`Enable HTTPS Browsing`** el acceso queda **restringido a HTTPS**.
> > * El `.docx` de la cámara lista **las dos** URL como válidas: `http://192.168.1.64/` y
> >   `https://192.168.1.64/`.
> >
> > **Lo que hay que hacer, entonces:** probar **`http://<IP>`** primero. Si no contesta, probar
> > **`https://<IP>`** — y entonces **el navegador va a avisar del certificado**: es normal (la
> > cámara trae uno autofirmado) y hay que **aceptar y continuar**.
>
> ### Datos de fábrica
>
> | | valor | de dónde sale |
> |---|---|---|
> | **IP** | `192.168.1.64` | ✅ **Manual de usuario, PDF pág. 15 / impresa 3** *(«The default IP address of the device is 192.168.1.64»)* |
> | Máscara | `255.255.255.0` · **GW** `192.168.1.1` | `.docx` §3 *(no aparece en los dos PDF)* |
> | Usuario | `admin` *(no se puede cambiar)* | `.docx` §3 |
> | Contraseña | **ninguna — activación obligatoria** | ✅ **Manual, PDF pág. 13** |
> | Longitud de contraseña | **mínimo 8**, con mayúsculas, minúsculas, números y símbolos | ✅ **Manual, PDF págs. 13 y 15**, literal |
> | Puertos: HTTP `80` · HTTPS `443` · RTSP `554` · SDK `8000` · descubrimiento SADP **UDP `37020`** | | 🟡 `.docx` §4 — **el número `37020` NO aparece en ninguno de los dos PDF oficiales** *(medido: cero coincidencias)*. Se publica citando su fuente, que es la recopilación del responsable, no el fabricante |
>
> > 🔴 **`SIN VERIFICAR`: el tope de 16 caracteres de la contraseña.** El `.docx` dice *«8 a 16»*. En
> > las **110 páginas** del manual oficial la cadena `16 characters` **no aparece ni una vez**
> > (medido); lo único que el fabricante escribe es *«a minimum of 8 characters»*. **El mínimo es
> > dato; el máximo no.** Se anota por si alguien elige una contraseña larga y la cámara la rechaza:
> > no sería un fallo, sería este límite sin confirmar.
>
> ### ⚠️ DOS AVISOS DE VERSIÓN, y valen para TODO este manual
>
> 1. 🔴 **El manual de usuario que tenemos es de la `5.7.20`; el firmware vigente de la cámara es la
>    `5.7.23`** *(`.docx` §1: build 260320, 20/03/2026)*. **El manual es MÁS VIEJO que el firmware**,
>    así que **las rutas de menú que aquí se citan pueden no coincidir con lo que el técnico vea en
>    pantalla.** Si una ruta no está donde se dice, **lo primero que hay que sospechar es esto**, no
>    un error del técnico. Cada ruta de este documento lleva su página; si discrepa, **manda la
>    pantalla** y se anota la diferencia.
>    > *(El propio manual lo avisa de sí mismo, pág. 2: «subject to change without notice, due to
>    > firmware updates or other reasons».)*
> 2. 🟡 **SADP está DESCONTINUADA** — sin actualizaciones desde abril de 2026. **La sustituye
>    `HiTools Delivery`** *(V2.1.1.2)*. **Aquí se documenta SADP porque es la que está instalada y
>    con ella se entra**, y funciona; se deja escrito el relevo para que dentro de seis meses nadie
>    busque una herramienta muerta ni se extrañe de no encontrar descargas.
>
> **Con la cámara ya accesible, sigue en el §4 Paso 0** —que es la comprobación que decide si todo
> el camino de `J16` sirve— y luego el resto del apartado 4.

> ## 📜 EL CONTRATO, ENTERO Y EN POSITIVO — **DE CADA CÁMARA EL SISTEMA CONSUME UN BIT** (D-12, 05/09/2026)
>
> **Esto es lo primero que hay que entender de este manual, y es más pobre de lo que suena
> «cámara IA».** Va aquí arriba, y no en una nota al pie, porque es lo que decide qué se puede
> prometer y qué no.
>
> ### Lo que el sistema consume
>
> ```text
>   [ CAMARA HIKVISION ]                       [ TARJETA STM32 ]
>          |                                          |
>    analitica embebida                                |
>    (dentro de la camara)                             |
>          |                                          |
>          v                                          |
>     rele de alarma  ---- DOS HILOS, UN BIT ---->  digitalRead(pin)
>          1A / 1B         contacto seco             activo en ALTO
>
>   Y NADA MAS CRUZA ESA FRONTERA.
> ```
>
> **UN CONTACTO SECO POR CÁMARA. Un bit: abierto o cerrado.** Eso es todo lo que el controlador
> recibe de una cámara, y todo lo que puede recibir por este camino.
>
> ### Lo que NO hay — y no es una carencia temporal, es el diseño
>
> | | |
> |---|---|
> | ❌ **No hay red entre la cámara y el controlador** | En el poste no hay switch, ni router, ni cable de datos. El `RJ45` de la cámara se usa **una vez, en taller**, para configurarla; después no va a ninguna parte |
> | ❌ **No hay imagen, ni vídeo, ni instantánea** | Nada en el controlador recibe, guarda ni reenvía un solo píxel |
> | ❌ **No hay analítica en el controlador** | El STM32 no clasifica nada: lee un pin. La clasificación *vehículo / persona* ocurre **dentro de la cámara**, y el resultado llega convertido en ese único bit |
> | ❌ **No hay ONVIF, ni ISAPI, ni SDK, ni FTP en el equipo** | La ficha de la cámara los lista *(pág. 3: «Open Network Video Interface (Profile S, Profile G), ISAPI, SDK»)* — **son capacidades de la cámara que este sistema no usa** |
>
> > ✅ **MEDIDO el 05/09, no heredado.** En todo el fuente del `ESP32` de expansión
> > (`01_Firmware/ESP32_Expansion/src` + `include`, 8 `.cpp` y 8 `.h`) hay **cero** coincidencias de
> > `WiFi`, `HTTPClient`, `WebServer`, `AsyncWebServer`, `WiFiClient`, `ONVIF`, `RTSP`, `esp_camera`,
> > `MQTT` y `Ethernet`. Sus únicos `#include` de sistema son `Arduino.h`, `BluetoothSerial.h`,
> > `Wire.h`, `Preferences.h`, `esp_task_wdt.h` y `esp_system.h`. Y en el STM32, las tres entradas de
> > cámara se leen con `digitalRead()` y nada más — símbolo `camara_leerPin()` de `botones.cpp`.
> >
> > ⚠️ **El matiz honesto, porque el dato bruto dice otra cosa y alguien lo va a mirar:** en el
> > `firmware.map` del ESP32 **sí** aparecen enlazadas `libesp_wifi.a`, `liblwip.a` y
> > `libnet80211.a`. **No las pide este proyecto**: las arrastra el arranque del ESP-IDF y la capa de
> > **coexistencia del Bluetooth** (`libesp_system.a(startup.c.obj)` pidiendo `g_coex_adapter_funcs`,
> > y `libbt.a(bt.c.obj)` pidiendo `esp_coex_version_get`). **Ningún objeto del proyecto aparece como
> > causa**, y no hay una sola llamada a `WiFi.begin()` ni a `esp_wifi_start()`. Está enlazada y
> > **nunca se arranca**.
>
> ### 🟢 EL SOPORTE DE ACCIDENTES Y LA AUDITORÍA **SÍ SON POSIBLES** — EN LA CÁMARA, NO EN EL CONTROLADOR
>
> > ⛔ **CORREGIDO EL 05/09, y se deja escrito cómo se cayó.** Este bloque decía:
> > ~~«Lo que se pierde: sin imágenes NO hay soporte de accidentes ni auditoría»~~ →
> > **FALSO, y la ficha lo desmiente en una línea.** Se escribió razonando desde *«el controlador no
> > recibe imágenes»* —que es cierto— hasta *«luego no hay imágenes»* —que no se sigue—. La cámara
> > **graba por su cuenta**, y eso no depende de nuestro firmware ni de una sola línea de código.
> > Es §4 otra vez: una conclusión plausible publicada sin medir la fuente que la contradecía.
>
> ✅ **VERIFICADO leyendo el PDF de la ficha, no citando de memoria:**
>
> | qué | dónde lo dice |
> |---|---|
> | **Ranura microSD/SDHC/SDXC de hasta 512 GB**, a bordo | Ficha, **pág. 3**, fila *On-board Storage*: *«Built-in memory card slot, support microSD/microSDHC/microSDXC card, up to 512 GB»* |
> | **Sabe grabar y capturar por evento**, y subir a FTP o NAS | Ficha, **pág. 4**, fila *Linkage Method*: *«Upload to FTP/memory card/NAS, notify surveillance center, trigger recording, trigger capture, send email»* |
> | **Almacenamiento en red y ANR** *(rellena los huecos cuando vuelve el enlace)* | `DS-2CD2683G2-IZS_Ficha_Tecnica_y_Configuracion.docx`, §2.3 |
>
> **Lo que eso significa, dicho sin adornos:** si el responsable quiere **soporte de accidentes o
> auditoría**, la cámara que ya ha comprado **puede darlo** — poniéndole una tarjeta de memoria y
> configurando la grabación. **No hay que tocar el firmware, ni la placa, ni el contrato del bit.**
>
> ⚠️ **Pero es un CAMINO SEPARADO, y hay que saber lo que arrastra antes de contar con él:**
>
> * **Las imágenes no pasan por el controlador.** El semáforo no las ve, no las guarda y no las
>   reenvía. Siguen sin existir *para el sistema de control*: el bit sigue siendo el bit.
> * **La tarjeta microSD no está comprada** — no está en `15_Lista_de_Compras_Hardware.md`, y a
>   512 GB no es una compra trivial. *(Nota de fuentes: la ficha de 2023 dice **512 GB** y el `.docx`
>   del 05/09 apunta que alguna edición indica 256 GB. **Se compra mirando la ficha del lote que
>   llegue**, no este manual.)*
> * **Alguien tiene que ir a recogerla.** Sin red en el poste, las imágenes se sacan **subiendo al
>   equipo y retirando la tarjeta**. La alternativa —FTP o NAS— **exige red hasta el poste, que hoy
>   no existe**.
> * **Nadie ha definido la política**: cuántos días se guardan, quién las mira, quién las borra, y
>   qué pasa con la privacidad de quien pase por delante. 🔴 **Eso no lo decide este manual.**
>
> > 🔴 **SIN VERIFICAR:** que la grabación por evento y la salida de alarma **puedan armarse a la vez
> > sobre la misma regla** de intrusión. Las dos aparecen como *Linkage Method* de la misma regla
> > (manual de usuario, **PDF pág. 79 / impresa 67**), lo que hace pensar que sí, pero **no está
> > comprobado con la cámara delante** y no se escribe como hecho.
>
> ### 🎯 LA CONSECUENCIA QUE ASCIENDE A ESTE MANUAL: TODA LA INTELIGENCIA VIVE EN LA CONFIGURACIÓN
>
> Si lo único que cruza es un bit, **lo que decide si ese bit vale algo es cómo quede configurada la
> cámara**: la **zona** que se dibuja, la **analítica** que se elige, la **sensibilidad**, el
> **filtro de tamaño / clasificador** y el **horario**. Eso —y sólo eso— es lo que convierte el bit
> en *«hay un vehículo esperando»* en vez de *«se movió una sombra»*.
>
> > 🔴 **Por eso este documento NO es un manual de apoyo: es el ENTREGABLE PRINCIPAL de la parte de
> > cámaras.** No hay ninguna otra capa donde corregir un error de parametrización. Un firmware
> > impecable leyendo un bit mal configurado da un cruce que abre solo cuando pasa un pájaro. **Lo
> > que aquí se configure mal, no lo arregla nadie más abajo.**
>
> ### ⚠️ UNA CÁMARA = UNA SALIDA = **UN SOLO SIGNIFICADO**
>
> ✅ **Verificado en la ficha oficial** (`DS-2CD2683G2-IZS_Datasheet_V5.5.113_20230303.pdf`, pág. 3,
> fila *Alarm*, leída del PDF): **`1 input, 1 output (max. 24VDC/24 VAC, 1 A)`**. **Una salida. No
> tres.**
>
> Así que una cámara **no puede decir dos cosas distintas**: su relé está cerrado o abierto, y ese
> estado significa **una** cosa que se decide al configurarla. ~~Con **dos cámaras por cruce** hay
> exactamente **DOS significados para todo el cruce**, y son un recurso escaso que se reparte una vez.~~
> → ✏️ **11/09 — caducado dos veces:** hoy son **cuatro cámaras, dos por poste** (`D-25`), y **no se
> reparten significados**: `D-13` (05/09) les da **a todas la misma configuración** y **el
> significado lo pone el estado del semáforo**, no la cámara. El firmware lo confirma: las dos
> entradas de `J16` se leen en el mismo bucle (`CAM_J16[2]`, `camaras_actualizar()`) y hacen lo mismo.
>
> | | |
> |---|---|
> | Significado **1** | 🟢 **TOMADO:** presencia de vehículo (demanda de paso) |
> | Significado **2** | ~~🟡 **SIN DECIDIR** — es la pregunta **A-1** de `DECISIONES.md`, y **no se decide en este manual**~~ → ✅ **`A-1` CERRADA por `D-13`**: no hay segundo significado; todas las cámaras llevan la misma regla |
>
> **Antes de dibujar una zona hay que saber qué significa esa cámara**, porque la zona, la analítica
> y el umbral se eligen **en función del significado** — no al revés. Un bit configurado para
> *«pasó alguien»* no sirve para *«hay alguien esperando»*, y volver atrás obliga a subir al poste.
>
> > 🛑 **Y la regla de seguridad que ordena los dos significados, sea cual sea el segundo:** una
> > cámara **desconectada, sin alimentación o averiada deja el pin en reposo**, y el reposo se lee
> > como **«no hay nadie»**. **El pin no distingue silencio de vía libre.** Por eso **nada donde la
> > AUSENCIA de presencia AUTORICE algo**: una cámara sólo puede **pedir**, nunca **permitir**. Es
> > `SFTY-27`, y es la razón por la que el todo-rojo de despeje es temporizado y no depende de que
> > una cámara diga que el tramo está vacío.

> ## 🎥 LA CÁMARA COMPRADA ES UNA `DS-2CD2683G2-IZS` — Y SÍ TIENE SALIDA DE ALARMA (05/09/2026)
>
> **La pregunta que había que contestar antes que ninguna otra, porque de ella colgaba todo el
> cableado:** este manual, `15_Lista_de_Compras_Hardware.md` y el mapa de `J16` **dan por hecho que
> la cámara entrega un CONTACTO SECO**. Si el modelo comprado no lo llevara, el camino de `J16` no
> serviría y habría que irse al relé de un NVR o a un evento por red hacia el `ESP32` — **otro
> diseño**.
>
> ```text
>   FICHA OFICIAL Hikvision, DS-2CD2683G2-IZS, V5.5.113 (03/03/2023), pagina 3, fila "Alarm":
>
>       Alarm      1 input, 1 output  (max. 24VDC/24 VAC, 1 A)
> ```
>
> ### ✅ **SÍ. Tiene UNA salida de alarma y UNA entrada. El diseño de `J16` SIGUE EN PIE.**
>
> Y los bornes son **exactamente los que este manual ya citaba**: la Guía Rápida oficial
> (`UD40284B`, 15/11/2024, pág. 8) rotula `ALARM OUT` como *«1A and 1B»* y `ALARM IN` como *«IN1 and
> GND1»*. **No hay que cambiar ni un hilo de lo escrito.**
>
> ⚠️ **Una sola salida por cámara.** La ficha dice `1 output`, no tres: **una cámara = un contacto =
> una demanda**. Los *«2A/2B, 3A/3B»* que menciona la Guía Rápida son de otros modelos de la misma
> familia — la propia Guía avisa: *«The interface varies with the models. Please refer to the product
> datasheet»*, y la ficha de ÉSTE dice uno.
>
> ### 🔴 PERO TENER LA SALIDA NO BASTA: LA ANALÍTICA TIENE QUE PODER ACCIONARLA — y ahí las dos fuentes oficiales NO dicen lo mismo
>
> **Es el eslabón del que cuelga todo el diseño**, y hay que enseñarlo tal cual está, porque las dos
> fuentes son del propio fabricante y se contradicen:
>
> | fuente oficial | qué dice |
> |---|---|
> | Manual de usuario, **pág. 67**, *Linkage Method Settings* | **`Trigger Alarm Output` EXISTE** como método de enlace… con esta nota literal debajo: *«**This function is only supported by certain models**»* |
> | **Ficha del modelo, pág. 4**, fila *Linkage Method* | *«Upload to FTP/memory card/NAS, notify surveillance center, trigger recording, trigger capture, send email»* — **y NO menciona la salida de alarma** |
> | Ficha del modelo, **pág. 3**, fila *Alarm* | `1 input, **1 output**` — **el hardware está** |
>
> 🔴 **Veredicto honesto: `SIN VERIFICAR`. La cámara TIENE la salida; que la regla de intrusión pueda
> ENLAZARSE a ella no lo confirma ninguna fuente de este modelo, y la fila que debería listarlo no lo
> lista.**
>
> 🟢 **Se resuelve en DIEZ MINUTOS, sin desmontar nada, y es lo PRIMERO que hay que hacer cuando
> llegue la cámara** — antes de dibujar zonas y antes de tocar un hilo:
>
> ```text
>   1. Configuration -> Event -> Smart Event -> Intrusion Detection
>   2. Marcar Enable y bajar a "Linkage Method" / "Metodo de Vinculacion"
>   3. MIRAR SI EXISTE LA CASILLA  "Trigger Alarm Output" / "Disparar Salida de Alarma"
>
>      SI ESTA  -> el diseno de J16 vale entero. Se sigue con el paso 4 del apartado 4.
>      NO ESTA  -> SE PARA Y SE AVISA. El contacto seco no se puede accionar desde la
>                  analitica, y la demanda tendria que entrar por otro camino
>                  -relé de un NVR, o evento de red hacia el ESP32-.  Eso es OTRO DISENO
>                  y NO se decide en este manual.
> ```
>
> 🛑 **Si esa casilla no está, no se cablea nada a `J16` por parte de la cámara** — no porque sea
> peligroso, sino porque **no serviría**, y un hilo puesto «por si acaso» en un conector que lleva
> 12 V crudos en el p1 es exactamente el riesgo que el aviso de arriba intenta evitar.
>
> ### 📏 La escala de tres niveles, la misma que usa el documento 17 — y aquí hace falta una raya más
>
> | | qué significa |
> |---|---|
> | ✅ **MEDIDO** | **medido en ESTE proyecto** — en cobre, en banco o en el fuente. Es lo único que ha tocado el equipo real |
> | 📖 **ESCRITO** | **lo dice la ficha o el manual oficial de Hikvision, y se puede citar** — pero **nadie lo ha visto en una cámara de este proyecto**. Un dato de fabricante es una afirmación, no una medida |
> | 🔴 **SIN VERIFICAR** | **ninguna fuente citable lo dice.** No se rellena con lo que parece razonable |
>
> 🛑 **`ESCRITO` NO ES `MEDIDO`, y aquí importa más que de costumbre**, porque **todo lo que este
> manual sabe de la cámara es `ESCRITO`**: la cámara no ha estado nunca delante de esta tarjeta. Lo
> `MEDIDO` de este manual es la TARJETA —`M3`, el paso 21, el fuente—, no la óptica.
>
> ### 🔴 Y lo que se ha CAÍDO al poner la ficha real delante: dos frases que nadie había comprobado
>
> | la frase | qué dice la fuente oficial |
> |---|---|
> | *«la salida de la AcuSense es **configurable `NO`/`NC`**»* — está en `CLAUDE.md` §9.bis, en el §4 Paso 4 de este manual y sostiene decisiones desde el 31/08 | 🔴 **SIN VERIFICAR.** La ficha dice `1 output (max. 24VDC/24 VAC, 1 A)` **y nada más**. En el manual de usuario oficial (`UD28967B-C`, v5.7.20) el desplegable **`Alarm Type` existe sólo para la ENTRADA** (*Set Alarm Input*, pág. 44, paso 3); los únicos parámetros documentados de la **salida** son `Alarm Output No.`, `Alarm Name` y `Delay` (*Automatic Alarm*, pág. 68). **Las palabras `Normally Open` / `Normally Closed` no aparecen ni una vez en las 110 páginas** |
> | *«clasificador ☑ Vehículo / ☐ Humano sobre **Detección de Intrusión**»* — es el §4 Paso 3, y es lo que hace que el `ENSAYO 2` deba salir bien | 🔴 **SIN VERIFICAR para ESA analítica.** El manual oficial documenta **`Detection Target`** en *Line Crossing*, *Region Entrance* y *Region Exiting*… **y NO en `Set Intrusion Detection`** (pág. 48-49), cuyas únicas reglas listadas son `Sensitivity` y `Threshold`. Ver **§4 Paso 3** |
>
> 🔎 **Y antes de escribir esos dos `SIN VERIFICAR`, se descartó al buscador (`CLAUDE.md` §4):** los
> PDF son texto extraíble —`Alarm Type` **sí** aparece, y `Detection Target` **sí** aparece cuatro
> veces—, y una búsqueda de `"NO/NC"` restringida a `hikvision.com` **devuelve resultados de sobra**
> en fichas de radares, centrales de alarma y barreras, donde Hikvision lo escribe así de claro:
> *«4 Relay Outputs(NO/NC)»*. **El buscador sabe encontrar `NO/NC` en las fichas de Hikvision. En la
> de esta cámara no está.** Un cero aquí sí se lee como «no lo dice».
>
> 🟢 **Ninguna de las dos bloquea nada, y esto es lo que hay que entender:** las dos **las contesta la
> propia cámara en diez minutos**, con los ensayos que este manual **ya tiene escritos** —el
> `ENSAYO 1` mide si el contacto está abierto o cerrado en reposo y cuánto dura el pulso; el
> `ENSAYO 2` mide si un peatón dispara—. **No hace falta un documento nuevo: hace falta enchufar la
> cámara y anotar dos números.** Ver §6, donde los dos ensayos llevan ya su casilla.
>
> ### 👷 Y son DOS TRABAJOS DISTINTOS, que casi nunca hace la misma persona
>
> | | quién y dónde | qué hace | dónde está en este manual |
> |---|---|---|---|
> | **A. CONFIGURAR LA CÁMARA** | en **taller**, con un portátil y cable de red, **una sola vez** | IP y contraseña · zoom y foco · analítica, zona y sensibilidad · vincular el evento a la salida de alarma · desarmar los eventos básicos | **§4**, pasos 1 a 4 |
> | **B. CABLEAR** | en **el poste o el banco**, con destornillador y multímetro, **en cada equipo** | tapar `J16` p1 · llevar `1A`/`1B` al pin de señal y a los 3,3 V del borne contiguo · alimentar la cámara | **§2.1**, **§6** y el bloque 🛑 de arriba |
>
> 🛑 **El que cablea no abre el navegador de la cámara, y el que la configura no toca `J16`.** Por eso
> el aviso de los 12 V va arriba del todo y repetido: **es del trabajo B**, y quien lo hace puede no
> haber leído nunca el §4.

> ## 🛑 AVISO DE POLARIDAD — EL CONTACTO SECO **NO** VA CONTRA `GND` (31/08/2026)
>
> **MEDIDO EN EL FUENTE**, `01_Firmware/Maestro/src/botones.cpp` (idéntico en las dos puntas). El
> lector de cámara se mudó aquí el 31/08 desde `modo_inteligente.cpp`, porque una entrada física
> existe desde que arranca la tarjeta y no solo mientras un modo está puesto:
>
> ```
> :108  if (digitalRead(pin) == HIGH) {          // <-- ACTIVO EN ALTO
> :176  pinMode(CAM_DEMANDA_PIN, INPUT);         // <-- INPUT PELADO, sin pull-up
> :177  pinMode(CAM_C_PIN, INPUT);               //     las tres, en el arranque
> :178  pinMode(CAM_D_PIN, INPUT);
>
> // el porque, en pines.h:41-46 (N-67):
> //   PB0 lleva R64 de 10 kOhm A MASA -pull-DOWN- y C25 de 100 nF tambien a masa;
> //   la bornera J14 saca ese pin JUNTO A 3,3 V. O sea que el contacto seco de la
> //   camara va entre los dos bornes de J14 y CIERRA A 3,3 V.
> ```
>
> El reposo del pin **ya lo fija la placa** con `R64` de 10 kΩ a masa. El contacto de la cámara tiene
> que **cerrar el pin contra los 3,3 V** de la bornera, **no contra masa**.
>
> ### 🟢 04/09 — Y ahora la polaridad **NO** la decide el fuente: LA DECIDE EL COBRE, y está medido
>
> Hasta hoy esta regla se sostenía en `botones.cpp`. **El banco la midió en la placa** (paso 20):
>
> ```text
>   J16 p10  ->  9,93 kOhm a masa    y  0 V con la tarjeta energizada
>   J16 p12  ->  9,94 kOhm a masa    y  0 V con la tarjeta energizada
>   Los CUATRO pines de J16 son identicos:  10K a masa + 100 nF
>   Y los cuatro tienen 3,3 V en la posicion de al lado:  J16.4 / .7 / .9 / .11
> ```
>
> **Un pull-down real de 10 kΩ y 3,3 V en el borne contiguo describen un solo gesto:** cerrar el
> contacto seco **contra esos 3,3 V**. **ACTIVO EN ALTO**, y ya no por deducción del firmware —que
> además coincide: `INPUT` pelado, `== HIGH`—, sino **por lo que hay soldado en la placa**.
>

> **Qué pasaba si se seguía el texto viejo:** con el contacto entre el pin y `GND`, el pin está en
> `LOW` en reposo **y sigue en `LOW` al detectar**. **La cámara no dispara jamás** y no hay síntoma:
> el equipo se comporta como si no hubiera cámara. Un ensayo de taller lo daría por bueno.
>
> 🔴 **Y eso es exactamente lo que hacía peligroso al error hermano de `MANUAL_USUARIO.md`**, que
> mandaba las cámaras a `PB9`/`PB13` —**los dos canales del mando de relés**—: mientras el cableado
> estuviera *«a `GND`»* la cámara no disparaba y nadie veía nada; el día que alguien **«arreglara» el
> cableado**, tres pulsos de tráfico dentro de la ventana de 12 s empezarían a **componer secuencias
> del mando y a cambiar el modo del semáforo solos**. Los dos errores se corrigen juntos o el arreglo
> es peor que el defecto. Ver `MANUAL_USUARIO.md` §6.

---

> ### 🛑 LAS CÁMARAS 2 Y 4 NO ESTÁN OPERATIVAS EN V9.0
>
> **La cámara de umbral no está en V9.0, y no es solo firmware: no hay dónde conectarla.**
> Medido el 27/08 sobre el esquemático: **`PB8` alimenta el LED testigo `D5` a través de `R16`
> 1 kΩ** — no es una bornera ni una entrada optoacoplada. El manual lo daba por una entrada
> *«en reposo»*, y eso venía de leer un esquemático incompleto (`roadmap.md` N-64).
>
> Para tenerla harían falta **dos cosas**, y ninguna es un `pinMode`: **(1)** una entrada física
> —un hilo desde el pad de `PB8` retirando `R16`/`D5`, o desde uno de los cuatro pines sin
> cablear (`PA11`, `PA12`, `PA15`, `PC13`)— y **(2)** un **comando de radio** que lleve la cuenta
> del tramo al Maestro, que es quien decide (`roadmap.md` N-59). Sin el comando, leer el pin es
> medio camino.
>
> Mientras tanto el despeje se hace **por tiempo** (`cfgDespejeSeg`), que es el criterio
> conservador: la cámara de umbral daría **eficiencia**, no seguridad.


## 1. Arquitectura Autónoma para Semáforos Móviles (Sin Raspberry Pi ni Micro-PC)

La cámara Hikvision AcuSense incorpora un procesador de inteligencia artificial con **clasificador Deep Learning de vehículos vs. humanos**. No se requiere ningún computador externo, switch Ethernet ni direccionamiento IP en obra:

```text
 ┌─────────────────────────────────────────────────────────────────────────────┐
 │                ARQUITECTURA AUTÓNOMA DE DETECCIÓN VEHICULAR                 │
 ├─────────────────────────────────────────────────────────────────────────────┤
 │                                                                             │
 │   [ HIKVISION DS-2CD2683G2-IZS  -  AcuSense, analitica embebida ]           │
 │           │                       8 MP - varifocal motorizada 2,8-12 mm     │
 │           ▼ (Intrusion Detection sobre el BARRIDO DE LA PLUMA, SIN FILTRO)  │
 │             ~~Clasificador ☑ Solo Vehiculo~~ <-- TACHADO 07/09 por D-13:    │
 │             bajo la pluma importa TAMBIEN una moto o una persona, y el      │
 │             Detection Target no esta documentado para Intrusion.  Ver 4.    │
 │   [ SALIDA DE ALARMA (Bornes 1A / 1B - Contacto Seco, 24 V / 1 A max) ]     │
 │             ^^^^ que sea "N/O" y CONFIGURABLE esta SIN VERIFICAR: ver 4     │
 │           │                                                                 │
 │           ▼ (2 Hilos directos por cámara)                                  │
 │   [ TARJETA CONTROLADORA STM32 - bornera J16 p10 (PB14) / p12 (PB15) ]      │
 │     ~~bornera J14, entrada PB0~~ <-- TACHADO 07/09: D-2 / D-3.             │
 │     J14 NO lo vigila CAM_CIEGA/CAM_PEGADA.                                  │
 │           │                                                                 │
 │           ▼                                                                 │
 │   [ LÓGICA VIAL: Demanda Vehicular + Despeje Todo-Rojo cfgDespejeSeg ]      │
 │                                                                             │
 └─────────────────────────────────────────────────────────────────────────────┘
```

---

## 1.1 La cámara real, dato a dato — y qué significa cada dato AQUÍ

**Fuente única de esta tabla, y se cita entera para que cualquiera pueda ir a comprobarla:**

| | |
|---|---|
| **Ficha técnica** | `DS-2CD2683G2-IZS_Datasheet_V5.5.113_20230303.pdf`, 6 páginas, descargada de `hikvision.com` el 05/09/2026 |
| **Manual de usuario** | `UD28967B-C_Network-Camera_User-Manual_5.7.20_20240131.PDF`, 110 páginas, misma procedencia |
| **Guía rápida** | `UD40284B_Baseline_1-3_Series_Multilingual_Quick_Start_Guide_20241115.pdf`, 40 páginas |

> 🛑 **Todas las filas de abajo son 📖 `ESCRITO` salvo donde diga otra cosa.** Son datos del
> fabricante sobre su producto: **se pueden citar y no se han medido**. Este manual no ha tenido una
> `DS-2CD2683G2-IZS` delante ni una sola vez.

### 1.1.1 Lo que decide si el diseño de `J16` vale — la alarma

| qué | valor | nivel y fuente |
|---|---|---|
| **¿Tiene salida de alarma (contacto seco)?** | ✅ **SÍ. `1 output`** | 📖 **ESCRITO** — ficha, pág. 3, fila *Alarm* |
| **¿Y entrada de alarma?** | ✅ **SÍ, `1 input`** (`IN1` + `GND1`). **Hoy NO se cablea**, y el porqué está escrito en **§4.bis.6**: es la vía de `D-14`, y le faltan dos cosas — el régimen eléctrico (🔴 **`SIN VERIFICAR`**: no lo publica ninguna de las tres fuentes del fabricante) y qué canal de la placa se le asigna | 📖 **ESCRITO** — ficha, pág. 3, fila *Alarm* |
| **Régimen del contacto** | **máx. `24 V DC` / `24 V AC`, `1 A`** | 📖 **ESCRITO** — ficha, pág. 3 |
| **Bornes físicos** | **`1A` + `1B` son UNA PAREJA — los dos bornes de UN contacto**, no dos salidas · la entrada es **`IN1` + `GND1`**, otra pareja | 📖 **ESCRITO, y leído del diagrama el 05/09** — Guía rápida `UD40284B`, **PDF pág. 9 de 40, impresa 8**: *«1A and 1B, 2A and 2B, 3A and 3B are **three pairs** of alarm outputs»*. **Cómo se cablea: §4.bis.5** |
| **¿`NO`/`NC` configurable?** | 🔴 **SIN VERIFICAR** — ver el bloque 🎥 de la cabecera y **§4 Paso 4** | — |
| **¿Duración del pulso configurable a `1 s`?** | 🔴 **SIN VERIFICAR.** Existe el parámetro `Delay` (*«the time duration that the alarm output remains after an alarm occurs»*, manual pág. 68) — **pero el manual no publica sus valores seleccionables**, y `1 s` puede no estar entre ellos | 📖 el parámetro, **ESCRITO** · 🔴 el valor `1 s`, **SIN VERIFICAR** |

> 🟢 **El régimen sobra por tres órdenes de magnitud, y esa parte es una cuenta, no una opinión.**
> El contacto va a conmutar **3,3 V** contra un pull-down de **10 kΩ** (`M3`, MEDIDO en cobre el
> 03-04/09):
>
> ```text
>   Lo que el contacto aguanta (ficha):     24 V   /  1 A
>   Lo que este equipo le pide:            3,3 V   /  3,3 / 10000 = 330 uA
>
>   Margen en tension:   24 / 3,3    =   7,3 veces
>   Margen en corriente:  1 / 0,00033 = 3030 veces
> ```
>
> **Un contacto de 24 V y 1 A no se rompe conmutando 3,3 V.** Por ese lado no hay ningún problema.
>
> 🔴 **Pero el margen enorme abre la pregunta CONTRARIA, y es la que hay que hacerle a quien firme:
> ¿hay corriente SUFICIENTE?** Un relé seco conmutando **330 µA a 3,3 V** trabaja en lo que la
> industria llama *carga seca* (*dry circuit*, típicamente por debajo de 1 mA), un régimen donde los
> contactos sin baño de oro pueden criar una película de óxido y **dejar de hacer contacto fiable al
> cabo de meses** — un fallo intermitente, en la calle, de los peores de diagnosticar.
>
> **La ficha NO publica corriente mínima de conmutación ni material de contacto: eso es
> 🔴 `SIN VERIFICAR`, y no se rellena adivinando.** Se anota aquí porque:
> 1. **No bloquea nada hoy** — el `ENSAYO 4` ya ejerció el gesto real en banco el 04/09 y funcionó.
>    Lo que ese ensayo **no** puede medir es el envejecimiento.
> 2. **Es una pregunta para quien firme la V2 de la placa, no una decisión de este manual.** Y
>    **tiene un vecino:** la línea `E1` de `15_Lista_de_Compras_Hardware.md` propone **2K2 en serie**
>    en las cinco entradas de campo; con ella la corriente por el contacto **baja** a
>    `3,3 / 12200 = 270 µA`. Las dos cuentas tiran en el mismo sentido y **hay que mirarlas juntas**.

### 1.1.2 Alimentación — y el número que hay que poner delante del responsable

| qué | valor | nivel y fuente |
|---|---|---|
| **Consumo a 12 V DC** | **`1,08 A`, máx. `13 W`** | 📖 **ESCRITO** — ficha, pág. 4, *Power Consumption and Current* |
| **Tensión admisible** | **`12 V DC ± 25 %`** → de **`9 V` a `15 V`** | 📖 **ESCRITO** — ficha, pág. 4, *Power Supply* |
| **PoE** | **`802.3at` (PoE+), Clase 4**, 42,5–57 V, máx. `15 W` | 📖 **ESCRITO** — ficha, pág. 4 |
| **Conector de alimentación** | jack coaxial **Ø 5,5 mm** | 📖 **ESCRITO** — ficha, pág. 4 |

> 🟢 **La buena noticia, y es real:** el rango `9 – 15 V` **cubre entero el vaivén de una batería de
> plomo de 12 V** (de ~10,5 V descargada a ~14,4 V en carga). **La cámara se alimenta directamente
> de la batería del equipo: no hace falta convertidor.**
>
> 🛑 **Y la que hay que decir en voz alta, porque es una decisión del responsable y no de este
> manual: `13 W` POR CÁMARA, DE FORMA CONTINUA, EN UN EQUIPO QUE VA A BATERÍA.**
>
> ```text
>   ~~Por poste, una camara:      13 W~~                  <-- CADUCADO 11/09 (D-25)
>   ~~En 24 h:                    13 W x 24 h  =  312 Wh~~
>   ~~A 12 V, eso es:             312 / 12     =   26 Ah/dia   SOLO LA CAMARA~~
>
>   11/09, D-25 -- DOS camaras por poste:
>   Por poste, dos camaras:     2 x 13 W     =   26 W
>   En 24 h:                    26 W x 24 h  =  624 Wh
>   A 12 V, eso es:             624 / 12     =   52 Ah/dia   SOLO LAS CAMARAS
> ```
>
> ~~**26 Ah al día por poste**~~ → **11/09 (`D-25`): 52 Ah al día por poste, el doble, porque son dos
> cámaras** — **es una cifra que decide el tamaño de la batería y del panel**, y es del
> mismo orden o mayor que todo lo demás del equipo junto. **No es un problema de este manual y no se
> resuelve aquí**: se pone el número delante de quien dimensiona la energía, que es lo que §2.quater
> de `CLAUDE.md` pide —**medir la causa antes de ofrecer opciones**—.
>
> ⚠️ **Lo que sí es una regla, y ya está escrita para el `ESP32`:** la cámara **NO cuelga del
> `LM7805` de la tarjeta**. `1,08 A` por un regulador lineal de 12 a 5 V son **7,5 W de disipación**;
> por el de 12 V no pasa, porque va directa a batería. Es la misma lección de la línea `A5` de la
> lista de compras: **lo que consume de verdad se cuelga de la batería, no del regulador que mantiene
> vivo al STM32 que gobierna el semáforo.**
>
> 🔵 **`13 W` es el MÁXIMO de la ficha, no el consumo medio** — incluye los IR a plena potencia de
> noche. De día será menos. **Cuánto menos: 🔴 `SIN VERIFICAR`**, y se mide con una pinza
> amperimétrica el día que la cámara esté montada, no antes.

### 1.1.3 Óptica y alcance — lo único que hace falta para encuadrar la vía

| qué | valor | nivel y fuente |
|---|---|---|
| **Resolución** | **8 MP, `3840 × 2160`**, sensor `1/2.8"` | 📖 **ESCRITO** — ficha, pág. 2 |
| **Óptica** | **varifocal MOTORIZADA, `2,8 – 12 mm`**, `F1.6`, iris fijo | 📖 **ESCRITO** — ficha, pág. 2 |
| **Campo de visión** | **horizontal `108°` a `30°`** · vertical `56°` a `17°` · diagonal `131°` a `35°` | 📖 **ESCRITO** — ficha, pág. 2 |
| **DORI** *(a `2,8 mm` … a `12 mm`)* | **Detectar `97 – 290 m`** · Observar `38 – 115 m` · Reconocer `19 – 58 m` · Identificar `9 – 29 m` | 📖 **ESCRITO** — ficha, pág. 2 |
| **Luz de apoyo** | **IR `850 nm`, alcance hasta `60 m`.** Tipo de luz suplementaria: **IR y sólo IR** | 📖 **ESCRITO** — ficha, pág. 2 |
| **Intemperie** | **`IP67`**, **`IK10`**, **`-30 °C` a `+60 °C`** | 📖 **ESCRITO** — ficha, págs. 4 y 5 |
| **Tamaño y peso** | **`308,5 × 97,9 × 93 mm`**, **`1.385 g`** | 📖 **ESCRITO** — ficha, pág. 4 |

> 🟡 **La óptica va sobrada para encuadrar la vía — pero OJO CON LEER EL DORI COMO ALCANCE ÚTIL.**
> A gran angular el escalón **Detectar** son **97 m**, así que **un carril de aproximación cabe
> entero** y el encuadre no es el problema.
>
> > ⛔ **CORREGIDO EL 05/09.** Aquí ponía: ~~«La demanda vehicular sólo pregunta “¿hay un vehículo
> > esperando?” — eso es **Detectar**, el escalón más bajo del DORI, y a gran angular son 97 m»~~ →
> > **la equivalencia es falsa, y es de las que se cuelan por sonar razonables.**
> >
> > **`Detectar` en la norma DORI significa *«hay un objeto»*, no *«es un vehículo»*.** Nuestro bit
> > **no** lo levanta una silueta: lo levanta la analítica AcuSense **clasificando** —y clasificar
> > pide **más píxeles por metro que detectar**, que es justo para lo que existen los escalones
> > `Reconocer` (`19–58 m`) e `Identificar` (`9–29 m`).
> >
> > 🔴 **Así que los `97–290 m` NO son el alcance útil de la analítica, y no se publican como tal.**
> > Son una **cifra de laboratorio** medida con un objetivo normalizado, no una promesa sobre a qué
> > distancia esta cámara va a decir «vehículo» en una vía real con lluvia, contraluz y una zona
> > dibujada a mano.
> >
> > **A qué distancia funciona de verdad la clasificación en ESTE montaje: `SIN VERIFICAR`.** Ninguna
> > de las tres fuentes oficiales publica ese número, y **no se estima**: se mide con la cámara
> > montada, en el sitio, mirando si el evento salta con el vehículo donde de verdad se detiene. Es
> > el `ENSAYO 2` del §6 quien lo cierra — y por eso ese ensayo no es opcional.
>
> ✅ **Y hay una noticia vial buena, que además retira un paso de este manual: ESTA CÁMARA NO TIENE
> LUZ BLANCA.** La ficha lista *Supplement Light Type: **IR***, sin fila de luz blanca ni ColorVu.
> **No hay nada que desactivar y no hay riesgo de deslumbrar de frente a un conductor** — que era
> justo lo que el §4 Paso 2 mandaba evitar. Ver el tachado allí.
>
> ⚠️ **Lo que el peso y el tamaño sí obligan a mirar, y no es cosa de este manual:** `1.385 g` y
> `30,8 cm` en lo alto de un poste **móvil** son carga de viento y palanca. **El soporte no está
> especificado en ningún documento de este proyecto: 🔴 `SIN VERIFICAR`.** Hikvision lista soportes
> opcionales (`DS-1275ZJ-S-SUS` de poste vertical, `DS-1260ZJ` / `DS-1280ZJ-S` de caja de
> conexiones) en la pág. 5 de la ficha.

### 1.1.4 Analítica — qué trae, y cuál es la que debe disparar la demanda

| qué | valor | nivel y fuente |
|---|---|---|
| **Perimeter Protection (*deep learning*)** | **Cruce de línea** (*Line Crossing*) y **Intrusión** (*Intrusion Detection*). *«Supports human and vehicle targets classification»* | 📖 **ESCRITO** — ficha, pág. 4 |
| **Eventos básicos** | Detección de movimiento **con clasificación humano/vehículo**, sabotaje de vídeo, excepción | 📖 **ESCRITO** — ficha, pág. 3 |
| **Otras analíticas del manual** | *Region Entrance*, *Region Exiting*, *Unattended Baggage*, *Object Removal*, detección de rostro | 📖 **ESCRITO** — manual, págs. 46-56 |
| **Interfaces de integración** | ONVIF (Profile S y G), ISAPI, SDK · 1 × RJ45 10/100 | 📖 **ESCRITO** — ficha, pág. 3 |

### 🎯 CUÁL DE LAS ANALÍTICAS ES LA NUESTRA — «hay alguien ESPERANDO», no «algo PASÓ por aquí»

**Ésta es la pregunta que decide si el semáforo le da verde a un carril vacío**, y se contesta con
las definiciones literales del manual oficial, no con criterio propio. **Lo que este equipo necesita
saber es que hay un vehículo DETENIDO esperando paso** — porque las tres entradas de cámara acaban
en `demanda_solicitar()` (✅ **MEDIDO** en el fuente: `camaras_actualizar()` de `botones.cpp` la llama en cada flanco): *piden paso*.

| analítica | qué dice el manual oficial que detecta | ¿es «esperando»? |
|---|---|---|
| **Detección de Intrusión** *(Intrusion Detection)* | *«objects **entering and loitering** in a predefined virtual region»*, y su regla `Threshold` es *«the threshold for the time of the object **loitering** in the region. If the time that one object **stays** exceeds the threshold, the alarm is triggered»* | ✅ **SÍ. Es la única que mide PERMANENCIA** |
| Cruce de Línea *(Line Crossing)* | *«objects **crossing** a predefined virtual line»* | 🛑 **NO.** Es un evento de paso: dispara con el vehículo **que ya se fue** |
| Región de Entrada *(Region Entrance)* | *«objects **entering** a predefined virtual region from the outside place»* | ⚠️ **A medias.** Dispara **al entrar**, sin exigir que se quede |
| Región de Salida *(Region Exiting)* | *«objects **exiting** from a predefined virtual region»* | 🛑 **NO.** Es lo contrario de lo que se busca |
| Detección de Movimiento *(básica)* | movimiento en una zona, con clasificación humano/vehículo | 🛑 **NO.** Un vehículo **detenido** deja de moverse y **dejaría de pedir paso** |

> ✅ **DECISIÓN, Y ESTÁ RAZONADA SOBRE LA FUENTE: `DETECCIÓN DE INTRUSIÓN`.** No es la que traía este
> manual por casualidad ni por costumbre: **es la única de las cinco cuyo parámetro de regla mide el
> TIEMPO QUE EL OBJETO SE QUEDA.** Esa palabra —*loitering*, permanecer— es exactamente *«hay un
> vehículo esperando»*, y es lo que separa una demanda real de un coche que pasó de largo.
>
> 🛑 **Y por eso la Detección de Movimiento básica queda descartada aunque tenga el clasificador de
> vehículo:** el movimiento **cesa cuando el coche frena**. Una demanda que se apaga justo cuando el
> vehículo termina de detenerse es la peor de todas — pediría paso al que pasa y no al que espera.
>
> 🔴 **Lo que queda abierto de esta elección, y no es la elección: el CLASIFICADOR.** El manual
> oficial documenta `Detection Target` (☑ Vehículo / ☐ Humano) en *Line Crossing*, *Region Entrance*
> y *Region Exiting*, **y no lo lista en `Set Intrusion Detection`** (págs. 48-49), cuyas reglas
> publicadas son sólo `Sensitivity` y `Threshold`. **Pero la ficha del modelo sí afirma que la
> Perimeter Protection —cruce de línea E intrusión— *«supports human and vehicle targets
> classification»* (pág. 4).** Las dos fuentes oficiales no dicen lo mismo.
>
> **Cómo se resuelve, y cuesta un minuto con la cámara delante:** al configurar la regla de intrusión
> se **mira si la casilla `Detection Target` está**. Si está, se marca ☑ Vehículo y se acabó. Si no
> está, **hay una decisión vial que NO se toma en este manual** y que se le lleva al responsable, con
> las dos opciones ya medidas: quedarse en intrusión sin filtro *(un peatón parado en el arcén
> pediría paso)* o irse a *Region Entrance* con filtro de vehículo *(no exige permanencia)*. **Lo que
> decide entre las dos es el `ENSAYO 2`, que ya está escrito en §6.**

---

## 2. Asignación de Pines y Distribución de Señales

~~En cada semáforo móvil (Maestro y Esclavo), los relés de las cámaras se conectan a los **dos únicos pines libres** del microcontrolador:~~

⛔ **La frase *«los dos únicos pines libres»* se retira.** Es la formulación que ya costó tres
defectos en este proyecto (`N-59`, `N-67`, `N-105`): **«pin libre» no es una observación, es una
medida contra `pines.h`.** El reparto real es el de la tabla de abajo.

```text
 ┌─────────────────────────────────────────────────────────────────────────────┐
 │    MAPA DE CONEXION FISICA DE CAMARAS  --  AL DIA EL 07/09 (D-2 / D-3)     │
 ├─────────────────────────────────────────────────────────────────────────────┤
 │                                                                             │
 │ !!! J16 p1 LLEVA 12 V CRUDOS.  SE TAPA ANTES DE CABLEAR.  NO SE CONECTA. !!!│
 │                                                                             │
 │ • ~~CAMARA DE DEMANDA, HOY (D-2 / D-3): Bornes 1A/1B --> J16 p10 (PB14)~~  │
 │   ~~en un poste y J16 p12 (PB15) en el otro. UNA CAMARA POR POSTE.~~       │
 │   <-- DEROGADO 11/09 por D-25.  AHORA, EN CADA POSTE, DOS CAMARAS:          │
 │     CAMARA 1:  Bornes 1A/1B --> J16 p10 (PB14, CAM_C) y J16 p9  (3,3 V)    │
 │     CAMARA 2:  Bornes 1A/1B --> J16 p12 (PB15, CAM_D) y J16 p11 (3,3 V)    │
 │   Contacto seco CONTRA LOS 3,3 V del borne contiguo: p9 para p10,          │
 │   p11 para p12.  ACTIVO EN ALTO.  DOS HILOS POR CAMARA: nada a masa.       │
 │   - Detecta si hay vehiculos esperando paso en el carril.                   │
 │   - LAS DOS HACEN LO MISMO (CAM_J16[2], un solo bucle): piden paso.        │
 │   - NINGUNA PROTEGE LA PLUMA: escribirPines() no lee camaras (A-1.bis).    │
 │   - p12 es el borne mas cercano a los 12 V: p1 TAPADO y trabajo limpio.    │
 │                                                                             │
 │   ~~ANTES DECIA: Bornes 1A/1B --> J14: pin PB0 CONTRA LOS 3,3 V~~           │
 │   <-- TACHADO EL 07/09.  J14 NO ESTA VIGILADO por CAM_CIEGA/CAM_PEGADA.    │
 │       Una camara ahi FUNCIONA y NADIE SABE si se estropea. J14/PB0 queda   │
 │       vivo, libre, y reservado a fin de carrera de barrera.                │
 │                                                                             │
 │   La corriente vuelve por R67/R68 dentro de la placa (ver 4.bis.5).        │
 │   M3 CERRADA EN COBRE el 03-04/09: 9,93 y 9,94 kOhm a masa, los dos a 0 V. │
 │                                                                             │
 │ • ENTRADA J14 (PB0), HOY SIN CAMARA:  el firmware la sigue leyendo, y      │
 │   pide paso igual.  Pero NO la vigila CAM_CIEGA/CAM_PEGADA:  si algun dia  │
 │   se le cuelga algo, ES EL UNICO BORNE SIN AVISO DE AVERIA.                │
 │   Reservado a fin de carrera de barrera (es el unico con antirrebote RC).  │
 │   !!! 11/09 CONFLICTO ABIERTO: A-2 manda ahi el fin de carrera, y el      │
 │   !!! firmware lee PB0 como CAMARA (CAM_DEMANDA_PIN): un fin de carrera   │
 │   !!! se leeria como demandas de paso.  NO SE CABLEA sin el responsable.  │
 │                                                                             │
 │ • TALANQUERA:  J15 (p1 = 12 V, p2 = drenador de Q10 -- p2 NO ES MASA),     │
 │   a la bobina de un rele; su contacto va a OPEN de la centralita (D-25).  │
 │                                                                             │
 │ • CAMARA 2 / 4 (Umbral):  NO EXISTE EN V9.0 - PB8 es un LED, no una entrada │
 │   (OJO: la "camara 2" de D-25 es OTRA COSA: la segunda de demanda, en p12) │
 │                                                                             │
 │ • PB9 (J16 p5) y PB13 (J16 p8):  canales A y B del MANDO.  El mando FISICO │
 │   NO EXISTE (D-1: se opera solo por app), pero SU CODIGO SE CONSERVA y los │
 │   dos pines VAN CABLEADOS: con MANDO_B al aire el veto de SFTY-21 se abre. │
 │   >>> NUNCA UNA CAMARA AQUI: tres pulsos en 12 s componen una secuencia <<< │
 │   Sus 3,3 V contiguos son p4 y p7 (mismo cobre que p9/p11: 10K + 100nF).    │
 │                                                                             │
 └─────────────────────────────────────────────────────────────────────────────┘
```

### 2.1 El reparto real, pin por pin

| pin | qué es | ¿cámara? | nivel |
|---|---|---|---|
| **`PB0`** (`J14`) | `CAM_DEMANDA_PIN`, con `R64` 10 kΩ + `C25` 100 nF (antirrebote 1 ms) | ✅ **Sí** *(para el firmware)* — pero **hoy no lleva cámara**, y 🔴 **11/09: CONFLICTO ABIERTO** con `A-2`, que manda aquí el fin de carrera: el firmware lo leería como demanda | ✅ **MEDIDO** (`pines.h:43-46`, cita fechada y vigente; su `pinMode(CAM_DEMANDA_PIN, INPUT)` en `botones_setup()`; leída en `modo_inteligente.cpp` y en `Esclavo/src/main.cpp:350`) |
| **`PB8`** | `LED_TESTIGO` → `R16` 1 kΩ → LED `D5` | ❌ **No es entrada de nada** | ✅ **MEDIDO** (símbolo `LED_TESTIGO`, `pines.h:63` — cita vigente; `modo_inteligente.cpp` lo deja en alta impedancia) |
| **`PB9`** (`J16` p5) | `BOTON1` = **`MANDO_A`** del mando de relés | 🛑 **NUNCA.** `A·A·A` en 12 s = Modo Automático | ✅ **MEDIDO** (símbolos `BOTON1`, `MANDO_A`, y `mando_registrarPulso(MANDO_A)` en `botones_actualizar()`) |
| **`PB13`** (`J16` p8) | `BOTON2` = **`MANDO_B`** | 🛑 **NUNCA.** `B·B·B` = Ámbar **y arma `ambarLocal`**, que veta las órdenes de radio | ✅ **MEDIDO** (símbolos `BOTON2`, `MANDO_B`, `mando_registrarPulso(MANDO_B)` y `mando_ambarLocal()`) |
| **`PB14`** (`J16` **p10**) | **`CAM_C_PIN` — entrada de cámara de DEMANDA**, con **`R67` 10 kΩ a masa CONFIRMADA en cobre: 9,93 kΩ**. 🎯 **11/09 (`D-25`): lleva la CÁMARA 1 de cada poste** | ✅ **Sí — firmware Y cobre** | ✅ **MEDIDO EN EL FUENTE** (símbolos `CAM_C_PIN`, su `pinMode(CAM_C_PIN, INPUT)` y `camaras_actualizar()`) **y MEDIDO EN BANCO el 04/09** (`M3`, paso 20: 9,93 kΩ a masa, 0 V con energía) |
| **`PB15`** (`J16` **p12**) | **`CAM_D_PIN` — entrada de cámara de DEMANDA**, con **`R68` 10 kΩ a masa CONFIRMADA en cobre: 9,94 kΩ**. 🎯 **11/09 (`D-25`): lleva la CÁMARA 2 de cada poste** —hace exactamente lo mismo que `p10`, mismo bucle `CAM_J16[2]`— | ✅ **Sí — firmware Y cobre** | ✅ **MEDIDO EN EL FUENTE** (símbolos `CAM_D_PIN` y su `pinMode(CAM_D_PIN, INPUT)`) **y MEDIDO EN BANCO el 04/09** (`M3`, paso 20: 9,94 kΩ a masa, 0 V con energía) |

> ### ✅ QUÉ CAMBIÓ EL 31/08 EN ESTAS DOS FILAS — Y QUÉ NO
>
> **`PB14` y `PB15` YA NO SON PULSADORES.** Este manual decía que eran `botonAceptar()` y
> `botonCancelar()` y que *«ningún firmware los lee todavía como cámara»*. **Las dos cosas son
> falsas hoy:**
>
> ```
>   Maestro/src/botones.cpp:305-306   bool botonAceptar()  { return false; }
>                                     bool botonCancelar(){ return false; }
>   Maestro/src/botones.cpp  botones_setup(): pinMode(CAM_C_PIN, INPUT);
>                                     pinMode(CAM_D_PIN, INPUT);
>   Maestro/src/botones.cpp  camaras_actualizar(): flanco de subida -> demanda_solicitar()
>   Esclavo/src/botones.cpp:316-317, :194-195, :164-172   identico
> ```
>
> **Son cámaras de DEMANDA**, exactamente como la de `J14`: piden paso. **No son cámaras de umbral**
> y no miden el despeje — el despeje sigue siendo por tiempo (`cfgDespejeSeg`).

> ### 🟢 QUÉ PASA CUANDO EL CONTACTO CIERRA — el camino completo, censado el 05/09
>
> **Las tres entradas acaban en la MISMA puerta**, y conviene saberlo antes de parametrizar el relé
> de la AcuSense, porque la ventana de silencio la comparten:
>
> ```
>    J16 p10 (PB14) --+
>                     +--> camaras_actualizar()  --FLANCO DE SUBIDA-->  demanda_solicitar()
>    J16 p12 (PB15) --+     camaras_actualizar()                     demanda_solicitar()
>                                                                              |
>                                                                     demanda_hayLocal()
>                                                                              |
>    J14 (PB0) -----------------------------------------------> camara_leerPin(CAM_DEMANDA_PIN)
>                                                                              |
>                                                        modo_inteligente.cpp:97:
>                                              camara_leerPin(CAM_DEMANDA_PIN) || demanda_hayLocal()
> ```
>
> ✏️ **11/09 — el `OR` de abajo del dibujo ya no tiene dos términos, tiene TRES, y la cita `:97` está
> caducada.** Medido por símbolo: `demandaLocalS1 = camara_leerPin(CAM_DEMANDA_PIN) ||
> camara_presenciaJ16() || demanda_hayLocal()` en `modoInteligente_loop()`
> (`grep -n "camara_presenciaJ16()" 01_Firmware/Maestro/src/modo_inteligente.cpp`). El término nuevo,
> `camara_presenciaJ16()`, **sólo existe en el Maestro** y es el que deja al Modo Inteligente
> **sostener** una fase mientras las cámaras de `J16` siguen viendo algo. **Nada de esto actúa fuera
> del Modo Inteligente**: en Automático y en Manual las cámaras no cambian ninguna luz.
>
> | propiedad | valor | dónde |
> |---|---|---|
> | **Ventana de silencio entre demandas** | **3 000 ms**, y es **la misma** para la cámara y para el botón de la app | `SILENCIO_MS`, `demanda.cpp:8` |
> | **Las de `J16` se toman POR FLANCO**, no por nivel | el relé cierra ~1 s; leer el nivel repetiría la petición cada vuelta del `loop` | símbolo `camaras_actualizar()` |
> | **La de `J14` se lee POR NIVEL** | tiene el antirrebote `RC` de la placa | símbolo `camara_leerPin(CAM_DEMANDA_PIN)` en `modo_inteligente.cpp` |
> | **Un contacto YA CERRADO al encender NO es una detección** | se siembra el nivel real en el arranque: no vuelve a pedir hasta que se **ABRA** y se cierre otra vez | símbolo `camaras_sembrar()` de `botones.cpp` |
>
> ⚠️ **Consecuencia de parametrización, y es la que decide NO/NC:** con la salida en **NC** el
> contacto está cerrado en reposo, así que **el pin nace ALTO**, la siembra lo marca como «ya
> disparado» y **no habrá flanco hasta que pase un coche y el relé ABRA y vuelva a cerrar**. Con
> **NO** el reposo es abierto y cada detección da su flanco. **La configuración prevista es NO**, y
> el criterio negativo del ensayo —*en reposo no debe pedir paso*— es lo que lo comprueba.
>
> ~~🔵 **Y un sitio donde las tres NO convergen, que NO es un defecto:** el bloque del LED testigo de `modo_inteligente.cpp`
> calcula el contador de presencia mirando **sólo** `PB0` y la demanda remota. **Medido: ese número
> sólo alimenta `lcd_dibujarInteligente()` (`:138`) — es el contador de la PANTALLA, y la pantalla se
> retira.** No decide ninguna luz. Se escribe con la medida al lado para que nadie vaya a
> «arreglarlo».~~ → ✏️ **11/09 — CADUCADO: ya convergen.** Hoy `presenciaActual` se calcula con
> `demandaLocalS1` —el mismo `OR` de tres términos que decide la fase, `J16` incluida— y la demanda
> remota: `grep -n "int presenciaActual" 01_Firmware/Maestro/src/modo_inteligente.cpp`. Lo que se
> dibuja es lo que decide. Sigue siendo sólo el contador de la pantalla.

> ## 🟢 04/09 — `M3` CERRADA EN BANCO: **`PB14`/`PB15` YA SE CABLEAN**
>
> ~~**AUN ASÍ, `PB14`/`PB15` NO SE CABLEAN TODAVÍA. Queda UN bloqueo, y es de cobre: falta la medida
> `M3` con óhmetro. `R67` y `R68` de 10 kΩ a masa sólo lo dice el netlist, y nadie lo ha medido en
> cobre.**~~ ⛔ **ANULADO: se midió el 04/09 (paso 20 de la sesión de banco) y el netlist tenía
> razón.** El texto viejo se conserva tachado porque **describía bien el riesgo** —un `INPUT` pelado
> sin resistencia real queda flotando y da demandas fantasma en un equipo que gobierna un cruce—; lo
> que ha cambiado es que **ese riesgo está descartado por medida, no por confianza**.
>
> ```text
>   M3, paso 20, tarjeta energizada y J16 vacio:
>     J16 p10 (PB14) ->  9,93 kOhm a masa   ->  0 V
>     J16 p12 (PB15) ->  9,94 kOhm a masa   ->  0 V
>   Los cuatro pines de J16 son identicos: 10K a masa + 100 nF,
>   y los cuatro tienen 3,3 V en la posicion contigua (J16.4 / .7 / .9 / .11).
> ```
>
> **Los tres resultados que `M3` podía dar están resueltos en el bueno:** hay resistencia, es de
> 10 kΩ, y es un **pull-DOWN**. Por tanto **el contacto seco cierra contra los 3,3 V del borne
> contiguo** —`p9` para `p10`, `p11` para `p12`—: **ACTIVO EN ALTO**, que es como el firmware ya lee.
>
> ✅ **Y el paso 21 cerró la otra mitad, que es la que se siente en la calle: NO hay falsa
> activación.** En reposo, **con el cable puesto y sin él**, el equipo **no pide paso por sí solo**.
> Era exactamente el criterio negativo del `ENSAYO 3`, ejercido sobre `J16`.
>
> 🛑 **Lo que NO se ha levantado con `M3`: `J16` p1 sigue llevando 12 V crudos, y taparlo sigue
> siendo obligatorio en cada equipo antes de cablear.** Ver el bloque de la cabecera. La separación
> real sobre cobre contra la red de 12 V es de **4,269 mm** en `p10` y de sólo **1,359 mm** en `p12`
> (**MEDIDO**, `03_Hardware_Tarjeta/MAPEO_TARJETA_KICAD.md:576-588`): ~~**si una de las dos cámaras es
> más crítica, va en `p10`.**~~ → ✏️ **11/09 (`D-25`): van las DOS, una en `p10` y otra en `p12`, y
> ninguna es «más crítica»: hacen lo mismo.** Lo que queda de la medida es un aviso de montaje: **`p12`
> es el borne con menos margen a los 12 V**, así que se cablea con `p1` ya tapado y sin restos de
> cobre ni humedad en el conector.
>
> ### 🪜 Y el orden sigue siendo ASIMÉTRICO — cargar firmware ANTES de tocar un hilo
>
> **Con el firmware nuevo dentro es seguro**: un pin en `INPUT` pelado no ejecuta nada.
> **Con el firmware viejo dentro NO lo es**: `PB14` sigue siendo `botonAceptar()` leído activo en
> BAJO, y cualquier hilo enchufado en `J16` p10 **pulsa *Aceptar* en un equipo que está en la
> calle**. 🔴 **11/09 (`D-25`): y ahora también `p12` lleva cámara — con el firmware viejo `PB15`
> es `botonCancelar()`**, así que lo enchufado en `p12` puede pulsar *Cancelar* (medido en la V8.4
> de campo: `git show e303485:01_Firmware/Maestro/src/botones.cpp`, `botonAceptar()` y
> `botonCancelar()` leen `BOTON3`/`BOTON4`). **Exija la carga verificada en la tarjeta antes de que
> nadie enchufe nada** — un commit no protege de un destornillador.

---

## 3. Filosofía de Movilidad: Configuración ÚNICA en Taller

> ### 🛑 REGLA DE ORO PARA EQUIPOS MÓVILES:
> **La geometría no discrimina; discrimina el clasificador AcuSense.**  
> Al configurar una **zona de intrusión amplia (~90% de la pantalla)** con el filtro `☑ Vehículo`, el semáforo puede trasladarse de kilómetro en la vía **sin necesidad de conectar una laptop, ni reencuadrar el zoom, ni redibujar máscaras de píxeles**.

---

## 4. Procedimiento de Parametrización en Taller (Paso a Paso)

Se realiza **una sola vez en taller** antes de enviar las cámaras a campo:

```
[ PASO 1: Red y Acceso ] ──► [ PASO 2: Óptica y Luz ] ──► [ PASO 3: Analítica AcuSense ] ──► [ PASO 4: Salida Relé ]
```

> ## 📖 DE DÓNDE SALE CADA PANTALLA DE ESTE APARTADO (05/09/2026)
>
> **Todo el paso a paso de abajo está contrastado, pantalla por pantalla, contra el manual de usuario
> oficial de esta cámara.** No está escrito «de memoria de cómo son las Hikvision»:
>
> | | |
> |---|---|
> | **Documento** | **`UD28967B-C_Network-Camera_User-Manual_5.7.20_20240131.PDF`** — *Network Camera User Manual*, 110 páginas |
> | **De dónde** | `hikvision.com`, pestaña **Resources → Technical documents → User Manual** de la página de producto de la `DS-2CD2683G2-IZS`. Descargado el **05/09/2026** |
> | **Complementos** | Ficha `DS-2CD2683G2-IZS_Datasheet_V5.5.113_20230303.pdf` · Guía rápida `UD40284B_..._20241115.pdf` |
>
> **Cada paso lleva su página.** Y lo que **no** se ha podido confirmar contra ese documento lleva
> escrito **`SIN VERIFICAR`** con esas palabras — **no se ha rellenado con lo que parecía razonable**,
> porque quien lea esto va a estar delante de la cámara dando por hecho que alguien lo comprobó.
>
> ⚠️ **Los nombres de menú van en inglés y en español.** El manual oficial está en inglés; la interfaz
> web de la cámara tiene español entre sus 33 idiomas (ficha, pág. 4), **pero la traducción exacta de
> cada rótulo es 🔴 `SIN VERIFICAR`**. Si un rótulo en español no aparece tal cual, **manda el
> inglés**, que es el que sale del documento.

### Paso 0: ANTES DE NADA — comprobar que la analítica puede accionar el relé

> 🛑 **Este paso es nuevo (05/09) y va el primero porque puede ahorrar todo el resto.** Ver el bloque
> 🔴 de la cabecera: **que la regla de intrusión se pueda enlazar a la salida de alarma es
> `SIN VERIFICAR`**, y si no se puede, el diseño de `J16` no sirve y hay que parar y avisar.

1. Alimentar la cámara y entrar por navegador (Paso 1).
2. Ir a **`Configuration → Event → Smart Event → Intrusion Detection`**
   *(Configuración → Evento → Evento Inteligente → Detección de Intrusión)*.
   **Manual, pág. 48.** En algunos modelos la ruta es **`VCA → Smart Event → Intrusion Detection`** —
   *el propio manual da las dos*, así que si una no existe, se prueba la otra.
3. Marcar **`Enable`** y bajar hasta **`Linkage Method`** *(Método de Vinculación)*.
4. **MIRAR SI EXISTE LA CASILLA `Trigger Alarm Output`** *(Disparar Salida de Alarma)*.
   * **Si está** → el diseño vale entero. Seguir con el Paso 1.
   * **Si NO está** → 🛑 **PARAR Y AVISAR.** No se cablea la cámara a `J16`: no serviría.
     **Lo que hay que hacer en ese caso está escrito abajo, en el bloque 🔻** — no se improvisa
     delante del poste, y **la cámara no se queda inservible**.

> ## 🔻 07/09 — LO QUE EL PAPEL SÍ CIERRA, Y NO ES LO QUE ESTE MANUAL VENÍA DICIENDO
>
> **Se volvió a abrir la ficha del modelo comprado, y hay una fila que este documento citaba de
> pasada y que es la respuesta específica a la pregunta del Paso 0.** La nota del manual de usuario
> —*«only supported by certain models»*— es una condición **sobre el modelo**; **la ficha ES el
> documento del modelo**, y contesta enumerando:
>
> ```text
>   DS-2CD2683G2-IZS_Datasheet_V5.5.113_20230303.pdf, pag. 4, seccion "General":
>
>     Linkage Method
>       Upload to FTP/memory card/NAS, notify surveillance center,
>       trigger recording, trigger capture, send email
> ```
>
> **Son cinco, es una lista cerrada, y `trigger alarm output` NO está en ella** — mientras que
> `trigger recording` y `trigger capture`, que en el manual genérico son enlaces como cualquier
> otro, **sí están**. O sea que la ficha **no calla** sobre los métodos de vinculación de este
> modelo: los enumera, y deja fuera justo el que necesitamos.
>
> ### ⛔ Y SE CAE EL ARGUMENTO CON EL QUE ESTE MANUAL LO DABA POR PROBABLE
>
> Aquí decía: ~~*«Que el hardware traiga `1 output` hace probable que sí»*~~. 🔴 **Esa inferencia no
> se sostiene, y el propio manual de usuario la desmonta dos páginas después de la nota:**
>
> | quién puede cerrar esa salida **sin analítica ninguna** | dónde |
> |---|---|
> | **`Manual Alarm`** — *«You can trigger an alarm output manually… Click **Manual Alarm** to enable manual alarm output»*: un botón en el navegador | **PDF pág. 80 · impresa 68** |
> | **`Automatic Alarm`** — *«the device triggers an alarm output automatically **in the set arming schedule**»*: cierra por HORARIO, no por detección | **PDF pág. 80 · impresa 68** |
>
> **Tener `1 output` queda enteramente explicado por esas dos.** Que el borne exista **no dice nada**
> sobre si la regla de intrusión puede llegar hasta él, que es lo único que este diseño necesita.
>
> ### 📌 El estado real, en una línea
>
> **Ninguna fuente oficial dice que la `DS-2CD2683G2-IZS` pueda enlazar analítica → salida de alarma.
> Dos fuentes oficiales apuntan a que NO** (la ficha, omitiéndolo de una lista que enumera; el
> manual, con la nota de modelos). **Sigue siendo 🔴 `SIN VERIFICAR` —el papel no lo cierra en
> positivo— pero ya no es «probablemente sí»: es «probablemente no».** Y el Paso 0 pasa de trámite a
> **la comprobación más cara del proyecto**: con las cámaras ya compradas, decide si el camino de
> `J16` existe.
>
> ### 🔻 SI LA CASILLA NO ESTÁ — lo que sobrevive y lo que no
>
> **No se tira la cámara, y esto hay que decirlo para que nadie lo lea como «las cámaras no valen».**
>
> | | estado si `Trigger Alarm Output` no existe |
> |---|---|
> | **El bit al controlador por `J16`** | 🛑 **MUERE.** Es el único camino que el firmware sabe leer hoy |
> | **La grabación por analítica en la microSD** | ✅ **VIVE, y no depende de la casilla.** `Trigger Recording` **sí está** en la lista de la ficha, y el `Record Schedule` admite el tipo **`Event`**: *«The video is recorded when configured event is detected»* (**PDF pág. 48 · impresa 36**). La cámara sigue dando prueba de lo que pasó |
> | **La vía de `D-14`** *(el controlador cierra un contacto y la cámara graba)* | 🔴 **NO ES UN RELEVO DISPONIBLE HOY.** Ver §4.bis.6: **el lado del controlador no está construido**, medido el 07/09 |
>
> 🛑 **Y lo que este manual NO decide: por dónde entraría entonces la demanda.** Relé de NVR, evento
> de red, u otro modelo de cámara son **otro diseño**, con red donde `D-12` dice que no hay red. **Se
> para, se anota el hallazgo con la pantalla delante, y se le lleva al responsable.**

> ⚠️ **Y una condición previa que el manual avisa y es fácil de pasar por alto:** *«For certain device
> models, you need to **enable the smart event function on VCA Resource page** first»* (**pág. 48**).
> Si *Intrusion Detection* no aparece o aparece en gris, ir a **`VCA → VCA Resource`** —o
> **`Configuration → System → System Settings → VCA Resource`**— **habilitar la función y guardar**
> (**pág. 85**). El manual advierte además que *«certain VCA functions are mutually exclusive»*:
> **activar una analítica puede apagar otra**, así que se habilita **sólo la de intrusión**.

> ### 🟢 05/09 — EL CAMINO ESTÁ DOCUMENTADO DE PUNTA A PUNTA EN EL MANUAL OFICIAL (pero sigue `SIN VERIFICAR`)
>
> **Leído del PDF que está en `04_Manuales/`, no de la web.** Los dos eslabones existen y **encajan**:
>
> | eslabón | qué dice, literal | dónde |
> |---|---|---|
> | La regla de intrusión **enlaza a los métodos de vinculación** | *«For the linkage method settings, refer to **Linkage Method Settings**»* — paso 7 de `Set Intrusion Detection` | **PDF pág. 61 · impresa 49** |
> | Los métodos de vinculación **incluyen disparar la salida de alarma** | *«**Trigger Alarm Output** — If the device has been connected to an alarm output device, and the alarm output No. has been configured, the device sends alarm information to the connected alarm output device when an alarm is triggered»* | **PDF pág. 79 · impresa 67** |
>
> **O sea: `Detección de Intrusión` → `Linkage Method` → `Trigger Alarm Output` es un camino que el
> fabricante documenta**, y el hardware existe en este modelo *(ficha pág. 3: `1 input, 1 output`)*.
> ~~Eso es **bastante más de lo que había el 04/09**, cuando lo único citable era la fila *Linkage
> Method* de la ficha, que no lo menciona.~~
>
> ⛔ **CORREGIDO EL 07/09, y la corrección invierte el tono de este bloque.** Lo de arriba es cierto
> **y es del manual GENÉRICO de la familia** — describe lo que una cámara Hikvision puede hacer, no
> lo que hace ésta. **La fila de la ficha que aquí se despachaba como *«lo único citable»* es
> precisamente la fuente específica del modelo comprado, y enumera cinco enlaces sin incluir la
> salida de alarma.** Ver el bloque 🔻 de arriba: **el peso de la prueba estaba puesto del revés.**
>
> > 🔴 **Y AUN ASÍ NO SE MARCA VERIFICADO, porque la frase que sobra sigue ahí.** Debajo de
> > `Trigger Alarm Output` el manual escribe: **«*This function is only supported by certain
> > models*»** (misma página). **Es una condición sobre el modelo, y no la resuelve ningún documento
> > que tengamos**: ni la ficha ni el manual dicen si la `DS-2CD2683G2-IZS` es uno de esos modelos.
> >
> > ~~Que el hardware traiga `1 output` **hace probable** que sí~~ ⛔ **RETIRADO EL 07/09: la
> > inferencia es falsa** — el mismo manual documenta `Manual Alarm` y `Automatic Alarm`
> > (**PDF pág. 80 · impresa 68**), que cierran esa salida **sin analítica ninguna**, así que el
> > borne se explica entero sin ella. Ver el bloque 🔻. **Lo que queda en pie es lo que sigue:**
> > *«probable»* no es una de las tres respuestas que este proyecto acepta. **Lo cierra mirar la
> > pantalla**, que es lo que este Paso 0 manda hacer, y cuesta diez minutos con la cámara delante.

> ### 🔴 Y SIGUE `SIN VERIFICAR` EL CLASIFICADOR SOBRE ESTA ANALÍTICA — medido en el manual el 05/09
>
> Al leer las dos secciones seguidas aparece una asimetría que **confirma** el aviso que ya llevaba
> este manual y **no** lo relaja:
>
> | analítica | reglas que el manual le documenta | ¿incluye `Detection Target` (persona/vehículo)? |
> |---|---|---|
> | `Set Intrusion Detection` *(**PDF 60-61 · impresa 48-49**)* | `Sensitivity`, `Threshold`, y filtro de tamaño *(`Set Size Filter`, paso 4)* | ❌ **NO aparece** |
> | `Set Line Crossing Detection` *(**PDF 62 · impresa 50**)* | `Direction`, `Sensitivity`, **`Detection Target`**, `Target Validity` | ✅ **Sí, literal**: *«Human and vehicle are available»* |
>
> **La casilla que filtra «sólo vehículo» está documentada en Cruce de Línea y NO en Intrusión.** Es
> el §4 Paso 3 de este manual, y por eso ahí sigue marcado `SIN VERIFICAR`: **puede existir en
> pantalla y no estar en el papel** —el manual es de la `5.7.20` y el firmware va por la `5.7.23`—,
> pero **hasta verlo no se escribe como hecho**. 🟢 **Lo que sí está en la ficha** (pág. 4) es que la
> familia soporta *«Line crossing detection, intrusion detection — Supports human and vehicle targets
> classification»*: la capacidad existe; **dónde está la casilla, es lo que falta por ver.**
>
> > **Qué hacer si en pantalla NO hay clasificador en Intrusión, para no improvisar delante del
> > poste:** es una decisión de diseño, no un ajuste — y está planteada en el **§1.1.4** de este
> > manual, que compara las cinco analíticas. **Sin filtro de vehículo, una persona o una rama
> > levantan el bit**, y eso cambia lo que ese bit significa. **No lo decide el técnico en taller.**

### Paso 1: Acceso Inicial y Activación

**Manual, capítulo 1 «Device Activation and Accessing», págs. 1-2.**

> 🔑 **Si todavía no se ha conseguido entrar en la cámara, este paso NO es donde empezar:** está
> desarrollado con sus atascos —lista vacía, bloqueo por intentos, `http`/`https`— en el bloque
> **🔑 EMPIEZA AQUÍ: ENTRAR EN LA CÁMARA** de la cabecera. Aquí queda el procedimiento en limpio.

1. Conectar la cámara por **cable Ethernet** (1 × RJ45 10/100) a un portátil **en la misma subred**.
2. Ejecutar **SADP** y **buscar los dispositivos en línea**. 🟢 **No hay que descargarla: está en el
   repositorio, en `04_Manuales/SADP＿EN/SADP.exe`** *(ojo al carácter `＿`, que no es un guion bajo
   normal — ver la cabecera)*. 🟡 SADP está **descontinuada**; su relevo es **HiTools Delivery**.
3. Seleccionar la cámara en la lista e **introducir la contraseña de administrador**, dos veces.
   > 🛑 **Requisito literal del manual:** *«a minimum of 8 characters, including upper case letters,
   > lower case letters, numbers, and special characters»*. **Una cámara sin activar no se configura**
   > — es el primer paso obligatorio, no una recomendación de seguridad.
4. Pulsar **`Activate`**. El estado pasa a **`Active`**.
5. **Fijar la IP de taller:** seleccionar el equipo, cambiar la IP a la subred del portátil *(o marcar
   `Enable DHCP`)*, escribir la contraseña de administrador y pulsar **`Modify`**.
   * Sugerencia de reparto, **que es una convención de este proyecto y no del fabricante**:
     ~~`192.168.1.61` la cámara del Maestro, `192.168.1.62` la del Esclavo.~~ → **11/09 (`D-25`),
     cuatro cámaras:** `192.168.1.61` Maestro·cámara 1 (`p10`) · `192.168.1.63` Maestro·cámara 2
     (`p12`) · `192.168.1.62` Esclavo·cámara 1 (`p10`) · `192.168.1.64` Esclavo·cámara 2 (`p12`).
     ⚠️ **`192.168.1.64` es también la IP de FÁBRICA** (bloque 🔑 de la cabecera): una cámara sin
     configurar contesta en esa misma dirección, así que **se configuran de una en una** y se anota
     en la ficha qué IP quedó en cada una.

> 🔵 **Alternativas documentadas, por si no hay SADP a mano:** activación **por navegador web**
> (pág. 3) o con **iVMS-4200** (pág. 2). Las tres valen y están en el mismo capítulo.
>
> 🟢 **La IP es SÓLO de taller.** Una vez configurada, **la cámara habla con el semáforo por el
> contacto seco, no por red**: en el poste no hay switch, ni router, ni cable de datos. La red se usa
> el día que se parametriza y nunca más.

### Paso 2: Ajuste Óptico y Nocturno

#### 2.a — Zoom y enfoque

~~1. Ir a **Configuración > Imagen > Pantalla**: **Zoom:** Ajustar en **Gran Angular Máximo (2.7 mm)** para obtener el campo de visión máximo ($102.4^\circ$). **Enfoque:** Presionar **One-Touch Focus** (Autofoco).~~

⛔ **RETIRADO EL 05/09, y por tres motivos distintos, cada uno con su fuente:**

| lo que decía | qué pasa de verdad |
|---|---|
| *«Configuración > Imagen > Pantalla»* para el zoom | **Ahí no está.** `Configuration → Image → Display Settings` (**pág. 24**) es brillo, contraste y WDR. **El zoom y el foco se mueven desde la vista en vivo**, en *Lens Parameters Adjustment* (**pág. 14**) |
| *«2.7 mm»* y *«102.4°»* | **No son de esta cámara.** La `DS-2CD2683G2-IZS` es **`2,8 – 12 mm`** con **FOV horizontal `108°` a `30°`** *(ficha, pág. 2)*. Eran las cifras del modelo de referencia que se retiró de la cabecera |
| *«One-Touch Focus»* | 🔴 **Ese rótulo NO aparece en el manual.** Lo que sí está documentado es **`Auxiliary Focus`** *(Enfoque Auxiliar)* y **`Lens Initialization`** *(Inicialización de Lente)*, **pág. 13** |

**El procedimiento correcto:**

1. Desde la **vista en vivo**, abrir la **página de ajuste rápido** y usar **`Lens Parameters
   Adjustment`** *(Ajuste de Parámetros de Lente)* — **manual, pág. 14**:
   * **`Zoom`:** llevarlo al **gran angular máximo (`2,8 mm`)**, que da el campo de visión más ancho:
     **`108°` horizontal** *(ficha, pág. 2)*. Es lo que permite mover el semáforo de sitio sin
     reencuadrar — ver §3.
   * **`Focus`:** ajustar hasta ver nítida la zona donde se detendrán los vehículos.
2. Si no enfoca bien, usar **`Auxiliary Focus`** *(pág. 13)*. Y si sigue sin enfocar, el manual manda
   este orden exacto: **`Lens Initialization` primero, y después `Auxiliary Focus` otra vez**.
3. **No volver a mover el zoom.** *(Precinto: es una práctica de este proyecto, no del fabricante.)*
4. 🔵 **Opcional, y lo documenta el manual:** `Configuration → PTZ` → ☑ **`Enable PTZ Lock`**
   (**pág. 14**) **bloquea zoom y foco**, para que una pulsación accidental no desencuadre la vía.

> 🟡 **El ENCUADRE va sobrado** *(el escalón `Detectar` del DORI alcanza `97 m` a `2,8 mm` — ficha,
> pág. 2)*, **pero eso no dice a qué distancia la analítica CLASIFICA un vehículo.** ~~«para saber si
> *hay* un vehículo basta el escalón **Detectar**»~~ ⛔ **corregido el 05/09: `Detectar` es *«hay un
> objeto»*, no *«es un vehículo»*.** El porqué completo y qué se hace en su lugar, en **§1.1.3**.
> **Lo que manda para enfocar es dónde se DETIENEN los vehículos, no los 97 m.**

#### 2.b — Luz nocturna

~~2. Ir a **Configuración > Imagen > Ajustes de Luz Suplementaria**: **Modo de Luz:** Seleccionar **Solo IR**. **Luz Blanca:** **DESACTIVADA** (evita deslumbrar de frente a los conductores en carretera).~~

⛔ **RETIRADO EL 05/09 — la ruta era otra, y la mitad del paso no existe en esta cámara:**

| lo que decía | qué pasa de verdad |
|---|---|
| *«Configuración > Imagen > Ajustes de Luz Suplementaria»* | La ruta documentada es **`Configuration → System → System Settings → External Device`**, apartado *Supplement Light Settings* — **manual, págs. 77-78** |
| *«**Luz Blanca: DESACTIVADA**»* | ✅ **NO HAY LUZ BLANCA QUE DESACTIVAR.** La ficha lista *Supplement Light Type: **IR***, sin fila de luz blanca ni ColorVu *(pág. 2)*. El manual describe los modos `IR / White Light / Mix / Off` **como opciones genéricas de la gama** *(pág. 77)*, y avisa: *«**When the device supports supplement light**, you can select supplement light mode»* |

**El procedimiento correcto:**

1. Ir a **`Configuration → System → System Settings → External Device`** *(pág. 78)*.
2. **`Supplement Light Mode`** *(Modo de Luz Suplementaria)*: dejarlo en **`IR Mode`**.
   * 🔵 **Si el desplegable no aparece, no es un fallo:** esta cámara **sólo tiene IR** y puede no
     ofrecer la elección. **IR `850 nm`, alcance hasta `60 m`** *(ficha, pág. 2)*.
3. **`Smart Supplement Light`**: **dejarlo activado**. El manual lo define como *«avoids over exposure
   when the supplement light is on»* *(pág. 77)* — evita que un vehículo cercano salga quemado de
   noche y deje de clasificarse.
4. **`Brightness Adjustment Mode`**: **`Auto`** *(pág. 78)*.

> ✅ **Y ésta es una buena noticia vial que conviene dejar escrita:** al no haber luz blanca,
> **esta cámara NO PUEDE deslumbrar de frente a un conductor**. El riesgo que el texto viejo trataba
> de evitar **no existe en este modelo** — no por haberlo configurado bien, sino porque el hardware
> no lo lleva.

### Paso 3: Configuración de la Analítica Inteligente

> # 🟢 07/09 — RESUELTO: **LA ZONA ES EL BARRIDO DE LA PLUMA** (`D-13`)
>
> ~~ANTES DE DIBUJAR LA ZONA: pregunte qué tiene que mirar esta cámara. Hay dos respuestas vivas a
> la vez y este manual NO elige.~~ — **La pregunta no iba: `D-13` ya la había cerrado el 05/09, y
> las dos «respuestas vivas» no competían.** Medido el 07/09:
>
> ```
> $ grep -rn "demanda_hayLocal" Maestro/src Maestro/include   (lectores reales)
>     modo_inteligente.cpp:217        <-- el UNICO
>     modo_automatico.cpp   -> cero
>     coordinador.cpp       -> cero
> ```
>
> **En Automático y en Manual las cámaras no tocan el ciclo**, que es lo que dice `D-13`. La demanda
> vive **sólo dentro del Modo Inteligente**, y ahí `D-19` la acota: la cámara **SOSTIENE** un verde
> —hasta el doble del tiempo configurado, saturado al máximo vial— y **nunca lo adelanta ni lo
> acorta**. Con la cámara muerta, el ciclo vuelve a los tiempos configurados.
>
> **Así que se dibuja la zona de `D-13`: el BARRIDO DE LA PLUMA.** Las dos cámaras igual — ✏️ **11/09
> (`D-25`): las CUATRO igual, las dos de cada poste con la misma zona y la misma regla.** Y que la zona
> sea el barrido de la pluma **no hace que la cámara la proteja**: `escribirPines()` no lee ninguna
> cámara, y la pluma baja aunque haya un coche debajo (el veto es `A-1.bis`, sin construir).
>
> Lo que sigue se conserva porque la medida del fuente es correcta y explica el camino del pin:
>
> | | qué dice | dónde vive |
> |---|---|---|
> | **(A) — lo que el FIRMWARE ejerce hoy** | la cámara mira **la vía de aproximación** y **PIDE PASO**: el pin entra por `demanda_solicitar()` | ✅ **MEDIDO el 07/09** en el fuente, las dos puntas — ver el `grep` de abajo |
> | **(B) — lo que `D-13` DECIDIÓ el 05/09** | las dos cámaras iguales, *Intrusion Detection* **sobre el BARRIDO DE LA PLUMA —no la zona de espera—**, y *«las cámaras no dan ni quitan verde»* | `DECISIONES.md`, fila `D-13` |
>
> ```text
>   $ grep -rn "demanda_solicitar()" 01_Firmware/Maestro/src/botones.cpp
>     507:      demanda_solicitar();          <-- aqui entra el pin de camara
>
>   $ grep -rn "camara_vetosPluma" 01_Firmware/Maestro/src 01_Firmware/Esclavo/src
>     Maestro/src/botones.cpp:254:  uint16_t camara_vetosPluma() {
>     Maestro/src/botones.cpp:434:    ... "VETO_HABRIA_ACTUADO_N:%u", camara_vetosPluma()
>     (idem Esclavo, :267 y :447)
> ```
>
> 🔴 **`camara_vetosPluma()` existe en las dos puntas y SÓLO CUENTA:** publica un contador por
> `$EVENT` y **no toca una luz**. Es la fase 1 de `D-13`, deliberadamente sin efecto vial. **O sea
> que el camino de la pluma está OBSERVADO pero no ACTUADO, mientras el de la demanda sí actúa.**
>
> ### 🔻 Qué hace usted con esto, de pie delante de la cámara
>
> **PREGUNTE ANTES DE DIBUJAR.** Es una decisión vial del responsable, no un ajuste de taller, y el
> encuadre físico de la cámara depende de ella: **la vía de aproximación y el barrido de la pluma no
> se ven desde el mismo sitio.**
>
> | si le dicen… | configure |
> |---|---|
> | **«demanda»** *(el comportamiento que el equipo tiene hoy)* | el bloque de abajo **tal cual**: zona sobre la **zona de parada del carril de aproximación** |
> | **«pluma»** *(`D-13`)* | el **mismo** bloque de abajo — misma analítica, mismos parámetros — pero con la **zona dibujada sobre el barrido de la pluma**, y sabiendo que hoy ese bit **entra igual por `demanda_solicitar()` y pedirá paso**, porque el firmware no distingue de dónde viene |
>
> 🛑 **Y ésa es la frase que hay que llevarse:** hoy **el equipo hace lo mismo con las dos zonas**.
> Lo que cambia es **qué significa** el bit, y el significado no está en la cámara: **está en el
> firmware que todavía no lo separa.** Se anota qué zona se dibujó en la ficha de instalación.

> 🔴 **TODAS LAS CÁMARAS DE ESTE EQUIPO SON DE DEMANDA. Configúrelas todas igual, con el bloque de
> abajo.** Las tres entradas que el firmware lee —`PB0` en `J14`, y `PB14`/`PB15` en `J16`— acaban
> en la misma llamada, `demanda_solicitar()`: **piden paso**. Ninguna mide el despeje del tramo.
>
> **No configure ninguna con *Detección de Cruce de Línea*.** El bloque de *«Cámaras 2 y 4
> (Umbral)»* de más abajo queda tachado: describe una función que no existe en este equipo.
>
> ✅ **Y la analítica elegida —`Detección de Intrusión`— está razonada sobre la definición literal del
> fabricante, no por costumbre: es la única cuya regla mide el TIEMPO QUE EL VEHÍCULO SE QUEDA**, que
> es lo que distingue *«hay uno esperando»* de *«pasó uno»*. **La comparación de las cinco analíticas
> está en §1.1.4** y conviene leerla antes de cambiar nada aquí.

#### Para TODAS las cámaras (Demanda de Cola - Aproximación):

**Manual, *Set Intrusion Detection*, págs. 48-49.** Los pasos van en el orden del manual.

1. Ir a **`Configuration → Event → Smart Event → Intrusion Detection`**
   *(Configuración → Evento → Evento Inteligente → Detección de Intrusión)*.
   En algunos modelos la ruta es **`VCA → Smart Event → Intrusion Detection`**: **el manual da las
   dos** y no dice cuál corresponde a ésta — se prueba la primera y si no está, la segunda.
   > ⚠️ Si la opción no aparece, **habilitarla antes en `VCA Resource`** — ver el Paso 0.
2. Marcar ☑ **`Enable`** *(Habilitar)*.
3. **Seleccionar la región** y dibujarla con **`Draw Area`**: un polígono amplio que cubra el
   **encuadre inferior y central**, donde se detienen los vehículos. *(El «90 %» es un criterio de
   este proyecto —ver §3, la regla de oro de movilidad—, no una cifra del fabricante.)*
4. **`Size Filter`** *(filtro de tamaño)* — **paso 4 del manual, y este manual no lo tenía**:
   fijar **tamaño mínimo y máximo** del objetivo. El manual: *«Only targets whose size are between
   the maximum size and the minimum size trigger the detection»*.
   > 🟢 **Es la herramienta contra los falsos disparos**, y aquí vale doble: un mínimo por encima de
   > una persona **descarta peatones por TAMAÑO**, sin depender del clasificador —que es justo lo que
   > está `SIN VERIFICAR` para esta analítica—. **Se ajusta con el `ENSAYO 2` delante.**
5. **Reglas** *(paso 5 del manual)*:
   * **`Threshold`** *(Tiempo de Permanencia)*: **`1 s`**. Definición literal: *«the threshold for the
     time of the object **loitering** in the region. If the time that one object **stays** exceeds the
     threshold, the alarm is triggered»*. **Es el parámetro que convierte «pasó» en «está
     esperando»**, y **suma al retardo total** — ver el Paso 4.
   * **`Sensitivity`** *(Sensibilidad)*: **`50`**. Definición literal: *«Sensitivity = 100 − S1/ST ×
     100»*, donde `S1` es la parte del objetivo que entra en la región y `ST` el objetivo completo.
     **A `50`, medio vehículo dentro de la zona ya dispara.**
     > 🔵 **`1 s` y `50` son los valores de partida de ESTE proyecto**, no una recomendación del
     > fabricante: 🔴 **`SIN VERIFICAR` contra una cámara real.** Se afinan con los ensayos del §6.
   * **`Detection Target`** *(Clasificación de Objetivo)*: **☑ Vehículo · ☐ Humano**.
     > 🔴 **`SIN VERIFICAR` QUE ESTA CASILLA EXISTA EN ESTA ANALÍTICA.** El manual la documenta en
     > *Line Crossing*, *Region Entrance* y *Region Exiting*, **y no la lista en `Set Intrusion
     > Detection`** — mientras la ficha afirma que la Perimeter Protection *«supports human and
     > vehicle targets classification»* *(pág. 4)*. **Si la casilla está, se marca. Si no está, se
     > anota y se avisa**, y se compensa con el `Size Filter` del punto 4 hasta que el responsable
     > decida. Ver §1.1.4.
6. **`Arming Schedule`** *(Programación Horaria)* — **paso 7 del manual, y este manual NO lo tenía**:
   > 🛑 **DEJARLA EN 24 × 7. Es obligatorio y es fácil de olvidar.** La programación horaria es *«the
   > valid time of the device tasks»* (**pág. 67**): **fuera de ella la cámara NO dispara**. Un
   > semáforo que deja de aceptar demanda de madrugada porque nadie miró esta pantalla es un fallo de
   > calle que **ningún ensayo de taller de día encontraría**. Se arrastra la barra hasta cubrir los
   > siete días completos.
7. **`Linkage Method`** *(Método de Vinculación)*: ☑ **`Trigger Alarm Output`** — se detalla en el
   **Paso 4**, porque es donde está el punto delicado.
8. Pulsar **`Save`** *(Guardar)*.

#### ~~Para Cámaras 2 y 4 (Umbral de Cruce de Tramo):~~ ⛔ NO SE CONFIGURA ASÍ NINGUNA CÁMARA
1. ~~Ir a **Configuración > Eventos > Evento Inteligente > Detección de Cruce de Línea** (*Line Crossing Detection*).~~
2. ~~Marcar ☑ **Habilitar** (*Enable*).~~
3. ~~Trazar la línea atravesando el carril de salida.~~
4. ~~**Dirección:** **Bidireccional (`A<->B`)**.~~
5. ~~**Clasificación de Objetivo:** ☑ **Vehículo** | ☐ **Humano**.~~

> **Por qué se tacha, y no se borra.** La cámara de umbral **no existe en este equipo** —no hay
> entrada física para ella y no hay comando de radio que lleve la cuenta del tramo al Maestro—.
> Configurar una cámara así y llevarla a `J16` p10 o p12 haría que **cada vehículo que SALE del
> tramo pidiera paso**, que es lo contrario de lo que se busca: esos dos pines llaman a
> `demanda_solicitar()` (símbolo `demanda_solicitar()`).
>
> Se conserva tachado porque describe lo que costaría construirla, si algún día se quiere.

### Paso 4: Configuración de la Salida de Relé (Contacto Seco)

> 🟢 **04/09 — LA SALIDA VA EN `NO`, EN LAS TRES ENTRADAS, Y YA NO DEPENDE DE NINGUNA MEDIDA
> PENDIENTE.** ~~La salida de la AcuSense **es configurable `NO`/`NC`**~~, así que se elige **qué
> estado significa demanda sin tocar placa ni firmware**; lo que decide cuál es la correcta **es el
> cobre**, y el cobre está medido.
>
> ### ⛔ 05/09 — LA FRASE TACHADA ARRIBA SE CAE: `NO`/`NC` CONFIGURABLE ES 🔴 `SIN VERIFICAR`
>
> **Lo que sigue en pie:** *«lo que decide cuál es la correcta es el cobre, y el cobre está medido»*.
> **`M3` y el paso 21 no se tocan** — el pull-down de 10 kΩ es real y el equipo necesita que el
> contacto **CIERRE** para leer ALTO. **La necesidad es la misma; lo que se cae es dar por hecho que
> la cámara deja elegirlo.**
>
> | fuente oficial de la `DS-2CD2683G2-IZS` | qué dice sobre `NO`/`NC` de la SALIDA |
> |---|---|
> | Ficha, pág. 3, fila *Alarm* | `1 input, 1 output (max. 24VDC/24 VAC, 1 A)` — **y nada más** |
> | Manual, *Set Alarm Input*, pág. 44 *(PDF 57)* | el desplegable **`Alarm Type`** existe… **para la ENTRADA**: *«Select Alarm Input NO. and **Alarm Type** from the dropdown list»*. 🔴 **Y OJO CON ESE `NO.` — ver el bloque de abajo** |
> | Manual, *Automatic Alarm*, pág. 68 | los únicos parámetros de la **SALIDA** son **`Alarm Output No.`**, **`Alarm Name`** y **`Delay`** |
> | Las 110 páginas del manual | **`Normally Open` / `Normally Closed` no aparecen ni una sola vez** |

> ### 🔴 07/09 — UNA TRAMPA DE LECTURA EN LA FUENTE DE ARRIBA, Y HAY QUE DESACTIVARLA
>
> **`Alarm Input NO.` de la pág. 44 NO es «entrada de alarma normalmente abierta». `NO.` con punto
> es la abreviatura de *Number*: «el NÚMERO de entrada de alarma».** Se demuestra sin salir del
> mismo documento: la pantalla de la **salida** usa el mismo campo escrito `Alarm Output **No.**`
> —*«Select the alarm output No. according to the alarm interface connected»*, **PDF pág. 80 ·
> impresa 68**—, y ahí nadie leería «salida normalmente abierta».
>
> **Consecuencia: la pág. 44 documenta el desplegable `Alarm Type` y NO documenta ni uno solo de sus
> valores.** Quien cite esa página como prueba de que la entrada admite `NO`/`NC` está citando el
> nombre del campo, no su contenido.
>
> 🟢 **Y hay UNA aparición real de `NO` como valor en las 110 páginas, en un sitio donde nadie había
> mirado** — se encontró al buscar la cadena `alarm type` insensible a mayúsculas en todo el
> documento, no sólo el rótulo con mayúsculas:
>
> ```text
>   Manual de usuario, "Set Vehicle Detection" (Road Traffic), PDF pag. 99 / impresa 87:
>
>     "When Alarm Input is selected, the alarm input A<-1 will automatically be
>      assigned to trigger vehicle detection and its alarm type is always NO."
> ```
>
> **Lo que eso prueba y lo que no:**
>
> | | |
> |---|---|
> | ✅ **`NO` SÍ existe como valor de `Alarm Type`** en el vocabulario de esta cámara | es de la **ENTRADA**, y en un modo —*Road Traffic*— que este proyecto **no usa** y que el propio manual marca *«only supported by certain device models»* |
> | ✅ Dice **`always NO`** | o sea que en ese modo **NO se elige**: se impone. No demuestra que el desplegable ofrezca alternativa |
> | 🔴 **`NC` sigue en CERO apariciones** en las 110 páginas, igual que `Normally Open` y `Normally Closed` | — |
> | 🔴 **De la SALIDA sigue sin haber ni una palabra sobre polaridad** | — |
>
> **La conclusión de este apartado NO cambia: la polaridad de la SALIDA es 🔴 `SIN VERIFICAR` y la
> cierra el `ENSAYO 1` con un óhmetro.** Lo que cambia es la evidencia con que se sostenía, y eso se
> corrige igual — una excepción que se apoya en una frase mal leída es un defecto con permiso
> (`CLAUDE.md` §2.ter).
>
> 🔎 **Y el buscador está descartado (`CLAUDE.md` §4):** `Alarm Type` **sí** se encuentra en el PDF, y
> una búsqueda de `"NO/NC"` limitada a `hikvision.com` devuelve fichas donde el fabricante lo escribe
> sin ambigüedad —*«4 Relay Outputs(NO/NC)»* en sus radares y centrales—. **Hikvision sabe escribir
> `NO/NC` cuando lo hay. En la ficha de esta cámara no está.**
>
> 🟢 **Por qué esto NO bloquea el montaje, y es importante entenderlo:** el diseño necesita
> **`NO` — abierto en reposo, cerrado al detectar**, que es el comportamiento natural de una salida de
> alarma. **Lo más probable es que ya sea así de fábrica y no haya nada que elegir.** Pero *probable*
> no es *medido*, y esto se comprueba **en diez segundos con un multímetro**, sin abrir un menú:
> **es el `ENSAYO 1` del §6, que ya está escrito.**
>
> | lo que dé el `ENSAYO 1` en REPOSO | qué significa | qué se hace |
> |---|---|---|
> | **circuito ABIERTO** *(sin pitido)* | la salida es **`NO`**. **Es lo que este diseño necesita** | ✅ **Nada. Se sigue.** |
> | **circuito CERRADO** *(pita en reposo)* | la salida se comporta como **`NC`** | 🛑 **Se para.** Buscar `Alarm Type` en la pantalla de salida; **si no existe la opción, el pin nacería ALTO** y —por la siembra de `camaras_sembrar()`— **no habría flanco hasta que el relé abriera y cerrara otra vez**. Es la inversión que costó `N-67`. **Se avisa antes de cablear** |
>
> | destino | reposo del pin lo fija | cómo se cablea el contacto | configuración |
> |---|---|---|---|
> | **`J16` p10 / p12** 🔵 **EL DE HOY (`D-2`/`D-3`)** | ✅ **MEDIDO EN BANCO el 04/09** (`M3`, paso 20): **9,93 kΩ** y **9,94 kΩ** a masa, los dos a **0 V** con energía. Pull-**DOWN** real de 10 kΩ | entre el pin de señal y el borne de **3,3 V contiguo** (`p9` para `p10`, `p11` para `p12`). **Dos hilos, sin tercero a masa** — §4.bis.5 | **`NO`**, pulso **1 s** |
> | ~~**`PB0` / `J14`** (el de hoy)~~ **`PB0` / `J14`** — 🔴 **HOY NO LLEVA CÁMARA** | ✅ **MEDIDO**: `R64` 10 kΩ a masa + `C25` 100 nF (`pines.h:43-46`) | ~~entre el pin y el borne de **3,3 V** de `J14` — **NO contra `GND`**~~ · se conserva por si el borne se usa algún día | ~~**`NO`**, pulso **1 s**~~ |
>
> 🔴 **CORREGIDO EL 07/09 — la etiqueta *«(el de hoy)»* estaba en la fila equivocada.** Gana
> `DECISIONES.md` `D-2`/`D-3`: **la cámara va a `J16` p10/p12**. Y el motivo que decide no es el
> antirrebote sino la vigilancia: **`CAM_CIEGA`/`CAM_PEGADA` miran `J16` y no miran `J14`** — una
> cámara en `J14` funciona y **nadie sabría que se estropeó**. La fila de `J14` se conserva tachada
> porque su medida de cobre sigue siendo cierta y el borne sigue vivo. 🔴 **11/09 — CONFLICTO
> ABIERTO, no resuelto aquí:** `A-2` reserva `J14` para el fin de carrera de la pluma, y el firmware
> sigue leyendo `PB0` como cámara (`CAM_DEMANDA_PIN`): lo que se cablee ahí entra como demanda.
>
> ~~**Según lo que dé la medida M3**, con la tarjeta energizada y `J16` vacío:~~ ⛔ **Esta tabla de
> tres ramas ya no se ejecuta: `M3` la resolvió en la primera.** Se conserva porque **es el
> procedimiento con el que se comprueba una tarjeta nueva**, y una placa de otro lote no está medida
> por que ésta lo esté.
>
> | lectura en el pin | qué significa | cómo se cablea |
> |---|---|---|
> | **≈ 0 V, y 10 kΩ a masa** ✅ **ES LO QUE DIO** | pull-**DOWN** de 10 kΩ: el netlist tiene razón, **activo en ALTO** | contacto entre el pin de señal y el pin de **3,3 V** contiguo (`J16` p9 para p10, p11 para p12) |
> | ~~**~3,3 V**~~ | pull-**UP**: el netlist no describe esta placa | contacto contra **`GND`** (`J16` p2) — **y habría que invertir la lectura de cámara en el firmware antes de cablear**, porque hoy lee `== HIGH`. **No es el caso de esta tarjeta** |
> | ~~**otra cosa**~~ | ni una ni otra | **no se cablea.** Se anota el número y se para. **No es el caso de esta tarjeta** |
>
> 🛑 **`NC` no se usa en ninguno de los dos casos.** Con `NC` el contacto está cerrado en reposo y se
> abre al detectar: el firmware vería **demanda permanente** mientras no pasa nada y **ausencia de
> demanda** justo cuando pasa un vehículo. Es la inversión exacta que ya costó `N-67`. **Que la
> cámara admita `NC` no lo convierte en una opción: la admite, y aquí se pide `NO`.**
>
> ✅ **El tercer resultado que `M3` podía dar —que no hubiera resistencia ninguna— queda descartado
> por medida.** El firmware pone el pin en `pinMode(INPUT)` **pelado** (símbolo `botones_setup()`: `pinMode(CAM_C_PIN, INPUT)`),
> sin pull-up ni pull-down internos, así que **el reposo lo tenía que fijar cobre real**: lo fija,
> son 9,93 y 9,94 kΩ. **Y el paso 21 lo confirmó por el otro lado:** en reposo, con y sin el cable
> puesto, **el equipo no pide paso por sí solo** — cero demandas fantasma.

**Manual, *Alarm Output* y *Automatic Alarm*, págs. 67-68. Los rótulos van tal como salen del manual.**

1. Ir a **`Configuration → Event → Basic Event → Alarm Output`**
   *(Configuración → Evento → Evento Básico → Salida de Alarma)* — **manual, pág. 67**.
   Configurar la **`Automatic Alarm`** *(Alarma Automática)* — **pág. 68**:
   * **`Alarm Output No.`** *(N.º de Salida)*: **`A->1`**, la única que tiene esta cámara
     *(ficha: `1 output`)*. Son los bornes físicos **`1A`** y **`1B`** *(Guía rápida, pág. 8)*.
   * **`Alarm Name`** *(Nombre)*: libre. Sugerencia: `DEMANDA`.
   * **`Delay`** *(Retención del relé)*: **el valor MÁS CORTO que ofrezca el desplegable**, idealmente
     **`1 s`**. Ver el bloque ⏱️ de abajo — **es el parámetro delicado de toda esta pantalla**.
   * **`Arming Schedule`**: **24 × 7**, igual que en el Paso 3 y por el mismo motivo.
   * ~~**Estado por Defecto:** **`NO` (Normally Open / Normalmente Abierto)**~~
     ⛔ **RETIRADO: ese campo NO está documentado para la salida.** Ver el bloque ⛔ de arriba. **Lo
     que hay que hacer en su lugar es MEDIR el reposo con el `ENSAYO 1`.**
2. Volver a la regla de **`Intrusion Detection`** del Paso 3, ir a **`Linkage Method`**
   *(Método de Vinculación)* y marcar ☑ **`Trigger Alarm Output`** *(Disparar Salida de Alarma)* —
   **manual, pág. 67**.
   > 🔴 **Éste es el punto que el Paso 0 manda comprobar antes que nada.** El manual advierte
   > *«**This function is only supported by certain models**»*, y la fila *Linkage Method* de la ficha
   > de este modelo **no menciona la salida de alarma**. **Si la casilla no está, se para y se
   > avisa** — el diseño de `J16` dependería de otro camino.
3. **DESARMAR LOS EVENTOS BÁSICOS.** Entrar uno por uno y **dejar `Trigger Alarm Output`
   DESMARCADO** *(una salida = un único significado: «hay un vehículo esperando»)*:
   * **`Configuration → Event → Basic Event → Motion Detection`** *(pág. 40)*
   * **`Configuration → Event → Basic Event → Video Tampering`** *(pág. 43)*
   * **`Configuration → Event → Basic Event → Exception`** *(pág. 43)*
   > 🛑 **No es cosmética.** *Exception* salta, entre otras, con **pérdida de red o de disco**: si
   > estuviera enlazada al relé, **el semáforo recibiría una demanda de paso cada vez que la cámara
   > tuviera un problema interno**. Una avería de la cámara no es un vehículo esperando.
4. Pulsar **`Save`** *(Guardar)* en cada pantalla.

> ## ⏱️ CUÁNTO TARDA Y CUÁNTO DURA EL CONTACTO — y por qué esto le importa al firmware
>
> **El firmware detecta el FLANCO DE SUBIDA, no el nivel** (✅ **MEDIDO**: `camaras_actualizar()`,
> símbolo `camaras_actualizar()` de `Maestro/src/botones.cpp`). **Un contacto que se queda pegado NO vuelve a pedir paso hasta
> que se suelte y cierre otra vez.** De ahí que la retención del relé no sea un detalle.
>
> ### Lo que tarda en cerrar
>
> | tramo | valor | nivel |
> |---|---|---|
> | Vehículo entra en la zona → **la analítica lo da por válido** | **≥ el `Threshold` configurado** — con `1 s`, **un segundo como mínimo**. Es su definición literal: la alarma salta cuando *«the time that one object stays exceeds the threshold»* | 📖 **ESCRITO** — manual, pág. 49 |
> | La analítica dispara → **el contacto cierra** | 🔴 **SIN VERIFICAR.** El manual **no publica** el retardo interno de la salida de alarma | 🔴 |
> | **Total detección → contacto cerrado** | 🔴 **SIN VERIFICAR**, pero **no menor que el `Threshold`** | — |
>
> 🟢 **No es crítico, y conviene decirlo para que nadie lo persiga:** ~~el equipo responde a una demanda
> con un ciclo de **amarillo `4 s` + despeje todo-rojo** por delante.~~ ✏️ **11/09 — corregido: la
> demanda de una cámara sólo la atiende el Modo Inteligente** (en Automático y en Manual no cambia
> ninguna luz), y **ni ahí adelanta un cambio**: por debajo del tiempo configurado de la fase no pasa
> nada, y cumplido, sólo decide si la fase se mantiene (con un tope) o cambia como en Automático —
> símbolos `sueloMs`/`techoMs` de `modo_inteligente.cpp`. **Un retardo de uno o dos segundos en el
> contacto se pierde dentro de eso.** Lo que sí importa es que el contacto **suelte**.
>
> ### Lo que dura cerrado — el `Delay`, y sus tres casos
>
> **`Delay`** está documentado —*«the time duration that the alarm output remains after an alarm
> occurs»*, manual pág. 68—, **pero el manual NO publica los valores del desplegable**:
> 🔴 **que `1 s` esté disponible es `SIN VERIFICAR`.**
>
> | si el `Delay` disponible es… | qué pasa en el equipo |
> |---|---|
> | **`1 s` – `3 s`** | ✅ **Ideal.** Cada vehículo que espera da su flanco. El firmware ya ignora repeticiones dentro de **`3 000 ms`** (`SILENCIO_MS`, `demanda.cpp:8` — ✅ **MEDIDO**), así que **por debajo de 3 s el valor exacto da igual** |
> | **`5 s` – `30 s`** | ⚠️ **Funciona, con menos finura.** El contacto tarda en soltarse, así que **una cola de vehículos produce UNA demanda por cada `Delay`** en vez de una por vehículo. **No es peligroso** —el ciclo ya tiene sus mínimos— pero **el carril tarda más en volver a pedir paso**. Se anota el valor real en la ficha de ensayo |
> | **`Manual`** *(el manual lo lista como opción, pág. 68)* | 🛑 **NO SE USA JAMÁS.** El relé **se queda cerrado hasta que alguien lo borre a mano** desde el navegador. El pin quedaría ALTO para siempre, **no habría un solo flanco más**, y el equipo **dejaría de recibir demanda de ese carril** ~~**sin dar ningún síntoma**~~ ✏️ *11/09 — corregido: el vigilante SÍ lo acusa, pero tarde: con el contacto cerrado sin abrirse el equipo acaba publicando `CAM_PEGADA` (`vigilante_nivel()`); hasta entonces nadie se entera, y en Modo Inteligente puede alargar verdes hasta su tope aunque no pase nadie* |
>
> 🔴 **Y un hallazgo que sale de cruzar esto con el firmware, y que hay que dejar por escrito:** el
> comentario que justifica los `3 000 ms` de `SILENCIO_MS` dice *«el rele de la camara AcuSense cierra
> ~1 s por deteccion»* (cabecera de `Maestro/src/demanda.cpp`, encima de `SILENCIO_MS`). **Ese «~1 s» es exactamente el dato que acaba
> de quedar `SIN VERIFICAR`**: una constante del firmware apoyada en un comportamiento de la cámara
> que nadie ha medido nunca. **No se toca el firmware por esto** —los tres casos de la tabla lo
> aguantan— pero **el `ENSAYO 1` tiene que anotar el número real**, y si no sale `~1 s`, ese
> comentario hay que corregirlo.

---

## 4.bis 🔌 CABLEADO A LA PLACA — EL PASO A PASO, CON EL DESTORNILLADOR DELANTE DEL GABINETE

**Este apartado es para quien está de pie delante del equipo, con la cámara en la mano y sin
contexto de este proyecto.** Todo lo que hace falta para ejecutarlo está aquí: no hay que leer los
apartados anteriores para cablear, aunque sí para entender por qué.

**Lo que se cablea hoy son DOS hilos por cámara.** Nada más. Ni red, ni vídeo, ni datos: el sistema
consume **un contacto seco** de cada cámara *(`D-12` de `DECISIONES.md`)*. 🎯 **11/09 (`D-25`): son
DOS cámaras por poste**, así que en cada tarjeta se cablean **cuatro hilos**: cámara 1 a `p9`/`p10` y
cámara 2 a `p11`/`p12`. Y **cada cámara se comprueba por separado en su borne** (§4.bis.7): la app no
distingue una de otra.

### 4.bis.0 🛑 EL ORDEN, Y NO ES UNA PREFERENCIA: FIRMWARE PRIMERO, HILO DESPUÉS

**Antes de enchufar un solo hilo en `J16`, el firmware nuevo tiene que estar CARGADO Y VERIFICADO EN
LA TARJETA.** No basta con que el cambio esté comiteado, ni mergeado, ni aprobado: **un commit no
protege de un destornillador.**

```text
  CON EL FIRMWARE NUEVO DENTRO  ->  SEGURO
    pinMode(CAM_C_PIN, INPUT) pelado + R67 10K a masa  =  el pin esta a 0 V
    y no ejecuta nada.  Es una entrada de camara.

  CON EL FIRMWARE VIEJO DENTRO  ->  PELIGROSO
    PB14 sigue siendo botonAceptar(), leido ACTIVO EN BAJO.  Cualquier cosa
    que un instalador enchufe en J16 p10 puede pulsar "Aceptar" en un equipo
    QUE ESTA EN LA CALLE gobernando un cruce.
    11/09 (D-25): Y PB15 sigue siendo botonCancelar(): la camara 2 en J16 p12
    puede pulsar "Cancelar".  Vale para la V8.4 que esta en campo (e303485).
```

**La asimetría es el punto:** cargar el firmware primero **no puede hacer daño** —un pin en `INPUT`
pelado no manda nada—; cablear primero **sí**. Por eso no vale «van en el mismo commit»: **se exige
la carga verificada en la tarjeta, no el merge.**

**Cómo se comprueba cada afirmación de este bloque, con el símbolo y su búsqueda:**

```text
  grep -rn "CAM_C_PIN" 01_Firmware/Maestro/include/pines.h
      -> #define CAM_C_PIN  PB14   // J16 p10
  grep -rn "pinMode(CAM_C_PIN" 01_Firmware/Maestro/src/botones.cpp
      -> pinMode(CAM_C_PIN, INPUT);      <-- INPUT PELADO, sin pull interno
  grep -n -A6 "bool camara_leerPin" 01_Firmware/Maestro/src/botones.cpp
      -> if (digitalRead(pin) == HIGH)   <-- ACTIVO EN ALTO
  Lo mismo, identico, en 01_Firmware/Esclavo/  (pines.h y src/botones.cpp)
```

✅ **MEDIDO el 05/09** ejecutando esas tres búsquedas sobre el fuente de las **dos** puntas.

### 4.bis.1 Lo que hay que tener a mano

| | |
|---|---|
| **Multímetro** | con posición de **continuidad** y de **tensión continua**. Es la herramienta de este apartado, y no es opcional |
| **Tapón para `J16` p1** | tapón ciego, funda termorretráctil, o el propio conector armado **sin terminal en la posición 1** |
| **Cable de dos hilos** por cámara | para el contacto seco. **Sin polaridad**, así que da igual el color de cada uno |
| **La alimentación de la cámara** | inyector PoE **802.3at** o el cable de **12 V** a batería. Ver **4.bis.2** |
| **El firmware ya cargado** | ver **4.bis.0**. Si esto no está hecho, **no se sigue** |

### 4.bis.2 PASO 1 — ALIMENTAR LA CÁMARA (y el número que decide la batería)

La cámara admite **dos formas de alimentación, y basta con una**:

| forma | qué hace falta | consumo | fuente |
|---|---|---|---|
| **PoE** | inyector o switch **`802.3at` (PoE+), Clase 4**. Un `802.3af` **NO** basta | **máx. `15 W`**, 42,5–57 V, 0,36 a 0,27 A | 📖 ficha, pág. 4, *Power Supply* y *Power Consumption and Current* |
| **12 V DC** | jack coaxial **Ø 5,5 mm** | **`1,08 A`, máx. `13 W`** | 📖 ficha, pág. 4 |

**Tensión admisible: `12 V DC ± 25 %`, o sea de `9 V` a `15 V`** *(ficha, pág. 4, fila Power
Supply)*. Ese rango cubre entero el vaivén de una batería de plomo de 12 V —de ~10,5 V descargada a
~14,4 V en carga—, así que **no hace falta convertidor**.

🛑 **LA CÁMARA VA DIRECTA A LA BATERÍA. NUNCA AL REGULADOR DE 5 V DE LA TARJETA.**

```text
  BATERIA 12 V ---+--- camara  (1,08 A)          <-- ASI SI
                  |
                  +--- J1 de la tarjeta --> LM7805 --> 5 V --> LM1117 --> 3,3 V --> STM32

  1,08 A por el LM7805 (12 -> 5 V lineal) son 7,5 W de disipacion EN EL
  REGULADOR QUE MANTIENE VIVO AL MICRO QUE GOBIERNA EL SEMAFORO.
```

🛑 **Y el número que hay que poner delante de quien dimensiona la energía, porque no lo decide este
manual:**

```text
  ~~Por poste, una camara:      13 W               (maximo de ficha)~~   <-- CADUCADO 11/09 (D-25)
  ~~En 24 h:                    13 W x 24 h  =  312 Wh~~
  ~~A 12 V, eso es:             312 / 12     =   26 Ah/dia   SOLO LA CAMARA~~

  11/09, D-25 -- DOS camaras por poste:
  Por poste, dos camaras:     2 x 13 W     =   26 W   (maximo de ficha)
  En 24 h:                    26 W x 24 h  =  624 Wh
  A 12 V, eso es:             624 / 12     =   52 Ah/dia   SOLO LAS CAMARAS
```

🔵 **`13 W` es el MÁXIMO de la ficha, no el consumo medio**: incluye los IR a plena potencia de
noche. **Cuánto menos consume de día es 🔴 `SIN VERIFICAR`**, y se mide con pinza amperimétrica con
la cámara montada, no antes.

### 4.bis.3 PASO 2 — IDENTIFICAR LA BORNERA: `J16`, NO `J17`

⚠️ **`J16` y `J17` comparten footprint y son idénticos a la vista** —`Molex KK-254`, uno al lado del
otro—. Enchufar la cámara en `J17` no cablea nada; enchufar el módulo Bluetooth en `J16` **le mete
12 V y lo quema**.

**Se distinguen con el multímetro, no con la vista:**

1. Multímetro en **tensión continua**, punta negra a masa, punta roja en la **posición 1** del
   conector, con el equipo **encendido**.
2. **Si marca ≈ 12 V, es `J16`** — ahí va la cámara.
3. **Si no marca 12 V, es `J17`** — ahí NO va la cámara. Se busca el otro.

✅ **MEDIDO** sobre el `.kicad_pcb`: `J16` p1 es la red `12V`, y `J17` no reparte 12 V en ningún pin
*(`03_Hardware_Tarjeta/MAPEO_TARJETA_KICAD.md`, apartados «`J16` — botones» y «`J17`»)*.

### 4.bis.4 PASO 3 — 🛑 TAPAR EL PIN 1 DE `J16`. OBLIGATORIO, EQUIPO POR EQUIPO

**`J16` p1 lleva 12 V crudos, y es el único conector de señal de toda la tarjeta que los trae.** Las
cinco entradas de campo van **desnudas al pin del STM32** —sin resistencia en serie, sin
optoacoplador, sin clamp—, así que un roce de 12 V contra cualquiera de ellas llega directo a la
patilla del micro que gobierna el semáforo. **El 04/09 una tarjeta Maestro quedó con un
cortocircuito de 3,3 V a masa en banco.**

1. **Tapar físicamente la posición 1** —tapón, termorretráctil, o armar el conector sin terminal ahí—.
2. **Comprobar que quedó tapada** antes de seguir.
3. **En el p1 no se conecta nada, nunca.**

🛑 **Esto es la decisión `D-4` de `DECISIONES.md`: se hace en CADA equipo que se monte.** No es
cautela de banco ni una recomendación.

🔴 **Lo que tapar el pin NO arregla:** protege del error de cableado, **no** de una sobretensión que
entre por el hilo de campo. La protección de verdad —**2K2 en serie en las cinco entradas**— es una
modificación de la **revisión V2 de la placa** y **hoy no existe en el cobre**.

### 4.bis.5 PASO 4 — LA SALIDA DE ALARMA A LA PLACA: **`1A` y `1B` SON UNA PAREJA**

🔴 **ESTO ES LO QUE MÁS SE LEE MAL, Y HAY QUE DECIRLO CON TODAS LAS LETRAS: `1A` y `1B` NO SON DOS
SALIDAS. SON LOS DOS BORNES DE UNA SOLA.** Un contacto tiene dos extremos; ésos son sus nombres. Si
alguien los cablea como si fueran dos salidas independientes, **lo hace mal**.

**Lo dice la guía rápida, literal:**

```text
  ALARM OUT   Alarm output interface
              1A and 1B, 2A and 2B, 3A and 3B are THREE PAIRS of alarm outputs

  ALARM IN    Alarm input interface
              IN1 and GND1, IN2 and GND2 are TWO PAIRS of alarm inputs
```

✅ **VERIFICADO el 05/09 leyendo la página con los ojos**, no con un extractor: la guía rápida que
está en `04_Manuales/` es un **PDF de imagen —40 páginas y CERO caracteres de texto, medido con
`PyMuPDF`—**, así que la página se renderizó a PNG y se miró. Es la **página 9 de las 40 del PDF,
impresa como página 8**, tabla *Interface / Description*.

**Nuestra cámara tiene `1 input, 1 output`** *(ficha, pág. 3, fila Alarm)*, luego de esos tres pares
**le corresponde el primero**:

| lo que trae la cámara | bornes | qué es |
|---|---|---|
| **la salida** *(`1 output`)* | **`1A`** + **`1B`** | **un contacto**, con sus dos extremos. **Es lo que se cablea hoy** |
| **la entrada** *(`1 input`)* | **`IN1`** + **`GND1`** | **hoy NO se cablea.** Ver **4.bis.6** |

⚠️ **Y el aviso de la propia guía, que va pegado a esa tabla:** *«The interface varies with the
models. Please refer to the product datasheet for details.»* Los tres pares son de los modelos que
los llevan; **el nuestro trae uno**.

#### Qué cámara va a qué poste, y a qué pines

~~**Hay UNA cámara por poste** *(`D-13` de `DECISIONES.md`)*~~ → 🎯 **11/09, `D-25`: hay DOS cámaras
por poste, cuatro en el cruce**, y **cada una se cablea a la tarjeta de SU PROPIO poste**. No se llevan
las de un poste a la placa del otro: el Esclavo transmite su demanda al Maestro por radio, no por
cobre.

| poste | tarjeta | ~~bornera y pines~~ | ~~queda libre~~ |
|---|---|---|---|
| ~~**Poste del Maestro**~~ | ~~la del Maestro~~ | ~~**`J16` p10** *(señal)* y **`J16` p9** *(3,3 V)*~~ | ~~`p12` y `p11`~~ |
| ~~**Poste del Esclavo**~~ | ~~la del Esclavo~~ | ~~**`J16` p10** *(señal)* y **`J16` p9** *(3,3 V)*~~ | ~~`p12` y `p11`~~ |

**La tabla que vale desde el 11/09 (`D-25`)** — igual en las dos tarjetas:

| poste | tarjeta | cámara 1 | cámara 2 | queda libre en `J16` |
|---|---|---|---|---|
| **Poste del Maestro** | la del Maestro | **`J16` p10** *(señal, `CAM_C_PIN`)* y **`p9`** *(3,3 V)* | **`J16` p12** *(señal, `CAM_D_PIN`)* y **`p11`** *(3,3 V)* | **nada de cámara.** `p5`/`p8` NO se tocan (mando retirado, el firmware los sigue leyendo); `p1` **tapado** |
| **Poste del Esclavo** | la del Esclavo | **`J16` p10** y **`p9`** | **`J16` p12** y **`p11`** | ídem |

**Las dos cámaras de un poste hacen exactamente lo mismo** —`botones.cpp` las recorre en un solo
bucle, `CAM_J16[2] = {CAM_C_PIN, CAM_D_PIN}`— **y llevan la misma configuración** (`D-13`). Da igual
cuál se monte arriba o abajo del mástil: lo que no da igual es que **cada una quede comprobada en su
propio borne** (§4.bis.7) y **anotada en la ficha** —las alarmas del equipo nombran el pin, `CAM_C`
(= `p10`) o `CAM_D` (= `p12`), no la cámara—.

~~**Por qué `p10` y no `p12`, y es una medida, no un gusto:**~~ **La medida que sigue en pie, y ahora
es un aviso de montaje y no una elección:** sobre el cobre, la separación real hasta
la red de 12 V es de **4,269 mm en `p10`** *(red `/Boton3`)* y de sólo **1,359 mm en `p12`** *(red
`/Boton4`, el peor de toda la placa)*. ✅ **MEDIDO** sobre el `.kicad_pcb`
*(`03_Hardware_Tarjeta/MAPEO_TARJETA_KICAD.md`, apartado «La distancia real entre los 12 V y las
señales»)*. ~~**Con una sola cámara por tarjeta, va en el pin con más margen.**~~ → **11/09 (`D-25`):
`p12` lleva la cámara 2, así que se cablea con `p1` YA tapado (`D-4`), con el terminal bien crimpado y
sin restos de cobre ni humedad en el conector.**

> ⚠️ **`/Boton3` y `/Boton4` son NOMBRES DE RED DEL NETLIST, no el papel del pin — y aquí es donde
> más se malinterpretan, porque van al lado de una medida de cobre.** Las pistas se bautizaron así
> cuando la placa se diseñó con botonera, y **el nombre no se puede cambiar sin retocar el
> `.kicad_sch`**. **Hoy `J16` p10 es `CAM_C_PIN` y p12 es `CAM_D_PIN`: los dos son ENTRADAS DE
> CÁMARA.** Quien lea *«/Boton3»* como «el botón 3» y cablee ahí un pulsador está cableando en el
> borne de una cámara. **La botonera no se monta** (`D-1`, 05/09) y **`botonAceptar()`/`botonCancelar()`
> —los que colgaban de estos dos pines— devuelven `false` siempre**.

#### Los cinco pasos del contacto

1. **Comprobar que el `PASO 3` está hecho:** `J16` p1 tapado. Si no lo está, se para aquí.
2. **Un hilo de `1A` a `J16` p10** —el pin de señal, `PB14` / `CAM_C_PIN`—.
3. **El otro hilo de `1B` a `J16` p9** —los 3,3 V del borne de al lado—.
4. **No importa cuál va a cuál: es un contacto y NO TIENE POLARIDAD.** `1A` a `p9` y `1B` a `p10`
   funciona exactamente igual.
5. **Y son DOS hilos, sólo dos. No se lleva un tercero a masa.**
6. 🎯 **11/09 (`D-25`) — LA CÁMARA 2, igual pero dos posiciones más allá:** un hilo de su `1A` a
   **`J16` p12** —`PB15` / `CAM_D_PIN`— y el otro de su `1B` a **`J16` p11** —sus 3,3 V—. Sin
   polaridad, dos hilos, nada a masa. **`p12` es el borne con menos margen a los 12 V**: `p1` tiene
   que estar tapado ANTES de este paso. El dibujo de abajo vale igual cambiando `p9`→`p11`,
   `p10`→`p12` y `R67`→`R68`.

```text
        CAMARA                                TARJETA (J16)
   +---------------+                    +----------------------+
   |           1A  |------- hilo 1 -----| p9   3,3 V           |
   |  (contacto)   |                    |                      |
   |           1B  |------- hilo 2 -----| p10  PB14 (senal)    |
   +---------------+                    |        |             |
                                        |        R67 10K       |
                                        |        |             |
                                        |       GND            |
                                        |                      |
                                        | p1  12 V  <-- TAPADO |
                                        +----------------------+

   CONTACTO ABIERTO  ->  R67 tira el pin a masa   ->  0 V    ->  no hay demanda
   CONTACTO CERRADO  ->  los 3,3 V llegan al pin  ->  3,3 V  ->  DEMANDA
```

🛑 **POR QUÉ CONTRA LOS 3,3 V Y NO CONTRA MASA — es el error que ya costó `N-105`.** La entrada es
**ACTIVA EN ALTO**: el pin nace en 0 V por la resistencia `R67` de **10 kΩ a masa**, así que llevar
el contacto a `GND` lo deja en 0 V **abierto y cerrado**, y **la cámara no dispara nunca**. El ensayo
saldría *«sin detección»* sin que nada esté roto. ✅ **MEDIDO en cobre el 03-04/09**: 9,93 kΩ en
`p10` y 9,94 kΩ en `p12`, los dos a 0 V con la tarjeta energizada.

⛔ **Y aquí se retira una frase que este mismo manual repetía: ~~«retorno de masa por `p2`»~~.** Ese
tercer hilo **no existe en este circuito y no hay que ponerlo**. El contacto es seco y flotante: la
corriente sale de los 3,3 V del `p9`, cruza el contacto, entra por el `p10` y **vuelve a masa por
`R67`, dentro de la propia tarjeta**. La prueba de que ése era el diseño está en la otra bornera de
cámara: **`J14` tiene exactamente dos pads, y ninguno de los dos es masa** —`p1` es la señal (`PB0`)
y `p2` son los 3,3 V—. ✅ **MEDIDO** sobre el `.kicad_pcb`. *(Si alguien quiere aterrizar la malla de
un cable apantallado, eso es otra cosa y **no se decide en este manual**: no va a `J16` p2 sin
preguntar.)*

🟢 **El régimen del contacto sobra, y es una cuenta:** aguanta `24 V` y `1 A` *(ficha, pág. 3)* y aquí
conmuta `3,3 V` y `3,3 / 10000 = 330 µA`. Margen de **7,3 veces** en tensión y **3.030 veces** en
corriente. **La pregunta que sí queda abierta es la contraria** —si `330 µA` es *demasiado poca*
corriente para un contacto sin baño de oro—: está desarrollada en **§1.1.1** y es 🔴 `SIN VERIFICAR`.

### 4.bis.6 PASO 5 — LA ENTRADA DE ALARMA (`IN1` / `GND1`): **HOY NO SE CABLEA**

**Se dejan sin conectar, y este manual dice por qué en vez de callarlo.**

`IN1`/`GND1` es la vía **contraria** a todo lo demás de este documento: no es la cámara avisando al
controlador, es **el controlador diciéndole algo a la cámara**. Es la vía de la decisión **`D-14`**:
*el controlador cierra un contacto cuando la luz está en rojo, y la cámara graba*. Tiene valor —la
cámara sabe hacer la `AND` ella sola, con `Motion & Alarm` en su *Record Schedule*— **pero no está
lista para cablearse**, y le faltan **dos cosas distintas**:

| lo que falta | estado |
|---|---|
| **Qué espera eléctricamente esa entrada** | 🔴 **`SIN VERIFICAR`** — ver la medida de abajo |
| **Qué canal de la placa se le asigna** | 🟡 **SIN DECIDIR.** ~~El `p12`/`p11` de `J16` queda libre, pero **no está asignado**~~ → **11/09 (`D-25`): `p11`/`p12` ya NO quedan libres — llevan la cámara 2.** En `J16` no queda ninguna posición de señal libre para esto (`p5`/`p8` son del mando retirado y el firmware los sigue leyendo) |
| **El lado del CONTROLADOR** | 🔴 **NO ESTÁ CONSTRUIDO — medido el 07/09, ver abajo.** No es «falta ajustar»: no existe |

> ## 🛑 07/09 — `D-14` NO ESTÁ CONSTRUIDO EN EL CONTROLADOR, Y ESTE MANUAL NO PUEDE DESCRIBIRLO COMO SI FUNCIONARA
>
> **`D-14` es una decisión de diseño con su lectura del manual del fabricante detrás, y es buena.
> Lo que no tiene es firmware.** Medido sobre el fuente el 07/09, en las **dos** puntas, censando
> **todas** las escrituras a pines:
>
> ```text
>   $ grep -rn "digitalWrite" 01_Firmware/Maestro/src 01_Firmware/Esclavo/src \
>       | grep -v "semaforo.cpp" | grep -v "^.*//"
>
>     Maestro/src/bluetooth.cpp   digitalWrite(RS485_IN_DE_RE, HIGH)
>     Maestro/src/protocolo.cpp   digitalWrite(LORA_DE_RE, LOW / HIGH / LOW)
>     Esclavo/src/bluetooth.cpp   digitalWrite(RS485_IN_DE_RE, HIGH)
>     Esclavo/src/protocolo.cpp   digitalWrite(LORA_DE_RE, LOW / HIGH / LOW)
> ```
>
> **Fuera de `semaforo.cpp` —que gobierna las luces y nada más (`CLAUDE.md` §6)— lo único que el
> firmware escribe es la DIRECCIÓN del bus RS485 y de la radio.** No hay ninguna salida de propósito
> general, ni una función que la encienda, ni un comando que la pida.
>
> **Y los tres canales de potencia que podrían serlo están declarados y muertos**, también medido
> hoy: `ROJO_PEATON` (`PA6`→`J11`), `VERDE_PEATON` (`PA7`→`J9`) y `BUZZER` (`PB1`→`J13`) aparecen
> **sólo en `pines.h`** de las dos puntas — sin `pinMode`, sin `digitalWrite`, sin llamador.
>
> 🔴 **Por qué esto va escrito aquí y no se calla:** un manual que describe una función inexistente
> es el defecto que este proyecto ya pagó con la **«Caja Negra de Alarmas»** —descrita en cuatro
> manuales, sin un solo llamador, y se cobró el día que un fallo de campo no se pudo diagnosticar
> porque no había registro que mirar—. **`D-14` es hoy un DISEÑO APROBADO, no una capacidad del
> equipo**, y cablear `IN1`/`GND1` no la haría aparecer.
>
> ✅ **Lo que sí funciona hoy sin firmware ninguno, y conviene saberlo antes de pedir el desarrollo:**
> la cámara **graba sola por su analítica** —`Trigger Recording` + `Record Schedule` tipo `Event`
> (**PDF pág. 48 · impresa 36**)—. **La `AND` con el estado del semáforo es lo que exige `D-14`; la
> grabación del evento, no.**

🔴 **Lo eléctrico no es un descuido de este manual: NO LO PUBLICA EL FABRICANTE.** El manual de
usuario **delega el cableado en la guía rápida**, y la guía rápida **no lo trae**:

```text
  Manual de usuario, "Set Alarm Input" / "Before You Start", PDF pag. 56 (impresa 44):
     "Make sure the external alarm device is connected.
      See Quick Start Guide for cable connection."

  Guia rapida, 40 paginas: la palabra "alarm" aparece SOLO en la pagina 9 (impresa 8),
  dentro de la tabla Interface/Description.  NI UN DIAGRAMA DE CABLE, ni una tension,
  ni una corriente, ni un color de hilo.
```

✅ **Y el buscador está descartado (`CLAUDE.md` §4), porque un «no aparece» no es un hallazgo hasta
haber descartado al buscador.** Medido el 05/09 sobre las **110 páginas** del manual de usuario, con
búsqueda **insensible a mayúsculas**, sobre los 160.348 caracteres que el extractor sí devuelve:

| lo buscado | veces que aparece en las 110 páginas |
|---|---|
| `relay` | **0** |
| `dry contact` | **0** |
| `Normally Open` y `Normally Closed` | **0** |
| `mA` | **0** |
| `Alarm Type` *(el desplegable — y es de la ENTRADA, no de la salida)* | **1** — PDF pág. 57, impresa 45 |

> 🔴 **CORREGIDO EL 07/09: esa última fila decía `1` y son `2`.** La búsqueda se hizo con el rótulo
> tal cual, `Alarm Type` con mayúsculas; **insensible a mayúsculas aparece también en PDF pág. 99 ·
> impresa 87** *(«…its **alarm type** is always NO»)*. Es §4 de `CLAUDE.md` en pequeño: **el
> buscador no estaba ciego por falta de herramienta, sino por el patrón.** La conclusión eléctrica
> de este apartado no cambia —**esa página no publica ni tensión ni corriente**—, pero un recuento
> publicado que está mal es un recuento que hay que corregir. El desarrollo, en §4 Paso 4.

**El extractor no está ciego:** sobre ese mismo texto, `Alarm Input` sí aparece —en tres páginas— y
`Quick Start Guide` también —en cuatro—. **Lo que no está es el dato eléctrico.**

🛑 **Consecuencia práctica, y es la única frase que hay que recordar de este apartado: `IN1` y `GND1`
se dejan sin conectar y se aíslan.** No se les enchufa «algo parecido a un contacto» a ver qué pasa:
es una entrada cuyo régimen no publica nadie, en un equipo que cuesta lo que cuesta.

### 4.bis.7 PASO 6 — COMPROBAR QUE QUEDÓ BIEN, ANTES DE DAR LA INSTALACIÓN POR BUENA

**Cuatro comprobaciones. No se firma la instalación sin las cuatro, y se anotan los números, no los
«correcto».** Las dos primeras son con el multímetro en el pin; las dos últimas son de
comportamiento.

1. **Reposo, sin nadie delante de la cámara.** Multímetro en tensión continua entre `J16` p10 y masa.
   **Tiene que dar `0 V`.** Si da otra cosa, el contacto está cerrado o hay un hilo donde no debe:
   **se para**.
2. **Con el contacto cerrado** —cruzando delante de la cámara, o puenteando `1A` con `1B` a
   propósito—. **Tiene que dar `3,3 V`.** Si sigue en 0 V, el contacto no cierra o los hilos están en
   pines equivocados.
3. **Criterio negativo, CON EL CABLE PUESTO:** en reposo, con la cámara conectada y nadie delante,
   **el equipo no debe pedir paso solo**. Se observa un ciclo entero.
4. **Criterio negativo, SIN EL CABLE:** se desconecta el cable de `J16` y se repite. **Tampoco debe
   pedir paso solo.**

> 🔴 **11/09 (`D-25`) — CON DOS CÁMARAS POR POSTE, LAS COMPROBACIONES 1 Y 2 SE HACEN DOS VECES: una
> en `p10` (cámara 1) y otra en `p12` (cámara 2), cada una provocando la detección delante de SU
> cámara.** Y **NO se sustituyen por la app**, por esto, medido en el firmware y en la app del 10/09:
>
> * la casilla **`CÁMARAS J16`** de la app pasa de **`SIN COMPROBAR`** a **`OK`** —con la frase
>   *«las dos ven y ninguna está pegada»*— **en cuanto CUALQUIERA de las dos da una detección**, y
>   **sigue en `OK` aunque la otra no haya detectado nunca**: `camara_estado()` salta el pin que
>   nunca dio un flanco. **No dice cuál, y con una muerta también sale `OK`;**
> * y el vigilante tampoco la cazará después: `vigilante_tick()` **no vigila el silencio de una
>   cámara que nunca dio un flanco**, así que **una cámara muerta desde el día de la instalación no
>   la detecta nadie**. Tras cada corte o reinicio vuelve a `SIN COMPROBAR` hasta la primera
>   detección, y es normal.
>
> **El multímetro en cada borne es la única prueba de que CADA cámara ve.** Se anotan las dos
> lecturas de cada pin en la ficha.

**Por qué las dos últimas y no una sola:** el cable de campo es una antena, y ~~`p10`/`p12` **no llevan
antirrebote de placa** —su único filtro son los 5 ms por software de `camara_leerPin()`—~~ *(✏️ 11/09:
el netlist dice lo contrario — `C28` y `C29`, 100 nF a masa en `/Boton3` y `/Boton4`, el mismo RC que
`J14`; los condensadores **no se han medido en cobre**, así que la prueba con el cable puesto sigue
siendo la que manda)*. La falsa
activación aparece **con el cable puesto**, que es justo el caso que una prueba de mesa se salta.

✅ **Este par ya se ejerció en banco el 04/09** *(paso 21 de la sesión)*: en reposo, con el cable y
sin él, **cero falsas activaciones**. Aun así **se repite en cada instalación**: lo que se midió es
*esa* tarjeta con *ese* cable.

#### 📋 Ficha de instalación — se rellena y se devuelve

**Esta ficha es parte del entregable.** Un hueco sin rellenar es una medida que no se hizo.

| # | qué se anota | valor |
|---|---|---|
| **1** | Equipo *(Maestro o Esclavo)* y nº de poste | ______________________ |
| **2** | ¿`J16` p1 **tapado**? *(sí / no)* | ______________________ |
| **3** | ¿Firmware nuevo **cargado y verificado** ANTES de cablear? *(sí / no)*, y su hash | ______________________ |
| **4** | Alimentación usada *(PoE 802.3at o 12 V a batería)* | ______________________ |
| **5** | Pines usados ~~*(p10 con p9, u otros — y por qué)*~~ → *11/09, `D-25`: cámara 1 en p10 con p9, cámara 2 en p12 con p11 — u otros, y por qué* · y **qué cámara (IP / posición en el mástil) quedó en cada pin** | ______________________ |
| **6** | Tensión en `p10` **en reposo** *(cámara 1, se espera 0 V)* | ____________ V |
| **7** | Tensión en `p10` **con el contacto cerrado** *(cámara 1, se espera 3,3 V)* | ____________ V |
| **6.bis** | 🆕 *11/09, `D-25`* — Tensión en `p12` **en reposo** *(cámara 2, se espera 0 V)* | ____________ V |
| **7.bis** | 🆕 *11/09, `D-25`* — Tensión en `p12` **con algo delante de la cámara 2** *(se espera 3,3 V)* | ____________ V |
| **8** | Falsas demandas **con el cable puesto**, un ciclo entero *(nº)* | ____________ |
| **9** | Falsas demandas **sin el cable**, un ciclo entero *(nº)* | ____________ |
| **10** | ¿`IN1` y `GND1` han quedado **sin conectar y aislados**? *(sí / no)* | ______________________ |
| **11** | Fecha, hora y quién lo hizo | ______________________ |
| **12** | 🆕 ¿Salió algún `$ALARM …CAM_PEGADA` **al terminar de cablear**? *(sí / no; si sí, a qué hora)* | ______________________ |
| **13** | 🆕 Estado que publica el equipo en el campo `CAM:` al acabar *(`OK` · `?` · `CIEGA` · `PEGADA`)* — ⚠️ *11/09: un `OK` NO prueba que vean las dos; lo prueban las filas 6/7 y 6.bis/7.bis* | ______________________ |

### 🆕 4.bis.7.bis EL EQUIPO VIGILA ESTA CÁMARA SOLO — qué le va a decir, y qué NO significa

> **Desde `4b90f98` (D-13 fase 1) el controlador observa las dos entradas de `J16` y avisa cuando
> una deja de comportarse como una cámara. `CAM_C` = `J16` p10 · `CAM_D` = `J16` p12.**
>
> 🔴 **11/09 (`D-25`) — LO QUE ESTE VIGILANTE NO VE, Y CON DOS CÁMARAS POR POSTE IMPORTA:** **una
> cámara que no ha dado NUNCA una detección no se vigila** —`vigilante_tick()` salta el silencio
> mientras `camHuboFlanco` es falso, y `camara_estado()` la salta al publicar `CAM:`—. Se escribió
> así **porque `p12` iba vacío a propósito** y un pin vacío no debía alarmar; **con `D-25` esa
> exención pierde su motivo y queda pendiente de rehacer en el firmware**. Hasta entonces: **una
> segunda cámara muerta desde el día que se monta NO sale en ninguna alarma, y la app sigue en
> `OK`** si la otra detecta. Y **tras cada corte o reinicio** la vigilancia de silencio de las dos
> queda desarmada hasta su primera detección. La alarma de contacto **pegado** sí funciona siempre
> (cuelga del nivel, `vigilante_nivel()`). **Por eso §4.bis.7 manda medir cada borne.**

| Lo que sale por Bluetooth | Cuándo | Qué mirar en la CÁMARA |
|---|---|---|
| `$ALARM …EVENTO:CAM_PEGADA,CAUSA:CAM_C_CONTACTO_FIJO,…,ACCION:NINGUNA` | el contacto lleva **20 min** cerrado **sin abrirse ni una vez** | el `Delay` de la salida (§4 Paso 4), un relé trabado… **o un vehículo parado ahí debajo** |
| `$ALARM …EVENTO:CAM_CIEGA,CAUSA:CAM_C_SIN_FLANCO,…,ACCION:NINGUNA` | **24 h de PASO ABIERTO** sin un solo cierre | óptica **tapada o desenfocada**, cámara **sin alimentación**, **cable cortado**, o la regla de intrusión desactivada |
| `$EVENT,…,ORIGEN:CAMARA,DETALLE:CAM_C_RECUPERADA` | vuelve a comportarse | la alarma **se cierra sola**: no hay que rearmar nada |

🛑 **`ACCION:NINGUNA` es el dato, no un hueco.** Esta versión **no ejecuta ninguna medida vial**: no
veta, no baja la pluma, no toca una luz. **El cruce funciona exactamente igual con las dos alarmas
puestas.** Son **avisos de mantenimiento** — 🔴 **un cruce NO se para por una cámara sucia.**

🔴 **Y NINGUNA CÁMARA PROTEGE LA PLUMA (medido el 11/09 en las dos puntas).** La pluma sale de
`escribirPines()` de `semaforo.cpp` con `(verde && !testLedsActivo) || estado == S_FALLO`, y **esa
condición no lee ninguna cámara**: **baja al acabar el verde aunque la cámara esté viendo un coche
debajo**, y **sube también con el ámbar intermitente** (Modo Ámbar, ámbar de emergencia, Degradado
en ámbar, o falta de enlace — también en un poste recién encendido que aún no habla con el otro). Lo
único que hay es el contador `$EVENT CAMARA_PLUMA VETO_HABRIA_ACTUADO_N`, que **cuenta** las bajadas
con algo debajo **después** de que ocurran. El veto es `A-1.bis` y **no está construido**. **Quien
trabaja en el poste no se pone debajo del brazo.**

🛑 **Y `CAM_PEGADA` no sabe distinguir un relé trabado de un vehículo parado veinte minutos debajo
de la pluma: dan el mismo nivel.** Por eso la causa dice `CONTACTO_FIJO` y **no «avería»**. Si va a
mirar y hay un camión parado, **la cámara está bien**; anótelo tal cual.

⏱️ **`CAM_CIEGA` cuenta 24 h de PASO ABIERTO, no 24 h de reloj**, así que **no puede saltar en una
noche tranquila**: el cronómetro sólo corre con la pluma arriba. Con el ciclo mínimo eso es del
orden de **2 dias de reloj**.

⚠️ **Lo que esto SÍ cambia en el trabajo de instalación:** al terminar de cablear, **no deje el
contacto puenteado ni la cámara mirando a una pared**. ~~Si lo hace y se va, a los 20 min el equipo
empieza a mandar alarmas que nadie pidió, y quien las reciba **saldrá a un poste que está bien**.~~
✏️ **11/09 — corregido, porque son dos casos distintos:** con el contacto **puenteado**, a los 20 min
el equipo empieza a mandar `CAM_PEGADA` que nadie pidió, y quien las reciba **saldrá a un poste que
está bien**. Con la cámara **tapada o mirando a una pared** no da detecciones: si ya había dado
alguna, acabará saliendo `CAM_CIEGA`; **si no había dado ninguna, no sale nada** —ver el recuadro de
arriba— y la cámara se queda muerta sin aviso.

🔴 **Y lo que este manual NO puede prometer todavía:** que `CAM_CIEGA` salte de verdad a las 24 h
**no está ejercido** — no es ejecutable en una sesión de banco, y hacerlo exigiría cargar una
compilación con el umbral reducido, **que no es la que va a campo**. Está comprobado en su
**forma**, no en su **tiempo**.

### 4.bis.8 Lo que este apartado NO autoriza

| | |
|---|---|
| **No autoriza a instalar en calle** | autoriza a **cablear**. La cámara real contra este equipo —zona, umbral, clasificador, duración del pulso— **sigue sin pasar banco** |
| **No sustituye al `Paso 0` del §4** | si la casilla `Trigger Alarm Output` no existe en la regla de intrusión, **el contacto no cerrará nunca** y este cableado no sirve de nada. Eso se comprueba **con la cámara delante y antes de subir a un poste** |
| **No cierra los `SIN VERIFICAR` del §7** | el `NO`/`NC` de la salida, los valores de `Delay` y el retardo de disparo se cierran con los **ensayos del §6**, no con el destornillador |

---

## 5. Dinámica de Control y Seguridad Vial

> ⛔ **11/09 — LOS PUNTOS 1 Y 2 DE ABAJO QUEDAN TACHADOS: describían otro equipo.** Las cámaras ya no
> van a `PB0` (van a `J16` p10/p12, dos por poste, `D-25`), y **una detección NO inicia el cierre del
> otro sentido**: en Automático y en Manual las cámaras **no cambian ninguna luz**, y en Inteligente
> **ni adelantan ni acortan** una fase. Lo que sí pasa, medido en el fuente, va debajo.

1. ~~**Llegada de Vehículo al Sentido 1:**~~
   * ~~La **Cámara 1** detecta el vehículo por intrusión ➔ Cierra el relé `1A`/`1B` en `PB0` del Maestro.~~
   * ~~El Semáforo Maestro registra la demanda vehicular. Si el sentido opuesto estaba en verde, inicia su cierre: **Verde ➔ Amarillo (4.0s) ➔ Despeje Todo-Rojo (`cfgDespejeSeg`) ➔ Verde Sentido 1**.~~
2. ~~**Llegada de Vehículo al Sentido 2:**~~
   * ~~La **Cámara 3** detecta el vehículo por intrusión ➔ Cierra el relé `1A`/`1B` en `PB0` del Esclavo.~~
   * ~~El Esclavo transmite la demanda al Maestro vía radio LoRa (`RS485_OUT`).~~
   * ~~El Maestro aplica la transición segura respetando siempre el **Despeje Todo-Rojo** antes de otorgar el verde al Esclavo.~~
3. **Invariable Vial:** Bajo ninguna circunstancia se omite el tiempo de Todo-Rojo de despeje ni el amarillo normativo de 4.0 segundos.

**Lo que hace de verdad una detección, 11/09 (firmware de las dos puntas):**

| | qué pasa | dónde se mide |
|---|---|---|
| **Cualquiera de las dos cámaras de un poste cierra su contacto** | el flanco entra por `demanda_solicitar()` y alimenta el vigilante — **las dos igual** | `camaras_actualizar()` recorre `CAM_J16[2]` en `botones.cpp`, idéntico en Maestro y Esclavo |
| **En el Esclavo** | esa petición viaja por radio al Maestro (`CMD_DEMANDA`) | `demanda_solicitar()` de `Esclavo/src/demanda.cpp` |
| **En Automático, Manual y cualquier modo que no sea Inteligente** | 🟢 **ninguna luz cambia.** El Maestro contesta que no la atiende | la rama `CMD_DEMANDA` de `coordinador.cpp` sólo la arma en `MODO_INTELIGENTE`; `demanda_hayLocal()` y `camara_presenciaJ16()` sólo tienen lector en `modo_inteligente.cpp` |
| **En Inteligente** | por debajo del tiempo configurado de la fase **no cambia nada**; cumplido, si en ese lado hay tráfico y enfrente nadie pide, **la fase se mantiene, con un tope**; si no, cambia como en Automático. **Nunca acorta** | `sueloMs`, `techoMs` y `TECHO_POR_SUELO` en `modo_inteligente.cpp` |
| **La pluma** | **no la toca ninguna cámara**: sube con verde y con el ámbar intermitente, baja con todo lo demás, **haya o no un coche debajo** | `escribirPines()` de `semaforo.cpp` |

> ⚠️ **Choque que NO se resuelve aquí:** `D-25` dice que las cámaras *«no tocan el ciclo»* y en
> Inteligente sí lo alargan (`D-19`). Lo aclara el responsable.

---

## 6. Protocolo de Pruebas y Validación Rápida en Taller

```text
 ┌─────────────────────────────────────────────────────────────────────────────┐
 │                         BANCO DE PRUEBAS EN TALLER                          │
 ├─────────────────────────────────────────────────────────────────────────────┤
 │                                                                             │
 │ • ENSAYO 0: LA CASILLA QUE DECIDE EL DISENO   (07/09 -- VA EL PRIMERO)      │
 │   Diez minutos, con la camara encendida y el navegador abierto.             │
 │   NO HACE FALTA NI TARJETA NI CABLE: es todo pantalla.                      │
 │                                                                             │
 │   1. Configuration -> Event -> Smart Event -> Intrusion Detection           │
 │      (si no aparece: VCA -> Smart Event -> Intrusion Detection, y si sigue  │
 │       sin aparecer, habilitarla antes en VCA -> VCA Resource)               │
 │   2. Marcar Enable y bajar hasta "Linkage Method".                          │
 │   3. MIRAR SI ESTA LA CASILLA "Trigger Alarm Output".                       │
 │                                                                             │
 │   [ 0 ] LA CASILLA "Trigger Alarm Output" EN LA REGLA DE INTRUSION:         │
 │             [ ] SI ESTA  -> marcarla. El diseno de J16 vale entero.         │
 │             [ ] NO ESTA  -> PARAR. No se cablea nada a J16: el contacto     │
 │                             no se podria accionar desde la analitica.       │
 │                             La camara NO queda inservible: sigue grabando   │
 │                             en su microSD (Trigger Recording + Record       │
 │                             Schedule tipo "Event"). Lo que muere es el bit  │
 │                             al controlador, y por donde entra entonces la   │
 │                             demanda LO DECIDE EL RESPONSABLE.               │
 │             (11/09, D-25: se mira en CADA UNA de las cuatro camaras.)       │
 │                                                                             │
 │   >>> LO QUE EL PAPEL YA DICE, PARA QUE NADIE SE SORPRENDA: la ficha del    │
 │   >>> modelo comprado ENUMERA cinco metodos de vinculacion (pag. 4) y       │
 │   >>> "trigger alarm output" NO esta entre ellos. Vaya esperando que NO.    │
 │                                                                             │
 │   [ 0.bis ] ANOTAR TAMBIEN, aunque la casilla este: la lista COMPLETA de    │
 │             metodos que ofrezca la pantalla: ___________________________    │
 │             Es el dato que contrasta la ficha contra el equipo real.        │
 │                                                                             │
 │ • ENSAYO 1: CONTINUIDAD DEL RELE                                            │
 │   - Conectar multimetro en modo continuidad en los bornes 1A y 1B.          │
 │   - Reposo: Circuito abierto (sin pito).                                    │
 │   - Al pasar una maqueta o vehiculo: cierra y vuelve a abrir.               │
 │                                                                             │
 │   >>> ESTE ENSAYO CIERRA TRES "SIN VERIFICAR". SE ANOTAN LOS TRES: <<<      │
 │                                                                             │
 │   [ 1 ] EN REPOSO EL CONTACTO ESTA:   [ ] ABIERTO -> es NO, correcto        │
 │                                       [ ] CERRADO -> PARAR Y AVISAR (ver 4) │
 │   [ 2 ] EL PULSO DURA:  ______ segundos   (medidos, no supuestos)           │
 │         Si no son ~1 s, hay que corregir el comentario de SILENCIO_MS   │
 │   [ 3 ] VALORES QUE OFRECE EL DESPLEGABLE "Delay": ____________________     │
 │         Si la lista incluye "Manual", NO SE ELIGE NUNCA (ver 4).            │
 │                                                                             │
 │ • ENSAYO 2: INMUNIDAD A PEATONES                                            │
 │   - Una persona camina o salta frente al lente.                             │
 │   - Criterio: El rele permanece ABIERTO (cero falsos disparos).             │
 │                                                                             │
 │   >>> Y ES EL QUE DECIDE SI HACE FALTA EL CLASIFICADOR: <<<                 │
 │                                                                             │
 │   [ 4 ] LA CASILLA "Detection Target" EN INTRUSION:                         │
 │             [ ] SI ESTA -> marcar Vehiculo, desmarcar Humano. Repetir.      │
 │             [ ] NO ESTA -> subir el MINIMO del Size Filter hasta que el     │
 │                            peaton deje de disparar, y anotar el valor.      │
 │             Si ni aun asi para de disparar: NO SE INSTALA. Se avisa: la     │
 │             demanda por peaton da verde a un carril sin vehiculos.          │
 │                                                                             │
 │ • ENSAYO 3: CONMUTACION EN SEMAFORO  --  SOBRE J14                          │
 │   >>> 07/09: ESTE ENSAYO YA NO ES EL DEL MONTAJE DE CAMPO. <<<              │
 │   >>> LA CAMARA VA A J16 p10/p12 (D-2 / D-3): USE EL ENSAYO 4.  <<<         │
 │   >>> Se conserva porque sirve para probar el borne J14 el dia que se le    │
 │   >>> cuelgue un fin de carrera de barrera.                                 │
 │   !!! 11/09 CONFLICTO ABIERTO: el firmware lee PB0 como CAMARA; un fin de  │
 │   !!! carrera ahi entraria como demanda.  A-2 contra firmware: decide el   │
 │   !!! responsable.  Este ensayo NO sirve para "aprobar" un fin de carrera.  │
 │   ~~- Conectar 1A/1B al pin PB0 y GND de la tarjeta STM32.~~  <-- ANULADO   │
 │   ~~- Conectar 1A/1B entre el pin PB0 y el borne de 3,3 V de J14.~~         │
 │     LA ENTRADA ES ACTIVA EN ALTO: contra GND no dispara nunca, y el         │
 │     ensayo saldria "sin deteccion" sin que nada este roto.                  │
 │   - ~~Al detectar vehiculo: El semaforo atiende la demanda y abre verde~~   │
 │     ~~tras el despeje de seguridad.~~  <-- FALSO, corregido 11/09: solo el  │
 │     Modo Inteligente mira la demanda, y ni ahi adelanta un verde.           │
 │   - CRITERIO NEGATIVO, obligatorio: con el contacto ABIERTO y nadie delante │
 │     de la camara, el equipo NO debe registrar demanda. Si la registra, el   │
 │     pin esta flotando o la polaridad no es la que se cree: se para.         │
 │                                                                             │
 └─────────────────────────────────────────────────────────────────────────────┘
```

```text
 ┌─────────────────────────────────────────────────────────────────────────────┐
 │  ENSAYO 4: J16 p10 / p12  --  ES EL ENSAYO DEL MONTAJE DE CAMPO (D-2/D-3)  │
 ├─────────────────────────────────────────────────────────────────────────────┤
 │                                                                             │
 │ 0. TAPAR EL PIN 1 DE J16 (12 V CRUDOS). Sin esto no se sigue.               │
 │ 1. Firmware nuevo YA CARGADO Y VERIFICADO en la tarjeta. Ver el orden       │
 │    asimetrico del apartado 2.1: primero firmware, despues hilo.             │
 │ 2. Ohmimetro entre el pin de senal y masa, con la tarjeta SIN energia:      │
 │    se esperan ~10 kOhm.   MEDIDO 04/09: p10 = 9,93k   p12 = 9,94k           │
 │ 3. Con energia y J16 vacio: el pin en reposo debe estar a 0 V.              │
 │    MEDIDO 04/09: los dos a 0 V.                                             │
 │ 4. Contacto seco 1A/1B entre el pin de senal y el borne de 3,3 V contiguo   │
 │    (p9 para p10, p11 para p12). SON DOS HILOS: nada a p2. Ver 4.bis.5.                     │
 │    11/09 (D-25): DOS CAMARAS POR POSTE -- camara 1 en p10/p9, camara 2 en   │
 │    p12/p11.  TODO ESTE ENSAYO SE HACE UNA VEZ POR CADA CAMARA.              │
 │ 5. ~~Al detectar vehiculo: el equipo registra demanda y abre verde tras~~   │
 │    ~~el despeje de seguridad.~~  <-- FALSO, corregido 11/09: en Auto y en   │
 │    Manual la camara no cambia ninguna luz; en Inteligente solo puede        │
 │    alargar una fase ya cumplida, con un tope.  Lo que se comprueba es el    │
 │    PIN: multimetro en p10 (y luego en p12) -> 0 V en reposo, 3,3 V con      │
 │    algo delante de ESA camara.  La app NO sirve: pone "OK" con cualquiera   │
 │    de las dos y sigue en OK aunque la otra no haya detectado nunca.         │
 │ 6. CRITERIO NEGATIVO, obligatorio y con MAS peso que en J14: en reposo,     │
 │    CON el cable puesto y SIN el, el equipo NO debe pedir paso solo.         │
 │    VERIFICADO EN BANCO el 04/09 (paso 21): no hay falsa activacion.         │
 │    (11/09: el banco del 04/09 solo ejercio p10; p12 con cable, no consta.)  │
 │                                                                             │
 └─────────────────────────────────────────────────────────────────────────────┘
```

> 🟢 **04/09 — el `ENSAYO 4` ya no es un hueco: se puede ejecutar, y sus dos primeros pasos ya se
> ejecutaron en banco.**
>
> ~~**No hay ensayo de `J16` p10/p12 en este manual, y falta escribirlo… Hasta que `M3` se haga, no
> se cablea cámara a `J16`.**~~ ⛔ **ANULADO el 04/09: `M3` está hecha (paso 20) y el ensayo está
> escrito arriba.** El texto viejo se conserva porque su razón era buena —*un ensayo que no se puede
> ejecutar no es una casilla pendiente: es la razón por la que no se cablea*— y hoy se aplica al
> revés: **ya se puede ejecutar, luego ya se puede cablear.**
>
> ⚠️ **Y el `ENSAYO 4` NO es igual que el `ENSAYO 3`, aunque se parezcan:** `PB0` lleva un antirrebote
> RC en la placa (`R64` + `C25`, ~1 ms) y ~~**`PB14`/`PB15` no llevan ninguno** — su único filtro son
> los 5 ms por software de `botones.cpp:87-93`~~ *(✏️ 11/09: falso en el netlist — `R67`+`C28` y
> `R68`+`C29` son el mismo 10 kΩ + 100 nF; sin medir en cobre)*. Por eso el criterio negativo pesa **más** ahí, y por
> eso se ejerce **con el cable puesto**, que es cuando entra el ruido: un cable de campo es una
> antena. **El paso 21 lo ejerció así y no hubo falsa activación.**
>
> 🛑 **Lo que ni el `ENSAYO 4` ni `M3` cubren:** la entrada sigue yendo **desnuda al pin del STM32**
> —sin serie, sin opto, sin clamp—. Un ensayo limpio en banco no protege de una sobretensión por el
> hilo de campo. Ver el bloque de la cabecera y la línea de la **V2** en la lista de compras.

---

## 7. 🛑 Nivel de prueba de este manual — no es un permiso para instalar

> 🔴 **07/09 — LAS CITAS DE ESTA TABLA IBAN POR NÚMERO DE LÍNEA Y **SEIS DE OCHO ESTABAN
> CADUCADAS.** Se sustituyen por el **símbolo** y por el `grep` que lo encuentra, corrido hoy. Es
> `CLAUDE.md` §4.sexies, y su aviso vale aquí literalmente: **renumerar a mano no es la cura** —
> genera números nuevos que caducan en una semana—. *(Las que sí seguían buenas: `pines.h:63`,
> `pines.h:43-46`, `Esclavo/src/main.cpp:350` y `demanda.cpp:8`.)*
>
> ```text
>   $ grep -n "define CAM_._PIN\|define BOTON[12]\|define LED_TESTIGO" \
>         01_Firmware/Maestro/include/pines.h
>     63:#define LED_TESTIGO        PB8   // -> R16 1K -> LED D5. NO es entrada de camara
>    163:#define BOTON1      PB9   // J16 p5  - Arriba / mando A
>    164:#define BOTON2      PB13  // J16 p8  - Abajo  / mando B
>    165:#define CAM_C_PIN   PB14  // J16 p10 - camara de contacto seco
>    166:#define CAM_D_PIN   PB15  // J16 p12 - camara de contacto seco
>
>   $ grep -n "pinMode(CAM" 01_Firmware/Maestro/src/botones.cpp
>    537:  pinMode(CAM_DEMANDA_PIN, INPUT);
>    538:  pinMode(CAM_C_PIN, INPUT);
>    539:  pinMode(CAM_D_PIN, INPUT);
>
>   $ grep -n "^bool botonAceptar\|^bool botonCancelar" \
>         01_Firmware/Maestro/src/botones.cpp 01_Firmware/Esclavo/src/botones.cpp
>     Maestro/src/botones.cpp:672:bool botonAceptar() { return false; }
>     Maestro/src/botones.cpp:673:bool botonCancelar(){ return false; }
>     Esclavo/src/botones.cpp:668:bool botonAceptar() { return false; }
>     Esclavo/src/botones.cpp:669:bool botonCancelar(){ return false; }
> ```

| lo que este manual afirma | nivel |
|---|---|
| La entrada de cámara es **activa en ALTO** y no se cablea contra `GND` | ✅ **MEDIDO EN EL FUENTE** — símbolo `camara_leerPin()` en `Maestro/src/botones.cpp`, que compara `digitalRead(pin) == HIGH`, y `pinMode(..., INPUT)` **pelado** en `botones_setup()` |
| `PB0`/`J14` con `R64` 10 kΩ + `C25` 100 nF es una entrada de cámara con firmware | ✅ **MEDIDO** — símbolo `CAM_DEMANDA_PIN` (`pines.h:43-46`, cita **fechada y vigente**), leído en `modo_inteligente.cpp` y en `Esclavo/src/main.cpp:350`. 🔴 **11/09: CONFLICTO ABIERTO con `A-2`**, que reserva `J14` para el fin de carrera — no se resuelve en este manual |
| 🆕 **11/09 (`D-25`) — las dos entradas de `J16` (`p10`, `p12`) hacen LO MISMO** | ✅ **MEDIDO en las dos puntas** — `CAM_J16[2] = {CAM_C_PIN, CAM_D_PIN}` recorrido por el mismo bucle de `camaras_actualizar()`: `demanda_solicitar()` + vigilante |
| 🆕 **11/09 — ninguna cámara protege la pluma; la pluma sube con verde y con `S_FALLO`** | ✅ **MEDIDO en las dos puntas** — `escribirPines()`: `(verde && !testLedsActivo) \|\| estado == S_FALLO`, sin ninguna lectura de cámara. El veto (`A-1.bis`) **no está construido** |
| 🆕 **11/09 — una cámara que nunca dio un flanco no se vigila, y `CAM:` sale `OK` sin ella** | ✅ **MEDIDO en las dos puntas** — la bandera `camHuboFlanco` salta ese pin en `vigilante_tick()` (silencio) y en `camara_estado()` (salvo que esté `PEGADA`). **La exención se escribió para un `p12` vacío; con `D-25` queda pendiente de rehacer en el firmware** |
| 🆕 **11/09 — las cámaras sólo actúan en Modo Inteligente, y sólo alargan** | ✅ **MEDIDO** — `demanda_hayLocal()` y `camara_presenciaJ16()` sólo tienen lector en `modo_inteligente.cpp`; `CMD_DEMANDA` sólo se arma en `MODO_INTELIGENTE` (`coordinador.cpp`) |
| 🆕 **11/09 — con firmware anterior a `deeeab4`, `p10` es *Aceptar* y `p12` es *Cancelar*** | ✅ **MEDIDO** — `git show e303485:01_Firmware/Maestro/src/botones.cpp` (V8.4, la de campo) |
| `PB8` es el `LED_TESTIGO`, no una entrada | ✅ **MEDIDO** — símbolo `LED_TESTIGO` (`pines.h:63`, vigente) |
| `PB9`/`PB13` son los canales `A` y `B` del mando y **no admiten cámara** | ✅ **MEDIDO** — símbolos `BOTON1`/`BOTON2` en `pines.h` y `MANDO_A`/`MANDO_B` en `mando.cpp`; el enlace lo hace `mando_registrarPulso()` desde `botones_actualizar()` |
| **`PB14`/`PB15` son `CAM_C_PIN`/`CAM_D_PIN`, entradas de cámara de DEMANDA** | ✅ **MEDIDO el 02/09, re-medido el 07/09** — símbolos `CAM_C_PIN`/`CAM_D_PIN`, con su `pinMode(..., INPUT)`. **`botonAceptar()`/`botonCancelar()` devuelven `false` siempre** en las dos puntas |
| Las separaciones de cobre de `J16` contra los 12 V | ✅ **MEDIDO** sobre el `.kicad_pcb` (`MAPEO_TARJETA_KICAD.md:576-588`) |
| Que el **firmware** de `J16` p10/p12 esté escrito | ✅ **HECHO Y MEDIDO** |
| ~~Que `R65`–`R68` estén montadas y la polaridad de `J16` sea la del netlist~~ | 🟢 **MEDIDO EN BANCO el 04/09 — `M3` CERRADA** (paso 20): `p10` **9,93 kΩ**, `p12` **9,94 kΩ** a masa, los dos a **0 V** con energía. Pull-**DOWN** real ⇒ **activo en ALTO**. ~~🔴 NO VERIFICADO, pendiente~~ |
| Que en reposo el equipo **no pida paso solo**, con y sin el cable puesto | ✅ **MEDIDO EN BANCO el 04/09** (paso 21): **cero falsas activaciones** |
| Que las cinco entradas de campo van **desnudas** al pin del STM32 y que `J16` p1 lleva **12 V crudos** | ✅ **MEDIDO EN BANCO el 04/09.** Es un **defecto de diseño de la V1**, no una advertencia de manual: se mitiga tapando el p1, y se corrige en la **V2** |
| ~~Los parámetros de la AcuSense (zona, umbral 1 s, sensibilidad 50, clasificador)~~ | ⛔ **Fila sustituida el 05/09 por el bloque de abajo, que la desglosa**: decía «LEÍDO del manual del fabricante» cuando **el manual del fabricante no se había abierto** — las cifras venían de un modelo de referencia distinto |

> ### 📷 05/09 — LA CÁMARA REAL, CLAIM A CLAIM: `DS-2CD2683G2-IZS`
>
> **Ninguna fila de aquí abajo ha tocado una cámara.** Todo lo que dice `ESCRITO` sale de un documento
> oficial de Hikvision que se puede abrir y comprobar; todo lo que dice `SIN VERIFICAR` **no lo dice
> ninguna fuente**, y no se ha rellenado.
>
> | lo que este manual afirma | nivel y fuente |
> |---|---|
> | **La cámara TIENE salida de alarma por contacto seco** — `1 input, 1 output` | 📖 **ESCRITO** — ficha `V5.5.113`, pág. 3. **Es lo que sostiene todo el cableado a `J16`** |
> | Los bornes son **`1A`+`1B`** (salida) e **`IN1`+`GND1`** (entrada), **y cada uno de esos dos es UNA PAREJA: los dos extremos de un contacto, no dos señales** | 📖 **ESCRITO** — Guía rápida `UD40284B`, **PDF pág. 9 de 40, impresa 8**. 🟢 **El 05/09 dejó de ser deducción y pasó a ser lectura**: la guía llegó a `04_Manuales/` y, por ser un **PDF de imagen** (40 páginas, **cero** caracteres de texto — medido), la página se **renderizó y se miró** |
> | **Régimen del contacto: `24 V DC` / `24 V AC`, `1 A` máx.** — sobra frente a los `3,3 V` / `330 µA` que le pide el equipo | 📖 **ESCRITO** — ficha, pág. 3 · la comparación es una **cuenta**, con sus dos entradas a la vista en §1.1.1 |
> | **Corriente MÍNIMA de conmutación y material del contacto** — importa porque `330 µA` es régimen de *carga seca* | 🔴 **SIN VERIFICAR.** La ficha no lo publica. **No bloquea** (el paso 21 funcionó), pero es pregunta para quien firme la V2 |
> | **Que la regla de intrusión pueda ENLAZARSE a la salida de alarma** | 🔴 **SIN VERIFICAR, y es el eslabón que decide el diseño — con las dos fuentes apuntando a que NO** (07/09). La ficha del modelo **enumera cinco** métodos de vinculación y **`trigger alarm output` no está** (pág. 4); el manual pone *«only supported by certain models»* (PDF 79 · impresa 67). ⛔ **Retirado el argumento *«`1 output` lo hace probable»***: `Manual Alarm` y `Automatic Alarm` cierran esa salida sin analítica (PDF 80 · impresa 68). Lo cierra el **Paso 0** en diez minutos |
> | **Que la salida se pueda accionar de ALGUNA forma** *(no por analítica)* | 📖 **ESCRITO, y no sirve para este diseño**: `Manual Alarm` es un botón del navegador y `Automatic Alarm` cierra **por horario**. Ninguna de las dos sabe que pasó un vehículo — **PDF pág. 80 · impresa 68** |
> | **Que la cámara grabe el evento SIN la casilla bloqueada** | 📖 **ESCRITO** — `Trigger Recording` **sí** está en la lista de la ficha (pág. 4) y el `Record Schedule` admite el tipo **`Event`** (PDF 48 · impresa 36). **Es lo que sobrevive si el Paso 0 sale mal** |
> | **`D-14`: que el CONTROLADOR pueda cerrar un contacto hacia la cámara** | 🔴 **NO EXISTE — MEDIDO el 07/09**, no *«sin verificar»*. Fuera de `semaforo.cpp` el firmware sólo escribe `RS485_IN_DE_RE` y `LORA_DE_RE`; `ROJO_PEATON`, `VERDE_PEATON` y `BUZZER` están **declarados y muertos** en las dos puntas. Ver §4.bis.6 |
> | **Que la salida sea `NO`/`NC` configurable** | 🔴 **SIN VERIFICAR.** `Alarm Type` sólo está documentado para la **entrada** (pág. 44); la salida sólo tiene `No.`, `Name` y `Delay` (pág. 68). **`Normally Open`/`Closed` no aparecen en las 110 páginas.** Lo cierra el **`ENSAYO 1`** |
> | **Que el pulso pueda fijarse en `1 s`** | 🔴 **SIN VERIFICAR.** El parámetro `Delay` existe y está definido (pág. 68); **sus valores seleccionables no se publican**. Lo cierra el **`ENSAYO 1`** |
> | **Retardo entre detección y cierre del contacto** | 🔴 **SIN VERIFICAR.** Lo único acotado: **no es menor que el `Threshold`** configurado, por definición del propio parámetro (pág. 49) |
> | **`Detección de Intrusión` es la analítica correcta** — es la única cuya regla mide *permanencia* (`Threshold` = *«time of the object loitering»*) | 📖 **ESCRITO** — manual, págs. 48-49. **El razonamiento completo, con las cinco analíticas comparadas, en §1.1.4** |
> | **El clasificador `☑ Vehículo` sobre la Detección de Intrusión** | 🔴 **SIN VERIFICAR, y las dos fuentes oficiales discrepan.** El manual lista `Detection Target` en *Line Crossing*, *Region Entrance* y *Region Exiting* **y no en `Set Intrusion Detection`**; la ficha (pág. 4) dice que la Perimeter Protection *«supports human and vehicle targets classification»*. Lo cierra el **`ENSAYO 2`** |
> | Umbral **`1 s`** y sensibilidad **`50`** | 🔴 **SIN VERIFICAR.** Son los valores de partida **de este proyecto**, no una recomendación del fabricante. Se afinan con los ensayos |
> | **Alimentación: `12 V DC ± 25 %` (`9`–`15 V`), `1,08 A`, máx. `13 W`** · PoE `802.3at` **Clase 4**, máx. `15 W` | 📖 **ESCRITO** — ficha, pág. 4. **El rango cubre entero el vaivén de una batería de plomo de 12 V** |
> | ~~**`26 Ah/día` por poste sólo de cámara**~~ → **11/09 (`D-25`): `52 Ah/día` por poste, las DOS cámaras** | 🧮 **CUENTA**, con sus entradas escritas al lado (§1.1.2: `2 × 13 W × 24 h / 12 V`). **Es un dato para quien dimensione la energía, no una decisión de este manual** |
> | Óptica **`2,8`–`12 mm`**, FOV horizontal **`108°`–`30°`**, DORI Detectar **`97`–`290 m`**, 8 MP | 📖 **ESCRITO** — ficha, pág. 2 |
> | **No hay luz blanca que desactivar: la luz de apoyo es IR `850 nm` y sólo IR** | 📖 **ESCRITO** — ficha, pág. 2. **Retira un paso del §4 y elimina el riesgo de deslumbrar a un conductor** |
> | **Rutas de menú del §4** *(SADP, `Lens Parameters Adjustment`, `External Device`, `Smart Event → Intrusion Detection`, `Basic Event → Alarm Output`, `VCA Resource`)* | 📖 **ESCRITO** — manual `UD28967B-C` v5.7.20, con la página al lado de cada paso. **Ninguna se ha ejecutado contra una cámara** |
> | La **traducción al español** de cada rótulo de menú | 🔴 **SIN VERIFICAR.** La interfaz tiene español (ficha, pág. 4), pero los rótulos exactos no se han visto. **Manda el inglés del manual** |
> | El **soporte de fijación** al poste, para `1.385 g` y `30,8 cm` | 🔴 **SIN VERIFICAR.** No está especificado en ningún documento de este proyecto |
>
> 🛑 **Y el aviso que va con la tabla: `ESCRITO` NO ES `MEDIDO`.** Este manual describe una cámara que
> **nunca ha estado delante de esta tarjeta**. Lo `MEDIDO` de este documento es la **tarjeta** —`M3`,
> el paso 21, el fuente del firmware—; **de la cámara no hay ni una sola medida propia todavía.**

> 🟢 **Lo que SÍ ha pasado banco de este manual, y sólo eso:** la medida `M3` (paso 20) y el ensayo
> de falsa activación (paso 21). **Con eso, cablear cámara a `J16` deja de estar bloqueado.**
>
> 🛑 **Lo que sigue SIN pasar banco:** la cámara AcuSense **real** contra este equipo —zona, umbral,
> clasificador, pulso de 1 s—, y el ciclo completo con demanda por `J16`. **Este manual no autoriza a
> instalar**: autoriza a **cablear** lo que `M3` desbloqueó, con el pin de 12 V tapado y con el
> firmware nuevo ya cargado en la tarjeta.
>
> La única forma correcta de verificar el firmware es `01_Firmware/compuerta.py`, y un verde suyo
> **no es un permiso**: dice que los modelos y los arneses de PC no encuentran nada, no que el
> firmware funcione en la tarjeta (`CLAUDE.md` §3).

---

## 8. 📋 LO PRIMERO QUE HAY QUE HACER ~~CUANDO LLEGUE~~ **AHORA QUE YA ESTÁ** LA CÁMARA

> 🟢 **05/09: LA CÁMARA YA ESTÁ AQUÍ.** Este apartado se escribió esperándola; hoy es una lista de
> tareas pendientes, no una previsión.
>
> 🔑 **PERO EL PASO CERO DE TODOS ES ENTRAR EN ELLA**, y ahí es donde el trabajo se paró: *«no
> encuentro ni la IP»*. **Si todavía no se ha visto la pantalla de login, nada de esta tabla se puede
> hacer** — empezar por el bloque **🔑 EMPIEZA AQUÍ: ENTRAR EN LA CÁMARA** de la cabecera
> *(descubrimiento con `SADP.exe`, que ya está en el repositorio; activación, porque la cámara sale
> **inactiva**; IP y acceso web)*. Las cuatro comprobaciones de abajo vienen después.

**Cuatro comprobaciones, en este orden, antes de dibujar una sola zona o cablear un solo hilo.**
Cada una cierra un `SIN VERIFICAR` de la tabla del §7, y **las cuatro juntas no llevan media hora**.
🎯 **11/09 (`D-25`): son cuatro cámaras, y las cuatro comprobaciones se hacen y se anotan EN CADA
UNA** — cada unidad trae su propia configuración y su propio relé.

| # | qué se comprueba | dónde | si sale mal |
|---|---|---|---|
| **1** | **¿Existe la casilla `Trigger Alarm Output` en la regla de intrusión?** 🔴 **Vaya esperando que NO: la ficha del modelo la omite de su lista de vinculaciones (pág. 4)** | **§4 Paso 0** | 🛑 **PARAR.** El contacto seco no se puede accionar desde la analítica → el camino de `J16` no sirve y hay que replantear por dónde entra la demanda. **No lo decide este manual.** ✅ Lo que **sí** sigue vivo: la grabación por analítica en la microSD. ❌ Lo que **no** es relevo: `D-14`, sin construir |
| **1.bis** | **¿Qué zona hay que dibujar: la vía de aproximación o el barrido de la pluma?** | **§4 Paso 3**, bloque 🛑 | 🛑 **PREGÚNTELO ANTES DE ENCUADRAR.** `D-13` dice pluma, el firmware ejerce demanda, y **una cámara sólo lleva una regla**. Las dos cosas están vivas y **no lo decide el técnico** |
| **2** | **¿Está la salida ABIERTA en reposo?** *(o sea, ¿es `NO`?)* | **`ENSAYO 1`**, §6 | 🛑 **PARAR.** Con el contacto cerrado en reposo el pin nace ALTO y **no habrá flanco** hasta que el relé abra y cierre. Ver §4 Paso 4 |
| **3** | **¿Cuánto dura el pulso, y qué valores de `Delay` ofrece?** | **`ENSAYO 1`**, §6 | ⚠️ Si sólo hay valores largos, **funciona con menos finura**: se anota y se sigue. **`Manual` no se elige jamás** |
| **4** | **¿Existe `Detection Target`, y aguanta el `ENSAYO 2` de peatones?** | **`ENSAYO 2`**, §6 | ⚠️ Sin clasificador se sube el mínimo del **`Size Filter`**. Si aun así un peatón pide paso, **se avisa**: sería dar verde a un carril sin vehículos |

> 🛑 **Y lo que NO cambia por mucho que las cuatro salgan bien:** antes de llevar un hilo a `J16` hay
> que **tapar el pin 1**, que lleva **12 V crudos**, y tener el **firmware nuevo ya cargado y
> verificado** en la tarjeta. Ver el bloque 🛑 de la cabecera y §2.1. **Un commit no protege de un
> destornillador.**

---
*Manual técnico oficial de integración y configuración de Cámaras IA para Semáforos Móviles V9.0. Polaridad corregida el 31/08 (`N-105`): el contacto seco cierra contra 3,3 V, no contra `GND`. **Confirmada en cobre el 04/09 con el cierre de `M3`**, que además levanta el bloqueo para cablear `J16` p10/p12 — con el pin 1 de esa misma bornera **tapado**. **Cámara real incorporada el 05/09: `DS-2CD2683G2-IZS`, con su ficha, su manual de usuario y su guía rápida citados página a página — y con lo que esas tres fuentes NO dicen marcado `SIN VERIFICAR`, no rellenado.** **11/09: `D-25`, cuatro cámaras, dos por poste en `J16` p10/p12, las dos iguales; ninguna protege la pluma.***
