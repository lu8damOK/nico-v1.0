@echo off
setlocal EnableDelayedExpansion
chcp 65001 >nul
echo ========================================
echo   Compilando Nico v1.0 para Windows 11
echo ========================================
echo.

:: Verificar gcc
where gcc >nul 2>&1
if %errorlevel% neq 0 (
    echo [ERROR] gcc no encontrado.
    echo Instale MinGW-w64 y agregue la carpeta bin al PATH.
    echo https://github.com/niXman/mingw-builds-binaries/releases
    pause
    exit /b 1
)

echo [1/2] Compilando fuentes...
set CFILES=
for %%f in (*.c) do (
    if /I not "%%f"=="nico_gpio_stub.c" set CFILES=!CFILES! %%f
)

gcc -o nico.exe %CFILES% ^
    -std=c11 -O2 -Wall -Wextra -Wno-stringop-truncation -Wno-unused-parameter ^
    -lm -lpthread ^
    -DWIN32_LEAN_AND_MEAN ^
    -include win_compat.h ^
    -I. 2>compile_errors.txt

if %errorlevel% neq 0 (
    echo [ERROR] Compilacion fallida. Detalles:
    type compile_errors.txt
    pause
    exit /b 1
)
del compile_errors.txt >nul 2>&1

echo.
echo [2/2] Verificando binario...
if exist nico.exe (
    echo [OK] nico.exe generado correctamente.
) else (
    echo [ERROR] Error desconocido.
    pause
    exit /b 1
)

echo.
echo ========================================
echo   ¡Listo! Use: nico.exe archivo.nico
echo ========================================
endlocal
pause
