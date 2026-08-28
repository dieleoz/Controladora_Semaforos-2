# Compila y ejecuta la validacion de pantalla en el PC.
# Requiere gcc/g++ (MinGW-w64). Se instalo con:
#   winget install --id BrechtSanders.WinLibs.POSIX.UCRT
#
# SON DOS EJECUTABLES, NO UNO.
#
# Maestro y Esclavo definen los MISMOS simbolos -lcd_setup(), menu_setup(),
# lcd_dibujarMenu()...- con implementaciones distintas. Enlazarlos en un solo
# binario es imposible: los simbolos chocan. Asi que se construyen dos programas,
# se ejecutan los dos y se SUMAN sus resultados. Un solo fallo en cualquiera de
# los dos tumba la ejecucion entera.
#
# El arnes del Esclavo no es un extra: hasta hoy las cinco vistas del Esclavo
# nunca habian pasado comprobacion geometrica dentro de un arnes permanente. Se
# validaron una vez con uno temporal que se desecho, y una validacion de usar y
# tirar comprueba el momento, no el proyecto.

$ErrorActionPreference = 'Stop'
$AQUI = Split-Path -Parent $MyInvocation.MyCommand.Path

# ---------------------------------------------------------------------------
# LOCALIZACION DEL COMPILADOR DE HOST
# ---------------------------------------------------------------------------
# Este bloque existe por un error de diagnostico real, no por comodidad.
#
# Durante dias la compuerta dio el arnes por ABORTADO con el motivo "no hay gcc de
# host", y con ese motivo se escribieron dos pantallas nuevas del Maestro contando
# los anchos a mano (ver el comentario de lcd_dibujarSyncHora en Maestro/src/lcd.cpp).
# El compilador ESTABA instalado: winget lo habia dejado en su carpeta de paquetes,
# que no esta en el PATH. Lo que fallaba era la BUSQUEDA, no la maquina.
#
# La leccion queda aqui en forma de codigo: no se pregunta solo al PATH, se miran
# tambien los sitios donde los instaladores habituales dejan el compilador. Si algun
# dia no aparece en ninguno, el mensaje de error dice exactamente donde se miro,
# para que el siguiente no vuelva a concluir "no hay compilador" sin comprobarlo.
function Buscar-Compilador {
    param([string]$Nombre)   # 'gcc' o 'g++'

    # 1) El PATH, que es lo barato.
    $enPath = Get-Command $Nombre -ErrorAction SilentlyContinue
    if ($enPath) { return $enPath.Source }

    # 2) Los sitios conocidos. WinLibs por winget va primero porque es el que la
    #    cabecera de este script dice que se instalo.
    $candidatos = @(
        "$env:LOCALAPPDATA\Microsoft\WinGet\Packages\BrechtSanders.WinLibs.POSIX.UCRT_Microsoft.Winget.Source_8wekyb3d8bbwe\mingw64\bin\$Nombre.exe",
        "$env:LOCALAPPDATA\Microsoft\WinGet\Links\$Nombre.exe",
        "C:\msys64\ucrt64\bin\$Nombre.exe",
        "C:\msys64\mingw64\bin\$Nombre.exe",
        "C:\mingw64\bin\$Nombre.exe",
        "C:\MinGW\bin\$Nombre.exe",
        "C:\ProgramData\chocolatey\bin\$Nombre.exe",
        "C:\Strawberry\c\bin\$Nombre.exe",
        "C:\TDM-GCC-64\bin\$Nombre.exe"
    )
    foreach ($c in $candidatos) { if (Test-Path $c) { return $c } }

    # 3) Ultimo recurso: cualquier paquete de winget cuyo nombre suene a MinGW.
    $raizWinGet = "$env:LOCALAPPDATA\Microsoft\WinGet\Packages"
    if (Test-Path $raizWinGet) {
        $hallado = Get-ChildItem -Path $raizWinGet -Recurse -Depth 4 -Filter "$Nombre.exe" `
                                 -File -ErrorAction SilentlyContinue |
                   Select-Object -First 1
        if ($hallado) { return $hallado.FullName }
    }

    Write-Error ("No se encuentra $Nombre. Se miro en el PATH y en:`n  " +
                 ($candidatos -join "`n  ") +
                 "`n  y recursivamente en $raizWinGet`n" +
                 "Si de verdad no esta, instalelo con:`n" +
                 "  winget install --id BrechtSanders.WinLibs.POSIX.UCRT")
}

$GCC = Buscar-Compilador 'gcc'
$GXX = Buscar-Compilador 'g++'
Write-Host "Compilador de host: $GCC" -ForegroundColor DarkGray
Write-Host "                    $GXX" -ForegroundColor DarkGray

$RAIZ = Split-Path -Parent $AQUI                      # 01_Firmware
$U8G2 = Join-Path $RAIZ 'Esclavo\.pio\libdeps\esclavo\U8g2\src'
$MAESTRO = Join-Path $RAIZ 'Maestro'
$ESCLAVO = Join-Path $RAIZ 'Esclavo'
$SALIDA = Join-Path $AQUI 'build'

if (-not (Test-Path $U8G2)) {
    Write-Error "No se encuentran las fuentes de U8g2 en $U8G2. Compile antes el proyecto Esclavo con PlatformIO."
}
New-Item -ItemType Directory -Force -Path $SALIDA | Out-Null

# El nucleo C de U8g2 (clib) es portable: se compila tal cual. Lo comparten los
# dos arneses, asi que se compila una sola vez.
$fuentesC = Get-ChildItem (Join-Path $U8G2 'clib') -Filter *.c | ForEach-Object { $_.FullName }

# Sin comillas internas: PowerShell ya pasa cada elemento como un argumento.
# OJO al orden de los -I: la cabecera del proyecto que se este validando tiene
# que ganar. Maestro y Esclavo tienen lcd.h y menu.h con el MISMO nombre y
# contenido distinto; coger la del proyecto equivocado daria un binario que
# compila y valida otra cosa.
$comunes = @(
    "-I$AQUI",              # Arduino.h y pines.h sustitutos (van primero)
    "-I$U8G2",
    "-I$U8G2\clib"
)
$incluyeMaestro = @("-I$AQUI", "-I$MAESTRO\include") + $comunes[1..2]
$incluyeEsclavo = @("-I$AQUI", "-I$ESCLAVO\include") + $comunes[1..2]

Write-Host "Compilando nucleo C de U8g2 ($($fuentesC.Count) archivos)..." -ForegroundColor Cyan
$objetos = @()
foreach ($f in $fuentesC) {
    $o = Join-Path $SALIDA ((Split-Path $f -Leaf) -replace '\.c$', '.o')
    & $GCC -c -O1 -w @comunes $f -o $o
    if ($LASTEXITCODE -ne 0) { Write-Error "Fallo compilando $f" }
    $objetos += $o
}

# ---------------------------------------------------------------------------
# ARNES DEL MAESTRO
# ---------------------------------------------------------------------------
# lcd.cpp y menu.cpp son los MISMOS que se compilan para la tarjeta. Se enlazan
# tal cual, de modo que la validacion no puede desviarse del firmware real.
Write-Host "MAESTRO: compilando lcd.cpp y menu.cpp (los mismos del firmware) y el arnes..." -ForegroundColor Cyan
& $GXX -c -O1 -w -DLCD_VALIDACION_NATIVA @incluyeMaestro (Join-Path $MAESTRO 'src\lcd.cpp') -o (Join-Path $SALIDA 'lcd.o')
if ($LASTEXITCODE -ne 0) { Write-Error "Fallo compilando lcd.cpp del Maestro" }
& $GXX -c -O1 -w -DLCD_VALIDACION_NATIVA @incluyeMaestro (Join-Path $MAESTRO 'src\menu.cpp') -o (Join-Path $SALIDA 'menu.o')
if ($LASTEXITCODE -ne 0) { Write-Error "Fallo compilando menu.cpp del Maestro" }
& $GXX -c -O1 -w -DLCD_VALIDACION_NATIVA @incluyeMaestro (Join-Path $AQUI 'arnes_lcd.cpp') -o (Join-Path $SALIDA 'arnes.o')
if ($LASTEXITCODE -ne 0) { Write-Error "Fallo compilando el arnes del Maestro" }

$exeMaestro = Join-Path $SALIDA 'validar_lcd.exe'
& $GXX (Join-Path $SALIDA 'lcd.o') (Join-Path $SALIDA 'menu.o') (Join-Path $SALIDA 'arnes.o') $objetos -o $exeMaestro
if ($LASTEXITCODE -ne 0) { Write-Error "Fallo el enlazado del arnes del Maestro" }

# ---------------------------------------------------------------------------
# ARNES DEL ESCLAVO
# ---------------------------------------------------------------------------
# Se enlazan TRES ficheros del firmware: lcd.cpp (las cinco vistas), menu.cpp
# (la navegacion y la composicion de los textos) y modo_degradado.cpp (la
# maquina de estados que produce esos textos). Los tres, tal cual van a la
# tarjeta. Lo demas lo sustituye el arnes.
#
# ARNES_MILLIS_CONTROLADO da al arnes del Esclavo un reloj que puede mover: sin
# el no se podrian comprobar ni el regreso al listado a los 90 s ni el limite
# duro de 48 h. El arnes del Maestro NO lleva ese define y compila el mismo
# Arduino.h de siempre.
$defsEsclavo = @('-DLCD_VALIDACION_NATIVA', '-DARNES_MILLIS_CONTROLADO')

Write-Host "ESCLAVO: compilando lcd.cpp, menu.cpp y modo_degradado.cpp (los mismos del firmware) y el arnes..." -ForegroundColor Cyan
& $GXX -c -O1 -w @defsEsclavo @incluyeEsclavo (Join-Path $ESCLAVO 'src\lcd.cpp') -o (Join-Path $SALIDA 'lcd_esc.o')
if ($LASTEXITCODE -ne 0) { Write-Error "Fallo compilando lcd.cpp del Esclavo" }
& $GXX -c -O1 -w @defsEsclavo @incluyeEsclavo (Join-Path $ESCLAVO 'src\menu.cpp') -o (Join-Path $SALIDA 'menu_esc.o')
if ($LASTEXITCODE -ne 0) { Write-Error "Fallo compilando menu.cpp del Esclavo" }
& $GXX -c -O1 -w @defsEsclavo @incluyeEsclavo (Join-Path $ESCLAVO 'src\modo_degradado.cpp') -o (Join-Path $SALIDA 'degradado_esc.o')
if ($LASTEXITCODE -ne 0) { Write-Error "Fallo compilando modo_degradado.cpp del Esclavo" }
& $GXX -c -O1 -w @defsEsclavo @incluyeEsclavo (Join-Path $AQUI 'arnes_esclavo.cpp') -o (Join-Path $SALIDA 'arnes_esc.o')
if ($LASTEXITCODE -ne 0) { Write-Error "Fallo compilando el arnes del Esclavo" }

$exeEsclavo = Join-Path $SALIDA 'validar_lcd_esclavo.exe'
& $GXX (Join-Path $SALIDA 'lcd_esc.o') (Join-Path $SALIDA 'menu_esc.o') (Join-Path $SALIDA 'degradado_esc.o') (Join-Path $SALIDA 'arnes_esc.o') $objetos -o $exeEsclavo
if ($LASTEXITCODE -ne 0) { Write-Error "Fallo el enlazado del arnes del Esclavo" }

# ---------------------------------------------------------------------------
# EJECUCION Y SUMA
# ---------------------------------------------------------------------------
# El recuento se lee de la linea RESULTADO que imprime cada arnes, en lugar de
# fiarse solo del codigo de salida: asi el total combinado sale de lo que los
# programas dicen haber comprobado y no de una cuenta escrita a mano aqui, que
# se quedaria vieja a la primera comprobacion que alguien anada.
function Invocar-Arnes {
    param([string]$Titulo, [string]$Exe)

    Write-Host "`nEjecutando $Titulo...`n" -ForegroundColor Cyan
    $salida = & $Exe 2>&1
    $codigo = $LASTEXITCODE
    $salida | ForEach-Object { Write-Host $_ }

    $texto = ($salida | Out-String)
    $total = -1
    $fallos = -1
    if ($texto -match 'RESULTADO:\s+(\d+)/(\d+)\s+comprobaciones OK') {
        $total = [int]$Matches[2]
        $fallos = 0
    } elseif ($texto -match 'RESULTADO:\s+(\d+)\s+de\s+(\d+)\s+comprobaciones FALLARON') {
        $fallos = [int]$Matches[1]
        $total = [int]$Matches[2]
    }

    if ($total -lt 0) {
        # Sin linea de resultado no se sabe cuanto se comprobo. Se trata como
        # fallo: un arnes que no llega al final es un arnes que no valido nada.
        Write-Host "`n$Titulo no imprimio linea de RESULTADO (codigo de salida $codigo)." -ForegroundColor Red
        return [pscustomobject]@{ Titulo = $Titulo; Total = 0; Fallos = 1; Roto = $true }
    }

    return [pscustomobject]@{ Titulo = $Titulo; Total = $total; Fallos = $fallos; Roto = ($codigo -ne 0 -and $fallos -eq 0) }
}

$res = @()
$res += Invocar-Arnes -Titulo 'MAESTRO' -Exe $exeMaestro
$res += Invocar-Arnes -Titulo 'ESCLAVO' -Exe $exeEsclavo

$totalGeneral = ($res | Measure-Object -Property Total -Sum).Sum
$fallosGeneral = ($res | Measure-Object -Property Fallos -Sum).Sum
$roto = ($res | Where-Object { $_.Roto }).Count -gt 0

Write-Host ""
Write-Host "===========================================================" -ForegroundColor Cyan
Write-Host " TOTAL COMBINADO DE LA VALIDACION DE PANTALLA" -ForegroundColor Cyan
Write-Host "===========================================================" -ForegroundColor Cyan
foreach ($r in $res) {
    $ok = $r.Total - $r.Fallos
    $color = if ($r.Fallos -eq 0 -and -not $r.Roto) { 'Green' } else { 'Red' }
    Write-Host ("   {0,-8} {1,3}/{2,-3} comprobaciones OK" -f $r.Titulo, $ok, $r.Total) -ForegroundColor $color
}
$okGeneral = $totalGeneral - $fallosGeneral
if ($fallosGeneral -eq 0 -and -not $roto) {
    Write-Host ("   TOTAL    {0,3}/{1,-3} comprobaciones OK" -f $okGeneral, $totalGeneral) -ForegroundColor Green
    Write-Host "===========================================================" -ForegroundColor Cyan
    exit 0
} else {
    Write-Host ("   TOTAL    {0} de {1} comprobaciones FALLARON" -f $fallosGeneral, $totalGeneral) -ForegroundColor Red
    Write-Host "===========================================================" -ForegroundColor Cyan
    exit 1
}
