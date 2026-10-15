Complementa a orquestador:entregar. Lo que aqui choque con esa skill, manda aqui.

## 1. Que lleva el paquete al funcional (decision de Diego, 28/09/2026 -- deroga la lista generica)

Va, y solo esto:

- **Fuente para PlatformIO** de `{Maestro,Esclavo,Repetidor,ESP32_Expansion}` bajo `01_Firmware/`
  -- sin `Simulaciones/`, sin `Validacion_*`, sin el `README.md` de cada carpeta.
- **La APK** compilada (ver seccion 3).
- **La guia/formulario `.html` de pruebas vigente** de `05_Funcional/` -- el que Diego este usando
  en la sesion en curso, el que genera un PDF al rellenarlo (hoy: `Pruebas_Funcional_2026-10-16.html`;
  el nombre lleva fecha y caduca solo -- si hay mas de uno con fecha reciente, se confirma con Diego
  cual toca antes de mandarlo. `historico/Guia_Cableado_y_Pruebas_Banco.html` es de banco, no de campo: no se
  manda salvo que se pida). Su marcador `XXXXXXX` lo sustituye el empaquetador por el hash del commit que
  empaqueta (seccion 4); la hoja tiene que estar en ese commit.
- **La app web**: `05_Funcional/App_Semaforo/www/` entera, como `App_Web/` (la misma que lleva la APK).

**Sin LEEME** (Diego, 06/10): el funcional ya sabe que un `.zip` nuevo se carga y se prueba. Lo que haya que
decirle (`SIN BANCO`, cargar los dos postes, el SHA-256 de la APK) va en el mensaje de envio.

No va: ningun `.bin`, ningun `.md`, el acta de la compuerta (se queda en `evidencia/` del repo), los
manuales `.docx` viejos de `05_Funcional/`. Diego los quito del paquete del 28/09 porque "marean, un
monton no aplican o son referenciales". Las SPEC `.docx` **solo si Diego las pide expresamente** en
el encargo -- entonces van aparte, no dentro de este paquete por defecto (ver seccion 6).

**No existe variante "paquete para auditor".** Nadie la ha decidido: si alguien la pide, se pregunta
que lleva antes de armarla, no se improvisa sobre esta receta.

**Si el SHA-256 que compila el funcional no coincide con el nuestro:** se le dice en el mensaje que
hacer -- el que carguen el binario que les compilo (con su hash), o que nos pidan el nuestro -- no se
decide en silencio cual usar.

## 2. Que carpetas entran de 01_Firmware

Lista por INCLUSION, no por exclusion -- mas corta y no envejece con carpetas nuevas:

```bash
git ls-files 01_Firmware/Maestro 01_Firmware/Esclavo 01_Firmware/Repetidor 01_Firmware/ESP32_Expansion
```

Cualquier fichero fuera de esas cuatro rutas (incluida `01_Firmware/Controladora_Semaforos/` y
`01_Firmware/Simulaciones/`) no entra. Contar el resultado contra el paquete armado: mismo numero de
lineas.

## 3. Compilar la APK

```bash
cd 05_Funcional/App_Semaforo
cp -r www/. android/app/src/main/assets/public/   # el arbol ENTERO, nunca una lista de ficheros
printf 'sdk.dir=C:/android-sdk\n' > android/local.properties
cd android
export JAVA_HOME="D:/@Proyect/Baliza/7 sw apk/jdk-17/jdk-17.0.12+7"   # JDK 17, NO 21
./gradlew clean assembleDebug --offline
# sale en app/build/outputs/apk/debug/app-debug.apk
```

- **JDK 21** (el de la extension del IDE) muere en `JdkImageTransform ... core-for-system-modules.jar`.
- El SDK de Android no vive bajo ruta con espacios: union a ruta limpia
  (`mklink /J C:\android-sdk "D:\@Proyect\Baliza sw apk\android-sdk"`), nunca mover el SDK.
- `local.properties` con BARRAS NORMALES (`C:/android-sdk`): con `\` la barra se come el caracter
  siguiente. Sigue en `.gitignore` porque lleva una ruta de esta maquina.
- **Nunca `rm -rf assets/public` antes de copiar**: se lleva `cordova.js`, `cordova_plugins.js` y
  `bluetoothSerial.js` (el plugin del socket SPP) -- ninguno viene del repo y su ausencia no la
  delata ningun `<script src=>`. `cp -r www/.` es seguro: anade y pisa, nunca borra.
- **Verificar contenido, no que el build salga verde**: abrir el `.apk` como zip y comparar
  entrada por entrada y por CRC contra `app.js`/`index.html`/`style.css`/`js/*.js` del repo. Dos APK
  con el mismo contenido NO comparten md5 (el contenedor cambia por marcas de tiempo/alineado).
- Antes de copiar: comprobar que `App_Semaforo/*` y `App_Semaforo/www/*` son identicos por md5. Si
  difieren, un cambio entro en una y no en la otra -- se para y se investiga, no se elige una.
- Nombre: `IOT_VIAL_Semaforos_<fecha>_<hash>_SIN_BANCO.apk` -- solo despues de verificar el contenido.
- **Probarla en el telefono de Diego** (personal): se abre SOLO con `am start -n com.iotvial.semaforos/.MainActivity`
  y se maneja por CDP. Nunca `monkey` (inyecta eventos al azar, 06/10) ni `input`. Sin equipo, Bluetooth falso.

## 4. El `.zip`: de donde sale y donde se deja

**Lo arma `herramientas/entrega/empaquetar_funcional.py <fecha> <hoja.html> <acta> [commit]`** (el unico; el
de revision con manuales se retiro el 06/10). Contenido desde `git show <commit>:`, nunca del disco, salvo la
APK: leer el arbol de trabajo con agentes escribiendo a la vez deja documentos a medias bajo el nombre de otro
commit. Sin `[commit]`, `HEAD`.

**Un paquete, un commit (`D-84`, 14/10).** Firmware, `www/`, la hoja, `FW_HASH.txt` y el hash del nombre salen
TODOS de `<commit>`; `HEAD` puede haberse movido (docs, otra tarea en el indice) y no cambia nada de lo que va
dentro. La lista de ficheros es `git ls-tree -r --name-only <commit>`, no `git ls-files` (lee el INDICE: con
otra tarea preparada incluye lo que no esta en ningun commit). Reglas del commit:

- Es ancestro de `HEAD` (`git merge-base --is-ancestor`): un commit colgado o de otra rama no va a campo.
- Su firmware es el que midio el acta (`git diff --name-only <hash del acta> <commit> -- 01_Firmware/{4}` vacio;
  el acta se lee de `HEAD:<acta>`, no se empaqueta). Lo mismo que antes, contra `<commit>` en vez de `HEAD`.
- **La hoja esta en `<commit>`**, o aborta. Si la hoja se escribio despues de la candidata (el 14/10: firmware
  `96c3d32`, hoja `b220f4d`), la candidata ES el commit de la hoja: el firmware sigue siendo el del acta y el
  hash que Marco lee en el PDF es el del arbol que lleva dentro (`CLAUDE.md` §0.2). Leerla del disco o de
  `HEAD` (la receta del scratchpad del 14/10) deja la procedencia de la hoja fuera del zip: no se repite.
- **La APK se comprueba entrada a entrada contra `www/` de `<commit>`** (`D-85`): bytes de cada
  `assets/public/<ruta>` iguales al blob, con `\r\n` normalizado a `\n` (el disco compila con CRLF y el blob
  es LF). El disco solo aporta la APK; que `www/` en disco difiera de `<commit>` no es motivo de aborto. Una
  ruta de `www/` que falte en la APK aborta.
- **Sin `Guia_Conectar_PC_Bluetooth.html`** (seccion 1, 28/09; el paquete `b4390a0d` del 14/10 salio sin ella
  por orden del responsable). El empaquetador la metia desde el 09/10 (`c695ee4`): fuera, y no se exige.

- **Con `zipfile` de Python, nunca `Compress-Archive`**: muere por el `PSModulePath` que la sesion
  del IDE hereda mezclando modulos de PS7 con los de la extension.
- **Destino: `entregas/` del repo (ignorada), nunca la raiz ni fuera del repo.** El `.zip` no se versiona
  (`.gitignore`) y no se deja suelto donde otro commit lo recoja por accidente.
- Nombre: `Paquete_Semaforos_<fecha>_<hash>_SIN_BANCO.zip` (o sin el sufijo si el banco ya paso).
- Antes de comprimir: recuento de artefactos de compilacion (`.pio/`, `build/`, `__pycache__`,
  `node_modules`) = **0**; el SHA-256 del APK que va dentro comprobado, no copiado.
- Cero diferencias entre el contenido del zip y `<commit>`, comprobado byte a byte documento a documento, y un
  solo `.html` fuera de `App_Web/` (la hoja), sin marcador `XXXXXXX` dentro.

**Comprobacion de recepcion del empaquetador** (no esta en la compuerta: necesita la APK en disco y deja un
zip por pasada; la corre quien lo toca, con el arbol como este). Vista en rojo el 15/10 sobre el script de HEAD
`19976b5`, con `www/` en disco distinto de `HEAD` por otra tarea:

```bash
python herramientas/entrega/empaquetar_funcional.py 2026-10-14 \
  05_Funcional/historico/Pruebas_Funcional_2026-10-14.html evidencia/2026-10-14_compuerta.txt b220f4d
```

- Rojo de hoy: con cuatro argumentos imprime el uso; sin el cuarto, `ABORTA: el disco no es HEAD en
  05_Funcional/App_Semaforo/www/app.js` (el fallo del 14/10, reproducido).
- Verde: `OK entregas/Paquete_Semaforos_2026-10-14_b220f4d_SIN_BANCO.zip`; sus 107 ficheros de `01_Firmware/`
  y los de `App_Web/` iguales byte a byte a los de `Paquete_Semaforos_2026-10-14_96c3d32_SIN_BANCO.zip` (el
  de Marco; mismo arbol, `git diff 96c3d32 b220f4d -- 01_Firmware/{4} www/` vacio); `FW_HASH.txt` = `b220f4d`;
  un solo `.html` fuera de `App_Web/`; ninguna `Guia_Conectar_PC_Bluetooth.html`. Se compara con `zipfile`
  desde fuera del script, no leyendo su `OK`.

## 5. Las cifras que se citan salen de la ultima acta, con el arbol quieto

Si un mensaje cita una cifra de flash o RAM, sale de una compuerta corrida con el arbol quieto y
confirmada con una segunda: un binario incremental viejo da una cifra correcta de algo que ya no existe.

## 6. Las SPEC `.docx`, solo si se piden

`python 05_Funcional/convertir_a_word.py` regenera **todos** los `.docx` de golpe, incluidos los que
otro agente tenga en vuelo -- comprobar que cada `.md` tocado tiene su `.docx` tocado tambien, no
fiarse del mensaje de exito. Defectos medidos del conversor: parte celdas de tabla por `|` sin
respetar `\|` y trunca la fila (ninguna barra dentro de una celda); aplasta un bloque `>` entero en
un parrafo (huecos y pasos numerados van en vinetas o bloques cercados, nunca en una cita). Si Diego
pide las SPEC, van **aparte** de este paquete, no mezcladas con la lista de la seccion 1.
