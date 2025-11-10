@echo off
echo Compiling OS Simulator Backend...
echo.

echo Compiling Phase 1...
g++ -o phase1.exe phase1.cpp
if %errorlevel% neq 0 (
    echo Error compiling phase1.cpp
    pause
    exit /b %errorlevel%
)

echo Compiling Phase 2...
g++ -o phase2.exe phase2.cpp
if %errorlevel% neq 0 (
    echo Error compiling phase2.cpp
    pause
    exit /b %errorlevel%
)

echo.
echo Compilation successful!
echo Files created: phase1.exe, phase2.exe
echo.
pause