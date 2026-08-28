@echo off
setlocal enabledelayedexpansion

echo ================================================================================
echo  📦 PIPELINE DE COMPILACION DE APK ANDROID — APP IOT-VIAL
echo ================================================================================

:: 1. Detección dinámica de JAVA_HOME (JDK 21 / JDK 17)
if not defined JAVA_HOME (
    if exist "C:\Program Files\Eclipse Adoptium\jdk-21" (
        set "JAVA_HOME=C:\Program Files\Eclipse Adoptium\jdk-21"
    ) else if exist "C:\Program Files\Microsoft\jdk-21" (
        set "JAVA_HOME=C:\Program Files\Microsoft\jdk-21"
    ) else if exist "C:\Program Files\Java\jdk-21" (
        set "JAVA_HOME=C:\Program Files\Java\jdk-21"
    ) else if exist "D:\@Proyect\Baliza\7 sw apk\jdk-17\jdk-17.0.12+7" (
        set "JAVA_HOME=D:\@Proyect\Baliza\7 sw apk\jdk-17\jdk-17.0.12+7"
    )
)

:: 2. Detección dinámica de ANDROID_HOME
if not defined ANDROID_HOME (
    if exist "%LOCALAPPDATA%\Android\Sdk" (
        set "ANDROID_HOME=%LOCALAPPDATA%\Android\Sdk"
    ) else if exist "D:\@Proyect\Baliza\7 sw apk\android-sdk" (
        set "ANDROID_HOME=D:\@Proyect\Baliza\7 sw apk\android-sdk"
    )
)
set "ANDROID_SDK_ROOT=%ANDROID_HOME%"
set "PATH=%JAVA_HOME%\bin;%PATH%"

echo [INFO] JAVA_HOME    : %JAVA_HOME%
echo [INFO] ANDROID_HOME : %ANDROID_HOME%
echo.

:: 3. Sincronizar assets con Capacitor
echo [1/3] Sincronizando assets web con Capacitor...
cd /d "%~dp0.."
call npx.cmd cap sync android
if %ERRORLEVEL% neq 0 (
    echo [ADVERTENCIA] Falla en npx cap sync, intentando compilar directamente...
)

:: 4. Compilar con Gradle
echo.
echo [2/3] Compilando APK con Gradle assembleDebug...
cd /d "%~dp0"
call gradlew.bat assembleDebug

if %ERRORLEVEL% equ 0 (
    echo.
    echo ================================================================================
    echo  🎉 [EXITO] APK compilada correctamente en:
    echo  %~dp0app\build\outputs\apk\debug\app-debug.apk
    echo ================================================================================
    
    :: 5. Copiar a carpeta funcional para distribución rápida
    if exist "%~dp0app\build\outputs\apk\debug\app-debug.apk" (
        copy /y "%~dp0app\build\outputs\apk\debug\app-debug.apk" "%~dp0..\..\IOT_VIAL_Semaforos_v9.0.apk" >nul
        echo [3/3] Copia maestra generada en: 05_Funcional\IOT_VIAL_Semaforos_v9.0.apk
    )
) else (
    echo.
    echo ================================================================================
    echo  ❌ [FALLA] Error durante la compilacion de la APK con Gradle.
    echo ================================================================================
    exit /b 1
)

endlocal
