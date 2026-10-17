@echo off
setlocal
cd /d "%~dp0"
cmake -S . -B build
if errorlevel 1 exit /b 1
cmake --build build --config Release --parallel
if errorlevel 1 exit /b 1
if exist build\Release\dependencies.exe (
    build\Release\dependencies.exe %*
) else (
    build\dependencies.exe %*
)
