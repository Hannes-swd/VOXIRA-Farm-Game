@echo off
echo === reset_world.exe wird gebaut ===
echo.

if not exist build (
    echo Build-Ordner wird erstellt...
    mkdir build
)

cd build

echo CMake wird konfiguriert...
cmake .. -G "Visual Studio 17 2022" -A x64
if errorlevel 1 (
    echo.
    echo CMake Konfiguration fehlgeschlagen! Versuche MinGW...
    cmake .. -G "MinGW Makefiles"
    if errorlevel 1 (
        echo CMake Konfiguration fehlgeschlagen!
        pause
        exit /b 1
    )
)

echo.
echo reset_world wird gebaut...
cmake --build . --target reset_world --config Release
if errorlevel 1 (
    echo.
    echo Build fehlgeschlagen!
    pause
    exit /b 1
)

cd ..
echo.
echo === Fertig! reset_world.exe liegt im Projektordner ===
pause
