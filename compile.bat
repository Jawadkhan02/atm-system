@echo off
echo ============================================
echo   Compiling ATM System with g++...
echo ============================================

g++ -std=c++17 -I include -o atm_system.exe ^
    src/main.cpp ^
    src/transaction.cpp ^
    src/card.cpp ^
    src/account.cpp ^
    src/customer.cpp ^
    src/bank.cpp ^
    src/atm.cpp

if %errorlevel% equ 0 (
    echo.
    echo ============================================
    echo   Compilation successful!
    echo   Run: atm_system.exe
    echo ============================================
) else (
    echo.
    echo ============================================
    echo   Compilation failed.
    echo   Make sure g++ is installed and in PATH.
    echo ============================================
)
