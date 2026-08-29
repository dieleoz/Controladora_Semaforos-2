# 🗺️ Roadmap y Estado del Ecosistema de Semáforos Móviles (V9.0)

**Fecha de Actualización:** 28 de Agosto de 2026 · **HEAD:** `e82fddc` · rama `main`  
**Compuerta de Verificación:** ✅ **15 PASS | 0 FALLA | 0 ABORTADO** (Exit code: 0) · Maestro: 88.3% Flash (57.880 B) · Esclavo: 64.4% Flash (42.176 B) · Repetidor: 20.6% Flash · 43 rutas parseadas · 405/405 en 38 packs · 271/271 pantalla · 29/29 ciclo · 71/71 automatico, en `evidencia/2026-08-28_compuerta.txt`

> ⚠️ **El acta se cabecea a si misma como `26479d9`, rama `main-nuevo`, «Arbol: CON CAMBIOS SIN
> COMMITEAR», y ella misma cierra con su AVISO.** Y esta vez **se puede decir exactamente sobre que
> arbol midio, porque dos de sus cifras lo delatan**: `43 rutas parseadas` y `57880 bytes` son
> justo las que `e82fddc` reporta —a `26479d9` le correspondian **42 rutas y 57.892 B**—. Es decir:
> el acta midio el arbol de trabajo que un rato despues se convirtio en **`e82fddc`**, y se cabecea
> con el commit anterior. Se copian aqui **con esa advertencia pegada**, no limpias: un acta que no
> corresponde a un commit no es una medida reproducible, y la unica forma de convertirla en una es
> volver a `e82fddc` y re-correr `python 01_Firmware/compuerta.py`. **Ese re-corrido no se ha
> hecho — es la tercera vez seguida que se publica un acta cabeceada con el commit equivocado, y
> arreglarlo cuesta una corrida de compuerta sobre el arbol limpio.**

---

## 📦 28 de Agosto de 2026 · El Bluetooth sale a una bornera, y con el se cae media arquitectura

> **Como se lee esta seccion.** Va en orden descendente, como el resto del fichero, pero el orden
> causal es el inverso: **N-76** movio el puerto serie a un conector enchufable, **N-77** recogio la
> arquitectura que el cliente decidio encima de ese movimiento, y **N-78 a N-87** son lo que
> aparecio al censar que se rompia. Ninguno de los diez ultimos se pregunto: salieron de cruzar la
> superficie de entrada del firmware con lo que se retira (CLAUDE.md §3.ter).
>
> **Y los dos ultimos, N-88 y N-89, salieron de ARREGLAR los anteriores**, que es el sitio donde
> este repositorio ya sabe que aparecen defectos: **N-88** es la asimetria que quedo a la vista al
> escribir las ramas nuevas del despachador, y **N-89** es un ahorro de flash que hubo que
> **rechazar** porque apagaba el instrumento recien construido. Ninguno de los dos se pregunto
> tampoco.
>
> 🔴 **Y la linea que manda sobre todas las demas: hoy no existe ni una sola fila «VERIFICADO EN LA
> PLACA».** Lo que aqui se llama MEDIDO se midio **sobre ficheros** —el `.cpp`, el `.h`, el
> `.kicad_sch`, el `.kicad_pcb`, el `.elf`—. Un fichero dice lo que alguien dibujo o escribio; una
> placa dice lo que se fabrico. **La sesion de banco sigue pendiente.**

| | estado |
|---|---|
| **N-76** Bluetooth a `J17`, `USART1` remapeado a `PB6`/`PB7` | 🟢 **CERRADO** — `020c2db` + `50a5380` |
| **N-77** La arquitectura del 28/08 (STM32 controlador · ESP32 expansion) | 🟢 documentada — `05_Funcional/17_...md` |
| **N-78** `botonCancelar()` es la unica salida de todos los modos | 🟡 **MITAD CERRADA** — `d34cfe2` pone los seis comandos; `PB15` sigue siendo la unica salida **fisica** |
| **N-79** Retirar el mando **borra un veto**, no deja `if` inertes | 🔴 ABIERTO |
| **N-80** `SET_RTC` rechaza en silencio y contesta `RESULT:OK` | 🟢 **CERRADO** — `d34cfe2`, y eran **tres** ramas, no una |
| **N-81** Telemetria fabricada: `RF`, `RTT`, `BAT` y `T` son literales | 🟠 ABIERTO |
| **N-82** `TEST_LEDS` escribe pines por fuera de SFTY-2 | 🟢 **CERRADO** — `caef8a1`, y **espejado en las dos puntas** porque el instrumento no dejo cerrar a medias |
| **N-83** `FORZAR_ROJO` del Esclavo: el nombre y el efecto no coinciden | 🟢 **CERRADO** — `caef8a1`, pasa a `CMD:AMBAR_EMERGENCIA` |
| **N-84** Contradiccion de polaridad en `J16` | 🔴 ABIERTO — **bloquea el cableado de camaras** |
| **N-85** El `.kicad_pcb` NO esta vacio | 🟢 identificado, correccion en curso por otra sesion |
| **N-86** `AiBus`: 280 B de RAM que el enlazador no puede descartar | 🟢 **CERRADO** — `26479d9`, **−280 B de RAM por punta** medidos con `nm` en los dos extremos |
| **N-87** La compuerta no es idempotente despues de un `--rapido` | 🟠 ABIERTO — no es defecto del pack |
| **N-88** Dos criterios distintos para abandonar el Modo Degradado | 🔴 ABIERTO — **decision de spec, no de implementacion** |
| **N-89** Un ahorro de 636 B que se RECHAZO: apagaba un pack sin romperlo | 🟢 **DECIDIDO Y ESCRITO** — regla permanente |
| **N-90** `ModoSistema` sale de la cabecera de la pantalla | 🟢 **CERRADO** — `e82fddc`, **Fase 0** de la retirada del LCD · −12 B · una copia de modelo menos |
| **N-91** El presupuesto de retirar la pantalla | 🟠 **ABIERTO — MEDIDO Y SIN EJECUTAR** · **19.513 B** en el Maestro, **9.659 B** en el Esclavo · y **tres instrumentos que no avisan** |
| **N-92** El orden entre camaras y botones **no** es «mismo commit» | 🔴 ABIERTO — **corrige a N-84**: el requisito es asimetrico y **operativo**, no de historia |

---

### 🔴 N-92 — El orden entre camaras y botones NO es «mismo commit»: es «firmware CARGADO antes que el destornillador»

**De donde sale:** de ir a escribir el procedimiento de N-84 y descubrir que la formulacion que se
venia usando —*el firmware que libera esos pines y el cableado de las camaras van en el mismo
commit*— **era mas estricta de lo necesario y en la direccion que no protege**. Es la regla del
instrumento aplicada a una regla: una salvaguarda que suena severa no es una salvaguarda si lo que
vigila no es lo que mata. **La version corregida ya vive en `CLAUDE.md §9.bis`**; esta entrada
guarda la medida y el porque.

**MEDIDO, sobre el fuente y sobre el cobre:**

```
Maestro/include/pines.h:94   #define BOTON3   PB14   // Aceptar
Maestro/include/pines.h:95   #define BOTON4   PB15   // Cancelar
Maestro/src/botones.cpp:131  bool botonAceptar()  { return consumir(2); }   // -> BOTON3 = PB14
Maestro/src/botones.cpp:132  bool botonCancelar() { return consumir(3); }   // -> BOTON4 = PB15
Maestro/src/botones.cpp:16   const unsigned long FLANCO_MS = 200;

MAPEO_TARJETA_KICAD.md 7.bis  J16.10 -> U1.27 -> PB14   (R67 + C28, cobre continuo)
                              J16.12 -> U1.28 -> PB15   (R68 + C29, cobre continuo)
```

**`PB14` no es un boton cualquiera: es `botonAceptar()`, EL QUE EJECUTA.** `PB15` es
`botonCancelar()`. Y con `FLANCO_MS = 200` el antirrebote admite un flanco cada 200 ms: **cada coche
que dispare la Camara 2 seria un ACEPTAR, hasta cinco por segundo**, sobre el menu que arranca modos.
No es ruido en una entrada muerta — es el pulsador de confirmacion del gabinete accionado por el
trafico.

> 🔴 **Pero el requisito real es ASIMETRICO, y por eso la regla vieja estaba mal escrita.** Las tres
> combinaciones no son equivalentes:
>
> | orden | que pasa | veredicto |
> |---|---|---|
> | **firmware primero** | sin `botones_setup()` los pines quedan como entradas sin configurar, y **`R67`/`R68` los fijan a 0 V** —son pull-**DOWN** a `GND`, medidos sobre el cobre en `MAPEO §7.bis.3`—. No hay lectura y no hay flotante | ✅ **seguro** |
> | **mismo commit** | el caso ideal, pero solo describe el repositorio | ✅ seguro |
> | **cableado primero** | la camara entra por `J16` a un `PB14` que el firmware **sigue leyendo** como `botonAceptar()` | 🔴 **NO seguro** |
>
> **La regla operativa que queda, y que un commit no puede dar:** *el firmware que deja de leer esos
> pines tiene que estar **CARGADO EN LA TARJETA** antes de que nadie enchufe nada a `J16`.*

**Por que esto es un N-x y no una correccion de estilo.** Exigir «mismo commit» **suena** mas seguro
y **protege menos**: se cumple con un commit que nadie ha cargado, y no dice nada del unico momento
que importa, que es el del destornillador delante de la bornera. Y al reves, prohibe el orden que
**si** es seguro —cargar el firmware hoy y cablear la semana que viene—, que es ademas el que un
tecnico haria de forma natural. **Un commit no protege de un destornillador**, y una regla que se
apoya en la historia del repositorio para vigilar el mundo fisico esta vigilando el sitio equivocado.

**Que haria falta para cerrarlo.** Tres cosas, y ninguna es un commit: **(a)** que N-84 se resuelva
—mientras la polaridad de `J16` este en contradiccion no hay firmware correcto que cargar—; **(b)**
que el paso *«cargar el firmware sin lectura de `PB14`/`PB15`»* aparezca **antes** del paso de
cableado en el protocolo de banco de `05_Funcional/`, con su verificacion de que la carga entro; y
**(c)** que la etiqueta fisica de `J16` lo diga, porque `J16` y `J17` son el mismo `Molex KK-254` de
16 posiciones y estan uno al lado del otro (§7 del mapeo).

---

### 🟠 N-91 — El presupuesto de retirar la pantalla: 19.513 B MEDIDOS, y tres instrumentos que no avisarian

**De donde sale:** de N-90. Cerrada la Fase 0, la pregunta siguiente es cuanto devuelve de verdad
retirar el LCD — y la respuesta honesta tenia que salir del `firmware.map` **por fichero objeto**, no
por nombre de simbolo, porque clasificar por `_Z...` mete libreria ajena dentro de *«lo nuestro»*
(§7, el primer censo de N-70 se equivoco exactamente asi).

**MEDIDO por fichero objeto sobre `01_Firmware/*/.pio/build/*/firmware.map`:**

| | Maestro | Esclavo |
|---|---|---|
| `lcd.cpp.o` | **4.439 B** | 1.743 B |
| `menu.cpp.o` | **390 B** | 997 B |
| U8g2 (todos sus `.o`) | **13.936 B** | 6.919 B |
| **subtotal pantalla** | **18.765 B** | **9.659 B** |
| `modo_hora.cpp.o` | 748 B | — (no existe) |
| **total retirable** | **19.513 B** | **9.659 B** |
| RAM de esos objetos | **1.269 B**, de los que **1.024 son el framebuffer** (`u8g2_d_memory.c.o`) | 1.254 B |

**Todos los objetos de U8g2 cuelgan de `lcd.cpp.o`; nada mas los toca.** Y **no hay ahorro
secundario**: la cadena `snprintf`/`sscanf` (~3,7 kB) la arrastra `bluetooth.cpp.o`, no `lcd.cpp.o`,
asi que retirar la pantalla **no** se la lleva de propina.

> ⚠️ **Una discrepancia que se deja escrita en vez de taparse (§4, «manda la medida»).** El censo
> entregado con el encargo daba `menu.cpp.o` = **490 B**, subtotal Maestro **18.865 B** y Esclavo
> **9.675 B**. El re-conteo sobre el `.map` que HEAD deja en disco da **390 / 18.765 / 9.659**, y
> coincide **al byte** en `lcd.cpp.o` (4.439), U8g2 (13.936) y `modo_hora.cpp.o` (748). Los 100 B de
> `menu.cpp` **se explican**: son la Fase 0 —`e82fddc` saco `modoActual` de `menu.cpp` a
> `modos.cpp.o` (+24 B)— asi que el 490 es la cifra **de antes** de N-90 y el 390 la de despues. Los
> 16 B del Esclavo **no se explican**, y el Esclavo no lo toco N-90. **No se elige un numero: antes
> de ejecutar la Fase 2 se vuelve a medir sobre un `.map` recien enlazado.**

**Y lo que la cifra NO dice, que es lo importante y lo unico que puede hacer daño:**

- 🔴 **`Validacion_LCD` tiene DIEZ comprobaciones que no miden un solo pixel.** Contadas una a una en
  el fuente: cuatro en `arnes_lcd.cpp` (`:815` que `CONFIGURACION` **no arranque ningun modo**;
  `:858` el regreso del submenu; `:875` y `:895` que cada opcion seleccione **su** modo, *incluido
  `MODO_DEGRADADO`*) y seis en `arnes_esclavo.cpp` (`:660`, `:668`, `:672`, `:940`, `:948`, `:954`,
  la inhibicion **SFTY-21** del mando del suelo mientras hay alguien delante del gabinete).
  **`grep` de `menu_loop()` y `menu_estaAbierto()` en todo el arbol: fuera de la tarjeta no los llama
  NADIE mas.** Retirar el arnes sin sustituto no cambia 18,4 KiB por bytes: los cambia por un
  **hueco de cobertura sobre el modo que da verde por reloj** y sobre la unica barrera que impide que
  el mando del suelo se mueva con un tecnico delante.
- 🔴 **`flash_01_lastre` hace `raise Abortado`** (`:95`) si **ninguna** punta declara U8g2 en su
  `platformio.ini`. Quitarlo del `.ini` sin invertir el pack no manda la compuerta a `FALLA`: la
  manda a **exit 2**. Y §3.quater es explicita — *un `ABORTADO` es una puerta abierta, no una casilla
  pendiente*: mientras dure, **todo lo que ese pack vigilaba entra sin mirar**.
- 🔴 **`esclavo_06_no_abre_paso` tiene `except Exception: continue`** (`:118`) dentro del bucle sobre
  `("main.cpp", "bluetooth.cpp", "modo_degradado.cpp", "menu.cpp")`. Si `menu.cpp` desaparece y nadie
  toca esa lista, **la comprobacion sigue pasando midiendo un fichero menos** — sin `FALLA`, sin
  `ABORTADO`, sin que la cuenta baje. Es N-51 otra vez: un `PASS` de algo que ya no se mira.

#### Las cuatro fases minimas, cada una con lo que hay que tocar EN EL MISMO COMMIT

Aqui «mismo commit» **si** es la regla correcta (§5: mover un `.cpp` y actualizar las rutas van
juntos), porque lo que se protege es el repositorio, no una bornera.

| fase | que sale | ahorro | **en el mismo commit** |
|---|---|---|---|
| **0** ✅ hecha (`e82fddc`) | `ModoSistema` de `menu.h` a `modos.h` | −12 B | `app_02_modos_simetricos` (su ruta), `Validacion_Automatico/menu.h` (deja de copiar el enum), `Validacion_LCD/compilar.ps1` (enlaza el `modos.cpp` real) |
| **1** `modo_hora` | la pantalla AJUSTAR HORA | **748 B** | `main.cpp` (`:207` y `:221`), `modos.h`, `menu.cpp` (`opcionesConfig`), `arnes_lcd.cpp` (los recorridos del submenu), `maestro_07_menu_opciones`, `app_02_modos_simetricos` (fila `HORA`) |
| **2** `lcd` + U8g2 | la pantalla entera, **`menu.cpp` SOBREVIVE** | **18.375 B** Maestro · **8.662 B** Esclavo · **1.269 B de RAM** | los dos `platformio.ini`, **`flash_01_lastre` invertido** (no borrado), `maestro_06_fuentes_pantalla`, `maestro_03_puerta_degradado` (`:549-550`), la entrada de `Validacion_LCD` en `compuerta.py`, la **guarda de rutas**, y **el sustituto de las diez comprobaciones no-pixel** |
| **3** `menu` + `botones` + `mando` | la maquina de estados pasa a Bluetooth | 390 B + 468 B + 592 B | `esclavo_02_inhibicion_menu` (SFTY-21 entera), `maestro_07_menu_opciones`, **`esclavo_06_no_abre_paso` (su lista de cuatro ficheros, A MANO)**, `maestro_01_mando`, `costura_08_silencio`, `esclavo_01_latch_ambar`, `app_02_modos_simetricos` · **y arrastra N-79 y N-92** |

> 🔴 **La Fase 1 tiene un agujero que conviene saber antes de empezar: NINGUN pack del banco nombra
> `modo_hora.cpp`.** `grep` sobre los 38 packs da cero. Su unica cobertura es indirecta —el enum, el
> menu y `Validacion_LCD`—, asi que borrarlo a secas **no bajaria ninguna cifra**. Su sustituto ya
> existe y esta probado (`SET_RTC`, con sus tres rechazos razonados de N-80), pero la asimetria que
> queda hay que decidirla, no descubrirla en la calle: **`AJUSTAR HORA` es hoy la unica via de poner
> el reloj sin telefono.**

> 🔴 **Y el orden importa por una razon que no es de bytes: `menu.cpp` NO es un menu** (N-90).
> Contiene la maquina de estados del sistema y `menu_setup()` llama a `coordinador_forzarMenu()`, que
> es **la maniobra de puesta en seguro de todo el firmware**. `menu_setup()` se invoca desde **13
> sitios** —`main.cpp` ×2, `bluetooth.cpp`, `modo_alcance`, `modo_ambar`, `modo_automatico`,
> `modo_degradado` ×2, `modo_hora` ×2, `modo_inteligente`, `modo_manual`, `modos.cpp`—. Por eso la
> Fase 2 se para **antes** de `menu.cpp`: retirar la pantalla no puede llevarse por delante el sitio
> por el que el equipo se pone en rojo.

**Nada de esto esta ejecutado, y esa es la fila importante.** N-91 es un presupuesto, no un trabajo
hecho: la unica fase cerrada es la 0.

---

### 🟢 N-90 — `ModoSistema` sale de la cabecera de la pantalla · **CERRADO en `e82fddc`**

**De donde sale:** de preguntar por donde se empieza a retirar el LCD. La respuesta no era «por el
LCD»: era por lo que el LCD **se llevaria de propina**.

> **`menu.cpp` NO es un menu, y descubrirlo vale mas que los bytes.** Contiene `ModoSistema
> modoActual`, que es **la maquina de estados del sistema**: `main.cpp` despacha sobre ella,
> `mando.cpp` decide con ella si inhibe las secuencias del suelo, y `bluetooth.cpp` la escribe desde
> los `SET_MODO`. Mientras el enum viviera dentro del fichero de la pantalla, **retirar la pantalla
> arrastraba la maquina de estados**. Ahora vive en `Maestro/include/modos.h`, que **no incluye
> nada** —ni STM32duino ni U8g2—.

**SOLO EL MAESTRO, y se midio ANTES de escribir** (§4): `grep` de `ModoSistema` y `modoActual` da
**48 coincidencias en el Maestro y CERO en el Esclavo**, que publica `MODO:SUBORDINADO` fijo desde
N-16. Crear alli un `modos.h` por simetria habria sido una cabecera huerfana.

**Tres cosas que aparecieron al hacerlo y merecen quedar escritas:**

- 🔴 **La guarda de rutas salto, y tenia razon.** `ABORTADO: Esclavo\include\modos.h NO existe`.
  **Causa medida, no deducida:** la guarda **expande a las DOS puntas** cualquier pareja
  `("include", "x.h")` escrita **sin rol**, y el pack la habia escrito asi. Se arreglo **en el pack y
  no en la guarda** —la tupla completa `("Maestro", "include", "modos.h")` es la verdad, y esa la lee
  bien—. El censo pasa de **42 a 43 rutas**. Es la red de §5 haciendo exactamente su trabajo, y la
  tentacion de tocar la guarda para que dejara de gritar era la salida equivocada.
- 🟢 **Desaparecio una copia de modelo, que no estaba en el encargo.** `Validacion_Automatico/menu.h`
  **redeclaraba el enum entero a mano**, con un comentario que **ya admitia** que divergir seria *«un
  casi igual capaz de mentir en silencio»*. Ahora toma el real por `-I`. Es §3.bis literal: los
  instrumentos no son pruebas, son **una segunda copia del firmware que alguien sincroniza**, y eso
  ya fallo tres veces. **Una copia menos que sincronizar.** Por lo mismo `Validacion_LCD` enlaza el
  `modos.cpp` **real**: ponerle un doble habria sido medir el menu real contra un estado de mentira.
- **Traslado literal salvo un punto, declarado:** `menu.cpp` ya no puede tocar la variable
  directamente, asi que sus siete `modoActual = X;` pasan a `modoActual_set(X);`. Mismo
  comportamiento, y de ahi salen los **−12 B** (Maestro 57.892 → 57.880 B, Esclavo intacto, RAM
  −4 B). La alternativa —dejar la variable en `menu.cpp`— era justo lo que este trabajo viene a
  deshacer.

**Verificado con el defecto inyectado (§8.bis):** un `MODO_INVENTADO` en el `modos.h` nuevo hace caer
`app_02_modos_simetricos` a `7/8` con salida `1`, **nombrando el defecto**. Restaurado despues.

---

### 🟢 N-89 — 636 B de ahorro que se RECHAZARON: refactorizar puede apagar un instrumento sin romper ni un test

**De donde sale:** de mirar la factura de `d34cfe2`. Los seis comandos de N-78 mas las tres ramas de
N-80 costaron **1.644 B** de flash sobre un Maestro que ya iba al 85,8 %, y la primera reaccion —la
correcta— fue buscar de donde recortarlos.

**Lo que se probo, y lo que ahorraba MEDIDO:** un compositor. Las veintitantas ramas del despachador
terminan escribiendo tramas que solo se diferencian en dos o tres campos, asi que en vez de un
literal completo por rama se escribieron dos funciones —`responderAck(cmd, resultado)` y
`responderErr(cmd, motivo)`— que arman la trama en un buffer. **636 B menos.** Con 7.632 B libres,
eso es casi un 9 % del margen que queda.

**Se retiro, y el motivo no es de estilo. `app_03_sin_ok_mudo` decide leyendo el bloque de cada rama
y buscando LITERALES:**

```
app_03_sin_ok_mudo.py:200    if '"$ACK' not in bloque:
                                 # "Una rama que no promete nada no puede mentir"
                                 return True, None
app_03_sin_ok_mudo.py:208    if '"$ERR' not in bloque:
```

Con el compositor, **ninguna rama contiene ya la cadena `"$ACK`**: la trama se arma dentro de
`responderAck()`, que vive en otro sitio. Asi que la linea 200 se cumple **para todas**, y todas
devuelven `(True, None)` — el veredicto de *«esta bien»*. El pack seguiria corriendo, seguiria
contando sus comprobaciones, y **no estaria mirando ninguna**.

> 🔴 **Lo que lo vuelve peligroso de verdad: sus dos redes tambien sobreviven.**
>
> - La **calibracion** (`:267`) exige que la rama patron `SET_TIEMPOS` del Maestro salga
>   `(True, None)`. Con el compositor sale **exactamente eso** —por el camino equivocado, el de la
>   linea 200— asi que la calibracion **pasa**. La rama que el pack usa para saber distinguir el bien
>   del mal deja de distinguir nada y sigue diciendo que si.
> - Los dos **controles negativos** (`:299` y `:303`) se ejercen contra bloques **sinteticos**
>   escritos dentro del propio pack, que llevan sus `"$ACK` y `"$ERR` literales. El compositor no
>   toca el fuente del pack, asi que los dos controles siguen en verde: el detector **sabe** fallar,
>   solo que ya no se le presenta nada sobre lo que fallar.
>
> Es decir: **cero tests rotos, cero cifras a la baja, cero ABORTADO. El acta habria seguido diciendo
> `371/371`.**

**La regla que queda: un refactor puede apagar un instrumento sin romper ni una prueba, y el banco no
tiene forma de avisarlo.** Es la misma familia que las pruebas 2.7 y 2.8 de N-51 —una condicion
siempre cierta que hacia `break` antes de evaluar ni un candidato, meses de `PASS` sin medir nada—
pero llegando por la puerta de al lado: alli el defecto se escribio dentro del pack, aqui **el pack
no se toca** y se le retira por debajo lo que media.

- **La senal es la misma que la de N-51: un `PASS` de algo que nadie ha visto fallar nunca.** Un pack
  que sigue verde despues de un cambio que altera **justo la forma que ese pack lee** no esta
  confirmando el cambio: esta callando.
- **Y la comprobacion es barata: rompele el firmware a proposito otra vez (CLAUDE.md §8.bis).** Un
  arnes se ve fallar **antes** de conectarlo, si — pero tambien **despues de cada refactor que toque
  la forma de lo que vigila**, porque un instrumento que se vio fallar en enero puede estar ciego en
  marzo sin que nada lo diga.
- **Corolario de presupuesto, que es lo que hace incomodo esto:** el ahorro era real y el margen es
  estrecho. No se rechazo por dudarlo, se rechazo **midiendo las dos cosas**: 636 B contra un pack
  que vigila que ninguna de las 20+ ramas del despachador prometa lo que no comprobo, incluidas las
  tres de N-80. **Ahorrar flash apagando el instrumento que vigila justo eso no es ahorrar: es
  cambiar bytes por ceguera**, y los bytes se recuperan (N-70 saco 5.160 B de un bus que el equipo no
  tiene, sin tocar codigo).

**Las cifras del episodio, MEDIDAS en los dos extremos y no estimadas** (CLAUDE.md §7: *"un delta
exige medir los DOS extremos"*):

| | antes | despues | delta |
|---|---|---|---|
| Maestro | **85,8 %** · 56.260 B | **88,4 %** · 57.904 B | **+1.644 B** · quedan **7.632 B libres** |
| Esclavo | 63,9 % | 64,0 % | las tres lineas del `SET_RTC` verificado |

> ⚠️ **Y una cifra que hay que dejar escrita porque incomoda: se estimo ~930 B y costaron 1.644 —el
> **77 % por encima**—.** Los dos motivos estan medidos y ninguno era imprevisible: **(a)** el alcance
> crecio a mitad de camino, porque el pack encontro dos `OK` mudos que no estaban en el encargo
> (N-80); y **(b)** `app_03` **obliga por diseno** a escribir la trama entera dentro de cada rama, que
> es precisamente lo que este N-89 decide conservar. O sea: **el sobrecoste no es un fallo de la
> estimacion, es el precio del instrumento, y ahora esta contado.** La proxima rama del despachador
> cuesta su literal completo — quien planifique con eso no se llevara la sorpresa dos veces.

**Que quedaria por hacer si el ahorro se quisiera de verdad.** No es imposible: el compositor puede
convivir con el pack si el pack deja de leer literales y pasa a exigir que **la respuesta cuelgue de
una condicion**, no que la cadena este escrita ahi. Pero eso es reescribir el detector, y un detector
reescrito **vuelve a nacer sin haberse visto fallar**. Hoy la decision tomada es la conservadora:
**los 1.644 B se pagan**, y por que se pagan queda escrito aqui para que nadie lo vuelva a proponer
como una mejora obvia.

---

### 🔴 N-88 — La asimetria al abandonar el Modo Degradado: dos criterios para lo mismo

**De donde sale:** de escribir las ramas nuevas de N-78. Al decidir que hacia `SET_MODO:MENU`
estando en Degradado quedo a la vista que las ramas **viejas** no se hacen esa pregunta.

**MEDIDO, en el mismo fichero y a diez lineas unas de otras:**

```
PREGUNTAN por el Degradado (ramas nuevas, d34cfe2). La guarda es
`if (modoActual_get() == MODO_DEGRADADO)` en :195, :215 y :226:

  Maestro/src/bluetooth.cpp:190   SET_MODO:MENU         -> modo_degradado_pedirSalida(), todo-rojo 30 s
  Maestro/src/bluetooth.cpp:211   SET_MODO:ALCANCE      -> $ERR ... EN_MARCHA_PARE_EL_MODO
  Maestro/src/bluetooth.cpp:222   SET_MODO:INTELIGENTE  -> $ERR ... EN_MARCHA_PARE_EL_MODO

NO PREGUNTAN, y salen sin el todo-rojo de 30 s (ramas viejas, sin tocar):

  Maestro/src/bluetooth.cpp:176   SET_MODO:AUTO         -> modoActual_set(MODO_AUTOMATICO)  (:177)
  Maestro/src/bluetooth.cpp:181   SET_MODO:MANUAL       -> modoActual_set(MODO_MANUAL)      (:182)
  Maestro/src/bluetooth.cpp:186   SET_MODO:AMBAR        -> modoActual_set(MODO_AMBAR)       (:187)
  Maestro/src/mando.cpp:116       A.A.A (ACC_AUTOMATICO)-> modoActual_set(MODO_AUTOMATICO)
  Maestro/src/mando.cpp:125       B.B.B (ACC_AMBAR)     -> modoActual_set(MODO_AMBAR)
```

Ninguna de las cinco de abajo consulta `modoActual_get() == MODO_DEGRADADO`. **Se ha comprobado leyendo
las ramas, no el mensaje del commit.**

**Lo que SI hacen las cinco, para no acusarlas de mas de lo que hacen:**

1. **El indicador del Degradado se borra igual.** `main.cpp:198` es el punto de estrangulamiento unico
   de N-20 —`if (modoAnterior == MODO_DEGRADADO) respaldo_guardarDegradado(false);`— y cualquier
   cambio de modo pasa por ahi. **El equipo no reanudara el Degradado tras un corte.** Esa mitad esta
   bien y no es lo que se discute.
2. **Las dos del mando pasan por un todo-rojo.** `confirmarYActuar()` (`mando.cpp:148`) llama a
   `coordinador_forzarRojoTotal()` **antes** de nada, en las dos puntas, mas los destellos de
   confirmacion. No es un salto a verde.

**Lo que NINGUNA de las cinco hace, que es la diferencia entera:** el todo-rojo de
`ROJO_TRANSICION_MS` —**`DEG_DESPEJE_SEG = 30` segundos**, `modo_degradado.cpp:56` y `:114`— con la
pantalla `"Saliendo: todo rojo" / "Vea las dos puntas"` y el estado `DEG_SALIDA_ROJO`, del que solo se
sale al menu cuando esos 30 s se han cumplido (`modo_degradado.cpp:489-494`). **El todo-rojo del mando
dura lo que duran unos destellos; el de la salida del Degradado dura media escena de trafico, y esa
duracion ES la verificacion visual de las dos puntas.**

> **Por que esto importa justo en este modo y no en otro:** el Degradado es **el unico modo del
> firmware que enciende un verde sin confirmacion del otro extremo** —lo dice su propia puerta de
> entrada—, y las dos puntas se coordinan **por reloj**, no por radio. Su escenario peligroso esta
> escrito en el fuente, en `modo_degradado.cpp:466-469`: *"el escenario peligroso de este modo es que
> una sola punta lo abandone"*. Salir por una via que no espera **es exactamente ese escenario**: esta
> unidad se pone a ciclar o a ambar mientras la otra sigue dando verde por reloj.

> 🔴 **Y la regla que lo convierte en un N-x en vez de en un detalle: dos puertas al mismo sitio con
> criterios distintos son UNA sola puerta — la mas floja.** Es lo mismo que el propio `d34cfe2`
> escribio como motivo para extraer `modo_degradado_pedirSalida()` (*"dos formas de abandonar el modo
> serian dos criterios, y en la calle mandaria el mas flojo"*) y para que `SET_MODO:DEGRADADO` reuse
> `modo_degradado_evaluarEntrada()` en vez de tener su propia tabla. La **entrada** quedo con una sola
> puerta; **la salida quedo con dos**, y una de ellas no cuesta nada de usar.

#### Las tres salidas posibles, sin recomendar ninguna

Esto **no es un defecto con arreglo evidente**: es una decision sobre que debe hacer el equipo, y las
tres opciones tienen coste real en la calle. **La eleccion es del responsable, no de quien escribe el
codigo.**

| | que se hace | lo que cuesta |
|---|---|---|
| **A. Que los nuevos tambien dejen salir** | se quitan las guardas de `SET_MODO:ALCANCE` e `INTELIGENTE`: cualquier `SET_MODO` abandona el Degradado como hoy hacen AUTO/MANUAL/AMBAR | el criterio queda unificado **por abajo**: el todo-rojo de 30 s deja de ser obligatorio para nadie, y el modo mas peligroso se abandona con un toque de telefono |
| **B. Que los viejos pasen por el todo-rojo** | `SET_MODO:AUTO/MANUAL/AMBAR` y las secuencias `A.A.A`/`B.B.B` llaman a `modo_degradado_pedirSalida()` estando en Degradado, y el modo pedido se aplica al terminar | unificado **por arriba**, pero **`B.B.B` es la salida de emergencia del mando** (`modo_ambar_fijarMotivo("Ambar pedido desde", "el mando (B.B.B)")`): meterle 30 s de espera a una peticion de emergencia es una decision de seguridad **en la otra direccion**, no una mejora automatica. Y hace falta guardar la accion pendiente durante esos 30 s |
| **C. Declararla deliberada** | se deja como esta y se escribe **por que** cada via tiene el criterio que tiene, en `OPTIMIZACIONES.md` con su regla `SFTY-x` | es una respuesta legitima **solo si el motivo esta escrito**. Una asimetria con motivo es una decision; sin motivo es un olvido que alguien confundira con una decision |

> **Lo que no vale es lo que hay hoy: dos criterios y ningun texto que diga cual es el bueno.** Ese es
> el estado en que un mantenedor futuro copia el criterio flojo creyendo que es el vigente —o el
> estricto, y rompe la salida de emergencia—.

**Que haria falta para cerrarlo.** (1) Que el responsable elija A, B o C. (2) Sea cual sea, **un pack
que lo fije**: hoy no hay ninguno que compare entre si las vias de salida del Degradado, y por eso la
asimetria pudo existir sin que nada la nombrara. El pack tiene que fallar el dia que alguien anada una
sexta via con el criterio contrario al elegido. (3) Si sale B, decidir tambien que pasa con `B.B.B`
—que es lo unico de las cinco que tiene argumento propio para NO esperar—.

---

### 🟠 N-87 — La compuerta no es idempotente despues de un `--rapido`, y nadie lo sabia

**De donde sale:** de correr `--rapido` para ir deprisa y encontrarse la corrida SIGUIENTE —completa,
sobre un arbol limpio, sin haber tocado nada— con el banco en rojo.

**El mecanismo, y no tiene ni un bug dentro.** Son dos hechos ciertos por separado:

1. `compuerta.py` escribe el acta **al final** (`compuerta.py:719`, dentro de `main()`), asi que
   cuando el banco corre **el acta mas reciente que existe es la de la corrida ANTERIOR**.
   `documentos_01_cifras_del_acta` lo dice sin rodeos: `ultima = fw.actas()[0]` (`:157`).
2. Con `--rapido` no se compila (`compuerta.py:653`, `if not rapido:`), asi que esa acta sale **sin
   las tres filas `compila maestro / esclavo / repetidor`**.

La corrida siguiente, **aunque sea completa**, lee esa acta mutilada.

**MEDIDO, reproducido sin tocar el arbol** —se le paso al pack la misma acta real con las tres filas
`compila *` quitadas—: `documentos_01` cae de **45/45 a 40/42**, con dos `FALLA` y ni uno mas:

```
FALLA la tabla del README anuncia 'compila maestro / esclavo / repetidor', que no
      corresponde a ninguna comprobacion del acta 2026-08-28_compuerta.txt
FALLA README publica 15 como total de comprobaciones y el acta trae 12
```

Las tres cifras de flash **no** fallan: el pack ya las manda a `reportar()` cuando el acta no las
trae (`:214-229`), que es el caso previsto. Lo que falla es la **cobertura** —una fila del README sin
comprobacion detras es una fila fantasma (§5 del pack)— y el **recuento del rotulo**. Y las dos
tienen razon: con esa acta delante, el README **esta** anunciando cobertura que la ultima corrida no
midio.

> **La regla que queda: un instrumento que se alimenta de su propia salida anterior no es
> idempotente, y eso hay que escribirlo donde se lee el comando.** No es un defecto del pack — es el
> pack haciendo exactamente su trabajo, que es negarse a que un documento publique cifras que
> ninguna corrida produjo. Lo que faltaba era **saberlo**: tras un `--rapido` hacen falta **dos
> pasadas completas** para volver a verde, y quien no lo sepa se pasa la tarde buscando una
> regresion que no existe. Es la cara benigna del mismo mecanismo que N-46 explotaba en la
> dirección contraria.

**Que haria falta para cerrarlo.** Nada de codigo, y por eso es facil que se quede: una linea en la
cabecera de `compuerta.py` junto a la del `--rapido` y una en `CLAUDE.md §3`. Si algun dia se quiere
codigo, la forma barata es que `--rapido` **marque el acta** como parcial y que `documentos_01` lo
distinga de un acta completa a la que le falta una fila — pero ojo: eso es exactamente lo que el pack
se niega a hacer hoy a proposito, porque un acta que se auto-declara incompleta y aun asi aprueba es
un `ABORTADO` contado como `PASS`.

---

### 🟢 N-86 — `AiBus`: 280 B de RAM vivos que el enlazador no puede descartar · **CERRADO en `26479d9`**

> 🟢 **CERRADO el 28/08 en `26479d9`, y el cierre confirma la prediccion completa.** `nm` sobre los
> dos extremos: **Maestro 3.804 → 3.524 B (−280 B, 18,6 % → 17,2 %)** y **Esclavo 3.640 → 3.360 B
> (−280 B, 17,8 % → 16,4 %)**. **Lo que NO importa es la flash: son 16 B.** El enlazador ya
> descartaba las tres funciones con `--gc-sections`, asi que quitarlas apenas mueve el binario. Lo
> que **no podia** descartar era el objeto: `AiBus` tiene constructor, su llamada vive en
> `.init_array`, y por eso se construia en cada arranque.
>
> **La regla que queda: un camino muerto que no cuesta flash puede seguir costando RAM.** Era el
> 5,2 % de la RAM viva del equipo, y ningun censo de flash lo habria encontrado nunca.
>
> El trinquete de `costura_10_funciones_muertas` **se vio disparar antes de darlo por bueno**
> (§8.bis): con la lista vieja y el firmware nuevo el pack cae a `11/13` pidiendo por su nombre
> huerfanas que ya no existen. Maestro pasa de 18 a 15 huerfanas, Esclavo de 11 a 8.



**De donde sale:** de N-76. Al dejar de abrir `AiBus` en `protocolo_setup()` quedaba la pregunta
obvia —*"si ya no se usa, se lo lleva `--gc-sections` y no cuesta nada"*—. **No se lo lleva.**

**MEDIDO** sobre el `.elf` que produjo el acta del 28/08
(`01_Firmware/Maestro/.pio/build/maestro/firmware.elf`, del 28/08 a las 18:26, posterior a
`50a5380`), con el `nm` del toolchain que vive fuera de la ruta con `ñ` (N-44):

```
arm-none-eabi-nm --size-sort -S -td firmware.elf | grep AiBus
536872380 00000280 b _ZL5AiBus
```

- **`protocolo_actualizarAI()` SI desaparece**: no esta entre los simbolos del `.elf`. Sus tres
  funciones se descartan.
- **El objeto NO.** `_ZL5AiBus` son **280 B en `.bss`**, y siguen ahi porque `HardwareSerial` tiene
  constructor: `arm-none-eabi-nm` encuentra `_GLOBAL__sub_I__Z15protocolo_setupv` y `objdump -h` da
  `.init_array` con `0x24` bytes de entradas. **Una llamada desde `.init_array` es una raiz para el
  enlazador**, y una raiz no se poda por mucho `--gc-sections` que se pase.

**Cuanto es, dicho de las tres formas para que nadie escoja la que le conviene:** 280 B son el
**5,2 %** de los 5.336 B que el binario del Maestro reserva en RAM (`.data` 260 + `.bss` 3.536 +
`._user_heap_stack` 1.540) y el **1,4 %** de los 20 KB que tiene el chip. Para algo **sin un solo
llamador en ninguna de las dos puntas**.

> **La regla que queda: `--gc-sections` poda codigo, no poda objetos con constructor.** «No lo llama
> nadie» y «no ocupa» son afirmaciones distintas y la segunda hay que medirla en el `.elf`. Es N-70
> por el otro lado: alli el enlazador **arrastraba** `Wire` entero por una referencia; aqui **no
> puede soltar** un objeto por su `.init_array`. Las dos veces la respuesta salio del binario, no del
> razonamiento.

**Que haria falta para cerrarlo.** Retirar `AiBus` y `protocolo_actualizarAI()` de las dos puntas en
un commit propio —N-76 lo dejo escrito asi a proposito, `protocolo.cpp:36`—, con la cifra de RAM
antes y despues, y comprobando que ningun documento sigue vendiendo el «puerto de camara IA». Ojo al
retirarlo: `costura_10_funciones_muertas` es un **trinquete**, y una huerfana que desaparece tambien
mueve su lista.

---

### 🟢 N-85 — El `.kicad_pcb` NO esta vacio: es N-64 repitiendose, con el mismo buscador roto

**De donde sale:** de ir a comprobar una cifra del mapeo y tropezar con la frase que sostiene todo lo
demas. `03_Hardware_Tarjeta/MAPEO_TARJETA_KICAD.md` §0 dice, literal:

> *"El `.kicad_pcb` de este proyecto **está vacío**, así que entre el esquemático y la tarjeta que hay
> encima de la mesa no existe ningún artefacto que las ate."*

**Esa frase es falsa. MEDIDO** sobre
`01_Firmware/Controladora_Semaforos/Controladora_Semaforos/Controladora_Semaforos.kicad_pcb`
(2.158.421 B):

| | |
|---|---|
| footprints | **185** |
| segmentos de pista | **1.447** |
| vias | **89** |
| pads | **485** |

Los `.kicad_pcb` de **78 bytes** existen, y son cinco: todos en
`99_Legacy/Controladora_Semaforos-backups/`. **Se midio sobre la copia equivocada y la conclusion se
quedo escrita apuntando al plano bueno.** Es exactamente N-64.

> 🔴 **Y la trampa del buscador, que es la parte que hay que llevarse:** el censo natural
> —`grep -c '(segment ' fichero`— devuelve **0** sobre el plano bueno. No porque no haya pistas, sino
> porque **el fichero indenta con tabuladores** y el patron pide un espacio. Un `0` de un `grep` se
> lee como *"no hay"* y es *"tu patron no casa"*. Contado con `grep -c $'\t(segment'` salen 1.447.
>
> **CLAUDE.md §4 otra vez, y ya van tres: un «no aparece» no es un hallazgo hasta haber descartado al
> buscador.** Aqui el coste fue mayor que una linea equivocada: esa frase es la que justifica que
> **todo** el mapeo se declare *"medido en el esquematico, no en el cobre"*. Con un `.kicad_pcb`
> ruteado encima de la mesa **si hay** un artefacto que ata el dibujo a la placa, y varias filas del
> mapeo se pueden subir de nivel sin tocar un multimetro.

**Estado: identificado, no corregido aqui.** La correccion de `MAPEO_TARJETA_KICAD.md` §0 y §9 la
esta haciendo **otra sesion sobre este mismo arbol**; este roadmap no toca ese fichero (CLAUDE.md
§8.quinquies: dos agentes sobre el mismo arbol se pisan sin avisar).

> ⚠️ **Lo que esto NO cambia, y conviene decirlo antes de que alguien lo celebre:** que el plano este
> ruteado no lo convierte en la tarjeta. Sigue sin haber **ni una fila «VERIFICADO EN LA PLACA»** en
> todo el mapeo. Lo que cambia es que ahora hay **dos** ficheros que comparar entre si, y una
> discrepancia entre `.kicad_sch` y `.kicad_pcb` es un hallazgo gratis que hoy nadie esta buscando.

---

### 🔴 N-84 — Contradiccion de polaridad en `J16`: los dos lados MEDIDOS, y no pueden ser ciertos a la vez

**Este es el bloqueante del cableado de camaras.** N-77 manda las camaras a `J16` p10 (`PB14`) y p12
(`PB15`), los pines que hoy son Boton 3 y Boton 4. Antes de cablear hay que saber en que sentido lee
ese pin, y **los dos lados estan medidos y se contradicen**.

> ⚠️ **El ORDEN de esa maniobra se corrigio el 28/08 y vive en N-92 (y en `CLAUDE.md §9.bis`): no es
> «mismo commit», es «firmware CARGADO en la tarjeta antes de que nadie enchufe nada a `J16`».**
> Resolver N-84 es condicion previa; el orden de ejecucion es lo que N-92 fija.

**Lado firmware — activo en BAJO. MEDIDO:**

```
01_Firmware/Maestro/src/botones.cpp:50-53   pinMode(BOTON1..4, INPUT_PULLUP);
01_Firmware/Maestro/src/botones.cpp:19      bool lecturaCruda = (digitalRead(b.pin) == LOW);
```

**Lado esquematico — activo en ALTO. MEDIDO** trazando la red sobre el `.kicad_sch` bueno, pin por
pin (union-find sobre los 602 `wire` del fichero, no lectura a ojo):

| | valor | un extremo | el otro extremo |
|---|---|---|---|
| `R65` | 10K | red `Boton1` | `GND` (`#PWR035`) |
| `R66` | 10K | red `Boton2` | `GND` (`#PWR036`) |
| `R67` | 10K | red `Boton3` | `GND` (`#PWR037`) |
| `R68` | 10K | red `Boton4` | `GND` (`#PWR038`) |

Son **pull-DOWN**. Y `J16` reparte **3,3 V en p4, p7, p9 y p11**, justo al lado de cada pin de boton
(p5, p8, p10, p12): el pulsador cierra **a 3,3 V**. Eso es activo en ALTO.

**Si gana el esquematico, es N-67 palabra por palabra**, y la cuenta ya esta hecha en el fuente
(`modo_inteligente.cpp:37-41`): pull-up interno de ~40 kOhm contra 10 kOhm externos deja el pin en
`3,3 x 10/50 = 0,66 V`, que es **LOW permanente**. Los cuatro botones se leerian **pulsados desde el
arranque y para siempre**, y al pulsar de verdad el pin subiria a 3,3 V y se leeria **soltado**.
Invertido y siempre activo a la vez.

**La prueba de que el mismo dibujante hizo lo mismo en la entrada de camara, y alli si se corrigio:**
`R64` —el de `PB0`, la camara de demanda— tambien es 10K entre la red `Puerta` y `GND`. **MEDIDO por
el mismo trazado.** Por eso `modo_inteligente.cpp:44` pone `pinMode(CAM_DEMANDA_PIN, INPUT)` a secas.
`botones.cpp` no recibio esa correccion.

> 🟡 **Y esto PUEDE explicar N-26, el «ACEPTAR fantasma». No se afirma.** N-26 se atribuyo a que
> `botones_setup()` declaraba los pines y **nunca los leia**, de modo que el estado arrancaba en
> `false` y la primera vuelta del `loop()` veia un flanco. Esa explicacion es buena y el arreglo
> —sembrar el estado real y `disparadoAnt`, `botones.cpp:96-103`— es correcto por si solo. Pero **un
> divisor que deja los cuatro pines en LOW permanente produce el mismo sintoma**, y ademas explicaria
> por que se vio en banco *"sin que nadie tocara nada"*. **Las dos hipotesis siguen abiertas**, y se
> separan con una sola medida: si la polaridad esta invertida, hoy los cuatro botones estan leidos
> como pulsados en reposo y **ninguno responde a una pulsacion real**.

> **La regla que queda: dos documentos que se contradicen sobre la misma pata no se resuelven
> eligiendo el que suena mas fiable.** Un `pinMode()` es una **hipotesis sobre el cobre**, no un dato
> del cobre. Aqui hay dos hipotesis, las dos escritas por gente que sabia lo que hacia, y la unica
> forma de cerrarla es el multimetro. Hasta entonces **cualquier cosa que se cablee en `J16` es una
> apuesta**.

**Que haria falta para cerrarlo (M3 del anexo de `05_Funcional/17_...md`):** con la tarjeta
alimentada y **sin nada enchufado en `J16`**, medir la tension de `J16` p5, p8, p10 y p12 contra GND.
**~0 V** = el esquematico manda, el firmware esta invertido y hay que quitar `INPUT_PULLUP` de los
cuatro. **~3,3 V** = el pull-down no esta poblado en la placa real y el firmware tiene razon; entonces
lo falso es el esquematico. **~0,66 V** = las dos cosas a la vez, que es el caso de N-67. Es una
medida de dos minutos y **desbloquea el cableado de camaras entero**.

---

### 🟢 N-83 — `FORZAR_ROJO` del Esclavo: el nombre y el efecto no coinciden · **CERRADO en `caef8a1`**

> 🟢 **CERRADO el 28/08 por la opcion (b), y con un matiz que no estaba en las dos que se listaron
> abajo.** El comando pasa a `CMD:AMBAR_EMERGENCIA` y **el comportamiento NO cambia** —ambar
> intermitente con la pluma arriba es decision del cliente y del PMT del 27/08—: lo que cambia es
> que el equipo deje de contestar *«rojo forzado, correcto»* mientras pone ambar y abre. Y
> `FORZAR_ROJO` **se rechaza por sus dos puertas con un motivo que enseña el nombre bueno**: ni
> alias mudo —eso conservaria la mentira— ni `DESCONOCIDO` generico, porque quien lo mande tiene una
> app o un manual viejo y merece enterarse.
>
> **La exencion de PIN se conserva y su razon se REESCRIBE, porque la vieja era falsa.** Decia que
> el PIN guarda lo que abre paso, y este camino **si** abre paso. La buena es otra: un ambar
> intermitente no le da prioridad a NADIE, y una caida segura que exija recordar una clave delante
> de un accidente no es una caida segura.
>
> **Y aparecio una tercera mitad que no estaba en el hallazgo: el ambar pedido por Bluetooth se
> revocaba solo.** El ambar del mando era sagrado y el de la app se lo llevaba el siguiente latido,
> en ~3 s: el operario veia el equipo obedecer y volverse atras sin que nadie se lo dijera. El latch
> entra en los **tres** vetos, y no por simetria —`:401` es la que revocaba de verdad, `:526` es un
> `if` independiente y **no** un `else`, asi que guardar solo uno dejaba la revocacion intacta, y
> `:408` porque si no el ambar duraria hasta el siguiente verde, que es el peor final—.



**De donde sale:** del censo de comandos, al cruzar cada despachador de `bluetooth.cpp` con lo que la
funcion llamada hace de verdad.

**MEDIDO.** El comando entra por dos puertas en el Esclavo —sin PIN
(`Esclavo/src/bluetooth.cpp:109-113`) y con PIN (`:124-127`)— y **las dos hacen lo mismo**:

```
semaforo_iniciarFallo();
enviarTramaConCrc("$ACK,CMD:FORZAR_ROJO,RESULT:OK");
```

`semaforo_iniciarFallo()` (`Esclavo/src/semaforo.cpp:227-231`) deja `S_FALLO`, que es **ambar
intermitente**, no rojo. Y `escribirPines()` (`Esclavo/src/semaforo.cpp:67-68`) escribe:

```
digitalWrite(MOTOR_TALANQUERA, (verde || estado == S_FALLO) ? TALANQUERA_ABRIR : TALANQUERA_CERRAR);
```

es decir, **la talanquera queda ARRIBA**.

> **La pluma arriba en `S_FALLO` NO es el defecto, y decirlo importa.** Es una **decision de
> operacion del cliente y del PMT, tomada el 27/08/2026** y razonada en el propio fuente
> (`semaforo.cpp:53-62`): el ambar intermitente significa *paso con precaucion*, y una pluma abajo en
> un corredor de obra sin salida es su propio peligro. Esta escrita, esta trazada a SFTY-28 y el
> arnes del automatico la conoce por nombre. **Quien vaya a cerrar este N-x no debe tocarla.**

**El defecto es el rotulo.** Un operario que pulsa *ROJO TOTAL* en la app sobre el Esclavo recibe
`RESULT:OK` y obtiene **ambar intermitente con la barrera abierta**: lo contrario de lo que el boton
promete. Y hay dos agravantes:

1. **Se revoca solo.** El siguiente `CMD_GO_RED` del Maestro lo deshace
   (`Esclavo/src/main.cpp:526`). No es una orden que se mantenga: es un parpadeo que dura hasta el
   proximo latido de radio.
2. **Es el UNICO comando exento de PIN**, y la exencion esta razonada asi (`bluetooth.cpp:100-108`):
   *"El PIN guarda lo que ABRE paso o mueve luces; no lo que las para"*. **Este camino no para el
   trafico: abre la barrera.** El criterio es bueno; lo que no cumple es este comando.

> **La regla que queda: una exencion de seguridad se concede a un EFECTO, no a un nombre de comando.**
> `FORZAR_ROJO` entro por la puerta sin PIN porque *"detener el trafico es la accion segura"* — y
> nadie volvio a comprobar que siguiera deteniendolo. El dia que el efecto cambio, la exencion se
> quedo donde estaba, porque estaba atada a la cadena `"CMD:FORZAR_ROJO"` y no a lo que hace.

**Que haria falta para cerrarlo.** Decidir cual de las dos: (a) que el Esclavo haga rojo de verdad
—`semaforo_forzarRojo()`— y entonces la exencion de PIN es correcta y el nombre tambien; o (b)
renombrarlo a lo que hace y **sacarlo de la exencion**. Y en los dos casos, un pack que ate la lista
de comandos sin PIN al efecto medido sobre los pines, no al nombre.

---

### 🟢 N-82 — `TEST_LEDS` escribe pines por fuera de SFTY-2, y ademas sube la pluma · **CERRADO en `caef8a1`**

> 🟢 **CERRADO el 28/08.** Las tres fases del test salen ahora por `aplicarSalidas()`, y la
> distincion va **DENTRO** de `escribirPines()` —`(verde && !testLedsActivo) || estado == S_FALLO`,
> `Maestro/src/semaforo.cpp:94`—, **no en un segundo `digitalWrite`**: la regla §6 dice que todo
> sale por esa puerta, y **una barrera con dos puertas no es una barrera**. Verde encendido con
> pluma abajo es la direccion admitida; al reves seria una invitacion a entrar que nadie autorizo.
>
> 🔴 **Y al arreglarlo aparecio un FALLO LATENTE que el codigo viejo tenia abierto, y que nadie
> habia pedido buscar.** Una senal del mando iniciada **a mitad de test** dejaba `actualizarSenal()`
> sin llamar por el `return` del bloque del test: la senal no terminaba nunca, **`senalActiva` se
> quedaba en `true` PARA SIEMPRE**, y `aplicarSalidas()` —que con `senalActiva` guarda y no
> escribe— **no volvia a tocar un pin en toda la vida del equipo**. Los pines se congelaban en el
> ultimo fotograma del test, **que podia ser el verde**. Ahora el test **espera** a que la senal
> suelte las luces y se rearma entero, en vez de correr por debajo o abandonar en silencio.
>
> 🔴 **N-82 NO SE PODIA CERRAR EN UNA SOLA PUNTA, y eso lo impuso el instrumento, no el criterio de
> nadie.** `barrera_02_dos_puntas` exige `escribirPines()` identica y el mismo multiconjunto de
> llamadas en los dos `semaforo.cpp`. El cambio va **espejado en el Esclavo, donde es inerte** —alli
> `TEST_LEDS` se rechaza y `testLedsActivo` nunca vale `true`—, porque la propiedad que se protege
> no es *«el test del Maestro esta bien»*, es que **las dos puntas escriban los pines IGUAL**.
>
> ✅ **Y `app_03_sin_ok_mudo` cazo el PRIMER INTENTO del arreglo, tres horas despues de nacer.** El
> primer intento rechazaba dentro de `semaforo_iniciarTestLeds()`, que es `void`: el `$ACK` habria
> salido igual y el rechazo habria sido **mudo**. El pack de la Fase 1 haciendo exactamente su
> trabajo, y sobre codigo escrito el mismo dia que el.
>
> **Instrumentos nuevos, los dos VISTOS CAER con el defecto inyectado en el `.cpp` real (§8.bis):**
> `maestro_09_test_leds` **18/18**, cae a `13/18`; `esclavo_07_ambar_emergencia` **16/16**, cae a
> `14/16`. La restauracion se verifico **por SHA-256**, no por la impresion de haberla hecho.



**De donde sale:** del censo de escrituras de pin. CLAUDE.md §6 dice que **solo `semaforo.cpp`
escribe pines de luz y todo pasa por `escribirPines()`**. Es cierto. Lo que no dice —y hay que
leerlo en el codigo— es que dentro de `semaforo.cpp` hay **dos** caminos hasta ahi, y solo uno pasa
por el enclavamiento.

**MEDIDO.** `aplicarSalidas()` (`Maestro/src/semaforo.cpp:71-98`) es quien lleva SFTY-2 dentro. El
test de lamparas no la usa:

```
Maestro/src/semaforo.cpp:237-250   if (testLedsActivo) {
                                     ...
                                     escribirPines(true, false, false);    // :240
                                     escribirPines(false, true, false);    // :242
                                     escribirPines(false, false, true);    // :244  <-- VERDE crudo, 2 s
```

Tres llamadas **directas a `escribirPines()`**, saltandose `aplicarSalidas()`. Consecuencias medidas:

- **El verde de `:244` no pasa por ningun enclavamiento.** Hoy no produce un rojo+verde simultaneo
  porque el bloque hace `return` y nadie mas escribe en esos 6 s — pero eso es una propiedad del
  orden de las lineas, no una barrera. SFTY-2 vive en `aplicarSalidas()` y aqui no se llama.
- **`ultR/ultA/ultV` no se actualizan** durante el test, porque quien los guarda es
  `aplicarSalidas()` (`:90`). Una senal del mando que terminara a mitad de test volcaria la foto
  vieja.
- 🔴 **Y la talanquera SUBE.** SFTY-28 cuelga de `escribirPines()` (`:67-68`) precisamente para que
  nadie pueda mover la barrera por su cuenta — y eso hace que el test de lamparas, que no sabe nada
  de barreras, **abra la pluma durante 2 s** al llegar al verde.

**El contraste esta escrito en el propio repositorio, y es lo que hace este hallazgo dificil de
excusar.** El Esclavo **rechaza** `TEST_LEDS` con su argumento delante
(`Esclavo/src/bluetooth.cpp:146-158`): *"ese verde sale mientras el Maestro esta dando paso al otro
sentido: dos vehiculos entrando de frente al tramo"*. El Maestro **lo ejecuta** sin condiciones
(`Maestro/src/bluetooth.cpp:145-148`), y el Maestro es el que tiene la talanquera.

> **La regla que queda: «todo pasa por una funcion» solo es una barrera si la barrera esta EN esa
> funcion.** Aqui la barrera esta un nivel por encima —en `aplicarSalidas()`— y basta llamar al nivel
> de abajo para rodearla sin salirse del fichero. Un censo de `grep escribirPines` da 12 llamadas y
> parece sano; el censo que hace falta es **cuantas de ellas pasan antes por el enclavamiento**.

**Que haria falta para cerrarlo.** Que el test entre por `aplicarSalidas()` como todo lo demas (o que
el enclavamiento y la barrera bajen a `escribirPines()`), y **un pack que lo mida sobre los pines**,
no sobre la lectura: `Validacion_Automatico` ya compila `semaforo.cpp` de verdad y mira lo que se
escribio. Y antes de conectarlo, verlo caer con el defecto inyectado (§8.bis).

---

### 🟠 N-81 — Telemetria fabricada: cuatro campos de `$STATUS` son texto, no medidas

**De donde sale:** de N-77. Si toda la operacion pasa a la app, la trama `$STATUS` deja de ser un
adorno y pasa a ser **el unico instrumento del tecnico**. Se fue a mirar que campos son datos.

**MEDIDO, campo por campo:**

| campo | Maestro | Esclavo |
|---|---|---|
| `MODO:` | real (`obtenerNombreModo()`) | **literal `SUBORDINADO`** — `Esclavo/src/bluetooth.cpp:215` |
| `ESTADO:` | real | real |
| `RF:` | real (`coordinador_calidadEnlace()`, `Maestro/src/bluetooth.cpp:226`) | **literal `98%`** — `:215` |
| `RTT:` | real (`coordinador_tiempoRespuestaMs()`, `:228`) | **literal `85ms`** — `:215` |
| `BAT:` | **literal `12.6`** — `Maestro/src/bluetooth.cpp:245` | **literal `12.6`** — `:215` |
| `T:` | `(millis()/1000) % 60` — `:238` | `(millis()/1000) % 60` — `:208` |
| `HORA:` | real, y **dice `--:--:--` cuando no la tiene** | idem |

- **No hay un solo `analogRead` en el firmware.** `grep -rn analogRead Maestro/src Esclavo/src` da
  **cero**. `BAT:12.6` no puede ser otra cosa que un literal: no hay por donde entrar una tension.
- **`T:` no es el tiempo de fase.** Es un contador de segundos que da la vuelta cada minuto, corriendo
  desde el arranque y **sin relacion ninguna con el cambio de fase**. Y el comentario que tiene encima
  dice lo contrario: `Maestro/src/bluetooth.cpp:237`, *"Cuenta de segundos transcurridos en fase
  actual (T:)"*. **El comentario es el instrumento que miente aqui**, y es peor que el campo.
- 🔴 **Lo mas caro no es lo que se inventa, es lo que se tira.** El Esclavo **si tiene contadores de
  linea reales** —`protocolo_tramasValidas()` y `protocolo_tramasDescartadas()`,
  `Esclavo/src/protocolo.cpp:183-184`—. Su **unico** consumidor es `Esclavo/src/menu.cpp:87`, o sea
  **la pantalla que N-77 retira**. Se esta a punto de tirar el unico dato de calidad de enlace que
  esa punta mide, mientras la trama publica un `98%` inventado en su lugar.

> **La regla que queda es la de la app, aplicada al firmware (CLAUDE.md §3.quinquies): lo que
> sustituye a un dato que no se tiene no es un valor plausible — es decirlo.** Un `RF:98%` fijo es
> peor que un campo ausente y muchisimo peor que un `RF:--`: el campo ausente se nota, el `--` se
> entiende, y el `98%` **se cree**. Y se cree justo el dia de lluvia en que el tecnico mira la app
> para decidir si el enlace aguanta. `HORA:` ya lo hace bien —escribe `--:--:--` cuando no la
> tiene—; es el modelo a copiar y esta en el mismo `snprintf`.

**Que haria falta para cerrarlo.** (1) `RF:` y `RTT:` del Esclavo: o se cablean a
`protocolo_tramasValidas/Descartadas` —que ya existen y quedan sin consumidor— o se emiten como `--`.
(2) `BAT:` a `--` en las dos puntas mientras no haya divisor y `analogRead`; si se quiere de verdad,
es hardware nuevo y va a la lista de compras, no al `snprintf`. (3) `T:` o mide la fase o se llama
`UPTIME`; y el comentario de `:237` cae con ella. (4) Un pack que exija que **todo campo de `$STATUS`
tenga un origen en el firmware**, con `control_negativo` que lo vea fallar al fijar uno a mano — es la
unica forma de que esto no vuelva.

---

### 🟢 N-80 — `SET_RTC` rechaza en silencio y contesta `RESULT:OK` · **CERRADO en `d34cfe2`**

> **Y crecio al medirlo: no era una rama, eran TRES — y las dos de mas las encontro el PACK, no una
> revision humana.** El encargo decia *"`SET_RTC` contesta OK a ciegas"*. Al escribir
> `app_03_sin_ok_mudo`, que **deriva del C++ la lista de funciones a vigilar en vez de teclearla**,
> aparecio que `MANUAL:CAMBIAR_TURNO` hacia lo mismo: llamaba a `coordinador_pedirCambio()`, que
> abandona en su `if (estadoC != C_IDLE) return;`, y contestaba `RESULT:OK` igual.
>
> **Esto es exactamente lo que CLAUDE.md §3.ter dice que tiene que pasar.** *"Si los defectos
> aparecen porque alguien pregunta, no hay metodo — hay suerte."* El 26/08 los ocho defectos de V9.0
> salieron todos de que el responsable preguntara. Aqui **dos de tres salieron del instrumento**, en
> la misma tarde, sin que nadie sospechara de esa rama: el pack censo las 20+ ramas del despachador
> en las dos puntas y las midio todas con el mismo criterio. La diferencia entre las dos formas de
> trabajar no es de esfuerzo, es que **una escala y la otra depende de que a alguien se le ocurra**.

**De donde sale:** de la arquitectura de N-77. El plan del ESP32 da por hecho que el reloj se
resuelve **poniendo un DS3231 en el modulo de expansion** y empujando la hora por `SET_RTC`. Se fue a
mirar si esa puerta funciona.

**MEDIDO, y son dos piezas que por separado estan bien razonadas:**

```
Maestro/src/reloj.cpp:290    if (!rtcOperativo) return;      // N-24, dentro de reloj_ajustar()
Maestro/src/bluetooth.cpp:175    enviarTramaConCrc("$ACK,CMD:SET_RTC,RESULT:OK");
```

El `return` de `reloj.cpp:290` es **correcto y su comentario lo explica bien** (`:281-289`): sin
oscilador, escribir la hora dejaria un contador que nadie hace avanzar, `horaValida` quedaria en
`true`, y **sobre esa mentira el Maestro empujaria la hora al Esclavo y autorizaria el Modo
Degradado**. Es la direccion segura.

El `$ACK ... RESULT:OK` de `bluetooth.cpp:175` es **incondicional**: se emite despues de llamar a
`reloj_ajustar()` sin mirar si sirvio de algo. `reloj_ajustar()` devuelve `void`, asi que el
despachador **no tiene forma de saberlo** — aunque `reloj_hayCristal()` (`reloj.cpp:219`) esta ahi
mismo para preguntarlo.

**Por que esto no es teorico:** **N-17 confirmo EN HARDWARE el 01/08/2026 que el cristal `Y2` no
oscila en las tarjetas actuales**, y N-37 lo cerro por eliminacion con tres medidas. O sea que hoy, en
las tarjetas que hay, `rtcOperativo` es **false** y **este camino esta activo**: el tecnico manda la
hora desde la app, recibe `OK`, y el equipo sigue sin hora.

> **La regla que queda: un acuse de recibo que no depende del resultado no es un acuse — es un eco.**
> Y es la version de protocolo de *«un `FALLA` contado como `PASS`»* (N-46): la parte que decide bien
> y la parte que informa estan en ficheros distintos y **nadie las ato**. El rechazo silencioso es la
> decision correcta; lo que falta es **contarlo**, y el sitio donde contarlo es el `$ERR` que ya
> existe tres lineas mas abajo.

> 🔴 **Y por eso este es el N-x que mas afecta al plan del ESP32.** El DS3231 se compra para
> **evitar** depender de `Y2`... y la unica via por la que su hora entra al STM32 es
> `reloj_ajustar()`, que **se cierra precisamente cuando `Y2` no oscila**. Tal y como esta hoy, poner
> el DS3231 y empujarle la hora por Bluetooth **no arregla nada y encima contesta que si**. Esta era
> la pieza que el plan daba por gratis. Ver `05_Funcional/17_...md` §3.2: o se repara `Y2`, o el
> STM32 necesita reloj de software alimentado por el ESP32 — y las dos salidas pasan por este
> defecto.

#### Como se cerro, que es la parte reutilizable

**Sin tocar ni `reloj.cpp` ni `coordinador.cpp`.** Las dos negativas eran correctas y estaban
razonadas donde deben; lo que faltaba era **contarlas**. El arreglo entero vive en los dos
`bluetooth.cpp`, y son tres ramas:

| rama | lo que faltaba mirar | como se pregunta ahora |
|---|---|---|
| **Maestro `MANUAL:CAMBIAR_TURNO`** | `coordinador_pedirCambio()` abandona en `if (estadoC != C_IDLE) return;` | `pedirCambioVerificado()` pregunta `coordinador_listoParaContar()`, que es literalmente `return estadoC == C_IDLE` |
| **Maestro `SET_RTC`** | DOS retornos tirados: `reloj_ajustar()` (que rechaza en silencio) y `coordinador_sincronizarHora()` (que devuelve `bool`) | `reloj_hayCristal()` antes, `ajustarRelojVerificado()` que **relee** la hora despues, y el `bool` de la sincronizacion |
| **Esclavo `SET_RTC`** | identico | `reloj_contadorSegundos()` |

> **La regla del arreglo, y es la que salva estos cierres de convertirse en el defecto siguiente: se
> pregunta LA MISMA condicion que mira la guarda, no una parecida.** `coordinador_listoParaContar()`
> no es *una aproximacion* a `if (estadoC != C_IDLE) return;` — **es esa comparacion, leida por la
> unica puerta publica que hay**. Una condicion *parecida* seria una segunda copia que alguien
> tendria que sincronizar, y el dia que difieran el `$ERR` mentiria en la otra direccion.

> 🔴 **Y en el Esclavo se aplico la regla del instrumento antes de copiar el arreglo.**
> `reloj_hayCristal()` **no existe en esa punta** —lo declara solo el `reloj.h` del Maestro,
> **comprobado con `grep` sobre `01_Firmware/Esclavo/` entero**, no supuesto por simetria—. La
> pregunta equivalente alli es `reloj_contadorSegundos()`, que **reserva el `0` para "no hay reloj"**
> y devuelve `1` antes que un cero real, de modo que su cero significa exactamente *"el RTC no esta
> operativo"*. Eso es un **proxy exacto**, no una aproximacion — y la diferencia entre las dos cosas
> es justo lo que este N-x castiga. Copiar `reloj_hayCristal()` a ciegas habria dado un ABORTADO de
> compilacion; copiar *"algo que se le parezca"* habria dado un `$ERR` que miente.

**Lo que queda vigilandolo:** `app_03_sin_ok_mudo`, con su calibracion contra `SET_TIEMPOS` —la rama
que ya estaba bien hecha, para que el detector sepa distinguir en vez de solo acusar— y sus dos
controles negativos. **Nacio en rojo y no se dio por bueno hasta que el firmware lo apago**
(CLAUDE.md §8.bis). Y lo que ese pack cuesta mantener vivo esta contado en **N-89**.

---

### 🔴 N-79 — Retirar el mando de reles no deja tres `if` inertes: **BORRA UN VETO**

**De donde sale:** de la lista de lo que N-77 retira. El mando de 4 reles parecia la baja mas barata
—**nunca se compro el receptor**, asi que no hay equipo que desmontar (`05_Funcional/17_...md` §2.7)—
y por eso casi se retira sin mirar.

**MEDIDO. La bandera se arma en UN solo sitio y se consume en TRES:**

```
arma:      01_Firmware/Esclavo/src/mando.cpp:132      ambarLocal = true;   (case ACC_AMBAR)
consume:   01_Firmware/Esclavo/src/main.cpp:401       if (!mando_ambarLocal()) { ... CMD_GO_RED
           01_Firmware/Esclavo/src/main.cpp:408       if (!mando_ambarLocal()) { ... CMD_GO_GREEN
           01_Firmware/Esclavo/src/main.cpp:526       if (!mando_ambarLocal() && ... recuperacion
```

Los tres estan comentados como **SFTY-21**: con el ambar pedido a mano desde el mando (`B.B.B`), el
Esclavo **no obedece ni acusa recibo** a las ordenes de radio. Callando, el Maestro agota sus
reintentos y el cruce entero termina en ambar — que es lo que el operario pidio.

**Quita el armador y esos tres `if` no quedan inertes: quedan SIEMPRE VERDADEROS.** `mando_ambarLocal()`
solo puede devolver `false`, asi que las tres condiciones se cumplen siempre y **el veto desaparece**.
Un `CMD_GO_GREEN` que hoy se ignora pasaria a **encender el verde**.

> **La regla que queda: retirar codigo no es neutro cuando otros dependen de que una bandera pueda ser
> CIERTA.** El razonamiento *"si nadie la enciende, los `if` que la leen no hacen nada"* es exacto y
> es exactamente al reves de lo que importa: una bandera que nunca se enciende convierte cada
> `if (!bandera)` en **codigo sin guarda**. Es la version borrada de N-73: alli una funcion perdio su
> llamador; aqui una **barrera** perderia su armador, y la barrera se abre en silencio.
>
> **Como se censa, que es la parte reutilizable:** para cada simbolo que se va a retirar, `grep` de
> **quien lo escribe** y `grep` de **quien lo lee**, por separado. Si la lista de lectores no esta
> vacia, retirarlo **cambia el comportamiento** — y hay que decir en que sentido, porque no siempre es
> el conservador.

**Que haria falta para cerrarlo.** Decidir que sustituye al veto **antes** de tocar `mando.cpp`: si el
`B.B.B` desaparece, SFTY-21 necesita otro armador (un `SET_MODO:AMBAR` que el Esclavo entienda — ver
N-78) o los tres `if` se retiran a la vez con su regla, anotandolo en `OPTIMIZACIONES.md`. Lo que no
vale es quitar el armador y dejar los lectores.

---

### 🟡 N-78 — `botonCancelar()` es la unica salida de TODOS los modos · **MITAD CERRADA en `d34cfe2`**

> **Las dos mitades de este N-x son distintas y no se cierran juntas. Va escrito asi a proposito,
> sin redondear:**
>
> | mitad | estado |
> |---|---|
> | **Que exista otra salida** — que MENU, ALCANCE, INTELIGENTE y DEGRADADO se puedan pedir y abandonar desde el telefono | 🟢 **CERRADA.** Seis comandos nuevos en el Maestro |
> | **Que `PB15` deje de ser la unica salida EN LA PRACTICA** — o sea, que los cuatro pulsadores se retiren y `J16` quede libre para las camaras | 🔴 **ABIERTA.** Es la Fase 2, y no ha empezado |
>
> **Medido hoy sobre el fuente:** `botonCancelar()` sigue con **12 apariciones** en el Maestro y sigue
> siendo la salida de los ocho modos (`menu.cpp:151`, `modo_alcance.cpp:50`, `modo_ambar.cpp:42`,
> `modo_automatico.cpp:80`, `modo_degradado.cpp:464`, `modo_hora.cpp:262`, `modo_inteligente.cpp:66`,
> `modo_manual.cpp:21`). **No se ha borrado ni una.** Lo que cambia es que **ya no es la unica**:
> antes de `d34cfe2`, retirar los pulsadores dejaba tres modos sin ninguna forma de alcanzarse; hoy
> ya no. Ese era el bloqueo, y ese es el que se levanto — **el trabajo, no**.

**De donde sale:** de N-77, que retira los cuatro pulsadores para dejar `J16` libre a las camaras.
Antes de retirarlos se censo que cuelga de ellos.

**MEDIDO. `botonCancelar()` (`PB15`, Boton 4) es la salida de ocho sitios del Maestro:**

```
menu.cpp:151   modo_alcance.cpp:50   modo_ambar.cpp:42       modo_automatico.cpp:80
modo_degradado.cpp:443/449           modo_hora.cpp:262       modo_inteligente.cpp:65
modo_manual.cpp:21
```

**Y la vuelta al menu no tiene puerta por radio. MEDIDO:** `coordinador_forzarMenu()` tiene **tres**
llamadores —`menu.cpp:82`, `modo_alcance.cpp:40`, `modo_hora.cpp:104`— y **ninguno esta en
`bluetooth.cpp`**. No es una funcion huerfana (N-73): es peor de diagnosticar, porque **tiene
llamadores y aun asi es inalcanzable desde la unica interfaz que quedaria**. Un censo de huerfanas no
la ve.

**Consecuencia directa:** el despachador de `Maestro/src/bluetooth.cpp:123-136` conoce
`SET_MODO:AUTO`, `SET_MODO:MANUAL` y `SET_MODO:AMBAR`. **No existe `SET_MODO:MENU`.** Retirar los
pulsadores antes de anadirlo deja **`MENU`, `ALCANCE` e `INTELIGENTE` sin ninguna forma de
alcanzarse** — y `MENU` es la puerta del Modo Degradado.

#### La correccion, que se escribe y no se borra (CLAUDE.md §4)

> 🔴 **Se afirmo al principio de la pasada que «del Modo Degradado no se saldria». ES FALSO, y la
> refutacion tambien va medida.**
>
> `SET_MODO:AUTO`, `SET_MODO:MANUAL` y `SET_MODO:AMBAR` **si sacan del Degradado**, y ademas lo hacen
> bien: `Maestro/src/main.cpp:197-199` borra el indicador de la pila en un **punto de
> estrangulamiento unico** —`if (modoAnterior == MODO_DEGRADADO) respaldo_guardarDegradado(false);`—
> y su comentario N-20 explica por que esta ahi y no en cada destino: *"del Degradado se sale por al
> menos cuatro caminos y basta olvidar uno para que el equipo reanude despues un modo del que ya
> habia salido"*. Un `SET_MODO` por Bluetooth pasa por ese punto como cualquier otra via.
>
> **Una causa que se cae se marca refutada, no se borra**: la que desaparece en silencio vuelve a
> proponerse, y la segunda vez ya nadie recuerda que se comprobo.

**Lo que si es cierto, y es mas grave que lo que se afirmo mal:**

1. 🔴 **El Esclavo no acepta NINGUN `SET_MODO`.** Su despachador
   (`Esclavo/src/bluetooth.cpp:109-170`) conoce `FORZAR_ROJO`, `SOLICITAR_PASO`, `TEST_LEDS`
   (rechazado) y `SET_RTC`, y cierra con `$ERR,CMD:DESCONOCIDO`. **Sacar al Maestro del Degradado por
   Bluetooth deja al Esclavo dentro**, dando verdes por reloj, que es exactamente *"el escenario
   peligroso de este modo: que una sola punta lo abandone"* — dicho en `modo_degradado.cpp:445-448`.
2. 🔴 **Y se salta el todo-rojo de despedida.** La salida por boton pasa por
   `modo_degradado.cpp:449-463`: borra el indicador **al pulsar**, fuerza todo-rojo, entra en
   `DEG_SALIDA_ROJO` y pide verificacion visual de las dos puntas antes de devolver el mando. Un
   `SET_MODO` por radio **no pasa por ahi**: cambia de modo y el `switch` de `main.cpp:201-210` llama
   directamente al `setup()` del destino. El indicador si se borra —lo salva N-20—, pero **la
   maniobra de despeje no ocurre**.

> **La regla que queda: contar las salidas de un modo por sus SALIDAS, no por su boton.** El censo
> util no fue `grep botonCancelar` —que da ocho y tranquiliza— sino preguntar, por cada camino de
> salida, **por donde pasa**. Dos vias que llevan al mismo estado final pueden diferir en toda la
> maniobra que hacen por el camino, y en un semaforo la maniobra **es** la seguridad.

#### Lo que se hizo en `d34cfe2`, punto por punto contra los cuatro que se pidieron

| | pedido | estado, **verificado leyendo el fuente** |
|---|---|---|
| **(1)** | `SET_MODO:MENU` colgado de `coordinador_forzarMenu()` | 🟢 **hecho, aunque no como se escribio.** La rama llama a `menu_setup()`, y es `menu_setup()` (`menu.cpp:82`) quien llama a `coordinador_forzarMenu()`. Cuelga de el **por dentro**, que es mejor que duplicar la llamada: una sola forma de volver al menu |
| **(2)** | Que el Esclavo acepte `SET_MODO`, o que el Maestro le propague el cambio | 🟡 **por propagacion, y solo por ahi.** El despachador del Esclavo sigue conociendo **cuatro** ordenes —`FORZAR_ROJO` (:124), `SOLICITAR_PASO` (:128), `TEST_LEDS` rechazado (:146), `SET_RTC` (:159)— y cerrando con `$ERR,CMD:DESCONOCIDO` (:204). **No acepta ningun `SET_MODO`, y eso no ha cambiado.** Lo que cierra el escenario es que la salida del Degradado termina en `menu_setup()` → `coordinador_forzarMenu()`, que *"fuerza Rojo Fijo en Maestro y Esclavo"*. SFTY-27 se respeta: el Esclavo no decide, obedece |
| **(3)** | Que la salida del Degradado por radio entre por la **misma** maniobra que la del boton | 🟢 **hecho, y de la unica forma que no crea un segundo criterio: extrayendo el bloque.** `modo_degradado_pedirSalida()` es el cuerpo de `:449-463` **movido literal** —el diff lo ensena: mismas lineas, mismos comentarios— con su guarda dentro y devuelto como `bool`. El `if` del boton pasa a `if (salir && modo_degradado_pedirSalida())`. **No hay dos salidas: hay una funcion con dos llamadores.** El todo-rojo de 30 s se conserva entero |
| **(4)** | Un pack que exija que todo modo alcanzable tenga salida por la interfaz que exista | 🟢 **`app_02_modos_simetricos`.** Cruza los `case X: return "Y";` de `obtenerNombreModo()` contra los `strcmp(accion, "SET_MODO:Y")` del despachador, **en los dos sentidos y leyendo el C++ en cada corrida**. `HORA` queda excluido **por nombre y con su motivo escrito**, no por un filtro generico. Si alguno de los dos censos sale vacio, **ABORTA** en vez de aprobar contra un conjunto vacio |

> **Por que el pack (4) importa mas que los seis comandos:** el desajuste vivia **entero dentro del
> mismo `.cpp`** —la telemetria y el despachador a ochenta lineas uno del otro, dos listas escritas a
> mano que nadie cruzaba—. `app_01_comandos` vigila la costura **app ↔ firmware** y no veia nada de
> esto. El proximo modo que alguien anada nacera otra vez legible y no ordenable, porque anadir el
> `case` del `switch` es lo primero que se hace y anadir la rama del despachador es lo que se olvida.
> **Los seis comandos arreglan hoy; el pack arregla el proximo.**

**Lo que queda ABIERTO de N-78, y no se cierra aqui.**

1. 🔴 **La retirada de los cuatro pulsadores (`PB9`, `PB13`, `PB14`, `PB15`) sigue sin hacerse.** Es
   lo que libera `J16` para las camaras, es lo que N-77 pide (§2.3), y es **la Fase 2**. Hasta que
   ocurra, `PB15` sigue siendo en la practica por donde sale todo el mundo — solo que ya no es la
   unica puerta que existe.
2. 🔴 **Y esa Fase 2 arrastra N-84 delante**, que sigue en rojo: mientras la polaridad de `J16` este
   contradicha, el cableado de camaras no puede empezar aunque el pin quede libre.
3. 🔴 **Nada de esto ha visto una tarjeta.** Los seis comandos, los dos packs y la interfaz de la app
   se midieron sobre ficheros y sobre el `.elf`. **La sesion de banco sigue pendiente**, y un `$ACK`
   que en el PC sale por el sitio correcto no demuestra que el telefono lo lea en el poste.

---

### 🟢 N-77 — La arquitectura del 28/08: el STM32 sigue mandando, el ESP32 es un accesorio

**De donde sale:** de una decision tomada en obra, no de un hallazgo. Se documenta aqui porque es la
que da sentido a los diez N-x de arriba.

**El reparto, en una frase:** **el STM32F103 sigue siendo el controlador del semaforo; el ESP32 pasa a
modulo de expansion colgado de un puerto serie** —aporta reloj (`DS3231`) y Bluetooth— **y no manda
sobre las luces**.

| | |
|---|---|
| **se queda en el STM32** | las 8 luces, la talanquera (`PB2`/`J15`), el buzzer, la radio LoRa (`USART3`/`J12`), las camaras |
| **se lleva el ESP32** | reloj `DS3231` (`GPIO21`/`GPIO22`, pila propia) y el Bluetooth; a futuro WiFi/GPS |
| **se retira** | pantalla LCD (las dos puntas), los cuatro pulsadores (`PB9`, `PB13`, `PB14`, `PB15`) y el mando de 4 reles |
| **camaras** | a `J16`, en los pines que dejan los pulsadores: p12 (`PB15`) Camara 1, p10 (`PB14`) Camara 2 |
| **enlace** | `J17` p2 (`PB7`, RX del micro) y p3 (`PB6`, TX), **9600 8N1**, masa comun obligatoria |
| **alimentacion** | el ESP32 lleva **fuente propia desde 12 V**: no cuelga de los 3,3 V de `J17` |

**Dos decisiones que llevan su porque escrito, para que no se reabran cada sesion:**

- **El ESP32 no comparte riel de 3,3 V** porque ese riel (`U5` LM1117DT-3.3) es el mismo que alimenta
  al STM32 que gobierna el semaforo, y un modulo con radio da picos del orden de 500 mA. *"El accesorio
  no puede tumbar al que manda."* La cifra de 500 mA es **de datasheet, SIN VERIFICAR sobre el modulo
  que llegue** — y no hace falta medirla para decidir: la decision no se cae si el pico resulta ser 300.
- 🔴 **`J16` p1 lleva 12 V crudos**, y es el unico conector de senal de la tarjeta que los trae.
  Entre esa posicion y los pines de camara **no hay opto, ni resistencia serie, ni clamp**. Se tapa
  fisicamente antes de cablear nada, y p5/p8 se dejan vacios como colchon (10,2 mm de p1 a p5;
  27,9 mm a p12, con el paso del footprint de 16 pads).

**Documento completo, con los ocho hallazgos, las cinco medidas de multimetro pendientes y la lista de
documentos que quedan falsos y en que orden tocarlos:**
`05_Funcional/17_Arquitectura_28-08_y_Decisiones_Abiertas.md` (915 lineas, ASCII sin acentos).

> ⚠️ **Lo que esta decision cuesta y hay que tener delante:** el protocolo de 80 pruebas pierde **49**
> —secciones enteras que ejercen pantalla, pulsadores y mando—, y `3_Protocolo_Pruebas_Rigurosas.md`
> es **el acta que se firma**. Y queda una pregunta sin dueno, anotada en §3.3 del documento: **sin
> pantalla, sin pulsadores y sin mando, como se opera el equipo el dia que el ESP32 se cuelga.** Hoy
> no tiene respuesta.

---

### 🟢 N-76 — El Bluetooth pasa a `J17`, `USART1` remapeado a `PB6`/`PB7` · CERRADO

**De donde sale:** de intentar enchufar el modulo SPP en la tarjeta real. `PA9`/`PA10` son
electricamente validos y **no salen a ninguna bornera**: usarlos obliga a soldar en las patas del
`MAX3485 U2` o del propio micro. Medido sobre el esquematico conector por conector, **los pines de la
pantalla son los unicos GPIO que quedan accesibles sin soldadura**.

**Se puede porque de los cinco hilos de la LCD hay DOS QUE NO LLEVAN DATOS.** Es la medida que
desbloquea todo lo demas:

| pin | que hacia | dato? |
|---|---|---|
| `PB6` (`LCD_PSB`) | **un `digitalWrite(LOW)` en el arranque y nunca mas** — un nivel estatico que elige el modo serie del ST7920 | no |
| `PB7` (`LCD_RST`) | **solo pulsa el reset** al arrancar; el ST7920 arranca sin el | no |
| `PB3`/`PB4`/`PB5` | `SCLK`, `CS`, `SID` | **si** — no se tocan |

La pantalla sigue funcionando con tres hilos (`lcd.cpp`: el constructor recibe `U8X8_PIN_NONE` en vez
de `LCD_RST`, y `lcd_setup()` deja de tocar `LCD_PSB`). El puerto queda en
`static HardwareSerial SerialBT(PB7, PB6);` — `Maestro/src/bluetooth.cpp:25` y
`Esclavo/src/bluetooth.cpp:26`.

> 🔴 **Y al mover el puerto salio un defecto que llevaba ahi desde que el firmware se partio en dos.**
> `protocolo_setup()` abria `AiBus`, declarado sobre `(RS485_IN_RX, RS485_IN_TX) = (PA10, PA9)` — **el
> mismo USART1 que `SerialBT`** — y ademas **a otra velocidad: 115200 contra 9600**. Dos objetos
> peleandose un unico periferico. **Funcionaba por accidente de orden de arranque**: `bluetooth_setup()`
> corre despues y ganaba, de modo que el puerto quedaba a 9600 y **el "puerto de camara IA" nunca
> existio a 115200**.
>
> **La regla que queda: dos objetos sobre el mismo periferico no dan error — dan el que arranco
> ultimo.** No hay compilador ni enlazador que lo cace, no hay `ABORTADO`, no hay `FALLA`: hay un
> puerto que funciona y una funcion documentada que jamas hablo. El delator no fue una prueba, fue
> **mover el puerto**: con `USART1` remapeado, abrirlo alli con el mapeo viejo dejaria dos mapeos
> peleando por el mismo hardware. Un pack que censara «un `HardwareSerial` por UART» lo habria visto
> desde el primer dia.

**Lo que NO se hizo, a proposito:** no se borra `AiBus` ni `protocolo_actualizarAI()`. Es superficie
muerta y su retirada es un cambio con sentido propio — **va en su propio N-x, y es N-86**, que ademas
mide lo que cuesta tenerla ahi.

**Y de paso, una inversion que llevaba desde el 31/07 en varios documentos:** el `MAX3485` del
`USART1` es **`U2`** (par A/B por `J10`); **`U3`** es el de la radio LoRa (par A/B por `J12`). Estaban
al reves y mandaban a soldar sobre el chip equivocado. Queda anotado que **estaba invertido**, no solo
corregido.

**Coste MEDIDO — y aqui hay que corregir una cifra que circulaba.** Se dijo *"+272 B por punta"*. **No
cuadra con la medida.** Comparando el acta anterior (`614065d`) con la actual:

| | antes de N-76 | despues | delta |
|---|---|---|---|
| Maestro | 56.308 B (85,9 %) | **56.260 B (85,8 %)** | **−48 B** |
| Esclavo | 41.920 B (64,0 %) | **41.872 B (63,9 %)** | **−48 B** |

**Bajan 48 B cada una**, no suben 272. Y el `.elf` cuadra con el acta al sumar `.text` + `.rodata` +
`.data` = `42.080 + 13.920 + 260 = 56.260` — que **no** es lo que devuelve `arm-none-eabi-size -B`
(56.580, porque incluye `.isr_vector`, `.ARM`, `.init_array` y `.fini_array`). Es §4 otra vez, sobre
el propio instrumento de medida: **la herramienta existe, responde, y cuenta otra cosa que la que
publica PlatformIO.** El numero de *"+272 B"* no aparece hoy en ningun fichero del repositorio; queda
anotado como **refutado** para que no vuelva.

**Evidencia:** `020c2db` (firmware, 8 ficheros) y `50a5380` (cuatro comentarios que seguian diciendo
`PA9`/`PA10` en las dos `bluetooth.h` y los dos `main.cpp`). Compuerta antes y despues; arnes de
pantalla `271/271`, del automatico `71/71`, del ciclo `29/29`.

> ⚠️ **Los comentarios desfasados los encontro una revision cruzada de documentacion, NO una prueba.**
> Ningun pack vigila que un comentario siga al codigo, y un comentario que dice donde esta el puerto y
> acierta a medias es peor que ninguno: **el que lo lee busca el cable en la bornera equivocada.**

**NADA DE ESTO ESTA VERIFICADO EN LA PLACA.** Todo sale del esquematico y de los arneses de PC.

---

## 📦 V9.0 · Cierre de la auditoría de pines y paquete de revisión (26 de Agosto de 2026)

> **V8.9 nunca se entregó.** Su paquete está en `99_Legacy/Entrega_V8.9_NO_ENTREGADA_reemplazada_por_V9.0.zip`.
> El motivo no fue el código —que compila y pasa la compuerta— sino el sobre: el `LEEME`
> decía *"ENTREGA OFICIAL"* y llamaba al acta *"certificado oficial de funcionamiento"*
> sobre un firmware que **jamás ha estado en un banco**. Un paquete que se lee como un
> permiso es peor que no tener paquete: el que no existe no autoriza a nadie.
>
> V9.0 es el mismo trabajo con el sobre correcto: **`Entrega_V9.0-rc1`, no apta para campo.**

### 🎯 Qué trae V9.0 sobre V8.4 (lo que hay en la calle)

| Bloque | Estado |
|---|---|
| Telemetría Bluetooth en `USART1`, `$STATUS`/`$ALARM`/`$EVENT` con checksum XOR | implementado, **sin banco** |
| Cámaras Hikvision AcuSense por contacto seco en `PB0`/`PB8`, sin ordenador intermedio | implementado, **sin banco** |
| Demanda del Esclavo por radio: `CMD_DEMANDA` (`0x11`) / `CMD_ACK_DEMANDA` (`0x12`) | implementado, **sin banco** |
| Desacoplo de `U3`: `PA8` en HIGH para dejar `PA10` al Bluetooth | implementado, **sin banco** |
| Test de lámparas detrás de la barrera (`semaforo_iniciarTestLeds()`) | implementado |
| App móvil + APK Android (compilación `debug`) | implementada |

### 🟢 N-75 — Dos ABORTADO dejaron entrar cuatro defectos, y la APK no era la APK

**De donde sale:** de un informe. El rewrite de la app a la arquitectura de 2 roles (`caa09c8`)
llego con un parte que decia *"consolidado, probado y subido con exito"* y una lista de artefactos
que existian todos. Al medirlo, la compuerta daba **12 PASS | 1 FALLA | 2 ABORTADO** contra los
**15 PASS | 0 | 0** del dia anterior. El informe no mencionaba `compuerta.py` ni una vez.

**Los dos ABORTADO no eran el sintoma: eran la causa.** Los dos instrumentos que habrian cazado
todo lo demas no llegaron a correr, y por eso pudo entrar detras:

| instrumento | por que aborto |
|---|---|
| `banco por packs` | `documentos_03` busca la rama de `$STATUS` **en `app.js`** y el parser se mudo a `js/nmea_parser.js`. Los **34 packs y sus 328 comprobaciones no se ejecutaron** |
| `app ejecutada en DOM` | `TypeError` sobre `null`: hacia `.click()` en la pestana `tab-control`, que la interfaz nueva ya no tiene |

Es CLAUDE.md §5 en las dos —*contenido que se muda de fichero*— y su consecuencia mas cara hasta
hoy. **Un `ABORTADO` no dice nada del firmware, y eso incluye no decir que hay cuatro defectos
debajo.**

**Lo que entro detras:**

1. **La app quedo SORDA.** `app.js` perdio el `subscribe()` y el manejador de `$STATUS`/`$ALARM`/
   `$ERR` **enteros**. Mandaba ordenes y pintaba un estado que se inventaba el propio telefono:
   el tecnico veia en la pantalla un verde que nadie le habia confirmado. Es la version movil de
   la prueba muerta —todo en verde sin haber medido nada—, y sobre un semaforo.
2. **Se autorizaba sola.** `const pin = state.correctPin || '1234'` inyectaba un PIN valido en
   **todos** los comandos; el modal solo cambiaba el rol de pantalla. `bluetooth.cpp:70-82` pone
   el PIN justo para *"lo que ABRE paso o mueve luces"*. Y la comprobacion que debia vigilarlo
   **daba PASS**: solo miraba que existieran `validatePin()` y la constante, y las dos existian.
3. **Un protocolo imaginario.** `js/nmea_parser.js` leia `FASE`, `TOT_FASE`, `BAT1`, `BAT2`,
   `RADIO` y `TELA`, que **no emite ninguna punta**, y rellenaba defaults —`AUTO`, `VERDE_P1`,
   `12.0 V`— que hacian pasar por equipo sano una trama vacia. Lo probaban 11 de los 29 tests
   anunciados como *"cobertura 100%"*, sobre un modulo que **la app no llamaba**.
4. **Trabajo sin interfaz.** `SOLICITAR_PASO` (N-58) y `SET_MODO:MANUAL` se quedaron sin boton;
   `actualizarCruce()` y `eliminarCruce()` sin llamador. N-73 otra vez.

**Y la APK no era la APK.** `IOT_VIAL_Semaforos_v9.0.apk` contenia el `app.js` de `8d75f4c`
**byte a byte**. Comparada como obliga N-74 —entrada por entrada y por CRC, nunca por tamano—
resulto identica a la del 27/08: **493 entradas, cero nombres distintos, cero CRC distintos**.
Era un renombrado que ademas perdia el marcador `SIN_BANCO`. Y `www/app.js` llevaba el app viejo,
asi que **lo que se probaba en el navegador y lo que se instalaba no eran el mismo programa**.

> **La regla que queda: un instrumento que ABORTA es una puerta abierta, no una casilla pendiente.**
> `ABORTADO no es PASS` ya estaba escrito, pero se leia como *"esa comprobacion no cuenta"*. Lo que
> costo N-75 es la otra mitad: mientras esta abortado, **todo lo que ese instrumento vigilaba entra
> sin mirar**. Los dos que abortaron aqui eran precisamente los dos que ejercen la app.
>
> **Y su corolario sobre los informes: un parte de trabajo no es una medida** (CLAUDE.md §4,
> segunda cara). Este decia *"probado"* de seis artefactos que existian y de 29 tests que pasaban
> de verdad —12 de ellos sobre codigo muerto—. Todo cierto por separado; el conjunto, falso. El
> unico dato que lo habria dicho en diez segundos es el que no estaba: la salida de `compuerta.py`.

**Como se arreglo, que es la parte reutilizable.** Los instrumentos se **reapuntaron, no se
aflojaron**: cada comprobacion fue a *borrar*, *invertir* o *conservar* (§8.quater), y las dos que
median de mentira se endurecieron —la del PIN ya no se conforma con que exista `validatePin()`,
exige que la trama **no se pueda construir** sin verificacion en la sesion—. Los dos arneses se
vieron caer con el defecto inyectado en el `.js` real: **DOM `58` -> `55`** nombrando el fallo del
PIN, **`app_01_comandos` `8/8` -> `6/8`** en las dos direcciones (§8.bis).

**Y una guinda que solo aparecio al re-correr sobre `main` limpio:** la suite unitaria cayo a
`31/32` y volvio a `32/32` **con el mismo codigo**. `SiteManager.agregarCruce()` generaba
`'cruce-' + Date.now()` **sin sufijo**: dos altas en el mismo milisegundo compartian `id`. No se
notaba mientras editar y borrar no tenian llamador; al reconectarlos, un id repetido significa
**renombrar o eliminar OTRO cruce**. El doble de prueba usaba `Math.random()` sobre 1000 valores
—colision en 1 de cada 6 corridas con sus 20 altas, medida en la 14 de 25—. Los dos pasan a un
contador monotono: **un id al azar hace la prueba improbable en vez de imposible, y una prueba
improbable de fallar es la que nadie sabe interpretar el dia que falla.**

**Evidencia:** compuerta **15 PASS | 0 FALLA | 0 ABORTADO**, codigo `0`, sobre `main` con el arbol
LIMPIO — `evidencia/2026-08-28_compuerta.txt`, reproducible con
`git checkout a6d75cb && python 01_Firmware/compuerta.py`. Banco `328/328` en 34 packs, DOM
`58/58`, funcional `42/42`, unitarios `32/32` (40 corridas seguidas sin un fallo).

**Y una segunda mitad, que llego al probarla en un telefono de verdad.** La compuerta estaba en
verde y la APK compilada cuando el responsable la instalo. Aparecieron tres cosas mas, y las tres
dicen algo distinto:

| lo que se vio | lo que era |
|---|---|
| *"no veo el boton de la derecha, DAR PASO y ROJO TOTAL salen a la mitad"* | La cabecera no encogia y ensanchaba el documento entero. **El sintoma y la causa no estaban en el mismo sitio** |
| *"aparece un simulador de pruebas en modo operario, debe ir?"* | No. Se retiro **con su gemelo**, `runLocalTicker()`, que hacia lo mismo sin que nadie lo pulsara |
| *"deja poner 1 minuto, sin mantener el minimo de 3"* | El firmware permite 1 (`VERDE_MIN_MIN`), y la app coincide **exactamente**. **El minimo de 3 no esta escrito en ninguna parte del repositorio** |

> 🔴 **UNA CAPTURA HECHA A UN SOLO ANCHO NO ES UNA PRUEBA DE INTERFAZ.** El desborde se midio con
> el navegador a cuatro anchos: **412 px -> 0 px, 390 -> 11, 360 -> 41, 320 -> 81**. Las capturas
> del `evidencia/` estaban limpias porque se hicieron a **412 px, el unico de los cuatro donde el
> fallo no aparece**. Un `.png` de una interfaz demuestra que a ESE ancho se veia bien, y nada mas.
>
> Y la causa raiz es la de siempre en CSS: **un hijo flex no baja de su ancho de contenido salvo
> que se le diga** (`min-width: auto` por defecto). Quien mira donde duele encuentra la botonera;
> quien mide encuentra el boton "Dispositivo" de la cabecera, tres bloques mas arriba.

> 🔴 **Un panel de demo que escribe en los MISMOS widgets que el dato real es la version de
> interfaz de la prueba muerta.** El *"SIMULADOR DE PRUEBAS - DEMO EN VIVO"* pintaba fases,
> bateria baja y radio caida sobre los mismos semaforos y el mismo contador que la telemetria, y
> avisaba con un *toast* que se va solo. Su gemelo era peor: `runLocalTicker()` animaba un ciclo
> completo **sin que nadie lo pulsara**, en cuanto no habia equipo conectado.
>
> **Lo que sustituye a un dato que no se tiene no es una simulacion: es decirlo.** Sin enlace la
> pantalla se congela y el rotulo pasa a `SIN ENLACE - sin datos del equipo`. Un tablero quieto
> que lo admite es honesto; uno que anima un cruce que no existe le miente a quien decide sobre
> el trafico mirandolo.

> ⚠️ **Y un limite que se recuerda no es un limite que exista.** Se pidio *"el minimo de 3 minutos"*
> de una simulacion antigua. Buscado: **no aparece en ningun `.md`, ni en el firmware, ni en un
> pack**. Lo unico escrito es `VERDE_MIN_MIN = 1` en `modo_automatico.cpp:31`, y la app valida
> **exactamente los mismos cuatro numeros**. No hay desajuste que arreglar; hay una **decision sin
> tomar**, y su sitio es el C++ -`bluetooth.cpp:123-126` ya explica por que los rangos viven en
> `modoAutomatico_fijarTiempos()` y no en el despachador-.
>
> **Pendiente y anotado:** los cuatro limites estan hoy escritos **dos veces** -en el C++ y a mano
> en `app.js:734` y en los `min`/`max` del formulario-, sin nada que los ate. Hoy coinciden. Hace
> falta un pack que lea los cuatro del `.cpp` y exija que la app publique esos y no otros, o el dia
> que alguien suba el minimo a 3 la app seguira dejando poner 1.

**Cerrado tambien:** la APK **esta compilada** (`BUILD SUCCESSFUL`, 499 entradas) y **verificada
por contenido**, no por confianza: sus nueve ficheros web son identicos byte a byte al repositorio
y dentro se comprueba que no quede el panel de demo ni el ciclo local. Las cuatro trampas de N-74
volvieron a valer tal cual. **Sigue SIN BANCO, y el nombre del fichero lo lleva dentro.**

### 🟢 N-74 — La APK, compilada de verdad y no solo verificada

**De donde sale:** de una correccion directa —*"pero bb, la apk no la compilaste"*— al entregar el
paquete. **Y era cierta.** Habia verificado su contenido —sus tres ficheros web son identicos byte
a byte a los del repositorio—, que es una medida buena, **pero no es lo mismo que haberla
construido**.

**Compilada, y las cuatro trampas que costo encontrar** (ahora en la skill `entregar`, para que no
se vuelvan a buscar):

| | |
|---|---|
| **JDK 17, no 21** | con el 21 muere en `JdkImageTransform ... core-for-system-modules.jar`: este AGP no lo soporta |
| **SDK fuera de rutas con espacios** | vive en `... sw apkndroid-sdk` y desde ahi falla. Union a `C:ndroid-sdk`. **Es N-44 aplicado al SDK de Android** |
| **`local.properties` con barras normales** | es un fichero de propiedades de Java: en `C\:ndroid-sdk` el `` se come la barra y queda `C:android-sdk` |
| **el `.zip` no se hace con `Compress-Archive`** | muere por el `PSModulePath` heredado, el mismo fallo que `CLAUDE.md` §4 ya documenta para `Get-FileHash` |

**Y el resultado confirma que el paquete anterior era correcto, medido en vez de supuesto:**

```
entradas: nueva=493   la del paquete=493
solo en la NUEVA (0): []
solo en la del ZIP (0): []
mismas rutas con CONTENIDO distinto (0): []
```

**Cero diferencias.** Los 104 KB de diferencia de tamaño —3.806.717 B contra 3.911.388— son
compresion y alineado del contenedor, no contenido.

> **Y de ahi sale la regla que se queda: dos APK del mismo contenido NO tienen el mismo `md5`.**
> Comparar el fichero entero da "distintas" y comparar el tamaño da "distintas": las dos respuestas
> son ciertas y las dos son inutiles. **La comparacion que sirve es entrada por entrada y por CRC.**
> Es §4.bis —*"compara hashes, no tamaños"*— con su letra pequeña: **el hash del contenedor
> tampoco decide; hay que comparar lo que el contenedor lleva dentro.**

### 🔴 N-73 — La Caja Negra que cuatro manuales anunciaban no la llamaba nadie

**De donde sale:** de ir a preparar el `.zip` para el funcional y comprobar, antes de empaquetar,
si los manuales seguian diciendo la verdad tras subir el umbral de N-71. Uno de ellos citaba una
trama de alarma: `$ALARM,NODE:MAESTRO,EVENTO:FALLO_RF_12S,...`.

**Dos cosas mal, y la segunda es la grave.** La primera, que el nombre del evento llevaba el numero
dentro y N-71 acababa de convertirlo en mentira. La segunda, al buscar quien lo emitia:

> **`bluetooth_reportarAlarma()` esta declarada, definida y documentada con ejemplo en las DOS
> puntas —y no la llama NADIE.** La *"Caja Negra de Alarmas"* que **cuatro manuales** describen como
> *"registro inmediato de eventos con timestamp para diagnosticar la causa exacta de cualquier caida
> de radio en obra"* **no ha emitido jamas una sola alarma.**

Es la forma exacta de N-63 —un `pinMode()` sin `digitalRead()`— pero con documentacion encima. Y se
pago la semana pasada: **el reporte de campo de la lluvia no se pudo diagnosticar porque no habia
registro que mirar.** El instrumento existia; nadie lo habia enchufado.

**Descartando al buscador antes de acusar (§4):** el mismo `grep` encuentra `bluetooth_reportarEvento`
en 15 sitios y `bluetooth_loop` llamado desde `main.cpp`. El buscador sabe encontrar; era la alarma
la que no estaba.

**Que se conecto, y donde.** Las tres puertas por las que un cruce se va a ambar, distinguibles por
la causa —que es justo lo que faltaba para diagnosticar la lluvia—:

| punta | cuando | causa que emite |
|---|---|---|
| Esclavo | vence el silencio de SFTY-6 | `SILENCIO_25000ms` |
| Maestro | vence el silencio de SFTY-6 | `SILENCIO_25000ms` |
| Maestro | se agotan los 5 reintentos | `REINTENTOS_AGOTADOS` |

**Distinguir *"se agotaron los reintentos"* de *"silencio total"* es la diferencia entre un enlace
que se degrada —lluvia, distancia, interferencia— y uno que se corta.** Hasta hoy las dos caidas se
veian igual desde fuera: una luz ambar.

**Y el nombre del evento ya no lleva el numero.** El umbral va en la causa. `FALLO_RF_12S` habria
quedado mintiendo el dia de N-71, y el siguiente que lo tocara.

**Coste:** Maestro 85,6 % → **85,9 %** (+224 B), Esclavo 63,7 % → **64,0 %** (+184 B).

#### El censo completo, que es lo que hace que esto no vuelva

No se arreglo solo el caso preguntado. Se censaron **todas** las funciones declaradas en los headers
de las dos puntas y se conto quien las llama: **139 declaradas en el Maestro, 95 en el Esclavo, y 29
sin un solo llamador.**

Revisadas una por una contra los documentos, que es lo que separa un hallazgo de una lista:

| | |
|---|---|
| las 4 de la franja nocturna | `OPTIMIZACIONES.md` las declara **"DISENO, NO IMPLEMENTADO"** y el protocolo de pruebas *"especificada, sin construir"*. **El documento no miente**: no es un hallazgo |
| `semaforo_iniciarTestLeds` y `semaforo_forzarVerde` del Esclavo | **es la barrera funcionando**: el Esclavo no enciende luces por su cuenta y rechaza `TEST_LEDS` |
| getters de telemetria que la pantalla dejo de pedir | no danan; quedan anotados |
| **`bluetooth_reportarAlarma`** | **la unica anunciada como funcion existente. Ese era el defecto** |

**`costura_10_funciones_muertas` (13 chk) congela ese censo como un TRINQUETE**, no como un
absoluto: exigir "cero huerfanas" seria falso y ruidoso. Falla si aparece una **nueva** —codigo
recien escrito que nadie ejecuta, o peor, algo que si se llamaba y se quedo sin llamador—, falla si
una de la lista **gana** llamador (para que salga de la lista, o los nombres obsoletos la vacian de
sentido), y exige aparte que **las funciones que los manuales anuncian tengan llamador**. Roto a
proposito: desconectando la alarma del Esclavo cae a `11/13` y **lo caza por las dos vias**.

**Y el arnes del automatico no se limita a un stub mudo.** Enlazaria igual, y nadie sabria si la
alarma sale —que es el defecto que este N-x arregla—. Su stub **captura** lo reportado y el arnes
exige que al agotar los reintentos haya salido `FALLO_RF/REINTENTOS_AGOTADOS`: **medido sobre el
`coordinador.cpp` real compilado y ejecutado**, no sobre un modelo. 70/70 → **71/71**.

### 🟢 N-72 — La app validada de verdad, y tres cosas que el informe no decia

**Que se midio, no lo que se conto.** El subsistema de la app llego con un informe de otro agente;
se corrio entero antes de darlo por bueno, y **la medida coincide con el informe**:

| | |
|---|---|
| unitarios puros de la app | **32/32** |
| ejecucion real en DOM (jsdom) | **53/53** |
| funcional estatico | **34/34** |
| estres app + Bluetooth | **5/5 suites** |
| compuerta completa | **15 PASS · 0 FALLA · 0 ABORTADO** |

El **Gestor de Cruces Viales** (CRUD reactivo en `localStorage`) se ejerce a escala real: 20 cruces
= 40 semaforos, borrado masivo de 17, re-agregado de 4 y renombrado reactivo con la cabecera
actualizandose. Y retira el `confirm()` bloqueante, **que en un WebView de Android no es un detalle
de estilo: es un dialogo que puede no volver**.

#### 1. Un commit de seguridad viajando dentro de un commit de pruebas

Revisando **por el diff y no por el informe** (§8.ter) aparecio esto:

> **`ff6bd19 fix(N-71): el techo de silencio...` contiene UN SOLO fichero: el acta.** Todo el
> firmware de N-71 —el umbral de SFTY-6, los dos packs, los cuatro instrumentos recalibrados—
> esta dentro de **`1bf9251`, cuyo mensaje dice *"validar ciclo completo de insercion, borrado de
> 17 y re-agregado de 4 cruces"***.

**Causa:** dos agentes trabajando sobre el MISMO arbol de trabajo. Uno hizo `git add` de ficheros
que el otro tenia en vuelo. El contenido es correcto y la compuerta esta verde; **lo que esta roto
es la historia**, y eso se cobra tarde: *"un commit = un cambio con sentido propio = un `git revert`
limpio"*. Hoy `git revert ff6bd19` **no deshace N-71**, y revertir `1bf9251` se llevaria por delante
las pruebas de la app.

**No se reescribe.** La rama esta publicada en dos remotos y otro agente sigue trabajando encima:
un `push --force` para arreglar un mensaje causa mas dano del que repara. Queda anotado aqui, que es
donde alguien lo buscara: **el codigo de N-71 vive en `1bf9251`**.

**Y la leccion operativa, que es la parte reutilizable:** cuando haya dos agentes a la vez, o cada
uno en su `git worktree`, o cada uno con `git add` de rutas explicitas —nunca `-A`—. El arbol
compartido no avisa: simplemente mezcla.

#### 2. Dos ficheros distintos llamados igual

Medido con `md5sum`, no por tamano (la leccion de §4.bis):

```
8f809ef0...  05_Funcional/IOT_VIAL_Semaforos_2026-08-27_2ff686f_SIN_BANCO.apk   (versionada)
8f809ef0...  05_Funcional/App_Semaforo/IOT_VIAL_Semaforos_v8.9.apk              (versionada)
83deb940...  05_Funcional/IOT_VIAL_Semaforos_v8.9.apk                           (sin versionar)
```

Las dos primeras son **el mismo binario con dos nombres**. La tercera es **otro binario con el mismo
nombre que la segunda**. A partir de aqui, *"instala la v8.9"* es una instruccion ambigua.

Por eso la convencion del repositorio es `IOT_VIAL_Semaforos_<fecha>_<hash>_SIN_BANCO.apk`: el
nombre dice de que commit salio. **La nueva no se ha renombrado ni versionado a proposito**: no se
puede certificar de que commit se compilo sin haberlo visto compilar, y estampar un hash que no se
ha medido es exactamente lo que §4 prohibe. Que la nombre quien la construyo.

#### 3. Lo que la app sigue sin poder probar

Las 53 comprobaciones de jsdom ejercen la app **contra un firmware simulado en JavaScript**. Ninguna
de ellas ha hablado nunca con un STM32. La costura app↔firmware que si es real —que todo comando
que la app manda lo atienda alguna punta— la cubre `app_01_comandos` leyendo los dos `.cpp`. **El
Bluetooth de verdad, sobre la tarjeta, sigue sin medirse: es la pregunta 6 del acta de banco.**

### 🔴 N-71 — El techo de silencio estaba por DEBAJO de los reintentos que el firmware hace

**De donde sale:** de un reporte de campo del 27/08 —*"se pasa sin mas a modo degradado cada nada
cuando llueve"*— y de ir a medir el codigo antes de tocar el numero.

**Lo que se encontro no necesita lluvia.** `SFTY6_SILENCIO_MS` no es un numero independiente: es el
**techo de todo el presupuesto de radio**, porque quien primero llegue manda. Y estaba por debajo de
lo que el propio firmware tarda en agotar sus reintentos:

| medido en `coordinador.cpp` | |
|---|---|
| cadencia del latido (lo mas tarde que arranca un intercambio) | 3,0 s |
| `CICLO_MAX_REINTENTOS` x `TIMEOUT_ACK_MS` = 5 x 3,5 s | 17,5 s |
| **peor caso** | **20,5 s** |
| `SFTY6_SILENCIO_MS` | **12,0 s** ← techo |

**Los reintentos 4 y 5 no podian ejecutarse jamas.** El ambar por orfandad saltaba sobre el segundo
o el tercero. Es codigo muerto dentro del mecanismo de recuperacion —y no lo delataba nada, porque
el equipo hacia algo razonable: irse a ambar—. Con lluvia, que tumba tramas sueltas que un reintento
habria recuperado, eso se convierte en el sintoma reportado.

**Y el comentario que lo acompanaba mentia con una cuenta hecha.** Decia `// Fallo tras 5 reintentos
(12.5s)`: cierto con el `TIMEOUT_ACK_MS` de 2500 ms **que se retiro el 31/07**. Otro decia *"con 3500
ms caben 4 intentos"*, cuando el codigo hace 5 y no cabian ni 3. **Un comentario no falla cuando
alguien cambia un numero: se queda ahi describiendo un equipo que ya no existe, y encima con la
autoridad de una cuenta.**

**El cambio, dimensionado desde la medida:**

| | antes | ahora | por que |
|---|---|---|---|
| `SFTY6_SILENCIO_MS` | 12 s | **25 s** | cubre los 20,8 s del peor caso con 4,2 s de margen |
| `SYNC_MAX_INTENTOS` | 2 | **3** | el unico motivo del 2 era el techo viejo |
| cadencia del latido | `3000` desnudo | **`LATIDO_MS`** | era la misma forma del defecto de N-69 |
| reintentos del ciclo | `5` repetido dos veces | **`CICLO_MAX_REINTENTOS`** | dos literales que podian divergir |

**Lo que cuesta subirlo, dicho sin adornos:** el cruce puede quedarse hasta 25 s en la fase que
tuviera al caer el enlace, en vez de 12. **No puede aparecer un verde nuevo en ese rato** —ninguna
punta enciende verde sin el ACK de la otra—, asi que lo que se alarga es una espera, no un riesgo de
verde simultaneo.

**Lo vigila `costura_09_presupuesto_radio` (9 chk)**, que recalcula la desigualdad desde las
constantes del C++ en cada corrida y **falla contra el firmware de ayer** (9 → 7). Vigila las dos
direcciones: que quepa el peor caso, y que el techo no lo desborde sin sentido.

---

#### Y por el camino, la cuarta cara de N-46: la compuerta no sabia ver estos fallos

Al subir el umbral, **tres pruebas del simulador funcional y dos del repetidor empezaron a fallar**
—daban por sentado el ambar a los 12 s—. La compuerta escribio `[OK]` en las dos.

El detector de N-46 busca la marca literal `[FALLA]`, que es la que imprimen los packs del banco...
y **ninguno de los dos simuladores mas viejos**. Aquellos escriben `X FAIL:` y cierran con
`VEREDICTO FINAL: 17/20 PASS - HAY FALLOS PENDIENTES`, saliendo con codigo 0.

> **El simulador funcional podia caer de 20/20 a 17/20 y el acta seguia diciendo `[OK]`, con la
> cuenta mala escrita al lado —que nadie lee cuando el semaforo de la izquierda esta verde—.**

**La regla nueva no depende del marcador, que es justo lo que fallaba:** si un instrumento publica
una cuenta `x/y`, se exige `x == y`. Un instrumento que anuncia 17 de 20 esta diciendo que tres
comprobaciones no cumplen, lo escriba como lo escriba y salga con el codigo que salga.

#### Las pruebas que celebraban el comportamiento viejo (§8.quater), una por una

| | que se hizo | por que |
|---|---|---|
| simulador: `range(150)  # 15 segundos` | **conservada, recalibrada** | median el fail-safe, no el segundo 15: la espera se deriva de `FALLBACK_S` |
| simulador: `avanzar_simulacion(13.0)` | **conservada, recalibrada** | idem, y su texto de PASS citaba *"tras 12s"* |
| repetidor: `banco.avanzar(13.0)` | **conservada, recalibrada** | idem |
| repetidor: `banco.avanzar(3.0)` del reenganche | **conservada, redefinida** | ver abajo |
| `maestro_04`: *"N es el maximo que cabe"* | **invertida** | con el techo holgado, *"usa el maximo"* es mal consejo: cada intento roba canal. Ahora acota la **ocupacion** al 60 % del techo, que hoy permite 3 y rechaza 4 |
| `arnes_automatico`: *"las DOS cuentas coinciden"* | **invertida** | existia porque el `5` estaba duplicado. Ahora exige **una** declaracion y **dos** usos por nombre: mas fuerte |

**El caso del reenganche merece su parrafo, porque es donde era facil hacer trampa.** La prueba
esperaba `3.0` s, que es **exactamente** un periodo de latido: vivia en la frontera y solo pasaba por
como caia la fase. Medido: con 3,0, 3,5 y 4,0 falla; desde 4,5 pasa. **No se subio el numero hasta
que dejo de fallar** —eso es ajustar el instrumento hasta que de verde—: se derivo del peor caso
real de un reenganche, `LATIDO_S + TIMEOUT_ACK_S`.

**Y cuatro instrumentos ABORTARON al dar nombre a los literales** —`modelos/maestro.py`,
`modelos/esclavo.py`, el pack del mando y el arnes del automatico buscaban `retryCount >= (\d+)` y
`tUltimoPing > (\d+)`—. **Abortaron en vez de aprobar**, que es su trabajo (§5): un instrumento que
pierde su patron no puede seguir dando veredicto.

**Lo que este cambio NO demuestra:** que la causa del reporte de lluvia sea esta. Lo medido es que
el mecanismo de recuperacion estaba mutilado y que eso produce exactamente ese sintoma. **La
atenuacion por lluvia no se ha medido**, y el firmware ya muestra `calidadPct` en pantalla: esa es
la lectura que hay que traer de campo antes de escribir una causa aqui.

### 🟢 N-70 — Cinco mil bytes de un bus que el equipo no tiene

**De donde sale:** de una pregunta de una linea —*"y la medida de flash, algo a optimizar"*— hecha
justo despues de que yo escribiera que quedaban 4.292 bytes y que *"la proxima funcion que entre
tiene que venir con su medida, o con algo que salga"*. Esa frase daba por sentado que lo que llena
la flash es firmware. **Nadie lo habia medido nunca.**

**Primer censo, y su error.** `arm-none-eabi-nm --size-sort -S` sobre el `.elf`, agrupando los 692
simbolos por su NOMBRE, daba *"solo un tercio es firmware propio"*. Es falso: los simbolos de C++
de cualquier libreria tambien empiezan por `_Z` —`TwoWire::setClock` es `_ZN7TwoWire8setClockEm`—,
asi que el clasificador metia libreria ajena dentro de *"lo nuestro"*. **El censo bueno no adivina
por el nombre: lo reparte por fichero objeto, que es lo que `firmware.map` sabe de verdad.**

| Maestro | antes | despues | |
|---|---|---|---|
| firmware propio (`Maestro/src`) | 18.505 B | 18.505 B | — |
| **HAL de STM32** | **14.800 B** | **11.068 B** | **−3.732** |
| libreria u8g2 (codigo + 7 fuentes) | 13.936 B | 13.936 B | — |
| libc / libgcc | 7.077 B | 7.061 B | −16 |
| core de Arduino | 3.201 B | 3.201 B | — |
| RTC / watchdog | 2.270 B | 2.270 B | — |
| **Wire / I2C** | **1.080 B** | **0** | **−1.080** |
| **SPI por hardware** | **332 B** | **0** | **−332** |
| **total** | **61.297 B** | **56.137 B** | **−5.160** |

El firmware propio son 18.505 B: **el 30 % del binario**. El resto es libreria, y ahi habia una fila
que no tenia por que existir —el bus por hardware esta copado (`PB6`/`PB7` los usa la LCD,
`PB10`/`PB11` el RS-485), las dos pantallas son `U8G2_ST7920_128X64_F_SW_SPI`, y un `grep` sin
comentarios de las tres puntas devuelve **cero** usos de `Wire` y **cero** de `SPI`—.

**La causa NO se dedujo: estaba escrita.** `firmware.map` lleva la seccion de quien arrastra a quien:

```
libU8g2.a(U8x8lib.cpp.o)  ->  TwoWire::setClock(unsigned long)
libWire.a(Wire.cpp.o)     ->  i2c_init
libWire.a(twi.c.o)        ->  el HAL de I2C entero
```

`U8x8lib.cpp` trae en **una sola unidad de compilacion** los transportes de todos los backends
—SW_SPI, HW_SPI e I2C—, asi que referenciar cualquiera arrastra el objeto completo y con el, `Wire`.
La libreria lo tenia previsto: `U8x8lib.h` define `U8X8_HAVE_HW_I2C` **salvo que se declare
`U8X8_NO_HW_I2C`**.

**Resultado, medido en las dos puntas quitando y poniendo las banderas sobre el mismo arbol:**

| | antes | despues | |
|---|---|---|---|
| Maestro flash | 61.244 B — **93,5 %** | 56.084 B — **85,6 %** | −5.160 B |
| Esclavo flash | 46.896 B — 71,6 % | 41.736 B — **63,7 %** | −5.160 B |
| RAM (cada punta) | 4.148 / 3.992 B | 3.796 / 3.640 B | −352 B |

**Dos lineas de `platformio.ini`. Cero cambios de codigo. Los bytes libres del Maestro pasan de
4.292 a 9.452**, y con eso SFTY-29 y `FW-PAIR` dejan de competir por el mismo hueco.

**Y una correccion que se queda escrita, porque es la regla del instrumento otra vez.** La primera
version de esta nota publicaba *"−5.492 B y −1.892 B de RAM"*. Ese numero salio de restar contra un
`.elf` que estaba en disco de una compilacion anterior, no contra una compilacion del arbol actual.
Era la cifra correcta de ninguna pareja de binarios. **Un delta exige medir los DOS extremos, no
uno.**

**Por que esto es un pack y no un commit suelto.** Una bandera de compilacion no la protege nada: no
hay error, no hay aviso, y el dia que alguien reescriba el `.ini` —o copie el del Repetidor, que no
las lleva porque no tiene pantalla— la flash sube 5 KB de golpe y el sintoma sera *"ya no cabe"*,
semanas despues y sin relacion aparente con el commit que lo causo. `flash_01_lastre` (11 chk) se
rompio a proposito antes de conectarlo —bandera comentada → `10/11` y `FALLA`— y **vigila las dos
direcciones**: el dia que entre el `PCF8574`, que si es I2C, exigira que las banderas SALGAN, en vez
de dejar un fallo de enlazado sin causa visible.

**Lo que queda medido y NO se toco, por si hace falta mas margen:** de los 13.936 B de u8g2,
**10.187 B son las siete fuentes tipograficas**, y dos de ellas se usan **una sola vez cada una**:
`ncenB14` (2.088 B) en `lcd.cpp:526` y `ncenB12` (1.806 B) en `lcd.cpp:235`. Cambiarlas por
`7x14B_tr`, que ya esta enlazada, libera **3.894 B** mas. No se hizo porque **cambia lo que el
tecnico ve en pantalla**, y eso es una decision de producto, no una limpieza.

### 🟠 N-69 — Los tiempos del ciclo: quien los cambia de verdad, y el 12000 escrito tres veces

**De donde sale:** de una pregunta directa —*"los 3 minutos entre semaforos o los 12 segundos para
pasar a ambar, son parametrizables en la app?"*— y de contestarla midiendo en vez de recordando.

| tiempo | donde vive | quien lo cambia HOY |
|---|---|---|
| **Rojo y verde del ciclo** (`minRojo`, `minVerde`, en MINUTOS) | `modo_automatico.cpp:112` | **la pantalla del Maestro**, en el asistente del Modo Automatico |
| **Despeje todo-rojo** (`segEstatico`, en segundos) | idem | **la pantalla del Maestro** |
| **Ambar por perdida de enlace** | literal `12000` | **NADIE.** No es configurable |
| `cfgVerdeSeg` / `cfgDespejeSeg` *(los que viajan por radio)* | `coordinador.cpp:122` | **solo el Modo Degradado**, con `DEG_VERDE_SEG = 30` y `DEG_DESPEJE_SEG = 30` compiladas |

**Respuesta a la pregunta: NO, la app no puede cambiar ninguno.** Su despachador solo conoce
`SET_MODO`, `FORZAR_ROJO`, `MANUAL:CAMBIAR_TURNO`, `TEST_LEDS`, `SET_RTC` y `SOLICITAR_PASO` — no hay
un solo comando de configuracion. De hecho el `GET_CONFIG` que la app mandaba se retiro en N-66
precisamente porque no existia al otro lado.

**Y hay un hallazgo de paso, que es el que mas duele:** el umbral de 12 s esta escrito **tres veces
como literal**, en `Esclavo/src/main.cpp:538` y en `Maestro/src/coordinador.cpp:632` y `:650`. Es el
mismo numero gobernando las dos puntas de una regla de seguridad —cuando el equipo se rinde a ambar—
**mantenido a mano en tres sitios**. Cambiar uno y olvidar otro deja al Maestro y al Esclavo
rindiendose en instantes distintos, que es exactamente la clase de desincronizacion silenciosa que
costo N-49.

**Lo que se decide a partir de aqui** (pendiente de aprobar, no hecho):

1. **Una sola constante** para el umbral de orfandad, en el contrato compartido, y un pack que exija
   que **las dos puntas usen la misma** — como ya se hace con el checksum y con la barrera.
2. **Comando de configuracion por Bluetooth**, para que los tiempos se ajusten desde el celular sin
   subir a la pantalla del poste. Con **limites duros en el firmware**, no en la app: el despeje es
   seguridad vial, y un maximo y un minimo que el C++ rechace valen mas que un campo de texto bien
   educado. El pack tendra que probar que **un valor fuera de rango se rechaza**, no que uno bueno se
   acepta.

### ✅ N-68 — Las camaras 2 y 4 vuelven, pero como PRESENCIA y no como conteo

**Como cambio la decision:** se habia recomendado no instalarlas, y el argumento era bueno **contra
el conteo**: mandar un mensaje por cada vehiculo que entra y sale del tramo no cabe en un enlace de
2,4 kbps semiduplex que ya lleva las ordenes de las luces, y encima **autoriza a acortar** el
todo-rojo, de modo que una deteccion perdida se paga con dos vehiculos de frente.

**El responsable corrigio el planteamiento, y la correccion es la buena:** esas camaras no cuentan,
**detectan presencia** — como validacion despues del tiempo programado, y como sensor para no bajar
la pluma sobre un coche.

**Eso invierte el riesgo, que es lo que la hace aceptable:**

| | conteo | presencia |
|---|---|---|
| autoriza | **acortar** el despeje | **retrasar** el verde |
| si falla la deteccion | se acorta lo que no debia -> choque frontal | se cae al temporizador de hoy |
| si detecta de mas | — | se espera un poco mas |

**Y la objecion de la radio se cae sola:** es **un bit**, y ni siquiera necesita trama nueva. Medido
el 27/08: el Esclavo manda `CMD_ACK_RED` con `param = 0` por defecto y el Maestro **no lo lee**
(`coordinador.cpp:743`). El byte esta libre y llega justo en el instante que importa. **Cero comandos
nuevos, cero bytes nuevos.**

**Lo que queda escrito en la spec** (`OPTIMIZACIONES.md` §SFTY-29), y lo que mas importa de ella:
**el veto necesita un TOPE**. Barro en el lente o un camion aparcado en el punto y el cruce se
congelaria para siempre. La regla es extender hasta un maximo configurable, y al llegar **cambiar
igual y levantar alarma**. Un enclavamiento sin tope no es mas seguro: es un semaforo colgado.

Y de ahi sale la comprobacion que nadie escribe, ya especificada para su pack: forzar la presencia a
activa **para siempre** y exigir que el cruce cambie igual. Un pack que solo probara el caso bueno
estaria certificando el cuelgue.

**Decision del cliente el 27/08: van las cuatro camaras.** Falta una segunda entrada por poste —la
decide la pregunta 5 del acta de banco: via libre de `J16`, pad de `PB8`, o placa hija— y medir el
flash antes de escribir la primera linea.

### 🔴 N-67 — La entrada de camara estaba leida al reves, y habria dado demanda permanente

**Como salio:** de ir a escribir en la guia de banco *"la camara se conecta aqui"* y no saber si el
otro borne de `J14` era masa o 3,3 V. Al medirlo aparecio que la pregunta tenia una respuesta que el
firmware contradecia.

**Lo medido en el esquematico bueno:**

```
   PB0 (pin 18) --+-- R64 10 kOhm --> GND        (pull-DOWN: fija el reposo en BAJO)
                  +-- C25 100 nF   --> GND        (antirrebote de 1 ms por hardware)
                  +-- bornera J14, que saca el pin JUNTO A 3,3 V
```

O sea que **el contacto seco de la camara se cierra contra los 3,3 V del propio `J14`**: la entrada
es **activa en ALTO**, y no hace falta traer masa ni poner resistencias.

**Lo que hacia el firmware, en las dos puntas:**

```cpp
   pinMode(CAM_DEMANDA_PIN, INPUT_PULLUP);          // pull-up interno ~40 kOhm
   ... digitalRead(CAM_DEMANDA_PIN) == LOW          // deteccion activa en BAJO
```

**Dos fallos a la vez, y el primero es peor:**

1. El pull-up interno (~40 kOhm) contra el pull-down de 10 kOhm de la placa deja el pin en
   `3,3 x 10/50 = 0,66 V`, que el micro lee **LOW**. El firmware habria visto **demanda permanente
   desde el arranque, sin ninguna camara conectada**.
2. Y al cerrar el contacto el pin sube a 3,3 V —**HIGH**— que ese mismo codigo lee como *"no hay
   demanda"*. Invertido.

**Arreglado:** `pinMode(..., INPUT)` a secas —el reposo lo fija el 10k de la placa— y deteccion
contra `HIGH`, en las dos puntas y en el antirrebote del Maestro.

**Y el instrumento, que es lo que impide que vuelva:** `camara_01_demanda` gana tres comprobaciones
que atan la polaridad **a la placa y no al gusto de nadie**: que el `pinMode` sea `INPUT` en las dos
puntas, y que **todas** las lecturas comparen contra `HIGH`. 11 -> 14 comprobaciones.

> **Por que no lo habia cazado nadie:** este defecto **no se ve en el PC**. Los modelos leen el pin
> que quieren, y los arneses no tienen bornera. Solo aparece con el contacto cableado — y para
> entonces ya hay alguien subido a un poste preguntandose por que el semaforo pide paso solo.

### 🔴 N-66 — La app mandaba comandos que ningun firmware conoce, y no ofrecia uno que si existe

**Como salio:** de preguntar *"como validamos la app, que tiene muchos errores"* y convertir esa
frase en una medida. El JavaScript y el C++ los escribe gente distinta en dias distintos, y el unico
sitio donde se encuentran es **un string**. Nadie cruzaba los dos conjuntos.

| lo medido | consecuencia |
|---|---|
| La app manda `GET_STATUS` **al conectar, dos veces**, y `GET_CONFIG` al leer la EEPROM | Ninguna punta los conoce: el despachador cae al `else` y contesta `$ERR,CMD:DESCONOCIDO`. **Lo primero que veia el tecnico al conectarse era un error** |
| El Esclavo atiende `SOLICITAR_PASO` desde V9.0 (N-58) | **La app no tenia boton.** Firmware sin interfaz: la funcion que evita subir con escalera al Maestro no se podia usar |

**Arreglado:** fuera los dos comandos fantasma —el firmware ya emite `$STATUS` cada segundo por su
cuenta, asi que pedirlo no aportaba nada— y anadido el boton de **SOLICITAR PASO**, con PIN como todo
lo que ABRE paso. El unico sin PIN sigue siendo el rojo de emergencia, que lo cierra.

**El instrumento: `app_01_comandos`** (8 comprobaciones, 2 controles negativos). Es `costura_03`
aplicada a la otra frontera: lee el despachador del `bluetooth.cpp` de las dos puntas —**una cadena
de `strcmp()`, sin tabla ni enum: el contrato ES esa cadena**— y lo cruza con lo que la app manda de
verdad. En las dos direcciones: ningun comando huerfano, y ninguna funcion del firmware sin forma de
llegar a ella.

> ⚠️ **Y el pack se equivoco primero, que es la parte que conviene contar.** Su primera version leia
> solo `executeCommand('...')` y `data-cmd`, y acuso a la app de no mandar `FORZAR_ROJO` ni
> `TEST_LEDS` —que manda desde hace meses, por `openPinModal()`, que guarda el comando y lo ejecuta
> al validar el PIN—. **Un "no aparece" no es un hallazgo hasta haber descartado al buscador**: se
> corrigio el extractor antes de tocar una linea de la app, y entonces quedaron los dos defectos
> reales en vez de cuatro inventados.

**Lo que este pack NO puede ver, y hay que decirlo:** compara textos. **Nadie ejecuta el JavaScript.**
Un `TypeError` en tiempo de ejecucion, un `null` del DOM o un boton que no dispara siguen siendo
invisibles para todo el banco. Ese es el siguiente escalon de la validacion de la app.

### ✅ N-65 — El censo de comandos miraba un solo fichero, y por eso no veia CMD_DEMANDA

**Encontrado** al comprobar la fila `BANCO-PACKS` de `ESTADO.md`, que acusaba a `costura_03` de no
contar *"dos `CMD_*` nuevos"*. Medido: contaba `CMD_ACK_DEMANDA` sin problema; el que no veia era
**`CMD_DEMANDA`**. La causa no era la que estaba escrita.

**El agujero:** el pack censaba las emisiones del Esclavo sobre `main.cpp` y **una lista de ficheros
escrita a mano**. El Esclavo emite `CMD_DEMANDA` desde `demanda.cpp:26`, que no estaba en la lista.
Un `.cpp` nuevo que emita paquetes era **invisible**, y la comprobacion salia en verde por no haber
mirado donde hacia falta — la misma forma del defecto que N-62 encontro en los documentos.

**Arreglado** sustituyendo las cuatro listas escritas a mano por `fw.fuentes_de(punta, "src")`, que
censa el **directorio**. Emisiones y atenciones, en las dos puntas. El Esclavo pasa de **6 a 7**
comandos emitidos, con `CMD_DEMANDA` dentro, y el pack sigue en `PASS`: **el Maestro ya lo atendia**
en `coordinador.cpp:595`. O sea que el firmware estaba bien y **el ciego era el instrumento**.

**Trabajo delegado, y revisado por el diff** (`CLAUDE.md §8.ter`): se corrio el pack y la compuerta
sobre el arbol propio antes de aceptarlo, se leyo el diff entero buscando si *replicaba* el arreglo o
*relajaba* la comprobacion —replica: `pkt.command` y `pkt->command` siguen los dos, y el control
negativo nuevo exige ver `CMD_DEMANDA` fuera de `main.cpp`—, y se anota una pega: **los tres
controles negativos quedaron fundidos en una sola comprobacion**, asi que si falla uno no se sabe
cual. No invalida el arreglo; se apunta para cuando se toque ese pack.

### 🔴 N-64 · CORREGIDO EL MISMO DIA — media entrada de N-64 era FALSA, y la culpa fue del buscador

**Lo que se afirmo abajo sobre `PB2` es falso, y se deja escrito en vez de borrarlo.** La causa es
exactamente la regla del instrumento: **habia DOS esquematicos en el repositorio y se midio el que
no era.**

| fichero | tamano | que tiene |
|---|---|---|
| `03_Hardware_Tarjeta/KiCad/*.kicad_sch` | 451 KB | **incompleto**: sin LCD, sin botones, sin el canal del motor. Era al que apuntaba `ESTADO.md`. **BORRADO el 27/08** — queda en el historial de git, que es donde tiene que estar una copia obsoleta |
| `01_Firmware/Controladora_Semaforos/.../*.kicad_sch` | **649 KB** | **el bueno**: 30 redes, `Boton1..4`, la LCD entera, y `Motor` |
| `03_Hardware_Tarjeta/KiCad/*.kicad_pcb` | **78 bytes** | vacio |
| `01_Firmware/Controladora_Semaforos/.../*.kicad_pcb` | 2,1 MB | el trazado real |

**Lo medido sobre el esquematico BUENO, que es lo que vale:**

| pin | red | etapa | bornera |
|---|---|---|---|
| `PA0..PA7` | `S1..S8` | opto `TLP127` + MOSFET `IRLZ44N` x8 | `J3..J9`, `J11` |
| **`PB1`** | `Buzzer` | opto `U13` + MOSFET `Q8` | **`J13`** |
| **`PB2`** | **`Motor`** | opto **`U15`** + MOSFET **`Q10`** | **`J15`** |
| `PB0` | `Puerta` | `R64` 10k + `C25` 100nF -> **entrada con antirrebote de 1 ms** | `J14` |
| `PB8` | `PB8` | `R16` 1k -> LED `D5` | — |
| `PB3..PB7` | `SCL`,`CS`,`SI`,`RS(A0)`,`RST` | pantalla ST7920 | conector LCD |
| `PB9`,`PB13..PB15` | `Boton1..4` | botonera | — |
| `PA11`,`PA12`,`PA15`,`PC13` | **sin cable** | — | libres |

**Son DIEZ MOSFET y DIEZ optos (`Q1..Q10`, `U6..U15`), no nueve.** Los diez canales, cada uno
`R 10K + R 220 -> opto TLP127 -> MOSFET IRLZ44N -> bornera`, sacados del plano y no de la memoria:

| red | pin | opto | MOSFET | bornera | |
|---|---|---|---|---|---|
| `S1` | `PA0` | `U6` | `Q1` | `J3` | 🔴 rojo 1 |
| `S2` | `PA1` | `U7` | `Q2` | `J4` | 🟡 ambar 1 |
| `S3` | `PA2` | `U8` | `Q3` | `J5` | 🟢 verde 1 |
| `S4` | `PA3` | `U9` | `Q4` | `J6` | 🔴 rojo 2 |
| `S5` | `PA4` | `U10` | `Q5` | `J7` | 🟡 ambar 2 |
| `S6` | `PA5` | `U11` | `Q6` | `J8` | 🟢 verde 2 |
| `S7` | `PA6` | `U14` | `Q9` | `J11` | 🚶 rojo peatonal |
| `S8` | `PA7` | `U12` | `Q7` | `J9` | 🚶 verde peatonal |
| `Buzzer` | `PB1` | `U13` | `Q8` | `J13` | 🔊 zumbador |
| **`Motor`** | **`PB2`** | **`U15`** | **`Q10`** | **`J15`** | 🚧 **talanquera** |

De donde se sigue:

1. ✅ **La talanquera YA esta en el pin correcto.** `MOTOR_TALANQUERA = PB2` de `pines.h` es exacto, y
   la implementacion de SFTY-28 no hay que mover a ningun sitio. **Y `J15` SI existe** — la nota que
   decia *"la bornera J15 NO existe, el esquematico tiene J1..J14 y J16"* salio del fichero
   incompleto y queda **refutada**.
2. ✅ **El buzzer conserva su canal** (`PB1` -> `J13`). No hay que sacrificarlo: no eran 10 demandas
   para 9 drivers, eran 10 para 10.
3. ✅ **`Puerta`/`J14` es una ENTRADA con antirrebote por hardware**, que es justo donde el firmware
   lee la camara de demanda.
4. 🟡 **La segunda camara sigue sin entrada fisica.** `PB8` alimenta un LED, y los cuatro pines sin
   cable (`PA11`, `PA12`, `PA15`, `PC13`) no tienen bornera. Un hilo, no un chip.

**Lo unico que sobrevive de la version equivocada, y sigue siendo cierto:** el bus I2C **no puede ir
por `PB0`** —`C25` son 100 nF sobre esa linea, 250 veces el limite del I2C—, y por `PB8` tampoco sin
retirar `R16`/`D5`.

> **La leccion, que es la de siempre:** *"un `no aparece` no es un hallazgo hasta haber descartado al
> buscador"*. Aqui el buscador leia un fichero de 451 KB creyendo que era el diseno, y `ESTADO.md`
> le daba la razon apuntando a esa carpeta. **Dos copias de un plano son peores que ninguna.**

### 🔴 N-64 (version original, con la parte de `PB2` YA REFUTADA arriba) — El bus I2C de la placa hija no puede existir en PB0

**Como se midio:** resolviendo los NUMEROS DE PIN del simbolo del STM32 en el `.kicad_sch` -no por
proximidad de etiquetas- y cruzandolos con las redes trazadas desde los cables. Es la diferencia
entre *"la etiqueta esta cerca de ese pin"* y *"ese pin es el 18"*.

| pin | lo que dice `pines.h` | lo que dice la PLACA |
|---|---|---|
| **18 = `PB0`** | `CAM_DEMANDA_PIN`, entrada de camara | red **`Puerta`** -> `R64` 10 kOhm + **`C25` 100 nF** -> bornera **`J14`** (con 3,3 V al lado) |
| 19 = `PB1` | `BUZZER`, **0 usos** | `Buzzer` -> opto `U13` -> MOSFET `Q8` -> bornera **`J13`** con 12 V. **Canal de potencia completo** |
| **20 = `PB2`** | `MOTOR_TALANQUERA` | `R3` 10 kOhm a masa y nada mas: es **`BOOT1`**. **No sale de la placa** |
| 45 = `PB8` | `CAM_UMBRAL_PIN` | `R16` 1 kOhm -> **LED `D5`**: indicador, no entrada |
| `PB3..PB7`, `PB9`, `PB13..PB15`, `PA15`, `PC13` | LCD, botones, JTAG | **sin un solo cable dibujado** en el esquematico |

### 🔴 Lo que mata el diseno de la placa hija tal y como estaba dibujado

`C25` son **100 nF colgados de `PB0`**, y esa es la linea que el Manual 13 proponia como `SDA`.

```
   I2C admite ~400 pF de capacidad TOTAL de bus.  C25 = 100.000 pF.
   -> 250 veces por encima del limite. Con 4,7 kOhm de pull-up el flanco
      de subida tarda ~0,5 ms; un bit a 100 kHz dura 10 us.
   NO ES UN BUS LENTO: NO ARRANCA.
```

Y `PB8`, el `SCL` propuesto, lleva `R16` 1 kOhm a un LED: carga y fija el nivel de la linea.

**O sea que la placa hija, dibujada asi, no habria funcionado nunca** — y el sintoma habria sido
*"el DS3231 no responde"*, que se busca durante horas en el software.

**Pero ese mismo `C25` es una buena noticia para lo que SI hay que hacer:** `R64` 10 kOhm + `C25`
100 nF es un **antirrebote por hardware** de 1 ms. `J14`/`PB0` estaba pensada como **ENTRADA de
contacto seco**, que es exactamente lo que la camara necesita y lo que el firmware ya hace.

### ✅ La asignacion que sale de la medida, y que no necesita ningun chip

| funcion | por donde | por que |
|---|---|---|
| 📷 camara de demanda | **`J14`** (`PB0`) | bornera al exterior **con antirrebote RC ya montado** |
| 🚧 talanquera | **`J13`** (`PB1`) | opto + MOSFET + 12 V conmutados: etapa de potencia **ya fabricada**, hoy sin usar |

**Coste:** se pierde el zumbador -0 usos en el firmware, 0 menciones en los manuales salvo el 13-.
**Se ahorra** ademas el modulo de rele externo: `J13` ya conmuta 12 V.

**Lo que queda pendiente y NO se puede resolver leyendo:** la talanquera esta escrita hoy en `PB2`
(N-63), que no sale de la placa. Mover el `#define` a `PB1` es una linea, pero antes toca `B3`: pito
del multimetro sobre `J13` y `J14`. El esquematico dice lo que se dibujo; la LCD y los botones
demuestran que **lo que se fabrico tiene mas cobre que el dibujo**.

### ✅ N-63 — La placa de expansión se recortó a la mitad al cruzarla con el cobre

**De dónde salió:** de una pregunta de diseño —*"cómo resolvemos la entrada de 2 cámaras, la salida de
2 barreras y el reloj"*— y de responderla **con el esquemático KiCad y `pines.h` delante** en vez de
con el manual de la placa hija.

**Lo medido, en dos censos que se cruzan:**

| nets propios del esquemático | usos reales en los `.cpp` |
|---|---|
| `S1`…`S8`, **`Puerta`**, `Buzzer`, **`PB8`**, `PA8/9/10`, `PB10/11/12` | `PB0` **5** · `PB8` **2, las dos `pinMode`** · `PB2` **0** · `PB1` **0** |

**Las tres conclusiones, y por qué recortan la compra:**

1. **Las talanqueras no necesitan expansor.** Son **una por poste** —Barrera 1 en el Maestro, Barrera 2
   en el Esclavo, cada una junto a su semáforo—, no dos por tarjeta. Y la salida existe: el net
   `Puerta`, con su etapa de potencia y su bornera. Lo que falta es firmware.
2. **Las cámaras tampoco.** `PB0` ya se lee con antirrebote y `PB8` está ruteado. Lo que falta ahí no es
   un chip: es el **comando de radio** que lleve la cuenta del tramo al Maestro, que es quien decide —el
   mismo motivo por el que el umbral se retiró en N-59—.
3. **El reloj es el único que necesita bus**, y solo si el cristal muerto está en el **Maestro**: si es el
   del Esclavo, ya toma la hora por radio (SFTY-23) y no hay nada que comprar.

**Y el bus no puede ser I²C por hardware.** `PB0` no tiene ninguna función alternativa de I²C
(`ADC12_IN8`/`TIM3_CH3`) y `PB8` solo sería `I2C1_SCL` con el remap completo, que pone `SDA` en `PB9`
—el Botón 1—. Los dos periféricos por hardware están copados desde hace tiempo (`PB6`/`PB7` la
pantalla, `PB10`/`PB11` la radio), como ya decía `reloj.h`. O sea: **bit-bang**, con presupuesto de
flash contado —quedan **4.728 bytes** en el Maestro— y con *timeout* no bloqueante, porque un bus
colgado dentro del bucle de luces es peor que no tener reloj.

**Tres riesgos que se escribieron antes de que alguien compre o suelde:**

- 🔴 **El módulo de relé mata al expansor si se alimenta mal.** Con `VCC` a 12 V, el pin `IN` queda a
  12 V a través del LED del opto, y el `PCF8574` está a 3,3 V con un *pull-up* de 100 µA: la corriente
  entra por sus diodos de protección. Va con el **jumper `JD-VCC` retirado**, lógica a 3,3 V y bobina a
  12 V —o un `2N2222` de por medio—.
- 🔴 **La talanquera no se manda desde donde se cablea.** Es una salida vial: va **detrás** de la barrera
  de `semaforo.cpp`, nunca dentro de `escribirPines()`. Hoy esa función son seis `digitalWrite` que no
  pueden bloquearse; una escritura I²C sí, y dejaría las luces esperando.
- 🟡 **`PCF8574A` no vale**: misma patilla, dirección base `0x38` en vez de `0x20`.

**Pila estandarizada: `LIR2032` recargable con el circuito de carga INTACTO.** La tarjeta pasa semanas
alimentada de la batería de 12 V, así que la pila se mantiene sola y nadie sube a cambiarla. Una
`CR2032` sirve, pero entonces `D1`/`R1` se desueldan **sí o sí**: `CR2032` + circuito de carga es cargar
una celda primaria. Las dos cosas van en la **misma línea de compra** para que nadie lea una sin la otra.

**Lo que queda por medir en banco, y no se supone:** que el net `Puerta` sale del pin que `pines.h`
llama `MOTOR_TALANQUERA` —el fuente lo deja escrito como *«bornera POR CONFIRMAR»* desde el primer
día—, y que `PA15` es utilizable (el core desactiva el JTAG al configurar `PB3`/`PB4` para la pantalla,
pero eso se comprueba con un parpadeo).

### 🧹 Limpieza de ramas del 27/08 — medida, no por intuición

Se contaron los commits exclusivos de cada rama contra la activa antes de tocar nada:

| rama | commits solo suyos | qué se hizo |
|---|---|---|
| `feat/n15-reloj-pantalla-hora` | **0** | borrada (local y `origin`) |
| `fix/n51-checksum-respaldo` · `fix/n52-arnes-mando` | **0** | borradas; sus *worktrees* retirados |
| `feat/n50-pantalla-sync-vencida` | 2, **pero su contenido ya estaba** (`lcd.cpp` byte a byte igual) y su árbol iba 533 líneas por detrás | archivada en el tag `archivo/n50-pantalla-sync-vencida` y borrada |
| 3 ramas `worktree-agent-*` | **0** | borradas |
| `antes-de-limpiar-historial` | 27 | **se queda**: es el respaldo pre-limpieza |

**No hacía falta ningún tag nuevo para la versión de campo: `V8.4` ya apuntaba a `e303485`.** Se creó
uno y se retiró al comprobarlo — dos nombres para el mismo commit es cómo se pierde el que vale.

> ⚠️ **Y queda dicho, porque es lo que más engaña:** `main` **no es lo que corre en la calle**. Lleva
> **240 líneas de firmware** por encima del `V8.4` de campo (`reloj.cpp`, `modo_alcance.cpp`,
> `coordinador.cpp`, `protocolo.cpp`, repetidor) y **no tiene ni `CLAUDE.md` ni `compuerta.py`**: es
> anterior a todo el aparato de verificación, así que hoy no hay forma de comprobar nada sobre ella.

### 🔴 N-62 — Los documentos decían "medido" encima de cifras que nadie volvió a medir

**La pasada:** cruzar lo que README, `ESTADO.md`, `OPTIMIZACIONES.md` y el Manual 10 **afirman haber
medido** contra la medida. No una lectura: un censo, como el que encontró `CAM_UMBRAL_PIN` y la
divergencia de SFTY-2. Salieron **diez defectos**, y ninguno se había preguntado.

**Lo que se midió, con el acta delante:**

| dónde | decía | medía |
|---|---|---|
| `README.md` (*"cifras copiadas del acta 2026-08-26"*) | 32 rutas · 86,4 % de flash | **38 rutas · 92,8 %** — en la propia acta que citaba |
| `README.md` §packs | *"32 rutas, ninguna escrita a mano"* | 38 |
| `OPTIMIZACIONES.md` (*"se levanta buscando la etiqueta"*) | `SFTY-2` → 1 pack | **3 packs etiquetados** |
| `ESTADO.md` cabecera | HEAD `63c3be2`, rama `n15-reloj` | `4f272be`, `feat/app-bluetooth-spp` |
| `ESTADO.md` fila BANCO-PACKS | *"155/155, 20 packs"* | 197/197 en 24, doce líneas más arriba |
| `ESTADO.md` §3 | secuencias `A·B·A`/`B·A·B` como *"Solución V9.0"* | el firmware sigue en `A·A·A`/`B·B·B` |
| `ESTADO.md` / `README.md` | *"78 pruebas"*, *"10 manuales"* | **80** y **14** |
| Manual 10 §4.2 | trama sin `SERIE` | el firmware lo emite desde `f7d613f` |
| Manual 10 §4.2 y §4.3 | checksums `*4F` y `*3B` | **`*42` y `*43`** |
| `app.js` | leía `SITE` y `PAIR`; partía cada campo por todos los `:` | nadie emite esos campos; `HORA:18:25:00` entraba como `18` |

**Dos de ellos son la misma enfermedad que N-51, y conviene decirlo así:**

- `simulador_app_bluetooth.py` **existía y no estaba en la compuerta**. La lista de la sesión de banco
  lo daba por hecho —*"5/5 PASS"*— y el acta no lo echaba de menos porque no lo conocía.
- Su prueba 2 contaba rechazos de PIN, imprimía *"100% efectividad"* y **no comprobaba ninguno**: con
  la barrera rota habría impreso `0/50000 … 100%` y la suite habría seguido en 5/5. Arreglada con un
  `assert` y un denominador honesto, **se vio caer con 49.996 intentos colados**.

**Por qué duele más aquí que en otro repositorio:** este proyecto ya sabe que *"lo que TÚ reportas
también es un instrumento"*. La regla estaba escrita en `CLAUDE.md` desde el principio —*"las cifras
del README se copian del acta, nunca se escriben a mano"*— y **no tenía instrumento**. Una regla sin
instrumento es una intención: dura hasta el primer día con prisa.

**Lo que queda, que es lo único que impide que vuelva:** tres packs, 46 comprobaciones, 6 controles
negativos, **nacidos en rojo**.

| pack | qué exige |
|---|---|
| `documentos_01_cifras_del_acta` | toda cifra publicada es la del acta **más reciente**, el acta citada es esa, y no hay un segundo recuento arrastrado en otro párrafo |
| `documentos_02_trazabilidad_sfty` | la tercera columna de la tabla SFTY sale del `grep` de `# EJERCE`, **en las dos direcciones**: ni etiqueta sin fila ni fila citando un pack que no la declare |
| `documentos_03_trama_status` | `$STATUS` dice lo mismo en las dos puntas, en el Manual 10 y en el parser de la app; los ejemplos del manual cuadran con su propio XOR; el PIN que ofrece la app lo acepta el firmware; y las tres copias de `app.js` son **una sola** |

> **El detalle que más vale del pack 3:** la primera versión de la comprobación del `:` contaba los
> dos puntos en la **plantilla** del C++ (`HORA:%s`), donde siempre hay uno. Habría dado `PASS`
> eternamente sin poder fallar nunca — la nota disfrazada de prueba de `CLAUDE.md §3`. Se corrigió a
> contar sobre el **ejemplo con valores** del manual, y entonces sí cayó.

**Lo que este hallazgo NO es:** ninguno de los diez enciende una luz indebida. Lo que rompen es la
cadena de confianza —el auditor que re-corre el acta equivocada, el que escribe un parser contra un
ejemplo con checksum falso, el técnico al que el manual le manda elegir un PIN que el firmware
rechaza siempre—. Y esa cadena es lo único que sostiene un `0` de la compuerta.

### 🔴 N-54 — El paquete decía "certificado" sobre algo que ningún PC puede certificar

Cuatro de las funciones de arriba **no se pueden validar sin hardware delante**, y el acta de
la compuerta no lo decía:

| Función | Por qué el PC no alcanza |
|---|---|
| Cámaras en `PB0`/`PB8` | Nadie ha cableado nunca un contacto seco a esos pines |
| `PA8` en HIGH | Cambia el estado eléctrico del `MAX3485 U3`. Es hardware, no lógica |
| `CMD_DEMANDA` | Jamás ha cruzado una radio real |
| Bluetooth en `USART1` | Comparte pista física con el transceptor `U3` |

**Lo que se corrigió:** el `LEEME` abre ahora con `🛑 NO PROBADO EN BANCO — NO APTO PARA CAMPO`
y la tabla de arriba; el acta deja de llamarse certificado de funcionamiento y pide comparar su
`HEAD` con el commit del paquete; y `ESTADO.md` deja de decir *"Certificada al 100%"*.

**La lección, que es la de siempre en este repositorio con una cara nueva:** ya sabíamos que un
`0` de la compuerta se confunde con un permiso. Lo que no estaba escrito es que **el sobre
también miente**. `ABORTADO no es PASS`, `FALLA no es PASS` — y ahora: **`compila` no es
`funciona`, y `verde` no es `entregable`.**

### 🔴 N-55 — El banco midió lo mismo antes y después de tres funcionalidades

```
   acta 05/08 :  banco por packs   155/155  |  20 packs
   acta 26/08 :  banco por packs   155/155  |  20 packs
   en medio   :  Bluetooth completo, 2 entradas de camara, 2 comandos RF, modo_inteligente reescrito
   comprobaciones nuevas: CERO
```

`costura_03_comandos.py` —el pack que censa los `CMD_*`— no se toca desde `a122fbf`, de la Fase 2,
y ya hay dos comandos que no cuenta. No existe pack de cámaras ni de protocolo Bluetooth.

**Lo delató la comparación de totales entre actas**, que es la tercera vez que salva una migración
en este proyecto. Y lo que enseña es el corolario del §3: *un instrumento que no está en la
compuerta no mide nada* tiene un hermano peor — **un instrumento que sigue ahí pero que no creció
con el firmware sigue dando verde, y su cifra idéntica es la única señal.**

> **ABIERTO.** Es la condición previa a la sesión de banco: `costura_03` cuadrando `0x11`/`0x12`,
> pack de cámaras (`PB0`/`PB8`) y pack de protocolo BT — cada uno roto a propósito antes de
> conectarlo, según §8.bis.

### ✅ N-57 resuelto por diseño — bus I²C, y la placa decide sola

**No se elige entre reloj y cámaras: son las dos.** `PB0`/`PB8` dejan de ser dos entradas sueltas y
pasan a ser **un bus I²C** del que cuelga una placa hija: `PCF8574` (8 entradas, **siempre montado**)
y `DS3231` (**solo donde el cristal esté muerto**).

**Un solo firmware para los dos casos.** Al arrancar escanea el bus: `0x68` presente → reloj externo;
`0x20` presente → cámaras por el expansor. Si a una tarjeta le falla el cristal más adelante, **se le
enchufa el módulo y arranca usándolo, sin recompilar**.

**Y avisa, que es la mitad importante.** `N-12` dejó escrito en este repositorio que *un respaldo
silencioso derrota el propósito*: la fuente de reloj se anuncia en el LCD (`RELOJ: DS3231` /
`INTERNO` / `SIN FUENTE`), en la telemetría (`CLK:EXT`/`INT`/`NONE`) y con un `$ALARM` cuando el
cristal interno **acepta la hora y no avanza**. Con eso **el censo de cristales muertos se construye
solo**, en uso normal — que era lo que bloqueaba la decisión de compra.

Detalle en `OPTIMIZACIONES.md` § **SFTY-26**. Montaje, plan de pruebas y listado de compras en
`05_Funcional/13_Manual_Modulo_Expansion_I2C_y_Compras.md`.

### 🔒 La app móvil queda congelada: Bluetooth Clásico SPP

El Manual 10 §1 pasa de *"funciona con todos los módulos del mercado (SPP y BLE)"* a **una sola
opción, con las alternativas prohibidas por escrito y con su motivo**. Una spec que admite las dos es
la que hace que mañana alguien elija la otra.

`navigator.bluetooth` **solo habla BLE**: no es que fallara con un HC-05, es que la API no existe
para SPP. Ésa era la causa de *"no abre el Bluetooth, no se conecta"*. Se copia el bloque de
`MainActivity2.java` de Baliza —`getBondedDevices()`, UUID `00001101-…`, RFCOMM con reintento
inseguro—, que lleva meses funcionando en la calle. Y quedan prohibidas: Web Bluetooth, módulos BLE,
**reconexión automática** y conexiones simultáneas.

### ✅ N-60 — La identidad sale del silicio, y el pack corrigió el diseño dos veces

`identidad.h`/`.cpp`, **contrato compartido** (los dos ficheros idénticos byte a byte, vigilados por
`costura_01`, que pasa de 5 a 7 contratos). La serie de cada equipo se deriva del **UID de 96 bits
que el STM32 trae grabado de fábrica** (`UID_BASE`, `0x1FFFF7E8`, confirmado en el CMSIS instalado,
no supuesto). Ya viaja en `$STATUS` como `SERIE:`.

Se descartó acuñarla con fecha y hora al arrancar, que era la idea natural: en un equipo recién
salido de taller `reloj_enHora()` es **falso**, así que **todas las unidades nacerían con el mismo
sello**.

> **El pack `identidad_01_serie` encontró dos defectos, los dos míos, y ninguno lo habría visto una
> revisión a ojo.**
>
> **1. La serie era demasiado estrecha.** Con 16 bits el barrido midió **88 colisiones** entre 4.096
> chips vecinos de una misma oblea. La tentación era relajar el barrido; la cuenta dijo otra cosa:
> la cota del cumpleaños para 4.096 códigos en 65.536 huecos son **128**. El mezclado iba **mejor que
> el azar** — lo estrecho era el **ancho**, no la función. Y como ninguna función puede batir esa
> cota, exigir cero colisiones a 16 bits habría sido *una comprobación que ningún firmware puede
> aprobar*, que es exactamente lo que `§3` prohíbe. Se amplió a **24 bits** y la comprobación se
> reformuló a lo que de verdad se quería medir: **que el mezclado no sea PEOR que el azar**.
>
> **2. El mezclado copiaba la mitad alta en vez de repartirla.** La avalancha medía **5,8 de 24 bits**
> cuando el ideal son 12. La causa: la multiplicación solo propaga acarreo **hacia arriba**, así que
> un cambio en el bit 31 de la entrada no llega nunca a los bajos. Un `h ^= h >> 15` dentro del bucle
> lo pliega: **5,78 → 11,84**. Sin esa medida, las series se habrían agrupado por lote de fabricación
> — justo entre los equipos que se compran juntos y acaban en la misma carretera.
>
> **Y el control negativo del propio pack salió ROTO la primera vez**, lo que también quedó anotado
> dentro: se usó un XOR plano como "mezclado malo", pero en un barrido donde solo varía la palabra 0
> un XOR plano sigue siendo inyectivo. Estaba demostrando que la prueba caza un caso que no era malo.
> Se cambió por un mezclado que **pierde información de verdad**.

Las tres constantes —multiplicador, ancho y desplazamiento— **se leen del C++ en cada corrida**. Un
espejo Python que se desincronice mide una función que no es la que corre en el micro; de hecho la
primera versión del espejo olvidó el `xorshift` y el pack lo delató al instante.

```
   packs   22 -> 23      comprobaciones   174 -> 184
   rutas   34 -> 38      Maestro flash  92.6% -> 92.8%
```

### ✅ N-59 — El umbral de tramo se retira de V9.0, y se deja un cable trampa

`CAM_UMBRAL_PIN` (`PB8`) se declaraba, se ponía en `INPUT_PULLUP` y **no se leía nunca**, mientras
cuatro documentos afirmaban que las cámaras 2 y 4 contaban entradas y salidas del tramo.

**No se implementa, y el motivo es de diseño, no de tiempo:** el conteo necesita que las dos puntas
se pasen la cuenta por radio, y **ninguno de los 18 comandos `CMD_*` sirve para eso**. Leer el pin
sin poder mandar la cuenta al Maestro —que es quien decide— daría un dato que no llega a nadie. Medio
camino aquí es peor que ninguno: dejaría un manual que promete conteo y un firmware que no lo hace,
que es exactamente el defecto que se está cerrando.

**Lo que sí regula el paso alternado en V9.0** son las cámaras 1 y 3 (demanda, `PB0`) y el todo-rojo
temporizado íntegro, que es lo que de verdad vacía la vía.

> 🪤 **El cable trampa.** El pack `camara_01_demanda` comprueba que **nadie lee `PB8`** — y esa
> comprobación **existe para caerse**. El día que alguien implemente el umbral, el banco falla con un
> mensaje que dice qué hacer: actualizar los Manuales 1, 2 y 9 y retirar la comprobación. Es la
> `§8.quater` del revés: en vez de una prueba que celebra un defecto y hay que invertir al arreglarlo,
> es una prueba que **sujeta una ausencia** y hay que retirar cuando se llene.
>
> Sin ese cable, la próxima vez el firmware se adelantaría a los manuales igual que esta vez los
> manuales se adelantaron al firmware.

**Se abre la familia `camara_*`**, que la matriz de cobertura tenía vacía: hasta hoy las cámaras eran
la única entrada del firmware sin una sola comprobación detrás. El pack mide además que la demanda se
tome **por flanco** (el relé cierra ~1 s: por nivel serían cientos de peticiones), que la ventana de
silencio **se lea del C++** y supere el pulso del relé, y que la **primera** demanda tras el arranque
no se pierda por el `millis()` en cero.

Visto caer con dos defectos distintos inyectados en el `.cpp` real —una lectura de `PB8`, y la
ventana bajada a 800 ms—, `9/9 → 8/9` en ambos casos. Restaurado y verificado por `diff`.

```
   packs   21 -> 22        comprobaciones   165 -> 174
```

### ✅ N-58 — El Esclavo pide en vez de ordenar, y el banco cazó dónde no iba la puerta

**El riesgo que se cierra:** `semaforo_iniciarTestLeds()` encendía 6 s de secuencia —rojo, ámbar y
**verde**— sin mirar nada, colgado del Bluetooth del Esclavo. Cualquiera con un móvil a 15 m sacaba
un verde en una punta mientras la otra también lo tenía. Y **conectarse al Esclavo correcto era
igual de peligroso**: el fallo no era equivocarse de poste, era que esa punta aceptase mover luces.

**Lo que queda:** el Esclavo puede **pedir** (`SOLICITAR_PASO` → `CMD_DEMANDA`, el Maestro decide) y
puede **parar** (`FORZAR_ROJO`, sin PIN). **No puede abrir.** Con eso el funcional del PMT trabaja
desde cualquier extremo sin saber cuál es el Maestro.

> 🟢 **Y aquí el banco hizo exactamente su trabajo, contra mí.** La puerta única de la demanda se
> puso primero en `protocolo.cpp` — y `costura_01_contratos` saltó: *"DIVERGEN entre proyectos:
> `include/protocolo.h`, `src/protocolo.cpp`"*. Tenía razón. **`protocolo.*` es contrato compartido y
> debe ser idéntico byte a byte en las dos puntas**, porque el formato de aire lo acuerdan los dos
> extremos; la ventana de silencio con la que **esta** punta decide cuándo pedir es política local, y
> el Maestro no la necesita. Se movió a `Esclavo/demanda.cpp`, y el propio pack nuevo vigila ahora que
> no vuelva.
>
> Es la tercera vez en el proyecto que una comprobación de identidad entre puntas evita una deriva —
> y la primera en que el defecto lo había metido quien escribía la prueba.

**Cifras que se movieron, que es la señal de que el instrumento creció con el firmware:**

```
   packs           20  ->  21          comprobaciones   155  ->  165
   guarda rutas    32  ->  34          Esclavo flash  71.0% -> 71.3%
```

Pack **`esclavo_06_no_abre_paso`**: lista blanca de comandos escrita a mano —a propósito, para que un
comando nuevo obligue a pasar por el fichero y justificarse—, más dos controles negativos. **Visto
caer a `7/9` con `semaforo_iniciarTestLeds()` reinyectado en el `.cpp` real**, con el código de salida
pasando de `0` a `1`; firmware restaurado y verificado por `diff`, no por impresión (§8.bis).

### 🔴 N-61 — La barrera SFTY-2 no era la misma en las dos puntas

Encontrado al ir a construir `barrera_02_dos_puntas`, comparando los dos `semaforo.cpp`. En el
**Esclavo**, dentro del enclavamiento:

```c
  if (rojo) {
    verde = false;
    amarillo = false; // Opcional: forzar amarillo apagado si rojo esta encendido...
    // Wait, in S_ROJO_AMARILLO state, the code explicitly passes (HIGH, HIGH, LOW).
    // So if rojo is HIGH, amarillo CAN be HIGH. But verde MUST be LOW.
  } else if (verde) {
    rojo = false;
    // If verde is HIGH, amarillo usually is LOW, but let's just force rojo LOW.
  }
```

**Tres cosas a la vez, y ninguna la habría visto una lectura por encima:**

1. **`S_ROJO_AMARILLO` no existe.** Los estados son `S_ROJO`, `S_VERDE`, `S_AMARILLO` y `S_FALLO`.
   El comentario **delibera sobre un estado inventado** y, sobre esa premisa falsa, añade una
   sentencia viva al enclavamiento. Es deliberación de un modelo dejada en el fuente, en inglés,
   dentro de una regla de seguridad.
2. **Hoy no cambiaba el comportamiento**: ninguna de las 9 llamadas pasa rojo y ámbar a la vez. Era
   **código muerto dentro de SFTY-2** — no falla, no se nota, y nadie lo mide.
3. **Era una trampa a plazo.** El día que alguien añada una transición rojo+ámbar —práctica corriente,
   y bien puede pedirla el auditor— el Maestro la mostraría y el Esclavo no. En silencio.

**Arreglado devolviendo el Esclavo a paridad.** Comparados sin comentarios, los dos ficheros quedan
con el **mismo cuerpo**: solo difieren en dos comentarios adaptados al contexto y la posición de una
función.

> **Y esto reencuadra lo que tenía que ser `barrera_02`.** Se planteó como un arnés que compilase los
> dos firmwares a la vez para medir «verde simultáneo» — caro y con colisión de símbolos. Pero el
> verde simultáneo no nace de la nada: **nace de que las dos puntas se comporten distinto**. Comparar
> el enclavamiento entre puntas cuesta un pack y habría cazado este defecto el día que se introdujo.
> El arnés de dos firmwares sigue teniendo sentido para el apretón de manos por radio, pero ya no es
> lo primero.

`barrera_02_dos_puntas`: **13 comprobaciones**, 2 controles negativos. Compara `aplicarSalidas()` y
`escribirPines()` **por código sin comentarios** —la identidad byte a byte sería demasiado estricta:
los comentarios divergen a propósito—, exige que las 9 combinaciones pedidas sean las mismas, que
ninguna pida rojo y verde, y que el enclavamiento siga haciendo lo que dice. Visto caer a **12/13**
con el defecto real reinyectado en el `.cpp`, con el código de salida pasando de `0` a `1`.

### 🚧 SFTY-28 — La talanquera, anotada antes de que se pierda

`MOTOR_TALANQUERA` (`PB2`) está declarado en las dos puntas, tiene MOSFET y bornera `J15`, y **el
firmware no lo escribe nunca**. La regla queda escrita en `OPTIMIZACIONES.md`: **la talanquera sigue
al semáforo, nunca lo manda y nunca lo contradice**, y su orden sale de `semaforo.cpp` como las
luces — `§6` extendida.

**Y al medir dónde se acciona aparecieron dos cosas:** hay **9 MOSFET para 8 luces**, así que queda
**un solo driver libre** — y lo reclaman **dos** funciones, el buzzer (`PB1`) y la talanquera (`PB2`):
10 demandas para 9 etapas de potencia. Además `pines.h` mandaba a cablear a la bornera **`J15`, que
no existe** (el esquemático tiene `J1`..`J14` y `J16`); corregido en las dos puntas. Y un MOSFET
conmuta encendido/apagado, no dos sentidos: la única opción que cabe en la placa actual —y la
correcta— es una **barrera con controlador propio que acepte contacto seco**, dejando el motor y la
seguridad antiaplastamiento a quien está diseñado para eso.

Con las tres preguntas que hay que cerrar antes de escribir código: qué hace al **perder el enlace**
(el ámbar intermitente significa «pasa con precaución», y ahí ni bajada ni subida son obviamente
correctas), qué hace **al cortarse la energía** —que no se resuelve en software sino eligiendo un
actuador que caiga por gravedad—, y si habrá **final de carrera**, porque sin él el firmware no puede
saber si la barrera bajó de verdad y **no debe fingir que sí**.

### 📍 Dónde está V9.0 esta noche, y qué falta

**Rama viva: `feat/app-bluetooth-spp`** (desciende de `feat/n15-reloj-pantalla-hora`, así que la
contiene entera). Compuerta **11/11 con compilación real**.

```
   Esta sesion movio el banco de:   155/155  ·  20 packs  ·  32 rutas
                                a:  184/184  ·  23 packs  ·  38 rutas
   y ahora si cubre lo que V9.0 anadio, que era el agujero de N-55.
```

#### ✅ Cerrado

| | |
|---|---|
| `N-69` | Los tiempos del ciclo se cambian por pantalla, no por app; y el umbral de 12 s esta escrito tres veces |
| `N-68` | Las cámaras 2 y 4 vuelven como **presencia** (veto con tope), no como conteo |
| `N-67` | La entrada de cámara leía al revés: `INPUT_PULLUP` contra el pull-down de la placa = demanda permanente |
| `N-66` | La app mandaba `GET_STATUS`/`GET_CONFIG` que nadie atiende, y no ofrecia `SOLICITAR_PASO` |
| `N-65` | El censo de comandos miraba un solo fichero: `CMD_DEMANDA` era invisible para `costura_03` |
| `N-64` | El bus I²C no cabe en `PB0` (100 nF de `C25`). **Y la mitad sobre `PB2` se refutó el mismo día: había dos esquemáticos y se midió el incompleto** |
| `N-63` | La placa de expansión se recortó al cruzarla con el cobre: talanqueras y cámaras ya tienen salida y pin |
| `N-62` | Los documentos decían «medido» sobre cifras viejas; tres packs `documentos_*` lo vigilan |
| `N-54` | El paquete decía «certificado» sobre lo que ningún PC puede certificar |
| `N-55` | El banco medía lo mismo antes y después de tres funcionalidades |
| `N-56` | El presupuesto de flash se consumió sin que nadie parase |
| `N-57` | `PB0`/`PB8` asignados dos veces → bus I²C, la placa decide sola (SFTY-26) |
| `N-58` | El Esclavo pide en vez de ordenar; ya no puede abrir paso |
| `N-59` | Umbral de tramo retirado de V9.0, con cable trampa que lo vigila |
| `N-60` | La identidad sale del silicio; el pack corrigió el diseño dos veces |

#### 🔜 Lo que sigue, en orden de riesgo

**1. `FW-PAIR` — completar la matrícula. Es lo único que aún es un riesgo vial abierto.**
Hoy `RF_Packet` no lleva direccionamiento, así que **dos parejas dentro del alcance de la radio —1 a
3 km— se mandan órdenes entre sí**. Falta el campo `PAIR`, el `SET_PAIR` por Bluetooth y el descarte
de lo ajeno, más el pack `costura_08_pareja`.

> ⚠️ **Es el cambio más delicado que queda, y conviene abordarlo con la cabeza fresca.** Toca `DR9`
> del respaldo, obliga a subir la `FIRMA`, y arrastra `Validacion_Respaldo` —que compila el
> `calcularSuma()` real— y las 19 comprobaciones de `maestro_02_respaldo`. Es exactamente la zona
> que ya produjo `N-49` y `N-51`.

**2. `barrera_02_dos_puntas` — la propiedad que puede matar y que nadie mide.**
`CLAUDE.md §8` ya lo dice del arnés del automático: *«solo el Maestro: verde simultáneo en las dos
puntas no se mide ahí»*. Exige un arnés que compile **los dos firmwares a la vez**. Es el pack más
caro que falta y el único que mide la propiedad que de verdad importa.

**3. `bluetooth_01_autorizacion` — que un comando sin PIN válido no abra paso.**
Y con él, el hallazgo que la revisión del simulador de la app dejó al aire: **el PIN no tiene
bloqueo ni límite de intentos**. Un atacante agota los 10.000 códigos posibles sin que nada lo frene.

**4. Flash — MEDIDO, ya no hace falta adivinar qué recortar.**
Maestro al **92.8 %**, quedan **4.728 bytes**. El `PAIR` cabe; es lo último que cabe holgado. Mapa de
símbolos del `.elf` (`arm-none-eabi-nm --size-sort`), de mayor a menor:

| Símbolo | Bytes | Comentario |
|---|---|---|
| `HAL_I2C_EV_IRQHandler` | **2.264** | 🔴 **El firmware no usa I²C.** Ni `Wire`, ni `DS3231`, ni un solo acceso. Son 2,2 KB de manejador de interrupción enlazados para nada |
| 7 fuentes de U8g2 (`ncenB14`, `ncenB12`, `ncenB10`, `7x14`, `7x14B`, `6x10`, `5x7`) | **9.483** | Siete tipografías en una pantalla monocroma de 128×64. Las dos mayores solas son **3,9 KB** |
| `__ssvfscanf_r` / `__ssvfiscanf_r` | **748** | Es `sscanf`, y entra por **una sola línea**: el parseo de `SET_RTC`. A mano cuesta ~40 bytes |
| `_printf_i` | **600** | El núcleo entero de `printf`, por los `snprintf` de la telemetría |

**Hay ~13 KB identificados sobre 4,7 KB libres**, así que el problema no es que no haya de dónde
sacar: es decidir qué se toca sin romper nada. Orden por relación ahorro/riesgo:

1. **`sscanf` → parseo a mano** (~750 B, riesgo bajo, una función)
2. **Retirar fuentes de U8g2 que no se usen** (~1-4 KB, riesgo bajo pero **lo vigila el arnés de
   pantalla con 271/271**: hay que comprobar cuáles usa de verdad `lcd.cpp`, no suponerlo)
3. **`HAL_I2C_EV_IRQHandler`** (2,2 KB, riesgo medio: hay que ver por qué lo enlaza el core)

> 🙃 **Y una ironía que conviene ver antes de tocarlo:** esos 2.264 bytes de I²C son exactamente lo
> que `SFTY-26` va a necesitar cuando entren el `DS3231` y el `PCF8574`. No están desperdiciados:
> están **pagados por adelantado**. Recortarlos hoy y volver a meterlos mañana sería trabajo doble.

**5. Banco.** Sigue siendo el bloqueante de todo: cámaras en `PB0`, `PA8` en HIGH, `CMD_DEMANDA` por
radio real y el Bluetooth compartiendo pista con `U3` **no se pueden validar en un PC**.

#### 🧹 Deuda que conviene no olvidar

- El **simulador de la app** (`simulador_app_bluetooth.py`) no está conectado a la compuerta y
  reimplementa la lógica del firmware con el PIN escrito a mano dos veces: hoy no es evidencia.
- El informe de la app declara **soporte BLE**, que la spec congelada del Manual 10 §1 prohíbe. Hay
  que resolver la contradicción antes de comprar módulos.
- **Dos APK de ~3,9 MB commiteados** y duplicados. Se guarda el generador, no el generado.
- `feat/n15-reloj-pantalla-hora` se quedó atrás en `569783b`. Decidir si se retira o se fusiona.

### 🔴🔴 N-57 — `PB0`/`PB8` están asignados dos veces, y el reloj llegó primero

**Encontrado por la primera pasada sistemática de la sesión** (`05_Funcional/12_Cobertura_de_Pruebas_y_Huecos.md`),
no preguntando. Los ocho defectos anteriores de V9.0 salieron de que alguien hiciera la pregunta
correcta; éste salió de censar el código y cruzarlo con todos los documentos. **Esa es la diferencia
entre revisar y parchear.**

**El conflicto:**

| Reclama `PB0`/`PB8` | Con qué respaldo |
|---|---|
| **`DS3231`** — N-37 | **medida de banco del 01/08/2026**, tres eliminaciones documentadas |
| **Cámaras IA** — V9.0 | decisión de diseño del 26/08/2026 |

N-37 cerró con el cristal `Y2` **muerto** y dejó escrito: *"Salida: `DS3231` por I²C software en
`PB0`/`PB8`, los únicos pines libres. Hacen falta DOS"*. **V9.0 le quitó al reloj los dos pines que
necesita**, y el firmware sigue en `STM32RTC` sobre el cristal muerto (`reloj.cpp:109`).

Sin reloj no hay Modo Degradado — `SFTY-18` lo prohíbe, y con razón.

**Y el subhallazgo que lo hace resoluble:** `CAM_UMBRAL_PIN` (`PB8`) **tiene `pinMode()` y ni un solo
`digitalRead()`** en ninguno de los dos micros. Las cámaras 2 y 4 no existen en el firmware, aunque
cuatro documentos digan que sí. La demanda real necesita **un** pin, no dos.

**Salida propuesta:** un expansor **`PCF8574`** colgado del mismo bus I²C que el `DS3231`. Deja
8 entradas digitales para las cámaras y **acaba con la disputa por pines para siempre**. A medir en
banco: que leer el expansor por I²C software no compita con el `IWDG` ni con el bit-bang del LCD.

> **Y una nota de método, que es la que más vale de esta entrada.** `MAPEO_TARJETA_KICAD.md §4`
> afirmaba que la culpa del cristal *"no está medida"*. Su cabecera lo fecha el **31 de julio**; la
> medida de banco es del **1 de agosto**. No era una contradicción: era **un documento un día viejo**
> que llevaba desde entonces contradiciendo al roadmap sin que nadie lo notara. Ya está corregido, y
> con la fecha visible para que se entienda por qué decía lo que decía.

### 🟠 N-56 — El presupuesto de flash se consumió sin que nadie parase

```
   05/08 (V8.7)                    86.4 %   56 616 B
   + Bluetooth                     89.0 %   58 304 B
   + camaras + CMD_DEMANDA         92.5 %   60 608 B
   + FORZAR_ROJO sin PIN           92.6 %   60 664 B   <- libre: 4 872 B
```

`§7` manda revisar a partir de **~2 %**. La subida de la sesión es **+6.2 %** y la revisión no se
hizo. No es urgente hoy; es urgente **antes** de la siguiente función, porque quedan menos de 5 KB.

### 🟢 El rojo de emergencia deja de pedir PIN

`bluetooth.cpp` atiende `CMD:FORZAR_ROJO` **antes** del bloque de autenticación, en los dos micros.
Se conserva además la forma con PIN, que es la que envía la app: las dos entradas hacen lo mismo.

El motivo estaba escrito desde hace meses en `mando.cpp`, para el mando de relés: *"ASIMETRÍA
DELIBERADA: lo seguro, fácil; lo peligroso, difícil"*. Detener el tráfico es la acción **segura**;
ponerle una clave delante solo retrasa a quien está viendo el incidente. El PIN guarda lo que
**abre** paso o mueve luces — `SET_MODO:*`, `TEST_LEDS`, `SET_RTC` —, no lo que las para.

### 📶 Dos reglas nuevas en `OPTIMIZACIONES.md`, ninguna implementada

- **`SFTY-24`** — enlace de respaldo por datos móviles entre dos teléfonos. Con su regla dura:
  **puede observar y documentar; no puede autorizar.** Y su criterio previo: medir la cobertura
  real en los tramos antes de escribir una línea, porque *"un canal que existe a veces es peor que
  uno que no existe nunca"*.
- **`SFTY-25`** — identidad de tramo en la telemetría. **No es una mejora: es un agujero abierto.**
  Con varios pares en una vía y un solo teléfono, las tramas de dos pares son indistinguibles —el
  rol `NODE:MAESTRO` no dice *de qué par*—, y el operario puede abrir un verde en un tramo que no
  está mirando. Hasta que exista el campo `ID:`, el manual de la app debe advertir **un solo par
  encendido a la vez**.

---

## 📦 V8.9 · Telemetría Bluetooth, App Móvil Baliza, Cámaras AcuSense y Cierre de Auditoría Hardware (26 de Agosto de 2026)

### 🎯 Estado Técnico Consolidado (Lo que está implementado y pasando compuerta)
1. **Firmware C++ (Maestro & Esclavo):**
   * Driver serie en `USART1` (`PA9` TX, `PA10` RX a 9600 bps) con telemetría periódica `$STATUS` (1s), cuenta regresiva `T:`, Caja Negra de caídas `$ALARM` y validación estricta de PIN (`1234`) para comandos.
   * **Desacoplo Hardware de U3:** `PA8` (`RS485_IN_DE_RE`) configurado en `HIGH` permanente, colocando la salida `RO` del transceptor `MAX3485 U3` en alta impedancia ($\text{Hi-Z}$) para liberar la pista de `PA10` hacia el módulo Bluetooth.
   * **Cámaras IA AcuSense en PB0/PB8:** Detección de presencia binaria con antirrebote para Cámara 1 (Demanda S1 en Maestro `PB0`) y Cámara 3 (Demanda S2 en Esclavo `PB0`), con retransmisión RF mediante `CMD_DEMANDA` (`0x11`) y `CMD_ACK_DEMANDA` (`0x12`).
   * **Barrera de Luces Intacta:** Test de focos de 6 segundos encapsulado en `semaforo.cpp` (`semaforo_iniciarTestLeds()`).
   * **Compilación PlatformIO:** Maestro (Flash: 92.5% / RAM: 20.3%), Esclavo (Flash: 70.9% / RAM: 19.5%) y Repetidor (Flash: 20.6%) compilan en `[SUCCESS]` en la compuerta.
2. **App Móvil de Campo (`05_Funcional/App_Semaforo/`):**
   * Frontend interactivo en HTML/CSS/JS con diseño oscuro Baliza IOT-VIAL, parser de telemetría NMEA `$STATUS` con `T:`, anillo de cuenta regresiva SVG, monitor de batería, PIN modal de seguridad y exportación a CSV/WhatsApp.
   * **Selector de Cruces Viales:** Un solo celular gobierna múltiples pares de semáforos en la carretera (Km 12, Km 24, etc.) con detección automática de rol (`👑 MAESTRO` vs `📡 ESCLAVO`).
   * **Modo Courier RTC (Sincronización Puente sin Radio):** Captura hora y ciclo en Maestro, cronometra el viaje en vehículo y programa el Esclavo con compensación automática ($\Delta t < 0.1\text{ s}$).

---

### 🛡️ RESOLUCIÓN DEFINITIVA DE AUDITORÍA DE HARDWARE (100% ARMONIZADO)

Toda la documentación técnica, esquemáticos y firmware han sido armonizados sin ninguna colisión física:

```text
 ┌─────────────────────────────────────────────────────────────────────────────┐
 │                    MAPA REAL DE PINES STM32F103C8T6 (PCB KICAD)             │
 ├─────────────────────────────────────────────────────────────────────────────┤
 │ • USART3 (PB10 TX / PB11 RX / PB12 DE/RE): MAX3485 U2 ➔ Radio LoRa E90-DTU. │
 │ • USART1 (PA9 TX / PA10 RX / PA8 en HIGH): Módulo Bluetooth (U3 en Hi-Z).   │
 │ • PB9, PB13, PB14, PB15: 4 BOTONES DEL PANEL LCD Y MANDO RF (Inmunes a IA). │
 │ • PC14 / PC15 + VBAT (Pin 1): Cristal nativo Y2 (32.768 kHz) + Pila CR2032.│
 │   (R5 de 0 Ω retirada obligatoriamente para evitar sobrecarga).             │
 │ • PB0 / PB8: CÁMARAS IA ACUSENSE (PB0 = Demanda de Cola, PB8 = Umbral).    │
 └─────────────────────────────────────────────────────────────────────────────┘
```

1. **Cámaras IA (Manual 9):** Analítica Deep Learning embebida en la propia cámara Hikvision AcuSense (Detección de Intrusión con zona ~90% y filtro `☑ Solo Vehículo`). Conexión directa por contacto seco a `PB0` y `PB8`. Cero consumo de Raspberry Pi / Jetson en el remolque.
2. **Telemetría Bluetooth (Manual 10):** Asignada a `USART1` (`PA9`/`PA10`) con protocolo NMEA estricto, Checksum XOR (`*XX`), emitiendo `T:` y comandos con validación de PIN (`1234`).
3. **Pila RTC y Reloj (Manual 11):** Cristal nativo $Y_2$ diagnosticado con `CONSULTA RELOJ` (`Oscilando OK / En hora`) y pila CR2032 en `VBAT` con $R_5$ retirada. Módulo DS3231 preservado como contingencia de taller.

---

### 🧭 PUNTO DE RETORNO Y DECISIONES PARA REANUDAR:

1. **Decisión de asignación de `PA9`/`PA10`:**
   * **Opción A (Recomendada):** `PA9`/`PA10` (`RS485_IN`) asignado a Telemetría Bluetooth / Diagnóstico con la App. RTC nativo con cristal `Y2` y pila en `VBAT` (retirando `R5`). Pines `PB0`/`PB8` libres.
   * **Opción B:** `PA9`/`PA10` para cámaras YOLO por RS-485 + Bluetooth por Software en `PB0`/`PB8` a 9600 bps.
2. **Reescritura y saneamiento de manuales:**
   * Actualizar [`05_Funcional/9_Manual_Parametrizacion_Camara_IA.md`](file:///d:/@Proyect/Controladora_Semaforos/05_Funcional/9_Manual_Parametrizacion_Camara_IA.md) (eliminar pines `PB9`/`PB13`).
   * Actualizar [`05_Funcional/10_Manual_Modulo_Bluetooth_Telemetria.md`](file:///d:/@Proyect/Controladora_Semaforos/05_Funcional/10_Manual_Modulo_Bluetooth_Telemetria.md) (aclarar bus `USART1` / `MAX3485 U3`).
   * Actualizar [`05_Funcional/11_Manual_Instalacion_RTC_DS3231_Bateria.md`](file:///d:/@Proyect/Controladora_Semaforos/05_Funcional/11_Manual_Instalacion_RTC_DS3231_Bateria.md) (procedimiento de desoldar `R5` y portapilas `VBAT`).
3. **Regenerar documentos Word (`05_Funcional/convertir_a_word.py`) y compuerta final.**

---

## 📦 V8.8 · La migración terminó y la compuerta dejó de mentir (5 de Agosto de 2026)

**Cerrado hoy: N-46, N-50, y N-49 completo en software.** Estado medido al cerrar la sesión:
**10 PASS · 1 FALLA · 0 ABORTADO** de 11, banco por packs `154/155`. Ese único `FALLA` es el
checksum de `maestro_02`, en curso en `fix/n51-checksum-respaldo`.

### N-46 — retirados los tres validadores monolíticos

Imprimían `FALLA` y salían con código `0`, así que la compuerta —que decide por el código de
salida— pintaba la suite en `[OK]`. Era *"ABORTADO no es PASS"* invertido: **`FALLA` contado como
`PASS`**, y uno de aquellos fallos era vial.

Ninguno se retiró por confianza. Cada uno exigió que los packs sumaran *exactamente* sus
comprobaciones **y que el texto literal de cada una coincidiera**:

| | packs | monolito | |
|---|---|---|---|
| Costura | `3+11+6+4+8+6+3 = 41` | `41/41` | ✅ |
| Maestro | `15+18+16+11+4 = 64` de `67` | `64/67` | ✅ |
| Esclavo | `7+7+5+5+7 = 31` | `31/31` | ✅ |

Cero comprobaciones huérfanas en ninguna dirección. `maestro_06` y `maestro_07` quedaron fuera de
la cuenta a propósito: documentan N-39/N-40 y el monolito no los contenía.

> **Y al retirarlos cayó la guarda de rutas — que es exactamente lo que tenía que pasar.**
> Censaba las tuplas de ruta *de los monolitos*; sin ellos bajó a 4 rutas, por debajo de su suelo
> de 20, y **abortó en vez de aprobar**: *"falló el buscador, no el árbol"*. Las rutas viven ahora
> en `banco/packs` y `banco/modelos`. Hoy censa 32, ninguna escrita a mano.

### La tercera cara del mismo error: un `FALLA` permanente tampoco es un aviso

El alias de ±60 s de `CMD_DELTA` se dejaba fallando **a propósito** para que el límite del
protocolo no se olvidara. Pero `CMD_DELTA` lleva un solo byte de segundos y ninguna aritmética
distingue 0 de 60: **esa comprobación no la podía aprobar ningún firmware posible**. Con ella
dentro, la compuerta no podía salir en verde jamás — y un código de salida que nunca cambia
enseña a ignorarlo, que es tan peligroso como uno que miente.

No se silenció, se **invirtió**: ya no exige lo imposible, exige que el agujero sea *exactamente*
el que el protocolo obliga y ni un caso más. Los 1674 que pasan son todos alias de un múltiplo de
60 s; si mañana se cuela uno que no lo es, eso sí es un defecto. Con `control_negativo` al lado
—un Esclavo que reportase siempre 0 s deja entrar no-alias, y la prueba lo distingue— y el
residual íntegro en un `reportar()`, que no cuenta. `maestro_03` pasó de `17/18` a `19/19`.

**Regla que queda en `CLAUDE.md`:** si una comprobación no la puede aprobar ningún firmware
posible, no es una comprobación — es una nota, y va en `reportar()`.

### N-50 — la pantalla del Maestro avisa `>48h`, y ahora algo lo comprueba

`lcd_dibujarDegradado()` del Esclavo recibía el indicador *"vencida"* y la del Maestro no, así que
pintaba el número que le dieran. El llamante ya se lo daba contrastado —el número no mentía—, pero
la pantalla no era una defensa.

**Lo que hace que esto cuente es la segunda mitad.** El cambio estuvo apartado en su rama marcado
`NO MERGEAR` porque el arnés solo se había *adaptado a la firma*: pasaba `casos[i].minSync >= 2880`
con un `minSync` máximo de 2759, así que el argumento era **siempre `false`** y la rama nueva no se
ejercía nunca. El delator fue la cifra: `295/295` antes y `295/295` después. **Si se añade
comportamiento y el total no se mueve, no se está midiendo.**

Ahora `271/271`, comparando byte a byte la banda del framebuffer contra una referencia dibujada
con la misma fuente, y exigiendo el texto exacto —`>48h` o `NNhNNm`—, no solo que haya tinta.
Verificado que sabe fallar: invertida la condición en `lcd.cpp` caen 7 de 271, y son exactamente
las 7 de texto exacto. Ancho **medido** con `getStrWidth()`: 69 px de 128 disponibles.

### Lo que se le mandó al funcional, y por qué son DOS paquetes

Con la compuerta en verde y el banco sin pasar, la tentación es mandar *"la entrega V8.7"*. Se
separó a propósito, porque **un paquete es una autorización implícita**: quien lo recibe asume que
puede instalarlo.

| | |
|---|---|
| `ENCARGO_Sesion_Banco_2026-08-05.zip` | **Pide una medida.** No entrega nada: pide que alguien ponga la tarjeta delante. Lleva los 16 binarios del bisect con sus MD5, los firmware marcados `SIN_VALIDAR` en el propio nombre, y el orden de las pruebas |
| `Entrega_V8.7_Firmware_y_Manuales.zip` | Fuente para PlatformIO + manuales + acta. **Sin `.bin`**: se compila del fuente, así lo que se carga se corresponde con lo que se revisa |

Tres cosas que se corrigieron antes de cerrarlos, y las tres son la misma clase de error:

- **El `LEEME` abría con *"Compuerta al 100% PASS"*.** Un lector se queda ahí y no llega a la línea
  que dice que no ha pasado banco. Ahora abre con qué corre en campo, que esto no es eso, y la
  regresión abierta — la cifra viene después.
- **Decía *"elimina las 7 transposiciones ciegas"*; el banco mide 8.** Nadie lo habría notado. Las
  cifras se copian del acta, nunca se escriben a mano.
- **Había dos encargos diciendo cosas distintas**, y el viejo mandaba tras un sospechoso que N-52
  acababa de refutar. Se archivó en `historico/` con la cabecera que explica por qué se cae, no se
  borró: una causa que desaparece en silencio vuelve a proponerse.

Y un aviso medido dentro del paquete de bisección: `2779d9b` y `831c4f0` tienen **el mismo MD5**
—cargar el segundo es una carga tirada—, mientras que `8a45ae7` y `f37581f` **pesan igual y son
ficheros distintos**. El tamaño no decide en ninguna de las dos direcciones.

El método está en la skill `entregar`.

### N-51 — la compuerta sale en verde, y el último `FALLA` escondía un camino explotable

**Cerrado. `11 PASS · 0 FALLA · 0 ABORTADO` de 11, banco por packs `155/155`, código de salida
`0` por primera vez en la vida del proyecto.**

> 🟢 **Y ese verde es más peligroso que el rojo.** Mientras la compuerta salía con `1`, nadie la
> confundía con un permiso. Un `0` sí se confunde. Lo que dice es *los modelos y los arneses de PC
> no encuentran nada* — no que el firmware funcione en la tarjeta. La prueba está delante: con la
> compuerta en verde sigue abierta la regresión del Modo Automático en banco.

**El instrumento fue primero, y por eso apareció lo importante.** El arreglo se hizo en dos
commits separados a propósito: `2e5a1c2` (instrumento) y `1c97986` (firmware). Al medir de verdad,
la prueba 2.7 pasó de *"10 pares ciegos"* a **8**, y la 2.8 —que llevaba meses en `PASS` sin
evaluar un solo candidato— encontró un caso **explotable**:

> Con `FLAGS=2` y `SYNC_BAJA=56799`, **permutarlos deja el checksum intacto** y produce
> `FLAGS=56799`, con `CICLO`, `SYNC` y `DEGRADADO` los tres encendidos, y una marca de
> sincronización aparentemente válida. **Un arranque tras corte lo leería como autorización
> vigente y reanudaría el Modo Degradado sobre contenido corrupto.**

El arreglo: `calcularSuma()` devuelve los 32 bits crudos de Horner, sin plegar, y `sellar()` los
guarda enteros en `REG_SUMA_BAJA` (`DR7`) y `REG_SUMA_ALTA` (`DR8`, que estaba libre). `FIRMA`
`0x5EB0`→`0x5EB1`. Aplicado idéntico en las dos puntas (`respaldo.cpp` es byte a byte el mismo).
Coste: **+20 B por punta**, Maestro al 86,4 %.

**0 pares ciegos, exacto y no por muestreo.** Sin pliegue, la ceguera se reduce a una congruencia
lineal: una transposición cambia la suma en `(coef_i − coef_j)·(va − vb)` mod 2³², y el periodo de
colisión más corto es **33.554.432 veces mayor** que el hueco máximo alcanzable de 65.535.

**Verificado que el banco sabe fallar**, y no por el informe del agente: reintroducido el pliegue
en las dos puntas, el pack cae a `17/19` reportando los 8 pares exactos y el caso explotable.
Restaurado con `git diff` vacío y de vuelta a `19/19`.

### El número que llevábamos meses arrastrando y nadie había medido

El `FALLA` del checksum decía *"quedan **10** pares de registros con transposiciones ciegas"*.
Ese 10 **nunca se midió**: `PESOS_SUMA` quedó fijado a `1` para todos los registros cuando
`calcularSuma()` pasó a un hash de Horner, con el comentario *"compatibilidad"*. La resta
`PESOS_SUMA[a] - PESOS_SUMA[b]` daba siempre 0, así que la prueba metía los `C(5,2)=10` pares
posibles en la lista **sin llamar jamás al checksum real** — y el mensaje decía *"con los pesos
leídos del C++"*.

Medidos por fuerza bruta contra el Horner real son **8**. Y la causa no son los pesos: es el
**pliegue final** `((s>>16)^s)&0xFFFF`, que tira la mitad de la información.

**Arreglo pendiente, ya medido:** guardar el Horner de 32 bits crudo en dos registros (`DR8` está
libre) deja **0 pares ciegos** por ~20-40 B de flash. CRC-16 cuesta 512 B de tabla y deja 7;
Fletcher-16 deja 9, peor que hoy. Ninguno de los dos está diseñado para resistir transposición de
bloques de igual longitud. **El instrumento se arregla antes que el firmware**, o la prueba
seguirá cantando "10" sobre un firmware ya sano.

---

> ## ✅ EL SISTEMA FUNCIONA EN CAMPO (1 de Agosto de 2026)
>
> Confirmado por el funcional: **con dos radios en enlace directo, todo opera correctamente.**
> El diagnóstico del 31/07 era correcto — **la radio B1 estaba averiada** y era la causa del fallo,
> no el firmware.
>
> **Configuración que funciona hoy:** 2 radios, enlace directo, **sin repetidor**.
> **En curso:** conseguir antenas VHF de la banda correcta para recuperar alcance.

---

## 🔄 PARA RETOMAR — sesión del 1 de Agosto de 2026 (copiar y pegar en la sesión nueva)

```
Proyecto: D:\@Proyect\Controladora_Semaforos  (rama feat/n15-reloj-pantalla-hora, HEAD f37581f)
Controladora de semáforo móvil de 3 estados: STM32 Maestro + STM32 Esclavo, radios E90-DTU.
Lee roadmap.md (N-22, N-32..N-37) y README.md antes de proponer nada.

⚠️ LO PRIMERO: EL MODO AUTOMATICO DEJO DE FUNCIONAR EN BANCO (02/08), SIN TRIAGE.

Fallo tras cargar el firmware de hoy en LAS DOS puntas a la vez, asi que lo primero
es separar "regresion de firmware" de "el banco no tiene enlace". La pantalla del
Maestro ya trae el instrumento: la linea inferior RF:. Tres cuadros posibles:

  RF:SIN ENLACE o RF:---, luces a ambar a los 12 s
      NO es regresion: es SFTY-6 haciendo su trabajo sin radio. Confirmar si las
      radios estan puestas y configuradas, o si se esta entre el desmontaje y el
      cable directo. Armar el cable RS-485 (A-A, B-B, GND, bornera RS485_OUT, sin
      cruzar) elimina la radio como variable en dos minutos.

  RF:100% pero se queda en ROJO fijo sin arrancar el ciclo
      Regresion real. Sospechosos en orden: (a) la maquina de sincronizacion de
      coordinador.cpp, por si su trafico interfiere la espera de ACK_GREEN en el bus
      half-duplex; (b) el gate del mando en semaforo.cpp -senalActiva congela las
      salidas mientras destella; pegado en true, la logica corre y las luces no se
      mueven-. DATO QUE LOS SEPARA: ¿el Esclavo SI cambia de luz mientras el Maestro
      no, o no se mueve ninguno?

  No se puede ni entrar al modo porque un boton no responde
      Es N-26 al reves. Un boton ya en LOW al encender queda como estado, no como
      pulsacion: inerte hasta soltarlo. Con el mando de reles EN PARALELO, un rele
      cerrado en reposo o un corto en los 5 m de cable lo deja muerto. Antes el mismo
      defecto fisico se veia como "entra solo"; ahora como "no responde". Probar
      desconectando la bornera del mando y reiniciando.

  PRUEBA DE 30 s QUE DESCARTA EL TERCER CUADRO: en AJUSTAR HORA, ¿el Boton 3 avanza
  de digito? Si no avanza, es el boton y es hardware.

ESTADO DEL BANCO — dos fallos de hardware confirmados, ninguno de firmware:

1. El cristal Y2 de 32.768 kHz está MUERTO en el Maestro (N-37), cerrado por
   eliminación: VBAT = 3 V con la tarjeta APAGADA (descarta pila y R5), el reintento
   de 30 s de N-25 (descarta "lento"), y REINICIAR RELOJ de N-31 devolvió
   SIGUE PARADO (descarta registros sucios del dominio de respaldo).
   ➡️ Comprar DOS módulos DS3231, I²C por software en PB0/PB8 (únicos pines libres).
      Llevan LIR2032 RECARGABLE, nunca CR2032. Falta implementar el driver.
      Sin reloj no hay sincronización, y sin ella el Modo Degradado rechaza — eso
      NO es un defecto, es la guarda funcionando.

2. La pantalla del Esclavo sigue AZUL sin píxeles (N-22). Software descartado
   comparando llamada por llamada. Hipótesis viva: niveles lógicos — con el módulo a
   5 V el V_IH del ST7920 es 0,7·VDD = 3,5 V y el STM32 da 3,3 V.
   ➡️ Está SIN CARGAR el firmware de diagnóstico en 01_Firmware/Diagnostico_LCD/
      (compila, 36,7 %). Cárgalo en la tarjeta del Esclavo: separa "bucle de
      reinicio" de "no pinta", llena la pantalla entera y trae la variante LENTA
      del SPI, que es la prueba directa de la hipótesis de niveles.

LO PRIMERO QUE HAY QUE ARREGLAR, Y EN ESTE ORDEN:

  1. LOS VALIDADORES (N-36). Los tres miden fuente que ya no existe. El del Maestro
     dice ser el port del código arreglado y modela el ANTERIOR, con los dos modelos
     INTERCAMBIADOS. Sale con código 0 llevando 7 fallos dentro, así que la compuerta
     lo marca OK. Sin instrumento fiable no se puede saber si un parche funcionó —
     y N-32 nació justo de un arreglo que pareció bueno.
  2. N-32 y N-33 (verde-contra-ámbar, los dos con la misma causa de fondo:
     respaldo_horasDesdeSync() devuelve el mismo CADUCADA para "cambió el mes" y para
     "pasaron más de dos días"). ⚠️ reloj_fijarEnero() SOLO EXISTE EN EL MAESTRO:
     cualquier parche que haga al Esclavo contar días tiene que portarlo primero, o
     reintroduce la asimetría en febrero.
  3. N-34 y N-35, baratos y los dos convierten un gesto en una asimetría.

REGLAS VIGENTES:
  · Carga por SWD en mode=UR -e all. Si falla con "Unable to get core ID", REINTENTA:
    es timing, NO falta de cableado (NRST está conectado, lo probó un
    DEV_TARGET_HELD_UNDER_RESET). No cambiar a HOTPLUG.
  · Verificación: python 01_Firmware/compuerta.py — ABORTADO NO ES PASS.
  · Falta gcc de host (MinGW-w64): el arnés de las 209 pantallas sale ABORTADO y N-29
    está bloqueado. Instalarlo desbloquea las dos cosas.
  · ⛔ EL MODO DEGRADADO NO VA A LA CALLE hasta cerrar N-32..N-35. Campo en V8.4.
  · PENDIENTE DEL USUARIO: cambiar la contraseña de la cámara Hikvision — estuvo en
    un repo público y sigue en 5 commits del historial.
```

> ⚠️ **Este bloque queda obsoleto: el estado vivo se lleva ahora en [`ESTADO.md`](ESTADO.md)**
> (≤60 líneas, se reescribe cada sesión) y las reglas permanentes en [`CLAUDE.md`](CLAUDE.md),
> que Claude Code carga solo. El roadmap pasa a ser **historial**: aquí se gradúa lo cerrado,
> con su porqué. Una sesión nueva lee 2 KB, no 72.
>
> Lo de *"falta gcc de host"* de arriba **ya no aplica**: `gcc` estaba instalado desde el
> principio, fuera del PATH, y ahora la compuerta lo busca sola.

---

## 🏛️ Plan de arquitectura (3 de Agosto de 2026)

Nace de una propuesta de capas que era correcta en el diagnóstico y genérica en el remedio.
Medida contra el árbol, cambió en tres puntos que conviene dejar escritos porque **son el
argumento, no el detalle**:

1. **La barrera del Pilar A ya existe.** `semaforo.cpp` concentra todas las salidas en un único
   `escribirPines()` estático, y los destellos del mando **interceptan** las escrituras en vez
   de rodearlas — se hizo así tras descartar lo obvio (ignorar las llamadas), que colgaba al
   coordinador esperando un `S_VERDE` que nunca llegaría. No hay que construir la capa: hay que
   **cerrar su única fuga** y ponerle una guarda.
2. **`lib/Common` NO unifica el arnés en un binario.** Los símbolos que colisionan son `lcd_*` y
   `menu_*`, que difieren en 680 y 412 líneas **por diseño** (el Esclavo no ofrece modos de
   operación). Seguirán siendo dos binarios, y está bien.
3. **El tamaño no es el criterio; el estado estático disjunto sí.** Dos mitades que comparten
   un `static` no se separan: se reparten en dos archivos y un header, que es peor.

### Lo que ya está en su sitio

| Byte-idéntico hoy entre puntas | Diverge de verdad | Diverge por diseño |
|---|---|---|
| `protocolo.h/.cpp`, `respaldo.h/.cpp`, `ciclo_degradado.h`, `pines.h`, `pines_repetidor.h` | `reloj.cpp` (274) · `modo_degradado.cpp` (865) · `mando.cpp` (182) | `lcd.cpp` (680) · `menu.cpp` (412) · `main.cpp` (750) |
| mover = **cero** fusión | fusión real, una fase por módulo | **no se tocan** |

### Fases

| # | Qué | Depende del triage |
|---|---|---|
| **0** | **Instrumentos.** Guarda de rutas, `gcc` fuera del PATH, acta de evidencia. ✅ **hecho el 03/08** | no |
| **1** | **Cerrar la barrera.** Borrar `iniciarParpadeoFallo()` (`main.cpp:28`) — es `static` **sin un solo llamador**, código muerto del viejo `estadoGlobal`. Borrar `maestro.txt` de los dos `src/`. Guarda en el validador: **ningún pin de luz se escribe fuera de `semaforo.cpp`**, los **8** — incluidos `ROJO_PEATON` (PA6) y `VERDE_PEATON` (PA7). | no |
| **2** | `Esclavo/main.cpp` (643) → `config_ciclo.cpp` · `hora_rx.cpp` · `respuesta_diferida.cpp`. Hoy `config_verdeSegundos()` —API pública— está implementada en el punto de entrada, y ahí vive el **30/31**. | **sí** |
| **3** | `coordinador.cpp` (858) → `sincronizacion.cpp` (líneas 103-475) · `calidad_enlace.cpp`. Tres máquinas de estado sin un `static` compartido. N-23 vivió aquí. | **sí** |
| **4** | `modo_degradado.cpp` (612) → sacar `modo_ambar.cpp` (últimas 22 líneas: otro modo, y camino de seguridad). | **sí** |
| **5** | `lib/Common` con los 7 idénticos. **Trampa:** `validador_costura.py:248-256` compara esos pares **en binario** — al moverlos, esas pruebas se quedan sin pareja y **aprueban vacías**. El reemplazo, mismo commit: la prueba pasa a ser *"no existe copia local que tape la común"*. | **sí** |
| **6** | Las funciones puras extraídas entran al arnés de PC: `modo_degradado_evaluarEntrada()`, `verdeDeEsteEnvio()`, `calcularDesfase()`. Hoy el simulador barre 86.400 posiciones **contra un modelo**; después, contra el C++ real. | **sí** |

### Invariantes de cada fase

1. **Un commit = un movimiento puro.** Cero cambios de comportamiento mezclados.
2. **Instrumentos actualizados en el MISMO commit.** Los validadores parsean por ruta: mover sin
   actualizarlos es N-36 otra vez, midiendo fuente muerto con exit 0.
3. **Flash anotado antes y después.** Tope +2 % por fase. Maestro va por el 83,7 % de 64 KB.
4. **Compuerta verde antes y después.**
5. **Nada de `entrar()/procesarEvento()/salir()` con clases.** Las vtables cuestan flash que no hay.
6. **Campo se queda en V8.4** hasta que todo pase banco.
7. **Una fase = un commit = un `git revert` limpio.** La reversibilidad es parte del argumento
   ante el auditor, no sólo higiene.

> **Lo que NO se hace:** el *"si la orden viola una regla, conmuta automáticamente a Ámbar"* de
> la propuesta original. Una orden inválida **se rechaza y se reporta**; el ámbar automático
> queda en los caminos que ya lo tienen (SFTY-6, watchdog). Este proyecto lleva un día entero
> documentando por qué la máquina no decide sola. Y dentro de `Common` **no va ni un
> `#ifdef ROL_*`**: las asimetrías deliberadas —`RETARDO_RESPUESTA_MS` sólo en el Esclavo por
> SFTY-17— viven en la capa de aplicación, a la vista.

---

## 🧭 ORGANIZACIÓN — qué está cerrado, qué se puede hacer y qué espera (3 de Agosto de 2026)

### La certificación que bendijo el firmware del banco estaba hueca — **comprobado**

Se corrió la compuerta **tal como estaba en `7ddb3d1`**, el commit que se cargó a la tarjeta:

```
4 PASS | 2 FALLA | 0 ABORTADO
[ FALLA] arnes de pantalla        <- SIN CIFRA: nunca llego a correr
```

La política de ejecución de PowerShell bloqueaba `compilar.ps1`, así que **las 241 comprobaciones
de pantalla no se ejecutaron** — y el error se anotaba como `FALLA`, que afirma *"el firmware no
cumple"*, mientras el resumen decía **`0 ABORTADO`**. Las cifras que el README publicaba entonces
(`234/239`) venían de una corrida anterior escrita a mano.

> **Dos cosas son ciertas a la vez, y conviene no quedarse con media:**
>
> 1. **Los instrumentos no estaban corriendo bien.** Ese es el fallo de proceso, y está corregido:
>    hoy el arnés corre de verdad (`295/295`), un arnés bloqueado se clasifica `ABORTADO`, y la
>    guarda de rutas impide medir fuente que no existe.
> 2. **Aun corriendo perfectos, no habrían cazado esta regresión.** Ningún simulador ejerce el
>    ciclo automático sobre hardware: son modelos en Python que *validan el modelo, no el código*.
>
> Por eso la conclusión no es "arreglar los simuladores y volver a certificar", sino **bisección
> con firmware real** — y, a plazo, la V8.8: que los instrumentos midan el código en vez de
> duplicarlo.

### Estado por bloques

| | estado | ¿necesita banco? |
|---|---|---|
| **Fase 0** — instrumentos *(guarda de rutas, `gcc`, acta, clasificación `ABORTADO`)* | ✅ cerrada | no |
| **Banco por packs** — 20 packs, `142/154`, tres validadores cuadrando | ✅ cerrada | no |
| **N-39 · N-40** — el arnés medía fuente y contaba opciones que ya no existían | ✅ cerrados | no |
| **Fase 1** — barrera de salidas sin excepciones | ✅ cerrada | **no, y es demostrable** |
| **Fase 2** — `config_ciclo.cpp` fuera del punto de entrada | ✅ cerrada | no |
| **Fase 4** — `modo_ambar.cpp` fuera del Degradado | ✅ cerrada | no |
| **Fase 3** — partir `coordinador.cpp` | 🔴 **espera** | sí — es sospechoso de la regresión |
| **Fases 5-6** — `lib/Common` y funciones puras al arnés | 🔴 esperan a la 3 | — |
| **Regresión del Automático** | 🔴 bisección pendiente | **sí** |
| **N-37 · N-22** — cristal muerto, pantalla del Esclavo | 🔴 hardware | **sí** |

### 🧩 Fase 2 (03/08) — y la lección que dejó, que vale más que el movimiento

```
Esclavo/src/main.cpp   643 -> 583 líneas
flash Esclavo          42.108 -> 42.100 bytes   (BAJA 8; el tope de fase era +2 %)
compuerta              9 PASS | 2 FALLA | 0 ABORTADO, idéntica
```

`config_verdeSegundos()` y sus tres hermanas son **API pública** —las declara `config_ciclo.h`,
las consume el Modo Degradado— y estaban **implementadas en el punto de entrada**. Ahí vive
también el `30/31`, que ahora se ejerce solo con `--pack esclavo_03`.

**División de responsabilidad:** `config_rxDespeje()` **devuelve** si el par se cerró y el
llamante decide si acusa. Dejar `programarRespuesta()` dentro del módulo lo ataría al transporte
—es la maquinaria de cortesía de SFTY-17— y lo que ahí se guarda no depende de cómo se conteste.
El silencio cuando el par no se cierra sigue siendo deliberado: provoca el reintento del Maestro.

> ### ⚠️ La guarda de rutas no cazó la rotura, y conviene saber por qué
>
> El validador de costura leía `cfgVerdeSeg = pkt.param;` **dentro de `main.cpp`**. Al mudarse el
> código, `main.cpp` **sigue existiendo** — así que la guarda no vio nada — pero el patrón dejó
> de encontrarse y la prueba se puso en `FALLA`, **acusando al firmware de un defecto que no
> tiene**.
>
> Lo cazó la **comparación de totales**: `36/41` contra los `37/41` de siempre. **Segunda vez**
> que esa comparación salva la migración; la primera fue `luz_esclavo()` cortada a media función.
>
> **La regla, afinada:** la guarda de rutas vigila *ficheros que desaparecen*; **no** vigila
> *contenido que se muda de fichero*. Para eso la única red es comparar totales contra el
> monolito — y por eso los monolitos no se retiran todavía.

### Por qué la Fase 1 no necesita banco, y se puede afirmar

Es el único cambio del que se puede demostrar que **no pudo alterar el comportamiento**:

1. Lo borrado —`iniciarParpadeoFallo()`, `tBlink`, `amarilloOn`, `estadoGlobal` y su enum— **no
   tenía un solo llamador ni un solo lector**. La función era `static`, así que ni siquiera podía
   invocarse desde otro fichero.
2. **El flash no cambió ni un byte**: 55.504 antes y después. El compilador ya lo descartaba, de
   modo que el binario que corre es *idéntico*.
3. La guarda `barrera_01_pines_de_luz` da `5/5` en las dos puntas, y se probó **contra la fuga
   real**: pasada sobre el `main.cpp` anterior detecta `ROJO1`, `VERDE1`, `ROJO2` y `VERDE2`.

> Un binario idéntico no puede comportarse distinto. **Esa es la diferencia entre "lo simulamos"
> y "no hay nada que simular"** — y es exactamente el tipo de cambio que conviene hacer mientras
> el banco está bloqueado.

---

## 📦 V8.8 · Que los instrumentos midan el código en vez de duplicarlo (3 de Agosto de 2026)

**El número que lo justifica:**

```
FIRMWARE      (Maestro + Esclavo, src + include)          8.895 líneas
INSTRUMENTOS  (simuladores, validadores, arnés, compuerta) 8.898 líneas
                                                    ratio  1,00 : 1
```

Hay tanto instrumento como firmware. Y los simuladores **no ejecutan el C++: lo reimplementan
a mano en Python**. Esas 8.898 líneas no son pruebas — son una **segunda copia del firmware, en
otro idioma, que alguien sincroniza a mano**.

Sincronizar a mano no escala, y falló **tres veces en una semana**, siempre igual: la copia se
queda atrás. **N-36** (el validador leía un fichero que ya no existía), **N-39** (el arnés mide
`ncenB10` donde el código ya dibuja `7x14B`) y **la propia compuerta**, que daba `FALLA` de un
arnés que ni arrancaba. Con la V8.4 la disciplina alcanzaba porque el firmware era la mitad.

> **Y la hipótesis alternativa se descartó midiendo, no razonando.** Los simuladores **no son
> lentos**: los cinco corren en **10,7 s** y el banco completo, con las tres compilaciones, en
> **77,5 s**. No hay nada que optimizar por velocidad. El problema es de tamaño y de
> sincronización.

### Lo construido

| | |
|---|---|
| `banco/fuente.py` | lectura del firmware, **una** copia. Había **tres** —`_ruta/_fuente/_codigo/cte`, `_ruta_firmware/_texto/_leer`, `ruta/texto/num`—: el instrumento cometía el defecto que denuncia del firmware. |
| `banco/contador.py` | una sola definición de qué cuenta como comprobación. `37/41` y `30/31` no se podían sumar porque cada validador contaba a su manera. |
| `banco/correr.py` | `--pack` corre uno solo; `--listar` los enumera. Un pack que aborta **ya no tumba a los demás**. |
| `banco/modelos/esclavo.py` | 862 líneas: el modelo separado de las pruebas, igual que el plan pide para el firmware. |
| `packs/costura_01` · `packs/esclavo_01..05` | **3/3** y **30/31** |

### La prueba de que la migración no perdió nada

```
packs del Esclavo   30/31
validador_esclavo   30/31    <- el monolito, sin tocar, corriendo al lado
```

Mismo total, mismas pasadas, mismo único fallo. **No se da por hecho: se demuestra.** Esa es la
regla de todas las migraciones que quedan.

> ⚠️ Mientras dure, la compuerta cuenta **3 FALLA en vez de 2** — el defecto del Esclavo aparece
> dos veces, una por pack y otra por monolito. Un número que sube sin explicación es justo lo
> que hace desconfiar de un instrumento.

### Por qué esto y la arquitectura son la misma tarea

Un `coordinador.cpp` de 858 líneas con tres máquinas de estado dentro **no se puede compilar
suelto en el PC**; un `sincronizacion.cpp` sí. Cada función pura que se extrae deja de tener
gemela en Python y pasa a medirse directamente, como ya se hace con `lcd.cpp`. **Partir el
código es lo que permite medirlo.**

### La migración está completa — y demostrada

**17 packs, 127/139 comprobaciones, 0 ABORTADO.** Los tres monolitos siguen intactos y corriendo
al lado, y los veredictos coinciden uno a uno:

| | packs | monolito | |
|---|---|---|---|
| Maestro | 60/67 | 60/67 | ✅ |
| Esclavo | 30/31 | 30/31 | ✅ |
| Costura | 37/41 | 37/41 | ✅ |

### Lo que salió al partirlos: el acoplamiento, con nombre y apellido

**Catorce dependencias cruzadas entre bloques**, ninguna declarada. Funcionaban *solo* porque
cada fichero se ejecutaba de arriba abajo y el bloque anterior ya había corrido:

| | |
|---|---|
| **Maestro** | `_main` (B1→B2) · `horas_desde_sync`, `marcar_sync`, `REG_*`, `ORDEN_SUMA` (B2→B3) · `DERIVA_PEOR_S_DIA` (B3→B4,B5) |
| **Costura** | `rama_verde`, `rama_despeje` (S2→S4) · `luz_maestro`, `luz_esclavo`, `fase` (S2→S5,S6) · los códigos `ROJO`/`AMBAR_FIJO`/`VERDE` |

Correr un bloque suelto —que es justamente lo que ahora se puede hacer— las habría roto todas
en silencio. **El fichero único no creaba el acoplamiento: lo ocultaba.** Es la forma concreta
de *"un cambio en una esquina rompe otra sin relación aparente"*.

### El banco se cazó a sí mismo

Al comparar totales, costura salió **35/41 contra 37/41** del monolito: dos comprobaciones que
allí reproducen y aquí no. La causa estaba en la migración —`luz_esclavo()` cortada a **media
función**, en la línea 423 de 426, devolviendo `None` en vez de la luz—.

**Ninguna prueba dio error.** Simplemente dos hallazgos dejaron de reproducirse. Sin la
comparación de totales, el banco habría seguido corriendo y diciendo `PASS` con un modelo
mutilado — que es, exactamente, N-36 otra vez.

> Por eso la regla no es *"revisar que quedó bien"* sino **comparar los números**. Lo que
> "parece que va" es lo que lleva tres veces engañando a este proyecto.

### Deuda anotada, no escondida

`banco/modelos/costura.py` conserva sus propios `ruta()`/`texto()` en vez de usar
`banco/fuente.py`. **No es descuido:** el `ruta()` de costura devuelve `None` cuando falta un
fichero, y varias secciones dependen de ese `None` para reportar *"FALTA EN UN PROYECTO"* —un
hallazgo— mientras que `banco.fuente` **aborta** —no poder medir—. Unificarlos cambia
comportamiento, y esta migración no cambia comportamiento. Se unifica después, con su propio
commit y su comparación de totales.

### Pendiente

Retirar los tres monolitos. **No antes** de que los packs lleven un tiempo corriendo en la
compuerta: mientras los dos corran, la comparación de totales es la red.

---

## 🧪 N-39 · El arnés medía una fuente que el firmware ya no usa

Con el `gcc` resuelto, el arnés corrió completo por primera vez: **236/239** (Maestro 110/113,
Esclavo 126/126). De los tres fallos, **dos son del instrumento**:

```
[FALLA] Ancho medido: "SINCRONIZADA" (ncenB10) termina en x=129 y cabe en 128
[FALLA] Ancho medido: "SOLO MAESTRO" (ncenB10) termina en x=131 y cabe en 128
```

`lcd.cpp:383` ya dibuja esos títulos en **`7x14B`** — el arreglo de N-38 está puesto. Pero
`arnes_lcd.cpp:576-579` sigue midiéndolos con **`ncenB10` escrito a mano**: la prueba no se
actualizó cuando se actualizó el fuente.

**Es la misma familia que N-36, con otro disfraz.** Allí el instrumento leía un archivo que ya
no existía; aquí lee el archivo bueno pero con una suposición vieja dentro. La lección se
mantiene: *el instrumento también envejece, y nadie le pone fecha de caducidad.*

Arreglo: medir `7x14B`, y **dejar la `ncenB10` como control negativo** — exigir que **no** quepa,
que era la intención declarada en N-38. Una comprobación de anchos que aprueba todo no comprueba
nada.

## 🔍 N-40 · La cuarta franja era una cuarta opción ✅ **CERRADO**

```
[FALLA] Submenu: se ven 4 opciones de 3
```

El submenú **tiene cuatro opciones**. `REINICIAR RELOJ` la añadió N-31, y `menu.cpp` lo dice en su
propio comentario —*"N-31: la cuarta, REINICIAR RELOJ"*—. La prueba se quedó exigiendo tres y
reportaba `FALLA` sobre un menú correcto.

> **Tercera vez el mismo patrón en una semana.** N-36: el validador leía un fichero que ya no
> existía. N-39: el arnés medía una fuente que el código ya no usa. N-40: el arnés cuenta opciones
> que el menú ya no tiene. Siempre igual — **el instrumento se queda atrás y acusa al firmware de
> su propio retraso**.
>
> Y lo que los tres comparten: **el instrumento llevaba el dato escrito a mano**. Mientras el
> número viva en dos sitios, alguien actualizará uno y no el otro.

### Con esto, el arnés de pantalla queda en verde por primera vez

```
MAESTRO  115/115   ESCLAVO  126/126   TOTAL  295/295
```

### Las dos guardas que impiden la recaída

No basta con corregir el número, porque el fallo volvería con el siguiente cambio. Dos packs
nuevos leen el firmware y exigen que el arnés y el código digan lo mismo:

| pack | qué cierra |
|---|---|
| `maestro_06_fuentes_pantalla` | la fuente con la que se **dibuja** cada título es la misma con la que el arnés lo **mide** — y además es de **paso fijo**, que es la propiedad que hace el ancho calculable |
| `maestro_07_menu_opciones` | `OPCIONES_RAIZ`/`OPCIONES_CONFIG`, el tamaño del array y el número de textos **coinciden los tres**, y la última opción cae dentro de los 63 px |

Ese último límite no es estético: en la V8.6 la sexta línea caía en `y=69`, y el peligro no era
que no se dibujara —la salvaguarda lo impide— sino que **el cursor sí podía navegar hasta ella**,
dejando al operario en una opción invisible.

---

## 🧮 N-41 · El modelo del Esclavo no tenía la ventana de vigencia del VERDE ✅ **CERRADO**

`modelos/esclavo.py` no implementaba `VENTANA_CONFIG_MS` y reportaba la mezcla del par de
configuración **también donde el firmware ya la rechaza**. Medido con 11 s de hueco: el firmware
rechaza, el modelo mezclaba **y encima acusaba**.

Corregido en las dos copias del modelo, con la constante **leída del C++** —sin valor por
defecto, que es la regla del banco: un banco que no puede fallar no demuestra nada—.

> **El `30/31` sigue fallando, y eso es lo correcto.** No se silenció nada: el defecto real de
> `config_ciclo.cpp` —el par se mezcla si dos configuraciones caen a menos de 3 s— sigue ahí y
> sigue reportado. Lo único que cambió es que **dejó de exagerar su alcance**. Un arreglo que
> hubiera puesto el contador en verde habría sido el fallo, no el éxito.

---

## 🚦 N-42 · La bisección del Modo Automático apuntaba a la ventana equivocada

**Contexto.** El Modo Automático dejó de funcionar entre dos compilaciones. El triage del 03/08
descartó radio *(`RF:` da porcentaje)* y botón trabado *(el Botón 3 responde)*, y dejó dos
sospechosos: **N-23** *(cambió quién mueve el coordinador)* y **N-21** *(siembra de botones)*.
Se levantó `05_Funcional/ENCARGO_bisect_automatico.md` con 8 commits candidatos.

**Al compilarlos aparecieron dos errores del propio encargo.**

### 1. Ninguno de los 8 candidatos toca el camino del ciclo

Cruzado commit contra fichero, los 8 tocan solo `reloj.cpp`, `botones.cpp`, `lcd.cpp`/`lcd.h`,
`modo_hora.cpp`, `menu.cpp`, `reloj.h` y **una línea** de `main.cpp`. Ninguno modifica
`coordinador.cpp`, `modo_automatico.cpp`, `semaforo.cpp` ni `mando.cpp`.

`470a5c9` (N-23), el *"sospechoso principal"*, toca `lcd.h`, `lcd.cpp` y `modo_hora.cpp`: cambió
quién mueve el coordinador **desde la pantalla AJUSTAR HORA**, no desde el ciclo automático. La
justificación del encargo —*"cambió quién y cuándo mueve el coordinador, que es exactamente la
máquina que hace correr el ciclo"*— es cierta a medias, y la mitad que falta es la que importaba.

### 2. La ventana empezaba una hora y media tarde

El responsable de banco sitúa el último firmware bueno hacia las **16:00 del sábado 01/08**. Los
8 candidatos van de las **17:58 a las 19:51**: todos posteriores al fallo.

> **Los 8 habrían fallado en banco.** El operario habría gastado la sesión entera —cada carga es
> SWD en `mode=UR` con sus reintentos— para concluir *"es más antiguo que el más antiguo"*. Una
> bisección cuyo extremo "bueno" no es bueno no acota nada: solo consume cargas.

### El sospechoso que la ventana dejaba fuera

```
2779d9b  01/08 16:00  feat(SFTY-21): Modo Degradado y mando de reles en Maestro y Esclavo
```

Cae en la hora exacta que da el banco y toca **15 ficheros del Maestro** — el cambio más grande
de ese día con diferencia. Y el mecanismo encaja: el mando de relés **intercepta las escrituras
de pines de luz** en vez de rodearlas, que es justo por donde el ciclo podría dejar de avanzar.

### Dos hipótesis propias, las dos refutadas por lectura antes de reportarlas

| hipótesis | por qué cae |
|---|---|
| El cristal muerto (N-37) bloquea el ciclo por los guardas de reloj del coordinador | Los 5 guardas solo suprimen sincronización y medida de desfase. Los dos que están en el bucle (`coordinador.cpp:443`, `:465`) bajan la bandera y **caen hacia abajo sin `return`**. No detienen el ciclo. |
| `reloj_actualizar()` de N-25 se atasca con el RTC parado *(es el único de los 8 que toca `main.cpp`)* | Con el cristal muerto sale en `LSERDY == RESET`, tercera línea, sin llamada bloqueante. `rtc.begin()` solo se alcanza con el oscilador ya listo. El encargo acertaba al descartarlo, aunque describiera mal el porqué. |

> Se anotan **refutadas**, no omitidas. Una hipótesis descartada en silencio vuelve a proponerse
> la sesión siguiente, y la segunda vez ya nadie recuerda que se había mirado.

### Lo que queda

Entregable en `05_Funcional/bisect_entregable/`: **16 firmwares del Maestro**, de las 15:49 a las
19:51 del 01/08, **todos `SUCCESS`**, con `LEEME.txt`. La escalera baja hasta `c72700e` (15:49),
que es el **ancla de "última buena"** — sin un extremo bueno de verdad, una bisección no acota,
solo consume cargas. Ruta de banco: **2 cargas** si el sospechoso acierta, **4** si hay que
bisecar de verdad.

**El salto de flash respalda la hipótesis:** `c72700e` (15:49) va al **74,7 %** y `2779d9b`
(16:00) salta al **80,2 %** — **+3.572 B de una vez**, el mayor incremento del día.

> **Comparar hashes, no tamaños.** `2779d9b` y `831c4f0` **son el mismo binario byte a byte**:
> `831c4f0` añade el módulo de respaldo pero no lo conecta hasta `ee01702`, así que el enlazador
> lo descarta entero. Por tamaño ya parecían sospechosos; por MD5 quedó demostrado. **Cargar el
> segundo es una carga de banco tirada**, y cada carga es SWD en `mode=UR` con sus reintentos.
> El mismo chequeo sobre la ventana vieja dio lo contrario: `8a45ae7` y `f37581f` pesan igual
> —55.504 B— y **no** son el mismo fichero. El tamaño no decide en ninguna de las dos direcciones.

**Los `.bin` no se versionan.** Cada uno se regenera con `git worktree add` + `pio run` sobre el
hash que lleva en el propio nombre — misma razón que los `*.zip`: el repositorio ya contiene el
fuente del que salen. El `LEEME.txt` **sí** se versiona: es el análisis y el árbol de carga, y eso
no se recompila. Si el banco señala un culpable, **ese** `.bin` puede subirse suelto y a
propósito, como evidencia de lo que se probó.

**Falta el banco.** Cargar por SWD `mode=UR` con `-e all` y decir cuál es el primero que falla.

> ⚠️ **Los simuladores están en verde sobre el firmware que falla:** compuerta `9 PASS / 2 FALLA
> / 0 ABORTADO` y arnés de pantalla `295/295`. Pasar los simuladores es **necesario y no
> suficiente** — ninguno ejerce el ciclo automático sobre hardware. Esta regresión es justo de
> las que el modelo no puede ver.

---

## 🔌 N-43 · Un arnés que compila el C++ real, roto y sin conectar — y nadie se enteraría

**Medido el 03/08 corriendo los tres arneses por separado, fuera de la compuerta.**

| arnés | compila C++ real | exit | resultado |
|---|---|---|---|
| `Validacion_LCD` | `lcd.cpp`, `menu.cpp`, `modo_degradado.cpp` | 0 | **295/295** |
| `Validacion_Ciclo` | `ciclo_degradado.h` | 0 | **29/29** |
| `Validacion_Respaldo` | `calcularSuma()`, Horner, `respaldo_horasDesdeSync()` | **1** | ❌ *"compila pero no responde al PING"* |

**`compuerta.py` solo conoce dos de los tres.** Buscadas las referencias, solo aparecen
`Validacion_LCD` y `Validacion_Ciclo`; `Validacion_Respaldo` no está en la compuerta ni en el
acta —12 suites, ninguna es esa—.

> **Es la enfermedad del proyecto en su forma nueva.** N-36, N-39 y N-40 fueron *el instrumento se
> queda atrás y acusa al firmware*. Aquí el instrumento **ni siquiera corre**: existe, compila el
> fuente real, está roto desde hace días, y **ninguna cifra del acta lo echa de menos**. Un
> instrumento que no está en la compuerta no mide nada, y lo peor es que no se nota — no deja un
> `ABORTADO`, deja un hueco.

### El fallo no es de compilación ni de librería

Ningún `#include` falla; `g++ -O1 -w` compila limpio. Revienta el autochequeo de vida de
`compilar.ps1`:

```powershell
$respuesta = "PING" | & $exe
if ($respuesta -ne 'PONG') { Write-Error "El arnes compila pero no responde al PING: '$respuesta'" }
```

> ⚠️ **CORRECCIÓN 03/08 — la causa que decía esta ficha era FALSA, y la escribí yo.**
>
> Aquí ponía que el fallo era *"el canal de piping de PS 5.1 hacia un proceso nativo"*, dado por
> reproducido. **No se sostiene.** Medido byte a byte: PS 5.1 manda `PING\r\n` **sin BOM** —bash
> manda `PING\n`— y `strncmp(linea,"PING",4)` casa con los dos. Ejecutado el binario bajo PS 5.1
> devuelve **`PONG`**, tipo `String`. La explicación era plausible y estaba equivocada.
>
> **La causa real sigue SIN identificar.** El diagnóstico se quedó a medias al agotarse la sesión;
> lo siguiente es correr `compilar.ps1` entero y ver cuál de sus tres `Write-Error` dispara —el de
> `gcc`, el de la identidad `respaldo.cpp`/`respaldo.h` entre Maestro y Esclavo, o el del PING—.
> **Ojo con el segundo:** si los ficheros difirieran entre puntas, el mensaje sería otro y el
> hallazgo mucho más grave que un arnés roto.
>
> Se deja escrito en vez de borrarlo: una causa falsa que se retira en silencio vuelve a
> proponerse, y la segunda vez ya nadie recuerda que se comprobó.

> ✅ **CONTRA-CORRECCIÓN 04/08 — la refutación de arriba era la equivocada. La causa original
> era BUENA.** PS 5.1 **sí** antepone un BOM UTF-8. Volcado con `od` sobre la tubería real:
>
> ```
> PS 5.1 :  ef bb bf  50 49 4e 47  0d 0a
> pwsh 7 :            50 49 4e 47  0d 0a
> ```
>
> Con esos tres bytes delante, `strncmp(linea,"PING",4)` no casa y el arnés contesta `ERROR`.
> Comprobado en los dos sentidos: bajo `pwsh` 7 y bajo bash responde `PONG`; bajo PS 5.1
> respondía `ERROR`.
>
> **Lo que esto enseña no es que PS 5.1 mande BOM.** Es que la frase *"medido byte a byte"* del
> 03/08 se escribió sobre algo que no se había medido byte a byte —o se midió con otro emisor—, y
> entró al repositorio con el mismo peso que una medida de verdad. Una refutación **también** es
> un instrumento, y también hay que descartar al buscador antes de creerla. Las dos notas se
> quedan aquí, en orden: es la única forma de que la tercera vez alguien vea el historial completo
> en vez de una afirmación limpia.

> **Si esto se conectara tal cual, la compuerta diría `FALLA`** —*"el firmware no cumple"*— sobre
> un checksum que nadie ha medido. Es literalmente el fallo de N-27 y el de la propia compuerta,
> por tercera vez. Va con `ABORTADO`, o no va.

### Y desbloquea N-29

`N-29` pedía *"dejar de reimplementar `calcularSuma()` en Python"* y estaba marcado **bloqueado
por falta de `gcc` de host**. Ese bloqueo era **falso desde el principio** —N-38 demostró que
`gcc` llevaba semanas instalado, fuera del `PATH`—, y **el arnés que N-29 pedía ya está escrito**:
lo trajo N-31 y ejerce `calcularSuma()` y el Horner contra el C++ real.

O sea: la pieza existe, el bloqueo no era real, y lo único que falta es **arreglar el PING y
enchufarla a la compuerta**. Sale barato y cierra el N-29 que lleva días abierto.

### ▶️ DÓNDE RETOMAR — un comando, y el árbol de lo que salga

```powershell
powershell -ExecutionPolicy Bypass -File "01_Firmware\Validacion_Respaldo\compilar.ps1"
```

`compilar.ps1` tiene **tres** `Write-Error`. Lo único que falta es leer **cuál dispara**:

| # | mensaje que saldría | veredicto | qué significa |
|---|---|---|---|
| 1 | *"No hay gcc de host en el PATH"* | ❌ **descartado** | `gcc` está: MinGW-W64 UCRT 16.1.0, medido el 03/08 |
| 2 | *"`respaldo.cpp` DIFIERE entre Maestro y Esclavo"* | ⚠️ **candidato por eliminación** | **No es un arnés roto.** Es que las dos puntas fechan la misma sincronización con **aritmética distinta** — condición de seguridad, y mucho más grave que N-43 |
| 3 | *"compila pero no responde al PING"* | ❌ **descartado** | Medido: bajo PS 5.1 responde `PONG`, tipo `String` |

**Descartados el 1 y el 3 por medida, queda el 2.** No se da por hecho —eso es justo el error que
se corrigió arriba—: **se corre el comando y se lee el mensaje.** Si sale el 2, esto deja de ser
una tarea de instrumentación y pasa a ser un hallazgo de firmware con su propio N-x.

### ✅ CERRADO 04/08 — se corrió el comando, y no era ninguno de los tres

**El árbol de arriba se queda como está, tachado por la medida y no reescrito**, porque su error es
lo que hay que recordar: se construyó *por eliminación* sobre tres ramas que alguien supuso
exhaustivas, y la que disparó era una cuarta que nadie había listado. Eliminar entre opciones
incompletas no es medir; es adivinar con tabla.

Lo que salió al correrlo fue un **cuarto** `Write-Error`, en `compilar.ps1:54` —*"Fallo compilando
el arnes de respaldo"*— con veinte líneas de `ld: cannot find crt2.o / -lgcc / -lmsvcrt`. Y eran
**dos fallos apilados**, que es justo por lo que no se dejaba diagnosticar:

| | qué era | estado |
|---|---|---|
| **A** | El toolchain **no enlazaba** por la `ñ` de su ruta. Tapaba todo lo demás. | ➡️ **N-44** |
| **B** | El **BOM de PS 5.1**, el fallo real del arnés — solo visible una vez que A deja compilar. | ✅ arreglado en las dos puntas |

➡️ **El candidato 2 queda REFUTADO por medida: no dispara.** `respaldo.cpp` y `respaldo.h` son
idénticos entre Maestro y Esclavo. **No hay hallazgo de firmware**, y conviene decirlo fuerte
porque esa ficha lo anunciaba como *"mucho más grave que N-43"* durante un día entero, apoyado
solo en que los otros dos estaban tachados.

**Ya enchufado a la compuerta**, con sus dos comprobaciones clasificadas distinto a propósito:
identidad entre puntas es `FALLA` *(eso sí es el firmware incumpliendo)*; cualquier cosa que
impida llegar hasta ahí es `ABORTADO`. **Cierra N-29.** La compuerta pasa de 12 a **13 suites**:
`11 PASS | 2 FALLA | 0 ABORTADO`.

### Toolchain, para no volver a dudarlo

`gcc`/`g++`/`mingw32-make` **en el `PATH`**, MinGW-W64 UCRT `16.1.0`, misma ruta en Bash y en
PowerShell. **No falta ninguna librería** —y esa frase, cierta, es la que despistó un día entero:
ver **N-44**. `Validacion_LCD` **ya no necesita** `-ExecutionPolicy Bypass`: corre con
`RemoteSigned` porque el `.ps1` no tiene marca de descarga. El bypass de la compuerta puede
quedarse como cinturón, pero ya no es lo que lo sostiene.

---

## 🔗 N-44 · Un `gcc` que existe, responde `--version` y compila — pero no enlaza ✅ **CERRADO 04/08**

**El delator fue el acta.** Del 03/08 al 04/08, con el mismo repositorio, los **dos** arneses que
compilan C++ real pasaron de `PASS` a `ABORTADO`:

| | 03/08 | 04/08 |
|---|---|---|
| arnés de pantalla | ✅ 295/295 | ⛔ ABORTADO |
| arnés del ciclo | ✅ 29/29 | ⛔ ABORTADO |
| resumen | 8 PASS · 2 FALLA · **0 ABORTADO** | 8 PASS · 2 FALLA · **2 ABORTADO** |

Y las dos actas registran **el mismo compilador**: `MinGW-W64 ... r3 16.1.0`. Nadie tocó el
toolchain — los binarios siguen fechados el 10/06 y sus carpetas el 31/07.

### Qué fallaba

`ld` no encontraba `crt2.o`, `libgcc.a`, `libmsvcrt.a`… **que existen**: `crt2.o` mide 9.870 bytes,
`libgcc.a` 6,9 MB, y un proceso los abre y los lee sin problema. `gcc -print-file-name=crt2.o`
devuelve la ruta correcta. Compilar a `.o` funcionaba; **solo reventaba el enlazado**.

El toolchain vivía bajo `C:\Users\Diego.Zu`**`ñ`**`iga\AppData\...`. Reproducido con un caso
mínimo — el mismo `hola.c`, dos carpetas hermanas que solo se diferencian en una letra:

```
gcc -c ...\sin_tilde\hola.o    -> OK          ld -r desde sin_tilde   -> OK
gcc -c ...\con_ñ_tilde\hola.o  -> FALLA       ld -r desde con_ñ_tilde -> cannot open output file
```

**Descartados por medida, no por razonamiento:** el shell *(falla igual en `cmd`, PS 5.1, `pwsh` 7
y bash — y funciona en los cuatro con el toolchain bueno)*, la codepage *(`chcp` 65001)*, que
hubiera otro `gcc` *(solo hay uno)*, y Defender *(sin detecciones, `ControlledFolderAccess` = 0)*.

### El arreglo, y por qué es del instrumento y no del PC

Copiado el toolchain a `D:\toolchain\mingw64` —ruta ASCII, 913 MB, sin tocar el original— el mismo
`gcc` enlaza y el ejecutable corre. Pero mover ficheros de sitio **no es el arreglo**: el arreglo
es que la compuerta deje de aceptar un `gcc` que no ha demostrado enlazar.

`_asegurar_gcc()` ya no devuelve el primero que encuentra: **le exige enlazar un `main()` vacío** y
se queda con el primero que lo consigue, aunque haya otro antes en el `PATH`. Censar el instrumento
no es comprobar que mide — es la misma distinción que separa `PASS` de `ABORTADO`, aplicada al
compilador. Y el test se monta en un directorio de ruta ASCII **a propósito**: el `TEMP` de este
usuario también lleva la tilde, así que un test montado ahí daría negativo hasta con un `gcc` sano
y **rechazaría el bueno**. La regla del instrumento, otra vez, dentro del propio arreglo.

Cuando ninguno enlaza, el `ABORTADO` **dice por qué** en lugar de *"no hay gcc"* — que era falso y
mandó el diagnóstico en la dirección equivocada durante días, exactamente como el bloqueo falso de
N-29.

### ⚠️ Lo que queda ABIERTO, y no se cierra con una explicación bonita

**Por qué el 03/08 sí enlazaba.** Mismo binario de `gcc`, misma ruta con `ñ` desde el 31/07, misma
acta certificando `295/295`. La `ñ` está reproducida con caso mínimo, pero **no explica el cambio
de un día para otro**, y no hay medida que lo explique todavía. Se propuso el terminal del IDE
*(VS Code)* como causa: **descartado** — el fallo se reproduce en `cmd`, PS 5.1, `pwsh` y bash por
igual.

Queda escrito como pregunta abierta y no como causa. Ya se pagó una vez este mes por escribir una
causa plausible con la palabra *"medido"* encima.

---

## 🗓️ N-49 · La grieta del día-del-mes — lo único que bloquea la V8.8 ⏳ **ABIERTO — T1+T2 hechas, 05/08**

**El defecto, en una frase:** durante una sesión de Modo Degradado, cuando el contador de días
vuelve de 31 a 1, **el Maestro se rinde a ámbar y el Esclavo sigue dando verde**. 24 días al año.

Lo dice el validador con estas palabras:

> `LAS DOS PUNTAS SE RINDEN EN INSTANTES DISTINTOS en 24 dias del ano. (...) MAESTRO EN AMBAR`
> `mientras el ESCLAVO SIGUE DANDO VERDE por reloj.`

Un ámbar dice *"decide tú"* y el conductor llega **alerta**; un verde dice *"el otro lado está en
rojo"* y llega **confiado**. Aquí el otro lado no está en rojo. Es el **riesgo residual nº 2 de
SFTY-21**, reintroducido.

### Por qué pasa, con los datos ya verificados

- `REG_SYNC_DIA` guarda **el día del mes (1..31)**, sin mes ni año. `respaldo_horasDesdeSync()`
  (`respaldo.cpp:176-204`) resta `hoy − guardado` y **declara `CADUCADA` si sale negativo o > 2**.
  Al volver de 31 a 1 la resta da −30 → `CADUCADA` → el Maestro se rinde.
- **Eso, solo, es correcto:** el propio código dice *"ante la duda, caducada"*. Fallar hacia el
  ámbar es fallar en la dirección segura.
- **El defecto es la asimetría.** El Esclavo (`Esclavo/src/modo_degradado.cpp:142`) mide con
  `millis()`; solo consulta el respaldo **al reanudar** (línea 260,
  `tUltimaSync = millis() - horas * 3600000UL`). Arranca bien y luego cuenta por su cuenta, así que
  **la vuelta del contador no le afecta** y sigue en verde.
- ⚠️ **Y no es un cruce de mes real.** `reloj_fijarEnero()` ancla las dos puntas a **enero a
  propósito**, para que vuelquen de 31 a 1 en el mismo instante. El calendario es artificial: lo que
  hay es un **contador de días que da la vuelta cada 31**, por diseño. Cualquier arreglo que hable
  de "meses" está resolviendo un problema que no existe.

### Las dos tareas, y la segunda es la que importa

**T1 — Que la fecha guardada se pueda restar aunque el contador vuelva.**
Hay **tres registros de respaldo libres** (`DR8`, `DR9`, `DR10`: se usan 7 de los 10), así que no
hay que pelear por espacio.

> ⚠️ **La solución de una línea es una trampa, y conviene dejarlo escrito antes de que alguien la
> proponga.** Con el calendario anclado a enero, `dias = (hoy - guardado + 31) % 31` resuelve la
> vuelta… y **abre un agujero**: no distingue *"hace 1 día"* de *"hace 32"*. Autorizaría el
> Degradado sobre una sincronización de hace un mes, que es exactamente lo que el límite duro de
> 48 h existe para impedir. **El módulo cambia un fallo hacia el lado seguro por uno hacia el lado
> peligroso.**
>
> Lo que hace falta es un **contador monótono que no vuelva en el rango de interés** — 16 bits de
> días son 179 años— escrito en un registro libre y entrando en el checksum (`REG_SUMA`).

**T2 — Que el Esclavo se rinda por el MISMO criterio que el Maestro.**
Es la tarea de verdad. Aunque T1 quede perfecta, si cada punta decide con una regla distinta la
asimetría vuelve por otro lado: hoy una consulta la pila y la otra cuenta con `millis()`. **Dos
relojes que se rinden por criterios distintos acabarán rindiéndose en instantes distintos.**

### Restricciones que no se negocian

- `respaldo.cpp` y `respaldo.h` deben quedar **byte a byte idénticos entre Maestro y Esclavo** — lo
  comprueba `Validacion_Respaldo` y es lo que garantiza que las dos fechan con la misma aritmética.
- **Pasa por banco.** Es un camino de seguridad vial y ningún simulador lo sustituye.
- El arnés del respaldo ya compila `respaldo_horasDesdeSync()` real: **ahí se prueba T1 antes de
  tocar la tarjeta**, y con control negativo que demuestre que caza la vuelta del contador.
- Flash del Maestro al **86,3 %**: quedan ~9 KB. Anotar la cifra antes y después.

---

### ✅ T1 HECHA Y MEDIDA — commit `4c93342`

**Lo que se hizo:** `DR5`/`DR6` dejan de guardar día-del-mes + segundo-del-día y pasan a llevar las
**dos mitades del contador del RTC** — 32 bits de segundos que mantiene la pila, monótonos, sin
vuelta en 136 años. Comparar es una resta.

Las dos funciones **siguen siendo puras** *(reciben el contador, no lo leen)* a propósito: si
leyeran el RTC por dentro, la única forma de probarlas sería con la tarjeta delante.

**Medido contra la función real compilada, no contra un modelo:**

| caso | antes | ahora |
|---|---|---|
| +1 h · +25 h · +48 h | 1 · 25 · 48 | ✅ igual |
| vuelta 31 → 1 | ❌ `CADUCADA` | *ya no existe: no hay días del mes* |
| **+31 días y 1 h** | ❌ **1** *(leído como reciente)* | ✅ **745** |
| el reloj retrocede | — | ✅ `CADUCADA` |
| sin reloj (contador 0) | — | ✅ `CADUCADA` |

**`FIRMA` `0x5EAF` → `0x5EB0`, y no es opcional.** `DR5`/`DR6` significan otra cosa; un equipo
actualizado que reconociera la firma vieja leería como fecha lo que es un día del mes. Al no
reconocerla, `respaldo_setup()` borra y se arranca **sin sincronización previa** —el estado seguro—,
a costa de **resincronizar a mano cada equipo que se actualice**. Es un coste de despliegue real.

`reloj_contadorSegundos()` entra **idéntico** en las dos puntas. Flash: Maestro `56.584 → 56.552`
(**−32 B**), Esclavo `42.112 → 42.072` (**−40 B**) — la lógica nueva es más simple que la que quita.

### ✅ INSTRUMENTOS PORTADOS — y el defecto vial desapareció

Portadas **una prueba por una**, anotando el destino de cada una:

| | |
|---|---|
| **borrada** | *"el cambio de mes da SIEMPRE caducada"* — documentaba el defecto |
| **borrada** | el rango `0..71 h`, límite artificial de la ventana de 2 días |
| **invertida** | fronteras: día 0 / día 32 / seg ≥ 86400 → ahora el **contador cero** y el retroceso |
| **invertida** | la 2.3, que **exigía el defecto** → ahora exige que 2 h se fechen como 2 h |
| **conservadas** | *"nunca subestima"* y *"el reloj que retrocede da caducada"* |
| **nueva** | que 31 días se midan como semanas. **Con el día del mes era imposible de escribir.** |

Y los bloques `3.x` llamaban a la función nueva **con datos del formato viejo**, lo que disparó los
recuentos a *"365 días"* y *"80300 escenarios"*. **No era el firmware empeorando: era el instrumento
mal llamado.** Migrados — el contador es `dia_absoluto*86400 + segundo`, así que la conversión a día
del mes desaparece y el modelo queda más simple.

> **Un fallo del instrumento que solo aparece AL CERRAR el defecto:** el mensaje de *"rendiciones
> prematuras"* se construye siempre y hacía `min()` sobre una lista vacía, tumbando el validador
> entero justo cuando dejaba de haber rendiciones. **Un instrumento que se cae al dar una buena
> noticia no sirve.**

**Resultado — el termómetro dio la razón al arreglo:**

```
ANTES  [FALLA] LAS DOS PUNTAS SE RINDEN EN INSTANTES DISTINTOS en 24 dias del ano.
               MAESTRO EN AMBAR mientras el ESCLAVO SIGUE DANDO VERDE por reloj.
AHORA  no aparece.
```

Validador Maestro **60/67 (7 FALLA) → 62/67 (5 FALLA)**; los cuatro que se van son los del
día-del-mes. Compuerta: **12 PASS · 2 FALLA · 0 ABORTADO** de 14, y **ninguno de los 5 restantes es
vial**.

### ▶️ DÓNDE RETOMAR — T2, y cuatro cabos

**T2 es lo que falta para cerrar N-49**, y no es cosmética: el Esclavo se rinde con `millis()`
(`Esclavo/src/modo_degradado.cpp:142`) y el Maestro consulta la pila. **Dos puntas que deciden con
reglas distintas acabarán rindiéndose en instantes distintos**, aunque la fecha ya sea correcta.
Ahora que `reloj_contadorSegundos()` existe idéntico en las dos, es el momento.

Y cuatro cabos sueltos, dichos y no escondidos:

1. **Costura bajó de `37/41` a `35/41`** — también modela el fechado y **no se ha portado**. Es
   pérdida de cobertura, no un defecto nuevo.
2. **Quedan 5 escenarios de veto** en el validador *(eran 108)*. Sin revisar.
3. **El control negativo de la prueba 3.4 dejó de discriminar**: *"el banco no distingue tomar el
   mayor de tomar el menor"*. Esa prueba **ya no mide su arreglo** — se rehace o se retira, porque
   una prueba que no puede fallar no demuestra nada.
4. Y lo de siempre: **esto pasa por banco**, y cambiar la `FIRMA` obliga a **resincronizar a mano**
   cada equipo que se actualice.

### Cómo estaba antes de portar los instrumentos

La compuerta marca lo pendiente como **`ABORTADO`, nunca `FALLA`**: no ha acusado al firmware ni una
vez. Los cuatro arneses de C++ ya siguen al firmware *(pantalla de vuelta en `295/295`)*. Falta
Python:

| | estado | qué pasa |
|---|---|---|
| `validador_maestro.py` | **53/67** | los 7 de siempre + 7 nuevos del fechado viejo |
| `banco/packs/maestro_02_respaldo.py` | `ABORTADO` | sus pruebas 2.2 y 2.3 son del formato anterior |

> ⚠️ **HAY PRUEBAS QUE EXIGEN EL DEFECTO.** La 2.3 del pack afirma que una sincronización de hace
> **2 horas** debe declararse `CADUCADA` al cruzar de mes, y lo documenta como *"la dirección
> segura"*. Ahora el firmware devuelve `2`, que es lo correcto, **y la prueba falla**.
>
> **Reescribirlas en bloque para que pasen sería ajustar el instrumento hasta que dé verde**, que es
> justo lo que este repo castiga. Van una por una, y cada una acaba en uno de tres sitios:
> **se borra** *(celebraba el defecto)*, **se invierte** *(pasa a exigir lo contrario)*, o **se
> conserva** *(medía otra cosa y sigue valiendo)*. Se anota cuál y por qué.

**Y de esas 14, cuatro deberían desaparecer solas:** son las del día-del-mes (`F3`,`F4`,`F5`,`F6` de
N-46). Si tras el port siguen ahí, el arreglo no está completo — es el mejor termómetro que hay.

**T2 sigue sin empezar.** Que el Esclavo se rinda por el **mismo criterio** que el Maestro y no por
`millis()`. Sin ella, N-49 no está cerrado: la asimetría puede volver por otro lado.

### ✅ T2 HECHA Y MEDIDA — commit `98d9058`, 05/08

**Lo que se hizo:** se porta al Esclavo el mismo `msDesdeSyncEfectivo()` que ya tenía el Maestro —
la RAM (`millis() - tUltimaSync`) manda cuando existe; la pila (`respaldo_horasDesdeSync(reloj_
contadorSegundos())`) solo puede **subir** la antigüedad, nunca vetar una medida de RAM válida.
Antes, el Esclavo solo leía la pila **una vez, al arrancar** (`degradado_reanudarTrasCorte()`); el
resto de la operación normal corría puro sobre `millis()`. Enchufado en los tres sitios que
decidían con esa medida: `degradado_actualizar()` (el latch de las 48 h), `degradado_avisoLimite()`
y `degradado_msDesdeSync()` (lo que ya leía `menu.cpp` para pantalla).

**Verificado, no solo compilado:**

- Compila y enlaza: Flash Esclavo `64,2 % → 64,3 %` (+56 B, muy por debajo del umbral de revisión
  del §7).
- **Comparación con y sin el cambio** (`git stash` / `pop`, no memoria): `costura_05_limite_48h.py`
  da el mismo `5/8` en los dos casos — las 3 `FALLA` que quedan son de `main.cpp`/`coordinador.cpp`
  y de un hueco del modelo, ajenas a este fix.
- Compuerta completa sobre el commit limpio (`98d9058`, árbol `LIMPIO`): **12 PASS · 2 FALLA · 0
  ABORTADO**, idéntico al resultado antes del cambio. Sin regresión.

**Hallazgo nuevo al correr el banco completo tras T2** — reabre el cabo nº 2 de arriba, y con causa
en vez de "sin revisar": los 5 escenarios de veto de `maestro_03_puerta_degradado` **no son
residual del modelo, son una regresión real del propio arreglo de T1**. Cuando la sincronización
cae el ÚLTIMO día del mes y la consulta ya está en el día 1, `respaldo_horasDesdeSync()` sigue
devolviendo `CADUCADA` -el día bajó- y `msDesdeSyncEfectivo()` lo traduce al máximo **aunque la RAM
tenga una medida sana y reciente** (ejemplo medido: sync hace 1 minuto, día 1, hora 0 →
`efectivo = CADUCADA`). El técnico leería *"nunca hubo sincronización RF"* sobre un equipo que
sincronizó hace minutos. Pendiente de resolver antes de dar N-49 por cerrado.

> ### ❌ REFUTADO EL MISMO 05/08, CON MEDIDA — commit `c769e71`
>
> El párrafo de arriba se cayó en la misma sesión en que se escribió. Antes de arreglar nada se
> reprodujo a mano el barrido de la prueba 3.6, imprimiendo los 5 escenarios uno a uno: **los 5 eran
> el mismo caso único**, `dia_abs_sync=0, seg_sync=0`. No hay ningún cruce de mes real en los otros
> cuatro "ejemplos" citados arriba -eran la misma fila del barrido leída con distinta hora del día-.
>
> **La causa real:** `ms_desde_sync_efectivo_v2()` (en el pack y, copiado literal, en el monolito)
> le pasa a `marcar_sync()` el valor crudo `dia_abs_sync * 86400 + seg_sync`. Cuando ese producto es
> exactamente `0`, `marcar_sync()` lo rechaza -fiel al firmware- como "no hay reloj" (`respaldo_
> marcarSync()` hace lo mismo con un `segundosRtc` de `0`). Pero un contador RTC real **nunca**
> entrega ese `0` crudo a esa función: `reloj_contadorSegundos()` (`Maestro/src/reloj.cpp:222-237`,
> idéntico en el Esclavo) lo remapea a `1` ANTES de que nadie lo use, precisamente para que el
> centinela de "no hay reloj" no choque con una medida real. El barrido saltaba ese remap y le
> preguntaba al modelo por una entrada que el hardware no puede producir. **No es un hallazgo del
> firmware: es la pregunta mal hecha en el instrumento.**
>
> **Arreglo:** `_rtc_leido()`, espejo de una línea del remap del firmware, aplicado en los dos
> puntos donde `ms_desde_sync_efectivo_v2()` fabrica un valor de RTC -al marcar la sync y al leer
> "ahora"-. Mismo cambio en el pack y en el monolito, porque siguen siendo copia literal mientras
> dure la migración. Medido: `maestro_03_puerta_degradado` `14/18 → 15/18`; validador Maestro
> `62/67 → 63/67`; banco por packs `143/154 → 144/154`. La compuerta no mueve su categoría (sigue
> en `12 PASS · 2 FALLA · 0 ABORTADO`) porque las dos `FALLA` de arriba ya contaban como `FALLA`
> antes y después — mejora el detalle de adentro, no la cuenta de arriba.
>
> **La lección, otra vez la del 04/08:** un informe -propio- no es una medida hasta que se reproduce
> a mano. La frase *"regresión real del arreglo de T1"* se escribió citando la prosa del hallazgo
> del banco sin trazar el cálculo hasta el final. El cálculo tardó cinco minutos en desmentirla.

### ✅ COSTURA PORTADA — 35/41 → 37/41, commit `526f924`, 05/08

El cabo nº 1 ("costura bajó de 37/41 a 35/41, no se ha portado") **tampoco era lo que parecía**:
los 2 perdidos no eran cobertura que faltara portar, eran dos comprobaciones de `costura_05` y
`costura_06` con un patrón de texto que dejó de casar cuando T1 reescribió
`respaldo_horasDesdeSync()` -variable renombrada, rama de día-del-mes eliminada-. Se verificó cada
una por separado antes de tocarla, como enseña el cabo nº 2 de arriba:

- **`costura_05` — el hallazgo SIGUE VIGENTE, solo el patrón estaba desactualizado.** *"Un reinicio
  en UNA sola punta regala hasta 59 min 59 s de crédito"* sigue siendo cierto: el truncado a horas
  enteras no cambió con T1. El patrón buscaba `return (uint32_t)(total / 3600);`, una variable
  `total` que ya no existe; el código real es `return (segundosRtcAhora - guardado) / 3600UL;`.
  Se actualiza el patrón al texto real y el hallazgo vuelve a confirmarse: `5/8 → 6/8`.
- **`costura_06` — el hallazgo YA NO APLICA, y no por casualidad.** *"Los dos calendarios son
  independientes, y el cambio de mes no cae el mismo día en las dos puntas"* dependía de que
  `respaldo_horasDesdeSync()` comparara **días de calendario** entre las dos puntas. T1 no acotó
  ese caso: le quitó el mecanismo entero. La marca ya no es día+segundo, es
  `reloj_contadorSegundos()` -un contador monótono-, y la resta siempre es entre dos lecturas **de
  la misma unidad**, nunca cruza puntas. Que los calendarios (`rtc.setDay(1)`) sigan siendo
  independientes ya no puede afectar a la reanudación, porque la reanudación no vuelve a mirarlos.
  **Se invierte** (§8.quater: no se borra sin dejar rastro) en un `verificar()` que exige la firma
  nueva de `respaldo_horasDesdeSync(uint32_t segundosRtcAhora)` -un solo argumento, no dos- como
  guarda de regresión: si algún día la función vuelve a tomar una fecha de calendario, esto tiene
  que volver a fallar. `4/6 → 5/6`.

Mismo cambio en cada pack **y** en el monolito -copia literal mientras dure la migración-.
**Verificado que la suma cuadra** (§3.bis): los 7 packs de costura dan `3+11+6+4+6+5+2 = 37` sobre
`3+11+6+4+8+6+3 = 41`, idéntico al `37/41` del monolito.

Con esto, **N-49 solo tiene dos cabos**: la prueba 3.4 (control negativo roto) y el banco. De los
otros tres cabos que esta sesión tocó, uno era firmware de verdad (T2, el Esclavo) y los otros dos
-el veto de 5 escenarios y esta costura- eran instrumento desde el principio: ninguno de los tres
se dio por bueno sin reproducirlo, y los dos de instrumento se cerraron con medida antes de
escribir la causa, no después.

---

## 🔁 N-47 · El arnés del ciclo automático — el hueco por el que se coló la regresión ✅ **04/08**

La regresión del Modo Automático pasó con **la compuerta en verde y el arnés de pantalla en
295/295**. No fue mala suerte: **ningún instrumento ejercía el ciclo**. Los simuladores son Python
escrito a mano que *reimplementa* el C++, así que su `PASS` hablaba del modelo.

`Validacion_Automatico/` compila los **tres `.cpp` reales** —`coordinador`, `semaforo`,
`modo_automatico`— y los conduce con un Esclavo simulado y un **espía de escrituras de pin**:
**26/26**.

| lo que mide | y antes lo medía |
|---|---|
| el Modo Automático real **llega a verde y lo suelta solo**, conducido por su asistente de botones | nadie |
| SFTY-5: ámbar de 4 s exacto *(comprobado en el límite ±1 ms)*, verde→rojo directo | un espejo en Python |
| SFTY-6: orfandad a los `12000 ms` **leídos del fuente** | un espejo en Python |
| SFTY-2: rojo y verde nunca a la vez, **sobre lo que `semaforo.cpp` escribió en los pines** | la lógica, no las salidas |

Las constantes se releen del C++ por regex y el arnés **ABORTA (código 2)** si el patrón no casa,
en vez de caer a un valor por defecto.

### Verificado adversarialmente antes de conectarlo

No se dio por bueno su informe. Se comprobó por separado:

1. **No es un espejo.** En la carpeta no hay ni una copia del firmware — solo sustitutos de
   `Arduino.h`, `pines.h`, `botones.h`, `lcd.h` y `menu.h`.
2. **Sabe fallar**, con un defecto inyectado en el C++ de producción: `VERDE1` forzado a HIGH **por
   debajo** del enclavamiento de `aplicarSalidas()`. Bajó a **`25/26` con código 1**, y la
   comprobación que cayó fue justo la de SFTY-2. Firmware restaurado y `git diff HEAD` vacío.

> **Lo que este arnés NO hace:** decir cuál de los 16 firmwares del bisect rompió el Automático.
> Eso sigue necesitando la tarjeta. Lo que hace es que **no vuelva a colarse una regresión del ciclo
> con la compuerta en verde**.

**Punto ciego declarado:** solo compila el lado Maestro. *"Verde simultáneo en las dos puntas"* no
es medible por este camino; esa propiedad es de `Validacion_Ciclo`, sobre el `ciclo_degradado.h`
compartido.

---

## 🔀 N-48 · El `30/31` del Esclavo: una bandera que contestaba a dos preguntas ✅ **04/08**

El pack `esclavo_03` rompía *"el par verde+despeje es indivisible"*: con dos reconfiguraciones
separadas por **menos de 3 s** y la segunda trama de VERDE perdida, el VERDE del par **anterior**
—aún dentro de la ventana— cerraba el segundo par. El Esclavo acusaba la mezcla y la guardaba.

El defecto estaba en el **firmware**, no en el modelo: comprobado que las dos copias en Python
portaban fielmente la misma lógica. **No era un caso N-36/N-41.**

### El arreglo evidente rompía otra cosa

`cfgVerdeRecibido` mezclaba **dos hechos distintos**:

| pregunta | quién la usa |
|---|---|
| *"la radio entregó el par"* | los cuatro getters públicos, vía `cfgRadioCompleto()` |
| *"hay un VERDE sin emparejar"* | `verdeDeEsteEnvio()`, al cerrar el par |

Apagarla al cerrar el par arregla la segunda **y rompe la primera**: `cfgRadioCompleto()` se queda
en `false` para siempre, los getters caen a la pila, y se invierte la regla que el propio fichero
declara —*"la radio SIEMPRE gana cuando está completa"*—. Con el dominio de respaldo no válido, el
Esclavo diría que **no tiene ciclo** con el par recién recibido **y ya acusado**.

**Son dos banderas porque son dos hechos.** `cfgVerdePendiente` se consume al cerrar;
`cfgVerdeRecibido` no se toca. Pack **4/5 → 5/5**, monolito **30/31 → 31/31**. Flash +12 B, RAM +4 B.

> **Cómo estuvo a punto de colarse:** el arreglo llegó de un agente con *"31/31 verificado"*. Había
> cambiado a la vez el firmware **y las dos copias del modelo que debían vigilarlo**, así que las
> tres decían lo mismo y el banco no podía verlo. Se cazó **leyendo el diff**, no repitiendo la
> cifra. Ver `CLAUDE.md` §8.ter.

---

## 🚨 N-46 · La compuerta pintaba `[OK]` sobre 7 `FALLA` reales — y uno es vial ✅ **CRITERIO CORREGIDO 04/08**

> **Corregido el mismo día.** `correr_python()` cuenta ahora las líneas `[FALLA]` de la salida: si
> hay alguna y el proceso sale con `0`, la suite se marca **`FALLA`**. **O falla o no falla** —
> ninguna etiqueta de *"residual aceptado"* convierte un `FALLA` en `PASS`. Si se decide convivir
> con uno, se convive **con el acta en rojo delante**, no escondiéndolo tras un código de salida.
>
> La compuerta pasa de `13 PASS · 1 FALLA` a **`12 PASS · 2 FALLA`**, y `validador Maestro` sale
> `[ FALLA] 7 FALLA impresos y exit 0 (N-46)`.
>
> ⚠️ **Lo que este criterio NO caza, y hay que decirlo:** las **4 de costura** siguen sin
> detectarse. No imprimen `[FALLA]`, imprimen *"no concluyeron"*, así que la marca no casa. Siguen
> tapadas dentro de un `[OK] 37/41`. El defecto de fondo —**la etiqueta que se lee no es la misma
> en todos los validadores**— se cierra al retirar los monolitos, no antes.
>
> El defecto de firmware que esto destapa **sigue abierto**: es la grieta del día-del-mes, abajo.

**Medido a mano, no reportado:**

```
python 01_Firmware/Simulaciones/validador_maestro.py   -> imprime 7 [FALLA]
echo exit code                                          -> 0
compuerta.py:292  anotar(nombre, PASS if p.returncode == 0 else FALLA, cuenta)
                                                        -> [ OK ] validador Maestro  60/67
```

Es **el fallo que motivó `compuerta.py`, por el otro lado**. Aquel era `ABORTADO` tratado como
`PASS`; este es **`FALLA` tratado como `PASS`**: el validador corrió, comparó contra el C++ real,
dijo que el firmware **no cumple** — y la compuerta lo pinta en verde porque el proceso salió con 0.
Los 7 llevan meses llamándose *"residuales documentados"*, que es una etiqueta que nadie vuelve a
abrir.

### El que importa

Cuatro de los siete **no son cuatro huecos: son la misma grieta**. `respaldo_horasDesdeSync()`
(`Maestro/src/respaldo.cpp:176-204`) solo puede comparar el **día del mes** —no el mes ni el año—,
así que al cruzar de mes el número baja, sale `dias < 0` y devuelve `CADUCADA`.

El validador lo dice con estas palabras:

> `LAS DOS PUNTAS SE RINDEN EN INSTANTES DISTINTOS en 24 dias del ano. El Maestro consulta la pila`
> `(y el cruce de mes se la invalida); el Esclavo usa millis() puro con latch y NO se entera del`
> `cambio de mes. Resultado: MAESTRO EN AMBAR mientras el ESCLAVO SIGUE DANDO VERDE por reloj.`

**El Maestro falla en dirección segura** —cae a ámbar, y el comentario del código dice
explícitamente *"ante la duda, caducada"*—. **El defecto vial no es que el Maestro caduque: es que
el Esclavo no caduca igual.** La asimetría es el defecto, y es literalmente el riesgo residual nº 2
de SFTY-21: el conductor negocia el ámbar de un lado y cruza confiado el verde del otro.

Arreglarlo es un rediseño de `respaldo.cpp` —fichero que debe seguir **byte-idéntico** entre las dos
puntas— y pasa obligatoriamente por banco. No es una tarde.

### Estado de verificación de los otros

⚠️ **Solo están verificados a mano el código de salida y las cuatro del día-del-mes.** El resto
—los 3 `FALLA` restantes del Maestro y las 4 de costura— viene de una auditoría delegada y **no se
ha reproducido**. Se anota como pista, no como medida, que es la regla de la sección 4 de
`CLAUDE.md`. Lo que dice esa auditoría, sin verificar: los otros tres del Maestro serían un checksum
con 10 pares ciegos (contenido por otra puerta), un residual del protocolo dejado fallando a
propósito, y una asimetría de pantalla dormida; las 4 de costura serían **instrumento atrasado**,
todas nacidas el 01/08 al añadirse `CMD_HORA_D` y `MDG_SIN_CONFIG` — y una de ellas perseguiría un
bug **ya arreglado**.

### Lo que hay que decidir

Que una suite con `FALLA` impresos deje de salir `PASS`. La compuerta pasaría de `11 PASS · 2 FALLA`
a **3 FALLA**: se vería peor, y sería lo cierto. Mientras no se cambie, **el acta certifica en verde
un defecto de seguridad vial conocido**.

---

## 🕐 N-45 · La pantalla acusaba a la pila y al cristal sin haber medido ninguno ✅ **04/08**

**El síntoma que lo destapó:** cambiados **pila, `R5` e `Y2`**, con el hardware sano, la pantalla
seguía diciendo exactamente lo mismo al intentar poner la hora:

```
SIN RELOJ                        SIGUE PARADO
No se envio nada                 No era el estado
Revisa Y2, pila y R5             Es Y2: toca hardware
```

Esos dos textos estaban **escritos a mano** en `lcd.cpp` (líneas 376 y 416). El firmware **no había
medido ninguna de las tres cosas** — y no podía: el **STM32F103 no tiene canal de ADC para `VBAT`**
*(eso llegó con el F2/F4)*, así que la pila no es observable por software. Eran conclusiones fijas
presentadas como diagnóstico, y por eso no cambiaban al cambiar el hardware: no dependían de nada
observado. Mandaron al soldador tres veces.

> Es la **regla del instrumento incumplida dentro del producto**, no dentro del banco. Hasta ahora
> la habíamos pagado en validadores y actas; aquí la pagó el técnico con la tarjeta delante.

### Lo que no era un bug

`reloj_ajustar()` rechaza el ajuste si el oscilador no arrancó (`reloj.cpp:231`, N-24), **y hace
bien**: escribir la hora en un contador que nadie hace avanzar dejaría `horaValida = true` sobre una
mentira, y sobre esa mentira el Maestro **empujaría la hora al Esclavo y autorizaría el Modo
Degradado**. Un verde por reloj con el reloj parado. Esa guarda no se toca.

### La consulta

```
CONSULTA RELOJ                 *      <- el punto parpadea en cada repintado
LSE ON:1 RDY:0 BYP:0
RTCSEL:1 EN:1 CFG:0
CNT:1234567890 A:26
Pedido, no oscila
```

| lo que veas | lo que significa |
|---|---|
| `ON:1 RDY:0` | el oscilador **está pedido y no arranca** → ahí sí, `Y2`, sus condensadores de carga y la soldadura |
| `ON:0` | ni se está pidiendo → es firmware o el dominio de respaldo, **no el cristal** |
| `BYP:1` | espera reloj externo por `OSC32_IN`; con un cristal normal no arrancará nunca |
| `RTCSEL:0` | el RTC no está atado a ninguna fuente: no cuenta aunque el cristal oscile |
| **`CNT` cambiando** | **el RTC cuenta.** Distingue *"no cuenta"* de *"cuenta y nadie lo ha puesto en hora"* |

**El contador se relee cada 500 ms**, porque una sola lectura no distingue parado de avanzando. Y
el **punto de latido** existe para la ambigüedad que quedaba: sin él, un contador detenido y una
pantalla congelada se ven idénticos. Si el punto parpadea y `CNT` no cambia, el RTC está parado de
verdad.

No se va sola a los 4 s como los demás resultados —son cuatro filas de números que se apuntan desde
los 5 m del gabinete—; sale con cualquier botón. El coordinador se sigue moviendo mientras tanto,
así que el Esclavo no ve orfandad y las luces siguen en el Rojo Fijo del menú.

### Y el arnés no la echaba de menos

Al escribirla, **el arnés seguía dando `115/115`: el mismo número de antes.** Sus anchos se habían
contado a mano — el error exacto que la regla del instrumento describe, y que en este mismo fichero
ya se había cometido con *"El esclavo no contesto"*. Cubierta con **6 casos**, incluidos el contador
de 10 dígitos y las dos pistas de 20 caracteres: **Maestro 115/115 → 133/133**, total **241 → 259**.

**Flash del Maestro:** 55.496 → 56.584 B *(84,7 % → 86,3 %, +1,6 %)*, por debajo del ~2 % que obliga
a revisar. Quedan 8.952 B.

---

## 🟢 Hitología de Versiones Completadas

- [x] **V7.0:** Menú interactivo en pantalla LCD ST7920 y navegación con 4 botones.
- [x] **V7.1:** Transmisión en ráfaga 3x (Burst) e integración del Watchdog IWDG. *(Revisados en V8.0: el watchdog pasa a la API `IWatchdog` y queda activo. La ráfaga se bajó a 1 copia y volvió a 3 en V8.1, al comprobarse que su coste dependía de la tasa aérea, no del protocolo.)*
- [x] **V7.2:** Ajuste a 0.3 kbps de tasa aérea con Heartbeat PING a 3.0s. *(La tasa de 0.3 kbps queda **derogada** en V8.0 — ver N-1.)*
- [x] **V7.3:** Puente repetidor ESP32 con passthrough asíncrono entre dos radios.
- [x] **V7.4:** Puerto UART AiBus para cámara YOLOv8 Edge (`AI_CARS:XX`).
- [x] **V7.5:** Self-Healing autónomo sin reinicio manual y ventana deslizante `memmove` con CRC-8 Maxim.
- [x] **V7.6:** Botón 3 en Modo Manual fija ambos semáforos en 🔴 ROJO FIJO INDEFINIDO. Menú independiente de la radio del Esclavo.
- [x] **V8.0 Definitiva (31 de Julio de 2026) — Auditoría independiente y correcciones de seguridad.**
- [x] **V8.1:** Telemetría de calidad de enlace (SFTY-14) y pantalla **PRUEBA ALCANCE**.
- [x] **V8.3:** Diagnóstico de línea en pantalla (SFTY-15) y puente que valida antes de retransmitir (SFTY-16).
- [x] **V8.4:** Retardo de cortesía del Esclavo antes de responder (SFTY-17).
- [x] **V8.5 (1 de Agosto de 2026):** Reloj de tiempo real (SFTY-18) y auditoría del repositorio.

> **Sobre el hueco de la V8.2.** No falta nada: el trabajo de SFTY-15 se **commiteó** como `v8.2`
> (`0cfe612`) y luego se **documentó** como V8.3, sin renumerar el commit. La numeración válida es la
> de este roadmap; el mensaje de aquel commit es la única huella de la etiqueta anterior.

---

## 🛡️ V8.0 — Correcciones de la auditoría independiente

### Seguridad vial

- [x] **H-1 — Esclavo atrapado en Verde ante fallo asimétrico de enlace.** El Maestro emite `CMD_GO_RED` (nunca `PING`) mientras está en `C_FALLO`; su propio heartbeat ya no suprime el fallback de orfandad del Esclavo. Backstop de verde máximo en el Esclavo, fijado por encima del máximo configurable.
- [x] **H-2 — Despeje All-Red configurable a 0s.** Piso inquebrantable de 5s. Fases Rojo/Verde en Modo Automático con mínimo de 1 minuto.
- [x] **H-3 — Watchdog IWDG desactivado.** `IWatchdog.begin(4000000)` + `reload()` activos en Maestro y Esclavo. *(El Repetidor ESP32 sigue sin watchdog.)*
- [x] **Parpadeo ámbar en Menú.** `semaforo_iniciarFallo()` se llamaba sin guarda en cada iteración, reiniciando el temporizador de 500ms; el Maestro nunca llegaba a parpadear. Corresponde a los tests de campo 3 y 4 del funcional.

### Comunicaciones

- [x] **N-1 — Saturación del canal a 0.3 kbps (causa raíz de las caídas al paso de ciclo).** Tasa aérea elevada a **2.4 kbps**, `RF_BURST_COPIES` reducido de 3 a 1 y `TIMEOUT_ACK_MS` elevado de 3500 a 8000 ms.
- [x] **H-4 — `msgID` podía valer 0** y se descartaba como duplicado (1 de cada 256 comandos). `if (msgIdCounter == 0) msgIdCounter = 1;`.
- [x] **H-6 — ACK duplicado del Esclavo** (2 ráfagas por orden). Eliminado el auto-envío redundante de `ACK_RED`.

### Verificación

- [x] **H-5 — El simulador no ejercitaba el C++.** `PRUEBA 8` parsea `protocolo.h` real, y el modelo lee `RF_BURST_COPIES` y `TIMEOUT_ACK_MS` del firmware en cada ejecución para no volver a divergir.
- [x] **Banco de pruebas:** las llamadas a `pedir_cambio()` descartaban la trama devuelta, de modo que el `GO_RED` nunca se transmitía y el ciclo solo arrancaba al vencer el reintento. Corregido; el test es ahora más estricto.
- [x] Compilación 3/3 SUCCESS y simulador **9/9 PASS** desde cualquier directorio de trabajo.

---

## 📶 V8.1 — Telemetría de enlace y prueba de alcance (31 de Julio de 2026)

Añadido **después** de que el campo validara la V8.0 en los tres modos sin una sola caída.

### Instrumento de campo
- [x] **Pantalla `PRUEBA ALCANCE`** (4.ª opción del menú): calidad de enlace en %, barra gráfica, tiempo de respuesta y fallos consecutivos. El operario desplaza el equipo y ve degradarse el número hasta perder el enlace, en vez de estimar la cobertura a ojo.
- [x] **Línea `RF:100% 340ms`** en los modos Automático, Inteligente y Manual.
- [x] **SFTY-14:** la telemetría se deriva del latido de 3 s ya existente. Sin cambios de protocolo ni soporte de la radio.

### Ajustes de protocolo revertidos tras la validación de campo
- [x] `TIMEOUT_ACK_MS` de vuelta a **3500 ms** (el valor que el campo validó), en lugar de 8000. Con 3.5 s caben 4 intentos dentro de la ventana de seguridad de 12 s; con 8 s solo 2.
- [x] `RF_BURST_COPIES` de vuelta a **3 copias**. A 2.4 kbps cuestan 0,13 s de aire y son la palanca correcta frente a distancias variables.

### Verificación
- [x] **Arnés de validación de pantalla en PC** (`01_Firmware/Validacion_LCD/`): compila el **mismo `lcd.cpp` y `menu.cpp`** del firmware contra un framebuffer en memoria y comprueba automáticamente que nada se salga de los 128×64. Requiere GCC (MinGW-w64). *(Ampliado a 30 comprobaciones en V8.3.)*
- [x] **Simulador de escenarios de repetidor** (`simulador_repetidor.py`): latencia por salto de aire, pérdida por trama, interferencia co-ubicada, corte y recuperación del puente, y barrido de pérdida. **8/8.**
- [x] Corregido un defecto propio: la 4.ª opción del menú caía en `y = 72`, **fuera de la pantalla de 64 px**. Interlineado reducido a 11 px.
- [x] Corregido en la telemetría: cerraba el latido con cualquier paquete recibido, lo que contaba como exitoso un latido perdido.

> **Regla de validación adoptada:** todo instrumento de prueba debe reconocer primero como correcto lo que **ya funciona en campo**. Si no lo hace, el equivocado es el instrumento. Aplicarla evitó dos veces "corregir" código sano guiándose por una medición rota.

---

## 🩺 V8.3 — Diagnóstico de línea en pantalla (31 de Julio de 2026)

Nace de un fallo real de campo: con el repetidor interpuesto, el sistema decía "fallo de
comunicación" y **había que ir al poste con LEDs, portátil y destornillador** para saber si no
llegaba nada o llegaba basura. El Maestro tenía el dato y lo tiraba.

- [x] **SFTY-15:** `protocolo.cpp` cuenta bytes recibidos, tramas válidas y descartadas por CRC.
- [x] La fila inferior de **PRUEBA ALCANCE** separa los tres casos:

| En pantalla | Significado | Dónde mirar |
|---|---|---|
| `RX 0 - nada llega` | No entra ni un byte | Cobertura, canal, antena |
| `RX 4k - BASURA` | Entran bytes, ninguna trama válida | Cableado, línea flotando, radio atascada |
| `RX 36  9 tr` | Enlace correcto | — |

- [x] Contadores a cero al entrar a la pantalla: lo que se ve corresponde a **esa** medición.
- [x] **Refresco por cambio de dato**, no por temporizador. Antes redibujaba cada 500 ms — seis veces la misma imagen entre latidos, bloqueando el bucle en cada volcado SPI de 1 KB. Ahora ~1 volcado cada 3 s.

### Dos desbordes detectados por el arnés antes de flashear
- Con 999.999 bytes, la fila medía **110 px** e invadía el `4=Menu`. Corregido abreviando (`999k`).
- Tras abreviar, el **caso normal** con dos contadores largos seguía en **100 px**. Corregido acortando la etiqueta a `tr`.

> U8g2 recorta fuera de pantalla **en silencio**, sin error. Ambos habrían llegado a la tarjeta sin que nadie lo notara hasta tenerla en la mano.

### 🌉 SFTY-16 — El puente valida antes de retransmitir

Corrección del fallo de campo del 31/07: con el repetidor interpuesto, el LED TX de B2 quedaba
**encendido fijo** y el enlace era inservible.

**Causa:** el ESP32 era un passthrough ciego. Activaba la transmisión ante *cualquier* byte y la
cerraba tras 5 ms de silencio. Con el par RS485 de entrada flotando (falta de resistencias de
polarización), el receptor lee ruido continuo, **el silencio nunca llega**, y la radio de salida
queda radiando basura de forma permanente, saturando el canal.

**Corrección:** el puente reconoce el formato (4 bytes + CRC-8 Maxim) y **solo relaya tramas
válidas**. El ruido muere dentro del ESP32.

Reproducido y verificado en simulación con ruido de línea realista (960 B/s, lo que da una UART
a 9600 baudios sobre un par flotando):

| | Puente ciego | Puente validador |
|---|---|---|
| Ruido radiado al aire | **94.656 bytes** | **0** |
| Perdido por saturación del canal | **80.400 bytes** | 0 |
| Relevo del ciclo | **NO → `C_FALLO`** | **SÍ** |

> `-D PUENTE_TRANSPARENTE` revierte al comportamiento anterior para poder comparar.

### 🔌 El bus RS-485 es half-duplex: el `DE/RE` lo es todo

Corolario del mismo fallo, y el menos evidente de los dos síntomas.

RS-485 usa **un solo par de hilos para ambos sentidos**: mientras un extremo excita la línea, el
otro **no puede hablar**. Con `M2_DE_RE` clavado en alto, el ESP32 ocupaba ese bus el 100% del
tiempo, así que **B2 no podía devolverle nada** — la respuesta del Esclavo llegaba a B2 y moría ahí.

*No es que el Esclavo callara: es que su respuesta no tenía por dónde entrar.* Eso explica el
"sin datos de vuelta" reportado en campo, que parecía un problema distinto del TX atascado.

Desde V8.3 el `DE/RE` se levanta **solo durante los ~4 ms que dura emitir una trama válida**. El
camino de vuelta queda libre el 99,9% del tiempo. Documentado en `05_Funcional/5_Manual_Puente_ESP32.md §3.0`.

### ⏱️ SFTY-17 (V8.4) — El Esclavo espera antes de contestar

Tercer hallazgo del mismo día de campo, y el que faltaba para cerrar el retorno.

**Síntoma:** con repetidor, la ida fluía pero **no volvía nada**. El contador del puente marcaba
`C<-Esclavo = 1 byte` en dos minutos, mientras `A<-Maestro` recibía sin parar.

**Causa:** en enlace directo hay **dos** conmutaciones de radio; con repetidor hay **cuatro**, y una
no estaba contada. B2 acaba de **transmitir** la orden hacia el Esclavo y necesita tiempo para volver
a **recepción**. El Esclavo contestaba a los ~2 ms, con B2 todavía conmutando: **su respuesta salía
al aire cuando nadie la escuchaba.**

**Corrección:** `RETARDO_RESPUESTA_MS = 200`. La respuesta se **programa**, no se bloquea el bucle,
de modo que el parpadeo de ámbar y el watchdog siguen atendidos con normalidad.

- En enlace **directo** es inofensivo: el Maestro espera hasta 3.500 ms.
- **El Maestro no necesita el mismo ajuste**: nunca contesta de inmediato, solo inicia y espera. Su
  única rama de respuesta (`PONG` a un `PING`) está muerta, porque el Esclavo nunca envía `PING`.
- El retardo del Esclavo **también** le da tiempo a la radio del Maestro para darse la vuelta.

El simulador modela ahora ese retardo, leyendo la constante del propio C++ para no divergir.

### 📢 El firmware de producción también informa

El informe de contadores solo existía en `repetidor_diag`, pero `pio run -t upload` sube el entorno
**por defecto** (producción, mudo). En campo se cargó producción, la consola mostró solo el arranque
de la ROM y se concluyó "no hay datos de flujo" — cuando nunca se había cargado el firmware que
informa. **Horas perdidas por una trampa del empaquetado, no del código.**

Ahora el encabezado y el informe cada 2 s van **siempre**. Si tras el arranque de la ROM no aparece
el encabezado, el firmware no está corriendo: es la primera comprobación en campo y es inequívoca.

### Verificación
Validación de pantalla **30/30** · Simulador funcional **9/9** · Simulador de repetidor **10/10** ·
Maestro 42.984 B (65,6%) · Esclavo 15.480 B (23,6%) · Repetidor 270.497 B (20,6%), sin warnings propios.

*Cifras vueltas a medir el 01/08/2026 recompilando los tres proyectos; las anteriores (42.620 /
15.416 / 269.357 B) eran previas al módulo del reloj y quedaron desfasadas. Las tres suites se
reejecutaron y siguen dando el mismo resultado.*

### Estado de despliegue
**Actualizado el 01/08/2026.** El funcional confirma operación correcta en campo con dos radios en
enlace directo. Lo que **sí** queda validado por esa prueba: el ciclo semafórico completo, el enlace
RF, los reintentos y el comportamiento sin comunicación.

Lo que **no** queda validado, y conviene no confundir:

- **SFTY-16 y SFTY-17** — se diseñaron para el camino con repetidor, que hoy no está en uso (N-11).
- **Telemetría (SFTY-14) y contadores de línea (SFTY-15)** — compilados y revisados, pero **sin
  prueba de banco** (checklist 5.6/5.7). Ninguna suite ejecuta la lógica del coordinador: solo se le
  inyectan valores para dibujar la pantalla.
- **SFTY-18** — el RTC no se ha contrastado contra hora patrón ni se ha comprobado que conserve la
  hora tras desconectar la alimentación.

### 📍 Versión que corre en campo

| | |
|---|---|
| **Versión** | **V8.4** |
| **Commit** | **`e303485`** — *"fix(esclavo): SFTY-17 retardo de cortesía antes de responder"* |
| **Fecha** | 31/07/2026 16:25 |
| **Confirmado por** | el funcional, el 01/08/2026 |

Es el último commit con cambios de firmware antes de esa prueba; lo que vino después hasta las 18:43
fue documentación. `e303485` es ancestro de la rama actual, así que la línea de trabajo es continua y
no hay bifurcación que reconciliar.

**Diferencia entre lo que corre en campo y `main` hoy:** comentarios, más el módulo del reloj
(inerte hasta `c6bce52`; **ya no lo es en la rama `feat/n15-reloj-pantalla-hora`** — ver el aviso de
la sección V8.5). El comportamiento es el mismo **solo en `main`**.

> ⚠️ **Al haber reescrito el historial, el hash `71e8904` que aparece en registros antiguos es el
> mismo trabajo con identidad previa.** El válido es `e303485`.

---

## 🕐 V8.5 — Reloj de tiempo real (31 de Julio de 2026)

Con la pila CR2032 soldada en `VBAT` y `R5` retirado, el Maestro ya tiene hora propia.

### Lo que se construyó

`reloj.cpp` / `reloj.h` sobre el RTC interno del STM32 con el cristal `Y2` de 32.768 kHz que la
tarjeta **ya traía en el diseño**. No ocupa ningún pin: el I²C por hardware está copado (LCD en
`PB6`/`PB7`, RS-485 en `PB10`/`PB11`), así que un módulo `DS3231` externo habría obligado a I²C por
software. Coste: **+364 B de flash** (65,0% → 65,6%).

### La decisión de diseño que importa

**El módulo declara la hora no fiable por defecto.** Al ajustarla escribe un año marcador; al
arrancar, `reloj_enHora()` solo devuelve `true` si ese marcador sobrevivió en el dominio de respaldo.
Pila agotada, equipo nuevo o arranque sucio ⇒ **`false`**, y quien dependa de la hora debe abstenerse.

Un reloj que se cree válido sin serlo es peor que no tener reloj: activaría la operación nocturna a
deshora. Ver **SFTY-18**.

### Dos correcciones durante el desarrollo

- **`platformio.ini` con dos claves `build_flags`.** La segunda **pisa** a la primera —
  no se suman — y se habría perdido `-D ROL_MAESTRO`, compilando el Maestro como si no lo fuera.
  Unificado en una sola clave con una nota que lo advierte.
- **`-D HAL_RTC_MODULE_ENABLED` era redundante:** el core de STM32duino ya lo define. Producía 37
  avisos de redefinición. Retirado.

### ¿Es seguro flashear esto sobre el equipo que ya funciona en campo?

**Sí, y está comprobado, no supuesto.** La duda es legítima: añadir una librería a un firmware que
está operando merece verificación, no confianza.

Diferencia real entre la versión validada en campo (`3cbd012`) y la actual:

| Archivo | Qué cambió |
|---|---|
| `protocolo.cpp` (Maestro y Esclavo) · `coordinador.cpp` · `Repetidor/main.cpp` | **Solo comentarios.** Cero líneas de código |
| `modo_alcance.cpp/h` | Aparecen como nuevos únicamente porque el `.gitignore` roto los ocultaba. Código idéntico |
| `reloj.cpp/h` | Nuevo, **pero no lo llama nadie** |

El único riesgo era el inicializador estático `static STM32RTC &rtc = STM32RTC::getInstance();`, que
se ejecuta **antes de `setup()`** aunque nada use el módulo. Revisado el fuente de la librería:

- El constructor solo llama a `setClockSource()`, que **asigna variables en RAM** — ni un registro.
- El hardware se configura en `RTC_init()`, invocado **únicamente desde `begin()`**.
- `begin()` está en `reloj_setup()`, **y `reloj_setup()` no lo llama ningún archivo.**

**Conclusión: el módulo del reloj es inerte.** El comportamiento es idéntico al validado en campo;
los 364 B son código muerto a la espera de tener consumidor.

> ### ⚠️ ESTE ANÁLISIS CADUCÓ EL 01/08/2026
>
> Válido solo hasta el commit `c6bce52`. **En la rama `feat/n15-reloj-pantalla-hora` ya no lo es:**
> `main.cpp` llama a `reloj_setup()`, existe `MODO_HORA` y el menú pasó de 4 a 5 opciones.
>
> **Lo que hay en esa rama NO es de comportamiento idéntico a la V8.4 de campo.** Antes de flashearla
> hay que revalidar, no reutilizar esta conclusión. En particular está pendiente la prueba de banco
> **N-17**: arranque con el cristal `Y2` desconectado.

### Alcance real: para qué sirve y para qué no

| Función | ¿Necesita RTC? | ¿Necesita pila? |
|---|---|---|
| **Operación autónoma al perder el radio** (N-9 / SFTY-19) | ❌ No | ❌ No |
| **Operación intermitente nocturna** (N-3, aplazado) | ✅ Sí | ✅ Sí |
| Hora en pantalla y registros | ✅ Sí | ✅ Sí |

**La pila del Maestro se queda** — es lo que hace que la hora sobreviva a un corte de energía; sin
ella habría que reponerla a mano tras cada apagón. **La tarjeta del Esclavo no necesita pila:** el
Esclavo no necesita saber la hora.

### Verificación

Compila **sin warnings propios** (queda uno de la librería `STM32duino RTC`, ajeno al proyecto).
Maestro 42.984 B (65,6%), RAM 17,8%. **Sin validación de banco:** el RTC no se ha contrastado contra
una hora patrón ni se ha comprobado que conserve la hora tras desconectar la alimentación.

### Lo que NO se hizo

No hay pantalla de ajuste de hora, ni entrada de menú, ni lógica que consuma el reloj. El módulo está
disponible pero **ningún otro archivo lo llama todavía**. La quinta opción de menú tampoco se añadió:
con el interlineado actual caería en `y = 72`, **fuera de la pantalla de 64 px** — el mismo defecto
que el arnés ya cazó una vez. Habrá que reducir a 9 px y revalidar en `Validacion_LCD`.

---

## 🔐 Auditoría del repositorio del 01/08/2026

Dos fallos que no son de firmware pero comprometían el proyecto.

### 1. `.gitignore` en UTF-16 — el repositorio publicado no compilaba

Git lee `.gitignore` byte a byte. Guardado en UTF-16, la primera línea `*.zip` se interpretaba como
el patrón `*`: **el proyecto entero quedaba ignorado.** Los archivos ya versionados seguían apareciendo
como modificados, así que el fallo era invisible en el trabajo diario, pero **ningún archivo nuevo
llegaba nunca al repositorio.**

Nueve archivos llevaban commits sin subir, entre ellos `modo_alcance.cpp` — **la pantalla PRUEBA
ALCANCE completa** — y las dos suites que este roadmap cita como evidencia. Lo publicado en GitHub
**no compilaba**: el Maestro invoca `modo_alcance` y ese archivo no estaba.

Reescrito en UTF-8. Comprobados **los 34 `.gitignore`** del proyecto: el resto estaban bien.

> **Regla:** tras añadir archivos nuevos, verificar que aparecen en `git status`. Un árbol
> sospechosamente limpio es un síntoma, no un logro.

### 2. Contraseña de la cámara expuesta en repositorio público

La credencial de la cámara Hikvision estaba escrita en los scripts de conteo, y el repositorio era
**público**: quedó expuesta del **28/07 al 01/08**, cuatro días.

Retirada del código —ahora se lee de `CAMARA_PASSWORD` y el script se niega a arrancar sin ella— y
limpiadas las ocho copias bajo `99_Legacy`. El repositorio se pasó a privado.

> ⚠️ **Sigue en cinco commits del historial.** Pasar el repositorio a privado detiene la exposición
> pero no recupera lo ya publicado: los repositorios públicos se rastrean de forma automática en
> busca de credenciales. **La única medida efectiva es cambiar la contraseña en la cámara**, y en
> cualquier equipo donde se haya reutilizado. Pendiente de confirmación.

### 3. Peso del repositorio

Retirados del versionado 127 MB en binarios que el repositorio no necesita: el
`Manual_de_Senalizacion_Vial.pdf` (97,87 MB, a 2 MB del límite duro de 100 MB por archivo de GitHub)
y dos `.rar` de entregables de 14,64 MB. **Siguen en disco**, solo dejan de versionarse. Se conservan
los dos manuales E90-DTU (3 MB) porque el funcional los necesita para configurar las radios.

---

## 🔧 Hallazgos de campo del 31/07/2026 que NO son de firmware

Tres causas físicas que costaron el día y quedaron documentadas para que no se repitan.

### 1. DIP switches `M0`/`M1` en modo incorrecto

Las cuatro radios quedaron en `M0=ON, M1=ON` — un modo especial donde **la transmisión puede quedar
deshabilitada mientras la recepción sigue activa**. Síntoma: radios que oyen pero no contestan.

**Origen del error: el propio manual.** `MANUAL_EXACTO_RADIOS_E90_DTU.md` indicaba `ON/ON` para el
modo de configuración; el datasheet del fabricante dice `M0=ON, M1=OFF`. **Ya corregido en los dos
manuales.**

> **Regla: en operación, M0 y M1 van SIEMPRE los dos en OFF.**

### 2. El puente por cable directo entre las dos radios no funciona

Se probó unir B1 y B2 bornera con bornera para prescindir del ESP32. **No es viable**: RS-485 usa un
solo par para ambos sentidos y las radios conmutan la dirección **cada una por su cuenta, sin saber
de la otra**. No hay árbitro, no hay turnos: una acapara la línea y la otra queda muda.

**El ESP32 existe justamente para arbitrar.** Documentado en `05_Funcional/5_Manual_Puente_ESP32.md §3.2`.

### 3. Transmisor de la radio B1 averiado — y cómo se identificó

Síntoma: B1 recibía correctamente pero **nadie oía sus transmisiones**.

**Método que lo resolvió — prueba de intercambio de frecuencias:** se intercambiaron las bandas
asignadas a cada extremo del repetidor y se observó de nuevo. El fallo **siguió a la misma radio**
pese a cambiar de banda y de destino: primero no la oía el Maestro, luego tampoco el Esclavo.
**Factor común identificado en una sola prueba, sin instrumentos.**

Un receptor capta algo aunque su antena esté floja; **un transmisor con la antena en mal estado no
radia casi nada**. De ahí el síntoma engañoso de "el LED dice que transmite pero nadie lo oye".

> ⚠️ **Transmitir con la antena en mal estado daña el amplificador.** Al montar el reemplazo hay que
> revisar primero el coaxial y el conector, o se quema también el radio nuevo.

Documentado en `01_Firmware/TROUBLESHOOTING.md` y `05_Funcional/5_Manual_Puente_ESP32.md §3.3`.

> ### ✅ CONFIRMADO EL 01/08/2026
>
> Retirada la B1 del montaje y **el sistema funciona con dos radios en enlace directo**. El
> diagnóstico era correcto: **la avería estaba en esa radio, no en el firmware.**
>
> **Lo que esto cierra:** el fallo de comunicación que se arrastraba desde el 31/07 queda explicado
> por completo entre las tres causas físicas de esta sección — DIP switches, tasa aérea y la radio
> averiada. **Ningún cambio de firmware fue necesario para resolverlo.**
>
> **Lo que esto NO valida:** el sistema opera hoy **sin repetidor**, así que **SFTY-16** (puente que
> valida antes de retransmitir) y **SFTY-17** (retardo de cortesía del Esclavo) **siguen sin
> comprobarse en campo** — se diseñaron precisamente para el camino de 4 radios que ahora no está en
> uso. SFTY-17 es inofensivo en enlace directo: el Maestro espera hasta 3.500 ms.

### 4. Antenas fuera de banda — alcance de 3 cuadras

Medido en campo: **1 cuadra, esquina y 2 cuadras más**. Para 1 W a 170 MHz es muy poco.

Causa probable: antenas genéricas de "LoRa" (433/915 MHz) montadas en radios de 170 MHz — **15 a
20 dB de pérdida**. La banda correcta es **VHF comercial 136–174 MHz**, y hay mercado profesional
para ella en Colombia. Selección de antenas, modelos, conectores y advertencias en
`05_Funcional/2_Manual_Hardware_y_Pruebas.md §2.1`.

---

## 🔥 PRIMERA PRUEBA DE BANCO — N-17 confirmado (1 de Agosto de 2026)

**El fallo que se venía anticipando desde que se soldó la pila apareció al primer flasheo, en las dos
tarjetas.** No lo encontró ninguna simulación: lo encontró el hardware.

### El síntoma, que no parecía de reloj

| | Lo que se veía |
|---|---|
| **Maestro** | **En bucle** en la pantalla de bienvenida. Sin luces, sin responder a los botones |
| **Esclavo** | **Pantalla en blanco** — pero **las luces funcionaban** |

Dos síntomas que no se parecen en nada: uno se lee como un problema de arranque y el otro como un
problema de pantalla. **La causa era la misma línea en las dos.**

### La causa

`rtc.begin()` con LSE **espera al oscilador sin límite**. El cristal `Y2` de 32.768 kHz no arranca —lo
que `MAPEO_TARJETA_KICAD.md §4` ya advertía para microcontroladores clonados— y el arranque se queda
ahí para siempre.

**Por qué dos síntomas distintos:**

```
   MAESTRO                          ESCLAVO
   lcd_setup() + bienvenida         semaforo_setup()   -> las luces YA funcionan
   IWatchdog.begin(4s)              IWatchdog.begin(4s)
   reloj_setup()   <- CUELGA        reloj_setup()      <- CUELGA
   menu_setup()    <- nunca         lcd_setup()        <- nunca -> PANTALLA EN BLANCO
   -> BUCLE EN LA BIENVENIDA
```

### El watchdog hizo su papel, y aun así no bastó

Moverlo por delante de `reloj_setup()` convirtió **un cuelgue mudo con las luces apagadas en un
reinicio visible y diagnosticable**, que era exactamente para lo que se movió. Pero el equipo seguía
sin arrancar.

> **Detectar el fallo no es sobrevivirlo.** El watchdog compró diagnóstico, no disponibilidad.

### La corrección

El oscilador se arranca a mano con **espera acotada a 2 s** —por debajo de los 4 s del watchdog— y
solo si responde se llama a `rtc.begin()`. Si no arranca, **el equipo continúa sin reloj**:
`reloj_enHora()` devuelve `false` y todo lo que dependa de la hora se abstiene.

Es lo que el comentario de `main.cpp` ya decía horas antes de que hiciera falta: **un semáforo no
puede depender de un cristal de reloj para encender.**

### Consecuencia práctica

**El Modo Degradado y la operación nocturna no se podrán usar en estas tarjetas** mientras el cristal
no oscile. El equipo funciona en todos los demás modos. Si hace falta el reloj, las salidas son
revisar el condensador de carga de `Y2` o el módulo `DS3231` externo por I²C software (`PB0`/`PB8`,
los únicos pines libres) que `MAPEO_TARJETA_KICAD.md §4` ya documenta.

---

## 🔬 Ronda de validación por agentes (1 de Agosto de 2026)

Tres validadores independientes —Maestro, Esclavo y **costura entre ambos**— sobre el firmware ya
construido. **16 desajustes encontrados.** Lo que sigue es el resumen; el detalle está en el commit
`9489e29`.

### El hallazgo con más valor: una corrección mía creó un fallo peor

Al cerrar el desbordamiento de `millis()` hice que la marca de la pila **vetara** una medida de RAM
válida. El validador demostró que eso reproducía **exactamente el escenario que N-20 existe para
evitar**:

```
   En Modo Degradado el Maestro CALLA en la radio
        -> la marca de la pila no se refresca nunca
        -> al cruzar fin de mes: CADUCADA
        -> Maestro a AMBAR con la sincronizacion a UNA HORA de antiguedad
        -> Esclavo, que cuenta con millis() puro, sigue dando VERDE

   24 dias al ano. Ambar contra verde.
```

**Corregido:** la pila solo puede **subir** la antigüedad, nunca vetar. `CADUCADA` significa *"no se
puede fechar"*, no *"es viejo"*.

> **La lección, que vale más que el arreglo:** un validador que solo confirma lo que ya crees no sirve.
> Éste encontró que la corrección era peor que el defecto, y lo dijo.

### Los otros tres críticos

| Defecto | Consecuencia en la calle |
|---|---|
| **El par de configuración no era indivisible** | Las banderas eran *pegajosas*: comprobaban que ambas tramas llegaron **alguna vez**, no que fueran del **mismo par**. Cambias el ciclo, se pierde una trama, el Esclavo queda con la mezcla **y la acusa**. Medido: Maestro 140 s de ciclo contra Esclavo 110 s, **primer solape verde-verde en el segundo 165 del día** |
| **El cabezal podía quedarse a oscuras** | En el asistente de Automático nadie refrescaba la máquina de luces. Una secuencia del mando **apagaba las seis salidas indefinidamente**, con el mando sordo y **sin recuperación desde el suelo** |
| **`pendConfig` no se reencolaba nunca** | Cambias el Esclavo o su pila: el Maestro reenvía la hora pero **nunca el ciclo**. El Maestro acepta y da verde mientras el Esclavo rechaza y cae a ámbar |

### La suma de comprobación necesitó tres versiones

| Versión | Punto ciego |
|---|---|
| Suma llana | Las **9** transposiciones pasaban sin detectarse |
| Pesos 1, 3, 5, 7, 11 | Quedaban **4**, y una explotable |
| **Acumulador multiplicativo** | Depende del orden **por construcción**, no por aritmética de pesos |

El caso que delató la segunda: `FLAGS = 7` con `SYNC_SEG = 32775` —una sincronización a las 18:12:30,
hora corriente— permutados dejaban la suma **intacta** y producían los tres indicadores encendidos.
Un arranque tras corte lo habría leído como **autorización vigente**.

### El arnés de pantalla pasa de 83 a 209 comprobaciones

Las **cinco vistas del Esclavo** entran por fin en la validación permanente, con su propio binario.
Antes se habían validado con un arnés temporal que se desechó — y una validación de usar y tirar
comprueba **el momento, no el proyecto**.

> **Hallazgo metodológico:** el criterio del arnés viejo para detectar recorte por la derecha **no
> podía fallar**. Con fuente 6×10 desde `x = 2`, el carácter 22 empieza en `x = 128` y U8g2 **no pinta
> ni un píxel**, así que 21 y 40 caracteres daban la misma imagen. El nuevo usa un criterio que
> discrimina y **lleva autocomprobación**.

### Respuesta cuantificada a la pregunta abierta del límite de 48 h

**Caen escalonadas, no a la vez: 216 ms** —200 ms de cortesía SFTY-17 más 16 ms de aire—. El Esclavo
cae primero, porque marca al *aplicar* la hora y el Maestro al *recibir* el acuse.

**Ese hueco es inofensivo, y está demostrado**: ambas pasan por todo-rojo y los todo-rojos se
solapan. Barriendo el ciclo completo salen **cero** segundos de verde simultáneo.

**El hueco peligroso es otro:** el de las puertas. Con el camino de vuelta averiado antes de entrar,
hasta **26 minutos de verde en el Esclavo contra ámbar en el Maestro**.

### Queda abierto, documentado y sin corregir

- ~~**Los dos calendarios son independientes.**~~ 🟡 **EN CURSO.** Se cierra añadiendo `CMD_HORA_D`
  (`0x10`) al principio de la secuencia de sincronización: la fecha viaja con la hora y las cuatro
  tramas se aplican juntas o ninguna. **No interesa la fecha real** —el equipo no la muestra ni la
  usa para nada más— sino que las dos puntas cuenten los días con **el mismo número**. Con
  calendarios acoplados, un corte en el cambio de mes hace fallar a **las dos a la vez**: simétrico y
  seguro. Desacoplados dejaba una en ámbar y la otra en verde
- El límite duro conserva **una sola capa** contra el desbordamiento, no dos
- La pantalla del Maestro no tiene el enclavamiento que sí tiene la del Esclavo
- El alias de ±30 s es **inherente al protocolo**; lo contiene la frescura de 2 h
- En Degradado el Maestro salta directo a verde y el Esclavo pasa por 4 s de ámbar: **30 s de verde
  útil contra 26 s**
- Los motivos de rechazo **no coinciden entre puntas**: el manual necesita dos tablas

---

## 🎯 Orden de trabajo propuesto (1 de Agosto de 2026)

Con el sistema ya operando en campo, cambia la prioridad: deja de ser "hacer que funcione" y pasa a
ser **consolidar lo que funciona antes de añadir nada**.

El criterio de ordenación es **cuánto le duele hoy al proyecto**, no cuánto cuesta ni cuán interesante
es de construir.

> ### 🔄 Replanificado el 01/08/2026 — dos hechos nuevos del hardware
>
> **1. Pila CR2032 instalada en AMBAS tarjetas.** El Esclavo también puede tener hora propia.
>
> **2. Pantalla LCD instalada en AMBOS controladores.** Antes solo el Maestro tenía interfaz.
>
> **Pero el firmware del Esclavo no las usa.** Tiene `lcd.h`, `botones.h`, `menu.h` y `coordinador.h`
> en `include/`, **copiados del Maestro y sin ningún `.cpp` que los implemente**: su `src/` solo
> contiene `main.cpp`, `protocolo.cpp` y `semaforo.cpp`. En firmware, **el Esclavo hoy no tiene
> pantalla, ni botones, ni menú, ni reloj**.
>
> Eso convierte "poner el Modo Degradado en las dos unidades" en un trabajo mayor de lo previsto:
> hay que **portar la interfaz completa al Esclavo** antes de poder configurar nada en él.
>
> **Espacio disponible:** el Esclavo usa 15.480 B de 65.536 (**23,6 %**), le quedan ~50 KB. Con U8g2
> subiría a unos 40 KB. **No es una restricción.**

| # | Trabajo | Por qué va aquí | Esfuerzo | Depende de |
|---|---|---|---|---|
| **1** | **N-10 · Antenas VHF 136–174 MHz** | El alcance está en **3 cuadras**. Es la limitación más grande del uso real y ya está en curso. **No es firmware** — avanza en paralelo con todo lo demás | Compra + montaje | — |
| **2** | **Prueba de banco: telemetría y contadores** *(checklist 5.6 / 5.7, parte de N-8)* | **Valida lo que ya está construido y flasheado, sin escribir código.** SFTY-14 y SFTY-15 llevan desde el 31/07 en el equipo sin que nadie confirme que sus números son ciertos. Lo más barato con más retorno | Media jornada de banco | — |
| **3** | **N-15 · Reloj y pantalla de ajuste de hora en el Maestro** | **Primera pieza de código.** Sin poder poner la hora no se valida la pila ni se habilita nada de lo que viene. Incluye resolver la **persistencia en `BKP->DR1..DR10`** y el acceso por pulsación larga, ya que el menú de 4 opciones está lleno | ~1 jornada | — |
| **4** | **N-16 · Portar la interfaz al Esclavo** | `lcd.cpp`, `botones.cpp`, `menu.cpp` y `reloj.cpp` **no existen** en el Esclavo. Sin ellos no se puede configurar el Modo Degradado en esa punta, y el procedimiento exige hacerlo **en las dos**. Incluye **limpiar las cabeceras huérfanas** y decidir **qué opciones lleva el menú del Esclavo**, que no son las del Maestro | ~1,5 jornadas | **3** |
| ~~**5**~~ | ✅ **N-12 · Anti-deriva del simulador — HECHO (01/08)** | Los tres tiempos de seguridad se leen del C++ y su lectura es **obligatoria**: si no se pueden leer, la prueba **aborta** en vez de caer a un valor por defecto que casualmente coincidía con el escrito a mano. Verificado en ambos sentidos | — | — |
| **5-bis** | **N-18 · SFTY-23 · Sincronización horaria por radio** | **Bloqueante, detectado en auditoría.** Sin él, ajustar a mano las dos puntas deja hasta **59 s de desfase el primer día** y dos pantallas `HH:MM` no lo detectan. Incluye `CMD_DELTA` —la validación pasa de inspección ocular a número registrable— y `CMD_CONFIG`, porque el ciclo debe viajar igual que la hora | ~1 jornada | **3** |
| **6** | **N-9 · Modo Degradado (SFTY-21)** | El objetivo de todo lo anterior. **Activación manual verificada** en ambas unidades, con el rediseño del **mando de 4 relés**. El ámbar sigue siendo el comportamiento por defecto ante pérdida de radio. Incluye el **límite duro de 48 h** sin resincronizar | ~2 jornadas + simulación de deriva | **3, 4, 5, 5-bis** |
| **7** | **N-5 · La cámara se da por viva el primer minuto** | Defecto real y acotado: `ai_ultimoMensaje` arranca en 0 y produce ciclos cortos espurios al encender. Independiente del resto | ~2 h + simulador | — |
| **8** | **N-3 · Modo intermitente nocturno (SFTY-20)** | Aplazado por decisión del cliente: el horario no es igual en todas las obras. Se apoya en el mismo reloj | ~1 jornada + validación | **3, 5** |
| **9** | **N-13 · Pantalla informativa en ámbar (SFTY-22)** | **Mejora.** Casi todo el dato ya existe (SFTY-14/15); falta juntarlo y añadir la hora | ~4 h | **3** |
| **10** | **N-11 · Validar el repetidor sobre hardware** | Solo aplica cuando vuelvan a las 4 radios. **SFTY-16 y SFTY-17 nunca se han ejercitado sobre hardware real** | Banco + campo | Uso del repetidor |

### La ruta crítica hacia el Modo Degradado

```
   3. Reloj + pantalla de hora (Maestro)   [PARCIAL: rama feat/n15]
        |
        +---> 5-bis. Sincronizacion por radio (SFTY-23)  ---+
        |                                                   |
        +---> 4. Portar interfaz al Esclavo  ---------------+
                                                            |
              5. Anti-deriva del simulador  ----------------+
                                                            |
                                                            v
                                                6. MODO DEGRADADO
```

**Son ~5,5 jornadas antes de que el Modo Degradado pueda existir.**

### Cómo cambió el punto 4 al aparecer SFTY-23

La sincronización por radio **simplifica el menú del Esclavo**: si la hora se cuadra una sola vez en
el Maestro y viaja por radio, **el Esclavo ya no necesita pantalla de ajuste de hora** para cumplir el
procedimiento. Queda, a lo sumo, como respaldo de emergencia.

**Menú mínimo del Esclavo:**

- **Ver hora y desfase** — *no ajustar*: mostrar su hora y cuándo fue la última sincronización
- **Entrar y salir del Modo Degradado**
- **Ver el estado del enlace** *(los contadores de SFTY-15 ya existen en su `protocolo.cpp`)*

**Sin los modos de operación:** quien decide el ciclo es el Maestro, y ofrecer esa opción en el
Esclavo invita a dejarlos peleando.

### Las pantallas deben mostrar SEGUNDOS

`HH:MM` no sirve para validar nada: dos relojes a 40 s de distancia muestran el mismo `14:32`. Ambas
pantallas deben mostrar **`HH:MM:SS`**, que es la comprobación de respaldo cuando el radio ya murió y
`CMD_DELTA` no está disponible.

### Por qué el 2 va antes que el 4

Añadir el modo nocturno mete **estados de luz nuevos** en un equipo que apenas se estabilizó ayer. Si
además la telemetría sigue sin verificar, una incidencia futura llega sin instrumentos de confianza
para diagnosticarla: no se sabría si el `RF:100%` de la pantalla dice la verdad.

**Primero se confirma que los instrumentos no mienten. Después se añaden funciones.**

### 🔎 Lo que dice el simulador, y que cambia el plan

Revisado `simulador_sistema_v7_6.py` (581 líneas) antes de cerrar la propuesta. Dos cosas:

**Buena noticia — el modo nocturno sale más barato de lo estimado.** El simulador ya tiene **eje de
tiempo** (`avanzar_simulacion(duracion_s, dt=0.1)` y `current_time` en todas las máquinas de estado)
y las luces son estados con nombre (`luz_local`). Simular el cruce de la franja horaria es añadir un
reloj al modelo y una prueba que verifique tres cosas: que ambas unidades entran y salen en fase, que
la transición **pasa por el todo-rojo**, y que con la hora no fiable **nunca entra**.

**Mala noticia — el simulador puede mentir sin que nadie lo note.** El bloque 0 lee del C++ solo tres
constantes: `RF_BURST_COPIES`, `TIMEOUT_ACK_MS` y `RETARDO_RESPUESTA_MS`. Pero **los tiempos de
seguridad están escritos a mano en el modelo**:

| Valor | En el simulador | En el C++ | ¿Protegido? |
|---|---|---|---|
| Despeje todo-rojo (SFTY-4) | `15.0` *(línea 171)* | `tiempoDespejeMs = 15000` | ❌ **No** |
| Fallback sin comunicación (SFTY-6) | `12.0` *(líneas 257, 266)* | `12000` | ❌ **No** |
| Ámbar fijo Rojo→Verde (SFTY-5) | `4.0` *(línea 286)* | — | ❌ **No** |

**Hoy coinciden**, así que los 9/9 son válidos. Pero nadie vigila que sigan coincidiendo: si mañana se
cambia el despeje en el C++, el simulador seguirá probando 15 s y **seguirá diciendo PASS**.

Esto importa especialmente para el modo nocturno, porque **el despeje todo-rojo es justo el margen de
seguridad al entrar y salir del modo**. Construir esa función sobre un valor que el modelo no vigila
es edificar sobre arena. Queda como **N-12**, y debe hacerse *antes* del punto 4.

### Lo que queda fuera de este orden

- **Cambiar la contraseña de la cámara** — no es una tarea de desarrollo, es una acción de seguridad
  pendiente desde el 01/08. Ver *Auditoría del repositorio*.
- **Fijar y anotar la versión que corre en campo** tras cada flasheo. Ya está resuelto para la V8.4;
  la disciplina es mantenerlo.

---

## 🔻 Pendientes conocidos (no corregidos en V8.0)

| Ref | Pendiente |
|---|---|
| ~~**N-1**~~ | ✅ **RESUELTO.** Radios reconfiguradas a 2.4 kbps y `M0`/`M1` en OFF. Junto con la retirada de la radio B1 averiada, el enlace **funciona en campo desde el 01/08/2026**. Sigue siendo requisito de operación, no una acción pendiente. |
| **N-10** | **Antenas VHF 136–174 MHz.** En curso. Las genéricas de "LoRa" (433/915 MHz) cuestan 15–20 dB y dejan la cobertura en 3 cuadras. Selección y advertencias en `05_Funcional/2_Manual_Hardware_y_Pruebas.md §2.1`. **No bloquea la operación, limita el alcance.** |
| ~~**N-12**~~ | ✅ **RESUELTO 01/08/2026.** Los tres tiempos de seguridad —despeje todo-rojo, fallback sin comunicación y ámbar fijo— se leen ahora del C++ en cada ejecución, ya no están escritos a mano. Y con una vuelta de tuerca: son de **lectura obligatoria**. Un respaldo silencioso habría derrotado el propósito, porque los valores por defecto eran **los mismos** que estaban escritos a mano: si alguien renombrase `tiempoDespejeMs`, el modelo caería a 15.0 y **seguiría dando PASS** mientras el firmware usa otro valor — la deriva disfrazada de éxito. Ahora **aborta con código 2**. Verificado de las dos formas: ejecución normal 9/9, y con la constante renombrada a propósito aborta como debe. |
| **N-11** | **Repetidor sin validar en campo.** El sistema opera hoy con 2 radios en enlace directo, así que **SFTY-16 y SFTY-17 nunca se han ejercitado sobre hardware real** — se diseñaron para el camino de 4 radios. Al reintroducir el repetidor hay que probarlos de nuevo, no darlos por buenos. |
| **N-3** | `MANUAL_USUARIO.md §2` especifica operación intermitente por bajo flujo (ámbar en una vía, rojo en la otra). **No está implementado** y queda **aplazado por decisión del cliente (31/07):** el horario no es el mismo en todas las obras, así que no se fija una franja en firmware. **Hardware y reloj ya resueltos** — ver V8.5 y SFTY-18. Al retomarlo hay que decidir antes si el disparo es por **horario** (basta el RTC) o por **flujo real** (exige la cámara instalada, que hoy no lo está). |
| **N-9** | **Modo Degradado por reloj (SFTY-21).** Especificado con su análisis de seguridad en `OPTIMIZACIONES.md`, **sin una sola línea de firmware escrita**. **Redefinido el 01/08:** activación **manual verificada por un operario**, no automática — el ámbar intermitente sigue siendo el comportamiento por defecto ante pérdida de radio. Incluye el rediseño del **mando de 4 relés** para operarlo desde el piso sin ver la pantalla. Requiere simular la deriva entre unidades y validarlo con el funcional **antes** de ir a campo. |
| **N-13** | **Pantalla informativa durante el ámbar (SFTY-22).** Mejora, no urgente. Al entrar en ámbar por pérdida de enlace, mostrar la causa y desde cuándo. Los contadores de línea y la telemetría ya existen; falta agruparlos y añadir la marca de hora del RTC. |
| **N-15** | 🟡 **EN CURSO** — rama `feat/n15-reloj-pantalla-hora`. **Hecho:** pantalla **AJUSTAR HORA** como quinta opción del menú, edición dígito a dígito, `reloj_setup()` ya invocado desde `main.cpp`, layout del menú adaptativo (11 px hasta 4 opciones, 9 px con 5) conservando intacto el de 4 validado en campo. Validación de pantalla **42/42**. **Falta:** la **persistencia de la configuración** —hoy vive solo en RAM y se perdería en cada apagón—; `STM32duino RTC` 1.9.0 no expone los registros de respaldo, en el STM32F1 son accesibles por `BKP->DR1..DR10`. Y **la prueba de banco**: comprobar contra hora patrón y que la hora sobreviva al corte de energía. |
| ~~**N-16**~~ | ✅ **RESUELTO 01/08/2026.** El Esclavo ya tiene interfaz completa: `lcd.cpp`, `botones.cpp`, `menu.cpp` y `modo_degradado.cpp`. Menú de **2 opciones** —`ESTADO` y `MODO DEGRADADO`—, con `ESTADO` primero **a propósito**: con el mando a ciegas, lo peor que puede hacer un pulso de aceptar perdido es mostrar un diagnóstico. **Sin ajuste de hora**: llega por radio (SFTY-23) y ajustarla a mano allí reintroduciría el desfase que ese mecanismo elimina. Borradas las 4 cabeceras huérfanas tras verificar que ningún `.cpp` las incluía. Flash 31,2 % → **59,7 %**. |
| **N-19** | **El Esclavo no tiene mando de relés.** El procedimiento del Modo Degradado exige activarlo **en las dos puntas**, y ambas pantallas están a 5 m dentro del gabinete. El Maestro tiene mando; el Esclavo no. **No hay atajo por software:** el Maestro no puede ordenárselo por radio, porque *el radio muerto es la razón de entrar al modo*. La tarjeta ya tiene las cuatro entradas (`PB9`, `PB13`, `PB14`, `PB15`) — falta solo el receptor. **Mientras tanto el sistema funciona**, con la salvedad de que activar el Degradado en el Esclavo exige subir al gabinete. ⚠️ **Al comprarlo, exigir código independiente del mando del Maestro**: si ambos receptores responden al mismo mando —y las dos puntas están a menos de una cuadra— una secuencia metería las dos en Degradado a la vez, saltándose la verificación por separado que justifica todo el diseño. |
| **N-22** | ⏳ **ABIERTO — pantalla del Esclavo en azul y sin píxeles.** Software DESCARTADO: `pines.h` byte a byte idéntico, mismo constructor `U8G2_ST7920_128X64_F_SW_SPI`, mismo `lcd_setup()`, y el orden de arranque ya igualado (incluido el `delay(2000)` que el Maestro tenía). Además la versión anterior tenía `lcd_setup()` en última posición —máximo reposo posible antes de tocar el módulo— y también salía en blanco: si los dos extremos fallan igual, el instante de inicialización no es la variable. **Hipótesis principal, que ningún diff puede ver: niveles lógicos marginales.** Con el módulo a 5 V, el `V_IH` garantizado del ST7920 es `0,7·VDD = 3,5 V`, y el STM32 entrega **3,3 V** en `SCLK`/`SID`/`CS`. Está fuera de especificación **en las dos puntas**: que el módulo del Maestro pinte no prueba que el diseño sea correcto, prueba que ese chip tolera. La dispersión unidad a unidad decide, y explica el síntoma de *"todo idéntico y solo esta falla"*. **Diagnóstico en dos pruebas:** (A) módulo+cable del Maestro a la tarjeta del Esclavo — si pinta, la tarjeta está sana; si no, mide `VDD` en el conector y revisa soldaduras de `PB3`–`PB7`. (B) módulo del Esclavo a la tarjeta del Maestro — si tampoco pinta ahí, es el módulo: **contraste `V0`** (potenciómetro del dorso; un ST7920 sin contraste es exactamente *azul con retroiluminación y sin píxeles*), o **puente `PSB` estrapado a VCC** en el propio módulo, que deja el pin del STM32 peleando contra el strap y el modo serie nunca entra. Si pinta en el Maestro pero no en el Esclavo, es **la combinación** → niveles. **Salidas:** alimentar el módulo a 3,3 V (el ST7920 admite 2,7–5,5 V; puede exigir reajustar contraste) o un buffer `74HCT` en las tres señales. ⚠️ **No bloquea la operación** — el Esclavo funciona en automático; lo que bloquea es activar el Degradado en esa punta. |
| **N-23** | ✅ **CERRADO.** Poner el reloj no era sincronizar. `modo_hora.cpp` encolaba el envío y volvía al menú, donde **nadie llama a `coordinador_actualizar()`**: las tramas nunca salían y el Degradado rechazaba con `Falta: nunca hubo sincronización RF` con los radios al 100 %. La pantalla ahora bombea el coordinador hasta el `CMD_ACK_HORA` y muestra el desenlace. |
| **N-24** | ✅ **CERRADO.** `reloj_ajustar()` ponía `horaValida=true` sin comprobar que el RTC contara: con el cristal muerto se escribía sobre un contador parado y la hora volvía a ceros al apagar. Peor, el Maestro habría empujado esa hora al Esclavo y autorizado el Degradado sobre un reloj detenido. Añadido `rtcOperativo`; la pantalla distingue `RELOJ SIN PONER EN HORA` de `SIN CRISTAL: Y2, PILA, R5`. |
| **N-25** | ✅ **CERRADO.** N-17 acotó el arranque a 2 s **y apagaba el oscilador** al agotarse, lo que convertía *"lento"* en *"muerto"* de forma permanente. Ahora se deja encendido y `reloj_actualizar()` mira `LSERDY` cada 30 s desde el `loop()` de las dos puntas: si el cristal despierta tarde, el equipo lo **adopta sin reiniciar**. |
| **N-26** | ✅ **CERRADO.** `botones_setup()` no sembraba el estado real de los pines: un botón ya pulsado al encender se leía como flanco. El Botón 3 **ejecuta**, y los pulsadores van en paralelo con el mando de relés, así que un relé cerrado en reposo bastaba para arrancar un modo que nadie pidió. |
| **N-27** | ⚠️ **`validador_maestro.py` estuvo ABORTANDO en silencio.** No fallaba: se rendía antes de probar nada porque no encontraba en `respaldo.cpp` la constante `s ^ 0xNNNNU`. El checksum se había reescrito a un **hash de Horner** (`s = s*31 + reg`, semilla `0x1F35`, plegado 32→16) y el validador seguía modelando la **suma llana ponderada** anterior. Y solo arreglar la máscara habría sido peor: su regex de reserva asigna peso 1 a los cinco registros, es decir, habría quedado modelando **el propio algoritmo mutante que él existe para demostrar que es inseguro** (insensible al orden), dando PASS sobre lo que no es. Corregido: lee semilla, multiplicador y **orden** del C++. ⚠️ **Un validador que aborta es peor que uno que falla** — desde fuera parece que corrió. |
| **N-28** | ✅ **Compuerta única de verificación** — `01_Firmware/compuerta.py`. Compila los tres firmwares y corre los dos simuladores, los tres validadores y el arnés de pantalla con **un solo código de salida**, distinguiendo `PASS` / `FALLA` / `ABORTADO`. Nace de N-27: *"desde fuera parecía que corrió"* solo fue posible porque cada comprobación se lanzaba a mano y por separado. **`ABORTADO` nunca cuenta como éxito** — cierra la compuerta igual que un fallo, porque no dice nada del firmware. Las cifras del README salen ya de aquí. |
| **N-29** | ✅ **CERRADO 04/08 — el arnés ya está en la compuerta** *(suite `arnes del respaldo`, ver N-43 y N-44)*. Lo que sigue abajo es el historial de por qué tardó: el bloqueo original era falso y el arnés llevaba días escrito. ~~⏳ PENDIENTE~~ — **dejar de reimplementar `calcularSuma()` en Python.** El validador mantiene un modelo del checksum y **ya divergió dos veces en un día** (suma llana → pesos → Horner); cada mejora del C++ obliga a re-modelar, y cada re-modelado es una oportunidad de medir otra cosa en silencio. Afinar regexes no es la solución de fondo. El patrón correcto ya está resuelto en `Validacion_LCD`: **compilar el fuente real** contra un arnés en PC. `calcularSuma()` es una función pura sin hardware —el caso ideal—: un `main.c` de veinte líneas que la enlaza con `leerReg()` sobre un array, la expone por stdin/stdout, y el validador le lanza los volteos de bit y las transposiciones **contra el binario del código real**. La deriva pasaría de riesgo vigilado por regex a **imposible por construcción**. ~~⚠️ **Bloqueado hoy:** esta máquina no tiene `gcc` de host~~ ✅ **DESBLOQUEADO 03/08 — y el bloqueo era falso.** `gcc` llevaba semanas instalado (N-38: winget lo dejó fuera del `PATH`, hoy ya está dentro: MinGW-W64 UCRT 16.1.0). **Y el arnés que pedía este N-29 YA EXISTE**: `01_Firmware/Validacion_Respaldo/arnes_respaldo.cpp` lo trajo N-31, ejerce `calcularSuma()` y el Horner sobre el C++ real, y compila limpio. Ver **N-43**: no está conectado a la compuerta. |
| ✅ **N-37** | **CERRADO POR ELIMINACION: el cristal `Y2` esta MUERTO. Banco del 01/08/2026.** No es diagnostico por descarte perezoso; cada sospechoso se elimino con una medida: (1) **`VBAT` = 3 V con la tarjeta APAGADA** — si `R5` siguiera puesto, `VBAT` estaria atado al riel de 3,3 V, que apagado es 0 V; que se mantenga en 3 V prueba a la vez que `R5` esta fuera y que la CR2032 alimenta el dominio de respaldo. (2) **`N-25`**, reintento cada 30 s: descarta "cristal lento". (3) **`N-31` REINICIAR RELOJ** devolvio `SIGUE PARADO / No era el estado`: descarta que unos registros del dominio de respaldo sucios —de los arranques colgados de `N-17`— impidieran arrancar el oscilador. **Ya no queda software que probar.** Es el caso que `MAPEO_TARJETA_KICAD.md §4` advertia en micros clonados: condensadores de carga mal calculados. ➡️ **Salida: `DS3231` por I²C software en `PB0`/`PB8`**, los unicos pines libres. No es consuelo — ±2 ppm compensado en temperatura lleva la deriva a 48 h de ~17 s a **menos de medio segundo**, y eso convierte el todo-rojo de 30 s en colchon comodo en vez de justo. ⚠️ Hacen falta **DOS**: el Esclavo casi con seguridad tiene el mismo problema —nunca acusa la hora, `N-24` la rechaza— y con su pantalla muerta (`N-22`) no hay forma de comprobarlo desde el menu. ⚠️ El modulo lleva **LIR2032 recargable**, NUNCA la CR2032. |
| ✅ **N-38** | **El arnés cazó un desborde que mi recuento a mano NO vio, y el `ABORTADO` de `gcc` era un falso negativo mío.** (1) **El compilador estaba instalado desde el principio**: winget lo dejó en `%LOCALAPPDATA%\Microsoft\WinGet\Packages\BrechtSanders.WinLibs...\mingw64in`, **fuera del PATH**. Lo que falló días fue la búsqueda (`shutil.which("gcc")` en `compuerta.py:124`), no la máquina. *Un "no aparece" no es un hallazgo hasta descartar al buscador* — y esta vez el buscador era mío. `compilar.ps1` ya lo localiza solo. Para `compuerta.py`: `setx PATH "$env:PATH;%LOCALAPPDATA%\Microsoft\WinGet\Packages\BrechtSanders.WinLibs.POSIX.UCRT_Microsoft.Winget.Source_8wekyb3d8bbwe\mingw64in"`. (2) **Resultado real: 234/239** (Maestro 108/113, Esclavo **126/126**). (3) **Desborde CONFIRMADO por medida en `lcd_dibujarSyncHora()`**: `"SINCRONIZADA"` terminaba en **x=129** y `"SOLO MAESTRO"` en **x=131**. El error de razonamiento: conté caracteres × 6 px, correcto para la `6x10` *de paso fijo* y **que no dice nada de la `ncenB10`, que es proporcional**. Las nueve líneas de `6x10` estaban todas bien; fallaban justo los títulos, los únicos que no se podían contar. **Corregido a `7x14B`** (paso fijo, 12×7 = 84 px). ⚠️ **No verificado con el arnés** — la política de ejecución de PowerShell bloquea `compilar.ps1` en esta sesión; el argumento es aritmético, no medido. **Correrlo es lo primero de la sesión nueva.** (4) El arnés incorpora ahora un **control negativo**: exige que el borrador descartado `"El esclavo no contesto"` **no** quepa. *Una comprobación de anchos que aprueba todo no comprueba nada.* (5) ⚠️ **`setBusClock()` NO sirve** para la variante lenta del diagnóstico: este display usa `u8x8_byte_arduino_4wire_sw_spi`, que mueve pines con `digitalWrite()` y nunca lo consulta. Compila, no da error y no cambia nada — la trampa perfecta para concluir *"con reloj lento tampoco pinta"*. Hay que sustituir `gpio_and_delay_cb`, y ya está hecho en `Diagnostico_LCD`. |
| 🔴 **N-32** | **CRÍTICO — el arreglo de `CADUCADA` dejó el desbordamiento de `millis()` SIN NINGUNA capa.** `Maestro/src/modo_degradado.cpp:292` hace `if (horasPila == RESPALDO_SYNC_CADUCADA) return ms;` y el comentario de 30 líneas justo encima describe **lo contrario**. Las dos condiciones coinciden **siempre**: cuando `millis()` da la vuelta han pasado 49,7 días, luego `respaldo_horasDesdeSync()` cae sí o sí en `dias > 2` y devuelve `CADUCADA`, así que la rama `max(msPila, ms)` que debía proteger **no puede ejecutarse nunca en el caso del desbordamiento**. El roadmap decía *"una capa en vez de dos"*: son **cero**. Medido: **1488 de 17.856 escenarios abren la puerta** con una sync real de ~50 días (0 con la versión anterior); ejemplo, sync del día 1 a las 00:00, 49,75 días reales → la puerta lee **57 min**. Deriva a 50 días @100 ppm: **432 s** contra un todo-rojo de 30 s. **Escenario:** radio muerto, Maestro encendido 50 días sin reiniciar, un operario hace `A·B·A·B` → Maestro entra y da VERDE por reloj; el Esclavo latcheó `syncVencidaLatch` a las 48 h y **rechaza para siempre** → Maestro VERDE ⟷ Esclavo ÁMBAR indefinidamente. **Dos auditorías independientes lo encontraron por caminos distintos.** ⚠️ Lo introduje al corregir el cruce de mes: arreglé una cosa y abrí otra. |
| 🔴 **N-33** | **ALTO — el veto de la pila SIGUE VIVO justo en reanudación, que es el único caso que N-20 existe para cubrir.** El arreglo de N-20 solo cubre la rama con la RAM válida. **Tras un corte la RAM del Maestro NUNCA es válida:** `coordinador_msDesdeUltimaSync()` devuelve `0xFFFFFFFF` mientras `syncAlgunaVez` sea false, y en Degradado el Maestro calla en la radio, así que jamás se pone a true. **Escenario:** sync a las 23:00 del día 31 → microcorte a las 23:10 → las dos reanudan en fase → a las 00:00 el día pasa de 31 a 1 → `dias = 1-31 = -30 < 0` → `CADUCADA` → el Maestro se va a **ÁMBAR con la sincronización a 1 h de antigüedad**, mientras el Esclavo —que sembró `tUltimaSync` y no vuelve a mirar la pila— **sigue dando VERDE**. Hasta 47 h. Es el riesgo residual nº 2 palabra por palabra. ⚠️ El validador del Maestro lo reporta pero contra el código viejo, así que **se estaba descartando como falso positivo cuando en esta rama sigue vivo**. |
| 🔴 **N-34** | **ALTO — la regla de inhibición del mando NO es la misma en las dos puntas.** Maestro: `mando.cpp:88-91` devuelve `(m == MENU \|\| m == MODO_HORA)`, y **el estado de reposo del Maestro ES `MENU`**. Esclavo: `mando.cpp:93-95` usa `menu_estaAbierto()`, y su reposo `P_MENU` **no** cuenta como abierto. Resultado: en reposo el mando del Maestro está **muerto** y el del Esclavo **armado**. **Escenario:** microcorte que el Maestro no puede reanudar → arranca en `MENU` → sin enlace queda en ámbar intermitente, indistinguible desde el suelo del ámbar de `C_FALLO`. El operario hace `A·B·A·B` en los dos gabinetes: el Esclavo **acepta y entra**; el Maestro limpia el buffer **en silencio**, sin destellos ni ámbar de rechazo. Esclavo VERDE ⟷ Maestro ÁMBAR. Hoy enmascarado porque el Esclavo no tiene receptor (N-19); **el día que se instale, este es el camino normal**. |
| 🔴 **N-35** | **MEDIO-ALTO — entrar al Degradado cuesta UNA pulsación en el Maestro y DOS en el Esclavo.** `Maestro/src/menu.cpp:106-113`: `Botón 3` sobre `MODO DEGRADADO` entra directo. **No existen las pantallas `Pulse 3 para entrar` ni `CONFIRMAR ENTRADA?`** que el procedimiento §3 y `Esclavo/src/menu.cpp:122-126` describen (*"entrar exige dos pulsaciones; salir, una. La asimetría es deliberada"*). El Esclavo sí las tiene. **Y N-26 acaba de demostrar en banco que el pulso fantasma de `Botón 3` es real:** con el cursor ahí y el radio vivo, un solo pulso mete al Maestro en Degradado → deja de llamar al coordinador → el Esclavo cae a ámbar por orfandad a los 12 s → 30 s después el Maestro da VERDE. Verde contra ámbar **sin que nadie haya pedido nada**. |
| ⚠️ **N-36** | **Los tres validadores miden fuente que ya no existe — la enfermedad de N-27, pero reportando en vez de abortar.** (a) `validador_maestro.py:1374-1397` dice ser *"el port tal como quedó tras el arreglo"* y modela la versión **ANTERIOR** (`return 0xFFFFFFFF` donde el C++ dice `return ms`): por eso sus fallos 4/5/6 son artefactos **y por eso no ve N-32 ni N-33, que viven justo en la diferencia entre modelo y código**. Encima etiqueta como *mutante* lo que el firmware hace hoy: los dos modelos están intercambiados. (b) `validador_esclavo.py:807-815` no conoce `verdeDeEsteEnvio()` **ni `CMD_HORA_D`**, así que la aplicación atómica de la hora **nunca se prueba con la trama de día**. (c) `validador_costura.py`: 4 de 41 no concluyen por regex contra símbolos renombrados, y su desajuste nº5 busca `"FECHA"` o `"DIA"` en los comandos — el que lo cerró se llama `CMD_HORA_D` y no contiene ninguna. (d) `validador_maestro.py` **sale con código 0 llevando 7 fallos dentro**, así que la compuerta lo marca `[OK]`: N-28 impide que `ABORTADO` cuente como `PASS`, pero **una `FALLA` dentro de un validador que sale con 0 sí cuenta**. ⚠️ **Arreglar los validadores va ANTES que los parches**, o no habrá forma de medir si funcionaron. |
| **N-21** | **Margen de flash del Maestro.** Tras el Modo Degradado va al **80,2 %** (52.540 de 65.536 B). Lo pendiente —`respaldo.cpp` conectado, SFTY-22 y el modo nocturno— lo dejaría sobre el **85 %**: ajustado pero viable. Una función grande más ya no cabría. **Salida conocida, con letra pequeña:** el `STM32F103C8` está especificado con 64 KB pero el silicio suele traer **128 KB físicos**, y cambiar la placa a `genericSTM32F103CB` duplicaría el presupuesto. Es práctica común **pero no documentada por ST**: en un equipo de seguridad vial exige verificarlo **chip a chip**, porque si un lote viene con 64 KB reales el firmware se corrompe en silencio al pasar del límite. **No tocar todavía** — al 85 % no hace falta, y el día que haga falta debe ser una decisión consciente con su prueba. |
| 🟡 **N-20** | **EN CURSO.** Módulo `respaldo.cpp`/`.h` escrito e idéntico en ambos proyectos: acceso a `BKP->DR1..DR7` con firma y suma de comprobación, para distinguir contenido propio de basura que quede en el dominio de respaldo tras un arranque sucio. Guarda ciclo, marca de sincronización y estado del Degradado. **Regla clave:** `respaldo_horasDesdeSync()` devuelve **CADUCADA ante cualquier ambigüedad** —solo se conoce el día del mes, así que un cambio de mes o un reloj que retrocede se declaran caducados—; confundir *"no sé cuánto ha pasado"* con *"ha pasado poco"* autorizaría el Degradado sobre una sincronización de antigüedad desconocida. **Falta conectarlo** en ambos firmwares. |
| **N-20 (origen)** | **Persistir en registros de respaldo el estado del Modo Degradado.** *Idea del cliente, 01/08/2026.* Hoy los indicadores de sincronización y la configuración del ciclo viven en RAM: tras un microcorte el Esclavo los pierde y cae a **ámbar**, mientras el Maestro sigue dando verde por reloj — **que es exactamente el riesgo residual nº 2 de SFTY-21**, el que se documentó como "sin solución técnica sin radio". Guardándolos en `BKP->DR1..DR10` —alimentados por **la misma pila CR2032 ya instalada**— el Esclavo **reanuda en fase** en lugar de crear la asimetría. **Reanudar es más seguro que caer a ámbar**, porque caer a ámbar es lo que crea el escenario peligroso. Cierra además la persistencia pendiente de **N-15**. ⚠️ **No confundir con autorización por adelantado** (*"si pierdes el radio X minutos, entra"*): eso es entrada automática con pasos extra y sigue descartado — nadie confirma que la otra punta siga viva. |
| ~~**N-17**~~ | ✅ **RESUELTO 01/08/2026, y confirmado en hardware — no en simulación.** El cristal `Y2` **no oscila** en las tarjetas actuales, y `rtc.begin()` esperaba al oscilador **sin límite**: las dos se colgaban en `reloj_setup()`. El Maestro quedaba en bucle en la bienvenida; el Esclavo, con la pantalla en blanco pero **las luces funcionando**, porque su `lcd_setup()` va después del cuelgue y `semaforo_setup()` antes. Corregido con arranque acotado a 2 s: si el cristal no responde, **el equipo arranca sin reloj**. ⚠️ **Consecuencia:** el Modo Degradado y la operación nocturna **no se pueden usar en estas tarjetas** hasta resolver el cristal — ver `MAPEO_TARJETA_KICAD.md §4` para el condensador de carga y la alternativa del `DS3231`. |
| **N-17 (origen)** | **Prueba de banco: arranque con el cristal `Y2` muerto.** `reloj_setup()` enciende el oscilador LSE, y `MAPEO_TARJETA_KICAD.md §4` documenta que en microcontroladores clonados el cristal de 32.768 kHz **a veces no arranca**. Ya se movió **detrás** de `IWatchdog.begin()` para que un bloqueo del HAL sea un reinicio visible y no un cuelgue mudo con las luces apagadas, pero **hace falta comprobarlo con la tarjeta en la mano**: arrancar con `Y2` desconectado y verificar que el equipo bootea igual y `reloj_enHora()` devuelve `false`. **Un semáforo no puede depender de un cristal de reloj para encender.** |
| 🟡 **N-18** | **EN CURSO — implementado, pendiente de banco.** Rama `feat/n15-reloj-pantalla-hora`. **Hecho:** comandos `0x07`–`0x0F` idénticos en ambos `protocolo.h`; lado Maestro (`coordinador_sincronizarHora/medirDesfase/enviarConfigCiclo`, máquina de estados separada de la del ciclo, resincronización cada hora, encolado al recuperar enlace); lado Esclavo (aplicación atómica con ventana de caducidad de 3 s, desfase circular, almacenamiento de configuración). Poner en hora **sincroniza en el mismo gesto**. Validación: **16/16** en el simulador funcional (7 pruebas nuevas), 10/10 repetidor, 42/42 pantalla. **Falta:** prueba de banco sobre hardware real y consumir la configuración en el ciclo, que hoy solo se almacena. |
| **N-18 (origen)** | **SFTY-23 · Sincronización horaria por radio.** Detectado en auditoría: el procedimiento de SFTY-21 pedía confirmar la hora mirando dos pantallas `HH:MM`, lo que **no detecta hasta 59 s de desfase** —casi cuatro veces el todo-rojo—. La hora debe cuadrarse **una sola vez en el Maestro** y viajar por radio (`0x07`–`0x0B`), junto con la configuración del ciclo. **Requisito previo del Modo Degradado**, no un extra. Incluye la medición de desfase (`CMD_DELTA`) que convierte la validación en un número registrable. |
| **N-14** | **Pilas del RTC: sustitución futura.** Instaladas **CR2032 no recargables en ambas tarjetas** (01/08/2026). Con ~1,4 µA de consumo por `VBAT` la autonomía teórica supera los 15 años, así que el límite real es la caducidad de la pila (~10 años). **No es mantenimiento periódico.** Queda anotado como mejora evaluar una solución recargable o de mayor vida. |
| **N-5** | Modo Inteligente da la cámara por viva durante el primer minuto tras arrancar (`ai_ultimoMensaje` inicializado en 0), produciendo ciclos cortos espurios. |
| ~~**N-6**~~ | ✅ **RESUELTO 01/08/2026.** La ventana deslizante estaba etiquetada `SFTY-11` en el código cuando esa numeración corresponde a la ráfaga: corregida a `SFTY-10`. `SFTY-14` y `SFTY-16` estaban implementadas sin etiqueta: añadidas. Levantada la **tabla de trazabilidad** en `OPTIMIZACIONES.md`, generada buscando las etiquetas en los fuentes y no a mano, de modo que una regla sin implementar salga vacía en vez de aparentar cobertura. Las 18 reglas quedan localizadas. |
| **N-8** | Sin pruebas de banco físico documentadas (Fase 4 de `ORDEN_EJECUCION.md`). La validación es de simulación, no de hardware. |
| — | Repetidor ESP32 sin watchdog. |
| — | Al arrancar el Maestro hay ~2s con todas las luces apagadas (`semaforo_apagarTodo()` + `delay(2000)` de bienvenida). |
