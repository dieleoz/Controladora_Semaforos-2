# Mecanica propia del repositorio — lo que no cabe en `CLAUDE.md`

Cada apartado lleva el numero del apartado de `CLAUDE.md` que lo cita. Aqui va el COMO; la regla esta alli. La cronica
de donde salio cada cosa esta en `HISTORIA.md`.

## §3. Carga por SWD

- `Unable to get core ID` no es falta de cableado: enganchar es cuestion de *timing*. Se reintenta con `mode=UR`; no se
  cambia el modo.

## §4. Como se escribe una guarda (dentro de un pack existente: el simulador esta congelado)

- Se trae el **bloque literal** de la logica ya probada en vez de reescribirla.
- Se releen las constantes del C++ en cada corrida, **sin valor por defecto, nunca**.
- Si dos constantes se relacionan por una desigualdad, la desigualdad se recalcula desde el C++, no se explica en un
  comentario (`N-71`).
- Primitivas: `verificar` cuenta · `propiedad` cuenta y marca `ROTA` cuando el banco logro romper una regla de
  seguridad · `control_negativo` exige que la prueba sepa fallar · `reportar` no cuenta, y es donde va el residual que
  ningun firmware puede aprobar.
- Un instrumento que no esta en la compuerta no mide nada y no deja rastro de que falta: un `ABORTADO` grita, un hueco no
  (`N-43`).

## §6. Inyectar un defecto sin perder trabajo

- Un `enum` de un solo valor que se compara compila a `movs r0,#1`: se mide con `arm-none-eabi-g++ -Os -S`.
- Antes de conectar un arnes, o tras un refactor que mueva la FORMA de un bloque que un pack lee por texto (`N-89`), se
  inyecta un defecto en el `.cpp` real y se exige que baje la cuenta y cambie el codigo de salida.
- Restauracion: copia tomada ANTES de inyectar (`cp` al scratchpad) y verificada por HASH. Con trabajo sin comitear,
  `git checkout -- <fichero>` vuelve a HEAD y se lleva el trabajo, y `git diff HEAD` vacio no lo detecta.

## §10. Flash, RAM y buffers

- Por FICHERO OBJETO leyendo `firmware.map`, no por nombre de simbolo (los de cualquier libreria C++ empiezan por `_Z`);
  el `.map` dice quien arrastra a quien, y un delta exige medir los DOS extremos.
- Un camino muerto que no cuesta flash puede costar RAM: el enlazador no descarta un objeto global (constructor en
  `.init_array`, `.bss` permanente).
- El acta puede publicar el binario ANTERIOR cuando PlatformIO sirve un incremental viejo (fuentes intercambiadas, dos
  agentes en el arbol): si dos pasadas no coinciden, manda la segunda.
- Una cota se acota donde se produce y se DERIVA de la constante que manda; no se agrandan buffers. Una trama truncada
  sale bien formada hasta la mitad, no casa el CRC, y el sintoma es «el equipo se callo», que manda a mirar el cable.
- Un `.cpp` por concepto, `static` para lo privado, header corto; las vtables cuestan flash que no hay.

## §11. Trampas de git con varios agentes

- `git add <valido> <ignorado>` devuelve error PERO deja los validos preparados: con `add ... && commit` el commit no
  corre y se los lleva el commit SIGUIENTE.
- `git add <valido> <ruta_que_no_existe>` no prepara NADA, y el commit sale con un mensaje que describe cambios que no
  lleva. Las dos se cubren leyendo `git diff --cached --name-only` y contando las lineas esperadas.
- Antes de comitear se mira `MERGE_HEAD`: un commit puede cerrar una fusion abierta.
- Editar con un script: `open(ruta, "w")` TRUNCA antes de escribir; si `.write()` falla (un `UnicodeEncodeError`) el
  fichero queda vacio. Se escribe a un temporal y se renombra, o se usa la herramienta de edicion; comitear antes de un
  script masivo es la red.
- Un borrado se delimita por SUS DOS extremos leidos, nunca por «el siguiente delimitador» (se lleva el bloque de al
  lado); despues se cuenta: si el fichero pierde mas lineas que el bloque, se llevo algo mas.
- `git worktree remove --force` sigue los enlaces que el agente dejo dentro y borra el ORIGINAL (un junction a
  `node_modules`): se censan los enlaces, se quita el ENLACE (`Delete(ruta, false)`), se comprueba el destino y despues
  el resto. Al encargar: que lo retire el agente.
- Un agente REANUDADO puede escribir en el arbol principal: su worktree se retira solo si termino sin tocar nada. Antes
  de integrar por parche, `git -C <ruta> diff` (falla en seco si no existe); si no existe, el trabajo esta en el arbol
  principal. Solo lo hace inofensivo comitear con rutas explicitas.
- No se reescribe la historia publicada (`push --force` con la rama en dos remotos): se anota donde vive el cambio.

## §14. Cifras, rotulos y copias

- Hacen falta varias barridas de `grep`: cada una ve lo que la anterior no podia (la cifra derivada, la palabra a
  medias).
- Una cifra caducada se sustituye; una contradiccion hay que reescribirla, y `sed` no sabe la diferencia: despues de
  sustituir se buscan `FALLA`, `sigue`, `espera`, `abierto`, `la que cae`. Igual con un rotulo cuyo sujeto murio.
- `app.js` vive cuatro veces: `www/`, la raiz de la app, `android/assets/public/` y `build/`. Antes de compilar,
  `md5sum` de las tres primeras: identicas o no se compila.

## §15. Como se escribe una spec

- Desde `DECISIONES.md` vigente y desde el fuente, nunca desde un manual anterior: el manual viejo se abre para censar
  que cubre, y ninguna frase suya entra sin verificarla contra el codigo. Una `A-x` abierta se nombra como HUECO.
- Un requisito = una spec, por debajo de 300 lineas; el reparto nombra lo que queda sin dueno.
- Cada afirmacion se clasifica: implementada y ejercida · implementada sin ejercer · NO implementada (a «HUECOS
  MEDIDOS») · no comprobable desde el fuente.
- La caducidad de un documento se mide por AUSENCIA de lo que el firmware hace hoy, buscando por los dos nombres
  posibles (`CAM:` y `CAM`) y mirando que hay dentro antes de jubilarlo.
- A un documento que recita una constante se le retira el numero y queda la consecuencia en palabras. Una decision que
  aun puede cambiar no se propaga a los manuales; se hace cuando el sujeto esta quieto.
- Excepcion a los manuales congelados: los `.html` de campo (guia de camaras y de cableado) se corrigen siempre.

## §16. Archivar

- `05_Funcional/historico/` sigue en git, consultable: ahi se MUEVEN los historicos (R100), no se marcan CRUDAS.
- `99_Legacy/` esta fuera de git ENTERO desde el 02/10 (`.gitignore` raiz; su `LEEME.md` tampoco se versiona): vive
  solo en el disco y el responsable la borra al entregar en campo.
- `git mv` a `99_Legacy/` deja los ficheros SEGUIDOS en la ruta nueva: se mueve en disco y se registra la BAJA; el
  indice sale con `D`, nunca `R`.
- Un fichero base que pasa de 1.000 lineas se parte: lo vivo se queda, la cronica se muda literal a su `_hist`, y el
  historico publica las dos cuentas (molde: `roadmap.md`).
