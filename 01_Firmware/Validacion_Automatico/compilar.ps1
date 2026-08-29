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

Write-Host "Compilando coordinador.cpp, semaforo.cpp, modo_automatico.cpp y mando.cpp (los MISMOS del firmware) y el arnes..." -ForegroundColor Cyan
Compilar-Fuente (Join-Path $MAESTRO 'src\coordinador.cpp')     'coordinador.o'
Compilar-Fuente (Join-Path $MAESTRO 'src\semaforo.cpp')        'semaforo.o'
Compilar-Fuente (Join-Path $MAESTRO 'src\modo_automatico.cpp') 'modo_automatico.o'
Compilar-Fuente (Join-Path $MAESTRO 'src\mando.cpp')           'mando.o'
Compilar-Fuente (Join-Path $AQUI 'arnes_automatico.cpp')       'arnes_automatico.o'

$exe = Join-Path $BUILD 'validar_automatico.exe'
& g++ @objetos -o $exe
if ($LASTEXITCODE -ne 0) { Write-Error "Fallo el enlazado del arnes del ciclo automatico" }

& $exe
exit $LASTEXITCODE
