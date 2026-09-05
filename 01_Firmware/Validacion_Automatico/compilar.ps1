# Compila y ejecuta el arnes del ciclo automatico en el PC.
#
# Compila coordinador.cpp, semaforo.cpp, modo_automatico.cpp Y mando.cpp REALES
# del Maestro -no una copia, no un espejo en Python- y ejercita el ciclo completo
# sobre ellos. Requiere gcc/g++ (MinGW-w64).
#
# N-52: mando.cpp se suma aqui. Antes el arnes media los pines de verdad pero
# senalActiva -el static de semaforo.cpp que SOLO pone mando.cpp- nunca se ponia a
# true en este binario, porque mando.cpp no se compilaba. La rama
# "if (senalActiva) return;" de aplicarSalidas() jamas se ejercia: el arnes miraba
# los pines pero no recorria el unico camino que puede congelarlos.
#
# UN SOLO BINARIO, como Validacion_Ciclo y a diferencia de Validacion_LCD: aqui no
# se compila nada del Esclavo, asi que no hay colision de simbolos que obligue a dos
# ejecutables.
#
# EL TOOLCHAIN QUE ENLAZA EN ESTA MAQUINA. El de winget (WinLibs por
# BrechtSanders.WinLibs.POSIX.UCRT) vive bajo una ruta con enie y su ld no encuentra
# crt2.o (N-44, ver Validacion_Respaldo/compilar.ps1 y CLAUDE.md 9). El que SI
# enlaza esta en D:\toolchain\mingw64\bin, y va al FRENTE del PATH para esta sesion:
# no se toca el PATH del usuario, solo el de este proceso.
$ErrorActionPreference = 'Stop'
$AQUI = Split-Path -Parent $MyInvocation.MyCommand.Path

$TOOLCHAIN = 'D:\toolchain\mingw64\bin'
if (Test-Path (Join-Path $TOOLCHAIN 'gcc.exe')) {
    $env:Path = "$TOOLCHAIN;$env:Path"
}

if (-not (Get-Command g++ -ErrorAction SilentlyContinue)) {
    Write-Error "No hay g++ de host en el PATH (se busco tambien en $TOOLCHAIN). Sin el, esto NO puede correr: es ABORTADO, no PASS."
}

$RAIZ = Split-Path -Parent $AQUI                      # 01_Firmware
$MAESTRO = Join-Path $RAIZ 'Maestro'
$BUILD = Join-Path $AQUI 'build'

if (-not (Test-Path $BUILD)) { New-Item -ItemType Directory -Path $BUILD | Out-Null }

# OJO AL ORDEN DE LOS -I: este directorio va PRIMERO para que Arduino.h, pines.h,
# botones.h, lcd.h y menu.h se resuelvan contra los sustitutos de aqui y no contra
# los reales del Maestro -que arrastrarian el framework STM32duino, U8g2 y el resto
# de modos, nada de lo cual hace falta para medir el ciclo automatico-.
# protocolo.h, reloj.h, respaldo.h, coordinador.h, semaforo.h, mando.h, modo_ambar.h,
# modo_degradado.h y modos.h SI son los reales: no hay sustituto de esos en este
# directorio -modos.h a proposito: es el enum ModoSistema, y una copia local seria
# justo el "casi igual" que puede divergir sin que nadie lo note-
# (mando.h y las dos ultimas solo necesitan Arduino.h, que ya esta sustituido),
# asi que caen a Maestro\include.
$incluye = @("-I$AQUI", "-I$MAESTRO\include")

$objetos = @()
function Compilar-Fuente($origen, $nombreObjeto) {
    $o = Join-Path $BUILD $nombreObjeto
    & g++ -std=c++11 -O1 -Wall -Wno-unused-parameter @incluye -c $origen -o $o
    if ($LASTEXITCODE -ne 0) { Write-Error "Fallo compilando $origen" }
    $script:objetos += $o
}

# A-12 (05/09): SE SUMAN modo_inteligente.cpp Y demanda.cpp REALES (Bloque E).
#
# El Modo Inteligente no leia ni uno de los tiempos que configura el operario -se
# fijaba VERDE_MIN_MIN en el arranque- y su Regla 1 podia cortar un verde a los 15 s.
# Al arreglarlo aparece una propiedad que NADIE ejercia y que es la que hace seguro
# todo el modo: CON LAS CAMARAS MUERTAS SE COMPORTA EXACTAMENTE COMO EL AUTOMATICO.
# Eso no se puede leer en el fuente, hay que correrlo, y hay que correr LOS DOS MODOS
# con la misma configuracion para poder compararlos.
#
# demanda.cpp entra REAL y no como sustituto: modo_inteligente.cpp mete
# demanda_hayLocal() en el mismo OR que la camara, o sea que la ventana de 3 s de la
# demanda a mano es parte del camino que decide cuando cambia una luz. Un sustituto
# aqui seria una segunda copia de esa ventana escrita a mano.
# D-13 (05/09): SE SUMA botones.cpp REAL (Bloque F).
#
# Hasta hoy este fichero NO SE COMPILABA EN NINGUN ARNES DEL PROYECTO: aqui se
# sustituia por un botones.h de once lineas con las definiciones puestas a mano en
# arnes_automatico.cpp. O sea que el vigilante de camaras de J16 -las dos alarmas,
# la siembra de N-26, el contador de vetos de la fase 2- solo estaba medido por
# packs que leen texto, y los dos defectos que se arreglaron hoy son de
# COMPORTAMIENTO EN EL TIEMPO: un pin sin camara que a las 6 h de paso abierto
# alarmaba de una camara inexistente, y un campo CAM: que con una camara por poste
# no podia decir OK jamas. Ningun pack de texto puede ver ninguno de los dos, y
# camara_03_vigilante -675 lineas, en verde- no los vio.
#
# Ademas es lo que permite ejercer la pregunta del encargo: si una deteccion en el
# PIN de J16 llega hasta el Modo Inteligente. Con el stub, J14 y J16 eran el mismo
# bool y la respuesta salia que si por construccion.
Write-Host "Compilando coordinador.cpp, semaforo.cpp, modo_automatico.cpp, modo_inteligente.cpp, demanda.cpp, mando.cpp y botones.cpp (los MISMOS del firmware) y el arnes..." -ForegroundColor Cyan
Compilar-Fuente (Join-Path $MAESTRO 'src\coordinador.cpp')     'coordinador.o'
Compilar-Fuente (Join-Path $MAESTRO 'src\botones.cpp')         'botones.o'
Compilar-Fuente (Join-Path $MAESTRO 'src\semaforo.cpp')        'semaforo.o'
Compilar-Fuente (Join-Path $MAESTRO 'src\modo_automatico.cpp') 'modo_automatico.o'
Compilar-Fuente (Join-Path $MAESTRO 'src\modo_inteligente.cpp') 'modo_inteligente.o'
Compilar-Fuente (Join-Path $MAESTRO 'src\demanda.cpp')         'demanda.o'
Compilar-Fuente (Join-Path $MAESTRO 'src\mando.cpp')           'mando.o'
Compilar-Fuente (Join-Path $AQUI 'arnes_automatico.cpp')       'arnes_automatico.o'

$exe = Join-Path $BUILD 'validar_automatico.exe'
& g++ @objetos -o $exe
if ($LASTEXITCODE -ne 0) { Write-Error "Fallo el enlazado del arnes del ciclo automatico" }

& $exe
exit $LASTEXITCODE
