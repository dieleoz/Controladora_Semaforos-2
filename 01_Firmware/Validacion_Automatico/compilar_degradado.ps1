# Compila y ejecuta el arnes de LAS DOS PUNTAS EN MODO DEGRADADO.
#
# Es hermano de compilar_dos_puntas.ps1 y NO lo sustituye. Aquel monta el Maestro en
# Modo Automatico gobernando por radio; este monta el modo en el que NO HAY RADIO y
# cada punta enciende su verde por su propio reloj, que es donde un choque frontal
# depende de una desigualdad numerica y no de un enclavamiento.
#
# TRES BINARIOS, POR EL MISMO MOTIVO que el arnes hermano: Maestro y Esclavo definen
# LOS MISMOS SIMBOLOS con implementaciones distintas y no pueden enlazarse juntos. Una
# DLL por punta, un orquestador que carga las dos con LoadLibrary en el MISMO proceso.
# El razonamiento completo esta en la cabecera de dos_puntas/orquestador.cpp.
#
# SE CONSTRUYE EN build_deg/ Y CON NOMBRES PROPIOS, a proposito: el arnes hermano
# tiene que poder correr antes y despues de este sin que ninguno pise los binarios del
# otro. Su cifra -42/42- es la red que dice si esto rompio algo que funcionaba.
#
# LA PUNTA MAESTRO ES DISTINTA de la del arnes hermano: incluye modo_degradado.cpp y
# modo_ambar.cpp REALES. La exclusion "arrastra u8g2" estaba medida al reves; la
# medida esta en la cabecera de dos_puntas/adaptador_maestro_deg.cpp.
#
# LA PUNTA ESCLAVO ES EL MISMO FICHERO adaptador_esclavo.cpp, sin una linea de
# diferencia: aquel ya compila Esclavo/src/modo_degradado.cpp REAL. Copiarlo para
# retocarlo habria creado un segundo adaptador que alguien tendria que sincronizar a
# mano, que es la forma en que este repositorio se ha equivocado tres veces.
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
$BUILD = Join-Path $DP 'build_deg'

if (-not (Test-Path $BUILD)) { New-Item -ItemType Directory -Path $BUILD | Out-Null }

# --- Punta MAESTRO ---------------------------------------------------------
# SEIS ficheros reales, y los dos que importan no los habia compilado nunca ningun
# arnes: modo_degradado.cpp -la unica linea del firmware del Maestro que enciende un
# verde sin confirmacion del otro extremo- y modo_ambar.cpp, donde ese modo cae al
# rendirse.
#
# OJO AL ORDEN DE LOS -I, Y ES DISTINTO DEL DEL ARNES HERMANO. Aqui NO hay directorio
# de sustitutos de la punta: solo comun/ (Arduino.h, pines.h, stm32f1xx_hal.h) y
# despues el include REAL del Maestro. lcd.h, menu.h y botones.h se resuelven contra
# las cabeceras REALES del firmware, porque ninguna de ellas incluye U8g2 -lo arrastra
# lcd.cpp, que aqui no se compila-. Una copia local de cualquiera de ellas seria el
# "casi igual" que puede divergir en silencio.
$fuentesMaestro = @(
    (Join-Path $MAESTRO 'src\coordinador.cpp'),
    (Join-Path $MAESTRO 'src\semaforo.cpp'),
    (Join-Path $MAESTRO 'src\modo_degradado.cpp'),
    (Join-Path $MAESTRO 'src\modo_ambar.cpp'),
    (Join-Path $MAESTRO 'src\modos.cpp'),
    (Join-Path $MAESTRO 'src\respaldo.cpp'),
    (Join-Path $DP 'adaptador_maestro_deg.cpp')
)

# --- Punta ESCLAVO ---------------------------------------------------------
# Los MISMOS SIETE ficheros y el MISMO adaptador que el arnes hermano. Ver la cabecera.
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

Write-Host "Compilando la punta MAESTRO CON DEGRADADO (coordinador + semaforo + modo_degradado + modo_ambar + modos + respaldo REALES)..." -ForegroundColor Cyan
& g++ @comunes "-I$DP" "-I$DP\comun" "-I$MAESTRO\include" -shared -o (Join-Path $BUILD 'punta_maestro_deg.dll') @fuentesMaestro
if ($LASTEXITCODE -ne 0) { Write-Error "Fallo construyendo punta_maestro_deg.dll" }

Write-Host "Compilando la punta ESCLAVO (semaforo + main + modo_degradado + config_ciclo + mando + demanda + respaldo REALES)..." -ForegroundColor Cyan
& g++ @comunes "-I$DP" "-I$DP\comun" "-I$DP\esclavo" "-I$ESCLAVO\include" -shared -o (Join-Path $BUILD 'punta_esclavo_deg.dll') @fuentesEsclavo
if ($LASTEXITCODE -ne 0) { Write-Error "Fallo construyendo punta_esclavo_deg.dll" }

Write-Host "Compilando el orquestador del Degradado..." -ForegroundColor Cyan
$exe = Join-Path $BUILD 'validar_degradado.exe'
& g++ -std=c++11 -O1 -Wall "-I$DP" (Join-Path $DP 'orquestador_degradado.cpp') -o $exe -static-libgcc -static-libstdc++
if ($LASTEXITCODE -ne 0) { Write-Error "Fallo el enlazado del orquestador del Degradado" }

& $exe
exit $LASTEXITCODE
