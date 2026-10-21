@echo off
REM Bezpieczne porządki VS Code:
REM - usuwa workspaceStorage
REM - usuwa historię VS Code
REM - usuwa pamięć Copilot Chat
REM - usuwa cache VS Code, ale nie usuwa samego VS Code ani rozszerzeń

SET USER_PROFILE=%USERPROFILE%
SET CODE_ROAMING=%USER_PROFILE%\AppData\Roaming\Code
SET VS_CODE_USER=%CODE_ROAMING%\User

echo Cleaning VS Code state and Copilot chat caches...

if exist "%VS_CODE_USER%\workspaceStorage" (
    echo  - Removing workspaceStorage
    rd /s /q "%VS_CODE_USER%\workspaceStorage"
) else (
    echo  - workspaceStorage missing
)

if exist "%VS_CODE_USER%\History" (
    echo  - Removing History
    rd /s /q "%VS_CODE_USER%\History"
) else (
    echo  - History missing
)

if exist "%VS_CODE_USER%\globalStorage\github.copilot-chat" (
    echo  - Removing Copilot chat storage
    rd /s /q "%VS_CODE_USER%\globalStorage\github.copilot-chat"
) else (
    echo  - Copilot chat storage missing
)

if exist "%VS_CODE_USER%\globalStorage\emptyWindowChatSessions" (
    echo  - Removing emptyWindowChatSessions
    rd /s /q "%VS_CODE_USER%\globalStorage\emptyWindowChatSessions"
) else (
    echo  - emptyWindowChatSessions missing
)

if exist "%USER_PROFILE%\.vscode\extensions\continue.continue-2.0.0-win32-x64" (
    echo  - Leaving continue extension installed, but remove manually if not needed.
) else (
    echo  - continue extension not found
)

echo Done. Restart VS Code after running this script.
pause
