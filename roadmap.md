# Roadmap — Controladora de Semaforos Moviles de 3 Estados (V9.1)

**Este fichero lleva SOLO lo pendiente y lo que falta validar.** El 07/09/2026 se le sacaron 4.631
lineas de cosas ya cerradas, y el **12/09/2026 se le sacaron sus apartados §3, §6, §7, §8, §9 y
§10** —el «porque» entero—: todo eso esta **integro, literal y con su evidencia** en
[`05_Funcional/historico/roadmap_hist.md`](05_Funcional/historico/roadmap_hist.md), que lleva indice por `N-x` y por
apartado. **No se borro nada** — las dos cuentas estan publicadas al final de aquel fichero.

> 🔴 **EL TOPE SON 600 LINEAS. `wc -l roadmap.md` DA LA CUENTA DE VERDAD** —cualquier cifra escrita
> aqui envejece sola (`CLAUDE.md` §14)—. **Hoy se cumple, y aun asi hay que medirlo cada vez, porque
> la ultima particion NO bastó**: el 07/09 este fichero se partio y quedo en **1.093 lineas**
> (`0f4662b`, *«1093 vivas + 4817 de historico»*) — **por encima del tope ya entonces** — y en cinco
> dias volvia a estar en **2.695**. Lo que faltaba no era la particion: eran las dos reglas de abajo.
>
> **Por que existe el tope, que importa mas que el numero.** Un fichero que no se puede leer entero
> **no lo lee nadie entero**: se lee a trozos, y entonces **cada lector deriva su propia version**.
> El 12/09 paso varias veces en una sola sesion —dos agentes trabajando sobre dos lecturas
> distintas del mismo apartado—, que es exactamente el bucle que `DECISIONES.md` vino a cerrar.
>
> **Las dos reglas que lo mantienen abajo con el tiempo. Sin ellas el numero vuelve en una semana:**
>
> 1. **Aqui solo vive lo PENDIENTE.** El porque —como se descubrio, que se midio, que se refuto—
>    vive en [`05_Funcional/historico/roadmap_hist.md`](05_Funcional/historico/roadmap_hist.md),
>    y una fila de aqui **apunta** a el, no lo copia.
> 2. 🔴 **UNA FILA QUE SE CIERRA SALE DE AQUI.** Se tacha con el commit o el `grep` que lo prueba, y
>    **en la pasada siguiente se muda al historico con ese cierre dentro**. Dejarla tachada para
>    siempre es como se llego a 2.695 lineas: **ningun commit retiraba nunca un apartado**, solo se
>    anadia. Lo tachado no se borra — **se muda**.
>
> ⚠️ **Y si el tope aprieta, lo que NO se hace es enterrar un pendiente para que quepa.** Se para,
> se dice la cifra y se pregunta: **perder una fila abierta cuesta mucho mas que pasarse de 600.**

> 🔴 **TODA CITA A `§3`, `§6`, `§7`, `§8`, `§9` O `§10` DE ESTE FICHERO APUNTA A
> [`05_Funcional/historico/roadmap_hist.md`](05_Funcional/historico/roadmap_hist.md)**, donde esos seis
> apartados viven integros desde el 12/09
> bajo la cabecera **«MUDANZA DEL 12/09/2026»**, con su propio indice por apartado y por `N-x`. Las
> citas a `§0`, `§1`, `§2`, `§4` y `§5` son de aqui. **Los numeros NO se renumeraron a proposito**
> (`CLAUDE.md` §5): el firmware, cuatro packs, `ARQUITECTURA.map` y once documentos citan
> `roadmap §3.16`, `§3.10.bis`, `§3.4.bis` y `§3.8` **desde fuera**, y renumerar los deja cojos a
> todos de golpe.

> **Como se lee.** Este fichero es **la lista de lo que falta**, ordenada por **quien lo
> desbloquea**: §0 la tiene entera por grupos —(1) teclado, (2) decision del responsable, (3)
> cobre/banco/campo— y §1, §2, §4 y §5 desarrollan cada grupo. **El porque esta en el historico.**
> Si algo no esta aqui, o esta hecho, o esta en `05_Funcional/historico/roadmap_hist.md`.

> ⚠️ **Las citas a `CLAUDE.md` con `§x.bis`, `§x.ter`, `§x.quater` o `§4.sexies` son de ANTES del 07/09**,
> cuando ese fichero se compacto (`74a1054`) y se renumero. Se deja la cita como estaba —es cronica— y la
> regla se busca **por su titulo**, no por el numero.

## Que manda sobre este fichero, y en que orden

| | |
|---|---|
| **la decision vigente** | la SPEC (`05_Funcional/SPEC_0..SPEC_8`) — **manda la SPEC; [`DECISIONES.md`](DECISIONES.md) es su indice; derogar es borrar la fila** |
| **el hardware medido** | `05_Funcional/17_Arquitectura_28-08_y_Decisiones_Abiertas.md` — gana a `DECISIONES.md` y a `CLAUDE.md` en cobre, pines y conectores |
| **las reglas permanentes** | [`CLAUDE.md`](CLAUDE.md) |
| **el estado de hoy** | [`ESTADO.md`](ESTADO.md) |
| **el porque de lo cerrado** | [`05_Funcional/historico/roadmap_hist.md`](05_Funcional/historico/roadmap_hist.md) |

## Las cifras NO se copian aqui

**La instalacion certificada es `V8.4`, commit `e303485` (31/07/2026).** 🔴 **Pero el 10/09 un Maestro
con firmware V9 `SIN_BANCO` corrio en campo, en El Sisga**: cargado el paquete del 08/09 (`7ff7d12`) y
probado despues el del 10/09 (`b354fe9`) (§3.16). En el
repositorio, `V9.1` en `main` **desde el 14/09** — la V9.0 fue el paquete de revision del 12/09. **Lo que cambia en la carretera son DOS cosas**: el verde del modo sin radio pasa de **30 s a 3 minutos** (`1d3ab02`, 13/09) y **la barrera ya no baja en el mismo instante del rojo** (`363e375`, 14/09): baja con retardo, para dar tiempo a los vehiculos y no golpearlos.

Compuerta, banco, flash y arneses: **la ultima acta de `evidencia/`**. No se transcriben a este
fichero **a proposito**. Este mismo apartado publicaba `15 PASS` y `39 packs` durante una semana
mientras el acta decia otra cosa; una cifra copiada a mano envejece por su cuenta y llega a un
encargo con autoridad de dato. **Se abre el acta.**

```
python 01_Firmware/compuerta.py        # las 20 filas. Es lo que AUTORIZA un commit
python 01_Firmware/Simulaciones/banco/correr.py   # solo los packs. Sirve para iterar, NO autoriza
```

> 🛑 **Verde no es entregable, y hay contraejemplo con fecha.** Los tres defectos que pararon el
> banco del 03-04/09 **pasaron las 20 comprobaciones sin despeinarlas**, porque ninguno es una
> propiedad del fuente: un chip que se calienta, un permiso de Android y una resistencia del cobre.

---

## 0. Lo que esta abierto de verdad, en cinco lineas

### ORDEN DE TRABAJO («construir todo, que no falte nada»): esto manda; el detalle, en cada `N-x`

| # | que | quien / donde | estado |
|---|---|---|---|
| 1 | Banco de Marco con `entregas/Paquete_Semaforos_2026-11-10_6ca489f_SIN_BANCO.zip` (zip `ae1c523f...`, APK `87211871...`) y la hoja del 10/11: Parte 9 (pruebas 26-34, `N-187`), Parte 10 (35-40, barrera arriba en reposo) y las de antes | Marco, 10/11 | esperando |
| 2 | Leer sus registros enteros y contando (`CLAUDE.md` §7): `RTT:` con LoRa, cobertura en metros, tiempo real de subida y bajada del brazo (con >10 s ningun retardo protege), camaras (NA/NC y veto), C5 (la camara no frena una bajada ya ordenada); y lo que las cintas aun deben (`D1`..`D8` de la mudanza del 09/11: `N-172`, `D-40`, `D-38`, camaras con peatones, centralita, `N-116`) | orquestador | tras 1 |
| 3 | Ajustar los rangos de `N-187` desde la app, sin firmware; si el brazo pide otro rango, cambia solo el rango y su prueba (`D-163`) | orquestador | tras 2 |
| 4 | Quitar `SIN_BANCO` solo con acta de banco (`evidencia/<fecha>_campo_<tema>.txt` con el hash cargado) | quien lo probo | tras 2 |
| 5 | El Sisga tras banco: respaldo md5 y carga; en la MISMA visita se corrige `Camaras_Sisga_4x.html` (pluma con coche debajo: cierto con `7ff7d12`, falso desde `d3606be`), se mide el fin de semana (`VERSION`, `CONSULTA_NOCHE`, cinta de un sabado) y se reenvia la instalacion (`-e all` la devuelve a fabrica) | Marco y orquestador | tras 4 |
| 6 | Version siguiente, abajo | -- | no bloquea |

Cerrado el 09/11: `N-186` (paquete `db07446`, a `RETIRADOS/`) y `N-187` construido sin banco (`3ea00d4`, `c10e60f`;
arquitecto y QA APTO CON CONDICIONES, compuerta 16/17 solo `D-22`). `RTT:` con LoRa medido: mediana 752 ms, max 952
(`evidencia/091120261729`), ida < 500 ms de `D-34`. La cronica de `N-173`..`N-186` (tablas A-I, V1..V10, M1..MF,
N1..NF, G0..G-RF, S..P, U, `N-182`) esta literal en
[`roadmap_hist_2026-11-09.md`](05_Funcional/historico/roadmap_hist_2026-11-09.md).

Decidido el 09/11 con simulacion (dos modelos + arbitro): la pluma sube con el verde, baja solo tras el rojo (3..5 s),
arriba en ambar intermitente; nada de eso es parametro salvo el retardo. Medir el recorrido del brazo manda sobre todo.

Regla desde el 09/11: cada cambio lleva su spec en el MISMO commit y un grep de lo nuevo en SPEC y DECISIONES.

### Version siguiente (fila 6; ninguna bloquea el banco)

En marcha (10/11, workflow `wf_c829c8d1-7fe`): medir `D-167` (retiro del Alcance), `D-166` (sitio del segundo bit de
veto) y `D-158` (que clave admite la libreria BT del ESP32), a scratchpad; y partir `botones.cpp` (641 en las dos
puntas), `Maestro/src/semaforo.cpp` (534) y `bluetooth.cpp` (1.581 y 1.665), un commit cada una, con sus rutas
(`CLAUDE.md` §5) y los comentarios de razon recortados en `N-187`. Despues, sobre los ficheros partidos y en este
orden: construir `D-166` (veto por camara), `D-167` (retirar Alcance) y `D-158` (clave BT comun; telefonos de Marco
a medir en banco), cada uno con su spec en el mismo commit.

- Hecho el 10/11: `D-98` tanda 4.a (`6b9f058`) y la frase de `js/luces.js`. Queda la tanda ultima (modales y PIN),
  con su condicion en [`ROADMAP_APP.md`](05_Funcional/App_Semaforo/ROADMAP_APP.md), y Pasos Peatonales alli.
- De arquitecto y QA de `N-187`: prueba de la pegada y del aviso de 90 s tras re-siembra; `camara_02_j16` no ve el
  `!= nc`. Hecho el 10/11: el Esclavo y los cortes de luz en PC (bloque I, 193/193, `2049501`).
- Decidido por el responsable el 10/11 (sin construir; DECISIONES al integrar `D-167`): principio «ninguna averia de
  camara o barrera deja el paso inoperante, la barrera es complemento»; veto por camara (`D-166`), inhibida = ni veto
  ni demanda; parametro `MALA1`/`MALA2` (BAJA|ARRIBA), fabrica ARRIBA/ARRIBA; Q2..Q7 de la propuesta aceptados
  (scratchpad `propuesta_camaras_degradadas.md`); SIN ENERGIA = barrera ARRIBA: central «sin senal = subir» y J15
  invertida, contra `D-164`/SFTY-28: en revision de arquitectura antes de construir (`CLAUDE.md` §11.7); firmware y
  central cambian en la MISMA visita. En construccion: `D-167` (retirar Alcance, worktree).
- Barrera con brazo lento (responsable, 10/11): cuando bajar y subir (con el verde, con el ambar, tras el rojo, N s
  antes) se decide con el tiempo del brazo que mide Marco (hoja, prueba 4) y una simulacion de dos modelos y arbitro
  con 10, 30 y 60 s. «Sin barrera» ya existe: fuera de servicio, arriba en todo modo (`D-96`). Recomendado no crear
  «abajo fija»: el rojo fijo ya la baja.
- Hueco medido (`N-182`): tras un corte con la barrera fuera, la pluma sube en la siguiente escritura de la luz, no al
  arrancar. Sin medir: el telefono (scroll a 360x640, 320 px, toques de 48 px; CDP 9333).
- Sin teclado: `G-RF`, dos parejas en banco (mismo canal y distintos, `D-118`) antes del filtro de pareja (SFTY-27).
- `N-166`, colision de nombre `A-15` (sin medir si sigue).
- Gobernanza, con tope semanal: dictamen de SPEC y manuales (`6f38ba4`, fase F); grafo fuera del repo (traerlo a
  `entregas/` o rehacerlo); repo ordenado (F1/F2); tras banco, lo duplicado Maestro/Esclavo a `lib/`.
- Bloque U (U-S, U-B; lo cita SPEC_4 §9.9): hecho en `N-182`; la tabla, literal en la mudanza del 09/11.

> ## 🔒 DECIDIDO Y CERRADO EL 11/09 — NO SE REABRE DESDE UN DOCUMENTO
>
> | | lo que fija | lo que deroga |
> |---|---|---|
> | **`D-25`** | **Cuatro camaras, dos por poste**: camara 1 entre `J16` p9 (3,3 V) y p10 (`CAM_C_PIN`), camara 2 entre `J16` p11 (3,3 V) y p12 (`CAM_D_PIN`), cada una por el contacto seco `1A`/`1B` de su salida de alarma. **Talanquera en `J15`**: p1 = 12 V, p2 = drenador de `Q10` (**no es masa**), a la bobina de un rele cuyo contacto va a `OPEN` de la centralita | de `D-13` **solo** «una camara por poste» y «`p12` vacio a proposito»; «las 4 camaras se revocaron el 28/08» (`ESTADO.md` C1) |
> | **`D-26`** | **La hora la manda el ESP32 de cada poste**: el `SET_RTC` del telefono sin PIN en el ESP32; cada STM32 se siembra desde el `DS3231` de su ESP32 **cada ~5 min** (`CMD:HORA_ESP32`); el Esclavo toma la hora del Maestro con radio y la de su ESP32 sin radio; en Degradado un salto mayor que el margen del cruce **pasa por rojo**; **dos alarmas**: perdida de radio, y enlace ESP32-STM32 caido o siembra rechazada | el numero de `A-15` (una hora -> ~5 min); la regla del Esclavo «acepta su ESP32 si la radio lleva 2 h sin sembrar» |
> | **`D-27`** | **Las cuatro camaras estan compradas** (modelo `D-10`, | `A-2` en «el fin de carrera va a |
> |  | `DS-2CD2683G2-IZS`); | `J14`» |
> |  | **`J14` LIBRE, sin cablear** —el fin de carrera no se instala en este | (sin efecto en este despliegue); los |
> |  | despliegue; mientras el firmware lea `PB0` como demanda, en `J14` no se | valores de configuracion de `D-13` |
> |  | conecta nada—; | (filtro, umbral, sensibilidad) en lo que |
> |  | **la configuracion de cada camara es la del manual del modelo** | choquen con el manual del modelo |
> |  | (`05_Funcional/historico/MANUAL_CONFIGURACION_CAMARAS_IA.md` y la ficha) y el |  |
> |  | Manual 9 se alinea con el; |  |
> |  | **talanquera por rele y centralita como en la guia del Sisga**, y la lista de |  |
> |  | compras se ajusta; los `.docx`, pendientes |  |
>
> **Si un manual, una guia o un comentario del fuente dice otra cosa, ESE DOCUMENTO esta caducado: se corrige hacia
> la SPEC, nunca al reves: manda la SPEC; DECISIONES.md es indice; derogar es borrar. Cambiar una de estas tres exige
> que el responsable cambie la SPEC y su fila del indice en el mismo commit.**
>
> ⚠️ *Medido al alinear `D-27` (11/09), para que nadie lo lea de mas:* el manual del modelo **fija
> el objetivo** (☑ Vehiculo · ☐ Humano, si la casilla existe) y **no fija** `Threshold`,
> `Sensitivity` ni el horario de la salida, que siguen en los de `D-13` (minimo, alta, 24x7). Su
> zona —«la zona donde el vehiculo espera»— es anterior a `D-13` y **no se aplica**: `D-27` no
> deroga la zona, que sigue siendo **el barrido de la pluma**. Tabla valor a valor: Manual 9 §4
> Paso 3. Lo que queda para el responsable, en la fila 2.1 de abajo.

### Lo pendiente de implementacion medido el 11/09

Sacado literal a [`roadmap_hist_2026-10-14.md`](05_Funcional/historico/roadmap_hist_2026-10-14.md) (260 lineas).
El estado vigente, en `ESTADO.md` y en las tablas de arriba.

## 1. Lo que necesita una TARJETA — no lo destraba nadie escribiendo

**Va primero porque es lo unico que no se puede sustituir.** `CLAUDE.md` §2.bis: *la pregunta antes
de escribir cualquier cosa es si acerca una tarjeta cargada o la sustituye.*

| | que | detalle |
|---|---|---|
| **T-1** | 🛑 **`N-116`: recorrer la escalera de 4 peldanos sobre la tarjeta Maestro** *(11/09: la de la **sesion 1**; la `179DB0` es otra placa y funciona, `N-126`)*. El primero —**desenchufar los cinco conectores y remedir el riel**— es **gratis, son cinco minutos, y puede cerrar el caso solo** | §6.1 |
| **T-2** | 🔴 **Subir `SFTY6_SILENCIO_MS = 25000UL` sobre `e303485`.** Solo esa constante, sobre la V8.4 que ya esta probada en la calle | §6.3 · **es lo unico que llega al conductor esta semana**, y no depende de la V9.0 · ⚠️ *11/09: los 25 s **si** tocaron una calle el 10/09, pero dentro de `7ff7d12` (V9 `SIN_BANCO`, Sisga); la V8.4 sigue en 12 s y en git no hay ningun `e303485` + solo esa constante* |
| **T-3** | 🔴 **La sesion de banco de** ~~**los siete POR VALIDAR** — `N-142`, `N-147`, `N-150`, `N-151`, `N-152`, `N-153` y `N-157`. **Los siete tocan el ambar, el Modo Manual o la camara**~~ **lo POR VALIDAR de §5**, que el 11/09 son bastantes mas de siete y ya no se cuentan desde la cinta del 05/09 sino desde la del 10/09 | §5 |
| **T-6** | 🟠 **Las** ~~21~~ **`SIN VERIFICAR` de `05_Funcional/17_...md`.** Es la lista de lo que el proyecto declara sin haber medido *(11/09: la cifra se retira — era de memoria y no se reproduce; `grep -c "SIN VERIFICAR"` da lineas, no items)* | ese fichero, no este |
| **T-7** | 🟠 **`N-110` B2: que el teclado del PIN no acepte pulsaciones con el modal cerrado.** ~~No se comprueba leyendo — hace falta ejercer el DOM~~ — 🟢 **construido (`b033f0b`, `tecladoPinAbierto()` en los cuatro handlers de `app.js`) y ejercido en el DOM** por `test_dom_execution.js` (teclea el PIN con el modal cerrado y exige que la sesion siga sin autorizar). **Solo falta el telefono** | §6.6 |

> 🔴 **Y la carga va ANTES que el cable, no en el mismo commit** (`CLAUDE.md` §9.bis). Un commit no
> protege de un destornillador: **el firmware nuevo tiene que estar DENTRO de la tarjeta antes de
> que nadie enchufe nada en `J16`.** Se exige la carga verificada, no el merge.

---

## 2. Lo que necesita una DECISION del responsable

~~**Cada una tiene ya su fila en [`DECISIONES.md`](DECISIONES.md), que es donde manda.**~~ 🔴 **FALSO
para siete de las diez, medido el 11/09:** solo `D-a` (`D-19`), `D-b` (`A-1.bis`) y `D-j` tienen fila;
`grep -c` sobre `DECISIONES.md` de `N-113`, `N-129`, `N-132`, `MATRIC`, «dos pantallas» y
«renumeraci» da **0**, y `D-d` no tiene fila propia (`J9` sale una vez, dentro de `D-14`). **Mientras
no la tengan, lo que manda de esas siete es lo que dice este fichero, y eso es justo lo que
`DECISIONES.md` vino a evitar.** Aqui van **como punteros, no repetidas** — repetirlas es exactamente
el problema que aquel fichero vino a resolver.

> ~~🔴 **CONFLICTO ABIERTO, para el responsable — `A-2` contra el codigo en `J14`/`PB0`** (tabla de §0,
> fila 2.1): `A-2` manda el fin de carrera a `J14`/`PB0`, y el firmware lee ese pin como camara de
> demanda (`CAM_DEMANDA_PIN`) en las dos puntas. **En `J14` no se cablea nada hasta decidirlo.**~~
> 🟢 **Decidido el 11/09 (`D-27`): `J14` libre y sin cablear; el fin de carrera no se instala.**

| | que hay que decidir | donde vive |
|---|---|---|
| **D-a** | 🟡 **La firma del funcional sobre el manual del «doble»** — `TECHO_POR_SUELO = 2` esta en el firmware y **ya salio en el paquete**; el recuadro de firma de `05_Funcional/historico/1_Manual_Usuario.md` esta **vacio campo por campo** y `evidencia/` no tiene ningun documento firmado | `D-19`, su condicion |
| **D-b** | 🔴 **`A-1.bis`: ¿se deroga SFTY-28 para el veto de la pluma?** Bloquea la fase 2 de `D-13`; la fase 1 ya esta construida | `A-1.bis` · §6.8 |
| **D-d** | 🔴 **`J9` `VERDE_PEATON` · `J11` `ROJO_PEATON` · `J13` `BUZZER`: tres canales de potencia FABRICADOS, con su opto y su MOSFET, y cero firmware.** `grep digitalWrite` sobre los tres da **0**. **16 B de flash cada uno**, medidos por desensamblado. ¿Se gasta uno en una cabeza peatonal o en un zumbador, o se declaran **no poblados**? | `CLAUDE.md` §6 |
| **D-e** | 🟠 **La renumeracion de camaras.** `17_` §1.7 **ya decidio `C` y `D`** y lo blindo. ~~**`README.md` publica hoy `p10 = Camara 2` / `p12 = Camara 1`**, que es lo contrario.~~ *(corregido el 11/09: `README.md` dice ya `CAM_C`/`CAM_D`; queda un tercer reparto en `05_Funcional/README.md`, *«p10 = poste 1, p12 = poste 2»*)* **La spec gana** — lo que falta es que la confirmes o la derogues, y si se deroga, que pasa con la Camara 3. ⚠️ *11/09: con `D-25` hay dos por poste, `p10` y `p12` en cada uno* | §3.4 |
| **D-f** | 🟠 **`N-129`: el rotulo Bluetooth.** Un modulo virgen anuncia `SEM-SIN-MATRICULA` y **las dos puntas se llaman igual**; el nombre bueno solo entra en la SIGUIENTE arrancada. ¿Paso de puesta en marcha, o se cambia el momento del rotulado? ~~**Nunca se ha visto en un telefono**~~ *(11/09: el rotulo APRENDIDO si se vio, `SEM-179DB0-M`, en el banco del 04/09 —`N-126`—; lo que no consta visto es el caso de dos modulos virgenes llamandose igual)* | `MATRIC` |
| **D-g** | 🟠 **`N-132`: no hay puente H y sale UNA sola linea de control a la pluma.** El `L298N` decidido necesita dos. ⚠️ *11/09, pregunta y no medida: con `D-25` la talanquera va por rele a la entrada `OPEN` de una centralita comercial, que mueve su propio motor — ¿sigue haciendo falta el `L298N`? No se contesta aqui* | `N-132` · censado sobre el `.kicad_pcb` |
| **D-h** | 🟠 **`N-113`: el aviso remoto es COSTE RECURRENTE** —SIM o WiFi en el cruce—, no una linea de firmware | §6.5 |
| **D-i** | 🟠 **`MATRIC`: matriculacion por ID de Bluetooth.** `RF_Packet` son 4 bytes `{msgID, command, param, crc}` **sin campo de direccion**, y el CRC cubre 3: meterle direccionamiento **cambia el contrato de la radio en las dos puntas** | `MATRIC` |
| **D-j** | 🟠 **`A-0`,** ~~`A-7`,~~ **`A-8`, `A-10`, `A-4`,** ~~`A-13`~~ — grabacion de las microSD, ~~el `Delay` real del rele,~~ los dos `Arming Schedule`, el LED `D21` de `VERDE2`, que pasa con `MENU`~~, donde va el campo `CAM:`~~ *(11/09: `A-7` se cerro el 08/09 sin medir y `A-13` esta resuelta y construida, `e3a21ec`: las dos tachadas en el indice de `DECISIONES.md`)* | `DECISIONES.md` |

> ⚠️ **`A-2` NO va en esta lista: la cerraste tu el 05/09.** `J16` p5/p8 son el mando con su codigo
> intacto (`D-1`) ~~y el fin de carrera va a `J14`/`PB0`~~ *(11/09, `D-27`: `J14` queda LIBRE y sin
> cablear; el fin de carrera no se instala en este despliegue)*. ~~**`DECISIONES.md` la sigue listando abierta
> con urgencia media, y su fila «Filas que chocan nº 2» (hoy en `05_Funcional/historico/DECISIONES_hist.md`) describe
> un conflicto que ya no existe**
> —`A-11` se resolvio por app en `D-18`—. Ver §3.2.~~ *(11/09: el indice ya la da cerrada, y «Filas
> que chocan nº 2» se tacho hoy como resuelta.)* ~~🔴 **Pero su mitad de `J14` choca con el CODIGO**
> —el recuadro de arriba y la fila 2.1 de §0—: **eso si es una decision tuya, y abierta.**~~
> 🟢 *(11/09: la tomaste — `D-27`, `J14` libre; su mitad de `J14` queda sin efecto en este despliegue.)*

---

## 4. Lo que necesita una COMPRA o un SOLDADOR

| | que | cuando |
|---|---|---|
| **C-1** | 🟢 **`2K2` en serie en las 5 entradas de campo, al soldar la placa nueva** (`N-120`) | **es el momento y no vuelve.** Si se suelda igual, la placa nueva nace con la misma averia dentro |
| **C-2** | 🟢 **No poblar los 12 V de `J16` p1** — y mientras exista, **taparlo es obligatorio en cada equipo que se monte** (`D-4`) | al montar |
| **C-3** | 🔴 **`A5`: el reductor** ~~`LM2596`~~ **DC-DC conmutado desde la bateria** (12 -> 5 V, >= 1 A) para el ESP32 *(11/09: `15_Lista_de_Compras_Hardware.md` retiro el `LM2596` el 31/08 —«ninguna referencia concreta esta elegida»—: no se da por elegido aqui)* | no esta pedido · 🔴 **y ya es fila de CAMPO**: el ESP32 del Sisga se reinicio con 2 partes `SUBIDA_DE_TENSION` (`ESP_RST_POWERON`) entre los 5 del 10/09 (§3.16). Que su alimentacion fuera la causa es HIPOTESIS; que esta linea no esta pedida, no |
| **C-4** | 🔴 **Quien disena y quien fabrica la placa portadora** | bloquea **desplegar**, no **probar** |
| **C-5** | 🟠 **Las microSD de las camaras** (comprarlas ya esta decidido; falta como se configura la grabacion) | `A-0` |
| **C-6** | 🔴 **El `Y2` de 32,768 kHz del Maestro `179DB0` NO OSCILA** —medido en campo el 16/09 con `ca2de3d` dentro: `ON:1 RDY:0 BYP:0 SEL:0 EN:0 CNT:--`, o sea el firmware lo PIDE (`LSEON`) y `LSERDY` no sube nunca. No es el `1.22` (aquel da `RDY:1` y `CNT` quieto): **es que el cristal no arranca**, y `REINICIAR_RELOJ` no puede levantarlo | **antes de la sesion de banco.** El firmware ya lo rotula bien (`1.49b3`); lo que queda es **soldador**: `Y2`, sus dos condensadores y las pistas al `PC14`/`PC15`. Las medidas, a `05_Funcional/17_...` (§12) |

> ⚠️ **El `2K2` esta `SIN VERIFICAR`: «no se ha probado en ninguna tarjeta».** La cuenta cumple las
> dos desigualdades, pero es una cuenta. Hoy la unica barrera real es tapar `J16` p1.

---

## 5. 🟡 POR VALIDAR — el codigo esta escrito y NADIE lo ha ejercido en una tarjeta

> **Esta es la caja que el proyecto olvida, y la que mas vale.** Un commit no es una prueba de
> banco; un pack verde tampoco. `CLAUDE.md` ~~§3~~ §0.3 *(la numeracion de hoy)*: *lo que ese `0` dice es que los modelos y los
> arneses de PC no encuentran nada.* **Nada de este apartado esta cerrado, y nada de esto viaja al
> historico hasta que una tarjeta lo diga.**

> ⚠️ **11/09: el Sisga (10/09, firmware `7ff7d12`) es la primera vez que parte de esto corrio en una calle.**
> La cinta del Maestro (§3.16) **ejerce, del lado del Maestro y por telemetria**: `N-150` (el ciclo arranca
> tras `SET_TIEMPOS:3,3,15`), `N-153` (`PLUMA:ARRIBA` con cada verde y `ABAJO` con cada rojo) y el ciclo
> Automatico 3/3/15. **`N-147` solo a medias**: Manual quedo 14 s en rojo/rojo sin ciclar, no 15. Nada de
> esto ve las luces ni el Esclavo: **no se tacha nada de esta caja hasta tener su cinta**.

~~**Los siete que salieron DESPUES de la ultima cinta (05/09, 22:19). Ninguno ha visto cobre:**~~

> 🔴 **CADUCO el titulo, medido el 11/09 con `git merge-base --is-ancestor`:** (1) **la ultima cinta
> ya no es la del 05/09** (firmware `42a52cd`) **sino la del Sisga del 10/09** (Maestro, `7ff7d12`);
> (2) **`N-142` (`6274acc`) y `N-147` (`8e9e8a9`) NO salieron despues**: ya iban dentro de `42a52cd`
> —y `ESTADO.md` da `N-142` por visto en aquella cinta—; (3) **la lista se quedaba corta**: despues de
> `42a52cd` entraron tambien la reescritura del reloj de `D-20` (`9dd8bbf`), los arreglos de `N-160`
> (`f151674`, `3a91973`, `6c90ff0`), `N-154` (`1d92bfb`) y `D-24` (`9550c57`) —**todos dentro de
> `7ff7d12`**, o sea en el Sisga, pero sin cinta que los ejerza salvo el sembrador—, y **despues de
> `7ff7d12`, sin ninguna tarjeta con cinta**: los getters de `c51cc85` (iban en `b354fe9`, que se
> «probo despues» sin cinta), el `ESC:` de `141f191` y el `GO_GREEN` idempotente de `63d6964`, mas el
> puente de `d68eb36`. **La tabla de abajo se conserva como estaba; esas filas nuevas estan debajo de
> ella.**

**Los siete de la tabla original** *(con la correccion de arriba)*:

| `N-x` | commit | que cambia, y por que hay que ejercerlo en tarjeta |
|---|---|---|
| **N-142** | `6274acc` | **el Esclavo AVISA por radio de su ambar de emergencia**, y los dos vetos se quedan. Es lo que desatasco el bloqueo del cruce **sin tocar el cerrojo**: el Maestro deja de agotar reintentos a ciegas. **Toca el ambar: se ejerce con las dos puntas** |
| **N-147** | `8e9e8a9` | **el Modo Manual ya no entra por la puerta del Automatico** — antes programaba un verde para dentro de `tiempoDespejeMs` y el equipo ciclaba solo. **Se ejerce entrando en Manual y esperando 15 s sin tocar nada** |
| **N-150** | `414b962` | **el ciclo no arrancaba tras aplicar tiempos** (quedaba en rojo para siempre), y **los parsers de la app eran TRES**. **Se ejerce aplicando tiempos desde el telefono** |
| **N-151** | `273b315` | **`DAR PASO` en un modo sin coordinador trababa el cruce PARA SIEMPRE**. **Se ejerce pulsando DAR PASO fuera de Automatico** |
| **N-152** | `d6ce67e` | **el Esclavo avisa de que RETIRA su ambar — y en `MODO_AMBAR` el Maestro ESTABA SORDO**. **Se ejerce cancelando el ambar desde el Poste 2** |
| **N-153** | `79ef5a6` | **la talanquera se publica y se dibuja**: la app no la ensenaba. **Se ejerce mirando la pantalla con la pluma arriba y abajo** |
| **N-157** | `4b90f98` + `ee957ef` | **la camara se vigila a si misma** (fase 1 de `D-13`). 🔴 **Su propio texto lo dice: `CAM_CIEGA` a su valor de produccion NO ES EJECUTABLE en una sesion de banco. El camino esta comprobado en su FORMA, no en su TIEMPO** — y esa es justo la clase de defecto que un pack de forma no puede ver |

**Y los que la tabla no tenia** *(anadidos el 11/09; ninguno ejercido en cobre por una cinta salvo lo dicho)*:

| | commit | que cambia, y por que hay que ejercerlo en tarjeta |
|---|---|---|
| **`D-20`, el reloj** | `9dd8bbf` | **el reloj de las dos puntas se reescribio**: base sembrada + `millis()`, y de ella cuelga `reloj_enHora()`, o sea **la autorizacion del Degradado**. En `7ff7d12`; la cinta del Sisga solo ve que la siembra entra (`SET_RTC_LO_ACUSA_EL_PUENTE`) y que la hora no llega al `$STATUS` (§3.16) |
| **`N-160`** | `f151674`, `3a91973`, `6c90ff0` | el sembrador que acusa lo que hizo, el contador que vuelve a decir «no hay reloj», la hora que saltaba si `Y2` arrancaba tarde, y el Diario del Maestro. En `7ff7d12` |
| **`N-154`** | `1d92bfb` | el `$ALARM` acotado por buffer: **la alarma que perdia la HORA**. En `7ff7d12`; la cinta del Sisga no trae ningun `$ALARM` |
| **`D-24`** | `9550c57` | `CAM_CIEGA` a 24 h de paso abierto. En `7ff7d12`; no ejecutable en una sesion (arriba) |
| **`N-162`, los getters** | `c51cc85` | la hora del STM32 sale de la base sembrada antes que del RTC. En `b354fe9`, **cargado sin cinta** |
| **`N-162`, `ESC:`** | `141f191` | el `ESC:` deja de decir `VERDE` con el Esclavo en rojo. **Solo en `main`** |
| **`N-162`, `GO_GREEN`** | `63d6964` | un `GO_GREEN` repetido ya no reinicia el ambar del Esclavo. **Solo en `main`**. Toca el ambar: se ejerce con las dos puntas |
| **`N-162`, el puente** | `d68eb36` | el puente vuelve a sellar solo el hueco `HORA:--:--:--`. **Solo en `main`**: el Sisga lleva el sello revertido |
| 🆕 **`D-26`, la hora la manda el ESP32** | `68dd2c5` (merge de `a0313fe`) | **las TRES tarjetas cambian a la vez**: el ESP32 siembra a su STM32 (`CMD:HORA_ESP32`) y se queda el `SET_RTC`; el STM32 ya no escribe su RTC al sembrar; el Esclavo elige fuente (radio / su ESP32); el Degradado pasa por rojo ante un salto > 29 s; `$ALARM …EVENTO:HORA_ESP32…` con tres causas. **Solo en `main`, sin banco, y el Sisga no lo lleva.** Se ejerce con las dos puntas y sus dos ESP32: arrancar y ver `HORA_ESP32_SEMBRADA` en el diario del poste 1 (sale **solo en el cambio**: la primera tras el arranque o tras una alarma, no cada 5 min); cortar la radio 25 s y ver `FALLO_RF` y luego `HORA_ESP32_SEMBRADA` en el diario del poste 2; desenchufar `J17` 15 min y ver `CAUSA:J17_MUDO`. ⚠️ **No subir a campo con `H1` abierto** (§0 fila 1.13) |

> 🔴 **LO QUE ESTA SESION DE BANCO NO VA A PODER PROBAR, y hay que saberlo ANTES de subir al poste
> —no descubrirlo alli—:**
>
> | | por que |
> |---|---|
> | ~~**El Modo Degradado del poste 2** (`D-18`)~~ | ~~su guarda abre con `if (!reloj_enHora())` y en esa punta esa bandera es **falsa siempre**: el cristal `Y2` esta muerto. El comando existe y llega; **el equipo contestara que no, correctamente, todas las veces**. Lo destraba `D-20` (§3.4.bis), decidida y **sin construir**. Quien lo pruebe sin saber esto lo anotara como defecto~~ 🟢 **SI SE PUEDE PROBAR desde `9dd8bbf`** (medido el 11/09): `reloj_ajustarConAcuse()` pone `horaValida = true` en el Esclavo sin `Y2`, por `SET_RTC` o por la hora que llega por radio. **Antes de probarlo, hora al Esclavo.** Lo que NO se podra ver es su guarda de hora ya dentro: es inalcanzable (§3.10.bis) |
> | **`CAM_CIEGA` en su tiempo real** | son ~~6 h~~ **24 h** de paso abierto (`CAM_CIEGA_MS = 86400000UL` en los dos `botones.cpp`, `9550c57`). **No es ejecutable en una sesion.** Solo se puede ejercer con una compilacion de umbral reducido, **y esa compilacion no es la que va a campo** |
> | **`D-14`, que la camara grabe al cerrar el contacto** | **no existe en el firmware**: cero anclas en las dos puntas (§3.5). La via esta confirmada en el manual de la camara; lo que falta es nuestro lado |
>
> **Los tres se anotan como NO PROBADO, no como fallo.** Es la distincion que el banco del 3-4/09
> ya obligo a escribir: un *«no se pudo probar»* no es un *«sigue roto»* ni un *«ya esta»*.

**Y los que no son de esos siete:**

| | que | por que no esta cerrado |
|---|---|---|
| **N-110** B2 | el teclado del PIN acepta pulsaciones con el modal cerrado | ~~**no se comprueba leyendo.**~~ 🟢 **construido (`b033f0b`, `tecladoPinAbierto()`) y ejercido en el DOM** por `test_dom_execution.js`; **falta el telefono**. B1 si esta cerrado: `NMEAParser.validarTrama(linea)` tiene llamador vivo en `app.js` (`dad4615`). §6.6 |
| **la app entera** | `disconnect()` existe (6 llamadas) · `padding-bottom: 90px` puesto · `parseInt(...)\|\|0` solo en comentarios que narran su retirada · `discoverUnpaired` usado (7) · `validarTrama()` con llamador | **los cinco arreglados en el fuente y NINGUNO ejercido en un telefono** |
| **`TECHO_POR_SUELO = 2`** | el Modo Inteligente puede alargar una fase **hasta el DOBLE** del tiempo configurado | **aprobado CON CONDICION —«si un funcional revisa el manual y este manual es claro»— y la firma NO EXISTE. Y ya salio en el paquete del 05/09** |
| **el `0x68`** del `DS3231` | | ~~`SIN VERIFICAR` sobre el modulo~~ 🟢 **verificada en el modulo del Maestro `179DB0`** por la cinta del Sisga (`SET_RTC`/`LEER_RTC` contestan y la hora avanza); **falta el del Esclavo** |
| **las** ~~21~~ **`SIN VERIFICAR`** de `17_` | | por definicion *(la cifra se retira el 11/09: no se reproduce)* |

---

