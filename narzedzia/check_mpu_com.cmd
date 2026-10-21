@echo off
setlocal
set "scriptDir=%~dp0"
set "scriptPath=%scriptDir%check_mpu_com.ps1"
if not exist "%scriptPath%" (
    echo Nie znaleziono pliku: %scriptPath%
    exit /b 1
)
powershell -NoProfile -ExecutionPolicy Bypass -File "%scriptPath%"
exit /b %ERRORLEVEL%
