@echo off
setlocal EnableExtensions
for /f "tokens=2 delims=:" %%C in ('chcp') do set "OLDCP=%%C"
chcp 65001 >nul
cd /d "%~dp0"
title Установка модуля учета отпусков

set "ERRFILE=%TEMP%\otpusk_install_err.txt"
set "SQLOUT=%TEMP%\otpusk_install_sql.txt"
set "CERTFLAG="
set "CODEPAGE_FLAG="
set "SQLCMD="

echo.
echo =================================================
echo   Модуль учета отпусков: установка
echo =================================================
echo.

rem --- 1. сборка программы, если ее еще нет ---
if not exist "build\otpusk.exe" (
    echo [1/3] Программа еще не собрана. Запускаю build.bat...
    call "%~dp0build.bat" quiet
    if errorlevel 1 (
        echo.
        echo Не удалось собрать программу. Установите Visual Studio 2022
        echo с компонентом "Разработка классических приложений на C++"
        echo и запустите install.bat снова.
        goto :fail
    )
) else (
    echo [1/3] Собранная программа найдена: build\otpusk.exe
)

rem --- 2. поиск sqlcmd ---
for %%C in (sqlcmd.exe) do if not defined SQLCMD set "SQLCMD=%%~$PATH:C"
if not defined SQLCMD if exist "%ProgramFiles%\Microsoft SQL Server\Client SDK\ODBC\170\Tools\Binn\SQLCMD.EXE" set "SQLCMD=%ProgramFiles%\Microsoft SQL Server\Client SDK\ODBC\170\Tools\Binn\SQLCMD.EXE"
if not defined SQLCMD if exist "%ProgramFiles%\Microsoft SQL Server\Client SDK\ODBC\180\Tools\Binn\SQLCMD.EXE" set "SQLCMD=%ProgramFiles%\Microsoft SQL Server\Client SDK\ODBC\180\Tools\Binn\SQLCMD.EXE"
for %%V in (130 140 150 160 170 180) do (
    if not defined SQLCMD if exist "%ProgramFiles%\Microsoft SQL Server\%%V\Tools\Binn\SQLCMD.EXE" set "SQLCMD=%ProgramFiles%\Microsoft SQL Server\%%V\Tools\Binn\SQLCMD.EXE"
    if not defined SQLCMD if exist "%ProgramFiles(x86)%\Microsoft SQL Server\%%V\Tools\Binn\SQLCMD.EXE" set "SQLCMD=%ProgramFiles(x86)%\Microsoft SQL Server\%%V\Tools\Binn\SQLCMD.EXE"
)
if not defined SQLCMD goto :no_sqlcmd
echo   sqlcmd найден: %SQLCMD%

rem --- поддержка UTF-8 в sqlcmd (старые версии ее не знают) ---
"%SQLCMD%" -? 2>&1 | findstr /i /c:"codepage" >nul
if not errorlevel 1 set "CODEPAGE_FLAG=-f 65001"

rem --- 3. проверка драйвера ODBC ---
set "ODBC="
for %%D in ("ODBC Driver 18 for SQL Server" "ODBC Driver 17 for SQL Server" "ODBC Driver 13 for SQL Server" "SQL Server") do (
    if not defined ODBC (
        reg query "HKLM\SOFTWARE\ODBC\ODBCINST.INI\ODBC Drivers" /v %%D >nul 2>&1
        if not errorlevel 1 set "ODBC=%%~D"
    )
)
if not defined ODBC (
    echo   [ВНИМАНИЕ] Драйвер ODBC для SQL Server не найден.
    echo              Установите "ODBC Driver 17 for SQL Server" или версию 18,
    echo              иначе программа не сможет подключиться к базе.
) else (
    echo   Драйвер ODBC найден: %ODBC%
)

rem --- обязательные файлы ---
set "SQL01=sql\01_sozdanie_bazy.sql"
set "SQL02=sql\02_zapolnenie.sql"
set "SQL04=sql\04_roli.sql"
set "SQL05=sql\05_audit.sql"
set "SQL07=sql\07_polzovateli.sql"
for %%F in ("%SQL01%" "%SQL02%" "%SQL04%" "%SQL05%" "%SQL07%") do if not exist "%%~F" goto :no_files

rem --- имя сервера ---
set "SERVER="
if exist "server.txt" for /f "usebackq delims=" %%S in ("server.txt") do if not defined SERVER set "SERVER=%%S"
if not defined SERVER set "SERVER=localhost"
echo.
echo Укажите имя SQL Server. Примеры: localhost, localhost\SQLEXPRESS, .\SQLEXPRESS
:ask_server
set "INPUT="
set /p "INPUT=Имя сервера [%SERVER%]: "
if not "%INPUT%"=="" set "SERVER=%INPUT%"
set /a TRY=0
:ping
set /a TRY+=1
if %TRY% GTR 5 (
    echo [ОШИБКА] Не удалось подключиться к "%SERVER%" за 5 попыток.
    goto :fail
)
"%SQLCMD%" -S "%SERVER%" -E %CERTFLAG% -l 5 -Q "SET NOCOUNT ON; SELECT @@SERVERNAME;" >nul 2>"%ERRFILE%"
if not errorlevel 1 goto :connected
if not defined CERTFLAG (
    set "CERTFLAG=-C"
    goto :ping
)
echo Подключиться к "%SERVER%" не удалось:
if exist "%ERRFILE%" type "%ERRFILE%"
echo Проверьте, что служба SQL Server запущена, имя экземпляра верное,
echo а учетная запись Windows имеет права на сервер.
echo.
goto :ask_server
:connected
echo   Подключение установлено.

> "server.txt" echo(%SERVER%
if exist "build" > "build\server.txt" echo(%SERVER%
echo   Имя сервера сохранено в server.txt.

echo.
echo ВНИМАНИЕ: скрипт sql\01_sozdanie_bazy.sql создает базу OtpuskKompanii заново.
echo Если база уже существует, все данные в ней будут удалены без возможности восстановления.
:confirm
set "ANSWER="
set /p "ANSWER=Продолжить установку? [y/N]: "
if /i "%ANSWER%"=="y" goto :install
if /i "%ANSWER%"=="yes" goto :install
if /i "%ANSWER%"=="д" goto :install
echo.
echo Установка отменена. Данные в базе не изменялись.
goto :done

:install
echo.
echo [2/3] Создание базы данных...
call :runsql "%SQL01%" || goto :fail
echo   Роли базы данных...
call :runsql "%SQL04%" || goto :fail
echo   Журнал аудита и триггеры...
call :runsql "%SQL05%" || goto :fail
echo   Пользователи программы...
call :runsql "%SQL07%" || goto :fail
echo Обязательные скрипты выполнены.

echo.
:ask_data
set "ANSWER="
set /p "ANSWER=Добавить демонстрационные данные (сотрудники и отпуска)? [y/N]: "
if /i "%ANSWER%"=="y" goto :add_data
if /i "%ANSWER%"=="yes" goto :add_data
if /i "%ANSWER%"=="д" goto :add_data
goto :tests
:add_data
echo.
call :runsql "%SQL02%" || goto :fail
echo Демонстрационные данные добавлены.

:tests
echo.
echo [3/3] Проверка программы...
if not exist "build\proverka.exe" (
    echo Автотесты не запущены: build\proverka.exe не найден.
    goto :launch
)
"build\proverka.exe"
if errorlevel 1 (
    echo [ВНИМАНИЕ] В автотестах есть ошибки, подробности выше.
) else (
    echo Автотесты пройдены.
)

:launch
echo.
:ask_run
set "ANSWER="
set /p "ANSWER=Запустить программу сейчас? [y/N]: "
if /i "%ANSWER%"=="y" goto :run
if /i "%ANSWER%"=="yes" goto :run
if /i "%ANSWER%"=="д" goto :run
goto :done
:run
echo.
"build\otpusk.exe"
if errorlevel 1 (
    echo.
    echo Программа завершилась с ошибкой. Проверьте имя сервера в server.txt
    echo и журнал zhurnal_programmy.txt.
)
goto :done

:done
echo.
echo Установка завершена.
echo Запустить программу позже можно через run.bat или build\otpusk.exe
if defined OLDCP chcp %OLDCP% >nul
pause
exit /b 0

:fail
echo.
echo Установка прервана из-за ошибки.
if defined OLDCP chcp %OLDCP% >nul
pause
exit /b 1

:no_sqlcmd
echo.
echo [ОШИБКА] Не найден sqlcmd.exe.
echo.
echo Установите SQL Server Management Studio или пакет SQL Server Command Line Utilities.
echo Можно также выполнить скрипты из папки sql вручную в SSMS:
echo   01_sozdanie_bazy.sql, 04_roli.sql, 05_audit.sql, 07_polzovateli.sql
echo Подробности есть в README.md.
goto :fail

:no_files
echo.
echo [ОШИБКА] В папке sql отсутствуют нужные скрипты.
goto :fail

:runsql
echo   Скрипт %~nx1 ...
"%SQLCMD%" -S "%SERVER%" -E %CERTFLAG% -d master %CODEPAGE_FLAG% -l 30 -b -i "%~1" >"%SQLOUT%" 2>&1
if errorlevel 1 (
    echo [ОШИБКА] Скрипт %~nx1 завершился с ошибкой:
    type "%SQLOUT%"
    exit /b 1
)
exit /b 0
