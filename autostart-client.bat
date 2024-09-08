@echo off
setlocal

set "scriptPath=%~dp0"
set "shortcutPath=%scriptPath%Centaurus-Client.exe.lnk"
reg add "HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Run" /v "CentaurusClient" /t REG_SZ /d "\"%shortcutPath%\"" /f

endlocal