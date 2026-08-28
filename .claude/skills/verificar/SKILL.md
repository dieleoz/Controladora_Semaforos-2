---
name: verificar
description: Verifica el firmware con la compuerta e interpreta el resultado sin tragarse los falsos verdes. Uso cuando haya que comprobar si algo esta bien antes o despues de tocar codigo, correr el banco, conectar un arnes nuevo, o revisar trabajo delegado a otro agente. Incluye las cuatro trampas ya medidas en este repo: el toolchain que compila pero no enlaza, la comprobacion que ningun firmware puede aprobar, la que nunca examina un candidato, y el arnes que solo se adapto a la firma.
---

# Verificar la Controladora de Semaforos

Un semaforo que falla mal mata a alguien. Aqui verificar no es un tramite.

## 1. El comando

```
python 01_Firmware/compuerta.py            # completo (compila)
python 01_Firmware/compuerta.py --rapido   # sin compilar
```

Codigo de salida: `0` PASS · `1` FALLA · `2` ABORTADO. Escribe acta en `evidencia/`.
**Las cifras del README se copian del acta, nunca se escriben a mano.**

Un pack suelto, en un segundo:

```
python 01_Firmware/Simulaciones/banco/correr.py --listar
python 01_Firmware/Simulaciones/banco/correr.py --pack esclavo_03
```

## 2. Las tres palabras, que no son dos

| | significa |
|---|---|
| `PASS` | corrio y el firmware cumple |
| `FALLA` | corrio y el firmware **no** cumple |
| `ABORTADO` | **no pudo correr** — no dice *nada* del firmware |

Tratar `ABORTADO` como aprobado es como el Maestro estuvo dias sin cobertura sin que nadie
se enterara.

## 3. Antes de creerte un verde, comprueba estas cuatro

Este repo se comio las cuatro. Ninguna es hipotetica, y **todas dieron numeros creibles**.

**a) El total no se movio al anadir algo.** Si escribes una pantalla o una comprobacion y el
arnes sigue dando el mismo numero, **no la estas midiendo**. Es la trampa mas barata de
detectar y la que mas veces ha mordido: `CONSULTA RELOJ` dio 115/115 antes y despues, y el
aviso `>48h` dio 259/259 antes y despues porque el arnes se habia *adaptado a la firma* sin
ejercer la rama nueva -pasaba `minSync >= 2880` con un maximo de 2759, siempre falso-.

> **Apunta el total ANTES de tocar nada.** Sin ese numero no hay forma de saberlo despues.

**b) Una comprobacion que NINGUN firmware puede aprobar.** El alias de +-60 s de `CMD_DELTA`
se dejaba fallando *a proposito* para que el limite no se olvidara. Pero un byte de segundos
no distingue 0 de 60: esa comprobacion no la podia aprobar nadie, **jamas**. Con ella dentro
la compuerta no salia en verde nunca, y un codigo de salida que no cambia ensena a ignorarlo.

> Si una comprobacion no la puede aprobar ningun firmware posible, **no es una comprobacion,
> es una nota**, y va en `reportar()`, que no cuenta. En su lugar se exige lo que SI se puede:
> que el agujero sea *exactamente* el que el protocolo obliga y ni un caso mas.

**c) Una comprobacion que nunca llega a examinar un candidato.** Peor que la anterior, porque
sale en VERDE. La prueba 2.8 de `maestro_02` hacia `break` sobre una condicion que siempre se
cumplia y **no evaluaba ni un solo caso**: llevaba meses dando `PASS` sin medir nada. Al
arreglarla aparecio un camino explotable de verdad -permutar `FLAGS` y `SYNC_BAJA` deja la
suma intacta y enciende `CICLO+SYNC+DEGRADADO`, y un arranque tras corte lo leeria como
autorizacion vigente-.

Su hermana: la misma prueba 2.7 contaba `C(5,2)=10` pares ciegos **sin llamar nunca al
checksum real**, porque `PESOS_SUMA` era un resto del algoritmo anterior fijado a `1`. El
mensaje decia *"con los pesos leidos del C++"*. Medidos de verdad eran 8.

> **Un numero redondo que coincide con "todas las combinaciones posibles" es sospechoso.**
> Y un `PASS` de una prueba que nadie ha visto fallar no vale mas que un comentario.

**d) La suite sale `[OK]` pero imprime `FALLA` dentro.** La compuerta decide por el **codigo
de salida**. `validador_maestro.py` imprimia `FALLA` y salia con `0`, asi que se pintaba en
verde; uno de esos fallos era **vial**. Se cerro retirando los tres monoliticos (N-46), asi
que hoy no deberia poder repetirse — pero si algun dia se anade una suite nueva a
`compuerta.py`, comprueba que su codigo de salida cambia cuando falla, antes de fiarte.

## 4. Si algo sale ABORTADO, sospecha del instrumento antes que del firmware

> 🔴 **Y no lo apuntes para luego: un ABORTADO es una puerta abierta (N-75).** Mientras un
> instrumento esta abortado, **todo lo que vigilaba entra sin mirar**. Dos ABORTADO a la vez -el
> banco entero y el arnes de DOM- dejaron pasar cuatro defectos de la app: quedo sorda a la
> telemetria y pintaba un estado inventado, se autorizaba sola inyectando el PIN en cada comando,
> parseaba un protocolo que ninguna punta habla, y perdio comandos que el firmware si atiende.
> Eran justo los dos unicos instrumentos que EJERCEN la app. **Se arregla antes de mirar nada mas.**

> 🔴 **La compuerta no corre solo con Python.** De sus 15 comprobaciones: 7 son `.py` puras,
> 3 necesitan PlatformIO + ARM GCC, 4 el `gcc` de host y **2 necesitan `node`**. Sin `node` esas dos
> salen ABORTADO y la compuerta devuelve **`2`, no `0`** — y son las dos que **ejecutan** la app en
> vez de leerla. Un entorno sin node no da un audit mas pequeno: da el audit ciego por el lado que
> ya fallo una vez.

> Un "no aparece" no es un hallazgo hasta haber descartado al buscador.

- **`gcc` que no enlaza:** el toolchain valido esta en `D:\toolchain\mingw64\bin`. El de winget
  esta bajo una ruta con `n` con tilde y su `ld` no encuentra `crt2.o` aunque el fichero exista
  (N-44). La compuerta ya exige enlazar un `main()` vacio antes de fiarse de un `gcc`.
- **`Get-FileHash` y otros cmdlets que "no existen":** el `PSModulePath` de una sesion de IDE
  mezcla modulos de PowerShell 7 con los de la extension y rompe el autocargado de PS 5.1.
- **Rutas:** los packs **parsean** el fuente por tuplas de ruta. Mover o renombrar un `.cpp`
  rompe un instrumento; el movimiento y la actualizacion van en el **mismo commit**. La guarda
  de rutas censa `Simulaciones/`, `banco/packs` y `banco/modelos`, y lleva un SUELO: si
  encuentra sospechosamente pocas rutas **aborta en vez de aprobar**. Al retirar los
  monoliticos cayo a 4 y aborto, que es exactamente su trabajo.

## 4.bis Al cambiar comportamiento, los instrumentos se quedan atras — y eso es ABORTADO

Cuando cambias el firmware a proposito, los validadores en Python dejan de saber medirlo. La
compuerta lo marcara **ABORTADO**, y esta bien: no acusa al firmware de nada.

```
[ABORTADO] no se pudo leer del C++ la constante 'segundosDelDia\s*/\s*(\d+)UL'
```

Eso es la guarda funcionando: la constante ya no existe, y el modelo **aborta en vez de caer a un
valor por defecto**. Actualizar el instrumento es parte del cambio, no un paso posterior.

**Y OJO CON ESTO, que es lo que muerde:** un banco maduro contiene **pruebas que EXIGEN el
comportamiento defectuoso**. Se escribieron cuando el defecto se creia inevitable y documentan su
coste con honestidad. Al arreglarlo, **fallan**.

No las reescribas en bloque para que pasen: eso es ajustar el instrumento hasta que de verde. Van
una por una y cada una acaba, anotada, en uno de tres sitios: **se borra** (solo documentaba el
defecto), **se invierte** (exige lo nuevo) o **se conserva** (medía otra cosa).

**El mejor termometro del arreglo son los fallos que desaparecen solos.** Si arreglas la causa raiz
de cuatro FALLA y tras actualizar el instrumento siguen ahi, el arreglo no esta completo.

## 5. Al conectar un arnes nuevo: rompele el firmware a proposito

Tener controles negativos escritos dentro es una etiqueta, no una prueba. Inyecta un defecto
en el `.cpp` **real**, corre, y exige que **baje la cuenta y cambie el codigo de salida**.
Luego restaura y confirma con `git diff HEAD` **vacio** — no con la impresion de haberlo
restaurado.

Y conectarlo a `compuerta.py` es parte de escribir el arnes, no un paso posterior: un
instrumento que no esta en la compuerta no mide nada, y **no deja rastro de que falta**.

## 6. Revisar trabajo delegado: por el diff, no por su informe

Un informe -propio o de un agente- **no es una medida**.

- Si el cambio toca **el firmware y su modelo a la vez**, mira si el modelo *replica* el
  arreglo o si *relaja* la comprobacion. Solo lo primero vale.
- Una cifra en verde tras tocar las dos puntas no demuestra nada: las copias pueden mentir
  juntas. Ya paso.
- Antes de escribir una causa en `roadmap.md` o `ESTADO.md`, **reproduce y pega la salida**.
- Una causa que se cae **se marca refutada, no se borra**. Y una refutacion tambien es un
  instrumento: tumbar algo exige el mismo rigor que afirmarlo.

## 7. Lo que ninguna de estas cifras sustituye

**La prueba de banco.** Campo va por V8.4; todo lo de V8.5 en adelante esta validado en
simulador y **sin banco completo**. Carga por SWD con `mode=UR` y `-e all`, y no se cambia: si
falla, se reintenta, no se cambia el modo.

> **Desde el 05/08 la compuerta puede salir en VERDE, y eso la hace mas peligrosa, no menos.**
> Mientras salia con `1` nadie la confundia con un permiso. Un `0` si se confunde. Lo que ese
> `0` dice es: los modelos y los arneses de PC no encuentran nada. **No dice que el firmware
> funcione en la tarjeta** — de hecho, con la compuerta en verde hay una regresion abierta en
> banco donde el Modo Automatico no mueve las luces.
>
> Verde **no** es entregable. Al funcional no sale nada sin banco pasado.

Estado de hoy en `ESTADO.md`. Reglas permanentes en `CLAUDE.md`. Historico en `roadmap.md`.

## Quinta trampa: el hueco que no grita (26/08/2026)

Las cuatro anteriores son cosas que **fallan**. Esta no falla: **falta**, y por eso es la peor.

- **`pinMode()` sin `digitalRead()`.** `CAM_UMBRAL_PIN` se configuraba como entrada y no lo leia
  nadie, mientras cuatro manuales describian su funcion. Compila, pasa la compuerta, y no hay
  ninguna prueba que lo eche de menos.
- **Dos copias de una regla de seguridad que divergen.** El enclavamiento SFTY-2 llevaba una
  sentencia de mas **solo en el Esclavo**. No cambiaba el comportamiento de hoy: era una trampa a
  plazo esperando a que alguien anadiera una transicion rojo+ambar.

**Como se cazan, y no es leyendo:** censando y comparando.

```bash
# que entradas se declaran pero nadie lee
grep -rn "pinMode(" --include=*.cpp Maestro/src Esclavo/src
grep -rn "digitalRead(" --include=*.cpp Maestro/src Esclavo/src

# que difiere entre las dos puntas en un fichero que deberia ser igual
diff --strip-trailing-cr Maestro/src/semaforo.cpp Esclavo/src/semaforo.cpp
```

**Y la senal de alarma en los comentarios:** si el fuente delibera —*"Wait, in X state..."*,
*"let's just..."*, en ingles, en primera persona— es que quien lo escribio no lo tenia claro y lo
dejo asi. En una regla de seguridad, eso se investiga siempre.
