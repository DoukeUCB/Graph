@echo off
echo ===================================
echo  Compilando Red Vial Urbana...
echo ===================================
echo.
g++ -std=c++17 -O2 -Wall -o red_vial.exe src/LectorCSV.cpp src/Grafo.cpp src/Algoritmos.cpp src/main.cpp -Isrc
if %ERRORLEVEL% EQU 0 (
    echo.
    echo Compilacion exitosa! Ejecute: red_vial.exe
) else (
    echo.
    echo Error en la compilacion.
)
echo.
pause
