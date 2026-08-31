# Compila los DOS arneses del puente ESP32: uno por punta.
#
# Cada uno enlaza el bluetooth.cpp REAL de su punta -no una copia, no un espejo en
# Python- junto con los fuentes que deciden el EFECTO de un comando: semaforo.cpp
# escribe los pines, coordinador.cpp gobierna el ciclo, modo_automatico.cpp y mando.cpp
# completan el camino. Lo que NO compila y por que esta escrito en arnes_puente.cpp.
#
# DOS EJECUTABLES Y NO UNO, como Validacion_LCD y a diferencia de Validacion_Ciclo: las
# dos puntas definen los MISMOS simbolos -bluetooth_loop, semaforo_actualizar,
# procesarComando- con contenidos distintos. En un solo binario el enlazador se
# quedaria con uno de los dos y el arnes mediria el Maestro creyendo medir el Esclavo,
# que es N-86 con otra ropa: no da error, da el ultimo que entro.
#
# EL TOOLCHAIN QUE ENLAZA EN ESTA MAQUINA. El de winget (WinLibs por
# BrechtSanders.WinLibs.POSIX.UCRT) vive bajo una ruta con enie y su ld no encuentra
# crt2.o (N-44, ver CLAUDE.md 9). El que SI enlaza esta en D:\toolchain\mingw64\bin, y
# va al FRENTE del PATH solo para este proceso.
$ErrorActionPreference = 'Stop'
$AQUI = Split-Path -Parent $MyInvocation.MyCommand.Path

$TOOLCHAIN = 'D:\toolchain\mingw64\bin'
if (Test-Path (Join-Path $TOOLCHAIN 'gcc.exe')) {
    $env:Path = "$TOOLCHAIN;$env:Path"
}

if (-not (Get-Command g++ -ErrorAction SilentlyContinue)) {
    Write-Error "No hay g++ de host en el PATH (se busco tambien en $TOOLCHAIN). Sin el, esto NO puede correr: es ABORTADO, no PASS."
}

$RAIZ  = Split-Path -Parent (Split-Path -Parent $AQUI)   # 01_Firmware
$BUILD = Join-Path $AQUI 'build'
if (-not (Test-Path $BUILD)) { New-Item -ItemType Directory -Path $BUILD | Out-Null }

# OJO AL ORDEN DE LOS -I: este directorio va PRIMERO para que Arduino.h, pines.h,
# botones.h, lcd.h y menu.h se resuelvan contra los sustitutos de aqui y no contra los
# reales, que arrastrarian STM32duino y U8g2. bluetooth.h, semaforo.h, coordinador.h,
# protocolo.h, reloj.h, identidad.h, demanda.h, modos.h, mando.h, modo_automatico.h y
# modo_degradado.h SI son los reales: no hay copia local de ninguno, a proposito -una
# copia local seria el "casi igual" que diverge sin que nadie lo note-.
#
# -DUID_BASE y el -include: identidad.cpp REAL se niega a compilar sin UID_BASE. Ver
# uid_arnes.h; se le da memoria legible en vez de sustituir la funcion.
function Construir($punta, $carpeta, $fuentes, $exe) {
    $inc = @("-I$AQUI", "-I$RAIZ\$carpeta\include")
    $flags = @('-std=c++11', '-O1', '-Wall', '-Wno-unused-parameter',
               "-DPUNTA_$punta", '-DUID_BASE=arnes_uid',
               '-include', (Join-Path $AQUI 'uid_arnes.h'))
    $objetos = @()
    foreach ($f in $fuentes) {
        $o = Join-Path $BUILD "$($punta.ToLower())_$f.o"
        & g++ @flags @inc -c (Join-Path $RAIZ "$carpeta\src\$f.cpp") -o $o
        if ($LASTEXITCODE -ne 0) { Write-Error "Fallo compilando $carpeta/src/$f.cpp" }
        $objetos += $o
    }
    $o = Join-Path $BUILD "$($punta.ToLower())_arnes.o"
    & g++ @flags @inc -c (Join-Path $AQUI 'arnes_puente.cpp') -o $o
    if ($LASTEXITCODE -ne 0) { Write-Error "Fallo compilando el arnes de $punta" }
    $objetos += $o

    $destino = Join-Path $BUILD $exe
    & g++ @objetos -o $destino
    if ($LASTEXITCODE -ne 0) { Write-Error "Fallo el enlazado del arnes de $punta" }
    Write-Host "OK $exe" -ForegroundColor Green
}

Write-Host "Compilando el bluetooth.cpp REAL de las dos puntas..." -ForegroundColor Cyan
Construir 'MAESTRO' 'Maestro' `
    @('bluetooth','semaforo','coordinador','modo_automatico','mando','modos','demanda','identidad') `
    'arnes_maestro.exe'
Construir 'ESCLAVO' 'Esclavo' `
    @('bluetooth','semaforo','demanda','identidad') `
    'arnes_esclavo.exe'
