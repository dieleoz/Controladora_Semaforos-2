# Compila y ejecuta el arnes de LAS DOS PUNTAS.
#
# Es hermano de compilar.ps1 -el arnes de una punta- y NO lo sustituye: aquel mide el
# ciclo del Maestro en soledad con veintitantas comprobaciones que este no repite. Este
# mide la propiedad que aquel declara como su punto ciego: "verde simultaneo en las dos
# puntas no se mide ahi".
#
# TRES BINARIOS, NO UNO NI DOS.
#
# Maestro y Esclavo definen LOS MISMOS SIMBOLOS con implementaciones distintas, asi que
# no pueden enlazarse juntos. Validacion_LCD resolvio ese choque con DOS EJECUTABLES que
# corren uno detras de otro y suman resultados; aqui eso no vale, porque la propiedad es
# "nunca las dos en verde EN EL MISMO INSTANTE" y dos ejecuciones separadas no tienen un
# instante comun.
#
# La solucion es UNA DLL POR PUNTA y un orquestador que carga las dos en el MISMO
# proceso con LoadLibrary. El razonamiento completo -y por que se descartaron los dos
# procesos comunicados, el prefijado de simbolos y los espacios de nombres- esta en la
# cabecera de dos_puntas/orquestador.cpp.
#
# EL TOOLCHAIN QUE ENLAZA EN ESTA MAQUINA (N-44). El de winget vive bajo una ruta con
# enie y su ld no encuentra crt2.o. El que SI enlaza esta en D:\toolchain\mingw64\bin y
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

$RAIZ = Split-Path -Parent $AQUI                      # 01_Firmware
$MAESTRO = Join-Path $RAIZ 'Maestro'
$ESCLAVO = Join-Path $RAIZ 'Esclavo'
$DP = Join-Path $AQUI 'dos_puntas'
$BUILD = Join-Path $DP 'build'

if (-not (Test-Path $BUILD)) { New-Item -ItemType Directory -Path $BUILD | Out-Null }

# OJO AL ORDEN DE LOS -I. El directorio de sustitutos de la punta va PRIMERO, luego el
# comun, y el include REAL de esa punta al final. Asi Arduino.h, pines.h, lcd.h, menu.h,
# botones.h e IWatchdog.h se resuelven contra los sustitutos, y TODO lo demas
# -semaforo.h, protocolo.h, reloj.h, respaldo.h, mando.h, config_ciclo.h,
# modo_degradado.h, ciclo_degradado.h, bluetooth.h, demanda.h, coordinador.h,
# modo_automatico.h, modos.h, modo_ambar.h- cae a la cabecera REAL del firmware. Una
# copia local de cualquiera de esas seria el "casi igual" que puede divergir en silencio.
$COMUN = @("-I$DP", "-I$DP\comun")

# --- Punta MAESTRO ---------------------------------------------------------
# Los mismos cuatro ficheros que ya ejerce el arnes de una punta.
$fuentesMaestro = @(
    (Join-Path $MAESTRO 'src\coordinador.cpp'),
    (Join-Path $MAESTRO 'src\semaforo.cpp'),
    (Join-Path $MAESTRO 'src\modo_automatico.cpp'),
    (Join-Path $MAESTRO 'src\mando.cpp'),
    (Join-Path $DP 'adaptador_maestro.cpp')
)

# --- Punta ESCLAVO ---------------------------------------------------------
# SIETE ficheros reales, y el que importa es src/main.cpp: el despachador de radio que
# decide si esta punta obedece un CMD_GO_GREEN. Ningun arnes lo habia compilado nunca.
$fuentesEsclavo = @(
    (Join-Path $ESCLAVO 'src\semaforo.cpp'),
    (Join-Path $ESCLAVO 'src\main.cpp'),
    (Join-Path $ESCLAVO 'src\modo_degradado.cpp'),
    (Join-Path $ESCLAVO 'src\config_ciclo.cpp'),
    (Join-Path $ESCLAVO 'src\mando.cpp'),
    (Join-Path $ESCLAVO 'src\demanda.cpp'),
    (Join-Path $ESCLAVO 'src\respaldo.cpp'),
    (Join-Path $DP 'adaptador_esclavo.cpp')
)

foreach ($f in ($fuentesMaestro + $fuentesEsclavo)) {
    if (-not (Test-Path $f)) {
        Write-Error "No existe el fuente $f. Mover o renombrar un .cpp rompe este arnes: el movimiento y la actualizacion de rutas van en el MISMO commit."
    }
}

$comunes = @('-std=c++11', '-O1', '-Wall', '-Wno-unused-parameter', '-DPUNTA_EXPORTA',
             '-static-libgcc', '-static-libstdc++')

Write-Host "Compilando la punta MAESTRO (coordinador + semaforo + modo_automatico + mando REALES)..." -ForegroundColor Cyan
& g++ @comunes @COMUN "-I$DP\maestro" "-I$MAESTRO\include" -shared -o (Join-Path $BUILD 'punta_maestro.dll') @fuentesMaestro
if ($LASTEXITCODE -ne 0) { Write-Error "Fallo construyendo punta_maestro.dll" }

Write-Host "Compilando la punta ESCLAVO (semaforo + main + modo_degradado + config_ciclo + mando + demanda + respaldo REALES)..." -ForegroundColor Cyan
& g++ @comunes @COMUN "-I$DP\esclavo" "-I$ESCLAVO\include" -shared -o (Join-Path $BUILD 'punta_esclavo.dll') @fuentesEsclavo
if ($LASTEXITCODE -ne 0) { Write-Error "Fallo construyendo punta_esclavo.dll" }

Write-Host "Compilando el orquestador..." -ForegroundColor Cyan
$exe = Join-Path $BUILD 'validar_dos_puntas.exe'
& g++ -std=c++11 -O1 -Wall @COMUN (Join-Path $DP 'orquestador.cpp') -o $exe -static-libgcc -static-libstdc++
if ($LASTEXITCODE -ne 0) { Write-Error "Fallo el enlazado del orquestador" }

& $exe
exit $LASTEXITCODE
