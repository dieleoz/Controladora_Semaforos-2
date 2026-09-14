# SPEC 0 — EL SISTEMA: que es, y como se pone un cruce a funcionar

**Version del equipo: `V9.1`** *(14/09/2026)*. Sube desde la `V9.0` del 12/09 por **dos cambios que se ven desde la carretera**, y ninguno de los dos cabe dentro de un numero de version ya entregado:

- **El verde del modo sin radio pasa de medio minuto a TRES MINUTOS.** Es el cambio mayor: cuando los dos postes pierden el enlace y se reparten el paso por reloj, cada lado da paso tres minutos en vez de treinta segundos. Lo que hace ese modo, en `SPEC_7`.
- **La barrera ya no baja en el mismo instante en que el semaforo se pone en rojo:** baja unos segundos despues, **para dar tiempo a los vehiculos que ya venian pasando y no golpearlos**, y **no baja en absoluto mientras una camara vea algo debajo**. Lo que hace y lo que no, en `SPEC_5`.

⚠️ **Y esta version NO ha visto una tarjeta.** Lo que hay instalado en campo sigue siendo la `V8.4`.

⚠️ **Los manuales de `04_Manuales/` y del resto de `05_Funcional/` siguen rotulados `V9.0`, y es a proposito:** no se reescriben hasta que haya banco y tarjeta, porque reescribir un manual antes de que el equipo este quieto es tirar el trabajo dos veces. **Las siete spec SI estan al dia.** La unica excepcion son las guias `.html` de campo, que se corrigen siempre: las sigue un instalador con un destornillador en la mano.

**La puerta de entrada.** Quien no ha visto este repositorio empieza aqui y sale sabiendo **que es el producto**, **como se
opera un cruce de principio a fin** y **que NO hace el equipo**. Escrita el 13/09/2026 con **cuatro agentes escribiendo en el
arbol a la vez**: todo lo medido esta fechado al commit **`ef3504c`** (`git show ef3504c:<fichero>`). **Mandan sobre este
fichero** `DECISIONES.md` en lo decidido, `05_Funcional/17_...` en el cobre medido, y **cada una de las otras siete spec en su
materia** — aqui vive el puntero, nunca el desarrollo. Se cita el **SIMBOLO**, nunca la linea (`CLAUDE.md` §7.3), y **ninguna
cifra vive aqui** (§14).

## 0. 🔴 LO QUE SALIO AL RECONSTRUIR LA OPERACION — y por eso va primero

**A · NO HAY ORDEN PARA LEER LOS TIEMPOS QUE EL EQUIPO TIENE DENTRO, Y EL FORMULARIO DE LA APP MIENTE POR OMISION.** Medido:
el despachador del Maestro (`Maestro/src/bluetooth.cpp`) no tiene rama que devuelva verde/rojo/despeje —la lista entera es
`SPEC_4` §3.1—, el `$STATUS` no los lleva (`SPEC_4` §5), y **`app.js` nunca escribe en `num-tiempo-verde`/`-rojo`/`-despeje`:
solo los LEE al enviar**. El tecnico que llega a un cruce en marcha ve **los valores por defecto del `.html`**, no los del
equipo, y **si pulsa «Aplicar tiempos» sin tocar nada los graba**. **No es un hueco de documento: falta la orden.**

**B · UN CORTE DE ENERGIA EN AUTOMATICO DEJA EL CRUCE PARADO EN ROJO, Y NADIE LO AVISA NI LO ARRANCA.** Medido en
`Maestro/src/main.cpp`: el `setup()` tiene **dos salidas y solo dos** — reanuda **Modo Degradado** si
`modo_degradado_reanudarTrasCorte()` lo autoriza (`D-29`), y **en cualquier otro caso entra en `MENU`**, el todo-rojo de las
dos puntas. **El Automatico no se reanuda: no se guarda el modo, solo los tiempos**, y volver a darlo exige **una persona con
el telefono en el poste**. `SPEC_3` §6 y `SPEC_6` A.5.3 documentan la reanudacion del Degradado; **la del modo normal no.**

**C · «Operacion normal» NO estaba escrita, y lo confirma el instrumento.** `grep -rincE "puesta en marcha|operacion
normal|primera vez" 05_Funcional/SPEC_*.md` da **0 en las siete** (reproducido hoy). Lo unico que habia: el procedimiento de
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
2. **Donde queda el equipo al encender.** El **Esclavo pone rojo en las tres primeras lineas de su `setup()`** —«las luces
   primero, siempre»—; el **Maestro arranca A OSCURAS** y su primer rojo no llega hasta `menu_setup()` →
   `coordinador_forzarMenu()` (`SPEC_1` §12.5). Ya arrancado: **Maestro en `MENU`, rojo fijo en las DOS puntas** repetido por
   el latido, **pluma ABAJO**; ambar intermitente en vez de rojo si no hay enlace. **Excepcion unica:** si estaba en Degradado
   cuando se fue la luz, reanuda en Degradado (`D-29`). **Nunca reanuda el Automatico** (§0.B).
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

## 3. EL MAPA DE LAS SIETE SPEC — a donde ir

| | contesta |
|---|---|
| **`SPEC_1` Ciclo y Luces** | que hace **un** poste con sus tres luces y su pluma: modos, transiciones, barrera de salidas, estado seguro |
| **`SPEC_2` Dos Puntas y Radio** | como se coordinan los dos postes, y que pasa con la radio sana, degradada y muerta |
| **`SPEC_3` La Hora** | de donde sale la hora, quien siembra a quien, cuando CADUCA y que se hace entonces |
| **`SPEC_4` App y Bluetooth** | **todas las ordenes y que contesta el equipo a cada una**; telefono → ESP32 → STM32 |
| **`SPEC_5` Cobre, Camaras y Pluma** | que hay en cada pin y que es peligroso. Se lee **con un destornillador en la mano** |
| **`SPEC_6` Campo, Radio y Alarmas** | el Degradado paso a paso, la configuracion de las radios, y **que hace el tecnico con cada alarma** |
| **`SPEC_7` Modo Sin Reloj** | por que ese modo **no se construye**, y que si falta (que la app diga POR QUE) |

## 4. GLOSARIO

- **Punta** — cada uno de los dos controladores. «Las dos puntas» = poste 1 + poste 2.
- **Cruce** — la instalacion entera: los dos postes y el tramo de un carril que regulan.
- **Paso alternado** — un solo carril util: se da paso a un sentido cada vez, nunca a los dos.
- **Cabezal** — las tres luces de un poste. Sus **dos caras van siempre juntas**: un poste tiene **un** color, no dos (`SPEC_1` §1).
- **Pluma / talanquera** — la barrera que baja sobre el carril. **REFUERZA la senal, no la sustituye**: quien regula es la lampara (`SPEC_1` §2).
- **Despeje** — el **todo-rojo entre los dos verdes**, que vacia el tramo. Es la **unica proteccion del que se quedo dentro** (`SPEC_1` §5).
- **Degradado (Modo)** — sin radio, cada punta decide su luz **por su reloj**: el unico que da verde sin confirmar con la otra. Entrada **siempre manual** (`SPEC_2` §7).
- **Orfandad** — que a una punta deje de llegarle nada de la otra durante `SFTY6_SILENCIO_MS`; se responde con **ambar intermitente** (SFTY-6, `SPEC_2` §4).
- **Siembra** — el envio periodico de la hora del ESP32 a su STM32 por el cable `J17` (`SPEC_3` §2).
- **Vale de via despejada** — lo que la app pregunta antes de una orden que **abre paso**: un PIN demuestra QUIEN eres, no que hayas MIRADO (`SPEC_4` §4).
- **PMT** — quien dirige el manejo de transito de la obra; consta como autor, con el cliente, de una decision vial del 27/08/2026 (`SPEC_5` §4). ⚠️ **El repositorio usa la sigla sin desarrollarla nunca**, y aqui no se inventa.

## 5. 🔴 LO QUE EL EQUIPO NO HACE — en un solo sitio, y **medido** en la spec que cada linea cita

1. **NO MIDE LA BATERIA.** `BAT:` sale `--` **fijo en las dos puntas** —falta el divisor y la entrada analogica—, y la app lo
   dice con esas palabras en vez de pintar un numero (`SPEC_4` §6).
2. **NO GOBIERNA CABEZAL PEATONAL NI ZUMBADOR, AUNQUE LOS PINES EXISTAN.** `ROJO_PEATON`, `VERDE_PEATON` y el buzzer estan
   **declarados y muertos** —ni un `pinMode` ni un `digitalWrite`—: **cableados a `J11`/`J9`/`J13` no se encienden nunca y no
   hay mensaje de error** (`SPEC_5` §2, `SPEC_1` §1). **No se venden como funciones del equipo.**
3. **NO DISTINGUE UN VEHICULO PARADO DEBAJO DE UNA CAMARA MAL APUNTADA.** 🟢 ~~Ninguna camara protege la pluma~~ →
   **desde el 14/09 SI la protege**: la barrera baja 3 s despues del rojo y **no baja mientras una camara vea algo debajo**
   (`SPEC_5` pag. 1 §4). 🔴 **Lo que el equipo NO puede hacer es juzgar lo que ve:** un vehiculo parado y una camara
   que dispara sola **dan el mismo contacto cerrado**, asi que ante la duda **no baja y avisa** —y quien decide si eso es
   una averia o trafico es una persona, no el equipo—. Y **los ultimos metros de la bajada no los ve nadie**: no hay fin
   de carrera, y esta **sin preguntar al fabricante** si la centralita trae fotocelula o borde sensible.
4. **NO HAY AMBAR AL CERRAR EL VERDE: salta de VERDE a ROJO.** **El conductor no recibe ningun aviso de que el verde se
   acaba**; en su lugar hay un margen, no un aviso: el todo-rojo de despeje (`SPEC_1` §3.1). Y **la pluma baja en el MISMO
   instante del rojo** — el retardo que pidio el responsable **no esta implementado** (`SPEC_1` §12.1).
5. **EL PIN VIAJA EN CLARO Y CUATRO ORDENES NO LO PIDEN.** El transporte es Bluetooth SPP **sin cifrar**; las cuatro que
   **cambian algo** sin clave estan contadas en `SPEC_4` §7.1. Y **`SET_RTC` con un PIN falso pone la hora** — riesgo
   **aceptado** por el responsable, `D-26` (1).
6. **NO HAY AVISO REMOTO: si no hay nadie conectado, nadie se entera.** Ni red, ni SMS, ni servidor: `$ALARM` sale **en el
   instante** y **solo lo ve quien esta conectado entonces** (`SPEC_6` PARTE C). Y el aviso previo al limite duro del
   Degradado **esta declarado y no se ejerce**: su lector era el LCD, retirado (`SPEC_7` §5).
7. **NO ENTRA EN DEGRADADO SOLO, Y LA APP INVENTA LA CAUSA.** La entrada es siempre de una persona; el equipo publica `MODO:`
   y `ESTADO:` y **nunca la causa**: los textos de causa que el operario lee estan escritos en `app.js` (`SPEC_7` §6, H-2).
8. **NO SE DIAGNOSTICA A SI MISMO EL COBRE NI LA RADIO.** Ningun instrumento lee la configuracion de una radio (`SPEC_6`
   HUECO 3); la pluma **no tiene realimentacion** —sabe «ordene abrir», nunca «esta abierta» (`SPEC_1` §2)—; y **una camara
   muerta desde la instalacion no la avisa nadie** (`SPEC_5` §3.2).
9. ⚠️ **Y lo que no se puede afirmar de nada de esto: NO HA VISTO UNA TARJETA.** Ninguna camara se ha conectado nunca a este
   equipo, y que firmware corre en cada poste **lo dice `ESTADO.md`, no este fichero** (`CLAUDE.md` §0.2).
