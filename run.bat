@echo off
setlocal EnableExtensions
cd /d "%~dp0"
chcp 65001 >nul
title Модуль учета отпусков

if not exist "build\otpusk.exe" (
    echo Программа еще не собрана. Запускаю сборку...
    call "%~dp0build.bat" quiet
    if errorlevel 1 exit /b 1
)

if not exist "server.txt" if not exist "build\server.txt" (
    echo.
    echo [ВНИМАНИЕ] Файл server.txt не найден.
    echo Сначала запустите install.bat: он настроит базу данных.
    echo Если база уже настроена, создайте server.txt с именем сервера,
    echo например: localhost\SQLEXPRESS
    echo.
    pause
    exit /b 1
)

"build\otpusk.exe"
set "CODE=%ERRORLEVEL%"
echo.
if not "%CODE%"=="0" (
    echo Программа завершилась с кодом %CODE%.
    echo Подробности подключения ищите в zhurnal_programmy.txt
)
pause
exit /b %CODE%
