@echo off
setlocal EnableExtensions
cd /d "%~dp0"
chcp 65001 >nul
title Сборка модуля учета отпусков

rem Тихий режим при вызове из install.bat: build.bat quiet
set "QUIET="
if /i "%~1"=="quiet" set "QUIET=1"

echo.
echo =================================================
echo   Сборка модуля учета отпусков
echo =================================================
echo.

rem --- поиск окружения Visual C++ ---
set "VCVARS="
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if exist "%VSWHERE%" for /f "usebackq delims=" %%I in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath 2^>nul`) do (
    if exist "%%I\VC\Auxiliary\Build\vcvars64.bat" set "VCVARS=%%I\VC\Auxiliary\Build\vcvars64.bat"
)

if not defined VCVARS (
    for %%E in (Community Professional Enterprise BuildTools) do (
        if not defined VCVARS if exist "%ProgramFiles%\Microsoft Visual Studio\2022\%%E\VC\Auxiliary\Build\vcvars64.bat" set "VCVARS=%ProgramFiles%\Microsoft Visual Studio\2022\%%E\VC\Auxiliary\Build\vcvars64.bat"
        if not defined VCVARS if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\2022\%%E\VC\Auxiliary\Build\vcvars64.bat" set "VCVARS=%ProgramFiles(x86)%\Microsoft Visual Studio\2022\%%E\VC\Auxiliary\Build\vcvars64.bat"
        if not defined VCVARS if exist "%ProgramFiles%\Microsoft Visual Studio\2019\%%E\VC\Auxiliary\Build\vcvars64.bat" set "VCVARS=%ProgramFiles%\Microsoft Visual Studio\2019\%%E\VC\Auxiliary\Build\vcvars64.bat"
        if not defined VCVARS if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\%%E\VC\Auxiliary\Build\vcvars64.bat" set "VCVARS=%ProgramFiles(x86)%\Microsoft Visual Studio\2019\%%E\VC\Auxiliary\Build\vcvars64.bat"
    )
)

if defined VCVARS (
    echo   Найдено: %VCVARS%
    call "%VCVARS%" >nul 2>&1
) else (
    where cl.exe >nul 2>&1
    if errorlevel 1 goto :no_msvc
    echo   Используется компилятор из PATH.
)

where cl.exe >nul 2>&1
if errorlevel 1 goto :no_msvc

if not exist "build" mkdir "build"

set "CXXFLAGS=/nologo /std:c++20 /EHsc /utf-8 /O2 /MT /W3 /D_CRT_SECURE_NO_WARNINGS /Iinclude"
set "APP_SRC=app\main.cpp src\date.cpp src\log.cpp src\parol.cpp src\sql_hranilishche.cpp src\tekst.cpp src\uchet_otpuskov.cpp src\konsolnoe_menu.cpp"
set "TEST_SRC=tests\proverka.cpp src\date.cpp src\log.cpp src\parol.cpp src\tekst.cpp src\uchet_otpuskov.cpp"

echo [1/2] Сборка build\otpusk.exe ...
cl %CXXFLAGS% %APP_SRC% /Fo:build\ /Fe:build\otpusk.exe /link odbc32.lib advapi32.lib
if errorlevel 1 goto :fail

echo.
echo [2/2] Сборка build\proverka.exe ...
cl %CXXFLAGS% %TEST_SRC% /Fo:build\ /Fe:build\proverka.exe /link advapi32.lib
if errorlevel 1 goto :fail

echo.
echo Сборка завершена успешно:
echo   build\otpusk.exe    - основная программа
echo   build\proverka.exe  - автотесты
echo.
if not defined QUIET pause
exit /b 0

:no_msvc
echo.
echo [ОШИБКА] Не найден компилятор C++ (cl.exe).
echo.
echo Установите Visual Studio 2022 Community и при установке отметьте
echo компонент "Разработка классических приложений на C++":
echo   https://visualstudio.microsoft.com/ru/vs/community/
echo.
echo После установки запустите build.bat еще раз.
goto :fail

:fail
echo.
echo Сборка прервана из-за ошибки.
if not defined QUIET pause
exit /b 1
