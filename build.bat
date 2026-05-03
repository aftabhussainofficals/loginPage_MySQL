@echo off
set MYSQL_DIR=C:\Program Files\MySQL\MySQL Server 9.7

g++ main.cpp -o login ^
  -I"%MYSQL_DIR%\include" ^
  -L"%MYSQL_DIR%\lib" ^
  -lmysql

if %ERRORLEVEL% == 0 (
    echo Build successful! Run: login.exe
) else (
    echo Build failed.
)
