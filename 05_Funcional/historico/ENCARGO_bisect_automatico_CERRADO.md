# ENCARGO — Localizar la regresión del Modo Automático por bisección

> # 🔴 CERRADO EL 05/08 — Y SU SOSPECHOSO ESTÁ REFUTADO. NO SEGUIR ESTE DOCUMENTO.
>
> **Este encargo ya se ejecutó** (03/08): los 16 binarios están en `bisect_entregable/`. Se
> conserva por lo que enseña sobre cómo se elige un sospechoso, **no para trabajar con él**.
>
> **Lo que dice más abajo sobre `2779d9b` ya no se sostiene.** Su argumento era que *"el mando de
> relés intercepta las escrituras de pines de luz, que es justo el mecanismo por el que el ciclo
> podría dejar de avanzar"*. **N-52 lo refutó con medida:** el arnés del automático ya compila
> `mando.cpp` real y ejerce esa ruta sobre los pines en nueve caminos distintos —las tres
> secuencias, secuencia a medias, pulsos A y B en el mismo tick, reinicio a medio destello y 600
> pulsos aleatorios—. **En ninguno se queda la bandera pegada.** Verificado además que el arnés
> sabe verlo: vaciando `terminarSenal()` cae de `67/67` a `39/67`.
>
> ➡️ **El encargo vigente es [`ENCARGO_SESION_BANCO.md`](../ENCARGO_SESION_BANCO.md).** Allí la
> bisección ya no es *"confirmar a `2779d9b`"* sino una bisección de verdad, sin sospechoso.
>
> Se conserva y no se borra porque **una causa que desaparece en silencio vuelve a proponerse**, y
> la segunda vez ya nadie recuerda que se comprobó.

---

> **Para pegar en una sesión aparte.** Devuelve un informe corto; no toca `main` ni sube nada
> a campo. Tiempo estimado: 20 min de máquina + 5 min de banco por cada firmware a probar.

---

## ⚠️ CORRECCIÓN DEL 03/08 — LA VENTANA DE ABAJO ESTÁ MAL

**Ejecutado el 03/08.** Los binarios están en `05_Funcional/bisect_entregable/`. Al compilarlos
aparecieron dos errores de este encargo, los dos medidos antes de darlos por buenos:

**1. Ninguno de los 8 candidatos de abajo toca el camino del ciclo.** Cruzados commit contra
fichero, ninguno modifica `coordinador.cpp`, `modo_automatico.cpp`, `semaforo.cpp` ni
`mando.cpp`. En concreto `470a5c9` (N-23) —el *"sospechoso principal"*— toca solo `lcd.h`,
`lcd.cpp` y `modo_hora.cpp`: cambió quién mueve el coordinador **desde la pantalla AJUSTAR
HORA**, no desde el ciclo automático. La frase *"cambió quién y cuándo mueve el coordinador, que
es exactamente la máquina que hace correr el ciclo"* es cierta a medias y llevaba a mirar mal.

**2. La ventana empieza demasiado tarde.** El responsable de banco sitúa el último firmware
bueno hacia las **16:00 del sábado 01/08**. Los 8 candidatos van de las **17:58 a las 19:51**:
son todos posteriores al fallo, así que **los 8 fallarían y no dirían cuál fue**. Cargarlos en
ese orden habría gastado la sesión de banco entera para concluir *"es más antiguo"*.

**Sospechoso nuevo, por hora y por ficheros:**

```
2779d9b  01/08 16:00  feat(SFTY-21): Modo Degradado y mando de reles en Maestro y Esclavo
```

Cae en la hora exacta que da el banco y toca **15 ficheros del Maestro** — el cambio más grande
de ese día. El mando de relés **intercepta las escrituras de pines de luz** en vez de rodearlas,
que es justo el mecanismo por el que el ciclo podría dejar de avanzar.

**Descartados por lectura, para que nadie los repita:** que un cristal muerto bloquee el ciclo
vía los guardas de reloj del coordinador *(los dos que están en el bucle bajan la bandera y
siguen sin `return`: no detienen nada)*, y que `reloj_actualizar()` de N-25 se atasque con el
RTC parado *(sale en `LSERDY == RESET`, sin llamada bloqueante)*.

➡️ **Orden de carga y árbol de decisión: `bisect_entregable/LEEME.txt`.** Bastan 3 cargas.

---

## Contexto mínimo

```
Proyecto : D:\@Proyect\Controladora_Semaforos
Rama     : feat/n15-reloj-pantalla-hora
Lee      : CLAUDE.md (reglas permanentes) y ESTADO.md (estado vivo). NO hace falta el roadmap.
Campo    : V8.4 (e303485). NO SE TOCA.
```

## El hecho

**El Modo Automático dejó de funcionar entre dos compilaciones.** Confirmado en banco por el
responsable del equipo:

- La compilación **anterior sí funcionaba**. La **última no**.
- La línea `RF:` de la pantalla del Maestro **muestra un porcentaje**, y cambia a `SIN ENLACE ---`
  al desconectar la radio. **El enlace está vivo y la telemetría es correcta.**
- En `AJUSTAR HORA` el **Botón 3 sí avanza** de dígito. **No es un botón trabado.**

## Lo ya descartado leyendo el código — no repetir

| sospechoso | por qué cae |
|---|---|
| Falta de radio / SFTY-6 | `RF:` da porcentaje. Hay enlace. |
| Botón trabado en LOW (N-26 al revés) | el Botón 3 responde. |
| Colisión half-duplex de la sincronización con el `ACK_GREEN` | `coordinador.cpp:334` ya excluye `C_ESPERANDO_ACK_GREEN`/`_RED`/`C_FALLO` de `atenderSincronizacion()`, y el PING está suprimido en las mismas condiciones. |
| El reintento del cristal de N-25 comiéndose el bucle | `reloj_actualizar()` sale por la primera línea si el RTC ya cuenta, y si no lee un flag **cada 30 s**. No bloquea. |

## Lo que hay que hacer

**Bisección con firmware real.** No hay atajo por lectura: hay que encontrar el commit exacto.

### 1. Compilar los candidatos

Commits que tocan el *runtime* del Maestro entre la versión buena y la mala, del más reciente al
más antiguo:

```
f37581f  N-38  lcd.cpp, titulos a 7x14B
8a45ae7  N-31  REINICIAR RELOJ
e604273  N-30  mensaje "Esclavo no responde"
e787a29  N-25  reintento del cristal en segundo plano
71f8de2  N-24  sin oscilador no se acepta la hora
470a5c9  N-23  la sincronizacion mueve el coordinador  <-- SOSPECHOSO PRINCIPAL
b581000  N-21  siembra del estado de los botones       <-- SOSPECHOSO PRINCIPAL
4d37db6  N-17  arranque sin esperar al cristal
```

Para cada uno:

```bash
git worktree add ../bisect_<hash> <hash>
cd ../bisect_<hash>/01_Firmware/Maestro
C:/.platformio/penv/Scripts/platformio.exe run
# guardar .pio/build/maestro/firmware.bin como firmware_<hash>.bin
```

**Empezar por `470a5c9` (N-23)**, que es el punto medio y además el sospechoso principal: cambió
quién y cuándo mueve el coordinador, que es exactamente la máquina que hace correr el ciclo.

### 2. Entregar los binarios etiquetados

Una carpeta con `firmware_<hash>.bin` y un `LEEME.txt` de tres líneas por cada uno diciendo qué
cambió ese commit. Quien esté en el banco los carga en orden y dice cuál es el primero que falla.

> **Carga por SWD: `mode=UR` con `-e all`. Si falla, se reintenta — NO se cambia a `HOTPLUG`.**
> Ver `CLAUDE.md` §9.

### 3. Reportar

Un informe corto: qué se compiló, qué flash dio cada uno, y qué falta por probar en banco. **No
proponer arreglos todavía**: primero saber cuál es el commit.

---

## Dos advertencias que evitan perder el día

**1. Los simuladores están en verde sobre el firmware que falla.** La compuerta da
`9 PASS | 2 FALLA | 0 ABORTADO` y el arnés de pantalla `241/241` — sobre esta misma versión que no
arranca el ciclo en banco. **Pasar los simuladores es necesario y NO es suficiente**: son modelos
escritos a mano que *validan el modelo, no el código*. Esta regresión es justo de las que el
modelo no puede ver. Si el informe dice "los simuladores pasan, luego está bien", el informe está
mal.

**2. `ABORTADO` no es `PASS`.** Si algo no pudo correr, se dice con esa palabra. Ver `CLAUDE.md` §2.

## Lo que NO entra en este encargo

- No arreglar nada todavía.
- No tocar `Esclavo/` ni el banco de packs.
- No mover ningún fichero de sitio: los validadores parsean el fuente **por ruta** y un movimiento
  sin actualizarlos los deja midiendo código que ya no existe (`CLAUDE.md` §5).
