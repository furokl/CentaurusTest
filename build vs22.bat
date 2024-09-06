@echo off

if exist .\Build (
	echo.
	echo rd /s /q .\Build
	rd /s /q .\Build
)

if exist .\Build (
    pause
    exit /b 1
) else (
	echo -- Success delete Build directory.
)

mkdir Build
cd Build

echo.
echo cmake -G "Visual Studio 17 2022" -A x64 ..
cmake -G "Visual Studio 17 2022" -A x64 ..

echo.
echo --build . --config Release
cmake --build . --config Release

echo.

for %%f in (*.sln) do (
	echo -- Shortcut created for %%~ff.
	powershell -command "$s=(New-Object -COM WScript.Shell).CreateShortcut('..\\%%~nf.lnk'); $s.TargetPath='%%~ff'; $s.Save()"
	
)

if exist Client\\Release (
	cd Client\\Release
	for %%e in (*.exe) do (
		echo -- Shortcut created for %%~ffe.
		powershell -command "$s=(New-Object -COM WScript.Shell).CreateShortcut('..\\..\\..\\%%~nxe.lnk'); $s.TargetPath='%%~ffe'; $s.Save()"
	)
	cd ..\\..
)

if exist Server\\Release (
	cd Server\\Release
	for %%e in (*.exe) do (
		echo -- Shortcut created for %%~ffe.
		powershell -command "$s=(New-Object -COM WScript.Shell).CreateShortcut('..\\..\\..\\%%~nxe.lnk'); $s.TargetPath='%%~ffe'; $s.Save()"
	)
	cd ..\\..
)

pause
