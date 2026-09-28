Complementa a orquestador:entregar. Lo que aqui choque con esa skill, manda aqui.

## 1. Que lleva el paquete al funcional (decision de Diego, 28/09/2026 -- deroga la lista generica)

Va, y solo esto:

- **Fuente para PlatformIO** de `{Maestro,Esclavo,Repetidor,ESP32_Expansion}` bajo `01_Firmware/`
  -- sin `Simulaciones/`, sin `Validacion_*`, sin el `README.md` de cada carpeta.
- **La APK** compilada (ver seccion 3).
- **La guia/formulario `.html` de pruebas vigente** de `05_Funcional/` -- el que Diego este usando
  en la sesion en curso, el que genera un PDF al rellenarlo (hoy: `Pruebas_Funcional_2026-09-28.html`;
  el nombre lleva fecha y caduca solo -- si hay mas de uno con fecha reciente, se confirma con Diego
  cual toca antes de mandarlo. `Guia_Cableado_y_Pruebas_Banco.html` es de banco, no de campo: no se
  manda salvo que se pida).
- **`LEEME_PRIMERO.txt`**, corto, texto plano (no `.md`, no `.htm`), con exactamente este orden:
  1. Si paso banco o no, con esas palabras (`SIN BANCO` si no paso). Nunca abre con una cifra en verde.
  2. Que trae el paquete (la lista de arriba, con nombres de fichero reales).
  3. Que hacer con ello (compilar y grabar por PlatformIO; instalar el APK).
  4. Nombre del APK y su **SHA-256 recalculado sobre el fichero que se envia**.

No va: ningun `.bin`, ningun `.md`, el acta de la compuerta (se queda en `evidencia/` del repo), los
manuales `.docx` viejos de `05_Funcional/`. Diego los quito del paquete del 28/09 porque "marean, un
monton no aplican o son referenciales". Las SPEC `.docx` **solo si Diego las pide expresamente** en
el encargo -- entonces van aparte, no dentro de este paquete por defecto (ver seccion 6).

**No existe variante "paquete para auditor".** Nadie la ha decidido: si alguien la pide, se pregunta
que lleva antes de armarla, no se improvisa sobre esta receta.

**Si el SHA-256 que compila el funcional no coincide con el nuestro:** se anota en el LEEME que
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

## 4. El `.zip`: de donde sale y donde se deja

**Contenido desde `HEAD`, nunca del disco**, salvo el APK (compilado aparte, no versionado):

```python
import subprocess, zipfile
contenido = subprocess.run(["git", "show", f"HEAD:{ruta}"], cwd=repo, capture_output=True).stdout
```

`git ls-files` da las rutas versionadas; leer el arbol de trabajo con agentes escribiendo a la vez
deja el paquete con documentos a medias y el nombre de un commit que no describe lo que lleva dentro.

- **Con `zipfile` de Python, nunca `Compress-Archive`**: muere por el `PSModulePath` que la sesion
  del IDE hereda mezclando modulos de PS7 con los de la extension.
- **Destino: `D:\@Proyect\Entregas_Semaforos\`, nunca la raiz del repo.** El `.zip` no se versiona
  (`.gitignore`) y no se deja suelto donde otro commit lo recoja por accidente.
- Nombre: `Paquete_Semaforos_<fecha>_<hash>_SIN_BANCO.zip` (o sin el sufijo si el banco ya paso).
- Antes de comprimir: recuento de artefactos de compilacion (`.pio/`, `build/`, `__pycache__`,
  `node_modules`) = **0**; el SHA-256 del APK que va dentro comprobado, no copiado; el LEEME cita el
  nombre exacto del APK que le acompana.
- Cero diferencias entre el contenido del zip y `HEAD`, comprobado por hash documento a documento.

## 5. Las cifras del LEEME salen de la ultima acta, con el arbol quieto

La compuerta puede servir un binario incremental viejo tras intercambiar fuentes o con dos agentes
tocando el arbol a la vez: una cifra de flash puede ser correcta y ser de un binario que ya no
existe. **La pasada que da las cifras se corre con el arbol quieto y se confirma con una segunda**;
si no coinciden, manda la segunda. Si el LEEME no lleva ninguna cifra de flash/RAM (no lo exige
Diego para el paquete al funcional), esta seccion no aplica: no se inventa una cifra para rellenar.

## 6. Las SPEC `.docx`, solo si se piden

`python 05_Funcional/convertir_a_word.py` regenera **todos** los `.docx` de golpe, incluidos los que
otro agente tenga en vuelo -- comprobar que cada `.md` tocado tiene su `.docx` tocado tambien, no
fiarse del mensaje de exito. Defectos medidos del conversor: parte celdas de tabla por `|` sin
respetar `\|` y trunca la fila (ninguna barra dentro de una celda); aplasta un bloque `>` entero en
un parrafo (huecos y pasos numerados van en vinetas o bloques cercados, nunca en una cita). Si Diego
pide las SPEC, van **aparte** de este paquete, no mezcladas con la lista de la seccion 1.
