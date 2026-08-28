# ENCARGO — Sesión de banco · Controladora de Semáforos

**Fecha:** 5 de Agosto de 2026 · **Rama:** `feat/n15-reloj-pantalla-hora` · **HEAD:** `7b93017`

---

## ⚠️ Léase esto primero: qué es y qué NO es este encargo

**Esto NO es una entrega.** No instale nada de aquí en un cruce abierto al tráfico.

Lo que corre en campo hoy es la **V8.4**. Todo lo que hay en este paquete está validado **solo en
simulador y en arneses de PC**. La suite de verificación del repositorio sale en verde —`11 PASS ·
0 FALLA · 0 ABORTADO`, primera vez en la vida del proyecto—, y precisamente por eso hay que decir
qué significa ese verde:

> Significa que **los modelos y los arneses de PC no encuentran nada**.
> **No significa que el firmware funcione en la tarjeta.**

Y la prueba está delante: **con esa suite en verde, hay una regresión abierta en la que el Modo
Automático no enciende las luces sobre hardware real.** Ningún simulador la ve. Por eso existe esta
sesión.

**El producto de esta sesión son MEDIDAS, no un visto bueno.** Si algo no se puede medir, se anota
como *no medido* — nunca como *correcto*.

---

## Orden de la sesión, y el orden importa

| | tarea | por qué en ese orden |
|---|---|---|
| **1º** | Bisección del Modo Automático | Es la regresión viva, en el modo que se usa a diario |
| **2º** | Ajuste de hora y veredicto del cristal `Y2` | **Esta carga borra el binario del bisect.** No se puede hacer antes |
| **3º** | Validación del Modo Degradado | Necesita el reloj ya verificado en el paso 2 |

---

## 1 · Bisección del Modo Automático

### El síntoma

Al cargar esta rama en las dos tarjetas: el enlace de radio va bien (`RF:` da porcentaje), la
botonera responde, pero **el ciclo del Modo Automático no mueve las luces**.

### ⚠️ El sospechoso principal ya está DESCARTADO — no lo persiga

Se sospechaba que una bandera interna (`senalActiva`, del mando de relés) se quedaba pegada y
congelaba las salidas. **Se ha refutado en software**, ejerciendo el código real sobre las salidas
de pines en nueve caminos distintos —las tres secuencias del mando, secuencia a medias, pulsos A y
B simultáneos, reinicio a medio destello, y 600 pulsos aleatorios—. En ninguno se queda pegada.

**Consecuencia práctica:** esto ya **no** es *"confirmar que el culpable es tal commit"*. Es una
bisección de verdad, sin sospechoso. No gaste cargas intentando reproducir la hipótesis vieja.

*(Alcance de esa refutación: cubre el Maestro corriendo solo. Quedan fuera el Esclavo físico, el
protocolo de radio real y el* timing *del relé en hardware. La causa sigue sin conocerse.)*

### Procedimiento

1. **La primera carga es el ancla `c72700e`, y su único trabajo es FUNCIONAR.**
   Si el ancla **no** arranca el ciclo, **pare la sesión**. Significa que la ventana de búsqueda
   está mal planteada y seguir solo consume cargas. Anótelo y devuélvalo.
2. Si el ancla funciona, siga el árbol de carga de `bisect_entregable/LEEME.txt`.
3. **Compare por hash, nunca por tamaño.** Hay binarios distintos que pesan lo mismo, y binarios
   idénticos con nombres distintos. Un `md5sum` antes de cada carga ahorra cargas enteras.

### Qué anotar en cada carga

- Hash del binario cargado.
- ¿Arranca el ciclo? ¿Las luces conmutan o se quedan fijas?
- Si se quedan fijas: **¿en qué color?** y ¿el resto de la interfaz responde (pantalla, botones)?
- Lectura de `RF:` en pantalla.

---

## 2 · Carga por SWD — cómo entrar cuando el micro no se deja

**`mode=UR` con `-e all`. No se cambia el modo.** `HOTPLUG` se engancha al micro en marcha: con un
firmware que se cuelga al arrancar, el watchdog reinicia cada 4 s en mitad del borrado y aparece
`failed to erase memory`. El delator es ver `NVM size: 128 KBytes (default)` en un chip de 64 KB.

**Si falla con `Unable to get core ID`: REINTENTE. No cambie el modo.** Enganchar es cuestión de
milisegundos y puede fallar varias veces seguidas. Eso no es falta de cableado.

### Si tras varios reintentos sigue sin entrar

**Método fiable, sin depender del tiempo de reacción de la mano:**

1. Mueva el puente `BOOT0` de `0` a `1` (a 3,3 V).
2. Pulse `RESET` una vez.
3. Lance la carga. El micro arranca en su bootloader de fábrica e **ignora por completo el firmware
   colgado en Flash**, así que entra a la primera.
4. Devuelva `BOOT0` a `0` y pulse `RESET`.

> El método de *"mantener pulsado el reset y soltarlo un segundo después"* funciona a veces y falla
> muchas: el instante útil dura milisegundos. Si se intenta, la regla es mantener `SW1` pulsado
> **antes** de lanzar la carga y soltarlo justo cuando la consola dice `Connecting to target...`.
> Pero si falla dos veces, use `BOOT0`: es determinista.

---

## 3 · Veredicto del cristal `Y2` — y cómo NO equivocarse

**Contexto honesto:** no sabemos si el cristal está mal. Se dio por culpable una vez apoyándose en
una pantalla que acusaba **sin haber medido nada**, y mandó a cambiar componentes sanos. Esta vez se
mide.

### Procedimiento

> `CONFIGURACION` → `AJUSTAR HORA` → poner la hora y confirmar.

**`CONSULTA RELOJ` no es una opción de menú y no se puede buscar.** Aparece sola, ahí dentro, y
**solo si el reloj no arranca**.

| lo que ocurre | qué significa |
|---|---|
| **La hora queda puesta y no aparece ninguna pantalla de error** | ✅ **Ese es el veredicto: el reloj arranca y el cristal NO era el problema.** No cambie ningún componente. Anótelo y siga |
| Sale `SIN RELOJ` o `SIGUE PARADO` con la línea `Mira CONSULTA RELOJ` | Siga leyendo abajo |

### Si aparece `CONSULTA RELOJ`

```text
   CONSULTA RELOJ          ●     <- este punto debe parpadear
   ─────────────────────────
   LSE ON:1 RDY:0 BYP:0
   RTCSEL:1 EN:1 CFG:1
   CNT:1043 A:26
   Pedido, no oscila            <- la línea que resume
```

**Anote las cuatro líneas tal cual**, incluida la última:

| última línea | qué dice |
|---|---|
| `Oscila y atado a LSE` | el reloj funciona — **el problema no es el cristal** |
| `Pedido, no oscila` | se pidió el oscilador y no arranca — **aquí sí apunta al `Y2` y su entorno** |
| `LSE no se pide` | el firmware no lo pidió — no es fallo de hardware |
| `Oscila; RTC no atado` | el cristal oscila pero el reloj no cuelga de él — configuración |
| `LSE en BYPASS ext.` | espera un reloj externo que no está |

### ⚠️ Dos trampas de lectura, y las dos invalidan el diagnóstico

- **Si el punto de arriba a la derecha NO parpadea**, lo que está parado es el **firmware**, no el
  reloj, y esta pantalla no dice nada del cristal. Anote *"no medido"*.
- **`CNT:sin RTCEN` no es `CNT:0`.** Significa que no se llegó a leer el periférico. Un `0` sería
  indistinguible de un contador parado, que es un diagnóstico distinto.
- Si el punto parpadea y **`CNT` no cambia** entre repintados, el reloj está parado de verdad.

> **No sustituya ningún componente en esta sesión.** El objetivo es traer la lectura. La reparación
> se decide después, con el dato delante.

---

## 4 · Resincronización obligatoria tras cargar (N-49 / N-51)

⚠️ **Esta versión cambia la firma del dominio de respaldo** (`0x5EB0` → `0x5EB1`). Es intencional:
el formato de lo guardado cambió, y aceptar el formato viejo sería leer basura como si fuera válida.

**Consecuencia en la tarjeta:** al primer arranque tras la carga, **cada equipo declarará su
respaldo inválido y arrancará sin sincronización previa.** Es el comportamiento seguro, no un fallo.

**Acción obligatoria antes de probar el Modo Degradado:**

1. Cargar el firmware nuevo en **las dos** tarjetas.
2. Con las radios enlazadas, poner la hora en el Maestro (`CONFIGURACION` → `AJUSTAR HORA`).
3. Confirmar que el Esclavo recibió la hora.
4. **Sin ese paso, el Modo Degradado se rechazará** — y el rechazo será correcto.

---

## 5 · Validación del Modo Degradado

### La regla que hay que confirmar antes que ninguna otra

**El equipo NUNCA entra solo al Modo Degradado.** Al perder la radio, el comportamiento correcto es
caer a **ámbar intermitente** en las dos puntas. Un ámbar le dice al conductor *"nadie controla este
cruce"*; un verde por reloj sin confirmar la otra punta le da confianza falsa. La entrada es
**100 % manual**.

> **Primera prueba, y es de seguridad:** corte la radio y compruebe que **ambas puntas caen a ámbar
> intermitente** y que **ninguna entra sola en Degradado**. Si alguna entra sola, **pare la sesión y
> repórtelo de inmediato.**

### Entrada por pantalla

**Maestro:** `CONFIGURACION` → `MODO DEGRADADO` → `Botón 3` → `CONFIRMAR ENTRADA?` → `Botón 3`.
**Esclavo:** su propio menú (`ESTADO` / `MODO DEGRADADO`) → mismo doble paso.

Dos pulsaciones para entrar, una para salir: la asimetría es deliberada.

Si lo rechaza, **anote el motivo exacto que muestra la pantalla** — ese texto es el dato. Las
condiciones son: reloj en hora · alguna sincronización previa · última sincronización de menos de
2 h · desfase medido · desfase dentro de ±3 s · el Esclavo debe constar con el ciclo.

### El ciclo, y qué cronometrar

Verde 30 s por sentido, todo-rojo 30 s entre medias. Ciclo completo 120 s.

- [ ] Fase 1 (30 s): Maestro 🟢 verde · Esclavo 🔴 rojo
- [ ] Fase 2 (30 s): **las dos en rojo**
- [ ] Fase 3 (30 s): Esclavo 🟢 verde · Maestro 🔴 rojo
- [ ] Fase 4 (30 s): **las dos en rojo**
- [ ] **Sin verde simultáneo en ningún instante** — obsérvelo al menos 3 ciclos completos
- [ ] **Sin verde en una punta contra ámbar en la otra**

### Salida segura

Salga desde la pantalla y compruebe que **las dos puntas terminan en ámbar intermitente**, sin
dejar jamás una en verde y la otra en ámbar.

### Límite de 48 h

No se cronometra en banco. Lo que sí puede comprobarse: a las 44 h la pantalla muestra
`AVISO: LIMITE 48h`, y a las 48 h cae sola a ámbar. Anótelo como pendiente de prueba larga.

---

## 6 · Mando de 4 relés

| secuencia | acción | confirmación |
|---|---|---|
| `A · B · A · B` | intenta entrar en Modo Degradado | 4 destellos **rojos** si acepta · ámbar rápido si rechaza |

**El rojo nunca significa "pase".** Los destellos de confirmación son rojos a propósito: un
destello verde desde el piso podría leerse como autorización de paso.

Con el menú de la pantalla abierto, el mando queda **inhibido** — compruébelo también.

### ❓ Pregunta abierta de hardware, no resuelta (N-19)

Hoy **solo el Maestro tiene receptor de mando**. Para activar el Degradado en el Esclavo hay que
subir a su pantalla.

Si se quiere mando en las dos puntas hace falta **un segundo receptor**, porque el Degradado se usa
justamente cuando **no hay radio**: el Maestro no puede avisar al Esclavo por aire.

**Lo que NO está decidido** es si los dos mandos deben llevar códigos distintos. Un solo código
metería las dos torres en Degradado a la vez desde el piso, saltándose la verificación de cada
punta; códigos distintos obligan a comprobar torre por torre. **Es una decisión de seguridad
pendiente, no una especificación cerrada.** Que el funcional opine antes de comprar nada.

---

## 7 · Qué devolver

1. **Bisección:** hash de cada binario cargado y qué hizo cada uno. Si el ancla falló, dígalo — es
   un resultado, no un fracaso.
2. **Reloj:** si la pantalla `CONSULTA RELOJ` **no apareció**, dígalo así; eso ya es el veredicto.
   Si apareció, las cuatro líneas tal cual y si el punto parpadeaba.
3. **Degradado:** el checklist de fases marcado, y cualquier instante de verde simultáneo o de
   verde-contra-ámbar, con la hora.
4. **Lo que no se pudo probar**, y por qué. Un hueco declarado vale; un hueco callado, no.

> **Nada de esta versión sube a un cruce abierto al tráfico hasta que esta sesión esté hecha y sus
> resultados revisados.**
