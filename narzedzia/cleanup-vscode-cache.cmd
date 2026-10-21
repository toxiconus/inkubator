@echo off
REM Bezpieczne usuwanie cache VS Code w profilu użytkownika toxic
SET USER_PROFILE=%USERPROFILE%
SET CODE_ROAMING=%USER_PROFILE%\AppData\Roaming\Code

echo Removing VS Code cache directories...
if exist "%CODE_ROAMING%\CachedExtensionVSIXs" (
    echo  - Deleting CachedExtensionVSIXs
    rd /s /q "%CODE_ROAMING%\CachedExtensionVSIXs"
) else (
    echo  - CachedExtensionVSIXs not found
)
if exist "%CODE_ROAMING%\Crashpad" (
    echo  - Deleting Crashpad
    rd /s /q "%CODE_ROAMING%\Crashpad"
) else (
    echo  - Crashpad not found
)
if exist "%CODE_ROAMING%\CachedData" (
    echo  - Deleting CachedData
    rd /s /q "%CODE_ROAMING%\CachedData"
) else (
    echo  - CachedData not found
)
if exist "%CODE_ROAMING%\Cache" (
    echo  - Deleting Cache
    rd /s /q "%CODE_ROAMING%\Cache"
) else (
    echo  - Cache not found
)
if exist "%CODE_ROAMING%\logs" (
    echo  - Deleting logs
    rd /s /q "%CODE_ROAMING%\logs"
) else (
    echo  - logs not found
)
echo Cleanup complete.
pause
