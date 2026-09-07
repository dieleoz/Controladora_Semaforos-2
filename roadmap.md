# Roadmap — Controladora de Semaforos Moviles de 3 Estados (V9.0)

**Este fichero lleva SOLO lo pendiente y lo que falta validar.** El 07/09/2026 se le sacaron 4.631
lineas de cosas ya cerradas: estan **integras y con su evidencia** en
[`roadmap_hist.md`](roadmap_hist.md), que lleva indice por `N-x` y por apartado. **No se borro
nada** — la cuenta esta publicada al final de aquel fichero.

> **Como se lee.** Arriba, **lo que se puede hacer y quien lo desbloquea**. Abajo, **el porque** de
> lo que sigue abierto, con su medida al lado. Si algo no esta aqui, o esta hecho o esta en el
> historico.

## Que manda sobre este fichero, y en que orden

| | |
|---|---|
| **la decision vigente** | [`DECISIONES.md`](DECISIONES.md) — **una fila suya gana a cualquier parrafo de aqui** |
| **el hardware medido** | `05_Funcional/17_Arquitectura_28-08_y_Decisiones_Abiertas.md` — gana a `DECISIONES.md` y a `CLAUDE.md` en cobre, pines y conectores |
| **las reglas permanentes** | [`CLAUDE.md`](CLAUDE.md) |
| **el estado de hoy** | [`ESTADO.md`](ESTADO.md) |
| **el porque de lo cerrado** | [`roadmap_hist.md`](roadmap_hist.md) |

## Las cifras NO se copian aqui

**En campo corre `V8.4`, commit `e303485` (31/07/2026).** En el repositorio, `V9.0` en `main-nuevo`.

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

**En la calle corre `e303485`, del 31 de julio.** Desde entonces hay 269 commits y 42 tocan el
firmware de las dos puntas. **Ninguno ha entrado en un poste.** Lo ultimo que toco una tarjeta fue
la cinta del 05/09 a las 22:19.

1. 🛑 **La tarjeta Maestro sigue muerta** y la escalera de diagnostico —cuyo primer peldano es
   **gratis**— no consta recorrida. **De esto cuelga todo lo demas.**
2. 🔴 **Las entradas de campo van desnudas al die** y `J16` p1 lleva 12 V crudos.
3. 🔴 **El `$ALARM` no cabe en su buffer** —158 B en el Maestro y 171 en el Esclavo contra 143— y
   **trunca sin decirlo**.
4. 🔴 **`CAM_CIEGA` sigue en 6 h.** Se decidieron **4 dias** el 05/09 y no se ha implementado.
5. 🟡 **La firma del funcional sobre el manual del «doble» no existe** — y `TECHO_POR_SUELO = 2`
   **ya salio en el paquete del 05/09**.

> 🔴 **Y desde el 07/09 hay una SEXTA, que puede ser la mas cara de todas y se contesta en diez
> minutos sin tarjeta ni cable: `N-159` — la analitica de las camaras COMPRADAS probablemente NO
> puede accionar el rele.** Ver §3.8. Si no puede, **el camino de `J16` no sirve** y la demanda
> entra por otro diseno.

> **Por donde se empieza manana:** por el **peldano gratis** de N-116 —desenchufar `J14`, `J15`,
> `J16`, `J17` y `J2` y remedir el riel de 3,3 V—. Cuesta cinco minutos y **decide si hay que
> fabricar placa**. Esta desarrollado en §6.1.
>
> **Y en paralelo, porque no compite por las manos:** el `ENSAYO 0` de §3.8 es **solo pantalla**.

---

## 1. Lo que necesita una TARJETA — no lo destraba nadie escribiendo

**Va primero porque es lo unico que no se puede sustituir.** `CLAUDE.md` §2.bis: *la pregunta antes
de escribir cualquier cosa es si acerca una tarjeta cargada o la sustituye.*

| | que | detalle |
|---|---|---|
| **T-1** | 🛑 **`N-116`: recorrer la escalera de 4 peldanos sobre la tarjeta Maestro.** El primero —**desenchufar los cinco conectores y remedir el riel**— es **gratis, son cinco minutos, y puede cerrar el caso solo** | §6.1 |
| **T-2** | 🔴 **Subir `SFTY6_SILENCIO_MS = 25000UL` sobre `e303485`.** Solo esa constante, sobre la V8.4 que ya esta probada en la calle | §6.3 · **es lo unico que llega al conductor esta semana**, y no depende de la V9.0 |
| **T-3** | 🔴 **La sesion de banco de los siete POR VALIDAR** — `N-142`, `N-147`, `N-150`, `N-151`, `N-152`, `N-153` y `N-157`. **Los siete tocan el ambar, el Modo Manual o la camara** | §5 |
| **T-4** | 🟠 **Confirmar `N-117` sobre el modulo** con el monitor serie: el arranque del ESP32 cronometrado de verdad, `reset -> primer byte` | §6.4 · el arreglo esta en el arbol; **la causa no esta confirmada sobre el modulo** |
| **T-5** | 🟠 **`0x68` del `DS3231` sobre el modulo real.** El reloj esta cerrado en cobre (`HORA:22:19:58` en la cinta del 05/09), pero la direccion I2C sigue `SIN VERIFICAR` | |
| **T-6** | 🟠 **Las 21 `SIN VERIFICAR` de `05_Funcional/17_...md`.** Es la lista de lo que el proyecto declara sin haber medido | ese fichero, no este |
| **T-7** | 🟠 **`N-110` B2: que el teclado del PIN no acepte pulsaciones con el modal cerrado.** No se comprueba leyendo — hace falta ejercer el DOM | §6.6 |

> 🔴 **Y la carga va ANTES que el cable, no en el mismo commit** (`CLAUDE.md` §9.bis). Un commit no
> protege de un destornillador: **el firmware nuevo tiene que estar DENTRO de la tarjeta antes de
> que nadie enchufe nada en `J16`.** Se exige la carga verificada, no el merge.

---

## 2. Lo que necesita una DECISION del responsable

**Cada una tiene ya su fila en [`DECISIONES.md`](DECISIONES.md), que es donde manda.** Aqui van
**como punteros, no repetidas** — repetirlas es exactamente el problema que aquel fichero vino a
resolver.

| | que hay que decidir | donde vive |
|---|---|---|
| **D-a** | 🟡 **La firma del funcional sobre el manual del «doble»** — `TECHO_POR_SUELO = 2` esta en el firmware y **ya salio en el paquete**; el recuadro de firma de `05_Funcional/1_Manual_Usuario.md` esta **vacio campo por campo** y `evidencia/` no tiene ningun documento firmado | `D-19`, su condicion |
| **D-b** | 🔴 **`A-1.bis`: ¿se deroga SFTY-28 para el veto de la pluma?** Bloquea la fase 2 de `D-13`; la fase 1 ya esta construida | `A-1.bis` · §6.8 |
| **D-c** | 🔴 **Las dos pantallas** (04/09). Es **la unica decision suya sin una sola linea de codigo detras** | §6.9 |
| **D-d** | 🔴 **`J9` `VERDE_PEATON` · `J11` `ROJO_PEATON` · `J13` `BUZZER`: tres canales de potencia FABRICADOS, con su opto y su MOSFET, y cero firmware.** `grep digitalWrite` sobre los tres da **0**. **16 B de flash cada uno**, medidos por desensamblado. ¿Se gasta uno en una cabeza peatonal o en un zumbador, o se declaran **no poblados**? | `CLAUDE.md` §6 |
| **D-e** | 🟠 **La renumeracion de camaras.** `17_` §1.7 **ya decidio `C` y `D`** y lo blindo. **`README.md` publica hoy `p10 = Camara 2` / `p12 = Camara 1`**, que es lo contrario. **La spec gana** — lo que falta es que la confirmes o la derogues, y si se deroga, que pasa con la Camara 3 | §3.4 |
| **D-f** | 🟠 **`N-129`: el rotulo Bluetooth.** Un modulo virgen anuncia `SEM-SIN-MATRICULA` y **las dos puntas se llaman igual**; el nombre bueno solo entra en la SIGUIENTE arrancada. ¿Paso de puesta en marcha, o se cambia el momento del rotulado? **Nunca se ha visto en un telefono** | `MATRIC` |
| **D-g** | 🟠 **`N-132`: no hay puente H y sale UNA sola linea de control a la pluma.** El `L298N` decidido necesita dos | `N-132` · censado sobre el `.kicad_pcb` |
| **D-h** | 🟠 **`N-113`: el aviso remoto es COSTE RECURRENTE** —SIM o WiFi en el cruce—, no una linea de firmware | §6.5 |
| **D-i** | 🟠 **`MATRIC`: matriculacion por ID de Bluetooth.** `RF_Packet` son 4 bytes `{msgID, command, param, crc}` **sin campo de direccion**, y el CRC cubre 3: meterle direccionamiento **cambia el contrato de la radio en las dos puntas** | `MATRIC` |
| **D-j** | 🟠 **`A-0`, `A-7`, `A-8`, `A-10`, `A-4`, `A-13`** — grabacion de las microSD, el `Delay` real del rele, los dos `Arming Schedule`, el LED `D21` de `VERDE2`, que pasa con `MENU`, donde va el campo `CAM:` | `DECISIONES.md` |

> ⚠️ **`A-2` NO va en esta lista: la cerraste tu el 05/09.** `J16` p5/p8 son el mando con su codigo
> intacto (`D-1`) y el fin de carrera va a `J14`/`PB0`. **`DECISIONES.md` la sigue listando abierta
> con urgencia media, y su fila «Filas que chocan nº 2» describe un conflicto que ya no existe**
> —`A-11` se resolvio por app en `D-18`—. Ver §3.2.

---

## 3. Lo que se puede hacer HOY desde el PC

### 3.1 · 🔴 Defectos abiertos con arreglo conocido

| | que | medida |
|---|---|---|
| **1** | 🔴 **El `$ALARM` no cabe en su buffer.** `bluetooth_reportarAlarma()`, `payload[144]` (guarda 143). Peor caso **por buffer**: **Maestro 158 B, Esclavo 171 B**. Sin guarda, **trunca en silencio** — y una trama truncada sale bien formada hasta la mitad, no casa el CRC, y la app la tira entera: el sintoma es *«el equipo se callo»*, que manda a mirar el cable | 🔴 **`esp32_07_presupuesto_bytes.py` LO DICE EN SU PROPIO COMENTARIO y lo deja fuera de su alcance.** El banco esta verde sabiendolo. Es `N-154` sin terminar |
| **2** | 🔴 **`CAM_CIEGA` de 6 h a 4 dias.** `CAM_CIEGA_MS = 21600000UL` en `botones.cpp` de **las dos puntas**; lo decidido el 05/09 son `345600000UL` | **toca las dos puntas + `camara_03` + 5 documentos, y va TODO EN EL MISMO COMMIT.** Y con el numero va su segunda mitad, que vale mas: **el Modo Inteligente se declara AVERIADO y pide revision** |
| **3** | 🔴 **En la SUBIDA no hay checksum.** `calcularChecksum()` es `static` y **su unico llamador en cada punta es `enviarTramaConCrc()`**; `procesarComando()` no lee el `*XX`. Un bit cambiado dentro de `SET_TIEMPOS` o `SET_RTC` **se obedece** | |
| **4** | 🟠 **`validateTiempos()` de los unitarios de la app sigue en 1..15 min** contra 3..15 del C++ y de `app.js`. **No falla porque ninguno de sus siete casos toca el borde** — que es la prueba muerta de `CLAUDE.md` §3.bis | |
| **5** | 🟠 **El Esclavo no tiene `reloj_diagnostico()`.** Porte **mecanico** desde el Maestro; ya tiene los ingredientes. Sin el, el tecnico que sube 5 m al poste del Esclavo **no puede distinguir `lseOn=0` de `lseRdy=0`** | |
| **6** | 🟠 **`state.correctPin = '1234'` en claro** en `app.js`. La caducidad **si** se construyo | V2 · `B3` |

### 3.2 · 🟠 `DECISIONES.md` tiene cinco filas caducadas — y es el fichero VINCULANTE

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

### 3.3 · 🔴 La evidencia del banco NO esta donde 8 documentos dicen

**Es la unica prueba fisica del proyecto, y las citas estan rotas.** Comprobado con dos patrones
—`find -iname "*Informe*"` y `git ls-files | grep -i informe`—, que es lo que `CLAUDE.md` §4 exige
antes de publicar un «no hay»:

- `evidencia/Informe_Pruebas_Banco_Semaforos_V9.0.pdf` — **14 citas en 8 ficheros**. El fichero vive
  en **`evidencia/old/`**: **las 14 citas estan rotas**.
- `evidencia/Informe_Pruebas_Banco_Semaforos_Sesion2.pdf` (`N-126`) — **NO EXISTE en el repositorio.**
- **La cinta de tramas del 05/09 tampoco esta** (`find -iname "*cinta*"` -> cero).

> 🔴 **Los cierres en cobre de `N-142`, `N-145`, `N-146` y `N-149` existen SOLO como lineas
> transcritas a mano en dos `.md`.** Es la regla del instrumento aplicada al propio roadmap: **las
> unicas veces que este fichero se apoya en una prueba fisica, el ancla no esta.**

### 3.4 · 🟠 Arreglos documentales que hacen dano si no se hacen

1. **La guia de banco le pide al responsable que COMPRE el mando que `D-1` retiro.** Su pregunta
   abierta 3 dice *«la compra ya no es pregunta: receptor con salida NO … ¿se pide ya?»*, y `D-1` y
   `17_` §4 dicen *«ya no se va a comprar»*. **Es una pregunta que no va** (`CLAUDE.md` §2.quater).
2. **`README.md` numera `p10 = Camara 2` / `p12 = Camara 1`** contra la decision escrita de `17_`
   §1.7. **La spec gana.**
3. **`N-118` sigue publicado como defecto abierto en `ESTADO.md`, `README.md` y 16 documentos.**
   Esta **REFUTADO** desde el 05/09 (`d020f3c`): los `0,6 V` eran el firmware viejo.
4. **Tres cifras de flash del Maestro vivas a la vez** —`88,3 %`, `89,3 %`, `86,3 %`— fuera del acta.
   **Solo el acta vale**; las otras se copiaron a mano y envejecieron por separado.

> ⚠️ **Y antes de copiar ninguna cifra de flash a ningun documento, una pasada completa con el arbol
> QUIETO.** El acta del 07/09 avisa ella misma: *«el arbol tenia cambios sin commitear al medir»*.
> `CLAUDE.md` §7: **una cifra correcta de un binario que quiza ya no existe.**

### 3.4.bis · 🔴 `D-20` — el reloj se muda del STM32 al ESP32. DECIDIDO el 07/09, sin construir

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
| **1** | `reloj.cpp` de las dos puntas pasa a **EXTRAPOLADOR sembrado cada `LATIDO_MS` (2 s)** con un `EPOCH` del `DS3231` — **no un reloj de software refrescado «cada tanto»**: con el HSI a 10.000–25.000 ppm, la frescura de la siembra ES el presupuesto de error. `reloj_enHora()` pasa a significar **«mi siembra es fresca»**. Se retiran `STM32RTC`, N-25, N-31, `reloj_ajustar()` y el truco de «enero» | toca **SFTY-18 y SFTY-23**. La desigualdad `SIEMBRA_CADUCA_MS x HSI_PPM + cadena + deriva48h < despeje - ambar` **va en un pack**, no en un comentario (N-71) |
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
> **Lo que eso obliga en el codigo: la app NO pone la hora en el poste 2, nunca.** El puente del
> Esclavo tiene que **rechazar** un `SET_RTC` dirigido a el —hoy lo atiende— porque no es una
> sincronizacion: **es una segunda fuente**. Cierra de paso el **rejuvenecimiento** de las 48 h, que
> se hacia poniendo ese `DS3231` hacia atras.
>
> ⚠️ **Y la consecuencia que hay que saber, porque parece un problema y no lo es:** si el unico
> camino hacia el reloj del Esclavo pasa por el Maestro, **con la radio muerta no se le puede poner
> en hora** — y eso es justo cuando se necesita el Degradado. **No importa: su `DS3231` tiene pila y
> conserva la hora que ya tenia.** Perder la radio no es perder la hora. Lo que si obliga es a que
> **el poste 2 se ponga en hora ANTES**, en la puesta en marcha, no durante la averia.

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

> 🔴 **Y esto es lo que no puede faltar al planificar la sesion de banco: MIENTRAS `D-20` no este
> construida, el Modo Degradado del poste 2 NO SE PUEDE PROBAR.** Su guarda de entrada abre con
> `if (!reloj_enHora()) return DEG_RECHAZO_SIN_HORA;`, y en esa punta esa bandera es **falsa
> siempre**. O sea: `D-18` esta construida —el comando existe y llega— **y el equipo va a contestar
> que no, correctamente, todas las veces**. Quien lo pruebe sin saber esto lo anotara como defecto.

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
> `D-14`, `D-20`, `D-21` y `D-22`, el banco lleva ya **cuatro decisiones VIGENTES SIN CONSTRUIR** y
> dos sin llegar al manual. Cada rojo es legitimo —**todos se apagan construyendolos**, ninguno es
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
> y, de rebote, la precision de `millis()`. Con `D-20` construida el `DS3231` siembra **cada 2 s**,
> y entre siembra y siembra **el error del oscilador interno es despreciable**.

| | que | toca el ciclo | por que va aqui |
|---|---|---|---|
| **1** | **`D-20` · la siembra y la propagacion.** Extrapolador cada 2 s; **el Maestro empuja y el Esclavo SOBRESCRIBE** | 🔴 **SI** | **es lo que DESBLOQUEA el Degradado del poste 2**, que hoy esta muerto: su guarda abre con `!reloj_enHora()` y esa bandera es falsa siempre |
| **2** | **`D-21` · que la hora que MIENTE llegue a las luces**, mas su publicacion en la app | 🔴 **SI** | **media ya esta construida** —el Maestro tiene `irAAmbar("Reloj no fiable")` en su bucle—; faltan el camino del `OSF` **(que `D-20` cierra de paso)** y la guarda equivalente en el Esclavo |
| **3** | **`D-14` · el contacto que hace grabar a la camara.** Antes, **medir con multimetro** si su entrada admite los ~12 V con masa compartida | **NO** | independiente de todo lo anterior; la via esta confirmada en el manual de la camara, con pagina |
| **4** | 🟡 **`D-22` · `Y1` como latido del micro — OPCIONAL, y va el ULTIMO** | **NO** el ciclo, **SI** todos los plazos | **la siembra de 2 s ya cubre lo que `Y1` mejoraria**, asi que **no compensa correr su riesgo antes**: si `Y1` no oscila, el `_Error_Handler` del nucleo es `noreturn` + `while(1)` y la tarjeta queda **A OSCURAS, sin luces y sin reiniciarse**. Lo que si arregla de verdad son los plazos largos de `millis()` — el watchdog, el techo de silencio y **las 48 h del Esclavo, que hoy pueden desviarse entre ~29 min y ~1,2 h** |

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

> 🔴 **El 1 y el 4 no tocan el ciclo. El 2 y el 3 SI**, y esos van con la compuerta delante y
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
| **1.bis** | ✅ **Y el 07/09 el MANUAL confirmo que la via existe, con pagina.** La camara **graba sola** (`Record Schedule` tipo `Continuous`, impresa 36) **y el `in` marca el evento**: tipo `Alarm`, *«the video is recorded after receiving alarm signal from external alarm input device»*. Se arma en `Event → Basic Event → Alarm Input` (impresa 44-45). **Ya no es una decision: es trabajo pendiente** | ⚠️ **Falta UNA medida, de multimetro:** el regimen electrico de la ENTRADA **no lo publica nadie** —la ficha solo da la SALIDA, `1 in, 1 out (max. 24VDC/24VAC, 1A)`—. ¿Admite los **~12 V con masa compartida** de `J9`/`J11`/`J13`, o **exige contacto seco**? Si lo exige, va un rele de por medio: **una pieza, no un cambio de diseno** |
| **2** | ~~La camara sigue llamando a `demanda_solicitar()` y `D-13` dice lo contrario~~ — 🟢 **RETIRADO el 07/09: NO ERA UN HALLAZGO, y la respuesta ya estaba escrita desde el 05/09.** `demanda_hayLocal()` tiene **UN SOLO lector real** y esta **dentro del Modo Inteligente**; `modo_automatico.cpp` y `coordinador.cpp` dan **cero**. O sea: **en Automatico y en Manual las camaras NO tocan el ciclo**, que es exactamente lo que dice `D-13`, y en Inteligente la camara **solo SOSTIENE** —nunca adelanta ni acorta—, acotada por `D-19`. Con la camara muerta el ciclo vuelve a los tiempos configurados: la ausencia no autoriza nada. Ya estaba en `roadmap_hist`: *«las camaras no hacen nada en Auto ni en Manual»* | 🔴 **La leccion es sobre mi, no sobre el firmware:** se publico como contradiccion y **se le llevo al responsable como decision abierta** sin buscar antes si ya estaba contestada. Es `CLAUDE.md` §8 —las opciones que se le ponen delante son un instrumento— repetido |
| **3** | 🔴 **`D-18` no tiene canal de vuelta.** El Esclavo entra en Degradado por app, pero **no existe ningun comando de radio por el que lo anuncie**, y el getter de estado del Maestro solo sabe devolver color: **el poste 1 no puede enterarse.** La fila pedia *«medir que hace el Maestro mientras el Esclavo esta dentro»*; no esta sin escribir por descuido, **esta sin canal** | censo entero de `protocolo.h`: 21 comandos, ninguno lo cubre |
| **4** | 🟠 **La antiguedad de la ultima sincronizacion no viaja** en la trama de estado del Esclavo. Al retirarse el menu, el tecnico que sube al poste 2 se queda sin ese dato **y sin sustituto por app** | plantilla del `$STATUS` del Esclavo: serie, modo, estado, hora, pluma, camara |
| **5** | 🟠 **No hay forma NO DESTRUCTIVA de leer los bits del reloj del STM32.** `reportarBitsDelReloj()` sigue vivo y su **unico llamador borra la hora y todo el respaldo** | |
| **6** | 🟠 **`A.A.A` entra al Modo Automatico SIN GUARDA en el Maestro** —arranca el ciclo, o sea **abre paso**—. Las otras dos secuencias del mando si estan frenadas. Con `A-2` cerrada *(el fin de carrera va a `J14`/`PB0`, no aqui)* **nadie deberia cablear `J16` p5/p8** — pero la asimetria estaba medida solo sobre el Esclavo y conviene que conste | `botones_actualizar()` + `mando.cpp` del Maestro |

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

### 3.6 · 🟠 Dos defectos de los propios INSTRUMENTOS, medidos al levantar el mapa (07/09)

| | que | por que importa |
|---|---|---|
| **1** | 🔴 **El `PASS` de la guarda de rutas depende del ORDEN ALFABETICO de los packs.** Con los ficheros invertidos da **`69 rutas, 4 inexistentes`**. Hoy sale bien porque `esp32_02` y `esp32_09` ordenan antes que `esp32_10` | **renombrar un pack pone la guarda en ABORTADO** sin que nadie toque el firmware |
| **2** | 🔴 **Catorce rutas que los instrumentos abren y la guarda NO censa** — los documentos de la raiz, dos de `04_Manuales`, el Manual 10, la app entera, `Validacion_LCD/arnes_lcd.cpp` y los dos `compilar_*.ps1`. Todas con `ruta_repo()`, que **aborta** | mover una tumba **la fila `banco por packs` entera** mientras la guarda publica *«64 rutas, todas existen»* |

### 3.7 · 🔴 Las etiquetas de las dos cabezas PEATONALES van al reves que los conectores

Trazado de la pata del micro a la bornera sobre el `.kicad_pcb`: **`/S7` es `PA6` → `J11`
(rojo peaton)** y **`/S8` es `PA7` → `J9` (verde peaton)**. Verificado ademas por pads de `U1`:
pad 16 = `/S7`, pad 17 = `/S8`.

> **Quien cablee guiandose por el numero de la senal invierte rojo y verde de peatones.**

Hoy no explota —esos dos canales **no tienen una linea de firmware detras**, ver `D-d`—, pero es
la trampa que espera al primero que enchufe una cabeza peatonal. **Los documentos lo tienen bien:
es la placa la que engana.**

### 3.8 · 🔴 `N-159` — la analitica de las camaras COMPRADAS probablemente no acciona el rele

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

`costura_10` censa **Maestro y Esclavo**. El modulo de expansion **no lo mira nadie**, y censado a
mano salen **siete huerfanas**. Es `N-73` —la Caja Negra documentada en cuatro manuales y sin un
solo llamador— repitiendose en el modulo nuevo, **y esta vez sin instrumento que lo vea**.

---

## 4. Lo que necesita una COMPRA o un SOLDADOR

| | que | cuando |
|---|---|---|
| **C-1** | 🟢 **`2K2` en serie en las 5 entradas de campo, al soldar la placa nueva** (`N-120`) | **es el momento y no vuelve.** Si se suelda igual, la placa nueva nace con la misma averia dentro |
| **C-2** | 🟢 **No poblar los 12 V de `J16` p1** — y mientras exista, **taparlo es obligatorio en cada equipo que se monte** (`D-4`) | al montar |
| **C-3** | 🔴 **`A5`: el reductor `LM2596` desde la bateria** (12 -> 5 V, >= 1 A) para el ESP32 | no esta pedido |
| **C-4** | 🔴 **Quien disena y quien fabrica la placa portadora** | bloquea **desplegar**, no **probar** |
| **C-5** | 🟠 **Las microSD de las camaras** (comprarlas ya esta decidido; falta como se configura la grabacion) | `A-0` |

> ⚠️ **El `2K2` esta `SIN VERIFICAR`: «no se ha probado en ninguna tarjeta».** La cuenta cumple las
> dos desigualdades, pero es una cuenta. Hoy la unica barrera real es tapar `J16` p1.

---

## 5. 🟡 POR VALIDAR — el codigo esta escrito y NADIE lo ha ejercido en una tarjeta

> **Esta es la caja que el proyecto olvida, y la que mas vale.** Un commit no es una prueba de
> banco; un pack verde tampoco. `CLAUDE.md` §3: *lo que ese `0` dice es que los modelos y los
> arneses de PC no encuentran nada.* **Nada de este apartado esta cerrado, y nada de esto viaja al
> historico hasta que una tarjeta lo diga.**

**Los siete que salieron DESPUES de la ultima cinta (05/09, 22:19). Ninguno ha visto cobre:**

| `N-x` | commit | que cambia, y por que hay que ejercerlo en tarjeta |
|---|---|---|
| **N-142** | `6274acc` | **el Esclavo AVISA por radio de su ambar de emergencia**, y los dos vetos se quedan. Es lo que desatasco el bloqueo del cruce **sin tocar el cerrojo**: el Maestro deja de agotar reintentos a ciegas. **Toca el ambar: se ejerce con las dos puntas** |
| **N-147** | `8e9e8a9` | **el Modo Manual ya no entra por la puerta del Automatico** — antes programaba un verde para dentro de `tiempoDespejeMs` y el equipo ciclaba solo. **Se ejerce entrando en Manual y esperando 15 s sin tocar nada** |
| **N-150** | `414b962` | **el ciclo no arrancaba tras aplicar tiempos** (quedaba en rojo para siempre), y **los parsers de la app eran TRES**. **Se ejerce aplicando tiempos desde el telefono** |
| **N-151** | `273b315` | **`DAR PASO` en un modo sin coordinador trababa el cruce PARA SIEMPRE**. **Se ejerce pulsando DAR PASO fuera de Automatico** |
| **N-152** | `d6ce67e` | **el Esclavo avisa de que RETIRA su ambar — y en `MODO_AMBAR` el Maestro ESTABA SORDO**. **Se ejerce cancelando el ambar desde el Poste 2** |
| **N-153** | `79ef5a6` | **la talanquera se publica y se dibuja**: la app no la ensenaba. **Se ejerce mirando la pantalla con la pluma arriba y abajo** |
| **N-157** | `4b90f98` + `ee957ef` | **la camara se vigila a si misma** (fase 1 de `D-13`). 🔴 **Su propio texto lo dice: `CAM_CIEGA` a su valor de produccion NO ES EJECUTABLE en una sesion de banco. El camino esta comprobado en su FORMA, no en su TIEMPO** — y esa es justo la clase de defecto que un pack de forma no puede ver |

> 🔴 **LO QUE ESTA SESION DE BANCO NO VA A PODER PROBAR, y hay que saberlo ANTES de subir al poste
> —no descubrirlo alli—:**
>
> | | por que |
> |---|---|
> | **El Modo Degradado del poste 2** (`D-18`) | su guarda abre con `if (!reloj_enHora())` y en esa punta esa bandera es **falsa siempre**: el cristal `Y2` esta muerto. El comando existe y llega; **el equipo contestara que no, correctamente, todas las veces**. Lo destraba `D-20` (§3.4.bis), decidida y **sin construir**. Quien lo pruebe sin saber esto lo anotara como defecto |
> | **`CAM_CIEGA` en su tiempo real** | son 6 h de paso abierto. **No es ejecutable en una sesion.** Solo se puede ejercer con una compilacion de umbral reducido, **y esa compilacion no es la que va a campo** |
> | **`D-14`, que la camara grabe al cerrar el contacto** | **no existe en el firmware**: cero anclas en las dos puntas (§3.5). La via esta confirmada en el manual de la camara; lo que falta es nuestro lado |
>
> **Los tres se anotan como NO PROBADO, no como fallo.** Es la distincion que el banco del 3-4/09
> ya obligo a escribir: un *«no se pudo probar»* no es un *«sigue roto»* ni un *«ya esta»*.

**Y los que no son de esos siete:**

| | que | por que no esta cerrado |
|---|---|---|
| **N-117** | el perro del ESP32 se comia su propio arranque | **arreglado en el arbol el 04/09; la causa NO esta confirmada sobre el modulo.** `ESP32_ARRANQUE_MEDIDO = 0`, y lo dice el propio fichero. §6.4 |
| **N-110** B2 | el teclado del PIN acepta pulsaciones con el modal cerrado | **no se comprueba leyendo.** B1 si esta cerrado: `NMEAParser.validarTrama(linea)` tiene llamador vivo en `app.js`. §6.6 |
| **la app entera** | `disconnect()` existe (6 llamadas) · `padding-bottom: 90px` puesto · `parseInt(...)\|\|0` solo en comentarios que narran su retirada · `discoverUnpaired` usado (7) · `validarTrama()` con llamador | **los cinco arreglados en el fuente y NINGUNO ejercido en un telefono** |
| **`TECHO_POR_SUELO = 2`** | el Modo Inteligente puede alargar una fase **hasta el DOBLE** del tiempo configurado | **aprobado CON CONDICION —«si un funcional revisa el manual y este manual es claro»— y la firma NO EXISTE. Y ya salio en el paquete del 05/09** |
| **el `0x68`** del `DS3231` | | `SIN VERIFICAR` sobre el modulo |
| **las 21 `SIN VERIFICAR`** de `17_` | | por definicion |

---

## 6. El porque de lo que sigue abierto — los hallazgos que no se han cerrado

**Nada de este apartado se resumio: son los bloques originales, con su medida.** Lo cerrado esta en
[`roadmap_hist.md`](roadmap_hist.md).

### 6.1 · La tarjeta Maestro

### 🛑 N-116 — El Maestro se calienta a los ~30 s: el firmware queda DESCARTADO por censo, no por opinion

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
pines de `J16`. Los dos que se puentearon estan en `INPUT_PULLUP` (`botones.cpp:139-140`): contra
masa consumen `3,3 V / 40 kOhm` ~= **80 uA**, o sea **0,27 mW**. Eso no calienta un chip.

> **Consecuencia dura y util: cargar otro firmware no arregla esto.** Es la clase de conclusion que
> ahorra una sesion entera de banco persiguiendo el sitio equivocado.

#### Una causa que se cayo, y se marca refutada en vez de borrarse

Se sospecho **contencion en `PB6`/`PB7`**: el netlist dice que `J17` es el LCD y el firmware lo usa
como UART del ESP32, asi que dos salidas *push-pull* enfrentadas en el mismo hilo explicarian el
calor perfectamente. **Es falsa.** `Maestro/src/lcd.cpp:74-75` construye el U8g2 con los **cuatro
pines en `U8X8_PIN_NONE`**, y `U8x8lib.cpp` pregunta `if (u8x8->pins[i] != U8X8_PIN_NONE)` antes de
cada `pinMode` y cada `digitalWrite`: no queda ni una escritura. La pantalla no conduce nada.

Queda escrita porque **es la sospecha natural** —la contradiccion netlist/fuente esta ahi y volvera a
proponerse—, y porque las medidas del paso 5 la explican mejor sin ningun defecto: `RST` (`PB7`) a
3,3 V es el **TX del ESP32 en reposo**, que es alto; `RS/A0` (`PB6`) variando entre 2,8 y 3,3 V es el
**TX del STM32 transmitiendo**. Todo coherente, cero conflicto.

#### 🔴 UNA SEGUNDA CAUSA REFUTADA, Y ERA LA MIA — la talanquera no puede ser

Se propuso aqui mismo, y hay que tacharla con el mismo rigor con que se escribio. El razonamiento era:
`semaforo.cpp:93` energiza la talanquera cuando `verde || estado == S_FALLO`; un Maestro solo cae a
`S_FALLO` a los **~20 s** —medido en el paso 8— y **en ese instante enciende `J15`**, que es lo unico
que conmuta solo dentro de la ventana de los 30 s. Encajaba en el tiempo.

**Se cae al leer el cobre.** Trazada la cadena entera sobre `Controladora_Semaforos.kicad_pcb`:

```
U1.20 (PB2) -> /Motor -> R70 220R -> U15.1   TLP127  (LED del optoacoplador)
                      -> R69 10K a masa      (pull-down de BOOT1, correcto)
U15.6 -> /5V    U15.4 -> R72 220R -> puerta de Q10 (IRLZ44N), con R71 10K y C30 100nF
Q10.2 (drenador) -> J15.2 + D30 1N4148 al riel de 12V     Q10.3 -> GND
```

**`U15` es un TLP127: aisla galvanicamente las dos mitades.** El STM32 no toca la etapa de potencia
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

> 🔴 **La placa protege cada SALIDA con 220 ohmios en serie y un optoacoplador, y no protege NINGUNA
> entrada de campo.** Los cinco pines que salen a bornera van **desnudos al die**. El `10K` y el
> `100nF` que llevan estan en **paralelo**, no en serie: fijan el reposo, **no limitan corriente**.

Y en ese mismo conector, **`J16.1` es el riel de `/12V` crudo** —el netlist lo confirma: comparte net
con `J15.1`, `J13.1`, `J11.1` y veintitantos mas—. `pines.h:120-121` ya lo tenia escrito: *«12 V
CRUDOS, sin opto, sin serie, sin clamp»*.

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
| **salidas** (9) | `PBx -> 220R -> TLP127 -> potencia` | serie **y** aislamiento galvanico |
| **entradas de campo** (5) | `bornera -> pin del STM32` | **ninguna** |

Y las entradas son justo las que un instalador toca: `J14` (camara de demanda) y `J16` p10/p12
(camaras C y D), en un conector cuyo **p1 lleva 12 V crudos**.

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

### 🔴 N-108 — El enlace no deja rastro de como se cayo, y el umbral que lo arreglaba lleva un mes sin subir a campo

**Lo aporto el responsable desde el campo el 31/08, y confirma N-71 POR EL OTRO LADO:**

> *"El problema de desconexion es no saber cuanto se va cuando se va, y por que se va. Se va a ambar
> a los 12 segundos de desconexion, y ese parametro toca alargarlo un poco mas, porque se va a
> ambar por nada."*

#### Los 12 s: ya esta arreglado, y ese es el problema

**MEDIDO:** `SFTY6_SILENCIO_MS = 25000UL` en `protocolo.h:149` de las dos puntas... **pero eso es la
rama**. El equipo de la calle es la **V8.4, `e303485`, del 31/07**, y ese commit **ni siquiera es
alcanzable desde esta rama** (`git merge-base --is-ancestor` -> no). **En el poste sigue habiendo
12 s**, y el arreglo lleva desde el **27/08** escrito sin poder subir.

#### Y el "por nada" es LITERAL — el equipo se rendia antes de terminar de intentarlo

El comentario del propio firmware (`protocolo.h:120-135`) describe el sintoma sin haberlo visto:

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

**Y lo que el Esclavo INVENTA** (`Esclavo/src/bluetooth.cpp:328`):

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

Lo que falta, y va con la Fase 4:

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
(`contrato.h:212`). Y el aprendizaje del nombre bueno **se guarda para el arranque SIGUIENTE**, nunca
en caliente: renombrar obligaria a cerrar el perfil y tirar la sesion del operario
(`transporte_app.cpp:109-114`, decision deliberada y razonada alli).

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
| **Automatico** | `millis()` — `modo_automatico.cpp:67,126,141` | 🟢 **sigue ciclando.** No toca el reloj |
| **Degradado** | `reloj_enHora()` — `modo_degradado.cpp:155,329,349,515` y `coordinador.cpp:335,476,498` | 🔴 **no se puede ni entrar** |

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

- **El STM32 cuenta el silencio de `J17`** igual que ya se cuenta el de la radio (N-108): cuanto lleva
  mudo el puente, cuantas veces se cayo, y cuanto duro cada corte. **No necesita cobertura, ni SIM, ni
  internet, ni que nadie este delante**, y cuando el tecnico por fin conecta, la app se lo descarga
  entero. Contesta justo lo que se dijo que falta en campo: *«no saber cuanto se va cuando se va, y
  por que se va»*.
- **Watchdog en el ESP32** (ya es la tarea **T4**) para que se reinicie solo, y que **declare el
  reinicio** al volver — un puente que revive en silencio esconde el fallo que hay que contar.
- **Y la decision que hay que tomar antes de escribir nada: que hace el equipo si el reloj se va.**
  Hoy la respuesta es *«el Degradado no entra»*, y eso hay que elegirlo a proposito, no heredarlo.

#### Lo que cuesta de verdad un aviso remoto, para que se decida con el precio delante

Un correo desde el poste necesita **camino propio a internet**: WiFi del sitio o un modulo celular con
SIM y su plan. El ESP32 trae WiFi —esa parte esta— pero **hace falta cobertura en el cruce y una red a
la que entrar**, y si el que avisa es el mismo que se cuelga, el aviso no sale. **Es una decision de
producto con coste recurrente, no una linea de firmware**, y va al responsable con esa etiqueta.

---


### 6.6 · Las dos barreras de la app

### 🟠 N-110 — Dos barreras de la app que no vigila nadie, salidas de invertir el arnes de DOM

**Las encontro el agente que invirtio `test_dom_execution.js`, y las verifique yo mismo antes de
escribirlas aqui** (§4: un informe no es una medida). **Ninguna de las dos se arreglo el 31/08** — se
dejan medidas y abiertas, porque tocar `app.js` obliga a recompilar la APK y rehacer el paquete.

#### 1. La app **no valida el checksum** de lo que pinta

```
app.js:1420  parseNmeaTelemetry()  ->  line.split('*')[0]      el CRC se tira sin mirarlo
js/nmea_parser.js:27  validarTrama()   4 definiciones en disco, CERO llamadores
```

Es **la forma exacta de N-73**: una funcion declarada, documentada y sin un solo llamador. Y hay
prueba dura de que no se mira: la trama de ejemplo del arnes lleva `*5F` **desde siempre** y su
checksum real es `*04` — la app la pinta como verdad. Sobre radio a **2,4 kbps**, eso significa que
un `$STATUS` corrompido en vuelo se dibuja como el estado del cruce. *(Ya estaba medido en
`simulador_puente_esp32.py:1295`; lo que es nuevo es que el llamador existe y esta ahi al lado.)*

#### 2. El teclado del PIN **acepta pulsaciones con el modal cerrado**

Los handlers de `.pin-btn[data-key]` (`app.js:2099`) **no consultan si `pin-modal` esta activo**, y
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
error de la app.

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
`modo_inteligente.cpp:106`. Y `coordinador_hayDemandaRemota()` solo se arma **si
`modoActual_get() == MODO_INTELIGENTE`** (`coordinador.cpp:758`).

| modo | que hace una deteccion de camara |
|---|---|
| **AUTOMATICO** | fija un `tUltima` que **nadie lee**. `modo_automatico.cpp` ni incluye `demanda.h` |
| **MANUAL** | lo mismo: nada |
| **DEGRADADO / AMBAR** | la trama del Esclavo **ni se lee** |
| **INTELIGENTE** | aqui si, y es el unico |

Lo que si corre en todos los modos es la LECTURA del pin —`botones_actualizar()` sin
condicion en `main.cpp:144`—, que es lo que hacia parecer que estaba construido. **Leer no
es consumir.** §2.ter otra vez, y esta vez el que la recito fui yo.

### 🔴 Y EL MODO QUE LAS USA VIOLA EL MINIMO DE 3 MINUTOS, EN EL FUENTE

`modo_inteligente.cpp` corta el verde a los **15 s** —constante escrita a mano— mientras
`limites_ciclo.h:54` fija `VERDE_MIN_MIN = 3` **minutos**, decidido por el responsable el
04/09 por seguridad vial.

**Consecuencia medida:** con la camara del Maestro pegada en «hay presencia» —o con cola
continua— el **Esclavo recibe 15 s de verde por ciclo y el Maestro 3 minutos**. Con las dos
camaras ruidosas, el cruce alterna al minimo indefinidamente: 15 s + despeje + 4 s de
ambar, en los dos sentidos.

> **Eso es exactamente «meter ruido», y estaba en el codigo antes de comprar la camara.**
> Y viola la regla que `modo_automatico.cpp:83-91` justifica asi: *«conductor convencido de
> que el semaforo esta averiado, adelantando en rojo»*. La rompe **justo el modo que usa
> camaras**.

### El Modo Inteligente es obra DECLARADA, no ejercida

- **Ningun arnes lo compila.** `grep -rl modo_inteligente Validacion_* compuerta.py` -> vacio.
  Lo leen tres packs **por texto**. Es el punto ciego de §8 en su forma pura.
- **Ninguna tarjeta lo ha corrido.** El informe del banco del 3-4/09 dice *«nunca aparecio
  la luz verde en ningun poste»* y *«sin camara de demanda real»*.
- Solo se entra por la app: la via del menu esta muerta -`menu.cpp:111` cuelga de
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

1. 🔴 **La alarma de «8 dias sin detectar» que el responsable enuncio NO EXISTE en el
   firmware.** Y sin ella, la camara de la barrera muda equivale a «baja sobre el coche»
   -o sea, lo de hoy-: aceptable solo CON esa alarma.
2. **La camara del Esclavo en Auto/Manual no es inofensiva:** manda **3 copias de
   `CMD_DEMANDA`** por deteccion en un canal de **2,4 kbps semiduplex** que tambien lleva
   `GO_RED`/`ACK_RED`, y lo hace **sin guarda** (`demanda.cpp:26`). *[SIN MEDIR: el impacto
   probablemente es pequeno, pero es una colision posible en el instante que mas importa.]*
3. **`camara_leerPin()` hace `delay(5)`** con el pin en alto (`botones.cpp:105-111`).
   Leerla POR NIVEL en cada vuelta cuesta 5 ms/vuelta mientras haya presencia: inofensivo
   para el watchdog de 4 s, pero condiciona como se implementa un veto.

---


### 6.8 · `SFTY-29` · las camaras como veto — declarada y SIN CONSTRUIR

> 🔴 **Medido el 07/09: `grep -rn "EJERCE SFTY-29" packs/` -> `0`**, y `OPTIMIZACIONES.md` la
> tiene como *«solo diseno»*. Es la decision `A-1.bis`.


`OPTIMIZACIONES.md:401` la tiene registrada como **«solo diseno»**: *«Presencia como veto
del todo-rojo y sensor de pluma»*. Lo que el responsable describio el 04/09 es exactamente
esa regla, y aqui queda con lo que se ha MEDIDO y con lo que sigue SIN DECIDIR.

### Lo que la talanquera hace HOY, medido

`Maestro/src/semaforo.cpp:100-102` — es una **funcion pura de la luz**, sin camara ninguna:

```c
digitalWrite(MOTOR_TALANQUERA,
             ((verde && !testLedsActivo) || estado == S_FALLO) ? ABRIR : CERRAR);
```

Verde -> arriba · Rojo -> abajo · Ambar (`S_FALLO`) -> arriba · **sin energia -> baja
sola**, porque el pin cae a `LOW` y el MOSFET no conduce (SFTY-28).

### 🔴 LAS DOS CAMARAS NO SON EL MISMO PROBLEMA: FALLAN EN DIRECCIONES OPUESTAS

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
   (`botones.cpp:144-152`) dispara en el flanco de subida y llama a `demanda_solicitar()`.
   Vetar es preguntar *«hay presencia AHORA»*: otra lectura del mismo pin.
2. **La solucion la dijo el responsable sin darse cuenta:** *«paralelo al cambio a rojo,
   lanza leer camara»* — o sea **una lectura EN EL INSTANTE de la transicion**, no un veto
   continuo. Un vistazo puntual esta mucho menos expuesto a un sensor pegado.
3. **Un veto CON TOPE DE TIEMPO deja de ser peligroso.** *«No aguanto mas de N segundos;
   pasado eso cambio igual y levanto alarma»*. Es el mismo patron de su alarma de
   mantenimiento aplicado al ciclo, y convierte el enclavamiento de la camara de salida de
   fail-dangerous en fail-safe.

### 🟡 LO QUE SIGUE SIN DECIDIR, y es del responsable

- **¿La camara de salida VETA el ciclo o solo AVISA?** El mismo dudo: *«para modo
  automatico no se, no parece que sea»*, y apunto que quiza vale mas para **imagenes y
  auditoria** —accidentes, soportes— guardadas en la Raspberry o la Nano.
- Si veta: **cuantos segundos como maximo**.
- **Lectura en el instante del cambio, o veto continuo.**
- 🔴 **Que pasa si la camara esta MUDA** —no «no hay presencia», sino sin responder—.
  Presencia y silencio no son lo mismo y **hoy el pin no los distingue**: un cable suelto
  lee igual que «via libre». Con `INPUT` pelado y pull-down, una camara desconectada dice
  «no hay nadie».
- Y su propio aviso, que va escrito porque condiciona todo lo demas: *«hay que hacer un
  laboratorio… no se como se comporta esa camara. No es una camara asi super fiable, ni
  super rapida»*. **Nada de esto se decide sin esa medida.**

---


### 6.9 · 🎯 DECISION: DOS PANTALLAS, PORQUE SON DOS COSAS (04/09, noche)

> 🔴 **Decision del responsable del 04/09 con CERO codigo detras.** Es la unica decision suya
> que no tiene una sola linea.


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
| `PB0` | camara de demanda | `J14` | vivo *(solo dentro del Modo Inteligente en el Maestro)* |
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
| **C1** | 🔴 **N-112: la compuerta alterna** | **es el aparato de medir, y miente**. Todo lo demas se juzga con el |
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
