Complementa a orquestador:verificar. Lo que aqui choque con esa skill, manda aqui.

## 1. Los comandos

```bash
python 01_Firmware/compuerta.py             # completo, compila. Acta en evidencia/
python 01_Firmware/compuerta.py --rapido    # sin compilar
python 01_Firmware/Simulaciones/banco/correr.py --listar
python 01_Firmware/Simulaciones/banco/correr.py --pack <nombre>
```

Corrida anterior para comparar: `ls -t evidencia/*_compuerta.txt | head -1`.

**Despues de un `--rapido` hacen falta dos pasadas completas**: la comprobacion de cifras del acta
lee la corrida ANTERIOR, y un `--rapido` deja un acta sin las filas `compila maestro/esclavo/repetidor`.
La corrida completa siguiente compara contra esa acta mutilada y protesta con razon. No se toca el
pack: se corre la completa dos veces y se copia de la que trae todas las filas.

## 2. El toolchain que compila y no enlaza

`gcc`/`g++` valido por RUTA ABSOLUTA: `D:\toolchain\mingw64\bin`. El del PATH (winget) vive bajo una
ruta con una letra con tilde y su `ld` no encuentra `crt2.o` aunque el fichero exista. Antes de
fiarse de un `gcc` nuevo: hacerle enlazar un `main()` vacio.

## 3. Que necesita cada bloque de la compuerta, y que ABORTA sin ello

La compuerta mezcla comprobaciones que solo leen Python con otras que compilan o ejecutan. Sin la
dependencia, esas filas dan `ABORTADO` y el codigo de salida de la compuerta sube a `2`, no a `0`:

- **PlatformIO + ARM GCC**: filas `compila maestro/esclavo/repetidor`.
- **`gcc` de host (ruta absoluta, ver seccion 2)**: los arneses (`arnes del ciclo`, `arnes del
  respaldo`, `arnes del automatico`, `arnes de las dos puntas`, `arnes del Degradado a dos puntas`,
  `simulador del puente ESP32`).
- **`node`**: `app ejecutada en DOM`, `test unitarios de la app`, `test unitarios TDD de la app`, y
  el simulador del puente ESP32 tambien lo pide. Son justo las que EJECUTAN la app en vez de leerla.

El numero exacto de filas de cada tipo se recuenta contra `01_Firmware/compuerta.py` antes de
confiar en el, no se recita aqui (`grep -n "ABORTADO" 01_Firmware/compuerta.py` da la lista viva).
Un entorno sin `node` no da un audit mas pequeno: da el audit ciego por el lado que ya fallo una vez.

## 4. Grep para el hueco que no grita

```bash
grep -rn "pinMode(" --include=*.cpp Maestro/src Esclavo/src
grep -rn "digitalRead(" --include=*.cpp Maestro/src Esclavo/src
diff --strip-trailing-cr Maestro/src/semaforo.cpp Esclavo/src/semaforo.cpp
```

Una entrada declarada con `pinMode()` sin ningun `digitalRead()` que la lea compila, pasa la
compuerta y ninguna prueba la echa de menos. El `diff` entre las dos copias de `semaforo.cpp` caza
una regla de seguridad que diverge en una sola sentencia entre Maestro y Esclavo.

## 5. Carga en la tarjeta

SWD con `mode=UR` y `-e all`, y no se cambia de modo: si falla, se reintenta -- enganchar es
cuestion de *timing*, y `Unable to get core ID` no es falta de cableado. `HOTPLUG` se engancha al
micro en marcha; con un firmware que se cuelga al arrancar, el watchdog reinicia cada 4 s en mitad
del borrado (`failed to erase memory`), y el delator es `NVM size: 128 KBytes (default)` en un chip
de 64 KB. Radios: `2.4 kbps` de Air Data Rate, `M0`/`M1` en OFF.

## 6. Presupuesto de flash y RAM

El umbral no caduca; la cifra si. Se mira en la ULTIMA acta (`ls -t evidencia/*_compuerta.txt | head
-1`), nunca se copia aqui ni al README. Antes de sacrificar una funcion porque "no cabe", medir de
que esta hecho ese porcentaje **por fichero objeto** leyendo `firmware.map`, no por nombre de
simbolo (los simbolos de cualquier libreria C++ tambien empiezan por `_Z`).

**La RAM no la mide la compuerta.** Se mide con `nm` sobre el `.elf`: un objeto global con
constructor cuesta `.bss` permanente aunque el enlazador tire sus funciones (el enlazador descarta
funciones, nunca un objeto global con constructor en `.init_array`).

**El acta puede publicar el binario ANTERIOR** cuando PlatformIO sirve un incremental viejo tras
intercambiar fuentes o con dos agentes tocando el arbol a la vez. La cifra de flash se confirma con
una segunda pasada del arbol quieto; si no coinciden, manda la segunda.

## 7. Cmdlets que "no existen"

`Get-FileHash`, `Compress-Archive`: el `PSModulePath` de la sesion del IDE mezcla modulos de PS7 con
los de la extension y rompe el autocargado de PS 5.1. Se usa Python (`hashlib`, `zipfile`) en su
lugar, no se persigue el cmdlet.

## 8. Antes de la compuerta completa, la copia de la app en Android

`documentos_03_trama_status` compara `www/` con `android/app/src/main/assets/public/` (ignorada por git): tras tocar la
app, `npx.cmd cap sync android` y `git checkout --` de `capacitor.build.gradle` y `capacitor.settings.gradle` (solo
cambian fin de linea). Y la compuerta converge en 3-4 pasadas cuando cambian las cifras: se copian del acta a
`README`/`ESTADO` y se repite hasta que dos pasadas den lo mismo (02/10: pasadas 3 y 4).

## 9. La APK desde Git Bash

`gradlew.bat` no arranca por el espacio de la ruta: `"$JAVA_HOME/bin/java.exe" -cp gradle/wrapper/gradle-wrapper.jar
org.gradle.wrapper.GradleWrapperMain assembleDebug -q` desde `android/`, con `JAVA_HOME` al jdk-17 de
`D:\@Proyect\Baliza\7 sw apk\`. Se verifica por md5 que `assets/public/` de la APK es `www/`.
