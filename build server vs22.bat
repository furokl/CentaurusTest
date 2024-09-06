@echo off
chcp 65001 > nul

cd Server

if exist .\build (
	echo.
	echo rd /s /q .\build
	rd /s /q .\build
)

if exist .\build (
    pause
    exit /b 1
) else (
	echo -- Success delete Client build directory.
)

mkdir build
cd build

echo.
echo cmake -G "Visual Studio 17 2022" -A x64 ..
cmake -G "Visual Studio 17 2022" -A x64 ..

echo.
echo --build . --config Release
cmake --build . --config Release

echo.

for %%f in (*.sln) do (
	echo -- Shortcut created for %%~ff.
	powershell -command "$s=(New-Object -COM WScript.Shell).CreateShortcut('..\\..\\%%~nf.lnk'); $s.TargetPath='%%~ff'; $s.Save()"
	
)

if exist Release (
	cd Release
	for %%e in (*.exe) do (
		echo -- Shortcut created for %%~ffe.
		powershell -command "$s=(New-Object -COM WScript.Shell).CreateShortcut('..\\..\\..\\%%~nxe.lnk'); $s.TargetPath='%%~ffe'; $s.Save()"
	)
	cd ..
)

pause
