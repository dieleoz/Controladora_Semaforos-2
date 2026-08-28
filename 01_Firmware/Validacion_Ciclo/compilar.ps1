# Compila y ejecuta el arnes del ciclo degradado en el PC.
#
# Compila el ciclo_degradado.h REAL del firmware -no una copia, no un espejo- y barre
# las 86.400 posiciones del dia sobre EL. Requiere gcc/g++ (MinGW-w64).
#
# UN SOLO BINARIO, a diferencia del arnes de pantalla. Alli hacian falta dos porque
# Maestro y Esclavo definen los mismos simbolos con implementaciones distintas; aqui el
# fichero es identico byte a byte en las dos puntas -lo comprueba el pack
# costura_01_contratos- asi que medir uno mide los dos.

$ErrorActionPreference = 'Stop'
$AQUI = Split-Path -Parent $MyInvocation.MyCommand.Path
$INC_FIRMWARE = Join-Path $AQUI '..\Maestro\include'
$STUB = Join-Path $AQUI '..\Validacion_LCD'      # el Arduino.h minimo, ya existente
$BUILD = Join-Path $AQUI 'build'

if (-not (Test-Path $BUILD)) { New-Item -ItemType Directory -Path $BUILD | Out-Null }

$exe = Join-Path $BUILD 'validar_ciclo.exe'

& g++ -std=c++11 -O1 -Wall `
    -I $INC_FIRMWARE -I $STUB `
    (Join-Path $AQUI 'arnes_ciclo.cpp') `
    -o $exe
if ($LASTEXITCODE -ne 0) { Write-Error "no compila el arnes del ciclo"; exit 2 }

& $exe
exit $LASTEXITCODE
