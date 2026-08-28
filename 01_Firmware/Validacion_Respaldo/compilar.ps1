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

if (-not (Get-Command gcc -ErrorAction SilentlyContinue)) {
    Write-Error "No hay gcc de host en el PATH. Sin el, esto NO puede correr: es ABORTADO, no PASS."
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
if ($LASTEXITCODE -ne 0) { Write-Error "Fallo compilando el arnes de respaldo" }

# N-43: sin esto, PowerShell 5.1 mete un BOM UTF-8 (EF BB BF) delante de "PING"
# al escribir en la tuberia de un proceso nativo -pwsh 7 y bash no-, el strncmp
# del arnes no casa y contesta ERROR. El arnes ya descarta el BOM por su cuenta;
# esto arregla la otra punta, para que lo que se le manda sea lo que se cree.
$OutputEncoding = New-Object System.Text.UTF8Encoding $false

# Prueba de vida. Un binario que compila pero no contesta dejaria al validador
# midiendo el vacio, que es la forma de perder cobertura sin enterarse.
$respuesta = "PING" | & $exe
if ($respuesta -ne 'PONG') { Write-Error "El arnes compila pero no responde al PING: '$respuesta'" }

Write-Host "OK: $exe" -ForegroundColor Green
