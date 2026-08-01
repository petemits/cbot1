@echo off
chcp 65001 >nul
echo ==========================================
echo    LEAD BOT - FIXED VERSION
echo ==========================================
echo.

echo 🔧 Checking compiler...
where g++ >nul 2>&1
if %errorlevel% neq 0 (
    echo ❌ C++ compiler not found.
    echo 📥 Please install TDM-GCC from:
    echo    https://jmeubank.github.io/tdm-gcc/
    pause
    exit /b 1
)

echo ✅ Compiler found.
echo.
echo 🔨 Compiling Fixed Lead Bot...

:: Simple compilation without complex flags
g++ lead_bot_fixed.cpp -o leadbot.exe -lwininet -lws2_32 -std=c++11

if exist leadbot.exe (
    echo.
    echo ✅ COMPILATION SUCCESSFUL!
    echo.
    echo 🚀 To run the bot:
    echo    leadbot.exe
    echo.
    echo 📁 Reports will be saved in 'reports/' folder
    echo 💾 RAM usage: ~15MB
    echo.
    pause
) else (
    echo.
    echo ❌ Compilation failed.
    echo.
    echo 🔧 Trying with minimal flags...
    g++ lead_bot_fixed.cpp -o leadbot.exe -lwininet
    
    if exist leadbot.exe (
        echo ✅ SUCCESS with minimal flags!
        echo 🚀 Run: leadbot.exe
        pause
    ) else (
        echo ❌ Still failed.
        echo 💡 Make sure Windows SDK is installed.
        pause
    )
)