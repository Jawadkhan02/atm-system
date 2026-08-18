@echo off
echo ============================================
echo   Building ATM System...
echo ============================================

if not exist build mkdir build
cd build

cmake .. -G "MinGW Makefiles" 2>nul
if %errorlevel% neq 0 (
    echo Trying with default generator...
    cmake ..
)

cmake --build .

if %errorlevel% equ 0 (
    echo.
    echo ============================================
    echo   Build successful!
    echo   Run: build\atm_system.exe
    echo ============================================
) else (
    echo.
    echo ============================================
    echo   Build failed. Check for errors above.
    echo ============================================
)

cd ..
