# Compila el arnes de respaldo.cpp para el PC.
#
# Requiere gcc/g++ de host (MinGW-w64). Se instala con:
#   winget install --id BrechtSanders.WinLibs.POSIX.UCRT
#
# N-44: durante dias el gcc de esta maquina NO ENLAZABA -estaba instalado bajo
# una ruta con 'n' con tilde y su ld no encontraba crt2.o-, y este script moria
# ahi mismo, antes de llegar a ninguna comprobacion. Se creyo falta de toolchain.
# La compuerta ya no da por bueno un gcc que solo existe: le exige enlazar.
#
# ES UN SOLO EJECUTABLE, no dos como en Validacion_LCD. Alli habia que separar
# Maestro y Esclavo porque definen los MISMOS simbolos con implementaciones
# distintas. Aqui no hace falta y ademas seria enganoso sugerir que hay dos
# versiones: respaldo.cpp y respaldo.h son BIT A BIT IDENTICOS en las dos puntas
# -lo comprueba el propio validador del Maestro- y esa identidad es lo que
# garantiza que las dos fechan la sincronizacion con la misma aritmetica. Se
# compila el del Maestro y se verifica de paso que el del Esclavo no ha derivado.

$ErrorActionPreference = 'Stop'
$AQUI = Split-Path -Parent $MyInvocation.MyCommand.Path
$RAIZ = Split-Path -Parent $AQUI                      # 01_Firmware
$MAESTRO = Join-Path $RAIZ 'Maestro'
$ESCLAVO = Join-Path $RAIZ 'Esclavo'
$SALIDA = Join-Path $AQUI 'build'
$TOOLCHAIN = 'D:\toolchain\mingw64\bin'
if (Test-Path (Join-Path $TOOLCHAIN 'g++.exe')) { $env:Path = "$TOOLCHAIN;$env:Path" }

if (-not (Get-Command g++ -ErrorAction SilentlyContinue)) {
    [Console]::Error.WriteLine("[ABORTADO] No hay g++ de host en el PATH (tampoco en $TOOLCHAIN)."); exit 2
}

# La identidad entre puntas no es un detalle de organizacion: si respaldo.cpp
# derivara, el Maestro y el Esclavo fecharian la misma sincronizacion de forma
# distinta y se rendirian en instantes distintos -una en ambar, la otra en verde-.
# Se comprueba ANTES de compilar, porque compilar solo uno de los dos y llamarlo
# "el respaldo" solo vale si de verdad hay uno solo.
# Se calcula con .NET y NO con Get-FileHash a proposito. Medido: lanzado desde la
# compuerta, ese cmdlet no se resuelve -el PSModulePath que hereda la sesion mezcla
# los modulos de PowerShell 7 y los de la extension del IDE, y el autocargado de
# PS 5.1 se queda sin encontrar Microsoft.PowerShell.Utility; con el PSModulePath
# limpio aparece-. El script moria ahi, antes de comprobar nada, y por fuera
# parecia el arnes roto. Un instrumento no puede depender del entorno de quien lo
# llama: [Security.Cryptography.SHA256] esta siempre.
function Get-Sha256 ([string]$ruta) {
    $sha = [System.Security.Cryptography.SHA256]::Create()
    try {
        return [System.BitConverter]::ToString($sha.ComputeHash([System.IO.File]::ReadAllBytes($ruta))).Replace('-', '')
    } finally { $sha.Dispose() }
}

foreach ($f in @('src\respaldo.cpp', 'include\respaldo.h')) {
    $a = Get-Sha256 (Join-Path $MAESTRO $f)
    $b = Get-Sha256 (Join-Path $ESCLAVO $f)
    if ($a -ne $b) {
        Write-Error "$f DIFIERE entre Maestro y Esclavo. Las dos puntas fecharian la sincronizacion con aritmetica distinta."
    }
}

New-Item -ItemType Directory -Force -Path $SALIDA | Out-Null

# OJO AL ORDEN DE LOS -I. Este directorio va PRIMERO para que <stm32f1xx_hal.h> y
# <Arduino.h> se resuelvan contra los sustitutos y no contra nada del sistema.
# respaldo.h se coge del Maestro.
$incluye = @("-I$AQUI", "-I$MAESTRO\include", "-I$MAESTRO\src")

$exe = Join-Path $SALIDA 'arnes_respaldo.exe'
Write-Host "Compilando respaldo.cpp (el MISMO que va a la tarjeta) y el arnes..." -ForegroundColor Cyan
& g++ -O1 -w @incluye (Join-Path $AQUI 'arnes_respaldo.cpp') -o $exe
if ($LASTEXITCODE -ne 0) { [Console]::Error.WriteLine("[ABORTADO] Fallo compilando el arnes de respaldo"); exit 2 }

# N-43: evita el BOM UTF-8 (EF BB BF) que PowerShell 5.1 antepone en tuberias nativas.
$OutputEncoding = New-Object System.Text.UTF8Encoding $false

# Prueba de vida: un binario que no responde al PING es ABORTADO, no FALLA.
$respuesta = "PING" | & $exe
if ($respuesta -ne 'PONG') { [Console]::Error.WriteLine("[ABORTADO] El arnes no responde al PING: '$respuesta'"); exit 2 }

# D-62: prioridad WATCHDOG > SOFTWARE > CORTE > PIN > OTRA sobre anotarArranque() REAL.
# Los bits son los de RCC_CSR del manual de referencia del F1 (RM0008 §7.3.10), NO las
# constantes de respaldo.cpp: PINRSTF 26, PORRSTF 27, SFTRSTF 28, IWDGRSTF 29. En el F1
# un POR y un IWDG levantan tambien PINRSTF, por eso los casos llevan PIN junto al motivo.
$PIN = [uint32]1 -shl 26; $POR = [uint32]1 -shl 27; $SFT = [uint32]1 -shl 28; $IWDG = [uint32]1 -shl 29
$casos = @(@(($IWDG -bor $PIN), 'WATCHDOG'), @(($SFT -bor $PIN), 'SOFTWARE'),
           @(($POR -bor $PIN), 'CORTE'), @($PIN, 'PIN'), @(0, 'OTRA'))
$causas = @($casos | ForEach-Object { "CAUSA $($_[0])" } | & $exe)
for ($i = 0; $i -lt $casos.Count; $i++) {
    if ($causas[$i] -ne $casos[$i][1]) {
        Write-Error "CAUSA DIFIERE de D-62: RCC_CSR=$($casos[$i][0]) da '$($causas[$i])', se esperaba '$($casos[$i][1])'"
    }
}

# D-114: respaldo_borrar() pone a 0 tambien DR9/DR10 (tiempos del ciclo, N-133). Si no, sellar()
# mete la basura en la suma y respaldo_tiemposCiclo() la da por un ciclo configurado.
$borrar = "BORRAR" | & $exe
if ($borrar -ne '0 0 0') { Write-Error "BORRAR DIFIERE de D-114: DR9 DR10 hayTiempos = '$borrar', se esperaba '0 0 0'" }

# D-95/D-148/D-149 (SPEC_8 6.1-6.2): la linea esperada sale de las DECISIONES, no del fuente.
$banderas = "BANDERAS" | & $exe
if ($banderas -ne '0 1 1 1 0 1 1 0 0200') { Write-Error "BANDERAS DIFIERE de D-95/D-148: '$banderas', se esperaba '0 1 1 1 0 1 1 0 0200'" }
Write-Host "OK: $exe (CAUSA D-62 $($casos.Count)/$($casos.Count))" -ForegroundColor Green
