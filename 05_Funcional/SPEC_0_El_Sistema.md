# SPEC 0 — EL SISTEMA: que es, y como se pone un cruce a funcionar

**Version del equipo: `V9.1`** *(14/09/2026)*. Sube desde la `V9.0` del 12/09 por **dos cambios que se ven desde la carretera**, y ninguno de los dos cabe dentro de un numero de version ya entregado:

- **El verde del modo sin radio pasa de medio minuto a TRES MINUTOS.** Es el cambio mayor: cuando los dos postes pierden el enlace y se reparten el paso por reloj, cada lado da paso tres minutos en vez de treinta segundos. Lo que hace ese modo, en `SPEC_7`.
- **La barrera ya no baja en el mismo instante en que el semaforo se pone en rojo:** baja unos segundos despues, **para dar tiempo a los vehiculos que ya venian pasando y no golpearlos**, y **no baja en absoluto mientras una camara vea algo debajo**. Lo que hace y lo que no, en `SPEC_5`.

⚠️ **Y esta version NO ha visto una tarjeta.** Lo que hay instalado en campo sigue siendo la `V8.4`.

⚠️ **Los manuales de `04_Manuales/` y del resto de `05_Funcional/` siguen rotulados `V9.0`, y es a proposito:** no se reescriben hasta que haya banco y tarjeta, porque reescribir un manual antes de que el equipo este quieto es tirar el trabajo dos veces. **Las siete spec SI estan al dia.** La unica excepcion son las guias `.html` de campo, que se corrigen siempre: las sigue un instalador con un destornillador en la mano.

**La puerta de entrada.** Quien no ha visto este repositorio empieza aqui y sale sabiendo **que es el producto**, **como se
opera un cruce de principio a fin** y **que NO hace el equipo**. Escrita el 13/09/2026 con **cuatro agentes escribiendo en el
arbol a la vez**: todo lo medido esta fechado al commit **`ef3504c`** (`git show ef3504c:<fichero>`). **Mandan sobre este
fichero** `05_Funcional/17_...` en el cobre medido y **cada una de las otras ocho spec en su materia**; `DECISIONES.md`
es su indice — aqui vive el puntero, nunca el desarrollo. Se cita el **SIMBOLO**, nunca la linea (`CLAUDE.md` §7.3),
y **ninguna cifra vive aqui** (§14).

## 0. 🔴 LO QUE SALIO AL RECONSTRUIR LA OPERACION — y por eso va primero

**A · NO HAY ORDEN PARA LEER LOS TIEMPOS QUE EL EQUIPO TIENE DENTRO, Y EL FORMULARIO DE LA APP MIENTE POR OMISION.** Medido:
el despachador del Maestro (`Maestro/src/bluetooth.cpp`) no tiene rama que devuelva verde/rojo/despeje —la lista entera es
`SPEC_4` §3.1—, el `$STATUS` no los lleva (`SPEC_4` §5), y **`app.js` nunca escribe en `num-tiempo-verde`/`-rojo`/`-despeje`:
solo los LEE al enviar**. El tecnico que llega a un cruce en marcha ve **los valores por defecto del `.html`**, no los del
equipo, y **si pulsa «Aplicar tiempos» sin tocar nada los graba**. **No es un hueco de documento: falta la orden.**

**B · UN CORTE DE ENERGIA EN AUTOMATICO DEJA EL CRUCE EN AMBAR INTERMITENTE, Y NADIE LO ARRANCA.** Medido en
`Maestro/src/main.cpp`: el `setup()` tiene **dos salidas y solo dos** — reanuda **Modo Degradado** si
`modo_degradado_reanudarTrasCorte()` lo autoriza (`D-29`), y **en cualquier otro caso pasa al ambar intermitente de
arranque** (`entrarAmbarDeArranque()`, `D-40`), tras esperar en `MENU` mientras la reanudacion siga pendiente.
**El Automatico no se reanuda: no se guarda el modo, solo los tiempos**, y volver a darlo exige **una persona con el
telefono en el poste**. `SPEC_3` §6 y `SPEC_6` A.5.3 documentan la reanudacion del Degradado; **la del modo normal no.**

**C · «Operacion normal» NO estaba escrita, y lo confirma el instrumento.** `grep -rincE "puesta en marcha|operacion
normal|primera vez" 05_Funcional/SPEC_*.md` da **0 en las ocho** (reproducido hoy). Lo unico que habia: el procedimiento de
campo del **Degradado** (`SPEC_6` PARTE A), y **tres pasos dentro de `app.js`** —*«1) VOLVER AL MENU; 2) mande los tiempos;
3) ARRANQUE EL CICLO con AUTOMATICO»*, literal de `EN_MARCHA_PARE_EL_MODO`, que `N-150` escribio despues de que un operario
se quedara el 05/09 delante de un cruce en rojo esperando a que arrancara solo—. **Era la mejor documentacion de operacion
del proyecto y vivia en un literal de la app.**

## 1. QUE ES EL PRODUCTO

**Un semaforo movil de obra de DOS POSTES para PASO ALTERNADO DE UN SOLO CARRIL**: mientras un extremo del tramo tiene verde
el otro tiene rojo, y entre los dos verdes hay un todo-rojo que vacia el tramo. **Los dos postes NUNCA dan verde a la vez** —
el unico requisito que explica todo lo demas (`SPEC_2`). **Un poste es el Maestro (poste 1) y el otro el Esclavo (poste 2)**:
el Maestro es el unico que ordena verde, el Esclavo obedece y acusa (`SPEC_2` §1). **El reparto, medido hoy** sobre el
`platformio.ini` de cada proyecto y `01_Firmware/*/src`:

| por poste | que es | que hace |
|---|---|---|
| **STM32** (`board = genericSTM32F103C8`) | `01_Firmware/Maestro` o `/Esclavo` | **gobierna las luces y la pluma**. Es el que manda |
| **ESP32** (`board = esp32dev`) | `01_Firmware/ESP32_Expansion` | **puente Bluetooth** al telefono, y **lleva el reloj** |
| **`DS3231` con pila** | colgado del ESP32 | **la autoridad de la hora, siempre** (`D-20`); el STM32 no tiene reloj que sirva (`D-9`) |
| **radio E90-DTU** | bornera `J12` | el **unico** vinculo entre los dos postes: no hay cable entre ellos |

**El STM32 es el controlador y el ESP32 un accesorio colgado de un puerto serie (`J17`): no manda sobre las luces** (`SPEC_5`
§1). **Repetidor: OPCIONAL y hoy NO montado** (`SPEC_6` B.4) — ⚠️ **y no es otro STM32: es un ESP32**
(`Repetidor/platformio.ini`), dos radios espalda con espalda que validan CRC y **no originan ni una trama**.

🔴 **UN TELEFONO CON LA APP ES LA UNICA FORMA DE OPERARLO** (`D-16`): no hay pantalla (`D-17.bis`), no hay pulsadores (`D-2`)
y el mando de reles no tiene receptor montado (`D-1`). **Sin telefono no se arranca el cruce, ni se para, ni se pone en
ambar.** Es una propiedad declarada del sistema, no una averia.

## 2. 🔴 LA OPERACION NORMAL, PASO A PASO

> Cada paso lleva **la orden real que se teclea**; lo que contesta cada una, literal por literal, es `SPEC_4` §3 y no se
> repite aqui. Donde el firmware no tiene camino, pone **HUECO**.

**Paso 0 — antes de subir al poste.** Las radios de las dos puntas tienen que estar **configuradas IGUAL** (misma tasa, mismo
canal, `M0`/`M1` en OFF, antena puesta): `SPEC_6` PARTE B. **Ningun instrumento de este repositorio verifica esa
configuracion** (`SPEC_6` HUECO 3), y si no se hizo el sintoma no dira «radio mal configurada»: dira que el equipo se callo.

1. **Encender los dos postes.** No hay interruptor de modo ni secuencia de arranque: se da energia.
2. **Donde queda el equipo al encender.** El **Esclavo pone rojo en las tres primeras lineas de su `setup()`**
   («las luces primero, siempre»). El **Maestro entra en AMBAR INTERMITENTE de fuera de servicio**
   (`entrarAmbarDeArranque()` y `modo_ambar_setup()`: todo-rojo y `CMD_GO_AMBAR` al Esclavo; `D-40`, `SPEC_1` §4.1)
   y sale con una orden. **Excepciones:** si estaba en Degradado cuando se fue la luz, reanuda en Degradado (`D-29`);
   si estaba en rojo fijo por hora perdida o por rojo total, arranca en ese rojo fijo (`D-47`). **Nunca reanuda el
   Automatico** (§0.B).
3. **Conectar el telefono al poste 1.** Emparejar buscando el rotulo `<ROTULO_PREFIJO><serie>-M` (`-E` en el poste 2).
   ⚠️ **Un ESP32 estrenado anuncia `ROTULO_PROVISIONAL` y el rotulo bueno aparece EN LA ARRANCADA SIGUIENTE** (`SPEC_4` §1):
   no es averia y no se arregla reintentando.
4. **Meter el PIN en la app.** Ella lo antepone a todo lo que lo necesita; que ordenes entran sin el, y por que, es `SPEC_4`
   §4 y §7.1.
5. **Poner la hora — a CADA poste por separado.** `SET_RTC` la atiende **el ESP32 del poste al que se esta conectado** y ya no
   sigue viaje (`D-15`): quien se conecta al poste 1 **no le pone la hora al poste 2**. ⚠️ **No hace falta para el
   Automatico; hace falta ANTES de poder usar el Degradado**, con la comprobacion obligatoria de `SPEC_6` A.6 —leer las dos
   puntas con `LEER_RTC` y contrastar con un reloj de fuera—.
6. **Fijar los tiempos.** En la app: **Modo Tecnico** → pestana **Tiempos** → verde (min), rojo (min), despeje todo-rojo (s) →
   **Aplicar**. Por el cable, `CMD:PIN:<pin>:SET_TIEMPOS:<verde>,<rojo>,<despeje>`. **Con el ciclo en marcha se rechaza**
   (`EN_MARCHA_PARE_EL_MODO`): hay que **parar primero** (paso 9), porque bajar un tiempo a mitad de fase acortaria la fase EN
   CURSO — y una de esas fases es el todo-rojo que deja salir a los vehiculos del tramo. 🔴 **FIJAR LOS TIEMPOS NO ARRANCA EL
   CICLO** (`D-11`): contesta `RESULT:OK` y **se queda en rojo en las dos puntas**; el tercer paso es del operario.
   ⚠️ **El formulario no ensena lo que el equipo tiene dentro (§0.A):** escribir los tres numeros a mano aunque parezcan los
   correctos. Los limites duros los fija el firmware, no la app (`D-5`, `limites_ciclo.h`, `SPEC_1` §5).
7. **ARRANCAR EL CICLO.** `SET_MODO:AUTO` — boton **AUTOMATICO**, o **«ARRANCAR EL CICLO AHORA»** del aviso que sale bajo el
   formulario. **Abre paso**, asi que la app pide antes el **vale de via despejada**. Entra por todo-rojo y su despeje: **el
   primer verde tarda, y no se repite la orden.**
8. **Verificar con los ojos, no en la pantalla — un ciclo entero.** Maestro en verde ➜ Esclavo en rojo; **todo-rojo largo con
   las dos en rojo**; Esclavo en verde ➜ Maestro en rojo; **en ningun momento verde en las dos**. ⚠️ Esa lista existe escrita
   **solo para el Degradado** (`SPEC_6` A.3) y vale igual aqui.
9. **PARAR.** `SET_MODO:MENU` —**sin PIN**— deja **rojo fijo en las dos puntas** y ningun cambio programado; es tambien el
   paso previo obligatorio para reconfigurar. **Emergencia, las dos sin PIN:** `FORZAR_ROJO` en el poste 1 y
   `AMBAR_EMERGENCIA` en el poste 2 — este queda **enclavado** y se retira con `CANCELAR_AMBAR`, que **si** pide PIN
   (`SPEC_6` A.4).
10. **Fin de jornada. 🔴 HUECO: no existe orden de apagado ni de parada segura.** Lo que hay es parar con `SET_MODO:MENU` y
    **quitar la energia**: sin energia el pin de la pluma cae a reposo, **la pluma BAJA** y las luces se apagan (`SPEC_5` §4).
    Al volver la energia, §0.B.

## 3. EL MAPA DE LAS OCHO SPEC — a donde ir

| | contesta |
|---|---|
| **`SPEC_1` Ciclo y Luces** | que hace **un** poste con sus tres luces y su pluma: modos, transiciones, barrera de salidas, estado seguro |
| **`SPEC_2` Dos Puntas y Radio** | como se coordinan los dos postes, y que pasa con la radio sana, degradada y muerta |
| **`SPEC_3` La Hora** | de donde sale la hora, quien siembra a quien, cuando CADUCA y que se hace entonces |
| **`SPEC_4` App y Bluetooth** | **todas las ordenes y que contesta el equipo a cada una**; telefono → ESP32 → STM32 |
| **`SPEC_5` Cobre y Conectores** | que hay en cada pin, que es peligroso y que no se ha medido nunca del hardware. Se lee **con un destornillador en la mano** |
| **`SPEC_6` Campo, Radio y Alarmas** | el Degradado paso a paso, la configuracion de las radios, y **que hace el tecnico con cada alarma** |
| **`SPEC_7` Modo Sin Reloj** | por que ese modo **no se construye**, y que si falta (que la app diga POR QUE) |
| **`SPEC_8` Camaras y Barrera** | **que HACEN** las camaras y la pluma: el veto, el retardo de bajada, cuando el equipo pide que se revise una camara, y **que NO protege** |
| **El `.zip` al funcional** (no es SPEC: `.claude/particularidades/entregar.md` §1 y §4) | que lleva y de donde sale: **un paquete, un commit** (`D-84`): firmware, `www/`, hoja y `FW_HASH.txt` de `git show <commit>:`, la APK comprobada contra ese `www/` (`D-85`); el hash del nombre y del PDF es el del arbol que va dentro |

> 🔴 **`SPEC_5` y `SPEC_8` son las dos mitades de un mismo aparato y se leen juntas** — se partieron
> el 14/09 porque un solo documento servia a dos lectores (`roadmap.md` 1.45). El corte: **`SPEC_5` es
> lo que decide el COBRE** (que hay en un borne, y que pasa sin firmware dentro); **`SPEC_8` es lo que
> decide el FIRMWARE.** Por eso la pluma **sin energia** esta en `SPEC_5` §4 y la pluma que **sube con
> el ambar intermitente de averia** esta en `SPEC_8` §5 — con la eleccion vial del cliente y el PMT
> del 27/08/2026 que **sigue sin fila en `DECISIONES.md`**.

## 4. GLOSARIO

- **Punta** — cada uno de los dos controladores. «Las dos puntas» = poste 1 + poste 2.
- **Cruce** — la instalacion entera: los dos postes y el tramo de un carril que regulan.
- **Paso alternado** — un solo carril util: se da paso a un sentido cada vez, nunca a los dos.
- **Cabezal** — las tres luces de un poste. Sus **dos caras van siempre juntas**: un poste tiene **un** color, no dos (`SPEC_1` §1).
- **Pluma / talanquera** — la barrera que baja sobre el carril. **REFUERZA la senal, no la sustituye**: quien regula es la lampara (`SPEC_1` §2).
- **Despeje** — el **todo-rojo entre los dos verdes**, que vacia el tramo. Es la **unica proteccion del que se quedo dentro** (`SPEC_1` §5).
- **Degradado (Modo)** — sin radio, cada punta decide su luz **por su reloj**: el unico que da verde sin confirmar con
  la otra. Entrada **manual**, o **automatica** tras 5 min sin radio si un tecnico dejo la opcion activada con PIN en
  los dos postes (`SPEC_2` §7, §7.ter; `A-15`).
- **Orfandad** — que a una punta deje de llegarle nada de la otra durante `SFTY6_SILENCIO_MS`; se responde con **ambar intermitente** (SFTY-6, `SPEC_2` §4).
- **Siembra** — el envio periodico de la hora del ESP32 a su STM32 por el cable `J17` (`SPEC_3` §2).
- **Vale de via despejada** — lo que la app pregunta antes de una orden que **abre paso**: un PIN demuestra QUIEN eres, no que hayas MIRADO (`SPEC_4` §4).
- **PMT** — quien dirige el manejo de transito de la obra; consta como autor, con el cliente, de una decision vial del 27/08/2026 que **no tiene fila** (`SPEC_8` §5) (`SPEC_5` §4). ⚠️ **El repositorio usa la sigla sin desarrollarla nunca**, y aqui no se inventa.

## 5. 🔴 LO QUE EL EQUIPO NO HACE — en un solo sitio, y **medido** en la spec que cada linea cita

1. **NO MIDE LA BATERIA.** `BAT:` sale `--` **fijo en las dos puntas** —falta el divisor y la entrada analogica—, y la app lo
   dice con esas palabras en vez de pintar un numero (`SPEC_4` §6).
2. **NO GOBIERNA CABEZAL PEATONAL NI ZUMBADOR, AUNQUE LOS PINES EXISTAN.** `ROJO_PEATON`, `VERDE_PEATON` y el buzzer estan
   **declarados y muertos** —ni un `pinMode` ni un `digitalWrite`—: **cableados a `J11`/`J9`/`J13` no se encienden nunca y no
   hay mensaje de error** (`SPEC_5` §2, `SPEC_1` §1). **No se venden como funciones del equipo.**
3. **NO DISTINGUE UN VEHICULO PARADO DEBAJO DE UNA CAMARA MAL APUNTADA.** 🟢 ~~Ninguna camara protege la pluma~~ →
   **desde el 14/09 SI la protege**: la barrera baja 3 s despues del rojo y **no baja mientras una camara vea algo debajo**
   (`SPEC_8` §1). 🔴 **Lo que el equipo NO puede hacer es juzgar lo que ve:** un vehiculo parado y una camara
   que dispara sola **dan el mismo contacto cerrado**, asi que ante la duda **no baja y avisa** —y quien decide si eso es
   una averia o trafico es una persona, no el equipo—. Y **los ultimos metros de la bajada no los ve nadie**: no hay fin
   de carrera, y esta **sin preguntar al fabricante** si la centralita trae fotocelula o borde sensible.
4. 🟢 ~~**NO HAY AMBAR AL CERRAR EL VERDE: salta de VERDE a ROJO.**~~ → **construido en `cda33df` (`D-45`), sin banco
   ni tarjeta:** todo verde se cierra por 3 s de amarillo (`AMARILLO_SEG`) antes del rojo, en las dos puntas y en todos
   los modos, y el verde se abre directo (`SPEC_1` §3.2). La pluma sigue arriba durante el amarillo, baja unos segundos
   despues del rojo y no baja mientras una camara vea algo debajo (`SPEC_8` §1). **Lo que los equipos de campo hacen lo
   dice `ESTADO.md`**: hasta cargar este firmware, siguen saltando de verde a rojo.
5. **EL PIN VIAJA EN CLARO Y CUATRO ORDENES NO LO PIDEN.** El transporte es Bluetooth SPP **sin cifrar**; las cuatro que
   **cambian algo** sin clave estan contadas en `SPEC_4` §7.1. Y **`SET_RTC` con un PIN falso pone la hora** — riesgo
   **aceptado** por el responsable, `D-26` (1).
6. **NO HAY AVISO REMOTO: si no hay nadie conectado, nadie se entera.** Ni red, ni SMS, ni servidor: `$ALARM` sale **en el
   instante** y **solo lo ve quien esta conectado entonces** (`SPEC_6` PARTE C). Lo mismo el aviso previo al limite duro
   del Degradado: **las dos puntas lo publican** por evento, pero solo lo lee quien este conectado (`SPEC_7` §5).
7. **SOLO ENTRA EN DEGRADADO SOLO SI UN TECNICO LO DEJO ACTIVADO ANTES, Y FUERA DE ESO LA APP INVENTA LA CAUSA.** Con la
   opcion automatica activada con PIN en los dos postes, entra tras 5 min sin radio y lo anuncia (`AUTO_ENTRADA`);
   sin ella la entrada es de una persona (`SPEC_2` §7.ter, `A-15`, construido, sin banco). En los demas casos el equipo
   publica `MODO:` y `ESTADO:` y **nunca la causa**: los textos de causa estan escritos en `app.js` (`SPEC_7` §6, H-2).
8. **NO SE DIAGNOSTICA A SI MISMO EL COBRE NI LA RADIO.** Ningun instrumento lee la configuracion de una radio (`SPEC_6`
   HUECO 3); la pluma **no tiene realimentacion** —sabe «ordene abrir», nunca «esta abierta» (`SPEC_1` §2)—; y **una camara
   muerta desde la instalacion no la avisa nadie** (`SPEC_8` §4).
9. ⚠️ **Y lo que no se puede afirmar de nada de esto: NO HA VISTO UNA TARJETA.** Ninguna camara se ha conectado nunca a este
   equipo, y que firmware corre en cada poste **lo dice `ESTADO.md`, no este fichero** (`CLAUDE.md` §0.2).
10. **MODO NOCHE: CONSTRUIDO EN `N-179`, SIN BANCO NI TARJETA (`D-103`, `D-106`..`D-111`, `D-115`).** Lo que HACE
   el fuente (`modo_noche.cpp`, `noche_flash.cpp`), medido en el arnes de dos puntas (bloque N); la escritura en la
   flash real, la luz y la pluma solo se cierran en la tarjeta. No va en el paquete `5288c3d` de Marco.
   El sitio, medido por el responsable:
   las dos puntas se ven y pasan 20-30 vehiculos entre 00:00 y 04:00. **Riesgo aceptado por el responsable:** con
   ambar en los dos, dos vehiculos pueden encontrarse de frente.
   - **(a) Que hace.** En los dias y franjas programados las dos puntas destellan: **ambar intermitente en los dos por
     defecto** o, elegido en la app, Poste 1 ambar y Poste 2 rojo intermitente (norma 4.4.3). **Pluma ARRIBA en los
     dos** durante la franja: la condicion de pluma de `escribirPines()` incluye los dos estados nuevos, como ya
     incluye `S_FALLO`. Es un modo del Maestro, `MODO_NOCHE`, que recuerda el modo del que vino; el Esclavo no tiene
     modo: obedece la orden de radio de (e).
   - **(b) Quien manda y quien guarda (`D-107`).** El Maestro guarda las franjas y decide entrada y salida con SU hora;
     el Esclavo no guarda nada de la franja: recibe `CMD_GO_NOCHE` con el color. Motivo: por radio el Esclavo solo
     tiene el dia del mes (`CMD_HORA_D`, `D-26`), sin dia de la semana, y dos horarios serian dos copias (`D-59`).
     **Sin radio el Esclavo hace lo de siempre:** `S_FALLO` a los `SFTY6_SILENCIO_MS` y su Degradado si lo dejaron
     activado (`A-15`); el Maestro en noche deja entrar al Degradado automatico igual que desde `AUTO`. Con rojo
     intermitente elegido, perder la radio lo vuelve ambar (el por defecto aceptado) y la app lo dice.
   - **(c) Franjas y donde caben (`D-108`).** Hasta `NOCHE_FRANJAS` = 4, cada una con mascara de 7 dias, inicio y fin
     en minutos del dia (cruza medianoche si fin <= inicio; `inicio == fin` = apagada), color, y comun a todas el
     todo-rojo `tr` de (d). En una pagina de flash PROPIA, `NOCHE_FLASH_DIR` = `0x0800F800` (la anterior a la del
     testigo), con `board_upload.maximum_size` = 63488 en los dos `platformio.ini`; molde `testigo_flash` (firma,
     version, suma, relectura; `-e all` la borra al cargar: tras una carga no hay franjas). Medido en `respaldo.cpp`:
     `BKP->DR1..DR10` estan todos ocupados (firma; DR2/DR3 con `ANTES`/`REIN` en el byte alto; DR4 con 9 bits de
     banderas; DR5-DR8; DR9/DR10 el ciclo automatico), y una franja son ~40 bits. Se escribe solo al cambiarla.
   - **(d) El todo-rojo (`D-106`, `D-109`).** Un solo parametro `tr`, de fabrica `NOCHE_TR_FABRICA_S` = 180 s, que el
     firmware admite entre `DESPEJE_SEG_MIN` (10 s: el mismo suelo y el mismo significado fisico que el despeje del
     ciclo, lo que tarda el tramo en vaciarse) y `NOCHE_TR_MAX_S` = 900 s (la fase mas larga del equipo,
     `VERDE_MIN_MAX`). El firmware no conoce la distancia: la app se la pide al administrador y propone
     `tr` = max(180 s, distancia / (20 km/h) redondeado ARRIBA a 10 s) (`D-116`); no deja guardar menos de esa
     propuesta ni menos de 180 s, y el administrador solo puede subirlo. Si la propuesta pasa de los 900 s del
     firmware la app NO la recorta: se niega a guardar y dice por que, sin jerga. No se quema un unico valor: lo que
     obligo a cambiar firmware y app fue el verde fijo de `D-35` (`D-59`).
   - **(e) Entrada (`D-111`).** Por FLANCO: en el minuto de inicio de una franja, con hora fiable (SFTY-18: sin hora no
     entra) y el Maestro en `MODO_AUTOMATICO` o `MODO_INTELIGENTE`; en cualquier otro modo (una persona en `MANUAL`,
     fuera de servicio, Degradado, ambar de emergencia, y la propia noche: una franja que empieza cuando otra acaba o
     dentro de ella) esa franja se salta y se anota `FRANJA_SALTADA` (`D-115`). Un corte dentro de la franja
     no reanuda: el flanco ya paso y el equipo arranca como siempre (`D-40`, `D-29`). La transicion es la de
     `SET_MODO:MANUAL`: `coordinador_forzarRojoTotal()` (amarillo 3 s si habia verde, `D-45`; `GO_RED`), espera el
     `ACK_RED` del Esclavo (`coordinador_rojoEsclavoConfirmado()`), rojo fijo en los dos durante `tr`, y entonces
     `CMD_GO_NOCHE` (param: bit0 = Poste 2 rojo intermitente) y la luz propia. Sin `ACK_RED` no entra: manda SFTY-6
     como hoy. El Maestro reenvia `CMD_GO_NOCHE` cada `NOCHE_REENVIO_MS` = 10 s mientras dure; para el Esclavo es
     orden de gobierno (renueva el silencio como `GO_RED`) e idempotente: tras un `S_FALLO` por radio perdida, la
     siguiente lo devuelve a su luz de noche.
   - **(f) Salida (`D-106`, `D-111`, `D-115`).** Por hora: en el primer minuto con hora fiable FUERA de la franja en
     curso, `[inicio, fin)` contado en minutos desde 2000 al entrar; asi la saca tambien un reloj que salta el minuto
     de fin (re-siembra, `SET_RTC`), que retrocede antes del inicio o que vuelve fiable pasado el fin. O desde la
     app. SIEMPRE por el todo-rojo: rojo
     fijo en los dos (desde el destello el rojo es directo: no hubo verde, SPEC 1 §3.3 (3)), `ACK_RED`, `tr` segundos,
     y despues el destino. Por hora, el modo del que vino (`modoActual_set()` y su `setup()`, con el todo-rojo de
     entrada que ya tiene: se acepta el doble rojo). Desde la app, a `AUTO`, `INTELIGENTE` o `MANUAL` (con PIN, como
     hoy): `RESULT:SALIENDO_TODO_ROJO` y el cambio de modo al cumplirse `tr`; una segunda orden mientras tanto,
     `$ERR ... DESC:SALIDA_EN_CURSO`. A `MENU`, `ALCANCE` y `FORZAR_ROJO` (sin PIN, como hoy: solo ponen rojo) el rojo
     y el cambio de modo son inmediatos y contestan como fuera de la noche: el todo-rojo protege el PRIMER VERDE y
     esos destinos no dan ninguno. `SET_MODO:DEG_T` y `SET_MODO:AMBAR` (con PIN) no son salidas de la noche: entrarian
     con su propio rojo y sin `tr`, y se rechazan sin tocar luces, `$ERR,CMD:<orden>,DESC:EN_NOCHE_SALGA_PRIMERO`.
     Una salida por la app vale hasta el fin de esa franja (el flanco de inicio ya paso).
     Si la hora deja de ser fiable DENTRO de la noche, se queda (la luz no depende del reloj) y publica
     `$ALARM,NODE:MAESTRO,EVENTO:NOCHE,CAUSA:HORA_NO_FIABLE,<enlace>,ACCION:SALGA_POR_APP,HORA:..`
     cada `AVISO_ROJO_SIN_HORA_MS` (molde `D-67`); al volver la hora, si el minuto ya
     esta fuera de la franja, sale por hora.
   - **(g) Que publica (`D-110`).** `$STATUS` con `MODO:NOCHE` y `ESTADO:AMBAR_INT` o `ESTADO:ROJO_INT`: estados
     nuevos `S_AMBAR_INT` (las dos puntas) y `S_ROJO_INT` (Esclavo), con la cadencia de `S_FALLO`; `FALLO COM` sigue
     siendo solo la averia de radio. Caben en las cotas de hoy, `modoStr` 11 y `estadoStr` 9, sin campo nuevo: el
     peor `$STATUS` del Maestro cabe con 3 B de margen (`esp32_07`, 15/10) y no se toca. Eventos `NOCHE`: `ENTRADA`,
     `SALIDA_HORA`, `SALIDA_APP`, `FRANJA_SALTADA`. Las franjas se leen con `CONSULTA_NOCHE`, nunca en el `$STATUS`.
     Sin radio dentro de la noche el Maestro publica una vez
     `$ALARM,...,EVENTO:FALLO_RF,CAUSA:SILENCIO_EN_NOCHE,...,ACCION:SIGUE_EN_NOCHE` y sigue destellando (b).
   - **(h) Las ordenes (`D-110`), molde `SET_TIEMPOS`: la pregunta DENTRO del `if`, un `$ERR` por motivo, `RESULT:OK`
     solo si lo releido de la flash es lo pedido.** `SET_NOCHE:<n>,<dias>,<HHMM>,<HHMM>,<A|R>` (n = 1..4; dias = siete
     letras `LMXJVSD`, `-` en su sitio para apagar el dia; A = ambar en los dos, R = Poste 2 rojo) y
     `SET_NOCHE:<n>,OFF`; `SET_NOCHE_TR:<s>`; las dos con PIN. `$ERR` por `FORMATO_INVALIDO` (el `%c` centinela),
     `RANGO`, `EN_NOCHE_SALGA_PRIMERO` (no se cambia la franja en curso) y `FLASH_NO_RELEE`. `CONSULTA_NOCHE` sin PIN:
     `$ACK,CMD:NOCHE,RESULT:OK,TR:<s>,F1:<dias>-<HHMM>-<HHMM>-<A|R|OFF>,...,F4:...` (131 caracteres como mucho, 136
     con `*CRC` y CRLF: cabe en `payload[155]` y en los 160 del puente). Solo el Maestro las tiene; el Esclavo
     contesta `COMANDO_NO_SOPORTADO_EN_ESCLAVO` con PIN y `AUTH_FAILED`/`PIN_INVALIDO` sin el (SPEC 4 §3.2). La
     mas larga mide 31 caracteres y cabe en `btBufIn[64]` detras de `CMD:PIN:1234:`.
   - **(i) La app (`D-110`).** Zona de administrador (`admin-only`, PIN): distancia entre postes, `tr` propuesto y
     editable solo hacia arriba, cuatro franjas con dias, horas y color, y lo guardado releido con `CONSULTA_NOCHE` al
     conectar; cabecera `MODO:NOCHE` con texto y color propios (`MODOS` de `app.js`), `AMBAR_INT`/`ROJO_INT` en las
     tablas de estado, y un aviso al entrar y al salir. Copias de `app.js` y `js/`: SPEC 4 §9.8 (8).
   - **(j) Quien lo ejerce (simulador congelado, `CLAUDE.md` §4: se amplian los existentes), cada uno visto en rojo
     antes del codigo (`CLAUDE.md` §9).** `Validacion_Automatico` (`arnes_automatico.cpp` compila el coordinador
     real): entrada por flanco con `ACK_RED`, `tr` contado, salida por hora y por la app, un `$ERR` por motivo;
     `costura_10`: `reloj_esHorarioNocturno()` deja de ser huerfana (sale de la lista o falla); `esp32_07`: cotas con
     `NOCHE`/`AMBAR_INT`; `barrera_01`: los estados nuevos escriben pines solo por `escribirPines()`; `app_02`/`app_04`:
     modos simetricos y valores de `$STATUS`; `tests/dom_*`: la pantalla y la cabecera.
11. **EL SELLO DE FIRMWARE PERDIA EL CERO INICIAL DE UN HASH QUE ES SOLO CIFRAS (medido el 15/10; `D-113`,
   construido en `N-179`: `strings` del `firmware.bin` del Maestro y del Esclavo da `FW:6115e59+SUCIO`).**
   El paquete `0479942` contesto `FW:479942`: el hash viaja como token sin comillas (`version_fw.h`
   explica por que no las lleva: el escape se rompe en silencio entre el shell y SCons) y platformio convierte a
   entero todo valor de `-D` que sea solo cifras (`piobuild.py:209`, `int(value)`; con un punto, `float`). Ningun
   instrumento lo veia: `version_01_sello_de_firmware` mide la linea del `.ini` y los literales, no un hash concreto.
   - **Lo que hace:** el one-liner de los tres `platformio.ini` imprime el hash como siete tokens de UN caracter
     separados por coma en una sola macro,
     `-DFW_HASH=0,4,7,9,9,4,2` (un caracter solo no pierde nada al pasar por `int()`), y
     `version_fw.h` los une al stringificar (`FW_UNIR(...)` llama a `FW_UNIR7(a,b,c,d,e,f,g) #a #b #c #d #e #f #g`: el
     nivel intermedio de siempre). `FW_HASH.txt` del empaquetador sigue siendo `<7 hex>`; la trama `FW:<7 hex>`, app y
     `FW_SUCIO` no cambian. La linea sigue IDENTICA en las tres puntas (A-3).
   - **Quien lo ejerce:** `version_01` gana el caso A-9: ejecuta el one-liner real leido del `.ini` con un
     `FW_HASH.txt` = `0479942` en un directorio temporal y sin `git` en el `PATH`, y exige que ningun valor de `-D`
     tenga mas de un caracter entre comas y que la union de los siete sea `0479942`; y, con `g++` a mano, `-E` sobre
     `version_fw.h` con esas macros da el literal `"0479942"`. Visto en rojo antes de tocar el `.ini`. La medida que
     cierra es `strings` sobre el `.elf` de un commit cuyo hash empiece por `0`, en el acta.
