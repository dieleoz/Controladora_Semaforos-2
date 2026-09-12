# Roadmap — Controladora de Semaforos Moviles de 3 Estados (V9.0)

**Este fichero lleva SOLO lo pendiente y lo que falta validar.** El 07/09/2026 se le sacaron 4.631
lineas de cosas ya cerradas: estan **integras y con su evidencia** en
[`roadmap_hist.md`](roadmap_hist.md), que lleva indice por `N-x` y por apartado. **No se borro
nada** — la cuenta esta publicada al final de aquel fichero.

> **Como se lee.** Arriba, **lo que se puede hacer y quien lo desbloquea**. Abajo, **el porque** de
> lo que sigue abierto, con su medida al lado. Si algo no esta aqui, o esta hecho o esta en el
> historico.

> ⚠️ **Las citas a `CLAUDE.md` con `§x.bis`, `§x.ter`, `§x.quater` o `§4.sexies` son de ANTES del 07/09**,
> cuando ese fichero se compacto (`74a1054`) y se renumero. Se deja la cita como estaba —es cronica— y la
> regla se busca **por su titulo**, no por el numero.

## Que manda sobre este fichero, y en que orden

| | |
|---|---|
| **la decision vigente** | [`DECISIONES.md`](DECISIONES.md) — **una fila suya gana a cualquier parrafo de aqui** |
| **el hardware medido** | `05_Funcional/17_Arquitectura_28-08_y_Decisiones_Abiertas.md` — gana a `DECISIONES.md` y a `CLAUDE.md` en cobre, pines y conectores |
| **las reglas permanentes** | [`CLAUDE.md`](CLAUDE.md) |
| **el estado de hoy** | [`ESTADO.md`](ESTADO.md) |
| **el porque de lo cerrado** | [`roadmap_hist.md`](roadmap_hist.md) |

## Las cifras NO se copian aqui

**La instalacion certificada es `V8.4`, commit `e303485` (31/07/2026).** 🔴 **Pero el 10/09 un Maestro
con firmware V9 `SIN_BANCO` corrio en campo, en El Sisga**: cargado el paquete del 08/09 (`7ff7d12`) y
probado despues el del 10/09 (`b354fe9`) (§3.16). En el
repositorio, `V9.0` en `main`.

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

> ## 🔒 DECIDIDO Y CERRADO EL 11/09 — NO SE REABRE DESDE UN DOCUMENTO
>
> | | lo que fija | lo que deroga |
> |---|---|---|
> | **`D-25`** | **Cuatro camaras, dos por poste**: camara 1 entre `J16` p9 (3,3 V) y p10 (`CAM_C_PIN`), camara 2 entre `J16` p11 (3,3 V) y p12 (`CAM_D_PIN`), cada una por el contacto seco `1A`/`1B` de su salida de alarma. **Talanquera en `J15`**: p1 = 12 V, p2 = drenador de `Q10` (**no es masa**), a la bobina de un rele cuyo contacto va a `OPEN` de la centralita | de `D-13` **solo** «una camara por poste» y «`p12` vacio a proposito»; «las 4 camaras se revocaron el 28/08» (`ESTADO.md` C1) |
> | **`D-26`** | **La hora la manda el ESP32 de cada poste**: el `SET_RTC` del telefono sin PIN en el ESP32; cada STM32 se siembra desde el `DS3231` de su ESP32 **cada ~5 min** (`CMD:HORA_ESP32`); el Esclavo toma la hora del Maestro con radio y la de su ESP32 sin radio; en Degradado un salto mayor que el margen del cruce **pasa por rojo**; **dos alarmas**: perdida de radio, y enlace ESP32-STM32 caido o siembra rechazada | el numero de `A-15` (una hora -> ~5 min); la regla del Esclavo «acepta su ESP32 si la radio lleva 2 h sin sembrar» |
> | **`D-27`** | **Las cuatro camaras estan compradas** (modelo `D-10`, `DS-2CD2683G2-IZS`); **`J14` LIBRE, sin cablear** —el fin de carrera no se instala en este despliegue; mientras el firmware lea `PB0` como demanda, en `J14` no se conecta nada—; **la configuracion de cada camara es la del manual del modelo** (`04_Manuales/MANUAL_CONFIGURACION_CAMARAS_IA.md` y la ficha) y el Manual 9 se alinea con el; **talanquera por rele y centralita como en la guia del Sisga**, y la lista de compras se ajusta; los `.docx`, pendientes | `A-2` en «el fin de carrera va a `J14`» (sin efecto en este despliegue); los valores de configuracion de `D-13` (filtro, umbral, sensibilidad) en lo que choquen con el manual del modelo |
>
> **Si un manual, una guia o un comentario del fuente dice otra cosa, ESE DOCUMENTO esta caducado: se tacha y se corrige hacia la decision, nunca al reves. Cambiar una de estas tres exige una fila nueva en DECISIONES.md firmada por el responsable.**
>
> ⚠️ *Medido al alinear `D-27` (11/09), para que nadie lo lea de mas:* el manual del modelo **fija
> el objetivo** (☑ Vehiculo · ☐ Humano, si la casilla existe) y **no fija** `Threshold`,
> `Sensitivity` ni el horario de la salida, que siguen en los de `D-13` (minimo, alta, 24x7). Su
> zona —«la zona donde el vehiculo espera»— es anterior a `D-13` y **no se aplica**: `D-27` no
> deroga la zona, que sigue siendo **el barrido de la pluma**. Tabla valor a valor: Manual 9 §4
> Paso 3. Lo que queda para el responsable, en la fila 2.1 de abajo.

### 🎯 LO QUE QUEDA PENDIENTE DE IMPLEMENTACION (medido el 11/09 sobre `b79d904`; `648b62f` solo toca `README.md`; **lo de la hora re-medido el 11/09 por la tarde sobre `68dd2c5`**, el merge de `D-26`; **lo del ambar de emergencia re-medido el 12/09 sobre `913c29c`** — cierra la fila 1.2 y **abre la 1.17**, que es lo que su arnes dejo a la vista al compilar el despachador de verdad)

> **Es la unica lista viva de este fichero; lo de debajo es el porque.** Cada fila de §0 a §6 que
> daba algo por abierto se midio el 11/09 contra el fuente de `main` —`grep` por los dos nombres
> posibles y filtrando comentarios, `CLAUDE.md` §7.1—: **lo que el codigo ya hace se cerro en su
> apartado, tachado y con el commit o el `grep` que lo prueba**, y lo que no, esta aqui. **Lo que
> otros agentes construyen hoy en worktrees NO esta en `main` y va como EN CONSTRUCCION, no como
> hecho.** Tres filas de §3.6 (la guarda de rutas y el intermitente del simulador) **no se
> re-midieron** y por eso no entran en ningun grupo.

**(1) TECLADO — se hace escribiendo en el PC** *(nada entra aqui sin un `grep` que diga que no existe)*

> ⚠️ **LA TABLA VA POR DANO, NO POR NUMERO.** Las tres primeras se subieron aqui el 12/09 desde §3,
> §6 y §9, donde eran «el porque» y **nadie las contaba como pendientes**; van arriba porque son las
> que pueden herir. **Los numeros NO se renumeran** —`adaptador_esclavo.cpp` y `orquestador.cpp`
> citan `roadmap 1.16(c)` desde el fuente (`CLAUDE.md` §5)—, asi que el orden de lectura y el orden
> numerico no coinciden a proposito.

| | que | `D-x` / `N-x` | ficheros que tocaria | la medida de que falta |
|---|---|---|---|---|
| 🆕 **1.20** | 🔴 **EL PIN `1234` VIAJA EN CLARO, Y CUATRO ORDENES NO LO PIDEN.** `state.correctPin = '1234'` literal sobre SPP sin cifrar, y `FORZAR_ROJO`, `SET_MODO:MENU`, `SET_MODO:ALCANCE` y `AMBAR_EMERGENCIA` **no lo exigen**. 🔴 **Hasta hoy su unico puntero era §9 `B3` —la lista de la V2—, que dice literal *«No esta clasificado como riesgo de seguridad, y lo es»*: la unica linea del repositorio que lo llamaba riesgo vivia en el apartado que nadie mira.** Sube aqui por eso, no porque haya cambiado nada | `N-109` §5 · §9 `B3` · §3.1-6 | `05_Funcional/App_Semaforo/{app.js,www/app.js}` y las otras **dos** copias de disco (`android/app/src/main/assets/public/`, `android/app/build/…`), `CLAUDE.md` §14 | `grep -n correctPin` sobre `7f90958`: `correctPin: '1234'` en la **linea 111** de las dos copias versionadas; `find -name app.js` da **cuatro** en el arbol de la app |
| 🆕 **1.21** | 🛑 **LEER `D-30` ANTES DE TOCAR ESTA FILA — la cura CAMBIO el 12/09, unas horas despues de escribirla.** `D-30` (`70c4c53`) **deroga la segunda mitad de `D-1` y retira `mando.cpp`, `menu.cpp` y `lcd.cpp` de las dos puntas.** O sea que **el arreglo YA NO ES ANADIR UNA GUARDA a `A.A.A`: es que el reconocedor de secuencias desaparece**, y meter una guarda en un fichero que se va es trabajo que otro deshace (`CLAUDE.md` §11.4). **La fila NO se borra por eso, por tres razones medidas:** **(1)** el defecto **esta vivo en `main` hoy** y lo seguira estando hasta que `D-30` se construya; **(2)** `D-30` avisa de que **no es un borrado mecanico** —`mando_ambarLocal()` tiene **seis** llamadas vivas y su veto es `SFTY-21`: retirar solo el armador **deja los `if` ABIERTOS, no inertes**—, asi que entre hoy y su cierre hay una ventana; **(3)** ⚠️ **lo de la guia NO lo cierra `D-30`**, porque los equipos que estan en la calle siguen con el firmware que SI tiene `mando.cpp`. ⬇️ **Lo medido, que es lo que hay que conservar:** 🔴 **`A.A.A` ENTRA AL MODO AUTOMATICO SIN GUARDA EN EL MAESTRO — arranca el ciclo, o sea ABRE PASO.** ⚠️ **Y corrijo como estaba escrito en §3.5-6, medido hoy en el fuente: «las otras dos si estan frenadas» es FALSO.** `B.B.B` tampoco tiene guarda **y es correcto que no la tenga** —va a ambar, que es el estado seguro, y su comentario lo razona: *«impide que nadie quede atrapado con un semaforo en estado raro a 5 m de altura»*—. **La asimetria real es `A.A.A` contra `A.B.A.B`: las dos entran a un modo EN MARCHA y solo una pregunta.** 🔴 **Lo que lo agrava, medido el 12/09: la guia de campo nombra las tres secuencias con su efecto, pero describe el `A.A.A` del ESCLAVO —«vuelve a obedecer al Maestro»— y NO dice que en el MAESTRO esas mismas pulsaciones arrancan el ciclo.** El documento que llega al poste esta incompleto **justo en la mitad que abre paso**. Con `J16` p5/p8 **vacios y pelados** (`D-1`), lo que se cablee ahi compone la secuencia sin que nadie la pida | **`D-30`** (manda), `D-1` (derogada su 2ª mitad), `A-2`, `FW-N53` · §3.5-6 | `Maestro/src/mando.cpp` (**lo retira `D-30`**) · la guia de campo (**otro agente**: no se toca desde aqui) | `Maestro/src/mando.cpp`: `A.B.A.B` → `if (modo_degradado_evaluarEntrada() == MDG_OK)`, con su comentario *«LA RED DE SEGURIDAD REAL NO ES LA SECUENCIA, ES ESTA COMPROBACION»*; `A.A.A` → `confirmarYActuar(ACC_AUTOMATICO, DESTELLOS_AUTOMATICO);` **sin un solo `if`**. En `Esclavo/src/mando.cpp` el mismo gesto es `ACC_OBEDECER` |
| 🆕 **1.22** | 🔴 **EL LIMITE DURO DE 48 h DEL DEGRADADO NO VENCE NUNCA.** Con el contador del RTC congelado en un valor NO nulo, `respaldo_horasDesdeSync()` **da 0 para siempre** y el Degradado concede 48 h nuevas en cada corte. **§0 1.7 recoge solo la LECTURA no destructiva de los bits; el ARREGLO —darle un contador que AVANCE— no lo pedia ninguna fila.** Es el mismo hueco escrito en tres sitios: §3.16-D, §3.4.bis-3 y la segunda mitad de `H8`. ⚠️ **El Maestro tiene el mismo patron y CERO instrumentos** | `N-160`, `N-162` `H8`, `D-20`, `D-29` · §3.16-D | `Esclavo/src/{reloj,respaldo,modo_degradado}.cpp` y los mismos del Maestro | el comentario de `Esclavo/src/reloj.cpp` lo razona entero: *«millis() vuelve a cero tras un corte de energia, y tBaseMillis se reasigna en CADA siembra, o sea que el contador BAJA cada vez que llega la hora del Maestro»*, contra una `respaldo_horasDesdeSync()` escrita sobre *«una resta de dos contadores monotonos… el contador no vuelve»* |
| | ↑ **lo que puede herir** · ↓ **el resto de lo pendiente, por orden de llegada** | | | |
| ~~**1.1**~~ | ~~🟠 **EN CONSTRUCCION (worktrees, 11/09): el ESP32 manda la hora** —siembra ESP32→STM32 desde el `DS3231` al arrancar, tras cada `SET_RTC` bueno y cada ~5 min; el `SET_RTC` del telefono deja de cruzar al STM32; en el Esclavo manda la radio; `esp32_05` se estrecha a una excepcion—~~ 🟢 **CONSTRUIDA Y EN `main` desde `68dd2c5`** (11/09 por la tarde, merge de `a0313fe`, revisado por el diff por el arquitecto: «fusionar con cambios», §3.16 `N-162`). **Medido sobre `68dd2c5`, filtrando comentarios:** la cadencia es `#define SIEMBRA_INTERVALO_MS 300000UL` (`ESP32_Expansion/include/contrato.h`; ~~`INTERVALO_SYNC_MS`~~ deja de reusarse **a proposito**, y en `ESP32_Expansion/` solo sale en dos comentarios que lo explican); `siembra_revisar()` se llama en el `loop()` de `ESP32_Expansion/src/main.cpp` y `siembra_ahora()` dentro de la rama `RELOJ_OK` del `SET_RTC` en `despachador.cpp`; `grep SET_RTC` sobre `{Maestro,Esclavo}/src/bluetooth.cpp` → **0 lineas de codigo** (la rama la sustituye `CMD:HORA_ESP32:`); `grep "rtc\.set"` sobre los dos `reloj.cpp` → solo `setClockSource()` y el `setMonth(1)` de `reloj_fijarEnero()`, que desde `68dd2c5` sale si hay base sembrada (`if (tBaseMillis > 0) return;`). **Cierra en el fuente §3.1-9 y la congelacion de ~3 s del Maestro.** ⚠️ **Sin banco y sin tarjeta** (§5), y **lo que deja abierto bloquea CAMPO**: `H1` → fila **1.13**; los instrumentos → **1.14** | `D-20`, `D-26`, `N-162` | — | ~~`grep INTERVALO_SYNC_MS ESP32_Expansion/src` → **0**; `reloj_ajustarConAcuse()` sigue llamando a `rtc.setHours()` con `rtcOperativo` en `true`~~ *(las dos medidas eran de `b79d904`; sobre `68dd2c5` la siembra existe con otro nombre y `reloj_ajustarConAcuse()` ya no escribe el RTC)* |
| ~~**1.2**~~ | ~~🔴 **`AMBAR_EMERGENCIA` sin PIN no avisa al Maestro**, y el pack que mire las DOS puertas~~ 🟢 **CONSTRUIDA y en `main` el 12/09 (`913c29c`)** — ⚠️ **sin banco y sin tarjeta**. Las dos puertas avisan, y el `$ACK` dice si el aviso pudo oirse: `OK_SIN_RADIO` / `YA_EN_AMBAR_LATCH_PUESTO_SIN_RADIO` cuando esta punta ya declaro `FALLO_RF`, leyendo `enlaceCaidoAnunciado` —el mismo dato con el que ya se publica esa alarma, no un segundo reloj de silencio (`CLAUDE.md` §2)—. **Y los TRES instrumentos que lo dejaban pasar, arreglados**: `esclavo_07` deduplicaba por NOMBRE y las dos ramas se llaman igual; `esclavo_08` comparaba seis prefijos y ninguno era `protocolo_`; y el arnes de dos puntas ejercia una **TRANSCRIPCION** de la puerta CON PIN escrita en su propio adaptador —dos copias buenas de una puerta mala, `CLAUDE.md` §8—: se retira y se compilan el `bluetooth.cpp` REAL del Esclavo y el `modo_ambar.cpp` REAL del Maestro, el telefono teclea la linea **leida del C++** y el `$ACK` que recibe pasa a ser observable. Bloque **H** nuevo (H0..H4): **76/77 → 85/86**. ⚠️ **Lo que NO cierra, y lo publica H4 como nota que no cuenta:** si muere solo el transmisor del Esclavo, el aviso no sale, esta punta **no puede saberlo** —solo oye silencios de lo que RECIBE— y el `$ACK` sale igual que con la radio sana. Cerrarlo pide un **acuse al aviso**: protocolo y LAS DOS puntas, como el `ACK_RED` de `G9` | `N-142`, §3.16-A | hecho: `913c29c` | ~~`protocolo_enviarPaquete(CMD_AMBAR_ESCLAVO)`: **una sola llamada** en `Esclavo/src`, dentro de la puerta CON PIN~~ → **dos**, una en la rama comparada contra `cmd` (sin PIN) y otra en la de `accion` (con PIN), medidas sobre `913c29c` filtrando comentarios |
| **1.3** | 🔴 **`D-23` — la pantalla del poste 2 por `$EVENT`, emitido tambien al conectar.** Es la mitad Esclavo de las «dos pantallas» (§6.9) y el sitio natural de la antiguedad de sincronizacion que no viaja (§3.5-4) | `D-23`, `A-14` | `Esclavo/src/bluetooth.cpp`, `app.js` (sus cuatro copias), la APK | `decisiones_01_anclas` acusa `D-23`; el `$STATUS` del Esclavo no lleva ningun campo de sincronizacion |
| **1.4** | 🔴 **`D-21` pieza A — que el `OSF` del `DS3231` llegue a `reloj_enHora()`**, y con ella que la guarda B del Esclavo deje de ser inalcanzable | `D-21`, §3.10.bis, §3.16-E | `ESP32_Expansion/src/`, `{Maestro,Esclavo}/src/reloj.cpp` | `grep -w OSF` fuera de `ESP32_Expansion/`: solo un comentario de `Maestro/include/reloj.h`; en el Esclavo el unico `horaValida = false` vive en `reloj_setup()` |
| **1.5** | 🟠 **Lo que queda de `D-20` en el STM32:** ~~diferir la reanudacion tras corte a la primera siembra~~ *(esa mitad la construyo `D-29` el 12/09, fila 1.19)*, y retirar `STM32RTC`, `N-25`, `N-31`, `reloj_ajustar()` y el calendario en enero · 🆕 **y con `STM32RTC` se va el residual de `H8` que no tenia fila propia: el `rtc.begin()` de `reloj_actualizar()` al adoptar el LSE, que bloquea hasta ~2 s** | `D-20`, `N-162` `H8`, §3.4.bis | `{Maestro,Esclavo}/src/{main,modo_degradado,reloj}.cpp` y los tres arneses que definen `reloj_ajustar()` | `modo_degradado_reanudarTrasCorte()` / `degradado_reanudarTrasCorte()` se llaman en `setup()` y borran el indicador sin hora —⚠️ **a proposito**: su comentario razona que dejarlo puesto seria una entrada automatica; diferirlo tiene que conservar esa barrera—; los cinco siguen en el fuente |
| **1.6** | 🟠 **El checksum de la SUBIDA** — ⚠️ sin decision escrita de si y cuando (grupo 2) | §3.1-3 | `{Maestro,Esclavo}/src/bluetooth.cpp` | `calcularChecksum()` solo lo llama `enviarTramaConCrc()` |
| **1.7** | 🟠 **`reloj_diagnostico()` en el Esclavo, y una lectura NO destructiva de los bits del RTC** | §3.1-5, §3.5-5 | `Esclavo/src/{reloj,bluetooth}.cpp`, `Maestro/src/bluetooth.cpp` | `reloj_diagnostico` solo existe en el Maestro; `reportarBitsDelReloj()` solo lo llama `REINICIAR_RELOJ` cuando falla, que borra hora y respaldo |
| **1.8** | 🟠 **`D-18` sin canal de vuelta**: el Esclavo en Degradado no se lo puede decir al Maestro — ⚠️ un comando nuevo cambia el contrato de la radio | §3.5-3 | `{Maestro,Esclavo}/include/protocolo.h`, `coordinador.cpp`, `Esclavo/src/main.cpp` | 21 `#define CMD_` en `Esclavo/include/protocolo.h`, ninguno de Degradado |
| **1.9** | 🟠 **`D-25` — el vigilante de la SEGUNDA camara:** una camara que nunca dio flanco no se detecta, y la exencion que lo permite se escribio porque `p12` iba vacio — ⚠️ **el COMO no esta decidido** | `D-25`, `D-13` | `{Maestro,Esclavo}/src/botones.cpp`, `camara_03_vigilante` | `vigilante_tick()` salta `CAM_CIEGA` mientras `camHuboFlanco[i]` es falso; `camara_estado()` la salta al publicar |
| **1.10** | 🟠 ~~**`D-25` — alinear `17_` §1.7, el Manual 9 y `ARQUITECTURA.map`**, que dicen `p12` vacio *(documento, no codigo; lo hace otro agente — `README.md` ya se alineo en `648b62f`)*, y~~ *(los documentos se alinearon con `D-25` en `10928ad` y con `D-27` el 11/09 por la tarde, sin comitear al escribir esto; quedan los `.docx`)* **el ancla `D-25` en el fuente** | `D-25` | esos tres; un ancla en los `botones.cpp` | `decisiones_01_anclas` da **48/53** el 11/09: `D-25` sin ancla y sin manual, ademas de `D-14`, `D-22`, `D-23` |
| **1.11** | 🟠 **Recompilar la APK desde `main`** — *(11/09 noche: ya lleva dentro los textos de `D-26` (`fd595ea`) y de `D-21` (1) (`fdccd5b`), en el modulo nuevo `js/avisos_equipo.js`, que el `cap sync` tiene que llevarse; los textos NO dependen de la cadencia de la fila 2.10, asi que puede compilarse ya, UNA vez, con el sufijo `SIN_BANCO`)* | `N-162` | `App_Semaforo/` → `05_Funcional/*.apk` | la mas nueva es la de `b354fe9`, y `git diff b354fe9..HEAD` sobre `app.js` no sale vacio (`e91854c`) |
| **1.12** | 🟡 **Instrumentos:** el caso borde de rojo de `validateTiempos()` · `costura_10` no censa `ESP32_Expansion` · la ventana `S_FALLO` con el Esclavo en verde, que nadie mide · ningun instrumento ejerce la base sembrada de `c51cc85` | §3.1-4, §3.9, §3.16 | `test_unitarios_app.js`, `costura_10_funciones_muertas`, `Validacion_Automatico/dos_puntas/orquestador.cpp` | el caso de rojo sigue siendo `validateTiempos(3, 1, 25)`; `PUNTAS = ("Maestro", "Esclavo")`; A9 excluye `S_FALLO` por nombre; `grep tBaseMillis` sobre `Simulaciones/` y `Validacion_*` → **0** *(sobre `68dd2c5`: **un** fichero, `reloj_03_manda_la_radio.py`, que lo LEE por texto — sigue sin compilarlo nadie, fila 1.14)* |
| 🆕 **1.13** | ~~🟠 **EN CONSTRUCCION (worktree, 11/09 por la tarde; NO esta en `main`)**~~ *(11/09 por la noche: **CONSTRUIDA y en `main`** — commit `merge(D-21 (1))`; `reloj_horaFiable()` en las dos puntas, plazo `HORA_CADUCA_MS` = ~~320 s~~ **280 s desde el 12/09** (`dca17cd`) y **400 s cuando entre `D-28`**, fila 1.18, derivado en `reloj.h` y recalculado por `reloj_04`; **sin banco**)* 🟠 **`D-21` pieza (1) — AMBAR INTERMITENTE EN LA PUNTA CUYA HORA CADUCO, dentro del Degradado.** Es la cura de **`H1`** del arquitecto (§3.16 `N-162`): con el `J17` del Maestro mudo su hora corre sobre el HSI, la radio la empuja al Esclavo, y **al caer la radio el Esclavo adopta su `DS3231` y el Maestro sigue con la derivada**: verde contra verde en cada ciclo. `D-26` (4) protege el INSTANTE del salto, no el desfase que deja. **Decidida el 07/09** (`D-21` (1)) **y sin construir hasta hoy**; `D-26` la vuelve urgente porque tumba la premisa de los «MESES» (fila 2.7). **Bloquea campo, no el merge de `D-26`** | `D-21` (1), `D-26`, `N-162` `H1` | lo dira su diff al integrarse | en `main`, `ESP32_Expansion/src/main.cpp` lo confiesa en su cabecera: *«NO declara vieja la hora que tiene ni cae a ambar por ello: eso es la pieza (A) de D-21, sin construir»*; `horaEsp32Vigilar()` publica `$ALARM …EVENTO:HORA_ESP32,CAUSA:J17_MUDO…` y **no hace nada mas** |
| 🆕 **1.14** | ~~🟠 **Los instrumentos de `D-26` — ningun arnes COMPILA el reloj del Esclavo.**~~ *(11/09 por la noche: **HECHO** con `D-21` (1): el arnes del Degradado a dos puntas compila el `reloj.cpp` REAL de las dos puntas —silicio sustituido en `dos_puntas/reloj_real/`— y su bloque F ejerce `H1`, los bordes de `H4` (±30 → rojo, ±29 → directo, adelante y atras) y la frontera de 25 s de `H5`; 53/53, con 13 defectos inyectados vistos caer. El comentario de `H9`, corregido)* La recomendacion del arquitecto: **un bloque en el orquestador de dos puntas que enlace el `reloj.cpp` REAL del Esclavo** y ejerza los tres casos de `H5` (radio fresca, arranque sin radio, radio intermitente) y los bordes de `H4` (salto `= DEG_DESPEJE_SEG` → rojo; `= DEG_DESPEJE_SEG - 1` → directo; y un salto hacia ATRAS). Y de paso el comentario caducado de `H9` | `D-26`, `N-162` `H4`/`H5`/`H9`, `CLAUDE.md` §6.3 | `Validacion_Automatico/dos_puntas/{orquestador_degradado,adaptador_esclavo}.cpp`, `Simulaciones/simulador_puente_esp32.py` | `arnes_puente.cpp`: *«reloj.cpp : NO se compila»* y `bool reloj_radioManda() { return rlj_radioManda; }`; `adaptador_esclavo.cpp`: `void reloj_notarRadio() { g_radioNotada++; }`; el bloque E del orquestador ejerce un salto de `DEG_VERDE_SEG + DEG_DESPEJE_SEG` y uno de 2 s, **ninguno en el borde ni hacia atras**; la frontera de 25 s de `reloj_radioManda()` solo la mira `reloj_03_manda_la_radio` **por texto**; `simulador_puente_esp32.py` sigue diciendo *«EN EL WORKTREE DEL ESP32 (11/09) ESTO FALLA»* |
| 🆕 **1.15** | ~~🟡 **`H6` (i): el Maestro TIRA el `bool` de `coordinador_sincronizarHora()`**~~ *(11/09 por la noche: **HECHO** — el diario mira el retorno y publica `HORA_ESP32_SEMBRADA_SIN_PROPAGAR`; hoy ese `false` no puede salir, se cierra la forma)* en la rama `CMD:HORA_ESP32:` de `bluetooth.cpp` —el patron de `CLAUDE.md` §2: la propagacion al Esclavo puede no salir y el diario dice `HORA_ESP32_SEMBRADA` igual—. *(`H6` (ii), el `$ACK` del puente del Esclavo que dice `OK` aunque su STM32 ignore la siembra porque manda la radio, esta **aceptado** por `D-20`: «inutil, no peligroso»)* | `N-162` `H6`, `CLAUDE.md` §2 | `Maestro/src/bluetooth.cpp` | `coordinador.h`: `bool coordinador_sincronizarHora();` y en `bluetooth.cpp` se llama como sentencia suelta |
| 🆕 **1.16** | 🟠 **Residuos de la integracion de `D-26`/`D-21` (1)** — ~~(a) los rotulos «RENDIDO 48h»~~ *(HECHO 11/09 noche, `532e39e`: bandera `rendidoPorHora` puesta DENTRO de cada guarda que rinde y rotulos «RENDIDO HORA» en `modo_degradado.cpp` y `menu.cpp` del **Esclavo** —el Maestro no mentia: `grep RENDIDO Maestro` -> 0, mi nota era falsa—; el arnes de pantalla lo caza por HUELLA DE PIXELES, 126 -> 142)* · ~~(b) los comentarios de `Esclavo/botones`~~ *(HECHO, sin ancla)* · **(c) el bloque D de `dos_puntas/orquestador.cpp` mide la reanudacion tras corte contra un modelo de RTC que el firmware ya no escribe** (`N-162` `H8`) · ~~(d) el aviso de truncado de `reloj_textoHora()`~~ *(en el encargo de la cadencia, 11/09 noche)* · ~~(e) `documentos_06` no vigilaba «la app no pone la hora en el poste 2»~~ *(HECHO: 8 patrones, nombre y consecuencias; al conectarlo aparecieron **6 sitios vivos** que la barrida de `fd74121` se dejo —`13_`, `16_` y cuatro en `3_`—, tachados y corregidos hacia `D-20`/`D-26` (5); 23/23)* · ~~(f) rutas de `ARQUITECTURA.map`~~ *(HECHO a medias y con una REFUTACION: `reloj_02` gana `M:bluetooth.h`; lo de `app_02` era falso —la pareja que yo habia visto vivia dentro de un comentario del propio pack, `CLAUDE.md` §7.1—)* · **(g) `D-21` pieza (2): el `$ALARM ...CADUCADA` sale UNA vez** y el STM32 no sabe cuando se conecta el telefono: quien llegue despues no la ve hasta `D-23` (fila 1.3) · 🆕 **(h) lo que se quedo fuera del alcance del agente y sigue mintiendo:** los comentarios de `Esclavo/src/bluetooth.cpp` y dos de `Esclavo/src/modo_degradado.cpp` que dan `DEG_RENDIDO` por «solo 48 h», y `banco/packs/app_02_modos_simetricos.py` describiendo `RENDIDO` como «el modo TERMINO SOLO al vencer el limite duro» | `N-162`, `D-21`, `D-25`, `CLAUDE.md` §14 | (c) `Validacion_Automatico/dos_puntas/orquestador.cpp` · (g) `{Maestro,Esclavo}/src/bluetooth.cpp` + la app · (h) `Esclavo/src/{bluetooth,modo_degradado}.cpp`, `banco/packs/app_02_*.py` | los informes de los agentes del 11/09, re-medidos al integrar (`fdccd5b`, `532e39e`) |
| 🆕 **1.17** | 🟠 **EL AVISO DEL AMBAR DEL ESCLAVO NO SE ACUSA, y por eso hay una averia que nadie puede ver.** Medido el 12/09 en el bloque **H4** del arnes de dos puntas, que lo publica como **nota que no cuenta**: si muere **solo el transmisor** del Esclavo, `CMD_AMBAR_ESCLAVO` no sale, **esa punta no puede saberlo** —su unico dato de radio es el silencio de lo que RECIBE, y el Maestro le sigue hablando— y el `$ACK` al telefono sale **identico al de la radio sana**. El tecnico se va del poste creyendo que el Poste 1 se entero, que es justo lo que el `SIN_RADIO` de 1.2 vino a evitar en el caso que SI se puede detectar. ⚠️ **Cerrarlo CAMBIA EL CONTRATO DE LA RADIO** (como 1.8): pide un acuse al aviso en **las dos puntas**, del mismo tipo que el `ACK_RED` sin identificador de `G9` · 🆕 **(b)** y el arnes **no ejerce la mitad de VUELTA de `N-152`** —`CMD_CANCELA_AMBAR_ESCLAVO`, que en `main.cpp` lleva el cruce a `MODO_MANUAL`—: `modo_manual.cpp` no se compila en esa DLL, y transcribir su destino seria medir un doble otra vez. Hoy lo mide `costura_14` **por texto** | `N-142`, `N-152`, §3.16-A | (a) `{Maestro,Esclavo}/include/protocolo.h`, los dos `bluetooth.cpp`, `coordinador.cpp` · (b) `Validacion_Automatico/dos_puntas/adaptador_maestro.cpp` | la nota `[NOTA] H4` del arnes y la cabecera del `adaptador_maestro.cpp`, las dos escritas al medirlo |
| ~~🆕 **1.18**~~ | 🟢 **CONSTRUIDA y en `main` el 12/09 (`6c25bda`)** — ⚠️ **sin banco y sin tarjeta**. `D-28` (2): **el plazo de caducidad de la hora pasa a cubrir DOS siembras perdidas.** Deja de derivarse del relevo y se deriva de `3 x cadencia` inflado por el HSI (369 s) -> deriva concedida 10 s -> `HORA_CADUCA_MS` **400 s** (hoy 280). **Lo que cuesta, medido y aceptado por el responsable:** el desfase relativo del cruce sube de 16 a 22 s contra un aguante de **29**, o sea que el margen para lo que difieran los dos `DS3231` **baja de 13 s a 7 s**. Sigue positivo, y el techo lo recalcula `reloj_04` en cada corrida. ⚠️ **El `static_assert` de minimalidad CAMBIA DE SUJETO** —el menor que cubre el caso peor, que ya no es el relevo—; el suelo del relevo se conserva porque vigila otro termino | `D-28`, `D-21` (1), `D-26` (2) | los dos `reloj.h`, `reloj_04`, `esp32_13` | la cuenta rehecha con `_aguante()` de `esp32_13` y `_relativa_s()` de `reloj_04`: 271000 -> 7 s -> 280000 -> margen 13 · 369000 -> 10 s -> 400000 -> margen 7 |
| ~~🆕 **1.19**~~ | 🟢 **CONSTRUIDA y en `main` el 12/09 (`42fead0`)** — ⚠️ **sin banco y sin tarjeta**. `D-29`: **N-20 MURIO EN EL ESCLAVO Y SE RECONSTRUYE.** Desde `N-162` (11/09) la siembra no escribe el RTC hardware, asi que `reloj_setup()` deja `horaValida` en `false` **tras cada corte**, la primera puerta de `degradado_reanudarTrasCorte()` cierra, **y en ese mismo arranque se borra el indicador de la pila**: la hora del ESP32 llega en el `loop()` un segundo despues y ya no hay nada que reanudar. 🔴 **NADIE LO DECIDIO: es un efecto colateral de `D-20`/`D-26`**, y el comentario del firmware que lo achaca a «sin cristal» se queda corto —pasa **tambien con el cristal vivo**—. Se difiere el borrado hasta despues de la primera siembra del arranque. **Lo que NO se toca:** la segunda puerta (el limite duro de 48 h) y la activacion MANUAL de `SFTY-21`. **Cierra de paso las DOS FLOTAS:** hoy una tarjeta cuyo RTC escribio un firmware anterior al 11/09 **si** reanuda y una recien grabada no, con el mismo binario y sin que el `$STATUS` lo distinga. ⚠️ **El Maestro tiene el mismo patron y CERO instrumentos**: el bloque D solo corta al Esclavo | `D-29`, `N-20`, `N-162` `H8` | los dos `modo_degradado.cpp` + el arnes de dos puntas | el bloque D del arnes lo reproduce (`D6b`/`D6c`/`D6d`), y su control `D8` demuestra que la reanudacion **sigue viva**: con el marcador del RTC puesto, el mismo escenario SI reanuda. 🔴 **Y TRES COSAS MEDIDAS AL CONSTRUIRLA, que corrigen lo que esta fila y `D-29` prometian de mas:** **(a)** `D-29` **NO alcanza a una tarjeta con el cristal `Y2` muerto** —ahi cierra la SEGUNDA puerta (`reloj_contadorSegundos()` devuelve 0 a proposito, `N-160`, y la marca sale `CADUCADA`) y el diferimiento ni se activa—, asi que **estrecha pero no cierra la divergencia de las dos flotas**; corregido tambien en `D-29`. **(b)** La ventana de 360 s abria una carrera que en `setup()` era imposible: el ambar puesto con el mando podia quedar pisado por la reanudacion. El responsable decidio el 12/09 **consultar `mando_ambarLocal()` en el camino diferido**; con el mando desmontado (`D-1`) la bandera no se arma nunca, y la guarda existe porque `J16` p5/p8 siguen **vacios y pelados**. ⚠️ ~~**`D-1` YA decidio que el codigo del mando SE QUEDA, con su medida —cinco llamadas vivas, el veto es SFTY-21, y quitar el armador deja los `if` ABIERTOS, no inertes—: no se vuelve a abrir esa pregunta.**~~ 🔴 **DEROGADO EL MISMO 12/09, unas horas despues, por `D-30` (`70c4c53`): el responsable revirtio la segunda mitad de `D-1` y el mando SALE del firmware.** *(No se borra la frase: la reabrio quien podia —una fila nueva del responsable, que es exactamente lo que `CLAUDE.md` §11.1 exige—, y su MEDIDA sigue siendo cierta y es ahora el ALCANCE del trabajo, no una objecion: las llamadas vivas de `mando_ambarLocal()` son **seis** desde que esta fila 1.19 anadio la de `D-29`, y el veto sigue siendo `SFTY-21`.)* **(c)** 🔴 **La mitad MAESTRO se construye pero NO la ejerce ningun instrumento**: `compilar_dos_puntas.ps1` no compila su `modo_degradado.cpp` ni su `main.cpp`, y el arnes del Degradado usa un adaptador que **transcribe a mano** su `loop()`, asi que no lleva la llamada nueva. Entra sin banco que la mire |
| 🆕 **1.23** | 🟠 **LOS TRES DE §3.6, QUE ESTE MISMO APARTADO DECLARA FUERA DE TODA LISTA** —*«Tres filas de §3.6 … **no se re-midieron** y por eso no entran en ningun grupo»*, y llevan asi desde el 11/09—: **(a)** el `PASS` de la guarda de rutas depende del **ORDEN ALFABETICO** de los packs; con los ficheros invertidos da `69 rutas, 4 inexistentes`, o sea que **renombrar un pack la pone en ABORTADO** sin tocar el firmware · **(b)** **catorce rutas** que los instrumentos abren con `ruta_repo()` y la guarda **NO censa** —los documentos de la raiz, dos de `04_Manuales`, el Manual 10, la app entera, `Validacion_LCD/arnes_lcd.cpp` y los dos `compilar_*.ps1`—: mover una tumba la fila entera mientras la guarda publica *«64 rutas, todas existen»* · **(c)** el **INTERMITENTE**, que es el peor porque ensena a volver a correr: el simulador del puente dio `100/101` y `101/101` **con el mismo arbol**, y **no se sabe cual cayo**. Su primera accion no es arreglarlo: es que **el arnes deje rastro de la que falla, a un fichero** | `N-43` · `CLAUDE.md` §5 · §3.6 | la guarda de rutas del banco, `Simulaciones/simulador_puente_esp32.py` | la de §3.6, **sin re-medir desde el 11/09 — y eso es parte del hallazgo, no una excusa** |
| 🆕 **1.24** | 🟠 **LO DOCUMENTAL QUE SEGUIA SIN FILA** — **(a)** §3.3: `evidencia/Informe_Pruebas_Banco_Semaforos_V9.0.pdf` tiene **14 citas en 8 ficheros** y el fichero vive en `evidencia/old/`; el `…_Sesion2.pdf` de `N-126` **no existe en el repositorio**; y **la cinta del 05/09 tampoco**. Es la unica prueba fisica del proyecto y el ancla no esta · **(b)** §7 `N-115`: *«la cuenta `24/29` NO reconcilia … Ocho documentos publican cuentas que no coinciden»* · **(c)** §7 `N-118` sigue *«publicado como abierto en 16 documentos»*, y §3.4-3 admite que `grep -l N-118` da 13 ficheros **sin haberlos leido linea a linea** (`CLAUDE.md` §14: una lista de alcance se RECUENTA) · **(d)** §7 `N-154`: residual *«el `$ALARM` NO se acoto»* · **(e)** §8: *«DOS CORRECCIONES AL MAPA … que la tabla todavia no trae»* · **(f)** §6.2: **tapar el pin de 12 V de `J16` p1 deja de ser precaucion de banco y pasa a ser obligatorio en cada equipo — y hay que ESCRIBIRLO en la guia de instalacion**, que hoy no lo dice | `N-115`, `N-118`, `N-126`, `N-154`, `D-4` · §3.3, §3.4-3, §7, §8, §6.2 | `evidencia/`, los 8 + 16 documentos, `ARQUITECTURA.map`, la guia de instalacion | las de §3.3 y §7, **ninguna re-contada al subirla aqui**: el recuento es el primer paso |
| 🆕 **1.25** | 🟠 **COMENTARIOS E INSTRUMENTOS QUE MIENTEN** — **(a)** §3.11.bis: `Maestro/src/demanda.cpp` **conserva el comentario de procedencia inventada** que `N-160` ya corrigio en el Esclavo, y **dos packs se citan a el** (`camara_01_demanda.py:39`, `camara_02_j16.py:89`). Es `CLAUDE.md` §7.3 en su forma mas cara: un comentario que se inventa su fuente y despues respalda a un instrumento · **(b)** §6.6: `avisarOtraPunta()` escribe el evento con `state.node || '?'` — una barrera de la app que nadie vigila · **(c)** §9 `C2`: repasar `OPTIMIZACIONES.md`, cuya trazabilidad regla→codigo→prueba se levanta buscando `# EJERCE` y **el firmware se movio mucho** · **(d)** §3.12: la receta de la skill `entregar` copia **3 de 13** ficheros de la app, y el conversor a Word sigue con sus dos defectos (el `\|` en celda y el bloque `>` aplastado) | `N-160` · §3.11.bis, §6.6, §3.12, §9 `C2` | `Maestro/src/demanda.cpp`, los dos packs de camara, `app.js`, `OPTIMIZACIONES.md`, la skill `entregar` | **medido el 12/09 sobre `7f90958`:** `Maestro/src/demanda.cpp` lineas 4-5 siguen diciendo *«Sale de la medida del contacto seco: el rele de la camara AcuSense cierra ~1 s por deteccion»*, y `Esclavo/src/demanda.cpp` lo tacha con *«ESA MEDIDA NUNCA SE TOMO»* · `app.js:4356` |
| 🆕 **1.26** | 🟠 **LA MITAD MAESTRO DE «DOS PANTALLAS» NO TENIA FILA.** §0 **1.3** es explicitamente *«la mitad Esclavo»* (`D-23`); lo que falta aqui es que el Maestro **traiga los datos del Esclavo**, que hoy no puede porque el `$STATUS` lleva **un solo ESTADO, el del que la manda**. Cierra la peticion literal del responsable —*«que el Maestro traiga los datos del esclavo»*— y convierte el hueco vacio de la app en una accion (*«Conectarse al esclavo»*) | `D-23`, `A-14` · §6.9 | `{Maestro,Esclavo}/include/protocolo.h`, los dos `bluetooth.cpp`, `app.js` | el `$STATUS` de hoy: un solo campo de estado, el de la punta que responde |
| 🆕 **1.27** | 🟠 **LAS CAMARAS NO HACEN NADA EN LOS MODOS QUE SE USAN, Y TRES COSAS QUE CUELGAN DE AHI** — **(a)** §6.7: `demanda_hayLocal()` tiene **UN SOLO LECTOR**, asi que en **Automatico y en Manual la camara no hace nada**; se afirmo **tres veces** que estaba construido y lo destapo una revision externa · **(b)** el Esclavo manda **3 copias de `CMD_DEMANDA`** por deteccion sobre un canal de **2,4 kbps** semiduplex y **sin guarda** *(SIN MEDIR)* · **(c)** `camara_leerPin()` hace `delay(5)` con el pin en alto, y eso **condiciona como se implementa cualquier veto** · **(d)** ~~la via del menu esta muerta: `menu.cpp` cuelga de `botonAceptar()`, que devuelve `false` siempre~~ *(12/09, unas horas despues de escribirlo: **lo cierra `D-30`**, que retira `menu.cpp` entero. Se deja escrito porque es la MEDIDA que hacia inofensiva esa via, y porque hasta que `D-30` se construya el fichero sigue compilando)* | `D-13`, `D-25`, `SFTY-29` · §6.7 | `Maestro/src/modo_inteligente.cpp`, los dos `botones.cpp`, `Esclavo/src/demanda.cpp`, `menu.cpp` | `grep -rn demanda_hayLocal 01_Firmware`: **un solo llamador de codigo**, `Maestro/src/modo_inteligente.cpp:222`; `delay(5)` en `Maestro/src/botones.cpp:116` y `Esclavo/src/botones.cpp:129` |
| 🆕 **1.28** | 🟡 **`A1` y `A2` DE LA V2 (§9), que no vivian en ninguna lista viva:** pantalla de **modo depuracion** con las tramas **en crudo** y las rechazadas **con su motivo**, y un **registro descargable** de esas tramas. `registro_enlace.js` (`N-108`) ya guarda los **cortes**; falta el **contenido**. ⚠️ **Sin esto, un intermitente en calle no se diagnostica** —es la misma carencia que dejo sin resolver el `100/101` de 1.23— y `N-109` ya dijo que el equipo *«no guarda nada: cuando el tecnico llega, lo que paso ya no esta»* | `N-108`, `N-109`, `N-113` · §9 `A1`/`A2` | `App_Semaforo/` (`app.js` y sus copias), `registro_enlace.js` | §9, que **nunca se re-midio**: el barrido del 11/09 se declaro solo sobre «§0 a §6» |

**(2) NECESITA UNA DECISION DEL RESPONSABLE**

| | que | donde vive |
|---|---|---|
| ~~**2.1**~~ | ~~🔴 **CONFLICTO ABIERTO `J14`/`PB0` — `A-2` contra el codigo, y es de SEGURIDAD.**~~ 🟢 **CERRADO por `D-27` (11/09): `J14` LIBRE, sin cablear — el fin de carrera no se instala en este despliegue.** Lo medido sigue valiendo y es el motivo de no conectar nada: el firmware lee ese pin como **camara de demanda** (`CAM_DEMANDA_PIN`) en las dos puntas —en el **Esclavo**, `loop()` de `main.cpp` llama a `demanda_solicitar()` en cada flanco de subida → `CMD_DEMANDA` por radio **en cualquier modo**; en el **Maestro**, `modoInteligente_loop()` lo lee por NIVEL como presencia—. ~~Un fin de carrera cableado ahi hoy **mete demandas falsas con cada movimiento de la pluma**. **No se resuelve aqui, y hasta que se resuelva no se cablea nada en `J14`**~~ Vacio, `R64` lo deja en 0 V (`J14` medido en banco el 03-04/09, pasos 17-18). ⚠️ **Si algun dia se quiere el fin de carrera, vuelve a ser esta decision, con la lectura de `PB0` delante** | `D-27` · ~~`A-2` · §2~~ |
| 🆕 **2.1.bis** | 🟠 **`D-27` punto 3, lo que la alineacion dejo a la vista (11/09) — tres preguntas, ninguna cambia el firmware:** **(a) la ZONA.** El manual del modelo (`04_Manuales/..._CAMARAS_IA.md` §4 Pasos 1-2) dice *«carril de parada / zona donde el vehiculo espera»*; `D-13` dice **barrido de la pluma**, y `D-27` deroga de `D-13` solo *filtro, umbral, sensibilidad*. **Se aplico `D-13`** en el Manual 9, el manual del modelo y las guias. Si la intencion era cambiar tambien la zona, hace falta una fila nueva. **(b) el FILTRO.** Con `D-27` vuelve ☑ Vehiculo · ☐ Humano: no quita ninguna proteccion (ninguna camara protege la pluma, `escribirPines()`), pero **el contador de la fase 1 (`camara_vetosPluma()`) deja de contar personas bajo la pluma**, y ese contador es el dato que decide `A-1.bis`; si el veto se construye, el filtro pasa a ser seguridad. Que una moto cuente como «vehiculo»: `SIN VERIFICAR`. **(c) «la ficha».** El `.docx` `DS-2CD2683G2-IZS_Ficha_Tecnica_y_Configuracion.docx` es una recopilacion, no del fabricante (`04_Manuales/README.md`); la ficha oficial es el PDF `..._Datasheet_V5.5.113`. Ninguna de las dos fija umbral, sensibilidad ni filtro, asi que no cambia ningun valor, pero conviene decir cual es | `D-27`, `D-13`, `A-1.bis` · Manual 9 §4 Paso 3 |
| ~~🆕 **2.1.ter**~~ | ~~🔴 **`05_Funcional/Camaras_Sisga_4x.html` contradice `D-27` en el objetivo** —paso 07, el recuadro «Y estos pasos cambian» y la nota de fuentes: *«Objetivo: no filtrar (vehiculo y persona)»*—. **La guia esta caducada en ese punto** (regla del recuadro 🔒 de arriba); el 11/09 solo se toco su `J14`, por encargo. Hay que corregirla hacia `D-27` antes de volver a mandarla al instalador~~ 🟢 **HECHO el 11/09** *(sin comitear al escribir esto)*: **la guia se corrigio HACIA `D-27`** —objetivo **solo Vehiculo** (☑ Vehiculo · ☐ Humano si la casilla existe; si no existe, se anota, se avisa y se sube el minimo del `Size Filter`)— en el paso 07, en «Y estos pasos cambian», en el recuadro de quien ya configuro el 10/09 y en la nota de fuentes, con una linea nueva en la fe de erratas. Por consecuencia, los pasos 09 y 11 prueban con **un vehiculo** (una persona ya no deberia disparar) y «la cuenta» es de vehiculos. Zona, umbral y sensibilidad siguen en los de `D-13` (barrido de la pluma, minimo, alta). Lo caducado, tachado con «D-27, 11/09». ⚠️ **Medido al corregirla: la guia del 10/09 (`86683e8`) ya decia ☑ Vehiculo · ☐ Persona** —en eso estaba bien; fue la corregida del 11/09 la que lo invirtio—, asi que quien configuro el 10/09 **no toca el objetivo**, pero **si** el umbral (`1 s`) y la sensibilidad (`50`), que el recuadro no le pedia revisar: se anadieron. Cierra solo el choque con `D-27`; **volver a mandarla al instalador sigue siendo decision del responsable** | `D-27` · la guia |
| **2.2** | 🔴 **`A-16` — el sitio de `D-22` en la cola.** Este fichero la da a la vez como «opcional y la ultima» y como «va sola y va PRIMERO» (§3.4.quater), y su fila dice que `Y1` «pasa a decidir los 29 s». **No se decide aqui** | `A-16` |
| **2.3** | 🔴 **`A-1.bis` — ¿se deroga SFTY-28 para el veto de la pluma?** Es la unica forma de que alguna camara proteja la pluma, tambien con `D-25` | `A-1.bis` · §6.8 |
| **2.4** | 🟠 **`D-14`: cual de `J9`/`J11`/`J13` se gasta, para siempre**, y **`D-d`: los otros dos** —cabeza peatonal, zumbador o «no poblados»— | §3.11.ter · `D-d` |
| **2.5** | 🟠 **«Pluma ARRIBA en `S_FALLO`» a la tabla vinculante** —hoy vive en un comentario de `semaforo.cpp` y en el Manual 1— | §3.16-F |
| **2.6** | 🟠 `D-e` (queda un tercer reparto de camaras en `05_Funcional/README.md`) · `D-f` `N-129` · `D-g` `N-132` · `D-h` `N-113` · `D-i` `MATRIC` · `FW-N53` · ~~`A-0`~~ *(12/09: sale de aqui, ver 1.29 — lo que falta ya no es decidir)* · ~~`A-8`~~ *(12/09: **puede cerrarse ya**, medido por la auditoria de camaras: 24x7 en los dos `Arming Schedule` es la unica configuracion que no depende del reloj de la camara, y asi esta ya en los cuatro documentos)* · `A-4` · si y cuando el checksum de la subida (1.6) | §2 |
| 🆕 **1.29** | 🔴 **LAS CUATRO microSD ESTAN COMPRADAS Y NADIE LAS CONFIGURA — un instalador que siga la guia deja las cuatro VACIAS.** `A-0` estaba en el grupo (2) como «falta decidir la configuracion»; medido el 12/09, lo que falta **no es decidir**: la guia que va al poste **no menciona la microSD ni una sola vez** (`grep -ci` = 0) y su paso 07 marca en `Linkage Method` **solo la salida de alarma**. El manual del modelo exige **DOS casillas y las dos faltan del procedimiento**: **`Trigger Recording`** en la misma regla de intrusion **y** un `Record Schedule` de tipo **`Event`** —*«Before You Start: select Trigger Recording in event settings for each record type except Continuous»*, impresa 36—. **Con una sola de las dos no se graba nada**, y la tarjeta hay que formatearla desde la camara antes (impresa 32). ⚠️ **Lo que SI sigue siendo decision del responsable** —y por eso `A-0` no se cierra— **son dias de retencion y continua-o-por-evento**; la capacidad ya no (64 GB, compradas). ⚠️ Y **sin verificar si son `high endurance`**, que importa mucho mas con grabacion continua | `A-0`, `D-27` | los dos `.html` de campo y el Manual 9 · **cero firmware** | el responsable, 12/09: «ya estan compradas, con SD de 64 GB, son 4 ya» · la auditoria de camaras del 12/09 |
| **2.7** | 🟡 **Texto de `DECISIONES.md` que pide su mirada:** `D-21` se cerro sobre *«el Maestro reenvia la hora periodicamente, tendrian que pasar MESES»*, ~~y **al Maestro no lo resiembra nadie** mientras 1.1 no entre (§3.10.bis, pregunta 1)~~ *(1.1 entro en `68dd2c5`: su ESP32 lo resiembra cada 300 s)*. 🔴 **Y la premisa de los «MESES» —la de `D-21` y la de `D-23`— la TUMBA LA MEDIDA el 11/09 (`N-162` `H1`/`H3`):** con el `J17` mudo no hay siembra, la hora corre sobre el HSI a **36–90 s por hora**, y el presupuesto que el cruce aguanta de desfase entre DS3231 es de **11 s** → **11 s / 90 s/h ≈ 7 min; 11 s / 36 s/h ≈ 18 min**. Minutos, no meses. La consecuencia de construccion ya esta en marcha (fila 1.13); lo que queda para el responsable es **el texto de las dos filas**. No se toco: son suyas | `D-21`, `D-23` |
| 🆕 **2.8** | 🟠 **La alarma por DISCREPANCIA entre los dos `DS3231`** (una `HORA_ESP32` IGNORADA porque manda la radio, comparada con la hora de radio, por encima de un umbral). Va en buena direccion (`H3`), **pero el umbral no puede ser 11 s a secas**: entre dos siembras el HSI de cada punta mete hasta **7,5 s** (300 s × 25.000 ppm), o sea hasta 2 × 7,5 s de ruido mas el truncado, y con 11 s saltaria sola. **La cura de raiz es la «cadena completa» que `D-26` aparca** —el STM32 Esclavo devuelve la hora de radio a su ESP32—. Hay que decidir si se construye la alarma, con que umbral, o la cadena | `D-26` («mejora, no condicion»), `N-162` `H3` |
| ~~🆕 **2.9**~~ | ~~🟠 **`G3` del arnes de dos puntas: la punta en verde tarda en soltar frente a un `S_FALLO`**~~ 🟢 **DECIDIDO Y CONSTRUIDO el 12/09 (`N-163`, `b24578c`) — la compuerta baja de DOS rojos a UNO.** El responsable puso la condicion, y paso a ser el criterio de aceptacion: *«lo que no puede ser es que por microcortes por mala senal de radio… cada nada el esclavo se pasa a ambar»*. **El umbral de 25 s NO se toca** —lo demuestra un `static_assert`: `LATIDO_MS + 5·TIMEOUT_ACK_MS + TIMEOUT_ACK_MS <= SFTY6_SILENCIO_MS`, 20500 ≤ 21500, o sea que soltar el verde antes **no recorta el presupuesto de reintentos de `N-71`**—; lo que cambia es **cuando** se suelta, a `SFTY6_SILENCIO_MS − TIMEOUT_ACK_MS`. **Criterio medido caso por caso sobre 32 cortes** de 3 a 24,95 s en las dos direcciones: ambares del Esclavo **8 → 8**, del Maestro **10 → 10**. Ventana **250 ms → 0**; `dos_puntas` **106/106**. 🔴 **Y la implementacion OBVIA rompia el criterio:** salir por `C_ESPERANDO_ACK_RED` **suprime el latido** (`SFTY-13`) y el Esclavo se va a ambar **16 veces en vez de 8** — descartada, y guardada como inyeccion. ⚠️ **Tres correcciones de premisa:** el desfase **no es un viaje de radio** —200 de los 250 ms son la cortesia de `SFTY-17`—; hay **TRES** puertas al verde propio, no una, mas una cuarta en el Degradado que **no se toca a proposito** (`SFTY-21`); y soltar el verde sin mas dejaria el cruce en todo-rojo **hasta 15 min** con el ciclo maximo, de ahi la reanudacion. **Flash +244 B: 89,8 %, quedan 6.656.** ⚠️ **Sin banco y sin tarjeta** | `SFTY-6`, `N-163` |
| ~~🆕 **2.10**~~ | 🟢 **CERRADA ENTERA EL 12/09. La cadencia esta CONSTRUIDA (`dca17cd`) y lo que quedaba del plazo lo decidio el responsable ese dia (`D-28`).** Tres cosas que esta fila NO sabia y se apuntan aqui porque son la leccion, no la cronica: **(1) su derivacion del plazo era una TAUTOLOGIA** —el tiempo en que el HSI acumula la deriva de UNA cadencia *es* la cadencia; los 20 s de holgura que veia a 300 s salian del redondeo `ceil(7,5)->8`—, asi que **a 120 s habria dado plazo = cadencia, margen CERO**: cambiar solo el numero, que es lo que esta fila pedia, habria metido el defecto. El plazo pasa a derivarse del **relevo**. **(2) Su relevo de «265 s» estaba mal: son 271 s** —aplica la inflacion del HSI al caso (a) y se le olvida en el (b)—; no cambia el veredicto. **(3) `D-28` lo lleva mas alla:** el responsable eligio que el plazo cubra **DOS** siembras perdidas y no una, o sea `3C` inflado = 369 s -> plazo **400 s**, y el margen de los dos `DS3231` **baja de 13 a 7 s** sobre un aguante de 29 (medido con `_aguante()` de `esp32_13` y `_relativa_s()` de `reloj_04`, no a mano). **EN CONSTRUCCION el 12/09**, fila **1.18**. ~~Lo que la motivo, medido~~ *(lo de abajo se conserva porque es como se hallo; sus cifras son de la cadencia vieja)* — medido al construirla el 11/09 por la noche. El plazo de caducidad sale de la cadencia: `HORA_CADUCA_MS` = deriva de UNA cadencia al HSI peor = **320 s** frente a una cadencia de 300 s. Consecuencias: **(a)** en Degradado, **una sola siembra perdida o tardia** (20 s de holgura) manda la punta a ambar, y **no vuelve sola**; tolerar una siembra perdida con 5 min no cabe (32 s de separacion contra 29); **(b)** el relevo de `D-26` (3): al callarse la radio, la hora del Esclavo puede tener hasta 300 + 25 + 300 = 625 s antes de la primera siembra de su ESP32 — **~52 % de las caidas de radio** (fases al azar: P(a+b > 295 s), a y b uniformes en [0, 300]; rehecho a mano) el Esclavo **no puede entrar en Degradado** durante esos minutos o se rinde si ya estaba. No da verde-verde; da un cruce en ambar. **Propuesta con la medida:** bajar la cadencia a **~2 min** (120 s): plazo ~280 s, cabe una siembra perdida (246 s), el relevo cabe (2x120+25 = 265 s), y el margen de los dos `DS3231` **sube** de 11 a 13 s. Coste: la hora por `J17` y por radio 2,5 veces mas a menudo. **Cambia el numero de una decision del responsable: es SUYA** | `D-26` (2), `D-21` (1) | `ESP32_Expansion/include/contrato.h` (`SIEMBRA_INTERVALO_MS`), `{Maestro,Esclavo}/include/reloj.h` (el plazo tendria que derivarse tambien del relevo: `2C + SFTY6`), `reloj_04`, `esp32_13` | `HORA_CADUCA_MS` 320000 contra `SIEMBRA_INTERVALO_MS` 300000UL; bloque F del Degradado a dos puntas |
| 🆕 **2.11** | 🔴 **`H7` — `SET_RTC` CON UN PIN FALSO PONE LA HORA.** El puente no conoce el PIN y el STM32 ya no ve la linea, asi que **no hay quien lo rechace**. Esta **abierto POR DECISION** (`D-26` (1)) y por eso no era un defecto — pero **no figuraba en ninguna lista**, y una excepcion que nadie cuenta es indistinguible de un olvido (`CLAUDE.md` §6). Lo que se pide aqui no es construir: es **decir si se queda asi y por que**, o darle una fila a su cierre. Cuelga del mismo sitio que 1.20: cuatro ordenes que tampoco piden PIN | `D-26` (1), `N-109` §5 · §3.16 `H7` |
| 🆕 **2.12** | 🟠 **EL AVISO A LA CAJA NEGRA — la opcion `B` de §6.7, y es una decision de construir o no.** §6.7 la mide como *«la unica que toca firmware sin poder degradar nada»* y, mas importante, **es el INSTRUMENTO que el laboratorio de camara necesita: cuenta cuantas veces habria actuado un veto ANTES de darle autoridad.** Coste **+60 B**, medido compilando. La opcion `A` es `SFTY-29` y ya esta en 2.3; `C` y `D` estan descartadas en su apartado. ⚠️ **Va antes que 2.3, no despues:** decidir el veto sin haber contado nunca cuantas veces habria disparado es decidir sin dato | `A-1.bis`, `SFTY-29`, `D-13` · §6.7 |

**(3) NECESITA COBRE, BANCO O CAMPO** *(ningun PC lo hace)*

| | que | donde vive |
|---|---|---|
| 🆕 **3.9** | 🔴 **`AB-3` — NADIE HA MEDIDO CUANTO TARDA EL ESP32 DESDE EL RESET HASTA VOLVER A PASAR BYTES, y de ese numero cuelga el techo del perro.** `ESP32_ARRANQUE_MS` vale `1500UL` con `ESP32_ARRANQUE_MEDIDO 0` —*«0 = SIN VERIFICAR (AB-3)»*, `contrato.h:120-121`— y el propio fichero avisa: *«EL TECHO DESCANSA SOBRE UN NUMERO SIN MEDIR»*. §6.4 lo dice con todas las letras: *«Ese es el cierre de `AB-3`, **y sigue abierto**»*. ⚠️ **NO es lo mismo que 3.2:** los reinicios del Sisga son `OTRO_PERRO` (`ESP_RST_WDT`) y §6.4 los separa —*«Mismo sintoma de superficie, otro mecanismo»*—, **pero se miden en la misma sesion y con el mismo USB-TTL**, asi que van juntos al banco | §6.4 · `ESP32_Expansion/include/contrato.h` |
| | ↑ **lo que puede herir** · ↓ **el resto** | |
| **3.1** | 🔴 **El Sisga:** la **cinta del Esclavo**; **avisar al instalador** de que la guia sigue RETIRADA por sus afirmaciones de seguridad —las conexiones si quedan, `D-25`—; **no usar el Degradado con `7ff7d12`**; y el siguiente paquete **carga las tres tarjetas**, porque campo y `main` difieren en el ESP32 | §3.16 |
| **3.2** | 🔴 **Los reinicios del ESP32 del Sisga** —USB-TTL en `TX0` a 115200, osciloscopio en 3V3 y `EN`, fuente de 5 V buena— y **la fuente `A5`, sin pedir** | §3.16 · §4 C-3 |
| **3.3** | 🔴 **La sesion de banco de lo POR VALIDAR** (§5), **incluido el Degradado del poste 2, que desde `D-20` ya se puede probar** | §5 |
| **3.4** | 🟠 **`T-2`: la V8.4 con `SFTY6_SILENCIO_MS = 25000UL`** — no existe en git ningun commit que sea `e303485` mas solo esa constante: sus descendientes (`git log --all --ancestry-path e303485..`) llevan la V9 entera. Construirla es teclado; lo que la hace valer es la calle | §1 · §6.3 |
| **3.5** | 🟠 **`N-116`: la escalera sobre la Maestro de la sesion 1**, que es otra placa que la `179DB0` | §6.1 |
| **3.6** | 🟠 **`D-22`/`Y1`: su carga, con una tarjeta delante**, cuando `A-16` diga en que sitio | `A-16` |
| **3.7** | 🟠 **La firma del funcional sobre el manual del doble** (`D-19`) y **la app en un telefono** (`N-110` B1/B2 y lo de §5) | §2 `D-a` · §5 |
| **3.8** | 🟡 `0x68` sobre el modulo del **Esclavo** · `Y2` de la segunda tarjeta (`BLQ-2`) · los `SIN VERIFICAR` de `17_` · el `2K2` · el LED `D21` (`A-10`) · **la talanquera real por rele a `OPEN`** (`D-25`) · compras `C-1`, `C-4`, `C-5` | §1 · §4 |
| 🆕 **3.10** | 🔴 **LAS ETIQUETAS DE LAS DOS CABEZAS PEATONALES VAN AL REVES QUE LOS CONECTORES** (§3.7): trazado sobre el `.kicad_pcb`, **`/S7` es `PA6` → `J11` (rojo peaton)** y **`/S8` es `PA7` → `J9` (verde peaton)**, verificado ademas por pads de `U1` (16 = `/S7`, 17 = `/S8`). **Quien cablee guiandose por el numero de la senal invierte rojo y verde de peatones.** Hoy no explota porque esos canales **no tienen una linea de firmware detras** (`D-d`, fila 2.4), y por eso es exactamente la trampa que espera al primero que enchufe una cabeza peatonal. **Los documentos lo tienen bien: engana la PLACA** | §3.7 · `D-d` · 2.4 |
| 🆕 **3.11** | 🔴 **EL LABORATORIO DE LA CAMARA, que es precondicion de 2.3 y de 2.12 y no tenia fila.** El responsable lo dejo escrito: *«hay que hacer un laboratorio… no se como se comporta esa camara. No es una camara asi super fiable, ni super rapida»*, y §6.8 cierra con **«Nada de esto se decide sin esa medida»**. De ella cuelgan el veto de la pluma (`A-1.bis`, 2.3), **cuantos segundos como maximo**, y si la lectura es **en el instante del cambio o veto continuo** | §6.8 · `A-1.bis` · 2.3, 2.12 |
| 🆕 **3.12** | 🟠 **LO QUE FALTA MEDIR DEL SISGA Y DE LAS CAMARAS, que 3.1 no cubre** — **(a)** §3.8: la analitica dio *«0 V sin deteccion, 3,3 V con deteccion»* en el borne, pero es **UNA** camara y **medida en el borne, no en una trama**: `CAM:?` sigue en toda la cinta de las 12:17 · **(b)** §3.16: falta **la cinta de la prueba de MANUAL** —*«lo de "Manual no cambia" no esta en esta medida»*— · **(c)** §3.11.ter: la configurabilidad **`NO`/`NC` de la SALIDA** del Manual 9 sigue 🔴 `SIN VERIFICAR` (la fila 3.8 solo cubre los `SIN VERIFICAR` de `17_`, que es otro documento) · **(d)** §3.16, dos ⚪ **SIN FUENTE**: la «especificacion de compra con muelle» que citan cuatro documentos **no esta** en `15_Lista_de_Compras`, y los angulos 15–20°/35–45° y las alturas **solo salen en las dos guias, que discrepan entre si** | §3.8, §3.16, §3.11.ter |
| 🆕 **3.13** | 🟠 **`D30` ES UN `1N4148` —200 mA— HACIENDO DE DIODO DE RUEDA LIBRE DE UNA SALIDA GOBERNADA POR UN `IRLZ44N`: infradimensionado en dos ordenes de magnitud** (§6.1). Va a la V2, y §9 **no lo listaba**, asi que no vivia en ninguna lista · y el aislamiento del `TLP127` con la red `GND` unica: *«**Si la conclusion de este parrafo aguanta con masa comun no se ha vuelto a medir**»*. 🛑 **Sigue en pie mientras tanto: no reenergizar «a ver si pasa»** | §6.1 · `N-116` · V2 |
| 🆕 **3.14** | 🟡 **`BLQ-4` — el anuncio Bluetooth del ESP32** (§3.14), que no tenia fila. *(⚠️ **`BLQ-5` NO se sube: ya esta**, y es el punto 2 de la cabecera de este mismo §0 —«las entradas de campo van desnudas al die y `J16` p1 lleva 12 V crudos»—. Lo que si faltaba de el es **escribirlo en la guia de instalacion**, y eso es 1.24 (f))* | §3.14 · `D-i` `MATRIC` (2.6) |

> ~~🔴 **EL CONFLICTO DE `J14`, dicho otra vez fuera de la tabla porque se ejecuta con un destornillador:**
> `A-2` dice *«el fin de carrera va a `J14`/`PB0`»* y el fuente de `b79d904` lee `PB0` como camara
> de demanda en las dos puntas … **Hasta que el responsable elija** (desarmar esa lectura,
> mover el fin de carrera, u otra), **en `J14` no se cablea nada**. No se ha tocado `A-2`.~~
> 🟢 **ELEGIDO el 11/09 (`D-27`): `J14` LIBRE y sin cablear, el fin de carrera no se instala.** Lo
> que se ejecuta con el destornillador no cambia: **en `J14` no se conecta nada**, porque el fuente
> sigue leyendo `PB0` como demanda (`digitalRead(CAM_DEMANDA_PIN)` en `Esclavo/src/main.cpp`,
> `camara_leerPin(CAM_DEMANDA_PIN)` en `modoInteligente_loop()` del Maestro).

**La instalacion certificada es `e303485`, del 31 de julio.** ~~Ninguno de los commits posteriores ha
entrado en un poste~~ — 🔴 **FALSO desde el 10/09: un Maestro (`SERIE:179DB0`) corrio en El Sisga con
firmware V9** —el paquete del 08/09, `7ff7d12`; despues se probo el del 10/09, `b354fe9`—. Es
el primer contacto de V9 con una calle, y su evidencia en el repositorio ~~es **UNA trama transcrita a
mano**~~ **son la cinta y el diario del Maestro**, exportados por la app y copiados a `evidencia/`
(`e7555f3`): `2026-09-10_Sisga_179DB0_cinta_tramas.txt` (286 tramas con checksum) y
`…_diario_ordenes.txt` (22), **308 en total, todas casan**. Falta la del Esclavo. §3.16.

1. ~~🛑 **La tarjeta Maestro sigue muerta** y la escalera de diagnostico —cuyo primer peldano es
   **gratis**— no consta recorrida. **De esto cuelga todo lo demas.**~~ — 🔴 **FALSO en su segunda
   mitad, medido el 11/09:** la tarjeta muerta es **la Maestro de la sesion 1 del banco**, y **no
   cuelga de ella todo lo demas**: el 04/09 se reprogramo **otra placa** como Maestro
   (`roadmap_hist.md` `N-126`: *«la Maestro de la sesion 1 sigue con el corto y se descarto entera»*),
   que se anuncio `SEM-179DB0-M` —la serie sale del UID del STM32, `identidad.cpp`— y **es la
   `SERIE:179DB0` que corrio V9 en el Sisga**. Lo que si sigue en pie es que la escalera de `N-116`
   no consta recorrida, y **de ella cuelga recuperar esa placa**, no ejercer firmware en cobre.
2. 🔴 **Las entradas de campo van desnudas al die** y `J16` p1 lleva 12 V crudos.
3. ~~🔴 **El `$ALARM` no cabe en su buffer**~~ — 🟢 **CERRADO el 08/09** (§3.13). Y de paso quedo
   medido que **el sintoma publicado era falso**: el CRC casaba, y lo que se perdia era la HORA.
4. ~~🔴 **`CAM_CIEGA` sigue en 6 h.**~~ — 🟢 **CERRADO ENTERO el 08/09** (`D-24`, §3.15): 24 h de
   paso abierto **y** el aviso del Modo Inteligente. **Ya no queda nada de esta.**
5. 🟡 **La firma del funcional sobre el manual del «doble» no existe** — y `TECHO_POR_SUELO = 2`
   **ya salio en el paquete del 05/09**.

> ~~🔴 **Y desde el 07/09 hay una SEXTA, que puede ser la mas cara de todas y se contesta en diez
> minutos sin tarjeta ni cable: `N-159` — la analitica de las camaras COMPRADAS probablemente NO
> puede accionar el rele.** Ver §3.8. Si no puede, **el camino de `J16` no sirve** y la demanda
> entra por otro diseno.~~ — 🟢 **CONTESTADA EN CAMPO el 10/09, y en positivo** (§3.8): una camara del
> Maestro con `Intrusion Detection` + `Trigger Alarm Output` dio *«0 V sin deteccion, 3,3 V con
> deteccion»* en el borne. **El camino de `J16` sirve.** Es una camara, medida en el borne y no en una
> trama.

> 🔴 **Y una SEPTIMA, del 07/09 por la noche y de otra naturaleza: `N-160` — el `20/20` que se compro
> con COMENTARIOS.** Tres decisiones se anclaron sin construirse, una APK se «recompilo» siendo el
> mismo `SHA-256` de la anterior, y al auditar el diff aparecieron **TRES** defectos vivos en el
> reloj, los tres arreglados: el retorno que no dependia de la llamada, el contador que apagaba los
> centinelas del respaldo, y **la hora que SALTA en silencio si `Y2` arranca tarde** —el cristal se
> adopta a los 30 s y los getters cambian de fuente a un RTC que nunca se sembro, con `horaValida`
> todavia en `true`, y en el Maestro eso se propaga por radio—. **Lo unico que los caza es revisar el
> DIFF aunque el numero salga verde.** Ver §3.10 y §3.10.bis.
>
> 🔴 **Y de ahi salieron DOS PREGUNTAS QUE MANDAN SOBRE EL ORDEN DE CONSTRUCCION, y las dos son del
> responsable:** (1) ~~**la siembra periodica del `DS3231` al STM32 NO EXISTE** —el ESP32 reenvia
> verbatim, no siembra—~~ *(cierto el 07/09; 🟢 **existe en `main` desde `68dd2c5`**, 11/09, §0 fila 1.1)*, y sobre esa premisa se degrado `D-22` a opcional y se cerro `D-21`; sin ella
> el Maestro corre con `millis()` desde la ultima visita humana y **da la vuelta a los 49,7 dias**.
> (2) **la guarda `D-21` del Esclavo esta construida y no puede dispararse**, porque nada devuelve
> `horaValida` a `false`. **Hasta contestarlas, el paso 4 del orden de construccion no se puede
> ordenar honestamente.** §3.10.bis.
>
> ✅ **CONTESTADAS EN PARTE esa misma noche (§3.11): la siembra periodica SE CONSTRUYE y `D-21` tiene
> su forma cerrada.** Lo que quedo abierto son ~~**`A-15`** —el numero de la cadencia— y sobre todo~~
> *(`A-15` se cerro el 08/09: ~~**una hora**, reusando `INTERVALO_SYNC_MS`~~ → **`D-26` (11/09): cada ~5 min**; ~~la siembra esta EN
> CONSTRUCCION el 11/09, fuera de `main`~~ → 🟢 **la siembra esta en `main` desde `68dd2c5`** (11/09 por la tarde:
> `siembra.cpp`, `SIEMBRA_INTERVALO_MS = 300000UL`; §0 fila 1.1))* **`A-16`**: `D-22` se contradice a si misma, y de eso
> depende que el Degradado sea seguro. **Sigue abierta.**

> 🔴 **Y la OCTAVA, del 11/09, que va DELANTE de todas porque lo que falta es preguntar y no cuesta
> nada: `N-162`, El Sisga (§3.16).** (1) ~~el hash del firmware~~ — contestado: `7ff7d12` cargado,
> `b354fe9` probado despues; la cinta y el diario **del Maestro** estan ya en `evidencia/` (308 tramas,
> 308 checksums que casan) y **falta la del Esclavo**; (2) **avisar al instalador**: la guia de 4
> camaras le llego **por WhatsApp** el 10/09 a las 15:32 y esta RETIRADA por dos afirmaciones de
> seguridad falsas; (3) la rama que salio de alli se valido por el DIFF: **se quedan** los getters del reloj y
> las cabeceras de la app, **se revierte** el sello del puente y **se retiran** los textos de DAR PASO.
> Y la auditoria destapo **`AMBAR_EMERGENCIA` sin PIN, que no avisa al Maestro** —el candidato mas
> firme para el DAR PASO del Sisga; ~~sin construir~~ 🟢 **construido el 12/09 en `913c29c`**, §0 fila 1.2,
> **sin que eso confirme la causa del Sisga**— y **la siembra de `A-15`, decidida y sin construir** *(~~11/09: EN
> CONSTRUCCION en un worktree; en `main` sigue sin una linea~~ → 🟢 **en `main` desde `68dd2c5`**, 11/09 por la
> tarde, con la cadencia de `D-26`; lo que deja abierto, §3.16 `N-162` y §0 filas 1.13–1.15)*.
>
> 🎯 **Y el 11/09 el responsable decidio las conexiones del Sisga como definitivas: `D-25`** —cuatro
> camaras, dos por poste, `J16` p9/p10 y p11/p12; talanquera en `J15` por rele a `OPEN`—. Deroga de
> `D-13` solo *«una camara por poste»*. §3.16.

> ~~**Por donde se empieza manana:** por el **peldano gratis** de N-116 —desenchufar `J14`, `J15`,
> `J16`, `J17` y `J2` y remedir el riel de 3,3 V—. Cuesta cinco minutos y **decide si hay que
> fabricar placa**. Esta desarrollado en §6.1.~~ *(11/09: el peldano sigue siendo gratis, pero ya no
> es por donde se empieza: la placa que decide es la de la sesion 1, y hay otra —`179DB0`— que
> funciona. Lo que va primero esta en la tabla de arriba.)*
>
> ~~**Y en paralelo, porque no compite por las manos:** el `ENSAYO 0` de §3.8 es **solo pantalla**.~~
> *(11/09: hecho en campo el 10/09 para una camara, §3.8.)*

---

## 1. Lo que necesita una TARJETA — no lo destraba nadie escribiendo

**Va primero porque es lo unico que no se puede sustituir.** `CLAUDE.md` §2.bis: *la pregunta antes
de escribir cualquier cosa es si acerca una tarjeta cargada o la sustituye.*

| | que | detalle |
|---|---|---|
| **T-1** | 🛑 **`N-116`: recorrer la escalera de 4 peldanos sobre la tarjeta Maestro** *(11/09: la de la **sesion 1**; la `179DB0` es otra placa y funciona, `N-126`)*. El primero —**desenchufar los cinco conectores y remedir el riel**— es **gratis, son cinco minutos, y puede cerrar el caso solo** | §6.1 |
| **T-2** | 🔴 **Subir `SFTY6_SILENCIO_MS = 25000UL` sobre `e303485`.** Solo esa constante, sobre la V8.4 que ya esta probada en la calle | §6.3 · **es lo unico que llega al conductor esta semana**, y no depende de la V9.0 · ⚠️ *11/09: los 25 s **si** tocaron una calle el 10/09, pero dentro de `7ff7d12` (V9 `SIN_BANCO`, Sisga); la V8.4 sigue en 12 s y en git no hay ningun `e303485` + solo esa constante* |
| **T-3** | 🔴 **La sesion de banco de** ~~**los siete POR VALIDAR** — `N-142`, `N-147`, `N-150`, `N-151`, `N-152`, `N-153` y `N-157`. **Los siete tocan el ambar, el Modo Manual o la camara**~~ **lo POR VALIDAR de §5**, que el 11/09 son bastantes mas de siete y ya no se cuentan desde la cinta del 05/09 sino desde la del 10/09 | §5 |
| ~~**T-4**~~ | ~~🟠 **Confirmar `N-117` sobre el modulo** con el monitor serie: el arranque del ESP32 cronometrado de verdad, `reset -> primer byte`~~ — 🟢 **el SINTOMA se cerro en banco el 04/09** (`roadmap_hist.md` `N-126`: el modulo se anuncia estable, `SEM-179DB0-M`). La causa ya no se puede discriminar con el arreglo dentro | §6.4 · 🔴 **lo que queda en esta superficie es OTRO sintoma**: los reinicios del ESP32 del Sisga (§3.16), que no son el perro de `N-117` |
| ~~**T-5**~~ | ~~🟠 **`0x68` del `DS3231` sobre el modulo real.** El reloj esta cerrado en cobre (`HORA:22:19:58` en la cinta del 05/09), pero la direccion I2C sigue `SIN VERIFICAR`~~ — 🟢 **verificada el 10/09 en el modulo del Maestro `179DB0`**: en la cinta del Sisga el puente, que habla con `DS3231_DIR 0x68`, contesta los `SET_RTC` con la hora releida y `LEER_RTC` la da avanzando (12:17:31 → 12:18:52) | falta el modulo del **Esclavo**; y el comentario de `contrato.h` sigue diciendo `SIN VERIFICAR` (firmware: no se toca desde aqui) |
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
| **D-a** | 🟡 **La firma del funcional sobre el manual del «doble»** — `TECHO_POR_SUELO = 2` esta en el firmware y **ya salio en el paquete**; el recuadro de firma de `05_Funcional/1_Manual_Usuario.md` esta **vacio campo por campo** y `evidencia/` no tiene ningun documento firmado | `D-19`, su condicion |
| **D-b** | 🔴 **`A-1.bis`: ¿se deroga SFTY-28 para el veto de la pluma?** Bloquea la fase 2 de `D-13`; la fase 1 ya esta construida | `A-1.bis` · §6.8 |
| **D-c** | 🔴 **Las dos pantallas** (04/09). ~~Es **la unica decision suya sin una sola linea de codigo detras**~~ *(11/09: no es la unica — `D-14`, `D-22` y `D-23` tampoco tienen codigo, `decisiones_01_anclas`; y su mitad Esclavo es `D-23`, decidida el 07/09 con la via `$EVENT`)* | §6.9 |
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
> con urgencia media, y su fila «Filas que chocan nº 2» describe un conflicto que ya no existe**
> —`A-11` se resolvio por app en `D-18`—. Ver §3.2.~~ *(11/09: el indice ya la da cerrada, y «Filas
> que chocan nº 2» se tacho hoy como resuelta.)* ~~🔴 **Pero su mitad de `J14` choca con el CODIGO**
> —el recuadro de arriba y la fila 2.1 de §0—: **eso si es una decision tuya, y abierta.**~~
> 🟢 *(11/09: la tomaste — `D-27`, `J14` libre; su mitad de `J14` queda sin efecto en este despliegue.)*

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

## 4. Lo que necesita una COMPRA o un SOLDADOR

| | que | cuando |
|---|---|---|
| **C-1** | 🟢 **`2K2` en serie en las 5 entradas de campo, al soldar la placa nueva** (`N-120`) | **es el momento y no vuelve.** Si se suelda igual, la placa nueva nace con la misma averia dentro |
| **C-2** | 🟢 **No poblar los 12 V de `J16` p1** — y mientras exista, **taparlo es obligatorio en cada equipo que se monte** (`D-4`) | al montar |
| **C-3** | 🔴 **`A5`: el reductor** ~~`LM2596`~~ **DC-DC conmutado desde la bateria** (12 -> 5 V, >= 1 A) para el ESP32 *(11/09: `15_Lista_de_Compras_Hardware.md` retiro el `LM2596` el 31/08 —«ninguna referencia concreta esta elegida»—: no se da por elegido aqui)* | no esta pedido · 🔴 **y ya es fila de CAMPO**: el ESP32 del Sisga se reinicio con 2 partes `SUBIDA_DE_TENSION` (`ESP_RST_POWERON`) entre los 5 del 10/09 (§3.16). Que su alimentacion fuera la causa es HIPOTESIS; que esta linea no esta pedida, no |
| **C-4** | 🔴 **Quien disena y quien fabrica la placa portadora** | bloquea **desplegar**, no **probar** |
| **C-5** | 🟠 **Las microSD de las camaras** (comprarlas ya esta decidido; falta como se configura la grabacion) | `A-0` |

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
| ~~**N-117**~~ | el perro del ESP32 se comia su propio arranque | ~~**arreglado en el arbol el 04/09; la causa NO esta confirmada sobre el modulo.**~~ 🟢 **el SINTOMA se cerro en banco el 04/09** (`roadmap_hist.md` `N-126`: *«CERRADO con evidencia en hardware»*, anuncio estable como `SEM-179DB0-M`). La causa ya no se puede discriminar con el arreglo dentro, y `ESP32_ARRANQUE_MEDIDO = 0` sigue en `contrato.h` (el responsable midio 2–3 s *energizado → primer dato en la app*, que no es la ventana del perro). **Los reinicios del Sisga son otro sintoma** (§3.16). §6.4 |
| **N-110** B2 | el teclado del PIN acepta pulsaciones con el modal cerrado | ~~**no se comprueba leyendo.**~~ 🟢 **construido (`b033f0b`, `tecladoPinAbierto()`) y ejercido en el DOM** por `test_dom_execution.js`; **falta el telefono**. B1 si esta cerrado: `NMEAParser.validarTrama(linea)` tiene llamador vivo en `app.js` (`dad4615`). §6.6 |
| **la app entera** | `disconnect()` existe (6 llamadas) · `padding-bottom: 90px` puesto · `parseInt(...)\|\|0` solo en comentarios que narran su retirada · `discoverUnpaired` usado (7) · `validarTrama()` con llamador | **los cinco arreglados en el fuente y NINGUNO ejercido en un telefono** |
| **`TECHO_POR_SUELO = 2`** | el Modo Inteligente puede alargar una fase **hasta el DOBLE** del tiempo configurado | **aprobado CON CONDICION —«si un funcional revisa el manual y este manual es claro»— y la firma NO EXISTE. Y ya salio en el paquete del 05/09** |
| **el `0x68`** del `DS3231` | | ~~`SIN VERIFICAR` sobre el modulo~~ 🟢 **verificada en el modulo del Maestro `179DB0`** por la cinta del Sisga (`SET_RTC`/`LEER_RTC` contestan y la hora avanza); **falta el del Esclavo** |
| **las** ~~21~~ **`SIN VERIFICAR`** de `17_` | | por definicion *(la cifra se retira el 11/09: no se reproduce)* |

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
